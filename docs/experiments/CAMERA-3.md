# The camera, part 7: at the boards the camera no longer turns on its own (lane A186, track U1, evolution 7, Sat 14 Nov 12:32 to 13:25)

R-0064 point 4, the operator's first weakest point after playing H31: "Camera around arena borders is autocorrecting unnaturally making hard to
recalibrate". Presentation only: no sim state, no cartridge key, the lockstep hash untouched (the three real-match runs below carry the same `JAMSIM`
hashes: `8e0fd36aeea524e2` at sim frame 600, `02b5afd69a5fa672` at 1200, `ebe03901d77c82a3` at 1800).

## What the stick does when the camera corrects (read first)

The cursor is shown and locked to the viewport (`AHokejGameMode::EnsureMatchInputMode`). Every sim step `AHokejGameMode::CursorAimRink` takes the
cursor's **screen** position, casts it through the **current** camera onto the ice (`CursorAimFromRay`) and points the blade from the skater to that
point. The aim is held in screen space and re-projected every frame. A camera that follows him rigidly keeps the cursor's ice point fixed relative to
him, so the blade holds. A correction (a turn, a lift, a tilt, a push-in that is not his own motion) slides the ice under a still hand, and **the blade
swings with no hand movement**. Worse, the move keys are read in the camera's yaw (`ReadHumanInput`), so a camera that turns also turns W. That is
"hard to recalibrate". `IA_StickMouse` (accumulated mouse delta) is only the fallback when the cursor is hidden.

Measured with the game's own `CursorAimFromRay`, the cursor held still over a point 2.5 m ahead-right of him, the blade's turn the camera adds:

| Where | A still correction costs the blade (before) | After |
|---|---|---|
| Model, 90 m, A135's boards path (6,000 frames) | median 15.3 deg, p90 62.0 deg; up to 10.8 deg in one frame | median 6.5, p90 15.8 deg |
| Model, scripted boards run (side board, corner, stop with his back to the glass) | 59.5 deg | 11.5 deg |
| Real match, seed 1, 90 m (5,114 common sim frames) | worst still correction 46.9 deg | 14.8 deg |

**So the re-projection is the mechanism and the camera's corrections are the trigger; the largest trigger was the yaw.** Before this lane the boards
turned the camera on 1,133 of 5,114 frames of the real match (up to 58 degrees off the rig, 2.6 degrees a frame): every one of those frames turned his
blade and his W key with no hand movement.

## The rule shipped

| Rule | Where | Default | Neutral value (the camera of before) |
|---|---|---|---|
| **The boards never turn the camera.** The yaw is the rig's (the skater's yaw through the lag, plus the far-net lead): no turn along the rink at an end board (lane A40's `S0 * WEnd * WHead * DeltaAlong`), no middle-third yaw budget (lane A49). The keep-in-frame stage holds him by position, height and pitch (`KeepNoYaw 2`, unchanged). | `FJamCameraSettings::bRigYaw`, `StepThirdPerson` | `jam.camera.RigYaw 1` | `jam.camera.RigYaw 0` |
| **Last night's pitch cap off.** With the turn gone, the cap only added corrections while he stood (model, 140 m: still frames over 3 px 0 -> 4, worst 1.1 -> 5.0 px) and never removed one. | `CVarJamCameraPitchCap` | `jam.camera.PitchCap 0` | `jam.camera.PitchCap 55` |
| The rate bound (opt-in, **not shipped**: measured not better, below). | `FJamCameraSettings::BoundPx`, `BoundOwnGain`, `BoundGuardFrac` | `jam.camera.Bound 0` | (off) |

## The corrections, stage by stage (the instrument)

`Jam.Camera.BoundsCorrections` (pure `StepThirdPerson`, no world): A135's modelled skater on three rinks (the seeds of `Jam.Camera.KeepsHuman`) and a
scripted boards run. Every frame, 16 ice points fixed to him (2.5 and 5 m around him) are projected through the camera and through the open-ice rig of
the same frame (`FJamCameraFilter::LastRigPose`). The difference in their motion is the boards' correction, in pixels a frame at 1920 x 1080. **Still**
means he neither moves nor turns that frame. Rows are cumulative, top to bottom; "how often" is per 6,000 frames (100 s of play).

| Stage (cumulative) | 90 m: still frames over 3 px (of 1,389) | 90 m: worst px/frame, still | 90 m: frames the boards turn the camera | 140 m: still over 3 px (of 1,566) / worst | 120 m: still over 3 px (of 819) / worst | boards run: still over 3 px (of 232) / worst |
|---|---|---|---|---|---|---|
| clamp, end lift, end-board turn (A40), pitch budget (A49) | 119 | 89.8 | 787 (worst 3.45 deg/frame) | 32 / 24.0 | 13 / 35.1 | 102 / 25.7 |
| + corner lift 3 m (A111) | 152 | 83.0 | 787 | 32 / 24.0 | 13 / 35.1 | 102 / 25.5 |
| + keep-in-frame (A135: crane, step, slide) | 142 | 92.7 | 787 | 29 / 27.1 | 13 / 32.7 | 105 / 30.5 |
| + pitch cap 55 (A158): **the game before** | 144 | 87.6 | 787 | 29 / 27.1 | 13 / 32.7 | 105 / 30.5 |
| **rig yaw, cap 0: the game after** | **58** | **36.1** | **0** | **0 / 1.1** | **8 / 28.6** | **19 / 40.7** |
| rig yaw, cap 55 | 59 | 37.2 | 0 | 4 / 5.0 | 8 / 28.6 | 20 / 40.7 |
| rig yaw + rate bound 3 px (opt-in) | 83 | 71.0 | 0 | 0 / 2.6 | 11 / 36.1 | 20 / 32.2 |

What holds after: the chest never leaves the frame (0 frames on every path), the chest is outside the middle third on exactly as many frames as before
(13, 0, 7, 0), the camera never crosses the boards' inset, and open ice is the rig of before to the bit (720 frames, both attack directions).

**The real match** (seed 1, 90 m, clay, the scripted human; `u5-downs.ps1 -CamLog`, `tools/qa/screen/u7-bound.py --common`, 5,114 common sim frames):

| | Frames the boards turn the camera | Still frames over 3 px (of 591) | Worst still px/frame | Blade, worst still correction | Frames looking down steeper than 60 deg |
|---|---|---|---|---|---|
| before (`RigYaw 0`, cap 55) | 1,133 (up to 58 deg, 2.64 deg/frame) | 94 | 7.8 | 46.9 deg | 71 |
| after (`RigYaw 1`, cap 0) | **0** | **17** | **6.2** | **14.8 deg** | 114 |
| rig yaw with cap 55 | 0 | 17 | 6.2 | 14.8 deg | 100 |

## What it costs, and what is left

- **Corners are seen from higher**: frames steeper than 60 degrees 71 -> 114 in the real match (100 with the cap). The end-board turn used to win arm by
  looking along the rink; without it the keep stage lifts the crane. This is the critic's fault 11 coming back in part. The answer is not the yaw
  (a turned camera turns his hands). It is the arm: a camera allowed behind the end glass (device B: the arena must be safe for a camera there), or
  a lower crane that holds him by position.
- **The worst still frame of the scripted boards run rose** 30.5 -> 40.7 px (one frame, the keep stage's step as he turns his back to the end glass and
  stops). Still frames over 3 px fell 105 -> 19 on that run.
- **The rate bound did not pay** (three designs, all measured). (1) Against the open rig, it lagged the glass and the guards snapped: worst 885 px. (2) With
  momentum, it carried speed past his stop: 46 to 91 px. (3) As a pure rate limit while he stands, the pivot's own settling after a stop outran it and the
  chest drifted to the guard: 71 px on 90 m against 36 without it. The keep stage's ask changes while he settles: a bound has to sit inside that
  stage (on its crane and step), not after it. That is the next evolution.
- **Not proven off screen**: how the steadier camera feels in a hand at the boards, and whether the higher corner view reads worse to an eye than the
  turn it replaces. One number gives the camera of before: `jam.camera.RigYaw 0` (and `jam.camera.PitchCap 55` for last night's cap).

## The stills and the frame sequence

The same match before (`RigYaw 0`, cap 55) and after, seed 1, 90 m, clay, the scripted human. Sim frames 2222 to 2824 take him along the side board
and through a corner (the stretch with the most board turns in the before log: `u7-stills.py find`). Every second sim frame was captured off screen
(`u5-downs.ps1 -StillEvery 2`; the two runs carry the same `JAMSIM` hashes). The marks are drawn from the camera log (`u7-stills.py mark`):

- the **white cross**: the cursor, held still for half a second, put 30 frames earlier over the ice 2.5 m ahead of him (inside the boards);
- the **orange ring**: that ice point carried with him (where the cross would sit had the camera only followed him);
- the **yellow line**: the blade the game reads from the held cross now (`CursorAimFromRay`);
- the **label**: the blade's turn over that half second that the camera added (the camera's turn minus the open-ice rig's).

Over the whole stretch (294 frames, he skates most of it), the half-second turn the camera adds goes from median 9.5, p90 34.2, worst 99.1 deg to median 8.3,
p90 38.1, worst 57.4 deg. While he skates along the glass the open-ice rig would stand past it, so these figures carry the glass's own push, which
this lane does not remove; the still-frame figures above are the measure of "the camera moves on its own".

Six stills, `docs/experiments/img/a186/` (before left, after right). The six frames are those with the largest drop in that figure, at least a
second apart; they are not a sample of the stretch:

| Still | Before -> after (deg the camera turned the blade in half a second) |
|---|---|
| `boards-f02298-before-after.jpg` | -62.1 -> -9.9: into the corner; before, the end-board turn swings the picture; after, the camera is higher (the cost above) |
| `boards-f02380-before-after.jpg` | 11.5 -> 0.5 |
| `boards-f02536-before-after.jpg` | -24.5 -> -1.6: along the side board |
| `boards-f02650-before-after.jpg` | -7.3 -> -4.3 |
| `boards-f02724-before-after.jpg` | -36.0 -> -36.0: unchanged (the glass's push as he turns along the board) |
| `boards-f02806-before-after.jpg` | 99.1 -> 57.4 |

The frame sequence (10 s, every second sim frame, before | after side by side, 294 files): `tools/qa/screen/out/a186-seq/`
(`boards-fNNNNN-before-after.jpg`), and the same as a 30 fps video, `tools/qa/screen/out/a186-boards-before-after.mp4`. The raw stills and the
marked frames are in `tools/qa/screen/out/a186-stills-{before,after}/seed1/shots/` and `a186-marked-{before,after}/` (with `added.csv`, the figure per frame).

## How to run it

```
UnrealEditor-Cmd ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Camera.BoundsCorrections; Quit" -nullrhi -Multiprocess -unattended   (JAMBOUND lines)
powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u5-downs.ps1 -Mode after -Seeds 1 -StillAt 0 -CamLog -PlaySeconds 100 -RunName a186-before -ExtraCVars "jam.camera.RigYaw=0,jam.camera.PitchCap=55"
powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u5-downs.ps1 -Mode after -Seeds 1 -StillAt 0 -CamLog -PlaySeconds 100 -RunName a186-after
python tools\qa\screen\u7-bound.py --common tools\qa\screen\out\a186-before\seed1\game.log tools\qa\screen\out\a186-after\seed1\game.log
python tools\qa\screen\u7-stills.py find <game.log>        (the boards stretches)
python tools\qa\screen\u7-stills.py mark <game.log> <shots> <out> --from F --to T --every 2 --label after
```
The camera log line `JAMCAM` gains `rx ry ryaw` (the open-ice rig's pose), `bg` (the opt-in bound's guard) and `fyaw fpitch fcx fcy fh` (the pose at
full precision); old parsers read on (the fields are appended).
