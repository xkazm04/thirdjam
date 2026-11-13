// Lane A143 (track E1, B-answer-0024 ask 3/4): every commentator line id's text on A equals B's, and A's supplement lines carry the length B measured.
//   node tools/commentary/parity.mjs                 compare; PASS/FAIL lines and a JSON line last; exit 1 on a mismatch that is not a known pending one
//   node tools/commentary/parity.mjs --write-seconds set A's supplement `seconds` to B's measured lengths (status/AU10-report.md), then compare
// A's side: briefs/commentary/lines.json (the commentator lines gen-lines.mjs writes), Game/Content/Commentary/abilities|rough.commentary.json (A's
// supplements). B's side: B's manifest Game/Plugins/JamAssets/Content/Audio/VO/commentary.json (read only) and the "All 73 lines" table of
// status/AU10-report.md (B's recording of the 72 supplement lines and the re-recorded lose.blowout.01: text as written and the measured seconds).
// The caption and the no-overlap rule both run on `seconds` (JamCommentaryData.cpp: DurationFrames = ceil(seconds x 60); HUDWidget.cpp: the strip holds
// for DurationFrames; CommentarySystem.cpp: LastEndFrame = start + DurationFrames), so a placeholder shorter than the wave cuts the caption early.
import { readFileSync, writeFileSync, existsSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const here = dirname(fileURLToPath(import.meta.url));
export const repo = resolve(here, '..', '..');
export const A_LINES = 'briefs/commentary/lines.json';
export const SUPPLEMENTS = ['Game/Content/Commentary/abilities.commentary.json', 'Game/Content/Commentary/rough.commentary.json'];
export const B_MANIFEST = 'Game/Plugins/JamAssets/Content/Audio/VO/commentary.json';
export const B_TABLE = 'status/AU10-report.md';
// Ids whose new text A already holds while B's manifest in this tree is older (B's file reaches main with the Director's merge): B's old text, so the
// check passes on either side of the merge and reports the pending id. Remove an entry once B's manifest on main carries the new text.
export const PENDING_B = { 'lose.blowout.01': 'A night to forget. Back to rehearsals.' };   // stale since the merge of 13 Nov 23:29 (B's manifest matches): the test below still uses the entry as its fixture

const read = (root, p) => JSON.parse(readFileSync(resolve(root, p), 'utf8'));
const linesOf = (j) => (Array.isArray(j) ? j : j.lines);

// A's id -> { text, seconds, from }.
export function aSide(root = repo) {
  const m = new Map();
  for (const l of linesOf(read(root, A_LINES))) if ((l.speaker || 'commentator') === 'commentator') m.set(l.id, { text: l.text, from: A_LINES });
  for (const f of SUPPLEMENTS) for (const l of linesOf(read(root, f))) m.set(l.id, { text: l.text, seconds: l.seconds, from: f });
  return m;
}

// B's manifest id -> { text, seconds }.
export function bManifest(root = repo) {
  const m = new Map();
  for (const l of linesOf(read(root, B_MANIFEST))) m.set(l.id, { text: l.text, seconds: l.seconds });
  return m;
}

// B's recording table (markdown): id -> { text, seconds }. A '*' after the seconds marks a live line over 2.5 s.
export function parseTable(md) {
  const m = new Map();
  const at = md.indexOf('| id | text as written | s |');
  if (at < 0) return m;
  for (const row of md.slice(at).split(/\r?\n/).slice(2)) {
    if (!row.startsWith('|')) break;
    const c = row.split('|').map((s) => s.trim());
    const s = Number(String(c[3]).replace('*', '').trim());
    if (c[1] && Number.isFinite(s)) m.set(c[1], { text: c[2], seconds: s });
  }
  return m;
}

export function bTable(root = repo) {
  const p = resolve(root, B_TABLE);
  return existsSync(p) ? parseTable(readFileSync(p, 'utf8')) : new Map();
}

// The comparison. textMismatch: ids on both sides whose text differs (pending: B still holds the known old text); onlyA / onlyB: ids on one side;
// tableText: supplement lines whose text differs from what B recorded; seconds: supplement lines whose `seconds` is not B's measured length.
export function compare(a, b, table) {
  const r = { compared: 0, textMismatch: [], pending: [], onlyA: [], onlyB: [], tableText: [], seconds: [], stalePending: [] };
  for (const [id, x] of a) {
    const y = b.get(id);
    if (!y) { r.onlyA.push(id); continue; }
    ++r.compared;
    if (x.text === y.text) { if (PENDING_B[id]) r.stalePending.push(id); continue; }
    if (PENDING_B[id] === y.text) r.pending.push(id);
    else r.textMismatch.push({ id, a: x.text, b: y.text, from: x.from });
  }
  for (const id of b.keys()) if (!a.has(id)) r.onlyB.push(id);
  for (const [id, t] of table) {
    const x = a.get(id);
    if (!x) continue;
    if (x.text !== t.text) r.tableText.push({ id, a: x.text, b: t.text });
    if (x.seconds !== undefined && Math.abs(x.seconds - t.seconds) > 1e-9) r.seconds.push({ id, a: x.seconds, b: t.seconds });
  }
  return r;
}

// Sets each supplement line's `seconds` to B's measured length; keeps the files' formatting (2-space JSON and a final newline, as committed).
export function writeSeconds(root = repo, table = bTable(root)) {
  let changed = 0;
  for (const f of SUPPLEMENTS) {
    const p = resolve(root, f);
    const raw = readFileSync(p, 'utf8');
    const j = JSON.parse(raw);
    for (const l of linesOf(j)) {
      const t = table.get(l.id);
      if (t && l.seconds !== t.seconds) { l.seconds = t.seconds; ++changed; }
    }
    const indent = (raw.match(/\n( +)"/) || [, '  '])[1];
    writeFileSync(p, JSON.stringify(j, null, indent.length) + (raw.endsWith('\n') ? '\n' : ''));
  }
  return changed;
}

if (process.argv[1] && /parity\.mjs$/.test(process.argv[1].replace(/\\/g, '/'))) {
  if (process.argv.includes('--write-seconds')) console.log(`INFO parity: ${writeSeconds()} supplement lengths set from ${B_TABLE}`);
  const r = compare(aSide(), bManifest(), bTable());
  const bad = r.textMismatch.length + r.tableText.length + r.seconds.length;
  for (const m of r.textMismatch) console.log(`FAIL text ${m.id}: A (${m.from}) "${m.a}" | B "${m.b}"`);
  for (const m of r.tableText) console.log(`FAIL recorded ${m.id}: A "${m.a}" | B recorded "${m.b}"`);
  for (const m of r.seconds) console.log(`FAIL seconds ${m.id}: A ${m.a} | B measured ${m.b}`);
  for (const id of r.pending) console.log(`INFO pending ${id}: B's manifest in this tree still holds the old text; A holds B's new one`);
  for (const id of r.stalePending) console.log(`INFO stale pending ${id}: B's manifest now matches; remove it from PENDING_B`);
  console.log(`${bad ? 'FAIL' : 'PASS'} parity: ${r.compared} ids compared, ${r.onlyA.length} only on A (supplements B has not merged), ${r.onlyB.length} only on B`);
  console.log(JSON.stringify({ tool: 'thirdjam.commentary/1', script: 'parity', verdict: bad ? 'fail' : 'pass', compared: r.compared, text_mismatch: r.textMismatch.length,
    recorded_mismatch: r.tableText.length, seconds_mismatch: r.seconds.length, pending: r.pending, stale_pending: r.stalePending, only_a: r.onlyA.length, only_b: r.onlyB.length }));
  process.exit(bad ? 1 : 0);
}
