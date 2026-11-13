// Lane A114: the menu -> match frame (the row with the longest game thread) of a bot.ps1 CSV, its exclusive game-thread stats over 2 ms, largest first.
// Usage: node tools/perf/loadrow.mjs <run dir> [min ms]
import fs from 'node:fs'; import path from 'node:path';
const dir = process.argv[2]; const min = Number(process.argv[3] || 2);
const split = l => { const o = []; let c = '', q = false; for (const ch of l) { if (ch === '"') q = !q; else if (ch === ',' && !q) { o.push(c); c = ''; } else c += ch; } o.push(c); return o; };
const f = fs.readdirSync(path.join(dir, 'csv')).filter(n => n.endsWith('.csv')).sort()[0];
const L = fs.readFileSync(path.join(dir, 'csv', f), 'utf8').split(/\r?\n/).filter(Boolean); const h = split(L[0]);
const rows = L.slice(1).map(split).filter(c => c.length >= h.length - 1 && c[0] !== h[0] && !(c[0] || '').startsWith('['));
const gt = h.indexOf('GameThreadTime'); let li = 0; rows.forEach((r, i) => { if (+r[gt] > +rows[li][gt]) li = i; });
// The CSV writes a frame's exclusive game-thread stats one row before its FrameTime / GameThreadTime: the larger of the two rows is the frame's.
const r = rows[li], p = rows[Math.max(0, li - 1)];
const items = h.map((n, i) => [n, Math.max(+r[i] || 0, +p[i] || 0)]).filter(([n, v]) => n.startsWith('Exclusive/GameThread/') && v >= min).sort((a, b) => b[1] - a[1]);
console.log(`${path.basename(dir)} row ${li} frame ${(+r[h.indexOf('FrameTime')]).toFixed(1)} GT ${(+r[gt]).toFixed(1)}: ` + items.map(([n, v]) => `${n.slice(21)} ${v.toFixed(1)}`).join(', '));
