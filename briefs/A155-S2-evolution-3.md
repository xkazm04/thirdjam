# Lane A155, track S2 (the acceptance matrix), evolution 3 (device A, Sonnet)

You are lane A155 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/experiments/ACCEPT.md` and `ACCEPT-MATRIX.md` (how the acceptance run drives the packaged game through the real cursor and buttons; the `machine`
verdict; the mode-aware table), `status/A131.md`, `docs/experiments/MENU.md` (section 7: what the acceptance run depends on in the menu; the buttons' places
and the QA verbs `jam.menuscreen walk | level | key`), `docs/experiments/CONTACT-5.md` (drill 22, the law-one drill), `MOVEMENT-5.md` (drill 21 by level),
`docs/PLAY-ME.md` (what the operator is told to try at 10:00), and the last lines of `status/A.md` in the **main checkout** (the newest `package` line names the
package you test).

Your worktree and the commit it stands on are named in the Director's message that starts you: the track's worktree moved to the newest main. You build
nothing of the game: you drive the packages the Director's release chain made in the main checkout: `Game/Packaged/Accept_H27` (Development: it has the
drills and the remote console) and `Game/Packaged/Hokej_Hokej_H27` (Shipping: what a player gets). Read them there; write only in your worktree. Your status
file: `status/A155.md`. Port 4708 (the track's). **You own the system cursor and the game windows until you report.** Family: `tools/qa/accept/**`, reports under
`docs/experiments/`, up to twelve stills under `submission/shots/` named `a155-*`.

## Evolution 3: the package the operator plays, looked at through a window
Everything landed tonight was proven headless or off screen. The operator plays at 10:00. This lane is the only eyes on the real window before him.
1. **The acceptance run on `Accept_H27`**, three times: the verdict and any `machine` repeat (the regular run is the release chain's gate: it should be green).
2. **A menu row** (new, in the driver): on a clean profile (rename the game's `Saved` folder aside and put it back after: only the package's own folder),
   without `-cut`: PLAY NOW by one Return and, in a second launch, by a real click: `JAMMATCH level=rookie source=settings` within 2 s of the press; then the
   switch under PLAY NOW: a click on PRO lights it; quit; start again: PRO is still lit and `[Jam.Settings]` has `DifficultyChosen=True`; then
   `jam.menuscreen walk` in the window (the pad walk the menu lane could not see off screen): its verdict line.
3. **The rows other lanes queued** on the Development package: "refused" (the human carries and presses RMB: the HUD's contact text or the pawn's
   `JAMVIS press ... result=` line), `jam.ctrl.Drill 22` with `jam.ctrl.LawHuman` 0 and 1 (a nudge, then a hit), `jam.ctrl.Drill 21` at rookie, pro and
   all_star (the stop passes), a goal (`jam.ctrl.EndMatch` or a drill): one `JAMRINK react goal` line a goal; the end card for a win and a loss.
4. **The matrix on tonight's modes**: regenerate the mode cuts from the newest main if the package's modes are newer than your cuts
   (`make-matrix-cuts.mjs`), run the twelve modes and the tiers once each with the automatic repeat; by-design rows go into `mode-table.json`.
5. **Twelve stills of the real window for the page and for the operator** (1920 x 1080 where the screen allows): the first screen with the switch, the twelve
   mode cards, the controls screen, the faceoff on clay with the YOU marker, a stop-and-go (drill 21: the frame he cuts), the nudge and the hit (drill 22),
   a board hit's flash, a goal with the lamp and the net, the end card, the pause card, a mode (Shotgun: F fired). Look at each: a still that shows a fault is
   a finding with its file.
6. **The Shipping zip, played once** (`Hokej_Hokej_H27`): unzip into a fresh folder under your worktree's `tools/qa/out/`, start it, press Return, play the
   first minute by posted keys or let the bots play, quit by the window's close: it starts, the first screen is right, a match runs, it exits clean; say the
   size of the zip and whether a firewall prompt appeared (never answer one: note it and go on).

Never kill a process you did not start; other lanes' headless editors run beside you: a run they disturbed is a `machine` verdict and is repeated. **The
Director's next release (H28) takes the cursor around 07:30**: be finished and reported before then. Write each row's verdict into the status file as it
completes. Time box 110 minutes; do not wait on a background watcher: poll inside one command.
