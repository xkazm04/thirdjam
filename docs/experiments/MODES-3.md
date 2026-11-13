# Director's choices, pass 3: twelve modes under the vision (lane A122, track M1, device A, 2026-11-13)

Owner: lane A122. Earlier passes: `MODES.md` (A67), `MODES-2.md` (A81), the keys `MODE-KEYS.md` (A72), the abilities `ABILITIES.md` (A84); the soak that found the
faded modes: `SOAK-2.md` section 3 (A107). Build: this worktree at main 25df19f (the vision, the boards as a body, the puck kept), editor Development.
Files changed: five cartridges and one card under `Game/Content/Cartridges/modes/`, `modes/MODES.md`, `tools/playtest/modes.mjs`. No sim, brain or profile code.

**The ladder is off on this commit** (R-0050: the regular class wins 100 / 78 / 19 at rookie / pro / all_star here against targets of 85 / 48 / 26). Read every
win rate below against the regular game on the same row, not as an absolute; they are re-checked when track C0's curve lands (next evolutions, 2).

## How it was measured

`node tools/playtest/modes.mjs --seeds 1-200 --jobs 6`: each mode cartridge validated, then 200 matches by the **regular** playtester at the mode's level, next to
`hockey.regular` at the same level on the same seeds and binary; one `UnrealEditor-Cmd` per cell (16 cells, 163 s for 3,200 matches, `-Multiprocess`). A mode passes
when its character number differs from regular's by a third or more and every match ends. The driver changed in this pass (it measures; it ships nothing):

- A timer mode's frame cap is its clock plus 6,600 frames (the sim's hard stop is the clock plus 6,000: `JamSim.cpp:1136`), so a long clock is not read as unfinished.
- `RUN` holds what a mode's cell needs: Marathon's playtester sprints (`-playersprint`) and Marathon is compared with a regular cell that sprints too (`pro+sprint`).
- `--dir <folder>` measures variant files (`hockey.mode.<slug>.<tag>.cartridge.json` is measured with the slug's number); `--baseline` reuses regular cells of the same seeds.
- The table carries the ratio, the match length, the human's knockdowns a match and a 15 to 85 % win band (a note: the level is the ladder's, not the mode's).
- Shotgun's number is per play minute, no longer per match (its match is a third shorter than a regular one: the per-match count read the length, not the blast).

Raw lines, tables, the 63 variant cartridges and their specs: `tools/playtest/out/a122/` (`base.*` before, `after.*` after, `r1..r5.*` the variant rounds at 60 to 100
seeds, `after-novice.*`, `after-good.*`; the folder does not travel). Determinism between the two 200-seed runs: the four regular cells and the seven untouched
modes replay with 200 of 200 identical hashes each.

## The twelve, before and after (200 seeds each, regular class)

Regular on this build: rookie 100 % wins in 87 s, pro 78 % in 238 s (pro with sprint 73 % in 231 s), all_star 19 % in 255 s; the human is knocked down 2.7 / 7.9 / 8.5
times a match.

| mode | level | character number | before: mode / regular = ratio | after: mode / regular = ratio | win % before -> after (regular) | match s before -> after | human knockdowns / match before -> after |
|---|---|---|---|---|---|---|---|
| Arsenal | rookie | human abilities used / match | 0.51 / 0 | unchanged | 96 (100) **out of band** | 99 | 3.1 |
| Bruisers | rookie | hits (landed + taken) / play min | 4.9 / 3.41 = 1.44 | unchanged | 99.5 (100) **out of band** | 118 | 5.8 |
| **Cage** | pro | checks landed / match | 5.0 / 4.43 = **1.13, fails** | 7.94 / 4.43 = **1.79** | 41.5 -> 53 (78) | 214 -> 207 | 6.9 -> 9.1 |
| **Fog of War** | pro | human breakaways / play min | 2.7 / 0.5 = 5.44 | 1.72 / 0.5 = 3.46 | **96.5 -> 78.5** (78) | 90 -> 132 | 3.8 -> 5.5 |
| Glass House (card only) | pro | human box seconds / match | 7.92 / 4.5 = 1.76 | unchanged | 41 (78) | 137 | 3.6 |
| **Marathon** | pro, sprint | human sprint frames / match | 3306 / 846 = 3.91 | 1939 / 846 = 2.29 | **13.5 -> 65** (73) | 670 -> 516 | 13.2 -> 10.7 |
| **Outnumbered** | rookie -> pro | shots against / play min | 3.61 / 3.07 = **1.18, fails** | 5.73 / 3.61 = **1.59** | **99.5 -> 59** (100 -> 78) | 93 -> 138 | 3.0 -> 2.4 |
| Pond | rookie | passes + received / play min | 1.2 / 3.16 = 0.38 (the wrong way) | unchanged | 94 (100) **out of band** | 214 | 0 |
| Power Play | pro | passes + received / match | 4.41 / 9.07 = 0.49 (the wrong way) | unchanged | 63.5 (78) | 278 | 7.2 |
| **Shotgun** | pro | knockdowns (all skaters) / play min | 4.21 / 3.94 = **1.07, fails** (per match 9.44 / 14.56 = 0.65) | 5.81 / 3.94 = **1.47** | 40.5 -> 67.5 (78) | 150 -> 147 | 3.8 -> 4.4 |
| Sniper | pro | human goals per human shot | 0.61 / 0.2 = 3.0 | unchanged | 26 (78) | 85 | 2.5 |
| Sudden Death | all_star | match seconds | 37.1 / 254.6 = 0.15 | unchanged | 19 (19) | 37 | 0.5 |

After: `modes.mjs` exits 0: twelve of twelve a third apart, 0 unfinished of 2,400, the longest match of any mode 34,800 frames (Marathon; before 42,047).

## The three that faded, re-made

### Cage: the game of boards and bodies (1.13 -> 1.79)
"Tight box, eight bodies. The boards hit back." 60 x 28 m with the four pillars, four a side, `contact.board_bonus` 2 (a hit into the boards counts double: the
vision's lever of lane A101), no whistle, `check_speed_min` 2 (under the momentum law the key scales every hit by 5 / value: `JamSim.cpp:1849`), `knockdown_frames` 75
(up quicker than regular's 90, so the hits keep coming), bots check on half their chances. Its own old number holds again: the human lands 7.9 checks a match
against 4.4. Beside it: knockdowns of all skaters 7.0 a play minute against 3.9 (1.78x), the human down 2.9 times a play minute against 2.1, shots against 2.6x.
What the rounds showed (knockdowns all / play min against regular): board bonus 2 with no whistle on the old rink 1.41x; plus `check_speed_min` 2: 1.53x; the 60 x 28
box at four a side: 1.74x; with bots that always check (`ai.body_check` 1) 2.05x, but the human is down every 16 s and wins 35 %: too much against R-0049 (a tackle
is an event, not a rhythm), so 0.5 stays.

### Outnumbered: keeping the puck under pressure (1.18 -> 1.59)
"Three on five. Keep the puck, take the heat." The pressure was not there at rookie (rookies cannot take a kept puck: 99.5 % wins; shots against 1.18x, carry share 1.09x and
knockdowns 1.05x of regular). Now pro bots, and F is a shield (`abilities` shield, mode, 3 s every 10 s: the lane-A81 ask A12b, with the shield instead of the hip check because the
vision's carrier cannot hit). Pressure: 5.7 shots against a play minute against 3.6. Keeping: the human carries 20.1 % of play against 14.2 % (1.42x), makes 1.47
carries of 3 s a play minute against 0.79 (1.86x), is knocked down 2.4 times a match against 7.9, and uses the shield 4.7 times a play minute. Win 59 % (regular 78).
What did not work: the vision's own keep levers for everybody (`burst` 30 frames at 2.6, `reverse_brake_decel` 30, `carry_speed_mult` 1, `sprint.carry_bonus` 0.15)
raised shots against to 1.96x but **lowered** the human's carry share to 0.89x of regular: five pro bots use a better first step better than one playtester does.
A lever that helps only the outnumbered side is what this mode wants next: `game.stick.take_grace_frames` (below).

### Shotgun: the one-shot ability on F (1.07 -> 1.47)
"Tight ice, ten bodies. F clears the lane." The finding first: **on regular ice the blast drops 0.3 bodies a shot.** It knocks down every skater within 1 m of the
puck's path (`JamSim.h:458`, a constant), and on 90 x 45 almost nobody stands there: at five a side with the cooldown at 2 s instead of 20 s the mode still reads
only 1.31x regular's knockdowns a play minute; a bigger shot (`magnitude` 2.5 to 4), bouncier boards and a stronger crease guard each changed nothing. The crowd is
the lever a cartridge has: 64 x 32 m, five a side, boards at 0.9. There the same rink **without** the ability reads 1.08x and with it 1.47x (5.8 knockdowns a play
minute against 3.9; 60 x 30 reads 1.49x): the blast, not the rink, carries the third. 2.4 uses a play minute, 1.6x the shots against and 1.65x the goals a minute
of regular: a loud three-minute game. The cooldown stays 1,200 frames: `Jam.Core.Abilities` pins it (`JamAbilityTests.cpp:194`), and the measurement says the
cooldown is the weak lever anyway (20 s -> 5 s on a 70 x 35 rink: 1.37x -> 1.51x).

## Marathon ends on its horn, and can be won (13.5 % -> 65 %)
- **The horn is not a sim fault; the clock did not fit the tools.** `rules.timer_frames` counts live play; goal pauses and period breaks are off the clock; a tie
  at the horn is sudden death; the hard stop is the clock plus 6,000 frames (`JamSim.cpp:1136`, R-0037 as amended). A 10:00 clock (36,000) therefore ends anywhere up to
  42,000 frames (measured: 42,047 with the last pause), while every tool caps a match at 36,000 (`JamSim.cpp:55` says both stops "are under the 36,000-frame cap
  every tool uses", which holds only for a clock of 29,400 or less). The cartridge now plays **8:00** (28,800): the hard stop is 34,800, the longest of 200 matches
  is exactly that, 0 unfinished at any tool's cap. The schema still allows 36,000 (finding 1).
- **Why the regular class lost 86 %:** on 140 x 70 at three a side the breakaway bonus (+60 % for 4 s) fed the bots' breakaways (9.9 goals against a match, 9.3 shots
  against a play minute), and the whistle put the human in the box 20 s a match. Clock alone: 15 % wins; no whistle: 27 %; no whistle and no breakaway bonus:
  49 %; with the bots sprinting on a quarter of their chases instead of half: 59 % at 100 seeds, **65 % at 200** (regular with sprint: 73 %). Shipped: that last one.
- **What it cost, said plainly:** the mode's number (the human's sprint frames a match) is 2.29x regular's, but the match is 2.2x longer: per play minute the human
  now sprints as much as in the regular game (241 frames against 235; before 329). Marathon is today a long game on long ice with a scarce sprint bar (2 s of +40 %, 10 s
  to refill), not a game of more sprinting. A pace number that holds per minute is the third next evolution.

## The other win rates (brief item 5)
- **Fog of War 96.5 %**: turned inside the cartridge: `ai.react_frames` 12 -> 6 gives 78.5 % (regular 78) and still 3.5x the breakaways (8 frames: 90 %).
- **Arsenal 96, Bruisers 99.5, Pond 94** (all rookie; regular rookie is 100 here): the lever is the level name, and the level is track C0's tonight. Not turned:
  at pro Bruisers measured 5 to 10 % in pass 2, and rookie itself is about to move. Re-check on the new curve; if rookie stays near 100, Bruisers goes to pro with
  `ai.body_check` 0.5 and Arsenal to pro.
- **Sniper 26, Sudden Death 19, Glass House 41**: inside the band today, on a pro that wins 78. If the curve takes pro to its 48 target, Sniper is the first
  to leave the band; its lever is `skater.goalie.block_height_m` 0.15 -> 0.25 (the bots' lifted shots score too: 2.6 goals against in 85 s).
- Other classes on the changed modes (seeds 201-300, 100 each): novice / good wins: Cage 26 / 56 (regular game 25 / 92), Outnumbered 5 / 87, Shotgun 16 / 86,
  Marathon 12 / 90, Fog of War 41 / 87. The character holds for both classes except two novice cells: Marathon (the novice never sprints: 0 against 0) and Cage
  (4.72 checks landed against 3.55 = 1.33x, a hair under the third, so it reads as a fail).

## Pond and Power Play pass the wrong way (not re-made here)
Both are passing modes, and both now pass **less** than the regular game: Pond 1.2 passes and receptions a play minute against 3.16 (0.38x), Power Play 4.4 a
match against 9.1 (0.49x). The test is two-sided, so they read "a third apart"; the claim on their cards ("Pass, skate", "Move it, fire it") is no longer what the
number shows, because the regular game became five a side that pass. First next evolution.

## The cards (`game.meta`)
Every pitch and fact checked against its cartridge; the lines the vision made false are gone ("checks from N m/s" on Fog of War and Glass House: under the momentum
law the key scales the hit, so the fact is now "hits 5x harder" and "hits 1.7x harder"; "burst +60 % for 4 s" and "10:00" on Marathon; "1.5x, knocks down" on Shotgun;
"rookies that sprint" on Outnumbered; "short stick", "bots hit back" on Cage gave way to the facts that changed).

| mode | pitch | facts |
|---|---|---|
| Pond | Pond hockey. Pass, skate, nobody gets hit. | 60 x 30 m of ice / no hits, no box / soft passes, 3:00 |
| Cage | Tight box, eight bodies. The boards hit back. | 60 x 28 m, 4 v 4 / boards hit 2x / no whistle, no box |
| Sniper | Big net, low guard. Lift it, bury it. | net 1.22 m high / pads at 0.15 m / shot 26 m/s |
| Marathon | Eight minutes, endless ice. Pace it, then go. | 140 x 70 m, 8:00 / sprint +40 % for 2 s / no whistle, no box |
| Power Play | Four on three. Move it, fire it, repeat. | 4 v 3 for you / passes land soft / one-timers, 4:00 |
| Shotgun | Tight ice, ten bodies. F clears the lane. | F: shotgun shot / back every 20 s / 64 x 32 m, 5 v 5 |
| Sudden Death | One goal ends it. The meanest bots alive. | first goal wins / 120 x 60 m rink / all-star bots |
| Outnumbered | Three on five. Keep the puck, take the heat. | 3 v 5 against you / F: shield, 3 s / pro bots that sprint |
| Glass House | Every hit is a gamble. Fouls cost 10 s. | 10 s in the box / bots always hit / hits 1.7x harder |
| Arsenal | Grab a boost, get a trick on F. | boosts give F tricks / 4 abilities / rookie bots |
| Bruisers | Five a side, no whistle. Hit everything. | 5 v 5 on 80 x 40 m / 130 kg skaters / no whistle, no box |
| Fog of War | Slow bots in the fog. Slip by, then hit. | bots 0.1 s late / hits 5x harder / night rink, fog |

Lengths: pitches 31 to 45 characters (cap 48), facts 11 to 20 (cap 20); teen rating: hockey words only, no names, no brand.

## The two grace keys that land tonight (not in this tree, not used)
- `game.contact.release_grace_frames` (default 120): **Cage** and **Bruisers** want it short (45 to 60). Cage's number is hits: eight bodies on 60 x 28 make 7.0
  knockdowns a play minute, and two seconds of grace after every get-up will cut that; the mode's own `knockdown_frames` 75 already says "up quick, hit again".
  **Shotgun** needs an answer from the sim's owner: does the grace stop a blast (cause shotgun)? It should not.
- `game.stick.take_grace_frames` (default 30): **Outnumbered** wants it long (60 to 90): it is the keep lever that helps the side that has just won the puck,
  which the first-step levers did not. **Sudden Death** wants it at 0 ("the meanest bots alive").
Re-measure all four with the keys in (next evolutions, 2).

## Revert path
Content only; nothing here is read by the regular game or the ladder (the four regular cells replay hash-identical, so no curve cell moves). Each mode reverts by
restoring its file from 25df19f, or by key:
- Cage: rink 70 x 35 with pillars at +-12 / +-8, three a side, no `contact` block, `check_speed_min` 3, `knockdown_frames` 120, `penalties` true with `penalty_frames` 120.
- Outnumbered: `ai.difficulty` rookie, no `abilities`.
- Shotgun: rink 90 x 45, corner 3.75, boards 0.4, three a side.
- Marathon: `timer_frames` 36000, `penalties` true, `ai.sprint` 0.5, `breakaway` 0.6 / 240.
- Fog of War: `ai.react_frames` 12.
`tools/playtest/modes.mjs` only measures; with `--max-frames 36000` and the old folder it reads the old files as before, except Shotgun's number (per play minute).

## Findings outside the family
1. `schema/game.schema.json` `rules.timer_frames` allows 600..36000, and `Game/Source/JamCore/Private/JamSim.cpp:55-57` says the stops are under the 36,000-frame
   cap of every tool: a clock over 29,400 can end as late as clock + 6,000 (`JamSim.cpp:1136`). Either the schema's maximum becomes 28,800, or every tool's cap
   becomes clock + 6,600 (as `modes.mjs` now does). Contract text proposed: "`rules.timer_frames` 600..28800: with the sim's 6,000 frames of pauses and sudden
   death a match always ends inside 36,000 frames".
2. `Game/Source/JamCore/Public/JamSim.h:458` `ShotgunPathM = 1.0` is a constant: the blast drops 0.3 bodies a shot on regular ice and no cartridge key widens it.
   Proposed key: `game.abilities[].radius_m` (0.5..4, absent = 1) for the shotgun's path.
3. `Game/Source/JamCore/Private/Tests/JamAbilityTests.cpp:194` pins the Shotgun mode's cooldown at 1,200: a mode's tuning number in a C++ test. It should test
   that the mode holds one shotgun entry, not its cooldown.
4. `schema/game.schema.json` and `schema/DEFAULTS.md:22` describe `skater.check_speed_min` as the closing speed above which a check knocks down; under the momentum
   law it scales every hit by 5 / value (`JamSim.cpp:1849`). The description is the old rule.
5. `Game/Source/JamCore/AI/JamPlaytester.cpp:824-828`: the playtest line has knockdowns but no stumbles, no board hits and no cause: Cage's number cannot say
   "into the boards" and Shotgun's cannot say "by the blast". Asked: `stumbles`, `board_hits`, `knockdowns_by_shotgun` in the line.
6. `tools/barks/barks.test.mjs:63`: the regex wants LF; `Game/Source/JamGame/Private/BarkData.inl` is CRLF in this worktree and in the main checkout, so the
   gate's node step reads 212 of 213 here. Not caused by this lane.
7. The breakaway bonus on big ice feeds the bots (Marathon: 9.9 goals against a match with it and the bots' sprint at half, 4.0 without it and the sprint at a quarter): the brain takes breakaways the playtester does
   not. For track Q2 with the numbers above.

## Proof
- Before and after tables above (`tools/playtest/out/a122/base.md`, `after.md`), 200 seeds a mode.
- `node schema/validate.mjs` on the twelve mode files: ok, 0 errors each. `node --test` on the gate's globs: 212 of 213 (finding 6).
- The full `Jam` automation filter on the changed tree, the gate's own invocation (`tools/gate/premerge.ps1` line 102: `Automation RunTests Jam; Quit`, `-nullrhi
  -Multiprocess -unattended`): **175 of 175 Success**, exit code 0, no "missing usage flag" line (log `Game/logs/a122-jam-after.log`). Among them
  `Jam.Menu.DirectorsChoices` (twelve cards, three facts each), `Jam.Core.CartridgeRoundTrip` (the loader on every mode file), `Jam.Core.CartridgeParity`,
  `Jam.Core.Abilities.*` (eight, the Shotgun mode's pinned cooldown among them), `Jam.Net.GuestSeat` and `Jam.Net.HashCoverage` (both play the Shotgun file, now five a side).
  The same filter in a headless game instance (`-game`, no build tool call): 135 of 135.
- The hash: `Jam.Core.Determinism` (hockey.regular, seed 1, frame 600) `d192973ac7c97102` and the seam test's `f3613b5fcd33ce32`, both as recorded on main
  (`SOAK-LOCKSTEP-2.md`, `CONTACT-2.md`): unchanged, as content under `modes/` cannot reach them. In the playtest runs the four regular cells and the seven untouched
  modes replay 200 of 200 identical hashes between the before and the after run.
- Not run: the acceptance run (no verb of the player changed; a packaged pass over the re-made modes is track S2's matrix) and the three curve cells (nothing the
  ladder reads changed: the regular cells are hash-identical).
- Process note: the gate-form run sat 15 minutes at the editor's `Build.bat -Mode=QueryTargets` behind four other lanes' builds before its 60 seconds of tests;
  `-Multiprocess` does not skip that call in a full editor start (it does in a commandlet).
