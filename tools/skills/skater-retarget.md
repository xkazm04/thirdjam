---
name: skater-retarget
description: Build a T-B1 or T-B2 skater visual on a skeletal mesh - import, IK Rig, retarget by Python, a minimal Animation Blueprint, montages, sockets, and the swap-in behind IJamSkaterVisual. Use on device B for the sourced-clip humanoid (T-B1) and for a generated rigged figure (T-B2).
track: T-B1 (device B, H12-H36) and T-B2 (device B, H24-H48)
evidence: docs/SKATERS-RESEARCH.md sections 2, 3, 6
---

# Recipe: a skeletal skater behind IJamSkaterVisual (T-B1 / T-B2)

The game ships on T-A. Your figure is promoted only if, in the test cartridge at H36 (T-B1) or H48 (T-B2), **the player bot still wins and the
frame gate holds**, and the Director prefers it on a screenshot next to T-A. Build to that gate, not to perfection.

## Before you start

- Read `contracts/skater-visual.md`, `contracts/assets-naming.md`, `ASSETS.md` (every file gets a ledger row in the same commit: `sourced`
  for Fab, Mannequin, Mixamo; `ai-generated` for Meshy/Tripo/Rodin, with the tool named).
- Your paths: `Game/Plugins/JamAssets/**` only. The C++ interface implementation is A's; you deliver a Blueprint `BP_SkaterVisual_TB1` /
  `BP_SkaterVisual_TB2` that implements `IJamSkaterVisual` in Blueprint, or ask A for a C++ base in a handoff.
- Import selectively. Never import a whole pack "to try". 2K texture cap. Read the staged size after packaging the plugin.
- Tools: MCP via `tools/mcp/mcp.mjs`; Python through `editor_toolset.toolsets.python_scripting.PythonScriptingTools.execute_python`.
  Check every Python name first with `search_symbols` / `get_symbol` (the 5.9 API, not your memory).

## Pick the source (in this order)

1. **T-B1, Hockey Player Animated** (Fab, owned by the operator): mesh and 14 clips on the Epic skeleton (skate fwd/bwd/left/right start-loop-stop,
   idle, shooting). Install with `FabToolset.InstallFabListing`. Little or no retarget: mesh and clips share a skeleton.
2. **T-B2, Meshy** (partner): generate the figure (stylized chunky toy hockey player, **A-pose, limbs clearly separated from the body, no
   logos, no text**), then rig with `skeleton_template = "ue5_mannequin"`; download FBX. Optional: mesh from Rodin, rig in Meshy (counts Rodin).
3. **T-B2, Tripo**: rig with `spec = "unreal"`; presets `preset:biped:fall`, `preset:biped:hit_to_side`, `preset:shoot` (max 5 per call).
4. Falls and hit reactions, any source: the 5.9 Mannequin content (`MM_Death_*` x6, `MM_HitReact_*` x8) via *Add Feature or Content Pack >
   Third Person* into the plugin folder you own, selectively.

## Steps, in order

1. **Import.** `EditorToolset.SkeletalMeshToolset.ImportFile` (or Fab install). Check `GetBoneNames`, `GetSkeleton`, `GetBounds` (height ~1.6-1.9 m
   = 160-190 units; fix import scale now, not later). Name per contract: `SK_Skater_TB1`, materials `M_`/`MI_`.
2. **Sockets.** `AddSocket` `stick_socket` (right hand bone), `puck_socket` (on the root, 60 cm ahead, z 0), `head_socket` (head bone).
   If the mesh has no stick, attach a static stick (basic cylinder or the pack's stick) to `stick_socket` in the Blueprint.
3. **Decide if you need a retarget.** Same skeleton asset as the clips (Hockey Player pack; Meshy `ue5_mannequin` with UE5 clips if the import
   used the existing UE5 skeleton): **skip to step 6**. Otherwise:
4. **IK Rigs (Python).**
   ```python
   import unreal
   at = unreal.AssetToolsHelpers.get_asset_tools()
   rig = at.create_asset("IK_SkaterTB1", "/JamAssets/Skaters/Rigs", unreal.IKRigDefinition, unreal.IKRigDefinitionFactory())
   c = unreal.IKRigController.get_controller(rig)
   c.set_skeletal_mesh(unreal.load_asset("/JamAssets/Skaters/SK_Skater_TB1"))   # verify name with get_symbol
   print(c.get_characterization_report())          # which template matched, which bones are missing
   ok = c.apply_auto_generated_retarget_definition()   # chains + pelvis; False = no template matched
   c.apply_auto_fbik()
   ```
   Do the same for the source skeleton if it has no IK Rig. If `ok` is False, set the retarget root (`set_retarget_root("pelvis")` or
   `"mixamorig:Hips"`) and add chains by hand (spine, neck/head, each arm, each leg); that is where the hours go.
5. **Retargeter and batch (Python).**
   ```python
   rtg = at.create_asset("RTG_Source_To_TB1", "/JamAssets/Skaters/Rigs", unreal.IKRetargeter, unreal.IKRetargetFactory())
   r = unreal.IKRetargeterController.get_controller(rtg)
   r.set_ik_rig(unreal.RetargetSourceOrTarget.SOURCE, src_rig)
   r.set_ik_rig(unreal.RetargetSourceOrTarget.TARGET, rig)
   r.auto_map_chains(unreal.AutoMapChainType.FUZZY, True)
   r.auto_align_all_bones(unreal.RetargetSourceOrTarget.TARGET)   # T-pose vs A-pose fix
   inp = unreal.IKRetargetBatchOperationInputs()
   inp.assets_to_retarget = [ad for ad in clips]                    # unreal.AssetData list
   inp.source_mesh, inp.target_mesh, inp.ik_retarget_asset = src_mesh, tgt_mesh, rtg
   inp.target_path = "/JamAssets/Skaters/Anims"; inp.suffix = "_TB1"; inp.include_referenced_assets = True
   for res in unreal.IKRetargetBatchOperation.run_batch_retarget_with_results(inp): print(res)
   ```
   `duplicate_and_retarget` is deprecated since 5.8: do not use it even if a doc page shows it. In 5.9 the retargeter has Proximity IK, Pelvis
   Volume and Limb Twist ops: try Pelvis Volume if a chunky toy figure's hips float or sink.
6. **Look at it.** Screenshot the retargeted skate loop at three frames on the target mesh (SceneCapture). Knees forward, feet on the ice, stick
   hand closed near the stick: if not, fix the retarget pose (`set_rotation_offset_for_retarget_pose_bone`) before anything else. **A human
   glance here is cheap and worth asking for** in the next attention window.
7. **Clips you need** (names are your choice, recorded in the ledger): `Skate_Loop`, `Skate_Idle` (or a slow glide), `Skate_Left`, `Skate_Right`
   (lean), `Shoot`, `Check` (a hit reaction played forward, or an attack), `Fall` (a death clip), `GetUp` (optional; reversed fall is acceptable).
   Make montages from the one-shots (`unreal.AnimMontageFactory` via Python, verify the name): `AM_Shoot`, `AM_Check`, `AM_Fall` (no auto blend
   out; hold the last frame while knocked down), `AM_GetUp`.
8. **Blend Space.** `BS_Skate` 2D: X = turn (-5..5 rad/s), Y = speed (0..8 m/s): idle at (0,0), loop at (0,8), left/right at (-5,8)/(5,8).
   If Python cannot place samples, a 1D speed blend space plus lean done by a bone rotation is enough.
9. **Animation Blueprint, kept to three nodes.** `ABP_Skater_TB1`, parent class: the C++ `UJamSkaterAnimInstance` if A provides one (variables
   `Speed`, `TurnRate`, `bKnockedDown`), else a Blueprint AnimInstance with those variables. AnimGraph: **Blend Space player (BS_Skate) -> Slot
   `DefaultSlot` -> Output Pose**. No state machine. **No MCP tool builds AnimGraph nodes**: try the Blueprint toolsets once; if they fail, write
   a `handoff/` note for the human (10-20 minutes). Alternative without any graph: retarget `ABP_Unarmed` itself (`run_batch_retarget` retargets
   Animation Blueprints too) and replace its blend space, if Python can edit samples.
10. **The visual Blueprint.** `BP_SkaterVisual_TB1` (actor component or the class the contract names), implements `IJamSkaterVisual`:
    - `SetTeam`: dynamic material instance on the jersey slot, `TeamColour` param (add the param to the imported jersey material, or tint
      through a `MI_` of a material you own).
    - `SetNumber`: text render on the back, or the same digit material as T-A if A shares it.
    - `UpdateMotion`: set `Speed`, `TurnRate` on the AnimInstance; set the actor yaw to `facing`.
    - `PlayShoot` / `PlayCheck`: `Montage_Play(AM_Shoot / AM_Check)`.
    - `SetKnockedDown(true)`: `Montage_Play(AM_Fall)`; `false`: `AM_GetUp` or stop with a 0.25 s blend.
    - `SetHasPuck`: optional additive stick-low pose; `SetBoost`: emissive param.
11. **Swap-in.** The test cartridge named in `contracts/skater-visual.md` selects your Blueprint. Run the bot tests on it (below). Report.

## The test to run (the promotion gate)

- `PlayerBotWins` at seeds 1, 2, 7 with your visual: **same result and same state hash as with T-A** (the visual must not change play).
- `FrameBudget` at the arena camera, six skaters, shipped tier: p99 under 8 ms on device A's Shipping build. Skeletal meshes: 6 x one LOD is fine;
  set `VisibilityBasedAnimTickOption` to `OnlyTickPoseWhenRendered`; a 30k-triangle mesh seen from 52 m wants a second LOD around 5k (generate
  LODs in the mesh editor); measure before and after.
- Size: plugin package delta under the art budget share the Director gives the track.
- A screenshot pair (T-A and yours) at the arena camera, 1080p, in `devlog/` for the Director's pick.

## Pitfalls

- Anim playback follows render time; it is cosmetic only. Never read a bone or socket position back into gameplay (the shot origin and puck carry
  come from the sim, not from `stick_socket`).
- Root motion: turn it off on every clip (`enable_root_motion = False`); the sim moves the pawn.
- Mixamo: no root bone, motion on Hips, T-pose. Avoid unless a single clip is missing.
- Fab packs often list engine versions up to 5.8; installing into 5.9 usually works, check the import log for warnings.
- Licence and ledger rows before the commit: Fab Standard licence + listing URL; Mannequin = Epic content; Meshy = paid plan (Free = CC BY 4.0
  needs attribution); Tripo/Rodin per their terms. No logos on the jersey: inspect the textures.
- Retarget on a big-headed toy: arms clip into the head and the body. Shorten arm chains' reach or accept it at 52 m camera distance.
