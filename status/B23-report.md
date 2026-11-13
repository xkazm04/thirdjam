# B23 report: clay tier pass 5: open boards, red cap, fill blobs, key pools, far zones, board foot (device B)

Lane B23, device B, machine clock 02:56 to 04:10 (2026-11-13). Main tree `C:/Users/kazda/jam/thirdjam` (branch `dev-b`), editor on port 8002 (pid 35576, mine), left through `ExitEditor` at 04:02:13 (`LogExit: Exiting.`, PIE stopped, dirty list empty just before, process gone). The other lane's editor was never touched.

Housekeeping:
- No git command that writes, no build, no cook. **No frame numbers** (shared GPU).
- `/Game/Maps/Rink` never saved (mtime still 11-12 11:10, 18,966 bytes; loaded once from disk after the rig edit, dirty list empty before and after). Nothing under `/Game/` written. `_Scratch/` does not exist. `Game/Saved/Screenshots/WindowsEditor/` is empty.
- `jam.Cut hockey.regular` and `jam.ScriptHuman 0` set back before the exit.
- `M_Toon` is unchanged (60,521 bytes, mtime 11-12 23:18). Not touched: `Skaters/`, `Audio/`, `Props/`, `VFX/`, `Look/P4Lab/`, `LR_P4_Broadcast`, `LR_L2_Night`, `MPC_Look`, `BP_LookParams`.
- 0 `[VSM] Non-Nanite Marking Job Queue overflow` lines in this session's log.
- Checkpoint: `status/B23-checkpoint.md`. Scratch (scripts, every capture, backups of the files as found in `bk/`): `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b23`.

Method: lanes B19 / B22's. Real matches in PIE on `/Game/Maps/Rink` through the scratch cartridges (`c_clay`, `c_clayz` zones cut, `c_night`, `c_bcast`, `c_wide`), frozen by `slomo` at t 2.17 (faceoff) or 4.6 (zones cut), `CaptureGameViewport` 1920 x 1085. Every "after" still is from the saved assets.

## Verdicts

| # | Task | Verdict | Still |
|---|---|---|---|
| 1 | Glass rail, stanchions and pane vanish on clay | **PASS**: none of them is drawn on clay; night and broadcast unchanged | `b23-boards-ab`, `b23-tier-stability` |
| 2 | Plain red top rail on clay | **PASS**: the trim strips are a matte red cap with no glow on clay | `b23-boards-ab`, `b23-game-after` |
| 3 | Key-light pools | **PASS**: the white blob is gone and the streaks hold through the pool. The blob was a side fill's mirror image, not a key's | `b23-low-before` / `b23-low-after` |
| 4 | Far zone rings | **PARTIAL**: far rings keep a visible width and their colour; they are still flat rings, not glowing objects | `b23-zones-far-ab` |
| 5 | Board base on the ice | **PASS**: soft contact shade and a warm band along the boards. The dark reflection band along the side boards is softened, not removed | `b23-boards-ab`, `b23-side-band-ssr` |
| 6 | 30-frame motion check | **PASS** in static regions; one existing fault seen in the fast turn (below) | `b23-motion` |
| | Night and broadcast stability | **PASS**: no leak | `b23-tier-stability` |

## 1. Rail, stanchions and pane

**How.** `MI_Toon_GlassRail` (same name, same path) now has its own parent, **`M_Toon_Rail`**: a duplicate of `M_Toon` with blend mode Masked and one Custom node, `B23RailMask`, on the opacity mask: 0 when `MPC_Look.Clay` >= 0.5 and the switch `RailHideOnClay` (default 1) is on, otherwise 1. The instance's own values (near-black rail, dark wood on clay) are kept. Skeletal-mesh usage is off on the duplicate. I chose the mask over a world-position offset because it also holds in passes that ignore the offset, and it adds no moving-vertex flag on the other tiers.

**The pane.** A's code loads `MI_Glass` for the pane on every tier but broadcast (`RinkActor.cpp` `SetGlassForTier`), not `MI_Toon_Glass`, so lane B19's `ClayGlassK` 0.12 never reached it and the pane's top edge showed as a faint line. `MI_Glass` now sets `ClayGlassK` 0: the pane is fully transparent on clay.

**Shadow.** A's `GlassRail` component has `CastShadow` false already. There was no shadow to remove.

**Verified on clay** (`b23-boards-ab`, `b23-game-after`, `b23-low-after`, `b23-wide-hi`): no rail, no posts, no pane edge, no dark line in the ice. Read in PIE: `GlassRail` still carries `MI_Toon_GlassRail` on both sections, `Glass` carries `MI_Glass`, `Clay` 1.

**What it costs.**
- One more master material (61,379 bytes on disk). Its shader count in a cooked build is not measured.
- Night and broadcast: the rail is drawn masked instead of opaque; the mask is one collection compare. Compiled pixel shader: 549 instructions, the same as `M_Toon`.
- Clay: A's component still submits the rail and post triangles; every pixel is clipped. The pane still runs as a translucent draw with opacity 0. Hiding the three on A's side would save those draws; it is no longer needed for the picture.

## 2. Red cap

`M_Trim` gains a clay-only path (group `Clay cap (B23)`), three Custom nodes that return the old value unless Clay >= 0.5 and `ClayCap` > 0:
- `B23TrimBase`: base colour `ClayCapColour` (0.62, 0.07, 0.045).
- `B23TrimEmis`: the team emissive times `ClayCapGlow` (0) plus the cap colour times `ClayCapFill` (0.6), so the unlit side faces stay red.
- `B23TrimRough`: `ClayCapRough` 0.45 on clay, 0.5 elsewhere (the value the unconnected pin had).

`ClayCap` is 0 in the master and 1 only on `MI_Trim_Blue` and `MI_Trim_Orange`. The other `M_Trim` instances (puck halo, puck rim, beacon, film lens) are unchanged on clay.

A's dynamic instances set only `Emissive`. That does not defeat the path: the team glow is multiplied by `ClayCapGlow`. Consequence: **`look.trim_emissive` has no effect on `p3_clay` any more.**

## 3. Key pools and the white blob

**Finding.** The white blob in `b22-low-after` is not a key spot. It is the mirror image of `Fill_P3_Clay_2` (y +30 m, z 8 m): from the low camera its reflection point is at x -13.3, y 1.0 m, where the blob sits. The same holds for the blob at the boards view (`Fill_P3_Clay_1`).

**Changes.**
- `LR_P3_Clay`: the six `Fill_P3_Clay_*` lights have specular scale 0. The keys are unchanged.
- `M_Ice`, three Custom nodes (group `Clay pools and board foot (B23)`), active only when Clay >= 0.5, `ClayIceB19` > 0 and the new switch `ClayIceB23` > 0 (master 0, `MI_Ice_Night` 1):
  - `B23IceFx`: the four keys' irradiance on the ice computed from their layout (`PoolKeyX` 1125 / 3375 cm, `PoolKeyZ` 1300, cones 16 / 58 deg, scaled by `RinkHalfLength / BRLayoutHalfLen`), and the distance to the rink outline.
  - `B23IceBase`: inside the pools the ice body is tinted down and warmer (`PoolDim` 0.45, `PoolTint` (0.78, 0.66, 0.56)).
  - `B23IceEmis`: the bulb reflections are lifted inside the pools (`PoolBoost` 1.0).

**Numbers (low camera, sRGB luma, before → after).**

| Area | Before | After |
|---|---|---|
| Blob area, right foreground | 194.3, max 255 | 173.6, max 195 |
| Right key pool | 191.1, rgb (208, 188, 172) | 186.3, rgb (207, 183, 160) |
| Left near ice | 141.6 | 140.1 |
| Figure, low camera | 109.7 | 112.9 |
| Jersey / helmet, game camera | 129.3 / 82.1 | 128.3 / 81.6 |
| Far orange figure, game camera | 85.8 | 87.7 |

The figures are as well lit as before. The scratches still catch light near the camera (`b23-low-after`). The right pool is still the palest part of the frame: the streaks cross it now but are weaker there than on the left.

`PoolBoost` 1.4 was tried and set back to 1.0: at the game camera the middle ice became a dense golden curtain behind the orange team.

## 4. Far zones

`M_Zone`, `B22ZoneGlow` extended (three new master parameters, clay path only):
- the edge is never thinner than `ZGlowMinPx` 1.5 pixels, and a widened edge sits inside the outline so the quad's border does not cut it;
- as the pixel footprint grows from 4 to 16 cm, the edge brightens (`ZGlowFarEdge` 1.3), the inward spill grows (`ZGlowFarSpill` 0.55) and the colour keeps its chroma.
- Near zones (footprint under 4 cm per pixel) are unchanged.

Result (`b23-zones-far-ab`): the ring at 33 m is a clean ring about 3 pixels wide instead of a broken 1.5 pixel line; the one at 25 m is a little more saturated. They do not glow. A zone is a flat translucent quad exactly as large as its outline, so no light can spill outside it, and brighter only means paler.

## 5. Board foot

`B23IceFx` also gives the distance to the boards (the same rounded outline lane B19 uses for the snow):
- contact shade: the ice body darkens over `ContactCm` 14 (`ContactK` 0.6);
- warm band: an amber reflection term along the boards, Fresnel-weighted, under the snow (`BoardBandCm` 400, `BoardBandK` 3.6, `BoardBandColour` (1, 0.52, 0.24)).

**Finding, not fixed.** The dark band with a hard edge on the ice along the side boards (strongest looking back from the far end) is the screen-space reflection of the dark barn above the boards: with `r.SSR.Quality 0` in the same frozen frame it is a smooth gradient (`b23-side-band-ssr`). The wide band softens it at the game camera and looking back; it is still there.

## 6. Motion

Method: lane B22's, 30 consecutive rendered frames per run, 1/60 s game step per frame (log: +0.0167 s). Captured at 03:36, before the board band was widened from 220 to 400 cm (an instance value of a static term).

| Run | Result |
|---|---|
| Held game camera, play running | mean \|d\| between consecutive frames: left boards where the rail stood 0.23, right boards 0.24, far cap 0.21 to 0.22, bulbs 0.17, streaks 0.22, near ice 0.22, far zone 0.31. Whole frame 0.20 to 0.31 |
| Fast turn, 3 deg per frame (180 deg/s at 60 fps) | by eye: no rail remnant, no popping of the cap or the zones |
| Following game camera in play | by eye: the same |

The only crop with strong change (net area, per-pixel temporal std p99 38) holds the moving figure at the far net.

Seen in the fast turn: the dark reflection band of item 5 appears and moves along the side boards between consecutive frames. I have no motion run from before my changes to compare.

## Tier stability

Night and broadcast stills at 02:58 to 03:02, before any change, and at 03:59 to 04:01, after the last save. Same cartridges and frozen frame.

| Still | Mean \|d\| | Tile max | Frame luma |
|---|---:|---:|---|
| Night, boards 8 m (rail close) | 0.43 | 1.21 | 60.36 → 60.36 |
| Night, boards 4.5 m | 0.45 | 1.48 | 69.30 → 69.28 |
| Night, game camera | 1.42 | 16.68 | 136.05 → 135.99 |
| Night, 24 m | 0.66 | 5.60 | 98.48 → 98.51 |
| Broadcast, boards 8 m (rail close) | 0.49 | 1.22 | 93.34 → 93.30 |
| Broadcast, boards 4.5 m | 0.55 | 3.79 | 87.35 → 87.64 |
| Broadcast, game camera | 1.16 | 14.01 | 147.98 → 148.01 |
| Broadcast, 24 m | 1.02 | 6.18 | 122.86 → 122.64 |

- Open-ice crops: night 0.61 (game) and 0.42 (24 m); broadcast 0.53 (game) and 1.37 (24 m, luma 190.61 → 189.39).
- The broadcast 24 m crop is above lane B19's 0.5. Two "after" runs of the same crop, with nothing of broadcast changed between them, differ by 0.97: that is the tier's own run-to-run variation. The signed difference map shows soft blotches and figure outlines, no pool, band or rail shape.
- The game-camera numbers over 1 are figure outlines (frozen t 2.20 against 2.19) and grain at the frost edges; two "after" runs differ by 1.21 in the same board-top crop.
- Every new path is behind `Clay >= 0.5`. **No leak.**

## Against `est-p3-barn.png`, point by point

| Estimate | Now |
|---|---|
| Open boards, no glass, no fence | **Reached** |
| Plain red top rail | **Reached** (a 10 cm cap on a red band) |
| No white lamp blobs on the ice | **Reached** |
| Golden reflections everywhere | **Closer, not reached**: streaks now cross the key pool and the ice is warmer there; ours are narrow regular streaks, the estimate's are wide golden smears, and our right pool is still pale |
| Glowing rings with reflections | **Not reached**: flat rings that now hold their width at distance |
| Light, even ice up to the boards | **Partly**: warm band and contact shade at the foot; the dark reflection band remains along the side boards |
| Bulbs filling the upper third | Unchanged (camera geometry, lane B22) |

## Assets created or changed (bytes on disk, `Game/Plugins/JamAssets/Content/`)

| File | Before | After |
|---|---:|---:|
| `Look/M_Toon_Rail.uasset` | new | 61,379 |
| `Look/MI_Toon_GlassRail.uasset` | 9,375 | 9,831 |
| `Look/MI_Glass.uasset` | 9,205 | 10,201 |
| `Look/M_Trim.uasset` | 14,393 | 20,682 |
| `Look/MI_Trim_Blue.uasset` | 8,302 | 8,718 |
| `Look/MI_Trim_Orange.uasset` | 8,483 | 8,914 |
| `Look/M_Ice.uasset` | 121,490 | 135,803 |
| `Look/MI_Ice_Night.uasset` | 28,891 | 32,282 |
| `Look/M_Zone.uasset` | 42,361 | 45,013 |
| `Rigs/LR_P3_Clay.umap` | 294,776 | 294,972 |

Compiled pixel shader instructions: `M_Ice` 1,309 → 1,375, `M_Trim` 208 → 221, `M_Zone` 429 → 448. Other tiers pay one branch per new node.

## Ledger rows

**Add** the first row. The other nine **replace** the existing rows of the same path.

```
| Game/Plugins/JamAssets/Content/Look/M_Toon_Rail.uasset | material | 0.06 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | duplicate of M_Toon for the glass rail and stanchions: Masked, B23RailMask opacity mask 0 on the clay tier (open boards), unchanged elsewhere (lane B23) (61379 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_GlassRail.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_Toon_Rail (B23; was M_Toon), near-black rail and stanchions (B15); not drawn on the clay tier (B23) (9831 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Glass.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance for A's Glass component (lane B13); ClayGlassK 0: invisible on the clay tier (B23) (10201 bytes) |
| Game/Plugins/JamAssets/Content/Look/M_Trim.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-13 | B | y | master material (painted-cel look) + clay cap path B23TrimBase/Emis/Rough: matte painted cap without team glow where ClayCap > 0, Clay >= 0.5 only (B23) (20682 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Trim_Blue.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-13 | B | y | instance; ClayCap 1: red board cap on the clay tier (B23) (8718 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Trim_Orange.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-13 | B | y | instance; ClayCap 1: red board cap on the clay tier (B23) (8914 bytes) |
| Game/Plugins/JamAssets/Content/Look/M_Ice.uasset | material | 0.14 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-13 | B | y | master material (painted-cel look) + scuff zones, scratches, frost (B13), clay pass 2 / broadcast mode (B14), tiles and NightWet (B15), glossy scratched clay-tier ice B19IceMask/Base/Rough/Normal/Spec/Emis (B19), B22BulbRefl analytic bulb-string reflection and ScratchNormFar footprint fade (B22), B23IceFx/Base/Emis key-pool compensation, contact shade and warm band at the board foot, Clay >= 0.5 only (B23) (135803 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Ice_Night.uasset | material | 0.03 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-13 | B | y | instance (B13 surface, B14 clay/broadcast, B15 tiles, NightWet 0.3; B19 clay ice; B22 bulb reflection layout of LR_P3_Clay's 12 strings, ScratchNormFar 0.85; B23 ClayIceB23 1, PoolDim 0.45, PoolBoost 1, contact 14 cm, board band 400 cm) (32282 bytes) |
| Game/Plugins/JamAssets/Content/Look/M_Zone.uasset | material | 0.05 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | zone strip master (B10, B12) + B19ZoneClay warmer rings (B19) + B22ZoneGlow bright edge and glow spilling on the ice (B22), minimum edge width in pixels and far spill at distance, clay tier only (B23) (45013 bytes) |
| Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap | other | 0.29 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | P3 clay rig, T3 barn: PPV (EV100 4.2, SSR override, focus 16 m f/4, no depth blur, motion blur 0), sky, 4 shadowed warm key spots (hardened), 6 shadowless fills without specular (B23), LookParams (FeltRim 1.4); plank walls, posts, trusses, roof, floor, 12 bulb strings (4 low ones in front of the end walls, B22), 4 lamp shades, all shadowless, no collision (lanes B9, B14, B18, B19, B22, B23) (294972 bytes) |
```

## What device A must know

1. **Handoff `B-to-A-0034` point 1 is no longer needed for the picture.** Rail, stanchions and pane are invisible on `p3_clay` through B's materials. Hiding the `GlassRail` and `Glass` components on clay would still save their draws; it is optional.
2. **The optional red top rail of 0034 is done too**, through `MI_Trim_Blue` / `MI_Trim_Orange`. `look.trim_emissive` (and the tier default 12) has no effect on `p3_clay` now. To get a glow back on clay, set `ClayCapGlow` on the instances, not `Emissive`.
3. `MI_Toon_GlassRail` keeps its name and path; its parent is `M_Toon_Rail`. A change to `M_Toon` no longer reaches the rail.
4. The pane uses `MI_Glass` in A's code. `MI_Toon_Glass` is not loaded by A any more.
5. The new ice path reads `RinkHalfLength`, `RinkHalfWidth` and `CornerRadiusCm` from A's ice instance, as lane B19's snow does. Keep them set per rink (checked on the wide cut: 7000 / 3500, `b23-wide-hi`).
6. Points 2 to 4 of 0034 stay open (Lumen reflections on clay, zones as ring meshes, markings under the ice). Ring meshes are now the only way to the estimate's glowing rings.

## What I could not verify

- Frame cost on any card, and the cooked size or cook of the new master `M_Toon_Rail` (no cook run in this lane; the lead's cook check covers it).
- The saved assets in a fresh editor process. Evidence instead: each save returned true, the dirty list was empty, sizes and times changed on disk, the new names are in the files, and every after-still was taken after the save in the same session.
- Ray-traced or Lumen hardware paths on device A: the masked rail should vanish there too; not seen.
- A live `jam.livecut` switch into and out of clay (the mask and the cap follow `MPC_Look.Clay`; not run).
- TSR at real 60 fps wall time; a motion run from before the changes.
- `hockey.arena.large` (only the wide cut).
- The pool compensation if the rig's key lights are moved: `PoolKeyX` / `PoolKeyZ` on `M_Ice` must follow them by hand.

## What the lead must decide

1. Close or keep `B-to-A-0034` point 1 (optional draw saving only).
2. Whether `look.trim_emissive` being ignored on clay is acceptable, or the designer prompt should say so.
3. The dark reflection band along the side boards: accept it, or a lane that lowers screen-space reflection strength near the boards (the rig's volume or the ice roughness there).
4. `MI_Toon_Glass`: now unused by A's code; delete or keep.
5. Paste the ledger rows; run the cook check before pushing (new master material).

## Stills (`C:/Users/kazda/jam/thirdjam/devlog/shots/`)

- `20261113-034219-B-b23-hero-low.png` (for the operator, beside `est-p3-barn.png`)
- `20261113-034219-B-b23-low-before.png`, `20261113-034219-B-b23-low-after.png`
- `20261113-034219-B-b23-game-before.png`, `20261113-034219-B-b23-game-after.png`
- `20261113-034219-B-b23-boards-ab.jpg`
- `20261113-034538-B-b23-tier-stability.jpg`
- `20261113-034219-B-b23-motion.jpg`
- `20261113-034219-B-b23-zones-far-ab.jpg`
- `20261113-040327-B-b23-side-band-ssr.jpg`, `20261113-040327-B-b23-wide-hi.png`

## Rejected

| Tried | Why rejected |
|---|---|
| World-position offset to collapse the rail | the mask also holds where the offset is ignored and adds no moving-vertex flag on other tiers |
| Lower specular on the key spots | the blob was a fill's mirror image; the keys' highlights were not the fault |
| `PoolBoost` 1.4 | a dense golden curtain behind the orange team at the game camera |
| Board band 450 cm x 5, pale amber | washed the near ice white at the boards view |
| Zone far spill 0.8, minimum 1.8 px, edge centred on the outline | the far ring became a filled pale patch cut square by its quad |
