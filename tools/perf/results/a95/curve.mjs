#!/usr/bin/env node
// Lane A95 (copy of A86 curve.mjs): one curve cell = one class x level x rink on A53's seeds (regular: 1-100 of each 150-band; large: 1..n), 8 headless JamTournament processes.
//   node curve.mjs <label> <class> <level> <regular|large> <n> [tune] [--stick rest] [--set "a.b=v"]
// Appends one JSON line to tools/perf/results/a95/cells.jsonl and the raw match lines to tools/perf/results/a95/raw/<label>.jsonl. Never kills a process it did not start.
import { spawn, execFileSync } from 'node:child_process';
import { appendFileSync, mkdirSync, writeFileSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { writeVariant } from '../../../playtest/run.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..', '..', '..');
const uproject = join(repo, 'Game', 'ThirdJam.uproject');
const exe = 'C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe';
const pos = process.argv.slice(2).filter((a, i, all) => !a.startsWith('--') && !(i > 0 && all[i - 1].startsWith('--')));
const opt = (n) => { const i = process.argv.indexOf(`--${n}`); return i > 0 ? process.argv[i + 1] : ''; };
const [label, cls, lvl, rink, nArg, tune = ''] = pos;
const n = Number(nArg);
const stick = opt('stick');
const set = opt('set');
const workers = Number(opt('workers') || 8);
let cart = rink === 'large' ? 'hockey.arena.large' : 'hockey.regular';
if (set) cart = writeVariant(cart, set);
// seed ranges (contiguous, 100 each at most)
const ranges = [];
if (rink === 'large') { for (let s = 1; s <= n; s += 100) ranges.push([s, Math.min(n, s + 99)]); }
else { for (let b = 0; b * 100 < n; b++) { const lo = b * 150 + 1; ranges.push([lo, lo + Math.min(100, n - b * 100) - 1]); } }
// split each range in halves so 8 workers stay busy
const jobs = [];
for (const [lo, hi] of ranges) { const mid = Math.floor((lo + hi) / 2); jobs.push([lo, mid], [mid + 1, hi]); }
function freeGB() { try { return Number(execFileSync('powershell', ['-NoProfile', '-Command', '(Get-CimInstance Win32_OperatingSystem).FreePhysicalMemory'], { encoding: 'utf8' }).trim()) / 1048576; } catch { return 99; } }
const lines = [];
const env = { ...process.env };
if (stick) env.JAM_PLAYTESTER_STICK = stick; else delete env.JAM_PLAYTESTER_STICK;
const verbs = opt('verbs');   // lane A95: JAM_PLAYTESTER_VERBS=old (job 3's before arm)
if (verbs) env.JAM_PLAYTESTER_VERBS = verbs; else delete env.JAM_PLAYTESTER_VERBS;
const brain = opt('brain');
if (brain) env.JAM_BRAIN_CARRIER = brain; else delete env.JAM_BRAIN_CARRIER;
function run([lo, hi]) {
  return new Promise((res) => {
    const args = [uproject, '-run=JamTournament', '-sim=jam', `-cartridge=${cart}`, `-difficulty=${lvl}`, `-player=${cls}`, `-seeds=${lo}-${hi}`, '-maxframes=36000',
      '-nullrhi', '-Multiprocess', '-unattended', '-nopause', '-nosplash', '-stdout', '-FullStdOutLogOutput'];
    if (tune) args.push(`-aitune=${tune}`);
    const p = spawn(exe, args, { windowsHide: true, env });
    let buf = '';
    p.stdout.on('data', (d) => { buf += d; });
    p.stderr.on('data', () => {});
    p.on('close', () => {
      for (const l of buf.split(/\r?\n/)) { const i = l.indexOf('{"tool":"thirdjam.playtest/1"'); if (i >= 0) { try { lines.push(JSON.parse(l.slice(i))); } catch {} } }
      if (/FAIL tournament/.test(buf)) console.error(buf.split(/\r?\n/).filter((l) => /FAIL/.test(l)).join('\n'));
      res();
    });
  });
}
const t0 = Date.now();
let next = 0;
async function worker() { while (next < jobs.length) { while (freeGB() < 12) await new Promise((r) => setTimeout(r, 15000)); const j = jobs[next++]; await run(j); } }
await Promise.all(Array.from({ length: workers }, worker));
// dedupe by seed
const by = new Map(); for (const m of lines) by.set(m.seed, m);
const ms = [...by.values()];
const N = ms.length, W = ms.filter((m) => m.result === 'win').length;
const wilson = (k, n, z = 1.96) => { if (!n) return [0, 0]; const p = k / n, d = 1 + z * z / n, c = (p + z * z / (2 * n)) / d, h = z * Math.sqrt(p * (1 - p) / n + z * z / (4 * n * n)) / d; return [100 * (c - h), 100 * (c + h)]; };
const [lo, hi] = wilson(W, N);
const sum = (k) => ms.reduce((a, m) => a + (Number(m[k]) || 0), 0);
const fo = sum('faceoffs'), fwo = sum('faceoffs_without_touch');
const row = { label, cls, lvl, rink, tune, stick: stick || 'aim', brain: brain || '', set, n: N, win: +(100 * W / N).toFixed(1), lo: +lo.toFixed(1), hi: +hi.toFixed(1),
  fwo_share: +(100 * fwo / fo).toFixed(2), fwo, faceoffs: fo, checks_taken: +(sum('checks_taken') / N).toFixed(2), knockdowns: +(sum('knockdowns') / N).toFixed(2),
  scored2: ms.filter((m) => m.first_goal_by_human >= 0 && m.first_goal_by_human <= 7200).length, shots: +(sum('shots') / N).toFixed(1),
  gf: +(ms.reduce((a, m) => a + (m.goals?.[0] ?? 0), 0) / N).toFixed(2), carry_pct: +(100 * sum('carry_frames') / Math.max(1, sum('play_frames'))).toFixed(1),
  touches_pm: +(sum('touches_per_min') / N).toFixed(1), verbs: verbs || 'new',
  rev_brakes: +(sum('rev_brakes') / N).toFixed(1), carry_sprint_s: +(sum('carry_sprint_frames') / N / 60).toFixed(1), burst_s: +(sum('burst_frames') / N / 60).toFixed(1), cuts: +(sum('cuts') / N).toFixed(1), shots_opp: +(sum('shots_opp') / N).toFixed(1),
  bot_sprint_s: +(sum('bot_sprint_frames') / N / 60).toFixed(1), bot_rev_brakes: +(sum('bot_rev_brakes') / N).toFixed(0), ga: +(ms.reduce((a, m) => a + (m.goals?.[1] ?? 0), 0) / N).toFixed(2), match_s: +(sum('frames') / N / 60).toFixed(0), unfinished: ms.filter((m) => m.result !== 'win' && m.result !== 'loss').length,
  secs: Math.round((Date.now() - t0) / 1000), at: new Date().toTimeString().slice(0, 5) };
mkdirSync(join(here, 'raw'), { recursive: true });
writeFileSync(join(here, 'raw', `${label}.jsonl`), ms.map((m) => JSON.stringify(m)).join('\n') + '\n');
appendFileSync(join(here, 'cells.jsonl'), JSON.stringify(row) + '\n');
console.log(JSON.stringify(row));
