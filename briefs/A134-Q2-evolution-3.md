# Lane A134, track Q2 (the brain), evolution 3 (device A, Fable)

You are lane A134 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, `docs/VISION.md` (law 2), R-0048 to R-0053 at the end of `contracts/RULINGS.md`, and the track's docs: `docs/experiments/BRAIN-2.md`,
`status/A115.md`, `docs/experiments/REVIEW-CODE.md`, plus what track F2 measured about the brain: `docs/experiments/MOVEMENT-2.md` and `MOVEMENT-3.md`.

Your worktree and the commit it stands on are named in the Director's message that starts you: a **fresh worktree at the newest main**, which carries your
second evolution switched on, the two hooks wired (the brain reads the body shield and the take grace), and track F2's rules themselves. It has no binaries:
build the editor first (builds no longer queue across worktrees). Your status file: `status/A134.md`. Port 4734. Family: `JamAICore.cpp/.h`, `JamBrain.cpp`,
`JamSimAdapter.cpp`, `JamPlaytester.cpp` (the scripted human's stick reads only: item 1), the bot tests, `docs/experiments/BRAIN-3.md`. The difficulty profiles'
numbers are the curve lane's tonight (lane A133 is re-setting them on the same commit): add a profile field if you must, never retune a level.

## Evolution 3: the brain defends the kept puck, and the puck stays with the human's side
1. **Read F2 for real**: on track F2's carry drill (`FJamCarryDrill`, drill 21) and in matches: the pro's refused lifts in 200 clean carries from 158 to under 20;
   refused swings among bot lifts and pokes from nine in ten to under one in ten; and the scripted human reads the same two things
   (`JamPlaytester.cpp:475-476`: its clean pokes fell from 2.8 to 0.9 a match because it swings into a grace).
2. **A defender that closes under control**: the sim has no backward skating (`JamSim.cpp:1234`: a body faces its move), so gap control as you built it stops
   and waits. Find the version that works inside the sim as it is (angle the carrier to the boards, stay goal-side at his speed, step up when he stops or
   loses the puck's protection) and prove it on the carry drill: the clean stop-and-go from 8 m past a pro stays above 80 % (law 3 wins: R-0053), the
   straight charge stays under 20 %, and a carrier that dawdles inside 7 m loses the puck more often than today.
3. **The puck stays with the human's side** (the two bars your second evolution missed): a mate's share of the puck while the human waits from 4-6 % to 15 %
   or more; the puck still the human's two seconds after a mate's pass from 6 % to 30 %: support that covers until the puck is kept, the chaser by arrival
   read with the human's real speed and facing (a bot leaves a puck he reaches within 0.3 s of it), shapes for three a side on the big rinks.
4. **Shot selection**: lane A120's two-process logs show 273 bot shots in a match, many from the corners near the goal line, and almost no goals. A bot shoots
   from where a shot can score (the slot, the point with a screen) and otherwise carries or passes. Numbers: shots a match and goals a shot before and after.

Measure the three cells before and after this evolution alone (600 a cell, paired seeds): report the move, do not tune the levels.
The gate: the full `Jam` filter green; `brain2=0` still gives back the first evolution's brain, and a new tune key gives back this evolution's start. Time box
120 minutes; interim in the status file.
