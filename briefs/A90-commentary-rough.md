# Lane A90: the commentator calls the hits: A89's sixty lines into the bank with the picker rules they need (device A, from H20, R-0041 point 4, Opus)

You are lane A90 of the Third Jam (team Hokej). Lane A89 wrote the game's rough voice (`docs/experiments/VOICE.md`) and sixty proposed commentator lines
(`tools/commentary/proposed/a89-lines.json`, in the bank's schema, filtered, not merged): twenty for checks and knockdowns by `Strength`, ten for cross-checks, ten for the
human knocked down, ten for a massacre shift, ten for a goal after a hit. Lane A84 added twelve ability lines and an `ability` event. The bank's data-only `when` cannot
express three of the new situations. Your job: the lines in the bank, the picker rules that make them fire, tests, and a bot match's log showing each situation spoken.

Read first: `docs/experiments/VOICE.md`, `COMMENTARY.md` (situations, priorities, `when`, the rules lane A47 added: a bark never cuts a line under 0.5 s old, must-play
lines cut lower ones, the caption clears), R-0033 point 3 (random among candidates, never the last line, prefer unheard), R-0021 (the teen rating; mild words at most one in
ten across the whole bank after the merge: count it), R-0041, `tools/commentary/` (`embed.mjs`, `check.mjs`, `commentary.test.mjs`, the filter under `tools/barks/filter.mjs`),
`Game/Content/Commentary/*.json`, `Game/Source/JamGame/Private/CommentarySystem.cpp` + `Public/CommentarySystem.h`, `CommentaryTests.cpp`, `CommentaryVarietyTests.cpp`,
`contracts/events.md` (the `check`, `knockdown`, `stickhit`/lift read-outs and their Kind and Strength; the `ability` event), `docs/experiments/TACKLING-2.md` (the `JAMVIS`
lines; the stick hit's `bLifted`).

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a90` (created at the current main; build it yourself, unity off; no interactive editor: the commentary runs
headless in a bot match with `-nullrhi`; the `JAMVO id=... event=...` lines are the proof). Run no git command; never touch `Game/Plugins/JamAssets` (the voice recordings are
B's: list the new line ids for B in your report); never print `.env`; never kill a process you did not start. The sim is lane A85's until it lands: you add **no sim rule**;
a cross-check is recognised from what the sim already emits (the stick-lift event or read-out with its result), in the commentary system. `InputMapping`, the HUD and
`HokejGameMode.cpp` are not yours.

You own: the commentary bank files and their embedded data (through `node tools/commentary/embed.mjs`), `CommentarySystem.*` (the picker: `Strength` thresholds for
`check.light` < 0.4, `check.solid` 0.4 to 0.8, `knockdown.big` >= 0.8; a per-team ring of knockdown frames for `knockdown.massacre`: the third by one team inside 1,200
frames; `goal.after_hit`: a goal within 300 frames after the scoring team's check; `check.cross`: the stick lift that lands), `CommentaryTests.cpp` (a test per new rule),
`tools/commentary/check.mjs` if a new situation needs listing, `docs/experiments/COMMENTARY.md` (a section for the rough pass), `status/A90.md`.

## Job
1. Merge the sixty lines into the bank (ids kept, situations as proposed or renamed to the bank's convention: say which); `check.mjs` and the filter green; the mild-word
   share of the whole bank counted and under one in ten.
2. The four picker rules with a test each (`Jam.Commentary.CheckByStrength`, `Massacre`, `GoalAfterHit`, `CrossCheck`); A47's and A50's rules and tests stay green.
3. A 3-minute pro bot match headless: the `JAMVO` lines counted per situation; every new situation spoken at least once, or the reason it never fired (a massacre needs
   three knockdowns in 20 s: if the match never has one, run all_star and Bruisers).
4. `COMMENTARY.md`: the rules, the counts, the list of new line ids for B's recordings.

Report: the counts per situation from the match log, the tests, the mild-word share, the ids for B, the file list. Write `status/A90.md` as you go. Time box 75 minutes.
