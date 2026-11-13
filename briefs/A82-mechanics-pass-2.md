# Lane A82: mechanics pass 2: the lift shielded by the spin and the sweep, the saucer's lane, the drills fixed, the bars re-proven (device A, from H18, R-0040, Fable)

You are lane A82 of the Third Jam (team Hokej). Lane A77 (`docs/experiments/MECHANICS.md`) proved the input path and the read-outs in the package but not the kept-puck bars:
against the pro brain the **stick lift at contact** (`TryLift`, 1.7 m body to body, no shield) took the puck inside the sweep's and the spin's shield windows (19 and 11 clean
lifts); the saucer's lane margins were too tight for live bots (4 saucers of 24 holds, 3 taken at 8 cm up); drills 10 and 11 never handed the puck to the human. The
Director's decisions (R-0040): the spin shields the lift fully (the body is between) and the sweep shields the lift on the side the puck left for `shield_frames`, the same
rule as the poke; the saucer's lane margin goes to 0.9 m with the airborne window from 1.5 m along; protocol 6 stays.

Read first: `MECHANICS.md` (every count, "what went wrong", the three fixes named), `STICK.md` (A51: `StepSweepShield`, the shield rule in `ResolvePokes`), `LIFTED-PUCK.md`,
`Game/Source/JamCore/Private/JamSim.cpp` (`TryLift`, `ResolvePokes`, `StepSweepShield`, `PickReceiver(bSaucer)`, `ApplySaucerLift`, `SaucerLaneMinM`, `BladeClearM`),
`JamAICore.cpp` (the saucer hold), `JamPlaytester.cpp` (the saucer use), `HokejGameMode.cpp` (drills 7-11, `MechDrillMate`, `DrillInput`, the still capture), `tools/qa/ctrl/
a77-prove.ps1`, `a77-analyze.mjs`, `a77-strip.py`, `a77-cut*.json`, `Game/Source/JamCore/Private/Tests/JamStickHandlingTests.cpp`, `JamSaucerTests.cpp`, R-0034, R-0040.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a82` (created at the current main; build it yourself, unity off; the Development game package with
`-WaitMutex`; no interactive editor). Run no git command; never touch `Game/Plugins/JamAssets`; never print `.env`; never kill a process you did not start. The eyes tool is
out of credits (Gemini 402): read your strips yourself and say so. A sim change is a calibration: the hash before and after written; A51's, A52's, A77's tests green.
Lane A79 changes the all_star profile in parallel: do not touch `DifficultyProfile.cpp`.

You own: `JamSim.cpp/.h` (the lift shield, the saucer margins), `JamStickHandlingTests.cpp` (`Jam.Core.SpinShieldsLift`, `SweepShieldsLift`), `JamSaucerTests.cpp` (the margin),
`HokejGameMode.cpp` (drills 10/11 hand the puck by `SetPuck` on a cvar; the sweep capture limited to drill 7; a `JAMSHOT` release line with `ChargeFrames` and `ShotReadyFrame`),
`tools/qa/ctrl/a82-*`, `docs/experiments/MECHANICS-2.md`, `docs/experiments/img/a82/**`, `status/A82.md`.

## Job
0. **First, the gate**: with A77 applied, `Jam.Bot.SeasonTakes` fails on `take-1 (The Read-Through) seed 2: 2:0 in 4427 frames, AI shots 0` (rookie; the clause: the AI shoots
   at least once in a match of a minute or more). Before A77 the same seed passed. Find what A77 changed for a rookie bot's shot or pass (the pass hold `bPassHeld` and
   `StepPassCharge` on a bot's pass edge, `PickReceiver(bSaucer)`, the blade-clear rule, the brain's saucer hold at pro only) and fix the cause, not the test; the Director
   lands A77 together with your fix, so the whole `Jam` filter must be green on your tree (`tools/gate/premerge.ps1`'s invocation, 115 tests + yours).
1. The lift shield: spin = no lift takes the carried puck; sweep = no lift from the side the puck left during `shield_frames`; tests; the hash before and after.
2. The saucer's lane: `SaucerLaneMinM` 0.9, airborne from 1.5 m along; the drill cue and the brain use the same numbers; `Jam.Core.SaucerPass` still green.
3. Drills 10 and 11 hand the puck; the unexplained unfired taps found (`JAMSHOT` line) and fixed if A's.
4. The five runs again on your package: sweep kept 10 of 10 attempts (the bar), spin with the puck 10 of 10 and kept, saucer 10 over a blade and landed before the receiver,
   lifted shot heights 10/10/10 against `shot.lift`, five one-timers. A bar still not met is reported with the count and the cause, never tuned past the ruling.
5. `MECHANICS-2.md`: the table against A77's, the strips, the hash.

Report: the counts against the bars, the strips, the hash, the file list. Interim at 40 minutes with steps 1-3 in the sim. Time box 100 minutes.
