# Movement: speed change is the carrier's weapon (lane A93, device A, R-0045 laws 3 and 4, 2026-11-13)

`docs/VISION.md` laws 3 and 4: the carrier wins by changing speed and direction; the sprint is a gamble. Before this lane a skater braked only on Ctrl,
accelerated at one rate, the carrier could not sprint (R-0041 point 6, replaced), and the carried puck always rode at the blade's full reach. This page is the
rules as built, the keys, the neutral block that gives back the game of main 8f6b157, the timings before and after, the tests, and what the measurements found
that the brief did not expect. Status with timestamps: `status/A93.md`.

Code: `Game/Source/JamCore/Private/JamSim.cpp` (`StepSkater`, `StepStick`, `ArmSweepShield`, `StepSweepShield`, new `SweepShieldFramesFor`, `StateHash`),
`Public/JamSim.h`, `Public/JamCartridge.h` + `Private/JamCartridge.cpp` (six keys), `schema/`, `Private/Tests/JamMovementTests.cpp` (new),
`JamGame/Private/HokejGameMode.cpp` (`ReadHumanInput`: the held move frame, section 5), `JamGame/Private/HokejAcceptDrill.cpp` (step 1).

## 1. The rules

1. **Braking by reversing** (`StepSkater`). Move input more than 120 degrees from the velocity (the input's direction dot the velocity's direction under -0.5)
   is a hard stop: the whole velocity loses `skater.reverse_brake_decel` 20 m/s^2 (times the input's size, up to 1) until the skater is still. The body keeps
   turning toward the input while it brakes (115 degrees in the 24 frames a stop from 8 m/s takes), so the push the other way starts at once. A skater slower
   than 0.5 m/s whose facing is still more than 120 degrees from the input turns on the spot instead of thrusting away from where it wants to go. Ctrl
   (`brake_decel` 6) is untouched and still works. Read-outs for the figure, the spray and the sound: `FJamSkaterState::bBraking` (either brake, above 1 m/s)
   and `bReverseBraking`; the statistic `ReverseBrakes`.
2. **The first-step burst** (`StepSkater`). While the skater is under 2 m/s, and for `skater.burst.frames` 20 after, the acceleration is
   `skater.burst.accel_mult` 1.8 times normal. A sharp cut arms it too: the move input turned more than 60 degrees inside 10 frames (a ring of the last ten
   input directions; a 90-degree key change arms it, one 45-degree step does not, two inside 10 frames do). The top speed is unchanged. State:
   `FJamSkaterState::BurstLeft`, hashed only while it runs.
3. **The carrier's sprint** (`StepSkater`). Allowed. Top speed and thrust = base x `carry_speed_mult` 0.92 x (1 + `skater.sprint.carry_bonus` 0.08) = 0.9936 of
   base, against a free skater's 1.15: a sprinting defender always catches a sprinting carrier in a straight line. The stamina bar drains as anyone's. Two
   guards make "always" hold for any cartridge: the product is capped at 0.97 x (1 + `sprint.bonus`), and the carrier's sprint does not stack with the
   breakaway burst (the larger of the two applies; 0.92 x 1.2 = 1.104 stays under 1.15). `bSprinting` is now set for the carrier too (A92's stability reads
   it); the statistic `CarrySprintFrames` counts it.
4. **The puck's distance by pace** (`StepStick`). A carrier that is not sprinting holds its blade, and the puck on it, at `stick.carry_reach_mult` 0.75 of the
   blade's distance from the body: 1.2 m of 1.6 m at full aim, 0.75 m of 1.0 m with no aim. Sprinting, the full distance. The direction is always the aim's.
   A skater without the puck keeps the full blade (a poke, a reception reach as far as before). `CarrierReachM` is unchanged (1.65 m): it is the full reach,
   which the live reach never exceeds, so a puck still out at 1.6 m on the frame the sprint ends stays carried while the blade draws it in, and shots and
   passes fire at either distance.
5. **The weaker shield while sprinting** (`ArmSweepShield`, `StepSweepShield`). A sprinting carrier's sweep shield (a sweep or a deke tap) lasts
   `stick.sweep.sprint_shield_mult` 0.5 of `shield_frames`: 6 frames of 12. A shield opened before the sprint is cut to that length when the sprint starts.

6. **The sweep's shield covers both sides** (`IsPokeShielded`; R-0044 as amended 16:35, added to this lane by the Director). Lane A91 measured a clean lift
   5 frames after one cursor crossing: the crossing was read as two sweeps and the second flipped the one-sided shield. While the shield's window runs, a poke
   or a lift from either side of the body is refused; the window's length (`shield_frames` 12) and the sprint's multiplier on it (rule 5) are unchanged;
   `SweepSide` stays as a read-out of the side the puck left. `Jam.Core.SweepShieldsLift`'s third case (a defender on the side the puck went to: it took the
   puck at frame 2 before) now expects the refusal, and `Jam.Core.SweepBeatsPoke` (b) checks a mirrored blade. This is a rule, not a calibration, so it has
   no cartridge key; the one-sided shield of main 8f6b157 survives only as the code switch `FJamStickConfig::bSweepShieldOneSided`, which
   `Jam.Core.MovementNeutral` sets to replay that commit's matches.

## 2. The keys

| Key | Default | Range | Neutral | What |
|---|---|---|---|---|
| `game.skater.reverse_brake_decel` | 20 | 0..40 | 0 | m/s^2 of the stop on a reversed input |
| `game.skater.burst.frames` | 20 | 0..60 | 0 | frames of burst after a stop or a cut |
| `game.skater.burst.accel_mult` | 1.8 | 1..3 | 1 | acceleration multiplier in the burst |
| `game.skater.sprint.carry_bonus` | 0.08 | 0..1 | 0 | the carrier's sprint on top of `carry_speed_mult` (0 = the carrier cannot sprint) |
| `game.stick.carry_reach_mult` | 0.75 | 0.5..1 | 1 | the carried puck's distance when not sprinting, as a share of the blade's full distance |
| `game.stick.sweep.sprint_shield_mult` | 0.5 | 0..1 | 1 | the sweep shield's length while sprinting, as a share of `shield_frames` |

`schema/game.schema.json`, `limits.json`, `README.md`, `DEFAULTS.md`; the three generators re-run (gen-schema-data 3 files, gen-parity, gen-designer-data 36
cuts); `node --test schema/*.test.mjs` 99 / 99. `sprint.bonus` 0.15, `carry_speed_mult` 0.92 and `brake_decel` 6 stay.

## 3. The keys the player presses

| Verb | Keyboard | Gamepad |
|---|---|---|
| Brake | pull back against your motion (W then S), or Ctrl | left stick against the motion, or left stick click |
| Go | any direction from a stop or after a sharp cut: the first third of a second is the burst | the same |
| Sprint | Shift, with or without the puck | left trigger |

The controls cards say so: `SettingsWidget.cpp` (the settings page), `DirectorsCallWidget.cpp` (the pause menu), `InputMapping.cpp` (the binding list).

## 4. The neutral block

```json
"skater": { "reverse_brake_decel": 0, "burst": { "frames": 0, "accel_mult": 1 }, "sprint": { "carry_bonus": 0 } },
"stick": { "carry_reach_mult": 1, "sweep": { "sprint_shield_mult": 1 } }
```

`Jam.Core.MovementNeutral` splices exactly this text into `hockey.regular` and loads it through the cartridge loader (and sets the code switch for the
one-sided sweep shield, rule 6, which has no key), then replays two bot matches of main
8f6b157 (their hashes read from the untouched worktree's run, `Game/logs/a93-tests-before-ue.log`):

| Match | Main 8f6b157 | This tree, neutral block | This tree, defaults |
|---|---|---|---|
| `Jam.Core.NetTrap`'s loop: pro, seed 15, the real brain and the player bot | 3823 frames, 0:3, `42d4c993ba61de9a` | 3823 frames, 0:3, `42d4c993ba61de9a` | 6000 frames, 2:2, `3d104d2c04002bec` |
| `Jam.Core.MatchAlwaysEnds`'s case: all_star, the regular playtester, seed 387 | 11167 frames, 0:3, `fc8d7123c130b1d2` | 11167 frames, 0:3, `fc8d7123c130b1d2` | another match |

The two constants are main 8f6b157's. When A92 and A94 land with their own neutral blocks the test needs those blocks too, or the constants re-read.

Determinism (`Jam.Core.Determinism`, seed 1, frame 600): hockey.regular `bd698403ed205fbd` -> `0581200bdaba6010`, hockey.raw `0e5637bb401646ae` ->
`b02f67a2da0314b4`. The default cartridge uses the new values, so every match moves, as a calibration of every match must.

## 5. The human's "back" under the third-person camera (found by measurement, outside the brief's list)

The human's move input is camera-relative, and the third-person camera follows the skater's facing `camera.lag_frames` 4 late. So a held S was never one
direction on the ice: as the skater turned toward it, the camera turned and "back" ran away (before this lane a held S skated circles). With the reverse brake
that means: the brake drops out part way through the stop and the skater then turns on the spot. `Jam.Core.Movement.BackUnderFollowCamera` models it with the
camera's own filter (yaw += (facing - yaw) x (1 - e^(-1/4)) per frame), W for 2.5 s and then S for 2 s:

| Move frame while S is held | Brake frames | Still after | Then |
|---|---|---|---|
| the live camera (no fix), this lane's rules | 20 | 45 frames | turns on the spot: 0.01 m/s, never goes back |
| the live camera, main's rules (neutral block) | 0 | never (a circle) | still circling at 2.6 m/s after 2 s, 6 m further on |
| **held at the press (what the game mode now does)** | 30 | **33 frames** | back the way it came at up to 11.0 m/s, 10.2 m back after 2 s |

The fix is 12 lines in `AHokejGameMode::ReadHumanInput` (and two members in `HokejGameMode.h`): while the move axis pulls back (Y under -0.3), the move input
is read in the camera's frame of the moment the pull began; let go, the live camera again. S is then one direction until released: the skater brakes, turns,
skates back that way and the camera comes round behind it; W after that goes on in the same direction. The stick aim keeps the live camera. The arena camera
(fixed yaw) is not affected.

## 6. Timings, before and after

`Jam.Core.Movement.ReverseBrake` and `.Burst` print them (`TIMING` lines). hockey.regular skates 11 m/s (the schema's base is 8).

| Measure | Before (neutral block = main) | Brake only | Burst only | After (defaults) |
|---|---|---|---|---|
| Free skater, full speed (11.0 m/s) one way to 6 m/s the other way | **76 frames (1.27 s)** | 64 | 76 | **50 frames (0.83 s)** |
| Carrier, full speed (10.12 m/s) one way to 6 m/s the other way | 79 frames (1.32 s) | 65 | 79 | 50 frames (0.83 s) |
| Full speed to no forward motion | 46 frames (a U-turn, never still) | 33, still | 46 | 33 frames, still |
| Metres travelled past the point of the reversal | 4.81 | 2.93 | 5.11 | 2.93 |
| 8 m/s (`skater.speed` 8) to still | a U-turn | 24 frames | - | **24 frames (0.40 s)** |
| 0 to top speed 11 m/s | 56 frames | - | 35 | 35 frames |
| 0 to 8 m/s (`Jam.Core.Feel`) | 41 frames | - | 23 | 23 frames |
| Standing, input straight behind: 6 m/s that way | - | - | - | 42 frames, 0.065 m the wrong way first |

The carrier's sprint (`Jam.Core.Movement.CarrierSprint`): carrier 10.118 m/s walking, 10.927 sprinting (0.9934 of base); defender 10.998 and 12.648 (1.1498);
a sprinting defender 8 m behind a sprinting carrier is 5.02 m behind after 2 s; both bars at 21600 of 64800 ticks. With `carry_bonus` 1 and
`carry_speed_mult` 1 the cap holds the carrier at 12.268 against 12.648.

The puck's distance at full aim (`.PuckDistance`): 1.200 m standing, 1.229 m skating, 1.632 m sprinting, 1.232 m after the sprint; carried on 120 of 120
frames through both changes; a skater without the puck holds a 1.600 m blade; a shot and a pass fire at both distances.

The shield (`.SprintShield`): 12 frames skating, 6 sprinting; a stick on the old side is refused on 11 frames after the sweep against a skating carrier, 5
against a sprinting one.

## 7. A bot match at pro, three minutes (`Jam.Core.Movement.BotMatchPro`)

hockey.regular, the real brain at pro on every bot, the scripted player bot in the human's slot, 10,800 frames (the final tree, the shield on both sides).

| Run | Carry frames skating | Carry frames sprinting | Sticks that took the puck from a skating carrier | from a sprinting carrier | Reverse brakes |
|---|---|---|---|---|---|
| As the brain plays it (`ai.sprint` 0; the brain sprints only on a chase) | 5944 | **0** | 179 (30.11 per 1000 carry frames) | 0 | 1121 |
| Every carrier holds the sprint | 1357 | **6085** | 0 | 116 (**19.06 per 1000**) | 936 |

(The same two runs before the shield covered both sides: 7524 skating frames and 150 taken, 19.94 per 1000; 5713 sprinting frames and 114 taken, 19.95 per
1000. The two runs are different matches, so a rate moves by a third between builds on its own.)

Two things the numbers say. The brain never sprints with the puck, so law 4's gamble is the human's alone until A94 teaches the carrier when to sprint. And
the bots do not take a sprinting carrier's puck more often than a skating one's (19.06 against 30.11 per 1000 carry frames on the final tree, 19.95 against
19.94 before the shield change): the farther puck and the shorter shield change nothing against bots that do not time their pokes on a sweep. "Easier for a
stick to take" is true for the rule (6 frames of shield instead of 12, the puck 0.4 m farther from the body) and not shown in the outcome. The lever is the
brain's (a poke at a sprinting carrier's puck) or the contact's (A92: a sprinting skater is easier to knock off the puck).

## 8. The curve: two bars are red on the defaults, and which rule moves them

`Bar.Movement.CurveSensitivity` (outside the gate's `Jam` filter) plays the two novice-at-rookie bars, 20 seeds each, rule by rule:

| Rules on | `Jam.Playtest.NoviceScoresAtRookie` (scores inside 7,200 frames; bar 10 of 20) | `Jam.Bot.HumanHasSomethingToDo` (faceoffs without a human touch; budget 10.4 %) | Goals |
|---|---|---|---|
| none (neutral = main) | 11 | 6 of 112 (budget 11) | 59:12 |
| reverse brake only | 15 | **21 of 151 (budget 15): fails** | 58:19 |
| burst only | 14 | 10 of 132 (budget 13) | 57:22 |
| carrier sprint + shield only | 11 | 6 of 112 (no change: nobody sprints with the puck) | 59:12 |
| carry reach 0.75 only | 15 | 10 of 133 (budget 13) | 59:20 |
| all (the defaults), shield on one side | **6: fails** | **21 of 160 (budget 16): fails** | 56:19 |
| all (the defaults), shield on both sides (the final tree, the gate's own two tests) | **7: fails** | **21 of 174 (budget 18): fails** | - |

Each rule alone leaves the novice scoring more often; together they do not. The bots brake and burst with perfect inputs (980 reverse brakes in three
minutes), so the rookies reach loose pucks sooner than the novice does. These are curve gates and this lane did not tune them (R-0045: the levels are tuned
once after the three lanes land). If the Director wants the gate green before then, the two cheapest neutral-compatible options are `reverse_brake_decel` at a
lower value for the measured pass, or the bots' use of it (the brain holding its line instead of reversing: A94).

## 9. Tests

`Jam.Core.Movement.*` (`JamMovementTests.cpp`): `ReverseBrake` (the stopping time, the read-outs, Ctrl unchanged, the pivot), `Burst` (the window, the
multiplier, the cut, the same top speed), `CarrierSprint` (slower than a defender, the bar, the cap, `carry_bonus` 0), `PuckDistance`, `SprintShield`,
`BackUnderFollowCamera`, `BotMatchPro`; `Jam.Core.MovementNeutral`. Changed for rule 6: `Jam.Core.SweepShieldsLift`, `Jam.Core.SweepBeatsPoke`.

Earlier lanes' tests whose numbers moved by this ruling, each change:
- `Jam.Core.CarrierSpeed` (A82): "sprint held while carrying changes nothing, the bar full" became "adds `carry_bonus`, slower than the free sprint, the bar
  drains as the free skater's"; A82's two assertions are kept under `carry_bonus` 0. The run is 150 frames, not 180: with the burst both skaters are past the
  red line at frame 180 and the carrier's breakaway burst is running (11.235 against 10.998 m/s).
- `Jam.Core.CarrierReach` (A85): its standing scenes test the seam at the blade's full distance, which is now the sprinting carrier's; they run with
  `carry_reach_mult` 1. A check at the defaults is added: the standing carrier's puck at 1.2 m, carried, a tap fires.
- `Jam.Core.Feel`: 0 to 8 m/s in 35..45 frames became 20..27 (41 -> 23 measured).
- `Jam.Core.LiveConfig`: the speed is read after 90 frames, not 120: with the burst the human is at 12 m/s after 42 frames and has skated into the opposing
  centre by frame 114 (4.8 m/s at the bump).
A51's tests (`SweepBeatsPoke`, `SpinShieldsPuck`, `SpinCooldown`, `SpinCheckedFalls`, `StickHandlingKeys`) and A82's lift shields pass unchanged.

Full `Jam` filter: 132 / 132 before; after, **138 pass and 2 fail** (`Game/logs/a93-tests-after-ue.log`): the two curve bars of section 8.

## 10. The acceptance run (`tools/qa/accept/accept.ps1`, the Development package of this tree, `Game/Packaged/A93-dev`)

Step 1 is re-staged and has a fourth verb. `sprint_carry` now passes when the carrier's sprint is live and lifts it above the carrier's walk (free walk x
`carry_speed_mult` + 0.2) and stays 0.2 under the free sprint. `brake`: W for 2 s, then S (the drill sends the S line once W is released, the driver's key
table gained S): pass when the skater is under 0.3 m/s inside 40 frames of the S input reaching the sim and then moves back at more than 1 m/s.

Two full runs (`tools/qa/out/accept/a93-run1`, with another lane's window up at the end, and `a93-run2`, clean): the same verdicts, **17 of 19 verbs**.

| step | verb | result | facts (run 2) |
|---|---|---|---|
| 1 | skate | PASS | top 11.00 m/s, moving 3 frames after the act line |
| 1 | sprint | PASS | top 12.65 m/s against 11.00, 121 sprinting frames |
| 1 | sprint_carry | PASS | top 10.93 m/s with the puck: above the carrier's walk 10.12, under the free sprint 12.65; 119 carrier sprint frames of 121 carried |
| 1 | brake | PASS | S against 10.66 m/s: still in 32 frames (30 brake frames), then back at 11.00 m/s |
| 2, 3, 5, 6, 8, 9, 10 | aim, carry, pass, saucer, onetimer, checked, deke, spin, sweep, resume_aim | PASS | the puck rides at 1.20 m in every one (`puck_dist_at_act=1.20`) and every verb fires |
| 4 | shoot_tap, shoot_half | PASS | goals at 3 and 9 charge frames from 1.20 m |
| 4 | shoot_full | **FAIL** (A91's row) | the full charge fires and misses the net at this tree's `shot.lift.full_m`; A91's 0.47 (R-0044 as amended) is on main, where the row is green |
| 7 | check | PASS | knockdown, strip, 90 frames down; closing 13.24 m/s |
| 7 | crosscheck | **FAIL** (the contact lane's row, moved by this lane's rules) | see below |

`crosscheck` (MMB, a stick lift in this tree) was probed alone (`a93-x7-*`): red 3 of 3 on the default cut, green with `burst.frames` 0
(`tools/qa/accept/a93-cut-noburst.json`) and green with `carry_reach_mult` 1 (`a93-cut-reach1.json`). Two things happen at this lane's pace. With the burst the
human closes at 13.2 m/s instead of 10.9, and the drill cues MMB with the body check's lead (gap under closing x 6 frames, 1.27 m here) although the lift is
instant with a 1.0 m range: pressed two frames after the cue it is 5 cm out of range (`stickhit=0`, a lift miss in the log). In the full run the lift lands
(`stickhit=2`) and the puck is the opponent's again 20 frames later: the lift sends the puck with half the lifter's velocity, into a carrier whose puck now
rides 0.25 m nearer its body. A92's tree re-rules the cross-check as a thrust (its row reports a knockdown), so the row is the contact lane's to restage after
the merge; this lane did not edit step 7 or `TryLift`.

## 11. What the player will feel

Skating used to be a car: a wide turn to come back, one gear, and with the puck no gas pedal at all. Now the feet answer. Pull back against your motion and
the skater digs in and is standing still in half a second, three metres on instead of five; push off in any direction and the first third of a second has
almost double the bite, so stop, cut, go gets you moving the other way at speed in 0.8 s instead of 1.3 s. With the puck you can sprint, and it is a bet: you
gain eight percent, any chaser who sprints still gains on you, the puck slides out to the end of the stick, a metre and a half away from your body, and a
sweep protects it for half as long. Walk the puck and it sits tucked in close; run it and it is out there for the taking. The way past a defender is no
longer straight ahead: it is the stop he did not expect and the first step he could not match.
