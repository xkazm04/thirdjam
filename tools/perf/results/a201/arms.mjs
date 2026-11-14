#!/usr/bin/env node
// lane A201: per arm of a JAM_TRACE_ARMS CarryTrace log: wins, the paired difference against the arm "ignore" (same seeds), the line's lit showings a match
import { readFileSync } from 'node:fs';
for (const f of process.argv.slice(2)) {
  const res = {}, lit = {}, order = []; let arm = '';
  for (const l of readFileSync(f, 'utf8').split(/\r?\n/)) {
    let m = l.match(/CARRYTRACE arm (\S+) \| stopmark "([^"]*)" \| rule "([^"]*)"/); if (m) { arm = m[1]; order.push([arm, m[2], m[3]]); res[arm] = {}; continue; }
    m = l.match(/CARRYTRACE match \S+ (\S+) seed (\d+): \d+ frames, (\d+):(\d+)/); if (m) { res[arm][m[2]] = Math.sign(m[3] - m[4]); continue; }
    m = l.match(/CARRYTRACE mark stop \S+ at \S+: lit in (\d+) episodes \(([\d.]+) a match\)/); if (m) lit[arm] = m[2];
  }
  console.log(`\n${f}`);
  const ig = res.ignore || {};
  for (const [a, sm, rule] of order) {
    const r = res[a]; const seeds = Object.keys(r); const n = seeds.length;
    const w = seeds.filter((s) => r[s] > 0).length, l = seeds.filter((s) => r[s] < 0).length;
    let flipUp = 0, flipDown = 0; for (const s of seeds) if (s in ig) { if (r[s] > 0 && ig[s] <= 0) flipUp++; if (r[s] <= 0 && ig[s] > 0) flipDown++; }
    console.log(`${a.padEnd(8)} wins ${String(w).padStart(3)} of ${n} (${(100 * w / n).toFixed(1)} %), losses ${l} | vs ignore: +${flipUp} -${flipDown} | lit ${lit[a]} a match | ${sm || '(ignores)'} ${rule}`);
  }
}
