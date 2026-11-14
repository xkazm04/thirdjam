// Lane A150 (track Q3 evolution 5): the arena's reactions in the CSV of a bot.ps1 run. Every `JAMRINK react goal|board|rattle` line of game.log is placed
// on its CSV row (the log's [frame % 1000] and timestamp against the `JAMPRELOAD match start ... frame=N` line, the load frame = the longest game thread
// among the first 30 rows, as firstsec.mjs), and the rows from the reaction to 10 after it are read: the worst frame, the GPU total and the GPU stats of
// those rows against the play's median of the same stat. Usage: node tools/perf/reactrows.mjs <run dir> [...] [--rows 10]
import fs from 'node:fs'; import path from 'node:path';
const args = process.argv.slice(2);
const span = args.includes('--rows') ? +args[args.indexOf('--rows') + 1] : 10;
const dirs = args.filter((a, i) => !a.startsWith('--') && args[i - 1] !== '--rows');
const split = l => { const o = []; let c = '', q = false; for (const ch of l) { if (ch === '"') q = !q; else if (ch === ',' && !q) { o.push(c); c = ''; } else c += ch; } o.push(c); return o; };
const med = a => { const s = [...a].sort((x, y) => x - y); return s.length ? s[Math.floor(s.length / 2)] : NaN; };
const ts = s => { const m = s.match(/(\d+)\.(\d+)\.(\d+)-(\d+)\.(\d+)\.(\d+):(\d+)/); return m ? Date.UTC(+m[1], +m[2] - 1, +m[3], +m[4], +m[5], +m[6], +m[7]) : NaN; };
const all = { goal: [], board: [], rattle: [] };
let winAll = 0, winOver = 0;   // every window of the same length in play: the base rate of a frame over 33 ms (the machine's own hitches)
for (const dir of dirs) {
  const csvDir = path.join(dir, 'csv');
  if (!fs.existsSync(csvDir)) { console.log(`${path.basename(dir)}: no csv`); continue; }
  const f = fs.readdirSync(csvDir).filter(n => n.endsWith('.csv')).sort()[0];
  const L = fs.readFileSync(path.join(csvDir, f), 'utf8').split(/\r?\n/).filter(Boolean); const h = split(L[0]);
  const rows = L.slice(1).map(split).filter(c => c.length >= h.length - 1 && c[0] !== h[0]); const ix = n => h.indexOf(n);
  const ft = ix('FrameTime'), gt = ix('GameThreadTime'), gpu = ix('GPUTime');
  const gpuCols = h.map((n, i) => [n, i]).filter(([n]) => n.startsWith('GPU/'));
  let li = 0; for (let i = 0; i < Math.min(30, rows.length); i++) if (+rows[i][gt] > +rows[li][gt]) li = i;
  const log = fs.readFileSync(path.join(dir, 'game.log'), 'utf8').split(/\r?\n/);
  const startLine = log.find(l => /JAMPRELOAD match start:.* frame=\d+/.test(l));
  if (!startLine) { console.log(`${path.basename(dir)}: no match start line`); continue; }
  const start = +startLine.match(/frame=(\d+)/)[1], t0 = ts(startLine);
  const play = rows.slice(li + 120);
  const base = { frame: med(play.map(r => +r[ft])), gpu: med(play.map(r => +r[gpu])) };
  for (let i = 0; i + span < play.length; i++) { winAll++; if (play.slice(i, i + span + 1).some(r => +r[ft] > 33.3)) winOver++; }
  const baseCol = Object.fromEntries(gpuCols.map(([n, i]) => [n, med(play.map(r => +r[i] || 0))]));
  for (const l of log) {
    const m = l.match(/\]\[\s*(\d+)\]LogJamSim: JAMRINK react (goal|board|rattle)/); if (!m) continue;
    const est = start + (ts(l) - t0) * 0.06;   // 60 frames a second, for the thousands only
    let F = Math.floor(est / 1000) * 1000 + +m[1]; for (const c of [F - 1000, F + 1000]) if (Math.abs(c - est) < Math.abs(F - est)) F = c;
    const at = li + (F - start); const win = rows.slice(at, at + span + 1); if (!win.length) continue;
    const worst = Math.max(...win.map(r => +r[ft])), gpuMax = Math.max(...win.map(r => +r[gpu]));
    const extra = gpuCols.map(([n, i]) => [n, Math.max(...win.map(r => +r[i] || 0)) - baseCol[n]]).filter(([, d]) => d > 0.3).sort((a, b) => b[1] - a[1]).slice(0, 4);
    all[m[2]].push({ run: path.basename(dir), worst, gpuMax, gpuBase: base.gpu });
    console.log(`  ${path.basename(dir)} ${m[2]} row ${at}: worst frame ${worst.toFixed(1)} (play median ${base.frame.toFixed(1)}), GPU max ${gpuMax.toFixed(1)} (median ${base.gpu.toFixed(1)})${extra.length ? '; GPU stats over median: ' + extra.map(([n, d]) => `${n} +${d.toFixed(2)}`).join(', ') : ''}`);
  }
}
console.log(`base rate: ${winAll ? (100 * winOver / winAll).toFixed(1) : '-'} % of all ${span + 1}-row windows in play hold a frame over 33 ms`);
for (const [k, v] of Object.entries(all)) if (v.length) {
  const over = v.filter(x => x.worst > 33.3).length;
  console.log(`${k}: ${v.length} reactions; worst frame in the ${span} rows after: median ${med(v.map(x => x.worst)).toFixed(1)}, max ${Math.max(...v.map(x => x.worst)).toFixed(1)}, over 33 ms ${over} (${(100 * over / v.length).toFixed(1)} %); GPU max - play median: median ${med(v.map(x => x.gpuMax - x.gpuBase)).toFixed(2)} ms`);
}
