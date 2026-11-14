# BRAIN-6: the man who has only just won the puck, and all_star's own dial (lane A181, track Q2, evolution 6; Sat 14 Nov 08:13-09:35)

Follows `BRAIN-5.md` (lane A167). Tree: main `9e452219` (package H30's game). **Every rule of this evolution ships at its neutral value**; nothing lands
before the operator has played; this document carries what each dial is worth so that he can choose. Source: `Game/Source/JamCore/AI/JamAICore.h/.cpp`,
`DifficultyProfile.cpp` (tune keys, one call), `Public/JamBaseline.h`, tests `Tests/HockeyBrain6Tests.cpp` (`Jam.Bot.Hockey6.*`, the bar
`Bar.Bot.Hockey6.Human`) and `Private/Tests/JamCarrierSkillTests.cpp` (the bar `Bar.Bot.Hockey6.Law3Row`). Raw logs under `Game/logs/a181/` (they do
not land); cells in `tools/perf/results/a95/cells.jsonl` (labels `A181*`). Every cell is the ladder's instrument (`JAM_PLAYTESTER_CHECK=closing`), 600
matches unless said; the baselines on this tree: **novice at rookie 85.3, regular at pro 50.5, good at all_star 29.8**.

## 1. The two choices, in one table

| | what it is | the tune | what it does | what it costs |
|---|---|---|---|---|
| **Item 2: the presser leaves the man who has only just won the puck** | a bot presses no body check on a carry younger than N frames | `opp.hk_fresh_take_frames=15` | pro: he is knocked down 6.83 -> **3.17** times a match, as he won it 4.35 -> **0.91**, lying 8.89 -> **4.11 %** of play | rookie +3.7, pro **+5.7**, all_star **+8.9** |
| | the same, half the window | `opp.hk_fresh_take_frames=8` | pro: 6.83 -> 4.97, 4.35 -> 2.71, 8.89 -> 6.85 % | rookie +2.7, pro +0.8, all_star +3.9 |
| | (the contact lane's cap, CONTACT-9, for comparison) | `take_shield` 15 / 45 in the sim | pro: 6.9 -> 3.7, as he won it 3.9 -> 0.5, lying 9.3 -> 4.5 % | pro +2.8, all_star +6.0 |
| **Item 1, candidate A: all_star's second man hunts** | only the first presser of a carry closes under control; a presser that takes over while the same skater still carries charges | `opp.hk_second_man_hunts=1` | alone: nothing (2,000 seeds: 28.8 -> 29.9; its -2.1 at 600 was noise) | law 3's row untouched |
| **Item 1, candidate B: all_star's backs step up** | a five's backs meet a carrier inside N metres from the front (the three's rule of evolution 3, for a five) | `opp.hk_step_up_m=16;opp.hk_step_up_max_team=99` | with candidate A, at 2,000 seeds: good at all_star **28.8 -> 26.6 (-2.2)**; at 10 m 27.6 (-1.2). (At 600 seeds the pair read -5.0 and -3.1: twice too much.) | law 3's row untouched |

## 2. Item 2: the presser charges the puck's first toucher

**The instrument.** The contact lane's "as he won it" read-out is not on main. `Bar.Bot.Hockey6.Human` plays the matches in process with the trace on
every frame and reads the brain's own world: a knockdown of the human; "as he won it" = his carry was younger than 30 frames (also: than 15) and ended
within 3 frames of the knockdown; his share of play lying down. As shipped at pro it reads 6.83 knockdowns a match (CONTACT-9: 6.86), 4.35 as he won it
under 30 frames and 3.61 under 15 (CONTACT-9: 3.71 under half a second, 3.19 inside 15 frames), lying 8.89 % (9.08 %).

**The rule.** `FJamHockeySense::FreshTakeFrames`: in `StickPlay` a bot presses no body check on a carrier whose carry (the brain's own `CarryFrames`)
is younger than this. Nothing else changes: the approach, the poke and the lift are as they were (the sim's take grace refuses the stick for those
frames anyway, and the brain already holds it), and the check goes on the frame his carry is old enough. The contact law is not touched: a check that
lands is the same check.

**The human, 200 matches a level** (knocked down a match / as he won it, under 30 frames / under 15 frames / lying, % of play):

| level | as shipped | 8 frames | 15 frames | 30 frames |
|---|---|---|---|---|
| rookie | 3.33 / 1.75 / 1.43 / 3.42 % | | 2.02 / 0.62 / 0.24 / 2.17 % | 1.50 / 0.17 / 0.07 / 1.60 % |
| pro | **6.83 / 4.35 / 3.61 / 8.89 %** | 4.97 / 2.71 / 1.94 / 6.85 % | **3.17 / 0.91 / 0.19 / 4.11 %** | 2.57 / 0.27 / 0.10 / 3.41 % |
| all_star | 8.55 / 5.18 / 4.40 / 11.55 % | 6.46 / 3.12 / 2.21 / 9.04 % | 4.86 / 1.25 / 0.25 / 6.62 % | 3.79 / 0.29 / 0.14 / 5.56 % |

**The three cells** (600 each):

| cell | as shipped | 8 frames | 15 frames | 30 frames |
|---|---|---|---|---|
| novice at rookie | 85.3 | 88.0 (+2.7) | 89.0 (+3.7) | |
| regular at pro | 50.5 | 51.3 (+0.8) | **56.2 (+5.7)** | 59.3 (+8.8) |
| good at all_star | 29.8 | 33.7 (+3.9) | **38.7 (+8.9)** | |

**Against the contact lane's cap.** The two answer the same fault from two ends. At a quarter of a second the brain's rule takes more knockdowns away
than the cap (pro 3.17 a match against 3.7; lying 4.1 % against 4.5 %) and costs about twice as much of the ladder (pro +5.7 against +2.8, all_star +8.9
against +6.0): the brain's bot does not hit at all in the window, where the cap lets the hit land as a stumble that still loosens the puck. At 8 frames
the brain's rule is cheap at pro (+0.8) and does half the work (as he won it 4.35 -> 2.71). **They are alternatives, not a pair**: with the cap on, a
brain that also holds its check for 15 frames would pay again for what the cap already gives; the brain's rule is the choice if the operator wants the
hit itself gone rather than softened, and its window is one number.

Law 3's row is identical to the digit with the rule at 8, 15 and 30 frames, at all_star and at pro (section 4): the drill's carrier has carried for
seconds when the presser meets him.

## 3. Item 1: all_star's own dial

All_star reads 29.8 on this tree's 600 seeds (the bar: 26) and would read about 37 with the contact set on. Each candidate below is a tune key at its
neutral value; a level's switch is one line in `MakeAllStar()` (`SetEvolution6` leaves a level's own values alone).

**Screens first** (seeds 1-100, 151-250, 301-400: the baseline on those 300 reads 28.0), keys that existed before this lane:

| tune | good at all_star | |
|---|---|---|
| `opp.hk_spin_end_share=1` (the stick answers every spin's exposed end) | 28.7 | flat |
| `opp.hk_shot_max_angle_deg=60` (shot selection: no shot from the corner) | 31.0 | the wrong way: a held shot is a kept puck the human gets back |
| `mate.hk_mate_stick_frames=-1` (his mates lose their stick) | 26.3 | -1.7, and it is the human's own side made worse, not all_star made better |
| `opp.lift_cooldown_frames=0` / `opp.clear_lane=0` / `opp.shot_charge_frames=18` | 27.7 / 28.7 / 34.0 | flat, flat, the wrong way |
| `mate.hk_support_human=0` (his mates stop offering a lane) | **38.0** | **+10**: at all_star the lane-offering mate is not behind him when he loses the puck: the largest single brain number found tonight, on the wrong side of the ledger for this brief and the first thing to look at for the human's side |

**Candidates built** (600 seeds each; law 3's row in section 4):

| candidate | tune | good at all_star | verdict |
|---|---|---|---|
| A. The second man hunts | `opp.hk_second_man_hunts=1` | **27.7 (-2.1)** | a dial in the right direction, law 3 untouched: the first presser still teaches the stop at 8 m; the presser that takes over once he is beaten charges as all_star did before R-0058. He is knocked down 8.55 -> 9.27 times a match (200 matches). |
| B. A five's backs step up | `opp.hk_step_up_m=<N>;opp.hk_step_up_max_team=99` | 6 m: 29.8 (0.0); 10 m: 30.3 (+0.5); 16 m: **27.5 (-2.3)** | alone it is flat until the backs come out far; its worth is with A |
| **A and B together** | the two tunes | 10 m: **26.7 (-3.1)**; 16 m: **24.8 (-5.0)** | the pair is the dial: the presser is beaten, the next man charges and the last line comes out to meet the carrier instead of waiting in its crease |
| (measured and not a candidate) the cover reads the pass | `opp.hk_cover_best_pass=1` | 34.2 (+4.4) | the wrong way: covering the team-mate the sim's pass would pick leaves the nearest one open; with the second man: 32.8 |

**How sure these sizes are.** A cell of 600 matches has a standard error of about 1.9 points, and the candidates' sizes are of that order. By halves
of the seeds (the first 300 against the other 300): the second man alone +4.0 / -8.2, the step-up at 16 m alone +1.7 / -6.3, the pair at 10 m
-3.7 / -2.5. Only the pair points the same way in both halves. **At 2,000 seeds** (the same instrument; a cell's standard error is about
1 point):

| all_star, 2,000 seeds | win | against the baseline |
|---|---|---|
| as shipped | **28.8** (1.57 : 2.49) | |
| A alone: the second man hunts | 29.9 | +1.1: nothing (its -2.1 at 600 seeds was noise) |
| B alone: the backs step up at 16 m | 27.1 | -1.7: the step-up carries most of the pair |
| A and B, the backs at 10 m | 27.6 | -1.2 |
| **A and B, the backs at 16 m** | **26.6** (1.49 : 2.53) | **-2.2** |

The 600-seed cells overstated every one of them by a factor of two. What stands: **all_star's own dial from the brain is worth about 2 points**
(28.8 -> 26.6: the bar's 26 from today's number), it is the last line coming out to meet the carrier, with or without the second man, and it does not
touch law 3's row.

**What the two candidates are worth against the brief's numbers.** From today's 28.8 toward 26: the pair at 16 m reads 26.6 at 2,000 seeds. From
37 toward 26 (the contact set on): no brain dial found tonight is worth 11 points; the pair is worth about 2. The contact set's cost at all_star is the take shield's
(+6.0), and the brain's own lever on that is in section 2: the shield and the fresh-take rule do the same thing, and neither is free at all_star.

## 4. Law 3's row under every tune (`Bar.Bot.Hockey6.Law3Row`: the carry scene of `Jam.Core.Movement.StopAndGo`, 200 seeds a number)

The stop begun at 8 / 9 / 9.5 / 11 / 12 m, and the straight charge:

| level | as shipped | every tune of this document |
|---|---|---|
| all_star | 100 / 99.0 / 93.0 / 58.0 / 39.0; straight 0 | identical to the digit: the fresh take at 8, 15, 30; the second man; the cover; the backs stepping up at 6, 10 and 16 m; the second man with the step-up at 10 and at 16 m |
| pro | 100 / 99.5 / 97.0 / 86.0 / 61.5; straight 0 | identical to the digit: the fresh take at 8, 15, 30; the second man; the cover |

## 5. Tests, the gate, the neutral proof

`Jam.Bot.Hockey6.*` (pure worlds, both arms in one run):

| test | before | after |
|---|---|---|
| `FreshTake` | a bot that meets a carrier head-on from his first frame with the puck presses its check on frame 0 | on frame 14 with 15 frames, on frame 29 with 30 |
| `SecondMan` | the presser that takes over once the carrier is past the first closes under control (move length 0.34) | charges (1.07); the first presser plays the same in both arms |
| `CoverBestPass` | the cover heads for the lane of the carrier's nearest team-mate | for the lane of the pass (1.00) |
| `Neutral` | | as shipped every rule is at its neutral value at every level; `brain6=0` and each older switch (`brain5=0` ... `brain2=0`) put them there; the same match twice is the same match; as shipped = `brain6=0` |

- **The full `Jam` filter on the final build (build 3): 271 of 271** (`Game/logs/a181/ue-full-3.log`; 267 on main, 4 new; run without
  `JAM_PLAYTESTER_CHECK`; no "missing usage flag" line). Law 3 in it as on main (pro: the stop at 8 m 200 of 200, the straight charge 0; all_star 8 m
  100 %, straight 0). `Jam.Core.Determinism` seed 1 frame 600: hockey.regular `ef34dd88c7c62b47`, hockey.raw `79d43eb5ea69a468`.
- **The default match is the tree's as it stood, hash for hash**: build 3 against build 0 (no edit), the ladder's instrument, 100 seeds each: regular at
  pro, good at all_star, novice at rookie: 100 of 100 each (`Game/logs/a181/batchE.txt`).
- The measured tunes are the final build's: the fresh take at 15 frames at pro and the second man at all_star on build 3 against the cells measured on
  build 1: 100 of 100 hashes each.
- No field was added to skater or puck state. The brain's new state is three small arrays (the carrier a team presses, its first presser, who is a
  second man), reset with the brain.

## 6. What does not hold, and what is open

1. All_star's dial from the brain is about 2 points at 2,000 seeds (the backs stepping up at 16 m, with the second man), enough for today's 28.8
   and far from the 11 the contact set would ask of it. The second man alone is nothing.
2. Item 2's rule costs more of the ladder than the contact lane's cap for the same knockdowns; it is the cheaper one only at 8 frames, where it does
   half the work.
3. Not measured: either rule on the large rink or the six-skater cuts; the second man and the fresh take with the contact set on (the set is not on
   this tree); 2,000-seed confirmations.
4. One run fault, said plainly: the first full gate ran with `JAM_PLAYTESTER_CHECK=closing` in its environment and six pinned-hash tests failed on it;
   the gate was run again without it. The measuring environment must never reach the gate.
5. Not run: the Shipping compile, the acceptance run, any hand play. Nothing a player does changes as shipped.

## 7. How to repeat

```
export JAM_PLAYTESTER_CHECK=closing                                             # the ladder's instrument: cells and the human's bar only, NEVER the gate
CLS=good tools/perf/results/a181/idle.sh <label> all_star regular 600 "<tune>"  # one cell, one summary line (CLS=novice|regular|good)
JAM_A181_TUNES="as shipped:|fresh15:opp.hk_fresh_take_frames=15" JAM_A181_LEVELS=pro,all_star JAM_A181_MATCHES=200 \
  tools/perf/results/a181/tests.sh Bar.Bot.Hockey6.Human <tag>                  # grep HUMAN in Game/logs/a181/ue-<tag>.log
JAM_A181_TUNES="as shipped:|x:<tune>" JAM_A181_LEVELS=all_star,pro tools/perf/results/a181/tests.sh Bar.Bot.Hockey6.Law3Row <tag>   # grep LAW3ROW
env -u JAM_PLAYTESTER_CHECK tools/perf/results/a181/tests.sh Jam <tag>          # the gate's filter
```
