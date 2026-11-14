# Lane A189, track C0 (the curve), evolution 8 (device A, Opus)

You are lane A189 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then **R-0064** at the end of `contracts/RULINGS.md` (the operator's words after playing; points 2 and 11), the track's `docs/experiments/CURVE-9.md`, `CURVE-10.md`,
`CURVE-11.md` (the ladder as it stood this morning, pro's hunt dial with the presser on its job, the price of the switched-off set) and the speed lane's
`docs/experiments/MOVEMENT-10.md` with `status/A185.md` (free skaters 15 % faster: law 3's new rows and the six cells after it).

Your worktree and the commit it stands on are named in the Director's message that starts you: the track's worktree moved to the newest main, which
carries the speed rule ON (`skater.free_speed_mult` 1.15 in every shipped cartridge). Status file `status/A189.md`. Port 4789. Family as the track's: the
difficulty profiles, the shipped cartridges' levels, the season's takes, `tools/perf/results/**`, `docs/experiments/CURVE-12.md`, the soak-set tests' budgets.
Your scripts are under `tools/perf/results/a162/`, `a170/`, `a182/`: copy, do not rewrite.

## Evolution 8: the ladder after the speed rule, in two passes
The operator ordered free skaters 15 % faster; it ships ON. On the speed lane's 600-seed cells it compresses the ladder: rookie 85.3 / 83.7 became
69.3 / 80.8 (regular / large rink), pro 50.5 / 46.7 became 53.3 / 51.2, all_star 29.8 / 25.3 became 36.3 / 38.7. Targets stay 85 / 48 / 26, within 3;
law 3 is a gate at every level (the stop begun at 8 m at 80 % or more, a straight charge under 20 %; the stop's window at pro is now 8 to 9 m).
**Pass 1, for the package of 15:30 (hard time: `LADDER SET 1` by 14:50)**: the cells that are furthest out, with the fewest numbers, at 600 then 2,000
where a cell is within 4: rookie on the regular rink first (16 points too hard: red skill down, the opponents' speed scale, their reaction: aggression
first, speed last), then all_star (10 points too easy on both rinks: its speed scale has room again now that everyone is faster; the brain lane's
two-point dial `opp.hk_second_man_hunts` / `opp.hk_step_up_m` is on main, off: BRAIN-6.md), then pro (the hunt dial with `Hk.bPressInHunt` and
`PressCommitM` 8: CURVE-10: 11 read 48.0 this morning; find it again on this tree). Each number moved: built = tune by seed (600 of 600) and the full
`Jam` filter. Write `LADDER SET 1` with the six cells, the numbers moved and the file list: the Director lands it at once.
**Pass 2, for the package of 18:30 (hard time: `LADDER SET 2` by 17:45)**: the held cross-check (lane A184, `contact.cross_hold` and the wind-up, ON)
will be on main by about 15:15: the Director moves your tree or tells you the patch: the six cells again at 2,000, the season's novice ramp (R-0057: no
step over 20 points from take 3 to take 7, take 7 at 15 % or more, 200 a take), the twelve modes' win rates for a regular player at 100 seeds (which
left 15 to 85 %: a list for the modes lane, you do not edit modes), and the second column of R-0060 (the player who lands his checks) for the sheet.
**Not decided by you, but measured for the Director in pass 2**: with the ladder set, what the take shield (`contact.take_shield_frames` 15 / 45) and the
check's cooldown (60 with restart) cost on this tree, one row each at 600: the Director decides whether either goes on for the freeze.

Sentences for the operator's sheet in your status file after each pass (the six cells; one line on what moved and why). Few processes, many seeds;
`-Multiprocess`; three measurement processes at most, one chain at a time; stop only processes whose ids you recorded at their start (never by name or
pattern: the common rules' last section); the measuring environment never reaches a gate run. Time box 300 minutes in two passes.
