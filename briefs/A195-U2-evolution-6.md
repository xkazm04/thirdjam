# Lane A195, track U2 (the front end), evolution 6 (device A, Opus)

You are lane A195 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then **R-0064** at the end of `contracts/RULINGS.md` (the operator's words after playing; point 3 is the track's charter), the track's own
`status/A187.md` and `docs/experiments/MENU-2.md` (the front end as it landed in 13e36667: landing, main, modes cards, the style table),
`handoff/A-to-B-0027.md` point 3 with device B's answer, and `handoff/A-to-B-0028.md` (the names device B delivers for the 18:00 merge).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A195.md`. Port 4795. Family:
`MenuWidget.cpp/.h`, `MenuFlow.cpp/.h`, `MenuGameMode.cpp`, `SettingsWidget.cpp/.h` (the controls lane has landed: it is yours for style only: do not
change what a binding does), the pause and end-of-match screens (find them: `Pause*`, `Results*`, the HUD's end card), the menu's tests,
`docs/experiments/MENU-3.md`. Not yours: `HUDWidget.cpp`'s in-play elements. Never `Game/Plugins/JamAssets` (device B's). You do not own the cursor: off
screen with `tools/qa/screen/u2-stills.ps1`.

## Evolution 6: every screen a player meets is one product
His words: "Menu HUD is a mess ... try to create profi NHL-like menu UI and landing experience". No league, team or broadcaster name, logo or likeness:
a standard of finish, not a look to copy. The front end's structure is on main; what is left is everything around it.
1. **Walk it as a stranger** (30 minutes, off screen, stills at 1920 x 1080): landing, main, PLAY's level switch, SEASON, MODES, MAKE YOUR OWN, CONTROLS
   (the rebinding screen), SETTINGS, PLAY WITH A FRIEND (host / join; **the hosting still shows the OS user name and LAN addresses: it stays in your
   worktree, never in a doc or the report**), the pause screen in a match, the end-of-match card, the season's between-takes card. One table: the screen,
   does it use the style table, what an eye trips on (type sizes that differ, a panel with no margin, text clipped, a button with no focus state, a
   screen with no header or footer hints, old "cut" / "direct" words still shown).
2. **Bring every one of them into the style table**: the pause screen, the end card, the season card, SETTINGS and CONTROLS: same header, footer hints
   from the live bindings, focus bar, spacing; words: no "cut", "take" only where the season means it, no "Director" outside MAKE YOUR OWN.
   `Jam.Menu.FitsTheScreen` extended to each at 1280 x 720, 1920 x 1080, 2560 x 1440; the no-dead-key rule on each.
3. **The backdrop**: `/JamAssets/UI/T_UI_MenuBackdrop` by name when it exists (it arrives with the 18:00 merge), behind the scrim; until then the best
   A can do without an asset: the placeholder's gradient with the rink's two accents, a slow drift. The live arena behind the menu (`L_MenuScene`) only
   if the first PLAY does not hitch more than today (say the number); otherwise leave it and say so.
4. **Device B's kit**: when the Director tells you dev-b's UI push is on main, check each name of A-to-B-0028 really shows (a still each), fix the
   nine-slice margins and font metrics the real assets need, and tell the Director the cook line the packaged game needs
   (`+DirectoriesToAlwaysCook=(Path="/JamAssets/UI")` goes into `Game/Config/DefaultGame.ini`: the Director adds it: say if more paths are needed).
5. **What must not break**: PLAY by Enter, Enter from the landing plays the lit level in ONE build; `jam.menuscreen where | click | key | walk | play`
   name every new control; `tools/qa/accept/release-menu.ps1` still passes its presses (read it; tell the Director any row that must change).

Write `SCREENS READY` with the file list and ten stills when items 1, 2 and 5 are closed, built, `Jam.Menu.*` and the full `Jam` filter green, Shipping
compiles: **wanted by 17:15 for the 18:30 package**; `KIT READY` after item 4 (by 19:15: it lands as a fault fix inside the freeze only if it is
presentation alone); `SOURCE OPEN` before any later source change. `-Multiprocess`; stop only processes whose ids you recorded. Time box 240 minutes.
