# C-0004: asset naming, folders and the ledger

id: C-0004
version: 1
status: active
owner: Director
parties: A, B
opened: H4

Rules for every contract are in `contracts/README.md` (including the absent-value convention). This file only fills in the specifics.

## Purpose

Every agent on both devices must find an asset by its name and folder without asking, and every shipped byte must be in the ledger so the size
budget (`docs/PLAN.md` 4b: art 350 MB, audio 80 MB, VFX/UI/fonts/cartridges 40 MB, zip under 1 GB) is counted before the package says so.

## Inputs, producers and first consumers

| Input | Producer | First consumer | Notes |
|---|---|---|---|
| an asset file under `Game/Plugins/JamAssets/Content/` | B (any lane) | A, gameplay (by object path) or B's own Blueprints | name and folder from this file |
| an asset file under `Game/Content/` (maps, blueprints, UI) | A | A | same prefixes |
| a ledger row in `ASSETS.md` | whoever adds the file, same commit | `tools/gate/check-assets-ledger.mjs` at every gate | one row per file |
| the staged size line of `tools/run/package.ps1` | A | Director (budget), B | at every package |

## Types and fields

### Ledger row (`ASSETS.md`, table "Ledger"; the columns are fixed by that file)

| Field | Type | Unit or range | Required | Meaning |
|---|---|---|---|---|
| `path` | string | repo-relative, forward slashes | yes | e.g. `Game/Plugins/JamAssets/Content/Props/SM_Puck.uasset` |
| `kind` | string | enum `AssetKind` | yes | |
| `size_mb` | number | MB, one decimal | yes | on-disk size of `.uasset` + `.uexp` + `.ubulk` |
| `origin` | string | enum `Origin` | yes | |
| `tool` | string | | for `ai-generated` | the generating tool or service |
| `licence` | string | | for `sourced` | |
| `url` | string | | for `sourced` | for a preset library item, the preset id |
| `date` | string | `YYYY-MM-DD` | yes | |
| `device` | string | `A` or `B` | yes | |
| `shipped` | string | `y` or `n` | yes | `n` rows do not count toward the budget |
| `note` | string | | no | |

In a Markdown table an absent optional value is an empty cell (the table has no keys to omit). One row per file; a material instance, a texture
and a mesh are three rows.

## Enum strings

### `AssetKind`

`mesh`, `texture`, `material`, `audio`, `vfx`, `anim`, `font`, `other`

### `Origin`

`original`, `ai-generated`, `sourced`

Unknown values are errors.

## Asset names and prefixes

| Prefix | Kind | Ledger `kind` | Example |
|---|---|---|---|
| `SM_` | static mesh | `mesh` | `SM_Puck` |
| `SK_` | skeletal mesh (see open question 1) | `mesh` | `SK_SkaterTB2` |
| `M_` | material | `material` | `M_Toon` |
| `MI_` | material instance | `material` | `MI_Trim_Blue` |
| `T_` | texture | `texture` | `T_SkaterTB2_BaseColor` |
| `SFX_` | sound wave (imported audio file) | `audio` | `SFX_PuckHit_01` |
| `MS_` | MetaSound source | `audio` | `MS_goal` |
| `NS_` | Niagara system | `vfx` | `NS_goal` |
| `BP_` | Blueprint | `other` | `BP_SkaterVisual_TB2` |
| `LR_` | light rig sub-level map | `other` | `LR_L2_Night` |

Rules:
- Name = prefix + PascalCase noun, optional `_<variant>` and `_<NN>` index: `SM_Board_Corner`, `SFX_PuckHit_02`.
- Event-bound assets keep the event's exact spelling after the prefix: `NS_pickup_take`, `MS_pickup_take` (`contracts/events.md`).
- Names fixed by other contracts: `LR_L1_Day`, `LR_L2_Night`, `LR_L3_Neon`, `MI_PP_ToonRamp`, `MI_PP_Outline` (`contracts/look-tiers.md`);
  `BP_SkaterVisual_TA`, `BP_SkaterVisual_TB1`, `BP_SkaterVisual_TB2` (`contracts/skater-visual.md`).
- No spaces, no hyphens, ASCII only. No trademarks or logos in any asset.

## Folders

All art, audio, VFX and light rigs live in the content-only plugin `JamAssets` (mount point `/JamAssets/`). Game logic content lives in `Game/Content/`.

| Folder (repo path) | Mount path | Holds | Written by | Read by |
|---|---|---|---|---|
| `Game/Plugins/JamAssets/Content/Rigs/` | `/JamAssets/Rigs/` | `LR_` maps | B | A, look switch |
| `Game/Plugins/JamAssets/Content/Look/` | `/JamAssets/Look/` | master materials, post materials, their instances, shared textures | B | B |
| `Game/Plugins/JamAssets/Content/Skaters/` | `/JamAssets/Skaters/` | skater meshes, skeletons, anims, `BP_SkaterVisual_TB*` | B | A, by class path |
| `Game/Plugins/JamAssets/Content/Arena/` | `/JamAssets/Arena/` | boards, nets, ice, room dressing | B | A, rink builder (by object path) |
| `Game/Plugins/JamAssets/Content/Props/` | `/JamAssets/Props/` | puck, pickups, small props | B | A |
| `Game/Plugins/JamAssets/Content/VFX/` | `/JamAssets/VFX/` | `NS_`, `BP_EventFx` | B | B |
| `Game/Plugins/JamAssets/Content/Audio/` | `/JamAssets/Audio/` | `SFX_`, `MS_`, music | B | B |
| `Game/Plugins/JamAssets/Content/UI/` | `/JamAssets/UI/` | fonts, icons, UI textures | B | A, UI |
| `Game/Content/Maps/` | `/Game/Maps/` | `Rink` and test maps (only `Rink` is cooked) | A | everyone |
| `Game/Content/Blueprints/` | `/Game/Blueprints/` | gameplay Blueprints, `BP_SkaterVisual_TA` | A | A |
| `Game/Content/UI/` | `/Game/UI/` | HUD, menus, Designer UI | A | A |
| `Game/Content/Cartridges/` | `/Game/Cartridges/` | cartridge JSON and data assets | A (lane A4) | A |

`Game/Content/Look/` and `Game/Content/Audio/` (B in `ownership.json`) are not used: B writes into the plugin. A folder not listed here needs a new version of this contract.

## Limits

- Textures: **2048 x 2048 maximum** (1K for small props). Set `MaxTextureSize` 2048 (or 1024) on import; the import lane checks it in the same Python call.
- One texture set per material family; no 4K source left in the project "to try".
- Audio: OGG/ADPCM compression, music stereo, everything else mono.
- Generated meshes decimated on import (the T-B2 skater was 20.8k triangles; keep a skater under 30k).
- Nothing imported from `prep-reference/`.

## Data asset names

None in this contract.

## Acceptance

- [ ] `tools/gate/check-assets-ledger.mjs` passes: every file under `Game/Plugins/JamAssets/Content/` has exactly one ledger row, sizes within a quarter.
- [ ] A Python check in the editor lists every asset under `/JamAssets/` and reports names whose prefix does not match their class.
- [ ] Every `T_` asset reports `max_texture_size` <= 2048 or a source of at most 2048.
- [ ] `tools/run/package.ps1` staged and zip sizes are within the budget lines of `docs/PLAN.md` 4b.

## Version history

- v1 (H4, lane A0): first version.

## Open questions

1. `SK_` (skeletal mesh), `SKEL_` (skeleton), `A_`/`AS_` (animation sequence), `ABP_` (animation Blueprint), `PM_` (physical material) and `DA_`
   (data asset) are not in the brief's prefix list but the skater and physics recipes use them; proposed here as `SK_`, `SKEL_`, `AS_`, `ABP_`, `PM_`, `DA_` for v2.
2. `ASSETS.md` gate checker `tools/gate/check-assets-ledger.mjs` is referenced by the ledger but does not exist yet in `tools/`.
