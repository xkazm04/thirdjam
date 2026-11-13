# Lane A116, track S1 (soak), evolution 2 (device A, Sonnet)

You are lane A116 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, and the track's first evolution in your worktree: `status/A107.md`, `docs/experiments/SOAK-2.md`, `tools/soak/a107/**`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a107` (the track's; no binaries: the commandlet runs from the built checkout
`.claude/worktrees/clean-main`, as lane A107 did). Your status file: `status/A116.md`. Family: `tools/soak/**`, reports under `docs/experiments/`.

## Evolution 2: one shared driver and a drift gate
1. **One shared driver** (`tools/soak/cells.mjs`, from A107's `tools/soak/a107/cells.mjs`): `-Multiprocess` on every start, the memory governor, a start guard, requeue
   after a crash, a per-seed fault ledger, a plan file in, one JSON line per match out, a node test. Proving number: A107's 18-cell matrix (36,000 matches) in 25
   minutes or less while other lanes build.
2. **A drift gate for modes and takes** (`tools/soak/drift.mjs`): a committed baseline of the twelve modes' character numbers and the eight takes' win rates per
   playtester class (`tools/soak/baseline.json`, taken from the build you run), a re-run in 8 minutes or less that prints which numbers moved more than 10 % and
   exits non-zero; documented in `docs/experiments/SOAK-DRIFT.md` so every track lane can run it before its report. Marathon needs its cap (`--max-frames 42000`).
3. **The read-outs the tracks keep asking for**, if the playtest line can give them without a source change: who holds the puck during the human's idle stretches,
   knockdowns of the human per match. A stumble counter needs a source change: propose the line (file and field) in your report.

The Director moves `clean-main` to the newest main and tells you when its build is done; until then you run on the build that is there (the vision's build) and say
which commit each number belongs to. Time box 90 minutes.
