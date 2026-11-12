# B1b report: look fixes checked on the real rink (device B)

Lane B1b, device B, 2026-11-12, 11:17 to 11:40 (box to 12:35). No git commands were run. I drove the lead's editor (port 8000) through `tools/mcp/mcp.mjs`.
At the end the editor is open on `/Game/Maps/Rink` with no dirty packages. PIE is stopped, and `_Scratch` is gone from disk.

**Every frame number below is unconfirmed, because two editors were up** (port 8002 was rendering on the same GPU). Each one is a pair taken within the same minute.
Brightness values are full-range luma (0-255), measured with `ffmpeg crop,scale=out_range=pc,format=yuvj444p,signalstats`.

**Incident:** at 11:23, the plan's T1 script crashed this editor during `build`. It calls `EditorAssetLibrary.duplicate_asset` on the map, then `load_level` while
Python still holds the new UWorld, which raises the fatal error `World Memory Leaks` (`EditorServer.cpp:2543`). I restarted the editor at 11:24 with `editor.ps1` on port 8000 (free memory was 44 GB).
Nothing was lost, because nothing was dirty. The crash dump is in `Game/Saved/Crashes/UECC-Windows-7CBF...`.

## Task 1: the real game today, and the acceptance check of handoff B-to-A-0005

What the game does today:
- PIE on `Rink` runs `HokejGameMode` and loads the cartridge `hockey.regular`: rink 120 x 60, visual `toy`, tier `l2_night`.
- The log shows `JAMLOOK tier=l2_night map=/JamAssets/Rigs/LR_L2_Night.LR_L2_Night`, so the night rig streams in by itself. No workaround was needed.
- The PIE world holds 28 `Rink` actors and 302 `LR_L2_Night` actors.
- What the stills show: painted pools with outlines on the figures and nets, and none on the ice. The side bands and the gaps between the pools were pure black (YAVG 1-4). The blue line was blotchy, and the white net mesh disappeared against the white boards.
- I took the centre-ice, far-end and dark-band views with a PIE-only view target, a moved `GameNetworkManager` actor. For the dark band I paused the game and moved the puck and two figures in PIE only.

| Handoff 0005 row | Handoff asks | Found in PIE (`RinkActor_0`, `JamSkater_0..5`, `JamPuck_0`) | Verdict |
|---|---|---|---|
| ice surface | custom depth on, stencil 2, `MI_Ice_Night` | `Ice` (ProceduralMesh): on, 2, MID of `MI_Ice_Night`. `RinkHalfLength` 6000 and `RinkHalfWidth` 3000 are set by `RinkActor.cpp:193-194`. | match (but see the note on rink size below) |
| boards | on, 1, `MI_Toon_Board` | `Boards`: on, 1, `MI_Toon_Board` | match |
| net frame / mesh | on, 1, `MI_Toon_NetFrame` / `MI_Toon_NetMesh` | `Nets` (one component, sections 0 and 1): on, 1, `MI_Toon_NetFrame` and `MI_Toon_NetMesh` | match |
| toy skater body, head, stick, blade | on, 1; dynamic instances of `M_Toon` with `Emissive` 2.0-2.5 and `RimStrength` 0.6; helmet `MI_Toon_Helmet`; stick `MI_Toon_Stick` | All 8 meshes per skater: on, 1. Body, cap, head and skates are MIDs of `M_Toon`; the helmet band is a MID of `MI_Toon_Helmet`; stick and blade are MIDs of `MI_Toon_Stick`. `RimStrength` 0.6. **`Emissive` is 1.0.** | **mismatch, `Emissive`.** `ToySkaterVisual.cpp:262` sets 2.2, then `SetBoost(NAME_None)` (line 413) resets every MID to 1.0. |
| puck | on, 3, `MI_Toon_Puck` | `JamPuck_0.Mesh`: on, 3, `MI_Toon_Puck` | match. The size is 24 x 8 cm (Cylinder at scale 0.24 / 0.08), not a 12 cm disc. No `MI_Trim_PuckHalo` disc is attached, because B3's puck kit (handoff 0007) is not wired yet. |
| trim strips | off, 0, no shadow; blue for x < 0, orange for x > 0 | `Trim` and `TrimOrange`: off, 0, no shadow, `MI_Trim_Blue` and `MI_Trim_Orange` | match |
| beacons | off, 0, no shadow, `MI_Trim_Beacon`, d 12 cm, 8 m | `BeaconPos` and `BeaconNeg` at x = ±58.5 m, z 4 m, scale 0.12 / 0.12 / 8: off, 0, no shadow, `MI_Trim_Beacon` | match |
| acceptance stills | ink on boards, nets, figures and puck; banded pools; no ink on the ice | seen in `b1b-before-*` and `b1b-after-*` | match |

Things not in the table:
- `Markings` is hidden with `BasicShapeMaterial`. That is correct: B's ice paints the lines.
- `Glass` uses `/Game/Placeholders/M_Net` at stencil 0, and `Obstacles` is at stencil 1 with no material (the cartridge has 0 obstacles).
- Each skater has a `number_0` TextRender (stencil 0, `DefaultTextMaterialOpaque`), so the jersey number is not ramped.

Problems on A's side (reported only, not fixed):
- **Two sky lights.** The persistent `SkyLight` (intensity 1, same cubemap) is visible in PIE next to `Sky_L2_Night`. This is C-0002 open question 2.
- **Floor overlap.** `ArenaFloor_UnderIce` (Plane 220 x 140 m, `BasicShapeMaterial`, casts shadow) has its top at z -5 cm, the same height as `Room_L2_Night_1`. That is a z-fighting risk outside the boards. I did not see it in my stills.
- **Goal line.** The game's goal line is `GoalLineX = L/2 - 4 m` (56 m), but B1's `M_Ice` painted it at `L/2 - 7 m` (53 m). **I fixed this on B's side** (task 3).
- **Rink size not proven.** The `RinkHalfLength` / `RinkHalfWidth` values equal the material defaults at 120 x 60. Whether A really drives them is proven only on a 90 x 45 cartridge, which I did not run.

## Task 2: plan test 1 on a scratch copy (`/JamAssets/_Scratch/V1T1`, deleted afterwards)

My fixed script is `C:\Users\kazda\AppData\Local\Temp\claude\6d8eaecb999746c2\1f5c7b1e-557b-4c70-b752-8370d9dbef0b\scratchpad\b1b\v1_t1_ice_floor_b1b.py`.

Fixes to the planning lane's script:
1. **The crash above.** The script now saves the duplicate, drops the Python reference, and runs `gc` and `collect_garbage` before `load_level`.
2. `execute_python` rejects an `import` inside a function, so the script uses `__import__`.
3. **One PIE for everything.** New actions `pie_case`, `pie_shot` and `pie_build_mat` act on the PIE world, so all cases and shots run in a single PIE session, not one per shot. Fill lights are hidden through component visibility in PIE.
4. **Emissive rebuilt (VER `b`).** `IceAmbient` was modulated by noise, which the ramp cut into blotches. It is now `BaseColour * AmbientColour * IceAmbient`.
5. **Extra cases.** I added stronger ambient cases.
6. **Measuring.** ffmpeg turns PNG into limited-range YUV by default (black reads 16). The plan's numbers assume full range, so measure with `out_range=pc`. Line crops must skip the 1 px frame border.

Camera positions are the script's `side` / `line` / `near` shots. The crops are fixed:
- band: 500 x 200 at (400, 500)
- pool: 250 x 60 at (100, 300)
- puck window: 40 x 30 at (1037, 301); surround: 200 x 80 at (957, 276)
- line: 1400 x 90 at (500, 800)

| Pass line | Baseline | Adopted variant (emissive follows base colour, `IceAmbient` 0.9, `LineFix` 1) | Result |
|---|---|---|---|
| Side-band ice YAVG 20-60 | 0.7 | 46.5 (on the real Rink: 1.2 -> 46.9) | **PASS** |
| Pool centre within 5 % of baseline | 211.8 | 212.4, +0.3 % (on the real Rink: 205.5 -> 206.9) | **PASS** |
| Ramp bands still visible | yes | yes: 2-3 steps at every pool edge (judged by eye, in the stills) | **PASS** |
| Dark-band puck at 24 m at least 40 levels from its surround | 0.8 (puck invisible) | surround 45.2, puck minimum 0, a difference of 45 (on the real Rink: 45.8) | **PASS** (the puck is about 4 px wide there) |
| Blue line YMAX-YMIN 30 or less | 74 (72..146) | 24 with `LineFix` alone; 21 with the adopted material | **PASS** |
| BasePass +0.05 ms or less | - | **not measurable**: BasePass ranged from 0.18 to 1.1 ms across identical frames with two editors up. Proxy: `M_Ice` pixel-shader instructions went from 380 to 392 (+3 %). | **unverified** |
| Reflection lever: SSR or none saves 0.5 ms or more with no visible change on dry ice | `LumenReflections` 2.2-5.5 ms (PIE view) | Scratch copy: with SSR or none, `LumenReflections` disappears from the PIE view, and SSR costs 0.13 ms. PSNR against the baseline: SSR 43.6 dB (near) / 50.1 dB (line); none 45.0 / 51.2. On the real Rink, 11:35, alternating SSR / Lumen / SSR / Lumen: `RenderDeferredLighting` 1.23 / **8.02** / 1.33 / **8.59** ms; `LumenReflections` 6.09 / 7.42 ms when on. | **PASS**: SSR adopted (unconfirmed, two editors up) |

Other cases:

| Case | Band YAVG | Verdict |
|---|---:|---|
| Sky 3.5 (`e_sky35`) | 0.7, unchanged | **FAIL.** The ceiling makes Lumen occlude the sky light completely, so sky intensity is not a floor lever, on either sky. |
| House fill, 2 spots of 600 cd (`d`) | 10.8 | FAIL (below 20). |
| The script's noise `IceAmbient` 0.40 / 0.60 | 33.7 / 50.0 | Puck difference 35.7 at 0.40 fails, and 0.60 is blotchy (still `b1b-t1-noisyambient-side`). Replaced by variant b. |
| Lighter `LineBlue` (`g`) | - | Line range 64: a lighter line shows more of the reflection gradient. Not adopted. |

The scratch-line cases (`h` and `h2`, direction D2) were not run.

`FPostProcessSettings.reflection_method` and `override_reflection_method` exist per volume in 5.9 and can be set from Python, so the reflection method is **not** a project setting.

## Task 3: what changed in the real assets

| Asset | Parameter or part | Old | New |
|---|---|---|---|
| `M_Ice` | goal-line position in the `IceColour+Lines` code | `RinkHalfLength - 700.0` | `RinkHalfLength - 400.0` (the game's goal line) |
| `M_Ice` | `Roughness` custom node | `lerp(lerp(0.28,0.75,M),0.05,Wet)` | Same, but painted lines (red, blue, circle, from `RinkHalfLength`/`RinkHalfWidth`) use `lerp(LineRoughness, 0.05, Wet)` weighted by `LineFix`. New inputs: P, RinkHalfLength, RinkHalfWidth, LineFix, LineRoughness. |
| `M_Ice` | **new** scalar `LineFix` | - | default 1.0 |
| `M_Ice` | **new** scalar `LineRoughness` | - | default 0.6 |
| `M_Ice` | **new** scalar `IceAmbient` | - | default 0.0 (the day tier is unaffected) |
| `M_Ice` | **new** vector `AmbientColour` | - | 0.25, 0.35, 0.65 |
| `M_Ice` | Emissive | none | new custom node `Ambient`: `BaseColour * AmbientColour * IceAmbient` |
| `MI_Ice_Night` | `IceAmbient` | not set (0) | 0.9 (`Scuff` 0.7 and `Wet` 0 unchanged) |
| `MI_Toon_NetMesh` | `TeamColour` | 0.85, 0.85, 0.85 | 0.28, 0.30, 0.34 |
| `MI_Toon_NetMesh` | `Emissive` | 1.5 | 0.8 |
| `LR_L2_Night` / `PPV_L2_Night` | `reflection_method` (with its override) | LUMEN, override off | SCREEN_SPACE, override on |

Unchanged:
- `TeamColour`, `Emissive`, `Wet`, `Scuff`, `RinkHalfLength` and `RinkHalfWidth` keep their names and defaults.
- Sky and spots: no change. The sky does not light the floor, and the spots make the pools.
- No `Fill_L2_Night_*` light was added, because the house fill failed.

Net check, same PIE frame, with the old values put back on a PIE-only dynamic instance: net mesh 168.8 against boards 164.2 before, **92.9 against 164.2 after**.

Rig map saved and read back: 301 actors. That is B1's 12 (`PPV_L2_Night`, `Sky_L2_Night`, `Spot_L2_Night_1..4`, `Room_L2_Night_1..6`) plus 289 `Dress_L2_Night_*`, 0 `Fill_`, and nothing else.

## Task 4: final stills (real `Rink`, PIE)

The new pairs use the same camera as the before stills:

| View | Before | After |
|---|---:|---:|
| centre, gap ice | 3.5 | 37.6 |
| centre, pool | 215.7 | 216.5 |
| far end, side ice | 1.7 | 42.0 |
| far end, pool | 217.6 | 217.9 |
| dark band | 1.2 | 46.9 |

The puck and both team colours read in the dark-band still. The game-camera pair differs in game moment and camera.

## Stills filed (`devlog/shots/`, JPEG, 1920 x 1085, each with a caption `.txt`)

`20261112-113700-B-b1b-before-game`, `-before-centre`, `-before-far`, `20261112-113701-B-b1b-before-band`, `-after-game`, `-after-centre`, `-after-far`, `-after-band`,
`-net-old`, `-net-new`, `20261112-113702-B-b1b-t1-baseline-side`, `-t1-noisyambient-side`, `-t1-ambient090-side`, `-t1-baseline-line`, `-t1-linefix-line` (15 stills, 30 files).

## Files changed (bytes on disk; the only plugin files modified in this session)

| File | Before | After |
|---|---:|---:|
| `Game/Plugins/JamAssets/Content/Look/M_Ice.uasset` | 19,754 | 23,795 |
| `Game/Plugins/JamAssets/Content/Look/MI_Ice_Night.uasset` | 9,490 | 9,852 |
| `Game/Plugins/JamAssets/Content/Look/MI_Toon_NetMesh.uasset` | 8,180 | 8,402 |
| `Game/Plugins/JamAssets/Content/Rigs/LR_L2_Night.umap` | 656,481 | 656,688 |

Also created: `status/B1b-report.md` (this file) and the 30 still files above.

## What I could not verify

- **BasePass cost of the ice change.** Profiler noise with two editors up is larger than the 0.05 ms line; I only have the instruction-count proxy.
- **Reflection saving on the 4090.** The pairs above are B-only numbers.
- **Wet ice under SSR.** How `Wet` 1 (the cartridge's `bWetIce`) looks under SSR instead of Lumen reflections was not checked.
- **Whether A drives the rink size.** That needs a non-120 x 60 cartridge.
- **Floor z-fighting.** The overlap between `ArenaFloor_UnderIce` and `Room_L2_Night_1` was not checked from a camera that sees the apron.
- **Scratches (D2).** Not tested.

## Decisions for the lead (and what needs device A)

1. **A, lane A5:** `SetBoost(NAME_None)` resets the skater `Emissive` to 1.0 (`ToySkaterVisual.cpp:413`). The handoff asks for 2.0-2.5, and the build value is 2.2. Restore 2.2 there.
2. **A:** remove or disable the persistent `Rink` `SkyLight` when a rig loads (C-0002 open question 2). Two sky lights are active. This is a correctness issue, not a look issue: the sky intensity does not reach the floor.
3. **A:** lower `ArenaFloor_UnderIce` (now z -5 cm, the same height as the rig floor), or hide it when a rig is loaded.
4. **A / B3:** the puck has no halo disc yet (handoff 0007). The puck is 24 cm, not 12 cm: confirm that this is intended.
5. **Lead:** the blue lines differ. `M_Ice` paints them at 0.29 x half-length (17.4 m), while A's hidden markings use length/6 (20 m). Pick one; the goal line is already aligned to the game.
6. **Lead:** SSR is now the L2 reflection method. Decide L3 and wet ice separately (plan test T3). Hand the 4090 confirmation to A at the next FrameBudget run.
7. **Ledger:** update the sizes of the four rows above. All are still under 0.1 MB except the rig (0.66 MB). No new files went into the plugin.
8. **Plan script:** the fixed T1 script and the crash cause should be copied back to `skater-lab/v1/tests/`. T2 (`v1_t2_atmosphere.py:286`) uses the same `dup = duplicate_asset(...)` then `load_level` pattern and will very likely crash the editor the same way. T3 (`v1_t3_neon.py:287`) holds only a temporary reference, so it may survive, but that is untested: apply the same fix to it.
