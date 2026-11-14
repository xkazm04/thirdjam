# Lane A201, track F2 (movement), evolution 11 (device A, Opus)

You are lane A201 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/VISION.md` (laws 3 and 4), **R-0064** at the end of `contracts/RULINGS.md`, the track's `docs/experiments/MOVEMENT-10.md` and `MOVEMENT-11.md`
with `status/A194.md` (the speed rule, the breakaway's bonus 0.3, drill 21 and the stop line at 8.5 m, the far edge 9.0 m; the finding that a player
who stops at every lit line wins 29 of 100 at pro against 58 for one who ignores it), `docs/experiments/BRAIN-7.md` (the bots plan with true speeds:
law 3's rows as shipped), and `JamCarrierMarks.h` with the marks' drawing code and `CarrierMarkTests.cpp`.

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A201.md`. Port 4801. Family: the
carrier's marks (`JamCarrierMarks.h`, their drawing, their tests), the carry drill and drill 21, the movement bar tests and traces,
`docs/experiments/MOVEMENT-12.md`. **No sim state and no cartridge number changes today**: the curve lane sets the ladder on main as it stands. The
marks are presentation: the default match's hash is untouched (prove it: 100 seeds identical).

## Evolution 11: the stop line only lights when stopping is right
The line on the ice is the game's only teacher of its central skill, and today it teaches wrongly: obeying it halves a pro player's wins, because it
lights for one defender's distance and the match has a second man. The sheet had to tell the operator "an offer, not an order". Make it an order.
1. **Why obeying loses** (measure, 30 minutes; the scripted player that obeys the line, 200 matches at pro and rookie): for every lit line he obeyed:
   what happened in the next 90 frames (kept the puck and went on / lost it to the man he stopped for / lost it to ANOTHER man / shot / passed), with
   the scene at the light: how many opponents within 6, 10 and 14 m and where (ahead, beside, behind), his own speed, the distance to the boards and to
   the net. One table: the scenes in which the stop pays and those in which it loses.
2. **The rule**: the line lights only in scenes where the stop pays: propose it from the table (for example: exactly one opponent closing inside the
   window, no second opponent able to reach the stopping point within N frames at his true speed, room behind the defender, not inside the attacking
   slot where shooting is better). Everything read from positions and velocities the mark's code already has. The far edge (9.0 m) and the line's place
   (8.5 m) stay unless the table says otherwise.
3. **Proof**: the scripted player who obeys every lit line wins within 5 points of the one who ignores it at pro (today 29 against 58), and better
   than him at rookie if that is reachable; a stop begun on the light passes law 3's drill at 90 % or more at pro; how often the line lights a match
   before and after (it must still light: at least twice a match at pro, or it teaches nothing: if the honest rule lights less, say so and give the
   Director the two arms); `Jam.Core.Movement.Marks` and `CarrierMarkTests` updated; the hash proof.
4. **What it looks like**: off-screen stills of the line unlit, lit, and at its last moment from the game camera on clay and on the night look: can
   an eye find it in a third of a second? Thickness and colour by the cvars that exist; no new look.
5. If time remains: the loose-puck ring (the other mark): the same question: when it shows, is going for the puck right? One table.

Write `MARKS READY` with the file list when items 1 to 3 are closed, built, the full `Jam` filter green and Shipping compiles: **wanted by 17:00 for
the 18:30 package**; `SOURCE OPEN` before any later source change. `-Multiprocess`; three measurement processes at most; stop only processes whose
ids you recorded. Time box 170 minutes.
