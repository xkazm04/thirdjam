# Lane A81: Director's modes pass 2: six more modes with the new keys, and the menu row that holds twelve (device A, from H17, R-0036, Opus)

You are lane A81 of the Third Jam (team Hokej). Lane A67 shipped six Director's modes and listed six more ideas with the keys each needs (`docs/experiments/MODES.md`); lane
A72 landed most of those keys (`ai.react_frames`, `ai.body_check`, `ai.sprint`, `rules.penalty_frames`, a ten-minute clock; `MODE-KEYS.md`). The operator's words (R-0036):
"ways/modes to change character of the game for creative variations of possibilities." Your job: six more modes that play, each a different character, proven the way A67
and A72 proved theirs; the menu row grows to two rows of six without crowding.

Read first: `docs/experiments/MODES.md` (the six shipped, the six ideas, the asks), `MODE-KEYS.md` (the keys landed, the ones not: `puck.count` design only, offside not
started), `Game/Content/Cartridges/modes/*.cartridge.json` + `MODES.md` there, `tools/playtest/modes.mjs` (the measurement: 20 seeds, the regular playtester at the mode's
level, a character number against regular), `schema/README.md`, `DEFAULTS.md`, `limits.json`, `Game/Source/JamGame/Private/MenuWidget.cpp` + `MenuFlow.cpp` (the row of six
cards; the pitch and facts live in a C++ table: A67's ask A11), `docs/experiments/CURVE-2.md` (the profiles moved: measure against today's regular), R-0037 (the horn; a timer
match's hard stop is the clock plus 6,000).

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a81` (created at the current main; build it yourself, unity off; one editor on MCP port 8040 only for the
menu still; drive it by a console command, not by calling the widget from Python: A67's editor crashed at StopPIE that way). Run no git command; never touch
`Game/Plugins/JamAssets`; never print `.env`; never kill a process you did not start. No sim code; a key that does not exist is an ask. Lane A79 changes the all_star profile
and lane A77 the pass rule at the same time: measure against the regular cartridge on your own build and say so.

You own: six new `Game/Content/Cartridges/modes/*.cartridge.json`, `modes/MODES.md`, the menu's two rows (`MenuWidget.cpp`, `MenuFlow.cpp`: twelve cards in two rows of
six, or one row that scrolls, whichever reads at 1080p and 720p; the pitch and facts moved from the C++ table into a `game.meta { title, pitch, facts[] }` cartridge object
if the schema lane's rule allows an informational object: ask in the report if not and keep the table), `tools/playtest/modes.mjs` (the new modes' character numbers),
`docs/experiments/MODES-2.md`, `docs/experiments/img/modes2/**`, `status/A81.md`.

## Job
1. Six modes from A67's ideas or your own, each on a different character number: suggestions: **Sudden Death** (first goal wins, all_star, big rink), **Outnumbered** (3 v 5
   with the human, rookie bots that sprint), **Glass House** (penalties long, body_check 1, the box the character), **Pinball** (restitution high, pillars, puck fast),
   **Tap-In** (goal height 1.5, shot speed low, pads 0: scoring is easy, passing decides), **Fog of War** (react_frames 40 for the bots, the human's check from 1 m/s, a
   night tier). Replace any that does not move its number by a third.
2. Each validated (Node validator, `Jam.Core.CartridgeRoundTrip`), each measured (20 seeds), each with a card (name, pitch, three facts) and the match card naming it.
3. The menu with twelve cards: stills at 1080p and 720p; eyes `readability` on both.
4. `MODES-2.md`: the twelve modes' numbers in one table, the asks that remain, and a sentence per mode on what a player feels.

Report: the six modes with their numbers, the stills, the asks, the file list. Interim at 45 minutes with three modes. Time box 100 minutes.
