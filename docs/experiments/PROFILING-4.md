# Profiling 4: the first second of a match (lane A124, track Q3 evolution 3, device A, 2026-11-13 20:40-22:30 CEST)

Follows `PROFILING-3.md` (lane A114). Same tools: a Development package of this tree (`Game/Packaged/A124-after`), driven cursor-free by
`tools/perf/bot.ps1` on port 4706, bot matches only, A/B inside one package by `-dpcvars`, interleaved (`tools/perf/ab.ps1`). Status log:
`status/A124.md`. Every row: `tools/perf/results/a124/startrows.txt`.

**Verdict.** No gain to land as a default. The quiet menu hold (stop every sound, hold the travel 4 frames) was built, fires as designed, and does not remove
LoadMap's audio wait: `EventWait/Audio` was 21-38 ms with it and without it. On a quieter machine than A114's (27 GB in use) both arms already have
10 of 12 start frames under 100 ms (median 89.8 ms before, 87.9 after); the misses (105, 177 / 120, 152 ms) are scattered spikes in both arms. The first 60 frames
after the load frame still carry hitches of 35-191 ms in 19 of 24 runs, in both arms. The commentary's default is now `jam.vo.SkipAbsent 0` (the Director's
ruling), and a 120 s match still has **0 synchronous loads after the first second**. The game now draws at the window's resolution
(`r.ScreenPercentage.Default.Desktop.Mode=0`); see section 4 for what that costs.

## 1. What was built (`EffectsBinder.h/.cpp`, `UJamPreloadSubsystem`)

- **The quiet menu hold**, `jam.StartQuietFrames` (default **0** = the game before; 4 was tried). `FCoreDelegates::OnBeginFrame` runs before
  `UGameEngine::Tick`'s `TickWorldTravel`, so it sees a pending travel to the Rink set by the menu (`OpenLevel`), a remote console `open` or anything else
  that calls `SetClientTravel`. It stops every sound (`FAudioDevice::StopAllSounds(true)`, queued to the audio thread), takes the `TravelURL` off the world
  context and gives it back N frames later. Log: `JAMPRELOAD quiet: sounds stopped, travel to /Game/Maps/Rink held 4 frames frame=428` ... `released frame=432`.
- **The light rig behind the menu**, `jam.PreloadRig` (default 1; 0 = before): the cut's tier's `LR_*` rig map (`ULookTierSwitcher::GetRigMapPath`) joins
  lane A65's match preload (packaged and `-game` only). The handle holds it as an Inactive world, which LoadMap's world-leak check exempts; the match's level
  instance loads its own renamed copy and finds the rig's meshes, materials and lights in memory. **Not proven here**: this tree does not have lane A111's
  blocking `FlushLevelStreaming` in `BuildMatch`, which is what it is for. On main, compare the `JAMSCREEN light hold started ... blocking load N ms` line with
  `jam.PreloadRig=0` and `1` (A111: 61-72 ms).
- `CommentarySystem.cpp`: `jam.vo.SkipAbsent` default 0 (Director's ruling; the once-per-process absent lookup stays either way).
- `DefaultEngine.ini`: `r.ScreenPercentage.Default.Desktop.Mode=0` (Director's ruling, identical to main).

## 2. Why the audio wait stays

`UEngine::LoadMap` calls `FAudioDevice::Flush(World)`, which ends in `FMixerDevice::FlushAudioRenderingCommands` ->
`FMixerSourceManager::FlushCommandQueue`: it returns at once only when **no** command is queued, otherwise it waits for the render callback twice
(`CommandsProcessedEvent`, the double pump). With 1024-frame callbacks at 48 kHz (`AudioCallbackBufferFrameSize=1024`, `AudioNumBuffersToEnqueue=1` in
`DefaultEngine.ini`) one callback is 21.3 ms, so two waits are 21-43 ms: the measured 21-38. Commands are queued every audio update whether or not a source
plays, so a silent menu does not help. What would: a smaller callback (512 frames: 10.7 ms a wait, both waits ~21 ms), at the cost of more audio-thread wakeups.

## 3. The start, A/B (`A124-after`, interleaved, six runs a tier per arm)

Worst game-thread frame of the start (the load frame or a later spike, `startframes.mjs`), ms; `EventWait/Audio` of the load frame where `loadrow.mjs` found it.

| run | old: StartQuietFrames=0, PreloadRig=0 | new: StartQuietFrames=4, PreloadRig=1 |
|---|---|---|
| l2_night 5 v 5 #1-6 | 80.8, 93.9, 177.3, 105.0, 66.5, 84.3 | 86.3, 82.8, 120.5, 82.3, 97.9, 92.6 |
| p3_clay 5 v 5 #1-6 | 82.7, 95.0, 90.6, 74.7, 93.1, 89.0 | 98.9, 81.4, 89.4, 151.6, 79.0, 84.0 |
| under 100 ms | 10 of 12 (l2 4, p3 6) | 10 of 12 (l2 5, p3 5) |
| median | 89.8 | 87.9 |
| EventWait/Audio | 21.0-38.2 | 22.6-37.9 |

First 60 frames after the load frame, frames over 33 ms (count / worst): old l2 2-8 / 39-149, p3 0-3 / 36-102; new l2 1-4 / 35-191, p3 0-5 / 36-60.
Both arms alike: item 2 of the brief (GPU trace, PSO precache, skin cache, effect warm-up) is the next evolution.

A first pass of the old arm only (12 runs, `a124-ab-old-*`, the same minutes) agrees: 76-130 ms, `FlushAsyncLoading` 0.05-0.2 (A114's map preload holds).

## 4. Screen percentage at 1920 x 1080 (Director's addition)

Same package, `-dpcvars=r.ScreenPercentage.Default.Desktop.Mode=1` (before: 72.8 % of the window, enlarged by TSR) vs `=0` (after: 100 %), interleaved,
three runs a tier per arm (reduced from six for the time box), about 28 s of 5 v 5 play each (~1700 frames, `five.mjs`), window 1920 x 1080, ms:

| tier | Mode 1: frame mean / p99 | Mode 1: GPU mean / p99 | Mode 0: frame mean / p99 | Mode 0: GPU mean / p99 |
|---|---|---|---|---|
| l2_night 5 v 5 | 16.70-16.71 / 16.86-17.19 | 3.04-3.90 (3.36) / 4.09-6.43 | 16.70-16.77 / 17.24-18.22 | 3.82-4.30 (4.09) / 5.64-6.64 |
| p3_clay 5 v 5 | 16.68-16.72 / 16.81-17.55 | 3.40-4.23 (3.70) / 4.26-6.35 | 16.72-16.84 / 17.29-19.74 | 5.14-5.87 (5.61) / 8.06-8.81 |

The frame is held at 60 Hz in both arms (mean 16.7 ms: the present interval, not the GPU). Full resolution costs GPU +0.7 ms on l2_night (+22 %) and +1.9 ms
on p3_clay (+52 %); the worst GPU p99 is 8.8 ms, half the 16.7 ms budget. **Both tiers keep their budget at 100 %: no fixed `r.ScreenPercentage` is
proposed.** Frame p99 rises by up to 2.2 ms (p3_clay #3: 19.7) and stays under the 33 ms hitch line. Rows: `tools/perf/out/a124-sp-*` (local).

## 5. The gate

Full `Jam` filter 156 of 156 (64.5 s, `Game/logs/a55-automation-ue-20261113-205403.log`); `Jam.Core.Determinism` hockey.regular seed 1 frame 600
`d192973ac7c97102`, hockey.raw `b02f67a2da0314b4`: unchanged (all of it is presentation, off in the editor). `jam.StartQuietFrames`' default went from 4 to 0
after the gate (a literal; the editor never takes that path: `GIsEditor` guard).

## 6. How to repeat

`powershell -File tools/perf/ab.ps1 -Prefix <p> [-Old <cvars>] [-New <cvars>] [-N 6] [-PlaySeconds 0]`,
then `node tools/perf/startframes.mjs`, `loadrow.mjs`, `five.mjs`, `syncrows.mjs` on `tools/perf/out/<run>`. Note: `bot.ps1 -Dp ''` refuses an empty
argument; give every arm an explicit cvar list.
