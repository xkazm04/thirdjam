# Lane A34: a human-like playtester and the fun metrics (device A, from H11.5)

You are lane A34 of the Third Jam (team Hokej). R-0028 point 3: a bot that plays like a person, through the real input path, so the game can be measured for fun while the operator is
away. Read first: `docs/experiments/BALANCING.md` (A3's scripted human and the sweep), `docs/experiments/FEEL.md` (what the operator felt at H7: puck ran away, stick, faceoff),
`Game/Source/JamCore/AI/{JamBotMatch,JamSimAdapter}.cpp` and the tournament commandlet (`-run=JamTournament`, `tools/tournament/run.mjs`), `Game/Source/JamCore/Tests/BotTests.cpp`
(HumanHasSomethingToDo, PlayerBotWins), `Game/Source/JamGame/Private/HokejGameMode.cpp` (`GatherInputs`, the scripted human), `contracts/events.md`, R-0021 (calibrations are the
Director's call, recorded), R-0023 (the curve), R-0028.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a34` (detached at the merge commit; build it yourself; headless only, no editor). Run no git command; change no profile
or sim number (you measure and propose; the Director applies through A3's worktree).

You own: `Game/Source/JamCore/AI/JamPlaytester.{h,cpp}` (new), `tools/playtest/**`, `docs/experiments/PLAYTEST.md`, `status/A34.md`.

## Job
1. **The playtester**: a controller for the human slot that behaves like a first-time player and like a good one, selectable by `-player=novice|regular|good`: reaction delay
   (12 / 6 / 3 frames), aim noise (10 / 5 / 2 degrees), a decision model (chase the puck, carry toward the net, shoot when the lane is open or the charge is full, pass when a teammate
   is ahead, poke when within reach, check when closing from the side, forecheck after a lost draw), stamina of attention (the novice stops pressing for 30 frames every 10
   seconds), and a controller-like input (analog move with ramp, buttons held for charge). It goes through `FJamInput` exactly as the pawn does.
2. **Fun metrics per match** (JSON lines from the commandlet): touches per minute, carries over 3 s, shots, shots on target, goals for and against, lead changes, time to the first
   touch and the first shot and the first goal, faceoffs without a touch, checks landed and taken, pokes, time with nothing to do (no touch for 15 s), result. Twenty seeds per
   cell: three players x three difficulties x two rinks.
3. **Reading**: `docs/experiments/PLAYTEST.md`: the table, what a novice experiences in the first two minutes at rookie (the M1 bar: a fresh human scores within two minutes),
   where the good player is bored at all_star, and five calibration proposals with the numbers that would move (for A3's worktree and the Director), plus three gameplay
   ideas with evidence from the data (for example: a one-timer bonus if the pass arrives in the shooting window; a breakaway speed burst; a deke input).
4. Tests: `Jam.Playtest` (the novice scores at rookie within 7,200 frames in 2 of 3 seeds; the good player never idles 15 s at all_star), headless.

Report: the table, the five proposals, the three ideas, the file list. Time box 90 minutes; interim at 45 with the novice numbers.
