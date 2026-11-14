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
| the contact ring | **stays, small** | radius 0.95 to 1.35 m (0.8 to 3.0 before; 2.6 m at 11 m/s was the loudest thing on the sheet and covered the two bodies' moment), bars at most 11 cm wide (28), white only (white then the checker's colour before), 8 frames (14), no second ring on a braced hit, still nothing within 0.75 m of either body |
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

## 5. FOR B (what the eye still lacks; exact)

See `status/A210.md`, section FOR B: the hold must read from behind; the cross-check's shove must show above the shoulders; the body check's
drive needs the shoulder and a flag; the clamp at 12 frames; the braced hit's sound.
