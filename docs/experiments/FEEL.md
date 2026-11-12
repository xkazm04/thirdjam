# Experiment A10: feel and first-play QA on `main` (device A, H5, 2026-11-12)

Lane A10, the first player who is not a builder. Everything here was done through the editor's MCP server on port 8010 and Python
(`tools/qa/`), no game code and no cartridge changed. State at the 60-minute report (11:57): the build on `main` as checked out at 11:00 does not
reach a faceoff (bugs 1-3); with QA copies of the cartridges (bug 1 key removed, `l1_day` instead of the GPU-hanging `l2_night`) four PIE runs of 12-60 s
and one packaged-build run were measured. The `boost` half of the script could not run (no pickup simulation on `main`).

## Verdict (five sentences)

1. As checked out at 11:00, `main` does not reach the first faceoff: every shipped cartridge is rejected at load (`game.ai.difficulty` is in all six
   cartridges but not in the C++ loader), PIE opens on an empty black rink, and the shipped look (`l2_night`) hangs the GPU within 4-7 s when a cartridge does load.
2. The skating is the Godot port to the centimetre and reads as ice: 0.67 s to top speed, a U-turn of 0.68 s that drifts 2.2 m sideways and never drops below 2.6 m/s,
   a glide that would take 218 m to stop, and no key to brake; snappy to start, impossible to stop, which on a pad reads as "slippery" first and "sluggish" after a minute.
3. The puck is the problem: with no possession and zero drag every bump sends it off at the skater's own speed, so a player who tries to carry spends half the match 2.6 m
   behind a puck doing 7.4 m/s (436 m skated, 14 bumps, 0 goals in 60 s), while a player who shoots the instant he touches it scores from centre ice every time (3:0 in 17.7 s).
4. The shot (18 m/s, 1.1 s from 20 m, 8-degree assist) and the check (lands on the first press, 90-frame knockdown) are right; the camera (6 / 3.5 / 70 / lag 4) is right on
   open ice and wrong at the boards, where it sits outside the rink; the puck does not read beyond 10 m on the day ice and there is no trail or disc yet.
5. On pacing the A2 numbers are decisive: on 120 x 60 a bot-quality human sees a goal every 4.8 minutes and 2 of 20 matches finish, on 90 x 45 a goal every 1.9 minutes and
   16 of 20 finish; the 2-minute first play of `GAME.md` exists only on the small rink, and only once the puck can be carried (proposal 0).

## Method

| Step | How | File |
|---|---|---|
| editor | `UnrealEditor.exe ThirdJam.uproject -nosplash -ModelContextProtocolStartServer -ModelContextProtocolPort=8010` started by hand (not `editor.ps1`, which would overwrite the shared `Game/logs/editor.json`); PID in `tools/qa/out/editor-8010.json` | `tools/qa/out/editor-8010*.log` |
| Python in the editor | `node tools/qa/py.mjs <script.py> --var K=V` wraps `execute_python` | `tools/qa/py.mjs` |
| drive the human | a slate post-tick callback reads `UJamPresentation::GetFrame` (positions, puck, score, phase) every sim frame and injects the left stick and the A / B buttons through `UJamPlayerInput::DebugInjectKey` (the real Enhanced Input path, camera-relative like `ReadHumanInput`) | `tools/qa/drive.py` (plans `skate`, `play`, `raw`, `boost`), `tools/qa/stop.py` |
| one run | `node tools/qa/run.mjs --plan play --cut tools/qa/cartridges/hockey.regular.qa.json --frames 7200` : sets `jam.Cut`, StartPIE, injects, polls, HUD capture at every phase change, teardown, StopPIE, JAMEVENT log lines, metrics | `tools/qa/run.mjs`, `tools/qa/analyze.mjs` |
| offline model | a port of `UJamSim::StepSkater` for the expected numbers and the what-ifs in the proposal table | `tools/qa/simfeel.mjs` |
| QA cartridges | copies of the four cuts with only `game.ai.difficulty` removed (bug 1) and `look.tier` set to `l1_day` (bug 2); the shipped cartridges are untouched | `tools/qa/cartridges/*.qa.json` |

Frames are the clock (fixed 60 Hz, `bUseFixedFrameRate=True` in `DefaultEngine.ini`, verified live).

## Bugs

| # | What | Evidence | Owner |
|---|---|---|---|
| 1 | **Every cartridge rejected at load**: `JAMCART hockey.regular at /game/ai/difficulty: unknown key "difficulty" is not allowed here` then `cut "hockey.regular" rejected; falling back to hockey.regular` (which fails again). PIE then shows a black rink, no skaters, no HUD. All six `Game/Content/Cartridges/*.json` got `"ai": {"difficulty": "pro"}` at 10:58 (lane A3) while `JamCartridge.cpp` (built 10:56, and the source on disk at 11:00) only allows `yield_margin_m, yield_radius_m, hold_radius_m, speed_scale, red_skill`; `schema/cartridge.schema.json` has no `difficulty` either. Note the replace also dropped the five explicit `ai.*` numbers (they equal the defaults, so no behaviour change once the key is accepted). The staged package of 10:59 (`Game/Saved/StagedBuilds/Windows`, `Manifest_UFSFiles_Win64.txt`: cartridges staged at 10:58:30) carries the same files, so the packaged game has it too; I played it with `-cut=<path to a QA copy>`. | `tools/qa/out/editor-8010.log` lines 2791-2793; `tools/qa/out/probe/mcp-image-1794477824025-1.png` (black viewport) | A3 (key) with A1 (loader) and A4 (schema): land the loader and the schema before the cartridges, or the gate `CartridgeRoundTrip` must catch it |
| 2 | **GPU hang in PIE with the L2 night rig**, reproducible 2 of 2: `LogD3D12RHI: Error: GPU crash detected: Device 0 Removed: DXGI_ERROR_DEVICE_HUNG`, page fault, same Aftermath hash `16151227421261170552` both times; the active breadcrumb is `ShadowDepths` of `UEDPIE_0_LR_L2_Night_LevelInstance_1.Spot_L2_Night_2` (virtual shadow maps on the four spot cones). First time 7 s after `LoadCut` of a valid cartridge mid-PIE, second time 4 s after a clean `StartPIE` with the valid cartridge (`JAMLOOK tier=l2_night map=/JamAssets/Rigs/LR_L2_Night`). The editor terminates (`D3D12Util.TerminateOnGPUCrash`). RTX 4090, 2.5 GB of 23 GB VRAM used. | `tools/qa/out/editor-8010.log` 2904-3306, `editor-8010-2.log` 2825-3078; `Game/Saved/Logs/D3D12.0.2026.11.12-11.04.46.nv-gpudmp` | B (rig `LR_L2_Night`: spot light shadows) with A5 (tier switcher): try `r.Shadow.Virtual.Enable=0` for the rig or non-shadowing spots; the `FrameBudget` gate should run PIE for 10 s on the shipped tier |
| 3 | **`main` does not compile** between 11:01 and at least 11:20: `JamNet` module added to the `.uproject` at 11:07 with no binaries, `DirectorsCallController.h` (11:05) references `UJamDirectorsCallWidget` that does not exist, then `MenuFlow.cpp` (11:17) fails on `CollectLeaves` / `FLeaf`. Any editor started in that window rebuilds and exits (`Still incompatible or missing module: JamNet`). Not a bug in the game, a bug in the process: lanes are landing half-states on the shared checkout. | `tools/qa/out/editor-8010-3.log` 445-466; `Game/logs/build-ThirdJamEditor-Development-20261112-1117*.log` | Director: builders should build in a worktree and land green |
| 4 | **Controls are camera-relative through the camera manager** in `AHokejGameMode::ReadHumanInput` (`PlayerCameraManager->GetCameraRotation().Yaw`, which is the render-interpolated, 4-frame-lagged camera). `docs/experiments/CAMERA-120M.md` section 6 and `tools/skills/camera-third-person.md` say to map the stick to the skater's heading or to a camera yaw computed from sim state, so recorded inputs replay; the HUD contract says nothing in the sim reads the camera. Feel impact measured as nil (the U-turn matches the model to 0.1 m); the cost is determinism: the same stick trace replays differently at a different render rate, and a recorded-input test cannot exist. | `Game/Source/JamGame/Private/HokejGameMode.cpp` 285-304 | A1 (with A5): compute the control yaw from the sim camera filter (`UJamFollowCamera::GetFilter().YawDeg`) or the skater yaw, never from the camera manager |
| 5 | **A scripted human wins 3:0 in 17.7 s from centre ice**: in the packaged Development build (`Game/Saved/StagedBuilds/Windows`, 10:59) with `jam.ScriptHuman 1` the match is three identical cycles, shot at faceoff + 85 frames from (0, 0), goal 189 frames later at x = 56.2 m, faceoff 120 frames after that; the five bots never touch the puck. Not a crash, but the first play `GAME.md` describes (skate, carry, shoot, score, get checked) does not happen: with no goalie and 8-degree aim assist, the winning move is "shoot at the faceoff". | `tools/qa/out/game-regular-l1/game.log` (JAMEVENT shot 85 / goal 274 / faceoff 394 / 480 / 669 / 789 / 874 / 1063 / win 1063) | A1 + A2 (design): the red back must guard the net (hold at x = +50, intercept the line to the mouth) or the shot needs a goalie-sized miss chance beyond 25 m; `PlayerBotWins` should also assert the first goal comes after frame 600 |

## Skating (measured in PIE through the real input path, run `tools/qa/out/skate-1`; model = `tools/qa/simfeel.mjs`)

Plan `skate` on the QA copy of `hockey.regular` (speed 8, accel 12, turn_rate 5, lateral_grip 3, glide_drag 0.147): left stick injected every sim frame, positions read from the
presentation frame, velocities by finite difference. Two runs (`skate-1` with scene captures, `skate-2` without, 901 frames) gave identical numbers to the last digit: the sim
is deterministic through the real input path too.

| Quantity | PIE, real input | Model | `tools/skills/skating-physics.md` says | Reads as |
|---|---|---|---|---|
| time to 95 % of top speed, full stick from rest | **40 frames (0.67 s), 2.57 m**; top speed 7.998 m/s; 99 % at 42 frames | 39 frames, 2.6 m | about 0.67 s | snappy; fine |
| glide: 5 s off the stick from 8 m/s | **7.27 m/s after 4.97 s, decel 0.1472 m/s², 38 m covered**; extrapolated stop 217 m / 54 s | 7.27 m/s, 218 m | > 7 m/s after 5 s | real ice, but on a pad it means "I cannot stop": letting go does nothing a player can feel |
| reverse-stick stop from 7.3 m/s (the only stop a player has) | **41 frames (0.68 s), 3.43 m along the old heading, 2.2 m sideways, speed never below 2.6 m/s** | 40 frames, 3.31 m, 2.24 m side, 3.2 m/s | n/a | the skater does not stop, it U-turns through 180 degrees of yaw with thrust on; a real skate stop is 1.3 s / 5.1 m with `bBrake`, which **no key binds** (`FJamInput::bBrake`, "no default key binds it in M0") |
| 90-degree turn at 8 m/s | (U-turn above: yaw 0 to 90 degrees in 20 frames, velocity at 36 degrees at that moment) | velocity follows the new heading after 1.05 s; slip peaks at 55 degrees; 4.6 m of drift along the old heading | 19 frames to turn the body (yaw), slip halves in about 14 frames | the body turns fast (0.33 s) but the velocity lags a full second: this is the "ice" and also the "sluggish" |
| sustained hard turn (stick 90 degrees to the velocity, 240 frames) | **settles at 2.76 m/s on a 0.53 m circle, yaw rate 5.0 rad/s, 56 degrees of slip** (`skate-2`) | 2.78 m/s, 0.56 m, 56 degrees | n/a | a hard turn costs 65 % of the speed; holding the stick sideways is the brake the game does not have |
| `lateral_grip` 2 / 4.5 | | 90-degree turn follows after 1.48 s / 0.75 s; drift 6.2 m / 3.5 m | | grip is the one knob that changes the turn feel without touching speed |
| `turn_rate` 4 / 6.5 | | follows after 1.12 s / 0.98 s | | turn_rate barely matters above 4: the velocity lag is grip-limited |
| `glide_drag` 0.3 / 0.5 | | stops after 107 m / 64 m; 6.5 / 5.5 m/s after 5 s | | more drag makes the pad feel controllable without touching the turn |

The sim is the model to the centimetre (the port is faithful), so the model's what-ifs in the proposal table are trustworthy. The input stack adds nothing measurable: no deadzone
loss at full stick (top speed 7.998), and the camera-relative mapping (bug 4) did not bend the straight phases (the 2.2 m of the U-turn is the physics, the model has the same 2.24 m).
Lateral grip reads as ice: during the U-turn the velocity trails the body by up to 60 degrees and the figure leans into it (`feel-reverse-stop.jpg`).

## Puck and shot (PIE runs `play-2`, `play-3`, `raw-1`: 3600 frames each, a chase-and-shoot controller on the real input path)

- **The puck runs away from its carrier.** There is no possession glue: the skater body (r 0.4 m) pushes the puck (r 0.12 m) to its own speed and `puck.drag` is 0, so after
  every bump the puck leaves at 8 m/s and the skater, also at 8 m/s, cannot close the gap. Measured over 60 s of chasing on `hockey.regular`: the human skated 436 m (mean 7.3 m/s),
  spent 1869 of 3601 frames at speed 2.6 m behind a puck doing 7.4 m/s (medians), bumped it 14 times, was "on the puck" (within 1.1 m) 13 % of the time with a longest run of 50
  frames (0.8 s), and scored 0. Same picture on `raw` (460 m, 2.4 m gap, 12 %) and with the shoot threshold at the 1.5 m reach (`play-3`: 16 shoot presses, 2 shots, both from
  the boards at y = 18-27 m where the chase had led, 0 goals). The scripted player bot wins because it shoots on first touch at the faceoff (bug 5); a player who tries to carry
  tours the rink behind the puck (`feel-play-carry.jpg`: frame 1564, in his own corner, 96 m from the far net).
- Taking the puck from a bot: the human got the first touch 5.3-5.5 s after kick-off in all three runs (the puck was already moving; a red bot shot at frame 311 each time),
  the yield rules then kept the bots off it. Keeping it while turning: 19 carries per minute, none longer than 77 frames; the hard-turn probe at the first long carry lost the puck
  within 40 frames in `raw-1` (as it should) and in `play-3` kept it (the puck was 1.4 m away and the turn brought the body back onto it), so "lose it on a hard turn" holds but so
  does "lose it on a straight line".
- Shot: 18 m/s confirmed (`puck_speed_after_mps` 18.0 on all four human shots); 1.11 s from 20 m, 189 frames = 3.15 s from centre ice to the goal line on 120 m (packaged build,
  three times exactly). Not weak; from anywhere inside the far blue line it is a goal unless a body is in the way, and no body ever was. `aim_cone_deg` 8 is live
  (`has_aim_cone True, 8.0` in the loaded config) and did not help the side shots: the cone bends only when the facing is already within 8 degrees of the net.
- Check (`hockey.raw`): B pressed when an opponent was within 1.3 m at speed > 5.2 m/s landed on the first try (`JAMEVENT check who=0` and `knockdown who=3 kind=check` at
  frame 889); the human was never knocked down in 60 s (the bots did not check back). Knockdown 90 frames reads fine on the toy figure (`20261112-102957-A-a5-knockdown-right.png`).
- Boost: `JamSim.cpp` has no pickup simulation (only `Config.Pickups` is copied), so the `boost` plan was not run; the pickup half of the 2-minute script does not exist on `main`.

## Camera (PIE captures of the five runs, against CAMERA-120M)

`game.camera` in every cut: third_person, 6 m back, 3.5 m up, look-ahead 5 m, FOV 70, lag 4 frames, radar on; `OffAxisM` 0.7 is a constant in `FJamCameraSettings` (not a cartridge key).
The M0 PIE screenshot of 10:39 (`devlog/shots/20261112-103953-A-m0-pie-scripted-human.png`) shows: skater bottom-centre, puck not visible on black ice, far net readable
only through the marker ("29 m" diamond) and the orange trim strip, the radar legible bottom-right, two opponents 100-160 px tall. The off-axis 0.7 m is the camera's, not the
puck's: the puck sits where the body pushes it, which is dead ahead on a straight carry, so the pawn hides it for the first 6.4 m (CAMERA-120M section 1) unless the ice shows a trail.
In the runs the puck was 2-3 m ahead and visible beside the figure when on pale ice up close (`feel-raw-carry.jpg`: 25 px at 2 m); the 4-frame lag reads fine on straight lines and
U-turns (the camera swung behind the figure without a visible snap in the captures). Two findings: (1) **no collision test at the boards**: with the skater within 6 m of the boards
and facing away, the camera sits outside the rink and the bloomed board fills the lower half of the frame (`feel-play-carry.jpg`), the common case after every corner chase;
(2) the near net has no marker, so after a U-turn the player's own net is off-screen with no cue. Proposed `game.camera`: keep 6 / 3.5 / 70 / lag 4; add a board clamp (pull the
camera in along the arm until it is inside the boards, A5), and a second marker for the own net when it is behind.

## Readability (L1 day rig in PIE; the shipped L2 night rig could not be rendered, bug 2)

- The M0 frame of 10:39 (unlit black ice) has an invisible puck at any distance. On the L1 day rig (`feel-first-frame-l1.jpg`, 1280 x 700) the puck 10 m ahead is a 6 px dark dot
  on a blotchy pale texture: findable when you know it is at centre ice, lost the moment it moves (`feel-glide-far-net.jpg`: the puck is in frame and I cannot point at it). CAMERA-120M
  measured 30 / 13 / 8 px at 10 / 25 / 40 m on plain ice; the L1 texture noise costs the 10 m case too. No trail, no contact disc: A5's effects binder logs
  `JAMFX missing /JamAssets/VFX/NS_*` and `MS_*` for every event (the VFX and audio assets are not in the checkout), so the readability aids `GAME.md` promises do not exist yet.
- Jerseys read: blue `#1D4ED8` and orange `#E8651A` separate at 60 m (two orange figures at the far blue line are unmistakable), numbers do not (the "1" on my skater is the same
  value as the body); the toy figure's lean reads as skating and sells the ice.
- Far net: marker plus distance text reads at 66 m and clamps correctly to the left edge when the net is behind ("13 m", `feel-reverse-stop.jpg`); the beacon pillar and the red outline
  on the net frame read at 45 m without the HUD. The near net has no marker: after a U-turn the player's own net is invisible until the radar is read.
- HUD: score boxes, "TAKE 1", radar (dots 4-6 px: too small at 1280 wide, the experiment used 9-12 px), and a permanent "ESC / START DIRECTOR'S CALL READY" line; no controls hint.

## Pacing: 120 x 60 versus 90 x 45 (A2 tournament, `tools/tournament/out/ue-final.jsonl`, 20 seeds per cell)

| Cell | finished (first to 3) | goals / min | median first goal | median gap between goals | human touches / min | human shots / min | human wins |
|---|---|---|---|---|---|---|---|
| regular 120 x 60, scripted player bot | 18 / 20 | 0.62 | 15 s (mean 81 s) | 21 s | 2.9 | 1.1 | 90 % |
| regular 120 x 60, brain as the human | **2 / 20** | **0.21** | 198 s | 168 s | 5.6 | 0.8 | 10 % |
| small 90 x 45 (speed 11), scripted player bot | 19 / 20 | 1.17 | 43 s | 39 s | 4.4 | 1.6 | 90 % |
| small 90 x 45 (speed 11), brain as the human | 16 / 20 | 0.54 | 112 s | 68 s | 9.1 | 1.3 | 65 % |

The scripted player bot is a perfect chaser (it beats the yield rules); the brain-as-human row is the honest estimate of a first-time player. On 120 x 60 a first-to-3 match
takes about 14 minutes and touches come every 11 s; on 90 x 45 it takes about 5.5 minutes and touches come every 6.6 s. `max_faceoff_to_touch` is 91-94 frames on the
large rink for the scripted bot (the `HumanHasSomethingToDo` gate holds) but 412 frames median for the brain on the small rink (and 1271 worst), so the yield rules are what keep
the large rink honest, not the geometry. Recommendation: **ship `hockey.regular` at 90 x 45 with speed 11** (or make `arena.small` the first cut) and keep 120 x 60 as the Wide cut.
End-to-end skating time is 15 s on 120 m at 8 m/s and 8.2 s on 90 m at 11 m/s.

## The first 30 seconds as a judge would see them (`feel-first-frame-l1.jpg`, the three PIE runs)

- Frame 1: a blue figure with a "1", a blue teammate, two orange figures 20 m off, a 6 px dot at centre ice, "00 00 TAKE 1", a magenta diamond "66 m", a radar, and the line
  "ESC / START DIRECTOR'S CALL READY" bottom-left. Nothing says what the buttons are and nothing says what to do. Needed: one hint line for the first 10 s, "Left stick / WASD skate,
  A / Space shoot, R restart" ("B / Shift check" in Raw Cut), and the objective on the TAKE card: **"TAKE 1: first to 3"** (timer cuts: "TAKE 1 of 3, 3:00").
- Seconds 1-5: the orange chaser reaches the puck first and shoots it away at frame 311 (every run, every seed 1 start); the human's first touch comes at 5.3-5.5 s, chasing a moving
  puck. A judge who hesitates for two seconds starts the match 20 m behind the play. The faceoff should give the human the first touch: spawn the human 5 m from the puck and the
  red chaser 10 m, or hold the red chaser for 60 frames after the drop.
- Seconds 5-30: the puck runs ahead (section "Puck and shot"), the chase goes to a corner, the camera ends up outside the boards, and the far net is 96 m away. The judge has not
  shot yet. With proposal 0 (drag) the same 25 s hold a carry, a shot and most likely a goal (the packaged build scores from anywhere inside the blue line).
- The "DIRECTOR'S CALL READY" line is on from frame 1 and never changes; it reads as a prompt to press ESC before the first touch. Show it after the first goal, or after 60 s.
- Not seen: the goal pause and the win card in PIE (0 goals in the carried runs); the packaged run had three goals and the win in 17.7 s, no captures (tooling error 10).

## Proposals (ranked by impact on fun; "safe" = no test risk, "re-run bots" = PlayerBotWins / AiBotPlays / HumanHasSomethingToDo must be re-run)

| Rank | Key | Now | Proposed | Why (numbers) | Risk |
|---|---|---|---|---|---|
| 0 | `game.puck.drag` | 0 | 0.5 (1/s) | the runaway puck: with exponential drag 0.5 a puck bumped to 8 m/s is at 5.6 m/s after 0.7 s, so the carrier (8 m/s) catches it every bump instead of chasing it 2.6 m behind for half the match; an 18 m/s shot has a range of 36 m (v0 / k): it reaches the goal line from 20 m in 1.6 s at 8 m/s and from the 90 x 45 blue line (about 22 m) in 1.9 s, but never from centre ice (bug 5 fixed as a side effect). 1.2 would be too much (range 15 m) | re-run bots (the bots' chase numbers change) |
| 1 | `game.rink.length_m` / `width_m` (hockey.regular) | 120 / 60 | 90 / 45 (corner 3.75) | brain-as-human: goals/min 0.21 to 0.54, finished matches 2/20 to 16/20, touches/min 5.6 to 9.1 | re-run bots (the `arena.small` cell already passes 16/20 brain, 19/20 playerbot) |
| 2 | `game.skater.speed` (with 1) | 8 | 11 | 90 m end to end in 8.2 s instead of 15 s; shot stays 1.64x the skater | re-run bots (same cell) |
| 2b | `game.shot.reach_m` | 1.5 | 2.0 | with the puck 2-3 m ahead of a skater at speed, a 1.5 m reach means "no shot while moving"; 2.0 m covers the median gap after a bump (2.6 m) only with drag (row 0); together they give a shot on the move | re-run bots |
| 3 | `game.skater.glide_drag` | 0.147 | 0.3 | off the stick the skater still does 6.5 m/s after 5 s (7.3 now) but stops in 107 m instead of 218 m; a pad player can let go of the stick to slow down | safe for determinism, re-run bots (bots glide too) |
| 4 | `game.skater.lateral_grip` | 3 | 4 | 90-degree turn: velocity follows in 0.85 s instead of 1.05 s, drift 3.8 m instead of 4.6 m, still 50 degrees of slip (still ice) | re-run bots |
| 5 | `game.skater.accel` | 12 | 16 | 0.48 s to top speed instead of 0.65 s; the reverse-stick stop keeps 4.7 m/s instead of 3.2 m/s (less dead time after a stop) | re-run bots |
| 6 | bind `bBrake` (Left trigger / Ctrl) | no key | bind it | the only real stop is 1.3 s / 5.1 m and nobody can press it | safe (input map, A5) |
| 7 | `game.camera.lag_frames` | 4 | keep 4 | measured: no visible snap on U-turns, no feel cost from the camera-relative stick; change only if bug 4 is fixed by reading the lagged sim yaw, then 3 (12 degrees of error instead of about 18) is the cheaper feel | safe |
| 8 | HUD hint line + TAKE objective text | none | see "first 30 seconds" | first-play clarity | safe (A5) |

Shot speed: keep 18. Turn rate: keep 5 (4 to 6.5 changes the follow time by 0.1 s; grip is the knob). Row 0 is the one that changes the game: everything else is tuning around a
puck nobody can carry. The drag figures are the model's (`v *= 1 - drag * dt` per substep, range = speed / drag); a sweep of 0.3 / 0.5 / 0.8 through `tools/tournament` is the cheap way to pick it.

## Screenshots

`devlog/shots/20261112-11*-A-feel-*.jpg` (1280 px wide JPEG, 57-89 KB each, caption in the sidecar `.txt`): `feel-first-frame-l1` (first play view with HUD), `feel-reverse-stop`
(U-turn lean, edge-clamped marker), `feel-glide-far-net` (scene capture at 45 m from the net), `feel-play-carry` (camera outside the boards, 96 m from the far net),
`feel-raw-carry` (puck readable at 2 m, three orange bots at the far net). Raw material: `tools/qa/out/<run>/shots/hud-*.png` (HUD capture at every phase change, 1280 x 700),
`Game/Saved/Screenshots/WindowsEditor/A10-skate-*.png` (78 scene captures, every 10th frame of the skate run, 1.1 MB each, not copied). The packaged-game run produced no
captures (the window handle was not found; `tools/qa/gamerun.ps1` needs a `FindWindow` by title).

## Errors and surprises (all of them, in order)

1. `World.get_subsystem` and `unreal.SubsystemBlueprintLibrary` do not exist in the editor's Python; a world subsystem is found with `unreal.ObjectIterator(unreal.JamPresentation)` filtered by outer.
2. `PlayerController.get_pawn` / `k2_get_pawn` are not exposed and the `pawn` property is protected; `GameMode.get_human_skater()` is the way to the human pawn.
3. Bug 1: both `LoadCut` calls at `StartPlay` fail and the match is never built, so `jam.Cut <path>` set through the console before `StartPIE` is the only way to play a cartridge by path; `LoadCut(path)` from Python mid-PIE worked (`True`) but was followed by bug 2 seven seconds later.
4. Bug 2 twice (one editor each); each restart is about 60 s to MCP; the `CrashReportClient` windows were closed (they were my editor's children).
5. Bug 3: the third editor start recompiled the project (new `JamNet` module in the `.uproject`), failed on another lane's file and exited after 13 s; a background watcher (`tools/qa/out/watch-and-start.ps1`, log `watch.log`) starts the editor once `UnrealEditor-JamNet.dll` exists, `UnrealEditor-JamGame.dll` is fresh and no UBT / cl.exe has run for 30 s.
6. `editor.ps1` writes the shared `Game/logs/editor.json`; it was not used so another lane's `-Stop` does not kill this editor and vice versa.
7. UE log timestamps are UTC (`10.01.10` = 11:01:10 local); `GetLogEntries` reads the current session's log only.
8. `str(unreal.JamPhase.PLAY)` prints `<JamPhase.PLAY: 0>`, so my first play run compared the phase to `"PLAY: 0>"`, never matched, and the human stood still for 60 s
   (`play-1`, discarded; the bots scored 0 against an idle human in that minute, which is its own data point). Fixed with a regex on the enum name.
9. A Python tick that takes a 1280 x 720 `take_high_res_screenshot` every 10th frame runs the fixed-rate engine at about 6 sim frames per wall second (58 without), so the
   skate run's circle phase hit my wall-clock budget; `run.mjs` now budgets 12x real time and the long runs use `--shots 0` with HUD captures at phase changes only.
10. `tools/qa/gamerun.ps1` captured 0 frames of the packaged game: `Process.MainWindowHandle` was zero for the game window (the `-log` console is a separate process window);
    needs `FindWindow` by title. The game's log and the JAMEVENT lines were enough for bug 5.
11. `StopPIE` after `stop.py` (unregister the tick, drop every PIE reference, `gc.collect()`) never crashed the editor in six runs; the CAMERA-120M pitfall holds and the mitigation works.
12. The `jam.Cut` console variable accepts an absolute path (`ResolveCartridgePath` returns a `.json` path as is), which is how the QA copies were played without touching `Game/Content/Cartridges`.

## Operator verdict (2026-11-12 14:28 CET, review page docs/reviews/feel-2026-11-12.html, export in feel-2026-11-12.decisions.json)
All eleven proposals pass. Comment on feel.00: "the stick does not have hockey stick shape, we have no hands on players. The puck does not stick with the stick." Standing note from the
operator: while the game is not yet playable and watchable, calibrations like these need no approval; the operator reviews calibrations again near the finished phase (R-0021 point 1).

## GPU soak (R-0016 point 6, 14:32-14:36, one editor, the GPU alone)

Setup: the Director's link of 14:31 (`build-ThirdJamEditor-Development-20261112-143113.log`, `Result: Succeeded`), no other editor, cook or build running (checked
with `tasklist` / `Get-Process` before the start); one editor, `-ModelContextProtocolPort=8010`, log `tools/qa/out/soak-1/editor-soak-1.log`. Driver **NVIDIA 610.88**
(internal 32.0.16.1088, date 7-22-2026) on the RTX 4090 (adapter 0; adapter 1 is the AMD iGPU, unused). Script `tools/qa/soak.mjs`, two runs (`soak-1`, `soak-2`, `soak.json` each),
PIE viewport 1280 x 704.

Sequence per run: `jam.ScriptHuman 1`, StartPIE on `/Game/Maps/Rink` with the default cut, `stat gpu`, `ProfileGPU` (dumped to the log), 60 s of play, `jam.playcut hockey.boost` at
20 s and `jam.playcut hockey.regular` at 40 s (each followed by a HUD capture and a `ProfileGPU`), StopPIE. The log confirms the default cut: `JAMCART loaded
hockey.regular.cartridge.json id=hockey-regular title="Regular Cut" seed=1 rink=90x45 visual=toy tier=l2_night`, `JAMLOOK tier=l2_night map=/JamAssets/Rigs/LR_L2_Night
scale=1.000 (rink 90.0 m)`, `rig loaded: 12 local lights scaled`; at 20 s `JAMDESIGNER cut loaded hockey.boost (Neon Rink)` + `JAMLOOK tier=l3_neon ... scale=1.333 (rink 120.0 m)`;
at 40 s `JAMCART loaded hockey.regular ... rink=90x45 ... tier=l2_night` again. The scripted human played (goals at frames 640 by a bot and 1652 by the human in soak-1).

**Result: no hang, 2 of 2 runs, 2 x 60 s plus two rig switches each.** `grep "GPU crash|DEVICE_HUNG|Device Removed|DXGI_ERROR|TerminateOnGPUCrash"` on the editor log: 0 lines;
`Virtual Shadow` / `VirtualShadow` warnings: 0; `LogD3D12RHI: Warning` / `LogRHI: Warning`: 0; no `.nv-gpudmp` written. The VSM-off retry was therefore not run.

ProfileGPU (inclusive ms of the PIE frame; the editor-world frame rendered alongside is listed for completeness; `stat gpu` was on but the Slate capture does not include the stat
canvas, so the totals below are ProfileGPU's, `tools/qa/out/soak-1/profilegpu.json`):

| Dump | Cut / rig | PIE frame total | Scene | ShadowDepths | VSM top row | Lights | Lumen (probe gather / scene lighting) | PostProcessing | editor-world frame |
|---|---|---|---|---|---|---|---|---|---|
| soak-1 t6 s | regular, l2_night, 90 x 45 | **3.09 ms** | 2.20 | **0.41** | BuildPageAllocation 0.13 | **0.06** | 0.92 / 0.35 | 0.51 | 1.39 |
| soak-1 t22 s | boost, l3_neon, 120 x 60 | 2.92 ms | 1.81 | 0.04 | (no VSM pages) | 0.01 | 0.97 / 0.27 (+ reflections 0.18) | 0.52 | 1.35 |
| soak-1 t43 s | regular, l2_night | 4.16 ms | 2.76 | 0.53 | BuildPageAllocation 0.19 | 0.12 | 1.25 / 0.44 | 0.67 | 1.83 |
| soak-2 t6 s | regular, l2_night | 4.02 ms | 2.63 | 0.50 | BuildPageAllocation 0.16 | 0.08 | 1.56 / 0.39 | 0.64 | 2.11 |
| soak-2 t22 s | boost, l3_neon | 5.65 ms | 3.42 | 0.04 | (no VSM pages) | 0.02 | 2.53 / 0.55 (+ reflections 0.30) | 1.65 | 3.33 |
| soak-2 t43 s | regular, l2_night | 4.20 ms | 2.71 | 0.52 | BuildPageAllocation 0.18 | 0.11 | 1.21 / 0.43 | 0.66 | 1.77 |

Reading: the hardened `LR_L2_Night` costs 0.4-0.5 ms of ShadowDepths and about 0.1 ms of Lights at 1280 x 704, with Lumen screen-probe gather the largest pass (0.9-1.6 ms);
the whole PIE frame is 3-4 ms on the night rig and 3-6 ms on the neon rig, well inside the 8 ms gate. The morning's two hangs (bug 2, page faults inside `ShadowDepths` of
`Spot_L2_Night_*`, 10:04 and 10:08) happened with four to five editors and a package cook sharing the GPU and the pre-hardening rig; with the hardened rig and the GPU alone
nothing reproduces in 120 s and two rig switches. The VSM hypothesis is neither confirmed nor excluded by this soak: the pass still runs (0.5 ms, pages allocated every frame)
and does not hang alone; the next discriminating test is the same soak with four editors open, with `r.Shadow.Virtual.Enable 0` as the B arm. Editor closed at 14:35:53,
port 8010 free, no other editor running at any point of the soak.

## Lane A16 calibrations (R-0013, C-0005, R-0021; applied without approval per R-0021 point 1, 2026-11-12 14:38)

| What | Value | Why (measured) |
|---|---|---|
| carry: the puck stays on the blade | closes on the blade by min(1, 100 x carry_assist x dt) per substep and moves with the carrier; leaves only on shot, pass, poke, lift or a check | operator feel.00 ("the puck does not stick with the stick"); the 90 deg/s cutoff of C-0005 is dropped; Jam.Core.Stick keeps the puck through an instant 180 at 11 m/s |
| carry assist gate | only while the puck moves with the carrier (relative speed under 4 m/s), never on the frame a shot or pass leaves | a 14 m/s pass was pulled back to the passer; a tap shot left at 15 instead of 18 m/s |
| pass lead | flight time solved against the receiver velocity, plus pass_lead_frames (20) of its motion | a fixed 20-frame lead missed a 5 m/s runner by 8 m (0 of 10 seeds) |
| pass reception | a teammate puck reaching the blade keeps 25 % of its relative speed and is the receiver's | the receiver's own blade deflected every pass (deflection and reception share the 0.6 m blade radius) |
| deflection | only an aimed stick (stick input that frame) deflects an opponent shot, at 70 % | with passive blades every bot back became a goalie: the A3 sweep fell to rookie 30 % (from 90) with 18 human shots per match |
| draw push | 2.5 m/s back towards the winner's side (was 4) | the player bot never presses at the draw; after a lost draw it needed 660-867 frames to the first touch (gate: 600) |
| charged shot | tap 17.85, half hold 21.81, full 25.78 m/s (speed 18 + bonus 8, cap 30) | measured in Jam.Core.FeelDepth |
| faceoff countdown | 90 frames, clock paused, puck pinned, early press loses, bot centres press at 6..18 frames, passive human = seeded coin | R-0013 point 2; the brain patch (A3) replaces the sim's bot timing |
| foul grace (`stick.foul_grace_frames` 12) | a check on a skater that carried the puck within the last 12 frames is not `foul_no_puck` (from behind still is) | R-0029 point 3: the A34 playtester took two of three rookie fouls right after the victim lost the puck; Jam.Core.Stick: clean at +6 frames, a foul at +20 |

## Lane A33: tackling that reads (R-0028 point 3; cosmetic over the sim, no calibration applied; 2026-11-12 19:40)

Full page: `docs/experiments/TACKLING.md` (payloads, the reaction table, proof, eyes, B's handoff draft). The sim changed only in what it announces; **the Determinism
baseline is unchanged and re-recorded here**: `Jam.Core.Determinism` hockey.regular seed 1 frame 600 `884f1c9a85793dd5`, hockey.raw `6027ac7eba473026` (headless,
`Game/logs/a33-tests-1.log` in worktree a33, 19:27); `Jam.Bot.VisualSwap` `1871c1d348fa8b41` on generated_b2 / humanoid_b1 / toy_skeletal.

| What | Value | Measured / why |
|---|---|---|
| check direction (`check` Kind) | `front` / `side_l` / `side_r` / `behind` from the push against the victim's facing, 0.5 bands (60 degrees) | 15 of 15 scripted collisions classify as set up (Jam.Core.TacklingEvents: 5 directions x 6 / 9 / 12 m/s) |
| check strength | closing / `skater.speed` (unchanged): raw 6 m/s start -> 0.698 (5.6 m/s at contact after 5 frames of glide, raw's top speed 8.0), 9 and 12 -> 1.0; the proof hit at 9.3 m/s head-on -> 1.0 | the clamp at 1 flattens every hit above the top speed: a harder skater reads the same as a top-speed one (fine for P2) |
| skater `board` event | >= 4 m/s into the boards; Strength = normal speed / `skater.speed` | a 6 m/s start 2 m out arrives at 4.15 m/s (0.519 on raw's 8.0); bots hit the end boards at 0.83-1.0 in bunches when a puck is dumped (3 within 7 frames in the proof run) |
| front fall | authored 40-frame fall at 0.85 + 0.3 x strength (a 1.0 hit lands 2 frames sooner than a 0.5 one), back to the push over 6 frames, 20-frame get-up | the only authored fall; strength is read from the `knockdown` event on the bus |
| side / behind fall | the toy tips 88 degrees toward the push over 14 frames (3 + 3 bounce), untips over 20 | the code figure's `KnockTipDeg` curve on the skeletal toy |
| board lean | 12 frames, 10 + 6 x strength degrees into the boards | |
| stumble (mistimed poke) | sim 20 frames (unchanged); visual: 6-frame jab, then 20 frames of 12 degree pitch, 5 degree wobble, stick 25 cm low | the old visual showed 8 frames of get_up |
| camera punch | 6 frames, 2 cm / 0.6 degrees x max(0.5, strength), linear decay; human's checks only; no time dilation | |
| puck pop | 18 frames (0.3 s), 12.5-25 cm by the puck's speed after the strip | |
| hit hold (`jam.HitHoldFrames`) | default 0; judged in TACKLING.md section 4 | |

Before / after pair: `docs/experiments/img/tackling/a33-hit1-front-s100-strip.png` (after: the fall by direction, the burst, the camera kick) against the A10 still
`devlog/shots/20261112-102957-A-a5-knockdown-right.png` (before: the toy figure's single backward fall, no direction, no kick, no pop).
Proposed, not applied (calibrations that change the hash; TACKLING.md sections 2 and 4): (1) a stay-up stagger instead of a knock-down below a strength band;
(2) a check lands on the checker's own speed along the normal but its strength is the closing speed, so a chase-down on a skater moving the same way landed at
strength 0.03 in the proof run (frame 1074) and still cost the victim 90 frames: land on the closing speed too, or floor the knockdown strength at 0.3.

## Lane A39 calibrations (R-0029 point 4, R-0021 point 1; applied without approval, 2026-11-12 20:25): the three payoffs

Each is a cartridge key with a default (DEFAULTS.md), a test (`Jam.Core.Payoffs.*`) and the playtester grid before and after (docs/experiments/PAYOFFS.md). The three keys at 0
replay the old game to the hash (20 of 20 all_star / good matches identical to the baseline grid). The Determinism baseline moves with the burst (a carrier counts its carry
frames and bursts once past the last man): **hockey.regular seed 1 frame 600 `b705e2def86ae838`, hockey.raw `e09eebd2cebd1337`** (re-recorded 20:32 in worktree a39; the same on main 5397edb merged, a39-main, 20:48: A31's charge, A32's read-outs and A33's kinds leave the scripted baseline where it was).

| What | Value | Why (measured) |
|---|---|---|
| one-timer `shot.one_timer.window_frames` / `cone_bonus_deg` | 10 frames / 8 degrees: a shot released inside the window after a teammate's pass landed on the blade gets the full charge (25.8 m/s instead of a 17.9 tap) and `aim_cone_deg` + 8 of aim help; `shot` event Kind `one_timer` | PLAYTEST.md section 6: the back fills the lane while the carrier holds the charge (`acts.charge` 4 % of frames, 60 frames of patience); a pass that arrives in the window beats it. In the grid the human gets almost none (0-0.1 per match: a pass is received only when the blade sits on its line), pro and all_star bots use it |
| breakaway burst `skater.breakaway.speed_bonus` / `frames` | 0.2 / 60: top speed and acceleration x 1.2 for one second once the carrier has carried for 30 frames (a touch-and-shoot never bursts), is past centre ice, skating at the net (facing within 60 degrees), with no standing opponent in a 4 m corridor between it and the edge of the crease (6 m from the net: the back holding the crease is the goalie, not the last man); once per possession (resets when the puck changes hands); event `breakaway` | PLAYTEST.md: carries over 3 s fall from 3.6-4.4 (rookie) to 0.7-1.4 (pro, all_star) because the bots catch every carrier. Test: the burst at frame 31 of the carry, the slot reached at frame 147 instead of 169, 7.6 m ahead of the chaser instead of 3.5. The first cut burst on the touch: the Jam.Core.Seams scripted attacker (speed 13, burst 15.6 m/s) outran its own 17.8 m/s tap and carried the puck to its own end for the rest of the match; the 30-frame carry fixed it (Jam.Core.Payoffs.EmptyNetScript guards the scene). Grid: 2-9 bursts per match, the human's goals at rookie 0.8 / 1.7 / 1.9 -> 1.1 / 2.3 / 2.5 (novice / regular / good). Symmetric: bots burst too (all_star GA 2.3 -> 2.6-2.8) |
| deke `stick.deke.offset_m` / `commit_frames` | 1.0 m / 15 frames: a `bDeke` tap by the carrier moves the puck to the blade side (the side the stick is aimed to; the middle of the ice when it rests) over 6 frames, riding 0.8 m ahead of the body (the offset is clamped so the puck stays inside shot reach: 1.15 m at reach 1.5); the crease back's blade, poke and shot block follow the puck's old side for the commit and for the flight of a shot released inside the window; that shot aims at the post on the deke side (0.6 m inside); a second tap inside 30 frames does nothing; event `deke` Kind left / right | PLAYTEST.md: shots on target die on the crease back (`LastManGoalie`), 1 in 7-8 goes in. Test: 3 of 3 seeds score from 8 m past the brain's pro back where the same shot without the deke is saved 3 of 3. Grid: the regular / good players deke 1.1-2.5 / 1.9-4.2 times per match, 1-2 payoff goals per match at rookie |

## Lane A46 calibrations (PAYOFFS.md section 6, R-0029 point 4, R-0021 point 1; applied without approval, 2026-11-12 23:00): passes reach the human

Full page: `docs/experiments/PASSING.md` (rules, keys, tests, the grid before and after, the controls, what the measurements say). Each is a cartridge or profile key
with a default, a test (`Jam.Core.PassReception`, `Jam.Bot.PassesToHuman`) and the playtester grid before and after. **The Determinism baseline is unchanged and re-recorded
here**: `Jam.Core.Determinism` hockey.regular seed 1 frame 600 `b705e2def86ae838`, hockey.raw `e09eebd2cebd1337` (the scripted baseline has no pass to a body; headless,
`Game/logs/a46-tests-core-final.log` in worktree a46). `stick.reception_m` 0 and `mate.pass_to_human` 0 replay the old game.

| What | Value | Why (measured) |
|---|---|---|
| body reception `stick.reception_m` | 0.9 m: a teammate's pass in flight (nobody carries it) that reaches the body within this radius at 6-14 m/s is seated on the blade like a blade reception, with 4 frames of settle in which the carry assist holds it there; the one-timer window opens; a pass above 14 m/s bounces off the body as before unless the blade meets it; a shot or a pass spends the settle | PAYOFFS.md section 6: the human received a pass only when the blade sat on its line (0-0.1 one-timers a match). Test: an 8.9 m/s pass to the body seats 0.00 m from the blade, the window opens, a tap inside the settle is a one-timer at 25.8 m/s that keeps flying (the first cut's settle pulled the shot back onto the blade: fixed, guarded); a 15.8 m/s pass is not received; the key at 0 is not received. Grid pro / regular: 1.6 passes received a match with the key at 0, 2.0 with it on |
| pass to the human `mate.pass_to_human` (profile knob, tune key) | rookie 0.6, pro 0.4, all_star 0.2 (was a 0 / 1 flag: rookie 1, pro and all_star 0): the share of the 2-second windows in which a teammate carrying the puck passes to the human when he is open (standing, 5-25 m away, ahead of or level with the puck, no opponent within 2 m of the lane) or set up for a shot (inside 26 m, squared to the net within 15 degrees); the `pass` signal (R-0007) makes it 0.9 while it runs; the back passes only on `space` / `pass`; the pass is the sim's (`FJamAIWorld::CarrierBestPass` = the human) else lane A3's aimed tap from 8 m or more; the coin is a fixed function of the skater and the window | PAYOFFS.md section 6: `bPassToHuman` was rookie-only. Grid pro / regular: 2.0 passes received a match with the share on and 2.0 with `mate.pass_to_human=0`: **the share buys nothing measurable, because the human's teammates carry the puck about 2 s per match** (40 playtester matches: 125 frames of mate carry per match, the human open 5-25 m ahead in 23 of them, 5 passes by the rule in 40 matches). One-timers stay at 0-0.1: every reception is 41-82 m from the net (the mates have the puck only on the breakout) |
## Lane A49 calibrations (the operator's H18 verdict; R-0021 point 1; applied 2026-11-12 23:25): the human's check, the sprint, the camera defaults

All three are human-only or unused by the scripted baseline, so **the Determinism baseline does not move**: `Jam.Core.Determinism` hockey.regular seed 1 frame 600
`b705e2def86ae838`, hockey.raw `e09eebd2cebd1337` (re-run headless 23:21 in worktree a49 with the changes in: `Game/logs/a49-det.log`; the same numbers as A39 / A45).
Root causes, bindings and the packaged proofs: `docs/experiments/CONTROLS.md`.

| What | Value | Why (measured) |
|---|---|---|
| the human's check window (`UJamSim::HumanCheckWindowFrames`) | 12 frames (bots keep `CheckWindowFrames` 6) | a person's press is not frame-exact: with 6 frames (0.1 s) the operator's checks never connected; 0.2 s is still a timed press, not a held button |
| the human's check speed (`UJamSim::HumanCheckSpeedMinMs`) | the check lands from 2 m/s of the human's own speed along the contact (bots keep `check_speed_min` 5) | the sim's rule reads the checker's speed, not the closing speed; the operator skates slowly into the carrier, so no `check` event, no knock-down, no strip, no reaction ("impossible to take puck") |
| sprint `skater.sprint.bonus` / `drain_frames` / `refill_frames` | 0.15 / 180 / 360: Shift or LT held adds 15 % to top speed and thrust while moving; the stamina (integer ticks, hashed only once it left full) empties in 3 s and refills in 6 s at rest; bots never sprint | the operator asked for Shift as the sprint; 15 % is under the breakaway burst's 20 % so the burst still reads as the bigger payoff |
| camera `far_net_bias` / `end_height_m` | 0.2 / 3 (were 0.5 / 5); the end height is at least the height, so 3 m under the 3.5 m rig means no lift; the yaw stays within 12 degrees of the bearing to the pivot and the pitch within 6.5 of the chest (`FJamCameraSettings::MiddleThird*`) | the lead and the lift turned the camera away from the human; the budget keeps the human in the frame's middle third every frame (`Jam.Camera.MiddleThird`, the packaged 20 s strip in CONTROLS.md) |
