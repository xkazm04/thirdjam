# Lane A52: the puck gets a height: lifted shots, the half-height goal that bites, saves by the crease skater, posts and crossbar (device A, from H13.3, Fable 5.2)

You are lane A52 of the Third Jam (team Hokej). Lane A50 landed the operator's half-height goal (`game.rink.goal_height_m`, default 0.61) and wrote in
`docs/experiments/B-DECISIONS.md` section 1 that it bites only visually: the sim's puck is two-dimensional, so every shot is on the ice and the crossbar never matters. The
Director decides (R-0035): the puck gets a height in the sim, so a lifted shot exists, the low goal is a real target, the crease skater's stance saves by height, and the
posts and the bar ring.

Read first: `docs/experiments/B-DECISIONS.md` sections 1-2 and "Open", `Game/Source/JamCore/Public/JamSim.h` (`FJamPuckState`, `InGoal(P, Side, HeightM)`),
`Game/Source/JamCore/Private/JamSim.cpp` (the puck's integration step, friction, the boards, `TryShoot` and the charge `ShootHeld`/`ShotChargeFrames`, `InGoal`, the save and
post read-outs if any), `Game/Source/JamCore/AI/JamAICore.cpp` (`CreaseStance`, how a bot picks its shot and the aim), `Game/Source/JamCore/AI/DifficultyProfile.cpp`,
`Game/Source/JamGame/Private/JamPawns.cpp` (`AJamPuck`, `PopLiftM`: the cosmetic hop to replace), `EffectsBinder.cpp` (the `save`, `post`, `goal` events already bound),
`CommentarySystem.cpp` (situations `save`, `post` if present), `docs/experiments/PAYOFFS.md`, `FEEL.md`, `TACKLING.md`, `schema/game.schema.json`, `limits.json`, `DEFAULTS.md`,
`README.md`, `Game/Source/JamCore/Private/Tests/JamGoalHeightTests.cpp` (A50's height-aware test, waiting for you).

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a52` (the Director created it at main a6b6f5b; build it yourself, unity off; one editor on MCP port 8030 only
while proving; at most three editors on the machine). Run no git command; never touch `Game/Plugins/JamAssets`; never print `.env`. Lane A51 works at the same time in
`JamSim.cpp` on the stick: the carried puck's offset on the blade, a sweep, a spin move, `ResolvePokes`; and in `JamAICore.cpp` on the defending bot's poke timing. Stay out
of those functions: your work is the free puck (its flight, the goal, the save, the posts) and the shot's release; the carried puck's height is always the ice.

You own: `FJamPuckState` height fields in `JamSim.h`, the free puck's step, `TryShoot`'s release (lift from the charge), `InGoal` by height, the save and post rules in
`JamSim.cpp`, the bot's shot height in `JamAICore.cpp`, `AJamPuck` in `JamPawns.cpp` (the visual follows the sim's height; `PopLiftM` goes), the new keys under `game.puck`
and `game.skater.shot` in `schema/` + `JamCartridge.*`, the determinism baseline, `docs/experiments/LIFTED-PUCK.md`, `status/A52.md`.

## Job
1. **Height in the sim**: `FJamPuckState` gains `Z` and `VZ` (metres, m/s; hashed like the rest); gravity `game.puck.gravity_mps2` (default 9.81), a bounce on the ice with
   `game.puck.ice_restitution` (0.3) that kills the hop below 2 cm, the glass stops a puck under `game.rink.glass_height_m` (if the key exists use it; else 2.4 m fixed with a
   note) and above it the puck is out of play (a faceoff, as the sim already does for a dead puck if it has one). A carried puck is on the ice (Z 0).
2. **The lifted shot**: `game.skater.shot.lift` { `tap_m` 0.05, `half_m` 0.45, `full_m` 0.95, `half_frames` 8 }: the release height follows the charge: a tap stays on the
   ice, a charge at `half_frames` leaves at `half_m`, a full charge at `full_m`, measured at the goal line distance (`VZ` from the flight time to the net); the one-timer and
   the breakaway burst keep their current rules with the lift added. With the operator's 0.61 bar a full charge from inside the blue line sails over: that is the design
   (risk against reward; the stamina and the sprint never change the lift).
3. **The goal bites**: `InGoal` with the puck's `Z`: in at `Z < GoalHeightM` between the posts; a puck inside 5 cm of the bar or a post rings `post` (a read-out event the
   binder already has) and reflects; A50's `Jam.Core.GoalHeight` moves from the hard-coded height to a real lifted shot: full charge from 12 m is no goal at 0.61 and a goal
   at 1.22 (same seeds).
4. **The save**: a skater in `CreaseStance` within `game.skater.goalie.reach_m` (0.9) of the puck's path saves a puck under `game.skater.goalie.block_height_m` (0.7) with the
   `save` read-out; a puck above the block height passes (and over the 0.61 bar is no goal anyway: the low shot through the stance is the hard one). The human in the
   stance saves the same way.
5. **Bots**: the shooter's charge choice reads the net's height: at 0.61 a pro bot charges to `half_frames` (0.45 m) from distance and taps from close; at 1.22 it goes full;
   rookie bots mis-charge by the profile's error. A bot match at pro on `hockey.regular` with the 0.61 bar still scores (goals per match within 2 of the FEEL.md baseline;
   write the numbers).
6. **The visual**: `AJamPuck` follows the sim's `Z` (no more `PopLiftM`), the shadow stays on the ice; a PIE still of a lifted puck in flight and one of a puck in the low net.
7. Tests `Jam.Core.LiftedShot` (release heights per charge), `Jam.Core.GoalHeight` (A50's, now real), `Jam.Core.SaveByHeight`, `Jam.Core.PostRings`; `Jam.Core.Determinism`
   re-recorded with the before/after hashes in `LIFTED-PUCK.md`; the playtester curve (R-0029) re-measured with `tools/playtest/run.mjs`; more than 10 points of movement
   at any class is reported with the numbers, not re-tuned (A3 owns the curve).
8. `docs/experiments/LIFTED-PUCK.md`: the model, the keys, the tests, the numbers, the stills, and what B's figure would need (a glove save pose is B's; name the call you
   would add to the interface, do not add it).

Report: the file list, each test with its result, the before/after hashes, the goals-per-match and curve numbers, the still paths. Interim at 40 minutes with steps 1-3;
time box 100 minutes.
