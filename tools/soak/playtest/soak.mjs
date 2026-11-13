#!/usr/bin/env node
// Playtester soak (lane A53): fans UnrealEditor-Cmd JamTournament -player= runs over a plan {class, level, seed} with N workers, resumable.
//
//   node tools/soak/playtest/soak.mjs [--workers 8] [--chunk 50] [--hours 5 | --until 15:00] [--round 150] [--rounds 0] [--out tools/soak/playtest/out]
//        [--classes novice,regular,good] [--levels rookie,pro,all_star] [--uproject <path>] [--minfree 12] [--lowworkers 4] [--max-frames 36000]
//
// One match = one seed on one cartridge: hockey.regular, or hockey.boost for the last third of each band of 150 seeds (1-100 regular, 101-150 boost) (so a seed range stays contiguous for
// the commandlet's -seeds=lo-hi). A round is --round seeds for all 9 cells; round k covers seeds (k-1)*round+1 .. k*round. --rounds 0 = while time remains.
// A match is deterministic (same cartridge, level, class, seed = same hash): re-running a seed adds nothing, more seeds do.
// Results: <out>/results/<cart>/<class>-<level>-<seed>.json (the parsed thirdjam.playtest/1 line + faults + chunk log name); logs: <out>/logs/*.log.
// A crashed chunk: the first seed with no line is the crash seed (a result with crash:{exit,head}); the seeds after it are re-queued.
// Never kills a process it did not start. Exit 0.
import { existsSync, mkdirSync, writeFileSync } from 'node:fs';
import { spawn, execFileSync } from 'node:child_process';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import os from 'node:os';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..', '..');
function arg(name, dflt) { const i = process.argv.indexOf(`--${name}`); return i > 0 && i + 1 < process.argv.length ? process.argv[i + 1] : dflt; }
const workers = Number(arg('workers', '8'));
const lowWorkers = Number(arg('lowworkers', '4'));
const minFreeGB = Number(arg('minfree', '12'));
const chunk = Number(arg('chunk', '50'));
const roundSize = Number(arg('round', '150'));
const maxRounds = Number(arg('rounds', '0'));
const maxFrames = Number(arg('max-frames', '36000'));
const classes = arg('classes', 'novice,regular,good').split(',');
const levels = arg('levels', 'rookie,pro,all_star').split(',');
const out = resolve(arg('out', join(here, 'out')));
const uproject = arg('uproject', join(repo, 'Game', 'ThirdJam.uproject'));
const hours = arg('hours', '');
const until = arg('until', '');
let deadline = Infinity;
if (hours) deadline = Date.now() + Number(hours) * 3600e3;
if (until) {
  const [h, m] = until.split(':').map(Number);
  const d = new Date(); d.setHours(h, m, 0, 0);
  if (d.getTime() < Date.now()) d.setDate(d.getDate() + 1);
  deadline = Math.min(deadline, d.getTime());
}

export const cartFor = (seed) => (((seed - 1) % 150) >= 100 ? 'hockey.boost' : 'hockey.regular');
const resFile = (cart, cls, lvl, seed) => join(out, 'results', cart, `${cls}-${lvl}-${seed}.json`);
const done = (cart, cls, lvl, seed) => existsSync(resFile(cart, cls, lvl, seed));

function engineRoot() {
  for (const c of [process.env.UE_ROOT, 'C:/Program Files/Epic Games/UE_5.9']) {
    if (c && existsSync(join(c, 'Engine', 'Binaries', 'Win64', 'UnrealEditor-Cmd.exe'))) return c;
  }
  return null;
}
function freeGB() {
  try {
    const s = execFileSync('powershell', ['-NoProfile', '-Command', '(Get-CimInstance Win32_OperatingSystem).FreePhysicalMemory'], { encoding: 'utf8' });
    return Number(s.trim()) / 1048576;
  } catch { return os.freemem() / 2 ** 30; }
}

// The plan for one round, split into chunks of <= --chunk contiguous seeds of one cartridge, skipping finished seeds.
function planRound(r) {
  const lo = (r - 1) * roundSize + 1, hi = r * roundSize;
  const list = [];
  for (const lvl of levels) for (const cls of classes) {
    let s = lo;
    while (s <= hi) {
      const cart = cartFor(s);
      if (done(cart, cls, lvl, s)) { s++; continue; }
      let b = s;
      while (b + 1 <= hi && b - s + 1 < chunk && cartFor(b + 1) === cart && !done(cart, cls, lvl, b + 1)) b++;
      list.push({ cart, cls, lvl, lo: s, hi: b });
      s = b + 1;
    }
  }
  return list;
}

const FAULT_RES = [
  ['crash', /Fatal error|Unhandled Exception|EXCEPTION_ACCESS_VIOLATION|Assertion failed|=== Critical error/],
  ['ensure', /Ensure condition failed/],
  ['jamfx_missing', /JAMFX missing/],
  ['usage_flag', /missing usage flag/i],
  ['error_line', /Error: /],
];
const MATCH_TAG = '{"tool":"thirdjam.playtest/1"';
const strip = (l) => l.replace(/^\[[^\]]*\]\[[^\]]*\]/, '').trim().slice(0, 240);

// Lines are attributed to the seed whose match line follows them; the tail after the last match line belongs to the next unfinished seed.
function scanLog(text) {
  const matches = new Map(); const faults = new Map(); let pending = [];
  for (const raw of text.split(/\r?\n/)) {
    const i = raw.indexOf(MATCH_TAG);
    if (i >= 0) {
      try {
        const m = JSON.parse(raw.slice(i));
        matches.set(m.seed, m); faults.set(m.seed, pending);
        pending = [];
      } catch { pending.push({ kind: 'truncated_line', line: raw.slice(0, 200) }); }
      continue;
    }
    for (const [kind, re] of FAULT_RES) if (re.test(raw)) { pending.push({ kind, line: strip(raw) }); break; }
  }
  return { matches, faults, tail: pending };
}

function runChunk(c, tag) {
  const exe = join(engineRoot(), 'Engine', 'Binaries', 'Win64', 'UnrealEditor-Cmd.exe');
  const logPath = join(out, 'logs', `${tag}.log`);
  const seeds = []; for (let s = c.lo; s <= c.hi; s++) seeds.push(s);
  const args = [uproject, '-run=JamTournament', '-sim=jam', `-cartridge=${c.cart}`, `-difficulty=${c.lvl}`, `-player=${c.cls}`, `-seeds=${c.lo}-${c.hi}`,
    `-maxframes=${maxFrames}`, '-nullrhi', '-unattended', '-nopause', '-nosplash', '-stdout', '-FullStdOutLogOutput', `-ABSLOG=${logPath}.ue`];
  return new Promise((res) => {
    const p = spawn(exe, args, { windowsHide: true });
    let buf = '';
    p.stdout.on('data', (d) => { buf += d; });
    p.stderr.on('data', (d) => { buf += d; });
    let finished = false;
    const t = setTimeout(() => { buf += '\nSOAK timeout: chunk exceeded 20 min wall'; p.kill(); }, 1200e3);
    const fin = (code, err) => {
      if (finished) return; finished = true;
      clearTimeout(t);
      const { matches, faults, tail } = scanLog(buf);
      writeFileSync(logPath, buf);
      let crashSeed = null;
      for (const s of seeds) {
        const m = matches.get(s);
        if (m) writeFileSync(resFile(c.cart, c.cls, c.lvl, s), JSON.stringify({ cart: c.cart, cls: c.cls, lvl: c.lvl, seed: s, m, faults: faults.get(s) || [], log: `${tag}.log` }));
        else if (crashSeed === null) crashSeed = s;
      }
      if (crashSeed !== null) {
        const hot = buf.split(/\r?\n/).filter((l) => /Fatal|Unhandled|Assertion|Critical error|Error:|Access violation|0x[0-9A-F]{8,}/i.test(l)).slice(0, 6);
        const head = hot.map(strip).join(' | ').slice(0, 600) || buf.slice(-400).replace(/\s+/g, ' ');
        writeFileSync(resFile(c.cart, c.cls, c.lvl, crashSeed), JSON.stringify({ cart: c.cart, cls: c.cls, lvl: c.lvl, seed: crashSeed, m: null,
          crash: { exit: code, spawn_error: err ? String(err) : undefined, head }, faults: tail, log: `${tag}.log` }));
        console.log(`FAIL crash ${c.cart} ${c.cls} ${c.lvl} seed ${crashSeed}: exit ${code}`);
        return res({ requeue: crashSeed < c.hi ? { ...c, lo: crashSeed + 1 } : null, n: crashSeed - c.lo + 1 });
      }
      res({ requeue: null, n: seeds.length });
    };
    p.on('error', (e) => fin(-1, e));
    p.on('close', (code) => fin(code));
  });
}

async function main() {
  if (!engineRoot()) { console.log('FAIL soak: Unreal Engine not found (set UE_ROOT)'); process.exit(2); }
  if (!existsSync(uproject)) { console.log(`FAIL soak: no uproject ${uproject}`); process.exit(2); }
  mkdirSync(join(out, 'logs'), { recursive: true });
  for (const cart of ['hockey.regular', 'hockey.boost']) mkdirSync(join(out, 'results', cart), { recursive: true });
  console.log(`INFO soak: ${workers} workers (${lowWorkers} when < ${minFreeGB} GB free), chunk ${chunk}, round ${roundSize} seeds, until ${deadline === Infinity ? 'never' : new Date(deadline).toString()}`);
  let round = 1, queue = [], active = 0, started = 0, matchesDone = 0, seq = 0, exhausted = false;
  const refill = () => {
    while (!queue.length) {
      if (maxRounds && round > maxRounds) { exhausted = true; return false; }
      queue.push(...planRound(round)); round++;
    }
    return true;
  };
  await new Promise((resolveAll) => {
    let ended = false;
    const pump = () => {
      if (ended) return;
      const free = freeGB();
      const cap = free < minFreeGB ? lowWorkers : workers;
      while (active < cap && Date.now() < deadline) {
        if (!queue.length && !refill()) break;
        const c = queue.shift();
        active++; started++;
        const tag = `${c.cart.split('.')[1]}-${c.cls}-${c.lvl}-${c.lo}-${c.hi}-${Date.now().toString(36)}-${seq++}`;
        runChunk(c, tag).then((r) => {
          active--; matchesDone += r.n;
          if (r.requeue) queue.unshift(r.requeue);
          console.log(`INFO soak: chunk ${tag} ${r.n} done; ${matchesDone} matches this run; free ${freeGB().toFixed(1)} GB; active ${active}`);
          pump();
        });
      }
      if (active === 0 && (Date.now() >= deadline || exhausted)) { ended = true; resolveAll(); }
    };
    pump();
    setInterval(pump, 15000).unref();
  });
  console.log(`INFO soak: finished, ${matchesDone} matches this run, ${started} chunks`);
  console.log(JSON.stringify({ tool: 'thirdjam.soak/1', verdict: 'pass', matches: matchesDone, chunks: started, out }));
  process.exit(0);
}
if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) main();
