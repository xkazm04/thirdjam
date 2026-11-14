# Lane A149, track M1 (the Director's modes), evolution 3 (device A, Fable)

You are lane A149 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/VISION.md`, R-0036, R-0041 and R-0048 to R-0057 in `contracts/RULINGS.md` (your worktree's copy is the newest), `docs/PLAY-ME.md` (what the operator is told
to try at 10:00 this morning: three modes are on it), and the track's docs: `docs/experiments/MODES-4.md`, `Game/Content/Cartridges/modes/MODES.md`, `status/A139.md`,
`tools/playtest/modes.mjs`, and the curve lane's `docs/experiments/CURVE-7.md` (the ladder as it now stands).

Your worktree and the commit it stands on are named in the Director's message that starts you: the track's worktree, moved to the newest main. **This is the
first main with the whole game and the ladder set**: the brain's second evolution, the shove rule, the standing start, the whistle out of the regular game,
and a ladder where a newcomer wins 85 % at rookie, a regular player 49 % at pro, a good player 26 to 30 % at all_star. It was built before: an incremental
build is enough. Your status file: `status/A149.md`. Port 4749. Family: `Game/Content/Cartridges/modes/**`, the cards' text (`game.meta`),
`tools/playtest/modes.mjs`, `tools/soak/baseline.json` (the modes' cells only, by `drift.mjs --only modes --update-baseline`), `docs/experiments/MODES-5.md`.

## Evolution 3: twelve modes on the real ladder, each winnable and each itself
Every mode number you have was measured before the brain changed and before the ladder was set. The operator will pick modes off the menu at 10:00.
1. **Measure all twelve on this build** (200 seeds, the regular class at the mode's level, plus the novice and the good class at 100): the character number
   and its ratio to the regular game at the same level, the win rate, the length, knockdowns of the human, the share of matches with 30 s without the puck.
2. **Every mode inside 15 to 85 % for the regular class and at 10 % or more for the novice**, by the mode's own levers and its level name (never a level's
   numbers). Your own list from the last report: Fog of War at 87.5, Sniper at 19.5, Marathon at 87, the rookie modes (Arsenal, Bruisers, Pond) at 94 to 99.5:
   say for each what the level name should be now that rookie is 85 % for a newcomer, and turn it.
3. **Every mode still a third apart from the regular game** in its character number after the brain's change (the brain now passes to the human and takes
   loose pucks: the passing modes and the hitting modes may both have moved). Re-make what faded.
4. **"No whistle" is no longer a differing fact** on four cards (Pond, Cage, Bruisers, Marathon: the regular game lost its whistle): each of those cards gets
   a third fact that is true and its own. Glass House is now the only whistle mode: its card should say so in the brand's voice.
5. **The three modes `docs/PLAY-ME.md` sends the operator to** (Shotgun, Cage, Pond): play each headless at the class he is (a regular player) and say in one
   line each whether the mode does what its card says in the first minute; if another three would show the game better, say which and why.
6. Re-take the modes' cells of the soak baseline (`node tools/soak/drift.mjs --only modes --update-baseline`; about three minutes) so the drift gate reads
   this build; regenerate the acceptance matrix's mode cuts (`node tools/qa/accept/make-matrix-cuts.mjs`).

The cartridges pass both validators; the menu test counts twelve cards; the full `Jam` filter green. Card text: raw, teen rating, no brands. `-Multiprocess`
on every headless start; few processes, many seeds; check memory before each batch. Time box 100 minutes; interim in the status file.
