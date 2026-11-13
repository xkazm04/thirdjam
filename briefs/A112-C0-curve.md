# Lane A112, track C0: the curve and the first ten minutes (device A, evolution 1, Opus)

You are lane A112 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, `docs/VISION.md`, R-0046, R-0048 and R-0049 at the end of `contracts/RULINGS.md`, `docs/experiments/CURVE-5.md` (how lane A95 set the ladder:
its tool is `tools/perf/results/a95/curve.mjs`, its targets are yours), and sections 3, 4 and 6 of `docs/design/REVIEW-1.md` (the whistle, take 3, all_star's speed).

Your worktree and status file are named in the Director's message that starts you. Family: the difficulty profiles (the level numbers in the brain's profile table and
the cartridges' `ai` blocks), the shipped cartridges under `Game/Content/Cartridges/**` (rules and levels only: no mode's character), `tools/perf/results/**`,
`docs/experiments/CURVE-6.md`, and the tests whose budgets are set from the soak (`BotTests.cpp`, `SeasonBotTests.cpp`: budgets only).

## Why now
Track F2 has landed the rule that a carried puck leaves its carrier only by an action (`game.stick.passive_take` false; R-0048). The human now keeps the puck, so
every cell of the ladder has moved. The operator accepted the ladder's shape (novice at rookie about 85 %, regular at pro about 48 %, good at all_star about 25 %,
on both rinks): give it back to him on the new rule.

## Evolution 1, in this order
1. **Measure first**: the three target cells on both rinks on main as it is, 600 matches a cell, few processes with many seeds each. Write the table to the status file.
2. **The whistle leaves the regular game** (R-0048 point 2): `rules.penalties` false in `hockey.regular`, `hockey.arena.wide`, `hockey.arena.large` and the season's
   takes (`ladder/take-*.cartridge.json` and their fallbacks) unless a take's own card text is about the whistle or the box (list each take's card line and your call in
   the report); the modes keep what they have. Fix the tests this turns (name each).
3. **The first ten minutes** (R-0048 point 3): the season's first three takes are rookie; a novice wins take 3 at least 60 % with a shot in every match; takes 4 to 8
   rise from there without a step of more than 20 points between neighbours (measure the season in order, the three playtester classes).
4. **all_star's speed is the opposing side's only** (R-0048 point 4): the human's mates skate at the human's pace at every level (today 15.5 against 11.3 m/s). If the
   profile cannot say this today, add the smallest key that can (schema, defaults, limits, README, a neutral value that gives back today's game), and say so.
5. **Re-set the ladder** to the targets within 3 points on 2,000 matches a cell on both rinks, by aggression first (how often and how hard the bots hit, how fast they
   close, how soon they react) and speed last. Report the numbers that moved and why.
Then the neutral proof for every key you added, the full `Jam` filter green, and `CURVE-6.md` with the tables.

The curve is a standing track: the next evolutions on other tracks (release grace, take grace, the brain braking on purpose and passing to the human) will move it
again. Your "next three" should say how the track re-measures cheaply after each landing (600 a cell) and when it re-sets (the last set is at the rule freeze,
Saturday 19:00).

Time box 120 minutes; interim tables in the status file as they complete.
