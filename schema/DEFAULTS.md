# Engine defaults for absent keys

The validator only checks what is written. It never fills anything in. The C++ engine (lane A1) applies these defaults when a key is absent
("absent value = omit the key": an omitted key means the default, never 0 and never null). Presets in `Game/Content/Cartridges/` write most
of them out explicitly, but a Designer or hand-written cartridge may leave them out.

| Key | Default | Note |
|---|---|---|
| `rink.corner_radius_m` | 5 | |
| `rink.board_restitution` | 0.4 | arcade bounce, not measured, A3 sweep may retune |
| `rink.ice_friction` | 0.015 | glide friction coefficient (0.015 x 9.8 = 0.147 m/s^2) |
| `rink.goal_width_m` | 4 | |
| `rink.obstacles` | none | |
| `skater.accel` | 12 | |
| `skater.turn_rate` | 5 | |
| `skater.lateral_grip` | 3 | |
| `skater.mass` | 80 | |
| `skater.glide_drag` | 0.147 | |
| `skater.brake_decel` | 6 | |
| `skater.check_enabled` | true | R-0013: the body check is in every cut (was false before H7) | |
| `skater.check_speed_min` | 5 | only read when `check_enabled` is true |
| `stick.reach_m` | 1.2 | lane A16 (R-0013); 1.2 by R-0022 point 3 (rest 1.0 m, full aim 1.6 m) |
| `stick.poke_window_frames` | 8 | lane A16 (R-0013) |
| `stick.lift_range_m` | 1.0 | lane A16 (R-0013) |
| `stick.stumble_frames` | 20 | lane A16 (R-0013); 0 in `hockey.raw` |
| `stick.mouse_sensitivity` | 1 | lane A16 (R-0013) |
| `skater.knockdown_frames` | 90 | only read when `check_enabled` is true |
| `skater.pass_speed` | 14 | lane A16 (C-0005) |
| `skater.pass_lead_frames` | 20 | lane A16 (C-0005) |
| `skater.visual` | `toy` | which figure is drawn: `toy`, `humanoid_b1`, `generated_b2`; gameplay is identical; the Designer may not change it (R-0001) |
| `puck.radius_m` | 0.12 | |
| `puck.max_speed` | 30 | |
| `puck.drag` | 0 | |
| `puck.carry_assist` | 0.6 | lane A16 (C-0005): pull towards the stick socket while the carrier turns under 90 deg/s |
| `puck.count` | 1 | lane A16 (C-0005) |
| `puck.colour_hex` | `#111111` | near-black; readability comes from trail and shadow disc |
| `shot.reach_m` | 1.5 | |
| `shot.speed` | 18 | |
| `shot.cooldown_frames` | 24 | |
| `shot.aim_cone_deg` | absent = no aim assist | |
| `shot.charge_frames_max` | 30 | lane A16 (C-0005): hold shoot up to this many frames for the bonus |
| `shot.charge_speed_bonus` | 8 | lane A16 (C-0005): m/s at full charge |
| `shot.deflect` | true | lane A16 (C-0005): a shot into a skater within 0.6 m of its stick keeps 70 % of its speed |
| `rules.goal_pause_frames` | 120 | |
| `rules.offside` | false | only false is accepted |
| `rules.penalties` | true | R-0013: fouls (from behind, or on a skater without the puck) = 300 frames in the box + a free faceoff at the fouler's end; `hockey.raw` sets false |
| `ai.difficulty` | `pro` | lane A3; the five numbers below, when absent, come from the level's profile (`docs/experiments/BALANCING.md`), not from this table |
| `ai.yield_margin_m` | 2 (pro) | |
| `ai.yield_radius_m` | 15 | |
| `ai.hold_radius_m` | 0 (pro; 4.5 before the difficulty sweep) | |
| `ai.speed_scale` | 1 | |
| `ai.red_skill` | 1 | |
| `pickups` | none | |
| `pickups[].max_on_ice` | 1 | lane A14 (C-0005) |
| `pickups[].zone` | absent = anywhere on the ice | |
| `effects` | none (the five built-ins are always available to pickups) | lane A14; `Game/Content/Designer/builtin-effects.json` |
| `effects[].kind` | `custom` | the visual family; built-ins carry their own |
| `effects[].stack` | `refresh` | |
| `effects[].icon` | by kind: bolt (speed), crosshair (precision), weight (heavy_puck), circle (big_puck), swirl (reverse_other), star (custom) | |
| `effects[].stats[].multiply` / `add` | 1 / 0 | |
| `look.trim_colour_hex` | `#38BDF8` | |
| `look.fog_density` | 0 | |
| `look.trim_emissive` | by tier: 30 (`l1_day`, `l2_night`, `l3_neon`), 12 (`p3_clay`) | B-to-A-0018 point 5; the beacon stays at 40 |
| `look.wet_ice` | false | |
| `camera` (whole object) | see below | measured by lane E4, `docs/experiments/CAMERA-120M.md` |
| `barks.enabled` | true | lane A12, ruling R-0007; read by `UJamBarkSystem` from the cartridge's raw JSON |
| `rink.obstacles[].kind` `bumper` | restitution 1.6 for the puck | lane A15 (C-0005); `goal_shield` is a wall `size` m wide and 0.3 m deep |
| `rink.zones` | none | lane A15 (C-0005) |
| `rink.zones[].strength` | 0.5 | slow_ice at 1 halves speed and makes the glide 4x stickier; fast_ice at 1 adds half the speed; a bumper zone at 0.5 has the bumper obstacle's 1.6 |
| `rink.zones[].visible` | true | |
| `rules.hooks` | none | lane A15 (C-0005); evaluated in id order |
| `rules.hooks[].after_frame` | 0 | the hook is armed from this frame |
| `rules.hooks[].team` | `scoring` | the team the `score` condition reads and the actions apply to; an action's own `team` wins |
| `rules.hooks[].then[].team` | the hook's team | |
| `rules.variants` | none | the loader expands each variant into a hook named `variant-*` after the explicit hooks (`expandVariants` in validate.mjs, `ExpandVariants` in JamCartridge.cpp): `sudden_death` = on_goal after frame 600 end_match; `last_goal_double` = on_goal at score_to_win add_score 1 (first_to only); `mercy_goals` = on_goal lead_at_least N end_match; `period_count` = on_frame every timer_frames / N (3600 in first_to) end_period; `shrink_per_goal` = on_goal shrink_rink by the fraction |
| `barks.intensity` | `normal` | `quiet` doubles the cooldowns and never mocks, `loud` halves them |

## Camera defaults (when `game.camera` or any of its keys is absent)

| Key | Default |
|---|---|
| `camera.mode` | `third_person` |
| `camera.distance_m` | 6 |
| `camera.height_m` | 3.5 |
| `camera.look_ahead_m` | 5 |
| `camera.lag_frames` | 4 |
| `camera.fov_deg` | 70 |
| `camera.radar` | true |

Wide option (not a default): 8 / 4 / 6 / 6 frames / fov 85. Pitch is derived, never stored: `-atan(height_m / (distance_m + look_ahead_m))`.
In `arena` mode the distance, height, look-ahead and lag keys are ignored.

## Derived by the engine, not in the cartridge

- Goal line at `length_m / 2 - 4` m from the centre on each end (the Godot rink: 60 m long, nets at x = +-26).
- Pickup `magnitude` absent: 1.5 for `speed`, 2 for `precision` (cone multiplier), 2 for `heavy_puck` (drag multiplier, max speed divided by it), 1.5 for `big_puck` (radius multiplier); `reverse_other` ignores it. For a pickup naming a custom effect, `magnitude` scales the effect's deviation from neutral (`multiply' = 1 + (multiply - 1) * magnitude`, `add' = add * magnitude`, then clamped to 0.25..4 and the stat range); absent = 1 = as written.
- Effect modifiers compose as `base * product(multiply) + sum(add)` per stat, clamped to the stat's schema range; the same effect on the same target while active: `stack` decides (refresh restarts the timer, extend adds the duration up to 7200 frames, ignore keeps the running one).
- A `values` entry overrides the game field a tunable is named after (README.md, "Tunables"); an absent entry leaves the game field alone.
