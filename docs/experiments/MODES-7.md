# Director's choices, pass 7: the twelve on main ca493723, Sniper's five against four, the good class (lane A173, track M1 evolution 5, device A, 2026-11-14)

Owner: lane A173. Earlier passes: `MODES.md` (A67) to `MODES-6.md` (A156).
Build: this worktree at main ca493723 (the poke, the brace, the ladder's last trim, device B's night; the modes of evolution 4 as landed, Fog of War at
three a side), editor Development. The modes were last measured on 3f5cbda.
Files changed: four cartridges under `Game/Content/Cartridges/modes/` (Sniper, Bruisers, Fog of War, Cage), `modes/MODES.md`, `tools/soak/baseline.json` (the
twelve mode cells), `tools/qa/accept/matrix-cuts/mode-*.json` (four, generated). No sim, brain, profile or tool code; no state field; no test added or changed.
**Nothing here lands before the operator has played unless the Director says so**: the package he plays (H28) carries the files of section 1's "as landed" column.

## How it was measured

As MODES-5 and MODES-6: `node tools/playtest/modes.mjs --seeds 1-200` (regular class), `--seeds 201-300 --player novice` and `--player good`; each mode at its
own level beside `hockey.regular` at the same level, same seeds, same binary; three `UnrealEditor-Cmd` at a time, `-Multiprocess`. Bars: a third apart in the
mode's own number, every match ends, 15 to 85 % for the regular class, 10 % or more for the novice; this evolution's own: Sniper at 1.33x with the novice at 14 %
and the human's passes at 2 a play minute; the good class under 85 % in Cage, Fog of War and Power Play. Rounds r1 to r3 at 100 seeds a class (39 variants), the
confirmation r5 and r6 at 200 a class (regular 1-200, novice and good 201-400), then the table. Raw lines, variants and specs: `tools/playtest/out/a173/`
(`base*` the landed files on this main, `final*` after, `before/` the ca493723 files; does not travel).

## 1. The twelve as landed, on this main (the operator's package), against the commit they were measured on

The regular game moved under them: the regular class wins **59.5 % at pro** (49.5 on 3f5cbda; 52 with sprint, 60.5 before) and 17.5 at all_star (13); the
novice 14 / 0 (12 / 4); the good player 81 / 28 (71 / 27). Regular class 200 seeds, novice and good 100; 3f5cbda in brackets.

| mode | number: mode / regular game = ratio (3f5cbda) | win %, regular class | win %, novice | win %, good | moved since 3f5cbda |
|---|---|---|---|---|---|
| Arsenal | 4.89 tricks a match (4.72) | 61.5 (58.5) | 16 (14) | 87 (80) | the good class 80 -> 87 |
| Bruisers | 5.72 / 4.31 = **1.327** (1.43) | 56.5 (56) | 15 (23) | 60 (63) | **under a third**; the novice 23 -> 15 |
| Cage | 6.66 / 4.53 = 1.47 (1.96) | 72.5 (72.5) | 33 (25) | 88 (92) | its number 1.96x -> 1.47x |
| Fog of War | 2.00 / 0.79 = 2.53 (3.11) | 78 (79) | 49 (48) | 95 (92) | its number 3.11x -> 2.53x |
| Glass House | 12.5 box s (11.5) | 46 (43.5) | 12 (14) | 62 (64) | no |
| Marathon | 427.6 / 263.0 = 1.63 (1.60) | 76 (73.5) | 15 (16) | 85 (83) | no (30 s without the puck in 14 % of matches) |
| Outnumbered | 6.44 / 3.76 = 1.71 (1.68) | 58.5 (57) | 23 (23) | 86 (82) | no |
| Pond | 7.21 / 4.24 = 1.70 (1.75) | 70 (69.5) | 20 (19) | 89 (80) | the good class 80 -> 89 |
| Power Play | 6.97 / 4.24 = 1.64 (1.62) | 64.5 (59) | 20 (19) | 79 (90) | the good class 90 -> 79 |
| Shotgun | 7.88 / 5.11 = 1.54 (1.67) | 56 (46.5) | 14 (14) | 84 (76) | the regular class 46.5 -> 56 |
| Sniper | 0.61 / 0.29 = 2.08 (2.08) | 53.5 (44.5) | 15 (10) | 67 (65) | the regular class 44.5 -> 53.5 |
| Sudden Death | 49 s / 182 s = 0.27 (0.27) | 40.5 (34.5) | 15 (18) | 43 (41) | no |

**What the night's rules moved out of a bar: one mode, Bruisers** (1.327x: the regular game itself hits 4.31 times a play minute now, 3.71 before). Nothing left
15 to 85 % for the regular class (40.5 to 78) and no novice is under 10 % (12 to 49). The five modes the operator's sheet names read 56 to 78 % for a regular
player: its "45 to 80 %" holds. A good player wins Fog of War 95 %.

## 2. After this evolution (the four changed files; the other eight rows are section 1's)

| mode | change | number, ratio (regular; novice; good) | win %, regular / novice / good (as landed on this main) | human passes a play min |
|---|---|---|---|---|
| **Sniper** | blue five (player, chaser, support, two backs) against orange four (chaser, two supports, back); `ai.hold_radius_m` 1 | 0.47 / 0.29 = **1.60**; 2.07; 1.49 | 45.5 / 24 / 60 (53.5 / 15 / 67) | **2.20** (0.57) |
| **Bruisers** | rink 56 x 28 (was 70 x 35) | 6.05 / 4.31 = **1.40**; 1.64; 1.13 | 52 / 14 / 58 (56.5 / 15 / 60) | 2.46 |
| **Fog of War** | `ai.speed_scale` 1.1 (every bot skates a tenth faster, still 4 frames late) | 1.98 / 0.79 = **2.51**; 3.36; 3.06 | 61 / 34 / **81** (78 / 49 / 95) | 0.79 |
| **Cage** | `ai.body_check` 0.85 (was 0.5), `contact.check_reach_m` 2 | 6.28 / 4.53 = **1.39**; 1.06; 1.55 | 62.5 / 21 / **75** (72.5 / 33 / 88) | 1.56 |

`modes.mjs --seeds 1-200` exits 0 on the final tree: twelve of twelve a third apart (1.39x at the thinnest: Cage), 0 unfinished of 2,400, the regular class inside
40.5 to 76, every novice at 12 or more. Of 200 novice matches (seeds 201-400): Sniper 22 %, Bruisers 16.5 %, Fog of War 27.5 %. Of 200 good-class matches: Fog of
War 80.5 %, Cage 76 %, Sniper 65.5 %. Power Play needed nothing: the good class is at 79 % as landed on this main.

**Not green, and said so:** Arsenal is not "every match a trick" (6 of 200 without one; section 4). Cage's third is 1.39x (it was 1.47x as landed: the good
class's lever costs it). The good class is over 85 % in four modes the brief did not name: Pond 89, Arsenal 87, Outnumbered 86, Marathon 85 (the regular game
itself gives the good player 81). The novice's own number in Cage is 1.06x (it lands few checks anywhere). Glass House's novice is 12 of 100.

## 3. Sniper: the lane at five is five against four

| variant (100 seeds; regular class / novice; the regular game 63 / 14) | goals per shot, ratio (regular; novice) | win | human passes a play min |
|---|---|---|---|
| three a side, as landed (200 seeds) | 2.08; 3.03 | 53.5 / 15 | 0.57 |
| four a side with a back / + `ai.yield_margin_m` 0 | 1.52; 2.52 / 1.83; 2.39 | 41 / 19; 57 / 19 | 1.13 / 1.06 |
| four a side with two supports / + Pond's pass keys | 1.67 / 1.66 | 50 / 14; 50 / 31 | 0.99 / 1.05 (he receives 2.2) |
| five a side with two supports / with a 6 m net | 1.17 / 1.24: fail | 57 / 6; 42 / 12 | 2.79 / 2.92 |
| five against four with two supports | 1.26: fails | 43 / 10 | 2.31 |
| **five against four** | 1.56; 2.12 | 48 / 11 | 2.20 |
| **five against four + `ai.hold_radius_m` 1** | 1.49; 2.07 | 47 / **24** | 2.12 |
| five against four + `ai.yield_margin_m` 0 | 1.50; 2.46 | 46 / 17 | 2.23 |

The wall at four a side is the passes (1.0 to 1.1 whatever the mates are): the playtester passes to a mate five metres ahead, and the weak-side mate ahead
exists only in a five's shape. The wall at five a side is the lane (1.17 to 1.41x across MODES-6 and here, a 6 m net included). **Five for the human's side and
four against keeps both**: the shape passes (2.2 a play minute) and one defender fewer leaves the lane (1.60x at 200 seeds). It is not an easier mode: the
regular class wins 45.5 % (53.5 at three a side). The hold radius of 1 m is the novice's: 22 % of 200 (11 of 100 without). The card says "5 v 4, shot 26 m/s".

## 4. Arsenal: every match a trick needs one test sentence

On this main Arsenal reads 4.89 tricks a match with none in 6 of 200 matches (13 on 3f5cbda). **A boost on the faceoff dot does not close it**: a second
`boost_only` zone (a 2 m circle at centre) with its own speed boost every 120 frames reads 5.76 tricks and none in 6 of 200; with two boosts on the dot 5.82
and 5 of 200 (r 1 m: 5 of 100; r 4 m: 6 of 100; a lane 8 m wide instead of 14: 10 of 100). Not taken: no change to Arsenal in this evolution.

**What closes it, measured and not landed** (`tools/playtest/out/a173/r7/`, `modeab.mjs`, 200 seeds, regular class):
- One of the four entries held from the drop instead of found: `abilities[3]` = `{ "kind": "slow_puck", "trigger": "mode", "charges": 1 }` beside three
  `pickup` entries: **5.17 tricks a match, no trick in 0 of 200**, 61.5 % (as landed 61.5). It passes the schema and the loader.
- A fifth entry does not load: the schema caps `abilities` at four ("the list has 5 items; the limit is 4") and the commandlet exits 2 on such a file. (MODES-6
  finding 2 said the schema allowed a `mode` entry beside the four: it allows one among the four, not a fifth. Corrected here.)
- The one thing in the way is `Jam.Core.Abilities` (`JamAbilityTests.cpp:198-199`), which asserts that all four are `pickup`. Proposed sentence, for the
  test's owner (no edit made): "arsenal mode: four abilities, at most one held from the drop (`mode`), the others granted by a boost (`pickup`)", i.e.
  `TestEqual(..., Abilities.Num(), 4)`; count the entries whose trigger is `mode` and `TestTrue(..., Modes <= 1)`; every other entry `pickup`.
  `JamNetGuestTests.cpp:123` reads the same file for a guest's pickup ability ("four kinds, all trigger pickup"): its comment changes with it, and the guest
  seat should be re-read with a `mode` entry in the pool.

## 5. The good class under 85 % in Cage, Fog of War and Power Play

On this main as landed: Cage 88, Fog of War 95, Power Play 79 (the regular game 81).

- **Power Play: nothing to do** (79 %). Tried and not taken: the killers' chase sprint 0.75 (88), every bot at 1.05 / 1.1 speed (86 / 83, the novice 9 at 1.1).
- **Fog of War: bots that skate a tenth faster and are as late as before** (`ai.speed_scale` 1.1): the good class 80.5 % of 200 (81 of 100), the regular class
  61 (78), the novice 34 (49), and the mode's number untouched: 1.98 breakaways a play minute (2.00), 2.51x. The other way, halving the lateness
  (`ai.react_frames` 2), reads the same good class (80 % of 200) with the regular class at 74 and the breakaways at 2.17x: it spends the mode's own key. Also
  measured: react 3 (86), react 1 (81; 2.34x), react 2 with speed 1.05 (76; 2.45x). The pitch says "Late bots" now: they are no longer slow.
- **Cage: the bots check on 85 % of their chances (was 50) and the human's check finds its man** (`contact.check_reach_m` 2): the good class 76 % of 200 (75 of
  100), the regular class 62.5 (72.5), the novice 21 (33), 1.39x. `ai.body_check` is the good class's lever (0.75: 82; 1: 61) and it thins the third (1.37x,
  1.35x) because the human is down more and lands less; the reach gives the checks back (alone: 1.57x, the good class 89). With the reach: 0.75 reads 85 % of
  200 (1.45x), 0.85 reads 76 (1.39x), 0.9 reads 75.5 (1.36x), 1 reads 69 (1.36x). Shipped: 0.85. Not levers: bots at 1.05 speed (84; 1.39x), knockdown 90
  (79; 1.30x: fails).
- **Bruisers** (not in the brief's list; under a third on this main): the ice is the only lever. 56 x 28: 1.40x at 200 seeds (52 % / 16.5 % of 200 / 58.5);
  60 x 30: 1.41x with the novice at 9; 60 x 30 with a hold of 2: 1.30x; 64 x 32, reach 3, grace 0, `check_speed_min` 1: 1.32 to 1.35x. The good class's own
  number is 1.13x there (it hits 5.6 times a play minute in the regular game): a finding, not a bar.

## 6. The drill's mate (the Director's item 3)

The acceptance drill takes as its mate the first teammate **whose role is not `back`** and as its opponent the first opposing skater that is not a back
(`Game/Source/JamGame/Private/HokejAcceptDrill.cpp:103-104`); with none, the index stays -1 and `:341` reads `Sk[-1]`. The old Marathon (the human and a
back) had a mate and still crashed: its mate was a back. **On this tree, before and after this evolution, all twelve modes give the human at least one mate
that is a chaser or a support, and every opposing side has a chaser:**

| mode | the human's mates | opposing non-backs |
|---|---|---|
| Arsenal, Glass House, Shotgun, Sudden Death, **Sniper (after)** | chaser, support, two backs | chaser, two supports |
| Bruisers, Pond | chaser, two supports, back | chaser, two supports (Bruisers: and a second chaser) |
| Cage | chaser, support, back | chaser, two supports |
| Power Play | two supports, two backs | chaser, support |
| Marathon | chaser, support | chaser, support |
| Fog of War, Outnumbered, Sniper (as landed) | chaser, back | chaser, support (Outnumbered: two chasers, two supports) |

## 7. The cards, the soak's baseline, the acceptance matrix

| mode | pitch | facts | changed |
|---|---|---|---|
| Sniper | Big net, low guard. Lift it, bury it. | net 1.22 m high / pads at 0.15 m / 5 v 4, shot 26 m/s | "3 v 3" -> "5 v 4" |
| Bruisers | Five a side, heavy bodies. Hit everything. | 5 v 5 on 56 x 28 m / 130 kg, first to 2 / checks find the man | the rink |
| Fog of War | Late bots in the fog. Slip by, then hit. | bots 0.07 s late / hits 5x harder / 3 v 3, night, fog | "Slow" -> "Late" |

Cage's card is unchanged and true. `Jam.Menu.FitsTheScreen` green: widest name 210, widest pitch 432 (of 458), widest fact 196 (of 262). Every card line
through `tools/barks/filter.mjs` `checkLine`: clean.
- `node tools/soak/drift.mjs --only modes --update-baseline --max 3 --chunk 100`: 1,200 matches in 128 s, 0 faults, the twelve `m-*` cells written on this
  main (the eight untouched modes moved with the build too: Shotgun 49 -> 60 % of its 100, Sudden Death 33 -> 45); the 24 take cells are byte-identical to
  before (still 41df3e6's). A drift run over the same output: 12 of 12 bit-identical, exit 0.
- `node tools/qa/accept/make-matrix-cuts.mjs`: four mode cuts differ from the main checkout's (bruisers, cage, fog-of-war, sniper); the rest byte-identical.

## Revert path

Content only. The three regular cells and the eight untouched modes replay 200 of 200 identical hashes between the before and the after run on this build.
Each mode reverts by restoring its file from ca493723 (`tools/playtest/out/a173/before/`), or by key:
- Sniper: three a side (player, chaser, back v chaser, support, back; spawns +-7.5 / +-22.5), no `ai.hold_radius_m`; fact "3 v 3, shot 26 m/s".
- Bruisers: rink 70 x 35 with corner 3.5 and the spawns of ca493723; fact "5 v 5 on 70 x 35 m".
- Fog of War: no `ai.speed_scale`; pitch "Slow bots in the fog. Slip by, then hit.".
- Cage: `ai.body_check` 0.5, no `contact.check_reach_m`.
- `tools/soak/baseline.json`: note that the twelve `m-*` cells were re-taken on ca493723: the cells of 3f5cbda no longer replay on this build for any mode.

## Findings outside the family

1. `Game/Source/JamGame/Private/HokejAcceptDrill.cpp:103-104`, `:341`: the drill needs a non-back mate and a non-back opponent and indexes them unguarded.
   Safe on the twelve modes; a cut with a lone human, or with only backs for mates, crashes the Development game's drill. A guard ("no mate: skip step 5 and 6")
   is the acceptance lane's.
2. `Game/Source/JamCore/Private/Tests/JamAbilityTests.cpp:198-199` (and the comment at `Game/Source/JamNet/Private/Tests/JamNetGuestTests.cpp:123`): section 4's
   sentence. With it Arsenal reads no trick in 0 of 200 matches.
3. `schema/game.schema.json` `abilities`: the description says "At most one mode entry" and the list is capped at four; the two together are easy to misread
   as four pickups plus one (this lane did, in MODES-6). One clause: "four entries in all".
4. `Game/Source/JamCore/AI/JamPlaytester.cpp:352-390`: the playtester passes only to a mate five metres ahead under pressure or with its lane blocked: under
   five a side no mate stands there, so the human's passes read 1.0 to 1.1 a play minute in every four-a-side variant tried (MODES-4 finding 4 stands). The
   passes bar measures the shape, not the mode, until the regular and good classes pass to a better-placed mate.
5. The good class: `hockey.regular` at pro gives it 81 % on this main (71 on 3f5cbda): the ladder's last trim moved pro for every class (the regular class
   49.5 -> 59.5 on these seeds). Seven modes now give the good player 79 to 89 %. A curve-lane line, no edit.
6. `tools/soak/baseline.json`: the 24 take cells are still 41df3e6's (MODES-5 finding 4, MODES-6 finding 7 stand).
7. `tools/barks/barks.test.mjs:63` still fails on CRLF: the gate's node step reads 252 of 253 here, as before this lane.

## Proof

- The table of section 1 (`base*.jsonl`) and of section 2 (`final*.jsonl`, `final-moved.md`); `modes.mjs` exit 1 on the landed files (Bruisers 1.327x), exit 0 after.
- Determinism: the four changed modes replay the confirmation rounds' 200 of 200 hashes in the final run; the three regular cells and the eight untouched
  modes replay the base run's 200 of 200.
- `node schema/validate.mjs` on the twelve mode files: ok, 0 errors each.
- The full `Jam` automation filter on the final mode files, the gate's own invocation (`Automation RunTests Jam; Quit`, `-nullrhi -Multiprocess -unattended`):
  **265 of 265 Success**, exit code 0 (`Game/logs/a173-jam-1.log`, 06:47), among them `Jam.Menu.FitsTheScreen`, `Jam.Menu.DirectorsChoices`,
  `Jam.Core.CartridgeRoundTrip`, `Jam.Core.CartridgeParity`, `Jam.Core.ContactModes` (plays Bruisers, Fog of War and Cage), `Jam.Core.Abilities.*`. Nothing the
  engine reads changed after that run.
- Node tests on the gate's globs: 252 of 253 (finding 7).
- Not run: the acceptance run on a package (cartridges; R-0056's line applies to a landing, and nothing here lands before the Director says), the Development
  game's drill on the four changed modes (section 6 is read from the files), the curve cells (nothing the ladder reads changed), a Shipping compile (no source).

## Next evolutions for the track, ranked

1. **Arsenal: every match a trick**, once the test's sentence is agreed: `abilities[3]` held from the drop with one charge: no trick in 0 of 200 matches
   (6 today), 5.2 tricks a match, the guest's seat re-read with a `mode` entry in the pool.
2. **The good class across the modes**: Pond 89, Arsenal 87, Outnumbered 86, Marathon 85: each under 85 % with its third, its regular class inside 15 to 85
   and its novice at 10 or more. Levers seen: every bot at 1.1 speed (Fog of War: -14 for the good class, -15 for the novice), `ai.body_check` (Cage).
3. **Cage's third and Bruisers' third with margin**: both at 1.39 to 1.40x on this main; each at 1.5x or more for the regular class with the good class under
   85 % (Cage) and the novice at 14 % or more (Bruisers). Untried: Cage on 56 x 26, two more pillars, `contact.board_bonus` read as its number (needs the
   playtest line's board-hit count); Bruisers with `skater.mass` 150 and a check lunge (`contact.lunge_mps`).
