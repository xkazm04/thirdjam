// Lane A161 (track Q3 evolution 6): the cost of a set of actors toggled inside one match (bot.ps1 -ToggleFile: device B's stands and clay crowd on
// LR_P3_Clay, hidden and shown every -ToggleSec seconds). Paired in time by construction: the two states alternate in the same match and minute.
// Each CSV row of play is put in a state by its RHI/PrimitivesDrawn (the actors' triangles leave the count when hidden: two clear levels); rows within
// --guard rows of a state change are dropped. Per state: medians and p95 of frame, game thread, render thread, GPU, draw calls, triangles; then the
// ratio shown / hidden per adjacent pair of segments (median of the pairs), and the rows after each JAMRINK react goal / board line by state.
// Usage: node tools/perf/stands.mjs <run dir> [...] [--guard 30] [--json out.json]
import fs from 'node:fs'; import path from 'node:path';
const args = process.argv.slice(2);
const opt = (k, d) => { const i = args.indexOf(k); return i >= 0 ? args[i + 1] : d; };
const guard = +opt('--guard', 30), jsonOut = opt('--json', ''), by = opt('--by', 'triangles');   // --by time: states from events.log TOGGLE lines (a cvar toggle)
const dirs = args.filter((a, i) => !a.startsWith('--') && !['--guard', '--json', '--by'].includes(args[i - 1]));
const split = l => { const o = []; let c = '', q = false; for (const ch of l) { if (ch === '"') q = !q; else if (ch === ',' && !q) { o.push(c); c = ''; } else c += ch; } o.push(c); return o; };
const q = (a, p) => { const s = [...a].sort((x, y) => x - y); return s.length ? s[Math.min(s.length - 1, Math.floor(p * s.length))] : NaN; };
const med = a => q(a, 0.5);
const ts = s => { const m = s.match(/(\d+)\.(\d+)\.(\d+)-(\d+)\.(\d+)\.(\d+):(\d+)/); return m ? Date.UTC(+m[1], +m[2] - 1, +m[3], +m[4], +m[5], +m[6], +m[7]) : NaN; };
const stats = ['FrameTime', 'GameThreadTime', 'RenderThreadTime', 'GPUTime', 'GPU/ShadowDepths', 'GPU/Basepass', 'GPU/Lights', 'GPU/RenderDeferredLighting', 'RHI/DrawCalls', 'RHI/PrimitivesDrawn'];
const out = { runs: [] };
const pool = { shown: {}, hidden: {} }, pairs = {}, events = { goal: { shown: [], hidden: [] }, board: { shown: [], hidden: [] } };
for (const s of stats) { pool.shown[s] = []; pool.hidden[s] = []; pairs[s] = []; }
for (const dir of dirs) {
  const csvDir = path.join(dir, 'csv');
  if (!fs.existsSync(csvDir)) { console.log(`${path.basename(dir)}: no csv`); continue; }
  const f = fs.readdirSync(csvDir).filter(n => n.endsWith('.csv')).sort()[0];
  const L = fs.readFileSync(path.join(csvDir, f), 'utf8').split(/\r?\n/).filter(Boolean); const h = split(L[0]);
  const rows = L.slice(1).map(split).filter(c => c.length >= h.length - 1 && c[0] !== h[0]); const ix = n => h.indexOf(n);
  const I = Object.fromEntries(stats.map(s => [s, ix(s)]));
  if (I['RHI/PrimitivesDrawn'] < 0) { console.log(`${path.basename(dir)}: no RHI/PrimitivesDrawn column`); continue; }
  let li = 0; for (let i = 0; i < Math.min(4000, rows.length); i++) if (+rows[i][I.GameThreadTime] > +rows[li][I.GameThreadTime]) li = i;   // the match load frame (the CSV starts at the menu)
  const from = li + 600;   // the toggles start at sim frame 600
  const prims = rows.map(r => +r[I['RHI/PrimitivesDrawn']]);
  const playPrims = prims.slice(from);
  const lo = q(playPrims, 0.1), hi = q(playPrims, 0.9), cut = (lo + hi) / 2;
  let state = prims.map((p, i) => i < from ? null : p > cut ? 'shown' : 'hidden');
  if (by === 'time') {
    // events.log is local time, game.log UTC: the offset from the JAMCART line both carry; rows from the load row by cumulative FrameTime.
    const ev = fs.readFileSync(path.join(dir, 'events.log'), 'utf8').split(/\r?\n/);
    const glog = fs.readFileSync(path.join(dir, 'game.log'), 'utf8').split(/\r?\n/);
    const hms = l => { const m = l.match(/^(\d+):(\d+):(\d+)\.(\d+)/); return m ? ((+m[1] * 60 + +m[2]) * 60 + +m[3]) * 1000 + +m[4] : NaN; };
    const utcMs = l => { const t = ts(l); return Number.isFinite(t) ? t % 86400000 : NaN; };
    const evCart = ev.find(l => l.includes('JAMCART loaded seen')), gCart = glog.find(l => l.includes('JAMCART loaded'));
    const off = Math.round((hms(evCart) - utcMs(gCart)) / 900000) * 900000;
    const gStart = glog.find(l => /JAMPRELOAD match start:.* frame=\d+/.test(l));
    const tLoad = utcMs(gStart) + off;
    const cum = []; let acc = 0; for (let i = 0; i < rows.length; i++) { cum.push(acc); if (i >= li) acc += +rows[i][I.FrameTime]; }
    const rowAt = t => { const d = t - tLoad; let i = li; while (i < rows.length && cum[i] < d) i++; return i; };
    const tg = ev.filter(l => / TOGGLE (hide|show) /.test(l)).map(l => ({ st: / TOGGLE hide /.test(l) ? 'hidden' : 'shown', row: rowAt(hms(l)) }));
    state = rows.map(() => null);
    tg.forEach((g, k) => { const end = k + 1 < tg.length ? tg[k + 1].row : rows.length; for (let i = g.row; i < end; i++) state[i] = g.st; });
  }
  // Segments: runs of one state; drop guard rows at each edge.
  const segs = []; let s0 = from; while (s0 < rows.length && !state[s0]) s0++;
  for (let i = from + 1; i <= rows.length; i++) if (i === rows.length || state[i] !== state[s0]) { segs.push({ st: state[s0], a: s0, b: i }); s0 = i; }
  const keep = segs.filter(g => g.b - g.a > 2 * guard + 30).map(g => ({ ...g, a: g.a + guard, b: g.b - guard }));
  const segMed = keep.map(g => ({ st: g.st, n: g.b - g.a, m: Object.fromEntries(stats.map(s => [s, med(rows.slice(g.a, g.b).map(r => +r[I[s]]))])) }));
  for (const g of keep) for (const s of stats) for (const r of rows.slice(g.a, g.b)) pool[g.st][s].push(+r[I[s]]);
  for (let k = 0; k + 1 < segMed.length; k++) {
    const A = segMed[k], B = segMed[k + 1]; if (A.st === B.st) continue;
    const sh = A.st === 'shown' ? A : B, hd = A.st === 'shown' ? B : A;
    for (const s of stats) pairs[s].push(sh.m[s] / hd.m[s]);
  }
  // Goal and board-hit rows by state.
  const log = fs.existsSync(path.join(dir, 'game.log')) ? fs.readFileSync(path.join(dir, 'game.log'), 'utf8').split(/\r?\n/) : [];
  const startLine = log.find(l => /JAMPRELOAD match start:.* frame=\d+/.test(l));
  if (startLine) {
    const start = +startLine.match(/frame=(\d+)/)[1], t0 = ts(startLine);
    for (const l of log) {
      const m = l.match(/\]\[\s*(\d+)\]LogJamSim: JAMRINK react (goal|board)/); if (!m) continue;
      const est = start + (ts(l) - t0) * 0.06;
      let F = Math.floor(est / 1000) * 1000 + +m[1]; for (const c of [F - 1000, F + 1000]) if (Math.abs(c - est) < Math.abs(F - est)) F = c;
      const at = li + (F - start); if (at < from || at + 11 > rows.length) continue;
      const st = state[at]; if (state[at + 10] !== st) continue;
      const win = rows.slice(at, at + 11);
      events[m[2]][st].push({ frame: Math.max(...win.map(r => +r[I.FrameTime])), gt: med(win.map(r => +r[I.GameThreadTime])), gpu: med(win.map(r => +r[I.GPUTime])) });
    }
  }
  const nS = keep.filter(g => g.st === 'shown').length, nH = keep.filter(g => g.st === 'hidden').length;
  console.log(`${path.basename(dir)}: ${rows.length} rows, load row ${li}, triangles cut at ${cut.toFixed(0)} (p10 ${lo.toFixed(0)}, p90 ${hi.toFixed(0)}), segments kept shown ${nS} / hidden ${nH}`);
  out.runs.push({ run: path.basename(dir), rows: rows.length, li, cut, segMed });
}
const fmt = v => Number.isFinite(v) ? (Math.abs(v) >= 1000 ? v.toFixed(0) : v.toFixed(2)) : '-';
console.log(`\nper state (rows pooled over runs; median / p95):`);
console.log(`stat | shown (stands and crowd on) | hidden | ratio shown/hidden: median of paired adjacent segments (pairs)`);
const table = {};
for (const s of stats) {
  const S = pool.shown[s], H = pool.hidden[s];
  table[s] = { shownMed: med(S), shownP95: q(S, 0.95), hiddenMed: med(H), hiddenP95: q(H, 0.95), pairRatio: med(pairs[s]), pairs: pairs[s].length, n: [S.length, H.length] };
  console.log(`${s} | ${fmt(med(S))} / ${fmt(q(S, 0.95))} | ${fmt(med(H))} / ${fmt(q(H, 0.95))} | ${fmt(med(pairs[s]))} (${pairs[s].length}; range ${fmt(Math.min(...pairs[s]))}-${fmt(Math.max(...pairs[s]))})`);
}
for (const [k, v] of Object.entries(events)) for (const st of ['shown', 'hidden']) if (v[st].length)
  console.log(`${k} ${st}: ${v[st].length}; worst frame in the 10 rows after: median ${fmt(med(v[st].map(x => x.frame)))}; GT median ${fmt(med(v[st].map(x => x.gt)))}; GPU median ${fmt(med(v[st].map(x => x.gpu)))}`);
if (jsonOut) fs.writeFileSync(jsonOut, JSON.stringify({ ...out, table, events }, null, 1));
