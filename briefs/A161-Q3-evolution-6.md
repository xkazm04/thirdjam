# Lane A161, track Q3 (performance and size), evolution 6 (device A, Opus)

You are lane A161 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then R-0056 to R-0061 in `contracts/RULINGS.md`, the track's `docs/experiments/PROFILING-6.md`, `SIZE.md`, `status/A150.md`, and device B's two handoffs in your
worktree: `handoff/B-to-A-0040.md` and `handoff/B-to-A-0041.md` (points 5 and 7), with `status/B.md` and `status/EYE-report.md`.

Your worktree and the commit it stands on are named in the Director's message that starts you. **It is special: the Director merged device B's branch
(`origin/dev-b`, 25 commits: timber stands and a clay crowd of 293 actors on the clay rig, effects at game size, new sounds) into it on a detached HEAD. That
merge is not on main yet: you are the one who says whether it is safe for the package the operator plays at 10:00.** It was built before: an incremental
build, then the content your runs need. Your status file: `status/A161.md`. Port 4761. Family: `Game/Config/**` packaging and cook settings, `tools/perf/**`,
`tools/size/**`, `docs/experiments/PROFILING-7.md`. Never `Game/Plugins/JamAssets` (B's): a fault there is a finding for B.
**You do not own the cursor or the foreground**: off-screen runs only; the machine is loaded (other lanes measure beside you), so every timing is a paired
A/B in the same minute, said as a ratio, never an absolute.

## Evolution 6: is device B's night safe to ship at 10:00, and half its weight out of the package
1. **The merge builds, the full `Jam` filter is green on it, and a cook-only of the shipped maps has no error** (first, before anything else): write
   `MERGE SAFE` or `MERGE UNSAFE: <what>` in the status file at once.
2. **The stands and the crowd on clay** (B asked: "please time them"): the clay match off screen with and without them (B's own switch if it has one; say
   which), game thread and GPU per frame at the faceoff, in a corner and at a goal, paired; the frame budget of `PROFILING-6.md` holds or say by how much it
   is missed and the one switch that buys it back (the far crowd alone, the stands without the crowd).
3. **183 MB of figures no shipped cut selects** (B's point 5: the `generated_b2` family, its generated meshes, the P4 lab): prove by reference walk that
   only test cuts and tests reach them; `DirectoriesToNeverCook` (or the project's own exclusion) for those folders; the tests and the fuzz sweep that name
   them skip cleanly in a package and still run in the editor; a cook-only before and after with the size by folder. No deletion.
4. **B's five timings** (point 7 a to e), each as a paired ratio on this loaded machine, or "not measurable tonight" with why: the Director sends B the table.
5. **The first match starts without a hitch on the merged tree** (your own A150 measurement repeated: the first 60 frames).

Write `SIZE READY` with the file list when item 3 is green. The gate: the full `Jam` filter green. `-Multiprocess`. Time box 110 minutes.
