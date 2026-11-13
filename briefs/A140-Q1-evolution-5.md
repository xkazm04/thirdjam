# Lane A140, track Q1 (determinism and lockstep), evolution 5 (device A, Opus)

You are lane A140 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, and the track's docs: `docs/experiments/SOAK-LOCKSTEP-2.md`, `docs/experiments/JOIN-GUIDE.md`, `status/A129.md`, `tools/net/twoproc.ps1`.

Your worktree and the commit it stands on are named in the Director's message that starts you: a **fresh worktree at the newest main** (the track's four
evolutions are on it, with every rule landed tonight). It has no binaries: build the editor first. Your status file: `status/A140.md`. Ports 4720 and 4721 (and
4740, 4741 for a second pair). Family: `Game/Source/JamNet/**`, the lockstep hash and serialised state in `JamSim.cpp/.h` (`Restart` for item 2),
`Jam.Net.*` tests, `tools/soak/lockstep/**`, `tools/net/**`, `docs/experiments/JOIN-GUIDE.md`.

## Evolution 5: what the operator will meet when two machines play
The operator and device B will try a joined match between two physical machines this weekend. Everything so far ran on one machine over loopback.
1. **A clean rematch**: `UJamSim::Restart` (`JamSim.cpp:173`) does not reset `bBraking`, `bReverseBraking`, `BladePos`, `LastPassStrength`, `BumpDir`,
   `LastBumpFrame` (lane A129's list; check for more against the state structs): a rematch is not a fresh match. Reset them; a test that a restarted sim equals
   a fresh one for two different histories; the lockstep rematch still agrees.
2. **The packaged game, two processes**: everything so far ran the editor binary as a game. Build one Development package of your tree (or use one the
   Director names) and run the two-process matrix on it headless (`-nullrhi`): three Shotgun and three regular matches to the end with equal hashes, the
   refusal of another build, the host leaving. Name any difference from the editor-binary runs.
3. **Not loopback**: bind the host on the machine's LAN address (not 127.0.0.1; no firewall or system setting changed: if the bind or the connect needs a
   firewall rule, stop and write exactly what prompt the operator will see and what to tick) and join through that address from the second process: does the
   lobby's address entry, the port (UDP 7700) and the status line behave as `JOIN-GUIDE.md` says?
4. **Two-process press accounting**: each machine counts its local F and edge presses and the applied ones; 0 lost across three impaired runs
   (`SimDelayMs=40`, `SimLoss=0.05`), measured in two processes.
5. **Live cuts that travel as lockstep input**, if time remains: the host's Director's Call or Designer change goes to both machines and applies on the same
   frame (today it is refused online). Proof: three two-process runs with a live cut at frame 1200, equal hashes after it; 600 soaked matches, 0 desyncs. A
   wire change bumps the protocol to 9: say so.
6. Update `JOIN-GUIDE.md`: one page, in the order the operator will do it, with what each screen says.

`-Multiprocess` on every headless editor start (not the packaged game). Never change firewall or system settings. Another lane owns the system cursor: headless
only. The gate: the full `Jam` filter green. Time box 110 minutes; interim in the status file.
