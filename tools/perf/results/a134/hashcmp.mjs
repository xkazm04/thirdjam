// Lane A134. usage: node hashcmp.mjs <labelA> <labelB>   per-seed hashes of two cells' raw files (tools/perf/results/a95/raw/<label>.jsonl), over the seeds both have
import { readFileSync } from 'node:fs';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
const here = dirname(fileURLToPath(import.meta.url));
const rows = (L) => new Map(readFileSync(join(here, '..', 'a95', 'raw', `${L}.jsonl`), 'utf8').trim().split(/\n/).map((l) => JSON.parse(l)).map((m) => [m.seed, m.hash]));
const a = rows(process.argv[2]), b = rows(process.argv[3]);
let same = 0, diff = 0; for (const [k, v] of a) { if (!b.has(k)) continue; if (b.get(k) === v) same++; else diff++; }
console.log(`hashes ${process.argv[2]} vs ${process.argv[3]}: ${same} equal, ${diff} different (of ${a.size} / ${b.size})`);
