# BRAIN-5: round a downed skater, pro's presser, and the stop's distance at three a side (lane A167, track Q2, evolution 5; Sat 14 Nov 04:58-)

Follows `BRAIN-4.md` (lane A153). Tree: the worktree `q2b` as the Director baselined it (a9f7443 plus the landed diffs of lanes A134 and A153; the
contact lane's `contact.downed_radius_m` is on main and not in this tree: everything here is measured without the sim's push-out). **Every rule of this
evolution ships at its neutral value** (the Director, 04:57: nothing goes on before the operator has played); this document carries the values that are
recommended, with their cells. Source: `Game/Source/JamCore/AI/JamAICore.h/.cpp`, `DifficultyProfile.cpp` (tune keys, one call), `Public/JamBaseline.h`,
tests `Tests/HockeyBrain5Tests.cpp` (`Jam.Bot.Hockey5.*`, the bar `Bar.Bot.Hockey5.Downed`) and `Private/Tests/JamCarrierSkillTests.cpp` (the bar
`Bar.Bot.Hockey5.Stops`). Raw logs under `Game/logs/a167/` (they do not land); cells in `tools/perf/results/a95/cells.jsonl` (labels `A167*`).

## 1. The verdicts

| item | verdict | recommended value (all neutral as shipped) | its cost |
|---|---|---|---|
| 1. The brain steers round a downed skater | **Steering alone does not do it.** The bots' spells at a downed skater's body fall by 41 to 55 %, the total by a third to 42 %; the bar (under 50 in 200 pro matches) is out of a brain rule's reach: the scripted human makes about 300 alone, and half of what is left begins at the knockdown itself. | For the human's mates: keep-out 1.3 m, way out 0.7 (`mate.hk_avoid_downed_m=1.3;mate.hk_avoid_downed_out=0.7`). For the opposing side only together with the curve. | mates only: pro +0.7. Both sides: rookie -1.5, pro **+2.7**, all_star -1.9 (the bar was 2 points: pro is over it). |
| 2. Pro's Press job | **Closed.** With a commit distance the presser kept on its job plays exactly what pro plays today, and the hunt radius no longer moves the stop's window. | pro: `opp.hk_press_in_hunt=1;opp.hk_press_commit_m=8` | none: 100 of 100 pro matches are hash-identical to the default with the two keys on |
| 3. The six-skater cuts | **Measured: the one distance holds at rookie only** (stop at 8 m: pro 47.5 %, all_star 0 % on the 120 m rinks). The smallest change is found: the three's chaser closes under control and commits at 6 m. | every level: `opp.hk_press_three=1;opp.hk_press_three_commit_m=6;opp.hk_press_retreat_cap=0.7` | large rink: rookie +0.6, pro -2.3, all_star **+4.4** |

## 2. Item 1: the brain steers round a downed skater

**The instrument.** `Bar.Bot.Hockey5.Downed` plays the matches in process with the playtest trace on every frame and counts, on the brain's own world,
the spells in which a standing skater's root is within 0.9 m of a downed skater's root for more than 6 frames (a pair; the scripted human counts as a
standing skater). On 200 pro matches as shipped it reads **1,511** (main's own counter, lane A157: 1,554 on a later tree).

**What a spell is.** Not a bot skating at a man who lies still. In 93 % of the frames a skater is at a downed man's body, the man is still sliding
faster than 1 m/s; the puck is within 1.5 m of him in 53 %; the standing skater is the puck's chaser in 44 %. Of the 1,511 spells, 603 begin within three
frames of the knockdown (the checker and the man it has just put down, travelling on together) and 908 later (a skater that arrives at a man already
down, usually for the puck that slides with him). By length: 472 of 7 to 10 frames, 505 of 11 to 20, 418 of 21 to 40, 116 longer; the longest 112.

**The rule** (`SteerRoundDowned`, `FJamHockeySense::AvoidDownedM`, applied in `Think` to every bot's move): a downed skater within the keep-out plus
2.5 m, ahead of the move and within the keep-out of its line, turns the move onto the tangent of his circle on the side it already leans to; a bot
inside the circle moves out and round (`AvoidDownedOut`: the share that goes straight out).

**Spells in 200 matches** (bots + the scripted human; build 4):

| level | as shipped | keep-out 1.3 m, out 0.7 | keep-out 1.6 m, out 0.7 |
|---|---|---|---|
| rookie | 662 (462 + 200) | 420 (209 + 211) | 423 (196 + 227) |
| pro | **1,511** (1,188 + 323) | **1,009** (700 + 309) | 907 (630 + 277) |
| all_star | 1,316 (1,056 + 260) | 765 (538 + 227) | 671 (451 + 220) |

At pro with the keep-out at 1.3 m the spells of 21 frames or more fall from 534 to 245 and those begun at the knockdown from 603 to 465.

**What was tried and did not help** (200 pro matches each; each was built behind a tune key and is out of the code again):

| variant (keep-out 1.3 m unless said) | spells | |
|---|---|---|
| the blend alone (no `AvoidDownedOut`) | 1,210 | the way out at 0.7 is worth a sixth more |
| keep-out 1.1 m / 1.6 m (blend) | 1,217 / 1,063 | |
| a brake at contact (the decided stop while the bot runs into him) | 1,501 with no keep-out; 1,190 with it | a bot that brakes beside a sliding man stops beside him |
| the circle drawn where he will be in 0.2 s / 0.4 s | 1,258 / 1,368 | worse than where he is |
| an exception for a puck within 1.5 m of him | 1,311 | the exception is most of the cases |
| the way out at 1.0 | 1,028 | as 0.7 |

**Why the bar is out of reach for a brain rule.** Two bodies that have just collided are 0.8 m apart (the sim's body circles). To be outside 0.9 m
inside 6 frames the gap must open at 1 m/s at once; a skater accelerates at 12 m/s2, so a tenth of a metre takes 8 frames whatever the stick says. A
spell that begins at the knockdown is over the line before the brain can end it. And the scripted human's own 300 spells are not the brain's. What
the still shows (a skater standing AT the lying figure) is the long spell, and those the rule halves; the short ones need the sim's push-out.

**The cells** (600 each, the ladder's seeds; before = lane A153's cells on the same seeds):

| cell | before | keep-out 1.3 m, out 0.7, both sides | opposing side only | mates only |
|---|---|---|---|---|
| novice at rookie, regular | 84.7 | 83.2 (-1.5) | | |
| regular at pro, regular | 50.5 | **53.2** (+2.7) | 53.8 (+3.3) | **51.2** (+0.7) |
| good at all_star, regular | 32.7 | 30.8 (-1.9) | | |

The move at pro is the opposing side's: five bots that go round a downed man reach the puck beside him later. The spells by side at pro (200 matches):
the mates only 1,295 (bots 978), the opposing side only 1,172 (bots 840), both 1,009 (bots 700).

**Recommendation.** (a) The mates' keep-out can go on by itself after the operator has played: it is inside the 2-point bar at pro (+0.7; its rookie and
all_star cells were not run), and it takes a seventh of the spells (1,511 to 1,295). (b) The opposing
side's goes on with the curve, or with the sim's push-out: `contact.downed_radius_m` does the short spells that steering cannot, and it cost all_star
8.4 points on main because the push-out turns a bot's course by force on 106 frames a match; a bot that already steers round should need it far less.
That pair (the push-out on, the keep-out on) is the measurement to make on main, where both exist; it could not be made in this tree.

## 3. Item 2: pro's Press job

`Decide` takes every bot inside its skill's hunt radius off its shape job. `bPressInHunt` (evolution 3) keeps the presser on its job there; all_star
ships with it (R-0058). At pro the pure Press job is too soft late and too hard early. `PressCommitM` (this evolution) is the distance inside which the
presser commits as a hunting bot does. Pro on hockey.regular, 200 seeds an arm (`Bar.Bot.Hockey5.Stops`; the carry scene of law 3):

| pro, the stop begins at | 5 m | 6 m | 8 m | 9.5 m | 11 m | straight |
|---|---|---|---|---|---|---|
| as shipped (hunt 8 m) | 7.5 | 7.5 | 100 | 97.0 | 86.0 | 0 |
| `opp.hk_press_in_hunt=1` (the brief's arm) | **31.0** | 92.0 | 99.0 | **55.0** | 21.5 | 0 |
| `opp.hunt_radius_m=20` (the hunt as a curve lever today) | 0 | 1.0 | **21.0** | 84.5 | 99.0 | 0 |
| **`opp.hk_press_in_hunt=1;opp.hk_press_commit_m=8`** | **7.5** | 7.5 | **100** | **97.0** | 86.0 | 0 |
| the same with `opp.hunt_radius_m=20`, and with 30 | 7.5 | 7.5 | 100 | 97.0 | 86.0 | 0 |
| commit 6 m, hunt 20 | 16.5 | 39.0 | 100 | 98.0 | 97.5 | 0 |
| commit 10 m, hunt 20 | 0.5 | 2.0 | 70.5 | 100 | 90.5 | 0 |

- The brief's bars with the switch on (the stop at 9.5 m at 95 % or more, at 5 m at 10 % or less): **not met by the switch alone (55.0 and 31.0), met
  with the commit distance at 8 m (97.0 and 7.5)**: the line is pro as shipped, to the digit.
- In matches too: with the two keys on, 100 of 100 pro matches on the regular rink are hash-identical to the default (and with item 3's three keys
  on top as well: they do not play at five a side).

**For the curve lane, in one paragraph.** With `opp.hk_press_in_hunt=1;opp.hk_press_commit_m=8` in pro's profile, the presser's approach is fixed by
the commit distance and no longer by the hunt radius: the stop's window stays where it is (8 m 100 %, 9.5 m 97 %, the straight charge 0) at hunt 8, 20
or 30. The hunt radius then decides only how early the other four leave their jobs for the carrier, so it can be raised at pro as an aggression dial
without touching law 3 (R-0055 point 1 said it could not be: at hunt 20 the stop at 8 m read 21 %). What the freed hunt is worth at pro (600 matches, the regular rink): with the commit at 8 m
and the hunt at 20 m the cell reads **37.7 %** against 50.5 % (13 points of aggression, law 3 where it is); the hunt at 20 m without the commit reads
33.2 % and breaks the drill.
The commit distance itself is the window's dial (6 m: the window opens at 6 m; 10 m: it moves out and 8 m reads 70.5 %): it is law 3's number and
should stay at 8.

## 4. Item 3: the six-skater cuts

Three a side has no shape jobs: the opposing chaser charges the carrier from wherever it is, and on the 120 m rinks all_star does it at 1.42 times the
pace. The stop begun at 8 m, as shipped (`Bar.Bot.Hockey5.Stops`, 200 seeds a cell; hockey.raw and hockey.boost read as hockey.arena.large does):

| cut | level | 6 m | **8 m** | 9.5 m | 11 m | 12 m | straight |
|---|---|---|---|---|---|---|---|
| hockey.arena.large (120 x 60) | rookie | 0.5 | **87.5** | 100 | 100 | 100 | 0 |
| | pro | 0 | **47.5** | 100 | 78.0 | 50.0 | 0 |
| | all_star | 0 | **0** | 52.5 | 100 | 88.0 | 0 |
| hockey.chaos (90 x 45) | rookie | 0 | **80.5** | 100 | 100 | 100 | 0 |
| | pro | 0 | **9.0** | 86.0 | 97.5 | 89.0 | 0 |
| | all_star | 0.5 | **0** | 22.5 | 91.5 | 100 | 0 |

**The one distance (8 m at 80 % or more) holds at rookie only.** At pro the window is 9.5 m; at all_star 11 to 12 m.

**The smallest change**: the same two-phase approach the five's presser has. `bPressThree`: the chaser that is goal-side of a carrier that goes plays
the Press job's gap; `PressThreeCommitM`: inside 6 m it commits; `PressRetreatCap` 0.7: the gap's retreat is capped at 0.7 of full stick (the Press
job moves back with the carrier's velocity over the skater's top speed; on the 120 m rinks the top speed is 8 m/s, a carrier at full speed makes that
1, the close is cancelled, the chaser stands still and a straight charge passes it: 79 % at rookie and 59.5 % at pro before the cap).
With `opp.hk_press_three=1`, the commit at 6 m and the cap at 0.7:

| cut | rookie: 8 m; straight | pro: 8 m; straight | all_star: 8 m; straight | the window at pro (6 / 8 / 9.5 / 11 / 12 m) |
|---|---|---|---|---|
| hockey.arena.large | **100**; 0 | **99.0**; 0 | **99.0**; 0 | 28.0 / 99.0 / 60.0 / 7.0 / 7.5 |
| hockey.chaos | **100**; 0 | **100**; 0 | **91.5**; 0 | 14.5 / 100 / 97.5 / 98.5 / 95.0 |
| hockey.arena.wide (140 x 70) | **100**; 0 | **100**; 0 | **99.0**; 0 | 28.0 / 100 / 60.5 / 4.0 / 3.5 |
| hockey.boost | **100**; 0 | **99.0**; 0 | **99.0**; 0 | 27.0 / 99.0 / 60.0 / 7.0 / 7.5 |

- Every level passes from 8 m on every cut measured, and the straight charge passes nowhere. On the 120 m rinks the long stop goes at pro and all_star
  (from 11 m the carrier is contained), as it did at all_star on the regular rink (BRAIN-4.md).
- The cost on the large rink (600 each, hockey.arena.large; before -> with the three keys): novice at rookie 86.2 -> 86.8 (+0.6), regular at pro
  54.8 -> 52.5 (-2.3), good at all_star 30.3 -> **34.7** (+4.4: over the ladder's 4). The share of matches with a half-minute without the puck: 61.8 ->
  66.3 %, 56.2 -> 54.3 %, 30.0 -> 27.8 %.
- The three's commit distance is its own field (`PressThreeCommitM`), so one profile can hold pro's five at 8 m and the three's chaser at 6 m: with all
  five keys on, the carry scene reads the same on both rinks as with each set alone (`Bar.Bot.Hockey5.Stops`, the last arm of each block).
- It is three numbers on the opposing side at every level, so it is a ladder change on the large cells: it goes on with the curve lane's next pass on
  the big ice, not alone.

## 5. Tests, the gate, the neutral proof

`Jam.Bot.Hockey5.*` (pure worlds, both arms in one run):

| test | before | after |
|---|---|---|
| `SteerRound` | a bot chasing a puck with a downed skater on its way: its line passes 0.30 m from him | 1.30 m; a standing skater there is not steered round; the pure read: the tangent on the side the move leans to, the length kept, a line that clears him untouched, inside the circle the move leads away |
| `PressThree` | the chaser 10 m goal-side of a carrier at 6 m/s: full stick at him (-1.07) | under control (-0.27); a carrier that dawdles: as before |
| `Neutral` | | as shipped every rule is at its neutral value at every level; `brain5=0`, `brain4=0`, `brain3=0`, `brain2=0` each put them there; the same match twice is the same match; as shipped = `brain5=0` |

- **The full `Jam` filter on the final build (build 5): 239 of 239** (`Game/logs/a167/ue-full-2.log`; 236 before the lane, 3 new; no "missing usage
  flag" line). Build 4 (before the three's own commit field): 239 of 239.
- **Law 3** in that run, unchanged: pro, the stop begun at 8 m 200 of 200 and the straight charge 0; all_star 8 m 100 %, 9.5 m 90 %, straight 0.
- **The default match is the tree's as it stood, hash for hash** (build 5 against lane A153's READY 3 cells, 100 seeds each, `Game/logs/a167/cells4.txt`):
  the regular rink at rookie, pro and all_star 100 of 100 each; the large rink at pro and all_star 100 of 100 each.
  `Jam.Core.Determinism` seed 1 frame 600: hockey.regular `ef34dd88c7c62b47`, hockey.raw `79d43eb5ea69a468`, as before the lane.
- The measured tunes are the final build's: the three's own key on build 5 against build 4's large pro cell, 100 of 100 hashes equal.
- No field was added to skater or puck state. The brain keeps no new state: every rule is a pure read of the world.
- The gate's node tests: 243 pass, 1 fail that is the worktree's (`BarkData.inl` against `barks.json`, the CRLF copy).

## 6. What does not hold, and what is open

1. Item 1's bar (under 50 spells in 200 pro matches without the push-out): **not met: 1,009** (907 with a keep-out of 1.6 m). Not reachable by steering.
2. Item 1's cell bar (within 2 points): **not met at pro with both sides steering (+2.7)**; met with the mates alone (+0.7).
3. The keep-out together with the sim's push-out was not measured (the key is not in this tree).
4. Item 3's change was measured in the carry scene and on three large-rink cells; hockey.chaos, hockey.raw, hockey.boost and the season's six-skater
   takes have no cells here. The modes' and the season's win rates would move with it.
5. Not run: the Shipping compile, the acceptance run, any hand play. Nothing a player does changes as shipped.

## 7. How to repeat

```
JAM_A167_TUNES="off:|on:opp.hk_avoid_downed_m=1.3;mate.hk_avoid_downed_m=1.3;opp.hk_avoid_downed_out=0.7;mate.hk_avoid_downed_out=0.7" \
JAM_A167_LEVELS=pro,rookie,all_star JAM_A167_MATCHES=200 tools/perf/results/a167/tests.sh Bar.Bot.Hockey5.Downed <tag>     # grep DOWNED in Game/logs/a167/ue-<tag>.log
tools/perf/results/a167/tests.sh Bar.Bot.Hockey5.Stops <tag>                                                             # grep "PRESSPRO\|SIX "
tools/perf/results/a167/tests.sh Jam <tag>                                                                               # the gate's filter
tools/perf/results/a167/idle.sh <label> pro regular 600 "<tune>"                                                         # one cell (CLS=novice|regular|good)
```
