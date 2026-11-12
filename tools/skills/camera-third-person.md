---
name: camera-third-person
description: Build the hockey third-person camera (spring-arm pawn, frame-based lag, derived pitch), the far-net screen marker and beacon, and the UMG radar for a 120 x 60 m rink, and prove them with offscreen and PIE screenshots through MCP and Python. Use at M0 for the camera and HUD, and when the camera or rink size changes.
track: gameplay presentation (device A); reads the sim, never writes it
evidence: docs/experiments/CAMERA-120M.md (numbers, screenshots, every error); docs/HOCKEY.md sections 2, 4 (`game.camera`), 7 (`FarNetReadable`)
---

# Recipe: third-person camera, far-net marker, radar

Everything below was run on UE 5.9.0 through `tools/mcp/mcp.mjs call ...` and `execute_python` (E4, 2026-11-12), with no human step. Numbers: `docs/experiments/CAMERA-120M.md`.

## Before you start

- `docs/HOCKEY.md` section 4 for `game.camera`; `tools/skills/skating-physics.md` for the sim (fixed 60 Hz, accumulator, `Alpha` interpolation).
- The project must have `bUseFixedFrameRate=True`, `FixedFrameRate=60` (`tools/mcp/PHYSICS.md`), and the `ModelContextProtocol`, `ToolsetRegistry`, `AllToolsets` plugins. A content-only `.uproject` (no Modules) is enough for the experiments.
- Git Bash: export `MSYS_NO_PATHCONV=1` before passing `/Game/...` paths. PowerShell 5.1: pass JSON through `@file.json`.

## 1. Defaults and geometry

| `game.camera` | default | wide |
|---|---|---|
| `distance_m` | 6 | 8 |
| `height_m` | 3.5 | 4 |
| `fov_deg` (horizontal, this is UE's `FieldOfView`) | 70 | 85 |
| `lag_frames` (sim frames) | 4 | 6 |
| `look_ahead_m` (not in the schema yet; hard-code or ask for the key) | 5 | 6 |

Derived: `arm_length = sqrt(d^2 + h^2)`, `arm_pitch = -atan(h / d)`, `camera_pitch = -atan(h / (d + look_ahead))`, camera relative pitch = `camera_pitch - arm_pitch` (positive, tilts it back up).
For 6 / 3.5 / 5: length 694.6 cm, arm pitch -30.26, camera relative pitch +12.61, final pitch -17.65. The engine put the camera at exactly (-6, 0, 3.5) m.

## 2. The pawn (all MCP, about 6 calls)

`BlueprintToolsets.BlueprintToolset.Create {"folderPath":"/Game","assetName":"BP_CamPawn","assetType":{"refPath":"/Script/Engine.Pawn"}}`, then with `B=/Game/BP_CamPawn.BP_CamPawn_C`:

| Step | Call |
|---|---|
| capsule | `EditorToolset.ActorToolset.AddComponent {"owner":{"refPath":"/Game/BP_CamPawn.BP_CamPawn"},"componentType":{"refPath":"/Script/Engine.CapsuleComponent"},"name":"Body"}` |
| make it root | `ActorToolset.SetParentComponent {"component":{"refPath":"B:DefaultSceneRoot_GEN_VARIABLE"},"parent":{"refPath":"B:Body_GEN_VARIABLE"}}` |
| arm | `AddComponent {"owner":{"refPath":"B:Body_GEN_VARIABLE"},"componentType":{"refPath":"/Script/Engine.SpringArmComponent"},"name":"Arm"}` |
| camera | `AddComponent {"owner":{"refPath":"B:Arm_GEN_VARIABLE"},"componentType":{"refPath":"/Script/Engine.CameraComponent"},"name":"Cam"}` |
| values | `EditorToolset.ObjectToolset.SetProperties {"instance":{"refPath":"B:Arm_GEN_VARIABLE"},"values":"{\"TargetArmLength\":694.62,\"RelativeLocation\":{\"X\":0,\"Y\":0,\"Z\":-90},\"RelativeRotation\":{\"Pitch\":-30.2564,\"Yaw\":0,\"Roll\":0},\"bDoCollisionTest\":false,\"bUsePawnControlRotation\":false,\"bInheritPitch\":false,\"bInheritRoll\":false,\"bInheritYaw\":true}"}` (Body: `CapsuleHalfHeight` 90, `CapsuleRadius` 30; Cam: `FieldOfView` 70, `RelativeRotation` pitch +12.6063) |
| compile | `BlueprintToolset.CompileBlueprint {"blueprint":{"refPath":"/Game/BP_CamPawn.BP_CamPawn"}}` |

`values` is a JSON string inside the JSON. The arm pivots at the feet (capsule centre is 90 cm up). Collision test off: boards are 1.1 m, the camera is above them and the arena has no ceiling. Spawning an instance and reading `camera.get_world_location()` is a one-line check of the formula.
Possession for PIE: `actor.set_editor_property('auto_possess_player', unreal.AutoReceiveInput.PLAYER0)` on the placed instance.

## 3. Lag that replays: `lag_frames`

Do **not** use the arm's `bEnableCameraLag`: it uses `DeltaSeconds` (0.67 m behind at 60 Hz, 0.43 m at 30 Hz for the same per-frame motion). Instead, make the arm absolute and filter in the sim step.

```python
# once:  arm.set_absolute(True, True, False)   # C++: Arm->SetAbsolute(true, true, false)
# per SIM frame (60 Hz step, after the skater moved), alpha = 1 - exp(-1.0 / lag_frames) (alpha = 1 when lag_frames == 0)
pivot_x += (skater_x - pivot_x) * alpha
pivot_y += (skater_y - pivot_y) * alpha
yaw     += wrap180(skater_yaw - yaw) * alpha
arm.set_world_location_and_rotation(Vector(pivot_x*100, pivot_y*100, 0), Rotator(roll=0, pitch=ARM_PITCH, yaw=yaw), False, False)
```

Measured (8 m/s, 180 degree turn at 5 rad/s): lag_frames 3 / 6 / 12 give 0.34 / 0.74 / 1.53 m extra distance and 12 / 26 / 53 deg max yaw error; identical rows at 60 and 30 Hz. `unreal.Rotator` is **(roll, pitch, yaw)**: use keywords.
Render-side smoothness: interpolate `(pivot, yaw)` between the previous and current sim state with the accumulator `Alpha`, like the pawns. The sim never reads the camera; the stick maps to the skater's heading, not to the camera.
Put the puck about 0.7 m to the side of the skater's axis: on the axis the pawn hides it up to `1.8 d / (h - 1.8)` m ahead (6.4 m at the defaults).

## 4. Far-net marker (screen space, ship this)

Widget: a 22 px magenta diamond (a `Border`, render angle 45) and a `TextBlock` (size 22, white) in the HUD canvas, both top-left anchored. Each tick:

```python
sp = unreal.WidgetLayoutLibrary.project_world_location_to_widget_position(pc, NET_WORLD_POS_CM, False)   # Vector2D, or None if it cannot project
vs = unreal.WidgetLayoutLibrary.get_viewport_size(world); sc = unreal.WidgetLayoutLibrary.get_viewport_scale(world)
vw, vh = vs.x / sc, vs.y / sc
if sp is None: sp = unreal.Vector2D(vw/2, 50)                 # behind the camera: clamp, better: an edge arrow
mx = min(max(sp.x, 30), vw - 30); my = min(max(sp.y, 50), vh - 50)    # clamp to the screen with a margin
diamond.slot.set_position(Vector2D(mx - 11, my - 11)); text.slot.set_position(Vector2D(mx, my + 16))   # tested at +16 (below); it overlaps the net box at 110 m, so try my - 40 (above), untested
text.set_text('%d m' % round(distance_m))
```

Net world point: the mouth centre, 3 m up (5800, 0, 300 cm for a net at x = 58 m). Optional beacon (flag `look.beacon`): a 0.8 m x 9 m emissive cylinder on the net (0.4 m was a hairline at 110 m). A brighter frame alone does not read beyond 60 m.

## 5. Radar (UMG), steps

1. `UMGEditor.WidgetBlueprintEditorSubsystem.CreateWidgetBlueprint {"folderPath":"/Game","assetName":"WBP_Hud","parentClass":{"refPath":"/Script/UMG.UserWidget"}}`.
2. `CreateAndAddWidget {"widgetBlueprint":{"refPath":"/Game/WBP_Hud.WBP_Hud"},"widgetClass":{"refPath":"/Script/UMG.CanvasPanel"},"widgetDisplayName":"Root"}`, then children with `"parentWidget":{"refPath":"/Game/WBP_Hud.WBP_Hud:WidgetTree.Root"}`: `Border` x (`RadarBg`, `RadarMid`, `Dot0`..`Dot5`, `PuckDot`, `NetDiamond`) and `TextBlock` `NetText`.
3. Layout by Python (each widget is `unreal.load_object(None, '/Game/WBP_Hud.WBP_Hud:WidgetTree.<Name>')`): `slot.set_anchors(Anchors(min=(1,1), max=(1,1)))`, `slot.set_alignment(Vector2D(0,0))`, `slot.set_position`, `slot.set_size`, `slot.set_auto_size(False)`, `widget.set_brush_color(LinearColor)`. Radar 200 x 100 px at position (-224, -124) (bottom-right, 24 px margin); dots 12 px (player, yellow) and 9 px (red 0.95/0.12/0.12, blue 0.25/0.5/1), puck 6 px white; each dot's slot position is the radar's top-left minus half its size, so that its render translation (u, v) centres it. Background (0.02, 0.05, 0.10, 0.65), centre line white 45 %.
4. Make the dots and the marker parts variables so a driver can reach them: `ToggleWidgetAsVariable {"widgetBlueprint":{...},"widget":{"refPath":"...WidgetTree.Dot0"},"bIsVariable":true}`; compile (`CompileBlueprint`) and save (`AssetToolset.SaveAsset`).
5. Create it in the pawn's `BeginPlay` with the Graph DSL (`BlueprintDslToolset.Write {"assetPath":"/Game/BP_CamPawn","dsl":...}`; mind `MSYS_NO_PATHCONV`):

```lisp
(event `self.ReceiveBeginPlay`
  (bind hud (|CreateWidget :`Class` "/Game/WBP_Hud.WBP_Hud_C" :`OwningPlayer` (`GameplayStatics.GetPlayerController` 0)))
  (`UserWidget.AddToViewport` hud 0))
```

6. Per tick, radar x = (world_x_m + 60) / 120 * 200, y = (world_y_m + 30) / 60 * 100 (rink 120 x 60 m, +X to the right; scale with the cartridge `length_m` and `width_m`), then `dot.set_render_translation(Vector2D(x, y))`. Proven in PIE with a Python tick; **the shipping driver is a C++ widget** (`NativeTick` reading the sim; the Graph DSL cannot author widget graphs, and a blueprint cannot hold an object variable for the HUD).
7. Reaching the HUD from Python in PIE: `gi = unreal.GameplayStatics.get_game_instance(world); hud = unreal.find_object(None, gi.get_path_name() + '.WBP_Hud_C_0'); dot = hud.get_editor_property('Dot0')`. `unreal.new_object(widget_class)` gives a widget without a tree: do not use it. To call a library function Python does not expose: `unreal.get_default_object(unreal.load_class(None, '/Script/UMG.WidgetBlueprintLibrary')).call_method('Create', args=(world, widget_class, pc))`.

## 6. Proving it (screenshots)

| What | Call |
|---|---|
| any camera, offscreen, no PIE | `SceneCaptureToolset.SceneCaptureToolset.Capture` with `{"world":{"refPath":"/Game/CamTest.CamTest"},"mode":"Lit","camera":{"location":{x,y,z in cm},"rotation":{pitch,yaw,roll},"projectionMode":"Perspective","fOV":70},"options":{"width":1920,"height":1080,"screenPercentage":1,"visualizationSubMode":""}}`, CLI `call ... @file.json --save-images <dir>`; 0.4 s each; no UMG |
| PIE with UMG at 1080p | Python once: `s = unreal.get_default_object(unreal.LevelEditorPlaySettings); s.set_editor_property('new_window_width', 1920); s.set_editor_property('new_window_height', 1080)`; then `EditorAppToolset.StartPIE {"options":{"playMode":"PlayMode_InEditorFloating"}}`; `SceneCaptureToolset.CaptureGameViewport {"world":{"refPath":"/Game/UEDPIE_0_<Map>.<Map>"},"maxResolution":1920}` (1920 x 1082); world names from `ListWorlds` |
| numbers | a Python tick (`unreal.register_slate_post_tick_callback`) that appends rows to a file; Python returns only its last expression, not `print` |
| repo size | quantise PNG with 256 colours (`FASTOCTREE`), 1920 x 1080 is 65-80 KB; 32 colours destroys the hues |

Scene helper used for the experiments: `StaticMeshActor` with `/Engine/BasicShapes/{Plane,Cube,Cylinder,Sphere}`, scale = size in metres (the shapes are 100 cm), unlit materials from one `M_Flat` (a `VectorParameter C` into Emissive Color, shading model Unlit) and `MaterialInstanceConstant` per colour (`MaterialEditingLibrary.set_material_instance_vector_parameter_value`). `EditorActorSubsystem.spawn_actor_from_class` places actors; destroy with `destroy_actor`.

## 7. Pitfalls (each one cost time)

1. **Never stop PIE with live Python references.** The tick callback and any `builtins`/global holding PIE objects crashed the editor at `StopPIE` ("preventing the old GameInstance from being GC'd"). Unregister callbacks, clear globals, `gc.collect()` first; if the editor dies, restart with `tools/run/editor.ps1` (12 s warm) and rebuild the scene by script, keep the scene as a script.
2. Fetch components inside the tick (`pawn.get_component_by_class(...)`): objects fetched earlier were stale (zero transforms, ignored property edits).
3. Reset position and rotation at the start of each PIE harness; a leftover 180 degree yaw looks like a broken camera.
4. `project_world_location_to_widget_position` returns `None` for an unprojectable point; handle it.
5. The arm's own `camera_lag_speed` could not be changed from Python in PIE (read back 10); irrelevant if you use the frame-based lag.
6. After a PIE session `does_asset_exist` / `list_assets` / `SaveAsset` stopped seeing assets that `load_asset` still found; save every asset before PIE (`unreal.EditorAssetLibrary.save_loaded_asset(obj)` is the untested fallback).
7. Offscreen `Capture` is lighter than the PIE viewport; compare within one image.
8. `FOV` is horizontal. A 70 degree setting is 43 degrees vertical at 16:9.

## Test (acceptance)

- `FarNetReadable` (`docs/HOCKEY.md` section 7): with the pawn at 60, 85 and 110 m from the far net, the marker's projected position is inside the viewport and its text equals the distance; the radar shows 7 dots whose render translations equal the formula to 0.01 px.
- Camera determinism: the same recorded inputs at engine rates 60 and 30 Hz give identical `(pivot, yaw)` rows.
- Framing: puck 15 m ahead, the two nearest opponents and the far net are inside the frame at 1920 x 1080 for the default and the wide camera.
