# Lane A164, track U2 (the menu), evolution 4 (device A, Opus)

You are lane A164 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then R-0056 to R-0061 at the end of `contracts/RULINGS.md` (R-0059 point 1 is the level wire), the track's `docs/experiments/MENU.md` and `status/A144.md`, and the
window lane's findings of tonight: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a108\status\A155.md` (read-only: rows 2 and the `LEVEL FIX VERIFIED` section).

Your worktree and the commit it stands on are named in the Director's message that starts you: the track's worktree moved to the newest main. It was built
long ago: expect a longer incremental build. Your status file: `status/A164.md`. Port 4764. Family as the track's: `MenuWidget.cpp/.h`, `MenuFlow.cpp/.h`,
`SettingsWidget.cpp/.h`, the menu's tests, `docs/experiments/MENU.md`. Presentation and settings only: no sim state.
**You do not own the cursor** (lane A155 has it): prove headless with the menu's QA verbs and tests; A155 or the release chain re-reads it in a window.

## Evolution 4: the first screen the operator touches at 10:00 has no dead key
What happened tonight, for context: the menu's PLAY NOW built the match twice in a real window and the second build dropped the player's level and the
look (fixed on main by the Director: `UJamMenuFlow::StartMatchAtLevel` takes back the Designer's scheduled apply; verified in a window). The headless
tests could not see it. Two smaller faults were found in the same window session and are yours:
1. **Return after a mouse click on a level segment starts nothing** (seen twice): the click moves keyboard focus off PLAY NOW. Ruling: after a click on
   ROOKIE / PRO / ALL-STAR the focus returns to PLAY NOW, so "click PRO, press Return" plays pro; Return and the pad's A on the first screen always start
   the match unless a text box has the focus. A test that clicks a segment by the widget's own click path (not `SetLevel` directly) and then sends Return.
2. **A level picked in the uncooked game was saved without `DifficultyChosen=True`** and the next start played rookie (the package wrote True; the editor's
   profile is shared with other sessions, so it is unproven): read the save path: every way a level can be picked (the segment's click, the settings
   screen, a pad press, `jam.menuscreen level`) writes both keys in one save, and a start reads them back the same in the editor game and in a package.
   Say whether the editor case was real, with the line of code.
3. **The walk is blind in a window that is not the active one** (`jam.menuscreen walk` read `verdict=blind`, `reached=0` of 76 with the game window behind
   another program's): make the walk's hit test independent of the window being active, or say why it cannot be (`MenuWidget.cpp` near 1133 to 1145 uses the
   tick geometry's absolute position), so the window lane can prove the pad walk without the foreground.
4. **`docs/experiments/MENU.md` section 3's coordinates are stale** (PLAY NOW is at 960,472 on a 1920 x 1080 client, the doc says 528; the season 635,
   CHOOSE 702, CONTROLS 900): regenerate them from the widget's own geometry by a QA verb that prints them, so no driver hard-codes a number.
5. One line for the operator's sheet on what the first screen does (keys, click, pad).

Write `MENU READY` in the status file with the file list as soon as items 1 and 2 are green: the Director lands it for the 09:00 package if it arrives
by 06:15; after that line change no source file without first writing `SOURCE OPEN`. `Jam.Menu.*` and the full `Jam` filter green. `-Multiprocess`.
Time box 80 minutes.
