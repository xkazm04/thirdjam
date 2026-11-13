// Lane A100 (track F0, REVIEW-1): one cell's first-five-minutes numbers from playtest lines (a .jsonl of run.mjs or a raw log). node cell.mjs <file> [filter k=v ...]
import { readFileSync } from 'node:fs';
const rows = [];
for (const l of readFileSync(process.argv[2], 'utf8').split(/\r?\n/)) { const i = l.indexOf('{"tool":"thirdjam.playtest/1"'); if (i < 0) continue; try { rows.push(JSON.parse(l.slice(i))); } catch {} }
const filt = process.argv.slice(3).map((s) => s.split('='));
const groups = new Map();
for (const r of rows) { if (!filt.every(([k, v]) => String(r[k]) === v)) continue; const key = `${r.cartridge} ${r.difficulty} ${r.player}`; if (!groups.has(key)) groups.set(key, []); groups.get(key).push(r); }
const avg = (a) => (a.length ? a.reduce((s, x) => s + x, 0) / a.length : 0);
const med = (a) => { if (!a.length) return -1; const s = [...a].sort((x, y) => x - y); return s[s.length >> 1]; };
const r1 = (x) => Math.round(x * 10) / 10;
for (const [key, ms] of groups) {
  const n = ms.length, f = (k) => r1(avg(ms.map((m) => m[k] || 0)));
  const min = avg(ms.map((m) => m.play_frames)) / 3600;
  const within = (k, fr) => ms.filter((m) => m[k] >= 0 && m[k] <= fr).length;
  console.log(JSON.stringify({ cell: key, n, win_pct: Math.round(100 * ms.filter((m) => m.result === 'win').length / n), play_min: r1(min), match_min: r1(avg(ms.map((m) => m.frames)) / 3600),
    gf: f('goals') , goals_for: r1(avg(ms.map((m) => m.goals[0]))), goals_against: r1(avg(ms.map((m) => m.goals[1]))), goals_by_human: f('goals_by_human'),
    human_goal_within_2min: within('first_goal_by_human', 7200), human_goal_ever: ms.filter((m) => m.first_goal_by_human >= 0).length, first_human_goal_s_med: r1(med(ms.map((m) => m.first_goal_by_human).filter((x) => x >= 0)) / 60),
    first_goal_against_s_med: r1(med(ms.map((m) => m.first_goal_against).filter((x) => x >= 0)) / 60), first_shot_s_med: r1(med(ms.map((m) => m.first_shot).filter((x) => x >= 0)) / 60), no_shot_matches: ms.filter((m) => m.first_shot < 0).length,
    shots: f('shots'), shots_mates: f('shots_mates'), shots_opp: f('shots_opp'), touches_per_min: f('touches_per_min'), carry_pct: r1(100 * avg(ms.map((m) => m.carry_frames / m.play_frames))), carries_3s: f('carries_3s'),
    knockdowns: f('knockdowns'), knockdowns_per_min: r1(f('knockdowns') / min), checks_landed: f('checks_landed'), checks_taken: f('checks_taken'), fouls: f('fouls'), fouls_from_behind: f('fouls_from_behind'), fouls_no_puck: f('fouls_no_puck'), matches_with_a_foul: ms.filter((m) => m.fouls > 0).length, breakaways: f('breakaways'), box_s: r1(f('box_frames') / 60),
    pokes: f('pokes'), pokes_clean: f('pokes_clean'), lifts: f('lifts'), passes: f('passes'), passes_received: f('passes_received'), one_timers: f('one_timers'), dekes: f('dekes'), cuts: f('cuts'), rev_brakes: f('rev_brakes'), bot_rev_brakes: f('bot_rev_brakes'),
    carry_sprint_s: r1(f('carry_sprint_frames') / 60), bot_carry_sprint_frames: f('bot_carry_sprint_frames'), idle_spells: f('idle_spells'), max_no_touch_s: r1(f('max_no_touch') / 60), faceoffs: f('faceoffs'), fwo: ms.reduce((s, m) => s + m.faceoffs_without_touch, 0) }));
}
