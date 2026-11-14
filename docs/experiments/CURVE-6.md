# The difficulty curve, pass 6: the ladder on the kept puck, the whistle out, the first ten minutes (lane A112, track C0, 2026-11-13 20:32-)

Scope (brief `briefs/A112-C0-curve.md`, R-0048, R-0050): main 25df19f carries `game.stick.passive_take` false and `game.stick.body_shield` true (lane A102,
`MOVEMENT-2.md`): the human keeps the puck and the ladder of R-0046 moved to 99.8 / 72.0 / 39.8 (targets 85 / 48 / 26). This pass re-sets it, aggression first.
Driver `tools/perf/results/a112/curve.mjs` (A102's copy; any cartridge as the rink; `noshot` and `fouls` read-outs); rows in `tools/perf/results/a112/cells.jsonl`,
raw matches in `tools/perf/results/a112/raw/` (not landed). Status with timestamps: `status/A112.md`.

(tables follow as they complete)

## 1. Base: main 25df19f as built (600 a cell; A53's seeds on hockey.regular, seeds 1-600 on hockey.arena.large; whistle on)

| cell | target | hockey.regular | hockey.arena.large |
|---|---|---|---|
| novice at rookie | 85 | 99.3 (98.3-99.7); goals 2.99 : 0.26 | 97.3 (95.7-98.4) |
| regular at pro | 48 | 71.2 (67.4-74.6) | 75.7 (72.1-78.9) |
| good at all_star | 26 | 40.0 (36.2-44.0) | 54.8 (50.8-58.8) |

## 2. The levers (600, hockey.regular, `-aitune` on the build; every row in section 7)

| level | lever | win % (base) | read |
|---|---|---|---|
| rookie | poke after 20 / 10 (+checks 4) / 6 frames (never) | 99.5 / 97.7 / 96.8 (99.3) | a late poke never arrives; 6 frames is the first that takes pucks |
| rookie | checks from 4 / 2 m/s (6) | 99.3 / 99.2 | checks taken 2.0 -> 5.5 and nothing: a loose puck is not a goal for a rookie |
| rookie | hunt 6 m (0); red skill 1.1 (0.97) | 99.3; 98.7 | speed is no lever at rookie |
| rookie | shot range reach+20 / +25 / +30 (5) with the poke 6 | 98.0 (no poke) / **86.2** / 82.3 | the rookie scored 0.26 a match: it has to shoot when it has the puck |
| rookie | + lift, poke 0 | 64.7 | too much |
| pro | hunt 14 m (8) | 66.0 (71.2) | |
| pro | checks from 1 m/s (2); sprint 0.7 (0.35) | 71.5; 74.2 | nothing alone |
| pro | **poke after 2 frames (6)** | **60.8** | the lever: the pro's stick before the carrier turns his body |
| pro | poke 2 + hunt 20 + checks 1 (+ sprint 0.7 + lift cooldown 45) | 50.2 (2,000) / 44.5 (2,000) | |
| pro | shot range 45 + hunt 14; red skill 1.15 + hunt 14 | 48.7; 64.8 | speed again weak |
| all_star | mates at the human's pace (`mate_speed_scale` 1; was 1.4) | 37.5 (40.0) | R-0048 point 4 is itself a lever |
| all_star | + hunt 30 (20) + shot range 60 (40) | **29.7**; 32.0 at 2,000 | |
| all_star | + shot range 80 / wind-up 6 / wind-up 14 / block shots / draw jump 25 / mates' passes 0 / hunt-from-behind / carry-more (pressure 0.1, better-by 20) / no lift | 29.8 / 32.3 / 35.5 / 36.3 / 29.3 (32.5 at 2,000) / 30.5 / 35.7 / 34.2 / 37.3 | saturated: nothing below ~30 |
| all_star | speed 1.3 / 1.45 / 1.5; red 0.8 / 1.0 | 36.2 / 35.7 / 44.5; 42.8 / 43.0 | 1.4 and 0.9 stay the optimum (A95's non-monotonic top) |

## 3. The re-set as written (`DifficultyProfile.cpp`, `JamAICore.h` ProOppSkill), each final at 2,000 with the numbers as a tune (whistle on)

| level | number | before | after |
|---|---|---|---|
| rookie | poke | never | **after 6 frames** (R-0050 point 4: the curve track's call) |
| rookie | shot range (reach +) | 5 m | **25 m** |
| rookie | large-rink red skill | 0.9 | **0.98** (0.9 94.9; 0.93 93.3; 0.96 89.7; 1.02 77.7) |
| pro | poke | after 6 frames | **after 2** |
| pro | hunt radius | 8 m | **20 m** |
| pro | checks from | 2 m/s | **1 m/s** |
| pro | large-rink red skill | 1.04 | **1.065** (1.04: 57.1) |
| all_star | hunt radius / shot range | 20 / 40 m | **30 / 60 m** |
| all_star | large-rink speed | 1.22 | **1.34** (1.27 37.3, 1.32 29.1 at 2,000) |
| every level | the human's mates' speed scale (new profile number + tune key `mate_speed_scale`, 0 = speed_scale) | = speed_scale (rookie 0.85, all_star 1.4 / 1.22) | **1.0** (the human's pace) |

| cell | target | hockey.regular (2,000) | hockey.arena.large (2,000) |
|---|---|---|---|
| novice at rookie | 85 | **85.5** (83.9-87.0) | **87.7** (86.2-89.1) |
| regular at pro | 48 | **50.2** (48.0-52.4) | **50.9** (48.7-53.1) |
| good at all_star | 26 | 32.0 (30.0-34.1) | 29.1 (27.2-31.1) at 1.32; 1.34 written, not re-measured |

## 4. The content items and what they did to the cells (as built, 600, whistle off)

`rules.penalties` false in hockey.regular, hockey.arena.wide, hockey.arena.large, take-1 .. take-8 and the four fallbacks. Card lines (season.ladder.json): take-1
"find the net twice", take-2 "more ice", take-3 "Drop the manners. Hit them, but keep the puck in frame.", take-4 "props", take-5 "rubber boards ... second wind",
take-6 "the last goal", take-7 "widescreen", take-8 "hits allowed, and the set may close in": none is about the whistle or the box (take-3 and take-8 are about
hitting, which the open whistle serves): all lose it. take-3 rookie.

| cell | whistle on (2,000, sec. 3) | **whistle off, as built (600)** |
|---|---|---|
| novice at rookie, regular / large | 85.5 / 87.7 | **90.3** (87.7-92.4) / **94.0** (91.8-95.6) |
| regular at pro, regular / large | 50.2 / 50.9 | **56.5** (52.5-60.4) / **61.7** (57.7-65.5) |
| good at all_star, regular / large | 32.0 / 29.1 (1.32) | **33.5** (29.8-37.4) / **25.0** (21.7-28.6) at 1.34 |

The whistle was the human's handicap: the playtesters foul 0.7 to 1.4 times a match (300 frames in the box each), the bots hardly. Off, every cell rises
5 to 10 points. Trims measured on the whistle-off build (600): rookie shot range 30: 87.2 regular / 78.5 large; pro sprint 0.7 + lift cooldown 45: 56.3 regular (nothing) / **48.5** large (-13).

## 5. The season in order (whistle off, take-3 rookie; 200 seeds a take, each take at its own level)

| take (level) | novice | regular | good |
|---|---|---|---|
| 1 (rookie) | 81.5 | 98.5 | 99.5 |
| 2 (rookie, large, timer) | 95.5 | 100 | 100 |
| 3 (rookie) | **88.5**; 2 of 200 without a novice shot | 99.0 | 100 |
| 4 (pro) | 25.5 | 60.0 | 79.0 |
| 5 (pro) | 21.0 | 61.5 | 87.0 |
| 6 (pro) | 16.0 | 54.0 | 85.0 |
| 7 (pro, wide) | 30.0 | 62.5 | 91.0 |
| 8 (all_star) | 6.0 | 18.5 | 41.0 |

Take-3 passes R-0048's bar (novice 60 % or more; a shot in 198 of 200). The step 3 -> 4 is 63 points for the novice and 39 for the regular player: the
"no step over 20" clause is **not met**; it needs takes 4 and 5 to ramp (section 6, next 2).

## 6. Gate: NOT green on this tree (full `Jam` filter, `Game/logs/a112-jam-1.log`: 168 pass, 7 fail)

| test | cause | fix (not yet made unless said) |
|---|---|---|
| `Jam.Core.Movement.StopAndGo` | the pro poke after 2 frames beats A102's clean stop-and-go: 43 % (bar 80): law 3's proof | a design call: the stop's window is shorter than the pro's poke. Either the drill keeps the old defender (`opp.poke_frames=6`) or the poke returns to 6 and pro loses about 10 points of its re-set (to be re-found in hunt / checks / shot range) |
| `Jam.Core.ContactBoardMatch` | its neutral arm pins main's pro hashes; the pro profile moved | re-pin, or give the neutral arm `JamAI::PreA95Tune(Pro)` as `MovementNeutral` does |
| `Jam.Core.ModeKeys.Sprint` | "without -playersprint the human sprints only with the puck" 26 vs 31 frames on the new pro | look at the frames (likely a sprint in a scramble the faster poke makes) |
| `Jam.Designer.CutsRoundTrip`, `Jam.Designer.PromptParity` | the Designer's cached cuts and its prompt are built from hockey.regular's text: `penalties` changed | regenerate the Designer data (`JamDesignerData.inl`, parity) per `schema/README.md`, cuts with `penalties` false |
| `Jam.Effects.BotMatchWithEffects` | "every take started an effect" | not diagnosed (the takes' content changed: whistle and take-3's level) |
| `Jam.Progression.SeasonRoundTrip` | asserted take-3 pro | fixed in source (`ProgressionTests.cpp:162`), not rebuilt |

Fixed and green: `Jam.Core.Stick` (regular has no whistle; its foul cases switch it on), `Jam.Core.ModeKeys.PenaltyFrames` (variants swap `"penalties": false`),
`Jam.Core.DifficultyProfiles` (every level pokes, rookie latest), `MovementNeutral` and `ContactNeutral` (PreA95Tune carries this lane's old values).

## 7. All rows

| label | class / level / rink | tune | n | win % (95 %) | checks taken | goals for : against | opp shots | carry % |
|---|---|---|---|---|---|---|---|---|
| B-rk-reg | novice / rookie / regular | (profile) | 600 | 99.3 (98.3-99.7) | 2.03 | 2.99 : 0.26 | 6.9 | 18 |
| L1-rk-poke20 | novice / rookie / regular | `opp.poke_frames=20` | 600 | 99.5 (98.5-99.8) | 2.16 | 2.98 : 0.32 | 7.6 | 16.8 |
| B-pro-reg | regular / pro / regular | (profile) | 600 | 71.2 (67.4-74.6) | 7.82 | 2.49 : 1.55 | 13.3 | 14.3 |
| L1-pro-hunt14 | regular / pro / regular | `opp.hunt_radius_m=14` | 600 | 66 (62.1-69.7) | 8.7 | 2.37 : 1.7 | 14.3 | 13.4 |
| B-as-reg | good / all_star / regular | (profile) | 600 | 40 (36.2-44) | 9.3 | 1.82 : 2.28 | 17.4 | 12.5 |
| L1-as-hunt30 | good / all_star / regular | `opp.hunt_radius_m=30` | 600 | 37.7 (33.9-41.6) | 9.1 | 1.77 : 2.3 | 17 | 12.3 |
| B-rk-large | novice / rookie / large | (profile) | 600 | 97.3 (95.7-98.4) | 3.56 | 4.24 : 0.59 | 11.1 | 31.1 |
| L2-rk-poke6 | novice / rookie / regular | `opp.poke_frames=6` | 600 | 96.8 (95.1-98) | 2.33 | 2.9 : 0.55 | 10.2 | 13.5 |
| L1-rk-check4 | novice / rookie / regular | `opp.check_min_closing=4` | 600 | 99.3 (98.3-99.7) | 4.07 | 2.98 : 0.29 | 7.9 | 16.4 |
| B-pro-large | regular / pro / large | (profile) | 600 | 75.7 (72.1-78.9) | 5.01 | 2.29 : 1.17 | 16.5 | 16 |
| L2-rk-check2 | novice / rookie / regular | `opp.check_min_closing=2` | 600 | 99.2 (98.1-99.6) | 5.49 | 2.98 : 0.33 | 8.4 | 15.4 |
| L1-pro-check1 | regular / pro / regular | `opp.check_min_closing=1` | 600 | 71.5 (67.8-75) | 8.03 | 2.49 : 1.55 | 13.4 | 14.2 |
| B-as-large | good / all_star / large | (profile) | 600 | 54.8 (50.8-58.8) | 5.6 | 2.88 : 2.46 | 15.2 | 16.9 |
| L1-as-check1 | good / all_star / regular | `opp.check_min_closing=1` | 600 | 40.5 (36.6-44.5) | 9.62 | 1.86 : 2.26 | 17.4 | 12.4 |
| L2-rk-combo | novice / rookie / regular | `opp.poke_frames=6;opp.check_min_closing=2;opp.hunt_radius_m=8;opp.sprint_share=0.35` | 600 | 94.5 (92.4-96.1) | 6.38 | 2.82 : 0.62 | 12.3 | 11.6 |
| L3-rk-shoot20 | novice / rookie / regular | `opp.shoot_extra_m=20` | 600 | 98 (96.5-98.9) | 2.03 | 2.98 : 0.47 | 9.8 | 18.1 |
| L1-rk-poke10-check4 | novice / rookie / regular | `opp.poke_frames=10;opp.check_min_closing=4` | 600 | 97.7 (96.1-98.6) | 4.39 | 2.94 : 0.49 | 9.7 | 13.9 |
| L2-pro-combo | regular / pro / regular | `opp.hunt_radius_m=20;opp.check_min_closing=1;opp.poke_frames=2;opp.sprint_share=0.7;opp.lift_cooldown_frames=45` | 600 | 45.5 (41.6-49.5) | 8.78 | 2.03 : 2.16 | 16.8 | 11.4 |
| L3-rk-shoot30-poke6 | novice / rookie / regular | `opp.shoot_extra_m=30;opp.poke_frames=6` | 600 | 82.3 (79.1-85.2) | 2.29 | 2.67 : 1.23 | 20.3 | 13.3 |
| L1-pro-sprint07 | regular / pro / regular | `opp.sprint_share=0.7` | 600 | 74.2 (70.5-77.5) | 7.43 | 2.55 : 1.42 | 12.9 | 14.4 |
| L2-as-combo | good / all_star / regular | `opp.hunt_radius_m=30;opp.check_min_closing=1;opp.lift_cooldown_frames=20` | 600 | 38.2 (34.4-42.1) | 9.21 | 1.76 : 2.31 | 16.9 | 12.2 |
| L3-rk-shoot30-poke0-lift | novice / rookie / regular | `opp.shoot_extra_m=30;opp.poke_frames=0;opp.lift=1;opp.lift_cooldown_frames=90` | 600 | 64.7 (60.8-68.4) | 2.36 | 2.31 : 1.61 | 26.2 | 10.2 |
| L1-pro-poke2 | regular / pro / regular | `opp.poke_frames=2` | 600 | 60.8 (56.9-64.7) | 6.91 | 2.28 : 1.79 | 15 | 13.1 |
| L2-rk-red11 | novice / rookie / regular | `red_skill=1.1` | 600 | 98.7 (97.4-99.3) | 1.94 | 2.98 : 0.33 | 7.3 | 16 |
| L5-rk-large | novice / rookie / large | `opp.shoot_extra_m=30;opp.poke_frames=6` | 600 | 82.5 (79.3-85.3) | 2.35 | 2.16 : 0.81 | 16.5 | 26.5 |
| L4-pro-hunt14-shoot45 | regular / pro / regular | `opp.hunt_radius_m=14;opp.shoot_extra_m=45` | 600 | 48.7 (44.7-52.7) | 8.32 | 2.02 : 2.08 | 18.4 | 13.8 |
| L3-rk-draw50 | novice / rookie / regular | `opp.shoot_extra_m=30;opp.poke_frames=6;opp.draw_jump_pct=50` | 600 | 83.2 (80-85.9) | 2.74 | 2.69 : 1.17 | 19.6 | 13.2 |
| L1-rk-hunt6 | novice / rookie / regular | `opp.hunt_radius_m=6` | 600 | 99.3 (98.3-99.7) | 2.56 | 2.99 : 0.28 | 6.9 | 17.2 |
| L5-pro-large | regular / pro / large | `opp.hunt_radius_m=20;opp.check_min_closing=1;opp.poke_frames=2;opp.sprint_share=0.7;opp.lift_cooldown_frames=45` | 600 | 47.3 (43.4-51.3) | 3.89 | 1.4 : 1.43 | 18.6 | 11.2 |
| L6-as-mate10 | good / all_star / regular | `mate_speed_scale=1` | 600 | 37.5 (33.7-41.4) | 9.98 | 1.78 : 2.3 | 17.2 | 12 |
| L4-pro-hunt14-red115 | regular / pro / regular | `opp.hunt_radius_m=14;red_skill=1.15` | 600 | 64.8 (60.9-68.5) | 9.14 | 2.35 : 1.64 | 14.7 | 13.3 |
| L5-rk-shoot25 | novice / rookie / regular | `opp.shoot_extra_m=25;opp.poke_frames=6` | 600 | 86.2 (83.2-88.7) | 2.27 | 2.76 : 1.04 | 17.7 | 13.4 |
| L6-as-mate10-shoot60 | good / all_star / regular | `mate_speed_scale=1;opp.shoot_extra_m=60;opp.hunt_radius_m=30` | 600 | 29.7 (26.2-33.4) | 9.35 | 1.54 : 2.47 | 19.8 | 12.2 |
| L4-as-shoot60 | good / all_star / regular | `opp.shoot_extra_m=60` | 600 | 38.3 (34.5-42.3) | 9.34 | 1.79 : 2.27 | 20.6 | 12.7 |
| L7-as-m1-red10 | good / all_star / regular | `mate_speed_scale=1;red_skill=1.0` | 600 | 43 (39.1-47) | 10.17 | 1.9 : 2.16 | 18.4 | 12.4 |
| L6-as-mate10-large | good / all_star / large | `mate_speed_scale=1` | 600 | 45 (41.1-49) | 5.89 | 2.77 : 2.94 | 16 | 16 |
| L4-as-red10 | good / all_star / regular | `opp.hunt_radius_m=30;red_skill=1.0` | 600 | 46 (42.1-50) | 9.46 | 1.98 : 2.18 | 18.7 | 12.5 |
| L7-as-m1-sp15 | good / all_star / regular | `mate_speed_scale=1;speed_scale=1.5` | 600 | 44.5 (40.6-48.5) | 10.14 | 1.89 : 2.16 | 18.2 | 12.2 |
| L8-as-shoot80 | good / all_star / regular | `mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=80` | 600 | 29.8 (26.3-33.6) | 9.72 | 1.55 : 2.46 | 22.8 | 12.1 |
| L7-as-m1-red08 | good / all_star / regular | `mate_speed_scale=1;red_skill=0.8` | 600 | 42.8 (38.9-46.8) | 9.32 | 1.86 : 2.2 | 16 | 12.6 |
| L8-as-shoot60-large | good / all_star / large | `mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60` | 600 | 53.8 (49.8-57.8) | 5.76 | 2.8 : 2.6 | 18.5 | 16.2 |
| L8-as-shoot60-chg6 | good / all_star / regular | `mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60;opp.shot_charge_frames=6` | 600 | 32.3 (28.7-36.2) | 9.65 | 1.62 : 2.41 | 26.8 | 12 |
| L9-as-large-sp127 | good / all_star / large | `mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60;speed_scale=1.27` | 600 | 37.3 (33.6-41.3) | 5.42 | 2.24 : 2.73 | 18.4 | 15.1 |
| L9-as-sp13 | good / all_star / regular | `mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60;speed_scale=1.3` | 600 | 36.2 (32.4-40.1) | 9.38 | 1.79 : 2.36 | 19.5 | 12.3 |
| F1-rk-reg | novice / rookie / regular | `opp.shoot_extra_m=25;opp.poke_frames=6;mate_speed_scale=1` | 2000 | 85.5 (83.9-87) | 2.48 | 2.73 : 1.08 | 18.7 | 12.4 |
| L9-as-large-sp132 | good / all_star / large | `mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60;speed_scale=1.32` | 600 | 28.3 (24.9-32.1) | 5.49 | 1.66 : 2.73 | 18.8 | 12.9 |
| F1-pro-reg | regular / pro / regular | `opp.hunt_radius_m=20;opp.check_min_closing=1;opp.poke_frames=2;opp.sprint_share=0.7;opp.lift_cooldown_frames=45;mate_speed_scale=1` | 2000 | 44.5 (42.3-46.6) | 8.62 | 1.94 : 2.16 | 16.8 | 11.4 |
| F2-as-reg | good / all_star / regular | `mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60` | 2000 | 32 (30-34.1) | 9.62 | 1.61 : 2.43 | 20.1 | 12 |
| L10-as-sp145 | good / all_star / regular | `mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60;speed_scale=1.45` | 600 | 35.7 (31.9-39.6) | 10.3 | 1.73 : 2.33 | 23.2 | 11.9 |
| F1-rk-large | novice / rookie / large | `opp.shoot_extra_m=25;opp.poke_frames=6;mate_speed_scale=1` | 2000 | 94.9 (93.8-95.8) | 1.94 | 2.39 : 0.37 | 15.5 | 28.3 |
| L11-as-chg14 | good / all_star / regular | `mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60;opp.shot_charge_frames=14` | 600 | 35.5 (31.8-39.4) | 9.64 | 1.71 : 2.35 | 18.1 | 12.2 |
| L11-as-block | good / all_star / regular | `mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60;opp.block_shots=1;opp.block_min_dist_m=10` | 600 | 36.3 (32.6-40.3) | 9.64 | 1.69 : 2.37 | 20.2 | 12.2 |
| F3-pro-reg | regular / pro / regular | `opp.hunt_radius_m=20;opp.check_min_closing=1;opp.poke_frames=2;mate_speed_scale=1` | 2000 | 50.2 (48-52.4) | 8.37 | 2.04 : 2.02 | 16.1 | 11.9 |
| L11-as-mpass0 | good / all_star / regular | `mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60;mate.pass_to_human=0` | 600 | 30.5 (26.9-34.3) | 9.36 | 1.56 : 2.45 | 19.6 | 12.2 |
| L11-as-draw25 | good / all_star / regular | `mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60;opp.draw_jump_pct=25` | 600 | 29.3 (25.8-33.1) | 9.3 | 1.55 : 2.47 | 19.9 | 12.2 |
| L12-rk-large-red093 | novice / rookie / large | `opp.shoot_extra_m=25;opp.poke_frames=6;mate_speed_scale=1;red_skill=0.93` | 600 | 93.3 (91-95.1) | 2.17 | 2.31 : 0.42 | 17 | 26.7 |
| F1-pro-large | regular / pro / large | `opp.hunt_radius_m=20;opp.check_min_closing=1;opp.poke_frames=2;opp.sprint_share=0.7;opp.lift_cooldown_frames=45;mate_speed_scale=1` | 2000 | 46.9 (44.7-49) | 3.94 | 1.39 : 1.44 | 18.6 | 11.2 |
| L13-as-carry | good / all_star / regular | `mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60;opp.hk_pressure_s=0.1;opp.hk_better_by_m=20` | 600 | 34.2 (30.5-38.1) | 9.45 | 1.68 : 2.38 | 21 | 11.4 |
| L12-rk-large-red096 | novice / rookie / large | `opp.shoot_extra_m=25;opp.poke_frames=6;mate_speed_scale=1;red_skill=0.96` | 600 | 89.7 (87-91.9) | 2.35 | 2.03 : 0.5 | 17.7 | 23.7 |
| L14-rk-large-red102 | novice / rookie / large | `opp.shoot_extra_m=25;opp.poke_frames=6;mate_speed_scale=1;red_skill=1.02` | 600 | 77.7 (74.2-80.8) | 2.18 | 1.58 : 0.61 | 19 | 18.9 |
| L13-as-behind | good / all_star / regular | `mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60;opp.check_from_behind=1` | 600 | 35.7 (31.9-39.6) | 12.64 | 1.71 : 2.36 | 21.6 | 10.7 |
| F3-pro-large | regular / pro / large | `opp.hunt_radius_m=20;opp.check_min_closing=1;opp.poke_frames=2;mate_speed_scale=1` | 2000 | 57.1 (54.9-59.3) | 4.44 | 1.73 : 1.4 | 18.3 | 13.3 |
| L14-rk-large-sp10 | novice / rookie / large | `opp.shoot_extra_m=25;opp.poke_frames=6;mate_speed_scale=1;speed_scale=1.0` | 600 | 82 (78.7-84.9) | 2.28 | 1.73 : 0.61 | 18.8 | 20.6 |
| F4-as-reg-draw25 | good / all_star / regular | `mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60;opp.draw_jump_pct=25` | 2000 | 32.5 (30.5-34.6) | 9.56 | 1.62 : 2.42 | 20.1 | 12 |
| L15-as-nolift | good / all_star / regular | `mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60;opp.lift=0` | 600 | 37.3 (33.6-41.3) | 9.87 | 1.73 : 2.31 | 18.9 | 12.8 |
| F4-rk-large-red098 | novice / rookie / large | `opp.shoot_extra_m=25;opp.poke_frames=6;mate_speed_scale=1;red_skill=0.98` | 2000 | 87.7 (86.2-89.1) | 2.33 | 1.92 : 0.49 | 18.2 | 22.5 |
| F5-pro-large-red1065 | regular / pro / large | `opp.hunt_radius_m=20;opp.check_min_closing=1;opp.poke_frames=2;mate_speed_scale=1;red_skill=1.065` | 2000 | 50.9 (48.7-53.1) | 3.98 | 1.42 : 1.34 | 17.1 | 11.3 |
| F5-as-large-sp132 | good / all_star / large | `mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60;speed_scale=1.32` | 2000 | 29.1 (27.2-31.1) | 5.5 | 1.69 : 2.69 | 18.7 | 13.2 |
| S-regular-take-1 | regular / cart / ladder/take-1 | (profile) | 200 | 98.5 (95.7-99.5) | 1.65 | 1.98 : 0.19 | 4 | 24.4 |
| W-rk-reg | novice / rookie / regular | (profile) | 600 | 90.3 (87.7-92.4) | 2.48 | 2.82 : 0.92 | 16.2 | 13 |
| S-novice-take-2 | novice / cart / ladder/take-2 | (profile) | 200 | 95.5 (91.7-97.6) | 1.75 | 2.11 : 0.28 | 7.3 | 18.5 |
| S-good-take-2 | good / cart / ladder/take-2 | (profile) | 200 | 100 (98.1-100) | 4.75 | 6.46 : 0.1 | 4.1 | 34.9 |
| S-regular-take-2 | regular / cart / ladder/take-2 | (profile) | 200 | 100 (98.1-100) | 4.11 | 5.14 : 0.17 | 7.1 | 32.6 |
| S-good-take-3 | good / cart / ladder/take-3 | (profile) | 200 | 100 (98.1-100) | 2.48 | 3 : 0.17 | 4.2 | 24.7 |
| S-novice-take-3 | novice / cart / ladder/take-3 | (profile) | 200 | 88.5 (83.3-92.2) | 2.48 | 2.8 : 0.96 | 16.6 | 13.1 |
| S-regular-take-3 | regular / cart / ladder/take-3 | (profile) | 200 | 99 (96.4-99.7) | 2.5 | 2.98 : 0.28 | 6.2 | 24.6 |
| W-pro-reg | regular / pro / regular | (profile) | 600 | 56.5 (52.5-60.4) | 9.13 | 2.18 : 1.86 | 15.7 | 12 |
| S-good-take-4 | good / cart / ladder/take-4 | (profile) | 200 | 79 (72.8-84.1) | 6.77 | 2.29 : 1.07 | 8.6 | 14.7 |
| S-novice-take-4 | novice / cart / ladder/take-4 | (profile) | 200 | 25.5 (20-32) | 2.5 | 0.77 : 1.61 | 12.8 | 6.1 |
| S-regular-take-4 | regular / cart / ladder/take-4 | (profile) | 200 | 60 (53.1-66.5) | 6.63 | 1.6 : 1.29 | 10.9 | 12.1 |
| S-good-take-5 | good / cart / ladder/take-5 | (profile) | 200 | 87 (81.6-91) | 7.21 | 2.81 : 1.11 | 8.5 | 14.7 |
| W-as-reg | good / all_star / regular | (profile) | 600 | 33.5 (29.8-37.4) | 9.59 | 1.63 : 2.38 | 19.3 | 12.5 |
| S-novice-take-5 | novice / cart / ladder/take-5 | (profile) | 200 | 21 (15.9-27.2) | 4.01 | 1.21 : 2.52 | 18.3 | 5.7 |
| S-regular-take-5 | regular / cart / ladder/take-5 | (profile) | 200 | 61.5 (54.6-68) | 8.6 | 2.32 : 1.77 | 14.1 | 11.9 |
| S-good-take-6 | good / cart / ladder/take-6 | (profile) | 200 | 85 (79.4-89.3) | 7.3 | 2.71 : 1.2 | 9.1 | 14.9 |
| S-novice-take-6 | novice / cart / ladder/take-6 | (profile) | 200 | 16 (11.6-21.7) | 3.92 | 1.05 : 2.6 | 19.2 | 6 |
| S-regular-take-6 | regular / cart / ladder/take-6 | (profile) | 200 | 54 (47.1-60.8) | 9.59 | 2.14 : 1.96 | 16.1 | 11.9 |
| W-rk-large | novice / rookie / large | (profile) | 600 | 94 (91.8-95.6) | 2.44 | 2.3 : 0.36 | 13.8 | 26.2 |
| S-good-take-7 | good / cart / ladder/take-7 | (profile) | 200 | 91 (86.2-94.2) | 8.57 | 2.81 : 0.85 | 7.6 | 16.7 |
| T1-rk-reg-shoot30 | novice / rookie / regular | `opp.shoot_extra_m=30` | 600 | 87.2 (84.3-89.6) | 2.61 | 2.76 : 1.03 | 18.3 | 12.8 |
| T1-pro-reg-full | regular / pro / regular | `opp.sprint_share=0.7;opp.lift_cooldown_frames=45` | 600 | 56.3 (52.3-60.2) | 8.63 | 2.2 : 1.94 | 15.2 | 11.8 |
| S-good-take-8 | good / cart / ladder/take-8 | (profile) | 200 | 41 (34.4-47.9) | 8.91 | 1.75 : 2.29 | 18.6 | 11.5 |
| S-novice-take-7 | novice / cart / ladder/take-7 | (profile) | 200 | 30 (24.1-36.7) | 4.54 | 1.09 : 1.62 | 14.6 | 5.4 |
| S-regular-take-7 | regular / cart / ladder/take-7 | (profile) | 200 | 62.5 (55.6-68.9) | 9.49 | 2.06 : 1.39 | 12.8 | 11.7 |
| W-pro-large | regular / pro / large | (profile) | 600 | 61.7 (57.7-65.5) | 3.86 | 1.56 : 1.15 | 16.6 | 12.6 |
| S-novice-take-8 | novice / cart / ladder/take-8 | (profile) | 200 | 6 (3.5-10.2) | 3.55 | 0.65 : 2.87 | 22.8 | 5.1 |
| S-regular-take-8 | regular / cart / ladder/take-8 | (profile) | 200 | 18.5 (13.7-24.5) | 7.51 | 1.11 : 2.63 | 22.2 | 10.3 |
| T1-rk-large-shoot30 | novice / rookie / large | `opp.shoot_extra_m=30` | 600 | 78.5 (75-81.6) | 2.87 | 2.28 : 1.05 | 14.2 | 24.2 |
| W-as-large | good / all_star / large | (profile) | 600 | 25 (21.7-28.6) | 5.07 | 1.41 : 2.63 | 18.9 | 13.2 |
| T1-pro-large-full | regular / pro / large | `opp.sprint_share=0.7;opp.lift_cooldown_frames=45` | 600 | 48.5 (44.5-52.5) | 3.57 | 1.18 : 1.19 | 16.6 | 10.3 |
