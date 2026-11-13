# Tackling that reads (lane A33, device A, H11.5-H12.x, 2026-11-12)

R-0025 point 1 and R-0028 point 3: P2 ships playable with tackling that reads. The sim already had body checks (R-0013: fouls from behind and on a skater without the
puck), knock-downs (`knockdown_frames` 90), the strip on a check (C-0005) and the stumble on a mistimed poke (`stick.stumble_frames` 20). What was missing is that a
player *sees and feels* the hit. Everything here is cosmetic over the sim: the sim gained two read-outs (a direction on the `check` event, a `board` event for a skater),
no state and no play changed; `Jam.Core.Determinism` keeps its hashes (regular `884f1c9a85793dd5`, raw `6027ac7eba473026`) and `Jam.Bot.VisualSwap` its
`1871c1d348fa8b41` on all three figure tracks.

Status blocks with timestamps: `status/A33.md`. Code: `Game/Source/JamCore/Private/JamSim.cpp` (the read-outs), `JamPawns.{h,cpp}` (the puck pop),
`Game/Source/JamGame/{Public,Private}/SkeletalSkaterVisual.{h,cpp}` (the reactions), `CameraFollow.{h,cpp}` (the punch), `EffectsBinder.cpp` (the Variant rows),
tests `JamCore/Private/Tests/JamTacklingTests.cpp` and `JamGame/Private/TacklingTests.cpp`, proof driver `tools/qa/tackle.py` (+ `stop33.py`, `cartridges/hockey.tackle.qa.json`).

## 1. Event payloads (C-0003 additions for the Director to sign; the bus and the binder already carry them)

| Event | Who | Where | Strength | Kind | New here |
|---|---|---|---|---|---|
| `check` | the checker | contact point | closing speed / `game.skater.speed`, clamped to 1 (unchanged) | **`front` / `side_l` / `side_r` / `behind`**: where the hit came from, seen by the victim | Kind |
| `knockdown` | the victim | the victim | the check's strength (unchanged) | `check` / `boost` (the cause, R-0001 item 3, unchanged) | nothing; the victim's visual reads the strength here |
| `board` | **the skater** (was always -1) | the contact on the boards | **normal speed / `game.skater.speed`** (the puck's stays / `puck.max_speed`) | **`skater`** (the puck's stays absent) | Who, Kind, a second meaning of Strength |
| `poke` | the poker | the blade | 1 took it from an opponent, else 0 | `clean` / `miss` (R-0013, unchanged; `miss` = the stumble starts this frame) | nothing |
| `foul` | the fouler | contact | the check strength | `foul_from_behind` / `foul_no_puck` (unchanged) | nothing |

The direction rule is the sim's foul rule made four-way: with `Dir` the push (checker towards victim) and `F` the victim's facing, `Dir . F > 0.5` is `behind`, `< -0.5` is
`front`, else the side the checker stood on (`side_l` = the checker came from the victim's left; the sim's yaw grows to the right, so the left is `(sin yaw, -cos yaw)`).
Thresholds: a check lands only above `check_speed_min` (5 m/s closing) with the window open (6 frames); a skater `board` event needs >= 4 m/s into the boards
(`SkaterBoardEventMinSpeed`), the puck's keeps its 3 m/s and 6-frame gap. The visual's `USkeletalSkaterVisual::HitKindFor` applies the same rule to
`SetKnockedDown(dir)` against the facing it was given, so B's figures can do the same from their own `SetKnockedDown` without a new interface call (section 6).

Binder (`UJamEffectsBinder::VariantFor`, B-to-A-0012 style): `check` -> `User.Variant` / `Variant` front 0, side_l 1, side_r 2, behind 3; `board` -> puck 0, skater 1.
`NS_board` / `MS_board` already exist and now also fire for a skater's hit with its Strength.

## 2. Reaction table (direction x strength -> take / effect / sound / camera)

Strength is the event's 0..1. "Take" is what `USkeletalSkaterVisual` (the P2 toy figure, A26) plays; the authored sequences are A22's (`knocked_down` falls backward,
`get_up`, `check`, `poke`), the rest is the whole-toy transform in `ReactionFor` (world-axis tips about the skates). Camera = the human's follow camera only.

| Direction (victim's view) | Strength | Victim's take | Checker's take | Effect / sound (binder) | Camera (human lands or takes) | Puck |
|---|---|---|---|---|---|---|
| `front` | 0.45-0.7 (5-7.7 m/s) | `knocked_down` at rate 0.99-1.06, back to the push (yaw over 6 frames), held at frame 39 while down; `get_up` 20 frames | `check` 20 frames | `NS_check` / `MS_check` Variant 0, `NS_knockdown` / `MS_knockdown` at the victim, Strength | 6-frame punch 1.0-1.4 cm, 0.3-0.42 deg | pop 17-25 cm over 18 frames if stripped |
| `front` | 0.7-1.0 | same, rate 1.06-1.15 (the fall lands 2 frames sooner) | same | same, Strength up | 1.4-2 cm, 0.42-0.6 deg | pop 21-25 cm |
| `side_l` / `side_r` | any | the toy holds idle and **tips 88 degrees toward the push** over 14 frames (x 0.85 + 0.3 strength), 3 + 3 frame bounce to 80, flat while down; untips over the 20 get-up frames | `check` | Variant 1 / 2 | punch, kicked to the side the contact is on | pop |
| `behind` | any | the toy tips **forward onto its face** (same curve), no yaw turn; the checker is also fouled when `rules.penalties` is on | `check`, then `PlayFoulReaction` (`check` at 0.6 for 33 frames) if a foul | Variant 3; `NS_foul` / `MS_foul` Variant 0 (`foul_from_behind`) | punch | pop |
| below `check_speed_min` | < 0.45 | **nothing** (no contact event exists; the skaters bounce apart by the contact exchange) | nothing | nothing | nothing | stays |
| boards, skater >= 4 m/s | 0.36-1.0 | 12-frame lean into the boards: 10 + 6 x strength degrees, 3 frames in, 9 out (never while down) | | `NS_board` / `MS_board` Variant 1 with Strength | none (no event for the human is a check) | |
| mistimed poke (`poke` `miss`) | 0 | `poke` as a 6-frame jab (rate 3), then **20 frames of stumble**: `get_up` frames 0-7 held, 12 degree forward pitch (bell), 5 degree yaw wobble (two cycles, dying), the stick 25 cm low | | `NS_poke` / `MS_poke` Variant 1 | none | |
| `jam.HitHoldFrames N` (judged, default 0) | | the first fall frame held N frames before the fall runs | | | | |

The stay-up hit reaction the brief names ("a hit reaction without a fall below the knock-down threshold") has no sim state to hang on: every landed check knocks down and a
contact below `check_speed_min` emits nothing. Proposal (a calibration under R-0021 point 1, **not applied** by this lane because its JamSim scope was read-outs only):
a landed check with Strength < 0.45 (a strength band chosen by feel, say under 0.5; note that a check lands on the checker's own speed while the strength is the closing speed, so the band
already catches the chase-down hits like the proof run's 0.03) sets `StumbleFrames` 30 on the victim instead of `KnockFrames` 90, with `knockdown` carrying Kind `stagger`; the visual would then play the stumble layer
(already built) in the hit's direction. That changes the state hash: FEEL.md gets the numbers and a re-recorded Determinism baseline when it lands.

## 3. Numbers (frames at 60 Hz; thresholds)

| What | Value | Where |
|---|---|---|
| check lands | closing >= `check_speed_min` 5 m/s within the 6-frame window; Strength = closing / `skater.speed` (11 regular, 8.6 raw) | `UJamSim::CollideSkaters` |
| direction bands | `Dir . F` > 0.5 behind, < -0.5 front, else side (60 degrees either side of the axis) | same, `HitKindFor` |
| front fall | authored 40 frames (88 degrees back in 14, 3 + 3 bounce, hold), played at 0.85 + 0.3 x strength, yaw to the push over 6 frames; get-up 20 | `ChooseLayer` |
| side / behind fall | tip 88 x (t/14)^2 degrees, 88 -> 80 over frames 14-17, back to 88 by 20, flat; untip over 20 | `ReactionFor`, `UToySkaterVisualComponent::KnockTipDeg` |
| hit hold | `jam.HitHoldFrames` 0..30, default 0 (judged below) | `SetKnockedDown` |
| board event (skater) | normal speed >= 4 m/s; Strength = v / `skater.speed`; one event per hit (the bounce reverses the normal speed, so no gap is needed) | `CollideSkaterStatic` |
| board lean | 12 frames, 10 + 6 x strength degrees, smoothstep 3 in / 9 out | `ReactionFor` |
| stumble | sim 20 frames (`stick.stumble_frames`, 0 on raw); visual 6-frame jab + 20 frames stumble, pitch 12 x sin(pi t), yaw 5 x sin(4 pi t)(1 - t), stick -25 x sin(pi t) cm | `ReactionFor` |
| camera punch | 6 frames, 2 cm right / 0.6 deg roll x max(0.5, strength) x (1 - i/6), only `check` Who = human or `knockdown` Who = human | `UJamFollowCamera::PunchOffset` |
| puck pop | 18 frames (0.3 s), 25 cm x (0.5 + 0.5 x min(1, |v| / 6)) x sin(pi t); only on a strip (LastTouch -> -1, puck moving, path continuous) | `AJamPuck::PopLiftM`, `IsStrip` |
| measured (Jam.Core.TacklingEvents, raw, top speed 8.0 m/s) | 6 / 9 / 12 m/s head-on -> strength 0.698 / 1.0 / 1.0 (5.6 m/s at contact after the glide), contact at frame 5 / 3 / 3 of the window; from behind, left, right and a -z facing give `behind`, `side_l`, `side_r`, `front`; a 6 m/s start 2 m from the boards arrives at 4.15 m/s (0.519) at frame 19, 3 m/s gives nothing | `Game/logs/a33-tests-2.log` |

## 4. Proof (PIE, worktree editor on 8020, `hockey.tackle.qa.json` = the regular cut with `toy_skeletal`, pro bots, penalties off, first to 9)

`tools/qa/tackle.py` drives the human through `UJamPlayerInput.DebugInjectKey` into the opposing carrier and presses check inside 1.35 m above 5.3 m/s, reads the bus
through `on_jam_event.add_callable`, runs the match at global time dilation 0.25 (the sim steps per dilated delta; one Python tick per sim frame) and captures the PIE
viewport (`take_high_res_screenshot` 1280 x 720) every frame in the hit window and every 2nd frame for the clip. Every number below is from the run's `status.json`
(`tools/qa/out/a33-tackle-1/`) and the `JAMEVENT` lines.

Run `a33-tackle-1` (19:30-19:46, 1664 sim frames, 539 captures, editor pid 32636 on 8020): three checks landed by the human, none taken (the pro bots never pressed
check on the human in 28 s of play), 13 skater `board` events, no `JAMFX missing` line (B's `NS_` / `MS_` for check, knockdown, board, poke, foul all load).

| Hit | Sim frame | Direction (victim) | Strength | Closing | What the camera shows | Strip / still |
|---|---|---|---|---|---|---|
| 1 | 697 | `front` | 1.00 | the human at 9.3 m/s head-on into orange 3 (the carrier) 0.8 m away | the check, the burst at the contact, orange 3 flat on its back by +18, the kick | `img/tackling/a33-hit1-front-s100-strip.png`, `a33-hit1-front-f705.jpg` |
| 2 | 1074 | `front` | 0.03 | both skaters moving the same way in the corner: the checker's own speed (> 5 m/s) lands it, the closing speed is 0.3 m/s | the camera's board clamp puts the two figures across the whole frame; not readable (the known A10 corner case) | `a33-hit2-front-s003-strip.png` |
| 3 | 1394 | `behind` | 0.26 | the human catches orange 3 from behind at 2.2 m/s closing (penalties off in the QA cut; with them on this is `foul_from_behind`) | the toy tips forward along the push, away from the camera (reads as the figure shrinking, then flat); the puck's disc stays by the fallen figure | `a33-hit3-behind-s026-strip.png`, `a33-hit3-behind-f1406.jpg` |

The visual's own classification matched the sim's on every hit (`SKELVIS JamSkater_3 knocked down kind=front strength=1.00 ... kind=front strength=0.03 ... kind=behind
strength=0.26`, the strength taken from the `knockdown` event in the same frame). **Clip**: `img/tackling/a33-tackle-clip.mp4` (9.85 s at 20 fps, 197 frames = every
2nd sim frame of frames 660-1260 at the real pace: the approach, hit 1 at the end boards, the retreat, the second approach). Camera punch: present on hits 1-3 in the
captures as a one-frame shift of the boards' line (2 cm at 6 m is small by design: it is felt, not seen; the roll of 0.6 degrees is below what a still shows).

**Found in the run** (for FEEL.md and the Director):
- A check lands on the checker's **own** speed along the contact normal (`ASpeed > check_speed_min`) while the strength is the **closing** speed: hit 2 landed at
  strength 0.03 and still put the victim down for 90 frames. Proposal (calibration, hash change, not applied): land on the closing speed too, or at least give the
  knockdown a floor of 0.3 in strength so the fall does not play at its slowest for a hit that counted.
- Skater `board` events: 13 in 1664 frames, 0.39-1.0, in bunches when the bots chase a dumped puck into the end boards (three within 7 frames). The sound will be busy
  on a regular cut; see section 7 point 3.
- The puck pop and the camera kick are both at the edge of what a 1280 x 720 still shows; the clip carries them.

**Reaction sheet: not captured.** `tools/qa/react33.py` freezes a PIE frame (time dilation 0.0001), knocks a figure down by hand through the interface in the four
directions plus `front` with `jam.HitHoldFrames 3`, steps the visual one `UpdateMotion` per tick and captures every 3rd frame for 45 down and 24 up. Three runs
(19:49 the human's own figure; 19:54 the opposing centre at the faceoff, 8 m in front of the camera; 19:57 the same with the game thawed to 1.0 for each capture tick)
all gave the same result: the first capture and the last render, every one between comes back as a flat ice-coloured fill with a tilted black band (345 ticks, 115
captures, no Python error each time; frames kept in this lane's scratchpad). The tackle run at dilation 0.25 captured every frame cleanly, so the viewport capture on
a near-frozen frame is the problem (A29 met the same black captures after an `asyncio.sleep` in `execute_python`; A26's single stills on a frozen frame worked).
Not resolved in the time box. Consequences: the side tips are proven by the pure-function test (`Jam.Tackling.HitKind`: 88 degrees at frame 14, flat, up over 20)
and by hit 3 in the match (behind), not yet by a side view; **`jam.HitHoldFrames` stays 0** (not judged; one PIE run of `tackle.py` with the cvar set to 3 before
`StartPIE` judges it when the capture works). A 0.25-dilation variant of the sheet during the faceoff countdown is the next thing to try (the pawn steps the figure
itself then; the centres stand still for 90 frames).


## 5. Eyes (tools/eyes, Gemini; advice, not instructions)

Both calls on `gemini-3.8-flash` with `--no-think` at 24 m, about one cent in all (`docs/experiments/eyes/a33-hit1-{bughunt,readability}.json`). Read against the
stills myself: what Gemini calls "no legs / truncated torso / sunk into the ice" is the authored backward fall with the pelvis below the ice plane (A22's `knocked_down`,
unchanged by this lane, visible at 10 m; a bug for the figure pass); the silhouettes are the look near the boards (the night rig lights the ice, not the figures by the
kick plate: team colour and the carrier do not read there, on the hit frames or any other); the "floating grey and red box" is the goal light of A25's construction
layer, which stands on nothing; the mirrored numbers on the boards are the jersey number text renders seen through the figures from behind (A26's known defect). None
of it is the reaction layer: the fall, the burst and the kick are there; what hurts the reading of a hit at 24 m is the lighting of the figures, which is B's and A23's.

**eyes/bug-hunt** A=a33-hit1-front-f705.jpg B=a33-win-f000715.png (gemini-3.8-flash, in 2895 out 616 tokens, ~$0.0045, 11.1 s)
- Still A, clean: false: figures rendered almost entirely as black silhouettes, jersey colours lost (severity 4, 0.95); the tilted skater "missing skate blades and legs,
  floating truncated above the ice" (4, 0.9); the grey and red box above the glass with no mounting (3, 0.85); mirrored numbers on the board (2, 0.8).
- Still B, clean: false: pitch-black silhouettes (4, 0.95); the fallen skater's torso truncated into the ice, the adjacent skater sunk to the hips (4, 0.95); the floating
  box (3, 0.85); detached text remnants on the board (2, 0.75).

**eyes/readability** A=a33-hit1-front-f705.jpg B=a33-win-f000697.png (gemini-3.8-flash, in 3066 out 671 tokens, ~$0.0048)
- A: first look "cluster of black silhouette skaters encircled by white particles"; overall 2 / 5; puck 1, carrier 1, team colours 1 (all figures black), far net 5,
  zone strips 4; noise: the particle ring, the yellow board strip, the numbers on the boards.
- B: first look "red net on the right foreground"; overall 2 / 5; the same element scores; noise: the glowing strip, white dots between the skaters' feet, numbers on the boards.

**eyes/bug-hunt** A=a33-hit3-behind-f1406.jpg B=a33-win-f001412.png (gemini-3.8-flash, in 2897 out 457 tokens, ~$0.0039, 7.1 s)
- A: both skaters unlit black silhouettes (5, 0.99); the left (fallen) skater "cut off and sunken into the ice without visible legs" (4, 0.95).
- B: the silhouettes (5, 0.99); the central skater "floating above the ice well above its particle ring" (4, 0.95: the glowing disc is the puck's SM_PuckDisc under the
  puck, not under the skater); the left skater "partially submerged below the ice plane" (4, 0.95); "two black fragments above the left skater's head" (3, 0.9: the
  jersey number text render of a figure behind).
- My reading of the same frames: on a hit from behind the toy tips forward along the push, which from this camera is *away*, so the figure foreshortens from the top
  down and the silhouette reads as a shorter figure (not as a body on its face); Gemini reads that as "sunken". A fall seen from the side reads; one seen along its
  axis does not. The authored front fall has the same problem from behind (A22's pelvis drop below the ice, EYES wrong-reading or not: the mesh does go under).


## 6. What B's figures need (handoff draft for the Director: A-to-B, "tackling that reads on TB1 / TB2")

> B: lane A33 put the direction of a hit into play. Nothing in C-0001 changed: `SetKnockedDown(bDown, DirX, DirZ)` still gives the push direction in rink metres
> (x along, z across). The toy figure now classifies it against its facing with the sim's rule (push along the facing = hit from behind, against it = from the front, else
> the side the checker stood on) and plays a different fall per class. Your figures already have `Fall` (backward) and `Fall_Forward` + `GetUp` / `GetUp_Front`; please map:
> front -> `Fall` with the back to the push (the figure turns away over ~6 frames), behind -> `Fall_Forward` facing the push, sides -> either take with the figure turned
> so it falls along the push (a side-fall take would be better: the Meshy library's "Knock Down 1" or the Manny `MM_Death_Left/Right_01` that A29 ledgered as
> `fall_left_manny` / `fall_right_manny`), get-up matching the fall. The hit's strength is on the bus: `knockdown` (Who = the victim) carries it; `check` carries
> `Kind` = `front | side_l | side_r | behind`. A board hit by a skater now emits `board` with Who = the skater, Kind `skater`, Strength = normal speed / top speed:
> `NS_board` / `MS_board` get `Variant` 1 for it (0 stays the puck); if the figure can lean into the boards for 12 frames (a `hit_in_back`-style take at rate 0.6 facing the
> boards, as P4-MOTION.md section 1 proposes), bind it to that event. A mistimed poke: the sim's stumble is 20 frames from `PlayPoke(false)`; your `Stumble` starts 18
> frames after the poke today, which lands it after the sim has recovered: start it at most 6 frames after. Glass: if `M_Glass` has a scalar a hit could drive (a wobble
> amplitude), name it and the binder will set it on the nearest panel for 12 frames; A33 did not open the material.

## 7. Open for the Director

1. C-0003 additions in section 1 (`check` Kind, `board` Who / Kind / Strength meaning for a skater). The binder and the bus already carry them; the contract text is yours.
2. The stay-up hit (section 2, last paragraph): a calibration with a hash change, proposed, not applied.
3. The skater `board` event rate: bots chasing a dumped puck hit the end boards at top speed in bunches (three at 0.83-1.0 within 7 frames in the proof run). Options: raise
   the threshold to 6 m/s, or a per-skater gap of 30 frames; both are read-out changes. The clip decides (section 4).
4. `jam.HitHoldFrames`: the judged value (section 4) can become the default in `SetKnockedDown` with one line.

## 8. C-0001 v4: B's four tackling calls, the down time on the visual, bots that check (lane A45, R-0031, from H12.4, 2026-11-12)

Handoff B-to-A-0028 found four takes on B's figures with no caller (`Check_Hit_*`, `Board_Hit`, `Board_Pin`, `Poke_Hit` / `Lift_Hit`) and asked for the down time on
`SetKnockedDown`; R-0031 point 2 ended R-0017 point 4's "no bot ever body-checks" for pro and all_star. Code: `JamSkaterVisual.h` (v4), `JamSim.{h,cpp}` (read-outs only,
never hashed), `JamPawns.{h,cpp}` (the triggers, `JAMVIS` lines), `SkeletalSkaterVisual.*` and `ToySkaterVisual.*` (defaults on existing curves), `JamAICore.{h,cpp}` +
`DifficultyProfile.*` (the bot check), `EffectsBinder.*` (the glass hook), tests `Jam.Core.TacklingReadouts`, `Jam.Tackling.V4Defaults`; proof driver `tools/qa/v4.py` (+ `stop45.py`,
`a45strip.sh`, `cartridges/hockey.v4.qa.json`). Status blocks: `status/A45.md`.

### 8.1 Trigger table (what the pawn sends, from which sim read-out)

| Call (C-0001 v4) | Sim read-out on `FJamSkaterState` (one frame, never hashed) | Set where | Rule | B's take | Toy / skeletal default |
|---|---|---|---|---|---|
| `SetKnockedDown(true, DirX, DirZ, KnockdownFrames)` | `KnockFrames` on the frame it starts (StepSkater counts it down from the next step) | `UJamSim::KnockDown` (unchanged) | `KnockdownFrames` = the whole down time; `SetKnockedDown(false, 0, 0, 0)` arrives exactly that many frames later | `Knockdown_*` then the matching `GetUp_*`, the get-up timed to end on it | unchanged (both hold their fall while down); the value is logged (`SKELVIS ... down_frames=`) |
| `PlayHitReaction(DirX, DirZ)` | `bBumpThisFrame`, `BumpDir` (the push on this skater: the other towards it), `LastBumpFrame` | `CollideSkaters`, on the velocities before the contact exchange | a skater-skater collision with closing speed >= 3 m/s (`BumpMinClosing`) and no check landed in the pair this frame; both standing skaters; one per skater per 20 frames (`BumpGapFrames`); never while down | `Check_Hit_Left/_Right/_Back` by the direction against the facing | skeletal: the board lean's 12-frame curve, 14 degrees toward the push; toy: 14 degrees on the check curve (8 in, 12 out) toward the push |
| `PlayBoardHit()` | `bBoardHitThisFrame` | `CollideSkaterStatic`, with A33's skater `board` event (>= 4 m/s normal speed) | the same frame as the event; Strength stays on the bus | `Board_Hit` | skeletal: the A33 lean (the bus event already starts it; the call stands in when none came); toy: body pitched 12 degrees forward on the check curve |
| `SetBoardPin(bOn)` | `BoardFrames` (consecutive frames within 5 cm of the boards, `BoardContactEpsM`), `bBoardPinned` | `CollideSkaterStatic` (the counter), the pin loop at the end of `Step` | `BoardFrames` >= 10 (`BoardPinFrames`) and a standing opponent not in the box within 1.2 m centre to centre (`BoardPinRangeM`); off while down or in the box; the pawn sends the change only | `Board_Pin` loop | skeletal: the lean held at its 10-degree floor; toy: the body pitch held 10 degrees |
| `PlayStickHit(bLifted)` | `StickHitResult` 1 (poke) / 2 (lift) on the carrier | `ResolvePokes` (a clean take from an opposing carrier), `TryLift` (a clean lift) | the frame the puck leaves the carrier; the poker's own `PlayPoke(true)` / `PlayLift()` still play | `Poke_Hit` / `Lift_Hit` | skeletal: the stumble bell (12 degrees, the yaw wobble, the stick 25 cm down for a poke, 25 cm up for a lift) over 20 frames, no jab; toy: the stick knocked 25 degrees aside (poke) or the blade lifted 20 degrees (lift) on the check curve |

The stay-up check B's first trigger names ("a check that lands and does not knock the victim down") has no sim state behind it: every landed check knocks down (section 2);
`PlayHitReaction` hangs on the second half of B's trigger, the collision. If the stagger proposal of section 2 ever lands (a weak check sets `StumbleFrames` instead of
`KnockFrames`), its trigger joins this call without a contract change.

Every call is logged once by the pawn: `JAMVIS hit frame=N skater=i team=t dir=(x, z)`, `JAMVIS board ...`, `JAMVIS pin ... on=0|1 board_frames=k`, `JAMVIS stickhit ... lifted=0|1`.
The frame is the bus's sim frame. B's `SK4KD` / `SK4UP` lines stay.

### 8.2 Bots check (R-0031 point 2; a calibration under R-0021 point 1)

`FJamBotSkill::CheckMinClosing` (-1 = never; **pro and all_star 5.0** = `game.skater.check_speed_min`'s default; rookie -1) and `bCheckFromBehind` (false everywhere); tune keys
`opp.check_min_closing`, `opp.check_from_behind`. In `FJamBrain::StickPlay`: a one-frame `bCheck` press (the sim re-arms its 6-frame window and plays `PlayCheck` on every frame
it is held, so the brain presses once and waits 30 frames) when all of these hold: the opposing carrier exists and stands, this bot stands, the carrier's centre is within
2 x 0.4 + 0.5 m (at 5 m/s closing that gap is the sim's 6-frame window), the closing speed along the line to the carrier and the bot's own speed along it are both >= 5 m/s
(the sim lands a check on the checker's own speed above `check_speed_min` while the strength is the closing speed), and the push along the carrier's facing is <= 0.35 (the
sim's `foul_from_behind` band is 0.5; the margin covers the frames to contact), never during a faceoff phase. `bCheck` is copied live into the brain's output like the shot
(a reaction-delayed press would miss the window). No bot presses on a skater without the puck, so neither of R-0013's fouls is ever earned by a bot's design.

**Grid (hockey.regular, 20 seeds, `tools/playtest/run.mjs`, before = main 1f964a9's binaries untouched, after = this lane; `tools/playtest/out/a45-before.*` / `a45-after.*`):**

| cell | win % before -> after | checks taken by the human per match | the human's checks landed | fouls (from behind / no puck) | idle s | 2-min goals of 20 |
|---|---|---|---|---|---|---|
| rookie / novice | 80 -> **80** | 0 -> 0 | 1.5 -> 1.5 | 0.9 (0.8 / 0.1) -> same | 10.2 -> 10.2 | 15 -> 15 |
| rookie / regular | 100 -> **100** | 0 -> 0 | 1.3 -> 1.3 | same | 0.7 -> 0.7 | 19 -> 19 |
| rookie / good | 100 -> **100** | 0 -> 0 | 1.6 -> 1.6 | same | 0 -> 0 | 19 -> 19 |
| pro / novice | 15 -> **10** | 0 -> 0.9 | 1.4 -> 1.5 | 0.7 (0.6 / 0.1) -> 0.9 (0.8 / 0.1) | 23.7 -> 34.7 | 5 -> 3 |
| pro / regular | 50 -> **35** | 0 -> 2.0 | 3.0 -> 3.5 | 1.2 (1.2 / 0) -> 1.2 (1.0 / 0.2) | 5.7 -> 8.8 | 13 -> 13 |
| pro / good | 80 -> **60** | 0 -> 3.5 | 3.1 -> 3.5 | 0.4 (0.3 / 0.1) -> 0.5 (0.4 / 0.1) | 1.7 -> 3.5 | 18 -> 15 |
| all_star / novice | 5 -> **0** | 0 -> 0.8 | 0.3 -> 0.3 | 0.2 -> 0.1 | 37.4 -> 52.5 | 2 -> 1 |
| all_star / regular | 15 -> **10** | 0 -> 1.6 | 1.2 -> 0.7 | 0.5 -> 0.3 | 14.1 -> 8.3 | 9 -> 8 |
| all_star / good | 35 -> **20** | 0 -> 2.9 | 2.2 -> 2.3 | 0.3 -> 0.2 | 7.9 -> 4.6 | 11 -> 9 |

Rookie is byte-identical (rookie never checks). At pro and all_star the human now takes 0.8-3.5 checks a match and the curve drops 5-15 points in every cell: pro / regular
50 -> 35 (R-0029 target about 50), all_star / good 35 -> 20 (target about 25; A39's 1.15 speed had it at 23 on 40 seeds). The fouls column is the human's own (unchanged
within noise: the bots' from-behind exclusion holds, `fouls_no_puck` stays at 0-0.2). **Reported, not tuned** (R-0031 point 2: lane A3's next pass re-measures); the knob to
move is `opp.check_min_closing` (a higher floor = fewer, harder checks) before any speed or skill number.

**Determinism baseline, re-recorded** (`Game/logs/a45-tests-1.txt`): `Jam.Core.Determinism` hockey.regular seed 1 frame 600 **`b705e2def86ae838`**, hockey.raw **`e09eebd2cebd1337`**:
unchanged from main, because that scene runs scripted inputs without the brain; the calibration moves every brain-driven hash instead: `Jam.Bot.VisualSwap` seed 1
**`df8392caa0c3924b`** on all three tracks (was `1871c1d348fa8b41`); `Jam.Bot.SeasonTakes` and `BundleKeepsRegularHash` agree between their own pairs as before.

### 8.3 Proof (PIE on 8025 from the worktree, 22:50-23:14; `tools/qa/v4.py`, bot-only through `jam.ScriptHuman 1`, pro bots, first to 9, dilation 0.5)

**Run 1, `hockey.v4.qa.json` = the figure-b1 cut (humanoid_b1, B's `BP_SkaterVisual_TB1`)**, 6000 sim frames, `Game/logs/editor-20261112-225027-0.log`: the pawn's calls fire from the
read-outs: `JAMVIS hit` 37, `board` 12, `pin` 20, `stickhit` 25 in the first 3300 frames (the human itself pinned at frames 482-540 with 29 board frames; a lift took the puck off
it at 498), and **the bots check**: `check who=4 kind=side_r strength=0.684` + `knockdown who=0` at frame 1872, again at 5903 (0.48) and 5995 (0.52), no foul on any. **B's figure did
not render**: with the four event names clashing against the interface the Blueprint's graphs fail to compile and the figure is absent from every capture (the pin strip
`img/tackling/a45-pin-human-f5248-strip.png` shows the boards, the puck disc and the spray at the human's pin at 5248 and no skater; eyes bug-hunt `eyes/a45-pin-b1-bughunt.json` reads
the disc as "a yellow conical debug indicator clipping into the dasher board" and does not notice the missing figures), so no `SK4KD` / `SK4UP` line was printed either. That is the
state until B's move of its events onto the interface; the calls, the triggers and the bot checks are proven by the log, the takes are proven when B re-hooks them.

**Run 2, `hockey.v4toy.qa.json` = the same cut on `toy_skeletal`** (the sim is deterministic, so the same match frame for frame; `tools/qa/out/a45toy/`): 1405 frames driven, 275
captures. **The bot check landing on the human**: orange 4 drives into blue 0 at the side boards, `check side_r 0.684` at 1872, `SKELVIS JamSkater_0 knocked down kind=side_r
strength=0.68 ... down_frames=90` (the v4 down time arrived), the burst, the toy tips to its right side and lies flat with the stick, `SetKnockedDown(false)` 90 frames later:
`img/tackling/a45-check-toy-f1872-strip.png` (frames 1869..1890 every 3rd). **The board pin**: blue 0 held on the side boards by orange, `JAMVIS pin frame=1693 on=1 board_frames=10`,
the figure holds its 10-degree lean into the boards until `on=0` at 1782: `a45-pin-toy-f1693-strip.png` (1690..1711). Around them on camera: `JAMVIS board` on 0 and 4 at 1490-1492,
`JAMVIS hit` on both at 1513 (the toy's 14-degree tip toward the push), `JAMVIS stickhit lifted=1` on the human at 1522 and 1552 (the stumble layer, stick up).

Eyes bug-hunt (gemini-3.8-flash, `--no-think`, about a quarter of a cent each; `eyes/a45-check-toy-bughunt.json`, `a45-pin-toy-bughunt.json`): on the check strip "the falling blue
skater's body and stick clip heavily into the ice, the head / torso below the ice plane in later frames" (severity 4): that is the side tip of the stiff toy about its skates (section 2:
88 degrees over 14 frames) seen from behind, the same reading A33 got on its behind hit; the strip caption text and a backdrop seam are the other two findings. On the pin strip "the
orange toy figure sinks into the ice up to its lower torso" (severity 4): the orange checker is pitched forward into the human against the boards (the hit tip and the board lean add
up on a figure that is already leaning), read from a low camera; worth a cap on the summed tips (a follow-up for the figure pass), not a trigger fault. Nothing in either strip is a
call that fired wrong or late.

Process: editor pid 38380 on 8025 only while proving (22:50-23:14, stopped by `editor.ps1 -Stop`, closed within the grace); one other UnrealEditor.exe on the machine (not mine, not
touched); never more than two editors plus my commandlets. The read-out test's pin scene needed its puck moved out of the net (A33's `ClearIce` parks it at `(Hx, 0)`, inside the goal
on the raw rink, which scores on the first step and holds the scene in the goal pause where the pin loop does not run); the rule itself pinned in play from the first run.
