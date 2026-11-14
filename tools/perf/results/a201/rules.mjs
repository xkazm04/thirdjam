#!/usr/bin/env node
// lane A201: candidate gates on the paired rows (out/pairs-*.json): per rule the share of lights it keeps, a match's lights, and the stop's paired effect on what it keeps
import { readFileSync } from 'node:fs';
const P = JSON.parse(readFileSync(process.argv[2], 'utf8'));
const lightsPerMatch = { pro: 13.0, rookie: 13.2 };
const rules = [];
for (const s of [0, 0.5, 0.7, 0.8, 1.0, 1.2]) for (const nlo of [0, 12, 15]) for (const nhi of [99, 40]) rules.push({ s, nlo, nhi });
for (const lv of ['pro', 'rookie']) {
  console.log(`\n== ${lv}: rule | kept % of lights | lights a match | n | lost to the man ign->stop | to another | goals 10 s ign->stop | match diff /100 | match diff of the dropped /100`);
  const L = P.filter((p) => p.lv === lv);
  for (const r of rules) {
    const keep = (p) => p.s.sec >= r.s && p.s.net >= r.nlo && p.s.net < r.nhi;
    const K = L.filter(keep), D = L.filter((p) => !keep(p)); const n = K.length;
    const pc = (Q, f, w) => Q.length ? (100 * Q.filter((p) => f(p[w])).length / Q.length).toFixed(0) : '-';
    const gd = (Q, w) => Q.length ? (100 * Q.reduce((x, p) => x + p[w].gf - p[w].ga, 0) / Q.length).toFixed(1) : '-';
    const md = (Q) => Q.length ? (100 * Q.reduce((x, p) => x + p.o.res - p.s.res, 0) / Q.length).toFixed(1) : '-';
    console.log(`sec>=${r.s} net ${r.nlo}-${r.nhi} | ${(100 * n / L.length).toFixed(1)} | ${(lightsPerMatch[lv] * n / L.length).toFixed(2)} | ${n} | ${pc(K, (x) => x.out === 1, 's')}->${pc(K, (x) => x.out === 1, 'o')} | ${pc(K, (x) => x.out === 2 || x.out === 3, 's')}->${pc(K, (x) => x.out === 2 || x.out === 3, 'o')} | ${gd(K, 's')}->${gd(K, 'o')} | ${md(K)} | ${md(D)}`);
  }
}
