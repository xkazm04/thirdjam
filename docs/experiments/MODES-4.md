# Director's choices, pass 4: the passing modes pass, the grace keys, a Marathon of legs (lane A139, track M1 evolution 2, device A, 2026-11-13)

Owner: lane A139. Earlier passes: `MODES.md` (A67), `MODES-2.md` (A81), `MODES-3.md` (A122, the track's first evolution under the vision).
Build: this worktree at main 41df3e6 (the kept puck, the release grace 120, the take grace 30, the harder stop, the earned first step), editor Development.
Files changed: eight cartridges under `Game/Content/Cartridges/modes/` (three re-made, four with one grace key, one card word), `modes/MODES.md`,
`tools/playtest/modes.mjs` (two numbers per play minute), one line in `tools/soak/drift.test.mjs` (finding 1). No sim, brain or profile code.

**The ladder is still off on this commit** (the regular class wins 100 / 76 / 27 at rookie / pro / all_star against targets of 85 / 48 / 26; the brain's second
evolution and the re-set ladder are not in this tree). Every win rate below is read against the regular game on the same row.

## How it was measured

`node tools/playtest/modes.mjs --seeds 1-200`: each mode validated, then 200 matches by the regular playtester at the mode's level next to `hockey.regular` at the
same level, same seeds, same binary; one `UnrealEditor-Cmd` a cell, `-Multiprocess`. A mode passes when its character number is a third or more from regular's and
every match ends. Variant rounds at 100 seeds (`--dir`, `--baseline`), the final table and the level-name round at 200. Two numbers changed in the driver:
**Marathon's and Power Play's are per play minute** (the brief's items 1 and 3: a per-match count read the clock). Raw lines, the 83 variant cartridges and
their specs: `tools/playtest/out/a139/` (`base.*` before, `after.*` after, `r1..r6.*`, `after-novice.*`, `after-good.*`; the folder does not travel).

## One table for the Director: all twelve on this build (200 seeds, regular class)

Regular on this build: rookie 100 % in 101 s, pro 76 % in 246 s (with sprint 77 % in 236 s), all_star 27 % in 293 s; the human is down 2.9 / 9.1 / 10.2 times a match.

| mode | level | character number | mode / regular = ratio (before) | win % (regular) | match s | human down a match (a play min; regular) | fun on paper? what would make it so |
|---|---|---|---|---|---|---|---|
| Arsenal | rookie | human abilities used / match | 0.65 / 0 | 93.5 (100) | 113 | 3.0 (1.79; 1.93) | Not yet: 108 of 200 matches see no trick at all. Boosts twice as often or first to 5: three tricks a match is the bar. |
| Bruisers | rookie | hits (landed + taken) / play min | 4.68 / 3.18 = **1.47** | 99.5 (100) | 126 | 6.0 (3.09; 1.93) | A brawl, yes (down 1.6x as often); a contest, no. The ladder's rookie makes it one; at pro it loses its third (1.28x). |
| **Cage** | pro | checks landed / match | 8.0 / 4.04 = **1.98** (1.97) | 61 (76) | 206 | 7.4 (2.36; 2.38) | Yes: the best-proven of the twelve. More: the board hit named on the screen. |
| Fog of War | pro | human breakaways / play min | 1.84 / 0.44 = **4.16** | 87.5 (76) | 120 | 5.1 (2.89; 2.38) | Yes for the breakaways; too easy on this build. `ai.react_frames` 6 -> 4. |
| Glass House | pro | human box seconds / match | 10.06 / 5.3 = **1.90** | 43.5 (76) | 156 | 4.9 (2.12; 2.38) | For the player who likes rules: the one whistle mode. The box as a countdown, the foul named. |
| **Marathon** | pro, sprint | human sprint frames / play min | 347 / 223 = **1.56** (1.08, failed) | 87 (77) | 525 | 12.2 (1.52; 2.41) | Half: the legs are there, the puck is not (5.9 touches a minute against 10.9; 30 s without it in 96 % of matches). A second puck. |
| **Outnumbered** | pro | shots against / play min | 5.45 / 3.28 = **1.66** (1.61) | 65.5 (76) | 150 | 3.0 (1.32; 2.38) | Yes: the human holds the puck 3 s or more 2.1x as often as in the regular game, under 1.66x the shots. |
| **Pond** | rookie | passes + received / play min | 5.40 / 3.25 = **1.66** (0.31) | 100 (100) | 209 | 0 | As play, yes (16 touches a minute, a one-timer every other match); as a contest, no (7.1 : 0.7). The ladder's rookie. |
| **Power Play** | pro | passes + received / play min | 4.10 / 2.55 = **1.61** (0.44) | 69.5 (76) | 270 | 10.5 (2.55; 2.38) | Yes: it passes, it one-times (0.57 a match against 0.10). The three killers hit a lot: a lower `ai.body_check`. |
| Shotgun | pro | knockdowns (all skaters) / play min | 6.14 / 4.16 = **1.48** | 65 (76) | 148 | 5.3 (2.39; 2.38) | Yes: loud, short. The blast's width is still a constant (MODES-3 finding 2). |
| Sniper | pro | human goals per human shot | 0.51 / 0.19 = **2.63** | 19.5 (76) | 95 | 3.8 (2.82; 2.38) | Too hard: a quarter of the regular game's wins. `skater.goalie.block_height_m` 0.15 -> 0.25 (the bots' lifted shots). |
| **Sudden Death** | all_star | match seconds | 37.2 / 292.9 = **0.13** (0.15) | 21.5 (27) | 37 | 0.6 (1.05; 2.21) | Yes, as a dare: 37 s, one in five. |

`modes.mjs` exits 0: twelve of twelve a third apart, 0 unfinished of 2,400, the longest match 34,800 frames (Marathon). Bold = changed in this pass.

**Level names once the ladder's rookie is near 85 %** (the three that are too easy today). Each was measured at pro as well (200 seeds, `r6.*`):

| mode | at rookie: win (regular game) | at pro: win (regular game), number | the name it should carry |
|---|---|---|---|
| Pond | 100 (100) | 75.5 (76), 5.36 / 2.55 = 2.11x | **rookie**: at both levels it wins what the regular game wins; a re-set rookie makes it a contest with no edit |
| Arsenal | 93.5 (100) | 47 (76), 0.71 tricks a match | **rookie**: it is already harder than the regular game at its level; pro would fall further once pro is at its target |
| Bruisers | 99.5 (100) | 69.5 (76), 4.39 / 3.43 = 1.28x (**loses its third**); with `ai.body_check` 0.5: 1.06x | **rookie**: at pro the regular game hits nearly as much; it needs a re-make before it can move |

So none of the three needs an edit when the ladder lands, if they keep following the regular game at their level as they do at the two levels measured here.
Fog of War (87.5), Marathon (87) and Sniper (19.5) are the ones to turn then (next evolutions, 1).

## 1. Pond and Power Play pass again

**Why they did not.** Three facts in the code, each confirmed by a variant:
- The playtester passes only under pressure (an opponent within 4 m) or with its lane to the net blocked, to a mate 5 m or more **ahead** with nobody within 3 m
  (`JamPlaytester.cpp:352-390`). The mate that stands ahead of the puck on the weak side is the **second support of a five** (`JamAICore.cpp:703-735`); under five a
  side nobody goes there. Pond at 3 v 3 and 4 v 4: 0.2 human passes a play minute; at 5 v 5: 1.5 to 2.1.
- A reception needs the puck at 6 m/s or more. Pond's "soft pass" (`pass_speed` 10) falls under that after 8 m of `puck.drag` 0.5; Power Play's 16 is over the
  14 m/s a body takes inside 4 m. Regular's 14 is the pass that arrives: Pond 4.22 with 12, 4.95 with 14.
- A man advantage takes away the playtester's reason to pass: 5 v 3 with everything else as it was reads 2.49, under the regular game's 2.69.

**Pond** (1.02 -> 5.40 a play minute; regular 3.25; **1.66x**). "Pond hockey. Pass, skate, nobody gets hit." Five a side on the same 60 x 30 (blue: player, chaser, two
supports, one back), the regular pass speed with a 20-frame lead, reception still at 3 m, and mates that do not yield the puck (`ai.yield_margin_m` 0: they take it and
give it: +0.6 receptions a minute). The human passes 1.92 times a play minute (0.19 before; regular 2.06) and receives 3.48 (0.82; regular 1.19), touches the puck 16
times a minute (regular 12.6), one-times 0.45 a match. Rounds at 100 seeds: 4 v 4 1.17; 5 v 5 3.29; + pass 12 / lead 20: 4.27; + two supports: 4.22 (12), 4.95 (14);
+ yield 0: 5.44. Not taken: `puck.drag` 0.3 (5.68: one more key for 4 %), a 70 x 35 rink (4.21), `carry_speed_mult` 0.8 (5.62).

**Power Play** (1.13 -> 4.10; regular 2.55; **1.61x**). "Five on three. Move it, fire it, repeat." Five on three (blue: player, two supports, two backs) on 70 x 35 (the
zone, not the sheet), the carrier at 80 % of a free skater's speed (`skater.carry_speed_mult` 0.8, regular 0.92: the puck moves faster than legs, the vision's own
lever), the regular pass speed with a 20-frame lead, reception at 3 m, mates that do not yield, no whistle. The human passes 1.94 (0.18) and receives 2.16 (0.95),
one-times 0.57 a match (0.05; regular 0.10), touches the puck 12.3 times a minute (8.2); matches with 30 s without the puck fall from 116 of 200 to 22.
Win 69.5 % (regular 76), 3.7 : 2.7. Rounds at 100 seeds: 5 v 4 2.23 to 2.57 (win 43 to 59), 5 v 3 2.49 to 2.72, all_star as a name 2.62; the slow carrier alone
2.89 (0.75) and 3.03 (0.65); 70 x 35 alone 3.03; pass keys + 70 x 35: 3.68; all three: 4.09 (win 47, the human 18 s a match in the box of its own power play);
the whistle off and carry 0.75: 4.31 (win 63); **carry 0.8: 4.14, win 74**: taken. Without the slow carrier it reads 3.62 (1.35x): too thin to ship.
What it costs: the human is down 10.5 times a match (2.55 a play minute against the regular game's 2.38): three killers on tight ice hit.

Other classes (seeds 201-300, 100 each): the novice passes 2.47x (Pond) and 2.18x (Power Play) the regular game and wins 96 / 39 % (regular game 100 / 33); the
good player 1.22x (Pond: it passes 4.27 a minute in the regular game already, so the mode reads under a third for that class) and 1.69x (Power Play), wins 100 / 84 (100 / 96).

## 2. The two grace keys

| mode | key | values measured (100 seeds) | what moved | shipped |
|---|---|---|---|---|
| Cage | `contact.release_grace_frames` | 0 / 45 / 60 / 120 | checks landed a play min 2.65 / 2.61 / 2.60 / 2.56; knockdowns (all) 6.11 / 6.07 / 6.07 / 6.00; human down a match 7.47 / 7.39 / 7.43 / 7.38 | **45** (1.98x at 200 seeds; 81 of 200 matches differ) |
| Bruisers | `contact.release_grace_frames` | 0 / 45 / 60 / 120 | hits a play min 4.66 / 4.63 / 4.58 / 4.62; human down a match 6.12 / 6.23 / 6.17 / 6.15 | **60** (1.47x; 39 of 200 matches differ) |
| Outnumbered | `stick.take_grace_frames` | 0 / 30 / 60 / 90 / 120 | carries of 3 s a play min 1.36 / 1.29 / 1.47 / 1.53 / 1.56; carry share 18.8 / 18.3 / 18.8 / 19.5 / 19.4 %; win 55 / 64 / 62 / 69 / 65; shots against 5.49 / 5.14 / 5.82 / 5.39 / 5.58 | **90** (1.66x; carries of 3 s 1.49 against 0.71 = 2.1x; win 57.5 -> 65.5) |
| Sudden Death | `stick.take_grace_frames` | 0 / 30 | match 37.2 / 42.8 s; win 21.5 / 20.5; human down a match 0.61 / 1.11 (200 seeds) | **0** (0.13x) |
| Shotgun | `contact.release_grace_frames` | 0 / 45 / 120 | knockdowns (all) a play min 6.10 / 6.09 / 6.08; blasts a play min 2.38 / 2.37 / 2.38 | **unchanged (120)** |

- **The release grace hardly bites in the hitting modes**: between none and two seconds Cage's number moves 3.5 % and Bruisers' 1 %. The bodies there hit somebody
  else in the meantime. The keys are set as the modes' own statement ("up quick, hit again": 0.75 s in the Cage beside its 75-frame knockdown, 1 s in Bruisers) and cost
  nothing measurable; they are not what makes these modes.
- **Shotgun and the grace together: still Shotgun.** R-0051 has the blast respect the grace; with a blast every 20 s it almost never meets a skater inside its two
  seconds: the number is the same at 0, 45 and 120. Nothing to turn down.
- **The take grace is Outnumbered's lever**, as MODES-3 guessed: it helps the side that has just won the puck. At 90 the human's long carries rise 19 % over the
  default at 100 seeds and the win rate 8 points at 200, with the pressure intact. 120 gives no more.
- **Sudden Death with none**: the match is 13 % shorter and the win rate does not move: one mistake ends it, for both sides.

## 3. Marathon: a game of legs

"Two on two on endless ice. Bring your legs." **347 sprint frames a play minute against 223 (1.56x; before 241 = 1.08x)**, win 87 % (regular with sprint 77),
8.0 : 4.0 in 8:00; 132 of the 347 are with the puck (66 before).
- **Two a side is the lever.** The playtester sprints with no opponent within 8 m (carrying) or chasing a puck 10 m away that no mate carries; at three a side on
  140 x 70 that happened no more often than in the regular five a side. 2 v 2: 322 (1.41x). The bar at 4 s on, 8 s to refill (was 2 s / 10 s): 349 (1.53x).
- **What did not move it** (100 seeds): mates that yield (240), a carrier's sprint worth 25 % (271), speed 8 with a 60 % sprint (251), 160 x 80 ice (335, no gain
  over 140 x 70), the bots sprinting on half their chases (303).
- **The mate is a back, on purpose.** With a chaser for a mate and nobody home the number is higher (386 = 1.73x) and the regular class wins 56.5 %, inside the note
  band; but the novice then wins **1 %** (24 % with the back; the regular game gives it 33) and the good player 100. Shipped: the back. The regular class's 87 % is
  2 points over the 15 to 85 note band on a pro that wins 76 and is about to be re-set downwards.
- **What it is not yet**: a game with the puck in it. The human touches the puck 5.9 times a minute (regular 10.9) and 96 % of matches hold a 30 s stretch without
  it. The novice class never sprints, so the mode's number cannot be read for it (0 against 0). Next evolutions, 2.

## The cards (`game.meta`)

| mode | pitch | facts | changed |
|---|---|---|---|
| Pond | Pond hockey. Pass, skate, nobody gets hit. | 60 x 30 m, 3:00 / no hits, no box / catch passes at 3 m | facts ("soft passes" was the fault) |
| Power Play | Five on three. Move it, fire it, repeat. | 5 v 3 for you / carry at 80 % speed / one-timers, 4:00 | pitch, facts |
| Marathon | Two on two on endless ice. Bring your legs. | 140 x 70 m, 8:00 / 2 v 2, no whistle / sprint +40 % for 4 s | pitch, facts |
| Outnumbered | Three on five. Keep the puck, take the heat. | 3 v 5 against you / F: shield, 3 s / won puck safe 1.5 s | third fact |
| Sudden Death | One goal ends it. The meanest bots alive. | first goal wins / 120 x 60 m rink / no grace on a steal | third fact |
| Glass House | Every hit is a risk. Fouls cost 10 s. | (unchanged) | "gamble" is a word the voice filter bans (`tools/barks/filter.mjs`) |

Pitches 31 to 45 characters across the twelve (cap 48), facts 11 to 20 (cap 20); every fact checked against its cartridge; hockey words only, no names, no brand. Every line of
all twelve cards was run through the filter's `checkLine`: no banned word left (it still flags the numbers and the key F, which a card may carry).

## Revert path

Content only; nothing here is read by the regular game or the ladder (the four regular cells and the five untouched modes replay 200 of 200 identical hashes
between the before and the after run, so no curve cell moves). Each mode reverts by restoring its file from 41df3e6, or by key:
- Pond: three a side (player, chaser, back v chaser, support, back; spawns +-7.5 / +-6 / +-17), `skater.pass_speed` 10, `pass_lead_frames` 30, no `ai.yield_margin_m`.
- Power Play: 4 v 3 on the regular 90 x 45, `pass_speed` 16, no `pass_lead_frames`, no `carry_speed_mult`, `stick.reception_m` 1.4, no `ai.yield_margin_m`, `rules.penalties` true.
- Marathon: three a side (player, chaser, back v chaser, support, back; spawns +-7.5 / +-30), `sprint.drain_frames` 120, `refill_frames` 600.
- Cage, Bruisers: remove `contact.release_grace_frames` (the default 120). Outnumbered, Sudden Death: remove `stick.take_grace_frames` (the default 30).
- `tools/playtest/modes.mjs`: the two `CHARACTER` entries back to `avg(... / match)`; then `tools/soak/drift.test.mjs:35` loses `play_frames: 3600`.

## Findings outside the family

1. `tools/soak/drift.test.mjs:35` pinned Marathon's number as sprint frames a match with fake matches that have no `play_frames`: with the number per play minute the
   test read 0. **One-line fix made** (the fake matches carry `play_frames: 3600`; the expected 25 holds). `tools/soak/baseline.json` holds `m-marathon` (3032.77 a
   match) and `m-power-play` (7.77 a match) under the old labels: the next drift run flags both; the soak's owner re-takes the baseline.
2. `tools/qa/accept/matrix-cuts/mode-*.json` are generated copies of the mode files (`tools/qa/accept/make-matrix-cuts.mjs`): eight are stale after this landing
   (pond, power-play, marathon, cage, bruisers, outnumbered, sudden-death, glass-house). Re-run the generator with the landing.
3. `Game/Source/JamCore/AI/JamAICore.cpp:703`: the support's shape runs only for a team of five, and the weak-side mate ahead of the puck is the second support
   (`:719-733`); the usual five (player, chaser, support, back, back) has one support, whose job is the outlet (`:710-718`). Under five a side no mate stands ahead for a
   pass (Pond 3 v 3 and 4 v 4: 0.2 human passes a play minute; 5 v 5 with two supports 2.1). R-0052 point 3 owes "shapes for three a side": this is its number.
4. `Game/Source/JamCore/AI/JamPlaytester.cpp:353`: the playtester passes only under pressure or with the lane blocked, never because a mate is better placed, so a
   man advantage lowers its passing (5 v 3: 1.28 a play minute against the regular game's 1.51). A person on a power play passes to the open man: a "better-placed
   mate" pass for the regular and good classes (as the brain's `bBetter`, `JamAICore.cpp:1090`) would make every passing number honest.
5. `JamPlaytester.cpp:49-67`: the novice profile has no `SprintPct`: Marathon's number is 0 against 0 for the class.
6. `schema/game.schema.json` `skater.pass_speed` (5..30) and `schema/DEFAULTS.md`: nothing says that a pass under about 12 m/s is no longer a pass after 8 to 12 m
   (a reception needs 6 m/s at arrival: `JamSim.cpp:2500-2542`; `puck.drag` 0.5). Pond shipped with 10 for two passes on that account. Proposed description:
   "with puck.drag 0.5 a pass loses 0.5 m/s a metre and is received between 6 and 14 m/s: 14 reaches 16 m, 10 reaches 8 m".
7. R-0048 point 2 takes the whistle out of the regular game; `Game/Content/Cartridges/hockey.regular.cartridge.json:150` and the ladder's takes still say
   `"penalties": true` on this main. When that lands, "no whistle" stops being a fact that differs on four cards (Pond, Cage, Bruisers, Marathon): they need a new fact.
8. No gate step reads card text: `game.meta` passes the schema (lengths) and nothing else; "gamble" sat on Glass House since pass 3 at the latest. A node test that runs every
   `game.meta` line through `tools/barks/filter.mjs` `checkLine` with a card allow-list (numbers, the key F) would hold the rating on the cards.
9. `tools/barks/barks.test.mjs:63` still fails on CRLF (`BarkData.inl`), as MODES-3 finding 6: the gate's node step reads 233 of 234 here.
10. MODES-3's findings 2 (the blast's width is a constant, `JamSim.h:458`) and 5 (the playtest line has no stumbles, board hits or cause) stand.

## Proof

- Before and after: the table above (`tools/playtest/out/a139/base.md`, `after.md`), 200 seeds a mode; `modes.mjs` exit 0 after (exit 1 before: Marathon 1.08x
  under its per-minute number). The shipped Marathon replays the final run 200 of 200 after the back was put back.
- Determinism between the two 200-seed runs: the four regular cells and arsenal, fog-of-war, glass-house, shotgun, sniper: 200 of 200 identical hashes each.
- `node schema/validate.mjs` on the twelve mode files: ok, 0 errors each. `node --test` on the gate's globs: 233 of 234 (finding 9; 232 before the one-line fix of finding 1).
- The full `Jam` automation filter on the final tree, the gate's own invocation (`tools/gate/premerge.ps1` line 102: `Automation RunTests Jam; Quit`, `-nullrhi
  -Multiprocess -unattended`): **191 of 191 Success**, exit code 0, no "missing usage flag" line (log `Game/logs/a139-jam-final.log`, 23:23). Among them
  `Jam.Menu.DirectorsChoices` (twelve cards, three facts each), `Jam.Core.CartridgeRoundTrip`, `Jam.Core.CartridgeParity`, `Jam.Core.ContactModes` (plays Cage and
  Bruisers), `Jam.Core.Abilities.*`, `Jam.Core.FuzzSmoke`, `Jam.Net.GuestSeat`, `Jam.Net.HashCoverage`.
- The hash: `Jam.Core.Determinism` (hockey.regular, seed 1, frame 600) `0ff7ea98d5570484`, hockey.raw `8425610943e6dd92`: as recorded on main (`MOVEMENT-3.md:164`);
  content under `modes/` cannot reach them, and the regular cells of the playtest runs are hash-identical before and after.
- Not run: the acceptance run (no verb of the player changed; a packaged pass over the re-made modes is track S2's matrix, with finding 2) and the three curve cells
  (nothing the ladder reads changed).

## Next evolutions for the track, ranked

1. **The win rates on the re-set ladder** (when the curve lane lands): all twelve re-measured; every mode inside 15 to 85 % for the regular class and at 10 % or
   more for the novice. Known levers: Fog of War `ai.react_frames` 6 -> 4 (87.5 today), Sniper `goalie.block_height_m` 0.25 (19.5), Marathon (87; the bots' sprint
   or the mate), Power Play's `ai.body_check` (human down 2.55 a play minute against 2.38). Pond, Arsenal and Bruisers keep the name rookie.
2. **Marathon's puck**: touches a minute 9 or more (5.9 today; regular 10.9) and a 30 s stretch without the puck in under half the matches (96 % today), with the
   sprint still 1.33x and the novice at 10 % or more. Levers: `puck.count` 2, 120 x 60 at two a side, a mate that gives the puck.
3. **Arsenal's tricks**: three abilities used a match (0.65 today; 108 of 200 matches see none). Levers: boosts every 240 frames with three on the ice, first to 5,
   one ability held from the faceoff (`trigger` mode) beside the pickups.
