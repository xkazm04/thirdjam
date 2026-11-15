# Lane A222, track U3 (the HUD), Sunday: a controls panel that stays on screen all match, switchable off for good in SETTINGS (device A, Opus)

You are lane A222 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
Then R-0064, **R-0069 and R-0070** at the end of `contracts/RULINGS.md`; device B's relay of the operator's order, `handoff/B-to-A-0044.md`
(it is on branch `origin/dev-b` only: read the copy the Director put at `tools/qa/screen/a222/B-to-A-0044.md` in your worktree; it is a
proposal from B, the Director's reading below decides); the HUD as it stands (`Game/Source/JamGame/Private/HUDWidget.cpp`:
`ControlsHintFrames`, `bControlsHintSpent`, `jam.hud.HintEveryMatch`, `ButtonsBadgeLine`, the bottom-left pill; `Public/HUDWidget.h`;
`UJamInputMap` and `IsPadLast()`; `JamControls::HasPuckForButtons`); the front end's style (`MenuWidget.cpp` `FJamMenuLook`, `MenuStyle.h`)
and SETTINGS with its saved values (`FJamSettings`, `MenuFlow.*`, `status/A220.md` for how the last lane filmed and tested the menu).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A222.md`. Two heavy
processes at most. Off screen with `-dpcvars=r.RDG.AsyncCompute=0`; you do not own the cursor; nothing of yours runs from the main checkout.

## His words (15 Nov, about 10:50, to device B's session, verbatim)
"Improve visually panel to show key bindings for player and keep it visible throughout the game, switchable in settings to take off permanently"

## The Director's reading
1. **One panel, all match.** Bottom left above the safe margin, while play runs, in every match and mode; hidden under the pause card, the
   GOAL banner and the end card. It REPLACES the 8-second controls strip and the bottom-left pill (one legend, not three): when the panel is
   OFF none of the three shows (he switched the legend off for good). The commentary strip and anything else that stood above the old strip
   steps over the panel; never over the radar, the score or the boost / ability slot.
2. **Live and by possession**, from the bindings as they are bound now (rebinds, pad glyph names when the pad was touched last):
   with the puck: SHOOT (LMB), PASS (RMB), DEKE, SPIN; without: CROSS-CHECK (LMB), CHECK (RMB), POKE; always: SKATE, SPRINT, BRAKE, and
   ABILITY only while he holds one. Six to eight rows at most. The possession half swaps on the frame possession changes (a fade of at
   most 6 frames) under a small dot in his team's colour; the row whose button is down lights, and its word is the held verb's (the words
   the pill already swaps at a hold). No hold bars, no meters, no lines out of the panel onto the ice (R-0069 / R-0070).
3. **The look: the front end's own.** A dark translucent panel in the menu's colours and corner cut, each row a keycap (a light box with the
   key's short name in the menu's type, about 22 px tall at 1080) and the verb in caps beside it. At most about 300 px wide at 1920 x 1080
   and in proportion at 1280 x 720; readable at both; say the panel's box in pixels at both sizes and what of the ice it covers compared
   with the old pill and strip together. Drawn by code (no new asset; no art from device B today). No logos, no controller art, key names only.
4. **SETTINGS: a row `CONTROLS PANEL  ON / OFF`**, default ON, saved with the other settings, so OFF stays off across sessions and
   packages (the saved file is the operator's profile: your tests use a test profile path, never `%LOCALAPPDATA%\ThirdJam\Saved`). The
   change applies on the frame it is made, also from the pause card's settings if that screen is reachable in a match. The CONTROLS screen
   and the pause card keep the full table either way.

## Prove
Stills from the game at 1280 x 720 and 1920 x 1080: with the puck, without it, a button held, the pad's names, the panel OFF, the pause
card over it, a rebind shown; the swap filmed frame by frame once; SETTINGS with the new row at both sizes (`Jam.Menu.FitsTheScreen`
green). Tests `Jam.Hud.ControlsPanel*` (rows by possession; the pad's names; a rebind; OFF hides it and survives a save / load round trip;
hidden under pause / GOAL / end card; nothing else moved: the radar's, the score's and the slot's boxes do not intersect it at both sizes);
update the tests that pinned the 8-second strip and the pill (`ControlsTests.cpp`, `CameraTests.cpp`) to the new rule and say which. The
release's rows read the pill and the strip in places (`tools/qa/accept/*.ps1`: grep `ButtonsBadge`, `controls_hint`, `JAMHUD`): keep the
log lines those rows read, or change the row with the screen and say so. The full `Jam` filter in a cleared environment; Shipping compiles.

`SEEN` with the first stills as soon as the panel draws (the Director looks at them himself); `PANEL READY` with the files by name, the
stills and the gate **by 14:30 by the machine clock**. Keep the files closed after the line.
