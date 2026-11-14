# Movement 10: skaters without the puck are 15 % faster (lane A185, track F2, evolution 9, 2026-11-14)

The operator after playing H31 (R-0064 point 2): "Skaters without puck should be 15% faster than now". Every skater without the puck, human and bot, at
every level (the crease skater too: it is a skater the brain plays), now has 15 % more top speed and 15 % more thrust, in a glide and in a sprint; the
carrier skates exactly as before, so it is 15 % slower against everyone. Status with timestamps: `status/A185.md`. Drivers and rows:
`tools/perf/results/a185/` (`cells.jsonl`, `raw/`); law 3's and the traces' logs: `Game/logs/a185/` (not landed). The main it stands on: 943df8e7.

## 1. The rule as built

**One key, `game.skater.free_speed_mult`, 1.15 (range 1..1.5; 1 = the neutral value, the game before the lane).** In `UJamSim::StepSkater` a skater
that is not `CarrierIndex()` multiplies its top speed and its acceleration by the key; nothing else reads it. Why a key and not the cartridges: of the 30
shipped cartridges only two (Marathon, Power Play) write `carry_speed_mult`; scaling `max_speed` and `sprint` up and `carry_speed_mult` down would have
edited all 30, `hockey.regular` among them (its hash pins the Designer's 36 cached cuts), and would have moved every number that reads `skater.speed`
(the stability's speed scale, the spin's cap, the brain's reach estimates, the carrier's sprint cap). The key leaves all of them where they were.

| | before | after |
|---|---|---|
| free skater, glide (base 11 m/s) | 10.998 m/s | **12.648** (x 1.1498) |
| free skater, sprint | 12.648 | **14.545** (x 1.3223 of base) |
| carrier, glide | 10.1176 | **10.1176** (the scene's state hash identical) |
| carrier, sprint | 10.9272 | **10.9272** (identical) |
| time to 95 % of its own top (free) | 36 frames | **36 frames** |

**Acceleration: the same time** (built). Thrust x 1.15 with top x 1.15 reaches the higher top in the same 36 frames: that reads as speed. The other
reading, the same distance, needs thrust x 1.3225 (1.15 squared); not built.

The carrier's sprint cap (`CarrierSprintCap` 0.97 x (1 + sprint.bonus)) is still a fraction of the base, so no cartridge can give the carrier more than
1.115 of the base against a free sprint of 1.3225. **Law 4** (a sprinting carrier's puck is easier to take) is unchanged in its rules (reach, sweep
shield); the speed a sprinting defender closes on a sprinting carrier grew from 1.15 - 0.993 to 1.3225 - 0.993 of the base.

Every shipped cartridge has the rule: `Jam.Core.Movement.FreeSpeedShipped` lists the folders at test time (the six `hockey.*` cuts, `modes/` twelve,
`ladder/` twelve takes and fallbacks; the `hockey.test.*` rigs and `modes/parked/` are not shipped) and asserts each loads to 1.15: **30 of 30**.

## 2. Law 3: the stop's window

`Bar.Curve.Law3`, 200 seeds a number, the drill of `Jam.Core.Movement.StopAndGo`. New in that Bar test: `JAM_LAW3_FREE` (the key for the run) and
`JAM_LAW3_AT` (the distances), so both arms ran on one build. The neutral arm (key 1) gave the before rows (measured on the morning build) to the digit.

| level | arm | 7 m | 8 m | 8.5 m | 9 m | 9.5 m | 10 m | 11 m | 12 m | straight |
|---|---|---|---|---|---|---|---|---|---|---|
| rookie | before | 100 | 100 | 100 | 100 | 100 | 100 | 100 | 100 | 0 |
| rookie | **after** | 91 | 100 | 100 | 100 | 100 | 100 | 96 | 94.5 | 0 |
| pro | before | 66 | 100 | 100 | 99.5 | 97 | 96 | 86 | 61.5 | 0 |
| pro | **after** | 38 | 94 | 99.5 | 94.5 | **78** | 72 | 59 | 59 | 0 |
| all_star | before | 97.5 | 100 | 100 | 99 | 93 | 86 | 58 | 39 | 0 |
| all_star | **after** | 85 | 99.5 | 100 | 99.5 | 96 | 91 | 63.5 | 32 | 0 |

**The window at pro came in and narrowed: 90 % or more from 8 to 9 m (8 to 10 m before); 8.5 m is its centre (99.5 %).** A defender who closes 15 %
faster makes a stop begun at 9.5 m too early (he is still coming when the carrier goes) and one begun at 7 m too late. The straight charge still never
passes. Law 3's gate bars (8 m at least 80 % at pro and all_star, the straight charge at most 20 %) hold: 94 / 99.5 and 0 / 0.

**"Five stick lengths" holds**: 8 m (94 % at pro, 100 at rookie, 99.5 at all_star) and 8.5 m both read FIVE; drill 21's distance stays 8 m.

**The stop line's far edge: 9.5 -> 9.0 m** (`JamCarrierMarks.h` `FStopRule::LitFarM`, built): a stop begun on the light by a hand that takes the
quarter second begins at the far edge; at 9 m it passes a pro 94.5 % (93.5 % in StopAndGo's own scene), at 9.5 m 78 %. All_star reads 99.5 % at 9 m.

## 3. The cells (the ladder's instrument, `JAM_PLAYTESTER_CHECK=closing`, 600 paired seeds)

Before = the morning build of 943df8e7, measured before any build of this lane; after = this tree (the stop line has no part in a cell).

| cell | before | after | change | knockdowns a match | human carry s a match | carries of 3 s+ | time down % of play | human passes / received | shots for / against |
|---|---|---|---|---|---|---|---|---|---|
| rookie regular | 85.3 | **69.3** | -16.0 | 3.18 -> 3.57 | 28.5 -> 29.6 | 2.96 -> 2.67 | 6.5 -> 6.8 | 5.3/2.4 -> 7.1/3.4 | 4.2/12.7 -> 3.9/17.4 |
| pro regular | 50.5 | **53.3** | +2.8 | 6.87 -> 7.05 | 23.5 -> 27.3 | 1.46 -> 1.22 | 14.9 -> 14.6 | 7.7/3.5 -> 10.8/4.5 | 5.4/10.1 -> 6.4/12.6 |
| all_star regular | 29.8 | **36.3** | +6.5 | 8.57 -> 8.77 | 21.4 -> 23.9 | 0.57 -> 0.55 | 18.7 -> 19.3 | 10.7/4.1 -> 13.2/4.8 | 5.6/17.7 -> 6.2/18.4 |
| rookie large | 83.7 | **80.8** | -2.9 | 3.77 -> 3.64 | 37.8 -> 35.4 | 4.52 -> 4.47 | 7.4 -> 8.8 | 0.4/1.2 -> 0.9/1.5 | 4.4/10.0 -> 4.1/11.7 |
| pro large | 46.7 | **51.2** | +4.5 | 7.73 -> 9.23 | 29.5 -> 32.1 | 2.88 -> 2.91 | 14.0 -> 17.8 | 1.4/2.5 -> 2.0/3.1 | 3.8/11.9 -> 4.9/13.0 |
| all_star large | 25.3 | **38.7** | +13.4 | 9.48 -> 9.95 | 25.8 -> 30.0 | 2.25 -> 2.63 | 18.7 -> 19.5 | 1.6/2.0 -> 2.3/2.6 | 5.9/14.9 -> 6.9/18.1 |

**Neutral identity: 3,600 of 3,600 matches the same** (hash, frames, goals, result; `node tools/perf/results/a185/same.mjs B-<cell> N-<cell>`, N =
this tree's build with `--set skater.free_speed_mult=1`). The brief's ladder (2,000-match cells: 85.8 / 83.8, 52.4 / 50.2, 28.8 / 26.1) and these
600-match before rows agree inside their intervals.

**It compresses the ladder.** Rookie regular falls 16 points (the novice's opponents arrive sooner: shots against 12.7 -> 17.4); all_star rises 6.5 /
13.4 (the good player is free faster too, and passes find him: received 4.1 -> 4.8). More than 4 points in four cells: by the operator's order the rule
ships ON; **the curve lane re-sets the ladder** on this table (targets 85 / 48 / 26, R-0064 point 11). Bots' completed passes are not a field of the
playtest row; the human's passes rose in every cell.

## 4. A carrier caught from behind, and the trace

`Bar.Movement.CarryTrace`, regular at pro, 100 matches, the shipped press (`JAM_CARRY_TRACE set=skater.free_speed_mult:1` for the before arm; the
stop line at 9.5 m in both arms):

| | key 1 | key 1.15 |
|---|---|---|
| the human's carries a match / median length | 16.6 / 0.88 s | 20.6 / 0.80 s |
| **carrying, knocked down from behind his facing** | **0.07 a match** (7 in 100) | **0.08 a match** (8) |
| knockdowns a match / frames down, % of play | 6.76 / 9.13 | 7.18 / 8.44 |
| "he braked in the last half second: the stop begun too late" | 29 | 53 |
| "no brake, the hitter inside 7 m a second earlier: no room" | 77 | 111 |
| contested ring lit a match / stop line lit a match | 13.2 / 12.3 | 21.1 / 15.0 |

Caught from behind stays rare: a chaser who comes from behind takes the puck with the stick far more than with the body. The stop begun too late
nearly doubles: the window narrowed (section 2).

## 5. Tests whose pinned speed or window moved

All in the gate's `Jam` filter, each line now carries the measured number:
- `Jam.Core.CarrierSpeed`: the carrier at carry_speed_mult / free_speed_mult of the free skater (0.8; was 0.92); "carry_speed_mult 1 skates as freely" with the key at 1.
- `Jam.Core.Movement.Burst` (b): both arms at the neutral block's free speed, so only the burst differs.
- `Jam.Core.Movement.ReverseBrake`: "8 m/s" is 8 m/s (key 1 in that scene; 9.2 m/s took 28 frames at 20 m/s^2).
- `Jam.Core.Contact.Fouls`: "a bump from behind at the same speed": the pusher's input / 1.15 (exactly the push before the lane).
- `Jam.Core.Movement.StopAndGo`: the far-edge bars at 9 m (90 % past: 93.5; 95 % kept; the mouse worth 10 points at 9 m: 71.5 against 93.5); 8 m at 90 % (94).
- `Jam.Core.Movement.SecondMove`: 8 m at 90 % (94); the drill's distance at every level 90 %.
- `Jam.Core.Movement.SpinPrice`: the spin from 6 m 45..70 % (48.5; 53 before); 8 m at 90 %.
- `Jam.Core.Movement.StopGrace`: the key's liveness on the first pro seed (3 to 12) whose match it changes (seed 5).
- `Jam.Core.Payoffs.Breakaway`: 3 frames and 0.5 m (4 frames, 0.74 m; 8 and 1.5 before): a free chaser at 1.15 is faster than the breakaway's 0.92 x 1.2 = 1.104.
- `Jam.Core.Movement.Marks` (JamCarryTests): lit at 12 m closing at 13 m/s (9 m + 3.25 m).
- Neutral replays, no pinned hash moved: `JamBaseline::MakeNeutral` (key 1), `JamMovementTests::MakeNeutral` and its spliced block, `Jam.Core.Contact.Neutral`.

New: `Jam.Core.Movement.FreeSpeed`, `Jam.Core.Movement.FreeSpeedShipped`.

## 6. Proof
- Full `Jam` filter in a clean environment (`tools/perf/results/a185/jam.sh`, every `JAM_*` variable unset): build 2 **275 pass, 0 fail**; build 3 (the far edge) in `status/A185.md`.
- The Shipping target compiles (on build 2's and build 3's trees).
- The hash: the default match moves (it is a rule); the neutral value gives back the previous game: 3,600 of 3,600 ladder matches and law 3's 18 rows.

## 7. Findings outside the family
- `Game/Source/JamGame/Private/HokejAcceptDrill.cpp:474`: the acceptance run's `sprint_carry` row compared the carrier's sprint with k=1's free walk x
  carry_speed_mult; with the free walk 15 % faster it would fail (10.93 < 11.64 + 0.2). One line: divided by free_speed_mult (needed for green).
- The brain's world (`Game/Source/JamCore/AI/JamSimAdapter.cpp:38`, `FJamMatchConfig::Speed`) knows the base speed only; it now underestimates every
  free skater by 15 % in its arrival estimates. Track Q2.
- The breakaway burst (`skater.breakaway.speed_bonus` 0.2) no longer outruns a gliding free chaser (1.104 against 1.15 of the base).

## 8. Next evolutions (ranked)
1. **The breakaway escapes again**: `skater.breakaway.speed_bonus` 0.2 -> 0.3 (0.92 x 1.3 = 1.196 > 1.15). Proof: `Jam.Core.Payoffs.Breakaway`
   back to 8 frames and 1.5 m; breakaway goals a match up at pro, the cells reported to the curve lane.
2. **The brain knows the new speed**: `FJamMatchConfig` carries free_speed_mult and the brain's reach and intercept estimates use it. Proof: a new
   row field for bots' completed passes, up at pro; the presser's arrivals inside the stop's window (law 3 at 9 m back to 97 % or more).
3. **The drill at the window's centre**: drill 21 and `StopAtForLevel` 8 -> 8.5 m (still FIVE stick lengths). Proof: the drill's pass rate at pro
   94 -> 99.5 %, `Jam.Core.Movement.SecondMove`'s 95 % bar restored.
