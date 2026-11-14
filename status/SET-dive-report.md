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
