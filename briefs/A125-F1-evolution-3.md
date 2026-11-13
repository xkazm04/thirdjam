# Lane A125, track F1 (contact feel), evolution 3 (device A, Fable)

You are lane A125 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, `docs/VISION.md`, R-0048 to R-0051 at the end of `contracts/RULINGS.md` (main checkout), `contracts/skater-visual.md` (the figure's contract:
the calls the pawn makes on device B's figure), device B's answers under `handoff/A-to-B-0021.md` and `handoff/A-to-B-0023.md` (main checkout after the next merge; the
Director's copy of the asks is in this brief), and the track's two evolutions in your worktree: `status/A101.md`, `status/A113.md`, `docs/experiments/CONTACT-2.md`,
`CONTACT-3.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a101` (the track's: evolutions 1 and 2 are committed there as the baseline; 1 is on main, 2 is landing;
its editor is built). Your status file: `status/A125.md`. Port 4725. Family as the track's, plus `TryLift`'s shove branch and the pawn's figure calls named below.

Know: track F2's rule is on main and not in your worktree: `game.stick.body_shield` (default true): a stick's take (lift or poke) through the carrier's body is
refused (`IsLiftShielded` / `IsPokeShielded`). Item 1 below is written against it: build item 1 so that it merges (your edit lives in the cross-check's shove
branch; say exactly which lines), and prove it with a scene that sets the shield's geometry by hand if the key is absent in your tree.

## Evolution 3: the figure is told what happened, and the human's check finds its man
1. **The shield refuses the stick, never the shove** (R-0050 point 2): a cross-check's shove inside its cone is contact and obeys the contact law from any side; only
   the puck's take through the body is refused. Proof: a test where the same MMB press from behind shoves the carrier (ratio by the law) and does not take the puck.
2. **`PlayStumble(float DirX, float DirZ, int32 Frames)`** on the figure (device B asked for it: today a bump and a stumble both arrive as `PlayHitReaction`, and the
   stumble should cost the figure its feet for exactly the frames the player has no control): the pawn calls it on the stumble's first frame with the sim's stumble
   frames; a bump keeps `PlayHitReaction`. A figure that does not implement it keeps compiling and falls back to `PlayHitReaction` (the pattern section 9 of the
   contract describes for v4 calls). Propose the contract row's text in your report.
3. **A braking read-out for the figure** (device B asked: it builds a hard-stop take and a spray, and guessing the brake from the pawn's speed misbehaves): a level,
   `SetBraking(bool bOn, float Decel)`, on the frames the sim applies the reverse brake (the same condition the sim uses, read from sim state, not from the pawn's
   velocity). Same fallback rule. Propose the contract row.
4. **The human's check finds its man** (your own next-1): the playtester whiffs 75 % of its check presses (30 whiffs against 10 landed in three-minute matches). A
   short lunge toward the nearest opponent in a cone ahead (`game.contact.check_reach_m`, neutral 0; it bends the press's direction, it does not teleport and it does
   not add speed beyond the lunge the model already has). Bar: the whiff share under 40 %; knockdowns per match and the three cells before and after (600 a cell,
   paired): more than 4 points off in a cell: report it, do not tune the levels.
5. If time remains: the hit's size on screen (the hold 2 to 6 frames and the kick by frames down). Otherwise it is your first "next".

Any counter you add to skater state goes into `StateHash()` behind its default (main's lockstep hash folds that in and its coverage test fails on an unlisted field).
The gate: the full `Jam` filter green; neutral values give back your baseline's hashes. Time box 100 minutes; interim in the status file.
