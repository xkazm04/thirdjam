# Lane A86: the curve on the 5 v 5 game with the carrier reach fixed: targets, aggression, budgets (device A, from H20, R-0029 / R-0039 / R-0041, Opus)

You are lane A86 of the Third Jam (team Hokej). Three landings moved the game under the curve: the carrier is slower and cannot sprint (A82), 5 v 5 is the regular game with
two new roles (A83: pro regular 47 -> 37 % on the old profiles; all_star good 40 -> 25), and the carrier's reach lets the human shoot and pass with the blade extended (A85).
Lane A79 measured the all_star lever family on 3 v 3 (`docs/experiments/CURVE-3.md`: which aggression levers shrink the gap between the good and the regular player). Your
job: the three R-0029 cells back on target on the 5 v 5 game, with aggression as the ladder (R-0041), the large rink too, and the gate budgets set from the soak's measured
shares.

Read first: `CURVE-2.md` (A70's method: 2,000 matches per cell on A53's seeds; the levers and their effects), `CURVE-3.md` (A79), `FIVE.md` (A83: the 5 v 5 numbers, the roles,
`mate.pass_to_human` does not move the share), `MECHANICS-3.md` (A85: the reach, the flat tap), `SOAK-PLAYTEST.md`, `Game/Source/JamCore/AI/DifficultyProfile.cpp`,
`JamAICore.h` (the pro defaults live there: `RedSkill`, `ProOppSkill`, `CheckMinClosing`, `bLift`, `SweepReactFrames`), `Game/Source/JamCore/Tests/BotTests.cpp`
(`HumanHasSomethingToDo`'s budget line), `SeasonBotTests.cpp`, `JamPlaytester.cpp` (the gate's 20-seed tests), `tools/soak/playtest/soak.mjs`, R-0029, R-0037, R-0039, R-0041.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a86` (the Director creates it at the commit that lands A83 and A85; build it yourself, unity off). Run no git
command; never touch `Game/Plugins/JamAssets`; never print `.env`; never kill a process you did not start; headless only; 8 soak workers, hold under 12 GB free. Lane A84
(abilities, input mapping) may still run: do not touch `InputMapping` or the ability rule.

You own: `DifficultyProfile.cpp`, the pro defaults in `JamAICore.h` (numbers only), `BotTests.cpp` (the budget line), `JamPlaytester.cpp` only for a 20-seed bar that must move
with the measured share (written), `docs/experiments/CURVE-4.md`, `status/A86.md`.

## Job
0. **The player the curve measures is the player** (R-0043 point 2): the playtester classes must carry the puck the way the mouse does: the stick aimed at where the class
   looks (the net, the pass target, away from the nearest defender when carrying), the blade at the reach the cursor gives (full aim: 0.4 + `stick.reach_m`), not at rest.
   Check what `JamPlaytester.cpp` writes into `FJamInput::Stick` today against what `ReadHumanInput` writes for a human who aims 6 m ahead; make the classes do the same
   (the novice sloppily: its aim noise applies); then the baseline. Write the before and after of this step alone: it is the size of the fiction the earlier curves had.
1. Baseline on your build, 2,000 matches per cell: the nine class x level cells on hockey.regular (5 v 5), the three target cells on the large rink; the human-touch share
   (faceoffs without a human touch) and checks landed on the human per match in every cell.
2. The ladder as aggression (R-0041): rookie bots rarely check and never lift; pro check and lift as a matter of course; all_star hunt (CheckMinClosing 2-3, lift at contact,
   knockdowns every shift); speed per level the last lever, down from 1.3 at all_star where aggression carries the win rate; A79's chosen lever applied if it holds on 5 v 5.
   Each lever one at a time, 600 matches, then the final 2,000.
3. The targets: novice 85 at rookie, regular 50 at pro, good 25 at all_star, inside the interval on both rinks; the regular player at all_star as high as the aggression levers
   allow (the record for the ruling).
4. Budgets from the soak: `HumanHasSomethingToDo` at the measured share plus 2 points; `NoviceScoresAtRookie` keeps 10 of 20 if the soak's novice first-goal-within-two-minutes
   share supports it (write the share); `SeasonTakes`' AI-shot clause stays. The full `Jam` filter green.
5. `CURVE-4.md`: the baseline, each lever's effect, the final table, the shares, the record for R-0029's amendment.

Report: the final table on both rinks, the levers, the budgets with their measured shares, the gate result, the file list. Interim at 45 minutes with the baseline. Time box
120 minutes.
