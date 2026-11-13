# Lane A113, track F1 (contact feel), evolution 2 (device A, Fable)

You are lane A113 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, `docs/VISION.md`, R-0048 and R-0049 at the end of `contracts/RULINGS.md` (from the main checkout), and the track's first evolution in your
worktree: `status/A101.md` and `docs/experiments/CONTACT-2.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a101` (the track's worktree: evolution 1 is committed there as the baseline and is landing on main; its
editor is built; you continue on it). Your status file: `status/A113.md`. Port 4713. Family as the track's (`briefs/A101-*.md`), plus one named line in the brain.

## Evolution 2: a tackle is an event, not a rhythm
Device B played the game for 267 s (`C:\Users\kazda\AppData\Local\Temp\tj\B-to-A-0038.md`): the human was knocked down 15 times in 104 s at pro, 7 of them within a
second of his release, 10 by the same bot from the same side. Ruling R-0049 point 1:
1. **Release grace.** `game.contact.release_grace_frames` (default 90, neutral 0; schema, limits, defaults, README): for that many frames after a skater is released from
   a knockdown, a hit on it is a bump at most (no stumble, no knockdown; the attacker's own price as for any bump), and the lockstep hash covers the new counter (track
   Q1 added `LockstepStateHash`: put the field in it, or its coverage test goes red when that lands). **The brain does not start a check on a skater inside its
   grace** (one condition where the brain picks a check target, in `JamAICore.*`: name the line in your report; track Q2 owns the rest of that file).
2. **The soft hit explains itself** (your own next-2): "TOO SOFT" for the human's own bump, "MISSED" for a whiff, in the HUD line you built; the two edges you left
   (a stumble's `strength=0.00` in the log line; a frame with a bot-on-bot knockdown and a stumble of the human holds nothing).
3. **Measure**: in 100 s at pro, five a side, the scripted human: no figure down again within 2 s of its release, the human down fewer than five times (before and
   after, ten seeds, one process). Knockdowns per match and the three target cells before and after (600 a cell, few processes, paired seeds as you did). More than 4
   points off in a cell: say so; the curve track re-sets (do not tune the levels).
The gate: the full `Jam` filter green; neutral 0 gives back the baseline's hashes (the three pro seeds you used). Time box 90 minutes; interim in the status file.
