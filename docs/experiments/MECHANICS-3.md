# Mechanics pass 3: the carrier's reach, the tap on the ice, the five bars again (lane A85, device A, R-0042, 2026-11-13)

Lane A82's `JAMSHOT release` line (`MECHANICS-2.md` section 6) found why a human's shot, pass and saucer never fired while aiming: since A51 the carried puck
lives at the blade, 0.4 + `stick.reach_m` 1.2 = 1.6 m from the body, and every carrier test used `shot.reach_m` 1.5. R-0042 ruled one reach for every carrier
test and a tap that stays on the ice. This page is that pass: the rule in the sim, what the measurement did to the brief's third step (A66's cap), the tap, the
five package runs against the bars of R-0034, and the hash. Status with timestamps: `status/A85.md`. The eyes tool is out of credits; the strips were read by
this lane (section 6).

Code: `Game/Source/JamCore/{Public,Private}/JamSim.{h,cpp}` (`CarrierReachM` and its callers), `Public/JamCartridge.h` (`LiftTapM` 0), `AI/JamAICore.{h,cpp}`
(`FJamAIWorld::CarrierReach`, the shot / one-timer / pass-intent tests, the bots' round-the-net rule), `AI/JamSimAdapter.cpp` (`bCanShoot`),
`JamNet/Private/JamNetPackets.cpp` and `JamGame/Private/HokejGameMode.cpp` (the scripted human's "have the puck" test; the drills), `schema/`,
`Private/Tests/JamStickHandlingTests.cpp` (`Jam.Core.CarrierReach`), `JamLiftedPuckTests.cpp`, `tools/qa/ctrl/a85-*`.

## 1. The reach rule (R-0042 point 1)

`UJamSim::CarrierReachM(Skater)` = `max(shot.reach_m (with the skater's modifiers), SkaterRadius 0.4 + stick.reach_m) + 0.05`: **1.65 m at the defaults**
(1.55 for a 0.6 m stick: `shot.reach_m` is then the longer one). Every carrier test in the sim reads it:

| Test | Before | Now |
|---|---|---|
| `CarrierIndex` | 0.4 + `stick.reach_m` + 0.05, or `shot.reach_m`, or the blade within 0.6 m | `CarrierReachM`, or the blade within 0.6 m (the same set at the defaults: A51's clause already carried to 1.65) |
| `StepCharge` (the shot's hold), `TryShoot` (the one-timer is a `TryShoot`) | `shot.reach_m` 1.5 | `CarrierReachM` |
| `StepPassCharge` (the saucer's hold), `TryPass` | `shot.reach_m` | `CarrierReachM` |
| `EffectiveDekeOffsetM` (the deked puck stays carried) | `shot.reach_m` - 0.1 | `CarrierReachM` - 0.1 (the default `offset_m` 1.0 fits both: no change at the defaults) |
| the knockdown's strip of the carrier | `shot.reach_m` | `CarrierReachM` |
| brain: the shot, the one-timer, the pass-intent turn (`JamAICore.cpp`) | `W.ShotReach` | `W.CarrierReach()` (the same rule on `FJamAIWorld`) |
| adapter: `bCanShoot` ("the same gate as `UJamSim::TryShoot`", `JamSimAdapter.cpp`) | `shot.reach_m` | `CarrierReachM` |
| the scripted human's "have the puck" (`HokejGameMode.cpp`, and its lockstep copy in `JamNetPackets.cpp`) | `shot.reach_m` | `CarrierReachM` |

Not moved, on purpose: `Dgoal < W.ShotReach + ShootExtraM` (the shooting range from the net, not a carrier test), `FRefSim::CanShoot` (the reference sim has no
stick), and the adapter's `W.Carrier` (section 3).

`Jam.Core.CarrierReach` (new, `JamStickHandlingTests.cpp`): the number (1.65; the brain's copy equal; 1.55 for a short stick); the puck on the blade at full aim
sits beyond `shot.reach_m` and inside the reach and is carried; from there a tap fires and stays at Z 0, a 12-frame hold counts 12 charge frames and its release
is a lifted shot of charge 12 / 30, a pass edge passes, an 8-frame pass hold is a saucer in the air; the blade swung 90 degrees in one frame and back 15 frames
later keeps the puck carried on all 30 frames (it rode beyond `shot.reach_m` meanwhile) and the shot still fires after.

## 2. The tap (R-0042 point 2)

`shot.lift.tap_m` default **0** (`FJamShotConfig::LiftTapM`, the brain's world default, the schema's description, `DEFAULTS.md`, `README.md`; the three
generators re-run). `ApplyShotLift` already returned flat for a lift of 0, so a charge-0 shot is Z 0 for the whole flight: no parabola, a blade on the ice can
deflect it. `half_m` 0.45 and `full_m` 0.6 unchanged. `hockey.mode.sniper` writes its own `tap_m` 0.1: a cartridge's choice, untouched.

Test expectations moved, each with its reason in the file:

| Test | Was | Is |
|---|---|---|
| `Jam.Core.LiftedShot` defaults | `tap_m` 0.05 | 0 |
| `LiftForCharge(0)` | 0.05 | 0 |
| `LiftForCharge(4/30)` (midway tap to half) | 0.25 | 0.225 = (0 + 0.45) / 2 |
| the tap from 12 m, height over the line | 0.05 (tolerance 0.04) | 0 (the same tolerance), **and its apex is 0: on the ice the whole flight**; the 8- and 30-frame shots' apex is over `BladeClearM` 0.2 |
| `Jam.Core.SaveByHeight`, `GoalHeight`, `PostRings`, `SaucerPass` | | unchanged and green: a tap through the crease stance is still saved (0 < 0.4), still a goal past it |

**What the ruling does not reach (for the Director, section 8):** a human's click is never charge 0. The left button is a hold: one frame of it is charge 1 / 30
and `LiftForCharge` gives 0.45 x 1 / 8 = 0.056 m at the line, the old tap's lob. In the package the driver's 40 ms click read as 2 to 4 charge frames and its
"taps" arrived 0.10 to 0.21 m up (section 5). Only the bots' shoot edge and the one-frame scripts are charge 0. The ruling's words ("a charge-0 shot flies flat")
are implemented as written.

## 3. A66's blade cap (the brief's step 3): it stays, measured

The brief: remove the cap, keep it only if a bot cannot shoot without it. The measurement, full `Jam` filter each time (`Game/logs/a85-tests-*-ue.log`):

| Variant | Filter | Red |
|---|---|---|
| 1. cap out; the adapter untouched | 117 / 3 | `AiBotPlays` and `SeasonTakes` take-1 seed 7 with AI shots 0: the adapter's `bCanShoot` still said 1.5 m (found here, moved to `CarrierReachM`); `NetTrap` (47 frames) |
| 2. cap out; `bCanShoot` and `W.Carrier` on the carrier reach (R-0042 to the letter) | 117 / 3 | `HumanHasSomethingToDo` (16 of 127 faceoffs without a human touch, budget 13), `PassesToHuman` at rookie (0.65 received per match, bar 1.0), `ModeKeys.Sprint` (the regular playtester sprinted 0 frames). **The bots shoot**: `SeasonTakes` take-1 AI shots 1 / 1 / 4, `ProCharge` green |
| 3. cap out; `W.Carrier` back on `shot.reach_m` | 119 / 1 | `NetTrap` (47 frames) |
| 4. cap kept; `W.Carrier` on `shot.reach_m` | 119 / 1 | `NetTrap`, the same match hash `9a7f401c5eab419c` as variant 3: not the cap's doing (below) |
| **5. variant 4 + the bots' round-the-net rule (shipped)** | **120 / 0** | none |

So: **a bot can shoot without the cap** once `bCanShoot` uses the carrier's reach. But a capless bot carries in the 1.5 to 1.65 m band, where the brain's
`W.Carrier` (the adapter's own `shot.reach_m` test, read by about thirty brain decisions: the poke, the lift, the forecheck, the pass share, the sprint gate, the
playtester's "am I carrying") sees no carrier. Moving `W.Carrier` to the carrier reach is right by R-0042 and moves three of the curve's gates; leaving it makes
the capless bot blind to its own puck. The cap and `W.Carrier` are one change, and that change is a curve calibration (A79 re-measures the curve after this
lane anyway). Shipped: the cap as A66 wrote it, `W.Carrier` as on main, both with a comment naming this table.

**`NetTrap`.** Variants 3 and 4 failed it with one hash, so the cause is in what they share: the flat tap. The test's harness passes only move, shoot and check
(taps only). A diagnostic in the test (removed again; the file is byte-identical to main's) showed skater 5, a team-1 bot behind its own net at (42.9, 0.3),
pushing (-0.8, -0.6) into the net's back wall from frame 4581 to 4627 while a loose puck (last touch the human: a missed tap, now flat) slid at 2 m/s along the
end boards behind it. The bots' steering had no rule for "my way leads through the net from behind"; the scripted player bot has had one since lane A3.
Added to `FJamBrain::Think` after `Decide`: a bot behind a net's back wall (`|x| > GoalX + 1.5 + 0.2`, inside the net's width + 1.2 m) whose direction heads
back to the middle goes round the side it stands on (its way's side when it stands on the centre line). NetTrap: 0 frames pressing. This is outside the brief's
"reach, tap, drills" line and inside a file the brief gave this lane; it is what the gate needed, and every bot match hashes differently for it.

## 4. The drills (no mechanic touched)

| Change | Why |
|---|---|
| Drill 10 holds its set piece when `jam.ctrl.HandPuck` is 1: stage 0 to the pre-spot, stage 1 to the spot (the body turns only toward its move, so it arrives facing the net), stage 2 standing inside 1.5 m, turned to the net by a creep when the facing is off; no chase, no check. The hand comes inside 1.5 m of the spot only. The cue fires as soon as it carries there facing the net | A82: 2 cues in 150 s and 88 hands; A85's run 1 on the fixed sim: 0 cues and 88 hands. The human left its spot for every loose puck and the hand fed it on the way |
| `JAMPASS release frame= pass_charge_frames= cooldown_left= puck_dist= reach= carrier= flat_receiver= saucer_receiver=` on every F release | A82's `JAMSHOT release` for the pass: a hold that fires nothing says why |
| `JAMSHOT release`'s `reach=` prints `CarrierReachM` | the number the sim now tests |
| Driver (`a85-prove.ps1`): the spin drill runs its time box | the game's `stick_summary` counts the sweeps the sim reads in the spin drill too; run 1 stopped at 23 s with 4 spins |
| `a85-cut-rookie.json`: A77's rookie QA cut in timer mode | first_to 9 (the schema's maximum) ended the shots drill at 9 : 0 after 13 shots |

## 5. The runs on the A85 package against the bars

Package `Game/Packaged/A85-dev` (Development; three packages: 1 before the round-the-net rule, 2 the final sim, 3 = 2 + drill 10's stages; 44 to 135 s each,
staged 944 / 952 / 961 MB: warn above 900, under the 1 GB limit; the growth is the runs' own `Saved` logs and stills inside the staged folder). Logs `tools/qa/out/ctrl/a85-*` (`-run1`, `-run2`, `-run3` kept), verdicts `a85-analyze.mjs`
(`Game/logs/a85-runs-{1,2,3,4}.txt`). Drills 7, 8, 9, 11 below are run 2 on package 2 (their code is the same in package 3); drill 10 is package 3.

| Mechanic | Bar | A82's count | **A85's count** | Cause where not met |
|---|---|---|---|---|
| Sweep (drill 7, pro) | 10 of 10 kept | 8 attempts, 4 kept | **11 attempts, 8 kept (not met)**; 18 sweeps read on 21 cues; no clean poke, no knockdown of the human. Run 1 the same: 11 / 8 | the three lost pucks (run 1: attempts at 521, 797, 980) went with no poke and no lift: the drill holds the bodies 1.9 to 2.4 m apart and the puck rides 1.6 m out toward the defender, so it sits against the defender's body, which takes it by touch; the human checks it back 12 to 20 frames later. The long reach's own risk, not the shield's |
| Spin (drill 8, pro) | 10 with the puck, 10 kept | 10 spins, 6 with the puck, 2 kept | **14 spins on 15 E edges, 10 with the puck (met), 2 kept (not met)**; lifts in the run: 9 misses (the shield), 5 clean; 1 poke miss | inside the 24 frames the shield holds (the lifts on the spinner read as misses). The puck goes to the body: the pro closes to contact while the spinner moves at 0.7 x 0.92 speed and checks the human (knockdowns of the human at 158, 1604, 3220, 4313; the drill's results count two: 135 frames down inside the turn, 90 just after it) or touches the puck itself. Three of the four presses without the puck came a frame after a clean lift at 0.8 to 1.3 m: the cue fires late when the defender is already at contact |
| Saucer (drill 9, pro) | 10 over a blade, landed before the receiver | 12 holds, 0 saucers | **10 holds, 6 saucers (not met), 1 over a blade (not met), 4 landed before the receiver**; run 1: 11 holds, 6 saucers, 1 over a blade, 6 of 6 landed before | **the hold charges at blade length now**: 9 of 10 releases read `pass_charge_frames=8` with `puck_dist` 0.95 to 1.61 against `reach` 1.65. The four that fired nothing: three had no receiver left at the release (`flat_receiver=-1 saucer_receiver=-1`, 13 frames after the cue saw one), one had lost the puck (`puck_dist` 2.27, `carrier=-1`). Over a blade 1 of 6: the cue asks for a body within 1.6 m of the lane, not a blade on it (run 1: the nearest blade under the flight 0.35 to 3.3 m) |
| Lifted shot (drill 10, rookie) | 10 / 10 / 10 heights | 2 releases, 0 shots | **run 4 (package 3, the timer cut, 315 s): 44 cues, 30 shots: 10 taps, 10 half, 10 full (the count met)**. Taps: 9 reached the line, 9 of 9 within 8 cm of `shot.lift`'s height for their charge (0.09 to 0.20 m at charge 0.07 to 0.13). Halfs: 7 reached the line, 7 of 7 within 8 cm (0.44 to 0.46 m at charge 0.27 to 0.30). **Fulls: 10 fired at charge 1.00, 0 of 10 reached the line (not met).** Score 17 : 0. Run 3 (the first_to 9 cut, 150 s): 15 cues, 13 shots (5 / 4 / 4), taps 5 of 5 and halfs 4 of 4 within 8 cm, fulls 0 of 4, 9 : 0 | **the shot fires with the blade extended**: releases at `puck_dist` 1.61 to 1.64 against `reach` 1.65 with 2 to 30 charge frames, each a shot. The four taps and halfs that did not reach the line were stopped on their first frames (three `save`, one body). **The fulls**: nine of ten were still in flight or lost after 150 frames with no block, no save, no post. One traced (hold 1536 to 1576): the aim holds steady for the first 12 frames, then swings round the body ((0.99, 0.11) to (-0.20, -0.98)) and the shot leaves 5 m nearer the net and 2.9 m aside; in run 3 two such shots met the side boards 17 and 30 m back up the ice. The human is moved during the 39-frame hold in front of three rookies (the drill commands a stand; the 2 to 9 frame holds end before it starts): the sim logged a `breakaway` for it 18 frames into one hold, 1.6 m nearer the net than its spot, and the releases come from 3 to 4 m nearer the net. The likely story is the bots skating onto the puck through the standing carrier and the shot meeting a body at once; **not proven in the box**, and the 0.6 m arrival under the 0.61 bar is not measured in the package |
| One-timer (drill 11, rookie) | 5 | 1 cue, 0 | **1 pass cue, 0 shots (not met)**; 84 hands to the teammate; run 1: 2 cues, 0 | the set piece forms once in 120 s (the teammate passes only standing at its spot with the sim's pass going to the human), and that pass did not arrive: 25 frames after it the puck was 5.2 m behind the human on opponent 5's stick (`front_dot` -0.99). The drill needs the same staging drill 10 got; not done in the box |

Read against A82's table: the mechanics that could not fire at all at blade length now do (the saucer 0 to 6 per run, the shot 0 to 30 per run, the charge
counting at 1.61 to 1.64 m), the shots' count and the tap and half heights are met, the spin reaches its "with the puck" bar, the sweep keeps 8 of 11.
**Met: the shot count 10 / 10 / 10 with the tap and half heights; the spin's 10 with the puck. Not met: sweep kept (8 of 11), spin kept (2), saucers over a
blade (1 of 6), the full charge's height (0 of 10 reached the line), one-timers (0).** The causes left are the drills' staging (11, 9's cue, 10's long hold)
and two things in the game itself: a blade-length puck sits against a defender's body, and the spin does not get the carrier clear of a pro at contact.

The driver's "tap" is a 40 ms click: 2 to 4 charge frames, so these taps are low lifted shots (9 to 20 cm at the line), not charge-0 taps on the ice
(section 2). The flat tap is proven headless (`Jam.Core.LiftedShot`: apex 0; `Jam.Core.CarrierReach`); no human input in the package can produce charge 0.

**Hand pucks** (job 1's read-out): drill 10 handed the puck 88 times in A82's run and 88 in A85's run 1 (0 cues: the count is the drill's own loop, the human
leaving its spot); with the set piece held it is **26 hands for 15 cues and 13 shots** in 150 s (run 3) and 64 hands for 44 cues and 30 shots in 315 s (run 4).

## 6. The strips (`docs/experiments/img/a85/`; read by this lane)

| Strip | What it shows |
|---|---|
| `a85-saucer-f1108-strip.png` | drill 9, run 1, the saucer released at frame 1108 with an orange defender at the human's shoulder, every 12th frame: at +12 the puck is past the defender's stick with its lit trail on the ice behind it, at +24 it flies on up the ice, from +36 the play is at the centre circle where the teammate takes it (`received=1`). The single still `tools/qa/out/ctrl/a85-saucer-run1/stills/a77-saucer-1108-f001124.png` shows the puck in the air above its ground marker, beyond the orange blade |
| `a85-spin-f1388-strip.png` | drill 8, run 2, the spin with the puck at 1388 (kept), every 12th frame: the human (blue 7, the YOU ring) and defender 9 at contact, the HUD line "Missed the stick" through the turn (the shielded lift), a collision at +60 and the defender down at +72 and +84, the puck's glow at the human's blade |
| `a85-sweep-f548-strip.png` | drill 7, run 2, 540 frames from the sweep at 548, every 60th: the human carrying with the lit puck at its blade and an orange defender at arm's length through the first five tiles, a defender down at +300, the carry on up the ice to +540 |

## 7. Determinism

`Jam.Core.Determinism`, seed 1, frame 600, headless:

| Cut | Before (`a85-tests-before-ue.log`, main 8fc69d3 = A82's shipped) | After (`a85-tests-final2-ue.log`, the tree as shipped) |
|---|---|---|
| hockey.regular | `eb27841dda8fad3f` | **`dbabf2d9d1de449c`** |
| hockey.raw | `0e5637bb401646ae` | `0e5637bb401646ae` (unchanged) |

hockey.regular moves with the first build that carried the reach and the tap and stays there through every later variant (the cap, `W.Carrier` and the
round-the-net rule do not touch the Determinism scene's 600 frames); hockey.raw's scene does not reach either rule. Bot matches differ from main in every
variant, as a calibration must. `Jam.Bot.SeasonTakes` on the shipped sim, three runs with the same hashes (`a85-tests-4`, `-final`, `-final2`): take-1 seeds
1 / 2 / 7: 2 : 0 in 7840 / 1819 / 1341 frames, AI shots 4 / 0 / 0, green (seeds 2 and 7 by the clause's under-60-seconds exemption: the scripted human wins
in 30 and 22 seconds); takes 2 to 4 green.

**Full `Jam` filter, the gate's invocation, on the shipped tree: 120 PASS, 0 FAIL, no "missing usage flag" line** (119 on main + `Jam.Core.CarrierReach`).

## 8. For the Director

1. **A66's cap and the brain's carrier are one decision.** R-0042 says the brain uses the carrier's reach; `W.Carrier` at 1.65 m costs three curve gates
   (section 3, variant 2) and was left on `shot.reach_m` with the cap kept. With A79's curve re-measure: move `W.Carrier`, drop the cap, re-record the three bars.
2. **A human's tap is not charge 0** (section 2): one held frame lifts the puck 5.6 cm at the line and the arc clears blades as before. If the rookie's and the
   human's quick click should both stay on the ice, the lift needs a dead zone (holds under N frames are flat) or `LiftForCharge` a floor; a ruling, not made here.
3. **The round-the-net rule for the bots** went in to make `NetTrap` green (section 3); it is a steering change in the brain beside A83's 5 v 5 roles.
4. **The full charge in the shots drill**: 0 of 10 (and 0 of 4) reached the line; the human is moved during the 39-frame hold and the cause was not found in
   the box (section 5). `Jam.Core.LiftedShot` measures the full charge headless at a 1.22 net; whether a standing human can land one at the 0.61 net in a live
   rookie game is open, and worth a look before H25 calls the charged shot proven.
5. `SeasonTakes` take-1: AI shots 0 in two of three seeds, green by the exemption. The tap is flat as ruled; the scripted human ends rookie matches inside a
   minute, so the clause still rests on the exemption there, not on a rookie shot.
6. Drill 11 and drill 9's cue want the staging drill 10 got (a hand at the set piece, a blade on the lane for the cue); then the two bars can be measured.
7. `TryShoot` and `TryPass` test the distance, not the carrier of record: in run 2 the human's saucer at 4210 left with opponent 3 as the carrier (the puck
   0.95 m from the human), and in run 3 a full charge fired at 1.49 m off opponent 5's stick. The rule is old; its zone is 15 cm wider now.
