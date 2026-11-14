# The difficulty curve, pass 11: the Sunday ladder candidate, everything on (lane A182, track C0 evolution 7, 2026-11-14 08:13-)

Scope (the Director's message, 08:10): what the game reads if the operator says yes to everything, and one number a level that brings it back. Tree: main
9e452219 plus the take shield's code (lane A177, `CONTACT-9.md`; its source and schema hunks only, the two generated embeds regenerated), every key at its
neutral value by default: the default pro cell replays A162's rows, 100 of 100 seeds identical. Instrument: the ladder's (`JAM_PLAYTESTER_CHECK=closing`).
Driver and rows: `tools/perf/results/a182/` (`curve.mjs` = A170's plus `--cart`, a variant cartridge written by `mkvar.mjs`: the `--set` file name of nine
keys passes Windows' path limit). Nothing lands: the output is `tools/perf/results/a182/A182-sunday-ladder.patch` and this table.

**Everything on** = cartridge keys `contact.downed_radius_m` 0.6, `check_cooldown_frames` 60 with `check_cooldown_restart`, `lunge_frames` 12 /
`lunge_step_mps` 1.5, `board_slam_frames` 30, `take_shield_frames` 15 / `take_shield_stagger_max` 45; tunes `mate.hk_screen_frames` 30,
`mate.hk_avoid_downed_m` 1.3 / `mate.hk_avoid_downed_out` 0.7.

## 1. The six cells (600, the same seeds as A162's rows)

| cell (target) | as built | everything on | the one number | with it (600) | with it (2,000) |
|---|---|---|---|---|---|
| rookie regular (85) | 85.3 | 89.3 (+4.0) | red skill 0.97 -> **1.03** (1.0: 90.8) | 84.7 | **85.8** (84.3-87.3) |
| pro regular (48) | 50.5 | 54.8 (+4.3) | hunt 11 (A170) -> **14** (12: 49.2, 13: 48.7) | 46.0 | **47.0** (44.8-49.2) |
| all_star regular (26) | 29.8 | 34.7 (+4.9) | none found | | everything on: **37.0** (34.9-39.1); as built 28.8 |
| rookie large (85) | 83.7 | 87.7 (+4.0) | none needed (red 1.17: 81.8) | | everything on: **86.9** (85.4-88.3) |
| pro large (48) | 46.7 | 60.8 (**+14.1**) | the same hunt **14** (16: 47.0, 18: 48.7) | 48.5 | **49.1** (46.9-51.3) |
| all_star large (26) | 25.3 | 31.7 (+6.4) | none found | | |

**All_star, plainly**: everything on puts all_star 11 points over its bar on the regular rink (37.0 at 2,000; 8 more than as built) and 6 on the large (the take shield alone is +6.0 there at 2,000,
CONTACT-9); no profile number moved it in A162 or A170 (checks, speed, red skill, pressure time, pass margin, the presser's commit: flat or the wrong
way). Its dial is the brain lane's open question (A181).

## 2. What the player gets with everything on (600, regular rink; as built -> on)

| per match | rookie | pro | all_star |
|---|---|---|---|
| lying on the ice, % of play | 3.3 -> 2.1 | 9.1 -> **4.5** (5.0 with hunt 14, 2,000) | 11.6 -> 6.1 |
| knocked down | 3.18 -> 2.01 | 6.87 -> 3.72 (4.26 with hunt 14) | 8.57 -> 4.44 |
| ... as he won the puck | 1.34 -> 0.24 | 3.83 -> 0.51 | 4.58 -> 0.54 |
| hits the shield turned into a stumble | 0 -> 1.1 | 0 -> 3.8 | 0 -> 4.0 |
| stumbling, % of play | 0.03 -> 0.47 | 0.12 -> 1.76 | 0.13 -> 2.00 |
| mashing: check presses dropped by the cooldown | 0 -> 0.16 (of 9.3) | 0 -> 0.31 (of 10.8) | 0 -> 0.20 (of 7.9) |
| lunges that met nobody | 0 -> 6.8 | 0 -> 6.2 | 0 -> 3.6 |
| a standing skater on a downed body (0.9 m, spells over 6 frames) | | 7.28 -> **0.01** | 6.04 -> 0.01 |

## 3. The season and the modes with everything on

Novice, 200 a take (takes 3-4 at rookie red 1.03, takes 5-7 at pro hunt 14; their own cartridge levels otherwise): 85.5 / 85.5 / 69.0 / 55.5 / 42.5
(as built 89.0 / 85.0 / 65.5 / 60.5 / 54.0). Steps 0 / 16.5 / 13.5 / 13.0 (bar 20); take 7 42.5 (bar 15): R-0057 holds.

Modes with everything on and the dial pinned at 8 (100, regular class; A170's as built on the same seeds): fog-of-war 86 -> 87, bruisers 53 -> 56,
glass-house 48 -> 45. The contact keys are global, so "pinned" no longer means "as measured", but here the set moves the three modes by 3 points or less
(one standard error is 5).

## 4. The prepared patch

`tools/perf/results/a182/A182-sunday-ladder.patch`, on top of `tools/perf/results/a170/A170-pro-hunt11-pinned.patch` (checked: main + the pinned patch's
`JamAICore.h` hunk + this patch apply clean): `JamAICore.h` pro `HuntRadiusM` 11 -> 14; `DifficultyProfile.cpp` rookie red skill 0.97 -> 1.03;
`JamBaseline.h` rookie's regular-rink neutral tune gains `red_skill=0.97` (the curve lanes' rule: the old number goes to the baseline). Profile numbers
only: the set itself is the contact lane's cartridge keys and the brain lane's tune keys. Not built as a profile in this lane: the profile = tune
equivalence of these two kinds of number was shown 600 of 600 by A162 (red skill) and A170 (the hunt). If A177's patch lands first, its own hunk in
`JamBaseline.h` sits beside this one (a different line).
