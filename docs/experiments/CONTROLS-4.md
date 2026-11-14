# Controls 4: the charge dies with the puck, the rows that prove the buttons, the whole screen listens (lane A203, track K1 evolution 4)

Tree: main 972ba542 (A199 landed). Status and every measurement as it came: `status/A203.md`. Instruments: `tools/perf/results/a203/`.

## 1. The charge dies with the puck (`game.skater.charge_dies_on_loss`, ships true: the Director, 14 Nov)

The Director's ruling on A199's `ASK`: when a skater stops being the carrier, his shot and pass charges are cleared and a held button charges nothing until it
is pressed again, inside the sim, for the human and the bots, host and guest alike.

- **Where**: `UJamSim::ClearChargeOnLoss(S, In)`, called once per skater in the possession loop just before `bHasPuck` is recomputed: he carried at the end
  of the last frame and `Puck.LastTouch` is now somebody else (a poke, a lift, a check, a teammate's touch). A held shot or pass (the level held, or a
  charge above 0) becomes `-1`: dead. `StepCharge` / `StepPassCharge`: a dead charge grows nothing while held; the release puts it back to 0 and fires
  nothing. A carrier's own flicker (the puck a frame out of reach, still his last touch) does not kill it.
- **Why under `game.skater`**: the rule is about a skater's own held buttons and his possession (skater state, read in his charge steps), not a property of
  the puck.
- **No new state field**: `ChargeFrames` (always hashed) and `PassChargeFrames` (hashed when not 0) carry the `-1`; with the key off they never go below 0.
  No wire change: the key rides the cartridge both peers share (protocol 9).
- **Known corner**: a skater knocked off his feet has his shot charge zeroed by the existing `!CanAct()` line (A184's); if he holds the button through
  the whole knockdown and gets up with the puck in reach, the held button charges again. One word in that line closes it; it is A200's line today.

Proof:

| proof | result |
|---|---|
| `Jam.Controls.ChargeDiesOnLoss` (the human holds 15 frames, a bot's poke takes the puck, it comes back to 0.8 m, he lets go) | shot: release shots 1 (off) -> 0 (on), charge 15 -> -1 at the strip, a new press shoots 1; pass: 1 -> 0 (charge 8 -> -1), a new press passes 1 |
| hash at neutral: build 0 (main's sim) against build 1 (key absent), 100 seeds a cell, six cells | 100 of 100 identical in every cell |
| `Jam.Net.HashCoverage`, `LockstepHashCovers`, `LockstepTwoPeers` | green |

The six cells, 600 paired seeds each, key off / on (the ladder's instrument, `--check closing`). Measured on this tree, without the four difficulty numbers
landing on main today (rookie's opponents' skill and shot reach, two large-rink skill numbers): read the delta, not the level.

| cell | off | on | delta | se | flips up / down | shots: human | mates | opp | goals: human | mates | opp |
|---|---|---|---|---|---|---|---|---|---|---|---|
| pro regular | 46.7 | 47.2 | +0.5 | 2.1 | 79 / 76 | 5.66 / 5.46 | 2.35 / 2.17 | 12.52 / 12.62 | 1.49 / 1.46 | 0.55 / 0.57 | 2.13 / 2.13 |
| all_star regular | 30.2 | 29.2 | -1.0 | 1.8 | 57 / 63 | 5.97 / 5.38 | 1.28 / 1.34 | 18.90 / 18.49 | 1.11 / 1.06 | 0.49 / 0.49 | 2.49 / 2.50 |
| rookie regular | 71.2 | 70.7 | -0.5 | 1.4 | 34 / 37 | 3.63 / 3.54 | 2.44 / 2.31 | 16.86 / 16.91 | 1.85 / 1.82 | 0.65 / 0.67 | 1.45 / 1.45 |
| all_star large | 26.7 | 27.2 | +0.5 | 1.7 | 52 / 49 | 6.13 / 5.73 | 1.64 / 1.58 | 20.55 / 20.47 | 1.70 / 1.70 | 0.57 / 0.54 | 3.69 / 3.63 |
| pro large | 56.5 | 58.7 | +2.2 | 1.7 | 59 / 46 | 5.32 / 4.87 | 2.12 / 2.07 | 13.01 / 12.91 | 1.92 / 1.93 | 0.61 / 0.59 | 2.13 / 2.14 |
| rookie large | 80.5 | 80.8 | +0.3 | 0.9 | 16 / 14 | 4.02 / 3.92 | 2.00 / 1.99 | 12.12 / 12.13 | 2.21 / 2.22 | 0.63 / 0.63 | 1.32 / 1.30 |

The risk the brief named (a bot that loses its charge on every bump stops scoring) does not show: the bots' goals a match move by 0.06 at most, their shots by
0.4 at most. The shots the rule removes are the scripted human's release shots at a stripped puck (0.1 to 0.6 a match).

Shipped on (the Director, on these rows): the code default is true, `JamBaseline::MakeNeutral` sets false; the neutral against main's binary, 100 seeds a
cell, again on the final build: 100 of 100 identical in all six cells. Revert: `"charge_dies_on_loss": false` (or the default back to false).
`Jam.Core.ContactNeutral` lists the neutral by hand (one line added); A199's `HeldAcrossChangeInTheSim` pins the sim's neutral (it compares the mapping's two rules).

## 2. The acceptance rows (step 11)

`HokejAcceptDrill.cpp` step 11, judged in `accept.mjs` (STEPS 11), the hand in `accept.ps1` (the `pass` act takes `ms=`); `keys.json` unchanged (the rows
press the default buttons through it). The pass rules are in `ACCEPT.md`. `jam.accept.To` is 11 by default. `jam.accept.SelfPress 1`: the drill presses
step 11's buttons itself through the human's `UJamPlayerInput` (the real mapping layer): `tools/qa/accept/controls-row.mjs` runs it headless.

Headless run (build 5, `tools/qa/out/a203-controls/`): 5 of 5 PASS.

| row | proven headless (`controls-row.mjs`) | needs the release chain (a real window, the driver's OS input) |
|---|---|---|
| held_across_change | yes, self-pressed | yes: LMB posted by the driver |
| tap_pass_flat | yes, self-pressed (exactly 12 frames) | yes: the driver holds 190 ms |
| saucer_held | yes, self-pressed (15 frames) | yes |
| rebind_refused | yes (no hand) | no |
| pad_possession | yes (the drill presses the virtual pad, RT) | no: the driver cannot post a pad; the row self-presses in the chain too |

## 3. The whole screen listens

While a CONTROLS cell listens, a Slate input pre-processor (`FJamListenCapture`, registered by `BeginListen`) takes every mouse press on the screen before
any widget: a press beside the panel binds and never clicks the menu behind; its release is swallowed; with no cell listening it passes everything.
`Jam.Controls.ListenCapturesOutside` presses through `FSlateApplication::ProcessMouseButtonDownEvent` 200 px right of the panel (924, 354 against a
724 x 709 panel): handled, SPIN bound to MOUSE5.

The same test found a fault: CROSS-CHECK onto R on the defaults was taken ("RESTART moved to no key (bind it again)"): A199's rule refused only a move
that left a verb with no key and no pad button, so a mouse-and-keyboard player lost restart. `FJamBindings::Rebind` now also refuses a key move that leaves
another verb with no key (a pad move: no pad button) where it had one; the screen says "Not moved: R on CROSS-CHECK would leave another control with no key."

Functions touched in `SettingsWidget.cpp` (for A195's merge): `UJamControlsWidget::BeginListen` (6 lines: the registration), new
`UJamControlsWidget::StopListenCapture`, `ReleaseSlateResources`, `BeginDestroy`, new class `FJamListenCapture`. Nothing of the look.

## 4. The first five minutes, by script

`Jam.Controls.FirstFiveMinutes`: a stranger's buttons through the real mapping layer, 3,000 frames on each of 5 seeds at rookie (the brain at rookie plays
the other nine): LMB 45 %, RMB 35 %, both 20 %; 40 % short (2-6 frames), 30 % middling (7-14), 30 % long (15-90); held through faceoffs and goals.
| arm (HeldAcrossChange, SaucerHold, BraceHold, sim rule) | presses | shots | passes (saucers) | checks | goals | shot unmeant | pass unmeant | check carrying | brace on tap | saucer on tap |
|---|---|---|---|---|---|---|---|---|---|---|
| before (0, 0, 0, off) | 285 | 27 | 18 (12) | 56 | 5 | 0 | 0 | 0 | 0 | 0 * |
| mapping shipped (1, 13, 13, off) | 285 | 25 | 15 (2) | 55 | 5 | 0 | 0 | 0 | 0 | 0 |
| shipped (1, 13, 13, on) | 285 | 23 | 16 (2) | 54 | 5 | 0 | 0 | 0 | 0 | 0 |

\* each arm's tap is judged against its own threshold; against the shipped 13 frames the arm of before made 10 saucers on hurried presses (12 -> 2).
No press of the stranger's produced a verb he could not have meant under the shipped mapping: nothing to fix in the mapping layer from this script. Its
limit: in 15,000 frames the stranger was almost never stripped and handed the puck back inside one press (the case `ChargeDiesOnLoss` stages directly).
