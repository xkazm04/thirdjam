# Lane A40: the chase camera stays outside the boards and reads the far net (device A, from H11.8, Fable 5.2)

You are lane A40 of the Third Jam (team Hokej). R-0030 point 3: every 20-second strip of the bake-off shows the chase camera inside the boards along the glass; the FEEL proposals
(feel.07, approved) asked for a board camera clamp. Read first: `docs/experiments/FEEL.md` (the camera findings: at the boards and puck readability beyond 10 m), `docs/experiments/
CAMERA-120M.md`, `Game/Source/JamGame/Private/HokejCamera*.cpp` (+ headers), `Game/Source/JamGame/Private/CameraProofScene.cpp` and `CameraTests.cpp`, `docs/experiments/FIGURE-BAKEOFF.md`
(the strips `docs/experiments/img/bakeoff/a30-*-strip.jpg` in worktree a30), `docs/experiments/EYES.md` (what the eye lands on), `tools/eyes/README.md`, R-0010 (90 x 45), R-0016 (rink
geometry rules), R-0021, R-0030.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a40` (the Director creates it at the current main; build it yourself, unity off; one editor on MCP port 8022 only
while proving; at most three editors on the machine). Run no git command; never touch `Game/Plugins/JamAssets`; the camera is presentation: the sim hash never changes.

You own: `HokejCamera*.cpp/.h`, `CameraProofScene.cpp`, `CameraTests.cpp`, `game.camera` keys in `schema/` (`boards_margin_m`, `end_height_m`, `far_net_bias`), `docs/experiments/CAMERA.md`,
`status/A40.md`.

## Job
1. **Clamp**: the camera never crosses the boards' inner face minus `boards_margin_m` (0.8) on any side; near the side boards it rises and tilts rather than clips; at the end boards
   it lifts to `end_height_m` and looks along the rink so the far net stays in frame; the transition is a critically damped spring in sim frames (no delta time), no pops.
2. **Far-net bias**: when the carrier skates toward the far net, the framing leads toward the net by `far_net_bias` (0..1) so the net and the slot stay visible at 24 m and beyond
   (today the puck reads 2 px at 24 m from camera height).
3. **Proof**: the bake-off's three frozen frames and a 20-second bot-match strip before and after (same seed, same frames), with the eyes `bug-hunt` and `readability` presets;
   `Jam.Camera.*` tests for the clamp (a scripted skater hugging each board; the camera position never inside) and the far-net bias; the camera proof scene updated.
4. `docs/experiments/CAMERA.md`: the rules, the keys with defaults, before/after stills, and what remains (a replay camera, the Director's Call framing).

Report: the still pairs, the tests, the file list. Time box 75 minutes.
