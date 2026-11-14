# Lane A163, track F2 (carrier skill), evolution 6 (device A, Fable)

You are lane A163 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/VISION.md` (laws 3 and 4), R-0056 to R-0061 at the end of `contracts/RULINGS.md`, the design eye's `docs/design/REVIEW-2.md` (the carry's median is
0.90 s at pro against a bar of 1.5; the human lies on the ice 7.8 % of play at pro; the puck is on nobody's blade 55 % of play), the track's
`docs/experiments/MOVEMENT-5.md`, `MOVEMENT-6.md` and `status/A152.md`.

Your worktree and the commit it stands on are named in the Director's message that starts you: the track's worktree moved to the newest main (your fifth
evolution, the poke, is on it). It was built before: an incremental build is enough. Your status file: `status/A163.md`. Port 4763. Family as the track's
(the list in `briefs/A152-F2-evolution-5.md`), plus the A side's presentation of the track's own states (the HUD line or the figure call by name that shows a
take grace or a stick lock): `docs/experiments/MOVEMENT-7.md`.

## Evolution 6: the player can see why, and a carry lasts
**The ladder gets its last pass tonight (R-0061) and the operator plays at 10:00. Every rule of this evolution that changes the sim ships behind a key at
its neutral value (the default match hash-identical to the main you stand on: 100 hashes at pro and at all_star), measured on 600 paired seeds, with the
value you recommend. Presentation that touches no sim state may land for the 09:00 package.**
1. **The cues** (presentation, first): the player sees **the take grace** (the half second after winning the puck in which a stick cannot take it: on the
   carrier, his and the opponents') and **his own stick lock after a missed poke** (20 frames in which Space and RMB do nothing: today nothing tells him).
   By the calls and HUD elements that exist (name them); headless proof by the log lines the pawn and the HUD already write; a capture off screen if the
   track's driver allows. You do not own the cursor. Write `CUES READY` with the file list when green.
2. **Where the carry ends** (measurement first): 300 of the human's carries at pro, by how each ends: a poke, a check, his own shot or pass, a lost
   handle, the whistle: one table, with the median length of each class and what share a person could have kept by a brake, a spin or a pass a second
   earlier. Then the one rule that buys the most carry time without making the puck untakeable (law 4 holds: a sprinting carrier's puck is easier to
   take), behind a key, neutral, measured: the bar is a median carry of 1.5 s at pro for the human with the cells inside 2 points.
3. **Time on the ice lying down** (7.8 % of play at pro): by cause after the poke's stumble left (your own table from `MOVEMENT-6.md`, re-read on this
   main): which knockdowns a person could have avoided and how he would know.
4. For the operator's sheet: two sentences on Space as it is on this main, and what the cue shows.

State fields: hashed or listed. Neutral tests through `JamBaseline.h`; no pinned hash of a shipped cut. The gate: the full `Jam` filter green.
`-Multiprocess`; three measurement processes at most; check memory before each batch. Time box 100 minutes.
