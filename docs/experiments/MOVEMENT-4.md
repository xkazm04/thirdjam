# Movement 4: every answer has a price, and the first second feels quick (lane A128, track F2, evolution 3, 2026-11-13)

Lane A121 (MOVEMENT-3.md) left three things: a standing start that is slow for everyone (0 to 8 m/s in 41 frames: the burst belonged to a stop or a cut of
the skater's own), a spin that was too sure (100 % past a pro from 6 m), and a drill that taught the stop where it no longer works best (9.5 m; the window's
centre is 8 m). This lane adds two rules, each one key with a neutral value, fixes `UJamSim::Restart`, and moves the drill's default. Status with timestamps:
`status/A128.md`. Raw logs: `Game/logs/a128*` (not landed).

Code: `Game/Source/JamCore/Private/JamSim.cpp` (`StepSkater`'s burst and rest counter, `IsPokeShielded`, `Restart`, `FaceoffReset`, `StepSpin`, `StateHash`),
`Public/JamSim.h`, `Public/JamCartridge.h` + `Private/JamCartridge.cpp` (two keys), `Public/JamCarryDrill.h` (default stop 8 m),
`Private/Tests/JamCarrierSkillTests.cpp`, `Private/Tests/JamMovementTests.cpp`, `JamGame/Private/HokejGameMode.cpp` (drill 21's texts and log), `schema/`.

## 1. The rules

| Rule | Key | Default | Neutral | State (in `StateHash`) |
|---|---|---|---|---|
| The standing start | `game.skater.burst.rest_frames` | 6 | 0 | `FJamSkaterState::RestFrames` (while not 0) |
| The spin's price | `game.skater.spin.exposed_frames` | 5 | 0 | none new (`SpinFrames` is hashed already) |

**The standing start.** A skater that has been slower than 1 m/s (`UJamSim::BurstRestSpeed`) and not pushing for `rest_frames` frames in a row has earned
its first step: its burst is armed while it stands and runs `burst.frames` (20) from its first push. A faceoff sets the counter full (the countdown does not
step skaters, and everybody has stood through it); a skater that lies on the ice counts its rest while it lies. Lane A121's rule stays beside it: a stop or a
cut of the skater's own arms the burst as before. Never on a charge already under way: the counter is 0 the frame the skater pushes or is at 1 m/s.

**Why "from its first push" and not "until it moves at 1 m/s".** The first form of the rule kept the burst armed until the skater was at 1 m/s. The carry
scene's pro defender does not charge at once: it creeps into its gap on a small move input for about 25 frames and then commits. Under the first form it
kept the burst through the creep and committed with it; the stop at 8 m fell from 100 % to 76 % (section 2). A burst that belongs to the push-off is the
vision's sentence taken literally (the first step is the step): the human on W gets all 20 frames; a skater that pushes off gently has spent it by the time
it decides to go. The switch between the two forms stays in code (`FJamSkaterConfig::bBurstRestPushOff`, no key) for the sweep.

**The spin's price.** For the last `exposed_frames` frames of a spin the figure has come round and the body is no longer between: neither the body nor a sweep shields the
carried puck from a stick (the take grace still does: R-0049 stands). A lift or a poke that lands there takes it. A spin begun too early ends in front
of the defender; a spin begun on time has the back turned when the stick comes. The brief named three places for the price (the recovery after the spin,
the puck's exposure at its end, the stamina); all three and a fourth (the first frames of the spin) were measured (section 4). The recovery and the head stay
in code as candidates, off, with no key (`SpinRecoveryFrames`, `SpinExposedHead`; `FJamSkaterState::SpinRecoverLeft` is their counter, 0 in a shipped game).

**Restart.** `UJamSim::Restart` now resets `BurstLeft`, `MoveHist` and `MoveHistAt` (lane A121's finding at `JamSim.cpp:173`), and the two counters of this lane.

**The drill.** `FJamCarryDrill::BrakeAtM` defaults to 8.0 (9.5 before): drill 21 script 1 teaches the stop at the centre of the window. The gate's rows that
mean "lane A102's clean stop at 9.5 m" say 9.5 explicitly (`DefaultGo()` in `JamCarrierSkillTests.cpp`).

## 2. The standing start, measured (carry scene, the real brain at pro, 200 seeds an arm)

| | 0 to 8 m/s from rest | stop at 9.5 m | stop at 8 m | stop at 7 m | straight charge |
|---|---|---|---|---|---|
| the baseline (`rest_frames` 0) | 41 frames | 95.0 | 100 | 83.5 | 0 |
| armed until 1 m/s, burst 20 (sweep 1) | 23 | 98.5 | **76.0** | 17.5 | 0 |
| armed until 2 m/s | 23 | 99.5 | 55.5 | | |
| armed until 1 m/s, the rest's burst 16 / 14 / 12 frames | 26 / 27 / 29 | 99.5 | 88.0 / 91.0 / 95.5 | | |
| **from the first push, burst 20 (the default)** | **24** | **97.0** | **100** | 65.5 | **0** |

The brief's bars: 0 to 8 m/s in 26 frames or fewer (24: after standing 30 frames and straight out of the faceoff); the stop at 8 m at 95 % or more (200 of
200); the straight charge under 20 % (0 of 200); the pro cell within 2 points of 79.0 (79.5 with this rule alone, section 5). A charge already under way has
no burst: W pressed while gliding at 6 m/s reaches 8 m/s in the same 11 frames with the key at 6 and at 0.

**The defender starts from rest too, and the level decides what it does with it.** With the default rule, the stop by where it begins (pro): 10 m 95.5,
9.5 m 97.0, 9 m 99.5, 8.5 m 100, 8 m 100, 7 m 65.5, 6 m 5.5, 5 m 0. The late edge moved half a metre out (7 m was 83.5, 6 m 23.5): the creeping pro is a
little quicker through its creep. **All_star does not creep: it commits at once and has its first step**: its stop at 9.5 m falls from 90 % to 38.5 %, at
8 m to 0, and the window is at 11 m (89.0; 94.5 before). Rookie: the stop at 8 m 100, the straight charge 48.5.
At 8 m under the defaults: the blade left ahead 42.5 (the mouse is worth 57 points), `body_shield` false 64.5.

## 3. What the bars rest on

The stop at 8 m passes because the pro brain creeps before it commits and the push-off rule lets the creep spend the burst. A brain that stands and then
commits (as all_star's does) keeps its first step, and the window moves out a metre and a half. The curve track and track Q2 should know: a change to the
pro defender's approach moves this number (from 100 toward 76).

## 4. The spin's price, measured (the spin with the cut, no stop; pro, 200 seeds; under the standing start)

| candidate | from 6 m | from 5 m | from 7 m |
|---|---|---|---|
| no price (`exposed_frames` 0) | 99.5 | 60.0 | 100 |
| **the exposed end inside the spin**: its last 2 / 3 / 4 / **5** / 6 / 8 frames | 83.5 / 75.0 / 64.5 / **53.0** / 42.5 / 27.0 | 60.0 / 60.0 / 58.5 / **53.5** / 44.0 / 21.5 | - / - / - / 90.5 / - / - |
| the recovery after the spin: 12 / 24 / 40 frames, with the thrust or without | 73.5 | 60.0 | |
| the end and after: 2+6 / 2+12 / 3+3 / 3+6 / 4+4 / 5+5 / 6+6 | 57.5 / 57.5 / 55.0 / 49.0 / 41.0 / 27.5 / 16.5 | 60.0 / 60.0 / 60.0 / 60.0 / 58.5 / 53.5 / 44.0 | 49.0 / 29.0 / 71.0 / 49.0 / 60.0 / 55.5 / 49.0 |
| the exposed head: the first 8 / 12 / 16 frames | 99.5 | 60.0 / 60.0 / 38.5 | |
| head 16 with end 4 (two numbers) | 64.5 | 37.0 | |
| the contact family's `spin_stability` 0.6 (read only, on build 1) | no move | no move | |

What the traces show: the pro presses one lift at contact; from 6 m it lands in the last five frames of the spin or just after it, from 5 m in the spin's
middle; the brain's lift cooldown then outlasts the encounter. From 5 m the losses are body checks (72 down of 200): no rule about sticks moves them.
The standing start alone had already taken the spin from 5 m from 82.5 % to 60.0 % and from 4 m from 26 % to 5.5 %.

**The brief's bars**: 50 to 70 % from 6 m (**53.0**, was 100); under 50 % from 5 m (**53.5: missed by 3.5 points**, was 82.5); the deke tap near its 56 %
(**52.0**, the same carry with the key at 5 and at 0); the stop at 8 m at 95 % or more (**100**). No single number gives both spin bars: the end of 6
frames gives 42.5 / 44.0, the end of 5 gives 53.0 / 53.5; only two numbers do (a head of 16 with an end of 4: 64.5 / 37.0), which leaves a shield of four
frames in a turn of 24. One key was the brief: the end of 5 is the nearest.

Left open on purpose: a spin begun at 7 m still passes 90.5 % (it is over before the defender arrives and the body then shields as for any carrier). The
recovery after the spin closes that (2+6: 49.0 from 7 m, 57.5 from 6 m) and is in the code, off: the next evolution's first candidate.

## 5. The three cells: the move of this evolution alone (the levels were not tuned)

600 matches a cell, hockey.regular, A53's seeds, paired, both arms on build 4 (the final sim); `e4-*` in `tools/perf/results/a102/cells.jsonl`, raw in
`raw/`; before = this lane's neutral block through the loader (`skater.burst.rest_frames=0;skater.spin.exposed_frames=0`). Two workers at a time.

| cell | R-0046 | before (neutral block) | **after** | move | goals for : against | the human knocked down a match | the human's carries of 3 s a match |
|---|---|---|---|---|---|---|---|
| novice at rookie | 85.3 | 99.3 (98.3-99.7) | **99.2** (98.1-99.6) | -0.1 | 2.98 : 0.28 -> 2.98 : 0.32 | 2.29 -> 2.17 | 3.24 -> 3.34 |
| regular at pro | 48.3 | 79.0 (75.6-82.1) | **77.0** (73.5-80.2) | **-2.0** | 2.60 : 1.32 -> 2.58 : 1.40 | 9.56 -> 9.32 | 2.72 -> 2.48 |
| good at all_star | 26.3 | 43.2 (39.3-47.2) | **47.5** (43.5-51.5) | **+4.3** | 1.87 : 2.17 -> 1.98 : 2.11 | 10.79 -> 11.10 | 0.99 -> 0.69 |

**The before column equals lane A121's after column in every figure** (99.3 / 79.0 / 43.2, the goals, the knockdowns): the neutral block gives back the
baseline on 1,800 matches, not only on the pinned one.

The pro cell with the standing start alone (build 2, the spin's price 0; `e3-pro-rest`): **79.5** (76.1-82.5), +0.5 on 79.0: inside the brief's 2 points.
So at pro the standing start is worth +0.5 and the spin's price -2.5: the scripted human spins, and its carries of three seconds fall from 2.72 to 2.48 a match.
The all_star cell moves by 4.3 points, 0.3 over the common brief's line of 4 (the intervals overlap: 39.3-47.2 against 43.5-51.5). The levels are not tuned
here (the brief: the curve track re-sets them). The all_star cell with one rule at a time (600 matches each, the same seeds; `e4-as-rest-only`,
`e4-as-spin-only`): the standing start alone **45.5** (41.6-49.5, +2.3), the spin's price alone **41.0** (37.1-45.0, -2.2), both 47.5: the two do not add
up, and each alone is inside the noise of 600 matches (about 4 points either way).

## 6. Gate, hash, neutral value

- **Full `Jam` filter on build 5 (the final tree): 163 pass, 0 fail** (`Game/logs/a128-tests-jam-full-3-ue.log`; lane A121's 161 +
  `Jam.Core.Movement.StandingStart` + `Jam.Core.Movement.SpinPrice`), no "missing usage flag" line. Builds 1 to 4 on the way: build 1 with both defaults
  neutral gave the baseline's hashes and numbers; build 3 was red twice (below); build 5 differs from build 4 in one test only.
- **Hash** (`Jam.Core.Determinism`, frame 600, defaults): hockey.regular `ef34dd88c7c62b47` (before `0ff7ea98d5570484`), hockey.raw `79d43eb5ea69a468` (before
  `8425610943e6dd92`). The default pro bot match (seed 15): 5618 frames, 0:3, `a54f600ec8d9b3de` (before: 6000 frames, 1:1, `c8ae80c1dbdc45b0`).
- **Neutral value** (`Jam.Core.MovementNeutral`): this lane's block written as a cartridge author would,
  `"skater": { "burst": { "rest_frames": 0 }, "spin": { "exposed_frames": 0 } }`, gives back the baseline's default match: 6000 frames, 1:1,
  `c8ae80c1dbdc45b0`. Lane A121's block (with this lane's two keys at 0: the blocks stack) still gives `fc59e23598229ebc`; all the neutral blocks together still
  give main bb18439's two matches (`874f67830b20dc4f`, `59762ff73c73f6e3`). The cells' before arms are lane A121's after arms figure for figure (section 5).
- **Fields in the lockstep hash** (`UJamSim::StateHash`): `FJamSkaterState::RestFrames` (tag 0x128A, while not 0: always 0 with `rest_frames` 0) and
  `FJamSkaterState::SpinRecoverLeft` (tag 0x128B, while not 0: always 0 in a shipped game, its number has no key). `Jam.Core.Movement.StandingStart` holds
  that the rest counter changes the hash. Nothing new is read by the sim outside the hash (`bThrust` is a local of `StepSkater`).
- **Tests re-pinned, in the family**: `Jam.Core.Movement.Burst` case d (lane A121's "a skater that merely stands has no burst" holds with `rest_frames` 0; with
  the default it has the whole burst); `.StopAndGo` (the mouse's 20-point bar is held at 8 m: 42.5 against 100; at 9.5 m the blade left ahead costs 14 points
  now, bar 10); `.LateAnswer` ("the spin is the surer answer" became "an answer, not the sure one": at most 70 %).
- **`Jam.Core.Movement.TakeGrace`, the in-play bar re-pinned, and why.** Under this lane's defaults the playtester's three pro matches are other matches, and one
  of them has a stick takeaway 8 frames after a win (lane A121's three: none inside 30 frames). The test now prints it: seed 2, frame 8488, a clean poke by
  skater 6 on owner 0; the frame before, the owner carried, its grace had 23 frames left, it was not spinning. The count is the same whether the spin's exposed
  end overrides the grace (build 3) or not (build 4), so it is not this lane's exposure: **the take grace has a hole of its own**. The code says where:
  `ResolvePokes` runs right after `StepPuck` (`JamSim.cpp` 1094-1096), the carrier is the last toucher in reach (`CarrierIndex`), and a new carrier's grace
  is set in the possession loop at the end of `Step`. A puck that comes to a new carrier in `StepPuck` (here from the owner to a mate beside it) can be poked
  by a window already open in that same frame: the new carrier's grace is not up yet. Not verified by a trace; the one-line candidate for the next evolution is
  in `IsPokeShielded`: a carrier that is not yet `TakeOwner` (and the puck not placed) counts as inside its grace. The bar is
  now "at most one in nine minutes, and at most a tenth of the count without the grace" (1 against 51); the staged scene of the rule is unchanged and green.
  It is the track's next evolution's first item.
- **Two reds on build 3, both this lane's, both fixed in build 4**: the spin's exposed end first also removed the take grace (R-0049 says a puck just won is
  kept: the exposed end now removes the body's and the sweep's shield only); the all-neutral block of `Jam.Core.MovementNeutral` did not hold
  `spin.exposed_frames` 0.
- **Outside the family, one line each, named in the report**: `JamContactTests.cpp` (`Jam.Core.ContactNeutral`'s block sets `SpinExposedTail = 0`),
  `JamCoreTests.cpp` (`Jam.Core.Feel`: the standing skater reaches 8 m/s in 20..26 frames again; 24 measured), `JamStickHandlingTests.cpp`
  (`Jam.Core.SpinShieldsPuck` pokes through the whole spin: it holds R-0040's shield with `SpinExposedTail = 0`).
- Node tests of the gate (`schema/**`, `tools/**`, `devlog/**`): 210 pass, 1 fail, not this lane's and the same as lanes A102 and A121 saw
  (`tools/barks/barks.test.mjs`: the worktree's `BarkData.inl` has CRLF). Schema tests 99 of 99; the three generators re-run after every schema edit
  (`gen-schema-data`, `gen-parity` 87 cases, `gen-designer-data` 36 cuts). Assets ledger PASS. The ownership step runs git: not run.
- **Revert path**: `"skater": { "burst": { "rest_frames": 0 }, "spin": { "exposed_frames": 0 } }` in a cartridge, or the two defaults in `JamCartridge.h`
  (`BurstRestFrames`, `SpinExposedTail`). Each rule alone: either key. The drill's default: `FJamCarryDrill::BrakeAtM` (9.5), or `jam.ctrl.CarryBrakeAtM 9.5`.

## 7. Drill 21 in the real game mode, and what is queued

`-game -nullrhi -Multiprocess`, hockey.regular at pro, 20 rounds a script, on build 4 (`Game/logs/a128/run-drill.ps1`, logs `Game/logs/a128-drill-*.log`); the
stage line now says `brake_at=8.00 ... rest_frames=6 spin_exposed=5`.

| script | this lane | lane A121 |
|---|---|---|
| 1, the stop-and-go at the drill's default | **20 of 20** (the default is 8 m) | 14 of 20 (the default was 9.5 m: 2 lifted, 4 time-outs) |
| 3, the spin with the cut at 6 m | **10 of 20** (8 lifted, 2 down) | 20 of 20 |
| 4, the deke tap with the cut at 6 m | 12 of 20 (8 down) | 14 of 20 |
| 0, the straight charge | 0 of 20 (20 lifted) | 0 of 20 |

Queued (another lane owns the cursor): a Development package of this tree and the whole acceptance run. Row 9 (deke / spin / sweep: "the puck stays the
human's for 30 frames after") is the row the spin's price could touch: a stick in reach during the spin's last five frames takes the puck. By hand: the
standing start's feel at a faceoff and after a get-up (W from rest), drill 21 script 2 at 8 m, and scripts 3 and 4.

## 8. The 9.5 m edge: what holds a carrier that stopped and kept the puck

Lane A121's stop at 9.5 m passed 95.0 % with no margin: 8 of its 10 failures were time-outs with the puck kept. Under this lane's defaults it is 97.0 %
(5 time-outs, 1 lift). The traces (seeds 9 and 51): the carrier is not held in place. It stops, cuts and goes, and reaches 12 m/s; the pro, which had come
early to a stop that early, turns with it and skates a metre and a half behind its shoulder for 250 frames at the same 12 m/s; its three lifts are refused by
the body between. The drill counts a carry as past when the carrier is 2 m beyond the defender along the attack; in a straight line that never comes (law 4:
a defender is always the faster). **What holds it is the drill's bar and a script with one move**: a second cut, or a stop, ends the footrace. No sim rule is
at fault; the fix is the script's (a second answer when the defender is still on the shoulder 60 frames after the go) or the bar's.

## 9. What the measurements say beyond the brief

- The carry scene's numbers are a property of the brain as much as of the rules: the same rule gives 100 % at pro and 0 % at all_star for the stop at 8 m.
  A level's window should be measured with the level.
- The recovery after a spin does nothing unless the spin was begun early: a price that is paid after the answer is only paid by the too-early answer.
- The deke tap (52.0 from 6 m) and the cut alone (53.0) are now equal to the spin (53.0) from 6 m: three answers of the same worth, each with its own
  failure (the deke and the cut go down to a body check, the spin loses the puck to a lift at its end).
