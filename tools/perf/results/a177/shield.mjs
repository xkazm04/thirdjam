#!/usr/bin/env node
// Lane A177 (track F1): REVIEW-3 item 2's read-outs over the raw lines of one or more arms (cells.mjs / arms1.mjs), a row an arm and cell.
//   node shield.mjs [--first=N] <arm> ...
// kd_won = the human knocked down with the puck his for under 30 frames ("as he won it"); lying = his play frames on the ice / play frames.
import { existsSync, readFileSync } from 'node:fs';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
const here = dirname(fileURLToPath(import.meta.url));
let args = process.argv.slice(2);
let first = Infinity;
if (args[0]?.startsWith('--first=')) { first = Number(args[0].slice(8)); args = args.slice(1); }
const cells = [['rk', 'rookie'], ['pro', 'pro'], ['as', 'all_star']];
const f2 = (x) => x.toFixed(2);
for (const arm of args) {
  for (const [key, cell] of cells) {
    const file = join(here, 'raw', `${arm}-${key}.jsonl`);
    if (!existsSync(file)) continue;
    const ms = readFileSync(file, 'utf8').split(/\r?\n/).filter(Boolean).map((l) => JSON.parse(l)).filter((m) => m.seed <= first);
    const N = Math.max(1, ms.length);
    const sum = (k) => ms.reduce((s, m) => s + (Number(m[k]) || 0), 0);
    const play = Math.max(1, sum('play_frames'));
    const win = (100 * ms.filter((m) => m.result === 'win').length / N).toFixed(1);
    console.log(`${arm} ${cell}: ${ms.length} matches, win ${win} % | human knocked down ${f2(sum('knockdowns') / N)} a match: as he won it ${f2(sum('kd_won') / N)} (inside 15 frames ${f2(sum('kd_won15') / N)}), carrying longer ${f2(sum('kd_carry') / N)}, without the puck ${f2(sum('kd_nopuck') / N)} | lying ${(100 * sum('human_down_frames') / play).toFixed(2)} % of play, stumbling ${(100 * sum('human_stumble_frames') / play).toFixed(2)} % (stumbles ${f2(sum('stumbles') / N)} a match) | hits the shield capped: on the human ${f2(sum('shield_human') / N)}, on the others ${f2(sum('shield_bots') / N)} a match | others knocked down ${f2((sum('knockdowns_all') - sum('knockdowns')) / N)} | the human's landed checks ${f2(sum('checks_landed') / N)} | goals ${f2(ms.reduce((s, m) => s + m.goals[0], 0) / N)}:${f2(ms.reduce((s, m) => s + m.goals[1], 0) / N)}`);
  }
}
