# Lane A133, track C0 (the curve and the first ten minutes), evolution 2 (device A, Opus)

You are lane A133 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, `docs/VISION.md` (law 3 especially), R-0046 and R-0048 to R-0053 at the end of `contracts/RULINGS.md`, and the track's first evolution in
your worktree: `status/A112.md`, `docs/experiments/CURVE-6.md`, `tools/perf/results/a112/**` (its method, its levers and every row it measured), then the three
docs of what landed under it: `docs/experiments/MOVEMENT-3.md` (track F2's carrier rules), `docs/experiments/BRAIN-2.md` (the brain's second evolution),
`docs/experiments/CONTACT-3.md` (the release grace).

Your worktree and the commit it stands on are named in the Director's message that starts you: a fresh worktree at the newest main, **with lane A112's changes
already applied on top by the Director** (its profile numbers, `MateSpeedScale`, the whistle off in the regular cuts and every take, take-3 at rookie). It has no
binaries: build the editor first. Your status file: `status/A133.md`. Port 4733. Family as the track's: the difficulty profiles, the shipped cartridges' rules
and levels, the season's takes, `tools/perf/results/**`, `docs/experiments/CURVE-7.md`, and the tests whose budgets are set from the soak.

## Where the game stands
Everything that moves the ladder tonight is now in one tree: the puck kept (+13 to +23 for the human), the carrier's take grace, harder brake and earned first
step (+0 / +7 / +3), the brain that plays as a side and reads the shield and the grace (about -4 / -4 / -10 on its own tree), the release grace (neutral), the
whistle off (+5 to +10), and lane A112's first profile (rookie pokes after 6 frames and shoots from range; pro pokes after 2, hunts 20 m; all_star hunts 30 m).
Nobody has measured the sum. **No package is cut until this lane lands** (R-0050): the operator has had no new build since 19:46.

## Evolution 2, in this order
1. **The tree is green first.** Lane A112 left seven red tests: fix what is yours (`ContactBoardMatch`'s pinned pro hashes need the profile's neutral in their
   neutral arm, `Designer.CutsRoundTrip` and `Designer.PromptParity` need the Designer data regenerated against the whistle-off `hockey.regular`,
   `Progression.SeasonRoundTrip` expects take-3 at rookie, `ModeKeys.Sprint` and `Effects.BotMatchWithEffects` undiagnosed: diagnose them). The seventh is a
   ruling, next item.
2. **Law 3 wins over the curve (R-0053).** `Jam.Core.Movement.StopAndGo` must pass at pro: a stop begun at 8 m beats a pro defender at least 80 % of the time.
   If pro's 2-frame poke still breaks it on this tree (the take grace, the harder brake and the brain's read of the grace may already have changed that:
   measure first), pro's poke goes back to the smallest value that keeps the drill green, and the points come from elsewhere (hunt, how soon and how hard
   the bots check, support, the shot). The same bar at all_star is 50 %.
3. **Measure the six cells** (three levels, regular and large rink) at 600 a cell on this tree as it stands; then **trim** to the targets (85 / 48 / 26, within 3
   points) by aggression first and speed last, confirm at 2,000 a cell. Lane A112's ranking of levers for each direction is in its report (status file).
4. **The season ramps**: the novice's win rate falls by no more than 20 points between neighbouring takes from 3 to 8 (63 points today between 3 and 4), with a
   shot in every take-1-to-3 match; takes 4 and 5 get the intermediate steps (a level name and at most one `ai` number each).
5. **The first look is clay** (R-0054): `look.tier` `p3_clay` in `hockey.regular`, `hockey.arena.wide`, `hockey.arena.large` and takes 1 to 4 (takes 5 to 8 and the
   modes keep theirs); the acceptance run's cut follows the regular cut; the Designer's cached cuts and prompt data regenerated; name every test that pinned
   the night tier. The sim's hash does not move (a look is not a rule): prove it.
6. Budgets of the soak-set tests (`BotTests.cpp:221` sits four faceoffs from red: reset from your own measurement).

Write `LADDER READY` in your status file the moment items 1 to 3 are green and measured, with the file list: the Director lands it at once and cuts H27; items
4 to 6 follow as a second landing. Proof: the full `Jam` filter green; `CURVE-7.md` with the tables; the neutral proofs (`PreA95Tune` and every key's neutral)
still passing. `-Multiprocess` on every headless start; few processes, many seeds; builds no longer queue across worktrees. Time box 120 minutes.
