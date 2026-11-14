# POSSESSION: the puck belongs to whoever touches it (lane A208, track F0, the night of 14 Nov)

The operator after H34 (R-0067 point 2): "Puck does not stick with players, it moves through them without locking."

## 1. What was true (seen on main b530b4eb, no code)

The Director's four suspicions, tested:

| suspicion | true? | what the code did |
|---|---|---|
| (a) R-0048's "kept until somebody presses" also stops a loose puck | **partly**. R-0048's skip (`StepPuck`, the `RidingCarrier()` line) applies only to a puck riding with a carrier, so a loose puck is not refused by it. But the same skip lets a CARRIED puck pass straight through every other body (a `continue` before the body collision): that is "moves through them" whenever a carrier skates his puck into a man. | |
| (b) no collision with bodies | **false for loose pucks, true for carried ones**. A loose puck meets the body circle (0.4 m + puck 0.12 m), but the push only removes the normal component of its motion: a puck met off-centre or by a skating man slides on along him and away (the "touch without lock"); the C-0005 blade branch `continue`s past the body for an opposing puck over 6 m/s at an unaimed blade. Body test at the substep's end point only (no sweep). | |
| (c) the pickup reach is the blade point | **true, worse**: the blade takes a loose puck only if it is faster than 6 m/s (a deflection or a mate's pass reception). A slow or resting puck at his blade is never his: he has to skate his BODY onto it. | |
| (d) rookie bots' poke off, late reads | true by the profile (`opp.poke_frames` -1 at rookie); with the blade unable to take a slow puck, a bot beside a loose puck takes it only by skating over it. The brain is A206's. | |

Also found: a puck taken from behind sat INSIDE the taker's body for 37 to 73 frames (the own-body push-out to the back fought the carry assist's pull to the front blade: it settled inside him), and a carried puck was behind its carrier's skates 3 % of carry frames.

Match counts (`Bar.Possession.Trace`, 100 matches each, the scripted human against the brain; a crossing = a loose puck's path in a frame meets a standing skater's arc: 1.5 m and 60 degrees of his blade or facing, his body, or his blade's 0.6 m disc):

| | rookie before | rookie after | pro before | pro after |
|---|---|---|---|---|
| crossings | 41,246 | 5,589 | 38,748 | 12,588 |
| picked up by him | 11.9 % | 38.5 % | 14.2 % | 34.9 % |
| by a nearer skater on the same frame | 19.1 % | 61.3 % | 25.0 % | 64.9 % |
| **nobody's** | **68.9 %** | **0.3 %** | **60.8 %** | **0.2 %** |
| why nobody's (before): slow puck at the blade / his own / a mate's pass missed / other | 6,745 / 7,595 / 5,225 / 7,038 | 6 / 5 / 1 / 2 | 3,042 / 7,432 / 3,434 / 7,560 | 4 / 9 / 1 / 11 |
| frames a loose puck sat inside a standing body | 5,581 | 1,490 | 5,559 | 3,209 |
| carry frames (the puck riding a carrier) | 336,083 | 737,497 | 332,805 | 956,327 |
| carry frames within 15 cm of the blade | 97.6 % | 98.6 % | 96.5 % | 97.9 % |

(The crossings fall because a puck that is his at once crosses no more arcs.)

## 2. The rule (`game.stick.pickup`, on by default)

| key | default | neutral | meaning |
|---|---|---|---|
| `stick.pickup.arc_m` | 1.5 | 0 | the stick's arc: from the body centre (the carry reach is 1.65 m) |
| `stick.pickup.arc_deg` | 60 | (any) | either side of the blade's direction and of the facing; the blade's 0.6 m disc always counts |
| `stick.pickup.body` | true | false | the body stops a loose puck (swept against the body circle: no speed tunnels) and it is his |
| `stick.pickup.sting_mps` | 18 | (any) | an opposing puck faster than this (relative) rings off the blade (keeps 0.4 of its speed) or bounces off the body (e 0.3): a loose rebound; a team-mate's puck rings off only above 1.5 times this (27 m/s: a shot through him, no pass is that fast) |
| `stick.pickup.body_catch_mps` | 18 | (any) | the body seats a loose puck only when it meets him slower than this (into the body); faster it bounces off as a rebound |
| `stick.pickup.block_carried` | true | (any) | with body: another's carried puck is pushed out of a body it meets, on the carrier's side (he keeps it); false = it passes through as before |

- A puck nobody carries (not riding with a carrier) under the blades (0.2 m), in play, no draw, no shotgun, that crosses a standing, unbraced skater's arc in a
  substep is his: the nearest such skater takes it, seated with his motion (SettleFrames 4), on his blade within a few frames. His own puck only when it comes
  back at him. Never through a net's wall.
- R-0048 holds: a carried puck is taken only by a press (poke, lift, check). Where this rule ends and that one begins: `RidingCarrier()`. With `pickup.body`
  a carried puck no longer passes through another body: it is pushed out of it like off the boards (`BlockCarried`, after the bodies and again after the
  carry's pull), and its carrier keeps it while it stays in his reach. If he skates on and leaves it at the man, it is a loose puck: the man's.
- His own carried puck does not collide with his own body any more (the carry owns it): a puck won from behind goes through to the blade in 1 to 2 frames.
- An OPPOSING puck in flight (faster than 6 m/s against him: a pass or a shot) is not met by the arc: only by an aimed blade's 0.6 m disc (caught under the
  sting, rung off above) and by the body. The arc across a passing lane intercepted every pass through traffic (section 5). When lane A207's pass state lands
  (`Puck.PassFrames`), "in flight" should read it instead of the speed.
- The old C-0005 blade branch is off while `arc_m` is above 0 (the arc and the aimed-blade disc replace it); A46's body reception stays.

Code: `UJamSim::MayPlayLoose`, `TryLoosePickup`, `BodyMeetsPuck`, `BlockCarried`, `NetBetween` (JamSim.cpp, their own block above `StepPuck`); call
lines in `StepPuck`: one after the goalie save, one at the R-0048 skip, one before the body push, one after the carry assist; one condition each on the C-0005
and A46 blocks. No new state field (the hash folds nothing new: `SettleFrames`, `LastTouch`, `DeflectCooldown`, `PassesReceived`, `PassReceivedFrame` were there).

## 3. The staged scenes (`Jam.Core.Possession.Staged`, `Sweep`, `Lock`; puck.max_speed 60; final build 14)

He stands at the origin facing the puck's side and looks at it (his blade aimed its way at rest length, as a person's mouse or a bot's blade near a puck).
his = he carries 10 frames on end (his@ = the frame he first carries); bounce = it came off him loose; untaken = it lies in his reach, not his. The full rows:
`tools/perf/results/a208/staged-final.txt`.

| scenes | neutral (the game before) | rule |
|---|---|---|
| loose (an opponent's) 2 and 6 m/s at feet / body / blade, front / side / behind (18) | his 15, untaken 3 (a slow puck at the blade: never his), his@ 3-54 | his 18, his@ 1-36 (the puck's travel) |
| loose 12 m/s (9) | bounce 9 (the aimed blade deflects, the body pushes it on) | his 9, his@ 3-13 |
| loose 20 m/s, over sting_mps (9) | bounce 9 | bounce 9 (stung: a rebound) |
| skate over a resting puck at walk / glide / sprint, 0 / 0.3 / 0.8 m aside (9) | his 5, bounce 1, untaken 3 (beside his feet) | his 9, his@ 10-33 |
| a mate's pass 8-26 m/s at his feet / 1 m off his stick / at his skates (12) | his 12 | his 12 |
| a shot 25 / 35 m/s into his body (2) | bounce 2 | bounce 2 |
| an opponent's carried puck through him (the carrier 0.9 m aside), 4 / 7 / 9 m/s | inside him 0 frames (that carrier never had it: a slow puck at a blade) | inside him 0 frames; the carrier keeps it 14-24 of 40 frames, then it is loose at the man |
| totals (59 scenes) | his 32, through 0 | his 48 (the other 11: stings), through 0, inside his body at most 2 frames |
| sweep: 1,440 arrivals into the body, 2..40 m/s, 24 directions, 3 offsets | his 504, through 0 | his 744, bounced 696, **through 0** |
| the lock: a carry through a sprint and two hard turns (234 frames) | | 100 % within 15 cm of the blade, worst 0.133 m, never behind his skates |

Before the blade aimed at the puck (an earlier run of the same scenes, the blade at rest ahead): a puck taken from behind sat INSIDE his body 37-73 frames
under the neutral value (the own-body push to the back against the carry's pull to the front); the rule's own-carry skip fixed it (at most 2 frames).

## 4. Proof

- Neutral (`stick.pickup` `{"arc_m": 0, "body": false}`): 100 seeds a level and rink (six cells), state hash, frames, goals and result: 600 of 600 identical
  with main b530b4eb (`tools/perf/results/a208/proof.sh`, labels B0 / N0). `Jam.Core.Possession.Neutral`: at arc_m 0 and body false the other two numbers do
  not move the hash. `JamBaseline::MakeNeutral` carries the neutral pair.
- The cells and law 3: see `status/A208.md`.

## 5. What the rule did to the curve, and what was changed for it (pro, large rink, 100 seeds an arm; main B0: 56 %)

| arm (build) | win % | opp shots a match | the human's carry % |
|---|---|---|---|
| main (B0) / pickup neutral | 56 | 12.7 | 20.9 |
| first rule: arc 1.5 m on every puck, body seats every puck under 18 m/s (build 7) | 1 | 54.6 | 8.2 |
| block_carried false | 10 | 44.4 | 11.7 |
| arc only (body false) | 23 | 18.3 | 11.2 |
| body only (arc 0) | 4 | 44.7 | 14.9 |
| body_catch 4 / 8 m/s, sting 12 / 80: no arm above 7 | | | |

Hand changes (who takes the puck from whom, `Bar.Possession.Trace` D1 / D2, 30 matches): with the rule the human took passes from his mates 162 -> 37
and pucks from opponents 548 -> 229; opponents' support men took from us 279 -> 609 (the arc across the passing lanes). With the body alone the human's own
takes fell (548 -> 355) and his chaser mate's rose (358 -> 639): the first body on a loose puck keeps it, and his own mates race him to every loose puck
near him. Changes made for it: an opposing puck in flight meets only an aimed blade and the body (not the arc); a poke's and a lift's puck is the presser's
(the stripped man cannot retake it for 15 frames); a shoved (stumbling) carrier keeps his puck in his reach; the A46 body reception stays; the carried
puck blocked by a body goes out on its carrier's side; nothing of the body path runs while a shotgun flies.
The rest is the brain's: mates who leave a loose puck near the human to him (A206).

## 6. READY 1 (build 15): the cells and law 3

Six cells x 600 (the ladder's instrument, `tools/perf/results/a208/cells.sh`, labels R4): rookie regular 99.3 / large 74.7, pro 43.7 / 3.0, all_star 6.7 / 0.3
(main: 87.3 / 86.8, 48.0 / 47.3, 27.5 / 24.5). Law 3 (`Bar.Curve.Law3`, 200 a row) is the same at the default and at the pickup's neutral: pro stop at 8 m 94.0 %,
straight charge 0 %; all_star 99.5 % and 0 %. The curve lane re-sets the ladder; wave 2 looks at the large rink, where the scripted human loses the most.

## 7. READY 1b: the merged tree (main d0d132bf: A206 evolution 9, A207's pass and doubled shot, A209, A210)

Seam rules: the passer's own puck is never his arc's while lane A207's clear_own_body window runs; a pass in flight to a mate is A207's `TryCatchPass`
(the arc skips it); a puck over the crease man's block height passes his arc; a deke's carried puck is not blocked by a body; a hard puck at a body is
the old push (`PickupBodyBounce` 0: a doubled shot glancing off the crease man slides on as before). Gate 337 / 337; neutral = main d0d132bf to the hash
(600 of 600). Opponents' long shots (20-35 m) at pro: the bodies of the human's side block 46 % (neutral 43 %).
