# Lane A50: the operator's decisions through B (R-0033): half-height goals, any-skater goalie stance, commentary variety, crowd variants, the clay class row, the sprint on the wire

You are lane A50 of the Third Jam (team Hokej). Read first: R-0033 in `contracts/RULINGS.md`, `handoff/B-to-A-0032.md` and `0033.md` (the operator's words and B's asks, with the
Director's answers), `docs/experiments/CONTROLS.md` (A49: the sprint, the human's check), `docs/experiments/COMMENTARY.md` and `QA1-FIXES.md` (A47's cut rules), `Game/Source/JamCore/Private/
JamSim.cpp` (the goal test for a lifted puck, `TryShoot`), `Game/Source/JamGame/Private/{RinkActor,HokejGameMode,CommentarySystem,EffectsBinder,JamPawns}.cpp`, `AI/JamAICore.cpp`
(the goalie stance, the shot aim height), `Game/Source/JamNet/Public/JamNetPackets.h` (A38's flag bits; A39 added Deke = 128; the next bit needs a wider mask), R-0013, R-0031.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a50` (the Director creates it at the H19 commit; build it yourself, unity off; one editor on MCP port 8028 only
while proving; at most three editors on the machine). Run no git command; never touch `Game/Plugins/JamAssets`.

You own: `JamSim.cpp/.h` and `JamCartridge.*` + `schema/` for `game.rink.goal_height_m` (0.61 default, range 0.3..1.5; the sim's goal test uses it for a lifted puck; the bots' aim
height follows), `RinkActor.*` (the net frame and netting height from the key; the mesh scaled), `JamAICore.cpp` (goalie stance for any skater covering its crease; aim height),
`JamPawns.cpp` + `HokejGameMode.cpp` (SetGoalieStance for the human in its crease without the puck while an opponent carries inside the blue line; `ResolveVisualClass` rows
humanoid_b1 x p3_clay -> `BP_SkaterVisual_TB1_P3` with the TB1 fallback), `CommentarySystem.cpp` (random pick among candidates, never the last line of the situation, prefer
unheard this match; tests), `EffectsBinder.cpp` (`MS_goal` Variant by the human's team; `MS_crowd_bed` started with the music when the asset exists), `JamNetPackets.*` +
`LobbyGameMode.cpp` (`bSprint` on the wire, protocol v4, `InputCoverage` test), `docs/experiments/B-DECISIONS.md`, `status/A50.md`.

## Job
1. Goal height key end to end (schema, limits, README, DEFAULTS, loader, sim test, net frame, netting, bots' aim); `Jam.Core.GoalHeight` (a puck at 0.8 m over the line is no goal
   at 0.61, a goal at 1.22); Determinism re-recorded if it moves; Designer data regenerated.
2. Goalie stance for any crease skater, the human included; a bot match log with `JAMGOALIE` lines from more than the back role.
3. Commentary variety rule with a test; `MS_goal` Variant and the crowd bed hook (guarded by asset-exists).
4. The clay class row; VisualSwap green.
5. Sprint on the wire: protocol v4; `Jam.Net.InputCoverage` and `LockstepTwoPeers` green.
6. Proof stills: a half-height net at 10 m, a goalie stance on a non-back skater; eyes `bug-hunt`.

Report: the file list, tests, stills. Time box 75 minutes.
