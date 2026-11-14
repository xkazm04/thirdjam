# Lane A187, track U2 (the front end), evolution 5 (device A, Opus)

You are lane A187 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then **R-0064** at the end of `contracts/RULINGS.md` (the operator's words after playing package H31; point 3 is your charter), `handoff/A-to-B-0027.md` point 3
(what device B draws for you today), the track's `docs/experiments/MENU.md` and `status/A164.md`, `status/A176.md`, `MenuWidget.cpp/.h`, `MenuFlow.cpp/.h`,
`MenuGameMode.cpp`, and the window lane's `docs/experiments/ACCEPT-WINDOW.md` (the drivers that press the menu: they read places from
`jam.menuscreen where`).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A187.md`. Port 4787. Family:
`MenuWidget.cpp/.h`, `MenuFlow.cpp/.h`, `MenuGameMode.cpp`, the menu's tests, `docs/experiments/MENU-2.md`. Not yours today: `SettingsWidget.*` (the
controls lane A183 builds the rebinding screen there: your CONTROLS entry opens it), `HUDWidget.*`. You do not own the cursor: off screen with
`tools/qa/screen/u2-stills.ps1`.

## Evolution 5: a front end that looks like a game a studio shipped
His words: "Menu HUD is a mess, leave concept of director's cut and try to create profi NHL-like menu UI and landing experience - you can use Device B
for that". The ruling: the menu stops presenting the game as cuts to direct. **No league, team or broadcaster name, logo or likeness: "NHL-like" is a
standard of finish, not a look to copy.**
1. **The structure** (build it first, with plain placeholders, so device B's art drops in by name):
   - **Landing**: the game's name large, one line under it, "PRESS ENTER" (any key, A, a click); behind it the arena (see 3).
   - **Main**: PLAY (one match: the ROOKIE / PRO / ALL-STAR switch and its line stay with it), SEASON (the eight takes: where he is, the next one),
     MODES (the twelve as a grid of cards: name, one line, players a side, a picture slot), CONTROLS, SETTINGS, QUIT. PLAY WITH A FRIEND where it is today.
   - **Gone from the front end**: DIRECT A CUT as a top entry, the cuts list as the first thing, the Designer's sentence box and chips. The Designer
     stays reachable as one card at the end of MODES ("MAKE YOUR OWN": opens today's screen unchanged) so nothing is deleted and the tests that use it hold.
   - Every screen: a header (where am I), a footer with the live key hints (Enter / A select, Esc / B back, the pad's glyph names as text), one focused
     element always (the no-dead-key rule of A164 on every screen: a click on empty space never leaves a screen without focus), Back from everywhere.
2. **The finish, in code**: one style table (type sizes, weights, the two team accents, panel and focus colours, spacing on an 8-unit grid) used by
   every screen: nothing styled in place; a focus state an eye finds in a third of a second from two metres (a filled accent bar, not a thin outline);
   transitions under 200 ms; the layout safe at 1280 x 720, 1920 x 1080 and 2560 x 1440 (`Jam.Menu.FitsTheScreen` extended to the new screens).
3. **The arena behind the menu**: until device B's menu scene arrives, open the clay rink map behind the menu with a slow fixed camera drift and no
   skaters (if the match's preload already loads the rig at the menu, reuse it: read `jam.PreloadRig` and lane A150's notes in `PROFILING-6.md`; the
   first PLAY must not hitch more than today: say the number), dimmed by a scrim so the type reads.
4. **What must not break**: PLAY NOW by one Enter from the main screen plays the lit level in ONE build (`UJamMenuFlow::StartMatchAtLevel`: do not touch
   its take-back of the Designer's apply); `jam.menuscreen where | click | key | walk | play` keep working and name the new buttons; a launch with
   `-cut=` still goes straight where the acceptance run expects (read `ACCEPT.md` section on the menu: the release chain's menu row presses Return on
   the first screen: with a landing screen in front, the first Return must still start the match or the row must be told: keep "Return on the first
   screen plays" by making the landing's Enter go to Main with PLAY focused and a second Enter play, AND keep `jam.menuscreen play` doing both: write
   the exact change the drivers need in your report: the Director edits the release rows).
5. **Device B's kit by name**: textures and a font from `/JamAssets/UI/...` when they exist (a soft reference with a fallback to the placeholder, so
   the build never depends on B's push); list the names you look for in the status file by 13:30 so the Director sends them to B.

Write `MENU READY` with the file list and six off-screen stills (landing, main, the level switch, modes, season, controls entry) when items 1, 2 and 4
are closed, built, `Jam.Menu.*` and the full `Jam` filter green, Shipping compiles: **wanted by 17:15 for the 18:30 package**; an early
`MENU STRUCTURE READY` (item 1 with placeholders, everything of item 4 holding) by 15:00 if it is clean enough to ship in the 15:30 package: say honestly
whether it is better than today's menu. `-Multiprocess`; stop only processes whose ids you recorded. Time box 240 minutes.
