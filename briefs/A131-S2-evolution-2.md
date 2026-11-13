# Lane A131, track S2 (the acceptance matrix), evolution 2 (device A, Sonnet)

You are lane A131 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, `docs/experiments/ACCEPT.md` (how the acceptance run drives the packaged game through the real cursor and buttons, and why: six packages
shipped without a working shot before it existed), and the track's first evolution in your worktree: `status/A108.md`, `docs/experiments/ACCEPT-MATRIX.md`,
`tools/qa/accept/**`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a108` (the track's: evolution 1 is committed there as the baseline and is landing; its Development
package is `Game/Packaged/A108dev`, built from the vision's commit). Your status file: `status/A131.md`. Port 4708 (the track's). **You own the system cursor and
the game windows** until you report. Family: `tools/qa/accept/**` (the drivers, `accept.ps1`, `accept.mjs`, the cuts), `HokejAcceptDrill.cpp` only for the two drill
changes named in item 2, reports under `docs/experiments/`.

## Why this matters tonight
The acceptance run is the gate of every package (R-0043): the Director's release chain cuts no Shipping package unless it is green. Lane A108 found that on a busy
machine it is **not stable**: `hockey.regular` gave three different verdicts in three runs (a first step lost because the driver read the log late; a check that
landed as a stumble; a tap charged 7 frames against a band of 4). The next package (H27) is cut later tonight on a machine with a dozen lanes running. A gate
that fails for the machine's reasons will be ignored; one that passes by luck is worse.

## Evolution 2: the acceptance run tells the game's faults from the machine's
1. **The first step is never lost**: the driver acts only after the game's `begin` line for the step, reads the log as it grows (not in a late burst: at 86 s it
   handled frames 387 to 1758 in 0.4 s of driver time), and rejects and repeats a step whose first act lags its log line by more than 1 s, saying so (a
   `machine` verdict, not a `fail`). Fix the `Substring` exception at `accept.ps1:258`.
2. **Rows that measure the game, not the frame rate**: the tap's charge is read in sim frames (the band is the game's: 4 or fewer), not wall time; the check
   and cross-check rows start the opponent fast enough for the hit the row names (a drill change in `HokejAcceptDrill.cpp`: say the speeds), or the row states
   the outcome it accepts (a stumble with the puck loose is a landed check under the contact law: decide by `docs/experiments/CONTACT.md` and say which).
3. **A mode-aware pass table**: which rows a mode's rules change by design (pond has no check, marathon's top speed changes the brake row, power-play's and
   glass-house's shot bands) live in a table the matrix reads, so 12 of 12 modes and 5 of 5 tiers show only named, by-design differences.
4. **Proof**: `hockey.regular` gives the same verdict in 5 of 5 repeats while other lanes build (say the processor load during the runs); the matrix re-run.
5. If time remains, on a fresh Development package of the newest main (the Director names the commit when you ask in your status file with a line starting
   `NEED PACKAGE`; build it yourself in your worktree only if told): the rows other lanes queued: "refused" (the human carries and presses RMB; the row reads
   the HUD's contact text or the pawn's `JAMVIS press ... result=` line), bump, stumble, braced, the brake with its spray, and stills of a stumble's hold, a
   check into the boards and the twelve-card menu.

Never kill a process you did not start (other lanes' games and editors run beside you: a run they disturbed is repeated, and your `machine` verdict is how the
tool now says so). Time box 100 minutes; write each run's verdict to the status file as it completes; do not wait on a background watcher: poll inside one command.
