// Lane A175 (track Q3, evolution 7): goals paired across arms. A deterministic cut (tools/perf/cuts/perf.p3_clay_goals) scores on the same sim
// frames in every run, so each goal of one arm has a twin in the other: the rows from the goal's frame to --rows after it (each JAMRINK react goal
// line placed on its CSV row as reactrows.mjs does) are compared goal by goal. Arms are named by the part of the run directory before -<round>.
// Usage: node tools/perf/goalpair.mjs <run dir> [...] [--rows 10] [--base <arm>]
import fs from 'node:fs'; import path from 'node:path';
const args = process.argv.slice(2);
const opt = (k, d) => { const i = args.indexOf(k); return i >= 0 ? args[i + 1] : d; };
const span = +opt('--rows', 10), baseArm = opt('--base', '');
const dirs = args.filter((a, i) => !a.startsWith('--') && !['--rows', '--base'].includes(args[i - 1]));
const split = l => { const o = []; let c = '', q = false; for (const ch of l) { if (ch === '"') q = !q; else if (ch === ',' && !q) { o.push(c); c = ''; } else c += ch; } o.push(c); return o; };
const med = a => { const s = [...a].sort((x, y) => x - y); return s.length ? s[Math.floor(s.length / 2)] : NaN; };
const ts = s => { const m = s.match(/(\d+)\.(\d+)\.(\d+)-(\d+)\.(\d+)\.(\d+):(\d+)/); return m ? Date.UTC(+m[1], +m[2] - 1, +m[3], +m[4], +m[5], +m[6], +m[7]) : NaN; };
const stats = ['FrameTime', 'GameThreadTime', 'RenderThreadTime', 'GPUTime', 'GPU/Lights', 'GPU/Basepass', 'GPU/Translucency', 'GPU/RenderDeferredLighting'];
const goals = {};   // arm -> round -> sim frame -> {stat: [worst, median]}
for (const dir of dirs) {
  const name = path.basename(dir); const m = name.match(/^(.*)-(\d+)$/); const arm = m ? m[1] : name, round = m ? m[2] : '1';
  const csvDir = path.join(dir, 'csv'); if (!fs.existsSync(csvDir)) { console.log(`${name}: no csv`); continue; }
  const f = fs.readdirSync(csvDir).filter(n => n.endsWith('.csv')).sort()[0];
  const L = fs.readFileSync(path.join(csvDir, f), 'utf8').split(/\r?\n/).filter(Boolean); const h = split(L[0]);
  const rows = L.slice(1).map(split).filter(c => c.length >= h.length - 1 && c[0] !== h[0]);
  const I = Object.fromEntries(stats.map(s => [s, h.indexOf(s)]));
  let li = 0; for (let i = 0; i < Math.min(4000, rows.length); i++) if (+rows[i][I.GameThreadTime] > +rows[li][I.GameThreadTime]) li = i;
  const log = fs.readFileSync(path.join(dir, 'game.log'), 'utf8').split(/\r?\n/);
  const startLine = log.find(l => /JAMPRELOAD match start:.* frame=\d+/.test(l)); if (!startLine) { console.log(`${name}: no match start`); continue; }
  const start = +startLine.match(/frame=(\d+)/)[1], t0 = ts(startLine);
  for (const l of log) {
    const g = l.match(/\]\[\s*(\d+)\]LogJamSim: JAMRINK react goal .*frame=(\d+)/); if (!g) continue;
    const est = start + (ts(l) - t0) * 0.06;
    let F = Math.floor(est / 1000) * 1000 + +g[1]; for (const c of [F - 1000, F + 1000]) if (Math.abs(c - est) < Math.abs(F - est)) F = c;
    const at = li + (F - start); const win = rows.slice(at, at + span + 1); if (win.length < span + 1) continue;
    const v = Object.fromEntries(stats.filter(s => I[s] >= 0).map(s => [s, [Math.max(...win.map(r => +r[I[s]] || 0)), med(win.map(r => +r[I[s]] || 0))]]));
    ((goals[arm] ??= {})[round] ??= {})[g[2]] = v;
  }
}
const arms = Object.keys(goals).sort();
for (const a of arms) console.log(`${a}: ${Object.values(goals[a]).reduce((n, r) => n + Object.keys(r).length, 0)} goals in ${Object.keys(goals[a]).length} runs`);
const base = baseArm || arms[0];
for (const a of arms) if (a !== base) {
  console.log(`\n${a} against ${base}: goal by goal (same round, same sim frame); medians over the pairs`);
  for (const s of stats) {
    const ratios = [], dw = [], dm = [];
    for (const [r, gs] of Object.entries(goals[a])) for (const [fr, v] of Object.entries(gs)) {
      const b = goals[base]?.[r]?.[fr]; if (!b || !v[s] || !b[s]) continue;
      ratios.push(v[s][1] / b[s][1]); dw.push(v[s][0] - b[s][0]); dm.push(v[s][1] - b[s][1]);
    }
    if (ratios.length) console.log(`  ${s}: median ratio ${med(ratios).toFixed(2)} (${ratios.length} goals, ${Math.min(...ratios).toFixed(2)}-${Math.max(...ratios).toFixed(2)}); median delta ${med(dm).toFixed(2)} ms, worst-row delta ${med(dw).toFixed(2)} ms`);
  }
}
console.log('\nper arm, all goals: worst frame in the window (median / max), GPU median, game thread median');
for (const a of arms) {
  const all = Object.values(goals[a]).flatMap(r => Object.values(r));
  const w = all.map(v => v.FrameTime[0]);
  console.log(`  ${a}: worst frame ${med(w).toFixed(1)} / ${Math.max(...w).toFixed(1)}, over 33 ms ${w.filter(x => x > 33.3).length} of ${w.length}; GPU ${med(all.map(v => v.GPUTime[1])).toFixed(2)} (window max median ${med(all.map(v => v.GPUTime[0])).toFixed(2)}); GT ${med(all.map(v => v.GameThreadTime[1])).toFixed(2)}`);
}
