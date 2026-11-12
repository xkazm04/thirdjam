# Lane A17: cuts as progression: the season of takes (device A, from H7)

You are lane A17 of the Third Jam (team Hokej). Read first: `contracts/depth.md` (C-0005 section 1 and the `game.ladder` object), `GAME.md`, `docs/THEME.md` (every match is a cut; the
Director changes the cut between takes), `docs/HOCKEY.md` (3), `docs/PLAN.md` 2 (judging: "level design and game progression" is a 5-point criterion), `Game/Content/Cartridges/*.json`,
`Game/Source/JamGame/Private/MenuFlow.cpp` and `MenuWidget.cpp` (A8, finished: one new entry "Play the season"), `Game/Source/JamGame/Public/DirectorsCallController.h` (the TAKE card),
`Game/Source/JamDesigner/Public/JamDesignerSubsystem.h` (`ListCuts`, `LoadCut`, `SaveCut`), `Game/Source/JamGame/Public/HokejGameMode.h` (win/lose, `period` events),
`docs/experiments/BALANCING.md` (difficulty per level), `briefs/A14-effects.md` and `A15-rules-arena.md` (the primitives your later takes will use once they land; design the ladder so the
first four takes need nothing new).

You own what C-0005 section 1 says. Run no git command; editor on MCP port **8004** for proofs; link windows; keep the tree compiling. Never kill a process you did not start.

## Deliverables
1. **Design** (`docs/experiments/LADDER.md`, first 30 minutes): a season of 8 takes, each one cut with ONE new twist and a one-line "the director's note" shown on the TAKE card, difficulty rising
   rookie -> pro -> all_star across the season, the twist text written in the theme's voice, and what the player unlocks (the next take; the cut itself appears in "Choose your cut" once played).
   Takes 1-4 use only what exists today (regular, raw, boost built-ins, large/wide rinks, difficulty, look tier); takes 5-8 use A14/A15 primitives (custom effect, rule variant, zones, chaos) and
   ship only if those lanes land (each later take has a fallback cut named).
2. **Data:** `Game/Content/Cartridges/ladder/season.ladder.json` (`game.ladder` schema object in its own file; add the schema to `schema/` as `ladder.schema.json` with a validator entry and tests)
   plus the 8 take cartridges as presets in `Game/Content/Cartridges/ladder/take-N.cartridge.json`, each validated.
3. **`UJamProgression`** (JamGame subsystem): loads the ladder, tracks the current take and results in `Saved/Progress.json` (unknown keys are errors; absent = start), decides unlock, provides
   the next take, exposes `GetSeasonState()` for the HUD/menu; "Play the season" in the menu starts take 1 or resumes; after a match, "NEXT TAKE" (on win) or "RETAKE" (on loss) on the win/lose
   card (hook A5's cards through the game mode's existing restart path; say what you touched); the TAKE card shows the director's note.
4. **Tests:** `LadderValidates`, `ProgressionUnlocks` (win -> next, loss -> same, resume), `SeasonRoundTrip` (all 8 takes load and the bot suite passes on takes 1-4 at their levels).
5. PIE proof: take 1 to take 2 transition with screenshots.

Report: the ladder table, what shipped, the menu/game-mode seams you touched, every error. Time box 3 hours; report at 60 minutes with the design.
