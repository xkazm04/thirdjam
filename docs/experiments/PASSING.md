# Passing: passes reach the human (lane A46, PAYOFFS.md section 6, R-0029 point 4, device A, from H13.9)

Status: 2026-11-12 23:05, final. Two calibrations under R-0021 point 1, each a key with a default, a test and the playtester grid before and after (sections 3-5), the
Determinism baseline re-recorded (unchanged: `b705e2def86ae838` / `e09eebd2cebd1337`, FEEL.md "Lane A46 calibrations"). Both keys at 0 replay the old game.
**The Designer sentence**: *"My team passes to me: a teammate's pass that reaches my body is mine (within `reception_m` metres), and my teammates look for me in a
share of their chances (`mate.pass_to_human`); 0 turns either off."*

## 1. The rules

| Rule | Where | What |
|---|---|---|
| **Body reception** | `UJamSim::StepPuck` (`Game/Source/JamCore/Private/JamSim.cpp`) | A teammate's pass **in flight** (nobody carries it) that reaches a skater's body within `stick.reception_m` (0.9 m from the body centre; the body itself is 0.4 m plus the puck's 0.12) at 6-14 m/s is seated on the blade like lane A16's blade reception (`Puck.Pos = BladePos`, `Puck.Vel = skater's`) and the skater is its carrier. `SettleFrames` (4) follow, in which the carry assist holds the puck on the blade whatever its relative speed; A39's one-timer window opens on the reception, so a shot inside the settle (and the 10-frame window) is a one-timer. A pass **above 14 m/s** (`BodyReceptionMaxSpeed`) bounces off the body as before, unless the blade meets it (the blade reception has no cap). A **shot or a pass spends the settle** (the first cut's settle pulled the just-shot puck back onto the blade: the test guards it). The human and the bots alike. `PassesReceived` counts both receptions per skater; `SettleFrames` is hashed only while it runs, so `reception_m` 0 hashes as the old game. |
| **Pass to the human** | `FJamBrain::Attack` (`AI/JamAICore.cpp`), `FJamSimAdapter::BuildWorld` | A teammate carrying the puck (the chaser or the support; the back only while the human calls `space` or `pass`) passes to the human when the human is **open**: standing, 5-25 m away, **ahead of or level with the puck** up the ice and no opponent within 2 m of the lane (lane A3's lane check), in a **share** of the 2-second windows: `mate.pass_to_human` (rookie 0.6, pro 0.4, all_star 0.2), **0.9 while the human's `pass` signal runs** (R-0007 key 3, 120 frames from `UJamSignalSystem`). A human **set up for a shot** (inside 26 m of the net, squared to it within 15 degrees) is passed to in every chance. The coin is `Hash11(skater, frame / 120)`: no randomness, stable inside the window. The pass is the sim's (`FJamAIInput::bPass`, the lead solve of C-0005) when `FJamAIWorld::CarrierBestPass` (the adapter's copy of `UJamSim::BestPass`) is the human; else lane A3's aimed tap at the human, now only from 8 m or more (17.9 m/s minus `puck.drag` 0.5 per metre arrives under the 14 m/s cap). A shot decided before it (one-timer, aimed shot) comes first. |

Deterministic: no new random draws; `SettleFrames` is the only new sim state and is hashed only when it left its default. The 14 m/s cap and the 4 frames are constants
(`UJamSim::BodyReceptionMaxSpeed`, `ReceptionSettleFrames`), as the 0.6 m blade disc and the 6 m/s floor already are.

## 2. The keys

| Key | Default | Range | 0 means | Where |
|---|---|---|---|---|
| `game.stick.reception_m` | 0.9 | 0..3 | the blade alone receives (the old game) | `schema/game.schema.json`, `limits.json`, `README.md`, `DEFAULTS.md`, `JamCartridge.{h,cpp}` (`FJamStickConfig::ReceptionM`), `JamSchemaData.inl` and `parity.json` regenerated (schema tests 98 / 98) |
| `mate.pass_to_human` (profile knob, `-aitune` / `--tune` key; 0..1 share, was 0 \| 1) | rookie 0.6, pro 0.4, all_star 0.2 | 0..1 | the teammates never look for the human | `AI/DifficultyProfile.{h,cpp}` (`FJamDifficultyProfile::MatesPassShare`), `AI/JamAICore.h` (`FJamBotSkill::PassToHuman`) |

`DESIGNER_GAME_EDITABLE` note for lane A7: `stick` is not in the list; `reception_m` would go in with `stick.deke` (A39's proposal) under the sentence above.

## 3. The tests (`Game/Source/JamCore/Private/Tests/JamPassTests.cpp`, all PASS on the final build, 23:00; `Game/logs/a46-tests-*-final.log` in worktree a46)

| Test | What it proves | Numbers |
|---|---|---|
| `Jam.Core.PassReception` | `reception_m` defaults to 0.9 and loads when written. A scripted pass 8 m up the line at the receiver's **body** (the receiver faces the net, the stick at rest: the blade 1.0 m ahead, 1.0 m off the pass line, more than the 0.6 m disc) at `pass_speed` 12 arrives at **8.9 m/s** and is received: 1 pass received, settle 3 (4 minus the frame's own countdown), the one-timer window open, the puck **0.00 m from the blade**, the receiver the carrier; a tap on the next frame is a **one-timer** (`shot` Kind `one_timer`, charge 1.00, **25.8 m/s**, still 25.6 m/s a frame later: the settle is spent, the shot flies). The same pass at `pass_speed` 19 arrives at **15.8 m/s**: not received (0 passes, no settle, no window, the puck 0.63 m off the blade). The 8.7 m/s pass with `reception_m` 0: not received (the old game) | 3 runs |
| `Jam.Bot.PassesToHuman` | 20 playtester matches (the regular player, hockey.regular, 36,000 frames) at pro and at rookie: the human receives at least 1.5 passes per match on average at each level | pro **1.95** (11 wins), rookie **1.75** (20 wins); 0.00 one-timers at both. The brief's rookie bar was 2.5: see section 6 |

Also PASS on the final build: the whole `Jam.Core` suite (Determinism **unchanged**: hockey.regular seed 1 frame 600 `b705e2def86ae838`, hockey.raw `e09eebd2cebd1337`;
Payoffs 5 / 5, Seams, Stick, Feel, FeelDepth, NetTrap, LiveConfig, PuckNeverTunnels, ScoreOnEmptyNet, TacklingEvents, CartridgeRoundTrip, CartridgeParity), the whole `Jam.Bot`
suite (AiBotPlays, BoostPickupsBundle, BundleKeepsRegularHash, DifficultyProfilesResolve, HumanHasSomethingToDo, PlayerBotWins, SeasonTakes, VisualSwap) and `Jam.Playtest`
(NoviceScoresAtRookie: 14 of 20 inside two minutes, need 10; seed 1 replays to `ab3e52883835dcb2`).

## 4. The grid before and after (hockey.regular, 20 seeds per cell, 3 players x 3 levels; `tools/playtest/out/a46-before.md` = main 0542360 untouched, `a46-after2.md` = the final build)

Before (`passes (rcv)` = the human's passes made (received); the before binary has no `passes_received` stat, so it reads 0 there):

| level | player | W-L | win % | GF / GA | by human | carries 3 s | shots | SOT % | first goal s (n / <2 min) | passes (rcv) | payoffs OT/BA/DK (goals) | idle spells (s) | match s |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| rookie | novice | 16-4 | 80 | 2.8 / 1.2 | 2 | 5.8 | 8.7 | 55 | 80.8 (19 / 15) | 0.1 (-) | 0.1 / 5.8 / 0 (1.6) | 1.9 (10.2) | 249 |
| rookie | regular | 20-0 | 100 | 3 / 0.2 | 2.5 | 3.5 | 12.7 | 64 | 21.9 (20 / 19) | 0.4 (-) | 0 / 3.7 / 1.8 (2) | 0.3 (0.7) | 118 |
| rookie | good | 20-0 | 100 | 3 / 0.1 | 2.5 | 3.4 | 22.2 | 59 | 36.6 (20 / 19) | 1 (-) | 0.1 / 2.7 / 4.3 (1.7) | 0.1 (0) | 129 |
| pro | novice | 3-17 | 15 | 1 / 2.8 | 0.6 | 1.1 | 4 | 45 | 105.8 (9 / 5) | 0.1 (-) | 0 / 1.9 / 0 (0.6) | 3.3 (23.7) | 215 |
| pro | regular | 10-10 | 50 | 2.1 / 2 | 1.5 | 1.6 | 10 | 47 | 57.1 (16 / 13) | 0.7 (-) | 0 / 3.3 / 1.2 (1.3) | 1.1 (5.7) | 167 |
| pro | good | 16-4 | 80 | 2.6 / 1.5 | 2.3 | 1.9 | 16 | 41 | 48.5 (18 / 18) | 1.6 (-) | 0.1 / 3.5 / 3.7 (2) | 0.2 (1.7) | 143 |
| all_star | novice | 1-19 | 5 | 0.7 / 3 | 0.1 | 0.4 | 1.2 | 43 | 70.7 (2 / 2) | 0 (-) | 0 / 1.1 / 0 (0.1) | 3.2 (37.4) | 156 |
| all_star | regular | 3-17 | 15 | 1.3 / 2.9 | 0.9 | 0.9 | 4.8 | 41 | 77.2 (13 / 9) | 0.9 (-) | 0 / 1.4 / 0.8 (0.6) | 1.7 (14.1) | 159 |
| all_star | good | 7-13 | 35 | 1.5 / 2.4 | 1.1 | 0.5 | 8.2 | 35 | 48.3 (12 / 11) | 1.4 (-) | 0 / 1.6 / 1.1 (0.8) | 1.2 (7.9) | 123 |

After (the two keys at their defaults; the final build):

| level | player | W-L | win % | GF / GA | by human | carries 3 s | shots | SOT % | first goal s (n / <2 min) | passes (rcv) | payoffs OT/BA/DK (goals) | idle spells (s) | match s |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| rookie | novice | 16-4 | 80 | 2.7 / 1.1 | 1.8 | 5.6 | 9.4 | 52 | 80.8 (19 / 14) | 0.1 (1.9) | 0 / 6.9 / 0 (1.4) | 1.7 (8.7) | 250 |
| rookie | regular | 20-0 | 100 | 3 / 0.2 | 2.5 | 3.7 | 13 | 63 | 21.9 (20 / 18) | 0.3 (1.8) | 0 / 3.8 / 1.7 (2) | 0.2 (0.2) | 122 |
| rookie | good | 20-0 | 100 | 3 / 0 | 2.6 | 3.4 | 22.2 | 58 | 36.6 (20 / 19) | 0.7 (2) | 0.1 / 2.8 / 4.3 (1.7) | 0 (0) | 124 |
| pro | novice | 3-17 | 15 | 1.1 / 2.8 | 0.5 | 1.1 | 4 | 44 | 98.2 (8 / 6) | 0.1 (0.9) | 0 / 1.9 / 0 (0.5) | 3.4 (26) | 213 |
| pro | regular | 11-9 | 55 | 2.3 / 2 | 1.9 | 2 | 9.5 | 51 | 50.6 (17 / 13) | 0.8 (2) | 0 / 3.6 / 1.2 (1.6) | 1.2 (5.4) | 171 |
| pro | good | 16-4 | 80 | 2.6 / 1.3 | 2.2 | 1.7 | 18.5 | 38 | 52.9 (18 / 16) | 1.8 (1.8) | 0.1 / 4.1 / 3.7 (2) | 0.4 (2.7) | 170 |
| all_star | novice | 1-19 | 5 | 0.8 / 3 | 0.2 | 0.3 | 1.6 | 45 | 33.1 (3 / 3) | 0 (0.4) | 0 / 1 / 0 (0.1) | 3.3 (34.2) | 157 |
| all_star | regular | 3-17 | 15 | 1.2 / 2.9 | 0.8 | 0.6 | 5.6 | 40 | 76.2 (11 / 9) | 0.5 (0.9) | 0 / 1.5 / 0.6 (0.7) | 1.8 (11.8) | 153 |
| all_star | good | 7-13 | 35 | 1.6 / 2.3 | 1.1 | 0.6 | 9 | 34 | 60.1 (13 / 11) | 1 (1.3) | 0 / 1.9 / 1.1 (0.9) | 1.1 (6.7) | 126 |

**What moved**: the human **receives passes**: 0.9-2.0 a match at rookie and pro, 0.4-1.3 at all_star (not measured before; the controls below say what each key buys).
**Win rates stay inside the +-10 points of 20 seeds in every cell** (one match = 5 points): the curve does not move, and nothing is tuned. **One-timers do not move:
0-0.1 a match before and after.** Section 6 says why, with the numbers.

## 5. Controls (pro / regular, 20 seeds, the final-build binary except where noted; `tools/playtest/out/a46-ctl-*.md`)

| cell | passes received | one-timers | win % | by human |
|---|---|---|---|---|
| both keys on | 2.0 | 0 | 55 | 1.9 |
| `--tune mate.pass_to_human=0` (reception on, share off; build 2's binary) | **2.0** | 0 | 45 | 1.6 |
| `--set stick.reception_m=0` (share on, reception off; build 4's binary) | **1.6** | 0 | 60 | 1.9 |

The body reception buys about +0.4 receptions a match; **the share buys nothing measurable**. Win rates are within noise of each other.

## 6. What the measurements say (for the Director)

1. **The human's teammates almost never have the puck.** A reception diagnostic on 40 playtester matches (regular player, 20 at pro and 20 at rookie; removed before the final
   build): the mates carried the puck **125 frames per match** (about 2 s), the human was open 5-25 m ahead in 23 of those frames, set up for a shot (inside 26 m, squared) in
   15, and the pass-to-human rule fired **5 times in 40 matches**. The yield rule (`yield_margin_m` 4, `yield_radius_m` 25: a teammate leaves the puck to the human whenever he
   is nearer) and the chaser / back roles mean the mates hold the puck only on a breakout or a loose puck in their own end. That is where every reception happens: **74
   receptions in 40 matches, all 41-82 m from the net** (the human at centre ice or in his own half), none inside the 28 m the regular playtester shoots a one-timer from
   (`ShootRangeM` 20 + 8), 19 of them with the human facing the net within 14 degrees. So the one-timer stays at 0-0.1: a reception in the slot never happens, not because the
   blade misses it (the body reception fixed that: the test scene one-times at 25.8 m/s) but because no teammate is there to pass.
2. **What would move the one-timer** (proposals, not applied, outside this lane's ownership): (a) the mates must carry in the attacking zone: a smaller `yield_radius_m` or a
   support role that carries up the far side while the human skates the slot (lane A3's brain roles, `yield_*` keys); (b) the playtester's support spot (`JamPlaytester.cpp`:
   12 m ahead of the carrier, 8 m off centre, facing its own travel) should square to the net inside the slot when the carrier is past the blue line, as a person waiting for
   a one-timer does (lane A34's instrument, so a measurement change to be ruled on); (c) the `pass` signal (R-0007 key 3) is the human's own lever in the real game: the share
   goes to 0.9 for 120 frames and the carrier turns to face him; the headless grid never presses it (no signal system headless), so its effect is unmeasured here.
3. **The rookie bar of the brief (2.5 received a match) is not met: 1.75.** At rookie the human carries 29 % of the match and wins 3-0 in about two minutes; the mates have the
   puck even less than at pro. The test asserts 1.5 at both levels, what the calibration delivers, so it guards regressions; the gap is this section's point 1.
4. **A pass off a teammate's body is a teammate's puck.** The sim's `LastTouch` moves to a skater whose body the puck bumps, so an opponent's shot that glances off a teammate
   and runs into the human within the same frame seats as a reception (lane A16's blade reception has the same property). Seen in the diagnostic a few times; left as is (it reads
   as a deflected pass) and noted here.
5. **Found and fixed on the way**: the first cut's settle let the carry assist pull a just-shot puck back onto the blade (the settle overrides the assist's relative-speed gate);
   a shot or a pass now spends the settle, and `Jam.Core.PassReception` checks the one-timer is still at 25.6 m/s a frame after it left.

## 7. Files (worktree a46 at main 0542360)

`Game/Source/JamCore/Public/JamSim.h`, `Private/JamSim.cpp` (body reception, settle, `PassesReceived`, hash), `Public/JamCartridge.h`, `Private/JamCartridge.cpp` (`reception_m`),
`Private/JamSchemaData.inl` and `Tools/parity.json` (regenerated), `schema/game.schema.json`, `schema/limits.json`, `schema/README.md`, `schema/DEFAULTS.md`,
`AI/JamAICore.h`, `AI/JamAICore.cpp` (`PassToHuman` share, `CarrierBestPass`, `Attack` / `Decide` carry `bPass`), `AI/JamSimAdapter.cpp` (`CarrierBestPass`),
`AI/DifficultyProfile.h`, `AI/DifficultyProfile.cpp` (`MatesPassShare`, tune key), `AI/DifficultyTests.cpp` (two comparisons renamed), `AI/JamPlaytester.h`, `AI/JamPlaytester.cpp`
(stat `passes_received`), `tools/playtest/run.mjs` (column `passes (rcv)`), `Private/Tests/JamPassTests.cpp` (new), `docs/experiments/PASSING.md` (this page), `docs/experiments/FEEL.md`
(the A46 rows), `status/A46.md` (main tree). Raw lines: `tools/playtest/out/a46-before.jsonl`, `a46-after.jsonl` (build 1), `a46-after2.jsonl` (final), `a46-ctl-noshare.jsonl`,
`a46-ctl-noreception.jsonl`.

## 8. How to reproduce

```
node tools/playtest/run.mjs --rinks regular --seeds 1-20 --out tools/playtest/out/grid                                          # the after table
node tools/playtest/run.mjs --rinks regular --seeds 1-20 --levels pro --players regular --tune "mate.pass_to_human=0"           # the share off
node tools/playtest/run.mjs --rinks regular --seeds 1-20 --levels pro --players regular --set "stick.reception_m=0"             # the body reception off
UnrealEditor-Cmd Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Core.PassReception+Jam.Bot.PassesToHuman; Quit" -nullrhi -unattended -nopause -stdout
```
