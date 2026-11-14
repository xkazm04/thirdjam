# Lane A199, track K1 (controls), evolution 3 (device A, Opus)

You are lane A199 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then **R-0064** at the end of `contracts/RULINGS.md` (the operator's words after playing; point 1 is the track's charter), the track's `status/A183.md` and
`status/A191.md` with `docs/experiments/CONTROLS.md` (the buttons as they are on main: left mouse shoots with the puck and cross-checks without, right
mouse passes with the puck (held: saucer) and body-checks without, the held cross-check, rebinding, the still-hand aim anchor, the buttons' badge),
the contact lane's `status/A192.md` (the wind-ups; the held cross-check's rules: from the 10th frame held), and `docs/PLAY-ME.md`'s Controls section.

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A199.md`. Port 4799. Family:
`InputMapping.*`, the player controller's input code, the bindings' file and its tests, the rebinding rows of `SettingsWidget.cpp` (the front end lane
restyles that screen today: you change what a row does, not how it looks: say each function you touch), the buttons' badge, `ControlsTests.cpp`,
`docs/experiments/CONTROLS-2.md`. You do not own the cursor: headless and off screen.

## Evolution 3: the buttons do what a hand expects, on mouse and on pad
His words: "Possibility to rebind needs to be there ... Cross check on Left mouse, available only if player not on puck, if yes then shoot ... Right
mouse if on puck: Pass. Hold for saucer pass ... Crosscheck should be holdable so player with speed momentum can run with held crosscheck into another
player". The first cut of all of it is on main. This evolution closes what a hand finds in the first five minutes.
1. **The moment the puck changes hands** (measure first): he holds left mouse braced, takes the puck in the hit: does the held button shoot at once?
   He shoots, the puck leaves, the button is still down: does he cross-check? He is passed to with right mouse down (a body check called): does it
   pass back at once? The rule: **a button held across a change of possession does nothing until it is released and pressed again**, except the brace,
   which ends. Table before and after from scripted presses, per button; tests.
2. **Tap against hold** on both buttons: the thresholds in frames (a tap pass, a held saucer; a pressed cross-check, a held brace from the 10th frame):
   with the wind-ups on main a pressed cross-check's shove lands 4 frames after the press: does a 6 to 9 frame press read as a press? What a 12-frame
   "slow tap" does. Say the thresholds and why; one cvar each; the badge shows the hold building (a fill) if it is cheap.
3. **The pad**: every action of the mouse scheme has a pad button with the same possession rule; the default pad layout written in the rebinding
   screen and PLAY-ME; rebinding on the pad by pressing the new button; the footer's glyph names follow the live bindings.
4. **Rebinding's edges**: a key bound twice (the screen swaps and says so), Escape cancels a capture, a mouse button captured by a real press (not
   eaten by the click that opened the capture), RESET TO DEFAULTS with a confirm, the bindings file written atomically (temp file then rename) so a kill
   in the write never leaves half a file (the hardening lane fuzzes the file today: its faults in your family come to you through the Director).
5. **The still-hand anchor's release** (A191's note): the anchor lets go smoothly when he moves the mouse again: no jump of the aim on the first moved
   frame: the worst jump in degrees before and after on A186's camera logs.
6. PLAY-ME's Controls section rewritten for what is true at the end (the Director copies it: write it in `docs/experiments/CONTROLS-2.md`).

Write `CONTROLS READY 4` with the file list when items 1, 2 and 4 are closed, built, the full `Jam` filter green, Shipping compiles: **wanted by 17:00
for the 18:30 package**; `PAD READY` after item 3 (by 18:30: it lands before the freeze only if green); `SOURCE OPEN` before any later source change.
`-Multiprocess`; stop only processes whose ids you recorded. Time box 200 minutes.
