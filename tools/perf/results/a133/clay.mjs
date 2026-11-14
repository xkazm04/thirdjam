#!/usr/bin/env node
// Lane A133 (R-0054 point 1): the first look is clay. look.tier "p3_clay" in hockey.regular, hockey.arena.wide, hockey.arena.large, the season's takes 1 to 4
// and the acceptance run's cut (tools/qa/accept/accept-cut.json follows the regular cut); takes 5 to 8 and the modes keep theirs. Text edits, formatting kept.
import { readFileSync, writeFileSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
const repo = resolve(dirname(fileURLToPath(import.meta.url)), '..', '..', '..', '..');
const c = (n) => join(repo, 'Game', 'Content', 'Cartridges', `${n}.cartridge.json`);
const files = ['hockey.regular', 'hockey.arena.wide', 'hockey.arena.large', 'ladder/take-1', 'ladder/take-2', 'ladder/take-3', 'ladder/take-4'].map(c)
  .concat([join(repo, 'tools', 'qa', 'accept', 'accept-cut.json')]);
for (const f of files) {
  const s = readFileSync(f, 'utf8');
  const m = s.match(/"tier": "([a-z0-9_]+)"/g) || [];
  if (m.length !== 1) { console.log(`FAIL ${f}: ${m.length} tier keys`); process.exit(1); }
  writeFileSync(f, s.replace(/"tier": "[a-z0-9_]+"/, '"tier": "p3_clay"'));
  console.log(`${m[0]} -> "p3_clay"  ${f.slice(repo.length + 1)}`);
}
