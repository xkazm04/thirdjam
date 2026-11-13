# Lane A95: the vision on one tree: A92 + A93 on A94, the acceptance run green, and the one curve (device A, from H23, R-0045 / R-0029, Opus)

You are lane A95 of the Third Jam (team Hokej). Three lanes built the operator's vision (`docs/VISION.md`, R-0045) in parallel: A94 the hockey brain (on main), A92 contact
by inertia and A93 movement, sprint and puck distance (both applied uncommitted in your worktree by the Director, who resolved their merge conflicts). Each was green
alone on its own base; together and on today's main they are not yet: A93 reports two novice gate tests red on its new defaults and the acceptance run's cross-check row
red under its burst and reach rules; A92 re-staged the contact rows; the neutral-proof tests of both pin hashes of an older main; the curve is off target in every cell
(`docs/experiments/CURVE-4.md`: with a mouse-style player on 5 v 5: novice at rookie 97, regular at pro 33, good at all_star 13; and A94 moved the regular player at pro
from 38 to 80). Your job: one tree that is green on the gate and the acceptance run, and the difficulty ladder set once, on the real player, as the vision describes it.

Read first: `docs/VISION.md`, `docs/PLAN-RECONCILED.md`, R-0029, R-0039, R-0041, R-0043, R-0044, R-0045, `docs/experiments/CONTACT.md` (A92), `MOVEMENT.md` (A93: the curve
sensitivity table, the held-S camera finding, the bots' 0 sprint frames), `BRAIN.md` (A94: the principles per level, the `hk_*` tune keys, the arm table that shows which
principle moved the win share), `CURVE-4.md` (A86: job 0, the baseline, the levers it would try), `CURVE-3.md`, `CURVE-2.md`, `MODE-KEYS.md` (`ai.sprint`, `ai.body_check`,
`ai.react_frames`), `ACCEPT.md`, `status/A92.md`, `status/A93.md`, `status/A94.md`, `Game/Source/JamCore/AI/DifficultyProfile.cpp`, `JamAICore.h` (the pro defaults),
`JamPlaytester.cpp`, `Game/Source/JamCore/Tests/BotTests.cpp`, `tools/perf/results/a86/curve.mjs` (A86's soak driver: 2,000 matches per cell in minutes), `tools/soak/playtest/`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a95` (main with A94, plus A92 and A93 applied uncommitted; build it first; unity off; the Development game
package with `-WaitMutex`; no interactive editor). Run no git command; never touch `Game/Plugins/JamAssets`; never print `.env`; never kill a process you did not start;
8 soak workers, hold under 12 GB free. You are the only lane in the sim, the brain and the profiles now: the fences are down for you, but a rule of the vision is changed
only through its cartridge key or a profile number, never rewritten; what you cannot make green by a number goes back to its neutral value and is reported (R-0045's
process line).

You own: everything needed to make the tree green and the ladder right: the profiles (`DifficultyProfile.cpp`, the pro defaults), the playtester's use of the new verbs
(`JamPlaytester.cpp`: the classes brake by reversing on purpose, not by accident: a class whose steering reverses its input against its velocity must mean to stop; the
regular and good classes use the burst cut and the sprint with the puck when the lane is open), the bots' sprint (`ai.sprint` shares per level in the profiles: a defender
that never sprints makes "a defender's sprint is always faster" a dead rule), the neutral-proof tests' pins, the acceptance drill's rows, the gate budgets set from the
soak (R-0039's rule: measured share plus 2 points), `docs/experiments/CURVE-5.md`, `status/A95.md`.

## Job
1. **Green on the gate**: build; the full `Jam` filter; fix what the merge broke (re-pin `Jam.Core.MovementNeutral` and `Jam.Core.ContactNeutral` on this tree's neutral
   hashes, each proving the neutral block still gives the pre-vision game on this tree's brain); list every red test with its cause before changing a number.
2. **Green on the acceptance run** (a Development package; `tools/qa/accept/accept.ps1`): all rows, including A93's brake row and A92's contact rows; the cross-check row that
   A93's rules moved is re-staged by its cue, not by weakening the rule.
3. **The playtester plays the vision**: the classes brake on purpose, cut and burst, sprint with the puck when it pays; before and after per class (the novice least).
4. **The ladder, once** (2,000 matches per cell on A53's seeds, hockey.regular 5 v 5; then the three target cells on the large rink): novice 85 at rookie, regular 50 at pro,
   good 25 at all_star, each inside its interval. Levers in this order: how well and how late a level executes the hockey principles (A94's shares and delays); how hard it
   hits (the checks' closing threshold and share, the cross-check; with A92's model the same press now means different things at different speeds); the bots' sprint share;
   red skill; speed last. Rookie: knows the principles, late and loose, rarely hits, sprints little. All_star: executes at once, hunts, sprints: "a beating massacre" (R-0041)
   that a good player wins one time in four. One lever at a time at 600 matches, then the final 2,000; every row into `status/A95.md` as it completes.
5. **Budgets from the soak**: `HumanHasSomethingToDo` at the measured share plus 2; `NoviceScoresAtRookie` at 10 of 20 if the soak's share of novice goals inside two minutes
   supports it, else the measured share minus a margin, written; the season test as the Director left it.
6. **The gate and the acceptance run once more on the final numbers.** `CURVE-5.md`: the red list of job 1 with causes, the before and after of job 3, each lever's effect,
   the final table on both rinks, the budgets, what went back to neutral if anything.

Report: the final table, the levers, the budgets, the gate and acceptance results, what went neutral, the file list. Write `status/A95.md` from the first minute. Interim at
45 minutes with jobs 1 and 2. Time box 150 minutes.
