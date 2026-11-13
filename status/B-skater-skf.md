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

## Fourth pass: fixes, falls and collisions (17:42 to 18:00)

The coordinator asked for this pass. Blender only.

**Where the files are**
- New files: `C:\Users\kazda\jam\skater-lab\skf\export4\<style>\`, same three file names, **70 takes** (55 plus 15 new).
- `export/` (15:27), `export2/` (16:17) and `export3/` (17:19) were not touched; nor was `s4/`.
- Work files: `skf\work4\<style>\` (`check_fbx.json`, `clips_report.json`, `aim_grid.json`, `summary4.md`).
- Rebuild: `SKF_SUFFIX=4 bash C:/Users/kazda/jam/skater-lab/skf/run_all.sh`, about 3 min with the new renders.
- Summaries: `node summary4.mjs` and `node wrap4.mjs`, run from `skf/`.

**R-0025 and R-0026 are not in my checkout.** `contracts/RULINGS.md` ends at R-0022. Part B follows the coordinator's description of them.

### Part A: fixes

**1. Aim holds are truly static.**
- Every `Aim_*` hold (24 grid holds and 4 aliases) is one key held for 30 frames. Every frame is identical: the largest joint motion over the 24 holds is **0 cm** in all styles, and the free hand has zero motion.
  - Before: a 0.1-degree wobble moved the free hand 5.5 cm at frames 7 to 8.
- The free hand of the one-handed holds now hangs beside the hip (left shoulder offset forward 6, out 10, down 48 cm), never behind the back.

**2. Wrap speed.**
- New check in `check_fbx.py`: in every looping take, the step from the last frame to the first is compared with the mean step of the cycle, for both hands and the blade toe. An offender is a wrap step above 1.2 × the mean.

| Style | Before (export3) | After (export4) |
|---|---|---|
| p2_cel | 0 | 0 |
| p3_clay | 0 | 0 |
| p4_broadcast | 2: `Stride` hand_r (wrap 4.48 cm against a 2.47 cm mean, ×1.81) and blade (5.62 against 4.24, ×1.32) | **0** |

- Cause: the top-hand grip correction (5 undamped iterations) oscillated in some frames. Frame 0 of P4 Stride sat 10 cm off frames 59 and 1, although the input pose was the same.
- Fix: the correction is damped (factor 0.7, up to 15 iterations) for both hands, in every take. Frames 59, 0 and 1 now agree within 0.2 cm.

**3. Locomotion hips within 2 cm of Glide** (pelvis height min..max, cm):

| Take | p2_cel | p3_clay | p4_broadcast |
|---|---|---|---|
| Glide | 76.0..77.0 | 72.0..72.9 | 80.4..81.7 |
| Stride | 75.0..76.0 | 71.1..72.0 | 79.1..80.4 |
| Crossover_Left / Right | 74.5..76.5 (was 69..73) | 70.6..72.5 (was 65.6..69.3) | 78.5..81.1 (was 71.3..76.5) |
| Lean_Left / Right | 75.0 (was 71.0) | 71.1 (was 67.4) | 79.1 (was 73.9) |
| Stop | 75.0..76.0 (was 61..76) | 71.1..72.0 (was 58.3..72) | 79.1..80.4 (was 60.9..80.4) |

- `Backward` does not exist in this set.
- The trunk now leans 2 to 4 degrees more in the leans and in Stop, and Stop's front skate is at 0.38 m instead of 0.52.
- **New leg reach guard.** With the hips up, wide authored steps were out of the legs' reach: crossover skates missed their targets by 12 to 16 cm. A target out of reach now moves in towards the hip horizontally and stays on the ice. Skate target miss after the fix: **0** in every take.

**4. P3 child's grip.**
- The top hand grips **10 cm below the knob**: grip-to-heel 1.30 m (was 1.40), and the shaft and knob continue 10 cm above the hand. The 56-degree lie from the third pass stays.
- The P3 hold solver treats a flat blade (0 to 2 cm) as a hard rule.
- Result: **all 24 P3 holds** have the blade 0.02 to 2.01 cm above the ice, within 0.03 cm of their point, and the blend test passes (worst error 2.75 cm, height -2.87..1.90 cm).
  - `Aim_N63_R100` was 3.9 cm heel-up; it is now flat and two-handed.
- The cost is more one-handed holds: `N63_R130`, `N63_R145`, `N63_R160`, `N46_R145`, `N46_R160`, `N30_R160`.
- I tried the hard rule for P2 too: one far-right hold could not be both flat and on its point (13 cm miss), so P2 and P4 keep the third-pass rule. Their blades sit 0.1 to 2.3 cm up, and their blend tests pass.

**Other checks after Part A and Part B, in all styles:**
- **Unchanged:**
  - pelvis and root travel 0.000 cm and loop seams 0.000 cm in all 70 takes;
  - aliases and Poke exactly on 1.000 m / 1.600 m;
  - foot pitch max 20.3 to 21.8 / 25.0 to 25.1 deg.
- **Twist over 45 deg:** 0 in P2 and P3. P4 has 2, both its left skate in the new `GetUp_Side_*` (below).
- **Moved against the third pass** (full lists in `work4/<style>/summary4.md`):

| Style | What | Third pass -> fourth |
|---|---|---|
| P2, P4 | crossover and lean stick low (stick carried higher because the hips are higher) | P2 4.4 to 5.8 -> 6.8 to 7.0 cm; P4 8.7 to 10.7 -> 12.3 to 12.4 cm |
| P2 | Stride stick low | -1.69 -> -1.08 cm |
| P2 | Check stick low | -0.64 -> -0.17 cm |
| P3 (child grip) | arm reach in the faceoffs and GetUp (better) | 1.29 to 1.44 -> 0.99 to 1.0 |
| P3 | GetUp stick low (better) | -2.01 -> +0.40 cm |
| P3 | Faceoff stick (better) | now above the ice |
| P3 | Shot_Slap / Shot_Wrist bottom-hand gap (worse) | 3.9 / 3.2 -> 5.7 / 5.4 cm |
| P3 | Lean_Left stick low | 3.2 -> 8.6 cm |
| P4 | Stride stick low | -0.14 -> +1.91 cm |

### Part B: falls and collisions (15 new takes, same solver and rules)

| Take | Frames (s) | Contact frame | Blends from -> to |
|---|---|---|---|
| `Check_Hit_Left` / `_Right` / `_Back` | 29 (0.47) | 3 | Glide -> Glide |
| `Knockdown_Left` | 31 (0.5) | 2 | Glide -> lying on the right hip and forearm (held from 21) -> **`GetUp_Side_Left`** |
| `Knockdown_Right` | 31 (0.5) | 2 | Glide -> lying on the left hip -> **`GetUp_Side_Right`** |
| `Knockdown_Back` | 31 (0.5) | 2 | Glide -> **`Fall_Forward`'s end pose** (frame 26 on) -> `GetUp_Front` |
| `Trip` | 27 (0.43) | 2 | Glide -> **`Fall_Forward`'s end pose** -> `GetUp_Front` |
| `GetUp_Side_Left` / `_Right` | 55 (0.9) | - | the knockdown's last frame exactly -> Glide |
| `Board_Hit` | 31 (0.5) | 4 | Glide -> Glide |
| `Board_Pin` | 61 (1.0) | - | loop |
| `Check_Deliver_Left` / `_Right` | 31 (0.5) | 16 | Glide -> Glide |
| `Poke_Hit` | 25 (0.4) | 3 | Glide -> Glide |
| `Lift_Hit` | 21 (0.33) | 3 | Glide -> Glide |

- **Check hits** (the receiver stays up): upper body thrown away from the hit, extreme at 6 held to 8, head lagging, free arm out, one recovery step at 15 on the push side, back in Glide at 28.
- **Knockdowns reach the ice within 0.35 s:**
  - the side knockdowns at frame 15 (0.25 s);
  - `Knockdown_Back` on its knees and hands at 9, chest at 19;
  - `Trip` (left leg taken, forward tumble) on its forearms at 12 (0.2 s), chest at 17.
- **Side get-ups:** sit up with the stick planted as a prop (12), kneel on the stick (34), up (54).
- **`Board_Hit`:** both forearms up and the near shoulder in, head turned away, compressed at 6 to 8, a rebound step back at 16, Glide at 30.
- **`Board_Pin`:** a braced lean on the forearms against the boards, stick up; a 60-frame breathing loop.
- **Delivered checks:** `Check_Deliver_Right` is today's `Check` (wind 9, drive 16, follow-through 21 to 30). **`Check` stays as its alias** (same keys).
  - `Check_Deliver_Left` mirrors body, head, clavicles and skates; the stick stays in the right hand.
- **`Poke_Hit`:** the stick is knocked out to the right (extreme 6 to 8), a half step with the right skate at 14, Glide at 24.
- **`Lift_Hit`:** the blade rises to 36 cm (7 to 9) and comes back by 20.

**The stick stays in the top hand in every new take.**

How the new takes measure (all three styles; per-part tables below):
- Every body part and the skates stay at or above the ice. The lowest are the forearms and arms in the side knockdowns (2 to 4 cm up) and the hands on the ice in Knockdown_Back and Trip (1.1 to 1.7 cm).
- The stick dips at most 0.7 to 1.0 cm into the ice, except P4 `GetUp_Side_Right`, -2.9 cm while it is planted as a prop.
- Bottom hand on the shaft within 0.2 cm in every two-handed frame.
- Pelvis horizontal travel 0.000 cm.

**Iteration on my own sheets:**
- In pass 1, the side knockdowns ended curled up like someone sitting. Their arms and torso were also 3 to 10 cm into the ice.
- `GetUp_Side_Left` had the arm over-reached (extension 2.9) with the stick 7.6 cm in the ice, from the switch between a stick lying on the ice and a propped stick.
- Pass 2:
  - the side pose is higher (pelvis drop 0.53, then 0.55; roll 60 to 66);
  - the side pose is stretched out (skates 0.52 to 0.58 forward, trunk back) and reads as sliding on the hip;
  - the get-up sits with the stick held at the grip as a prop;
  - the get-ups are 0.9 s, which removed the twist steps in P2 and P3.

**40 px (chase camera, blue and orange).** Left is distinguishable from right: the body goes down to opposite sides. Both read differently from the back and the trip. **`Knockdown_Back` and `Trip` look alike from behind**: both go forward onto the chest. Only the kicked leg differs, and it doesn't read at 40 px.

**What is weak**
- `Board_Hit` reads only as arms up and a lean at the 6-frame sampling; the impact is small. `Lift_Hit` is subtle, because the stick moves but the body barely does.
- `Check_Deliver_Left` keeps the stick on the right side, so it is a left-shoulder drive with a right-handed stick.
- The steps over 45 deg grew with the new takes (P2 31 -> 88, P3 37 -> 91, P4 34 -> 93). These are the fast arcade extremes (hits at frames 2 to 6) and are all swing.
- **P4 `GetUp_Side_Left` / `_Right`: the left skate turns 50 to 65 deg** at frames 24 to 25 when the shin swings from in front of the skate to behind it. Skates are authored, not twist-limited. A longer sit-to-kneel phase, or turning the skate in the table before the swing, would fix it.
- The knockdowns end in their own lying pose, held from frame 21. A knock-down during a get-up has no transition. Knock direction relative to facing is for the visual to pick.

### Per-style measurements (from `work4/<style>/summary4.md`)

### p2_cel: 70 takes, tris {"body":6550,"stick":196}
| Take | Frames | s | Contact | Lowest by part (cm) | Stick low | Steps >45 (max twist) | Hand gap | Arm ext |
|---|---|---|---|---|---|---|---|---|
| Check_Hit_Left | 29 | 0.467 | 3 | arms 62.71, forearms 52.03, hands 45.66, head 95.47, hips 49.74, knees_shins 13.42, skates 0, thighs 35.97, torso 59.96 | -0.74 | 8 (12) | 0.04 | 0.886 |
| Check_Hit_Right | 29 | 0.467 | 3 | arms 68.62, forearms 57.17, hands 46.23, head 95.47, hips 49.74, knees_shins 13.43, skates 0, thighs 35.97, torso 59.96 | 0.2 | 8 (12) | 0.04 | 0.886 |
| Check_Hit_Back | 29 | 0.467 | 3 | arms 68.13, forearms 57.36, hands 46.23, head 83.8, hips 51.07, knees_shins 14.01, skates 0, thighs 35.87, torso 63.62 | 0.73 | 8 (12) | 0.04 | 0.886 |
| Knockdown_Left | 31 | 0.5 | 2 | arms 2.99, forearms 3.67, hands 9.82, head 38.52, hips 5.93, knees_shins 8.36, skates 0, thighs 7.26, torso 8.88 | -0.41 | 6 (12) | 0.04 | 0.886 |
| Knockdown_Right | 31 | 0.5 | 2 | arms 8.07, forearms 8.71, hands 14.29, head 38.52, hips 5.93, knees_shins 11.15, skates 0, thighs 8.23, torso 8.88 | 0.73 | 6 (12) | 0.04 | 0.987 |
| Knockdown_Back | 31 | 0.5 | 2 | arms 16.87, forearms 13.22, hands 1.71, head 1.6, hips 7.41, knees_shins 8.97, skates 0, thighs 9.96, torso 8.77 | 0.61 | 8 (12) | 0.04 | 0.886 |
| Trip | 27 | 0.433 | 2 | arms 16.87, forearms 14.02, hands 1.71, head 1.6, hips 7.41, knees_shins 8, skates 0, thighs 8.9, torso 8.77 | 0.73 | 7 (12) | 0.04 | 0.886 |
| GetUp_Side_Left | 55 | 0.9 | - | arms 3.31, forearms 3.93, hands 9.86, head 39.52, hips 6.93, knees_shins 4.5, skates 0, thighs 6.12, torso 9.88 | -0.74 | 1 (22.4) | 0.04 | 0.985 |
| GetUp_Side_Right | 55 | 0.9 | - | arms 8.13, forearms 8.45, hands 6.05, head 39.52, hips 6.93, knees_shins 3.86, skates 0, thighs 5.48, torso 9.88 | -0.45 | 2 (41.5) | 0.04 | 0.987 |
| Board_Hit | 31 | 0.5 | 4 | arms 62.71, forearms 57.41, hands 46.22, head 76.83, hips 48.17, knees_shins 14.05, skates 0, thighs 32.85, torso 57.48 | 0.73 | 6 (12) | 0.05 | 0.889 |
| Board_Pin | 61 | 1 | - | arms 63.66, forearms 64.76, hands 72.83, head 74.09, hips 50.31, knees_shins 17.29, skates 0.23, thighs 26.83, torso 59.32 | 82.76 | 0 (0) | one hand | 0.588 |
| Check_Deliver_Left | 31 | 0.5 | 16 | arms 71.32, forearms 57.41, hands 46.23, head 97.52, hips 47.61, knees_shins 13.05, skates 0, thighs 33.04, torso 62.02 | -0.18 | 0 (0) | 0.04 | 0.886 |
| Check_Deliver_Right | 31 | 0.5 | 16 | arms 70.97, forearms 57.08, hands 46.23, head 97.52, hips 47.61, knees_shins 13.05, skates 0, thighs 33.04, torso 62.02 | -0.17 | 0 (0) | 0.04 | 0.886 |
| Poke_Hit | 25 | 0.4 | 3 | arms 68.62, forearms 52.32, hands 40.61, head 100.37, hips 51.81, knees_shins 13.78, skates 0, thighs 36.44, torso 65.92 | 0.72 | 3 (10.7) | 0.06 | 0.982 |
| Lift_Hit | 21 | 0.333 | 3 | arms 72.12, forearms 56.76, hands 44.99, head 106.21, hips 56.24, knees_shins 14.06, skates 0, thighs 37.25, torso 72.04 | 0.73 | 0 (0) | 0.2 | 0.986 |
Pelvis height min..max cm: Glide 76..77 (was 76..77); Stride 75..76 (was 75..76); Crossover_Left 74.5..76.5 (was 69..73); Crossover_Right 74.5..76.5 (was 69..73); Lean_Left 74.95..75 (was 70.95..71); Lean_Right 74.95..75 (was 70.95..71); Stop 75..76 (was 61..76)
Skate target miss max cm (IK could not reach): 
Grid: holds error max 0.02 cm; blade heel/toe 0.11..2.28 cm; one-handed Aim_N63_R160 Aim_N46_R160; blend worst 4.98 cm, height -2.5..1.9 cm, fails 0
Holds static: largest joint motion first->last frame over the 24 holds 0 cm; free hand (wrap check) all zero motion: true
All takes: pelvis x,y max 0 cm, root 0 cm, loop seams 0 cm; steps>45 31 -> 88 (new takes included), twist>45 0 -> 0; foot pitch max 21.8 / 25.1
Old reach: Aim_Front 1 m 0 deg; Aim_Left 1 m 70 deg; Aim_Right 1 m -63 deg; Aim_FarReach 1.6 m 0 deg; Poke 1.6 m 0 deg
Moved against the third pass: Check: stick low -0.64->-0.17; any low -0.64->-0.17 | Crossover_Left: stick low 4.37->6.78 | Faceoff_Ready: stick low 0.05->0.26 | Lean_Left: stick low 5.81->7.02 | Shot_Slap: steps>45 13->11 | Stride: stick low -1.69->-1.08; any low -1.69->-1.08 | Stumble: steps>45 6->4

### p3_clay: 70 takes, tris {"body":11556,"stick":412}
| Take | Frames | s | Contact | Lowest by part (cm) | Stick low | Steps >45 (max twist) | Hand gap | Arm ext |
|---|---|---|---|---|---|---|---|---|
| Check_Hit_Left | 29 | 0.467 | 3 | arms 58.6, forearms 47.3, hands 41.02, head 86.91, hips 47.35, knees_shins 12.67, skates 0.16, thighs 34.53, torso 56.48 | 0.27 | 8 (12) | 0.19 | 0.897 |
| Check_Hit_Right | 29 | 0.467 | 3 | arms 64.37, forearms 58.93, hands 50.8, head 86.83, hips 47.23, knees_shins 12.73, skates 0.16, thighs 34.77, torso 56.32 | 0.41 | 8 (12) | 0.19 | 0.897 |
| Check_Hit_Back | 29 | 0.467 | 3 | arms 65.82, forearms 54.7, hands 48.73, head 79.79, hips 48.22, knees_shins 12.58, skates 0.14, thighs 35.61, torso 57.11 | 0.41 | 8 (12) | 0.19 | 0.897 |
| Knockdown_Left | 31 | 0.5 | 2 | arms 6.96, forearms 6.98, hands 13.9, head 41.72, hips 11.67, knees_shins 10.86, skates 0.16, thighs 13.81, torso 16.18 | -0.56 | 7 (12) | 0.19 | 0.897 |
| Knockdown_Right | 31 | 0.5 | 2 | arms 11.34, forearms 11.58, hands 13.56, head 41.95, hips 11.4, knees_shins 11.14, skates 0.16, thighs 14.49, torso 15.8 | 0.41 | 6 (12) | 0.19 | 0.987 |
| Knockdown_Back | 31 | 0.5 | 2 | arms 24.23, forearms 11.63, hands 1.66, head 23.13, hips 13.76, knees_shins 11.24, skates 0.16, thighs 12.68, torso 14.16 | -0.41 | 6 (12) | 0.19 | 0.897 |
| Trip | 27 | 0.433 | 2 | arms 24.23, forearms 12.15, hands 1.66, head 23.13, hips 13.76, knees_shins 10.82, skates 0.16, thighs 11.95, torso 14.16 | 0.41 | 7 (12) | 0.19 | 0.914 |
| GetUp_Side_Left | 55 | 0.9 | - | arms 7.47, forearms 7.44, hands 13.9, head 42.63, hips 11.48, knees_shins 6.74, skates 0.14, thighs 7.97, torso 17.1 | -0.62 | 1 (27.4) | 0.19 | 0.979 |
| GetUp_Side_Right | 55 | 0.9 | - | arms 11.61, forearms 9.91, hands 6.87, head 42.86, hips 11.78, knees_shins 5.95, skates 0.14, thighs 7.3, torso 16.72 | -0.96 | 2 (41.9) | 0.19 | 0.987 |
| Board_Hit | 31 | 0.5 | 4 | arms 59.84, forearms 58.93, hands 50.78, head 80.87, hips 45.98, knees_shins 12.72, skates 0.16, thighs 32.59, torso 54.54 | 0.4 | 6 (12) | 0.2 | 0.904 |
| Board_Pin | 61 | 1 | - | arms 61.11, forearms 61.35, hands 71.54, head 80.11, hips 47.17, knees_shins 17.65, skates 1.53, thighs 26.87, torso 55.57 | 75.84 | 0 (0) | one hand | 0.62 |
| Check_Deliver_Left | 31 | 0.5 | 16 | arms 68.15, forearms 58.25, hands 47.04, head 87.44, hips 45.38, knees_shins 12.36, skates 0.14, thighs 32.88, torso 56.8 | -0.55 | 0 (0) | 0.19 | 0.897 |
| Check_Deliver_Right | 31 | 0.5 | 16 | arms 68.44, forearms 57.52, hands 50.49, head 87.44, hips 45.26, knees_shins 12.33, skates 0.16, thighs 32.6, torso 56.77 | 0.13 | 0 (0) | 0.19 | 0.897 |
| Poke_Hit | 25 | 0.4 | 3 | arms 67.74, forearms 50.8, hands 42.1, head 90.79, hips 49.27, knees_shins 12.65, skates 0.15, thighs 34.99, torso 60.5 | 0.32 | 3 (7.5) | 0.2 | 0.984 |
| Lift_Hit | 21 | 0.333 | 3 | arms 70.43, forearms 56.34, hands 47.97, head 95.42, hips 53.13, knees_shins 12.73, skates 0.16, thighs 36.05, torso 64.79 | 0.41 | 0 (0) | 0.2 | 0.986 |
Pelvis height min..max cm: Glide 72..72.92 (was 72..72.92); Stride 71.08..72 (was 71.08..72); Crossover_Left 70.62..72.46 (was 65.58..69.25); Crossover_Right 70.62..72.46 (was 65.58..69.25); Lean_Left 71.04..71.08 (was 67.37..67.42); Lean_Right 71.04..71.08 (was 67.37..67.42); Stop 71.08..72 (was 58.25..72)
Skate target miss max cm (IK could not reach): 
Grid: holds error max 0.03 cm; blade heel/toe 0.02..2.01 cm; one-handed Aim_N63_R130 Aim_N63_R145 Aim_N63_R160 Aim_N46_R145 Aim_N46_R160 Aim_N30_R160; blend worst 2.75 cm, height -2.87..1.9 cm, fails 0
Holds static: largest joint motion first->last frame over the 24 holds 0 cm; free hand (wrap check) all zero motion: true
All takes: pelvis x,y max 0 cm, root 0 cm, loop seams 0 cm; steps>45 37 -> 91 (new takes included), twist>45 0 -> 0; foot pitch max 21.7 / 25
Old reach: Aim_Front 1 m 0 deg; Aim_Left 1 m 70 deg; Aim_Right 1 m -63 deg; Aim_FarReach 1.6 m 0 deg; Poke 1.6 m 0 deg
Moved against the third pass: Aim_FarReach: stick low 0.94->0.73; arm ext 0.396->0.557 | Aim_Front: arm ext 0.939->0.869 | Aim_N30_R100: stick low 0.53->0.81 | Aim_N30_R145: stick low 0.94->0.19 | Aim_N30_R160: stick low 0.93->0.67; arm ext 0.347->0.493 | Aim_N46_R100: stick low 0.74->0.41; arm ext 0.879->0.914 | Aim_N46_R145: stick low 0.94->0.68; arm ext 0.452->0.501 | Aim_N46_R160: stick low 0.93->0.67; arm ext 0.382->0.512 | Aim_N63_R100: stick low 0.75->0.4; arm ext 0.901->0.938 | Aim_N63_R145: stick low 0.93->0.71; arm ext 0.506->0.55 | Aim_N63_R160: stick low 0.92->0.71; arm ext 0.45->0.565 | Aim_P00_R100: arm ext 0.939->0.869 | Aim_P00_R130: stick low 0.93->0.72 | Aim_P00_R145: stick low 0.95->0.69; arm ext 0.389->0.486 | Aim_P00_R160: stick low 0.94->0.73; arm ext 0.396->0.557 | Aim_P35_R100: arm ext 0.791->0.742 | Aim_P35_R130: arm ext 0.466->0.533 | Aim_P35_R145: arm ext 0.45->0.582 | Aim_P35_R160: arm ext 0.562->0.721 | Aim_P70_R130: arm ext 0.463->0.59 | Aim_P70_R145: arm ext 0.58->0.734 | Aim_P70_R160: arm ext 0.769->0.928 | Aim_Right: stick low 0.75->0.4; arm ext 0.901->0.938 | Celebrate: arm ext 0.97->0.897; steps>45 6->2 | Check: stick low -0.4->0.13; any low -0.4->0.13; arm ext 0.97->0.897 | Crossover_Left: stick low 1.85->8.3 | Crossover_Right: stick low -0.47->-0.86; any low -0.47->-0.86 | Faceoff_Draw: stick low -0.79->0.39; any low -0.79->0.16; arm ext 1.388->1.005 | Faceoff_Ready: stick low -0.52->0.82; any low -0.52->0.16; arm ext 1.293->0.997 | Fall: arm ext 0.97->0.913 | Fall_Forward: arm ext 0.97->0.914 | Foul_Reaction: arm ext 0.97->0.897 | GetUp: stick low -2.01->0.4; any low -2.01->0.14; arm ext 1.439->0.987 | GetUp_Front: stick low -1.38->-0.94; any low -1.38->-0.94; arm ext 1.052->0.979 | Glide: arm ext 0.973->0.903 | Goalie_Block_Left: stick low 18.62->19.02 | Goalie_Block_Right: stick low 15.52->15.95 | Goalie_Hold: stick low 20.17->21.27 | Idle_Faceoff: arm ext 1.088->0.99 | Lean_Left: skate low 1.2->0.92; stick low 3.17->8.58; any low 1.2->0.92 | Lean_Right: skate low 1.19->0.94; stick low 1.01->0; any low 1.01->0 | Poke: arm ext 1.052->0.981 | Shot_Charge: stick low 77.93->70.52 | Shot_Slap: hand gap 3.94->5.7; steps>45 11->8 | Shot_Wrist: hand gap 3.17->5.4 | Stop: arm ext 0.97->0.897 | Stride: stick low -1.15->-0.28; any low -1.15->-0.28 | Stumble: arm ext 0.97->0.897; steps>45 10->11

### p4_broadcast: 70 takes, tris {"body":9396,"stick":240}
| Take | Frames | s | Contact | Lowest by part (cm) | Stick low | Steps >45 (max twist) | Hand gap | Arm ext |
|---|---|---|---|---|---|---|---|---|
| Check_Hit_Left | 29 | 0.467 | 3 | arms 72.58, forearms 60.71, hands 53.87, head 103.11, hips 57.07, knees_shins 17.54, skates 0.03, thighs 40.98, torso 58.25 | 1.06 | 6 (12) | 0.04 | 0.966 |
| Check_Hit_Right | 29 | 0.467 | 3 | arms 79.35, forearms 67.12, hands 56.23, head 103.11, hips 57.07, knees_shins 17.55, skates 0.03, thighs 41, torso 58.25 | 0.79 | 6 (12) | 0.04 | 0.966 |
| Check_Hit_Back | 29 | 0.467 | 3 | arms 74.73, forearms 63.03, hands 56.23, head 92.88, hips 57.47, knees_shins 18.14, skates 0, thighs 39.52, torso 58.95 | 1.06 | 6 (12) | 0.04 | 0.966 |
| Knockdown_Left | 31 | 0.5 | 2 | arms 3.9, forearms 4.8, hands 12.59, head 41.45, hips 10.43, knees_shins 12.42, skates 0.03, thighs 10.8, torso 10.02 | 0.32 | 9 (12) | 0.04 | 0.966 |
| Knockdown_Right | 31 | 0.5 | 2 | arms 9.81, forearms 10.97, hands 17.38, head 41.45, hips 10.43, knees_shins 15.79, skates 0.03, thighs 12.77, torso 10.02 | 1.06 | 6 (12) | 0.04 | 0.987 |
| Knockdown_Back | 31 | 0.5 | 2 | arms 17.25, forearms 15.31, hands 1.06, head 18.06, hips 9.32, knees_shins 9.97, skates 0.03, thighs 11.5, torso 8.28 | 0.09 | 6 (12) | 0.04 | 0.966 |
| Trip | 27 | 0.433 | 2 | arms 17.25, forearms 15.88, hands 1.06, head 18.06, hips 9.32, knees_shins 12.1, skates 0.03, thighs 11.78, torso 8.28 | 1.06 | 7 (12) | 0.04 | 0.966 |
| GetUp_Side_Left | 55 | 0.9 | - | arms 4.43, forearms 5.22, hands 12.7, head 42.58, hips 10.01, knees_shins 2.21, skates 0, thighs 4.62, torso 10.5 | -0.06 | 1 (50.5) | 0.04 | 0.988 |
| GetUp_Side_Right | 55 | 0.9 | - | arms 9.26, forearms 9.74, hands 8, head 42.58, hips 10.01, knees_shins 1.64, skates 0, thighs 3.92, torso 10.5 | -2.85 | 1 (65.1) | 0.04 | 0.988 |
| Board_Hit | 31 | 0.5 | 4 | arms 70.84, forearms 67.12, hands 56.21, head 94.41, hips 54.93, knees_shins 18.16, skates 0, thighs 36.25, torso 56.36 | 1.06 | 6 (12) | 0.05 | 0.966 |
| Board_Pin | 61 | 1 | - | arms 70.62, forearms 72.4, hands 82.27, head 93.29, hips 56.52, knees_shins 19.5, skates 0.16, thighs 28.95, torso 57.76 | 90.69 | 0 (0) | one hand | 0.58 |
| Check_Deliver_Left | 31 | 0.5 | 16 | arms 81.47, forearms 67.12, hands 56.23, head 104.29, hips 53.38, knees_shins 17.03, skates 0.03, thighs 36.15, torso 55.7 | 0.39 | 0 (0) | 0.04 | 0.966 |
| Check_Deliver_Right | 31 | 0.5 | 16 | arms 80.53, forearms 66.91, hands 56.23, head 104.29, hips 53.38, knees_shins 17.03, skates 0.03, thighs 36.15, torso 55.7 | 0.63 | 0 (0) | 0.04 | 0.966 |
| Poke_Hit | 25 | 0.4 | 3 | arms 77.16, forearms 59.7, hands 46.01, head 109.09, hips 59.06, knees_shins 17.88, skates 0.03, thighs 41.63, torso 61.33 | 1.06 | 5 (12) | 0.06 | 0.985 |
| Lift_Hit | 21 | 0.333 | 3 | arms 82.33, forearms 65.81, hands 53.49, head 115.61, hips 63.78, knees_shins 18.93, skates 0.03, thighs 42.16, torso 66.37 | 1.06 | 0 (0) | 0.15 | 0.987 |
Pelvis height min..max cm: Glide 80.4..81.7 (was 80.4..81.7); Stride 79.1..80.4 (was 79.1..80.4); Crossover_Left 78.45..81.05 (was 71.3..76.5); Crossover_Right 78.45..81.05 (was 71.3..76.5); Lean_Left 79.03..79.1 (was 73.83..73.9); Lean_Right 79.03..79.1 (was 73.83..73.9); Stop 79.1..80.4 (was 60.9..80.4)
Skate target miss max cm (IK could not reach): 
Grid: holds error max 0.04 cm; blade heel/toe 1.24..2.04 cm; one-handed Aim_N63_R160; blend worst 2.97 cm, height -1.45..1.94 cm, fails 0
Holds static: largest joint motion first->last frame over the 24 holds 0 cm; free hand (wrap check) all zero motion: true
All takes: pelvis x,y max 0 cm, root 0 cm, loop seams 0 cm; steps>45 34 -> 93 (new takes included), twist>45 0 -> 2; foot pitch max 20.3 / 25.1
Old reach: Aim_Front 1 m 0 deg; Aim_Left 1 m 70 deg; Aim_Right 1 m -63 deg; Aim_FarReach 1.6 m 0 deg; Poke 1.6 m 0 deg
Moved against the third pass: Aim_N30_R145: stick low 1.08->0.88 | Crossover_Left: stick low 8.65->12.29 | Faceoff_Draw: arm ext 1.038->1.073 | Lean_Left: stick low 10.68->12.43 | Lean_Right: stick low 0.21->0.48 | Stride: stick low -0.14->1.91


### Sheets (iteration 2)

- `devlog/shots/20261112-175941-B-lab-skf4-p2_cel-collisions.png`, `...-p2_cel-knockdowns.png`, `...-p2_cel-knockdowns-40px.png`
- the same three for `p3_clay` (`175942`) and `p4_broadcast` (`175942` / `175943`)

Collisions and knockdowns: 6 frames each from the game camera (left half) and from the side (right half). The 40 px sheet shows Knockdown_Left, Knockdown_Right, Knockdown_Back and Trip in blue and orange.

### Not verified

- **Rulings.** The text of R-0025 and R-0026 (not in my checkout).
- **Unreal.** Anything there, including:
  - how the engine picks Knockdown_Left / Right / Back from a hit direction;
  - whether `Check_Hit_*` reads at speed when the pawn moves.
- **Contacts.** Contact between the bodies is not modelled; the receiver and the checker are separate takes.
- **Skate lift on skinned figures.** It needs the retarget lane on `export4/`.
- **Static holds in Unreal.** Whether a constant 30-frame take imports correctly (B4 saw 2-frame holds lose their pose). The holds are 31 frames of identical keys, not 2.

## Fifth pass: lift, stumble, stronger contact takes (20:17 to 20:35)

The coordinator asked for this pass: the scripted figure is now the shipped default, and device A's bake-off and the engine lane named takes to fix. Blender only.

**Where the files are**
- New files: `C:\Users\kazda\jam\skater-lab\skf\export5\<style>\`, same three file names, **72 takes** (70 plus `Stumble_Short` and `Idle`).
- `export/` to `export4/` were not touched; nor was `s4/`.
- Work files: `skf\work5\<style>\` (`summary5.md`, `check_fbx*.json`, `clips_report.json`).
- Old-beside-new sheets: `skf\sheets5\<style>_old_new.png`.
- Rebuild: `SKF_SUFFIX=5 bash C:/Users/kazda/jam/skater-lab/skf/run_all.sh`, then `bash skf/sheet5.sh` (old-beside-new renders and the Stumble_Short start check).
- Summaries: `node summary5.mjs` and `node wrap5.mjs`, run from `skf/`.

### Take table

| Take | Frames (main file) / s | Contact | What changed |
|---|---|---|---|
| `Lift` | 22 / 0.35 | flick top at 8, held 8 to 10 | **Reworked as a hockey lift.** Knees bent and the chest down (pelvis drop 0.19 / 0.15), the blade low in front (heel 0.82 m ahead), then a flick up 36 to 40 cm with the bottom hand. The shaft stays mostly flat across the body (tilt 70 to 76 deg, leaning 32 deg to the left), not standing up as before (tilt 58 to 80 with the heel 42 cm up). Both hands on. Back to the carry at 21 (0.35 s). Iteration 2: deeper reach, the chest rises with the flick, 40 cm. |
| `Stumble_Short` (new) | 21 / 0.33 | frame 0 = `Poke` frame 6 | Exactly 20 frames from `Poke` frame 6, measured **0.0 cm** apart on every joint in all three styles. Weight thrown over the front (left) skate (pelvis pitch 40, back skate up), the stick dragging low, extreme 5 to 7, recovering at 13, Glide at 20. The solver starts from Poke's state at frame 5, so the elbows and twist match. `Stumble` is unchanged. |
| `Check_Hit_Left` / `_Right` / `_Back` | 29 / 0.47 | 3 | Displacement doubled: spine, head and clavicles twice as far, head lag plus a turn of 25 deg. The extreme reaches frame 5 and **holds 5 to 8**. A visible step at 14: 46 cm out to the side, or 34 cm forward for the hit from behind. Glide at 28. |
| `Poke_Hit` | 25 / 0.4 | 3 | The stick knocked far out to the right (heel 0.78 m right, yaw -85 deg), shoulders twisted (pelvis yaw -22, head 30), **held 5 to 8**, a half step of 42 cm at 14, Glide at 24. |
| `Lift_Hit` | 21 / 0.33 | 3 | The stick rises to 40 cm with the hands, the body jolts back (pelvis pitch 4, spine -6), **held 5 to 8**, Glide at 20. Iteration 2 carries the hands up with the stick; iteration 3 (final) keeps the bottom hand on the shaft. |
| `Board_Hit` | 31 / 0.5 | 4 | A bigger impact: pelvis drop 0.23, pelvis pitch 40 deg with hip roll and yaw (the hips follow), forearms and the near shoulder in, head turned 70 deg away, **held 6 to 9**. Rebound step **back 30 cm** at 17, Glide at 30. |
| `Check_Deliver_Left` | 31 / 0.5 | 16 | The stick now moves across to the left side with the hands. The stick and hand targets are mirrored about the body as well as the body itself. It starts from the real Glide, not a mirrored one. |
| `Trip` | 31 / 0.5 | 2 | **Unmistakable now.** The right leg is swept out sideways and up (skate 62 cm out, 42 cm up, frames 2 to 4), a half twist in the air (pelvis yaw 70 deg at 10), and it lands **on the left forearm and hip**. It ends in `Knockdown_Right`'s lying pose, so **`GetUp_Side_Right`** starts from it exactly. Before, it went flat onto the chest like `Knockdown_Back` and used `GetUp_Front`. |
| `Idle` (new) | 121 / 2.0 loop | - | Standing still in the carry pose, stance 19 cm each side, with a 2 s weight shift (pelvis roll ±3 deg). Hips **76.5..77.0 / 72.5..72.9 / 81.1..81.7 cm** (P2 / P3 / P4) against Glide's 76..77 / 72..72.9 / 80.4..81.7. `Idle_Faceoff` is unchanged for the faceoff (65.8..67 / 62.7..63.8 / 67.1..68.7). |

The main file repeats the last frame, so a 20-frame take has 21 frames (as every one-shot since s4); 0.33 s is frames 0 to 20.

**Iterations on my own sheets** (`sheets5/<style>_old_new.png`):
1. In the first build, `Lift` barely read from the game camera, and `Lift_Hit` looked almost unchanged. Iteration 2 gave `Lift` a deeper reach and a 40 cm flick, and carried the hands up with the stick in `Lift_Hit`.
2. That `Lift_Hit` took the bottom hand 19 to 26 cm off the shaft. Iteration 3 (final) keeps a 40 cm blade lift with both hands on and the body jolting back.
3. My first check of `Stumble_Short` against `Poke` frame 6 read 27 cm apart. The check was wrong: it read absolute frames, and the re-imported actions do not start at 0. Measured from each action's first frame, the match is 0.0 cm.

### Checks (re-imported main and loopcut files, all 72 takes)

**Unchanged:**
- pelvis and root travel 0.000 cm and loop seams 0.000 cm;
- wrap-speed offenders 0 in all styles;
- aim grid: holds within 0.04 cm, blend test 0 failures;
- old reaches exact;
- foot pitch max 20.3 to 21.8 / 25.0 to 25.1 deg;
- skate target miss 0;
- no body part below the ice;
- bottom hand on the shaft within 0.2 cm in the changed two-handed takes, except P3 `Lift` at 1.4 cm.

**Moved:**
- **Stick in the ice:** `Check_Hit_*` now dip the stick up to 0.9 cm (P3 `Check_Hit_Back` -0.93 cm, the stick blade grazing the ice in the stronger lean).
- **Steps over 45 deg** grew (P2 88 -> 107, P3 91 -> 112, P4 93 -> 117). These are the stronger extremes, reached in 2 frames and held 3.
- **New twist steps over 45 deg**, the same in all three styles:
  - `Check_Hit_Back` clavicles 52.5 deg at frame 4, the shoulders thrown forward in 2 frames;
  - `Poke_Hit` stick_socket 55.8 deg at frame 4, the stick knocked out (the stick bone, not a limb).
  - P4's two skate steps in `GetUp_Side_*` from the fourth pass remain.

### What is still weak

- **`Lift_Hit`.** Still mostly a stick motion: the body jolt is small at the game camera.
- **`Lift`.** The flick reads best from the side. From behind, the shaft across the body hides some of it.
- **Twist steps.** The two-frame extremes produce the clavicle and stick twist steps above. A skinned figure may show the shoulders snapping in `Check_Hit_Back`; a 3-frame rise would halve them.
- **`Stumble_Short`.** Its frame 0 is Poke frame 6 exactly, but the engine's cut will still blend; the takes are built for a hard cut.
- **`Trip` / `Knockdown_Right`.** Both now end in the same lying pose, on the left hip, so both use `GetUp_Side_Right`.
- **Not verified:** anything in Unreal (the bake-off's own look at the new `Lift`, the engine's cut timing for `Stumble_Short`), and skinned figures.

### Sheets

- `devlog/shots/20261112-203453-B-lab-skf5-p2_cel-old-new.png`, `20261112-203454-B-lab-skf5-p3_clay-old-new.png`, `20261112-203454-B-lab-skf5-p4_broadcast-old-new.png`

## Sixth pass: clay style, bands, stitches, felt (23:32 to 23:50)

The coordinator asked for this pass, for `p3_clay` only (`BP_SkaterVisual_TB1_P3` in the engine). Blender only.

**Where the files are**

| What | Folder |
|---|---|
| Shipped mesh | `C:\Users\kazda\jam\skater-lab\skf\export6\p3_clay\` (SK_Skater_p3_clay.fbx, _loopcut.fbx, SM_Stick_p3_clay.fbx), **without fuzz** |
| The fuzz variant | `skf\export6fz\p3_clay\` |
| Measurements | `skf\work6\p3_clay\` (`compare_anim*.json`, `check_fbx*.json`) |

- `export/` to `export5/` and `s4/` were not touched.
- Rebuild:
  - shipped: `SKF_SUFFIX=6 SKF_FUZZ=0 bash C:/Users/kazda/jam/skater-lab/skf/run_all.sh p3_clay`
  - fuzz variant: `SKF_SUFFIX=6fz SKF_FUZZ=1 bash .../run_all.sh p3_clay`
  - sheets: `bash skf/sheet6.sh`
  - Builder switch `--fuzz 0|1` or env `SKF_FUZZ`.

**Mesh-only re-import is safe** (`compare_anim.py`, `export5` against `export6`, main and loopcut files):
- same 24 bones and parents, rest matrices max difference **0.0**;
- 72 / 72 takes, same names; **672,051 keys (main) and 662,091 (loopcut) compared, max difference 0.0**, no curve missing.
- The clip pass is unchanged and re-checked: pelvis and root travel 0, seams 0; the old reaches 1.000 / 1.600 m with the same blade heights; aim grid within 0.03 cm, blend test 0 failures.
- The lowest point over all takes is -0.97 cm (the stick), against -0.96 in `export5`. No new below-ice point in any take.
- At most **1 bone weight per vertex** (rigid parts). The UV0 back-number projection is unchanged.

### What was added (all `Trim` unless stated)

| Item | How | Triangles |
|---|---|---|
| **Cream band on each upper sleeve** | 5 cm wide, 4.5 mm proud (edges 2 mm), ray-cast onto the clay surface (28 points round), bound to `upperarm_l/r` | about 170 each |
| **Two cream bands on each sock** | 4 cm wide, same build, bound to `calf_l/r` | about 170 each |
| **Hem band and collar ring** | the existing cream clay rolls (hem 2 cm and collar 2.4 cm round), kept | unchanged |
| **Running stitches** | **71 dashes of 4.0 x 1.4 x 0.5 cm, 2.4 cm gaps, 3 mm above the felt** (the texture lane's readable size). Around the hem 3.5 cm above the hem roll, round the collar 3.5 cm below the collar roll, down both sides, over both shoulders, and round both cuffs 3.5 cm from the cuff roll. They replace the old 3.0 x 0.9 cm cross-stitches and their jersey ridges. | 852 |
| Forearm rolls | removed (the concept bands the upper sleeves) | -400 |
| **Felt** | seeded fine displacement on every `Jersey` part (socks included), 1.5 mm, two octaves (60 and 130 per m), on top of the clay lumps, in the vertices and smooth normals | 0 |
| **Stick** | darker shaft (sRGB 0.17 / 0.12 / 0.10), the blade in `Stick`, with **cream tape** as a raised spiral (7 turns, 4.5 mm round cord) inside the blade height | +572 (984 in all) |
| **Fuzz** (variant only) | 600 two-sided fibre cards, 10 to 20 x 3 to 4.5 mm, on the upper chest and back, shoulder pads, outer upper sleeves and sock tops; slot **`Fuzz`**, bound to the bone of the cloth under it | 1,200 |

**Totals.** Shipped `export6`: **12,436** triangles (body 11,452 + stick 984), 9 slots as before. Fuzz variant: **13,636**, with a 10th slot `Fuzz`. The cap is 16k.

**Band and stitch deformation.** Every band and stitch is weighted 1.0 to the same bone as the cloth under it, so it moves rigidly with it.
- The bands sit 2 to 4.5 mm above the ray-cast surface at 28 points round, so they don't z-fight.
- Iteration 1 used 14 points round the sock: the felt bulged through between the points and the band broke into blocks (`lab-skf6-close` was re-filed after the fix).
- Checked on the rendered stride, slap and knockdown sheets: no tearing. The joins at elbows and knees are the existing ball-joint overlaps.

### Vertex colours (one layer `Col`, exported in the FBX as linear data; face-corner domain after import)

| Channel | Meaning |
|---|---|
| **R** | felt amount: Jersey and Trim (cloth, bands, stitches, patch) 1.0, Pads (breezers) 0.6, Fuzz 1.0, clay parts (helmet, gloves, skates, skin, blade, stick shaft) 0.0 |
| **G** | a random value per part (island), 0..0.97, for mottling |
| **B** | ambient occlusion baked per vertex: 12 hemisphere rays of 25 cm. Dark under the arms, between the legs, under the helmet rim, where the bands meet the felt. |
| A | 1 |

**What the engine should do**
- Read `Col` in the jersey and clay masters.
- R: mix the felt response (rim and roughness 1) with the clay response.
- G: a small brightness or hue jitter per part (mottling, about ±4 %).
- B: multiply it into the base colour or ambient (about 0.6 + 0.4 × B), so the figure gets depth without a texture.
- If the fuzz variant is used, the **`Fuzz` slot** needs:
  - the jersey's team colour;
  - two-sided rendering;
  - masked or dithered opacity that fades the cards towards their tips;
  - no shadow casting.

### Did the fuzz earn its place? No

On my own renders, close up it reads as a few stray hairs on the shoulders. At 8 m, 70 px and 40 px the fuzz and no-fuzz figures are indistinguishable (`lab-skf6-8m`, `-game70`, `-game40`, middle row against bottom row). It is noise, not wool, so `export6` ships without it. The variant stays in `export6fz` if the engine's dithered alpha makes more of it.

### What reads now (70 px and 40 px from the game camera, blue and orange)

At 70 px:
- the cream upper-sleeve bands and the two sock bands read clearly in both colours;
- the hem roll and the stitched band above it read as a cream line;
- the patch reads as before.

At 40 px the sleeve and sock bands still separate the limbs, and the stitches merge into the hem line.

### Still far from the concept

- **Not fuzzy wool.** The felt displacement is barely visible at our vertex density (2 to 3 cm spacing), so the jersey still reads as matte clay or felt, not woolly. True wool needs the engine (a fuzz shader or a normal map), or a much denser mesh.
- **Stitches.** The stitches are hard-edged boxes, not soft thread. They read as dashes, as wanted, but up close they look sculpted.
- **Clay thumbprints.** The helmet, gloves and skates have dents and lumps from the earlier passes, but no fingerprint ridges.
- **Sleeve bands.** The concept's bands sit near the shoulder and are wider; mine are at mid upper arm, 5 cm wide.
- **Tape wrap.** The tape spiral is visible on the blade only up close.
- **Not verified:** anything in Unreal (vertex-colour import, the `Col` layer on the skeletal mesh, how the masters use it).

### Sheets

- `devlog/shots/20261112-234642-B-lab-skf6-turntable.png` (old above new)
- `devlog/shots/20261112-234642-B-lab-skf6-close.png` (back, sleeve, sock: old / with fuzz / without fuzz)
- `devlog/shots/20261112-234642-B-lab-skf6-game70.png`
- `devlog/shots/20261112-234642-B-lab-skf6-game40.png`
- `devlog/shots/20261112-234642-B-lab-skf6-8m.png`
- `devlog/shots/20261112-234642-B-lab-skf6-knockdowns.png`

## Seventh and eighth pass: an athletic stride and posture fixes for the broadcast style

### Seventh pass (00:42 to 00:49; `export7`, used as delivered by lanes SKC4 and B21)

`skf/export7/p4_broadcast/` has 75 takes (72 plus `Stop_Hockey_Left`, `Stop_Hockey_Right`, `Push_Start`). Same skeleton as before.

- **Style flag `athletic`** in `skf_style.py` (p4 only): a new base Glide (`GLIDE_ATH`) that every Glide join uses.
- **Stride:** a long diagonal push to near extension, skate turned out 38 deg on its inside edge, support knee under the body, trunk 42 deg, shoulders counter-rotated.
- **Leans and crossovers:** on the same attitude.
- **Two new takes:** the two-foot hockey stops (contact frame 6, held 8 frames) and `Push_Start` (first push at frame 9).
- **Aim grid:** rebuilt on the new Glide.

What happened to the pass:
- `export7` was built at 00:45 with the aim-grid setting `aim_lean = -0.8` (blend test 0 failures). I had settled on 0.0, but the final full run, its sheets and this section never happened: the run stalled.
- I stopped without writing this section, and no pass-7 sheets were filed.

### Eighth pass (02:55 to 03:05; `export8`), the lead's posture fixes, p4 only

**Where the files are**
- New files: `skf/export8/p4_broadcast/`, same file names, `READY.txt` in `skf/export8/`.
- `export7` and the other styles were not touched.
- Rebuild: `SKF_SUFFIX=8 bash C:/Users/kazda/jam/skater-lab/skf/run_all.sh p4_broadcast`, then `bash skf/sheet8.sh` for the old-against-new sheets.
- Measurements: `measure8.py` (posture on the exported FBX, what a retarget sees) and `work8/posture.md`.

**The main finding**
- In Blender, `export7`'s Stride already leaned 42 deg with both hands on the stick (glove-to-shaft gap 0.0 cm).
- The upright chest and lifted glove in the engine come from two places:
  - **The aim layer.** It replaces everything from `spine_01` up with the aim hold's own mesh-space rotations. The holds carried Glide's trunk (25 to 32 deg) and, at 1.6 m, a head 17 deg up.
  - **The crossovers and leans.** They had the free-arm pump: the bottom glove was 75 to 95 cm off the shaft.
- So the holds now carry more trunk (36 deg) themselves, and the crossovers and leans are two-handed.

What changed per take:
- **Glide:** a ready stance. Feet just over shoulder width (±0.225 m), knees forward (pole straight ahead, not outward), hips back (skates 8 cm ahead of the pelvis), trunk 27.6 deg, head level, blade on the ice in front.
- **Idle:** the same stance and head, with a small weight shift. The stool look (upright 23 deg on deep knees, skates ±0.21 m) is gone.
- **Stride:** folded at the hips. Pelvis tilt 34 deg (was 28) plus spine, so a retarget keeps it: trunk **50.6 deg**. Head up. Push leg further back and low (0.38 m behind), support skate further forward (knee over the toe). Hips within 2 cm of Glide. Both hands on.
- **Crossovers and leans:** trunk 48 deg, **both hands on the stick**, blade in front on the ice. The bottom glove went from 75–95 cm off the shaft to 0, and the blade from 9.5–14.4 cm up to 1.9–4.0 cm.
- **Stops:** the head is level, looking along the travel (head elevation max 26.1 -> -2 deg), both hands on, body leaning into the stop.
- **Push_Start:** head up (-18 -> -2 deg at worst).
- **Shot_Wrist:** 8 deg more hip fold (trunk 23 -> 30.6 deg), the bottom glove closer (6.9 -> 4.2 cm).
- **Aim holds:** the extra trunk lean is a style setting (`aim_trunk` 1.0) with the head counter-tilted. Trunk 36 deg in every hold; head elevation at 1.6 m 10 deg (was 17). The blade-on-ice checks were re-run.
- **Pass and Faceoff_Ready:** checked and left (31 deg; 57 deg with the head on the dot, which is right for a faceoff).

### Measured posture (on the exported FBX, every frame; `work8/posture.md`)

| Take | Trunk from vertical, mean (min..max) deg: export7 -> export8 | Pelvis tilt deg | Head elevation deg (0 level, + up) | Bottom glove to shaft, max cm | Blade toe low cm |
|---|---|---|---|---|---|
| Glide | 31.6 (31.3..31.9) -> **27.6** (27.3..27.9) | 24..24 -> 20..20 | -4..0 -> -2..-2 | 0 -> 0 | 4.2 -> 4.2 |
| Idle | 23.1 (23.1..23.1) -> **26** (25.9..26) | 14..14.1 -> 18..18.1 | 4..4.1 -> 0..0.2 | 0 -> 0 | 4.2 -> 4.2 |
| Stride | 42 (41.7..42.3) -> **50.6** (50.3..50.9) | 27.3..28 -> 33.3..34 | -4..-3.2 -> -2..-1.2 | 0.1 -> 0.1 | 3.8 -> 2.9 |
| Crossover_Left | 44.6 (44.5..44.7) -> **47.9** (47.7..48) | 26.7..26.7 -> 28.7..28.7 | -4.6..-4.1 -> -4.5..-4.1 | 95.3 -> 0 | 13 -> 1.9 |
| Crossover_Right | 44.6 (44.5..44.7) -> **47.9** (47.7..48) | 26.7..26.7 -> 28.7..28.7 | -4.6..-4.1 -> -4.5..-4.1 | 82.7 -> 0 | 9.5 -> 3.8 |
| Lean_Left | 46.2 (46.1..46.3) -> **48** (47.9..48) | 26.7..26.8 -> 28.7..28.8 | -6.6..-6.4 -> -4.6..-4.4 | 94.9 -> 0 | 14.4 -> 1.9 |
| Lean_Right | 46.2 (46.1..46.3) -> **48** (47.9..48) | 26.7..26.8 -> 28.7..28.8 | -6.6..-6.4 -> -4.6..-4.4 | 75.5 -> 0 | 11.6 -> 4 |
| Push_Start | 44.8 (31.9..49.2) -> **45.4** (27.9..51.1) | 24..36 -> 20..36 | -18..0 -> -2..4 | 0.1 -> 0.1 | 2.5 -> 2.3 |
| Stop_Hockey_Right | 28.1 (22.2..31.9) -> **26.7** (21..31.7) | -21.2..24 -> -21.2..20.2 | 0..26.1 -> -12.3..-2 | 0.1 -> 0.1 | 3.5 -> 3.2 |
| Stop_Hockey_Left | 28.1 (22.2..31.9) -> **26.7** (21..31.7) | -21.2..24 -> -21.2..20.2 | 0..26.1 -> -12.3..-2 | 0.1 -> 0.1 | 2.9 -> 2.7 |
| Shot_Wrist | 23.1 (19.8..26.6) -> **30.6** (27..34.1) | 8.6..14.2 -> 16.4..22.2 | -5.8..1.7 -> -4.9..2.7 | 6.9 -> 4.2 | 1.8 -> 1.1 |
| Pass | 32.6 (31.9..33.1) -> **31** (27.9..33.1) | 19.3..24 -> 19.3..20 | -7..0 -> -7..-2 | 0 -> 0 | 0.5 -> 0.5 |
| Faceoff_Ready | 57.5 (57.4..57.6) -> **57.5** (57.4..57.6) | 34..34.1 -> 34..34.1 | -18.2..-18 -> -18.2..-18 | 0.1 -> 0.1 | 0.5 -> 0.5 |
| Aim_P00_R100 | 31.9 (31.9..31.9) -> **36.5** (36.5..36.5) | 24..24 -> 20..20 | -2..-2 -> 2..2 | 0.1 -> 0.1 | 2 -> 2 |
| Aim_P00_R130 | 28.5 (28.5..28.5) -> **36.5** (36.5..36.5) | 24..24 -> 20..20 | 7.6..7.6 -> 6..6 | 0.1 -> 0 | 1.8 -> 1.9 |
| Aim_P00_R160 | 25.1 (25.1..25.1) -> **36.5** (36.5..36.5) | 24..24 -> 20..20 | 17.2..17.2 -> 10..10 | 0 -> 0.1 | 1.8 -> 1.8 |
| Aim_N63_R160 | 25.2 (25.2..25.2) -> **35.9** (35.9..35.9) | 24..24 -> 20..20 | 16.9..16.9 -> 10.4..10.4 | 79.6 -> 80.7 | 1.8 -> 1.8 |
| Aim_P70_R160 | 25.2 (25.2..25.2) -> **35.8** (35.8..35.8) | 24..24 -> 20..20 | 16.8..16.8 -> 10.4..10.4 | 0.1 -> 0.1 | 2 -> 1.9 |

The trunk is measured as the angle of the pelvis -> neck line from vertical. Head elevation is the face's forward direction above horizontal: 0 is level, + is towards the sky. The hand gap is the bottom glove's grip point to the shaft line.

### Checks against export7

**Unchanged:**
- skeleton: rest matrices max difference 0.0;
- the same 75 take names, 60 fps, sockets unchanged;
- pelvis and root travel 0.000 cm (both files);
- loop seams 0.000 cm, wrap-speed offenders 0;
- no new point below the ice; skate target miss 0;
- foot pitch max 20.1 / 25.1 deg;
- reaches exactly 1.000 / 1.600 m;
- aim grid holds within 0.05 cm, blend test 0 failures (worst 3.37 cm, blade -1.9..1.9 cm);
- knockdown-to-get-up joins unchanged (those poses were not touched; Glide joins use the new Glide exactly).

**Locomotion hip heights (cm):**

| Take | Pelvis height |
|---|---|
| Glide | 76.8..77.8 |
| Stride | 76.2..76.9 |
| Crossovers | 75.9..78.5 |
| Leans | 77.7..77.8 |
| Hockey stops | 75.9..77.8 |
| Push_Start | 76.2..77.8 |
| Idle | 78.5..79.1 |
| The old `Stop` | 77.8..79.8 |

All are within 2 cm of Glide.

**Moved:** a new twist step over 45 deg in `GetUp` (the left skate turns 52 deg as it rises into the new, narrower-hipped Glide). Twist steps over 45 deg went 5 -> 6.

### What still looks wrong

- **The game's lean is the aim hold's (36 deg), not Stride's (50 deg).** That is how the engine layers the aim from `spine_01` up in mesh space. Showing the full 50 deg at speed would need either:
  - a speed-dependent pitch offset in the engine's layer;
  - or a second set of "skating" aim holds blended in by speed.
- **The bottom hand comes off in `Aim_N63_R160`** (far right at 1.6 m, by design one-handed): the glove is 81 cm off the shaft. When the game aims far right, the free glove shows.
- **`Shot_Wrist`:** the bottom glove is still 4.2 cm off the shaft at the follow-through, and the shot stays more upright than the stride.
- **Hockey stops:** the trunk is 21 to 32 deg, leaning back into the stop. The concept's stop throws snow from a lower body.
- **No checks in the engine.** Only rendered in Blender: no retarget onto P4E, no engine frames.

### Sheets (old export7 against new export8; rows: side old, side new, low behind old, low behind new)

`C:\Users\kazda\jam\thirdjam\devlog\shots\`:

| Take | Sheet |
|---|---|
| Stride | `20261113-030315-B-lab-skf8-stride.png` |
| Glide | `20261113-030315-B-lab-skf8-glide.png` |
| Idle | `20261113-030316-B-lab-skf8-idle.png` |
| Crossover_Left | `20261113-030316-B-lab-skf8-crossover-left.png` |
| Lean_Left | `20261113-030316-B-lab-skf8-lean-left.png` |
| Push_Start | `20261113-030317-B-lab-skf8-push-start.png` |
| Stop_Hockey_Right | `20261113-030317-B-lab-skf8-stop-hockey-right.png` |
| Stop_Hockey_Left | `20261113-030318-B-lab-skf8-stop-hockey-left.png` |
| Shot_Wrist | `20261113-030318-B-lab-skf8-shot-wrist.png` |
| Aim_P00_R130 | `20261113-030319-B-lab-skf8-aim-p00-r130.png` |

Each sheet has its `.txt` caption.

## Ninth pass: the hockey ready stance for the shipped toy styles (03:20 to 03:30)

The lead asked for the pass-8 posture fixes, scaled to the toy proportions (big head, short legs), on `p2_cel` (the night figure `BP_SkaterVisual_TB1`) and `p3_clay`.

**Where the files are**

| What | Path |
|---|---|
| New exports | `skf/export9/p2_cel/`, `skf/export9/p3_clay/` (same file names as before) |
| `READY.txt` | `skf/export9/` (files, takes changed, measurements, mesh provenance) |
| Posture tables | `work9/<style>/posture.md` |
| Rebuild | `bash C:/Users/kazda/jam/skater-lab/skf/run9.sh` (about 1 min for both) |

`export6`, `export7`, `export7c` and `export8` were not touched.

**Mesh: copied, not regenerated (an animation-only re-import)**
- `p2_cel`: the mesh of `export5/p2_cel`, the shipped night figure, from `work5/p2_cel/skater.blend`.
- `p3_clay`: the mesh of `export7c/p3_clay` (lane SK7: `SKF_SUFFIX=7c SKF_FUZZ=0 SKF_STITCH_SCALE=2.0`), from `work7c/p3_clay/skater.blend`.
- I used the source blends because re-running the clay builder is not bit-identical (the clay surface moved 3 mm between runs in an earlier pass).
- `compare_mesh.py` (new) against the shipped export: vertex positions, polygons, materials, bone weights, UV0 and vertex colours are identical (max difference 0.0) for body and stick.
- `compare_anim.py`: the same 24 bones, parents and rest matrices (0.0), the same 72 take names in both styles. Only the keys differ.

**What changed (style flag `ready`, the toy version of pass 8)**
- **Glide:** a ready stance. Skates ±0.20 m (a little over the 0.43 m shoulders), knees forward (pole straight ahead), hips back (skates 7 to 8 cm ahead of the pelvis), hips at a 10 cm drop (was 12), head level, both hands on the stick, blade on the ice in front.
- **Idle:** the same stance and head with a small weight shift.
- **Stride:** two-handed, blade in front, trunk about 33 deg, head up, push back and out, hips within 2 cm of Glide.
- **Crossovers and leans:** two-handed with the blade in front (the free-arm pump is gone).
- **Toy `Stop`:** head levelled (it looked 15 deg up).
- **24 aim holds and 4 aliases:** rebuilt on the new Glide. P3 needs extra trunk lean in its holds (`aim_trunk` 1.6) or its short top arm goes straight at 1.0 m on the right and the blend test fails (3 failures at 0.8, 1 at 1.2, 0 at 1.6). P2 needs none.
- **No hockey stops or `Push_Start`** in these styles (they stay at 72 takes).

### Measured posture (on the exported FBX, every frame)

**p2_cel** (shipped export5 -> export9)

| Take | Trunk deg, mean (min..max): shipped -> export9 | Head elevation deg | Bottom glove to shaft, max cm | Blade toe low cm |
|---|---|---|---|---|
| Glide | 29.6 (29.3..29.8) -> **22.3** (22..22.5) | -11..-5 -> -5..-4 | 1 -> 1 | 3.9 -> 3.9 |
| Idle | 23.3 (23.3..23.3) -> **20.6** (20.5..20.6) | -1..-0.9 -> -2..-1.8 | 1 -> 1 | 3.9 -> 3.9 |
| Stride | 40.7 (40.7..40.7) -> **32.8** (32.7..32.9) | -10..-9.2 -> -6..-5.6 | 127.9 -> 1 | 6.6 -> 3.9 |
| Crossover_Left | 39 (39..39) -> **39.6** (39.4..39.6) | -12.5..-11.3 -> -11.7..-11 | 95.4 -> 1 | 16.1 -> 2.8 |
| Crossover_Right | 39 (39..39) -> **39.6** (39.4..39.6) | -12.5..-11.3 -> -11.7..-11 | 83.2 -> 1 | 10.1 -> 3.9 |
| Lean_Left | 40.4 (40.3..40.5) -> **37.2** (37.2..37.3) | -14.1..-13.9 -> -8.5..-8.3 | 94.6 -> 1 | 16.4 -> 3 |
| Lean_Right | 40.4 (40.3..40.5) -> **37.2** (37.2..37.3) | -14.1..-13.9 -> -8.5..-8.3 | 76 -> 1 | 12.1 -> 3.9 |
| Stop | 26.4 (19.7..29.8) -> **25** (19.7..28.7) | -7.1..15.2 -> -15.5..1.4 | 1 -> 1 | 3.9 -> 3.9 |
| Shot_Wrist | 22 (18.8..25.5) -> **22** (18.8..25.5) | -5.8..1.7 -> -5.8..1.7 | 2.2 -> 2.2 | 2.4 -> 2.4 |
| Pass | 31 (29.8..31.8) -> **28.1** (22.5..31.8) | -7..-5 -> -7..-4 | 1 -> 1 | 0.4 -> 0.4 |
| Aim_P00_R100 | 29.8 (29.8..29.8) -> **22.5** (22.5..22.5) | -5..-5 -> 7..7 | 0.9 -> 0.9 | 1.7 -> 1.6 |
| Aim_P00_R130 | 33.7 (33.7..33.7) -> **26.4** (26.4..26.4) | -8..-8 -> 4..4 | 0.8 -> 0.8 | 1.5 -> 1.5 |
| Aim_P00_R160 | 37.5 (37.5..37.5) -> **30.2** (30.2..30.2) | -11..-11 -> 1..1 | 0.8 -> 0.9 | 1.6 -> 1.6 |
| Aim_N63_R100 | 29.4 (29.4..29.4) -> **22.3** (22.3..22.3) | -4.5..-4.5 -> 7.1..7.1 | 0.9 -> 0.8 | 1.7 -> 1.7 |
| Aim_P70_R160 | 36.8 (36.8..36.8) -> **29.7** (29.7..29.7) | -10..-10 -> 1.6..1.6 | 0.5 -> 0.6 | 1.8 -> 1.7 |
| GetUp | 34.8 (0.7..86) -> **33.3** (0.7..86) | -18..51 -> -18..51 | 125.2 -> 125.2 | 0.4 -> 0.4 |
| Idle_Faceoff | 51.9 (51.1..52.6) -> **51.9** (51.1..52.6) | -17.5..-14 -> -17.5..-14 | 1.1 -> 1.1 | 0.4 -> 0.4 |

**p3_clay** (shipped export7c -> export9)

| Take | Trunk deg, mean (min..max): shipped -> export9 | Head elevation deg | Bottom glove to shaft, max cm | Blade toe low cm |
|---|---|---|---|---|
| Glide | 29.7 (29.4..29.9) -> **22.3** (22..22.6) | -11..-5 -> -5..-4 | 0.5 -> 0.5 | 2.1 -> 2.1 |
| Idle | 23.4 (23.3..23.4) -> **20.6** (20.6..20.7) | -1..-0.9 -> -2..-1.8 | 0.5 -> 0.5 | 2.1 -> 2.1 |
| Stride | 40.8 (40.8..40.8) -> **32.9** (32.8..32.9) | -10..-9.2 -> -6..-5.6 | 127.2 -> 0.6 | 5.8 -> 1.9 |
| Crossover_Left | 39.1 (39.1..39.1) -> **39.6** (39.5..39.7) | -12.5..-11.3 -> -11.7..-11 | 96.4 -> 0.5 | 16 -> 1.8 |
| Crossover_Right | 39.1 (39.1..39.1) -> **39.6** (39.5..39.7) | -12.5..-11.3 -> -11.7..-11 | 85.5 -> 0.6 | 8.5 -> 2.1 |
| Lean_Left | 40.5 (40.5..40.6) -> **37.3** (37.2..37.4) | -14.1..-13.9 -> -8.5..-8.3 | 95.4 -> 0.5 | 16.3 -> 1.8 |
| Lean_Right | 40.5 (40.5..40.6) -> **37.3** (37.2..37.4) | -14.1..-13.9 -> -8.5..-8.3 | 78.8 -> 0.6 | 10 -> 2.1 |
| Stop | 26.4 (19.7..29.9) -> **24.9** (19.7..28.7) | -7.1..15.2 -> -15.5..1.4 | 0.6 -> 0.6 | 2.1 -> 2.1 |
| Shot_Wrist | 22.1 (18.9..25.5) -> **22.1** (18.9..25.5) | -5.8..1.7 -> -5.8..1.7 | 4.7 -> 4.7 | 1.8 -> 1.8 |
| Pass | 31.1 (29.9..31.9) -> **28.2** (22.6..31.9) | -7..-5 -> -7..-4 | 0.6 -> 0.6 | 1.8 -> 1.8 |
| Aim_P00_R100 | 29.9 (29.9..29.9) -> **35** (35..35) | -5..-5 -> 7..7 | 0.5 -> 0.5 | 3.2 -> 3.2 |
| Aim_P00_R130 | 33.8 (33.8..33.8) -> **38.9** (38.9..38.9) | -8..-8 -> 4..4 | 0.5 -> 0.5 | 3.1 -> 3.1 |
| Aim_P00_R160 | 37.7 (37.7..37.7) -> **42.8** (42.8..42.8) | -11..-11 -> 1..1 | 0.5 -> 0.5 | 3.1 -> 3.1 |
| Aim_N63_R100 | 29.5 (29.5..29.5) -> **34.4** (34.4..34.4) | -4.5..-4.5 -> 7.5..7.5 | 0.5 -> 0.6 | 2.1 -> 2 |
| Aim_P70_R160 | 37 (37..37) -> **41.9** (41.9..41.9) | -10..-10 -> 2..2 | 0.3 -> 0.4 | 3.2 -> 3.2 |
| GetUp | 34.9 (0.5..85.9) -> **33.4** (0.5..85.9) | -18..51 -> -18..51 | 119.6 -> 119.6 | 2 -> 2 |
| Idle_Faceoff | 52.1 (51.3..52.9) -> **52.1** (51.3..52.9) | -17.5..-14 -> -17.5..-14 | 0.6 -> 0.6 | 1.8 -> 1.8 |

The head elevation is the face's forward direction above horizontal: 0 is level, + is up.

### Checks (both styles, main and loopcut files)

**Unchanged:**
- pelvis and root travel 0.000 cm, loop seams 0.000 cm;
- wrap-speed offenders 0;
- no new point below the ice (lowest -1.41 cm in P2 and -0.91 in P3, as shipped);
- skate target miss 0; foot pitch max 21.5 / 25.1 and 22.2 / 25.0 deg;
- reaches 1.000 / 1.600 m;
- aim holds within 0.03 cm; blend test 0 failures (worst 3.14 and 2.84 cm, blade -2.2 / -2.8 cm at worst);
- knockdown-to-get-up joins unchanged;
- **no `GetUp` twist** (the pass-8 P4 skate step does not happen in these styles).

Pelvis height (cm), every locomotion take within 2 cm of Glide:

| Take | p2_cel | p3_clay |
|---|---|---|
| Glide | 78.0..78.5 | 73.8..74.3 |
| Idle | 78.5..79.0 | 74.3..74.8 |
| Stride | 77.5..78.0 | 73.4..73.8 |

**Moved:** the clavicle twist steps in `Check_Hit_Back` grew from 52.5 to 61.5 deg (frame 4), and `Knockdown_Back` gained a 45.7 deg clavicle step (frame 1). Both hits now start from the more upright Glide, so the shoulders travel further in the 2-frame hit.

### What still looks wrong

- **The game shows the hold's trunk, not Stride's (the pass-8 finding).**
  - P2's holds are 22 to 30 deg, so a skating P2 shows a little less lean than its Stride (33).
  - P3's holds are 34 to 43 deg, more than its Stride: the price of P3's blend test.
- **From the game camera the difference is modest.** The figure is about 40 px tall; the new stance reads as standing (legs under the hips, knees not splayed), but the toy proportions still make it chunky.
- **`Shot_Wrist`** was left as it was (22 deg; bottom glove 2.2 / 4.7 cm off the shaft at the follow-through).
- **`GetUp`** is one-handed by design while pushing up.
- **Clavicle steps** in the back hits, as above.
- **Not verified:** nothing has been seen in the engine.

### Sheets (`C:\Users\kazda\jam\thirdjam\devlog\shots\`, each with a `.txt`)

| Style | Glide behind | Stride | Idle | Crossover |
|---|---|---|---|---|
| p2_cel | `20261113-032917-B-lab-skf9-p2-cel-glide-behind.png` | `20261113-032918-B-lab-skf9-p2-cel-stride.png` | `20261113-032918-B-lab-skf9-p2-cel-idle.png` | `20261113-032918-B-lab-skf9-p2-cel-crossover.png` |
| p3_clay | `20261113-032919-B-lab-skf9-p3-clay-glide-behind.png` | `20261113-032919-B-lab-skf9-p3-clay-stride.png` | `20261113-032919-B-lab-skf9-p3-clay-idle.png` | `20261113-032919-B-lab-skf9-p3-clay-crossover.png` |

## Tenth pass: the faceoff stance and the clay aim lean (05:03 to 05:11)

The lead asked for this after the engine still (`sk8-stance-before-after`). It showed that the stool look is the **faceoff** pose: thighs horizontal, knees far outside the hips, the seat low. Pass 9 had not touched it.

**Where the files are**

| What | Path |
|---|---|
| New exports | `skf/export10/p2_cel/`, `skf/export10/p3_clay/` |
| `READY.txt` | `skf/export10/` |
| Rebuild | `bash C:/Users/kazda/jam/skater-lab/skf/run10.sh` (about 1 min for both) |

Earlier exports were not touched.

**Meshes and takes**
- **Meshes** are the same as export9: the mesh input is copied from `work9/<style>/skater.blend`, and `SM_Stick_<style>.fbx` are byte-identical copies.
- `compare_mesh.py`: vertices, polygons, materials, weights, UV0 and vertex colours are identical (0.0).
- Same 24 bones and rest pose, the same 72 take names.
- **Takes that differ from export9** (`compare_anim.py` now lists them; re-import only these):
  - `p2_cel` (3): `Faceoff_Draw`, `Faceoff_Ready`, `Idle_Faceoff`.
  - `p3_clay` (31): the same three, plus all 24 aim holds, plus the aliases `Aim_Front`, `Aim_Left`, `Aim_Right`, `Aim_FarReach`.
  - Every other take is identical to export9 (0.0).

**The faceoff (style flag `ready`; `p4_broadcast` keeps its own)**
- **Before:** `FACEOFF` dropped the hips 21 cm with the knee poles 0.5 outward and the skates ±0.27 m.
- **Now:**
  - skates ±0.26 m (about 1.2 x shoulder width);
  - knee poles straight ahead (the knees go forward over the toes);
  - skates 9 to 15 cm ahead of the pelvis (hips back);
  - hip drop 14 cm, so the thighs sit at about 42 deg;
  - trunk folded to about 50 deg, head up looking at the dot;
  - bottom hand low on the shaft (0.60 down it), blade flat at the dot.
- `Faceoff_Ready` builds on it; `Faceoff_Draw` starts in the new `Faceoff_Ready` and ends in Glide, with its own sweep as before.

**p2_cel** (export9 -> export10)

| Take | Trunk deg | Thigh below horizontal deg | Knee spread cm (hips) | Skate spread cm | Head deg | Bottom glove to shaft cm | Blade toe low cm |
|---|---|---|---|---|---|---|---|
| Idle_Faceoff | 51.9 -> **49** | 27.8 -> **41.6** | 70.4 -> **44.5** (23) | 55.9 -> 52 | -17.5..-14 -> -9.5..-6 | 1.1 -> 1 | 0.4 -> 0.4 |
| Faceoff_Ready | 55.2 -> **50.3** | 28.9 -> **42.6** | 69.9 -> **44.3** (23) | 55.9 -> 52 | -18.2..-18 -> -8.2..-8 | 1.1 -> 1 | 0.5 -> 0.4 |
| Faceoff_Draw | 41.8 -> **39.3** | 37.3 -> **40.3** | 58.4 -> **41.9** (22.5) | 51 -> 49.1 | -18..-4 -> -8..-3.7 | 1.1 -> 1.1 | 0.3 -> 0.4 |

| Aim row | Trunk deg, export9 -> export10 |
|---|---|
| R100 | 22.3..22.5 -> **22.3..22.5** |
| R130 | 26..26.4 -> **26..26.4** |
| R145 | 27.8..28.3 -> **27.8..28.3** |
| R160 | 29.7..30.2 -> **29.7..30.2** |

One-handed holds: Aim_N63_R160, Aim_N46_R160. Holds blade 1.12..2.3 cm above the ice, within 0.03 cm of their points; blend test worst 3.14 cm, blade -2.2..1.9 cm, failures 0.

Checks: pelvis 0 cm, root 0 cm, loop seams 0 cm, wrap-speed offenders 0, lowest point -1.41 cm, reaches 1 / 1 / 1 / 1.6 / 1.6 m.

**p3_clay** (export9 -> export10)

| Take | Trunk deg | Thigh below horizontal deg | Knee spread cm (hips) | Skate spread cm | Head deg | Bottom glove to shaft cm | Blade toe low cm |
|---|---|---|---|---|---|---|---|
| Idle_Faceoff | 52.1 -> **49.1** | 27.5 -> **41.6** | 69.8 -> **46.4** (26) | 58.4 -> 54.7 | -17.5..-14 -> -9.5..-6 | 0.6 -> 0.6 | 1.8 -> 1.7 |
| Faceoff_Ready | 55.4 -> **50.4** | 28.6 -> **42.7** | 69.3 -> **46.3** (26) | 58.4 -> 54.7 | -18.2..-18 -> -8.2..-8 | 0.6 -> 0.5 | 1.7 -> 1.8 |
| Faceoff_Draw | 42 -> **39.4** | 37 -> **40.4** | 58.9 -> **44** (25.5) | 53.9 -> 52 | -18..-4 -> -8..-3.7 | 0.6 -> 0.6 | 1.6 -> 1.7 |

| Aim row | Trunk deg, export9 -> export10 |
|---|---|
| R100 | 34.3..35 -> **22.3..29.5** |
| R130 | 38.1..38.9 -> **26..29.7** |
| R145 | 40..40.8 -> **27.9..28.4** |
| R160 | 41.9..42.8 -> **29.8..30.3** |

One-handed holds: Aim_N63_R100, Aim_N63_R130, Aim_N63_R145, Aim_N63_R160, Aim_N46_R100, Aim_N46_R130, Aim_N46_R145, Aim_N46_R160, Aim_N30_R145, Aim_N30_R160, Aim_P00_R160. Holds blade 0.22..2.01 cm above the ice, within 0.03 cm of their points; blend test worst 2.46 cm, blade -2.95..1.87 cm, failures 0.

Checks: pelvis 0 cm, root 0 cm, loop seams 0 cm, wrap-speed offenders 0, lowest point -0.91 cm, reaches 1 / 1 / 1 / 1.6 / 1.6 m.

**The clay aim lean: what was traded**
- The pass-9 extra lean (`aim_trunk` 1.6, holds at 34 to 43 deg) is gone. The holds now carry Glide's 22 deg plus the normal reach lean.
- To keep the blend check passing with P3's short top arm on the right at 1.0 to 1.3 m:
  1. **Three more holds are one-handed**: `Aim_N46_R100`, `Aim_N63_R100` (= `Aim_Right`) and `Aim_N46_R130`. The bottom hand comes off and hangs beside the hip, as in the far-right cells.
  2. **A little extra lean only on the right side of the near rows**: up to +7 deg at -63 deg in the 1.0 m row (29.5 deg at most), half of that in the 1.3 m row (29.7 at most).
- The far row is as before in kind (29.8 to 30.3 deg). Without the extra lean the blend check failed 3 times; with the lean in the 1.0 m row only, 4 times. I did not use a pelvis shift or a top hand sliding down the shaft.


### What still looks wrong

- **The faceoff thighs are at about 42 deg, not 45.** Lowering them further pushes the knees out again on the toy's short legs.
- **The knees are now 44 to 46 cm apart, against hips of 23 to 26 cm.** Still about 10 cm outside each hip, but inside the skates (52 to 55 cm): from behind it reads as a crouch over the dot, no longer as seated.
- **At `Aim_Right` (1.0 m, -63 deg) the clay figure aims one-handed.** In the game, aiming to the right at short range shows the bottom glove off the shaft beside the hip.
- **`p4_broadcast` has the same faceoff squat** (not built; render filed as `lab-skf10-p4-broadcast-faceoff-note`): knees out, and its bottom glove is up off the shaft at `Faceoff_Ready`.
- **Not verified:** nothing has been seen in the engine.

### Sheets (`C:\Users\kazda\jam\thirdjam\devlog\shots\`, each with a `.txt`)

| Style | Sheet | What it shows |
|---|---|---|
| p2_cel | `20261113-051011-B-lab-skf10-p2-cel-faceoff-gamecam.png` | Idle_Faceoff and Faceoff_Ready, old against new, game camera |
| p2_cel | `20261113-051011-B-lab-skf10-p2-cel-faceoff-side.png` | the same from the side |
| p2_cel | `20261113-051012-B-lab-skf10-p2-cel-faceoff-draw.png` | Faceoff_Draw, old against new |
| p3_clay | `20261113-051012-B-lab-skf10-p3-clay-faceoff-gamecam.png` | Idle_Faceoff and Faceoff_Ready, old against new, game camera |
| p3_clay | `20261113-051012-B-lab-skf10-p3-clay-faceoff-side.png` | the same from the side |
| p3_clay | `20261113-051013-B-lab-skf10-p3-clay-faceoff-draw.png` | Faceoff_Draw, old against new |
| p3_clay | `20261113-051013-B-lab-skf10-p3-clay-aim-rest-side.png` | three aim holds from the side, old against new |
| p4_broadcast | `20261113-051013-B-lab-skf10-p4-broadcast-faceoff-note.png` | the P4 faceoff fault, not built |

## Eleventh pass: the tackling takes (SKF11, 09:45 to 09:58)

The operator made tackling the key feature: "the ability to tackle other players to the ground, collide and animate fall, getting up". Styles `p2_cel` and `p3_clay`.

**Where the files are**

| What | Path |
|---|---|
| Exports | `skf/export11/<style>/`, full take sets |
| `READY.txt` | `skf/export11/` |
| Strips | `skf/sheets11/before/`, `skf/sheets11/after/` |
| Before/after sheets | `skf/sheets11/ba/` |
| Rebuild | `bash skf/run11.sh`, then `bash skf/part1.sh export11 r11p1n sheets11/after`, `bash skf/pairs.sh export11 r11pairn sheets11/after`, `bash skf/compose11.sh` |

### Part 1: what export10 had (both styles move the same; the toy proportions differ)

**Strips:** every 3rd frame, game camera (behind, 35 deg down, orthographic, figure about 110 px) and side (300 px).

**Pair sheet assumption:** the sim knocks down at contact distance. I placed the victim 0.35 m ahead and 0.55 m to the side of the attacker, so the knocked side faces the attacker's driving shoulder (0.65 m straight ahead for the hit from behind). Contact is `Check_Deliver_*` frame 16 against `Knockdown_*` frame 2.

At 0.65 m the toy bodies already overlap at contact, so the game's contact distance for these wide figures is probably larger, or the attacker's drive should stop shorter. I have not verified which.

**Critique per take (before, judged at 110 px):**

| Take | Verdict |
|---|---|
| `Knockdown_Left` / `_Right` | **Fail.** You can tell a hit and its side by frame 3, but the body crumples into a ball on the spot. No legs going out, no airborne or sliding moment, a soft landing, and the end pose reads as seated, not lying. |
| `Knockdown_Back` | **Weak.** Pitched forward onto hands and knees, then chest: readable, but slow (31 frames), no bounce. |
| `Fall` | **Fail.** Tips back like a board over 12 frames with the arms folded at the chest; no airborne moment; a long slow lie-down. |
| `Fall_Forward` | **Weak.** Same board tip forward, arms out late. |
| `Trip` | **Passable** since the fifth pass (leg swept, half twist, side landing), but 31 frames. |
| `GetUp`, `GetUp_Front` | **Wrong length.** 73 frames: with the fall the game plays them 1.2x fast and there is no time lying. They do show stages (roll, sit, knee, up). |
| `GetUp_Side_*` | 55 frames, same problem; sit, kneel on the stick, up. |
| `Check`, `Check_Deliver_*` | **Weak.** Wind 9, drive 16, follow-through 21 to 30: the shoulder drive reads from the side, but from the game camera it is a body turn without a clear lunge; the recovery is fine. |
| `Check_Hit_*` | **Passable** since the fifth pass: a 2-frame snap, head lag, a step; the extreme is held 3 frames. Small at 110 px. |
| `Stumble`, `Stumble_Short` | **Passable.** Forward pitch, arms out; reads as a stumble. |
| `Board_Hit` | **Weak.** The impact reads from the side, little from behind. `Board_Pin` is a static hold, fine. |
| `Foul_Reaction` | Arms spread, head shake: reads, not a fall. |

### Part 2: reworked (knockdowns, falls, trip, get-ups); NOT reworked this pass

Not reworked: `Check_Deliver_*` / `Check`, `Check_Hit_*`, `Stumble`, `Board_Hit`. They are identical to export10.

**Changes (style flag `ready`; p4 untouched)**

*Falls* (`Fall`, `Fall_Forward`, `Knockdown_Back`, `Knockdown_Left`, `Knockdown_Right`, `Trip`), 26 frames each, contact at frame 2:
- a 2-frame snap to the extreme, with the head and arms lagging and the stick swung wide (still in the top hand);
- to frame 6, the legs going out from under (an airborne moment);
- impact at 10, a bounce at 13, a settle at 17, lying to 26.
- The side knockdowns now lie flatter on the hip and forearm (pelvis roll 80 instead of 66). The first rework still read as seated; this one also sank P2's torso 6.4 cm into the ice, so the hip drop is a style setting (-0.535 P2, -0.60 P3).
- `Trip` keeps its sweep and twist, retimed.

*Get-ups*, 44 frames each:
- from the lying pose, hands push (sit or push up) at 8 to 9;
- one knee at 18 to 20;
- rise at 28 to 30;
- a small overshoot into the ready stance at 36 to 37;
- the ready Glide at 44.

**Timing:** 26 + 44 = 70 of the Blueprint's 90 frames, so about 20 frames lying and no speed-up.

**Lying pose per direction** (a fall's last frame is its get-up's first frame, exactly):

| Fall | Lying pose | Get-up |
|---|---|---|
| `Fall` | on the back | `GetUp` |
| `Fall_Forward`, `Knockdown_Back` | face down | `GetUp_Front` |
| `Knockdown_Left` | right hip | `GetUp_Side_Left` |
| `Knockdown_Right`, `Trip` | left hip | `GetUp_Side_Right` |

Each fall starts from the ready Glide and each get-up ends in it, as before.

**Proof**
- **Takes that differ from export10:** exactly these 10 in each style (`compare_anim.py` now counts takes of a different length as different).
- **Everything else:** identical (0.0), including the clay aim holds of export10.
- **Mesh:** identical; stick FBX byte-identical.
- **Travel and seams:** root and pelvis travel 0.000 cm, loop seams 0.

**p2_cel** (export10 -> export11)

| Take | Frames before -> after | Lowest body / skate / stick cm (after) | Worst part (cm) | Steps > 45 deg before -> after | Twist > 45 deg (bone, frame) |
|---|---|---|---|---|---|
| Fall | 37 -> 27 | 0 / 0 / 0.73 | skates 0 | 4 -> 12 | clavicle_l f1 -58.5, clavicle_r f1 58.5 |
| Fall_Forward | 37 -> 27 | 0 / 0 / 0.73 | skates 0 | 2 -> 5 | none |
| Knockdown_Back | 31 -> 27 | 0 / 0 / 0.58 | skates 0 | 9 -> 12 | clavicle_l f1 66.7, clavicle_r f1 -66.7 |
| Knockdown_Left | 31 -> 27 | -0.5 / 0 / -0.5 | skates 0 | 6 -> 14 | none |
| Knockdown_Right | 31 -> 27 | 0 / 0 / 0.73 | skates 0 | 6 -> 13 | none |
| Trip | 31 -> 27 | 0 / 0 / 0.73 | skates 0 | 8 -> 9 | none |
| GetUp | 73 -> 45 | 0 / 0 / 0.1 | skates 0 | 2 -> 5 | foot_l f14 45.5, foot_l f23 -48.9 |
| GetUp_Front | 73 -> 45 | -1.41 / 0 / -1.41 | skates 0 | 0 -> 0 | none |
| GetUp_Side_Left | 55 -> 45 | -0.63 / 0 / -0.63 | skates 0 | 2 -> 3 | foot_l f25 -48.9 |
| GetUp_Side_Right | 55 -> 45 | -0.62 / 0 / -0.62 | skates 0 | 3 -> 3 | foot_l f25 -48.9 |

All takes: pelvis horizontal 0 cm, root 0 cm, loop seams 0 cm, lowest anything -1.41 cm (export10 -1.41).

**p3_clay** (export10 -> export11)

| Take | Frames before -> after | Lowest body / skate / stick cm (after) | Worst part (cm) | Steps > 45 deg before -> after | Twist > 45 deg (bone, frame) |
|---|---|---|---|---|---|
| Fall | 37 -> 27 | 0.13 / 0.13 / 0.34 | skates 0.13 | 5 -> 13 | clavicle_l f1 -58.6, clavicle_r f1 58.5 |
| Fall_Forward | 37 -> 27 | 0.13 / 0.13 / 0.34 | skates 0.13 | 2 -> 9 | none |
| Knockdown_Back | 31 -> 27 | -0.63 / 0.13 / -0.63 | skates 0.13 | 9 -> 12 | clavicle_l f1 66.8, clavicle_r f1 -66.7 |
| Knockdown_Left | 31 -> 27 | -0.37 / 0.13 / -0.37 | skates 0.13 | 7 -> 14 | none |
| Knockdown_Right | 31 -> 27 | 0.13 / 0.13 / 0.34 | skates 0.13 | 6 -> 14 | none |
| Trip | 31 -> 27 | 0.13 / 0.13 / 0.34 | skates 0.13 | 9 -> 10 | none |
| GetUp | 73 -> 45 | 0.13 / 0.13 / 0.34 | skates 0.13 | 2 -> 4 | foot_l f14 49.8, foot_l f23 -52.7 |
| GetUp_Front | 73 -> 45 | -0.91 / 0.13 / -0.91 | skates 0.13 | 0 -> 0 | none |
| GetUp_Side_Left | 55 -> 45 | -0.55 / 0.13 / -0.55 | skates 0.13 | 2 -> 3 | foot_l f25 -52.7 |
| GetUp_Side_Right | 55 -> 45 | 0.13 / 0.13 / 0.15 | skates 0.13 | 3 -> 3 | foot_l f25 -52.7 |

All takes: pelvis horizontal 0 cm, root 0 cm, loop seams 0 cm, lowest anything -0.91 cm (export10 -0.91).

### What still fails

- **`Knockdown_Left` / `_Right` from the side.** The body falls away from the side camera and still reads as a heap. From the game camera it reads as down on the hip.
- **Snap twist steps.** In `Fall` and `Knockdown_Back` the clavicles turn 58 to 67 deg about their own axis between frames 1 and 2. The left-shoulder reach rule switches off when the hands leave the stick. A skinned figure may show a shoulder pop.
- **Get-up skate turns.** The left skate turns 45 to 53 deg in one frame at the kneel transitions of every get-up.
- **No knee or elbow flips.** The bend limiter engaged a few frames per take.
- **Unreworked takes are still weak.** `Check_Deliver_*`, `Check`, `Check_Hit_*`, `Stumble` and `Board_Hit`: the attacker's drive is the next job.
- **Pairing distance.** The contact distance in the pair sheets overlaps the bodies; the game's real contact distance for these figures is unverified.
- **Not verified:** nothing has been seen in the engine; the Blueprint's hold-and-get-up timing is assumed from the brief.

### Stills (`C:\Users\kazda\jam\thirdjam\devlog\shots\`, prefix `20261113-0957xx-B-lab-skf11-`, each with a `.txt`)

| Still | What it shows |
|---|---|
| `p2-cel-part1-gamecam`, `p2-cel-part1-side`, `p3-clay-part1-gamecam`, `p3-clay-part1-side` | all 21 takes before |
| `p2-cel-pairs-before`, `-pairs-after`, `p3-clay-pairs-before`, `-pairs-after` | the attacker/victim pairs |
| `<style>-<take>-ba` (20) | each reworked take, before above after |
| `tackle-before-after` | the operator sheet |

## Twelfth pass: the second tackling round (SKF12, 10:02 to 10:24)

Delivered `C:\Users\kazda\jam\skater-lab\skf\export12\` (p2_cel, p3_clay, full sets) with `READY.txt`. Item 1 (the attacker) and item 2 (the falls) are complete. Item 3 (`Check_Hit_*`, `Stumble`, `Board_Hit`) is not started; those takes are identical to export10.

**Difference list against export11 (both styles):** exactly 13 takes differ: `Check`, `Check_Deliver_Right`, `Check_Deliver_Left`, `Fall`, `Fall_Forward`, `Knockdown_Back`, `Knockdown_Left`, `Knockdown_Right`, `Trip`, `GetUp`, `GetUp_Front`, `GetUp_Side_Left`, `GetUp_Side_Right`. Every other take, the clay aim holds included, is identical. Meshes, bones, rest pose and sockets are identical (diff 0.0); `SM_Stick` is byte-identical; the root never moves.

### What changed

| Take | Before (export11) | After (export12) |
|---|---|---|
| `Check`, `Check_Deliver_Right`, `Check_Deliver_Left` | A body turn whose drive peaked at frame 16, after every possible hit | A lunge from the button press. Frame 3 is the extreme: hips 23 cm lower, trunk 65 deg forward (was 22), the lead shoulder low and ahead, the stick pulled back and across, the back leg straight behind as a push. Frame 12 is deeper still, so a hit anywhere in 0-12 meets the peak. Follow-through at 19, ready Glide at 28. 30 frames; `Check` keeps the `Deliver_Right` keys. |
| `Fall` | Lands with the knees up | Legs thrown up above the pelvis line at frames 4-10 with the head snapping forward. Lies as a starfish on the back: legs straight and 73 cm apart, arms out, stick to the right. The body is thrown 30 cm back. |
| `Fall_Forward`, `Knockdown_Back` | Face down with the knees bent | Skates up behind (above the pelvis at frames 6-9 / 7-9). Lies face down with the legs straight and spread 67 cm, arms out at the shoulders, stick ahead to the right. Thrown 30 cm forward. |
| `Knockdown_Left` / `_Right` | A seated heap on the hip | Legs flung up and out to the hit side, above the pelvis at frames 4-10, head snapping the other way. Lies long on the far side across the camera: legs straight along the ice opposite the head, top leg and top arm out, the bottom arm stretched past the head, the stick flat. Thrown 30 cm away from the hit. |
| `Trip` | | Same sweep and twist, ending in `Knockdown_Right`'s long lying pose |
| The four get-ups | | Start exactly on the new lying poses. The 30 cm throw comes back by half at the push (frame 8-9) and fully at the kneel (18-20). Stages and 44 frames as in pass 11. |

**Timing:**
- Falls: 26 frames, contact at 2, airborne 5-8, impact 11, bounce 14, settle 18, lying 18-26.
- Get-ups: 44 frames.
- 26 + 44 = 70 of the 90 sim frames, so about 20 frames are spent lying.

**Joins (fall's last frame equals the get-up's first, exact):**

| Fall | Lying pose | Get-up |
|---|---|---|
| `Fall` | starfish on the back | `GetUp` |
| `Fall_Forward`, `Knockdown_Back` | face down | `GetUp_Front` |
| `Knockdown_Left` | long on the right side | `GetUp_Side_Left` |
| `Knockdown_Right`, `Trip` | long on the left side | `GetUp_Side_Right` |

**Solver changes:**
- Body offset `px`/`py`: the pelvis and every armature-space target move together. Zero in every other take, so they are numerically unchanged.
- Tackling takes only:
  - the left clavicle's reach decays over a few frames instead of snapping back when the hands leave the stick;
  - a skate turns at most 18 deg per frame in world space, unless that would put a blade under the ice.

### Pops reported in pass 11

| Pop | Result |
|---|---|
| Clavicle turn at frames 1-2 of `Fall` / `Knockdown_Back` | Their clavicles' local one-frame turn is now at most 8 deg. Across all 13 takes the largest is 23.5 deg (p2) / 32.7 deg (p3), in the `Check` lunge at frames 1-3. |
| Skate turn in the get-ups | The largest local skate step in the get-ups went from 110 / 98 deg to 47-55 / 40-45 deg. In world space the skate turns at most 18 deg per frame except where that would cut the ice. |
| Still open | The arms swing 106-149 deg in the one frame after contact (frame 1 to 2) in every fall: the 2-frame snap. The arms read as flung, but a skinned elbow may show it. |

### Pair sheets at 0.8 m

- **Setup:** the hit at attacker frame 2, 6 and 12 (`Deliver_Right` against `Knockdown_Left`), plus `Deliver_Left` against `Knockdown_Right` and the hit from behind at frame 6.
- **Victim slide:** after contact the victim slides along the hit direction as the sim would move it. I assumed 60 % of a 4 m/s closing speed (2.4 m/s), losing 63 % every 0.5 s, so about 1.0 m by contact +24.
- **Every case reads:** the attacker is in the lunge at contact, and the victim is airborne, then long on the ice and sliding away.
- **Correction to pass 11:** its pair render left the victim at the attacker's spot. The victim's location was set before the frame change, and the take's object keys reset it to 0. So pass 11's "the bodies overlap at contact" came from that error, not from the distance. Fixed in `pair_render12.py`.

### Per take (p2_cel)

| Take | Frames | Lowest body (part) cm | Lowest skate / stick cm | Biggest one-frame step (bone) before -> after | Pelvis thrown cm | Trunk deg min/max | Skate spread cm |
|---|---|---|---|---|---|---|---|
| Check | 31 -> 31 | 13.96 (knees_shins) | -0.00 / -0.61 | 44 (upperarm_r f15->16) -> 83 (stick_socket f1->2) | 0 | 22/65 | 46.3 |
| Check_Deliver_Right | 31 -> 31 | 13.96 (knees_shins) | -0.00 / -0.61 | 44 (upperarm_r f15->16) -> 83 (stick_socket f1->2) | 0 | 22/65 | 46.3 |
| Check_Deliver_Left | 31 -> 31 | 13.94 (knees_shins) | -0.00 / 0.73 | 40 (clavicle_r f16->17) -> 96 (stick_socket f1->2) | 0 | 22/65 | 46.3 |
| Fall | 27 -> 27 | 3.63 (hands) | -0.00 / 0.73 | 144 (lowerarm_l f1->2) -> 144 (lowerarm_l f1->2) | 30 | 22/86 | 72.7 |
| Fall_Forward | 27 -> 27 | 1.60 (head) | -0.00 / 0.73 | 131 (lowerarm_r f1->2) -> 131 (lowerarm_r f1->2) | 30 | 22/89 | 67.3 |
| Knockdown_Back | 27 -> 27 | 1.60 (head) | -0.00 / 0.58 | 122 (upperarm_l f1->2) -> 122 (upperarm_l f1->2) | 30 | 22/89 | 66.8 |
| Knockdown_Left | 27 -> 27 | -0.62 (arms) | -0.00 / 0.18 | 108 (upperarm_l f1->2) -> 106 (upperarm_l f1->2) | 30 | 22/103 | 17.4 |
| Knockdown_Right | 27 -> 27 | -0.62 (arms) | -0.00 / 0.73 | 128 (upperarm_l f1->2) -> 129 (upperarm_l f1->2) | 30 | 22/103 | 17.9 |
| Trip | 27 -> 27 | -0.68 (arms) | -0.00 / 0.73 | 135 (lowerarm_l f1->2) -> 135 (lowerarm_l f1->2) | 30 | 22/87 | 37.3 |
| GetUp | 45 -> 45 | 1.28 (hands) | -0.00 / 0.10 | 79 (foot_l f14->15) -> 47 (thigh_l f14->15) | 30 | 3/86 | 45.9 |
| GetUp_Front | 45 -> 45 | 1.60 (head) | -0.00 / -1.41 | 31 (thigh_l f15->16) -> 31 (thigh_l f15->16) | 30 | 22/89 | 46.1 |
| GetUp_Side_Left | 45 -> 45 | -0.62 (arms) | -0.00 / -1.05 | 75 (foot_l f25->26) -> 46 (foot_l f15->16) | 30 | 22/87 | 36.2 |
| GetUp_Side_Right | 45 -> 45 | -0.62 (arms) | -0.00 / -0.62 | 75 (foot_l f25->26) -> 55 (foot_l f15->16) | 30 | 22/87 | 36.1 |

### Per take (p3_clay)

| Take | Frames | Lowest body (part) cm | Lowest skate / stick cm | Biggest one-frame step (bone) before -> after | Pelvis thrown cm | Trunk deg min/max | Skate spread cm |
|---|---|---|---|---|---|---|---|
| Check | 31 -> 31 | 12.39 (hands) | 0.13 / -0.80 | 44 (upperarm_r f15->16) -> 83 (stick_socket f1->2) | 0 | 23/65 | 49.5 |
| Check_Deliver_Right | 31 -> 31 | 12.39 (hands) | 0.13 / -0.80 | 44 (upperarm_r f15->16) -> 83 (stick_socket f1->2) | 0 | 23/65 | 49.5 |
| Check_Deliver_Left | 31 -> 31 | 13.05 (knees_shins) | 0.13 / 0.34 | 40 (neck_01 f16->17) -> 96 (stick_socket f1->2) | 0 | 23/65 | 49.5 |
| Fall | 27 -> 27 | 2.96 (hands) | 0.13 / 0.34 | 145 (lowerarm_l f1->2) -> 143 (lowerarm_l f1->2) | 30 | 23/86 | 72.6 |
| Fall_Forward | 27 -> 27 | 2.67 (hands) | 0.13 / 0.34 | 123 (hand_l f1->2) -> 126 (lowerarm_l f1->2) | 30 | 23/89 | 68.3 |
| Knockdown_Back | 27 -> 27 | 2.67 (hands) | 0.13 / -0.63 | 109 (upperarm_l f1->2) -> 116 (upperarm_l f1->2) | 30 | 23/89 | 67.8 |
| Knockdown_Left | 27 -> 27 | -1.19 (forearms) | 0.13 / 0.34 | 113 (upperarm_l f1->2) -> 110 (upperarm_l f1->2) | 30 | 23/103 | 16.0 |
| Knockdown_Right | 27 -> 27 | -1.70 (arms) | 0.13 / 0.34 | 109 (upperarm_l f1->2) -> 93 (head f1->2) | 30 | 23/103 | 17.7 |
| Trip | 27 -> 27 | -1.43 (arms) | -0.19 / 0.34 | 146 (lowerarm_l f1->2) -> 149 (lowerarm_l f1->2) | 30 | 23/87 | 36.3 |
| GetUp | 45 -> 45 | 1.33 (hands) | 0.13 / 0.34 | 76 (foot_l f23->24) -> 42 (stick_socket f4->5) | 30 | 4/86 | 49.0 |
| GetUp_Front | 45 -> 45 | 1.68 (hands) | 0.13 / -0.91 | 30 (thigh_l f15->16) -> 30 (thigh_l f15->16) | 30 | 23/89 | 49.2 |
| GetUp_Side_Left | 45 -> 45 | -0.13 (forearms) | 0.13 / -0.78 | 76 (foot_l f25->26) -> 48 (upperarm_r f13->14) | 30 | 23/87 | 39.3 |
| GetUp_Side_Right | 45 -> 45 | -0.44 (arms) | 0.12 / 0.15 | 76 (foot_l f25->26) -> 38 (lowerarm_r f15->16) | 30 | 23/87 | 39.3 |

**How to read the tables:**
- Frames are key counts, 31 = 30 frames.
- Steps are whole-bone one-frame turns.
- "Pelvis thrown" is the intended body throw. The root stays 0.
- Airborne frames (Blender frames, 1 = take frame 0), ankles above the pelvis:

  | Take | p2_cel | p3_clay |
  |---|---|---|
  | `Fall` | 5-11 | 5-11 |
  | `Fall_Forward` | 6-10 | 7-9 |
  | `Knockdown_Back` | 7-10 | 7-9 |
  | `Knockdown_Left` / `_Right` | 5-11 | 5-11 |

**Ground:**
- Body and skates are at worst -0.62 cm (p2) and -1.70 cm (p3), both arms in the side lying pose.
- The stick is at worst -1.41 cm (p2 `GetUp_Front`, as in export10).
- The other takes are unchanged: -0.77 / -0.88 cm.

### What still fails

1. **Item 3 is not done:** `Check_Hit_*`, `Stumble`, `Board_Hit` are export10's.
2. **Face-down arms:** the arms are spread at the shoulders rather than thrown forward past the head. The toy's short arms stop at about 0.5 m.
3. **Arm swing at contact:** 106-149 deg in one frame (frame 1 to 2) in every fall.
4. **Attacker's stick:** it whips from the carry to behind the body in 2 frames (83-96 deg per frame at frames 1-2).
5. **Side lying arms:** the bottom shoulder's upper arm dips 0.6-1.7 cm into the ice.
6. **Not verified in the engine:** the sim's slide and hit timing are taken from the coordinator's reading of `JamSim.cpp`.

### Files

- **Scripts** (`skater-lab\skf\`):
  - `run12.sh` (build, checks, compare against export11)
  - `summary12.py`, `steps12.py`, `air12.py`
  - `pairs12.sh` + `pair_render12.py`
  - `part1.sh export12`
  - `compose12.sh`
- **Sheets:** `sheets12\after`, `sheets12\before`, `sheets12\ba`.
- **Stills** (`devlog\shots\20261113-1023xx-B-lab-skf12-*`, 29):

  | Still | Content |
  |---|---|
  | `tackle-before-after` | the operator sheet: export10 above export12, both styles, attacker and victim at 0.8 m (hit at attacker frame 6), then `GetUp_Side_Left` |
  | `<style>-pairs-before`, `-pairs-after` | the pairs at frames 2, 6, 12 plus the right and back cases |
  | `<style>-<take>-ba` (24) | game camera, export10 above export12 |

## Thirteenth pass: hits taken standing, the board hit, stumbles, cleanups, spin variants (SKF13, 10:26 to 10:35)

Delivered `C:\Users\kazda\jam\skater-lab\skf\export13\` (p2_cel, p3_clay, full sets) with `READY.txt`. All five items are done; what still fails is listed below.

### Difference list against export12

| Takes (both styles) | Change |
|---|---|
| `Check_Hit_Back`, `Check_Hit_Left`, `Check_Hit_Right`, `Board_Hit`, `Stumble`, `Stumble_Short` | New work |
| `Check`, `Check_Deliver_Right`, `Check_Deliver_Left`, `Fall`, `Fall_Forward`, `Knockdown_Back`, `Knockdown_Left`, `Knockdown_Right`, `Trip`, `GetUp_Front`, `GetUp_Side_Left`, `GetUp_Side_Right` | Cleanups |
| `Knockdown_Left_B`, `Knockdown_Right_B` | New takes (74 in all). The existing 72 keep their names. |

- **Unchanged:** `GetUp`, `Board_Pin` and every other take. The clay aim holds are identical to export10.
- **Rig:** meshes, bones, rest pose and sockets are identical; `SM_Stick` is byte-identical; the root never moves.

### What changed

| Take | Before (export10/12) | After (export13) |
|---|---|---|
| `Check_Hit_Left` / `_Right` / `_Back` | A small lean and one step, 28 frames | Contact at frame 0. A 2-frame snap away from the push with the head lagging. The far arm and the stick are thrown out for balance (sideways: the arm on the push side up and out, the stick out; from behind: both arms up and forward). Two staggering steps: the skate on the push side slides out (5-7), the other crosses under and catches (11-13). The body is carried up to 30 cm along the push and back, overshoots against it at 20, and is in the ready Glide at 30. 30 frames. |
| `Board_Hit` | A brace and a turn of the head | Flattened upright as against a wall in front at 3-6: body straight, hips up, arms and stick raised over the head, head back. A rebound step back at 10 (pelvis 16 cm back), a head shake at 13-19, Glide at 30. It reads from every side because the arms and stick go up. |
| `Stumble`, `Stumble_Short` | A forward pitch | Pitched over and rolled to the right. The left arm windmills (back-up, forward-up, down-forward at 4, 8, 12), the stick goes out right, the right skate kicks up and out, then catches wide. 20 frames; `Stumble_Short` still starts exactly on `Poke` frame 6. |
| Cleanups | | 1. Falls and standing hits: the contact snap uses a linear ease, so the arm swing is spread over frames 0-2. 2. Check: the stick whip is spread over 3 frames and pulled back less (140 deg). 3. Side lying pose raised 2 cm: the lower arm is now 0.4-1.8 cm above the ice. 4. Face down: the arms are thrown forward with the elbows forward and the hands beside the helmet; `GetUp_Front` starts from that pose. |
| `Knockdown_Left_B` / `_Right_B` (new) | | The same snap, airborne moment and landing as the A versions, with one full turn about the vertical (frames 2-11, about 40 deg per frame). The legs and skates turn with the body, and it ends on the A version's lying pose, so the same get-up joins. The last frames match the A last frame in rotation and location exactly; 5 bones carry the negated quaternion of the same rotation. |

### Pairs at 0.8 m

- **Setup:** the pusher plays plain Glide. The victim's `Check_Hit_*` starts on the collision. After contact the victim is pushed on at half the knockdown slide (assumed 1.2 m/s, losing 63 % every 0.5 s). Both keep their feet.
- **Result:** from behind and above, the push direction is unmistakable in all three cases.

### Per take (p2_cel)

| Take | Frames | Lowest body (part) cm | Lowest skate / stick cm | Biggest one-frame step (bone) before -> after | Pelvis thrown cm | Trunk deg min/max | Skate spread cm |
|---|---|---|---|---|---|---|---|
| Check_Hit_Back | 29 -> 31 | 13.97 (knees_shins) | -0.00 / 0.73 | 122 (upperarm_l f4->5) -> 144 (hand_l f2->3) | 30 | 12/93 | 39.3 |
| Check_Hit_Left | 29 -> 31 | 12.20 (knees_shins) | -0.00 / 0.20 | 111 (upperarm_l f4->5) -> 130 (hand_l f2->3) | 30 | 22/74 | 41.7 |
| Check_Hit_Right | 29 -> 31 | 12.20 (knees_shins) | -0.00 / 0.73 | 126 (upperarm_l f4->5) -> 140 (lowerarm_l f2->3) | 30 | 22/74 | 41.7 |
| Board_Hit | 31 -> 31 | 13.97 (knees_shins) | -0.00 / 0.73 | 72 (hand_r f1->2) -> 91 (upperarm_l f1->2) | 16 | 4/38 | 39.1 |
| Stumble | 21 -> 21 | 13.69 (knees_shins) | -0.00 / 0.73 | 66 (upperarm_l f1->2) -> 90 (upperarm_l f1->2) | 6 | 22/56 | 54.0 |
| Stumble_Short | 21 -> 21 | 13.73 (knees_shins) | -0.00 / 0.71 | 57 (upperarm_l f1->2) -> 52 (lowerarm_r f2->3) | 6 | 22/56 | 54.1 |
| Knockdown_Left_B | 0 -> 27 | 0.80 (arms) | -0.00 / 0.16 | 0 (new) -> 103 (calf_l f4->5) | 30 | 22/103 | 14.8 |
| Knockdown_Right_B | 0 -> 27 | 1.75 (arms) | -0.00 / 0.73 | 0 (new) -> 99 (hand_l f2->3) | 30 | 22/103 | 15.1 |
| Check | 31 -> 31 | 13.96 (knees_shins) | -0.00 / -0.61 | 83 (stick_socket f1->2) -> 43 (stick_socket f1->2) | 0 | 22/65 | 46.2 |
| Check_Deliver_Right | 31 -> 31 | 13.96 (knees_shins) | -0.00 / -0.61 | 83 (stick_socket f1->2) -> 43 (stick_socket f1->2) | 0 | 22/65 | 46.2 |
| Check_Deliver_Left | 31 -> 31 | 13.94 (knees_shins) | -0.00 / 0.73 | 96 (stick_socket f1->2) -> 51 (stick_socket f2->3) | 0 | 22/65 | 46.2 |
| Fall | 27 -> 27 | 3.63 (hands) | -0.00 / 0.73 | 144 (lowerarm_l f1->2) -> 124 (lowerarm_l f2->3) | 30 | 11/86 | 72.7 |
| Fall_Forward | 27 -> 27 | 1.60 (head) | -0.00 / 0.73 | 131 (lowerarm_r f1->2) -> 100 (upperarm_l f1->2) | 30 | 22/89 | 67.3 |
| Knockdown_Back | 27 -> 27 | 1.60 (head) | -0.00 / 0.56 | 122 (upperarm_l f1->2) -> 87 (upperarm_l f1->2) | 30 | 22/89 | 66.8 |
| Knockdown_Left | 27 -> 27 | 1.75 (arms) | -0.00 / 0.15 | 106 (upperarm_l f1->2) -> 86 (lowerarm_r f2->3) | 30 | 22/103 | 17.3 |
| Knockdown_Right | 27 -> 27 | 1.75 (arms) | -0.00 / 0.73 | 129 (upperarm_l f1->2) -> 95 (lowerarm_l f2->3) | 30 | 22/103 | 17.9 |
| Trip | 27 -> 27 | 1.74 (arms) | -0.00 / 0.73 | 135 (lowerarm_l f1->2) -> 135 (lowerarm_l f1->2) | 30 | 22/87 | 37.2 |
| GetUp | 45 -> 45 | 1.28 (hands) | -0.00 / 0.10 | 47 (thigh_l f14->15) -> 47 (thigh_l f14->15) | 30 | 3/86 | 45.9 |
| GetUp_Front | 45 -> 45 | 1.60 (head) | -0.00 / -1.41 | 31 (thigh_l f15->16) -> 31 (thigh_l f15->16) | 30 | 22/89 | 46.1 |
| GetUp_Side_Left | 45 -> 45 | 1.75 (arms) | -0.00 / -1.05 | 46 (foot_l f15->16) -> 46 (foot_l f15->16) | 30 | 22/87 | 36.2 |
| GetUp_Side_Right | 45 -> 45 | 1.75 (arms) | -0.00 / -0.62 | 55 (foot_l f15->16) -> 55 (foot_l f15->16) | 30 | 22/87 | 36.1 |

### Per take (p3_clay)

| Take | Frames | Lowest body (part) cm | Lowest skate / stick cm | Biggest one-frame step (bone) before -> after | Pelvis thrown cm | Trunk deg min/max | Skate spread cm |
|---|---|---|---|---|---|---|---|
| Check_Hit_Back | 29 -> 31 | 13.06 (knees_shins) | 0.13 / 0.33 | 93 (upperarm_l f4->5) -> 150 (lowerarm_l f2->3) | 30 | 12/94 | 43.1 |
| Check_Hit_Left | 29 -> 31 | 12.46 (knees_shins) | 0.13 / -0.08 | 116 (upperarm_l f4->5) -> 135 (hand_l f2->3) | 30 | 23/74 | 43.5 |
| Check_Hit_Right | 29 -> 31 | 12.09 (knees_shins) | 0.13 / 0.34 | 108 (upperarm_l f4->5) -> 131 (lowerarm_l f2->3) | 30 | 23/74 | 43.5 |
| Board_Hit | 31 -> 31 | 13.10 (knees_shins) | 0.13 / 0.34 | 71 (lowerarm_r f1->2) -> 116 (upperarm_l f1->2) | 16 | 4/38 | 42.9 |
| Stumble | 21 -> 21 | 13.05 (knees_shins) | 0.13 / 0.34 | 68 (upperarm_l f1->2) -> 90 (upperarm_l f1->2) | 6 | 23/56 | 56.4 |
| Stumble_Short | 21 -> 21 | 13.07 (knees_shins) | 0.13 / 0.33 | 60 (upperarm_l f1->2) -> 53 (hand_r f2->3) | 6 | 23/56 | 56.5 |
| Knockdown_Left_B | 0 -> 27 | 0.69 (forearms) | 0.13 / 0.34 | 0 (new) -> 105 (calf_l f4->5) | 30 | 23/103 | 14.0 |
| Knockdown_Right_B | 0 -> 27 | 0.44 (arms) | 0.13 / 0.34 | 0 (new) -> 105 (lowerarm_l f2->3) | 30 | 23/103 | 14.5 |
| Check | 31 -> 31 | 12.43 (hands) | 0.13 / -0.69 | 83 (stick_socket f1->2) -> 44 (stick_socket f1->2) | 0 | 23/65 | 49.4 |
| Check_Deliver_Right | 31 -> 31 | 12.43 (hands) | 0.13 / -0.69 | 83 (stick_socket f1->2) -> 44 (stick_socket f1->2) | 0 | 23/65 | 49.4 |
| Check_Deliver_Left | 31 -> 31 | 13.05 (knees_shins) | 0.13 / 0.34 | 96 (stick_socket f1->2) -> 51 (stick_socket f2->3) | 0 | 23/65 | 49.4 |
| Fall | 27 -> 27 | 2.96 (hands) | 0.13 / 0.34 | 143 (lowerarm_l f1->2) -> 89 (thigh_l f3->4) | 30 | 11/86 | 72.6 |
| Fall_Forward | 27 -> 27 | 7.20 (hands) | 0.13 / 0.34 | 126 (lowerarm_l f1->2) -> 104 (upperarm_l f1->2) | 30 | 23/89 | 68.3 |
| Knockdown_Back | 27 -> 27 | 7.65 (hands) | 0.13 / -0.65 | 116 (upperarm_l f1->2) -> 96 (upperarm_l f1->2) | 30 | 23/89 | 67.8 |
| Knockdown_Left | 27 -> 27 | 0.69 (forearms) | 0.13 / 0.34 | 110 (upperarm_l f1->2) -> 88 (lowerarm_r f2->3) | 30 | 23/103 | 16.2 |
| Knockdown_Right | 27 -> 27 | 0.44 (arms) | 0.13 / 0.34 | 93 (head f1->2) -> 102 (lowerarm_l f2->3) | 30 | 23/103 | 17.8 |
| Trip | 27 -> 27 | 0.57 (arms) | -0.20 / 0.34 | 149 (lowerarm_l f1->2) -> 149 (lowerarm_l f1->2) | 30 | 23/87 | 36.4 |
| GetUp | 45 -> 45 | 1.33 (hands) | 0.13 / 0.34 | 42 (stick_socket f4->5) -> 42 (stick_socket f4->5) | 30 | 4/86 | 49.0 |
| GetUp_Front | 45 -> 45 | 1.95 (hands) | 0.13 / -0.91 | 30 (thigh_l f15->16) -> 30 (thigh_l f15->16) | 30 | 23/89 | 49.2 |
| GetUp_Side_Left | 45 -> 45 | 1.82 (forearms) | 0.13 / -0.78 | 48 (upperarm_r f13->14) -> 48 (upperarm_r f13->14) | 30 | 23/87 | 39.3 |
| GetUp_Side_Right | 45 -> 45 | 1.56 (arms) | 0.12 / 0.15 | 38 (lowerarm_r f15->16) -> 38 (lowerarm_r f15->16) | 30 | 23/87 | 39.3 |

**Ground:**
- No body part or skate is under the ice in any changed or new take (0.0 cm p2, 0.13 cm p3).
- The stick is at worst -1.41 cm (p2 `GetUp_Front`, as in export10).

### What still fails

1. **Arm swing after contact:** the snap now has a linear ease, but the arms still swing 87-150 deg in a single frame between frames 0 and 2, in the falls and in the standing hits. The left hand leaving the stick for a balance position crosses near the body. Fixing it needs an elbow-path key, not just easing.
2. **`Board_Hit` after the rebound:** the stick is held out to the right, and the arms read as "spread" rather than "shaken".
3. **`Board_Pin`:** unchanged; I did not judge it bad enough from behind to spend time on it.
4. **Spin variants:** the turn is fast (about 40 deg per frame). They are only checked at the game camera.
5. **Not verified in the engine.** The pusher and slide numbers are assumptions.

### Files

- **Scripts** (`skater-lab\skf\`):
  - `run13.sh`, `summary13.py`
  - `pairs13.sh` (`pair_render12.py` now takes a slide scale)
  - `part1.sh export13`
  - `compose13.sh`, `joinb13.py`
- **Stills** (`devlog\shots\20261113-1033xx-B-lab-skf13-*`, 43):

  | Still | Content |
  |---|---|
  | `hits-before-after` | the operator sheet: export10 left, export13 right; rows are the standing hit from the left (pair), from behind (pair), `Board_Hit`, `Stumble`; both styles |
  | `<style>-hitpairs-before`, `-hitpairs-after` | the three standing hits at 0.8 m |
  | `<style>-<take>-ba` (34) | game camera, export10 above export13 |
  | `<style>-Knockdown_Left_B-new`, `-Knockdown_Right_B-new` | the A version above its spin variant |

## Fourteenth pass: fixes from the engine (SKF14, 10:48 to 10:59)

Delivered `C:\Users\kazda\jam\skater-lab\skf\export14\` (p2_cel, p3_clay, full sets) with `READY.txt`. Items 1 to 5 are done and item 6 is checked. The take count stays 74, with the same names.

**Difference list against export13:** 11 takes per style differ: `Idle_Faceoff`, `Faceoff_Ready`, `Faceoff_Draw`, `Check`, `Check_Deliver_Right`, `Check_Deliver_Left`, `GetUp_Side_Left`, `GetUp_Side_Right`, `Knockdown_Left`, `Knockdown_Right`, `Board_Hit`. Everything else is identical to export13; the clay aim holds are identical to export10. Meshes, bones, sockets and the root are unchanged.

### 1. The faceoff from behind

The shaft now stands exactly at the blade's lie, so the blade stays flat on the ice. The blade heel is placed far enough ahead that the knob is 8 cm in front of the pelvis, beside the right hip. The bottom hand is a third of the way down the shaft. The body (crouch, knee spread, skates, trunk) is export10's. `Faceoff_Draw` sweeps with the knob at the right side, then ends in the ready Glide as before.

| Measure (whole take) | p2_cel export10 | p2_cel export14 | p3_clay export10 | p3_clay export14 |
|---|---|---|---|---|
| Knob height cm (`Idle_Faceoff` / `Faceoff_Ready`) | 106.9 / 106.0 | 90.0 / 90.0 | 100.4 / 99.6 | 72.7 / 72.7 |
| Knob above the shoulders cm | +8.9 / +7.1 | -8.0 / -8.8 | +7.3 / +5.7 | -20.4 / -21.2 |
| Knob behind the pelvis cm | 30.9 / 30.3 | -7.9 (in front) | 26.9 / 26.3 | -8.0 (in front) |
| `Faceoff_Draw`: knob above the shoulders cm | +13.0 | -7.5 | +10.7 | -20.2 |
| Lowest stick point above the ice cm | 0.16 | 0.69 | 0.69-0.80 | 0.77 |
| Knee spread cm | 44.5 | 44.5 | 46.4 | 46.4 |

- **From the game camera directly behind:** the silhouette is helmet, shoulders, back and hips, and the stick shows only in front and to the left. Both hands stay on the stick (gap 1.1 / 0.5 cm).
- **First try failed:** the shaft 3 deg past the lie lifted the blade 4-6 cm off the ice, so it was put back at the lie exactly.

### 2. The check by frame 2

- **Timing:** the drive extreme is at frame 2 and goes deeper through frame 12. The stick only starts back at frame 2 (yaw 45) and reaches the pulled-back carry at frame 6.
- **Stick turn** (world, per frame): at most 23.6 deg (`Deliver_Right`) and 29.3 deg (`Deliver_Left`), down from 43 / 51.
- **Hands:** they travel up to 48 cm in frame 0-1. That is the lunge itself.

### 3. Side get-ups

The left skate now lifts 12 cm at frame 13 and 10 cm at frame 17 while it turns. Its largest world turn per frame is 28.4 deg (p2) and 29.4 deg (p3), down from 46-55 and 43. `GetUp` / `GetUp_Front` were not touched (28-29 deg).

### 4. Arm swing at contact (`Knockdown_Right` first)

The extra key: at the contact frame the bottom hand is only half way to its flung position, so it leaves the stick on a shorter path.

| Take | Left upper arm deg/frame (export13 → 14) | Left hand cm/frame (export13 → 14) |
|---|---|---|
| `Knockdown_Right` | 80 → 55 (p2), 89 → 61 (p3) | 40 → 33 (p2), 38 → 33 (p3) |
| `Knockdown_Left` | 70 → 53 (p2), 72 → 63 (p3) | unchanged |

`Fall` (up to 115 deg), `Fall_Forward`, `Knockdown_Back`, the standing hits and the spin variants are not changed yet.

### 5. `Board_Hit`

After the rebound the stick comes back in, carried in front with both hands, through the head shake. The shake now reads as a shake, not as arms spread.

### 6. `Glide`, `Stride`, `Idle` from behind and above

Not changed: the top hand sits at the hip, not above the back, so it is not as plain as the faceoff.

- **Height:** the knob stands at 80-86 cm, 22-29 cm under the shoulders, so nothing stands above the shoulders.
- **Position:** it sits 25-33 cm behind the pelvis, beside the right hip.
- **From behind:** the top glove shows at the right hip as a small dark shape. It is visible in the last frames of `Faceoff_Draw` on the operator sheet. If it should go, the same change as the faceoff applies, but it changes every locomotion take and the aim joins.

### Per take (p2_cel)

| Take | Frames | Lowest body (part) cm | Lowest skate / stick cm | Biggest one-frame step (bone) before -> after | Pelvis thrown cm | Trunk deg min/max | Skate spread cm |
|---|---|---|---|---|---|---|---|
| Idle_Faceoff | 121 -> 121 | 13.65 (knees_shins) | -0.00 / 0.69 | 4 (lowerarm_r f46->47) -> 0 (lowerarm_r f25->26) | 0 | 48/50 | 52.0 |
| Faceoff_Ready | 31 -> 31 | 13.84 (knees_shins) | -0.00 / 0.69 | 0 (upperarm_r f8->9) -> 0 (lowerarm_r f6->7) | 0 | 50/50 | 52.0 |
| Faceoff_Draw | 19 -> 19 | 13.56 (knees_shins) | -0.00 / 0.62 | 22 (hand_r f1->2) -> 22 (lowerarm_r f1->2) | 0 | 22/50 | 49.1 |
| Check_Hit_Back | 31 -> 31 | 13.97 (knees_shins) | -0.00 / 0.73 | 144 (hand_l f2->3) -> 144 (hand_l f2->3) | 30 | 12/93 | 39.3 |
| Check_Hit_Left | 31 -> 31 | 12.20 (knees_shins) | -0.00 / 0.20 | 130 (hand_l f2->3) -> 130 (hand_l f2->3) | 30 | 22/74 | 41.7 |
| Check_Hit_Right | 31 -> 31 | 12.20 (knees_shins) | -0.00 / 0.73 | 140 (lowerarm_l f2->3) -> 140 (lowerarm_l f2->3) | 30 | 22/74 | 41.7 |
| Board_Hit | 31 -> 31 | 13.97 (knees_shins) | -0.00 / 0.71 | 91 (upperarm_l f1->2) -> 91 (upperarm_l f1->2) | 16 | 4/38 | 39.1 |
| Stumble | 21 -> 21 | 13.69 (knees_shins) | -0.00 / 0.73 | 90 (upperarm_l f1->2) -> 90 (upperarm_l f1->2) | 6 | 22/56 | 54.0 |
| Stumble_Short | 21 -> 21 | 13.73 (knees_shins) | -0.00 / 0.71 | 52 (lowerarm_r f2->3) -> 52 (lowerarm_r f2->3) | 6 | 22/56 | 54.1 |
| Knockdown_Left_B | 27 -> 27 | 0.80 (arms) | -0.00 / 0.16 | 103 (calf_l f4->5) -> 103 (calf_l f4->5) | 30 | 22/103 | 14.8 |
| Knockdown_Right_B | 27 -> 27 | 1.75 (arms) | -0.00 / 0.73 | 99 (hand_l f2->3) -> 99 (hand_l f2->3) | 30 | 22/103 | 15.1 |
| Check | 31 -> 31 | 13.96 (knees_shins) | -0.00 / -0.61 | 43 (stick_socket f1->2) -> 56 (lowerarm_r f2->3) | 0 | 22/65 | 46.3 |
| Check_Deliver_Right | 31 -> 31 | 13.96 (knees_shins) | -0.00 / -0.61 | 43 (stick_socket f1->2) -> 56 (lowerarm_r f2->3) | 0 | 22/65 | 46.3 |
| Check_Deliver_Left | 31 -> 31 | 13.94 (knees_shins) | -0.00 / 0.33 | 51 (stick_socket f2->3) -> 51 (clavicle_l f1->2) | 0 | 22/65 | 46.3 |
| Fall | 27 -> 27 | 3.63 (hands) | -0.00 / 0.73 | 124 (lowerarm_l f2->3) -> 124 (lowerarm_l f2->3) | 30 | 11/86 | 72.7 |
| Fall_Forward | 27 -> 27 | 1.60 (head) | -0.00 / 0.73 | 100 (upperarm_l f1->2) -> 100 (upperarm_l f1->2) | 30 | 22/89 | 67.3 |
| Knockdown_Back | 27 -> 27 | 1.60 (head) | -0.00 / 0.56 | 87 (upperarm_l f1->2) -> 87 (upperarm_l f1->2) | 30 | 22/89 | 66.8 |
| Knockdown_Left | 27 -> 27 | 1.75 (arms) | -0.00 / 0.15 | 86 (lowerarm_r f2->3) -> 86 (lowerarm_r f2->3) | 30 | 22/103 | 17.3 |
| Knockdown_Right | 27 -> 27 | 1.75 (arms) | -0.00 / 0.73 | 95 (lowerarm_l f2->3) -> 77 (calf_l f3->4) | 30 | 22/103 | 17.9 |
| Trip | 27 -> 27 | 1.74 (arms) | -0.00 / 0.73 | 135 (lowerarm_l f1->2) -> 135 (lowerarm_l f1->2) | 30 | 22/87 | 37.2 |
| GetUp | 45 -> 45 | 1.28 (hands) | -0.00 / 0.10 | 47 (thigh_l f14->15) -> 47 (thigh_l f14->15) | 30 | 3/86 | 45.9 |
| GetUp_Front | 45 -> 45 | 1.60 (head) | -0.00 / -1.41 | 31 (thigh_l f15->16) -> 31 (thigh_l f15->16) | 30 | 22/89 | 46.1 |
| GetUp_Side_Left | 45 -> 45 | 1.75 (arms) | -0.00 / -1.05 | 46 (foot_l f15->16) -> 33 (thigh_l f15->16) | 30 | 22/87 | 36.2 |
| GetUp_Side_Right | 45 -> 45 | 1.75 (arms) | -0.00 / 0.09 | 55 (foot_l f15->16) -> 44 (calf_l f15->16) | 30 | 22/87 | 36.1 |


### Per take (p3_clay)

| Take | Frames | Lowest body (part) cm | Lowest skate / stick cm | Biggest one-frame step (bone) before -> after | Pelvis thrown cm | Trunk deg min/max | Skate spread cm |
|---|---|---|---|---|---|---|---|
| Idle_Faceoff | 121 -> 121 | 13.12 (knees_shins) | 0.13 / 0.77 | 8 (lowerarm_l f18->19) -> 0 (upperarm_l f8->9) | 0 | 48/50 | 54.7 |
| Faceoff_Ready | 31 -> 31 | 13.12 (knees_shins) | 0.13 / 0.78 | 0 (upperarm_r f7->8) -> 0 (lowerarm_l f7->8) | 0 | 50/50 | 54.7 |
| Faceoff_Draw | 19 -> 19 | 13.10 (knees_shins) | 0.13 / 0.34 | 23 (lowerarm_r f1->2) -> 24 (lowerarm_r f1->2) | 0 | 23/50 | 52.0 |
| Check_Hit_Back | 31 -> 31 | 13.06 (knees_shins) | 0.13 / 0.33 | 150 (lowerarm_l f2->3) -> 150 (lowerarm_l f2->3) | 30 | 12/94 | 43.1 |
| Check_Hit_Left | 31 -> 31 | 12.46 (knees_shins) | 0.13 / -0.08 | 135 (hand_l f2->3) -> 135 (hand_l f2->3) | 30 | 23/74 | 43.5 |
| Check_Hit_Right | 31 -> 31 | 12.09 (knees_shins) | 0.13 / 0.34 | 131 (lowerarm_l f2->3) -> 131 (lowerarm_l f2->3) | 30 | 23/74 | 43.5 |
| Board_Hit | 31 -> 31 | 13.10 (knees_shins) | 0.13 / 0.33 | 116 (upperarm_l f1->2) -> 116 (upperarm_l f1->2) | 16 | 4/38 | 42.9 |
| Stumble | 21 -> 21 | 13.05 (knees_shins) | 0.13 / 0.34 | 90 (upperarm_l f1->2) -> 90 (upperarm_l f1->2) | 6 | 23/56 | 56.4 |
| Stumble_Short | 21 -> 21 | 13.07 (knees_shins) | 0.13 / 0.33 | 53 (hand_r f2->3) -> 53 (hand_r f2->3) | 6 | 23/56 | 56.5 |
| Knockdown_Left_B | 27 -> 27 | 0.69 (forearms) | 0.13 / 0.34 | 105 (calf_l f4->5) -> 105 (calf_l f4->5) | 30 | 23/103 | 14.0 |
| Knockdown_Right_B | 27 -> 27 | 0.44 (arms) | 0.13 / 0.34 | 105 (lowerarm_l f2->3) -> 105 (lowerarm_l f2->3) | 30 | 23/103 | 14.5 |
| Check | 31 -> 31 | 12.43 (hands) | 0.13 / -0.74 | 44 (stick_socket f1->2) -> 54 (lowerarm_r f2->3) | 0 | 23/65 | 49.5 |
| Check_Deliver_Right | 31 -> 31 | 12.43 (hands) | 0.13 / -0.74 | 44 (stick_socket f1->2) -> 54 (lowerarm_r f2->3) | 0 | 23/65 | 49.5 |
| Check_Deliver_Left | 31 -> 31 | 13.05 (knees_shins) | 0.13 / 0.34 | 51 (stick_socket f2->3) -> 51 (clavicle_l f1->2) | 0 | 23/65 | 49.5 |
| Fall | 27 -> 27 | 2.96 (hands) | 0.13 / 0.34 | 89 (thigh_l f3->4) -> 89 (thigh_l f3->4) | 30 | 11/86 | 72.6 |
| Fall_Forward | 27 -> 27 | 7.20 (hands) | 0.13 / 0.34 | 104 (upperarm_l f1->2) -> 104 (upperarm_l f1->2) | 30 | 23/89 | 68.3 |
| Knockdown_Back | 27 -> 27 | 7.65 (hands) | 0.13 / -0.65 | 96 (upperarm_l f1->2) -> 96 (upperarm_l f1->2) | 30 | 23/89 | 67.8 |
| Knockdown_Left | 27 -> 27 | 0.69 (forearms) | 0.13 / 0.34 | 88 (lowerarm_r f2->3) -> 88 (lowerarm_r f2->3) | 30 | 23/103 | 16.2 |
| Knockdown_Right | 27 -> 27 | 0.44 (arms) | 0.13 / 0.34 | 102 (lowerarm_l f2->3) -> 82 (calf_l f3->4) | 30 | 23/103 | 17.8 |
| Trip | 27 -> 27 | 0.57 (arms) | -0.20 / 0.34 | 149 (lowerarm_l f1->2) -> 149 (lowerarm_l f1->2) | 30 | 23/87 | 36.4 |
| GetUp | 45 -> 45 | 1.33 (hands) | 0.13 / 0.34 | 42 (stick_socket f4->5) -> 42 (stick_socket f4->5) | 30 | 4/86 | 49.0 |
| GetUp_Front | 45 -> 45 | 1.95 (hands) | 0.13 / -0.91 | 30 (thigh_l f15->16) -> 30 (thigh_l f15->16) | 30 | 23/89 | 49.2 |
| GetUp_Side_Left | 45 -> 45 | 1.82 (forearms) | 0.13 / -0.78 | 48 (upperarm_r f13->14) -> 32 (thigh_l f16->17) | 30 | 23/87 | 39.3 |
| GetUp_Side_Right | 45 -> 45 | 1.56 (arms) | 0.12 / -0.28 | 38 (lowerarm_r f15->16) -> 44 (calf_l f15->16) | 30 | 23/87 | 39.3 |


**Ground:** nothing goes under the ice in any changed take. The stick's worst is -1.41 cm (p2 `GetUp_Front`, as in export10). The per-frame world measures for export13 and export14 are in `work14/<style>/world_13.json` and `world_14.json`.

### Still open

1. Contact arm swing in `Fall` (115 deg), `Fall_Forward`, `Knockdown_Back`, the standing hits and the B variants.
2. The locomotion top glove at the right hip (item 6).
3. Not verified in the engine.

### Files

- **Scripts** (`skater-lab\skf\`): `run14.sh`, `summary14.py`, `knob14.py` (knob vs shoulders and pelvis), `world14.py` (world turns and hand travel per frame), `compose14.sh`, `part1.sh export14`.
- **Stills** (`devlog\shots\20261113-1058xx-B-lab-skf14-*`, 21):

  | Still | Content |
  |---|---|
  | `faceoff-behind-before-after` | the operator sheet: directly behind, rows p2 export10, p2 export14, p3 export10, p3 export14; `Idle_Faceoff` then `Faceoff_Draw` |
  | `<style>-<take>-ba` (20) | export10 above export14 |

## Fifteenth pass: arm swing, standing-hit stick, a spin that reads (SKF15, 11:42 to 11:51)

Delivered `C:\Users\kazda\jam\skater-lab\skf\export15\` (p2_cel, p3_clay, full sets) with `READY.txt`.
- **Item 1 (the falls):** done, apart from one forearm/stick figure each (see "Against the targets").
- **Item 3 (the spin):** done.
- **Item 2:** improved but not at the target.
- **Items 4 and 5:** not started.

**Difference list against export14:** 7 takes per style differ: `Knockdown_Right`, `Fall`, `Check_Hit_Back`, `Check_Hit_Left`, `Check_Hit_Right`, `Knockdown_Left_B`, `Knockdown_Right_B`. Everything else is identical to export14; the clay aim holds are identical to export10. The take count stays 74 with the same names.

**The measure.** The engine's numbers match the local turn (each bone against its parent), not the world turn I reported in pass 14. My world numbers were therefore lower than what the figure shows. Everything below uses the local measure (`local15.py`).

**Why the right side was worse: it is which arm was measured, not the stick.** The large contact turn is always on the arm of the side the body falls towards:
- the left arm in `Knockdown_Right`;
- the right arm in `Knockdown_Left` (122 deg in export14);
- the measure in the engine was taken on one arm only.

**The cause.** The snap rolls the trunk about 46 deg a frame. The hand targets stay put in the world: offsets from the shoulder and stick positions do not turn with the trunk. So against the trunk the arm swings the other way.

**The fix (items 1 and 2):**
- At the contact frame (2), both hands and the stick are the ready Glide's arms carried round with the trunk.
- From frame 2 to 5 they fling out on an arc: extra keys at 3 and 4 keep each hand at least 85 % of the arm's reach from its shoulder, instead of a straight line that folds and unfolds the elbow.
- It is applied to `Knockdown_Right`, `Fall` and the three standing hits.
- In the two falls only, the arms also reach their flung position at frame 8 instead of 5, so the fling spreads over 6 frames. In the hits that moved the stick turn to frames 6-7, so the hits keep the 3-frame fling.
- On `Knockdown_Left`, `Fall_Forward` and `Knockdown_Back` it made the other arm worse (up to 166 deg), so those stay as export14.

**Local one-frame turns, export14 → export15** (upper arm L / R, forearm L / R, stick, in deg):

| Style | Take | export14 | export15 |
|---|---|---|---|
| p2_cel | Knockdown_Right | 109.1 / 50.1 / 84.2 / 53.8 / 67.3 | 47.1 / 40.9 / 54.2 / 28.1 / 67.3 |
| p2_cel | Knockdown_Right_B | 134.0 / 60.5 / 99.8 / 62.5 / 66.9 | 100.8 / 68.6 / 81.8 / 87.6 / 132.6 |
| p2_cel | Knockdown_Left_B | 49.1 / 124.4 / 54.1 / 78.7 / 76.0 | 94.6 / 50.2 / 135.0 / 67.5 / 121.2 |
| p2_cel | Fall | 123.1 / 58.6 / 143.2 / 75.9 / 124.9 | 52.6 / 35.9 / 61.2 / 81.3 / 78.4 |
| p2_cel | Check_Hit_Left | 46.1 / 112.2 / 135.3 / 121.2 / 131.2 | 31.5 / 61.9 / 54.2 / 44.9 / 80.0 |
| p2_cel | Check_Hit_Right | 178.4 / 48.9 / 158.8 / 103.4 / 113.5 | 65.1 / 44.6 / 61.6 / 70.7 / 72.5 |
| p2_cel | Check_Hit_Back | 151.8 / 112.7 / 141.1 / 130.1 / 123.1 | 63.4 / 51.8 / 58.7 / 72.9 / 94.6 |
| p3_clay | Knockdown_Right | 113.0 / 49.1 / 89.3 / 51.7 / 64.5 | 48.6 / 40.8 / 75.4 / 30.6 / 64.5 |
| p3_clay | Knockdown_Right_B | 140.0 / 58.6 / 106.0 / 62.2 / 65.5 | 104.3 / 69.7 / 76.9 / 86.2 / 130.5 |
| p3_clay | Knockdown_Left_B | 48.6 / 124.7 / 51.8 / 84.8 / 75.3 | 110.2 / 55.5 / 141.8 / 64.2 / 121.7 |
| p3_clay | Fall | 104.7 / 66.2 / 155.0 / 83.6 / 130.4 | 60.7 / 39.0 / 77.6 / 83.6 / 73.8 |
| p3_clay | Check_Hit_Left | 41.5 / 111.7 / 152.3 / 128.0 / 134.9 | 50.0 / 68.7 / 58.1 / 59.6 / 73.4 |
| p3_clay | Check_Hit_Right | 166.3 / 46.2 / 166.4 / 117.2 / 127.2 | 62.4 / 44.4 / 76.4 / 70.1 / 74.7 |
| p3_clay | Check_Hit_Back | 156.5 / 124.2 / 145.9 / 135.6 / 132.3 | 74.3 / 40.8 / 76.9 / 62.6 / 97.0 |

**Against the targets:**
- **Contact upper arm, target about 40-65 deg:** met for `Knockdown_Right` (47 / 49, was 109 / 113) and `Fall` (53 / 61, was 123 / 105). The forearm is 54-78 and the stick 64-78. The standing hits' upper arms are 31-74.
- **Standing-hit stick, target under about 45 deg:** now 72-97 deg per frame (was 113-135). Not met. Most of it is the stick turning in the hand, a wrist twist, not its swing: halving the balance stick's swing changed it by only 1-6 deg. The flung-wide stagger is kept.

### 3. The spin reads as a spin

`Knockdown_Left_B` and `Knockdown_Right_B` are now a helicopter:
- both arms out, one leg flung straight out sideways;
- the stick a horizontal spoke, all turning with the body;
- one full turn over frames 2-14 at 30 deg per frame (shoulders 30 deg per frame, measured);
- landing at 14, bounce at 17, lying from 21 to 30.

**Timing:** 30 frames, 30 + 44 = 74 of 90, so lying is about 16 frames.

**Join:** it ends on the A version's lying pose, so the same get-up joins. The rotation is the same; some bones carry the negated quaternion, as before.

**From the game camera** the spoke points right, down-left, up-left, then up over frames 3-12, so the rotation is visible. See `skf15-spin-vs-plain`.

**Still failing in the spin:** at the landing (frames 13-14) the arms and the stick turn 95-150 deg in one frame against their parents, coming from the spokes into the lying pose.

### Ground

- Body and skates are at 0 cm or above.
- The stick's worst is -1.41 cm (p2 `GetUp_Front`, as in export10) and -1.87 cm (p3 `Check_Hit_Back`, new: the stick during the fling).

### Not done

- **Item 4:** stick along the body in the lying poses.
- **Item 5:** `Fall_B`, `Knockdown_Back_B`.
- Plus the open values above. Nothing is verified in the engine.

### Files

- **Scripts** (`skater-lab\skf\`):
  - `run15.sh`, `summary15.py`
  - `local15.py` (the engine's local measure)
  - `series15.py` (per-frame world turns)
  - `compose15.sh`, `part1.sh export15`
- **Tables:** `work15/<style>/summary15.md`, `local_14.json`, `local_15.json`.
- **Stills** (`devlog\shots\20261113-1150xx to 1152xx-B-lab-skf15-*`, 11):

  | Still | Content |
  |---|---|
  | `spin-vs-plain` | per style, `Knockdown_Left` above `Knockdown_Left_B`, `Knockdown_Right` above `Knockdown_Right_B` |
  | `<style>-<take>-ba` (10) | export10 above export15 for the three standing hits, `Knockdown_Right` and `Fall` |

## Sixteenth pass: the stick in the lying poses, two new falls (SKF16, 11:58 to 12:04)

Delivered `C:\Users\kazda\jam\skater-lab\skf\export16\` (p2_cel, p3_clay, full sets) with `READY.txt`. Items 1 and 2 are done and verified; item 3 (the spin's landing) is not done.

**Difference list against export15:** 12 existing takes per style differ:
- the falls, now with the stick in from the landing on: `Fall`, `Fall_Forward`, `Knockdown_Back`, `Knockdown_Left`, `Knockdown_Right`, `Knockdown_Left_B`, `Knockdown_Right_B`, `Trip`;
- the get-ups, which start on those poses: `GetUp`, `GetUp_Front`, `GetUp_Side_Left`, `GetUp_Side_Right`.

There are also two new takes, `Fall_B` and `Knockdown_Back_B` (76 takes in all). Everything else is identical to export15; the clay aim holds are identical to export10.

### 1. The stick in along the body

In every lying pose the top hand reaches past the head along the ice and the shaft lies along the trunk (on the right side of the body). The shaft slopes down to the ice instead of floating at hand height. The air moment and the spin keep their wide stick.

Measured on the last frame of each fall (`lie16.py`), p2 / p3:

| Lying pose (takes) | Blade end from pelvis, export15 → 16 (cm) | Lowest stick point, export15 → 16 (cm) |
|---|---|---|
| On the back (`Fall`, `Fall_B`) | stick out to the side → 64.7 / 65.6 | 9.6 / 8.8 → 2.0 / 3.9 |
| Face down (`Fall_Forward`, `Knockdown_Back`, `_B`) | → 77.3 / 76.6 | 16.0 / 14.4 → 1.6 / 0.1 |
| Right side (`Knockdown_Left`, `_Left_B`) | → 66.3 / 65.2 | 9.6 / 8.8 → 1.3 / 1.3 |
| Left side (`Knockdown_Right`, `_Right_B`, `Trip`) | → 89.8 / 85.0 | 29.4 / 30.7 → 0.8 / 1.0 |

- **Not at 60 cm:** face down and on the left side the blade stays 77-90 cm from the pelvis. The toy's top arm is at full reach (arm extension 0.987), and on the left side it is the upper arm.
- **Joins:** every fall's last frame equals its get-up's first, at most 0.0001 cm, the new takes included.
- **Ground:** body and skates are at 0 cm or above. The stick's worst is -1.41 cm (p2 `GetUp_Front`, as in export10), -1.47 cm (p3 `Knockdown_Back_B`) and -1.87 cm (p3 `Check_Hit_Back`, from export15).

### 2. Two new takes

| Take | Frames | Beats | Ends on | Joins |
|---|---|---|---|---|
| `Fall_B` (hit from the front) | 28 | Snap at 2; onto the backside 5-7; slides seated with the legs out in front and the arms and stick thrown up 7-14 (pelvis carried back 24 cm); drops back flat at 18; lying 18-28 | `Fall`'s pose | `GetUp` |
| `Knockdown_Back_B` (hit from behind) | 28 | Snap at 2; forward dive at 5 with the arms reaching forward; belly slide 9-15 with the arms out in front and the legs kicked up behind (pelvis carried 28 cm forward); flat at 20; lying 20-28 | `Knockdown_Back`'s pose | `GetUp_Front` |

`Fall_B` + `GetUp` = 72 of 90 frames, and the same for `Knockdown_Back_B` + `GetUp_Front`.

### Not done

- **Item 3:** the spin's landing (arms and stick 95-150 deg in one frame at frames 13-14).
- **Arm swing in the new takes:** the contact-frame swing in `Fall_B` and `Knockdown_Back_B` is not tamed (forearm up to 124 deg, upper arm up to 96 deg against the parent).

### Files

- **Scripts** (`skater-lab\skf\`):
  - `run16.sh`, `summary16.py`
  - `lie16.py` (blade end, stick height, joins)
  - `compose16.sh`, `part1.sh export16`
- **Tables:** `work16/<style>/summary16.md`, `lie16.txt`.
- **Stills** (`devlog\shots\20261113-1202xx-B-lab-skf16-*`, 10):

  | Still | Content |
  |---|---|
  | `lying-poses-before-after` | the four lying poses, export14 above export16, both styles |
  | `new-falls` | `Fall` above `Fall_B`, `Knockdown_Back` above `Knockdown_Back_B`, both styles |
  | `<style>-<take>-ba` (8) | export14 above export16 for `Fall`, `Fall_Forward`, `Knockdown_Left`, `Knockdown_Right` |

### Addendum: item 3 as an optional set, export16s (12:05)

`C:\Users\kazda\jam\skater-lab\skf\export16s\` (with `READY.txt`) differs from export16 only in `Knockdown_Left_B` and `Knockdown_Right_B`. export16 stays the main set.

**What changed:**
- The spin lands at frame 15 instead of 14 (bounce at 18, settle at 22, lying 22-30).
- Arc keys at 12-14 bring the arms from the spokes into the lying pose over 4 frames.
- The lying pose and the join are unchanged (0.0001 cm), and nothing goes under the ice.

**Local one-frame turns at the landing (p2 / p3, deg), export16 → export16s:**

| Take | Bone | export16 | export16s |
|---|---|---|---|
| `Knockdown_Right_B` | right upper arm | 144 / 155 | 104 / 113 |
| `Knockdown_Right_B` | stick | 136 / 137 | 107 / 109 |
| `Knockdown_Left_B` | left upper arm | 95 / 110 | 77 / 88 |
| `Knockdown_Left_B` | left forearm | 135 / 142 | 81 / 72 |
| `Knockdown_Left_B` | stick | 154 / 153 | 164 / 163 (worse) |

The result is mixed: the arms are better, but the stick in `Knockdown_Left_B` is worse. Take it only if the arm pop at the landing shows more than the stick.

**Why the stick got worse:** the measure is the stick bone against the hand. In the new lying pose the stick lies in along the body, so it has about 90 deg further to turn at the landing than before.

**Still:** `devlog\shots\20261113-120507-B-lab-skf16-spin-landing.png`: export16 above export16s for both spin takes, both styles.

## Seventeenth pass: exact joins, the frozen hit frame, the new takes' arms, the stick out of the ice (SKF17, 12:35 to 12:42)

Delivered `C:\Users\kazda\jam\skater-lab\skf\export17\` (p2_cel, p3_clay, full sets) with `READY.txt` and `JOINS.md`. It is built on export16; the optional export16s is not in it. All four items are done; what remains open is listed below. The take count stays 76 with the same names.

**Difference list against export16:** 23 takes per style differ:

| Item | Takes |
|---|---|
| 1, the joins | `Fall`, `Knockdown_Right`, `Knockdown_Right_B`, `Knockdown_Left_B`, `Trip` |
| 2, frame 2 | the ten falls except `Trip`, and the three `Check_Hit_*` |
| 3, the arm swing | `Fall_B`, `Knockdown_Back_B` |
| 4, the stick out of the ice | also `Check`, `Check_Deliver_*`, `Board_Hit`, `Stumble`, `Stumble_Short` and the four get-ups |

Everything else is identical to export16, and the clay aim holds are identical to export10.

### 1. The joins, with rotations

**Cause:** in `Fall` the left thigh's roll had not settled by the end of the lying hold. The solver lets a limb's roll catch up only 12 deg a frame, so the fall's last frame kept a roll the fresh solve at the get-up's start does not have.

**Fix:** in the tackling takes, while a pose is held still, the roll may settle at up to 45 deg a frame. The fall's last frame is then the get-up's first exactly.

The join check (`join17.py`) now compares bone rotations as well as joint positions, for every pair:

| Fall | Get-up | p2_cel position cm (16 -> 17) | p2_cel rotation deg (16 -> 17) | p3_clay position cm (16 -> 17) | p3_clay rotation deg (16 -> 17) |
|---|---|---|---|---|---|
| Fall | GetUp | 0.0 -> 0.0 | 6.313 (thigh_l) -> 0.0 | 0.0 -> 0.0 | 27.55 (thigh_l) -> 0.0 |
| Fall_B | GetUp | 0.0001 -> 0.0 | 0.0 -> 0.0 | 0.0 -> 0.0 | 0.0 -> 0.0 |
| Fall_Forward | GetUp_Front | 0.0 -> 0.0 | 0.0 -> 0.0 | 0.0 -> 0.0 | 0.0 -> 0.0 |
| Knockdown_Back | GetUp_Front | 0.0 -> 0.0 | 0.0 -> 0.0 | 0.0 -> 0.0 | 0.0 -> 0.0 |
| Knockdown_Back_B | GetUp_Front | 0.0 -> 0.0 | 0.0 -> 0.0 | 0.0 -> 0.0 | 0.0 -> 0.0 |
| Knockdown_Left | GetUp_Side_Left | 0.0 -> 0.0 | 0.0 -> 0.0 | 0.0 -> 0.0 | 0.0 -> 0.0 |
| Knockdown_Left_B | GetUp_Side_Left | 0.0 -> 0.0 | 0.0 -> 0.0 | 0.0001 -> 0.0001 | 0.0 -> 0.0 |
| Knockdown_Right | GetUp_Side_Right | 0.0 -> 0.0 | 0.0 -> 0.0 | 0.0 -> 0.0 | 0.0 -> 0.0 |
| Knockdown_Right_B | GetUp_Side_Right | 0.0001 -> 0.0001 | 0.0 -> 0.0 | 0.0001 -> 0.0001 | 0.0 -> 0.0 |
| Trip | GetUp_Side_Right | 0.0 -> 0.0 | 0.0 -> 0.0 | 0.0 -> 0.0 | 0.0 -> 0.0 |

### 2. The frozen hit frame (frame 2)

The engine holds frame 2 for 3 frames, so it must read on its own.

**What every frame 2 has:**
- the full snap: trunk and head thrown along the hit;
- the arms carried round with the trunk, lagging the fling that follows;
- new in this pass: the first-going skate already 10-14 cm off the ice.

**Per take, frame 2 shows:**

| Take | Frame 2 |
|---|---|
| `Fall`, `Fall_B` | Trunk thrown back, head forward, the left skate up and forward |
| `Fall_Forward`, `Knockdown_Back`, `Knockdown_Back_B` | Pitched forward over the skates, head back, the right skate up behind |
| `Knockdown_Left` / `_B` | Upper body thrown right, head lagging left, the left skate lifting out to the left |
| `Knockdown_Right` / `_B` | The mirror of `Knockdown_Left`, the right skate lifting |
| `Trip` | The right leg swept out sideways and up (as before) |
| `Check_Hit_Left` / `_Right` / `_Back` | Thrown away from the push, the stick and far arm going out, the lead skate lifting 10 cm |
| `Check_Deliver_*` | Frames 2-4 are the lunge from export14: shoulder through and low, back leg pushing |

**Sheet:** `skf17-hit-frames` has one cell per take, attacker beside victim at 0.8 m.
- Falls: the attacker is at `Check_Deliver` frame 2.
- Standing hits: the pusher is in `Glide`.
- `Fall` and `Fall_B`: the victim faces the attacker (hit from the front).
- The same sheet for export16 is filed beside it for comparison.

**Weakness:** from behind, the falls hit from behind (`Knockdown_Back`, `Fall_Forward`) have the victim partly hidden by the attacker, as in the game.

### 3. The new takes' arm swing

`Fall_B` and `Knockdown_Back_B` use export15's method: the arms ride with the trunk at frame 2, then fling on an arc to the flung position at the key after 5.

| Style | Take | export16 (upper arm L / R, forearm L / R, stick, deg) | export17 |
|---|---|---|---|
| p2_cel | Fall_B | 123.1 / 73.6 / 143.2 / 109.0 / 124.9 | 77.6 / 73.6 / 69.7 / 109.0 / 87.4 |
| p2_cel | Knockdown_Back_B | 100.2 / 92.7 / 85.4 / 61.5 / 84.2 | 98.4 / 51.4 / 82.7 / 51.5 / 94.3 |
| p3_clay | Fall_B | 104.7 / 78.4 / 155.0 / 114.7 / 130.4 | 85.4 / 78.4 / 69.4 / 114.7 / 83.5 |
| p3_clay | Knockdown_Back_B | 109.4 / 92.6 / 91.2 / 62.5 / 83.7 | 102.9 / 60.2 / 95.3 / 47.1 / 79.0 |

**Result:** `Fall_B` is much better. `Knockdown_Back_B`'s left upper arm stays at 98-103 deg against its parent: it peaks in frames 0-1, before the ride key, and is not fixed.

### 4. The stick out of the ice

In every tackling take the stick's ice clamp now keeps 2 cm above the ice at its bone points, because the blade mesh is thicker than those points.

- **Stick:** the lowest stick point in the whole set is now at least 0.08 cm above the ice (was -1.41 cm p2 `GetUp_Front`, -1.91 cm p3 `Knockdown_Back_B`, -1.87 cm p3 `Check_Hit_Back`).
- **Body:** bodies stay at 0 cm or above.
- **One skate:** -0.2 cm (p3 `Trip`).
- **Lying poses:** the stick still lies along the body, 1.3-3.9 cm above the ice at its lowest; the blade end is 65-90 cm from the pelvis as in export16.

### Files

- **Scripts** (`skater-lab\skf\`):
  - `run17.sh`, `summary17.py`
  - `join17.py` (joins with bone rotations)
  - `hit17.sh` (frame-2 sheet; `pair_render12.py` takes `PAIR_K0=1`)
  - `local15.py`
- **Tables:** `export17\JOINS.md`, `work17/<style>/summary17.md`, `work17/local_table.md`.
- **Stills** (`devlog\shots\`):
  - `20261113-124150-B-lab-skf17-hit-frames.png` (export17)
  - `20261113-124150-B-lab-skf17-hit-frames-export16.png` (before)
