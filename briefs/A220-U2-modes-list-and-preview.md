# Lane A220, track U2 (the front end), Sunday: MODES is a list of names with a preview (device A, Opus)

You are lane A220 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
Then **R-0069** at the end of `contracts/RULINGS.md` (point 4), R-0064 (his order of a "profi" menu: a standard of finish, no marks copied),
the front end as it stands (`Game/Source/JamGame/Private/MenuWidget.cpp` with its style table `FJamMenuLook`, `MenuFlow.cpp/.h`, the MODES
screen and its cards, `Jam.Menu.*` tests, `status/A213.md` for what the night changed there: a Director's mode plays the level lit on HOME).

Your worktree is `.claude/worktrees/u2`, on main (the Director names the commit). Status file `status/A220.md`. Two heavy processes at most.
Nothing of yours runs from the main checkout. Off screen with `-dpcvars=r.RDG.AsyncCompute=0`; you do not own the cursor.

## His words (R-0069, verbatim)
"For modes selection the previous images are deformed, rather create list with titles on left side with mode names sorted asc, and preview on
right half of the screen."

## The screen
1. **Left: the list.** The modes' names, one a row, sorted ascending by the name shown (A to Z), in the front end's type and colours; the row
   under the cursor or the pad's focus is lit; the list scrolls if it is longer than the screen (it is about a dozen: fit them without a scroll
   at 1280 x 720 if the type stays readable, else scroll). Up / Down, W / S, the pad's stick and D-pad, the mouse's hover and wheel move it;
   Enter / A / a click starts the mode (as a card's click did); Esc / B goes back. The first row is selected when the screen opens.
2. **Right half: the preview of the selected mode.** Its picture shown WHOLE in its own proportions (letterboxed inside the half, never
   stretched or cropped: find why the cards deformed them: a brush sized to the card instead of the image, most likely), its name large, its
   one or two lines of what changes in the game, and the level it will play (the level lit on HOME: the night's rule). The preview changes the
   moment the selection moves (no fade longer than 6 frames).
3. Nothing else changes: HOME, SEASON, SETTINGS, the landing and what a mode launches stay as they are. The pictures are the ones the cards
   use (device B's art: you change no asset; if a mode has no picture, a plain panel in the look's colour with the mode's name, never a
   stretched neighbour).
4. It reads on 1280 x 720 and 1920 x 1080, and with a pad alone.

## Prove
Stills from the game at both resolutions: the screen as it opens, three different modes selected (the first, a middle one, the last), each
picture measured against its file's own proportions (say the numbers); a strip of the pad walking the list top to bottom; `Jam.Menu.*`
extended (the list is sorted ascending; every mode of the old cards is in it exactly once; the selected row's preview is that mode's; a
picture's drawn proportions equal its source's within 1 %; `Jam.Menu.FitsTheScreen` green); the release's menu row
(`tools/qa/accept/release-menu.ps1`) walks MODES: read it and keep its command (`jam.menuscreen ...`) working, or change the row with the
screen and say so; the full `Jam` filter in a cleared environment; Shipping compiles.

`SEEN` with the first stills of the new screen as soon as it draws (the Director looks at them himself); `MODES READY` with the files by
name, the stills and the gate **by 12:30 by the machine clock**. Keep the files closed after the line.
