# Skater lab SKF: the scripted skater family (p2_cel, p3_clay, p4_broadcast), report at 15:31

Lane SKF, device B, Blender 4.2 headless only. No Unreal, no git, no network, no credits. Nothing under `skater-lab/s4/` was changed (its newest file is still from 14:13).
Lab folder: `C:\Users\kazda\jam\skater-lab\skf\`.

**State: all three styles are done.** Each has a mesh and rig, all 31 takes, both FBX variants, a stick FBX, re-import checks and sheets.
Each new style got at least two look passes (P3 two, P4 three). Everything Unreal-side is **unverified until an import lane takes it**.

## What was built

- **One builder, three styles.** Run it as `build_skater.py -- <style>`. The proportions, colours and clip-adaptation factors live in one table, `skf_style.py`.
  `build_p2()`, `build_p3()` and `build_p4()` place every part relative to the joint heights from that table.
- **The same skeleton for all three.** The bone list and hierarchy are identical (checked on the re-imported files): 24 bones, a real `root` with an identity
  rest matrix, and the sockets `stick_socket`, `puck_socket` and `head_socket` as bones. The nine material slot names are identical:
  `Jersey Trim Helmet Gloves Skin Pads Skate Blade Stick`.
- **Clips: one set of pose tables (s4's) feeds every style.** `build_clips.py -- <style>` rescales the table positions by the style's proportions,
  solves feet and hands by IK, solves the aim and poke reach, then bakes, self-checks and exports.
- **A one-command rebuild**, about 45 s for all three styles in parallel. It renders from the re-imported FBX, not from the source scene.

## Proportions table (`skf_style.py`; metres)

| Parameter | p2_cel | p3_clay | p4_broadcast |
|---|---|---|---|
| Height (measured, rest) | 1.732 | 1.637 | 1.823 (about 7 heads) |
| ankle / knee / hip joint | 0.13 / 0.49 / 0.85 | 0.14 / 0.47 / 0.80 | 0.12 / 0.52 / 0.93 |
| hip half-width | 0.115 | 0.13 | 0.105 |
| pelvis / spine_01 / spine_02 / spine_03 / neck / head joint | .88/.98/1.08/1.17/1.30/1.37 | .83/.93/1.02/1.11/1.24/1.30 | .96/1.07/1.18/1.29/1.50/1.56 |
| shoulder height / half-width | 1.27 / 0.215 | 1.20 / 0.235 | 1.44 / 0.20 |
| upper arm / forearm | 0.29 / 0.26 | 0.26 / 0.23 | 0.31 / 0.27 |
| head scale (about the neck) | 1.10 (toy head) | 1.0 (big head modelled directly) | 1.0 |
| stick, grip to heel / blade length | 1.40 / 0.34 | 1.40 / 0.34 | 1.46 / 0.33 |
| skate blade, heel / toe from the ankle (ice clamp) | -0.115 / +0.195 | -0.115 / +0.20 | -0.12 / +0.19 |
| **kx** leg scale: skate targets x, y, lift, lying pelvis drop | 1.0 | 0.917 | 1.125 |
| **kz** crouch scale: pelvis drop in standing poses | 1.0 | 0.917 | **1.30** (long legs read too upright at 1.125) |
| **kr** reach scale: stick heel, grip and hand targets | 1.0 | 0.961 | 1.048 |
| **stance**: extra skate width each side | 0 | **0.035** (fat thighs) | 0 |
| **lie_add**: pelvis raised in lying poses | 0 | **0.07** (thicker body; fixes a 3.9 cm sink in `Fall`) | 0 |
| lunge (far-reach and poke pelvis drop multiplier) | 1.0 | 1.15 | 0.9 |

The style fixes for poses that broke are all in this table: `kz` for P4's crouch, `stance` and `lie_add` for P3. No clip was edited by hand
for one style. One key of the shared table changed for every style (the slap downswing, see "Hands on the stick").

## Triangles, bones, materials

| | p2_cel | p3_clay | p4_broadcast |
|---|---|---|---|
| Triangles (body + stick) | **6,746** (6,550 + 196); cap 7k | **11,968** (11,556 + 412); cap 12k | **9,636** (9,396 + 240); cap 14k |
| Bones | 24, same names and hierarchy | same | same |
| Rest bone heads (cm): pelvis, head, hand_r, stick_socket, head_socket | 88; 137; (0,-56.9,84.9); (0,-61,79.9); 186 | 83; 130; (0,-55,82.5); (0,-59.5,77.1); 179 | 96; 156; (0,-57.3,99.6); (0,-62.1,93.8); 194 |
| `puck_socket` | (78, 10, 0) cm in all three | | |
| Textures | 0 | 0 | 0 |
| Team-colour slots (tinted in the orange renders) | Jersey | Jersey, Helmet | Jersey, Helmet, Gloves, Pads |

**Bone list differences: none.** Only the lengths differ (cm):

| Bone | p2_cel | p3_clay | p4_broadcast |
|---|---|---|---|
| pelvis / spine_01 / spine_02 / spine_03 | 10 / 10 / 9 / 13 | 10 / 9 / 9 / 13 | 11 / 11 / 11 / 21 |
| neck_01 / head | 7 / 25 | 6 / 25 | 6 / 24 |
| clavicle_l, _r | 15 | 17 | 13.5 |
| upperarm / lowerarm | 29 / 26 | 26 / 23 | 31 / 27 |
| thigh / calf / foot | 36 / 36 / 17.9 | 33 / 33 / 18.4 | 41 / 40 / 18.4 |

## The looks (what each pass changed)

- **p2_cel**: the s4 figure plus the items its reports listed as weak.
  - **Face**: a round head with a button nose, white eyes with dark pupils, friendly brows (outer ends raised) and a smile.
  - **Cage**: an open cage with two horizontal bars below the eyes, side bars and a short centre bar, so the face reads.
  - **Helmet**: the shell is opened higher at the front, with a low centre ridge, ear guards and a chin cup.
  - **Gloves**: mitten gloves with a thumb.
  - **Passes**: pass 1 built all of the above; pass 2 replaced angry brows, a hidden smile and a fin-like ridge.
- **p3_clay**: the same toy made of clay. Rounder and chunkier, every form a superellipsoid or capsule.
  - **Surface**: every part gets a seeded two-octave low-frequency displacement plus thumb-press dimples (smooth bowls, about one per 70 cm²)
    baked into the vertices, with smooth normals.
  - **Rolled trims**: fat cream clay rolls at the hem, collar, sleeves and socks, and a cuff roll on the mittens.
  - **Stitched seams**: a raised ridge with cream cross-stitches as geometry, around the collar, above the hem and down both sides.
  - **Back**: a raised felt number patch with a dark running stitch round it, and no digit.
  - **Head and helmet**: a big clay head with a nose, cheeks, eyes, brows and a smile, under a lumpy helmet with vent ridges, brim and ear guards.
  - **Skates**: coiled-clay blades (one rolled snake that curls up and over at the toe) and clay boots.
  - **Passes**: pass 1 was 16.6k triangles; pass 2 cut resolution to 11,968, strengthened the lumps and dents, and added the close-up row.
- **p4_broadcast**: 1.82 m, about 7 heads, real gear at a glance.
  - **Head**: a helmet with a visor, ear guards, vent grooves and a chin strap over a small face.
  - **Torso**: wide shoulder pads under the jersey, which reads as one shoulder line. A loose jersey hem with a white stripe hangs over boxy padded breezers.
  - **Arms**: loose sleeves with two stripes, elbow pads and big gauntlet gloves with a thumb.
  - **Legs and skates**: shin-guard knee caps under striped socks, and skates with a toe cap, tendon guard, white holders and a steel blade.
  - **Joints and materials**: ball joints are hidden under the shoulder caps, elbow pads and breezer shells. Cloth is matte; the helmet, visor and boots are glossy.
  - **Passes**: pass 1 read as a blue robot (short jersey, bulbous pants, knob shoulders). Pass 2 brought dark breezers, a long hem, integrated
    shoulders, looser sleeves and a deeper crouch. Pass 3 replaced tube pants with boxy breezers, made the cloth matte and the nose smaller,
    set `kz` to 1.30, and moved the eyes off the team-tinted `Pads` slot.

## Measured stick reaches (R-0018: 1.0 m aims, 1.6 m far reach and poke tip; tolerance 5 cm)

The reach is measured on the **baked bones**, not on the solver: the horizontal distance from the pelvis to the blade centre.

| Pose | p2_cel | p3_clay | p4_broadcast |
|---|---|---|---|
| Aim_Front (target 1.0) | 1.000 m, +1.3 deg, blade centre 5.1 cm up | 1.000 m, +1.3 deg, 5.1 cm | 1.000 m, +1.2 deg, 5.2 cm |
| Aim_Left (1.0) | 1.000 m, +66.3 deg, 5.1 cm | 1.000 m, +66.3 deg, 5.2 cm | 1.000 m, +66.3 deg, 5.2 cm |
| Aim_Right (1.0) | 1.000 m, -63.6 deg, 5.2 cm | 1.000 m, -63.7 deg, 4.5 cm | 1.000 m, -63.6 deg, 4.7 cm |
| Aim_FarReach (1.6) | 1.600 m, +0.3 deg, 9.4 cm; right arm at 0.77 of its length | 1.600 m, 9.4 cm; arm 0.73 | 1.600 m, 8.9 cm; arm 0.83 |
| Poke tip, frame 10 (1.6) | 1.600 m, 9.4 cm; arm 0.76 | 1.600 m, 9.4 cm; arm 0.73 | 1.600 m, 9.4 cm; arm 0.87 |

How the reach is reached:
- **The solver.** It moves the blade heel along the aim direction until the blade centre sits at the target (within 1 mm). It also picks the shaft tilt: the
  flattest blade with a reachable grip for two-handed aims, and an arm held out at about 0.86 of its length for one-handed reaches.
- **The far reach and the poke.** They are one-handed at the knob, with a front-knee lunge: the left skate 34 to 38 cm forward, pelvis down, chest over the knee.
- **What I found about the stick.** At these proportions the stick did not need lengthening for 1.6 m. With a 1.40 m stick a deep lunge folds the arm,
  because the knob ends at the shoulder: the first attempt measured an arm extension of 0.17. The lunge was therefore made moderate. P4's stick was
  *shortened* from 1.52 to 1.46 m because at 1.52 the two-handed 1.0 m aims put the knob out of reach.
- **Arms are never stretched.** The two-bone IK clamps, and a reach guard (below) pulls the knob into reach.
- **Blade height.** The heel is on the ice in every aim. In the one-handed reaches the toe is lifted: the blade centre is 9 cm up, because the shaft
  is tilted 4 to 10 degrees past the blade lie.

## Hands on the stick, and what the new metrics found

s4 never measured hand contact. Measuring it showed that on s4's own tables (so in P2 as well), the right hand could not reach the knob
the pose asked for in about 10 takes: the stick then hung up to 28 cm off its target. The bottom hand also left the shaft by up to 44 cm, worst in the slap
downswing. New solver rules, applied to every style:

1. **Reach guard.** When the knob is out of reach, the shaft swings about the blade heel until the knob is reachable, and the stick is lifted if a
   blade point would go under the ice. "Knob moved" in the tables is how far this moved it (up to 53 cm in P3 `Faceoff_Draw`).
2. **Grip correction.** The wrist target is iterated until the glove's grip point (along the solved forearm) lands on the knob or on its point of the shaft.
3. **Bottom hand.** The left hand slides along the shaft to the nearest reachable point. If none is reachable, the left clavicle turns up to 35 degrees
   (the player reaches with the shoulder).
4. **One shared key changed**: `Shot_Slap` frame 19, the downswing start, now uses grip (0.04, 0.42, 0.82), yaw 130, tilt 120. s4's key left the bottom
   hand 32 cm off the shaft.

Result: the bottom hand is at most 0.2 cm off the shaft in every two-handed take except `Shot_Slap` (2.7 / 3.9 / **9.3** cm for P2 / P3 / P4) and
`Shot_Wrist` (3.3 / 3.3 / **7.5** cm). In those takes the left shoulder is at its 35-degree limit. The right-arm extension stays at 1.0 or below except P3
`Faceoff_Draw`, `Faceoff_Ready` and `GetUp`, and P4 `Faceoff_Draw` (1.04 to 1.33). There the IK clamps and the knob ends at most 2.7 cm short of its
target; the hand still holds the knob, because the stick is the hand's child. In the arms-spread poses (`Stumble`, `Foul_Reaction`, `Fall`) the stick sits
10 to 14 cm from where the table put it, in the air.

## Measurements: all 31 takes per style (re-imported FBX, both variants)

These hold for all three styles, both files, all 31 takes:
- **Pelvis horizontal travel and root movement: 0.000 cm.**
- **Loop seams (main file): 0.000 cm.** Loopcut shows one frame of motion: 0.004 cm in holds, 0.63 to 0.79 cm in Stride, 1.3 to 1.6 cm in the crossovers.
- **Bones and root**: 24 bones in every take; `root` rest matrix identity; armature object `diag(0.01)` (cm file read back as metres, as for s4 after the export fix); unit scale 1.

The full per-take tables are at the end of this report.

Lowest points:
- **Skates**: 0.0 to 0.25 cm in every take. The exceptions are the carving leans, which float 1.1 to 1.2 cm (s4's conservative edge clamp, unchanged).
- **Stick, where the pose puts the blade on the ice**: aims 0.5 to 1.1 cm, Poke 0.05 to 0.35, Faceoff_Ready 0.1 to 0.2, Glide 0.7 to 1.1.
- **Stick into the ice**: P2 `Check` -0.6, `Crossover_Right` -0.8, `Stride` -0.7, `GetUp_Front` -1.4. P3 down to -1.4 in `GetUp_Front`. P4 down to -0.4.
- **Body**: P2 and P3 `GetUp_Front` -1.4 cm (the chest at frame 0, inherited from s4's `Fall_Forward` end). P3 `Fall` is fixed by `lie_add`
  (it was -3.9 cm, now +0.14).

## Rubric rows (judged harshly)

| Candidate | Look fit | Technical | Motion | Cost | Provenance | Import risk |
|---|---|---|---|---|---|---|
| SKF p2_cel | **3.5**: same clean read as T-B1 at 40 px, now with a face that reads close up (eyes, brows, nose, smile under an open cage), mitten gloves and a proper helmet. Still primitive boxes and tubes; programmer-toy charm. | **5**: 6.7k tris, 0 textures, 24 mannequin-named bones, identity root, A-pose, rigid weights. | **4**: as T-B1, plus measured hand contact and exact R-0018 reaches. Pose-to-pose; slap bottom hand 2.7 cm off; far-reach blade toe up 9 cm. | **5**: 0 credits, 45 s rebuild of all three. | **5**: original | **2.5**: the skeleton changed (the s4 head_socket rule, new hands and face), so it is a re-import of T-B1, not a drop-in. Same FBX settings as the s4 export after its fix, which B4 never imported (B4 imported the file before the fix). |
| SKF p3_clay | **3.5**: reads as a hand-made clay toy under matcap + cavity: lumps, dents, rolled trims, stitched seams, felt back patch, coiled blades. The clay character is in the geometry, so it survives any shader, but the fingerprint and felt read needs the Unreal material; without it, it is a lumpy matte toy. At 40 px the cream back patch is a bright square. | **4.5**: 12.0k tris at the cap, 0 textures, same skeleton. | **4**: same clips; the wider stance and lie height fixed in the table; no limb-intersection measurement. | 5 | 5 | **3**: the dense noisy normals need checking under the cel and post outline; the patch is a mesh, not a UV decal. |
| SKF p4_broadcast | **3**: reads as a hockey player in gear (visor helmet, pads, loose jersey, breezers, socks, holders) and as a team at 40 px. Still clearly low-poly: blobby shoulders and gloves, sleeves made of capsules, no cloth folds, a small simple face. It would sit in a broadcast-look arena as a stylised sports figure, not a realistic one. | **5**: 9.6k tris, same skeleton, 0 textures. | **3.5**: proportions are right and the deeper crouch reads, but the rigid parts show at the elbow and knee in the stride side view; slap and wrist bottom hand 7 to 9 cm off the shaft. | 5 | 5 | **3**: four team-tinted slots (Jersey, Helmet, Gloves, Pads) need four dynamic material instances; the visor is opaque Blade grey. |

## Verdict

- **p2_cel: queue for import as T-B1 v2.** It is the shipped look, with a face, hands and a better helmet at +0.8k triangles. Same clip names; reaches now match R-0018.
- **p3_clay: keep, and queue if the operator picks the P3 tier.** It is the one asset that makes the clay tier believable without texture work.
- **p4_broadcast: keep as the P4 placeholder.** It reads as a player and moves as well as T-B1, but it is the weakest look of the three against
  the generated broadcast stills.

## Sheets (filed with `devlog/shot.mjs`)

- `devlog/shots/20261112-152846-B-lab-skf-p2_cel-turntable.png`: rest, Glide, close-up (8 views each)
- `devlog/shots/20261112-152846-B-lab-skf-p2_cel-stride-slap.png`: Stride 3/4 front, side, directly behind; Shot_Slap from the side
- `devlog/shots/20261112-152846-B-lab-skf-p2_cel-distance.png`: about 40 px from the chase angle, blue and orange Stride, orange slap
- `devlog/shots/20261112-152846-B-lab-skf-p2_cel-reach.png`: Aim_Front, Aim_FarReach, Poke tip
- `devlog/shots/20261112-152846-B-lab-skf-p3_clay-turntable.png`, `-152847-...-p3_clay-stride-slap.png`, `-152847-...-p3_clay-distance.png`, `-152847-...-p3_clay-reach.png`
  (matcap + cavity)
- `devlog/shots/20261112-152847-B-lab-skf-p4_broadcast-turntable.png`, `-152847-...-p4_broadcast-stride-slap.png`, `-152847-...-p4_broadcast-distance.png`,
  `-152848-...-p4_broadcast-reach.png`
- `devlog/shots/20261112-153047-B-lab-skf-compare-final.png`: the three styles side by side: Glide frame 0 (3/4 front) and the Shot_Slap wind-up top
  (frame 16, side). An earlier copy, `-152848-...-compare.png`, shows P4 before its eye-slot fix (it looks the same).

The P3 lying fix and the P4 eye-slot fix came after the per-style sheets were filed. Neither changes what those sheets show.

## Rebuild

```
bash C:/Users/kazda/jam/skater-lab/skf/run_all.sh                 # all three in parallel, then the comparison sheet (about 45 s)
bash C:/Users/kazda/jam/skater-lab/skf/run_all.sh p3_clay         # one style
```

Step by step for one style (`B="C:/Program Files/Blender Foundation/Blender 4.2/blender.exe"`, `S=C:/Users/kazda/jam/skater-lab/skf`):

```
"$B" -b --factory-startup --python $S/build_skater.py -- p4_broadcast $S       # work/p4_broadcast/skater.blend, build.json, export/p4_broadcast/SM_Stick_p4_broadcast.fbx
"$B" -b --factory-startup --python $S/build_clips.py  -- p4_broadcast $S       # anim.blend, clips_report.json, SK_Skater_p4_broadcast.fbx and _loopcut.fbx
"$B" -b --factory-startup --python $S/check_fbx.py -- $S/export/p4_broadcast/SK_Skater_p4_broadcast.fbx $S/work/p4_broadcast/check_fbx.json
"$B" -b --factory-startup --python $S/render_skf.py -- <fbx> <out_dir> p4_broadcast <name:mode:action:n:size[:tint[:frame]]> ...
node $S/summary.mjs p4_broadcast all                                            # the take table, from the JSON files
```

Outputs:
- `export/<style>/SK_Skater_<style>.fbx`, `SK_Skater_<style>_loopcut.fbx` and `SM_Stick_<style>.fbx`
- `work/<style>/` (blends, `build.json`, `clips_report.json`, `check_fbx*.json`), `r/<style>/` (frames), `sheets/`, `logs/`

`s4_*_orig.*` are untouched copies of the s4 scripts, kept for reference.

## What an importer must know (differences from the T-B1 import)

- **One skeleton asset per style.** The bone names and hierarchy are identical, but the reference poses differ, so each FBX imports with its own
  skeleton: `SKEL_Skater_P2/P3/P4`, `SK_Skater_P2/P3/P4`. Each file carries its own 31 takes, solved for its own proportions. Import every style
  with its own takes; do not play P2's takes on P4 (the skates would float or sink and the reaches would be wrong).
- **One AnimBP structure serves all three.** The clip names, the state machine, variables, blend space axes and montage slots are identical.
  - Make the T-B1 AnimBP a template, or duplicate it, and make one child per skeleton with asset overrides.
  - Or mark the three skeletons as compatible skeletons. Even then, use each style's own sequences.
  - `BP_SkaterVisual_TB1` logic carries over unchanged, apart from the material instances.
- **The export is the post-fix s4 recipe, which B4 never imported.**
  - Centimetres in the data, `UnitScaleFactor` 1, an identity `Armature` object, `root` lying along +Y with an identity rest.
  - `axis_forward -Z`, `axis_up Y`, no leaf bones, 60 fps, one take per action. The track-strip step is **off**, so `root` carries identity keys.
  - Expected in Unreal: root rotation 0, scale 1, so the sockets need **no** compensation (B4's `s2_socket_xf.py` should not be applied). Unverified.
- **Faces +X**, as before: no -90 yaw on the mesh component.
- **Heights differ**: 173 / 164 / 182 cm. `head_socket` sits at 186 / 179 / 194 cm and `puck_socket` at (78, 10, 0) in all three. Sockets are bones;
  add `SkeletalMeshSocket`s with the same names, as B4 did.
- **Material slots**: the same nine names, but the order may differ per style. Assign by name, not by index; B4's Blueprint used indices 1 and 4.
  - Team colour: P2 `Jersey`; P3 `Jersey` + `Helmet`; P4 `Jersey`, `Helmet`, `Gloves`, `Pads` (breezers).
  - P2 and P3 eye dots are in `Pads`, so do not team-tint `Pads` there. P4's eyes are in `Skate`.
- **`SM_Stick_<style>.fbx` is in centimetres** (s4's was in metres): pivot at the grip, shaft along -Z, `FBX_SCALE_NONE`. It is optional, because the
  stick is skinned into the main mesh. Stick lengths: 1.40 / 1.40 / 1.46 m from grip to heel.
- **Clip changes against T-B1**:
  - The aims now hit the R-0018 reaches exactly: 1.0 / 1.0 / 1.0 / 1.6 m. Before they were 0.97 / 0.96 / 0.98 / 1.51.
  - `Poke` frame 10 is a 1.6 m one-handed lunge; before it was about 1.3 m.
  - `Shot_Slap` frame 19 changed. `Shot_Charge` is still identical to `Shot_Slap` frame 16.
  - Hands stay on the stick in more frames, so the stick path differs slightly in about 10 takes.
  - Frame counts and names are unchanged. All holds are 30-frame loops with the wobble.
- **No GLB this time** (FBX only).

## What is weak

- **P2**: still primitive (boxes, tubes, ball joints); the face is cute only up close. The helmet ridge shows as a thin line from behind.
- **P3**:
  - The clay read depends on matcap + cavity; under the game's cel ramp and outline the dents may vanish unless the material keeps cavity or AO.
  - Fingerprint micro-detail and felt fuzz are not modelled.
  - At 12.0k it is at its cap. The cheek balls read as pale dots.
  - Limb intersection was not measured (only the stance was widened).
- **P4**:
  - Capsule sleeves and blob gloves look low-poly close up; the elbow and knee joints show when bent; no cloth folds; the face is minimal.
  - The visor is opaque grey; the shoulder pads still show as bumps; the stride from behind looks like sitting in a deep squat.
  - The bottom hand is 7 to 9 cm off the shaft in the slap and wrist shots.
- **All styles**:
  - Pose-to-pose motion with no secondary motion. The far reach and poke lift the blade toe (centre 9 cm up).
  - The leans float the skates 1.2 cm. `GetUp_Front` starts 1.4 cm into the ice.
  - The reach sheets' side camera is too close to show the whole 1.6 m stick (the numbers are measured on the bones instead).
- Renders are Workbench stand-ins (outline and studio light for P2; matcap and cavity for P3; studio light with specular, cavity and outline for P4).
  They are not the game's shaders.

## What I could not verify

- Anything in Unreal: the import with the post-fix export (root, scale, sockets), slot order, three skeletons with one AnimBP structure,
  the cel shader on P3's dented normals, frame cost (12k triangles times 6 is still small).
- Limb and body interpenetration (only the arm and stick reach and ice contact are measured), and arms passing through the helmet (s4 measured
  that; SKF did not re-measure it for the new head shapes).
- That P3 reads as clay under the real P3 material and lighting rather than Workbench matcap.

## Appendix: the 31-take tables per style (re-imported main FBX; loopcut frame counts and seams from the loopcut FBX)

"Right arm ext. max" is the largest wrist distance asked of the right arm, as a fraction of its length; above 1.0 the IK clamps and the hand ends short. "Bottom hand off shaft" covers two-handed frames only. "Left shoulder reach" is the clavicle turn used to reach the shaft (cap 35 deg).

#### p2_cel (pelvis x,y max over all takes, both files: 0 cm; root movement max: 0 cm)

| Take | Frames main / loopcut | Kind | Seam main / loopcut (cm) | Lowest skate / stick / any (cm) | Pelvis z range (cm) | Right arm ext. max | Bottom hand off shaft max (cm) | Knob moved by reach guard (cm) | Left shoulder reach (deg) |
|---|---|---|---|---|---|---|---|---|---|
| Aim_FarReach | 31 / 30 | loop | 0 / 0.004 | 0 / 0.48 / 0 | 0.05 | 0.774 | one hand | 0 | 0 |
| Aim_Front | 31 / 30 | loop | 0 / 0.004 | 0 / 0.65 / 0 | 0.05 | 0.765 | 0.09 | 0 | 0 |
| Aim_Left | 31 / 30 | loop | 0 / 0.004 | 0 / 0.63 / 0 | 0.05 | 0.476 | 0.17 | 0 | 0 |
| Aim_Right | 31 / 30 | loop | 0 / 0.004 | 0 / 0.75 / 0 | 0.05 | 0.918 | 0.06 | 0 | 0 |
| Celebrate | 73 / 73 | one-shot | n/a | 0 / 0.73 / 0 | 10 | 0.885 | 0.2 | 0 | 0 |
| Check | 31 / 31 | one-shot | n/a | 0 / -0.64 / -0.64 | 16 | 0.885 | 0.07 | 0 | 0 |
| Crossover_Left | 41 / 40 | loop | 0 / 1.402 | 0.23 / 4.37 / 0.23 | 4 | 0.758 | one hand | 0 | 0 |
| Crossover_Right | 41 / 40 | loop | 0 / 1.402 | 0.23 / -0.79 / -0.79 | 4 | 0.758 | one hand | 0 | 0 |
| Faceoff_Draw | 19 / 19 | one-shot | n/a | 0 / 0.12 / 0 | 9 | 1.007 | 0.19 | 38.4 | 13.7 |
| Faceoff_Ready | 31 / 30 | loop | 0 / 0.004 | 0.04 / 0.08 / 0.04 | 0.05 | 0.992 | 0.09 | 28.5 | 0 |
| Fall | 37 / 37 | one-shot | n/a | 0 / 0.73 / 0 | 55.5 | 0.885 | 0.07 | 0 | 0 |
| Fall_Forward | 37 / 37 | one-shot | n/a | 0 / 0.73 / 0 | 53.5 | 0.885 | 0.07 | 0 | 0 |
| Foul_Reaction | 49 / 49 | one-shot | n/a | 0 / 0.73 / 0 | 7 | 0.885 | 0.07 | 0 | 0 |
| GetUp | 73 / 73 | one-shot | n/a | 0 / -0.24 / -0.24 | 51.5 | 0.973 | 0.07 | 0 | 0 |
| GetUp_Front | 73 / 73 | one-shot | n/a | 0 / -1.36 / -1.36 | 51.5 | 0.986 | 0.07 | 10.5 | 0 |
| Glide | 121 / 120 | loop | 0 / 0.004 | 0 / 0.73 / 0 | 1 | 0.89 | 0.08 | 0 | 0 |
| Goalie_Block_Left | 25 / 25 | one-shot | n/a | 0.2 / 16.49 / 0.2 | 18 | 0.986 | one hand | 7.2 | 0 |
| Goalie_Block_Right | 25 / 25 | one-shot | n/a | 0.2 / 13.84 / 0.2 | 18 | 0.986 | one hand | 6.3 | 0 |
| Goalie_Hold | 31 / 30 | loop | 0 / 0.004 | 0.2 / 19.57 / 0.2 | 0.05 | 0.986 | one hand | 6.3 | 0 |
| Idle_Faceoff | 121 / 120 | loop | 0 / 0.004 | 0.04 / 0.11 / 0.04 | 1.2 | 0.988 | 0.08 | 17 | 0 |
| Lean_Left | 31 / 30 | loop | 0 / 0.004 | 1.2 / 5.81 / 1.2 | 0.05 | 0.758 | one hand | 0 | 0 |
| Lean_Right | 31 / 30 | loop | 0 / 0.004 | 1.2 / 0.69 / 0.69 | 0.05 | 0.758 | one hand | 0 | 0 |
| Lift | 19 / 19 | one-shot | n/a | 0 / 0.64 / 0 | 3 | 0.985 | 0.19 | 18.5 | 0 |
| Pass | 19 / 19 | one-shot | n/a | 0 / 0.13 / 0 | 2 | 0.985 | 0.1 | 18.1 | 7.7 |
| Poke | 22 / 22 | one-shot | n/a | 0 / 0.05 / 0 | 5 | 0.981 | 0.16 | 28 | 0 |
| Shot_Charge | 31 / 30 | loop | 0 / 0.004 | 0 / 82.65 / 0 | 0.05 | 0.838 | 0.09 | 0 | 0 |
| Shot_Slap | 43 / 43 | one-shot | n/a | 0 / 0.81 / 0 | 6 | 0.986 | 2.74 | 10.8 | 35 |
| Shot_Wrist | 22 / 22 | one-shot | n/a | 0 / 0.04 / 0 | 3 | 0.986 | 3.32 | 8.2 | 35 |
| Stop | 37 / 37 | one-shot | n/a | 0 / 0.52 / 0 | 15 | 0.885 | 0.12 | 0 | 0 |
| Stride | 61 / 60 | loop | 0 / 0.632 | 0.01 / -0.68 / -0.68 | 4 | 0.843 | one hand | 0 | 0 |
| Stumble | 21 / 21 | one-shot | n/a | 0 / 0.73 / 0 | 8 | 0.885 | 0.07 | 0 | 0 |

#### p3_clay (pelvis x,y max over all takes, both files: 0 cm; root movement max: 0 cm)

| Take | Frames main / loopcut | Kind | Seam main / loopcut (cm) | Lowest skate / stick / any (cm) | Pelvis z range (cm) | Right arm ext. max | Bottom hand off shaft max (cm) | Knob moved by reach guard (cm) | Left shoulder reach (deg) |
|---|---|---|---|---|---|---|---|---|---|
| Aim_FarReach | 31 / 30 | loop | 0 / 0.004 | 0.14 / 0.49 / 0.14 | 0.05 | 0.73 | one hand | 0 | 0 |
| Aim_Front | 31 / 30 | loop | 0 / 0.004 | 0.14 / 0.88 / 0.14 | 0.05 | 0.802 | 0.11 | 0 | 0 |
| Aim_Left | 31 / 30 | loop | 0 / 0.004 | 0.14 / 0.96 / 0.14 | 0.05 | 0.466 | 0.08 | 0 | 0 |
| Aim_Right | 31 / 30 | loop | 0 / 0.004 | 0.14 / 0.93 / 0.14 | 0.05 | 0.925 | 0.15 | 0 | 0 |
| Celebrate | 73 / 73 | one-shot | n/a | 0.14 / 0.94 / 0.14 | 9.17 | 0.97 | 0.2 | 0 | 3.7 |
| Check | 31 / 31 | one-shot | n/a | 0.14 / -0.42 / -0.42 | 14.67 | 0.97 | 0.1 | 0 | 3.7 |
| Crossover_Left | 41 / 40 | loop | 0 / 1.285 | 0.22 / 1.85 / 0.22 | 3.67 | 0.8 | one hand | 0 | 0 |
| Crossover_Right | 41 / 40 | loop | 0 / 1.285 | 0.23 / -0.47 / -0.47 | 3.67 | 0.8 | one hand | 0 | 0 |
| Faceoff_Draw | 19 / 19 | one-shot | n/a | 0.14 / -0.37 / -0.37 | 8.25 | 1.328 | 0.2 | 53.5 | 14.4 |
| Faceoff_Ready | 31 / 30 | loop | 0 / 0.004 | 0.14 / -0.21 / -0.21 | 0.05 | 1.204 | 0.17 | 45 | 0 |
| Fall | 37 / 37 | one-shot | n/a | 0.14 / 0.94 / 0.14 | 43.88 | 0.97 | 0.1 | 0 | 3.7 |
| Fall_Forward | 37 / 37 | one-shot | n/a | 0.14 / 0.94 / 0.14 | 42.04 | 0.97 | 0.1 | 0 | 3.7 |
| Foul_Reaction | 49 / 49 | one-shot | n/a | 0.14 / 0.94 / 0.14 | 6.42 | 0.97 | 0.1 | 0 | 3.7 |
| GetUp | 73 / 73 | one-shot | n/a | 0.14 / -0.73 / -0.73 | 40.21 | 1.24 | 0.1 | 30.6 | 3.7 |
| GetUp_Front | 73 / 73 | one-shot | n/a | 0.14 / -1.38 / -1.38 | 40.21 | 0.999 | 0.1 | 27.2 | 3.7 |
| Glide | 121 / 120 | loop | 0 / 0.004 | 0.14 / 0.94 / 0.14 | 0.92 | 0.973 | 0.12 | 0 | 6 |
| Goalie_Block_Left | 25 / 25 | one-shot | n/a | 0.25 / 18.64 / 0.25 | 16.5 | 0.987 | one hand | 9.6 | 0 |
| Goalie_Block_Right | 25 / 25 | one-shot | n/a | 0.25 / 15.54 / 0.25 | 16.5 | 0.987 | one hand | 8.3 | 0 |
| Goalie_Hold | 31 / 30 | loop | 0 / 0.004 | 0.25 / 20.19 / 0.25 | 0.05 | 0.987 | one hand | 8.3 | 0 |
| Idle_Faceoff | 121 / 120 | loop | 0 / 0.005 | 0.14 / 0.43 / 0.14 | 1.1 | 1.04 | 0.17 | 32 | 0 |
| Lean_Left | 31 / 30 | loop | 0 / 0.004 | 1.19 / 3.16 / 1.19 | 0.05 | 0.8 | one hand | 0 | 0 |
| Lean_Right | 31 / 30 | loop | 0 / 0.004 | 1.19 / 1.01 / 1.01 | 0.05 | 0.8 | one hand | 0 | 0 |
| Lift | 19 / 19 | one-shot | n/a | 0.14 / 0.9 / 0.14 | 2.75 | 0.986 | 0.17 | 28.2 | 5.2 |
| Pass | 19 / 19 | one-shot | n/a | 0.14 / 0.35 / 0.14 | 1.83 | 0.982 | 0.15 | 26.7 | 7.7 |
| Poke | 22 / 22 | one-shot | n/a | 0.14 / 0.06 / 0.06 | 6.92 | 0.992 | 0.14 | 39.4 | 3.7 |
| Shot_Charge | 31 / 30 | loop | 0 / 0.003 | 0.14 / 77.95 / 0.14 | 0.05 | 0.895 | 0.13 | 0 | 0 |
| Shot_Slap | 43 / 43 | one-shot | n/a | 0.14 / 0.24 / 0.14 | 5.5 | 0.987 | 3.91 | 18.3 | 35 |
| Shot_Wrist | 22 / 22 | one-shot | n/a | 0.14 / 0.41 / 0.14 | 2.75 | 0.986 | 3.31 | 13.9 | 35 |
| Stop | 37 / 37 | one-shot | n/a | 0.14 / 0.73 / 0.14 | 13.75 | 0.97 | 0.2 | 0 | 3.7 |
| Stride | 61 / 60 | loop | 0 / 0.79 | 0.16 / -0.72 / -0.72 | 3.67 | 0.89 | one hand | 0 | 0 |
| Stumble | 21 / 21 | one-shot | n/a | 0.14 / 0.94 / 0.14 | 7.33 | 0.97 | 0.1 | 0 | 3.7 |

#### p4_broadcast (pelvis x,y max over all takes, both files: 0 cm; root movement max: 0 cm)

| Take | Frames main / loopcut | Kind | Seam main / loopcut (cm) | Lowest skate / stick / any (cm) | Pelvis z range (cm) | Right arm ext. max | Bottom hand off shaft max (cm) | Knob moved by reach guard (cm) | Left shoulder reach (deg) |
|---|---|---|---|---|---|---|---|---|---|
| Aim_FarReach | 31 / 30 | loop | 0 / 0.004 | 0 / 0.96 / 0 | 0.07 | 0.828 | one hand | 0 | 0 |
| Aim_Front | 31 / 30 | loop | 0 / 0.004 | 0 / 1 / 0 | 0.06 | 0.886 | 0.12 | 0 | 0 |
| Aim_Left | 31 / 30 | loop | 0 / 0.004 | 0 / 0.97 / 0 | 0.06 | 0.609 | 0.15 | 0 | 0 |
| Aim_Right | 31 / 30 | loop | 0 / 0.004 | 0 / 1.1 / 0 | 0.06 | 0.949 | 0.12 | 0 | 0 |
| Celebrate | 73 / 73 | one-shot | n/a | 0 / 1.06 / 0 | 13 | 0.965 | 0.16 | 0 | 8.1 |
| Check | 31 / 31 | one-shot | n/a | 0 / 0.66 / 0 | 20.8 | 0.965 | 0.1 | 0 | 8.1 |
| Crossover_Left | 41 / 40 | loop | 0 / 1.577 | 0.23 / 8.64 / 0.23 | 5.2 | 0.747 | one hand | 0 | 0 |
| Crossover_Right | 41 / 40 | loop | 0 / 1.577 | 0.23 / -0.22 / -0.22 | 5.2 | 0.747 | one hand | 0 | 0 |
| Faceoff_Draw | 19 / 19 | one-shot | n/a | 0 / 0.24 / 0 | 11.7 | 1.038 | 0.2 | 48 | 16 |
| Faceoff_Ready | 31 / 30 | loop | 0 / 0.008 | 0.04 / 0.24 / 0.04 | 0.06 | 0.997 | 0.15 | 37.2 | 0 |
| Fall | 37 / 37 | one-shot | n/a | 0 / 1.06 / 0 | 61.04 | 0.965 | 0.1 | 0 | 8.1 |
| Fall_Forward | 37 / 37 | one-shot | n/a | 0 / 1.06 / 0 | 58.44 | 0.965 | 0.1 | 0 | 8.1 |
| Foul_Reaction | 49 / 49 | one-shot | n/a | 0 / 1.06 / 0 | 9.1 | 0.965 | 0.1 | 0 | 8.1 |
| GetUp | 73 / 73 | one-shot | n/a | 0 / 0.05 / 0 | 55.84 | 0.987 | 0.1 | 5.1 | 8.1 |
| GetUp_Front | 73 / 73 | one-shot | n/a | 0 / -0.41 / -0.41 | 55.84 | 0.985 | 0.1 | 16.5 | 8.1 |
| Glide | 121 / 120 | loop | 0 / 0.003 | 0 / 1.05 / 0 | 1.3 | 0.973 | 0.1 | 0 | 13 |
| Goalie_Block_Left | 25 / 25 | one-shot | n/a | 0.16 / 14.96 / 0.16 | 23.4 | 0.987 | one hand | 7 | 0 |
| Goalie_Block_Right | 25 / 25 | one-shot | n/a | 0.16 / 11.44 / 0.16 | 23.4 | 0.987 | one hand | 7 | 0 |
| Goalie_Hold | 31 / 30 | loop | 0 / 0.004 | 0.18 / 21.39 / 0.18 | 0.06 | 0.987 | one hand | 7 | 0 |
| Idle_Faceoff | 121 / 120 | loop | 0 / 0.004 | 0.04 / 0.32 / 0.04 | 1.56 | 0.991 | 0.15 | 24.7 | 0 |
| Lean_Left | 31 / 30 | loop | 0 / 0.004 | 1.11 / 10.67 / 1.11 | 0.06 | 0.747 | one hand | 0 | 0 |
| Lean_Right | 31 / 30 | loop | 0 / 0.004 | 1.11 / 0.2 / 0.2 | 0.06 | 0.747 | one hand | 0 | 0 |
| Lift | 19 / 19 | one-shot | n/a | 0 / 1.04 / 0 | 3.9 | 0.986 | 0.12 | 25.5 | 12.2 |
| Pass | 19 / 19 | one-shot | n/a | 0 / 0.33 / 0 | 2.6 | 0.986 | 0.12 | 24.2 | 13.1 |
| Poke | 22 / 22 | one-shot | n/a | 0 / 0.35 / 0 | 4.29 | 0.984 | 0.18 | 34 | 8.1 |
| Shot_Charge | 31 / 30 | loop | 0 / 0.004 | 0 / 87.37 / 0 | 0.06 | 0.854 | 0.15 | 0 | 0 |
| Shot_Slap | 43 / 43 | one-shot | n/a | 0 / 1.07 / 0 | 7.8 | 0.987 | 9.28 | 14.7 | 35 |
| Shot_Wrist | 22 / 22 | one-shot | n/a | 0 / 0.44 / 0 | 3.9 | 0.984 | 7.47 | 13.2 | 35 |
| Stop | 37 / 37 | one-shot | n/a | 0 / 0.88 / 0 | 19.5 | 0.965 | 0.17 | 0 | 8.1 |
| Stride | 61 / 60 | loop | 0 / 0.688 | 0.01 / -0.33 / -0.33 | 5.2 | 0.83 | one hand | 0 | 0 |
| Stumble | 21 / 21 | one-shot | n/a | 0 / 1.06 / 0 | 10.4 | 0.965 | 0.1 | 0 | 8.1 |

## Second pass: twist continuity and foot pitch (16:04 to 16:20)

The coordinator asked for this pass after the retarget lane (SKH, `status/B-skater-skh.md`) found limb bones flipping 180 degrees about their own axis.
Blender only, no credits.
- `skf/export/` is untouched: its files are still from 15:27 to 15:30, and the engine lane imports from it.
- `s4/` is untouched.
- New files are in `C:\Users\kazda\jam\skater-lab\skf\export2\<style>\`, with the same file names: `SK_Skater_<style>.fbx`, `SK_Skater_<style>_loopcut.fbx`, `SM_Stick_<style>.fbx`.
- Work files, checks and comparisons are in `skf\work2\<style>\`.
- Rebuild with `SKF_SUFFIX=2 bash C:/Users/kazda/jam/skater-lab/skf/run_all.sh`, about 1 min. That writes `export2/ work2/ r2/ sheets2/ logs2/`; without the suffix the script writes the first-pass folders.
- `node C:/Users/kazda/jam/skater-lab/skf/compare2.mjs`, run from `skf/`, prints the comparison below.

**Bind.** Bone names, hierarchy and rest bone heads match `export/` exactly in all three styles. Triangle counts are unchanged (6,746 / 11,968 / 9,636).
- p2_cel and p4_broadcast: rest bounds also identical.
- p3_clay: rest bounds differ by 3 mm at the left edge (y 69.9 to 69.6 cm), so its clay surface is not bit-identical between runs. I have not found the cause.
- An animation-only re-import onto the existing skeletons should be safe in all three. For P3 the mesh itself differs by a few millimetres.

### 1. Measurement (added to `check_fbx.py`, run on the re-imported FBX)

For every take, bone and frame step (including the wrap from the last frame to the first in looping takes), the check measures:
- the total rotation angle;
- its twist about the bone's own axis.

It lists every step over 45 degrees and counts those over 60. It also measures foot pitch relative to the shin per take. "Before" is the first-pass `export/`, re-checked in `skf/before2/`.

| Style | Steps > 60 deg, before -> after | Steps > 45 deg | **Twist > 60 deg** | **Twist > 45 deg** | Loopcut file, twist > 45 |
|---|---|---|---|---|---|
| p2_cel | 110 -> **4** | 129 -> 18 | 84 -> **0** | 93 -> **0** | 93 -> 0 |
| p3_clay | 114 -> **4** | 139 -> 31 | 84 -> **0** | 97 -> **0** | 97 -> 0 |
| p4_broadcast | 109 -> **3** | 130 -> 26 | 92 -> **0** | 99 -> **0** | 99 -> 0 |

Before, takes with steps over 60 deg (counts per style P2 / P3 / P4):

| Take | Before | After |
|---|---|---|
| Stumble | 21 / 21 / 21 | 1 / 1 / 1 |
| Shot_Slap | 19 / 12 / 17 | 2 / 2 / 1 |
| Fall | 15 / 24 / 14 | 0 or 1 |
| Foul_Reaction | 14 / 14 / 14 | 0 |
| GetUp | 8 / 9 / 8 | 0 |
| GetUp_Front | 4 / 6 / 6 | 0 |
| Fall_Forward | 6 / 4 / 6 | 0 or 1 |
| Celebrate, Check | 5 each | 0 |
| Stride | 2 / 4 / 4 | 0 |
| Poke, Crossover_Left, Goalie_Block_L/R | 2 each | 0 |
| Shot_Wrist | 3 / 2 / 1 | 0 |

The full per-take list, with foot pitch, is in `skf/work2/<style>/compare.md`.

**P4 `Stride` (the retarget lane's case)**: no step over 45 deg remains. The largest is 22.4 deg (`lowerarm_l`, frames 25 to 26), and the wrap is clean.

**Every step still over 45 deg is a swing, not a twist** (twist at most 12 deg per frame for limb bones). Each comes from a fast move in the pose table:

| Take, frame | Bones, largest step | Why |
|---|---|---|
| `Shot_Slap` f20 | spine_03, neck_01, clavicles 46.2 deg; left forearm and hand up to 70 deg | The downswing: key 19 eases "in" to the hit at 23, so the body unwinds fastest here. spine and neck carry 39 to 43 deg of authored yaw. |
| `Stumble` f1-3; `Fall` and `Fall_Forward` f1-3 | upper arms up to 70 deg, forearms up to 50 deg | These one-shots leave Glide with an "out" ease to the hit key at frame 6, so the arms fling out fastest in the first frames. |
| `Poke` f6 | free upper arm 46 to 58 deg | Thrown back for balance; key 5 to 10 eases "out". |
| `Celebrate` f4 | right forearm 45 to 54 deg | The stick comes up fast into the arms-up key. |

The 45-deg target is met for twist everywhere. For swing it is met everywhere except these authored fast moves. If skinned figures need them slower, retime or ease those keys in the pose table.

### 2. The solver fix (`build_clips.py`, applies to all three styles)

Root cause: the IK fixes each limb bone's direction, but takes the roll and the bend side from the pole projected off the bone, or off the root-to-target line. When the pole nearly lines up with that line, the projection swings round in one frame:
- a twist flip of up to 180 deg about the bone (invisible on ball joints);
- or the elbow or knee flipping sides (up to 144 deg forearm swing in `Foul_Reaction` and `Stumble`).

Fixes:
- **Twist continuity** (`Solver.enforce_twist`). For thigh, calf, upper arm, forearm and hand, on both sides, the twist relative to the previous frame (after removing the swing) is limited to 12 deg per frame. The bone follows the IK roll at that rate, unwrapped, never snapped.
  - Children keep their world pose. They sit on the bone's axis, so no joint position moves.
  - The stick stays exactly where it was, because it is placed in world space.
- **Bend-direction continuity** (`Solver.limb`). The knee or elbow bend direction may swing at most 30 deg per frame towards the pole.
- **Loops** run two warm-up laps before the recorded lap, so frame 0 continues from the end of the loop. Frame 0 and the last frame still agree exactly (main seams 0.000 cm), and the loopcut wrap is one ordinary frame step.

### 3. Foot pitch limit (relative to the shin): 20 deg dorsiflexion, 25 deg plantar flexion

- **Flat skates** (authored skate pitch under 10 deg) whose shin leans too far forward: the blade stays flat and the ankle slides forward under the knee, at most 6 cm. The knee and hip take up the difference through the IK; the pelvis does not move.
- **Pitched skates** (kneels, falls): the skate turns about the ankle to the limit, and the ankle is raised only if a blade point would go under the ice.
- Between 10 and 20 deg of authored pitch the two blend, so nothing toggles.

Largest foot pitch, dorsi / plantar in degrees (positive plantar = toes away from the shin):

| Style | All takes, before -> after | Fall | GetUp | Goalie_Block_L/R | Glide | Stride (dorsi) |
|---|---|---|---|---|---|---|
| p2_cel | 80.8 / 34.8 -> **20.2 / 25.1** | 32 / 30 -> 20 / 25 | 81 / 34 -> 20 / 25 | 42 / 35 -> 20 / 25 | 32 -> 20 | 34 -> 20 |
| p3_clay | 81.3 / 37.9 -> **20.8 / 25.0** | 32 / 29 -> 20 / 25 | 81 / 25 -> 20 / 25 | 42 / 38 -> 20 / 25 | 32 -> 20 | 33 -> 20 |
| p4_broadcast | 80.7 / 47.2 -> **21.2 / 25.1** | 35 / 47 -> 20 / 25 | 81 / 35 -> 20 / 25 | 45 / 41 -> 20 / 25 | 35 -> 20 | 37 -> 20 |

Fall_Forward and GetUp_Front went from 58 to 65 deg of dorsiflexion to 20. The worst remaining is 21.2 deg (P4 crossovers: the slide cap is reached and the turn takes the rest).
Lowest skate point in the lying and kneeling takes after the fix: 0.0 to 0.25 cm, so no skate dips into the ice. I did not measure the skinned-figure skate lift that the retarget lane reported (9 to 20 cm); that needs a retarget run on `export2/`.

### 4. Other checks re-run (first pass -> second pass)

**Unchanged** in all three styles:
- pelvis horizontal travel and root movement 0.000 cm (both files, all 31 takes);
- loop seams 0.000 cm in the main file;
- 24 bones, 31 / 31 takes;
- reaches exactly as before: Aim_Front, Aim_Left, Aim_Right 1.000 m; Aim_FarReach and Poke 1.600 m; same blade heights (4.5 to 5.2 cm and 8.9 to 9.4 cm);
- the bottom hand off the shaft is unchanged except P2 `Shot_Slap`, which improved from 2.74 to 0.10 cm;
- the right-arm extension is unchanged.

**Moved:**

| Style | Take | Number | First pass -> second pass |
|---|---|---|---|
| p2_cel | Lean_Left / Lean_Right | lowest skate | 1.2 -> 0.41 cm (better) |
| p2_cel | Stumble | first vs last frame | 0 -> **4.4 cm** (see below) |
| p3_clay | Aim_Front | lowest skate | 0.14 -> 0.65 cm (the dorsiflexion limit tips the boot slightly) |
| p3_clay | Aim_Right | lowest skate | 0.14 -> 1.11 cm |
| p3_clay | Stumble | first vs last frame | 0 -> 1.3 cm |
| p4_broadcast | Lean_Left / Lean_Right | lowest skate | 1.11 -> 0.36 cm (better) |
| p4_broadcast | Stumble | first vs last frame | 0 -> 3.4 cm |
| p4_broadcast | Goalie_Block_Left | first vs last frame | 59.3 -> 60.4 cm (ends in the block by design) |

`Stumble` does not end exactly in Glide any more: its 20 frames are too short for the elbow bend direction to finish swinging back at 30 deg per frame. A blend out of the one-shot hides 1 to 4 cm.

One intermediate attempt was wrong and is gone: pivoting the skate on the touching blade end made the ankle jump 17 cm in `GetUp` and the P4 crossover wrap jump 21 cm. The final version pivots about the ankle and caps the slide; the numbers above are from it.

### Sheets (only where the picture changed)

- `devlog/shots/20261112-161833-B-lab-skf2-p2_cel-falls-side.png`
- `devlog/shots/20261112-161833-B-lab-skf2-p3_clay-falls-side.png`
- `devlog/shots/20261112-161833-B-lab-skf2-p4_broadcast-falls-side.png`

Each is from the side, from `export2/`: Fall, Fall_Forward, GetUp, GetUp_Front, Goalie_Block_Left, 8 frames each. Boots now stay in line with the shin when lying and kneeling.

### Not verified

- **Skinned figures.** My figures are rigid. Whether the hand jumps (18 to 80 cm) and the skate lift on skinned figures are gone needs the retarget lane to re-run on `export2/`. Nothing in the bone data now turns more than 12 deg per frame about its own axis.
- **Unreal.** Nothing was imported there.
- **Authored fast moves.** The swing steps over 45 deg listed above (slap downswing, the starts of Stumble and the falls, the poke's free arm) are authored. I left them as they are.
- **Skate lift.** The dorsiflexion limit lifts P3's skate 0.65 to 1.1 cm in two aim holds.

## Third pass: aim grid (17:05 to 17:22)

The coordinator asked for this pass after the engine lane's findings (`wt/b-vfx/status/SK2-report.md`). Blender only.

**Where the files are**
- New files: `C:\Users\kazda\jam\skater-lab\skf\export3\<style>\`, same three file names.
- `export/` (15:27 to 15:30) and `export2/` were not touched; nor was `s4/`.
- Work files, `aim_grid.json` and `compare3.md` (every number that moved, per take): `skf\work3\<style>\`.
- Rebuild: `SKF_SUFFIX=3 bash C:/Users/kazda/jam/skater-lab/skf/run_all.sh`, about 1.5 min.
- Comparison: `node compare3.mjs`, run from `skf/`.
- Each FBX now has **55 takes**: the 31 old ones plus 24 grid holds.

**The design point everything rests on.** The engine blends the aim holds as a layer from `spine_01` up, using mesh-space rotations.
- The pelvis and the legs always come from locomotion.
- So every grid hold keeps **Glide's pelvis height, pelvis rotation and planted skates exactly**. The pelvis does not travel.
- The turn is in the shoulders: spine_01 to spine_03 carry 45 % of the aim angle as yaw, with the head counter-turned.
- The reach comes from the spine leaning over the stick, the top arm and the stick. There is no lunge, because the engine would never show the legs of a hold.
- For holds on the right side, the right shoulder comes forward (right clavicle yaw 0.15 × the angle).
- Hips do not turn in a hold: the layer cannot carry them.

### 1. The grid: 24 holds, names `Aim_<N|P><angle>_R<reach in cm>`

- Angles: -63, -46, -30, 0, +35, +70 degrees. N = to the skater's right, P = to the left, so `P00` is straight ahead.
- Reaches: 1.00, 1.30, 1.45, 1.60 m.
- Examples: `Aim_N63_R160`, `Aim_P35_R100`, `Aim_P00_R145`.
- Each hold is a 30-frame loop with a 0.1-degree wobble, like the other holds. Its frame 0 is the measured pose.
- The -46 column and the 1.45 m row were **added after the first blend test**: the 15-hold grid failed it, see below.
- The old names are kept as **aliases**, full takes with the same pose as a grid hold:

| Old name | Is the same pose as |
|---|---|
| `Aim_Front` | `Aim_P00_R100` |
| `Aim_Left` | `Aim_P70_R100` (was 66 deg, now 70) |
| `Aim_Right` | `Aim_N63_R100` |
| `Aim_FarReach` | `Aim_P00_R160` (was one-handed with a lunge; now two-handed, from the Glide stance) |

**How each hold is solved**
- The blade heel moves on the ice until the **blade centre is on the asked point** (the angle and the reach from the pelvis).
- Then the solver picks the shaft tilt and two hands or one: two hands whenever the bottom hand reaches the shaft without over-reaching the top arm, and the flattest blade among those.
- **One-handed holds**, where the bottom hand comes off as a player's does for a long reach:

| Style | One-handed holds | Why |
|---|---|---|
| p2_cel | `Aim_N63_R160`, `Aim_N46_R160` | Far to the right at 1.6 m the shaft runs across in front of the right hip, out of the left arm's reach. |
| p3_clay | `Aim_N63_R145`, `Aim_N63_R160`, `Aim_N46_R160`, `Aim_N30_R160` | Shorter arms (49 cm against P2's 55). |
| p4_broadcast | `Aim_N63_R160` | Longer arms. |

Every other hold is two-handed, with the bottom hand within 0.2 cm of the shaft.

**Error per hold**, measured on the baked bones: position error / blade heel and toe height above the ice / hands. The blade "flat on the ice" sits 0 to 2.3 cm up in P2 and P4. That is the existing ice clamp on the shaft end; the blade itself is level.

| Hold | p2_cel: error cm / heel, toe cm / hands | p3_clay: error cm / heel, toe cm / hands | p4_broadcast: error cm / heel, toe cm / hands |
|---|---|---|---|
| Aim_N63_R100 | 0 / 1.82, 1.23 / two | 0 / 3.92, 0.37 / two | 0.01 / 1.9, 1.32 / two |
| Aim_N63_R130 | 0 / 1.76, 1.17 / two | 0.01 / 1.87, 1.27 / two | 0.02 / 1.8, 1.23 / two |
| Aim_N63_R145 | 0.01 / 1.75, 1.16 / two | 0.01 / 2.11, 1.52 / **one** | 0.01 / 2.01, 1.43 / two |
| Aim_N63_R160 | 0.01 / 1.77, 1.18 / **one** | 0.01 / 2.11, 1.51 / **one** | 0.01 / 2.01, 1.43 / **one** |
| Aim_N46_R100 | 0 / 1.82, 1.23 / two | 0 / 3.91, 0.36 / two | 0.01 / 1.9, 1.32 / two |
| Aim_N46_R130 | 0.01 / 1.75, 1.16 / two | 0.01 / 1.86, 1.26 / two | 0 / 1.79, 1.22 / two |
| Aim_N46_R145 | 0.02 / 1.74, 1.15 / two | 0.01 / 2.12, 1.52 / two | 0.02 / 2.02, 1.44 / two |
| Aim_N46_R160 | 0 / 1.73, 2.33 / **one** | 0.02 / 2.11, 1.51 / **one** | 0.02 / 2.01, 1.44 / two |
| Aim_N30_R100 | 0 / 1.82, 1.23 / two | 0.01 / 2.25, 0.47 / two | 0.01 / 1.9, 1.32 / two |
| Aim_N30_R130 | 0.01 / 1.74, 1.15 / two | 0 / 2.1, 1.51 / two | 0 / 1.78, 1.21 / two |
| Aim_N30_R145 | 0.01 / 1.77, 1.17 / two | 0.01 / 2.12, 1.53 / two | 0.02 / 2.02, 1.45 / two |
| Aim_N30_R160 | 0.02 / 1.82, 0.04 / two | 0.02 / 2.11, 1.52 / **one** | 0.02 / 2.01, 1.43 / two |
| Aim_P00_R100 | 0 / 1.81, 1.22 / two | 0 / 1.99, 1.39 / two | 0.01 / 1.88, 1.3 / two |
| Aim_P00_R130 | 0.02 / 1.73, 1.14 / two | 0 / 2.11, 1.52 / two | 0.01 / 2.02, 1.44 / two |
| Aim_P00_R145 | 0.02 / 1.74, 1.15 / two | 0.01 / 2.13, 1.54 / two | 0.02 / 2.03, 1.45 / two |
| Aim_P00_R160 | 0 / 1.76, 1.17 / two | 0.01 / 2.12, 1.53 / two | 0.01 / 2.03, 1.45 / two |
| Aim_P35_R100 | 0.01 / 1.8, 1.21 / two | 0.01 / 1.94, 1.35 / two | 0.02 / 1.86, 1.28 / two |
| Aim_P35_R130 | 0.01 / 1.74, 1.15 / two | 0 / 2.12, 1.52 / two | 0.01 / 2.02, 1.44 / two |
| Aim_P35_R145 | 0 / 1.76, 1.17 / two | 0.01 / 2.12, 1.53 / two | 0.01 / 2.02, 1.45 / two |
| Aim_P35_R160 | 0.01 / 1.8, 1.2 / two | 0.01 / 2.1, 1.51 / two | 0 / 2.01, 1.44 / two |
| Aim_P70_R100 | 0.01 / 1.78, 1.19 / two | 0 / 2.09, 1.49 / two | 0.02 / 1.82, 1.25 / two |
| Aim_P70_R130 | 0.01 / 1.77, 1.18 / two | 0.01 / 2.11, 1.52 / two | 0.01 / 2.01, 1.44 / two |
| Aim_P70_R145 | 0.01 / 1.8, 1.2 / two | 0.01 / 2.1, 1.51 / two | 0.03 / 1.79, 1.21 / two |
| Aim_P70_R160 | 0.01 / 1.82, 1.23 / two | 0.02 / 1.91, 1.32 / two | 0.03 / 1.83, 1.26 / two |

All 24 holds are on their point within **0.03 cm** in all three styles.

### Blend test (engine-style)

How it is measured:
- 53 test poses per style: the midpoint of every pair of neighbouring holds (angle and reach), and the centre of every cell (four holds at 25 % each).
- The layer bones (spine_01 and everything below it in the hierarchy) are blended as mesh-space rotations: a weighted, sign-aligned and normalised quaternion sum, as the engine does.
- The pelvis and legs come from Glide, and positions are rebuilt by forward kinematics from the rotations alone.
- The blade position error is measured against the asked mid-angle and mid-reach.

| Style | Worst position error | Worst blade height | Over 8 cm or 3 cm |
|---|---|---|---|
| p2_cel | **4.95 cm** (`Aim_N46_R160` / `Aim_N30_R160`) | -2.5 cm | none |
| p3_clay | **3.51 cm** (`Aim_P70_R100` / `Aim_P70_R130`) | +3.78 cm heel / +0.23 toe | **1**: `Aim_N63_R100` / `Aim_N46_R100`, heel 3.78 cm up |
| p4_broadcast | **2.96 cm** (`Aim_P70_R100` / `Aim_P70_R130`) | +1.96 cm | none |

How the grid got there:
1. The 15-hold grid that was asked for (3 reaches × 5 angles) failed. In P2 and P3 the blade dipped 3.3 to 4.7 cm, or lifted 7.9 cm, between -63 and -30 at 1.3 to 1.6 m, next to the one-handed holds.
2. Adding the -46 column and the 1.45 m row fixed P2 and P4.
3. P3 still failed badly, up to 21 cm. Its top arm was folded almost shut (extension 0.18 to 0.3), because the knob sat at the shoulder. Such arms swing the hand when blended.
4. Fix in the style table: **P3 now uses a flatter-lie stick (56 deg instead of 50)**. The knob sits about 16 cm lower and the arm opens, with the blade still flat.
5. Bringing the right shoulder forward on right-side aims removed one more failure.

**P3's remaining limit.** At 1.0 m on the right (`Aim_N63_R100`, `Aim_N46_R100`) the holds themselves have the blade heel 3.9 cm up and the toe on the ice. A flat blade with the 56-deg lie would put the knob past the top arm's reach (extension 0.88 to 0.90 already). A denser grid cannot fix a hold, so this one failure stays. The fix would be a lower or shorter stick for P3 at short reach, or a grip lower on the shaft.

**P3 now covers the full range.** It reaches -63 to +70 at 1.6 m, which was the earlier gap at -56 / +63. The far right side at 1.45 to 1.6 m is one-handed.

**Aim layer over Stride** (lower body from every 3rd Stride frame, layer from the hold): blade bottom -0.7 to +1.6 cm in all styles, for `Aim_P00_R100`, `_R130`, `_R160`, `Aim_N63_R160` and `Aim_P70_R160`. The engine's -7 cm dip came from Stride's low hips (point 4).

### 2-3. Elbows, upper back, P4's left arm

- **Elbows lower and forward.** The arm poles of Glide (and so of every pose built on it), Stride and every grid hold now point down and a little forward. Before they pointed back.
  - `Fall`, `Fall_Forward`, `GetUp`, `GetUp_Front` keep the old poles. With the new ones the elbows went 2 to 4 cm into the ice when lying; restored.
- **Rear-high sheets** (behind, 30 deg above, aimed at the upper back): Glide, Stride frames 0, 15, 30, and four aim holds.
  - In all three styles the 40 x 30 cm upper back stays clear of arms and gloves.
  - In P3 the felt number patch is fully visible in every frame.
  - This was checked by eye on the sheets only; I did not compute an occlusion number.
- **P4 `Lift`, `Pass` at frames 6, 8, 11** (rear-high): both hands forward on the shaft (bottom hand within 0.2 cm in my metrics).
  - The engine probably saw the first-pass export, whose elbow and twist flips were fixed in the second pass. Unverified.
  - `Poke` is one-handed by design: the free arm goes out and back for balance at frame 10.

### 4. Stride hips at Glide height (all styles)

The Stride keys now drop the pelvis 0.13 and 0.12 instead of 0.22 and 0.18. The push comes from the knees.

Pelvis height min..max over the take (cm):

| Take | p2_cel | p3_clay | p4_broadcast |
|---|---|---|---|
| Glide | 76.0..77.0 | 72.0..72.9 | 80.4..81.7 |
| **Stride** | **75.0..76.0** (was a 4.0 range, about 9 cm below Glide) | **71.1..72.0** | **79.1..80.4** |
| Crossover_Left / Right | 69.0..73.0 | 65.6..69.3 | 71.3..76.5 |
| Lean_Left / Right | 71.0 | 67.4 | 73.9 |
| Idle_Faceoff | 65.8..67.0 | 62.7..63.8 | 67.1..68.7 |

Stride stays within 1.3 cm of Glide's range in every style. Crossovers and leans were not asked for and were not changed: they are 4 to 10 cm lower than Glide. With the aim layer they would dip the blade the way Stride did, so the same fix may be wanted there.

### 5. Other checks (second pass -> third pass; full lists in `work3/<style>/compare3.md`)

**Unchanged in all styles:**
- pelvis and root travel 0.000 cm, both files, 55 takes;
- loop seams 0.000 cm in the main file;
- 24 bones; triangles the same;
- twist steps over 45 deg: **0**;
- foot pitch max 20.2 to 21.2 / 25.0 to 25.1 deg, unchanged.

**Old reach numbers.** The aliases are exactly on their points now (1.000 m at 0 / +70 / -63 deg; 1.600 m at 0 deg). Before they were 1.000 m at +1.3 / +66.3 / -63.6 deg.
- `Aim_FarReach`'s blade centre went from 9.4 cm up to 4.4 to 4.8 cm, a flatter blade.
- `Poke` is unchanged at 1.600 m.

**Moved:**

| Style | What | Second pass -> third |
|---|---|---|
| all | steps over 45 deg | P2 18 -> 31, P3 31 -> 37, P4 26 -> 34 |
| P3 | steps over 60 deg | 4 -> 9 |
| P2 | Stride stick low | -0.68 -> -1.69 cm |
| P3 | Stride stick low | -0.72 -> -1.15 cm |
| P3 | GetUp stick low | -0.73 -> -2.01 cm |
| P3 | Faceoff_Draw / Faceoff_Ready stick low | -0.79 / -0.52 cm |
| P3 | top-arm extension (Faceoff_*, GetUp, Idle_Faceoff, Poke) | up 0.05 to 0.2, to 1.05 to 1.44 |
| P4 | Stride loopcut wrap | 0.71 -> 5.7 cm (not a pop, see below) |
| P2, P3, P4 | Stumble first vs last frame | 4.4 -> 2.4, 1.3 -> 2.5, 3.4 -> 1.5 cm |

- **Steps over 45 deg.** All the new ones are swings in the first frames of `Celebrate`, `Stumble`, `Fall` and `Fall_Forward` and in the slap downswing (frames 20 to 26). The arms now leave a low-elbow Glide for poses authored with elbows back, so they move further in the same "out" ease. Twist is still 0.
- **P3's stick.** Most of the P3 moves come from the flatter-lie stick: its blade sits differently in the old tables. Where the extension is above 1, the IK clamps and the knob ends a few centimetres short; the hand still holds it.
- **P4 Stride wrap.** The right hand moves 7.5 cm in the frame before the wrap and 8 cm in the frame after it; the wrap step is 4.5 cm. The low, forward elbow makes the stick arm sweep faster.
- **Body into the ice.** None: lowest body points as in the second pass.

### Sheets

- `devlog/shots/20261112-172056-B-lab-skf3-p2_cel-aimgrid-above.png`, `...-aimgrid-side.png`, `...-rearhigh.png`
- the same three for `p3_clay` (`172056`) and `p4_broadcast` (`172057`)
- `devlog/shots/20261112-172057-B-lab-skf3-p4_broadcast-verbs.png` (Lift and Pass rear-high, Poke from the side)

Above: columns -63 to +70 (+ = the skater's left, image left), rows 1.00 to 1.60 m. Side: orthographic, whole stick in frame.

### Not verified

- **The engine's blend space.** My blend test assumes plain quaternion averaging of mesh-space rotations, with linear weights within a grid cell. UE's 2D blend space with uneven sample spacing (-63 / -46 / -30 / 0 / 35 / 70 and 1.00 / 1.30 / 1.45 / 1.60) may triangulate differently.
- **The engine's reach clamp.** It clamps to the figure's hold angles, which are now -63 / +70 for every style.
- **Back occlusion.** No number, only the rear-high sheets.
- **P4 Lift / Pass.** Whether the engine's left-arm swing came from the first-pass flips.
- **Unreal.** Nothing was imported.
