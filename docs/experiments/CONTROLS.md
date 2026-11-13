# Controls that work in the shipped build (lane A49, device A, H12.9-H14, 2026-11-12)

The operator played the H18 package (main 0e5ca5a) and found it unplayable: "Camera does not stay where player moves, mouse is not binded to work with the stick, the
stick blinks back and forth. Player should have option with mouse to tackle or cross-check instead of the shift. Using shift rather for sprint. No player reaction on
tackle making impossible to take puck from opponent." This page has the root cause of each complaint, the binding table, the proofs (every one from a packaged
Development Game build driven through the real input path) and what device B must change. Status blocks with timestamps: `status/A49.md`.

Code: `Game/Source/JamGame/{Public,Private}/InputMapping.{h,cpp}` (the bindings, `jam.ctrl.Log`), `HokejGameMode.{h,cpp}` (the match input mode, the cursor aim, the
proof lines, the drill), `Game/Source/JamCore/{Public,Private}/JamPawns.{h,cpp}` (`SmoothAimDeg`), `CameraFollow.{h,cpp}` (the middle-third budget, `FrameFraction`,
`jam.camera.Log`), `JamSim.{h,cpp}` + `JamCartridge.{h,cpp}` (the sprint, the human's check calibration), `schema/game.schema.json` + `DEFAULTS.md` (`game.skater.sprint`),
`HUDWidget.cpp` (the strip text, the stamina bar), `SettingsWidget.cpp` and `DirectorsCallWidget.cpp` (the controls card). Proof driver: `tools/qa/ctrl/a49-prove.ps1`
(user32 SendInput: the OS cursor, mouse buttons, wheel and keys reach the game window as from the operator's hand) and `tools/qa/ctrl/a49-analyze.mjs` (reads the log).

## 1. Binding table (the operator's mapping)

| Action | Mouse / keyboard | Gamepad | Sim field | Notes |
|---|---|---|---|---|
| Skate | W A S D (arrows too) | left stick | `Move` | camera-relative: up = away from the camera |
| Aim the stick | **mouse**: the blade points at the cursor on the ice | right stick | `Stick` | cursor shown and locked to the window; dead zone 0.5 m around the skater (hold ahead), reach 0.5..1 over 0.5..3 m from the skater; right stick wins when moved; the accumulated mouse delta (100 px = one reach at `stick.mouse_sensitivity`) is the fallback when the cursor is hidden |
| Shoot | **left button**, hold to charge, release to shoot | RT | `bShootHeld` | `shot.charge_frames_max`, `charge_speed_bonus` as before |
| Body check / tackle | **right button** | RB | `bCheck` | opens the check window: 12 frames for the human (bots 6); lands from 2 m/s of the human's own speed (bots `check_speed_min`) |
| Cross-check (stick lift) | **middle button** or **mouse 4** | LB | `bLift` | strips an opposing carrier within `stick.lift_range_m` (1.2 m beyond the two bodies); 30-frame cooldown |
| Deke | **mouse wheel** (either way) or Q | Y | `bDeke` | lane A39's deke: the one-tap sweep (it arms the same poke shield, lane A51) |
| Spin | **E** | B | `bSpin` | **lane A51 (R-0034)**: a full turn over 24 frames, the puck on the far side of the body from the nearest opponent (no poke reaches it), speed x0.7, a quarter of the stamina bar, 90-frame cooldown; a check landing mid-spin knocks down 1.5x longer |
| Sprint | **Shift** (held) | LT | `bSprint` | **new**: +15 % top speed and thrust while moving and the stamina lasts; drains in 3 s (180 frames), refills in 6 s (360); `game.skater.sprint { bonus, drain_frames, refill_frames }`; a bar bottom-centre of the HUD while it is not full |
| Poke | Space | A | `bPoke` | |
| Pass | **F** | X | `bPass` | lane A51 (R-0034): moved from E (E is the spin) |
| Team signals | 1 2 3 4 | D-pad | (signal system) | unchanged (lane A3) |
| Brake | Ctrl | left stick click | `bBrake` | the left trigger went to the sprint (A49), B to the spin (A51) |
| Restart | R | View / Back | | |
| Pause / Director's Call | Esc | Menu / Start | | |

Removed: Shift from the check, Space / A from the shoot (now poke), left mouse from the poke, right mouse from the lift. The table is also `UJamInputMap::BindingTableText`,
the first-10-seconds strip (`HUDWidget.cpp`), the settings card (`SettingsWidget.cpp`) and the pause menu's controls card (`DirectorsCallWidget.cpp`).
LOCKSTEP: `FJamInput::bSprint` is a new input field; lane A38's `FJamNetQuantInput` must carry it as a flag bit (as it does `bDeke`) or two peers desync at the first sprint.

## 2. Root causes

### Complaint 1: "mouse is not binded to work with the stick" (the package only)

`AJamMenuGameMode::PostLogin` (`MenuGameMode.cpp:47-52`) puts the player in `FInputModeGameAndUI` with the cursor shown so the film-slate menu can be clicked. That mode
sets the game viewport's mouse capture to *capture during mouse down only*. The `UGameViewportClient` belongs to the game instance and survives the `OpenLevel` into the Rink
map, and nothing in the match set an input mode (the only `FInputModeGameOnly` calls were the pause menu's `Resume()` and `QuitToMenu()`). `FSceneViewport::OnMouseMove`
accumulates `MouseDelta`, the source of the `MouseX` / `MouseY` axis events and so of `Mouse2D` -> `IA_StickMouse`, **only while the viewport has mouse capture**. In the
package the stick therefore moved only while a mouse button was held down, and the accumulator then kept whatever it had: a stuck aim, as often as not behind the skater
(which is what made complaint 3 so visible). PIE starts on the Rink map, never passes the menu and gets the ini default `CapturePermanently_IncludingInitialMouseDown`: that
is why the mouse worked in every PIE proof (A16, A39) and never in the shipped build. After one Esc + Resume the package worked by accident (GameOnly = permanent capture).
Not a Shipping guard (the Development package shows the same), not the cook, not a binding overwritten by A39 or A42 (`IMC_Hokej` is built in code and had every key).

Fix: `AHokejGameMode::EnsureMatchInputMode` on possess and again after any Resume (the game mode reads the human every sim frame and re-applies it when the cursor was
hidden while the world is not paused): `FInputModeGameAndUI`, cursor shown, locked to the viewport, the HUD `HitTestInvisible` so no plate eats a click. The aim itself no
longer depends on capture at all: `CursorAimRink` deprojects the cursor onto the ice (z = 0) and sends the vector from the skater to that point, so the blade points at the
cursor the operator sees. The right stick wins when moved; the delta accumulator stays as the fallback when the cursor is hidden.

### Complaint 3: "the stick blinks back and forth"

`AJamSkater::ApplySimFrame` sent the visual the blade angle relative to the facing, `-(atan2(blade - pos) - yaw)`, unwound to (-180, 180]. The mouse aim is held in rink
space (camera-relative) while the skater turns, so an aim that ends up behind the skater sits at +-180, and both figures clamp to their holds (+-70 on the skeletal toy,
-63 / +70 on B's grid): a facing that jitters by a degree (the sim's grip dynamics at speed) flips the clamped aim from -70 to +70 and back every frame. It was not the
bilinear blend, not the per-bone blend oscillating at the clamp (both are stable for a stable input), not two aim sources alternating (the right stick is read first and is
zero without a gamepad), not the zero-length normalisation (`StepStick` guards it with `Mag > 0.05`). The stuck-behind aim of complaint 1 made it constant.

Fix on A's side (`AJamSkater::SmoothAimDeg`, pure, tested): the side changes only through the front (|raw| <= 90 decides the side, with a 2 degree band that keeps the last
side straight ahead); behind, the stick stays on the side it was on, at the clamp; the sent angle approaches its target by 35 % per frame (within 10 % after 6 frames) and
never more than 15 degrees in one frame; the reach follows with the same approach. B's part: none required; B's grid receives a side-stable, slewed angle in [-70, 70].

### Complaint 2: "camera does not stay where player moves"

The pivot was the human every frame (`Frame.HumanIndex`); what moved the human out of the frame was the rotation: A40's far-net lead (`far_net_bias` 0.5 = half the angle
to the net, up to 30 degrees when the skater crosses the slot sideways) and the end-board yaw (up to the whole along-the-rink turn when the boards take the arm) rotate the
camera away from the skater while the position stays behind it, and the lift to 5 m tilts up the rink; with a 70 degree FOV the skater then sits at the frame's edge or
out of it. Fix: the yaw may differ from the bearing camera -> pivot by at most 12 degrees (the middle third's edge at 70 degrees of FOV is 13.1), the pitch from the
bearing to the chest point (1 m up) by at most 6.5 degrees (the vertical third's edge at 16:9 is 7.5); in open ice the F1 rig is inside both (6.7 degrees of off-axis,
4.9 of pitch) and nothing changes; the lead and the end-board rule work inside the budget. Defaults `far_net_bias` 0.2 and `end_height_m` 3 (`DEFAULTS.md`,
`FJamCameraConfig`, `FJamCameraSettings`). `jam.camera.Legacy 1` stays the fallback.

### Complaint 4: "mouse to tackle or cross-check instead of the shift; shift for sprint"

Bindings (section 1) and the sprint in the sim: `FJamInput::bSprint`, `FJamSkaterState::Stamina` in integer ticks (full = drain_frames x refill_frames; a sprinting frame costs
refill_frames, a resting frame returns drain_frames, so the bar empties in exactly drain_frames and fills in refill_frames), hashed only once it left full so a match
without a sprint hashes as before; bots never set `bSprint`.

### Complaint 5: "no player reaction on tackle, impossible to take the puck"

Two sim rules, both tuned for bots: the check window of 6 frames (0.1 s: a person's press had to land within six frames of the contact) and `check_speed_min` 5 m/s of the
checker's own speed along the contact normal (a human closing on a carrier at a walking pace never reached it, so no `check` event, no knock-down, no strip, no reaction:
nothing happened at all, exactly what the operator saw). Calibration (FEEL.md, lane A49): the human's window is 12 frames and the human's check lands from 2 m/s; bots keep
both old values, so the scripted Determinism baseline does not move. The strip itself (C-0005: a landed check on the carrier always strips) and the victim's reaction
(`SetKnockedDown` on the IsDown edge, B's `BP_SkaterVisual_TB1` and the skeletal toy both play a fall) were already there and fire once the check lands.

## 3. Proofs (packaged Development Game build, `Game/Packaged/A49-dev/Windows/ThirdJam/Binaries/Win64/ThirdJam.exe`)

Every proof below ran in the packaged Development Game build (BuildCookRun, `tools/run/package.ps1 -Config Development`, 80 s with the cached cook; the Shipping
exe has no log and no cvars, A48) and was driven through the **real input path**: `tools/qa/ctrl/a49-prove.ps1` sends the OS cursor, mouse buttons, wheel and keys with
user32 `SendInput` into the game's Slate window (class `UnrealWindow`, with the foreground verified), so Windows -> `FWindowsApplication` -> Slate -> `UGameViewportClient` ->
Enhanced Input -> `UJamPlayerInput` -> `AHokejGameMode::ReadHumanInput` -> `FJamInput` is the path the operator's hand takes. The match is entered from the film-slate menu
with Enter (A48's path), as the operator does. Logs: `tools/qa/out/ctrl/<run>/game.log` (+ `events.log`, what was sent and when); the verdicts are `a49-analyze.mjs`'s.
The one scripted part is the tackle / cross-check drill (section 3.5): the human's `FJamInput` is scripted there (WASD toward the carrier, one frame of `bCheck` or `bLift`,
exactly what the right / middle button produce, which the bindings run proved), because a blind cursor cannot find a bot carrier.

### 3.1 The mouse reaches the stick (complaint 1)

Runs `bindings4` and `bindings5` (the final package; 81 s of match, 4837 sim frames in `bindings5`): `JAMCTRL input mode set: game+ui, cursor shown and locked to the viewport, the HUD hit-test invisible` at the match
start **and again after the pause menu's Resume** (frame 3000); `JAMCTRL frame=N stick=(x,y) src=mouse` on **4407 frames** (the other 430 are `src=none`: the cursor
parked on the skater in the dead zone during the key tests), the stick vector changing on 1112 frames while the cursor circled; never a `src=mousedelta` fallback. Before
the fix, the same package logged `src=none` for every frame the cursor moved without a button down (run `bindings`, the unmodified capture mode).

### 3.2 Every binding once (complaint 4), through the OS

`JAMCTRL frame=1202 btn=shoot held=1 (left mouse / RT)` ... `held=0` at 1262 (a 1 s hold), `1321 btn=check (right mouse / RB)`, `1411 btn=lift (middle mouse)`,
`1501 btn=lift (mouse 4)`, `1591 / 1681 / 1772 btn=deke` (wheel up, wheel down, Q), `1860 btn=poke (space)`, `1951 btn=pass (E)`, `JAMSIGNAL intent=defend / Space /
praise` (keys 1-3; 4 was the praise's target), `2310 btn=sprint held=1 (shift)` ... `2552 held=0` (4 s), `2642 btn=brake held=1 (ctrl)` ... `2701 held=0`,
`JAMCALL paused at frame 3000` (Esc) and the input mode re-applied on Resume (Esc). WASD: `move=` non-zero on 2460 frames (W held, A S D taps). The gamepad half of the
table is the same actions with the pad keys (`Jam.Input.Mapping` asserts every key of the table; no pad on the machine).

### 3.3 The stick does not blink (complaint 3)

`JAMAIM frame=N raw= sent= d=` on all 4843 frames (81 s, the 10 s asked for eight times over): **max |d| 15.0 degrees per frame, 0 frames over 20**, while the cursor
circled the skater every 3 s (the raw aim swept every direction, behind included: the raw flips +-180 while the sent aim holds the side at the clamp). Unit test
`Jam.Core.AimSmoothing`: 600 frames of +-179 jitter hold +70 without a flip; the swing through the front slews at 15 degrees a frame and settles.

### 3.4 The camera stays with the player (complaint 2)

Final run `bindings5`: `JAMCAM frame=N x= y= third=` on **4839 frames (80.7 s), 4839 in the middle third**, max |x| 0.237, max |y| 0.227 (the edge is 0.333), through two
goals and their faceoff resets. The run before it (`bindings4`, 4828 of 4845) had 17 frames outside right after a goal: the faceoff reset teleports the skaters to the
dots and the 4-frame lag filter caught up over 17 frames with the skater behind the camera; since then a pivot jump above 5 m snaps the filter (the human is the pivot
every frame, by position as well as by index).
Unit tests: `Jam.Camera.MiddleThird` (1690 frames across the slot with the lead, into and along the end boards, out of a corner, the old 0.5 / 5 m defaults too: max |x| 0.154, |y| 0.221, 0 outside), A40's
`BoardsClamp` (no pop: the budget bounds the spring's target), `EndBoardsLift` (the lift still happens, 3.98 m with the bounded yaw; the "look along the rink" and "far
net in frame" assertions became informational, superseded by the human's framing).

### 3.5 A tackle takes the puck and the victim reacts (complaint 5)

Runs `drill1` (tackle) and `drill2` (cross-check), `jam.ctrl.Drill 1 / 2`, 100 s each: the human's `FJamInput` is the scripted drill (`AHokejGameMode::DrillInput`): `Move`
toward the opposing carrier at the unit length WASD gives, `Stick` toward him, and **one frame of `bCheck` (the right button's edge) when the bodies are about to meet inside
the human's 12-frame window** (gap <= closing speed x 9 frames), or one frame of `bLift` (the middle button's edge) inside `lift_range_m`. The run `bindings5` proved that
the right / middle buttons produce exactly those edges. Each attempt reads the sim's own events for 20 frames (`check` by the human, `knockdown` of the victim, `lift` clean),
whether the puck left the victim (`Puck.LastTouch`), and whether the victim's figure was told to fall (`IsDown` edge -> `SetKnockedDown(true)` in `AJamSkater::ApplySimFrame`).

| Drill | Presses in 100 s | Landed | Victim knocked down and the figure sent `SetKnockedDown` | Puck left the victim within 20 frames |
|---|---|---|---|---|
| tackle (RMB) | 7 | **5 of 7** (the two misses: pressed 2.5-3.1 m out at 15-18 m/s closing and the carrier turned away before contact) | 5 of 5 | 3 of 5 landed (+1 of the misses, a bump); on the other two the puck was already away from the victim at the hit (`LastTouch` elsewhere), the knock-down happened all the same |
| cross-check (MMB) | 7 | **7 of 7 lifts clean** (`lift` strength 1) | (a lift knocks nobody down) | 6 of 7 |

So through the real input path a check that connects knocks the carrier down and the figure plays its fall (B's `BP_SkaterVisual_TB1` gets the same `SetKnockedDown` the
skeletal toy does), and a cross-check in range takes the puck every time. A16's 10 of 10 was a scripted head-on hit on a standing carrier at 9 m/s; in open play against
bots the hit itself lands when the bodies meet inside the window (5 of 7 here), which is the calibration's job: before it (6 frames, 5 m/s of own speed) the operator's
checks never connected at all. The press timing a person has is the 12-frame window; the drill's second miss (closing 15.5 m/s from 2.5 m) is a press a frame too early.
Logs: `tools/qa/out/ctrl/drill1/game.log`, `drill2/game.log` (`JAMDRILL press` / `attempt` lines).

### 3.6 Determinism

`Jam.Core.Determinism` on the editor binary with every change in: hockey.regular seed 1 frame 600 **`b705e2def86ae838`**, hockey.raw **`e09eebd2cebd1337`**: the baseline
of A39 / A45, unchanged (the calibrations are human-only, the sprint stamina is hashed only once used). `Jam.Core.Stick`, `CartridgeRoundTrip` (the embedded schema with
`game.skater.sprint`), `Jam.Signals`, `Jam.Input.Mapping`, the camera suite including `Jam.Camera.MiddleThird`: **15 of 15 PASS** (`Game/logs/a49-tests4.log`). Schema: 98 of 98 node tests, 11 of 11 cartridges validate.


## 4. What device B must change

Nothing for the blink: A sends a side-stable, slewed aim in [-70, 70] (B's grid clamps -63 / +70 as before). Two asks remain from SK2: the far-reach side holds
(`Aim_FarLeft` / `Aim_FarRight`) so the blade stays on the ice above 1.0 m of reach at +-60 degrees, and the stride's pelvis height (blade at -7 cm under the aim layer).

## 5. Stick handling with the mouse, the spin on E, the pass on F (lane A51, R-0034, 2026-11-13 00:45-02:15)

The model, the keys, the tests, the curve and what comes next are in `docs/experiments/STICK.md`; this section is the control path's part.

- **The puck lives where the blade is.** The carried puck follows `BladePos` every frame (the carry assist, 68 % of the gap a frame), and `BladePos` is the
  cursor's aim on the ice within `game.stick.reach_m` (1.1, range 0.6..1.6): forehand to backhand across the body as the cursor sweeps. An aim turning more than
  `game.stick.sweep.min_deg_per_frame` (6) in one frame is a **sweep**: for `shield_frames` (12) an opponent's poke from the side the puck left misses. A
  defending bot reads the puck's place on the carrier late (rookie 14, pro 8, all_star 5 frames), so a sweep timed as its stick closes beats its poke.
- **The spin on E** (gamepad B): `game.skater.spin` (24 frames, cooldown 90, speed x0.7, a quarter of the bar; the bar shows while not full). The figure turns the
  full circle through `UpdateMotion`'s facing and the actor's rotation (no new C-0001 call); the puck rides the far side of the body from the nearest opponent.
- **Bindings**: E spin, **F pass** (E no longer passes), gamepad B spin, the brake to the left stick click; the strip, the settings card, the pause card and
  the table in section 1 say so. `Jam.Input.Mapping` asserts all of it.
- **Log lines in the package**: `JAMCTRL frame=N btn=spin (E / B)`, `btn=pass (F / X)`, and the sim's read-outs on the human `JAMCTRL frame=N sweep side=S
  shield_frames=12 blade=(x,y)`, `JAMCTRL frame=N spin frames=24 dir=D stamina=0.75 carrying=1`.
- **The 1v1 drill** `jam.ctrl.Drill 3` (sweep) / `4` (spin): unlike A49's drills the human's `FJamInput` is **not** scripted; `StickDrillInput` calls
  `ReadHumanInput` (the cursor and the keys are the real path) and overrides only `Move` (carry the puck at the nearest defender at 60 % speed, hold the line
  1.6 m out, chase a loose puck). `StickDrillObserve` logs `JAMDRILL stick <kind> frame= n= defender= dist=` on every sweep / spin the sim saw and
  `JAMDRILL stick_result <kind> n= kept= poked_clean= knocked= knock_frames=` 30 frames later, a `stick_summary` after 10 attempts.
- **Harness**: `tools/qa/ctrl/a51-prove.ps1 -Exe <Development ThirdJam.exe> -Mode sweep|spin` (A49's user32 SendInput bridge: the cursor crosses the skater in
  three steps every 0.6 s, or E every 2.5 s), `a51-analyze.mjs` on `tools/qa/out/ctrl/a51-<mode>/game.log`. Results: `status/A51.md` and STICK.md section 7.
- **Lockstep**: `FJamInput::bSpin` is flag bit 512, protocol 5 (`Jam.Net.InputCoverage`, `PacketRoundTrip`, `LockstepTwoPeers` PASS).
