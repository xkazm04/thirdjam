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


Director ruling 2026-11-12: a mesh, texture, rig or motion clip produced by a generation service from our own prompt (Meshy, Tripo, Rodin, text-to-motion) is `ai-generated`, not `sourced`; a clip or mesh taken from a service's preset library (for example Meshy library action 187 `Knock_Down`) is `sourced` and carries the service's licence and the preset id in `url`.
| Game/Content/UI/T_UI_TitleSlate.uasset | texture | 2.7 | ai-generated | Codex image generation |  | tools/images/menu-jobs.json | 2026-11-12 | A | y | menu title art: HOKEJ / A DIRECTOR'S CUT on a clapperboard (text fallback in UJamMenuWidget when absent) |
| Game/Content/UI/T_UI_TakeCard.uasset | texture | 2.5 | ai-generated | Codex image generation |  | tools/images/menu-jobs.json | 2026-11-12 | A | y | period card clapperboard; the take number is drawn into its chalk box by UJamDirectorsCallHudWidget |
| Game/Content/UI/T_UI_Card_Regular.uasset | texture | 2.2 | ai-generated | Codex image generation |  | tools/images/menu-jobs.json | 2026-11-12 | A | y | Choose your cut card background behind the row text (Regular) |
| Game/Content/UI/T_UI_Card_Boost.uasset | texture | 2.4 | ai-generated | Codex image generation |  | tools/images/menu-jobs.json | 2026-11-12 | A | y | Choose your cut card background behind the row text (Boost) |
| Game/Content/UI/T_UI_Card_Wide.uasset | texture | 2.2 | ai-generated | Codex image generation |  | tools/images/menu-jobs.json | 2026-11-12 | A | y | Choose your cut card background behind the row text (Wide) |
| Game/Content/UI/T_UI_Card_DirectorsCut.uasset | texture | 2.4 | ai-generated | Codex image generation |  | tools/images/menu-jobs.json | 2026-11-12 | A | y | Choose your cut card background behind the row text (DirectorsCut) |
| Game/Content/UI/T_UI_Icons_Cuts.uasset | texture | 1.1 | ai-generated | Codex image generation |  | tools/images/menu-jobs.json | 2026-11-12 | A | y | icon sheet 3x2: regular, raw, boost, small rink, wide, director's cut (not yet sliced) |
| Game/Content/UI/T_UI_Icons_Pickups.uasset | texture | 1.0 | ai-generated | Codex image generation |  | tools/images/menu-jobs.json | 2026-11-12 | A | y | icon sheet 1x3: speed, precision, heavy puck (not yet sliced) |
| Game/Content/UI/T_UI_Icons_System.uasset | texture | 1.0 | ai-generated | Codex image generation |  | tools/images/menu-jobs.json | 2026-11-12 | A | y | icon sheet 3x2: settings, director's call, pause, signal 1-4 (not yet sliced) |
| Game/Plugins/DLSS/Binaries/ThirdParty/Win64/nvngx_dlss.dll | other | 56.2 | sourced | NVIDIA DLSS 4.5 Unreal plugin v8.9.0 (UE 5.8 build, ported to 5.9) | NVIDIA RTX SDKs License + RTX Supplement (v. 2024-03-14) | https://developer.nvidia.com/downloads/assets/gameworks/downloads/secure/dlss/ue-dlss-5.8/8.9.0/2026.10.16_ue5.8_dlss4.5plugin_v8.9.0.zip | 2026-11-12 | A | y | the DLSS Super Resolution / DLAA runtime staged next to the exe; plugin folder (Game/Plugins/DLSS, StreamlineNGXCommon) is gitignored and fetched by tools/dlss/fetch-dlss.ps1 (zip 1,166,608,575 bytes, sha256 adeeb59b...380c); no Ray Reconstruction, Frame Generation, Reflex or NIS files; text credit only, no logo (docs/experiments/DLSS.md) |
