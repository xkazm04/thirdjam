# Lane A57: Development against Shipping parity, then an hourly package from main with a smoke launch (device A, night filler, Sonnet)

You are lane A57 of the Third Jam (team Hokej). PIE and Development builds hid for a whole evening that the Shipping package's controls were broken (R-0032). Your job:
prove the two packaged configurations run the same game, then keep packaging main every hour so the morning has the latest build smoke-tested and measured.

Read first: `docs/experiments/CONTROLS.md` (A49: the Development package and SendInput harness `tools/qa/ctrl/`), `tools/run/package.ps1`, `tools/run/run-packaged.ps1`,
`tools/run/smoke.ps1`, `tools/size/**` (size history), `Game/Source/JamCore/AI/JamPlaytester.cpp` (`-player=`; check which flags survive `UE_BUILD_SHIPPING`),
`Game/Source/JamGame/Private/HokejGameMode.cpp` (the `jam.*` commands and their Shipping guards; the match log lines `JAMHASH` or the state hash print if one exists; if no
hash reaches a Shipping log, the parity test uses the final score, the event counts and the match length from whatever the Shipping build does write, and says so),
`status/A.md` (the `link green` lines are the commits to package), R-0022.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a57` (created at the current main). Run no git command except the read-only `git -C C:\Users\kazda\kiro\thirdjam log -1
--format=%h main` and `git -C <your worktree> checkout --detach <sha>` to move your worktree to the commit you package (those two only). Never touch
`Game/Plugins/JamAssets`; never print `.env`; never kill a process you did not start (the games you launch are yours). Open no interactive editor. Packages use
`-ExtraArgs "-ubtargs=-WaitMutex"`; other lanes build concurrently.

You own: `tools/qa/parity/**`, `tools/qa/nightly/**`, `docs/experiments/PARITY.md`, `docs/experiments/NIGHTLY.md`, `status/A57.md`. Packages go to
`Game/Packaged/Hokej_Hokej_N<HHMM>/` (N for nightly, never the Director's H numbers).

## Job
1. Parity: package your worktree's commit in Development and in Shipping; run the same five seeds of a bot match (`hockey.regular`, pro) in both for 3600 frames with
   whatever drives a match without a human in each (the playtester, `jam.ScriptHuman`, or a bot-only mode: say which survives Shipping); compare the state hash per 600
   frames if a hash is logged, else score, goals, checks, saves, posts, match length. Any difference is the finding of the night: the frame, the first differing line.
2. Nightly loop until 06:30: every 60 minutes read main's head; if it moved since the last package, move your worktree there, package Shipping, zip, smoke-launch for 30 s
   (process alive, memory, no `LoadErrors`, no `missing usage flag` in the cook log), record the size into the size history, append a row to `NIGHTLY.md` (sha, time,
   size, cook warnings count, smoke result). Hold the loop when free memory is under 10 GB and say so in the row.
3. `PARITY.md`: the method, the five-seed table, the verdict (identical / differs at frame N / not comparable because ...), and the list of `jam.*` commands and flags
   that Shipping compiles out (this list is what every proof lane needs).

Report: the parity verdict, the nightly table, the file list. Interim at 60 minutes with the parity result. Time box: until 06:30.
