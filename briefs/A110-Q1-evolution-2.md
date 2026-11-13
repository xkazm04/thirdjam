# Lane A110, track Q1 (determinism and lockstep), evolution 2 (device A, Opus)

Read `briefs/EVOLUTION-COMMON.md` first (its rules are yours in full), then `docs/EVOLUTION.md`, `docs/VISION.md`, and the track's first evolution: `status/A104.md` and
`docs/experiments/SOAK-LOCKSTEP-2.md` in your worktree.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a104` (the track's worktree: main ee14438 plus evolution 1 uncommitted and already exported for landing; its
editor is built: you continue on it). Your status file: `status/A110.md`. Family as the track's: the lockstep hash and serialised state in `JamSim.cpp/.h`,
`Game/Source/JamNet/**`, the `Bar.Net.*` and `Jam.Net.*` tests, `tools/soak/lockstep/**`, plus the two lines in `HokejGameMode.h/.cpp` named below.

## Evolution 2: the online guest plays the whole game
Lane A104 found three things a joined match lacks:
1. **Abilities for the guest.** The guest's seat is a bot slot in the sim, so a mode's ability goes only to the host; in the Shotgun mode the guest's F does nothing. Grant
   the mode's ability (and a pickup's) to the guest seat. Proof: guest abilities used above 0 in the Shotgun soak, 0 desyncs over 600 matches clean and at 30 % loss.
2. **The guest's stick follows the cursor on the ice.** `AHokejGameMode::CursorAimRink` is private, so online the stick follows mouse movement, not the cursor's point on the
   ice (A49's fix for single player never reached the lobby). Make it reachable (protected, or a shared helper) and use it in `LobbyGameMode::ReadLocalInput`; the quantised
   aim on the wire stays as it is. Proof: a test that the lobby's local input for a cursor at a known screen point equals the single-player read.
3. **A hash-coverage lint.** A reflection test (`Jam.Net.HashCoverage`) that fails when a field of the skater, puck or sim state is neither in `LockstepStateHash` nor tagged
   as a read-out (a metadata tag or a listed name): 0 untagged fields today, and the test goes red when a field is added without a tag (prove by adding one in a scratch
   build and removing it).
Then, if the box allows: soak all twelve modes on protocol 8, 200 seeds each, clean and lossy: 0 desyncs, with the table in `SOAK-LOCKSTEP-2.md`.

The gate: the full `Jam` filter green; the protocol number bumps only if the wire changes (say so). Time box 90 minutes; interim in the status file at 40.
