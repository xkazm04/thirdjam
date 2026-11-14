#!/usr/bin/env node
// Lane A174 (S1 evolution 4): per-process memory of the soak's own UnrealEditor-Cmd children.
//   node tools/soak/memwatch.mjs sample --match <substring of the .uproject path> --out <file.jsonl> [--every 15] [--until-file <file>]
//   node tools/soak/memwatch.mjs report <file.jsonl> [--min-samples 6]
// sample: every N seconds one line per matching process {t, pid, ws_mb, priv_mb}; stops when --until-file exists. Reads only; kills nothing.
// report: per process (pid), mean working set of its first and last quarter of samples and the change; a process with too few samples is listed but not judged.
import { appendFileSync, readFileSync, existsSync } from 'node:fs';
import { execFileSync } from 'node:child_process';
const argv = process.argv.slice(2);
const opt = (n, d) => { const i = argv.indexOf(`--${n}`); return i >= 0 ? argv[i + 1] : d; };
// a process warms up for about 40 s (working set 0.7 to 1.8 GB): samples before that are left out of the quarters.
export const WARMUP_MS = 40000;
export function quarters(rows) {
  const w = rows.filter((r) => r.t - rows[0].t >= WARMUP_MS); if (w.length >= 4) rows = w;
  const k = Math.max(1, Math.floor(rows.length / 4)); const m = (a) => a.reduce((s, x) => s + x, 0) / a.length;
  return { first: m(rows.slice(0, k).map((r) => r.ws_mb)), last: m(rows.slice(-k).map((r) => r.ws_mb)) };
}
export function report(file, minSamples = 12) {
  const by = new Map();
  for (const l of readFileSync(file, 'utf8').split('\n').filter(Boolean)) { const r = JSON.parse(l); if (!by.has(r.pid)) by.set(r.pid, []); by.get(r.pid).push(r); }
  const out = [];
  for (const [pid, rows] of by) {
    const q = quarters(rows); out.push({ pid, samples: rows.length, span_s: Math.round((rows.at(-1).t - rows[0].t) / 1000), first_q_mb: Math.round(q.first), last_q_mb: Math.round(q.last), change_pct: Math.round(1000 * (q.last - q.first) / q.first) / 10, max_mb: Math.round(Math.max(...rows.map((r) => r.ws_mb))), judged: rows.length >= minSamples });
  }
  return out;
}
function sample(match) {
  const ps = `Get-CimInstance Win32_Process -Filter "Name like 'UnrealEditor-Cmd%'" | Where-Object { $_.CommandLine -like '*${match.replace(/'/g, "''")}*' } | ForEach-Object { "$($_.ProcessId) $([math]::Round($_.WorkingSetSize/1MB,1)) $([math]::Round($_.PrivatePageCount/1MB,1))" }`;
  const txt = execFileSync('powershell', ['-NoProfile', '-Command', ps], { encoding: 'utf8' });
  return txt.split('\n').map((l) => l.trim().split(' ')).filter((a) => a.length === 3).map(([pid, ws, pr]) => ({ pid: +pid, ws_mb: +ws, priv_mb: +pr }));
}
if (argv[0] === 'sample') {
  const out = opt('out'); const every = +opt('every', 15) * 1000; const stop = opt('until-file'); const match = opt('match');
  while (!(stop && existsSync(stop))) { const t = Date.now(); for (const r of sample(match)) appendFileSync(out, JSON.stringify({ t, ...r }) + '\n'); await new Promise((r) => setTimeout(r, every)); }
} else if (argv[0] === 'report') {
  const rows = report(argv[1], +opt('min-samples', 6)); for (const r of rows) console.log(JSON.stringify(r));
}
