// Lane A58: the hitch list from an Insights trace, headless.
//   1. Exports the game thread's frame scopes (FEngineLoop::Tick) through insights-export.ps1 and finds the frames whose
//      start-to-start time is over 33.3 ms (a hitch the player sees), after the A58_match_start bookmark when -fromBookmark.
//   2. For each of the worst N hitches exports every timing event of that frame on every thread and prints the top stack:
//      from the frame scope down, always the longest child, on the game thread; plus the longest scopes on the other threads.
// Usage: node tools/perf/hitches.mjs <trace.utrace> [--n 10] [--from <seconds>] [--json out.json]
import fs from 'node:fs'; import path from 'node:path'; import { execFileSync } from 'node:child_process';
const args = process.argv.slice(2);
const trace = path.resolve(args[0]);
const opt = (k, d) => { const i = args.indexOf(k); return i >= 0 ? args[i + 1] : d; };
const N = Number(opt('--n', 10)); const from = Number(opt('--from', 0)); const jsonOut = opt('--json');
const here = path.dirname(new URL(import.meta.url).pathname.replace(/^\/([A-Za-z]:)/, '$1'));
const outDir = path.join(path.dirname(trace), 'insights');
function insights(cmd) {
  execFileSync('powershell', ['-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', path.join(here, 'insights-export.ps1'), '-Trace', trace, '-Cmd', cmd], { stdio: 'pipe' });
}
function readCsv(f) {
  // Columns ThreadName,TimerName,StartTime,EndTime,Duration,Depth; a timer name may hold commas, so the four numbers are read from the end.
  const L = fs.readFileSync(f, 'utf8').split(/\r?\n/).filter(Boolean);
  return L.slice(1).map(l => { const c = l.split(','); const n = c.length;
    return { ThreadName: c[0], TimerName: c.slice(1, n - 4).join(','), StartTime: c[n - 4], EndTime: c[n - 3], Duration: c[n - 2], Depth: c[n - 1] }; });
}
const cols = '-columns=ThreadName,TimerName,StartTime,EndTime,Duration,Depth';
const framesCsv = path.join(outDir, 'gt-frames.csv');
if (!fs.existsSync(framesCsv)) insights(`TimingInsights.ExportTimingEvents {out}\\gt-frames.csv ${cols} -threads=GameThread -timers=FEngineLoop::Tick`);
const fr = readCsv(framesCsv).filter(r => r.TimerName === 'FEngineLoop::Tick').map(r => ({ s: +r.StartTime, e: +r.EndTime, d: +r.Duration })).sort((a, b) => a.s - b.s);
const frames = fr.map((f, i) => ({ i, s: f.s, e: f.e, tick_ms: f.d * 1000, frame_ms: i + 1 < fr.length ? (fr[i + 1].s - f.s) * 1000 : f.d * 1000 })).filter(f => f.s >= from);
const hitches = frames.filter(f => f.frame_ms > 33.34).sort((a, b) => b.frame_ms - a.frame_ms);
const ft = frames.map(f => f.frame_ms).sort((a, b) => a - b);
const q = p => ft[Math.min(ft.length - 1, Math.floor(p * (ft.length - 1)))];
const res = { trace, frames: frames.length, from, mean_ms: ft.reduce((a, b) => a + b, 0) / ft.length, p99_ms: q(0.99), max_ms: ft[ft.length - 1], hitches: [] };
console.log(`frames ${frames.length} from t=${from}s: mean ${res.mean_ms.toFixed(2)} p99 ${res.p99_ms.toFixed(2)} max ${res.max_ms.toFixed(2)} ms; hitches > 33.3 ms: ${hitches.length}`);
for (const h of hitches.slice(0, N)) {
  const f = path.join(outDir, `hitch-${h.i}.csv`);
  if (!fs.existsSync(f)) insights(`TimingInsights.ExportTimingEvents {out}\\hitch-${h.i}.csv ${cols} -startTime=${h.s.toFixed(6)} -endTime=${(h.s + h.frame_ms / 1000).toFixed(6)}`);
  const ev = readCsv(f).map(r => ({ t: r.ThreadName, n: r.TimerName, s: +r.StartTime, e: +r.EndTime, d: +r.Duration * 1000, z: +r.Depth }));
  // Game thread: descend from the frame scope by the longest child.
  const gt = ev.filter(x => x.t === 'GameThread').sort((a, b) => a.s - b.s);
  let node = gt.filter(x => x.z === 0).sort((a, b) => b.d - a.d)[0]; const stack = [];
  while (node && stack.length < 14) {
    stack.push(`${node.n} ${node.d.toFixed(1)}ms`);
    const kids = gt.filter(x => x.z === node.z + 1 && x.s >= node.s && x.e <= node.e).sort((a, b) => b.d - a.d);
    node = kids[0];
  }
  // Longest leaf-ish scopes anywhere (depth >= 2) on any thread, for the "what else ran" line.
  const others = ev.filter(x => x.t !== 'GameThread' && x.z >= 1).sort((a, b) => b.d - a.d).slice(0, 4).map(x => `${x.t}:${x.n} ${x.d.toFixed(1)}ms`);
  const gtTop = gt.filter(x => x.z >= 2).sort((a, b) => b.d - a.d).slice(0, 6).map(x => `${x.n} ${x.d.toFixed(1)}ms`);
  const rec = { frame_index: h.i, t_s: +h.s.toFixed(3), frame_ms: +h.frame_ms.toFixed(1), tick_ms: +h.tick_ms.toFixed(1), stack, game_thread_longest: gtTop, other_threads: others };
  res.hitches.push(rec);
  console.log(`\nhitch frame#${h.i} t=${rec.t_s}s ${rec.frame_ms} ms (tick ${rec.tick_ms} ms)\n  stack: ${stack.join(' > ')}\n  GT longest: ${gtTop.join(' | ')}\n  other threads: ${others.join(' | ')}`);
}
if (jsonOut) fs.writeFileSync(jsonOut, JSON.stringify(res, null, 1));
