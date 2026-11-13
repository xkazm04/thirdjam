#!/usr/bin/env node
// Lane A55: compile-time tables for the editor target from what UBT leaves in Game/Intermediate after a build.
//  - headers by inclusion count: every <TU>.dep.json (MSVC /sourceDependencies; written by a plain build) lists the headers a TU opens beyond its PCH.
//  - slowest translation units and heaviest headers by time: every <TU>.timing (written by a `-Timing` build: /Bt+ /d1reportTime) gives
//    the front-end (c1xx) and back-end (c2) seconds and the include tree with inclusive seconds per header.
// Usage: node tools/analysis/compiletime.mjs [--top 20]   Prints TU, HDRCOUNT and HDRTIME rows, then a JSON line last.
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..', '..');
const inter = path.join(root, 'Game', 'Intermediate', 'Build', 'Win64', 'x64', 'UnrealEditor', 'Development');
const topArg = process.argv.indexOf('--top');
const TOP = topArg > 0 ? Number(process.argv[topArg + 1]) : 20;
function walk(d, out = []) {
  if (!fs.existsSync(d)) return out;
  for (const e of fs.readdirSync(d, { withFileTypes: true })) {
    const p = path.join(d, e.name);
    if (e.isDirectory()) walk(p, out); else out.push(p);
  }
  return out;
}
const all = walk(inter);
const short = (p) => {
  const s = p.split('\\').join('/');
  const i = s.toLowerCase().indexOf('/game/source/');
  if (i >= 0) return s.slice(i + 1);
  const e = s.toLowerCase().indexOf('/engine/');
  if (e >= 0) return 'Engine' + s.slice(e + 7);
  const g = s.toLowerCase().indexOf('/intermediate/');
  return g >= 0 ? s.slice(g + 1) : s;
};

// 1. inclusion counts
const incCount = new Map();
let deps = 0;
for (const f of all.filter((x) => x.endsWith('.dep.json'))) {
  let j; try { j = JSON.parse(fs.readFileSync(f, 'utf8')); } catch { continue; }
  const src = j?.Data?.Source || '';
  if (!/game[\\/]source[\\/]jam/i.test(src)) continue;
  deps++;
  for (const h of j.Data.Includes || []) { if (/\.gen\.|definitions\.|\.generated\.h$/i.test(h)) continue; const k = short(h); incCount.set(k, (incCount.get(k) || 0) + 1); }
}
const hdrCount = [...incCount.entries()].sort((a, b) => b[1] - a[1]).slice(0, TOP);

// 2. timing files
const tus = [];
const hdrTime = new Map();
for (const f of all.filter((x) => x.endsWith('.timing'))) {
  const t = fs.readFileSync(f, 'utf8');
  const name = path.basename(f).replace(/\.timing$/, '').replace(/\.obj$/, '');
  const fe = [...t.matchAll(/time\(.*?c1xx\.dll\)=([\d.]+)s/g)].reduce((s, m) => s + Number(m[1]), 0);
  const be = [...t.matchAll(/time\(.*?c2\.dll\)=([\d.]+)s/g)].reduce((s, m) => s + Number(m[1]), 0);
  tus.push({ tu: name, module: f.split(path.sep).slice(-2, -1)[0], frontend: fe, backend: be, total: fe + be });
  // /d1reportTime "Include Headers:" section: lines "\t<path>: 0.12345s" at depth = leading tabs; count top-level (depth 1) inclusive times
  const sec = t.split(/Include Headers:/)[1];
  if (sec) {
    const body = sec.split(/\n\s*\n|Class Definitions:|Function Definitions:/)[0];
    for (const line of body.split(/\r?\n/)) {
      const m = line.match(/^(\t+)(.+?):\s*([\d.]+)s/);
      if (!m) continue;
      const k = short(m[2].trim());
      const cur = hdrTime.get(k) || { seconds: 0, tus: 0 };
      cur.seconds += Number(m[3]); cur.tus += 1;
      hdrTime.set(k, cur);
    }
  }
}
tus.sort((a, b) => b.total - a.total);
const hdrT = [...hdrTime.entries()].sort((a, b) => b[1].seconds - a[1].seconds).slice(0, TOP);
const byModule = {};
for (const u of tus) byModule[u.module] = Math.round(((byModule[u.module] || 0) + u.total) * 100) / 100;
for (const u of tus.slice(0, TOP)) console.log(`TU ${u.total.toFixed(2)}s fe ${u.frontend.toFixed(2)} be ${u.backend.toFixed(2)} ${u.module}/${u.tu}`);
for (const [h, n] of hdrCount) console.log(`HDRCOUNT ${n} ${h}`);
for (const [h, v] of hdrT) console.log(`HDRTIME ${v.seconds.toFixed(2)}s in ${v.tus} opens ${h}`);
for (const [m, s] of Object.entries(byModule)) console.log(`MODULE ${s}s ${m}`);
console.log(JSON.stringify({ schema: 'a55.compiletime/1', depFiles: deps, timingFiles: tus.length, byModule }));
