# The difficulty curve, pass 4: the player the curve measures holds the stick like the mouse (lane A86, device A, 2026-11-13 15:16-16:15)

Scope (R-0043 point 2; cut by the Director at 15:49, R-0045 / `docs/VISION.md`): job 0 (the playtester classes hold the stick the way the mouse does) with its before and
after, the baseline on 5 v 5 with that player, the full `Jam` filter, and the levers the next curve lane should try first. **No lever and no budget was set**:
`DifficultyProfile.cpp`, the pro defaults in `JamAICore.h` and `HumanHasSomethingToDo`'s budget are as on main. Built on main 0f51082 (5 v 5 with A83's roles,
A85's carrier reach and flat tap, carry speed 0.92 with no sprint, A79's all_star). **All numbers are from before lane A91** (`shot.lift.full_m` 0.6 -> 0.5,
`game.shot.tap_frames` 4); section 6 names the cells to re-measure.

## 1. Method

A70's method (CURVE-2.md section 1): `UnrealEditor-Cmd -run=JamTournament -sim=jam -player=<class> -difficulty=<level> -seeds=lo-hi -maxframes=36000`, 8 headless
processes, n 2,000 per cell on A53's hockey.regular seeds (1-100 of each 150-band, 20 bands) and seeds 1-2,000 on hockey.arena.large; every arm of a comparison plays
the same matches (paired). Win % with the 95 % Wilson interval. Driver: `tools/perf/results/a86/curve.mjs` (one row per cell in `tools/perf/results/a86/cells.jsonl`,
raw match lines in `tools/perf/results/a86/raw/`). "fwo" = faceoffs with no human touch inside 600 frames (the `HumanHasSomethingToDo` window) as a share of all
faceoffs; "checks taken" = landed checks on the human a match (equal to its knockdowns in every cell); "scored < 2 min" = matches with a human goal inside 7,200 frames.
A 2,000-match cell takes 1.5 to 2.5 minutes.

## 2. Job 0: the stick (the size of the fiction)

**What the classes wrote before.** `JamPlaytester.cpp` left `FJamInput::Stick` at zero while carrying: the sim's rest blade is ahead of the body at half reach,
0.4 + 0.5 x 1.2 = **1.0 m**. Only the sweep (regular and good, a defender in front) and the forecheck's poke aimed it. **What a human writes.** `ReadHumanInput` ->
`CursorAimRink`: a cursor 6 m ahead gives magnitude 0.5 + 0.5 x clamp(5.5 / 2.5) = 1, so the blade sits at **1.6 m** in the cursor's direction, and the carry assist
pulls the puck there.

**The change** (`JamPlaytester.cpp`, `JamPlaytester.h`): while carrying, a unit stick (full reach) at the net while lining up a shot or taking a one-timer (the class's
noisy aim), at the mate while passing or holding a saucer, otherwise where it skates, bent away from the nearest opponent inside its pressure distance; the class's aim
noise turns it (the novice's 10 degrees: sloppy). Without the puck (and no mate carrying) the cursor sits on the puck (its direction, the cursor's magnitude rule by
distance). The sweep, the forecheck and the pass reception keep their own aims. The class's "am I carrying" reads the sim's rule (the puck last touched by me, within
`CarrierReach()` 1.65 m and riding with me), and the match's carry statistic reads `UJamSim::CarrierIndex`. The old stick stays as a measurement arm
(`JAM_PLAYTESTER_STICK=rest`, env only, default off; also `carry`, `front`, `short` below).

**Before and after, job 0 alone** (hockey.regular, n 2,000, same seeds, the brain's view of the carrier as on main):

| cell | before: stick at rest | after: the mouse's stick | target |
|---|---|---|---|
| novice at rookie | 95.0 (93.9-95.8); fwo 9.9 %; scored < 2 min 1,519 | **56.3 (54.1-58.5)**; fwo 15.5 %; scored < 2 min 361 | 85 |
| regular at pro | 39.0 (36.9-41.2); checks taken 3.30 | **45.0 (42.8-47.2)**; checks taken 3.06 | 50 |
| good at all_star | 13.4 (12.0-15.0); checks taken 4.18 | **19.9 (18.3-21.8)**; checks taken 2.83 | 25 |

The novice's collapse was not the stick's own doing. Arms on novice at rookie (n 600, same seeds): the blade held at 0.87 (1.44 m, inside `shot.reach_m`) **93.3**;
the mouse's stick only while carrying 57.8; the carry aim kept within 60 degrees of facing 58.2; **the mouse's stick with the brain's carrier on the carrier reach
96.8**. The cause is the brain's blind band: the adapter's `W.Carrier` tested `shot.reach_m` 1.5 (A85 left it there, MECHANICS-3.md section 3), so a human carrying at
the mouse's 1.6 m was a loose puck to every bot. Rookie bots never poke a carrier, but they pounce on a loose puck: the opponents' shots 11.4 -> 24.1 a match, the
human's goals 2.60 -> 0.85. **This is the game a mouse player plays on main today**, not a playtester artefact: R-0042 point 1 (one reach for every carrier test) was
not met on the brain's side. At pro and all_star the blind band made the bots *easier* (no poke, lift or hunt on a carry they did not see): +6 points for the regular
and the good player.

**The rule, landed** (`Game/Source/JamCore/AI/JamSimAdapter.cpp`, outside this lane's file list, flagged for the Director): `W.Carrier` uses
`UJamSim::CarrierReachM` (R-0042 to the letter). A66's cap on the bots' own blade stays (R-0044 point 4); the old view replays with `JAM_BRAIN_CARRIER=shot`
(measurement only). Built into the default and played with no environment, novice at rookie equals the env arm hash for hash on all 2,000 seeds.

Whole step, main's game (rest stick, old brain view) -> this baseline (mouse stick, brain on the carrier reach): novice at rookie 95.0 -> 97.2, regular at pro
39.0 -> 33.1, good at all_star 13.4 -> 12.9.

## 3. Baseline: 5 v 5, the mouse's stick, the brain on the carrier reach, profiles as on main (n 2,000 each)

hockey.regular (win % with 95 % interval; fwo share; checks taken a match):

| class | rookie | pro | all_star |
|---|---|---|---|
| novice | **97.2** (96.4-97.8); fwo 13.1 %; 0 | 5.3 (4.4-6.4); fwo 13.0 %; 1.42 | 0.9 (0.6-1.4); fwo 9.2 %; 0.94 |
| regular | 100.0 (99.7-100); fwo 4.8 %; 0 | **33.1** (31.1-35.2); fwo 5.8 %; 3.28 | 4.3 (3.5-5.3); fwo 4.9 %; 2.38 |
| good | 100.0 (99.8-100); fwo 2.1 %; 0 | 59.4 (57.2-61.5); fwo 3.4 %; 4.67 | **12.9** (11.5-14.5); fwo 2.8 %; 3.47 |

hockey.arena.large, the three target cells:

| cell | win % (95 %) | fwo | checks taken | target |
|---|---|---|---|---|
| novice at rookie | **96.7** (95.8-97.4) | 14.4 % | 0 | 85 |
| regular at pro | **58.3** (56.1-60.4) | 6.4 % | 2.13 | 50 |
| good at all_star | **13.8** (12.4-15.4) | 3.5 % | 2.25 | 25 |

Other shares on the target cells (hockey.regular): the novice scores inside two minutes at rookie in **1,363 of 2,000 (68 %)**; match length rookie 219 s, pro 277 s,
all_star 217 s. Rookie bots land no check on any class (they never check); the human takes 3.3 checks a match at pro and 3.5 at all_star (the good player 4.7 at pro).

Read against R-0029 (85 / 50 / 25): **every target cell is outside its interval on hockey.regular** (rookie 12 points too easy, pro 17 too hard, all_star 12 too
hard); on the large rink rookie and all_star likewise, pro 8 too easy. The step from rookie to pro is a cliff for everyone below the good player (novice 97 -> 5,
regular 100 -> 33), and all_star is out of reach for the regular player (4 %).

## 4. The gate on job 0 and the brain's rule (full `Jam` filter, `tools/analysis/jamtests.ps1 -Filter Jam`)

**126 passed, 1 failed** (`Game/logs/a55-automation-ue-20261113-155106.log`):
- `Jam.Bot.HumanHasSomethingToDo` **FAIL**: 20 of 163 faceoffs without a human touch (12.3 %), budget 10.4 % (16). The soak's share in that cell is **13.1 %**
  (1,869 of 14,282); by R-0039's rule (share + 2) the budget would be 15.1 %. **Not moved** (the scope change: no budget today). It is the only red test.
- `Jam.Playtest.NoviceScoresAtRookie`: 16 of 20 (need 10; A83: 13 of 20). The soak supports 10 of 20 (68 % inside two minutes over 2,000 matches).
- `Jam.Bot.PassesToHuman`: rookie 1.45, pro 2.75 received a match (bar 1.0). `SeasonTakes`: green, take-1 AI shots 5 / 6 / 7. `Jam.Core.Determinism`, `ModeKeys.Sprint`,
  `ProCharge`, `PlayerBotWins`, `DifficultyProfilesResolve`: green. Bot matches hash differently from main (the brain sees blade-length carries).
- Job 0 alone (the old brain view, `Jam.Playtest+Jam.Bot` at 15:24): `HumanHasSomethingToDo` 23 of 145 and `NoviceScoresAtRookie` **2 of 20**, both FAIL: the blind
  band again.

## 5. Levers to try first once the vision's lanes land (R-0045; for the final curve lane)

Re-measure this baseline first: contact by momentum, braking, burst and the carrier's sprint, and the bots' hockey principles all move every cell.
1. **rookie (novice 97 -> 85): red skill up.** Information only, n 600, this build: red 0.9 -> 92.8 (90.5-94.6), red 0.95 -> 87.2 (84.3-89.6). Then R-0041's
   "rarely checks": `opp.check_min_closing` 6 to 7 at rookie (rookie never checks today), which also starts the aggression ladder at its bottom rung. The large rink's
   `LargeRedSkill` 0.97 at rookie needs its own step (96.7 there).
2. **pro (regular 33 -> 50): red skill down** (0.95 -> 0.9 / 0.85) before speed (0.95 / 0.9); keep checks from 4 m/s and the lift (R-0041: pro checks and lifts as a
   matter of course). The large rink is already 58 at pro: its `LargeRedSkill` 0.97 moves the other way (try 1.0).
3. **all_star (good 13 -> 25): speed 1.2 -> 1.1 / 1.0** with the aggression kept (hunt 10 m, poke 0, lift, checks from 3). If momentum contact makes checks land from
   lower speeds, `CheckMinClosing` 2 becomes a real lever (A79: below 3 it added nothing because the sim's `check_speed_min` 5 gates the checker's own speed).
4. **The rookie-to-pro cliff** (novice 5 % at pro, regular 4 % at all_star): no R-0029 target; report it with the final table.
5. **Budgets**: `HumanHasSomethingToDo` = the final rookie novice share + 2 (13.1 % here -> 15.1 %); `NoviceScoresAtRookie` keeps 10 of 20 while the share inside two
   minutes stays above about 60 % (68 % here).
6. `mate.pass_to_human` does not move the touch share (A83); the 13 % fwo at rookie is the 5 v 5 crowd at the draw.

## 6. After lane A91 (the full charge's lift 0.5, the 4-frame tap)

Every number here predates A91. Re-measure first where shots decide matches: **regular at pro and good at all_star** on both rinks (the regular and good players
release full charges from range: the lower lift moves their shots on target), and **novice at rookie** on both rinks (its taps become flat by rule, which moves its
close-range goals and the two-minute share behind `NoviceScoresAtRookie`).

## 7. Files (relative to the worktree)

- `Game/Source/JamCore/AI/JamPlaytester.cpp`, `Game/Source/JamCore/AI/JamPlaytester.h`: job 0 (the stick, the class's carrying test, the carry statistic, the env arms).
- `Game/Source/JamCore/AI/JamSimAdapter.cpp`: `W.Carrier` on `UJamSim::CarrierReachM` (outside the brief's list; the finding of section 2).
- `docs/experiments/CURVE-4.md`, `status/A86.md`, `tools/perf/results/a86/**` (driver, chain scripts, cell rows, raw matches, gate logs).
