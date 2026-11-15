# Lane A214, track U1 (the game camera), the night: near the boards he sees ice and the play, never wood, crowd or his own helmet (device A, Opus)

You are lane A214 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
Then **R-0064** (the operator's 12:20 notes of 14 Nov: the camera at the boards was his first point), **R-0067 and R-0068** at the end of
`contracts/RULINGS.md`; the track's own history (`status/A186.md`, `docs/experiments/` of the camera's evolutions 1 to 7 as they stand on main);
`Game/Source/JamGame/Private/CameraFollow.cpp` and its header with every `jam.camera.*` cvar; and item 5 of EYE LIST 1 in
`.claude/worktrees/a108/status/A211.md` with its stills (`.claude/worktrees/a108/docs/experiments/eye/06-camera-on-the-stands-f3860-4300.jpg`,
`sheets/r1w-f1506-1594.jpg`, and the list under `FOR A214` when the eye has written it).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A214.md`. Port 4816. Family:
`CameraFollow.cpp/.h` (lane A210 had it for one night's item and has given it back; if `C:\Users\kazda\AppData\Local\Temp\tj\a210-camera.patch`
exists, read it first), the camera's tests `Jam.Camera.*`, `tools/qa/camera/` (new; copy the screen track's drivers, do not edit theirs),
`docs/experiments/CAMERA-8.md`. Presentation only: the sim's hash untouched. Off screen with `-dpcvars=r.RDG.AsyncCompute=0`; you do not own the
cursor. **One heavy process at a time** (the machine is at 76 % of memory with eight lanes). Nothing of yours runs from the main checkout.

## What a stranger saw tonight (the eye, 00:42)
"Near the boards the screen is wood and crowd or a straight-down view of his helmet; holding the puck behind his net he sees the audience." 42
stills of 96 in a rookie match (the script hugs the boards: a person would see about one in five), 10 flips to a top-down view in two minutes.
The track has lifted, capped and bounded this camera seven times (corner lift 3 m, the pitch cap off since A186) and it is still his first
complaint. Do not tune the same knobs an eighth time before you know why they lose.
1. **Measure as the eye does** (first 30 minutes): a rookie match and a pro match on the regular rink and one on the large rink, the game camera,
   a still every 20 frames, plus staged scenes: he carries along each side board, into each of the four corners, behind each net, stands in a
   corner facing out, takes a hit against the boards, a faceoff at each dot. Per still, by pixels (a mask of the ice from the render or from the
   projected rink polygon): the share of the screen that is ice; whether the puck, his skater and the nearer net are on screen and unhidden; the
   camera's pitch; whether the camera or its sight line is outside the boards' line. One table: the share of stills failing each.
2. **The bar** (each as a test in `Jam.Camera.*` over a sweep of his position x facing along the whole boards' line, and as counted stills):
   - ice covers at least 55 % of the screen in 98 % of stills, never under 40 %;
   - the camera's pitch never steeper than 55 degrees down; no change of yaw over 90 degrees inside 30 frames (no flip);
   - the camera's eye never outside the boards' inner line nor inside the glass; nothing between the eye and his skater but air;
   - his skater, the puck (when within 20 m) and the direction he skates are on screen: 3 m of ice visible ahead of him in his direction of
     travel in 95 % of stills;
   - in open ice (more than 8 m from any board) the camera is the one of main, to the pixel (the open-ice look is liked: "does not look bad").
3. **Find the cause, then the smallest design that meets the bar.** Candidates to weigh by measurement, not by taste: the eye clamped inside
   the rink with its yaw eased toward the rink's interior as he nears a board (he skates along the boards: the camera rides inside him, looking
   across and ahead); behind a net the camera stays on the rink side of the goal line looking at the net from the front, over it; in a corner
   the yaw turns to the diagonal toward centre; the distance shortened before the height is raised. Whichever: the move in and out is eased
   (no pop), and the stick's aim by cursor still means what it shows (check `CursorAimFromHit` and the controls' tests: the aim is in rink
   space, so a turned camera must not turn his aim: prove with `Jam.Controls.*`).
4. **Prove**: the tables of item 1 before and after, twelve stills at 1280 x 720 (four corners, behind both nets, both side boards, two hits on
   the boards, two faceoffs), a strip of 120 frames of him skating from open ice into a corner and out (no pop, no flip); the full `Jam` filter
   green in a cleared environment; Shipping compiles; the mean frame time unchanged.

`CAMERA READY 1` (items 1 to 3 at the bar or the honest table of what is not) by 03:30 by the machine clock; `CAMERA READY 2` (what the eye
still finds after it looks at READY 1) by 06:00. Time box: the night.
