# Movement 3: the carrier's answer inside ten metres (lane A121, track F2, evolution 2, 2026-11-13)

Lane A102 (MOVEMENT-2.md) measured that a stop-and-go beat a charging pro only when the stop began 9.5 m out; at 8 m it was 25.5 %, at 6 m 2.5 %, and the
first-step burst helped the charging defender more than the carrier. Device B counted a stick takeaway 1.4 times a second (B-to-A-0038, R-0049). This lane
adds three rules, each with a neutral value, and the late answers as scripts of the carry drill. Status with timestamps: `status/A121.md`. Raw logs:
`Game/logs/a121*` (not landed).

Code: `Game/Source/JamCore/Private/JamSim.cpp` (`StepSkater`'s burst, `IsPokeShielded`, `ResolvePokes`, `SetPuck`, the possession loop at the end of `Step`,
`FaceoffReset`, `Restart`, `StateHash`), `Public/JamSim.h`, `Public/JamCartridge.h` + `Private/JamCartridge.cpp` (two keys, one default),
`Public/JamCarryDrill.h` (the late answers), `Private/Tests/JamCarrierSkillTests.cpp`, `Private/Tests/JamMovementTests.cpp`,
`JamGame/Private/HokejGameMode.cpp` (drill 21), `schema/`.

## 1. The three rules

| Key | Default | Neutral (the game before the lane) | What |
|---|---|---|---|
| `game.stick.take_grace_frames` | 30 | 0 | a skater that has just won the puck cannot be poked or stick-lifted for this many frames |
| `game.skater.reverse_brake_decel` | 26 | 20 | the stop's grip: 11 m/s to still in 26 frames (33 before), 8 m/s in 19 (24 before) |
| `game.skater.burst.own_stop` | true | false | the first step is earned: being slow arms the burst only after a stop of the skater's own |

Neutral block: `"stick": { "take_grace_frames": 0 }, "skater": { "reverse_brake_decel": 20, "burst": { "own_stop": false } }`.

1. **Take grace** (R-0049 point 2). The puck coming to a skater from somebody else is a puck won: by a poke, a lift, a loose puck, a pass, the draw. For
   `take_grace_frames` frames after it, no poke and no stick lift takes that puck (`UJamSim::InTakeGrace`, read by `IsPokeShielded`, so the lift's strip and, as
   with every shield today, the cross-check's shove follow it). **A body check still loosens it**: contact never reads the grace. A poke whose window runs out
   on a carrier in its grace costs the poker no stumble: nobody can see the grace, so it refuses and does not punish.
   - Who has "just won": `UJamSim::TakeOwner` is the skater that has carried since the puck last changed hands. A carrier's own flicker (a stumble, its own
     rebound, the puck out of reach for a frame) never starts a new grace; the puck going to anybody else and coming back does.
   - A puck placed by `SetPuck` (a test's scene, a drill, the acceptance drill's staging) is not won: its first carrier has no grace. Staged scenes that take a
     puck at once keep working; the rule is about play.
   - State: `FJamSkaterState::TakeGraceLeft` (hashed while it runs) and `UJamSim::TakeOwner` (hashed only when the key is above 0). With 0 neither leaves a trace.
2. **The stop's grip.** `reverse_brake_decel` 20 to 26. Lane A93's stop timings are re-pinned in `Jam.Core.Movement.ReverseBrake` (26 frames from 11 m/s, 19
   from 8 m/s; the value 20 still gives lane A93's 24).
3. **The first step belongs to whoever earned it** (`burst.own_stop`). Lane A93 armed the burst for any skater slower than 2 m/s, so a defender that stood and
   then charged had it for free. Now being slow arms it only after a brake of the skater's own (the reverse brake or the brake button took speed off it and it
   has not been back at 2 m/s since); a sharp cut arms it as before. A skater that merely stands (a waiting defender, the faceoff, a skater getting up) starts
   at the normal acceleration. State: `FJamSkaterState::bBurstEarned`, hashed while set, never set with the key false. This is `docs/VISION.md` law 3's own
   sentence: "after a stop or a sharp cut".

**Chosen by the numbers** (the brief offered this rule or a multiplier of the carrier's own; carry scene, pro, 200 seeds, the stop begun at 8 m; the neutral
block is 25.5 %):

| one rule on, the rest neutral | 8 m | 9.5 m |
|---|---|---|
| take grace only | 25.5 | 100 |
| `reverse_brake_decel` 26 only | 57.0 | 99.5 |
| `burst.own_stop` only | **98.0** | 98.0 |
| the carrier's own multiplier 2.6 only | 47.0 | 100 |
| decel 26 + the carrier's multiplier 2.4 / 3.0 | 76.0 / 86.0 | - / 98.0 |
| decel 26 + own_stop (the defaults) | **100** | 95.0 |

The earned first step alone does more than both alternatives together, needs no asymmetry number and is the vision's wording. The carrier's multiplier stays
in code only (`FJamSkaterConfig::BurstCarryAccelMult`, 0 = off; a key of `Bar.Movement.StopAndGoSweep`).

## 2. The brief's bars (carry scene, the real brain at pro, 200 seeds an arm, `Jam.Core.Movement.StopAndGo`)

| bar | before (the neutral block) | **after** |
|---|---|---|
| a stop begun at 8 m passes a pro at least 55 % | 25.5 | **100** (200 of 200) |
| 9.5 m stays at 95 % or more | 100 | **95.0** (190 of 200: 8 time-outs with the puck kept, 1 lift, 1 stumble) |
| a straight charge stays under 20 % | 0 | **0** (200 lifted) |

By where the stop begins (pro):

| the stop begins at | 5 m | 6 m | 7 m | 8 m | 9 m | 9.5 m | 10 m | 11 m |
|---|---|---|---|---|---|---|---|---|
| before (lane A102's tree = the neutral block) | 0 | 2.5 | 1.0 | 25.5 | 85.5 | 100 | 100 | 91.5 |
| **after** | 0 | 23.5 | 83.5 | **100** | 99.0 | 95.0 | 91.0 | 56.0 |

The window moved about 1.5 m toward the defender and is wider at its late edge: clean from 7 to 10 m (before: 9 to 11 m). A stop begun at 11 m is now too
early (56 %): the carrier stands and the defender arrives at speed. The 9.5 m bar is met with no margin: 8 of the 10 carries that are not past are time-outs
in which the carrier still has the puck (the defender backs off instead of committing and the scripted human never gets 2 m beyond it).
Other rows (after): a looser person (spread 2) at 8 m 87.5; the blade left ahead at 9.5 m 61 (the mouse is still the skill); `body_shield` false at 9.5 m 36.
Rookie: 9.5 m 100, 5 m 93.5, straight 57.5 (as lane A102 found: rookie bots do not press).
**All_star moved most: 9.5 m 5.5 -> 90, 11 m 57.5 -> 94.5, 12 m 91 -> 80.5; 8 m 26; straight 0.** At all_star the free burst was most of the charge.

## 3. The take grace in play (`Jam.Core.Movement.TakeGrace`)

Staged: an opponent lifts a placed puck from the human (it wins it: its grace is 30). The human's lift back, or its poke back, 6 frames later is refused (the
poke costs no stumble); 40 frames later it is clean; with `take_grace_frames` 0 it is clean at once.

In play: hockey.regular at pro, the regular playtester with the brain's bots, 3 seeds of 10 800 frames (9 minutes), penalties off; every clean poke or lift
on a carrier timed from the frame that carrier won the puck. Per 104 s (device B's stretch):

| | take_grace_frames 0 | **30** |
|---|---|---|
| lift presses / clean lifts | 34 / 9.6 | 35 / **2.7** |
| poke presses / clean pokes | 28 / 6.5 | 31 / **2.3** |
| pucks won (the puck changed hands) | 86 | 83 |
| stick takeaways from a carrier, in the 9 minutes | 84 | **26** |
| the shortest hold before a stick takeaway | 1 frame | **31 frames** |
| the median hold before a stick takeaway | 7 frames | **60 frames** |
| stick takeaways inside 30 frames of the win | 63 | **0** |

Device B counted 79 lifts and 68 pokes in 104 s on the build before the vision (stick reactions of six figures); this build presses 34 and 28 times at pro
with the neutral value. Three takeaways in four happened inside half a second of the victim winning the puck (median hold 7 frames): that is the rhythm B
saw. With the grace a carrier keeps the puck at least 31 frames between stick takeaways, in every one of the 26. The presses do not fall: **the brain still
presses on a carrier in its grace** (it does not read `TakeGraceLeft`): 9 in 10 lifts are now refused swings. That is the brain's to learn (section 8).

## 4. The first step, measured (`Jam.Core.Movement.Burst` case d, `.ReverseBrake`)

| from rest to 6 m/s | frames |
|---|---|
| a skater that merely stands (no burst) | 36 |
| after its own reverse stop (the burst) | 18 |
| after the brake button (the burst) | 21 |
| `own_stop` false, standing (lane A93's rule) | 22 |

Stop, cut, go from 11 m/s to 6 m/s the other way: 45 frames (50 with `reverse_brake_decel` 20; 76 on the neutral block of lane A93). **The standing start is
slower for everyone**: 0 to 8 m/s takes 41 frames again (23 with the free burst); `Jam.Core.Feel` is re-pinned to the window it held before lane A93. A
player earns the quick first step with a stop or a cut, which is the vision's sentence; whether the faceoff start now feels heavy is for a person to say
(queued, section 7).

## 5. The late answer inside 7 m (the brief's point 4; `FJamCarryDrill::Late`, `Jam.Core.Movement.LateAnswer`, drill 21 scripts 3 and 4)

A player who reacts late does not stop: it cuts, with the spin (E: the body between for the spin's 24 frames) or the deke tap (the puck to the blade side, the
sweep's shield) pressed on the first frame of the cut. Pro, 200 seeds, cut 60 degrees:

| the answer begins at | 7 m | 6 m | 5 m | 4 m | 6 m, cut 90 | 6 m on the neutral block |
|---|---|---|---|---|---|---|
| the stop-and-go | 83.5 | 23.5 | 0 | | | 2.5 |
| the cut alone | 93.0 | 64.5 | 15.5 | 0.5 | 92.0 | 10.0 |
| **the spin with the cut** | 99.5 | **100** | 82.5 | 26.0 | 100 | 61.5 |
| **the deke tap with the cut** | 65.5 | **56.5** | 45.5 | 20.0 | 84.0 | 39.0 |

The brief's bar (from 6 m one time in three) is met by both with **no key changed**: the windows do nothing at 6 m (`spin.frames` 14 / 24 / 36: 100 each;
`spin.speed_mult` 1: 99.5; `sweep.shield_frames` 12 / 24: 56.5 each). What remains is lost to body checks, which no stick move shields. Late, a stop is the
wrong answer (the spin with a stop at 6 m 21.5; the deke with a stop 0.5), and the deke without the mouse (the blade left ahead) is 12. Inside 4 m nothing
answers more than one time in four. All_star at 8 m: the stop-and-go 26, the cut alone 91.5, the spin with the cut 99.5.
**The spin with the cut is very sure** (100 at 6 m, 82.5 at 5 m): the pro lifts into a spin 188 times in 200 and checks 31 times. It costs a quarter of the
sprint bar and 90 frames of cooldown; whether that is price enough is the next evolution's question (section 8).

## 6. The three cells: the move of this evolution alone (the levels were not tuned)

600 matches a cell, hockey.regular, A53's seeds, both arms on the same build and the same seeds (`tools/perf/results/a102/curve.mjs`, labels `e2-*` in
`cells.jsonl`, raw matches in `raw/`). Before = the neutral block written into the cartridge.

| cell | R-0046 (2,000) | before (neutral block) | **after** | move | goals for : against | the human knocked down a match | the human's clean pokes / lifts a match |
|---|---|---|---|---|---|---|---|
| novice at rookie | 85.3 | 99.8 (99.1-100) | **99.3** (98.3-99.7) | -0.5 | 3.00 : 0.24 -> 2.98 : 0.28 | 2.00 -> 2.29 | 0.5 / 1.4 -> 0.4 / 0.7 |
| regular at pro | 48.3 | 72.0 (68.3-75.4) | **79.0** (75.6-82.1) | **+7.0** | 2.47 : 1.56 -> 2.60 : 1.32 | 7.74 -> 9.56 | 2.8 / 4.1 -> 0.9 / 1.5 |
| good at all_star | 26.3 | 39.8 (36.0-43.8) | **43.2** (39.3-47.2) | +3.4 | 1.82 : 2.26 -> 1.87 : 2.17 | 9.40 -> 10.79 | 4.3 / 4.2 -> 1.4 / 1.4 |

The before column is lane A102's after column in every figure (72.0, 39.8, 99.8, the goals, the carries, the knockdowns): the neutral block gives back the
baseline on 1,800 matches. The ladder was 13 to 23 points off before this lane (R-0050); this evolution adds 7 at pro and 3.4 at all_star and takes half a
point at rookie. Pro is more than 4 points: by R-0047 that is a rule that would ship neutral, and the brief says the curve track re-sets after this lane.

The pro cell with one rule on and the other two neutral (600 matches, the same seeds, `e2-pro-*-only`):

| regular at pro | neutral block | take grace only | decel 26 only | own_stop only | all three |
|---|---|---|---|---|---|
| win % | 72.0 | 77.2 | 75.7 | 70.7 | 79.0 |
| the human knocked down a match | 7.74 | 8.61 | 7.99 | 7.98 | 9.56 |

The grace is the mover in matches (the bots lose the takeaways it refuses and answer with the body: the human is knocked down more, 1.99 -> 2.31 a
minute at pro). The earned first step, the big lever of the carry scene, costs the scripted human a point in matches: its own standing starts are slower, and
the bots reverse-brake about 1,500 times a match, so they earn their bursts. The scene stages the defender standing; a match rarely does.

## 7. Gate, hash, neutral value

- Full `Jam` filter on the final tree (build 3, `Game/logs/a121-tests-jam-full-2-ue.log`): **161 pass, 0 fail** (159 on the baseline + `Jam.Core.Movement.TakeGrace`
  + `Jam.Core.Movement.LateAnswer`). Node tests of the gate: 210 pass, 1 fail that is the worktree's (a CRLF copy of `BarkData.inl`, as lane A102 saw); schema
  tests 99 of 99; assets ledger PASS.
- `Jam.Core.Determinism` (seed 1, frame 600): hockey.regular `d192973ac7c97102` -> `0ff7ea98d5570484`, hockey.raw `b02f67a2da0314b4` -> `8425610943e6dd92`
  under the defaults (the owner is hashed, the burst arms differently).
- **Neutral**: `Jam.Core.MovementNeutral` loads hockey.regular with this lane's neutral block written into its text and replays the baseline's default pro
  match (seed 15, 6000 frames, 2:0): hash `fc59e23598229ebc`, the hash lane A102's final build printed. The three cells' before arms repeat lane A102's after
  arms figure for figure (section 6). Lane A93's and lane A92's neutral tests (main bb18439's two pinned matches) stay green: their blocks carry
  `take_grace_frames` 0 (text in one, the config in the other); `reverse_brake_decel` 0 and `burst.frames` 0 cover the other two rules.
- The lockstep hash: `TakeGraceLeft`, `bBurstEarned` (skater) and `TakeOwner` (sim) are in `UJamSim::StateHash`, which lane A104's `LockstepStateHash`
  includes; that function and lane A110's coverage test are not in this tree. No read-out of an earlier frame feeds the new state.
- Tests re-pinned: `Jam.Core.Movement.ReverseBrake` (the default 26; 26 frames from 11 m/s, 17..22 from 8 m/s; the value 20 gives 24), `.Burst` (cases a and b
  measure the burst with `own_stop` false; case d is the earned step), `.StopAndGo` (the brief's bars; the timing bar is the 5 m stop, and 6 m 40 points under
  the clean stop), `Jam.Core.Feel` (`JamCoreTests.cpp`, outside the family: the standing start 35..45 frames), `Jam.Core.ContactNeutral` (one line: the grace off).

## 8. Drill 21 in the real game mode, and what is queued

Headless, the editor binary as the game on Rink.umap (`-game -nullrhi -Multiprocess`, no window, no cursor, no console port; `Game/logs/a121/run-drill.ps1`),
hockey.regular at pro, 20 rounds a script, build 3:

| `jam.ctrl.CarryScript` | rounds | past | lost | the headless scene (200 seeds) |
|---|---|---|---|---|
| 1, the stop-and-go at its default 9.5 m | 20 | **14** | 2 lifted, 4 time-outs with the puck | 95.0 % |
| 1 with `jam.ctrl.CarryBrakeAtM 8` | 20 | **20** | | 100 % |
| 0, the straight charge | 20 | **0** | 18 lifted, 2 knocked down | 0 % |
| 3, the spin with the cut at 6 m | 20 | **20** | | 100 % |
| 4, the deke tap with the cut at 6 m | 20 | **14** | 4 knocked down, 1 stumble, 1 touch | 56.5 % |

In the real game mode the 9.5 m stop is 14 of 20 (lane A102: 20 of 20): the window moved, 9.5 m is its early edge now and 8 m its centre (20 of 20). The
drill keeps 9.5 m as its default so that lane A102's numbers stay comparable; a hand or packaged proof should set `jam.ctrl.CarryBrakeAtM 8`.

`jam.ctrl.CarryScript` 3 and 4 are new (the spin with the cut and the deke tap with the cut, both at 6 m); `jam.ctrl.CarryBrakeAtM` moves the stop or the cut.

Queued for the cursor's owner (this lane did not hold it):
1. A Development package of this tree and **the whole acceptance run** (`tools/qa/accept/accept.ps1`): the brake row (the stop is 26 m/s^2 now), the carry
   rows, and any row in which a puck is taken by a stick within half a second of the victim winning it (the drill stages with `SetPuck`, which gives no grace,
   so the staged takes should hold; a take that follows a take will not).
2. Drill 21 by hand (`jam.ctrl.CarryScript 2`): is the stop at 8 m learnable with real keys, and **does the standing start feel heavy** (0 to 8 m/s in 41
   frames; 23 with a stop or a cut before it)?
3. `jam.ctrl.Drill 21` with `jam.ctrl.CarryScript` 3 and 4 on the package (the figure's spin and deke takes against a charging pro).

## 9. What the measurements say beyond the brief

1. **The grace is right for the carrier and the brain does not know it.** Lift presses stay at 35 per 104 s and clean lifts fall from 9.6 to 2.7: nine lifts
   in ten are refused swings, and the bots turn to the body (the human is down 9.56 times a pro match, 7.74 before). The brain should hold its stick while
   `TakeGraceLeft` runs and close under control instead (track Q2; lane A115 is on the shield's half of this).
2. **The scene overstates the earned first step.** Staged against a standing defender it is worth 72 points at 8 m; in 600 pro matches it is worth -1.3,
   because the brain's skaters stop themselves all the time. The rule is still the vision's sentence and it is what makes the late answers work
   (the cut alone at 6 m: 10 -> 64.5 %), but the stop-and-go's value in a match depends on how the defender arrives, which is the brain's gap control.
3. **The spin with the cut may be too sure**: 100 % from 6 m, 82.5 % from 5 m at pro, with windows that do not matter. The vision's law 1 says a spin
   lowers stability; the pro checks a spinning carrier 31 times in 200 and lifts into it 188 times.
4. **All_star's stop-and-go is no longer a wall** (9.5 m: 5.5 -> 90 %): its 1.4 speed made the free burst most of its charge. The cell moved 3.4 points.
5. A stop begun too early is punished now (11 m: 91.5 -> 56 %), and the clean window sits at 7 to 10 m. The drill's default stop stays at 9.5 m (95 %); 8 m is
   the centre of the window.
6. `UJamSim::Restart` does not reset `BurstLeft`, `MoveHist`, `MoveHistAt` (lane A93's state); the faceoff countdown that follows hides it (the ring is
   overwritten and the burst runs out in its 20 frames). Left as found: the next lane on this track should reset them with the two fields this lane resets.
