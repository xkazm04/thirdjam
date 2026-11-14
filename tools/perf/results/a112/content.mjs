#!/usr/bin/env node
// Lane A112: the content items of R-0048 on the shipped cartridges, as text edits (formatting kept).
//   node content.mjs   -> penalties false in hockey.regular / hockey.arena.wide / hockey.arena.large and every take (and fallback); take-3 rookie.
import { readFileSync, writeFileSync, readdirSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
const root = join(resolve(dirname(fileURLToPath(import.meta.url)), '..', '..', '..', '..'), 'Game', 'Content', 'Cartridges');
const files = ['hockey.regular', 'hockey.arena.wide', 'hockey.arena.large'].map((n) => join(root, `${n}.cartridge.json`))
  .concat(readdirSync(join(root, 'ladder')).filter((f) => /^take-.*\.cartridge\.json$/.test(f)).map((f) => join(root, 'ladder', f)));
for (const f of files) {
  let s = readFileSync(f, 'utf8');
  const n = (s.match(/"penalties": true/g) || []).length;
  if (n !== 1) { console.log(`SKIP ${f}: ${n} "penalties": true`); continue; }
  s = s.replace('"penalties": true', '"penalties": false');
  if (/[\/]take-3\.cartridge\.json$/.test(f)) s = s.replace('"difficulty": "pro"', '"difficulty": "rookie"');
  writeFileSync(f, s);
  console.log(`edited ${f}`);
}
