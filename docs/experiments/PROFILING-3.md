# Profiling 3: the match start at five a side (lane A114, track Q3 evolution 2, device A, 2026-11-13 20:00-21:30 CEST)

Follows `PROFILING-2.md` (lane A106). Same tools: a Development package of this tree (`Game/Packaged/A114-after`; `A106-dev` for the before), driven
cursor-free by `tools/perf/bot.ps1` on port 4706, bot matches only. Status log: `status/A114.md`.

**Verdict.** The Rink map no longer loads synchronously in the menu -> match frame: its package is loaded asynchronously while the menu is up and held until
the match map is up. In the same package, A/B by `-dpcvars=jam.PreloadMap=0/1`, interleaved, six runs each: the load frame's `FlushAsyncLoading` went from
20-115 ms (mean 53) to 0-17 ms (mean 7), the load frame from median 131 ms (0 of 6 under 100) to median 106 ms (**3 of 6 under 100**). The brief's 6 of 6 is not
reached: what is left in the frame is LoadMap's own audio flush (`EventWait/Audio` 24-56 ms), the world's start (`EngineTickMisc` 33-86 ms) and other waits. No
synchronous package load remains in play (absent commentary waves, the crowd bed and the score plate are found absent once per process). The native
tournament build compiles again.

## 1. The map preload (`EffectsBinder.h/.cpp`, `UJamPreloadSubsystem`)

- When the menu map is up (packaged or `-game` only, never in the editor), `LoadPackageAsync("/Game/Maps/Rink")`; the package **and its UWorld** are held in
  `UPROPERTY`s (a package does not keep its objects alive: holding only the package, LoadMap's garbage collection took the world and the match failed to load,
  "Failed to load package"). No `UWorld::WorldTypePreLoadMap` entry: the world waits as `EWorldType::Inactive`, which LoadMap's world-leak check
  (`UEngine::CheckAndHandleStaleWorldObjectReferences`) exempts; marked Game it was a fatal "world leak". `UEngine::LoadMap` finds the package in memory
  (`FindPackage`) and takes the world. At the match map's `PostLoadMapWithWorld` the references are dropped (the world holds itself; it is collected when the
  match ends); every return to the menu requests it again. If the match starts before the load finished, `PreLoadMap` flushes that one request.
- Proved twice in one process (menu -> match -> menu -> match): both starts "map in memory", no leak error, map released at each match.
- `jam.PreloadMap` (default 1; **0 = the game before**: the map loads in the menu -> match frame).

| run (A114-after, busy machine) | PreloadMap=0 load frame / Flush ms | PreloadMap=1 load frame / Flush ms |
|---|---|---|
| l2_night 5 v 5 #1 | 134.0 / 33.9 | 98.7 / 0.1 |
| l2_night 5 v 5 #2 | 170.2 / 77.8 | 94.4 / 12.4 |
| l2_night 5 v 5 #3 | 206.5 / 114.8 | 112.5 / 10.4 |
| p3_clay 5 v 5 #1 | 111.3 / 27.2 | 211.5 / 0.1 (EngineTickMisc 85.8, EndPhysics wait 21.7: noise) |
| p3_clay 5 v 5 #2 | 113.2 / 20.0 | 137.3 / 16.9 |
| p3_clay 5 v 5 #3 | 127.8 / 46.7 | 99.3 / 0.1 |
| under 100 ms | 0 of 6 | 3 of 6 |

Before, `A106-dev` the same session (my build compiling beside it): 216, 130, 117, 308, 201, 111 ms. Every row's breakdown: `tools/perf/results/a114/loadrows.txt`
(new `tools/perf/loadrow.mjs`: the load frame's exclusive game-thread stats).

What remains in the load frame with the preload (ms): `EventWait/Audio` 24-56 (LoadMap's `AudioDevice->Flush` of the menu's sounds), `EngineTickMisc` 33-86
(world bring-up, BeginPlay: the rink build and ten figures), `EventWait` 18-57, `UI` 7, `ActorSpawning` 5-6, a residual `FlushAsyncLoading` 0-17 (other
packages still in flight when LoadMap flushes).

## 2. No synchronous lookup in play (`CommentarySystem.cpp`, `MenuStyle.h`)

- `IsAbsentAsset`: a path is absent when the preload's list (`UJamPreloadSubsystem::IsKnownMissing`) says so, else one cached `FPackageName::DoesPackageExist`
  (a package-store query, never a load). A picked commentary line whose wave is absent is skipped (`JAMVO_SKIP reason=no_wave`, no caption) under
  `jam.vo.SkipAbsent` (default 1; **0 = the game before**: played as silence with its caption); `LoadWave` never asks the loader for an absent wave; the crowd bed
  is asked for only when present. `FJamUiStyle::Art` remembers an absent texture per process (`T_UI_ScorePlate`).
- Proof: a 120 s l2_night 5 v 5 match on `A114-after`: `syncrows.mjs` **0 synchronous loads after the first second** (A106's before: 14 loads of 7 absent VO
  packages, 2-26 ms each); 6 picked lines skipped for want of a wave, 27 played. `T_UI_ScorePlate` is asked once per process, in the first match-start frame.
- Missing waves (72 of 412 bank lines; list `tools/perf/results/a114/missing-waves.txt`, for device B): ability_{hip_check, shield, shotgun, slow_puck}_01-03,
  check_cross_01-10, check_light_01-06, check_solid_01-07, goal_after_hit_01-10, knockdown_big_01-07, knockdown_human_04-13, knockdown_massacre_01-10.

## 3. The native tournament build

`tools/tournament/run.mjs` `nativeArgs()` defines `-DJAMCORE_API=` (UBT's export macro, used in `JamAICore.h`); the exe is rebuilt when run.mjs changes.
`tools/tournament/run.test.mjs` (node test, premerge's `node --test tools/**/*.test.mjs`): the flag is passed, and the driver compiles with clang (skipped without one).

## 4. The gate

Full `Jam` filter 156 of 156 (`tools/analysis/jamtests.ps1`, now with `-Multiprocess`, 88 s). `Jam.Core.Determinism` hockey.regular seed 1 frame 600
`d192973ac7c97102`, hockey.raw `b02f67a2da0314b4`: unchanged (the preload is off in the editor; the commentary skip is presentation).

## 5. How to repeat

`tools\perf\bot.ps1 -Exe Game\Packaged\A114-after\Windows\ThirdJam\Binaries\Win64\ThirdJam.exe -Tier l2_night_5v5 -Name <run> -PlaySeconds 0 -Dp jam.PreloadMap=0|1`,
then `node tools/perf/startframes.mjs`, `node tools/perf/loadrow.mjs`, `node tools/perf/syncrows.mjs` on `tools/perf/out/<run>` (syncrows now anchors on the
`JAMPRELOAD match start ... frame=` line when the map no longer loads synchronously, and prints the count).
