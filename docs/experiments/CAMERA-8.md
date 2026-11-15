# The camera, part 8: at the boards he sees ice and the play (lane A214, track U1, evolution 8, the night of 15 Nov)

EYE LIST 1 item 5 (lane A211, 00:42): "Near the boards the screen shows wood and crowd, or the top of his own helmet." 42 stills of 96 in a rookie
match; 10 flips to a top-down view in two minutes. Presentation only: no sim state, no cartridge key, the lockstep hash untouched.

## Why seven rounds of tuning did not cure it

The yaw is the skater's, and rightly (lane A186, R-0064 point 4: W and the cursor's blade are read through the camera, so a camera that turns at the
boards turns his hands). With the yaw fixed, every round answered the boards with **height**:

| Round | Knob | What it does at the boards |
|---|---|---|
| A40 | end-board lift (height to `end_height_m`) | 3 m, under the rig's 3.5: inert since A49 |
| A111 | corner lift +3 m, look-ahead cut to 20 % | the camera goes UP when he faces a board: from 6.5 m the eye sees over the boards into the stands |
| A135 | keep-in-frame crane: the arm's length kept by height | his back at the glass takes the arm, the crane stands the camera over his head: the straight-down view |
| A158 / A171 | pitch cap 55 | kept "never under the open-ice height, never nearer than 5 m": it could not bite where it was needed; off since A186 |

Geometry, the clamped rig of the game (`tools/qa/camera/geom.py`, the share of a 16:9 frame that is ice inside the boards' inner line):
facing a board 1 m off from the rig's 6 m behind him, height 3.5 m and pitch 30 degrees shows 58 % ice; the corner lift's 6.5 m and 43 degrees shows 50 %.
**A higher camera sees more of the stands, not less.** The knobs that were never turned: come **down**, come **in** along the arm, **slide** the camera's
foot into the rink, and tilt by the ice actually ahead of the camera.

## The design (`FJamCameraSettings::BoardsView`, `jam.camera.Boards`, default 1)

On the open-ice rig's pose (the yaw untouched, to the bit):
- **slide**: each board within 6 m pushes the camera's foot into the rink along the camera's own right (3 m at 1 m from a board at his side); a board
  ahead or behind slides nothing; in a corner the two pushes add to the diagonal (no switch at the bisector);
- **pull in**: where the boards take the arm the foot comes in along the line to the point over him, never sideways along the glass; 0.4 m inside the line;
- **come down**: from 3.5 m to 2.0 m as the arm shortens under 4 m (an over-the-shoulder look with his back at the glass; `jam.camera.BoardsMinHeight`);
- **tilt**: no flatter than the rig, down until the boards' foot straight ahead is no higher than 45 % of the half frame over the centre; his head-to-chest
  and his blade's spot 0.9 m ahead kept inside 85 % of the half frame; never steeper than 55 degrees;
- **gate**: the correction is weighted by the nearest board (all of it inside 4 m, none beyond 8 m: there the pose is the rig's to the bit);
- **no pop**: the correction moves at most 0.25 m of foot, 5 cm of height, 5 degrees of pitch a sim frame (where the arm runs along the glass the
  pulled-in point slid 2.6 m in one frame as he turned); where that limit would lose him, the least share of the rest of the way that keeps him.
With it on, the corner lift (A111) and the keep-in-frame stage (A135) step aside. Neutral value: `jam.camera.Boards 0` (main's camera).

## Results

### The model (`Jam.Camera.BoardsSweep`, `Jam.Camera.BoardsPaths`, pure `StepThirdPerson`; both green)

| | Before (main's camera) | After |
|---|---|---|
| Sweep 90 m, 8,760 settled poses (0.5 to 7 m from the boards, every 4 m of the line, 24 facings): ice < 55 % / < 40 % / min | 28.6 % / 3.6 % / 0.23 | **0.3 % / 0 / 0.53** |
| same: pitch > 55 (steepest) | 21.6 % (86.0) | **0 (50.4)** |
| same: him off screen / 3 m ahead off / eye outside the line or in the glass | 0 / 0 / 0 | 0 / 0 / 0 |
| Sweep 120 m, 12,072 poses: ice < 55 % / < 40 % / pitch > 55 | 27.2 % / 2.7 % / 20.9 % | **0.2 % / 0 / 0** |
| Staged scenes 90 m (5,380 frames: side boards, four corners, behind both nets, standing in a corner facing out): ice < 55 % / pitch > 55 | 34.5 % / 16.7 % (79.7) | **0 / 0 (36.2)** |
| A135's boards path 90 m (6,000 frames): ice < 55 % / pitch > 55 / him off | 13.3 % / 5.3 % / 0 | **0 / 0** / 0.3 % |
| Largest pitch step a frame (staged 90 m) | 10.0 deg | 3.1 deg |
| Largest camera step a frame (staged 90 m / A135 90 m) | 0.90 / 1.03 m | 1.30 / 1.69 m (the guard) |
| Frames whose yaw differs from main's | - | **0** (W and the blade untouched) |
| Open ice (> 8 m from every board): frames off the open-ice rig | (main: 229 of 3,948 off the rig: the corner lift's spring tail) | **0** |

### Real matches (`tools/qa/camera/cam-match.ps1`, 5,400 sim frames, stills every 20; pose geometry by `cam-log.py`, pixels by `view.py`)

Sim hashes before and after are equal in every pair (presentation only).

| Match | Stills | Ice < 55 % | Ice < 40 % | Pitch > 55 (max) | Within 5 m of a board: ice < 55 % | Pixels: stands + no horizon |
|---|---|---|---|---|---|---|
| rookie 90 m, old script (`ScriptHuman 1`, hugs the boards) before | 271 | 26.9 % | 10.7 % | 4.1 % (80.0) | 73 % of 97 | 99 of 270 (eye cadence f500-4300/40: 38 of 96) |
| same, after | 271 | **1.1 %** | **0** | **0 (55.0)** | **3 % of 97** | **26 of 270 (11 of 96)** |
| rookie 90 m, person hand (`ScriptHuman 2`) before | 277 | 3.2 % | 1.1 % | 1.4 % (77.4) | 33 % of 24 | 28 of 270 |
| same, after | 272 | **0** | **0** | **0 (40.9)** | **0 of 24** | **10 of 270** (6 are open-ice looks at the far stands, same before) |
| pro 90 m, person hand before / after | 271 / 272 | 2.2 % / **0** | 0.4 % / **0** | 0.4 % (60.3) / **0 (32.5)** | 35 % / **0** of 17 | 19 / **2** |
| rookie 120 m (large cut), person hand before / after | 272 / 272 | 4.4 % / **0** | 0.7 % / **0** | 0.7 % (64.7) / **0 (32.8)** | 42 % / **0** of 26 | 30 / **6** |

Stills: `camera-8/01-scenes-before-after-rookie-s1.jpg` (twelve scenes, before left, after right: corners, behind the net, side boards, two
hits at the boards, two faceoffs), `camera-8/02-flagged-stills-before-after-rookie-s2.jpg` (the stills the pixel tool still flags).

## Not at the bar yet

- **His back at the glass**: the yaw may not turn, so the camera can only stand over his shoulder; at 2.0 m his helmet fills the bottom of the frame
  (02-flagged, f1040, f4200). 2.3 m and 2.6 m lose him under the frame at the 55-degree cap (2.9 % and 4.5 % of the sweep). About 1 % of a person's stills.
- **The guard's step**: where the limited move would lose him, the camera moves up to 1.7 m in one frame (before: 1.0 m).
- **3 m ahead**: 1.2 to 1.5 % of the A135 path's frames, 8 % of a person's board stills (he skates straight at the boards).
- **The puck**: the bar counts a puck within 20 m; behind him it is off screen in 10 to 17 % of stills, before and after alike (not a boards matter).
- The net between eye and chest: 28 frames of the staged path, all of them the path walking him through the net box (the same before).
