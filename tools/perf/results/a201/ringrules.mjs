#!/usr/bin/env node
// lane A201 evolution 12: candidate gates for the ring on the paired rows (out/rings-*.json)
import { readFileSync } from 'node:fs';
const P = JSON.parse(readFileSync(process.argv[2], 'utf8'));
const per = { pro: 20.9, rookie: 13.0 };
const rules = [];
for (const o of [0, 0.3, 0.5]) for (const h of [1, 0.7, 0.55]) for (const m of [0, 1]) rules.push({ o, h, m });
for (const lv of ['pro', 'rookie']) {
  console.log(`\n== ${lv}: others / hsec / mate | kept % | rings a match | n | stayed up went->held | down went->held | match /100 kept | match /100 dropped`);
  const L = P.filter((p) => p.lv === lv);
  for (const r of rules) {
    const keep = (p) => (r.o === 0 || p.s.othersec >= p.s.hsec + r.o) && p.s.hsec <= r.h && (!r.m || !(p.s.matesec < p.s.osec));
    const K = L.filter(keep), D = L.filter((p) => !keep(p));
    const pc = (Q, f, w) => Q.length ? (100 * Q.filter((p) => f(p[w])).length / Q.length).toFixed(0) : '-';
    const md = (Q) => Q.length ? (100 * Q.reduce((x, p) => x + p.o.res - p.s.res, 0) / Q.length).toFixed(1) : '-';
    console.log(`${r.o} / ${r.h} / ${r.m} | ${(100 * K.length / L.length).toFixed(1)} | ${(per[lv] * K.length / L.length).toFixed(1)} | ${K.length} | ${pc(K, (x) => x.out === 0, 's')}->${pc(K, (x) => x.out === 0, 'o')} | ${pc(K, (x) => x.down === 1, 's')}->${pc(K, (x) => x.down === 1, 'o')} | ${md(K)} | ${md(D)}`);
  }
}
