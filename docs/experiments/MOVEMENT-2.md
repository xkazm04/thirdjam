# Movement 2: the carrier keeps the puck, and the stop-and-go is measured (lane A102, track F2, evolution 1, 2026-11-13)

`docs/VISION.md` law 3: the carrier wins by changing speed and direction with a stick the mouse controls. This lane built the scene that measures it (a carry
at a pro defender, headless and as drill 21), added the two stick rules the measurement and the design eye's review (R-0048) asked for, and found where a
stop has to begin. Status with timestamps: `status/A102.md`. Raw sweeps: `Game/logs/a102*` (not landed).

Code: `Game/Source/JamCore/Private/JamSim.cpp` (`IsPokeShielded`, new `IsBodyShielded`, new `RidingCarrier`, one guard in `StepPuck`'s skater loop, one factor
in `StepStick`), `Public/JamSim.h`, `Public/JamCartridge.h` + `Private/JamCartridge.cpp` (two keys), `Public/JamCarryDrill.h` (new: the scene),
`Private/Tests/JamCarrierSkillTests.cpp` (new), `JamGame/Private/HokejGameMode.cpp` (drill 21), `schema/`.

## 1. The two rules

1. **Nobody takes a carried puck without a press** (`game.stick.passive_take`, default **false**; R-0048). Lane A100 measured on 924 matches that a human's
   carry lasted a median 0.67 to 0.87 s and the puck was on nobody's blade 60 % of play: in `StepPuck` an aimed blade within 0.6 m deflected a carried puck as
   if it were a shot (the carrier above 6 m/s), and any body's touch made that skater the last toucher. Now a puck that **rides** with its carrier
   (`UJamSim::RidingCarrier`: the carry assist's own test: carried, not just shot or passed, moving with the body or settling on the blade) is skipped by every
   other skater's blade and body. It leaves by a shot, a pass, a poke, a lift, a check or stumble that loosens it (a carrier that cannot act carries nothing),
   or the carrier's own release: R-0021's sentence, now the rule. A shot is not a riding puck: a body two metres ahead still blocks it. **A teammate never
   takes a carried puck by touch, under either value of the key** (a pass is a pass); the old teammate touch survives only as the code switch
   `FJamStickConfig::bTeammateTouchTakes` for the tests that replay older commits. Cuts without a carry assist (`puck.carry_assist` 0) dribble with the body
   and are not touched by the rule.
2. **Puck protection with the body, no button** (`game.stick.body_shield`, default **true**). A defender's poke or lift does not take the puck while the
   carrier's body is between (`UJamSim::IsBodyShielded`, called from `IsPokeShielded`, so the lift and the cross-check's shove follow it as they follow the
   spin): the defender's body within 2 m of the carrier's; the puck on the far side (within 60 degrees of the line defender -> carrier carried on); and the
   carrier turned: the defender more than 60 degrees off the carrier's facing. A carrier that faces the defender shields nothing, wherever it holds the puck:
   the poke, the lift and the cross-check from the front stand (law 1). A body check is never shielded. No state: nothing new in the hash.

| Key | Default | Neutral (the game before the lane) | What |
|---|---|---|---|
| `game.stick.passive_take` | false | true | true = an unpressed opposing blade or a body's touch takes a carried puck |
| `game.stick.body_shield` | true | false | true = a poke or a lift fails while the carrier's body is between the defender and the puck |

Neutral block: `"stick": { "passive_take": true, "body_shield": false }`. `Jam.Core.MovementNeutral` and the contact lane's neutral test carry it (plus the
teammate switch) and still replay their pinned matches hash for hash.

## 2. The scene (`FJamCarryDrill`, one piece of code, two uses)

The human carries along the attack at 9 m/s (+-0.5). The opposing chaser stands 12 m ahead (+-1), up to 1.5 m off the line, and **the real brain plays it**
from the moment the puck is seated; everyone else is parked in a corner. The scripted human uses the move keys and the mouse-style stick aim only:

- **stop-and-go**: at a gap of `BrakeAtM` (+-0.75 m) it pulls back and to the open side (the reverse brake) until it is slower than 2 m/s, waits 0 to 4
  frames, cuts 60 degrees (+-10) off the line (the first-step burst) and goes straight again a metre past the defender; from the stop on, the cursor is on the
  far side of the body from the defender (at most 15 degrees a frame).
- **straight charge**: W held, the blade ahead.

The seed gives the spread (a person is not a metronome). **Past** = 2 m beyond the defender along the attack with the puck, and still with it half a second
later; otherwise the carry ends by how the puck was lost (poke, lift, down, stumble, touch) or a time-out.

- Headless: `Jam.Core.Movement.StopAndGo` (the gate, 200 seeds an arm) and `Bar.Movement.StopAndGoSweep` (outside the gate; rows from
  `Game/logs/a102/sweep.txt` when it exists, "name | key=value, ...", so one build serves every sweep).
- In a package: **`jam.ctrl.Drill 21`**, 20 rounds (the round number is the seed), a `JAMDRILL carry result` line each and a `JAMDRILL carry summary`;
  `jam.ctrl.CarryScript` 1 = the scripted stop-and-go, 0 = the straight charge, 2 = your own keys and mouse (the drill stages and scores).

## 3. What the scene showed

**The pro defender does not stand.** The brain sends it at the carrier (1.2 m/s after 18 frames, 6.5 m/s when the gap is 5 m, 11 m/s at contact). The brief's
script (the stop begins at 5 m) is run over in the middle of its brake: the gap closes in 16 frames, the stop from 9 m/s takes 21. So the number that matters
is where the stop begins (pro, 200 seeds, this tree):

| the stop begins at | 5 m | 6 m | 7 m | 8 m | 9 m | 9.5 m | 10 m | 11 m |
|---|---|---|---|---|---|---|---|---|
| past, this tree | 0 | 2.5 | 1 | 25.5 | 85.5 | **100** | 100 | 91.5 |
| past, the game before the lane | 0 | | | 0 | 38.5 | 73 | 89 | 66.5 |

| level | clean stop-and-go | straight charge | notes |
|---|---|---|---|
| rookie | 100 (9.5 m), 60.5 (5 m) | 53.5 (0 before the lane) | rookie bots neither poke nor lift: only a hit takes a carried puck now |
| pro | **100** (9.5 m) | **0** (200 lifted; before: 52 lifted, 148 taken by a touch) | the brief asked two in three against one in five |
| all_star | 5.5 (9.5 m), 57.5 (11 m), 91 (12 m; 48 before the lane) | 0 | all_star commits at once and at 1.4 speed: the stop has to begin as it moves |

What makes the clean stop-and-go work, measured at 9.5 m (pro): the body shield 100 against 73 (54 lifted without it); the mouse 100 against 46.5 with the
blade left ahead (the shield is earned by the turn and the cursor: with the blade ahead the key changes nothing); the reverse brake 100 against 37 without it;
the burst 100 against 94.5 without it; a person twice as loose in timing (spread 2) 81.5. A bare cut without the stop works in a narrower band (85.5 at
8 m, 55.5 at 7 m, 76 at 9 m).

**The keys the brief named, at the late edge (the stop begins at 8 m: 25.5):**

| key | values -> past at 8 m | read |
|---|---|---|
| `skater.reverse_brake_decel` (20) | 14 / 26 / 32 / 40 -> 29 / 57 / 78.5 / 92.5 | the one lever: a harder stop moves the window a metre toward the defender |
| `skater.burst.frames` (20) | 0 / 12 / 30 -> 66 / 33.5 / 22.5 | **more burst is worse**: the burst is every skater's, and the defender starts from a standstill |
| `skater.burst.accel_mult` (1.8) | 1.4 / 2.2 / 2.6 -> 39 / 19 / 14.5 | the same |
| the puck drawn in while braking (x0.6) | 31 | 5 points: not worth a key (`FJamStickConfig::BrakeReachMult` stays code only, 1) |
| `stick.carry_reach_mult` (0.75) | 1 -> 23.5 | no change |

**No movement key was changed.** The clean stop-and-go already beats the pro more than two times in three; the straight charge is never past it (the vision's
law 2 calls it the wrong decision; no key in the family moves it). `reverse_brake_decel` 26 would widen the window by a metre (65 ms of timing) but it is
every skater's stop, the vision's "about 20", lane A93's timings and the curve: it is the next evolution, with its number.

## 4. The human's carries (the design eye's read, R-0048)

Traced tournament logs, 12 seeds, the target class at each level, hockey.regular, read by lane A100's `docs/design/review-1/humancarry.mjs`. Before = this
build with the neutral block.

| cell | carry median | p90 | carries of 3 s a match | ended by an opponent's take | puck on nobody's blade | A100 on main |
|---|---|---|---|---|---|---|
| novice at rookie | 0.83 -> **1.85 s** | 3.45 -> 5.25 s | 3.3 -> 3.6 | 49 -> 24 % | 63.0 -> 54.9 % | 0.87 s, 2.5 |
| regular at pro | 0.77 -> **0.85 s** | 2.08 -> 3.40 s | 1.1 -> 2.6 | 61 -> 43 % | 58.6 -> 54.4 % | 0.75 s, 0.8 |
| good at all_star | 0.70 -> **0.77 s** | 1.53 -> 2.48 s | 0.0 -> 1.3 | 60 -> 36 % | 59.7 -> 56.1 % | 0.67 s, 0.1 |

At rookie the carry doubles. At pro and all_star the median hardly moves: the bots press the moment they are in reach (pro pokes after 6 frames and lifts at
contact, all_star at once), so a short carry is still short; the long carries grew. The puck is still on nobody's blade more than half of play: that share is
shots, passes, dumps and pokes, not touches. The lever for the median at pro is the bots' press (the brain: when it pokes and lifts), not the sim.

## 5. The three cells: the ladder moves (R-0048 expected it; the levels were not tuned)

600 matches a cell, hockey.regular, A53's seeds, both arms on the same build and the same seeds (`tools/perf/results/a102/curve.mjs`, a copy of A95's tool with
`-Multiprocess`; `cells.jsonl`, raw matches in `raw/`). Before = the neutral block (`stick.passive_take` true, `stick.body_shield` false).

| cell | R-0046 (2,000) | before, this build | **after (both rules)** | move | goals for : against, before -> after | carry share of play | the human's goal inside 2 minutes |
|---|---|---|---|---|---|---|---|
| novice at rookie | 85.3 | 86.2 (83.2-88.7) | **99.8** (99.1-100) | +13.6 | 2.52 : 0.83 -> 3.00 : 0.24 | 10.9 -> 18.5 % | 270 -> 521 of 600 |
| regular at pro | 48.3 | 49.3 (45.3-53.3) | **72.0** (68.3-75.4) | +22.7 | 1.97 : 2.02 -> 2.47 : 1.56 | 11.1 -> 14.4 % | 235 -> 355 of 600 |
| good at all_star | 26.3 | 26.5 (23.1-30.2) | **39.8** (36.0-43.8) | +13.3 | 1.43 : 2.52 -> 1.82 : 2.26 | 10.5 -> 12.4 % | 198 -> 254 of 600 |

The body shield alone (build 1, passive take as before, 600 matches): 84.2 against 83.3, 49.3 against 49.0, 29.8 against 27.2: +0.9 / +0.3 / +2.6. So the move
is the passive take: the human keeps the puck, the bots have to press to get it, and rookie bots do not press at all (they never poke or lift, and check only
from 6 m/s of closing). Every cell is more than 4 points off: by R-0047 point 4 that is a rule that ships with its neutral value until the curve is re-set;
R-0048 set the default to false and announced a curve lane. Both paths are one key: `passive_take` true in the three presets' text or the default in
`JamCartridge.h` gives back the ladder of R-0046 (the before column) with the body shield still on.

## 6. Gate, hash, neutral value

- Full `Jam` filter on the final tree (`Game/logs/a102-tests-jam-full-3-ue.log`): **159 pass, 0 fail** (156 on main + `Jam.Core.Movement.StopAndGo`, `.BodyShield`, `.PassiveTake`).
- `Jam.Core.Determinism` (seed 1, frame 600): hockey.regular `d192973ac7c97102`, hockey.raw `b02f67a2da0314b4`, before and after (its scripted 600 frames meet
  neither rule; real matches differ, as the cells show).
- Neutral: `Jam.Core.MovementNeutral` (the two pinned bot matches of the commit before the vision) and the contact lane's neutral test replay hash for hash with
  `"passive_take": true, "body_shield": false` in their neutral blocks plus the teammate code switch.
- Two older tests asserted a clean lift from 1.2 m straight behind a carrier whose puck is ahead, which the body shield refuses: `Jam.Core.Contact.StandingBrace`
  (its case is about the shove: it now sets `body_shield` false) and `Jam.Core.TacklingReadouts` (the lifter now stands beside the carrier).
- Bars of `Jam.Core.Movement.StopAndGo` (pro, 200 seeds an arm; measured): clean stop-and-go at least 80 % (100); straight charge at most 20 % (0); gap at
  least 47 points (100); the body shield worth at least 10 points (100 against 73); the stop begun at 6 m at most 25 % (2.5); the blade left ahead at least
  20 points worse (46.5).

## 7. Drill 21 in the real game mode, and what is queued

Headless, the editor binary as the game on Rink.umap (`-game -nullrhi`, no window, no cursor; `Game/logs/a102/run-drill.ps1`), hockey.regular at pro:

| `jam.ctrl.CarryScript` | rounds | past | lifted | other |
|---|---|---|---|---|
| 1 (stop-and-go) | 20 | **20** | 0 | |
| 0 (straight charge) | 20 | **0** | 20 | the first run lost 1 round of 20 (staged while the human was still down from the round before); a round now waits for the human to be on its feet |

Queued for the cursor's owner (this lane did not hold it): a Development package of this tree; `jam.ctrl.Drill 21` with `jam.ctrl.CarryScript` 1, 0 and 2 (by
hand: is the 9.5 m stop learnable with real keys and a real mouse?); **the whole acceptance run** (`tools/qa/accept/accept.ps1`), not only step 1's brake and
carry rows: the passive take changes how every puck is won, so any row in which the human or a scripted skater took a puck by skating into it can change.

## 8. What the measurements say beyond the brief

1. The brief's stop-and-go (brake at 5 m) cannot work against a pro, and no movement key makes it work: the pro closes those 5 m faster than any stop.
   The clean stop begins when the pro commits (9.5 m at pro, 12 m at all_star). The window is about 2 m of gap wide, a quarter of a second: at 8 m 25.5 %,
   at 7 m 1 %.
2. The first-step burst is every skater's. Against a defender that starts from a standstill it helps the charge more than the cut (at the late edge, no burst
   66 % against 25.5 %).
3. With the passive take off, the brain's defenders still skate into the puck expecting a touch to take it. At rookie nothing takes a carried puck but a hit
   (a straight charge at a rookie defender: 0 -> 53.5 %). The brain has to learn to press (Q2's family).
4. The pro lifts into the body 158 times in 200 clean carries and every one is refused: the brain does not read `IsBodyShielded` (it reads `IsPokeShielded`
   nowhere either). A defender that went round, or hit, would be the answer in hockey.
