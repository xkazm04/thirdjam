#!/usr/bin/env node
// Lane A200 (track F1, evolution 11): the wind-up table. For each arm of cells.mjs (raw/<arm>-<cell>.jsonl) against a base arm on the same seeds:
// the win rate and its paired difference, the human's knockdowns a match, the bots' landed checks a match (every victim), the opponents' landed hits on
// the human, the hits he saw coming and the share he got out of (1 - seen_landed / seen_hits), his dodges.
//   node table.mjs <base> <arm> [<arm> ...]          prints a markdown table, one row per arm and cell
import { existsSync, readFileSync } from 'node:fs';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
const here = dirname(fileURLToPath(import.meta.url));
const [base, ...arms] = process.argv.slice(2);
const cells = [['rk', 'rookie'], ['pro', 'pro'], ['as', 'all_star']];
const read = (arm, key) => { const f = join(here, 'raw', `${arm}-${key}.jsonl`); return existsSync(f) ? readFileSync(f, 'utf8').split(/\r?\n/).filter(Boolean).map((l) => JSON.parse(l)) : null; };
const avg = (ms, k) => ms.reduce((s, m) => s + (Number(m[k]) || 0), 0) / Math.max(1, ms.length);
const win = (ms) => 100 * ms.filter((m) => m.result === 'win').length / Math.max(1, ms.length);
console.log('| arm | level | n | win % | vs base (paired) | flips +/- | identical | human knockdowns | bot checks landed | opp hits on him | seen hits | got out % | dodges |');
console.log('|---|---|---|---|---|---|---|---|---|---|---|---|---|');
for (const arm of [base, ...arms]) {
  for (const [key, lvl] of cells) {
    const a = read(arm, key), b = read(base, key);
    if (!a || !b) { console.log(`| ${arm} | ${lvl} | missing |`); continue; }
    const bs = new Map(b.map((m) => [m.seed, m]));
    let plus = 0, minus = 0, same = 0, both = 0;
    for (const m of a) { const o = bs.get(m.seed); if (!o) continue; ++both; const wa = m.result === 'win', wb = o.result === 'win'; if (wa && !wb) ++plus; if (!wa && wb) ++minus; if (m.hash && m.hash === o.hash) ++same; }
    const seen = avg(a, 'seen_hits'), landed = avg(a, 'seen_landed');
    const out = seen > 0 ? (100 * (1 - landed / seen)).toFixed(1) : '-';
    console.log(`| ${arm} | ${lvl} | ${a.length} | ${win(a).toFixed(1)} | ${(win(a) - win(b) >= 0 ? '+' : '')}${(win(a) - win(b)).toFixed(1)} | ${plus}/${minus} | ${same}/${both} | ${avg(a, 'knockdowns').toFixed(2)} | ${avg(a, 'bot_checks_landed').toFixed(2)} | ${avg(a, 'bot_hits_on_human').toFixed(2)} | ${seen.toFixed(2)} | ${out} | ${avg(a, 'dodges').toFixed(2)} |`);
  }
}
