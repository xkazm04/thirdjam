# B12 report: the operator's picked images in the engine, jersey numbers and crests (device B)

Lane B12, 2026-11-12, 15:05 to 15:50 local. Tree `C:/Users/kazda/jam/thirdjam` (dev-b), no git command run, nothing built.

**Editor:** port 8000, started on the existing binaries with plain `editor.ps1`. No rebuild modal came up; MCP answered after 14 s.

**Other editor:** the one on port 8002 was up, so no frame numbers were taken.

**Rink:** `/Game/Maps/Rink` was never saved (`Rink.umap` mtime is still 11:10). All test actors lived in memory only. They were discarded by opening a blank map, then Rink was reopened from disk. The editor is left on Rink with no dirty map and no dirty content package.

**Untouched:** `Rigs/` and `Arena/` (verified: no file there is newer than my start). There is no `_Scratch/`.

Scratch folder: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b12` (`$B12` below).
- Processed PNGs: `$B12/out/`, with crop logs in `log.json` and `zones.json`.
- Image prep scripts: `prep.py`, `zones.py`.
- Engine scripts: `deco_lib.py` (the shared HLSL), `m1_tb1.py`, `m2_tb2.py`, `m3_vi.py`, `m5_pickup.py`, `m6_zone.py`, `m7_backdrop.py`, `m8_fix.py`, `m9.py`, `b_TB1.dsl`, `b_TB2.dsl`, `bpw.mjs`, `setdef.py`.
- Test-scene and capture scripts: `v1_setup.py`, `v2.py`, `v3.py`, `d0.py`, `s*.py`, `z_discard.py`.

## Verdicts

| # | Part | Verdict | Stills (`devlog/shots/`) |
|---|---|---|---|
| 1 | Import all twelve at the n1 sizes with the texture rules; look again for letters or logos | **PASS** | `20261112-154032-B-b12-keyed.png`, `-154032-B-b12-tiles.png` |
| 2 | Jersey numbers on T-B1 and T-B2, `SetNumber` live | **PASS** at 6 m. At 30 m the number reads as a white mark (one digit or two), but the digits do not resolve. | `-154029-B-b12-numbers-6m.png`, `-154029-B-b12-numbers-6m-frame.png`, `-154030-B-b12-numbers-30m.png`, `-154030-B-b12-numbers-30m-frame.png` |
| 3 | Crests by `TeamId`, outside the hue key | **PASS**, with two placement notes (below) | `-154030-B-b12-crests.png`, `-154030-B-b12-crests-frame.png` |
| 4a | Glyph input on `M_Pickup`; five instances wired | **PASS**. Glyphs read up close (4 m); at 10 m the kind is carried by colour. | `-154031-B-b12-glyphs.png` |
| 4b | Tile input on `M_Zone`; tiles tried on three instances | **PASS**. Kept on `fast_ice` only. `slow_ice` and `bumper` are left exactly as lane B10 had them. | `-154031-B-b12-zones-10m.png`, `-154031-B-b12-zones-30m.png` |
| 5 | Backdrop flat: import plus `M_BackdropFlat` / `MI_BackdropFlat_Hills` at Brightness 4.5, not placed | **PASS** | `-154031-B-b12-backdrop.png` |

All stills come from PIE plus `CaptureGameViewport` on an in-memory stand-in:
- a 90 x 45 m plane with `MI_Ice_Night` and stencil 2;
- an unbound PPV with `MI_PP_ToonRamp` and `MI_PP_Outline`, manual exposure, bias -1.5;
- two spot lights 25 m up;
- two `BP_SkaterVisual_TB1` and two `BP_SkaterVisual_TB2`, driven from Python as lane B4 did it.

The night rig was **not** loaded.

## 1. Per image: what was done, final size and compression

Every image was looked at again at full size. Nothing letter-like or logo-like is in any of them except the digits on the number sheet. The reel has sprocket holes, the clapperboard has stripes, and the tin has a spoke pattern on its lid.

The keying method is the same for all keyed images:
- Alpha is a soft ramp on the RGB distance from the measured flat background.
- The alpha is then pulled in by one pixel (`min(a, 3x3 mean)`) to remove the fringe.
- The RGB of every pixel that is not fully opaque is replaced by the colour of the nearest opaque pixel (distance transform). This leaves no magenta, grey or green in the fringe; 0 magenta pixels remain on the crests.

Sizes were only ever reduced, never upscaled.

| Engine texture | Source | Done (numpy/PIL, `prep.py`, `zones.py`) | Final px | Compression |
|---|---|---|---|---|
| `Look/Textures/T_Backdrop_Hills` | backdrop-a-hills (2172 x 724) | Downscaled to 2048 x 512 (x0.943 across, x0.707 down). The material shows it on a 3:1 surface, so it un-squashes. This keeps it power of two with mips, and uses no upscale. | 2048 x 512 | BC1 (`TC_Default`, NoAlpha), Lossy Medium, MaxTextureSize **2048** (set by hand: the rules script would cap `Look/` at 1024, so the backdrop was skipped in the script), clamp |
| `Look/Textures/T_Crest_Blue_Comet` | crest-blue-comet (1254) | Magenta key, ramp 40..140, fringe pass; square crop around the disc (x 0, y 4, 1249 px), 2 % margin | 512 | BC7, Medium, clamp, never stream |
| `Look/Textures/T_Crest_Orange_Fox` | crest-orange-fox (1254) | Same; crop (51, 53, 1148 px) | 512 | BC7, Medium, clamp, never stream |
| `Look/Textures/T_Numbers_Varsity` | numbers-b-varsity (2172 x 724) | Grey key, ramp 14..45, fringe pass. The ten digits were cut as connected components and scaled by 0.728 (largest digit 243 x 289 px). Each was centred in a **5 x 2 atlas** with at least 14 px of transparent padding. Colour was bled into the transparent area so mips do not halo. | 1024 x 512 | BC7, Medium, clamp, never stream |
| `Look/Textures/T_Glyph_Speed` | glyph-speed-b | Background key (47, 60, 53), ramp 10..32, fringe pass; square crop with 5 % margin. The holes in the reel are transparent. | 256 | BC7, Medium, clamp, never stream |
| `Look/Textures/T_Glyph_Precision` | glyph-precision-b | Same | 256 | same |
| `Look/Textures/T_Glyph_HeavyPuck` | glyph-heavy-a | Same | 256 | same (see the build fault under "What went wrong") |
| `Look/Textures/T_Glyph_BigPuck` | glyph-big-a | Same | 256 | same |
| `Look/Textures/T_Glyph_Reverse` | glyph-reverse-b | Same | 256 | same |
| `Look/Textures/T_Zone_SlowIce` | zone-slow-b | Crop searched for the best wrap: x 2..1253, y 0..1253; then 1024. **A mild seam remains**: wrap 6.0 against 4.5 between neighbouring columns (left-right) and 10.3 against 7.8 (top-bottom). The image has no period, so no crop removes it. | 1024 | BC1, NoAlpha, Medium |
| `Look/Textures/T_Zone_FastIce` | zone-fast-b | Crop x 0..1253, y 1..1252; then 1024. It already tiled: wrap 2.2 against 0.7, and 5.7 against 6.7. | 1024 | BC1, NoAlpha, Medium |
| `Look/Textures/T_Zone_Bumper` | zone-bumper-b | Cropped to a whole number of chevron periods: the full width, rows 95..1137 (1042 rows). Resized to 1024 x 1024, a slight horizontal squash (x 0.817, y 0.983). The top-bottom seam went from **48.0 to 3.2** (neighbour 2.5); left-right is 2.3 against 2.3. | 1024 | BC1, NoAlpha, Medium |
| `UI/T_Crest_Blue_Comet`, `UI/T_Crest_Orange_Fox` | same PNGs | UI copies: `TEXTUREGROUP_UI`, no mips, never stream | 512 | BC7, Medium |
| `UI/T_Numbers_Varsity` | same PNG | UI copy | 1024 x 512 | BC7, Medium |
| `UI/T_Glyph_Speed`, `_Precision`, `_HeavyPuck`, `_BigPuck`, `_Reverse` | same PNGs | UI copies | 256 | BC7, Medium |

Applied to all: virtual texture off, sRGB on, LOD bias 0.
- The rules script (`b8/apply_import_rules.py`, with `SKIP = ["T_Backdrop_Hills"]`) changed exactly my 19 textures and no other texture.
- The `Look/` copies of the crests, numbers and glyphs are set to never stream. They are small, and a streamed-out mip was one suspect in the glyph fault.
- `Lossy Medium` still has no effect until device A sets `bFinalUsesRDO=True` (handoff A-to-B-0004).

**For device A's UI**, the `/JamAssets/UI/` paths:

```
/JamAssets/UI/T_Crest_Blue_Comet.T_Crest_Blue_Comet        team 0 (blue) badge, 512, alpha
/JamAssets/UI/T_Crest_Orange_Fox.T_Crest_Orange_Fox        team 1 (orange) badge, 512, alpha
/JamAssets/UI/T_Numbers_Varsity.T_Numbers_Varsity          scoreboard digit atlas, 1024 x 512, alpha
/JamAssets/UI/T_Glyph_Speed.T_Glyph_Speed                  speed
/JamAssets/UI/T_Glyph_Precision.T_Glyph_Precision          precision
/JamAssets/UI/T_Glyph_HeavyPuck.T_Glyph_HeavyPuck          heavy_puck
/JamAssets/UI/T_Glyph_BigPuck.T_Glyph_BigPuck              big_puck
/JamAssets/UI/T_Glyph_Reverse.T_Glyph_Reverse              reverse_other
```

Layout of the digit atlas (both copies):
- 5 columns x 2 rows. Digit `d` sits at column `d % 5`, row `d / 5`: 0 to 4 on the top row, 5 to 9 on the bottom.
- Each cell is 204.8 x 256 px, so in UV the cell origin is `((d % 5) / 5, (d / 5) / 2)` and its size is `(0.2, 0.5)`.
- Each digit is centred in its cell. The digits are white with a navy outline, slanted varsity; set two digits about 0.8 of a cell width apart.

## 2. Jersey numbers

**Shared code:** one Custom HLSL node, `JerseyDeco (lane B12)`, with the same code in both masters (`deco_lib.py`). I used a Custom node rather than a material function because both masters are already built from Custom nodes. It outputs float4 (rgb, alpha):
- **Number:** two slots are drawn from the atlas with `Texture2DSampleGrad`, so there is no mip jump at cell edges. If `NumberTens` is 0, only one slot is drawn, centred: there is no leading zero. The slots are composited and masked to back-facing surfaces with `PreSkinnedNormal.x < NumberBackMask` (the figure faces +X).
- **Crest:** projected along X onto front-facing surfaces (`PreSkinnedNormal.x > 0.3`), centred at (`CrestY`, `CrestZ`) in pre-skinned local cm. It picks `CrestTeam0` (comet) or `CrestTeam1` (fox) by the `CrestTeam` scalar.
- **Pre-skinned inputs:** `PreSkinnedPosition` and `PreSkinnedNormal` are only available in the vertex shader, so they go through Vertex Interpolators. Because they are pre-skinned, the decal stays fixed to the jersey through every animation.

**T-B1 (scripted figure):**
- New master `Skaters/M_SkaterTB1Jersey`: a duplicate of `M_Toon` with `used_with_skeletal_mesh` on (`M_Toon` itself is untouched). The decal is mixed in before the clay base node (`Orig`) and, scaled by `DecoGlow`, into the rim and emissive node in place of `TeamColour`.
- New instance `Skaters/MI_SkaterTB1_JerseyDeco`, with the values of `MI_SkaterTB1_Jersey` (Emissive 2.0, RimStrength 0.6, blue).
- The number uses **UV0**, which I measured on the mesh as u = 0.025·y + 0.5 and v = 4.267 − 0.0333·z. So the box is 40 cm across the back and 30 cm from z 128 down to z 98. v runs downwards and nothing is mirrored.
- `BP_SkaterVisual_TB1` now creates its Jersey dynamic instance (slot 1) **from `MI_SkaterTB1_JerseyDeco`**. The mesh's own slot still holds `MI_SkaterTB1_Jersey`; `SK_SkaterTB1` was not mine to change.

**T-B2 (generated figure, one texture):**
- **No second UV channel was added.** Instead, `M_SkaterTB2` projects a box onto the back by pre-skinned object-space position: `NumberUseUV0` = 0, box 40 x 30 cm centred at y 0, z 96. The figure is a chibi with a short torso (z about 70 to 110) and a slanted back, so the number is set smaller: height 0.45 of the box, width 0.40.
- The decal is mixed **after** the hue key and the glove key (after `Custom_2`), into both base and emissive. The crest and the number are therefore never recoloured; the swap still proves this.
- The clay node's `Base` input still takes the undecorated colour. That input only drives the clay sheen.

**Blueprints:** in both, `SetNumber(N)` and `SetTeam(TeamId, Colour)` are live.
- New variables: `bHasNumber`, `PendingNumber`, `PendingTeamId`.
- `SetNumber` clamps N to 0..99, stores it, and on a valid instance sets `NumberTens` = N / 10, `NumberOnes` = N % 10 and `NumberOn` = 1.
- `SetTeam` also sets `CrestTeam` = clamp(TeamId, 0, 1).
- BeginPlay re-applies the stored number and team, so a call made before BeginPlay works. Verified: `SetTeam(1)` and `SetNumber(42 / 5)` on the editor actors, then PIE gives Tens 4, Ones 2 (and 0, 5) with CrestTeam 1.
- `NumberOn` defaults to 0, so **no number is drawn until `SetNumber` is called**.
- Both Blueprints compile `BS_UP_TO_DATE`.

**Verified (stills `b12-numbers-6m` and `-30m`):**
- Numbers 1, 7, 13, 42 and 99 on both figures and both teams.
- At 6 m all are clean, with single digits centred.
- On T-B2 the right mitt covers part of the number in Idle and Stride (for example "1" and "13"). This depends on the pose.
- At 30 m the frame is 1920 wide and each digit is about 3 to 4 px: you can tell a number is there and whether it has one digit or two, not which digits.

## 3. Crests

- **T-B1:** 12 cm, at y −9 and z 117 (figure's left chest, under the collar). In a glide the shoulder pad (slot `Pads`, not the jersey) clips its top edge.
- **T-B2:** 11 cm, at y −3 and z 101, **centre-left** rather than left chest. In the glide and idle poses the figure's left arm crosses the left chest; at y −13 the crest was hidden (seen in a debug capture).
- On the blue team the comet's blue disc sits on a blue jersey. It reads through its navy ring and the white comet.
- Still `b12-crests`, bottom row: the swap with TeamId against colour shows the comet on orange and the fox on blue. **The crest follows `TeamId`, and the hue key leaves it alone.**

## 4. Glyphs and zone tiles

**`M_Pickup`:** new node `GlyphLayer (lane B12)` plus two mix nodes. Nothing else in the master changed.
- The glyph is drawn on both faces of the token (local ±X, `|LP.x| > GlyphFaceDepth`). The back face is mirrored so the glyph reads the right way round from either side.
- It uses the pre-WPO local position, so it spins with the token.
- It replaces the base colour and is added into emissive (× `GlyphEmissive`).
- Tinting: I compared untinted against tinted (luminance × `PickupColour`) and GlyphEmissive 3 against 1. **Chosen: untinted, GlyphEmissive 1.0.** At 3 the amber parts of the glyphs blow out to white; tinted, the glyph reads only as an engraving.
- All five `MI_Pickup_<kind>` have `GlyphOn` 1 and their `GlyphTex`. `MI_Pickup_custom` and `MI_Pickup_Base` are unchanged (GlyphOn defaults to 0).
- **Honest read:** clear up close; at 10 m only a darker mark on the coloured token, so colour stays the cue. The token's 16 cm hub sits on top of the glyph's centre.

**`M_Zone`:** new inputs on the existing `ZoneStrip` Custom node, plus a texture parameter and four scalars.
- In the code, one `if (TileOn > 0.5)` block composites the tile over the fill and under the pattern ink, the pattern colour, the band and the rim. With TileOn 0 the output is unchanged.
- The tile is laid out in world metres (`TileScale` m per repeat). For `Pattern` 1 it scrolls with the fast-ice streaks.
- Stills at 10 m and 30 m, procedural against tiled. Settings: TileAlpha 0.5, TileEmissive 3, 2 m repeat.

| Instance | Verdict |
|---|---|
| `slow_ice` | Better at 10 m (frost under the hatch); **the same at 30 m**. Left as it was. |
| `fast_ice` | Better at both. Speed streaks at 10 m; at 30 m the tiled zone is darker and clearly different from `slow_ice`, which was lane B10's closest pair. **Kept**: `TileTex` T_Zone_FastIce, TileOn 1, TileAlpha 0.5, TileEmissive 3, TileScale 2. |
| `bumper` | **Worse.** The zigzag fills the interior and competes with the outward edge chevrons. Left as it was. |

The master's default `TileTex` is `T_Zone_FastIce`, so the unused tiles are not pulled in as defaults.

## 5. Backdrop flat

- `Look/M_BackdropFlat`: Unlit, two-sided. Emissive = `BackdropTex` × `Brightness` (default 4.5).
- `Look/MI_BackdropFlat_Hills`: `T_Backdrop_Hills`, Brightness 4.5.
- Still: a 60 x 20 m stand-in plane at x −90 m in the dark. It shows the right 3:1 aspect and a dark painted night with readable stars, and is not black.
- **Placement note for the rig lane:** the still was taken from the plane's back side, so it is mirrored (the moon belongs at the right). Put the front face toward the rink, or flip U.
- Not placed in any rig.

## New parameters and assets

- **`M_SkaterTB1Jersey` and `M_SkaterTB2`** (group `Deco`; defaults T-B1 / T-B2):
  - Number position and size: `NumberTens` 0, `NumberOnes` 7, `NumberOn` 0, `NumberCentreU` 0.5, `NumberCentreV` 0.5, `NumberWidth` 0.54 / 0.40, `NumberHeight` 0.9 / 0.45, `NumberPitch` 0.8, `NumberBackMask` −0.3.
  - Projection box: `NumberUseUV0` 1 / 0, `NumberBoxY` 0, `NumberBoxZ` 113 / 96, `NumberBoxW` 40, `NumberBoxH` 30.
  - Crest: `CrestTeam` 0, `CrestY` −9 / −3, `CrestZ` 117 / 101, `CrestSize` 12 / 11, `CrestOn` 1.
  - Glow: `DecoGlow` 0.6 / 1.0.
  - Textures: `NumberAtlas`, `CrestTeam0`, `CrestTeam1`.
- **`BP_SkaterVisual_TB1` and `_TB2`:** variables `bHasNumber`, `PendingNumber`, `PendingTeamId`.
- **`M_Pickup`** (group `Glyph`): `GlyphTex`, `GlyphOn` 0, `GlyphFaceDepth` 2.5, `GlyphSize` 34, `GlyphTint` 0, `GlyphEmissive` 1.0.
- **`M_Zone`** (group `Tile`): `TileTex`, `TileOn` 0, `TileAlpha` 0.5, `TileScale` 2, `TileEmissive` 3.
- **`M_BackdropFlat`:** `BackdropTex`, `Brightness` 4.5.

## Files (bytes on disk), all under `Game/Plugins/JamAssets/Content/`

Created (24):

| File | Bytes |
|---|---|
| `Look/Textures/T_Backdrop_Hills.uasset` | 1,143,811 |
| `Look/Textures/T_Crest_Blue_Comet.uasset` | 262,979 |
| `Look/Textures/T_Crest_Orange_Fox.uasset` | 278,930 |
| `Look/Textures/T_Numbers_Varsity.uasset` | 438,485 |
| `Look/Textures/T_Glyph_Speed.uasset` | 88,866 |
| `Look/Textures/T_Glyph_Precision.uasset` | 65,586 |
| `Look/Textures/T_Glyph_HeavyPuck.uasset` | 97,868 |
| `Look/Textures/T_Glyph_BigPuck.uasset` | 63,688 |
| `Look/Textures/T_Glyph_Reverse.uasset` | 73,870 |
| `Look/Textures/T_Zone_SlowIce.uasset` | 1,760,685 |
| `Look/Textures/T_Zone_FastIce.uasset` | 1,411,281 |
| `Look/Textures/T_Zone_Bumper.uasset` | 1,293,304 |
| `UI/T_Crest_Blue_Comet.uasset` | 263,729 |
| `UI/T_Crest_Orange_Fox.uasset` | 279,489 |
| `UI/T_Numbers_Varsity.uasset` | 440,065 |
| `UI/T_Glyph_Speed.uasset` | 88,005 |
| `UI/T_Glyph_Precision.uasset` | 66,807 |
| `UI/T_Glyph_HeavyPuck.uasset` | 98,421 |
| `UI/T_Glyph_BigPuck.uasset` | 63,126 |
| `UI/T_Glyph_Reverse.uasset` | 68,436 |
| `Skaters/M_SkaterTB1Jersey.uasset` | 52,087 |
| `Skaters/MI_SkaterTB1_JerseyDeco.uasset` | 8,270 |
| `Look/M_BackdropFlat.uasset` | 11,492 |
| `Look/MI_BackdropFlat_Hills.uasset` | 8,546 |

Changed (11). "Before" is the last size reported by B4, B8 or B10; the clay lane may have changed `M_SkaterTB2` since.

| File | Before | After |
|---|---|---|
| `Skaters/M_SkaterTB2.uasset` | 27,598 | 64,130 |
| `Skaters/BP_SkaterVisual_TB1.uasset` | 312,403 | 413,300 |
| `Skaters/BP_SkaterVisual_TB2.uasset` | 287,855 | 388,134 |
| `Look/M_Pickup.uasset` | 27,189 | 34,769 |
| `Look/MI_Pickup_speed.uasset` | 7,228 | 9,126 |
| `Look/MI_Pickup_precision.uasset` | 7,355 | 9,257 |
| `Look/MI_Pickup_heavy_puck.uasset` | 7,318 | 9,176 |
| `Look/MI_Pickup_big_puck.uasset` | 7,291 | 9,200 |
| `Look/MI_Pickup_reverse_other.uasset` | 7,477 | 9,339 |
| `Look/M_Zone.uasset` | 25,822 | 31,631 |
| `Look/MI_Zone_fast_ice.uasset` | 8,417 | 13,525 |

Also: this report, and 12 stills (`devlog/shots/20261112-1540*-B-b12-*.png`, each with a `.txt` caption).

**Shipped-size estimate** (arithmetic, not a cook; BC1 0.5 B/px, BC7 1 B/px, × 4/3 for mips):
- `Look/` textures: about 4.6 MB.
- `UI/` copies: about 1.4 MB.
- Total about 6.0 MB before RDO.

## Ledger rows (columns as in lane B10's report)

`size_mb` is the estimate above until a cook.

```
| Game/Plugins/JamAssets/Content/Look/Textures/T_Backdrop_Hills.uasset | texture | 0.7 | ai-generated | Codex image generation |  | C:/Users/kazda/jam/image-lab/jobs.json#backdrop-a-hills | 2026-11-12 | B | y | 2048x512 BC1 Medium; downscaled from 2172x724, shown 3:1 by M_BackdropFlat (lane B12) |
| Game/Plugins/JamAssets/Content/Look/Textures/T_Crest_Blue_Comet.uasset | texture | 0.35 | ai-generated | Codex image generation |  | C:/Users/kazda/jam/image-lab/jobs.json#crest-blue-comet | 2026-11-12 | B | y | 512 BC7 Medium; magenta keyed, defringed; team 0 jersey crest (lane B12) |
| Game/Plugins/JamAssets/Content/Look/Textures/T_Crest_Orange_Fox.uasset | texture | 0.35 | ai-generated | Codex image generation |  | C:/Users/kazda/jam/image-lab/jobs.json#crest-orange-fox | 2026-11-12 | B | y | 512 BC7 Medium; magenta keyed, defringed; team 1 jersey crest (lane B12) |
| Game/Plugins/JamAssets/Content/Look/Textures/T_Numbers_Varsity.uasset | texture | 0.7 | ai-generated | Codex image generation |  | C:/Users/kazda/jam/image-lab/jobs.json#numbers-b-varsity | 2026-11-12 | B | y | 1024x512 BC7 Medium; 5x2 digit atlas for NumberTens/NumberOnes (lane B12) |
| Game/Plugins/JamAssets/Content/Look/Textures/T_Glyph_Speed.uasset | texture | 0.09 | ai-generated | Codex image generation |  | C:/Users/kazda/jam/image-lab/jobs.json#glyph-speed-b | 2026-11-12 | B | y | 256 BC7 Medium; pickup glyph (lane B12) |
| Game/Plugins/JamAssets/Content/Look/Textures/T_Glyph_Precision.uasset | texture | 0.09 | ai-generated | Codex image generation |  | C:/Users/kazda/jam/image-lab/jobs.json#glyph-precision-b | 2026-11-12 | B | y | 256 BC7 Medium; pickup glyph (lane B12) |
| Game/Plugins/JamAssets/Content/Look/Textures/T_Glyph_HeavyPuck.uasset | texture | 0.09 | ai-generated | Codex image generation |  | C:/Users/kazda/jam/image-lab/jobs.json#glyph-heavy-a | 2026-11-12 | B | y | 256 BC7 Medium; pickup glyph (lane B12) |
| Game/Plugins/JamAssets/Content/Look/Textures/T_Glyph_BigPuck.uasset | texture | 0.09 | ai-generated | Codex image generation |  | C:/Users/kazda/jam/image-lab/jobs.json#glyph-big-a | 2026-11-12 | B | y | 256 BC7 Medium; pickup glyph (lane B12) |
| Game/Plugins/JamAssets/Content/Look/Textures/T_Glyph_Reverse.uasset | texture | 0.09 | ai-generated | Codex image generation |  | C:/Users/kazda/jam/image-lab/jobs.json#glyph-reverse-b | 2026-11-12 | B | y | 256 BC7 Medium; pickup glyph (lane B12) |
| Game/Plugins/JamAssets/Content/Look/Textures/T_Zone_SlowIce.uasset | texture | 0.7 | ai-generated | Codex image generation |  | C:/Users/kazda/jam/image-lab/jobs.json#zone-slow-b | 2026-11-12 | B | n | 1024 BC1 Medium; tried under MI_Zone_slow_ice, not kept (unreferenced) (lane B12) |
| Game/Plugins/JamAssets/Content/Look/Textures/T_Zone_FastIce.uasset | texture | 0.7 | ai-generated | Codex image generation |  | C:/Users/kazda/jam/image-lab/jobs.json#zone-fast-b | 2026-11-12 | B | y | 1024 BC1 Medium; tile under MI_Zone_fast_ice (lane B12) |
| Game/Plugins/JamAssets/Content/Look/Textures/T_Zone_Bumper.uasset | texture | 0.7 | ai-generated | Codex image generation |  | C:/Users/kazda/jam/image-lab/jobs.json#zone-bumper-b | 2026-11-12 | B | n | 1024 BC1 Medium; cropped to whole chevrons; tried under MI_Zone_bumper, not kept (unreferenced) (lane B12) |
| Game/Plugins/JamAssets/Content/UI/T_Crest_Blue_Comet.uasset | texture | 0.26 | ai-generated | Codex image generation |  | C:/Users/kazda/jam/image-lab/jobs.json#crest-blue-comet | 2026-11-12 | B | y | UI copy, 512 BC7, no mips; for device A's HUD/menu (lane B12) |
| Game/Plugins/JamAssets/Content/UI/T_Crest_Orange_Fox.uasset | texture | 0.26 | ai-generated | Codex image generation |  | C:/Users/kazda/jam/image-lab/jobs.json#crest-orange-fox | 2026-11-12 | B | y | UI copy, 512 BC7, no mips (lane B12) |
| Game/Plugins/JamAssets/Content/UI/T_Numbers_Varsity.uasset | texture | 0.52 | ai-generated | Codex image generation |  | C:/Users/kazda/jam/image-lab/jobs.json#numbers-b-varsity | 2026-11-12 | B | y | UI copy, 1024x512 BC7, no mips; scoreboard digit atlas 5x2 (lane B12) |
| Game/Plugins/JamAssets/Content/UI/T_Glyph_Speed.uasset | texture | 0.07 | ai-generated | Codex image generation |  | C:/Users/kazda/jam/image-lab/jobs.json#glyph-speed-b | 2026-11-12 | B | y | UI copy, 256 BC7, no mips (lane B12) |
| Game/Plugins/JamAssets/Content/UI/T_Glyph_Precision.uasset | texture | 0.07 | ai-generated | Codex image generation |  | C:/Users/kazda/jam/image-lab/jobs.json#glyph-precision-b | 2026-11-12 | B | y | UI copy, 256 BC7, no mips (lane B12) |
| Game/Plugins/JamAssets/Content/UI/T_Glyph_HeavyPuck.uasset | texture | 0.07 | ai-generated | Codex image generation |  | C:/Users/kazda/jam/image-lab/jobs.json#glyph-heavy-a | 2026-11-12 | B | y | UI copy, 256 BC7, no mips (lane B12) |
| Game/Plugins/JamAssets/Content/UI/T_Glyph_BigPuck.uasset | texture | 0.07 | ai-generated | Codex image generation |  | C:/Users/kazda/jam/image-lab/jobs.json#glyph-big-a | 2026-11-12 | B | y | UI copy, 256 BC7, no mips (lane B12) |
| Game/Plugins/JamAssets/Content/UI/T_Glyph_Reverse.uasset | texture | 0.07 | ai-generated | Codex image generation |  | C:/Users/kazda/jam/image-lab/jobs.json#glyph-reverse-b | 2026-11-12 | B | y | UI copy, 256 BC7, no mips (lane B12) |
| Game/Plugins/JamAssets/Content/Skaters/M_SkaterTB1Jersey.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | n | M_Toon copy + JerseyDeco: number from UV0 back, crest by CrestTeam (lane B12) |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB1_JerseyDeco.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | n | jersey instance used by BP_SkaterVisual_TB1 at BeginPlay (lane B12) |
| Game/Plugins/JamAssets/Content/Look/M_BackdropFlat.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | unlit two-sided flat, BackdropTex x Brightness (lane B12) |
| Game/Plugins/JamAssets/Content/Look/MI_BackdropFlat_Hills.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | T_Backdrop_Hills, Brightness 4.5 (lane B12) |
```

Notes to add on existing rows:
- `M_SkaterTB2`: "JerseyDeco after the hue key: number by pre-skinned box projection on the back, crest by CrestTeam (lane B12)".
- `BP_SkaterVisual_TB1` and `BP_SkaterVisual_TB2`: "SetNumber live (NumberTens, NumberOnes, NumberOn); SetTeam sets CrestTeam (lane B12)".
- `M_Pickup`: "optional GlyphTex layer (lane B12)".
- `MI_Pickup_<kind>` (five rows): "glyph on (lane B12)".
- `M_Zone`: "optional TileTex layer, TileOn 0 = unchanged (lane B12)".
- `MI_Zone_fast_ice`: "T_Zone_FastIce tile on (lane B12)".

T-B1 rows stay `shipped = n` as in B4.

## What went wrong

**1. PreSkinned inputs in the pixel shader.** The first compile of both masters failed: `PreSkinnedPosition` is not available in the pixel shader. In that PIE run T-B2 rendered with the default material. Fixed with Vertex Interpolators.

**2. Missing skeletal usage flag.** The `M_Toon` duplicate lacked `used_with_skeletal_mesh`, so the T-B1 jersey rendered black. Fixed by setting the flag.

**3. Two textures did not finish building.** `Look/T_Glyph_HeavyPuck` and `UI/T_Glyph_Reverse` stayed at the 32 x 32 placeholder ("contains no miplevels"). The heavy glyph rendered as a grey block.
- Toggling `lossy_compression_amount` was not enough for the heavy glyph. A delete-and-recreate also failed: the delete took the asset out of the registry, but the object stayed in memory and the file stayed on disk.
- A fresh import over the same name, followed by toggling `compression_settings` (Default, then BC7), finally forced a real rebuild. It now renders correctly (still `b12-glyphs`).
- `MI_Pickup_heavy_puck` was pointed at the UI copy for a few minutes during this and points back at `Look/T_Glyph_HeavyPuck` now.
- `UI/T_Glyph_Reverse` reports 256 after its toggle, but was not seen in a widget.

**4. My cleanup prefix deleted the PPV.** The `B12_P` prefix in the cleanup also removed `B12_PPV`, so it was re-added. This affected only in-memory test actors.

**5. Saves did not stick.** `save_loaded_asset` left about 20 of my packages dirty; it looks like rebuilds re-dirtied them. I saved them with `save_packages` after checking that every dirty package was mine.

## What I could not verify

- **Look and camera:** the real game camera, rink and night rig. The stand-in lights and exposure are mine, not `LR_L2_Night` (EV100 4.2), so brightness and contrast in the real rig may differ; the glyph and decal glow values most of all.
- **Frame cost:** not measured, because another editor shared the GPU.
- **Cooked sizes:** shipped sizes and the effect of `Lossy Medium` need a cook and `bFinalUsesRDO`.
- **UI copies:** none was seen in a UMG widget.
- **Poses:** the numbers in falls, get-ups and shots. Only Idle, Glide and Stride were seen, in stills; no video.
- **Clay tier:** the look tier with Clay > 0 on the new jersey master.
- **Heavy glyph after a restart:** the fix rendered in this session and was saved; the editor was not restarted to confirm it persists.
- **Backdrop in the rig:** not seen at its real placement (that is the rig lane's job).

## For the lead to decide

1. **T-B1 jersey material.** Assign `MI_SkaterTB1_JerseyDeco` to `SK_SkaterTB1` slot 1 permanently; the mesh was not in my write list. Until then the number and crest exist only after BeginPlay.
2. **T-B2 crest position.** Accept centre-left, or have the hybrid's arm pose or the crest moved to the sleeve (needs a sleeve projection).
3. **Unused tiles.** `T_Zone_SlowIce` and `T_Zone_Bumper` were imported and tried but are unreferenced: about 1.4 MB cooked and 3 MB of source in `Content/`, which the ledger gate counts. Keep them for later, or delete them.
4. **Duplicate textures for UI.** The crest, glyph and number copies in `UI/` cost about 1.4 MB cooked. If device A can use the `Look/` copies (they have mips and the World group), the `UI/` copies can go.
5. **Glyph read at range.** Glyphs read only close up. A flat token face without the 16 cm hub, or a camera-facing glyph billboard above the token, would read further. Colour carries the kind at 10 m.
6. **`bFinalUsesRDO=True`** in device A's config is still needed for `Lossy Medium` to do anything.
7. **Device A must call `SetNumber`** for any number to show; nothing is drawn by default.
