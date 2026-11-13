# Lane A53: the playtester soak (device A, from H13.3, night filler 1 of 11, Sonnet)

You are lane A53 of the Third Jam (team Hokej). The machine idles at night; the operator wants it used. Your job: run the three playtester classes against the three
difficulty levels over hundreds of seeds in parallel headless games all night, and leave a morning report with the win curve and every fault the soak found.

Read first: `docs/experiments/PLAYTEST.md` (R-0029: the curve the game wants: novice 80/85 at rookie, regular 50/55 at pro, good 30/25 at all_star), `tools/playtest/run.mjs`
(how one playtester match is launched headless and what it prints), `Game/Source/JamCore/AI/JamPlaytester.cpp` (`-player=`, the classes), `tools/run/run-packaged.ps1`,
`docs/experiments/CONTROLS.md` (how A49 ran a packaged Development build), `tools/gate/premerge.ps1` (how automation is invoked), `docs/experiments/BALANCING.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a53` (created at main a6b6f5b; build what you need yourself, unity off). Run no git command; never touch
`Game/Plugins/JamAssets`; never print `.env`; never kill a process you did not start. Open no interactive editor (other lanes hold the three slots): headless only
(`UnrealEditor-Cmd.exe -game -nullrhi` or a packaged Development game, whichever `tools/playtest/run.mjs` already uses). Machine budget for you: at most **8 parallel games**,
about 12 GB; check `Get-CimInstance Win32_OperatingSystem` free memory before each batch and drop to 4 games when less than 12 GB is free.

You own: `tools/soak/playtest/**` (the fan-out runner, the aggregator), `docs/experiments/SOAK-PLAYTEST.md`, `docs/experiments/img/a53/**`, `status/A53.md`. Nothing in
`Game/Source` (the lane reports, it does not fix; a fault's reproduction command goes in the report).

## Job
1. `tools/soak/playtest/soak.mjs`: fans out matches over a plan `{class, level, seed}` with N workers, each a headless match of one full game on `hockey.regular`
   (and `hockey.boost` for one third of the seeds), capturing the log; resumable (a results file per match, skip those done); a `--hours` budget. Plan: 3 classes x 3 levels
   x 40 seeds = 360 matches, then more seeds while time remains.
2. `aggregate.mjs`: per class x level: win %, goals for and against, the human's touches, shots, passes received, checks landed and taken, one-timers, dekes, match length;
   a 95 % interval on the win %; the R-0029 target next to each; a flag where the interval misses the target.
3. Faults from the logs, each with the seed and the command to reproduce: crashes (exit code, callstack head), ensures and errors, matches with zero human touches, stuck
   pucks (no shot or pass for 20 s), overtime without end, `JAMFX missing`, `missing usage flag`, any line with `Error:`. Deduplicate by the first distinct line.
4. Every two hours: refresh `SOAK-PLAYTEST.md` (the table, the fault list, matches done, the machine's memory at the time) so a morning reader sees progress even if the
   soak stops. The report's first section is "What A3 should change", written as numbers (which class x level is off by how much), never as a tuning decision.
5. Stop at 06:30 or when the `--hours` budget ends; the last report is final; `status/A53.md` says how many matches ran.

Report: the table, the fault list with reproduction commands, the file list. Interim report at 60 minutes with the first 90 matches. Time box: until 06:30.
