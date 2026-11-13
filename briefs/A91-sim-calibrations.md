# Lane A91: the last sim calibrations before the freeze: the full charge under the bar, a human tap that stays flat, the drills staged, the acceptance run green (device A, from H21, R-0044, Fable)

You are lane A91 of the Third Jam (team Hokej). The sim freezes Saturday 12:00 (R-0043). Lane A85 made the player shoot and pass with the blade extended; lane A87's acceptance
run (`tools/qa/accept/accept.ps1`, `docs/experiments/ACCEPT.md`) then showed 17 of 18 verbs green on a stand-in for that fix: the one red verb is the **full-charge shot, which
hits the crossbar** from 10 m at an empty net (`shot.lift.full_m` 0.6 against the 0.61 bar: the puck's radius and the bar's ring take the rest). A85 also measured that a
human's click is never charge 0 (a 40 ms click arrives 9 to 20 cm up), that full holds in drill 10 never reached the line (the human is moved 3 to 4 m during the hold), and
that drills 9 and 11 are not staged. The Director's rulings (R-0044): (1) `shot.lift.full_m` 0.5: the top shelf clearly under the bar; (2) a hold of 4 frames or fewer is a tap
(charge 0, flat): `game.shot.tap_frames` 4, for the human and the bots alike; (3) the mechanics' bars restated to what the mechanic is for: the sweep and the spin are proven
by their shield (pokes and lifts refused inside the window), not by the puck kept against a body check; the saucer by its flight over a blade when one is on the lane.

Read first: R-0042, R-0043, R-0044, `docs/experiments/MECHANICS-3.md` (A85: the tables, "for the Director", the unexplained full charge), `ACCEPT.md` (A87: the ten steps, the
control run, the crossbar), `LIFTED-PUCK.md` (`LiftForCharge`, the bar's ring `BarRingM` 0.05 and the post radius), `Game/Source/JamCore/Private/JamSim.cpp` (`StepCharge`,
`TryShoot`, `ApplyShotLift`, `LiftForCharge`, `StepPuck`'s crossbar test), `Game/Source/JamCore/Private/Tests/JamLiftedPuckTests.cpp`, `JamGoalHeightTests.cpp`,
`Game/Source/JamGame/Private/HokejGameMode.cpp` (drills 9, 10, 11: A85 staged 10), `HokejAcceptDrill.cpp` (drill 20), `tools/qa/ctrl/a85-runs.ps1`, `a85-analyze.mjs`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a91` (the Director creates it at the commit that carries A85, A87 and the 5 v 5 landing; build it yourself, unity
off; the Development game package with `-WaitMutex`; no interactive editor). Run no git command; never touch `Game/Plugins/JamAssets`; never print `.env`; never kill a process
you did not start. The acceptance run needs the machine's cursor: run it when no other lane's game is up (its verdict warns otherwise) and repeat a run that was disturbed.
Lane A86 tunes the difficulty profiles and the playtester's stick at the same time: do not touch `DifficultyProfile.cpp`, the pro defaults in `JamAICore.h` or
`JamPlaytester.cpp`. A sim change is a calibration: the hash before and after; the full `Jam` filter green.

You own: `JamSim.cpp/.h` (the two calibrations; the cause of the moved carrier during a full hold if it is the sim's), `JamCartridge.*` + `schema/` (`shot.lift.full_m` 0.5,
`shot.tap_frames` 4; the three generators), the lift and goal-height tests (expectations moved with the ruling, each written), `HokejGameMode.cpp` drills 9 and 11 (staged
like 10; the saucer's cue requires a blade on the lane), `tools/qa/ctrl/a91-*`, `docs/experiments/MECHANICS-4.md`, `docs/experiments/img/a91/**`, `status/A91.md`.

## Job
1. The two calibrations with their tests; the hash before and after; the full filter.
2. The full charge that never reaches the line in drill 10: reproduce, find what moves the human during the hold (bots through a standing carrier, the drill's own staging,
   the carrier's 0.92 speed under thrust), fix what is yours, report what is not.
3. Drills 9 and 11 staged; the five runs (A85's scripts) with the bars as R-0044 restates them: sweep and spin by shield (every poke and lift inside the window refused), the
   saucer over a blade 10 of 10 when the cue has a blade on the lane, shot heights 10/10/10 at the line within 8 cm, five one-timers.
4. The acceptance run on your Development package: all ten steps green, exit 0; the verdict table in your report. This is the gate for H25.
5. `MECHANICS-4.md`: the calibrations, the tables against A85's, the acceptance table, the hash.

Report: the acceptance table, the bars' counts, the hash, the full-filter result, the file list. Write `status/A91.md` as you go from the first minute. Interim at 30 minutes
with step 1. Time box 90 minutes.
