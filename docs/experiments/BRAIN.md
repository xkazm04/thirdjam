# The hockey brain (lane A94, R-0045 law 2, Fri 13 Nov)

The operator: "AI should know going all the way forward will lead to failure ... awareness of free space around, and free positioning of team players to decide when
to push solo, when to pass ... different during attack and defense." This page is what the brain now does, the fields that switch it, and what it measured.

## 1. The principles as implemented (`Game/Source/JamCore/AI/JamAICore.cpp`)

- **Space read** (`JamAI::ReadSpace`, pure, every frame, a few hundred flops on ten skaters). For the carrier: the first opponent on the lane ahead (a cone 2.5 m wide plus
  a fifth of the distance, out to 15 m), the seconds until it arrives (its distance less a metre over the closing speed, at least 1.5 m/s; 0 when any opponent is within
  2.2 m), and how many opponents stand on that lane. For each teammate: its distance to the nearest opponent, whether the pass lane is open (no body or blade within 1.5 m
  of the line), open (lane open, 3 m free, 4-30 m away) and better placed (open and `BetterByM` nearer the opposing net than the carrier).
- **Carry or pass** (in `FJamBrain::Attack`, for the carrier or the holder: see section 3). The shot decided as before comes first. Then the carrier passes when pressure
  arrives (inside `PressureSec`, or the first opponent on the lane is inside 5 m), when two opponents stand on the lane inside 12 m (a set defence: the wall), or, after
  `MinCarryFrames`, when a teammate is open and better placed. The receiver is the sim's `BestPass`. The human's teammates give it to the open human first (the sim's pass
  when the sim would pick the human, else A46's aimed tap from 8 m). With no pass on, a wall or a lane closed inside 4 m turns the carry 55 degrees away from the
  first opponent (never into the boards). It skates while the lane is open and nobody better placed is free.
- **Phase** (`ReadPossession`): the team in possession is the carrier's, else the last toucher's; none while frozen, in the faceoff countdown or before the first touch.
  One entry per frame in a 64-frame ring, so a level can read it late.
- **Attack shape** (`AssignJobs` / `JobSteer`, teams of five or more): the carrier, or the skater nearest a loose puck, has the puck; of the other forwards (the human
  takes a place in the order and is given nothing) the nearest is the **close support** (4 m behind, 6 m beside, on the middle's side, moving with the carrier: 7 m off),
  the next the **far side** (10 m ahead in the far lane; in their end the net front at the far post); the **backs hold the points** (7 m behind the puck, at most to
  their blue line, at least 8 m out of their own net, one each side). Nobody stands in a teammate's lane (a spot within 2 m of the line carrier -> teammate moves 3 m off
  it). A shape spot keeps its distance from the puck (the support 7 m, the others 9 m).
- **Defence shape**: one forward **presses**: the nearest goal-side one (its spot between the carrier and our net); with a fast carrier it holds a gap of a third of a
  second of the carrier's speed (1-3 m) and moves with it, shaded to the middle so the carrier is steered to the boards; a slow carrier, a loose puck, or a presser
  that is not goal-side plays the puck (`Attack`: the stick, the poke, the check as before). The next **covers the nearest pass** (on the lane to the carrier's
  nearest teammate, 70 % of the way: goal-side of the receiver; on a loose puck, goal-side of the opponent nearest our net); the rest take **the far side** between the
  puck and our net (a second one the high slot). The backs keep their posts (guard, slot, crease: A83, A85, R-0013). Nobody chases behind the play: the presser is
  chosen goal-side; the others' spots are all between the puck and our net.
- **Transitions**: for `TransitionFrames` (90) after possession changes, the presser is the nearest forward at all, not the nearest goal-side one (the counter-press),
  and the next covers the lane; on a win the nearest goes to the puck and the next is the close support; the rest skate to their shape.
- **Late and loose by level**: a bot reads the phase `LateFrames` late; in the share `1 - ShapeShare` of its two-second windows (a fixed coin per bot and window) it
  plays its job loosely: a spot off by up to 4 m and three quarters' pace.
- Unchanged: the stall rule, A79's hunt, the yield to the human's puck (a yielding bot's puck job goes back to the old support), the faceoff, the payoffs, the shot.
  A back of a team with the shapes keeps its post under the yield (the old rule made it a support that collapsed onto the others).

## 2. The fields (`FJamBotSkill::Hk`, `FJamHockeySense` in `JamAICore.h`)

All off (`FJamHockeySense()`) is the brain before A94, bit for bit: `-aitune "opp.hk_on=0;mate.hk_on=0"` reproduced the baseline's per-match rows on 20 of 20 seeds.

| field | meaning | rookie | pro | all_star | the human's mates (every level) |
|---|---|---|---|---|---|
| `bCarryOrPass`, `bAttackShape`, `bDefenceShape`, `bTransition` | the four principles | on | on | on | on |
| `PressureSec` | pass when the first opponent on the lane arrives inside | 0.6 s | 1.4 s | 1.4 s | 1.4 s |
| `BetterByM` | an open teammate this much nearer their net is better placed | 3 m | 3 m | 3 m | 3 m |
| `MinCarryFrames` | carry at least this long before passing to a better-placed teammate | 12 | 12 | 12 | 12 |
| `ShapeShare` | windows a bot keeps its job tight (the rest: loosely) | 0.6 | 0.85 | 1.0 | 0.85 |
| `LateFrames` | the phase read this many frames late | 24 | 8 | 0 | 8 |
| `TransitionFrames` | the transition rule's window | 90 | 90 | 90 | 90 |
| `bReadHumanPressure` | an opposing carrier passes away from the human's forecheck too | no | no | yes | (n/a) |

Set in `DifficultyProfile.cpp` (rookie, all_star; pro is `ProHockeySense()` via `ProOppSkill()`; the mates in `ApplyDifficultyProfile`). Tune keys (`-aitune`, `opp.` or
`mate.`): `hk_on`, `hk_carry_or_pass`, `hk_attack_shape`, `hk_defence_shape`, `hk_transition`, `hk_share`, `hk_late_frames`, `hk_pressure_s`, `hk_min_carry_frames`,
`hk_better_by_m`, `hk_read_human_pressure`. A86 and the final curve own these numbers.

**`PressureSec` is 1.4 s, not the brief's "a second"**, measured (30 seeds, pro): 1.0 s with better-placed 2 m and min carry 6 gave 11.7 passes a minute and the human's
touches -2.3; 1.4 s and 3 m gave 11.6 and -1.7. `bReadHumanPressure` off at pro and rookie keeps the human's forecheck as the old `ForecheckYieldFrames` rule left it
(without it the human's touches fell 2.8 a minute: carriers passed away from the human the moment it came).

## 3. The fact the brain lacked (`JamSimAdapter.cpp`)

`W.Carrier` is the last toucher with the puck within `shot.reach_m` (1.5 m). Outside its shooting range a bot carries the puck at the blade's full reach (A51, 1.6 m +),
so the brain did not see its own carrier there and the carry-or-pass read never ran (A85's note on `W.Carrier`). New in `FJamAIWorld`: `Holder` (`UJamSim::CarrierIndex`,
the sim's own carrier reach) and `HolderBestPass`. Only the hockey sense reads them (`CarrierOrHolder()`); every older decision still reads `W.Carrier`. This one fact took
bot passes from 9.0 to 12.7 a minute (30 seeds).

## 4. The shape, before and after (`tools/playtest/shape.mjs`, the playtester through the tournament commandlet with `-trace=1`)

Pro, the regular playtester, hockey.regular (5 v 5), seeds 1-60. Before = main 8f6b157's brain (and the same numbers with `hk_on=0`).

| | spread (m between teammates) | pile (frames with 3+ of a team within 6 m of the puck) | bot passes / min | bot carries / match | carries ending pass / shot / lost | goals / match | human touches / min | faceoffs w/o human touch |
|---|---|---|---|---|---|---|---|---|
| before | 18.26 | 17.0 % | 5.65 | 40.8 | 26.8 / 2.9 / 70.2 % | 3.90 | 25.52 | 21 / 458 |
| after | 18.13 | **12.0 %** | **11.70** | 30.9 | 43.6 / 3.3 / 53.0 % | 3.72 | 23.57 | 3 / 381 |

The brief's bars: bot passes at least double: **met** (x2.07). Goals within one: **met** (-0.18). The pile halved: **not met** (-29 %; 8.5 % was the bar). Where the pile
is (seeds 3-5, by the puck's place): own end within 15 m of the net 8.0 -> 4.5 %, the neutral zone 2.7 -> 5.7 %, elsewhere 5.1 -> 3.0 %. The neutral-zone piles that
remain are passes in flight past positioned skaters and the faceoff line-ups (the sim's spots, until the shapes start at the first touch); the own-end piles are the
presser with the guard and the slot backs collapsing on a puck in the slot. Tried and dropped: the loose puck's owner chosen by where the puck will be in 0.4 s with the
shape round that skater (pile 12.6 %, the human's touches -3.2: the bots beat the human to loose pucks).

The other levels (20 seeds, before / after): rookie with the novice: pile 17.2 -> 11.8 %, passes 3.58 -> 9.51, goals 3.55 -> 3.20, novice touches 15.61 -> 13.47 / min.
all_star with the good player: pile 18.7 -> 12.7 %, passes 8.55 -> 12.38, goals 3.95 -> 4.15, touches 23.30 -> 24.66.

Carries: a carry is 20+ frames carried by a bot; a pass is the last toucher going from a bot to a teammate with the puck 4 m or more from where the passer last had it.

## 5. The human (A53's soak, 600 matches, the regular class at pro: seeds 1-600, two thirds hockey.regular, a third hockey.boost)

Touches a minute are the mean of the matches' `touches_per_min`; faceoffs without a human touch are summed over the matches' faceoffs.

| | n | win % | GF / GA | touches / min | faceoffs w/o human touch | passes received / match |
|---|---|---|---|---|---|---|
| before, hockey.regular | 400 | 38.5 | 1.73 / 2.25 | 25.39 | 160 / 3081 = 5.19 % | 3.74 |
| after, hockey.regular | 400 | **79.5** | 2.57 / 1.15 | 23.58 (-1.81) | 38 / 2665 = 1.43 % | 9.34 |
| before, all 600 | 600 | 51.0 | 2.02 / 1.99 | 24.91 | 230 / 4354 = 5.28 % | 2.90 |
| after, all 600 | 600 | 74.7 | 2.50 / 1.35 | 23.82 (-1.09) | 84 / 3900 = 2.15 % | 6.63 |

The bars: touches within 2 of today's: **met** (-1.81 on the regular rink). Faceoffs without a human touch within 2 points: they moved **down** 3.8 points (better for
the human; outside "within 2" only in that direction). The teammates still pass to the open human: received passes 3.74 -> 9.34 a match (the mates give it to the open
human first). 0 crashes.

**The win share moves: the regular playtester at pro wins 79.5 % (38.5 before; R-0029's target 50).** The opponents' principles take away the old swarm, which won
pucks off the human. 100-match arms (run.mjs, regular rink, pro, the regular class; one key changed from the shipped sense):

| arm | win % | GF / GA | shots by the AI / match |
|---|---|---|---|
| shipped (everything on) | 79.5 (soak) | 2.57 / 1.15 | ~7 |
| opponents' sense off (`opp.hk_on=0`) | 41 | 1.7 / 2.3 | 14.8 |
| the mates' sense off (`mate.hk_on=0`) | 91 | 2.8 / 0.8 | 7.2 |
| opponents' carry-or-pass off | 58 | 2.2 / 1.9 | 11.8 |
| opponents' attack shape off | 61 | 2.2 / 1.5 | 10.4 |
| opponents' attack and defence shapes off | 64 | 2.1 / 1.5 | 12.1 |
| opponents' defence shape off | 71 | 2.5 / 1.5 | 8.9 |
| opponents' transition off | 76 | 2.6 / 1.3 | 7.8 |
| opponents never pass to the better placed (`opp.hk_better_by_m=50`) | 84 | 2.7 / 1.2 | 9.2 |
| opponents read the human's pressure (`opp.hk_read_human_pressure=1`) | 79 | 2.5 / 1.1 | 6.8 |
| opponents tight and at once (`opp.hk_share=1;opp.hk_late_frames=0`) | 83 | 2.6 / 1.2 | 7.3 |

Each principle costs the opponents something against the human (their shots halve); no single key gives it back. This is the final curve's (A86, R-0045 "the levels
are tuned once more after the three land"): the brain's numbers above are the levers, and the old speed / red_skill levers stand as before.

## 6. Tests (`Game/Source/JamCore/Tests/HockeyBrainTests.cpp`)

- `Jam.Bot.CarrierDoesNotChargeTheWall`: a carrier at 6 m/s, two opponents on its lane 9-11 m ahead, an open teammate 11 m to the side; the real sim, pro, the brain on
  every skater; 10 seeds (the scene varies by seed). **10 of 10 pass inside 60 frames; the brain before A94: 0 of 10.**
- `Jam.Bot.Hockey.SpaceRead`: the read's numbers on a built world (two on the lane, 1.19 s to arrive, the open / blocked / marked teammates, open ice).
- `Jam.Bot.Hockey.Shapes`: attack: one close support, one far side, both backs at the points, only the carrier on the puck, a teammate 2.8 m off skates away; defence:
  one presser, one cover; the sense off: no jobs.

## 7. What a viewer will notice

The puck moves. A bot that picks the puck up no longer skates head-down into two defenders: it looks, and when the ice closes it moves the puck to a teammate who is
already standing in space, a few metres beside and behind it or wide on the far side; the two backs sit up at the blue line instead of hanging off their own net.
Without the puck the other team does not swarm: one skater steps up in front of the carrier and backs off as it comes, pushing it towards the boards, a second stands in
the passing lane, the rest are between the puck and the net. Bots pass about twice as often, the scrums around a loose puck are rarer (not gone: a puck in the slot
still draws three), and a rush up the middle into a set defence ends in a pass or a cut to the side rather than a lost puck.
