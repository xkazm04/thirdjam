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
5. **Enums, exact strings, case-sensitive:** `rules.mode` first_to|timer; `teams[].skaters[].role` player|chaser|support|back; `pickups[].kind` a built-in (speed|precision|heavy_puck|big_puck|reverse_other) or the id of an effect in `game.effects` (cross-field check); `effects[].target`, `trigger`, `stack`, `icon`, `kind` and `stats[].stat` are the enums in `game.schema.json` (`game_ranges` lists them); `commentary.intensity` quiet|normal|loud; `look.tier` l1_day|l2_night|l3_neon|p3_clay|p4_broadcast (B-to-A-0018, B-to-A-0025); `look.trim_emissive` 0..100 (absent = the tier default: 30, clay 12); `camera.mode` third_person|arena; `skater.visual` toy|humanoid_b1|generated_b2|toy_skeletal (absent = toy_skeletal); `rink.obstacles[].kind` box|pillar. `rules.offside` accepts only `false`; `rules.penalties` is a real switch since R-0013 (absent = true).
6. **Ranges** (inclusive). The table below is the source; `limits.json` `game_ranges` repeats it as `min..max` strings (enums as `a|b|c`) and a test keeps it equal to the schema. The loader should read `game_ranges` rather than hard-code numbers.
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
| `skater.pass_speed` | 5..30 | 14 (C-0005); below a shot, above skating, so a pass arrives before the lane closes |
| `skater.pass_lead_frames` | 0..60 | 20 (C-0005); the receiver's position a third of a second ahead |
| `stick.reach_m` | 0.8..3 | 1.2 (R-0022 point 3; was 1.6 in R-0013): rest 1.0 m, full aim 1.6 m from the skater centre, which both of B's figures can pose |
| `stick.poke_window_frames` | 2..30 | 8; the timing window of a clean poke |
| `stick.lift_range_m` | 0.5..2.5 | 1.0; contact range of a stick lift beyond the two bodies |
| `stick.stumble_frames` | 0..90 | 20; `hockey.raw` sets 0 |
| `stick.mouse_sensitivity` | 0.1..5 | 1; 100 px = one reach |
| `stick.foul_grace_frames` | 0..120 | 12 (R-0029); a check right after a strip is not a check on a player without the puck |
| `puck.radius_m` | 0.06..0.4 | 0.12 measured; 0.35 made the player lose 1:3, so the cap is just above it for the big_puck look |
| `puck.max_speed` | 10..60 | 30 measured, 4 sub-steps per frame hold to 60 m/s without tunnelling a post (1 m per frame) |
| `puck.drag` | 0..2 | per second; 0 measured |
| `puck.carry_assist` | 0..1 | 0.6 (C-0005); the pull of a carried puck towards the stick while turning under 90 deg/s |
| `puck.count` | 1..3 | 1; more pucks is a Designer twist, three keeps the ice readable |
| `shot.reach_m` | 0.5..4 | 1.5 measured |
| `shot.speed` | 5..40 | 18 measured; stays under `puck.max_speed` ranges |
| `shot.cooldown_frames` | 6..120 | 24 measured; 6 is a machine gun, 120 is 2 s |
| `shot.aim_cone_deg` | 1..45 | half-angle; 8 in presets; the "must face within 60 deg" rule is engine-fixed |
| `shot.charge_frames_max` | 0..120 | 30 (C-0005); 0 = no charge; 120 is a two-second wind-up |
| `shot.charge_speed_bonus` | 0..20 | 8 (C-0005); capped by `puck.max_speed` |
| `rules.score_to_win` | 1..9 | one digit on the HUD |
| `rules.timer_frames` | 600..18000 | 10 s to 5 min at 60 Hz |
| `rules.goal_pause_frames` | 30..600 | 120 measured |
| `ai.yield_margin_m` | 0..6 | 2 measured |
| `ai.yield_radius_m` | 0..40 | 15 measured |
| `ai.hold_radius_m` | 0..15 | 4.5 measured |
| `ai.speed_scale` | 0.3..1.5 | below 0.3 bots stand still, above 1.5 they cannot be beaten |
| `ai.red_skill` | 0..1 | fraction (wire name kept from HOCKEY.md; it drives the orange side) |
| `pickups[]` | 0..8 items, `spawn_every_frames` 120..7200, `duration_frames` 1..3600, `magnitude` 0.1..4, `max_on_ice` 1..4, `zone` a `rink.zones` id | 2 s to 2 min between spawns; effect up to 1 min; more than 4 of one kind on the ice is clutter (C-0005, A14) |
| `effects[]` | 0..12 items, `duration_frames` 60..7200, 1..4 `stats`, `multiply` 0.25..4, `add` within the stat's range width (per stat, `checkGame`) | C-0005 section 4: 12 is the HUD strip's limit; a quarter to four times a stat is the widest change that still reads as the same game; 1 s to 2 min |
| `look.fog_density` | 0..0.1 | engine exponential fog density; 0.02 neon; L2 ships with none |
| `camera.distance_m` | 2..20 | 6 default, 8 wide |
| `camera.height_m` | 1..15 | 3.5 default |
| `camera.look_ahead_m` | 0..20 | 5 default |
| `camera.lag_frames` | 0..30 | 4 default, 6 wide |
| `camera.fov_deg` | 40..110 | horizontal; 70 default, 85 wide |

## Rule hooks, variants, zones (lane A15, C-0005)

`game.rules.hooks[]` (max 8): `{ id, when, every_frames?, after_frame?, team?, score?, then[] }`. `when` is one of `on_goal | on_period_start | on_period_end |
on_check | on_knockdown | on_pickup_take | on_faceoff | on_frame | on_score_reached`; `on_frame` needs `every_frames` (60..18000) and nothing else may carry it;
`on_score_reached` needs `score.at_least`. `team` is `scoring | conceding | any` relative to the event (the scorer, the checker, the taker; default scoring).
`score` is `{ at_least?, at_most?, lead_at_least? }` read on the hook's team after the goal has been counted (`any`: either team). `then` holds 1..4 actions
`{ do, team?, value?, param?, kind?, frames?, text? }` with `do` one of `add_score | set_score_multiplier | end_period | end_match | shrink_rink | grow_rink | set_param |
spawn_pickup | swap_nets | announce | apply_modifier`: add_score needs a whole `value`; set_score_multiplier a `value` 1..5; shrink_rink / grow_rink a `value` that is the
fraction 0.05..0.5 (0.1 = 10 %; the sim scales by 1 - f / 1 + f and clamps to the rink range); set_param needs `param` (a tunable key) and `value`; spawn_pickup `kind`;
apply_modifier `kind` and `frames`; announce `text` (1..60 chars). Hook ids are unique; the engine evaluates hooks in id order.
`game.rules.variants` is sugar `{ sudden_death?, last_goal_double?, mercy_goals? (2..9), period_count? (1..3), shrink_per_goal? (0.02..0.3) }` the loader expands into hooks
(DEFAULTS.md says how). `game.rink.zones[]` (max 8): `{ id, shape: rect | circle, x, z, w?, d?, r?, kind, strength?, visible? }`; rect needs `w` and `d`, circle needs `r`
(and not the other way round); the centre must be inside the rink like a spawn; `kind` is `slow_ice | fast_ice | bumper | no_shoot | boost_only`. `rink.obstacles[].kind`
gains `bumper` (round, restitution 1.6) and `goal_shield` (a wall `size` m wide, 0.3 m deep). **Designer guard** (`validateDesignerOutput` only): a hook with an
`end_match` action must not be able to fire before frame 600: give it `after_frame >= 600`, an `every_frames >= 600`, `when: on_period_end`, or a `score` needing a second goal.
The C++ mirrors are `JamValidate::CheckHook` and the zone block of `CheckGame` in `JamCartridge.cpp`; the Designer guard's mirror is in `JamDesignerPrompt.cpp`.

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

## Effects and pickups (C-0005 section 4, lane A14)

`game.effects[]` is a list of timed stat modifiers: `{ id, kind?, target, duration_frames, stats: [ { stat, multiply?, add? } ], trigger, zone?, stack?, icon? }`.
The sim resolves `effective(stat) = base * product(multiply) + sum(add)` over the active modifiers whose target covers the skater, the puck or the rink, clamped to the stat's range
(`EFFECT_STAT_FIELD` in `validate.mjs` maps each `stat` to the game field whose range clamps it). Cross-field checks in `checkGame`: ids unique and never a built-in kind; `add`
within the width of the stat's range; `zone` only with `zone_enter`/`zone_exit` and only a `rink.zones` id; a pickup `kind` is a built-in or a declared effect id; a pickup `zone`
is a `rink.zones` id. The five built-in kinds live in `Game/Content/Designer/builtin-effects.json` in the same shape (`UJamEffectSystem` merges them under every cartridge).
`trigger` names: `pickup` (a pickup naming the id is taken; the pickup's `duration_frames` and `magnitude` win), `goal_for` / `goal_against` (seen from the scoring team),
`check_landed` / `check_received`, `period_start`, `zone_enter` / `zone_exit`, `director` (a Director's Call or a Designer cut applied live). `zone` on an effect is lane A14's proposal, not yet in C-0005.
