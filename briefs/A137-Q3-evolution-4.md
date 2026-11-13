# Lane A137, track Q3 (performance at five a side), evolution 4 (device A, Opus)

You are lane A137 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, and the track's docs: `docs/experiments/PROFILING-2.md`, `PROFILING-3.md`, `PROFILING-4.md`, `status/A124.md`, the tools under
`tools/perf/` (`bot.ps1` the cursor-free driver, `ab.ps1` the interleaved A/B runner, `five.mjs`, `syncrows.mjs`, `loadrow.mjs`), and `docs/experiments/SCREEN.md`
(track U1 made the first match wait for its light rig and finishes the rig's streaming inside `BuildMatch`).

Your worktree and the commit it stands on are named in the Director's message that starts you: a **fresh worktree at the newest main**, the first tree on this
track that has everything: U1's light hold, the arena's pieces and reactions, the map preload, full-resolution rendering, and lane A124's rig preload
(`jam.PreloadRig`, landed switched **off** because it was never measured against U1's blocking load). It has no binaries: build the editor first, then one
Development package (`tools/run/package.ps1 ... -ExtraArgs "-ubtargs=-WaitMutex"`; the last one took 58 minutes on a loaded machine: start it first and
measure the editor-side things while it cooks). Your status file: `status/A137.md`. Port 4706 (the track's). Family as the track's: `EffectsBinder.*`, the
match-start path in `HokejGameMode.*` / `RinkActor.*` / `MenuFlow.*` (start-up order only), `DefaultEngine.ini` rendering and loading settings, `tools/perf/**`,
`docs/experiments/PROFILING-5.md`.

## Evolution 4: the first second, on the real main
1. **The rig preload against the blocking rig load**: on main, the `JAMSCREEN light hold started ... blocking load N ms` line with `jam.PreloadRig` 0 and 1
   (lane A111 measured 61 to 72 ms): bar under 15 ms with it on, and the load frame under 100 ms in 6 of 6 runs a tier at 5 v 5. If it holds, the default
   becomes 1.
2. **The first 60 frames after the load frame, none over 33 ms** (frames of 35 to 191 ms in 19 of 24 runs today): an Insights GPU and render-thread trace of
   the first second; pipeline-state precaching for what it names; the skin cache for ten skeletal meshes; warm the effects of the first faceoff; and the
   arena's new point lights and dynamic material instances (lane A127), which nobody has profiled.
3. **The budget table on main at full resolution**, both tiers, 5 v 5, with the arena's reactions firing (score a goal by a drill): frame mean and p99, game
   thread, render thread, GPU; the HUD tick with track U1's two projections and socket read; the brain is not on main in its second evolution yet: skip it.
4. Not for this lane: the audio callback buffer (`AudioCallbackBufferFrameSize`): the Director ruled it stays at 1024 before the submission (a smaller buffer
   risks crackle on a weak machine for 20 ms once per match).

Six runs a tier per arm, interleaved, in one session; say the machine's load during them. The gate: the full `Jam` filter green, the hash unchanged; each change
behind a console variable that gives back the old start. Time box 120 minutes; interim in the status file.
