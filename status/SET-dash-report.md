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

---

# Part 2: the second loop (02:21 to 03:12)

## 1. Done / not done

All three evolutions are done, plus the lead's silhouette addition, in both styles. `C:\Users\kazda\jam\sets-lab\dash\export\` is replaced in place: 10 takes per style (`READY.txt` last), `JOINS.md` and `GATE.md`. The rebuild is on the lab's new `build_clips_pass30.py`; my 8 reference takes are identical to export30 (0 cm, 0 deg).

Met:
- The stick's swing: the blade end moves at most 38.3 cm in one frame in all takes (target 40; part 1: 56).
- The shaft: 0 frames more than 3 cm inside a leg, the trunk or the head, all takes, both styles (part 1: 8 on p2_cel).
- The clay supporting skate: at most 0.29 cm in `Dash_Loop` and 0.64 cm through `Dash_Skid`'s hold (target 0.8; part 1: 1.3 and 1.4).
- The top hand stays on the stick. Joins are exact (21 pairs). No take pops. The figure lane's `gate.sh` passes all 20 take-and-style rows.

New takes and names:
- `Dash_Skid_Left`, the mirror of `Dash_Skid`.
- `Dash_Bump_Right`, which leads with the right (stick) shoulder.
- `Dash_Bump_Left`: today's bump leads with the **left** shoulder, so it now has a side name. `Dash_Bump` is kept with identical keys so the import already made does not break.

`Dash_Wipeout` has no side bias. Every take was rendered and looked at frame by frame, and two things only visible that way were fixed. The dash loop's silhouette was reworked after the critic's note: from the game camera it now differs from the sprint by 2.65 to 2.83 times what the sprint differs from the stride, and by 2.19 to 2.36 times with the stick hidden.

Changed from part 1, which the engine lane must know:
- `Dash_End` now ends on `Stride` frame **34**, not 30. `Stride`'s own frame 30 has the shaft in the leg and the trunk on p2_cel.
- `Dash_Skid`'s hold is frames **6-9**, not 6-10.

## 2. Evolutions

### Evolution 1: the stick's swing tamed, the shaft out of the legs, the clay skates down

What changed:
- **The trail.** The trailing stick sits 20 to 25 deg nearer the side (yaw -130 instead of -155) and nearly level (tilt 88). The top hand is held 10 cm further back.
- **The swing in and out of the trail.** It is now one even sweep over 10 frames in `Dash_Start` (0-10), `Dash_End` (0-10) and `Dash_Skid` (9-20); in part 1 it took 9 frames. During the sweep the hand rises 20 cm and the blade travels 5 cm over the ice (10 cm in the skid's return). `Dash_Wipeout`'s flung stick is swept over frames 4-13.
- **Clay skates.** The clay support skate sits under the knee in the recover key (x +10 cm). The clay skid stands 3 cm higher with its skates 6 cm further toward the way it faces.
- **`Dash_End`** ends on `Stride` 34.

Proving numbers, p2_cel / p3_clay:

| Take | Blade end's largest one-frame move cm (part 1) | Shaft frames inside leg / trunk / head (part 1, p2_cel) |
|---|---|---|
| Dash_Start | 38.3 / 36.8 (45.7 / 43.6) | 0 / 0 (2) |
| Dash_Loop | 2.3 / 2.3 (2.2) | 0 / 0 (0) |
| Dash_End | 37.7 / 36.3 (43.5 / 42.1) | 0 / 0 (3) |
| Dash_Bump(_Left) | 21.8 / 22.3 | 0 / 0 (2) |
| Dash_Bump_Right | 10.8 / 11.3 | 0 / 0 |
| Dash_Skid | 38.2 / 37.0 (55.8 / 53.7) | 0 / 0 (1) |
| Dash_Skid_Left | 37.6 / 35.8 | 0 / 0 |
| Dash_Wipeout | 37.0 / 35.1 (53.7 / 50.9) | 0 / 0 (0) |
| Boost_Take | 0.3 / 0.2 | 0 / 0 |

For comparison, `Stride` itself scores 12 shaft frames (p2_cel) and 6 (p3_clay) on the same measure (`clear29.py`).

Other proving numbers:
- **Clay supporting skate in `Dash_Loop`:** at most 0.29 cm on every frame (p2_cel 0.42).
- **Clay `Dash_Skid` hold (frames 4 to 11):** 0.62 to 0.64 cm.
- **Top hand on the stick:** 0 cm in every frame.
- **Joins:** 21 pairs, at most 0.00004 cm, 0 deg.
- **Gate (export30 base):** PASS, 20 of 20; joins to `Glide`, `Stride`@0/11/34 and `GetUp_Front`@0 all at 0.0000 cm (one at 0.0010 cm), 0 deg.
- **Pops:** 0 of 18 played takes over the limits.

Still: `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-030634-B-set-dash-swings.png` (Start, End and Skid at every frame).

What I saw: the stick still swings round the right side, but it sweeps evenly and stays low and near the figure. Seen at every frame, there is no frame where it jumps.

### Evolution 2: mirrors and missing sides

- **`Dash_Bump_Left`** (the old `Dash_Bump`) leads with the left shoulder: pelvis yaw -40 at contact frame 5.
- **`Dash_Bump_Right`** leads with the right (stick) shoulder: pelvis yaw +40 at 5. Its body is mirrored. The stick stays trailing in the right hand and the free arm swings out to the left for balance.
- **`Dash_Skid_Left`** turns left, pelvis +86 deg (`Dash_Skid`: -86). Same hips (56.0 / 56.4 cm), same hold, the stick trailing behind the turned body.

Joins (both styles): the four new pairs (`Dash_Loop` > `Dash_Bump_Left`/`Dash_Bump_Right` > `Dash_Loop`; `Dash_Loop` > `Dash_Skid_Left` > `Glide`) are at most 0.00002 cm, 0 deg.

`Dash_Wipeout` side bias, measured on the FBX (`bias.js`):
- Pelvis and head lateral position: 0 cm from frame 3 to the end.
- Pelvis yaw: 0 deg from frame 3 on. The 9 deg and 10 cm at frame 0 are the loop's right-push lean.
- The body's lateral centre stays within ±3 cm and ends at +2 / +1 cm, exactly as `Fall_Forward` ends.

**It does not veer.**

Still: `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-030634-B-set-dash-mirrors.png`. Seen at every frame: both mirrors read as the mirror of their twin. The right bump reads as a drive of the stick-side shoulder.

### Evolution 3: seen frame by frame, the silhouette, and the effect anchors

**The silhouette (the lead's addition).** I rendered silhouette masks from the game camera, at about 110 px, of `Stride`, `Stride_Sprint` and the loop at 8 matching phases (`sil.sh`). I measured the bounding box and the pixels that differ (XOR) between sprint and stride, and between dash and sprint, at the same phase.

Candidates tried on p2_cel (ratio of dash-vs-sprint to sprint-vs-stride; body only = stick hidden):

| Candidate | With stick | Body only |
|---|---|---|
| Part 1's loop | 2.14 | 1.67 |
| A: free hand tucked on the lower back | 2.25 | 1.78 |
| B: A + pushes 42 cm back | 2.23 | 1.76 |
| D: A + pushes 56 cm out to the side | | 1.97 |
| E: D + trunk 8 deg lower, head kept up | | 2.17 |
| **H (chosen): E + the recovering skate kicked up 50 cm behind (speed skater)** | **2.83 / 2.65** (p2 / p3) | **2.36 / 2.19** |

The chosen loop against the references (p2_cel / p3_clay):

| | Stride | Stride_Sprint | Dash_Loop (loop 2) |
|---|---|---|---|
| Trunk lean from vertical | 32.8 deg | 50.3 | **72.5** (part 1: 64.5) |
| Hips | 77.7 / 73.6 cm | 74.7 / 70.8 | 68.5 / 65.1 |
| Top of the figure | 155.7 / 147.0 cm | 143.6 / 135.5 | **122.6 / 115.1** (21 cm under Sprint) |
| Body length front to back | 96 / 86 cm | 89 / 84 | **113 / 101** (+27 % / +20 % on Sprint) |
| Body width | 82 / 83 cm | 98 / 97 | 104 / 102 |
| Silhouette box from the game camera (px, w × h) | 54 × 102 | 63 × 99 | 131 × 95 with stick, 67 × 90 / 66 × 83 without |
| XOR against Sprint, 8 phases | 4507 / 4447 px | – | 12758 / 11772 px with stick, 10667 / 9776 without |

What the camera now sees:
- One arm gone (tucked on the back).
- A low squat back with the helmet sunk into it.
- Legs splayed wide, with a skate kicking up behind on every push.
- The stick as a long diagonal line trailing toward the camera.

**Honestly:** with the stick, I think a stranger would tell dash from sprint in one still: the trailing stick line and the missing arm are unlike anything else in the set. With the stick hidden, probably not reliably in one still. The body is squatter and wider, but at 110 px it is still "a blue skater crouching". In motion the kicked-up skate on every push and the 2.5-times cadence should carry it (not verified in motion).

The footprint grew by about a quarter in length, not by a third. The pushes reach 30 cm back and 56 cm out, wider than Sprint's 41 cm, against the brief's "shorter pushes" from part 1; the lead's silhouette note overrides that.

Still: `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-030634-B-set-dash-silhouette.png` (masks), `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-030634-B-set-dash-loop2.png` (renders against Stride and Sprint).

**Every frame looked at.** All 10 takes × 2 views × 2 styles, 592 frames (`strip.sh`, sheets in `C:\Users\kazda\jam\sets-lab\dash\sheets\strip\`). What every frame showed that every second frame had not:

1. `Boost_Take` frame 1: the free arm was flung out sideways for one frame before going up (the upper arm turned 104 deg in that frame). **Fixed:** the hand now leaves the shaft upward. The largest upper-arm step is 49.5 deg; seen again at every frame, it is clean.
2. `Dash_Wipeout`: after the belly landing (frame 10) nothing moved for 14 frames, and the bounce with the skates kicking up at 16 was invisible (14 cm). **Fixed:** the skates rise 17 cm, from 23 to 40 cm up (frames 13 to 16). Seen at every frame, the kick reads on p2_cel and faintly on clay.
3. Not changed, noted:
   - `Dash_Start` frames 4 to 7: from the side the stick disappears behind the body mid-sweep, so from the side the launch reads only as a crouch.
   - In every take, from the game camera, the trailing stick overlaps the right skate's line for frames 3 to 9 of each push. Measured clear of the leg, but at 110 px it can read as touching.
   - `Dash_Bump_Left` at game size is still a rise and a quarter turn: modest.

Still: `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-030634-B-set-dash-wipeout-boost.png`.

**Effect anchors (for the effects lane).** One row per spray, from `meas21.py` on the FBX (`anchors2.js`).
- A skate sprays while it is on the ice (lowest point under 0.8 cm) and moves against the root faster than 1.5 cm per frame; the spray leaves along that motion. In the skids' hold both skates throw snow ahead along the travel. The wipeout's chest and hips are on the ice from frame 13.
- Positions are the ankle joint, cm, root space (X forward = the travel, Y = the figure's left, Z up); the blade's bottom edge is 13 to 14 cm under it.
- Direction is a unit vector in root space. Frames are each take's own, 60 fps.

| Style | Take | Frames | Part | Position at the first frame (cm) | Position at the last frame (cm) | Spray direction (X, Y) | What |
|---|---|---|---|---|---|---|---|
| p2_cel | Dash_Start | 3-6 | left skate | 9, 23, 14 | 16, 5, 14 | 0.33, -0.94 | push: snow thrown out and back |
| p2_cel | Dash_Start | 3-9 | right skate | 7, -23, 15 | -27, -51, 13 | -0.78, -0.63 | push: snow thrown out and back |
| p2_cel | Dash_Loop | 2-12 | left skate | 15, 6, 14 | -26, 50, 13 | -0.68, 0.73 | push: snow thrown out and back |
| p2_cel | Dash_Loop | 14-23 | right skate | 15, -6, 14 | -25, -51, 13 | -0.66, -0.75 | push: snow thrown out and back |
| p2_cel | Dash_Skid | 0-1 | left skate | 13, 4, 13 | 18, 4, 13 | 1.00, 0.00 | push: snow thrown out and back |
| p2_cel | Dash_Skid | 2-13 | left skate | 29, 4, 13 | 36, 4, 13 | 1.00, 0.00 | skid: snow thrown ahead along the travel |
| p2_cel | Dash_Skid | 14-19 | left skate | 33, 7, 13 | 10, 19, 13 | -0.88, 0.48 | push: snow thrown out and back |
| p2_cel | Dash_Skid | 2-13 | right skate | -15, -32, 13 | 3, -16, 15 | 1.00, 0.00 | skid: snow thrown ahead along the travel |
| p2_cel | Dash_Skid | 16-17 | right skate | 5, -17, 14 | 7, -18, 13 | 0.88, -0.48 | push: snow thrown out and back |
| p2_cel | Dash_Skid_Left | 2-13 | left skate | 10, 9, 15 | 3, 16, 15 | 1.00, 0.00 | skid: snow thrown ahead along the travel |
| p2_cel | Dash_Skid_Left | 16-17 | left skate | 5, 17, 14 | 7, 18, 13 | 0.92, 0.38 | push: snow thrown out and back |
| p2_cel | Dash_Skid_Left | 0-1 | right skate | -26, -50, 13 | -13, -50, 14 | 0.91, 0.41 | push: snow thrown out and back |
| p2_cel | Dash_Skid_Left | 2-13 | right skate | 12, -27, 21 | 36, -4, 13 | 1.00, 0.00 | skid: snow thrown ahead along the travel |
| p2_cel | Dash_Skid_Left | 14-20 | right skate | 33, -7, 13 | 7, -20, 13 | -0.88, -0.47 | push: snow thrown out and back |
| p2_cel | Dash_Wipeout | 13-19 | chest (neck joint) | 72, 0, 18 | 72, 0, 19 | 1.00, 0.00 | belly on the ice: a puff thrown ahead along the travel |
| p2_cel | Dash_Wipeout | 13-19 | hips | 30, 0, 18 | 30, 0, 18 | 1.00, 0.00 | belly on the ice: a puff thrown ahead along the travel |
| p3_clay | Dash_Start | 3-6 | left skate | 9, 25, 15 | 15, 8, 15 | 0.33, -0.94 | push: snow thrown out and back |
| p3_clay | Dash_Start | 8-9 | right skate | -16, -44, 14 | -24, -49, 14 | -0.79, -0.61 | push: snow thrown out and back |
| p3_clay | Dash_Loop | 1-12 | left skate | 15, 7, 14 | -23, 48, 14 | -0.68, 0.73 | push: snow thrown out and back |
| p3_clay | Dash_Loop | 12-23 | right skate | 12, -7, 14 | -21, -50, 14 | -0.62, -0.78 | push: snow thrown out and back |
| p3_clay | Dash_Skid | 0-1 | left skate | 12, 7, 14 | 16, 5, 14 | 0.87, -0.49 | push: snow thrown out and back |
| p3_clay | Dash_Skid | 2-13 | left skate | 26, -1, 14 | 33, 4, 14 | 1.00, 0.00 | skid: snow thrown ahead along the travel |
| p3_clay | Dash_Skid | 14-19 | left skate | 31, 8, 14 | 9, 21, 14 | -0.83, 0.55 | push: snow thrown out and back |
| p3_clay | Dash_Skid | 2-13 | right skate | -15, -37, 14 | 2, -19, 14 | 1.00, 0.00 | skid: snow thrown ahead along the travel |
| p3_clay | Dash_Skid_Left | 2-13 | left skate | 6, 14, 14 | 2, 19, 14 | 1.00, 0.00 | skid: snow thrown ahead along the travel |
| p3_clay | Dash_Skid_Left | 0-1 | right skate | -23, -48, 14 | -11, -47, 15 | 0.84, 0.54 | push: snow thrown out and back |
| p3_clay | Dash_Skid_Left | 3-13 | right skate | 30, 1, 14 | 33, -4, 14 | 1.00, 0.00 | skid: snow thrown ahead along the travel |
| p3_clay | Dash_Skid_Left | 14-20 | right skate | 31, -8, 14 | 6, -22, 14 | -0.85, -0.53 | push: snow thrown out and back |
| p3_clay | Dash_Wipeout | 13-19 | chest (neck joint) | 71, 0, 19 | 71, 0, 20 | 1.00, 0.00 | belly on the ice: a puff thrown ahead along the travel |
| p3_clay | Dash_Wipeout | 13-19 | hips | 30, 0, 19 | 30, 0, 19 | 1.00, 0.00 | belly on the ice: a puff thrown ahead along the travel |

For a streak: from the hips (0, 0, 68 / 65 cm) for the whole of `Dash_Loop`, pointing straight back (-1, 0). Also from the trailing blade end, which sits at about (-115, -110, 19 to 42) cm on p2_cel, i.e. behind and right of the root.

## 3. Not verified

- Nothing in the engine: the import, the layered `Boost_Take`, the play-rate rule and the start-frame rules after `Dash_Wipeout` are untried.
- "Seen in motion" here means stills of every frame side by side, not playback at 60 fps. Whether the 24-frame loop with a skate kicking up every 12 frames looks fast or frantic in motion is unjudged.
- The silhouette numbers come from one camera (the lab's 110 px game-camera stand-in, orthographic, 35 deg down), not the engine's camera, lens or lighting.
- The clay skid's hips are now 2 cm under `Stop_Hard`'s (56.4 against 58.3; in part 1 they were 5 cm under); on p2_cel it is still 5 cm (56.0 against 61.0).
- The effect anchors are numbers only; no effect was built on them.
- `Dash_Bump` and `Dash_Bump_Left` are the same take under two names, both in the export.

## 4. For the engine lane

As part 1's table, with these changes:

| Take | Frames | Starts on | Ends on | Note |
|---|---|---|---|---|
| Dash_End | 12 | Dash_Loop 0 | **Stride frame 34**: continue Stride from 34 | was 30 |
| Dash_Skid | 20 | Dash_Loop 0 | Glide 0 | turns right; **hold any frame of 6-9** (was 6-10), then 9-20 |
| Dash_Skid_Left | 20 | Dash_Loop 0 | Glide 0 | turns left; hold 6-9; otherwise as Dash_Skid |
| Dash_Bump_Left | 9 | Dash_Loop 0 (cross-fade 3) | Dash_Loop 0 | left shoulder leads; contact frame 5; identical to `Dash_Bump` |
| Dash_Bump_Right | 9 | Dash_Loop 0 (cross-fade 3) | Dash_Loop 0 | right shoulder leads; contact frame 5 |

- Which side: pick the bump by the side the victim is on (victim to the dasher's left: `_Left`), and the skid by the braking turn's side.
- Unchanged: `Dash_Start` (10 frames, from Glide 0), `Dash_Loop` (24, loop), `Dash_Wipeout` (24, face down, then `Lie_Loop_Front` and the `GetUp_*Front` family), `Boost_Take` (11, over Stride 0-11, layer from `spine_01` up).
- Never: start Stride at 30 after `Dash_End` (use 34). Never pair a skid's side against the turn. Never move the root.

## 5. For the other device

As part 1, plus a side for the outcome: `EndDash(uint8 Reason, float DirX, float DirZ)`.
- For Reason 1 (skid), the sign of `DirZ` (the turn, + = left) picks `Dash_Skid_Left` or `Dash_Skid`.
- For Reason 2 (bump), `DirZ`'s sign (the victim on the dasher's left, +) picks `Dash_Bump_Left` or `Dash_Bump_Right`.
- Nothing new is needed beyond that sign.

## 6. Decisions for the operator

1. Still open from part 1: the trailing stick, the wipeout's tone. The left skid now exists.
2. New: the reworked loop is a speed skater's: one arm on the back, very wide pushes, a skate kicked up behind on every push. It reads as "a different gait" more than as a hockey player at full speed. Is that the dash the game wants, or should the arm stay swinging (body-only silhouette ratio 1.97 instead of 2.36)?

## 7. The next three evolutions, each with a proving number

1. **The loop watched at 60 fps** from frames already rendered (a clip per style at game size), judged once for fast against frantic. Retime to a 28-frame loop if frantic. Proving numbers: clips watched 2 of 2; silhouette ratio kept at 2.0 or more.
2. **The dash stick held clear of the skate line from the game camera:** the trail lifted 10 cm and 10 deg further out, so its line does not cross the right skate at 110 px. Proving number: frames where the stick's mask overlaps the right skate's mask, 0 of 24 (today about 7 per push, by eye).
3. **Side and size variants of the wipeout** (a wall clip ending at the wall lying pose, and a lighter tumble). Proving numbers: joins to `Lie_Loop_Wall` and `Lie_Loop_Front` exact; gate PASS.

Files (loop 2): the builder parts (`dash_a.py`, `dash_b.py`, `dash_bake.py`, `dash_tail.py`, `assemble.sh` on `build_clips_pass30_orig.py`; loop-1 parts kept as `*_loop1.py`), `iter.sh`, `sil.sh`/`sil.js` (silhouettes), `strip.sh` (every frame), `anchors2.js`, `bias.js`, `gate_joins_dash.txt`. Numbers: `work\checks.txt`, `work\clear.txt`, `work\blade.txt`, `work\sil_final.txt`, `work\anchors.md`. Gate input: `export_gate\` (its `GATE.md` copied into `export\`).

---

# Part 3: the third loop (stopped at the lead's call; closed 10:00 by this machine's clock)

## 1. Done / not done, and what went wrong

**The export is consistent again.** `C:\Users\kazda\jam\sets-lab\dash\export\` holds 12 takes per style (14 files, counting the two loop `_loopcut` files). `READY.txt` lists all 12 with start pose, end pose and key frames and was written last. The fixed gate (base export35, `--out C:\Users\kazda\jam\sets-lab\dash\gate_out`) gives **GATE PASS: 24 rows, 0 failing; 12 files, 12 takes found, 12 checked, READY.txt lists 12** per style; only its `GATE.md` is copied in. No Blender process of mine is running.

**Why the folder was inconsistent: my fault.** `run_dash.sh` writes the per-take FBX into `export\` on every build, so the iteration builds of this loop (from about 03:13) overwrote the delivered loop-2 files and added unlisted ones, while `READY.txt` still said loop 2.

The final chain (build, then checks, silhouettes, gate) was moved to the background when a message arrived, and I did not get back to it until the lead's call. That chain removed the export at its start and rebuilt it; by this machine's clock it was at 09:57 in its check step, and I stopped it there. It did not reach its join, silhouette or overlap re-runs, so those numbers below are from the identical build at 03:29.

The fix for next time: iteration builds write to a scratch folder; only a final build writes `export\`.

**Did any of the ten takes the engine has change after 03:07? No re-keying.** The ten are `Dash_Start`, `Dash_Loop`, `Dash_End`, `Dash_Bump`, `Dash_Bump_Left`, `Dash_Bump_Right`, `Dash_Skid`, `Dash_Skid_Left`, `Dash_Wipeout`, `Boost_Take`.
- Their source was not edited in this loop: the only edits that touch them are a stick parameter that was tried at -0.20 for one build around 03:15 and set back to its loop-2 value (-0.12), and the new takes added after them.
- Their gate rows now are identical to loop 2's gate rows: lowest points, worst turns, frames, root keys and joins. For example:
  - `Dash_Loop`: 31.2 (calf_r, 9) / 24.4, lowest stick 19.26.
  - `Dash_Start`: 27.2 / 28.6.
  - `Dash_Bump_Right`: 51 / 87.6.
  - `Dash_Wipeout`: 43.3 / 63.
- The files were rewritten (new timestamps) with the same keys. I cannot byte-compare with the loop-2 files: they were overwritten. The engine's loop-2 import stays valid.

**The four items:**

| Item | State |
|---|---|
| 1. The stick's line off the right skate | **Met without a change.** Measured (`overlap.py`: the stick's and the right skate's triangles rasterised from the 110 px game camera): 0 of 25 `Dash_Loop` frames overlap, both styles; the nearest gap is 8.1 px (cel) and 5.1 px (clay). My loop-2 "about 7 frames per push" was by eye and wrong: what touches the skate's line in the renders is the glove and the stick's grip end. A try with the top hand 8 cm further out (gap larger) broke the 40 cm line in `Dash_Wipeout` (40.5) and put the shaft in a leg in `Dash_Skid_Left`, so it was not kept. Silhouette ratio unchanged (2.83 / 2.65). |
| 2a. `Dash_Wipeout_Light` | **Done, in the export.** 22 frames, `Dash_Loop` 0 to `Stride` 0: a skate catches (3), three scrambling steps (7, 11, 15) with the free arm windmilling, a crouch (19). The stick sweeps back to the front over 12-22 (blade at most 37.9 / 36.4 cm a frame). Shaft 0 frames in the body. Gate PASS with its join `Stride@0`. Looked at every frame, both styles: the windmilling arm and the stumble read; the steps are small. |
| 2b. `Dash_Wipeout_Wall` | **Built, looked at, held back** (`C:\Users\kazda\jam\sets-lab\dash\held\`, not in the export). It ends exactly on the pose `Board_Fall_Front` ends on **in export30** (the front wall lying pose = `Lie_Loop_Wall` 0 = `GetUp_Wall` 0). The figure lane has since changed that pose: against export35 the gate fails its join by 10.7 cm (cel) / 13.3 cm (clay) at the right thigh. It needs one rebuild on `build_clips_pass35.py` (about a minute). Otherwise: blade at most 15.1 cm a frame, shaft clear, slam into the boards at frame 3, slide down, the bounce at 18. In its first two frames the body is up to 35 cm past the 35 cm wall plane: the dash pose leans far forward before contact. The engine should start it about 2 frames (0.6 m at dash speed) before the root reaches 35 cm from the boards. |
| 3. The loop judged for speed | **Done: strides, not flicker; no `Dash_Loop_B` built.** Speed used: the dash at 1.6 × the 11 m/s top speed (`docs\experiments\MOVEMENT.md` "0 to top speed 11 m/s", `ABILITIES.md` "1.6x top speed") = 17.6 m/s = 0.293 m a frame; the sprint at 12.648 m/s (MOVEMENT.md, the defender sprinting). Side view, orthographic, 110 px figure. Two cycles, every frame on one row (`speed.sh`): the figures cover each other into a smear at 19 px a frame. With every 3rd frame (`speed2.sh`, 57 px apart) each push shows as its own shape: the leg goes out wide, back, kicks up behind, and returns over 12 frames, and each push covers 3.5 m, two body heights. That reads as long, fast strides, not a flicker. It is brisk (5 pushes a second), and only stills were judged, not playback. Sheets: `C:\Users\kazda\jam\sets-lab\dash\sheets\speed-p2.png`, `C:\Users\kazda\jam\sets-lab\dash\sheets\speed-p2-k.png`. |
| 4. `Dash_Carry` | **Done, in the export.** 24 frames, a loop on `Dash_Loop`'s phase. Pelvis, legs, spine, neck and head are `Dash_Loop` frame for frame (0.000 cm). Both hands are on the shaft and the blade lies flat on the ice 1.2 m ahead and to the stick side, swaying 6 cm with the pushes (blade end 151 / 147 cm from the root, lowest stick 0.7 / 0.31 cm). Shaft 0 frames in the body after moving the blade to the stick side (centred, it went through the folded trunk for 6 to 8 frames). Gate PASS. For the engine: a layer from `spine_01` up over `Dash_Loop` (only the clavicles, arms, hands and the stick differ). |

**What the trailing-stick dash should do when the skater already carries the puck:** never trail. A carrier plays `Dash_Carry`'s upper body over the dash legs from the first frame:
- `Dash_Start`'s legs with `Dash_Carry` layered from `spine_01`.
- `Dash_Loop` with the layer.
- `Dash_End`'s legs with the layer until it joins `Stride`.

`Dash_Start` and `Dash_End` swing the stick 170 deg round the right side and would throw the puck off the blade. The trailing stick is for the dash without the puck, and for a carrier whose puck is lost by the bump or the wipeout.

## 2. Proving numbers (from the 03:29 build, identical source to the final export)

- Gate on the final export: PASS 24 of 24. Joins to the base: `Glide`@0, `Stride`@0 / 11 / 34, `GetUp_Front`@0 all within the gate's limits. Pops 0. Top hand 0 cm. Nothing under the ice beyond 0.05 cm.
- Blade end's largest one-frame move: at most 38.3 cm in the 12 exported takes (`Dash_Carry` 1.6).
- Shaft more than 3 cm inside a leg, the trunk or the head: 0 frames in all 12 takes, both styles (03:29 build).
- Stick and right skate overlap from the game camera: 0 of 25 loop frames, both styles.

## 3. Not verified

- `JOINS.md` is not in the export: the join script for the dash-to-dash pairs (`Dash_Loop` > `Dash_Wipeout_Light`, `Dash_Carry` wrap) was in the stopped part of the chain. The gate checks only joins to base takes. The 21 dash-to-dash joins of loop 2 come from the same code and were exact.
- The silhouette and overlap numbers were not re-measured on the final files (same source as 03:29).
- Nothing seen in the engine. The speed judgement is from stills, not playback.
- `Dash_Wipeout_Wall` is held back (above).

## 4. For the engine lane

Part 2's table holds for the ten takes. New in the export:

| Take | Frames | Starts on | Ends on | Note |
|---|---|---|---|---|
| Dash_Wipeout_Light | 22 | Dash_Loop frame 0 (cross-fade 3) | Stride frame 0 | the dash clips something small; no lying |
| Dash_Carry | 24, loop | (layer) | (layer) | layer from `spine_01` over Dash_Loop on the same frame; blend in and out over 4 to 6 frames |

## 5. For the other device

`EndDash` Reason 4 = clipped something small and kept the feet → `Dash_Wipeout_Light`. Reason 5 (when `Dash_Wipeout_Wall` ships) = met the boards head-on. With the puck, the figure needs to know the dasher carries it (the game already knows the carrier) to pick `Dash_Carry`.

## 6. Decisions for the operator

Unchanged: the tucked arm against a swinging one, the trailing stick, the wipeout's tone.

## 7. Next three

1. `Dash_Wipeout_Wall` rebuilt on export35's wall pose. Proving numbers: gate PASS with `end=wall`; 0 frames more than 2 cm past the wall plane from frame 3 on.
2. `JOINS.md` for every dash-to-dash pair, and iteration builds moved out of `export\`. Proving numbers: all pairs within 0.0005 cm and 0 deg; the export folder's file times equal `READY.txt`'s build.
3. The loop played at 60 fps for a real fast-against-frantic judgement (a clip from the existing frames). Proving number: clips watched 2 of 2.
