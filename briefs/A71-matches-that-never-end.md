# Lane A71: matches that never end, and the long stretches without a human touch (device A, from H15, Sonnet)

You are lane A71 of the Third Jam (team Hokej). Lane A53's soak (`docs/experiments/SOAK-PLAYTEST.md`, raw results under `tools/soak/playtest/out/`) ran about 27,000
headless matches and found two faults: **36 matches reached the 36,000-frame cap (10 minutes) without ending**, 33 of them on `hockey.boost`, 3 on `hockey.regular`; and
**1,752 matches had a stretch of 30 seconds or more without a human touch** (1,014 on boost, 738 on regular). Your job: the first is a bug to find and fix; the second is
to be measured properly and either explained or routed.

Read first: `SOAK-PLAYTEST.md` (the fault list with the seeds and the reproduction commands), `tools/soak/playtest/soak.mjs` and `aggregate.mjs` (how a match is run and what
is recorded), `tools/playtest/run.mjs`, `Game/Content/Cartridges/hockey.boost.cartridge.json` (first_to or timer, pickups, zones), `Game/Source/JamCore/Private/JamSim.cpp`
(the match end: `rules.mode` first_to / timer, overtime, `Phase`, the dead-puck and out-of-play faceoffs A52 added, the puck's height on a pickup), `Game/Source/JamCore/
AI/JamAICore.cpp` (bots that stop chasing: `StallFrames`, the stance, the pickup seeking), `Game/Source/JamCore/Effects/JamPickupActor.cpp` and the pickup rule in the sim,
`docs/experiments/LIFTED-PUCK.md`, `STICK.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a71` (created at the current main; build it yourself, unity off). Run no git command; never touch
`Game/Plugins/JamAssets`; never print `.env`; never kill a process you did not start. Open no interactive editor: everything headless (`tools/playtest/run.mjs` with the seed,
the automation tests). A sim fix changes the hash: say so with the before and after.

You own: the fix in `JamSim.cpp` or `JamAICore.cpp` for the never-ending match (the smallest rule: a puck nobody can reach for N seconds is a dead puck and a faceoff; a
pickup or zone that swallows the puck; a bot stance that never leaves the crease), a test `Jam.Core.MatchAlwaysEnds` (the three regular seeds and three boost seeds from the
soak end inside the cap), `tools/soak/playtest/idle.mjs` (the idle-stretch measurement from the per-frame log, not the per-match totals: the longest stretch per match, where
the puck was, who had it), `docs/experiments/NEVER-ENDING.md`, `status/A71.md`.

## Job
1. Reproduce the three regular seeds and three boost seeds that hit the cap (`tools/playtest/run.mjs` with the seed and the cartridge, a per-frame log); write where the
   puck sat at frame 30,000, who was nearest, what the bots were doing (stance, stall, pickup), and the score.
2. Fix the cause with the smallest sim or brain rule; `Jam.Core.MatchAlwaysEnds` green; the whole `Jam.Core+Jam.Bot+Jam.Playtest` filter green; the hash before and after.
3. The idle stretches: from a 200-match sample of the soak's regular seeds (fresh runs with a per-frame log), the distribution of the longest no-touch stretch per match
   (p50, p90, max), what the puck was doing during the p90 stretch (a bot carry up ice, a dead puck, a faceoff pause, the human in the box), and whether the human's
   teammates kept the puck from the human (A46's `mate.pass_to_human`). Write it as numbers; a tuning proposal goes to the Director, not into the code.
4. `NEVER-ENDING.md`: the six reproductions, the cause, the fix, the idle table, the proposal.

Report: the cause and fix, the idle table, the test results, the file list. Time box 75 minutes.
