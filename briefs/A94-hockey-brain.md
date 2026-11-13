# Lane A94: the bots play hockey: space, carry or pass, an attack shape and a defence shape (device A, from H22, R-0045 law 2, Opus)

You are lane A94 of the Third Jam (team Hokej). Read `docs/VISION.md` first: law 2 is yours, word for word. Today a carrying bot skates at the net and shoots when it can; its
teammates fill roles by distance (a chaser, supports, backs: A83 added a weak-side support and a slot back for 5 v 5); nobody reads open ice, and the result is ten skaters
who converge. Your job: the hockey principles in the brain, different in attack, in defence and in the two transitions, so a match looks like hockey and going straight up the
middle fails against a set defence.

Read first: `docs/VISION.md`, R-0045, `docs/PLAN-RECONCILED.md`, `docs/experiments/FIVE.md` (A83: the roles for five, the measured arms and what they did to the win share and
to the human's touches), `PASSING.md` (A46: `mate.pass_to_human`, the pass to the open human), `CURVE-2.md`, `CURVE-3.md` (A70, A79: the levers, the hunt, what moved wins),
`Game/Source/JamCore/AI/JamAICore.cpp` + `.h` (`FJamBrain::Think`, `Decide`, `Attack`, `StickPlay`, the roles, `FJamBotSkill`, `CarrierBestPass`, `bReadLanes`, `bPassOut`,
`bClearLane`, the forecheck yield, the guard post, A85's round-the-net rule, A66's blade cap), `JamSimAdapter.cpp` (what the brain sees: `FJamAIWorld`), `JamPlaytester.cpp`
(the human's classes: not yours), `Game/Source/JamCore/Tests/BotTests.cpp`, `SeasonBotTests.cpp`, `tools/soak/playtest/soak.mjs` (the measurement), `tools/playtest/run.mjs`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a94` (created at main 8f6b157; build it yourself, unity off). Run no git command; never touch
`Game/Plugins/JamAssets`; never print `.env`; never kill a process you did not start; headless only; 8 soak workers at most, hold under 12 GB free. Lanes A92 (contact) and A93
(movement, sprint) change the sim under you and lane A86 the difficulty numbers: you own the brain's decisions (`JamAICore.cpp`, new fields in `FJamBotSkill` defaulted so no
level's numbers move without the principle being on), not the sim, not `DifficultyProfile.cpp`'s numbers (you add the on/off of each principle per level there as booleans or
shares and say so; A86 and the final curve tune them).

You own: the brain in `JamAICore.cpp/.h` (the space read, the carry-or-pass decision, the attack and defence shapes, the transitions), `JamSimAdapter.cpp` only for a fact
the brain lacks (say which), the on/off per level in `DifficultyProfile.cpp` (new fields only), `Game/Source/JamCore/Tests/HockeyBrainTests.cpp` (new), `tools/playtest/shape.mjs`
(the measurement of shape from a match log), `docs/experiments/BRAIN.md`, `status/A94.md`.

## Job
1. **The space read**: for the carrier: the first opponent on the lane ahead and the time until it arrives; for each teammate: its distance to the nearest opponent and
   whether the pass lane to it is open (a blade or a body on the line); cheap (ten skaters, every frame or every few frames with a stagger).
2. **Carry or pass**: the carrier keeps skating while the lane ahead is open for more than a second and no teammate is open nearer the net; it passes when pressure arrives
   inside a second, when a teammate is open and better placed, or when the lane closes; it shoots as today when the shot is on; it does not skate into two defenders
   (`Jam.Bot.CarrierDoesNotChargeTheWall`: a set defence of two on the lane and an open teammate: a pass inside 60 frames, 10 of 10 seeds).
3. **Attack shape** without the puck: a close support (an outlet behind or beside the carrier, 5 to 8 m), a far-side or net-front skater, the backs at the blue line, nobody
   in a teammate's lane; **defence shape**: one pressures and steers to the boards, the next covers the nearest pass, the rest take the slot and the far side, gap control (back
   off at the carrier's speed, close when it slows), nobody chases behind the play; **transitions**: on a turnover the nearest two react first (one to the puck, one to the lane),
   the others get to their shape. Each principle a field with a share or a boolean per level: rookie knows them and executes them late and loosely, all_star at once.
4. **Measure the shape** (`shape.mjs` from a per-frame match log): the mean distance between teammates (spread), the share of frames with three or more skaters of one team
   inside 6 m of the puck (the pile), passes per minute by bots, the carrier's carries that end in a pass, a shot, or a lost puck; before and after on 60 pro matches 5 v 5.
   The pile share should fall by half; bot passes per minute should at least double; goals per match within one of today's.
5. **The human keeps the game**: the human's touches per minute and the faceoffs without a human touch within 2 points of today's (A53's runner, 600 matches at pro with the
   regular class); the teammates still pass to the open human.
6. **The gate**: the full `Jam` filter green; `SeasonTakes`, `NoviceScoresAtRookie`, `HumanHasSomethingToDo` reported with their numbers if they move (their bars are the
   final curve's to re-set, not yours: a red one is reported with its count).
7. `BRAIN.md`: the principles as implemented, the fields, the shape table before and after, what a viewer will notice in one paragraph.

Report: the shape table, the human's numbers, the tests, the gate result, the file list. Write `status/A94.md` from the first minute. Interim at 45 minutes with steps 1-2.
Time box 120 minutes.
