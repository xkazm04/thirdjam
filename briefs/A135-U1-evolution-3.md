# Lane A135, track U1 (the screen), evolution 3 (device A, Fable)

You are lane A135 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, `docs/VISION.md`, and the track's doc and status: `docs/experiments/SCREEN.md` (parts 1 and 2), `status/A123.md`,
`tools/qa/screen/u1-stills.ps1` (the off-screen stills and the projection probe are yours to reuse).

Your worktree and the commit it stands on are named in the Director's message that starts you: a **fresh worktree at the newest main** (your two evolutions are on
it, with the whole game: the kept puck, the goal frames in clay, the arena that answers a goal). It has no binaries: build the editor first (builds no longer
queue across worktrees). Your status file: `status/A135.md`. Port 4735. Family: `CameraFollow.*` framing (not the hit's kick: track F1's), `HUDWidget.*` layout
and texts (not the contact line), `BarkWidget.*`, `docs/experiments/SCREEN.md`.

## Evolution 3: the camera never loses the player
Your own stills found it: on the 90 m rink the human is out of the frame at sim frame 4500 on both tiers and at the bottom edge at 1440; on 140 m he sits in a
corner of the frame at 2520 and 2700. A player who cannot see his own skater cannot stop, cut or aim.
1. **The human never leaves the frame**: 0 of 6,000 frames of the 100 s pro match with the human's chest outside the middle third of the frame, on 90 m and
   140 m, both tiers (measure with your projection probe, headless where you can). Without breaking what lane A64 measured: a camera yaw feeds back through
   the cursor's aim, so prefer position, height and pitch; the cursor's point on the ice stays true (`CursorAimRink`); no snap; the puck stays in frame when
   the human carries it, and the net he attacks comes into frame inside the blue line.
2. **The pill and the prompt nobody has seen**: stills of the CALL READY pill after a goal and of the prompt's lower slot (drive a goal by a drill or a console
   command rather than waiting for the scripted human to score).
3. **Bark text on a fixed lane**: 0 of 60 stills with a bubble on a net label, a ring or a figure, none within 100 px of the human's skates.
4. **Prompts in the player's own input**: the brake prompt names the bound key or the pad's control (a test over the two bindings).

Look at your stills before you report. The acceptance run is the cursor owner's: name the rows your camera change could move (the aim rows first).
The gate: the full `Jam` filter green; the hash unchanged (nothing here is sim); each rule behind a console variable that gives back the old screen.
Time box 100 minutes; interim in the status file.
