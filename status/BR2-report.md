# BR2 report: the final logo and banner on B (gouged into the ice)

Lane BR2, device B, 2026-11-13, written 15:05. Blender only, no Unreal. In the game repository only this file was written. Nothing inside `C:\Users\kazda\jam\skater-lab\` was modified: its figure export was only read.

## Result

The full set is in `C:\Users\kazda\jam\brand-lab\out\final\`, rendered four times and looked at after each pass. The last pass finished at 15:03. HOKEJ is spelled right and reads in every output, including the logo at 64 px and the banner at 480 px. The banner has the game's own clay figures in a shoulder check: loading the scripted-figure lane's export worked, so the block-figure fallback was not needed.

## Output files

| File | Size | What |
|---|---|---|
| `C:\Users\kazda\jam\brand-lab\out\final\logo.png` | 512 x 512 | the mark, one line, on the ice with a strip of the red line |
| `C:\Users\kazda\jam\brand-lab\out\final\logo-1024.png` | 1024 x 1024 | the same render at full size (`logo.png` is this one halved) |
| `C:\Users\kazda\jam\brand-lab\out\final\logo-alpha.png` | 1024 x 1024 | transparent background: the cuts and their snow rim only |
| `C:\Users\kazda\jam\brand-lab\out\final\logo-sizes.png` | 1160 x 1184 | the logo at 512, 256, 128 and 64 px on a dark page and on a white page |
| `C:\Users\kazda\jam\brand-lab\out\final\banner.png` | 1920 x 1080 | the title in the ice, two clay figures in a check beyond it |
| `C:\Users\kazda\jam\brand-lab\out\final\banner-480.png` | 480 x 270 | the thumbnail |
| `C:\Users\kazda\jam\brand-lab\out\final\final-sheet.png` | 1728 x 1812 | banner, logo, size strip, the alpha cut on dark, white and cream, the 480 thumbnail |
| `C:\Users\kazda\jam\brand-lab\out\final\test\line-vs-stack.png` | 1088 x 592 | the one-line logo against "HO / KEJ" at 256, 128 and 64 px |
| `C:\Users\kazda\jam\brand-lab\out\final\test\logo-stack.png` | 256 x 256 | the stacked test alone |

Scenes: `C:\Users\kazda\jam\brand-lab\work\final-logo.blend` and `final-banner.blend`. Logs: `C:\Users\kazda\jam\brand-lab\logs\final-*.log`.

## What was changed on B

- **The cuts.** Each stroke now has a scraped floor with a noise-chipped edge, and four to six blade passes of different depth, width and length, each bent in the middle. Passes overshoot the stroke ends, some by most of a stroke width. There are slips that run out of the cut at an angle, angular bites broken out at the corners, stray gashes and short stick-heel dents around the word. The dark wet ice in the cuts is kept and made darker.
- **Snow.** A thin rim on both sides, more thrown to one side (right and towards the viewer) as by a hard stop, loose lumps, a spray of chips and a few larger ice chunks.
- **The ice around.** Long blade scratches through and past the word, two curved stop marks, two rubber puck marks, scuffed patches.
- **Palette.** A warm lamp low from the left against a cool top fill, so the snow rims catch warm light. The red centre line runs between the word and the figures in the banner and along the foot of the logo. The banner shows the yellow kick strip and the boards along the top edge.
- **Legibility guards**, found by looking at the renders:
  - The O gets no run-outs: a tail turned it into a Q in the second pass.
  - The J's hook and flag get none either: a slip made the hook read like a second stem.
  - Long run-outs happen only on uprights and diagonals, because horizontal ones joined E to J.

## The logo

- **One line kept.** The stack "HO / KEJ" has larger letters at 64 px, but it splits the word, and the one line still reads at 64 px. It was not clearly stronger. Note that the comparison strip was made from an earlier preview pass, before the last wear changes.
- **Contrast.** Dark navy cuts (2nd percentile luma 0.19) on lavender ice (median 0.68) with white rims. The camera is steeper than in BR1 (80 degrees down), and the letters are tall and condensed so the word fills the square.
- **Alpha.** A second, emission-only render of the cut and snow masks from the same camera gives the alpha; loose specks are removed by a blur and threshold. About 33 % of the square is opaque and 58 % clear.

## The banner

- **Camera.** 3.6 m up, 33 degrees down, 28 mm lens, like the game's camera behind the attacker.
- **Figures.** The lane's `export18\p3_clay\SK_Skater_p3_clay.fbx`:
  - Blue attacker: `Check_Deliver_Right`, frame 10.
  - Orange victim: `Knockdown_Left`, frame 6, placed 0.30 m ahead and 0.74 m to the attacker's right (0.8 m centre to centre), plus the lane's slide formula. His lowest point is 7 cm off the ice.
  - Poses are baked to static meshes with the same placement logic as `pair_render12.py`.
- **The hit.** A burst of snow lumps and ice chips at the skates, a smaller push-off burst behind the attacker, and the victim's stick (the lane's `SM_Stick_p3_clay.fbx`) in the air beside him.
- **Clipping.** In the last pass no corner is blown to white, apart from 1.3 % of the bottom right corner, which is lit snow. The third pass had 97 % of the top left corner at full red channel; moving the lamp back fixed it.

## What works

- The word reads at a glance at every size tested, and now looks hacked rather than drawn.
- The banner tells the game in one picture: the title, the check, snow flying, the red line, the boards.
- The figures are the game's own, in the team colours, at the game's camera angle.

## What is weak

- **Softened edges.** The height field is blurred by 0.75 px to remove spiky slivers on the cut walls, so the fractured edges are softer than real broken ice. The snow lumps and chunks are rounded, not sharp shards.
- **A chip at the O.** A corner bite at the O's lower left shows as a small dark spot between H and O at thumbnail size.
- **Logo letter shapes.** The H's crossbar is thin, and the letters are tall and narrow.
- **Alpha on a dark page.** The navy cuts sink into the dark and the snow rim looks speckled. On white and on cream it is clean.
- **Marks on the ice.** The stop marks are faint, and the puck marks look like short dark dashes.
- **The lamp.** After it was moved back to stop the clipping, the warm side light is subtle. The ice reads lavender-pink with a mild warm gradient at the left.
- **Figure materials.** The felt and clay are my own procedural materials on the lane's meshes, matched by eye to the engine stills. The export carries only flat colours, so this is an approximation of the Unreal look.
- **Figure details.** The back patch has no number, and the victim's hands are empty because his stick was replaced by the flying prop.
- **The burst.** It is static lumps with no motion blur.
- **The boards.** They are plain boxes, sharp, not out of focus, and darker than the game's cream so the lamp does not clip them.
- **Perspective.** The wide lens leans the outer letters of the banner outward.

## How to rebuild

Run from Git Bash:

```
cd /c/Users/kazda/jam/brand-lab/scripts
bash run.sh final logo             # logo-1024.png, logo.png, logo-alpha.png, work/final-logo.blend
bash run.sh final banner           # banner.png, banner-480.png, work/final-banner.blend
bash run.sh final sheet            # logo-sizes.png, final-sheet.png (also rewrites banner-480.png)
bash run.sh final logo preview     # half size, a third of the samples, into out/final/test/
bash run.sh final stack preview    # the HO / KEJ test
BR2_SEED=42 bash run.sh final logo         # another pattern of cuts (default 43)
BR2_AF=9 BR2_VF=5 bash run.sh final banner # other frames of the attacker and the victim
BR2_NOFIG=1 BR2_OUT=banner-title.png bash run.sh final banner   # the title alone (not rendered in this lane)
```

- `C:\Users\kazda\jam\brand-lab\scripts\final_lib.py`: the gouged word, the used-ice marks, the ice material, the figure loader and felt materials, the snow burst.
- `C:\Users\kazda\jam\brand-lab\scripts\build_final.py`: the logo, stack and banner scenes; all settings and environment variables are at the top.
- `C:\Users\kazda\jam\brand-lab\scripts\final_sheet.py`: the size strip and the sheet.
- `C:\Users\kazda\jam\brand-lab\scripts\hokej_lib.py` and `run.sh`: shared with BR1; the BR1 commands still work.

The figure path can be pointed at a newer export with `BR2_SKF=<folder>`.

## Render engine and time

Blender 4.2.16 LTS, Cycles on CPU, adaptive sampling with OpenImageDenoise, Standard view transform. Times are from the last pass, with the logo and the banner rendering at the same time:

| Still | Samples | Render time |
|---|---|---|
| logo-1024 | 96 | 154 s, plus about 15 s for the alpha mask pass |
| banner | 72 | 223 s |

Scene building adds about 10 s for the logo and 40 s for the banner (two FBX imports). All are under the 8 minute limit.

## Rules kept

- **No font file:** the letters are polygons from the script.
- **Nothing from outside:** no downloaded texture, model or image, and no service. The only assets not built here are the team's own figure and stick exports from `skater-lab`.
- **Teen rating:** no blood or injuries; the hit is sold by snow, chips and the flying stick.
- **No other marks:** no brands and no text other than HOKEJ; the sheets carry no labels.

## What I could not do or did not do

- The engine's exact felt materials and the number on the back patch are not in the Blender export, so they are approximated or left out.
- The taped stick prop by the word was dropped from the banner: with the attacker's stick and the victim's flying stick it would have been a third. The puck stays.
- The boards are not out of focus: no depth of field was used.
- The fallback versions (title-only banner, block figures) were not rendered because the figures loaded.
- The sizes test was judged by me alone, by eye.
