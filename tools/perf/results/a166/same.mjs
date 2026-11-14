#!/usr/bin/env node
// Lane A147: are two arms of cells.mjs the same matches? Compares raw/<a>-<cell>.jsonl with raw/<b>-<cell>.jsonl seed by seed (frames, goals, hash).
//   node same.mjs <armA> <armB>
import { readFileSync } from 'node:fs';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
const here = dirname(fileURLToPath(import.meta.url));
const [a, b] = process.argv.slice(2);
const read = (arm, key) => new Map(readFileSync(join(here, 'raw', `${arm}-${key}.jsonl`), 'utf8').split(/\r?\n/).filter(Boolean).map((l) => JSON.parse(l)).map((m) => [m.seed, m]));
let bad = 0;
for (const key of ['rk', 'pro', 'as']) {
  const A = read(a, key), B = read(b, key);
  let same = 0, differ = 0, missing = 0;
  for (const [seed, m] of A) { const o = B.get(seed); if (!o) { missing++; continue; } if (o.hash === m.hash && o.frames === m.frames && String(o.goals) === String(m.goals)) same++; else differ++; }
  bad += differ + missing;
  console.log(`${key}: ${A.size} against ${B.size} matches, same hash ${same}, different ${differ}, missing ${missing}`);
}
console.log(bad === 0 ? `PASS ${a} and ${b} are the same matches` : `FAIL ${a} and ${b} differ`);
