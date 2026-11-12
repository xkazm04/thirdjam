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
| `skater.check_enabled` | false | |
| `skater.check_speed_min` | 5 | only read when `check_enabled` is true |
| `skater.knockdown_frames` | 90 | only read when `check_enabled` is true |
| `puck.radius_m` | 0.12 | |
| `puck.max_speed` | 30 | |
| `puck.drag` | 0 | |
| `puck.colour_hex` | `#111111` | near-black; readability comes from trail and shadow disc |
| `shot.reach_m` | 1.5 | |
| `shot.speed` | 18 | |
| `shot.cooldown_frames` | 24 | |
| `shot.aim_cone_deg` | absent = no aim assist | |
| `rules.goal_pause_frames` | 120 | |
| `rules.offside`, `rules.penalties` | false | only false is accepted |
| `ai.yield_margin_m` | 2 | |
| `ai.yield_radius_m` | 15 | |
| `ai.hold_radius_m` | 4.5 | |
| `ai.speed_scale` | 1 | |
| `ai.red_skill` | 1 | |
| `pickups` | none | |
| `look.trim_colour_hex` | `#38BDF8` | |
| `look.fog_density` | 0 | |
| `look.wet_ice` | false | |
| `camera` (whole object) | see below | measured by lane E4, `docs/experiments/CAMERA-120M.md` |

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
- Pickup `magnitude` absent: 1.5 for `speed`, 2 for `precision` (cone multiplier), 2 for `heavy_puck` (mass multiplier), 1.5 for `big_puck` (radius multiplier); `reverse_other` ignores it.
- A `values` entry overrides the game field a tunable is named after (README.md, "Tunables"); an absent entry leaves the game field alone.
