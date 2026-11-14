# The front end, evolution 6: every screen a player meets is one product (lane A195, track U2, Sat 14 Nov)

R-0064 point 3, the operator: "Menu HUD is a mess ... try to create profi NHL-like menu UI and landing experience". Evolution 5 (`MENU-2.md`, lane A187)
built the front end's structure and its style table (`FJamMenuLook`); everything a player meets around it still wore the film slate of lanes A8 and A18
(the clapper, TIME OUT, DIRECTOR'S CALL, yellow chalk pill buttons) or the slate kit inside the new panel (SETTINGS, CONTROLS). This evolution draws those
screens from the same table. No league, team or broadcaster name, logo or likeness: a standard of finish, not a look to copy.

## 1. The walk as a stranger (main 7d944ee1, off screen, 1920 x 1080)
| screen | style table before | what an eye tripped on | now |
|---|---|---|---|
| landing, HOME, PLAY's level switch, MODES, ALL RINKS, MAKE YOUR OWN | yes (A187) | the backdrop's two strokes are flat bands | the backdrop: a gradient (arena blue into ink) and the two strokes drifting (item 3) |
| SEASON | yes | nothing; the take titles ("ROUGH CUT", "THE FINAL CUT") are the season's own names: "take" is the season's meaning and stays | unchanged |
| SETTINGS | half | the slate kit inside the panel: yellow headings with chalk icons, chalk pill buttons, "CUT'S OWN", "the cuts", "the Designer directs live cuts", "key: none (cached cuts only)", a CONTROLS line naming a path that is gone | the table: white heads, dim captions, items with the focus fill and bar; "EACH RINK'S OWN", "MODE'S OWN", "AI LEVEL", "MAKE YOUR OWN writes new games live with it" |
| CONTROLS (A183's rebinding screen) | half | a second "CONTROLS" title in yellow under the header's, yellow verbs, chalk pill cells | "REBIND ANY KEY", white verbs, cells and RESET / BACK in the item look, the listening veil in the table's scrim |
| PLAY WITH A FRIEND | no | a grey modal of its own, "pick the cut", the raw id `hockey.regular` | NOT this family (JamNet's lobby): finding, next evolution 1 |
| pause | no | the film slate: clapper band, megaphone, TIME OUT, DIRECTOR'S CALL, yellow pills, a lower-case key line | PAUSED on a panel with the team rule, RESUME / CHANGE A RULE / RESTART / CONTROLS / QUIT TO MENU as items, the key hints as caps |
| end of match | no | a slate plate with a clapper, yellow YOU WIN, a fixed "R  REMATCH      ESC  MENU" though R is rebindable | a panel: the word at 72, the score, the keys as caps with the restart key from the bindings ("R / VIEW") |
| season between-takes | no | the same end card, "R  NEXT MATCH: <TAKE>" fixed | the same panel; "R / VIEW  NEXT MATCH: THE BIG ROOM", "SEASON COMPLETE" as the card's own line |

## 2. The style table outside the menu (`FJamMenuLook`, `FJamMenuPainter`, `MenuWidget.h`)
- `ItemStyle(State, bSegment, Pad)`, `FillBrush`, `KitOr`, `PanelBrush`: the menu's item look, now public (the menu's own call sites forward to them).
- `MakeItem` (label + the focus bar), `MakePanel` (the team rule over the panel), `MakeTeamRule`, `MakeText`, `FillHints` / `HintsLine` (the footer's key
  caps), `RestartKeyCap()` (the restart verb's key and pad button from the live bindings).
- `FJamMenuPainter`: a widget outside the menu adds its buttons; every tick the focused one gets the orange fill and the white bar, the rest the panel
  fill, a disabled one the dim fill; the hovered item takes the focus (one item lit; not while a key is typed or a CONTROLS cell listens).
- Used by: the pause card and the rule card (`DirectorsCallWidget.cpp`), the end card (`HUDWidget.cpp`, the card only), SETTINGS and CONTROLS
  (`SettingsWidget.cpp`; the controls lane's bindings logic untouched).

## 3. The words
No "cut" as a word on any screen of this family (labels, captions, the pause card's level line "THIS MODE'S OWN"; the season's take titles ROUGH CUT and THE FINAL CUT are titles and stay); "take" only on SEASON and its cards;
"Director" only in MAKE YOUR OWN and the credits' one line naming the jam's theme ("2026 Epic MegaJam, theme Director's Cut": a fact, kept).
The pause card's CHANGE A RULE is lane A8's Director's Call (one change per period, unchanged); after a call the card says "RULE CHANGED: ...".
Left as they are (not this family): the scoreboard's "DIRECTOR'S CALL: ..." line and the in-play "ESC / START   CALL READY" pill
(`UJamDirectorsCallHudWidget`, pinned by `Jam.Menu.DirectorsCallOncePerPeriod`).

## 4. The backdrop (item 3)
`/JamAssets/UI/T_UI_MenuBackdrop` is drawn by name under the 0.62 scrim the moment it is in the plugin (A187's slot, unchanged). Until then: a vertical
gradient from `#172034` to the ink in 18 bands (one or two levels a step), and the two team strokes drifting 40 units in opposite phase over 24 s.
The live arena (`L_MenuScene`) is NOT opened: the preload's warm scene (EffectsBinder.cpp, lane A150) stands the rig, two skaters and the puck in
front of the menu camera behind an opaque menu; a see-through menu would show it, and opening the arena behind the menu would move that warm scene
(another family). The first PLAY's hitch is therefore unchanged by this lane (no level, no asset is loaded at the menu).

## 5. What the drivers press (item 5)
- Nothing of the landing, HOME or PLAY NOW changed: the landing's Return then PLAY NOW's Return plays the lit level (`Jam.Menu.FrontEnd`,
  `Jam.Menu.ClickThenReturn`, `Jam.Menu.FirstMatchLevel` green); `tools/qa/accept/release-menu.ps1` needs no row changed (its presses: the
  landing's Return, `JAMMENU landing press`, PLAY NOW's Return or `jam.menuscreen where` + the click on PLAY NOW: all as they were).
- New names: `jam.menuscreen continue` (SEASON's CONTINUE from any screen), `jam.pausemenu rule` (the rule card; `call` still works) and
  `jam.pausemenu controls` (the pause card's CONTROLS). The pause card's items keep their actions (RESUME, the rule card, RESTART, CONTROLS, QUIT).
- No dead key on the pause card: the focus never rests on the card itself (it goes home to RESUME, or to the first slider on the rule card); an Enter
  or pad A that reaches the card itself resumes. SETTINGS and CONTROLS live inside the menu, whose own rule (A164, A187) sends the focus home.
- `tools/qa/screen/u2-stills.ps1`: `-EndCard -StartCmd "jam.menuscreen continue"` takes the season's between-takes card; `-Level -LevelSettleSec 12`
  waits before the first still of a match; `-Level` also takes the rule card and the pause CONTROLS; the end-card wait
  counts its lines from a shared read (the Select-String count failed its compare and timed every end-card run out).

## 6. A fault of this machine, not of this change
Off-screen (`-RenderOffScreen`) game processes die at the first frames of a match, now and then, of a D3D12 page fault in the sky light's
`ComputeSingleAverageBrightnessFromCubemap` (exit code 3). The same crash is in lane A187's `u2-a187ret` / `u2-a187click` logs (12:06) and lane
f2b's `a178b-free-m0` (06:39). Here: 3 of 8 match starts in this lane's off-screen runs (two `-Level`, one season `-EndCard`), the baseline binary's two among the five that lived; the crash comes at the match's first frames whatever the stills do (`-LevelSettleSec` does not prevent it; a crashed run is re-run).
A windowed package is not known to be touched by it; the release chain's runs are windowed.

## 7. Proof (main 7d944ee1 + this lane, editor build, off screen)
- `Jam.Menu` + `Jam.Controls`: 30 of 30 (the new `Jam.Menu.EveryScreenOneLook`; `Jam.Menu.FitsTheScreen` extended); the full `Jam` filter 293 of 293
  on the final source (`Game/logs/a195/tests-jam3.log`). No sim, brain or state field touched: the `Jam.Net` hash tests are in the 293.
- `FitsTheScreen`, in 1920 x 1080 units: the project's DPI curve gives 1280 x 720 at 0.666 (1922 x 1081 units), 1920 x 1080 at 1.0, 2560 x 1440 at
  1.333 (1920 x 1080): one layout for the three sizes, now held from the curve. Pause 808 x 707, rule card 808 x 345, pause CONTROLS 808 x 872; the
  end card 704 wide (275 high; 312 with SEASON COMPLETE); each card's key row inside it. The test caught the between-takes card's keys at 584 of 576
  units with "THE FINAL CUT" (the card was 640): it is 704 now. Menu screens as before: settings 828, controls 978 (on the 880 panel), modes 1037 of 1080.
- `EveryScreenOneLook`: the pause items (RESUME, CHANGE A RULE, RESTART, CONTROLS, QUIT TO MENU), its hints ("ESC / START RESUME   ENTER / A SELECT
  R / VIEW RESTART"), the rule card's and the pause CONTROLS' hints, the end card's keys from the bindings (a restart rebound to T reads "T / VIEW"
  on the match's card and on the season's), no "cut" and no "Director" in the settings' labels or on the pause card.
- Off-screen keys (`u2-stills -Switch`, `tools/qa/screen/out/u2-a195fw`): right, right on PLAY NOW, Enter: `JAMMATCH level=pro source=settings`.
- Stills: `docs/experiments/menu-3/` (ten: landing, HOME, SETTINGS, CONTROLS, pause, rule card, pause at 720, end card won and beaten, the season's
  between-takes card).
- Not proven here (no cursor, no window): the mouse moving the focus on the pause card and SETTINGS, the pad walk's directions; the hosting lobby
  still stays in the worktree.

## 8. Device B's kit (item 4) and the cook
Every name of A-to-B-0028 is read by name with a fallback (`FJamMenuLook::Kit`, `KitOr`, `Font`): the panel, the item plates (`T_UI_Button_*`,
`T_UI_Segment_*`) and the font now dress the pause card, the end card, SETTINGS and CONTROLS too, with no line of code more. The cook needs one line:
`+DirectoriesToAlwaysCook=(Path="/JamAssets/UI")` in `Game/Config/DefaultGame.ini` (it holds `UI/Fonts` and `UI/Modes`); `/JamAssets/Maps` is not
needed (the menu scene is not opened). The nine-slice margins (0.2 x 0.35 of the plate, drawn at half its pixels) and the font's faces are checked
against the real assets when they are on main (KIT READY).

## 9. Findings outside this family
- `Game/Source/JamNet/Private/LobbyWidget.cpp` (PLAY WITH A FRIEND): a grey modal of its own; line 134 "pick the cut with < >", the raw cut id on
  screen. Wants the same makers (`FJamMenuLook::MakePanel`, `MakeItem`, `FJamMenuPainter`) and the mode's display name.
- `Game/Source/JamGame/Private/DirectorsCallWidget.cpp` line 111 (`AnnouncementLine`: "DIRECTOR'S CALL: ...", pinned by MenuTests.cpp ~171) and
  line 593 (the in-play pill "ESC / START   CALL READY", yellow): the scoreboard's words, in-play HUD (track U1).
- `Game/Source/JamGame/Private/Progression.cpp` lines 452-454 (`EndHint`): a fixed "R" in the season's key lines; the end card now shows the bound
  key in its place (`UJamHudWidget::EndCardHints`), the text itself is still fixed.
- The off-screen GPU fault of section 6 (`ComputeSingleAverageBrightnessFromCubemap`), three lanes so far.
