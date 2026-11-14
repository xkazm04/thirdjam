#!/usr/bin/env node
// lane A201: the paired instrument. Arms "ignore" and "kN" (the scripted player obeys only the N-th light) on the same seeds are the same match up to
// that light: the first LITSCENE row of kN that differs from ignore's row at the same seed and frame is the obeyed scene. Per scene: the stop's own effect
// on the 90-frame outcome, the goals inside 10 s and the match. node pairs.mjs <log> [...] [--json out.json]
import { readFileSync, writeFileSync } from 'node:fs';
const args = process.argv.slice(2); const ji = args.indexOf('--json'); const jout = ji >= 0 ? args.splice(ji, 2)[1] : '';
const by = {}; let arm = '';
for (const f of args) for (const l of readFileSync(f, 'utf8').split(/\r?\n/)) {
  let m = l.match(/CARRYTRACE arm (\S+)/); if (m) { arm = m[1]; continue; }
  const i = l.indexOf('LITSCENE '); if (i < 0) continue;
  const r = {}; for (const kv of l.slice(i + 9).trim().split(' ')) { const [k, v] = kv.split('='); r[k] = isNaN(+v) ? v : +v; }
  const key = `${r.level}|${r.arm}`; (by[key] ||= new Map()); const s = by[key].get(r.seed) || []; s.push(r); by[key].set(r.seed, s);
}
const pairs = [];
const sig = (r) => [r.braked, r.out, r.at, r.down, r.gf, r.ga, r.res].join(',');
for (const key of Object.keys(by)) {
  const [lv, a] = key.split('|'); if (!/^k\d+$/.test(a)) continue;
  const ign = by[`${lv}|ignore`]; if (!ign) continue;
  for (const [seed, rows] of by[key]) {
    const base = new Map((ign.get(seed) || []).map((r) => [r.f, r]));
    for (const r of rows.sort((x, y) => x.f - y.f)) {
      const b = base.get(r.f);
      if (!b) break;   // diverged before any scene differed: skip
      if (sig(b) !== sig(r)) { if (r.braked) pairs.push({ lv, k: a, seed, s: b, o: r }); break; }
    }
  }
}
if (jout) writeFileSync(jout, JSON.stringify(pairs));
const good = (r) => r.out === 0 || r.out === 4 || r.out === 5;
const groups = [
  ['all', () => 'all'],
  ['second man s', (r) => r.sec < 0.4 ? 'a <0.4' : r.sec < 0.7 ? 'b 0.4-0.7' : r.sec < 1.0 ? 'c 0.7-1.0' : r.sec < 1.5 ? 'd 1.0-1.5' : 'e >=1.5'],
  ['others in 10 m', (r) => `n=${Math.max(-1, Math.min(2, r.a10 + r.s10 + r.b10 - 1))}`],
  ['others in 6 m', (r) => `n=${Math.min(2, r.a6 + r.s6 + r.b6 - (r.d <= 6 ? 1 : 0))}`],
  ['ahead (not the man) in 14 m', (r) => `a=${Math.min(2, r.a14 - 1)}`],
  ['behind in 10 m', (r) => `b=${Math.min(2, r.b10)}`],
  ['net m', (r) => r.net < 12 ? 'a <12' : r.net < 20 ? 'b 12-20' : r.net < 30 ? 'c 20-30' : r.net < 40 ? 'd 30-40' : 'e >=40'],
  ['boards at stop m', (r) => r.brd < 2 ? 'a <2' : r.brd < 5 ? 'b 2-5' : 'c >=5'],
  ['his speed', (r) => r.v < 7 ? 'a <7' : r.v < 9 ? 'b 7-9' : 'c >=9'],
  ['off deg', (r) => r.off < 20 ? 'a <20' : r.off < 40 ? 'b 20-40' : 'c >=40'],
  ['def dist', (r) => r.d < 9.5 ? 'a <9.5' : r.d < 11 ? 'b 9.5-11' : 'c >=11'],
  ['closing', (r) => r.cl < 11 ? 'a <11' : r.cl < 14 ? 'b 11-14' : 'c >=14'],
];
const fmt = (x) => (x >= 0 ? '+' : '') + x.toFixed(1);
for (const lv of [...new Set(pairs.map((p) => p.lv))]) {
  const P = pairs.filter((p) => p.lv === lv);
  for (const [g, f] of groups) {
    console.log(`\n== ${lv} | ${g} == n | good% ignore -> stop | lost to the man % | lost to another % | down % | goal diff in 10 s /100 ignore -> stop | match result diff (wins-losses)/100 stops`);
    for (const k of [...new Set(P.map((p) => f(p.s)))].sort()) {
      const Q = P.filter((p) => f(p.s) === k); const n = Q.length;
      const pc = (fn, w) => (100 * Q.filter((p) => fn(p[w])).length / n).toFixed(0);
      const gd = (w) => 100 * Q.reduce((x, p) => x + p[w].gf - p[w].ga, 0) / n;
      const res = 100 * Q.reduce((x, p) => x + p.o.res - p.s.res, 0) / n;
      console.log(`${k.padEnd(10)} ${String(n).padStart(4)} | ${pc(good, 's')} -> ${pc(good, 'o')} | ${pc((r) => r.out === 1, 's')} -> ${pc((r) => r.out === 1, 'o')} | ${pc((r) => r.out === 2 || r.out === 3, 's')} -> ${pc((r) => r.out === 2 || r.out === 3, 'o')} | ${pc((r) => r.down === 1, 's')} -> ${pc((r) => r.down === 1, 'o')} | ${fmt(gd('s'))} -> ${fmt(gd('o'))} | ${fmt(res)}`);
    }
  }
}
