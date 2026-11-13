# Lane A77: the mechanics proven to their bar in the package, and the saucer pass (device A, from H16, R-0036, Fable)

You are lane A77 of the Third Jam (team Hokej). R-0036 puts gameplay mechanics second after collisions, "each proven in a package through the real input path". Lane A51
shipped the sweep and the spin but fell short of its own bar in the package (7 sweep read-outs, 3 kept; 19 spins accepted but the drill pressed E without the puck; no
stills) because a foreign dialog held the foreground and its first game did not close. Lanes A58, A73 and A74 have since driven packaged games by the remote console
and PostMessage. Your job: prove the sweep, the spin, the lifted shot and the one-timer to their bars with stills, and add one mechanic the lifted puck makes possible:
the saucer pass.

Read first: `docs/experiments/STICK.md` (A51: the model, section 7 "what the next run must change"), `LIFTED-PUCK.md` (A52: Z, VZ, `shot.lift`, the glass, the save),
`PAYOFFS.md` (A39: one-timer, breakaway burst, deke), `CONTROLS.md` (A49: bindings, `tools/qa/ctrl/`), `TACKLING-2.md` (A73: `jam.ctrl.Drill 5/6`, `jam.ctrl.Stills`, the
remote console `-RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=4658`, `a73-prove.ps1`, `a73-analyze.mjs`, `a73-strip.py`), `tools/qa/ctrl/a51-prove.ps1` + `a51-analyze.mjs`,
`Game/Source/JamCore/Private/JamSim.cpp` (`TryPass`, `TryShoot`, `ApplyShotLift`, `ResolvePokes`, the carried puck's offset, `StepStick`), `Game/Source/JamGame/Private/
HokejGameMode.cpp` (`ReadHumanInput`, the drills), `InputMapping.cpp` (F pass, LMB shoot), `schema/` (`game.pass`, `game.shot.lift`), R-0034, R-0035.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a77` (created at the current main; build it yourself, unity off; the Development game package with
`-WaitMutex`; one editor on MCP port 8038 only if a PIE still is needed, closed after; at most three editors on the machine). Run no git command; never touch
`Game/Plugins/JamAssets`; never print `.env` (copy the main checkout's `.env` for the eyes tool without printing it; say so); never kill a process you did not start.
A sim change is a calibration: the hash before and after written; `Jam.Core.Determinism` and A51's and A52's tests stay green.

You own: the saucer pass in `JamSim.cpp/.h` (`TryPass`: a held F charges a lift like the shot: `game.pass.saucer { half_frames 8, lift_m 0.35 }`, the puck flies over a
defender's blade in its path and lands on the ice before the receiver, a tap stays flat), its keys in `schema/` + `JamCartridge.*`, `Jam.Core.SaucerPass`, the brain's
use of it (`JamAICore.cpp`: a pro bot saucers when a defender's blade is in the lane; the same reach rule A66 fixed), the drills `jam.ctrl.Drill 7` (sweep past a defender)
and `8` (spin with the puck) and `9` (saucer over a defender) in `HokejGameMode.cpp`, `tools/qa/ctrl/a77-*`, `docs/experiments/MECHANICS.md`, `docs/experiments/img/a77/**`,
`status/A77.md`.

## Job
1. **Sweep**: ten sweeps past a pro defender in the package with the mouse crossing (`JAMCTRL sweep` read-outs, the puck kept), a 10-second strip of the puck on the blade
   through a sweep (stills every 4 frames; eyes `bug-hunt` on three).
2. **Spin**: ten spins with the puck (E pressed only while carrying: the drill reads the carrier), the puck kept and the stamina bar visible; one spin checked mid-turn.
3. **Lifted shot and one-timer**: ten taps, ten half holds, ten full holds from 10 m: the arrival heights from the log against `shot.lift`; five one-timers off a pass.
4. **The saucer pass**: the sim rule, the keys, the test, the brain's use; ten saucers over a defender's blade in the package (the puck's Z over the blade at the crossing,
   landed before the receiver); the playtester's regular and good classes use it when the lane is blocked.
5. `MECHANICS.md`: one table per mechanic: the bar, the count, the proof path, the still; the saucer's model and keys; what the operator will feel.

Report: the counts against the bars, the still paths, the hash if it moved, the file list. Interim at 40 minutes with steps 1-2. Time box 100 minutes.
