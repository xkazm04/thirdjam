# Director's choices: six modes that change the character of a match (lane A67, R-0036, device A)

Status: 2026-11-13 10:40, six of six proven headless. Owner: lane A67. Files: `Game/Content/Cartridges/modes/*.cartridge.json` (six cuts),
`Game/Content/Cartridges/modes/MODES.md` (the keys per file), `tools/playtest/modes.mjs` (the proof runner), the menu's "Director's choices" row
(`MenuWidget.cpp` `RefreshModes`, `MenuFlow.cpp` `ModeCards`), the match card's line (`DirectorsCallWidget.cpp` `ShowTake`), tests `Jam.Menu.DirectorsChoices`
and `Jam.Core.CartridgeRoundTrip` (now also loads `modes/`). No sim code changed; every mode is a cartridge of keys that exist.

## How a mode is proven

`node tools/playtest/modes.mjs` validates each file with `schema/validate.mjs`, then plays it with lane A34's **regular** playtester (`-player=regular`, the real
input path, PLAYTEST.md section 1) at the mode's own level for seeds 1-20 (36,000-frame cap), next to `hockey.regular` at the same level on the same binary. Each mode
has one **character number**, the thing the mode is for; a mode passes when that number differs from regular's by a third or more and all 20 matches end. Raw lines:
`tools/playtest/out/modes-final.jsonl` (240 matches: 6 modes and 2 regular baselines; the baselines hash equal to round 1's, so the comparison is deterministic).
Rounds r1-r5 in the same folder are the retunes below.

## The six modes against regular (final run, 20 seeds each)

| mode | pitch | level | W-L | win % (regular) | GF / GA (regular) | touches / min (regular) | checks landed (regular) | character number | mode | regular | change |
|---|---|---|---|---|---|---|---|---|---|---|---|
| **Pond** | Skate and pass on an outdoor pond. Nobody hits. | rookie | 13-7 | 65 (100) | 1.6 / 1.2 (3.0 / 0.3) | 24.6 (22.0) | 0 (1.2) | passes made + received per play minute | **2.2** | 1.44 | **+55 %** |
| **Cage** | A tight box with live boards and pillars. Hit, don't poke. | pro | 6-14 | 30 (50) | 2.8 / 4.3 (2.2 / 2.3) | 22.7 (25.9) | 5.2 (3.2) | checks landed per match | **5.2** | 3.2 | **+63 %** |
| **Sniper** | A full-height net and a low crease guard. Lift it. | pro | 15-5 | 75 (50) | 2.5 / 1.6 (2.2 / 2.3) | 23.0 (25.9) | 3.4 (3.2) | human goals per human shot | **0.35** | 0.17 | **+103 %** |
| **Marathon** | Long ice and a five-minute clock. Pace it, then go. | pro | 15-5 | 75 (50) | 3.9 / 1.6 (2.2 / 2.3) | 21.3 (25.9) | 4.3 (3.2) | minutes per goal | **0.98** | 0.65 | **+51 %** |
| **Power Play** | Four of yours against three. Move it and shoot. | pro | 11-9 | 55 (50) | 2.9 / 2.3 (2.2 / 2.3) | 24.3 (25.9) | 6.3 (3.2) | passes made + received per match | **4.9** | 2.0 | **+145 %** |
| **Blind Side** | Dekes that send the back the wrong way, checks that stick. | pro | 12-8 | 60 (50) | 2.3 / 2.1 (2.2 / 2.3) | 26.9 (25.9) | 3.8 (3.2) | dekes per match | **1.5** | 1.1 | **+36 %** |

Other numbers that show the character (same lines): Pond has no checks and no fouls, 7.6 shots per play minute (regular rookie 6.7) and carries of 3 s drop from 2.0 to
0.4 per play minute (the puck moves by pass, not by carry). Cage's human spends 54 % of play frames skating or carrying (regular 78 %): the rest is contact; the match is
a 3:00 clock (3:26 with goal pauses) and 7.0 goals are scored in it (regular 4.5 in 2:54). Marathon's matches last 5:25 with 6.2 breakaway bursts (regular 3.8) and 27 s of waiting per match
(regular 4 s): long ice is the pace. Power Play's 4:00 has 6.3 checks by the human (the extra teammate frees him to forecheck) and 4.0 passes received (regular 1.4).
Blind Side is the thinnest margin (+36 %): the playtester's deke count is set by its own trigger (inside 12 m, the crease back on the line, one coin in two), so the
cartridge moves it only through more chances in front; its other half (checks from 2 m/s, down 4 s) is +19 % checks.

## What the measurement changed against the Director's suggestions

- **Pond**: suggested 60 x 30, 3 v 3, low friction, no penalties, rookie bots that never check, "touches" as the number. Touches per minute did not move in any of four
  variants (24.8 first to 5, 21.8 at 2 v 2 on 56 x 28, 24.9 3 v 3 on a clock, 24.7 with soft passes; regular rookie 22.0): the human's touch rate is set by the
  playtester's chase, not by the rink. First to 5 left 10 of 20 matches unfinished at 10 minutes (67 shots per match, 3 % in: the rookie back owns a 4 m net on a 60 m rink).
  Shipped: 3:00 clock, 6 m nets, `pass_speed` 10 and `stick.reception_m` 3 (a soft pass seats on the blade from 3 m), and the number is the passing (+55 %).
  Win 65 % against regular rookie's 100 %: the pond is a scramble, not a gift.
- **Cage**: suggested all_star bots. At all_star the human landed fewer checks than on regular (2.0 against 2.5): all_star carriers never come close. Shipped at pro,
  speed 12, stick reach 0.6 m with a 2-frame poke window (the stick can hardly poke, so the body does the work), 3:00 clock. Checks +63 %; the human loses 30 %:
  hitting costs position. "Checks taken" stays 0 in every mode because the bots never body check (ask A5).
- **Sniper**: kept as suggested. Its number is the human's goals per shot, not goals per match: under first to 3 the score caps goals per match (4.1 against 4.5 is the
  cap, not the net). The human shoots less (6.8 shots against 10.5) and scores twice as often per shot.
- **Marathon**: the suggested number, sprint frames, cannot be measured: neither the playtester nor the bots ever press sprint (`JamSim.h`: "Bots never set it";
  `JamPlaytester.cpp` has no sprint input), and 10 minutes is above `rules.timer_frames`' 18,000 (5:00). The mode keeps the Director's name and pace with the keys that
  exist (5:00, 140 x 70, breakaway burst +60 % for 4 s, sprint +40 % for the human) and its number is the pace: a goal every 0.98 minutes against 0.65.
- **Power Play**: first to 5 left 2 of 20 unfinished at 10 minutes (4.0 / 3.2 goals: the three bots score too). Shipped on a 4:00 clock; passes +145 %.
- **Blind Side**: "bots' reaction frames high" has no cartridge key (ask A4: `opp.reaction_frames` exists only as `-aitune`). With the deke and the check alone, dekes rose
  23 %; adding a shorter crease reach (0.5 m) and a longer commit (60 frames) gives the human more drives to the slot: +36 %.
- **Tried and dropped: Three Pucks** (`puck.count` 3). All 20 matches hash equal to regular: `puck.count` is parsed (`JamCartridge.cpp`) and used nowhere in the sim
  (ask A10).

## The menu

Choose your cut opens with **DIRECTOR'S CHOICES**: six cards in a 3 x 2 grid, each with the mode's name, its pitch and three facts that differ from the Regular Cut,
above the cut list (which keeps its film-strip rows; its box is 250 px tall now so the card fits 720p). A card is chosen like a cut (`ChooseCut("modes/hockey.mode.<slug>")`
-> `LoadCut` -> `StartMatch`; a name with a slash travels in `jam.Cut` only). The match card's small line names the mode: `POND   -   PERIOD 1   -   FACEOFF`. The pause
menu's controls card is unchanged. Stills: `docs/experiments/img/modes/`.

## Asks: keys that do not exist (each mode ships without)

| ask | key, range | effect | for |
|---|---|---|---|
| A1 | `rules.timer_frames` max 18000 -> **36000** | a 10-minute match; 5:00 is the ceiling today | Marathon |
| A2 | `ai.sprint` boolean (absent = false) | bots press lane A49's sprint on breakouts and backchecks, so stamina is a shared economy | Marathon |
| A3 | playtester sprint (tooling in `JamPlaytester.cpp`, not a key) | the regular / good player sprints on a breakaway and a backcheck; without it no headless run measures sprint frames | Marathon |
| A4 | `ai.react_frames` 0..60 (absent = the level's profile) | the bots' reaction delay from the cartridge (today `opp.reaction_frames` only through `-aitune`) | Blind Side |
| A5 | `ai.body_check` 0..1 (absent = 0) | the share of chances a bot closes with a body check; checks taken by the human are 0 in every cut | Cage, Blind Side |
| A6 | `teams[].skater_scale` 0.5..1.5 | one side's speed and shot scaled: a handicap without changing the head count | Power Play, Penalty Kill |
| A7 | `rules.offside` true | offside as a real rule (today only `false` is accepted) | Marathon |
| A8 | `rules.penalty_frames` 120..3600 | how long a foul sits, so Cage could make fouls cost instead of switching penalties off | Cage |
| A9 | `look.tier` `pond_day` (B's asset: open sky, snowbanks for boards) | Pond's own look; it uses `l1_day` meanwhile | Pond |
| A10 | `puck.count` 1..3 wired into the sim | the key validates and loads but no system reads it | a Three Pucks mode |
| A11 | an envelope `description` (or `mode.pitch` / `mode.facts`) | the pitch and facts live in the cartridge, not in a C++ table | the menu |

## Six more mode ideas and the keys each needs

| idea | pitch | keys (exist unless marked **new**) |
|---|---|---|
| Moon Rink | The puck floats and bounces; lift it over everyone. | `puck.gravity_mps2` 1.6, `puck.ice_restitution` 0.8, `shot.lift.full_m` 2, `rink.goal_height_m` 1.5 |
| Shrinking Rink | Every goal pulls the boards in by a tenth. | `rules.variants.shrink_per_goal` 0.1, `rules.score_to_win` 5 (the headless playtester runs no rule hooks: prove it with `JamCutSweep`, CUTS.md) |
| Sudden Death 2 v 2 | Two a side, one goal ends it. | `teams[].skaters` 2 each, `rules.score_to_win` 1, `rules.variants.sudden_death` |
| Penalty Kill | Three of yours against four; hold the clock. | `teams[].skaters` 3 v 4, `rules.mode` timer 2:00; **new** `rules.win_if` `fewest_against`, or A6 |
| Pinball | Bumpers and a dead-fast puck off live boards. | `rink.obstacles[]` bumper x6, `rink.board_restitution` 1, `puck.drag` 0 |
| Heavyweights | Slow, heavy skaters; every check moves the play. | `skater.mass` 140, `skater.speed` 7, `skater.accel` 6, `skater.check_speed_min` 2; **new** A5 so the bots hit back |
