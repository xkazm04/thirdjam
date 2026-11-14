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
// The match start: the Rink map's own sync load (before lane A114), else the "JAMPRELOAD match start ... frame=" line (A114: the map is preloaded).
const start = Number((log.match(/JAMPRELOAD sync \/Game\/Maps\/Rink frame=(\d+)/) || log.match(/JAMPRELOAD match start:.* frame=(\d+)/) || [])[1]);
// Lane A150: the skip was `fr <= start + 60`, which hid the match-start frame itself (the loads ARinkActor's build asks for). Now every load from the
// match-start frame on is listed; those within 60 frames of it are counted apart as the match start's.
let n = 0, total = 0, atStart = 0;
const seen = new Set();
for (const m of log.matchAll(/JAMPRELOAD sync (\S+) frame=(\d+)/g)) {
  const fr = Number(m[2]); if (!(fr >= start)) continue; total++; if (fr <= start + 60) atStart++; if (seen.has(m[1])) continue; seen.add(m[1]);
  const at = li + (fr - start); const win = [-1, 0, 1].map(d => rows[at + d]).filter(Boolean);
  const worst = win.reduce((a, r) => (+r[ix('GameThreadTime')] > +a[ix('GameThreadTime')] ? r : a), win[0]);
  n++; console.log(`${fr <= start + 60 ? 'START ' : ''}${m[1]} frame=${fr} row=${at} frame_ms=${(+worst[ix('FrameTime')]).toFixed(1)} game_ms=${(+worst[ix('GameThreadTime')]).toFixed(1)}`);
}
console.log(`sync loads from the match start: ${total} (${n} packages; ${atStart} in the match start's first 60 frames; match start frame ${start})`);
