# Lane A101, track F1: F1-contact-feel (device A, evolution 1, Fable)

You are lane A101 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full**
(how a track works, the machine's band and the cursor, the walls, the status file, the report with the next three evolutions). Then `docs/EVOLUTION.md` and `docs/VISION.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a101` (the Director created it at the commit that carries the whole vision: contact by inertia, the
movement laws, the hockey brain, the ladder of R-0046). Your status file: `status/A101.md`. Your remote-console port if you run a game: 4701.

## Track F1: contact feel
Family: the contact functions in `JamSim.cpp/.h` (`StabilityOf`, `ContactImpulse`, `ResolveContact`, the collision resolve), `JamPawns.cpp/.h` (the hit-stop, the read-outs to
the figure), `CameraFollow.*` (the kick), `HUDWidget.*` only for contact texts, the `game.contact` keys, `Jam.Core.Contact.*` tests, `docs/experiments/CONTACT-2.md`.
First evolution: **every outcome reads.** (a) the refused press (`bCheckRefused`) shows a short HUD line in the voice ("NOT WITH THE PUCK") and the attacker's bounce shows one
("BOUNCED OFF"); (b) the stumble gets a hit-stop of 1 to 2 frames and a camera kick scaled by the ratio (today both fire only on a knockdown that involves the human); a
knockdown's hit-stop scales 2 to 5 frames by the ratio; (c) **the boards are a body**: a skater hit within 1.5 m of the boards takes the boards' reaction: the impulse
counts higher by `game.contact.board_bonus` (default 1.25, neutral 1.0) and the victim's `board` read-out fires with the hit's strength so device B's board-hit take plays;
(d) a headless measurement over three pro matches: outcomes by kind, the share of hits near the boards, and the three target cells (the board bonus can move a win rate).
Proof: the tests, the neutral proof for the bonus, the acceptance rows check / crosscheck / checked on a package only if you hold the cursor (else say queued).

Time box 90 minutes for this evolution. Interim in the status file at 40 minutes.
