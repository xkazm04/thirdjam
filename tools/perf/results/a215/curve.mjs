#!/usr/bin/env node
// lane A209: copy of a202/curve.mjs plus --hand (env JAM_PLAYTESTER_HAND: person = the person-like hand of JamHitHand.h) and --seam (env JAM_TEST_A151); rows in a215/cells.jsonl.
// lane A202: copy of a193/curve.mjs unchanged (rows in a202/cells.jsonl).
// lane A193: copy of a185/curve.mjs (rows in a193/cells.jsonl) plus the bots' landed checks and cross-checks in the row.
// lane A185: copy of a182/curve.mjs (rows in a185/cells.jsonl) plus each child's pid in out/pids.txt, and the human's time down, passes and carries in the row.
// lane A182: copy of a170/curve.mjs (rows in a182/cells.jsonl).
// lane A170: copy of a162/curve.mjs unchanged (rows in a170/cells.jsonl).
// Lane A162: copy of A154 curve.mjs plus --check / --poke (the scripted human's press and poke arms, env JAM_PLAYTESTER_CHECK / JAM_PLAYTESTER_POKE). Lane A154: copy of lane A133 curve.mjs (rows in a154/cells.jsonl). Lane A112 (copy of A102 curve.mjs, which is A95's, plus -Multiprocess: the editor start does not wait on Build.bat's lock, status/A102.md): one curve cell = one class x level x rink on A53's seeds (regular: 1-100 of each 150-band; large: 1..n), 8 headless JamTournament processes.
//   node curve.mjs <label> <class> <level> <regular|large> <n> [tune] [--stick rest] [--set "a.b=v"]
// Appends one JSON line to tools/perf/results/a112/cells.jsonl and the raw match lines to tools/perf/results/a112/raw/<label>.jsonl. Never kills a process it did not start.
import { spawn, execFileSync } from 'node:child_process';
import { appendFileSync, mkdirSync, writeFileSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { writeVariant } from '../../../playtest/run.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..', '..', '..');
const uproject = process.env.JAM_UPROJECT || join(repo, 'Game', 'ThirdJam.uproject');   // lane A112: JAM_UPROJECT = another project dir's binaries (raw/shadow: main's build while ours queued)
const exe = 'C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe';
const pos = process.argv.slice(2).filter((a, i, all) => !a.startsWith('--') && !(i > 0 && all[i - 1].startsWith('--')));
const opt = (n) => { const i = process.argv.indexOf(`--${n}`); return i > 0 ? process.argv[i + 1] : ''; };
const [label, cls, lvl, rink, nArg, tune = ''] = pos;
const n = Number(nArg);
const stick = opt('stick');
const set = opt('set');
const workers = Number(opt('workers') || 6);
// lane A112: any other rink word is a cartridge name (e.g. ladder/take-3), played on seeds 1..n; level 'cart' = the cartridge's own level (no -difficulty)
let cart = rink === 'large' ? 'hockey.arena.large' : rink === 'regular' ? 'hockey.regular' : rink;
if (set) cart = writeVariant(cart, set);
if (opt('cart')) cart = opt('cart');   // lane A182: a variant cartridge file written beforehand (a --set name past Windows' path limit); the rink word still picks the seeds
// seed ranges (contiguous, 100 each at most)
const ranges = [];
if (rink !== 'regular') { for (let s = 1; s <= n; s += 100) ranges.push([s, Math.min(n, s + 99)]); }
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
const check = opt('check');   // lane A162: closing (the ladder's instrument, R-0060) | course | old
if (check) env.JAM_PLAYTESTER_CHECK = check; else delete env.JAM_PLAYTESTER_CHECK;
const poke = opt('poke');   // lane A162: absent = the config as it ships (read off)
if (poke) env.JAM_PLAYTESTER_POKE = poke; else delete env.JAM_PLAYTESTER_POKE;
const hand = opt('hand');   // lane A209
if (hand) env.JAM_PLAYTESTER_HAND = hand; else delete env.JAM_PLAYTESTER_HAND;
const seam = opt('seam');   // lane A209: the measurement seam JAM_TEST_A151
if (seam) env.JAM_TEST_A151 = seam; else delete env.JAM_TEST_A151;
const brain = opt('brain');
if (brain) env.JAM_BRAIN_CARRIER = brain; else delete env.JAM_BRAIN_CARRIER;
function run([lo, hi]) {
  return new Promise((res) => {
    const args = [uproject, '-run=JamTournament', '-sim=jam', `-cartridge=${cart}`, ...(lvl === 'cart' ? [] : [`-difficulty=${lvl}`]), `-player=${cls}`, `-seeds=${lo}-${hi}`, '-maxframes=36000',
      '-nullrhi', '-unattended', '-nopause', '-nosplash', '-Multiprocess', '-stdout', '-FullStdOutLogOutput'];
    if (tune) args.push(`-aitune=${tune}`);
    const p = spawn(exe, args, { windowsHide: true, env });
    try { mkdirSync(join(here, 'out'), { recursive: true }); appendFileSync(join(here, 'out', 'pids.txt'), `${p.pid} ${label} ${lo}-${hi}
`); } catch {}
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
async function worker() { while (next < jobs.length) { while (freeGB() < 14) await new Promise((r) => setTimeout(r, 15000)); if (next >= jobs.length) break; const j = jobs[next++]; await run(j); } }   // lane A202: the last job taken while this worker waited on memory (a crash, FA-pro-large)
await Promise.all(Array.from({ length: workers }, worker));
// dedupe by seed
const by = new Map(); for (const m of lines) by.set(m.seed, m);
const ms = [...by.values()];
const N = ms.length, W = ms.filter((m) => m.result === 'win').length;
const wilson = (k, n, z = 1.96) => { if (!n) return [0, 0]; const p = k / n, d = 1 + z * z / n, c = (p + z * z / (2 * n)) / d, h = z * Math.sqrt(p * (1 - p) / n + z * z / (4 * n * n)) / d; return [100 * (c - h), 100 * (c + h)]; };
const [lo, hi] = wilson(W, N);
const sum = (k) => ms.reduce((a, m) => a + (Number(m[k]) || 0), 0);
const fo = sum('faceoffs'), fwo = sum('faceoffs_without_touch');
const row = { label, cls, lvl, rink, tune, check: check || 'config', poke: poke || 'config', stick: stick || 'aim', brain: brain || '', hand: hand || '', seam: seam || '', set, n: N, win: +(100 * W / N).toFixed(1), lo: +lo.toFixed(1), hi: +hi.toFixed(1),
  fwo_share: +(100 * fwo / fo).toFixed(2), fwo, faceoffs: fo, checks_taken: +(sum('checks_taken') / N).toFixed(2), knockdowns: +(sum('knockdowns') / N).toFixed(2),
  scored2: ms.filter((m) => m.first_goal_by_human >= 0 && m.first_goal_by_human <= 7200).length, shots: +(sum('shots') / N).toFixed(1),
  gf: +(ms.reduce((a, m) => a + (m.goals?.[0] ?? 0), 0) / N).toFixed(2), carry_pct: +(100 * sum('carry_frames') / Math.max(1, sum('play_frames'))).toFixed(1),
  noshot: ms.filter((m) => !(Number(m.shots) > 0)).length, fouls: +(sum('fouls') / N).toFixed(2), touches_pm: +(sum('touches_per_min') / N).toFixed(1), verbs: verbs || 'new',
  rev_brakes: +(sum('rev_brakes') / N).toFixed(1), carry_sprint_s: +(sum('carry_sprint_frames') / N / 60).toFixed(1), burst_s: +(sum('burst_frames') / N / 60).toFixed(1), cuts: +(sum('cuts') / N).toFixed(1), shots_opp: +(sum('shots_opp') / N).toFixed(1),
  bot_sprint_s: +(sum('bot_sprint_frames') / N / 60).toFixed(1), bot_rev_brakes: +(sum('bot_rev_brakes') / N).toFixed(0), ga: +(ms.reduce((a, m) => a + (m.goals?.[1] ?? 0), 0) / N).toFixed(2), match_s: +(sum('frames') / N / 60).toFixed(0), unfinished: ms.filter((m) => m.result !== 'win' && m.result !== 'loss').length,
  carries_3s: +(sum('carries_3s') / N).toFixed(2), carry_s: +(sum('carry_frames') / N / 60).toFixed(1), downed_pct: +(100 * sum('downed_frames') / Math.max(1, sum('play_frames'))).toFixed(1),
  passes: +(sum('passes') / N).toFixed(1), passes_received: +(sum('passes_received') / N).toFixed(1), checks_landed: +(sum('checks_landed') / N).toFixed(2), bot_checks_landed: +(sum('bot_checks_landed') / N).toFixed(2), bot_cross_landed: +(sum('bot_cross_landed') / N).toFixed(2), hashes: N, knockdowns_all: +(sum('knockdowns_all') / N).toFixed(2),
  secs: Math.round((Date.now() - t0) / 1000), at: new Date().toTimeString().slice(0, 5) };
mkdirSync(join(here, 'raw'), { recursive: true });
writeFileSync(join(here, 'raw', `${label}.jsonl`), ms.map((m) => JSON.stringify(m)).join('\n') + '\n');
appendFileSync(join(here, 'cells.jsonl'), JSON.stringify(row) + '\n');
console.log(JSON.stringify(row));
