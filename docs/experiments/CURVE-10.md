# The difficulty curve, pass 10: the answers ready before the operator plays (lane A170, track C0 evolution 6, 2026-11-14 06:00-)

Scope (brief `briefs/A170-C0-evolution-6.md`): main ca493723 = package H28 (c5dc15e5) plus rules that are all switched off. Measurement only; nothing lands
before the operator has played. The ladder as shipped (R-0062, 2,000 a cell): 85.8 / 52.4 / 28.8 regular, 83.8 / 50.2 / 26.1 large. Instrument: the
ladder's (`JAM_PLAYTESTER_CHECK=closing`) unless a row says "course" (the press as the config ships, R-0060's second column). Driver and rows:
`tools/perf/results/a170/` (`curve.mjs` = A162's; `cells.jsonl`; raw match lines under `raw/`, not landed). Status and every interim row: `status/A170.md`.

The tree is the package's: pro on hockey.regular as built here against A162's rows, 100 of 100 seeds identical (result, goals, frames, state hash), on
build 1 and again on build 8.

## 1. Pro's hunt radius as a dial (BRAIN-5 section 3's lever)

With `opp.hk_press_in_hunt=1;opp.hk_press_commit_m=8` the pro presser keeps its Press job inside the hunt and commits at 8 m, as it does today; the hunt
radius then moves only the other four. The two keys at hunt 8 are today's pro (100 of 100 hashes).

| hunt (m) | pro regular, 600 | pro regular, 2,000 | law 3 (200 a drill): 8 / 9.5 / 10 / 11 / 12 m; straight |
|---|---|---|---|
| 8 (as built) | 50.5 | 52.4 | 100 / 97.0 / 96.0 / 86.0 / 61.5; 0 |
| 10 | 49.7 | | identical |
| **11** | **48.0** | **48.0 (45.9-50.2)** | **identical** |
| 12 | 44.7 | 45.6 (43.5-47.8) | identical |
| 13 / 14 | 42.0 / 42.2 | | identical |
| 16 | | | identical |

Large rink pro at hunt 11: **47.7** (45.5-49.9; 2,000; as built 50.2). A121's bar (9.5 m at 95 %) holds at every value: the drill does not read the hunt.

**What it costs the player** (pro, regular, closing; the same 2,000 seeds; traced columns: REVIEW-3's `laws.mjs` / `humancarry.mjs` on 200 seeds):

| | hunt 8 | hunt 11 | hunt 12 |
|---|---|---|---|
| knocked down a match / a play minute | 6.88 / 2.66 | 7.63 / 2.72 | 7.71 / 2.76 |
| on the ice, % of play (traced) | 8.36 | 8.56 | 9.34 |
| shots against a match / a play minute | 10.03 / 3.87 | 10.88 / 3.88 | 10.96 / 3.92 |
| opponents' goals a match; from 25 m or more (traced) | 1.88; 0.77 | 2.12; 0.76 | 2.27; 0.82 |
| touches a play minute; his carry median (won himself) | 12.44; 0.78 s | 12.34; 0.78 s | 12.43; 0.78 s |

Hunt 11 costs 0.75 more knockdowns a match (+2 % a play minute; the matches run 8 % longer) and 0.2 points of play lying down; hunt 12 a full point.

**What else it moves** (no shipped cartridge writes a hunt radius; every cartridge at pro inherits it; 100 a mode / 200 a take, paired, closing):

| | as built | hunt 11 |
|---|---|---|
| novice take-5 / 6 / 7 | 65.5 / 60.5 / 54.0 | 72.0 / 57.5 / 50.0 (R-0057 steps 13.0 / 14.5 / 7.5: holds, better) |
| regular take-5 / 6 / 7 | 100 / 99.0 / 89.5 | 99.5 / 99.0 / 87.5 |
| modes (regular class) | arsenal 60, bruisers 53, cage 68, fog-of-war 86, glass-house 48, marathon 77, outnumbered 62, pond 73, power-play 62, shotgun 51, sniper 53 | 59, **43**, 64, **76**, **37**, 75, 66, 64, 63, 43, 54 |

Two prepared patches (`patch -p1` from the repo root; both dry-run clean on the main checkout; neither applied):
- `tools/perf/results/a170/A170-pro-hunt11.patch` (everywhere at pro): `JamAICore.h` ProOppSkill `HuntRadiusM` 8 -> 11, `Hk.bPressInHunt` true,
  `Hk.PressCommitM` 8; `HockeyBrain4Tests.cpp:107` (pro's switch now asserted on); `HockeyBrain5Tests.cpp:278` (the "as shipped = brain5=0" arm keeps pro's
  profile commit: brain5=0 clears every PressCommitM). Built = tune 600 of 600 (regular and large); full Jam 265 of 265.
- `tools/perf/results/a170/A170-pro-hunt11-pinned.patch` (the regular game and the season only): the above plus a new cartridge key
  `game.ai.hunt_radius_m` (0..40; loader, resolver, schema, limits, DEFAULTS/README rows, the two generated embeds) written as 8 in the eleven modes, the
  two parked modes and hockey.arena.wide / boost / chaos / raw; `DifficultyTests.cpp:174` allows that one explicit key. Fog-of-war, bruisers, glass-house and
  hockey.chaos pinned play the shipped tree's matches 100 of 100; full Jam 265 of 265; node tests 252 / 1 (the worktree's CRLF BarkData, known).
- Neutral (either patch): `opp.hunt_radius_m=8;opp.hk_press_in_hunt=0;opp.hk_press_commit_m=0`; `JamBaseline::BrainTune("pro")` already gives it back
  (`brain4=0;brain5=0;opp.hunt_radius_m=8`): no baseline line is needed.

## 2. The same dial for the player who lands his checks (course press; pro reads 56.0 as built, 600)

| hunt | 12 | 14 | 16 |
|---|---|---|---|
| course press (600) | 46.8 | 48.0 | 41.2 |
| closing press, the same hunt (600) | 44.7 | 42.2 | 42.7 |

At 2,000: course hunt 11 **49.3**, 12 **46.8**, 13 **44.0**; the whiffer (closing) 48.0 / 45.6 / -. **Hunt 11 serves either player** (49.3 / 48.0).

## 3. The switched-off set, all on together (contact: push-out 0.6, cooldown 60 with restart, lunge 12 / 1.5, slam 30; the mates' screen 30; the mates' keep-out 1.3 / 0.7)

| cell (600, closing) | as built (A162, same seeds) | all on | one number with all on |
|---|---|---|---|
| rookie regular | 85.3 | 89.2 (+3.9) | red 1.0: **86.8** |
| pro regular | 50.5 | 56.5 (+6.0) | the dial, hunt 12: **47.8** (13: 43.7) |
| all_star regular | 29.8 | 32.8 (+3.0) | commit 8: 33.5, 6: 33.7, `opp.hk_pressure_s` 0.2: 31.7 (no lever found) |
| rookie large | 83.7 | 87.0 (+3.3) | none needed (within 3); red 1.15: 81.7 |
| pro large | 46.7 | 51.5 (+4.8) | hunt 12: 43.3, 13: 45.0 (11 not measured) |
| all_star large | 25.3 | 28.8 (+3.5) | within 3; pressure 0.2: 27.3 |
