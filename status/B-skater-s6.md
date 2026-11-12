# B lab S6: skater texture cleanup (glyph-like marks removed)

Verdict: **done for the texture; in-engine look not verified.** Cleaned 2048 x 2048 PNG: `C:\Users\kazda\jam\skater-lab\s6\T_SkaterTB2_BaseColor_clean.png`
(3.0 MB; original extracted next to it as `T_SkaterTB2_BaseColor_orig.png`, from the no-TAPose Rodin GLB, task c07557c7). The Meshy rig carries the same texture and UVs, so the same PNG applies.

## Where the marks are (texture px, 2048 square, origin top-left; mesh is 1.7 m, centred, up = Z)
The UV table is made from the mesh (faces by centroid height z): lower legs, boots and blades are z < -0.20; blades and runner caps are z < -0.68. `uv_locator_bands.png` shows the bands.
1. **Lace tiles on the boot instep (the "shin" scribbles; three dark horizontal bars per boot, they read as an M/W in the crouch).** They are not drawn into big islands: about 160 thin UV strips
   (about 10 x 40..90 px each, z -0.55..-0.60, navy with a darker near-black navy ring) e.g. x 379..391 y 1238..1320; x 400..409 y 1826..1906; x 1294..1309 y 82..164; x 1357..1370 y 1480..1557;
   x 371..413 y 256..273; x 1282..1341 y 807..820; x 1317..1360 y 1660..1674; x 2..15 y 1565..1615.
2. **Blade side pills** (white islands, z about -0.77..-0.79) with a row of 4 black dots or a blurred grey dotted smear: x 374..694 y 482..578 (smear, plus 643: x 393..729 y 578..669);
   x 1430..1524 y 1030..1355 (4 dots); x 1537..1629 y 1254..1578 (4 dots); x 1443..1537 y 1649..1972 (smear); x 1979..2019 y 1026..1121; x 1570..1662 y 574..882; x 463..822 y 703..877.
3. Not touched on purpose: black runner end caps and long black runner bars (e.g. x 138..459 y 279..329, x 1168..1485 y 320..374, x 1675..1763 y 1383..1434, x 7..99 y 454..506), eyes, helmet vents, chest and sleeve stripes.
Note: the white skate "side" the brief expected to be dark is white here; its marks are the dots and smears of item 2. The skate boots are navy, not black.

## Method (`clean_texture.py`, Blender 4.2 Python + numpy, nothing else)
Only pixels inside the UV raster of faces with z < -0.20 (969k px) can change; all else is byte-identical (checked: 0 changed px outside the mask).
- **Lace tiles**: every UV island in z -0.68..-0.20 that is at most 30 px thin and 160 px long and has a navy median colour is painted flat with the boot navy (median of broad navy pixels = rgb 0.071, 0.165, 0.475);
  plus the dark navy fringe within 5 px of each tile (it bleeds into the render through texture filtering). Also navy pixels darker than 0.8 x boot navy in small clusters (bbox <= 400 px), via `candD`.
- **Dark stroke on dark surround** (candA): luminance below the grayscale closing (R = 13 px) by > 0.02, closing < 0.35, 8 px blur of L < 0.28, cluster bbox <= 400 px (clusters grouped by 5 px dilation); only z > -0.68.
- **Near-black blob on bright surround** (candB, blade dots): diff > 0.07, L < 0.10, bright 8 px blur (> 0.65), cluster bbox <= 60 px.
- **Grey smear on white** (candC): diff > 0.07, closing > 0.75, saturation < 0.12, L < 0.8, bbox <= 220 px.
- Protected: any neutral near-black structure (L < 0.15, sat < 0.2) at least 110 px long (runner bars, outlines), dilated by 6 px, is never changed.
- Mask dilated by 4 px, filled by normalised Gaussian blur (sigma 5, 14, 40) using only same-class neighbours (navy for A, white for B/C); leftover navy -> boot navy. All parameters are the constants at the top of the script.
- Pads and boots were not repainted wholesale; only the lace tiles are flat colour (the boot navy was already flat).

## Before / after evidence (filed in `devlog/shots/`, sources in `skater-lab\s6\`)
- `20261112-120213-B-lab-s6-closeups.png` (`sheet_closeups_before_over_after.png`): 8 azimuths, top = before, bottom = after. Laces, dots, smears gone; black runner end caps intact.
- `20261112-120213-B-lab-s6-turntable.png`: 8-view turntable before (top) and after (bottom): nothing else moved.
- `20261112-120213-B-lab-s6-diff.png` (`diff_overlay.png`): changed texels magenta, UV region outline yellow.
- `20261112-120213-B-lab-s6-upperscan.png`: step 4, cleaned figure head/chest/back/gloves from four sides.
- Also `uv_locator_bands.png` (z bands over the texture), `lace_before_after` crop `z.png`.

## Numbers
Changed pixels: **130,234** of 4,194,304 (3.1 %; 13.4 % of the lower-body region), threshold 1.5/255, none outside the mask. Resolution 2048, 8-bit RGB PNG.

## Step 4: scan for other lettering
Looked at the whole texture (brightened quadrants) and the cleaned figure from front/sides/back (`sheet_upper_scan`): helmet sides, back of the helmet, chest, back, sleeves, gloves are plain colour with white stripes. Nothing reads as a letter, digit or logo.
Left alone but worth a glance in-engine: two black vent dots on the helmet front, the black chin strap arcs (x about 700..780, y 190..420), the two eye ovals (round, fine); they are shapes, not marks. The atlas is a very blocky, low-detail painting, so any stray dark pixel group could read as a glyph at some angle:
residual tiny specks remain on the white stripe lower edges of both shin cuffs (a few px, from the removal of the dark seam next to the lace tiles, slightly ragged edge).

## Rerun
`sh C:/Users/kazda/jam/skater-lab/s6/rerun.sh` (extract.py writes the orig PNG and faces.pkl, clean_texture.py writes the clean PNG and masks, verify.py counts changes and writes diff_overlay.png). Render checks: `blender -b -P render.py -- <png> <outdir> <tag> close|turn|upper`.

## Not verified
- The in-engine look (toon ramp, outline pass, normal and ORM maps): not run, no engine here. A later lane reimports the PNG (sRGB, same name pattern) and must re-look at the shins. If a dark scribble still shows in engine it is probably the outline pass on the lace-tile geometry rather than texture; then flatten the normal map in the same 160 islands.
- The in-engine "W" on the shin was matched to the lace tiles by position and look (crouched pose), not by a render from the engine.
- Meshy FBX not opened; assumed to share UVs (same texture per the S3 notes). 
