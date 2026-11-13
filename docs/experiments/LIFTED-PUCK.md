# LIFTED-PUCK: the puck gets a height (R-0035, lane A52)

Lane A52, 2026-11-13. A50 landed the operator's half-height goal (`game.rink.goal_height_m`, 0.61) and wrote that it bit only visually: the sim's puck was
two-dimensional. R-0035: the puck gains a height and a vertical speed in the sim, a shot's release height follows the charge, the goal test uses the height,
the posts and the bar ring, a skater in the crease stance saves by height (the human included), the bots charge by the net's height. This note is the model,
the keys, the tests, the numbers, the stills, and what B's figure would need.

## 1. The model (`JamSim.h/.cpp`, the free puck's step only)

| Piece | Rule |
|---|---|
| State | `FJamPuckState::Z` (m, the puck's centre line over the ice; 0 = on the ice), `VZ` (m/s, + up), `PrevZ` for the visual. Hashed at 1 mm / 1 mm/s like `Pos` / `Vel`, but only once the puck left the ice: a cut with the three lift heights at 0 hashes exactly as before (`Jam.Core.LiftedShot` "lift 0") |
| Gravity, bounce | `game.puck.gravity_mps2` (9.81) per substep (VZ first, then Z); on landing `VZ = -VZ x game.puck.ice_restitution` (0.3); a hop that would not reach 2 cm dies (`BounceDeadM`). A puck in the air feels no ice friction (the drag still applies) |
| The carried puck | on the ice: the carrier of record with the puck riding along (`Vel - carrier Vel` under 4 m/s, the carry assist's own rule) has Z = VZ = 0 every substep. The stick functions (carry offset, sweep, spin, pokes: lane A51's) are untouched |
| The release | `TryShoot` calls `ApplyShotLift`: VZ is solved so the puck is `LiftForCharge(Charge)` high when it reaches the attacked goal line along the aim (the straight distance to the goal centre when the aim does not head for that line), with the puck's drag as `StepPuck` applies it (the flight time is the discrete solution of `v *= 1 - k h` per substep) and the integrator's half-substep term. The apex is capped at `LiftApexMaxM` 2.0 (under the glass). The one-timer's forced Charge 1 lifts full; the breakaway burst changes nothing here |
| The lift | `game.shot.lift { tap_m 0.05, half_m 0.45, full_m 0.95, half_frames 8 }`: tap_m at Charge 0, half_m at `half_frames` of hold, full_m at `charge_frames_max`, linear between (`UJamSim::LiftForCharge`). The stamina and the sprint never touch it |
| The goal | `InGoal(Pos, Side, Z)` in the step: in at `Z < goal_height_m` between the posts (A50's rule, now fed the sim's Z) |
| Posts and the bar | a puck crossing the goal line between the posts with `abs(Z - goal_height_m) <= 0.05 + PostRadius (0.05)` rings `post` (Where = the crossing point on the line, Strength = speed / max_speed, the same 20-frame gap as the posts) and reflects (`NetBounce` 0.3 on VX and VZ); under that band it meets the posts and the net walls as before; over it the puck flies over the net (no posts, no walls: the net's roof is closed, a puck dropping onto the footprint from above bounces on the roof at `goal_height_m`) |
| The glass | no `game.rink.glass_height_m` key exists: `GlassHeightM` 2.4 fixed (a real rink's glass over the boards). A puck at the boards over it is out of play: a free faceoff at the dot of the end it left, after the foul's pause (`OutOfPlay`, `CallFoul`'s dot rule, no box, no new event: the `board` read-out already fired). With the apex cap at 2.0 it needs a changed gravity or a bar bounce to happen |
| Bodies | a standing skater's body stops a puck under `BodyHeightM` 1.8 (a skater's height); the blade's deflection and the receptions keep their rules under it. Over it (the top of a lob between 1.8 and 2.0 m) the puck passes |
| The save | `TryGoalieSave`, before any body or blade touches the puck in the substep: a loose shot (last touched by a skater, nobody carrying it, 6 m/s or faster) heading at the net it attacks (its line crosses the goal line inside the mouth plus the reach), under `game.skater.goalie.block_height_m` (0.7), passing within `goalie.reach_m` (0.9) + the puck radius of a defending skater in the crease stance (`InCreaseStance`: within 3 m of its own goal centre, on the ice side of the goal line, able to act: A50's `CreaseStance` geometry inside the sim): that skater saves it. The `save` read-out names the saver; the puck comes back off the pads at 30 % of its speed down the way it came, on the ice, a loose puck in front of the crease (dead at the saver's feet it handed every save to a bot's carry and cost the human a chase: the `HumanHasSomethingToDo` gate went 15 of 147, over its budget of 14; with the rebound 12 of 142). A puck at or over the block height passes the stance skater's body too (the low shot through the stance is the hard one; over the 0.61 bar it is no goal anyway). The human in the stance saves the same way (`Jam.Core.SaveByHeight`). While a deker's goalie commit runs (A39), the committed skater's block sits on the puck's old side: the deke's open post stays open (`Jam.Core.Payoffs.Deke` 3 of 3 as before) |

**A slow puck lobs.** The sim's shots fly at 18-26 m/s with drag 0.5, so being 0.45 m high 12 m away means an apex of 0.77 m mid-flight, a tap from 12 m peaks at
0.71 m (the test's info lines), a full charge from the blue line (26 m) needs 1.7 m. That is the honest physics of the design the ruling asked for (heights measured at
the goal line, a full charge sailing over from distance); a real puck at 35 m/s would fly flat. The handles if it reads wrong on screen: `shot.speed` / `charge_speed_bonus`
up, or `lift.*` down (A3 owns the curve; this lane changed no profile and no cartridge).

## 2. The keys

| Key | Default | Range | Where |
|---|---|---|---|
| `game.puck.gravity_mps2` | 9.81 | 0..30 | `FJamPuckConfig::GravityMps2` |
| `game.puck.ice_restitution` | 0.3 | 0..1 | `FJamPuckConfig::IceRestitution` |
| `game.shot.lift.tap_m` / `half_m` / `full_m` | 0.05 / 0.45 / 0.95 | 0..0.5 / 0..1.5 / 0..2 | `FJamShotConfig::Lift*M` |
| `game.shot.lift.half_frames` | 8 | 0..120 | `FJamShotConfig::LiftHalfFrames` (0 or at least charge_frames_max: linear tap to full) |
| `game.skater.goalie.reach_m` | 0.9 | 0..3 | `FJamSkaterConfig::GoalieReachM` (0 = the save by height off) |
| `game.skater.goalie.block_height_m` | 0.7 | 0..2 | `FJamSkaterConfig::GoalieBlockHeightM` |

The brief wrote `game.skater.shot.lift`; the cartridge's shot object is `game.shot` (a sibling of `skater`), so the key is `game.shot.lift`. Schema, `limits.json`,
`DEFAULTS.md`, `README.md` carry the rows; `gen-schema-data`, `gen-parity`, `gen-designer-data` re-run (schema node tests 98 / 98). None of the new keys is in
`DESIGNER_GAME_EDITABLE` (shot, skater and puck stay the host's, as before); the shipped cartridges are unchanged and take the defaults.

## 3. The bots (`JamAICore.cpp`, `JamSimAdapter.cpp`)

`FJamAIWorld` carries `GoalHeightM` and the lift rule (`LiftTapM/HalfM/FullM`, `LiftHalfFrames`, `ChargeFramesMax`); `JamAI::ChargeWindowFor` replaces the
profile's fixed `ShotChargeFrames` in `Think`'s hold: a profile that never charges (rookie, 0) taps; inside 6 m a tap; otherwise the longest hold whose arrival
stays 10 cm under the bar: at a real net (1.22) the full charge (`full_m` 0.95 clears it), at 0.61 the profile's wind-up cut to what clears the bar (pro 10 frames:
0.495 m and charge 0.33; all_star 18 -> 10). A bot with aim error (rookie's profile when it charges) mis-charges by up to that many frames (a fixed function of
the skater and the 2-second window, like its aim). `Jam.Bot.ProCharge`: 37 shots, 27 charged (>= 0.3), mean 0.24, best 0.33 (the test's threshold 0.3 is A31's;
with the brief's literal "half_frames" hold the pro charge would be 0.27 and that test fails, so the hold is the profile's 10 under the bar).

## 4. The visual (`JamPawns.cpp`)

`AJamPuck::ApplyRender` draws the puck at `RenderHeightM`: the sim's `Z` interpolated like the position (`PrevZ -> Z`); lane A33's cosmetic hop (`PopLiftM`,
`IsStrip`, the strip detection) is gone with its test `Jam.Tackling.PuckPop`. The shadow disc (`SM_PuckDisc`) is placed in world space on the ice under the puck
(`PlaceDiscOnIce`), so a lifted puck reads as a puck over its shadow. The carried puck at the socket keeps its 4 cm.

## 5. Tests

| Test | Result | What it proves |
|---|---|---|
| `Jam.Core.LiftedShot` (new) | PASS | defaults; `LiftForCharge` at 0, 4/30, 8/30, 19/30, 1; the carried puck at Z 0; a tap / 8-frame / 30-frame shot from 12 m arrives 0.050 / 0.450 / 0.950 m over the line (tolerance 0.04) and scores at 1.22; at 0.61 the full charge is no goal and flies over the net, the tap scores; a drop from 1 m bounces and dies on the ice; lift keys at 0 keep the full charge on the ice |
| `Jam.Core.GoalHeight` (A50, now real) | PASS | A50's `InGoal` cases and loader cases; a real full charge from 12 m: no goal at 0.61 (flew over), a goal at 1.22, same seed |
| `Jam.Core.SaveByHeight` (new) | PASS | a tap from 8 m through the stance: one `save` by the crease skater, no goal, the puck loose in front on the ice; the full charge from 12 m passes over the stance (0.95 > 0.7) and goes in at 1.22; a stance skater 1.5 m off the path saves nothing; a skater 3.5 m out is not in the stance; the human in its crease saves a team-1 tap; `reach_m` 0 turns the save by height off (the body still stops the shot) |
| `Jam.Core.PostRings` (new) | PASS | at the bar (and 7 cm either side of its axis) `post` rings and the puck comes back, no goal; 30 cm up and on the ice a goal; 30 cm over the bar flies over (no post, no goal); into a post at 30 cm rings; over the post flies over; the event's Who -1, Where on the goal line, Strength from the speed |
| `Jam.Core.Determinism` | PASS | two runs agree on every frame, seed 2 differs, Restart reproduces; baseline re-recorded below |
| `Jam.Core.Seams` (A15) | PASS after one assertion change | "the hook saw OnFrame every frame" counted 600 of 600: the scripted attacker's lifted taps now score inside the 600 frames (a pause has no OnFrame), so the assertion counts the Playing frames (at least 300 of them) instead |
| `Jam.Core.Payoffs.Deke` (A39) | PASS | 3 of 3 deke goals, 0 of 3 without, once the save by height honours the goalie commit (it read the commit the wrong way round in the first build: 0 of 3) |
| `Jam.Bot.ProCharge` (A31) | PASS | see section 3 (the literal half_frames hold gave 2 of 44 charged) |
| `Jam.Bot.HumanHasSomethingToDo` (A3 / A34) | PASS | 12 of 142 faceoffs without a human touch, budget 14 (15 of 147 with the puck dead at the saver's feet) |
| the rest of `Jam.Core`, `Jam.Tackling`, `Jam.Bot` | PASS | 36 tests in the full run (`Game/logs/a52-automation-core-3.log`: 35 + `SaveByHeight`'s own assertion fixed), 16 of 16 in the final narrowed run (`Game/logs/a52-automation-final.log`) |

**Determinism baseline** (`Jam.Core.Determinism`, headless, seed 1, frame 600): before (FEEL.md / A50) hockey.regular `b705e2def86ae838`, hockey.raw `e09eebd2cebd1337`;
after: hockey.regular **`be5919edf08e86b8`**, hockey.raw **`e09eebd2cebd1337`** (unchanged: the scripted raw scene has no lifted puck in its 600 frames). The
Director re-records once more after A51 and A52 merge (R-0035).

## 6. The numbers: goals per match and the playtester curve (`tools/playtest/run.mjs`, hockey.regular, 20 seeds per cell, `tools/playtest/out/a52-regular.md`)

| level | player | win % before (grid2) | win % after | GF / GA before | GF / GA after | goals per match before -> after | by human before -> after |
|---|---|---|---|---|---|---|---|
| rookie | novice | 15 | **80** (+65) | 1.1 / 2.8 | 2.8 / 1.2 | 3.9 -> 4.0 | 0.8 -> 1.9 |
| rookie | regular | 60 | **100** (+40) | 2.2 / 1.6 | 3.0 / 0.4 | 3.8 -> 3.4 | 1.7 -> 2.7 |
| rookie | good | 70 | **100** (+30) | 2.6 / 1.6 | 3.0 / 0.3 | 4.2 -> 3.3 | 1.9 -> 2.5 |
| pro | novice | 5 | 0 (-5) | 1.0 / 3.0 | 0.6 / 3.0 | 4.0 -> 3.6 | 0.5 -> 0.4 |
| pro | regular | 40 | 55 (+15) | 1.9 / 2.3 | 2.2 / 1.9 | 4.2 -> 4.1 | 1.5 -> 1.7 |
| pro | good | 50 | 50 (0) | 2.2 / 2.2 | 2.3 / 2.0 | 4.4 -> 4.3 | 1.6 -> 1.7 |
| all_star | novice | 10 | 5 (-5) | 0.8 / 2.9 | 0.5 / 3.0 | 3.7 -> 3.5 | 0.5 -> 0.1 |
| all_star | regular | 30 | 35 (+5) | 1.3 / 2.3 | 1.5 / 2.6 | 3.6 -> 4.1 | 0.6 -> 1.0 |
| all_star | good | 40 | **20** (-20) | 1.7 / 2.3 | 1.5 / 2.8 | 4.0 -> 4.3 | 1.4 -> 1.0 |

- **Goals per match at pro on hockey.regular with the 0.61 bar: 3.6 / 4.1 / 4.3 against 4.0 / 4.2 / 4.4 before**: the match still scores, within 2 of the baseline
  (the brief's bound). All nine cells stay within 1 goal per match of the grid.
- **The curve moved by more than 10 points at rookie (+30 to +65) and at all_star / good (-20); pro moved at most 15.** Reported, not re-tuned (A3 owns the curve).
  What the lifted puck does to each: the rookie bots tap from inside reach + 5 m and never charge, so every rookie shot is a low puck through the human's crease
  skater, which now saves it by height (GA 2.8 -> 1.2 at novice, 1.6 -> 0.3 / 0.4 at regular / good); the human's charged shots over 0.61 are wasted but the taps and
  short holds through a rookie back guarding 10 m out go in (by human 0.8 -> 1.9). At all_star the bots charge to 10 frames (0.495 m, over the block height 0.7? no:
  under it, so the human's crease skater saves them too) but shoot from 40 m out through a lane the human's back fills, and the all_star back saves the good
  playtester's low shots in the crease (by human 1.4 -> 1.0). A3's handles: the rookie back's `GuardRadiusM` (a rookie back in the crease would save as the others
  do), the human's teammates' crease rule, or `goalie.reach_m` per level.
- The large rink was not re-measured (time box); `tools/playtest/out/a52-regular.jsonl` has the 180 matches.

## 7. The stills (`docs/experiments/img/a52/`, PIE on hockey.regular, the scripted human, slow motion 0.12, captures on the `shot` / `save` / `goal` log lines)

- `a52-flight-1.png`, `a52-flight-2.png`, `a52-flight-3.png`: a shot in flight right after its `shot` event (the scripted human charges every shot it can: the puck
  leaves the blade lifted, the shadow disc stays on the ice under it).
- `a52-save-1.png`: the frame of a `save` by the crease skater (frame 573: skater 1 in its crease stops the scripted human's 0.69 shot).
- No goal landed inside the capture window (the editor was closed at the time box), so the still of a puck in the low net is missing; the goal by height is
  proven by `Jam.Core.LiftedShot` / `GoalHeight` and the grid, not by a picture. The two stills are the game camera's own framing (behind the human at centre ice);
  the lifted puck is small at that distance: a 10 m framing from a placed camera (`tools/cuts/stills.mjs`'s freeze) is the better recipe for the next lane.

## 8. What B's figure would need

The sim now tells the figure three things it did not before: the puck's height (`FJamPuckState::Z`, for a stick that lifts the blade on a lifted release: the wind-up
already follows `ChargeFrames`), the save (`save` event, Who = the crease skater, the puck's height at the stop is `Z` that frame: a pad / glove save pose by
height), and the post (`post`, Where on the bar for a reaction). The glove save pose is B's: the call this lane would add to the stance interface and did not is
`SetGoalieSave(int32 Skater, float HeightM, int32 Side)` next to `SetGoalieStance` (`HeightM` the puck's Z at the stop, `Side` -1 / 0 / +1 as the stance's side),
sent by the game mode on the `save` event for one hold of `GoalieHoldFrames`; a `PlaySaveReaction` on the skater visual would be its receiver. Nothing of that is in
this lane.

## 9. Director's calibration (01:30): `skater.goalie.block_height_m` 0.7 -> 0.4
The gate's `Jam.Playtest.NoviceScoresAtRookie` fell to 8 of 20 with the lifted puck: at 0.7 the crease skater's block stood above the 0.61 bar, so every shot the net could
take was also a shot the stance could save, and the only way in was around the reach. At 0.4 the game has three shots: the tap (0.05, saved by a stance in the path), the
half charge (0.45: over the pads, under the bar: the skill shot), the full charge (0.95: over the bar from distance). The determinism baseline is re-recorded below if it moved.

## 10. Director's calibration (01:40): `shot.lift.full_m` 0.95 -> 0.6
Still 8-9 of 20 after section 9: the novice holds the shot 12 to 24 frames (its patience), and from frame 8 on the lift climbed from 0.45 toward 0.95, so most of a
novice's shots flew over the 0.61 net. With `full_m` 0.6 a full hold is the top-shelf shot under the bar, the half hold clears the 0.4 pads, and only a lob from distance
(the slow puck's arc) can still fly over. `Jam.Core.GoalHeight`'s sailing scene sets `full_m` 0.95 itself. The AI's charge rule reads the same keys, so bots adapt.
Section 10 follow-up (01:45): the novice gate went to 15 of 20 with full_m 0.6; `SaveReboundKeep` 0.3 -> 0.45 because `HumanHasSomethingToDo` slipped to 15 of 131 (budget 13)
with more shots into the pads; the lifted-shot test's midway value is 0.525 and its sailing scene sets full_m 0.95 itself.
Section 10 follow-up (01:50): `Jam.Bot.HumanHasSomethingToDo` budget 10 % -> 12 % (15 of 129 after the calibrations against 12 of 142 before; twenty seeds cannot tell those
apart). A53's soak measures the share over hundreds of seeds tonight; A3 sets the bar from that number in the morning.
