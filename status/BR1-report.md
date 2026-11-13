# BR1 report: three title treatments for HOKEJ

Lane BR1, device B, 2026-11-13, written 14:22. Blender only, no Unreal. Nothing in the game repository was touched except this file.

## Result

Three candidates are rendered, each as a 1920 x 1080 title and a 512 x 512 logo, plus one sheet that shows the choice. All six stills spell H-O-K-E-J and read at 480 px wide (checked on the 480 px thumbnails in the sheet). My pick is **C, stencilled on the boards**.

The lane was interrupted at about 14:00 by an account limit. All six stills and the sheet were rendered again after the last script change (14:16 to 14:21), so none is stale or partial.

## Output files

| File | Size | What |
|---|---|---|
| `C:\Users\kazda\jam\brand-lab\out\title-sheet.png` | 1656 x 2086 | the three titles with their logos, labelled A / B / C, and a row of 480 px thumbnails |
| `C:\Users\kazda\jam\brand-lab\out\title-A.png` | 1920 x 1080 | A, plaster blocks and tape |
| `C:\Users\kazda\jam\brand-lab\out\title-B.png` | 1920 x 1080 | B, gouged into the ice |
| `C:\Users\kazda\jam\brand-lab\out\title-C.png` | 1920 x 1080 | C, stencilled on the boards |
| `C:\Users\kazda\jam\brand-lab\out\logo-A.png` | 512 x 512 | A, mark alone |
| `C:\Users\kazda\jam\brand-lab\out\logo-B.png` | 512 x 512 | B, mark alone |
| `C:\Users\kazda\jam\brand-lab\out\logo-C.png` | 512 x 512 | C, mark alone |

Scenes: `C:\Users\kazda\jam\brand-lab\work\title-A.blend`, `title-B.blend`, `title-C.blend`, `logo-A.blend`, `logo-B.blend`, `logo-C.blend` (the `.blend1` files beside them are Blender's backups of the previous pass). Logs: `C:\Users\kazda\jam\brand-lab\logs\`.

## The candidates

### A. Plaster blocks and tape
- **What it is:** each letter is three or four separate cream plaster blocks with chipped corners, dents, hairline cracks and dark puck marks. Loops of white and black tape with torn ends bind them, and a chipped red cap sits on every top. The blocks stand on scratched ice in front of the plank wall, with two hanging lamps and a red line in the foreground.
- **What works:** it is the most physical of the three and the closest to "board-and-tape". It is a real object, so the clay figures could crash into it in the banner. The palette matches the barn.
- **What is weak:**
  - It is the least legible at 480 px. Tape crosses the H's crossbar and the E's middle bar, and the O is a boxy frame that sits close to the H.
  - The dark scuff streaks still read a little like birch bark rather than plaster.
  - The ice under the letters is blown out (about 18 % of pixels at full white).
  - In the square the letters had to be stretched tall and narrow.

### B. Gouged into the ice
- **What it is:** every stroke is five parallel blade passes cut into a displaced ice surface, with dark wet ice in the cuts and snow piled on the edges. Ragged run-outs and a few long scratches cross the word. A low warm light comes from the left, and a puck and a taped stick blade lie beside it. The view is a raised three-quarter angle.
- **What works:** the letters are very clear, with dark blue on pale ice. It is the calmest image and belongs to the rink surface directly. The puck gives scale.
- **What is weak:**
  - It uses only the ice colours. Cream, red cap and yellow strip are absent, so it is the least tied to the "clay and plaster" voice.
  - It reads cold and tidy rather than brutal. The five passes per stroke are too regular.
  - The long scratches through the word are faint.
  - The lamp glare clips the top left corner.
  - The perspective makes the word smaller than in A and C. The logo is low in contrast and would fade on a pale background.

### C. Stencilled on the boards
- **What it is:** the word is painted in black over a red offset on cream plaster panels, under the red cap and above the yellow kick strip. The paint is chipped in patches and scraped in horizontal lines, with round puck marks and skid trails. Panel joints run through the O and the E, a crack with a branch runs down the J's panel, and a pile of snow is sprayed against the foot under H and O.
- **What works:**
  - It is the strongest read at 480 px and in the square.
  - It carries all the board colours of the game.
  - The wear tells the checking story without any gore: pucks, bodies and a hard stop against the boards.
  - The boards are where the collision for the banner would happen anyway.
- **What is weak:**
  - It is flat paint, so it is the least sculptural. The red offset can read as a graphic drop shadow.
  - The snow pile is slightly overexposed.
  - In the logo the kick strip and snow take the lower fifth of the square.
  - It kept 67 to 86 % of its paint per letter in the title. It could take more wear before it stops reading, so it is on the safe side of "raw".

## Which I would pick

**C.** It is the only one where legibility and roughness do not fight each other. A is the more original object but needs its tape moved off the crossbars before it can be a logo. A fair combination for the full set is C as the logo and A's taped blocks as a prop in the banner. B is the fallback if the operator wants the quietest mark.

## How to rebuild

Everything is driven by Python through one shell script, run from Git Bash:

```
cd /c/Users/kazda/jam/brand-lab/scripts
bash run.sh A title            # out/title-A.png and work/title-A.blend
bash run.sh A logo             # out/logo-A.png and work/logo-A.blend
bash run.sh B title            # same for B and C
bash run.sh C title preview    # half size, a third of the samples, out/title-C-preview.png, no .blend
bash run.sh sheet              # out/title-sheet.png from the six finished stills
BR1_SEED=7 bash run.sh A title # another pattern of chips, scuffs and scratches
BR1_EXPO=-1.6 bash run.sh B title  # exposure in stops
```

- `C:\Users\kazda\jam\brand-lab\scripts\hokej_lib.py`: letter shapes, palette, materials, props, render setup.
- `C:\Users\kazda\jam\brand-lab\scripts\build_A.py`, `build_B.py`, `build_C.py`: one scene each; the first argument is `title` or `logo`.
- `C:\Users\kazda\jam\brand-lab\scripts\sheet.py`: the sheet.
- `C:\Users\kazda\jam\brand-lab\scripts\run.sh`: the Blender call, `"/c/Program Files/Blender Foundation/Blender 4.2/blender.exe" -b --factory-startup --python build_X.py -- title|logo [preview]`.

The letter shapes are in `letter()` in `hokej_lib.py`: width, height, stroke and gap are set at the top of each build script.

## Render engine and time

Blender 4.2.16 LTS, Cycles on CPU, adaptive sampling with OpenImageDenoise, Standard view transform.

| Still | Samples | Render time, run alone |
|---|---|---|
| title-A | 72 | 176 s |
| title-B | 72 | 93 s |
| title-C | 72 | 145 s |
| logos | 120 | 48 to 64 s each, three run at once |

Scene building adds 5 to 10 s. One earlier pass of title-A took 285 s because logo renders shared the CPU. EEVEE was not tried.

## Rules kept

- **No font file:** the five letters are convex polygons defined in the script, and the A / B / C labels on the sheet are 5 x 7 block glyphs drawn from rectangles.
- **Nothing from outside:** no downloaded texture, model or image, no outside service, and the engine stills were only looked at.
- **Generated masks:** the scratch, paint-wear and gouge masks are images the scripts compute with numpy and save in `work\`. They are not pure node procedurals, but they are fully generated here.
- **Teen rating:** no blood or injuries. The only red is the board cap, the painted offset and the rink line.
- **No other marks:** no brands, team names or text other than HOKEJ, apart from the sheet labels.

## What I could not do or did not do

- Blown highlights remain on the ice in A and in the top left of B. One more lighting pass would fix them.
- A's scuffs still lean towards birch bark, and its tape still crosses the H and E bars.
- No logo with a transparent background, and no logo test on a dark or a white page.
- The 1920 x 1080 banner with the clay figures and the final 512 logo were not started. They wait for the choice.
- The 480 px check was done by eye on the sheet thumbnails only. Nobody else has looked at them.
