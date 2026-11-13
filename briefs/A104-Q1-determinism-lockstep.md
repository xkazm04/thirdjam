# Lane A104, track Q1: Q1-determinism-lockstep (device A, evolution 1, Opus)

You are lane A104 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full**
(how a track works, the machine's band and the cursor, the walls, the status file, the report with the next three evolutions). Then `docs/EVOLUTION.md` and `docs/VISION.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a104` (the Director created it at the commit that carries the whole vision: contact by inertia, the
movement laws, the hockey brain, the ladder of R-0046). Your status file: `status/A104.md`. Your remote-console port if you run a game: 4704.

## Track Q1: determinism and lockstep
Family: the state hash and the serialised state in `JamSim.cpp/.h` (which fields are hashed; nothing else there), `Game/Source/JamNet/**`, the `Bar.Net.*` and `Jam.Net.*` tests,
`tools/soak/lockstep/**`, `docs/experiments/SOAK-LOCKSTEP-2.md`. Read `docs/experiments/SOAK-LOCKSTEP.md` (lane A56's method and its fix) and `MULTIPLAYER.md`.
First evolution: **everything added since protocol 5 is in the hash and on the wire.** (a) list every skater, puck and sim field added by lanes A77 to A95 (the saucer hold,
abilities, shield frames, the contact model's stumble and recoil, the burst window, the sprint and stamina with the puck, the reverse brake, the lift cooldown, the sweep's
two-sided shield, the tap window) and say for each: hashed or not, and whether leaving it out can let two peers differ unseen; hash what must be hashed (the Determinism
baseline moves: write before and after); (b) every input the human can press reaches the wire (`Jam.Net.InputCoverage`): check the ability, the pass hold, the spin and the
sprint in `LobbyGameMode::ReadLocalInput`; (c) A56's soak again on this tree: `Bar.Net.SoakClean` over 1,000 seeds and the lossy profiles, with two playtesters that now brake,
cut, sprint and use abilities (a mode with the shotgun): desyncs and lost inputs; fix what it finds; (d) the REFUSE message A56 proposed for a protocol mismatch (about 20
lines), only if (a) to (c) are green inside the box.

Time box 90 minutes for this evolution. Interim in the status file at 40 minutes.
