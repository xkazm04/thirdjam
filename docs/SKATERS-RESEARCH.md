# Skaters, animation, skating physics: research for the three tracks

Status: research notes, 2026-11-11 (prep, rule 7(h): no asset, Blueprint, C++ file, animation or mesh exists). Builder R1.
Reads with `docs/HOCKEY.md` (sections 2, 5, 7) and `docs/TRACKS.md` (section 3). Recipes for the jam builders are in `tools/skills/`.

**How to read the evidence tags.** `[local]` = I read it on this machine (UE 5.9.0 install at `C:\Program Files\Epic Games\UE_5.9`,
`tools/mcp/TOOLS.md`, the Third Life Godot source). `[vendor]` = the vendor's own page or docs, quoted, not tested. `[3rd]` = a forum post,
blog or price tracker. `[unverified]` = a claim I could not check. Every URL was seen on **2026-11-11**. Prices on Fab pages rendered in CZK
for this region; the operator sees the local currency. Fab's licence dropdown did not render in a text fetch, so each Fab licence row says
"Standard (default, confirm on page)".

## 0. The answer in six lines

1. **T-A: drive component transforms from C++ (option a), no skeleton.** 4-6 hours, deterministic because every pose is a function of sim state and
   frame counters, and it is the only option an agent can build end to end without a human in an editor. Recommended.
2. **Movement and puck: a kinematic 2D simulation in C++, stepped at a fixed 60 Hz, no Chaos and no Character Movement Component in gameplay.**
   It is a direct port of the Godot formulas (quoted in section 4), it is deterministic by construction, and the bot tests can run it headless.
3. **T-B1 has a cheap path nobody planned for:** a Fab hockey player already rigged to the Epic skeleton with 14 skating clips plus a shot (section 2b),
   and the 5.9 Mannequin content that ships with the engine has 6 fall clips and 8 hit reactions. Retargeting is scriptable in 5.9 Python
   (`IKRigController.apply_auto_generated_retarget_definition`, `IKRetargetBatchOperation.run_batch_retarget`) **[local]**. The Animation Blueprint
   graph is the step no MCP tool covers: plan 10-20 minutes of a human in the editor, or skip the AnimGraph (section 2d).
4. **T-B2: Meshy is the best bet.** Its rigging API outputs a `ue5_mannequin` skeleton template, so UE5 clips play on it with no retarget **[vendor]**;
   Tripo has an `unreal` skeleton spec and the only preset list with `fall`, `hit_to_side`, `shoot`, `golf` **[vendor]**. Rodin generates meshes but
   its own page claims no rigging; to count Rodin, generate the mesh in Rodin and rig it in Meshy or Tripo.
5. **No vendor or Fab listing has a hockey stick-handling, body-check or slapshot set.** Skating exists in one Fab pack (above) and in two cinematic
   clips; everything else is the T-A code or a sourced fall/hit clip.
6. **Contradictions with the plan** (details in section 8): `SequencerAnimMixerToolset` does not exist in the inventory; no toolset authors an
   Animation Blueprint graph or an IK Rig; `duplicate_and_retarget` is deprecated in 5.8 (the 5.9 doc page still shows it); Chaos async/substepping
   are marked experimental in the 5.9 settings page; Godot's puck drag was `0.0`, and the skater's glide drag is a separate number HOCKEY.md does not carry.

---

## 1. Track T-A: a toy figure driven by code

The figure: capsule body, sphere head, cylinder stick, maybe two small skate boxes. Gameplay calls `IJamSkaterVisual` (HOCKEY.md 5):
`SetTeam(team_id, colour_hex)`, `SetNumber(n)`, `UpdateMotion(speed, turn_rate, facing)`, `PlayShoot()`, `PlayCheck()`, `SetKnockedDown(bool)`,
`SetHasPuck(bool)`, `SetBoost(kind | none)`.

### (a) Component transforms from C++ each tick (recommended)

- **What the builder writes.** One `UJamToyFigureComponent : USceneComponent, IJamSkaterVisual` in `JamGame`. It creates child
  `UStaticMeshComponent`s at runtime from engine basic shapes (`/Engine/BasicShapes/Cylinder`, `Sphere`, `Cube`; those ship with every project)
  in a hierarchy: `Root (yaw = facing) -> Lean pivot (at the skate contact point) -> Body -> Head`, `Body -> StickPivot (shoulder) -> Stick`.
  Every tick it computes local transforms from a small state struct: lean angle, bob offset, stick swing phase, tip angle. About 250-400 lines.
- **Poses (formulas, units are degrees, metres, frames at 60 Hz):**
  - Lean into turns: `lean_target = clamp(k_lean * speed * turn_rate, -max_lean, max_lean)`, `k_lean ~ 0.6` deg per (m/s x rad/s), `max_lean ~ 25`
    (full speed, full turn = 24 deg). Physically the angle is `atan(v*omega/g)`: 76 deg at 8 m/s and 5 rad/s, so a toy must under-lean. Smooth with a
    critically damped follow, `lean += (lean_target - lean) * (1 - exp(-12 * dt))`, dt = 1/60.
  - Bob with speed: `bob_z = A * |sin(pi * phase)|`, `phase += speed / stride_len * dt`, `A = 0.04 m * min(speed / 8, 1)`, `stride_len ~ 2.4 m`
    (the Godot rig's stride: 1.2 s cycle at its skate speed). Add a pitch forward of `-(10 + 6 * speed/8)` deg for the hockey crouch.
  - Stick swing (`PlayShoot`): keyframes in frames copied from the Godot shot clip (`third-life/engine/rigs/tl_rig_biped.gd` `_shoot`): base 0,
    wind-up at 18 (0.30 s), hit at 25 (0.42 s), follow-through 34-40, back to base at 51. Stick yaw: 0 -> -100 -> +10 -> +75 -> 0 deg, ease
    in-out between keys. `PlayCheck`: body pitch forward 20 deg and a 0.15 m forward lunge over 8 frames, back over 12.
  - Tip-over (`SetKnockedDown(true)`): rotate the Lean pivot about the skate contact point to 85-90 deg over 14 frames with ease-in (gravity
    feel), one 8 deg bounce over 6 frames, hold; `SetKnockedDown(false)`: back up over 20 frames. Direction = away from the checker
    (gameplay passes it via facing or a separate call; see the contract question in section 8).
  - `SetHasPuck(true)`: stick blade lowered 10 deg and forward 0.1 m. `SetBoost(kind)`: emissive param on the body material, scale pulse.
- **Determinism.** Every animated value is a pure function of (sim state at frame N, frames since the last trigger). Keep the trigger frame,
  not a wall-clock timer, and never read `GetWorld()->DeltaTimeSeconds` for the pose: use the sim's fixed dt. The visual never writes
  back into gameplay, so it cannot change `state_hash`; it can also be replayed exactly (useful for the M4 goal replay). Render-rate
  smoothing between two sim frames is allowed (interpolate with the sim's alpha) because it changes nothing the hash sees.
- **Team colour and number as material params.** One parent material `M_ToyPlastic` (flat tint, rim fresnel, plastic gloss: HOCKEY.md 6) with
  vector param `TeamColour` and scalar `Emissive` (names from the look-tiers seam, TRACKS.md 3); a `UMaterialInstanceDynamic` per figure,
  `SetVectorParameterValue("TeamColour", FLinearColor::FromSRGBColor(FColor::FromHex(colour_hex)))`. Number: two options, both with no imported
  asset: (1) a seven-segment digit drawn procedurally in the material (scalar params for the tens and ones digit; a box SDF per segment, about
  30 nodes or one Custom node); (2) a `UTextRenderComponent` on the back with the engine default font (unlit, cheap, readable from the high
  camera only if large). Option 1 looks better from above; option 2 takes 10 minutes. The parameter name for the number is not in the seam
  yet (section 8).
- **Hours:** 4-6 for one builder (component 2-3, material 1-2, a pose-review map and test 1). **Risk: low.** Look ceiling: "good toy", raised
  by the art tiers (L1-L3 change the material, not the figure).
- **Agent fit:** C++ plus a material. MCP has `MaterialToolset` (25 tools) and `MaterialInstanceToolset` **[local, tool exists]**; the C++ needs no editor.

### (b) Control Rig on a 3-5 bone skeleton

- **What the builder writes.** A skeletal mesh with 3-5 bones (root, pelvis/lean, spine, head, stick), skin weights, then a Control Rig with
  procedural nodes (Spring Interp for lean, sine for bob), driven from an Animation Blueprint's Control Rig node with variables from C++.
- **Blocker.** There is no skeletal mesh to rig. Making one in 5.9 needs either a DCC tool (Blender) or the in-editor Skeleton Editing and
  Skin Weights tools of the Modeling Mode (`SkeletalMeshModelingTools` plugin is in the install **[local]**). No MCP toolset creates bones or
  paints weights: `SkeletalMeshToolset` only imports, reads bones and edits sockets; `animation_toolset...ControlRigTools` has `add_bone` but for
  the rig hierarchy, not the mesh **[local, TOOLS.md]**. So this is hand work.
- **Determinism:** fine if driven by the same frame-based inputs; Control Rig has no randomness. Springs integrate with the anim tick dt, which
  follows the render rate, so poses would differ between machines unless the spring is replaced by the same closed-form follow as in (a).
- **Hours:** 8-12, with a human in the editor for the mesh and weights. **Risk: medium-high.** Gain over (a): soft deformation at the waist. Not worth it.

### (c) Physics-asset tip-over

- **What it is.** On knockdown, set `SimulatePhysics(true)` on a body (or a 2-body physics asset) and let Chaos tip it; on recovery, blend back.
- **Determinism:** Chaos is not guaranteed deterministic (section 5), so the tip must stay cosmetic: gameplay keeps the skater's sim position and
  `knockdown_frames`; the physics body is a visual child. Even cosmetic, runs differ, which breaks an exact replay and screenshot tests.
- **Hours:** +2-3 on top of (a). **Use:** only as an L2/L3 polish ("tipped figures" in M2) if the keyframed tip in (a) looks stiff.

**Recommendation for T-A: (a).** It is the only option with no human editor work, it is deterministic and replayable, it is fastest, and it
carries the Godot timings that already played well. Recipe: `tools/skills/skater-toy-figure.md`.

---

## 2. Track T-B1: a humanoid with sourced clips

### 2a. What Epic ships

- **UE5 Mannequins (Manny, Quinn).** In the local 5.9 install under `Templates/TemplateResources/High/Characters/Content/Mannequins` **[local]**:
  meshes `SKM_Manny_Simple`, `SKM_Quinn_Simple`, `SK_Mannequin`; rigs `CR_Mannequin_Body`, `CR_Mannequin_FootIK`, `CR_Mannequin_Procedural`,
  physics asset `PA_Mannequin`; `ABP_Unarmed` with `BS_Idle_Walk_Run`; **6 death/fall clips** (`MM_Death_Front_01..03`, `_Back_01`, `_Left_01`,
  `_Right_01`), **8 hit reactions** (`MM_HitReact_Front_Lgt_01..04`, `_Front_Med_01..02`, `_Front_Hvy_01`, `_Back_Med_01`), 4 unarmed attacks,
  walk/jog/jump sets. 126 MB on disk for the folder. No skating. Brought into a blank C++ project with *Add Feature or Content Pack* (Third
  Person), which is allowed by 7(d)(e) as content (the project still starts blank) but every file is **sourced** and unscored.
- **Licence.** I could not open the UE EULA pages (HTTP 403 to the fetcher). Epic forum staff replies say the mannequin may be used in commercial
  projects with trademark markings removed (https://forums.unrealengine.com/t/can-i-use-the-default-mannequin-in-a-commercial-game/430945/4) **[3rd]**.
  The UE5 Mannequins have no logo on the body, which avoids the trademark question. Treat as usable inside an Unreal product, ledger as sourced.
- **Game Animation Sample** (free, Epic): 500+ clips for the UE5 mannequins with a motion-matching Animation Blueprint; "only licensed for use
  with Unreal Engine, but can be used in commercial projects" (https://www.unrealengine.com/en-US/blog/game-animation-sample, via search
  snippet) **[vendor]**. Updated for 5.7 (https://www.unrealengine.com/en-US/tech-blog/explore-the-updates-to-the-game-animation-sample-project-in-ue-5-7).
  No skating; useful for starts, stops, pivots and falls. It is a whole project, so take clips selectively (size budget, PLAN 4b).
- **MetaHuman.** For 5.6 and later the Unreal Engine EULA applies; free under USD 1M revenue (https://www.metahuman.com/license) **[vendor]**.
  Not a fit: realistic look against the toy rink, heavy (grooms, LODs) against the 350 MB art budget, and a face no one sees from a 52 m camera.

### 2b. Fab candidates (searched 2026-11-11; nothing downloaded)

Fab licence terms (https://www.fab.com/eula summary) **[vendor]**: "Use the assets commercially or privately"; "Commercially distribute your
Projects with the Fab assets incorporated into it"; not allowed: "Resell or redistribute the asset ... on a standalone basis". No credit
required. Personal tier for buyers under USD 100,000 revenue (https://dev.epicgames.com/documentation/en-us/fab/licenses-and-pricing-in-fab).
A jam entry is a project with the asset incorporated: allowed. Ledger every file as `sourced` with the listing URL.

| Listing | URL | Price seen | Licence | Clips | Rig | Notes |
|---|---|---|---|---|---|---|
| **Hockey Player Animated** (Code This Lab) | https://www.fab.com/listings/bc1ddfed-dfad-412f-8280-dadafc2772fd | 435.85 CZK (~USD 19) | Standard (default, confirm on page) | **14**: idle; skate fwd/bwd/left/right each start, loop, stop; shooting; plus T-pose | "Scaled to and rigged to the Epic Skeleton" (3ds Max Biped, extra face bones); 30k tris; stick, helmet, gloves, skates, puck included | **The best fit found.** Hand-keyed, not mocap. Engine versions not listed; branding on the jersey not stated: check the preview images |
| Knockdown & Get-Up Animation Pack (Raise Creation) | https://www.fab.com/listings/17ffa308-76fe-4b5a-918e-32012ad8a580 | 239.62 CZK (~USD 10) | Standard (default) | 50+: knockdown, ground idle, get-up | not stated | Skeleton unknown; FBX |
| Mocap - Christmas Animation Library (Raised By Monsters) | https://www.fab.com/listings/a33f088c-82c2-431e-8f79-e47d05463948 | 762.9 CZK | Standard (default) | 63, two ice skating (`A_C_F_IceSkating_02`, `A_C_M_IceSkating_01`) | UE5 skeletons | Cinematic, "not meant for state machines": couple skating, not hockey |
| Low Poly Ice Skating People (StudioOchi) | https://www.fab.com/listings/24d18350-4f1b-477f-814e-a62a93668a39 | 871.92 CZK | Standard (default) | 6 | not stated (FBX/BLEND) | Recreational skaters; not UE-ready |
| MoCap Online Free Animation Pack | https://www.fab.com/listings/64c53af0-dcb7-4483-9d65-5cbc84bd9a93 | free | Standard (default); "Allows usage with AI: No" | 20: idles, walk/jog/run, transitions, one fall-back death | retargets to UE5 Manny with the seller's modified retargeters | No skating |
| Crowd Stadium - Mocap Pack (MoCap Online) | https://www.fab.com/listings/94817284-0413-4be5-b444-4b176e5d434c | not read | Standard (default) | 108 crowd clips (tagged Hockey) | retargets to UE5 Manny | Spectators, not players: an L2/L3 dressing idea |

Not found on Fab: stick-handling, body-check, slapshot sets. Outside Fab (not recommended, listed so nobody searches again): Flipped Normals
"Ice Hockey" 160 clips, FBX/Unity (https://flippednormals.com/product/ice-hockey-26349) **[3rd, not opened]**; Rokoko 12 free sports clips
(https://rokoko.com/resources/rokoko-mocap-12-free-sports-animations) **[not opened]**. Many Fab listings name engine versions up to 5.8; a 5.9
project can usually still add them, but **check the "supported versions" line** before buying.

### 2c. Mixamo

- Terms (Adobe community FAQ, 2022 post, still pinned): "available for free, with no licensing or royalty fees, for unlimited commercial or non
  commercial use"; "Video Games" listed as allowed; "the only thing you can't do is distribute the raw character and animation files"
  (https://community.adobe.com/t5/mixamo-discussions/mixamo-faq-licensing-royalties-ownership-eula-and-tos/td-p/13234775) **[vendor community]**.
  A game is not raw-file distribution, so a jam build is fine; it **can and must** be listed as a sourced asset (ledger licence: "Mixamo, Adobe
  General Terms"). An Adobe ID is needed. A 2026 guide calls Mixamo unmaintained (https://app.cinevva.com/guides/free-character-animations-rigging) **[3rd]**.
- Content: no hockey; a forum user asking for a slapshot was told to commission one
  (https://community.adobe.com/t5/mixamo-discussions/need-a-new-animation-created/td-p/12237504) **[3rd]**. Generic falls, hit reactions and pushes exist.
- Rig and retarget: Mixamo skeleton has no root bone (motion on `mixamorig:Hips`) and is a T-pose against Manny's A-pose; set Hips as the
  retarget root and align the retarget pose (https://forums.unrealengine.com/t/ik-retarget-issues-with-upper-body-mixamo-x-bot-ue5-manny/655488) **[3rd]**.
  In 5.9 `apply_auto_generated_retarget_definition` recognises known templates (see 2d); whether Mixamo is one of them is **[unverified]** (R2 checks
  with `get_characterization_report`). Use Mixamo only for a fall or hit clip the Mannequin set lacks.

### 2d. Retargeting in 5.9, and what an agent can do

Verified in the 5.9 headers on this machine **[local]** (`Engine/Plugins/Animation/IKRig/Source/IKRigEditor/Public/...`):

| Step | API (Python via `execute_python`) | Status |
|---|---|---|
| Create IK Rig for a mesh | `asset_tools.create_asset(..., unreal.IKRigDefinition, unreal.IKRigDefinitionFactory())`, then `unreal.IKRigController.get_controller(rig).set_skeletal_mesh(mesh)` | factory and controller exist; `set_skeletal_mesh` from 5.x docs, verify with `get_symbol` |
| Auto chains + pelvis | `controller.apply_auto_generated_retarget_definition()` -> bool; `get_characterization_report()` says which template matched and which bones are missing; `set_retarget_root(name)` | **header read** |
| Auto full-body IK | `apply_auto_fbik()` / `apply_auto_fbik_with_report()` | header read |
| Retargeter | `create_asset(..., unreal.IKRetargeter, unreal.IKRetargetFactory())`; `IKRetargeterController.get_controller(rtg)`: `set_ik_rig(SOURCE/TARGET, rig)`, `auto_map_chains(FUZZY, True)`, `auto_align_all_bones(...)`, retarget pose functions, ops (`add_retarget_op` with `PinBoneOp`, `RootMotionGeneratorOp`, `CurveRemapOp`) | header read + 5.9 doc page https://dev.epicgames.com/documentation/unreal-engine/using-python-to-create-and-edit-ik-retargeter-assets-in-unreal-engine |
| Batch retarget | `unreal.IKRetargetBatchOperation.run_batch_retarget(inputs)` or `run_batch_retarget_with_results(inputs)` with `IKRetargetBatchOperationInputs` (`assets_to_retarget`, `source_mesh`, `target_mesh`, `ik_retarget_asset`, `target_path`, `prefix`/`suffix`, `include_referenced_assets`, `overwrite_existing_files`) | header read. **`duplicate_and_retarget` is deprecated since 5.8** although the 5.9 doc page still shows it. Animation Blueprints are retargeted too |
| New in 5.9 | retargeter "spatially aware" ops: Proximity IK, Pelvis Volume, Limb Twist (https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-9-release-notes, Preview notes) | vendor; helps chunky proportions |
| Animation Blueprint graph | **no tool.** `BlueprintToolset` / `BlueprintDslToolset` source has no AnimGraph or state machine handling (grep of the 5.9 plugin source finds none) | gap |

What an agent can do alone: import (`SkeletalMeshToolset.ImportFile`), IK rigs, retargeter, batch retarget, montages from sequences, sockets
(`AddSocket` for `stick_socket`, `puck_socket`, `head_socket`), physics asset (`PhysicsAssetToolset.CreateFromMesh`). What needs a human or an
untested route: the AnimGraph (three nodes: Blend Space player -> Slot `DefaultSlot` -> Output Pose), and the visual check of the retarget pose
(an agent can screenshot with `SceneCaptureToolset`, but judging "the knees bend the wrong way" needs eyes).

**Way around the AnimGraph.** (1) Retarget `ABP_Unarmed` itself with `run_batch_retarget` (it is retargeted with its blend space), then
swap the walk/jog samples for skate clips: whether blend space samples are editable from Python is **[unverified]**. (2) A C++ `UAnimInstance`
subclass computes `Speed`, `Lean`, `bKnockedDown`, and plays one-shots as montages (`Montage_Play`), so the graph stays three nodes a human
makes in 10-20 minutes during an attention window. (3) Single-node mode (`PlayAnimation` + `UAnimSingleNodeInstance::SetBlendSpacePosition`)
needs no graph but pops between clips. Recommend (2).

**Honest risk, 6-10 hours.** Retarget of a skating set by script: likely (2-3 h) if source and target are both Epic-skeleton or both UE5
Mannequin, because auto-characterisation matches templates; much less likely from Mixamo or an odd rig. Blending it in an AnimBP: likely only with
the human 20 minutes. With the Hockey Player pack there is little to retarget (mesh and clips share a skeleton), so the realistic work is:
import, sockets, AnimBP, `IJamSkaterVisual` component, team colour on its jersey material, retarget 2-3 fall/hit clips from the Mannequin set.
**Estimate 6-10 h with one human touch; 40-50% chance an agent alone finishes it inside 10 h.** Look ceiling: realistic hockey player in a toy
rink; mismatch risk is high, and it is sourced (not scored).

---

## 3. Track T-B2: generated rigged characters (partners)

| | Meshy | Tripo | Hyper3D Rodin |
|---|---|---|---|
| Output formats | FBX, GLB, USDZ (rigged FBX and GLB from the rigging API) | GLB (default) or FBX | GLB, FBX, OBJ, STL, USDZ |
| Auto-rig | Yes, humanoid (and quadruped in the web app). `skeleton_template`: `meshyrig` (24 bones), `mixamo`, **`ue5_mannequin`**, `unity_humanoid`, others. Input: textured GLB, at most 380k tris, facing +Z, height in metres (https://docs.meshy.ai/en/api/rigging) | Yes. Rig v3.0-20260909 for humanoids; `spec`: tripo, mixamo, actorcore, unity, **`unreal`** (Unreal Mannequin naming), vrm. Input GLB/FBX/OBJ/STL up to 150 MB, any model (https://developers.tripo3d.com/en/docs/animations-rig) | **Not on the vendor page.** "exports rigging-ready 3D assets"; T/A-pose option for downstream rigging (https://hyper3d.ai/use-cases/animation) |
| Clips with the rig | walk and run with every rig; library of "more than 600 presets", categories Walk & Run, Daily Actions, Dancing, Fighting, Body Movements; **Text to Motion** for paid credits (https://docs.meshy.ai/en/webapp/guides/animate) | v1.0 biped presets include `fall`, `hit_to_side`, `hit_to_body_01/02`, `shoot`, `slash`, `golf`, `pitch_baseball`, `surf`; v3.0: idle, walk, run, dive, climb, jump, slash, shoot, hurt, fall, turn; max 5 per request (https://developers.tripo3d.com/en/docs/animations-retarget) | none |
| UE import | FBX with `ue5_mannequin` -> imports onto (or as compatible with) the UE5 skeleton: Mannequin clips and Game Animation Sample clips play without a retarget **[vendor claim, verify]** | FBX with `unreal` spec; same expectation **[unverified]** | FBX static mesh; rig elsewhere |
| Price / limits | Free 100 credits/month, Free outputs CC BY 4.0; paid "private license with full commercial use"; rigging and preset animation 0 credits (https://docs.meshy.ai/pricing); Pro ~USD 20/month (https://www.costbench.com/compare/meshy-vs-rodin-hyper3d/) **[3rd]** | credit-based, failed tasks free; Pro ~USD 19.90 for 3,000 credits **[3rd, unverified]** | Free pay-per-download USD 1.50/credit; Creator ~USD 15-30/month **[3rd]** |
| Skating / sport evidence | None in the library. A third party made a figure skater by generating the mesh in Meshy and the motion in DeepMotion SayMotion from text (https://deepmotion.com/post/create-and-animate-figure-skaters-using-gen-ai-in-minutes) | sports presets exist (golf, basketball, baseball, volleyball), no skating or hockey | none |

**Best bet: Meshy**, for three reasons: the `ue5_mannequin` template removes the retarget step entirely (the T-B1 risk), rigging and preset clips
cost 0 credits, and Text to Motion is the only partner route to a skating stride that is generated rather than sourced (generated content is
not "sourced": ASSETS.md). For the toy look, prompt a chunky figure **in A-pose with separated limbs** (the rigger fails on "unclear limb and
body structure"); expect retarget artefacts on a big-headed toy (short limbs against Manny proportions); the 5.9 Pelvis Volume op helps.

**Prize fit.** T-B2 is the one track that feeds the partner prizes; the plan names the Hyper3D pool ("needed for the Hyper3D pool: list it under
tools used", PREP-CHECKLIST) and the criteria are still an open item (PLAN 12). Cheapest way to count both: **mesh in Rodin, rig and clips in
Meshy** (Meshy rigging accepts any textured GLB). Main-prize relevance is through Render Royalty only if the figure clearly beats T-A on screen.

Also new in 5.9 (Preview notes): an NVIDIA Kimodo plugin for "text-described animations" and an AnimGen plugin **[vendor, not tried]**; Fab has
several third-party Kimodo front ends (e.g. https://forums.unrealengine.com/t/altercode-text-to-anim-local-ai-text-to-animation/2838544) that need the
NVIDIA Open Model License and the Llama 3 Community License. A text-to-motion skating stride retargeted onto Manny is a real T-B2 variant and
could interest the NVIDIA prize; it is untested and the licences need reading before use.

---

## 4. Skating movement model

The Godot behaviour to reproduce (`third-life/engine/kernel/api/tl_biped.gd` `_physics_process`, mode skate, read 2026-11-11 **[local]**):

```
turn:   yaw = rotate_toward(yaw, atan2(input), turn_rate * dt)            turn_rate 5 rad/s
split:  along = v . fwd ;  side = v - fwd * along
thrust: if input and not brake and along < speed: along = min(speed, along + accel * |input| * dt)   speed 8, accel 12
glide:  along = move_toward(along, 0, (glide_mu * 9.8 + brake_decel) * dt)   glide_mu = steel-on-ice 0.015 -> 0.147 m/s2
grip:   side *= max(0, 1 - (lateral_grip + brake * 0.5) * dt)             lateral_grip 3 /s
v = fwd * along + side
```

| | Character Movement Component (low friction) | Chaos rigid-body pawn | **Kinematic pawn, own integration (recommended)** |
|---|---|---|---|
| Feel | Ground friction, braking friction, acceleration knobs give an ice slide (https://docs.unrealengine.com/4.26/en-US/API/Runtime/Engine/GameFramework/UCharacterMovementComponent/GroundFriction/index.html), but the forward/side split above is not native: override `CalcVelocity` or a custom movement mode | forces and a sideways damping force; tuning by trial | the formula above, line for line; HOCKEY numbers carry over unchanged |
| Determinism at 60 Hz | steps on the frame delta (split by `MaxSimulationTimeStep`); deterministic only if the frame delta is fixed | not guaranteed; async tick and substepping are "still experimental" in 5.9 (https://dev.epicgames.com/documentation/unreal-engine/physics-settings-in-the-unreal-engine-project-settings) | by construction: fixed dt, fixed order, no engine solver |
| Checks, knockdowns | `MOVE_Custom` mode, launch | physical impulses, but chaotic outcomes | circle-circle contact, relative normal speed > `check_speed_min` -> knockdown for `knockdown_frames` |
| Headless bot tests | need a world and the component | need a world and Chaos | **pure C++: step N frames in an automation test, no map** |
| Network | built for it (irrelevant: single player, no replication) | n/a | n/a |
| Cost | fighting a walker component | fighting a solver | ~300 lines for skaters + puck + rink |

**Recommendation: the kinematic simulation.** A `FJamSim` (plain C++ struct) holds skaters, puck, score, frame counter and a seeded
`FRandomStream`; `Step()` advances exactly 1/60 s. A world subsystem or the game mode calls it from Tick through an accumulator
(`acc += dt; while (acc >= 1/60) { Step(); acc -= 1/60; }`, cap 4 steps per frame) and actors copy the sim state into their transforms
(interpolated with `acc*60` for smooth rendering). Collision is 2D: skater circle (r ~0.35-0.4 m) against the rounded-rectangle rink
(60 x 30, corner radius 5: a signed distance function), circle-circle between skaters, puck against both. Chaos stays for cosmetics only
(tipped figures, debris). The Mover plugin (Epic's CMC successor, fixed-tick physics-thread variant "Chaos Mover") is still experimental/beta
by every source I found (https://dev.epicgames.com/documentation/unreal-engine/mover-in-unreal-engine,
https://forums.unrealengine.com/t/any-roadmap-updates-on-mover/2833898) **[vendor/3rd]**: not for a 93-hour jam. Recipe: `tools/skills/skating-physics.md`.

---

## 5. Puck

| | Chaos rigid body | **Kinematic (recommended)** |
|---|---|---|
| Board bounce | physical material restitution, combine mode | `v -= (1 + e) * (v . n) * n` when moving into the board; `e = rink.board_restitution` |
| Ice friction | physical material friction (contact with a 0.08 m disc is unstable: it tips, rolls, jitters) | `v *= max(0, 1 - drag * dt)` (or a linear decel), `drag = puck.drag`, Godot used 0.0 |
| 30 m/s cap | clamp in a callback | `if |v| > max_speed: v *= max_speed / |v|` |
| Shot | impulse; exit speed depends on mass and contact | `v = aim * shot.speed` (18 m/s regardless of mass, as in Godot) |
| Tunneling | at 30 m/s the puck moves 0.5 m per frame, four puck widths: needs CCD or substeps | 4 fixed substeps per frame (0.125 m each, about one radius) or a swept circle test |
| Determinism | not guaranteed: no "enhanced determinism" setting appears on the 5.9 physics settings page; async tick, substepping marked experimental; forum reports of divergence at fixed async ticks (https://forums.unrealengine.com/t/possible-non-determinism-with-chaoswheeledvehiclemovementcomponent-constraintsuspsension/2705335) | bit-identical on the same build and machine |

**If a builder must use Chaos for the puck anyway**, the Project Settings to set and test: Physics > Simulation: Substepping on, Max Substep
Delta Time 0.004167 (1/240), Max Substeps 4, Max Physics Delta Time 0.0333; Tick Physics Async off (experimental); puck body: CCD on, mass
0.17 kg, linear damping = drag, physical material with restitution combine Max and friction combine Min; constrain to the XY plane (lock Z
translation and X/Y rotation); General > Framerate: Use Fixed Frame Rate 60 for test runs. Then run the same seed twice and compare hashes;
expect divergence and do not let the bot test depend on it.

**Fixed frames for tests (either model).** `-deterministic` is "Shortcut for -UseFixedTimeStep -FixedSeed"
(https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-command-line-arguments-reference), with `-FPS=60` for the step size
(https://forums.unrealengine.com/t/fix-or-cap-time-step-for-debugging-purposes/305543/3) **[3rd for -FPS]**. With the kinematic sim the test does
not need them at all: it calls `Step()` 3,600 times in a C++ automation test.

---

## 6. Animation via agents: what the 5.9 MCP toolsets cover

From `tools/mcp/TOOLS.md` (inventory of 2026-11-11, UE 5.9.0-60351404). "tool exists" = in the inventory; "verified" = called by someone and
seen to work. **Nothing below is verified by me**; R2 verifies.

| Toolset (as named in the brief) | What is really there | Animation use | Status |
|---|---|---|---|
| `AnimationAssistantToolset` | only `AnimationAssistantToolset.SequencerDirectorToolset`, 4 tools: event keys, Director Blueprint endpoints, dynamic binding | none for skaters | tool exists |
| `ControlRigEditor` | `ControlRigSequencerEditorLibrary`, 230 tools: Control Rig tracks in level sequences, set control transforms at frames, bake to Control Rig, **`ExportAnimSequenceFromSequencer`**, auto-bake | **author original clips by keying a Control Rig (e.g. `CR_Mannequin_Body`) in a level sequence and exporting an Anim Sequence**: original, scored as ours | tool exists |
| `SequencerAnimMixerToolset` | **not in the inventory.** Nearest: `SequencerAnimTools.SequencerAnimToolsEditorLibrary` (29 tools: motion trails, Time Posing, Edit Pivot) | key editing helpers | name does not exist |
| `animation_toolset` | `animation_toolset.toolsets.controlrig.ControlRigTools`, 44 tools: create a Control Rig, add bones/controls/nulls, graphs, RigUnit nodes, pins, variables, `import_bones_from_asset` | build a procedural Control Rig graph on an existing skeleton | tool exists |
| (others relevant) | `SkeletalMeshToolset` (import, sockets, physics asset assign), `PhysicsAssetToolset` (create from mesh, bodies, constraints), `SequencerToolsFunctionLibrary.ExportAnimSequence` / `LinkAnimSequence`, `FabToolset` (search, read listing, install owned or free), `python_scripting.execute_python` (full `unreal` module) | import, sockets, ragdoll, Fab install | tool exists |

| Can an agent, without a human in the editor... | Answer |
|---|---|
| create an Animation Blueprint asset | probably, via `execute_python` (`AnimBlueprintFactory`) **[unverified]** |
| add a state machine / AnimGraph nodes | **no dedicated tool**; the Blueprint toolsets' source has no AnimGraph handling; generic `CreateNode` on an AnimGraph is untested |
| retarget | **yes via `execute_python`** (IK Rig and Retargeter controllers, `run_batch_retarget`: section 2d); no MCP toolset of its own |
| bake a Control Rig to an animation | yes: `BakeToControlRigFromReference`, `ExportAnimSequenceFromSequencer`, `AddAutoBake` exist |
| build a procedural Control Rig | yes: `ControlRigTools.create`, `create_node`, `connect_pins` exist |
| install a Fab pack | yes for free or owned listings: `FabToolset.InstallFabListing` (sign-in opens a browser: a human step once) |

---

## 7. The three tracks side by side

| | T-A toy figure (code) | T-B1 humanoid, sourced clips | T-B2 generated rigged |
|---|---|---|---|
| Hours (one builder) | 4-6 | 6-10 with a 20 min human AnimGraph step; 10-14 from Mixamo | 6-10 (generation iterations 2-3, rig/import 1-2, AnimBP 2, interface 2) |
| Risk | low | medium: AnimGraph, retarget pose, look mismatch | medium-high: rig quality on toy proportions, no skating clip unless Text to Motion works |
| Look ceiling | stylized toy, consistent with the rink; rises with the light tiers | realistic player in a toy world (mismatch); best motion quality | stylized and on-theme if the prompt lands; motion limited to presets |
| Prize fit | main score (Fun, Visuals) | none; sourced, unscored | partner pools (Hyper3D via Rodin mesh; Meshy/Tripo), Render Royalty if it beats T-A |
| Determinism | full: pose = f(state, frames) | gameplay unaffected; anim playback follows render dt (cosmetic only) | same as T-B1 |
| Agent-only? | yes | almost (AnimGraph) | almost (AnimGraph; web generation needs accounts/API keys) |

**Decision.** Keep the plan: **T-A ships** in M1 and stays the fallback. Device B runs **T-B2 with Meshy first** (H24-H48), because with the
`ue5_mannequin` template it absorbs most of T-B1's value (UE5 fall and hit clips play on it) and it is the only track with prize upside. Run
**T-B1 only as the Hockey Player pack variant** (H12-H36), because it is the only source of real skating strides; its promotion gate is the
same (player bot wins, frame gate holds) plus a look gate: a screenshot at the arena camera next to T-A, Director picks. Both device-B tracks
share one `BP_SkaterVisual_<track>` and one AnimBP shape (Blend Space by speed and turn -> Slot -> Output), so the human AnimGraph step is done once.

## 8. Contradictions and open questions for the Director

1. **`SequencerAnimMixerToolset` does not exist** in the 5.9 inventory; `AnimationAssistantToolset` has only the 4 Sequencer Director tools. Plans or
   prompts naming them should be corrected.
2. **No toolset authors an AnimGraph or an IK Rig/Retargeter.** Retarget only through `execute_python`. Plan a human 20-minute AnimGraph step in an
   attention window (H12 or H24) for device B.
3. **`duplicate_and_retarget` is deprecated since 5.8** (`UE_DEPRECATED(5.8, ...)` in `IKRetargetBatchOperation.h`), yet the 5.9 doc page uses it. Use
   `run_batch_retarget` / `run_batch_retarget_with_results`.
4. **Chaos is not a safe base for deterministic gameplay**: no determinism switch on the 5.9 physics settings page; async and substepping are
   experimental. HOCKEY.md's determinism test (same state hash per seed) assumes a deterministic sim: build it kinematic.
5. **HOCKEY.md section 2 is missing the skater's glide drag.** Godot used `glide_mu * 9.8` with steel-on-ice friction 0.015 (0.147 m/s2) plus a
   separate brake; `game.rink.ice_friction` should be defined as that `glide_mu` (and the puck's `drag` as its own 1/s value; Godot's was 0.0).
   The cartridge has no `brake` value; Godot's skater had `skate_brake`.
6. **Contract gaps in `IJamSkaterVisual`**: no direction for the knockdown tip (add a `knock_dir` or reuse facing), and no material parameter name for
   the jersey number (propose `NumberTens`, `NumberOnes` scalars or `JerseyNumber`, in `contracts/look-tiers.md`).
7. Porting the Godot pose keyframes (our own Third Life code) during the jam is derived from pre-existing team code, not content made before the
   jam in this repo; I read it as allowed (it is a tool's data, re-keyed in the jam), but the Director should say so in the dev log.

## 9. What to buy or claim before H0 (operator; nothing downloaded)

| Item | Action | Why | Cost |
|---|---|---|---|
| Epic account signed in to Fab in the browser | sign in once; the editor's `FabSignIn` opens a browser later | `FabToolset.InstallFabListing` needs it | free |
| **Hockey Player Animated** | open the listing, check the jersey preview for logos and the supported engine versions; if clean, **buy (adds to library, do not download)** | only skating clip set found; T-B1 | ~USD 19 |
| Knockdown & Get-Up Animation Pack | favourite; buy only if the T-B1 track starts | falls and get-ups beyond the 6 Mannequin deaths | ~USD 10 |
| Game Animation Sample | "add to library" (free) | starts, stops, falls for UE5 skeleton figures | free |
| MoCap Online Free Animation Pack | favourite only | fallback locomotion; no skating | free |
| **Meshy** account on a paid plan (or the partner perk) | claim the account only; no generation before H0 (a generated figure is game content) | Free outputs are CC BY 4.0; paid is "private license with full commercial use"; T-B2 | ~USD 20/month or perk |
| Tripo account and API key | claim | `unreal` spec, fall/hit presets | perk |
| Hyper3D Rodin account | claim | Hyper3D pool needs Rodin listed under tools | perk |
| Adobe ID for Mixamo | make sure one exists | fallback clips | free |
| Read the NVIDIA Open Model License (Kimodo) | read only | if T-B2 tries the 5.9 Kimodo plugin | free |

## 10. Sources (all seen 2026-11-11)

Local: `C:\Program Files\Epic Games\UE_5.9\Templates\TemplateResources\High\Characters\Content\Mannequins\**`;
`...\Engine\Plugins\Animation\IKRig\Source\IKRigEditor\Public\RigEditor\IKRigController.h` (lines 491-549);
`...\RetargetEditor\IKRetargetBatchOperation.h` (lines 122-177, 234-264); `...\RetargetEditor\IKRetargeterController.h` (382, 550);
`...\Engine\Plugins\Experimental\Toolsets\BlueprintToolsets\Source` (grep, no AnimGraph); `tools/mcp/TOOLS.md`;
`third-life/engine/kernel/api/tl_biped.gd` (155-200); `third-life/engine/rigs/tl_rig_biped.gd` (381-534);
`third-life/dist/ThirdLife/games/ice_hockey_three_against/data/tuning.json`; `third-life/engine/kernel/data/surfaces.json`.

Web: https://www.fab.com/eula · https://dev.epicgames.com/documentation/en-us/fab/licenses-and-pricing-in-fab ·
https://www.fab.com/listings/bc1ddfed-dfad-412f-8280-dadafc2772fd · https://www.fab.com/listings/17ffa308-76fe-4b5a-918e-32012ad8a580 ·
https://www.fab.com/listings/a33f088c-82c2-431e-8f79-e47d05463948 · https://www.fab.com/listings/24d18350-4f1b-477f-814e-a62a93668a39 ·
https://www.fab.com/listings/64c53af0-dcb7-4483-9d65-5cbc84bd9a93 · https://www.fab.com/listings/94817284-0413-4be5-b444-4b176e5d434c ·
https://www.unrealengine.com/en-US/blog/game-animation-sample · https://www.metahuman.com/license ·
https://forums.unrealengine.com/t/can-i-use-the-default-mannequin-in-a-commercial-game/430945/4 ·
https://community.adobe.com/t5/mixamo-discussions/mixamo-faq-licensing-royalties-ownership-eula-and-tos/td-p/13234775 ·
https://forums.unrealengine.com/t/ik-retarget-issues-with-upper-body-mixamo-x-bot-ue5-manny/655488 ·
https://dev.epicgames.com/documentation/unreal-engine/using-python-to-create-and-edit-ik-retargeter-assets-in-unreal-engine ·
https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-9-release-notes ·
https://dev.epicgames.com/documentation/unreal-engine/physics-settings-in-the-unreal-engine-project-settings ·
https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-command-line-arguments-reference ·
https://dev.epicgames.com/documentation/unreal-engine/mover-in-unreal-engine · https://docs.meshy.ai/en/api/rigging ·
https://docs.meshy.ai/en/webapp/guides/animate · https://docs.meshy.ai/pricing · https://developers.tripo3d.com/en/docs/animations-rig ·
https://developers.tripo3d.com/en/docs/animations-retarget · https://hyper3d.ai/use-cases/animation ·
https://www.costbench.com/compare/meshy-vs-rodin-hyper3d/ · https://deepmotion.com/post/create-and-animate-figure-skaters-using-gen-ai-in-minutes ·
https://forums.unrealengine.com/t/altercode-text-to-anim-local-ai-text-to-animation/2838544


## Corrections from lane E1 (measured against the Meshy API on 2026-11-12)
- Rigging and animation are **not free** through the API: rig 5 credits, each animation action 3, text-to-motion prime 10 / swift 3, preview 5 (6-lite, t2.5) or 20 (7.1), texture 10.
- The `skeleton_template: ue5_mannequin` rig (added 2026-11-10) carries UE5 bone **names** on 28 bones (`pelvis` at the top, no `root`, no twist/IK/finger bones, extra `_End` bones): it is not the UE5 Mannequin skeleton, so Manny clips still need a retarget; the matching names should make it quick. Unreal import untested (not allowed before the jam).
- Meshy text-to-motion produced a skating stride that reads as skating; library clip 187 `Knock_Down` works as a check-fall; the stride carries 0.9 m forward / 0.38 m sideways drift on `pelvis` over 4 s that must be stripped (no root bone).
- Full numbers, task ids and the device-B command sequence (about 55 credits per figure with four clips): `docs/experiments/MESHY.md`.
