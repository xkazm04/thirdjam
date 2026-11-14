# Lane A158, track U1 (the screen), evolution 5 (device A, Fable)

You are lane A158 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then R-0056 to R-0061 at the end of `contracts/RULINGS.md`, the track's `docs/experiments/SCREEN.md`, `CAMERA.md`, `status/A135.md`, `docs/PLAY-ME.md`, and device
B's critic: `git show origin/dev-b:handoff/B-to-A-0041.md` and `git show origin/dev-b:status/EYE-report.md` (the only git commands you may run; the stills it
names are on that branch under `devlog/shots/`: `git show origin/dev-b:PATH > tools/qa/out/NAME.png` to look at one).

Your worktree and the commit it stands on are named in the Director's message that starts you: the track's worktree moved to the newest main. It was built
before: an incremental build is enough. Your status file: `status/A158.md`. Port 4758. Family as the track's (the list in `briefs/A135-U1-evolution-3.md`): the
HUD widget and its painters, the camera, the captions' placement, `tools/qa/screen/**`, `docs/experiments/SCREEN-2.md`. Presentation only: no sim state, no
cartridge rule, the lockstep hash untouched. **You do not own the cursor** (the acceptance lane has it until about 05:30): prove everything off screen with
the track's capture driver (`tools/qa/screen`), never in a foreground window.

## Evolution 5: nothing is drawn on top of the key feature
The operator named the fall, the lying and the get-up the key feature. B's critic found the HUD drawn over them. He plays at 10:00.
1. **No ring over a downed human**: the selection ring is not drawn while the human is down (the chevron stays); it returns with the get-up. The same for
   any marker drawn at a downed skater's feet.
2. **Captions leave the middle third**: the commentary caption sits where it never covers the human (the bottom edge with a margin, or the side away from
   him); taunt bubbles never over the net or the middle ice: anchor them to the speaker with a screen-edge clamp. Bar: in 20 captured knockdowns on clay no
   HUD element overlaps the downed figure's screen bounds for the length of the knockdown (measure it: project the figure's bounds and intersect).
3. **The camera's pitch in corners**: a cap so a corner pile-up is not seen from straight above (the boards and the bodies readable); the player still
   never leaves the frame (the track's own test holds) and no yaw comes back (322e6c5).
4. **Small things from the same list**, each a still before and after: the empty panel at top centre with the period text on its border; distance markers
   drawn on the goalie and the net when both are on screen; the lifted puck shown as a black post (a shadow or a ring on the ice instead).
5. Say which of the critic's eleven A-side faults were already gone on this main (the HUD and the load order changed since its stills).

Write `SCREEN READY` in the status file with the file list as soon as items 1 and 2 are green and captured: the Director lands them for the 09:00 package;
the rest follows. `Jam.Menu.FitsTheScreen` and the HUD's tests stay green; the full `Jam` filter green. `-Multiprocess`. Time box 100 minutes.
