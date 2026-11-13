# Lane A74: the off-screen puck marker and the far-net frame, without moving the camera (device A, from H15, Sonnet)

You are lane A74 of the Third Jam (team Hokej). Lane A64 fixed the eyes grid's faults (`docs/experiments/EYES-GRID-FIXES.md`) except the camera rows: the puck, the
carrier or the far net were out of frame in 19 of 47 stills, and A64's camera lead toward the puck fought the mouse aim (the cursor aims the stick, the camera's
turn moved the cursor's meaning), so it shipped off (`jam.camera.KeepYaw` 0). The Director's decision: the camera stays bound to the human as A49 made it; the HUD
points at what is off screen.

Read first: `docs/experiments/EYES-GRID-FIXES.md` rows 3 and 3b, `docs/experiments/CONTROLS.md` (A49: `CursorAimRink`, the camera bounds, why the camera must not
turn under the cursor), `CAMERA.md`, `UI.md` section 7 (HUD pass 2: the YOU ring, the net-distance diamonds, the radar), `Game/Source/JamGame/Private/{HUDWidget,
CameraFollow,HokejGameMode}.cpp`, `tools/qa/eyesgrid/shoot.ps1` (how A54 and A64 photographed the packaged build), `tools/eyes/README.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a74` (created at the current main; build it yourself, unity off; one editor on MCP port 8036 only while
proving; at most three editors on the machine). Run no git command; never touch `Game/Plugins/JamAssets`; never print `.env` (copy the main checkout's `.env` into the
worktree for the eyes tool without printing it; say so). Presentation only: the sim hash and the input path never change; `CameraFollow.cpp` is lane A73's at the same
time (the hit kick): touch only `jam.camera.KeepYaw`'s default if at all, nothing else there.

You own: `HUDWidget.*` (the marker), `docs/experiments/PUCK-MARKER.md`, `docs/experiments/img/a74/**`, `docs/experiments/eyes/a74-*.json`, `status/A74.md`.

## Job
1. **The puck marker**: when the puck (or its carrier) is outside the frame's inner 85 %, a small arrow on the frame's edge at the puck's direction, the distance in
   metres beside it, the team colour of the carrier (grey for a loose puck); it fades in over 6 frames and out over 6; never shown during the faceoff card, a goal
   pause or the pause menu; a `jam.hud.PuckMarker` cvar 0/1 (default 1).
2. **The far net**: when the human faces the far net and it is outside the frame, the net-distance diamond already points there: make sure it does (A64 moved the label
   beside the diamond) and that the diamond never sits on a board or a net (A54 row 10: 32 stills).
3. Proof: A64's package driver on a Development package of your worktree, the same cells (`hockey.regular` l2 card / faceoff / far net, `hockey.boost` p3, `hockey.
   arena.large` l2): stills before and after, eyes `bug-hunt` and `hud` on each; the 19 "not visible" findings should fall to the cases where the marker shows.
4. `PUCK-MARKER.md`: the rule, the stills, the eyes table before and after.

Report: the before/after counts, the still paths, the file list. Time box 60 minutes.
