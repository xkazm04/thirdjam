# Blender pipeline for the skater figures, the stick and the props (lane A22, H9.9-H11.9)

Status: experiment notes, 2026-11-12 15:57-16:50 CET (first pass to 16:33, second pass after the Director's review 16:33-16:42, third pass after lane A26 put the figure on the ice 17:40-17:50). Scripts in `tools/blender/` (README there is the job list), FBX sources in `tools/blender/out/`,
the toy variant imported to `Game/Content/Figures/`. The swap of the code-built figure for the skeletal one is the Director's call after the review
(brief A22); nothing here touches `ToySkaterVisual.cpp` or the cartridges.

## 0. The answer in four lines

1. One parametric bpy script makes three figures (toy, clay, broadcast) with the same seven-bone armature and the same 17 actions in 10 s each, headless.
2. The toy figure imports into Unreal headless (`UnrealEditor-Cmd -run=ImportAssets`, legacy FbxFactory) with the three socket bones and 17 sequences.
3. The props are seven low-poly meshes on one 1K atlas (one material slot each); the net uses the atlas's alpha for the netting.
4. The arena's film props stay out (R-0021); these are the hockey props the rink needs instead.

## 1. Contact sheet and turntables

![3 x 3: rows toy, clay, broadcast; columns idle f0, carry f15, poke f6](img/blender-sheet-1536.png)

Full size (cells 1024 px): `docs/experiments/img/blender-sheet.png` (3.1 MB). Turntables (2 s, 30 fps, 512 px, H.264): `img/blender-turntable-toy.mp4` (70 KB),
`img/blender-turntable-clay.mp4` (76 KB), `img/blender-turntable-broadcast.mp4` (65 KB). Props overview: `img/blender-props.png`.

What the sheet shows (second pass, after the Director's review of the first): the toy figure at 1.6 m with toy proportions (a 49 cm head on a torso
tapered from 46 cm at the hips to 64 cm at the shoulders, a dark helmet dome and 13 cm band with a visor box, a skate plate with two bright blade runners,
a white number patch standing off the back), two arms with forward elbows and two mitten hands on the stick, the upper hand at the top of the shaft and the
lower a third down, in every pose (the arms are STRETCH_TO bones so they follow the stick). The stick: a shaft tapered from 32 to 24 mm at a 42 degree lie,
a heel ball, a 32 cm wooden blade lying flat and turned 30 degrees out with a dark tape band on its outer half, a knob at the top; the carry cell shows a
puck at `puck_socket` (a render prop, not exported). Clay: the same figure with 3.5 cm thumb dents on the torso (simple subdivision level 2, torso only,
22.7k tris) and a felt stitch seam around the waist and over the head, lifted by a warm rim light. Broadcast: a 1.85 m humanoid with a neck, upper and
lower arms with elbows, gloves with cuffs, thighs, knees and shins, a helmet with a cage line and no face, a 1.55 m stick held below the shoulder.
First-pass sheet (kept for the record): bottle-shaped toy, one visible arm, no visible blade; superseded.

## 2. Figures: what the script makes

| Variant | Triangles | Vertices | FBX | Bones | Actions | Build + export | Renders (3 cells + 60 turntable frames) |
|---|---|---|---|---|---|---|---|
| toy | 3,866 | ~2,000 | 2.22 MB | 14 (9 deform + 3 sockets + 2 IK helpers) | 19 | 1.5 s | 9 s |
| clay | 22,750 | ~11,500 | 2.73 MB | 14 | 19 | 1.8 s | 9 s |
| broadcast | 4,330 | ~2,200 | 2.23 MB | 14 | 19 | 1.5 s | 9 s |

Armature (third pass): `root` > `pelvis` > `torso` > {`head` > `head_socket`, `arm_l_upper` > `arm_l_fore`, `arm_r_upper` > `arm_r_fore`,
`stick_root` > {`stick_socket`, `puck_socket`, `ik_hand_l`, `ik_hand_r`}}. The mittens are skinned to `stick_root`, so they ride the shaft in every verb by
construction. The arms are two bones each, posed by a two-bone IK to the helper bones `ik_hand_*` on the stick and **baked to plain location/rotation
keys every frame** (`bake_arms`: evaluate, `convert_space` POSE to LOCAL, key, then drop the constraints), so the FBX carries nothing an engine has to
re-solve; the first pass's STRETCH_TO scale did not survive the import (lane A26). Proof in the build stats: the worst gap between a forearm tip and its
mitten is the same before and after the bake (`ik_worst_gap_cm` = `baked_worst_gap_cm`), and `tools/blender/check_fbx.py` re-imports the FBX into a
clean Blender and measures the same gap per action from the plain keys (PASS, 0.7 cm over 19 actions). What the arms cannot do: reach around the body at
the extremes of the long verbs; the forearm tip falls short of the mitten by up to 16 cm in `poke` (frame 6), 15 in `draw`, 12 in `shoot_release`, 11 in
`check` (toy; broadcast only `poke` 12). The shot swing is already scaled from the code's -100/10/75 degrees to -65/6/50 and the poke slide from 55 to 30 cm
to get there; the rest is the price of a 1.3 m stick on a 1.6 m toy. Next step if it shows on the ice: a short IK stretch allowance (`ik_stretch` 0.2) or
a 10 cm shorter shaft for the toy. The sockets of C-0001 are leaf bones: Unreal answers `DoesSocketExist("puck_socket")` on a skeletal mesh component from bone names, so
`AJamSkater::GetPuckSocketWorld` finds them without a socket asset. `puck_socket` is 12 cm in front of the blade's face on the ice and rides the stick.

Material slots (third pass, plain names in the mesh's first-use order): `base` (jersey, arms, shoulders: `TeamColour`), `number` (a **flat UV 0..1
square** standing 2 cm off the back, u left to right as seen from behind, for the game's digits material), `head`, `trim` (helmet, band, visor, mittens and
cuffs, skates, plate, legs, tape, knob), `steel` (the 1 cm light-grey blade strips under the black skate blocks, the broadcast cage line), `stick` (shaft,
heel, blade wood). Nothing but the number square is in `number`, so a digits material never lands on a runner. Materials and
textures are not imported; B's `M_Toon` instances go on the slots in the Blueprint (R-0002 item 5).

Actions (60 fps, frame counts; timings from `ToySkaterVisual.cpp`): idle 60 (loop), skate 36 (loop, one stride cycle: at 8 m/s the code's phase makes one
cycle every 18 frames, so play rate = speed / 4), skate_left and skate_right 36 (loop, the same stride with an authored 15 degree lean, left side down for
skate_left, plus a 4.5 degree sway and the stick turned 6 degrees into the turn; the game blends by turn rate and caps the roll it adds on top: the 35 degree
code lean read as a fall on the tapered body), carry 60 (loop), aim_left 30, aim_right 30 (hold, 70 degrees and a 30 cm reach), poke 18 (thrust at frame 6),
lift 20, shoot_windup 24, shoot_release 51 (the code's key yaw 0/-100/10/75/75/0 at 0/18/25/34/40/51), check 20 (8 in, 12 out, 30 degree pitch, 15 cm shift),
knocked_down 40 (88 degrees backward in 14 frames, a 3 + 3 bounce, hold), get_up 20, faceoff_ready 30, draw 15, goalie_left / centre / right 30 (loop).

Two lessons that cost the most time: (1) the stick is a child of the torso, so the torso's 10 degree forward pitch at idle dropped the blade 17 cm under the
ice (the first sheet showed a stick without a blade); the script now counter-pitches the stick about the grip so a pose's `stick_pitch` is relative to the ice,
pre-rotates the forward slide so a poke stays horizontal, and cancels the grip's own displacement under the torso pitch (with the grip 33 cm ahead of the hip
the lean alone sank the blade 7 cm): the blade stays put and the hands slide on the body instead. (2) A Blender text label parented to the camera never rendered in headless EEVEE; the labels are
drawn by ffmpeg `drawtext` on the tiled sheet instead.

## 3. Props

| Mesh | Triangles | Size (m, X x Y x Z) | FBX | Note |
|---|---|---|---|---|
| `SM_Net` | 316 | 1.6 x 4.1 x 1.58 | 25 KB | tubular frame (11 tubes, 8 sides), four netting cards on the atlas's alpha pattern (5 cm cells); the mouth faces +X, the goal line at x = 0 |
| `SM_Board_Segment` | 72 | 0.12 x 2.0 x 2.26 | 20 KB | kick plate 0.25 m, board to 1.2 m, cap rail, 1.0 m glass (alpha 0.3 band), two stanchions |
| `SM_Bench` | 72 | 0.43 x 2.4 x 0.9 | 20 KB | seat, back board, two legs |
| `SM_Puck` | 380 | 0.24 x 0.24 x 0.08 | 25 KB | the game's puck (r 0.12, h 0.08), chamfer 12 mm in two segments |
| `SM_GoalLight` | 356 | 0.38 x 0.25 x 0.33 | 25 KB | body, red lens dome, rim, wall bracket |
| `SM_Pedestal_Round` | 276 | 0.99 x 0.99 x 0.14 | 23 KB | socket `PickupTop` at 45 cm (`SOCKET_PickupTop` empty), blue inlay |
| `SM_Pedestal_Hex` | 60 | 0.86 x 0.99 x 0.14 | 21 KB | same socket, orange inlay |

One texture for all seven: `T_Props_Atlas.png`, 1024 x 1024 RGBA, 23 KB: a tiling 32 x 32 cell net pattern (opaque thread on alpha 0) in the top half,
twelve flat colour bands of 32 px in the bottom half (frame red, board white, kick yellow, glass at alpha 0.3, dark, black, wood, metal, lens red, team blue,
team orange, ice white). Faces map to the centre of their band; the netting cards tile u across the pattern. The material must be masked (alpha) for the net
and glass. Not imported (B owns `Props/` and `Arena/`; the FBX and PNG are in `tools/blender/out/` for a B import or the Director's call).

## 4. Import result (toy)

Headless, `UnrealEditor-Cmd ThirdJam.uproject -run=ImportAssets -importsettings=tools/blender/import_toy.json -NullRHI`, legacy `FbxFactory` with
`bImportMaterials` and `bImportTextures` false, 60 fps sample rate: `/Game/Figures/SK_SkaterToy` (11 bones digested, 5 sections, 3,808 triangles after the second pass),
its skeleton and 17 animation sequences, written in about 10 s of commandlet time after a 25 s editor start. The one-group import names the sequences
`SK_SkaterToy_Anim_AS_SkaterToy_<action>` and the skeleton `SK_SkaterToy_Skeleton`; a second headless run (`-run=pythonscript -script=tools/blender/verify_toy.py`)
renamed them to `AS_SkaterToy_<action>` and `SKEL_SkaterToy` (C-0004 open question 1; 19 files, no redirector left, the mesh and every sequence reference
`/Game/Figures/SKEL_SkaterToy`, 1.2 MB in all after the second pass, the mesh 0.6 MB) and printed: 11 bones (`AS_SkaterToy` armature root, root, pelvis, torso, head, head_socket, arm_l, arm_r,
stick_root, stick_socket, puck_socket); `DoesSocketExist` true for `stick_socket`, `puck_socket`, `head_socket`; bounds origin (62, 15, 80) cm, extent (94, 53, 80)
cm, so the mesh spans x -32..156 (the blade ahead: forward is +X), y -37..68 (the stick on +Y, the right side, as the code's `StickPivot`) and z 0..160 (the 1.6 m toy); five material
slots in the order base, number, head, trim, stick; 17 sequences with the exact frame counts of the actions (idle 60, skate 36, carry 60, aim 30 + 30, poke 18,
lift 20, shoot_windup 24, shoot_release 51, check 20, knocked_down 40, get_up 20, faceoff_ready 30, draw 15, goalie 3 x 30) at 60 fps. What did not work: an animation-only second import group
(`bImportMesh` false, `Skeleton` set) is routed to Interchange by 5.9 and re-imports the mesh instead of the takes; so one group, then rename.

Process rule kept: the import ran only in the windows when no other `UnrealEditor-Cmd` was alive (two other lanes' commandlets were running at 16:14 and 16:19;
A23's MCP editor runs on its own worktree and was never touched).

## 5. How a visual would drive it (`BP_SkaterVisual_TA2`, or `UToySkaterVisualComponent` with a skeletal mesh child)

The same `IJamSkaterVisual` calls, mapped to the sequences; a code-driven approach (no Animation Blueprint state machine, since MCP cannot add states, HOCKEY.md 5):

| Call | What the visual does with `SK_SkaterToy` |
|---|---|
| `SetTeam` | `TeamColour` on the dynamic instance of slot `M_SkaterBase` (and the lighter head on `M_SkaterHead`) |
| `SetNumber` | `NumberTens`, `NumberOnes` on slot `M_SkaterNumber` (the patch is UV 0..1, a digits material reads them) |
| `UpdateMotion` | base layer: `AS_SkaterToy_idle` below 0.3 m/s, else a blend of `AS_SkaterToy_skate_left` / `skate` / `skate_right` by turn rate (full left at `LeanTarget` <= -15, full right at >= +15) at play rate speed / 4 (one 36-frame cycle per 2.4 m, as the code's phase); the component adds at most the remaining roll, capped at 15 degrees, so the figure never leans past 30 in total |
| `SetHasPuck` | base layer `AS_SkaterToy_carry` while true (stickhandling loop) |
| `SetStickAim(angle, reach)` | blend between `aim_left` (angle -70) and `aim_right` (+70) on the `stick_root` bone only (per-bone blend, or drive `stick_root` directly by yaw = -angle and a forward offset of (reach - 1.0) m, as the code does now) |
| `SetShotCharge` | `shoot_windup` scrubbed to frame 24 x charge (hold at the wind-up) |
| `PlayShoot(charge)` | `shoot_release` once (51 frames); charge scales the blend weight of the wind-up |
| `PlayPoke(clean)` | `poke` once; a mistimed poke adds `get_up`'s first 8 frames as a stumble |
| `PlayLift` | `lift` once |
| `PlayPass` | `shoot_release` from frame 25 (the follow-through only) at rate 1.5 |
| `PlayCheck` | `check` once (20 frames) |
| `SetKnockedDown(down, dir)` | `knocked_down` once and hold the last frame while down; `get_up` on false; the fall direction is a yaw of the whole component toward (dir_x, dir_z), as the code's lean node does |
| `SetFaceoff(ready)` | `faceoff_ready` while true |
| `PlayFaceoffDraw(won)` | `draw` once |
| `SetGoalieStance(on, side)` | `goalie_left` / `goalie_centre` / `goalie_right` while on |
| `SetBoost`, `PlayFoulReaction` | material `Emissive` pulse as now; the foul reaction reuses `check` at rate 0.6 |
| sockets | `puck_socket`, `stick_socket`, `head_socket` are bones: `GetSocketTransform` works on the skeletal mesh component unchanged |

Frames, never seconds: every one-shot is played by setting the sequence position from the sim's frame counter (`LocalFrame - Start` / 60), as the code
does with its curves, so the same call sequence gives the same pose at 60 and 144 Hz.

## 6. Risks

1. **Nobody has looked at it in the editor yet.** The bounds prove forward +X and the stick on +Y, the sockets exist and the sequences have the right frame
   counts, but no one has seen `AS_SkaterToy_skate` play on `SK_SkaterToy` in a viewport: a bone-roll or bake mistake (the stretch arms, the heel bone) would
   show only there. First thing for the review: open the mesh, play the 17 sequences, check the blade stays on the ice through `poke` and `faceoff_ready`.
2. **Arm stretch as bone scale.** The STRETCH_TO result is baked into `arm_l` / `arm_r` scale keys; Unreal plays non-uniform bone scale fine, but any
   retarget or additive blend between actions will scale the arms oddly. Per-bone blending of `stick_root` alone (the aim case) is safe; blending whole
   actions with different hand positions is not.
3. **Determinism and frame budget are unproven for a skeletal visual.** The code-built figure costs nothing to animate and is deterministic by construction;
   six skeletal meshes with sequences are cheap on the 4090, but the `Jam.Core.Determinism` baseline and the frame gate must be re-run with the swap, and the
   visual must drive sequence positions from the sim frame, never from delta time.
