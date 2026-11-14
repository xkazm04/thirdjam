# The difficulty curve, pass 9: the last pass before the operator plays (lane A162, track C0 evolution 5, 2026-11-14 03:35-)

Scope (brief `briefs/A162-C0-evolution-5.md`, R-0061): main a7d89871 carries CURVE-8's ladder (A154's three trims) plus three landings: CONTACT-6 (the
standing brace; the scripted human's course press), MOVEMENT-6 (the poke that takes the puck; a miss locks the stick) and BRAIN-4 (all_star's presser inside the
hunt). Targets 85 / 48 / 26 within 3; at most three profile numbers (aggression first, speed last; the hunt radius is not a lever); law 3 a gate at every level.
Driver `tools/perf/results/a162/curve.mjs` (A154's, plus `--check` / `--poke`, `-Multiprocess`); rows `a162/cells.jsonl`; status `status/A162.md`.

**The instrument** (R-0060 point 1, R-0061 point 1): `JAM_PLAYTESTER_CHECK=closing` (lane A132's press, the one the ladder was set with) and the poke as the
config ships it: `stick.bPlaytesterPokeRead` false, no `JAM_PLAYTESTER_POKE`. The poke's two rules (assist 60 degrees, miss recovery 20 frames) are on, as the game
ships them. That is the right arm because the read is an instrument change, not a rule (MOVEMENT-6 section 4 kept it off for that reason): switching it on
would move the cells by how the script presses, not by what the game does, and the ladder was set on the press as it was. A person who presses Space "when it
can land" is the read's arm; its cost on this tree is MOVEMENT-6's +1.8 at pro and -3.5 at all_star (600), reported with R-0060's table below.

## 1. The six cells on main a7d89871 as built

| cell | target | hockey.regular, 600 | hockey.regular, 2,000 | hockey.arena.large, 600 | hockey.arena.large, 2,000 |
|---|---|---|---|---|---|
| novice at rookie | 85 | 85.3 (82.3-87.9) | 85.8 (84.1-87.2) | 88.0 (85.2-90.4) | **88.5** (87.0-89.8): 3.5 over |
| regular at pro | 48 | 50.5 (46.5-54.5) | **52.4** (50.2-54.5): 4.4 over | 46.7 (42.7-50.7) | 50.2 (48.0-52.4) |
| good at all_star | 26 | 29.8 (26.3-33.6) | 28.8 (26.8-30.8) | 25.3 (22.0-29.0) | 26.1 (24.2-28.0) |

Two cells outside 3: rookie on the big ice and pro on the small. R-0061 expected pro about five over; it came in four over on the small ice and inside on the big.

## 2. The trims (tunes on build 1)

| cell | number | values tried (win %, n) | chosen |
|---|---|---|---|
| all_star, regular | `opp.check_min_closing` (2.0) | 1.0: 30.2; 0.5: 30.7 (600) | flat: not a lever |
| all_star, regular | speed (1.45) | 1.47: 32.5 (600) | worse (A95's non-monotonic top) |
| all_star, regular | `opp.hk_pressure_s` (0.3) | 0.6: 37.2; 0.2: 27.8; 0.1: 29.3 (600); 0.2: 28.1 (2,000) | -0.7 at 2,000: not worth a number |
| all_star, regular | `opp.hk_better_by_m` (12) | 6: 35.0 (600) | the wrong way |
| all_star, regular | red skill (0.9) | 0.8: 38.8; 1.0: 31.3 (600) | flat upward |
| rookie, large | large-rink red skill (1.12) | 1.14: **83.8** (82.2-85.4) (2,000) | **1.14** |
| pro, regular | red skill (1.07) | 1.09: 51.3; 1.10: 51.5; 1.13: **49.3** (47.1-51.5); 1.15: 49.5 (2,000) | built and reverted: breaks `StopAndGo` (below) |
| pro, regular | `opp.hk_pressure_s` (0.3) | 0.2: 46.7 (600, paired with 50.5); **52.1** (49.9-54.3) (2,000) | noise: not a lever |

All_star on the small ice reads 28.8 as built at 2,000: inside 3, and no lever moves it down (its aggression and speed are saturated, as CURVE-8 found;
carrying through the forecheck and keeping the puck are what beat the human). It stays as built. Pro's red skill moves the small-rink cell about half as much
as the big-rink one (A154: 2.2 points per 0.01 on 120 x 60; here about 0.5).

Law 3 (`Bar.Curve.Law3`, 200 seeds; the stop begun at 8 / 9.5 / 10 / 11 / 12 m; the straight charge):

| arm | 8 m | 9.5 | 10 | 11 | 12 | straight |
|---|---|---|---|---|---|---|
| pro as built (red 1.07) | 100 | 97 | 96 | 86 | 61.5 | 0 |
| pro red 1.09 | 99.5 | 90.5 | 82.5 | 68 | 53 | 0 |
| pro red 1.13 (built, reverted) | **99** | 81.5 | 64 | 44 | 32.5 | **0** |
| pro red 1.15 | 98 | 78 | 59.5 | 40.5 | 37 | 0 |
| all_star as built | **100** | 93 | 86 | 58 | 39 | **0** |

R-0061's law 3 (8 m at 80 % or more, the straight charge under 20 %) holds at every arm. **The gate does not**: `Jam.Core.Movement.StopAndGo` also carries lane A121's older bar, pro's stop begun at 9.5 m at 95 % or more (`JamCarrierSkillTests.cpp:231`, and the mouse-is-the-skill margin at `:291`); it read 80.5 % on build 2 with red 1.13 (245 pass, 1 fail), and the drill puts 1.09 at 90.5 %. So pro's red skill is not a lever under the gate on this main. The drill does not read the bot carrier's choices: `opp.hk_pressure_s` 0.2, `opp.deke` 1, `opp.shoot_extra_m` 45 and `opp.pass_out` 1 give rows identical to the base (law3-3); the first measured flat at 2,000; the other three were not measured before the deadline. The rookie number is a large-rink offset; the drill is a
hockey.regular scene, so it does not touch law 3.

## 3. The ladder as set, 2,000 a cell (build 3: full `Jam` filter 246 pass, 0 fail)

| cell | target | hockey.regular | hockey.arena.large |
|---|---|---|---|
| novice at rookie | 85 | **85.8** (84.1-87.2) | **83.8** (82.2-85.4), large red 1.12 -> 1.14 (88.5 at 1.12) |
| regular at pro | 48 | **52.4** (50.2-54.5): 4.4 over | **50.2** (48.0-52.4) |
| good at all_star | 26 | **28.8** (26.8-30.8) | **26.1** (24.2-28.0) |

One profile number: rookie `LargeRedSkill` 1.12 -> 1.14 (`DifficultyProfile.cpp:60`), built and checked against its tune (600 of 600 seeds identical in
result, goals, frames and hash). Neutral: `JamBaseline::BrainTune("rookie", true)` carries its own `red_skill=0.9`, so no pinned test moves; the revert is
the one line. Pro on the small ice as built plays build 1's matches exactly (600 of 600).

**Pro on the small ice is outside 3** and no number fixes it under the gate as it stands. Two ways out, both the Director's: (a) ship it as built
(52.4: inside EVOLUTION-COMMON's 4-point rule); (b) pro red 1.07 -> 1.13 (49.3; measured, built, reverted) with A121's 9.5 m bar in `StopAndGo` lowered
to what R-0061 names as law 3 (the stop begun at 8 m: 99 % at 1.13). All_star on the small ice is 2.8 over and stays: no lever moves it down.

## 4. R-0060's two columns on the package's ladder (600 a cell, the same seeds)

| cell | closing press (A132: the ladder's instrument) | course press (A151: as the config ships) | course - closing |
|---|---|---|---|
| novice at rookie, regular | 85.3 | 86.8 | +1.5 |
| regular at pro, regular | 50.5 | **56.0** | +5.5 |
| good at all_star, regular | 29.8 | 31.0 | +1.2 |
| novice at rookie, large | 83.7 | 85.2 | +1.5 |
| regular at pro, large | 46.7 | 50.0 | +3.3 |
| good at all_star, large | 25.3 | 25.7 | +0.4 |

A player who lands most of his checks finds pro about five points easier on the small ice (R-0060 read +3.5 on the tree before the brace and the poke).

**`Jam.Bot.HumanHasSomethingToDo`**: the novice at rookie on hockey.regular, faceoffs without a human touch inside 600 frames: 0 of 2,905 with the course
press (what the test plays; seeds 1-20 0 of 100), 0 of 9,680 with the closing press at 2,000. Proposed budget (R-0039, the share plus 2 points): **2.0 %**.
The number in `BotTests.cpp` is unchanged (the Director sets it).

## 5. The season (200 a take, the closing press, the level each take's cartridge carries)

| take | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
|---|---|---|---|---|---|---|---|---|
| novice | 81.5 | 84.5 | 89.0 | 85.0 | 65.5 | 60.5 | 54.0 | 7.0 |
| regular | | | | | | | | **30.0** |
| good | | | | | | | | 54.0 |

R-0057 holds with no take changed: steps from take 3 to 7 are 4.0, 19.5, 5.0, 6.5 (bar 20); take 7 54.0 (bar 15). The step from 4 to 5 is half a point
inside its bar on 200 matches (one standard error is about 4): the first take to watch if anything lands under pro.

## 6. Findings
- `Jam.Core.Movement.StopAndGo` (`JamCarrierSkillTests.cpp:231`, `:291`) holds A121's 9.5 m bar at pro (95 %), stricter than R-0061's law 3 (8 m at 80 %).
  It is what keeps pro's red skill from being a curve lever: a contract question for the Director (which distance is law 3's gate at pro).
- The ladder's 600-seed cells read pro's levers with ±4 points of noise even paired (hk_pressure_s 0.2: -3.8 at 600, -0.3 at 2,000): a trim is chosen at 2,000.
