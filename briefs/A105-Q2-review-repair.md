# Lane A105, track Q2: Q2-review-repair (device A, evolution 1, Opus)

You are lane A105 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full**
(how a track works, the machine's band and the cursor, the walls, the status file, the report with the next three evolutions). Then `docs/EVOLUTION.md` and `docs/VISION.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a105` (the Director created it at the commit that carries the whole vision: contact by inertia, the
movement laws, the hockey brain, the ladder of R-0046). Your status file: `status/A105.md`. Your remote-console port if you run a game: 4705.

## Track Q2: review and repair
Family: the ability rule in `JamSim.cpp` (the functions lane A84 added, nothing else in the sim), `JamAICore.cpp/.h` and `JamSimAdapter.cpp` (the brain), `CommentarySystem.*`,
`MenuFlow.*` / `MenuWidget.*`, `JamCartridge.cpp` (the loader), their tests, `docs/experiments/REVIEW-CODE.md`. Tracks F1 (contact) and F2 (movement, stick) hold the rest of
the sim: a fault there is a finding.
First evolution: **read today's fast landings as an adversary and fix what is real.** For each real fault, a test that fails before the fix: (a) abilities (A84): an ability
used while stumbling, down, in the box, during a faceoff or a goal pause; the shotgun's knockdowns under the new contact model and the hit-stop; a pickup ability at 5 v 5; the
slow puck with the saucer and the lifted shot; the hip check against a braced skater; (b) the brain (A94, A95): the space read with skaters down or boxed, a team short-handed
by two, the holder read while the puck is in flight, a division by zero or a stale index after a rebuild (`jam.livecut`), the lift cooldown across a faceoff; (c) the commentary
picker (A90): the knockdown ring across periods and rebuilds, a line cut during a pause; (d) the menu and the loader (A81, A84): a mode file with `game.meta` over its limits, a
file named in the order table that is missing, twelve cards with one unreadable. List every finding by severity; fix severities 1 and 2; the rest stay listed for the next
evolution.

Time box 90 minutes for this evolution. Interim in the status file at 40 minutes.
