# Lane A121, track F2 (carrier skill), evolution 2 (device A, Fable)

You are lane A121 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, `docs/VISION.md` (laws 3 and 4), R-0048 to R-0050 at the end of `contracts/RULINGS.md` (from the main checkout), device B's play-through
(`C:\Users\kazda\AppData\Local\Temp\tj\B-to-A-0038.md`, point 2), and the track's first evolution in your worktree: `status/A102.md`, `docs/experiments/MOVEMENT-2.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a102` (the track's: evolution 1 is committed there as the baseline and is landing; its editor is built).
Your status file: `status/A121.md`. Port 4721 is taken by another lane: use 4722. Family as the track's: the movement and stick functions in `JamSim.cpp/.h`, the
`game.skater` and `game.stick` keys, the carry drill (`JamCarryDrill.h`, drill 21 in `HokejGameMode.cpp`), `Jam.Core.Movement.*` tests, `docs/experiments/MOVEMENT-3.md`.

## Evolution 2: the carrier's answer inside ten metres
Lane A102 measured that a stop-and-go beats a pro defender only when the stop begins 9.5 m out (100 %); at 8 m it is 25.5 %, at 6 m 2.5 %, and the first-step burst
helps the charging defender more than the carrier (at 8 m: 66 % with no burst against 25.5 % with it). A player who reacts late has no answer. Three rules, each
with a neutral value, each measured on the carry scene (200 seeds an arm, real brain, pro and all_star):
1. **Take grace** (R-0049 point 2): `game.stick.take_grace_frames` (default 30, neutral 0): a skater that has just won the puck cannot be lifted or poked for that
   many frames; a body check still loosens it. Proof: a carrier keeps the puck at least half a second between stick takeaways; lifts and pokes per match before
   and after (device B counted 79 lifts and 68 pokes in 104 s).
2. **The stop's grip**: `skater.reverse_brake_decel` 20 to 26 (your measurement: 8 m rises from 25.5 % to 57 %); re-pin the stop timings lane A93's tests hold.
3. **The first step belongs to the carrier**: the burst arms only after a stop or a cut of the skater's own (not on a straight charge), or the carrier gets its own
   multiplier: choose by the numbers. Bar: a stop begun at 8 m passes a pro at least 55 %, 9.5 m stays at 95 % or more, and a straight charge stays under 20 %.
4. If time remains: **a late answer inside 7 m**: a spin and a deke script in the carry drill, their windows tuned so that from 6 m the carrier passes a pro at
   least one time in three. Otherwise it is your first "next".

The lockstep hash: lane A110's coverage test (`Jam.Net.HashCoverage`, landing now) fails when a skater field is neither hashed nor listed: any counter you add
(the grace) goes into `LockstepStateHash`; if the test is not yet in your tree, say in your report which fields you added.

Measure the three target cells before and after (600 a cell, paired seeds, `-Multiprocess`): they are off already (your evolution 1 moved them 13 to 23 points;
the curve track re-sets after you): report the move of this evolution alone and do not tune the levels.
The gate: the full `Jam` filter green; the neutral block gives back your baseline's hashes. Time box 100 minutes; interim in the status file.
