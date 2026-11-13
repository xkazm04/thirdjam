# Perf fixes: the profiler's offenders, the A-side ones fixed and measured (lane A65, device A, 2026-11-13 09:59-10:50 CEST)

Lane A58 profiled the packaged game (`PROFILING.md`) and ranked ten offenders. This page has, for each one, what A changed, the number before and after on
**my own two Development packages of the same tree** (`Game/Packaged/A65-before` = main e99e005 untouched, `A65-after` / `A65-after2` = with the fixes),
what was withdrawn because it did not move its number, and the exact parameters for B's two. Default tier l2_night, 1920x1080 windowed, RTX 4090.

**Verdict.** The menu -> match frame fell from **163-177 ms to 77-84 ms** on a comparable machine load (synchronous loading inside it from 91-96 to 19 ms,
`LoadMap(Rink)` 0.150-0.166 -> 0.068-0.072 s);
the first-lit-frame hitch (41 ms) is **gone** (the virtual-shadow-map pool commit 28 -> 7 ms); **~560 MB of VRAM** came back (VSM pool + HZB 597 -> 149 MB,
TSR history 179 -> 61 MB); TSR costs **0.30 ms less GPU per frame**; the UI textures take 1.0 MB instead of 3.5 MB. `SetShotCharge` no longer fires
(6 Blueprint events per frame -> 0, 0.026 ms). The HUD "rebuild only on change" did **not** move its number and was withdrawn. Nothing here touches the sim
(presentation and loading only: the sim hash is unchanged by construction; no sim file was edited).

## 0. Method and the premises that did not hold

| Brief's premise | What I found | What I used |
|---|---|---|
| Measure the menu -> match frame with `loadtime.mjs` | `loadtime.mjs` gives `LoadMap(Rink)` seconds from the log, not the frame; A58 had the frame from an Insights trace | `tools/perf/start.ps1` (new): the CSV profiler runs from the menu (a fixed 8 s after it appears, the same in every run) through the Enter, the load frame and the first lit frames to sim frame 600; `tools/perf/startframes.mjs` (new) reads the load frame, its `FlushAsyncLoading`, the first 60 frames' hitches and `ShadowInitDynamic`. `loadtime.mjs` for LoadMap. Note: the CSV writes a frame's exclusive game-thread stats one row before its `FrameTime`; the tool reads both rows |
| A quiet machine | Every run shared the CPU with other lanes (A53's eight tournament commandlets, A64's cook, a Shipping game and an editor of other lanes, see each run's `events.log`) | Before and after were taken under the same kind of load, interleaved where it mattered; the ms are read as relative. **GPU figures are labelled busy** wherever another game window was open |
| The UI textures need BC7 | **They already are BC7 in the asset.** The build stored them as `PF_B8G8R8A8` because 653 and 171 are not multiples of 4 (a block format needs 4x4 blocks) | the build resizes them to the next multiple of 4 (see offender 10) |
| VSM / TSR need a rebuild per value | `-dpcvars=` sets them before the renderer allocates | A/B of 1024 / 2048 pages and of the TSR history on the baseline package, then the chosen values in `DefaultEngine.ini` |
| Audio flush: measure with speakers, window focused | No foreign dialog this time, but this machine has **no audio endpoint**: `Using Audio Hardware Device Unknown`, `StartRunningNullDevice()`; a game launched from a background shell is also unfocused and muted | not verifiable here (offender 5) |

## 1. Before / after per offender

| # | Offender | Change (file) | Measure | Before | After | Verdict |
|---|---|---|---|---|---|---|
| 3 + 4 | Synchronous loads at match start (EffectsBinder 29-31 ms, figure class, rink look, puck, MPC_Look, ...) | `UJamPreloadSubsystem` (`EffectsBinder.h/.cpp`): when the menu map is up, one `RequestAsyncLoad` of everything the match start loads, held for the game instance; `AHokejGameMode::VisualClassPathsFor` (`HokejGameMode.h/.cpp`) shares the figure class list with it; absent packages are looked up once | menu -> match frame ms (CSV) | 162.7, 165.0, 170.7, 176.8 (`before-1..4`); 182.0 (`before-play180`, busier) | pass 1: **112.0, 111.0, 86.6**; pass 2 (shipped): **77.3, 83.9** (and 127.1, 179.4 with two or three other games open) | **kept** |
| | | | of it `FlushAsyncLoading` ms | 92.0, 90.9, 95.9, 94.2; 109.8 | pass 1: **29.2, 25.1, 20.4**; pass 2: **18.8, 19.4** (48.4, 42.9 busy) | |
| | | | `LoadMap(Rink)` s (`loadtime.mjs`) | 0.150, 0.153, 0.162, 0.166 | pass 1: **0.101, 0.101, 0.075**; pass 2: **0.068, 0.072** (0.116, 0.146 busy) | |
| | | | synchronous package loads in the frame (`JAMPRELOAD sync`) | ~87 (A58) | pass 1: 19; pass 2: **6** (the map + 5 lookups of 2 absent packages) | |
| 2 | VSM 512 MB page pool committed on the first shadowed frame | `r.Shadow.Virtual.MaxPhysicalPages=1024` (`DefaultEngine.ini`) | `ShadowInitDynamic` (render thread, first shadowed frame) ms | 27.6, 29.1, 31.6 | **6.9, 7.3, 7.9** (2048 pages: 16.5) | **kept** |
| | | | first-lit frames > 33 ms | 40.9, 40.8, 71.7 | **none** (one run 34.3) | |
| | | | VSM pool + VSM HZB pool MB | 512 + 85.4 = 597 | **128 + 21.4 = 149** (2048 pages: 256) | |
| | | | page-pool overflow ("Virtual Shadow Map Page Pool overflow" warning) | none | **none** in 3 min of play | |
| 6 | TSR at a 200 % history | `r.TSR.History.ScreenPercentage=100` (`DefaultEngine.ini`) | `GPU/TemporalSuperResolution` ms per frame, 3-min match (busy GPU) | 0.634 | **0.330** | **kept** |
| | | | TSR history MB (Color + Metadata + Guide) | 127.5 + 31.9 + 19.3 = 178.6 | **33.8 + 8.4 + 19.3 = 61.4** | |
| 8 | `SetShotCharge` sent every frame | sent inside the change guard; `SetupVisual` sets `LastCharge = -1` so a new figure gets the first value (`JamPawns.cpp`) | `SetShotCharge` calls in 55 s of play (Insights, `-statnamedevents`) | 19 692 (6 per frame), 0.026 ms per frame | **0** in the window (sent on change only) | **kept** |
| | | | figure Blueprint events per frame (`ExecuteUbergraph_BP_SkaterVisual_TB1`) | 54.1 | **42.2** | |
| | | | game thread ms, 3-min match (CSV) | 2.20 | 2.48 | the whole-thread figure is CPU noise from other lanes (+-0.3 ms between runs); the change is 0.026 ms |
| 9 | HUD `NativeTick` sets every text / font / brush each frame | setters only when the value differs (`HUDWidget.cpp`) | `JamHudWidget` Tick + Paint ms per frame (Insights) | 0.094 + 0.080 = 0.175 | 0.090 + 0.086 = 0.175 | **withdrawn**: did not move. `HUDWidget.cpp` is back to main |
| 10 | UI textures stored uncompressed | `PowerOfTwoMode = ResizeToSpecificResolution` 1024x656 / 512x172 on `T_UI_CallCard`, `T_UI_Button_Hover`, `T_UI_Button_Pressed` (`Game/Content/UI`, A's); BC7 was already set | in-memory size (memreport `ListTextures`) | 2 816 + 384 + 384 KB, `PF_B8G8R8A8` | **768 + 128 + 128 KB, `PF_BC7`** | **kept** (-2.5 MB) |
| 5 | Audio-thread flush when the menu world dies (26-39 ms) | none | | | | **not verifiable on this machine**: no audio endpoint (null device) |
| 1, 7 | sky-light readback, Lumen | B's: section 3 | | | | |

What the "after" frame still holds: the Rink map package itself (one synchronous `LoadPackage`, inherent to `OpenLevel`), the world's BeginPlay and the
rink build (`EngineTickMisc` 23-25 ms, unchanged), and two absent packages looked up by files that are not mine (`CommentarySystem.cpp:766` asks for
`MS_crowd_bed`, `FJamUiStyle::Art` for `T_UI_ScorePlate`: one synchronous lookup each per match start; delete the requests or add the assets).

## 2. The second preload pass

The `JAMPRELOAD sync` diagnostic of pass 1 listed what was still loaded in the frame: the Rink map, seven HUD textures (loaded in the menu too, but
collected with the menu world), the two absent packages above and one commentary wave. Pass 2 (`A65-after2`) adds the HUD textures and the commentary
waves to the preload: 428 assets, loaded in 194 ms while the menu is up (`JAMPRELOAD done: 428 assets in 194 ms`; at the Enter: "complete").
Pass 1 and pass 2 interleaved at 10:45-10:48 under the same load (one other lane's game window open):

| run | preload | menu -> match frame ms | FlushAsyncLoading ms | LoadMap(Rink) s | sync loads in the frame |
|---|---|---|---|---|---|
| after1-b | pass 1 | 111.0 | 25.1 | 0.101 | 19 |
| after2-3 | pass 2 | 77.3 | 18.8 | 0.068 | 6 |
| after1-c | pass 1 | 86.6 | 20.4 | 0.075 | 19 |
| after2-4 | pass 2 | 83.9 | 19.4 | 0.072 | 6 |
| before-4 (10:48, no other game open) | none | 176.8 | 94.2 | 0.166 | ~87 |

Pass 2 is a smaller step than pass 1 (5 and 13 fewer synchronous loads; ~1-6 ms of flush, 3-33 ms of LoadMap), but it moved its number in both pairs and
is kept. The cost: the 340 commentary waves (streamed, ~0.4 MB resident per A58) and nine HUD textures stay loaded from the menu on.

## 3. B's offenders: the exact parameters and the expected saving

**Offender 1, the sky-light brightness readback (26-38 ms, twice, on the first lit frames of l2_night, p3_clay, p4_broadcast; 16 MB cubemap).** The stack
is `USkyLightComponent::UpdateSkyCaptureContents -> FScene::UpdateSkyCaptureContents -> ComputeAverageBrightness -> ReadSurfaceFloatData` (a synchronous
GPU readback). In this engine (`SkyLightComponent.cpp:1041`) a sky light whose `IsRealTimeCaptureEnabled()` is true is removed from that capture queue. On
the sky light of `LR_L2_Night`, `LR_P3_Clay`, `LR_P4_Broadcast` (`Game/Plugins/JamAssets/Content/Rigs`):
- `Mobility = Movable` (or Stationary), `bRealTimeCapture = True`, `SourceType = SLS_CapturedScene`, `Cubemap = None` (drops the reference to
  `/Engine/MapTemplates/Sky/DaylightAmbientCubemap`, 16 MB FloatRGBA).
- The same on the persistent sky light of `Game/Content/Maps/Rink.umap` (hidden at run time by `LookTierSwitcher.cpp`), or delete it if the rigs light
  every tier (l1_day has no readback today, so its rig needs nothing).
- Alternative that keeps today's static look: this engine's `USkyLightComponent::SetPreFilteredSky(Cube, Irradiance, AverageBrightness)` with a cube baked
  from the light's own capture (`r.SkyLight.UsePreFiltered 1`): no capture and no readback at run time.
- Expected: **56-72 ms less per match start** (both readbacks) and **16 MB** of memory; cost of real-time capture ~0.1-0.3 ms GPU per frame (time-sliced;
  B checks the night ambient level on the stills: it must not change).

**Offender 7, Lumen GI (2.48 ms GPU, the largest per-frame cost).** Parameters (as `[SystemSettings]` lines or a scalability group; B decides because the
bounce light is the look): `r.Lumen.ScreenProbeGather.DownsampleFactor=32`, `r.LumenScene.Radiosity.UpdateFactor=128`,
`r.Lumen.ScreenProbeGather.RadianceCache.ProbeResolution=16` (or `sg.GlobalIlluminationQuality=2`). Expected per A58: 0.8-1.2 ms GPU per frame. **My A/B
of exactly these three on the baseline package did not show it**: `GPUTime` 3.59 (`lumbase-60`) vs 3.72 ms (`lumen-60`), 60 s each, busy GPU (another
lane's game open); the CSV has no Lumen GI column in this build (`GPU/LumenScreenProbeGather` reads 0.005), so the saving must be measured by B with a
GPU Insights trace (`-trace=gpu`, `gpu-passes.mjs`) on a quiet GPU before it is adopted. Still: `docs/experiments/img/a65/lumen-b-params-l2_night.png`.

## 4. Stills

`docs/experiments/img/a65/`: `before-l2_night.png` (baseline), `vsm1024-l2_night.png` and `vsm2048-l2_night.png` (shadow pool; the far end of the rink and
the far skater's contact shadow show no loss at 1024, and the engine logged no page-pool overflow in 3 minutes), `tsr100-l2_night.png` (TSR 1080p history:
the jersey numbers and the ink outlines stay crisp), `after-l2_night.png` (the after package). Each still is taken at sim frame 600 + a few frames of wall
time, so the framing differs slightly between runs (the sim is the same; the capture moment is not): compare edges and shadow edges, not positions.
The stills were read by eye in this lane (bug-hunt eyes: shadows, edges, banding); no visible loss.

## 5. The remaining frame budget per tier (16.67 ms frame, 60 Hz locked)

A58's per-tier means with this lane's measured deltas applied (TSR -0.30 ms GPU on every tier with TSR; VSM pool commit is a start-only cost; the
game-thread change is 0.026 ms):

| Tier | Game thread ms (left) | Render thread ms (left) | GPU ms (left) | Note |
|---|---|---|---|---|
| l2_night | 1.62 (15.0) | 3.10 (13.6) | ~3.95 (12.7) | default |
| p3_clay | 1.77 (14.9) | 3.68 (13.0) | ~4.86 (11.8) | p99 GPU 10.3 is the tightest figure of the game |
| p4_broadcast | 1.54 (15.1) | 3.14 (13.5) | ~4.65 (12.0) | |
| l1_day | 1.78 (14.9) | 3.07 (13.6) | ~3.07 (13.6) | busy GPU in A58's run |

The per-frame budget was never the problem on this machine; the match start was, and it is now ~80 ms in one frame with no first-lit hitch.

## 6. Files and how to repeat

- `tools/perf/start.ps1 -Exe <ThirdJam.exe> -Tier l2_night -Name <run> [-Dp cvar=value,...] [-Memreport] [-Shot <png>] [-PlaySeconds 180] [-Trace]`
  (one run, remote console on port 4665), then `node tools/perf/startframes.mjs tools/perf/out/<run>` and `node tools/perf/loadtime.mjs tools/perf/out/<run>`.
- Game-thread per timer: a `-Trace -TraceChannels cpu,frame,bookmark -PlaySeconds 60` run, then
  `tools/perf/insights-export.ps1 -Trace <run>/trace.utrace -Cmd "TimingInsights.ExportTimerStatistics {out}\gt-play.csv -threads=GameThread -startTime=<s> -endTime=<e>"`.
- Runs of this page: `{before-1,before-2,before-play180,vsm1024-1,vsm1024-2,vsm2048-1,tsr100-1,after-1,after-play180,trace-before,trace-after,lumbase-60,lumen-60,after2-1..4}`
  (raw output moved to the gitignored `Game/Packaged/perf-out/a65/`; the summaries are in `tools/perf/results/a65/`: `startframes.json|txt`, `loadtime.txt`,
  `gt-play-before|after.csv` (Insights game-thread timers), and per run `events.txt`, `run.json`, `jampreload.txt`).
