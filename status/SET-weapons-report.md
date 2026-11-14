# SET-WEAPONS: toy weapons for the modes (device B, night of 14 Nov, 01:40 to 03:00, with the critic's corrections folded in)

## 1. What is done, what is not

Both evolutions are done in Blender on the CPU. The critic's three points are folded in; I did not restart (my line on each is in section 1a). Everything is rebuilt, re-checked and re-rendered.

The delivery is three props (`Prop_GiantStick`, `Prop_Mallet`, `Prop_PuckCannon`), one FBX per style, and ten new takes per style, one FBX per take: `Giant_Carry`, `Giant_Poke`, `Giant_Sweep`, `Mallet_Carry`, `Mallet_Swing`, `Mallet_Miss`, `Fall_Bonk`, `Cannon_Carry`, `Cannon_Fire`, `Cannon_Kick`. All of them pass these checks:
- Joins: exact, 0 cm / 0 deg.
- Ice: nothing under it (lowest skate point -0.07 cm on cel, about 0.7 mm).
- Pops: largest bone turn in one frame 55.8 deg (lab limit 60).
- Root: never moves.
- Top hand: on the prop in every weapon frame.

What is not done, and where I depart from the brief:
- **Weapon takes start and end on the weapon's own ready stance, not on Glide.** Each weapon has a carry hold (`*_Carry`). Below `spine_01` it is Glide's frame 0, key for key. Above `spine_01` it holds the weapon. Layered over Glide from `spine_01` up, it is exactly the pose the weapon takes start and end on. Held the way Glide holds the stick, the giant stick's blade would be 0.5 m under the ice.
- **The straight `Giant_Poke` barely reads from the game camera.** From behind the stick is seen end-on. The sweep reads well.
- **The bottom hand comes off the cannon during `Cannon_Kick`.** Up to 28 cm off in frames 12–14 and 19–25. From frame 24 to 30 it stays 19 cm off, because the side lying pose's own hand is not on the barrel. On clay it is up to 3.1 cm off in `Cannon_Fire`.
- **The cannon has four material slots** (`ClayTimber`, `ClayCream`, `ClayRed`, `ClayPuck`), not one or two. The critic asked for cream, timber and a red band, and the puck sits in the mouth. The engine lane may merge the slots.
- **Evolution 3 (a prop of my own) is not started.**

### 1a. The critic's three points (one line each)

1. **Cannon: rebuilt as a toy, and a stranger shown the silhouette alone would say "toy" (someone carrying a keg), not "weapon".** It is now a short fat wooden barrel: timber staves, two cream hoops, a red band, a cream lip, a puck sitting in the mouth, and a big red crank wheel with a cream knob. It is carried at the right hip in both arms, never on the shoulder, with no grey and no black except the puck.
   - How I judged it: I looked at three silhouette-only cells (game camera, side, front left) in the cannon sheet. They show a fat round block held in front of the hip.
   - At the game camera the barrel shows beside the body. Cradled in front of the belly, as tried first, the body hid it completely.
2. **Mallet: no blow to any head.** The swing goes up at the right side, comes down and round low and flat, and **the drum's end face meets the victim's hip** at contact frame 17. I show the contact frame from the front left and from the game camera. The face is at 89 / 86 cm high (cel / clay), 118 / 113 cm from the attacker's hips.
   - `Fall_Bonk` is now a victim hit at the hip from its side: hips knocked over, legs swept out, sat down hard (frame 8), onto its back (frame 14). Nothing comes down on it from above.
3. **Spread-eagle endings: `Cannon_Kick` now ends on its side, not on its back.** The figure is sat down by the recoil, rolls onto its right side and ends on `Knockdown_Right`'s lying pose, so `Lie_Loop_Side_Right` and the `GetUp_*Side_Right` get-ups follow. `Fall_Bonk` still ends on the back lying pose: a second side ending was not needed for the critic's point, and the back is where a sat-down figure tips.

## 2. Per evolution

### Evolution 1: the giant stick and the mallet

| Prop | Tris | Size cm p2_cel / p3_clay (stick frame x, y, z) | Materials, sRGB base colours |
|---|---|---|---|
| Prop_GiantStick | 312 | 56.6 x 11.7 x 303 / 287 | `ClayTimber` (0.62, 0.42, 0.22), `ClayTape` (0.90, 0.86, 0.74) |
| Prop_Mallet | 404 | 48 x 36 x 111 / 107.9 | `ClayTimber`, `ClayFelt` (0.78, 0.20, 0.14) |

| Take (cel / clay) | Frames | Contact | Reach from the hips at contact | Lowest prop point cm | Palm r / palm l to the prop cm (max) | Largest bone turn in one frame, deg |
|---|---|---|---|---|---|---|
| Giant_Carry | 31 | - | blade centre 210 / 205 | 0.17 / 3.22 | 0.0 / 0.12 ; 0.0 / 0.08 | 0 |
| Giant_Poke | 17 | frame 8 | 265 / 256, blade 3.7 / 3.2 cm over the ice | 0.17 / 1.02 | 0.0 / 1.49 ; 0.0 / 0.48 | 52.2 / 54.0 |
| Giant_Sweep | 23 | frames 10-14 (blade within 35 deg of ahead) | 197-221 in the window; arc -55 to +55 deg | 0.17 / 1.97 | 0.0 / 1.62 ; 0.0 / 0.19 | 51.7 / 47.7 |
| Mallet_Carry | 31 | - | - | 89.2 / 85.3 | 0.0 / 0.03 ; 0.0 / 0.02 | 0 |
| Mallet_Swing | 33 | frame 17, the victim's hip | drum 118 / 113; end face 89 / 86 cm high | 70.8 / 67.4 | 0.0 / 0.14 ; 0.0 / 0.18 | 55.8 / 50.2 |
| Mallet_Miss | 45 | (17 passes the same point) | as Swing | 21.4 / 19.6 | 0.0 / 0.17 ; 0.0 / 0.17 | 55.8 / 50.2 |
| Fall_Bonk | 19 | hit at 0-2 | - | (own stick) 0.73 / 0.34 | - | 54.8 / 53.8 |

- **Wind-up:** 10 frames (0–10). The mallet goes up and out to the right of the helmet, to 214 / 206 cm.
- **Downswing:** frames 10–17.
- **What fixed the pops:** in the downswing the right elbow flipped. An elbow aim that points back stopped it; without it there were 68–80 deg forearm steps.
- **Lowest points:** body at least 13 cm in the standing takes; in `Fall_Bonk` at least 0.6 / 0.66 cm, with the seat 2.5 / 1.8 cm off the ice at frame 8.
- **Stills:**
  - `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-025730-B-set-weapons-props.png`
  - `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-022604-B-set-weapons-giant.png` (unchanged by the corrections)
  - `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-025730-B-set-weapons-mallet.png`
- **What I saw:**
  - The mallet reads as a carnival mallet. The wind-up is clearly visible beside the head at the game camera.
  - The low swing reads in the closer pair row. The drum hits the victim's side at belly and hip height, the victim sits, then lies down.
  - The giant stick reads clearly in the sweep and from the side, and weakly in the straight poke.

### Evolution 2: the puck cannon (rebuilt)

`Prop_PuckCannon`: 412 tris, 41.5 x 50.3 x 62.8 cm. Materials: `ClayTimber` (0.62, 0.42, 0.22), `ClayCream` (0.92, 0.86, 0.72), `ClayRed` (0.78, 0.20, 0.14), `ClayPuck` (0.16, 0.14, 0.13). The barrel is 38 cm across and 56 cm long; its mouth is 24 cm to the right of the top hand.

| Take | Frames | The puck leaves at | Muzzle at that frame, figure space cm (X forward, Y left, Z up, origin = root on the ice) | Muzzle direction |
|---|---|---|---|---|
| Cannon_Fire p2_cel | 21 | frame 8 | (80.0, -35.9, 91.9); pelvis (0, 0, 71.5) | (1, 0, 0): level, ahead |
| Cannon_Fire p3_clay | 21 | frame 8 | (78.6, -35.4, 88.8) | (1, 0, 0) |
| Cannon_Kick p2 / p3 | 31 | frame 8 | (79.9, -35.9, 92.1) / (78.5, -35.4, 89.0) | (1, 0, 0) |

- **Recoil:** the muzzle tips up 21 deg at frame 9 and 28 deg at frame 10, swinging back 28 cm in one frame (frames 8 to 9). The upper body is thrown back 12 cm over still skates. The cannon is level again by 13 and back on the carry at 17.
- **Kick:** the same frames 0–8, then the figure is thrown back with the front skate up, sits down (15–17), rolls (20) and lies on its right side from 23.
- **The cannon stays above the ice in every frame:** at least 10.3 / 13.3 cm in the kick.
- **Largest bone turn in one frame:** 54.4 deg. Joins 0 / 0.
- **Still:** `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-025730-B-set-weapons-cannon.png`
- **What I saw:**
  - A keg with hoops, a red band and a crank, held at the right hip. It shows beside the body at the game camera.
  - The recoil reads small but visible at game size.
  - The kick reads clearly: thrown back, the barrel up, sat down, rolled onto the side holding the barrel.

## 3. Not verified

- **Nothing has been seen in the engine:** not an import, not the prop on the socket, not the carry over the stride.
- **The socket offset is computed, not tested.** It is the SM_Stick frame, and SM_Stick has never been attached in the engine.
- **Joins and hand distances were measured on the baked actions in Blender**, not on the FBX. The lab's `join29.py` was not run. The lab's `check_fbx.py` was run on every exported take: steps at most 55.8 deg, root 0.
- **Prop against the body was not measured.** Seen in the stills:
  - the mallet drum touches the back of the helmet in `Mallet_Carry`;
  - in the pair, the drum overlaps the victim's side for frames 17–18 (the victim is the engine's to place);
  - the barrel's inner end is near the belly in the carry.
- **The carry layered over Stride, crossovers and leans was not built.**
- **The renders are stills only**, never in motion: Cycles on the CPU, plain base colours, no toon shader or outline.
- **"Toy, not weapon" is my judgement** from the silhouette cells, not a measure.
- **No puck flight, no effect, no sound.**

## 4. For the engine lane

Files:
- takes: `C:\Users\kazda\jam\sets-lab\weapons\export\<style>\SK_Skater_<style>_<Take>.fbx`
- props: `...\export\props\Prop_<Name>_<style>.fbx`
- `READY.txt` and `JOINS.md` in `...\export\`

All files are in cm, 60 fps, root identity, on the export29 skeleton. Import them by take name onto `SK_SkaterTB1` / `_TB1_P3`.

| Take | Frames (60 fps) | Starts on | Ends on | Trigger | Length | Must never |
|---|---|---|---|---|---|---|
| Giant_Carry | 31, loop | - | - | while weapon 1 is held: layer from `spine_01` up, in place of the aim layer | loop | be played full body over a falling pelvis |
| Giant_Poke | 17 | Giant_Carry f0 | Giant_Carry f0 | use, aim within about 20 deg of ahead | contact 8 | play with the normal stick |
| Giant_Sweep | 23 | Giant_Carry f0 | Giant_Carry f0 | use, aim to a side | contact window 10-14 | - |
| Mallet_Carry | 31, loop | - | - | while weapon 2 is held: layer from `spine_01` up | loop | - |
| Mallet_Swing | 33 | Mallet_Carry f0 | Mallet_Carry f0 | use, with a victim in reach | contact 17 (hip height, 1.1-1.2 m ahead, sweeping right to left); locks 27 | start the victim's fall before frame 16 |
| Mallet_Miss | 45 | Mallet_Carry f0 | Mallet_Carry f0 | use, nobody in reach | locks 40 | - |
| Fall_Bonk | 19 | Glide f0 | GetUp f0 (back lying) | the victim, from the attacker's frame 16 (`SetKnockedDown(true)`, cause mallet) | then `Lie_Loop` and the back get-ups, as after `Fall` | play on a skater lying down already |
| Cannon_Carry | 31, loop | - | - | while weapon 3 is held: layer from `spine_01` up | loop | - |
| Cannon_Fire | 21 | Cannon_Carry f0 | Cannon_Carry f0 | use; launch the puck at frame 8 from the muzzle above | locks 17 | - |
| Cannon_Kick | 31 | Cannon_Carry f0 | GetUp_Side_Right f0 (right side lying) | use, when the game knocks its own skater down (A's rule) | puck at 8; then `Lie_Loop_Side_Right` and the `GetUp_*Side_Right` get-ups | play those get-ups with the cannon: swap to the stick when the kick ends (frame 30) |

**Attaching a prop.** The stick is skinned into the skater mesh, so:
1. Hide it with `HideBoneByName("stick_socket")` while a weapon is held.
2. Do not attach the prop to `stick_socket` itself: a hidden bone scales its attachments to zero. Put a socket on `hand_r` at stick_socket's rest offset instead:
   - location (0, 6.5, 0) cm on cel, (0, 7.0, 0) cm on clay;
   - rotation (Blender bone frame, euler XYZ) 115.12, 70.53, 48.6 deg.
3. The prop frame in the stick_socket bone frame is euler XYZ (90, -62, 0) deg, quaternion (w 0.60611, x 0.60611, y -0.36419, z 0.36419). Prop -Z runs along the bone's +Y. This is the SM_Stick frame.
4. Test it with `SM_Stick_<style>` in `Glide` first, then swap in the prop.

**A prop in the hands during existing takes** (a prop point under the ice in some frame; full lists in `C:\Users\kazda\jam\sets-lab\weapons\work\table_under.md`):
- Giant stick: 126 of 134 takes on cel, 125 on clay, down to 92 cm deep.
- Mallet: 62 / 60 takes (every fall, knockdown, get-up, lying loop, the trip and the stumbles), down to 17 cm.
- Cannon at the hip: only `Knockdown_Right_B` (cel; 9.8 cm deep) and two takes on clay (deepest `Knockdown_Left_B`, 16.1 cm).
- **Rule:** a weapon is shown only in its own takes and its carry layer. On any knockdown, stumble, hit reaction, board hit, shot, poke or lift, swap back to the stick first. The one exception is `Cannon_Kick`, which keeps the cannon to its end.

**The carried puck:** the giant stick's resting blade is 2.1 m ahead, against the sim's long-arm rest of about 1.5 m.

## 5. For the other device

- `SetWeapon(int Kind)`: a level. 0 none, 1 giant stick, 2 mallet, 3 cannon.
- `PlayWeapon(int Kind, float DirX, float DirZ, int Frames, bool bHit)`:
  - (DirX, DirZ): the aim, unit, in rink x / z.
  - `Frames`: the sim frames the user is locked.
  - `bHit`: the mallet lands, or the cannon's recoil knocks its own skater down.
  - The visual picks Poke or Sweep from the aim, Swing or Miss from `bHit`, Fire or Kick from `bHit`.
- **Rules in the game's terms:**
  - Giant stick: the long arm, 3 s of `stick.reach_m` + 0.6. The takes reach 2.65 m on a full poke and sweep a 2.0–2.2 m arc of plus or minus 55 deg.
  - Mallet: a knockdown at short range, 1.1–1.2 m ahead at hip height, sweeping from the right. The contact comes 17 frames after the use, with the mallet visibly raised for 10 of them. On contact the victim gets `SetKnockedDown(true)` with a new cause value `mallet`. A miss locks the attacker for 40 frames.
  - Cannon: the shotgun's shot, launched from the hip. The puck leaves at frame 8, 0.80 m ahead, 0.36 m to the right and 0.92 m up (cel), level.

## 6. Decisions for the operator

1. **Is the rebuilt cannon (a keg on the hip with a crank) far enough from a weapon?** My answer is yes, from the silhouettes.
2. **Accept the carry holds as the weapons' ready stance**, layered over skating?
3. **Giant stick length:** keep 1.6x (resting blade 2.1 m out) or shorten to 1.4x (about 1.8 m)?

## 7. The next three evolutions

1. **A giant poke that reads from behind, and the carry over Stride.**
   - Angle the poke 15 deg off line with the blade open.
   - Build the carry-over-Stride layer with the lab's layer test.
   - Proving numbers: the blade at least 20 px across at the game camera at frame 8 (today a few px: estimated, not measured); the giant blade 0 cm or more over the ice through Stride and Stride_Sprint.
2. **Props clear of the body.**
   - Run the lab's `clear29.py` against the props: the drum against the helmet in the carry, the drum against the victim at contact, the barrel against the belly.
   - Proving number: frames with a prop more than 3 cm inside a body, 0 (today not measured; the stills show touches).
3. **Evolution 3: a giant clay rolling pin.**
   - My three ideas: a spring-loaded boxing glove, a rolling pin, a bellows. I pick the rolling pin: it reads best at 110 px and is a flat two-handed shove at the hip.
   - Proving numbers: as for the other props (tris, reach at contact, wind-up of 8 frames or more, hands on the prop, nothing under the ice, joins 0, no pop) and a game-camera row that reads.

## Files

All under `C:\Users\kazda\jam\sets-lab\weapons\`.

Scripts:
- `build_props.py`, `props_mesh.py`, `weapons_spec.py`
- `build_weapons.py`: the lab's pass-29 file plus two `exec` lines, `weapons_clips.py` and `weapons_tail.py`
- `render_w.py`, `cyc.py` (Cycles on the CPU, 12–16 samples, 6 threads), `compose.py`, `summ.py`

The pre-critic versions of the cannon and clip files are kept in `work\*_e2a.py`.

Numbers:
- `work\<style>\weapons_meas.json`
- `work\table_takes.md`, `work\table_under.md`

The superseded props, mallet and cannon sheets of 02:26 were removed from `devlog\shots`; the giant sheet of 02:26 still stands.

Rebuild: one Blender run per style, about 25 s.

# Part 2: second loop (03:00 to 03:45): the export gate, a poke that reads from behind, carries over skating, props clear of the body, a cannon shot that reads

## 1. Done and not done

Items 1 to 3 are done. Item 4 is done in part. Item 5 (the rolling pin) is not started: item 4 is not finished, and the order put it last.

- **The gate:** the figure lane's `gate.sh` (against export31) gives `GATE PASS: 20 rows, 0 failing, 2 with a known exception`. The exceptions are the hidden-stick shaft check on clay `Cannon_Fire` and `Cannon_Kick` (why: section 2). The first run failed three rows; I fixed all three.
- **The poke:** it now goes 15 deg right of the line with the blade opened, and is 43 px across at the game camera at contact (asked: 20 or more).
- **The carries over Stride, Stride_Sprint and Glide:** all three props stay at or above the ice and clear of the legs in every frame **when the layer is blended in mesh space**. In local space the giant blade goes 29 to 91 cm under the ice: the engine must blend this layer in mesh space (section 4).
- **The mallet carry:** the drum is off the helmet (0.0 cm into the head, both styles).
- **Props clear of the body:** 0 frames with a prop more than 3 cm inside its carrier, in every take, both styles. At contact the mallet's drum meets the victim's left hip and never goes more than 5 cm into him.
- **The cannon shot:** bigger now (a step back, the keg jumping up and settling). The bottom hand stays on the keg in `Cannon_Fire` (0.2 / 0.3 cm). Not done:
  - **The crank turning before the shot cannot be done with one rigid prop on one socket.** The crank turns with the hand, so the keg would turn too. Counter-proposal: a second prop, or a crank bone; one pass, and it needs the engine to attach two parts.
  - **The bottom hand still comes off the keg in `Cannon_Kick` frames 12 to 14:** 5.0 / 12.0 / 7.0 cm on cel, 8.3 / 15.7 / 9.4 cm on clay (it was 28 cm). Out there the arm cannot reach the keg.

## 2. Per item

**1. The gate.** First run: `FAIL p2_cel Giant_Poke: ground` (a skate 0.07 cm under the ice, limit 0.05), `FAIL p2_cel Giant_Sweep: shaft`, `FAIL p3_clay Giant_Poke: shaft`. The giant stick lies on the same line as the hidden stick from the grip, so those shaft frames are real. All three are fixed: the poke's skates are raised 0.6 cm, and the heel targets are moved out to the stick side by 15 to 30 cm.

The gate itself changed during this loop (from 03:12 it reads every FBX in the style folder and its `takes\` subfolder, and counts `READY.txt`'s take list):
- My first export layout (one combined file plus a `takes\` subfolder) counted every take twice. The export is back to one file per take in the style folder.
- `READY.txt` now has a `Takes:` line and a `Take  frames` table the gate reads: 10 listed, 10 checked, both styles.

**The gate is wrong about one check, which I left as a known exception** (`gate_known_weapons.txt`): `shaft` on the cannon and mallet takes. It measures the skinned stick, which the engine hides while a weapon is held. The keg and the mallet are not on that line. The props themselves are measured against the body in item 3.

Final run: all 20 pass. Joins at both ends are within 0.001 cm and 0.000 deg of the named pose (`own:<Weapon>_Carry@0`, `glide`, `lying_back`, `lying_side_right`). The largest bone turn in one frame is 54.8 deg.

Full table: `C:\Users\kazda\jam\sets-lab\weapons\export\GATE.md`.

**2. The poke and the carries over skating.**

`Giant_Poke` is now 18 frames (was 17), contact at frame 9:
- 15 deg right of the line, the blade turned 80 deg open about the shaft.
- At the game camera the blade is 43.0 px across and 18.7 px high (cel), 42.9 / 19.3 px (clay). Before: about 6 px across.
- Reach at contact: 273 / 264 cm from the hips, 23 deg to the right.
- What I saw (giant sheet, rows 1 to 3): the stick goes out to the right of the figure and the blade shows as a block from behind.

The carries, layered from `spine_01` up (mesh space keeps spine_01's armature rotation; local space copies the local channels). Lowest prop point, cm, cel / clay:

| Carry | over Glide | over Stride | over Stride_Sprint | Frames with the prop more than 3 cm inside the carrier |
|---|---|---|---|---|
| Giant_Carry, mesh space | 5.7 / 5.4 | 4.7 / 4.4 | 0.19 / 0.19 | 0 |
| Giant_Carry, local space | 5.7 / 5.4 | -28.6 / -28.0 | -91.0 / -88.6 | 0 |
| Mallet_Carry, mesh space | 89.1 / 82.1 | 88.1 / 81.1 | 83.6 / 76.9 | 0 |
| Cannon_Carry, mesh space | 77.2 / 73.9 | 76.2 / 72.9 | 71.7 / 68.7 | 0 |

- To keep the giant blade over the ice in the sprint, the carry's blade now rides 9 cm over the ice in Glide (was 0).
- What I saw: from the side (clay, over the sprint) the blade slides just over the ice ahead and the shaft stays clear of the legs. From the game camera the giant carry still shows only as a short stub over the head, because the stick points straight ahead.
- The mallet's drum is off the helmet: the deepest the drum goes into the head is 0.0 cm on both styles. I moved it 5 cm further out to the right and back.

**3. Props clear of the body.** Measured on the skinned body, every frame:
- Samples: the prop's vertices plus its axis.
- A sample is inside when it lies behind the nearest face; the figure lane's method.
- Parts: legs, trunk, head. Arms left out, and samples within 13 / 11 cm of the wrists.
- A frame counts when 2 or more samples are more than 3 cm inside.

Result: **0 frames in all 9 prop takes, both styles** (first run: `Giant_Poke` 4, `Giant_Sweep` 3, `Mallet_Swing` 1, `Mallet_Miss` 1, `Cannon_Fire` 4, `Cannon_Kick` 2). The deepest single sample left in any take is 4.1 cm (cel `Giant_Poke`, at the hip beside the top hand).

What changed to get there:
- The giant stick's heel targets moved to the stick side.
- The mallet's recovery comes back low across the front, not over the head.
- The keg sits 4 cm further out to the right.
- The cannon kick's roll holds the keg up at the chest.

**The mallet against a victim:**
- Setup: the victim plays `Fall_Bonk` from the attacker's frame 16, facing the attacker, its root at (1.08, 0.72) m from the attacker's root (x forward, y left).
- The drum's leading end face at contact (frame 17) is at (107, 46, 73) cm (cel) and (103, 45, 70) cm (clay). The swing is now lower: its centre is at 73 / 70 cm high.
- What it meets: the victim's left hip, 0.8 cm into the pelvis (cel) and 1.4 cm into the upper thigh (clay).
- Deepest over frames 14 to 25: 3.6 / 2.4 cm. Frames more than 5 cm in: 0 (asked: no more than 2).
- Placements from y 0.44 to 0.82 m are in `weapons_meas2.json`: closer than 0.48 m the drum goes over 5 cm into him for 1 to 2 frames; past 0.76 m it misses.
- What I saw (mallet sheet, rows 1 to 4): at 17 the red drum's end face is on the victim's left hip and backside; he sits down and lies back by 25.

**4. The cannon shot.** `Cannon_Fire` is now 25 frames (was 21). The puck still leaves at frame 8:
- frame 9: the upper body thrown back 13.5 cm and the muzzle 34 deg up;
- frame 10: 18 cm back and 45 deg up; the front skate lifts and steps back, set down at 12;
- frame 14: the keg dips 6 deg low;
- frame 17: back level;
- frame 21: back on the carry.

Muzzle at frame 8: (79.8, -39.9, 92.1) cm (cel), (78.5, -39.4, 89.0) cm (clay), level and straight ahead. The keg sits 4 cm further right than in the first loop.

The bottom hand's target now follows the keg in every frame, not just at the keys. Bottom hand to the keg:
- `Cannon_Fire`: at most 0.22 / 0.31 cm.
- `Cannon_Kick`: frames 12 to 14 as given in section 1 (was 28 cm).

What I saw (cannon sheet): at the game camera, frames 10 to 12 show the figure leaning back with the keg up, a clear pose change against frames 0 to 8. The crank does not turn.

**Stills (Blender Cycles on the CPU, frozen frames):**
- `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-034146-B-set-weapons-loop2-giant.png`
- `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-034146-B-set-weapons-loop2-mallet.png`
- `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-034146-B-set-weapons-loop2-cannon.png`

## 3. Not verified

- **Nothing seen in the engine:** the mesh-space layer, the prop sockets, the victim spacing.
- **The prop-against-body measure is mine** (`weapons_meas2.py`, built on clear29's idea). It ran in Blender on the baked actions, not on the FBX, and nobody else has checked it.
- **The victim measure uses one fixed placement and facing.** The engine places the victim wherever the sim puts him.
- **The layer test is my model of the engine's layered blend** (local versus mesh space). It has not been compared with the engine's actual result.
- **Stills only**, not in motion.

## 4. For the engine lane (changes from part 1)

Lengths and timings:

| Take | Frames | Contact / event | Back on the carry from |
|---|---|---|---|
| `Giant_Poke` | 18 (was 17) | contact frame 9 | - |
| `Mallet_Swing` | 37 (was 33) | contact frame 17 | 30 |
| `Mallet_Miss` | 50 (was 45) | - | 42 |
| `Cannon_Fire` | 25 (was 21) | puck leaves at frame 8 | 21 |

Everything else as in part 1.

- **Carry layers:** blend them **in mesh space** from `spine_01` (spine_01 keeps its mesh-space rotation). In local space the giant blade sinks up to 91 cm in the sprint.
- **Victim spacing for a mallet hit:** about 1.1 m ahead and 0.7 m to the attacker's left, facing the attacker. Start `Fall_Bonk` at the attacker's frame 16.
- **Files:** `C:\Users\kazda\jam\sets-lab\weapons\export\<style>\SK_Skater_<style>_<Take>.fbx` and `...\export\props\`, with `GATE.md` beside them.
- **The props were rebuilt:** the cannon's keg is 4 cm further from the grip. Same attach rule as before.

## 5. For the other device
No change. The mallet's reach for a hit is 1.1 to 1.2 m ahead and 0.4 to 0.8 m to the attacker's left at contact.

## 6. Decisions for the operator
1. **The cannon's crank:** accept a keg that does not show its crank turning, or spend a pass on a two-part prop (keg on one hand, crank on the other).
2. **The giant carry from the game camera** shows only as a stub over the head. Accept, or carry it angled out to the side the way the poke goes.

## 7. The next three evolutions
1. **Finish the cannon:**
   - A crank that turns: a second prop on `hand_r` and the keg on a socket on `hand_l`.
   - The bottom hand kept within 2 cm through the kick: a shorter throw-back, or let the hand off on purpose and catch the keg again.
   - Proving numbers: the crank turns 360 deg in the 2 frames before frame 8; the bottom hand to the keg 2 cm or less in every frame up to 21.
2. **The rolling pin** (item 5 of this loop). Proving numbers: as for the other props; contact at the hip; nothing to the head.
3. **The giant carry readable from the game camera:** the carry angled 15 deg to the side like the poke. Proving numbers: the blade 20 px or more across at the game camera in the carry over Stride; still 0 cm or more over the ice and 0 frames inside the body.

## Files
- Lab: `C:\Users\kazda\jam\sets-lab\weapons\`
- New this loop: `weapons_meas2.py` (items 2C, 2D, 3), `gate_joins_weapons.txt`, `gate_known_weapons.txt`
- Results: `work\<style>\weapons_meas2.json`
- Changed: `build_weapons.py` (one per-frame pose hook), `weapons_clips.py`, `weapons_tail.py`
- Backups of the first loop: `work\*_e2b.py`, `work\export_e2b_backup\`
- Rebuild per style: `GT=-80 blender -b --factory-startup --python build_weapons.py -- <style> <lab>`, then the `gate.sh` line in section 2.
