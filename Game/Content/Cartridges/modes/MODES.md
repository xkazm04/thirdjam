# Director's choices: the twelve mode cartridges (lanes A67 and A81, R-0036, R-0041)

Each file is a full `tj.cartridge/1` cut built from `hockey.regular` with only the keys below changed, plus `game.meta` (lane A81: the card's `pitch` and up to three
`facts`; the card's name is the envelope `title`). The menu lists them in the "Director's choices" rows of Choose your cut, two rows of six (`UJamMenuFlow::ModeCards`,
`MenuFlow.cpp`, which keeps only the order and reads the words from the file), chosen like a cut (`LoadCut("modes/hockey.mode.<slug>")`); the match card's line names the mode.
They are not in the cut list: `ListCuts()` scans only `Content/Cartridges/*.cartridge.json`. Proof: `node tools/playtest/modes.mjs` (numbers in
`docs/experiments/MODES.md`, `MODES-2.md`). Validate: `node schema/validate.mjs Game/Content/Cartridges/modes/<file>`; the C++ loader runs them in `Jam.Core.CartridgeRoundTrip`
and `Jam.Menu.DirectorsChoices`.

| file | mode | pitch | keys changed from regular | level |
|---|---|---|---|---|
| `hockey.mode.pond.cartridge.json` | Pond | Skate and pass on a pond. Nobody hits. | rink 60 x 30, corner 3, `ice_friction` 0.005, `board_restitution` 0.6, `goal_width_m` 6; `skater.check_enabled` false, `pass_speed` 10, `pass_lead_frames` 30; `stick.reception_m` 3; `rules.penalties` false, timer 10800 (3:00); `look.tier` l1_day | rookie |
| `hockey.mode.cage.cartridge.json` | Cage | A tight box, live boards. Hit, don't poke. | rink 70 x 35, corner 2, `board_restitution` 0.95, 4 pillars (size 1.5 at +-12, +-8); `skater.speed` 12, `check_speed_min` 3, `knockdown_frames` 120; `stick.reach_m` 0.6, `lift_range_m` 0.5, `poke_window_frames` 2; `rules.penalties` true with `penalty_frames` 120 (lane A72), timer 10800; `ai.body_check` 0.5 (lane A72); `look.tier` l3_neon | pro |
| `hockey.mode.sniper.cartridge.json` | Sniper | A full-height net, a low guard. Lift it. | `rink.goal_height_m` 1.22; `skater.goalie` reach 0.9 / block height 0.15; `shot.speed` 26, `charge_frames_max` 60, `charge_speed_bonus` 12, `shot.lift` 0.1 / 0.7 / 1.1 / 20; `look.tier` p3_clay | pro |
| `hockey.mode.marathon.cartridge.json` | Marathon | Long ice, a long clock. Pace it, then go. | rink 140 x 70, corner 5.83; `skater.speed` 10, `breakaway` 0.6 / 240, `sprint` 0.4 / 120 / 600; timer 36000 (10:00, lane A72); `ai.sprint` 0.5 (lane A72); `look.tier` p3_clay | pro |
| `hockey.mode.power-play.cartridge.json` | Power Play | Four against three. Move it and shoot. | blue 4 skaters (player, chaser, support, back) v orange 3; `skater.pass_speed` 16; `stick.reception_m` 1.4; `shot.one_timer` 20 / 14; timer 14400 (4:00); `look.tier` p3_clay | pro |
| `hockey.mode.blind-side.cartridge.json` | Blind Side | Dekes that fool the back, checks that stick. | `skater.check_speed_min` 2, `knockdown_frames` 240, `goalie` reach 0.5 / block 0.4; `stick.deke` 2 / 60, `foul_grace_frames` 30; `ai.react_frames` 4, `ai.body_check` 1 (lane A72) | pro |
| `hockey.mode.sudden-death.cartridge.json` | Sudden Death | One goal ends it. The best bots. | rink 120 x 60, corner 5 (the large arena's spawns); `rules` first_to 1; `look.tier` p3_clay | all_star |
| `hockey.mode.outnumbered.cartridge.json` | Outnumbered | Three against five. Hold on, then strike. | orange 5 skaters (chaser x2, support x2, back) v blue 3; `ai.sprint` 0.75; `look.tier` l1_day | rookie |
| `hockey.mode.glass-house.cartridge.json` | Glass House | Every hit is a risk. Fouls cost 10 s. | `rules.penalty_frames` 600; `skater.check_speed_min` 3, `knockdown_frames` 150; `stick.foul_grace_frames` 0; `ai.body_check` 1; `look.tier` l3_neon | pro |
| `hockey.mode.pinball.cartridge.json` | Pinball | Bumpers, live boards, a puck that never slows. | `rink.board_restitution` 1, `ice_friction` 0.005, 8 bumpers size 2 (+-30/+-12, +-15/0, 0/+-12); `puck.drag` 0, `max_speed` 50, `ice_restitution` 0.8; `shot.speed` 30; `look.tier` l3_neon | pro |
| `hockey.mode.bruisers.cartridge.json` | Bruisers | Five a side, heavy bodies, no whistle. Hit. | rink 80 x 40; 5 v 5; `skater.mass` 130, `speed` 10, `accel` 9, `check_speed_min` 1.5, `knockdown_frames` 150; `rules.penalties` false; `ai.body_check` 1; `look.tier` l3_neon | rookie |
| `hockey.mode.fog-of-war.cartridge.json` | Fog of War | Slow bots in the fog. Slip past, hit soft. | `ai.react_frames` 12; `skater.check_speed_min` 1; `look` l2_night, `fog_density` 0.04 | pro |

Adding a mode: write the file here with a `game.meta` (pitch, up to three facts of at most 40 characters); the menu picks it up after the twelve in file order
(add its slug to the order in `UJamMenuFlow::ModeCards` to place it, and note the rows hold twelve at six a row); add its character number to `CHARACTER` in
`tools/playtest/modes.mjs` and prove it differs from regular by a third. Parked by lane A81: Tap-In (`tools/playtest/out/a81-variants/parked-tap-in.cartridge.json`, MODES-2.md).
