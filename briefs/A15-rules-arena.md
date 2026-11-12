# Lane A15: rule hooks, rule variants, arena zones and obstacles (device A, from H7)

You are lane A15 of the Third Jam (team Hokej). Read first: `contracts/depth.md` (C-0005: you own `JamCore/Rules/**`, the schema's `game.rules.hooks`, `game.rules.variants`, `game.rink.zones`,
`game.rink.obstacles` extension, and the zone/obstacle rendering in `RinkActor.cpp`; the sim's rule-hook seam `JamRuleHooks.h` is landed by lane A16 within 30 minutes: build against the contract
text until the header exists), `GAME.md`, `docs/THEME.md` (Director's Call), `docs/HOCKEY.md` (3 M3b, 4), `schema/game.schema.json`, `schema/validate.mjs`, `prompts/designer.system.md`,
`Game/Source/JamGame/Private/RinkActor.cpp` (A1: obstacles `box|pillar` exist; stencils and materials per R-0002), `Game/Source/JamCore/Public/JamEventBus.h`, `Game/Source/JamDesigner/Public/*`
(Director's Call path for `SetParam`), `Game/Source/JamGame/Public/HUDWidget.h` (the scoreboard line that shows `announce`).

You own what C-0005 section 1 says. Run no git command; headless first; editor on MCP port **8003** for the PIE proof; link windows; keep the tree compiling. Never kill a process you did not start.

## Deliverables
1. **Schema:** `game.rules.hooks[]`, `game.rules.variants`, `game.rink.zones[]`, obstacles `bumper` and `goal_shield` exactly as C-0005 section 4, with limits (max 8 hooks, 8 zones, zone inside
   the rink, shrink/grow bounded by the rink range), the loader expanding `variants` sugar into hooks, README/DEFAULTS notes, tests. Designer: hooks, variants, zones, obstacles editable within
   limits; three worked examples in the system prompt ("last goal counts double", "the rink shrinks by 10 % every goal", "a slow-ice circle at centre"); the Designer may not create a hook that
   ends the match before frame 600.
2. **`UJamRuleSystem`** (JamCore/Rules, the only `IJamRuleHook`): evaluates hooks per trigger in id order, `every_frames` cadence, score conditions, team selectors (`scoring|conceding|any`),
   emits actions; `Announce` text goes through the bus (`announce`) and the HUD scoreboard line (A8's line format `DIRECTOR'S CALL: ...` exists: add a sibling `RULE: ...` style, say where).
3. **Zones** in the sim: per-frame membership for skaters and the puck (`zone_enter/exit` events), `slow_ice`/`fast_ice` as modifier requests to the effect system (A14's seam) or as direct
   friction/speed factors in the sim via a small `IJamZoneSource` you define in your files and A16 consumes (propose it; until merged, apply through the modifier request path);
   `bumper` obstacles restitution 1.6; `no_shoot` zones block shots with a bark-able event; `goal_shield` obstacle in front of a net; zone rendering in `RinkActor` as flat decal-like strips with
   the contract's stencil 2 and a `MI_Ice_Night` parameter if B provides one (ask via your report), else a tinted placeholder.
4. **Tests:** `HooksFire` (goal -> AddScore double on last goal), `ShrinkAtFaceoff` (rink rebuilds only at faceoff, geometry size changes, hash deterministic), `ZonesMembership`,
   `BumperRestitution`, `DesignerRulesRoundTrip` (ten mock sentences -> valid hooks/zones; a match-ending-before-600 hook rejected), `Jam.Bot` still green with `hockey.raw` and a new preset
   `hockey.chaos` you add (sudden death, shrink per goal, two bumpers, one slow circle).
5. PIE proof of `hockey.chaos`: screenshots of the shrinking rink and a bumper hit.

Report: what shipped, the zone seam proposal, the HUD seam, every error, the brain note for A3 (bots and zones: avoid `slow_ice` when carrying). Time box 3 hours; report at 90 minutes.
