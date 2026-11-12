# Lane A14: composable boosts and effects (device A, from H7)

You are lane A14 of the Third Jam (team Hokej). Read first: `contracts/depth.md` (C-0005: you own `JamCore/Effects/**` and the schema's `game.effects` and `game.pickups[]` extension; the
sim's modifier seam `JamModifiers.h` is landed by lane A16 within 30 minutes: build against the contract text until the header exists), `GAME.md`, `docs/THEME.md`, `docs/HOCKEY.md` (3 M3, 4),
`schema/game.schema.json`, `schema/README.md`, `schema/validate.mjs` (`DESIGNER_GAME_EDITABLE`), `prompts/designer.system.md`, `Game/Source/JamDesigner/Tools/gen-cut-library.mjs` (how cuts are
generated), `Game/Source/JamCore/Public/JamEventBus.h`, `contracts/events.md`, `Game/Source/JamGame/Public/HUDWidget.h` (A5) and `BarkSystem.h` (A12) for where effects show.

You own what C-0005 section 1 says. Run no git command; headless first; editor on MCP port **8002** for the PIE proof; link windows at :00 and :30; keep the tree compiling. Never kill a process you did not start.

## Deliverables
1. **Schema:** `game.effects[]` and the extended `game.pickups[]` exactly as C-0005 section 4, with limits (max 12 effects, stats multiply 0.25-4, add within the stat's range, duration 60-7200
   frames), defaults, `README.md`/`DEFAULTS.md` notes, tests; the built-in pickup kinds (`speed`, `precision`, `heavy_puck`, `big_puck`, `reverse_other`) become shipped effect definitions in a
   `builtin-effects.json` the loader merges (so the Designer sees them as examples). Designer: `effects` and `pickups` editable within limits; three worked examples in the system prompt
   ("a boost that slows the other team when I score", "sticky puck for 5 s on pickup", "turbo lane") and the rule that an effect must reference known stats only.
2. **`UJamEffectSystem`** (JamCore/Effects, the only `IJamModifierSource`): active modifiers with stack policy, triggers from the bus (`pickup_take`, `goal`, `check`, `period`, `zone_enter/exit`,
   `directors_call`) and from the rule system's `ApplyModifier` requests, deterministic ordering (by frame then id), `effect_start`/`effect_end` events, HUD feed of active effects per skater
   (icon name + frames left) through a small struct lane A5's HUD can read (name it; A5 is finished: add the HUD strip yourself in `HUDWidget` only if trivial, else report the seam).
3. **Pickup spawner** (if the sim has none yet: check with A16's files; the spawner lives in Effects): spawns from the table on the ice with the seeded stream, optional zone restriction,
   `max_on_ice`, a `pickup_spawn` event, a toy pickup actor (glowing capsule, team-neutral colour per kind, stencil 0) that the toy look reads; `pickup_take` on contact by any skater.
4. **Tests:** `EffectsResolveStats` (multiply and add compose, clamped), `EffectsStackPolicies`, `PickupSpawnsDeterministic` (same seed same frames), `DesignerEffectsRoundTrip` (ten generated sentences
   through the mock produce valid effects; one bad stat rejected with a pointer), the `Jam.Bot` suite still green with `hockey.boost`.
5. PIE proof: `hockey.boost` with three built-ins and one custom effect from a sentence through the CLI transport; screenshots.

Report: what shipped, the stat list as implemented, the HUD seam, every error, the brain note for A3 (bots chase pickups? propose a rule: only `all_star` detours for pickups within 8 m). Time box 3 hours; report at 90 minutes.
