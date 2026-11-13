#!/usr/bin/env node
// Lane A80: after each release (JAMVIS getup) does a figure stay down or pop? Reads JAMLOW (a80-prove.ps1 logs): the pelvis at the release frame against the standing pelvis,
// the largest one-frame pelvis step in [release-2, release+10], and the per-skater counts of the JAMVIS events (bot vs bot: skater != the human).
//   node tools/qa/ctrl/a80-pop.mjs <game.log> [humanIndex]
import { readFileSync } from 'node:fs';
const L = readFileSync(process.argv[2], 'utf8').split(/\r?\n/);
const human = parseInt(process.argv[3] ?? '0');
const n = (re, l) => { const m = l.match(re); return m ? parseFloat(m[1]) : NaN; };
const rows = new Map(); const gu = []; const cnt = {};
for (const l of L) {
  let m;
  if (l.includes('JAMLOW frame=')) { const s = n(/skater=(\d+)/, l); if (!rows.has(s)) rows.set(s, new Map()); rows.get(s).set(n(/frame=(\d+)/, l), { p: n(/pelvis_cm=(-?[\d.]+)/, l), st: n(/stand_pelvis_cm=(-?[\d.]+)/, l) }); }
  if ((m = l.match(/JAMVIS getup frame=(\d+) skater=(\d+)/))) gu.push({ f: +m[1], s: +m[2] });
  if ((m = l.match(/JAMVIS (hit|board|pin|stickhit|knockdown) frame=(\d+) skater=(\d+)/))) { const k = `${m[1]}${m[1] === 'pin' && /on=0/.test(l) ? '_off' : ''}`; const who = +m[3] === human ? 'human' : 'bot'; cnt[`${k} ${who}`] = (cnt[`${k} ${who}`] || 0) + 1; }
}
let stayDown = 0, pops = 0, worst = 0, k = 0;
for (const g of gu) {
  const r = rows.get(g.s); if (!r) continue; const at = r.get(g.f); if (!at) continue; k++;
  if (at.p < 0.9 * at.st) stayDown++;
  let mx = 0; for (let f = g.f - 2; f <= g.f + 10; f++) { const a = r.get(f), b = r.get(f + 1); if (a && b) mx = Math.max(mx, Math.abs(b.p - a.p)); }
  worst = Math.max(worst, mx); if (mx > 10) pops++;
}
console.log(`releases ${gu.length}, with a pose ${k}: pelvis below 90% of standing at the release frame ${stayDown}; largest one-frame pelvis step within -2..+10 frames of the release ${worst.toFixed(1)} cm; steps over 10 cm ${pops}`);
console.log('JAMVIS counts (skater = human or bot):', JSON.stringify(cnt));
