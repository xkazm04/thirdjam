# BRAIN-8: the bots do nothing stupid in front of the operator (lane A202, track Q2, evolution 8; Sat 14 Nov 14:41-)

Follows `BRAIN-7.md` (lane A193). Tree: main `58af359f` (brain7 on at pro and all_star, the hits' wind-ups 4 / 4, the held cross-check, the breakaway
bonus 0.3, free skaters 15 % faster). Source: `Game/Source/JamCore/AI/JamAICore.h/.cpp`, `DifficultyProfile.cpp`, `Public/JamBaseline.h`; tests and the
tracer `Tests/HockeyBrain8Tests.cpp` (`Jam.Bot.Hockey8.*`, `Bar.Brain.Watch`). Drivers and rows: `tools/perf/results/a202/` (`cells.jsonl`; raw matches
under `raw/`, not landed). Logs: `Game/logs/a202/`. Status with every step: `status/A202.md`. **Shipped ON (the Director, 16:52): all four rules
(`kEvo8Ship*` true); `brain8=0` (and every older `brainN=0`) gives back evolution 7, the old game, to the hash.** The bots' brace stays off.

## 1. The instrument: `Bar.Brain.Watch`

The match is played in process (`RunPlaytestMatch`, the regular scripted player, `JAM_PLAYTESTER_CHECK=closing`) with the playtest trace on every frame;
the trace reads the whole `UJamSim` (positions, velocities, downs, the boards, the shots, the hits) and the brain's role and job per skater. Eighteen
detectors keep runs (a whistle or a faceoff ends a run uncounted) and write each episode with its cell, seed, frame, skater and length; `dump=f0:f1:every`
prints every skater per frame (`WDUMP` lines) for a still-by-still read. `env JAM_BRAIN_WATCH="levels=..;carts=..;seeds=6;from=1;tune=a=1|b=2;dump=.."`;
driver `tools/perf/results/a202/watch.sh <tag> "<env>"`.

## 2. The watch table (item 1)

60 matches on main: pro and all_star x `hockey.regular`, `hockey.arena.large`, Arsenal, Shotgun, Marathon x seeds 1 to 6. Two runs, identical count for
count. A match = per match over the 60.

| rank | what the trace shows (bots unless said) | a match | regular / large / arsenal / shotgun / marathon (of 60) | worst three (cell seed frame: frames) |
|---|---|---|---|---|
| 1 | shot released from beyond 30 m in the attacking half | 6.05 | 56 / 88 / 35 / 0 / 184 | marathon all_star s1 f2157; s3 f17324; marathon pro s3 f20508 |
| 1a | a shot from beyond 30 m with a mate open 5 m nearer the net | 2.55 | 35 / 14 / 37 / 28 / 39 | marathon all_star s4 f1719 (137.7 m); s6 f2691; marathon pro s4 f12097 |
| 2 | the carrier skates into its own zone with open ice ahead (> 45 f) | 2.68 | 16 / 40 / 30 / 12 / 63 | large all_star s5 f1843 (421 f); large all_star s2 f8867 (354 f); marathon all_star s1 f13721 (345 f) |
| 3 | standing inside 1.2 m of a downed body (> 30 f) | 2.60 | 16 / 33 / 16 / 33 / 58 | large all_star s3 f9401 (126 f); shotgun pro s5 f2432 (117 f); arsenal pro s2 f3976 (95 f) |
| 3a | ... pushing him (closing > 0.5 m/s the whole spell) | 1.10 | 2 / 16 / 10 / 11 / 27 | large all_star s3 f9401 (126 f); large pro s2 f10279 (87 f); arsenal pro s2 f3976 (81 f) |
| 4 | two mates inside 1.5 m (> 90 f) | 1.37 | 3 / 38 / 1 / 4 / 36 | marathon all_star s2 f9672 (413 f); large all_star s4 f7180 (368 f); large pro s6 f1154 (343 f) |
| 5 | the bots never brace (braced hits by bots 0; by the human 1.10 a match) | 0 | | section 4 |
| 6 | idle: standing, no puck, > 3 s | 0.77 | 2 / 26 / 4 / 0 / 14 | large pro s5 f9225 (502 f); marathon pro s6 f284 (468 f); large pro s4 f9458 (394 f) |
| 7 | the slot empty of defenders with the puck carried below the circles (> 60 f) | 0.50 | 1 / 11 / 1 / 0 / 17 | marathon all_star s1 f7933 (181 f); large all_star s4 f10376 (175 f) |
| 8 | pressing into a net / into the boards (> 60 f, slow) | 0.20 / 0.22 | | marathon pro s2 f9347 (374 f, a net); regular pro s5 f17855 (170 f, boards) |
| 9 | circling: a net turn of the travel over 720 degrees in 3 s, moving, no touch | 0.10 | | shotgun all_star s3 f253; arsenal pro s5 f3819 |
| 10 | a goal from beyond 30 m | 0.08 | 0 / 3 / 1 / 0 / 1 | marathon all_star s6 f17782 (37.1 m); large all_star s4 f5610 (34.1 m) |
| - | the human standing at a downed body (the script's, not the brain's) | 1.02 | | |
| - | not seen: a loose puck nobody goes to, a check on a man without the puck 15 m from it (the brain checks only carriers), a carrier stalled | 0 / 0 / 0.02 | | |

What the stills say (`WDUMP`, `Game/logs/a202/d1..d4.log`):
- **The own-zone carry** (large all_star s5 f1843): team 1's carrier on the side boards with the human and his mate 4 to 9 m behind it skates away from
  them, the whole length of its zone and round its own net, for seven seconds.
- **The clump** (large all_star s4 f7180): the carrier and its support skate the length of the ice 1.3 m apart, the puck passed between them: two bodies glued.
- **The downed push** (large all_star s3 f9401): the downed carrier in the corner; the human's mate and the downed man's own mate both press on him on the
  boards for the loose puck at his body.
- **The idle** (large pro s5 f9225): the backs standing in their own crease while play is at the far end: the last man playing goalie (no goalies in
  this game). Not a fault: not fixed.
- A first circling detector (the absolute heading change) read 28 a match: the facing's jitter. The net turn of the travel reads 0.10.

## 3. The fixes (item 2): one function each, one call line in `Think`, a key per side

| rule (function) | keys (`opp.` / `mate.`) | what it does | brain8=1 value |
|---|---|---|---|
| A. `LeaveDowned` (+ evolution 5's `SteerRoundDowned`) | `hk_downed_leave_frames`, `hk_avoid_downed_m`, `hk_avoid_downed_out` | a bot (not carrying) that has stood inside 1.2 m of a downed skater for that many frames leaves him: its move goes out from his body, half its own move across that line kept, until 2 m clear; it also steers round a downed man on its way | 6 frames; 1.3 m, 0.7 |
| B. `LongShotToPass` | `hk_long_pass_m` | a shot from beyond that distance while a team-mate stands open 5 m nearer the net is not taken: the sim's pass goes when its receiver is such a mate, else the carrier carries on | 30 m |
| C. `OwnZoneTurn` | `hk_own_zone_turn` | the carrier in its own zone whose move leads back towards its own end, with no standing opponent within 8 m ahead of it up the ice, turns its move up the ice | on |
| D. `SplitClump` | `hk_clump_frames` | a bot without the puck inside 1.5 m of a team-mate for that many frames gives way (the carrier, the human and the lower index keep their line) until 3 m apart | 45 frames |

`brain8=1` puts all four on (both sides), `brain8=0` takes them off; `ShipEvolution8` plays each one's `kEvo8Ship*` flag (all false). The brain gained
per-bot runs (`DownNear`, `ClumpRun` and their frames: brain state, not sim state); no field was added to skater or puck state.

**Before / after on the watch's 60 matches** (a match; each rule alone; `Game/logs/a202/w1.rows` against `wA*`, `wB*`, `wC`, `wD`):

| detector (a match) | main | A: leave 15 | A: leave 6 | **A: leave 6 + steering** | B: first form | **B: final** | **C** | **D** | brain8=1 (all four, final) |
|---|---|---|---|---|---|---|---|---|---|
| standing at a downed body > 30 f | 2.60 | 2.03 | 1.15 | **0.33** | 2.57 | 3.00 | 2.35 | 2.45 | 0.43 |
| pushing a downed body > 30 f | 1.10 | 0.85 | 0.53 | **0.12** | 1.05 | 1.13 | 1.18 | 0.98 | 0.22 |
| shot beyond 30 m with a mate open nearer | 2.55 | 2.58 | 2.42 | 2.77 | 2.03 | **0.65** | 2.52 | 2.78 | 0.43 |
| every shot beyond 30 m / in the attacking half | 11.25 / 6.05 | | | 11.83 / 6.40 | 10.62 / 5.92 | 9.58 / 5.97 | 12.18 / 7.15 | 11.82 / 6.50 | 10.12 / 6.18 |
| goal from beyond 30 m | 0.08 | 0.27 | 0.13 | 0.23 | 0.18 | 0.22 | 0.17 | 0.15 | 0.20 |
| own-zone carry, open ice ahead | 2.68 | 3.07 | 2.68 | 2.73 | 2.88 | 2.63 | **0.05** | 3.02 | 0.05 |
| two mates inside 1.5 m > 90 f | 1.37 | 1.53 | 1.45 | 1.37 | 1.25 | 1.30 | 1.37 | **0.25** | 0.35 |
| the slot empty, puck below the circles | 0.50 | 0.48 | 0.13 | 0.35 | 0.43 | 0.45 | 0.15 | 0.35 | 0.22 |
| idle > 3 s / boards / net | 0.77 / 0.22 / 0.20 | | | 0.93 / 0.28 / 0.20 | | 0.80 / 0.18 / 0.22 | 0.62 / 0.15 / 0.18 | 0.77 / 0.35 / 0.23 | 0.88 / 0.08 / 0.25 |
| circling (net turn) | 0.10 | 0.08 | 0.07 | 0.07 | 0.12 | 0.20 | 0.05 | 0.13 | 0.05 |

The goals from beyond 30 m are 5 in 60 matches on main and 8 to 16 with a rule on: a count this small moves with any change of the match (every rule
re-rolls the matches after its first act); the cells' goals against (section 3's rows) are the number to read.

**The six cells** (600 paired seeds, the ladder's instrument; B = the baseline row, build A = main; win % of the scripted player, the difference from B, and
the bots' goals a match; FA = rule A both sides, FAm = rule A on the human's mates only; a paired 600-seed difference carries about 2.7 points):

| arm | rk-reg | pro-reg | as-reg | rk-large | pro-large | as-large |
|---|---|---|---|---|---|---|
| B | 71.2 (ga 1.45) | 46.7 (ga 2.13) | 30.2 (ga 2.49) | 80.5 (ga 1.32) | 56.5 (ga 2.13) | 26.7 (ga 3.69) |
| FA | 73.2 (+2.0; ga 1.45) | 52.5 (+5.8; ga 1.99) | 34 (+3.8; ga 2.39) | 83.3 (+2.8; ga 1.24) | 60.2 (+3.7; ga 1.97) | 38 (+11.3; ga 3.25) |
| FAm | 70.3 (-0.9; ga 1.52) | 50.5 (+3.8; ga 2.06) | 29.7 (-0.5; ga 2.5) | 82 (+1.5; ga 1.27) | 50.5 (-6.0; ga 2.25) | 27.3 (+0.6; ga 3.58) |
| FB | 70.2 (-1.0; ga 1.5) | 47.5 (+0.8; ga 2.13) | 33.2 (+3.0; ga 2.4) | 80.8 (+0.3; ga 1.33) | 55.8 (-0.7; ga 2.15) | 27.2 (+0.5; ga 3.69) |
| FC | 68.7 (-2.5; ga 1.58) | 40.8 (-5.9; ga 2.3) | 21.3 (-8.9; ga 2.63) | 76.8 (-3.7; ga 1.48) | 46.2 (-10.3; ga 2.44) | 16.2 (-10.5; ga 4.28) |
| FD | 73.7 (+2.5; ga 1.49) | 46.8 (+0.1; ga 2.14) | 33 (+2.8; ga 2.42) | 83.5 (+3.0; ga 1.25) | 58 (+1.5; ga 2.11) | 30 (+3.3; ga 3.6) |

Reading: **A** makes the bots easier everywhere (a bot that goes round a downed man and leaves him loses the race to the puck at his body: all_star's big
ice +11.3); on the mates alone it is mixed (pro big ice -6.0: the human's mates leave the puck at a body to the opponents). **B** is inside 3 points in
every cell (a pass instead of a shot from far: as many goals against). **C** makes the bots stronger everywhere (a carrier that goes up the ice keeps
the puck instead of giving it away in its own corner: -5.9 to -10.5 at pro and all_star, goals against up). **D** is inside 3.3 points (a support that
gives way opens a lane; a little easier).

## 4. Do the bots brace? (item 3)

`opp.cross_hold_closing` at 4 and 8 m/s (600 seeds; B = main, where it is -1 at every level):

| cell | B | 4 m/s | 8 m/s | bots' braced hits a match (4 / 8) | the human's knockdowns (B / 4 / 8) |
|---|---|---|---|---|---|
| pro regular | 46.7 | 53.8 (+7.1) | 54.2 (+7.5) | 0.38 / 0.16 | 7.33 / 7.70 / 7.79 |
| all_star regular | 30.2 | 29.3 (-0.9) | 32.2 (+2.0) | 0.23 / 0.10 | 8.26 / 8.24 / 8.37 |
| pro large | 56.5 | 64.0 (+7.5) | 62.7 (+6.2) | 0.30 / 0.11 | 9.07 / 9.44 / 9.35 |
| all_star large | 26.7 | 29.2 (+2.5) | 30.7 (+4.0) | 0.29 / 0.08 | 9.85 / 9.77 / 9.85 |

A bot holding the brace gives up its poke and its lift while it holds (`StickPlay`'s press is the hold), and lands one braced hit in three to ten matches:
the bots get easier, not rougher. **Stays off.** The tell is real (a human sees a man run through others with a held stick and no bot ever doing it); the
brace that would read is a short one: held only inside the last 1.5 m of a closing at 6 m/s or more, the stick kept for a poke until then (next evolutions).

## 5. The proof

- **Hash: brain8=0 and brain8 at its default are main.** Build C (the final tree) against build A (main `58af359f` with the tracer test only), the
  ladder's instrument, 100 seeds a cell, six cells (three levels x two rinks): **600 of 600 identical with brain8=0 (`P8-*`), 600 of 600 with the
  default (`Q8-*`)** (hash, frames, goals, result; `same.mjs`, `out/proof.log`). The watch's two runs on main: count for count the same.
- **Gate**: the full `Jam` filter on build C in a clean environment (`jam.sh gateC Jam`, every `JAM_*` unset): **300 Success, 0 Fail**
  (`Game/logs/a202/gateC.results`; 295 on main + the five `Jam.Bot.Hockey8.*`). `node tools/gate/check-test-guards.mjs`: PASS. The Shipping target
  of build C's tree compiles (705.6 s).
- `Jam.Bot.Hockey8.*` (new, in the gate): `DownedLeave` (the chaser 1 m from a downed man with the puck at his body: off its move stays at him (-0.642, 0);
  with the rule, after the run, out (1.070, 0); before the run its own), `LongPass` (35 m out, a mate open 14 m from the net: off it charges the shot;
  on it passes; the mate no nearer: it shoots; the mate open but no pass lane: neither), `OwnZoneTurn` (moving back in its own zone with open ice: up
  the ice (-0.849, 0.424); an opponent 5 m ahead or the neutral zone: its own move), `Clump` (two mates 1 m apart: the higher index gives way after 45
  frames, the lower keeps its line), `Neutral` (each level ships evolution 8 as `kEvo8Ship*` say, all off; brain8=0 off and evolution 5's steering back
  at its own value; brain8=1 every rule at its value; the one-rule keys; pro seed 1 as shipped `a6978eaed2903df2` = brain8=0; brain8=1 `9f3fe7306257cf6f`).
- No field was added to skater or puck state (`Jam.Net.HashCoverage` untouched); the brain gained per-bot runs (brain state).
- Not measured: law 3's drill (`Bar.Curve.Law3`) with a rule on (the drill's carrier is the human; C reads only a bot carrier, A and D only bots
  without the puck): to read before C goes on.

## 7. What goes on (the lane's recommendation at 16:44; the Director's decision is section 10)

- **B, the long pass: on** (`kEvo8ShipLongPass = true`, or `opp.hk_long_pass_m=30;mate.hk_long_pass_m=30`): the shot from far with a mate open nearer
  falls from 2.55 to 0.65 a match and no cell moves more than 3 points.
- **D, the clump: on** (`kEvo8ShipClump`, `hk_clump_frames=45`): two mates glued together fall from 1.37 to 0.25 a match; every cell inside 3.3 points.
- **A, the downed body: on with the curve**, or with C (section 3's all-four row): the stand at a downed body falls from 2.60 to 0.33 a match and the
  push from 1.10 to 0.12 (the operator's own "Known to be unfinished" line), but alone it makes the bots 2 to 11 points easier.
- **C, the own-zone carry: on with the curve**, or with A: the retreat falls from 2.68 to 0.05 a match, but alone it makes the bots 2.5 to 10.5
  points harder.
- **The brace: off** (section 4).

## 8. Findings outside the evolution

1. `tools/perf/results/a193/curve.mjs` (and every copy before it), `worker()`: a worker that waits on free memory (`freeGB() < 14`) takes
   `jobs[next++]` after another worker took the last job: `TypeError: undefined is not iterable` and the cell's row is never written (FA-pro-large,
   16:00, during a Shipping build). Fixed in `a202/curve.mjs` (break when no job is left); the shared driver `tools/perf/results/a95/curve.mjs`
   should take the same line.
2. Evolution 7's open findings (the plain lead at 8 m/s, `JamAICore.cpp` `D / 8.0`; the mate planning its arrival at `SpeedScale`): not touched
   (no build slot was left without stopping the measurements); next evolutions.

## 10. Shipped (the Director, 16:52: all four on; the curve lane sets the ladder once on the sum, with A200's wind-ups)

- `kEvo8Ship*` true; `JamBaseline::BrainTune` carries `brain8=0`; every older switch `brain2=0` ... `brain7=0` now also switches evolution 8 off
  (as lane A193 did for evolution 7): the neutral-block replays (`Jam.Core.ContactNeutral`, `Jam.Core.MovementNeutral`, `PreA95Tune`'s `brain2=0`)
  give back main again. Tests that read evolution 5 / 6 at neutral now name `brain8=0`; `Jam.Bot.Hockey8.Neutral`: as shipped = `brain8=1`.
  `Jam.Bot.BoostPickupsBundle` (`AI/JamSystemsTests.cpp`) plays its seed with `brain8=0` (with the rules on seed 1 re-rolls and no bot happens to take
  a pickup; the test pins the pickups' path, not the brain).
- **Law 3 with all four on** (200 seeds a number, wind-ups 4 / 4): pro 8 m 94.0, 8.5 m 99.5, 9 m 93.0, 9.5 m 83.5, the straight charge 0.0; all_star
  99.5 / 99.5 / 99.0 / 93.0 / 0.0; with `brain8=0` the same to the run (no rule touches the drill's defender against the human carrier).
- **The watch with all four on** (60 matches, as shipped): standing at a downed body 2.60 -> 0.43, pushing 1.10 -> 0.22, the own-zone carry 2.68 -> 0.05,
  the clump 1.37 -> 0.35, the long shot with a mate open 2.55 -> 0.43, the empty slot 0.50 -> 0.22, the boards 0.22 -> 0.08; up: a bot at a net
  0.20 -> 0.25 (marathon all_star s2 f11680: 282 f; large all_star s1 f8772: 241 f), idle 0.77 -> 0.88 (the backs in their crease), goals from beyond
  30 m 0.08 -> 0.20 (5 -> 12 in 60 matches). No new detector count above 0.3 a match.
- **The six cells with all four on (as shipped, F8), 600 a cell, against main**: the sum makes the bots harder (C outweighs A), most on the big ice:

| arm | rk-reg | pro-reg | as-reg | rk-large | pro-large | as-large |
|---|---|---|---|---|---|---|
| B | 71.2 (ga 1.45) | 46.7 (ga 2.13) | 30.2 (ga 2.49) | 80.5 (ga 1.32) | 56.5 (ga 2.13) | 26.7 (ga 3.69) |
| F8 | 66.8 (-4.4; ga 1.66) | 44.2 (-2.5; ga 2.21) | 27.5 (-2.7; ga 2.51) | 79.2 (-1.3; ga 1.51) | 47 (-9.5; ga 2.48) | 20.5 (-6.2; ga 4.2) |

## 11. Evolution 7's open findings (item 4): built off, with rows, for after the freeze

- `FJamHockeySense::bTrueLead` (`hk_true_lead`): the plain lead to a moving puck (`Attack`'s `D / 8.0` and the intercept's fallback) reads the chaser's
  free top speed (`FreeSpeed x SpeedScale`) instead of Godot's 8 m/s. Off = 8.0, the same division. On top of the shipped game (F8), 600 a cell:

| arm | rk-reg | pro-reg | as-reg | rk-large | pro-large | as-large |
|---|---|---|---|---|---|---|
| B | 71.2 (ga 1.45) | 46.7 (ga 2.13) | 30.2 (ga 2.49) | 80.5 (ga 1.32) | 56.5 (ga 2.13) | 26.7 (ga 3.69) |
| F8 | 66.8 (-4.4; ga 1.66) | 44.2 (-2.5; ga 2.21) | 27.5 (-2.7; ga 2.51) | 79.2 (-1.3; ga 1.51) | 47 (-9.5; ga 2.48) | 20.5 (-6.2; ga 4.2) |
| G1 | 72.2 (+1.0; ga 1.55) | 39.3 (-7.4; ga 2.29) | 24.3 (-5.9; ga 2.57) | 78 (-2.5; ga 1.47) | 48.7 (-7.8; ga 2.5) | 19.5 (-7.2; ga 4.25) |

  Against F8: rookie reg +5.4, pro reg -4.9, all_star reg -3.2, rookie large -1.2, pro large +1.7, all_star large -1.0: off (a ladder change).
- `bMateTrueScale` (`hk_mate_true_scale`): the yield's arrival race plans the human's mate at `MateSpeedScale`. **Dead as shipped**: the race runs only
  with `YieldArriveS >= 0`, and every level ships `kEvo3YieldArriveS = -1`; not measured.
- Proof: build F (both off) against build E's shipped game, 100 seeds a cell, six cells: 600 of 600 identical; full `Jam` filter 300 / 0; Shipping compiles.

## 9. How to repeat

```
cd tools/perf/results/a202
./watch.sh <tag> "seeds=6;tune=opp.hk_own_zone_turn=1|mate.hk_own_zone_turn=1"     # the watch (Game/logs/a202/<tag>.rows)
./watch.sh <tag> "carts=hockey.arena.large;levels=all_star;seeds=1;from=5;dump=1830:2270:15"   # the stills of one moment
W=2 ./cells.sh <prefix> 600 "<tune>" ["pro as rk"]                                  # six cells; rows in cells.jsonl
node rows.mjs <prefix> ...                                                          # rows against B-*, the bots' braced hits
node same.mjs <labelA> <labelB>                                                     # shared seeds compared (hash, frames, goals, result)
./jam.sh <tag> Jam                                                                  # the gate's filter, clean environment
```
