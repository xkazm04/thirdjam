# Lane A5: third-person camera, radar, far-net marker, HUD, input, toy skater visual (device A, from H5)

You are lane A5 of the Third Jam (team Hokej). Repo `C:/Users/kazda/kiro/thirdjam`. Lane A1 owns the simulation and pawns (`AJamSkater` calls `IJamSkaterVisual` on a visual
component); you own the visual component, the camera, the HUD and input mapping. Read first: `GAME.md`, `docs/THEME.md` (the film-slate skin), `docs/HOCKEY.md` (2 camera row, 5),
`tools/skills/camera-third-person.md` and `docs/experiments/CAMERA-120M.md` (the measured defaults and the C++ radar need), `tools/skills/skater-toy-figure.md` (lean 2.5, cap 35),
`docs/experiments/ANIMATION-SOURCES.md` (what reads at distance), `contracts/skater-visual.md`, `contracts/events.md`, `briefs/A1-core.md`.

You own: `Game/Source/JamGame/Camera*`, `Game/Source/JamGame/HUD*`, `Game/Source/JamGame/Radar*`, `Game/Source/JamGame/ToySkaterVisual*`, `Game/Source/JamGame/Input*`,
`Game/Content/UI/**`, `Game/Content/Blueprints/**`, `Game/Config/DefaultInput.ini`. Run no git command; your editor on MCP port **8003**; never kill a process you did not start.

## Deliverables
1. **Camera** `UJamFollowCamera`: third-person from `game.camera` (distance 6, height 3.5, look_ahead 5, fov 70, lag 4 frames, skater 0.7 m off-axis), **frame-based lag** on the sim
   tick (not DeltaSeconds), pitch derived `-atan(height / (distance + look_ahead))`; `arena` mode as the alternative (fixed high camera framing both nets).
2. **Toy skater visual** `UToySkaterVisualComponent` implementing `IJamSkaterVisual`: capsule body, sphere head with a dark helmet band, 6 cm stick cylinder in `stick_socket`,
   team colour and number as material parameters (`TeamColour`, `NumberTens`, `NumberOnes`), lean `clamp(2.5 * speed * turn_rate, -35, 35)` with the recipe's follow,
   bob, stick swing on shoot, check lunge (pitch 30 deg), tip-over on knockdown in the given direction, blade spray Niagara hook on hard turns (a placeholder emitter named per contract).
3. **HUD** (UMG + C++ driver): score as a film counter, period name "TAKE n", timer, far-net screen marker with distance text (from E4), **radar** 200 x 100 px bottom-right drawn by a C++
   widget from sim positions (6 dots in team colours + puck), "Director's Call" prompt placeholder, win/lose cards "THAT'S A WRAP" / "RETAKE?", restart hint.
4. **Input**: Enhanced Input mapping context: WASD / left stick move, space / A shoot, shift / B check, R restart, Esc pause; gamepad proven in PIE.
5. **Tests**: `FarNetReadable` (marker inside viewport and radar shows 7 entries in PIE), `CameraLagFrameBased` (same camera positions at 30 and 60 Hz for the same sim frames).
6. Prove in PIE with A1's sim if it has landed, else with a scripted mover; screenshots to `devlog/shots/`; stop your editor.

Report: what works, screenshots, every error, deviations with evidence. Time box 3 hours; report at 90 minutes with the state either way.
