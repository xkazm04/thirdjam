// Lane A106: the CSV rows of the frames where the game thread loaded a package synchronously after the match start (JAMPRELOAD sync lines of
// game.log; GFrameCounter -> CSV row via the match-start frame, the row with the longest game thread). Usage: node tools/perf/syncrows.mjs <run dir>
import fs from 'node:fs'; import path from 'node:path';
const dir = process.argv[2];
const split = l => { const o = []; let c = '', q = false; for (const ch of l) { if (ch === '"') q = !q; else if (ch === ',' && !q) { o.push(c); c = ''; } else c += ch; } o.push(c); return o; };
const f = fs.readdirSync(path.join(dir, 'csv')).filter(n => n.endsWith('.csv')).sort()[0];
const L = fs.readFileSync(path.join(dir, 'csv', f), 'utf8').split(/\r?\n/).filter(Boolean); const h = split(L[0]);
const rows = L.slice(1).map(split).filter(c => c.length >= h.length - 1 && c[0] !== h[0]); const ix = n => h.indexOf(n);
let li = 0; rows.forEach((r, i) => { if (+r[ix('GameThreadTime')] > +rows[li][ix('GameThreadTime')]) li = i; });
const log = fs.readFileSync(path.join(dir, 'game.log'), 'utf8');
const start = Number((log.match(/JAMPRELOAD sync \/Game\/Maps\/Rink frame=(\d+)/) || [])[1]);
const seen = new Set();
for (const m of log.matchAll(/JAMPRELOAD sync (\S+) frame=(\d+)/g)) {
  const fr = Number(m[2]); if (fr <= start + 60 || seen.has(m[1])) continue; seen.add(m[1]);
  const at = li + (fr - start); const win = [-1, 0, 1].map(d => rows[at + d]).filter(Boolean);
  const worst = win.reduce((a, r) => (+r[ix('GameThreadTime')] > +a[ix('GameThreadTime')] ? r : a), win[0]);
  console.log(`${m[1]} frame=${fr} row=${at} frame_ms=${(+worst[ix('FrameTime')]).toFixed(1)} game_ms=${(+worst[ix('GameThreadTime')]).toFixed(1)}`);
}
