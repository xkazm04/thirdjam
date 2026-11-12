# Skater lab S4: scripted original (Blender Python only), report at 11:25

Question: can a Blender Python script alone build an *original* articulated toy skater with scripted, perfectly looping clips, exported as FBX?
**Answer: yes.** The whole lane rebuilds from nothing in 82 s with one command. No service, no download, no credits. Nothing was imported into
Unreal (lane rule), so everything Unreal-side below is **unverified until lane B5 imports it**.

Lab folder: `C:\Users\kazda\jam\skater-lab\s4\` (scripts, `export\`, renders in `r\`, sheets in `sheets\`, measurements in `inspect.jsonl`,
`check_fbx.json`, `clips_report.json`).

## Steps: PASS/FAIL with evidence

| Step | Result | Evidence |
|---|---|---|
| 1. `build_skater.py`: mesh + rig | **PASS** | 5,344 triangles (body 5,148 + stick 196; cap 15k), 2,776 vertices, 0 textures, 9 material slots `Jersey Trim Helmet Gloves Skin Pads Skate Blade Stick`. 24 bones (below), real `root` at the origin, rest A-pose (arms 50 deg down), limbs separate. Rest bounds 172.7 cm tall (helmet top), 0.0 cm lowest point. Every part is rigid: one vertex group per part at weight 1.0. Normals: smooth shading with sharp edges above 40 deg (60 deg on spheres), so the rounded bevels shade as one surface and box edges stay crisp under a toon ramp. |
| 2. `build_clips.py`: 10 clips from pose tables | **PASS** | All 10 clips, pelvis horizontal travel **0.000 cm** at every frame of every clip (pelvis x and y are locked; only its height is keyed); root never moves (0.000 cm). Loop seams 0.000 cm (first frame copied into the last frame). Lowest mesh point over every clip (stick included) >= 0.0 cm after the ice clamps. Table below. |
| 3. Export FBX + GLB, then prove | **PASS** (Blender side) | `export\SK_Skater_S4.fbx` 2.4 MB (mesh, stick, armature, 10 takes), `export\SK_Skater_S4.glb` 0.57 MB (same, 10 animations), `SM_S4_Stick.fbx` (optional static stick, pivot at the grip). `tools/meshy/inspect.py` on both: same 24 bones, 9 materials, 10 actions, `move_root` [0,0,0] and `move_pelvis` x,y = 0 on every action. Re-import of the FBX in Blender: armature scale 1.0, root bone `root`, 60 fps. Renders and sheets made from the **re-imported FBX**, not from the source scene. |
| Iterate at least twice | **PASS** (4 passes) | Pass 1: slim, small head (about 5 heads), stick sank 14 cm into the ice in Stride. Pass 2: chunkier torso, limbs, gloves, breezers; bigger helmet; deeper stride lean (pelvis 26 deg + spine 26 deg), faceoff bent over further. Pass 3: slap follow-through raised, celebrate stick visible overhead, thicker stick for distance read, ice clamps for skates and stick, hands kept above the ice in Fall. Pass 4: head scaled 1.10 (about 4 heads), rest-pose stick out of the ice, bigger free-arm swing in Stride. |

Bones (parent > child): `root` > `pelvis` > `spine_01` > `spine_02` > `spine_03` > `neck_01` > `head` > `head_socket`; `spine_03` >
`clavicle_l|r` > `upperarm_l|r` > `lowerarm_l|r` > `hand_l|r`; `hand_r` > `stick_socket` (the stick mesh is skinned to it); `pelvis` > `thigh_l|r` >
`calf_l|r` > `foot_l|r`; `root` > `puck_socket` (78 cm ahead, 10 cm left, z 0). `head_socket` is 186 cm up. `spine_02` was added so the spine
chain matches the mannequin names. No twist, ball or finger bones.

| Clip (take `Armature|<name>`) | Frames @60 | Seconds | Loop | Pelvis horizontal, max over clip | Pelvis height range | Seam first-to-last (max joint) |
|---|---|---|---|---|---|---|
| Idle_Faceoff | 121 | 2.0 | yes | 0.000 cm | 1.2 cm (breathing) | **0.000 cm** |
| Stride | 61 | 1.0 | yes | 0.000 cm | 4.0 cm | **0.000 cm** |
| Glide | 121 | 2.0 | yes | 0.000 cm | 1.0 cm | **0.000 cm** |
| Stop | 37 | 0.6 | one-shot | 0.000 cm | 15 cm | n/a (60 cm, by design) |
| Shot_Slap | 43 | 0.7 | one-shot | 0.000 cm | 6 cm | n/a |
| Shot_Wrist | 22 | 0.35 | one-shot | 0.000 cm | 3 cm | n/a |
| Check | 31 | 0.5 | one-shot | 0.000 cm | 14 cm | n/a |
| Fall | 37 | 0.6 | one-shot, ends lying flat | 0.000 cm | 55.5 cm | n/a |
| GetUp | 73 | 1.2 | one-shot, from the Fall end pose to Glide | 0.000 cm | 51.5 cm | n/a |
| Celebrate | 73 | 1.2 | one-shot, ends arms and stick up | 0.000 cm | 10 cm | n/a |

How the clips are authored: each clip is a list of key poses `(frame, pose, ease)` in `build_clips.py` (`CLIPS`, with named poses `GLIDE`, `FACEOFF`,
`SLAP_UP` and so on). A pose holds pelvis height, (pitch, roll, yaw) per spine, neck, head and clavicle, **skate targets** (x, y, z, yaw, pitch,
edge roll) solved by two-bone IK, a **stick target** (blade heel or grip position plus yaw, tilt, twist) that the right arm reaches by IK, and the left
hand as "grip at fraction f of the shaft", "offset from the shoulder" or an absolute point. Loops use cyclic Catmull-Rom between keys; one-shots ease
per key. Every frame is baked (linear keys at 60 Hz), so any number can be changed and the clip rebuilt in 6 s. The left-shot slap (bottom hand left)
winds up over the head, contacts at the front left, and follows through high to the front right.

## Rubric row (judged as harshly as a generated candidate)

| Candidate | Look fit | Technical | Motion | Cost | Provenance | Import risk |
|---|---|---|---|---|---|---|
| S4 scripted toy (`SK_Skater_S4`) | **3**: reads as a hockey skater at a glance (helmet with cage, wide jersey with white hem and sleeve stripes, team-colour socks, dark breezers, skates, stick); clean blocky silhouette from behind; at 40 px the white helmet and the jersey colour carry it and blue against orange is unmistakable. But it is visibly primitive "programmer art": rounded boxes and tubes, no face, no hands, no cloth folds; next to a good generated figure it looks cheaper. | **5**: 5.3k tris, no textures, 24 bones with mannequin names, real root at the origin, A-pose, limbs separate, rigid weights (nothing to skin badly), sockets as bones, one material slot per colour. | **4**: loops exact (0.000 cm seam), 0.000 cm pelvis drift, no ice penetration, stride reads as skating (side push, full leg extension, deep knee, 52 deg forward lean, see side and behind rows), slap wind-up readable even at 40 px, fall ends flat. Minus: rigid joints and pose-to-pose motion look like a toy; Check's shoulder thrust is weak; Stop reads only as "turn and lean", no spray. | **5**: 0 credits, 82 s full rebuild, about 25 min of authoring from scratch. | **5**: `original` (made by our scripts); counts as ours for every pool that scores own work; no licence questions. | **3**: unverified in Unreal: the extra "Armature" node, front axis, take naming, body + stick as two skinned mesh nodes (expect one combined skeletal mesh), sockets are bones not `USkeletalMeshSocket`s. Same skeleton for mesh and clips, so no retarget. |

## Contact sheets (filed with `devlog/shot.mjs`)

- `devlog/shots/20261112-112323-B-lab-s4-turntable.png`: 8-view turntable, rest A-pose (top) and Glide pose (bottom)
- `devlog/shots/20261112-112323-B-lab-s4-skating.png`: Idle_Faceoff, Stride (3/4 front), Stride (side), Stride (from behind), Glide, Stop
- `devlog/shots/20261112-112323-B-lab-s4-shots.png`: Shot_Slap, Shot_Wrist, Check
- `devlog/shots/20261112-112324-B-lab-s4-fall.png`: Fall, GetUp, Celebrate
- `devlog/shots/20261112-112324-B-lab-s4-distance.png`: about 40 px tall from the chase angle, 2x nearest: Stride blue, Stride orange, Shot_Slap, Fall orange, Stop, Idle_Faceoff orange

Look in the renders: Workbench studio light, material colour, black object outline. It stands in for the P2 cel look; it is not the game's shader.

## Rebuild (exact commands)

Everything, one command (about 82 s):
```
bash C:/Users/kazda/jam/skater-lab/s4/run_all.sh
```
Or step by step:
```
"C:/Program Files/Blender Foundation/Blender 4.2/blender.exe" -b --factory-startup --python C:/Users/kazda/jam/skater-lab/s4/build_skater.py -- C:/Users/kazda/jam/skater-lab/s4
"C:/Program Files/Blender Foundation/Blender 4.2/blender.exe" -b --factory-startup --python C:/Users/kazda/jam/skater-lab/s4/build_clips.py -- C:/Users/kazda/jam/skater-lab/s4
"C:/Program Files/Blender Foundation/Blender 4.2/blender.exe" -b --factory-startup --python C:/Users/kazda/jam/thirdjam/tools/meshy/inspect.py -- C:/Users/kazda/jam/skater-lab/s4/export/SK_Skater_S4.fbx C:/Users/kazda/jam/skater-lab/s4/export/SK_Skater_S4.glb
"C:/Program Files/Blender Foundation/Blender 4.2/blender.exe" -b --factory-startup --python C:/Users/kazda/jam/skater-lab/s4/check_fbx.py -- C:/Users/kazda/jam/skater-lab/s4/export/SK_Skater_S4.fbx C:/Users/kazda/jam/skater-lab/s4/check_fbx.json
bash C:/Users/kazda/jam/skater-lab/s4/render_all.sh
```
`render_s4.py` is my own Workbench renderer. `tools/meshy/render.py` does not fit this file: it hides every mesh but the largest, which would hide the
stick, and it assumes a figure that faces -Y. Its modes are `turntable | motion | side | back | game` (game = orthographic, 40 px per 1.70 m, from
behind and above), with an `orange` tint option.

## Export settings and what Unreal should do with them (unverified until imported)

- Built in Blender facing **+X**, left = +Y, Z up, metres. FBX: `axis_forward=-Z, axis_up=Y` (Blender defaults), `apply_unit_scale=True`,
  `apply_scale_options=FBX_SCALE_ALL`, `global_scale=1.0`, `add_leaf_bones=False`, `primary_bone_axis=Y, secondary_bone_axis=X`,
  `armature_nodetype=NULL`, `mesh_smooth_type=FACE`, `bake_anim_use_all_actions=True` (one take per action), `bake_anim_use_nla_strips=False`,
  `bake_anim_force_startend_keying=True`, step 1, no simplification, 60 fps.
- Expected in UE 5.9 with default FBX skeletal import options ("Force Front XAxis" **off**, Import Uniform Scale 1.0, Convert Scene on): the figure
  stands about 173 units tall, Z up, **faces +X** (unlike the UE mannequin, which faces +Y: do not add the usual -90 yaw on the mesh component), its
  left hand on -Y. Units arrive as centimetres with no 100x or 0.01x scale on the root (the Blender re-import shows armature scale 1.0).
- The armature object is named `Armature`. The expectation, from community practice, is that UE drops a root null named "Armature" so `root` is the
  skeleton root. **If it does not**, the skeleton gets one extra bone `Armature` above `root`. That is harmless for this visual, but B5 should report it.
- Body and stick are two skinned mesh nodes on one armature. Expect one skeletal mesh `SK_Skater_S4` with 9 material slots. 10 anim sequences, probably
  named `SK_Skater_S4_Anim_Armature_<Clip>`; rename on import. Root motion: the root has identity keys, so there is nothing to extract.
- Looping clips repeat their first frame as the last frame, as the brief asked: a 1/60 s duplicate at the wrap. Trimming the last frame in UE
  would make the wrap seamless.
- Sockets: `stick_socket`, `puck_socket`, `head_socket` exist as **bones**. `GetSocketLocation` / `DoesSocketExist` accept bone names, so the
  contract's names work as they are. Adding real skeletal-mesh sockets on those bones (the import skill's step 2) is still cleanest for acceptance.

## Team colour, jersey number, knock-down direction

- **Orange team**: only the `Jersey` slot carries team colour (jersey, sleeves, socks: the largest areas, front and back). A material instance whose
  base colour is the `TeamColour` vector param tints it. `Trim` stays white on both teams. The distance sheet shows the orange tint reading at
  40 px. `Helmet` (off-white), `Gloves` and `Pads` (near black) are neutral on purpose, and can take `TeamColour` too if the Director wants
  more colour.
- **Jersey number**: no texture is shipped. UV0 is a planar projection seen from behind that frames the jersey back (u 0..1 across the back, v 0..1
  from hem to shoulders). A jersey material can draw the seven-segment digits `NumberTens` / `NumberOnes` in that UV box, masked to back-facing
  normals (the same digit approach as the T-A skill). That is material work on the Unreal side; it has not been built.
- **Knock-down direction**: `Fall` falls **backwards** (towards the figure's -X) and ends flat. The visual yaws the mesh component so that its
  backward axis points along (DirX, DirZ) (yaw = atan2(DirZ, DirX) + 180 deg in rink terms), holds the last frame while down, then plays
  `GetUp` and eases the yaw back to facing. A `Fall_Forward` clip would take about 10 min in the pose table if the Director wants falls in both
  directions without the yaw trick.
- Other calls: `PlayShoot` = `Shot_Wrist` (or `Shot_Slap` for a charged shot), `PlayCheck` = `Check`, speed blend `Glide` <-> `Stride`, lean from
  `TurnRate` by an additive spine roll or a component roll (no lean clips were authored), `Idle_Faceoff` for faceoffs and pauses, and `Celebrate`.

## Verdict: **keep as fallback** (and queue it for import if the editor has a free 20-minute slot)

It is the only candidate with zero cost, zero licence risk and a full, exactly looping clip set on one skeleton with no retarget, and it is
re-tunable in seconds. It is a step up from T-A (real skates, cage, stripes, a stride that reads as skating at 40 px). It does not beat a
good generated figure on looks. Use it if T-B2 or T-B1 stall on rig or retarget work, or as the "ours" option for the provenance pool.

## What is weak

- Primitive look: no face, no hands, no fabric. It reads well as a toy, but it is not a character.
- Rigid parts: joints show gaps and overlaps when bent (elbow and knee spheres hide most of it). The raised arms pass into the helmet in the
  Slap wind-up and in Celebrate (visible in the sheets at 160 px, invisible at 40 px).
- Motion is pose-to-pose with splines: no overlap, no follow-through on the head or arms, no secondary motion. `Check` is the weakest clip; `Stop`
  needs blade spray from the game to sell it.
- The stick is hidden by the body from directly behind in Stride.
- No lean or turn clips (`Skate_Left/Right`); lean must come from code.

## What I could not verify

- Anything in Unreal: the import itself, the "Armature" node handling, the front axis, the scale, take naming, the material slot order, sockets,
  performance (5.3k tris times 6 skaters should be trivial), and the cel shader on these normals.
- Whether the GLB is useful downstream. It inspects correctly; Blender re-imports it at 24 fps with the same durations.
- Hands do not grip exactly: the left hand targets the shaft by IK, but the glove is a box, so the grip is approximate (centimetres, invisible at
  game distance).

## Ledger rows for the lead to paste (ASSETS.md)

| File | Origin | Tool | Licence | Note |
|---|---|---|---|---|
| `SK_Skater_S4.fbx` / `.glb` (mesh, rig, 10 clips) | original | Blender 4.2 Python, `skater-lab/s4/build_skater.py`, `build_clips.py` | ours | no generated or downloaded input |
| `SM_S4_Stick.fbx` | original | same | ours | optional static stick, pivot at the grip |

## For the lead or the operator to decide

1. Queue S4 for import (about 20 min of B5's editor time) as the "ours" fallback, or keep it on disk only.
2. If it is imported: should `Helmet` and `Gloves` also take `TeamColour`, and is the back-UV number approach wanted (who builds the digit material)?
3. Should I add `Fall_Forward`, `Skate_Left/Right` lean clips and a crossover? Each is about 10 min of pose-table work.

## Polish pass (lead's request, 11:26 to 11:35)

Same rules: Blender only, nothing in Unreal. All Unreal behaviour is still **unverified until lane B5 imports it**.

**Triangles: 5,932** (body 5,736 + stick 196; cap 6k). Still 0 textures, 9 slots, 24 bones (unchanged), rest height 172.7 cm. To stay under 6k
I lowered the shoulder-pad spheres to 12x8 segments.

### What changed

1. **Toy ball joints**, each weighted to the parent bone: shoulder balls on `clavicle_l/r`, elbow balls moved to `upperarm_l/r`, hip balls
   on `pelvis`, knee balls on `thigh_l/r` (the knee pads stay on the calves). The gaps at bent joints are closed in every sheet.
   **Arms in the helmet are fixed by pose, not by head size.** A measurement counts arm and glove vertices inside the helmet volume on every
   frame. It is now **0 for every frame** of `Shot_Slap`, `Celebrate`, `Check`, `Stride` and `Fall_Forward`; before the fix it was 36 in the slap
   and 40 in Celebrate. The fix needed extra keys: in Celebrate the hands go out in front first, then up; in the slap wind-up the hands come up
   on the left side and the downswing is routed outside the head; the slap follow-through ends lower and wider.
2. **Face**: two dark eye dots between the cage bars plus a dark visor line under the helmet brim (turntable, front views). These are 3 small
   pieces in the existing near-black `Pads` slot. No new slot was added: `Skin` and `Helmet` are light colours, so they cannot make dark dots.
3. **New clips**: `Fall_Forward` (0.6 s, ends face down with the head turned, arms out); `Lean_Left` / `Lean_Right` (2-frame holds of the glide
   carve: 25 deg body roll, skates out to the side of the turn on edge, inside knee bent more, stick trailing behind on the outside, for blending
   by turn rate); `Crossover_Left` / `Crossover_Right` (40-frame in-place loops: the outside skate steps over, the inside skate pushes under,
   built on the lean pose); and a new **`Check`** (0.5 s: wind-back with the right shoulder and elbow pulled back, then a step in with the right
   skate, a drive up 16 cm, and the right shoulder and elbow leading).
4. **Stride from behind**: the stick is carried wide on the right (grip 26 to 32 cm outside the shoulder, blade turned out). The 40 px chase-view
   rows show the stick to the right of the body on every frame. The side push (leg out on either side) and the free-arm swing read at 40 px.
5. **`export\SK_Skater_S4_loopcut.fbx`**: looping clips without the repeated last frame (Stride 60, Glide 120, Idle_Faceoff 120, crossovers 40
   frames). The 2-frame leans keep both frames. `SK_Skater_S4.fbx` (first frame repeated at the end) is still exported next to it.

### Clip table (measured on the re-imported FBX files)

| Clip | Frames, main FBX | Frames, loopcut | Pelvis x,y max | Seam first-last, main | Seam, loopcut (one frame step) | Lowest point |
|---|---|---|---|---|---|---|
| Idle_Faceoff | 121 | 120 | 0.000 cm | **0.000 cm** | 0.004 cm | 0.04 cm |
| Stride | 61 | 60 | 0.000 | **0.000** | 0.611 | -0.12 |
| Glide | 121 | 120 | 0.000 | **0.000** | 0.004 | 0.0 |
| Crossover_Left | 41 | 40 | 0.000 | **0.000** | 1.402 | 0.23 |
| Crossover_Right | 41 | 40 | 0.000 | **0.000** | 1.402 | -0.39 |
| Lean_Left (hold) | 2 | 2 | 0.000 | 0.000 | 0.000 | 1.2 |
| Lean_Right (hold) | 2 | 2 | 0.000 | 0.000 | 0.000 | 1.17 |
| Stop | 37 | 37 | 0.000 | one-shot | | 0.0 |
| Shot_Slap | 43 | 43 | 0.000 | one-shot | | 0.0 |
| Shot_Wrist | 22 | 22 | 0.000 | one-shot | | 0.0 |
| Check | 31 | 31 | 0.000 | one-shot | | 0.0 |
| Fall | 37 | 37 | 0.000 | one-shot, ends flat on the back | | 0.0 |
| Fall_Forward | 37 | 37 | 0.000 | one-shot, ends flat face down | | -1.14 |
| GetUp | 73 | 73 | 0.000 | one-shot (from the back fall) | | 0.0 |
| Celebrate | 73 | 73 | 0.000 | one-shot | | 0.0 |

Root motion is 0.000 cm in every take of both files. The "loopcut seam" column is the motion over one frame (last frame to first), which is the
wanted continuation, not a jump. Knock-down: the visual picks `Fall` or `Fall_Forward`, whichever is nearer to (DirX, DirZ) relative to facing,
and yaws the mesh by at most 90 deg for the rest. There is no forward `GetUp` yet: from `Fall_Forward` the visual would need either a short
roll-over blend into `GetUp` or a `GetUp_Front` clip (about 10 min).

### Contact sheets (filed with `devlog/shot.mjs`)

- `devlog/shots/20261112-113449-B-lab-s4b-turntable.png`: ball joints and face, rest and Glide
- `devlog/shots/20261112-113449-B-lab-s4b-skating.png`: faceoff, stride (3/4, side, behind), glide, leans (4 views each), stop
- `devlog/shots/20261112-113449-B-lab-s4b-crossover.png`: crossovers
- `devlog/shots/20261112-113450-B-lab-s4b-shots.png`: slap, wrist, new check
- `devlog/shots/20261112-113450-B-lab-s4b-fall.png`: fall, fall forward, get-up, celebrate
- `devlog/shots/20261112-113450-B-lab-s4b-distance.png`: about 40 px, chase angle: stride blue and orange, slap, fall, stop, check

### Still weak

- Crossovers read best from behind; from the 3/4 front the leg crossing is subtle and the trailing stick is hidden.
- The slap wind-up now goes up beside the left shoulder rather than straight over the head: less dramatic from the front, still clear in the
  40 px chase view.
- The leans float the skates 1.2 cm (the ice clamp is conservative on a 25 deg edge). `Fall_Forward` dips 1.1 cm into the ice at the end
  (the chest).
- The eye dots use the `Pads` slot. If a material instance ever tints `Pads` with the team colour, the eyes would change colour too; a separate
  `Eyes` slot would cost one more draw.
- Motion is still pose-to-pose with no secondary motion; there is no forward get-up.

### Rebuild (unchanged command; about 2 min now, 15 takes and two FBX files)

```
bash C:/Users/kazda/jam/skater-lab/s4/run_all.sh
```
Outputs: `export\SK_Skater_S4.fbx`, `export\SK_Skater_S4_loopcut.fbx`, `export\SK_Skater_S4.glb`, `SM_S4_Stick.fbx`, `inspect.jsonl` (all three
files), `check_fbx.json`, `check_fbx_loopcut.json`, `sheets\*.png`.

## Export fix (lead's request after lane B4's import, 12:01 to 12:12)

Blender only. **Unreal is the real test.** Everything below is what the FBX file itself contains and what Blender's importer makes of it. The
reasoning about Unreal rests on one fact from B4's first import, explained under point 1.

### 1. Root scale and rotation

**What the first file contained** (read straight from the FBX with Blender's FBX parser, `fbx_nodes.py`):
- `UnitScaleFactor` was 100: metres.
- The `Armature` null had `Lcl Rotation` (-90, 0, 0): Blender's axis turn.
- The `root` bone node had `Lcl Rotation` (90, 0, 0), because the root bone pointed up (+Z).

Unreal canceled the null's -90 with its own Y-up to Z-up conversion and dropped the null. That left `root` with its own 90 deg ("roll 90") and
the metre unit as "scale 100". This matches B4's numbers exactly.

**What changed:**
- **Centimetres in the data.** Before the FBX export, `build_clips.py` scales the bone heads and tails and the mesh vertices by 100, scales the
  pelvis location keys by 100, and sets the scene unit to 0.01 m. The armature object keeps identity rotation and unit scale. Export uses
  `apply_unit_scale=True`, `apply_scale_options=FBX_SCALE_NONE` and `global_scale=1.0`, so the file says `UnitScaleFactor` 1.0 (cm) and every
  node `Lcl Scaling` is 1. This is lane B5's recipe. The GLB is written before the scaling and stays in metres.
- **`root` lies along +Y with roll 0** (head (0,0,0), tail (0,15,0) cm) instead of pointing up. Its rest matrix is identity, so its FBX node
  has `Lcl Rotation` (0, 0, 0). The world rest position of every other bone, the mesh, the bone names and all clip names are unchanged.
  Only `root`'s own direction changed; it carries no geometry and no keys. **Side effect for lane B4:** the socket compensation
  (`s2_socket_xf.py`) should no longer be needed. Children of `root` (`pelvis`, `puck_socket`) now have different local rotations relative to
  `root`, but the same world rest pose.
- **Axes stay `axis_forward=-Z`, `axis_up=Y`**, which B4 proved gives +X facing and a null that Unreal cancels. I first tried B5's `-Y`/`Z`:
  the file then had a 180 deg Z rotation on the null, so I went back. Other settings are unchanged: `add_leaf_bones=False`, primary bone axis Y,
  secondary X, `mesh_smooth_type=FACE`, one take per action, step 1, no simplification. The only other change is
  `bake_anim_use_all_bones=False`, and the file is written from the rest pose (no clip assigned).
- **No take keys `root` or the armature object.** Blender's exporter writes a curve node for every bone and for the armature object in every take
  whatever the options; I tested `bake_anim_use_all_bones` and `force_startend_keying` both off. So a new post-step, `strip_tracks.py`,
  rewrites each FBX without the curve nodes and curves of `root` and `Armature`: 96 curve nodes and 288 curves per file. It uses Blender's
  own FBX parser and binary writer; everything else is copied as parsed.

**Proof (Blender side):**
- In the file (`fbx_nodes.txt`, both FBX files): `UnitScaleFactor` 1.0; `Armature` null translation 0, rotation (-90, 0, 0) (the axis turn,
  as before), scaling 1; `root` translation 0, rotation **(0, 0, 0)**, scaling **1**; `pelvis` at 88 (cm).
- Curve nodes per model over the 16 takes: 48 for every bone except `root` and `Armature`, which have **none**.
- Re-imported with Blender's importer (`check_fbx.json`, `check_fbx_loopcut.json`):
  - The armature object's matrix is `diag(0.01, 0.01, 0.01, 1)`, meaning Blender turns the cm file back into metres on the object. The first
    file re-imported at scale 1.0 because its node carried the 100.
  - `root` rest `matrix_local` is the **identity**.
  - 24 bones, rest height **172.7 cm**.
  - Every take keys 23 bones, `root` not among them.

Expected in Unreal: `root` reference pose with no rotation, scale 1.0, the figure facing +X, 172.7 units. **Unverified until B4 or B5
re-imports.**

### 2. Hold clips

Every hold is now a **30-frame loop** (31 frames in the main file with the first frame repeated, 30 in loopcut). A breathing wobble of
0.1 deg (pelvis and spine_02 pitch) plus 0.05 mm of pelvis height runs through the IK, so no curve of a solved bone is constant. This applies to
`Lean_Left` and `Lean_Right` (`hold_keys()` in `build_clips.py`). Seam: 0.000 cm in the main file, 0.004 cm per frame in loopcut. Left and right
render mirrored (skating sheet, leans row).

### 3. `GetUp_Front` (new, 1.2 s, 73 frames)

Starts from the `Fall_Forward` end pose. It pushes up on both hands with the stick flat under the right glove, brings the left knee under,
kneels on the right, rises into a crouch and ends in the `Glide` pose. Pelvis horizontal travel 0.000 cm; the lowest point is -1.14 cm (the
chest at frame 0, inherited from the `Fall_Forward` end). Sheet: `devlog/shots/20261112-121127-B-lab-s4c-getup-front.png` (top row
`Fall_Forward`, bottom row `GetUp_Front`).

### Measurements (re-imported FBX files, both variants)

| Clip | Frames main / loopcut | Keyed bones (root?) | Pelvis x,y max | Seam main / loopcut (cm) | Lowest point (cm) |
|---|---|---|---|---|---|
| Idle_Faceoff | 121 / 120 | 23 (no) | 0.000 | 0.000 / 0.004 | 0.04 |
| Stride | 61 / 60 | 23 (no) | 0.000 | 0.000 / 0.611 | -0.12 |
| Glide | 121 / 120 | 23 (no) | 0.000 | 0.000 / 0.004 | 0.0 |
| Lean_Left | 31 / 30 | 23 (no) | 0.000 | 0.000 / 0.004 | 1.2 |
| Lean_Right | 31 / 30 | 23 (no) | 0.000 | 0.000 / 0.004 | 1.09 |
| Crossover_Left | 41 / 40 | 23 (no) | 0.000 | 0.000 / 1.402 | 0.23 |
| Crossover_Right | 41 / 40 | 23 (no) | 0.000 | 0.000 / 1.402 | -0.39 |
| Stop | 37 | 23 (no) | 0.000 | one-shot | 0.0 |
| Shot_Slap | 43 | 23 (no) | 0.000 | one-shot | 0.0 |
| Shot_Wrist | 22 | 23 (no) | 0.000 | one-shot | 0.0 |
| Check | 31 | 23 (no) | 0.000 | one-shot | 0.0 |
| Fall | 37 | 23 (no) | 0.000 | one-shot | 0.0 |
| Fall_Forward | 37 | 23 (no) | 0.000 | one-shot | -1.14 |
| GetUp | 73 | 23 (no) | 0.000 | one-shot | 0.0 |
| GetUp_Front | 73 | 23 (no) | 0.000 | one-shot | -1.14 |
| Celebrate | 73 | 23 (no) | 0.000 | one-shot | 0.0 |

Triangles are unchanged at 5,932. In the loopcut seam column, a value above 0 is the motion over one frame, not a jump.

### Rebuild (one command, about 2.5 min)

```
bash C:/Users/kazda/jam/skater-lab/s4/run_all.sh
```

Order: `build_skater.py`, `build_clips.py` (GLB in metres, then both FBX files in cm), `strip_tracks.py` on both FBX files,
`tools/meshy/inspect.py`, `check_fbx.py` on both, `fbx_nodes.py` / `fbx_tracks.py` into `fbx_nodes.txt`, then `render_all.sh`. File names are
unchanged: `export/SK_Skater_S4.fbx`, `export/SK_Skater_S4_loopcut.fbx`, `export/SK_Skater_S4.glb`.

### Not verified / for the hybrid lane

- Whether Unreal now shows `root` at rotation 0 and scale 1, and the figure still facing +X. The file-level evidence says yes; only an import
  proves it.
- The FBX files are rewritten by `strip_tracks.py` through Blender's binary writer, not by the exporter. Blender re-imports them identically.
  The FBX SDK in Unreal has not read such a file yet; if it rejects it, run with the strip step removed (only root's identity keys come back,
  which are harmless now that the reference pose is identity at scale 1).
- `SM_S4_Stick.fbx` is unchanged (metres, `FBX_SCALE_ALL`).
