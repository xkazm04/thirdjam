# B1 report: painted-cel materials, post passes, LR_L2_Night (device B)

Lane B1, device B. Worked 10:23 to 10:35 on 2026-11-12, inside the 10:25 to 12:25 time box. No git commands were run. Everything was driven through
`node tools/mcp/mcp.mjs` (`execute_python` and toolset calls) against the running editor (`Game/ThirdJam.uproject`, UE 5.9, x64 under emulation on
ARM64, NVIDIA RTX Spark N1X). At the end the editor is open on `/Game/Maps/Rink`, with no dirty packages and nothing under `/Game/` saved.

## Verdict per deliverable

| # | Deliverable | Result | Evidence (the call that proves it) |
|---|---|---|---|
| 1 | Masters `M_Toon`, `M_Trim`, `M_Ice`, `M_PP_ToonRamp`, `M_PP_Outline` | **PASS** | `MEL.get_material_error_expressions` is `[]` for all five. `MEL.get_statistics` shows they compiled (PS instructions 214 / 195 / 380 / 91 / 91). Parameter names read back: `TeamColour`, `Emissive` (Toon, Trim), `Scuff`, `Wet`, `RinkHalfLength`, `RinkHalfWidth` (Ice). Both PP materials are `MD_POST_PROCESS`, Before DOF. All five masters were built in one call that took 7.7 s. |
| 2 | Instances, 17 of them (recipe section 4 + `MI_PP_ToonRamp`, `MI_PP_Outline`, `MI_Trim_PuckHalo`) | **PASS** | A read-back of `scalar_parameter_values` and `vector_parameter_values` on every instance matches the recipe table. `MI_Trim_PuckHalo` is `M_Trim`, TeamColour 0.75, 1.0, 0.15, Emissive 18. The two PP instances carry no overrides, so they use the masters' defaults. |
| 3 | `LR_L2_Night` filled and saved | **PASS** | Loaded `/JamAssets/Rigs/LR_L2_Night` as the editor world. `get_all_level_actors` labels: `PPV_L2_Night`, `Sky_L2_Night`, `Spot_L2_Night_1..4`, `Room_L2_Night_1..6`, and nothing else (no PlayerStart, no gameplay actors, no trim strips). The read-back of every value is listed under "Rig contents". `save_current_level` returned true, and the file is 43,084 bytes. |
| 4 | Verify on a throwaway layout, stills through PIE + `CaptureGameViewport` | **PASS, one process slip (fixed)** | Six stills filed (list below). Ramp bands on ice, ink lines on figures, stick, net and puck, no lines on the ice. `RampOn` and `OutlineOn` each switch their pass off. `_Scratch` deleted, so the plugin `Content/` holds only `Look/` and `Rigs/`. The slip is described under "What went wrong". |
| 5 | Cost on this machine | **PASS (measured; device B numbers, PIE only)** | `GetFrameTimings` x 60 in PIE with editor viewport realtime off gave gpuMs median **10.51 ms**. One `TriggerGPUProfile {"minInclusiveMs":0}` shows `LumenSceneLighting` and `VirtualShadowMapMarkPages` present, so **Lumen and VSM do run on this GPU**. Numbers below. |
| 6 | Ledger rows | **PASS** | 23 rows below, sizes taken from the files on disk. |

## Asset files created or changed (bytes on disk; each is a single `.uasset` / `.umap`, no `.uexp` / `.ubulk`)

| File | Bytes |
|---|---:|
| `Game/Plugins/JamAssets/Content/Look/M_Toon.uasset` | 14,344 |
| `Game/Plugins/JamAssets/Content/Look/M_Trim.uasset` | 11,433 |
| `Game/Plugins/JamAssets/Content/Look/M_Ice.uasset` | 19,754 |
| `Game/Plugins/JamAssets/Content/Look/M_PP_ToonRamp.uasset` | 12,059 |
| `Game/Plugins/JamAssets/Content/Look/M_PP_Outline.uasset` | 14,761 |
| `Game/Plugins/JamAssets/Content/Look/MI_Toon_Blue.uasset` | 9,063 |
| `Game/Plugins/JamAssets/Content/Look/MI_Toon_Orange.uasset` | 9,124 |
| `Game/Plugins/JamAssets/Content/Look/MI_Toon_Board.uasset` | 8,595 |
| `Game/Plugins/JamAssets/Content/Look/MI_Toon_Helmet.uasset` | 8,597 |
| `Game/Plugins/JamAssets/Content/Look/MI_Toon_Stick.uasset` | 8,719 |
| `Game/Plugins/JamAssets/Content/Look/MI_Toon_NetFrame.uasset` | 8,717 |
| `Game/Plugins/JamAssets/Content/Look/MI_Toon_NetMesh.uasset` | 8,180 |
| `Game/Plugins/JamAssets/Content/Look/MI_Toon_Puck.uasset` | 7,502 |
| `Game/Plugins/JamAssets/Content/Look/MI_Toon_Dark.uasset` | 6,995 |
| `Game/Plugins/JamAssets/Content/Look/MI_Trim_Blue.uasset` | 8,302 |
| `Game/Plugins/JamAssets/Content/Look/MI_Trim_Orange.uasset` | 8,483 |
| `Game/Plugins/JamAssets/Content/Look/MI_Trim_PuckRim.uasset` | 7,609 |
| `Game/Plugins/JamAssets/Content/Look/MI_Trim_Beacon.uasset` | 7,343 |
| `Game/Plugins/JamAssets/Content/Look/MI_Trim_PuckHalo.uasset` | 8,037 |
| `Game/Plugins/JamAssets/Content/Look/MI_Ice_Night.uasset` | 9,490 |
| `Game/Plugins/JamAssets/Content/Look/MI_PP_ToonRamp.uasset` | 4,842 |
| `Game/Plugins/JamAssets/Content/Look/MI_PP_Outline.uasset` | 4,832 |
| `Game/Plugins/JamAssets/Content/Rigs/LR_L2_Night.umap` (changed) | 43,084 |

Total: about 0.27 MB.

## Ledger rows (paste into `ASSETS.md`)

The existing `LR_L2_Night.umap` row (device A, placeholder) must be **replaced** by the last row below, not duplicated, because the checker
wants exactly one row per file.

```
| Game/Plugins/JamAssets/Content/Look/M_Toon.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | toon master: TeamColour, Emissive fill, rim (lane B1) |
| Game/Plugins/JamAssets/Content/Look/M_Trim.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | emissive trim master (lane B1) |
| Game/Plugins/JamAssets/Content/Look/M_Ice.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | procedural ice, Scuff, Wet, lines from RinkHalfLength/RinkHalfWidth (lane B1) |
| Game/Plugins/JamAssets/Content/Look/M_PP_ToonRamp.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | post-process toon ramp by CustomStencil (lane B1) |
| Game/Plugins/JamAssets/Content/Look/M_PP_Outline.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | post-process ink outline by CustomStencil (lane B1) |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Blue.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Orange.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Board.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Helmet.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Stick.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_NetFrame.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_NetMesh.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Puck.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Dark.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | dark room of the rigs |
| Game/Plugins/JamAssets/Content/Look/MI_Trim_Blue.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | |
| Game/Plugins/JamAssets/Content/Look/MI_Trim_Orange.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | |
| Game/Plugins/JamAssets/Content/Look/MI_Trim_PuckRim.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | |
| Game/Plugins/JamAssets/Content/Look/MI_Trim_Beacon.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | |
| Game/Plugins/JamAssets/Content/Look/MI_Trim_PuckHalo.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | puck halo for lane B3 |
| Game/Plugins/JamAssets/Content/Look/MI_Ice_Night.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | |
| Game/Plugins/JamAssets/Content/Look/MI_PP_ToonRamp.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | name fixed by C-0002 |
| Game/Plugins/JamAssets/Content/Look/MI_PP_Outline.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | name fixed by C-0002 |
| Game/Plugins/JamAssets/Content/Rigs/LR_L2_Night.umap | other | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | night rig filled by lane B1: PPV, sky, 4 spots, dark room (replaces A0's placeholder row) |
```

Every file is under 0.05 MB, so each row reads 0.1 as the brief says. The checker's "within a quarter" rule may therefore warn on these rows.

## Rig contents (read back from the saved map)

- `PPV_L2_Night`: `PostProcessVolume` at the origin, `unbound` true. Exposure `AEM_HISTOGRAM`, min = max 4.2, bias 0. Bloom 0.9, threshold 1.0. Vignette 0.3.
  Shadow gain (0.85, 0.93, 1.15, 1). Saturation (1.1, 1.1, 1.1, 1). Every `override_*` flag is true. Blendables are `MI_PP_ToonRamp` then `MI_PP_Outline`, weight 1 each.
- `Sky_L2_Night`: `SkyLight` at (0, 0, 20 m), Movable, `SLS_SPECIFIED_CUBEMAP` `/Engine/MapTemplates/Sky/DaylightAmbientCubemap`, intensity 2.0, colour (150, 185, 255).
- `Spot_L2_Night_1..4`: `SpotLight` at x = -45, -15, 15, 45 m, y 0, z 25 m, pitch -90, Movable. 20000 cd, attenuation 4000, cones 22 / 30 degrees,
  colour (255, 244, 230), shadows on, source radius 30.
- `Room_L2_Night_1..6`: `StaticMeshActor` built from engine basic shapes, all `MI_Toon_Dark`. Cast shadow off, custom depth off (stencil 0), `NoCollision`
  profile and `NO_COLLISION`. Sizing: largest cut 140 x 70 m plus 12 m on every side puts the inner faces at |x| = 82 m and |y| = 47 m.
  - 1: floor apron, `Plane` 164 x 94 m, top at z = -5 cm.
  - 2 and 3: walls at -x and +x, `Cube` 0.5 m thick, from z -5 cm to 28 m.
  - 4 and 5: walls at -y and +y, same.
  - 6: ceiling, `Cube` 0.5 m thick, underside at 28 m.
  - Bounds read back: walls inner faces at x ±8200, y ±4700; ceiling 2800 to 2850.

## M_Ice line derivation (brief item 1)

The constants are replaced by scalar parameters `RinkHalfLength` (default 6000) and `RinkHalfWidth` (default 3000), group `Rink`, in cm. Inside the
`IceColour+Lines` custom node:

- goal lines (red, 12 cm): `|x| = RinkHalfLength - 700`, which gives 5300 at the default.
- blue lines (60 cm): `|x| = RinkHalfLength * 0.29`, which gives 1740 at the default (the recipe had 1750).
- red centre line `|x| < 30` and centre circle radius 900 (16 cm line): unchanged.
- `RinkHalfWidth` masks every painted line to `|y| <= RinkHalfWidth`, so lines never run onto the apron beyond the ice. This use is my choice; the brief did not say what the width drives.

## Frame and memory numbers (device B, PIE, not the gate figure)

- `GetFrameTimings`, 60 samples, PIE floating window, editor viewport realtime off, near-figure camera:
  - gpuMs: median **10.51**, p10 7.57, p90 12.54, min 1.10, max 14.18.
  - frameMs: median 16.67 (60 fps cap).
  - game thread: median 12.71 ms.
  - RHI thread: median 0.17 ms.
- One `TriggerGPUProfile {"minInclusiveMs":0}`. The frame holds **two view families**: one at 1563x860 (the level viewport still renders even with realtime off) and the PIE view at 1398x790 internal, which TSR resolves to 1920x1085. The queues are Graphics 13.93 ms and Async Compute 12.76 ms, overlapping.

| Pass | PIE view (1398x790) | Other view (1563x860) |
|---|---:|---:|
| `Scene` (graphics) | 7.56 | 4.99 |
| `LumenSceneLighting` (async) | 1.51 | 1.30 |
| `DiffuseIndirectAndAO` (async; `LumenScreenProbeGather` 4.25) | 4.76 | 4.53 |
| `RenderDeferredLighting` | 3.62 | 2.62 |
| of which `LumenReflections` | 2.71 | 1.85 |
| `VirtualShadowMapMarkPages` | 0.094 | 0.102 |
| `ShadowDepths` | 0.344 | 0.387 |
| `PostProcessMaterial Material=M_PP_ToonRamp` | **0.037** | 0.042 |
| `PostProcessMaterial Material=M_PP_Outline` | **0.042** | 0.058 |

  Reading the table: the two post passes cost about 0.08 ms together on this GPU, about 2x the 4090. Lumen dominates the frame. The PIE gpuMs here is about 2x device A's PIE figure (4.85 / 5.26).
- Memory during PIE:
  - `UnrealEditor` working set 4,452 MB, private 9,023 MB.
  - Free RAM 44,368 MB of 65,529 MB.
  - `nvidia-smi`: 6,470 MiB used of 64,000 MiB (unified memory).

## Stills filed (`devlog/shots/`, JPEG q88, 1920x1085, each with a caption `.txt`)

- `20261112-103011-B-b1-l2night-near-figure.jpg`: camera 6 m behind and 3.5 m above the blue figure, RampOn 1, OutlineOn 1.
- `20261112-103012-B-b1-l2night-mid-ice.jpg`: orange figure, centre circle and blue line.
- `20261112-103012-B-b1-l2night-far-net.jpg`: net at about 100 m.
- `20261112-103012-B-b1-l2night-ramp-off.jpg`: RampOn 0, same frame as near-figure.
- `20261112-103012-B-b1-l2night-outline-off.jpg`: OutlineOn 0, same frame.
- `20261112-103012-B-b1-l2night-both-on.jpg`: both restored to 1, same frame.

The throwaway layout:
- ice: `Plane` 120 x 60 m, stencil 2, `MI_Ice_Night`.
- two figures: a cylinder body with a sphere head and a cylinder stick, stencil 1, `MI_Toon_Blue` and `MI_Toon_Orange`. The engine has no `/Engine/BasicShapes/Capsule`, so the body is a cylinder.
- puck: cylinder, stencil 3.
- board segment: 20 m, stencil 1.
- net box: stencil 1, at x 53.5 m.
- `CameraActor` with `auto_activate_for_player PLAYER0`, FOV 70.
- a `PlayerStart` in a corner out of view.
- world settings game mode `GameModeBase`, so A's game mode did not run.

## Names I chose that no contract gives

- `RinkHalfLength`, `RinkHalfWidth`: B proposed these in handoff 0005 and the brief fixed them. Parameter group `Rink`.
- What `RinkHalfWidth` drives: the line mask, described above.
- Room numbering: `Room_L2_Night_1` floor, `_2` wall -x, `_3` wall +x, `_4` wall -y, `_5` wall +y, `_6` ceiling.
- All rig lights are **Movable**. The recipe does not say; a streamed sub-level has no built lighting, so Movable is the only option that works.
- `Sky_L2_Night` placed at z 20 m (position does not matter for a sky light).
- Throwaway names (deleted): `/JamAssets/_Scratch/L_B1Verify`, actors `V_*`.

## What in the recipe was wrong or different on this engine or GPU

1. **Tool and Python names: no differences.** Every toolset call and Python API in the recipe worked as written: `execute_python`, `StartPIE`, `StopPIE`, `ListWorlds`, `CaptureGameViewport`, `GetFrameTimings`, `TriggerGPUProfile`, `MaterialEditingLibrary`, `LevelEditorSubsystem`.
2. **`EditorLevelUtils.add_level_to_world` makes the added sub-level the current level.** My verify actors were spawned into `LR_L2_Night`, and the later `save_current_level` saved them into the rig (64,995 bytes).
   - Fix: I reloaded the rig as the main world, destroyed the 12 `V_*` actors, saved (43,084 bytes) and re-read the labels: only the 12 rig actors remain.
   - The stills are unaffected; they were rendered with the verify shapes in the rig sub-level instead of the persistent scratch level.
   - Recipe pitfall to add: after `add_level_to_world`, call `LevelEditorSubsystem.set_current_level_by_name` back to the persistent level before spawning.
3. **Deleting a freshly created map through Python failed silently.**
   - `EditorAssetLibrary.delete_asset('/JamAssets/_Scratch/L_B1Verify')` returned True but did nothing. The log says the asset "could not be found in the Asset Registry". `delete_directory` returned False.
   - I removed the file and the empty folder from disk. After that the editor reports the asset and folder gone, and no package is dirty.
4. **The PIE window set to 1920x1080 captures at 1920x1085.** PIE also renders internally at about 73 % (1398x790) and TSR upscales, so these PIE numbers are not native 1080p.
5. **With realtime off, the level viewport still rendered during PIE** (a second view family at 1563x860), so gpuMs contains both views, as the recipe warns.
6. **A recipe issue to note: the ramp on the dark-blue line.** Close to the camera the 60 cm blue line shows a blotchy pattern under the ramp (mid-ice still, bottom edge). The scuff noise on a low-albedo line passes through the `sc / max(dc, 0.03)` divide.
   - Not fixed. A taste call: draw the lines lighter, or exclude them from the ramp.
7. **SM6, Lumen and VSM confirmed on this GPU.** The log says `Using Highest Feature Level of D3D12: SM6`, `r.CustomDepth` is 3, and the profile shows the Lumen and VSM passes.

## What is left or not verified

- **Not measured: a standalone CSV run** (`-game -csvCaptureFrames`). The brief asks only for PIE numbers.
- **Not checked: the rig inside `Rink` with the real rink.** I did not run A's `ULookTierSwitcher` with A's spawned meshes, so the acceptance stills of handoff 0005 are still to take.
- **Not checked: the double-sky question.** Whether the persistent `Rink` `SkyLight` adds to `Sky_L2_Night` (C-0002 open question 2) was not tested.
- **Not done: the puck halo, beacon and trim strips placed in a level.** Only the instances exist; placement is B3's job and A's (handoff 0005).
- **Not added to the contract:** C-0002 does not yet list the `Room_L2_Night_<n>` label pattern; it is the brief's pattern.

## For the lead to decide

1. Paste the ledger rows above and **replace** A0's `LR_L2_Night.umap` row with the new one.
2. A must drive `RinkHalfLength` / `RinkHalfWidth` on the ice's dynamic instance (handoff 0005). Until then the lines sit at the 120 x 60 m defaults.
3. Taste: the blotchy blue line under the ramp (item 6 above).
