# Lane A70: the difficulty curve re-tuned against the soak (device A, from H15, Opus)

You are lane A70 of the Third Jam (team Hokej). Lane A53's soak (`docs/experiments/SOAK-PLAYTEST.md`, `tools/soak/playtest/*`) measured the curve on main e99e005 with
500 to 3,000 matches per cell instead of the gate's 20 seeds, on `hockey.regular`:

| cell | win % (95 % interval) | R-0029 target |
|---|---|---|
| novice at rookie | 93 (91 to 95) | 85 |
| regular at pro | 56 (52 to 61) | 50 |
| good at all_star | 45 (41 to 50) | 25 |

The game is easier than R-0029 wants in every cell since the lifted puck (A52), the stick handling (A51: pro bots read the carried puck 8 frames late, all_star 5) and the
Director's calibrations (pads 0.4, full charge 0.6, rebound 0.45). Your job: bring the three cells onto their targets with the soak as the instrument, by the profile
levers, and leave the curve's record.

Read first: `contracts/RULINGS.md` R-0029 (the curve and who owns it: this lane is A3's successor), R-0033 to R-0036, `docs/experiments/PLAYTEST.md`, `BALANCING.md`,
`SOAK-PLAYTEST.md` (the final table and the fault list), `STICK.md` sections 5 and 6 (A51's lever `game.stick.sweep.bot_react_frames`, its own grid), `LIFTED-PUCK.md`
sections 6, 9, 10 (A52's grid and the calibrations), `Game/Source/JamCore/AI/DifficultyProfile.cpp` (the three profiles and the large-rink offsets), `JamAICore.h`
(`FJamBotSkill`: CheckMinClosing, SweepReactFrames, ReactionFrames, PokeFrames, ShotChargeFrames, the lift rule), `tools/soak/playtest/soak.mjs` (`--set key=value`
if it exists; else `tools/playtest/run.mjs --set`), `tools/playtest/run.mjs`, `Game/Source/JamCore/AI/JamPlaytester.cpp` (the classes are fixed: never tune the player).

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a70` (created at the current main; build it yourself, unity off). Run no git command; never touch
`Game/Plugins/JamAssets`; never print `.env`; never kill a process you did not start. Open no interactive editor: the soak and the playtester run headless. Machine
budget: 8 parallel games, under 12 GB; check free memory before a batch.

You own: `DifficultyProfile.cpp` (the profile numbers only; no new levers unless a cell cannot reach its target with the existing ones, then the lever is named and
asked for), the gate tests' expectations only where a number moved past a bar by design (`Jam.Playtest.NoviceScoresAtRookie` 10 of 20 stays), `docs/experiments/
CURVE-2.md`, `status/A70.md`.

## Job
1. Baseline: re-run the soak's regular rows on your build (600 matches per cell is enough: the interval is 4 points) to confirm A53's table.
2. Levers per cell, one at a time, 600 matches each, the number written down before the next: rookie (novice 93 -> 85): the rookie bots' speed scale or red skill up a
   notch, or the rookie crease skater's save on; pro (regular 56 -> 50): `bot_react_frames` 8 -> 6 or red skill; all_star (good 45 -> 25): `bot_react_frames` 5 -> 3,
   speed 1.15 -> 1.2, checks from 4 m/s, the all_star bots' lift. Never move a level by more than two levers.
3. The large rink (`hockey.arena.large`) rows measured once after the regular rows land (R-0029's offsets may need the same nudge).
4. The gate: `Jam.Playtest.*`, `Jam.Bot.*`, `Jam.Core.Determinism` green on the final numbers; `HumanHasSomethingToDo` measured by the soak (the share over 600 matches)
   and its 12 % budget confirmed or moved to the measured share plus 2 points with the number written.
5. `CURVE-2.md`: the baseline table, each lever's effect, the final table with intervals against the targets, the large-rink rows, and the record for R-0029's next
   amendment (the Director writes the ruling).

Report: the final table, the levers moved, the test results, the file list. Interim at 45 minutes with the baseline and the first lever. Time box 120 minutes.
