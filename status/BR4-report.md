# BR4 report: the K rebuilt so the word cannot read HOXEJ, and a softer snow burst

Lane BR4, device B, 2026-11-13, started 20:41, written 20:57. Blender only, no Unreal. In the game repository only this file was written. Nothing inside `C:\Users\kazda\jam\skater-lab\` was modified.

## Result

The K is rebuilt in the title everywhere: the banner, its two pieces, the logo, both alpha cuts and the size strip. I looked at the word at 1920, at 480 px and at 64 px after the last render: it reads H-O-K-E-J at each size, and the K has a plain stem on its left with nothing of the diagonals beside it. The burst is finer and has snow dust in it. Nothing else in the composition was changed; the other four letters keep their BR3 pattern cut for cut.

## What changed in the K's geometry

**Why it read as an X.** Three things added up:

- The old K (BR1's, in `hokej_lib.py`) let its two diagonals overlap: the upper arm began 0.3 stroke widths below the middle line and the lower leg ended 0.3 above it, so they crossed just right of the stem.
- The blade passes that rough up every stroke overshoot its ends by up to 0.42 stroke widths, with run-outs of up to 0.85 more, plus slips and corner bites. On the K's diagonals these ran through the stem and out on its left: a notch on the left side, which is what makes an X.
- In the banner the word lies in perspective and the K's stem leans about 27 degrees. The leg then leans the other way by almost the same angle, so the lower half was an upside-down V and the upper half a V.

**The new K** (`k_strokes` in `C:\Users\kazda\jam\brand-lab\scripts\final_lib.py`; BR1's letter code is untouched):

- **Stem:** a full-height upright, one stroke width wide, the same as the H's and the E's uprights.
- **Joint:** one point on the stem's right side at 56 % of the height. The upper arm ends on that line and the lower leg starts on it; they share the line and do not overlap.
- **Diagonals:** 0.85 stroke widths wide (measured along the line), lighter than the stem. They bite 0.3 stroke widths into the stem, so the joint is solid; their outlines reach no further left than 70 % of the stem's width, and no cut of theirs can pass the stem's middle.
- **Why 56 % and 0.85.** My first try (joint at the middle, diagonals 1.05 wide) was a correct K as flat polygons, but in the rendered banner the wedge between stem and arm closed up and the top read as one thick bar. Raising the joint and thinning the diagonals opens that wedge. `test\k-sketch.png` shows the candidates as flat polygons in the banner's perspective.

**The guard**, as for the O and the J:

- **Stem:** no run-outs, no slips, no corner bites.
- **Diagonals at the joint:** the passes stop just short of the joint end, there is no run-out or slip toward the stem, and no bite at the joint corners. Every cut of a diagonal is also masked off left of the stem's middle.
- **Around the K:** stray gashes that would cross the K's cell are dropped, and the edge chipping is halved inside the cell so the two wedges stay open.
- **The other letters:** every random draw is still made, so H, O, E and J are cut exactly as in BR3.
- The far ends of the diagonals (top right, bottom right) keep their run-outs and bites: the K is still hacked, not drawn.

## The burst (item 2)

It was a small change, so I did it:

- 6500 lumps at 0.36 of the size, instead of 2700 at 0.62, with slightly longer streaks.
- Nine soft puffs of snow dust low along the throw: flattened shapes that are opaque in the middle and fade to nothing at the outline, glowing slightly so they read as lit powder and not as smoke. No volume rendering.
- The flying lumps and the puffs cast no shadow; with shadows the ice under the burst went grey.

`banner-figures.png` was therefore re-rendered too. The burst is the same in `banner.png` and `banner-figures.png`.

## Output files

All in `C:\Users\kazda\jam\brand-lab\out\final\`, all overwritten in this lane except the two kept copies:

| File | Size | What |
|---|---|---|
| `C:\Users\kazda\jam\brand-lab\out\final\banner.png` | 1920 x 1080 | the accepted banner with the new K and the softer burst |
| `C:\Users\kazda\jam\brand-lab\out\final\banner-480.png` | 480 x 270 | its thumbnail |
| `C:\Users\kazda\jam\brand-lab\out\final\banner-figures.png` | 1920 x 1080 | the scene without the word, with the softer burst |
| `C:\Users\kazda\jam\brand-lab\out\final\banner-title.png` | 1920 x 1080 | the word alone, with the new K |
| `C:\Users\kazda\jam\brand-lab\out\final\logo.png` | 512 x 512 | the mark with the new K |
| `C:\Users\kazda\jam\brand-lab\out\final\logo-1024.png` | 1024 x 1024 | the same at full size |
| `C:\Users\kazda\jam\brand-lab\out\final\logo-alpha.png` | 1024 x 1024, alpha | for light pages |
| `C:\Users\kazda\jam\brand-lab\out\final\logo-alpha-dark.png` | 1024 x 1024, alpha | for dark pages |
| `C:\Users\kazda\jam\brand-lab\out\final\logo-sizes.png` | 1160 x 1184 | the logo at 512, 256, 128 and 64 px on dark and on white |
| `C:\Users\kazda\jam\brand-lab\out\final\final-sheet.png` | 1784 x 1812 | everything on one sheet |
| `C:\Users\kazda\jam\brand-lab\out\final\banner-v2.png`, `banner-v2-480.png` | | the BR3 banner, kept |
| `C:\Users\kazda\jam\brand-lab\out\final\banner-v1.png`, `banner-v1-480.png` | | the BR2 banner, still kept |

Tests in `C:\Users\kazda\jam\brand-lab\out\final\test\`: `k-sketch.png` (K candidates), `k-check.png` (the word from the banner enlarged, at 480 px, and the logo at 64 px), `word-final-check.png` (the same from the final renders), `burst-crop.png`, `logo-64-x6.png`, both `logo-alpha*-on-pages.png`, `banner-figures-v2.png` and `logo-1024-br3.png` (the BR3 stills for comparison). The BR3 scripts are kept as `C:\Users\kazda\jam\brand-lab\work\final_lib.br3.py` and `build_banner.br3.py`.

## What is weak

- **The K is lighter on its right side.** The diagonals are thinner than the stem and the joint sits above the middle, so the upper arm is short and the leg long. In the banner the leg stands almost upright on screen because of the perspective; it reads as an italic K.
- **The K is cleaner than its neighbours.** Its stem has no run-outs and its edges are chipped half as much, which a close look shows.
- **The word still leans.** The lean is what made the old K fragile; I did not change it because the composition is accepted.
- **The dust is shapes, not a volume.** At full size the puffs can read as a splash of water more than as powder, and their outlines are soft blobs. The flying lumps are now fine needles rather than grains. The small push-off burst at the attacker's skates is unchanged and still lumpy.
- **Judged by me alone, by eye.**

## How to rebuild

```
cd /c/Users/kazda/jam/brand-lab/scripts
bash run.sh final logo               # logo-1024.png, logo.png, both alpha cuts
bash run.sh banner full              # banner.png, banner-480.png
bash run.sh banner figures           # banner-figures.png
bash run.sh banner title             # banner-title.png
bash run.sh final sheet              # logo-sizes.png, final-sheet.png
BR4_MIST=0 BR4_BN=2700 BR4_BS=0.62 bash run.sh banner full preview    # the BR3 burst again, for comparison
BR4_KJOINT=0.5 BR4_KDIAG=1.05 bash run.sh banner title preview        # my first K, for comparison
```

New settings: `BR4_KJOINT` (joint height, 0.56), `BR4_KDIAG` (diagonal width in stroke widths, 0.85), `BR4_KCALM` (how much the edge chipping is reduced in the K's cell, 0.5), `BR4_BN` and `BR4_BS` (burst lump count and size), `BR4_MIST`, `BR4_MISTD`, `BR4_GLOW` (the dust: on or off, density, glow).

## Render times

Blender 4.2 LTS, Cycles on CPU with denoising; other jobs were running.

| Still | Samples | Render time |
|---|---|---|
| logo-1024 | 96 | 78 s, plus about 7 s for the mask pass |
| banner, banner-figures, banner-title (three at once) | 56 | 362 s, 334 s, 293 s |
| a half-size preview | 18 | 13 to 25 s |

## Rules kept

No font file (the K is three polygons from the script), nothing from outside, no service, no text other than HOKEJ, no gore.

## What I could not do or did not do

- **A K as heavy as the old one:** with diagonals as wide as the stem, the K closes up in the banner's perspective.
- **A real volumetric mist and real motion blur:** not used, to stay cheap.
- Not changed, as asked: camera, figures, poses, light, the other letters. The digit 7, the 25 to 30 degree camera and alpha cut-outs of the banner pieces remain open from BR3.
