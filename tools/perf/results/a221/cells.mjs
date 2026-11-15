#!/usr/bin/env node
// lane A221: several curve cells through the PERSON'S PATH (-run=JamHandCells) in ONE pool of processes (the brief's cap: four of mine at a time).
// A cell = "label|level|regular|large|n[|tune[|set]]"; seeds as a217/hand.mjs (regular: 1-100 of each 150-band; large: 1..n), so rows compare with A217's.
//   node cells.mjs --workers 4 --chunk 50 "P-pro-reg|pro|regular|300" "P-as-large|all_star|large|300|opp.shoot_extra_m=30" ...
// Writes raw/<label>.jsonl, appends one row a cell to cells.jsonl, each child's pid to out/pids.txt. Never kills a process.
import { spawn, execFileSync } from 'node:child_process';
import { appendFileSync, mkdirSync, writeFileSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { writeVariant } from '../../../playtest/run.mjs';
const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..', '..', '..');
const uproject = join(repo, 'Game', 'ThirdJam.uproject');
const exe = 'C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe';
const opt = (n, d = '') => { const i = process.argv.indexOf(`--${n}`); return i > 0 ? process.argv[i + 1] : d; };
const workers = Number(opt('workers', '4')), chunk = Number(opt('chunk', '50')), maxframes = opt('maxframes', '36000');
const specs = process.argv.slice(2).filter((a, i, all) => !a.startsWith('--') && !(i > 0 && all[i - 1].startsWith('--')));
const cells = specs.map((s) => { const [label, lvl, rink, n, tune = '', set = ''] = s.split('|'); let cart = rink === 'large' ? 'hockey.arena.large' : 'hockey.regular'; if (set) cart = writeVariant(cart, set); return { label, lvl, rink, n: Number(n), tune, set, cart, lines: [], t0: 0 }; });
const jobs = [];
for (const c of cells) {
  const ranges = [];
  if (c.rink !== 'regular') { for (let s = 1; s <= c.n; s += 100) ranges.push([s, Math.min(c.n, s + 99)]); }
  else { for (let b = 0; b * 100 < c.n; b++) { const lo = b * 150 + 1; ranges.push([lo, lo + Math.min(100, c.n - b * 100) - 1]); } }
  for (const [lo, hi] of ranges) for (let s = lo; s <= hi; s += chunk) jobs.push({ c, lo: s, hi: Math.min(hi, s + chunk - 1) });
}
function freeGB() { try { return Number(execFileSync('powershell', ['-NoProfile', '-Command', '(Get-CimInstance Win32_OperatingSystem).FreePhysicalMemory'], { encoding: 'utf8' }).trim()) / 1048576; } catch { return 99; } }
mkdirSync(join(here, 'out'), { recursive: true }); mkdirSync(join(here, 'raw'), { recursive: true });
function run({ c, lo, hi }) {
  return new Promise((res) => {
    const args = [uproject, '-run=JamHandCells', `-cartridge=${c.cart}`, `-difficulty=${c.lvl}`, `-seeds=${lo}-${hi}`, `-maxframes=${maxframes}`,
      '-buttons=1', '-hand=1', '-sprint=1', '-kind=-1', ...(c.tune ? [`-aitune=${c.tune}`] : []),
      '-LogCmds=LogJamScript Warning', '-nullrhi', '-unattended', '-nopause', '-nosplash', '-Multiprocess', '-stdout', '-FullStdOutLogOutput'];
    const env = { ...process.env };
    for (const k of Object.keys(env)) if (k.startsWith('JAM_')) delete env[k];
    const p = spawn(exe, args, { windowsHide: true, env });
    appendFileSync(join(here, 'out', 'pids.txt'), `${p.pid} ${c.label} ${lo}-${hi} ${new Date().toTimeString().slice(0, 5)}\n`);
    let buf = '';
    p.stdout.on('data', (d) => { buf += d; }); p.stderr.on('data', () => {});
    p.on('close', () => {
      for (const l of buf.split(/\r?\n/)) { const i = l.indexOf('{"tool":"thirdjam.hand/1"'); if (i >= 0) { try { c.lines.push(JSON.parse(l.slice(i))); } catch {} } }
      if (/FAIL hand/.test(buf)) console.error(buf.split(/\r?\n/).filter((l) => /FAIL/.test(l)).join('\n'));
      res();
    });
  });
}
const wilson = (k, n, z = 1.96) => { if (!n) return [0, 0]; const p = k / n, d = 1 + z * z / n, c = (p + z * z / (2 * n)) / d, h = z * Math.sqrt(p * (1 - p) / n + z * z / (4 * n * n)) / d; return [100 * (c - h), 100 * (c + h)]; };
export function summarize(label, lvl, rink, tune, set, linesIn) {
  const by = new Map(); for (const m of linesIn) by.set(m.seed, m);
  const ms = [...by.values()].sort((a, b) => a.seed - b.seed);
  const N = ms.length, W = ms.filter((m) => m.result === 'win').length; const [lo, hi] = wilson(W, N);
  const sum = (k) => ms.reduce((a, m) => a + (Number(m[k]) || 0), 0);
  const sumA = (k, i) => ms.reduce((a, m) => a + (m[k]?.[i] ?? 0), 0);
  const ga = ms.reduce((a, m) => a + m.goals[1], 0); const gd = (k) => ms.reduce((a, m) => a + (m.ga_dist?.[k] ?? 0), 0);
  const len = ms.map((m) => m.frames).sort((a, b) => a - b);
  const playMin = sum('play_frames') / 3600;
  return { label, lvl, rink, tune, set, n: N, win: N ? +(100 * W / N).toFixed(1) : 0, lo: +lo.toFixed(1), hi: +hi.toFixed(1), open: ms.filter((m) => m.result === 'open').length,
    gf: N ? +(ms.reduce((a, m) => a + m.goals[0], 0) / N).toFixed(2) : 0, ga: N ? +(ga / N).toFixed(2) : 0, ga_ge28_pct: ga ? +(100 * gd('ge28') / ga).toFixed(1) : 0,
    shots: N ? +(sum('shots') / N).toFixed(2) : 0, len_med_s: N ? +(len[Math.floor(N / 2)] / 60).toFixed(0) : 0, len_p80_s: N ? +(len[Math.floor(N * 0.8)] / 60).toFixed(0) : 0,
    pass_mate_pct: +(100 * sum('passes_mate') / Math.max(1, sum('passes'))).toFixed(1), kd_min: playMin ? +(sum('knockdowns') / playMin).toFixed(2) : 0,
    down_pct: +(100 * sum('down_frames') / Math.max(1, sum('play_frames'))).toFixed(1), ga600_matches: ms.filter((m) => m.ga_600 > 0).length, ga_own: sum('ga_own'),
    opp_shots: N ? +(sum('opp_shots') / N).toFixed(2) : 0, gain_shot: [0, 1, 2].map((i) => N ? +(sumA('gain_shot', i) / N).toFixed(2) : 0), gain_shot_ring: [0, 1, 2].map((i) => N ? +(sumA('gain_shot_ring', i) / N).toFixed(2) : 0),
    gain_opp_carry: N ? +(sum('gain_opp_carry') / N).toFixed(2) : 0, gain_opp_loose: N ? +(sum('gain_opp_loose') / N).toFixed(2) : 0, gain_opp_loose_ring: N ? +(sum('gain_opp_loose_ring') / N).toFixed(2) : 0, gain_mate: N ? +(sum('gain_mate') / N).toFixed(2) : 0,
    at: new Date().toTimeString().slice(0, 5), ms };
}
const t0 = Date.now(); let next = 0;
async function worker() { while (next < jobs.length) { while (freeGB() < 13) await new Promise((r) => setTimeout(r, 15000)); if (next >= jobs.length) break; const j = jobs[next++]; await run(j); } }
await Promise.all(Array.from({ length: workers }, worker));
for (const c of cells) {
  const { ms, ...row } = summarize(c.label, c.lvl, c.rink, c.tune, c.set, c.lines); row.secs = Math.round((Date.now() - t0) / 1000);
  writeFileSync(join(here, 'raw', `${c.label}.jsonl`), ms.map((m) => JSON.stringify(m)).join('\n') + '\n');
  appendFileSync(join(here, 'cells.jsonl'), JSON.stringify(row) + '\n');
  console.log(JSON.stringify(row));
}
