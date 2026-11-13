# Mechanics pass 4: the last sim calibrations, the drills staged, the acceptance run green (lane A91, device A, R-0044, 2026-11-13)

R-0044 ruled the last calibrations before the sim freezes (Saturday 12:00, R-0043): the full charge under the bar, a human tap that stays flat, the mechanics'
bars restated to what each mechanic is for, and the acceptance run (`ACCEPT.md`) green as the gate for H25. This page is that pass. Status with timestamps:
`status/A91.md`. The eyes tool is out of credits; no strip was cut in this box (section 7). Five packages were made on the way; **package 5
(`Game/Packaged/A91-dev2`) is the final tree** and carries the acceptance verdict; drills 7, 8, 10 and 11 were run on package 4 (`Game/Packaged/A91-dev`), whose
code differs from package 5 only in drill 9's cue.

Code: `Game/Source/JamCore/Public/JamCartridge.h` (`LiftFullM`, `TapFrames`), `Private/JamCartridge.cpp` (the loader), `Private/JamSim.cpp` (`StepCharge`),
`AI/JamAICore.cpp` (`ChargeWindowFor`: one condition, section 3), `schema/` (+ the three generators' output), `Private/Tests/JamLiftedPuckTests.cpp`,
`JamGoalHeightTests.cpp` (two comments), `JamGame/Private/HokejGameMode.cpp` (drills 9, 10, 11; the shield read-out of 7 and 8),
`JamGame/Private/HokejAcceptDrill.cpp` (the tap's band, one rule), `tools/qa/ctrl/a91-*`.

## 1. The full charge: 0.47, not 0.5 (the measurement over the ruling's number)

R-0044 point 1 said `shot.lift.full_m` 0.5, "the top shelf clearly under the bar". Measured headless at the 0.61 net, an empty net, the full charge from every
metre of 3 to 30 m (`Jam.Core.LiftedShot` prints the table on every run; one letter per metre, `G` a goal, `b` the bar):

| `full_m` | 3 .. 30 m | the bar rings from |
|---|---|---|
| 0.60 (main) | `GbbbbbbbbbbbbbbbbbbbbbGGGGGG` | 4 to 24 m: every full charge a player takes |
| 0.50 (the ruling) | `GGGGGGGGGGGGbbbbbbbbbbGGGGGG` | 15 to 24 m: the blue line's range |
| 0.49 | `GGGGGGGGGGGGGGGGGbbbbbGGGGGG` | 20 to 24 m |
| 0.48 | `GGGGGGGGGGGGGGGGGGGbbGGGGGGG` | 22 and 23 m |
| **0.47 (shipped)** | `GGGGGGGGGGGGGGGGGGGGGGGGGGGG` | never |
| 0.46, 0.45 | all `G` | never |

Why: the bar's ring is `BarRingM` 0.05 + `PostRadius` 0.05 = 0.10 m around its axis at 0.61, so a puck at 0.51 m or more is a post. The lift is solved for the
height over the line; the bar is tested when the puck's leading edge reaches the line (one puck radius, 0.12 m, earlier), and a long shot is falling there: from
15 m a shot solved for 0.50 is over 0.51 at the test. Beyond 24 m the puck has landed before the net and slides in. At 0.6 the full charge was never a goal
between 4 and 24 m: A87's red verb, and every full wind-up a player has taken since the 01:40 calibration.

Shipped: **`shot.lift.full_m` 0.47**. `Jam.Core.LiftedShot` asserts it: 728 shots (every hold of 5 to 30 frames from every metre of 3 to 30 m): **0 rang the bar;
the full charge 28 of 28 goals, the half charge 28 of 28**; and the old 0.6 from 10 m is a post and no goal (the fault, kept as a test). If the Director wants the
ruling's 0.5 to the letter it is one number (`FJamShotConfig::LiftFullM`, the schema's description, `DEFAULTS.md`, `README.md`, three expectations in the test),
and the assert's range becomes 3 to 14 m.

## 2. The tap: a hold of 4 frames or fewer (R-0044 point 2)

`game.shot.tap_frames` (new key, integer 0..30, default 4; 0 = every held frame charges, the game before the ruling). `UJamSim::StepCharge`: a release whose
hold counted `tap_frames` or fewer charge frames (and less than the full charge) fires as charge 0: no speed bonus, the tap's height (`tap_m` 0: on the ice the
whole flight). The human and the bots alike (one code path: a bot's short hold is the same release). The one-timer is untouched (`TryShoot` gives it the full
charge whatever the hold). `LiftForCharge` itself is unchanged: the fifth frame is the first charged one (5 / 30: 0.28 m at the line).

Schema: `game.schema.json`, `limits.json` (`shot.tap_frames` 0..30), `DEFAULTS.md`, `README.md`; the three generators re-run (schema-data 3 files, parity 87
cases, designer-data 36 cuts); schema tests 99 / 99.

Test expectations moved, each with its reason in the file (`Jam.Core.LiftedShot`):

| Expectation | Was | Is |
|---|---|---|
| default `full_m` | 0.6 | 0.47 |
| default `tap_frames` | (no key) | 4 |
| `LiftForCharge(1)` | 0.6 | 0.47 |
| `LiftForCharge(19/30)` (midway half to full) | 0.525 | 0.46 = 0.45 + 11/22 of 0.02 |
| the full charge from 12 m, height over the line (a 1.22 net) | 0.6 | 0.47 (measured 0.470, apex 0.57) |
| new: holds of 1, 2, 3, 4 frames | | charge 0, apex 0 (on the ice the whole flight), the tap's speed, a goal |
| new: a hold of 5 frames | | charge 5 / 30, lifted over 0.2 m, faster than the tap |
| new: `tap_frames` 0, a 1-frame hold | | charge 1 / 30, lifted (the old lob) |
| new: the 0.61 net, section 1's table and asserts | | 0 bars in 728 shots; 0.6 from 10 m is a post |

`JamGoalHeightTests.cpp`: two comments say the default is 0.47 now; its scene sets 0.95 itself and its expectations stand.

## 3. What the calibration would have done to the bots, and the one line that stops it

The brain's `ChargeWindowFor` (`JamAICore.cpp`, lane A52) gives every charging bot the FULL wind-up when the full charge clears the bar by 10 cm ("a real net
takes the full charge", written for the 1.22 net). With `full_m` 0.5 or 0.47 that is true at the 0.61 bar too, and pro (10 frames) and all_star (18) would both
hold 30 frames. Measured with the brain untouched (variant 1): `Jam.Bot.ProCharge` mean charge 0.29 -> 0.54. A difficulty change no ruling asked for, while lane
A86 tunes the curve. Shipped: the full wind-up is the bots' shot only at a net of real height (`ChargeRealNetM` 1.0: the sniper mode's 1.22); under a low bar the
profile's wind-up stands, cut to what clears the bar. ProCharge after: 26 shots, 15 charged, mean 0.24 (before: 32, 22, 0.29).

What still moves, for A86: all_star's hold is its profile's 18 frames now (16 before: the 0.6 lift cut it at the bar), and any bot hold of 4 frames or fewer is a
flat tap. The curve-side read-outs in the filter, before -> after: the novice scores inside 7,200 frames at rookie in 11 -> 15 of 20 seeds (need 10); human wins at
all_star 0 -> 1 of 5 seeds on hockey.regular (goal diff -12 -> -9), 0 -> 0 on the large arena (-11 -> -21); rookie 5 of 5 both; take-1 (seeds 1, 2, 7) identical
hashes to before (rookies tap by the edge). The brain's `LiftAtFrames` does not know `tap_frames` (it only bounds the window from above; no effect at the defaults).

## 4. Determinism and the filter

`Jam.Core.Determinism`, seed 1, frame 600, headless:

| Cut | Before (`a91-tests-before-det-ue.log`, main 0f51082) | After (`a91-tests-final-ue.log`, the tree as shipped) |
|---|---|---|
| hockey.regular | `bd698403ed205fbd` | `bd698403ed205fbd` (unchanged; the same in `a91-tests-final2-ue.log`, the final tree) |
| hockey.raw | `0e5637bb401646ae` | `0e5637bb401646ae` (unchanged) |

Both unchanged: the Determinism scene's 600 frames hold no charged release (no hold of 1 to 4 frames, no full charge). The calibration shows in the matches:
`hockey.regular` rookie novice, seeds 1 / 2 / 3 (the bundle test's hashes): `8fc0ae2c5b2082ec` -> `29aefb5ec5d76575`, `b3d422b470400dcd` -> `fc2c2c996f35989a`,
`e24e81398fa44f3b` -> `40d421a117199afa`.

**Full `Jam` filter, the gate's invocation (`tools/qa/ctrl/a91-tests.ps1`): 127 PASS, 0 FAIL, no "missing usage flag" line, before and after** (the same 127
tests; `Game/logs/a91-tests-before-ue.log`; after: `a91-tests-final-ue.log`, and `a91-tests-final2-ue.log` on the final tree, after drill 9's last change). The variants on the way (`a91-tests-v1`, `-v2`, `-v3`): v1 the two calibrations with
`full_m` 0.5 and the brain untouched: 126 / 1 (the new distance block: the bar from 15 m); v2 + the brain's condition: 127 / 0; v3 = v2 with 0.47: 127 / 0.

## 5. The full hold that never reached the line (the brief's job 2): the drill's own staging

Reproduced and found. It is not the bots through a standing carrier and not the carrier's 0.92: **the human never stood.** Drill 10's stage 0 skates to the
pre-spot at 0.8 thrust (8.8 m/s), stage 1 "at 0.6" does not slow the body (Move only stops the acceleration; a skater loses speed by glide drag or the brake),
and stage 2 commanded `Move = 0` with no brake. A diagnostic line (`JAMDRILL stage`, kept; read on drill 11's human, which stands by the same code, in this
lane's third package, where stage 2 already braked) showed stage 2 entered at 6.5 to 9 m/s 0.3 m from the spot and the body out of the 1.5 m ring 14 frames
later even under the brake (about 6 m/s2), the stage back at 0 and the body turning to skate back to the pre-spot: a 246-frame cycle. Without the brake the
glide is longer still. A85's cues fired while the body glided through the ring facing the net. A tap or a half hold was over before it mattered; a 39-frame full hold was released 3 m further on, with the
body already turning for the pre-spot, and `TryShoot` aims along the body: the release went up the ice (A85's two full charges into the side boards 17 and 30 m
back, and "the aim swings round the body").

Fix (the drill only): an arrival controller for stages 0 and 1 (the speed a 3 m/s2 stop from here allows; over it the brake), the brake in stage 2 while the
body moves or the button is held, no stage change during a hold, the cue only under 0.5 m/s. Drill 11's human stands by the same code.

What is not the drill's and stays: a standing carrier in front of three rookies is checked and poked like anyone (section 6: three full charges saved by the
crease skater, one blocked at the blade), and `TryShoot` / `TryPass` still test the distance, not the carrier of record (A85's point 7).

## 6. The five runs on the A91 package against R-0044's bars

Packages (Development): `Game/Packaged/A91-dev` (package 4: BuildCookRun 153 s, staged 949 MB) for drills 7, 8, 10, 11 and the saucer's first run;
`Game/Packaged/A91-dev2` (package 5, the final tree: 85 s, 946 MB) for the saucer's second run and the acceptance run. Both warn above 900 MB and are under the
1 GB limit; the size over a fresh package is the runs' own `Saved` logs inside the staged folder. Driver `tools/qa/ctrl/a91-prove.ps1` (A85's input path; the pass on Q since A84:
A85's driver presses F, which is the ability now; the one-timer's button held over the reception), verdicts `a91-analyze.mjs`, logs `tools/qa/out/ctrl/a91-*`.

| Mechanic | Bar (R-0044) | A85's count | **A91's count** | Read |
|---|---|---|---|---|
| Sweep (drill 7, pro, `a91-sweep`, package 4) | by its shield: every opponent poke and lift inside the window refused | 11 attempts, 8 kept; the shield not counted | **21 cues, 17 sweeps read, 11 attempts; inside the 11-frame windows: 1 refused, 1 clean on the human (not met by the letter: 1 of 2)**; kept 30 frames later 7 of 11 (reported, not a bar); 0 knockdowns | the window is 11 frames and a poke or a lift lands inside it twice in a run. The refused one: defender 3's lift on the sweep's own frame (1270, kind miss). The clean one: defender 3's lift at frame 2353, 8 frames after the sweep at 2345 was read to side 1 and 5 after a second reading to side -1 at 2348 (one cursor crossing read as two sweeps): the shield covers the side the puck left, and that side had just flipped. Over the run the defender's lifts were 10 clean and 1 miss: the lift at contact outside the 11 frames is what takes a blade-length puck |
| Spin (drill 8, pro, `a91-spin`, package 4) | by its shield, the same | 14 spins, 10 with the puck, 2 kept; 9 lift misses in the run | **15 cues, 14 E edges, 14 spins, 6 with the puck; inside the 23-frame windows: 1 refused, 0 clean (met: nothing got through; one event)**; kept 3 of 14 (reported); knocked mid-spin 2 (135 frames each); the run's lifts 10 clean, 4 misses | nothing took the puck through a spin's shield. With the puck 6 of 14 (A85: 10): in 5 v 5 the pro has the puck off the carrier before E arrives on 8 cues (the cue fires at 2.5 m, the lift lands at contact) |
| Saucer (drill 9) | by its flight over a blade when the cue has a blade on the lane, 10 of 10 | 10 holds, 6 saucers, 1 over a blade | **not met: 2 saucers per run, 1 over a blade in the last.** Run 1 (`a91-saucer`, pro cut, package 4, the blade cue without the launch test, 200 s): 5 cues, 5 holds, 2 saucers, 0 over a blade: both taken 0.07 to 0.08 m up, 0.53 and 0.69 m from the blade of an opponent at the passer's shoulder. Run 2 (`a91-saucer-p5`, rookie cut, package 5, + the clear launch, 300 s): 4 cues (a blade 0.22 to 0.42 m from the lane), 4 holds of 8 charge frames, 2 saucers: **one over a blade (0.11 m from the blade at Z 0.349, landed before the receiver, an opponent took it after the landing), one taken 0.18 m up 0.64 m from a blade**; the other 2 releases found no receiver (`flat_receiver=-1 saucer_receiver=-1`, 16 frames after the cue); 50 hands | the cue and the sim's rule pull apart in live play: the cue wants a blade within 0.45 m of the lane, the sim refuses a saucer whose lane has a body within 0.9 m (`SaucerLaneMinM`), and the defender whose blade is on the lane closes that distance inside the 13-frame hold; and a carrier among five live opponents rarely has two clear metres to launch into. The flight over a blade stays proven headless (`Jam.Core.SaucerPass`, green in the filter) and the saucer through the real input in the acceptance run (step 5: the flag, received in 22 frames). The bar of ten needs a set piece: a parked defender 1.2 m off the lane with its blade on it and a standing receiver, the way drill 20 stages its steps; not built in this box |
| Lifted shot (drill 10, rookie, `a91-shots`, package 4) | 10 / 10 / 10 at the line within 8 cm | 30 shots (10 / 10 / 10); taps 9 at the line at 0.09 to 0.20 m (never flat), halfs 7 of 7 within 8 cm, **fulls 0 of 10 at the line** | **30 shots on 35 cues in 150 s, 45 hands: 10 taps, 10 half, 10 full (the count met). Taps: all charge 0.00; 4 at the line, 4 of 4 at Z 0.000. Halfs (0.27 to 0.30): 7 at the line, 7 of 7 within 8 cm (0.434 to 0.450 for 0.45), 4 goals. Fulls (1.00): 5 at the line, 5 of 5 within 8 cm (0.457 to 0.469 for 0.47), 4 goals. At the line 16 of 30, within 8 cm 16 of 16; the bar's "10 / 10 / 10 at the line" not met (4 / 7 / 5)** | the 14 that did not reach the line were stopped by the rookies standing in the lane, not by the shot: taps 6 (five at a blade or a body on the first frame: the flat tap is stoppable, R-0042's point), halfs 3 (one save, two bodies), fulls 5 (three saved by the crease skater 9 frames out, one at a blade, one lost). The full charge from a standing body reaches the line and scores: section 5's fix. No post in 30 shots |
| One-timer (drill 11, rookie, `a91-onetimer`, package 4) | 5 | 1 pass cue, 0 | **13 pass cues (12 the sim's own pass, 1 by the hand), 12 receptions, 12 shots, 9 one-timers (met), 8 goals**; 17 hands | the three that were not one-timers were released 16 to 18 frames after the reception (the driver saw the line late; the window is 10): plain charged shots (0.47, 0.50) and a tap |

**Met: the one-timers (9, bar 5); the spin's shield (0 clean through it); the shots' count 10 / 10 / 10 with every arrival within 8 cm; the full charge at the
line and in the net. Not met by the letter: the sweep's shield (1 clean of 2 events inside the window), the shots' "10 / 10 / 10 at the line" (4 / 7 / 5: defenders
in the lane), the saucer (the row above).** The shield bars rest on one or two events per run: an 11- or 23-frame window is rarely poked. A bar that counts
refusals needs a staged poker (a defender scripted to poke inside the window), which the drill does not have.

The drills' changes (no mechanic touched; `HokejGameMode.cpp`):

| Change | Why |
|---|---|
| Drills 10 and 11: the arrival controller, the brake at the spot, no stage change during a hold, the cue under 0.5 m/s; `JAMDRILL stage` on each stage change | section 5 |
| Drill 11 staged with the hand: the human stands at its spot by drill 10's code; the mate goes to its spot and stands (no chase, no check); the hand gives the mate the puck when both stand; the mate passes after 20 frames of carry: the sim's pass when `BestPass` is the human (`kind=sim`), else the hand sends the puck at the human at the pass speed (`kind=hand`); a human left with the puck for a second gives it back through the hand | A85: 1 pass cue in 120 s. The reception is the sim's own either way (a teammate's puck in flight: `PassReceivedFrame`) |
| `JAMCUE onetimer_recv` on the reception; the driver presses the left button 10 frames before the ETA and releases on that line | A85's 40 ms tap at the ETA charged nothing when the puck was not yet on the blade (a release with 0 charge frames fires nothing) |
| Drill 9's cue: an opponent BLADE within 0.45 m of the lane inside the airborne part (half a metre in from both ends), and a clear launch (no opponent body within 1.3 m, no blade within 0.9 m, of the lane's first 2 m); the hand gives the human the puck after a second without it; the rookie timer cut | A85's cue asked for a body within 1.6 m (1 of 6 over a blade); the first A91 cue without the launch test: 2 saucers, both taken 0.07 to 0.08 m up at the passer's shoulder |
| `JAMDRILL stick_result` gains `shield_frames`, `shield_refused`, `shield_clean` (opponent pokes and lifts within 3.5 m inside the window: kind miss, or clean with the human carrying the frame before) | R-0044's bar for the sweep and the spin |
| Driver `a91-prove.ps1`: Q for the pass (F is the ability since A84), the one-timer's hold, the log tailed by offset; `a91-runs.ps1`; `a91-analyze.mjs` with R-0044's bars | A85's driver cannot pass on this tree |

## 7. The acceptance run (R-0043 point 1): the gate for H25

`tools/qa/accept/accept.ps1 -Package Game\Packaged\A91-dev2 -Name a91-final` on **package 5 = the final tree** (`Game/Packaged/A91-dev2`, Development,
BuildCookRun 85 s, staged 946 MB), no other ThirdJam game running at the start or the end: **exit 0, 18 of 18 verbs PASS, all ten steps green, `warnings` empty**
(`tools/qa/out/accept/a91-final/`).

| step | verb | result | facts |
|---|---|---|---|
| 1 | skate | PASS | top 11.00 m/s, moving 3 frames after the act |
| 1 | sprint | PASS | top 12.65 m/s against 11.00 walking, 114 sprinting frames |
| 1 | sprint_carry | PASS | top 10.12 m/s, 121 frames carried, 0 sprinting frames |
| 2 | aim | PASS | 4 of 4 points followed, error 0.0 to 0.1 deg |
| 3 | carry | PASS | 13 sweeps at full reach, the puck kept, puck at 1.60 m |
| 4 | shoot_tap | PASS | a shot and a goal; 3 charge frames, released as charge 0.00 (flat) |
| 4 | shoot_half | PASS | a shot and a goal; 10 charge frames, charge 0.33 |
| 4 | shoot_full | **PASS** | a shot and a goal; 30 charge frames, charge 1.00; no post (A87's one red verb) |
| 5 | pass | PASS | passed on Q, received in 22 frames |
| 5 | saucer | PASS | the saucer flag, received in 22 frames |
| 6 | onetimer | PASS | 2 of 2 tries: received after 35 frames, the one-timer flag, a goal each |
| 7 | check | PASS | knockdown, strip, down 90 frames (cfg 90), closing 11.11 m/s |
| 7 | crosscheck | PASS | stickhit 2 (lift), the puck free |
| 8 | checked | PASS | down; the view target the human; camera 6.1 m; up after 90 frames |
| 9 | deke | PASS | fired 2 frames after the wheel, the puck kept 30 frames |
| 9 | spin | PASS | fired 2 frames after E, kept |
| 9 | sweep | PASS | fired 3 frames after the cursor crossed, kept |
| 10 | resume_aim | PASS | Esc paused (1.34 s gap), Esc resumed, 2 of 2 points followed |

Run 1 of this lane (package 1: the calibrations, the drill as A87 wrote it; `a91-run1`): 17 of 18. `shoot_full` already green; the red was `shoot_tap`
with **shot=1 goal=1 charge=0.00 charge_frames_max=4**: the click read as 4 frames, the sim fired the flat tap R-0044 rules, and the drill's band still said
"at most 3 frames". The band is the sim's now (`HokejAcceptDrill.cpp`, one rule: at most `game.shot.tap_frames` and released as charge 0; the half band starts
above it; `ACCEPT.md`'s step-4 row says so). A87's file, changed by this lane because R-0044 point 6 gives it the run's green; in the final run the click read
as 3 frames and would have passed the old band too.

No strip was cut: the runs' `jam.ctrl.Stills` sequences fire on the saucer, the sweep and the spin only and the box went to the drills' staging. The proofs
above are the logs' read-outs (each line named), which the strips would only illustrate.

## 8. For the Director

1. **`full_m` is 0.47, not the ruling's 0.5** (section 1): at 0.5 every full charge from 15 to 24 m rings the bar. One number back if the letter is wanted.
2. **The brain's "real net" is a net of 1.0 m or more** (section 3, `JamAICore.cpp`, one condition): without it the calibration gives every charging bot the
   30-frame wind-up. Not in this lane's file list; it keeps the difficulty where A86 is measuring it. all_star's hold is 18 frames now (16 before).
3. **The acceptance drill's tap band is `tap_frames`** (section 7): A87's file, one rule.
4. **The bar's ring reaches 10 cm under its axis** (`BarRingM` 0.05 + `PostRadius` 0.05): the 0.61 mouth scores up to about 0.50 m. Left alone (a collision
   change the day before the freeze); it is why "just under the bar" was never a goal.
5. **The sweep's shield let one lift through in its window** (section 6): the shield is one-sided by A51's design and one cursor crossing was read as two
   sweeps, the second flipping the side; R-0044's words ("every poke and lift inside the window refused") read as both sides. One event; the Director's to rule
   whether the sweep shields both sides, and a staged poker would measure it.
6. **A standing carrier in front of rookies loses half its shots to bodies and the crease skater** (drill 10: 14 of 30 stopped). That is the game, not the shot;
   the strict "10 / 10 / 10 at the line" needs an empty lane (the acceptance run's step 4 is that: 3 of 3 goals).
7. `TryShoot` and `TryPass` still test the distance, not the carrier of record (A85's point 7): untouched.
8. The saucer: see its row; the drill's cue is the limit, not the flight.
