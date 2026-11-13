# Lane A120, track Q1 (determinism and lockstep), evolution 3 (device A, Opus)

You are lane A120 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, and the track's two evolutions in your worktree: `status/A104.md`, `status/A110.md`, `docs/experiments/SOAK-LOCKSTEP-2.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a104` (the track's: evolutions 1 and 2 are committed there as the baseline; 1 is on main, 2 is landing;
its editor is built). Your status file: `status/A120.md`. Port 4720 (and 4721 for a second process). Family as the track's: the lockstep hash and serialised state in
`JamSim.cpp/.h`, `Game/Source/JamNet/**`, the `Jam.Net.*` tests, `tools/soak/lockstep/**`, `tools/net/**`, plus the three lines named in item 3.

## Evolution 3: two real processes, and the guest is spoken to
The operator will play a joined match between two machines this weekend (device B hosts or joins): nobody has run that. Everything so far is in-process.
1. **Two processes on this machine, headless** (`tools/net/twoproc.ps1`, `-game -nullrhi`, loopback, both through the real lobby flow): a scripted host and a
   scripted guest play a Shotgun match and a regular match to the end with sprint, saucer, brake, spin, check and F each. Proof: equal final hashes in both
   processes' logs, guest abilities above 0 in the guest's log, no desync card, three matches each.
2. **The refusal is quick and says why**: a guest on another build (a patched version byte) is refused in under 1 second with the message on its screen, instead
   of a 60-second wait; a host that goes away ends the guest's match with a card, not a freeze (measure the seconds).
3. **Online barks, commentary and the Director's trigger speak to the local seat**: `JamEffectSystem.cpp:68`, `BarkSystem.cpp:398` and `CommentarySystem.cpp:644` key
   on `GetHumanIndex()`, which is the host; on the guest's machine they must key on the guest's seat (presentation only: nothing enters the sim or the hash). Proof: a
   test that a guest's goal, hit and ability fire the human's bark on the guest's machine, the lockstep hash unaffected.
4. **The soak's field compare covers whole arrays** (`JamNetSoakTests.cpp:419` compares only element 0 of `MoveHist`): a seeded one-field divergence in
   `MoveHist[7]` is found at its exact frame; 0 desyncs over 1,000 clean matches.
5. **A one-page guide for the operator** (`docs/experiments/JOIN-GUIDE.md`): how to host and join between two machines on one network (the port, the firewall
   prompt, what each screen shows, what the desync and refusal cards mean), tested by your two-process run.

Other tracks are adding skater state tonight (a release grace counter, a passive-take rule, a board read-out): your coverage test `Jam.Net.HashCoverage` will go red
when they land without a tag; the Director tags them at landing. Do not chase them here.

The gate: the full `Jam` filter green; the protocol number bumps only if the wire changes (say so). Time box 100 minutes; interim in the status file.
