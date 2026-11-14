# Director's choices, pass 5: twelve modes on the set ladder, the whistle out of eleven (lane A149, track M1 evolution 3, device A, 2026-11-14)

Owner: lane A149. Earlier passes: `MODES.md` (A67), `MODES-2.md` (A81), `MODES-3.md` (A122), `MODES-4.md` (A139).
Build: this worktree at main 322e6c5 (the brain's second evolution, the shove rule, the standing start, the whistle out of the regular game, the ladder set:
`CURVE-7.md`), editor Development. Rulings: R-0048 point 2, R-0059 (`rules.penalties` false in every mode except Glass House).
Files changed: eleven cartridges under `Game/Content/Cartridges/modes/` (nine re-tuned, two card text only; Power Play untouched), `modes/MODES.md`,
`tools/playtest/modes.mjs` (a column and the novice's floor), `tools/soak/baseline.json` (the twelve mode cells), `tools/qa/accept/matrix-cuts/mode-*.json`
(generated). No sim, brain or profile code; no state field added.

## How it was measured

`node tools/playtest/modes.mjs --seeds 1-200` (regular class), `--seeds 201-300 --player novice` and `--player good`: each mode at its own level next to
`hockey.regular` at the same level, same seeds, same binary; one `UnrealEditor-Cmd` a cell, three at a time, `-Multiprocess`. A mode passes when its character
number is a third or more from the regular game's and every match ends; the win band is 15 to 85 % for the regular class and 10 % or more for the novice
(brief item 2). Variant rounds r1 to r4 at 100 seeds a class (66 variants), the confirmation round r5 (six) and the before and after tables at 200.
Raw lines, the variant cartridges and their specs: `tools/playtest/out/a149/` (`base*` before, `final*` after, `r1..r5*`, `before/` the 322e6c5 files; the
folder does not travel).

The regular game on this build and these seeds: the regular class wins 99 / 51 / 9 % at rookie / pro / all_star (48.5 at pro with sprint), the novice
91 / 15 / 3, the good player 100 / 81 / 30. (The curve lane's cells are on its own seeds: 85 / 49 / 26 to 30.)

## One table: all twelve on this build (regular class 200 seeds; novice and good 100; "before" = the files of 322e6c5 on the same build and seeds)

| mode | level (before) | character number | mode / regular game = ratio (before) | win %, regular class (before) | win %, novice (before) | win %, good (before) | match s | human down a match (a play min; regular game) | 30 s without the puck, % of matches (before) |
|---|---|---|---|---|---|---|---|---|---|
| Arsenal | pro (rookie) | human abilities used / match | **0.94** / 0 (0.42) | 53 (95) | 20 (72) | 61 (100) | 103 | 4.5 (2.98; 2.52) | 6 (1) |
| Bruisers | pro (rookie) | hits (landed + taken) / play min | 5.15 / 3.67 = **1.40** (1.55) | 40.5 (97) | 16 (79) | 58 (99) | 81 | 3.8 (3.08; 2.52) | 2 (1) |
| Cage | pro | checks landed / match | 7.02 / 3.15 = **2.23** (2.23) | 61 (61) | 25 (25) | 82 (82) | 211 | 6.9 (2.25; 2.52) | 3 (3) |
| Fog of War | pro | human breakaways / play min | 1.83 / 0.67 = **2.73** (3.05) | 74 (81.5) | 40 (47) | 82 (91) | 101 | 5.1 (3.43; 2.52) | 2 (5) |
| Glass House | pro | human box seconds / match | **7.13** / 0 (7.13) | 53 (53) | 15 (15) | 62 (62) | 106 | 4.2 (2.77; 2.52) | 13 (13) |
| Marathon | pro, sprint | human sprint frames / play min | 593.5 / 263.6 = **2.25** (1.60) | 77.5 (60) | 13 (6) | 100 (88) | 410 | 9.5 (1.57; 2.55) | 84 (91) |
| Outnumbered | pro | shots against / play min | 7.01 / 3.87 = **1.81** (1.55) | 57.5 (41) | 15 (8) | 77 (62) | 96 | 2.0 (1.46; 2.52) | 2 (5) |
| Pond | pro (rookie) | passes + received / play min | 7.16 / 4.17 = **1.72** (1.25, failed) | 66.5 (98) | 18 (80) | 89 (100) | 216 | 0 (0; 2.52) | 2 (0) |
| Power Play | pro | passes + received / play min | 6.05 / 4.17 = **1.45** (1.45) | 65.5 (65.5) | 18 (18) | 80 (80) | 276 | 12.1 (2.97; 2.52) | 2 (2) |
| Shotgun | pro | knockdowns (all skaters) / play min | 7.61 / 4.31 = **1.77** (1.44) | 49.5 (49) | 17 (17) | 70 (66) | 108 | 4.8 (3.04; 2.52) | 0 (1) |
| Sniper | pro | human goals per human shot | 0.58 / 0.25 = **2.29** (2.24) | 36.5 (30.5) | 14 (4) | 60 (63) | 78 | 4.0 (3.60; 2.52) | 3 (6) |
| Sudden Death | all_star | match seconds | 29.5 / 172.0 = **0.17** (0.17) | 21 (18.5) | 11 (10) | 38 (37) | 30 | 0.6 (1.36; 2.56) | 10 (9) |

`modes.mjs` exits 0: twelve of twelve a third apart, 0 unfinished of 2,400, the longest match 30,549 frames (Marathon). Every win rate of the regular class is
inside 15 to 85 (21 to 77.5); every novice is at 10 or more (11 to 40). Before: three out of the band (Arsenal 95, Bruisers 97, Pond 98), three novices
under 10 (Sniper 4, Marathon 6, Outnumbered 8), one mode under a third (Pond 1.25x), seven files with the whistle.

**Not green, and said so:** Marathon still holds a 30 s stretch without the puck in 84 % of matches (the regular game 6 %); the good class wins Marathon
100 % and Pond 89 %; Power Play's third is thin (1.45x); Sudden Death's novice is 11 % of 100 matches (one standard error over the floor).

## 1. Level names: no mode is rookie any more

On the set ladder a newcomer wins 85 % at rookie, and the regular class 99 %: a mode at rookie cannot be inside 15 to 85 for the regular class unless it is
re-made to be much harder than the game it is cut from. All three rookie modes were measured at both names (regular class, novice in brackets; at rookie Arsenal with the whistle off at
100 seeds, Bruisers and Pond the base run; at pro 100 seeds):

| mode | at rookie, as it stood | at pro, nothing else changed | the name it carries | what else turned |
|---|---|---|---|---|
| Arsenal | 96 (88), 0.45 tricks a match | 45 (16), 0.50 | **pro** | boosts every 180 / 240 frames with four of each kind on the ice (were 480 / 720, two of each): 0.94 tricks a match at 200 seeds, 79 of 200 matches with none (134 before), 53 % (novice 20). The pace alone (240 / 360, two on the ice) gave 0.50: the count on the ice is the lever. A stiffened rookie (bots at speed 1, skill 1) read 82 % with 0.34 tricks: not taken. |
| Bruisers | 97 (79), 1.55x | 52 (12), 1.33x: on the line, as A139 found (1.28x) | **pro** | re-made, section 3 |
| Pond | 98 (80), **1.25x: failed** | 56 (18), **1.74x** | **pro** | nothing: the rookie game itself now passes 6.9 times a play minute (the brain's mates pass to the human in 0.6 of their chances at rookie, 0.4 at pro), the pro game 4.2 |

Eleven modes are pro, Sudden Death is all_star. A mode carries the name only: no level number is written into any file.

## 2. The whistle (R-0059): out of six more files; Glass House is the only one left

`hockey.regular` lost `rules.penalties` at the curve's landing; Arsenal, Fog of War, Outnumbered, Shotgun, Sniper and Sudden Death were cut from it before and
still carried `true`. Each with only that key turned (100 seeds a class, the same seeds before and after):

| mode | matches with a foul, of 100 (before) | fouls a match; the human in the box, s a match (before) | win %, regular class: before -> after | win %, novice: before -> after | character number: before -> after (ratio to the regular game) |
|---|---|---|---|---|---|
| Arsenal (rookie) | 57 | 0.91; 4.5 | 95 -> 96 | 72 -> 88 | 0.41 -> 0.45 |
| Fog of War | 45 | 0.60; 3.0 | 82 -> 82 | 47 -> 50 | 2.06 -> 1.93 (3.17 -> 2.95) |
| Outnumbered | 53 | 0.79; 3.9 | 40 -> 47 | 8 -> 9 | 6.09 -> 5.67 (1.53 -> 1.43) |
| Shotgun | 44 | 0.62; 3.1 | 56 -> 50 | 17 -> 11 | 6.12 -> 5.77 (1.45 -> 1.37) |
| Sniper | 35 | 0.46; 2.3 | 31 -> 34 | 4 -> 4 | 0.59 -> 0.57 (2.22 -> 2.12) |
| Sudden Death | 8 | 0.09; 0.5 | 19 -> 22 | 10 -> 11 | 28.6 -> 28.8 s (0.16 -> 0.16) |

Read: the whistle was called in a third to a half of the matches of five of the six; taking it out moves the regular class by 0 to 7 points (no mode left the
band on that account) and thins the hitting and pressure numbers by 5 to 7 %: Shotgun fell to 1.37x and Outnumbered to 1.43x, which is why both were re-made
(section 3). Fog of War stood at 82 %: `ai.react_frames` 6 -> 4 (68 % at 100 seeds, 74 at 200; the breakaways 2.73x).
Glass House keeps `rules.penalties` true with its 10 s box, and its card says so (section 4). In its first minute a foul is called in one match in three.

## 3. What faded, what was re-made, and the novice's floor

Levers found (each a cartridge key of the mode; the numbers are 100 seeds, regular class, novice in brackets, unless 200 is said):

- **`ai.hold_radius_m`** (the opposing forwards hold off a puck the human is within this many metres of) is the lever for the newcomer and, on big ice, for
  the legs. Marathon: 2 / 3 / 4.5 m read 577 / 624 / 644 sprint frames a play minute against 421 (the human sprints to pucks he can now win) with wins of
  75 / 85 / 88 (7 / 17 / 17). Outnumbered: 2 m gave the novice 19 % (9) but took the pressure down to 1.31x; Bruisers: 0 / 1 / 2 m gave the novice
  12.5 / 17.5 / 25 % at 200 seeds.
- **Tight ice gives a crowd its number back.** Outnumbered with the hold on 90 x 45 / 80 x 40 / 70 x 35: 1.31x / 1.61x / 1.82x. Shotgun on 56 x 28: 1.46x
  (64 x 32: 1.37x). Bruisers on 70 x 35: 1.37x (80 x 40: 1.33x).
- **The bots' speed is too coarse for the novice's floor**: Marathon at 0.97 / 0.95 / 0.9 read 73 / 80 / 98 for the regular class and 5 / 9 / 33 for the
  novice: no value holds both ends. The bots' chase sprint costs the regular class only (83 without it, 60 with a quarter) and the human's own sprint number
  (1.27x with the bots at 0.9 and `ai.sprint` 1). `ai.red_skill` cannot be set between the schema's cap of 1 and the big rink's profile value of 1.12 (finding 3): 1 read 99 %.
- **The pro bot checks on every chance already**: `ai.body_check` 0.5 in Shotgun took the bodies on the ice down to 0.93x. So `ai.body_check` 1 on Bruisers
  and Glass House is the pro default now and says nothing; Cage's 0.5 is a real key.
- No lever: the release grace (again), Sniper's `goalie.block_height_m` 0.25 (33 %, novice 5), Outnumbered's `ai.sprint` 0.4 and a longer shield, Pond's
  `puck.drag` 0.3, Sudden Death's wider nets (22 % either way; novice 14 / 12 against 11: inside the noise), Marathon's shorter clock or smaller ice alone
  (novice 6 and 0).

What each re-made mode is now (200 seeds; novice 100 unless said):

| mode | change | number | win, regular class | win, novice |
|---|---|---|---|---|
| **Bruisers** | pro; 70 x 35 (was 80 x 40); `contact.check_reach_m` 2 (the human's check turns onto the nearest standing opponent within 2 m: the regular game has none); first to 2; `ai.hold_radius_m` 1 | 5.15 / 3.67 = 1.40x (novice 1.50x) | 40.5 | 16 (17.5 of 200) |
| **Shotgun** | whistle off; 56 x 28 (was 64 x 32); `skater.check_speed_min` 2 (under the momentum law hits count 2.5x) | 7.61 / 4.31 = 1.77x (novice 2.0x) | 49.5 | 17 |
| **Outnumbered** | whistle off; 70 x 35 (was 90 x 45); `ai.hold_radius_m` 2 | 7.01 / 3.87 = 1.81x (novice 1.88x); carries of 3 s 0.79 a play minute against 0.56 | 57.5 | 15 (18 of 200) |
| **Marathon** | clock 6:00 (was 8:00); `ai.hold_radius_m` 2.5 | 593.5 / 263.6 = 2.25x; 348 of the 594 are with the puck | 77.5 | 13 (15.5 of 200) |
| **Sniper** | whistle off; `shot.lift.half_frames` 10 (was 20), `shot.charge_frames_max` 40 (was 60): the lift comes sooner | 0.58 / 0.25 = 2.29x | 36.5 | 14 (4 before: the novice lets go early, and a tap stays under the pads) |
| **Fog of War** | whistle off; `ai.react_frames` 4 (was 6) | 1.83 / 0.67 = 2.73x | 74 | 40 |
| **Arsenal**, **Pond** | section 1 | 0.94 tricks; 1.72x | 53; 66.5 | 20; 18 |
| **Sudden Death** | whistle off | 0.17x | 21 | 11 |

Marathon's choice between hold 3 (81.5 %, novice 21.5, 2.38x at 200 seeds) and hold 2.5 (77.5 / 15.5 / 2.25x): 2.5, both ends two standard errors inside.
Its 30 s without the puck is 84 % of matches at any of these (78 to 88 across the variants): the hold radius gives the human the puck he reaches, not a
puck nearer to him. The novice playtester never presses sprint (finding 2), so the mode's number is 0 against 0 for that class and its win rate is that of a
player who ignores the mode's verb.

## 4. The cards (`game.meta`)

"No whistle" is the regular game now: Pond, Cage, Bruisers and Marathon each carried it as a fact and Bruisers in its pitch.

| mode | pitch | facts | changed |
|---|---|---|---|
| Pond | Pond hockey. Pass, skate, nobody gets hit. | 5 v 5 on 60 x 30 m / 3:00, nets 6 m wide / catch passes at 3 m | "no hits, no box" -> the nets (the pitch says the hits) |
| Cage | Tight box, eight bodies. The boards hit back. | 60 x 28 m, 4 v 4 / boards hit 2x / 4 pillars on the ice | "no whistle, no box" -> the pillars |
| Bruisers | Five a side, heavy bodies. Hit everything. | 5 v 5 on 70 x 35 m / 130 kg, first to 2 / checks find the man | pitch ("no whistle"), all three |
| Marathon | Two on two on endless ice. Bring your legs. | 140 x 70 m, 6:00 / 2 v 2, a back each / sprint +40 % for 4 s | the clock; "no whistle" -> the back |
| Glass House | The only whistle left. Foul and sit 10 s. | no hits from behind / hit the carrier only / hits 1.7x harder | pitch; two facts say what the whistle calls ("bots always hit" is the pro default) |
| Arsenal | Grab a boost, get a trick on F. | boosts give F tricks / a boost every 3 s / up to 8 on the ice | "4 abilities", "rookie bots" |
| Fog of War | Slow bots in the fog. Slip by, then hit. | bots 0.07 s late / hits 5x harder / 3 v 3, night, fog | the delay; three a side |
| Shotgun | Tight ice, ten bodies. F clears the lane. | F: shotgun shot / back every 20 s / 56 x 28 m, 5 v 5 | the rink |
| Sniper | Big net, low guard. Lift it, bury it. | net 1.22 m high / pads at 0.15 m / 3 v 3, shot 26 m/s | three a side |
| Sudden Death | One goal ends it. The meanest bots alive. | first goal wins / 3 v 3 on 120 x 60 m / no grace on a steal | three a side |

Pitches 31 to 45 characters, facts 13 to 20. `Jam.Menu.FitsTheScreen` green: widest name 210, widest pitch 417 (of 458), widest fact 191 (of 262). Every line
through `tools/barks/filter.mjs` `checkLine`: clean (it trips on three titles and on "Arsenal", as before). Every fact was checked against its file.

**Three a side.** The regular game is five a side; Arsenal, Fog of War, Glass House, Sniper and Sudden Death are still three a side (cut from the regular game
when it was). Three cards now say so; Arsenal's and Glass House's facts are full. Whether they should be five a side is next evolution 1.

## 5. The first minute of the three modes `docs/PLAY-ME.md` names (regular class, pro, 200 seeds, stopped at 3,600 frames)

The regular game's first minute: 11.3 touches, 3.7 passes and receptions, 1.0 checks landed, 4.5 bodies down, the human down 2.7 times and scoring in 33 %.

- **Shotgun: yes.** F is fired 2.3 times, in 200 of 200 first minutes; 6.5 bodies go down (1.43x); the human scores in 61 %.
- **Cage: yes for the hits, not shown for the boards.** 2.0 checks landed (2x), one or more in 84 % of first minutes (64 %), the human down 1.7 times (2.7);
  the playtest line counts no board hit, so "the boards hit back" is unproven headless (finding 5).
- **Pond: yes.** A pass or a reception in 200 of 200 (5.8 in the minute, 1.55x), 13.9 touches, no check, nobody down.

**Another three?** Keep Shotgun; put **Fog of War** and **Outnumbered** before Cage and Pond if the ten minutes are tight. Fog of War is the mode of the skill
the guide asks him to judge (1.47 breakaways in the first minute, 2.3x; he scores in 74 % of first minutes; the friendliest mode to a newcomer at 40 %), and
Outnumbered is "keep the puck" with a second F (the shield, 4.2 uses in the first minute; 6.2 shots against, 1.76x); both end in about 100 s where Cage and
Pond are three-minute clocks. Pond is the one mode that switches off the hit, which is the game's first law.

## 6. The soak's baseline and the acceptance matrix

- `node tools/soak/drift.mjs --only modes --update-baseline --max 3 --chunk 100`: 1,200 matches in 102 s, 0 faults, the twelve `m-*` cells written; the 24
  take cells are byte-identical to before (they are still 41df3e6's: finding 4), and the file's `build` field says which is which. A drift run over the same
  output: 12 compared, 12 bit-identical, exit 0.
- `node tools/qa/accept/make-matrix-cuts.mjs`: the twelve `mode-*.json` regenerated (all twelve differ from the main checkout's: Power Play's was stale since
  A139); the five tier cuts are byte-identical to the main checkout's.

## Revert path

Content only. Nothing here is read by the regular game or the ladder: the three regular cells and Cage, Glass House and Power Play replay 200 of 200 identical
hashes between the before and the after run, so no curve cell moves. Each mode reverts by restoring its file from 322e6c5, or by key:
- Arsenal: `ai.difficulty` rookie, `rules.penalties` true, `pickups` 480 / 720 with `max_on_ice` 2.
- Bruisers: `ai.difficulty` rookie, rink 80 x 40 (no `corner_radius_m`), no `contact.check_reach_m`, `rules.score_to_win` 3, no `ai.hold_radius_m`, the spawns of 322e6c5.
- Pond: `ai.difficulty` rookie. Fog of War: `rules.penalties` true, `ai.react_frames` 6. Sudden Death: `rules.penalties` true.
- Marathon: `rules.timer_frames` 28800, no `ai.hold_radius_m`.
- Outnumbered: `rules.penalties` true, rink 90 x 45 with corner 3.75, no `ai.hold_radius_m`, the spawns of 322e6c5.
- Shotgun: `rules.penalties` true, rink 64 x 32, no `skater.check_speed_min`, the spawns of 322e6c5.
- Sniper: `rules.penalties` true, `shot.lift.half_frames` 20, `shot.charge_frames_max` 60.
- `tools/soak/baseline.json`: the twelve `m-*` cells from 322e6c5's file; `tools/qa/accept/matrix-cuts/`: re-run the generator.

## Findings outside the family

1. **Five modes are three a side in a five-a-side game** (`Game/Content/Cartridges/modes/hockey.mode.{arsenal,fog-of-war,glass-house,sniper,sudden-death}.cartridge.json`,
   `game.teams`): `hockey.regular` went to five a side (R-0041) after they were cut. In them the human passes 0.4 to 0.7 times a play minute against the regular
   game's 2.8. In the family, but a re-make of five modes: next evolution 1.
2. `Game/Source/JamCore/AI/JamPlaytester.cpp:49-67`: the novice profile has no `SprintPct` (the regular class has 50 at `:84`, the good 100 at `:105`): the
   novice never sprints, so Marathon cannot be read for the class (0 against 0) and its win rate there is a floor, not a measurement. A novice that sprints on a
   tenth of its chances would make the mode's number readable.
3. `schema/game.schema.json:1185` caps `ai.red_skill` at 1, while the profile's big-rink value is 1.12 at rookie and pro (`DifficultyProfile.cpp:22`, `:60`;
   applied for a rink of `LargeRinkM` or longer at `DifficultyResolve.cpp:41-45`, then overwritten by the cartridge's key at `:52`). A mode on big ice (Marathon,
   Sudden Death) can only make its bots much weaker with that key (1 read 99 % wins), never a little. Proposed schema text: maximum 1.5, "1 = the regular rink's pro;
   the big rinks play 1.12".
4. `tools/soak/baseline.json`: the 24 take cells (`t-take-*`) are 41df3e6's; the season's takes moved with the curve (R-0057), so a full drift run flags them
   until the curve or soak lane re-takes them (`drift.mjs --only takes --update-baseline`).
5. The playtest line (`thirdjam.playtest/1`, written by the tournament commandlet) has no board-hit count and no knockdown cause (MODES-3 finding 5 stands):
   Cage's "the boards hit back" and Shotgun's blast cannot be told from other hits headless. Two fields (`board_hits`, `knockdowns_by_ability`) would do.
6. `Game/Source/JamCore/AI/JamAICore.h:206`: `BodyCheckShare` is 1.0 at pro, so `game.ai.body_check` 1 (Bruisers, Glass House) is a no-op at their level and
   the schema's description ("0 never, 1 every chance") reads as if 1 were more than the default. One sentence in the schema: "absent = the level's share (pro 1)".
7. `docs/PLAY-ME.md:30` names Shotgun, Cage and Pond; with this landing Shotgun is on 56 x 28 and Pond is a pro contest (66.5 %), which the guide's one-liners
   still describe correctly. Section 5 proposes Fog of War and Outnumbered ahead of Cage and Pond.
8. `tools/barks/barks.test.mjs:63` still fails on CRLF (`BarkData.inl`): the gate's node step reads 243 of 244 here, as before this lane.
9. No gate step reads card text (MODES-4 finding 8 stands); `tools/playtest/out/a149/cards.mjs` holds the three-line check that would do it.

## Proof

- Before and after: the table above (`tools/playtest/out/a149/base*.jsonl`, `final*.jsonl`; `final-table.md`); `modes.mjs` exit 0 after, exit 1 before (Pond 1.25x).
- Determinism: the final Bruisers, Marathon and Outnumbered replay round 5's 200 of 200 hashes; the three regular cells, Cage, Glass House and Power Play replay
  the base run's 200 of 200.
- `node schema/validate.mjs` on the twelve mode files: ok, 0 errors each.
- The full `Jam` automation filter on the final tree, the gate's own invocation (`Automation RunTests Jam; Quit`, `-nullrhi -Multiprocess -unattended`):
  **215 of 215 Success**, exit code 0 (`Game/logs/a149-jam-1.log`, 02:17), among them `Jam.Menu.FitsTheScreen`, `Jam.Menu.DirectorsChoices` (twelve cards),
  `Jam.Core.CartridgeRoundTrip`, `Jam.Core.CartridgeParity`, `Jam.Core.ContactModes`, `Jam.Core.Abilities.*`, `Jam.Core.Determinism`, `Jam.Net.GuestSeat`,
  `Jam.Net.HashCoverage`. After the three "3 v 3" card facts (text only), the baseline and the matrix cuts: the full filter again on the tree as it lands,
  **215 of 215 Success**, exit code 0 (`Game/logs/a149-jam-final.log`, 02:27).
- The hash: nothing under `modes/` reaches `hockey.regular`; `Jam.Core.Determinism` is green inside the filter and the regular cells of the playtest runs are
  hash-identical before and after.
- Node tests on the gate's globs: 243 of 244 (finding 8).
- Not run: the acceptance run (no verb of the player changed; the modes' packaged pass is track S2's matrix on the regenerated cuts), the three curve cells
  (nothing the ladder reads changed), a Shipping compile (no source changed).

## Next evolutions for the track, ranked

1. **Five a side in the five that are three a side** (Arsenal, Fog of War, Glass House, Sniper, Sudden Death), each kept inside 15 to 85 % for the regular
   class and 10 % for the novice with its number still a third apart: the human's passes there 2 a play minute or more (0.4 to 0.7 today; the regular game 2.8).
2. **Marathon's puck**: a 30 s stretch without the puck in under half the matches (84 % today; the regular game 6 %) and touches at 9 a play minute or more
   (7.0), with the sprint still 1.33x, the regular class under 85 % and the good class under 95 % (100 today). Levers not yet tried: `puck.count` 2, a mate
   that gives the puck, three a side with the hold radius.
3. **Arsenal's tricks and Power Play's third**: three abilities used a match (0.94 today; 79 of 200 matches see none: one ability held from the faceoff
   beside the pickups), and Power Play at 1.6x or more (1.45x; the human down 12.1 times a match there: the three killers' `ai.body_check`).
