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
