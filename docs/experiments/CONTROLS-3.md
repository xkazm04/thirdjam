# CONTROLS-3: the buttons do what a hand expects (lane A199, track K1 evolution 3, R-0064 point 1)

The operator after H31: "Possibility to rebind needs to be there ... Cross check on Left mouse, available only if player not on puck, if yes then
shoot ... Right mouse if on puck: Pass. Hold for saucer pass ... Crosscheck should be holdable so player with speed momentum can run with held
crosscheck into another player". The first cut is on main (CONTROLS-2.md, lanes A183 and A191). This evolution closes what a hand finds in the
first five minutes. Everything here is done in the mapping layer (`FJamButtonResolver`, `JamControls::ApplyButtons`), before the sim's
`FJamInput` is filled: the input struct and protocol 9 are untouched, and a recorded stream replays bit-identical.

## 1. The moment the puck changes hands

The rule: **a button held across a change of possession does nothing until it is released and pressed again, except the brace, which ends.**

Main measured first (`Jam.Controls.HeldAcrossChange`, the resolver stepped frame by frame with each phase's possession facts; the pad's RT / RB give
the same rows step for step). "brace" is the mapping's held level (the sim braces from it only while he does not carry); "RELEASE FIRES" = the level
fell with the puck ours and in reach: the frame the sim fires a held shot or pass.

| button | scene, phase | main (`jam.ctrl.HeldAcrossChange 0`) | shipped (`1`) |
|---|---|---|---|
| LMB / RT | held braced without the puck (20 f) | cross-check 1, brace 20 | cross-check 1, brace 20 |
| | takes the puck in the hit (20 f) | brace level up (gated off while carrying) | nothing (the brace ends) |
| | **loses it, still held (20 f)** | **braces again (20 f)** | nothing |
| | let go, pressed again without | cross-check 1, brace 3 | cross-check 1, brace 3 |
| LMB / RT | shot loading with the puck (20 f) | shot level 20 | shot level 20 |
| | stripped, out of reach, held (10 f) | shot level 10 | nothing |
| | puck back on his stick, held (10 f) | shot level 10 (charging) | nothing |
| | **let go** | **RELEASE FIRES (the old charge)** | nothing |
| LMB / RT | stripped, the puck still in his reach (5 f) | shot level 5 | shot level 5 (held up: its fall would fire at the loose puck) |
| | then out of reach, held (10 f) | shot level 10 | nothing (falls once, nothing in reach) |
| RMB / RB | check called without the puck | check 1 | check 1 |
| | passed to: our pass in flight, then on his stick, held | nothing | nothing |
| | let go, pressed again | pass | pass |
| RMB / RB | saucer loading (12 f), stripped, back, **let go** | **RELEASE FIRES (passes)** | nothing |

In the sim (`Jam.Controls.HeldAcrossChangeInTheSim`: ApplyButtons from a real UJamPlayerInput, a 15-frame charge, an opponent takes the puck, it
comes back to his blade, he lets go): shots fired by the release, main **1**, shipped **0**.

The brief's three questions, on main: a held brace that takes the puck does **not** shoot (the verb is fixed at the press); a held shot whose puck
leaves does **not** cross-check; a held check that is passed to does **not** pass back. The faults were the levels: the re-brace after a loss, and the
old charge fired by the release after the puck came back.

What "a change" is, per verb:
- a **shot or pass** dies when the puck is no longer ours (`JamControls::HasPuckForButtons` goes false). Its level is held up while the puck is still
  inside his carrier reach and falls the first frame it is not (a fall with nothing in reach fires nothing and clears the charge). The corner left: a
  stripped puck that never leaves his reach can still be shot by the release. Closing it needs the sim to clear the charge when the skater stops
  carrying: **ASK** (sim semantics, not the mapping's).
- a **cross-check** dies when he takes the puck (`CarrierIndex` is him): the brace ends and does not come back under the same press. A teammate's
  pass in flight to him does not end it.
- a **check** (an edge) dies when the puck becomes ours.

`jam.ctrl.HeldAcrossChange 1` ships; `0` gives back A183's rule exactly. A dead key logs `JAMCTRL btn=dead verb=<Verb>`.

## 2. Tap against hold

The sim fires on the release and counts the hold itself (shot tap: 4 charge frames or fewer, `shot.tap_frames`; saucer: 8 charge frames,
`pass.saucer.half_frames`; brace: the 10th frame of `bCrossHold`, `UJamSim::CrossHoldTapFrames`). The mapping cannot shorten a threshold, only start
the level late, so the two cvars can only raise it:

- `jam.ctrl.SaucerHoldFrames 13`: the pass level starts 5 frames late; a press let go before then sends one frame of it (a flat pass, one frame
  later: 17 ms). A saucer from the 13th frame held.
- `jam.ctrl.BraceHoldFrames 13`: `bCrossHold` starts 3 frames late. Braced from the 13th frame held.
- `0` on either = the sim's own (8 / 10), the game of before. The shot is untouched (C-0005: its lift grows with the hold; no step to move).

Why 13: a deliberate click lasts 60 to 150 ms and a slow one up to 200 ms (12 frames). At 8, a hurried 9-frame pass tap was a saucer; a 12-frame slow
tap of the cross-check braced for 3 frames (no poke, no reception, 0.9 speed). From 13 a slow tap stays a tap on both buttons: one number for the
hand. The brace's 3 frames of delay cost a held run-in nothing (he holds long before the contact). The pressed cross-check's shove lands 4 frames
after the press (the wind-up), so a 6 to 9 frame press is a pressed cross-check and is never braced, under either value.

`Jam.Controls.TapAgainstHold` (press length, what the sim reads):

| frames held | pass, neutral (8) | pass, shipped (13) | braced from, neutral (10) | braced from, shipped (13) |
|---|---|---|---|---|
| 1, 3, 6 | flat | flat (taps under 6 frames: +1 frame) | never | never |
| 8, 9 | SAUCER | flat | never | never |
| 10 | SAUCER | flat | frame 10 | never |
| 12 | SAUCER | flat | frame 10 (3 braced frames) | never |
| 13, 16, 20 | SAUCER | SAUCER | frame 10 | frame 13 |

The badge shows the hold: RMB reads **SAUCER** from the frame the hold makes it one, LMB **BRACED** from the frame the brace counts. The acceptance
drill's saucer row holds `SaucerHoldFrames + 6` frames (HokejAcceptDrill.cpp, one line).

## 3. The pad

Every verb of the mouse scheme has a pad button (`Jam.Controls.PadScheme`); RT (shoot / cross-check) and RB (pass / check) follow the same
possession rule step for step. The HUD's strip and the badge name the pad's buttons when the pad was touched last (a pad button or the right stick),
and the keys again after a mouse move, a mouse button or a key. The sticks' directions are the game's own (a nudge while a pad cell listens is refused,
not bound). Rebinding on the pad: stand on a pad cell, A, press the new button (Start cancels).

## 4. Rebinding's edges

- **A key bound twice** swaps and says so ("SPRINT is now on SPACE. It was POKE's: POKE moved to SHIFT.").
- **Rebinding never strips a verb** (A198's FAULT 2; the Director's ruling): rebinding one verb of a shared pair splits the pair (the rebound verb
  takes the new key alone, its partner keeps the old one); a swap happens only with the verb that held the new key, which takes the old key when it
  fits, else gives the new key up; a move that would leave any verb with no key and no pad button is refused ("Not moved: ... would leave another
  control with no key"). Before: `Rebind(CrossCheck, Down)` from the defaults left the shot with no mouse key and put LMB on skate back.
  `Jam.Controls.RebindNeverStrips`: every verb onto 44 keys, from the defaults and from 200 random tables: every verb keeps a key or a pad button, no
  key serves two verbs outside a with / without pair, a refused rebinding changes nothing.
- **A saved entry never unbinds** (A198's FAULT 1): an entry with no usable key (unknown names, reserved keys, empty) keeps the verb's default, with
  a `JAMCTRL bindings:` line; a table with a clash or an unreachable verb is refused whole for the defaults, with the reason
  (`Jam.Controls.SavedEntryNeverUnbinds`).
- **Esc cancels** a capture; the Esc that cancelled, still held, does not leave the screen.
- **A key repeat never binds**: the held Enter or pad A that opened the capture is not taken by its own repeat.
- **A mouse button is captured by a real press**: the two-tick grace that could eat a fast press is gone (the capture starts on the click's
  release, so the next press is new), and a quick second click, which Slate sends as a double-click, is taken (it was eaten).
- **RESET TO DEFAULTS asks**: the first press arms it (the button reads CONFIRM RESET for 4 s), the second resets; a capture or the time disarms it.
- **The profile is written whole or not at all**: a packaged game writes its ini in place (`FConfigFile::WriteTempFileThenMove` is false outside the
  editor). `JamControls::InstallAtomicSave` takes GameUserSettings.ini's save through the engine's pre-save hook (`ini.SkipSaveIfPreSaveSucceeded` 1):
  a temp file beside it, then `MoveFileEx(REPLACE_EXISTING | WRITE_THROUGH)`; a failed write leaves the old file and the engine writes as before
  (`Jam.Controls.AtomicSave`).

## 5. The still hand's release (measured, no change)

`tools/qa/aim/a199-release.py` on A186's three camera logs: every frame of every standing stretch tried as the release (the hand moves one pixel).
The blade's jump beyond the hand's own move, median / p90 / worst: with the camera fix on main 0.0 / 0.3 / 1.4 and 0.0 / 0.7 / 1.4 deg; the old camera
0.0 / 0.4 / 9.1. A glide of tau 0.1 s would give a worst of 0.3 / 0.3 / 1.1. Under 1.5 deg is not seen: no change shipped; the glide waits for a
camera that corrects harder.

## 6. PLAY-ME's Controls (the Director copies this)

**The mouse buttons change with the puck.** Mouse aims the stick.
**LMB**: with the puck, shoot (a tap is a flat shot, hold to lift); without it, cross-check; **hold it** (from 0.2 s) and skate into a man: your speed
is the hit (the pill bottom left reads BRACED).
**RMB**: with the puck, pass; **hold it** (from 0.2 s) for a saucer (the pill reads SAUCER); without it, body check.
A press keeps its meaning until you let go, and **a button held while the puck changes hands does nothing until you let go and press again** (a brace
ends when you take the puck). A puck your own side has just passed counts as "with the puck", so a one-timer is still one press.
**Space** poke. **F** ability. **E** spin. **Wheel** deke. **Shift** sprint. **S against your motion** or **Ctrl** brake. **Esc** pause.
Q (pass) and MMB / mouse 4 (cross-check) still work as second keys.
**The pad**: left stick skate (back against your motion brakes), right stick aim, **RT** shoot / cross-check (hold to brace), **RB** pass (hold for a
saucer) / check, **A** poke, **Y** deke, **B** spin, **R3** ability, **LT** sprint, **L3** brake, **D-pad** signals, **View** restart, **Start** pause;
LB and X are second cross-check and pass buttons. The hints on screen follow the device you touched last.
**Every control can be rebound**: SETTINGS (or the pause card), CONTROLS: click a cell (or stand on it and press A), press the new key or pad button;
Esc cancels; a clash swaps the two and the line says so; RESET TO DEFAULTS asks once more. Saved at once with your profile.
