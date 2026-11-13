# Sky-light readback on the clay tier (lane A78, device A, 2026-11-13 12:30-12:50 CEST)

Question (handoff `A-to-B-0016`, B's point 3): with `LR_P3_Clay` no longer referencing `DaylightAmbientCubemap` (B28), are the two synchronous sky-light
readbacks on the first lit frames gone on the clay tier?

**Verdict for B: still there. The readback is not gone on clay with the small specified cube (two `UpdateCaptureContents` with `ReadSurfaceFloatData_Flush`
in all three clay runs, 25-26 ms in the two comparable runs, as long as the night's), so do not do the same to night: switch both rigs to a movable real-time
capture (and A clears `Rink.umap`'s sky light, which still carries `DaylightAmbientCubemap`).**

## Method

Development package of main 2678378 (`Game/Packaged/A78-dev`, built here; the editor target had to be built first, `-nocompileeditor` needs it). A65's
`tools/perf/start.ps1` three times per tier (`-Tier p3_clay`, `-Tier l2_night`, 1920x1080 windowed, `-Trace -TraceChannels cpu,frame,bookmark`), menu up 8 s, Enter,
CSV profiler to sim frame 600; `tools/perf/startframes.mjs` for the load frame and the first 60 frames; `insights-export.ps1` (`ExportTimerStatistics`, all
threads) for the sky-light timers. Raw: `tools/perf/results/a78/` (`startframes.txt|json`, `skylight-timers.txt`, per run `events.log`, `run.json`).
Load: runs clay-1, clay-2, clay-3 had another lane's ThirdJam window open (pid 39604; GPU busy, CPU shared); night-1..3 had none.

## Table

"Match start" is the menu -> match frame (FrameTime of the longest game-thread frame). "First lit" lists frames over 33.3 ms in the next 60 frames.
Readback: the two `UpdateCaptureContents` calls (render thread; each contains `ComputeSingleAverageBrightnessFromCubemap > ReadSurfaceFloatData_Flush`), ms each.

| Tier | Run | Match-start frame ms | First-lit frames over 33 ms | Readback present | `UpdateCaptureContents` ms (2 calls) |
|---|---|---|---|---|---|
| p3_clay | 1 (busy) | 77.4 | 171.5 | yes | 12.7 + 167.3 = 180.0 |
| p3_clay | 2 (busy) | 72.3 | none | yes | 11.9 + 13.5 = 25.3 |
| p3_clay | 3 (busy) | 71.2 | none | yes | 13.1 + 13.2 = 26.3 |
| l2_night | 1 | 75.9 | none | yes | 7.8 + 9.8 = 17.7 |
| l2_night | 2 | 69.3 | 219.5, 56.6 | yes | 39.3 + 210.2 = 249.5 |
| l2_night | 3 | 72.4 | 34.4, 58.9 | yes | 31.5 + 43.5 = 75.0 |

Each run has exactly two captures (`STAT_UpdateSkyCaptureContents` x2, `ReadSurfaceFloatData_Flush` x4 calls); the number does not differ between tiers.
Typical (median of the three) readback sum: clay 26 ms, night 75 ms; the spread (17.7 to 249.5 ms) is larger than any difference between tiers, so the
runs do **not** show that clay is cheaper. In the quiet-GPU night run 1 the two readbacks cost 17.7 ms in total, the cheapest of all six; the spikes
(167, 210 ms) are GPU stalls on the first capture (shader/pipeline first use, other games), not a property of the cube.

## Why it is not gone

- The clay rig has no daylight cubemap (checked in the umap), but `Rink.umap`, the persistent map, still references `/Engine/MapTemplates/Sky/DaylightAmbientCubemap`
  (as does `LR_L2_Night`). `LookTierSwitcher` hides the persistent sky light only after the rig loads (`JAMLOOK persistent lights hidden: 2 actors`), when the
  world has already registered the component and queued its capture; this trace cannot tell which of the two sky lights each of the two captures belongs to.
- A specified cubemap still goes through `FScene::UpdateSkyCaptureContents > ComputeAverageBrightness > ReadSurfaceFloatData` (a synchronous readback)
  as soon as a sky light is registered with `SLS_SpecifiedCubemap` and non-real-time capture. The size of the cube changes the cost of the readback, not its
  existence.
- Two captures at 12-13 ms (clay, busy GPU) is what A58 measured as 26-38 ms each when GPU-busier; it is not "gone".

## Remaining match-start budget per tier

Match-start frame 69-77 ms on both tiers (of that FlushAsyncLoading 13-16 ms, EngineTickMisc 13-18 ms, game thread 45-52 ms). The readback sits on the render
thread and, when it spikes, in the first-lit frames, not in the load frame: clay-2 and clay-3 and night-1 show **no** frame over 33 ms (clay 25 ms of readback
hidden inside frames under 33 ms), the others 1-2 hitch frames of 34-220 ms. With the readback gone, the typical remaining cost is the load frame
(~72 ms, one frame) and no hitch; the readback is worth 18-75 ms of the first lit frames on a quiet GPU and the whole of the 100-250 ms spikes.

## Caveats

Three runs per tier on a shared machine; the clay runs had a busy GPU, the night runs did not (so clay vs night is not a like-for-like comparison of GPU
ms, only of presence). Presence is unambiguous (11 `ComputeSingleAverageBrightnessFromCubemap` calls, 2 `UpdateCaptureContents` in every run).
