# BRAIN-3: the human's side takes a kept puck back (lane A134, track Q2, evolution 3; Sat 14 Nov 00:11-02:05)

Follows `BRAIN-2.md` (lane A115) and `INTEGRATION.md` (lane A138). Tree: main `e44df14` (the kept puck, the body shield, the take grace, the brain's second
evolution, all on). Source: `Game/Source/JamCore/AI/JamAICore.h/.cpp`, `DifficultyProfile.cpp` (tune keys, one call), `JamPlaytester.h/.cpp` (the scripted
human's stick read), `Public/JamBaseline.h` (one pair), tests `Game/Source/JamCore/Tests/HockeyBrain3Tests.cpp` (`Jam.Bot.Hockey3.*`). Raw logs under
`Game/logs/a134/` (they do not land); scripts under `tools/perf/results/a134/`; cells in `tools/perf/results/a95/cells.jsonl` (labels `A134*`).

**In one paragraph.** On the big rinks (three a side) at pro, 39.2 % of matches held a 30 s stretch in which the human never had the puck. The trace says
why: one opposing carrier skates at full speed with the puck kept, the human and one mate trail it 3 to 5 m behind, where its body shields the puck from
both sticks, and the third skater waits at its net 50 m away. Nine candidate rules were measured one at a time as profile tunes on one build. Two hold and
ship on, both for teams of fewer than five only: **the mates body-check** an opposing carrier from the front or the side, and **the back steps up** on a
carrier within 10 m. Together: **32.0 %** on 600 seeds (bar 25 %: **not met**). The regular rink (five a side) replays hash for hash, so the ladder's three
cells do not move. Seven rules are in the code at their neutral values with their numbers.

## 1. What was measured first (build 0, no edit; regular at pro, hockey.arena.large)

| | value |
|---|---|
| matches with a 30 s stretch without a human touch, 600 seeds (`A134-base-large`) | **39.2 %** (p50 28 s, p90 43 s); win 79.5 %, goals 3.38 : 1.74 |
| the same, traced seeds 1 to 100 (`idle.mjs`) | 37 %; in the p80-p95 stretches: opponents carry 48 %, loose 31 %, a mate 12 % |
| dominant cause of the 37 long stretches | an opposing carry 31, loose 4, a mate 2 |
| 10 s or more after the human's last touch (`poss.mjs`): human / mate / opponents / loose | 1 / 8 / 48 / 43 % |
| during those opposing carries (`tools/perf/results/a134/where.mjs`): the human to the puck | p50 4.4 m; within 2 to 8 m in 65 % of the samples |
| ... the nearest mate to the puck; the other mate (the back) | p50 5.2 m; p50 48.7 m |
| ... the carrier | anywhere on the ice (its x is uniform), p50 7.9 m/s; the opposing chaser in 70 % |
| ... goal-side of the carrier: the human; a mate | 57 %; 99 % (the back, at its post) |
| the scripted human's stick, a match | 14.0 pokes, 0.7 clean; 1.0 lifts; 3.4 checks landed |

**The long stretch is a chase.** Nothing the trailing two can do with a stick takes the puck (the body shield refuses it from behind), nobody meets the
carrier from the front, and nobody on the human's side uses the body: `FJamAIParams::Mate` has no `CheckMinClosing`.

## 2. The rules (each a field of `FJamHockeySense` with a tune key; `SetEvolution3()` in `JamAICore.h` holds what ships, per side)

| field, tune key | rule | ships | where |
|---|---|---|---|
| `MateCheckClosing`, `mate.hk_mate_check_closing`; `MateCheckMaxTeam`, `mate.hk_mate_check_max_team` | The human's mates body-check an opposing carrier at this closing speed and up, from the front or the side, never from behind (the rules of `FJamBotSkill::CheckMinClosing`), in a team of fewer skaters than the limit. | **2 m/s, teams under 5** (neutral -1) | `StickPlay` |
| `StepUpM`, `mate.hk_step_up_m` | A team without shape jobs (fewer than five): the back goes at an opposing carrier in its own half within this distance, while it is goal-side of it, instead of waiting for a puck inside 2.5 m. | **10 m** (neutral 0) | `Decide` |
| `YieldArriveS`, `mate.hk_yield_arrive_s` | A loose puck is left to the human by arrival, not by distance: a mate yields only while the human reaches the puck no later than this after the mate would. `ArriveSec()` reads his real velocity and facing: a turn costs 0.2 to 0.8 s and he drifts on his velocity meanwhile. | neutral (-1) | `Decide` |
| `BackcheckMps`, `mate.hk_backcheck_mps` | A team without shape jobs: a bot that is not goal-side of an opposing carrier faster than this skates for the carrier's lane to its net (`BackcheckPoint()`: the first point it reaches before the carrier), not for the puck behind the body. | neutral (0) | `Attack` |
| `PincerM`, `mate.hk_pincer_m` | A team without shape jobs: while the human is within this distance of an opposing carrier, his mate comes at it from the other side. | neutral (0) | `Attack` |
| `ShotMaxAngleDeg`, `hk_shot_max_angle_deg` (both sides) | No aimed shot and no one-timer from further off the net's axis than this, or from behind the goal line; the carrier takes the puck to the slot (9 m out) or passes. | neutral (0) | `Attack` |
| `bPressInHunt`, `opp.hk_press_in_hunt` | The presser of a five keeps its Press job inside its skill's hunt radius (today the hunt takes every bot inside the radius off its shape job, the presser included). | neutral (off) | `Decide` |
| `FJamPlaytester::bReadShield`, env `JAM_PLAYTESTER_READ=1` | Item 1: the scripted human presses no poke and no lift that `StickCannotTake` says the sim would refuse. | neutral (off) | `JamPlaytester.cpp` |

Switches: the profile tune `brain3=0` (both sides, every rule at its neutral value: lane A138's tree, hash for hash); `brain2=0` switches evolution 3 off
together with evolution 2; one rule by its key. `JamBaseline::BrainTune()` is `brain2=0;brain3=0`. For a run whose levels take no tune (the law-3 drill):
env `JAM_BRAIN3_ON=k` plays the opposing side with `bPressInHunt`.

**Why the two that ship are for teams under five.** A five has a second and a third defender near the puck (the Press, Cover and Weak jobs); three a side
has one mate near and one at the net. The rule that takes a kept puck there is the body, and it is the rule the opposing side has had since lane A45.
At five a side the same check moves the regular cell at pro by 3.2 points (section 4); the limit is one tune key for the curve lane to lift.

## 3. Each rule alone (profile tunes on one build; regular at pro, hockey.arena.large; idle30 = matches with a 30 s stretch without a human touch)

Seeds 1 to 600 (build 0 for the first block, tunes that need no code):

| arm | idle30 | p50 / p90 | win | goals |
|---|---|---|---|---|
| baseline | 39.2 % | 28 / 43 s | 79.5 | 3.38 : 1.74 |
| the mates check at 2 m/s (`mate.check_min_closing=2`) | **33.2 %** | 26 / 40 s | 84.3 | 3.81 : 1.61 |
| the mates' stick after 2 frames (`mate.hk_mate_stick_frames=2`) | 41.0 % | 28 / 43 s | 78.3 | 3.41 : 1.69 |
| the mates sprint in every chase (`mate.sprint_share=1`) | 38.8 % | 28 / 43 s | 81.3 | 3.56 : 1.61 |
| the mates lift every 30 frames (`mate.lift=1;mate.lift_cooldown_frames=30`) | 39.3 % | 28 / 42 s | 79.3 | 3.45 : 1.74 |
| the mates hardly ever leave a loose puck to the human (`yield_margin_m=-2`) | **51.5 %** | 30 / 46 s | 77.5 | 3.24 : 1.66 |
| **the check and the step-up at 10 m: what ships** (`A134c-su10-check2-large`) | **32.0 %** | 26 / 39 s | 82.3 | 3.71 : 1.60 |

Seeds 1 to 300 (build 2 and build 4; the baseline on these seeds: 39.0 %, win 77.7; what ships on these seeds: 33.0 %, win 84.7):

| arm | idle30 | p50 / p90 | win | read |
|---|---|---|---|---|
| the step-up at 10 m alone | 34.0 % | 27 / 43 s | 81.0 | helps; with the check it adds about a point at 600 seeds |
| the step-up at 16 m alone | 40.3 % | 28 / 45 s | 77.0 | no: the back leaves its net too early |
| the backcheck (`mate.hk_backcheck_mps=3`) | 42.7 % | 28 / 42 s | 75.3 | no: the mate that leaves the chase for the lane takes pressure off |
| the scripted human reads the shield (`JAM_PLAYTESTER_READ=1`) | 37.3 % | 28 / 41 s | 77.3 | his pokes 14.0 to 4.7 a match, clean 0.6 (unchanged), lifts 1.0 to 1.7, checks landed 3.4 to 2.6 |
| the pincer at 8 m, on top of what ships | 37.3 % | 28 / 42 s | 78.7 | no (33.0 without it) |
| the pincer at 5 m, on top of what ships | 36.0 % | 27 / 41 s | 81.7 | no |
| the yield by arrival at 0.3 s, on top of what ships | 35.3 % | 27 / 41 s | 82.0 | no (33.0 without it) |
| the shot angle at 60 degrees, both sides, on top of what ships | 33.3 % | 26 / 41 s | 82.0 | no gain; section 5 has the shots |
| the mates sprint in every chase, on top of what ships | 33.3 % | 26 / 40 s | 84.3 | no |
| the mates check from 1 m/s of closing, on top of what ships | 32.0 % | 26 / 39 s | 85.0 | inside the noise of 300 seeds |
| the scripted human reads the shield, on top of what ships | 37.3 % | 27 / 42 s | 81.7 | worse by 4 points: section 5 |

What the table says:

1. **The stick is not the lever, the body is.** A faster poke, a faster lift and a faster mate each move nothing: from behind the carrier the stick is
   refused whatever its timing. The check takes 6 points.
2. **A mate that takes the human's pucks costs him more than it wins** (51.5 %): the yield stays the human's.
3. **Leaving the chase costs.** The backcheck and the pincer both send the mate somewhere smarter than the puck's tail, and both read worse than the
   plain chase with a check at its end: with a scripted human who only chases, the mate's pressure on the carrier is what makes it pass or shoot.
4. **The bar of 25 % is not met: 32.0 %.** The traced 100 seeds of the shipped tree: 32 % (37 before); the opponents' share of all play 31 to 27 %; frames
   20 s or more after the human's last touch 2,775 to 1,908 (-31 %); the dominant cause of the long stretches is still an opposing carry (24 of 32).
   During those carries the picture of section 1 is unchanged (the human p50 4.5 m behind, the carrier at 7.9 m/s).
5. Item 3's two older bars, on this tree:
   - a mate's share of the puck 10 s or more after the human's touch (`poss.mjs`): big ice 8 % before and after; regular rink 7 % (bar 15 %: **not met**,
     not moved);
   - the puck still the human's two seconds after a mate's pass (`docs/design/review-1/humancarry.mjs`, 30 matches traced frame by frame, hockey.regular
     at pro, 5.3 such carries a match): **26 %** (lane A115 on its tree: 6 %; bar 30 %: **not met, by 4 points**); one second later 56 % (28); the carry's
     median 1.22 s (0.72). That gain is track F2's kept puck with the brain's second evolution, not this lane: nothing that ships here plays at five a
     side. The carries he won himself: 18 % held two seconds, median 0.75 s.

## 4. The cells (600 a cell, lane A53's seeds, `tools/perf/results/a95/curve.mjs`; before = lane A138's rows `A138b-*` on the same tree)

| cell | before | this evolution as shipped | proof |
|---|---|---|---|
| novice at rookie, regular | 98.2 | **98.2** (2.97 : 0.45) | 600 of 600 hashes equal to lane A138's raw file |
| regular at pro, regular | 67.8 | **67.8** (2.40 : 1.63) | 600 of 600 hashes equal |
| good at all_star, regular | 39.3 | **39.3** (1.82 : 2.30) | lane A138's row to the digit (five a side: the two rules do not run) |
| regular at pro, large | 79.5 | **82.3** (3.71 : 1.60) | +2.8 |
| good at all_star, large | 82.0 | **84.3** (3.73 : 1.60) | +2.3 |

Each rule alone at five a side, where it was measured:

| rule | cell | before | with the rule | ships |
|---|---|---|---|---|
| the mates check at 2 m/s in every team (`mate.hk_mate_check_max_team=99`) | novice at rookie, regular | 98.2 | 98.2 (2.98 : 0.40): 0.0; idle30 24.3 to 16.8 % | on for teams under five; **at five a side neutral**: all_star moves more than 4 points |
| | regular at pro, regular | 67.8 | 71.0 (2.50 : 1.50): **+3.2** | |
| | good at all_star, regular | 39.3 | 45.5 (1.95 : 2.18): **+6.2** | |
| the shot angle at 60 degrees, both sides | regular at pro, regular | 67.8 | 68.3 (2.48 : 1.60): +0.5 | neutral: it changes nothing (section 5) |
| the presser keeps its job inside the hunt (`opp.hk_press_in_hunt=1`) | good at all_star, regular | 39.3 | 38.3 (1.78 : 2.29): -1.0 | neutral (section 6) |

The yield by arrival, the backcheck, the pincer and the scripted human's read were measured on the big ice only (section 3: none helps there); they
ship at their neutral values and their regular cells were not run. The backcheck, the pincer and the step-up do not play at five a side at all.

## 5. Item 1 (the brain reads F2 for real): what was left

- **The pro's refused lifts in 200 clean carries: 0** (158 before the hooks; bar under 20). `Jam.Core.Movement.StopAndGo` prints it for every arm: "lifts
  refused by the body 0", pro and all_star, the stop at 5, 6, 8 and 9.5 m, the straight charge (`Game/logs/a134/ue-full-2.log`). Refused swings in
  matches were not counted (no counter in the playtest line: proposed below).
- **The scripted human's read** (`JamPlaytester.cpp`, the forecheck's poke and lift): built behind `FJamPlaytester::bReadShield`, off. With it he swings a
  third as often (14.0 to 4.7 pokes a match on the big ice) and takes the same 0.6 clean pokes: he swung into a shield nine times in ten, and not
  swinging does not open the puck, because he is still behind the carrier. It moves the big-ice pro cell by 0.4 points (77.7 to 77.3 on 300 seeds). It
  changes the instrument every cell is measured with, so it ships off until the curve lane sets a ladder with it (`JAM_PLAYTESTER_READ=1` plays it;
  `Jam.Bot.Hockey3.HumanReads` proves it: 14 swings in 20 frames unread, 0 read, 14 read in front of an open puck).

- **Item 4, shot selection** (`ShotMaxAngleDeg`): built, proven on a pure world (`Jam.Bot.Hockey3.ShotSelect`), and it finds nothing to remove in
  matches. Shots a match by the human / his mates / the opposing side, and the opposing side's goals a shot:

  | cell (600, or 300 on the big ice) | without | with the rule at 60 degrees |
  |---|---|---|
  | regular at pro, regular | 5.9 / 2.5 / 8.7; 1.63 goals: 0.19 a shot | 6.3 / 2.4 / 8.7; 1.60 goals: 0.18 a shot |
  | regular at pro, large, on top of what ships | 5.5 / 2.4 / 10.8; 1.57 goals: 0.15 a shot | 5.4 / 2.4 / 10.7; 1.66 goals: 0.15 a shot |

  The bots of a playtest match take 11 to 13 shots a match between them, not 273, and one opposing shot in six scores. Lane A120's count came from a
  two-process match; whatever it counted (dumps under the forecheck and board chips are shot presses too, and the rule leaves both alone), it is not
  an aimed shot from a bad angle. The rule ships at its neutral value; the count is worth a look in that lane's own log before anyone builds on it.

## 6. How all_star approaches (R-0055 point 2), and what the hunt radius really is

`Decide` takes a bot off its shape job while it "hunts": an opposing carrier within `HuntRadiusM`. That test does not look at the job, so it also takes
**the presser** off its Press job, the one job that closes on a carrier under control (goal-side, at the carrier's speed's gap). The hunt radius is
therefore the distance at which the presser stops closing and charges: 8 m at pro, 20 m at all_star, where it also sprints in every chase at 1.4 times
the pace. That is why all_star "does not creep", why its stop-and-go window is 10 to 12 m, and why raising the hunt at pro moves pro's window out
(lane A138's table, R-0055 point 1). `bPressInHunt` keeps the presser on its job inside the radius (the others still hunt). The law-3 drill
(`Jam.Core.Movement.StopAndGo`, 200 seeds an arm, env `JAM_BRAIN3_ON=k`; `Game/logs/a134/ue-sg-base.log`, `ue-sg-press.log`):

| the stop begins at | pro as shipped | pro, presser kept on its job | all_star as shipped | all_star, presser kept on its job |
|---|---|---|---|---|
| 5 m | 1.0 | 31.0 | | |
| 8 m | 100 | 98.0 | **0** (57 lifted, 143 knocked down) | **100** |
| 9.5 m | 97.0 | **54.5** | 38.5 | 90.0 |
| 11 m | | | 89.0 | 47.5 (86 timed out with the puck kept) |
| 12 m | | | 100 | 35.0 (93 timed out) |
| straight charge | 0 (200 lifted) | 0 (197 poked) | 0 | 0 (200 poked) |

- **At all_star it moves the window in to where pro's is**: a stop begun at 8 m beats all_star 200 times of 200 and the straight charge never does. A
  player who learned the stop on pro keeps his timing on all_star; what he loses is the long stop (from 11 m the presser contains him and the carry
  times out).
- **At pro it breaks the drill** (9.5 m 54.5 % against a bar of 95; a stop begun at 5 m passes 31 % against a bar of 10): pro's presser inside 8 m is
  better as it is.
- So it is a level's switch, not a rule for every level, and a level's numbers are the curve lane's: it ships off. **Its cost at all_star is one
  point**: good at all_star on the regular rink 39.3 to 38.3 % (600 matches, 1.78 : 2.29), inside the ladder's 4. It is the one lever found tonight that
  changes how all_star approaches without touching its speed or its cell.
- To switch it on for a level: one line in that level's profile (`Opp.Hk.bPressInHunt = true;` in `MakeAllStar()`, `DifficultyProfile.cpp`).
  `SetEvolution3` leaves a level's own value alone and `brain3=0` switches it off (`Jam.Bot.Hockey3.Neutral` asserts both). With it on,
  `Jam.Core.Movement.StopAndGo`'s all_star bars hold as they are written (the stop passes at least 50 % at the best of the four distances: 100 % at 8 m;
  the straight charge 0 %). It plays only where the Press job exists: at five a side.

## 7. Tests and the gate

`Jam.Bot.Hockey3.*` (8 tests, both arms in one run; the neutral arm must miss the bar):

| test | before | after |
|---|---|---|
| `MateCheck` | a mate meets an opposing carrier head-on at 8 m/s of closing: no check | checks; never from behind; not in a team at the limit |
| `StepUp` | a carrier 11 m from the back: the back's way is away from it (-1.00: its post) | at it (1.00); a carrier 31 m off: unchanged |
| `YieldArrive` | the human 4 m from a loose puck and skating away from it, the mate 6.3 m: the mate stands off | the mate takes it (`ArriveSec` 2.0 s against 0.4 s for a human skating at it); a human skating at it keeps it in both arms |
| `Backcheck` | behind a carrier that goes: the mate's way is at the puck (0.99) | along the carrier's lane to its net (0.99); the point is 40 m down the lane and the mate is there first (5.68 s against 5.71 s); inside 2.5 m and for a slow carrier: unchanged |
| `Pincer` | the human goal-side of the carrier: the mate comes to the same side | from the other side; the human 20 m off: unchanged |
| `ShotSelect` | the corner by the goal line, 77 degrees off the axis: shoots | no shot, its way is to the slot (0.98); the slot and the point shoot in both arms |
| `HumanReads` | the scripted human behind the body, or in front of a carrier inside its take grace: 14 swings in 20 frames | 0; in front of an open puck 14 |
| `Neutral` | | `brain3=0` and `brain2=0` put every rule at its neutral value, `brain3=0` leaves evolution 2 on, the opposing side's approach is untouched as shipped; pins of this evolution's start with `brain3=0`: hockey.regular seeds 1, 2 `f63ac57227255064` `3277289d27155239`, hockey.arena.large `95e0858675dfe149` `4c4de08a20d2f482` |

- **The full `Jam` filter on the final build (build 5): 215 of 215** (`Game/logs/a134/ue-full-3.log`; 207 on main, 8 new; no "missing usage flag" line).
  Build 3 (before the pincer): 214 of 214; build 4 (before the level's switch survived `SetEvolution3`): 215 of 215.
- The gate's node tests: 243 pass, 1 fail that is the worktree's (`BarkData.inl` against `barks.json`, the CRLF copy lanes A102, A121 and A138 reported).
- **Law 3** (`Jam.Core.Movement.StopAndGo`, in that run): pro, the stop begun at 8 m 200 of 200, at 9.5 m 97.0 %, the straight charge 0 %; all_star 0 / 38.5 /
  89.0 / 100 % from 8 / 9.5 / 11 / 12 m: every number as lane A138 left it. The opposing side's approach is not touched by anything that ships.
- **Neutral at volume:** `brain3=0` on the final source against build 0 (no edit), hockey.arena.large at pro: 100 of 100 hashes equal. The shipped
  defaults against the tune that was measured (`mate.hk_step_up_m=10;mate.hk_mate_check_closing=2` on the all-neutral build 2): 100 of 100 equal, on
  builds 3, 4 and 5. The regular rink at pro on build 5 against build 3's cell: 100 of 100 equal.
- `Jam.Core.Determinism` (seed 1, frame 600): hockey.regular `ef34dd88c7c62b47`, hockey.raw `79d43eb5ea69a468`, before and after.
- No field was added to skater or puck state: nothing new for `UJamSim::StateHash` or `Jam.Net.HashCoverage`. The new state is brain-side only (none:
  every rule is a pure read of the world).
- Not run: the acceptance run on a package (no verb of the player changed; the cursor is another lane's), the premerge gate's git steps, any hand play.

## 8. What does not hold, and what is open

1. **The bar of R-0055 point 5 is not met: 32.0 % against 25 %.** The human's side still does not take a kept puck back often enough on the big ice.
2. A mate's share of the puck while the human waits: 8 % (bar 15 %). Not moved.
3. The step-up's own worth is about one point at 600 seeds (5 at 300): it ships because it costs nothing measurable (goals against 1.61 to 1.60 with it)
   and is what a back does; the mates' check is the rule that carries the number.
4. Every three-a-side cut plays the two rules (hockey.arena.wide, hockey.raw, hockey.boost, hockey.chaos, the season's takes on six skaters): the gate's
   mode and season tests are green; their win rates were not measured.
5. The big-ice pro cell rises 2.8 points (79.5 to 82.3) and all_star's with it (section 4): R-0055 point 3 (all_star harder than pro on the big ice) is
   the curve lane's, and it is not helped by a stronger human's side.

## 9. How to repeat

```
tools/perf/results/a134/idle.sh <label> <level> <regular|large> <n> "<tune>"     # one cell, 3 workers, one summary line (CLS=novice|regular|good)
SEEDS=1-300 node tools/perf/results/a134/sum.mjs <label>                         # the same line over a subset of the seeds
node tools/perf/results/a134/hashcmp.mjs <labelA> <labelB>                       # per-seed hashes of two cells
tools/perf/results/a134/pt.sh <tag> hockey.arena.large pro regular 1-100 30      # one traced process -> Game/logs/a134/<tag>.log
node tools/soak/playtest/idle.mjs <log>; node tools/perf/results/a115/poss.mjs <log>; node tools/perf/results/a134/where.mjs <log>
tools/perf/results/a134/tests.sh Jam <tag>                                       # the gate's filter, headless, -Multiprocess
JAM_BRAIN3_ON=k tools/perf/results/a134/tests.sh Jam.Core.Movement.StopAndGo <tag>
```
