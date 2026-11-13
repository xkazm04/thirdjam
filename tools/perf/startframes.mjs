// Lane A65: the match start in a CSV capture of tools/perf/start.ps1 (the CSV runs from the menu through the menu -> match frame to sim frame 600).
//   load frame   the row with the longest game thread (the menu -> match frame): FrameTime, game thread, its FlushAsyncLoading and EventWait (the audio flush
//                waits there), the game thread's Audio
//   first lit    the next 60 rows: every frame over 33.3 ms with its render thread, ShadowInitDynamic (the VSM pool commit), GPU and TSR
//   steady       the rows from 120 after the load frame on: mean GPU, GPU/TemporalSuperResolution, GameThreadTime
// Usage: node tools/perf/startframes.mjs <run dir> [...]   (reads <run>/csv/*.csv)   [--json out.json]
import fs from 'node:fs'; import path from 'node:path';
const args = process.argv.slice(2);
const jsonOut = args.includes('--json') ? args[args.indexOf('--json') + 1] : null;
const dirs = args.filter((a, i) => !a.startsWith('--') && args[i - 1] !== '--json');
const split = l => { const out = []; let cur = '', q = false; for (const ch of l) { if (ch === '"') q = !q; else if (ch === ',' && !q) { out.push(cur); cur = ''; } else cur += ch; } out.push(cur); return out; };
const r2 = v => v == null || !Number.isFinite(v) ? null : Math.round(v * 100) / 100;
const all = [];
for (const dir of dirs) {
  const csvDir = path.join(dir, 'csv');
  const f = fs.readdirSync(csvDir).filter(n => n.endsWith('.csv')).sort()[0];
  const lines = fs.readFileSync(path.join(csvDir, f), 'utf8').split(/\r?\n/).filter(Boolean);
  const h = split(lines[0]);
  const rows = lines.slice(1).map(split).filter(c => !(c[0] || '').startsWith('[') && c[0] !== h[0] && c.length >= h.length - 1);
  const v = (row, name) => { const i = h.indexOf(name); return i < 0 ? null : Number(row[i]); };
  let li = 0; rows.forEach((r, i) => { if (v(r, 'GameThreadTime') > v(rows[li], 'GameThreadTime')) li = i; });
  const L = rows[li];
  // The CSV writes a frame's exclusive game-thread stats one row before its FrameTime / GameThreadTime: the larger of the two rows is the frame's.
  const both = name => r2(Math.max(v(L, name) || 0, li > 0 ? v(rows[li - 1], name) || 0 : 0));
  const load = { row: li, frame_ms: r2(v(L, 'FrameTime')), game_ms: r2(v(L, 'GameThreadTime')), flush_async_ms: both('Exclusive/GameThread/FlushAsyncLoading'),
    engine_tick_misc_ms: both('Exclusive/GameThread/EngineTickMisc'), audio_gt_ms: both('Exclusive/GameThread/Audio') };
  const win = rows.slice(li + 1, li + 61);
  const wmax = name => r2(Math.max(0, ...win.map(r => v(r, name)).filter(Number.isFinite)));
  const lit = [];
  for (let i = li + 1; i < Math.min(rows.length, li + 61); i++) {
    const r = rows[i]; if (v(r, 'FrameTime') <= 33.34) continue;
    lit.push({ row: i, frame_ms: r2(v(r, 'FrameTime')), game_ms: r2(v(r, 'GameThreadTime')), render_ms: r2(v(r, 'RenderThreadTime')), gpu_ms: r2(v(r, 'GPUTime')),
      shadow_init_ms: r2(v(r, 'Exclusive/RenderThread/ShadowInitDynamic')), tsr_ms: r2(v(r, 'GPU/TemporalSuperResolution')) });
  }
  const steadyRows = rows.slice(li + 120);
  const mean = name => { const a = steadyRows.map(r => v(r, name)).filter(Number.isFinite); return a.length ? r2(a.reduce((s, x) => s + x, 0) / a.length) : null; };
  const res = { run: path.basename(dir), load, first_lit_hitches: lit, first_lit_sum_ms: r2(lit.reduce((s, x) => s + x.frame_ms, 0)), first_lit_max_ms: lit.length ? Math.max(...lit.map(x => x.frame_ms)) : 0,
    first_lit_window: { shadow_init_max_ms: wmax('Exclusive/RenderThread/ShadowInitDynamic'), render_max_ms: wmax('RenderThreadTime'), gpu_max_ms: wmax('GPUTime'), tsr_max_ms: wmax('GPU/TemporalSuperResolution') },
    steady: { rows: steadyRows.length, gpu_ms: mean('GPUTime'), tsr_ms: mean('GPU/TemporalSuperResolution'), game_ms: mean('GameThreadTime'), render_ms: mean('RenderThreadTime'), physical_mb: mean('PhysicalUsedMB') } };
  all.push(res);
  console.log(`${res.run.padEnd(22)} load frame ${load.frame_ms} ms (GT ${load.game_ms}, FlushAsyncLoading ${load.flush_async_ms}, EngineTickMisc ${load.engine_tick_misc_ms})  ` +
    `first 60 frames: hitches ${lit.map(x => x.frame_ms).join(', ') || 'none'}; max ShadowInitDynamic ${res.first_lit_window.shadow_init_max_ms}, RT ${res.first_lit_window.render_max_ms}, GPU ${res.first_lit_window.gpu_max_ms}, TSR ${res.first_lit_window.tsr_max_ms}  ` +
    `steady GPU ${res.steady.gpu_ms} TSR ${res.steady.tsr_ms} GT ${res.steady.game_ms} RT ${res.steady.render_ms}`);
}
if (jsonOut) fs.writeFileSync(jsonOut, JSON.stringify(all, null, 1));
