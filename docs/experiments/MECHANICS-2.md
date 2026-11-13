# Mechanics pass 2: the lift shielded, the saucer's lane, the drills that hand the puck, the carrier slowed (lane A82, device A, R-0040 / R-0041, 2026-11-13)

Lane A77 (`MECHANICS.md`) proved the input path and the read-outs in a package and found three things the Director then ruled on (R-0040): the stick lift at
contact took the puck inside the sweep's and the spin's shield windows; the saucer's lane margins were too tight for live bots; drills 10 and 11 never handed the
puck to the human. This page is that pass: the three fixes in the sim and the drills, one more calibration from the operator (R-0041 point 6: the puck carrier is
slower and cannot sprint), the cause of the `Jam.Bot.SeasonTakes` failure A77's tree carried, and the five package runs against the bars. Status with
timestamps: `status/A82.md`. The eyes tool is out of credits (402); the strips were read by this lane, section 6 says what it saw.

Code: `Game/Source/JamCore/{Public,Private}/JamSim.{h,cpp}` (`TryLift` shield, `IsLiftShielded`, `SaucerLaneMinM`, `SaucerAirFromM`, `StepSkater` carry), `JamCartridge.{h,cpp}`
(`skater.carry_speed_mult`), `schema/`, `Private/Tests/JamStickHandlingTests.cpp` (`Jam.Core.SpinShieldsLift`, `SweepShieldsLift`, `CarrierSpeed`), `JamSaucerTests.cpp`,
`JamGame/Private/HokejGameMode.cpp` (`jam.ctrl.HandPuck`, the `JAMSHOT release` line, the sweep capture on drill 7 only, the drill-9 cue on the sim's numbers),
`tools/qa/ctrl/a82-prove.ps1`, `a82-analyze.mjs`.

## 1. Job 0: why `Jam.Bot.SeasonTakes` failed on A77's tree, and what was (not) changed

The failure: `take-1 (The Read-Through) seed 2: 2:0 in 4427 frames, AI shots 0` (rookie, first_to 2; the clause: the AI shoots at least once in a match of a minute
or more; seeds 1 / 7 had AI shots 0 / 1 and passed by being under 60 s or by the one shot). `Game/logs/a82-tests-before.log` reproduces it on the untouched worktree.

Bisected by compile (`a82-tests-bisect-1.log`): with **only the blade-clear rule reverted** (`Puck.Z < BladeClearM` on the deflection, the body reception and the poke)
and every other A77 change kept (the pass hold, `StepPassCharge`, `PickReceiver(bSaucer)`, the brain's saucer hold, the playtester's), take-1 passes on all three seeds
with AI shots 3 / 2 / 4 (and the matches run 2731 / 4885 / 13862 frames). So the brief's first suspects are cleared: at rookie `ShotChargeFrames` is 0, the brain never
holds pass, `bPassHeld` is never set for a bot, and a bot's pass edge goes through `TryPass(S, false)` exactly as before A77.

What the blade-clear rule does to a rookie match: A52's lift solve (`ApplyShotLift`) makes **every** shot a parabola that comes down to `shot.lift`'s height at the goal
line, so a **tap** from 10 m (target 0.05 m at the line) is about 0.5 m up at mid-flight (VZ 3.3 m/s; `LIFTED-PUCK.md`). Since A77 a puck over 0.2 m passes every blade.
Together: a shot can no longer be deflected by a stick between the shooter and the net for most of its flight; only a body (1.8 m) or the crease stance (0.4 m, near
the line where the puck is low again) stops it. The scripted human's shots go in sooner (seed 7: 13862 -> 2590 frames), and the rookie bots, who shoot about once a
minute (3 in 45 s, 2 in 81 s, 4 in 231 s on the old rule; 0 in 21 s, 0 in 74 s, 1 in 43 s on the new), got no shot off before 2:0 in seed 2: 74 s, over the clause's
60-second exemption. At one shot a minute the chance of none in 74 s is about 30 %: the clause is a coin at rookie, and A77's rule re-rolled the seeds.

**Not changed for this.** The rule is R-0040's and physically right (a puck half a metre up passes a blade on the ice); reverting it, or exempting shots from it, would be
tuning past the ruling, and changing the rookie's shot rate is the curve (A79 / R-0039). The test clause is lane A17's and the Director's. Three ways out, for the
Director (section 8): (a) the clause's floor follows the rookie's measured rate (matches of two minutes or more, or AI shots or a human goal count as "the AI played"); (b) a
tap that stays low: R-0035 says "tap on the ice", A52 made it 0.05 m *at the line*, which the solve turns into a 0.5 m lob that now clears sticks; a tap capped under
`BladeClearM` the whole way would make rookie matches (taps only) hash as before A77 but needs A52's `LiftedShot` expectation changed; (c) accept the coin and re-seed.
On the finished tree `Jam.Bot.SeasonTakes` is green again (take-1 seeds 1 / 2 / 7: 2:0 in 1990 / 2229 / 3645 frames, AI shots 1 / 0 / 1): `carry_speed_mult`
re-rolled the match and seed 2 now ends under the clause's 60 s. The same coin, landing the other way; the structural finding stands and the Director's choice below is
still asked. The whole filter is 119 of 119 (`Game/logs/a82-tests-final.log`).

## 2. The lift shield (R-0040 point 1)

| Piece | Rule |
|---|---|
| `TryLift` | after the range test (1.7 m body to body, `stick.lift_range_m` 1.0 + two bodies) and before the take: `IsLiftShielded(S, Carrier)` = `IsPokeShielded(S, Carrier)`: a spinning carrier (`SpinFrames > 0`) shields fully (the body is between); a sweeping carrier (`SweepFrames > 0`) shields the side the puck left (the lifter's blade on that side of the carrier's forward line) for `stick.sweep.shield_frames`. A shielded lift is spent (the 30-frame cooldown stands) and emits `lift` kind `miss`; the carrier's `StickHitResult` is not set (no `PlayStickHit`) |
| The frame of the sweep | the lift runs in the input loop before `StepStick` sets the shield, so a lift on the very frame of the crossing still takes (the poke, resolved after the puck step, is shielded on that frame). One frame; the tests lift from the frame after |
| Tests | `Jam.Core.SpinShieldsLift` (a defender 1.2 m beside the carrier lifts on every allowed frame: with the spin no lift takes for 24 frames and the first reads as a miss; without it the first lift takes on frame 1), `Jam.Core.SweepShieldsLift` (defender at lift range on the left, the sweep left to right on frame 1, lifts from frame 2: shield 12 keeps the puck through the shield and every lift misses; shield 0 loses it on frame 2; a defender on the side the puck went to takes it on frame 2) |

## 3. The saucer's lane (R-0040 point 2)

`SaucerLaneMinM` 0.55 -> **0.9**, the airborne window from **`SaucerAirFromM` 1.5 m** along the lane (was 0.5; the saucer leaves at 14 m/s and is 0.2 m up after 1.3 m, so a
blade nearer than that took it: A77's "taken at 8 cm"). The drill-9 cue in `HokejGameMode.cpp` reads `UJamSim::SaucerAirFromM` (it had its own 0.5); the brain and the
playtester use `BestPass(..., true)`, so they carry the numbers by construction.

**What 0.9 means** (found while moving the test): the flat pass refuses a lane whose nearest opponent **body** is within 0.9 m of the line ("a stick in the lane"), and
the saucer requires that body to be at least 0.9 m off. The two sets are complementary: **at 0.9 the saucer's receiver set equals the flat pass's**; the saucer no longer
rescues a receiver the flat rule refuses. What it buys is the flight: a body 0.9-1.6 m off the line reaches its blade onto the line, and the flat pass is deflected by it
(`StepPuck`'s deflection) while the saucer flies over at 0.35 m. `Jam.Core.SaucerPass` (a) now proves exactly that: body 1.0 m off, blade on the line: the flat rule
allows the lane and the flat pass does not arrive clean; the saucer is picked, clears the blade (Z >= 0.2 at the defender), lands before the receiver, is received; a
body 0.8 m off refuses the saucer too. Consequence for the brain (`JamAICore.cpp`, not this lane's): its trigger `CarrierBestPass < 0 && CarrierBestSaucer >= 0` can no
longer be true, so **pro bots never saucer**; the playtester's trigger (`Corridor` blocked with a saucer receiver) still can. A brain that saucers when the flat lane has
a blade in it (a body 0.9-1.6 m off the line) is one condition in `Think`; left for the Director.

## 4. The carrier is slower (R-0041 point 6, the coordinator's add)

| key | default | range | what |
|---|---|---|---|
| `game.skater.carry_speed_mult` | 0.92 | 0.6..1 | the puck carrier's top speed and acceleration as a fraction of a free skater's, bots and the human alike (`StepSkater`: `CarryMul` on `Speed` and `Accel`, on top of the breakaway burst); the sprint is refused while carrying and the bar does not drain; 1 = no penalty |

The carrier is `CarrierIndex() == S.Index` as the puck step before this frame left it (deterministic, hashed through the positions). `Jam.Core.CarrierSpeed`: a carrier
and a free skater at full input from rest for 180 frames run at 0.92 within 0.01; sprint held while carrying changes nothing (the same speed to 1e-6, the bar full) while
the free skater's sprint is live; `carry_speed_mult` 1 is no penalty; 0.8 reads back; 0.3 is refused. Schema, `limits.json`, `DEFAULTS.md`, `README.md`; gen-schema-data,
gen-parity, gen-designer-data re-run (3 files / PASS / 36 cuts); schema node tests 98 / 98. **This is a calibration of every match**: the hash moves (section 7).

## 5. The drills (job 3)

- `jam.ctrl.HandPuck 1` (drills 10 and 11): the shots drill puts the puck on the human's blade whenever it is not carrying and no shot of its is being measured; the
  one-timer drill puts it on the scripted teammate's blade whenever nobody on the human's team carries, no pass of the teammate's is in flight (90 frames) and no shot is
  being measured; `JAMDRILL hand_puck frame= to= carrier_was=`; 60 frames between hands. A drill change, not a mechanic one (R-0040 point 4: the bars are measured on the
  real input path; the puck's origin is the drill's, as in the tests).
- `JAMSHOT release frame= charge_frames= shot_ready_frame= cooldown_left= puck_dist= reach= front_dot= carrier=` on every left-button release of the human, as the sim will
  see it that step: the answer to A77's two taps that fired nothing (section 6).
- The sweep still capture (150 stills, once) only in drill 7: A77's spin runs spent it on a cursor crossing.
- `a82-prove.ps1`: A77's driver with `HandPuck`, the tap held 40 ms (two frames, so one sim frame surely reads the level; charge 2/30 is still a tap), lane A82.

## 6. The five runs on the A82 package against the bars

Package `Game/Packaged/A82-dev` (Development), logs `tools/qa/out/ctrl/a82-*/game.log`, verdicts `a82-analyze.mjs` (`Game/logs/a82-runs.txt`). The real input path
is proven again on every run (the cursor `src=mouse` 3450-9323 frames, E `btn=spin` 11, F `btn=pass held=1` 12, the left button 3).

| mechanic | the bar | A77's count | **A82's count** | the cause where not met |
|---|---|---|---|---|
| Sweep (drill 7, pro, 60 s) | 10 of 10 kept | 10 attempts, 3 kept, 1 poked, 1 knocked; 19 clean lifts | **21 cues, 9 sweeps read, 8 attempts, 4 kept, 0 poked clean, 0 knocked (not met)**; lifts: 15 clean, **3 misses** (the shield: two lifts at 2031 right after the 2028 sweep read as misses) | attempt 2 lost to a clean lift at 1495, 24 frames after the 1471 sweep, **after the 12-frame shield ended**; the other three lost with no poke, no lift and no check inside 30 frames: the puck came off the blade in the crossing itself (the drill holds the carrier 1.9-2.4 m off a pro that closes; strip below: along the boards) |
| Spin (drill 8, pro, 90 s) | 10 with the puck, 10 kept | 8 spins, 5 with the puck, 1 kept | **15 cues, 11 E edges, 10 spins, 6 with the puck (not met), 2 kept (not met), 1 poked clean, 0 checked mid-turn; lifts 12 clean, 4 misses** | the shield holds inside the 24 frames (the misses); the puck goes after the spin ends to the lift of the defender the body turned away from, now at contact; the human spins at 0.7 speed and 0.92 of that carrying, so it does not skate clear |
| Saucer (drill 9, pro, 150 s) | 10 over a blade, landed before the receiver | 24 cues, 21 holds, 4 saucers, 1 over | **12 cues, 12 F holds arrived, 0 saucers, 0 passes (not met)** | the release fired nothing: `puck_dist` 1.61-1.63 m > `reach` 1.50 (below); the cue fires on the saucer receiver, the hold never charges because the puck is out of pass reach |
| Lifted shot (drill 10, rookie, 150 s) | 10 / 10 / 10 heights | 2 cues, 0 shots | **88 hands, 2 cues, 2 releases, 0 shots (not met)**; `JAMSHOT release frame=3455 charge_frames=0 shot_ready_frame=0 cooldown_left=0 puck_dist=1.63 reach=1.50 front_dot=0.98 carrier=0` (and 5426: 1.61) | **found**: the carried puck sits at the blade, 0.4 + `stick.reach_m` 1.2 = 1.6 m from the body with the cursor at the drill's home, beyond `shot.reach_m` 1.5; `StepCharge` never counts (not in reach), `TryShoot` refuses. A51 saw the seam ("the puck at the full reach sat exactly at shot.reach_m"), A66 capped the bots' blade for it; the human has no cap. The puck handed 88 times says the human at its spot kept losing it the same way (a blade-length puck is a loose puck to a bot) |
| One-timer (drill 11, rookie, 120 s) | 5 | 0 cues | **77 hands, 1 pass cue, 1 release (`puck_dist` 3.15, `carrier=-1`: the pass had not arrived), 0 shots (not met)** | the set piece forms once in 120 s even with the hand: the teammate passes only when `BestPass == human`, and the human at its spot holds the blade a metre and a half out; the same reach seam |

**The one fix left for A (not made here, out of the box)**: for the carrier, the shot's, the pass's and the saucer's reach tests should cover the stick's full reach
(`max(shot.reach_m, SkaterRadius + stick.reach_m)` in `StepCharge`, `TryShoot`, `StepPassCharge`, `TryPass`), or `stick.reach_m` stays under `shot.reach_m - 0.4`;
either is a calibration (the hash moves), and then drills 9-11 can be re-run as they stand. The driver's cursor home (the client centre) is the hand's natural place;
moving it nearer the body would hide the seam, not fix it.

## 6. The strips (`docs/experiments/img/a82/`; read by this lane, the eyes tool is out of credits)

| strip | what it shows |
|---|---|
| `a82-sweep-f1414-strip.png` | drill 7, the 150-still capture from the sweep at frame 1414, every 24th frame: the human (blue, YOU ring) carrying along the boards and round the corner, the orange defenders at arm's length, through 1414-1990; nothing fires wrong or late; the puck at tile scale is a dot on the blade |
| `a82-spin-f483-strip.png` | drill 8, the spin pressed with the puck at 483, every 4th frame of 24: the figure's turn, the defender at the body, the stamina bar in the HUD |

## 7. Determinism

`Jam.Core.Determinism`, seed 1, frame 600, headless:

| cut | before (`a82-tests-before.log`, = A77's after) | after the lift shield + lane margins only | after `carry_speed_mult` 0.92 (shipped) |
|---|---|---|---|
| hockey.regular | `ccb658f82e4ca910` | (not recorded separately: the scenes have no shielded lift and no saucer, so these two leave it) | **`eb27841dda8fad3f`** |
| hockey.raw | `e09eebd2cebd1337` | (same) | **`0e5637bb401646ae`** |

The hash moves with `carry_speed_mult` (every carrier in every match is slower), as a calibration of every match should; the lift shield and the lane margins alone do
not touch the scripted Determinism scenes (no shielded lift, no saucer in them), and a bot match with either will differ from A77's tree. `Jam.Core.Seams`'s own
hockey.regular line reads `f778484a62221724`: that is its scene (a `SetParam` at frame 10), not Determinism's, and not a comparison this lane recorded before.

## 8. For the Director

1. `Jam.Bot.SeasonTakes` is green on this tree by the coin of section 1 (seed 2 ends at 2229 frames), not by a fix; the choice (a) / (b) / (c) is the Director's. Nothing in the saucer is involved.
2. At the 0.9 margin the brain's saucer trigger is dead (section 3); one condition in `Think` revives it, or the margin is read as "a blade in the lane" rather than "a body
   0.9 m off".
3. The lift on the frame of the sweep itself is not shielded (section 2); moving `TryLift` after `StepStick` would be the fix, at the cost of the input loop's order.
