# Lane A147, track F2 (carrier skill), evolution 4 (device A, Fable)

You are lane A147 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, `docs/VISION.md` (laws 3 and 4), R-0048 to R-0055 at the end of `contracts/RULINGS.md`, the track's docs `docs/experiments/MOVEMENT-2.md`,
`MOVEMENT-3.md`, `MOVEMENT-4.md`, `status/A128.md`, and the integration lane's account of the tree you start on: `docs/experiments/INTEGRATION.md`.

Your worktree and the commit it stands on are named in the Director's message that starts you: a **fresh worktree at the newest main**: the integrated game
(your three evolutions, the brain's second evolution reading the shield and the grace, the shove rule). It has no binaries: build the editor first. Your
status file: `status/A147.md`. Port 4747. Family as the track's: the movement and stick functions in `JamSim.cpp/.h`, the `game.skater` and `game.stick` keys,
the carry drill (`JamCarryDrill.h`, drill 21 in `HokejGameMode.cpp`), `Jam.Core.Movement.*` tests, `docs/experiments/MOVEMENT-5.md`.

## Evolution 4: the carrier's second move, and a drill that teaches each level's window
On the integrated tree the stop begun at 8 m beats a pro 200 times in 200 and a straight charge never does. What is left is in your own reports:
1. **A second move** (your next-3): 8 of the 10 failed stops at 9.5 m were time-outs with the puck kept: a footrace with the defender 1.5 m behind the
   carrier's shoulder for 250 frames. A carrier that has stopped and gone should be able to shake the chaser with a second cut about a second later. Bar:
   the stop at 9.5 m from 97 % to 99 % or more with no time-out; the straight charge still under 20 %; the stop at 8 m still 95 % or more.
2. **The window by level, taught**: all_star does not creep, so its window is 10 to 12 m (0 % at 8 m, 56.5 % at 10 m, 100 % at 12 m): drill 21 takes its
   stop distance from the level (pro 8 m, all_star 11 m, rookie whatever its defender allows: measure) and the drill's own on-screen line says the distance
   in words a player can use ("stop when he is two stick lengths away"). Bar: drill 21 script 1 passes 18 of 20 or better at each level in the real game
   mode, headless.
3. **The spin from 5 m** (the bar your last evolution missed by 3.5 points: 53.5 % against under 50 %): the losses from 5 m are body checks, so the answer is
   in contact, not the stick: propose it with numbers (a spinning skater's stability, the check's window against a spin), build it only if it is inside your
   family, otherwise it is a finding for track F1 with the measurement.
4. **The standing start by hand**: 0 to 8 m/s in 24 frames is from rest; what about a skater that was bumped to 2 m/s, or released from a knockdown, or the
   faceoff's loser? List the cases where a player would expect the quick first step and does not get it (measure each), and fix the ones the rule's intent
   covers.

Use `Game/Source/JamCore/Public/JamBaseline.h` for neutral tests (a key you add gets its neutral there in one line). The curve lane is re-setting the levels
on this commit now: once it lands the ladder is set, and a rule of yours that moves a cell more than 4 points ships at its neutral value: measure the three
cells for each rule alone (600 a cell, paired seeds) and say which ships on. Law 3 is a gate. `-Multiprocess` on every headless start; never more than three
measurement processes at once. The gate: the full `Jam` filter green. Time box 100 minutes; interim in the status file.
