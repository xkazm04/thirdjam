#!/usr/bin/env node
// Lane A133: the waiting-room read-outs of a cell's raw matches: share with a 30 s stretch without a human touch (max_no_touch >= 1800), hold seconds, idle seconds
import { readFileSync } from 'node:fs';
for (const label of process.argv.slice(2)) {
  const ms = readFileSync(new URL(`raw/${label}.jsonl`, import.meta.url), 'utf8').trim().split('\n').map(JSON.parse);
  const n = ms.length, avg = (f) => (ms.reduce((a, m) => a + f(m), 0) / n);
  const hold = avg((m) => (m.hold_frames || []).reduce((a, b) => a + b, 0)) / 60;
  console.log(`${label}: n ${n}, 30 s without a touch ${(100 * ms.filter((m) => m.max_no_touch >= 1800).length / n).toFixed(1)} %, longest no-touch p50 ${(ms.map((m) => m.max_no_touch).sort((a, b) => a - b)[n >> 1] / 60).toFixed(1)} s, hold ${hold.toFixed(1)} s a match, idle ${(avg((m) => m.idle_frames) / 60).toFixed(1)} s (${(100 * avg((m) => m.idle_frames / m.play_frames)).toFixed(1)} % of play), touches/min ${avg((m) => m.touches_per_min).toFixed(1)}, win ${(100 * ms.filter((m) => m.result === 'win').length / n).toFixed(1)}`);
}
