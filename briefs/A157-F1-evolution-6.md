# Lane A157, track F1 (contact), evolution 6 (device A, Fable)

You are lane A157 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/VISION.md` (law 1), R-0056 to R-0061 at the end of `contracts/RULINGS.md` (your worktree's copy is the newest), the track's `docs/experiments/CONTACT-5.md`,
`CONTACT-6.md` and `status/A151.md`, the design eye's `docs/design/REVIEW-2.md`, and device B's handoff `B-to-A-0041` point 2 (read it with
`git show origin/dev-b:handoff/B-to-A-0041.md`: the one read-only git command you may run).

Your worktree and the commit it stands on are named in the Director's message that starts you: the track's worktree moved to the newest main (your fifth
evolution is on it). It was built before: an incremental build is enough. Your status file: `status/A157.md`. Port 4757. Family as the track's (the list in
`briefs/A151-F1-evolution-5.md`): the contact functions in `JamSim.cpp/.h`, the `game.contact` keys, the contact tests, drill 22, `docs/experiments/CONTACT-7.md`.

## Evolution 6: nobody stands in a man who is down, and a check is a commitment
**The ladder gets its last pass tonight (R-0061) and the operator plays at 10:00. So every rule of this evolution ships behind a key at its neutral value
(the default match hash-identical to the main you stand on: prove it with 100 hashes at pro and at all_star), measured on 600 paired seeds, with the value
you recommend. The Director turns a rule on with the curve; the operator's priorities at 10:00 decide the rest.**
1. **A skater does not stand in a downed body** (B's critic: an opponent skated into a downed skater and stood in him for 24 frames; seed 1, sim frame 5242,
   clay, pro). A push-out of about 0.6 m around a downed skater (`contact.downed_radius_m`, neutral 0), deterministic, no impulse to the downed man, no
   puck effect. Bar (B's acceptance): no skater's root within 0.6 m of a downed skater's for more than 6 frames in 300 s, over 200 matches; the three
   cells before and after. Write `PUSHOUT READY` with the file list and the cells as soon as this item is green: it may land on at once if the cells
   move under 1 point.
2. **A check press has a cooldown** (R-0060 point 4: RMB every 12 frames keeps the window open): `contact.check_cooldown_frames`, neutral 0; the value at
   which a mashed button lands no more checks than a timed one; the scripted human and the bots under it.
3. **The lunge as a motion**: a pressed check commits the body for its window (a short forward impulse, steering reduced), so a miss is seen and costs
   position: `contact.lunge_mps` and `contact.lunge_frames`, neutral 0. The figure call is by name in `contracts/skater-visual.md` if one fits: name it in
   your report, call nothing new.
4. **The board slam as a play** (the board bonus is 1.25 today): what a player must do to earn it (closing speed along the normal, the victim inside 1 m of
   the boards), and its read in a match: board hits a match by level, frames down against open-ice hits.
5. For the operator's sheet: two sentences on what RMB does now and what it would do with items 2 and 3 on.

State fields: hashed or listed. Neutral tests through `JamBaseline.h`; no pinned hash of a shipped cut. The gate: the full `Jam` filter green.
`-Multiprocess`; three measurement processes at most; check memory before each batch. Time box 100 minutes.
