# Lane A106, track Q3: Q3-performance-five (device A, evolution 1, Opus)

You are lane A106 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full**
(how a track works, the machine's band and the cursor, the walls, the status file, the report with the next three evolutions). Then `docs/EVOLUTION.md` and `docs/VISION.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a106` (the Director created it at the commit that carries the whole vision: contact by inertia, the
movement laws, the hockey brain, the ladder of R-0046). Your status file: `status/A106.md`. Your remote-console port if you run a game: 4706.

## Track Q3: performance at five a side
Family: `EffectsBinder.*`, `HUDWidget.*` (tick cost only), the brain's cost in `JamAICore.cpp` (how often it computes, not what it decides), `Game/Config/DefaultEngine.ini`,
`tools/perf/**`, `docs/experiments/PROFILING-2.md`. Read `PROFILING.md`, `PERF-FIXES.md`, `FIVE.md` (lanes A58, A65, A83).
First evolution: **the frame at 5 v 5 with the vision.** Cursor-free: the CSV profiler and Insights traces on a Development package driven by the remote console on your own
port, bot matches only (no posted input, no cursor): l2_night and p3_clay with ten figures: game thread, render thread, GPU, hitches over 33 ms in play, the brain's ms per
frame (the space read, the roles), the sim's ms, the figure Blueprint's ms for ten, the HUD's; against A83's numbers. Fix the top offender inside your family (a stagger of
the space read across frames if it costs more than 0.3 ms; the HUD's ability slot and puck marker if they rebuild every frame) and measure again. Is the match-start frame
still under 100 ms? Report the budget table and the next three offenders.

Time box 90 minutes for this evolution. Interim in the status file at 40 minutes.
