# PASS-SHOT: a pass arrives, a shot is a shot (lane A207, track P1, the night of 14 Nov)

The operator after H34 (R-0067): "They want to pass but no pass reaches the end (inprecise direction or speed), passess need to be able to be hold enough
to cross 1/3 of arena length on max, shoot speed should double".

## 1. The pass as it was (b530b4eb, `Bar.Pass.Measure`, 40 matches a level, the regular scripted player, the closing press)

Every pass from its release to the first touch by anybody but the passer. "The man" = the mate the sim aimed it at.

| level | class | passes | the man seated it | another mate | an opponent | nobody | median closest to his blade | median angle error |
|---|---|---|---|---|---|---|---|---|
| rookie | human -> mate | 446 | 11.7 % | 13.7 | 29.1 | 42.2 | 6.6 m | 21 deg |
| rookie | bot -> human | 333 | 18.0 | 9.3 | 19.2 | 38.1 | 2.3 | 15 |
| rookie | bot -> bot | 1398 | 20.9 | 20.2 | 28.0 | 23.0 | 6.3 | 15 |
| pro | human -> mate | 396 | 14.1 | 11.1 | 50.3 | 19.4 | 7.2 | 19 |
| pro | bot -> human | 414 | 14.3 | 14.3 | 27.8 | 32.4 | 2.7 | 19 |
| pro | bot -> bot | 2227 | 15.1 | 22.0 | 29.4 | 28.7 | 8.1 | 23 |

Why (read from the rows):
1. **The human's pass never went where he aimed.** `TryPass` sent every pass to `PickReceiver`'s "best open mate ahead" whatever the stick pointed at,
   and with nobody open the press did nothing at all.
2. **Every pass left at 14 m/s.** hockey.regular has `puck.drag` 0.5 /s and ice friction 0.147 m/s^2: a 14 m/s pass is under a skater's 11 m/s after
   5.8 m and under 6 m/s after 15.3 m, and the blade takes nothing slower than 6 m/s (`DeflectMinSpeed`). Passes of 15 m or more reached the man 6 to 14 %.
3. **The lead overshot.** The aim was the receiver's position after the flight PLUS `pass_lead_frames` 20 of his velocity: 2.7 m in front of him at 8 m/s.
4. **The passer's own body stopped his pass** when the man stood on the far side of it (rows with the release 140 to 177 degrees off the man, the puck back
   on the passer's stick 12 frames later).
5. The shot: tap 17.8, part charge 20 to 22, full 25.8 m/s (`shot.speed` 18, `charge_speed_bonus` 8, `puck.max_speed` 30).

## 2. The rules (all ship on; each key has a neutral value that replays the game before)

| key | ships | neutral | what it does |
|---|---|---|---|
| `game.pass.power.full_frames` | 30 | 0 | the human's RMB hold is the power: tap (<= `shot.tap_frames` 4) = `skater.pass_speed` 14; then evenly up to the full speed at 30 frames (0.5 s) |
| `game.pass.power.reach_share` | 0.3333 | | the full pass covers this share of the rink's length (30 m on 90 m) ... |
| `game.pass.power.arrive_mps` | 8 | | ... and still moves 8 m/s there: full speed solved = 23.3 m/s on hockey.regular. A bot's edge pass gets the speed that arrives at 8 m/s at its man (14..23.3) |
| `game.pass.aim.cone_deg` | 25 | 0 | the human's pass goes to the mate nearest the stick's line inside 25 deg (the release frame's stick input; the facing with the stick at rest); nobody inside it: along the stick, it still goes |
| `game.pass.catch_m` | 1.2 | 0 | a pass in flight is seated on a team-mate's blade within 1.2 m at more than 1 m/s (closing on it) |
| `game.pass.clear_own_body` | true | false | the passer's (shooter's) body lets his own just-released puck by while his cooldown runs |
| `game.shot.speed_scale` | 2 | 1 | the shot's speed and the puck's speed cap x 2: tap 36, half 40, full 52 m/s |

The lead under power: `UJamSim::AimPassAt` aims at the receiver's BLADE where it will be when the puck gets there (his velocity held; the puck's drag and
friction in closed form: v(t) = (v0 + f/k) e^(-kt) - f/k), no extra lead frames. A man the pass cannot reach (he outruns it) is aimed at the point the
puck comes nearest to him. `BestPass()` (the brain's `W.CarrierBestPass`) under power picks only men the pass reaches alive.

The saucer under power is the sim's, not the hold's: the hold is power, and a second meaning on the same button made the human's every longer pass a
lob. The sim lifts the pass when an opposing blade sits on the flat line where the saucer flies over it (from 1.5 m to three quarters of the way, the body
0.9 m or more off the line). A bot's held saucer works as before. Nothing to learn, nothing to rebind.

Hold length for full power: **30 frames (0.5 s)**. A tap stays a tap (4 frames); 30 frames is the shot's own full charge, so both buttons read the same
to the hand; under `charge_dies_on_loss` a strip during a hold kills the pass: at half a second that is right (a man who holds the puck half a second
under pressure has chosen to be stripped; a quick 10-frame pass of 16 m/s is still there for the hurried).

## 3. Proof (READY 1, 14 Nov 22:00)

**Staged (`Bar.Pass.Staged`, `Bar.Shot.Staged`, hockey.regular, the human passing):**

| bar | neutral (keys off) | on |
|---|---|---|
| tap at an open standing mate 7-10 m: caught (bar 95 %) | 100 % | 100 % |
| full hold into open ice: way to 7 m/s; speed at 30 m (bar >= 30 m, > 6 m/s) | 13.6 m; never got there | 31.5 m; 7.8 m/s |
| full hold at a standing mate 28-32 m: caught | 0 of 20 | 20 of 20, arriving at 10 m/s |
| stick 0-20 deg off a mate 10-25 m, 0-8 m/s across: caught (bar 80 %) | 30 % | 76 % with a hand that does not track him (re-measured with the tracking hand: see wave 2) |
| shot tap / half / full (bar 2.0 x) | 17.8 / 20.0 / 25.8 m/s | 35.7 / 39.9 / 51.6 = 2.00 x each |
| 200 full shots on the ice at a body, the boards, a post from 2-20 m | | 0 through a body, 0 out of the rink |

The power curve (release speed by frames held): 1-4: 13.9 | 5: 14.2 | 6: 14.6 | 11: 16.4 | 16: 18.1 | 21: 19.9 | 26: 21.7 | 31+: 23.1 m/s.

**Matches (`Bar.Pass.Measure`, 40 a level):** bot -> bot to a mate (the man or another) at pro 37 % -> 73 % (the man 15 -> 38 %); bot -> human seated
14 -> 42 % (pro), 18 -> 55 % (rookie); bot passes to an opponent 29 -> 17 %. Median release angle error 15-23 -> 6-7 deg. What still misses: the receiver
changes course after the release (median closest approach of his blade 4.6-5.7 m): the brain's part (A206: hold the line, or skate onto a pass in flight).

**The neutral proof:** with the five keys at their neutral values, 600 matches (100 seeds x rookie, pro, all_star x regular, large; the regular player, the
closing press) replay the pre-edit build (b530b4eb) result, goals, frames and state hash, 600 of 600.

**The six cells (600 each; before = this tree with the keys neutral, same instrument):**

| cell | before | on |
|---|---|---|
| rookie regular | 98.3 | 98.7 |
| pro regular | 48.0 | 44.8 |
| all_star regular | 9.8 | 3.8 |
| rookie large | 99.7 | 99.3 |
| pro large | 50.8 | 31.3 |
| all_star large | 4.7 | 21.0 |

Passes the human receives a match: 3.3 -> 6.1 (rookie regular), 4.2 -> 6.4 (pro regular), 2.5 -> 4.8 (pro large). The curve lane re-sets the ladder.

**Law 3** (200 seeds a number, the game as it ships): rookie 100 / 100 / 100 / 100 / straight 0; pro 94.0 / 99.5 / 93.0 / 83.5 / straight 0; all_star
99.5 / 99.5 / 99.0 / 93.0 / straight 0: the same rows as before the lane (A202): the drill has no pass or shot in it.

## 4. Wave 2 (14 Nov 22:17 onwards)

**Which half moves the ladder (600 a cell, the same instrument):**

| cell | neutral | pass rules only | shot x2 only | both (READY 1) |
|---|---|---|---|---|
| pro regular | 48.0 | 62.7 | 33.2 | 44.8 |
| all_star regular | 9.8 | 14.5 | 4.3 | 3.8 |
| pro large | 50.8 | 58.2 | 26.0 | 31.3 |

**What the doubled shot breaks** (`Bar.Pass.Measure` SHOTFATE, 100 matches at pro): the bots shoot at the human's net from 20-35 m about 4.4 times a
match (the scripted human 0.2). At 1x those shots die on the way or are collected (in: 6.5 % regular, 20.8 % large); at 2x they reach the net (44-48 %,
77 %). A defender stands in the crease for only 4 % (regular) to 18 % (large) of them. Not tunnelling (0 frames through a body or out of the rink in 300
matches; the goal counts at 52 m/s), not the lift. **The crease man reads a long shot** (`game.skater.goalie.read_mps` 2.5, `read_max_m` 1.0; neutral 0; `UJamSim::GoalieReadExtraM`): past 0.35 s of
the shot's flight (estimated from the shooter's place now and the puck's speed, no new state) his save reach and his block height grow by 2.5 m a second,
to 1 m more: the glove goes up for the shot he sees coming, the quick shot from the slot is not read. Staged net (`Bar.Shot.Staged`, a defender 1.5 m
in front of the goal, shots from 12 / 25 / 30 m, on the axis and 6 m to each side, tap / half / full, 9 each): doubled shot, read off: 5 / 6 / 6 goals of
9; read on: 5 / 0 / 0. At a 0.25 s reaction it also took the 12 m shot (2 of 9): 0.35. In matches it moves little (the bots' 25-35 m shots in: regular
48.5 %, large 64.2 % at pro), because a defender stands in the crease at 3 % (regular) to 17 % (large) of those shots: the read cannot save a shot with
nobody there. What else must give: a crease man (the brain), the bots' long-shot choice (the brain), then, only if both fall short, a shot speed that
falls off with the distance at release (keeps "double" from the slot).

**The pass aimed at a man flies by the others:** `Puck.PassTo` (hashed with `PassFrames`) and `UJamSim::PassFliesBy`: a team-mate who is not the pass's
man lets it by while it moves faster than 6 m/s against him and is clear of his body (his body stays solid); under 6 m/s any mate's blade within
`catch_m` takes it. `Jam.Bot.PassesToHuman`: pro 7.05, rookie 4.70 passes received a match (bar 1.0).

**The acceptance rows** (`HokejAcceptDrill.cpp` step 5, `accept.mjs`): the held row `saucer` is now `pass_power` (36 frames, the mate 28 m ahead, full
power, caught). `Jam.Core.PassShotRows` replays steps 4 to 6 headless: all six rows pass (shots 35.7 / 39.9 / 51.6 m/s score from 10 m).

**The power line** (`HUDPassMeter.cpp`): stills off screen from the game camera (`tools/perf/results/a207/meterstill.ps1`: -RenderOffScreen, the scripted human,
`jam.hud.PassMeterDemo 20` draws the line as a 20-frame hold while he carries, `jam.hud.PassMeterStills 10`): `pass-shot/u1-passmeter-f000135.png`
(at the draw: the line from the puck along the stick, 22.9 m), `pass-shot/u1-passmeter-f000640.png` (in the corner). White with an ink edge, on the ice under
every HUD element; his team's colour when a mate is in the cone. Read by eye only: the operator's eye on a package is the real proof.

## 5. Wave 2b on the merged tree (main 015f1618: the brain's evolution 9, A208's pickup and bodies, A209's contact)

**The long shot, merged, before the fade** (100 matches at pro; the bots' shots at the human from 25-35 m): regular 46.4 % in (blocked 27.7 %, saved 10.9 %,
a crease man at 4 % of them); large 53.1 % (blocked 3.1 %, saved 19.4 %, crease man 15 %).

**The shot fades** (`game.shot.fade_m` 12; neutral 0): the added speed (what `speed_scale` put on top of the 1x shot) fades as e^(-x / 12 m) over the
distance the puck travels (`FJamPuckState::ShotExcess`, hashed while not 0; any touch ends it): 2x off the stick, about 1.4x after 10 m, 1.1x after 30 m.

| bots' shots at the human, pro | under 12 m | 12-25 m | 25-35 m |
|---|---|---|---|
| regular, no fade | 35.0 % | 34.9 % | 46.4 % |
| regular, fade 25 | 42.2 | 31.1 | 34.8 |
| regular, fade 12 (ships) | 46.2 | 33.5 | 21.4 |
| large, no fade | 32.7 | 34.4 | 53.1 |
| large, fade 25 | 30.2 | 33.2 | 44.8 |
| large, fade 12 (ships) | 35.5 | 23.6 | 34.7 |

The close shot keeps its danger (under 12 m: 46 % / 36 % in). The long one falls but not to the bar (15 % at pro): at 1x the large rink's long shot already
went in 20.8 %: the shot's speed alone cannot get it under 15 % while the crease is empty at 84-98 % of those shots. What else must give: the crease man
(A206 wave 2), the human's side in the lane (A208's bodies block 41.6 % on the regular rink, 5 % on the large one).

**A bot passes to nobody nearer than 3 m** (`UJamSim::BotPassMinM`; 2 m when an opponent within 2 m closes on him; the human keeps 2 m): no key (the Director's
ruling 01:12).
