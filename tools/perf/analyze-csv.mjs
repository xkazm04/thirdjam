// Lane A58: read a CSV profiler capture (csvprofile start/stop in the packaged Development build) and print
//   - the stat-unit figures (FrameTime, GameThreadTime, RenderThreadTime, RHIThreadTime, GPUTime): mean, p50, p99, max
//   - the hitches (FrameTime > 33.3 ms) with their row (frame within the capture) and seconds into the capture
//   - the per-system budget: Exclusive/GameThread/*, Exclusive/RenderThread/*, GPU/* means, sorted
//   - memory columns at start and end (PhysicalUsedMB, LLM/*): the leak view
// Usage: node tools/perf/analyze-csv.mjs <capture.csv> [--json out.json] [--top 25]
import fs from 'node:fs';

const args = process.argv.slice(2);
const file = args[0];
if (!file) { console.error('usage: analyze-csv.mjs <csv> [--json out.json]'); process.exit(2); }
const opt = (k, d) => { const i = args.indexOf(k); return i >= 0 ? args[i + 1] : d; };
const top = Number(opt('--top', 25));

const lines = fs.readFileSync(file, 'utf8').split(/\r?\n/).filter(l => l.length);
// RFC-ish split: the EVENTS column may hold quoted text with commas.
const split = l => { const out = []; let cur = '', q = false; for (const ch of l) { if (ch === '"') q = !q; else if (ch === ',' && !q) { out.push(cur); cur = ''; } else cur += ch; } out.push(cur); return out; };
const header = split(lines[0]);
const rows = [];
const meta = {};
for (let i = 1; i < lines.length; i++) {
  const c = split(lines[i]);
  if (c[0] && c[0].startsWith('[')) { for (let j = 0; j + 1 < c.length; j += 2) meta[c[j].replace(/[\[\]]/g, '')] = c[j + 1]; continue; }
  if (c[0] === header[0] && c[1] === header[1]) continue; // header repeated at the end
  if (c.length < header.length - 1) continue;
  rows.push(c);
}
const col = name => header.indexOf(name);
const series = name => { const i = col(name); return i < 0 ? null : rows.map(r => Number(r[i])).filter(v => Number.isFinite(v)); };
const stats = a => { if (!a || !a.length) return null; const s = [...a].sort((x, y) => x - y); const q = p => s[Math.min(s.length - 1, Math.floor(p * (s.length - 1)))];
  return { n: a.length, mean: a.reduce((x, y) => x + y, 0) / a.length, p50: q(0.5), p99: q(0.99), max: s[s.length - 1] }; };
const r2 = v => Math.round(v * 100) / 100;

const out = { file, frames: rows.length, meta: { commandline: meta.commandline, buildversion: meta.buildversion, platform: meta.platform } };
out.unit = {};
for (const k of ['FrameTime', 'GameThreadTime', 'RenderThreadTime', 'RHIThreadTime', 'GPUTime', 'GameThreadTime_CriticalPath', 'RenderThreadTime_CriticalPath', 'InputLatencyTime']) {
  const s = stats(series(k)); if (s) out.unit[k] = Object.fromEntries(Object.entries(s).map(([a, b]) => [a, a === 'n' ? b : r2(b)]));
}
// Hitches.
const ft = series('FrameTime');
const evI = col('EVENTS');
let t = 0; out.hitches = [];
rows.forEach((r, i) => { const f = Number(r[col('FrameTime')]); if (f > 33.34) out.hitches.push({ row: i, t_s: r2(t / 1000), frame_ms: r2(f), game_ms: r2(Number(r[col('GameThreadTime')])), render_ms: r2(Number(r[col('RenderThreadTime')])), gpu_ms: r2(Number(r[col('GPUTime')])), events: evI >= 0 ? r[evI] : '' }); t += f; });
out.capture_s = r2(t / 1000);
// Events (bookmarks, csv events) for the timeline.
out.events = []; t = 0;
rows.forEach((r, i) => { if (evI >= 0 && r[evI]) out.events.push({ row: i, t_s: r2(t / 1000), ev: r[evI] }); t += Number(r[col('FrameTime')]); });
// Per-system budgets.
const budget = prefix => header.map((h, i) => [h, i]).filter(([h]) => h.startsWith(prefix)).map(([h]) => { const s = stats(series(h)); return { name: h, mean: r2(s.mean), p99: r2(s.p99), max: r2(s.max) }; }).sort((a, b) => b.mean - a.mean);
out.gameThread = budget('Exclusive/GameThread/').slice(0, top);
out.renderThread = budget('Exclusive/RenderThread/').slice(0, top);
out.gpu = budget('GPU/').slice(0, top);
out.counts = ['RHI/DrawCalls', 'RHI/PrimitivesDrawn', 'Ticks/Total', 'Ticks/SkeletalMeshComponent', 'ActorCount/TotalActorCount', 'PSO/PSOMisses', 'PSO/PSOMissesOnHitch', 'LightCount/All', 'Shaders/NumShadersCreated', 'FileIO/PerFrameKB']
  .map(k => { const s = stats(series(k)); return s ? { name: k, mean: r2(s.mean), max: r2(s.max) } : null; }).filter(Boolean);
// Memory first and last row.
const memCols = header.filter(h => h === 'PhysicalUsedMB' || h === 'MemoryFreeMB' || h.startsWith('LLM/') && h.split('/').length === 2 || h.startsWith('GPUMem/'));
out.memory = memCols.map(h => { const s = series(h); return s && s.length ? { name: h, first: r2(s[0]), last: r2(s[s.length - 1]), delta: r2(s[s.length - 1] - s[0]), max: r2(Math.max(...s)) } : null; })
  .filter(Boolean).sort((a, b) => b.last - a.last);

if (opt('--json')) fs.writeFileSync(opt('--json'), JSON.stringify(out, null, 1));
console.log(`frames ${out.frames}, capture ${out.capture_s} s`);
for (const [k, v] of Object.entries(out.unit)) console.log(`${k.padEnd(30)} mean ${v.mean} p50 ${v.p50} p99 ${v.p99} max ${v.max}`);
console.log(`hitches (> 33.3 ms): ${out.hitches.length}`);
for (const h of out.hitches.slice(0, 30)) console.log(`  row ${h.row} t=${h.t_s}s frame ${h.frame_ms} game ${h.game_ms} render ${h.render_ms} gpu ${h.gpu_ms} ${h.events}`);
console.log('game thread exclusive (ms):'); for (const b of out.gameThread.slice(0, 15)) console.log(`  ${b.name.padEnd(50)} ${b.mean} p99 ${b.p99} max ${b.max}`);
console.log('render thread exclusive (ms):'); for (const b of out.renderThread.slice(0, 12)) console.log(`  ${b.name.padEnd(50)} ${b.mean} p99 ${b.p99} max ${b.max}`);
console.log('GPU (ms):'); for (const b of out.gpu.slice(0, 20)) console.log(`  ${b.name.padEnd(50)} ${b.mean} p99 ${b.p99} max ${b.max}`);
console.log('counts:'); for (const b of out.counts) console.log(`  ${b.name.padEnd(30)} mean ${b.mean} max ${b.max}`);
console.log('memory (MB, first -> last):'); for (const m of out.memory.slice(0, 20)) console.log(`  ${m.name.padEnd(30)} ${m.first} -> ${m.last} (${m.delta >= 0 ? '+' : ''}${m.delta}) max ${m.max}`);
