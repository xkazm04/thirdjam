# The cartridge keys the modes asked for (lane A72, R-0036 / R-0037, device A)

Status: 2026-11-13 11:30, six keys landed (asks A1, A2, A3, A4, A5, A8), three modes re-measured with them, A10 (`puck.count`) a design only, A7 (offside) not
started (the brief stops at A10 when it is more than 40 minutes of work). Owner: lane A72. Asks and the six modes: `docs/experiments/MODES.md` (lane A67).

## The keys

Every key: `schema/game.schema.json` + `limits.json` `game_ranges` + `DEFAULTS.md` + `README.md` + the loader (`JamCartridge.h/.cpp`) + its reader + a case in
`Jam.Core.ModeKeys` (`Game/Source/JamCore/AI/ModeKeysTests.cpp`, the HUD case in `Game/Source/JamGame/Private/ModeKeysHudTests.cpp`); `gen-schema-data`,
`gen-parity` (81 cases), `gen-designer-data` (36 cuts) re-run; `node --test schema/*.test.mjs` 98 / 98. An absent key plays exactly as before (below).

| ask | key | range, absent | what it does | reader | test |
|---|---|---|---|---|---|
| A4 | `game.ai.react_frames` | 0..60, the level's profile (0 at every level) | the opposing bots' steering is applied this many frames late (shots, checks and pokes stay live; the back reacts at once, as before) | `DifficultyResolve.cpp` -> `Opp.ReactionFrames`; `kMaxReactionFrames` 30 -> 60 (`JamAICore.h`) | `Jam.Core.ModeKeys.ReactFrames` |
| A5 | `game.ai.body_check` | 0..1, the level's rule | the share of the chances an opposing bot takes with a body check (a coin per bot and one-second window); a chance is the level's rule (the carrier in reach, front or side, never from behind) **at the sim's own `skater.check_speed_min`** instead of the fixed 5 m/s; 0 never; at rookie (never by rule) any share above 0 turns the rule on | `DifficultyResolve.cpp` -> `Opp.BodyCheckShare`, `Opp.CheckMinClosing`; coin in `FJamBrain::StickPlay` | `Jam.Core.ModeKeys.BodyCheck` |
| A1 | `game.rules.timer_frames` | 600..**36000** (was 18000) | a ten-minute clock; the HUD reads 10:00 (`UJamHudWidget::FormatTimer`) | loader, sim (unchanged) | `Jam.Core.ModeKeys.TimerTenMinutes`, `Jam.Core.ModeKeys.HudClock` |
| A2 | `game.ai.sprint` | 0..1, 0 | the share of two-second windows a bot (both sides) sprints while it chases the puck from more than 6 m without carrying it; same stamina bar as the human's (`skater.sprint`) | `DifficultyResolve.cpp` -> `FJamAIParams::SprintShare`; `FJamBrain::Think` sets `FJamAIInput::bSprint`; copied to `FJamInput::bSprint` by `JamSimAdapter`, `HokejGameMode`, `LobbyGameMode` | `Jam.Core.ModeKeys.Sprint` |
| A3 | playtester sprint (tooling, not a key) | off unless asked | the regular player (half the 2-second windows) and the good one (every window) press sprint on a breakaway (carrying with the burst on or no opponent within 8 m) or on a chase of a loose or opposing puck more than 10 m away; the novice never | `JamPlaytester.cpp` (`SprintPct`, `bSprint`); commandlet `-playersprint`; `tools/playtest/run.mjs` / `modes.mjs` `--sprint`; playtest lines gain `sprint_frames` and `bot_sprint_frames` | `Jam.Core.ModeKeys.Sprint` |
| A8 | `game.rules.penalty_frames` | 60..600, 300 | how long a hard foul sits in the box (read only with `penalties` true) | `UJamSim::CallFoul` (`JamSim.cpp`; `UJamSim::BoxFrames` is the default) | `Jam.Core.ModeKeys.PenaltyFrames` |

Why A5 reads `check_speed_min`: the human was checked down 0 times in all 160 of A67's matches, also at pro where bots check (R-0031). The brain pressed only with
5 m/s of its own speed along the contact; the bots that meet a carrier from the front are mostly the back on its guard post, standing. Cage and Blind Side lower
`skater.check_speed_min` (3 and 2) and the sim would land those checks, but the brain kept 5. A share alone ("1 = the level's rule") would have changed nothing there.

Why A3 is opt-in: a playtester that sprints is a different player. On the same binary, regular pro moves from 35 % to 40 % wins (20 seeds) and every curve lane A70
is re-tuning against would move with it. The playtester sprints only with `-playersprint` / `--sprint`; `Jam.Core.ModeKeys.Sprint` checks that without it the match
hashes exactly as before. Making it the default is a Director's call once A70's curve has landed.

## Determinism

`Jam.Core.Determinism`: hockey.regular seed 1 frame 600 `ccb658f82e4ca910`, hockey.raw `e09eebd2cebd1337`, the same as before the lane. Every new reader is guarded by
"the key is written" (the coins are thrown only below 1, the sprint only above 0, the box length defaults to the old 300, the ring's size does not change which
entry a delay reads), so a cartridge without the keys plays bit-identical: `Jam.Core.ModeKeys.ReactFrames` records hockey.regular, seed 1, 3600 frames, regular
playtester `94587b9f7b9e289d` before and after the sprint code. Automation after the last build: `Jam.Core + Jam.Bot + Jam.Menu + Jam.Playtest`, 51 / 51 Success.

## The modes re-measured

`node tools/playtest/modes.mjs --seeds 1-20 --max-frames 48000` (regular playtester, A67's method), table `tools/playtest/out/modes-keys.md`, lines `modes-keys.jsonl`.
Marathon with `--sprint`: `tools/playtest/out/modes-keys-r2-sprint.{md,jsonl}`. "Before" is A67's file played on **this** binary (`tools/playtest/out/a72-variants/before.*`):
the regular baseline is no longer A67's (regular pro 35 % wins against A67's 50 %, the human checked down 1.9 times a match against 0: main moved between A67's run and
0da18f2), so A67's own table is not the comparison.

| mode | keys added | character number | regular | before | after | change vs regular (before -> after) | win % before -> after (regular 35) | other |
|---|---|---|---|---|---|---|---|---|
| **Cage** | `ai.body_check` 0.5, `rules.penalties` true, `penalty_frames` 120 | checks landed / match | 3.6 | 4.9 | **5.4** | +36 % -> **+50 %** | 15 -> 15 | checks taken 6.9 -> 4.4 (regular 1.9); fouls 0 -> 1.9 a match at 2 s each |
| **Blind Side** | `ai.react_frames` 4, `ai.body_check` 1 | dekes / match | 0.75 | 1.0 | **1.85** | +33 % -> **+147 %** | 25 -> 70 | checks taken 2.05 -> 3.5 (both ways now: "checks that stick") |
| **Marathon** | `rules.timer_frames` 36000, `ai.sprint` 0.5 | human sprint frames / match (`--sprint`) | 654 | 1312 | **2526** | +101 % -> **+286 %** | 60 -> 85 (regular 40 with `--sprint`) | bots sprint 27,256 frames a match (sum of 5 bots); sprint per play minute 251 (regular 237); minutes per goal 0.95 |

The other three modes do not use a new key and measure as before on this binary (Pond +55 %, Power Play +194 %, Sniper +93 %; all pass).

Tuning on the way (20 seeds each, `tools/playtest/out/a72-variants/`): Cage `body_check` 1 / 0.5 / 0.3 gave checks landed 4.45 / 5.4 / 5.7 and wins 5 / 15 / 20 %:
at 1 the bots out-hit the human (taken 7.35) and the mode fails its number; 0.5 keeps the human's hitting and makes it two-way. Blind Side `react_frames` 20 / 10 / 6 / 4 / 3
gave dekes 1.95 / 1.7 / 1.4 / 1.85 / 1.25 and wins 90 / 95 / 75 / 70 / 60 %: 4 is the most dekes short of a walkover.

Findings for others:
- **Marathon's sudden death runs long**: without `--sprint`, seed 8 is 7-7 at the 10:00 horn and the next goal comes 9,900 live frames later (the match ends at frame
  49,284, 13:41); with `--sprint` all 20 end inside 48,000. R-0037's hard stop at 33,000 live frames is written for `first_to`; if its landing applies it to "any match
  still running", it must sit above `timer_frames` for a timer match (a 36,000 clock would be cut at 33,000). For the lane that lands R-0037.
- The menu's cards (`UJamMenuFlow::ModeCards`) said "five-minute clock" and "no penalties": Marathon's pitch and facts now read 10:00 and "everyone sprints", Cage's
  "bots hit back, 2 s in the box", Blind Side's "slow-reacting bots" and "checks from 2 m/s both ways". `Game/Content/Cartridges/modes/MODES.md` lists the new keys.

## A10 `puck.count` (1..3): the design, not built

Estimate: well over the 40 minutes the brief allows (a day with tests). `UJamSim` holds one `FJamPuckState Puck` and reads `Puck.` 176 times in `JamSim.cpp`
(carry, shot, charge and lift, pass and reception, poke, stick lift, deke, sweep shield, goalie save, goal test, posts, boards, obstacles, zones, pickups, faceoff, hash),
`GetPuck()` is read in 27 places outside the sim (adapter, game mode and puck actor, HUD feed, barks, commentary, net packets, lobby), and the AI world carries one
`PuckPos / PuckVel / PuckLastTouch / Carrier`.

1. **Sim**: `TArray<FJamPuckState> Pucks` (size `puck.count`, fixed for the match); `Puck` stays as a reference to `Pucks[0]` for the read-outs so every outside caller
   keeps working. Each step loops the puck phase per puck in index order (`StepPuck(int32 P)`): integration, boards, obstacles, posts, goal test. A skater carries at
   most one puck: `CarrierIndex(P)` per puck, and the carry, shot, pass, poke, lift and deke act on **the puck in the skater's reach that it carries, else the nearest
   in reach** (ties by index). A goal by any puck scores; after a goal every puck resets: puck 0 to the centre dot, pucks 1 and 2 to the two neutral-zone dots on the
   side of the team that conceded (seeded drop like the faceoff). Puck-puck contact: an elastic disc bounce in index order (needed, or two pucks overlap on a carry).
2. **Hash**: each extra puck hashed after puck 0 only when `Pucks.Num() > 1`, so `puck.count` 1 hashes as today (Determinism unchanged); a 2- or 3-puck match records
   its own baseline in `Jam.Core.ModeKeys.PuckCount`.
3. **Brain**: `FJamAIWorld` gains `NumPucks` and `Pucks[3]` (pos, vel, last touch, carrier); today's `PuckPos` etc. become puck 0's. The chaser and the support chase
   the nearest uncarried puck, the back guards the puck nearest its net, a carrier attacks with its own puck; poke and check target the carrier of the puck nearest.
   The playtester does the same (chase the nearest).
4. **Game**: one puck actor per puck (`HokejGameMode` spawns `Pucks.Num()`), the camera and the HUD's puck arrow follow puck 0 or the human's puck; events carry the
   puck index in `Kind` (`puck_1`, `puck_2`) for the barks and the commentary, which keep reading puck 0 otherwise.
5. **Net**: lockstep needs nothing new (inputs only), the snapshot packet (`JamNetPackets.cpp`) gains the extra pucks; protocol 6.
6. **Proof**: `Jam.Core.ModeKeys.PuckCount` (each puck scores and resets; a skater carries one; count 1 hashes as before), then a "Three Pucks" mode (A67 tried it as
   `puck.count` 3: 20 matches identical to regular) measured with `modes.mjs` on goals per minute.

## Not done

- A7 `rules.offside` true: not started (after A10 in the brief's order).
- A6, A9, A11: not in this brief.
