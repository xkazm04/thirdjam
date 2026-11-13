#!/usr/bin/env node
// Lane A116 (track S1 soak, evolution 2): the drift gate for modes and takes. Documented in docs/experiments/SOAK-DRIFT.md.
//   node tools/soak/drift.mjs --uproject <abs .uproject of a BUILT checkout> [--out tools/soak/out/drift-<stamp>] [--seeds 100] [--max 16] [--only modes|takes]
//                             [--baseline tools/soak/baseline.json] [--threshold 10] [--update-baseline] [--from <out dir of an earlier run>]
// It plays the twelve modes (regular class, at the mode's own level; Marathon with -playersprint and --max-frames 42000, the cap its ten minutes need) and the eight season takes
// (novice, regular and good class, at the take's level) through the shared driver (cells.mjs), reads each mode's character number (tools/playtest/modes.mjs CHARACTER) and
// each take's win rate per class, and compares them with tools/soak/baseline.json. A number that moved more than --threshold percent (and by more than a floor that keeps a
// single match from tripping it: 2 win points, 0.02 of a character number) is printed and the exit is 1. The sim is deterministic: a cell whose per-seed hashes equal the
// baseline's is bit-identical and cannot have moved. Exit: 0 nothing moved, 1 a number moved or a fault, 2 could not run. Last line: {"tool":"thirdjam.drift/1",...}.
import { readFileSync, writeFileSync, existsSync, mkdirSync, readdirSync } from 'node:fs';
import { createHash } from 'node:crypto';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { run as runSoak } from './cells.mjs';
import { CHARACTER } from '../playtest/modes.mjs';

const here = dirname(fileURLToPath(import.meta.url));
export const CLASSES = ['novice', 'regular', 'good'];
export const MARATHON_CAP = 42000;

// The plan: derived from the cartridges of the build that is played, so a new mode or take joins the gate (and shows as "new" until the baseline is updated).
export function buildPlan(cartDir, seeds) {
  const plan = [];
  const levelOf = (f) => JSON.parse(readFileSync(f, 'utf8')).game?.ai?.difficulty || 'pro';
  const modesDir = join(cartDir, 'modes');
  if (existsSync(modesDir)) for (const f of readdirSync(modesDir).filter((x) => x.endsWith('.cartridge.json')).sort()) {
    const slug = f.replace(/^hockey\.mode\./, '').replace('.cartridge.json', ''); const cart = `modes/${f.replace('.cartridge.json', '')}`;
    plan.push({ label: `m-${slug}`, cls: 'regular', lvl: levelOf(join(modesDir, f)), cart, seeds: `1-${seeds}`,
      ...(slug === 'marathon' ? { maxframes: MARATHON_CAP, extra: ['-playersprint'] } : {}) });
  }
  const ladder = join(cartDir, 'ladder');
  if (existsSync(ladder)) for (const f of readdirSync(ladder).filter((x) => /^take-\d+\.cartridge\.json$/.test(x)).sort()) {
    const take = f.replace('.cartridge.json', ''); const lvl = levelOf(join(ladder, f));
    for (const cls of CLASSES) plan.push({ label: `t-${take}-${cls}`, cls, lvl, cart: `ladder/${take}`, seeds: `1-${seeds}` });
  }
  return plan;
}

const r2 = (x) => Math.round(x * 100) / 100;
const winPct = (ms) => (ms.length ? r2(100 * ms.filter((m) => m.result === 'win').length / ms.length) : 0);
const digest = (ms) => createHash('sha1').update(ms.map((m) => `${m.seed}:${m.hash}`).join(',')).digest('hex').slice(0, 16);
const avg = (a) => (a.length ? a.reduce((s, x) => s + x, 0) / a.length : 0);

// Reads one cell's matches into the numbers the gate keeps.
export function measure(label, ms) {
  const cell = { n: ms.length, win: winPct(ms), unfinished: ms.filter((m) => !['win', 'loss', 'draw'].includes(m.result)).length, frames: Math.round(avg(ms.map((m) => m.frames))), digest: digest(ms),
    // read-outs the tracks ask for (kept, not gated): the human's knockdowns per match, idle seconds per match, share of matches with a 30 s stretch without a human touch
    kd: r2(avg(ms.map((m) => m.knockdowns || 0))), idle_s: r2(avg(ms.map((m) => (m.idle_frames || 0) / 60))), no_touch30_pct: ms.length ? r2(100 * ms.filter((m) => (m.max_no_touch || 0) >= 1800).length / ms.length) : 0 };
  if (label.startsWith('m-')) {
    const slug = label.slice(2); const ch = CHARACTER[slug];
    if (ch) { cell.character = ch.label; cell.value = r2(ch.of(ms)); }
  }
  return cell;
}
export function measureDir(dir, plan) {
  const cells = {};
  for (const c of plan) {
    const f = join(dir, `${c.label}.jsonl`);
    if (!existsSync(f)) continue;
    const ms = readFileSync(f, 'utf8').split('\n').filter(Boolean).map((l) => JSON.parse(l));
    cells[c.label] = { ...measure(c.label, ms), level: c.lvl, class: c.cls };
  }
  return cells;
}

// The metrics of a cell: mode -> character number and win rate; take -> win rate.
function metrics(label, cell) {
  const m = [];
  if (label.startsWith('m-')) {
    if (cell.value !== undefined) m.push({ key: `${label} ${cell.character}`, v: cell.value, floor: 0.02, unit: '' });
    m.push({ key: `${label} win %`, v: cell.win, floor: 2, unit: ' pts', win: true, n: cell.n });
  } else m.push({ key: `${label} win %`, v: cell.win, floor: 2, unit: ' pts', win: true, n: cell.n });
  return m;
}
// base and now are {label: cell}. Returns {moved: [...], identical, compared, added, removed}.
export function compare(base, now, threshold = 10) {
  const moved = [], added = [], removed = []; let identical = 0, compared = 0;
  for (const label of Object.keys(now)) {
    if (!base[label]) { added.push(label); continue; }
    compared++;
    if (base[label].digest === now[label].digest && base[label].n === now[label].n) { identical++; continue; }
    const bm = new Map(metrics(label, base[label]).map((x) => [x.key, x]));
    for (const x of metrics(label, now[label])) {
      const b = bm.get(x.key); if (!b) continue;
      const abs = Math.abs(x.v - b.v); const rel = 100 * abs / Math.max(Math.abs(b.v), 1e-9);
      if (rel > threshold && abs > x.floor) {
        const p = b.v / 100; const sigma = x.win && x.n ? 100 * Math.sqrt(Math.max(p * (1 - p), 1e-4) / x.n) : undefined;
        moved.push({ key: x.key, base: r2(b.v), now: r2(x.v), delta: r2(x.v - b.v), pct: r2(100 * (x.v - b.v) / Math.max(Math.abs(b.v), 1e-9)), ...(sigma ? { sigmas: r2(abs / sigma) } : {}) });
      }
    }
    if (base[label].unfinished !== now[label].unfinished) moved.push({ key: `${label} unfinished`, base: base[label].unfinished, now: now[label].unfinished, delta: now[label].unfinished - base[label].unfinished, pct: null });
  }
  for (const label of Object.keys(base)) if (!now[label]) removed.push(label);
  return { moved, identical, compared, added, removed };
}

export async function main(argv) {
  const opt = (n, d) => { const i = argv.indexOf(`--${n}`); return i >= 0 ? argv[i + 1] : d; };
  const has = (n) => argv.includes(`--${n}`);
  const die = (msg) => { console.log(`FAIL drift ${msg}`); console.log(JSON.stringify({ tool: 'thirdjam.drift/1', verdict: 'could_not_run', exit: 2, error: msg })); return 2; };
  const uproject = opt('uproject'); const from = opt('from');
  if (!uproject) return die('--uproject <built checkout .uproject> is required (its cartridges define the plan)');
  const cartDir = join(dirname(resolve(uproject)), 'Content', 'Cartridges');
  const baselineFile = resolve(opt('baseline', join(here, 'baseline.json')));
  const baseline = existsSync(baselineFile) ? JSON.parse(readFileSync(baselineFile, 'utf8')) : null;
  const seeds = +opt('seeds', baseline?.seeds || 100); const threshold = +opt('threshold', 10); const only = opt('only');
  let plan = buildPlan(cartDir, seeds);
  if (!plan.length) return die(`no cartridges under ${cartDir}`);
  if (only) plan = plan.filter((c) => c.label.startsWith(only === 'modes' ? 'm-' : 't-'));
  const stamp = new Date().toISOString().replace(/[-:]/g, '').replace(/\..+/, '').replace('T', '-');
  const out = resolve(from || opt('out', join(here, 'out', `drift-${stamp}`)));
  let soakCode = 0, faults = 0;
  if (!from) {
    mkdirSync(out, { recursive: true }); const pf = join(out, 'plan.json'); writeFileSync(pf, JSON.stringify(plan, null, 1));
    const lines = []; const log = console.log; console.log = (...x) => { lines.push(x.join(' ')); log(...x); };
    try { soakCode = await runSoak([pf, '--uproject', resolve(uproject), '--out', out, '--max', opt('max', '16'), '--chunk', opt('chunk', '50')]); } finally { console.log = log; }
    if (soakCode === 2) return 2;
    const last = lines.filter((l) => l.startsWith('{"tool":"thirdjam.soak/1"')).pop(); faults = last ? JSON.parse(last).faults : 0;
  }
  const now = measureDir(out, plan);
  if (has('update-baseline')) {
    if (faults) return die(`refusing to write a baseline from a run with ${faults} faults (see ${join(out, 'faults.jsonl')})`);
    const prev = baseline?.cells || {}; const cells = only ? { ...prev, ...now } : now;
    const doc = { schema: 'thirdjam.soak.baseline/1', taken: new Date().toISOString().slice(0, 16), build: opt('build', 'unnamed (pass --build "<commit> <what>")'), seeds, marathon_cap: MARATHON_CAP, cells };
    writeFileSync(baselineFile, JSON.stringify(doc, null, 1) + '\n');
    console.log(`PASS drift baseline written: ${Object.keys(cells).length} cells, ${seeds} seeds each -> ${baselineFile}`);
    console.log(JSON.stringify({ tool: 'thirdjam.drift/1', verdict: 'baseline_written', exit: 0, cells: Object.keys(cells).length, seeds }));
    return 0;
  }
  if (!baseline) return die(`no baseline at ${baselineFile}: run once with --update-baseline`);
  if (baseline.seeds !== seeds) return die(`baseline has ${baseline.seeds} seeds per cell, this run ${seeds}: seeds must match (the digests compare seed by seed)`);
  const r = compare(baseline.cells, now, threshold);
  for (const m of r.moved) console.log(`MOVED ${m.key}: ${m.base} -> ${m.now} (${m.delta >= 0 ? '+' : ''}${m.delta}${m.pct === null ? '' : `, ${m.pct >= 0 ? '+' : ''}${m.pct} %`}${m.sigmas !== undefined ? `, ${m.sigmas} sigma of sampling noise` : ''})`);
  for (const l of r.added) console.log(`NEW cell ${l}: not in the baseline (update it to keep this one)`);
  for (const l of r.removed) console.log(`GONE cell ${l}: in the baseline, not played`);
  const bad = r.moved.length > 0 || faults > 0 || (!only && r.removed.length > 0);
  console.log(`${bad ? 'FAIL' : 'PASS'} drift ${r.compared} cells compared, ${r.identical} bit-identical, ${r.moved.length} numbers moved more than ${threshold} %, ${faults} faults, ${r.added.length} new`);
  console.log(JSON.stringify({ tool: 'thirdjam.drift/1', verdict: bad ? 'fail' : 'pass', exit: bad ? 1 : 0, compared: r.compared, identical: r.identical, moved: r.moved, added: r.added, removed: r.removed, faults, out }));
  return bad ? 1 : 0;
}

if (process.argv[1] && import.meta.url === pathToFileURL(resolve(process.argv[1])).href) process.exit(await main(process.argv.slice(2)));
