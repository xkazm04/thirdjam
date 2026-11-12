# Lane A4: the hockey cartridge schema, validator, presets and tests (H4, device A)

You are lane A4 on device A of the Third Jam (team Hokej). The jam started 2026-11-12 06:00 local; it is about 10:00. Repo: `C:/Users/kazda/kiro/thirdjam`. Read first:
`docs/HOCKEY.md` (2, 3, 4, 7), `schema/cartridge.schema.json`, `schema/game.schema.json` (the placeholder you replace), `schema/limits.json`, `schema/validate.mjs`,
`schema/fixtures/`, `prompts/README.md`, `tools/designer-eval/README.md` if present, `docs/experiments/CAMERA-120M.md` (camera defaults), `tools/skills/skating-physics.md`.

You own: `schema/**`, `prompts/**`, `tools/designer-eval/**`, `Game/Content/Cartridges/**`. Run no git command. Node 24, zero dependencies.

## Laws
Unknown keys are errors (`additionalProperties: false` everywhere); absent value = omit the key, never null, never 0; lists not singletons; durations in `_frames` as integers;
`seed` integer; wire names exactly as HOCKEY.md section 4 writes them. Your measurement outranks this brief.

## Deliverables
1. `schema/game.schema.json`: the hockey `game` object from HOCKEY.md section 4 **plus** `game.camera.look_ahead_m` and `game.skater.glide_drag`, `game.skater.brake_decel`
   (both already listed there). Enums exactly: `rules.mode` `first_to | timer`; `teams[].skaters[].role` `player | chaser | support | back`; `pickups[].kind`
   `speed | precision | heavy_puck | big_puck | reverse_other`; `look.tier` `l1_day | l2_night | l3_neon`; `camera.mode` `third_person | arena`. Ranges in `schema/limits.json`
   (rink 40-160 x 20-80 m, 2-5 skaters per side, score_to_win 1-9, timer 600-18000 frames, speeds, etc.; justify each).
2. `schema/validate.mjs`: extend the engine checks: exactly one `role: "player"` across all teams; two teams; spawns inside the rink; pickup `duration_frames` > 0; `camera` defaults applied
   when absent (document that the validator only checks, the engine applies defaults, and list the defaults in `schema/DEFAULTS.md`: camera 6 / 3.5 / 5 / 70 / 4 frames, third_person, radar true).
3. Presets in `Game/Content/Cartridges/` (JSON, validated): `hockey.regular.cartridge.json` (M1: 120 x 60 m rink per the operator's decision, skater speed 8, accel 12, turn 5,
   grip 3, mass 80, glide_drag 0.147, brake_decel 6, puck r 0.12 max 30 drag 0, shot reach 1.5 speed 18 cooldown 24 aim_cone 8, rules first_to 3, goal_pause 120, no checks,
   no pickups, look l2_night, camera third_person defaults, teams blue `#1D4ED8` and orange `#E8651A` with roles player/chaser/back vs chaser/support/back and spawns),
   `hockey.raw.cartridge.json` (M2: check_enabled true, check_speed_min 5, knockdown_frames 90, offside false, penalties false), `hockey.boost.cartridge.json` (M3: three pickups),
   `hockey.arena.small.cartridge.json` (90 x 45, speed 11: the E4 alternative for the sweep), `hockey.arena.wide.cartridge.json` (140 x 70). Theme/tunables sections filled sensibly.
4. Tests (`node --test`): every preset validates; each enum rejects a wrong value with a pointer; one-player rule; spawns outside the rink fail. Update `prompts/designer.system.md`
   only where the game object changes what the Designer may touch (it may change `values`, `theme`, `pickups`, `rules.mode/score_to_win/timer_frames`, `look.tier`, `rink` numbers inside limits;
   never `teams[].skaters[].role`, `camera.mode`).
5. `schema/README.md`: how the C++ loader (lane A1) must mirror the checks (unknown keys, ranges, enums) so a cartridge rejected by Node is rejected by the game identically.

## Report
Commands run with results, the list of files, any HOCKEY.md field you had to change and why. Time box 45 minutes.
