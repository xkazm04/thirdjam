# Lane A124, track Q3 (performance at five a side), evolution 3 (device A, Opus)

You are lane A124 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, and the track's two evolutions in your worktree: `status/A106.md`, `status/A114.md`, `docs/experiments/PROFILING-2.md`, `PROFILING-3.md` (its
tools are yours: `tools/perf/bot.ps1` on port 4706, `five.mjs`, `syncrows.mjs`, `loadrow.mjs`, the package `Game/Packaged/A114-after`).

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a106` (the track's: evolutions 1 and 2 are committed there as the baseline; its editor is built).
Your status file: `status/A124.md`. Port 4706 (the track's). Family as the track's: `EffectsBinder.*`, the match-start path in `HokejGameMode.*` / `RinkActor.*` /
`MenuFlow.*` (start-up order only, never the sim's first frame), `DefaultEngine.ini` rendering and loading settings, `tools/perf/**`, `docs/experiments/PROFILING-4.md`.

Know: track U1 (lane A111, landing now) made the first match wait for its light rig and finishes the rig's streaming inside `BuildMatch`
(`UWorld::FlushLevelStreaming`: a blocking load of 61 to 72 ms measured in the editor binary). That cost is now inside your load frame on main. Your worktree does
not have it: read `docs/experiments/SCREEN.md` in the main checkout and plan for it (the rig can be requested while the menu is up, as you did for the map).

## Evolution 3: the first second of a match
1. **The load frame under 100 ms in 6 of 6 runs at 5 v 5, both tiers** (3 of 6 today): stop the menu's music and sounds the frame before the map opens so the
   audio flush has nothing to wait for (24 to 56 ms); move the rink's build and the ten figures' spawn off the load frame where the sim's first frame and its hash
   allow (33 to 86 ms); request the light rig behind the menu.
2. **The first 60 frames after the load frame, none over 33 ms** (hitches of 35 to 183 ms today, render-thread bound, in both arms): a GPU trace of the first
   second; pipeline-state precaching; the skin cache for ten skeletal meshes; warm the effects the first faceoff uses.
3. Report the load frame and the first-60 table before and after in one session, six runs a tier, interleaved, as you did.

The figure Blueprint's 0.59 ms a frame is with device B (handoff A-to-B-0023): leave `UpdateMotion`'s call site.
The gate: the full `Jam` filter green, the hash unchanged; each change behind a console variable that gives back the old start. Time box 100 minutes; interim in the
status file.
