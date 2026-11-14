#!/usr/bin/env node
// lane A201: the LITSCENE rows of Bar.Movement.CarryTrace logs (Game/logs/a201/<name>.log ...) -> the scene table: obeying against ignoring by scene class.
// node scenes.mjs <log> [<log> ...]   (outcome 0 kept, 1 lost to the man, 2 lost to another, 3 lost otherwise, 4 shot, 5 pass, 6 whistle)
import { readFileSync } from 'node:fs';
const rows = [], wins = {};
let arm = '';
for (const f of process.argv.slice(2)) {
  for (const l of readFileSync(f, 'utf8').split(/\r?\n/)) {
    let m = l.match(/CARRYTRACE arm (\S+)/); if (m) { arm = m[1]; continue; }
    m = l.match(/CARRYTRACE table \S+ at (\S+), (\d+) matches.* (\d+) wins \|/); if (m) { wins[`${arm} ${m[1]}`] = `${m[3]} of ${m[2]}`; continue; }
    const i = l.indexOf('LITSCENE '); if (i < 0) continue;
    const r = {}; for (const kv of l.slice(i + 9).trim().split(' ')) { const [k, v] = kv.split('='); r[k] = isNaN(+v) ? v : +v; }
    rows.push(r);
  }
}
console.log('wins', JSON.stringify(wins));
const good = (r) => r.out === 0 || r.out === 4 || r.out === 5;
const lostDef = (r) => r.out === 1, lostOther = (r) => r.out === 2 || r.out === 3;
const groups = [
  ['all', () => 'all'],
  ['lit', (r) => `lit=${r.lit} gate=${r.gate}`],
  ['second man s', (r) => r.sec < 0.5 ? 'a <0.5' : r.sec < 0.8 ? 'b 0.5-0.8' : r.sec < 1.2 ? 'c 0.8-1.2' : r.sec < 1.6 ? 'd 1.2-1.6' : 'e >=1.6'],
  ['others in 10 m', (r) => `n=${Math.min(3, r.a10 + r.s10 + r.b10 - 1)}`],
  ['others in 6 m', (r) => `n=${Math.min(2, r.a6 + r.s6 + r.b6 - (r.d <= 6 ? 1 : 0))}`],
  ['behind in 10 m', (r) => `b=${Math.min(2, r.b10)}`],
  ['net m', (r) => r.net < 10 ? 'a <10' : r.net < 14 ? 'b 10-14' : r.net < 20 ? 'c 14-20' : r.net < 30 ? 'd 20-30' : 'e >=30'],
  ['boards at stop m', (r) => r.brd < 1.5 ? 'a <1.5' : r.brd < 3 ? 'b 1.5-3' : r.brd < 6 ? 'c 3-6' : 'd >=6'],
  ['his speed', (r) => r.v < 7 ? 'a <7' : r.v < 9 ? 'b 7-9' : 'c >=9'],
  ['off deg', (r) => r.off < 20 ? 'a <20' : r.off < 40 ? 'b 20-40' : 'c >=40'],
  ['def dist', (r) => r.d < 9 ? 'a <9' : r.d < 10.5 ? 'b 9-10.5' : r.d < 12 ? 'c 10.5-12' : 'd >=12'],
];
const levels = [...new Set(rows.map((r) => r.level))];
for (const lv of levels) {
  const arms = [...new Set(rows.filter((r) => r.level === lv).map((r) => r.arm))];
  for (const [gname, gf] of groups) {
    console.log(`\n== ${lv} | ${gname} ==  per arm: n | good (kept/shot/pass) % | lost to the man % | lost to another % | down % | goals for-against in 10 s per 100 scenes`);
    const keys = [...new Set(rows.filter((r) => r.level === lv).map(gf))].sort();
    for (const k of keys) {
      let line = `${k.padEnd(14)}`;
      for (const a of arms) {
        const s = rows.filter((r) => r.level === lv && r.arm === a && gf(r) === k && (a.startsWith('obey') ? (r.lit === 0 || r.braked === 1) : true));
        const n = s.length; const p = (f) => n ? (100 * s.filter(f).length / n).toFixed(0).padStart(3) : '  -';
        const gd = n ? (100 * s.reduce((x, r) => x + r.gf - r.ga, 0) / n).toFixed(0).padStart(4) : '   -';
        line += ` | ${a}: ${String(n).padStart(4)} ${p(good)} ${p(lostDef)} ${p(lostOther)} ${p((r) => r.down === 1)} ${gd}`;
      }
      console.log(line);
    }
  }
}
