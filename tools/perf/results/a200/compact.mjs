#!/usr/bin/env node
// Lane A200: the WINDUP TABLE in one row an arm (rookie / pro / all_star in each cell), against a base arm on paired seeds.
//   node compact.mjs <base> <arm>[=label] ...
import { existsSync, readFileSync } from 'node:fs';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
const here = dirname(fileURLToPath(import.meta.url));
const [base, ...arms] = process.argv.slice(2);
const keys = ['rk', 'pro', 'as'];
const read = (arm, key) => { const f = join(here, 'raw', `${arm}-${key}.jsonl`); return existsSync(f) ? readFileSync(f, 'utf8').split(/\r?\n/).filter(Boolean).map((l) => JSON.parse(l)) : null; };
const avg = (ms, k) => ms.reduce((s, m) => s + (Number(m[k]) || 0), 0) / Math.max(1, ms.length);
const win = (ms) => 100 * ms.filter((m) => m.result === 'win').length / Math.max(1, ms.length);
const sg = (v) => `${v >= 0 ? '+' : ''}${v.toFixed(1)}`;
console.log('| arm | win % (r / p / a) | vs base, paired | human knockdowns a match | bots\' landed checks a match | opponents\' hits on him | hits he saw coming | of those he got out of, % |');
console.log('|---|---|---|---|---|---|---|---|');
for (const spec of [base, ...arms]) {
  const [arm, label = arm] = spec.split('=');
  const A = keys.map((k) => read(arm, k)), B = keys.map((k) => read(base, k));
  if (A.some((x) => !x)) { console.log(`| ${label} | (running) |`); continue; }
  const col = (f) => A.map(f).join(' / ');
  const out = (ms) => { const s = avg(ms, 'seen_hits'); return s > 0 ? (100 * (1 - avg(ms, 'seen_landed') / s)).toFixed(1) : '-'; };
  console.log(`| ${label} | ${col((m) => win(m).toFixed(1))} | ${A.map((m, i) => sg(win(m) - win(B[i]))).join(' / ')} | ${col((m) => avg(m, 'knockdowns').toFixed(2))} | ${col((m) => avg(m, 'bot_checks_landed').toFixed(2))} | ${col((m) => avg(m, 'bot_hits_on_human').toFixed(2))} | ${col((m) => avg(m, 'seen_hits').toFixed(1))} | ${col(out)} |`);
}
