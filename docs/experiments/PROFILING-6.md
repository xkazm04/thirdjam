# Profiling 6: the operator's first match starts clean (lane A150, track Q3 evolution 5, device A, Sat 14 Nov 01:29 - 03:19 CEST)

Follows `PROFILING-5.md` (lane A137). Tree: main `322e6c5` (the warm scene and preloads of evolution 4, the new menu with PLAY NOW, the regular cut on the
clay tier). Packages of this tree: `Game/Packaged/A150` (main as landed), `A150c` (this lane's change). Every run goes through **PLAY NOW's own path**
(`jam.menuscreen play` on the remote console, port 4706; no posted input, no cursor): `tools/perf/bot.ps1 -StartPath play`. The p3 runs carry **no
`-cut`**: the menu's own guess and `hockey.regular` exactly as the operator gets it (`-NoCut`); the l2 runs take `perf.l2_night_5v5` through the same
press. A/B inside one package by `-dpcvars`, interleaved (`tools/perf/ab.ps1 -StartPath play -OperatorP3`), six runs a tier per arm, 5 v 5.
Status log: `status/A150.md`.

**Machine during the A/B** (01:51-02:11, 30 s samples in `tools/perf/out/a150-load.csv`, local): 23-42 GB of 63 GB, CPU mean 90 %, 0-13 other
`UnrealEditor-Cmd`, at most one other `ThirdJam`. Interleaving shares the load between the arms.

**Verdict.**
1. **The tier guess was already right** on a clean profile: `LikelyNextCut` is `hockey.regular`, whose cartridge says `p3_clay`, so evolution 4's warm
   scene streamed `LR_P3_Clay`. It ignored the settings' look tier override, which the match applies (`DirectorsCallController.cpp:140`); it now takes it.
2. **What the first match draws, warmed with the rink**: a figure of each team (the skinned vertex factory: a plane never covered it), the puck, lane A136's
   board caps (the instanced vertex factory), the HUD's two typefaces. PSO precache misses **25-31 graphics + 2-7 compute -> 14-18 + 0-4**; the worst of the
   first 60 frames **median 44.9 -> 39.9 ms**, clean runs 1 -> 4 of 12. **The bar (6 of 6 under 33 ms, misses under 10) is not reached.**
3. **The load frame is under 100 ms in 12 of 12** (6 of 6 a tier; was 5 of 12), median **101.6 -> 82.2 ms**: the three synchronous loads of the
   match-start frame are gone in every run (`SM_Arena_BoardCap`, not in the preload since lane A136; `T_UI_ScorePlate` twice, an absent texture looked up
   in that frame).
4. **What removes the rest is a pipeline cache**: with the Shipping default `r.ShaderPipelineCache.SaveUserCache=1`, the second launch precompiles 92-94
   pipeline states at start and the match creates **none**; its first 60 frames had 0 and 1 frames over 33 ms.

## 1. What was built (`EffectsBinder.cpp`, `UJamPreloadSubsystem`)

`jam.WarmScene.Match` (new, default **1**; 0 = evolution 4's warm scene and guess, the exact start before):
- **The guess**: `StartPreload` keeps the cartridge's tier for the figure and the preload's paths, and moves the rig (`LikelyTier`) to the settings' look
  tier override when one is set (`FJamSettings::Load().LookTierOverride`), as the match does.
- **The warm scene's match items** (`WarmMatchItems`, called when the warm scene goes up, taken down with it after `jam.WarmScene.Frames`): two `AJamSkater`
  with `InitSkater(team, team, the cartridge's team colour, 7 / 9, the visual class the match resolves)`, found with `FindObject` among the preload's
  classes (`AHokejGameMode::VisualClassPathsFor`), 4.8 m ahead of the menu camera either side; an `AJamPuck` with the cartridge's radius and colour; an
  `UInstancedStaticMeshComponent` of `SM_Arena_BoardCap` with two instances, stencil 1 as on the rink. Log: `JAMPRELOAD warm scene up: rig p3_clay
  (streaming in), 22 items in front of the menu camera, 11 match items (figure humanoid_b1), 90 frames`.
- **The HUD's typefaces**: Slate loads a face file synchronously at first use; `Roboto-BoldCondensed` and `Roboto-Black` loaded in the match-start frame
  (`Synchronously lazy loading font file` there in every run). The warm scene measures a sample string in the HUD's fonts (`FJamUiStyle::ScoreFont`,
  the outlined Black and Bold fonts): the faces load behind the menu.
- **The preload** (`MatchPaths`): `ARinkActor::BoardCapMeshPath`, `/Game/UI/T_UI_ScorePlate` (absent today: the existence check files it as known missing)
  and `/Water/Materials/MPC/MPC_Water` (named by the brief; not asked for on this tree in any run). The absent score plate is looked up once behind the
  menu (`FJamUiStyle::Art` remembers an absent texture for the process), so the HUD's build no longer tries to load it in the match-start frame.
- Tools: `bot.ps1 -StartPath play` (PLAY NOW) and `-NoCut`; `ab.ps1 -StartPath play -OperatorP3`; `syncrows.mjs` line 16 skipped every load within 60
  frames of the match start (`fr <= start + 60`), which hid the match-start frame itself: it now lists them from the start frame on, tagged `START`;
  `reactrows.mjs` (new): each `JAMRINK react goal|board|rattle` line on its CSV row and the 10 rows after it against the play's median and the base rate.

## 2. Items 1 and 2: the A/B (`A150c`, `jam.WarmScene.Match=0` vs `=1`, `a150-ab-*`, rows in `tools/perf/out/a150-ab-firstsec.txt`)

`firstsec.mjs`: the load frame (the longest game thread among the first 30 rows), the worst of the 60 frames after it, the CSV's PSO counters; ms.

| run | old: load frame / worst of first 60 / misses g + c | new: load frame / worst of first 60 / misses g + c |
|---|---|---|
| l2_night #1-6 | 89.1 / 42.9, 133.6 / 54.1, 97.8 / 44.5, 102.0 / 44.2, 101.4 / 51.4, 94.5 / 35.9; 25-31 + 2-7 | 60.2 / 42.0, 99.4 / 41.6, 84.9 / 58.9, 86.7 / 38.1, 75.8 / none, 85.7 / none; 14-18 + 0 |
| p3_clay (hockey.regular) #1-6 | 88.6 / 45.2, 122.5 / 141.1, 107.1 / 65.0, 83.7 / 45.2, 101.7 / 35.4, 154.6 / none; 26-27 + 6 | 77.0 / 37.0, 96.2 / 46.3, 78.9 / 44.6, 74.1 / 46.6, 81.1 / none, 83.3 / none; 14-15 + 4 |
| load frame median / under 100 ms | 101.6 / 5 of 12 | **82.2 / 12 of 12** |
| worst of first 60 median / runs with none over 33 | 44.9 / 1 of 12 | 39.9 / 4 of 12 |
| synchronous loads from the match start (`syncrows.mjs`) | 3 in 12 of 12 (`SM_Arena_BoardCap`, `T_UI_ScorePlate` x2) | **0 in 12 of 12** |

`PSO/GraphicsPSOHitchTime` was 0 in every run of both arms. What is left in the first rendered frame is no thread's own work: game thread 4-6 ms, render
thread 6-8 ms (its `EventWait/Visibility` 12-18 ms), GPU passes about 11 ms inside a GPU time of 38-45 ms (the CSV row after the load frame): the GPU waits
while the RHI thread (9.5-12 ms) submits, and the 14 untracked graphics pipeline states are created at their first draw. Section 4 shows they are the cost.
The load frame's remainder (`loadrow.mjs`): `EventWait/Audio` 21-39 (LoadMap's audio flush, ruled to stay), `EventWait` 33-40 (the render thread),
`EngineTickMisc` 22-57 (the world's BeginPlay); LoadMap's garbage collection 4-31 ms in 6 of 24 runs, both arms (2 new, 4 old); the light hold's
blocking rig flush (`FlushAsyncLoading` 22-39 ms) in one run a tier of each arm, the same pair (#2).

## 3. Item 3: the arena's reactions in a 5 v 5 package match (the reactions in this tree)

The board-hit flash light and the goal's wave of lights land with the arena lane's fifth evolution after this tree was cut: not measured here (section 6).
In the tree: the goal's lamp and its red point lights (lanes A127, A136) and the board caps' ripple on a hit (lane A136). `reactrows.mjs` over the 24 A/B runs
(20 s of play each, 1650-1730 rows in a full window):

| reaction | count | worst frame in the 10 rows after: median / max | windows with a frame over 33 ms | base rate (any 11-row window in play) | GPU max over the play median |
|---|---|---|---|---|---|
| board cap ripple, l2_night | 188 | 16.9-17.4 / 49.8 (one 460.8: a machine stall) | 7.3 % old, 17.4 % new | 5.9 % old, 9.5 % new | +0.4-0.8 ms |
| board cap ripple, p3_clay | 189 | 16.9-17.3 / 48.1 | 2.1 % old, 9.5 % new | 6.4 % old, 5.2 % new | +0.6 ms |
| rattle of a piece | 24 | 16.8-18.1 / 24.6 | 0 | | +0.2-0.9 ms |

The ripple moves instance transforms (CPU); no GPU pass stands out after a hit (`GPU/RenderDeferredLighting` and TSR rise by 1-3 ms in the busiest
windows, where play itself is busiest). The windows over 33 ms after a hit follow the base rate of the loaded machine within its scatter; they carry
render-thread `EventWait` at an idle GPU, as the machine's hitches do everywhere in play.

**The goal** (no goal fell in the A/B's 20 s windows; four 80 s runs of the new arm, `a150-react-*`): p3_clay 2 goals in one run, the worst frame in the
10 rows after them 25.9 and 17.0 ms, none over 33; on the goal's frames the GPU rose 4.5 ms over the play median to 9.3 ms (`GPU/Basepass` +2.2, `GPU/Lights`
+1.8: the lamp's emissive and the two red point lights, TSR +1.5), inside a 16.7 ms frame. l2_night: no goal in three 80 s runs (the night perf cut's bots
did not score); its goal row is still to take, with a drill (`jam.ctrl.Drill`) or a longer window. The board ripple in the four 80 s runs: 123 hits, 1 window over
33 ms (0.8 %; base rate 3.1 %), GPU +0.34 ms (median) over the play median.

Budget at 5 v 5 in the new arm (`five.mjs`, full play windows): frame mean 16.7-17.6 ms, p99 17.5-34.6; game thread 1.9-3.0 / p99 3.0-5.4; render thread
3.7-5.0 / p99 5.6-10.7; GPU 4.4-5.7 / p99 6.4-11.0.

## 4. Item 4: a Shipping player's first launch against the second

The release chain's last Shipping package (`Hokej_Hokej_H26`, main `a1c712f`, from Friday 19:46) predates the warm scene and has neither the CSV profiler
nor the remote console, so a Shipping run cannot be read cursor-free. Measured instead is what makes Shipping different: the engine saves the user
pipeline cache in Shipping only (`r.ShaderPipelineCache.SaveUserCache`). The same switch on `A150c` (`-dpcvars=r.ShaderPipelineCache.SaveUserCache=1`),
the package's own `Saved` cleared first (a clean profile for the game; the GPU driver's own shader cache is the machine's and was not touched), three
launches in a row through PLAY NOW (`a150-psocache-1..3`):

| launch | what the cache did | new PSOs logged in the match | worst of first 60 frames | frames over 33 |
|---|---|---|---|---|
| 1 (clean) | none at start; `Saved/ThirdJam_PCD3D_SM6.upipelinecache` written at exit (40 KB) | 11 | 46.7 | 1 |
| 2 | `FShaderPipelineCache starting pipeline cache 'ThirdJam_usr' and enqueued 92 tasks for precompile` | **0** | under 33 | **0** |
| 3 | 94 tasks precompiled | **0** | 39.0 (the first rendered frame) | 1 |

(The CSV's `PSO/PSOMisses` stays at 15 + 4 in all three: it counts pipeline states the precache system did not request, whether or not the file cache
compiled them already.) **A Shipping player's second launch starts clean of new pipeline states; the first does not.** For the operator at 10:00: a
Shipping build launched once on his machine and profile, played into a match through PLAY NOW and quit normally, leaves the cache his launch then uses.
The Shipping user cache lives in the profile's `Saved` folder (look for `ThirdJam_PCD3D_SM6.upipelinecache`).

## 5. The gate

Full `Jam` filter **215 of 215** (149.6 s, `Game/logs/a55-automation-ue-20261114-014659.log`). `Jam.Core.Determinism` hockey.regular seed 1 frame 600
`ef34dd88c7c62b47`. The change is presentation only and runs only in packaged and `-game` builds (`TickWarmScene` returns under `GIsEditor`; the preload's
rig and the guess feed nothing the sim reads). The Shipping game target compiles with the change (`tools/run/build.ps1 -Config Shipping`, 80 s, PASS).

## 6. Next

1. **A bundled pipeline cache** recorded from a bot match of each tier (`-logPSO` on a Development package, `ShaderPipelineCacheTools expand` into the
   project's `Build/Windows/PipelineCaches`, cooked in): the first launch then has what section 4's second launch had. Proof: new PSOs logged in the
   match 0 in 6 of 6 runs a tier on a clean `Saved`, the worst of the first 60 frames under 33 ms in 6 of 6.
2. **The arena lane's board flash and goal wave** (on main within the hour of this lane): `reactrows.mjs` over 6 runs a tier of 80 s with the arena's
   new log lines (`JAMRINK react ... flash` / `wave`; add their names to the script's regex), the flash's `GPU/Lights` delta over the play median at or
   under 0.1 ms, no frame over 33 ms in the 10 rows after a goal or a hit above the base rate; the four new meshes: 0 synchronous loads with `syncrows.mjs`.
3. **The first rendered frame's GPU bubbles**: one Insights trace (`bot.ps1 -Trace`, `rhi,gpu,frame`) of the first second with the user cache warm, to see
   whether the 38-45 ms GPU time without new PSOs is render target allocation (TSR history, shadow maps at full resolution) that a warm scene drawn
   through the match's own view settings would take earlier.
