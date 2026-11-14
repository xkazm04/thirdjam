# Profiling 8: the operator's first start on H28, a goal on a set frame, the figure's tick, the cook guard (lane A175, track Q3 evolution 7, device A, Sat 14 Nov 06:00 - 07:40)

Follows `PROFILING-7.md` (lane A161). Tree: main `ca493723` (A161 landed as `a46c8081`, the dev-b merge in it). Package measured: the acceptance twin
of the operator's build, `Game/Packaged/Accept_H28` (Development; read-only, copied before every run), standing in for `Hokej_Hokej_H28` (Shipping:
no CSV profiler, no remote console, no log, so it cannot be read without a window). Every run off screen (`bot.ps1 -Offscreen`), port 4775, PLAY NOW's
own path. Status log: `status/A175.md`.

## 1. The first start, cold against warm

`tools/perf/a175-cold.sh`: each round copies the package into a new folder without `Saved/` (a fresh unzip into a new folder), launches it twice
through PLAY NOW with no cut (the operator's `hockey.regular` on clay) and `r.ShaderPipelineCache.SaveUserCache=1` (the Shipping default; Development
leaves it off): launch 1 cold, launch 2 warm. The first 120 frames after the load frame (`firstsec.mjs --rows 120`), three rounds:

| round | cold: over 33 ms / worst / new PSOs in the match | warm: over 33 ms / worst / new PSOs | load frame cold / warm |
|---|---|---|---|
| 1 | 1 / 49.0 / 10 | 1 / 33.7 / 0 | 84.7 / 98.3 |
| 2 | 1 / 36.6 / 11 | 3 / 64.6 / 0 | 100.4 / 88.6 |
| 3 | 1 / 52.2 / 11 | 1 / 44.0 / 0 | 107.5 / 95.0 |

- Pair ratio of the worst frame cold / warm: 1.45, 0.57, 1.19 (median **1.19**); frames over 33 ms: 3 cold against 5 warm in all.
- The warm launch precompiles 93-95 pipeline states at start ('enqueued 93-95 tasks for precompile') and creates none in the match; the cold one
  creates 10-11, and `PSO/GraphicsPSOHitchTime` is 0 ms in all six. Synchronous loads from the match start: 0 in 6 of 6.
- **Verdict: on this machine a fresh unzip starts no worse than a second launch** beyond scatter. The 10-11 pipeline states it creates cost nothing
  measurable, because the GPU driver's own shader cache is machine-wide and already holds this content. That cache is the one difference this test
  cannot reproduce: clearing it is a system setting, which I did not touch. The operator's machine, if this content has never run on it, starts cold
  there too.
- **What buys it back**: no project setting closes a cold driver cache. Only a **bundled pipeline cache** (recorded with `-logPSO` from a bot match
  per tier, expanded into `Build/Windows/PipelineCaches`, cooked in) makes the first launch precompile what launch 2 does here. Without it, the
  practical move is the one from PROFILING-6: launch once on his machine, play into a match, quit normally.
- The load frame itself (the level open) is 85-108 ms, median 96.7 (A150's main before B: 82.2; A161 measured the light hold growing with the
  rig's 293 more actors): a single frame under the start's fade, unchanged by the cache.

## 2. A goal on a set frame (B's b and c)

`tools/perf/cuts/perf.p3_clay_goals.cartridge.json`: the clay 5 v 5 perf cut with 10 m nets, `skater.goalie.reach_m` 0 and a 60-frame goal pause.
The sim is deterministic, so the goals fall on the same sim frames in every run (784, 1470, 3238, 4693 in 100 s with the scripted human):
**the arms pair goal by goal** (`tools/perf/goalpair.mjs`: the goal's row and the 10 after, same round, same sim frame).
`tools/perf/a175-goals.sh`: arms `jam.rink.react.light` 6000 (the loud goal as shipped), 800 (B41's clay candelas), 0 (no light); two rounds
interleaved, order reversed in round 2.

| paired at the goal (median of goal pairs) | 800 cd / 6000 cd (8 goals) | no light / 6000 cd (6 goals) |
|---|---|---|
| frame | 1.00 | 1.00 |
| game thread | 1.01 | 1.05 |
| GPU | 1.03 | 0.97 (-0.17 ms) |
| GPU/Lights | 1.03 | **0.80 (-0.15 ms)** |
| GPU/RenderDeferredLighting | 1.00 | 0.90 (-0.11 ms) |

- **The loud light costs about 0.15 ms of GPU on a goal's frames** (the lights pass). Candelas do not move it: 800 and 6000 cd cost the same,
  because a light's cost follows its radius, not its brightness. B's radius 350 is not a console variable (the light is built in `ARinkActor` at
  1600 cm), so it was not measured directly; it can only cost less than the 0.15 ms of the light as it stands.
- The worst moment at a goal: the worst frame in the goal's 11 rows was median 23.6 / max 41.9 ms (6000), 18.1 / 27.2 (800), 18.8 / 115.7 (0);
  GPU 5.6-6.0 ms, game thread 2.5-2.6. The frames over 33 ms come in both light arms and paired against each other they carry no GPU difference:
  they are game- or render-thread stalls, not the light. The slide trail's sprites were not isolated (no slide was scripted onto a goal frame).

## 3. The figure Blueprint's tick (B's d)

`tools/perf/a175-trace.sh`: an Insights trace (`cpu,frame,bookmark`, `-statnamedevents`) of the clay 5 v 5 match, ten `BP_SkaterVisual_TB1_P3`,
25 s of play, off screen, exported headless (`insights-export.ps1`, game thread timer statistics):

| timer | calls | per call (avg / median / max) | per frame for ten figures |
|---|---|---|---|
| `UpdateMotion` | 21,920 (about 2,190 frames x 10) | **0.064 / 0.076 / 7.5 ms** | **about 0.64 ms** |
| `ExecuteUbergraph_BP_SkaterVisual_TB1_P3` (inclusive) | 200,502 | 0.008 / 0 / 7.5 ms | about 0.72 ms |
| `ReceiveTick` | 25,026 | 0.006 ms | 0.07 ms |

The figure's `UpdateMotion` costs **0.06-0.08 ms per figure per tick**, and the figures' Blueprint about 0.7 ms of a game thread whose median is
2.5-3.0 ms. That is about a quarter of the game thread, and inside the budget. Named events inflate small scopes, so these are upper bounds. The
7.5 ms maximum is a single call (the first tick after spawn).

## 4. The cook guard (a gate step, proposed, not wired)

`tools/size/cookguard.mjs`: `nevercook.mjs` in check mode (the ini's never-cook block equals B's unshipped families as they are on disk) plus
`refwalk.mjs` (no shipped root reaches a never-cook package, no cooked package names one); PASS/FAIL lines and a `thirdjam.gate/1` JSON line; no
Unreal start.

| case | result |
|---|---|
| the tree | PASS in 3.0 s |
| planted reference: a fake `/Game/UI/A175_PLANT.uasset` (always-cook folder) naming `SK_SkaterTB2_P3` | **FAIL** in 1.7 s: "/Game/UI/A175_PLANT -> SK_SkaterTB2_P3 (cooked by AlwaysCook /Game/UI)" |
| drift: the block without `SK_SkaterGen_P4E` (as when B adds an asset) | **FAIL** in 2.4 s: "missing from the ini: ... SK_SkaterGen_P4E" and the three holes it opens |
| plant deleted, ini restored byte-identical | PASS |

Proposed for `tools/gate/premerge.ps1` beside the asset ledger check: `node tools/size/cookguard.mjs` (the Director decides).
**Finding**: the A161 landing carried the ini block but not `tools/size/nevercook.mjs`, `refwalk.mjs` or `cookonly.sh`. They are restored in this
tree.

## 5. B's five timings as they stand (for device B)

| | timing | result | source |
|---|---|---|---|
| a | four shadowed key spots on clay, shadow resolution 1.0 vs 0.5 | shadow depths 0.38 vs 0.31 ms (ratio 1.23); GPU total no change (0.99), frame 1.00. Moving lights are already at half resolution in the project | A161, paired in one match |
| b | goal light 6,000 cd 16 m vs B's numbers | the light costs about 0.15 ms of GPU on goal frames (no light / 6000: lights pass 0.80); 800 cd = 6000 cd (1.03); radius 350 not settable without code, can only cost less | A175, 6-8 goals paired by sim frame |
| c | the worst moment | goal frames: GPU 5.6-6.0 ms, game thread 2.5-2.6, worst frame median 18-24 ms (the stalls are threads, not GPU); board hits: worst 18.6, GPU +0.8 ms max over play. Slide trail not isolated | A161 + A175 |
| d | the figure Blueprints' `UpdateMotion`, ten figures | 0.064 ms per figure per tick (median 0.076), about 0.64 ms a frame for ten; the figure's whole Blueprint about 0.72 ms | A175, Insights trace |
| e | `M_Ice` on clay, full screen, 1920 x 1080 | at most the whole base pass: 0.37-0.42 ms median (p99 1.6) | A161, upper bound |

## 6. Next

1. **A bundled pipeline cache** for the Shipping package (the only lever on a cold driver cache). Proof: a cold launch on a fresh unzip creates 0
   pipeline states in the match, 6 of 6.
2. **The slide trail on a goal frame**: a scripted knockdown slide timed to the goal cut's first goal, sprites counted. Proof: the GPU of B's
   worst moment as one paired number.
3. **The cook guard in the gate**, if the Director wires it. Proof: it runs in each premerge in under 5 s and fails on B's next unshipped figure.
