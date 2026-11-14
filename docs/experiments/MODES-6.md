# Director's choices, pass 6: five a side where the mode allows it, the puck in the Marathon, Arsenal's tricks (lane A156, track M1 evolution 4, device A, 2026-11-14)

Owner: lane A156. Earlier passes: `MODES.md` (A67), `MODES-2.md` (A81), `MODES-3.md` (A122), `MODES-4.md` (A139), `MODES-5.md` (A149).
Build: this worktree at main 3f5cbda (lane A149's twelve modes as landed, the menu's level wire, the brain's third evolution: for teams under five the human's
mates check the carrier and the back steps up; the playtester's check press of R-0060), editor Development.
Files changed: five cartridges under `Game/Content/Cartridges/modes/` (Arsenal, Glass House, Sudden Death, Marathon, Power Play), `modes/MODES.md`,
`tools/playtest/modes.mjs` (one column), `tools/soak/baseline.json` (the twelve mode cells), `tools/qa/accept/matrix-cuts/mode-*.json` (five of the twelve, generated).
**Fog of War was measured at five a side and, by the Director's ruling (04:06), kept at three: its file is 3f5cbda's, byte for byte** (section 1).
No sim, brain or profile code; no state field added; no test added or changed.

## How it was measured

As MODES-5: `node tools/playtest/modes.mjs --seeds 1-200` (regular class), `--seeds 201-300 --player novice` and `--player good`; each mode at its own level
next to `hockey.regular` at the same level, same seeds, same binary; three `UnrealEditor-Cmd` at a time, `-Multiprocess`. Bars: a third apart in the mode's own
number, every match ends, 15 to 85 % for the regular class, 10 % or more for the novice; for a mode that went to five a side the human's own passes at 2 a play
minute or more (the brief). Rounds r1 to r6 at 100 seeds a class (61 variants), the confirmation r7 at 200 a class (regular 1-200, novice 201-400), then the
table. Raw lines, variant cartridges and specs: `tools/playtest/out/a156/` (`base*` before, `final*` after, `r1..r7*`, `before/` the 3f5cbda files; does not travel).

The regular game on this build and these seeds: the regular class wins 49.5 % at pro (60.5 with sprint) and 13 at all_star; the novice 12 / 4; the good
player 71 (81 with sprint) / 27. The regular class passes 2.80 times a play minute and touches the puck 11.8 times.

## One table: all twelve on this build (regular class 200 seeds; novice and good 100; "before" = the files of 3f5cbda on the same build and seeds)

| mode | a side (before) | character number | mode / regular game = ratio (before) | win %, regular class (before) | win %, novice (before) | win %, good (before) | match s | human passes a play min (before) | 30 s without the puck, % of matches (before) |
|---|---|---|---|---|---|---|---|---|---|
| Arsenal | 5 v 5 (3 v 3) | human abilities used / match | **4.72** / 0 (0.79) | 58.5 (56.5) | 14 (23) | 80 (81) | 173 | 3.03 (0.76) | 6 (3) |
| Bruisers | 5 v 5 | hits (landed + taken) / play min | 5.31 / 3.71 = **1.43** | 56 | 23 | 63 | 74 | 2.75 | 2 |
| Cage | 4 v 4 | checks landed / match | 6.87 / 3.50 = **1.96** | 72.5 | 25 | 92 | 212 | 1.60 | 2 |
| Fog of War | 3 v 3 (kept; at 5 v 5, measured and not kept: 1.53x, 79 / 33 / 90, 3.22 passes) | human breakaways / play min | 1.94 / 0.62 = **3.11** | 79 | 48 | 92 | 103 | 0.69 | 3 |
| Glass House | 5 v 5 (3 v 3) | human box seconds / match | **11.45** / 0 (6.07) | 43.5 (50) | 14 (20) | 64 (69) | 174 | 2.38 (0.67) | 13 (11) |
| Marathon | 3 v 3 (2 v 2) | human sprint frames / play min | 417.7 / 260.6 = **1.60** (2.34) | 73.5 (77) | 16 (13) | 83 (96) | 408 | 1.12 (0.02) | **16** (71) |
| Outnumbered | 3 v 5 | shots against / play min | 6.39 / 3.81 = **1.68** | 57 | 23 | 82 | 97 | 1.19 | 1 |
| Pond | 5 v 5 | passes + received / play min | 7.34 / 4.19 = **1.75** | 69.5 | 19 | 80 | 218 | 3.20 | 0 |
| Power Play | 5 v 3 | passes + received / play min | 6.77 / 4.19 = **1.62** (1.41) | 59 (61) | 19 (25) | 90 (84) | 278 | 3.71 (3.19) | 1 (1) |
| Shotgun | 5 v 5 | knockdowns (all skaters) / play min | 7.37 / 4.40 = **1.67** | 46.5 | 14 | 76 | 109 | 2.77 | 0 |
| Sniper | 3 v 3 | human goals per human shot | 0.56 / 0.27 = **2.08** | 44.5 | 10 | 65 | 81 | 0.53 | 2 |
| Sudden Death | 5 v 5 (3 v 3) | match seconds | 47.1 / 175.6 = **0.27** (0.13) | 34.5 (18) | 18 (8) | 41 (31) | 47 | 2.76 (0.21) | 3 (3) |

`modes.mjs` exits 0: twelve of twelve a third apart, 0 unfinished of 2,400, the longest match in a mode 29,495 frames (Marathon; Glass House 28,218 at five a side).
Every win rate of the regular class is inside 15 to 85 (34.5 to 79); every novice at 10 or more (10 to 48). (The eleven other modes are the `final*`
run; Fog of War's row is the `base*` run, which its restored file replays 200 of 200: `fog3.jsonl`.)

**Not green, and said so:** Sniper's novice is 10 of 100 (on the floor; the file is untouched); Arsenal still has no trick in 13 of 200 matches (the brief
asks for every match); the good class wins Cage 92 %, Fog of War 92 % and Power Play 90 %; five a side cost the novice 6 to 9 points in Arsenal and Glass House
(they are still over the regular game's 12); Fog of War and Sniper keep the human at 0.5 to 0.7 passes a play minute; Marathon's human passes 1.1 times a play minute (it is three a side).

## 1. Five a side

The five that were three a side, each with nothing changed but the teams (the regular game's own layout: player, chaser, support, two backs against chaser,
two supports, two backs), 100 seeds a class:

| mode | number, ratio at five a side (at three) | win, regular class / novice | human passes a play min | does the mode's idea survive? |
|---|---|---|---|---|
| Arsenal | 1.36 tricks (0.79) | 48 / 18 | 2.94 | yes: more bodies, more boosts taken |
| Glass House | 12.8 box s (6.1) | 41 / 14 | 2.42 | yes: more hits to judge, twice the box |
| Fog of War | 0.95 / 0.68 = 1.41 (3.11) | 75 / 33 | 3.10 | **no** (the Director's ruling): five defenders close the open ice and the breakaways halve |
| Sniper | 0.40 / 0.29 = 1.39; **novice 1.08, 4 % wins** | 44 / 4 | 2.7 | **no** |
| Sudden Death | 51 s = 0.30 (23 s = 0.13) | 16 / 7 | 2.8 | yes: the match becomes one of hockey before the goal; the novice needs help at either size |

- **Arsenal, Glass House and Sudden Death are five a side now.** The human's passes there: 3.03 / 2.38 / 2.76 a play minute (0.76 / 0.67 / 0.21 before; the
  regular game 2.80).
- **Fog of War stays three a side: measured at five, not kept.** At five a side every bar of the brief held at 200 seeds (1.53x, 79 %, the novice 32.5 % of
  200, 3.22 passes a play minute), but the number that is the mode halves (1.94 breakaways a play minute to 0.96; in the first minute 1.47 to 0.83, 2.3x to
  1.2x the regular game's; the human scoring in 74 % of first minutes to 53 %). The Director's ruling: the mode's own idea is the slip-by, five a side buys
  passes with exactly that, and the operator is sent there as "the stop-and-go mode": the idea does not survive, so the 3f5cbda file stands.
  Making the bots slower to bring the breakaways back leaves the win band first: `ai.react_frames` 6 / 9 read 1.45x / 1.63x with 88 % wins; a bigger goalie
  (reach 1.2, block 0.55) is no counterweight (87 to 88 %); 100 x 50 ice gives nothing (1.42x, 80 %); four a side reads 2.03x (81 %, the novice 45) with 1.17 passes.
  No lever found gives five a side its breakaways back inside the win band.
- **Sniper stays three a side, and why**: its idea is a lane to a big net, and five defenders close it. At five a side: 1.39x as it is, 1.37x with the
  goalie's reach at 0.6, 1.41x with the block height at 0.08, 1.26x with the shot at 30 m/s, 1.32x / 1.26x on 100 x 50; the novice's number is 1.03 to 1.46x
  and he wins 4 to 14 %. Four a side works as a mode (1.93x, 42 % / 15 %, the novice's number 2.65x) with 1.14 passes a play minute: under the bar, so not
  taken; it is next evolution 1.
- **Sudden Death did not need to stay three.** At five a side on the big sheet the match lasts 47 s instead of 23 (a 23 s match holds 0.08 passes), still
  0.27x the regular game's length. Its novice was under the floor on this main at any size (8 / 9 / 7 % at three / four / five a side): `ai.hold_radius_m` is
  the lever (three a side: 22 % at 2 m, 27 at 3; five a side, of 100: 5 at 2 m, 18 at 2.5, 18 at 3, 14 at 3.5; of 200: 16.5 at 2.5, 13 at 3). Shipped: five a side, hold 2.5: the regular
  class 34.5 %, the novice 16.5 % of 200 (2.5 standard errors over the floor; brief item 4).

## 2. Marathon's puck

On this main the Marathon as landed held a 30 s stretch without the puck in 71 % of matches (84 on the last main: the brain's step-up helps a two-a-side
team). **`puck.count` is not a lever**: the loader reads it (`JamCartridge.cpp:1142`) and no sim does (the only other mention in `Game/Source` is a default
test, `JamFeelDepthTests.cpp:81`): one puck is played whatever the key says, in the tournament path and in the game.

| variant (100 seeds; base on the same seeds: 76 %, 74 % of matches) | sprint ratio | win, regular class / novice | 30 s without the puck | touches a play min |
|---|---|---|---|---|
| a chaser for a mate (2 v 2, nobody at home) | 2.01 | 84 / 9 | 23 % | 9.2 |
| a support for a mate | 2.12 | 78 / 7 | 56 % | 8.4 |
| three a side: chaser and back / chaser and support | 1.85 / 1.78 | 92 / 28; 71 / 14 | 57 % / 48 % | 8.3 / 9.0 |
| `ai.yield_margin_m` 0.5 / 6; hold 4.5 | 2.31 / 2.33 / 2.49 | 77 / 79 / 87 | 75 / 74 / 71 % | 7.2 |
| chaser mate + the bots' chase sprint 0.5 / 0.75 | 1.88 / 1.80 | 86 / 85; 13 / 12 | 31 / 30 % | 9.5 |
| chaser mate + `skater.carry_speed_mult` 0.8 (hold 2.5 / 3) / 0.7 | 2.16 / 2.31 / 2.14 | 96 / 100 / 99; 20 / 20 / 47 | 14 / 12 / 6 % | 10.3 |
| chaser mate, hold 3 + sprint bar 150 frames / clock 4:00 | 2.10 / 2.21 | 93 / 87; 13 / 22 | 17 / 19 % | 9.3 |
| **three a side (chaser, support) + carry 0.8**, hold 2.5 / 2 / 1.5 | 1.80 / 1.75 / 1.58 | 79 / 89 / 77; 31 / 22 / 16 | **18 / 12 / 11 %** | 10.1 to 10.7 |

Two levers give the puck: **a mate that hunts it** (a back for a mate waits at home on 140 m of ice) and **a carrier that can be caught**
(`skater.carry_speed_mult` 0.8: the puck slows whoever has it, a bot too). At two a side the two together push the regular class to 96 to 100 %; at three a
side with the opposing back at home they hold. At 200 seeds a class: hold 2.5 reads 83 % (novice 30, good 93, 1.81x, 14 % of matches); **hold 1.5, shipped:
73.5 % (novice 19 of 200, good 83), the sprint 417.7 / 260.6 = 1.60x, a 30 s stretch without the puck in 16 % of matches** (the bar under half), touches
10.5 a play minute (7.2). The price: the sprint number falls from 2.34x to 1.60x (the bar 1.33) and the mode is three on three, which its pitch now says.

## 3. Arsenal's tricks

| variant (five a side, 100 seeds, regular class) | tricks a match | matches with none, of 100 | win, regular class / novice |
|---|---|---|---|
| as landed, at five a side | 1.36 | | 48 / 18 |
| a trick has 2 / 3 charges, 3 s apart (`abilities[*].charges`, `cooldown_frames` 180) | 2.14 / 2.74 | 23 (at 3) | 47 / 18; 53 / 19 |
| 4 charges, 2 s apart | 3.38 | 23 | 51 / 23 |
| 3 charges + boosts only in a 50 x 24 lane in the middle (`rink.zones` `boost_only`, `pickups[*].zone`) | 3.90 | 16 | 56 / 15 |
| 3 charges + a 70 x 14 lane | 4.32 | 7 | 62 / 15 |
| **3 charges + the 70 x 14 lane + boosts every 120 frames** (were 180 / 240) | **4.70** | **5** | 62 / 14 |

Charges multiply what a boost is worth; the lane puts a boost under his feet (matches with no trick 23 to 7 of 100). At 200 seeds: **4.72 tricks a match
(0.79), three or more in 156 of 200, none in 13 (96 before)**, 58.5 % (novice 14.5 of 200), the novice 3.6 tricks a match. The lane is not painted
(`visible` false): the boosts show where it is; whether a painted lane reads better is a look question for a window.
**Not reached: every match at least one.** The brief's "one ability held from the faceoff" is a `mode` entry beside the four `pickup` entries; the schema
allows it, but `Jam.Core.Abilities` (`JamAbilityTests.cpp:198-199`) asserts Arsenal has exactly four entries, all `pickup`: a test outside this family
(finding 2).

## 4. Power Play's third

| variant (100 seeds; the regular game 3.97 on these seeds) | ratio | win, regular class / novice | one-timers a match |
|---|---|---|---|
| as landed (70 x 35, carry 0.8) | 1.49 | 61 / 25 | 0.62 |
| carry 0.7 / 60 x 30 / both | 1.56 / 1.51 / 1.66 | 66 / 65 / 59; 22 / 19 / 14 | 0.86 / 1.23 / 1.18 |
| **60 x 30, carry 0.65** | **1.76** | 52 / 19 | 1.07 |
| 56 x 28, carry 0.7 / 5 v 4 | 1.57 / 1.35 | 65 / 27; 19 / 10 | |

Shipped: 60 x 30 with the carrier at 65 % of a free skater's speed: 6.77 / 4.19 = **1.62x at 200 seeds** (1.41x before; the bar 1.6), 59 % (novice 19),
one-timers 1.25 a match (0.76), the human passes 3.71 times a play minute. The good class reads 1.40x (it passes 5.2 a minute in the regular game) and wins 90 %.

## 5. The cards (`game.meta`; the schema caps a fact at 20 characters now)

| mode | pitch | facts | changed |
|---|---|---|---|
| Arsenal | Grab a boost, get a trick on F. | boosts give F tricks / each trick fires 3x / boosts down mid-ice | two facts |
| Marathon | Three on three on endless ice. Bring your legs. | 140 x 70 m, 6:00 / sprint +40 % for 4 s / carry at 80 % speed | pitch ("Two on two"), the back's fact -> the carrier's |
| Power Play | Five on three. Move it, fire it, repeat. | 5 v 3 for you / carry at 65 % speed / one-timers, 4:00 | 80 -> 65 |
| Sudden Death | One goal ends it. The meanest bots alive. | first goal wins / 5 v 5 on 120 x 60 m / no grace on a steal | 3 v 3 -> 5 v 5 |

Glass House's card never named a team size and is unchanged; Sniper's still says "3 v 3, shot 26 m/s" and Fog of War's "3 v 3, night, fog": both true. `Jam.Menu.FitsTheScreen` green: widest
name 210, widest pitch 432 (of 458: Marathon's 47 characters), widest fact 196 (of 262). Every line through `tools/barks/filter.mjs` `checkLine`: clean.

## 6. The soak's baseline and the acceptance matrix

- `node tools/soak/drift.mjs --only modes --update-baseline --max 3 --chunk 100`: 1,200 matches in 182 s, 0 faults, the twelve `m-*` cells written; after the ruling Fog of War's cell alone was re-taken on its restored file (81 % of 100, 2.0 breakaways a play minute;
  the other 35 cells byte-identical) (Marathon's
  30 s share 84 -> 11 % of its 100 matches, its sprint 593.6 -> 412.4; Arsenal 0.86 -> 4.7 tricks; Sudden Death 28.8 -> 41.7 s); the 24 take cells are
  byte-identical to before (still 41df3e6's), and the `build` field says which is which. A drift run over the same output: 12 compared, 12 bit-identical, exit 0.
- `node tools/qa/accept/make-matrix-cuts.mjs`: the twelve `mode-*.json` regenerated; five differ from the main checkout's (arsenal, glass-house,
  marathon, power-play, sudden-death), the other seven and the five tier cuts are byte-identical.

## 7. The first minute of the three modes `docs/PLAY-ME.md` names now (regular class, 200 seeds, stopped at 3,600 frames)

The regular game's first minute: 11.5 touches, 2.5 passes, 0.68 breakaways, 3.5 shots against, 4.4 bodies down, the human scoring in 40 %.
- **Shotgun: yes.** F fired 2.4 times, in 200 of 200 first minutes; 6.6 bodies down (1.5x).
- **Outnumbered: yes.** The shield on F 4.2 times, in 200 of 200; 5.3 shots against (1.5x); he scores in 65 %.
- **Fog of War (three a side, as shipped): yes, on the last build's reading.** 1.47 breakaways in the first minute (2.3x) and the human scoring in 74 % of
  first minutes were read by lane A149 on 322e6c5 with the same file; this build's first minute at three a side was not re-run (its whole match here: 3.11x).
  At five a side, measured here and not kept: 0.83 breakaways in the first minute (1.2x), scoring in 53 %.

## Revert path

Content only. The three regular cells and the six untouched modes (Bruisers, Cage, Outnumbered, Pond, Shotgun, Sniper) replay 200 of 200 identical hashes
between the before and the after run: nothing the regular game or the ladder reads moved. Each mode reverts by restoring its file from 3f5cbda, or by key:
- Arsenal: three a side (player, chaser, back v chaser, support, back; spawns +-7.5 / +-22.5), no `rink.zones`, no `pickups[*].zone`, `spawn_every_frames`
  180 / 240, no `charges` and no `cooldown_frames` on the four abilities; facts "a boost every 3 s", "up to 8 on the ice".
- Glass House: three a side (the same layout). (Fog of War is 3f5cbda's file already.)
- Sudden Death: three a side (spawns +-10 / +-30), no `ai.hold_radius_m`; fact "3 v 3 on 120 x 60 m".
- Marathon: 2 v 2 (player, back v chaser, back; spawns +-11.67 / +-35), no `skater.carry_speed_mult`, `ai.hold_radius_m` 2.5; pitch "Two on two on endless
  ice. Bring your legs.", facts "140 x 70 m, 6:00", "2 v 2, a back each", "sprint +40 % for 4 s".
- Power Play: rink 70 x 35 with corner 3.5, `skater.carry_speed_mult` 0.8, the spawns of 3f5cbda; fact "carry at 80 % speed".
- `tools/soak/baseline.json`: the twelve `m-*` cells from 3f5cbda's file; `tools/qa/accept/matrix-cuts/`: re-run the generator.

## Findings outside the family

1. `Game/Source/JamCore/Private/JamCartridge.cpp:1142` reads `puck.count` into the config and nothing plays it (`schema/game.schema.json:588` offers 1 to 3,
   "Number of pucks on the ice"; `schema/DEFAULTS.md:69`). A cartridge with `puck.count` 2 validates, loads and plays one puck. Either the schema's maximum goes
   to 1 with the sentence "not implemented", or the key leaves the schema.
2. `Game/Source/JamCore/Private/Tests/JamAbilityTests.cpp:198-199` pins Arsenal to exactly four abilities, all `pickup`. One `mode` entry held from the faceoff
   (the schema: "at most one mode entry") would give every Arsenal match a trick (13 of 200 have none today). The test would read "four pickup entries and at
   most one mode entry".
3. `Game/Source/JamCore/AI/JamPlaytester.cpp:49-67`: the novice still has no `SprintPct` (MODES-5 finding 2): Marathon's number is 0 against 0 for the class and
   `modes.mjs --player novice` exits 1 on that row alone.
4. `schema/game.schema.json` `ai.hold_radius_m` says "Opposing non-back bots hold when the human is within this distance of the puck": it is now the newcomer's
   lever in four modes (Bruisers 1, Marathon 1.5, Outnumbered 2, Sudden Death 2.5) and worth 10 to 14 points to the novice per mode. The season's takes 4 to 7
   (R-0057 gives them one `ai.red_skill` each) could use it: a curve-lane question, not an edit.
5. `skater.carry_speed_mult` on big ice: at 0.8 a two-a-side match has a 30 s stretch without the puck in 14 % of matches instead of 74 (section 2): a carrier
   that can be caught is the brain track's "the human's side takes a kept puck back" (R-0055 point 5) by a cartridge key. `hockey.arena.large` plays 0.92.
6. The playtest line (`thirdjam.playtest/1`) still has no board-hit count and no knockdown cause (MODES-5 finding 5).
7. `tools/soak/baseline.json`: the 24 take cells are still 41df3e6's (the file's `build` field says so): a full drift run flags the season's takes until
   somebody runs `drift.mjs --only takes --update-baseline` on main (MODES-5 finding 4 stands).
8. One regular game in 600 (pro with sprint, seed 48) ends at the sim's 30,000-frame stop as a 1 : 2 loss in a first-to-three (`over` true): the hard stop of
   R-0037 is reached in the regular cut by the regular class. A line for the curve lane, no edit.
9. `tools/barks/barks.test.mjs:63` still fails on CRLF (`BarkData.inl`): the gate's node step reads 243 of 244 here, as before this lane.

## Proof

- Before and after: the table above (`tools/playtest/out/a156/base*.jsonl`, `final*.jsonl`, `final-table.md`); `modes.mjs` exit 0 before and after.
- Determinism: the five changed modes replay round 7's 200 of 200 hashes in the final run; the three regular cells and the six untouched modes replay the base
  run's 200 of 200; the restored Fog of War replays the base run's 200 of 200 (`fog3.jsonl`).
- `node schema/validate.mjs` on the twelve mode files: ok, 0 errors each.
- The full `Jam` automation filter on the final tree, the gate's own invocation (`Automation RunTests Jam; Quit`, `-nullrhi -Multiprocess -unattended`):
  **237 of 237 Success**, exit code 0 (`Game/logs/a156-jam-1.log`, 03:52), run with Fog of War at five a side and the other five files final; among them
  `Jam.Menu.FitsTheScreen`, `Jam.Menu.DirectorsChoices` (twelve cards), `Jam.Core.CartridgeRoundTrip`, `Jam.Core.CartridgeParity`, `Jam.Core.ContactModes`,
  `Jam.Core.Abilities.*`, `Jam.Net.GuestSeat` and the other `Jam.Net` tests that load Arsenal and Shotgun. After the ruling the one file the engine reads that
  changed is Fog of War, back to 3f5cbda's bytes (the file main's own gate passed): `Jam.Menu`, `Jam.Core.Cartridge*` and `Jam.Core.ContactModes` (which
  plays it) re-run on the tree as it lands: 17 of 17 Success, exit code 0 (`Game/logs/a156-jam-fog3.log`, 04:09); the full filter was not run again.
- Node tests on the gate's globs: 243 of 244 (finding 9).
- The hash: nothing under `modes/` reaches `hockey.regular`; `Jam.Core.Determinism` is green inside the filter and the regular cells of the playtest runs are
  hash-identical before and after. No test pins a shipped cut's default match here, and none was added.
- Not run: the acceptance run (cartridges, landed before the 08:00 line of R-0056; the modes' packaged pass is track S2's matrix on the regenerated cuts),
  the three curve cells (nothing the ladder reads changed), a Shipping compile (no source changed).

## Next evolutions for the track, ranked

1. **Sniper's lane at more than three a side**, and its novice off the floor: goals per shot still 1.33x or more and the novice at 14 % or more (10 today)
   with the human's passes at 2 a play minute (0.53 today; four a side reads 1.93x, 42 % / 15 %, 1.14 passes). Untried: four a side with two supports, a
   60 m sheet with a 6 m net, a no-shoot zone that keeps defenders out of the slot.
2. **Arsenal: every match a trick**: no match without one (13 of 200 today), with the count still 3 or more: one `mode` ability held from the faceoff
   (finding 2: one test line), or a boost dropped at the faceoff dot.
3. **The good class in the modes**: Cage 92 %, Fog of War 92 %, Power Play 90 % for a good player; each under 85 % with its third and its novice kept.
   Levers seen tonight: `ai.react_frames` (Fog), the rink's size (Power Play), `ai.body_check` (Cage).
