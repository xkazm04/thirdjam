# Movement 5: the second move, the window by level, the first step kept (lane A147, track F2, evolution 4, 2026-11-14)

Lane A128 (MOVEMENT-4.md) and the integration lane (INTEGRATION.md) left four things on the integrated tree (main d4bb4da): the footrace that a stop begun
early leaves (5 time-outs in 200 at 9.5 m), a drill that teaches one distance for three levels (all_star passes nothing at 8 m), the spin from 5 m (3.5
points over its bar), and the standing start by hand. This lane answers the four in the brief's order. Status with timestamps: `status/A147.md`. Raw logs,
sweep files and edit scripts: `Game/logs/a147/` (not landed).

Code: `Game/Source/JamCore/Public/JamCarryDrill.h` (the second move, the level's stop distance, the drill's line), `JamGame/Private/HokejGameMode.cpp`
(drill 21), `JamCore/Private/JamSim.cpp` (`StepSkater`, three blocks marked A147), `Public/JamCartridge.h` + `Private/JamCartridge.cpp` (one key),
`Public/JamBaseline.h` (its neutral, one line), tests `Private/Tests/JamCarrierSkillTests.cpp`, `JamMovementTests.cpp`, `schema/`.

## What landed

| Item | What | Default | Neutral / off |
|---|---|---|---|
| 1 | The drill's second move: the cut back when the first move left a footrace | `FJamCarryDrill::SecondMove` 1 | 0 (one move: the script of lanes A102 to A128) |
| 2 | Drill 21 stops where the level's window is, and says so on screen | pro 8 m, all_star 12 m, rookie 8 m | `jam.ctrl.CarryBrakeAtM` overrides |
| 3 | The spin from 5 m | nothing built: the bar is met on the integrated tree (35.0 %) | |
| 4 | The first step is kept through a bump, a slide and a turn on the spot | `game.skater.burst.rest_kept_mps` 3 | 0 (lane A128's rule) |

No state field was added to skater or puck (`RestFrames` and `BurstLeft` are hashed since lanes A128 and A93).

## 1. The second move (pro, the stop begun at 9.5 m, 200 seeds an arm, the real brain)

The footrace, from the traces: the carrier has stopped, cut and gone; the pro turned with it and skates 1.5 m behind its shoulder (seed 9) or level with
it on the goal side (seed 98) at the same 12 m/s; the carrier is pinned along the side boards and is never 2 m beyond. One move: **97.0 %** (5 time-outs:
seeds 9, 51, 98, 140, 183; 1 lift: 161).

Three forms of a second move were built and measured. All numbers are `Bar.Movement.StopAndGoSweep` rows (`Game/logs/a147/sweep-1..6.txt`).

| the second move | trigger | 9.5 m | what happened |
|---|---|---|---|
| none (one move) | | **97.0** | 5 time-outs, 1 lift |
| stop again, cut back 60 degrees | 60 frames after the go, defender within 3 m (build 1) | 61.0 | it fires in carries that were passing: 32 lifted, 25 down, 17 time-outs. After 40 / 50 / 75 / 90 frames: 33.5 / 42.0 / 71.0 / 66.0 |
| stop again, cut back | straight again for 30 frames, lead under 2 m (build 2) | 88.5 | still fires in carries about to pass and in carries already past (the 30-frame confirmation): 15 time-outs, 7 down. Seed 9 is the picture of the fix: the pro overruns along the boards, past at frame 262 |
| stop again, cut back | **the footrace**: not yet past, straight again, the last 30 frames gained under 0.5 m (build 3) | 96.0 | frees seeds 9, 51, 161; loses five carries one move passed (4, 37, 39, 95, 183 lifted), 67 stumbles, 140 down, 98 times out |
| the same, the wait 20 / 45 / 60 frames | | 95.5 / 94.0 / 97.5 | at 60 the five footraces all end **down**: no time-out and no puck |
| the same, gain 0.25 / 1.0 m; stop to 4 / 6 m/s; cut 45 / 80 degrees; only with the defender 1 m or more behind; 1 / 2 / 3 answers | | 95.5 / 96.0; 95.5 / 93.5; 94.0 / 96.5; 96.0; 96.0 each | |
| **the cut back without the stop** (the default) | the footrace | **97.5** | 4 time-outs, 1 lift: seed 98 (the pro level on the goal side) is freed, nothing is lost. 100 degrees, wait 20, gain 1: the same |

By where the stop begins (pro; one move / the cut back / the second stop):

| | 8 m | 9 m | 9.5 m | 10 m | 11 m | 12 m |
|---|---|---|---|---|---|---|
| one move | 100 | 99.5 | 97.0 | 94.0 | 86.0 | 58.0 |
| **the cut back** | **100** | **99.5** | **97.5** | **96.5** | **88.0** | **63.0** |
| the second stop, then the cut back | 100 | 97.5 | 96.0 | 87.5 | 77.5 | 54.0 |

**The brief's bar is not met**: the stop at 9.5 m is 97.5 %, not 99 %, and 4 carries still time out. The other two hold: the straight charge 0 % (bar
under 20), the stop at 8 m 100 % (bar 95). What the measurements say:

- **A second stop is read by a pro on the shoulder.** The brain brakes by decision (lane A115): it slows with the carrier, and the stop ends under its
  stick (the body is no longer between when both stand) or its body (the cut back crosses its path: seed 18, a check at frame 212). Stopping is the answer
  to a defender that comes at you, not to one that is with you.
- **The cut back is free**: never worse than one move at any distance measured, and it is worth 2 to 5 points where the stop was begun too early (10 to 12 m).
- **The footrace that is left is law 4.** The physics are the same for both skaters and the chaser is the faster one; a carrier 1.5 m ahead with the puck
  kept is not beaten, and it is not past. What ends it is not a third move of the same kind: it is a pass, a shot, or the drill's bar (section 8).

`FJamCarryDrill`: `SecondMove` (1), `SecondWait` (30 frames), `SecondGainM` (0.5), `SecondGapM` (3), `SecondMax` (2 answers, each back the other way),
`SecondCutDeg` (60), `SecondStopMps` and `SecondLeadMinM` for the measured stop, `SecondAfter` for the first form. The late answers (`Late()`) and the
gate's "lane A102's clean stop" (`DefaultGo()`) stay one move, so every older row is the carry it was.

## 2. The window by level, taught

The stop by where it begins, one move, 200 seeds a cell (build 1; pro and all_star agree with INTEGRATION.md section 4 to the carry):

| the stop begins at | 5 m | 6 m | 7 m | 8 m | 9 m | 10 m | 11 m | 11.5 m | 12 m | 13 m | straight |
|---|---|---|---|---|---|---|---|---|---|---|---|
| rookie | 27.5 | 70.0 | **100** | **100** | 100 | 100 | 100 | | 100 | | 42.5 |
| pro | | 6.0 | 65.5 | **100** | 99.5 | 94.0 | 86.0 | | 58.0 | | 0 |
| all_star | | | | 0 | | 56.5 | 89.0 | 98.0 | **100** | 100 | 0 |

(Rookie from 9 m up read 95.5 / 84.0 / 83.0 / 71.0 in sweep 1 with the first form of the second move firing; with one move or the cut back they are 100: sweeps 5 and 7. The level rows are the same carries with one move and with the cut back, except pro from 9.5 m out: section 1.)

- **Rookie's defender allows anything from 7 m out** (it does not press: R-0050). The drill keeps 8 m for rookie: the distance a player learns there is
  the one that works at pro.
- **All_star: 12 m, not the brief's 11 m.** 11 m passes 89.0 % (16 down, 3 stumbles, 2 lifts); 11.5 m 98.0; 12 m 200 of 200. The drill teaches the
  distance that works.

`FJamCarryDrill::StopAtForLevel(Level)`: pro 8, all_star 12, rookie 8. Drill 21 scripts 1 and 2 (by hand) take it from the level the brain plays
(`UJamBrain::Difficulty`); `jam.ctrl.CarryBrakeAtM` still overrides. **The drill's line** (`FJamCarryDrill::CoachLine`) goes to the HUD's notice
(`UJamHudWidget::ShowToast`, top left, 4 s) when a round is staged, and to the log as `JAMDRILL carry line ... text="..."`. The unit is a stick length: the
body's centre to the blade at full reach (`SkaterRadius` 0.4 + `game.stick.reach_m` 1.2 = 1.6 m), so the number follows a cut that changes the reach:

    pro, rookie:  STOP WHEN HE IS FIVE STICK LENGTHS AWAY. CUT. GO. STILL ON YOUR SHOULDER? CUT BACK
    all_star:     STOP WHEN HE IS EIGHT STICK LENGTHS AWAY. CUT. GO. STILL ON YOUR SHOULDER? CUT BACK
    script 3:     FOUR STICK LENGTHS AWAY: CUT AND SPIN, YOUR BACK TO HIS STICK        script 0:  STRAIGHT AT HIM: THIS IS WHAT DOES NOT WORK

**Drill 21 script 1 in the real game mode, headless** (`-game -nullrhi -Multiprocess`, 20 rounds, build 4; `Game/logs/a147/run-drill.ps1`, a copy of
hockey.regular with `ai.difficulty` set, logs `Game/logs/a147/drill-<level>.log`). The bar: 18 of 20 or better at each level.

| level | the stop begins at | past |
|---|---|---|
| rookie | 8 m | **20 of 20** |
| pro | 8 m | **20 of 20** |
| all_star | 12 m | **20 of 20** |

## 3. The spin from 5 m: the bar is met, and contact is not the lever

The spin with the cut, pro, 200 seeds (sweep 2): from 4 / 5 / 6 / 7 m: 5.5 / **35.0** / **59.5** / 95.5. Lane A128 missed "under 50 % from 5 m" by 3.5
points on its own tree; on the integrated tree the brain answers half of the spins' exposed ends (lane A138, `hk_spin_end_share` 0.5) and the number is
35.0 (with `spin.exposed_frames` 0: 60.0 / 99.5). The losses from 5 m: 53 lifted, 72 down, 3 stumbles, 2 touches.

The brief asked whether the answer lies in contact (a spinning skater's stability). Measured, read only (`game.contact.spin_stability`, default 0.8):

| `spin_stability` | 0.5 | 0.65 | 0.8 | 1.0 | 1.25 | 1.6 |
|---|---|---|---|---|---|---|
| the spin from 5 m, past % | 35.0 | 35.0 | 35.0 | 35.0 | 36.0 | 40.5 |
| down / stumble of 200 | 75 / 2 | 73 / 3 | 72 / 3 | 68 / 4 | 50 / 21 | 34 / 25 |
| the spin from 6 m | 59.5 | 59.5 | 59.5 | 59.5 | 59.5 | 59.5 |

Doubling the spinner's stability turns knockdowns into stumbles with the puck loose and moves the share by 5 points; from 6 m nothing. **No rule was
built and none is proposed**: the bar holds, and the check that lands on a spin begun at 5 m is a check at 11 m/s on a slow skater. For track F1: nothing
to change for this bar. The cut alone from 5 / 6 m: 5.0 / 53.5; the deke 33.5 / 52.0.

## 4. The first step by hand

`Bar.Movement.FirstStepCases` (outside the gate): the human alone, or with one opposing skater that bumps or checks it; frames from the push to 8 m/s,
metres covered in the first 30 frames, the burst it had. Build 2; `rest_kept_mps` 2.5, 3 and 4 give the same rows.

| the start | lane A128's rule (`rest_kept_mps` 0) | **this lane (3)** | |
|---|---|---|---|
| from rest (30 frames), W | 24 frames, 2.65 m, burst 20 | 24, 2.65 m, 20 | the reference |
| at rest, bumped to 2 m/s, W at once | 17, burst 19 | 17, burst 20 | covered |
| at rest, bumped to 2 m/s, W six frames later | 20, 3.40 m, burst 13 | **17, 3.64 m, burst 20** | the burst bled from the bump |
| **at rest, bumped to 2 m/s, W 24 frames later** | **31, 2.50 m, burst 0** | **17, 3.62 m, burst 20** | **the hole: fixed** |
| at rest, run into by a gliding skater, W from the contact | 37, burst 19 | 36, burst 20 | covered (the time is the turn across) |
| **the push straight behind the facing (180 degrees)** | **63, 0.84 m** | **55, 0.95 m** | the pivot spent the burst: fixed; the rest is the turn |
| the push 135 degrees off the facing | 48, 1.81 m | 46, 1.87 m | |
| the push 90 degrees off the facing | 33, 2.30 m | 33, 2.30 m | the turn, not the first step |
| one frame of W tapped, W held three frames later | 25, burst 16 | 25, burst 16 | the tap was the push-off: as meant |
| knocked down at 8 m/s head-on (150 frames down), W held throughout | 33, burst 20 | 33, burst 20 | covered (it lies still by the release; pushed across: the 90 degree start) |
| the faceoff, W held through the countdown (pushed across) | 33, burst 20 | 33, burst 20 | covered: the loser of the draw has its first step too |
| standing, body-checked at 9 m/s (a bump: no stumble, not down) | 42, leaves the hit at 6.6 m/s | 42 | no standing start: it is sliding at 6.6 m/s (section 9) |
| a charge let go and taken up again (never at rest) | 23, burst 0 | 23, burst 0 | no burst, as meant |

**Where a player expects the quick first step and did not get it**: (a) after a bump or a shove that moved it while it stood: the armed burst counted
down from the bump and was gone 20 frames later (0.3 s), although the skater had not pushed; (b) pushing off behind its facing: the frames it turns on
the spot counted as the push-off, so the burst ran out during the turn; (c) by the code and the arithmetic, not staged: a skater knocked down at more
than 7.6 m/s for the shortest time (60 frames; the slide decays by 2 /s) is still above 1 m/s at the release and never counted its rest.

**The rule** (`game.skater.burst.rest_kept_mps`, 3 m/s; `UJamSim::StepSkater`): the first step a skater earned by standing is lost only to its own push.
While it does not push and is slower than `rest_kept_mps`, a rested skater keeps its rest and its armed burst; a skater lying on the ice or held by a
stumble counts its rest whatever it slides at; turning on the spot is not the push-off. Earning the rest is unchanged (under 1 m/s for `rest_frames`,
not pushing), so a glide never earns it, and a charge of the skater's own never has it. With `rest_frames` 0 the rule is dead.

**Not fixed, on purpose**: the start behind the facing still takes 55 frames (0.9 s) against 24 forward. That is the turn (the skater thrusts along its
facing from 120 degrees off the input), not the first step: the next evolution's candidate (section 8).

## 5. The three cells: the first step kept, alone

The second move and the drill are a script and a drill: no match changes. The key is the one sim rule. 600 matches a cell, hockey.regular, seeds 1 to
600, paired; `tools/perf/results/a147/cells.mjs` and `pair.mjs` (copies of lane A132's, track F1: one process a cell, three at once), rows in
`tools/perf/results/a147/cells.jsonl`, raw in `raw/`. Before = the key at 0 through the loader; after = the default.

| cell | R-0046 | before (key 0) | **after (3)** | move | the human knocked down a match | goals for : against |
|---|---|---|---|---|---|---|
| novice at rookie | 85 | 98.0 (96.5-98.9) | **98.8** (97.6-99.4) | +0.8 | 2.35 -> 2.47 | 2.97 : 0.47 -> 2.98 : 0.39 |
| regular at pro | 48 | 66.7 (62.8-70.3) | **68.3** (64.5-71.9) | +1.6 | 7.40 -> 7.04 | 2.40 : 1.63 -> 2.45 : 1.57 |
| good at all_star | 26 | 38.0 (34.2-41.9) | **36.0** (32.3-39.9) | -2.0 | 7.75 -> 7.48 | 1.76 : 2.29 -> 1.75 : 2.32 |

No cell moves more than 4 points: **the rule ships on**. The seeds are 1 to 600 and not A53's bands (the track's `a102/curve.mjs`), so that the before
arm could be laid match for match on main's own base cells (lane A132 measured them tonight): **the before arm is main's game hash for hash, 1,800 of
1,800 matches** (`node tools/perf/results/a147/same.mjs off main`: rookie, pro, all_star 600 of 600 each; `raw/main-*.jsonl` are lane A132's base raw).

## 6. Gate, hash, neutral value

- **Full `Jam` filter on build 5 (the final tree): 210 pass, 0 fail** (`Game/logs/a147/jam-full-2.log`; build 4, which differs by one help text and
  one comment, the same 210: `jam-full-1.log`; the drill runs of section 2 are build 4's): the 208 of this commit, plus
  `Jam.Core.Movement.SecondMove` and `Jam.Core.Movement.FirstStepKept`. `Jam.Core.Movement.StopAndGo` (law 3) green and untouched: pro 8 m 100, straight
  0, 9.5 m 97.0; all_star 11 m 89.0, 12 m 100. `Jam.Net.HashCoverage` green (no new field).
- **Hash** (`Jam.Core.Determinism`, seed 1, frame 600): hockey.regular `ef34dd88c7c62b47`, hockey.raw `79d43eb5ea69a468`, before and after (the rule
  does not fire in those 600 frames). The default pro bot match (seed 15): before 4229 frames, 1:3, `7683cee2cf6589d1`; after 4959 frames, 3:1,
  `c4d27f8a96171ed9`.
- **Neutral value**: `"skater": { "burst": { "rest_kept_mps": 0 } }` gives back main d4bb4da: `Jam.Core.MovementNeutral` prints the default pro match
  of that block as 4229 frames, 1:3, `7683cee2cf6589d1`, the hash the full filter of main printed for its default match; and the 1,800 playtester matches
  of section 5. The line is printed, not pinned (R-0053 point 5: no new pin on a default-profile match while the curve lane re-sets the levels).
  `JamBaseline::MakeNeutral` carries `BurstRestKeptMps = 0` in one line: every older pin holds.
- **Tests**: new `Jam.Core.Movement.SecondMove` (the cut back is never worse than one move and takes time-outs away; the stop at 8 m 95 % or more; the
  straight charge under 20 %; each level's drill distance passes that level 95 % or more; the words), `Jam.Core.Movement.FirstStepKept` (the bump 24
  frames before the push keeps the whole burst and is worth 8 frames or more; the key at 0 is lane A128's rule; the start behind the facing is quicker;
  never on a charge). `Jam.Core.MovementNeutral` gained this lane's block. Outside the gate: `Bar.Movement.FirstStepCases`; `Bar.Movement.StopAndGoSweep`
  reads `Game/logs/a147/sweep.txt` or the file env `JAM_SWEEP_FILE` names, and knows the `second*` keys and `burst.rest_kept`. No test was re-pinned.
- Node tests of the gate (`schema/**`, `tools/**`, `devlog/**`): 243 pass, 1 fail that is the worktree's and not this lane's (`tools/barks`: the
  embedded C++ copy against `barks.json`, the CRLF copy lanes A102, A121, A128 and A138 reported). The ownership step runs git: not run.
- **Schema**: `game.skater.burst.rest_kept_mps` (0..8, default 3) in `schema/game.schema.json`, `limits.json`, `DEFAULTS.md`, `README.md`; the three
  generators re-run (`gen-schema-data` 3 files, `gen-parity`, `gen-designer-data` 36 cuts).
- **Revert path**: the key at 0 in a cartridge, or `FJamSkaterConfig::BurstRestKeptMps` in `JamCartridge.h`. The second move: `FJamCarryDrill::SecondMove`
  0. The drill's distance: `StopAtForLevel`, or `jam.ctrl.CarryBrakeAtM`.

## 7. What is queued

Another lane owns the cursor: a Development package of this tree and the acceptance run are queued with the Director. No acceptance row stages a bump on
a standing skater or a start behind the facing; row 9 (deke / spin / sweep) is untouched. By hand: drill 21 script 2 at each level with the line on
screen (the notice is top left for 4 s a round: whether that is long enough to read is a call for eyes), W after a bump at a faceoff scrum, S from rest.

## 8. Open, and what the numbers say for the next evolution

1. **The footrace is the drill's bar.** 4 carries in 200 at 9.5 m (5 at 10 m, 9 at 11 m) end with the puck kept and the pro behind or level. A carrier
   that has kept the puck for six seconds with the defender behind it has not lost. Either the drill counts "kept and ahead for 90 frames" as past, or the
   script ends the carry with what a player does there: a shot or a pass. A third skating move of the same kind does not do it (section 1).
2. **The turn on the spot.** 180 degrees from rest: 55 frames to 8 m/s against 24 forward; the skater begins to thrust along its facing while it is
   still 120 degrees off the input. Holding the pivot to 60 degrees is the candidate; it moves every skater, so it needs the cells.
3. **The second stop wants a defender that commits.** It worked exactly where the pro was still catching up (seed 9, 30 frames into the footrace) and
   failed where it had settled. A level whose defender overruns a second stop (rookie, perhaps pro) and one whose defender reads it (all_star) would be a
   skill a player can feel between levels: a profile number for the brain track, with `Bar.Movement.StopAndGoSweep` `second=2` as the instrument.

## 9. Seen on the way, outside the family

- A standing skater that is body-checked by a skater at 9 m/s and takes it as a bump (no stumble, not down) leaves the hit at 6.6 m/s
  (`Bar.Movement.FirstStepCases`, the row "standing, body-checked"). VISION law 1 says of a bump "a little speed changes hands". Not traced: for track F1.
- A Bash command line over about 8,000 characters is cut by this machine's shell wrapper (three heredocs failed that way tonight); long files go through
  a file write.
