# Assets ledger

One row per asset file that ships or might ship. The ledger is three things at once:

1. the **provenance** record the contest asks for (what is original, what is AI-generated, what is sourced),
2. the **size** ledger for the packaged-game budget (`docs/PLAN.md` section 4b: art 350 MB, audio 80 MB, zip under 1 GB),
3. the input of `tools/gate/check-assets-ledger.mjs`, which runs at every gate (`docs/GATES.md`).

Add the row in the same commit as the file. Device B owns `Game/Plugins/JamAssets/**`; any device may append rows.

## The three origins

Source: the official rules (digest in `docs/PLAN.md` section 2). Read the rules text again before submission; this is our reading of it.

| Origin | Meaning | What we must do |
|---|---|---|
| `original` | Made by us during the jam: modelled, drawn, recorded or composed by the team or its tools from nothing. | Nothing special. It is scored as ours. |
| `ai-generated` | Produced by a generative tool or service (an image, mesh, texture or audio model). **This is not "sourced"**: the rules treat AI-generated content separately. | Name the tool or service in the `tool` column and list it in the dev log and the tools list at submission. Disclosed AI use does not, on its own, raise or lower a score. |
| `sourced` | Anything not created by us and not generated: Fab or Quixel content, a marketplace pack, a free asset, a font, a sound library. | **Must be noted at submission.** Sourced assets are not scored. Record the `licence` and the `url`; no row without them. |

Rules that apply to every row:

- No trademarks or logos in the game or the video, except the Unreal Engine logo. No Claude, Anthropic, NVIDIA or Meshy logos; text credits go in the dev log.
- A row with `shipped` = `n` is kept (it records what was tried) but does not count toward the budget. Anything imported "to try" should not be committed.
- `size_mb` is the on-disk size of the asset file in megabytes with one decimal (for a `.uasset`, include its `.uexp` and `.ubulk`). The checker warns when it differs from the file by more than a quarter.
- `path` is relative to the repository root, with forward slashes, for example `Game/Plugins/JamAssets/Content/Props/SM_TestAsset.uasset`.
- `kind` is one of: `mesh`, `texture`, `material`, `audio`, `vfx`, `anim`, `font`, `other`.
  Budget groups: art = mesh, texture, material, anim; audio = audio; `vfx`, `font` and `other` are reported but have their own slice (40 MB, owned by A).
- `origin` is one of `original`, `ai-generated`, `sourced`. `device` is `A` or `B`. `shipped` is `y` or `n`. `date` is `YYYY-MM-DD`.
- Asset names follow the contract prefixes: `SM_` `M_` `MI_` `T_` `SFX_` `NS_` `BP_` (see `contracts/README.md`).

## Ledger

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Rigs/LR_L1_Day.umap | other | 0.0 | original | UE 5.9 editor via MCP |  |  | 2026-11-12 | A | y | empty placeholder sub-level made by lane A0 (C-0002); B fills it |
| Game/Plugins/JamAssets/Content/Rigs/LR_L2_Night.umap | other | 0.6 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | night rig: PPV, sky, 4 spots, room (lane B1) and 289 Dress_L2_Night actors of engine basic shapes: truss, film lights, slate, chair, crane, cases (lane B2); placeholder by lane A0 |
| Game/Plugins/JamAssets/Content/Rigs/LR_L3_Neon.umap | other | 0.0 | original | UE 5.9 editor via MCP |  |  | 2026-11-12 | A | y | empty placeholder sub-level made by lane A0 (C-0002); B fills it |
| Game/Plugins/JamAssets/Content/Look/MI_Ice_Night.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance |
| Game/Plugins/JamAssets/Content/Look/MI_PP_Outline.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance |
| Game/Plugins/JamAssets/Content/Look/MI_PP_ToonRamp.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Blue.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Board.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Dark.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Helmet.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_NetFrame.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_NetMesh.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Orange.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Puck.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Stick.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance |
| Game/Plugins/JamAssets/Content/Look/MI_Trim_Beacon.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance |
| Game/Plugins/JamAssets/Content/Look/MI_Trim_Blue.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance |
| Game/Plugins/JamAssets/Content/Look/MI_Trim_Orange.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance |
| Game/Plugins/JamAssets/Content/Look/MI_Trim_PuckHalo.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance |
| Game/Plugins/JamAssets/Content/Look/MI_Trim_PuckRim.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance |
| Game/Plugins/JamAssets/Content/Look/M_Ice.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | master material (painted-cel look) |
| Game/Plugins/JamAssets/Content/Look/M_PP_Outline.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | post-process master (painted-cel look) |
| Game/Plugins/JamAssets/Content/Look/M_PP_ToonRamp.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | post-process master (painted-cel look) |
| Game/Plugins/JamAssets/Content/Look/M_Toon.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | master material (painted-cel look) |
| Game/Plugins/JamAssets/Content/Look/M_Trim.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | master material (painted-cel look) |
| Game/Plugins/JamAssets/Content/Arena/MI_Dress_Case.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | dressing colour, instance of M_Toon or M_Trim (lane B2) |
| Game/Plugins/JamAssets/Content/Arena/MI_Dress_Cream.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | dressing colour, instance of M_Toon or M_Trim (lane B2) |
| Game/Plugins/JamAssets/Content/Arena/MI_Dress_ExitGreen.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | dressing colour, instance of M_Toon or M_Trim (lane B2) |
| Game/Plugins/JamAssets/Content/Arena/MI_Dress_Ink.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | dressing colour, instance of M_Toon or M_Trim (lane B2) |
| Game/Plugins/JamAssets/Content/Arena/MI_Dress_Lens.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | dressing colour, instance of M_Toon or M_Trim (lane B2) |
| Game/Plugins/JamAssets/Content/Arena/MI_Dress_Steel.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | dressing colour, instance of M_Toon or M_Trim (lane B2) |
| Game/Plugins/JamAssets/Content/Props/SM_PuckDisc.uasset | mesh | 0.0 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | puck halo disc, 60 cm x 4 mm (engine cylinder vertices rescaled), material MI_Trim_PuckHalo, no collision (32710 bytes) |
| Game/Plugins/JamAssets/Content/VFX/M_Fx_Sprite.uasset | material | 0.0 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | unlit additive soft-disc sprite, ParticleColor x Glow 15 (13711 bytes) |
| Game/Plugins/JamAssets/Content/VFX/M_Fx_Ribbon.uasset | material | 0.0 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | unlit additive ribbon with soft edges, ParticleColor x Glow 15 (13748 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_puck_trail.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | puck ribbon trail; user params Speed (m/s), Colour (311628 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_shot.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event shot (331864 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_board.uasset | vfx | 0.6 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event board (dust puff + flash) (564054 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_goal.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event goal (331173 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_directors_call.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event directors_call (ring) (318476 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_win.uasset | vfx | 0.5 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event win (confetti) (542894 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_check.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event check (M2) (332103 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_knockdown.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event knockdown (M2) (328807 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_pickup_spawn.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event pickup_spawn (M3) (323118 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_pickup_take.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event pickup_take (M3) (322740 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_arena_room.uasset | audio | 0.1 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (119715 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_board.uasset | audio | 0.2 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (154023 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_check.uasset | audio | 0.1 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (142017 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_directors_call.uasset | audio | 0.2 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (239073 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_faceoff.uasset | audio | 0.2 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (154032 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_goal.uasset | audio | 0.2 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (228158 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_knockdown.uasset | audio | 0.1 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (135832 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_lose.uasset | audio | 0.2 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (167504 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_pickup_spawn.uasset | audio | 0.1 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (126005 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_pickup_take.uasset | audio | 0.2 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (173745 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_shot.uasset | audio | 0.1 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (146628 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_skate_loop.uasset | audio | 0.1 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (110569 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_win.uasset | audio | 0.2 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (248461 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SCM_Volume.uasset | other | 0.0 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | sound mix (one adjuster per class) for the volume slider (2420 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SC_Ambience.uasset | other | 0.0 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | sound class (1064 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SC_Music.uasset | other | 0.0 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | sound class (1049 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SC_SFX.uasset | other | 0.0 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | sound class (1039 bytes) |


Director ruling 2026-11-12: a mesh, texture, rig or motion clip produced by a generation service from our own prompt (Meshy, Tripo, Rodin, text-to-motion) is `ai-generated`, not `sourced`; a clip or mesh taken from a service's preset library (for example Meshy library action 187 `Knock_Down`) is `sourced` and carries the service's licence and the preset id in `url`.
