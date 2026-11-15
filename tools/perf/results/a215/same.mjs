#!/usr/bin/env node
// lane A209: hash-identical seeds between two labels' raw files. node same.mjs <labelA> <labelB>
import { readFileSync } from 'node:fs';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
const here = dirname(fileURLToPath(import.meta.url));
const load = (l) => new Map(readFileSync(join(here, 'raw', `${l}.jsonl`), 'utf8').split('\n').filter(Boolean).map((x) => JSON.parse(x)).map((m) => [m.seed, m.hash]));
const [a, b] = process.argv.slice(2).map(load);
let same = 0, n = 0; for (const [s, h] of a) { if (b.has(s)) { n++; if (b.get(s) === h) same++; } }
console.log(`${process.argv[2]} vs ${process.argv[3]}: ${same} / ${n} hash-identical`);
