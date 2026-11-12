# Balancing: three AI difficulties (lane A3, H5-H6)

Status: 2026-11-12 12:30. All numbers come from the real `UJamSim` through `tools/sweep/run.mjs --runner ue` (commandlet `-run=JamTournament`), 20 seeds per cell.
The scripted human is A2's player bot (`bot_hockey.gd` port): it is the stand-in for a person, and its habits (long centre shots, no deking) shape every number here.

## What a level is

`game.ai.difficulty`: `rookie | pro | all_star`, absent = `pro`. A level sets how good the **other** team's bots are and how much room the bots give the human.
The human's two teammates keep A2's skill at every level (they also pass to the human at rookie). Resolution order: the level's profile -> the `game.ai`
numbers the cartridge writes -> the Designer's `values` (`ai_speed_scale`, `ai_red_skill`). Code: `Game/Source/JamCore/AI/DifficultyProfile.*` (data, engine-free),
`DifficultyResolve.*` (reads which keys a cartridge wrote, from `RawJson`), `UJamBrain::ApplyCartridge`. Every preset says `"ai": { "difficulty": "pro" }` and no numbers.

## Profiles (final)

| field | rookie | pro (= `FJamAIParams` defaults) | all_star |
|---|---|---|---|
| `speed_scale` (all bots) | 0.9 | 1.0 | 1.0 |
| `red_skill` (opponents' extra speed factor; never applied to a `back`) | 0.85 | 1.0 | 1.0 |
| teammates' yield margin / radius (m) | 4 / 25 | 2 / 15 | 1.5 / 12 |
| opponents' hold radius (m) | 0 | 2 | 0 |
| stall tolerance (frames) | 180 | 120 | 90 |
| opponents' reaction delay (frames, steering only; shots stay live; backs 0) | 2 | 0 | 0 |
| opponents' aim error (deg, max; per skater and 2-s window) | 3 | 0 | 0 |
| opponents' shot range (reach + m) | 13 | 16 | 30 |
| opponents' lead cap (frames) / intercept solve | 15 / no | 45 / yes | 45 / yes |
| opponents know the aim assist (`shot.aim_cone_deg`) | no | yes | yes |
| opponents' back blocks shots | only from >= 35 m | yes | yes |
| teammates pass to the human | yes | no | no |

At every level (R-0009): a `back` guards its net. When the puck is in its half or within 45 m of its net, it stands on the line from the goal centre to the puck,
at most 5 m out, damped so it stops on the post instead of sliding through. It runs at full speed and reacts without delay. Teammates honour the human's
team signals (R-0007; `FJamAIWorld::IntentKind/Target/FramesLeft`, copied in by `AHokejGameMode::GatherInputs`).

Two findings shaped the table:
- A steering delay of 2 frames or more stopped the opponents from ever winning the puck (100% human wins). Delaying the shot as well made the AI stop scoring
  at all, because a delayed shot misses the sim's reach and facing gate. Reaction is therefore small, and shots are never delayed.
- The opponents' hold radius (A2's 4.5 m) was the reason pro was 90% before this lane. With 0 the human touches the puck more (contested pucks), not less.

## Sweep table, final profiles (R-0010 rinks: regular 90 x 45 speed 11, large 120 x 60 speed 8), scripted human vs AI, seeds 1-20

| rink | level | W-L-unfinished | human win % | goals/match | GF | GA | shots human | shots mates | shots AI opp | human touches/min (min) | match s (max) | s/goal | faceoffs without a human touch |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| regular | rookie | 17-3-0 | **85** | 3.8 | 2.8 | 1.1 | 8.7 | 0.5 | 5.3 | 9.7 (6.2) | 271 (562) | 70 | 0 |
| regular | pro | 9-5-6 | **45** | 3.1 | 1.8 | 1.4 | 8.9 | 1.2 | 12.2 | 8.6 (6.6) | 399 (600) | 154 | 0 |
| regular | all_star | 6-14-0 | **30** | 4.0 | 1.5 | 2.5 | 5.1 | 2.4 | 14.3 | 9.6 (7.2) | 232 (383) | 58 | 0 |
| large | rookie | 7-1-12 | 35 | 2.6 | 1.8 | 0.9 | 7.7 | 3.0 | 9.6 | 6.2 (3.8) | 504 (600) | 238 | 0 |
| large | pro | 4-5-11 | 20 | 2.6 | 1.3 | 1.4 | 6.8 | 0.9 | 11.8 | 4.8 (3.5) | 514 (600) | 246 | 0 |
| large | all_star | 1-15-4 | 5 | 3.3 | 0.6 | 2.7 | 4.0 | 1.6 | 15.4 | 6.4 (4.5) | 405 (600) | 137 | 0 |

The target curve holds on the regular rink: rookie 85% (target 85), pro 45% (target 50, within 10), all_star 30% (target 25). The human touches the puck
at least 6.2 times per minute in every regular match, and never waits more than 600 frames for a touch after a faceoff (`HumanHasSomethingToDo` holds at every level).

Pure bot-vs-bot (`--mode brain`, the opposing side at the level):
- regular: pro 5-3-12, all_star 5-10-5.
- large: pro 0-0-20, all_star 1-0-19.

The guarding backs make bot-vs-bot on 120 x 60 nearly goalless (R-0003's "all_star should finish" no longer holds there).

## Rink size at pro (the operator's question; the operator ruled R-0010 in the meantime)

| rink | human win % | goals/match | match s | s/goal | human touches/min | unfinished in 10 min |
|---|---|---|---|---|---|---|
| 90 x 45, speed 11 (regular) | 45 | 3.1 | 399 | 154 | 8.6 | 6 / 20 |
| 120 x 60, speed 8 (large) | 20 | 2.6 | 514 | 246 | 4.8 | 11 / 20 |

The big rink is the slower and emptier game by every measure: goals take 60% longer, the human touches the puck about half as often, and more than half the
matches do not finish in 10 minutes. With backs that guard their net, the big rink also turns against the scripted human: it shoots from far, the back has time to set.
R-0010 (90 x 45 as regular) is what the numbers support. 120 x 60 needs its own profile, or a longer timer, before it is a fair cut.

## Feel proposals (lane A10, R-0009), each measured alone and all together

Measured on the 12:11 build, whose rookie did not block shots. Pro and all_star rows are the final profiles; rookie differs from the final table.

| variant | regular rookie / pro / all_star win % | regular touches/min (pro) | regular match s (pro) | large rookie / pro / all_star win % |
|---|---|---|---|---|
| baseline | 75 / 45 / 30 | 8.6 | 399 | 5 / 20 / 5 |
| `puck.drag` 0.5 | 100 / 100 / 80 | 12.4 | 236 | 90 / 80 / 40 |
| `skater.glide_drag` 0.3 | 80 / 50 / 15 | 8.8 | 417 | 5 / 15 / 0 |
| `skater.lateral_grip` 4 | 55 / 45 / 15 | 8.8 | 485 | 5 / 5 / 0 |
| `skater.accel` 16 | 90 / 70 / 60 | 10.6 | 319 | 5 / 25 / 15 |
| `shot.reach_m` 2.0 | 85 / 65 / 30 | 8.3 | 312 | 0 / 20 / 5 |
| all five together | 100 / 95 / 85 | 12.1 | 168 | 70 / 80 / 40 |

Reading: `puck.drag` 0.5 is the big one. Carrying becomes possible, the human touches the puck 45% more, and matches are 40% shorter. But it hands the scripted
human the game (pro 100%), so adopting it means re-running this sweep and raising every level (the `-aitune` hook makes that a few minutes).
`glide_drag` and `lateral_grip` barely move the curve; `accel` 16 and `reach` 2.0 help the human moderately. None of them breaks `HumanHasSomethingToDo` (0 faceoffs without a touch in every cell).

## How to reproduce

```
node tools/sweep/run.mjs --runner ue --seeds 1-20                       # the table above
node tools/sweep/run.mjs --runner ue --levels pro --tune-pro "hold_radius_m=1;opp.shoot_extra_m=18"   # try numbers without a rebuild
node tools/sweep/run.mjs --runner ue --set "puck.drag=0.5"              # a cartridge variant (validated, written to tools/sweep/out/)
node tools/sweep/run.mjs --runner ue --mode brain --levels pro,all_star # bot-vs-bot
```
