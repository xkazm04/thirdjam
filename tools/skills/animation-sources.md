---
name: animation-sources
description: The two skater-animation routes that survived lane E2 - (1) procedural toy motion with the tuned numbers and a frame-exact look-review harness (Python + SceneCapture through MCP), (2) sourced clips (Fab Hockey Player pack + engine Mannequin falls/hits) retargeted to a UE5 skeleton and sampled under Simulate. Use when building or reviewing skater motion; read docs/experiments/ANIMATION-SOURCES.md for why video-to-motion and Kimodo were dropped.
track: route 1 = T-A (device A, ships M1); route 2 = T-B1 / T-B2 (device B)
evidence: docs/experiments/ANIMATION-SOURCES.md (run 2026-11-12, UE 5.9.0, MCP)
---

# Recipe: where skater motion comes from (the top two routes)

**Route 1** is code: the figure in `skater-toy-figure.md` with the numbers below, plus a review harness that renders any pose sequence to
PNGs in seconds. **Route 2** is clips: the Fab Hockey Player skate set and the engine Mannequin falls and hits, retargeted to a UE5 skeleton and
checked frame by frame. Neither route uses video-to-motion or Kimodo (reasons in the evidence file).

All calls go through `node tools/mcp/mcp.mjs call --url http://127.0.0.1:<port>/mcp <Module.Toolset.Tool> @args.json`. "Python" means
`editor_toolset.toolsets.python_scripting.PythonScriptingTools.execute_python` with `{"script":{"content":"<python>","language":"Python"}}`.
The script's last expression is the return value, and `await asyncio.sleep(x)` at top level lets the editor tick.

## Route 1: procedural toy motion (T-A)

### Numbers that changed after looking from 52 m (apply them to `skater-toy-figure.md` step 5)

| Parameter | Recipe | Use | Why (seen in `docs/experiments/img/anim-procedural-arena52m.png`) |
|---|---|---|---|
| `k_lean` (deg per m/s x rad/s) | 0.6 | **2.5** | at 0.6, 8 m/s x 2 rad/s gives 10 deg: invisible at 52 m |
| lean cap | 25 | **35** | 35 reads clearly as carving; above it the toy looks like it is falling |
| check body pitch | 20 | **30** | at 20 it looks like the normal crouch |
| stick | 4 cm, wood | **6 cm, bright blade** | a 4 cm stick is a hairline at 52 m |
| helmet | team colour | **team colour with a dark band** | a pale head blends into white ice |
| skate stride | static | optional: alternating push-off 18 cm out, 12 cm back x `min(speed/8,1)` | visible only from the chase or replay camera |
| bob | 4 cm | keep | free; it does not read at 52 m but it helps the replay |

**Add blade spray.** Use a Niagara burst from the `DirectionalBurst` template (R2: `NiagaraToolset_System.CreateNiagaraSystem`,
`ANIMATION.md` section 7), white, 20-40 sprites, about 0.4 s. Spawn it at the skates when `abs(turn_rate) * speed > 8` (hard carve), when
speed drops by more than 3 m/s within 10 frames (stop), and on the knockdown frame. Gate it on sim frames: one burst at most per 12 frames per
skater. It is cosmetic only and never read back. Of everything tested, this is the change that reads most at 52 m after lean and tip-over.

### Look-review harness (frame-exact, no PIE, about 6 s for 450 frames)

Use it to judge any pose function before or after it is C++. It renders a scripted input sequence from a fixed camera. The parts here are
separate StaticMeshActors whose world transforms the script composes; the jam's component does the same with child components.

1. **Level and material**, one Python call (6.6 s). Check `unreal.EditorAssetLibrary.does_asset_exist` first: a second run fails with
   `An asset already exists at this location`, and unattended runs cannot answer the overwrite prompt.
   ```python
   les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
   les.new_level_from_template('/Game/Review/Rink', '/Engine/Maps/Templates/Template_Default')
   at, mel = unreal.AssetToolsHelpers.get_asset_tools(), unreal.MaterialEditingLibrary
   m = at.create_asset('M_Toy', '/Game/Review', unreal.Material, unreal.MaterialFactoryNew())
   p = mel.create_material_expression(m, unreal.MaterialExpressionVectorParameter, -400, 0); p.set_editor_property('parameter_name', 'TeamColour')
   mel.connect_material_property(p, '', unreal.MaterialProperty.MP_BASE_COLOR)
   r = mel.create_material_expression(m, unreal.MaterialExpressionConstant, -400, 200); r.set_editor_property('r', 0.35)
   mel.connect_material_property(r, '', unreal.MaterialProperty.MP_ROUGHNESS); mel.recompile_material(m)
   mi = at.create_asset('MI_Red', '/Game/Review', unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
   mel.set_material_instance_parent(mi, m); mel.set_material_instance_vector_parameter_value(mi, 'TeamColour', unreal.LinearColor(0.75,0.05,0.04,1))
   ```
2. **Parts and hierarchy.** `eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)`; for each part,
   `eas.spawn_actor_from_object(unreal.load_asset('/Engine/BasicShapes/Cylinder'), loc, rot)`, then `static_mesh_component.set_material(0, mi)`
   and `set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)`. Compose transforms child to parent with
   `unreal.MathLibrary.compose_transforms(child_local, parent_world)` and set them with `actor.set_actor_transform(t, False, False)`.
   Keep pivot nodes unscaled; put the scale only on the final mesh transform. Units are cm; the pose formulas are in metres, so multiply by 100.
   - Pitch sign: a negative pitch tips the top of a vertical cylinder forward (+X).
   - The lean and tip pivots sit at z 0 (the skate contact point).
3. **Camera and capture.**
   ```python
   cap = eas.spawn_actor_from_class(unreal.SceneCapture2D, unreal.Vector(0,0,500), unreal.Rotator(0,0,0)); cc = cap.capture_component2d
   rt = unreal.RenderingLibrary.create_render_target2d(world, 1920, 1080, unreal.TextureRenderTargetFormat.RTF_RGBA8_SRGB)
   cc.set_editor_property('texture_target', rt); cc.set_editor_property('capture_source', unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR)
   cc.set_editor_property('capture_every_frame', False); cc.set_editor_property('fov_angle', 60.0)
   pp = cc.get_editor_property('post_process_settings')          # fixed exposure, else frames flicker
   pp.set_editor_property('override_auto_exposure_method', True); pp.set_editor_property('auto_exposure_method', unreal.AutoExposureMethod.AEM_MANUAL)
   pp.set_editor_property('override_auto_exposure_bias', True); pp.set_editor_property('auto_exposure_bias', 10.0)
   cc.set_editor_property('post_process_settings', pp)
   ```
   Arena camera: 33 m behind and 40 m above the subject, pitch -50, FOV 60 (52 m away). Chase camera: 4.5 m back, 7.5 m to the side, 3.5 m up,
   pitch -18.
4. **Step frames.** In a loop `for f in range(N)`: advance the sim state by `1/60`, pose every figure, and every k frames move the camera, then
   `await asyncio.sleep(0.03)`, `cc.capture_scene()`, `unreal.RenderingLibrary.export_render_target(world, rt, out_dir, f'f_{f:04d}.png')`.
   The export is an 8-bit PNG. Measured: 450 frames with 113 captures at 640x360 in 5.6 s; 75 captures at 1080p in 19.1 s.
5. **Judge.** Make a contact sheet of 6-8 labelled frames (PIL) and look at lean, check, shot and knockdown at the arena camera. Keep a sheet
   under 300 KB (`quantize(128)`) if it goes into the repo.

The test that matters remains `Jam.Skater.ToyPose` and the null-visual state-hash test from `skater-toy-figure.md`. The harness is for the
human or Director look check only.

## Route 2: sourced clips on a UE5 skeleton (T-B1, and the clip set for T-B2)

1. **Mannequin content, no download.** Copy `C:\Program Files\Epic Games\UE_5.9\Templates\TemplateResources\High\Characters\Content\*` to
   `<project>\Content\Characters\` (126 MB; the mount path must be exactly that). If the editor is running, follow with
   `unreal.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous(['/Game/Characters'], True)`.
   Useful clips (length, root motion):

   | Clip | Length | Root motion | Use |
   |---|---|---|---|
   | `Anims/Death/MM_Death_Back_01` | 1.10 s | no | `Fall` |
   | `MM_Death_Front_01..03`, `_Left_01`, `_Right_01` | | | `Fall`, other directions |
   | `Anims/Rifle/HitReact/MM_HitReact_Front_Hvy_01` | 0.80 s | | `Check` received (small) |
   | `Anims/Unarmed/Attack/MM_Attack_02` | 1.0 s | on: turn it off | `Check` given |

   Death clips end before the body is fully down: hold the last frame in `AM_Fall` or add a lying pose.
2. **Fab Hockey Player Animated** (only if the operator owns it): `FabToolset.InstallFabListing` (the first sign-in opens a browser: a human
   step once) into the plugin folder you own. Its 14 clips use the Epic UE4 skeleton.
3. **Retarget UE4 to UE5 (one Python call, proven by R2 in 4.1 s, `tools/mcp/ANIMATION.md` section 4).** Create an IK rig per mesh
   (`IKRigController.set_skeletal_mesh`, `apply_auto_generated_retarget_definition`, `apply_auto_fbik`), then the retargeter
   (`set_ik_rig` source and target, `add_default_ops`, `assign_ik_rig_to_all_ops`, `auto_map_chains(FUZZY, True)`, `auto_align_all_bones` both
   sides). Batch it with `unreal.IKRetargetBatchOperation.run_batch_retarget_with_results(inputs)`.
   - R2 ran the deprecated `duplicate_and_retarget`. The batch-inputs form is from the 5.9 headers and has not yet been run; if it fails,
     `duplicate_and_retarget(asset_datas, src_mesh, tgt_mesh, rtg, prefix=..., target_path=...)` still works in 5.9.
   - For a toy figure with a UE5 skeleton (Meshy `ue5_mannequin`), use the same script with that mesh as the target. If the hips float or sink,
     try the 5.9 Pelvis Volume op.
4. **Sample clips frame by frame to judge them. This works only under Simulate; the editor world does not tick skeletal animation.**
   ```
   EditorToolset.EditorAppToolset.StartPIE {"options":{"bSimulate":true}}
   ```
   ```python
   gw = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
   for a in unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.SkeletalMeshActor):
       sk = a.skeletal_mesh_component
       sk.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE); sk.set_animation(anim); sk.set_play_rate(0.0)
   for k, frac in enumerate([0, .25, .5, .75, 1]):
       sk.set_position(anim.get_play_length()*frac*0.999, False)
       await asyncio.sleep(0.15)
       print(sk.get_socket_location('head').z)       # must change between samples
       cc.capture_scene(); unreal.RenderingLibrary.export_render_target(gw, cc.texture_target, out_dir, f'clip_{k}.png')
   ```
   ```
   EditorToolset.EditorAppToolset.StopPIE {}
   ```
   - Use the SceneCapture actor copied into the game world (find it with `get_all_actors_of_class(gw, unreal.SceneCapture2D)`).
   - Set `visibility_based_anim_tick_option = ALWAYS_TICK_POSE_AND_REFRESH_BONES` on the meshes before Simulate.
   - The trap: in the editor world, `override_animation_data` and `set_position` return without error but the bones never move (the head
     stayed at z 160.2 for every sample).
5. **Montages, blend space, AnimBP, visual Blueprint, promotion gate:** as in `skater-retarget.md` steps 7-11. The AnimBP needs the
   `UTJAnimGraphEdLibrary` helper (`ANIMATION.md` section 2) if it has a state machine. A Blend Space plus a Slot needs none.

## Do not

- Do not try video-to-motion for the stride. The foot clean-up in every service plants sliding feet, and nobody publishes a result on ice.
  Off-ice slapshot or check takes through Rokoko Basic are an optional T-B extra only.
- Do not enable the Kimodo plugin in the jam project. On first load it pip-installs torch and other packages, then downloads about 21 GB of
  models including the gated Llama 3 8B. There is no MCP or Python entry point; it runs from Sequencer only.
- Do not count on Meshy's animation library for skating: IDs 0-587 contain none. Its falls (366, 502-505) are fine.
