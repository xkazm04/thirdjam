# The difficulty curve, pass 8: the ladder the operator plays is the ladder that was measured (lane A154, track C0 evolution 4, 2026-11-14 02:32-)

Scope (brief `briefs/A154-C0-evolution-4.md`): main 4cab387 carries lane A133's ladder and season plus four landings since it was set at 2,000 a cell:
CONTACT-5 (the shove under the shields; the scripted human presses its check on closing speed), MOVEMENT-5 (a kept first step), BRAIN-3 (for teams under
five the human's mates check the carrier and the back steps up: the large rink), and take 2's 2:00 clock. Targets 85 / 48 / 26 within 3 (R-0046); law 3 a
gate (R-0053); at most three profile numbers. Driver `tools/perf/results/a154/curve.mjs` (lane A133's; `-Multiprocess`); rows `a154/cells.jsonl`; status
`status/A154.md`.

## 1. The six cells on main 4cab387 as built (600 a cell)

| cell | target | hockey.regular | hockey.arena.large |
|---|---|---|---|
| novice at rookie | 85 | 84.7 (81.6-87.3) | 86.2 (83.2-88.7) |
| regular at pro | 48 | 50.5 (46.5-54.5) | **54.8** (50.8-58.8): 6.8 over |
| good at all_star | 26 | **31.2** (27.6-35.0): 5.2 over | **30.3** (26.8-34.1): 4.3 over |

The large rink moved most (BRAIN-3 is for teams under five: the large rink is three a side), as the brief expected.

## 2. The trims (600 a cell, tunes on the build)

| cell | number | values tried (win %) | chosen |
|---|---|---|---|
| pro, large | large-rink red skill (1.12) | 1.15: 48.2; 1.17: 43.3 | **1.15** |
| all_star, large | large-rink speed (1.42) | 1.45: 29.0; 1.47: 27.3 | **1.47** |
| all_star, regular | red skill (0.9) | 0.95: 31.5; 0.85: 32.3 | flat: not a lever |
| all_star, regular | speed (1.4) | 1.45: 25.3; 1.5: 29.7 | see section 3 |

Law 3 (`Bar.Curve.Law3`, 200 seeds; stop at 8 / 9.5 / 10 / 11 / 12 m): all_star as built 0 / 39.5 / 57.5 / 89.5 / 100; at speed 1.45 0 / 30.5 / 53.5 / 84.5 / 100
(this main's bar: 50 % where the window is); pro 100 % at 8 m; the straight charge 0 at both. The two large-rink numbers do not touch the drill (a
hockey.regular scene).

## 3. The ladder as set, 2,000 a cell

| cell | target | hockey.regular | hockey.arena.large |
|---|---|---|---|
| novice at rookie | 85 | **84.9** (83.3-86.4) | **87.3** (85.8-88.7) |
| regular at pro | 48 | **48.4** (46.2-50.6) | **45.9** (43.7-48.0), large red 1.12 -> 1.15 |
| good at all_star | 26 | **26.9** (25.0-28.9), speed 1.4 -> 1.45 (30.6 at 1.4) | **25.8** (23.9-27.7), large speed 1.42 -> 1.47 |

Three profile numbers (the brief's limit), speed and the large offsets only: the aggression numbers were set last pass and are flat here (all_star red 0.85 /
0.95: 32.3 / 31.5). The regular cells and the large ones were measured as tunes on build 1 and the three numbers built in build 2 (`Jam.Core.Movement` 17
pass: law 3 pro 100 % at 8 m, all_star 84.5 % at 11 m and 100 % at 12 m). Neutral: `JamBaseline::BrainTune("all_star")` gains `speed_scale=1.4`.
R-0058's all_star presser switch is not on this main; it costs about one point of all_star's cell when the brain lane lands it (inside the margin above).

## 4. The season on main 4cab387 (200 a take; the level each take's cartridge carries)

| take | level, red skill | novice | regular |
|---|---|---|---|
| 1 | rookie | 80.5 | 95.5 |
| 2 | rookie (2:00 clock) | 84.5 | 95.0 |
| 3 | rookie | 84.5 | 98.0 |
| 4 | rookie | 82.0 | 99.0 |
| 5 | pro, red 0.4 -> **0.37** | 58.5 -> **72.0** | 99.5 |
| 6 | pro, red 0.45 | 58.0 | 99.5 |
| 7 | pro, red 0.9 | 49.5 | 83.5 |
| 8 | all_star, red 0.7 | 4.0 | 33.5; **31.5** with all_star speed 1.45 (good 50.5) |

R-0057 holds after the one take change: novice steps from take 3 to 7: 2.5, 10, 14, 8.5 (bar 20); take 7 49.5 (bar 15); the finale 31.5 for the regular player
(bar 25), 50.5 for the good one. Take-5 had fallen to 58.5 on this main (the step 4 -> 5 23.5): `ai.red_skill` 0.37 in take-5 and its fallback.

## 5. For the operator's sheet (proposed sentences for `docs/PLAY-ME.md`)
- "The ladder, measured on this build with the scripted players over 2,000 matches a cell: a newcomer wins 85 % at rookie, a regular player 48 % at pro, a
  good one 27 % at all_star; on the big ice 87 / 46 / 26."
- "Measured with a scripted player who lands most of his checks, pro reads about 56 %; with one who whiffs half, 49 to 53 %: which are you?" (R-0060)
- "The season ramps: a newcomer wins the first four matches about 4 times in 5, then 72, 58, 50 %; the finale is won by a regular player one time in three."
- "Known: the opponents shoot from the red line (median release 36 m at pro). Taking the long shot away costs 10 points at pro and 23 at all_star (REVIEW-2).
  A second curve pass could buy back about 3 to 5 of pro's 10 with the stick and the checks (the rows of CURVE-7: the checks are flat, sprint and lift
  cooldown about 3, the poke is already at 2 and 0); all_star's 23 cannot be bought with numbers (its aggression is saturated and its speed is at the top
  that still helps): that needs the brain track (a second presser, the presser kept on its job). About two hours for pro, a brain evolution for all_star;
  decide at 10:00 whether Saturday goes there."
