#!/usr/bin/env node
// Lane A107 (S1 soak): a memory-governed pool over many cells. One cell = class x level x cartridge x seed list; chunks of <= --chunk seeds are one headless
// JamTournament process each. Keeps total RAM in use inside a band: starts a chunk only below --hi GB used (default 48), never above --max workers.
//   node cells.mjs <plan.json> --uproject <abs uproject> --out <dir> [--max 16] [--hi 48] [--chunk 25] [--trace 30]
// plan.json: [{label, cls, lvl, cart, seeds:"1-2000" | "regularband:2000", extra:["-aitune=..."], trace:false}]
// Output: <out>/<label>.jsonl (match lines, deduped by seed), <out>/<label>.faults.jsonl (crash / ensure / error lines with seed, command), <out>/logs/<label>-<lo>-<hi>.log only
// when the cell has trace:true or the chunk had a fault or a missing seed. Writes <out>/<label>.done when complete. Resumable: a cell with .done is skipped.
// Never kills a process it did not start.
import { spawn, execFileSync } from 'node:child_process';
import { readFileSync, writeFileSync, appendFileSync, mkdirSync, existsSync } from 'node:fs';
import { join, resolve } from 'node:path';
const A = process.argv.slice(2);
const opt = (n, d) => { const i = A.indexOf(`--${n}`); return i >= 0 ? A[i + 1] : d; };
const plan = JSON.parse(readFileSync(A[0], 'utf8'));
const uproject = opt('uproject'); const out = resolve(opt('out')); const MAX = +opt('max', 16), HI = +opt('hi', 48), CHUNK = +opt('chunk', 25);
const exe = 'C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe';
mkdirSync(join(out, 'logs'), { recursive: true });
const usedGB = () => { try { return Number(execFileSync('powershell', ['-NoProfile', '-Command', '$o=Get-CimInstance Win32_OperatingSystem;($o.TotalVisibleMemorySize-$o.FreePhysicalMemory)'], { encoding: 'utf8' }).trim()) / 1048576; } catch { return 99; } };
const FAULT = [['crash', /Fatal error|Unhandled Exception|EXCEPTION_ACCESS_VIOLATION|Assertion failed|=== Critical error/], ['ensure', /Ensure condition failed/], ['jamfx_missing', /JAMFX missing/], ['error_line', /Error: /]];
const IGN = /Failed to load '(aqProf|Vtune)|Failed to open descriptor|LogWindows: Error|Error: Could not find (module|class)/i;
const range = (s) => { const [a, b] = s.split('-').map(Number); return [a, b ?? a]; };
function seedList(s) {
  const out = [];
  if (s.startsWith('regularband:')) { const n = +s.slice(12); for (let b = 0; out.length < n; b++) for (let k = 1; k <= 100 && out.length < n; k++) out.push(b * 150 + k); return out; }
  const [a, b] = range(s); for (let i = a; i <= b; i++) out.push(i); return out;
}
function chunksOf(list) { // contiguous runs, <= CHUNK
  const r = []; let lo = list[0], prev = list[0];
  for (let i = 1; i <= list.length; i++) { const x = list[i]; if (x === undefined || x !== prev + 1 || prev - lo + 1 >= CHUNK) { r.push([lo, prev]); lo = x; } prev = x; }
  return r;
}
const cells = plan.filter((c) => !existsSync(join(out, `${c.label}.done`))).map((c) => ({ ...c, left: 0, got: new Map(), faults: [] }));
const queue = [];
for (const c of cells) { const ch = chunksOf(seedList(c.seeds)); c.left = ch.length; for (const [lo, hi] of ch) queue.push({ c, lo, hi }); }
let active = 0, doneChunks = 0, totalChunks = queue.length, lastStart = 0;
const tfmt = () => new Date().toTimeString().slice(0, 8);
function finishCell(c) {
  const ms = [...c.got.values()].sort((a, b) => a.seed - b.seed);
  writeFileSync(join(out, `${c.label}.jsonl`), ms.map((m) => JSON.stringify(m)).join('\n') + '\n');
  writeFileSync(join(out, `${c.label}.faults.jsonl`), c.faults.map((f) => JSON.stringify(f)).join('\n') + (c.faults.length ? '\n' : ''));
  writeFileSync(join(out, `${c.label}.done`), `${ms.length} matches, ${c.faults.length} faults, ${tfmt()}\n`);
  console.log(`CELL ${c.label} done: ${ms.length} matches, ${c.faults.length} faults`);
}
function runChunk({ c, lo, hi }) {
  return new Promise((res) => {
    const args = [uproject, '-run=JamTournament', '-sim=jam', `-cartridge=${c.cart}`, `-difficulty=${c.lvl}`, `-player=${c.cls}`, `-seeds=${lo}-${hi}`, `-maxframes=${c.maxframes || 36000}`,
      '-nullrhi', '-unattended', '-nopause', '-nosplash', '-stdout', '-FullStdOutLogOutput', '-Multiprocess', ...(c.extra || [])];
    if (c.trace) args.push('-trace=30');
    const p = spawn(exe, args, { windowsHide: true });
    let buf = ''; p.stdout.on('data', (d) => { buf += d; }); p.stderr.on('data', (d) => { buf += d; });
    const to = setTimeout(() => { buf += '\nSOAK timeout 25 min'; p.kill(); }, 1500e3);
    p.on('close', (code) => {
      clearTimeout(to);
      let seen = new Set(), pending = [], keep = !!c.trace;
      for (const l of buf.split(/\r?\n/)) {
        const i = l.indexOf('{"tool":"thirdjam.playtest/1"');
        if (i >= 0) { try { const m = JSON.parse(l.slice(i)); c.got.set(m.seed, m); seen.add(m.seed); for (const f of pending) c.faults.push({ ...f, seed: m.seed }); pending = []; } catch {} continue; }
        if (IGN.test(l)) continue;
        for (const [kind, re] of FAULT) if (re.test(l)) { pending.push({ kind, line: l.replace(/^\[[^\]]*\]\[[^\]]*\]/, '').trim().slice(0, 220) }); break; }
      }
      let missing = []; for (let s = lo; s <= hi; s++) if (!seen.has(s)) missing.push(s);
      if (missing.length) { c.faults.push({ kind: 'missing_seeds', seed: missing[0], count: missing.length, exit: code, tail: pending.slice(0, 4), tailLog: buf.slice(-500).replace(/\s+/g, ' ') }); keep = true; }
      for (const f of pending) c.faults.push({ ...f, seed: missing[0] ?? hi, after_last_match: true });
      if (pending.length) keep = true;
      if (keep) writeFileSync(join(out, 'logs', `${c.label}-${lo}-${hi}.log`), buf);
      c.cmd = `UnrealEditor-Cmd <uproject> -run=JamTournament -sim=jam -cartridge=${c.cart} -difficulty=${c.lvl} -player=${c.cls} -seeds=SEED-SEED -maxframes=36000 -nullrhi -unattended -nopause -nosplash -stdout -FullStdOutLogOutput`;
      for (const f of c.faults) f.cmd ??= c.cmd.replaceAll('SEED', f.seed);
      if (missing.length && missing[0] < hi && (c.requeues = (c.requeues || 0) + 1) < 60) { c.left++; queue.unshift({ c, lo: missing[0] + 1, hi }); totalChunks++; }
      if (--c.left === 0) finishCell(c);
      res();
    });
  });
}
await new Promise((resolveAll) => {
  const pump = () => {
    if (!queue.length && active === 0) return resolveAll();
    if (queue.length && active < MAX && Date.now() - lastStart > 4000) {
      if (active < 2 || usedGB() < HI) { lastStart = Date.now(); active++; runChunk(queue.shift()).then(() => { active--; doneChunks++; if (doneChunks % 20 === 0) console.log(`${tfmt()} ${doneChunks}/${totalChunks} chunks, active ${active}, used ${usedGB().toFixed(1)} GB`); pump(); }); }
    }
  };
  setInterval(pump, 2000);
  pump();
});
console.log('ALL DONE');
process.exit(0);
