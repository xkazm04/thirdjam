# Lane A128, track F2 (carrier skill), evolution 3 (device A, Fable)

You are lane A128 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, `docs/VISION.md` (laws 3 and 4), R-0048 to R-0051 at the end of `contracts/RULINGS.md` (main checkout), and the track's two evolutions in
your worktree: `status/A102.md`, `status/A121.md`, `docs/experiments/MOVEMENT-2.md`, `MOVEMENT-3.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a102` (the track's: evolutions 1 and 2 are committed there as the baseline; 1 is on main, 2 is
landing; its editor is built). Your status file: `status/A128.md`. Port 4728. Family as the track's: the movement and stick functions in `JamSim.cpp/.h`, the
`game.skater` and `game.stick` keys, the carry drill (`JamCarryDrill.h`, drill 21 in `HokejGameMode.cpp`), `Jam.Core.Movement.*` tests,
`docs/experiments/MOVEMENT-4.md`.

## Evolution 3: every answer has a price, and the first second feels quick
Lane A121 left three things, each measured on the carry scene (200 seeds an arm, the real brain, pro and all_star) and in the real game mode's drill 21:
1. **The standing start** (the Director's ruling on your open question): a skater that starts from rest has earned its first step. Today 0 to 8 m/s takes 41 frames
   for everyone (23 with the burst), so every faceoff and every get-up feels slow, against the vision's "high acceleration and speed change". The burst arms
   after a stop or a cut of the skater's own (as you built it) **and from rest** (below about 1 m/s for a few frames: after a faceoff, a get-up, a full stop),
   never on a straight charge already under way. Bars: 0 to 8 m/s from rest in 26 frames or fewer; the 8 m stop still passes a pro 95 % or more; the straight
   charge still under 20 %; the pro cell within 2 points of the lane's 79.0 (the defender starts from rest too: measure, do not assume).
2. **The spin's price**: the spin with the cut passes a pro 100 % from 6 m and 82.5 % from 5 m: too sure. Give it a cost a defender can punish (the spin's
   recovery frames, the puck's exposure at its end, or its stamina), so that it passes 50 to 70 % from 6 m and under 50 % from 5 m, the deke tap stays near its
   56 %, and the 8 m stop stays at 95 % or more. One key, with a neutral value.
3. **The stop taught where it works**: the drill's default stop at 8 m; `UJamSim::Restart` resets `BurstLeft` and `MoveHist` (your finding at `JamSim.cpp:173`);
   drill 21 script 1 passes 20 of 20 in the real game mode (14 today).
4. If time remains: the 9.5 m edge (95.0 % with no margin: 8 of the 10 failures are time-outs with the puck kept): a carrier that has stopped and kept the puck
   should be able to go again; say what holds it.

The lockstep hash: every counter you add goes into `StateHash()`; name the fields in your report (main's coverage test fails on an unlisted field).
Measure the three target cells before and after this evolution alone (600 a cell, paired seeds, `-Multiprocess`, never more than three measurement processes at
once: your last run fell under 12 GB free): report the move, do not tune the levels (the curve track is re-setting them tonight).
The gate: the full `Jam` filter green; the neutral block gives back your baseline's hashes. Time box 100 minutes; interim in the status file.
