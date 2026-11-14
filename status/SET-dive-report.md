# SET-DIVE: the dive block and the slide tackle (device B, night of 14 Nov, 01:38 to 02:25)

## 1. Done and not done

Both evolutions are done as Blender takes in both styles. There are five new takes: `Dive_Block_Left`, `Dive_Block_Right`, `Dive_Block_Hit_Left`, `Dive_Block_Hit_Right` and `Slide_Tackle`. Each style has one FBX per take in `C:\Users\kazda\jam\sets-lab\dive\export\p2_cel\` and `...\p3_clay\` (`SK_Skater_<style>_<Take>.fbx`). Beside them are `READY.txt` and `JOINS.md`. The FBX files use the same skeleton, bones, rest pose, meshes and FBX settings as the lab, and the root keys are identity. Every take ends exactly on an existing lying pose. Every take starts exactly where the lab's falls start (frame 0 of `Knockdown_Left` and `Trip`). Nothing in the lab or export29 was changed or re-exported.

Two things fall short of the brief:
- **The flat block's highest point is 60 to 62 cm. The brief asked for about 45 or less.** The side lying pose that the take must end on is 70.5 cm tall itself (the toy's barrel trunk on its side, measured at the `spine_03` skin). Rolling the trunk 48 degrees toward the ice brings it to 60. A cover under 45 cm needs the chest-down variant (proposal 1 below).
- **The slide tackle's blade is 10 cm (cel) and 15 cm (clay) above the ice at contact**, not on it.

For the victim I chose the existing `Trip`, so no new victim take was made (reasons in 4).

## 2. Evolutions

### Evolution 1: the dive block

Keys:
- 0: the falls' start pose.
- 3: gather. The body sinks, the far skate swings out and the stick reaches out low.
- 6: in the air, rolled 55 degrees.
- 9 to 11: flat on the side across the line. Both legs are straight along the ice and the stick lies flat on the ice past the glove, turned 18 degrees toward the shooter.
- 13: the slide. The head lifts and the trunk eases.
- 13 to 27: the stick sweeps round in front over the ice to where it lies along the body, and the trunk rolls back onto the side.
- 27 to 30: the side lying pose, held.

`Dive_Block_Hit_*` (12 frames): the lying pose, then at frame 2 a jolt (trunk curls, knees up 0.6, skates drawn 24 cm, head down), a rebound at 5, and the lying pose from 9.

Proving numbers. All are measured on the re-imported FBX (meas21.py, check_fbx.py, join_dive.py) and given as p2_cel / p3_clay:

| Number | Asked | Dive_Block_Left | Dive_Block_Right |
|---|---|---|---|
| Frame of full cover (width at least 95 % of the take's widest, hips within 3 cm of their lowest) | 12 or less | **10 / 10** (95 % width already from 7 / 6 on the left, 8 / 8 on the right) | **10 / 10** |
| Width across the shot line at full cover, stick tip to skate (skin and stick mesh, y extent) | as wide as possible | **330 / 311 cm** | **315 / 297 cm** |
| Highest point at full cover | under about 45 | **60.6 / 61.1 cm (NOT MET)** | **60.6 / 61.6 cm (NOT MET)** |
| Width and highest point at the lying pose, for comparison | | 192 / 193 cm; 70.5 / 71.4 cm | 183 / 179 cm; 70.5 / 71.6 cm |
| Join of the last frame to Lie_Loop_Side_*, GetUp_Side_*, GetUp_Late_Side_*, Knockdown_* last frame (export29) | 0.0005 cm / 0 deg | 0.00001 / 0.00001 cm, 0 deg | 0.00003 / 0.00002 cm, 0 deg |
| Lowest body / skate / stick | 0 or more | 0 / 0 / 1.11 cm; 0.55 / 0.13 / 0.58 cm | 0 / 0 / 0.95; 0.95 / 0.13 / 2.25 |
| Steps over the pop limits (60 deg from frame 3, 120 deg in frames 0 to 3; pops24 rule) | 0 | 0 / 0 (worst 62 / 70 deg, frames 2 to 3) | 0 / 0 (worst 66 / 74 deg, frames 2 to 3) |
| Blade end, largest one-frame move | (the lab's line is 40 cm) | 70.0 / 68.1 cm in the throw (frames 1 to 5); about 40 cm in the sweep | 63.3 / 58.8 cm in the throw |

`Dive_Block_Hit_*`:
- Joins at frame 0 and the last frame: 0 to 0.00001 cm, 0 degrees, both styles.
- Largest joint travel: 32.3 / 28.7 cm (the skate, frame 2).
- Body under the ice: 0 cm. A first, bigger jolt put it 0.36 cm under; I raised the jolt pose 7.5 cm to fix that.
- Pop steps: 0.
- Root: identity in all five takes and both styles (root20.py, deviation 0.0000000), 60 fps.

The still is `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-022026-B-set-dive-dive-block.png` (txt beside it).

What I saw in it, game camera at 110 px: frames 0 to 3 are the gather; at 6 the figure is airborne and tipping; from 9 it is flat across the screen, skates at one end and the stick reaching past the glove at the other, which reads as a body thrown across the lane. At 18 to 24 the stick swings up the screen (forward, over the ice) and comes back along the body. From 27 it is the lying pose. Clay reads the same. The hit's jolt (knees and trunk curling, frames 1 to 4) is visible at game scale but small. From the shooter's closer row the stick lies on the ice from frame 6 and the body is flat by 9 to 10.

### Evolution 2: the slide tackle

`Slide_Tackle` is 24 frames, feet first. Keys:
- 3: drop. The left skate goes forward and the stick reaches forward-left.
- 6: down on the right hip, trunk back, the leading leg along the ice.
- 10: **contact**. The leading leg is at full reach and the stick, blade flat sideways, has swept from forward-left to forward-right.
- 14: rolling onto the back.
- 19 to 24: `Fall`'s lying pose on the back (feet forward), held. `Lie_Loop`, `GetUp`, `GetUp_Quick`, `GetUp_Snap`, `GetUp_Late` and `GetUp_LateLong` all follow it.

| Number (p2_cel / p3_clay) | Value |
|---|---|
| Contact frame | **10** |
| Leading (left) skate's reach from the hips at contact, horizontal | ankle **66 / 57 cm**; skate toe **84 / 74 cm** ahead of the hips (near a straight leg: the IK reach guard limits it) |
| Stick blade end's reach from the hips at contact | **214 / 200 cm** (blade 10 / 15 cm above the ice: NOT on it) |
| Trunk during the slide (frames 6 to 14) | hips 26 to 32 / 33 to 39 cm; neck 44 to 66 / 51 to 72 cm; highest point 77 to 104 / 79 to 106 cm (sitting back on the hip, not lying) |
| Joins: last frame to Lie_Loop, GetUp, GetUp_Late frame 0 and Fall's last frame | 0.00003 to 0.00004 cm, 0 deg; frame 0 = Trip frame 0: 0 cm, 0 deg |
| Ground | body 0.6 / 0.66, skate 0 / 0.13, stick 1.0 / 0.42 cm |
| Pop steps over the limits | 0 / 0 (worst 81 / 90 deg upperarm_l frames 1 to 2, inside the 120 contact window) |
| Blade end, largest one-frame move | 53.5 / 50.4 cm (frames 8 to 9, the sweep) |

The still is `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-022026-B-set-dive-slide-tackle.png` (txt beside it).

What I saw in it: from the side (closer row) it reads as a feet-first slide. The figure sits down and back by 6, legs and stick forward along the ice through 8 to 12, then rolls flat on the back by 18. From the game camera (behind) the stick's swing forward and the lean back read. **The leading leg is mostly hidden under the body from behind**, so "legs first" reads less well than "sat back with the stick out".

The victim is the existing `Trip`, seen in the same still at game scale: a skate kicked out sideways and up, a half twist, down on the side by 14. It reads as legs taken from under him. `Fall` (feet up in front, onto the back) also reads, but as being knocked over backward, not as legs swept.

A miss needs nothing more: `Slide_Tackle` already ends lying, and the attacker's cost is the existing lying loop and get-up.

## 3. Not verified

- Nothing has been seen in the engine: not the imports, not the take names after import, not the cross-fades, not the turn the game gives the figure.
- Stills only. I did not look in motion. I did not look at the hit or the right-hand takes from above, or at the slide from the front.
- The flat-cover frames were checked for frames 10 to 14 and the lying hold. I did not check frame by frame whether the stick passes through the body during the sweep (frames 15 to 26) or the slide's roll. In the above view the shaft sweeps in front of the body, but the left glove is near the shaft around frame 24 and that was not looked at closely.
- "Width" is the extent across the figure's facing line (Y) of skin plus stick mesh. It assumes the game faces the figure to the shooter.
- The engine still needs a test trigger.
- These renders use Cycles with simple materials, not the lab's Workbench look (no ink outline, no clay matcap). Colours and silhouette are right; the look is not the game's.
- My check FBX (work\<style>\check_<style>.fbx) holds the lab's lying takes rebuilt by the same code. It was used only for the renders. The joins were measured against export29 itself.

## 4. For the engine lane

Import by take name into `BP_SkaterVisual_TB1` (p2_cel) and `_TB1_P3` (p3_clay). The takes are additions, at 60 fps, with the root identity on every key.

| Take | Frames (last frame index) | Starts on | Ends on (held from frame) | Trigger | Key frames | Must never |
|---|---|---|---|---|---|---|
| Dive_Block_Left | 30 | the ready stance as every fall starts (= Knockdown_Left frame 0) | the side lying pose of Knockdown_Left / Lie_Loop_Side_Left (held from 27) | block call, side +1 | full cover 10 (95 % width from 7); stick sweep 13 to 27 | move the root (travel and slide are the game's); be cut before 27 without a cross-fade |
| Dive_Block_Right | 30 | same (= Knockdown_Right frame 0) | Lie_Loop_Side_Right's pose (held from 27) | block call, side -1 | full cover 10 | same |
| Dive_Block_Hit_Left | 12 | Lie_Loop_Side_Left frame 0 | the same pose (held from 9) | puck hits a lying blocker | jolt peak 2 | play from anything but the side lying pose (or a clean loop frame 94-107, 0-14, 40-68 with a 4-frame fade) |
| Dive_Block_Hit_Right | 12 | Lie_Loop_Side_Right frame 0 | the same (held from 9) | same | jolt peak 2 | same |
| Slide_Tackle | 24 | = Trip / Knockdown frame 0 | Fall's lying pose on the back = Lie_Loop frame 0 (held from 19) | slide-tackle call | **contact 10**: land the hit on the victim there | move the root |

Side naming follows the lab's lying-pose suffix, not the travel direction:
- `_Left` ends on the `_Side_Left` pose: down on the right hip, head to the figure's right, legs to its left, the stick held out to the right at cover.
- `_Right` is the mirror (down on the left hip, head to the left).

After the take, the existing chain follows, using the rule and the clean frames of the twenty-seventh pass:
- Dive: `Lie_Loop_Side_*` with the side get-ups (`GetUp_Side_*`, `_Quick_`, `_Snap_`, `_Late_`, `_LateLong_`).
- Slide: `Lie_Loop` with the back get-ups.

Start the loop on the frame the take first holds its end pose (27, 19 or 9) with a 4-frame fade, as for the falls.

Timing: the takes play at play rate 1 to the hold frame. A lock longer than the take is the lying loop. A lock shorter than 27 frames for the dive should scale the play rate rather than cut: frames 10 to 14 must be seen.

Victim of the slide tackle: play the existing `Trip` on the carrier at the attacker's frame 10. It ends on `Lie_Loop_Side_Right`'s pose; nothing new is needed.

## 5. For the other device (the lead sends it)

- `PlayDiveBlock(DirX, DirZ, Frames)`
  - DirX/DirZ: the unit direction toward the shooter or the puck in the ice plane. The game turns the figure to face it and picks the side from the sign of the shot line's offset to the figure: left of the figure means `Dive_Block_Right` (head and stick toward the left), right means `_Left`.
  - Frames: the total lock in sim frames (60 Hz), at least 30. The dive plays at rate 1 to frame 27, then the lying loop until Frames minus the get-up's length, then the get-up.
  - The lying and get-up time is the game's `SetKnockedDown(true)` from the dive's frame 27 to the get-up. The dive's own 27 frames are the move.
- `PlayDiveBlockHit()`: no inputs. Valid only while the blocker is lying from a dive. Plays 12 frames on top of the lying time; does not extend it.
- `PlaySlideTackle(Frames, ContactFrame = 10)`
  - Frames: the total lock in sim frames, at least 24.
  - The game resolves the hit at frame 10 of the take (166 ms after the call). On a hit it calls the victim's `PlayTrip` (or the existing stumble or knockdown call that plays `Trip`).
  - The attacker always goes to `SetKnockedDown` from frame 19, hit or miss, so the cost of a miss is the lying time plus the get-up.
- If the game sends nothing more after the move: the figure holds the take's last frame (an exact lying pose). The existing lying loop then plays only if the game starts it, and the existing get-up follows when `SetKnockedDown(false)` is sent.

## 6. Decisions for the operator

1. The cover's height. The side block is 60 cm high at its flattest (the trunk of the toy figure lying on its side). Accept it, or ask for the chest-down block (`Dive_Block_*_B`, ending face down, about 50 cm) as the main block.
2. The victim of the slide tackle: the existing `Trip` (legs swept sideways, down on the side). The alternative is a new `Fall_Legs_Front` (feet taken forward, onto the back) if a front-on slide should look different.
3. Which side naming the game uses: the lab's lying suffix (as built) or the travel direction.

## 7. The next three evolutions

1. **`Dive_Block_Left_B` / `_Right_B`, chest first with the stick out in front, ending on `Lie_Loop_Front`'s pose.** Proving numbers:
   - highest point at full cover 50 cm or less (the face-down pose is 54.5 cm today; today's block is 60.6);
   - full cover by frame 10;
   - width 250 cm or more;
   - joins 0.0005 cm / 0 deg;
   - pop steps 0.
2. **The slide tackle reads from behind, with the blade on the ice.** The leading leg out to the side of the body as seen from the game camera (skate visible), and the blade flat on the ice through the sweep. Proving numbers:
   - blade height at contact 3 cm or less (today 10 / 15);
   - leading skate outside the trunk's silhouette in the game camera at frame 10 (today hidden);
   - blade end's largest move 40 cm or less (today 53.5).
3. **The stick's throw and sweep within the lab's 40 cm line, and a check for the stick through the body.** Run clear29.py on the five takes. Proving numbers:
   - blade end's largest one-frame move 40 cm or less (today 70 in the throw, about 40 in the sweep);
   - frames with the shaft through a leg or the trunk: 0 (today not measured).

Files: the lab is `C:\Users\kazda\jam\sets-lab\dive\`.
- Takes and build: `dive_block.py` holds the takes, inserted into the lab's `build_clips.py` as `build_dive.py`, with its own export section for one take per FBX.
- Checks: `it.sh` (build, meas, check, join; one Blender at a time), `join_dive.py`, `sum.js`, `prove.js`.
- Sheets: `render_dive.py` (Cycles on the CPU) and `strip2.sh`.
- Numbers and logs: `work\<style>\m\` and `logs\final_run.txt`.

---

# Part 2, second loop (02:22 to 03:45): the stick's path and the body, the block as an act, the chest-first block, the slide tackle from behind

## 1. Done and not done

All three evolutions are built in both styles, plus the lead's addition (the block is an act). The lab export `C:\Users\kazda\jam\sets-lab\dive\export\` was replaced in place: one FBX per take, `READY.txt` (written last), `JOINS.md` (29 pairs) and the gate's `GATE.md`.

The export holds 8 takes:
- `Dive_Block_Left`, `Dive_Block_Right`: changed, now **38 frames**, with the act.
- `Dive_Block_Hit_Left`, `Dive_Block_Hit_Right`: changed, the stick kept off the legs.
- `Dive_Block_Front`: new.
- `Slide_Tackle_Left`, `Slide_Tackle_Right`: new.
- `Slide_Tackle`: kept under the first loop's name, the same keys as `_Left`.

The gate passed: `GATE PASS: 16 rows, 0 failing ... p2_cel: 8 files, 8 takes found, 8 checked, READY.txt lists 8; p3_clay: the same`, against base export31, in 65 s. Besides `GATE.md` the gate also wrote its work files `gate_work\` into the export folder.

Not met:
- **The highest point at full cover:**
  - side blocks 59.6 to 60.6 cm; the chest-first block 61.4 (cel) and 57.6 (clay). Asked: 50 for the chest-first block, about 45 for the side blocks.
  - On this figure the floor is the head. The face-down lying pose itself is 54.5 / 52.1 cm, and its highest point is the helmet.
  - Pushing the crown further forward put the helmet into the ice face first (-1.4 cm), so I backed off to keep the helmet clear: it is 3.7 cm off the ice at frame 10 on cel and 19.9 on clay.
  - During the act (head up) the highest point is 64 cm.
- **`Slide_Tackle_Right`: the shaft is 3.9 to 4.7 cm inside a leg for 2 frames (cel, 12 to 13) and 1 frame (clay, 13).** I tried four stick paths after contact. Each one that cleared the leg put the stick or the body under the ice or broke the 40 cm line, so I kept this one.
- **The dive is 38 frames.** The first brief asked for 24 to 30. The act (10 to 20) plus a settle inside the 40 cm blade line (a 160 degree stick turn takes at least 13 frames) does not fit in 30.

## 2. Evolutions

### Evolution 1: the stick's path and the body, and the block as an act (Dive_Block_Left / _Right, the hit takes)

What changed:
- **The throw** is spread evenly over frames 0 to 10 (linear keys).
- **The stick at cover** is laid across the game camera's view (yaw 70 degrees out to the side, was 45 toward the shooter), and stays braced out on the ice to frame 20.
- **The act, frames 12 to 20:**
  - The head comes up off the ice and the face turns to the shooter. Neck and head angles are solved per key toward a target direction (`aim_head26`).
  - The top leg stacks and lifts.
  - At frame 15 the stick hand and the top glove reach once more (8 cm out), and at 18 there is a shuffle.
- **The settle, frames 20 to 35:** the stick comes in over the ice as part of it. There is one key a frame on an even speed profile with 3-frame ramps, the trunk rolls back with it, and the top glove lifts 16 cm while the shaft passes under it.
- **Lying pose** from 35, held to 38.
- **Hit takes:** the stick's far end is turned 16 degrees off the drawn-up legs during the jolt, and the head moves away from the stick on the right side. The take is back on the lying pose from 10.

Proving numbers, measured on the FBX, cel / clay:

| Number | Asked | Dive_Block_Left | Dive_Block_Right |
|---|---|---|---|
| Blade end, largest one-frame move | 40 cm or less | 39.2 / 38.1 | 39.1 / 36.0 |
| Frames with 5 cm of shaft more than 3 cm inside a leg, the trunk or the head (clear29.py) | 0 | 0 of 39 / 0 of 39 | 0 / 0 |
| Full cover from frame | 12 or less | 10 / 10 | 10 / 10 |
| Width across the shot line at full cover; frames 10 to 20 | | 328 / 309 cm; 328-338 / 309-319 | 313 / 295; 313-327 / 295-307 |
| Highest point at full cover; during the act | ~45 (not met) | 59.6 / 60.2; 66.7-69.7 / 63.0-66.5 | 59.6 / 60.6; same |
| Act 12-20: helmet's lowest point above the ice (lying pose) | up | 17.6-21.4 (10.6) / 24.8-29.1 (14.9) cm | same |
| Act 12-20: face direction from the shooter (yaw; at cover f10; lying) | turned | 0.1-0.4 deg (29.9; 7.8), pitch +9 to +16 (lying +2.7) | same |
| Act 12-20: the top leg's lift (ankle over the lying pose's) | | 26.0 / 23.9 cm | 26.0 / 23.9 cm |
| Pop steps over the limits | 0 | 0 / 0 | 0 / 0 |

- **Hit takes:** blade end at most 37.8 cm in one frame; shaft through 0 of 13 frames; body 0 cm or above; pops 0.
- **The slides:** blade end at most 36.4 cm. Shaft through: `_Left` 0, `_Right` 2 / 1 (above).
- **Glove and shaft**, `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-033953-B-set-dive-glove-shaft.png`, cel, frames 24 to 35, close from above and from the shooter:
  - The shaft swings round in front over the ice and passes under the lifted top glove at 32 to 33. The glove is above it and touching it.
  - By 35 the stick lies along the body as in the lying pose.
  - I saw no shaft through the glove at that size; hands are left out of clear29's measure.
- **Knockdown comparison**, in `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-033953-B-set-dive-loop2-blocks-slides.png` (rows 1 and 2 of each style):
  - From frame 10 to 24 the block shows the stick laid out across the screen past the glove and a leg lifted off the other leg. `Knockdown_Left` lies with the stick tucked and the legs together from 15.
  - Honestly: a stranger would tell them apart by the stick and the raised leg while they are on screen together, frames 10 to 24.
  - The head's turn does not read at 110 px: the helmet is a white ball.
  - From 32 on the two are the same picture, because the block settles onto the knockdown's own lying pose by design.

### Evolution 2: the chest-first block (`Dive_Block_Front`, one centred take)

**Why one take, not `_Left_B` / `_Right_B`:** with the body flat on the chest across the shot line, the two sides would differ only in the head's turn toward the shooter. That turn does not read at 110 px (see above), so the two would be the same picture. The game faces the figure along the dive, across the lane, and the width counts along the dive (x).

Keys:
- 3: gather.
- 6: airborne, near horizontal.
- 9: the forearms take the ice, the head held off it.
- 10: full cover, arms stretched ahead, the stick flat out ahead-right.
- 12-20: the act. The head comes up looking along the ice, a skate lifts behind, the stick reaches at 15, a shuffle at 18.
- 20-35: the settle. The stick turns back round the stick side to lie along the body.
- 35-38: the face-down lying pose of Fall_Forward / Lie_Loop_Front.

| Number (cel / clay) | Asked | Value |
|---|---|---|
| Full cover from frame | 10 | 10 / 10 |
| Width across the shot line (along the dive) at full cover; frames 10 to 20 | 250 or more | 317 / 297 cm; 306-317 / 287-297 |
| Highest point at full cover | 50 or less | **61.4 / 57.6 cm, NOT MET** (the lying pose itself 54.5 / 52.1; during the act 64.0-64.5 / 61.2-61.6) |
| Landing: the helmet's lowest point at frames 8, 9, 10, 11 | not face first | 24.9, 13.2, 3.7, 14.4 cm / 37.6, 27.8, 19.9, 25.2 cm. The elbow joints are at 19.8 / 20.3 cm at frame 9, below the helmet, so the forearms land first. On cel the helmet comes within 3.7 cm of the ice at frame 10. |
| Act: face pitch (lying pose) | | +16.5 (-22.9) |
| Act: the lifted skate (lying pose's ankle) | | 26 / 25.9 cm (22.8 / 23.6): only 3 cm of lift |
| Joins (last frame = Lie_Loop_Front, GetUp_Front, GetUp_Late_Front frame 0, Fall_Forward end; frame 0 = Fall_Forward frame 0) | exact | ≤ 0.00004 cm, 0 deg |
| Lowest body / stick | 0 or above | 0.31 / 1.39 cm; 0.47 / 0.54 cm |
| Pops; shaft through; blade end | 0; 0; 40 | 0; 0 of 39; 38.2 / 33.0 cm |

What I saw (game camera, `...20261114-033953-B-set-dive-loop2-blocks-slides.png`, rows 4 and 5 of each style): the game camera here is behind the figure, along the dive. From there the body is foreshortened and the stick reaches out ahead-right from frame 9. The settle swings the stick round to the right and back, 28 to 32. From the side (closer row) the body is flat along the ice from 9, with the stick ahead. In the game the camera will mostly see this take from the side, because the dive runs across the lane. That view at game scale was not rendered.

### Evolution 3: the slide tackle that reads from behind (Slide_Tackle_Left / _Right)

- **Leading leg:** goes out to the side of the trunk (ankle 37 cm off the centre line) and forward.
- **Stick at contact:** the grip is higher (30 cm) with the shaft angled down, so the ice clamp lays the blade on the ice. Its margin is 1.4 cm for these takes; a per-take setting was added in my copy of the lab's bake.
- **`_Right`:** down on the left hip, the right leg leading. The stick sweeps across from right to left. The trunk rolls less and leans back more than the mirror would, so the high stick shoulder can reach the ice.
- **Contact** stays at frame 10.

| Number (cel / clay) | Asked | Slide_Tackle_Left | Slide_Tackle_Right |
|---|---|---|---|
| Blade end's height at contact (frame 10) | 3 cm or less | 1.70 / 1.93 cm (stick's lowest point 0.81 / 0.41) | 1.67 / 1.96 cm (0.78 / 0.44) |
| Leading skate seen from the game camera at frame 10 (vis_dive.py: share of the skate's vertices a ray to the camera reaches) | outside the silhouette | 70 % / 52 % (trailing skate 16 % / 31 %) | 70 % / 52 % (trailing 26 % / 31 %) |
| Leading ankle from the hips; blade from the hips at contact | | 71 / 63 cm; 205 / 196 cm | 75 / 68 cm; 159 / 152 cm |
| Joins (Lie_Loop, GetUp, GetUp_Late frame 0, Fall end; frame 0 = Trip frame 0) | exact | ≤ 0.00004 cm, 0 deg | same |
| Ground, pops, blade end | | body 0.6 / 0.66, stick 0.48 / 0.02; pops 0; 36.4 / 34.7 cm | body 0.6 / 0.66, stick 0.76 / 0.07; pops 0; 32.7 / 32.1 cm |

**The pair strip:** `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-033953-B-set-dive-slide-meets-trip.png`, both styles, game camera behind the attacker, the attacker in blue and the victim in orange.
- **Setup:** the victim faces the attacker with its skates on the attacker's leading skate at frame 10, and starts `Trip` there. The takes have no travel, so the attacker's slide (1.2 m to contact, 1.6 m in all) is put on the attacker's object for this strip only.
- **What I saw:** up to frame 14 it reads as a tackle on the ice. The blue figure slides in low, stick and skate reaching the orange skates at 10, and orange's leg kicks out sideways as he twists away (12 to 14).
- **From frame 17 on it reads as two players falling over each other.** `Trip` throws the victim onto his side toward where the attacker lies, and from 20 the two bodies lie across and partly inside each other.
- The tackle reads; the landing does not. Proposal 1 below addresses it.

## 3. Not verified

- Nothing has been seen in the engine.
- I looked at stills only, never in motion.
- The head's turn toward the shooter is measured (0.1 to 0.4 degrees) but does not read at game scale.
- I did not render `Dive_Block_Front` from the side at game scale, which is the view the game will mostly give.
- The pair strip's travel is invented for the strip. In the game the victim also moves, and the attacker's slide is the game's.
- The glove and shaft were looked at close only on cel `Dive_Block_Left`, frames 24 to 35; hands are not in clear29's measure.
- The renders are Cycles with simple materials, not the game's look.
- Not done in this loop: `Dive_Block_Hit_*` and the slides from the shooter's side; the head yaw of the right-hand takes beyond the numbers.

## 4. For the engine lane

| Take | Frames (0..N) | Starts on | Ends on (held from) | Trigger | Key frames |
|---|---|---|---|---|---|
| Dive_Block_Left | 38 | the falls' start (= Knockdown_Left frame 0) | Lie_Loop_Side_Left frame 0 (35) | block, side +1 | cover 10; act 12-20; settle 20-35 |
| Dive_Block_Right | 38 | = Knockdown_Right frame 0 | Lie_Loop_Side_Right frame 0 (35) | block, side -1 | same |
| Dive_Block_Hit_Left / _Right | 12 | Lie_Loop_Side_* frame 0 | same pose (10) | puck hits a lying blocker | jolt 2 |
| Dive_Block_Front | 38 | = Fall_Forward frame 0 | Lie_Loop_Front frame 0 (35) | block, chest first (the figure faced along the dive) | forearms 9; cover 10; act 12-20 |
| Slide_Tackle_Left | 24 | = Trip frame 0 | Lie_Loop frame 0 (19) | slide tackle, down on the right hip | contact 10 |
| Slide_Tackle_Right | 24 | = Trip frame 0 | Lie_Loop frame 0 (19) | slide tackle, down on the left hip | contact 10 |
| Slide_Tackle | 24 | | | the first loop's name, same keys as `_Left` | contact 10 |

- **Must never:**
  - move the root;
  - be cut in the act (10 to 20);
  - for a lock shorter than 38 frames, cut the dive: scale its play rate instead, so the act and the settle both play;
  - play `Trip` on a victim standing where the attacker will lie (see the pair strip).
- **Lying and get-ups:** loops and get-ups follow from the hold frame with the twenty-seventh pass's clean-frame rule and a 4-frame fade.

## 5. For the other device

As sent in the first part, with these changes:
- `PlayDiveBlock(DirX, DirZ, Frames)`: Frames is at least 38. `SetKnockedDown(true)` starts at the dive's frame 35.
- New: `PlayDiveBlockFront(DirX, DirZ, Frames)`. Dir is the dive's direction: the figure is faced along it, across the shot line. Frames is at least 38.
- `PlaySlideTackle(Frames, Side, ContactFrame = 10)`: Side is +1 for `_Left` (right hip) and -1 for `_Right`, for example the side away from the carrier's stick.
- **On a hit, the game should push the victim away from the attacker's lying spot** (along DirX/DirZ, about 1 m) while `Trip` plays, or the two land on each other.

## 6. Decisions for the operator

1. **The cover height.** The toy's head sets a floor of about 52 to 55 cm for any lying block on this figure. Accept about 60 cm, or make the head smaller in the lying blocks (a figure change, not an animation one).
2. **The dive's length.** 38 frames (the act plus a clean settle), or a 30-frame version with no act and a faster stick (blade about 55 cm a frame).
3. **The slide's victim.** Keep `Trip` with the game pushing the victim clear, or add a victim fall that goes away from the attacker (`Fall_Legs_Front`, falling forward over the slider).

## 7. The next three evolutions

1. **The tackle's landing apart.** A victim take that falls forward and away over the slider's legs, ending face down (`Fall_Legs_Front`), and the pair strip again with the same placement. Proving numbers:
   - frames after contact in which the two bodies interpenetrate (mesh against mesh, measured): 0 (today about 16, from 20 on, by eye);
   - victim lying 80 cm or more from the attacker's hips.
2. **`Slide_Tackle_Right` clear, and the slides at the game's side view.** Proving numbers:
   - shaft through a leg: 0 frames (today 2 / 1);
   - leading skate visible from the game camera: 60 % or more on clay (today 52 %).
3. **The act reads at 110 px.** Make the head and top arm's move bigger than the turn of a white ball: the top glove lifting off the ice at the second reach, and the stick sliding 20 cm along the ice. Proving numbers:
   - at game scale, pixels changed between frames 12 and 18 inside the figure's box: 3 times today's;
   - a stranger tells the block from `Knockdown_Left` in a single still at frame 18 (a two-person check, not something I can measure alone).

Files, in the lab `C:\Users\kazda\jam\sets-lab\dive\`:
- Takes and build: `dive_block.py`, built by `mkbuild.sh` into `build_dive.py`.
- Checks: `it.sh` (steps build meas check clear head join), `clr.js`, `prove2.js`, `head_dive.py`, `vis_dive.py`, `clear29.py` (copy).
- Renders: `pair_dive.py`, `render_dive.py`, `strip2.sh`.
- Gate: `gate_joins_dive.txt`, the gate's log `logs\gate.txt`.

---

# Part 3, third loop (03:42 to 04:30): the victim falls clear, Slide_Tackle_Right clear, the act in the big parts

## 1. Done and not done

All three evolutions are built in both styles. The export `C:\Users\kazda\jam\sets-lab\dive\export\` was replaced in place. It now holds 10 takes per style:
- Changed: `Dive_Block_Left` and `_Right` (40 frames now), `Slide_Tackle_Left` and `_Right`, and `Slide_Tackle`, which is still the same keys as `_Left`.
- New: `Fall_Legs_Front` and `Fall_Legs_Back`.
- Unchanged: `Dive_Block_Hit_*` and `Dive_Block_Front`.

Beside the FBX files are `READY.txt` (written last, every take with its start and end pose and key frames), `JOINS.md` (37 pairs) and `GATE.md`. The gate ran with `--out C:\Users\kazda\jam\sets-lab\dive\gate3`, so its work files are there; only `GATE.md` was copied into the export.

**Gate:** `GATE PASS: 20 rows, 0 failing, 0 with a known exception; p2_cel: 10 files, 10 takes found, 10 checked, READY.txt lists 10; p3_clay: the same`, base export32, 47 s.
- Its new jitter column (informational, not a fail) shows a joint turning back by more than 1 cm on 2 to 3 frames in a row in:
  - `Fall_Legs_Back`: hand_l up to 21 cm on cel, lowerarm_r 11.3 cm on clay;
  - `Dive_Block_Left` on cel: lowerarm_l 17.1 cm from frame 1;
  - `Dive_Block_Right` on clay: lowerarm_l 13.1 cm from frame 2;
  - the slides: calf or lowerarm, 2.7 to 6 cm.
- These are the throw and the fall's flailing. I did not fix them.

**Not met:**
- **The act's pixel change between frames 12 and 18 is 2.2 to 3.5 times the second loop's** (asked: 3 times). Only one of four take-and-style cases reaches 3 times.
- **The victim falls clear only with invented travel.** Zero overlap needs the carrier's own skating (front 2.1 m, back 1.6 m) and a sideways knock (1.0 to 1.2 m). Both are invented in the strip, like the attacker's slide.
  - With the takes alone it cannot be done. With no travel, any lying pose lies on the contact point, and the slider stops 40 cm past it and lies across it.
  - Without the sideways knock and with the skates exactly on the leading skate, the overlap in my count on cel was 13 frames (front: 7, 8 and 10 to 20, so 10 after contact) and 26 frames (back: 4 to 8 and 16 to 36, so 21 after contact).
  - This is a request to the other device (section 5).

## 2. Evolutions

### Evolution 1: the victim falls clear

Both takes are 26 frames.
- **Contact** is at frame 2: the engine starts every fall there, on the attacker's contact frame (thirty-first pass).
- **`Fall_Legs_Front`**, the slider coming from his front:
  - The skates are swept back at 2. He tips forward with the feet up behind (5), flops onto the forearms and chest with the head held up (9 to 12), bounces (15).
  - Ends on **the face-down pose of Fall_Forward / Lie_Loop_Front**, held from 19.
- **`Fall_Legs_Back`**, from behind:
  - The skates are swept forward and up at 2. He is upended with the feet high (5 to 9) and sits down hard: **the hips land first** (11 to 12), back curled, head up.
  - Ends on **Fall's / Lie_Loop's pose on the back**, held from 19.
- **Joins:** exact against export29 (0.00004 cm or less, 0 deg) and in the gate against export32 (0.0000 cm, 0.000 deg).

**Not head first:**
- `Fall_Legs_Back` on cel: hips 5 cm off the ice at frame 11, helmet 47 cm.
- `Fall_Legs_Front` on cel: the helmet's lowest point never goes below 7.5 cm before the settle (at 12). Its last 1.4 cm is the lying pose's cheek on the ice, not an impact.

**Pair strips** (pair3.py): `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-042658-B-set-dive-victims-fall-clear.png`.
- `Slide_Tackle_Left` with each victim, from his front and from behind, both styles, game camera behind the attacker. The victim's take runs from its frame 2 at the attacker's frame 10.
- The overlap count tests every second body vertex of each figure against the other's body (more than 1 cm behind the nearest face, within 15 cm).
- Travel invented for the strip:
  - the attacker slides 1.6 m (1.2 m by contact);
  - the carrier skates on along his facing: front 2.1 m, back 1.6 m, slowing;
  - he is knocked sideways to the leading leg's side: front 1.2 m over 5 frames, back 1.0 m over 20;
  - the victim's skates are 20 cm (front) and 30 cm (back) ahead of the leading skate at contact, i.e. at his toe tips or his heels.

| Number | Asked | Front, cel / clay | Back, cel / clay |
|---|---|---|---|
| Frames with the bodies overlapping, frames 0 to 36 (after contact) | 0 | 0 / 0 (0 / 0) | 0 / 0 (0 / 0) |
| Hips apart when both lie (frame 36) | 80 cm or more | 276 / 279 cm | 218 / 211 cm |
| Same placement, existing `Trip` from its frame 2, front: frames with overlap | (about 16) | 13 (11 to 23) / 11 | |

With the back placement at 20 cm ahead (not 30), frames 5 to 8 had 1 to 7 vertices touching before contact, up to 4.7 cm deep.

What I saw:
- **From his front:** the orange carrier skates in. At 10 his skates go back and he pitches forward and to the side, flying past the blue slider, and is flat on his chest by 17 to 20, well clear. It reads as being upended over the slider.
- **From behind:** his feet fly up forward and he sits down hard ahead and to the side of the slider by 20.
- Both read as a tackle on the ice, not as two players falling over each other.
- The early front frames are cropped at the top of the tile. The camera is set for the landing.

### Evolution 2: `Slide_Tackle_Right` clear

- The stick is lifted over the trailing leg on the way back from the sweep: an extra key at 12, grip 48 cm up.
- The leading skate is 10 cm further out beside the trunk, with a 40-degree roll, and the trunk rolls 10 degrees more onto the hip. The same applies to `_Left`.

| Number | Asked | Slide_Tackle_Left cel / clay | Slide_Tackle_Right cel / clay |
|---|---|---|---|
| Shaft through a leg, the trunk or the head | 0 | 0 / 0 | **0 / 0** (was 2 / 1) |
| Leading skate seen from the game camera at frame 10 (vis_dive.py) | 60 % on clay | 71 % / **64 %** | 70 % / **63 %** (was 52 %) |
| Blade end's height at contact | 3 cm or less | 1.7 / 1.9 cm | 1.7 / 2.0 cm |
| Blade end's largest one-frame move | 40 cm | 36.4 / 34.7 | 32.7 / 32.1 |
| Joins, ground, pops | exact; 0 or above; 0 | yes; body 0.6 / 0.66, stick 0.47 / 0.01; 0 | yes; 0.6 / 0.66, 0.76 / 0.44; 0 |

### Evolution 3: the act in the big parts (Dive_Block_Left / _Right, now 40 frames)

What changed:
- **12:** the free glove is thrown up (70 cm on the left side) and the top leg is down: the wind-up.
- **15:**
  - the stick slides 26 cm toward the shot (forward) and 8 cm out;
  - the glove is slapped down on the ice in front of the chest (82 cm ahead of the root on the left side);
  - the top leg is kicked up and held to 20, its ankle 44 cm over the lying pose's on cel and 40.4 on clay (was 26 / 24);
  - the trunk opens to the shot (pelvis pitched 32 degrees, was 50).
- **The settle** runs 20 to 37, two frames longer, so the larger stick turn stays inside the 40 cm line. The lying pose is held from 37.

| Number (game camera, the figure 110 px; maskdiff.js: figure pixels that change between frames 12 and 18) | Second loop | Third loop | Ratio (asked 3) |
|---|---|---|---|
| Dive_Block_Left p2_cel | 728 | 1802 | **2.5** |
| Dive_Block_Right p2_cel | 816 | 1755 | **2.2** |
| Dive_Block_Left p3_clay | 563 | 1553 | **2.8** |
| Dive_Block_Right p3_clay | 526 | 1823 | **3.5** |
| Knockdown_Left, same frames, for comparison | | 259 / 357 | |

Kept:
- full cover at 10 (width at cover 328 / 309 cm on the left side);
- blade end's largest one-frame move 39.2 / 38.1 cm;
- shaft through 0 of 41 frames, both styles;
- pops 0, joins exact;
- nothing under the ice. A first build had the trunk 0.26 cm under at 17 to 19 on cel, fixed by raising the shuffle key 1.2 cm.

**Would a stranger say "blocking" or "knocked down"?** From `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-042658-B-set-dive-loop3-act-slide.png`, rows 1 and 2 of each style:
- Frames 15 to 24 show a figure flat on its side with one leg raised high, the stick reaching out ahead and a glove planted forward. `Knockdown_Left` in the same frames lies still, legs together, stick tucked.
- A stranger would tell them apart and would say the block player is doing something on purpose.
- Whether he would say "blocking" rather than "sprawling" I cannot claim: the raised leg reads as a leg in the air more than as a wall.
- In motion the frame 12 to 15 slap and kick will be clearer than in a still. That is not verified.

## 3. Not verified

- Nothing has been seen in the engine.
- Stills only.
- The victims' travel, the sideways knock and the skates' placement at contact are invented for the strip.
- The overlap count is a vertex sample (every second vertex) against the nearest face; a thin part passing between samples could be missed.
- I did not look at the victims' takes alone, from the side or close.
- Not rendered: the act from the shooter's side, `Dive_Block_Front` at game scale from the side.
- The renders are Cycles with simple materials.

## 4. For the engine lane

| Take | Frames | Starts on | Ends on (held from) | Key |
|---|---|---|---|---|
| Fall_Legs_Front | 26 | = Fall_Forward frame 0 (play from 2) | Lie_Loop_Front frame 0 (19) | contact at its frame 2 |
| Fall_Legs_Back | 26 | = Fall frame 0 (play from 2) | Lie_Loop frame 0 (19) | contact at its frame 2 |
| Dive_Block_Left / _Right | 40 | the falls' start | Lie_Loop_Side_* frame 0 (37) | cover 10, act 12-20 (slap and kick 15), settle 20-37 |

The other takes are as in part 2 (READY.txt).
- **Trigger:** the attacker's `Slide_Tackle_*` frame 10 starts the victim's `Fall_Legs_Front` (slider coming at his front) or `Fall_Legs_Back` (from behind) at its frame 2.
- **Must never:** move the root; play the victim's fall without the game moving him clear (section 5).

## 5. For the other device

When the slide tackle lands (attacker frame 10):
- keep the carrier's own velocity (he slides on);
- add **a sideways impulse of about 1.0 to 1.2 m over 5 to 20 frames**, to the side of the attacker's leading leg (the attacker's left for `Slide_Tackle_Left`), so the two bodies do not end on one line;
- choose `Fall_Legs_Front` when the attacker comes at the carrier's front half, and `Fall_Legs_Back` otherwise.

Without the impulse the victim lands on the slider (10 to 21 frames of overlap after contact in my count).

## 6. Decisions for the operator

1. **The sideways knock on a slide tackle.** It is a game rule, not an animation. Accept it, or accept two players piled up.
2. **The act's size.** The leg kick and glove slap are as big as the toy allows without breaking the stick line; the raised leg may read as comic. Keep it, or go back to the second loop's quieter act.

## 7. The next three evolutions

1. **The act at 3 times everywhere.** A body shuffle of 10 cm along the line at 15 (the whole body sliding toward the shot) and the top arm's glove slap with the elbow lifted. Proving number: maskdiff ratio 3 or more in all four cases (today 2.2 to 3.5).
2. **The gate's jitter column to zero in the new takes.** `Fall_Legs_Back`'s hand (21 cm), the dives' throw forearm (13 to 17 cm), the slides' calves. Proving number: jitter runs 0 in all 20 rows (today 10).
3. **The victims seen alone and in motion.** A side and close strip of both falls, and the head's lowest point against the forearms and hips at every landing frame. Proving number: on every frame from contact to the lying pose, the helmet above the forearms or the hips (measured, today only at the landing frames).

Files: the lab is `C:\Users\kazda\jam\sets-lab\dive\`. This loop added `pair3.py`, `pairs3.sh`, `maskdiff.js`, `gate3\` and the log `logs\final3b.txt`; the takes are still in `dive_block.py`.
