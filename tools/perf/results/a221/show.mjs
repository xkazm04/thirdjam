#!/usr/bin/env node
// lane A162: the rows of a162/cells.jsonl, one line each (label filter optional)
import { readFileSync } from 'node:fs';
const f = process.argv[2] || '';
for (const l of readFileSync(new URL('cells.jsonl', import.meta.url), 'utf8').trim().split('\n')) { const r = JSON.parse(l); if (!r.label.includes(f)) continue;
  console.log(`${r.at} ${r.label} ${r.cls}/${r.lvl}/${r.rink} ${r.tune || '-'} check=${r.check} n=${r.n} win=${r.win} (${r.lo}-${r.hi}) fwo=${r.fwo}/${r.faceoffs} ${r.secs}s`); }
