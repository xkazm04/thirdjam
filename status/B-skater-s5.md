# Skater lab S5: hybrid (S4's scripted clips drive the generated mesh R), report at 12:05

Question: can S4's scripted clips drive the Rodin+Meshy mesh R, so the game gets R's face and soft shapes with S4's exact loops, zero drift and
real hockey motion? **Answer: yes.** All 15 S4 takes play on R:
- Loop seams are 0.000 cm and pelvis horizontal travel is 0.000 cm (measured on the re-imported FBX).
- The lower skate sits on the ice within 0.29 cm on every standing clip.
- R carries S4's stick in its right mitten, and the blade never goes below the ice.

The whole lane rebuilds in about 3 min with one command. It used no credits and no network, and nothing was imported into Unreal, so everything
Unreal-side is **unverified**.

Lab folder: `C:\Users\kazda\jam\skater-lab\s5\`:
- scripts: `retarget.py`, `verify.py`, `render_s5.py`, `render_all.sh`, `run_all.sh`, plus the probes `probe.py` and `probe_weights.py`
- inputs: snapshots in `in\`
- outputs: `export\`, `s5_skater.blend`, the sheets in `sheets\`, the frames in `r\`
- measurements: `retarget_report.json`, `verify.json`, `verify_loopcut.json`, `inspect.jsonl`

**Inputs (read-only, copied into `in\`):**
- **S4:** `SK_Skater_S4.fbx` is S4's **11:32:34 polish export** (sha256 `479986DA...B4DA`, 15 takes). I took a first snapshot at 11:26: the 11:22:06
  export (`SK_Skater_S4_1122.fbx`, sha256 `81D80A20...F182`, 10 takes). I switched to the 11:32 file when S4 re-exported, because the script is
  clip-agnostic. If S4 exports again before 12:10, copy the new file into `in\` and rerun; nothing else changes.
- **R:** `R_rigged.fbx` = `s3\RIG2\rigged_character_fbx_url.fbx` (Meshy rig task `01a1cf39-74e0-7496-9b7c-cd8a09ffe4d3`).

## Steps: PASS/FAIL with evidence

| Step | Result | Evidence |
|---|---|---|
| 1. `retarget.py`: S4 takes drive R, pelvis rule, bake at 60 fps, foot pass | **PASS** | All 15 takes. Direct matrix maths, not constraints, so there is nothing to remove: each mapped bone gets the world-space rotation change from rest of its S4 partner, and positions come from R's own hierarchy. Pelvis: R's rest x,y (no travel) and z = R rest + K x (S4 pelvis z - S4 rest), with **K = 0.586** (hip-to-ankle, R over S4). Every frame is keyed (linear, 60 fps). Foot pass: the pelvis moves vertically so that R's lowest body vertex sits at K x S4's lowest. The vertical move is rate-limited to 2 cm per frame as an upper envelope, which never pushes the figure into the ice. **Lower-skate error on frames where S4's lower skate is on the ice: <= 0.29 cm on every standing clip.** Fall 9.5 cm, Fall_Forward 10.3 cm and GetUp 6.0 cm: R's big helmet and body touch first when lying, so its skates stay up. Table below. |
| 2. `root`, scale, stick, sockets | **PASS** | `root` at the origin, parent of `pelvis` (and of `puck_socket`). R is turned to face +X like S4, and the Meshy 90 deg X on the armature object is applied: re-import shows armature rotation 0, **scale 1.0**, top bone `root`. `stick_socket` sits under `hand_r` at the right mitten's centre, with S4's socket rest rotation (0.000 deg difference); S4's stick mesh is skinned to it at weight 1. `head_socket` sits under `head`, 13 cm above the helmet top (as in S4). `puck_socket` sits under `root` at S4's spot (78 cm ahead, 10 cm left, z 0). R's mesh vertex positions, UVs, material and textures are untouched. **One change to R's skin weights:** Meshy had weighted the whole skate boot to `calf` (2,630 verts, z 0 to 0.47 m); `foot` held only a 144-vertex toe. With no working ankle, S4's knee bend would tip every boot onto its toe. Below the sock band (z 0.26 to 0.34 m, smoothstep) the boot is moved to `foot`, the `ball`/`toe` weights are merged into `foot`, and `foot`'s pivot moves to the boot top (z 0.30). |
| 3. Export `SK_Skater_S5.fbx` and prove it | **PASS** (Blender side) | `export\SK_Skater_S5.fbx`, 7.5 MB (textures embedded), plus `SK_Skater_S5_loopcut.fbx` (loops without the repeated last frame, as S4 ships). Re-import with `inspect.py`: 32 bones, **19,944 tris** (body 19,748 + stick 196), 15 actions, `move_root` [0,0,0], `move_pelvis` x,y = 0 on every action. `verify.py` re-import: pelvis horizontal **max over every frame** 0.0000 cm and root 0.0000 cm on all 15 takes. Loop seams (max joint, first to last) 0.0000 cm on all 5 loops; the loopcut wrap is one frame step (Stride 0.44 cm, crossovers 1.09 cm, Glide and Idle 0.005 cm). Lowest body point >= 0.00 cm on every take except Fall_Forward (-0.67 cm, which is K x S4's own -1.14 cm dip). Lowest stick point >= 0.00 cm on every take. Every sheet was rendered from the re-imported FBX. Looked at and iterated: 4 passes (below). |
| 4. Side-by-side comparison | **PASS** | `lab-s5-compare`: same side camera; rows are Stride S4 / S5 / R+S2, then Shot_Slap S4 / S5 / R+S2. |

## Bone map (S4 24 bones -> S5 32 bones)

| S4 bone | S5 bone | Treatment |
|---|---|---|
| `root` | `root` (new, at the origin) | identity keys |
| `pelvis` | `pelvis` (now a child of `root`) | rotation delta; z = K x S4 dz + foot pass; x,y locked |
| `spine_01`, `spine_02`, `spine_03`, `neck_01`, `head` | same names | rotation delta |
| `clavicle_l/r`, `upperarm_l/r`, `lowerarm_l/r`, `hand_l/r` | same names | rotation delta; then the head clamp (both upper arms) and the left-arm IK onto the shaft |
| `thigh_l/r`, `calf_l/r`, `foot_l/r` | same names (`foot` pivot moved to the ankle, boot re-weighted) | rotation delta |
| `stick_socket` (under `hand_r`) | `stick_socket` (new, under `hand_r`, in the mitten) | rotation delta (so the stick has S4's exact world orientation); then the stick-helmet tilt and the slide along the shaft |
| `head_socket` (under `head`) | `head_socket` (new) | follows `head` |
| `puck_socket` (under `root`) | `puck_socket` (new) | static |

R bones with no S4 partner: `head_end`, `headfront` (leaves under `head`); `LeftHand_End`, `RightHand_End` (leaves under the hands); `ball_l/r`,
`LeftToe_end`, `RightToe_end` (under `foot`). All keep identity keys and follow their parent. Their weights: `headfront` (57 verts) follows the
head; the toe bones' weights were merged into `foot`. R has **no twist or extra spine bones** (the same 5-bone spine and neck chain as S4), so
nothing on S4's side is left unmapped.

## Per take (from `retarget_report.json`; seam, travel, root and lowest points from the re-imported FBX)

How to read the columns:
- *Foot pass dz*: the vertical correction applied on top of the K-scaled pelvis. *step* = the largest change per frame after the rate limit (raw =
  before it).
- *Lower-skate err*: R's lower skate height on the frames where S4's lower skate is on the ice.
- *Blade min*: the lowest stick point. *contact err*: R's blade height minus S4's, on frames where S4's blade is within 3 cm of the ice.
- *Slide*: how far the stick slides through the mitten (max 25 cm).
- *Grip*: frames with the left hand on the shaft (S4's glove within 5 to 12 cm of its shaft), and the largest left-mitten gap left after IK.
- *Clamp*, *tilt*: frames and largest angle of the head clamp and of the stick tilt.

| Take | Frames | Loop seam | Pelvis horiz. | Foot pass dz (step) | Lower-skate err | Blade min / contact err | Slide | Grip frames, gap | Clamp | Tilt |
|---|---|---|---|---|---|---|---|---|---|---|
| Idle_Faceoff | 121 | 0.000 | 0.000 | -2.1 cm (0.0) | 0.03 cm | 0.64 / 0.0 cm | -1 cm | 121, 3.4 cm | 0 | 0 |
| Stride | 61 | 0.000 | 0.000 | -2.9 to 3.5 cm (0.7) | 0.16 cm | 0.0 / 0.4 cm | 5 to 25 cm | 0 (free arm) | 0 | 0 |
| Glide | 121 | 0.000 | 0.000 | 0.0 to 0.1 cm | 0.00 cm | 1.6 / 0.0 cm | 0 | 121, 9.3 cm | 0 | 0 |
| Stop | 37 | one-shot | 0.000 | 0.1 to 10.0 cm (1.0) | 0.26 cm | 1.7 / 0.0 cm | -6 to 18 cm | 37, 19.4 cm | 0 | 0 |
| Shot_Slap | 43 | one-shot | 0.000 | -1.5 to -0.9 cm (0.3) | 0.00 cm | 0.6 / 0.0 cm | -7 to 3 cm | 29, 12.5 cm | 1 frame, 2.5 deg | 4 frames, 6.4 deg |
| Shot_Wrist | 22 | one-shot | 0.000 | -0.7 to -0.3 cm | 0.00 cm | 0.5 / 0.0 cm | -5 to 4 cm | 16, 6.3 cm | 0 | 0 |
| Check | 31 | one-shot | 0.000 | -2.0 to 2.9 cm (1.5) | 0.29 cm | 0.0 / 0.1 cm | -1 to 17 cm | 3, 8.9 cm | 0 | 0 |
| Fall | 37 | one-shot | 0.000 | -17.7 to 0.1 cm (2.0, raw 4.4) | **9.5 cm** (lying) | 1.7 / 0.0 cm | 0 | 1 | 0 | 0 |
| GetUp | 73 | one-shot | 0.000 | -17.5 to 3.7 cm (2.0, raw 4.3) | **6.0 cm** | 0.8 / 8.4 cm | 0 to 25 cm | 15, **66 cm** | 2 frames, 15 deg | 0 |
| Celebrate | 73 | one-shot | 0.000 | -0.7 to 0.1 cm | 0.00 cm | 1.7 / 0.0 cm | 0 | 73, 8.9 cm | 0 | 63 frames, 16.4 deg |
| Fall_Forward | 37 | one-shot | 0.000 | -19.2 to 0.1 cm (2.0, raw **11.4**) | **10.3 cm** (lying) | 1.7 / 0.0 cm | 0 to 3 cm | 1 | 0 | 0 |
| Crossover_Left | 41 | 0.000 | 0.000 | 5.2 to 13.1 cm (1.5) | 0.28 cm | 8.0 / n/a | 19 to 25 cm | 0 | 0 | 0 |
| Crossover_Right | 41 | 0.000 | 0.000 | 5.5 to 13.2 cm (1.5) | 0.28 cm | 0.3 / **9.0 cm** | 25 cm (at max) | 24, **40 cm** | 0 | 0 |
| Lean_Left (hold) | 2 | n/a | 0.000 | 9.1 cm | n/a (S4 floats 1.2 cm; S5 0.7 cm) | 10.8 cm | 25 cm | 0 | 0 | 0 |
| Lean_Right (hold) | 2 | n/a | 0.000 | 9.4 cm | n/a (0.7 cm) | 6.1 / 4.9 cm | 25 cm | 2, 20 cm | 0 | 0 |

## Iterations (I looked at every sheet; each fix is a pose-independent rule)

1. **First run**: the stride, glide, shots and falls already read well. The knees bend the right way and the re-weighted boots stay flat on the
   ice. Found three problems:
   - The mitten went into the face in the slap wind-up.
   - In Celebrate (S4's 11:22 version) the stick lay across the face.
   - The R+S2 rows faced away from the camera: S2's clips key the **armature object's** transform. The renderer now yaws through a parent
     empty.
2. **Clamps**: two rules added.
   - Head clamp: a mitten centre must stay >= 0.40 m from the helmet centre (helmet half-size about 0.28 m plus the mitten radius about 0.12 m). If
     not, the upper arm swings about the shoulder by the smallest rotation that restores it.
   - Stick tilt: the stick tilts about the grip until the shaft clears the helmet sphere.
3. **Foot-pass pop**: Fall_Forward's correction jumped 11.4 cm in one frame, when the helmet became the lowest point. Fixed with the 2 cm/frame
   upper-envelope rate limit. It raises earlier frames by up to 9.4 cm and never lowers any frame, so it adds no penetration.
4. **Chin**: the helmet is not a sphere, and the chin juts to about 0.33 m from its centre. The stick clearance went from 0.31 to 0.36 m. In
   Celebrate the shaft now rises clear of the helmet in the side view; from the front it still passes in front of the mouth (occlusion, not
   penetration).

## Rubric rows (judged harshly, next to S4's and R's)

| Candidate | Look fit | Technical | Motion | Cost | Provenance | Import risk |
|---|---|---|---|---|---|---|
| S4 scripted toy (S4's own row) | 3 | 5 | 4 | 5 | 5 | 3 |
| R with S2 text-to-motion clips (S3 mesh, S2 clips) | 4: R's look (S3) | 3: 19.7k tris, 4 x 2K maps, 28 bones, no root | **2**: open Stride seam 11 deg (0 only with a repeated frame); no stick, so the shots read as hand waving; in `lab-s5-compare` the stride is an upright walk-run with no crouch or side push; pelvis yaw drift kept; the clips also key the armature object transform | 4: 5.5 credits mesh+rig plus 27 for the clips | 3: mesh and clips `ai-generated`, library clips `sourced` | 3 |
| **S5 hybrid** (`SK_Skater_S5`) | **4**: R's face, soft chunky shapes and big helmet, plus a real stick. The stride, slap and fall read at 40 px. Minus: the deep navy is darker against the ice than S4's white helmet. Celebrate reads as "stick held in front of the face", not a raise. A hood-like shoulder fold appears when an arm swings far back or overhead (Meshy weights; visible at 160 px and up, not at 40 px). | **3**: 19,944 tris (cap 30k). 32 bones with mannequin names and a real `root`, armature scale 1, A-pose rest, limbs separate. But 4 x 2K maps are embedded (base colour, normal, metallic, roughness: over the one-set rule unless three are dropped). Meshy's odd bone axes are kept. The boot weights are mine, not Meshy's. | **4**: S4's motion exactly. Loop seams 0.000 cm; pelvis travel 0.000 cm. Lower skate on the ice within 0.29 cm on standing clips. The stick blade reaches the ice in Stride and the slap (contact error 0.4 cm), and the slap wind-up reads from the side. Minus: the left mitten misses the shaft by up to 12.5 cm in the slap, 19 cm in Stop, 40 cm in Crossover_Right and 66 cm in GetUp (R's arms are 37 cm long against S4's 55 cm). Lying poses leave the skates 6 to 10 cm up. Same pose-to-pose toy timing as S4. | **5**: 0 new credits (R's 5.5 already spent); 17 s retarget, about 3 min full rebuild with sheets, about 30 min of authoring | **3**: mesh `ai-generated` (Hyper3D Rodin), rig and weights `ai-generated` (Meshy, edited by our script at the ankle), clips and stick `original`. Counts for the Hyper3D pool; only the motion and stick are scored as ours | **3**: unverified in UE: the `Armature` node, the +X front, take names, 2 skinned mesh nodes, 3 material slots, embedded textures, sockets as bones. S2's R clips will **not** play on this skeleton without a retarget (new `root`, turned rest, moved `foot` pivots) |

## Contact sheets (filed with `devlog/shot.mjs`, all from the re-imported `SK_Skater_S5.fbx`)

- `devlog/shots/20261112-115433-B-lab-s5-turntable.png`: 8-view turntable, rest with the stick (top) and Glide (bottom)
- `devlog/shots/20261112-115433-B-lab-s5-skating.png`: Stride 3/4 front, Stride side, Stride from behind, Glide, Idle_Faceoff
- `devlog/shots/20261112-115434-B-lab-s5-shots.png`: Shot_Slap side, Shot_Slap 3/4, Shot_Wrist side, Check, Stop
- `devlog/shots/20261112-115434-B-lab-s5-fall.png`: Fall side, GetUp side, Celebrate 3/4, front and side
- `devlog/shots/20261112-115434-B-lab-s5-extra.png`: Crossover_Left from behind, Crossover_Right 3/4, Fall_Forward side
- `devlog/shots/20261112-115434-B-lab-s5-distance.png`: about 40 px tall from the chase angle (2x nearest): Stride, Shot_Slap, Glide, Fall
- `devlog/shots/20261112-115434-B-lab-s5-compare.png`: same side camera; Stride S4 / S5 / R+S2, then Shot_Slap S4 / S5 / R+S2
- `devlog/shots/20261112-115435-B-lab-s5-closeups.png`: weights close up: Stride frames 1, 24, 31 (shoulder fold), Slap frames 14 and 26 from
  the right, Celebrate front

Renders use Workbench with texture colour, studio light and a black outline: a stand-in for the P2 cel look, not the game's shader.

## Rebuild (one command, about 3 min)

```
bash C:/Users/kazda/jam/skater-lab/s5/run_all.sh
```

It runs, in order: `retarget.py` (17 s, writes both FBX files, `s5_skater.blend` and `retarget_report.json`), then `verify.py` on both FBX files,
then `tools/meshy/inspect.py` (to `inspect.jsonl`), then `render_all.sh` (the sheets). The retarget step alone:
`"C:/Program Files/Blender Foundation/Blender 4.2/blender.exe" -b --factory-startup --python C:/Users/kazda/jam/skater-lab/s5/retarget.py -- C:/Users/kazda/jam/skater-lab/s5`.
Tunables sit at the top of `retarget.py`: `HEAD_CLEAR` 0.40, `STICK_HEAD_CLEAR` 0.36, `STICK_SLIDE` 0.25, `DZ_RATE` 0.02, `ANKLE_Z` 0.30.

## Export settings and what Unreal should do with them (unverified until imported)

- **Settings** (S4's, plus textures): `axis_forward=-Z`, `axis_up=Y`, `apply_unit_scale=True`, `apply_scale_options=FBX_SCALE_ALL`,
  `global_scale=1.0`, `add_leaf_bones=False`, `primary_bone_axis=Y`, `secondary_bone_axis=X`, `armature_nodetype=NULL`,
  `use_armature_deform_only=False`, `mesh_smooth_type=FACE`, `bake_anim_use_all_actions=True`, `bake_anim_use_nla_strips=False`,
  `bake_anim_force_startend_keying=True`, step 1, simplify 0, 60 fps. Also `path_mode=COPY` and `embed_textures=True`, with objects `Armature`,
  `SK_Skater_S5` and `SK_Skater_S5_Stick` only.
- **Expected import** (UE 5.9, default skeletal options, "Force Front XAxis" off): the figure stands about 170 units tall, Z up, and **faces +X**
  like S4 (do not add the usual -90 yaw). There is no 100x scale, since the re-import gives armature scale 1.0. `root` should be the skeleton root
  if UE drops the `Armature` null, as with S4; if not, there is one extra `Armature` bone above `root`.
- **Mesh and textures**: one skeletal mesh with 3 material slots (`Material_0` for R, `Trim` and `Stick` for the stick). UE should create 4
  textures from the embedded maps. The cel material should use base colour only; the other three can be dropped.
- **Takes**: 15 anim sequences, probably named `SK_Skater_S5_Anim_Armature_<Clip>`. Root motion is identity on `root`. Use `_loopcut` for
  wrapping loops.
- **Sockets**: `stick_socket`, `puck_socket` and `head_socket` are bones, which `GetSocketLocation` accepts by name.

## Provenance and ledger rows for the lead to paste

| File | Origin | Tool | Licence | Note |
|---|---|---|---|---|
| `SK_Skater_S5.fbx` / `_loopcut.fbx`: body mesh, UVs, textures | ai-generated | Hyper3D Rodin Gen-2.5-Low, task `c07557c7-cf01-462a-97a3-9f0336c0ff31` (the no-TAPose skater) | Rodin terms (S3: plan-dependent export rights, to confirm) | untouched geometry and UVs |
| same: skeleton and skin weights | ai-generated, modified | Meshy rig `01a1cf39-74e0-7496-9b7c-cd8a09ffe4d3`; `root`, sockets, `foot` pivot and boot weights by `skater-lab/s5/retarget.py` | Meshy terms | |
| same: 15 takes | original | S4's `build_clips.py` (Blender Python), retargeted by `retarget.py` | ours | |
| same: stick mesh | original | S4's `build_skater.py` | ours | |

## Verdict

- **Better than R with generated clips: clearly yes.** Same mesh, with exact loops instead of an 11 deg seam, a real stick, a skating crouch and
  side push instead of a walk-run, and 15 clips instead of 10. Every S4 clip that S4 adds comes along for free on rerun.
- **Better than S4 alone: on looks, yes; overall it is a trade.** It has the same motion with a much better figure (face, soft shapes, a
  character rather than programmer art). It loses S4's clean provenance (mesh `ai-generated`), its 5.3k-tri, texture-free, rigid-weight
  simplicity, and some arm quality, because R's short arms cannot match S4's two-hand poses.
- **Which I would ship: S5, as "queue for import".** Keep **S4 as the fallback** and the "all ours" option; **drop R with S2 clips** for the
  game.

## What is weak

- **Short arms:** R's arms are 37 cm against S4's 55 cm, so S4's two-handed poses cannot be fully matched. The left mitten misses the shaft by
  12.5 cm in the slap, 19 cm in Stop, 40 cm in Crossover_Right and 66 cm in GetUp. Celebrate cannot lift the mitts above the 55 cm helmet; it
  reads as "stick in front of the face". A real fix is pose work: S4's pose tables re-solved with R's arm lengths (S4's IK could take a
  proportions table).
- **Shoulder fold:** Meshy's shoulder weights fold into a hood-like bulge when an arm swings far back (Stride frames 24 to 31) or overhead (Slap
  follow-through). Visible at 160 px and up, not at 40 px. A clavicle-share rule would only move the fold.
- **Lying poses:** R's helmet and body touch the ice first, so the skates float 6 to 10 cm in Fall, Fall_Forward and GetUp.
- **Stick slide:** the stick reaches the ice by sliding up to 25 cm through the mitten. The butt then sticks out about 30 cm above the mitten in
  Stride and the crossovers. Crossover_Right's blade still floats 9 cm where S4's touches.
- **Darker at distance:** the deep navy jersey and helmet are darker against the ice at 40 px than S4's white helmet.
- **Textures:** four 2K maps where the budget wants one set.

## What I could not verify

- Anything in Unreal: the import, the `Armature` null, the +X front, scale, take names, slot order, embedded-texture handling, sockets, and the
  cel shader on R's normals.
- How UE treats the re-weighted ankle. I did not count influences per vertex: the blend adds `foot` to vertices that may already carry 4
  Meshy influences, which UE could trim on import.
- Whether S4 exports again after 11:32: S4 said it may add clips until about 12:10. Rerun `run_all.sh` after copying its newest export into
  `in\`.
- Licence: the Rodin export rights (S3's open question) still apply to this file.
- **Housekeeping note:** my first test render used a relative path and wrote `C:\r\rest\f00-f03.png`. That folder already existed from another
  lane at 11:06, and I overwrote its first four frames. Deletion of `C:\r` is blocked, so I left it.

## For the lead or the operator to decide

1. Queue `SK_Skater_S5_loopcut.fbx` for B5's import (about 20 min), with S4 as the fallback.
2. Keep R's normal, metallic and roughness maps, or ship base colour only (the one-set rule).
3. Ask S4 for an R-proportioned pose pass (arm length 0.37 m, hip-to-ankle 0.42 m) for Celebrate, the two-hand grips and GetUp, or accept the
   gaps listed above.
4. Confirm Hyper3D export rights before the mesh ships (open from S3).

## Rerun on the fixed S4 export (lead's request, 12:13 to 12:23)

**Result: done.** Both files carry all 16 takes, are written in centimetres, have `root` with an identity rest at scale 1, carry no tracks on `root` or `Armature`, and embed no textures. Everything below is measured on the files as written and re-imported in Blender; **Unreal is still the real test**.

**Input:** S4's `export/SK_Skater_S4.fbx` written 12:09:54 (sha256 `AC97F221...961E`, cm, 16 takes), copied to `s5/in/` together with `SK_Skater_S4_loopcut.fbx`, `strip_tracks.py`, `fbx_nodes.py` and `fbx_tracks.py`. The 11:32 snapshot is kept as `in/SK_Skater_S4_1132.fbx`.

### Files

| File | Bytes |
|---|---|
| `C:\Users\kazda\jam\skater-lab\s5\export\SK_Skater_S5.fbx` | 5,351,644 |
| `C:\Users\kazda\jam\skater-lab\s5\export\SK_Skater_S5_loopcut.fbx` | 5,337,228 |

Both are written with no embedded images.

### What changed in `retarget.py`

- **S4 input:** the cm S4 file re-imports with an armature object scale of 0.01. Every S4 matrix and vertex is now taken in world space (metres, rotation normalised). The retarget maths and all tunables are unchanged.
- **`root`:** head at the origin, tail 15 cm along +Y, roll 0. Its rest `matrix_local` is the identity.
- **Centimetres (S4's recipe):** after baking, the bones, body mesh and stick mesh are scaled by 100, every location key is scaled by 100, and the scene unit is set to 0.01 m.
- **Export settings:** `apply_unit_scale=True`, `apply_scale_options=FBX_SCALE_NONE`, `global_scale=1.0`, `bake_anim_use_all_bones=False`, `path_mode=AUTO`, `embed_textures=False`. The file is written from the rest pose. Axes, bone axes, leaf bones, smoothing, take, step and simplify settings are as before.
- **Track strip:** S4's `strip_tracks.py` runs on both files (`run_all.sh`) and removes 96 curve nodes and 288 curves per file, for `root` and `Armature`.
- **Material slots:** the three slots do **not** share one base-colour map.
  - Body: one slot, `Material_0`, which now holds only a base-colour image node. That node references `C:/Users/kazda/jam/skater-lab/s6/T_SkaterTB2_BaseColor_clean.png` by path, not embedded. The normal, metallic and roughness references are removed.
  - Stick: two slots, `Trim` (white tape) and `Stick` (wood), flat colours with no texture, inherited from S4's stick.
  - I checked the cleaned PNG against R's original base colour: same 2048 atlas layout, so it fits R's UVs.
- **Unchanged:** bone names, sockets as bones, the stick in the file.

### Proof from the file itself (`s5/fbx_nodes.txt`)

- `UnitScaleFactor` is 1.0 (centimetres).
- The `Armature` null has translation 0, rotation (-90, 0, 0) and scaling 1. This is the axis turn, as in S4.
- `root` has translation 0, rotation **(0, 0, 0)** and scaling **1**.
- `pelvis` sits at (-4.36, -0.37, 76.05) cm.
- Over 16 stacks, every bone except `root` has 48 curve nodes; `root` and `Armature` have **none**.

### Proof on re-import (`verify.json`, `verify_loopcut.json`, `inspect.jsonl`)

- Bones: 32, with `root` the only top bone.
- **`root` rest `matrix_local` is the identity.** The armature object scale is 0.01: Blender turns the cm file back into metres, exactly as with S4's file.
- **Rest height 170.0 cm**, lowest rest point 0.0 cm.
- Tris: 19,748 body + 196 stick.
- 60 fps, 16 takes.
- No take keys `root` or the armature object.
- Root motion is 0.0000 cm on every take.

### Takes (main / loopcut frames; numbers from the re-imported main file)

| Take | Frames | Loop seam main / loopcut (cm) | Pelvis horizontal max | Lowest skate (cm) | Lowest body (cm) | Lowest stick (cm) |
|---|---|---|---|---|---|---|
| Idle_Faceoff | 121 / 120 | 0.000 / 0.005 | 0.000 | 0.02 | 0.02 | 0.64 |
| Stride | 61 / 60 | 0.000 / 0.442 | 0.000 | 0.01 | 0.01 | 0.00 |
| Glide | 121 / 120 | 0.000 / 0.005 | 0.000 | 0.00 | 0.00 | 1.63 |
| Stop | 37 | one-shot | 0.000 | 0.00 | 0.00 | 1.68 |
| Shot_Slap | 43 | one-shot | 0.000 | 0.00 | 0.00 | 0.63 |
| Shot_Wrist | 22 | one-shot | 0.000 | 0.00 | 0.00 | 0.48 |
| Check | 31 | one-shot | 0.000 | 0.00 | 0.00 | 0.00 |
| Fall | 37 | one-shot | 0.000 | 0.00 | 0.00 | 1.68 |
| GetUp | 73 | one-shot | 0.000 | 0.00 | 0.00 | 0.81 |
| Celebrate | 73 | one-shot | 0.000 | 0.00 | 0.00 | 1.68 |
| Fall_Forward | 37 | one-shot | 0.000 | 0.00 | -0.67 | 1.68 |
| **GetUp_Front** (new) | 73 | one-shot | 0.000 | 0.00 | -0.67 | 0.12 |
| Crossover_Left | 41 / 40 | 0.000 / 1.085 | 0.000 | 0.13 | 0.13 | 8.00 |
| Crossover_Right | 41 / 40 | 0.000 / 1.089 | 0.000 | 0.13 | 0.13 | 0.30 |
| **Lean_Left** (now a 30-frame loop) | 31 / 30 | 0.000 / 0.005 | 0.000 | 0.70 | 0.70 | 10.70 |
| **Lean_Right** (now a 30-frame loop) | 31 / 30 | 0.000 / 0.005 | 0.000 | 0.70 | 0.70 | 6.02 |

Notes on the table:
- "Lowest skate" is the minimum over the take of the lowest boot vertex. A loopcut seam above 0 is one frame of motion, not a jump.
- The lowest body points of -0.67 cm in Fall_Forward and GetUp_Front are K × S4's own -1.14 cm, which is the chest at the end of Fall_Forward.
- In the lying poses the skates sit well above the ice where S4's touch it (the lower skate's height on frames where S4's lower skate touches; not in the table above, which shows the lowest boot vertex over the whole take):
  - Fall 9.5 cm
  - Fall_Forward 10.3 cm
  - GetUp 6.0 cm
  - GetUp_Front 6.5 cm
- On every standing take the lower skate stays within 0.29 cm of the ice, as before.

### Sheets (from the re-imported file, textured with the cleaned PNG)

- `devlog/shots/20261112-122329-B-lab-s5b-getup-front.png`: Fall_Forward side, GetUp_Front side, GetUp_Front 3/4. It reads as a push-up on the hands, a kneel and a rise into the glide crouch, with the stick flat under the right mitten.
- `devlog/shots/20261112-122329-B-lab-s5b-leans.png`: Lean_Left and Lean_Right from behind and from the front. They mirror, with the stick trailing on the outside.
- The other sheets in `s5/sheets/` were re-rendered from the new file and look as before. They were not re-filed.

### What got worse or is new

- **The leans** float 0.7 cm, which is K × S4's own 1.2 cm float. The stick slide is at its 25 cm maximum and the blade stays 6 to 11 cm above the ice: R's arm cannot reach S4's lean stick position. In Lean_Right the left mitten misses the shaft by 20 cm.
- **GetUp_Front** inherits R's lying-pose problems: skates up to 6.5 cm off the ice while S4's touch, and a left-mitten gap of up to 11.5 cm. Its stick is never below the ice (0.12 cm).
- **The material** now references a texture by absolute path on this machine. If Unreal finds it, it may build a texture from it; if not, it warns. The importer is assigning the PNG anyway.
- **The strip step is new:** the files are rewritten by Blender's FBX writer through S4's `strip_tracks.py`. Blender re-imports them correctly; Unreal's FBX SDK has not read such a file yet (the same risk S4 named).
- Nothing else changed: the retarget numbers for the 15 older takes are identical to the earlier table.

### Rebuild

```
bash C:/Users/kazda/jam/skater-lab/s5/run_all.sh
```

It takes about 4 min: retarget, strip on both files, the node and track dump into `fbx_nodes.txt`, `verify.py` on both files, `inspect.py`, then the sheets.

## Stick verbs on the hybrid (lead's request, 14:15 to 14:25)

**Result.** All 31 of S4's takes now play on R, measured on the re-imported files in Blender. Unreal has not seen these files.
- Pelvis horizontal travel and root motion: 0.000 cm on every take.
- Loop seams: 0.000 cm in the main file.
- Lower skate: on the ice within 0.12 cm on every new standing take; the two goalie blocks kneel and are off by 1.9 cm.
- The aim poses work on R. R's short arms do show, in three places:
  - **Aims:** the stick slides through the mitten to keep the blade on the ice. As a result R reaches about 10 cm farther than S4 in the two-handed aims and 21 cm farther in Aim_FarReach.
  - **Goalie:** the flat stick rides about 17 cm higher than S4's.
  - **Grip:** the left mitten is 3 to 13 cm off the shaft.

**Input.** S4's `export/SK_Skater_S4.fbx` from 14:09, 31 takes (sha256 `f72929cb...`), with its loopcut file, copied to `s5/in/`. The 12:09 snapshot is kept as `in/SK_Skater_S4_1209*.fbx`.

**Track strip.** It stays **disabled**: the lead disabled it at 12:35 in `s5/run_all.sh`, and I left it so. Every take carries `root` identity keys again; the file has 93 curve nodes on `root` over 31 stacks.

**Fixed during this pass.** My loop test, "first pose equals last pose", also caught Poke, Stumble, Lift, Pass and Foul_Reaction, which start and end in Glide. The loopcut file then dropped their last frame. Loops now come from S4's list: the 14 holds and locomotion loops. In the final loopcut, those five one-shots are full length again.

### Files (14:23)

| File | Bytes |
|---|---|
| `C:\Users\kazda\jam\skater-lab\s5\export\SK_Skater_S5.fbx` | 9,119,436 |
| `C:\Users\kazda\jam\skater-lab\s5\export\SK_Skater_S5_loopcut.fbx` | 9,085,500 |

### Export and re-import checks

Export settings are unchanged from the rerun section, except that there is no strip step:
- `UnitScaleFactor` 1.0 (centimetres).
- `root` rotation (0, 0, 0), scale 1; its rest matrix is the identity.
- One base-colour slot for the body, referencing `s6/T_SkaterTB2_BaseColor_clean.png` by path; the stick keeps its `Trim` and `Stick` slots.

On re-import, both files have 32 bones, are 170.0 cm tall and hold 31 takes. Frames, main file:

| Kind | Takes |
|---|---|
| Locomotion | Idle_Faceoff 121, Stride 61, Glide 121, Stop 37, Crossover_Left 41, Crossover_Right 41, Lean_Left 31, Lean_Right 31 |
| Shots | Shot_Slap 43, Shot_Wrist 22, Shot_Charge 31 |
| Contact | Check 31, Fall 37, Fall_Forward 37, GetUp 73, GetUp_Front 73, Celebrate 73 |
| Stick verbs | Poke 22, Stumble 21, Lift 19, Pass 19, Foul_Reaction 49 |
| Faceoff | Faceoff_Ready 31, Faceoff_Draw 19 |
| Goalie | Goalie_Hold 31, Goalie_Block_Left 25, Goalie_Block_Right 25 |
| Aims | Aim_Front 31, Aim_Left 31, Aim_Right 31, Aim_FarReach 31 |

In the loopcut file the 14 loops have one frame fewer. The 16 older takes measure exactly as in the sections above.

### New takes (retarget report plus the re-imported main file)

How to read the columns:
- *Skate err*: R's lower skate height on frames where S4's lower skate is on the ice.
- *Low skate* and *Low stick*: the lowest boot vertex and the lowest stick vertex over the take.
- *Blade err*: R's blade height minus S4's, on frames where S4's blade is within 3 cm of the ice.
- *Slide*: how far the stick slides through the mitten.
- *Grip*: frames with the left hand on the shaft, and the largest gap left after IK.
- *Clamp/tilt*: frames with the head clamp and frames with the stick tilt.
- *Mitt / stick to helmet*: the nearest approach, in cm, of a mitten centre and of any stick vertex to the helmet centre. The helmet half-size is about 28 to 30 cm.

| Take | Frames main / loopcut | Seam main / loopcut (cm) | Pelvis horiz. (cm) | Skate err (cm) | Low skate (cm) | Low stick (cm) | Blade err (cm) | Slide (cm) | Grip frames, gap (cm) | Clamp/tilt | Mitt / stick to helmet (cm) |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Poke | 22 / 22 | one-shot | 0.000 | 0.00 | 0.00 | 1.34 | 0.0 | -2 to 7 | 9, 9.2 | 0/0 | 50 / 48 |
| Stumble | 21 / 21 | one-shot | 0.000 | 0.12 | 0.00 | 1.68 | 0.0 | 0 to 1 | 3, 8.9 | 0/0 | 44 / 51 |
| Lift | 19 / 19 | one-shot | 0.000 | 0.00 | 0.00 | 1.13 | 0.0 | 0 to 8 | 19, 11.1 | 0/0 | 64 / 70 |
| Pass | 19 / 19 | one-shot | 0.000 | 0.00 | 0.00 | 0.62 | 0.0 | -2 to 8 | 19, 10.2 | 0/0 | 69 / 66 |
| Foul_Reaction | 49 / 49 | one-shot | 0.000 | 0.00 | 0.00 | 1.64 | 0.0 | 0 to 1 | 4, 8.9 | 0/0 | 54 / 48 |
| Faceoff_Ready | 31 / 30 | 0.000 / 0.005 | 0.000 | 0.03 | 0.02 | 0.56 | 0.0 | -1 | 31, 4.5 | 0/0 | 66 / 81 |
| Faceoff_Draw | 19 / 19 | one-shot | 0.000 | 0.03 | 0.00 | 0.59 | 0.0 | -2 to 2 | 19, 10.1 | 0/0 | 66 / 70 |
| Goalie_Hold | 31 / 30 | 0.000 / 0.005 | 0.000 | 0.12 | 0.12 | **26.27** | n/a | 0 | 0 | 0/0 | 44 / 85 |
| Goalie_Block_Left | 25 / 25 | one-shot | 0.000 | 1.91 | 0.11 | **26.27** | n/a | 0 | 0 | 21/0 | 40 / 85 |
| Goalie_Block_Right | 25 / 25 | one-shot | 0.000 | 1.84 | 0.11 | **25.46** | n/a | 0 | 0 | 0/0 | 44 / 80 |
| Shot_Charge | 31 / 30 | 0.000 / 0.004 | 0.000 | 0.00 | 0.00 | 85.56 (stick up) | n/a | 0 | 31, 12.6 | 0/0 | 43 / 57 |
| Aim_Front | 31 / 30 | 0.000 / 0.005 | 0.000 | 0.00 | 0.00 | 1.78 | 0.0 | 2 | 31, 10.4 | 0/0 | 71 / 70 |
| Aim_Left | 31 / 30 | 0.000 / 0.005 | 0.000 | 0.00 | 0.00 | 1.68 | 0.0 | 5 | 31, 2.9 | 0/0 | 63 / 61 |
| Aim_Right | 31 / 30 | 0.000 / 0.005 | 0.000 | 0.00 | 0.00 | 1.41 | 0.0 | 3 | 31, 8.8 | 0/0 | 72 / 70 |
| Aim_FarReach | 31 / 30 | 0.000 / 0.005 | 0.000 | 0.00 | 0.00 | 4.26 | 2.4 | **25 (max)** | 0 (one hand) | 0/0 | 70 / 66 |

### R's aim numbers, for device A's blend

The blade centre is measured relative to the pelvis, on R's solved pose after all passes. The angle is from the skater's forward (+ is left); the reach is horizontal. S4's numbers come from the same code on S4's own pose.

| Pose (frame) | R angle | R reach | R blade height | S4 angle | S4 reach | Left mitten gap on R |
|---|---|---|---|---|---|---|
| Aim_Front (1) | **+3.6 deg** | **1.03 m** | 7.4 cm | +3.0 deg | 0.94 m | 10.4 cm |
| Aim_Left (1) | **+70.3 deg** | **1.03 m** | 7.3 cm | +68.3 deg | 0.93 m | 2.9 cm |
| Aim_Right (1) | **-63.0 deg** | **1.05 m** | 7.0 cm | -63.5 deg | 0.95 m | 8.6 cm |
| Aim_FarReach (1) | **+3.7 deg** | **1.69 m** | 13.3 cm | +2.2 deg | 1.48 m | one hand |
| Poke (10, the thrust) | -0.4 deg | 1.93 m | 28.6 cm | -1.9 deg | 2.14 m | one hand |
| Pass (11, the release) | -1.7 deg | 1.19 m | 9.4 cm | -0.2 deg | 1.08 m | 8.2 cm |
| Lift (10, the flick top) | +1.6 deg | 1.20 m | 56.2 cm | +0.8 deg | 1.16 m | 7.7 cm |
| Faceoff_Ready (1) | +2.4 deg | 0.96 m | 5.8 cm | +2.7 deg | 0.90 m | 4.5 cm |
| Goalie_Hold (1) | +72.0 deg | 1.29 m | 29.1 cm | +66.5 deg | 1.27 m | free glove |
| Goalie_Block_Left (25) | +86.7 deg | 1.39 m | 35.6 cm | +84.2 deg | 1.37 m | |
| Goalie_Block_Right (25) | -63.4 deg | 1.81 m | 30.7 cm | -61.3 deg | 1.98 m | |

- **Blend range on R:** by angle, Right -63.0, Front +3.6, Left +70.3. Then blend toward Aim_FarReach by (reach - 1.03) / (1.69 - 1.03).
- **Why S4's numbers differ here:** S4's report gives 0.97 m / 1.51 m for its aims, but the same code measures 0.94 / 1.48 m on S4's own pose here. I used S4's stick-blade vertex mean and the pelvis head as the centre, so the angles agree within 2 deg; the definitions differ by about 3 cm.
- **Why R reaches farther:** R's pelvis sits 4.4 cm behind the origin. R's hands are higher because its arms are shorter, so the stick slides 2 to 5 cm through the mitten to keep the blade on the ice; in Aim_FarReach it slides the full 25 cm. The stick's direction is S4's exactly.

### Sheets (from the re-imported main file)

- `devlog/shots/20261112-142324-B-lab-s5c-stick-verbs.png`: Poke, Stumble, Lift, Pass, Foul_Reaction.
- `devlog/shots/20261112-142324-B-lab-s5c-faceoff-goalie.png`: Faceoff_Draw; Goalie_Block_Left and _Right; the holds Faceoff_Ready, Goalie_Hold and Shot_Charge.
- `devlog/shots/20261112-142324-B-lab-s5c-aims-top.png`: the four aims from straight above (forward is up) and from the 3/4 front.

### Looked at; what still looks wrong

- **Goalie stick floats.** In Goalie_Hold and both blocks the stick lies flat, but its lowest point is 25 to 26 cm above the ice; S4's is 16 to 19 cm. The flat stick is 17 to 18 cm higher than S4's. R's mitten cannot reach as low, and the slide rule does nothing for a flat shaft. The free glove is up near the cage, as in S4 (mitten to helmet centre 40 to 44 cm, so touching, not inside). The head clamp fires on 21 frames of Goalie_Block_Left. A fix needs arm work: an IK pull of the right hand down to S4's stick height. That is not a clamp; I did not do it.
- **Aim_FarReach.** It relies on the full 25 cm slide, so the butt of the stick sticks out about 30 cm behind the mitten. The blade is still 2.4 cm higher than S4's. R's one arm cannot lunge as far as S4's.
- **Poke.** The thrust's blade is 28.6 cm up at frame 10, 4.5 cm above S4's, at 1.93 m reach. It reads as a thrust at 160 px.
- **Two-handed aims.** The left mitten misses the shaft by 3 to 10 cm on Aim_Front and Aim_Right; it is visible at 160 px and not at 40 px.
- **Faceoff_Ready: clean.** The nearest stick vertex is 81 cm from the helmet centre and the nearest mitten 66 cm, so no helmet-and-stick contact. Faceoff_Draw stays 70 cm clear.
- **Shot_Charge.** Same wind-up as the slap; the stick stays 57 cm from the helmet centre; the left mitten is 12.6 cm off the shaft.
- **Small motions.** Lift and Pass are small, as in S4, and read only at 160 px.

### Rebuild

```
bash C:/Users/kazda/jam/skater-lab/s5/run_all.sh
```

About 8 min with the new sheets. The strip step stays disabled.
