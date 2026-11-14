# BRAIN-7: the bots know how fast everyone now is (lane A193, track Q2, evolution 7; Sat 14 Nov 13:50-)

Follows `BRAIN-6.md` (lane A181) and the speed lane's `MOVEMENT-10.md` (lane A185: a skater without the puck has `skater.free_speed_mult` 1.15 more
top speed and thrust; the carrier none). Tree: main `7d944ee1`. Source: `Game/Source/JamCore/AI/JamAICore.h/.cpp`, `DifficultyProfile.h/.cpp`,
`DifficultyResolve.cpp`, `JamSimAdapter.cpp`, `JamBotMatch.h/.cpp`, `JamPlaytester.cpp`, `Public/JamBaseline.h`; tests `Tests/HockeyBrain7Tests.cpp`
(`Jam.Bot.Hockey7.*`). Drivers and rows: `tools/perf/results/a193/` (`cells.jsonl`; raw matches under `raw/`, not landed). Logs: `Game/logs/a193/`.
Status with every step: `status/A193.md`.

## 1. Where the brain used a skater's speed

Every speed the brain planned with came from one number, `FJamAIParams::InterceptSpeed` = `skater.speed` (11 m/s on hockey.regular), set by
`DifficultyResolve.cpp` (the game, the lobby) and by `JamBotMatch.cpp` / `JamPlaytester.cpp` from `FJamMatchConfig::Speed` (the adapter's
`C.Speed = G.Skater.Speed`). The sim gives a free skater 12.65 m/s gliding and 14.55 sprinting; the carrier 10.12 / 10.93. **The brain reads no
acceleration anywhere**: `ArriveSec` and `LoosePuckOwner` model a turn time and then a constant speed; `C.Accel` feeds only the reference sim
(`JamBotRefSim.cpp`). The scripted human (`JamPlaytester.cpp`) has no speed model of its own: it reads velocities.

| place (`JamAICore.cpp` on main) | what it estimates | whose speed | before | with `hk_true_speed` / `hk_true_gap` |
|---|---|---|---|---|
| `:608` the yield's arrival race (`Hy.YieldArriveS`, the human's mates) | who reaches a loose puck first: the human or his mate | both free skaters | skater.speed | x free_speed_mult (`hk_true_speed`) |
| `:627` own pass (`LoosePuckOwner`) | which team-mate reaches the passer's loose pass first | free | skater.speed x speed_scale | x free_speed_mult (`hk_true_speed`) |
| `:1915` the attack's loose-puck owner (`AssignJobs`, `LoosePuckOwner`) | who of a five goes to a loose puck | free | skater.speed x speed_scale | x free_speed_mult (`hk_true_speed`) |
| `:1499` the backcheck point (`BackcheckPoint`) | where a chaser meets a carrier on its lane to the net | the chaser free; the carrier at its measured velocity | skater.speed x speed_scale | x free_speed_mult (`hk_true_speed`) |
| `:1515` the chase intercept (`bIntercept`) | the earliest point where the bot meets a moving puck | the chaser free | skater.speed x speed_scale | x free_speed_mult (`hk_true_speed`) |
| `:2051` the Press job's gap, `:700` the three's chaser's gap | how fast the presser backs off with the carrier (Move = the carrier's velocity over the presser's top speed) | the presser free | over skater.speed | over skater.speed x free_speed_mult (`hk_true_gap`) |
| `:1509` / `:1533` the plain lead | the time to the puck when there is no intercept solve | (hard-coded 8 m/s, Godot's speed) | 8 m/s | unchanged (a finding, section 6) |
| `ReadSpace`, the check's wind-up lead, the stop-and-go reads, `JamPlaytester.cpp` | closing speeds, arrival on the lane | measured velocities | | no planned speed: unchanged |
| the carrier | nowhere planned: every read of a carrier uses its measured velocity | | | |

## 2. The rule as built

- `FJamAIParams::FreeSpeedMult`: the cartridge's `skater.free_speed_mult`, filled wherever `InterceptSpeed` is (`DifficultyResolve.cpp`,
  `JamBotMatch.cpp`, `JamPlaytester.cpp`, through the new `FJamMatchConfig::FreeSpeedMult` from `ToMatchConfig`). A sim fact, not a tune.
- `FJamHockeySense::bTrueSpeed` (tune `opp.hk_true_speed`, `mate.hk_true_speed`): the five estimates of rows 1 to 5 above use
  `FJamBrain::FreeSpeed(H)` = skater.speed x free_speed_mult. Off: exactly `InterceptSpeed` (x 1.0: the same double).
- `FJamHockeySense::bTrueGap` (tune `opp.hk_true_gap`, `mate.hk_true_gap`): the Press job's and the three's chaser's retreat term divides by
  `FJamBrain::GapSpeed(H)` = the presser's true top speed. **Built, measured, ships off** (`kEvo7TrueGap = false`): section 4.
- `SetEvolution7(H, bOn)`; `ApplyDifficultyProfile` calls it on both sides with the level's `FJamDifficultyProfile::bTrueSpeeds`: **true at pro and
  all_star, false at rookie** (rookie keeps the old planning: its matches are main's, hash for hash; section 3). `brain7=0` switches evolution 7 off on
  both sides; every older switch (`brain6=0` ... `brain2=0`) switches it off too; `brain7=1` puts the shipped values on at any level.
- `JamBaseline::BrainTune()` carries `brain7=0` (with MakeNeutral's free_speed_mult 1 the rule reads nothing anyway).
- No field was added to skater or puck state; the brain gained no state.

## 3. The cells (the ladder's instrument, `JAM_PLAYTESTER_CHECK=closing`, 600 paired seeds; regular = hockey.regular, large = hockey.arena.large)

Before = build 0 (this worktree's main, no edit). Full = true speed and true gap at every level (build 2). **As shipped** = true speed at pro and
all_star (build 3; rookie as shipped is the before column: build 4's rookie is 200 of 200 hash-identical to build 0). Targets 85 / 48 / 26.

| cell | before | full rule | true speed only | **as shipped** | its distance from the bar, before -> as shipped |
|---|---|---|---|---|---|
| rookie regular | 67.8 | 68.3 (+0.5) | 70.8 (+3.0) | **67.8** (rookie off) | -17.2 -> -17.2 |
| pro regular | 53.0 | 44.7 (-8.3) | 43.5 (-9.5) | **43.5** | +5.0 -> -4.5 |
| all_star regular | 37.3 | 28.2 (-9.1) | 29.2 (-8.1) | **29.2** | +11.3 -> +3.2 |
| rookie large | 84.0 | 82.2 (-1.8) | 82.2 (-1.8) | **84.0** (rookie off) | -1.0 -> -1.0 |
| pro large | 52.7 | 50.8 (-1.9) | 50.8 (-1.9) | **50.8** | +4.7 -> +2.8 |
| all_star large | 34.7 | 33.0 (-1.7) | 33.0 (-1.7) | **33.0** | +8.7 -> +7.0 |

On the large rink (three a side) the full rule and true speed alone are the same matches: the gap is a five's Press job (the three's chaser plays it
only with `hk_press_three`, off). A paired 600-seed difference carries about 2.7 points.

**What the bots do differently** (a match; before -> as shipped):

| cell | bots' landed body checks | bots' landed cross-checks | his knockdowns | knocked down carrying | his passes / received | shots for / against | goals for / against | his breakaways / payoff goals |
|---|---|---|---|---|---|---|---|---|
| pro regular | 10.75 -> 11.59 | 0.50 -> 0.49 | 6.75 -> 7.32 | 2.92 -> 3.25 | 10.2/4.2 -> 10.5/4.3 | 5.9/12.1 -> 5.8/13.2 | 2.10/1.97 -> 1.94/2.20 | 2.18/0.66 -> 2.15/0.58 |
| all_star regular | 11.91 -> 11.76 | 0.48 -> 0.41 | 8.66 -> 8.51 | 3.78 -> 3.72 | 12.6/4.9 -> 12.6/4.8 | 6.2/18.2 -> 5.5/18.6 | 1.77/2.31 -> 1.60/2.52 | 1.81/0.43 -> 1.72/0.40 |
| pro large | 15.90 -> 15.82 | 0.79 -> 0.74 | 9.09 -> 9.34 | 3.66 -> 3.64 | 2.0/3.2 -> 1.8/3.1 | 4.9/13.3 -> 5.3/13.5 | 2.14/2.02 -> 2.16/2.08 | 4.36/0.89 -> 4.37/0.89 |
| all_star large | 14.79 -> 15.28 | 0.55 -> 0.63 | 9.60 -> 10.03 | 3.90 -> 4.20 | 2.3/2.5 -> 2.4/2.6 | 6.3/18.9 -> 6.1/19.1 | 2.42/3.31 -> 2.34/3.34 | 4.55/0.73 -> 4.58/0.76 |

The bots' completed passes are not a field of the playtest row (nor of this lane's family). The change reads as the opposing side getting to loose
pucks and to the human's carry sooner: shots against up, the human's goals down, at pro more landed checks and more knockdowns of the carrier.

**Why rookie is off.** With true speed at rookie the regular rink read 70.8 (+3.0) and the large 82.2 (-1.8), and `Jam.Bot.SeasonTakes` failed
(take-2, The Big Room, a rookie timer cut: the scripted human won 1 of 3; the gate needs 2). Rookie is 17 points under its bar after the speed rule;
its bots misjudging a free skater is in the level's favour, and keeping rookie's matches exactly main's keeps the gate and the curve lane's rookie
baseline where they are. `brain7=1` or `opp.hk_true_speed=1;mate.hk_true_speed=1` plays it at rookie.

## 4. Law 3's row (`Bar.Curve.Law3`, the drill of `Jam.Core.Movement.StopAndGo`, 200 seeds a number; the stop begun at 8 / 8.5 / 9 / 9.5 m; the straight charge)

| level | arm | 8 m | 8.5 m | 9 m | 9.5 m | straight |
|---|---|---|---|---|---|---|
| pro | brain7=0 (= main) | 94.0 | 99.5 | 94.5 | 78.0 | 0 |
| pro | **as shipped** (true speed) | **94.0** | **99.5** | **93.5** | **82.5** | **0** |
| pro | full (with the true gap) | 81.5 | 98.0 | 100 | 94.5 | 0 |
| pro | the true gap alone | 80.0 | 97.5 | 100 | 90.5 | 0 |
| all_star | brain7=0 (= main) | 99.5 | 100 | 99.5 | 96.0 | 0 |
| all_star | **as shipped** | **99.5** | **99.5** | **99.5** | **93.5** | **0** |
| all_star | full | 86.0 | 99.0 | 100 | 100 | 0 |
| rookie | as shipped = brain7=0 | 100 | 100 | 100 | 100 | 0 |

**The gate holds as shipped** (pro: the stop at 8 m 94 %, the straight charge 0). **The true gap breaks it** (pro 8 m 80 to 81.5 %): a presser that
no longer backs off faster than the carrier comes holds a smaller gap, so a stop begun at 8 m is begun too late and the window moves back out to 9 to
9.5 m, where it was before the speed rule (MOVEMENT-10: 8 m 100, 9.5 m 97). That is the speed lane's and the curve lane's choice (drill 21's
distance, the stop line's far edge), not this lane's: `opp.hk_true_gap=1` is the key that plays it, and it is worth about 1 more point at all_star and
a 1-point move at pro (full against true speed alone: 28.2 against 29.2, 44.7 against 43.5).

## 5. The proof

- **Hash: `brain7=0` is this main.** Build 2 and build 4 (the final tree), the ladder's instrument, 100 seeds a cell, the six cells (three levels x
  two rinks) against build 0 (the unedited worktree): **600 of 600 identical** on each build (hash, frames, goals, result; `same.mjs B-<cell> N-<cell>`
  and `N4-<cell>`). Rookie as shipped on build 4 against build 0: 200 of 200. Pro and all_star as shipped on build 4 against build 3's measured cells:
  400 of 400.
- **Gate**: the full `Jam` filter on build 4 in a clean environment (`tools/perf/results/a193/jam.sh`: every `JAM_*` variable unset): **295 PASS,
  0 FAIL** (`Game/logs/a193/gate4.results`). (Build 3's run, rookie on: 293 / 2: the take-2 rookie coin above and a test geometry of this lane's.)
- `node tools/gate/check-test-guards.mjs`: PASS. The Shipping target compiles (build 3's and build 4's trees).
- `Jam.Bot.Hockey7.*` (new): `Speeds` (11 -> 12.65 m/s with the rule; 11 exactly off and with free_speed_mult 1), `Moves` (the chaser of a crossing
  puck moves (-1.065, 0.107) planning at skater.speed, (-1.064, -0.114) at the true speed; the presser 10 m goal-side backs off less with the true
  gap; free_speed_mult 1 changes neither), `Neutral` (the profile per level, brain7=0 and every older switch, the one-rule keys; pro seed 1: as
  shipped `b109eb2ecafcf8b7`, brain7=0 `d96e0878c1f0fc47` (= build 0's seed 1); with free_speed_mult 1 the two are the same match `f64f387c082832ee`).
- `Jam.Bot.Hockey4/5/6.Neutral` compare their switch with `brainN=0;brain7=1` (each older switch now also switches evolution 7 off).

## 6. The breakaway (the brief's item 5: observed, not changed)

The breakaway burst (0.92 x 1.2 = 1.104 of base) is slower than a free skater gliding (1.15). What the brain does with such a carrier: the opposing
side has no backcheck rule as shipped (`hk_backcheck_mps` 0 on that side): a chaser behind a breakaway carrier is a puck chaser, and its intercept
solve (row 5) aims at the puck's path at its own speed; planning at 12.65 m/s instead of 11 it finds an earlier meeting point and aims nearer the
puck (the reading of the code; no trace of a breakaway was taken). In the cells: the human's
breakaways a match do not move (pro 2.18 -> 2.15, all_star 1.81 -> 1.72), the goals from them fall (payoff goals pro 0.66 -> 0.58, all_star
0.43 -> 0.40) and at pro he is knocked down carrying more (2.92 -> 3.25 a match). A carrier the chaser will always catch is caught: a breakaway
is now a race the human loses unless he shoots early. The fix is the movement lane's (`skater.breakaway.speed_bonus` 0.2 -> 0.3: 1.196 > 1.15);
the brain's side, if the operator wants a breakaway to stay a breakaway with the bonus as it is: the chaser of a breakaway carrier gives up the
intercept for the backcheck point (row 4) or for the stick from behind only (both exist; a level's switch).

## 7. Findings outside the evolution

1. `JamAICore.cpp:1509` and `:1533`: the plain lead (`D / 8.0`) and the intercept's fallback assume 8 m/s, Godot's speed; the rink's base is 11, a free
   skater 12.65. A bot without `bIntercept` leads every moving puck as if it were 37 % slower than it is.
2. `JamAICore.cpp:608`: the human's mate plans its own arrival at `SpeedScale` (all_star 1.45), but it skates at `MateSpeedScale` (1.0 at every level,
   lane A112): at all_star the mate thinks it is 45 % faster than it is in the race for a loose puck with the human, and keeps pucks it should leave.
3. The opposing side's `RedSkill` (pro 1.07, large 1.15) scales its move (`JamAICore.cpp:353`) and no estimate reads it.
4. The sim's pass lead (`JamSim.cpp:2719`, `pass_lead_frames` x the receiver's velocity) uses the receiver's measured velocity: no planned speed; no
   change needed.

## 8. How to repeat

```
cd tools/perf/results/a193
node curve.mjs <label> <novice|regular|good> <rookie|pro|all_star> <regular|large> 600 "<tune>" --workers 2 --check closing   # one cell; row in cells.jsonl
node same.mjs <labelA> <labelB>                                                   # the shared seeds compared (hash, frames, goals, result)
JAM_LAW3_AT="8,8.5,9,9.5" ./law3.sh <tag> "pro:-|pro:brain7=0|all_star:-"          # law 3's rows (Game/logs/a193/<tag>.txt)
./jam.sh <tag> Jam                                                                # the gate's filter, clean environment
```
