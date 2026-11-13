// Lane A106 (track Q3): the 5 v 5 budget row of a bot.ps1 capture. The CSV runs from the menu: the load frame is the row with the longest game thread
// (as startframes.mjs); play = the rows from 120 after it to the end. Prints frame / game / render / GPU mean and p99, hitches > 33.3 ms in play,
// the brain (Jam/GameThread/BrainThink), draw calls, primitives, and the top exclusive game-thread and render-thread stats.
// Usage: node tools/perf/five.mjs <run dir> [...] [--json out.json]
import fs from 'node:fs'; import path from 'node:path';
const args = process.argv.slice(2);
const jsonOut = args.includes('--json') ? args[args.indexOf('--json') + 1] : null;
const dirs = args.filter((a, i) => !a.startsWith('--') && args[i - 1] !== '--json');
const split = l => { const out = []; let cur = '', q = false; for (const ch of l) { if (ch === '"') q = !q; else if (ch === ',' && !q) { out.push(cur); cur = ''; } else cur += ch; } out.push(cur); return out; };
const r3 = v => v == null || !Number.isFinite(v) ? null : Math.round(v * 1000) / 1000;
const all = [];
for (const dir of dirs) {
  const csvDir = path.join(dir, 'csv');
  const f = fs.readdirSync(csvDir).filter(n => n.endsWith('.csv')).sort()[0];
  const lines = fs.readFileSync(path.join(csvDir, f), 'utf8').split(/\r?\n/).filter(Boolean);
  const h = split(lines[0]);
  const rows = lines.slice(1).map(split).filter(c => !(c[0] || '').startsWith('[') && c[0] !== h[0] && c.length >= h.length - 1);
  const col = name => h.indexOf(name);
  const v = (row, name) => { const i = col(name); return i < 0 ? NaN : Number(row[i]); };
  let li = 0; rows.forEach((r, i) => { if (v(r, 'GameThreadTime') > v(rows[li], 'GameThreadTime')) li = i; });
  const play = rows.slice(li + 120);
  const stat = name => { const a = play.map(r => v(r, name)).filter(Number.isFinite); if (!a.length) return null; const s = [...a].sort((x, y) => x - y);
    return { mean: r3(a.reduce((t, x) => t + x, 0) / a.length), p99: r3(s[Math.floor(s.length * 0.99)]), max: r3(s[s.length - 1]) }; };
  const hitches = play.map((r, i) => ({ i, ms: v(r, 'FrameTime') })).filter(x => x.ms > 33.34);
  const top = prefix => h.filter(n => n.startsWith(prefix)).map(n => ({ n: n.slice(prefix.length), m: stat(n)?.mean ?? 0 })).sort((a, b) => b.m - a.m).slice(0, 12).map(x => `${x.n} ${x.m}`);
  const res = { run: path.basename(dir), load_row: li, load_frame_ms: r3(v(rows[li], 'FrameTime')), load_game_ms: r3(v(rows[li], 'GameThreadTime')), play_rows: play.length,
    frame: stat('FrameTime'), game: stat('GameThreadTime'), render: stat('RenderThreadTime'), gpu: stat('GPUTime'), rhi: stat('RHIThreadTime'),
    brain: stat('Jam/GameThread/BrainThink'), draws: stat('RHI/DrawCalls'), prims: stat('RHI/PrimitivesDrawn'), hitches_33: hitches.length,
    hitch_list: hitches.slice(0, 10).map(x => `${x.i}:${r3(x.ms)}`), top_gt: top('Exclusive/GameThread/'), top_rt: top('Exclusive/RenderThread/'), top_gpu: top('GPU/') };
  all.push(res);
  console.log(JSON.stringify(res, null, 1));
}
if (jsonOut) fs.writeFileSync(jsonOut, JSON.stringify(all, null, 1));
