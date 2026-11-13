// usage: node agg.mjs <log...>   per-log means of the playtest line (lane A115)
import { readFileSync } from 'node:fs';
for (const f of process.argv.slice(2)) {
  const rows = [];
  for (const l of readFileSync(f, 'utf8').split(/\r?\n/)) { const i = l.indexOf('{"tool":"thirdjam.playtest/1"'); if (i >= 0) { try { rows.push(JSON.parse(l.slice(i))); } catch {} } }
  const n = rows.length; if (!n) { console.log(f, 'no rows'); continue; }
  const mean = (k) => (rows.reduce((s, r) => s + (Number(r[k]) || 0), 0) / n);
  const keys = process.env.KEYS ? process.env.KEYS.split(',') : ['frames', 'bot_rev_brakes', 'rev_brakes', 'bot_carry_sprint_frames', 'passes_received', 'passes', 'one_timers', 'knockdowns', 'knockdowns_all', 'max_no_touch', 'touches'];
  const wins = rows.filter((r) => r.result === 'win' || r.winner === 'player' || r.won === true || r.win === 1 || r.win === true).length;
  const idle30 = rows.filter((r) => (Number(r.max_no_touch) || 0) >= 1800).length;
  console.log(f.split(/[\/]/).pop(), `n=${n} win=${(100 * wins / n).toFixed(1)}% idle30=${idle30}`, keys.map((k) => `${k}=${mean(k).toFixed(1)}`).join(' '));
}
if (process.env.SHOW) { const l = readFileSync(process.argv[2], 'utf8').split(/\r?\n/).find((x) => x.includes('{"tool":"thirdjam.playtest/1"')); console.log(l.slice(l.indexOf('{'))); }
