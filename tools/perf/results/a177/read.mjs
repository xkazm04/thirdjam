#!/usr/bin/env node
// Lane A177 (track F1; lane A157's, with -aitune): the read-outs of this lane over the raw lines of one or more arms (cells.mjs / arms1.mjs), a row an arm and cell.
//   node read.mjs downed <arm> ...     B-to-A-0041 point 2's bar: spells of a root within 0.6 m (and 0.9 m) of a downed skater's root for more than 6 frames
//   node read.mjs hits <arm> ...       item 4: landed hits where the boards answered against open ice, the frames down they gave
//   node read.mjs press <arm> ...      items 2 and 3: presses, dropped presses, whiffs, landed, lunges missed; the bots' beside the human's
// Optional first argument --first=N reads the first N seeds only (B's bar is written for 200 matches).
import { existsSync, readFileSync } from 'node:fs';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
const here = dirname(fileURLToPath(import.meta.url));
let args = process.argv.slice(2);
let first = Infinity;
if (args[0]?.startsWith('--first=')) { first = Number(args[0].slice(8)); args = args.slice(1); }
const [what, ...arms] = args;
const cells = [['rk', 'novice at rookie'], ['pro', 'regular at pro'], ['as', 'good at all_star']];
const f2 = (x) => x.toFixed(2);
for (const arm of arms) {
  for (const [key, cell] of cells) {
    const file = join(here, 'raw', `${arm}-${key}.jsonl`);
    if (!existsSync(file)) continue;
    const ms = readFileSync(file, 'utf8').split(/\r?\n/).filter(Boolean).map((l) => JSON.parse(l)).filter((m) => m.seed <= first);
    const N = Math.max(1, ms.length);
    const sum = (k) => ms.reduce((s, m) => s + (Number(m[k]) || 0), 0);
    const max = (k) => ms.reduce((s, m) => Math.max(s, Number(m[k]) || 0), 0);
    const cnt = (k) => ms.filter((m) => (Number(m[k]) || 0) > 0).length;
    const win = (100 * ms.filter((m) => m.result === 'win').length / N).toFixed(1);
    if (what === 'downed') {
      const minutes = sum('frames') / 3600;
      console.log(`${arm} ${cell}: ${ms.length} matches (${f2(sum('frames') / N / 60)} s a match), frames lying ${f2(sum('downed_frames') / N)} a match | within 0.6 m: spells over 6 frames ${sum('downed_in06_spells')} in ${cnt('downed_in06_spells')} matches, longest ${max('downed_in06_max')} frames, pair-frames ${sum('downed_in06_frames')} | within 0.9 m: spells over 6 frames ${sum('downed_in09_spells')} (${f2(sum('downed_in09_spells') / N)} a match, ${f2(300 * sum('downed_in09_spells') / Math.max(1e-9, minutes * 60))} per 300 s) in ${cnt('downed_in09_spells')} matches, longest ${max('downed_in09_max')} frames, pair-frames ${f2(sum('downed_in09_frames') / N)} a match | push-out frames ${f2(sum('downed_push_frames') / N)} a match | win ${win} %`);
    } else if (what === 'hits') {
      const bd = sum('hits_board_down'), od = sum('hits_open_down');
      console.log(`${arm} ${cell}: ${ms.length} matches | board: down ${f2(bd / N)} a match (${f2(sum('hits_board_down_frames') / Math.max(1, bd))} frames down each), stumble ${f2(sum('hits_board_stumble') / N)} | open ice: down ${f2(od / N)} a match (${f2(sum('hits_open_down_frames') / Math.max(1, od))} frames down each), stumble ${f2(sum('hits_open_stumble') / N)} | board share of landed hits ${(100 * (bd + sum('hits_board_stumble')) / Math.max(1, bd + od + sum('hits_board_stumble') + sum('hits_open_stumble'))).toFixed(1)} % | the human's own: board ${f2(sum('hits_human_board') / N)}, open ${f2(sum('hits_human_open') / N)} a match | matches with a board knockdown ${cnt('hits_board_down')}, with one by the human ${cnt('hits_human_board')}`);
    } else {
      console.log(`${arm} ${cell}: ${ms.length} matches, win ${win} % | human: presses ${f2(sum('check_presses') / N)} dropped ${f2(sum('checks_cooling') / N)} whiffs ${f2(sum('check_whiffs') / N)} landed ${f2(sum('checks_landed') / N)} lunges missed ${f2(sum('lunges_missed') / N)} | bots: presses ${f2(sum('bot_check_presses') / N)} dropped ${f2(sum('bot_checks_cooling') / N)} lunges missed ${f2(sum('bot_lunges_missed') / N)}, knockdowns of the human by a check ${f2(sum('checks_taken') / N)} | all knockdowns ${f2(sum('knockdowns_all') / N)} | goals ${f2(sum('goals_for') / N)}`);
    }
  }
}
