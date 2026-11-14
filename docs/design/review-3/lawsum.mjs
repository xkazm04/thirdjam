// Lane A169 (track F0, REVIEW-3): the laws and the carries of the three traced cells, REVIEW-2 beside REVIEW-3, one compact line a number.
// node docs/design/review-3/lawsum.mjs        (reads review-2/data and review-3/data: <cell>.laws.json of review-2/laws.mjs, <cell>.carry.json of review-1/humancarry.mjs)
import { readFileSync } from 'node:fs';
const rd = (d, n, k) => JSON.parse(readFileSync(`docs/design/${d}/data/${n}.${k}.json`, 'utf8'));
const cells = ['rk-novice', 'pro-regular', 'as-good'];
const rows = [
  ['win % of the 12 traced', (L) => L.win_pct], ['play minutes a match', (L) => L.play_min_per_match],
  ['L1 human on the ice, % of play', (L) => L.law1.human_on_the_ice_pct_of_play], ['L1 human knockdowns a minute', (L) => L.law1.human_knockdowns_per_min_trace],
  ['L1 human is victim: n, closing median, frames median', (L) => [L.law1.human_is_victim.n, L.law1.human_is_victim.closing_mps.median, L.law1.human_is_victim.frames_down.median]],
  ['L1 human victim had the puck %', (L) => L.law1.human_is_victim.victim_had_the_puck_pct],
  ['L1 human victim at 10 m/s and up: n, frames', (L) => { const b = L.law1.human_is_victim.by_closing.at(-1); return [b.n, b.frames_down_median]; }],
  ['L1 human is attacker: n, closing, frames', (L) => [L.law1.human_is_attacker.n, L.law1.human_is_attacker.closing_mps.median, L.law1.human_is_attacker.frames_down.median]],
  ['L1 all: n; frames down by closing (<2,2-4,4-6,6-8,8-10,10+)', (L) => [L.law1.all.n, L.law1.all.by_closing.map((b) => `${b.n}:${b.frames_down_median}`).join(' ')]],
  ['L1 head-on n, closing, frames | same way n, closing, frames', (L) => [L.law1.all.head_on_both_over_3mps.n, L.law1.all.head_on_both_over_3mps.closing_median, L.law1.all.head_on_both_over_3mps.frames_down_median, '|', L.law1.all.same_way_both_over_3mps.n, L.law1.all.same_way_both_over_3mps.closing_median, L.law1.all.same_way_both_over_3mps.frames_down_median]],
  ['L1 line: human stumbles, checks taken, landed', (L) => [L.law1.line.human_stumbles, L.law1.line.human_checks_taken, L.law1.line.human_checks_landed]],
  ['L2 opp fast releases a match; goals traced; at net %', (L) => [L.law2.opp.per_match_trace, L.law2.opp.goals_trace, L.law2.opp.at_net_pct]],
  ['L2 opp release distance p10 / median / p90', (L) => [L.law2.opp.dist_m.p10, L.law2.opp.dist_m.median, L.law2.opp.dist_m.p90]],
  ['L2 opp by distance (0-10,10-15,15-25,25-41,own half): pct of shots', (L) => L.law2.opp.by_distance.map((b) => b.pct).join(' / ')],
  ['L2 opp goals by distance', (L) => L.law2.opp.by_distance.map((b) => b.goals).join(' / ')],
  ['L2 opp own-half shots at net %', (L) => L.law2.opp.by_distance.at(-1).at_net_pct],
  ['L2 mates releases a match; from inside 15 m a match', (L) => [L.law2.mates.per_match_trace, Math.round(100 * (L.law2.mates.by_distance[0].per_match + L.law2.mates.by_distance[1].per_match)) / 100]],
  ['L2 human releases a match; median distance', (L) => [L.law2.human.per_match_trace, L.law2.human.dist_m.median]],
  ['L3 carries; defenders met; beaten (pct)', (L) => [L.law3.human_carries_per_match, L.law3.defenders_met_per_match, L.law3.beaten_per_match, L.law3.beaten_pct_of_met]],
  ['L3 beaten by straight / stop / cut / down', (L) => [L.law3.beaten_by.straight, L.law3.beaten_by.stop, L.law3.beaten_by.cut, L.law3.beaten_by.defender_down]],
  ['L3 not beaten: robbed / hit loose / moved', (L) => [L.law3.not_beaten.robbed, L.law3.not_beaten.hit_loose, L.law3.not_beaten.moved_the_puck]],
  ['L4 human lost per carried minute slow / skate / fast', (L) => [L.law4.human.slow.lost_per_carried_minute, L.law4.human.skate.lost_per_carried_minute, L.law4.human.fast.lost_per_carried_minute]],
  ['L4 carry sprint s: human, bots', (L) => [L.law4.line.carry_sprint_s, L.law4.line.bot_carry_sprint_s]],
  ['C  human carry median s, p90, over 3 s a match', (L, C) => [C.human_carries.s_median, C.human_carries.s_p90, C.human_carries.over_3s_per_match]],
  ['C  human carries end: lost / knocked / pass / release / loose %', (L, C) => [C.human_carries.lost, C.human_carries.knocked, C.human_carries.pass, C.human_carries.release, C.human_carries.loose]],
  ['C  opp carry median s; mate carry median s', (L, C) => [C.opp_carries.s_median, C.mate_carries.s_median]],
  ['C  puck: human / mates / opp / loose %', (L, C) => [C.puck_pct.human, C.puck_pct.mates, C.puck_pct.opp, C.puck_pct.loose]],
  ['C  after a mate pass: a match, median s, still his 1 s %, 2 s %', (L, C) => [C.human_after_mate_pass.n_per_match, C.human_after_mate_pass.s_median, C.human_after_mate_pass.held_1s_pct, C.human_after_mate_pass.held_2s_pct]],
  ['C  speed p90: human / mates / opp', (L, C) => [C.speed_mps.human.p90, C.speed_mps.mates.p90, C.speed_mps.opp.p90]],
];
for (const n of cells) {
  console.log(`===== ${n}   (REVIEW-2 -> REVIEW-3)`);
  const A = rd('review-2', n, 'laws'), B = rd('review-3', n, 'laws'), CA = rd('review-2', n, 'carry'), CB = rd('review-3', n, 'carry');
  for (const [label, f] of rows) { let a, b; try { a = f(A, CA); } catch { a = '?'; } try { b = f(B, CB); } catch { b = '?'; } console.log(`${label.padEnd(72)} ${[].concat(a).join(' ')}  ->  ${[].concat(b).join(' ')}`); }
}
