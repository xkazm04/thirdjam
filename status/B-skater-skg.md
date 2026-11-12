# Skater lab SKG: the goalie kit and goalie clips (Blender only), report at 15:52

Lane SKG, device B. Started 15:11, reported 15:52. Blender 4.2 headless only: no Unreal, no git, no network, no credits.
Lab folder: `C:\Users\kazda\jam\skater-lab\skg\`. Lanes `s4/` and `skf/` were only read. SKF's three figures existed in `skf/export/<style>/` when
I got there, so **p3_clay and p4_broadcast are fitted to SKF's figures** (`SK_Skater_p3_clay.fbx`, `SK_Skater_p4_broadcast.fbx`, as exported
around 15:30). p2_cel uses the s4 figure. If SKF re-exports, re-run `run_all.sh`. Every Unreal-side statement below is **unverified**.

**Answer: done for all three styles.** The output is a 9-piece rigid kit per style, 17 goalie takes on the s4 skeleton (14 new clips plus s4's 3
goalie names kept working), the exports, re-import checks and 15 filed sheets. From 30 m at 40 px the goalie reads as the goalie: it is wider and
squarer than a skater, with white pads, a white blocker and a big white mask.

## 1. The kit (`goalie_kit.py`)

Pieces are `SM_Goalie_<Piece>`. Every piece is one static mesh. Its origin is the head of the bone it attaches to. Every piece except the stick is
authored in **its bone's own rest frame**, so its relative transform is the identity. The stick follows `SM_S4_Stick`'s grip convention: pivot at
the grip, shaft along -Z, blade toe along +X once the shaft is tilted by its lie about Y.

All sizes are read from the armature: bone heads and tails, plus the bounds of the body vertices weighted to `head`, `calf_*`, `hand_*` and
`spine_03` when a skinned body is present. That is how the mask encloses each style's helmet and the pads sit on each style's shin. One parameter
table, `KIT_STYLES`, sets the three looks:
- **p2_cel**: flat-shaded boxes and an octagon mitt.
- **p3_clay**: bevelled, puffier (x1.08), seeded noise displacement of 7 mm, thicker edges.
- **p4_broadcast**: four vertical rolls on each pad face, a 3-block knee stack, a curved cupped catching glove, and a mask with a chin and a
  throat guard.

Material slots come from the skater's set, plus **`Pad`** for the white pad faces. Watch the names: the skater's `Pads` is dark, the new `Pad` is
white.

| Piece | Attach bone | Tris p2 / p3 / p4 | Slots | Relative transform (bone space) |
|---|---|---|---|---|
| SM_Goalie_Pad_L / _R | calf_l / calf_r | 72 / 552 / 828 each | Pad, Jersey | location 0, rotation identity |
| SM_Goalie_Blocker | hand_r | 72 / 264 / 264 | Pad, Jersey, Gloves | 0, identity |
| SM_Goalie_Glove (catching) | hand_l | 88 / 488 / 432 | Pad, Gloves, Jersey | 0, identity |
| SM_Goalie_Chest (wider torso shell, belly flap, hem, collar) | spine_03 | 48 / 432 / 432 | Jersey, Trim | 0, identity |
| SM_Goalie_Arm_L / _R (shoulder floater, fat sleeve, elbow pad) | upperarm_l / upperarm_r | 272 / 312 / 332 each | Jersey, Trim | 0, identity |
| SM_Goalie_Mask (shell over the helmet, cage, team stripe, throat guard; chin on p3/p4) | head | 370 / 788 / 932 | Helmet (p2) or Pad (p3, p4), Jersey, Blade | 0, identity |
| SM_Goalie_Stick (paddle and wide blade) | stick_socket | 60 / 444 / 444 | Stick, Trim, Pad, Gloves | 0, quaternion wxyz (0.606109, 0.606109, -0.364187, 0.364187) in Blender bone space; written for UE as (0.606109, -0.606109, -0.364187, -0.364187) |
| **Total** | | **1,326 / 4,144 / 4,824** (cap 5k) | | |

- **Goalie stick:** 72 cm grip to heel on the s4 figure, scaled by figure height (68.3 cm p3, 82.0 cm p4). Lie 34 deg, 30 cm of shaft above the
  grip, a 40 cm paddle of 8 to 9.5 cm width, a 40 cm blade of 11.5 to 13 cm height.
- **`attach.json`** (`export/attach.json` = p2; `export/<style>/attach.json` per style) gives, per piece: the bone, the triangles, the slots, the
  relative location and rotation (Blender bone space, plus my conversion to UE), the rest origin in UE component space (cm), and the rest axes.
- **Proof that the attach data is enough:** `check_kit.py` re-imports the skeletal FBX and the 9 piece FBX files. It parents each piece to its bone
  using **only** `attach.json`, then compares world bounds with the source scene at 4 poses (Stance, Butterfly_Hold, GloveSave, PokeCheck).
  Max error is **0.000 cm** for all three styles.

## 2. The clips (`build_goalie.py`)

The solver is s4's: pose tables, legs and arms by two-bone IK, pelvis x and y locked (only its height is keyed), `root` never moved, loops by
cyclic Catmull-Rom with frame 0 copied into the last frame, and one-shots eased per key. I added these goalie parts:
- **Absolute hand orientation**: finger direction plus back-of-hand normal. The blocker faces the shooter and the glove pocket opens forward. The
  wrist is placed so the grip lands exactly.
- **Explicit knee pole and calf roll pole**, so the pads face the shooter and stand on their inner edges in the butterfly.
- **Skate orientation that can follow the calf.**
- **Goalie stick length and lie.**
- **A butterfly fit per figure.** It searches the knee pole and skate position until the pads overlap 0 to 0.8 cm at the knees with no gap at the
  ice.
- **A pad clearance pass.** While the pad slabs overlap by more than 0.8 cm, both knee poles are nudged outward.
- **An ice clamp on pads and skates.** It translates pelvis and ankles up when anything is below the ice. In the butterfly it also lowers them
  until the pads touch.

Pose positions are rescaled for other figures: legs by hip height, heights by head height, reach by arm length.

**s4's goalie names still work.** `Goalie_Hold` is now the new stance as a 30-frame hold. `Goalie_Block_Left` and `_Right` are now the new pad saves
(drop and slide that leg out), held on the last frame like s4's. s4's own versions stay unchanged in s4's files.

**Measurements on the s4 skeleton (p2_cel kit), 60 Hz.**
- **Pelvis x,y travel and root travel are 0.000 cm in every take.** This was measured in the solver and again on the re-imported FBX.
- **Seams:** 0.000 cm in `SK_Goalie_clips.fbx`. In loopcut they are 0.008 cm (Stance), 0.017 cm (Butterfly_Hold) and 1.997 cm (Shuffles: one
  frame of motion, not a jump).
- **Re-import (`check_fbx.py`):** 24 bones, root `root` with identity rest, armature scale 0.01 (cm file), 17 takes, rest height 172.7 cm, body
  lowest point -0.0 cm or higher in every take.

| Clip | Frames (main) | Seconds | Kind | Key frame (s) | Lowest skate | Lowest pad | Lowest stick | Pad overlap, max over clip | Five-hole gap at key | Ice-clamp lift, max |
|---|---|---|---|---|---|---|---|---|---|---|
| Goalie_Stance | 61 (60 loopcut) | 1.0 | loop | 0 | 0.10 | 2.84 | 0.00 | 0.24 | 35.4 (open by design) | 0 |
| Goalie_ShuffleL / R | 25 (24) | 0.4 | loop | 6 (0.10) | 0.00 | 0.00 | 0.00 | 0.41 | 45.3 | 1.32 |
| Goalie_Butterfly_Drop | 15 | 0.233 | one-shot, ends in the butterfly | 10 (0.167) | 0.00 | 0.00 | 0.00 | 0.70 | **0.0** | 0.50 |
| Goalie_Butterfly_Hold | 31 (30) | 0.5 | loop | 0 | 9.74 (skates up behind) | **0.00** | 0.00 | 0.59 | **0.0** | 0.61 |
| Goalie_Butterfly_Recover | 27 | 0.433 | one-shot, right leg first | 12 (0.20) | 0.00 | 0.00 | 0.00 | 0.79 | 18.9 | 1.64 |
| Goalie_GloveSave | 43 | 0.7 | one-shot | 10 (0.167), held to 16, back at 42 (0.43 s) | 0.03 | 0.70 | 0.00 | -0.08 | 38.9 | 0 |
| Goalie_BlockerSave | 43 | 0.7 | one-shot | 10 (0.167), back at 42 | 0.03 | 0.46 | 0.00 | -0.08 | 37.7 | 0 |
| Goalie_PadSaveL / R | 47 | 0.767 | one-shot | 11 (0.183), held to 17, back at 46 (0.48 s) | 0.00 | 2.57 | 0.00 | -0.08 | 63.3 | 0.76 |
| Goalie_PokeCheck | 41 | 0.667 | one-shot | 9 (0.15), back at 40 (0.45 s) | 0.09 | 0.00 | -0.17 | -0.08 | 34.5 | 4.73 |
| Goalie_Freeze | 25 | 0.4 | one-shot, holds the last frame | 12 (0.20) drop, settle at 24 | 0.00 | 0.00 | -0.09 | 0.68 | **0.0** | 1.66 |
| Goalie_Beaten | 61 | 1.0 | one-shot, holds (head down, slump, stick hanging) | 60 | 0.00 | 3.21 | -0.19 | -0.08 | 24.3 | 0 |
| Goalie_Celebrate_Save | 37 | 0.6 | one-shot (glove fist up twice) | 8 (0.133) | 0.10 | 2.02 | 0.00 | 0.71 | 0 | 0 |
| Goalie_Hold (s4 name) | 31 (30) | 0.5 | loop (hold) | 0 | 0.10 | 3.20 | 0.00 | -0.07 | 35.4 | 0 |
| Goalie_Block_Left / Right (s4 names) | 25 | 0.4 | one-shot, holds | 11 / 24 | 0.00 | 2.62 | 0.00 | -0.08 | 63.3 | 0.74 |

How the table columns are measured:
- All values are in cm.
- "Pad overlap" is the penetration depth of the two pad slabs as oriented boxes (separating-axis test). Negative means apart.
- "Five-hole gap" is the smallest distance between the two pad slabs within 15 cm of the ice. Over all three styles the **largest pad overlap
  is 0.79 cm**, and the **lowest point of any pad, skate or stick in any take is -0.19 cm** (p2 Beaten stick). Both are inside the 1 cm limit.
- "Ice-clamp lift" is how far the clamp raised the legs and pelvis to keep pads and skates out of the ice.

**IK misses** (a hand or skate target out of reach, so the limb ends straight):
- p2: 0 cm in Stance, Shuffles and Butterfly; up to 2.9 cm in GloveSave and 5.5 to 7.8 cm in the pad saves.
- p4: up to 9.8 cm in GloveSave and 12.7 cm in PadSaveR.
- These numbers are not split per limb.

The other styles meet the same limits. Their key numbers:

| | Kit tris | Butterfly fit (knee pole y, skate x) | Butterfly pad overlap / five-hole | Freeze overlap / five-hole | Largest overlap, any take | Lowest pad / stick, any take |
|---|---|---|---|---|---|---|
| p2_cel (s4 figure) | 1,326 | 0.075 / 0.04 | 0.59 / 0.0 | 0.68 / 0.0 | 0.79 | 0.00 / -0.19 |
| p3_clay (SKF figure) | 4,144 | 0.0 / 0.06, plus 12 spread steps | 0.79 / 0.0 | 0.73 / 0.0 | 0.79 | 0.00 / -0.12 |
| p4_broadcast (SKF figure) | 4,824 | 0.05 / 0.10, plus 12 spread steps | 0.77 / 0.0 | 0.78 / 0.0 | 0.78 | 0.00 / -0.18 |

## 3. Goal coverage from the front (183 x 122 cm opening, orthographic, goalie 40 cm out)

The fraction of the opening's area covered by the goalie's silhouette (2 px per cm mask, counted in Blender):

| | Stance | Butterfly_Hold | GloveSave (key) | PadSaveL (key) |
|---|---|---|---|---|
| p2_cel | **49.2 %** | **53.0 %** | 49.6 % | 52.4 % |
| p3_clay | **48.7 %** | **49.2 %** | 49.5 % | 48.6 % |
| p4_broadcast | **47.6 %** | **50.3 %** | 45.7 % | 51.3 % |

- **Bottom 15 cm of the opening:** about 38 % covered in stance against 52 to 67 % in the butterfly.
- **See-through slot at the ice within 40 cm of the centre:** 0 cm in stance and butterfly for every style.
- **Exceptions:** a 1.5 cm slot in PadSaveL (p2, p3); in p4, 5.5 cm in GloveSave and 11 cm in PadSaveL.

## 4. Sheets (filed with `devlog/shot.mjs`, all under 200 KB)

Per style (`p2_cel`, `p3_clay`, `p4_broadcast`), stamped 15:49:08 to 15:49:12:
- `devlog/shots/20261112-1549xx-B-lab-skg-<style>-turntable.png`: 8 views, Goalie_Stance.
- `...-<style>-keys.png`: the key frame of all 14 clips. Two rows from the front (the shooter's view) and two rows from the game camera (high and
  in front, orthographic).
- `...-<style>-goal.png`: in the goal frame (Stance, Butterfly, GloveSave, PadSaveL), with the coverage masks under them.
- `...-<style>-40px.png`: 40 px figures, 3x nearest: skater blue, goalie blue, goalie orange, skater orange. The skater is s4's for p2 and SKF's
  for p3 and p4.
- `...-<style>-motion.png`: 8 frames each of Butterfly_Drop, GloveSave, PadSaveL, PokeCheck and ShuffleL.

Exact names: `20261112-154908-B-lab-skg-p2_cel-turntable`, `154909-...-p2_cel-keys|goal|40px|motion`, `154910-...-p3_clay-turntable|keys|goal`,
`154911-...-p3_clay-40px|motion`, `154911-...-p4_broadcast-turntable|keys`, `154912-...-p4_broadcast-goal|40px|motion`.

**What the sheets showed me, and what I changed (five passes):**
1. The pads sank 6 cm into the ice in stance. The butterfly pads overlapped 1.3 to 6 cm. I raised the pad bottoms, added the ice clamp, set the
   butterfly knee and calf poles, and moved the knee wing flush with the inner pad edge.
2. The pads read as knee-high boxes and the stance was too deep. I raised the stance (pelvis -20 cm, skates forward), lengthened the thigh rise
   to +20 cm above the knee, and lengthened the stick to put the blocker at hip height. Interpolating from the implicit to the explicit knee pole
   snapped the knees together, so I made the stance poles explicit and added the knee-spread pass.
3. Both pad saves looked like the same split. The kick leg now goes out to 80 cm with the body leaning over it, and the other knee drops near
   the centre with its calf tucked. I also gave the shuffle more amplitude.
4. On p3 and p4 the mask took the team-coloured `Helmet` slot and read as a blue helmet, so those masks are now white (`Pad`). The p4 glove's
   blue pocket hid the white cup, so the pocket is now smaller.
5. On p4 the goalie stick floated 1.75 cm and the arms missed their targets by up to 17 cm. The stick now scales with the figure and the hand
   targets scale by reach. On p3 and p4 the butterfly pads floated 3.8 cm, so the butterfly now lowers onto the ice.

## 5. What is weak (judged harshly)

- **Programmer-art kit on p2.** It is 1.3k triangles of boxes. The catching glove is a flat octagon with a dark square, which reads as "a
  screen", not a mitt. The blocker is a plain board. They read correctly at 40 px but look crude at 160 px.
- **Coverage is only about 50 % of the mouth.** The toy goalie is as tall as the crossbar, but the gaps under the arms and between the knees in
  stance stay open (the stance five-hole is about 35 cm by design). The butterfly gains only 1 to 4 points because the torso stays fairly upright
  and the gloves stay at hip height.
- **p3 butterfly has a hole.** From the front there is a small (about 5 cm) see-through hole between the knees just above the stick blade. The
  3D five-hole metric says 0 because the slabs touch at the ice; the mask render shows the hole.
- **Rigid pads ride the calf.** The thigh rise rotates with the shin, so in the stance it juts forward rather than covering the thigh. Pads do not
  rotate on the leg as real ones do; the calf roll fakes it.
- **The knee-spread pass works in discrete steps** (pole nudges of 0.06; up to 12 steps in the p3/p4 butterfly). It can pop a few degrees between
  frames in Butterfly_Drop and Recover. I looked at 8-frame strips, not at every frame.
- **The ice clamp lifts the whole leg pair.** In PokeCheck the body rises about 5 cm at the lunge, because the pads would dip, instead of the legs
  re-posing.
- **IK misses.** Hands miss their targets by up to 9.8 cm in p4 GloveSave, and legs by up to 12.7 cm in p4 PadSaveR. The limbs end straight,
  which looks fine, but the poses were not refit per style.
- **The blocker wrist bends up to about 60 deg** against the forearm, because the hand orientation is absolute. It shows as a kink at the cuff in
  BlockerSave.
- **PokeCheck is foreshortened** from both the shooter's view and the game camera: the stick points at the viewer. The shuffles read weakly:
  feet only, small bob.
- **The motion is pose-to-pose with no secondary motion.** Celebrate_Save is a stiff glove pump.
- **Mask on p2:** the skater's face shows through the cage, which is good, but the dome is large. The p3 and p4 masks ignore a team-tinted
  `Helmet` because they use `Pad`.
- **The 40 px test passes, but only in stance.** The goalie is clearly wider and squarer, with white pads, blocker and mask, and team colour on the
  chest and arms. I did not check other clips at 40 px.

## 6. Rebuild and re-run on another skeleton

```
bash C:/Users/kazda/jam/skater-lab/skg/run_all.sh                     # all three styles, about 3 min
bash C:/Users/kazda/jam/skater-lab/skg/run_all.sh p3_clay             # one style
bash C:/Users/kazda/jam/skater-lab/skg/run_all.sh p4_broadcast --skeleton C:/path/SK_Skater_X.fbx   # any skeleton (.fbx in cm or .blend in m)
bash C:/Users/kazda/jam/skater-lab/skg/run_all.sh all --no-render     # kit, clips, exports and checks only
```

**Steps, in order:**
1. `build_skater.py` (copy of s4's) builds the s4 figure into `base/`.
2. Per style, `build_goalie.py` runs: kit, butterfly fit, 17 takes, measurements to `report_<style>.json`, `preview/goalie_<style>.blend`, the
   piece FBX files, `attach.json`, `kit_reference_bounds.json`, and `export/<style>/SK_Goalie_clips[_loopcut].fbx`.
3. `check_fbx.py` (s4's) runs on both skeletal FBX files.
4. `check_kit.py` runs.
5. `render_sheets.sh` (`render_g.py`) makes the sheets.

The p2 run on the s4 figure is copied to `export/SK_Goalie_clips_loopcut.fbx`, `export/SK_Goalie_clips.fbx` and `export/attach.json`.

**Default skeleton per style:** without `--skeleton`, p2 uses the s4 figure and p3/p4 use `skf/export/<style>/SK_Skater_<style>.fbx` when it exists.

**What another skeleton needs:**
- s4's bone names.
- Rest A-pose with the arms 50 deg down, facing +X.
- An optional body mesh with vertex groups per bone (used for sizing).
- Any mesh with "stick" in its name is dropped.

The butterfly is refit automatically. Each style's `SK_Goalie_clips_loopcut.fbx` carries the takes solved on that style's own skeleton.

## 7. What the engine side needs

**Attach.** Pick one of these two:
- **(a) Sockets on the skeleton, preferred.** Add `goalie_pad_l` (calf_l), `goalie_pad_r` (calf_r), `goalie_blocker` (hand_r), `goalie_glove`
  (hand_l), `goalie_chest` (spine_03), `goalie_arm_l` / `goalie_arm_r` (upperarm_l / upperarm_r) and `goalie_mask` (head), each with zero
  offset and zero rotation. Add `goalie_stick` on `stick_socket` with the stick's relative rotation from `attach.json`. Then attach one
  `UStaticMeshComponent` per piece with `SnapToTargetNotIncludingScale`.
- **(b) Without sockets.** Attach to the bone names directly with the same relative transforms.

Either way, the visual needs four more things:
- Material instances for the new `Pad` slot: white, no team colour, Emissive like `Trim`.
- Collision off, no shadow casting from the small pieces if the frame budget asks.
- **Hide the skater's own stick**: TB1's skeletal mesh has the stick skinned in, slot `Stick`. Either use the stickless body in
  `SK_Goalie_clips.fbx` or give that slot an invisible material.
- The kit adds 9 draws per goalie.

**Caution on axes.** B4 kept s2_socket_xf's rotation alignment on the existing sockets. These new sockets must **not** get that compensation: the
pieces are authored in the raw bone frames. My UE quaternion is the Blender one with x and z negated (the y mirror). This is unverified.

**Clips.** The goalie takes are on the s4 skeleton, with the same bone names and rest pose. They should import anim-only onto `SKEL_SkaterTB1`
(rename `Armature|Goalie_*` to `AS_SkaterTB1_Goalie_*`).

**Which call selects clips (proposal).** `SetGoalieStance(bOn, Side)` is the state:
- bOn true and Side 0 loops `Goalie_Stance`.
- While the pawn moves sideways in the crease, `ShuffleL` / `ShuffleR` run, chosen by the sign of lateral speed relative to facing from
  `UpdateMotion`, blended back to `Stance` when stopped.
- Side ±1 plays `Goalie_Block_Left` / `_Right`, which drops into the pad save and holds (s4 meaning kept).
- bOn false blends back to the skater set.

For the saves I propose **one new cosmetic call**, `PlayGoalieShot(float LocalY, float LocalZ)`. It takes the predicted crossing point of the shot
at the goal line, in the goalie's frame (+Y = its glove side, metres), and the visual picks the clip:

| Shot crossing point | Clip |
|---|---|
| LocalZ < 0.25 and abs(LocalY) < 0.35 | `Butterfly_Drop`, then `Butterfly_Hold`, then `Butterfly_Recover` |
| LocalZ < 0.35 and abs(LocalY) >= 0.35 | `PadSaveL` / `PadSaveR` |
| LocalZ >= 0.35 and LocalY > 0 | `GloveSave` |
| LocalZ >= 0.35 and LocalY <= 0 | `BlockerSave` |

Events from C-0003:
- **`shot`** (at this goalie's net): call `PlayGoalieShot`.
- **`shot_blocked` / a `save`** (Who = the goalie): `Goalie_Freeze` if the puck stops at the goalie (held until the faceoff), otherwise
  `Celebrate_Save` on every Nth save or a period-ending save.
- **`poke`** by the goalie pawn: `Goalie_PokeCheck`.
- **`goal`** against: `Goalie_Beaten`, held until the faceoff.

There is no `save` event in C-0003 today. `shot_blocked` (R-0019) is the nearest; a `save` kind is needed if blocks by the goalie should differ
from zone blocks.

**A ruling is needed first.** `docs/HOCKEY.md` says "no goalies", and R-0013 point 3 says "last man as goalie ... No goalie figure, no new role".
This lane's brief says each team has a goalie in the net. If the goalie stays the "last man" skater, the kit would pop on and off at
`SetGoalieStance(bOn)`, which will look odd. A dedicated goalie pawn, or a decision to show the kit only on a designated goalie, is the
Director's call.

## 8. What I could not verify

- Anything in Unreal: static-mesh import of the pieces (cm, axes as the skater), my bone-space to UE quaternion conversion for the stick, socket
  creation, anim-only import onto `SKEL_SkaterTB1`, the toon material on `Pad`, and draw cost.
- That SKF's figures are final. I fitted p3 and p4 to their 15:3x exports. A re-run refits them.
- Frame-by-frame motion: I saw the key frames and 8-frame strips. The knee-spread pops are unverified by eye.
- The game camera is approximated: front, about 20 deg down, orthographic. Coverage is a silhouette count, not the simulation's blocking (the sim
  does not use meshes).
- IK miss per limb: the tables give the largest miss over the whole pose.
