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
| `rink.goal_height_m` | 0.61 | R-0033 point 2 (lane A50): the half-height net of a game without goalies; a real net is 1.22. Net frame, netting, goal triggers and the sim's lifted-puck goal test read it |
| `rink.obstacles` | none | |
| `skater.accel` | 12 | |
| `skater.turn_rate` | 5 | |
| `skater.lateral_grip` | 3 | |
| `skater.mass` | 80 | |
| `skater.glide_drag` | 0.147 | |
| `skater.brake_decel` | 6 | |
| `skater.check_enabled` | true | R-0013: the body check is in every cut (was false before H7) | |
| `skater.check_speed_min` | 5 | only read when `check_enabled` is true |
| `stick.reach_m` | 1.2 | lane A16 (R-0013); 1.2 by R-0022 point 3; the range 0.6..1.6 by R-0034 (lane A51): the carried puck sits at the blade, forehand to backhand as the aim sweeps (the brief's 1.1 halved the regular player's passes received at pro: kept 1.2, STICK.md section 6) |
| `stick.poke_window_frames` | 8 | lane A16 (R-0013) |
| `stick.lift_range_m` | 1.0 | lane A16 (R-0013) |
| `stick.stumble_frames` | 20 | lane A16 (R-0013); 0 in `hockey.raw` |
| `stick.mouse_sensitivity` | 1 | lane A16 (R-0013) |
| `stick.foul_grace_frames` | 12 | lane A16 (R-0029 point 3): no `foul_no_puck` on a skater that carried within the last 12 frames |
| `stick.deke.offset_m` | 1.0 | lane A39 (R-0029 point 4): a deke tap (`bDeke`) moves the carried puck this far to the blade side (the side the stick is aimed to; the middle of the ice when it is not aimed) within 6 frames; a shot inside the commit window aims at the post on that side; a second tap inside 30 frames does nothing; 0 = off |
| `stick.deke.commit_frames` | 15 | lane A39: the crease back's stance and blade follow the puck's old side this long |
| `stick.sweep.min_deg_per_frame` | 6 | lane A51 (R-0034): an aim turning more than this in one frame on the carrier is a sweep (read-out flag `bSweepThisFrame`) |
| `stick.sweep.shield_frames` | 12 | lane A51: frames after a sweep (or a deke tap) in which an opponent's poke from the side the puck left misses; 0 = off |
| `stick.sweep.bot_react_frames` | absent | lane A51: a defending bot reads the puck's place on the carrier this many frames late; absent = the difficulty profile (rookie 14, pro 8, all_star 5) |
| `stick.reception_m` | 0.9 | lane A46 (PAYOFFS.md section 6): a teammate's pass reaching the body within this radius at 6-14 m/s seats on the blade with a 4-frame settle; faster passes bounce off the body unless the blade meets them; 0 = blade reception only |
| `skater.knockdown_frames` | 90 | only read when `check_enabled` is true |
| `skater.pass_speed` | 14 | lane A16 (C-0005) |
| `skater.pass_lead_frames` | 20 | lane A16 (C-0005) |
| `skater.breakaway.speed_bonus` | 0.2 | lane A39 (R-0029 point 4): a carrier past centre ice with no opponent between it and the net gets this fraction more top speed and acceleration for `frames`, once per possession (event `breakaway`); 0 = off |
| `skater.breakaway.frames` | 60 | lane A39 |
| `skater.carry_speed_mult` | 0.92 | lane A82 (R-0041 point 6): the carrier's top speed and acceleration as a fraction of a free skater's, bots and the human alike; the carrier cannot sprint and its bar does not drain; 1 = no carry penalty |
| `skater.sprint.bonus` | 0.15 | lane A49: Shift / LT held adds this fraction to top speed and acceleration while the skater moves and the stamina lasts; bots only with `ai.sprint` (lane A72); 0 = off |
| `skater.sprint.drain_frames` | 180 | lane A49: frames of sprint from full to empty (3 s) |
| `skater.sprint.refill_frames` | 360 | lane A49: frames at rest from empty to full (6 s) |
| `skater.goalie.reach_m` | 0.9 | lane A52 (R-0035): a skater in the crease stance (within 3 m of its own goal centre, the human included) saves a loose shot passing within this of it; 0 = off |
| `skater.goalie.block_height_m` | 0.4 | lane A52 (R-0035), Director 01:30: a shot at or above this height passes the crease skater; below the 0.61 bar so a half charge (0.45) is the skill shot: over the pads, under the bar (0.7 saved everything the net could take; the novice gate failed 8 of 20) |
| `skater.spin.frames` | 24 | lane A51 (R-0034): the spin on E / gamepad B: a full turn of the figure over this many frames, the puck on the far side of the body from the nearest opponent (a poke never reaches it); a check landing mid-spin knocks down 1.5x longer; 0 = off |
| `skater.spin.cooldown_frames` | 90 | lane A51: frames before the next spin |
| `skater.spin.speed_mult` | 0.7 | lane A51: top speed while spinning as a fraction of the skater's |
| `skater.spin.stamina_cost` | 0.25 | lane A51: the share of the sprint bar a spin takes; refused with an empty bar |
| `skater.visual` | `humanoid_b1` | which figure is drawn: `toy_skeletal` (the Blender toy figure as a skeletal mesh, lane A26, the default since H11.7), `toy` (the code-built figure), `humanoid_b1`, `generated_b2`; gameplay is identical; the Designer may not change it (R-0001) |
| `puck.radius_m` | 0.12 | |
| `puck.max_speed` | 30 | |
| `puck.drag` | 0 | |
| `puck.carry_assist` | 0.6 | lane A16 (C-0005): pull towards the stick socket while the carrier turns under 90 deg/s |
| `puck.count` | 1 | lane A16 (C-0005) |
| `puck.colour_hex` | `#111111` | near-black; readability comes from trail and shadow disc |
| `puck.gravity_mps2` | 9.81 | lane A52 (R-0035): gravity on a lifted puck |
| `puck.ice_restitution` | 0.3 | lane A52 (R-0035): the bounce on the ice; a hop under 2 cm dies. The glass is 2.4 m (fixed, no key): a puck over it at the boards is out of play (free faceoff) |
| `shot.reach_m` | 1.5 | |
| `shot.speed` | 18 | |
| `shot.cooldown_frames` | 24 | |
| `shot.aim_cone_deg` | absent = no aim assist | |
| `shot.charge_frames_max` | 30 | lane A16 (C-0005): hold shoot up to this many frames for the bonus |
| `shot.charge_speed_bonus` | 8 | lane A16 (C-0005): m/s at full charge |
| `shot.deflect` | true | lane A16 (C-0005): a shot into a skater within 0.6 m of its stick keeps 70 % of its speed |
| `shot.one_timer.window_frames` | 10 | lane A39 (R-0029 point 4): a shot inside this many frames of a received pass is a one-timer (full charge without the hold, `aim_cone_deg` + `cone_bonus_deg`, `shot` event kind `one_timer`); 0 = off |
| `shot.one_timer.cone_bonus_deg` | 8 | lane A39 |
| `shot.lift.tap_m` | 0.05 | lane A52 (R-0035): the height a tap arrives at over the goal line |
| `shot.lift.half_m` | 0.45 | lane A52: the height of a shot held `half_frames` |
| `shot.lift.full_m` | 0.6 | lane A52 (0.95), Director 01:40: the height of a full charge: just under the 0.61 bar (top shelf); at 0.95 every hold past 8 frames flew over the low net; a cartridge may still choose 0.95 for a sailing full charge |
| `shot.lift.half_frames` | 8 | lane A52: frames of hold that reach `half_m`; linear tap -> half -> full |
| `pass.saucer.half_frames` | 8 | lane A77 (R-0036): pass (F / X) held this many frames with the puck in reach is a saucer; a shorter hold or the tap is the flat pass; 0 = off |
| `pass.saucer.lift_m` | 0.35 | lane A77: the saucer's apex height; it clears a blade (0.2 m) in the lane and lands three quarters of the way to the receiver; 0 = off |
| `rules.goal_pause_frames` | 120 | |
| `rules.offside` | false | only false is accepted |
| `rules.penalties` | true | R-0013: fouls (from behind, or on a skater without the puck) = 300 frames in the box + a free faceoff at the fouler's end; `hockey.raw` sets false |
| `rules.penalty_frames` | 300 | lane A72 (ask A8): frames a hard foul sits in the box; read only when `penalties` is true |
| `ai.difficulty` | `pro` | lane A3; the numbers below (and `react_frames`, `body_check`), when absent, come from the level's profile (`docs/experiments/BALANCING.md`), not from this table |
| `ai.yield_margin_m` | 2 (pro) | |
| `ai.yield_radius_m` | 15 | |
| `ai.hold_radius_m` | 0 (pro; 4.5 before the difficulty sweep) | |
| `ai.speed_scale` | 1 | |
| `ai.red_skill` | 1 | |
| `ai.react_frames` | the level's profile (0 at rookie, pro and all_star) | lane A72 (ask A4): the opposing bots' steering delay in frames; the back reacts at once |
| `ai.sprint` | 0 | lane A72 (ask A2): the share of the time a bot sprints on a chase from more than 6 m (both sides' bots, same stamina bar as the human); 0 = bots never sprint |
| `ai.body_check` | the level's rule (rookie never; pro and all_star from 5 m/s of closing) | lane A72 (ask A5): when written, the share of the chances an opposing bot checks, at `skater.check_speed_min` instead of 5 m/s; 0 never |
| `pickups` | none | |
| `pickups[].max_on_ice` | 1 | lane A14 (C-0005) |
| `pickups[].zone` | absent = anywhere on the ice | |
| `abilities` | none (nobody holds an ability; F does nothing) | lane A84 (R-0041 point 3) |
| `abilities[].charges` | unlimited for `trigger` mode, 1 for `trigger` pickup | lane A84 |
| `abilities[].cooldown_frames` | 1200 (20 s) | lane A84 |
| `abilities[].duration_frames` | 360 shield (6 s), 120 slow_puck (2 s); unused by shotgun and hip_check | lane A84 |
| `abilities[].magnitude` | 1.5 shotgun (shot speed x), 1.5 hip_check (knockdown frames x), 0.5 slow_puck (the puck's speed share); unused by shield | lane A84 |
| `effects` | none (the five built-ins are always available to pickups) | lane A14; `Game/Content/Designer/builtin-effects.json` |
| `effects[].kind` | `custom` | the visual family; built-ins carry their own |
| `effects[].stack` | `refresh` | |
| `effects[].icon` | by kind: bolt (speed), crosshair (precision), weight (heavy_puck), circle (big_puck), swirl (reverse_other), star (custom) | |
| `effects[].stats[].multiply` / `add` | 1 / 0 | |
| `look.trim_colour_hex` | `#38BDF8` | |
| `look.fog_density` | 0 | |
| `meta` (whole object) | absent | lane A81 (A67 ask A11): the menu's words for a Director's choice (`pitch`, up to three `facts`; the card's name is the envelope `title`); informational, never read by the sim; a mode file without it gets no card |
| `look.trim_emissive` | by tier: 30 (`l1_day`, `l2_night`, `l3_neon`, `p4_broadcast`), 12 (`p3_clay`) | B-to-A-0018 point 5, B-to-A-0025 (lane A31); the beacon stays at 40 |
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
| `commentary.enabled` | true | lane A32 (B-to-A-0026); read by `UJamCommentarySystem` from the cartridge's raw JSON, console `jam.Commentary -1|0|1` overrides |
| `commentary.intensity` | `normal` | `quiet` plays priority 3 and up only and doubles the situation cooldowns, `loud` halves them; the 2.5 s gate between lines never changes |

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
| `camera.boards_margin_m` | 0.8 | lane A40: the camera never crosses the boards' inner face minus this, corner arcs included |
| `camera.end_height_m` | 3 | lane A49 (was 5, lane A40): the lift when the boards take the arm (proportional to the arm lost), looking along the rink at an end board |
| `camera.far_net_bias` | 0.2 | lane A49 (was 0.5, lane A40): the yaw lead toward the far net when heading for it, fading in from 8 to 20 m |

Wide option (not a default): 8 / 4 / 6 / 6 frames / fov 85. Pitch is derived, never stored: `-atan(height_m / (distance_m + look_ahead_m))`.
In `arena` mode the distance, height, look-ahead and lag keys are ignored.

## Derived by the engine, not in the cartridge

- Goal line at `length_m / 2 - 4` m from the centre on each end (the Godot rink: 60 m long, nets at x = +-26).
- Pickup `magnitude` absent: 1.5 for `speed`, 2 for `precision` (cone multiplier), 2 for `heavy_puck` (drag multiplier, max speed divided by it), 1.5 for `big_puck` (radius multiplier); `reverse_other` ignores it. For a pickup naming a custom effect, `magnitude` scales the effect's deviation from neutral (`multiply' = 1 + (multiply - 1) * magnitude`, `add' = add * magnitude`, then clamped to 0.25..4 and the stat range); absent = 1 = as written.
- Effect modifiers compose as `base * product(multiply) + sum(add)` per stat, clamped to the stat's schema range; the same effect on the same target while active: `stack` decides (refresh restarts the timer, extend adds the duration up to 7200 frames, ignore keeps the running one).
- A `values` entry overrides the game field a tunable is named after (README.md, "Tunables"); an absent entry leaves the game field alone.
