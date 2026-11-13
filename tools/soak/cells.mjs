#!/usr/bin/env node
// Lane A116 (track S1 soak, evolution 2): THE shared soak driver (from A107's tools/soak/a107/cells.mjs). A plan file in, one JSON line per match out.
//   node tools/soak/cells.mjs <plan.json> --uproject <abs .uproject of a BUILT checkout> --out <dir> [--max 16] [--chunk 100] [--lo 38] [--hi 50] [--stop 56]
//                             [--cpu-lo 0.85] [--cpu-hi 0.97] [--cpu-floor 4] [--stagger-ms 1000] [--exe <UnrealEditor-Cmd.exe>] [--runner <script.js>]   (runner: spawn `node script args...` instead of the exe; used by the node test)
// plan.json: [{label, cls, lvl, cart, seeds:"1-2000" | "regularband:2000" | "3,5,9-12", extra:["-playersprint"], trace:false, maxframes:36000}]
// Output (all under --out): <label>.jsonl one thirdjam.playtest/1 line per match (sorted by seed, deduped), <label>.faults.jsonl one line per fault with seed and the command that
// repeats it, <label>.done, faults.jsonl (the per-seed fault ledger of the whole run: every fault with its label), logs/<label>-<lo>-<hi>.log only for a chunk with a fault, a
// missing seed or trace:true. Last stdout line: {"tool":"thirdjam.soak/1",...}. Exit 0 no faults, 1 faults, 2 could not start (the start guard).
// Rules it keeps (EVOLUTION-COMMON): -Multiprocess on every start (skips the UBT ValidatePlatforms mutex: no wait behind other lanes' builds); memory band: under --lo GB in use
// start up to --max workers, between --lo and --hi trickle (up to 4 active), over --hi start nothing (over --stop, 56, not even a trickle); a start guard (the exe and project
// exist, the first chunk runs alone and must print a match line, else exit 2); a crashed chunk is requeued after its crashing seed (a chunk with no match at all is retried
// once whole first); resumable (<label>.partial.jsonl keeps the seeds of finished chunks, <label>.done skips a cell). Kills only the processes it started.
import { spawn } from 'node:child_process';
import { readFileSync, writeFileSync, appendFileSync, mkdirSync, existsSync } from 'node:fs';
import { join, resolve } from 'node:path';
import { totalmem, freemem, cpus } from 'node:os';
import { pathToFileURL } from 'node:url';

export const DEFAULT_EXE = 'C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe';
const FAULT = [['crash', /Fatal error|Unhandled Exception|EXCEPTION_ACCESS_VIOLATION|Assertion failed|=== Critical error/], ['ensure', /Ensure condition failed/], ['jamfx_missing', /JAMFX missing/], ['error_line', /Error: /]];
const IGN = /Failed to load '(aqProf|Vtune)|Failed to open descriptor|LogWindows: Error|Error: Could not find (module|class)/i;

// "1-2000", "regularband:2000" (A53's band: 1-100 of each 150), or a list "3,5,9-12".
export function seedList(s) {
  const out = [];
  if (s.startsWith('regularband:')) { const n = +s.slice(12); for (let b = 0; out.length < n; b++) for (let k = 1; k <= 100 && out.length < n; k++) out.push(b * 150 + k); return out; }
  for (const part of s.split(',')) { const [a, b] = part.split('-').map(Number); for (let i = a; i <= (b ?? a); i++) out.push(i); }
  return out;
}
// contiguous runs of at most `chunk` seeds: one process each (the command line takes a range)
export function chunksOf(list, chunk) {
  const r = []; if (!list.length) return r; let lo = list[0], prev = list[0];
  for (let i = 1; i <= list.length; i++) { const x = list[i]; if (x === undefined || x !== prev + 1 || prev - lo + 1 >= chunk) { r.push([lo, prev]); lo = x; } prev = x; }
  return r;
}
// The governor: may a new worker start? Memory band 38-50 GB in use (R-0047: the operator's 60-80 % of 63 GB) and, lane A126, the processor: `cpu` is the whole machine's
// busy share (0..1) over the last sample (Windows has no load average, so os.cpus() deltas stand in for it; other lanes' builds count, ours too). Below cpuLo every worker up
// to max may run; above cpuHi only cpuFloor (4) may, so a build next door is never starved and the matrix never stops; between, the limit slides linearly. cpu undefined = ignored.
export function cpuLimit({ cpu, max, cpuLo = 0.85, cpuHi = 0.97, cpuFloor = 4 }) {
  if (cpu === undefined || cpu === null || cpu <= cpuLo) return max;
  const floor = Math.min(cpuFloor, max);
  if (cpu >= cpuHi) return floor;
  return Math.max(floor, Math.round(floor + (max - floor) * (cpuHi - cpu) / (cpuHi - cpuLo)));
}
export function mayStart({ usedGB, active, max, lo = 38, hi = 50, stop = 56, cpu, cpuLo, cpuHi, cpuFloor }) {
  if (active >= max) return false;
  if (usedGB >= stop || usedGB >= hi) return false;
  if (usedGB >= lo) return active < Math.min(4, max);
  return active < cpuLimit({ cpu, max, cpuLo, cpuHi, cpuFloor });
}
// the machine's busy share since the previous call (0..1); the first call reads over a short window. SOAK_FAKE_CPU overrides (tests).
let lastCpu = null;
export function cpuBusy() {
  if (process.env.SOAK_FAKE_CPU) return Number(process.env.SOAK_FAKE_CPU);
  const t = cpus().reduce((a, c) => { const x = Object.values(c.times); a.all += x.reduce((p, q) => p + q, 0); a.idle += c.times.idle; return a; }, { all: 0, idle: 0 });
  const prev = lastCpu; lastCpu = t;
  if (!prev || t.all === prev.all) return undefined;
  return 1 - (t.idle - prev.idle) / (t.all - prev.all);
}
// One process's stdout: match lines, faults pinned to the next match line that follows them, and the seeds that never came.
export function parseChunk(buf, lo, hi) {
  const matches = [], faults = []; let pending = [];
  for (const l of buf.split(/\r?\n/)) {
    const i = l.indexOf('{"tool":"thirdjam.playtest/1"');
    if (i >= 0) { try { const m = JSON.parse(l.slice(i)); matches.push(m); for (const f of pending) faults.push({ ...f, seed: m.seed }); pending = []; } catch { /* torn line */ } continue; }
    if (IGN.test(l)) continue;
    for (const [kind, re] of FAULT) if (re.test(l)) { pending.push({ kind, line: l.replace(/^\[[^\]]*\]\[[^\]]*\]/, '').trim().slice(0, 220) }); break; }
  }
  const seen = new Set(matches.map((m) => m.seed)); const missing = [];
  for (let s = lo; s <= hi; s++) if (!seen.has(s)) missing.push(s);
  return { matches, faults, pending, missing };
}
export function chunkArgs(uproject, c, lo, hi) {
  const a = [uproject, '-run=JamTournament', '-sim=jam', `-cartridge=${c.cart}`, `-difficulty=${c.lvl}`, `-player=${c.cls}`, `-seeds=${lo}-${hi}`, `-maxframes=${c.maxframes || 36000}`,
    '-nullrhi', '-unattended', '-nopause', '-nosplash', '-stdout', '-FullStdOutLogOutput', '-Multiprocess', ...(c.extra || [])];
  if (c.trace) a.push('-trace=30');
  return a;
}
export const repeatCommand = (c, seed) => `UnrealEditor-Cmd <uproject> ${chunkArgs('', c, seed, seed).slice(1).join(' ')}`;
export const usedGB = () => (Number(process.env.SOAK_FAKE_USED_GB) || (totalmem() - freemem()) / 2 ** 30);

export async function run(argv) {
  const opt = (n, d) => { const i = argv.indexOf(`--${n}`); return i >= 0 ? argv[i + 1] : d; };
  const fail2 = (msg) => { console.log(`FAIL soak could not start: ${msg}`); console.log(JSON.stringify({ tool: 'thirdjam.soak/1', verdict: 'could_not_run', exit: 2, error: msg })); return 2; };
  if (!argv[0] || argv[0].startsWith('--')) return fail2('usage: cells.mjs <plan.json> --uproject <abs> --out <dir>');
  const plan = JSON.parse(readFileSync(argv[0], 'utf8'));
  const uproject = opt('uproject'), outArg = opt('out'), runner = opt('runner'), exe = opt('exe', DEFAULT_EXE);
  const CPU_LO = +opt('cpu-lo', 0.85), CPU_HI = +opt('cpu-hi', 0.97), CPU_FLOOR = +opt('cpu-floor', 4);   // lane A126: the processor band
  const MAX = +opt('max', 16), CHUNK = +opt('chunk', 100), LO = +opt('lo', 38), HI = +opt('hi', 50), STOP = +opt('stop', 56);
  // the start guard, first half
  if (!outArg) return fail2('--out missing');
  if (!runner && !uproject) return fail2('--uproject missing');
  if (!runner && !existsSync(exe)) return fail2(`exe not found: ${exe}`);
  if (!runner && !existsSync(uproject)) return fail2(`uproject not found: ${uproject}`);
  const labels = plan.map((c) => c.label); const dup = labels.find((l, i) => labels.indexOf(l) !== i);
  if (dup) return fail2(`duplicate label in the plan: ${dup}`);
  for (const c of plan) for (const k of ['label', 'cls', 'lvl', 'cart', 'seeds']) if (!c[k]) return fail2(`plan cell ${c.label || '?'} lacks ${k}`);
  const out = resolve(outArg); mkdirSync(join(out, 'logs'), { recursive: true });
  const tfmt = () => new Date().toTimeString().slice(0, 8);
  const ledger = join(out, 'faults.jsonl');

  function finishCell(c) {
    const ms = [...c.got.values()].sort((a, b) => a.seed - b.seed);
    writeFileSync(join(out, `${c.label}.jsonl`), ms.map((m) => JSON.stringify(m)).join('\n') + (ms.length ? '\n' : ''));
    writeFileSync(join(out, `${c.label}.faults.jsonl`), c.faults.map((f) => JSON.stringify(f)).join('\n') + (c.faults.length ? '\n' : ''));
    writeFileSync(join(out, `${c.label}.done`), `${ms.length} matches, ${c.faults.length} faults, ${tfmt()}\n`);
    console.log(`CELL ${c.label} done: ${ms.length} matches, ${c.faults.length} faults`);
  }
  const cells = plan.filter((c) => !existsSync(join(out, `${c.label}.done`))).map((c) => ({ ...c, left: 0, got: new Map(), faults: [], requeues: 0 }));
  const queue = [];
  for (const c of cells) {
    const part = join(out, `${c.label}.partial.jsonl`);
    if (existsSync(part)) for (const l of readFileSync(part, 'utf8').split('\n').filter(Boolean)) { try { const m = JSON.parse(l); c.got.set(m.seed, m); } catch { /* torn */ } }
    const ch = chunksOf(seedList(c.seeds).filter((s) => !c.got.has(s)), CHUNK); c.left = ch.length;
    for (const [lo, hi] of ch) queue.push({ c, lo, hi, attempt: 0 });
    if (!ch.length) finishCell(c);
  }
  let cpuNow, cpuAt = 0;
  let active = 0, doneChunks = 0, totalChunks = queue.length, lastStart = 0;
  const t0 = Date.now();

  function runChunk(job) {
    const { c, lo, hi } = job;
    return new Promise((res) => {
      const args = chunkArgs(uproject, c, lo, hi);
      const p = runner ? spawn(process.execPath, [runner, ...args], { windowsHide: true }) : spawn(exe, args, { windowsHide: true });
      let buf = ''; p.stdout.on('data', (d) => { buf += d; }); p.stderr.on('data', (d) => { buf += d; });
      const to = setTimeout(() => { buf += '\nSOAK timeout'; p.kill(); }, +opt('timeout-s', 1500) * 1000);
      p.on('error', (e) => { buf += `\nSOAK spawn error ${e.message}`; });
      p.on('close', (code) => {
        clearTimeout(to);
        const r = parseChunk(buf, lo, hi); let keep = !!c.trace || r.missing.length > 0 || r.faults.length > 0 || r.pending.length > 0;
        for (const m of r.matches) { c.got.set(m.seed, m); appendFileSync(join(out, `${c.label}.partial.jsonl`), JSON.stringify(m) + '\n'); }
        const fresh = [...r.faults];
        let requeue = null;
        if (r.missing.length) {
          if (!r.matches.length && job.attempt < 1) requeue = { c, lo, hi, attempt: 1 };   // a start that gave nothing: once more, whole
          else {
            const skip = r.missing[0];   // the seed that was running when the process died
            fresh.push({ kind: 'missing_seeds', seed: skip, count: r.missing.length, exit: code, tail: r.pending.slice(0, 4), tailLog: buf.slice(-500).replace(/\s+/g, ' ') });
            if (skip < hi && ++c.requeues < 60) requeue = { c, lo: skip + 1, hi, attempt: 0 };
          }
        }
        for (const f of r.pending) fresh.push({ ...f, seed: r.missing[0] ?? hi, after_last_match: true });
        for (const f of fresh) { f.cmd = repeatCommand(c, f.seed); c.faults.push(f); appendFileSync(ledger, JSON.stringify({ label: c.label, ...f }) + '\n'); }
        if (keep) writeFileSync(join(out, 'logs', `${c.label}-${lo}-${hi}.log`), buf);
        if (requeue) { queue.unshift(requeue); c.left += 0; totalChunks++; } else if (--c.left === 0) finishCell(c);
        res(r.matches.length);
      });
    });
  }
  // the start guard, second half: the first chunk runs alone (twice at most) and a match line must come out
  for (let k = 0; k < 2 && queue.length && !cells.some((c) => c.got.size); k++) { await runChunk(queue.shift()); doneChunks++; }
  if (cells.length && !cells.some((c) => c.got.size)) return fail2('the first chunk printed no match line (logs/ in the out dir): is the project built and the cartridge name right?');
  await new Promise((resolveAll) => {
    const iv = setInterval(() => pump(), runner ? 20 : 2000);
    const pump = () => {
      if (!queue.length && active === 0) { clearInterval(iv); return resolveAll(); }
      if (Date.now() - cpuAt >= 1500) { const x = cpuBusy(); if (x !== undefined) cpuNow = x; cpuAt = Date.now(); }   // resampled at most every 1.5 s (a pump after each chunk would read a tiny window)
      const cpu = cpuNow;
      if (queue.length && Date.now() - lastStart > (runner ? 0 : +opt('stagger-ms', 1000)) && mayStart({ usedGB: usedGB(), active, max: MAX, lo: LO, hi: HI, stop: STOP, cpu, cpuLo: CPU_LO, cpuHi: CPU_HI, cpuFloor: CPU_FLOOR })) {
        lastStart = Date.now(); active++;
        runChunk(queue.shift()).then(() => { active--; doneChunks++; if (doneChunks % 20 === 0) console.log(`${tfmt()} ${doneChunks}/${totalChunks} chunks, active ${active}, used ${usedGB().toFixed(1)} GB, cpu ${cpuNow === undefined ? '?' : Math.round(100 * cpuNow)} %`); pump(); });
      }
    };
    pump();
  });
  let matches = 0, faults = 0;
  for (const c of plan) { const f = join(out, `${c.label}.done`); if (existsSync(f)) { const m = /^(\d+) matches, (\d+) faults/.exec(readFileSync(f, 'utf8')); if (m) { matches += +m[1]; faults += +m[2]; } } }
  const secs = Math.round((Date.now() - t0) / 1000);
  console.log(`${faults ? 'FAIL' : 'PASS'} soak ${matches} matches in ${secs} s, ${faults} faults, ${plan.length} cells`);
  console.log(JSON.stringify({ tool: 'thirdjam.soak/1', verdict: faults ? 'fail' : 'pass', exit: faults ? 1 : 0, matches, faults, cells: plan.length, seconds: secs, chunks: totalChunks, out }));
  return faults ? 1 : 0;
}

if (process.argv[1] && import.meta.url === pathToFileURL(resolve(process.argv[1])).href) { const code = await run(process.argv.slice(2)); process.exit(code); }
