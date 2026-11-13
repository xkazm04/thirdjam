// node agg.mjs <rawdir> [prefix]  : one table row per finished cell (n, win %, wilson, unfinished, knockdowns/match, idle, faults)
import { readdirSync, readFileSync, existsSync } from 'node:fs';
import { join } from 'node:path';
const dir = process.argv[2], pre = process.argv[3] || '';
const wil = (k, n, z = 1.96) => { if (!n) return [0, 0]; const p = k / n, d = 1 + z * z / n, c = (p + z * z / (2 * n)) / d, h = z * Math.sqrt(p * (1 - p) / n + z * z / (4 * n * n)) / d; return [100 * (c - h), 100 * (c + h)]; };
const rows = [];
for (const f of readdirSync(dir).filter((f) => f.startsWith(pre) && f.endsWith('.jsonl') && !f.endsWith('.faults.jsonl')).sort()) {
  const label = f.replace('.jsonl', ''); if (!existsSync(join(dir, `${label}.done`))) continue;
  const ms = readFileSync(join(dir, f), 'utf8').split('\n').filter(Boolean).map((l) => JSON.parse(l));
  const fl = readFileSync(join(dir, `${label}.faults.jsonl`), 'utf8').split('\n').filter(Boolean).map((l) => JSON.parse(l));
  const N = ms.length, W = ms.filter((m) => m.result === 'win').length, [lo, hi] = wil(W, N);
  const avg = (k) => ms.reduce((a, m) => a + (Number(m[k]) || 0), 0) / Math.max(1, N);
  const past = ms.filter((m) => m.over === false || (m.result !== 'win' && m.result !== 'loss'));
  const kinds = {}; for (const x of fl) kinds[x.kind] = (kinds[x.kind] || 0) + (x.count || 1);
  const mx = (k) => Math.max(0, ...ms.map((m) => m[k] || 0));
  rows.push({ label, n: N, win: +(100 * W / N).toFixed(1), lo: +lo.toFixed(1), hi: +hi.toFixed(1), unfinished: past.length, kd: +avg('knockdowns').toFixed(2), kd_all: +avg('knockdowns_all').toFixed(1),
    checks_taken: +avg('checks_taken').toFixed(2), max_no_touch_max_s: +(mx('max_no_touch') / 60).toFixed(0), no_touch_30s: ms.filter((m) => m.max_no_touch >= 1800).length, match_s: +(avg('frames') / 60).toFixed(0), faults: kinds });
}
console.log(JSON.stringify(rows));
