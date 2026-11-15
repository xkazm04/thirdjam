# The brain, evolution 9: ten skaters who play hockey (lane A206, track Q2, the night of 14 Nov)

R-0067 point 1 (the operator after H34): "AI is the main flaw, it does not try to play hockey to achieve a goal, all teammates/opponents are not interested
in game, disengaged." Worktree `.claude/worktrees/q2b` on main b530b4eb. Status and every number with its time: `status/A206.md`.

## 1. The instrument: `Bar.Brain.Interest`
`Game/Source/JamCore/Tests/HockeyBrain9Tests.cpp` (outside the gate), driver `tools/perf/results/a206/interest.sh <log> "<options>"`, one-line bars per cell
`tools/perf/results/a206/brief.py <log>`. Real matches (RunPlaytestMatch, the ladder's scripted human, `JAM_PLAYTESTER_CHECK=closing`) traced every frame:
per side (the human's mates; the opponents) the share of bot time above half top speed, closing on the puck, standing (under 1.5 m/s, no puck, not in
its crease, not at the puck); standing by role and job; possession; a bot carry's length and end (shot / pass / lost) and whether it gains 5 m on the net;
passes completed (the next carrier a mate within 2 s); shots by distance band and AIMED (the puck leaves within 12 deg of the net) versus chips and dumps;
shots taken back by the shooter's own stick; seconds in the attacking third; a mate at the net front during an attacking-third carry; the human's open
mate ahead; loose pucks (a bot on it within 1.5 s, bots going). Options: `levels`, `carts`, `seeds`, `from`, `tune` (`|` for `;`), `player`,
`carrylog=1|2` (CARRY lines per bot carry inside 30 m; CFRAME lines with the shot gate every 10 frames or every frame).

## 2. What he saw (b530b4eb, regular rink, 20 seeds a cell)
Nobody stands (3-6 % of bot time; two thirds of it the backs on their guard post) and loose pucks are chased (2.2 bots, a bot on it in 0.3 s). What reads
as "not interested" is that nobody plays to score: the human's mates take 0.25 aimed shots a match at rookie and 0.7 at pro; a bot carry lasts about a
second and ends in a pass that arrives 31-49 % of the time or a turnover (33-60 %); at rookie the opponents (0.85 x red 0.80 = 0.68 of a mate's pace)
spend 2.4 s a match in their attacking third with nobody at the net front (0 %); 67-77 % of bots' "shots" at rookie are chips, dumps and clears.
Stills: `docs/experiments/brain9/seen-rookie-f*.jpg` (the human alone on the boards for 8 s with no other skater in the frame).

| # | rule (cause) | what is seen | number |
|---|---|---|---|
| 1 | the human's mates: `bUseAimCone` false, reach 13 m, no one-timer (FJamBotSkill defaults) | they carry through the slot and never shoot | 0.25 / 0.7 aimed shots a match |
| 2 | rookie's opposing pace 0.68 | the orange side never arrives | attacking third 2.4 s a match, net front 0 % |
| 3 | carry-or-pass passes at every arrival (mates 1.4 s) | a hot potato | carries 0.9-1.4 s |
| 4 | at full pace the dribble runs 2-3 m ahead and the facing swings | in range, no shot | CFRAME dumps |
| 5 | the board chip whenever the bot faces off the boards | the puck whacked away | 67-77 % of rookie's bot "shots" |
| 6 | the attack shape: support 4 m behind the carrier, Far at the net only past the third line | nobody at the net with him | net front 16-24 %, open mate ahead 29-46 % |
| 7 | (sim) an 18 m/s shot from a 13 m/s skater | his own stick takes it back | 7 % (rookie) to 34 % (pro opps) of bot shots |

## 3. Evolution 9 (`brain9`; every rule a FJamHockeySense value, neutral = evolution 8 as shipped)
- the human's mates shoot: aim cone (`hk_aim_cone9`), reach 1.5 + 15 m (`hk_shoot_extra9_m`), one-timer (`hk_one_timer9`), pressure time 0.6 s (`hk_pressure9_s`);
- both sides drive at the net (`hk_drive_net9_m` 22): inside 22 m with the lane open (no standing opponent within 1.5 m of the line in the 6 m ahead) the
  carrier skips carry-or-pass, skates straight (no sway) and slows to 0.55 inside its shot's range + 4 m so the puck stays on the blade and it squares up
  (`FJamBrain::DriveNet`, one call line in Attack);
- the net front (`hk_net_front9`): the Far job's first skater at the net front whenever its side has the puck in the attacking half; the close support
  level with the carrier (1 m ahead), not 4 m behind;
- the board chip only up the ice (`hk_chip_up9`);
- rookie's opponents arrive and are clumsy, not absent (item 3): pace floor 0.9 on the regular rink (`hk_opp_pace9`; was 0.68; the large rink is already
  0.9 x 1.11), and eased by execution: the poke after 20 frames in reach (`hk_poke9_frames`; 10), the aim 6 deg wide (`hk_aim_error9_deg`; the bot then
  lines up without the aim cone), the shot from inside 11.5 m (`hk_shoot_extra9_m` 10; 21.5 m). Sweep (the novice on the regular rink, 200 a cell): pace
  0.95 alone 58.7 (600); pace 0.9 71.0; poke 20 65.5; aim 6 60.0; reach 10 71.0; all four 81.5. Built and off: `hk_react9_frames` (a later read of 6
  frames broke rookie's stop window: the drill's 8.5 m stop past rookie's defender 88.5 %, bar 95), `hk_shot_charge9_frames` (rookie's wind-up 10: its
  shots fell from 2.5 to 1.3 a minute, pokes took them during the hold).
Switch: `brain9=0` (and every older `brainN=0`) = evolution 8 as shipped; `JamBaseline::BrainTune` carries it. Tests `Jam.Bot.Hockey9.Neutral`, `.OppPace`.

## 4. READY 1 (build 9d): the proof
- Neutral: `brain9=0` against lane A205's A-* rows (the H34 game, 600 a cell), 100 seeds a cell, six cells: 600 of 600 identical (result, goals, frames,
  hash). Rookie's two cells re-proved on 9d (the profile moved there last).
- Gate: the full `Jam` filter in a cleared environment, 330 Success / 0 Fail; check-test-guards PASS; Shipping compiles.
- Law 3 (200 a number; 8 / 8.5 / 9 / 9.5 m / straight): rookie 100 / 100 / 100 / 100 / 4.5 (brain9=0: 100 x 4 / 0); pro 94.0 / 99.5 / 93.0 / 83.5 / 0
  and all_star 99.5 / 99.5 / 99.0 / 93.0 / 0, both equal to brain9=0 row for row.
- The six cells (600; the novice at rookie, the regular player at pro, the good one at all_star; A205's H34 row first): rookie regular 87.3 -> 77.5,
  rookie large 86.8 -> 83.3, pro regular 48.0 -> 49.2, pro large 50.8 -> 48.3, all_star regular 27.5 -> 21.3, all_star large 24.5 -> 23.5 (pro and
  all_star on 9a: 9b-9d change rookie's profile only). Rookie regular and all_star regular are more than 4 points off: the curve lane re-sets them.
- Stills after: `docs/experiments/brain9/ready1-rookie-f*.jpg` (the orange side on him in his corner, at the boards, in front of his net).

## 5. Wave 2 (on 9d; each rule a FJamHockeySense value inside brain9, its own key)
- `hk_meet_pass9`: a team-mate's pass in flight is met where it will be (the intercept solve, up to 2 s), not chased where it is (lane A207: the aim is
  right, the receiver's blade still 4-6 m from the puck because he changes course). 40 seeds: passes completed rookie mates 36 -> 41 %, pro 39 -> 37 %:
  re-read on A207's tree.
- `hk_ahead_weak9`: the Far job's second skater 7 m ahead of the puck on the weak side, not 2 m behind it on the boards. The open man ahead of the human
  rookie 37.7 -> 41.0 %, pro 32.0 -> 38.8 % (40 seeds).
- `hk_mark_man9`: the second back of a five marks the opponent nearest our net (goal-side, moving with him) instead of standing on the slot spot
  (`MarkSlotMan`, one call line in Decide's back branch). Skaters over 10 % standing at pro 20 -> 9 of 360.
- Outside the family, one test: `Jam.Core.ModeKeys.Sprint` pinned a one-match sprint count (52 against 51 on the new match): its three matches play
  `brain9=0` explicitly (ModeKeysTests.cpp, Play() takes an optional tune).

## 6. Wave 2 on the merged tree (main d0d132bf: A207's pass and doubled shot, A209's hits; my w2b four files)
- The pass arrives now (bot passes completed 65-76 %, were 37-46 %); the opponents hardly enter the zone (2.3-2.9 s a match) and shoot from 20-30 m.
- `hk_meet_pass9` reads the sim's own pass in flight: `FJamAIWorld::PuckPassFrames / PuckPassBy`, filled in `FJamSimAdapter::BuildWorld` from
  `Puck.PassFrames / PassBy` (one line). 60 seeds: mates' shots a minute pro 0.80 -> 0.94, human wins pro 36 -> 42 of 60.
- `hk_shot_lane9`: the Weak job's first skater in the shooting lane (on the line carrier -> net, 9 m or a third of the way out) while an opposing
  carrier is inside 35 m of the net, both sides. 60 seeds: pro human wins 35 -> 43, opponents' aimed shots from 20-30 m 95 -> 80.
- `hk_drive_first9`: a driving mate passes to the human only when he is set up for the one-timer (no measurable change; kept, harmless).
- `hk_dodge9_share` (pro 0.3, all_star 0.5, rookie 0): a bot without the puck that reads the human's hit coming (closing at 3 m/s inside 4.5 m, or his
  check / cross-check winding up at it) steps off the line (`FJamBrain::DodgeHit`, one call line in Think), never against the human carrier (that first
  version emptied all_star's large rink: 59 -> 86.5 for the good player). Pro: bots felled by the human 6.70 -> 4.95 a match (my instrument).
- Not shipped: the mates' reach 24.5 m (no shot gained). Why the mates stay near 1 shot a minute: they carry inside 22 m about 3 times a match.

## 7. Wave 3 (R-0068, the eye's lists; main 015f1618 with A208's pickup)
- The pickup alone (READY 2's brain): bot passes completed fell to 42-58 %; at pro the opponents keep the puck (41 % possession) and shoot 5.8 times a
  minute; the human's pro wins fell 78 -> 42 of 100; the mates' shots a minute at rookie rose 0.69 -> 1.43.
- `hk_pass_care9` (EYE 8, item 12): no pass to a mate nearer than 5 m, none in the carrier's first 20 frames, none straight back inside 60 frames unless
  he is nearer the net, none through the own crease; a man on the carrier (2 m) lets the pass out. Re-passed within 15 frames 14-36 -> 3-4 %; straight
  back 7-18 -> 3-11 %.
- `hk_drop_no_hit9_frames` (EYE 1): no body check or cross-check press for 90 frames after a drop at rookie, 45 at pro and all_star.
- `hk_defend_team9` (EYE 2, EYE 3; the opponents only as shipped): the human is not his side's presser unless goal-side within 6 m; a defending clump
  splits at 3 m after 20 frames; a carrier inside 35 m alone over 30 frames gets the nearest bot (`NoCarrierAlone`); the crease man steps onto a shot's
  line; the guard back stands in the crease (1.5 m) while an opposing carrier is inside 35 m. On the human's side it raised goals against (pro 2.38 ->
  2.70 a match): off there; the mates' defence and the far-shot crease bar (60 %) are open.
