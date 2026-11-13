# Lane A83: 5 v 5 as the regular game (device A, from H18, R-0041 point 2, Opus)

You are lane A83 of the Third Jam (team Hokej). The operator (R-0041): "the arena can be built for 5v5 game, tackles and cross-checks can be vital". Today `hockey.regular`
and the ladder's takes play 3 v 3 on 90 x 45 (the schema allows 2..5 skaters a side; the brain `kMaxSkaters` 10; A58 measured no hitch in play at 3 v 3). Your job: make
5 v 5 the regular game if it plays and performs, with the measurements that say so.

Read first: `Game/Content/Cartridges/hockey.regular.cartridge.json` and `ladder/take-*.cartridge.json` (teams, roles `player|chaser|support|back`, spawns), `schema/README.md`
(teams[].skaters[]), `Game/Source/JamCore/AI/JamAICore.cpp` (the roles: how a chaser, a support and a back behave; what a fourth and fifth skater would do; the forecheck,
the lane reads, the guard post), `JamPlaytester.cpp`, `docs/experiments/PROFILING.md` and `PERF-FIXES.md` (`tools/perf/match.ps1`, `start.ps1`; the figure's cost per skater: 54
Blueprint events a frame per figure before A65), `docs/experiments/SOAK-PLAYTEST.md`, `CURVE-2.md` (the curve was tuned at 3 v 3: 5 v 5 moves it; A79 re-measures after you),
`docs/experiments/RINK.md`, R-0029, R-0037, R-0039, R-0041.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a83` (created at main c45face; build it yourself, unity off; the Development game package with `-WaitMutex`;
no interactive editor unless a still of ten figures is needed, then one on MCP port 8041, closed after). Run no git command; never touch `Game/Plugins/JamAssets`; never print
`.env`; never kill a process you did not start. Lane A79 changes the all_star profile and lane A82 the sim's pass and carrier rules in parallel: do not touch
`DifficultyProfile.cpp`, `TryPass`, `TryShoot` or the carrier's speed; your brain work is the roles for five, in their own functions.

You own: `hockey.regular.cartridge.json` and the ladder takes' teams (five a side, spawns on a 90 x 45 or, by measurement, 100 x 50 rink), `JamAICore.cpp` (two more roles
or the existing ones doubled: a second support that covers the weak side, a second back that holds the slot; the forecheck with two), `JamPlaytester.cpp` only if a class
breaks with five, `tools/perf/results/a83/**`, `docs/experiments/FIVE.md`, `docs/experiments/img/a83/**`, `status/A83.md`.

## Job
1. 5 v 5 plays: a cartridge copy of `hockey.regular` with five a side; 60 bot matches at pro headless: match length, goals, checks, knockdowns, stuck pucks, the human-touch
   share with the regular playtester (A53's runner); against 3 v 3 on the same seeds.
2. 5 v 5 performs: `tools/perf/match.ps1` on a Development package at l2_night and p3_clay with ten figures: game thread, render thread, GPU, hitches over 33 ms, the figure
   Blueprint cost; against A58's 3 v 3 numbers; a 1080p still of a ten-figure scramble at the net from the game camera.
3. The roles for five: what the fourth and fifth skater do on each side (a second support on the weak side, a second back in the slot; the forecheck with two) so the
   middle is not a pile; the human-touch share stays within 2 points of 3 v 3 (teammates pass to the human: A46's share may need a nudge, say the number).
4. The rink: if ten skaters crowd 90 x 45 (checks per minute double, the puck is never free), measure 100 x 50 and propose; the Director decides.
5. If 1-3 hold: `hockey.regular` and the takes go to five a side (spawns and roles), `Jam.*` green (`SeasonTakes`, `NoviceScoresAtRookie`, `HumanHasSomethingToDo` may move:
   report the numbers; the bars are the curve lane's to re-set, not yours), the Determinism hash re-recorded.
6. `FIVE.md`: the tables, the still, the roles, the proposal.

Report: the play and perf tables against 3 v 3, the roles, the test results, the hash, the file list. Interim at 45 minutes with steps 1-2. Time box 100 minutes.
