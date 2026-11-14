# Lane A186, track U1 (the camera), evolution 7 (device A, Opus)

You are lane A186 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then **R-0064** at the end of `contracts/RULINGS.md` (the operator's words after playing package H31; point 4 is your charter), the track's
`docs/experiments/CAMERA.md`, `CAMERA-120M.md`, `SCREEN-2.md` (the pitch cap of last night, and why its second reading did nothing), `CameraFollow.cpp/.h`
and `CameraTests.cpp`.

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A186.md`. Port 4786. Family:
`CameraFollow.cpp/.h`, `CameraTests.cpp`, the camera's cvars, `tools/qa/screen/**` (the capture driver and its camera log), `docs/experiments/CAMERA-3.md`.
Presentation only: no sim state. You do not own the cursor: off screen with the track's driver.

## Evolution 7: the camera at the boards moves only as the player does
His words, the first of his weakest points: "Camera around arena borders is autocorrecting unnaturally making hard to recalibrate". He steers the stick
with the mouse on the ice: when the picture moves under his cursor without his doing, his aim is lost: that is what "hard to recalibrate" means.
1. **Find the corrections** (measurement first, 30 minutes): in captured matches with the human skating along and into the boards and the corners (the
   scripted human, or the camera model fed with recorded positions): the camera's position, pitch and field of view per frame against the player's own
   velocity: every stretch where the picture moves faster than the player does or against him: the keep-in-frame stage, the board push-in, the corner
   squeeze, last night's pitch cap (`jam.camera.PitchCap` 55), the look-ahead, any snap or re-centre. One table: the stage, how often a match, the
   worst screen-space speed of a fixed point on the ice (pixels a frame at 1920 x 1080) with the player standing still.
2. **The rule**: near the boards the camera's own motion is bounded: a fixed point on the ice never moves on screen faster than K pixels a frame from
   the camera's corrections alone (the player's motion is his own), no reversal of direction inside half a second, and **the cursor's point on the ice
   stays under the cursor** through a correction if that is cheaper (the aim is re-projected each frame: check what the stick's aim does today when the
   camera moves: if the aim is held in screen space and the camera corrects, the stick swings on its own: that alone may be his complaint: say). The
   player still never leaves the frame (the track's own test), no yaw.
3. **Last night's pitch cap**: keep it only if it passes the new bound; else `jam.camera.PitchCap` 0 by default and say so.
4. **Proof**: the table of item 1 before and after on the same matches; the model's tests; six stills along one boards run with the cursor's ice point
   marked; a 10-second off-screen frame sequence along the boards and through a corner for the Director to look at (frames as files).
5. If time remains: the attacked net out of frame inside the blue line when he works the boards (the review's old finding): a bias toward the net
   that obeys the same bound.

Write `CAMERA READY` with the file list when items 1 to 4 are closed, built, the full `Jam` filter green and Shipping compiles: **wanted by 14:45 for the
15:30 package**; `SOURCE OPEN` before any later source change. `-Multiprocess`; stop only processes whose ids you recorded. Time box 130 minutes.
