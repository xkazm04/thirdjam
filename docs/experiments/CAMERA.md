# The chase camera at the boards and the far net (lane A40, device A, H12.8-H13.9, 2026-11-12)

R-0030 point 3 and FEEL proposal feel.07 (approved R-0021): every bake-off strip showed the chase camera at the glass with the board filling the frame, and the
puck does not read beyond 10 m. This page is the rule set the camera follows now, the three new `game.camera` keys with their defaults, the before / after
stills on the same sim frames, and what remains. Code: `Game/Source/JamGame/Public/CameraFollow.h`, `Private/CameraFollow.cpp` (the brief's `HokejCamera*`
is this file pair), tests `Private/CameraTests.cpp` (`Jam.Camera.*`), proof scene `CameraProofScene.*`. The camera is presentation: it reads the presentation frame,
the sim never reads it, and the match hash of the proof runs is the same with the old and the new camera (section 4).

## 1. The rules (one simulation frame of `UJamFollowCamera::StepThirdPerson`)

All of it runs once per 60 Hz sim frame on the sim's frame, never on DeltaSeconds; the render tick only interpolates between the last two poses with the sim's render alpha.
Same cut, same seed, same frames = same camera, at 30, 60 or 144 Hz (`Jam.Camera.CameraLagFrameBased`).

1. **The F1 rig, unchanged on open ice** (CAMERA-120M): the pivot and yaw are the skater's position and heading through the first-order lag filter (`lag_frames` 4); the
   camera wants to sit `distance_m` 6 behind, `height_m` 3.5 up, 0.7 m off-axis to the right, pitched at `-atan(height / (distance + look_ahead))` = -17.65 degrees.
   `Jam.Camera.BoardsClamp` part 4 checks that at centre ice the new step equals `ComputeThirdPersonPose` to 0.5 cm and 0.05 degrees.
2. **The boards box** (`boards_margin_m`, 0.8): the camera's position never leaves the rink's inner face pulled in by the margin on every side, **corner arcs included**
   (the inset region is the rounded rectangle with half extents `length/2 - m`, `width/2 - m` and arc radius `corner_radius_m - m`, at least 0; the presentation frame now
   carries `CornerRadiusM`). The desired camera point (the F1 arm) is replaced by the **nearest point of that region**: the camera slides along the glass, never through
   it. The projection onto a convex set is non-expansive, so the camera never moves more in a frame than the point it wanted to be at: the clamp itself cannot pop. (The
   first cut pulled the camera in along the arm toward the skater; that amplifies a 1.4-degree yaw step into a 2.6 m slide when the arm is nearly parallel to the glass,
   the lap test caught it, and it is gone.) If the skater himself stands in the margin band (his centre can be 0.4 m from the glass), the camera stands at the face right
   behind him.
3. **Rise and tilt, not clip**: the displacement the boards imposed over the arm length, `s` (0 free, 1 a whole arm), drives a lift from `height_m` to **`end_height_m`** (5)
   and a longer look-ahead (`look_ahead + 1.5 * s * distance`), so a camera pushed against the glass looks up the rink instead of down at the skater's back. The pitch uses
   the arm measured along the camera's forward axis, so open ice keeps the F1 pitch exactly. Both lift and yaw go through a **critically damped spring** (time constant
   8 sim frames, 95 % in about 38 frames = 0.63 s; `FJamSpring`, exact per-frame solution, never overshoots: `Jam.Camera.SpringCriticallyDamped`).
4. **Along the rink at the end boards**: within the last `max(arc radius, 3 m)` before an end face the yaw turns toward the rink axis, away from that end board
   (0 degrees at the -X board, 180 at the +X board), by `s * w_end * w_heading * (axis - yaw)` where `w_end` is a smoothstep over that band and `w_heading = cos(axis - yaw)`
   clamped to 0..1. The last factor matters: a skater skating *across* his end zone (behind the net) keeps his own framing, a skater leaving the end zone at an angle is
   framed down the rink with the far net in view. At a side board the rule is off (`Jam.Camera.EndBoardsLift`: end board yaw 2.6 with the skater at 35, side board stays
   at the skater's yaw, crossing the end zone stays within 5 degrees of the skater's -90).
5. **Far-net bias** (`far_net_bias`, 0.5): the yaw leads toward the net the human attacks by `bias * toward * band * (yaw_to_net - yaw)`, with `toward = cos(angle between
   heading and the net direction)` clamped to 0..1 (nothing when skating away) and `band` fading in from 8 m to 20 m from the net (nothing in the slot, where the direction
   swings). The lead goes through the same yaw spring as rule 4. At 26.8 m, 26.6 degrees off the net, the lead is 11.9 degrees and moves the net from 745 px to about
   290 px off the frame centre; the net and the slot are inside 1920 x 1080 (`Jam.Camera.FarNetBias`). It does not make the puck larger: a 24 cm puck at 24 m from a
   camera 6 m behind the carrier is 2 px at 1080p whatever the yaw (CAMERA-120M section 1); the bias keeps the net and the slot in frame, the marker and the trail carry the puck.
6. **Arena mode** is untouched. `jam.camera.Legacy 1` (a cvar, presentation only) restores the H11 behaviour (rectangle pull-in to 0.3 m, nothing else) for A/B stills.

Order inside the step: lag filter; lead (rule 5); the arm with the spring's current yaw offset gives the targets of rules 3 and 4; the springs step; the arm is placed again with
the new yaw so position and rotation agree; the clamp; the lift and the pitch. Everything is `float`/`double` arithmetic on the frame's values; no world, no clock.

## 2. The keys (`game.camera`, schema/game.schema.json; defaults in schema/DEFAULTS.md; absent = default)

| Key | Default | Range | What |
|---|---|---|---|
| `boards_margin_m` | 0.8 | 0..5 | the camera never crosses the boards' inner face minus this, on any side, corner arcs included |
| `end_height_m` | 5 | 1..15 | the height the camera lifts to when the boards take the whole arm (proportional to the arm lost) |
| `far_net_bias` | 0.5 | 0..1 | how much of the angle to the far net the framing leads by when the skater heads for it (0 none, 1 aim at the net) |

Constants of the recipe (in `FJamCameraSettings`, not keys): spring 8 frames, lifted look-ahead gain 1.5 m per metre of arm lost, bias band 8..20 m, off-axis 0.7 m. The keys
are read by JamCore's loader (`FJamCameraConfig`, `JamCartridge.cpp`), the embedded schema (`JamSchemaData.inl`) and `parity.json` are regenerated, `limits.json` mirrors the
ranges; 95 schema tests pass. No shipped cartridge sets them (the defaults are the measured values); the Designer may not change the camera (unchanged rule).

## 3. Method of the proof

- Worktree a40 at main 969fbc3, `ThirdJamEditor Development`, unity off, built with no other UnrealBuildTool alive; one editor on MCP port 8022; never more than three
  `UnrealEditor.exe` on the machine (three foreign ones ran at 19:41; mine started when they had left).
- `tools/qa/cam40run.mjs`: `jam.camera.Legacy 0|1`, `jam.ScriptHuman 1`, `jam.Cut hockey.regular`, the harness `tools/qa/cam40.py` registered **before** StartPIE (a slate
  post-tick that reads the presentation frame and fires `HighResShot 2560x1440` at exact **sim frames**, with the world's time dilation at 0.5 from 90 frames before the first
  shot so no sim frame is skipped), one row per sim frame (human, camera position, yaw, pitch, distance to the boards), `cam40stop.py` before StopPIE.
- Frames: the bake-off's frozen frames **60** (faceoff), **222 / 223** (A26's play frame), the strip **500, 650, 800, 950, 1100, 1250, 1400, 1550** (eight captures over 17.5 s
  of the same bot match, the span the a30 strips used), and **1127**, the legacy pass's worst frame (the human 0.6 m from the +Y glass). Both passes shoot the same list, so
  every pair is the same sim state; the render alpha differs by less than a frame.
- Eyes (R-0024 point 2, advice not instructions): `bug-hunt` on both strips, `readability` on the 1127 pair at `--distance 6`; JSON in `docs/experiments/eyes/a40-*.json`.

## 4. Before and after (filled by the proof pass; stills in `docs/experiments/img/camera/`)

Both passes: `hockey.regular`, scripted human, 1551 sim frames each, the same sim on every frame (the human's position differs by 0.0 m on all 1551; `JAMSIM frame=600
hash=034316ec7ef27999` and `frame=1200 hash=6dc4a35bf5be202c` in both logs). Per-frame camera rows: `tools/qa/out/cam40-{legacy,clamp}/rows.jsonl`.

| Measure over the 1551 frames | legacy (H11) | A40 |
|---|---|---|
| nearest the camera came to the boards' inner face | **0.30 m** (frame 1124) | **0.80 m** (frame 1098), never less |
| frames with the camera inside the 0.8 m band | 173 | 0 |
| camera height | 1.01 to 3.50 m (the old pull-in sinks toward the shoulder) | 3.50 to 4.64 m (lifted at the glass) |
| frames within 1.5 m of the glass | 203 | 213 (it slides along the face instead of sinking into it) |
| largest camera move in one frame outside the faceoff reset | 2.52 m (frame 1358, the old pull-in sliding along the arm) | 0.70 m (frame 866; yaw steps under 6 degrees a frame in both) |

(The faceoff reset at frame 562 teleports the human 33 m; the lag filter follows it over two frames, 4.27 m and 28.6 degrees in both runs: the sim's doing, not the clamp's.)

Stills, 2560 x 1440 JPEG, `a40-legacy-fNNNNN.jpg` / `a40-clamp-fNNNNN.jpg`, and the 4 x 2 strips `a40-{legacy,clamp}-strip.jpg` (frames 500, 650, 800, 950, 1100, 1250, 1400, 1550 at 1280 px):

| Frame | What the sim shows | legacy | A40 |
|---|---|---|---|
| 60 (the bake-off's faceoff) | the human at (-7.5, 0) facing the far net | identical: camera (-13.5, 0.7, 3.5), yaw 0 | identical to the pixel (no lead: he already faces the net) |
| 222 / 223 (A26's play frame) | the human at (-7.1, 3.2) turning, 48 m from the net | yaw -37.0 / -32.2 | yaw -31.5 / -26.1: the far-net lead (5.5 and 6.1 degrees) turns the framing toward the net the carrier is turning to; same position within 0.6 m, same height and pitch |
| **1127** (the legacy pass's worst) | the human 0.6 m from the +Y glass, heading +X, the checker beside him | camera 0.3 m from the face at 2.84 m: the rail and the glass fill the right third, the far net is cut at the left edge, the blue jersey fills the middle | camera 0.8 m inside at 3.90 m, yaw -12 (lead toward the net): the far net with its goalie top-left, the slot, both figures and the glass on the right, all in frame |
| strip 500-950 | open ice (faceoff scrum, centre ice, the chase) | | the same frames to the eye (the lead moves 950 by a few degrees) |
| strip 1100 | the carry along the +Y glass | the camera between the stanchions, the rail across the frame | higher and inside: the far net top-left, the glass on the right |
| strip 1250 / 1400 | the rush and the crease | the crease from 3 m up | from 4 m up, more ice ahead |
| strip 1550 | the carrier at the +Y glass heading -X | the camera at the glass with the rail at the bottom of the frame | lifted above the dasher, the glass panel and the ice ahead in frame |

Eyes (gemini-3.8-flash, `--no-think`, three calls, about one cent; `docs/experiments/eyes/a40-*.json`; advice, not instructions):

- `readability` on the 1127 pair at 6 m: **A (legacy) overall 3, B (A40) overall 4**. Far net **2 to 4** ("cut off at the extreme upper-left edge of the frame, showing only a
  sliver of red post" against "fully visible in the upper-left ice area as a red rectangular frame about 160 pixels wide"); puck 4 / 4 (a 20-25 px disc with the glow ring),
  carrier 4 / 4, team colours 5 / 5, zone strips 4 / 4. Noise on both: the emissive trim strip and the yellow bench through the glass (the look, not the camera).
- `bug-hunt` on the legacy strip: four findings, all figures (the blue centre through the net post at the faceoff, the orange pair intersecting at the crease, the collision
  at 950, a figure cut by the frame edge at 1250): nothing on the camera this time, where the a30 strips drew "camera inside the boards" at severity 4; the a30 strips were
  1280 px viewport captures with HUD and motion blur, these are clean HighResShots, so the two readings are not the same instrument.
- `bug-hunt` on the A40 strip: the same net-post clip at 500 and a blank number plate at 800 (figures); **"uncapped geometry showing hollow inner faces beneath the player bench
  or dasher board structure outside the glass" at 1100, severity 3, confidence 0.95**: true, and new: the lifted camera looks over the dasher at the bench construction's
  open back, which the old camera at 2.4 m never saw (a construction finding for lane A25 / A31, not a camera fault); and "blue skater standing on top of the dasher board
  cap rail" at 1550, severity 5: false, the skater is 0.6 m inside the boards and the camera 4.6 m up looks over the rail at him (the perspective the preset cannot read).

What I see myself (the two strips side by side): frames 500-950 are the same picture; at 1100 and 1550 the old camera sits in the glass with the rail across the lower frame
and the jersey filling the middle, the new one is a metre inside and a metre higher with the far net or the ice ahead in view; at 1250 and 1400 the new view is slightly
higher and the crease reads the same. The trade: from the higher camera the construction behind the glass (bench backs, stanchion bases) is in frame more often.


## 5. What remains

- **A replay camera.** The sim is deterministic and the presentation frame is all the camera reads, so a replay is a second camera driven by the recorded frames; the clamp
  and the springs are pure functions of the frame stream (`StepThirdPerson(C, State, Frame)`) and work unchanged on a replay. Not built: no frame recorder exists yet.
- **The Director's Call framing.** When the Call changes the rink (ShrinkRink / GrowRink) the frame's rink size follows at the faceoff and the clamp follows it; a camera
  move that shows the change (an arena shot for 60 frames, then back to the chase) is a mode blend the step does not have. The arena pose exists; the blend is a spring
  on a mode weight, the same shape as the lift.
- **What "never" costs.** A skater with his back to the glass facing the rink is framed from a camera at the face right behind him, lifted, looking up the rink: he is
  at the bottom edge or below it for the half second it takes him to leave. The alternative (letting the camera cross the boards and fading the board mesh) was not tried;
  it would need the rink's construction to render inside-out and is a look question for B.
- **Controls are still camera-relative through the camera manager** (`AHokejGameMode::ReadHumanInput`, FEEL bug 4): with the end-board rule the camera yaw can differ from the
  skater's by up to 30 degrees, so a human's stick maps differently at the end boards than before. The scripted human and the bots never read it (the proof's hash is
  unchanged); the fix named in FEEL (map to the skater's yaw or to the camera's sim yaw) belongs to the game mode's owner.
- **Lockstep matches** (`LobbyGameMode`) do not fill `CornerRadiusM` on their frames yet (lane A38's files), so their clamp treats the corners as square (generous by
  up to `r (sqrt 2 - 1)` = 1.55 m at a 3.75 m arc); one line when that lane is done.
- The far-net marker (HUD) and the puck trail still carry the puck beyond 10 m; the bias frames the slot, it cannot enlarge a 2 px puck. A longer lens (FOV 60) when the
  carrier is in the far third would (puck 17-21 px at 15 m, CAMERA-120M), at the cost of the flank view; not tried here.
