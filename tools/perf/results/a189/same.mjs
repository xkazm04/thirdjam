#!/usr/bin/env node
// lane A162: a built number against its tune: the matches of raw/<a>.jsonl and raw/<b>.jsonl on the seeds they share, compared on result, goals and frames (and state_hash when the rows carry it)
import { readFileSync } from 'node:fs';
const load = (l) => new Map(readFileSync(new URL(`raw/${l}.jsonl`, import.meta.url), 'utf8').trim().split('\n').map(JSON.parse).map((m) => [m.seed, m]));
const [a, b] = process.argv.slice(2).map(load);
let n = 0, same = 0;
const key = (m) => JSON.stringify([m.result, m.goals, m.frames, m.state_hash ?? m.hash ?? '']);
for (const [s, m] of a) { if (!b.has(s)) continue; n++; if (key(m) === key(b.get(s))) same++; }
console.log(`${process.argv[2]} vs ${process.argv[3]}: ${same} of ${n} shared seeds identical`);
