# Lane A107, track S1: S1-soak (device A, evolution 1, Sonnet)

You are lane A107 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full**
(how a track works, the machine's band and the cursor, the walls, the status file, the report with the next three evolutions). Then `docs/EVOLUTION.md` and `docs/VISION.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a107` (the Director created it at the commit that carries the whole vision: contact by inertia, the
movement laws, the hockey brain, the ladder of R-0046). Your status file: `status/A107.md`. Your remote-console port if you run a game: 4707.

## Track S1: the soak
Family: `tools/soak/**`, `tools/perf/results/a107/**` (raw data under `raw/`), `docs/experiments/SOAK-2.md`. No source edits: a fault is a finding with its seed and command.
First evolution: **the whole game on the vision.** With lane A53's runner and lane A95's driver (`tools/soak/playtest/soak.mjs`, `tools/perf/results/a95/curve.mjs`): (a) the nine
class by level cells on hockey.regular and the large rink, 2,000 matches each: confirm R-0046's six target cells and report the cells that have no target; (b) all twelve modes
on the menu, 200 matches each with the regular class at the mode's level: the mode's character number (`tools/playtest/modes.mjs`) against regular on this build: which modes
still differ by a third and which broke under the contact and movement laws; (c) the season's takes, 200 seeds each; (d) from every match: crashes, ensures, matches past the
horn, the longest stretch without a human touch (lane A71's `idle.mjs`), knockdowns and stumbles of the human per match by level. Use up to 16 workers while the machine is
under 38 GB in use. The report's first table is what the Director must fix first.

Time box 90 minutes for this evolution. Interim in the status file at 40 minutes.
