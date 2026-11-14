# Lane A162, track C0 (the curve), evolution 5 (device A, Opus)

You are lane A162 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then **R-0060 and R-0061** at the end of `contracts/RULINGS.md` (R-0061 is your charter, word for word), the track's `docs/experiments/CURVE-8.md` and
`status/A154.md` (your method, your levers, your rows of two hours ago), and what landed beside your ladder check: `docs/experiments/CONTACT-6.md` (the
standing brace; the scripted human's course press), `MOVEMENT-6.md` (the poke that takes the puck; a miss locks the stick; the scripted human pokes when it
can land) and `BRAIN-4.md` (all_star's presser inside the hunt: one stop distance, 8 m, at every level).

Your worktree and the commit it stands on are named in the Director's message that starts you: the track's worktree moved to the newest main, which carries
your three trims and all three of those landings. It was built before: an incremental build is enough. Your status file: `status/A162.md`. Port 4762.
Family as the track's: the difficulty profiles, the shipped cartridges' rules and levels, the season's takes, `tools/perf/results/**`,
`docs/experiments/CURVE-9.md`, the soak-set tests' budgets.

## Evolution 5: the last pass before the operator plays
1. **The six cells on this main with the instrument the ladder was set with** (`JAM_PLAYTESTER_CHECK=closing`; the poke's script as the config ships it:
   say which arm that is and why it is the right one for "a person who presses Space when it can land"): 600 a cell, then 2,000 where a cell is within 4
   points of its bar. Targets 85 / 48 / 26, within 3. Expect pro about five points high (the brace +1.5, the poke +3.2).
2. **At most three profile numbers move** (aggression first, speed last; the hunt radius is not a lever: R-0055). Law 3 is a gate at every level: the stop
   begun at 8 m at 80 % or more, a straight charge under 20 % (`Bar.Curve.Law3`, `Jam.Core.Movement.StopAndGo`).
3. Write **`LADDER SET`** in the status file with the six cells, the numbers moved and the file list the moment items 1 and 2 are done: the Director lands
   it at once and cuts the operator's package from it. **Nothing of yours changes a source or cartridge file after that line** (write `SOURCE OPEN` first
   if it must). The hard time: `LADDER SET` by 06:15.
4. Then, measurement only: **the two-column table owed under R-0060** (the old press and the new course press, six cells each, 600 a cell), the season at
   200 a take for the novice (the ramp of R-0057: no step over 20 points from take 3 to take 7; take 7 at 15 % or more) and the regular player's finale;
   `Jam.Bot.HumanHasSomethingToDo`'s budget from your own matches (propose the number; the Director sets it after 10:00).
5. **The operator's sentences** for `docs/PLAY-ME.md` (the Director's file: propose them in your status): the six cells as they are on the package's commit;
   what pro would read for a player who lands most of his checks; the season's table; one line on the long shot (cost and what it buys), as in `CURVE-8.md`.

Few processes, many seeds; `-Multiprocess`; three measurement processes at most; check memory before each batch. Time box 110 minutes.
