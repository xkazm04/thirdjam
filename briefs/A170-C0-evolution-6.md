# Lane A170, track C0 (the curve), evolution 6 (device A, Opus)

You are lane A170 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then R-0060 to R-0062 at the end of `contracts/RULINGS.md`, the track's `docs/experiments/CURVE-9.md` and `status/A162.md` (two hours old: your method, your
scripts under `tools/perf/results/a162/`, every row), and what landed since with every key OFF: `docs/experiments/BRAIN-5.md` (**section on pro's Press job: the
lever this evolution is about**), `CONTACT-8.md`, `MOVEMENT-7.md`, `MOVEMENT-8.md`.

Your worktree and the commit it stands on are named in the Director's message that starts you: the track's worktree moved to the newest main. It was built
before: an incremental build is enough. Your status file: `status/A170.md`. Port 4770. Family as the track's: the difficulty profiles, the shipped
cartridges' rules and levels, `tools/perf/results/**`, `docs/experiments/CURVE-10.md`, the soak-set tests' budgets.

## Evolution 6: the answers ready before the operator gives the questions
The operator plays package H28 (main c5dc15e5) at 10:00 and then says which player he is and what the last day goes to. **Nothing of this evolution lands
before he has played: it is measurement, and at most a prepared profile diff kept as a patch.** Each item ends in one table and one sentence he can say yes
or no to.
1. **Pro on the regular rink to its bar without touching the stop.** Last night no lever did it (red skill breaks the 9.5 m stop; the pressure time was
   noise). The brain lane has since made one: with `opp.hk_press_in_hunt=1;opp.hk_press_commit_m=8` at pro (hash-identical to today's pro by itself, 100
   of 100), the hunt radius is an aggression dial the stop drill does not read (hunt 20: about 13 points of pro, the stop at 8 m still 100 %, 9.5 m 97 %).
   Find the hunt radius that puts pro at 48 (600 a point for three or four points on the dial, then 2,000 at the chosen one), with law 3's whole row
   (8 / 9.5 / 10 / 11 / 12 m and the straight charge) beside each: the bar stays A121's 95 % at 9.5 m. Then the large rink's pro cell at that value.
   Output: the two profile lines and the hunt value as a patch under `tools/perf/results/a170/`, the cells, law 3's rows.
2. **The same dial for the player who lands his checks** (R-0060: `JAM_PLAYTESTER_CHECK` unset, the course press: pro reads 56.0): the hunt value that puts
   THAT player at 48, and what the whiffing player then reads. So the Director can re-set the ladder in ten minutes whichever the operator says he is.
3. **What the switched-off set costs together, and its price in one dial**: the contact set (push-out 0.6, cooldown 60 with restart, lunge 12 / 1.5, slam
   30), the half-second screen, the mates' keep-out: all on, the six cells at 600, then at most one number a level to bring each back within 3 (pro: the
   hunt dial of item 1; all_star: say what moves it now that pro's lever exists there too: `opp.hk_press_commit_m` at all_star is one to try).
4. One paragraph for the operator: "if you say X, the ladder is re-set by Y and you get a package Z minutes later".

Write `DIAL READY` in the status file with item 1's table the moment it exists (the Director wants it by 08:30); items 2 to 4 follow. Few processes, many
seeds; `-Multiprocess`; three measurement processes at most, one chain at a time (a stopped background task leaves its `sh` child running: stop the child
by its own process id, never by name); check memory before each batch. Time box 150 minutes.
