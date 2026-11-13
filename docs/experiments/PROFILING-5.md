# Profiling 5: the first second on the real main (lane A137, track Q3 evolution 4, device A, 2026-11-13 22:39 - 14 Nov 00:20 CEST, a usage-limit stop at 23:37)

Follows `PROFILING-4.md` (lane A124). First tree on this track with everything: track U1's light hold (the rig's blocking load in `BuildMatch`), the arena's
pieces and reactions (lanes A103, A119, A127), A114's map preload, full-resolution rendering, A124's rig preload. Same tools: Development packages of this tree
(`Game/Packaged/A137` = main as landed, `A137b` = the rig and arena preloads, `A137c` = with the warm scene), driven cursor-free by `tools/perf/bot.ps1` on port 4706, A/B inside one package by
`-dpcvars`, interleaved (`tools/perf/ab.ps1`), six runs a tier per arm at 5 v 5. Status log: `status/A137.md`.

**Machine during every timing run**: loaded. CPU 100 % (Win32_Processor LoadPercentage), 36-46 GB of 63 GB in use, 3-11 other `UnrealEditor-Cmd` and
2-3 other `ThirdJam` windows (other lanes' games share the GPU). 30 s samples: `tools/perf/out/a137-load.csv` (local). Interleaving shares the load between
the arms; the absolute numbers are worse than on a quiet machine.

**Verdict.**
1. **The rig preload holds**: the light hold's blocking load is median 9 -> 5.5 ms, worst 21 -> 15 ms. A111's 61-72 ms is gone in both arms (A114's map
   preload already brings most of the rig's imports). `jam.PreloadRig` default is now **1**.
2. **A new offender on main, removed**: the arena's static meshes and one team instance (8 packages) were loaded **synchronously in the menu -> match
   frame** by `ARinkActor`'s build (`JAMPRELOAD sync /Game/Arena/...` in 24 of 24 runs; `FlushAsyncLoading` 10-76 ms, once 575 ms). They now join the
   match preload behind the menu: `jam.PreloadArena` (default **1**; 0 = before). Section 3 has the A/B.
3. **The first rendered match frame, warmed behind the menu**: that frame (60-150 ms in nearly every run, under the light hold's black picture) waited on
   pipeline states created at first use: 60-70 graphics and 85-105 compute PSO precache misses (engine CSV `PSO/PSOMisses`, `PSO/PSOComputeMisses`),
   `PSO/GraphicsPSOHitchTime` up to 127 ms, render thread `EventWait` 130-190 ms at an idle GPU. The menu draws an opaque black card
   (`MenuWidget.cpp:108-112`), so its world's 3D view is never seen: **`jam.WarmScene`** (default **1**; 0 = before) streams the tier's light rig into the
   menu world and stands a plane of each rink look material and the arena's four pieces in front of the menu camera for 90 frames (1.8 s) once the match
   preload is done. The misses fall to **26-31 graphics and 1-6 compute**; the worst frame of the first 60 falls from **median 78 to 47 ms** (6 a tier per arm,
   interleaved); the load frame from median 91 to 82 ms, **11 of 12 under 100 ms** (7 of 11 before). With `r.PSOPrecache.Validation=2` no mesh-pass miss
   is left (before: the rink's four procedural-mesh materials); the rest are untracked global pipeline states. Not yet none over 33 ms: 2 of 12 runs.

## 1. What was built (`EffectsBinder.cpp`, `UJamPreloadSubsystem`)

- `jam.PreloadRig` default 0 -> **1** (lane A124's rig preload: the tier's `LR_*` rig map loads asynchronously behind the menu).
- `jam.PreloadArena` (new, default **1**, 0 = before): `MatchPaths` adds `ARinkActor::TeamBenchMeshPath`, `PenaltyBoxMeshPath`, `ScorerTableMeshPath`,
  `GoalLightMeshPath`, the three goal and net variants of `GArenaGoalVariants` (`SM_Arena_Goal_W400H061`, `W600H061`, `W400H122` and their nets), and
  `MI_Toon_Orange`, `MI_Toon_Blue`, `MI_Toon_Dark`. Absent packages are filtered by `StartPreload`'s existence check as before; the handle holds them as it
  holds every match asset.
- `jam.WarmScene` (new, default **1**, 0 = before) and `jam.WarmScene.Frames` (90): `UJamPreloadSubsystem::TickWarmScene`, called from the
  `OnBeginFrame` hook while the menu map is up with no travel pending, packaged and `-game` only. When the match preload is complete it loads the likely tier's
  `LR_*` rig as a level instance of the menu world (`ULevelStreamingDynamic::LoadLevelInstance`; the preload holds the rig's package, so its imports are in
  memory) and spawns one transient actor 6 m ahead of the menu camera with a small `/Engine/BasicShapes/Plane` per look material the preload holds
  (`MI_Ice_Night` ... `MI_Glass`, 18 found) and the four arena meshes, custom depth stencil 1 as on the rink. After 90 rendered frames the actor is destroyed
  and the level instance unloaded; the pipeline states stay in the engine's cache. Nothing is loaded here (`FindObject` only). Log:
  `JAMPRELOAD warm scene up: rig l2_night (streaming in), 22 items ... frame=14` and `... down after 90 frames (1.82 s)`.
- `tools/perf/firstsec.mjs` (new): the first 60 frames after the menu -> match frame: each frame over 33 ms with its render-thread stats and GPU passes,
  and one summary line a run: the light hold's blocking load (game log), the load frame and its `FlushAsyncLoading`, frames over 33 ms, the PSO hitch time
  and precache misses (CSV), the PSO misses the log names, the goal reactions. Its load frame is the longest game thread **among the first 30 rows** (a
  later stall in play is never taken for it, which `startframes.mjs` / `loadrow.mjs` do in about 1 run in 8).

## 2. Item 1: the rig preload against the blocking rig load (`A137`, 25 s of play each)

`-dpcvars=jam.PreloadRig=0` vs `=1`, interleaved, six a tier per arm (`a137-rig-*`). The `JAMSCREEN light hold started ... blocking load N ms` line:

| tier | PreloadRig 0 | PreloadRig 1 |
|---|---|---|
| l2_night 5 v 5 | 12, 9, 12, 15, 8, 9 | 6, 15, 6, 10, 4, 10 |
| p3_clay 5 v 5 | 21, 5, 5, 15, 5, 9 | 4, 3, 7, 4, 5, 3 |
| median / worst | 9 / 21 | 5.5 / 15 |

The brief's bar (under 15 ms with it on): 11 of 12 (one at 15). The load frame under 100 ms in 6 of 6: **not reached in either arm** (2 of 12 each):
on this tree the load frame carried the arena's synchronous loads (section 3) and LoadMap's audio wait (`EventWait/Audio` 21-37 ms, ruled to stay).

## 3. The arena preload (`A137b`)

`-dpcvars=jam.PreloadRig=0,jam.PreloadArena=0` (main's start before this lane) vs `jam.PreloadRig=1,jam.PreloadArena=1` (the new defaults), interleaved,
six a tier per arm, the start only (`a137-arena-*`). `firstsec.mjs`; ms.

| run | old (0, 0): load frame / FlushAsyncLoading / sync arena loads | new (1, 1): load frame / FlushAsyncLoading / sync arena loads |
|---|---|---|
| l2_night 5 v 5 #1-6 | 717 / 575, 134 / 33, 296 / 200, 170 / 77, 109 / 16, 92 / 17; 7 each | 285* / 2.6, 103 / 10, 89 / 8.4, 115 / 28, 91 / 2.9, 89 / 2.3; 0 each |
| p3_clay 5 v 5 #1-6 | 135 / 36, 208 / 101, 108 / 15, 130 / 24, 114 / 9.2, 88 / 9.7; 7 each | 116 / 2.4, 104 / 3.4, 105 / 3.1, 117 / 13, 100 / 1.8, 88 / 2.2; 0 each |
| median load frame / FlushAsyncLoading | 132 / 28.5 | 104 / 2.8 |
| load frame under 100 ms | 2 of 12 | 4 of 12 (100.1 once) |

\* game thread 55 ms in a 285 ms frame: a wait outside the game thread (the machine). The light hold's blocking load in the same runs: old 5-20 ms, new 3-23 ms.
What stays in the load frame (`loadrow.mjs`): `EngineTickMisc` 31-42 (the world's BeginPlay: the rink and ten figures), `EventWait/Audio` 21-37 (ruled to
stay), `EventWait` 50-120 (the render thread).

## 3b. The warm scene (`A137c`)

`-dpcvars=jam.WarmScene=0` vs `=1` (the other defaults of this lane on in both arms), interleaved, six a tier per arm, the start only (`a137-warm-*`,
`tools/perf/out/a137-warm-firstsec.txt`). Machine: 26-45 GB, CPU 59-100 %, 4-12 other `UnrealEditor-Cmd`, one other `ThirdJam`. Worst frame of the
first 60 after the load frame / frames over 33 ms / PSO misses graphics + compute (CSV, the load frame and the 60 after it):

| run | WarmScene 0 | WarmScene 1 |
|---|---|---|
| l2_night #1-6 worst | 89.9, 77.9, 78.6, 79.3, 70.4, 59.1 | 50.0, 48.5, 39.8, 52.2, none, none |
| p3_clay #1-6 worst | 271.4, (#2: capture began after the start), 66.3, 78.5, 48.2, 47.2 | 41.2, 45.1, 57.7, 43.2, 48.8, 44.1 |
| median worst / runs with none over 33 | 78 / 0 of 11 | 47 / 2 of 12 |
| PSO misses | 60-71 + 83-103 | 26-31 + 1-6 |
| load frame median / under 100 ms | 91 / 7 of 11 | 82 / 11 of 12 |

The menu pays for it: 90 frames of the rig drawn behind the black card while the menu is up (from about 0.3 s after the menu map loads to 2.1 s).

## 4. Item 3: the budget at full resolution, 5 v 5, the arena's reactions firing

From the item 1 runs (`A137`, 25 s of play from 120 rows after the load frame, ~1950 rows each, two goals with the arena's goal reaction in every run,
`JAMRINK react goal`); `five.mjs`, rows `tools/perf/out/a137-five.json` (local). Runs whose GPU mean doubled (another lane's game on the GPU: l2 old #3,
new #3; p3 old #6) and one without a play window (l2 new #2) are left out of the ranges; two p3 runs had short play windows (437, 683 rows).

| tier | frame mean / p99 | game thread mean / p99 | render thread mean / p99 | GPU mean / p99 |
|---|---|---|---|---|
| l2_night 5 v 5 (9 runs) | 16.9-19.3 / 20.7-47.6 | 2.6-3.2 / 4.2-20.7 | 4.3-5.7 / 6.3-37.4 | 4.4-4.8 / 7.0-8.3 |
| p3_clay 5 v 5 (11 runs) | 16.9-18.2 / 19.1-44.1 | 2.5-3.2 / 4.5-7.4 | 4.5-5.4 / 5.7-21.8 | 4.7-6.4 / 6.8-9.7 |

The frame is held at 60 Hz (mean 16.9-17.3 ms on most runs); game thread, render thread and GPU each stay under a third of the budget in the mean and, except
l2 new #1 (81 hitches in play, game p99 20.7, render p99 37.4 at GPU 4.5: the machine), under 10 ms at p99. Frame p99 over 25 ms and the hitches in play (2-81 a run over 33 ms) are the loaded machine: they come with render-thread
`EventWait` at an idle GPU, in both arms alike. The goal reaction (lamp emissive, two shadowless point lights shown) shows no frame of its own.

HUD tick (Insights, `a137-diag-*`, `hudtimers.mjs`): `JamHud_Tick` 0.10 ms a call, of which track U1's two projections `JamHud_NetMarkers` 0.023 and
`JamHud_Yours` 0.030 ms, `JamHud_PuckMarker` 0.010 ms; `HokejGameMode` 0.6-0.7 ms a frame; the figures' Blueprint (`ExecuteUbergraph_BP_SkaterVisual_*`)
0.46-0.54 ms a frame over ~48 calls. The brain is not on main in its second evolution: skipped.

## 5. The first 60 frames: what is left, and the next evolution

| what | evidence | the fix (next evolution) |
|---|---|---|
| First rendered match frame 40-58 ms with the warm scene (60-150 before) | 26-31 graphics + 1-6 compute misses left, all untracked by the validation (global pipeline states) | warm them too: draw the warm scene through the match's own view settings (TSR at the window size, the HUD's Slate materials, a figure and the puck), or a bundled PSO cache recorded from a bot match |
| The rink's procedural meshes take no part in precaching | before the warm scene: `PSO PRECACHING MISS` x4, `MDCStatsCategory: ProceduralMeshComponent`; with it: none | done by the warm planes (same local vertex factory and material); a `CollectPSOPrecacheData` on a JamGame subclass would make it independent of the menu |
| Later frames 35-85 ms (1-3 a run) | RT `EventWait` at GPU 3-6 ms, no PSO event: machine contention | repeat on a quiet machine before any fix |

Shipping note: `r.ShaderPipelineCache.SaveUserCache` is on in Shipping only (engine default): a Shipping player's **second** launch precompiles what the
first launch missed; the first launch has nothing but the warm scene.

Skin cache: `r.SkinCache.CompileShaders` is not set; the ten figures skin through the GPU-skin vertex factory and no miss names a skinned vertex factory:
no change proposed. The arena's point lights (A127) are movable, shadowless and hidden at rest: no frame of the goal reaction stands out (section 4).

## 6. The gate

Full `Jam` filter **191 of 191** (119 s, `Game/logs/a55-automation-ue-20261113-232445.log`). `Jam.Core.Determinism` frame 600: hockey.regular
`0ff7ea98d5570484`, hockey.raw `8425610943e6dd92`. The same hashes with `jam.PreloadRig 0, jam.PreloadArena 0` (`Game/logs/a137-det-neutral2.log`). The change is presentation only: the preload runs in packaged and `-game` builds, never in
the editor's tests. After the warm scene: full `Jam` 191 of 191 again, the same two hashes.

## 7. How to repeat

`powershell -File tools/perf/ab.ps1 -Exe Game\Packaged\<pkg>\Windows\ThirdJam\Binaries\Win64\ThirdJam.exe -Prefix <p> -Old jam.PreloadRig=0,jam.PreloadArena=0
-New jam.PreloadRig=1,jam.PreloadArena=1 -N 6 [-PlaySeconds 25]` (the warm scene: `-Old jam.WarmScene=0 -New jam.WarmScene=1`), then `node tools/perf/firstsec.mjs tools/perf/out/<p>-*` and `node tools/perf/five.mjs ...`.
PSO miss names: add `r.PSOPrecache.Validation=2` to `-Dp` and grep the game log for `PSO PRECACHING MISS`.
