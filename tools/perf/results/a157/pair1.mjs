#!/usr/bin/env node
// Lane A157 (track F1): two single-cell arms of arms1.mjs compared on the seeds both played (pair.mjs wants all three cells). Prints one row; with a
// fourth argument "from=N" only seeds above N count (the fresh half of a longer run).
//   node pair1.mjs <armA> <armB> <rk|pro|as> [from=N]
import { readFileSync } from 'node:fs';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
const here = dirname(fileURLToPath(import.meta.url));
const [armA, armB, key, fromArg = 'from=0'] = process.argv.slice(2);
const from = Number(fromArg.split('=')[1]);
const read = (arm) => readFileSync(join(here, 'raw', `${arm}-${key}.jsonl`), 'utf8').split(/\r?\n/).filter(Boolean).map((l) => JSON.parse(l)).filter((m) => m.seed > from);
const a = read(armA), bs = new Map(read(armB).map((m) => [m.seed, m]));
let both = 0, wa = 0, wb = 0, aOnly = 0, bOnly = 0, identical = 0;
for (const m of a) {
  const o = bs.get(m.seed); if (!o) continue; ++both;
  const x = m.result === 'win', y = o.result === 'win';
  if (x) ++wa; if (y) ++wb; if (x && !y) ++aOnly; if (!x && y) ++bOnly; if (m.hash && m.hash === o.hash) ++identical;
}
const pct = (k) => (100 * k / Math.max(1, both)).toFixed(1);
const se = (100 * Math.sqrt(aOnly + bOnly) / Math.max(1, both)).toFixed(1);
console.log(`${key} seeds above ${from}: ${both} paired | ${armA} ${pct(wa)} | ${armB} ${pct(wb)} | delta ${(100 * (wb - wa) / Math.max(1, both)).toFixed(1)} (standard error ${se}) | flips ${aOnly}/${bOnly} | identical hashes ${identical}/${both}`);
