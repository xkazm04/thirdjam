# Contact 9: he keeps his feet as he wins the puck (lane A177, track F1, evolution 8; 2026-11-14)

On main `3fa9c2e6` (package H30, the one the operator plays at 10:00). Three things: **a hit inside a carry's first frames is capped at a stumble** (the
second thing he will feel: "I touched the puck and I was on the ice", REVIEW-3 item 2), in three forms measured against each other; **the push-out with
the brain lane's mates' keep-out**, the pair nobody had measured together; and **the whole contact set as one number** for the curve lane. Every rule is
behind a key at its neutral value: the default match is hash-identical to `3fa9c2e6`. Every cell is measured with the ladder's instrument
(`JAM_PLAYTESTER_CHECK=closing`), seeds 1 to 600, paired. Nothing lands before the operator has played: this is a measured choice for him. Status with
timestamps: `status/A177.md`. Raw: `Game/logs/a177*`, `tools/perf/results/a177/raw/` (not landed).

Code: `Game/Source/JamCore/Private/JamSim.cpp`, `Public/JamSim.h` (the take shield's clock beside the take grace's, the cap in `ResolveContact`; one state
field, two read-outs), `Public/JamCartridge.h`, `Private/JamCartridge.cpp` (three keys; two names in the measurement seam), `Public/JamBaseline.h`,
`AI/JamPlaytester.cpp/.h` (eight read-outs on the playtest line), the test in `JamCore/Private/Tests/JamContactTests.cpp`,
`JamNet/Private/Tests/JamNetGuestTests.cpp`, `schema/` (three rows in each file), the two generated copies, drivers in `tools/perf/results/a177/`.

## 0. The choice

**For the human, novice at rookie / regular at pro / good at all_star, 600 matches a cell, the ladder's instrument:**

| | as shipped | the cap alone (15 frames, scaled to 45) | the cap with the push-out (0.6) and the mates' keep-out |
|---|---|---|---|
| he is knocked down, a match | 3.21 / **6.86** / 8.48 | 2.01 / **3.67** / 4.89 | 2.02 / **3.50** / 4.61 |
| ... of those as he won the puck (his for under half a second) | 1.30 / **3.71** / 4.50 | 0.21 / **0.48** / 0.61 | 0.22 / **0.50** / 0.57 |
| share of play he lies on the ice | 3.29 / **9.08** / 11.49 % | 1.99 / **4.52** / 6.69 % | 2.04 / **4.48** / 6.47 % |
| share of play he stumbles (on his feet, no control) | 0.03 / 0.13 / 0.13 % | 0.41 / 1.64 / 1.91 % | 0.42 / 1.71 / 1.96 % |
| every skater's frames on the ice, a match | 749 / 1,413 / 1,735 | 347 / 687 / 937 | 356 / 664 / 911 |
| a skater standing at a downed body (a root within 0.9 m for more than 6 frames), spells in 200 matches | 667 / **1,468** / 1,202 | 363 / 702 / 624 | **1 / 2 / 0** |
| the three cells (600 paired seeds) | 86.5 / 53.2 / 30.0 | 86.2 / 54.0 / 35.0 = **-0.3 / +0.8 / +5.0** | 88.7 / 59.0 / 38.5 = **+2.2 / +5.8 / +8.5** |
| pro and all_star at 2,000 paired seeds | 53.0, 29.7 | 55.8 (**+2.8**), 35.8 (**+6.0**) | 56.5 (**+3.5**), 36.7 (**+7.0**) |

In words: with the cap he is on the ice half as often and half as long at every level, the fall "as I touched it" is all but gone (one in eight
is left), and he stumbles on his feet instead, four and a half times a pro match. The push-out and the keep-out add nothing to that for him; what
they add is that nobody stands on a man who is down. **The price is the ladder**: the cap is worth about 3 points to him at pro and 6 at all_star at
2,000 seeds (the 600-seed +0.8 at pro was a low third: +0.8 / +6.7 / +1.4 by seed range), and the three together about 3.5 at pro and 7 at all_star. Nothing here is free at
all_star; at pro the curve lane's dial can pay (section 3).

## 0b. Everything measured, in one table

The tree with every key at its default: **86.5 / 53.2 / 30.0** (novice at rookie / regular at pro / good at all_star).

| What | Keys (under `game.contact` unless said) | Ships | Recommended | Cost: rookie / pro / all_star |
|---|---|---|---|---|
| He keeps his feet as he wins the puck, and a bigger hit still costs more | `take_shield_frames`, `take_shield_stagger_max` | 0, 0 (neutral) | **15 and 45** | **-0.3 / +0.8 / +5.0** (86.2 / 54.0 / 35.0); at 2,000 seeds pro **+2.8**, all_star **+6.0** |
| The same with the flat cap (the review's suggestion as written) | `take_shield_frames` | | 15 | **+1.3 / +7.0 / +4.0** (87.8 / 60.2 / 34.0) |
| The same for the human's side only | `take_shield_human_side` | false (neutral) | not recommended | pro **+8.0** flat at 15 frames (every skater: +7.0), +7.0 at 30 (every skater: +3.7) |
| The push-out with the mates' keep-out | `downed_radius_m` 0.6 with `mate.hk_avoid_downed_m=1.3;mate.hk_avoid_downed_out=0.7` | 0 and off | as written | **-2.3 / -1.2 / +2.8** (84.2 / 52.0 / 32.8) |
| The whole contact set: push-out 0.6, cooldown 60 with the restart, lunge 12 / 1.5, slam 30, the take shield 15 / 45 | | | | **+1.0 / +2.8 / +6.7** (87.5 / 56.0 / 36.7); at 2,000 seeds pro **+3.4**, all_star **+7.3** |

## 1. A hit inside a carry's first frames is capped at a stumble (item 1)

**The fault** (REVIEW-3 item 2, MOVEMENT-7 section 4): two skaters race head-on for a loose puck, he touches it first, that makes him the carrier, and
the pro's pressed check lands a quarter of a second later at 16 m/s of closing, for 131 frames. Read here on the sim's own possession flag, 600 pro matches
with the ladder's instrument: he is knocked down **6.86 times a match, 3.71 of them with the puck his for under half a second** (3.19 inside 15 frames), and
lies **9.08 % of play** (3.29 % at rookie, 11.49 % at all_star). (MOVEMENT-7's trace takes its rows in another order and reads the class at 4.39 a match;
the bar "4.4 to 2.2" is read here as "halved".)

**The rule** (`game.contact.take_shield_frames`, 0..60, default 0 = neutral). The clock is the take grace's own (`UJamSim::TakeOwner`, lane A121): a skater
that WINS the puck (it comes to it from somebody else; a placed puck is not won) holds `TakeShieldLeft` = the key from the end of that frame; it counts
down every frame and ends the moment the puck is no longer its. A hit that would fell a skater inside it (`ResolveContact`, outcome DOWN; an armed hip
check keeps its rule) is a **stumble**: it keeps its feet, loses half its speed, its control for the stumble **and the puck**; the attacker pays as for
any stumble. Two more keys choose the form:
- `take_shield_human_side` (boolean, default false): only for skaters of a side with a human on it.
- `take_shield_stagger_max` (0..90, default 0): **the cap that scales.** The capped hit's stumble lasts from `stagger_frames` (16) at `down_ratio` to this
  many frames at `down_full_ratio`: the very line the frames down follow. At 0 the cap is flat: every capped hit is 16 frames.
State: `FJamSkaterState::TakeShieldLeft` (hashed behind 0); read-outs `bContactShield`, `ContactShielded`.

Staged (`Jam.Core.Contact.TakeShield`): a standing skater that won the puck four frames earlier, hit head-on at 9 m/s: down 67 frames at 0; at 15 a
stumble of 16 frames, the same ratio (1.166), the puck loose; the same hit 18 frames after the take fells him; a placed puck is no take; with
`take_shield_human_side` a bot of the other side is felled; scaled to 45: 18 frames at ratio 1.17, 26 at ratio 1.72.

### What the rule does to law 1's table, said plainly

Law 1: a hit's strength is the closing speed along the line between the bodies, and by the ratio of impulse to stability the victim bumps, stumbles
or goes down for 60 to 150 frames. A carrier at 8 m/s who has just won the puck (stability 430; the pressed check's 1.5 m/s is in):

| closing speed | ratio | today | take shield, flat | take shield, scaled to 45 |
|---|---|---|---|---|
| 2 m/s | 0.65 | stumble 16 frames | the same | the same |
| 4 m/s | 1.02 | **down 61 frames** | stumble 16 | stumble 16 |
| 8 m/s | 1.77 | **down 95** | stumble 16 | stumble 27 |
| 12 m/s | 2.51 | **down 128** | stumble 16 | stumble 38 |
| 16 m/s (the class's average) | 3.26 | **down 150** | stumble 16 | stumble 45 |

- **Under the flat cap law 1 is switched off for fifteen frames from 4 m/s up**: a nudge at 4 m/s and the biggest hit in the game at 16 give the same
  16 frames. "A hit obeys momentum" is false of that window; the hit he took for winning a race costs him a quarter of a second and the puck.
- **Under the cap that scales, law 1's order is kept and its top is cut off**: every row still costs more than the one above it, by the same line the
  law uses, but the cost is counted in frames without control on his feet (16 to 45) and not in frames on the ice (60 to 150). What is no longer true:
  "head-on at full speed is down". What stays true: "head-on at full speed is the biggest hit in the game", and he loses the puck to it.
- Below a ratio of 1 nothing changes in either form, and after the fifteen frames nothing changes at all: law 3's case (the carrier who had the puck for
  a second and a half and did not brake) is untouched: section 4.

So the answer to "does a cap that scales with closing speed keep it truer than a flat one": **yes, and it is also the cheaper one**: 3.0 points cheaper
at pro on the same 2,000 seeds (standard error 1.5).

### At pro, six arms, 600 paired seeds each (a paired difference carries about 2.7 points)

| arm | win | delta | knocked down a match | as he won it | lying, % of play | stumbling, % of play | hits capped a match: on him / on the others |
|---|---|---|---|---|---|---|---|
| off | 53.2 | | 6.86 | 3.71 | 9.08 | 0.13 | |
| 15 frames, every skater, flat | 60.2 | **+7.0** | 3.90 | 0.51 | 4.83 | 0.78 | 3.50 / 2.50 |
| 15, the human's side only, flat | 61.2 | **+8.0** | 3.80 | 0.48 | 4.74 | 0.73 | 3.45 / 1.82 (his mates) |
| **15, every skater, scaled to 45** | **54.0** | **+0.8** | **3.67** | **0.48** | **4.52** | 1.64 | 3.47 / 2.49 |
| 15, every skater, scaled to 30 | 55.5 | **+2.3** | 3.67 | 0.54 | 4.47 | 1.20 | 3.62 / 2.46 |
| 30, every skater, flat | 56.8 | **+3.7** | 3.42 | 0.00 | 4.27 | 0.90 | 4.04 / 3.19 |
| 30, the human's side only, flat | 60.2 | **+7.0** | 3.49 | 0.00 | 4.28 | 0.83 | 4.04 / 2.23 |
| 30, every skater, scaled to 45 | 55.7 | **+2.5** | 3.03 | 0.00 | 3.86 | 1.90 | 4.09 / 3.03 |

**The bars.** "As he won it" from 4.4 knockdowns a pro match to 2.2 or under: **met by every arm** (3.71 to 0.5 at 15 frames, to none at 30). His share of
play lying down under 6.5 % at pro: **met by every arm** (4.5 to 4.8 % at 15 frames; 3.9 to 4.3 % at 30). The arms differ in what they hand him: the flat
cap is worth 4 to 8 points at pro, because the hit that used to take 131 frames now takes 16; the cap that scales takes up to 45 for the same hit and is
worth 0.8 on these 600 seeds and 2.8 at 2,000 (below; the flat cap 5.8 there). For every skater against the human's side only: the side-only arm is the dearer one (it keeps the bots falling to
the same hit he now survives), and it is a rule a player can see is not the same for both sides.

**Recommended: 15 frames, every skater, scaled to 45** (`take_shield_frames` 15, `take_shield_stagger_max` 45).

### The three cells, and 2,000 seeds where it matters

| arm against off (86.5 / 53.2 / 30.0) | cells | delta | he is knocked down a match | as he won it | lying, % of play |
|---|---|---|---|---|---|
| off | 86.5 / 53.2 / 30.0 | | 3.21 / 6.86 / 8.48 | 1.30 / 3.71 / 4.50 | 3.29 / 9.08 / 11.49 |
| **15 frames, scaled to 45** | 86.2 / 54.0 / 35.0 | **-0.3 / +0.8 / +5.0** | 2.01 / 3.67 / 4.89 | 0.21 / 0.48 / 0.61 | **1.99 / 4.52 / 6.69** |
| 15 frames, flat | 87.8 / 60.2 / 34.0 | **+1.3 / +7.0 / +4.0** | 2.01 / 3.90 / 4.84 | 0.24 / 0.51 / 0.58 | 2.06 / 4.83 / 6.69 |

At 2,000 paired seeds, the recommended form: pro 53.0 against 55.8 = **+2.8** (standard error 1.5; by seed range +0.8 / +6.7 / +1.4); he is knocked down 6.95 -> 3.69 a match, as he won it 3.87 -> 0.49, lying 9.27 -> **4.53 %**; all_star 29.7 against 35.8 = **+6.0** (standard error 1.5; +5.0 / +8.3 / +5.1); lying 11.5 -> **6.62 %**. The flat form at pro on the same 2,000 seeds: 58.8 = **+5.8** (+7.0 / +4.7 / +5.6); paired against the scaled form it is **3.0 points dearer** (standard error 1.5).

**The cost, for the curve**: the recommended form is worth **about 3 points to him at pro and 6 at all_star** (2,000 seeds; the 600-seed cells read -0.3 / +0.8 / +5.0, and pro's +0.8 was a low third). The flat form is worth about 6 at pro. Either lands only with the curve: at pro about two metres of lane A170's hunt dial pay for the scaled form; all_star needs a number of its own, and six points is more than any single contact rule of this track has cost before.

## 2. The push-out with the mates' keep-out (item 2)

`contact.downed_radius_m` 0.6 (lane A157, its own circle hard since lane A165) with the brain lane's keep-out for the human's mates
(`-aitune=mate.hk_avoid_downed_m=1.3;mate.hk_avoid_downed_out=0.7`, BRAIN-5), 600 paired seeds a cell:

| | rookie | pro | all_star |
|---|---|---|---|
| the three cells against off | 84.2 (**-2.3**) | 52.0 (**-1.2**) | 32.8 (**+2.8**) |
| spells of a root within 0.9 m of a downed skater's for more than 6 frames, in 200 matches (off: 763 / 1,554 / 1,235 with the shipped press) | **2** | **3** | **7** |
| a root within 0.6 m for more than 6 frames (B's bar), in 600 matches | 0, not one frame | 0, not one frame | 0, not one frame |
| frames a match in which the push-out turned a course (the push-out alone: 41 / 95 / 89) | 38 | 84 | 68 |

**Verdict**: **the bar is met with room: 3 spells in 200 pro matches against the bar of 50** (the brain's steering alone could not reach it: about 300 are the scripted human's own, BRAIN-5; the push-out takes those too). The mates' keep-out adds little that the push-out has not already done to the count (the push-out alone: 0 / 4 / 4), and what it changes is how it looks: the mates go round on their own and the rule turns them on 84 frames a pro match instead of 95. The cells: -2.3 / -1.2 / +2.8 for the pair against -2.2 / +0.3 / +5.2 for the push-out alone at 600 seeds: inside the noise of one arm (2.7 points), with pro and all_star each a point or two lower with the mates' keep-out on. The two can go on together.

## 3. The whole contact set as one number (item 3)

Push-out 0.6, cooldown 60 with the restart, lunge 12 frames / 1.5 m/s, slam 30, the take shield 15 scaled to 45 (the mates' keep-out is the brain's and
is not in it):

| | rookie | pro | all_star |
|---|---|---|---|
| the three cells against off, 600 seeds | 87.5 (**+1.0**) | 56.0 (**+2.8**) | 36.7 (**+6.7**) |
| at 2,000 paired seeds | | 53.0 against 56.4 = **+3.4** (standard error 1.6; by seed range +2.8 / +4.5 / +2.9) | 29.7 against 37.0 = **+7.3** (standard error 1.5; +6.7 / +8.7 / +6.6) |
| he is knocked down a match (off 3.21 / 6.86 / 8.48) | 2.11 | 3.63 | 4.45 |
| lying, % of play (off 3.29 / 9.08 / 11.49) | 1.98 | 4.51 | 6.16 |

**What the curve lane's dial pays, and what is left.** Lane A170's dial is pro's alone: the presser on its job and `opp.hunt_radius_m` 8 to 11 takes pro
from 50.5 to 48.0 at 600 seeds (52.4 to 48.0 at 2,000), about 1.5 to 2 points a metre, with law 3's drill unchanged to the digit. **At pro the set costs 3.4 points at 2,000 seeds, which is about two metres of that dial**: where the dial alone reaches 48.0 at 11 m, with the set on the same 48 lies between 12 and 13 m (12 would read about 49, 13 about 46.5, if the two add up: not measured together, the curve lane's to confirm on its own seeds). **What the set leaves for all_star: all of it: 37.0 against 29.7 at 2,000 seeds, 7.3 points over** (6.0 of them are the take shield's; the push-out's 2.7 and the lunge's 2.5 measured alone in CONTACT-8 do not add on top of it). The dial does not reach all_star, and nothing measured tonight pays there. With the mates' keep-out as well (`setm`, 600 seeds): +0.7 / +1.1 / +8.0.

## 4. The proof

- **The default match is hash-identical to `3fa9c2e6`.** `off2` (the final build, every key at its default, the ladder's instrument) against `base`
  (the tree as it arrived): **600 of 600 matches hash-identical in each of the three cells.** (And that baseline is itself identical, 600 of 600 a cell,
  to this lane's evolution-7 tree: the code that came in with main is neutral for the default match.)
- **The gate**: the full `Jam` filter on the final tree: **268 PASS, 0 FAIL (267 on `3fa9c2e6`; the new test is the difference)**. `Jam.Net.HashCoverage` green: `TakeShieldLeft` hashed behind 0 (tag -1771),
  `bContactShield` and `ContactShielded` listed. Of the 230 lines that carry a hash, 228 are identical to the baseline gate's character for character and the other two (`Determinism`, a timing in their text) carry the same hashes.
- **Law 3's row with the rule ON**: `Jam.Core.Movement.StopAndGo` run with the take shield at 15 / 45 on every cartridge it loads (the measurement seam):
  PASS, and its 11 `LAW3` lines are identical, character for character, to the baseline gate's (pro: the stop begun at 8 m past 200 of 200; all_star 200 of 200; the straight charge 0): by the time a carry reaches law 3's case the shield has long run out.
- **Both validators**: `node --test schema/**/*.test.mjs` 99 pass, 0 fail; the cartridge tests and `Jam.Designer.PromptParity` green with both embedded
  copies regenerated.
- **Neutral values**: one line in `JamBaseline::MakeNeutral` (0, false, 0). No pinned hash of a shipped cut was added.
- The Shipping compile of the Game target on the final tree: PASS (89 s, 98 actions; `Game/logs/build-ThirdJam-Shipping-20261114-074252.log`). Not run: the acceptance run (nothing a player can do changes at the defaults).

## 5. Findings outside the family

1. The brain does not know the shield: a pro presser still charges the puck's first toucher (MOVEMENT-8's finding for track Q2) and now gets a stumble
   and a loose puck for it, and under the flat cap its own recoil. Whether the presser should arrive a quarter of a second later and hit a man who has the
   puck is the brain track's question; the scaled cap makes the charge still worth making.
2. The stumble has no line on the HUD's contact line for the victim ("I kept my feet"): with the shield on he stumbles 4.5 times a match where he
   fell 3.2 times; the figure's stumble take is the only cue. A line such as `HELD ON` (from `ContactShielded` rising on the human) is the screen track's.
3. `tools/barks/barks.test.mjs:64` still fails in this worktree (as in CONTACT-7 and 8): not this lane's files.

## 6. The next three evolutions for the track

1. **The presser that hits a man who has the puck** (with the brain track Q2). The pro's presser charges the puck's first toucher and, with the shield on, gets a stumble for it 3.5 times a match. Arriving a quarter of a second later (or playing the puck off the stumble) is the brain's half of this rule. Proving numbers: hits the shield caps on the human from 3.5 to under 1.5 a pro match, his knockdowns no higher than 3.7, the three cells within 2 of the cap alone.
2. **All_star's own number for the contact set** (with the curve track C0): the set leaves all_star 7.3 over and the hunt dial is pro's. One all_star profile number (not the stop's distance) measured at 2,000 paired seeds with the set on. Proving numbers: all_star within 3 of its target with the set on, his share of play lying down at all_star under 8 % (6.1 with the set, 11.5 today), law 3's eleven lines unchanged.
3. **The stumble that reads** (sim side, headless; the HUD line is the screen track's): a log line and a read-out for every capped hit, the figure's stumble take told apart at 16 and at 45 frames, the hit-stop scaled to the stumble. Proving numbers: in one scripted pro match every capped hit on the human has its log line (count = `shield_human`), and a 45-frame stumble holds at least 2 frames longer than a 16-frame one.
