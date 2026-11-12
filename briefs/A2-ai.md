# Lane A2: bot brain, roles, yield rules, headless bot tests and tournaments (device A, from H5)

You are lane A2 of the Third Jam (team Hokej). Repo `C:/Users/kazda/kiro/thirdjam`. Lane A1 owns the simulation (`UJamSim`, `FJamInput`, `state_hash`) and is building it
in parallel: build against the interfaces in `briefs/A1-core.md` and `contracts/`, and keep your code in your own files so both lanes compile alone. Read first: `GAME.md`,
`docs/HOCKEY.md` (2: the AI rows, 7), the Godot brain the numbers come from (`C:/Users/kazda/kiro/third-life/site/ice-hockey-three-against-20261111-1/pack/scripts/ai/brain.gd`
and `pack/tests/bot_hockey*.gd`, read-only), `tools/skills/skating-physics.md`.

You own: `Game/Source/JamCore/AI/**`, `Game/Source/JamCore/Tests/Bot*`, `tools/tournament/**`. Run no git command; headless work only (no editor needed: the sim is plain C++;
use the automation test runner through `UnrealEditor-Cmd -ExecCmds="Automation RunTests ..." -nullrhi` or a commandlet; if you need the editor, use MCP port **8002**).

## Laws
Stateless per tick where the Godot brain was; all randomness from `FJamRandom`; decisions read only sim state; outputs are `FJamInput` per bot; no wall-clock anywhere. Your
measurement outranks this brief.

## Deliverables
1. `UJamBrain`: roles `chaser | support | back` from the cartridge; steering `arrive` + `separate` (1.5 m) as in the Godot brain; stall detector (puck anchor not moved 2 m for 120
   frames -> nearest support becomes chaser); **yield rules**: teammates yield when the human is nearer the puck by 2 m within 15 m; opponents who are not `back` hold when the
   human is within 4.5 m of the puck and they are more than 3 m away; attack: shoot when within reach, facing within min(20 deg, atan(1.4 / dist_goal)), dist_goal < reach + 13 m,
   not near a wall; lead = clamp(dist / 8, 0, 30 frames); sway 1.5 m; `speed_scale` and `red_skill` from `game.ai`.
2. **Player bot** (`UJamPlayerBot`, a scripted `FJamInput` for the human's skater used only by tests): the Godot `bot_hockey.gd` logic (approach behind the puck, swing-out,
   lane detour, peel off boards, shot rule).
3. **Tests**: `PlayerBotWins` (seeds 1, 2, 7: WIN with >= 3 shots, identical `state_hash` per seed on two runs), `AiBotPlays` (every skater moved >= 3 m, puck path > 30 m,
   >= 1 AI shot, no skater outside the rink), `HumanHasSomethingToDo` (the human's skater touches the puck within 600 frames after every faceoff for a full match at default tuning).
4. **Tournament harness** `tools/tournament/run.mjs` + a commandlet or automation test it drives: N matches bot-vs-bot per cartridge and seed range, headless, printing goals, shots,
   touches per skater, puck path, stalls, match length in frames, as JSON lines; a summary table. Run it on `hockey.regular` and `hockey.arena.small` (90 x 45, speed 11) for 20 seeds
   each and report the numbers: this is the evidence the operator asked for on rink size.

Report: tests PASS/FAIL with numbers, the tournament table, every error, deviations with evidence. Time box 3 hours; report at 90 minutes with the state either way.
