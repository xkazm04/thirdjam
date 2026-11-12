# B14 checkpoint: clay tier pass 2 done, broadcast rig next (device B)

Lane B14, device B, 2026-11-12, machine clock 16:45 to 17:10 for part 1. Main-tree editor on port 8000; a second editor (port 8002) shares the GPU, so no frame numbers.
No git command, no build. `Rink` never saved, nothing under `/Game/` written, nothing under `_Scratch/`. Editor left on `/Game/Maps/Rink`, no dirty package.

## How the tier and the figures were selected

- **Tier: through the game.** A scratch copy of `hockey.regular` with `"tier": "p3_clay"` and `"visual": "generated_b2"`, loaded with `jam.Cut <path>` before PIE. Log:
  `JAMLOOK tier=p3_clay map=/JamAssets/Rigs/LR_P3_Clay.LR_P3_Clay scale=1.000 (rink 90.0 m) trim_emissive=12.0`, `JAMLOOK rig loaded: 3 local lights scaled`,
  `JAMLOOK persistent lights hidden: 2 actors`. In PIE: `MPC_Look` Clay 1, ClayBump 2, FeltRim 0.5 (now 1.0), ClayFill 0.15, ClayTile 25; both trim MIDs at Emissive 12; beacons 40.
- **Figures: by script.** The game still spawns `BP_SkaterVisual_TB2` for `generated_b2` (B-to-A-0021 open), and Python cannot spawn actors during PIE, so each game visual
  is turned into the P3 figure in place: `SK_SkaterTB2_P3`, `ABP_SkaterTB2_P3`, a MID of `MI_SkaterTB2_P3` with the pawn's team colour. Rclay is the same with `SK_SkaterTB2` and `MI_SkaterTB2_Clay`.
  Nothing under `Skaters/` was changed.

## What changed (all clay-only: at `Clay` 0 every new node returns the old value, and every new parameter defaults to the old constant)

| Asset | Change |
|---|---|
| `Look/Textures/T_Clay_Dent_N` (new) | 512 tileable thumb-dent normal map made by numpy (64 soft elliptical dents with a lip, 10 smears, fine grain), BC5, no generated image |
| `M_Ice` | `ClayNormalTex` (texture parameter, default `T_Clay_Thumb_N` as before), `ClayIceTileMul` 4, `ClayIceBumpMul` 0.5, `ClayIceRough` 0.5 (were constants), `ClayScuffMul` 1 (gate on the scuff mask), `ClayInlay` 0 / `ClayInlayCm` 3 (dark groove along every marking edge) |
| `M_Toon` | `ClayNormalTex`, `ClayTileMul` 1, `ClayBumpMul` 1, `ClayFeltRail` 0, `ClayFeltBandCm` 12, `ClayFeltColour` (felt band under the rail top, roughness 1) |
| `MI_Ice_Night` | ClayNormalTex `T_Clay_Dent_N`, ClayIceTileMul 4 → 18 (dent tile 1 m → 4.5 m, a dent about 50 cm), ClayIceBumpMul 0.5 → 0.22, ClayIceRough 0.5 → 0.8, ClayScuffMul 1 → 0.35, ClayInlay 0 → 1, ClayInlayCm 3 → 5 |
| `MI_Toon_Board` | ClayNormalTex `T_Clay_Dent_N`, ClayTileMul 1 → 6 (1.5 m tile), ClayBumpMul 1 → 0.3, ClayFeltRail 0 → 1 (12 cm dark red felt band under the rail) |
| `M_ClayBackdrop` (new), `MI_Clay_Backdrop` | unlit two-sided paper sweep: warm (0.55, 0.36, 0.22) at the table to near-black at 45 m, dark below the table, faint folds |
| `MI_Clay_Table`, `MI_Clay_Wire`, `MI_Clay_Card` (new, `M_Toon`), `MI_Clay_Lens` (new, `M_Trim`, Emissive 18, warm) | tabletop, wire poles, cardboard lamp heads, lit lenses |
| `LR_P3_Clay` | `Con_P3_Clay_Table` (150 x 110 m slab, top -3 cm) + 4 legs; `Back_P3_Clay_Sweep` (cylinder, 110 m radius, -12 to 68 m); 3 fill stands and a key stand (`Con_P3_Clay_Stand_<Fill1/Fill2/Fill3/Key>_*`: pole, base, 3 legs, head, yoke, lens). `Fill_P3_Clay_2/3` moved from (±56, 0, 13) to (56, 18, 13) / (-56, -18, 13), aimed at centre ice, so no stand stands behind a net. PPV focus 7 m → 11 m, f/1.4 → f/3.2, depth blur 0.06 km / 6 px. `LookParams_P3_Clay` FeltRim 0.5 → 1.0. All new meshes shadowless, no collision, no distance-field or indirect contribution. |

## Focus numbers (game camera at the faceoff: (-16.3, 1.2, 3.5) m, pitch -17.65, FOV 70; sensor 100 mm, so f = 71.4 mm)

| Point | Distance | CoC old (7 m, f/1.4) | CoC new (11 m, f/3.2) | + depth blur |
|---|---:|---:|---:|---:|
| foreground edge of the frame | 5.5 m | 2.8 px | 2.8 px | 0 |
| own player | 6.5 m | 0.8 | 1.9 | 0 |
| rink centre | 16.6 m | 5.8 | 0.9 | ~1.0 |
| far blue line | 31.5 m | 7.9 | 1.8 | ~1.8 |
| far net | 57.4 m | 8.9 | 2.3 | ~2.9 |
| backdrop | ~110 m | 9.5 | 2.5 | ~4.3 |

(CoC = blur-circle diameter at 1920 px wide.) The near own net is 24.7 m behind the camera at the faceoff; when the player attacks, the camera is 11-17 m from that net, inside the sharp band (`b14-clay-final-attack`).
No tracking focus is needed for the fixed follow camera (6 m behind, 3.5 m up). If A ever wants one: a `MPC_Look` scalar cannot drive a PPV; A would set the camera's
`PostProcessSettings.DepthOfFieldFocalDistance` to the camera-to-player distance each frame.

## Verdict so far

The rink now reads as a clay set on a table: cream clay ice with soft thumb-pressed dents (about 50 cm, a visible irregularity at 24 m), markings as inlaid strips with a groove, felt band under the rail, warm paper sweep and lit card lamps on wire stands. P3 figures in both colours are distinct; the puck reads near and at 24 m with the halo (`b14-clay-puck-*`). Rejected: the first ice values (tile 3 m, bump 0.5: a crater field), board bump 0.5 (golf-ball look), any bump on the table slab (local-space triplanar on a 150 m scaled cube makes dunes), FeltRim 1.2 (kept 1.0).

Stills: `b14-clay-before-*` (16:51), `b14-clay-after-*`, `b14-clay-figs-*`, `b14-clay-puck-*` (17:06), `b14-clay-final-*` (17:08).
