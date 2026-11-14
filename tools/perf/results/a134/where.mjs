#!/usr/bin/env node
// Lane A134: in the frames 10 s or more after the human's last touch (TRACE since >= 600) with an opposing carrier: where everyone is.
// usage: node where.mjs <traced log...>
import { readFileSync } from 'node:fs';
for (const f of process.argv.slice(2)) {
  let n = 0; const acc = { dh: [], dm: [], db: [], cx: [], cspd: [], role: {}, hgs: 0, mgs: 0, loose: 0, looseDh: [], looseDm: [], looseDo: [], looseHfirst: 0, looseMfirst: 0, looseOfirst: 0 };
  let prev = null;
  for (const line of readFileSync(f, 'utf8').split(/\r?\n/)) {
    const i = line.indexOf('TRACE f='); if (i < 0) continue;
    const t = line.slice(i); if (t.includes(' frozen ')) { prev = null; continue; }
    const m = /f=(\d+) stall=(\d) puck\(([-\d.]+),([-\d.]+) v([\d.]+)\)/.exec(t); if (!m) continue;
    const sk = [...t.matchAll(/ (\d)([PCSBH])\(([-\d.]+),([-\d.]+)\)>(-?\d+)\[([-\d.]+),([-\d.]+)/g)].map((x) => ({ team: +x[1], role: x[2], x: +x[3], y: +x[4] }));
    const s = /since=(\d+) ht=(\d) car=(-?\d+) lt=(-?\d+) fo=(\d)/.exec(t); if (!s || s[5] === '1') continue;
    const hi = sk.findIndex((k) => k.role === 'H'); if (hi < 0) continue;
    const car = +s[3], since = +s[1]; const px = +m[3], py = +m[4];
    const H = sk[hi]; const sg = H.team === 0 ? 1 : -1;
    const d = (a) => Math.hypot(a.x - px, a.y - py);
    const mates = sk.filter((k, j) => j !== hi && k.team === H.team), opps = sk.filter((k) => k.team !== H.team);
    if (since < 600) { prev = { f: +m[1], sk }; continue; }
    if (car >= 0 && sk[car].team !== H.team) {
      n++; const C = sk[car];
      acc.dh.push(d(H)); acc.dm.push(Math.min(...mates.map(d))); acc.db.push(Math.max(...mates.map(d)));
      acc.cx.push(C.x * sg); acc.role[C.role + car] = (acc.role[C.role + car] || 0) + 1;
      if (prev && prev.f === +m[1] - 30) { const p = prev.sk[car]; acc.cspd.push(Math.hypot(C.x - p.x, C.y - p.y) * 2); }
      const og = { x: -sg * 56, y: 0 }; const gd = (a) => Math.hypot(a.x - og.x, a.y - og.y);
      if (gd(H) < gd(C)) acc.hgs++;
      if (Math.min(...mates.map(gd)) < gd(C)) acc.mgs++;
    } else if (car < 0) {
      acc.loose++; acc.looseDh.push(d(H)); acc.looseDm.push(Math.min(...mates.map(d))); acc.looseDo.push(Math.min(...opps.map(d)));
      const a = d(H), b = Math.min(...mates.map(d)), c = Math.min(...opps.map(d));
      if (a <= b && a <= c) acc.looseHfirst++; else if (b <= c) acc.looseMfirst++; else acc.looseOfirst++;
    }
    prev = { f: +m[1], sk };
  }
  const q = (a, p) => { const s = [...a].sort((x, y) => x - y); return s.length ? s[Math.floor(s.length * p)].toFixed(1) : '-'; };
  const hist = (a, edges) => edges.map((e, i) => `${i ? edges[i - 1] : 0}-${e}:${(100 * a.filter((v) => v >= (i ? edges[i - 1] : -1e9) && v < e).length / (a.length || 1)).toFixed(0)}%`).join(' ');
  console.log(f.split(/[\/]/).pop());
  console.log(` opp carries, 10 s+ since the human's touch: ${n} samples`);
  console.log(`  human -> puck m: p25 ${q(acc.dh, 0.25)} p50 ${q(acc.dh, 0.5)} p75 ${q(acc.dh, 0.75)} | ${hist(acc.dh, [2, 4, 8, 15, 30, 999])}`);
  console.log(`  nearest mate -> puck m: p25 ${q(acc.dm, 0.25)} p50 ${q(acc.dm, 0.5)} p75 ${q(acc.dm, 0.75)} | ${hist(acc.dm, [2, 4, 8, 15, 30, 999])}`);
  console.log(`  far mate -> puck m: p50 ${q(acc.db, 0.5)}`);
  console.log(`  carrier x (the human attacks +): ${hist(acc.cx, [-40, -20, 0, 20, 40, 999])} | speed m/s p25 ${q(acc.cspd, 0.25)} p50 ${q(acc.cspd, 0.5)} p75 ${q(acc.cspd, 0.75)}`);
  console.log(`  human goal-side of the carrier ${(100 * acc.hgs / (n || 1)).toFixed(0)} %, a mate goal-side ${(100 * acc.mgs / (n || 1)).toFixed(0)} % | carrier by slot ${JSON.stringify(acc.role)}`);
  console.log(` loose, 10 s+: ${acc.loose} samples | nearest: human ${(100 * acc.looseHfirst / (acc.loose || 1)).toFixed(0)} % mate ${(100 * acc.looseMfirst / (acc.loose || 1)).toFixed(0)} % opp ${(100 * acc.looseOfirst / (acc.loose || 1)).toFixed(0)} % | human m p50 ${q(acc.looseDh, 0.5)} mate p50 ${q(acc.looseDm, 0.5)} opp p50 ${q(acc.looseDo, 0.5)}`);
}
