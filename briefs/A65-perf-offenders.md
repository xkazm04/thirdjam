# Lane A65: the profiler's ten offenders, the A-side ones fixed and measured (device A, from H14, Opus)

You are lane A65 of the Third Jam (team Hokej). Lane A58 profiled the packaged build at four tiers (`docs/experiments/PROFILING.md`): no hitches during play, every stutter
at match start, and ten offenders ranked by ms saved per hour of work. Fix the ones on A's side, measure each with A58's own tools, and name B's.

Read first: `docs/experiments/PROFILING.md` (the offenders with file and fix; the method; the remote console `-RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=4658` that
drives a packaged game when the foreground is held by a foreign dialog), `tools/perf/**` (`match.ps1`, `post.ps1`, `analyze-csv.mjs`, `hitches.mjs`, `loadtime.mjs`,
`memreport.mjs`; the perf cartridges under `tools/perf/cuts/`), `status/A58.md`, `Game/Source/JamGame/Private/{EffectsBinder,HokejGameMode,RinkActor,JamPawns,
LookTierSwitcher,MenuFlow,HUDWidget}.cpp`, `Game/Config/DefaultEngine.ini`, `docs/experiments/DLSS.md`, `docs/experiments/UI.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a65` (created at the current main; build it yourself, unity off; the Development game package with
`-WaitMutex`; no interactive editor unless a Blueprint must be inspected, then one on MCP port 8032 and closed after). Run no git command; never touch
`Game/Plugins/JamAssets` (offender 1's rigs and the sky-light cubemap in the map are B's: write the parameter and the saving into your report); never print `.env`;
never kill a process you did not start. Presentation and loading work never changes the sim hash; a GPU figure taken while another game window is open is labelled busy.

You own: `EffectsBinder.*` (the async preload from the menu), `HokejGameMode.cpp` + `RinkActor.cpp` + `JamPawns.cpp` + `LookTierSwitcher.cpp` (the match-start loads moved
to the menu: `ResolveVisualClass`, the rink's meshes, the puck, the tier's assets), `MenuFlow.cpp` (the audio flush, if real on a machine with speakers: say which),
`HUDWidget.cpp` (`NativeTick` rebuilds only on change), `JamPawns.cpp:230` (`SetShotCharge` sent only on change), `Game/Config/DefaultEngine.ini` (`r.Shadow.Virtual.
MaxPhysicalPages`, `r.TSR.History.ScreenPercentage`), the UI textures' compression settings if they are A's content (`Game/Content/UI/*`), `docs/experiments/PERF-FIXES.md`,
`status/A65.md`.

## Job
1. Offenders 3 and 4: one preload pass started when the menu appears (effects, sounds, the visual class, the rink meshes, the tier's assets), so the menu-to-match frame
   drops from 163-175 ms; measure before and after with `tools/perf/loadtime.mjs` on your Development package at l2_night.
2. Offender 2: `r.Shadow.Virtual.MaxPhysicalPages=1024` in `DefaultEngine.ini`; measure the first shadowed frame and the shadow pool's memory before and after; look at a
   far-net still for shadow quality loss (eyes `bug-hunt`); if it loses visibly, try 2048.
3. Offender 6: `r.TSR.History.ScreenPercentage=100`; GPU ms and the 179 MB before and after; a still pair for sharpness.
4. Offenders 8 and 9: `SetShotCharge` and the HUD rebuild only on change; the game-thread ms before and after over a 3-minute bot match.
5. Offender 10: compress `T_UI_CallCard` and the two button textures to BC7 if they are A's content; memory before and after.
6. Offender 5: the audio-thread flush: measure once with the game window focused (if the foreign dialog allows; else say so) and decide whether it is real.
7. Offenders 1 and 7 are B's (the rigs' sky light and Lumen settings): write the exact parameters and the expected saving for B in your report; no asset edits.
8. `PERF-FIXES.md`: the before/after table per offender (ms, MB), the stills, what went to B, the remaining frame budget per tier.

Report: the before/after table, what is B's, the file list. Interim at 45 minutes with step 1. Time box 100 minutes.
