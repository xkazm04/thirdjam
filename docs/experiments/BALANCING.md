# Balancing: three AI difficulties (lane A3, H5-H6)

Status: 2026-11-12 12:40, final for R-0011.
The scripted human is A2's player bot (`bot_hockey.gd` port): it is the stand-in for a person, and its habits (long centre shots, no deking) shape every number here.

## What a level is

`game.ai.difficulty`: `rookie | pro | all_star`, absent = `pro`. A level sets how good the **other** team's bots are and how much room the bots give the human.
The human's two teammates keep A2's skill at every level (they also pass to the human at rookie). Resolution order: the level's profile -> the `game.ai`
numbers the cartridge writes -> the Designer's `values` (`ai_speed_scale`, `ai_red_skill`). Code: `Game/Source/JamCore/AI/DifficultyProfile.*` (data, engine-free),
`DifficultyResolve.*` (reads which keys a cartridge wrote, from `RawJson`), `UJamBrain::ApplyCartridge`. Every preset says `"ai": { "difficulty": "pro" }` and no numbers.

## Profiles (final, R-0011: every preset has `puck.drag` 0.5; no other feel change adopted)

| field | rookie | pro (= `FJamAIParams` defaults) | all_star |
|---|---|---|---|
| `speed_scale` / `red_skill` | 1.0 / 1.0 | 1.0 / 1.0 | 1.0 / 1.0 |
| teammates' yield margin / radius (m) | 4 / 25 | 2 / 15 | 4 / 25 |
| opponents' hold radius (m) | 0 | 0 | 0 |
| stall tolerance (frames) | 180 | 60 | 60 |
| opponents' reaction delay (frames, steering only; shots live; backs 0) | 0 | 0 | 0 |
| opponents' aim error (deg) | 0 | 0 | 0 |
| opponents' shot range (reach + m) | 13 | 30 | 40 |
| opponents' lead cap (frames) / intercept solve | 15 / no | 60 / yes | 60 / yes |
| opponents know the aim assist | yes | yes | yes |
| opponents hold a shot while a defender is in the lane | no | no | yes |
| opponents' back blocks shots | only from >= 35 m | yes | yes |
| teammates pass to the human | yes | no | no |

At every level (R-0009): a `back` guards its net. When the puck is in its half or within 45 m of its net, it stands on the line from the goal centre to the puck,
at most 5 m out, damped. It runs at full speed and reacts without delay. Teammates honour the human's team signals (R-0007: `FJamAIWorld::IntentKind/Target/FramesLeft`,
copied in by `AHokejGameMode::GatherInputs`).

What the sweeps taught:
- Reaction delay and aim error are cliffs. A 2-frame steering delay or a 3-degree aim error already gives the scripted human 90-100%, so neither is used.
  Levels differ by shot range, anticipation (lead, intercept), stall tolerance, lane discipline, shot blocking, and how much the human's own teammates leave the puck to the human.
- A2's 4.5 m hold radius was why pro was 90% before this lane. A contested puck also gives the human more touches, not fewer.
- A teammate yielding more (4 / 25 at all_star) makes the human's side weaker. That is why all_star shares rookie's yield numbers, for the opposite reason.

## Phase 2 brain (R-0013 point 3, R-0017 point 4, R-0019)

| behaviour | rookie | pro | all_star | where |
|---|---|---|---|---|
| poke check (blade aimed at the puck; pokes after the puck sat N frames inside blade reach) | never | N = 6 | N = 2 | `FJamBrain::StickPlay`, skill `PokeFrames` |
| stick lift at contact range | no | yes | yes | `StickPlay`, `bLift` |
| pass (`bPass`) when a defender is within 2 m and a teammate within 25 m has no opponent within 3 m | no | yes | yes | `StickPlay`, `bPassOut` |
| the support stands in the pass lane from the opposing carrier to its nearest teammate | no | yes | yes | `Decide`, `bReadLanes` |
| detour for a pickup within 8 m, nearer than the puck, never while carrying | no | no | yes | `Decide`, `bSeekPickups` |
| faceoff draw (opposing centre; the sim's own bot timing is 6-18 frames after the drop; the scripted human presses 12 frames after it) | jumps early in 45% of faceoffs (loses those draws) | sim timing | takes 40% of draws 8 frames after the drop | Think, `DrawJumpPct` / `DrawPressPct` |
| the scripted human forechecks: against an opposing carrier its blade follows the puck and it pokes once the puck is in reach | | | | `FJamPlayerBot::Think` |
| body checks | never at any level, so no check from behind and no check on a skater without the puck | | | `StickPlay` |
| last man as goalie: the carrier is inside our blue line (the outer third) and no teammate is within 2.5 m of the line from our net to the carrier: the `back` holds the crease 1.5 m out, squared to the carrier, and pokes at its level | holds, never pokes | holds, pokes | holds, pokes | `LastManGoalie` |
| the carrier steers round a slow-ice circle whose edge is within 3 m of its line to the net; others cut through | yes | yes | yes | `Attack`, `FJamAIWorld::SlowIce*` |
| never shoots from inside a no_shoot zone | yes | yes | yes | `StickPlay`, `FJamAISkater::bNoShoot` |

The human's teammates keep A2's skill: they never poke, lift or read lanes, and they pass to the human at rookie.

The game mode fills the inputs in `AHokejGameMode::GatherInputs`:
- pickups from `UJamEffectSystem::GetPickups`;
- slow ice and no_shoot from `UJamZoneSystem` (`GetZones`, `BlocksShot`);
- intents from `UJamSignalSystem`.

The headless tournament has none of these, so the sweep measures stick play, passing and the goalie rule only.

## Phase 2 rookie fix (15:36): speed 0.95, red 0.9, stall tolerance 60 frames, so rookie bots shoot in every match (SeasonTakes needs one AI shot). Rookie measured over 30 seeds: regular 83% (25-5-0), large 100%. Takes 1 and 2, seeds 1-7: every match has at least 1 AI shot, take-1 seeds 1, 2 and 7 won.

## Phase 2 sweep (15:15, gate binary of 15:00, rookie speed 0.85 / red 0.8, pro without pass-out; 20 seeds)

| rink | level | W-L-unf | human win % | goals/match | shots AI | human touches/min (min) | match s | faceoffs without a human touch |
|---|---|---|---|---|---|---|---|---|
| regular | rookie | 17-1-2 | **85** | 3.6 | 12.8 | 32.1 (19.6) | 284 | 0 |
| regular | pro | 9-11-0 | **45** | 4.4 | 18.6 | 44.8 (23.1) | 233 | 2 |
| regular | all_star | 6-14-0 | **30** | 4.3 | 18.7 | 40.5 (18.0) | 224 | 7 |
| large | rookie | 19-1-0 | 95 | 3.0 | 2.7 | 22.4 (14.3) | 195 | 0 |
| large | pro | 6-14-0 | 30 | 1.9 | 12.5 | 25.6 (9.8) | 232 | 5 |
| large | all_star | 8-11-1 | 40 | 2.0 | 9.7 | 24.6 (7.5) | 236 | 9 |

The curve holds on regular. Large misses at pro and all_star, and all_star still keeps the puck past 600 frames after some faceoffs.
The rookie early-jump draw knob changed nothing (90% gave the baseline numbers exactly); it doesn't take effect as built and is under investigation.
The tables below are the pre-phase-2 history (R-0011).

## Sweep table, final (regular 90 x 45 speed 11 first-to-3; large 120 x 60 speed 8 timer 3 min), scripted human vs AI, seeds 1-20

| rink | level | W-L-unfinished | human win % | goals/match | GF | GA | shots human | shots mates | shots AI opp | human touches/min (min) | match s (max) | s/goal | faceoffs without a human touch |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| regular | rookie | 18-2-0 | **90** | 3.5 | 2.9 | 0.7 | 8.2 | 0.1 | 3.7 | 14.7 (11.3) | 134 (302) | 36 | 0 |
| regular | pro | 9-11-0 | **45** | 4.1 | 2.0 | 2.0 | 6.8 | 0.5 | 12.8 | 15.0 (10.4) | 138 (245) | 32 | 0 |
| regular | all_star | 5-15-0 | **25** | 4.3 | 1.9 | 2.4 | 5.7 | 0.8 | 13.4 | 14.7 (11.3) | 131 (226) | 29 | 0 |
| large | rookie | 17-3-0 | 85 | 2.2 | 1.8 | 0.4 | 5.9 | 0.6 | 3.9 | 14.6 (11.0) | 196 (290) | 126 | 0 |
| large | pro | 9-11-0 | 45 | 3.4 | 1.6 | 1.8 | 4.9 | 1.9 | 16.9 | 13.4 (8.3) | 197 (309) | 72 | 0 |
| large | all_star | 5-15-0 | 25 | 4.0 | 1.5 | 2.5 | 5.3 | 1.1 | 15.1 | 13.5 (10.3) | 190 (212) | 55 | 0 |

The curve holds on both rinks: rookie 90 / 85, pro 45 / 45, all_star 25 / 25 (targets 85 / 50 / 25, slack 10). The human touches the puck at least 8.3 times per
minute in every match, and never waits more than 600 frames after a faceoff. On the large rink, a draw at the
3-minute timer counts as not won.

Bot-vs-bot (`--mode brain`): regular pro 8-11-1 and all_star 11-7-2; large pro 10-10-0 and all_star 7-12-1. Every big-rink match ends (timer).

## Rink size at pro

| rink | human win % | goals/match | match s | s/goal | human touches/min |
|---|---|---|---|---|---|
| 90 x 45, speed 11 (regular, first to 3) | 45 | 4.1 | 138 | 32 | 15.0 |
| 120 x 60, speed 8 (large, 3-minute timer) | 45 | 3.4 | 197 | 72 | 13.4 |

With carrying possible (drag 0.5), the two rinks play at the same difficulty. The big rink still needs more than twice the time per goal, and the human touches the
puck about 10% less. 90 x 45 is the livelier game; 120 x 60 works as a timed cut.

Earlier rink-size numbers, at drag 0 and before the backs guarded (the evidence behind R-0010): at pro, 90 x 45 gave 45% wins and 154 s per goal; 120 x 60 gave 20% wins,
246 s per goal, and 11 of 20 matches unfinished in 10 minutes.

## Feel proposals (lane A10, R-0009), measured before R-0011, at drag 0, each alone and all together

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
