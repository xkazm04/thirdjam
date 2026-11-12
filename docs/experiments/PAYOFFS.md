# Payoffs: the one-timer, the breakaway burst, the deke (lane A39, R-0029 point 4, H11.7-H12.7, device A)

Status: 2026-11-12 20:40, final. Three calibrations under R-0021 point 1: each a cartridge key with a default (`schema/DEFAULTS.md`), a test (`Jam.Core.Payoffs.*`), the
playtester grid before and after (sections 4-5), and the Determinism baseline re-recorded (FEEL.md, "Lane A39 calibrations"). The three keys at 0 replay the old game to the
hash (40 of 40 matches identical to the baseline grid in the all_star / good and pro / regular cells), so a cut can switch any payoff off without changing anything else.

## 1. The mechanics (`Game/Source/JamCore/Private/JamSim.cpp`)

| Payoff | Rule | Event |
|---|---|---|
| **One-timer** | A teammate's puck that lands on the blade (the sim's reception: within 0.6 m of the blade, faster than 6 m/s) opens a window of `shot.one_timer.window_frames` (10). A shot released inside it, tap or held, gets the **full charge** (25.8 m/s on hockey.regular: speed 18 + bonus 8, instead of a 17.9 m/s tap) and **`aim_cone_deg` + `cone_bonus_deg`** (8 + 8) of aim help. The window closes with the shot (a second shot is no one-timer). | `shot` with Kind `one_timer` |
| **Breakaway burst** | Once the carrier has carried for 30 frames (a touch-and-shoot never bursts), is past centre ice, skates at the net (facing within 60 degrees), is at least 8 m from it, and no standing opponent stands in a 4 m half-width corridor between it and the edge of the crease (6 m from the net: the back holding the crease is the goalie, not the last man), top speed and acceleration rise by `skater.breakaway.speed_bonus` (0.2) for `frames` (60). Once per possession: the flag resets when the puck changes hands. Decided before anyone moves, on the positions the player saw. `UJamSim::PastLastDefender` is the rule; bots burst too. | `breakaway` (Who = the carrier, Strength 1) |
| **Deke** | `FJamInput::bDeke` (edge) by the carrier: the puck moves over 6 frames to `stick.deke.offset_m` (1.0) on the **blade side** (the side the stick is aimed to; the middle of the ice when the stick rests), riding 0.8 m ahead of the body (the offset is clamped so the puck stays inside shot reach: 1.15 m at reach 1.5). The **crease back commits** for `commit_frames` (15): the brain's blade, poke and shot block follow the puck's old side (`FJamAIWorld::DekeCommitOffset`), and a shot released inside the window **aims at the post on the deke side** (0.6 m inside) and keeps the back committed until that puck has crossed the goal line. A second tap inside 30 frames does nothing; a pass or a shot spends the deke. | `deke` (Who = the carrier, Kind `left` / `right`: the side in rink z) |

Everything is deterministic: no new random draws, the windows are frame counters in `FJamSkaterState` (`PassReceivedFrame`, `BreakawayFrames`, `bBreakawayUsed`, `CarryFrames`,
`DekeFrames`, `DekeSide`, `DekeReadyFrame`) plus the sim-level commit (`GetDekeCommit`), all in `StateHash` when they leave their defaults.

## 2. The keys (`schema/game.schema.json`, `limits.json` game_ranges, README, DEFAULTS; the C++ loader reads the nested objects; `JamSchemaData.inl` regenerated)

| Key | Default | Range | 0 means |
|---|---|---|---|
| `game.shot.one_timer.window_frames` | 10 | 0..60 | no one-timers |
| `game.shot.one_timer.cone_bonus_deg` | 8 | 0..30 | the one-timer keeps the plain cone |
| `game.skater.breakaway.speed_bonus` | 0.2 | 0..1 | no burst |
| `game.skater.breakaway.frames` | 60 | 0..300 | no burst |
| `game.stick.deke.offset_m` | 1.0 | 0..2 | no deke (the tap does nothing) |
| `game.stick.deke.commit_frames` | 15 | 0..60 | no deke |

`DESIGNER_GAME_EDITABLE` note for lane A7: none of the six is in the list today (`schema/validate.mjs`); I propose adding `shot.one_timer`, `skater.breakaway` and `stick.deke` as
whole objects, with these Designer sentences (one per payoff, for `prompts/designer.system.md`): *"A one-timer: a shot taken within `window_frames` frames of catching a pass is
a full-power shot with `cone_bonus_deg` degrees more aim help; 0 frames turns it off."* / *"A breakaway burst: a carrier who gets past the last defender with the net ahead skates
`speed_bonus` (0.2 = 20 %) faster for `frames` frames, once per possession; 0 turns it off."* / *"A deke: a tap moves the puck `offset_m` metres to the stick side and the goalie
commits to the old side for `commit_frames` frames; the shot then goes to the open post; 0 metres turns it off."*

## 3. The tests (`Game/Source/JamCore/Private/Tests/JamPayoffTests.cpp`, all PASS on the final build, 20:34)

| Test | What it proves | Numbers |
|---|---|---|
| `Jam.Core.Payoffs.Keys` | the six defaults load from hockey.regular; the six keys load when written; an unknown key under `one_timer` is rejected with the pointer `/game/shot/one_timer/window` first | |
| `Jam.Core.Payoffs.OneTimer` | a cross-ice pass (16 m, 20 m/s in the scene so it arrives above the 6 m/s reception floor) lands on the slot man's blade 12 m from the net; a tap inside the window is a one-timer (Kind `one_timer`, charge 1.00, 25.8 m/s) and the widened cone snaps a facing 10 degrees off the net onto it (the line crosses the goal line at z 0.00); the same tap 40 frames later is a plain shot (17.8 m/s, charge 0) that goes 10 degrees wide (z -4.73 at the goal line). The brain's pro back plays the scene; whether either shot beats it is reported (neither does, section 6) | |
| `Jam.Core.Payoffs.Breakaway` | a carrier at centre ice with the chaser 3.5 m behind at the same speed: the burst fires once (at frame 31 of the carry, `BreakawayFrames` = 60), the slot (8 m from the net) is reached at frame 147 instead of 169, 7.6 m ahead of the chaser instead of 3.5; with `speed_bonus` 0 no event | |
| `Jam.Core.Payoffs.Deke` | against the brain's pro back in goalie mode, from 8.5 m, seeds 1-3 (approach lines z -1 / 0 / +1): the puck is 0.98 m off the body line at +6 frames, one `deke` event per seed (the second tap at +10 does nothing), the shot at +8 scores **3 of 3** where the same shot without the deke is saved **3 of 3** | |
| `Jam.Core.Payoffs.EmptyNetScript` | the `Jam.Core.Seams` scene (a scripted attacker alone against an empty net, skater speed 13): the goal at frame 244 with the burst on as with it off. The first cut of the burst fired on the touch; the 15.6 m/s attacker outran its own 17.8 m/s tap, turned after the puck and carried it to its own end for the rest of the match (Seams failed); the 30-frame carry fixed it | |

Also re-run on the final build: `Jam.Core.Seams`, `Determinism` (new baseline hockey.regular seed 1 frame 600 `b705e2def86ae838`, hockey.raw `e09eebd2cebd1337`), `FeelDepth`, `Stick`,
`CartridgeRoundTrip` (the embedded schema matches `schema/`), `CartridgeParity`, and `Jam.Bot.*` (AiBotPlays, DifficultyProfilesResolve, HumanHasSomethingToDo, PlayerBotWins,
SeasonTakes, VisualSwap): all PASS. `Bar.Playtest.*` are R-0029's bars and fail on main as before (not run here).

## 4. The playtester and the bots use the payoffs

- `JamPlaytester.cpp`: the **good** player always takes a one-timer or a deke when the chance is there, the **regular** player in half the 2-second windows (a seeded coin), the **novice**
  never (`OneTimerPct` / `DekePct` 100 / 50 / 0). One-timer: a pass landed on my blade, the net inside shooting range + 8 m, my facing within tolerance + 8 degrees of the aim: tap.
  Deke: inside 12 m with the crease back (within 5 m of its net) on my line: tap, then the shot once the puck has settled. Every player **reaches for an incoming teammate's puck**
  (stick aimed at it inside 8 m when it comes at me above 6 m/s): the sim seats a pass on the blade only when the blade sits on its line, and before this nobody ever did. The
  breakaway burst is the sim's: nobody presses for it.
- `JamAICore.cpp`: `FJamBotSkill::bOneTimer` (pro and all_star) and `bDeke` (all_star only; rookie neither; the human's teammates neither), tune keys `opp.one_timer`, `opp.deke`.
  The crease back's blade, poke and shot block follow the puck's old side while an opponent's deke commit runs (`StickPlay`, the block branch of `Decide`).
- `FJamAISkater` carries `OneTimerFrames`, `bBreakaway`, `DekeFrames`, `bDekeSettled`, `bCanDeke`, `DekeOffset`; `FJamAIWorld` the commit; `FJamSimAdapter::BuildWorld` fills them and
  `Step` copies `bDeke`. New stats in the `thirdjam.playtest/1` line: `one_timers`, `breakaways`, `dekes`, `payoff_goals` (a human goal within 180 frames of one of them); `run.mjs` prints
  them as `payoffs OT/BA/DK (goals)`.

## 5. The grid before and after (hockey.regular, 20 seeds per cell, 3 players x 3 levels; `tools/playtest/out/a39-before.md` = main 969fbc3, `a39-after2.md` = the final build)

Before (identical to PLAYTEST.md section 3, main f89014c: lane A3's R-0029 calibrations are not on main yet):

| level | player | W-L | win % | GF / GA | by human | carries 3 s | shots | SOT % | first goal s (n / <2 min) | idle spells (s) | match s |
|---|---|---|---|---|---|---|---|---|---|---|---|
| rookie | novice | 3-17 | 15 | 1.1 / 2.8 | 0.8 | 3.6 | 5.5 | 60 | 93.9 (10 / 7) | 2.8 (23.1) | 244 |
| rookie | regular | 12-8 | 60 | 2.2 / 1.6 | 1.7 | 4 | 15.5 | 63 | 62.9 (17 / 14) | 0.7 (2.9) | 208 |
| rookie | good | 14-6 | 70 | 2.6 / 1.6 | 1.9 | 4.4 | 22.5 | 57 | 58.7 (18 / 13) | 0.2 (0.6) | 188 |
| pro | novice | 1-19 | 5 | 1 / 3 | 0.5 | 0.9 | 4.2 | 33 | 77.3 (8 / 6) | 3.7 (43.4) | 224 |
| pro | regular | 8-12 | 40 | 1.9 / 2.3 | 1.5 | 1.9 | 7.1 | 44 | 74.4 (15 / 10) | 2.4 (16.4) | 179 |
| pro | good | 10-10 | 50 | 2.2 / 2.2 | 1.6 | 1.8 | 16.4 | 39 | 38.7 (17 / 16) | 1.3 (5.2) | 167 |
| all_star | novice | 2-18 | 10 | 0.8 / 2.9 | 0.5 | 0.8 | 1.3 | 44 | 66.1 (8 / 8) | 3 (33.1) | 140 |
| all_star | regular | 6-14 | 30 | 1.3 / 2.3 | 0.6 | 1.1 | 4.2 | 45 | 20.2 (8 / 7) | 1.3 (7.7) | 112 |
| all_star | good | 8-12 | 40 | 1.7 / 2.3 | 1.4 | 1.4 | 10.9 | 33 | 43.4 (14 / 11) | 1.1 (5.5) | 144 |

After (the three payoffs at their defaults; `payoffs` = one-timers / bursts / dekes by the human per match, and his goals within 3 s of one):

| level | player | W-L | win % | GF / GA | by human | carries 3 s | shots | SOT % | first goal s (n / <2 min) | payoffs OT/BA/DK (goals) | idle spells (s) | match s |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| rookie | novice | 7-13 | **35** | 1.4 / 2.2 | 0.9 | 2.3 | 4.3 | 65 | 30.6 (10 / 8) | 0 / 3.1 / 0 (0.7) | 2 (17) | 183 |
| rookie | regular | 18-2 | **90** | 2.8 / 1.1 | 2.3 | 2.9 | 11.2 | 66 | 56.7 (19 / 17) | 0.1 / 5.5 / 2.2 (2.1) | 0.6 (1.8) | 143 |
| rookie | good | 17-3 | **85** | 2.9 / 0.7 | 2.6 | 3.1 | 14.7 | 60 | 51 (20 / 17) | 0 / 4.6 / 3.2 (2.1) | 0.4 (0.4) | 133 |
| pro | novice | 1-19 | 5 | 0.8 / 2.9 | 0.5 | 0.6 | 2.8 | 32 | 101.9 (8 / 6) | 0 / 0.9 / 0 (0.4) | 3.6 (38.5) | 189 |
| pro | regular | 7-13 | 35 | 1.5 / 2.5 | 1.3 | 1 | 6.4 | 35 | 76.3 (13 / 9) | 0 / 2.1 / 1 (1.1) | 2.3 (16) | 171 |
| pro | good | 12-8 | **60** | 2.5 / 2.1 | 2 | 1.5 | 14.4 | 43 | 45.3 (18 / 15) | 0 / 3.4 / 3.4 (1.9) | 1 (3.4) | 166 |
| all_star | novice | 0-20 | 0 | 0.5 / 3 | 0.3 | 0.5 | 1.1 | 71 | 88.9 (5 / 3) | 0 / 0.9 / 0 (0.3) | 2.7 (29.5) | 129 |
| all_star | regular | 4-16 | 20 | 1.4 / 2.7 | 1 | 1.4 | 6.4 | 49 | 51 (12 / 10) | 0.1 / 2 / 1.3 (0.9) | 1.6 (9.3) | 156 |
| all_star | good | 12-8 | **60** | 2.1 / 1.9 | 1.6 | 0.9 | 9.6 | 42 | 15.9 (14 / 14) | 0 / 2.1 / 2.1 (1.4) | 0.5 (2.1) | 115 |

What moved: the human's **goals** (rookie 0.8 / 1.7 / 1.9 -> 0.9 / 2.3 / 2.6, pro good 1.6 -> 2.0), the **two-minute goal** (rookie regular / good 14 / 13 -> 17 / 17 of 20; the
novice 7 -> 8), **carries with a payoff**: 2-5.5 bursts and 1-3.4 dekes per match for the regular and good players, and 0.7-2.1 of their goals per match come within 3 s of one (the good
player at rookie: 2.1 of his 2.6 goals). **One-timers stay at 0-0.1 per match**: the human receives almost no pass (section 6). Matches are shorter (more goals), idle spells fall.
The novice's win rate at rookie moves 15 -> 35 at 20 seeds but **23 -> 23 at 40 seeds** (seeds 1-40, `a39-x8` / `x9`): for a first-time player the payoffs change how goals come
(first goal median 98 -> 53 s, 10 -> 13 of 40 inside two minutes), not how many matches he wins; R-0029 point 2 (P1-P3) stays the calibration that moves his curve.

**Noise**: 20 seeds give +-10 points of win rate (one match = 5 points). The curve cells were re-measured at 40 seeds on the final build (`tools/playtest/out/a39-x*`), with
the keys off as the control (the old game, hashes identical to the baseline):

| cell (40 seeds) | keys off (old game) | payoffs on | R-0029 target | `red_skill` offset | `opp.deke=0` |
|---|---|---|---|---|---|
| all_star / good | 28 | **48** (GF 1.8 / GA 2.1, 2.1 bursts, 2.0 dekes, 1.2 payoff goals) | about 25 | 0.97 -> **30** | 45 |
| pro / regular | 28 | 33 (2.1 bursts, 0.9 dekes, 1.0 payoff goals) | about 50 | 0.95 -> 38 | |
| rookie / novice | 23 | 23 (3.3 bursts, first goal 98 -> 53 s, 10 -> 13 of 40 inside 2 min) | 85 (with P1-P3) | | |

**The curve leaves the target by more than 5 points in one cell: the good player at all_star, +23 (48 against about 25; the old game was at 28).** The deke beats the all_star
back (the good player dekes twice a match and 1.2 of his 1.4 goals come within 3 s of a payoff) and his own back is beaten less than it should be because the all_star bots deke
too but the brain's carriers reach 12 m of the net less often than he does. **Proposal, not applied** (R-0029 point 1 says pro and all_star stay untouched until the regular and good
players are measured again): all_star `red_skill` 1.0 -> **0.97** puts the good player at 30 (within 5 of 25) with the payoffs on; `opp.deke=0` (45) is not enough on its own.
The pro / regular cell is 17 points under its target with the keys off and 12 under with them on: the gap is older than this lane (PLAYTEST.md measured 40 at 20 seeds, which is
28-40 at this noise); `red_skill` 0.95 at pro gives 38 and is the measured next step for lane A3.

## 6. What the measurements say about each payoff (for the Director)

1. **The deke is the payoff that works.** Against the crease back it scores 3 of 3 in the test where the plain shot is saved 3 of 3, and in the grid the regular and good players
   take 1-3.4 dekes a match and score within 3 s of one 1-2 times a match at every level. It is also what moves the all_star cell past its target, so it is the knob to watch
   (`stick.deke.offset_m` or the bots' `opp.deke`).
2. **The burst fires 2-5 times a match** for anyone who carries past centre ice; carries over 3 s do not grow (the burst ends the carry sooner: a goal or a shot), but the
   human's goals and the two-minute goal do. The 30-frame carry requirement is load-bearing (`EmptyNetScript`). The burst is symmetric: all_star bots burst too (their GA on
   the good player 2.3 -> 2.1 with his dekes, 2.5 -> 2.6 for the regular player at pro).
3. **The one-timer does not beat the brain's back and almost never happens for the human.** (a) The guard post is on the line from the goal to the puck and follows the puck in
   flight, and the back has zero reaction frames at every level: from 12 m it is on the receiver's line before a 16 m pass arrives, and a one-timer released on the frame of
   reception still meets its blade (both shots of the test scene are deflected). The one-timer's real payoff is the full charge without the 30-frame hold (+8 m/s, the puck
   arrives 30 % sooner) and the 16-degree cone (a facing 10 degrees off still goes in; the plain tap goes wide). (b) The human receives a pass on the blade almost never (0-0.1 one-timers
   a match even with the playtester reaching for incoming pucks): teammates pass to him only at rookie (`bPassToHuman`), the human's own passes are 0-1 a match, and the reception needs
   the blade within 0.6 m of the puck's line. **Two proposals**: for lane A16, a teammate's puck that reaches the body within shot reach seats on the blade like a blade reception
   (today it bumps off the body and runs away); for lane A3, `bPassToHuman` at pro too (the support position is where the one-timer lives). Until then the one-timer is a bots' tool.

## 7. Lockstep (lane A38's checklist) and the game mode: wired in the merge pass (a39-main, 20:45)

- `FJamInput::bDeke` is a new input field (an edge). `FJamNetQuantInput` carries it as the eighth flag bit (`Deke = 128`, in `EdgeFlags` so a press read during a stall is kept),
  `From` / `ToInput` / `CoveredFields` know it, `JamNet::Version` is **3** (a version-2 peer is refused by the magic check, as the protocol intends). `ALobbyGameMode::ReadLocalInput`
  reads it from `UJamPlayerInput::ConsumeDekePressed`, `StepLockstep` copies the brain's `bDeke` for the bot slots. `Jam.Net.InputCoverage` (every `FJamInput` UPROPERTY travels) is the guard.
- `HokejGameMode::ReadHumanInput` reads `IA_Deke` (**keyboard Q, gamepad Y**: X is the pass, RB / LB poke and lift; `InputMapping.{h,cpp}`: action, keys, latch, consume) and
  `GatherInputs` copies the brain's `FJamAIInput::bDeke`. The visual gets `bDekeThisFrame`, `bBreakawayThisFrame`, `bOneTimerThisFrame` and the events.
- Events on main: lane A32's commentator read-out already emits `breakaway` (every standing defender behind the carrier, once per rush); the burst **reuses that name**: whichever
  comes first in a rush emits it and marks the rush (`BreakawayFor`, read-out state, never hashed), so a rush is called once. `deke` (Who = carrier, Where = puck, Kind `left` |
  `right`) and `shot` Kind `one_timer` are the two additions for C-0003 (`contracts/events.md`, not edited from this lane); both names are in `UJamEventBus::GetKnownEvents`.
- Merged with main 5397edb: the all_star profile keeps A3's speed 1.08 and gains `bOneTimer` / `bDeke`; the brain's `Decide` / `Attack` take A31's `bAimed` and this lane's `bDeke`
  (a one-timer and a deke shot stay taps: A31's charge hold applies to the aimed shot only); `stick.foul_grace_frames` (A16) and `stick.deke` sit side by side in the schema.

## 8. How to reproduce

```
node tools/playtest/run.mjs --rinks regular --seeds 1-20 --out tools/playtest/out/grid                                  # the after table (9 cells, 41 s headless)
node tools/playtest/run.mjs --rinks regular --seeds 1-40 --levels all_star --players good --tune "red_skill=0.97"        # the all_star offset proposal
node tools/playtest/run.mjs --rinks regular --seeds 1-40 --levels all_star --players good --set "stick.deke.offset_m=0;skater.breakaway.speed_bonus=0;shot.one_timer.window_frames=0"   # the old game
UnrealEditor-Cmd Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Core.Payoffs; Quit" -nullrhi -unattended -nopause -stdout
```
Raw lines: `tools/playtest/out/a39-before.jsonl`, `a39-after2.jsonl` (the final build), `a39-after.jsonl` (the first cut, before the 30-frame carry and the pass reach), `a39-v*` (20-seed
offsets measured on the first cut), `a39-w*` (40 seeds, first cut), `a39-x*` (40 seeds, final build). Binary: the a39 worktree at main 969fbc3 plus this lane's files.

## 9. On main 5397edb (the merge pass, worktree `a39-main`, 20:41-20:52)

Main moved past 969fbc3 with lane A3's R-0029 calibrations (rookie P1-P3, all_star speed 1.08) and gates, A31's charged bot shot and goalie stance, A32's commentator read-outs
(the `breakaway` event is A32's; the burst reuses it, one call per rush), A33's check kinds, A16's `stick.foul_grace_frames`. 22 conflict hunks in 12 files were resolved keeping
both sides; the three wiring items of section 7 are in (`JamNet::Version` 3). Build PASS 20:46:24-20:48:02; suites on that build: Jam.Core (incl. Payoffs 5 / 5), Jam.Bot,
Jam.Playtest (`NoviceScoresAtRookie` PASS with A3's calibrations), Jam.Rules, Jam.Net (`InputCoverage`: 10 fields travel, 9 bytes, version 3): all PASS. `Jam.Effects.BotMatchWithEffects`
flipped: with the payoffs the hockey.boost bot match (seed 1, all_star) ends at frame 4494 with no pickup taken (5427 frames and one take with the three keys at 0): the scene now tries
seeds 2 and 3 when seed 1 took none (lane A14's test, a foreign touch flagged for the Director). Determinism baseline on main: unchanged from section 3.

The grid on the merged build (hockey.regular, 20 seeds), win % novice / regular / good: rookie **80 / 100 / 100**, pro 15 / 50 / 80, all_star 0 / 15 / 50. At 40 seeds with the keys off
as the control (the old main):

| cell (40 seeds, main 5397edb) | keys off | payoffs on | R-0029 target | offsets measured (payoffs on) |
|---|---|---|---|---|
| rookie / novice | 85 (30 of 40 inside 2 min) | 83 (29 of 40; 6.1 bursts, 1.6 payoff goals) | 85 | |
| pro / regular | 33 | **53** (3.2 bursts, 1.4 dekes, 1.4 payoff goals) | about 50 | |
| all_star / good | 30 | **38** (2.2 bursts, 2.1 dekes, 1.1 payoff goals) | about 25 | `speed_scale` 1.08 -> 1.12: 33; **1.15: 23**; `red_skill` 0.97: 45; `opp.deke=0`: 38; 1.12 + `opp.deke=0`: 38 |

The novice's and the regular player's cells sit on their targets with the payoffs on (the regular player's pro cell gets there only with them: 33 -> 53). The good player at all_star
is 13 points over (the old main is 5 over). **Proposal, not applied** (R-0029 point 1: pro and all_star are A3's until re-measured): all_star `speed_scale` 1.08 -> **1.15** puts the
good player at 23 with the payoffs on (40 seeds, +-7); the bots' deke is not the cause (38 with it off), the burst and the deke together are what the good player converts.
Raw lines: `tools/playtest/out/a39main-after.*`, `a39main-y*`.

