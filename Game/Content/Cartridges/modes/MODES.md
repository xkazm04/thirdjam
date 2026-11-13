# Director's choices: the six mode cartridges (lane A67, R-0036)

Each file is a full `tj.cartridge/1` cut built from `hockey.regular` with only the keys below changed. The menu lists them in the "Director's choices" row of
Choose your cut (`UJamMenuFlow::ModeCards`, `MenuFlow.cpp`), chosen like a cut (`LoadCut("modes/hockey.mode.<slug>")`); the match card's line names the mode.
They are not in the cut list: `ListCuts()` scans only `Content/Cartridges/*.cartridge.json`. Proof: `node tools/playtest/modes.mjs` (numbers in
`docs/experiments/MODES.md`). Validate: `node schema/validate.mjs Game/Content/Cartridges/modes/<file>`; the C++ loader runs them in `Jam.Core.CartridgeRoundTrip`
and `Jam.Menu.DirectorsChoices`.

| file | mode | pitch | keys changed from regular | level |
|---|---|---|---|---|
| `hockey.mode.pond.cartridge.json` | Pond | Skate and pass on an outdoor pond. Nobody hits. | rink 60 x 30, corner 3, `ice_friction` 0.005, `board_restitution` 0.6, `goal_width_m` 6; `skater.check_enabled` false, `pass_speed` 10, `pass_lead_frames` 30; `stick.reception_m` 3; `rules.penalties` false, timer 10800 (3:00); `look.tier` l1_day | rookie |
| `hockey.mode.cage.cartridge.json` | Cage | A tight box with live boards and pillars. Hit, don't poke. | rink 70 x 35, corner 2, `board_restitution` 0.95, 4 pillars (size 1.5 at +-12, +-8); `skater.speed` 12, `check_speed_min` 3, `knockdown_frames` 120; `stick.reach_m` 0.6, `lift_range_m` 0.5, `poke_window_frames` 2; `rules.penalties` false, timer 10800; `look.tier` l3_neon | pro |
| `hockey.mode.sniper.cartridge.json` | Sniper | A full-height net and a low crease guard. Lift it. | `rink.goal_height_m` 1.22; `skater.goalie` reach 0.9 / block height 0.15; `shot.speed` 26, `charge_frames_max` 60, `charge_speed_bonus` 12, `shot.lift` 0.1 / 0.7 / 1.1 / 20; `look.tier` p3_clay | pro |
| `hockey.mode.marathon.cartridge.json` | Marathon | Long ice and a five-minute clock. Pace it, then go. | rink 140 x 70, corner 5.83; `skater.speed` 10, `breakaway` 0.6 / 240, `sprint` 0.4 / 120 / 600; timer 18000 (5:00); `look.tier` p3_clay | pro |
| `hockey.mode.power-play.cartridge.json` | Power Play | Four of yours against three. Move it and shoot. | blue 4 skaters (player, chaser, support, back) v orange 3; `skater.pass_speed` 16; `stick.reception_m` 1.4; `shot.one_timer` 20 / 14; timer 14400 (4:00); `look.tier` p3_clay | pro |
| `hockey.mode.blind-side.cartridge.json` | Blind Side | Dekes that send the back the wrong way, checks that stick. | `skater.check_speed_min` 2, `knockdown_frames` 240, `goalie` reach 0.5 / block 0.4; `stick.deke` 2 / 60, `foul_grace_frames` 30 | pro |

Adding a seventh: write the file here, add a row to `UJamMenuFlow::ModeCards` (name, id, pitch, three facts), add its character number to `CHARACTER` in
`tools/playtest/modes.mjs`, and prove it differs from regular by a third.
