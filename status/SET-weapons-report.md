# SET-WEAPONS: toy weapons for the modes (device B, night of 14 Nov, 01:40 to 02:30)

## 1. What is done, what is not

Both evolutions are done in Blender on the CPU, so I am handing back now, before the 04:00 limit. There are three props: `Prop_GiantStick`, `Prop_Mallet` and `Prop_PuckCannon`, one FBX per style. There are ten new takes per style: `Giant_Carry`, `Giant_Poke`, `Giant_Sweep`, `Mallet_Carry`, `Mallet_Swing`, `Mallet_Miss`, `Fall_Bonk`, `Cannon_Carry`, `Cannon_Fire` and `Cannon_Kick`. Each take is exported as its own FBX in both styles. Every take passes these checks: joins exact, nothing under the ice, no bone turn over the lab's limits, the root never moves. The weapon takes keep the top hand on the prop in every frame.

What is not done, and what I changed from the brief:
- **Weapon takes start and end on the weapon's own ready stance, not on Glide.** The brief asked for "from and to the ready stance". Held the way Glide holds the stick, the giant stick's blade would be 0.5 m under the ice. So each weapon has a carry hold (`*_Carry`). Below `spine_01`, the hold is Glide's frame 0, key for key. Above `spine_01`, it holds the weapon. When the engine layers the carry over Glide (the lab's aim layer from `spine_01` up), the result is exactly the pose the weapon takes start and end on.
- **`Cannon_Kick` ends with the cannon 12.8 cm under the ice.** The figure lands in the existing back lying pose, and that pose puts whatever is in the hands along the trunk. The engine has to swap the cannon for the stick at frame 15 (section 4).
- **The bottom hand comes off the cannon in some frames:**
  - in the kick, frames 10 to 14 (up to 23 cm on cel, 28 cm on clay);
  - from frame 21 on, as it lets go into the lying pose;
  - on clay in `Cannon_Fire`, up to 5.0 cm (1.8 cm in the clay carry).
- **The straight poke barely reads from the game camera.** `Giant_Poke` goes straight out ahead, and from behind the stick is seen end-on. The sweep reads well.
- Evolution 3 (my own extra prop) is not started.

## 2. Per evolution

### Evolution 1: the giant stick and the mallet

**Props** (triangles; size in cm along the stick frame's x / y / z):

| Prop | Tris | Size p2_cel | Size p3_clay | Materials (sRGB base colours) |
|---|---|---|---|---|
| Prop_GiantStick | 312 | 56.6 x 11.7 x 303.0 | 56.6 x 11.7 x 287.0 | `ClayTimber` (0.62, 0.42, 0.22), `ClayTape` (0.90, 0.86, 0.74) |
| Prop_Mallet | 404 | 48 x 36 x 111 | 48 x 36 x 107.9 | `ClayTimber` (0.62, 0.42, 0.22), `ClayFelt` (0.78, 0.20, 0.14) |
| Prop_PuckCannon | 376 | 30.1 x 39.1 x 78 | 30.1 x 39.1 x 78 | `ClayTimber` (0.62, 0.42, 0.22), `ClayTin` (0.46, 0.50, 0.52) |

- **The giant stick:** 1.6 times the stick's grip-to-heel length (2.24 m on cel, 2.08 m on clay). The blade is 72 x 24 x 7.5 cm, at a flatter lie (66 deg), so it lies flat at the carry's tilt. The shaft is in two pieces with a splint over the joint, the joint a little skew, held by seven tape wraps. There is a tape patch on the blade and a fat tape knob.
- **The mallet:** a bulged felt drum (33 cm across, 42 cm long) with timber rims and two felt straps, on a turned timber handle. The grip is 80 cm from the drum's centre on cel, 77 cm on clay.
- **What I saw on the props sheet:**
  - The mallet reads at once as a carnival mallet.
  - The giant stick reads as a long taped stick with a big blade, once it is seen from the side.

**Proving numbers** (measured on the baked actions in Blender, every frame, both styles; cel / clay):

| Take | Frames | Contact | Reach from the hips at contact | Lowest prop point cm | Palm r / palm l to the prop cm (max) | Largest bone turn in one frame, deg |
|---|---|---|---|---|---|---|
| Giant_Carry | 31 | - | blade centre 210 / 205 cm, 5.5 deg left | 0.17 / 3.22 | 0.0 / 0.12 ; 0.0 / 0.08 | 0 |
| Giant_Poke | 17 | frame 8 | 265 / 256 cm, blade 3.7 / 3.2 cm over the ice | 0.17 / 1.02 | 0.0 / 1.49 ; 0.0 / 0.48 | 52.2 / 54.0 |
| Giant_Sweep | 23 | frames 10-14 | 197 to 221 cm in the window; arc -55 to +55 deg | 0.17 / 1.97 | 0.0 / 1.62 ; 0.0 / 0.19 | 51.7 / 47.7 |
| Mallet_Carry | 31 | - | - | 89.2 / 85.3 | 0.0 / 0.03 ; 0.0 / 0.02 | 0 |
| Mallet_Swing | 31 | frame 15 | drum centre 117 / 113 cm, 176 / 169 cm high; leading face 158 / 151 cm high | 16.2 / 14.6 | 0.0 / 0.14 ; 0.0 / 0.19 | 46.9 / 46.9 |
| Mallet_Miss | 43 | (frame 15 passes the same point) | as Swing | 21.4 / 19.6 | 0.0 / 0.17 ; 0.0 / 0.19 | 51.2 / 47.3 |
| Fall_Bonk | 19 | struck at 0 | - | (own stick) 0.73 / 0.25 | - | 59.2 (head_socket) / 59.2 |

- **Wind-up:** 12 frames (0 to 12; asked: at least 8). The mallet rises up and out to the right of the helmet, peaking at frame 12 at 214 / 206 cm high.
  - The first version swung straight back over the head. From the game camera it hid behind the helmet, so I moved the wind-up out to the right side.
- **Nothing under the ice.**
  - Body: at least 13.0 cm in the standing weapon takes. In `Fall_Bonk` at least 0.5 / 0.66 cm; the seat is 0.5 / 1.07 cm off the ice at frame 8.
  - Skates: at least -0.07 cm on cel (`Giant_Poke`; Glide itself reads -0.00) and 0.07 cm on clay.
  - Props: as in the table.
- **Joins:** 0 cm / 0 deg after the blend, in both styles (`JOINS.md`). Before the blend, the worst were:
  - `Fall_Bonk` start against Glide: 2.56 cm / 2.9 deg;
  - `Giant_*` start and end against the carry: 0.72 cm / 3.4 deg.
- **No pop:** every bone turn in one frame is 59.2 deg or less (limits: 60 from frame 3 on, 120 in frames 0 to 3). The lab's `check_fbx.py` on the re-imported FBX agrees: at most 57.0 deg, root movement 0 in every take.

Stills (described in section 3):
- `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-022604-B-set-weapons-props.png`
- `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-022604-B-set-weapons-giant.png`
- `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-022604-B-set-weapons-mallet.png`

### Evolution 2: the puck cannon

**The prop:** a 21 cm stovepipe, 78 cm long, made of two pipe pieces pushed together (a bulge where they meet), with a flared round mouth and a capped back. It sits on a timber cradle (two saddles and a plank), with a peg for the top hand and a crank wheel on the left. Two tin patches are riveted on. There is no stock, no trigger and no sight. It is carried on the right shoulder; the bottom hand is under the front of the plank.

| Take | Frames | Firing frame | Muzzle at the firing frame, figure space (cm, X forward, Y left, Z up, origin = root on the ice) | Muzzle direction |
|---|---|---|---|---|
| Cannon_Fire p2_cel | 21 | 8 | (78.0, -30.0, 135.7); pelvis at (0, 0, 71.5) | (1, 0, 0): level, straight ahead |
| Cannon_Fire p3_clay | 21 | 8 | (77.2, -28.9, 131.0) | (1, 0, 0) |
| Cannon_Kick p2_cel | 31 | 8 | (77.8, -30.0, 135.8) | (1, 0, 0) |
| Cannon_Kick p3_clay | 31 | 8 | (80.1, -25.5, 122.3) | (1, 0, 0) |

- **The recoil:**
  - The muzzle kicks up 31.5 deg at frame 9 and 42 deg at frame 10.
  - It travels 43 cm in one frame (frames 8 to 9).
  - The upper body is thrown back 12 cm over still skates.
  - The cannon is level again by frame 13 and back on the carry at 17.
- **The kick:** the same frames 0 to 8, then the figure is thrown back with the front skate up (10). It sits down hard (15 to 17), tips back (20) and lies in the back lying pose from 24, held to 30.
  - The clay kick's frame 8 is 9 cm lower than the clay fire's. The lab's pop pass evened a step around frame 9 differently in the two takes.
- **Other numbers:**
  - Body at least 0.6 / 0.66 cm; skates at least 0.0 / 0.13 cm.
  - Cannon at least 118.7 / 113.8 cm in Fire; in Kick down to -12.8 / -12.4 cm from frame 23 on.
  - The figure's own (hidden) stick stays at least 1.55 / 1.74 cm over the ice in Kick.
  - Largest bone turn in one frame: 55.0 deg. Joins: 0 / 0.
- **What I saw on the sheet:**
  - The carry reads as a fat grey pipe on the shoulder.
  - The brace and the kick-up at frames 9 to 12 read at the game camera, but small.
  - The kick reads clearly: thrown back, the pipe pointing up, sitting down, flat on the back.

Still (described in section 3): `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-022604-B-set-weapons-cannon.png`

## 3. Not verified

- **Nothing has been seen in the engine:** not an import, not the prop on the socket, not the carry layered over the stride.
- **The prop's offset on the socket is computed, not tested.** It is the SM_Stick frame, and SM_Stick itself was imported once (lane SKI3) but never attached.
- **Joins and the hand-to-prop distances were measured on the baked actions in Blender**, before the centimetre export. They were not measured on the FBX; the lab's `join29.py` was not run. The lab's `check_fbx.py` was run on every exported take.
- **Prop against the body was not measured.**
  - From the stills: the mallet's drum touches the back of the helmet in the carry.
  - The cannon tube touches the side of the helmet in the carry, seen from the front left.
  - In the pair render, the mallet's drum overlaps the victim's chest at frames 16 to 18.
- **The carry layered over `Stride`, crossovers and leans was not built or rendered.** The giant blade's height over the ice during a stride is unknown.
- **What I looked at:** stills only, never in motion.
  - The game-camera rows (figure about 110 px, every 2nd or 3rd frame).
  - Closer rows on clay for every take.
  - A two-figure pair for the mallet hit.
- **The renders are not the game's look:** Cycles on the CPU, plain diffuse base colours, no toon shader, no ink outline.
- **Whether the cannon reads as a firearm is a judgement**, not a measure (section 6).
- **No sound, no effect, no puck.** The puck leaving the muzzle was not simulated.

## 4. For the engine lane

**Files** (cm, 60 fps, root identity, same skeleton as export29; import as animation onto `SK_SkaterTB1` / `_TB1_P3` by take name):
- `C:\Users\kazda\jam\sets-lab\weapons\export\<style>\SK_Skater_<style>_<Take>.fbx`
- the props in `C:\Users\kazda\jam\sets-lab\weapons\export\props\`
- `READY.txt` and `JOINS.md` beside them.

| Take | Frames (60 fps) | Starts on | Ends on | Trigger | How the length fits the game | Must never |
|---|---|---|---|---|---|---|
| Giant_Carry | 31, loop | - | - | while weapon 1 is held: layer from `spine_01` up over locomotion, in place of the aim layer | loop (static hold) | be played full body over a take that moves the pelvis (e.g. falls) |
| Giant_Poke | 17 | Giant_Carry f0 | Giant_Carry f0 | use, aim within about 20 deg of ahead | fixed; contact 8; may be play-rate scaled | play with the normal stick (the blade swing is 2.6 m) |
| Giant_Sweep | 23 | Giant_Carry f0 | Giant_Carry f0 | use, aim to a side, or a "clear the zone" use | fixed; contact window 10-14 | the same |
| Mallet_Carry | 31, loop | - | - | while weapon 2 is held: layer from `spine_01` up | loop | - |
| Mallet_Swing | 31 | Mallet_Carry f0 | Mallet_Carry f0 | use, with a victim in front in reach | contact 15; locks the attacker 25 frames (back on the carry at 25) | start the victim's fall before frame 14 |
| Mallet_Miss | 43 | Mallet_Carry f0 | Mallet_Carry f0 | use, nobody in reach at frame 15 | locks 38 frames (carry again at 38) | - |
| Fall_Bonk | 19 | Glide f0 (no weapon) | GetUp f0 = back lying pose | on the victim of a mallet hit, from the attacker's frame 14 (`SetKnockedDown(true)` with cause mallet) | then `Lie_Loop` and the back get-ups as after `Fall` (it ends like `Fall`) | play on a skater lying down already |
| Cannon_Carry | 31, loop | - | - | while weapon 3 is held: layer from `spine_01` up | loop | - |
| Cannon_Fire | 21 | Cannon_Carry f0 | Cannon_Carry f0 | use; launch the puck at frame 8 from the muzzle above | locks 17 frames | - |
| Cannon_Kick | 31 | Cannon_Carry f0 | GetUp f0 = back lying pose | use, when the game decides the recoil knocks its own skater down (the rule is A's) | frames 0-8 as Fire, puck at 8; then `Lie_Loop` and the back get-ups | keep the cannon shown after frame 15: swap it for the stick at frame 15 (it goes 12.8 cm under the ice from frame 23) |

**Attaching a prop.** The stick is skinned into the skater mesh, so:
1. Hide it with `HideBoneByName("stick_socket")` while a weapon is held.
2. Attach the prop with the stick_socket's rest offset. Do not attach to `stick_socket` itself: a hidden bone scales its attachments to zero. Instead put a socket on `hand_r` at stick_socket's rest offset:
   - location (0, 6.5, 0) cm on cel, (0, 7.0, 0) cm on clay;
   - rotation (Blender bone frame, euler XYZ): 115.12, 70.53, 48.6 deg.
3. In the stick_socket bone's own frame (Blender axes), the prop frame is:
   - euler XYZ (90, -62, 0) deg, quaternion (w 0.60611, x 0.60611, y -0.36419, z 0.36419);
   - prop -Z (the shaft) runs along the bone's +Y;
   - prop +X = (0.469, 0, 0.883).
4. This is exactly the SM_Stick frame. The safe test: attach `SM_Stick_<style>` the same way, check it lies on the skinned stick in `Glide`, then swap in the prop.

**Existing takes with a prop in the hands** (a prop point under the ice in any frame):
- The giant stick: under the ice in 126 of 134 takes on cel and 125 of 134 on clay, down to 92 cm deep (the full table is in `C:\Users\kazda\jam\sets-lab\weapons\work\table_under.md`).
- The mallet: under the ice in 62 (cel) and 60 (clay) takes, down to 17 cm.
- The cannon: under the ice in 44 (cel) and 43 (clay) takes, down to 15 cm.
- The mallet and the cannon fail in every fall, knockdown, get-up and lying loop, the trip and the stumbles. They are clean in all aim holds, locomotion, shots and celebrations.
- **Rule:** a weapon is shown only in its own takes and its carry layer. On `SetKnockedDown(true)`, `PlayStumble`, `PlayHitReaction`, `PlayBoardHit`, a shot, a poke or a lift, swap back to the normal stick and play the existing take. The one exception is `Cannon_Kick`: swap at its frame 15.

**The carried puck.** The giant stick's blade rests 2.1 m in front of the hips, but the sim's resting blade with long arm is about 1.5 m. A puck drawn at `puck_socket` (0.78 m) will not be on the giant blade.

## 5. For the other device

- `SetWeapon(int Kind)`: a level. 0 none, 1 giant stick, 2 mallet, 3 cannon. The visual swaps the prop and the carry layer and hides the stick.
- `PlayWeapon(int Kind, float DirX, float DirZ, int Frames)`: one-shot on a use.
  - (DirX, DirZ): the aim, unit, in rink x / z.
  - `Frames`: the sim frames the user stays locked.
  - The visual picks the take: Kind 1 picks Poke when the aim is within 20 deg of the skater's facing, otherwise Sweep. Kind 2 picks Swing or Miss from `PlayWeapon`'s hit result. Kind 3 picks Fire, or Kick when the sim says so.
  - It is better as `PlayWeapon(int Kind, float DirX, float DirZ, int Frames, bool bHit)`. `bHit` is a mallet that lands, or a cannon that knocks its own skater down; it chooses Miss or Kick.
- **As game rules, in the game's own terms:**
  - Giant stick: the "long arm". 3 s of `stick.reach_m` + 0.6, pokes and lifts from further. The takes' blade reach is 2.65 m at full poke and a 2.0 to 2.2 m sweep arc of plus or minus 55 deg.
  - Mallet: a knockdown at short range, 1.1 to 1.2 m in front, with a long telegraph. The contact comes 15 frames after the use, so the victim can see it coming for 12 frames. On contact, the victim gets `SetKnockedDown(true)`; send the cause (e.g. `mallet`) so the visual plays `Fall_Bonk`. A miss locks the attacker 38 frames.
  - Cannon: the shotgun's shot from the shoulder. The puck leaves at frame 8 of the use, 0.78 m ahead, 0.30 m to the right and 1.36 m up (cel), level. Optionally there is a self-knockdown chance that plays `Cannon_Kick`.
- The knockdown cause needs one new value, `mallet`, or the visual has to infer it from `PlayWeapon`.

## 6. Decisions for the operator

1. **Does the cannon read as a firearm?** It is fat, short, round, patched, has no stock or trigger, and is a tin stovepipe. But a tube on the shoulder of a crouching figure is the silhouette of a toy bazooka (the closer rows of the cannon sheet). If that is over the line: carry it cradled in both arms at the hip, like a drum, with a crank you can see turning. That takes about one pass.
2. **The carry holds as the weapons' ready stance** (instead of the plain Glide the brief named). Is the "carry layered over the stride" model accepted?
3. **The giant stick's reach.** A 1.6x stick puts the resting blade 2.1 m out (+0.6 m over the sim's long arm at rest). Either keep it as a silly long reach, or shorten it to 1.4x (blade about 1.8 m at rest).

## 7. The next three evolutions

1. **The giant poke that reads from behind, and the carry over the stride.**
   - Angle `Giant_Poke` 15 deg off the skater's line, with the blade twisted open, so the game camera sees the blade.
   - Build the carry-over-`Stride` blend with the lab's layered-blend test.
   - Proving numbers: the poke's blade in the game-camera cell at frame 8 is at least 20 px across (today a few px: estimated from the sheet, not measured). The giant blade is 0 cm or more over the ice in every frame of `Stride` and `Stride_Sprint` with the carry layer.
2. **Props clear of the body, measured.**
   - Run the lab's `clear29.py` measure against the props: the drum against the helmet, the tube against the helmet, the drum against the victim in the pair.
   - Fix the carry poses and the pair spacing.
   - Proving number: frames with a prop more than 3 cm inside the figure's own body: 0 (today not measured; the stills show the drum and the tube touching the helmet).
3. **Evolution 3: one more prop.**
   - My three ideas: a boxing-glove-on-a-spring (a punch that pops out of a toy box on a stick, knocking down from 2 m); a giant clay rolling pin (a two-handed shove that flattens); a toy bellows (puffs the puck sideways).
   - I would pick the rolling pin: it reads best at 110 px (a fat light cylinder held across the body) and reuses `Fall_Bonk`'s squash.
   - Proving numbers: as for the other props (triangles, reach at contact, wind-up of 8 frames or more, hands on the prop, nothing under the ice, joins 0, no pop) and a game-camera row that reads.

## Files

All under `C:\Users\kazda\jam\sets-lab\weapons\`.

Scripts:
- `build_props.py`, `props_mesh.py`, `weapons_spec.py`
- `build_weapons.py`: the lab's pass-29 file. Two `exec` lines are the only change: `weapons_clips.py` before the adapt loop, and `weapons_tail.py` in place of the bake/export section.
- `render_w.py`, `cyc.py` (Cycles on the CPU, 12 to 16 samples, 6 threads), `compose.py`, `summ.py`

Numbers:
- `work\<style>\weapons_meas.json`: every frame of every new take, the joins, and the props in every existing take.
- `work\table_takes.md`, `work\table_under.md`, `work\props_<style>.json`

Rebuild: `blender -b --factory-startup --python build_weapons.py -- <style> <lab>` takes about 25 s per style. Then run `build_props.py <lab> <style> --render`.
