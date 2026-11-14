#!/usr/bin/env node
// Lane A133 evolution 3: the season table before / after from cells.jsonl (SF-* as written; before: S-novice-* / S4-* / SA-novice-t6 for the novice, SO-* for regular / good; takes 1-3 unchanged = SF-*)
import { readFileSync } from 'node:fs';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
const rows = readFileSync(join(dirname(fileURLToPath(import.meta.url)), 'cells.jsonl'), 'utf8').trim().split('\n').map(JSON.parse);
const by = (l) => rows.filter((r) => r.label === l).at(-1);
const w = (r) => (r ? String(r.win) : '-'), s = (r) => (r ? String(r.shots) : '-');
const after = { 1: ['rookie', '-'], 2: ['rookie', '-'], 3: ['rookie', '-'], 4: ['rookie', '-'], 5: ['pro', '0.4'], 6: ['pro', '0.45'], 7: ['pro', '0.9'], 8: ['all_star', '0.7'] };
const before = { 1: 'rookie', 2: 'rookie', 3: 'rookie', 4: 'pro', 5: 'pro', 6: 'pro', 7: 'pro', 8: 'all_star' };
const novBefore = { 4: 'S-novice-take-4', 5: 'S-novice-take-5', 6: 'SA-novice-t6', 7: 'S-novice-take-7', 8: 'S-novice-take-8' };
console.log('| take | level, red skill (before -> after) | novice before -> after | regular before -> after | good before -> after | novice shots a match before -> after |');
console.log('|---|---|---|---|---|---|');
for (let t = 1; t <= 8; t++) {
  const a = [by(`SF-novice-take-${t}`), by(`SF-regular-take-${t}`), by(`SF-good-take-${t}`)];
  const b = t <= 3 ? a : [by(novBefore[t]), by(`SO-regular-take-${t}`), by(`SO-good-take-${t}`)];
  const lv = `${before[t]} -> ${after[t][0]}${after[t][1] !== '-' ? ', red ' + after[t][1] : ''}`;
  console.log(`| ${t} | ${t <= 3 ? before[t] + ' (unchanged)' : lv} | ${w(b[0])} -> **${w(a[0])}** | ${w(b[1])} -> **${w(a[1])}** | ${w(b[2])} -> **${w(a[2])}** | ${s(b[0])} -> ${s(a[0])} |`);
}
