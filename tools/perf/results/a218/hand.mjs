#!/usr/bin/env node
// lane A217: one curve cell through the PERSON'S PATH (-run=JamHandCells: the game mode's match, the human played by jam.ScriptHuman 2 through the button
// resolver). The seeds are curve.mjs's (regular: 1-100 of each 150-band; any other rink: 1..n), so a cell by both instruments plays the same sim seeds.
//   node hand.mjs <label> <level> <regular|large|cartridge> <n> [--buttons 0|1] [--hand 0|1] [--sprint 0|1] [--kind -1|0|1|2] [--simseed N] [--seeds lo-hi]
//        [--tune "k=v;..."] [--workers 2] [--cart <file>] [--set "a.b=v"] [--maxframes 36000] [--hashes] [--passlog]
// lane A218: copy of a217/hand.mjs (writes beside itself: a218/cells.jsonl, a218/raw/<label>.jsonl, a218/out/pids.txt). Never kills a process.
import { spawn, execFileSync } from 'node:child_process';
import { appendFileSync, mkdirSync, writeFileSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { writeVariant } from '../../../playtest/run.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..', '..', '..');
const uproject = join(repo, 'Game', 'ThirdJam.uproject');
const exe = 'C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe';
const pos = process.argv.slice(2).filter((a, i, all) => !a.startsWith('--') && !(i > 0 && all[i - 1].startsWith('--') && all[i - 1] !== '--hashes' && all[i - 1] !== '--passlog'));
const opt = (n, d = '') => { const i = process.argv.indexOf(`--${n}`); return i > 0 ? process.argv[i + 1] : d; };
const [label, lvl, rink, nArg] = pos;
const n = Number(nArg);
const workers = Number(opt('workers', '2'));
let cart = rink === 'large' ? 'hockey.arena.large' : rink === 'regular' ? 'hockey.regular' : rink;
if (opt('set')) cart = writeVariant(cart, opt('set'));
if (opt('cart')) cart = opt('cart');
const ranges = [];
if (opt('seeds')) { const [a, b] = opt('seeds').split('-').map(Number); ranges.push([a, b]); }
else if (rink !== 'regular') { for (let s = 1; s <= n; s += 100) ranges.push([s, Math.min(n, s + 99)]); }
else { for (let b = 0; b * 100 < n; b++) { const lo = b * 150 + 1; ranges.push([lo, lo + Math.min(100, n - b * 100) - 1]); } }
const jobs = [];
for (const [lo, hi] of ranges) { if (hi - lo < 4) { jobs.push([lo, hi]); continue; } const mid = Math.floor((lo + hi) / 2); jobs.push([lo, mid], [mid + 1, hi]); }
function freeGB() { try { return Number(execFileSync('powershell', ['-NoProfile', '-Command', '(Get-CimInstance Win32_OperatingSystem).FreePhysicalMemory'], { encoding: 'utf8' }).trim()) / 1048576; } catch { return 99; } }
const lines = [], hashes = [], fates = [];
const sw = { buttons: opt('buttons', '1'), hand: opt('hand', '1'), sprint: opt('sprint', '1'), kind: opt('kind', '-1') };
function run([lo, hi]) {
  return new Promise((res) => {
    const args = [uproject, '-run=JamHandCells', `-cartridge=${cart}`, `-difficulty=${lvl}`, `-seeds=${lo}-${hi}`, `-maxframes=${opt('maxframes', '36000')}`,
      `-buttons=${sw.buttons}`, `-hand=${sw.hand}`, `-sprint=${sw.sprint}`, `-kind=${sw.kind}`, ...(opt('simseed') ? [`-simseed=${opt('simseed')}`] : []), ...(opt('tune') ? [`-aitune=${opt('tune')}`] : []),
      ...(process.argv.includes('--hashes') ? ['-hashes'] : []), ...(process.argv.includes('--passlog') ? ['-passlog'] : []),
      '-LogCmds=LogJamScript Warning', '-nullrhi', '-unattended', '-nopause', '-nosplash', '-Multiprocess', '-stdout', '-FullStdOutLogOutput'];
    const env = { ...process.env };
    for (const k of Object.keys(env)) if (k.startsWith('JAM_')) delete env[k];   // the person's path as shipped: no measurement arm
    const p = spawn(exe, args, { windowsHide: true, env });
    try { mkdirSync(join(here, 'out'), { recursive: true }); appendFileSync(join(here, 'out', 'pids.txt'), `${p.pid} ${label} ${lo}-${hi} ${new Date().toTimeString().slice(0, 5)}\n`); } catch {}
    let buf = '';
    p.stdout.on('data', (d) => { buf += d; });
    p.stderr.on('data', () => {});
    p.on('close', () => {
      for (const l of buf.split(/\r?\n/)) {
        const i = l.indexOf('{"tool":"thirdjam.hand/1"'); if (i >= 0) { try { lines.push(JSON.parse(l.slice(i))); } catch {} }
        const h = l.indexOf('JAMHAND seed='); if (h >= 0) hashes.push(l.slice(h));
        const f = l.indexOf('JAMPASSFATE '); if (f >= 0) fates.push(l.slice(f));
      }
      if (/FAIL hand/.test(buf)) console.error(buf.split(/\r?\n/).filter((l) => /FAIL/.test(l)).join('\n'));
      res();
    });
  });
}
const t0 = Date.now();
let next = 0;
async function worker() { while (next < jobs.length) { while (freeGB() < 14) await new Promise((r) => setTimeout(r, 15000)); if (next >= jobs.length) break; const j = jobs[next++]; await run(j); } }
await Promise.all(Array.from({ length: workers }, worker));
const by = new Map(); for (const m of lines) by.set(m.seed, m);
const ms = [...by.values()].sort((a, b) => a.seed - b.seed);
const N = ms.length, W = ms.filter((m) => m.result === 'win').length;
const wilson = (k, n, z = 1.96) => { if (!n) return [0, 0]; const p = k / n, d = 1 + z * z / n, c = (p + z * z / (2 * n)) / d, h = z * Math.sqrt(p * (1 - p) / n + z * z / (4 * n * n)) / d; return [100 * (c - h), 100 * (c + h)]; };
const [lo, hi] = wilson(W, N);
const sum = (k) => ms.reduce((a, m) => a + (Number(m[k]) || 0), 0);
const ga = ms.reduce((a, m) => a + m.goals[1], 0);
const gd = (k) => ms.reduce((a, m) => a + (m.ga_dist?.[k] ?? 0), 0);
const fga = ms.filter((m) => m.first_ga >= 0).map((m) => m.first_ga).sort((a, b) => a - b);
const row = { label, instrument: 'buttons', lvl, rink, ...sw, simseed: opt('simseed'), tune: opt('tune'), set: opt('set'), n: N, win: N ? +(100 * W / N).toFixed(1) : 0, lo: +lo.toFixed(1), hi: +hi.toFixed(1),
  open: ms.filter((m) => m.result === 'open').length, gf: N ? +(ms.reduce((a, m) => a + m.goals[0], 0) / N).toFixed(2) : 0, ga: N ? +(ga / N).toFixed(2) : 0,
  ga_dist_pct: Object.fromEntries(['lt10', '10_20', '20_28', 'ge28', 'none'].map((k) => [k, ga ? +(100 * gd(k) / ga).toFixed(1) : 0])), ga_own: sum('ga_own'),
  first_ga_median_s: fga.length ? +(fga[Math.floor(fga.length / 2)] / 60).toFixed(1) : -1, ga600_matches: ms.filter((m) => m.ga_600 > 0).length,
  down_pct: +(100 * sum('down_frames') / Math.max(1, sum('play_frames'))).toFixed(1), stumble_pct: +(100 * sum('stumble_frames') / Math.max(1, sum('play_frames'))).toFixed(1),
  knockdowns: N ? +(sum('knockdowns') / N).toFixed(2) : 0, shots: N ? +(sum('shots') / N).toFixed(1) : 0,
  passes: N ? +(sum('passes') / N).toFixed(1) : 0, pass_mate_pct: +(100 * sum('passes_mate') / Math.max(1, sum('passes'))).toFixed(1), pass_opp_pct: +(100 * sum('passes_opp') / Math.max(1, sum('passes'))).toFixed(1),
  carry_pct: +(100 * sum('carry_frames') / Math.max(1, sum('play_frames'))).toFixed(1), match_s: N ? +(sum('frames') / N / 60).toFixed(0) : 0,
  secs: Math.round((Date.now() - t0) / 1000), at: new Date().toTimeString().slice(0, 5) };
mkdirSync(join(here, 'raw'), { recursive: true });
writeFileSync(join(here, 'raw', `${label}.jsonl`), ms.map((m) => JSON.stringify(m)).join('\n') + '\n');
if (fates.length) writeFileSync(join(here, 'raw', `${label}.passes.txt`), fates.join(String.fromCharCode(10)) + String.fromCharCode(10));
if (hashes.length) writeFileSync(join(here, 'raw', `${label}.hashes.txt`), hashes.join('\n') + '\n');
appendFileSync(join(here, 'cells.jsonl'), JSON.stringify(row) + '\n');
console.log(JSON.stringify(row));
