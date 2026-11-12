# Lane A10: feel and first-play QA (device A, from H5)

You are lane A10 of the Third Jam (team Hokej): the first player who is not a builder. Repo `C:/Users/kazda/kiro/thirdjam`. On `main` right now: the sim, rink, pawns, game mode
(lane A1), bots (A2), camera/HUD/toy figure/input (A5), Designer (A7), six cut presets (A4). Read first: `GAME.md` (the 2-minute first play is your yardstick), `docs/HOCKEY.md` (2, 3, 7),
`status/A.md`, `tools/mcp/README.md`, `tools/mcp/ANIMATION.md` and `PHYSICS.md` (how lanes drove PIE through MCP and Python), `Game/Source/JamGame/Public/HokejGameMode.h`,
`Game/Source/JamCore/Public/JamSim.h`, `Game/Source/JamGame/Public/InputMapping.h`, `tools/skills/camera-third-person.md`, `docs/experiments/CAMERA-120M.md`.

You own: `docs/experiments/FEEL.md`, `devlog/shots/*-A-feel-*.png`, `tools/qa/**` (scripts that drive PIE and record). You change no game code and no cartridge: you **propose** values with
evidence, and the Director routes them. Editor on MCP port **8010**; never kill a process you did not start; one heavy job at a time; free-space floor 40 GB. Run no git command.

## What to do
1. **Play it**: PIE on `Rink.umap` with `hockey.regular`, through MCP + Python: drive the human skater with injected input (A5 proved the input stack: stick axes and buttons through the real
   input path) following the 2-minute script in `GAME.md`: faceoff, skate to the puck, carry it, shoot, score, faceoff again, get checked (`hockey.raw`), pick up a boost (`hockey.boost`), win.
   Record each phase with SceneCapture frames (every 10th frame for 300 frames per phase) into `tools/qa/out/` and keep 6-10 representative PNGs in `devlog/shots/` (under 300 KB each).
2. **Judge the feel against numbers**, not taste alone, in `docs/experiments/FEEL.md`:
   - skating: time to top speed, stopping distance, turn radius at speed, whether lateral grip reads as ice; compare with `tools/skills/skating-physics.md` and say what to change (`skater.accel`,
     `lateral_grip`, `turn_rate`, `glide_drag`, `brake_decel`) and why;
   - puck: can the human take it from a bot, keep it while turning, lose it on a hard turn (it should); shot: does 18 m/s feel weak at 120 m (time of flight to the net from 20 m), aim cone;
   - camera: is the puck hidden by the figure, is the far net readable with the marker, does the lag feel right at 4 frames; propose `game.camera` changes;
   - readability: puck visibility at 10/25/40 m in the real build (A1's placeholder ice is pale; B's night ice lands later), jersey contrast;
   - pacing on 120 x 60 vs 90 x 45 (`hockey.arena.small`): time to first touch, time to first goal, how often the human is far from play; your recommendation with the A2 tournament numbers
     in `status/A.md` and `tools/tournament/out/ue-final.jsonl`;
   - the first 30 seconds as a judge would see them: what is unclear, what needs a text hint, what the TAKE card should say.
3. **Bugs**: anything broken (stuck puck, skater through boards, HUD wrong, restart failing) with the frame, cut, seed and a screenshot; file each as a line in `FEEL.md` "Bugs" with the lane
   that owns it (A1 sim/rink, A2 bots, A5 camera/HUD, A7 designer).
4. **Proposals**: a table of concrete value changes per cartridge key with before/after reasoning, ranked by impact on fun; mark each as "safe" (no test risk) or "re-run bots".

Report: the feel verdict in five sentences, the proposal table, the bug list, screenshots, every error. Time box 2.5 hours; report at 60 minutes with the state either way.
