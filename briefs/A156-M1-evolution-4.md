# Lane A156, track M1 (the Director's modes), evolution 4 (device A, Fable)

You are lane A156 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then R-0036, R-0041 and R-0056 to R-0060 in `contracts/RULINGS.md` (your worktree's copy is the newest), `docs/PLAY-ME.md` (the operator is sent to Shotgun, Fog of
War and Outnumbered at 10:00), the track's `docs/experiments/MODES-5.md` and `status/A149.md`, the brain's `docs/experiments/BRAIN-3.md` (for teams under five
the human's mates now check the carrier and the back steps up) and the design eye's `docs/design/REVIEW-2.md` (its item 9 and its modes table).

Your worktree and the commit it stands on are named in the Director's message that starts you: the track's worktree moved to the newest main (your third
evolution landed). It was built before: an incremental build is enough. Your status file: `status/A156.md`. Port 4756. Family: `Game/Content/Cartridges/modes/**`,
the cards' text (`game.meta`), `tools/playtest/modes.mjs`, `tools/soak/baseline.json` (the modes' cells only), `tools/qa/accept/matrix-cuts/mode-*.json`
(generated), `docs/experiments/MODES-6.md`.

## Evolution 4: five a side where the mode allows it, and the puck in the Marathon
Your own findings, in the order the operator will meet them:
1. **Five modes are still three a side in a five-a-side game** (Arsenal, Fog of War, Glass House, Sniper, Sudden Death): the human passes 0.4 to 0.7 times a
   play minute there against 2.8 in the regular game, and the brain's shapes run for a team of five. Take each to five a side **where the mode's own idea
   survives it** (Sudden Death on big ice may want to stay three: say why), keeping it inside 15 to 85 % for a regular player and 10 % or more for a
   newcomer, a third apart in its own number, its card true. Bar: the human's passes at 2 a play minute or more in each mode that moves.
2. **Marathon's puck**: 30 s without the puck in 84 % of matches (the regular game 6 %): `puck.count` 2 (the curve lane found the tournament path may not
   read that key: check it first and say), a mate that gives the puck, or the brain's new step-up on big ice (it is on this main now: re-measure before
   changing anything). Bar: under half the matches, with the sprint still 1.33x and a good player under 95 %.
3. **Arsenal's tricks**: 0.94 abilities used a match and 79 of 200 matches see none: three a match, every match at least one (boosts where the play is, one
   ability held from the faceoff).
4. **Power Play's third** (1.45x) to 1.6x or more; Sudden Death's novice above the floor by more than a standard error.
5. Re-take the modes' cells of the soak baseline and regenerate the matrix cuts, as you did.

**From 08:00 nothing that changes what a player meets lands without an acceptance run on its own package (R-0056)**: these are cartridges, so they may land
until the package is cut at about 07:30: write `MODES READY` in the status file with the file list as soon as items 1 and 2 are green and measured.
The cartridges pass both validators; the menu test counts twelve cards and `Jam.Menu.FitsTheScreen` holds your card texts; the full `Jam` filter green.
`-Multiprocess`; three measurement processes at most; check memory before each batch. Time box 100 minutes.
