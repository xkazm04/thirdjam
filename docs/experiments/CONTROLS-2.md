# CONTROLS-2: the operator's hands (lane A183, track K1, R-0064 point 1)

The operator after H31: "Cross check on Left mouse, available only if player not on puck, if yes then shoot. Right mouse if on puck: Pass. Hold for
saucer pass. Crosscheck should be holdable so player with speed momentum can run with held crosscheck into another player. Possibility to rebind."

## The default bindings (FJamBindings::Defaults, InputMapping.cpp)

| Verb | Mouse and keys | Pad | Notes |
|---|---|---|---|
| Shoot (with the puck) | LMB | RT | tap flat, hold to lift (C-0005, unchanged) |
| Cross-check (without) | LMB, MMB, mouse 4 | RT, LB | held = FJamInput::bCrossHold (A184) every frame while not carrying |
| Pass (with the puck) | RMB, Q | RB, X | hold = saucer (A77, unchanged) |
| Check (without) | RMB | RB | the body check |
| Poke | Space | A | |
| Deke | wheel up / down | Y | |
| Spin | E | B | |
| Ability | F | R3 | |
| Sprint | Shift | LT | |
| Brake | Ctrl (and S against the motion) | L3 (and the stick back) | |
| Skate | W A S D, arrows | left stick | four rebindable directions; the stick is fixed |
| Restart | R | View | |
| Pause | Esc | Start | fixed (Esc cancels a listening row) |
| Aim | mouse | right stick | fixed |
| Signals | 1-4 | D-pad | fixed (SignalSystem) |

Q (pass), MMB and mouse 4 (cross-check), X (pass) and LB (cross-check) stay as second bindings: the operator did not ask to free them (lane A191,
the Director's word); a player who wants them free rebinds them on the CONTROLS screen.

What moved on the pad: RB was the check only, now pass with the puck / check without; RT was the shot only, now shot / cross-check; LB and X stay as
second bindings (cross-check, pass). On the mouse: RMB was the check only; Q and MMB stay as second bindings.

## The rule (FJamButtonResolver, JamControls::ApplyButtons)
- A key bound to a with-verb (shoot, pass) and a without-verb (cross-check, check) is a two-role key. Its press is resolved at the sim step that takes it,
  from that frame's sim: "with the puck" = the skater carries it (`UJamSim::CarrierIndex`), or the puck is loose and was last touched by his own team
  (our pass in flight: the one-timer's press comes about 10 frames before the reception). `jam.ctrl.OurPuckShoots 0` = carrying only.
- The verb holds until the key is released: a shot or a pass begun with the puck stays one when the puck leaves; a cross-check begun without stays one
  (it never becomes a shot) when the puck arrives. A press released before its first step counts for that step (a tap).
- A key on one role only (Q, MMB, mouse 4, LB, X) always does that role.
- The resolution happens before the FJamInput is recorded or sent (AHokejGameMode::ReadHumanInput, ALobbyGameMode's local read): the lockstep sees
  verbs only, so a recorded stream replays to the same hash under any binding (Jam.Controls.RecordedStreamUnchanged: 28ed08cec25589dd both ways).
- Shared keys: only one with-verb and one without-verb may share a key; any other clash is refused or swapped (FJamBindings::Rebind, IsValid).

## The held cross-check (A184's field)
InputMapping.h (lane A191 moved it from InputMapping.cpp): `#define JAM_INPUT_HAS_CROSS_HOLD 0`. When A184's `FJamInput::bCrossHold` is on main the Director sets it to 1;
`JamControls::SetCrossHold` then writes the level (held and not carrying). Until then the level is computed and logged (`JAMCTRL btn=crosshold held=`).
The lockstep wire for the field (a flag bit) is A184's.

## Rebinding (UJamControlsWidget, SettingsWidget.cpp)
`UJamSettingsWidget::MakeControlsScreen(Outer)` returns the screen; `OnBack` fires on BACK / Esc / pad B. A row per verb (the two-role buttons as two
rows), a keys cell and a pad cell; click (or pad A) to listen, the next key / mouse button / wheel / pad button binds, Esc cancels; a clash swaps (the
line says which verb moved where); RESET TO DEFAULTS. Saved at once with the profile: GameUserSettings.ini `[Jam.Controls]`, one key per verb
(`Shoot=LeftMouseButton|Gamepad_RightTriggerAxis`), in the same flush as `[Jam.Settings]`; read at start (UJamInputMap::Build). Tests use
`FJamSettings::SetOverrideForTest`, never the player's file.

## The words follow the bindings
`UJamInputMap::HintStripText` (HUD first strip, at most 145 characters with any binding: entries drop from the end), `PauseCardText` (the pause card's
five lines), `ControlRowsFromBindings` (the menu's CONTROLS rows), `BindingTableText`, `KeyName` (at most 12 characters). The brake prompt already read
the live mapping context.

## The acceptance run
`tools/qa/accept/keys.json`: shoot LMB, crosscheck LMB, pass RMB, check RMB, onetimer LMB; `accept.ps1` presses from it (`-OldKeys` = the table of
before). Jam.Controls.AcceptKeysMatchDefaults holds the file against the defaults.

## Evolution 2 (lane A191)

### A still hand keeps its point on the ice (jam.ctrl.AimAnchor, FJamAimAnchor)
The blade aims at the cursor's ice point, re-projected through the camera every sim step (AHokejGameMode::CursorAimRink), so a camera correction
swings the blade under a still mouse (A186: up to 14.8 deg a still correction after its rig-yaw fix). Measured on A186's three real-match camera logs
with `tools/qa/aim/a191-aim.py` (the table in status/A191.md): A = the blade's turn under a still cursor through corrections with the skater still;
B = the drift between the blade's aim and the cursor's true ice point in play after the hand stops.

| variant (camera with A186's fix) | A worst deg | B p90 at 0.25 / 1 / 2 s | B worst at 2 s |
|---|---|---|---|
| 0 re-project every step (before) | 17.1 | 0 / 0 / 0 | 0 |
| 1 anchor whenever the pixel is still | 0.0 | 48 / 140 / 147 | 179 |
| 2 anchor while the camera's correction stage moves the picture | 0.0 | 19 / 87 / 116 | 176 |
| 3 catch-up, tau 0.25 s | 2.5 | 28 / 35 / 37 | 89 |
| **4 anchor while the skater stands (< 0.3 m/s)** | **0.0** | **0 / 0 / 0** | **5.5** |

Shipped: variant 4, `jam.ctrl.AimAnchor 1`, `jam.ctrl.AimAnchorSpeed 0.3`. A pixel move of half a pixel or more (summed from where the anchor began),
or the skater skating, re-projects at once. `jam.ctrl.AimAnchor 0` = the aim of before.

### The held cross-check row (acceptance step 7 k=3)
The drill stages the human 9 m out at 5 m/s, the carrier standing; the driver holds LMB (`act=lmb_hold ms=2200`) while W carries him in; the verdict
`crosscheck_held`: the man went down, with the closing speed at the first touch. With `JAM_INPUT_HAS_CROSS_HOLD 0` the row logs `skipped=no_hold`
(accept.mjs: not run, not a fail); Jam.Controls.HeldCrossCheck runs the same staging in the sim once the switch is 1.

### What the buttons do right now (the HUD's badge)
A pill bottom left: "LMB  SHOOT     RMB  PASS" with the puck, "LMB  CROSS-CHECK     RMB  CHECK" without, from the live bindings and the possession the
buttons read (JamControls::HasPuckForButtons) on the frame shown. `jam.hud.ButtonsBadge 0` hides it. Widest with long key names 446 of 480 units.

### CONTROLS from the pause card
The pause card's CONTROLS button opens the same screen (its third screen); BACK or Esc returns to the pause menu. The screen is 724 wide (the card's
inner 744).
