# Lane A119, track F3 (the arena in Blender), evolution 2 (device A, Fable)

You are lane A119 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, `docs/VISION.md`, and the track's first evolution in your worktree: `status/A103.md`, `docs/experiments/ARENA.md`, `tools/blender/arena/**`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a103` (the track's: evolution 1 is committed there as the baseline and is landing; its editor is built).
Your status file: `status/A119.md`. Port 4719. Family: `Game/Content/Arena/**`, `tools/blender/arena/**`, `RinkActor.*` placement only, `docs/experiments/ARENA.md`,
`ASSETS.md` rows for your meshes.

## Evolution 2: the goal is the most looked-at thing on the ice
The puck, the net and the goalie are where the player's eye lives, and the goal frames are still code bars. Device B takes the crowd band, the scoreboard and the
stands (handoff A-to-B-0022); the goals and the pieces at ice level are yours.
1. **Goal frames in the clay voice**: a chunky, thumb-pressed frame with a net that reads as a net from the game camera, built by the parametric script to
   `game.rink.goal_width_m` / `goal_height_m` (the sim's goal volume is the truth: the frame's inner opening within 2 cm of it on all three rink sizes, on wide
   goals too), replacing the code bars on the tiers where your pieces show; `jam.rink.arena 0` restores the code frame. Under 3,000 triangles each. The net must not
   hide the puck or the goalie from the game camera (a still from the game camera's angle in Blender for both ends, and from the far end through the net).
2. **A scorer's table between the penalty boxes**, same voice, under 3,000 triangles.
3. **The pieces seen in the game**: stills from the game on `p3_clay` and `l2_night` at 90 x 45 and 120 x 60 with the benches, boxes and goals in frame. Use a game
   process of your own (windowed, off-screen position, your port, `jam.ctrl.Stills` or `HighResShot`; never the system cursor, never the acceptance run). If a
   still cannot be taken without the cursor, say so and queue it.
4. The cook line for `/Game/Arena` and the ownership entry are landing with evolution 1: do not edit `DefaultGame.ini` or `ownership.json`.

Proof: the import and placement checks you built (IMPORT PASS, VERIFY PASS on three rink sizes), the ledger check, the full `Jam` filter green, the hash unchanged
(nothing here is sim), eight stills or fewer. For device B, in your report: every material you reference by name, and where the night rig's snow banks still cross
your pieces. Time box 90 minutes; interim in the status file.
