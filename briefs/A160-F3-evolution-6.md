# Lane A160, track F3 (the arena), evolution 6 (device A, Fable)

You are lane A160 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then R-0054 and R-0056 to R-0061 in `contracts/RULINGS.md`, the track's `docs/experiments/ARENA.md`, `RINK.md`, `status/A145.md`, and device B's
`git show origin/dev-b:handoff/B-to-A-0041.md` (points 4 and 6; the only git command you may run, with `git show origin/dev-b:PATH` for a still it names).

Your worktree and the commit it stands on are named in the Director's message that starts you: the track's worktree moved to the newest main. It was built
before: an incremental build is enough. Your status file: `status/A160.md`. Port 4760. Family as the track's (the list in `briefs/A145-F3-evolution-5.md`): the
rink's own actors and their reactions on the A side, `tools/blender/arena/**`, `docs/experiments/ARENA-2.md`. Presentation only: no sim state. Never
`Game/Plugins/JamAssets` (device B's). **You do not own the cursor**: capture off screen with the track's driver.

## Evolution 6: the rink the operator sees at 10:00 has no dead props
1. **The penalty boxes' clock faces are black on clay** (B's critic): a face that reads on the clay look, or no face; the same check on every prop the A
   side places on clay: a still of each from the game camera, at the faceoff and from each corner.
2. **The box seen from the ice** (the track's own next): in the one mode that whistles (Glass House) the boxed skater is in the box, seen from the game
   camera; outside that mode the boxes are furniture and say nothing.
3. **Round caps on every rink** (the regular, the large, the pond): the boards' ends and the goal frames' joints closed from the game camera's angles;
   a still per rink.
4. **The goal is a payoff at the net** (B's point 6: B's effect `NS_goal` carries the moment; the light stays at B's numbers): the A side's part: the lamp,
   the net's bulge and the boards' answer are in time with the effect's start (one `JAMRINK react goal` line a goal; frames between the sim's goal and each
   reaction in a table).
5. Twelve stills from the game camera on this main for the Director's page (clay, 1920 x 1080 off screen): look at each; a fault is a finding with its file.

Write `ARENA READY` in the status file with the file list as soon as items 1 and 3 are green: the Director lands them for the 09:00 package. The full `Jam`
filter green. `-Multiprocess`. Time box 100 minutes.
