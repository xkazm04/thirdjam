# Profiling 2: the frame at five a side with the vision (lane A106, track Q3, device A, 2026-11-13 18:47-20:15 CEST)

Lanes A58 (`PROFILING.md`), A65 (`PERF-FIXES.md`) and A83 (`FIVE.md`) profiled 3 v 3 and the first 5 v 5. This page is the frame of main ee14438 (contact by inertia,
the movement laws, the hockey brain of A94, the ladder of R-0046) at 5 v 5 on l2_night and p3_clay, **cursor-free**: a Development package of this tree
(`Game/Packaged/A106-dev`; `A106-after` for the HUD A/B), driven by the remote console on port 4706, bot matches only (`jam.ScriptHuman=1`: all ten skaters
are bots), no posted input. Status log: `status/A106.md`.

**Verdict.** The game thread costs **2.4 ms** of the 16.7 ms frame at 5 v 5 on both tiers, the render thread 4.4-4.6, the GPU 4.7-4.8 (busy machine). **The brain is
not a cost**: 0.013 ms per frame in the engine (CSV `Jam/GameThread/BrainThink`, Insights `JamBrainThink`), 0.0017 ms natively; the space read is 0.14 us per call.
No stagger: it would change when the brain decides for a saving nobody can measure. The largest game-thread item is the figure Blueprint (`BP_SkaterVisual_TB1`,
**0.59 ms, 81 calls per frame** for ten figures, inside the game mode's tick of 0.77 ms), which Q3 does not own. In Q3's family the HUD tick (0.10 ms) is the top
item; a trim of its per-frame setters saved 0.005 ms and was withdrawn (section 3). **The match-start frame is no longer under 100 ms at 5 v 5**: 96-191 ms over six
runs, driven by the Rink map's own synchronous load (15-88 ms) and the world's BeginPlay with ten figures (31-59 ms). Hitches in play are machine noise on
this shared box: 127 of 139 have no game thread, render thread or GPU over 30 ms.

## 1. Method

| What | How |
|---|---|
| Driver | `tools/perf/bot.ps1` (new): A65's `start.ps1` without `post.ps1`. The cut is on the command line (`-cut=`), the menu comes up, a fixed 8 s (the preload runs), `csvprofile start`, then the remote console's `open /Game/Maps/Rink` (the same `OpenLevel` the menu's PLAY does). CSV from the menu through the load frame and `-PlaySeconds` of play. Port 4706. |
| Cuts | `tools/perf/cuts/perf.l2_night_5v5` and `perf.p3_clay_5v5` (A83's): hockey.regular's game, five a side, `rules.mode timer`, 12 000 frames |
| Budget row | `node tools/perf/five.mjs <run>` (new): play = the CSV rows from 120 after the load frame; mean / p99 of frame, game, render, GPU; brain; draws; prims; hitches > 33.3 ms; the top exclusive stats |
| Match start | `node tools/perf/startframes.mjs <run>` (A65's) |
| Timers | Insights trace (`-Trace`, channels cpu, gpu, frame, bookmark), `tools/perf/insights-export.ps1 ... ExportTimerStatistics -threads=GameThread -startTime=40 -endTime=150`. The traced run's hitches are the profiler's own (CSV `EndFrame` 640 ms, `WaitForStats` 454 ms): read timers from it, never hitches |
| Brain, native | `node tools/perf/brainbench/run.mjs 3 7200` (new): clang -O2 of the tournament driver's sources, three bot matches recorded, `FJamBrain::Think` and `ReadSpace` timed over every recorded world |
| Mid-match sync loads | `node tools/perf/syncrows.mjs <run>` (new): every `JAMPRELOAD sync` after the match start with its CSV row |
| Machine | RTX 4090, 1920 x 1080 windowed. No other game or editor window at the start or end of any run; other lanes' headless builds, cooks and soaks ran throughout (63 GB box at 40 GB in use). GPU figures are labelled busy. |

## 2. The budget at 5 v 5 (CSV-only runs, 120 s of play, `A106-dev`)

| tier | frame mean / p99 | game thread mean / p99 | render thread mean / p99 | GPU mean / p99 (busy) | brain | draws | prims | hitches > 33 ms in play |
|---|---|---|---|---|---|---|---|---|
| l2_night 5 v 5 | 18.07 / 38.5 | **2.41** / 7.32 | 4.42 / 14.4 | 4.81 / 8.53 | **0.014** / 0.034 | 338 | 115 k | 139 (127 with no thread over 30 ms) |
| p3_clay 5 v 5 | 17.91 / 38.8 | **2.41** / 6.39 | 4.56 / 13.4 | 4.68 / 9.39 | **0.013** / 0.035 | 433 | 197 k | 142 |
| A83, l2_night 5 v 5 (scripted human, busy) | 17.50 / 35.5 | 3.24 / 7.12 | 5.22 / 13.0 | 4.48 / 8.55 | | 325 | 114 k | 128 |
| A83, p3_clay 5 v 5 | 16.83 / 19.1 | 2.91 / 4.56 | 4.74 / 6.67 | 4.07 / 8.84 | | 412 | 193 k | 20 |

Game thread per system, l2_night 5 v 5, Insights over 6 348 frames of play (ms per frame, inclusive):

| timer | ms / frame | calls / frame | owner |
|---|---|---|---|
| `HokejGameMode` tick (sim, brain, gather, events, presentation push) | 0.766 (exclusive 0.092) | 1 | A's game mode |
| of it `ExecuteUbergraph_BP_SkaterVisual_TB1` (the figure Blueprint) | **0.586** | 80.6 | B's Blueprint, driven from `JamPawns.cpp:380` (`UpdateMotion` every frame) |
| Slate total (HUD, radar, engine) | 0.496 | 2 | |
| `JamHudWidget` Tick | **0.098** | 1 | Q3 (`HUDWidget.cpp`) |
| `JamHudWidget` Paint | 0.066 | 1 | |
| animation (game thread) | 0.127 | 26.6 | |
| Niagara manager (game thread) | 0.040 | 10 | |
| **`JamBrainThink`** | **0.013** | 1 | Q3 counts it, Q2 owns what it decides |
| the sim step (headless, `Jam.Core.Determinism`: 600 frames of hockey.regular) | 0.0055 | 1 | Q1 |

Native brain (`brainbench`): 3 v 3 Think mean 1.10 us (p99 2.4); **5 v 5 mean 1.69 us, p99 3.8 us, max 31.9 us**; `ReadSpace` 0.14 us per call. The brief's
threshold for a stagger was 0.3 ms: the brain is 20x under it in the engine and 180x natively.

## 3. The HUD: where its 0.1 ms goes, and a trim that was withdrawn

`HUDWidget.cpp` now carries Insights sub-scopes for its parts (`JamHud_Tick`, `_Refresh`, `_EffectsFeed`, `_Radar`, `_NetMarkers`, `_Yours`, `_PuckMarker`,
`_Commentary`; no behaviour change). The trim tried in the `A106-after` package (`jam.hud.Trim`): score, take, timer, ability and countdown texts set only when
their string changed, the countdown font only when its style changed, the YOU chevron and tag brushes only when the team colour changed. Same package, A/B by
`-dpcvars`, l2_night 5 v 5, traced, Insights 40-95 s (`tools/perf/results/a106/hud-ab.txt`):

| ms per frame | Trim 0 (the HUD as on main) | Trim 1 |
|---|---|---|
| `JamHud_Tick` (all of the HUD's own tick) | 0.0980 | 0.0928 |
| `JamHud_Refresh` exclusive (texts, plates, countdown) | 0.0321 | 0.0256 |
| `JamHud_Yours` (ring, chevron, tag) | 0.0253 | 0.0260 |
| `JamHud_NetMarkers` (far and own net) | 0.0248 | 0.0255 |
| `JamHud_PuckMarker` + edge slots | 0.0100 | 0.0100 |
| `JamHud_Commentary`, `_Radar`, `_EffectsFeed` | 0.0022, 0.0015, 0.0005 | 0.0020, 0.0014, 0.0006 |
| `JamHudWidget` Paint | 0.070 | 0.072 |

**Withdrawn**: 0.005 ms per frame is real for its own scope (-20 % of Refresh's own work) and nothing for the frame; a per-widget cache that can go stale is not
worth 5 us (A65 found the same in its HUD attempt). The HUD's whole tick is 0.6 % of the frame; the projections (net markers, YOU ring) are half of it.

## 4. The match-start frame (is it still under 100 ms?)

| run | load frame ms | FlushAsyncLoading ms | EngineTickMisc ms | first-lit frames > 33 ms |
|---|---|---|---|---|
| l2_night 5 v 5 #1 | 96.2 | 23.4 | 31.6 | 37, 63, 35 |
| l2_night 5 v 5 #2 | 161.5 | 49.5 | 35.9 | 36, 64, 34 |
| l2_night 5 v 5 #3 | 191.1 | 88.2 | 36.2 | 45, 54, 84, 34, 40, 36 |
| p3_clay 5 v 5 #1 | 168.7 | 61.9 | 31.7 | none |
| p3_clay 5 v 5 #2 | 99.2 | 17.2 | 59.4 | 44 |
| p3_clay 5 v 5 #3 | 107.9 | 15.3 | 31.5 | 58, 66, 45 |
| A65 after (3 v 3, l2_night) | 77-84 | 19 | 23-25 | none |

**No: 2 of 6 runs are under 100 ms.** The preload holds (429 assets complete 0.45-0.53 s after the menu is up; the only synchronous loads in the frame are the
Rink map package itself and the absent `/Game/UI/T_UI_ScorePlate`), so the swing is the map's own load on a disk and CPU shared with other lanes (15-88 ms) plus
the world's BeginPlay, the rink build and ten figures (31-59 ms; 23-25 at 3 v 3). The traced run had a 1992 ms match-start frame in the texture streamer's mip
calculation (`FRenderAssetStreamingMipCalcTask` synchronous completion, 1746 ms, with the GPU's first scene render at 1730 ms): a trace-only artefact here, but the
first-frame texture streaming is the next thing to watch. The first-lit frames (33-84 ms, render-thread bound, GPU up to 59 ms) came back at 5 v 5.

## 5. Mid-match synchronous loads: absent commentary waves

Seven per two-minute match on both tiers, every one an absent package that the commentary picks and then looks up on the game thread: `/JamAssets/Audio/VO/`
`VO_Commentator_check_cross_02/08/09/10`, `check_solid_02/07`, `knockdown_human_06/09/10`, `knockdown_massacre_04/06` ("does not exist on disk or in the loader";
`JAMVO missing`, logged once each). Each lookup costs 2-7 ms of game thread (one 25.8 ms). The lines are in the embedded bank (A89's rough supplement and A92's
contact lines) but their waves are not in the cook: B's to record or ship, or Q2's to skip a line with no wave at pick time (silence today).

## 6. The next offenders, ranked

1. **The match-start frame at 5 v 5** (96-191 ms): the Rink map's synchronous load. Preload the map package from the menu (an async load of `/Game/Maps/Rink` held
   until `PreLoadMap`), and move the ten figures' spawn and the rink build off the load frame. Number: the load frame under 100 ms in 6 of 6 runs.
2. **The figure Blueprint** (0.59 ms, 81 calls per frame for ten): `UpdateMotion` from `JamPawns.cpp:380` every frame per figure plus the Blueprint's own graph;
   B moves the per-frame part to native (`ASkeletalSkaterVisual`) or A sends `UpdateMotion` only on change. Number: `ExecuteUbergraph_BP_SkaterVisual_TB1` under 0.2 ms.
3. **First-lit frames** (33-84 ms render-thread bound at 5 v 5, none at 3 v 3 after A65): a GPU Insights trace of the first 60 frames (PSO precache, skin cache
   for ten skeletal meshes). Number: no frame over 33 ms in the first 60 after the load frame.

## 7. The gate

Full `Jam` filter on this tree: **156 of 156 pass** (`tools/analysis/jamtests.ps1`, 456 s). `Jam.Core.Determinism` hockey.regular seed 1 frame 600 `d192973ac7c97102`,
hockey.raw `b02f67a2da0314b4`: the same as main before (A95's final gate). The landed source change is instrumentation only (trace scopes and one CSV stat).

## 8. Files and how to repeat

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\perf\bot.ps1 -Exe Game\Packaged\A106-dev\Windows\ThirdJam\Binaries\Win64\ThirdJam.exe -Tier l2_night_5v5 -Name <run> -PlaySeconds 120 [-Trace] [-Dp cvar=value,...]`
  then `node tools/perf/five.mjs tools/perf/out/<run>`, `node tools/perf/startframes.mjs tools/perf/out/<run>`, `node tools/perf/syncrows.mjs tools/perf/out/<run>`.
- `node tools/perf/brainbench/run.mjs [seeds] [frames]`.
- `node tools/perf/hudtimers.mjs <insights gt.csv>` (new): the HUD sub-scopes, the brain, the game mode and the figure Blueprint per frame.
- Summaries: `tools/perf/results/a106/` (budget rows, start frames, Insights game-thread timers, the HUD A/B, the native brain bench, the mid-match sync loads). Raw CSVs, traces and logs stay in `tools/perf/out/` (not landed).
