# B22 report: clay tier pass 4: bulb reflections in the near ice, bulbs in the frame, motion, effects, zones (device B)

Lane B22, device B, machine clock 01:21 to 02:22 (2026-11-13). Main tree `C:/Users/kazda/jam/thirdjam` (branch `dev-b`), editor on port 8000 (pid 36932, mine), started 01:22, left through `ExitEditor` at 02:20:51 (`LogExit: Exiting.`, PIE stopped, `get_dirty_*_packages` empty just before). Checkpoint: `status/B22-checkpoint.md` (after item 1).

Housekeeping:
- No git command, no build. **No frame numbers** (another editor shares the GPU).
- `/Game/Maps/Rink` never saved (mtime still 11-12 11:10, 18,966 bytes). It was loaded once from disk, unchanged, after the rig edit (the rig had to be the editor world to be saved; Rink had no unsaved change: dirty list empty before and after). Nothing under `/Game/` written. `_Scratch/` does not exist. `Game/Saved/Screenshots/WindowsEditor/` emptied (the motion frames were moved to scratch).
- Cvars set back: `jam.Cut hockey.regular`, `jam.ScriptHuman 0`.
- Not touched: `Skaters/`, `Audio/`, `Props/`, `Look/P4Lab/`, `LR_P4_Broadcast`, `LR_L2_Night`, `M_Toon`, `M_Pickup`, `M_Fx_Tier`, `MPC_Look`, `BP_LookParams`.
- 0 `[VSM] Non-Nanite Marking Job Queue overflow` lines in this session's log.
- Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b22` (scripts `mrefl.py` + `refl.hlsl` the reflection, `addpin.py`, `nfar.py`, `zone2.py` + `zone.hlsl`, `rigstr.py`, `motion.py` + `grab.sh` + `ana.py` the motion capture, `fx.sh` the effect strips; every capture under `shots/`, `mo/`, `fx/`; the files as found in `bk/`).

Method: everything in the real game in PIE on `/Game/Maps/Rink` through scratch cartridges (B19's `c_clay`, `c_clayz` zones cut, `c_night`, `c_bcast`, `c_wide`), frozen by `slomo` at t 2.17 (faceoff) or 4.6 (zones cut), captured with `CaptureGameViewport` (1920 x 1085). The analytic term was switched on and off on the live ice MID inside one frozen frame for clean before / after pairs.

## Verdicts

| # | Item | Verdict |
|---|---|---|
| 1 | Bulb reflections in the near ice | **PASS**: long warm streaks of every bulb across the near ice, under the scratches, moving with the camera like real reflections |
| 2 | Bulbs in the frame | **PASS within the camera's geometry**: the dark band above the far boards is now full of bulbs from the faceoff and both ends; the top fifth cannot be filled lower than the far boards' top (15 % from the top at the faceoff) without hanging bulbs in front of the play |
| 3 | In motion | **PASS** after one fix: no shimmer in static regions; far-ice glitter of screen-space reflections during fast turns fixed at the source |
| 4 | Effects and puck | **PASS** effects (white chips and snow); **PARTIAL** puck (found at a glance at 24 and 40 m; hidden behind its carrier from the game camera, as on every tier) |
| 5 | Zones and pickups | **PARTIAL**: zones glow at the edge and spill light onto the ice; pickup bases are opaque emissive already and reflect through screen-space reflections; no analytic pickup reflection (positions are not known to the ice) |
| 6 | Far wall | **NOT DONE** (reason below) |
| | Night and broadcast stability | **PASS**: no leak |

## 1. The bulbs' reflections in the near ice

**`M_Ice`: one Custom node, `B22BulbRefl`** (group `Clay bulb reflection (B22)`), between `B19IceEmis` and the emissive output. It returns `Orig` unless `MPC_Look.Clay >= 0.5`, `ClayIceB19 > 0` and the new switch **`BulbReflB22 > 0`** (master default 0). Feeding it: a `CameraPositionWS` node and a `Transform` (tangent to world) of `B19IceNormal`; B19's `B19IceRough`, `B19IceMask`, `ClayIceSpec`, `RinkHalfLength`.

Per ice pixel:
1. **Mirror.** The view ray is mirrored about the ice normal: B19's scratch normal, flattened by `BRNormalK` 0.35, so the scratches break the streaks without scattering them into dust.
2. **Strings.** Each string (up to 12) hangs in a vertical plane across the rink (run direction `BRYaw`). The mirrored ray meets the string's plane (or, when it runs nearly parallel to it, the string's mid-height layer).
3. **Bulbs.** The three nearest bulbs are tested by their true perpendicular distance to the ray. Their positions follow the mesh's own curve: `z = anchor - BRSag (1 - (y / BRHalfSpan)^2) - BRDrop`, bulbs every `BRPitch` from `BRBulbY0`.
4. **Disc.** Each bulb adds a soft, energy-normalised disc of radius `BRBulbR`. Its angular size grows with roughness (`rough^2 x BRBlurK + BRStreak`) and with the pixel footprint of the unperturbed mirror ray (`fwidth`: the anti-aliasing). It is `BRStretch` times longer along the reflection's vertical than across it, as a rough reflection stretches.
5. **Shading.** Times the ice's own Fresnel (Schlick, F0 = 0.08 x `ClayIceSpec`), times (1 - `BRScratchK` x scratch mask) and (1 - snow): the reflection sits under the scratches and the snow. Clamped at `BRMax`.
6. **Screen-space overlap.** Where the bulb itself is on screen (`ResolvedView.ViewForward/Right/Up` and the projection), the term is lowered by `BRScreenFade`.
   - Measured: screen-space reflections show the on-screen bulbs only as fine sparkle in the scratches, at the same positions as the analytic streaks, so the two never double.
   - With fade 1 the middle ice lost most of its streaks (`shots/cmp_fadez.png`), so I chose 0.3.
7. **Scale.** All lengths scale by `RinkHalfLength / BRLayoutHalfLen`. Checked on the wide cut: MID `RinkHalfLength` 7000, rig scale 1.556, streaks in place (`shots/wide.jpg`).

**Instance values (`MI_Ice_Night`, clay-only):**

| Group | Parameters |
|---|---|
| Switch and layout | `BulbReflB22` 1, `BRLayoutHalfLen` 4500, `BRCount` 12, `BRYaw` 90 |
| String positions and anchors (cm) | `BRStrX_A` (-5250, -3750, -2250, -750), `BRStrX_B` (750, 2250, 3750, 5250), `BRStrX_C` (-5650, 5650, -5850, 5850); `BRStrZ_A` (820, 880, 880, 880), `BRStrZ_B` (880, 880, 880, 820), `BRStrZ_C` (640, 640, 440, 440) |
| String and bulb geometry (cm) | `BRSag` 260, `BRHalfSpan` 3700, `BRBulbY0` -3520, `BRPitch` 185, `BRBulbCount` 39, `BRDrop` 21, `BRBulbR` 16 |
| Look | `BRColour` (1.0, 0.40, 0.11), `BRIntensity` 95, `BRStreak` 0.022, `BRBlurK` 0.5, `BRStretch` 6, `BRNormalK` 0.35, `BRFootK` 1, `BRScreenFade` 0.3, `BRScratchK` 0.6, `BRMax` 40 |

Moving a string in the rig means changing its x or anchor number here.

**Key lamps' discs:** not added. The four key spots already draw their own specular hot spot (source radius 30 cm) exactly where the lamp discs would reflect; an analytic disc would double it.

**Stills:**
- `b22-low-before` / `b22-low-after`: lane B19's low camera (x -17, y -7, z 2.2 m, pitch -10, yaw 25), zones cut, t 4.6.
- `b22-game-before` / `b22-game-after`: game camera at the faceoff, same frozen frame, term off / on.
- `b22-refl-views-ab`: 8 m at the boards, 1.9 m face view, 10 m behind centre.
- `b22-refl-yaw4`: 19, 23, 27, 31 deg yaw at the low camera. The streaks stay attached to their bulbs' azimuths and slide across the ice as the camera turns, as real reflections do.

**Against the estimate `est-p3-barn.png` (same low camera):**
- Reached: the near and middle ice now carry long warm vertical streaks of every string, broken by the scratches, which is the estimate's main promise.
- Not reached:
  - our streaks are thinner and less golden;
  - they wash out inside the bright key pool on the right of the frame, where the estimate keeps them;
  - the estimate's open boards with a red rail (device A's glass and rail);
  - its standing glowing rings with reflections (zones are device A's decals).
- Honestly: about two thirds of the estimate's ice, none of its rings, none of its boards.

## 2. Bulbs in the frame

**Camera geometry (`CameraFollow.cpp`).** The camera stays inside the boards (it slides along the glass, never crosses it), at 3.5 m. `end_height_m` defaults to 3, so the lift never raises it, and it pitches about -17.7 deg. The top of the frame is +3.8 deg above level.
- A bulb at the usual 6 to 9 m enters the frame only beyond about 40 m. Strings "crossing nearer to the camera" would always be above the frame: they show only as reflections, which item 1 now gives.
- Inside the top fifth (+3.8 to -4.8 deg) the far boards' top already sits at about 15 % from the top at the faceoff. So the only free place for bulbs is the dark band between the boards and the old strings: the end walls.

**`LR_P3_Clay` (now 112 actors):**
- `Con_P3_Clay_String_1` and `_8` (x -52.5 / +52.5 m, behind the end boards): anchor 8.8 → **8.2 m**.
- **4 new strings** `Con_P3_Clay_String_9..12` (`SM_Con_ClayBulbString`, same settings as B19's: shadowless, no collision, no distance field, static):
  - x -56.5 / +56.5 m, anchor 6.4 m, sagging to 3.8 m;
  - x -58.5 / +58.5 m, anchor 4.4 m, sagging to 1.8 m.
- All of them stand between the end boards (45 m) and the end walls (60 m): never over the play and never in the camera's path (the camera cannot leave the boards).
- They hang well above the net opening:
  - at the faceoff the lowest row is about 1.5 deg above the far net's top;
  - from the attacking end (camera at 30 m) the lowest bulbs sit about 220 px (about 9 deg) above the net's crossbar in the frame.
- The strings inside the rink keep their lowest bulb at 5.8 m. That is above any default camera. A designer cut with `camera.end_height_m` over 5.8 could reach that height at the end boards, but only near x ±44, which is 6.5 m from the nearest inner string (x ±37.5).
- The reflection parameters were updated to the 12-string layout (above).

**Result (`b22-bulbs-frame-ab`):**
- **Faceoff:** bulbs now fill the whole band above the far boards, top 0 to about 14 % (before: the top 5 %).
- **Attacking end:** three rows over the end wall, top 0 to about 30 % (before: one row in the top 8 %).
- **Looking back from the far end:** the same.

## 3. In motion

**Method.**
- A Python post-tick callback issues `shot` (the back-buffer screenshot) on 24 consecutive rendered frames.
- Game time per frame is held at 1/60 s by an adaptive `slomo` (log: +0.0167 s per frame).
- The renderer, TSR included, sees exactly the per-frame motion of a 60 fps game, only slower in wall time (8 frames a second with the shots).
- One stray editor-viewport shot per run was dropped.

**Four runs:**

| Run | What | Mean \|d\| between consecutive frames |
|---|---|---|
| Static baseline | camera held, game frozen | whole frame 0.22 to 0.25 after one settling frame (0.38) |
| Held game camera | play running, a skater crossing the scratched foreground | streak crop 0.26, bulbs 0.11, boards 0.24, near scratched ice 0.57 (the skater's shadow enters the crop in 4 frames). Temporal std per pixel 0.3 to 0.6 in static crops |
| Following the skater | game camera, two runs | reviewed by eye |
| Fast yaw at the faceoff | 2 deg per frame, 120 deg/s at 60 fps | reviewed by eye |

**Findings.**
- **No shimmer or crawl in static regions:**
  - the bulbs (6 to 9 px at the far end) do not flicker;
  - the board joints and the scratches do not crawl;
  - the analytic streaks are stable frame to frame (24-frame tile in `b22-motion`).
- **No popping:**
  - the reflections settle in one frame after a cut;
  - the depth of field is a fixed focus (16 m), so it cannot hunt.
- **One fault: glitter in the far ice during turns.** In the fast yaw, the far ice's streaks were full of single-pixel sparkles that changed every frame.
  - Isolated in a frozen frame: high-frequency energy in the crop was 5.42 with the analytic term off, 5.58 on, 5.37 with `BRNormalK` 0. **The glitter is lane B19's screen-space reflection of the scratch normal map at grazing angles, not the new term.**
  - **Fixed at the source:** `B19IceNormal` now flattens the scratch normal as the pixel footprint grows past 0.6 cm. New input `NFar` = new parameter **`ScratchNormFar`** (master default 0 = B19's behaviour; `MI_Ice_Night` 0.85).
  - Frozen-frame energy 5.58 → 4.90 (-12 %).
  - In the yaw the dust of sparkles is gone; a few larger glints remain (`b22-motion`, bottom).
  - My frame metrics (sparkle share, motion-aligned residual) did not separate before from after, because the perspective of a yaw is not a pure shift. The judgement is visual.
- **Not changed:** the fine scratches' own fade distance (B19's `ScratchFar`) and the joint lines' filtering (B19's anti-aliased joints): both were stable in the static and held runs.

## 4. The effects and the puck on this look

**Method.** Lane B17's: systems spawned as the binder does, stepped to 0.03 / 0.08 / 0.14 / 0.22 / 0.32 s.
- Camera: 25 m behind at the game camera's 7.3 m and -17.7 deg.
- One burst under a key lamp (x -11.25, y -2.5, blue).
- One in the warm side band (y 13.5, orange; on this rig the side band is a warm brown streaked by the reflections).

**Before.** Chips and snow were grey-brown crumbs, which read as dirt on light ice.

**After.** Instances changed (clay values only):

| Instance | Changes |
|---|---|
| `MI_Fx_Chip` | `ClayTint` (1.0, 0.98, 0.95), `ClayBright` 2.2 → 3.4, `ClayRim` 0.8 → 0.55, `ClayRimCol` (0.30, 0.18, 0.10): a warm shadow side instead of a brown rim |
| `MI_Fx_Puff` | `ClayTint` (1.0, 0.98, 0.95), `ClayBright` 3.0 → 3.6 |

`M_Fx_Tier` is unchanged.

| Effect | Under a key lamp | In the side band |
|---|---|---|
| check (team ring, star, chips) | yes | yes |
| knockdown | yes (white chips, weaker white-on-white) | yes |
| board, skater | yes | yes, loudest |
| poke | barely (a small star by design) | yes |
| shot (streaks) | barely | yes |
| blade spray | barely to yes | yes |

Strip: `b22-fx-ab`.

**Puck.** `MI_Toon_Puck` now sets `ClayBase` (0.010, 0.008, 0.007), `ClayBaseMix` 1: lane B19's clay-only base override, so the body is near-black clay on this tier.
- At the faceoff the core was already dark: minimum 10 against ice 134 at 24 m, and 20 against 152 at 40 m, both before and after.
- **Found at a glance at 24 and 40 m:** the yellow halo, the dark ring and the post read on the light ice.
- **In play (`b22-puck-play`):**
  - from 24 m the carried puck reads by its halo and ring next to the carrier;
  - from the game camera behind the carrier his figure hides it, and the ring alone is not enough (the HUD marker carries it, as on the other tiers).

## 5. Zones and pickups

**`M_Zone`: one Custom node, `B22ZoneGlow`** (group `Clay zone glow (B22)`), after `B19ZoneClay`. It now feeds both component masks (emissive and opacity); it returns its input unless Clay >= 0.5.
- It recomputes the zone's signed edge distance from the UVs, exactly as `ZoneStrip` does, and adds:
  - a bright inner edge (`ZGlowEdge` 2.5, `ZGlowEdgeCm` 7);
  - light spilling inward (`ZGlowSpill` 0.35 over `ZGlowSpillCm` 45);
  - a soft outer glow outside the edge (`ZGlowOut` 0.5), faded to zero at the decal's box so it never cuts hard. For a circle, the glow therefore spills only into the box's corners.
- All values are master defaults on the clay path; no zone instance was touched.
- Result (`b22-zone-glow`): the near zone reads as a warm ring with light on the ice around it. The far zones are still thin flat rings at 40 m.

**Pickup bases.**
- Checked: `M_Pickup` is opaque, default-lit, with an emissive ring, so screen-space reflections already put the bases, tokens and props in the ice when they are on screen (`b22-low-after`).
- An analytic reflection under each pickup is not possible from the ice material: pickup positions change at run time and reach no material.
- `M_Pickup` is unchanged.

## 6. The far wall: not done

- The new end strings (item 2) now fill the band of the end wall that the game camera sees: 2 to 6 m high, above the far boards.
- A row of windows or a lamp at 9 to 11 m would sit above the frame's top (+3.8 deg) from every game camera position.
- Plank gaps would need a wall-mesh or `M_ClayBarn` change for a band that is now mostly covered by bulbs.
- I spent the time on the wide-rink check and the stability check instead.

## Honest remaining distance to `path-p3-t3`

- **Matched:** the barn and the bulbs; warm reflections of the bulbs across the whole ice under the scratches; white chips and snow.
- **Still far:**
  - T3's camera is low and level, so its bulbs fill the upper third; ours stops at the far boards (15 %).
  - T3's rings are 3D glowing tori standing on the ice and reflected; ours are flat decals with a glow.
  - T3 has plain boards with a red top rail and no glass (device A).
  - T3 has felt jerseys with cross-stitches (a skater asset).
  - T3's reflections are golden and wide everywhere; ours wash out inside the key pools.

## Tier stability

Method: lane B19's. Night and broadcast game camera and 24 m stills at 01:24, before any change, and at 02:17, after every save. Same cartridges and frozen frame.

| Still | Mean \|d\| | Tile max | Open-ice crop | Frame luma |
|---|---:|---:|---:|---|
| Night, game camera | 0.98 | 10.5 | 0.61 | 136.05 → 136.05 |
| Night, 24 m | 0.61 | 2.9 | 0.69 | 98.47 → 98.50 |
| Broadcast, game camera | 1.52 | 17.8 | 1.15 | 133.74 → 133.76 |
| Broadcast, 24 m | 0.81 | 9.9 | 1.28 | 94.25 → 94.28 |

- The broadcast crops are above B19's 0.5. The difference map (`shots/dbc.png`, x10) shows only:
  - figure outlines (frozen 2.18 against 2.20);
  - unstructured temporal grain in the broadcast rig's own reflections;
  - no streak pattern, no zone or ice-area change; luma is equal to 0.03.
- Every new path is behind `Clay >= 0.5`. `ScratchNormFar` sits inside `B19IceNormal`'s clay branch, and `B22ZoneGlow` returns its input below 0.5. **No leak.**
- Still: `b22-tier-stability`.

## Expected cost (not measured: shared GPU)

`M_Ice` pixel shader instructions (`get_statistics`, whole material): **1,072 before, 1,302 with the reflection, 1,309 with the normal fade**. The reflection loop is counted once; per clay ice pixel it runs 12 strings x 3 bulbs, about 1,300 ALU. Other tiers pay one uniform branch.

| Addition | RTX 4090, 1080p | Mid-range (RTX 3060 class) |
|---|---|---|
| Analytic bulb reflection, 12 strings | about 0.08 to 0.15 ms | 0.4 to 0.7 ms. If needed: `BRCount` 8 (drop the four end strings: they only reach the far ice) saves a third, and 2 bulbs per string instead of 3 another third (code) |
| Scratch normal footprint fade | negligible | negligible |
| Zone glow (decals only) | negligible | negligible |
| 4 more bulb strings (32k static triangles, emissive, shadowless) | under 0.05 ms | about 0.05 to 0.1 ms. A 6 x 8 bulb sphere halves the string mesh if needed |
| Effect and puck instance values | none | none |

## Assets created or changed (bytes on disk, `Game/Plugins/JamAssets/Content/`)

| File | Before | After |
|---|---:|---:|
| `Rigs/LR_P3_Clay.umap` | 284,918 | 294,776 |
| `Look/M_Ice.uasset` | 97,749 | 121,490 |
| `Look/MI_Ice_Night.uasset` | 22,439 | 28,891 |
| `Look/M_Zone.uasset` | 36,010 | 42,361 |
| `Look/MI_Toon_Puck.uasset` | 8,070 | 8,840 |
| `VFX/MI_Fx_Chip.uasset` | 8,655 | 8,563 |
| `VFX/MI_Fx_Puff.uasset` | 8,139 | 7,796 |

`MI_Fx_Chip` and `MI_Fx_Puff` came out smaller on re-save. I read the values back after the save and they are present.

No asset created.

## Ledger rows (replace these rows)

```
| Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap | other | 0.29 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | P3 clay rig, T3 barn: PPV (EV100 4.2, SSR override, focus 16 m f/4, no depth blur, motion blur 0), sky, 4 shadowed warm key spots (hardened), 6 shadowless fills, LookParams (FeltRim 1.4); plank walls, posts, trusses, roof, floor, 12 bulb strings (4 low ones in front of the end walls, B22), 4 lamp shades, all shadowless, no collision (lanes B9, B14, B18, B19, B22) (294776 bytes) |
| Game/Plugins/JamAssets/Content/Look/M_Ice.uasset | material | 0.12 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-13 | B | y | master material (painted-cel look) + scuff zones, scratches, frost (B13), clay pass 2 / broadcast mode (B14), tiles and NightWet (B15), glossy scratched clay-tier ice B19IceMask/Base/Rough/Normal/Spec/Emis (B19), B22BulbRefl analytic bulb-string reflection and ScratchNormFar footprint fade, Clay >= 0.5 only (B22) (121490 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Ice_Night.uasset | material | 0.03 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-13 | B | y | instance (B13 surface, B14 clay/broadcast, B15 tiles, NightWet 0.3; B19 clay ice; B22 bulb reflection layout of LR_P3_Clay's 12 strings, ScratchNormFar 0.85) (28891 bytes) |
| Game/Plugins/JamAssets/Content/Look/M_Zone.uasset | material | 0.04 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | zone strip master (B10, B12) + B19ZoneClay warmer rings (B19) + B22ZoneGlow bright edge and glow spilling on the ice, clay tier only (B22) (42361 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Puck.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_Toon, black puck body; near-black clay base on the clay tier (ClayBase, ClayBaseMix 1, B22) (8840 bytes) |
| Game/Plugins/JamAssets/Content/VFX/MI_Fx_Chip.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_Fx_Tier, chips and snow (B17); white with a warm shadow side on the clay tier (B22) (8563 bytes) |
| Game/Plugins/JamAssets/Content/VFX/MI_Fx_Puff.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_Fx_Tier, soft snow puffs (B17); white on the clay tier (B22) (7796 bytes) |
```

## Six stills for the operator (`devlog/shots/`)

1. `20261113-015239-B-b22-low-before`: lane B19's low camera, before.
2. `20261113-015239-B-b22-low-after`: the same camera, after: bulb streaks across the near ice (to sit beside `est-p3-barn`).
3. `20261113-015239-B-b22-game-after`: the game camera at the faceoff (`-b22-game-before` is its pair).
4. `20261113-015818-B-b22-bulbs-frame-ab`: bulbs in the frame from the faceoff, the attacking end and looking back.
5. `20261113-021920-B-b22-fx-ab`: the contact effects before and after (white chips).
6. `20261113-020552-B-b22-motion`: 24-frame tile in play, and the fast yaw before and after the glitter fix.

**Also filed:**
- `20261113-015239-B-b22-refl-views-ab`, `20261113-015240-B-b22-refl-yaw4`;
- `20261113-021920-B-b22-zone-glow`, `-b22-tier-stability`;
- `20261113-021921-B-b22-puck-play`;
- `20261113-015239-B-b22-game-before`.

## Rejected

| Tried | Why rejected |
|---|---|
| Full scratch normal in the reflection (`BRNormalK` 1) | the streaks broke into scattered dust |
| Pixel footprint from the perturbed reflection | the scratch normal's per-pixel change made the footprint huge: one flat warm wash, no discs |
| `BRStreak` 0.03 / `BRStretch` 7 / intensity 110 | a solid curtain over the middle ice at the game camera |
| `BRStreak` 0.01 / `BRStretch` 3 | short round blobs, not T3's streaks |
| `BRScreenFade` 1 | erased the middle-ice streaks; screen-space reflections show those bulbs only as sparkle |
| Colour (1, 0.55, 0.22) | read pale yellow-white after tone mapping |
| More strings crossing over the rink nearer the camera | always above the frame from a 3.5 m camera; they would only add reflections |
| A row of windows or a lamp high on the end wall | above the frame from every game camera |
| Analytic reflections under pickups | pickup positions are not available to the ice material |

## What device A must know

1. **`M_Ice` has two new clay-only parameters on `MI_Ice_Night`: `BulbReflB22` and `ScratchNormFar`.** A MID that overrides them would change the clay look. Device A's ice MID today sets only `RinkHalfLength` / `RinkHalfWidth` (read 4500 / 2250 and 7000 / 3500 in PIE), which the reflection uses to follow the rig's scale. **Keep `RinkHalfLength` set per rink.**
2. **The clay rig has 12 bulb strings.** The 4 new ones stand between the end boards and the end walls (x ±56.5 and ±58.5 m), down to 1.8 m above the floor. They are outside the camera's bounds today. A camera mode that leaves the boards (a replay or end-zone camera) on `p3_clay` would meet them.
3. Still open from lane B19 (`handoff/B-to-A-0034.md`): the glass, the rail and the stanchions on clay, and opaque ring meshes for the zones. The estimate's open boards with a red rail and its standing rings are exactly these two.
4. The puck is hidden by its carrier from behind on every tier. The game's HUD marker is what finds it.

## What I could not verify

- Frame cost on any card (shared GPU).
- TSR behaviour at real 60 fps wall time. The motion runs give the renderer 60 fps game steps at about 8 frames a second of wall time, so TSR sees the same per-frame motion but not the same timing.
- A proper motion-compensated shimmer number for the moving-camera runs. They were judged by eye from consecutive crops.
- `hockey.arena.large` (only the wide cut).
- The look on device A's 4090 with Lumen reflections, if the lead ever removes the SSR override: the analytic term would then double the off-screen bulbs and should be switched off with `BulbReflB22` 0.
- A live `jam.livecut` into and out of clay with the new strings.

## What the lead must decide

1. Keep the analytic reflection at 12 strings (cost above) or drop to 8 on mid-range cards.
2. Whether the near-ice streak density (`BRIntensity` 95, `BRStreak` 0.022) is right for play: at the 7.3 m, 25 m view it is a strong curtain; at the game camera it reads as T3.
3. Send device A the B-to-A-0034 requests again (glass and rail hidden on clay; ring meshes for the zones): they are the largest remaining gap to the estimate.
4. Item 6: a far-wall pass only makes sense with a lower or level camera.
5. Paste the ledger rows.
