# Cartridge schema (tj.cartridge/1) and what the C++ loader must mirror

Files: `cartridge.schema.json` (envelope), `game.schema.json` (the hockey `game` object, HOCKEY.md section 4), `limits.json` (counts and `game_ranges`),
`validate.mjs` (schema walker plus engine checks, the reference implementation), `DEFAULTS.md` (what the engine fills in), `fixtures/`, `*.test.mjs`.
Run: `node schema/validate.mjs file.json` (exit 0 valid, 1 invalid, 2 could not run), `node --test schema/*.test.mjs`.

## The rule for lane A1

A cartridge Node rejects, the game rejects: same verdict, same JSON pointer in the message (wording may differ, the pointer and the kind of failure must not).
Parse with a strict JSON parser (duplicate keys: reject). Then, in this order:

1. **Unknown keys are errors** in every object of both schemas (`additionalProperties: false` everywhere except `values`, which is keyed by tunable key, and `params`, which is keyed by snake_case names). Message: `unknown key "k" at <pointer>`.
2. **Absent = omit.** A `null` anywhere is an error. A key that is absent takes the default in `DEFAULTS.md`. Never treat 0 as absent.
3. **Types.** `integer` means a JSON number with no fraction; every `*_frames` is an integer. Numbers must be finite.
4. **Required keys** per object (`required` arrays in the schema files). Hockey: `game` needs `rink`, `skater`, `rules`, `teams`, `look`; `rink` needs `length_m`, `width_m`; `skater` needs `speed`; `rules` needs `mode`; `look` needs `tier`; each team needs `id`, `colour_hex`, `skaters`; each skater `role`, `spawn`; each pickup `kind`, `spawn_every_frames`, `duration_frames`; each obstacle `kind`, `x`, `z`, `size`.
5. **Enums, exact strings, case-sensitive:** `rules.mode` first_to|timer; `teams[].skaters[].role` player|chaser|support|back; `pickups[].kind` speed|precision|heavy_puck|big_puck|reverse_other; `look.tier` l1_day|l2_night|l3_neon; `camera.mode` third_person|arena; `rink.obstacles[].kind` box|pillar. `rules.offside` and `rules.penalties` accept only `false`.
6. **Ranges** (inclusive). The table below is the source; `limits.json` `game_ranges` repeats it as `min..max` strings and a test keeps it equal to the schema. The loader should read `game_ranges` rather than hard-code numbers.
7. **Lists:** `teams` exactly 2; `skaters` 2 to 5 per team; `obstacles` at most 12; `pickups` at most 8. Counts from `limits.json` for the generic lists (rules, entities, ai, tunables, palette, mood words), 24000 characters of compact JSON at most.
8. **Hex colours:** `^#[0-9A-Fa-f]{6}$`. Ids: kebab-case (`^[a-z][a-z0-9]*(-[a-z0-9]+)*$`, 48 chars at most). Tunable keys and param names: snake_case, no `_seconds`/`_ms`/`_minutes`... names.
9. **Cross-field checks** (all in `checkGame` in `validate.mjs`):
   - exactly one skater across both teams has `role: "player"`;
   - every `spawn` and obstacle centre is inside the rink: |x| <= length/2 - 0.5, |z| <= width/2 - 0.5, and inside the quarter circle in each corner (radius `corner_radius_m` - 0.5, default corner 5 when absent); two spawns closer than 1 m are an error;
   - `mode first_to` needs `score_to_win`, `mode timer` needs `timer_frames`;
   - every pickup `duration_frames` > 0;
   - `corner_radius_m` <= half of the shorter side; `goal_width_m` < `width_m`; `check_speed_min` <= `skater.speed`;
   - the two teams have different ids and different colours;
   - `values` keys must be declared tunables and sit inside the tunable's type, range and options.
10. **The validator does not apply defaults.** It checks only what is written. The loader applies `DEFAULTS.md` after validation (camera: 6 / 3.5 / 5 / 70 / 4 frames, third_person, radar true).

Cartridge errors are sentences that start `at <JSON pointer>:`, because the Designer repair loop hands them back to a model. Keep the pointer first.

## Ranges and why

| Key | Range | Reason |
|---|---|---|
| `rink.length_m` | 40..160 | below 40 m six skaters and two nets do not fit; 160 m is 20 s end to end at 8 m/s and the radar and far-net marker stop being readable. Presets 90, 120, 140 |
| `rink.width_m` | 20..80 | 20 m is the Godot rink's half; 80 m keeps the 2:1 look at 160 |
| `rink.corner_radius_m` | 0..15 | 0 is square; 15 is a near-oval; check also caps it at half the width |
| `rink.board_restitution` | 0..1 | a coefficient |
| `rink.ice_friction` | 0..0.1 | 0.015 real ice; 0 never slows; 0.1 is carpet |
| `rink.goal_width_m` | 2..10 | Godot 4.0; 2 is hard to score, 10 is nearly the whole end of a 20 m rink |
| `rink.obstacles[]` | 0..12 items, size 0.5..10, x/z within +-80/+-40 | clutter beyond 12 breaks the bot path planner and the view |
| `skater.speed` | 3..16 | 8 is the measured Godot value; 3 is walking pace; above 16 a skater crosses the rink faster than the camera lag recovers |
| `skater.accel` | 4..40 | 12 measured; 4 is a heavy slide, 40 is instant |
| `skater.turn_rate` | 1..12 | 5 measured; 12 rad/s is a spin |
| `skater.lateral_grip` | 0.5..12 | 3 measured; low drifts like ice, high is rails |
| `skater.mass` | 40..160 | 80 measured; only matters for checks |
| `skater.glide_drag` | 0..1 | 0.147 measured; 1 m/s^2 stops a glide within 8 s |
| `skater.brake_decel` | 1..30 | 6 measured; must exceed glide_drag in play |
| `skater.check_speed_min` | 1..12 | M2 uses 5; 12 is above any default top speed |
| `skater.knockdown_frames` | 15..300 | 90 (1.5 s); 15 is a stumble, 300 (5 s) is a penalty |
| `puck.radius_m` | 0.06..0.4 | 0.12 measured; 0.35 made the player lose 1:3, so the cap is just above it for the big_puck look |
| `puck.max_speed` | 10..60 | 30 measured, 4 sub-steps per frame hold to 60 m/s without tunnelling a post (1 m per frame) |
| `puck.drag` | 0..2 | per second; 0 measured |
| `shot.reach_m` | 0.5..4 | 1.5 measured |
| `shot.speed` | 5..40 | 18 measured; stays under `puck.max_speed` ranges |
| `shot.cooldown_frames` | 6..120 | 24 measured; 6 is a machine gun, 120 is 2 s |
| `shot.aim_cone_deg` | 1..45 | half-angle; 8 in presets; the "must face within 60 deg" rule is engine-fixed |
| `rules.score_to_win` | 1..9 | one digit on the HUD |
| `rules.timer_frames` | 600..18000 | 10 s to 5 min at 60 Hz |
| `rules.goal_pause_frames` | 30..600 | 120 measured |
| `ai.yield_margin_m` | 0..6 | 2 measured |
| `ai.yield_radius_m` | 0..40 | 15 measured |
| `ai.hold_radius_m` | 0..15 | 4.5 measured |
| `ai.speed_scale` | 0.3..1.5 | below 0.3 bots stand still, above 1.5 they cannot be beaten |
| `ai.red_skill` | 0..1 | fraction (wire name kept from HOCKEY.md; it drives the orange side) |
| `pickups[]` | 0..8 items, `spawn_every_frames` 120..7200, `duration_frames` 1..3600, `magnitude` 0.1..4 | 2 s to 2 min between spawns; effect up to 1 min |
| `look.fog_density` | 0..0.1 | engine exponential fog density; 0.02 neon; L2 ships with none |
| `camera.distance_m` | 2..20 | 6 default, 8 wide |
| `camera.height_m` | 1..15 | 3.5 default |
| `camera.look_ahead_m` | 0..20 | 5 default |
| `camera.lag_frames` | 0..30 | 4 default, 6 wide |
| `camera.fov_deg` | 40..110 | horizontal; 70 default, 85 wide |

## Tunables (mine to define: HOCKEY.md did not)

`tunables` are the Designer's main knob (`values`). For hockey a tunable named after a game field dials that field:
`skater_speed` -> `game.skater.speed`, `skater_turn_rate`, `shot_speed`, `puck_max_speed`, `ice_friction` -> `game.rink.ice_friction`, `board_restitution`,
`ai_speed_scale` -> `game.ai.speed_scale`, `ai_red_skill`, `goal_pause_frames` -> `game.rules.goal_pause_frames`.
Effective value = `values[key]` when present, else the game field (the preset keeps tunable default and game field equal; a test checks it).
The loader applies overrides after validation, before defaults for absent game keys. Unknown tunable keys in `values` are already rejected by the validator.

## Designer editing

`DESIGNER_GAME_EDITABLE` in `validate.mjs` lists the `game` paths a Designer reply may change; any other difference in `game`, notably any skater `role`, any `spawn`,
and `camera.mode`, is rejected with the pointer of the changed value. The game does not need this check at load time (a human-authored cartridge may set anything valid);
it applies to `validateDesignerOutput` and the C++ `JamDesigner` mirror of it.

## Changes to HOCKEY.md fields

None to wire names. Additions, all optional: `rink.obstacles[].kind` is `box | pillar` (HOCKEY.md left it open); `rules.offside`/`penalties` accept only `false`.
Required-ness was not specified there: the lists above are mine. Ranges are mine except where "measured" is said.
