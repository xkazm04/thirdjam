# Lane A58: Unreal Insights traces of the packaged build at every tier (device A, night filler, Opus)

You are lane A58 of the Third Jam (team Hokej). Nobody has profiled the shipped game. Your job: trace bot matches in the packaged build at every look tier, and leave the
morning a hitch list, a frame budget per system and a memory report with the ten worst offenders and the file to fix for each.

Read first: `docs/experiments/CONTROLS.md` (A49: packaging a Development game build in a worktree, `tools/run/package.ps1 -Config Development`), `tools/run/run-packaged.ps1`,
`Game/Source/JamGame/Private/HokejGameMode.cpp` (`jam.*` commands, the look tiers, the bot-only match path), `Game/Content/Cartridges/*.cartridge.json` (`look.tier`),
`docs/experiments/LOOK-P2-NIGHT.md`, `docs/experiments/DLSS.md`, `docs/experiments/SIZE.md`, `Game/Config/DefaultEngine.ini` (scalability, Lumen, shadows),
`Game/Source/JamGame/Private/EffectsBinder.cpp` (Niagara spawns), `SkeletalSkaterVisual.cpp` and B's `status/SK*-report.md` (the figure's cost), `CommentarySystem.cpp`.
Unreal Insights: `C:\Program Files\Epic Games\UE_5.9\Engine\Binaries\Win64\UnrealInsights.exe`; traces with `-trace=cpu,gpu,frame,memory,loadtime -tracefile=<path>`;
the CSV path `-csvprofile` / `csv.profile` for a text table when Insights' UI is not scriptable (`UnrealInsights.exe -OpenTraceFile=... -ExecOnAnalysisCompleteCmd=...` exports
tables; if that does not work on this install, say so and use `stat unit`, `stat gpu`, `stat memory`, `csvprofile` dumps and `memreport -full`).

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a58` (created at the current main; build the Development game package yourself with `-WaitMutex`). Run no git
command; never touch `Game/Plugins/JamAssets`; never print `.env`; never kill a process you did not start. Open no interactive editor. One game at a time at 1920x1080
windowed (the operator's resolution class); GPU numbers taken while another lane renders are labelled busy.

You own: `tools/perf/**`, `docs/experiments/PROFILING.md`, `docs/experiments/img/a58/**`, `status/A58.md`. No `Game/Source` edits: findings name the file and the fix.

## Job
1. Per tier (`p2_night` or whatever the cuts name, `p3_clay`, `l2_night`, and the default): a 3-minute bot match with a trace; `stat unit` averages and 99th percentile
   for frame, game, draw, GPU; hitches (frames over 33 ms) with their frame numbers and the top stack of each.
2. Frame budget per system at the default tier: sim step, AI, animation (the figure's anim graph, per skater), Niagara, rendering passes (Lumen, shadows, post), UI, audio,
   commentary; a table with ms and % of a 16.7 ms frame.
3. Memory: `memreport -full` at match start and end; the top 20 assets by size; textures over budget; a leak check across three rebuilds (`jam.livecut` three times, memory
   before and after).
4. Load time: menu to match in seconds per tier; what dominates (shader compile, asset load, rink build).
5. `PROFILING.md`: the per-tier table, the hitch list, the budget table, the memory table, the ten worst offenders ranked by ms saved per hour of work, each with the file,
   the fix and the risk; stills of the Insights timeline for the worst hitch.

Report: the ten offenders, the per-tier numbers, the file list. Interim at 60 minutes with step 1. Time box 150 minutes.
