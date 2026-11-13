// Lane A137 (track Q3): the first second after the menu -> match frame of a bot.ps1 CSV. For every frame over 33.3 ms in the 60 rows after the load frame
// (see the pick below): its frame, game, render thread and GPU times and the render thread's
// exclusive stats, GPU passes and RenderThread waits over a floor, largest first. Ends with one summary line per run (count of frames over 33 ms, worst).
// Usage: node tools/perf/firstsec.mjs <run dir> [...] [--min 3] [--rows 60]
import fs from 'node:fs'; import path from 'node:path';
const args = process.argv.slice(2);
const opt = (k, d) => { const i = args.indexOf(k); return i < 0 ? d : Number(args[i + 1]); };
const min = opt('--min', 3), span = opt('--rows', 60);
const dirs = args.filter((a, i) => !a.startsWith('--') && !['--min', '--rows'].includes(args[i - 1]));
const split = l => { const o = []; let c = '', q = false; for (const ch of l) { if (ch === '"') q = !q; else if (ch === ',' && !q) { o.push(c); c = ''; } else c += ch; } o.push(c); return o; };
const sums = [];
for (const dir of dirs) {
  const csvDir = path.join(dir, 'csv');
  if (!fs.existsSync(csvDir)) { console.log(`${path.basename(dir)}: no csv`); continue; }
  const f = fs.readdirSync(csvDir).filter(n => n.endsWith('.csv')).sort()[0];
  const L = fs.readFileSync(path.join(csvDir, f), 'utf8').split(/\r?\n/).filter(Boolean); const h = split(L[0]);
  const rows = L.slice(1).map(split).filter(c => c.length >= h.length - 1 && c[0] !== h[0] && !(c[0] || '').startsWith('['));
  const col = n => h.indexOf(n);
  const gt = col('GameThreadTime'), ft = col('FrameTime'), rt = col('RenderThreadTime'), gpu = col('GPUTime');
  // The load frame: the longest game thread among the first 30 rows (bot.ps1 starts the CSV a moment before 'open'), so a later stall in play is never taken
  // for it (startframes.mjs / loadrow.mjs take the longest of the whole capture). The next row (+1) is the first rendered match frame, a frame of its own.
  let li = 0; rows.slice(0, 30).forEach((r, i) => { if (+r[gt] > +rows[li][gt]) li = i; });
  const loadMs = +rows[li][ft] || 0;
  const flushCol = col('Exclusive/GameThread/FlushAsyncLoading');
  const flushMs = Math.max(+rows[li][flushCol] || 0, +(rows[Math.max(0, li - 1)] || [])[flushCol] || 0);
  const hit = [];
  // The PSO cost of the start (engine CSV stats): graphics PSO hitch time and precache misses, summed over the load frame and the next rows.
  let psoHitch = 0, psoMiss = 0, psoCMiss = 0;
  for (let i = Math.max(0, li - 1); i <= Math.min(rows.length - 1, li + span); i++) {
    psoHitch += +rows[i][col('PSO/GraphicsPSOHitchTime')] || 0; psoMiss += +rows[i][col('PSO/PSOMisses')] || 0; psoCMiss += +rows[i][col('PSO/PSOComputeMisses')] || 0;
  }
  for (let i = li + 1; i <= Math.min(rows.length - 1, li + span); i++) {
    const r = rows[i]; if (+r[ft] <= 33.3) continue;
    const p = rows[i - 1];
    const items = h.map((n, k) => [n, Math.max(+r[k] || 0, +p[k] || 0)])
      .filter(([n, v]) => (n.startsWith('Exclusive/RenderThread/') || (n.startsWith('GPU/') && n !== 'GPU/Total')) && v >= min).sort((a, b) => b[1] - a[1]).slice(0, 8);
    hit.push(+(+r[ft]).toFixed(1));
    console.log(`  ${path.basename(dir)} +${i - li} frame ${(+r[ft]).toFixed(1)} GT ${(+r[gt]).toFixed(1)} RT ${(+r[rt]).toFixed(1)} GPU ${(+r[gpu]).toFixed(1)}: ` +
      items.map(([n, v]) => `${n.replace('Exclusive/RenderThread/', 'RT/')} ${v.toFixed(1)}`).join(', '));
  }
  // From game.log: the light hold's blocking rig load (track U1, HokejGameMode.cpp) and the PSO precache misses from the match start on (engine log lines).
  let hold = '', miss = 0, missFirst = 0, goals = 0;
  const log = path.join(dir, 'game.log');
  if (fs.existsSync(log)) {
    const G = fs.readFileSync(log, 'utf8').split(/\r?\n/);
    const m = G.map(l => l.match(/JAMSCREEN light hold started.*blocking load (-?[\d.]+) ms/)).find(Boolean);
    hold = m ? m[1] : (G.some(l => l.includes('JAMSCREEN light hold not started')) ? 'not started' : '?');
    const fr = l => { const x = l.match(/^\[[^\]]*\]\[\s*(\d+)\]/); return x ? +x[1] : -1; };
    const start = G.find(l => l.includes('JAMPRELOAD match start')); const f0 = start ? fr(start) : -1;
    for (const l of G) { if (!l.includes('PSOPrecacheState: Missed')) continue; const f = fr(l); if (f0 >= 0 && f >= f0) { miss++; if (f <= f0 + 70) missFirst++; } }
    goals = G.filter(l => l.includes('JAMRINK react goal')).length;
  }
  sums.push(`${path.basename(dir)} hold ${hold} ms; load ${loadMs.toFixed(1)} GT ${(+rows[li][gt]).toFixed(1)} flush ${flushMs.toFixed(1)}; first ${span}: ${hit.length} over 33 ms${hit.length ? ' (worst ' + Math.max(...hit) + ')' : ''}; PSO hitch ${psoHitch.toFixed(0)} ms, misses ${psoMiss} graphics ${psoCMiss} compute (csv); log misses ${missFirst} in the first 70 frames, ${miss} in the match; goal reactions ${goals}`);
}
for (const s of sums) console.log(s);
