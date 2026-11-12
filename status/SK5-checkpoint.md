# SK5 checkpoint after item 2 (device B, 2026-11-12 ~20:56)

Lane SK5, main tree, editor on port 8000 (pid 6924). No git, no build. `/Game/Maps/Rink` not saved (it is dirty only with my test figures tagged `SK5`; discarded at the end).

## Item 1: re-import. PASS
- 72 takes from `skf/export5/p2_cel/SK_Skater_p2_cel_loopcut.fbx` (animation only, SK4's `ri` script): 70 replaced in place, 2 new (`Idle`, `Stumble_Short`), 0 failures, nothing left behind.
- `BS_SkaterTB1v2_Skate` 14 samples / 0 empty, `BS_SkaterTB1v2_Aim` 24 / 0 empty after the import.
- Lengths: `Stumble_Short` 0.333 s (20 frames), `Poke` 0.35, `Idle` 1.983 loop, `Trip` 0.5, `Check_Hit_Back` 0.467, `Poke_Hit` 0.4.
- Aim and correction re-measured on 15 test figures at the cell centres in Glide (PIE, figures driven every tick): blade error 0.0 cm, blade bottom 1.6 cm everywhere, correction used max 3.7 deg pitch / 3.1 deg yaw / 2.7 cm slide (SK3: 3.5 / 3.1 / 2.4). Same as SK4/SK3.

## Item 2: BP_SkaterVisual_TB1 (and ABP_SkaterTB1). PASS
- (a) `PlayPoke(false)`: `Poke`, then a hard cut (blend 0) to `Stumble_Short` once 0.1 s of game time has passed, at the phase it should be. Match: `JAMEVENT poke ... frame=272 kind=miss`, `SK5POKE false f 32933`, `SK5STUMBLE dt 0.1 f 32939`: **the cut is 6 frames after the call**. The stumble then runs 20 frames, to frame +26; the sim's stumble is frames +0..+20.
- (b) Rest: the AnimBP `Idle` state now plays `Idle` (was `Idle_Faceoff`). The faceoff pose is unchanged: it is `Faceoff_Ready` through `bStFace`, as before (`Idle_Faceoff` is no longer played by anything). Pelvis at rest 76.5..77.0 cm (was 65.8). Correction at rest, 6 figures x 8 samples: max 2.9 deg pitch / 2.2 deg yaw / **1.9 cm slide** (SK4 match p90 7.7 cm, SK3 rig max 10.2 cm).
- (c) Trip: `SetTripped(true)` plays `Trip`, holds, `SetTripped(false)` plays `GetUp_Side_Right` (was `GetUp_Front`).
- (d) Front hit: push against the facing plays `Fall` with the figure facing the checker, falling backward along the push, away from the checker: already correct, unchanged.
- (e) Strength: the Blueprint now binds to `JamEventBus.OnJamEvent`; a `knockdown` event whose Who is its own skater's sim index sets the strength; the knockdown plays at 0.95 + 0.2 x strength (x the learned time-scale). Match: `knockdown ... strength=1.000` -> `SK4KD AS_SkaterTB1v2_Fall rate 1.15`. The event arrives before `SetKnockedDown` in the same frame; the handler also re-rates a knockdown already started.
- (f) Reaction events tested from Python: hit from behind / left / right -> `Check_Hit_Back` / `_Left` / `_Right`; board -> `Board_Hit`; pin -> `Board_Pin`; stick hit -> `Poke_Hit` / `Lift_Hit`. The frame 3->4 step shows in the engine on all three `Check_Hit_*` and `Poke_Hit` (clavicle 47-66 deg in one frame, Poke_Hit stick 72-80 deg). A 2-frame blend-in does not reduce it (52 deg); **0.12 s blend-in** (was 0.08) brings it to 27-30 deg (stick 46 deg). Applied to `Check_Hit_*` and `Poke_Hit` through a new `PlayCut` event.

Next: item 4 (puck), item 5 (glass), then item 3 (haze) after 21:00, then item 6.
