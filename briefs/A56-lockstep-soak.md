# Lane A56: the lockstep soak (device A, night filler 4 of 11, Opus)

You are lane A56 of the Third Jam (team Hokej). Joined play (two peers in lockstep, `JamNet`) has only ever run inside the automation tests and one QA session on device B.
Your job: run hundreds of joined matches, inject the faults a real link has, and leave the morning the first desync frame of every failure with what caused it.

Read first: `docs/experiments/MULTIPLAYER.md`, `Game/Source/JamNet/Public/*.h` and `Private/*.cpp` (the packet, `JamNet::Version` 5, `Window`, the two-peer harness in
`Private/Tests/JamNetTests.cpp`: `LockstepTwoPeers`, `PacketRoundTrip`, `InputCoverage`), `Game/Source/JamGame/Private/LobbyGameMode.cpp` (how a real join is made),
`Game/Source/JamCore/AI/JamPlaytester.cpp` (two playtesters can be the two humans), `docs/experiments/STICK.md` section on protocol 5, `tools/gate/premerge.ps1` (how
automation runs headless), `docs/experiments/CONTROLS.md` (packaging a Development game build).

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a56` (created at the current main). Run no git command; never touch `Game/Plugins/JamAssets`; never print
`.env`; never kill a process you did not start. Open no interactive editor. Headless only: either an automation test that runs N joined matches in-process (the existing
two-peer harness extended with a seed loop, loss and delay injection), or two packaged Development games on localhost driven by the playtester, whichever the existing code
supports without new networking. Budget: 6 parallel processes, 8 GB; hold under 10 GB free.

You own: `Game/Source/JamNet/Private/Tests/JamNetSoakTests.cpp` (new; `Bar.Net.*` names so the gate's `Jam` filter does not run them), `tools/soak/lockstep/**`,
`docs/experiments/SOAK-LOCKSTEP.md`, `status/A56.md`. A fix in `JamNet` is yours only when it is a bug the soak proves (a desync, a stall) and `Jam.Net.*` stays green;
say what it was.

## Job
1. `Bar.Net.SoakClean`: 200 seeds x `hockey.regular` pro, two playtesters (regular vs good) as the two humans through the lockstep path, 3600 frames each; the two peers'
   state hashes compared every 60 frames; any difference is a desync with the frame and the first differing field (dump both states).
2. `Bar.Net.SoakLossy`: the same with the harness dropping 2 %, 10 % and 30 % of packets, delaying by 0..6 frames, duplicating and reordering; what the protocol does on
   each (waits, drops inputs, desyncs) and whether it recovers inside the window.
3. `Bar.Net.SoakMismatch`: a protocol 4 peer against a 5 host, an unknown flag bit, a packet of the wrong size: the host refuses with the right reason, never crashes.
4. Every hour: refresh `SOAK-LOCKSTEP.md` (matches run, desyncs with their frames and fields, loss table, mismatch table) so a morning reader sees progress.
5. Report the frame-time of the lockstep step itself (serialize + apply) from the test's own timing.

Report: the desync table, the loss table, fixes if any, the file list. Interim at 60 minutes with step 1. Time box 150 minutes.
