# Lane A79: a skill lever at all_star that only the good player beats (device A, from H17, R-0039 open point, Opus)

You are lane A79 of the Third Jam (team Hokej). Lane A70 (`docs/experiments/CURVE-2.md`, R-0039) put the curve on R-0029's targets with speed 1.3 at all_star; the price
is a steep step: the regular player wins 13 % there (29 % before) and the novice 3 %. Speed hits every class alike. Your job: find a lever at all_star that the good
player beats and the regular player does not, so all_star can come down from speed 1.3 toward 1.2 with the good player still at 25 % and the regular player back
near 25-30 %.

Read first: `CURVE-2.md` (every lever measured and its effect; `bot_react_frames` was flat), `SOAK-PLAYTEST.md`, `PLAYTEST.md` (the three classes: what the good player does
that the regular does not: one-timers always vs half, dekes, goalside, the chase lead), `Game/Source/JamCore/AI/JamPlaytester.cpp` (the profiles), `JamAICore.h/.cpp`
(`FJamBotSkill`: `bIntercept`, `LeadFrames`, `bReadLanes`, `PokeFrames`, `bLift`, `bClearLane`, `bPassOut`, `AimErrorDeg`, `CheckMinClosing`, the goalie commit on a deke,
A51's `SweepReactFrames`), `DifficultyProfile.cpp` (all_star), `tools/soak/playtest/soak.mjs` (A70's method: 2,000 matches per cell on A53's seeds, `--set` overrides if
they exist, else the tune hook `ApplyDifficultyTune` through `tools/playtest/run.mjs --tune`), R-0029, R-0039.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a79` (created at the current main; build it yourself, unity off). Run no git command; never touch
`Game/Plugins/JamAssets`; never print `.env`; never kill a process you did not start; headless only; 8 workers, hold under 12 GB free. Lane A77 edits `JamSim.cpp` and
the brain's saucer use at the same time: stay out of `TryPass`/`TryShoot` and the saucer; your levers are skill fields and the all_star profile.

You own: `DifficultyProfile.cpp` (all_star only), one new `FJamBotSkill` field at most if no existing one separates the classes (named, defaulted so pro and rookie do not
move: their cells re-measured to prove it), `docs/experiments/CURVE-3.md`, `status/A79.md`.

## Job
1. Baseline at all_star, 2,000 matches per class (novice, regular, good) on hockey.regular: confirm 3 / 13 / 27.
2. Candidate levers, each measured for all three classes (the separation is the number: good minus regular): the bots read the deke (goalie commit shorter or
   none at all_star), `bReadLanes` and `bIntercept` already on: the intercept lead longer; the bots' poke timing (`PokeFrames` lower) against a carrier who does not
   sweep (the regular sweeps only when blocked; the good always: check `JamPlaytester.cpp`); the bots' one-timer defence (cover the pass lane on a received pass);
   `AimErrorDeg` 0 with a higher shot speed; a new field only if none of these separates by 8 points or more.
3. Compose: speed down from 1.3 in steps of 0.025 with the chosen lever on, until good is 25 +- 3 and regular is as high as the lever allows; write the pair.
4. Pro and rookie re-measured once (no movement allowed beyond the interval); the gate's `Jam.Playtest.*`, `Jam.Bot.*`, `Jam.Core.Determinism` green.
5. `CURVE-3.md`: the baseline, each lever's three numbers, the chosen pair, the record for the ruling.

Report: the table, the lever and the pair, the test results, the file list. Interim at 45 minutes with step 2's first two levers. Time box 100 minutes.
