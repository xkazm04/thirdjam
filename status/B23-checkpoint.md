# B23 checkpoint 1 (03:24, 13 Nov)

Lane B23, clay tier pass 5. Editor: mine, port 8002, pid 35576. No git command. `/Game/Maps/Rink` not saved (mtime 11-12 11:10, 18,966 bytes).

## Saved on disk (read back after the save, PIE stopped)

| File (`Game/Plugins/JamAssets/Content/`) | Before | Now | What |
|---|---:|---:|---|
| `Look/M_Toon_Rail.uasset` | new | 61,379 | duplicate of `M_Toon`, blend mode Masked, opacity mask 0 when `MPC_Look.Clay` >= 0.5 (`B23RailMask`, switch `RailHideOnClay` 1); skeletal-mesh usage off |
| `Look/MI_Toon_GlassRail.uasset` | 9,375 | 9,831 | same name and path; parent is now `M_Toon_Rail` |
| `Look/MI_Glass.uasset` | 9,205 | 10,201 | `ClayGlassK` 0: the pane (A loads `MI_Glass`, not `MI_Toon_Glass`) is fully invisible on clay |
| `Look/M_Trim.uasset` | 14,393 | 20,682 | clay cap path `B23TrimBase` / `B23TrimEmis` / `B23TrimRough`, master default off (`ClayCap` 0) |
| `Look/MI_Trim_Blue.uasset` | 8,302 | 8,718 | `ClayCap` 1 |
| `Look/MI_Trim_Orange.uasset` | 8,483 | 8,914 | `ClayCap` 1 |
| `Look/M_Ice.uasset` | 121,490 | 135,803 | `B23IceFx` / `B23IceBase` / `B23IceEmis`: key-pool compensation, contact shade and warm band at the board foot (switch `ClayIceB23`) |
| `Look/MI_Ice_Night.uasset` | 28,891 | 31,506 | `ClayIceB23` 1, `PoolDim` 0.45, `PoolBoost` 1.4, `ContactCm` 14, `ContactK` 0.6, `BoardBandK` 1.6 |
| `Look/M_Zone.uasset` | 42,361 | 45,013 | `B22ZoneGlow` extended: minimum edge width in pixels, far spill, far chroma |
| `Rigs/LR_P3_Clay.umap` | 294,776 | 294,972 | the six `Fill_P3_Clay_*` lights: specular scale 0 |

## Found

- The white blob on the ice is the mirror image of a shadowless side fill (`Fill_P3_Clay_2` at y +30 m, z 8 m), not of a key spot. Fills without specular: blob gone, figure luma 116.3 -> 116.1.
- A's `GlassRail` component has `CastShadow` false already: no shadow to remove.
- The trim MIDs set only `Emissive`; the clay cap path multiplies the team emissive by `ClayCapGlow` (0), so A's value cannot bring the glow back.

## Stills so far (scratch `scratchpad/b23/shots/`)

Before: `pre_night_*`, `pre_bcast_*`, `pre_clay_*` (game, 24 m, boards), `pre_clayz_*` (low camera, game camera with zones, boards). After (in memory, before the save): `t3_zlow`, `t3_g1`, `t4_b8`.

## In progress

After stills from the saved assets, tier stability (night, broadcast), 30-frame motion check, hero still, report.

# B23 checkpoint 2 (04:10, 13 Nov): finished

- Everything is saved; the editor (port 8002, pid 35576) left through `ExitEditor` at 04:02:13, process gone. `/Game/Maps/Rink` not saved.
- Since checkpoint 1: `MI_Ice_Night` 31,506 -> 32,282 bytes (`PoolBoost` 1.0, board band 400 cm x 3.6, amber); `M_Trim` `ClayCapFill` 0.6. All other files as in the table above.
- Final tier stability after the last save (03:59 to 04:01): boards views 0.43 to 0.55, no leak.
- Motion check done (30 frames, held camera, fast turn, following camera).
- Stills filed under `devlog/shots/20261113-034219-B-b23-*`, `-034538-B-b23-tier-stability`, `-040327-B-b23-*`.
- Report: `status/B23-report.md`.
