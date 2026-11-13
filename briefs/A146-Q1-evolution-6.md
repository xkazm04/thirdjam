# Lane A146, track Q1 (determinism and lockstep), evolution 6 (device A, Opus)

You are lane A146 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, and the track's docs: `docs/experiments/JOIN-GUIDE.md`, `docs/experiments/SOAK-LOCKSTEP-2.md`, `status/A140.md`, `tools/net/twoproc.ps1`.

Your worktree and the commit it stands on are named in the Director's message that starts you: a **fresh worktree at the newest main** (the track's five
evolutions are on it, with the integrated game: the brain's second evolution, the shove rule, the standing start). It has no binaries: build the editor first.
Your status file: `status/A146.md`. Ports 4720/4721 and 4740/4741. Family: `Game/Source/JamNet/**`, the lockstep hash and serialised state in `JamSim.cpp/.h`,
`Jam.Net.*` tests, `tools/soak/lockstep/**`, `tools/net/**`, `docs/experiments/JOIN-GUIDE.md`.

## Evolution 6: the joined match on the game as it is tonight, and the host's list a stranger can use
The lockstep was last soaked before tonight's integration landed (the brain's second evolution with a second human, the shove rule, the rest and spin
counters, the take grace's same-frame cover, a restart that rebuilds every skater). The operator plays a joined match with device B this weekend.
1. **Re-soak on the integrated main**: all twelve modes (three of them re-made tonight: five a side, five on three, two on two), clean and at 30 % loss with
   jitter and duplicates, 200 seeds each: 0 desyncs, 0 presses lost; `Jam.Net.HashCoverage` green (every field added tonight hashed or listed); then the
   two-process matrix (three Shotgun, three regular, to the end, a rematch each) on this tree.
2. **The host's list shows only addresses another machine can reach**: today it lists a code for the WSL virtual adapter (172.25.208.1) and not the
   Tailscale address (`JamLink.cpp`, `LocalAddresses`): filter virtual, loopback-only and link-local adapters, list the LAN address first, and say which is
   which on the host's screen ("same network", "this machine only"). A test (`InviteCodeRoundTrip`) and the host's screen text in `JOIN-GUIDE.md`.
3. **Live cuts that travel as lockstep input** (protocol 9; your design from the last report): the host's Director's Call or Designer change goes to both
   machines and applies on the same frame; the guest stalls at that frame until it holds the cut; a guest on protocol 8 is refused with the build-mismatch
   line. Proof: three two-process runs with a cut at frame 1200, equal hashes after it; 600 soaked matches with a cut in each, 0 desyncs. If it cannot be
   proven green inside the time box, leave it switched off (the refusal stays) and say what is left.
4. **The firewall sentence for the package's README** (the operator's first host or join on a new package folder raises the Windows prompt; you found about
   45 allow rules, one per package folder): one paragraph for `submission/package/README.txt` and `JOIN-GUIDE.md`, proposed in your report (the submission's
   text is another track's).

Never change firewall or system settings or answer a security prompt. `-Multiprocess` on every headless editor start. Another lane owns the system cursor.
The gate: the full `Jam` filter green; say the protocol number you end on. Time box 110 minutes; interim in the status file.
