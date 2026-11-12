// Lane A44: the paraphrase check of the Designer's key families through the real chain (claude CLI, R-0004 point 2) over hockey.regular.
//   node Game/Source/JamDesigner/Tools/paraphrase-keys.mjs [--only <family>] [--out <json>]
// One sentence per family (the brief's seven). A family passes when the reply is a valid cut, its key changed to a sensible value, and
// every other changed path is inside the family (a theme change is tolerated and noted: the prompt lets the model rename the cut).
// Prints a markdown table and one JSON line last; exit 0 every family passes, 1 a miss, 2 could not run. Spends one CLI call per
// attempt (at most three per sentence); the count and the cost are in the JSON line. The C++ twin of the deterministic part is
// Jam.Designer.KeyFamilies (mock replies); its live section runs these sentences through the game's own transport.
import { readFileSync, writeFileSync, mkdirSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { runDesigner } from '../../../../tools/designer-eval/loop.mjs';
import { createCliModel, hasCli } from '../../../../tools/designer-eval/cli-model.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..', '..', '..');

// paths: JSON pointers (prefixes) the family may change; tolerated: changed, noted, not a miss; expect: the sensible value.
export const FAMILIES = [
  { id: 'one_timer', sentence: 'make one-timers deadly', paths: ['/game/shot/one_timer'], expect: (g) => (g.shot?.one_timer?.window_frames ?? 10) > 10 || (g.shot?.one_timer?.cone_bonus_deg ?? 8) > 8, want: 'shot.one_timer window > 10 or cone > 8' },
  { id: 'breakaway', sentence: 'give breakaways a burst', paths: ['/game/skater/breakaway'], expect: (g) => (g.skater?.breakaway?.speed_bonus ?? 0.2) > 0.2 || (g.skater?.breakaway?.frames ?? 60) > 60, want: 'skater.breakaway speed_bonus > 0.2 or frames > 60' },
  { id: 'deke', sentence: 'let me deke the last man', paths: ['/game/stick/deke'], expect: (g) => (g.stick?.deke?.offset_m ?? 1) > 1 || (g.stick?.deke?.commit_frames ?? 15) > 15, want: 'stick.deke offset > 1 or commit > 15' },
  { id: 'commentary', sentence: 'a quiet commentator', paths: ['/game/commentary'], expect: (g) => g.commentary?.intensity === 'quiet', want: 'commentary.intensity quiet' },
  { id: 'camera', sentence: 'camera higher at the end boards', paths: ['/game/camera/end_height_m'], expect: (g) => (g.camera?.end_height_m ?? 5) > 5, want: 'camera.end_height_m > 5' },
  { id: 'foul_grace', sentence: 'no cheap fouls after a strip', paths: ['/game/stick/foul_grace_frames'], expect: (g) => (g.stick?.foul_grace_frames ?? 12) > 12, want: 'stick.foul_grace_frames > 12' },
  { id: 'broadcast', sentence: 'the broadcast look', paths: ['/game/look/tier'], tolerated: ['/game/look/trim_emissive'], expect: (g) => g.look?.tier === 'p4_broadcast', want: 'look.tier p4_broadcast' },
];
const TOLERATED = ['/theme'];

const isObj = (v) => v !== null && typeof v === 'object' && !Array.isArray(v);
// Leaf differences between two JSON values as { ptr, before, after }. An object added where the other side has none (hockey.regular has
// no `stick`, no `one_timer`) is walked leaf by leaf, so the pointers name the keys that appeared.
export function diff(a, b, ptr = '', out = []) {
  const objs = (isObj(a) || a === undefined) && (isObj(b) || b === undefined) && (isObj(a) || isObj(b));
  if (objs || (Array.isArray(a) && Array.isArray(b) && a.length === b.length)) {
    for (const k of new Set([...Object.keys(a ?? {}), ...Object.keys(b ?? {})])) diff(a?.[k], b?.[k], `${ptr}/${String(k).replace(/~/g, '~0').replace(/\//g, '~1')}`, out);
  } else if (JSON.stringify(a) !== JSON.stringify(b)) out.push({ ptr, before: a, after: b });
  return out;
}

export function judge(family, base, r) {
  const row = { id: family.id, sentence: family.sentence, outcome: r.outcome, via: r.attempts.map((a) => `${a.model.replace(/^claude-/, '')}:${a.kind}`).join(' > '), ms: r.ms, tokens_in: r.tokens_in, tokens_out: r.tokens_out, changed: [], notes: [], verdict: 'miss' };
  if (!r.cartridge) { row.notes.push(r.refusal ? `refusal: ${r.refusal}` : (r.attempts.at(-1)?.errors ?? []).slice(0, 2).join(' | ')); return row; }
  const changes = diff(base, r.cartridge);
  row.changed = changes.map((c) => `${c.ptr} ${JSON.stringify(c.before)} -> ${JSON.stringify(c.after)}`);
  const inside = (ptr, list) => list.some((p) => ptr === p || ptr.startsWith(`${p}/`));
  const own = changes.filter((c) => inside(c.ptr, family.paths));
  const noted = changes.filter((c) => !inside(c.ptr, family.paths) && inside(c.ptr, [...TOLERATED, ...(family.tolerated ?? [])]));
  const foreign = changes.filter((c) => !inside(c.ptr, [...family.paths, ...TOLERATED, ...(family.tolerated ?? [])]));
  if (own.length === 0) row.notes.push(`the family's key did not change (${family.want})`);
  else if (!family.expect(r.cartridge.game ?? {})) row.notes.push(`the value is not the sensible one (${family.want})`);
  if (noted.length) row.notes.push(`also changed (tolerated): ${noted.map((c) => c.ptr).join(', ')}`);
  if (foreign.length) row.notes.push(`changed outside the family: ${foreign.map((c) => c.ptr).join(', ')}`);
  row.verdict = own.length > 0 && family.expect(r.cartridge.game ?? {}) && foreign.length === 0 ? (noted.length ? 'pass (noted)' : 'pass') : 'miss';
  return row;
}

export function table(rows) {
  const head = ['family', 'sentence', 'outcome', 'chain', 'changed', 'verdict', 'notes', 'tokens in/out', 'ms'];
  const line = (c) => `| ${c.map((x) => String(x).replace(/\|/g, '\\|')).join(' | ')} |`;
  return [line(head), `|${head.map(() => '---').join('|')}|`, ...rows.map((r) => line([r.id, r.sentence, r.outcome, r.via, r.changed.join('; '), r.verdict, r.notes.join('; '), `${r.tokens_in}/${r.tokens_out}`, r.ms]))].join('\n');
}

function argValue(name, fallback) {
  const i = process.argv.indexOf(name);
  return i === -1 ? fallback : process.argv[i + 1];
}

async function main() {
  if (!hasCli()) { console.log('FAIL paraphrase-keys: claude is not on PATH'); console.log(JSON.stringify({ tool: 'paraphrase-keys', ok: false, could_not_run: true })); return 2; }
  const only = argValue('--only', undefined);
  const out = argValue('--out', undefined);
  const base = JSON.parse(readFileSync(resolve(repo, 'Game/Content/Cartridges/hockey.regular.cartridge.json'), 'utf8'));
  const inner = createCliModel();
  let calls = 0;
  let costUsd = 0;
  const callModel = async (body) => { calls += 1; const r = await inner(body); costUsd += r.cost_usd ?? 0; return r; };
  const rows = [];
  for (const family of FAMILIES.filter((f) => only === undefined || f.id === only)) {
    const r = await runDesigner({ cartridge: base, sentence: family.sentence, callModel });
    const row = judge(family, base, r);
    rows.push(row);
    console.log(`${row.verdict.startsWith('pass') ? 'PASS' : 'FAIL'} ${family.id} [${r.outcome}] ${r.ms} ms "${family.sentence}" ${row.via}${row.notes.length ? ` (${row.notes.join('; ')})` : ''}`);
    for (const c of row.changed) console.log(`     ${c}`);
  }
  console.log('');
  console.log(table(rows));
  const misses = rows.filter((r) => r.verdict === 'miss').length;
  const firstTry = rows.filter((r) => r.outcome === 'first_try').length;
  const summary = { tool: 'paraphrase-keys', ok: misses === 0, families: rows.length, pass: rows.length - misses, miss: misses, first_try: firstTry, calls, cost_usd: Math.round(costUsd * 10000) / 10000, tokens_in: rows.reduce((s, r) => s + r.tokens_in, 0), tokens_out: rows.reduce((s, r) => s + r.tokens_out, 0), ms: rows.reduce((s, r) => s + r.ms, 0) };
  if (out) { mkdirSync(dirname(resolve(out)), { recursive: true }); writeFileSync(resolve(out), JSON.stringify({ ...summary, rows }, null, 2) + '\n'); }
  console.log(`${misses ? 'FAIL' : 'PASS'} paraphrase-keys ${rows.length - misses} of ${rows.length} families, ${firstTry} first try, ${calls} CLI calls`);
  console.log(JSON.stringify(summary));
  return misses ? 1 : 0;
}

if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  main().then((c) => { process.exitCode = c; }, (e) => { console.error(e.message); process.exitCode = 2; });
}
