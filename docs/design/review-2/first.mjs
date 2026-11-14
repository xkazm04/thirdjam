// Lane A148 (track F0, REVIEW-2): a player's first matches from playtest lines (.jsonl of tools/playtest/run.mjs, one or more files).
// node docs/design/review-2/first.mjs <file.jsonl> [more.jsonl ...] [player=novice]      One JSON line per cell (cartridge, level, player). It measures only.
// Per cell: the result, the clock, the first touch / shot / goal, what he does and what is done to him per match and per minute of play, and for each
// verb the number of matches in which he used it at all (the line carries counts, not the frame of a first use).
import { readFileSync } from 'node:fs';
const args = process.argv.slice(2);
const filt = args.filter((a) => a.includes('=')).map((s) => s.split('='));
const rows = [];
for (const f of args.filter((a) => !a.includes('='))) for (const l of readFileSync(f, 'utf8').split(/\r?\n/)) { const i = l.indexOf('{"tool":"thirdjam.playtest/1"'); if (i < 0) continue; try { rows.push(JSON.parse(l.slice(i))); } catch { /* cut line */ } }
const groups = new Map();
for (const r of rows) { if (!filt.every(([k, v]) => String(r[k]) === v)) continue; const key = `${r.cartridge}|${r.difficulty}|${r.player}`; if (!groups.has(key)) groups.set(key, []); groups.get(key).push(r); }
const avg = (a) => (a.length ? a.reduce((s, x) => s + x, 0) / a.length : 0);
const med = (a) => { if (!a.length) return -1; const s = [...a].sort((x, y) => x - y); return s[s.length >> 1]; };
const r1 = (x) => Math.round(x * 10) / 10, r2 = (x) => Math.round(x * 100) / 100;
for (const [key, ms] of groups) {
  const [cartridge, level, player] = key.split('|');
  const n = ms.length, f = (k) => avg(ms.map((m) => m[k] || 0)), min = avg(ms.map((m) => m.play_frames)) / 3600;
  const used = (k) => ms.filter((m) => (m[k] || 0) > 0).length;
  const hold = ms.reduce((a, m) => { if (Array.isArray(m.hold_frames)) for (let i = 0; i < 3; i++) a[i] += m.hold_frames[i]; return a; }, [0, 0, 0]); const hs = hold[0] + hold[1] + hold[2];
  console.log(JSON.stringify({ cartridge, level, player, n, win_pct: Math.round(100 * ms.filter((m) => m.result === 'win').length / n), lost: ms.filter((m) => m.result === 'loss').length,
    match_min: r1(avg(ms.map((m) => m.frames)) / 3600), play_min: r1(min), goals_for: r1(avg(ms.map((m) => m.goals[0]))), goals_against: r1(avg(ms.map((m) => m.goals[1]))), goals_by_human: r1(f('goals_by_human')),
    first_touch_s: r1(med(ms.map((m) => m.first_touch).filter((x) => x >= 0)) / 60), first_shot_s: r1(med(ms.map((m) => m.first_shot).filter((x) => x >= 0)) / 60), no_shot_matches: ms.filter((m) => m.first_shot < 0).length,
    first_goal_by_him_s: r1(med(ms.map((m) => m.first_goal_by_human).filter((x) => x >= 0)) / 60), goal_by_him_inside_2min: ms.filter((m) => m.first_goal_by_human >= 0 && m.first_goal_by_human <= 7200).length, goal_by_him_at_all: ms.filter((m) => m.first_goal_by_human >= 0).length,
    first_goal_against_s: r1(med(ms.map((m) => m.first_goal_against).filter((x) => x >= 0)) / 60),
    shots: r1(f('shots')), shots_mates: r1(f('shots_mates')), shots_opp: r1(f('shots_opp')), carry_pct: r1(100 * avg(ms.map((m) => m.carry_frames / m.play_frames))), carries_3s: r1(f('carries_3s')), touches_per_min: r1(f('touches_per_min')),
    knockdowns_taken: r1(f('knockdowns')), knockdowns_taken_per_min: r2(f('knockdowns') / min), stumbles: r1(f('stumbles')), stumbles_per_min: r2(f('stumbles') / min), hits_landed: r1(f('checks_landed')), hits_landed_per_min: r2(f('checks_landed') / min),
    fouls: r1(f('fouls')), matches_with_a_foul: used('fouls'), box_s: r1(f('box_frames') / 60),
    pokes: r1(f('pokes')), pokes_clean: r1(f('pokes_clean')), pokes_clean_pct: r1(100 * f('pokes_clean') / Math.max(1e-9, f('pokes'))), lifts: r1(f('lifts')), passes: r1(f('passes')), passes_received: r1(f('passes_received')), passes_received_per_min: r2(f('passes_received') / min),
    one_timers: r1(f('one_timers')), breakaways: r1(f('breakaways')), dekes: r1(f('dekes')), cuts: r1(f('cuts')), rev_brakes: r1(f('rev_brakes')), carry_sprint_s: r1(f('carry_sprint_frames') / 60), sprint_s: r1(f('sprint_frames') / 60),
    bot_rev_brakes: r1(f('bot_rev_brakes')), bot_rev_brakes_per_min: r1(f('bot_rev_brakes') / min), bot_carry_sprint_frames: r1(f('bot_carry_sprint_frames')), board_hits: r1(f('board_hits')), knockdowns_all: r1(f('knockdowns_all')),
    idle_s: r1(f('idle_frames') / 60), idle_s_per_min: r1(f('idle_frames') / 60 / min), max_no_touch_s: r1(f('max_no_touch') / 60), matches_with_30s_without_the_puck: ms.filter((m) => m.max_no_touch >= 1800).length,
    puck_held_pct_side_opp_nobody: hs ? hold.map((x) => Math.round(100 * x / hs)) : null,
    matches_in_which_he: { landed_a_hit: used('checks_landed'), poked: used('pokes'), took_a_puck_by_a_poke: used('pokes_clean'), passed: used('passes'), received_a_pass: used('passes_received'), shot: n - ms.filter((m) => m.first_shot < 0).length, scored: ms.filter((m) => m.first_goal_by_human >= 0).length, held_it_3s: used('carries_3s'), braked_by_reversing: used('rev_brakes'), cut: used('cuts'), deked: used('dekes'), sprinted: used('sprint_frames'), sprinted_with_the_puck: used('carry_sprint_frames'), lifted: used('lifts'), one_timed: used('one_timers'), was_knocked_down: used('knockdowns'), stumbled: used('stumbles') } }));
}
