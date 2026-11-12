# Lane A9: multiplayer by invite (lobby, lockstep on the deterministic sim), feasibility first (device A, from H5)

You are lane A9 of the Third Jam (team Hokej). Repo `C:/Users/kazda/kiro/thirdjam`. The operator wants "possible multiplayer and lobby on invite". This is a stretch goal for M3+
(`docs/PLAN.md` 9 cut lines; it is cut before anything else if it endangers M1/M2), so your job is: **prove the cheapest viable path in two hours, then build it only if the proof holds.**
Read first: `GAME.md`, `docs/HOCKEY.md` 2 (the sim is a fixed-step 60 Hz deterministic integrator with `state_hash`; `FJamInput` per skater per frame), `Game/Source/JamCore/Public/JamSim.h`,
`JamPawns.h`, `Game/Source/JamGame/Public/HokejGameMode.h` (lane A1, finished), `briefs/A8-main-menu.md` (lane A8 builds the menu in parallel; your lobby screens plug into its menu through a
single entry "Play with a friend"; agree names in your report), `tools/mcp/README.md`, `tools/run/*.ps1`.

You own: `Game/Source/JamNet/**` (a new Runtime module you create; add it to the `.uproject` and `ThirdJam.Target.cs`; nothing else in the project depends on it), `Game/Source/JamGame/Lobby*`,
`docs/experiments/MULTIPLAYER.md`. Run no git command; editor on MCP port **8007**; a second editor instance for the two-client test on **8008** is allowed (count it as your second heavy job: run
no package while both are up); never kill a process you did not start; free-space floor 40 GB. The shared `Game/Source` tree must compile at all times.

## Design to prove (deterministic lockstep, not state replication)
- The match runs the same `UJamSim` on both machines; only inputs travel. Each frame each peer sends its `FJamInput` for frame N; the sim advances frame N only when both inputs for N are present
  (lockstep with a small input delay of 2-3 frames to hide latency); `state_hash` is exchanged every 60 frames and a mismatch ends the match with a "desync" card (never silently diverge).
  Bots run identically on both sides from the same seed. This needs no replication of positions at all and keeps the game identical to single player.
- Transport: Unreal's own networking (listen server, `UNetDriver`) carrying a tiny replicated input struct, OR a plain UDP socket pair via `FSockets` (simpler to reason about for lockstep;
  harder through NAT). Measure both in the prototype if time allows; otherwise pick the one you can prove in two hours and say why.
- **Invite**: a 6-8 character code the host shows and the guest types. First version: the code encodes the host's reachable address and port (LAN or a port-forwarded WAN address);
  the lobby screen shows "LAN only unless your router forwards port X" in plain words. No account service, no EOS, no Steam (none is set up and the rules forbid extra software for judges).
  Document in `MULTIPLAYER.md` what a relay would take if the operator wants internet play later.
- Lobby: host creates (choose cut, show code), guest joins (enter code), both see each other's name and team colour, host presses Start, both load `Rink.umap` with the same cartridge and seed.

## Deliverables
1. `docs/experiments/MULTIPLAYER.md` (first 30 minutes): the design above with the numbers it needs (input delay frames, hash interval, bandwidth per frame), the risks (NAT, desync from
   float differences between machines: both are x64 with the same compiler flags; device B is ARM64 running x64 under emulation, so a cross-device test is the real desync test), the cut line.
2. **Prototype** (two hours): two PIE/standalone clients on this machine (ports 8007/8008 editors or `-game` windows), host + guest through the lobby, a full match in lockstep with hashes agreeing
   every 60 frames; record the frame count, hash agreement, input delay used and the bandwidth. If it does not hold by the two-hour mark, stop, write what broke and what it would take, and report.
3. If it holds: `JamNet` module (`UJamLobbySubsystem`, `UJamLockstepDriver`, the input packet, the hash check, the desync card), `Lobby*` screens for A8's menu entry, a `LockstepTwoPeers`
   automation test that runs two sims in-process with simulated delay and asserts hash equality, and the "Play with a friend" flow proven end to end with screenshots.

Report: the verdict (holds / does not hold), numbers, screenshots, every error, the seam names for A8. Time box 3 hours; report at 60 minutes with the state either way.
