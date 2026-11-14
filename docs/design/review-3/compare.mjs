// Lane A169 (track F0, REVIEW-3): before and after, cell by cell. It reads two files of cells (the one-line-a-cell output of review-2/first.mjs) and prints,
// for every cell that is in both, the chosen fields as "before -> after".
// node docs/design/review-3/compare.mjs <before.cells.jsonl> <after.cells.jsonl> [player=novice] [level=rookie] [cartridge=hockey.regular] [fields=a,b,c]
import { readFileSync } from 'node:fs';
const args = process.argv.slice(2);
const files = args.filter((a) => !a.includes('='));
const opt = Object.fromEntries(args.filter((a) => a.includes('=')).map((s) => s.split('=')));
const read = (f) => readFileSync(f, 'utf8').split(/\r?\n/).filter((l) => l.startsWith('{')).map((l) => JSON.parse(l));
const A = read(files[0]), B = read(files[1]);
const DEF = ['n', 'win_pct', 'match_min', 'play_min', 'goals_for', 'goals_against', 'first_shot_s', 'no_shot_matches', 'first_goal_by_him_s', 'goal_by_him_inside_2min', 'shots', 'shots_mates', 'shots_opp',
  'carries_3s', 'carry_pct', 'touches_per_min', 'knockdowns_taken', 'knockdowns_taken_per_min', 'stumbles', 'hits_landed', 'hits_landed_per_min', 'matches_with_a_foul', 'box_s', 'pokes', 'pokes_clean', 'pokes_clean_pct',
  'passes', 'passes_received', 'passes_received_per_min', 'one_timers', 'rev_brakes', 'bot_rev_brakes', 'bot_carry_sprint_frames', 'idle_s_per_min', 'max_no_touch_s', 'matches_with_30s_without_the_puck', 'puck_held_pct_side_opp_nobody'];
const fields = opt.fields ? opt.fields.split(',') : DEF;
for (const b of B) {
  if (opt.player && b.player !== opt.player) continue; if (opt.level && b.level !== opt.level) continue; if (opt.cartridge && b.cartridge !== opt.cartridge) continue;
  const a = A.find((x) => x.cartridge === b.cartridge && x.level === b.level && x.player === b.player);
  const row = {}; for (const f of fields) row[f] = a ? `${JSON.stringify(a[f])} -> ${JSON.stringify(b[f])}` : `? -> ${JSON.stringify(b[f])}`;
  console.log(`${b.cartridge} | ${b.level} | ${b.player}`); console.log('  ' + Object.entries(row).map(([k, v]) => `${k} ${v}`).join('; '));
}
