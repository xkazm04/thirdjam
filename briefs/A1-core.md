# Lane A1: core simulation, rink, puck, shooting, goals, rules, cartridge loader (device A, from H5)

You are lane A1 of the Third Jam (team Hokej). Repo `C:/Users/kazda/kiro/thirdjam`; the project skeleton from lane A0 exists in `Game/` (read `briefs/A0-skeleton.md` and its
report in `status/A.md`), the seam stubs compile, the cartridge schema and presets from lane A4 are in `schema/` and `Game/Content/Cartridges/`. Read first: `GAME.md`,
`docs/HOCKEY.md` (2, 3 M0-M1, 4, 7), `tools/skills/skating-physics.md` (the formulas and the determinism test), `tools/mcp/PHYSICS.md`, `schema/README.md`, `schema/DEFAULTS.md`,
`contracts/events.md`, `contracts/skater-visual.md`, `tools/mcp/README.md`, `tools/run/*.ps1`.

You own: `Game/Source/JamCore/**` (except the seam files A0 made, which you extend but do not rename), `Game/Content/Maps/Rink.umap`, `Game/Source/JamGame/Rink*` and
`Game/Source/JamGame/HokejGameMode*`. Run no git command. One heavy job at a time; your editor on MCP port **8001** (`tools/run/editor.ps1 -McpPort 8001`), own worktree not
needed (you are the only writer of these paths). Never kill a process you did not start. Free-space floor 40 GB.

## Laws
Fixed-step simulation at 60 Hz, our own integrator (`UJamSim`), Chaos only for cosmetics; randomness only from `FJamRandom(seed)`; unknown cartridge keys are errors; absent value =
omit; durations in frames; every state change that gameplay cares about goes through `UJamEventBus` with the contract's event names. Your measurement outranks this brief.

## Deliverables (M0 by H6, M1 core by H12)
1. **Cartridge loader** (`UJamCartridge`): parses JSON with the same rules as `schema/validate.mjs` (closed objects, enums, ranges from `schema/limits.json`, exactly one player,
   two teams, spawns in rink); a rejected cartridge names the pointer; a `CartridgeRoundTrip` automation test loads every file in `Game/Content/Cartridges/`.
2. **Rink builder** (`ARinkActor`): procedural rounded-corner rink from `game.rink` (length, width, corner radius, board height 1.2 m, glass 0.9 m above), two nets
   (`goal_width_m`, 1.5 high, 1.5 deep) at x = +-(length/2 - 4), goal trigger volumes behind the lines, centre and faceoff markings as decals, ice plane; materials
   are placeholders with the contract's parameter names so B's masters drop in (`M_Ice`, `M_Boards`, `M_Net`, `M_Trim` in `Game/Content/Placeholders/`).
3. **Simulation** (`UJamSim`, ticked from the game mode at a fixed 60 Hz accumulator; `Use Fixed Frame Rate` is on): skater state (pos, vel, facing, lateral grip, glide drag, brake),
   puck state (pos, vel, 4 sub-steps, board restitution, ice friction, 30 m/s cap, post collision), shot (reach, exit speed, cooldown frames, aim cone), goal detection,
   faceoff reset, score, `first_to` / `timer` rules, `goal_pause_frames`; `state_hash()` of all integer-quantised state each frame; a `FJamInput` per skater (axis x/y, shoot, check).
4. **Pawns**: `AJamSkater` (owns a sim index, pushes `FJamInput` from Enhanced Input for the human, and calls `IJamSkaterVisual` on its visual component each frame); `AJamPuck`
   (render-only follower of the sim). The visual component for M0 is a capsule + sphere + cylinder stick placeholder that implements the interface (lane A5 replaces it with the
   toy figure).
5. **Game mode** `AHokejGameMode`: loads the cartridge named by a console variable / launch arg (`-cut=hockey.regular`), spawns rink, pawns, puck, runs the sim, broadcasts events,
   restart on R, win/lose states.
6. **Tests** (automation, headless-capable): `CartridgeRoundTrip`, `ScoreOnEmptyNet` (scripted input scores within 1800 frames), `Determinism` (two runs of 600 frames of scripted
   input at seed 1 give the same `state_hash`), `PuckNeverTunnels` (30 m/s into a post for 600 frames stays inside the rink).
7. **Prove**: build, editor on 8001, PIE through MCP: a human-input stub skates and scores; screenshot to `devlog/shots/` (naming in `devlog/README.md`); `tools/run/package.ps1
   -Config Development` passes; stop your editor.

Report: what works, numbers (frame time of the sim, package size), every error, deviations with evidence. Time box 3 hours; report at 90 minutes with the state either way.
