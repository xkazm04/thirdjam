# Lane A205, track V1 (the freeze's verification), measurement only (device A, Opus)

You are lane A205 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then **R-0064, R-0065 and R-0066** at the end of `contracts/RULINGS.md` (what was ordered at noon, what was ruled this afternoon, and the freeze at
20:00: from then only faults are fixed), `docs/PLAY-ME.md` (the sheet the operator plays package H34 by), the curve lane's `status/A189.md`
(`LADDER SET 2`) and `docs/experiments/CURVE-12.md`, the brain lane's `docs/experiments/BRAIN-8.md`, the modes lane's `docs/experiments/MODES-8.md`
section 5, and the hardening lane's `docs/experiments/HARDENING-5.md` with `status/A198.md` (its runs and how they were driven).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A205.md`. Port 4805. **You
change no source, no cartridge, no test and no tool in the tree: you measure the freeze commit and report.** New scripts of your own go under
`tools/perf/results/a205/` (copy the tracks' drivers from `tools/perf/results/a189/`, `a202/`, `a201/` and the hardening lane's from `tools/soak/`,
`tools/fuzz/`: do not rewrite them). Headless only: **the operator plays in a real window from about 20:00: you start nothing that opens a window or
takes the cursor, and every off-screen GPU run of yours carries `-dpcvars=r.RDG.AsyncCompute=0`** (R-0066 point 5). Four processes at most; leave the
machine light from 19:55 (two processes at most while he plays).

## What is owed on the freeze commit, in this order
1. **Built = tune for the ladder** (the open line of LADDER SET 2): the six cells were set with the brain's evolution 8 played by the tune
   `brain8=1`; it now ships as the built default. Replay the six cells as built (600 seeds a cell, the ladder's instrument) against the set's rows
   by seed: 600 of 600 identical a cell, or the count and the first differing seed. One more thing landed after the set: the charge rule
   (`game.skater.charge_dies_on_loss` ON: its lane read every cell within 2.2 points): so the built game is NOT expected to equal the set's rows
   hash for hash: report the six cells as built at 600 with their difference from the set, then with the charge rule switched to its neutral by
   tune or cartridge override (that arm IS expected to equal the set, 600 of 600). If a cell as built is more than 3 from its target (85 / 48 / 26),
   write `FAULT` with the row at once and confirm it at 2,000.
2. **Law 3 as built** (pro and all_star: 8 / 8.5 / 9 / 9.5 m and the straight charge; rookie's 8 m): the gate rows hold (pro 8 m and 9 m at 90 %,
   the straight charge 0).
3. **The twelve modes on the freeze commit** (100 seeds a mode, a regular player, the modes lane's driver): the table against MODES-8's (win rate,
   goals a match): every mode outside 15 to 85 % is a `FAULT` line for the operator's list (you change nothing).
4. **The season's novice ramp** (R-0057: no step over 20 points from take 3 to take 7, take 7 at 15 % or more; 200 a take): the eight takes' table.
5. **Soak and determinism on the freeze commit** (the hardening lane's plan: 60 cells x 100 seeds twice: 0 crash, ensure, error or NaN line, every
   cell bit-identical between the two runs), then lockstep in one process on protocol 9 with the brace on (clean, 30 % loss, jitter: 0 desync), then
   the input fuzz at 200 matches.
6. **The stop line and the ring as built** (the marks lane's `final.sh` and `final2.sh`): a player who obeys every lit line, and one who obeys every
   ring, against one who ignores them, 600 paired seeds at pro: both within 5 points (they were measured before the bots' longer wind-ups, the new
   sense and the second ladder).

One table in the status file, refreshed after every item: the run, the number, PASS or the fault with its smallest repeat. A fault is a line starting
`FAULT` the minute it is reduced. Write `VERIFY READY 1` when items 1 and 2 are in (wanted by 20:00), `VERIFY READY 2` at the end. Time box 240 minutes.
