# Lane A154, track C0 (the curve), evolution 4 (device A, Opus)

You are lane A154 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then R-0053 to R-0059 at the end of `contracts/RULINGS.md` (your worktree's copy is the newest), the track's `docs/experiments/CURVE-7.md` and `status/A133.md`
(your method, your levers, every row), the design eye's second review `docs/design/REVIEW-2.md` (its fresh-seed reading of the ladder: 86 / 51 / 23; its
items 4 and 5), and what landed on the sim and the brain since your ladder was set at 2,000 a cell: `docs/experiments/CONTACT-5.md` (the shove under the
shields; the scripted human presses its check on closing speed: the human's cells read about +1 / +1 / -2), `MOVEMENT-5.md` (a kept first step: +0.8 / +1.6 /
-2.0), `BRAIN-3.md` (for teams under five the human's mates check the carrier and the back steps up: the large rink's pro and all_star cells +2.8 and +2.3;
the regular rink hash-identical), and the season you set (`CURVE-7.md` section 9).

Your worktree and the commit it stands on are named in the Director's message that starts you: the track's worktree moved to the newest main. It was built
before: an incremental build is enough. Your status file: `status/A154.md`. Port 4754. Family as the track's: the difficulty profiles, the shipped
cartridges' rules and levels, the season's takes, `tools/perf/results/**`, `docs/experiments/CURVE-8.md`, the soak-set tests' budgets.

## Evolution 4: the ladder the operator plays at 10:00 is the ladder that was measured
Four landings have touched the cells since they were set, each inside the 4-point rule alone; nobody has measured their sum. The operator's package is
cut from main by 09:00, and from 08:00 nothing that moves the ladder lands without an acceptance run on its own package (R-0056): so this is the last set.
1. **Measure the six cells on this main** (three levels, the regular and the large rink), 600 a cell first, then 2,000 where a cell is within 4 points of
   its bar. Targets 85 / 48 / 26, within 3.
2. **Trim at most three profile numbers** if a cell is off by more than 3 (aggression first, speed last; never the hunt radius beyond what law 3 allows;
   `Bar.Curve.Law3` and `Jam.Core.Movement.StopAndGo` stay green). The large rink's cells moved most (the brain's third evolution): its red-skill and speed
   offsets are the natural levers.
3. **The season on this main**, 200 a take for the novice and the regular player: the ramp of R-0057 still holds (no novice step over 20 points from take
   3 to take 7; take 7 at 15 % or more; the finale 25 % or more for a regular player).
4. **`Jam.Bot.HumanHasSomethingToDo`'s budget** from your own 2,000 matches with the scripted human's new press (the Director kept 9.3 % at the landing
   because the ladder and the new press had never been measured together).
5. For the operator's sheet (`docs/PLAY-ME.md` is the Director's: propose the sentences): the six cells as they are, the season's table, and one line on the
   long shot (the review: opponents release from a median 35.7 m at pro; a third of pro goals come from 25 m or more): what a second pass would cost and buy
   (your own estimate from `CURVE-7.md`'s rows), so he can decide at 10:00 whether to spend Saturday on it.

Write `LADDER CHECKED` in the status file with the tables and, if you changed a number, the file list, as soon as items 1 and 2 are done: the Director lands
a trim at once. Few processes, many seeds; `-Multiprocess`; check memory before each batch. Time box 90 minutes.
