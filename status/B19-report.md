# B19 report: the clay tier rebuilt against the T3 concept (barn, string lights, glossy scratched ice) (device B)

Lane B19, device B, machine clock 2026-11-12 22:50 to 2026-11-13 00:05. Main tree `C:/Users/kazda/jam/thirdjam` (branch `dev-b`), editor on port 8000 (pid 32212). I started it and left it through `ExitEditor` at 00:01:51 (`LogExit: Exiting.`, PIE stopped, `get_dirty_*_packages` empty just before).

Housekeeping:
- No git command, no build. **No frame numbers** (rule: shared GPU).
- `/Game/Maps/Rink` was never saved or reloaded; its mtime is still 11:10. Nothing under `/Game/` was written. `_Scratch/` does not exist. `jam.Cut` is set back to `hockey.regular`.
- Not touched: `Skaters/`, `Props/`, `VFX/`, `Audio/`, `LR_L2_Night`, `LR_P4_Broadcast`, `MPC_Look`, `BP_LookParams` (the rig's own `LookParams_P3_Clay` instance carries the new value).
- Checkpoint: `status/B19-checkpoint.md` (23:14).
- Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b19`. It holds every script (`mice.py`, `mspec.py`, `mtoon.py`, `zp.py`, `mats.py`, `mesh.py`, `bulbmesh.py`, `rig.py` + `rigp.py`, `mk_ice.py`, `mk_wood.py`, variants `vt1.py` / `vt2.py`), the texture PNGs, every capture under `shots/`, and backups of the files as found in `bk/`.

## Method

- Every still is from the real game in PIE on `/Game/Maps/Rink` through scratch cartridges (copies of `hockey.regular` with only the tier, the visual, zones or pickups changed), using B15's frozen frame:
  - `slomo 0.05`, then 0.0001 at t 2.17 (the faceoff camera);
  - t 8.0 for the game camera in play;
  - t 4.6 on the zones cut, so that the pickups have spawned.
- Captures: `CaptureGameViewport`, 1920 x 1085. Fixed views at 10 / 24 / 40 m behind centre ice (B15's), plus a high corner inside the barn, an 8 m board view and a low pickup view.
- Clay runs show `JAMLOOK tier=p3_clay ... scale=1.000`, Clay 1. The wide cut shows `scale=1.556`.
- Cartridges with zones and pickups: none of the shipped ones has both (`hockey.chaos` has 1 zone; `hockey.boost` has pickups but is `l3_neon` on 120 x 60). So I used `c_clayz.json`:
  - 4 zones: `boost_only`, `fast_ice` and `slow_ice` circles, a `bumper` rect;
  - 4 pickups, `spawn_every_frames` 120 to 135 (the schema minimum is 120, the maximum duration 3600).

## Verdicts

| # | Item | Verdict |
|---|---|---|
| 1 | Ice | **PASS** (motion and TSR shimmer not checked) |
| 2 | Light and barn | **PASS** |
| 3 | Boards, glass, nets | **PASS**, with one A-side limit: the glass rail and stanchions stay, as a dark wooden fence |
| 4 | Zones and pickups | **PARTIAL**: warmer, kinds still distinct, amber pickup rings; the zones cannot reflect (translucent decals) |
| 5 | Figures and depth of field | **PASS** |
| 6 | Variants T2 and T1 (stills, not saved) | **PASS** |
| | Night and broadcast stability | **PASS**: no leak |

### 1. The ice

**What changed (all gated by `if (Clay < 0.5 || ClayIceB19 <= 0) return Orig`, group `Clay ice (B19)`).** `M_Ice` gains six Custom nodes at the end of each output chain:

| Node | Output it feeds | What it does |
|---|---|---|
| `B19IceMask` | float4 for the others | scratch mask: two layers of the scratch map (4 m, and 9.5 m rotated 23 deg) times a 37 m density field, plus a short dense set weighted to the slots, the centre circle and the boards; snow dusting along the boards (rounded-rectangle distance, width 120 cm, noise-broken); soft red and blue line masks from A's own `RinkHalfLength` / `RinkHalfWidth` / `BlueLineFrac` / `GoalLineOffsetCm` |
| `B19IceBase` | base colour | lavender ice body (0.36, 0.38, 0.52); lines 15 % under the ice colour; white scratches on top (lift 0.45); snow |
| `B19IceRough` | roughness | 0.04 between scratches, 0.4 in them, 0.8 in the snow. A's `Wet` (from `wet_ice`) only halves it further |
| `B19IceNormal` | normal | groove and undulation normal from the scratch map, flattened under the snow |
| `B19IceSpec` | specular | 1.0 on clay (F0 0.08, stylised, so reflections read at the 17 deg camera) |
| `B19IceEmis` | emissive | the night ambient emissive times 0, so the clay ice has no flat blue glow |

The tier-owned switch is `ClayIceB19` (instance value 1). A's `Wet` overwrite cannot turn it off.

**New textures (numpy, no image generator, nothing from the references):**
- `T_Ice_Scratch_M`, 1024, masks: R 380 long skate scratches (0.4 to 3.8 m, 60 % within 30 deg of the long axis, curvature radius 2 to 12 m, blade-lift breaks, wide ones as two rails); G 1,100 short scratches; B soft undulation.
- `T_Ice_Scratch_N`, 1024, normal map: grooves plus undulation.

**Instance:** `MI_Ice_Night` sets ClayIceB19 1, SnowAmt 1, ScratchAmt 0.7, ScratchLift 0.45, ClayIceGloss 0.04, ClayIceSpec 1.0, ClayIceBody (0.36, 0.38, 0.52).

**Reflections.** `PPV_P3_Clay` now overrides the reflection method to screen-space reflections (the project default is Lumen): quality 80, max roughness 0.5.
- The far ice shows streaks of the bulb strings, the lamps' hot spots, and the figures standing in the light (`b19-final-10`, `b19-zones-low`).
- **What Lumen reflections would add on device A:** off-screen reflections. These are the bulbs above and behind the camera streaking in the near ice, which is the strongest thing in T3's foreground. They would also add the barn walls along the boards and no fade at the screen edges. Expected cost +0.5 to 0.9 ms on the 4090 (A23 measured up to 0.9 ms on the night ice); too much for a mid-range card.

**Numbers** (faceoff camera, sRGB luma, before → after):

| Area | Before | After |
|---|---:|---:|
| Near ice | 195 | 183 |
| Mid ice under a key | 211 | 126 |
| Side bands by the far boards | – | 33 and 43 |
| Blue jersey | 74 | 92 |
| Puck halo | – | 131 |

The side bands are darker than the night tier's 80, but they are a warm brown, so a blue or orange figure reads against them by hue and value.

**Puck readability** (puck core minimum against the surrounding ice):

| Distance | Before | After |
|---|---|---|
| 24 m | 109 against 142: blurred into the ice | 12 against 125 |
| 40 m | puck not found | 15 against 142 |

For comparison, SK5's night readability was a 4 to 5 px black puck against 50 in the dark band.

**Tiling and aliasing:**
- No visible tiling at the game camera, at 24 m or at 40 m (two layers, rotated, plus the density field).
- The scratches are mip-filtered texture, not procedural lines, and fade by screen derivative past 0.6 cm per pixel.
- Not checked in motion.

Stills: `20261112-231332-B-b19-ck-before-game` / `-ck-ice-game`, `20261112-235501-B-b19-before-{10,24,40}`, `20261112-235502-B-b19-final-{game,10,24}`, `20261112-235503-B-b19-final-40`.

### 2. The light and the barn

**`LR_P3_Clay` rebuilt: 108 actors.** The B14 table, paper sweep, light stands, directional key and three fills are gone.

| Prefix | Count | What |
|---|---:|---|
| `PPV` | 1 | exposure unchanged (EV100 4.2 fixed); SSR as above; depth of field (item 5); motion blur 0 kept |
| `Sky` | 1 | 0.35, warm |
| `LookParams` | 1 | `felt_rim` 1.0 → 1.4 |
| `Key` | 4 | `Key_P3_Clay_1..4`: the **only shadow casters**, spot lights at x ±11.25 / ±33.75 m, z 13 m, 4800 cd, cones 16/58 deg, attenuation 30 m, sRGB (255, 214, 172). Hardened as the night spots: shadow resolution 0.5, ray-traced shadows disabled, contact shadow 0, no volumetric shadow |
| `Fill` | 6 | shadowless, no translucency lighting, sRGB (255, 208, 160): two side fills aimed at the near boards (2500 cd, 80 deg), two end fills (2000 cd), two wall washes on the end walls (5000 cd) |
| `Con` | 95 | listed below; every one shadowless, no collision, no distance field, static |

**`Con` actors and triangles:**

| Actors | Mesh | Triangles |
|---|---|---:|
| 1 floor (top -3 cm) | engine cube | 12 |
| 34 plank walls at x ±60 m, y ±37 m (scaled to 13.5 m) | `SM_Con_ClayWall` | 588 each |
| 2 gables | `SM_Con_ClayGable` | 3,180 each |
| 34 posts with knee braces | `SM_Con_ClayPost` | 36 each |
| 10 trusses at the 13.5 m eave | `SM_Con_ClayTruss` | 72 each |
| 2 roof slabs | engine cube | 12 each |
| **8 bulb strings across the rink**, wall to wall, hung at 8.8 m and sagging to 6.2 m, 39 bulbs (16 cm) each | `SM_Con_ClayBulbString` | 8,064 each |
| 4 lamp shades with a glowing disc over the keys (T2's lamps) | `SM_Con_ClayLampShade` | 224 each |

Total: about **93,700 static triangles**, of which 64,500 are bulbs.

**Bulbs:**
- They are emissive only (`M_ClayBulb`, unlit, Glow 9, amber edge `EdgeK` 0.7, so the core is warm white and the rim orange).
- The ice reflects them through SSR where they are on screen.
- The warm-to-dark falloff toward the walls comes from the key cones and the fills.

**Constraints checked:**
- Nothing is inside the boards.
- From the game camera the strings appear only in the top 5 % of the frame at the far end, above the far net. The net top is at -2.3 deg; the strings are at +2.5 deg or higher.
- On `hockey.arena.wide` (scale 1.556) the barn, the strings (9.6 to 13.7 m) and the lamps scale with the rink, and nothing sits on the ice (`b19-wide-hi`, `b19-wide-game`).
- **0 `[VSM] Non-Nanite Marking Job Queue overflow`** in this session's roughly 15 clay starts. The rig no longer has a directional light, so the clipmap invalidation B18 suspected is gone on this tier (not proven to be the cause).

**Flicker under TSR:** not verified in motion. Bulbs are 6 to 9 px at the far end of the game camera, with a soft unlit edge, so they are not sub-pixel.

Stills: `b19-ck-hi`, `b19-final-hi`, `b19-final-40`, `b19-wide-hi`.

### 3. Boards, glass, nets

**`M_Toon`** gains `B19ToonBase` (group `Clay (B19)`), which returns `Orig` unless Clay ≥ 0.5:
- `ClayBase` / `ClayBaseMix`: a clay-only base override.
- `B19Board`: painted panels on the board faces, with a dark joint every `PanelCm` 244 cm (2.5 cm, anti-aliased, fading out when sub-pixel, so no shimmer at 40 m) and ±6 % panel-to-panel tone. The thumbprint is B14's existing dent normal.

**Instances (clay-only values):**
- `MI_Toon_Board`: B19Board 1; `ClayFeltColour` (0.55, 0.07, 0.05), band 14 cm, which gives the red top band under the trim.
- `MI_Toon_KickPlate`: yellow (0.85, 0.60, 0.08).
- `MI_Toon_GlassRail`: dark wood (0.10, 0.06, 0.035).
- Nets: unchanged (red frame).

**Glass:** `M_Glass` gains `B19GlassClay` (on clay, colour and opacity × `ClayGlassK`). `MI_Toon_Glass` sets 0.12, so the glass is nearly invisible on this tier.

**Limit:** A's rail and stanchions remain as a thin dark fence at 2.1 m (`b19-final-board`). See "What device A must know".

### 4. Zones and pickups

At Clay 1 the zones were cream-white discs (emissive 8 clipped against the bright ice) and the pickup bases were white rings (`b19-zones-before-24`).

**`M_Zone`** gains `B19ZoneClay` after `ZoneStrip`: the hue is pulled toward amber, the band and pattern are brightened, and the flat fill is made fainter.
- Kept at warm 0.3, band 0.4, fill 0.5. At warm 0.65 and band 1.8, every kind clipped to the same cream ring and kind identity was lost.

**`M_Pickup`** gains `B19PickupClay` (warm 0.5, glow 0.6 for tokens).
- `MI_Pickup_Base` overrides warm 1.0, glow 0.18 and amber (1, 0.45, 0.12), so the base ring reads amber instead of white.

**Result:** coloured rings that read by kind and warm amber pickup rings. The pickups and their props reflect in the ice (`b19-zones-low`).

**Not reached:** T3's rings glow and reflect. The zones are A's translucent unlit decals, and screen-space reflections never include translucency.

Stills: `b19-zones-24`, `b19-zones-game`, `b19-zones-low`.

### 5. Figures on the tier

- `generated_b2` shows `BP_SkaterVisual_TB2_P3` (P3C).
- Blue and orange stay distinct under the warm light: blue jersey luma 92 against warm ice 126 to 183, orange saturated red-orange.
- **Felt:** `FeltRim` 1.0 → 1.4 (through the rig's `LookParams_P3_Clay`). 1.8 washed the blue toward pastel (`b19-felt-ab`). The felt reads as a soft lighter edge, not as fibre (SKI2's texture limit).
- Figure reflections show faintly under the lit figures (SSR, F0 0.08).

**Depth of field (`PPV_P3_Clay`, not scaled):** focus 11 → **16 m**, f/3.2 → **f/4**, depth blur 0.06 / 6 px → **0**.

| Point | Before (px) | After (px) |
|---|---:|---:|
| Foreground | 2.8 | about 2.9 |
| Far net | 2.3 + 2.9 | about 1.1 |

The rink is now sharp from the player to the far net with a soft foreground, as in the concepts. Puck numbers are under item 1. Still: `b19-dof-40-ab`.

### 6. Variants (stills only, PIE memory, nothing saved)

- **T2** (`vt2.py`):
  - the barn hidden over the same ice and boards, a dark void (sky 0.05) and the wall washes off;
  - four warm additive light cones spawned under the key lamps (MID of `MI_HazeCard_End`, warm, `SideFade` 0);
  - stills: `b19-variant-t2-24`, `-t2-game`.
  - The cone bases put a stack of translucency over the ice (a visible disc where each cone meets it): this is a picture, not a shippable setting.
- **T1** (`vt1.py`):
  - the barn and the rig lights off, the persistent directional light as a 14 lux sun, the rig sky light at 2 (cool);
  - one barn actor re-used as a painted sky sweep (MID of `MI_Clay_Backdrop`), the floor pale;
  - stills: `b19-variant-t1-game`, `-t1-24`, `-t1-hi`.
  - Exposure kept at EV100 4.2.

## Honest side-by-side with the concepts

**T3 (the target).**
- **Matches:**
  - a dark plank barn with posts and trusses, and strings of warm bulbs sagging across the top of the frame;
  - glossy, finely scratched ice with bulb and lamp streaks in the far ice;
  - cream plaster boards with panel joints, a yellow kick strip and a red band;
  - warm, close light falling to dark at the walls; the far net clear;
  - figures in two strong team colours, and amber pickup rings.
- **Still far:**
  - *Near-ice reflections.* T3's near ice is full of warm reflections of bulbs above and behind the camera. SSR cannot see them (technique: Lumen reflections on A, or a planar reflection, which is too costly).
  - *Glowing tori and capsules.* T3's are 3D, standing on the ice and reflected. Ours are A's flat translucent decals plus B10's bases and props (technique and ownership).
  - *Felt.* T3's jerseys are hairy needle felt with thick cream cross-stitches. P3C is smooth clay with thin stitches (asset: SKI2's texture or mesh).
  - *Camera.* T3's camera is about 1.5 m high and level, so bulbs fill the upper third. Our game camera is 3.5 m high at -17.7 deg and shows only the far strings (game design, not changed).
  - *Top rail.* T3 has a plain red top rail and no glass. Ours has A's team trim glowing on top and a dark rail fence at 2.1 m (A's construction).
  - *Ice colour.* T3's ice is lighter and pinker overall. Ours is lavender-grey under warm light, a little darker so that reflections show (time).

**T2.** The variant gets the dark void, warm lamp cones and pools on the same ice.
- Far: the cones are additive cards with a hard foot instead of volumetric light. Real volumetric fog costs 0.3 to 0.5 ms (C-0002).
- T2's boards carry a lit strip; ours carry the trims.

**T1.** The variant reads as an outdoor daylight rink with glossy blue-white scratched ice and a pale sky.
- Far: no pines, clay snowbanks, painted clouds or mountains (time: they would need meshes and a painted backdrop). The sky is a plain gradient.

## Tier stability (night Clay 0, broadcast Clay -1)

Method: one game-camera still and one fixed 24 m still of each tier before any change (22:53) and after the last save (23:58), through the game, same seed and frozen frame.

| Still | Mean \|d\| whole frame | Tile max | Open-ice crop mean \|d\| | Frame luma |
|---|---:|---:|---:|---|
| Night, game camera | 2.08 | 27.8 | 0.40 | 135.70 → 135.75 |
| Night, 24 m | 0.81 | 8.7 | 0.42 | 98.43 → 98.42 |
| Broadcast, game camera | 0.98 | 10.7 | 0.26 | 167.42 → 167.48 |
| Broadcast, 24 m | 0.45 | 5.6 | 0.26 | 132.42 → 132.40 |

- The two numbers over 1 level are figure-edge motion: frozen t 2.205 against 2.179. The difference map (`scratch/shots/diff_night_game.jpg`) shows only the outlines of the moving figures, and no area change.
- Every area crop is under 0.5. **No leak.**
- Every new path is behind `Clay ≥ 0.5`. The only uniform branch the other tiers execute is that test.

Stills: `b19-tier-{night,bcast}-{before,after}`.

## Expected cost per addition (not measured: shared GPU)

| Addition | RTX 4090, 1080p | Mid-range card (RTX 3060 class) |
|---|---|---|
| `M_Ice` clay path: 7 texture samples and about 150 ALU over the full-screen ice. Other tiers pay one uniform branch (the compiled pixel shader lists 1,071 instructions after the patch, most not executed on other tiers; the count before was not recorded) | about 0.05 to 0.1 ms | 0.2 to 0.3 ms |
| SSR at quality 80 on this tier, replacing Lumen reflections (which the project default would have run) | about 0.2 ms; a net saving against Lumen reflections | 0.5 to 0.8 ms |
| 4 shadowed key spots (VSM), replacing 1 directional with clipmaps. B1c measured the night rig's 4 spots at 0.84 to 1.0 ms `ShadowDepths` on device B | 0.4 to 0.6 ms (about the same as the night tier) | 1 to 1.5 ms |
| 6 shadowless fills (large radii) | about 0.2 to 0.3 ms | 0.6 to 0.9 ms |
| 93.7k static triangles, non-Nanite, shadowless (312 bulbs are 64.5k: a 6 x 8 sphere halves that if needed) | under 0.1 ms | 0.1 to 0.2 ms |
| Barn triplanar wood material (3 samples) on mostly dark, partly covered walls | under 0.05 ms | about 0.1 ms |
| Zone, pickup and glass branches | negligible | negligible |
| Depth blur off | a small saving | a small saving |

## Assets created or changed (bytes on disk, `Game/Plugins/JamAssets/Content/`)

| File | Before | After |
|---|---:|---:|
| `Rigs/LR_P3_Clay.umap` | 135,667 | 284,918 |
| `Look/M_Ice.uasset` | 72,494 | 97,749 |
| `Look/M_Toon.uasset` | 53,164 | 60,521 |
| `Look/M_Glass.uasset` | see ledger row | 23,477 |
| `Look/M_Zone.uasset` | see ledger row | 36,010 |
| `Look/M_Pickup.uasset` | see ledger row | 38,438 |
| `Look/MI_Ice_Night.uasset` | 19,110 | 22,439 |
| `Look/MI_Toon_Board.uasset` | 14,144 | 15,462 |
| `Look/MI_Toon_GlassRail.uasset` | 8,927 | 9,375 |
| `Look/MI_Toon_KickPlate.uasset` | 8,981 | 10,372 |
| `Look/MI_Toon_Glass.uasset` | 7,025 | 10,670 |
| `Look/MI_Pickup_Base.uasset` | 10,476 | 12,279 |
| `Look/M_ClayBarn.uasset` | new | 17,902 |
| `Look/M_ClayBulb.uasset` | new | 12,967 |
| `Look/MI_ClayBarn_Wall.uasset` | new | 10,240 |
| `Look/MI_ClayBarn_Post.uasset` | new | 9,937 |
| `Look/MI_ClayBarn_Floor.uasset` | new | 9,123 |
| `Look/MI_ClayBarn_Roof.uasset` | new | 8,399 |
| `Look/MI_ClayBulb_Warm.uasset` | new | 9,085 |
| `Look/MI_ClayBulb_Lamp.uasset` | new | 8,139 |
| `Look/Textures/T_Ice_Scratch_M.uasset` | new | 1,579,328 |
| `Look/Textures/T_Ice_Scratch_N.uasset` | new | 1,432,190 |
| `Look/Textures/T_Clay_WoodGrain_M.uasset` | new | 229,982 |
| `Arena/SM_Con_ClayWall.uasset` | new | 28,703 |
| `Arena/SM_Con_ClayGable.uasset` | new | 84,506 |
| `Arena/SM_Con_ClayPost.uasset` | new | 14,527 |
| `Arena/SM_Con_ClayTruss.uasset` | new | 15,111 |
| `Arena/SM_Con_ClayBulbString.uasset` | new | 86,474 |
| `Arena/SM_Con_ClayLampShade.uasset` | new | 19,105 |

`MI_Toon_GlassRail`, `_KickPlate` and `_Glass` "before" sizes are B15's numbers. Later lanes may have re-saved them.

**Now unreferenced by any rig** (not deleted; the lead decides): `MI_Clay_Table`, `MI_Clay_Wire`, `MI_Clay_Card`, `MI_Clay_Lens` (B14). `MI_Clay_Backdrop` is used only by the T1 variant. `M_ClayBackdrop` stays: it is the parent of `MI_Bcast_Volume`.

## Ledger rows

**Replace** the `LR_P3_Clay` row. **Append** "+ clay-only paths (B19)" notes to the rows of `M_Ice`, `M_Toon`, `M_Glass`, `M_Zone`, `M_Pickup`, `MI_Ice_Night`, `MI_Toon_Board`, `MI_Toon_GlassRail`, `MI_Toon_KickPlate`, `MI_Toon_Glass`, `MI_Pickup_Base`, as below. **Add** the rest.

```
| Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap | other | 0.28 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | P3 clay rig, T3 barn: PPV (EV100 4.2, SSR override, focus 16 m f/4, no depth blur, motion blur 0), sky, 4 shadowed warm key spots (hardened), 6 shadowless fills, LookParams (FeltRim 1.4); plank walls, posts, trusses, roof, floor, 8 bulb strings, 4 lamp shades, all shadowless, no collision (lanes B9, B14, B18, B19) (284918 bytes) |
| Game/Plugins/JamAssets/Content/Look/M_Ice.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | master material (painted-cel look) + scuff zones, scratches, frost (B13), clay pass 2 / broadcast mode (B14), tiles and NightWet (B15), glossy scratched clay-tier ice: B19IceMask/Base/Rough/Normal/Spec/Emis, Clay >= 0.5 and ClayIceB19 only (B19) (97749 bytes) |
| Game/Plugins/JamAssets/Content/Look/M_Toon.uasset | material | 0.06 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | master material (painted-cel look) + Construct (B13), clay pass 2 / broadcast mode (B14), CelConstructGate (B15), B19ToonBase: clay-only base override and painted board panels with joints (B19) (60521 bytes) |
| Game/Plugins/JamAssets/Content/Look/M_Glass.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | unlit translucent rink glass (B13), HitWobble (SK5), B19GlassClay: nearly invisible on the clay tier (B19) (23477 bytes) |
| Game/Plugins/JamAssets/Content/Look/M_Zone.uasset | material | 0.04 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | zone strip master (B10, B12) + B19ZoneClay: warmer rings, fainter fill on the clay tier only (B19) (36010 bytes) |
| Game/Plugins/JamAssets/Content/Look/M_Pickup.uasset | material | 0.04 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | pickup master (B10, B12) + B19PickupClay: warm glow on the clay tier only (B19) (38438 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Ice_Night.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance (B13 surface, B14 clay/broadcast, B15 tiles, NightWet 0.3; B19 clay ice: ClayIceB19 1, gloss 0.04, spec 1, lavender body, snow) (22439 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Board.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance; kick plate and rail band (B13-B15); clay: panels with joints every 2.44 m, red band (B19) (15462 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_GlassRail.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance of M_Toon, near-black rail and stanchions (B15); dark wood on the clay tier (B19) (9375 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_KickPlate.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance of M_Toon, ochre kick plate (B15); yellow on the clay tier (B19) (10372 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Glass.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | child of MI_Glass for A's Glass component (B15); ClayGlassK 0.12 (B19) (10670 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Pickup_Base.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | pedestal, ring glow (B10); amber ring on the clay tier (B19) (12279 bytes) |
| Game/Plugins/JamAssets/Content/Look/M_ClayBarn.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | barn wood master: world triplanar grain, dents, roughness (lane B19) (17902 bytes) |
| Game/Plugins/JamAssets/Content/Look/M_ClayBulb.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | unlit emissive bulb master with a soft amber edge (lane B19) (12967 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayBarn_Wall.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance of M_ClayBarn, plank walls (lane B19) (10240 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayBarn_Post.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance of M_ClayBarn, posts, trusses, wire, shades (lane B19) (9937 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayBarn_Floor.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance of M_ClayBarn, floor (lane B19) (9123 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayBarn_Roof.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance of M_ClayBarn, roof (lane B19) (8399 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayBulb_Warm.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance of M_ClayBulb, string bulbs Glow 9 (lane B19) (9085 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayBulb_Lamp.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance of M_ClayBulb, lamp discs (lane B19) (8139 bytes) |
| Game/Plugins/JamAssets/Content/Look/Textures/T_Ice_Scratch_M.uasset | texture | 1.58 | original | Python script (numpy, scratchpad/b19/mk_ice.py) |  |  | 2026-11-12 | B | y | procedural tileable skate-scratch masks, 1024 = 4 m: R long scratches, G short dense, B undulation; masks compression (lane B19) |
| Game/Plugins/JamAssets/Content/Look/Textures/T_Ice_Scratch_N.uasset | texture | 1.43 | original | Python script (numpy, scratchpad/b19/mk_ice.py) |  |  | 2026-11-12 | B | y | procedural tileable scratch-groove and undulation normal map, 1024, BC5 (lane B19) |
| Game/Plugins/JamAssets/Content/Look/Textures/T_Clay_WoodGrain_M.uasset | texture | 0.23 | original | Python script (numpy, scratchpad/b19/mk_wood.py) |  |  | 2026-11-12 | B | y | procedural tileable wood grain, tone and dent masks, 512 (lane B19) |
| Game/Plugins/JamAssets/Content/Arena/SM_Con_ClayWall.uasset | mesh | 0.03 | original | UE 5.9 GeometryScript via MCP/Python |  |  | 2026-11-12 | B | y | 12 x 9 m vertical-plank wall panel with two girts, 588 tris, no collision (lane B19) |
| Game/Plugins/JamAssets/Content/Arena/SM_Con_ClayGable.uasset | mesh | 0.08 | original | UE 5.9 GeometryScript via MCP/Python |  |  | 2026-11-12 | B | y | 74 m plank gable, 3180 tris, no collision (lane B19) |
| Game/Plugins/JamAssets/Content/Arena/SM_Con_ClayPost.uasset | mesh | 0.01 | original | UE 5.9 GeometryScript via MCP/Python |  |  | 2026-11-12 | B | y | 9 m barn post with knee braces, 36 tris (lane B19) |
| Game/Plugins/JamAssets/Content/Arena/SM_Con_ClayTruss.uasset | mesh | 0.02 | original | UE 5.9 GeometryScript via MCP/Python |  |  | 2026-11-12 | B | y | 74 m king-post roof truss, 72 tris (lane B19) |
| Game/Plugins/JamAssets/Content/Arena/SM_Con_ClayBulbString.uasset | mesh | 0.09 | original | UE 5.9 GeometryScript via MCP/Python |  |  | 2026-11-12 | B | y | 74 m sagging wire with 39 bulbs (16 cm), slots wire / bulb, 8064 tris (lane B19) |
| Game/Plugins/JamAssets/Content/Arena/SM_Con_ClayLampShade.uasset | mesh | 0.02 | original | UE 5.9 GeometryScript via MCP/Python |  |  | 2026-11-12 | B | y | lamp shade with glowing disc and 4 m rod, 224 tris (lane B19) |
```

## Six stills for the operator (`devlog/shots/`)

1. `20261112-235502-B-b19-final-game`: the clay tier through the game, faceoff camera.
2. `20261112-235901-B-b19-zones-low`: low camera on the zones cut, the nearest to T3's framing: bulbs, barn, pickups and figures reflected in the ice.
3. `20261112-235502-B-b19-final-10`: 10 m, bulb reflections across the far ice.
4. `20261112-235503-B-b19-final-board`: the boards, the scratched ice and the snow dusting at 8 m.
5. `20261112-235503-B-b19-variant-t2-24`: T2 setting on the same ice (variant).
6. `20261112-235504-B-b19-variant-t1-game`: T1 daylight on the same ice (variant).

**Before:** `20261112-231332-B-b19-ck-before-game`, `20261112-235501-B-b19-before-{10,24,40}`.

**Also filed:**
- `b19-ck-*`, `b19-final-{24,40,hi,play}`;
- `b19-zones-{before-24,24,game}`;
- `b19-variant-{t2-game,t1-24,t1-hi}`;
- `b19-tier-*`, `b19-felt-ab`, `b19-dof-40-ab`, `b19-wide-{hi,game}`.

## Rejected

| Tried | Why rejected |
|---|---|
| Ice body (0.45, 0.52, 0.60) at gloss 0.08 | the diffuse swamped every reflection |
| Ice body (0.62, 0.68, 0.78) | clipped to white, scratches gone |
| Strongly warm keys (255, 188, 128) | turned the ice orange-brown |
| Keys 4500 cd / 48 deg | side bands at luma 23 |
| Keys 6500 cd / 62 deg | flattened the pools |
| Trusses every 12 m from x -60 at a 9 m eave | a truss and king post stood 2 m in front of the 40 m camera; the trusses moved and the eave rose to 13.5 m |
| Bulb Glow 30 and 18 | white cores, no amber |
| 6 x 8 bulb spheres | faceted at close range |
| Zone warm 0.65 / band 1.8 | every kind became the same clipped cream ring |
| Pickup glow 1.6 | white rings |
| FeltRim 1.8 | pastel blue |
| Collapsing the glass rail by world-position offset in `M_Toon` | adds WPO to every `M_Toon` user, a virtual-shadow-map cache risk on every tier |
| Lumen reflections on this tier | the brief: SSR is what this machine has |

## What device A must know

1. **One instance per part limits this tier.** The clay look comes only from master paths reading `MPC_Look.Clay`, and opaque parts cannot vanish.
   - Ask: in `SetGlassForTier` (or the construction build), **hide `Glass`, the glass rail and the stanchions on `p3_clay`.** The concepts have none. Today they show as a dark wooden fence at 2.1 m.
   - Optional: a red top-rail colour on clay instead of the team trim (`look.trim_emissive` 12 keeps the trims glowing).
2. **`wet_ice` → `Wet`:** the clay ice ignores it except as extra polish. The tier switch is the instance's `ClayIceB19`. Nothing to change.
3. **Markings:** A's construction markings (faceoff circles, dots, crease, trapezoid; `Markings` sections 2 and 3, `BasicShapeMaterial` MIDs) sit on top of the ice as flat opaque paint. B's soft lines are under the surface, so the two differ on clay. If A wants them under the ice too, B can paint them in `M_Ice` from the same numbers.
4. **Zones** are translucent decals and cannot appear in screen-space reflections. T3's glowing rings would need opaque emissive ring meshes (A's spawn, B's material).
5. **Shadow casters:**
   - the rig adds **0** shadow-casting meshes;
   - its 4 key spots shadow A's non-Nanite procedural boards, glass, nets, skaters, puck, and the 120 x 60 temporary floor (B18 suggested `CastShadow = false` on that floor);
   - the clay rig has **no directional light any more**.
6. **Contract C-0002:** the `p3_clay` row says "directional key; the local-light scaling leaves it alone".
   - The keys are now 4 spot lights `Key_P3_Clay_1..4`, so the switcher scales them by s² and their attenuation by s. That is correct for a uniformly scaled rig.
   - The note needs the Director's update; no code change.
7. **Large rinks:** at 1.556 the pools cover the centre; the last 15 to 20 m at each end are darker (the same shape as the night rig, B1c).

## What I could not verify

- Frame cost on any card: shared GPU, no numbers by rule.
- Lumen reflections on the 4090.
- Motion: TSR shimmer of the scratches, the bulbs and the board joints, and the reflections in motion. Stills only.
- `hockey.arena.large` (only the wide cut was run).
- A live `jam.livecut` switch into and out of clay with the new rig. The `BP_LookParams` mechanism is unchanged.
- Readability of the puck in a real far-end carry (only the faceoff puck was measured).
- The felt at close range under the new light. SKI2's material is unchanged; only the rim value moved.

## What the lead must decide

1. **Setting:** T3 barn (saved), or one of the variants (T2 void with cones, T1 daylight) from the stills. Each would need its own pass.
2. **Send device A** the requests above: glass, rail and stanchions hidden on `p3_clay`; optionally opaque ring meshes for zones; markings under the ice; the C-0002 note.
3. **Lumen reflections on `p3_clay`** for device A (remove the SSR override in `PPV_P3_Clay`), if the budget allows. This is the largest single step toward T3's foreground.
4. **The old B14 clay instances**, now unreferenced: delete or keep.
5. **The figure felt:** a texture or mesh lane for bold cream stitches and a hairy jersey (SKI2's open item). The material path is ready.
6. Paste the ledger rows.
