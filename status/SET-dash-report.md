# SET-DASH: the speed boost (device B, night of 14 Nov, 01:38 to 02:20)

Lane: Blender, CPU only. Lab: `C:\Users\kazda\jam\sets-lab\dash\`. Built on the figure lab's last delivered state (`build_clips_pass29.py`, export29), copied; nothing written in `skater-lab\skf\`.

## 1. Done / not done

Both evolutions are built, checked and looked at, in both styles: seven new takes (`Dash_Start`, `Dash_Loop`, `Dash_End`, `Dash_Bump`, `Dash_Skid`, `Dash_Wipeout`, `Boost_Take`), one FBX per take per style in `C:\Users\kazda\jam\sets-lab\dash\export\p2_cel\` and `...\p3_clay\` (plus `Dash_Loop_loopcut`), `READY.txt` and `JOINS.md` beside them. All 15 joins are exact (0.00004 cm or less, 0 deg). No take goes over the lab's pop limits. The top hand is on the stick in every frame. The reference takes rebuilt in my dev file are identical to export29 (0 cm, 0 deg, all 8 takes, both styles), so no existing take changed. Not done or not met: the stick's swing between "in front" and "trailing" moves the blade end 42 to 56 cm in one frame, over the lab's 40 cm get-up line. On p2_cel the shaft passes more than 3 cm into a leg for 1 to 3 frames in four takes. On clay the support skate rides 1.3 cm up for 4 frames of the loop, as the clay `Stride_Sprint` already does. Evolution 3 (effects anchors) is given as a table of numbers only. Nothing has been seen in the engine.

How the takes are made: `build_dash.py` = the lab's pass-29 builder + `dash_a.py` (key poses, before the lab's adapt step) + `dash_b.py` (`Boost_Take` on the adapted poses, and an even-pace stick sweep) + `dash_bake.py` (the bake copies the solved frame of the take each new take starts from or ends on into its first or last frame, as the lab does for `Burst`) + `dash_tail.py` (it bakes only the 8 reference takes and the 7 new ones, then exports a dev file and one file per take). Run `bash assemble.sh; bash run_dash.sh; bash check_dash.sh "meas join check root"; bash clear_dash.sh; bash sheet_dash.sh <loop|evo1|evo2> <style>`.

## 2. Evolutions

### Evolution 1: the dash

What changed: four new takes.
- `Dash_Start` (10 frames): from the ready stance, a deep crouch at frame 3 with both hands on the stick, the blade lifted and drawn in. The drive follows: the right leg pushes out and back, the free hand lets go, and the stick swings out round the right side into the trail. It ends on `Dash_Loop` frame 0.
- `Dash_Loop` (24 frames, a loop): two pushes, right at 0 and left at 12, recovering at 6 and 18. The body is folded like a speed skater's with the head up. The stick is held low in the top hand alone and trails back past the right skate, the blade 36 to 42 cm off the ice. The free arm swings across the front on the right push and back high on the left push.
- `Dash_End` (12 frames): the trunk rises, the stick swings back to the front, and the free hand returns to the shaft by frame 10. It ends on `Stride` frame 30.
- `Dash_Bump` (9 frames): the trunk rises and turns so the left shoulder leads and is driven through (contact frame 5, pelvis yaw 40 deg, the head counter-turned to keep looking ahead). It returns to `Dash_Loop` frame 0.

How `Dash_Loop` differs from `Stride_Sprint` (measured on the FBX, `meas21.py`; p2_cel / p3_clay):

| | Stride | Stride_Sprint | Dash_Loop | Dash against Sprint |
|---|---|---|---|---|
| Loop length, pushes | 60 frames, 2 | 60, 2 | **24, 2** | a push every 12 frames instead of 30 (2.5 times the cadence) |
| Trunk lean from vertical, average | 32.8 / 32.9 deg | 50.3 / 50.3 | **64.5 / 64.5** | +14.2 deg (+31.7 against Stride) |
| Hips, average height | 77.7 / 73.6 cm | 74.7 / 70.8 | **68.5 / 65.1** | -6.2 / -5.7 cm (-9.2 / -8.5 against Stride) |
| Top of the figure, average | 155.7 / 147.0 cm | 143.6 / 135.5 | **127.9 / 120.8** | -15.7 / -14.7 cm: the silhouette is a low wedge |
| Push skate target (key) | | 28 cm back, 41 out | 24 back, 36 out | 4 cm shorter, 5 cm narrower |
| Blade end, average | 108 cm ahead, on the ice | 148 ahead, on the ice | **117 / 111 behind, 38 / 36 cm up** | the stick trails instead of leading |
| Free hand fore-and-aft travel | 8 / 11 cm | 38 / 35 | 44 / 40 | +6 / +5 cm |

**Proving numbers:**
- Trunk and hips: table above.
- Skate contacts (0.8 cm line, `low_skate_l/r`), p2_cel: right skate leaves the ice at 4 and lands at 11, left skate leaves at 16 and lands at 23. The supporting skate stays at 0.1 to 0.4 cm and the skate in the air climbs to 5.1 cm, so nothing skims. p3_clay: the skates in the air are the same (up to 5.6 cm). The supporting skate rides 0.9 to 1.3 cm for frames 7 to 10 and 19 to 22, against 1.1 to 1.5 cm for frames 4 to 11 before I moved the clay support skate under the knee. The clay `Stride_Sprint` has the same kind of bump (0.9 to 1.3 cm). On clay the pushing skate leaves the ice 3 frames after the push key, at 0.7 to 1.7 cm, as in the clay sprint. Under the ice: nothing (skates 0.00 / 0.13 cm at the lowest).
- Joins: `Glide`@0 > `Dash_Start`@0 > `Dash_Loop`@0, `Dash_Loop` wrap, `Dash_Loop` > `Dash_End`@0, `Dash_End`@12 > `Stride`@30, and `Dash_Bump` both ends: 0.00002 cm at most, 0 deg, both styles (`export\JOINS.md`).
- Top hand on the stick: 0 cm every frame (`rh_cm`). Pops: 0 of 7 new takes over the lab's limits (60 deg from frame 3, 120 before), both styles.
- The stick (known gap): the blade end's largest one-frame move is 45.7 / 43.6 cm in `Dash_Start` and 43.5 / 42.1 in `Dash_End` (the stick turns 170 deg about the hand at an even pace over 9 to 10 frames). It is 2.2 cm in `Dash_Loop` and 21.8 / 22.3 in `Dash_Bump`. The lab's line for get-ups is 40 cm.
- Shaft against the body (the lab's `clear29.py`, frames with 2 or more samples more than 3 cm inside): `Dash_Loop` 0 frames, both styles. On p2_cel: `Dash_Start` frames 7 and 8, `Dash_End` frames 1, 9 and 12, `Dash_Bump` frames 1 and 2 (leg). On p3_clay: 0 everywhere. For scale, `Stride` itself scores 12 frames (p2_cel) and 6 (p3_clay) on the same measure.

Still: `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-021549-B-set-dash-loop.png` (Stride, Sprint, Dash_Loop) and `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-021549-B-set-dash-evo1.png`.

What I saw on the sheets (Cycles CPU renders of the lab FBX, game camera at about 110 px and a closer side view, both styles):
- From the game camera, `Dash_Loop` is clearly lower and wider-shouldered than `Stride_Sprint`. The helmet sits low over the back, and the stick reads as a line trailing toward the camera on the right. The free arm's swing shows.
- From the side the trunk is near level, the face looks forward, and the stick lies behind at knee height.
- `Dash_Start` reads as a crouch and then an oar-like swing of the stick out to the right side (frames 4 to 7) into the trail.
- `Dash_End` reads as the reverse swing while the figure rises into the upright stride.
- `Dash_Bump` is the weakest read: at game size it is a rise and a quarter turn of the shoulders, modest. It was made bigger once (pelvis yaw 26 to 40 deg) and only looked at again in the final sheet.

### Evolution 2: the cost and the end

What changed: three new takes.
- `Dash_Skid` (20 frames): the hips and skates turn 86 deg across the travel by frame 4. The body leans back hard against the slide, the skates sit wide apart, the free arm is thrown out ahead for balance and the stick keeps trailing. Frames 6 to 10 are identical (the hold). From frame 10 the stick swings back to the front and the figure rises, ending on `Glide` frame 0.
- `Dash_Wipeout` (24 frames): a skate catches and the other kicks up behind (3), a headlong superman dive with the arms ahead (7), belly on the ice (13), and a bounce with both skates kicking up (16). From frame 20 to 24 it holds the face-down lying pose, exactly `Fall_Forward`'s last frame.
- `Boost_Take` (11 frames): over `Stride` frames 0 to 11, the free (left) hand leaves the shaft by 2, punches up over the helmet at 5, and is back on the shaft at 11. Legs, pelvis and the stick on the ice are `Stride`'s.

**Proving numbers** (p2_cel / p3_clay):
- `Dash_Skid` against `Stop_Hard`, at the lowest frame: hips 56.0 / 53.7 cm against 61.0 / 58.3 (5 cm lower). The neck is 16.5 / 16.1 cm behind the pelvis along the travel, against 9.0 / 8.7 (lean back almost twice as big). The skates are 47.1 / 45.2 cm apart against 28.6 / 28.5. Pelvis yaw is -86 against -82.
- Hold frames 6 to 10 identical: largest joint difference 0.000 cm, both styles.
- Both skates on the ice through the skid on p2_cel (0.0 to 0.3 cm). On p3_clay they sit 0.8 to 1.4 cm up through the hold, as the clay `Stop_Hard` does (1.1 to 1.6 cm).
- `Dash_Wipeout`@24 against `Fall_Forward`@26, `Lie_Loop_Front`@0 and `GetUp_Front`@0: 0.00004 cm or less, 0 deg. Lowest body 0.41 / 0.56 cm, the same as `Fall_Forward`. Skates and stick never under the ice.
- `Boost_Take` against `Stride` frames 0 to 11: skates, knees and pelvis 0.000 cm apart in every frame. The free hand rises up to 83.6 / 80.9 cm. Joins to `Stride`@0 and @11: 0 cm, 0 deg.
- Pops: 0 takes over the limits.
- Blade end's largest one-frame move: `Dash_Skid` 55.8 / 53.7 cm (the swing back to the front over frames 10 to 19), `Dash_Wipeout` 53.7 / 50.9 (the stick flung in the dive, frame 9 to 10), `Boost_Take` 0.3.
- Shaft in a leg: `Dash_Skid` 1 frame (18) on p2_cel; 0 on clay; `Dash_Wipeout` and `Boost_Take` 0.

Still: `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-021549-B-set-dash-evo2.png` (Stop_Hard above the three).

What I saw:
- The skid is visibly bigger than `Stop_Hard` from both views: deeper, leaning back, wide skates, the arm out ahead. From the game camera that arm shows above the helmet, so it could be misread as a wave.
- The wipeout reads as a comic belly-flop: the dive is flat and long, and the skates come up on the bounce. Nothing reads as hurt.
- `Boost_Take` reads at game size as a fist punched up over the helmet while the legs keep striding.

### Evolution 3 (numbers only, not looked at): anchors for the effects lane

Ankle joints in cm, armature space (X forward, Y left, Z up, root at the origin; the blade's bottom edge is 13 to 14 cm under the ankle). Last column: the lowest skate point, L and R, p2_cel / p3_clay.

| Take | Frame | Left ankle p2 / p3 | Right ankle p2 / p3 | Hips | Skate low L R | Use |
|---|---|---|---|---|---|---|
| Dash_Start | 3 | 9, 23, 14 / 9, 25, 15 | 7, -23, 15 / 7, -25, 16 | 0, 0, 66 / 63 | 0.1 0.2 / 0.3 0.7 | both skates: launch spray starts |
| Dash_Start | 6 | 16, 5, 14 / 15, 8, 15 | -7, -31, 17 / -6, -32, 17 | 0, 0, 67 / 63 | 0.1 0.3 / 0.3 1.2 | right skate drives |
| Dash_Start | 10 | 13, 4, 13 / 12, 7, 14 | -19, -40, 15 / -17, -40, 15 | 0, 0, 68 / 65 | 0.1 0.3 / 0.1 0.7 | = Dash_Loop 0 |
| Dash_Loop | 0 | 13, 4, 13 / 12, 7, 14 | -19, -40, 15 / -17, -40, 15 | 0, 0, 68 / 65 | 0.1 0.3 / 0.1 0.7 | right push: spray at the right skate frames 0-3 |
| Dash_Loop | 2 | 14, 8, 13 / 12, 10, 14 | -11, -33, 18 / -11, -34, 19 | 0, 0, 68 / 65 | 0.1 0.3 / 0.2 1.4 | |
| Dash_Loop | 4 | 7, 14, 16 / 12, 15, 14 | 2, -22, 19 / 2, -24, 20 | 0, 0, 69 / 65 | 0.2 0.9 / 0.2 1.7 | right skate leaves the ice (spray stops) |
| Dash_Loop | 12 | -19, 40, 15 / -17, 40, 15 | 13, -4, 13 / 12, -7, 14 | 0, 0, 68 / 65 | 0.3 0.1 / 0.7 0.2 | left push: spray at the left skate frames 12-15 |
| Dash_Loop | 14 | -11, 33, 18 / -11, 34, 19 | 14, -8, 13 / 12, -10, 14 | 0, 0, 68 / 65 | 0.3 0.1 / 1.4 0.2 | |
| Dash_Loop | 16 | 2, 22, 19 / 2, 24, 20 | 7, -14, 16 / 12, -15, 14 | 0, 0, 69 / 65 | 0.9 0.2 / 1.7 0.2 | left skate leaves the ice |
| Dash_Skid | 2 | 29, 4, 13 / 27, 7, 14 | -10, -26, 15 / -9, -28, 15 | 0, 0, 63 / 60 | 0.3 0.1 / 0.3 0.3 | skid spray starts (both skates) |
| Dash_Skid | 4-10 | 44, -2, 17 / 41, 1, 19 | -2, -12, 17 / -1, -15, 16 | 0, 0, 56-58 / 54-56 | 0.3 0.3 / 1.4 0.8 | the full skid: the biggest spray, along -Y of the turned skates |
| Dash_Skid | 13 | 38, 3, 13 / 35, 5, 15 | 2, -15, 16 / 2, -18, 14 | 0, 0, 64 / 61 | 0.2 0.2 / 0.6 0.3 | spray fades by 14 |

A streak, if any: from the hips (0, 0, 65 to 69 cm) for the whole of `Dash_Loop`, and from the trailing blade end (about 115 cm behind, 40 cm up, 0 to 50 cm right).

## 3. Not verified

- Nothing in the engine: no import, no blend, no play rate, no layered blend of `Boost_Take`.
- Motion was not watched: everything is from stills every 1 to 5 frames and from numbers. Whether the 24-frame loop looks frantic or fast in motion, and whether the stick's 170-deg swing in `Dash_Start` and `Dash_End` reads as a launch or as a flail, is unjudged.
- Renders are Cycles CPU stand-ins without the game's materials or outline; the clay look is not the engine's.
- The blade-end move over 40 cm and the leg-shaft frames listed above.
- `Dash_Bump` was looked at once, at the smaller turn; the final larger version only in the last sheet.
- The cross-fades the engine table assumes (from any stride frame into `Dash_Start`, from any loop frame into `Dash_Bump`, `Dash_Skid`, `Dash_Wipeout`) were not built or measured.

## 4. For the engine lane

Files: `C:\Users\kazda\jam\sets-lab\dash\export\<style>\SK_Skater_<style>_<Take>.fbx`, take name `Armature|<Take>`, same skeleton as export29. Import into `BP_SkaterVisual_TB1` (p2_cel) and `_TB1_P3` (p3_clay). Root keys are identity; travel is the game's.

| Take | Frames at 60 fps | Starts on | Ends on | Trigger | Timing | Must never |
|---|---|---|---|---|---|---|
| Dash_Start | 10 | Glide frame 0 (from a stride: cross-fade 4 frames into its frame 0) | Dash_Loop frame 0 | dash begins | plays once; with Dash_Loop it may be rate-scaled to the dash length | be skipped into Dash_Loop without a blend |
| Dash_Loop | 24, loop (frame 24 = 0; `_loopcut` without it) | Dash_Start's end | its own frame 0 | while the dash or boost lasts | for one dash of F frames: play Start + N loop cycles at rate (10 + 24N) / F, N = max(1, round((F - 10) / 24)); a 30-frame dash = Start + 1 cycle at 1.13 | play under 0.7 or over 1.4 rate (the pushes stop reading) |
| Dash_End | 12 | Dash_Loop frame 0 | Stride frame 30: start Stride at 30 | dash ends normally | leave the loop at its frame 0; if the dash ends mid-cycle, either wait (at most 23 frames) or cross-fade 6 frames | start Stride at frame 0 after it (the feet would swap) |
| Dash_Bump | 9 | Dash_Loop frame 0 (cross-fade 3 frames from any loop frame) | Dash_Loop frame 0 | the dash body-bumps someone and keeps going; contact frame 5 | fixed length; resume Dash_Loop at 0 | be used when the dasher loses (that is Dash_Wipeout); the victim uses the existing hit reactions |
| Dash_Skid | 20; hold 6-10 identical | Dash_Loop frame 0 (cross-fade 3 to 4 frames) | Glide frame 0 | boost runs out or the player brakes during it | play 0-6, hold any frame of 6-10 for as long as the game wants (extra = skid frames - 20), then 10-20 | turn left: it turns right only (a mirror take is easy, not built) |
| Dash_Wipeout | 24; holds the lying pose 20-24 | Dash_Loop frame 0 (cross-fade 3 frames) | face-down lying pose (= Fall_Forward last frame) | the dash hits a wall or body and loses | then exactly as after Fall_Forward: Lie_Loop_Front with the lab's start-frame rule (twenty-eighth pass table, L counted from Wipeout frame 20), then GetUp_Front / _Quick_Front / _Snap_Front / _Late_Front / _LateLong_Front | be followed by any other lying loop or get-up |
| Boost_Take | 11 | Stride frame 0 (full body); any frame if layered | Stride frame 11 (full body) | boost pickup collected | full body: only when Stride is at frame 0, continue Stride at 11. Layered: blend per bone from spine_01 up (spine_01, spine_02, spine_03, both clavicles, both arms, hands, neck, head, stick_socket); legs and pelvis stay the base's | be layered over a fall, a get-up or Dash_Loop (its right arm holds the stick on the ice) |

## 5. For the other device (the lead sends this)

- `PlayDash(int32 Frames)`, on the first frame of the dash: the figure plays `Dash_Start` + `Dash_Loop`, scaled to `Frames` (rule above).
- `SetBoost(bool bOn, float SpeedScale)` as a level for pickups and longer boosts. `bOn` true: Start then Loop for as long as it is on. False: `Dash_End`. `SpeedScale` (1.0 = top speed) sets the loop's play rate between 0.9 and 1.3.
- `EndDash(uint8 Reason, float DirX, float DirZ)`: Reason 0 = timed out (`Dash_End`), 1 = braked out or out of boost (`Dash_Skid`, with `SetBraking` frames as the hold), 2 = bumped through and kept going (`Dash_Bump`, the victim gets `PlayHitReaction(DirX, DirZ)`), 3 = hit a wall or body and lost (`Dash_Wipeout`, then `SetKnockedDown(true)` with its length as today). Dir is the contact direction in the dasher's frame, for the victim and for a future mirrored skid.
- `PlayBoostPickup()`: `Boost_Take`.
- With only what the game sends today: speed above **1.25 x the normal top skating speed for 6 frames or more** switches the stride to `Dash_Loop`, entered through `Dash_Start`. Below 1.1 x for 6 frames, `Dash_End` plays. 1.25 sits well below the dash's 1.6 x, so the switch happens early in the burst. It also sits above ordinary sprint speed, so `Stride_Sprint` stays the sprint. The two thresholds keep it from flickering. The call is better: the launch should play on the press, not 6 frames into the burst. A speed read cannot tell a timeout from a brake from a lost collision, so `Dash_Skid`, `Dash_Bump` and `Dash_Wipeout` need the call. And boost pickups that are not dashes should not look like dashes.

## 6. Decisions for the operator

1. **The trailing stick.** The brief asked for the stick trailing in one hand. It reads well at game size, but every way in and out swings the stick 170 deg about the hand, moving the blade 42 to 56 cm in a frame. The alternative is the stick held low in front, off the ice: smaller swings, a less "fast" silhouette. I kept trailing.
2. Whether the wipeout's comic belly-flop (dive, bounce, skates kicking up) is the right tone for losing a dash, or should be shorter and plainer.
3. `Dash_Skid` turns right only: is one direction enough, or is a mirrored `Dash_Skid_Left` wanted?

## 7. The next three evolutions, each with a proving number

1. **Watched in motion and the stick swing tamed:** 60 fps clips of Start > Loop > End and Loop > Skid and Wipeout, both styles, judged once. Then the swing spread with the stick drawn in close to the body halfway through, so the radius shrinks. **Proving numbers:** blade end's largest one-frame move 40 cm or less in all seven takes (today 56); clips watched 4 of 4 (today 0).
2. **Clean shaft and clean clay skates:** the four p2_cel leg frames cleared with the lab's capsule correction (`clear=True` in `path27`). The clay support skate's 1.3 cm ride removed by a small foot pitch key on the recover frames. **Proving numbers:** frames with the shaft more than 3 cm inside a leg: 0 (today 8 over four takes, p2_cel); clay supporting skate at or under 0.8 cm in every support frame of `Dash_Loop` (today 1.3 cm for 8 frames).
3. **Mirrors and the bump with a hit side:** `Dash_Skid_Left`, and `Dash_Bump_Right` (right shoulder, stick side). The lab's `mirror_all` gives them cheaply. **Proving numbers:** joins 0.0005 cm / 0 deg for the 4 new pairs; pops 0; the mirror's pelvis yaw +86 against -86.

Files: lab `C:\Users\kazda\jam\sets-lab\dash\` (`build_dash.py` and its parts, `run_dash.sh`, `check_dash.sh`, `clear_dash.sh`, `sheet_dash.sh`, `render_dash.py`, `join_dash.py`, `sum_dash.js`, `blade.js`, `skid.js`; numbers in `work\<style>\` `meas.json`, `joins.json`, `check_fbx.json`, `clear.json`; `work\checks.txt`).
