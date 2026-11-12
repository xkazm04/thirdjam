# Lane A46: passes reach the human: body reception and a pass-to-human at pro (device A, from H12.6)

You are lane A46 of the Third Jam (team Hokej). Lane A39 shipped the one-timer and measured that a human almost never gets one: 0 to 0.1 per match, because a teammate's pass rarely
lands on the human's blade (`docs/experiments/PAYOFFS.md` sections 3 and 6: a body reception seating the puck on the blade, and `bPassToHuman` at pro). Read first: PAYOFFS.md,
`docs/experiments/FEEL.md` (the pass lead solve for drag, A16's reception rule: the blade counts as carried), `Game/Source/JamCore/Private/JamSim.cpp` (`PickReceiver`, the reception
radius, `TryPass`), `Game/Source/JamCore/AI/JamAICore.cpp` (the support's pass decision, `bPassOut`, the intent `pass`), `AI/DifficultyProfile.*`, `AI/JamPlaytester.cpp`
(how the regular and good players call for a pass), R-0007 (team signals: `pass` and `space` intents), R-0023, R-0029, R-0031.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a46` (the Director creates it at the current main; build it yourself, unity off; headless only). Run no git command.
Every change is a calibration under R-0021 point 1: deterministic, cartridge keys with defaults, recorded in FEEL.md, the Determinism baseline re-recorded if play changes from
frame 1, the playtester grid (`tools/playtest/run.mjs`, 20 seeds, regular rink) before and after, the curve move reported for A3 (not tuned).

You own: `JamSim.cpp` (reception), `JamAICore.cpp` + `DifficultyProfile.*` (pass to the human), `JamCartridge.*` and `schema/` for `game.stick.reception_m` (body reception radius,
default 0.9) and the profile knob `mate.pass_to_human` (rookie 0.6, pro 0.4, all_star 0.2 as a share of the support's pass choices when the human is open), `Private/Tests/JamPassTests.cpp`
(new), `docs/experiments/PASSING.md`, `status/A46.md`.

## Job
1. **Body reception**: a pass that reaches a skater's body within `reception_m` (not only the 0.6 m blade disc) is seated on the blade next frame, with a 4-frame "settle" in which a
   shot is a one-timer if it comes inside A39's window; a pass at over 14 m/s bounces off a body (no settle) unless the blade meets it; the human and bots alike.
2. **Pass to the human**: the support (and the back when the human calls `space` or `pass`) chooses the human as the receiver with `pass_to_human` probability when the human is
   open (lane clear, ahead of or level with the carrier); the brain's existing lane check decides "open". The signal `pass` from the human (R-0007 key 3) raises the share to 0.9 for
   120 frames.
3. Tests: `Jam.Core.PassReception` (a pass to the body seats on the blade; a 15 m/s pass does not; a one-timer inside the settle window fires with Kind `one_timer`), `Jam.Bot.PassesToHuman`
   (20 matches at pro: the human receives at least 1.5 passes per match on average; at rookie at least 2.5).
4. Grid before and after: passes received, one-timers, goals per match, win rates; the fun metrics must move (one-timers from 0.1 to at least 0.5 per match for the regular player
   at pro) and the curve move is reported.
5. `docs/experiments/PASSING.md`: the rules, keys, tests, the grid, and the Designer sentence ("my team passes to me").

Report: the grid, the tests, the file list. Time box 75 minutes.
