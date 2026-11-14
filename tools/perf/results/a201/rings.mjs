#!/usr/bin/env node
// lane A201 (evolution 12): the contested ring. node rings.mjs <log> [...] [--json out.json]
// 1. per arm: wins, paired difference against "ignore", rings shown a match, the human's knockdowns a match and those at a loose puck (RINGMATCH rows)
// 2. the paired read: arm kN obeys only the N-th ring; the first RINGSCENE row of kN that differs from ignore's at the same seed and frame (obey=1) is it.
import { readFileSync, writeFileSync } from 'node:fs';
const args = process.argv.slice(2); const ji = args.indexOf('--json'); const jout = ji >= 0 ? args.splice(ji, 2)[1] : '';
const scenes = {}, matches = {}, order = [];
const kv = (s) => { const m = s.match(/^arm=(.*?) level=/); const r = {}; if (m) { r.arm = m[1]; s = s.slice(m[0].length - 6); } for (const x of s.trim().split(' ')) { const i = x.indexOf('='); if (i < 0) continue; const k = x.slice(0, i), v = x.slice(i + 1); if (k === 'arm') continue; r[k] = isNaN(+v) ? v : +v; } return r; };
for (const f of args) for (const l of readFileSync(f, 'utf8').split(/\r?\n/)) {
  let i = l.indexOf('RINGSCENE '); if (i >= 0) { const r = kv(l.slice(i + 10)); const k = `${r.level}|${r.arm}`; (scenes[k] ||= new Map()); const a = scenes[k].get(r.seed) || []; a.push(r); scenes[k].set(r.seed, a); continue; }
  i = l.indexOf('RINGMATCH '); if (i >= 0) { const r = kv(l.slice(i + 10)); const k = `${r.level}|${r.arm}`; if (!matches[k]) { matches[k] = new Map(); order.push(k); } matches[k].set(r.seed, r); continue; }
}
const res = (k, seed) => { const a = scenes[k]?.get(seed); return a && a.length ? a[0].res : undefined; };
// match results come from RINGSCENE (res) or are unknown when a match had no ring: read them from CARRYTRACE match lines instead
const results = {}; let arm = '';
for (const f of args) for (const l of readFileSync(f, 'utf8').split(/\r?\n/)) {
  let m = l.match(/CARRYTRACE arm (\S+)/); if (m) { arm = m[1]; continue; }
  m = l.match(/CARRYTRACE match \S+ (\S+) seed (\d+): \d+ frames, (\d+):(\d+)/); if (m) { (results[`${m[1]}|${arm}`] ||= {})[m[2]] = Math.sign(m[3] - m[4]); }
}
console.log('== arms: wins | vs ignore | rings a match | knockdowns a match | at a loose puck a match');
for (const k of order) {
  const [lv, a] = k.split('|'); if (/^k\d+$/.test(a)) continue;
  const R = results[k] || {}, I = results[`${lv}|ignore`] || {}; const seeds = Object.keys(R); const n = seeds.length;
  const w = seeds.filter((s) => R[s] > 0).length; let up = 0, dn = 0; for (const s of seeds) if (s in I) { if (R[s] > 0 && I[s] <= 0) up++; if (R[s] <= 0 && I[s] > 0) dn++; }
  const M = [...matches[k].values()]; const avg = (f) => (M.reduce((x, r) => x + r[f], 0) / M.length).toFixed(2);
  console.log(`${lv} ${a.padEnd(28)} ${w} of ${n} (${(100 * w / n).toFixed(1)} %) | +${up} -${dn} | ${avg('rings')} | ${avg('downs')} | ${avg('downs_loose')}`);
}
const pairs = [];
const sig = (r) => [r.obey, r.out, r.down, r.gf, r.ga, r.res].join(',');
for (const key of Object.keys(scenes)) {
  const [lv, a] = key.split('|'); if (!/^k\d+$/.test(a)) continue;
  const ign = scenes[`${lv}|ignore`]; if (!ign) continue;
  for (const [seed, rows] of scenes[key]) {
    const base = new Map((ign.get(seed) || []).map((r) => [r.f, r]));
    for (const r of rows.sort((x, y) => x.f - y.f)) { const b = base.get(r.f); if (!b) break; if (sig(b) !== sig(r)) { if (r.obey) pairs.push({ lv, s: b, o: r }); break; } }
  }
}
if (jout) writeFileSync(jout, JSON.stringify(pairs));
const groups = [
  ['all', () => 'all'],
  ['another opponent at the puck (s after the human)', (r) => { const d = r.othersec - r.hsec; return d < 0.3 ? 'a <0.3' : d < 0.8 ? 'b 0.3-0.8' : 'c >=0.8'; }],
  ['the ring\'s man skates at him (m/s)', (r) => r.opptoward < 3 ? 'a <3' : r.opptoward < 6 ? 'b 3-6' : 'c >=6'],
  ['closing m/s', (r) => r.cl < 14 ? 'a <14' : r.cl < 17 ? 'b 14-17' : 'c >=17'],
  ['his time to the puck s', (r) => r.hsec < 0.4 ? 'a <0.4' : r.hsec < 0.7 ? 'b 0.4-0.7' : 'c >=0.7'],
  ['the man arrives (opp - human) s', (r) => { const d = r.osec - r.hsec; return d < 0 ? 'a before him' : d < 0.3 ? 'b 0-0.3 after' : 'c 0.3+ after'; }],
  ['a mate at the puck before the man', (r) => r.matesec < r.osec ? 'yes' : 'no'],
  ['his speed', (r) => r.v < 6 ? 'a <6' : r.v < 9 ? 'b 6-9' : 'c >=9'],
];
const fmt = (x) => (x >= 0 ? '+' : '') + x.toFixed(1);
for (const lv of [...new Set(pairs.map((p) => p.lv))]) {
  const P = pairs.filter((p) => p.lv === lv);
  for (const [g, f] of groups) {
    console.log(`\n== ${lv} | ${g} == n | he got it and stayed up %: went -> held off | down within 150 f % | the ring's man got it % | goals +/- 10 s /100 | match (w-l) /100`);
    for (const k of [...new Set(P.map((p) => f(p.s)))].sort()) {
      const Q = P.filter((p) => f(p.s) === k); const n = Q.length;
      const pc = (fn, w) => (100 * Q.filter((p) => fn(p[w])).length / n).toFixed(0);
      const gd = (w) => 100 * Q.reduce((x, p) => x + p[w].gf - p[w].ga, 0) / n;
      const md = 100 * Q.reduce((x, p) => x + p.o.res - p.s.res, 0) / n;
      console.log(`${k.padEnd(14)} ${String(n).padStart(4)} | ${pc((r) => r.out === 0, 's')} -> ${pc((r) => r.out === 0, 'o')} | ${pc((r) => r.down === 1, 's')} -> ${pc((r) => r.down === 1, 'o')} | ${pc((r) => r.out === 2, 's')} -> ${pc((r) => r.out === 2, 'o')} | ${fmt(gd('s'))} -> ${fmt(gd('o'))} | ${fmt(md)}`);
    }
  }
}
