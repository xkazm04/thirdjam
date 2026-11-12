# SKI report: import of the P4B (broadcast) and P3A (clay) skater figures, plus Rclay (device B)

Lane SKI, 2026-11-12, 15:33 to 15:56 local. Tree `C:/Users/kazda/jam/wt/b-vfx` (branch `lane/b-vfx`). I ran no git command and did not build. The editor ran on port 8002 on the existing binaries, started with `tools\run\editor.ps1`; no modal box appeared. I closed it through `ExitEditor`, after `new_blank_map` had discarded the in-memory `Rink` edits.

- **No map was saved, nothing under `/Game/` was written, and no existing asset was modified.** A `find -newer` over `JamAssets/Content` after the session lists only the 84 new files.
- `_Scratch/` does not exist.
- The port 8000 editor shared the GPU, so **I took no frame numbers**.

The scratch scripts are re-runnable and live in `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/ski/` (`$SKI` below).

## Verdict per step

| # | Step | Result | Evidence |
|---|---|---|---|
| 1 | Import P4B as `SK_SkaterTB2_P4` | **PASS** | Own skeleton `SKEL_SkaterTB2_P4` (32 bones), no physics asset, 31 takes `AS_SkaterTB2_P4_<Take>`. 180 cm tall, faces +X. Slots: `M_Skater_P4B` (body), `Trim`, `Stick`. `T_SkaterTB2_P4_BaseColor` is 1K in game. `MI_SkaterTB2_P4` is an instance of `M_SkaterTB2` and keeps the master's hue-key defaults, which fit this blue: 75 % of the texture atlas keys, median mask 1.0. |
| 2 | Import P3A as `SK_SkaterTB2_P3` and join the clay look | **PASS** | Same structure, 170 cm tall. `MI_SkaterTB2_P3` is an instance of `M_SkaterTB2`, so `MPC_Look` drives the clay roughness, the triplanar thumb-dent bump and the felt rim exactly as on T-B2. Values: KeyGain 8, **KeyBias 0.5**, Roughness 0.9, two-sided override on. With KeyBias 0.15 the black clay skates turned brown-red on the orange team (`ski-slap-windup`). **Blade holes: not visible** at 1.7 m from the side, with or without two-sided (`ski-p3-blades`). I kept the two-sided override as cheap insurance; the lead may drop it. |
| 3 | Rclay: `MI_SkaterTB2_Clay` | **PASS, with a new master** | `M_SkaterTB2` has no normal-map input, so I made **`M_SkaterGen`** (details below the table). It is identical to the master while `UVNormalStrength` is 0. `MI_SkaterTB2_Clay` uses `T_SkaterTB2_Clay_BaseColor` and `T_SkaterTB2_Clay_Normal` (BC5), with UVNormalStrength 1 and Roughness 0.9. In PIE it was a component material override on `BP_SkaterVisual_TB2`. The existing instances and the master are untouched. |
| 4 | One Blueprint per figure | **PASS, as duplicates, not child classes** | `BP_SkaterVisual_TB2_P4` and `BP_SkaterVisual_TB2_P3` are duplicates of `BP_SkaterVisual_TB2`, re-pointed as listed below. A child class does **not** work with the current structure (reason below). Both compile to `BS_UP_TO_DATE` and implement `JamSkaterVisual`. Their `.uasset` import tables reference only their own figure's assets. |
| 5 | Stills | **PASS** (19 stills) | Listed below. They cover: Stride at 6 m and 30 m; the slap through the `PlayShoot` montage, wind-up and contact; fall, get-up and recovery through `SetKnockedDown`; `Goalie_Hold`; P4B with and without the cel post-process; P3A against Rclay under `LR_P3_Clay` at 6 m and 30 m. |
| 6 | Sizes | **FAIL on disk against 12 MB** (P4 14.38 MB, P3 12.80 MB). Cooked size not measured. | Table below. The overshoot comes from the 2K texture source and the 31 takes. Both can be cut (see "What the lead must decide"). |

**`M_SkaterGen`, in detail:**
- It is a duplicate of `M_SkaterTB2` as it stood at 15:33.
- It adds `NormalTex` (texture parameter, default `/Engine/EngineMaterials/FlatNormal`) and `UVNormalStrength` (scalar, default 0).
- A Custom node, `UVNormalMix` (reoriented-normal blend), blends the UV normal onto the `MPC_Look` clay normal before the Normal output.

## Import settings used

- **FBX import:** `AssetImportTask` with `FbxImportUI`, settings as below. Source: `skater-lab/skh/export/<P4B|P3A>/SK_Skater_<c>_loopcut.fbx`, the loopcut files, as for T-B2.
  - on: `import_mesh`, `import_as_skeletal`, `import_animations`; `mesh_type_to_import` is skeletal; `animation_length` is the exported time.
  - off: `import_materials`, `import_textures`, **`create_physics_asset`**.
  - no skeleton given, so the import makes a new one.
  - defaults kept: Import Uniform Scale 1.0, Force Front X off, Convert Scene on.
- **Take names:** Unreal named the takes `SK_SkaterTB2_<K>Armature_<Take>`. I renamed them to `AS_SkaterTB2_<K>_<Take>` and the skeleton to `SKEL_SkaterTB2_<K>`. All 31 takes are on the new skeleton. The skh files imported without the FBX parser error that the stripped S4/S5 files once hit.
- **Textures** come from the PNG beside each FBX, with B8's rules applied by hand:
  - base colours: `TC_Default` with `CompressionNoAlpha` (BC1), Lossy **Medium**, virtual texturing off, LOD group World, **`MaxTextureSize` 1024**. No still needed 2K: the 4 m close-ups `ski-p4-plain` and `ski-front-p4-p3` are sharp at 1K.
  - normal map: `TC_Normalmap` (BC5), sRGB off, group WorldNormalMap, 1024.
- **Sockets:**
  - `stick_socket` and `head_socket` are on their bones; `puck_socket` is on bone `stick_socket` (B8's convention).
  - The relative transforms are copied from `SK_SkaterTB2`, which has the same stick and the same rest orientation on the socket bones.
  - Measured at rest: world rotation identity, scale 1.0.
- **Slots:** body uses `MI_SkaterTB2_<K>`. `Trim` uses the existing `MI_SkaterTB2_Trim` and `Stick` the existing `MI_SkaterTB2_Stick`; both are referenced, not changed.
- **One accident, cleaned up:**
  - I ran the P3A import twice. The second run left an orphan `SK_SkaterTB2_P3_Skeleton`, unsaved and unreferenced.
  - I deleted it before anything was saved.
  - `SK_SkaterTB2_P3` and all of its takes reference `SKEL_SkaterTB2_P3` (checked).

## What was re-pointed, so the lead can repeat it after the other lane's `BP_SkaterVisual_TB2` changes land

Route: duplicate `BP_SkaterVisual_TB2` to the new name, then:
1. **Graph** (DSL `Edit`, replace all; script `$SKI/b2.mjs`):
   - ``cast `/JamAssets/Skaters/ABP_SkaterTB2` `` becomes `ABP_SkaterTB2_<K>`. There are two such casts, in `UpdateMotion` and `SetKnockedDown`.
   - The four variable sets `/JamAssets/Skaters/ABP_SkaterTB2.{Speed,TurnRate,bFallForward,bKnockedDown}` become `ABP_SkaterTB2_<K>.…`.
   - The `Montage_Play` literals: `AM_SkaterTB2_Shot_Slap` becomes `AM_SkaterTB2_<K>_Shot_Slap` (PlayShoot), and `AM_SkaterTB2_Check` becomes `AM_SkaterTB2_<K>_Check` (PlayCheck).
2. **Component `Mesh`** (script `$SKI/b3.py`): `skeletal_mesh_asset` = `SK_SkaterTB2_<K>` and `anim_class` = `ABP_SkaterTB2_<K>_C`. Stencil 1, collision and the rest come with the duplicate.
3. **Nothing else.** The material comes from slot 0 of the mesh (`MI_SkaterTB2_<K>`), because BeginPlay builds `JerseyMID` from element 0.

**AnimBP** (`$SKI/a2.py`):
1. Duplicate `ABP_SkaterTB2`.
2. Set `target_skeleton` to `SKEL_SkaterTB2_<K>`.
3. Re-point the five sequence players (Idle_Faceoff, Fall, Fall_Forward, GetUp, GetUp_Front) and the blend-space player to the `<K>` assets.
4. Compile.

**Blend space** (`$SKI/a1.py`): a new asset on the figure's skeleton. It copies `BS_SkaterTB2_Skate`'s axes, axis smoothing (0.15 / 0.25 s) and 14 samples, with the clips swapped to `<K>`.

**Montages:** new, made from `AS_SkaterTB2_<K>_Shot_Slap` and `_Check`.

**Why not a child class:** `BP_SkaterVisual_TB2` casts the anim instance to the concrete class `ABP_SkaterTB2` and plays montages named as literals.
- A child Blueprint could swap the mesh. But its AnimBP must then be `ABP_SkaterTB2` or a child of it.
- A child AnimBP keeps the parent's target skeleton (`SKEL_SkaterTB2`), so it cannot play the P4/P3 takes on their own skeletons.

Two small changes to `BP_SkaterVisual_TB2` would make child classes possible. I did not make them, because the other lane owns the Blueprint now:
- (a) Drive the four AnimBP variables through an anim interface instead of the cast, for example `IJamSkaterAnim.SetMotion(Speed, TurnRate, bDown, bFwd)` implemented by each figure's AnimBP.
- (b) Make the two montages class-default variables (`ShootMontage`, `CheckMontage`).

Then `BP_SkaterVisual_TB2_P4` becomes a child that overrides `Mesh` (mesh and anim class) and the two montage defaults, and it inherits the numbers and crests.

## Interface calls on the new Blueprints (driven from Python in PIE)

Both Blueprints are the duplicated TB2 graph, so every call behaves exactly as on T-B2:
- **`SetTeam`** works before and after BeginPlay. It writes `TeamColour` on the dynamic instance of slot 0; I read back (0.80, 0.13, 0.01) after `SetTeam(1, orange)`.
- **`UpdateMotion`:** Speed and TurnRate reach `ABP_SkaterTB2_<K>`, and the mesh yaw follows the facing. After a get-up the yaw eases back over 72 calls, so a script that calls it only once still sees the down yaw, as on T-B2.
- **`PlayShoot`** plays `AM_SkaterTB2_<K>_Shot_Slap`, and **`PlayCheck`** plays `AM_SkaterTB2_<K>_Check`. I read the active montage back by name.
- **`SetKnockedDown`:** Fall or Fall_Forward depending on the direction, then GetUp or GetUp_Front on release (`ski-fall-*`).
- **`SetBoost`:** `Emissive` 1.0 for any boost, 0.5 for `None` (read back).
- **No-ops: `SetNumber` and `SetHasPuck`**, because they are no-ops in the TB2 graph I duplicated. The other lane's jersey number and crest work is **not** inherited (see above).
- **Sockets:** all three exist on both meshes (`does_socket_exist`).

## What takes the team colour and what does not

| Figure | Takes the team colour | Keeps its texture colour |
|---|---|---|
| P4B | Helmet, jersey, shoulder pads, **gloves**, pants and socks. All are the same jersey blue on the mesh, because the prompt asked for deep blue gear. | Face and hair, visor cage, the white trim (V-neck, sleeve, hem and sock stripes), black skates with laces, steel blades, stick |
| P3A | Helmet, jersey and legs (all deep blue clay on the mesh) | Face, white mittens, white stitched seams, black clay skates (after KeyBias 0.5), the helmet holes, stick |
| Rclay (`SK_SkaterTB2`) | As on T-B2: suit and helmet | Face, white stripes, mitts (the glove key makes them dark leather), skates |

Blue and orange both read on both new figures at 6 m and at 30 m (`ski-lineup`, `ski-stride30`). The glove key does nothing on P4B and P3A, because neither has yellow mitts.

## How P4B reads

- **Under the night rig with the cel post-process** (`ski-p4-cel`, `ski-lineup`): it reads as a tall, broad-shouldered hockey player. The ink outline and the toon ramp put it in the P2 language, but its realistic face and proportions look out of place next to T-B2's toy. The jersey is a flat team colour, because `JerseyLum` 0.08 saturates every keyed pixel and no cloth shading is left.
- **Plain lighting** (`ski-p4-plain`, with the toon ramp and outline removed from `PPV_L2_Night` in memory): a clean, semi-realistic broadcast candidate. The face behind the visor cage, the gloves, the shoulder pads and the laced skates all read. For P4 I would raise `JerseyLum` to about 0.25 on its instance, so that the cloth keeps its painted shading.
- **At 30 m** it is the thinnest of the four figures (`ski-stride30`).

## The clay question: which one reads as clay at game distance?

**P3A.**
- **At 6 m** (`ski-clay-6m`, `ski-clay-6m-stride`): both figures get the same matte grade and triplanar dents from `MPC_Look`, but P3A *is* a clay object. It has a lumpy round helmet with thumb holes, irregular hand-made limbs, stitched white seams, black clay boots and a modelled face. Rclay is the T-B2 toy in matte paint, and its own UV dent map adds little on top of the MPC bump.
- **At 30 m with the rig's fixed 7 m focus,** nothing reads, because of the tilt-shift blur (`ski-clay-30m`).
- **At 30 m with the focus moved to the figures** (in memory, `ski-clay-30m-focus`): neither shows dents, and both look like matte toys. P3A keeps its silhouette cues: the helmet shape and the black skates.

So P3A is the figure for P3, and Rclay is only the zero-risk fallback.

## Stills (`devlog/shots/`, 1920 x 1085, PIE)

| Still | What |
|---|---|
| `20261112-154335-B-ski-lineup.png` | P3A orange and blue, P4B orange and blue, TB2 and TB1 for scale, under `LR_L2_Night` |
| `20261112-154415-B-ski-stride6-new.png` | Stride at 7 m/s from behind, game camera 6 m back and 3.5 m up: two P3A, two P4B |
| `20261112-154415-B-ski-stride6-mix.png` | Same camera: P3A, P4B, TB2, TB1 |
| `20261112-154437-B-ski-stride30.png` | Stride from 30 m: all six |
| `20261112-154537-B-ski-slap-windup.png`, `-slap-contact.png` | Slap at 0.27 s and 0.40 s, side view: P3A, P4B, TB2 |
| `20261112-154537-B-ski-front-p4-p3.png` | Front close-up of what takes the team colour (P3A after KeyBias 0.5) |
| `20261112-154644-B-ski-p3-blades.png` | P3A skates, one-sided against two-sided |
| `20261112-154801-B-ski-fall-down.png`, `-fall-getup.png`, `20261112-154802-B-ski-fall-recovered.png` | Falls in both directions, get-up, back to skating |
| `20261112-154901-B-ski-goalie-front.png`, `-goalie-side.png` | Goalie_Hold, played as a slot montage (it is not wired in the AnimBP) |
| `20261112-154957-B-ski-p4-cel.png`, `-p4-plain.png` | P4B with and without the cel post-process |
| `20261112-155143-B-ski-clay-6m.png`, `20261112-155144-B-ski-clay-6m-stride.png`, `-clay-30m.png`, `20261112-155145-B-ski-clay-30m-focus.png` | P3A against Rclay under `LR_P3_Clay` |

**The test set:**
- **Map:** `/Game/Maps/Rink` in memory (never saved), with `LR_L2_Night` as a sub-level.
- **Ice:** a 90 x 45 m plane with `MI_Ice_Night` and stencil 2.
- **Figures:** spawned in the editor world before PIE.
- **Clay:** `LR_P3_Clay` was streamed in during PIE with B9's `clayload.py`, which hides L2 and Rink's sky light and directional light. The rig's `BP_LookParams` had already set the clay values itself.
- **The fingerprint pattern on the ice in the clay stills comes from the stand-in plane** (local-space triplanar on a plane scaled 90 x 45), not from the real rink.

## Sizes (bytes on disk; a `.uasset` holds the editor source, not the cooked data)

| Figure | Files | Bytes | Breakdown |
|---|---:|---:|---|
| P4 (`*_P4*`) | 40 | **14,381,455** | mesh 4,208,384; texture 3,028,518 (2K source, 1K cooked); 31 takes 6,447,876; ABP 339,246; BP 287,252; BS 28,189; montages 17,900; skeleton 13,688; MI 10,402 |
| P3 (`*_P3*`) | 40 | **12,798,616** | mesh 3,551,201; texture 2,075,009; 31 takes 6,473,606; ABP 338,915; BP 286,911; BS 27,935; montages 17,651; skeleton 13,022; MI 14,366 |
| Rclay and the new master | 4 | 4,236,727 | `T_SkaterTB2_Clay_BaseColor` 2,991,733; `T_SkaterTB2_Clay_Normal` 1,186,735; `M_SkaterGen` 45,567; `MI_SkaterTB2_Clay` 12,692 |
| **Total new** | 84 | **31,416,798** | |

Both figures are over 12 MB on disk. The cooked size is not measured, because nothing was cooked. By arithmetic, each base colour cooks at 1024 in BC1 to about 0.7 MB, and the BC5 normal to about 1.4 MB, instead of the 2 to 3 MB source. The 31 takes are ACL-compressed at cook and will be well under their source bytes.

Two ways to get under 12 MB on disk:
- Re-import the base colours from a 1K PNG. This saves about 2.3 MB on P4 and 1.5 MB on P3.
- Delete the 15 takes that nothing uses yet: Aim_*, Faceoff_*, Foul_Reaction, Lift, Pass, Poke, Shot_Charge, Stumble and Goalie_Block_*. This saves about 2.8 MB per figure.

## What device A needs to select these figures per look tier (proposal)

Today `AHokejGameMode::ResolveVisualClass` maps `game.skater.visual` = `generated_b2` to `BP_SkaterVisual_TB2`.

**Proposal:** keep `generated_b2` as the figure family, and let the active look tier choose the class:

| `game.look.tier` | Class for `generated_b2` |
|---|---|
| `l2_night` (P2 cel, shipped) | `/JamAssets/Skaters/BP_SkaterVisual_TB2.BP_SkaterVisual_TB2_C` |
| `p3_clay` | `/JamAssets/Skaters/BP_SkaterVisual_TB2_P3.BP_SkaterVisual_TB2_P3_C` |
| `p4_broadcast` (when it exists) | `/JamAssets/Skaters/BP_SkaterVisual_TB2_P4.BP_SkaterVisual_TB2_P4_C` |
| anything else, or a failed load | `BP_SkaterVisual_TB2` (then the toy, as today) |

How device A could do it:
- **Lookup:** a small table (tier to class path) next to `ResolveVisualClass`, or the per-tier entry in `contracts/look-tiers.md`.
- **Spawn:** the visual is spawned once per match in `AJamSkater::InitSkater`. A tier switch during a match would need A to respawn the visuals and repeat `SetTeam`, `SetNumber`, `SetHasPuck` and `SetBoost`, or the tier switch could be allowed only between matches.
- **Cook list:** the three Blueprints are referenced only by soft path, so A's cook list or an asset manager rule must include `/JamAssets/Skaters/` (it already does for T-B2 if the plugin content is cooked whole).
- **Rclay** needs no class: it would be `BP_SkaterVisual_TB2` with `MI_SkaterTB2_Clay` on slot 0. If the lead prefers Rclay for P3, I can make one more duplicate, `BP_SkaterVisual_TB2_Clay`, with only that slot override.

## What I could not verify

- **The real game:** with these binaries the game builds no rink in PIE, so everything was checked on the stand-in set. There is no bot test, no state-hash check with the new visuals, and no `jam.Cut` run.
- **Frame cost:** not measured, because two editors were up.
- **Cooked sizes:** no cook was run, so the cooked sizes and the effect of RDO Medium (which needs `bFinalUsesRDO`, see A-to-B-0004) are unknown.
- **Influence count:** whether more than 4 influences per vertex survive the import. I saw no skinning artefact in the stills.
- **Takes not wired** (they exist as assets but are not used): Celebrate, Stop, Shot_Wrist, and all 15 new takes (Aims, Faceoff_*, Goalie_*, Pass, Poke, Lift, Shot_Charge, Stumble, Foul_Reaction). Goalie_Hold was only played as a slot montage.
- **Motion faults from SKH that I did not measure in Unreal:** P4B's skates point up when lying face-down (visible in `ski-fall-down`). P3A's left-mitten grip gap and the stick near its face; in the slap still the mitten is off the shaft.
- **Lean and crossovers** on the new figures: not looked at separately; they share the TB2 blend layout.
- **`M_SkaterGen` at Clay 0:** no B9-style proof was taken. It equals the master by construction while `UVNormalStrength` is 0.
- **Hyper3D export rights** for the Rodin meshes: still open from S3.

## What the lead must decide

1. **Size:** 1K texture sources and/or dropping the 15 unused takes to get each figure under 12 MB on disk. Or accept on-disk 14.4 / 12.8 MB, since the cooked size will be smaller.
2. **Child classes or duplicates:** whether to change `BP_SkaterVisual_TB2` once the other lane lands (an anim interface plus montage variables), so that the P3/P4 Blueprints become children and inherit the numbers and crests. Until then the re-point list above must be repeated on a fresh duplicate after each TB2 change.
3. **The per-tier class proposal** for device A (table above), and the tier name for P4.
4. **P4 instance:** raise `JerseyLum` (about 0.25) for a shaded broadcast jersey. Also decide whether gloves, pants and socks should stay in the team colour on P4B; the mesh made them jersey-blue, so the hue key cannot tell them apart without a mask.
5. **P3A's two-sided override:** keep it or drop it (no hole was visible).
6. **`M_SkaterGen` is a fork** of `M_SkaterTB2` at 15:33. Any later master change (the other lane's numbers and crests, if they live in `M_SkaterTB2`) does not reach it. Alternatively, `NormalTex` and `UVNormalStrength` could go into `M_SkaterTB2` once the other lane is done, and `MI_SkaterTB2_Clay` could be re-parented there.

## Ledger rows ready to paste (format as in `ASSETS.md`: path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note)

`size_mb` is the `.uasset` size on disk. `shipped = n` until a tier ships these.

```
| Game/Plugins/JamAssets/Content/Skaters/SK_SkaterTB2_P4.uasset | mesh | 4.21 | ai-generated | Hyper3D Rodin + Meshy rig | | | 2026-11-12 | B | n | skh P4B export (loopcut FBX), Rodin task 2a6834a4-36f5-4011-8f6e-073f4ffe724a, Meshy rig 01a1d015-47f4-70f0-8dbb-440d3b289555; root, sockets, foot pivot, boot weights by skh/retarget.py; includes the S4 stick (original); sockets stick/head/puck copied from SK_SkaterTB2 (lane SKI) |
| Game/Plugins/JamAssets/Content/Skaters/SKEL_SkaterTB2_P4.uasset | other | 0.01 | ai-generated | Hyper3D Rodin + Meshy rig | | | 2026-11-12 | B | n | own skeleton of the P4B import, 32 bones; Rodin task 2a6834a4-36f5-4011-8f6e-073f4ffe724a, Meshy rig 01a1d015-47f4-70f0-8dbb-440d3b289555 |
| Game/Plugins/JamAssets/Content/Skaters/T_SkaterTB2_P4_BaseColor.uasset | texture | 3.03 | ai-generated | Hyper3D Rodin + Meshy rig | | | 2026-11-12 | B | n | P4B base colour (2048 source), MaxTextureSize 1024, BC1 no alpha, Lossy Medium; Rodin task 2a6834a4-36f5-4011-8f6e-073f4ffe724a, Meshy rig 01a1d015-47f4-70f0-8dbb-440d3b289555 |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Aim_FarReach.uasset | anim | 0.18 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Aim_Front.uasset | anim | 0.18 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Aim_Left.uasset | anim | 0.18 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Aim_Right.uasset | anim | 0.18 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Celebrate.uasset | anim | 0.28 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Check.uasset | anim | 0.20 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Crossover_Left.uasset | anim | 0.19 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Crossover_Right.uasset | anim | 0.20 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Faceoff_Draw.uasset | anim | 0.17 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Faceoff_Ready.uasset | anim | 0.18 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Fall.uasset | anim | 0.21 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Fall_Forward.uasset | anim | 0.21 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Foul_Reaction.uasset | anim | 0.24 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_GetUp.uasset | anim | 0.30 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_GetUp_Front.uasset | anim | 0.30 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Glide.uasset | anim | 0.33 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Goalie_Block_Left.uasset | anim | 0.18 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Goalie_Block_Right.uasset | anim | 0.18 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Goalie_Hold.uasset | anim | 0.18 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Idle_Faceoff.uasset | anim | 0.32 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Lean_Left.uasset | anim | 0.17 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Lean_Right.uasset | anim | 0.18 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Lift.uasset | anim | 0.17 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Pass.uasset | anim | 0.17 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Poke.uasset | anim | 0.18 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Shot_Charge.uasset | anim | 0.18 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Shot_Slap.uasset | anim | 0.22 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Shot_Wrist.uasset | anim | 0.18 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Stop.uasset | anim | 0.21 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Stride.uasset | anim | 0.23 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Stumble.uasset | anim | 0.18 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P4B by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/BS_SkaterTB2_P4_Skate.uasset | anim | 0.03 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | n | 2D blend space Speed x TurnRate, sample layout and smoothing copied from BS_SkaterTB2_Skate (lane SKI) |
| Game/Plugins/JamAssets/Content/Skaters/AM_SkaterTB2_P4_Shot_Slap.uasset | anim | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | n | montage for PlayShoot |
| Game/Plugins/JamAssets/Content/Skaters/AM_SkaterTB2_P4_Check.uasset | anim | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | n | montage for PlayCheck |
| Game/Plugins/JamAssets/Content/Skaters/ABP_SkaterTB2_P4.uasset | other | 0.34 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | n | duplicate of ABP_SkaterTB2 re-pointed to SKEL_SkaterTB2_P4 and the P4 clips (lane SKI) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB2_P4.uasset | other | 0.29 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | n | duplicate of BP_SkaterVisual_TB2 re-pointed to the P4 mesh, AnimBP and montages; implements IJamSkaterVisual (lane SKI) |
| Game/Plugins/JamAssets/Content/Skaters/SK_SkaterTB2_P3.uasset | mesh | 3.55 | ai-generated | Hyper3D Rodin + Meshy rig | | | 2026-11-12 | B | n | skh P3A export (loopcut FBX), Rodin task 2dd1928f-a27f-4a1d-bd25-353d12fba0dd, Meshy rig 01a1d015-4939-7601-88af-f12ffd035312; root, sockets, foot pivot, boot weights by skh/retarget.py; includes the S4 stick (original); sockets stick/head/puck copied from SK_SkaterTB2 (lane SKI) |
| Game/Plugins/JamAssets/Content/Skaters/SKEL_SkaterTB2_P3.uasset | other | 0.01 | ai-generated | Hyper3D Rodin + Meshy rig | | | 2026-11-12 | B | n | own skeleton of the P3A import, 32 bones; Rodin task 2dd1928f-a27f-4a1d-bd25-353d12fba0dd, Meshy rig 01a1d015-4939-7601-88af-f12ffd035312 |
| Game/Plugins/JamAssets/Content/Skaters/T_SkaterTB2_P3_BaseColor.uasset | texture | 2.08 | ai-generated | Hyper3D Rodin + Meshy rig | | | 2026-11-12 | B | n | P3A base colour (2048 source), MaxTextureSize 1024, BC1 no alpha, Lossy Medium; Rodin task 2dd1928f-a27f-4a1d-bd25-353d12fba0dd, Meshy rig 01a1d015-4939-7601-88af-f12ffd035312 |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Aim_FarReach.uasset | anim | 0.18 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Aim_Front.uasset | anim | 0.18 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Aim_Left.uasset | anim | 0.19 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Aim_Right.uasset | anim | 0.18 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Celebrate.uasset | anim | 0.29 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Check.uasset | anim | 0.20 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Crossover_Left.uasset | anim | 0.19 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Crossover_Right.uasset | anim | 0.19 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Faceoff_Draw.uasset | anim | 0.17 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Faceoff_Ready.uasset | anim | 0.18 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Fall.uasset | anim | 0.21 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Fall_Forward.uasset | anim | 0.21 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Foul_Reaction.uasset | anim | 0.24 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_GetUp.uasset | anim | 0.31 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_GetUp_Front.uasset | anim | 0.30 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Glide.uasset | anim | 0.33 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Goalie_Block_Left.uasset | anim | 0.18 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Goalie_Block_Right.uasset | anim | 0.17 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Goalie_Hold.uasset | anim | 0.17 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Idle_Faceoff.uasset | anim | 0.32 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Lean_Left.uasset | anim | 0.17 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Lean_Right.uasset | anim | 0.18 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Lift.uasset | anim | 0.17 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Pass.uasset | anim | 0.17 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Poke.uasset | anim | 0.18 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Shot_Charge.uasset | anim | 0.18 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Shot_Slap.uasset | anim | 0.22 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Shot_Wrist.uasset | anim | 0.18 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Stop.uasset | anim | 0.21 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Stride.uasset | anim | 0.23 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Stumble.uasset | anim | 0.18 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | S4 build_clips.py take retargeted onto P3A by skh/retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/BS_SkaterTB2_P3_Skate.uasset | anim | 0.03 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | n | 2D blend space Speed x TurnRate, sample layout and smoothing copied from BS_SkaterTB2_Skate (lane SKI) |
| Game/Plugins/JamAssets/Content/Skaters/AM_SkaterTB2_P3_Shot_Slap.uasset | anim | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | n | montage for PlayShoot |
| Game/Plugins/JamAssets/Content/Skaters/AM_SkaterTB2_P3_Check.uasset | anim | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | n | montage for PlayCheck |
| Game/Plugins/JamAssets/Content/Skaters/ABP_SkaterTB2_P3.uasset | other | 0.34 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | n | duplicate of ABP_SkaterTB2 re-pointed to SKEL_SkaterTB2_P3 and the P3 clips (lane SKI) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB2_P3.uasset | other | 0.29 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | n | duplicate of BP_SkaterVisual_TB2 re-pointed to the P3 mesh, AnimBP and montages; implements IJamSkaterVisual (lane SKI) |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB2_P4.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | n | instance of M_SkaterTB2, BaseColorTex = T_SkaterTB2_P4_BaseColor, master hue-key defaults |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB2_P3.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | n | instance of M_SkaterTB2, BaseColorTex = T_SkaterTB2_P3_BaseColor, KeyGain 8, KeyBias 0.5, Roughness 0.9, two-sided override |
| Game/Plugins/JamAssets/Content/Skaters/M_SkaterGen.uasset | material | 0.05 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | n | duplicate of M_SkaterTB2 (state of 15:33) + NormalTex / UVNormalStrength (UV normal map blended onto the MPC_Look clay normal) |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB2_Clay.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | n | instance of M_SkaterGen for SK_SkaterTB2 (Rclay): T_SkaterTB2_Clay_BaseColor, T_SkaterTB2_Clay_Normal, UVNormalStrength 1, Roughness 0.9 |
| Game/Plugins/JamAssets/Content/Skaters/T_SkaterTB2_Clay_BaseColor.uasset | texture | 2.99 | ai-generated | Hyper3D Rodin + Meshy rig | | | 2026-11-12 | B | n | skh Rclay: S6 clean T-B2 texture with a clay treatment (skh/tex), 2048 source, MaxTextureSize 1024, BC1, Lossy Medium; Rodin task c07557c7-cf01-462a-97a3-9f0336c0ff31 |
| Game/Plugins/JamAssets/Content/Skaters/T_SkaterTB2_Clay_Normal.uasset | texture | 1.19 | original | Python script (skh/tex/clay_r.py) | | | 2026-11-12 | B | n | procedural thumb-dent normal on R's UVs, 1024, DirectX, BC5 (TC_Normalmap), Lossy Medium |
```
