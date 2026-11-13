# Lane A114, track Q3 (performance at five a side), evolution 2 (device A, Opus)

You are lane A114 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, and the track's first evolution in your worktree: `status/A106.md` and `docs/experiments/PROFILING-2.md` (its tools are yours:
`tools/perf/bot.ps1` on port 4706, `five.mjs`, `syncrows.mjs`, the package `Game/Packaged/A106-dev`).

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a106` (the track's worktree: evolution 1 is committed there as the baseline and is landing on main; its
editor is built). Your status file: `status/A114.md`. Port 4706 (the track's). Family as the track's (`briefs/A106-*.md`), plus the named lines below.

## Evolution 2: the match starts without a hitch at five a side
Lane A106 measured the match-start frame at 96 to 191 ms (under 100 ms in 2 of 6 runs), the first lit frames at 33 to 84 ms, and found synchronous lookups in play:
1. **The match-start frame under 100 ms in 6 of 6 runs at 5 v 5, both tiers**: the Rink map's package loaded asynchronously from the menu (it loads synchronously
   today: 15 to 88 ms), the rink's build and the ten figures' spawn spread off the load frame where they can be (never changing the sim's first frame or its hash).
2. **No synchronous lookup during play**: (a) a commentary line whose wave does not exist is skipped when the line is picked, not looked up and played as silence
   (`CommentarySystem.cpp` near line 709, `LoadWave`: this one condition is yours; the rest of the file is track Q2's; list the missing waves for device B in your
   report); (b) the two absent assets asked for at every match start (`/Game/UI/T_UI_ScorePlate`, the `MS_crowd_bed` request near `CommentarySystem.cpp:766`): ask
   once per process, or not at all. Proof: `syncrows.mjs` on a two-minute match: zero synchronous load rows after the first second.
3. **The native tournament build compiles again**: `-DJAMCORE_API=` at `tools/tournament/run.mjs:74` (your finding 3), with its node test.
4. If time remains: the first 60 frames after the load frame, none over 33 ms (a GPU Insights trace of the first second; pipeline-state precaching, the skin cache
   for ten skeletal meshes). Otherwise it is your first "next".
The figure Blueprint's 0.59 ms a frame has gone to device B as a question (handoff A-to-B-0023): do not change `UpdateMotion`'s call site.

The gate: the full `Jam` filter green, the hash unchanged. Runs are cursor-free through your driver. Time box 90 minutes; interim in the status file.
