# C-0002: look tiers (light rigs)

id: C-0002
version: 1
status: active
owner: Director
parties: A, B
opened: H4

Rules for every contract are in `contracts/README.md` (including the absent-value convention). This file only fills in the specifics.

## Purpose

The arena geometry is one set; the look tier changes the lighting rig, the post process and material parameters. Device B builds the three rigs;
device A owns the switch that loads exactly one of them from the cartridge key `game.look.tier`. Both must spell the rig maps, the tier strings,
the volume names and the parameter names the same way. C++ source of the switch: `Game/Source/JamGame/Public/LookTierSwitcher.h`.

## Inputs, producers and first consumers

| Input | Producer | First consumer | Notes |
|---|---|---|---|
| `game.look.tier` (cartridge) | A, lane A4 (schema and presets); the Designer at runtime | A, `ULookTierSwitcher::SetTier` | enum `LookTier` |
| rig maps `LR_L1_Day`, `LR_L2_Night`, `LR_L3_Neon` | **A, lane A0: empty placeholders (H4)**; B fills them | A, `ULookTierSwitcher` (streams one in) | B owns the contents from H4 on; A never edits them again |
| actors inside a rig (table below) | B | the renderer | |
| material parameters `TeamColour`, `Emissive`, `Wet`, `Scuff` | B (master materials) | A (gameplay drives them), B (rig defaults) | names from `contracts/skater-visual.md` |
| log line `JAMLOOK tier=<tier> map=<path>` | A, `ULookTierSwitcher` | tests, B's verification | `JAMLOOK_UNKNOWN` and `JAMLOOK_FAIL` are errors |
| project render settings (Lumen, VSM, SM6, `r.CustomDepth=3`, distance fields) | A, `Game/Config/DefaultEngine.ini` (H4) | B, every rig and post material | set; see `tools/skills/look-painted-cel.md` section 0 |

## Types and fields

### `ULookTierSwitcher` (C++ `UWorldSubsystem`, module `JamGame`; game and PIE worlds only)

| Member | Signature | Meaning |
|---|---|---|
| `SetTier` | `bool SetTier(const FString& Tier)` | streams in the tier's rig map at the origin (`ULevelStreamingDynamic`), requests unload of every other rig it loaded; unknown tier: error log, nothing changes, returns false; same tier again: no-op, true |
| `GetCurrentTier` | `FString GetCurrentTier() const` | last tier set; empty before the first call |
| `GetKnownTiers` | `static TArray<FString>` | the three tier strings, ladder order |
| `GetRigMapPath` | `static FString GetRigMapPath(const FString& Tier)` | object path of the rig map; empty for an unknown tier |

### Rig map (one per tier)

A sub-level map whose actors are placed in world coordinates of the arena (rink centre at the origin, ice top at Z = 0). It contains only
lighting, sky, post process, fog and look dressing; no gameplay actors, no `PlayerStart`, no collision that touches the rink.

| Actor (label) | Class | Required | Notes |
|---|---|---|---|
| `PPV_<Tier>` (e.g. `PPV_L2_Night`) | `PostProcessVolume`, `unbound` true | yes | exposure, bloom, grade, blendables `MI_PP_ToonRamp`, `MI_PP_Outline` |
| `Sky_<Tier>` | `SkyLight` | yes | |
| `Sun_<Tier>` | `DirectionalLight` | L1 only | |
| `Spot_<Tier>_<n>` (n = 1..) | `SpotLight` | L2, L3 | |
| `Fog_<Tier>` | `ExponentialHeightFog` | L3 only | absent in L1 and L2 (fog costs 0.3-0.5 ms) |
| `Trim_<Tier>_<n>` | emissive strip meshes | optional | stencil 0 |

The persistent map `Game/Content/Maps/Rink.umap` has its own `SkyLight` and a `DirectionalLight` that is off, for the editor only; when a rig is loaded
the rig's lights are the lighting. **Open question 2** decides whether the persistent ones are removed.

## Enum strings

### `LookTier` (`game.look.tier`)

`l1_day`, `l2_night`, `l3_neon`

Unknown values are errors. Shipped order: `l2_night` first (Director decision 2026-11-12), the others when time allows.

## Asset names and prefixes

| Asset name | Kind | Object path (what the code loads) | File |
|---|---|---|---|
| `LR_L1_Day` | map (sub-level) | `/JamAssets/Rigs/LR_L1_Day.LR_L1_Day` | `Game/Plugins/JamAssets/Content/Rigs/LR_L1_Day.umap` |
| `LR_L2_Night` | map (sub-level) | `/JamAssets/Rigs/LR_L2_Night.LR_L2_Night` | `Game/Plugins/JamAssets/Content/Rigs/LR_L2_Night.umap` |
| `LR_L3_Neon` | map (sub-level) | `/JamAssets/Rigs/LR_L3_Neon.LR_L3_Neon` | `Game/Plugins/JamAssets/Content/Rigs/LR_L3_Neon.umap` |
| `MI_PP_ToonRamp`, `MI_PP_Outline` | material instances (post) | `/JamAssets/Look/...` | `Game/Plugins/JamAssets/Content/Look/` |

**The three rig maps exist as empty placeholders (H4) so the switch and the cook list work today; device B fills them.** Each is cooked because it is listed
in `+MapsToCook` in `Game/Config/DefaultGame.ini` (the switch loads them by path, so the cooker cannot find them through references). A new rig map
needs a cook-list line from A (handoff request).

## Folders

| Folder | Written by | Read by |
|---|---|---|
| `Game/Plugins/JamAssets/Content/Rigs/` | B (A created the empty maps once) | A, `ULookTierSwitcher` |
| `Game/Plugins/JamAssets/Content/Look/` | B | B's rigs |
| `Game/Config/` | A | everyone |

## Frame budget per tier (1080p, device A, RTX 4090, Shipping; GPU ms, p99 frame under 8 ms for the whole game)

| Tier | GPU budget for lighting + post | Evidence |
|---|---|---|
| `l1_day` | 4.0 ms | not measured |
| `l2_night` | 4.5 ms | measured 4.3 ms (2.9 ms low profile), `docs/experiments/LOOK-P2-NIGHT.md` |
| `l3_neon` | 5.5 ms | not measured; ships as `l2_night` with neon trim if it misses (HOCKEY.md M3) |

## Data asset names

None. A rig is a map, not a data asset.

## Acceptance

- [x] The three maps exist at the paths above (H4, empty).
- [x] In PIE, `SetTier("l2_night")` then `SetTier("l3_neon")` log `JAMLOOK tier=...` and return true; `SetTier("bogus")` logs `JAMLOOK_UNKNOWN` and returns false (H4, lane A0, through MCP).
- [ ] Each filled rig contains `PPV_<Tier>` and `Sky_<Tier>`; a Python check lists the actor labels of each map.
- [ ] A package contains the three rig maps (cooked), and the frame gate holds for each tier that ships.

## Version history

- v1 (H4, lane A0): first version, with empty rig maps and the compiled switch.

## Open questions

1. Who calls `SetTier` at game start: the game mode on cartridge load (proposed), so it is A's skater/game-mode lane.
2. Keep or remove the persistent map's `SkyLight` and off `DirectionalLight` once rigs are filled (two sky lights would add up).
3. Fog and post volume names above are proposals by A0; B may counter-propose before filling the first rig.
