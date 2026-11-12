# Lane A49: controls that work in the shipped build (device A, from H12.9, top priority, Fable 5.2)

You are lane A49 of the Third Jam (team Hokej). The operator played the H18 package (main 0e5ca5a) and found it unplayable. The operator's words: "Camera does not stay where player
moves, mouse is not binded to work with the stick, the stick blinks back and forth. Player should have option with mouse to tackle or cross-check instead of the shift. Using shift
rather for sprint. No player reaction on tackle making impossible to take puck from opponent." Everything else on device A yields to this lane: nobody else edits input, the pawn's
control path, the follow camera or the stick aim until you report.

Read first: R-0013 (the mouse-aimed stick: poke, lift, fouls), `Game/Source/JamGame/Private/InputMapping.cpp` + `.h` (the code-built Enhanced Input contexts: IA_StickMouse, IA_StickAim,
IA_Poke, IA_Lift, IA_Pass, IA_Deke (A39), check, brake, shoot), `HokejGameMode.cpp` (`ReadHumanInput`, `GatherInputs`: how the mouse becomes `FJamInput::Stick`), `JamPawns.cpp`
(`ApplySimFrame`, `SetStickAim`), `CameraFollow.cpp` (A40's clamp, end-board lift, far-net lead; A33's punch; `jam.camera.Legacy`), `SkeletalSkaterVisual.cpp` and B's `BP_SkaterVisual_TB1`
(`status/SK2-report.md`, `SK3-report.md`: the 24-hold aim grid with bilinear blend; the aim holds exist from -63 to +70 degrees and the blade is corrected onto the ice),
`Game/Source/JamCore/Private/JamSim.cpp` (`TryCheck`, the strip on a check, `ResolvePokes`, `TryLift`, the stick aim's effect on the blade position), `docs/experiments/FEEL.md`,
`docs/experiments/TACKLING.md`, `docs/experiments/CAMERA.md`, `docs/experiments/FIGURE-BAKEOFF.md`, `tools/run/run-packaged.ps1`, `Game/Config/DefaultInput.ini`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a49` (the Director creates it at the current main; build it yourself, unity off; one editor on MCP port 8027 while
proving; at most three editors on the machine). Run no git command; never touch `Game/Plugins/JamAssets` (B's figure Blueprint is B's: if the blink is inside it, write the exact
cause and the fix for B into your report and work around it on A's side with a smoothing of the aim you send). The sim hash never changes for control and camera work; a sim
change (sprint, the strip rule) is a calibration recorded in FEEL.md with the baseline re-recorded.

You own: `InputMapping.*`, the control path in `HokejGameMode.cpp` and `JamPawns.*`, `CameraFollow.*`, `SkeletalSkaterVisual.cpp` (aim smoothing), `JamSim.cpp` only for the sprint and
the strip-on-check rule, `schema/` keys for the sprint, `docs/experiments/CONTROLS.md`, `status/A49.md`.

## Job, in this order, each proven in a packaged Development Game build (`tools/run/build.ps1 -Target Game -Config Development` in your worktree, run with `run-packaged.ps1` or
the exe), not only in PIE, with the mouse and keyboard driven through the real input path (Enhanced Input injection from a test harness, or a scripted `FJamInput` sequence that
mirrors what the bindings produce, stated which)
1. **Find out why the mouse does not reach the stick in the package**: the mapping context not added in the packaged game, a Shipping/Development guard, a config missing from
   the cook, the mouse capture mode, a binding overwritten by A39's or A42's additions. Fix it. Prove: a log line per frame `JAMCTRL stick=(x,y) src=mouse` in the packaged build.
2. **Bindings** (operator's mapping; write them in `docs/experiments/CONTROLS.md` and on the pause menu's controls card):
   - mouse: aim the stick (blade follows the cursor direction around the skater); **left button** shoot (hold to charge, release to shoot); **right button** body check / tackle;
     **middle button or mouse 4** cross-check (the stick lift); mouse wheel or Q deke;
   - keyboard: WASD skate; **Shift sprint** (new: +15 % speed while held, a stamina bar that drains in 3 s and refills in 6 s; a cartridge key `game.skater.sprint { bonus, drain_frames,
     refill_frames }`); Space poke; E pass; 1-4 team signals; Ctrl brake; R restart; Esc pause;
   - gamepad: left stick skate, right stick aim, RT shoot, RB tackle, LB cross-check, LT sprint, A poke, X pass, Y deke.
   Remove Shift from the check. Every binding exercised once in the packaged build with a log line.
3. **The stick blinks back and forth**: find whether it is the aim grid's bilinear blend flipping between holds, the per-bone blend oscillating at the 70-degree clamp, the aim
   input alternating between the mouse and the gamepad default, or the sim's `Stick` vector normalised at zero length. Fix on A's side (a 6-frame smoothing of the sent aim, a
   dead zone, a single source of aim per frame) and name B's part if any. Prove: the sent aim's frame-to-frame change stays under 20 degrees in a 10-second capture.
4. **Camera stays with the player**: the camera's pivot is the human's skater every frame; the far-net lead and the end-board rule never move the player out of the frame's
   middle third; if A40's lead fights the mouse aim, set `far_net_bias` to 0.2 by default and the end-board lift to 3 m; `jam.camera.Legacy` stays as the fallback. Prove with a
   20-second strip in the packaged build: the human never leaves the middle third.
5. **A tackle takes the puck and the victim reacts**: a right-button check on the carrier at closing speed above `check_speed_min` strips the puck 10 of 10 (A16 measured it; verify
   it still holds through the real input path and the figure shows the knock-down or the hit reaction, B-to-A-0028 / A45's calls if present); a cross-check lifts the stick and the
   puck is free; if the sim's check needs the human's own speed and the operator skates slowly, lower `check_speed_min` for the human's check to 2 m/s as a calibration.
6. `docs/experiments/CONTROLS.md`: the binding table, the root causes found, the proofs, and what B must change.

Report: the root cause of each of the five complaints, the binding table, the proof paths, the file list. Interim at 30 minutes with complaint 1 diagnosed; time box 90 minutes.
