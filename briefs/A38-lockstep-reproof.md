# Lane A38: lockstep multiplayer re-proven on the new sim (device A, from H11.5)

You are lane A38 of the Third Jam (team Hokej). R-0028 point 3. The JamNet lockstep (invite code, input delay 3, hash every 60 frames) was proven at H6 on a sim that has since gained
the faceoff phase, the stick verbs, fouls, effects, rule hooks, zones, the net slide and new profiles. Read first: `docs/experiments/MULTIPLAYER.md`, `Game/Source/JamNet/**`,
`Game/Source/JamCore/Public/JamSim.h` (`FJamInput` now carries stick, poke, lift, pass, charge), `contracts/RUNNER.md`-equivalent here: `docs/HOCKEY.md` section on multiplayer,
`Game/Source/JamNet/Private/Tests/*` (the two-process headless proof), R-0013, R-0018, R-0028.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a38` (detached at the merge commit; build it yourself; headless; two game processes on localhost are allowed, no
editor). Run no git command.

You own: `Game/Source/JamNet/**`, `tools/net/**`, `docs/experiments/MULTIPLAYER.md`, `status/A38.md`.

## Job
1. Re-run the two-process proof: 9,000 frames, hashes every 60 frames must match, with the new input fields serialized (check the packet layout covers every `FJamInput` field;
   a missing field is a silent desync); the faceoff phase, a foul, a pickup and a rule hook must occur inside the proof (use `hockey.boost` and `hockey.chaos`).
2. Measure: bytes per frame, the worst stall under a simulated 80 ms / 2 % loss link (if the harness has one; else say so), and the time from invite code to the first frame.
3. The menu path: a bot-driven run of "host, join by code, play 60 s, leave" through the real menu (headless automation if the menu is testable; else list the manual steps for the
   operator with the console commands).
4. `docs/experiments/MULTIPLAYER.md` updated with the numbers, what changed, and what breaks lockstep if a future lane adds an input field (a checklist line in `contracts/events.md`
   or `JamSim.h` as a comment, proposed to the Director).

Report: hashes result, bytes per frame, stall, the file list. Time box 60 minutes.
