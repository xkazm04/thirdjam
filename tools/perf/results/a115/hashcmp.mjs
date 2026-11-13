// usage: node hashcmp.mjs <logA> <logB>   compares the per-seed hashes of two playtest logs (lane A115)
import { readFileSync } from 'node:fs';
const rows = (f) => { const m = new Map(); for (const l of readFileSync(f, 'utf8').split(/\r?\n/)) { const i = l.indexOf('{"tool":"thirdjam.playtest/1"'); if (i >= 0) { try { const r = JSON.parse(l.slice(i)); m.set(r.seed, r.hash); } catch {} } } return m; };
const a = rows(process.argv[2]), b = rows(process.argv[3]);
let same = 0, diff = 0; for (const [k, v] of a) { if (b.get(k) === v) same++; else diff++; }
console.log(`hashes: ${same} equal, ${diff} different, of ${a.size} / ${b.size}`);
