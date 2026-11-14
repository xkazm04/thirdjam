#!/usr/bin/env node
// Lane A174: the verdict of a soak run. node tools/soak/soak-report.mjs <out dir> [<out dir> ...] [--mem <memwatch.jsonl>] [--budget 30000]
// Reads the cells.mjs output folders: matches, faults (faults.jsonl), matches that did not finish or ran to the frame budget (label, cut, level, class, seed, frames, result),
// non-finite numbers (the playtest line is JSON: a NaN or Infinity shows as the bare word, which this scans for in the raw text), and memory per process from memwatch.mjs.
import { readFileSync, readdirSync, existsSync } from 'node:fs';
import { join } from 'node:path';
import { report } from './memwatch.mjs';

const argv = process.argv.slice(2);
const opt = (n, d) => { const i = argv.indexOf(`--${n}`); return i >= 0 ? argv[i + 1] : d; };
const dirs = argv.filter((a, i) => !a.startsWith('--') && !(i > 0 && argv[i - 1].startsWith('--')));
const budget = +opt('budget', 30000);
const groups = {};
let matches = 0, faults = 0, nan = 0; const long = [], unfinished = []; const faultLines = [];
for (const d of dirs) {
  for (const f of readdirSync(d)) {
    const p = join(d, f);
    if (f.endsWith('.jsonl') && !f.endsWith('.partial.jsonl') && !f.endsWith('.faults.jsonl') && f !== 'faults.jsonl') {
      const txt = readFileSync(p, 'utf8');
      if (/[:,\[]\s*-?(NaN|Infinity|nan|inf)\b/.test(txt)) nan++;
      for (const l of txt.split('\n').filter(Boolean)) {
        const m = JSON.parse(l); matches++;
        const row = { label: f.replace('.jsonl', ''), cart: m.cartridge, level: m.difficulty, cls: m.player, seed: m.seed, frames: m.frames, result: m.result, over: m.over };
        const g = (groups[row.cart + ' ' + row.level + ' ' + row.cls] ??= { n: 0, over: 0, max: 0, sum: 0 }); g.n++; g.sum += m.frames; g.max = Math.max(g.max, m.frames); if (m.frames >= budget) g.over++;
        if (!['win', 'loss', 'draw'].includes(m.result) || m.over === false) unfinished.push(row);
        else if (m.frames >= budget) long.push(row);
        for (const [k, v] of Object.entries(m)) if (typeof v === 'number' && !Number.isFinite(v)) nan++;
      }
    }
    if (f === 'faults.jsonl') for (const l of readFileSync(p, 'utf8').split('\n').filter(Boolean)) { faults++; faultLines.push(l.slice(0, 300)); }
  }
}
console.log(JSON.stringify({ matches, faults, nan_files_or_fields: nan, unfinished: unfinished.length, at_or_over_budget: long.length }));
for (const [k, g] of Object.entries(groups).sort()) console.log('GROUP ' + k + ': n ' + g.n + ', mean frames ' + Math.round(g.sum / g.n) + ', max ' + g.max + ', at or over ' + budget + ': ' + g.over + ' (' + (100 * g.over / g.n).toFixed(2) + ' %)');
if (!argv.includes('--quiet')) for (const r of [...unfinished, ...long]) console.log(`LONG ${JSON.stringify(r)}`);
for (const l of faultLines) console.log(`FAULT ${l}`);
const mem = opt('mem');
if (mem && existsSync(mem)) for (const r of report(mem)) console.log(`MEM ${JSON.stringify(r)}`);
