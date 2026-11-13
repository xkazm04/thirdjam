# Lane A87: the acceptance run: a human hand plays the first five minutes in the package, pass or fail per verb (device A, from H20, R-0043, Opus)

You are lane A87 of the Third Jam (team Hokej). Six packages (H19 to H24) shipped with a fault no gate could see: the player could not shoot or pass while aiming (R-0042),
because every gate drives `FJamInput` directly (bots, playtester classes) and every package proof checked a verb's read-out, never a goal scored through the cursor and the
buttons. Your job (R-0043 point 1, `docs/PLAN-RECONCILED.md`): one script that plays the first five minutes of a match in a packaged build through the real input path and
says pass or fail for each verb the player has. It must **fail today** on shooting and passing with the blade extended (main 8fc69d3) and pass once lane A85's fix lands.

Read first: `docs/PLAN-RECONCILED.md`, R-0042, R-0043, `docs/experiments/MECHANICS-2.md` (A82: the `JAMSHOT release` line with `puck_dist`, `reach`, `charge_frames`; how the
runs were driven: `tools/qa/ctrl/a82-prove.ps1`, `a82-runs.ps1`, `a82-analyze.mjs`: `SetCursorPos` + posted `WM_MOUSEMOVE`, posted key and button messages, the remote
console `-RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=4658`), `CONTROLS.md` (A49: the bindings; `JAMCTRL` lines per input; `CursorAimRink`), `TACKLING-2.md` (A73: drills 5
and 6, `JAMVIS` lines), `Game/Source/JamGame/Private/HokejGameMode.cpp` (the drills `jam.ctrl.Drill N`, `DrillInput`, `MechDrillMate`, `SetPuck` on a cvar), the match log
lines for goal, shot, pass, reception, check, knockdown, get-up.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a87` (created at main 8fc69d3; build the editor and the Development game package yourself with `-WaitMutex`;
no interactive editor). Run no git command; never touch `Game/Plugins/JamAssets`; never print `.env`; never kill a process you did not start. A foreign "Windows Security"
dialog may hold the foreground: posted messages and the remote console, as A82 did. Lane A85 edits the sim's carrier reach and the drills' hand-puck in `HokejGameMode.cpp`
at the same time: add your own drill number and your own functions; do not edit drills 7-11.

You own: `tools/qa/accept/**` (`accept.ps1`: launch, drive, collect; `accept.mjs`: the verdict table and the `thirdjam.accept/1` JSON line last; exit 0 pass, 1 fail, 2 could
not run), one drill in `HokejGameMode.cpp` (`jam.ctrl.Drill 20`: an empty rink with the human, one mate, one opponent, the puck handed to whoever the step needs), a
`JAMACCEPT step=<n> verb=<v> ok=<0|1> <facts>` line per step, `docs/experiments/ACCEPT.md`, `status/A87.md`. It is not a unit test: it runs on a package, and
`tools/run/package.ps1`'s caller (the Director's landing script) runs it after packaging.

## The steps (each through the cursor and the buttons; each with its pass rule)
1. Skate: W for 2 s; the human's speed passes 6 m/s. Sprint: Shift without the puck raises it; with the puck it does not (R-0041 point 6).
2. Aim: the cursor at four points around the skater; the blade's world angle follows each within 15 degrees inside 12 frames.
3. Carry: the puck handed; the cursor swept left to right at full reach three times; the puck is still the human's.
4. Shoot, blade extended (the cursor 6 m ahead toward the net): a tap, a half hold (8 frames), a full hold (30 frames) from 10 m at an empty net: three `JAMSHOT release`
   lines with `charge_frames` 0 / 8 / 30 and three goals.
5. Pass, blade extended: Q (or F until A84 lands: read the binding from `JAMCTRL`) to the mate 8 m away: a pass event and a reception by the mate; a held pass: a saucer.
6. Receive and one-timer: the mate passes back (the drill's mate); the human shoots inside the one-timer window: a goal or a shot with the one-timer flag.
7. Check: RMB into the carrying opponent at 4 m/s: a knockdown, a strip, the get-up 90 frames later. Cross-check: MMB at contact: a `stickhit` and the puck free.
8. Be checked: the opponent (scripted in the drill) checks the carrying human: the human is down, the camera stays on it, it stands at the release.
9. Deke (wheel), spin (E), sweep (the cursor crossing): each read-out fires with the puck kept for 30 frames after.
10. Pause and resume (Esc): the mouse still aims after the resume (A49's first root cause).

## Job
1. The drill, the driver, the verdict table; run on a Development package of main 8fc69d3: steps 4 and 5 must fail with the blade extended (that is the proof the run sees
   what the gates could not); every other step's result written as it is.
2. Run the same on the Shipping package `Game/Packaged/Hokej_Hokej_H24` as far as Shipping allows (no log: say which steps can be judged without one: goals by the score
   read through the remote console if it survives Shipping, else not judgeable, and say so).
3. `ACCEPT.md`: how to run it in one command, the step table with today's results, what each failure looks like in the log.

Report: the step table (today's pass/fail with the facts), the command, the file list. Time box 90 minutes; interim at 45 with the first five steps.
