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
