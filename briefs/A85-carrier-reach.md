# Lane A85: the human can shoot and pass with the blade extended; the tap stays on the ice; the five bars re-proven (device A, from H19, R-0042, Fable, top priority)

You are lane A85 of the Third Jam (team Hokej). Lane A82's `JAMSHOT release` line (`docs/experiments/MECHANICS-2.md`) found why shots, passes and saucers never fire when the
player aims: since A51 the carried puck lives at the blade, 0.4 + `stick.reach_m` 1.2 = 1.6 m from the body, and every carrier test in the sim uses `shot.reach_m` 1.5
(`puck_dist=1.61-1.63 reach=1.50 charge_frames=0`), so `StepCharge` and `StepPassCharge` never charge and `TryShoot` and `TryPass` refuse; the human also keeps losing a
blade-length puck. A66 capped the bots' own blade inside shot reach so bots shoot; the human got no such cap and should not: the cursor aim is the mechanic. The Director's
decisions (R-0042): (1) the carrier's reach for shooting, passing, charging and carrying is `max(shot.reach_m, body radius + stick.reach_m) + 0.05`, one function used by
every carrier test (`CarrierIndex`, `StepCharge`, `StepPassCharge`, `TryShoot`, `TryPass`, the one-timer window); A66's bot blade cap can then go; (2) a **tap stays on the
ice**: `shot.lift.tap_m` default 0 and a charge-0 shot flies flat (no parabola, Z 0), so a rookie bot's tap can be deflected by a stick again (A82's SeasonTakes coin);
`half_m` and `full_m` unchanged; (3) the bars of R-0034 / A51's brief stand and are re-proven on a package.

Read first: `MECHANICS-2.md` (sections 1, 7, 8: the release line, the proposal, the SeasonTakes bisection), `MECHANICS.md`, `STICK.md` (A51's carrier rule), `LIFTED-PUCK.md`
(A52: `LiftForCharge`, `ApplyShotLift`, the tests' expectations: tap 0.05 becomes 0), `Game/Source/JamCore/Private/JamSim.cpp` (`CarrierIndex`, `StepCharge`, `StepPassCharge`,
`TryShoot`, `TryPass`, `PickReceiver`, `ApplyShotLift`, `LiftForCharge`), `JamAICore.cpp` (A66's `StickPlay` blade cap: `shot.reach_m - 0.1 - 0.4` of full reach; the brain's
`D < W.ShotReach` tests), `Game/Source/JamCore/Private/Tests/JamLiftedPuckTests.cpp`, `JamGoalHeightTests.cpp`, `JamStickHandlingTests.cpp`, `JamSaucerTests.cpp`,
`Game/Source/JamCore/Tests/SeasonBotTests.cpp`, `tools/qa/ctrl/a82-prove.ps1` + `a82-analyze.mjs` + `a82-runs.ps1` (the five runs), `HokejGameMode.cpp` (drills 7-11, `JAMSHOT`).

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a85` (created at the commit that lands A82; build it yourself, unity off; the Development game package with
`-WaitMutex`; no interactive editor). Run no git command; never touch `Game/Plugins/JamAssets`; never print `.env`; never kill a process you did not start. The eyes tool is
out of credits: read your strips yourself. A sim change is a calibration: the hash before and after; every `Jam` test green on the full filter (the gate's invocation).
Lanes A79 (all_star profile), A83 (5 v 5 roles) and A84 (abilities on F, pass on Q) edit in parallel: stay in the carrier reach function, the lift's tap, the brain's reach
tests and the drills; do not touch `DifficultyProfile.cpp` or `InputMapping`.

You own: the carrier reach function in `JamSim.cpp/.h` and its callers, `JamAICore.cpp` (the brain's reach tests use the same number; A66's cap removed if the shot fires
without it: measure `Jam.Bot.SeasonTakes` and `ProCharge`), the tap in `JamSim.cpp` + `JamCartridge.*` + `schema/` (default 0), the tests named above (expectations moved with
the ruling, each change written), `tools/qa/ctrl/a85-*`, `docs/experiments/MECHANICS-3.md`, `docs/experiments/img/a85/**`, `status/A85.md`.

## Job
1. The carrier reach function; every carrier test through it; `Jam.Core.CarrierReach` (a carrier with the blade at full aim fires a shot, a pass and a saucer; the puck at
   1.6 m is still carried through a fast aim change); the human's hand-puck count in a drill run falls (A82: 88 and 77 hand pucks per run).
2. The tap flat: `tap_m` 0, charge 0 = Z 0 the whole flight; `LiftedShot`, `GoalHeight`, `SaveByHeight` expectations updated and explained; `SeasonTakes` take-1 seeds 1-3 green
   on three runs.
3. A66's blade cap: remove it, run `SeasonTakes` and `ProCharge`; keep it only if a bot cannot shoot without it, and say why.
4. The five runs (A82's scripts): sweep kept 10 of 10 attempts, spin with the puck 10 of 10 and kept, saucer 10 over a blade, shot heights 10/10/10, five one-timers; the
   bars reported with the count and the cause when not met.
5. `MECHANICS-3.md`: the reach rule, the tap, the table against A82's, the strips, the hash before and after.

Report: the counts against the bars, the hash, the test results on the full filter, the file list. Interim at 30 minutes with steps 1-3 in the sim. Time box 90 minutes.
