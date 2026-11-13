# Lane A139, track M1 (the Director's modes), evolution 2 (device A, Fable)

You are lane A139 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, `docs/VISION.md`, R-0036, R-0041 and R-0048 to R-0053 in `contracts/RULINGS.md`, and the track's doc and status:
`docs/experiments/MODES-3.md`, `Game/Content/Cartridges/modes/MODES.md`, `status/A122.md`, `tools/playtest/modes.mjs`.

Your worktree and the commit it stands on are named in the Director's message that starts you: a **fresh worktree at the newest main**: it has the kept puck,
the release grace (`game.contact.release_grace_frames`, 120), the take grace (`game.stick.take_grace_frames`, 30), the harder stop and the earned first step,
your own first evolution, the arena that answers a goal. It does **not** have the brain's second evolution or the re-set ladder: both are in an integration
lane and a curve lane tonight and will move every win rate again. So, as before: judge a mode by its **character number against the regular game on the same
build**, report win rates as read, and never put a level's numbers in a mode (a level name only). It has no binaries: build the editor first. Your status
file: `status/A139.md`. Port 4739. Family: `Game/Content/Cartridges/modes/**`, the cards' text (`game.meta`), `tools/playtest/modes.mjs`,
`docs/experiments/MODES-4.md`.

## Evolution 2: the passing modes pass, and each mode uses the vision's new levers
1. **Pond and Power Play pass again**: both are passing modes that now pass less than the five-a-side regular game (0.38x and 0.49x: they read "a third
   apart" only because the test is two-sided). Passes plus receptions per play minute at 1.33x the regular game or more for both, each still one sentence
   on its card.
2. **The two grace keys where a mode wants them** (your own notes): Cage and Bruisers with a short release grace (45 to 60: the box and the brawl keep
   hitting), Outnumbered with a long take grace (60 to 90: keeping the puck under pressure is the mode), Sudden Death with none (one mistake ends it); confirm
   the Shotgun's blast and the release grace together (the blast passes over a skater inside its grace: is that still Shotgun? turn the grace down there if
   the mode fades). Each mode keeps or gains its third against the regular game.
3. **Marathon's pace**: its 2.29x is only its length; per play minute the human sprints as in the regular game. Make the marathon a game of legs (stamina,
   the sprint's price, the rink): sprint frames per play minute at 1.33x or more with a regular player winning 25 % or more.
4. **One table for the Director**: all twelve, on this build: the character number and its ratio, the regular class's win rate, the match length, knockdowns
   of the human, and one line a mode: is it fun on paper, and what would make it so. The modes that are too easy at rookie (Arsenal, Bruisers, Pond at 94 to
   99.5 %) are the curve lane's until the ladder lands: say which level name each should carry once rookie is near 85 %.

The cartridges pass both validators; the menu test counts twelve cards; the full `Jam` filter green. Card text in the brand's voice (raw, teen rating).
`-Multiprocess` on every headless start; few processes, many seeds. Time box 100 minutes; interim in the status file.
