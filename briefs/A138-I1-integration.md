# Lane A138, track I1: the sim-and-brain seam, integrated (device A, Fable)

You are lane A138 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/VISION.md` (the four laws: they decide every call below), R-0048 to R-0053 at the end of `contracts/RULINGS.md` **in the main checkout** (R-0053 is
your charter and is newer than your worktree's copy), `docs/EVOLUTION.md`, and the four lane documents whose work meets in your tree:
`docs/experiments/MOVEMENT-2.md` and `MOVEMENT-3.md` (track F2: the kept puck, the body shield, the take grace, the stop's grip, the carry drill),
`docs/experiments/CONTACT-3.md` and `CONTACT-4.md` (track F1: the release grace, the shove under the shields, the figure calls),
`docs/experiments/BRAIN-2.md` (track Q2: the brain's second evolution and its two hooks), `docs/experiments/CURVE-6.md` (the curve lane's first profile,
**not** in your tree: read it from `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\c0\docs\experiments\CURVE-6.md` for what it found about the pro's poke).

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\i1`, at the parked commit `wip-batch6`: **main plus lane A115's brain (switched on, its two
hooks wired) plus lane A125's shove rule and figure calls**. It builds; its gate is red (below). It has no binaries: build the editor first (builds no longer
queue across worktrees). Your status file: `status/A138.md`. Port 4738. **Family: for this lane, the whole seam**: `JamSim.cpp/.h` (contact, stick, movement),
`JamAICore.*`, `JamBrain.cpp`, `JamSimAdapter.cpp`, `JamPlaytester.cpp`, the tests under `Game/Source/JamCore/**`, `docs/experiments/INTEGRATION.md`. Not yours:
the difficulty levels' numbers (the curve lane takes your tree next), the HUD, the figure.

## The red gate you start from (194 passed, 7 failed)
- `Jam.Core.Movement.StopAndGo`: "a clean stop-and-go beats a pro defender at least 8 times in 10 (50.0 %)"; "the stop begun at 9.5 m stays at 95 % or more
  (50.0 %)"; "with the blade left ahead the stop-and-go is at least 20 points worse (53.5 %)".
- `Jam.Core.Movement.TakeGrace`: "with the grace no carrier loses the puck to a stick inside half a second of winning it: 1"; "the shortest hold before a stick
  takeaway is more than 30 frames (8)".
- Five pinned-hash neutral tests: `Jam.Bot.Hockey2.Neutral`, `Jam.Core.ContactBoardMatch`, `Jam.Core.ContactGraceMatch`, `Jam.Core.ContactReachMatch`,
  `Jam.Core.MovementNeutral`: each lane pinned "my neutral equals my baseline" on its own tree, and the other lanes' rules and the new brain moved the matches.

## What to do, in this order
1. **One shared baseline for the neutral tests** (R-0053 point 5): a single helper (config and brain profile) that puts every rule and brain evolution added
   since the vision's commit at its neutral: `contact.board_bonus` 1, `contact.release_grace_frames` 0, `contact.check_reach_m` 0, `stick.passive_take` true with
   the teammate-touch switch, `stick.body_shield` false, `stick.take_grace_frames` 0, `skater.reverse_brake_decel` 20, `skater.burst.own_stop` false, the brain
   tune `brain2=0`, the shove rule's own neutral (give it one: a switch that restores the refusal), and a comment telling the next lane to add its neutral in
   one line. The five tests call it and un-neutralise only what they test. Their pinned hashes hold again, or you say exactly why one cannot and re-pin it
   from a measured run of the commit it names.
2. **Find what broke law 3**: the carry scene (200 seeds an arm, the real brain at pro) with the brain's second evolution on and off, its shield and grace
   reads on and off, and the shove rule on and off: the table of the stop begun at 8 m, at 9.5 m, the straight charge, and how each failed carry ended (lifted,
   poked, shoved from behind, checked, timed out with the puck kept).
3. **Make law 3 hold with everything on** (R-0053 points 1, 3, 4): at pro a stop begun at 8 m passes at least 80 %, a straight charge under 20 %; at all_star
   the stop passes at least 50 %. The rulings give you the direction: a shove from behind at matched speed is a bump by the contact law (the cross-check's own
   thrust must not turn a push in the back into a stumble); a shove from behind that fells a carrier is not a play the brain chooses where the whistle is off;
   a defender beaten by a stop must lose a step (it overran: that is the skill); a defender that contains goal-side wins when the carrier dawdles, not when
   he stops and goes. The take grace refuses the stick's take only; the test counts stick takeaways only.
4. **The gate green with everything on**, and the numbers the curve lane needs: the three cells on both rinks (600 a cell) as your tree stands, knockdowns
   of the human a match, the share of big-ice matches with a 30 s stretch without the puck (lane A115's 25 % must not be lost), reverse brakes a match.
5. Write `INTEGRATION READY` in your status file with the file list the moment items 1 to 4 are green: the Director lands it at once and starts the curve
   lane on your tree.

A parallel lane (track F2, lane A128) is changing the standing start and the spin's price tonight on an older tree; its rules arrive with their own neutral and
are added to your baseline at landing. State fields: hashed or listed as the common brief says. `-Multiprocess` on every headless start; few processes, many
seeds; check memory before each batch. Time box 120 minutes; write each table to the status file as it completes.
