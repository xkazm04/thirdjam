# BR3 report: the banner in the barn with the hit as second subject, and the logo fixes

Lane BR3, device B, 2026-11-13, started 15:25, written 16:08. Blender only, no Unreal. In the game repository only this file was written. Nothing inside `C:\Users\kazda\jam\skater-lab\` was modified: its figure and stick exports were only read.

## Result

The full set is in `C:\Users\kazda\jam\brand-lab\out\final\`. I took over from the stopped agent's `build_banner.py` (it ran, but its figures were 24 % of the frame, the victim lay flat and nothing was rendered at full size), rebuilt the composition, and looked at every output after each pass. HOKEJ is spelled right and every letter reads in the banner at 1920 and at 480 px wide, in the title piece, in the logo down to 64 px and in both alpha cuts.

## Output files

| File | Size | What |
|---|---|---|
| `C:\Users\kazda\jam\brand-lab\out\final\banner.png` | 1920 x 1080 | the word in the ice across the lower left, the check directly right of the J, the barn across the top |
| `C:\Users\kazda\jam\brand-lab\out\final\banner-480.png` | 480 x 270 | the thumbnail of it |
| `C:\Users\kazda\jam\brand-lab\out\final\banner-figures.png` | 1920 x 1080 | the same scene, camera and light without the word (the same snow burst, lump for lump) |
| `C:\Users\kazda\jam\brand-lab\out\final\banner-title.png` | 1920 x 1080 | the word alone on that ice, with the barn and the puck |
| `C:\Users\kazda\jam\brand-lab\out\final\logo.png` | 512 x 512 | the mark with the thicker H bar, without the chip |
| `C:\Users\kazda\jam\brand-lab\out\final\logo-1024.png` | 1024 x 1024 | the same at full size |
| `C:\Users\kazda\jam\brand-lab\out\final\logo-alpha.png` | 1024 x 1024, alpha | for light pages: navy cuts and their snow rim |
| `C:\Users\kazda\jam\brand-lab\out\final\logo-alpha-dark.png` | 1024 x 1024, alpha | for dark pages: the same cut shapes filled pale ice-blue, a tight white rim |
| `C:\Users\kazda\jam\brand-lab\out\final\logo-sizes.png` | 1160 x 1184 | the logo at 512, 256, 128 and 64 px on a dark and on a white page |
| `C:\Users\kazda\jam\brand-lab\out\final\final-sheet.png` | 1784 x 1812 | banner, logo, size strip, both alpha cuts each on dark and on white, the 480 thumbnail, the two pieces small |
| `C:\Users\kazda\jam\brand-lab\out\final\banner-v1.png`, `banner-v1-480.png` | 1920 x 1080, 480 x 270 | the BR2 banner, kept untouched |

Tests in `C:\Users\kazda\jam\brand-lab\out\final\test\`: `pose-sheet.png` (the frames I chose from), `banner-preview.png`, `banner-figures-preview.png`, `banner-title-preview.png` (the last half-size previews), `logo-alpha-on-pages.png` and `logo-alpha-dark-on-pages.png` (each alpha cut on black and on white, large). Scenes: `C:\Users\kazda\jam\brand-lab\work\final-banner5.blend`, `final-logo.blend`. Logs: `C:\Users\kazda\jam\brand-lab\logs\banner-*.log`, `final-logo.log`.

## The banner

- **Layout.** The word lies across the lower left and middle (5 to 56 % of the width, 64 to 95 % of the height). The hit happens directly right of the J, on the same patch of ice. The barn runs across the top: dark plank wall, two hanging lamps, cream boards with the red cap and the yellow kick strip, then the red centre line behind the play. The puck lies loose in front of the hit.
- **Figure size.** Measured from the projected meshes: the attacker's body is 33 % of the frame's height, the victim 32 % (BR2: about 8 %). Faces, stitches and felt show at full size. The attacker's stick crosses the right frame edge.
- **Camera.** 3.0 m up, 20 degrees down, 34 mm. This is flatter than the 25 to 30 degrees suggested: at 25 degrees the wall and the lamps leave the frame. To get the barn into the top at 20 degrees the boards stand close behind the play (5 m behind the word) and the lamps hang low (2.1 m).
- **Poses.** Chosen from `test\pose-sheet.png` (Knockdown_Left 4 to 9, Fall_B 4 to 9, Check_Deliver_Right 7 to 12):
  - Victim: `Knockdown_Left` frame 5. In `Fall_B` the skates never leave the ice (highest skate point 0.31 m, lowest 0.00), so it was rejected. In Knockdown_Left the skates are highest from frame 6 (0.60 to 0.92 m), but 6 to 9 are one stiff pose; frame 5 (0.51 to 0.87 m) has the legs apart and an arm out.
  - The take holds the body level, 12 cm over the ice, which read as "lying down" in the stopped agent's preview. I placed the posed figure on its arc: tipped 30 degrees about the axis across the throw (skates up, head leading) and lifted 0.36 m. The pose itself is the take's; only the placement is mine. His lowest point is 0.30 m over the ice, his skates are the highest thing in the collision.
  - Attacker: `Check_Deliver_Right` frame 9 (frames 7 to 12 differ by a centimetre).
  - Figures from `C:\Users\kazda\jam\skater-lab\skf\export19\p3_clay\` (it has a `READY.txt`; the takes used are listed there as identical to export18).
- **The hit.** A burst of about 2700 flying lumps on ballistic arcs along the hit direction, each stretched along its velocity (cheap motion streaks), what has landed lying on the ice, a smaller push-off burst at the attacker's skates, and the victim's stick in the air in front of the boards.
- **Light.** One lamp pool on the hit from above and a weaker one over the start of the word, both falling off to the edges (no post-processing vignette); warm pools under the two visible lamps on the far ice and the boards; a dim low warm light from the front left; a dim cool fill. Median luma 0.49 against 0.66 in the first banner. No corner is blown to white (0.0 % in all four; the 1.3 % "any channel at full" in the top corners are the two bulbs).
- **Three passes at full size.** Pass 1: the snow heap lay over the J's stem and read as a pile of pebbles. Pass 2: the burst made finer, shorter and streakier and kept off the J. Pass 3: the ice in front still measured mauve (163, 146, 160), so the key lamps were made cool white and the warm side light halved; it now measures lavender (155, 145, 177 between the line and the word, 200, 197, 241 in the pool by the puck), while the far band under the lamps stays warm (201, 161, 150).

## The logo

- **H crossbar.** 50 % thicker, and it now runs half a stroke into each upright. The stopped agent's setting (thicker only) made the bar taller than long, so its blade passes turned upright and the H fell apart into two posts in my first render; the bar has to stay longer than thick.
- **The chip between H and O.** Gone: no corner bites on the O and on the J's hook. The random draws are kept, so the rest of the pattern is BR2's.
- **`logo-alpha-dark.png`.** The same cut shapes, filled pale ice-blue (150, 200, 250) with the grooves kept as shading. The snow rim is pushed to white and kept tight to the cuts (it fades out away from them), because the full rim joined the letters into one pale block on black. Loose specks are removed: a stray dent between E and J read as a full stop.
- **Pinholes.** Loose ice chunks lying in a cut punched holes into both alpha cuts; they are now hidden in the mask pass.
- `logo-alpha.png` (navy) is unchanged in method and is for light pages only: on black it still sinks.

## What works

- The collision is now the second subject: two large characters, one of them upside down in the air, readable at 480 px.
- The word still reads at a glance at 480 px, and the J is clean.
- The picture belongs to the game's barn: dark warm wood and lamps above, cool ice below, the team colours.
- The dark-page mark reads clearly on black.

## What is weak

- **The camera is flatter than asked** (20 degrees, not 25 to 30), and the lamps hang lower than a real barn's. The camera looks down on the shades, so they are black cones with a bare bulb, not glowing discs as in the engine frame.
- **The word is foreshortened.** The O is a slanted box, H and O stand close, and the word is smaller than in the first banner (51 % of the width).
- **The ice is not white.** In front it is a mid lavender; the far band under the lamps is warm peach. It is less pink than before, but darker than the engine frame's ice.
- **The burst is lumps, not powder.** At full size the streaked lumps look like grains of rice; there is no mist. It partly covers the victim's face, which is upside down anyway.
- **The victim's tilt and lift are mine,** not the animation's. In the game he does not fly this high.
- **Tight right edge.** The attacker's body ends at 97 % of the width.
- **Boards and figures.** The boards are plain boxes, large and evenly lit. The felt and clay are still my procedural approximations of the engine look. No depth of field.
- **The pieces are full opaque frames,** not cut-outs: `banner-figures.png` and `banner-title.png` share camera, light and ice, but layering one over the other needs a mask, which I did not make. The title piece has no figure shadows.
- **Logo.** The angular bites at the feet of K, E and J remain, as in BR2. On a white page `logo-alpha-dark.png` is faint, as intended.
- **All judged by me alone, by eye.**

## How to rebuild

Run from Git Bash:

```
cd /c/Users/kazda/jam/brand-lab/scripts
bash run.sh banner full              # banner.png, banner-480.png, work/final-banner5.blend
bash run.sh banner figures           # banner-figures.png
bash run.sh banner title             # banner-title.png
bash run.sh banner full preview      # half size, a third of the samples, into out/final/test/ (about 30 s)
bash run.sh final logo               # logo-1024.png, logo.png, logo-alpha.png, logo-alpha-dark.png
bash run.sh final alpha              # both alpha cuts again from the stills on disk, no render
bash run.sh final sheet              # logo-sizes.png, final-sheet.png (also rewrites banner-480.png)
bash run.sh poses                    # test/pose-sheet.png
BR3_VF=6 BR3_TILT=20 BR3_LIFT=0.2 bash run.sh banner full preview   # another victim frame and placement
```

- `C:\Users\kazda\jam\brand-lab\scripts\build_banner.py`: the BR3 banner; every setting and environment variable is at the top (`BR3_AF`, `BR3_VF`, `BR3_VTAKE`, `BR3_TILT`, `BR3_LIFT`, `BR3_HIT`, `BR3_AX`, `BR3_AY`, camera `BR3_CX`, `BR3_CY`, `BR3_CH`, `BR3_PITCH`, `BR3_LENS`, light `BR3_KEY`, `BR3_KEY2`, `BR3_SUN`, `BR3_FILL`, `BR3_EXPO`, `BR3_SPP`, `BR3_OUT`). It logs each letter's and each figure's box in the frame.
- `C:\Users\kazda\jam\brand-lab\scripts\pose_sheet.py`: the contact sheet, with skate heights per frame in the log.
- `C:\Users\kazda\jam\brand-lab\scripts\final_lib.py`: added the stretched-letter layout, the H bar, `snow_burst2` and `add_streaks`, `write_alpha_cuts`.
- `C:\Users\kazda\jam\brand-lab\scripts\build_final.py`: the logo; its old `banner` mode now writes `banner-v1.png`, so it cannot overwrite the new banner.
- `C:\Users\kazda\jam\brand-lab\scripts\final_sheet.py`: the sheet's right column changed (the cream page dropped, the dark-page cut and the two pieces added).
- The figure export can be changed with `BR2_SKF=<folder>`; the banner defaults to export19.

## Render engine and time

Blender 4.2 LTS, Cycles on CPU with OpenImageDenoise, adaptive sampling, Standard view transform. Other heavy jobs were running on the machine.

| Still | Samples | Render time |
|---|---|---|
| logo-1024 | 96 | 91 s, plus about 8 s for the mask pass |
| banner (last pass, three stills at once) | 56 | 415 s |
| banner-figures (last pass, three at once) | 56 | 397 s |
| banner-title (last pass, three at once) | 56 | 414 s |
| banner-figures alone, for comparison | 56 | 136 s |
| a half-size preview | 18 | 15 to 26 s |

Scene building adds 10 to 30 s. All are under the 8 minute limit.

## Rules kept

- **No font file:** the letters are polygons from the script.
- **Nothing from outside:** no downloaded texture, model or image, no service, nothing sent anywhere. The engine frame was only looked at. The only assets not built here are the team's own figure and stick exports.
- **Teen rating:** no blood, no injuries; the hit is sold by the thrown body, snow and the flying stick.
- **No other marks:** no brands, no text other than HOKEJ, no labels on the sheet.

## What I could not do or did not do

- **The digit 7 on the back patch:** not built. Both figures face the camera, so no back patch is in view.
- **The 25 to 30 degree camera:** not used, see above.
- **Real motion blur and snow mist:** not used; the streaks are stretched geometry.
- **Cut-out pieces with alpha for the menu:** not made; the pieces are full frames.
- The two `*-on-pages.png` test composites were made with the machine's Python (Pillow) for a quick look; they are not part of the rebuild. The sheet shows the same thing from the Blender scripts.
