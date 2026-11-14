# Lane A153, track Q2 (the brain), evolution 4 (device A, Fable)

You are lane A153 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/VISION.md` (law 2), R-0053 to R-0059 at the end of `contracts/RULINGS.md` (your worktree's copy is the newest: R-0058 point 1 is your charter), the
track's docs `docs/experiments/BRAIN-2.md`, `BRAIN-3.md`, `status/A134.md`, the curve's `docs/experiments/CURVE-7.md` (the ladder as set: 85.3 / 49.1 / 29.6 on
the regular rink, 84.8 / 49.9 / 26.4 on the large, at 2,000), track F2's `docs/experiments/MOVEMENT-5.md` (the carry drill takes its stop distance from the
level: all_star 12 m today), and the design eye's `docs/design/REVIEW-2.md` (items 4, 6, 7 and 10 are the brain's).

Your worktree and the commit it stands on are named in the Director's message that starts you: the track's worktree moved to the newest main (your third
evolution landed). It was built before: an incremental build is enough. Your status file: `status/A153.md`. Port 4753. Family: `JamAICore.cpp/.h`,
`JamBrain.cpp`, `JamSimAdapter.cpp`, `DifficultyProfile.cpp` for the one all_star line and your tune keys (no other level's number), the bot tests,
`JamCarryDrill.h` for the drill's all_star distance only, `docs/experiments/BRAIN-4.md`.

## Evolution 4: one stop distance at every level, and a mate that finishes
1. **All_star's presser stays on its job inside the hunt** (R-0058): `Opp.Hk.bPressInHunt = true` in all_star's profile (your measurement: the stop at 8 m
   passes 100 % at all_star, 9.5 m 90 %; the regular cell moved 39.3 to 38.3 before the ladder was set). On this tree, with the ladder set: law 3 at all_star
   (the stop at 8 m at 80 % or more; a straight charge 0), all_star's two cells at 600 (regular and large: within 4 points of 29.6 and 26.4, or say the one
   profile number that pays it back and the curve lane's rule for it: aggression first), and `Jam.Core.Movement.StopAndGo`'s all_star assertion updated to
   the new bar (it asserts the best of four distances today). The carry drill's all_star distance goes back to 8 m and its on-screen line to five stick
   lengths (`StopAtForLevel` in `JamCarryDrill.h`; drill 21 script 1 at 18 of 20 or better at all three levels, headless).
2. **A mate inside 15 m with an open lane shoots** (the review's item 10: mates release from inside 15 m 0.4 times a match; passes received by the human a
   minute at pro fell from 1.73 to 1.54): the human's side finishes its chances and finds him: mates' shots from inside 15 m a match, passes received by the
   human a minute, the puck still his two seconds after a mate's pass (19 %: the bar is 30). For the human's side only; measure the three cells: a rule that
   moves one more than 4 points ships neutral.
3. **Take 2 and the big ice** (the review's item 7 and R-0055 point 5): the human's side wins a kept puck back on 120 m: your third evolution took the
   large-rink share of matches with a 30 s stretch without the puck from 39.2 to 32.0 %: the bar stays 25 %. The candidates you named: a carry clock for an
   opposing three (attack or move the puck within N seconds); the same cell with the scripted human sprinting (`-playersprint`: a person has Shift). One of
   them, measured on 600.
4. Not yours tonight: where the opposing side shoots from (the long shot is a ladder dial: the curve track's, after the operator has played).

**From 08:00 nothing that moves the ladder or a verb lands without an acceptance run on its own package (R-0056)**: write `BRAIN READY` in the status file with
the file list as soon as item 1 is green and measured: the Director lands it before that hour; items 2 and 3 follow. Neutral tunes in `JamBaseline.h`. Law 3
is a gate at pro too (the stop at 8 m 100 %, a straight charge 0: unchanged). `-Multiprocess`; three measurement processes at most; check memory first.
Time box 100 minutes.
