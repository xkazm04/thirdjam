# The difficulty curve, pass 2 (lane A70, device A, 2026-11-13)

The R-0029 curve re-tuned with the playtester soak as the instrument instead of the gate's 20 seeds. Targets (R-0029 point 1, hockey.regular): the novice wins 85 % at
rookie, the regular player about 50 % at pro, the good player about 25 % at all_star. Built on main 7dfbeb5 (A51 stick handling, A52 lifted puck, A66's pro-shooting
hotfix, A71's horn, the Director's calibrations).

## 1. How it was measured

- The same commandlet line as lane A53's soak (`UnrealEditor-Cmd -run=JamTournament -sim=jam -player=<class> -difficulty=<level> -seeds=lo-hi -maxframes=36000`),
  8 headless processes, 100 to 250 seeds each.
- **n 2,000 per number** on hockey.regular, always the same seeds: A53's regular seeds (1-100 of each 150-band, 20 bands, seeds 1 to 2,950). Every lever is
  therefore a paired comparison against the baseline on the same 2,000 matches. hockey.arena.large: seeds 1-2,000.
- Levers went through `-aitune` (profile keys, applied after the level resolves) and A51's lever through a variant cartridge
  (`game.stick.sweep.bot_react_frames`, `tools/playtest/run.mjs` `writeVariant`). The final numbers were then built into the profiles and re-measured without any
  override: every final row equals its tune row to the match (the override and the shipped number play the same game).
- Win % = human-side wins over all matches (no draws or unfinished matches occurred), 95 % Wilson interval. 2,000 matches put the interval at about +-2 points.
- A 2,000-match cell costs about 3 minutes; the whole pass played about 95,000 matches.

## 2. Baseline (this build, before any change)

| cell | A53 soak (main e99e005 + later, n 2,000) | baseline here (n 2,000) | target |
|---|---|---|---|
| novice at rookie | 90.5 (89.1-91.7) | **90.8 (89.4-91.9)** | 85 |
| regular at pro | 58.1 (55.9-60.2) | **52.3 (50.1-54.4)** | 50 |
| good at all_star | 44.8 (42.6-47.0) | **40.5 (38.4-42.7)** | 25 |

The rookie cell confirms A53. Pro and all_star were already 6 and 4 points harder than A53 measured, on the same seeds: the build moved under the soak (A66's
hotfix, "the pro bots stopped shooting", landed after A53's build). Pro sat 0.1 point above its band, so its nudge is small.

## 3. Each lever's effect (n 2,000 on the same seeds unless marked n 600; one lever at a time, then at most two per level)

### rookie (novice; baseline 90.8)
| lever | win % (95 %) | note |
|---|---|---|
| speed 0.85 -> 0.90 | 90.5 (87.9-92.6), n 600 | nothing |
| **red skill 0.75 -> 0.80** | **85.8 (84.2-87.3)** | chosen |
| red skill 0.85 | 82.3 (80.6-83.9) | too far |
| red 0.80 + speed 0.90 | 85.7 (84.0-87.1) | speed adds nothing |

The "crease skater's save" lever was not needed. Red skill scales the opposing skaters' speed (not the crease back), so it is the rookie's speed lever in practice.

### pro (regular; baseline 52.3)
| lever | win % (95 %) | note |
|---|---|---|
| `bot_react_frames` 8 -> 6 | 54.0 (51.9-56.2) | nothing (A51's lever does not move this cell on this build) |
| **red skill 0.94 -> 0.95** | **50.6 (48.5-52.8)** | chosen |
| red skill 0.96 | 51.2 (49.0-53.4) | |
| red skill 0.97 | 44.9 (42.7-47.0) | a cliff between 0.96 and 0.97 |
| red skill 1.0 | 44.5 (42.3-46.6) | |
| **red 0.95 + checks from 4 m/s (5 before)** | **51.0 (48.9-53.2)** | chosen; knockdowns of the human 1.74 -> 1.96 a match |

### all_star (good; baseline 40.5)
| lever | win % (95 %) | note |
|---|---|---|
| `bot_react_frames` 5 -> 3 | 39.8 (36.0-43.8), n 600 | nothing |
| checks from 4 m/s (5 before) | 37.5 (33.7-41.4), n 600 | nothing |
| checks from 3 m/s | 41.0 (38.9-43.2) | nothing on the result; knockdowns 2.42 -> 2.71 a match |
| speed 1.15 -> 1.20 | 34.5 (30.8-38.4), n 600 | |
| speed 1.20 + checks 4 | 33.9 (31.8-36.0) | |
| **speed 1.30** | **26.5 (24.6-28.5)** | chosen |
| speed 1.32 | 23.9 (22.1-25.8) | also in band |
| speed 1.40 | 14.8 (13.3-16.4) | |
| speed 1.30 + checks 4 | 26.8 (24.9-28.7) | |
| **speed 1.30 + checks from 3 m/s** | **26.8 (24.9-28.7)** | chosen; knockdowns 2.73 -> 3.00 a match |

The check lever (the coordinator's input from lane A73: bot checks are near zero in a packaged build) moves the knockdown count, not the win share: a check needs
both the closing speed and the bot's own speed above the bar inside 1.3 m, and the stick lift reaches first (about 18 lifts a match against 2 to 3 knockdowns).
It is in the final numbers because it costs nothing on the curve and makes tackling read more often, which R-0031 point 2 asked for. The all_star lift
(`bLift`) is already on at pro and all_star; there was nothing to switch.

## 4. The final table (built numbers, no override, n 2,000 each)

| cell (hockey.regular) | before | **final** (95 %) | target | in band |
|---|---|---|---|---|
| novice at rookie | 90.8 | **85.8 (84.2-87.3)** | 85 | yes |
| regular at pro | 52.3 | **51.0 (48.9-53.2)** | 50 | yes |
| good at all_star | 40.5 | **26.8 (24.9-28.7)** | 25 | yes |

The whole grid on the final numbers (hockey.regular, n 2,000 each, win % with 95 % interval; A53's soak in brackets, a different build):

| class | rookie | pro | all_star |
|---|---|---|---|
| novice | **85.8** (84.2-87.3) [90.5] | 17.4 (15.8-19.2) [20.5] | 2.6 (2.0-3.4) [6.5] |
| regular | 99.3 (98.8-99.6) [99.2] | **51.0** (48.9-53.2) [58.1] | 13.3 (11.9-14.9) [29.3] |
| good | 99.8 (99.5-99.9) [99.9] | 66.0 (63.8-68.0) [74.6] | **26.8** (24.9-28.7) [44.8] |

For the Director: all_star is now a steep step. The regular player wins 13 % there (29 % in A53's soak) and the novice 3 %; R-0029 sets no target for those
cells, but if all_star should stay reachable for a regular player, the next lever is a smaller speed step with a skill the good player beats and the regular
player does not (the speed lever hits every class alike). The good player at pro (66 %) and the rookie column for regular and good players (99 %+) carry no target.

## 5. The large rink (hockey.arena.large, seeds 1-2,000)

R-0029's large-rink offsets were far too kind on this build: the novice won 97.7 % at rookie and the regular player 71.0 % at pro.

| cell | before (rookie large 0.9 / 0.8, pro large red 0.9) | levers (n 2,000) | **final** (95 %) | target |
|---|---|---|---|---|
| novice at rookie | 97.7 (96.9-98.2) | large red 0.9: 93.3; 0.95: 89.0; 1.0: 81.8; speed 1.0 + red 0.9: 82.0 | **84.7 (83.0-86.2)** at large red 0.97 | 85 |
| regular at pro | 71.0 (69.0-72.9) | large red 1.0: 45.5; speed 1.1 + red 0.95: 45.4; red 0.95 + checks 4: 55.6; red 0.97 + checks 4: 51.1 | **51.1 (49.0-53.3)** at large red 0.97 | 50 |
| good at all_star | 36.3 (34.2-38.4) | speed 1.3 (no large offset): 27.0 | **27.2 (25.3-29.2)** with checks 3 | 25 |

## 6. The numbers that moved (`Game/Source/JamCore/AI/DifficultyProfile.cpp`, `JamAICore.h`)

| level | number | before | after |
|---|---|---|---|
| rookie | red skill | 0.75 | **0.8** |
| rookie | large-rink red skill | 0.8 | **0.97** (large speed 0.9 unchanged) |
| pro | red skill (`FJamAIParams::RedSkill`; the pro profile is those defaults) | 0.94 | **0.95** |
| pro | `CheckMinClosing` (`ProOppSkill`) | 5 | **4** |
| pro | large-rink red skill | 0.9 | **0.97** |
| all_star | speed scale | 1.15 | **1.3** (the sim clamps at 1.5) |
| all_star | `CheckMinClosing` | 5 | **3** (never from behind, unchanged) |

Unchanged: `SweepReactFrames` 14 / 8 / 5 (A51's lever measured flat in both cells it was offered for), every other skill.

## 7. The gate on the final numbers

`tools/analysis/jamtests.ps1 -Filter "Jam.Playtest+Jam.Bot+Jam.Core.Determinism"`: 12 of 12 PASS (`Game/logs/a55-automation-ue-20261113-121500.log`).
- `Jam.Playtest.NoviceScoresAtRookie`: 14 of 20 seeds (need 10).
- `Jam.Bot.HumanHasSomethingToDo`: 10 of 126 faceoffs without a human touch in seeds 1-20. **The soak's share over 2,000 matches is 8.4 % (1,055 of 12,548)**
  at the final rookie (7.3 % before, 871 of 11,987). The budget moves from 12 % to the share plus 2 points, **10.4 %** (13 of 126 today).
- `Jam.Bot.DifficultyProfilesResolve` (pro equals the FJamAIParams defaults), `PassesToHuman`, `PlayerBotWins`, `SeasonTakes`, `ProCharge`, `Jam.Core.Determinism`: PASS.
- Full `Jam` suite (`-Filter Jam`, `Jam.Slow.DifficultyCurve` included): 109 of 109 PASS (`Game/logs/a55-automation-ue-20261113-121550.log`).

## 8. For R-0029's next amendment (the Director writes the ruling)

1. The curve on hockey.regular, n 2,000 each: novice at rookie 85.8, regular at pro 51.0, good at all_star 26.8; on hockey.arena.large 84.7 / 51.1 / 27.2. All
   six intervals contain their targets.
2. Numbers: rookie red 0.8 (large 0.97; speed 0.85, large 0.9); pro red 0.95 (large 0.97), checks from 4 m/s; all_star speed 1.3, checks from 3 m/s.
3. `game.stick.sweep.bot_react_frames` (R-0034) does not move the curve at pro or all_star on this build; it stays a feel number.
4. The check speed moves knockdowns, not results: a free lever for how much tackling a level shows.
5. The soak is cheap enough (2,000 matches in about 3 minutes) that a curve claim should carry n 2,000 and its interval; 20 seeds put about +-20 points on a cell.
6. `HumanHasSomethingToDo`'s budget is 10.4 % (measured 8.4 % + 2).
