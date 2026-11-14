# A check and a cross-check are animations, not marks (lane A210, track F3, the night of 14 to 15 Nov, device A)

The operator after package H34 (R-0067): "crosscheck has bug in animation displaying technical blue rectangle instead of animating player with stick
into its position. Tackle is also unrecognizable and animation too quick". He plays blue, PLAY NOW at ROOKIE on clay, mouse and keyboard: without
the puck LMB cross-checks (held from 13 frames it braces), RMB body-checks.

Everything here is presentation: no sim file's rule is touched, no state field added; the sim's hash is the same with and without (section 4).
Follows `HITS.md` (lane A196), whose marks this lane takes back.

## 1. SEEN: what the blue rectangle was

**A's own mark.** `FJamHitReadModel::Draw` in `Game/Source/JamGame/Private/HitRead.cpp`, the block "The held brace: the streak across the chest and
an arc on the ice ahead" (`EJamHitLayer::Brace`, lane A196): one `/Engine/BasicShapes/Cube` of 140 x 12 x 12 cm in `/Game/Placeholders/M_Trim`, the
team's colour (blue: he plays blue) at emissive 3 to 6, 42 cm ahead of the figure at 110 cm height, across his facing, drawn on every frame of
`bCrossBraced` (all the time LMB is held from frame 13). From the game camera, which stands behind and above him, it is a flat blue slab across
his shoulders, wider than the body, that does not move with the arms. A TAP of LMB drew its sibling for 12 frames (`EJamHitLayer::Streak`: white
through the 4 wind-up frames, pure blue on the shove's frame and the next, then grey).

Proved by switching one thing at a time on the same sim frames (drill 23 and drill 2 from frame 0 at rookie on clay, seed 1; the cvars
`jam.hit.BraceBar`, `jam.hit.Streak`, `jam.hit.BraceArc` were added for this, default 1 in build 1; strips in `tools/qa/screen/out/a210-*`):

| switch (alone) | what goes | what is left on the screen |
|---|---|---|
| `jam.hit.BraceBar 0`, LMB held | the blue slab across the shoulders | device B's `CrossCheck_Hold`: the arms forward, the stick's blade end beside the left arm; the arc at the feet |
| `jam.hit.Streak 0`, LMB tapped | the white / blue / grey slab (12 frames) | the stick swung out level to the left (B's `Lift` take) |
| `jam.hit.BraceArc 0` | the 7 dashes on the ice ahead | the figure alone |
| `jam.hit.Lean 0`, held | nothing (the lean already stands down: B's figure declares `SetCrossCheckHold`) | |
| `jam.hit.BTakes 2`, tap at 4 frames | A's lean | B's `CrossCheck_Wind` from its frame 6: the coil is cut, the stick out from +3, held to +14 |
| `jam.hit.BTakes 2`, tap at 8 frames | A's lean | the hands rise by the right shoulder +0..+5, the stick sweeps level +6..+8 (the sim's shove is +8), held to +14, down by +19 |
| `jam.hit.BTakes 2`, body check at 4 | A's lean (the checker folded over the man) | B's take: turned, the shoulder into the man, the stick up |
| `jam.hit.BTakes 2`, body check at 8 | | the crouch +2..+5, the spring +7..+8, contact +9 |

Stills: `img/a210/seen-blue-rectangle.jpg` (top: H34 as shipped, sim frames 420 and 470; bottom: the same frames with `jam.hit.BraceBar 0`),
`img/a210/seen-streak-pressed.jpg` (the tap: H34 over `jam.hit.Streak 0`).

Why A196 drew it: **from behind, B's hold is hidden by the player's own body.** The bar stood where an animation should have been. Taking the bar
away shows the truth, and the truth is a weak read (section 5, FOR B).

## 2. What a player sees now

| he does | the figure (device B's takes, on the human as on the bots: `jam.hit.BTakes 2`) | marks left |
|---|---|---|
| taps LMB (pressed cross-check), wind-up 8 | `CrossCheck_Wind` from frame 2: the hands rise beside the right shoulder (+0..+5), the stick sweeps level across to the left (+6..+8), the shove on the sim's frame (+8, the take's frame 10: 0 frames off), the stick held out to +14, down by +19. `img/a210/cross-8-now.jpg` | none over the figure; the contact dashes if it lands |
| the same at 4 frames (until lane A209's 8 lands) | the take from its frame 6: no coil, the stick out from +3 to +14 | |
| holds LMB (the brace) | `CrossCheck_Hold` with its in and out (6 frames each): the arms forward, the blade end out to the left. `img/a210/brace-before-after.jpg` | 7 thin white dashes on the ice 1.35 m ahead |
| presses RMB (body check), wind-up 8 | `Check_Deliver_*_Wind` from frame 2: the crouch (+2..+5: the helmet drops about a third of a head), the spring into the man (+7..+8), contact +9 in the strip (the take's frame 10 is +8: one frame), the two bodies held by the hit-stop (2 to 6 frames), the follow-through with the stick out to +17. `img/a210/check-8-now.jpg` | a thin white chevron on the ice toward the man through the wind-up; at contact a few white dashes; the downed man's oval |
| the same at 4 frames | the take from its frame 6: turned, the shoulder in, the stick up (A's lean folded him over the man). `img/a210/check-4-before-after.jpg` | |
| is hit by a bot (8 frames) | thrown back along the hit, laid flat, his oval round him. `img/a210/bot-check-on-human.jpg` | |

A's stand-in lean was tried ON TOP of B's takes (`jam.hit.Lean 2`, the same sim frames side by side: `tools/qa/screen/out/a210-sheets/b2-check-8-pair-zoom.jpg`,
`b2-brace-pair.jpg`): nothing an eye catches. It stands down wherever B's take plays, as before (`jam.hit.Lean 1`).

## 3. The marks: which stayed, which went

| mark | verdict | now |
|---|---|---|
| the bar across the chest (held) | **gone** (`jam.hit.BraceBar 0`) | this was the rectangle |
| the bar across the stick (tap) | **gone** (`jam.hit.Streak 0`) | |
| the arc on the ice ahead of a brace | **stays, quiet** | 7 cm wide (16 before), the ice's white (the team's colour before), dimmer: the only sign of "braced" while B's hold is hidden from behind; `jam.hit.BraceArc 0` removes it |
| the chevron of a body check | **stays, quiet** | 7 cm wide (18), arms 60 % as long, white (white-hot over the team's colour before): a direction cue on the ice; grey on a miss as before |
| the contact ring | **stays, small near the eye** (wave 2: it grows back with the distance from the camera, section 5.6) | radius 0.95 to 1.35 m (0.8 to 3.0 before; 2.6 m at 11 m/s was the loudest thing on the sheet and covered the two bodies' moment), bars at most 11 cm wide (28), white only (white then the checker's colour before), 8 frames (14), no second ring on a braced hit, still nothing within 0.75 m of either body |
| the downed man's oval | **stays** | unchanged |
| the stripped puck's flash, the off-screen arrows, the radar's down marks | stay | unchanged |

`jam.hit.Quiet 0` with `jam.hit.BraceBar 1`, `jam.hit.Streak 1`, `jam.hit.BTakes 1` is package H34 again (the revert path; presentation only, any time).

## 4. Proof

- **The sim's hash**: drill 1 from frame 0 on the rookie cut (seed 1), package H34's look on build 1 (run `a210-x9-check-4-default`) against the new
  defaults on build 2 (run `a210-b2-check-4`): `JAMSIM` at 600 / 1200 / 1800 = `a34d4f88952450e6` / `249a688987f7795f` / `47f1c77d68337fc0` in both.
  The 8 / 8 cut: `eff4e990e4bc5112` / `9e48f22614e7115f` / `8c31ffba1ce220d8` in both builds. Drill 23: `17cca3717063067f` / `6a21692d10c389e6` and
  drill 2: `35ded7dbf42c4e95` / `0400fd2418c79eb0` whatever the `jam.hit.*` switches (10 runs). No JamCore file is changed.
- **Tests**: `Jam.Hit.Quiet` (new): the four shipping defaults; package H34's switches draw the bar over the figure (1 bar above 50 cm), the new ones
  nothing above the ice in a wind-up or a hold; the arc and the chevron 7 cm and white; a hard hit's ring within 1.35 m against over 2 m, white,
  clear of both bodies, gone 8 frames after the contact; the downed oval still drawn. The other `Jam.Hit.*` tests draw with package H34's switches
  (the struct's defaults) and pass untouched.
- **Strips** (a still a sim frame from the game camera, off screen, `-dpcvars=r.RDG.AsyncCompute=0`): `tools/qa/screen/a196-strips.ps1` now sends each
  run's stills to its own folder (`-JamHitShots=`), so two runs film side by side; `jam.hit.CaptureBots 1` films the bots' presses within 14 m too;
  `jam.ctrl.DrillLead N` makes drill 1 press N frames earlier (a staged 8-frame wind-up needs 4). Cuts: `tools/qa/screen/a196/cut-clay-rookie.json`,
  `cut-clay-rookie-8-8.json` (`check_windup_frames` 8, `cross_windup_frames` 8: lane A209's target, staged).

## 5. Wave 2: a tackle that is recognised, and a bridge for the brace (READY 2)

The regular match has no whistle (R-0048): the strips' cuts carried `penalties: true` from an older cut; from here on they are the regular cartridge
at rookie (`tools/qa/screen/a196/cut-clay-rookie.json`, `-8-8`, `-pro`, `-cam20`), so a check from behind no longer sends the camera to the box.

### 5.1 The coil and the drive on the figure (`jam.hit.Pop 1`)

Device B's body-check take crouches and springs; from behind at 7 m that is a helmet travelling about 33 px (960 x 540). The drawn figure (the
skater actor's scale, never the sim) now does the same thing larger: through the wind-up it sinks and widens (to height x 0.90, width x 1.06, fully
sunk three frames before the window), over the two frames up to the window it springs tall and narrow (x 1.07 / x 0.96), holds through the contact
(the hit-stop keeps the contact frame's size), settles over 8 frames. A contact inside the wind-up (lane A209's rule) goes straight to the drive. A
cross-check gets 0.7 of it. Measured on the same sim frames (the landed 8-frame check, sim 4061 to 4070): the helmet's coil-to-spring travel **33 px
without, 55 px with**; no frame reads as rubber (`tools/qa/screen/out/a210-sheets/g6-check-8-pop-zoom.jpg`). `FJamHitReadModel::CheckPop` / `PopFor`,
`AJamHitRead::PopScale`; `jam.hit.Pop 0` = off.

### 5.2 The brace: a bridge on the figure (`jam.hit.BraceSet 1`, `jam.hit.BraceLean 12`)

What can be had without device B's pose, and what cannot. **The stick cannot be shown**: it is a mesh skinned to B's animated `stick_socket` bone; A
has no call that poses it, and a line drawn where it is would be a slab in the air again, seen through his back. What A can move is the figure
itself: while the cross-check is held the drawn figure **sets**: over 6 frames (the length of B's `CrossCheck_Hold_In`) it sinks a tenth, widens 6 %
and pitches 12 degrees forward; over 6 frames it stands up again when the button is let go or the brace is spent. An eye sees "he hunkers down" and
"he stands up": the state is on the figure, nothing is drawn on it. It does not say "he holds his stick across": that is B's (section 7).
`img/a210/brace-bridge.jpg`: package H34 (the bar), READY 1 (the bar gone, the hold hidden), now (the figure set), the same sim frames.
Set `jam.hit.BraceSet 0` when B's pose reads from behind.

Where B's hold has the stick (new `JAMHIT stick` lines, the figure's own frame, cm; the head bone stands at 114): the right hand at (43 ahead, 9 to
23 right, 96 up), the left at (38 to 46 ahead, 25 to 40 left, 98 up): hands 49 cm apart at chest height, the stick level and across but gripped at
its butt: all its free length (about 100 cm) runs out to the left, nothing passes the right hand.

### 5.3 The hit-stop and the kick: measured, left as they are

- **Hit-stop** (`jam.hitstop 3`, `AJamSkater::HitStopFramesFor`): already by the hit's weight: a knockdown holds 2 frames at ratio 1.0 to 6 at 3.0 (the
  11 m/s check of the strips: 6 frames, the two bodies standing in the contact pose from +5 to +10: reads); a stumble 1 or 2. `jam.hitstop 5` changes
  nothing on the heavy hit (the cap is 6). Not changed: the weak knockdown's 2 frames are short, but the rule and its test
  (`Jam.Core.Contact.HitStop`) live in the contact lane's files, which are being edited tonight; the change is one number (a knockdown from 3, not 2)
  and is offered as the next evolution.
- **Kick** (`jam.hitkick 1`, 12 cm along the hit, 8 frames): at 1, 2 and 3 on the same frames the pictures differ (mean absolute difference 9 grey
  levels over 7 frames at 2 against 1) but side by side an eye cannot tell them apart in stills: it is felt in motion. No proof that more is
  better: left at 1.
- **The victim thrown along the hit's line**: `SetKnockedDown(dir)` and the sim's own push do it: in every landed strip the man leaves along the
  checker's line and lies with his oval round him.

### 5.4 Before and after, the same sim frames

`img/a210/tackle-4-before-after.jpg`: the first hit of drill 1 (rookie, the regular cut, sim 142 to 160: the human's check at 4 frames, 11 m/s, the
man down): top row package H34's look (`jam.hit.Quiet 0`, `BraceBar 1`, `Streak 1`, `BTakes 1`, `BraceSet 0`, `Pop 0`), bottom row the defaults. Before:
the checker folded over the man under a ring of white slabs 5 m across that turns blue. After: he turns his shoulder in with the stick up, the two
bodies stand held, a few white dashes at the feet, the stick stays raised through the follow-through. JAMSIM 600 / 1200 / 1800 equal in both runs
(`a34d4f88952450e6` / `249a688987f7795f` / `47f1c77d68337fc0`).

### 5.5 A check that lands inside its wind-up (`jam.hit.Retime 1`)

Lane A209's rule for the human ("a meeting inside the wind-up is the hit") finds B's take in its coil when the man falls: the take puts contact on
its frame 10 and was started at `10 - wind-up`. Device B's figure does nothing with `PlayCheckWindup` on a running take (tested: a proof switch,
`jam.hit.RetimeTest 3`, on the staged 8 / 8 cut: the crouch goes on to +6 as shipped). So A runs the figure forward itself: on such a contact (two
frames or more before the wind-up's end, `AJamHitRead::RetimeFor`) the figure actor's time dilation is set, for ONE sim frame, to the frames the take
is short of its contact pose (wind-up - press to contact), then put back to the hit-stop's stand-still (or to 1). Seen with the proof switch at +3
(`tools/qa/screen/out/a210-sheets/k12-ff-pair.jpg`, run a210-k12-check8-ff3 against a210-j9-check8-now on the same sim frames): the figure stands
in its drive pose on +4 (head bone 87 cm ahead and 110 up at +6; 49 / 100 or 29 / 91 without). The sim never sees the figure's clock (hashes
equal). On today's sim it cannot fire by itself (a contact comes only in the window): the real case, with the hit-stop behind it, is to be seen on
A209's build. `jam.hit.Retime 2` sends the call instead (for the day B's figure honours it); 0 = off.

### 5.6 The tackle at 20 and 35 m: the ring grows back with distance (`jam.hit.RingFar 1`)

A hit between bots inside the picture at 15 to 40 m could not be had by waiting (a human standing still, `jam.hit.CaptureBots 2`: one press, a miss,
in 25 000 frames; the two landed hits filmed from the press were outside the picture, where A196's edge arrow with its distance did show). So the
same hit was filmed from far: the human's first check of drill 1 with the cut's camera 20 m behind and 9 m up (`cut-clay-rookie-cam20.json`; the
schema's limit is 20 m, so the 35 m rows are the same stills at 57 % size). Read on those strips (wave 1's marks against package H34's): at 20 m the
raised stick and the two bodies still read with the quiet marks; **at 35 m the bodies are 30 px and a hit with the small ring is almost not
there, while H34's ring said "a hit, there"**. The small ring was right near the eye, where it covered the two bodies of the player's own hit, and
wrong far from it. Now the quiet ring reads the distance from the camera: within 10 m (every hit of the player's own: his camera is 7 m from him)
the small one (0.95 to 1.35 m, 8 frames); from 10 to 30 m it grows back to package H34's radius (to 3 m), bar width and 14 frames; white at any
distance. On the screen it is about the same size wherever the hit is. `img/a210/tackle-at-20-and-35-m.jpg`: rows H34, now, at 20 m; rows H34,
now, at 35 m's size; sim frames 144 to 170.

### 5.7 On the real sim (lane A209's contact in the tree: the human's wind-ups 8 / 8, the pull toward the man)

Drills 1 and 2 from frame 0 on the regular cut at rookie (runs a210-n15-check, a210-n15-cross; 20 presses of the human):

- **The check**: 10 of 10 landed; press to contact 4, 0, 6, 2, 7, 6, 7, 7, 7, 7 frames. At 7 the take is one frame before its contact pose. The five
  that landed two frames or more inside the wind-up were run forward (5.5): `img/a210/early-contact-retime.jpg` is the same sim frames with
  `jam.hit.Retime 0` (top: the checker crouched through the hit-stop, standing up when the man is gone) and 1 (bottom: in the drive pose from the
  frame after the contact, through the hold). The contact frame itself still shows the coil: one frame.
- **The cross-check**: 4 of 10 landed, every one at press + 8: the shove on the take's frame 10, 0 frames off. `img/a210/cross-check-lands-real.jpg`.
- **The pull** (`JAMHIT step` lines, `tools/qa/screen/a196/steps.py`): beyond the step his own speed explains it added at most 5.6 cm a frame along
  his facing (his own step at 12.8 m/s is 21 cm); the step changed by at most 6.2 cm from one frame to the next, as without the pull; across the
  facing up to 13.7 cm a frame, against 9.7 to 15.2 without the pull (a turning skater slides across his facing by himself). **It reads as a
  lunge; nothing looks drawn by a string.** Where it would: over about 15 cm a frame across for three frames with the facing not turning, or a step
  growing by over a third in one frame (status/A210.md, FOR A209).
- Hashes on the real sim (drill 1, seed 1): the defaults against package H34's look: section 4's proof repeated in status/A210.md, HITS READY 3.

## 6. The boosts' colours (device B's answer to A-to-B-0031 point 3)

`JamEffects::KindColour` (`Game/Source/JamCore/Effects/JamEffectTypes.cpp`), the one table behind the pickup's tint, its beacon (pedestal and ground
ring), the radar's dots and the HUD's chips, is now `contracts/events.md`'s: speed (0.1, 0.9, 1.0), precision (1.0, 0.2, 0.9), heavy_puck
(0.55, 0.3, 1.0), big_puck (1.0, 0.75, 0.1), reverse_other (1.0, 0.15, 0.1), custom and unknown white (before: yellow, cyan, violet, white, magenta,
green). The binder's `PickupKindColour` already had them. Pinned in `Jam.Fx.BoostLook`. `img/a210/boost-kinds-contract-colours.jpg`: the five kinds on
the ice in one still (cut `tools/qa/screen/a196/cut-clay-boost-kinds.json`): prop, pedestal and ring agree for each kind.

## 7. FOR B (what the eye still lacks; exact)

See `status/A210.md`, section FOR B: the hold must read from behind; the cross-check's shove must show above the shoulders; the body check's
drive needs the shoulder and a flag; the clamp at 12 frames; the braced hit's sound.
