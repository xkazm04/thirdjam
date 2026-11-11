#!/usr/bin/env node
// Assets ledger gate. ASSETS.md is both the provenance record and the size ledger.
//
//   node tools/gate/check-assets-ledger.mjs [--root <repo>] [--ledger ASSETS.md] [--assets Game/Plugins/JamAssets/Content]
//
// Checks
//   1. every file under Game/Plugins/JamAssets/Content/** has a row in ASSETS.md (absent folder passes);
//   2. every row is well formed (kind, origin, device, shipped, size_mb; sourced rows carry licence and url,
//      ai-generated rows carry the tool);
//   3. shipped=y sizes per device and in total, per budget group, against the budgets of PLAN.md section 4b:
//      art (mesh, texture, material, anim) 350 MB, audio 80 MB. Over budget fails.
// Warnings (never fail): size_mb far from the size on disk, rows whose file is missing,
//   vfx/font/other total (their slice is 40 MB, owned by device A, not enforced here).
// Exit 0 pass, 1 fail, 2 could not run.
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

export const COLUMNS = ['path', 'kind', 'size_mb', 'origin', 'tool', 'licence', 'url', 'date', 'device', 'shipped', 'note'];
export const KINDS = ['mesh', 'texture', 'material', 'audio', 'vfx', 'anim', 'font', 'other'];
export const ORIGINS = ['original', 'ai-generated', 'sourced'];
export const BUDGETS_MB = { art: 350, audio: 80 };
export const GROUP_OF_KIND = { mesh: 'art', texture: 'art', material: 'art', anim: 'art', audio: 'audio', vfx: 'other', font: 'other', other: 'other' };
const IGNORED = /(^|\/)(\.gitkeep|\.gitattributes|\.DS_Store|Thumbs\.db|desktop\.ini)$/i;
const COMPANION = /\.(uexp|ubulk|uptnl)$/i;

const norm = (p) => p.trim().replace(/\\/g, '/').replace(/^\.\//, '').replace(/^`|`$/g, '');
const key = (p) => norm(p).toLowerCase();

function splitRow(line) {
  const s = line.trim().replace(/^\|/, '').replace(/\|$/, '');
  const cells = [];
  let cur = '';
  for (let i = 0; i < s.length; i++) {
    if (s[i] === '\\' && s[i + 1] === '|') {
      cur += '|';
      i++;
    } else if (s[i] === '|') {
      cells.push(cur.trim());
      cur = '';
    } else cur += s[i];
  }
  cells.push(cur.trim());
  return cells;
}

/** Parse the ledger table. Returns {rows, errors}. Throws if there is no ledger table at all. */
export function parseLedger(text) {
  const lines = text.replace(/\r\n?/g, '\n').split('\n');
  let i = lines.findIndex((l) => l.trim().startsWith('|') && splitRow(l).map((c) => c.toLowerCase()).includes('path') && splitRow(l).map((c) => c.toLowerCase()).includes('kind'));
  if (i < 0) throw new Error('no ledger table found (a header row with "path" and "kind")');
  const header = splitRow(lines[i]).map((c) => c.toLowerCase());
  const errors = [];
  if (header.join('|') !== COLUMNS.join('|')) {
    throw new Error(`ledger columns must be exactly: ${COLUMNS.join(' | ')} (found: ${header.join(' | ')})`);
  }
  i++;
  if (!lines[i] || !/^\s*\|?\s*:?-{2,}/.test(lines[i])) throw new Error('ledger header is not followed by a separator row');
  i++;
  const rows = [];
  for (; i < lines.length; i++) {
    const l = lines[i];
    if (!l.trim().startsWith('|')) break; // the table ends at the first non-table line
    const cells = splitRow(l);
    const row = Object.fromEntries(COLUMNS.map((c, n) => [c, (cells[n] ?? '').trim()]));
    row.line = i + 1;
    if (cells.length !== COLUMNS.length) errors.push(`ASSETS.md line ${i + 1}: ${cells.length} columns, expected ${COLUMNS.length}`);
    rows.push(row);
  }
  return { rows, errors };
}

export function validateRows(rows) {
  const errors = [];
  const seen = new Map();
  for (const r of rows) {
    const at = `ASSETS.md line ${r.line} (${r.path || 'no path'})`;
    if (!r.path) errors.push(`${at}: path is empty`);
    else {
      const k = key(r.path);
      if (seen.has(k)) errors.push(`${at}: duplicate of line ${seen.get(k)}`);
      seen.set(k, r.line);
    }
    if (!KINDS.includes(r.kind)) errors.push(`${at}: kind "${r.kind}" must be one of ${KINDS.join(', ')}`);
    if (!ORIGINS.includes(r.origin)) errors.push(`${at}: origin "${r.origin}" must be one of ${ORIGINS.join(', ')}`);
    if (!['A', 'B'].includes(r.device)) errors.push(`${at}: device "${r.device}" must be A or B`);
    if (!['y', 'n'].includes(r.shipped)) errors.push(`${at}: shipped "${r.shipped}" must be y or n`);
    if (!/^\d+(\.\d+)?$/.test(r.size_mb)) errors.push(`${at}: size_mb "${r.size_mb}" must be a number such as 12.4`);
    if (r.origin === 'sourced' && (!r.licence || !r.url)) errors.push(`${at}: sourced assets need licence and url (they must be noted at submission)`);
    if (r.origin === 'ai-generated' && !r.tool) errors.push(`${at}: ai-generated assets need the tool or service`);
    if (r.date && !/^\d{4}-\d{2}-\d{2}$/.test(r.date)) errors.push(`${at}: date "${r.date}" must be YYYY-MM-DD`);
  }
  return errors;
}

/** All asset files under a folder, as repo-relative paths. Companion files (.uexp ...) are folded into their .uasset. */
export function listAssetFiles(root, rel) {
  const base = path.join(root, rel);
  const files = [];
  if (!fs.existsSync(base)) return files;
  const walk = (d) => {
    for (const e of fs.readdirSync(d, { withFileTypes: true })) {
      const p = path.join(d, e.name);
      if (e.isDirectory()) walk(p);
      else if (e.isFile()) {
        const r = path.relative(root, p).split(path.sep).join('/');
        if (!IGNORED.test(r) && !COMPANION.test(r)) files.push(r);
      }
    }
  };
  walk(base);
  return files.sort();
}

/** On-disk size in MB of a file plus its same-stem companions. */
export function diskMb(root, rel) {
  const abs = path.join(root, rel);
  if (!fs.existsSync(abs)) return undefined;
  let bytes = fs.statSync(abs).size;
  const stem = abs.replace(/\.[^.\\/]+$/, '');
  for (const ext of ['.uexp', '.ubulk', '.uptnl']) if (fs.existsSync(stem + ext)) bytes += fs.statSync(stem + ext).size;
  return bytes / (1024 * 1024);
}

export function budgetReport(rows) {
  const groups = ['art', 'audio', 'other'];
  const sum = (device, group) =>
    Math.round(
      rows.filter((r) => r.shipped === 'y' && KINDS.includes(r.kind) && GROUP_OF_KIND[r.kind] === group && (!device || r.device === device)).reduce((a, r) => a + (parseFloat(r.size_mb) || 0), 0) * 10,
    ) / 10;
  const out = { A: {}, B: {}, total: {} };
  for (const g of groups) {
    out.A[g] = sum('A', g);
    out.B[g] = sum('B', g);
    out.total[g] = sum(undefined, g);
  }
  return out;
}

export function parseArgs(argv) {
  const o = { ledger: 'ASSETS.md', assets: 'Game/Plugins/JamAssets/Content' };
  for (let i = 0; i < argv.length; i++) {
    const a = argv[i];
    const take = () => {
      const v = argv[++i];
      if (v === undefined || v.startsWith('--')) throw new Error(`${a} needs a value`);
      return v;
    };
    if (a === '--root') o.root = take();
    else if (a === '--ledger') o.ledger = take();
    else if (a === '--assets') o.assets = take();
    else throw new Error(`unknown argument: ${a}`);
  }
  return o;
}

export function run(argv, out = console) {
  let o;
  try {
    o = parseArgs(argv);
  } catch (e) {
    out.error(`FAIL ${e.message}\nusage: node tools/gate/check-assets-ledger.mjs [--root <repo>] [--ledger ASSETS.md] [--assets <folder>]`);
    return 2;
  }
  const root = path.resolve(o.root || path.join(path.dirname(fileURLToPath(import.meta.url)), '..', '..'));
  const ledgerFile = path.join(root, o.ledger);
  let parsed;
  try {
    parsed = parseLedger(fs.readFileSync(ledgerFile, 'utf8'));
  } catch (e) {
    out.error(`FAIL ${path.basename(ledgerFile)}: ${e.message}`);
    return 2;
  }
  const failures = [...parsed.errors, ...validateRows(parsed.rows)];
  const warnings = [];

  const inLedger = new Set(parsed.rows.map((r) => key(r.path)));
  const files = listAssetFiles(root, o.assets);
  const missing = files.filter((f) => !inLedger.has(key(f)));
  for (const m of missing) failures.push(`no row in ASSETS.md for ${m}`);

  const onDisk = new Set(files.map(key));
  for (const r of parsed.rows) {
    if (!r.path) continue;
    const mb = diskMb(root, norm(r.path));
    if (mb === undefined) {
      if (key(r.path).startsWith(key(o.assets) + '/') && !onDisk.has(key(r.path))) warnings.push(`row for ${r.path} (line ${r.line}) has no file on disk`);
      continue;
    }
    const claimed = parseFloat(r.size_mb);
    if (Number.isFinite(claimed) && Math.abs(mb - claimed) > Math.max(0.2, mb * 0.25)) {
      warnings.push(`${r.path}: size_mb ${r.size_mb} but the file is ${mb.toFixed(1)} MB on disk`);
    }
  }

  const rep = budgetReport(parsed.rows);
  const over = [];
  for (const g of ['art', 'audio']) if (rep.total[g] > BUDGETS_MB[g]) over.push(g);
  for (const g of over) failures.push(`${g} budget exceeded: ${rep.total[g]} MB shipped, budget ${BUDGETS_MB[g]} MB`);
  if (rep.total.other > 40) warnings.push(`vfx, font and other total ${rep.total.other} MB; their slice (device A) is 40 MB`);

  for (const w of warnings) out.log(`WARN ${w}`);
  const line = (label, d) => `${label}: art ${d.art} MB, audio ${d.audio} MB, vfx/font/other ${d.other} MB`;
  out.log(`INFO shipped sizes, ${line('device A', rep.A)}`);
  out.log(`INFO shipped sizes, ${line('device B', rep.B)}`);
  out.log(`INFO shipped sizes, ${line('total', rep.total)} (budgets: art ${BUDGETS_MB.art}, audio ${BUDGETS_MB.audio})`);
  for (const f of failures) out.log(`FAIL ${f}`);
  const ok = failures.length === 0;
  if (ok) out.log(`PASS assets ledger: ${parsed.rows.length} row(s), ${files.length} file(s) under ${o.assets}, inside budget`);
  else out.log(`FAIL assets ledger: ${failures.length} problem(s)`);
  out.log(
    JSON.stringify({
      schema: 'thirdjam.gate/1',
      step: 'assets-ledger',
      ok,
      rows: parsed.rows.length,
      files: files.length,
      missing,
      shipped_mb: rep,
      budgets_mb: BUDGETS_MB,
      over_budget: over,
      failures: failures.length,
      warnings: warnings.length,
    }),
  );
  return ok ? 0 : 1;
}

if (process.argv[1] && path.resolve(process.argv[1]) === fileURLToPath(import.meta.url)) process.exitCode = run(process.argv.slice(2));
