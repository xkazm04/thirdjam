# Lane A126, track S1 (soak), evolution 3 (device A, Sonnet)

You are lane A126 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, and the track's two evolutions in your worktree: `status/A107.md`, `status/A116.md`, `docs/experiments/SOAK-2.md`, `SOAK-DRIFT.md`,
`tools/soak/cells.mjs`, `tools/soak/drift.mjs`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a107` (the track's; evolutions 1 and 2 are committed there as the baseline). It has no binaries of its
own. Your status file: `status/A126.md`. Family: `tools/soak/**`, `tools/gate/premerge.ps1` (one new step only), the playtest line's fields in
`Game/Source/JamCore/AI/JamPlaytester.cpp` (item 2 only), reports under `docs/experiments/`.

## What changed under you
Main moved a long way since the build your baseline was taken on: a carried puck now leaves its carrier only by a press (every win rate rose 13 to 23 points), the
boards count as a body, a release grace of 120 frames, and the difficulty levels are being re-set by the curve lane tonight. Your committed `baseline.json` is
therefore the old game's. The built checkout `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\clean-main` is yours alone now: the Director has moved it to the newest
main (the commit is in the message that starts you) and you build it (`tools/run/build.ps1 -Project Game -Target Editor -Config Development` inside it: the build
tool serialises builds across the machine, so the wait can be long: wait it out).

## Evolution 3: the soak follows main
1. **A fresh baseline on the newest main** (`tools/soak/baseline.json`, 100 seeds a cell as before), and the table of what moved against the old one
   (`docs/experiments/SOAK-3.md`): every mode's character number, every take's win rate by class, knockdowns of the human, the idle share. This is the first full
   reading of the game with the puck kept: write it for the curve lane (C0) and the modes lane (M1), who are working on the same commit.
2. **`"stumbles"` in the playtest line** (`JamPlaytester.cpp`, next to `knockdowns_all`, counted from the human's `StumbleFrames` going from 0 to above 0; no
   hash moves) and a **puck-holder summary** in the line (the share of frames the human's side, the other side and nobody holds the puck) so the idle read-out
   needs no trace run. This needs a build of your own tree: copy the changed file into `clean-main` only after its baseline run is done and say so (or build in
   your worktree if you prefer: a full build).
3. **The drift gate as a step of `premerge.ps1`**, run only when the changed paths touch cartridges, the sim or the brain (`--only modes` or `--only takes` as
   fits), under 8 minutes, off by a switch; a no-op change reports 36 of 36 identical and a seeded 12 % change is caught.
4. **A CPU-aware governor** in `cells.mjs` (the load average, not memory alone): the 36,000-match matrix in 20 minutes or less while other lanes build.

When the curve lane lands (the Director tells you), re-take the baseline once more: say in your report how long that takes so it can be a routine.
Time box 100 minutes; interim in the status file. `-Multiprocess` on every headless start.
