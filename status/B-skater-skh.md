# Skater lab SKH: generated-mesh skaters for P3 (clay) and P4 (broadcast), report at 15:31

**Short answer.**
- **Step 1, the retarget for any rigged mesh: done.** `retarget.py` now takes any Meshy `ue5_mannequin` rig. R rebuilt through it matches `s5/export` with **0.00000 cm** difference on every bone, every body vertex and every stick vertex, on all 31 takes, in both files.
- **P4 (broadcast):** I would import **P4B** first: the Rodin player with shoulder pads.
- **P3 (clay):** I would import **P3A** first: the Rodin hand-made clay figure. **Rclay** needs no import at all. It is the P2 hybrid with a clay base colour and a 1K thumb-dent normal map: a material change on T-B2.

All of this is measured in Blender on the re-imported FBX files. **Nothing has been in Unreal.**

- Lab folder: `C:\Users\kazda\jam\skater-lab\skh\`.
- Credits: Rodin **2.0** of 8, balance 406.5 to 404.5. Meshy **20** of 60, balance 806 to 786 on the shared account. Ledgers: `skh\rodin-ledger.jsonl`, `skh\meshy-ledger.jsonl`, `skh\balances.jsonl`.

## 1. `retarget.py` takes any rigged mesh

**Usage:** `blender -b --factory-startup --python skh/retarget.py -- <skh> <cfg.json>`.

**Outputs:**
- `export/<name>/SK_Skater_<name>.fbx` and `_loopcut.fbx`. The base colour is referenced by path from `export/<name>/`. Nothing is embedded.
- `out/<name>/retarget_report.json`, which now also holds the per-frame S4 contact flags and the grip weights that `verify.py` uses, and `out/<name>/skater.blend`.

What used to be written for R alone is now measured from the input:

| Was (S5, R only) | Now measured as | R measures |
|---|---|---|
| K = 0.586 | hip, knee and ankle bones of the figure over S4's (already a measurement in S5) | 0.5858 |
| helmet centre | bbox centre of the head-dominant vertices (as in S5) | (0.003, -0.002, 1.424) |
| `HEAD_CLEAR` 0.40 | head radius + mitten radius. Head radius = mean bbox half-extent of the head vertices; mitten radius = the same for the hand vertices. | 0.278 + 0.133 = **0.411** |
| `STICK_HEAD_CLEAR` 0.36 | chin distance (farthest head vertex from the centre) + 0.03 | 0.328 + 0.03 = **0.358** |
| bottom-hand spacing 0.22 | 2 x mitten radius | **0.266** |
| boot weighted to calf? (R only) | share of the vertices below the foot bone's head that are calf-dominant | 0.876 |
| `ANKLE_Z` 0.30 | the foot bone's head z, unless the config sets it | 0.217 (config: 0.30) |
| stick socket in the mitten | mean of the hand-dominant vertices (as in S5) | unchanged |
| `DZ_RATE` 2 cm/frame | 2 cm x K / 0.5858: the same speed in S4 units | 2.0 cm |

**Per-figure config** (`skh/cfg/<name>.json`): rig FBX, base colour, optional normal map and roughness, plus `override` for what cannot be measured.
- **R:** the sock band, `ankle_z` 0.30, is a texture feature and cannot be measured.
- **R regression only:** S5's hand-tuned clearances are pinned, and `stick_ice_tilt` is off.

**New rules, found by looking at the sheets.** Both are general; R has them off so that the regression stays exact:
1. **Boot re-weight whenever more than 2 % of the boot is calf-dominant**, not only when most of it is.
   - Why: the generated figures' heels were partly calf-weighted.
   - Effect: on P4A, Check went from 11.8 to 9.2 cm (together with rule 3), but this did not cure it (see below).
2. **Stick ice tilt.** After the slide (max 25 cm), the stick tilts about the grip, across the shaft, by the smallest angle (up to 60 deg) that does two things:
   - puts the blade at S4's height while S4's blade is on the ice;
   - keeps every stick vertex at or above the ice.

   Then the left hand re-grips the tilted shaft.
   - Before: P3B's stick went **40 cm into the ice** (Stop), P3A's 11 cm, and the long-armed P4 blades floated 20 cm in Stride.
   - After: no stick vertex is below 0.00 cm on any take of any candidate. Blade contact error is at most 1.6 cm on standing takes.
   - On R (`Rm`), Crossover_Right's 9 cm blade float drops to 1.5 cm.
3. **Foot-pass rate scaled by K** (above).

**Regression for R** (`compare.py` re-imports both files and compares them frame by frame): `out/R/compare_s5.json` and `compare_s5_loopcut.json`.
- 31 of 31 takes in both files.
- Worst difference: bone 0.00000 cm, body vertex 0.00000 cm, stick vertex 0.00000 cm.
- So pelvis travel, seams, foot errors and stick errors are identical, well within the 1 mm asked for.
- The only file-level changes are the material name (`M_Skater_R`, was `Material_0`) and the object names (`SK_Skater_R`, `SK_Skater_R_Stick`).

**`Rm`** is R with only the measured values, plus the ice tilt, kept as a diagnostic (`export/Rm`, not a candidate).
- Its clearances are 0.411, 0.358 and 0.266, against 0.40, 0.36 and 0.22.
- What changes: the grip on Crossover_Right and GetUp is 1 to 2 cm wider, and the ice tilt closes the blade floats (Crossover_Right 9.0 to 1.5 cm, GetUp_Front 20.7 to 0.9 cm).
- Everything else is identical to R.

**`verify.py`** now also measures on the re-imported file:
- the lower-skate height on frames where S4's skate is on the ice;
- the blade error on frames where S4's blade is on the ice;
- the left-mitten centre's distance from the shaft. The shaft is the `stick_socket` Y axis; every stick vertex lies within 2.4 cm of it.
- mitten or stick inside the head radius.

## Candidates

All of them have these in common:
- **Skeleton and file:** 32 bones, `root` is the only top bone with an identity rest, centimetre file (armature object scale 0.01 on re-import), 31 takes at 60 fps.
- **Motion checks:** **pelvis horizontal travel 0.000 cm and root motion 0.000 cm on every take**, and **loop seams 0.000 cm** in the main file. The loopcut wrap is one frame step: 1.05 to 1.52 cm on the crossovers, 0.00 to 0.44 cm otherwise.
- **Materials and textures:** one body slot with the base colour by path, plus the stick's two flat slots. The stick is 196 tris.
- **Rendering:** sheets are rendered from the re-imported FBX. Workbench (texture colour plus outline) stands in for P4. EEVEE matte stands in for the P3 comparison and Rclay.

### Sources

**Rodin calls:** Gen-2.5-Low, `quality_override` 20000, GLB, no TAPose flag, 0.5 credit each, 35 to 65 s.

**Meshy rigs:** `--template ue5_mannequin`, 5 credits each. P4 at `--height 1.8`; P3 at `--height 1.7`, R's height, so that the clay figures stay at the P2 scale.

Prompts, verbatim:
- **P4A:** `a stylised but realistically proportioned adult ice hockey player in full gear, about seven heads tall with long arms and long legs: a deep blue hockey helmet with a clear half visor, broad shoulder pads under a loose deep blue jersey with white trim, big padded deep blue hockey gloves, padded deep blue hockey shorts, shin guards under deep blue socks with white trim, black ice skates with silver blades, standing in an A-pose with the arms held away from the body and the legs apart, arms and legs clearly separated from the body, sports broadcast look, clean simplified shapes, no text, no letters, no numbers, no logos`
- **P4B:** `a professional ice hockey player in full equipment, real athletic proportions, stylised video game character: a deep blue helmet with a clear visor over the eyes, bulky shoulder pads making a broad silhouette under a loose deep blue jersey with white trim at the sleeves and hem, large deep blue padded gloves, deep blue padded hockey pants, deep blue socks over shin guards, black hockey skates with steel blades, A-pose with straight arms angled down away from the body and legs slightly apart, limbs clearly separated, semi-realistic, clean shapes, no text, no letters, no numbers, no logos`
- **P3A:** `a hand-made clay figure of a chunky ice hockey skater, sculpted in deep blue modelling clay with visible thumb dents and soft irregular rounded forms, a round deep blue clay helmet, a deep blue clay jersey with a white stitched seam along the shoulders and sides, round white clay mittens, short thick deep blue clay legs, black clay skates, a friendly simple clay face, standing in an A-pose with arms and legs clearly separated from the body, matte clay miniature, stop-motion puppet, no text, no letters, no numbers, no logos`
- **P3B:** `a stop-motion clay puppet of an ice hockey skater with soft rounded chunky proportions, made of matte deep blue modelling clay with fingerprint dents: a round deep blue clay helmet, a deep blue felt jersey with white stitched seams and a white stitched stripe at the hem, deep blue clay hockey pants, thick white clay mittens, black clay skates, a small clay face with dot eyes, standing in an A-pose with arms and legs clearly separated from the body, hand-made clay miniature, no text, no letters, no numbers, no logos`

| Candidate | Mesh | Rig | Credits (this lane) | Tris (body) | Height | Texture |
|---|---|---|---|---|---|---|
| P4A | Rodin `ee808709-8240-48f2-a01a-6e5451a87f9f` | Meshy `01a1d015-46ca-719b-b1fe-3b24148acaac` | 0.5 R + 5 M | 19,796 | 180 cm | 2048 base colour from the rigged FBX, clean, straight copy |
| P4B | Rodin `2a6834a4-36f5-4011-8f6e-073f4ffe724a` | Meshy `01a1d015-47f4-70f0-8dbb-440d3b289555` | 0.5 R + 5 M | 19,830 | 180 cm | 2048, clean (one small dark smudge on a cheek: an artefact, not a glyph) |
| gB | Meshy meshy-6-lite (S1: preview `01a1cf30-0ea4-76da-95eb-b3e1904af014`, refine `01a1cf31-3e93-7397-8a62-d4ac3a7f665c`; prompt in `B-skater-s1.md`) | Meshy `01a1cf32-6719-70bb-9e69-c26981e29390` (1.7 m) | 0 (S1 paid 20) | 20,722 | 170 cm | 2048. **Cleaned:** a brand-like wordmark on the helmet brow plus marks on the helmet back, 29,621 px changed, 0 outside the mask (`devlog lab-skh-gb-helmet`). |
| P3A | Rodin `2dd1928f-a27f-4a1d-bd25-353d12fba0dd` | Meshy `01a1d015-4939-7601-88af-f12ffd035312` | 0.5 R + 5 M | 19,646 | 170 cm | 2048, clean (light dashes on the skate sides are holes in the blade mesh) |
| P3B | Rodin `7a67b623-0d2d-4e6f-a465-52cad324d033` | Meshy `01a1d015-4a69-7766-ad3c-c3ae33990608` | 0.5 R + 5 M | 18,846 | 170 cm | 2048, clean. The face is blue clay, not skin. |
| Rclay | R (Rodin `c07557c7-cf01-462a-97a3-9f0336c0ff31`) | R's (`01a1cf39-74e0-7496-9b7c-cd8a09ffe4d3`) | 0 | 19,748 | 170 cm | S6's clean PNG with a clay treatment, 2048: softened, saturation x0.85, darks lifted. Plus `T_Skater_Rclay_Normal.png`, **1024, DirectX green-down**: one oval Gaussian dent per 2.2 cm cell, 1.0 to 1.6 x 0.6 to 1.0 cm, 1.1 to 2.5 mm deep, computed in 3D on R's rest mesh so there are no UV-seam lines; median slope 5 deg. |

**Glyph check.** Lane S6's detector, generalised (`skh/tex/extract_detect.py`, by height band), flags 40 to 150 clusters per band on every atlas, R included, so it is only a pointer. The decisions came from 320 to 480 px close-ups (`lab-skh-texcheck-legs`, `tex/sheet_texcheck_heads.png`). No candidate has marks on the lower legs, so the leg cleaner was not needed. gB's helmet wordmark was the only glyph found.

### Measured errors per clip group (cm, re-imported main file; full tables in `skh/summary.md`)

How to read the columns:
- *skate*: the worst lower-skate height on frames where S4's skate is on the ice (take).
- *stick min*: the lowest stick vertex.
- *blade*: the worst blade error while S4's blade is on the ice.
- *grip*: the left-mitten centre's distance from the shaft on two-handed frames, mean / max.
- *in head*: frames with a mitten centre or a stick vertex inside the head radius.

| Group | R (P2, S5 rules) | gB | P4A | P4B | P3A | P3B | Rclay |
|---|---|---|---|---|---|---|---|
| Locomotion: skate / blade / grip | 0.28 / 9.0 (Crossover_R) / 9.7, 17.8 | 0.98 / 1.5 / 8.7, 13.9 | 4.42 (Crossover_L) / 1.4 / 6.3, 14.9 | 2.90 (Crossover_L) / 1.6 / 8.2, 18.9 | 2.23 / 1.5 / 15.0, 21.1 | 2.43 / 0.6 / 6.6, 19.5 | = R |
| Shots | 0.00 / 0.0 / 9.4, 17.7 | 0.00 / 0.4 / 9.4, 17.4 | 0.00 / 0.0 / 7.8, 18.1 | 0.00 / 0.5 / **5.7, 10.6** | 0.00 / 0.4 / **20.0, 30.2** | 0.00 / 0.4 / 11.8, 20.9 | = R |
| Contact (falls, getups, Check, Celebrate) | 10.3 (Fall_Forward) / 20.7 / 5.4, 18.8 | 9.6 (GetUp_Front) / 0.8 / 6.4, 16.1 | **21.2 (GetUp_Front)**; Check 9.2 / 0.6 / 1.9, 17.7 | **18.2 (Fall_Forward)**; Check 6.5 / 0.9 / 2.7, 17.0 | 16.2 (Fall) / 1.0 / 12.7, 23.6 | **24.1 (GetUp)** / 1.0 / 2.4, 33.1 | = R |
| Stick verbs | 0.12 / 0.0 / 6.1, 8.4 | 0.19 / 0.5 / 6.8, 10.9 | 0.20 / 0.0 / **2.0, 6.4** | 0.20 / 0.6 / 2.8, 4.9 | 0.13 / 0.2 / 12.2, 16.9 | 0.15 / 0.4 / 0.8, 4.0 | = R |
| Faceoff | 0.03 / 0.0 / 3.8, 8.7 | 0.04 / 0.6 / 5.8, 9.5 | 0.04 / 0.0 / **1.1, 6.0** | 0.04 / 0.5 / **1.1, 5.1** | 0.03 / 0.4 / 8.1, 17.1 | 0.03 / 0.2 / 0.5, 0.9 | = R |
| Goalie: skate / stick min (S4: 16 to 19) | 1.9 / 25.5 | 3.7 / 38.0 | **14.1** / 32.8 | **16.8** / **19.7** | 9.7 / 31.7 | 10.7 / 17.9 | = R |
| Aims: grip | 6.1, 9.6 | 5.3, 7.1 | **2.1, 3.5** | 4.5, 9.8 | 11.1, 13.5 | **0.4, 0.8** | = R |
| Stick min over all takes / in-head frames | 0.00 / 0 | 0.04 / 0 | 0.01 / 0 | 0.00 / 0 | 0.00 / **9** | 0.00 / 2 | 0.00 / 0 |

- **Longer arms work.** The left mitten is on the shaft within 1 to 2 cm in Glide, Faceoff and the aims on P4A, and within 1 to 4 cm on P4B. On R it is 6 to 10 cm.
- **Where the two-handed grip still misses:** the slap follow-through (11 to 18 cm) and Crossover_Right (15 to 19 cm). S4's own pose separates the hands there.

## What looks wrong (I looked at every sheet)

- **P4A** (`lab-skh-p4a-motion`, `-holds`, `-feet`):
  - **Goalie_Hold from the side:** the raised free glove drags the shoulder into a hood-like blob over the helmet. These are Meshy's shoulder weights, the same fault as R, but much worse on a realistic body.
  - **Skates on toes:** with S4's foot roll, the long skate stands on its toe. Goalie_Block_Left lifts the figure 14 cm and Check lifts it 9 cm (2 frames). Lying poses: 16 to 21 cm.
  - **Face:** a blank face with a beard and no eyes.
  - **Behind the knees:** a sheer grey patch.
  - **At 40 px:** dark and thin.
  - The slap wind-up passes the shaft in front of the visor (occlusion; 0 in-head frames).
- **P4B** (`lab-skh-p4b-*`):
  - Same toe and lying faults: blocks 16.8 cm, Fall_Forward 18.2 cm, Check 6.5 cm.
  - Otherwise clean: the free glove stays in front of the chest, and the shoulder pads give the broadest, most hockey-like silhouette at 40 px.
  - Arms hang close to the body at rest, but I saw no torso smear in the sheets.
  - A small dark smudge on one cheek.
- **gB** (`lab-skh-gb-*`):
  - The thinnest figure, with a small head and white gloves. At 40 px it reads as a dark blob with white specks, as S1 found.
  - Black shorts against navy.
  - The goalie stick floats highest (38 cm).
  - Faint dark specks remain on the helmet brow rim after cleaning.
- **P3A** (`lab-skh-p3a-*`, `lab-skh-compare-p3`):
  - The look: real clay forms with helmet dents, stitched seams and a friendly face. Best 40 px read of all, thanks to the helmet.
  - **Grip:** its short arms and wide mittens leave the left mitten 12 to 20 cm off the shaft (30 cm in the slap). That is worse than R.
  - **Stick near the head:** 9 frames where a mitten or the stick comes inside the big head radius (Stride and the shots, the shaft crossing in front of the face).
  - **Falls:** the skates lift 16 cm in Fall.
  - **Skates:** holes in the blade mesh.
- **P3B** (`lab-skh-p3b-*`):
  - The face is blue clay: no face reads at all.
  - Giant mittens (mitten radius 0.20 m): in the 3/4 views they swamp the shaft.
  - A black ear-hole on the helmet.
  - The worst lying lift: 24 cm in GetUp.
- **Rclay** (`lab-skh-rclay-*`, `lab-skh-clay-normal`):
  - The dents read on the helmet and back in close-ups only. At game distance it is the P2 figure: same silhouette, same face, no stitched seam.
  - Clay is a surface on a cel character, not a hand-made figure.
  - The EEVEE stand-in overexposes the ice and the stick.

**A rule could not fix these:**
- **Toe-standing in kneels and lying poses:** it needs a foot-pitch limit on planted skates, which is S4 pose work.
- **The P4A shoulder:** it needs re-weighting or a clavicle share.

I did not attempt either.

## Rubric rows (1 to 5, judged harshly)

| Candidate | Look fit | Technical | Motion | Cost | Provenance | Import risk |
|---|---|---|---|---|---|---|
| **P4B** Rodin pads player | **4**: the broadest, most hockey-like silhouette (shoulder pads, gloves, pants); believable proportions, stylised not toy. Minus: dark at 40 px; cheek smudge. | 3: 19.8k tris, one 2K base colour (the normal and ORM maps are dropped), 32 bones, `root`, scale 1, A-pose, limbs separate. | **3**: zero travel and seams; the best two-hand grips with R's clips (1 to 4 cm); goalie stick at 19.7 cm, nearest S4. Minus: skates 6.5 to 18 cm off the ice in Check, kneels and falls. | 5: 5.5 credits, about 1 min of generation | 3: mesh ai-generated (Rodin), rig ai-generated (Meshy), clips and stick original | 3: same skeleton as T-B2, so the import path is known; unverified in UE |
| P4A Rodin lean player | 3: realistic and well proportioned, with a visor. Minus: eyeless face, a sheer patch at the knees, a thin silhouette at 40 px. | 3: as P4B | **2**: the best grips (1 to 2 cm), but the goalie shoulder blob is visible at 160 px, skates stand on toes (14 cm in blocks, 21 cm lying), Check 9.2 cm. | 5 | 3 | 3 |
| gB Meshy lean athlete | 2: weakest at 40 px; black shorts; leftover specks after the wordmark cleaning | 3: 20.7k tris, 1.7 m | 3: lowest standing skate errors of the three (Check 1.4 cm); grips 5 to 9 cm; goalie stick 38 cm | 5: 0 new credits | 3: ai-generated (Meshy), **its helmet carried a brand-like wordmark**, now painted out | 3 |
| **P3A** Rodin clay figure | **4**: clearly hand-made clay (dents, stitched seams, round helmet, face); best 40 px read | 3: 19.6k tris, one 2K; holes in the skate blades | **2**: grips 12 to 20 cm (30 in the slap); 9 frames with stick or mitten inside the head radius; falls lift 16 cm | 5: 5.5 credits | 3 | 3 |
| Rclay (R + clay surface) | 3: soft dents and a matte colour on the P2 figure; reads as clay only close up | 4: R's mesh, one 2K base colour plus one 1K normal | **4**: = the P2 hybrid exactly (regression-proven) | 5: 0 credits | 3: mesh ai-generated; normal map and colour treatment `original` (our script) | **4**: no new mesh; a material on T-B2 |
| P3B Rodin clay puppet | 2: blue face, giant mittens, ear-hole | 3: 18.8k tris | 2: lying lift 24 cm; mittens swamp the shaft | 5 | 3 | 3 |

## Ranking and verdicts

**P4:**
1. **P4B: queue for import.** Best silhouette and gear read, best goalie stick, good grips.
2. P4A: keep as fallback. Comparable grips, but the shoulder blob and the eyeless face.
3. gB: drop for gameplay. Weakest at distance and it carried a wordmark.

**P3:**
1. **P3A: queue for import** (the only new mesh worth importing). It is the only candidate that *is* a clay figure; its grip gaps are the price.
2. **Rclay: do it in UE as a material on T-B2**, no import: assign `T_Skater_Rclay_BaseColor.png` and `T_Skater_Rclay_Normal.png` with roughness about 0.9. It is the zero-risk P3 fallback.
3. P3B: drop.

## Files and provenance

| File | Origin | Tool |
|---|---|---|
| `skh/export/P4A`, `P4B`, `P3A`, `P3B` `/SK_Skater_*.fbx` and `_loopcut.fbx`: mesh, UVs, base colour | ai-generated | Hyper3D Rodin Gen-2.5-Low, task ids above |
| same: skeleton and skin weights | ai-generated, modified | Meshy rig (ids above); `root`, sockets, foot pivot and boot weights by `skh/retarget.py` |
| `skh/export/gB/*`: mesh, UVs, texture | ai-generated, texture edited | Meshy meshy-6-lite (S1 ids); helmet wordmark removed by `skh/tex/clean_gB.py` |
| `skh/export/R/*` (the S5 regression build), `Rclay/*`, `Rm/*`: mesh | ai-generated | Rodin `c07557c7...` plus Meshy rig `01a1cf39...` |
| `T_Skater_Rclay_BaseColor.png` | ai-generated, modified | S6's clean PNG with a clay treatment by `skh/tex` |
| `T_Skater_Rclay_Normal.png` | original | `skh/tex/clay_r.py` (procedural) |
| all 31 takes in every file | original | S4's `build_clips.py`, retargeted by `skh/retarget.py` |
| the stick | original | S4's `build_skater.py` |

Every export has one base-colour PNG (2048, 8-bit RGB) beside it; Rclay also has its normal map. No textures are embedded. The track-strip step is not run anywhere: it stays disabled.

## Rebuild

`bash C:/Users/kazda/jam/skater-lab/skh/run_all.sh` takes about 6 min. It runs:
1. retarget and verify (main and loopcut) for R, Rm, Rclay, gB, P4A, P4B, P3A and P3B;
2. the R regression against `s5/export`;
3. `summary.md`;
4. the Workbench sheets.

To rebuild one figure: `bash skh/build.sh <name>`. The textures come from `skh/tex/` (`extract_detect.py`, `clean_gB.py`, `clay_r.py`). EEVEE sheets: `skh/tex/render_clay.py -- --in <fbx> --out <abs dir> --mode front34|side|back --action <take> [--normal <png> --normal-dx]`.

## Contact sheets (`devlog/shots/20261112-1530xx-B-lab-skh-*`)

| Sheet | What it shows |
|---|---|
| `turntables` | the four Rodin meshes as generated |
| `compare-p4` | R, gB, P4A, P4B in Glide 3/4 and the slap wind-up from the side |
| `compare-p3` | R, Rclay, P3A, P3B (EEVEE) in the same two poses |
| `<cand>-motion` for p4a, p4b, gb, p3a, p3b and rclay | Stride 3/4, side and behind; Glide; slap from the side and 3/4 (rclay: Stride rows and the slap from the side) |
| `<cand>-holds` for the same six | turntable at rest; Goalie_Hold front and side; 40 px strips from behind (rclay: no turntable) |
| `p4a-feet` | P4A's foot faults |
| `clay-normal` | R without and with the clay normal map |
| `gb-helmet` | gB's helmet before and after cleaning |
| `texcheck-legs` | the lower-leg glyph check |

## What I could not verify

- **Anything in Unreal:** import, take names, the material slot and texture-by-path handling, the DirectX normal on the cel or clay shader, and sockets.
- **Clay in the game:** whether Rclay's dents survive the game's lighting at 6 m. EEVEE is only a stand-in.
- **Influence count:** whether more than 4 influences per vertex remain after the boot re-weight.
- **The 40 px renders:** they are Workbench downsamples, not the engine at 1080p.
- **Hyper3D export rights** for the four new Rodin meshes (S3's open question).
- **Other figures:** the generalised script is proven identical only for R. On the other figures, "correct" is judged by the measurements and the sheets above.
- **Housekeeping:** my first turntable render used relative paths and wrote `C:\r\tt_P4A`, `tt_P4B`, `tt_P3A` and `tt_P3B`. I moved them into `skh\r\`. `C:\r\rest` (S5's) was not touched.

## Second pass: retarget from the scripted family (16:03)

**Short answer:**
- **R and P3A should take the new export.**
- **P4B:** the new clips are better, but the proportion-matched source `p4_broadcast` has a pop at the Stride loop wrap. Use the **`p2_cel` control** for now, or wait for SKF to fix `p4_broadcast`.
- **Skeleton and bind pose are identical** to `skh/export/` for all exports, so only the animations change.

All of this is measured in Blender on the re-imported FBX. Unreal is unverified.

Paths:

| What | Where |
|---|---|
| New exports | `C:\Users\kazda\jam\skater-lab\skh\export2\<folder>\SK_Skater_<asset>.fbx` and `_loopcut.fbx`, each with its base-colour PNG |
| Measurements | `skh\out2\<folder>\` |
| The table | `skh\old_new.md` |
| Rebuild | `bash C:/Users/kazda/jam/skater-lab/skh/run2.sh` (about 4 min) |

`skh/export/` was not touched.

### What changed in the script

- **The clip source is a config parameter.** `clips_fbx` takes any file with the 24 S4 bone names, a body and a stick. `stick_fbx` takes the stick *mesh* and the stick-socket rest from another file. The second pass takes both from S4's file, so the mesh and the bind stay identical to the first export.
- **New config keys:** `asset` (the asset name in the file, so `export2/P4B_ctl_p2/SK_Skater_P4B.fbx` keeps the old names), `export_dir` and `out_dir`.
- **Reach rule (R-0018).** On `Aim_Front`, `Aim_Left`, `Aim_Right`, `Aim_FarReach` (frame 1) and `Poke` (frame 10), the reach is the horizontal distance from the pelvis to the blade centre. If it misses 1.0 m (aims) or 1.6 m (far reach, poke) by more than 5 cm:
  - the stick slides along its shaft through the mitten: on every frame for the holds, and weighted by the frame's reach for the poke;
  - then the ice tilt runs again;
  - this repeats up to 10 times.
- **Two continuity fixes** for the second pass, found because P4B's Stride wrap jumped 28 cm:
  - The slide now fades out as the shaft flattens. S5's hard switch at a shaft slope of 0.15 sent the slide to its 25 cm clamp.
  - A 5-frame median (cyclic on loops) on the per-frame slide.
- **Tried and turned off:** a median on the tilt angle (it put sticks 16 cm into the ice), and an "untwist" that flips 180-degree limb twists back (it changed 839 to 887 keys per figure and added pops). Both are still in the code behind `if False`.
- **Regression:** R built with the first-pass config is still **0.00000 cm** from `s5/export` (`out_reg/compare_s5.json`).

### Bind check (`bindcheck.py`)

I re-imported `export/<c>` and `export2/<c>` and compared them:

| Compared | Result |
|---|---|
| Bone names and parents | equal |
| Rest matrices, heads, tails | max difference **0.0** |
| Body and stick vertex positions | difference 0.0 |
| Weights, polygons | hashes equal |
| UVs | difference 0.0 |
| Material names | equal |

This holds for `P4B`, `P4B_ctl_p2` (against `export/P4B`), `P3A` and `R`. An animation-only re-import onto the existing skeletons is safe.

The files are not byte-identical (the curves differ, and the texture path points into `export2/`). I compared content on re-import, not the FBX bytes.

### Old against new (re-imported main files; `skh/old_new.md`)

How to read the columns:
- *Two-hand grip*: the left-mitten centre to the shaft on two-handed frames, mean / max.
- *Lying*: skate lift on frames where S4's skate is on the ice, for Fall / Fall_Forward / GetUp / GetUp_Front / Goalie_Block_Left / Goalie_Block_Right.
- *Reach*: before the rule -> after, on the re-imported file.

| Cand. | Source | Two-hand grip, cm | Slap/wrist grip max | Stick or mitt in head (frames) | Skate lift lying/kneeling (cm) | Standing skate err max | Goalie_Hold stick min (S4 16-19) | Blade err max | Reach err cm: Front / Left / Right / FarReach / Poke |
|---|---|---|---|---|---|---|---|---|---|
| P4B | old: S4 | 4.4 / 18.9 | 10.6 | 0 | 0.5 / 18.2 / 10.9 / 18.2 / 16.8 / 16.7 | 6.51 | 19.7 | 1.6 | 10.9 / 9.3 / 14.3 / -1.0 / **36.1** |
| P4B | new: p2_cel (control) | **1.6** / 17.3 | 10.0 | 0 | 0.5 / 17.6 / 10.9 / 17.6 / 16.8 / 16.7 | 6.51 | 21.6 | 1.9 | 14.7->0.9 / 11.2->0.0 / 16.9->1.9 / -0.3 / -3.5 |
| P4B | new: p4_broadcast | **1.3** / 15.7 | 13.0 | 0 | 0.4 / **14.9** / 13.5 / 17.3 / **20.0 / 19.9** | 7.91 | **16.5** | 1.2 | 6.7->0.8 / -10.8->0.0 / 10.4->0.0 / -4.4 / -8.7->0.0 |
| P3A | old: S4 | 13.2 / 28.4 | 30.2 | 9 | 16.2 / 0.8 / 16.2 / 7.4 / 9.7 / 8.9 | 2.23 | 31.7 | 1.5 | -0.6 / -0.9 / -4.7 / 14.0 / **44.5** |
| P3A | new: p2_cel (control) | 10.7 / 29.3 | 23.6 | 11 | 16.2 / 0.3 / 16.2 / 7.4 / 9.7 / 8.9 | 2.23 | 34.1 | 2.2 | 2.5 / 0.7 / 1.6 / 31.4->0.0 / 28.5->0.0 |
| P3A | new: p3_clay | **9.7** / 25.9 | **20.7** | 8 | **13.6** / 0.3 / **13.1** / **0.3** / 9.5 / 8.6 | **1.47** | 34.3 | 2.2 | 3.1 / 5.6->1.4 / -1.1 / 30.6->0.0 / 28.1->0.0 |
| R | old: S4 (= S5) | 6.9 / 18.8 | 17.7 | 0 | 9.5 / 10.3 / 6.0 / 6.5 / 1.9 / 1.8 | 0.29 | 26.3 | **20.7** | 3.3 / 3.2 / 5.2 / 9.0 / **32.8** |
| R | new: p2_cel | **4.4** / 22.9 | **11.9** | 0 | 9.1 / 10.3 / 6.0 / 6.5 / 1.9 / 1.8 | 0.29 | 27.9 | **1.9** | 4.7 / 4.2 / 8.0->0.0 / 5.2->0.5 / 4.2 |

**On every new export:**
- Pelvis travel 0, root 0, loop seams 0.000 cm in the main file. 31 takes.
- Lowest stick vertex at least -0.01 cm.
- Loopcut wraps are one frame step: 1.4 to 2.0 cm on the crossovers, except P3A `Crossover_Right` at 5.4 cm and **P4B (`p4_broadcast`) `Stride` at 19.2 cm** (below).

### What the two effects are

- **Fixed clips (the controls against old):**
  - The bottom hand is on the shaft far more often: P4B mean 4.4 to 1.6 cm, R 6.9 to 4.4 cm.
  - The slap and wrist grips improve.
  - The poke stops overreaching: it was 33 to 45 cm past 1.6 m, now -3.5 to +4.2 cm.
  - R's GetUp_Front blade error drops from 20.7 to 1.9 cm.
- **Proportion matching (p4 / p3 against the p2 control):**
  - Small on the hands.
  - Clear on P3A's lying heights: GetUp_Front 7.4 to 0.3 cm, Fall and GetUp 16 to 13 cm. That is SKF's `lie_add`.
  - Mixed on P4B: Fall_Forward 17.6 to 14.9 cm and the goalie stick 21.6 to 16.5 cm (now inside S4's range), but the kneeling blocks get worse (16.8 to 20.0 cm) and the standing skate error rises (6.5 to 7.9 cm).
- **Lying heights, the question asked:** the family's fixes help P3A and partly P4B. They do nothing for R, because `p2_cel` has no lying fix. The skates still stand on their toes in kneels and falls on the generated meshes (9 to 20 cm). That remains a foot-pitch problem.
- **Reach:** after retargeting, the aims miss by up to 17 cm (P4B) and the far reach and poke by up to 31 cm (P3A's short arms over-extend). After the rule every take is within 5 cm. The largest are R's `Aim_Front` at +4.7 cm and `Poke` at +4.2 cm, which are under 5 cm so the rule did not fire.

### Winner per candidate, and replace or not

| Candidate | Winner | Replace the engine's import? |
|---|---|---|
| **R** (P2) | **`p2_cel`** | **Yes, from `export2/R/`.** Better grips, blade errors and reaches; nothing got worse except the GetUp grip maximum (18.8 to 22.9 cm, one frame). |
| **P3A** (P3) | **`p3_clay`** | **Yes, from `export2/P3A/`.** Best on almost every column; reaches fixed. Watch the `Crossover_Right` loop wrap: a 5.4 cm one-frame step. |
| **P4B** (P4) | **`p4_broadcast` on the numbers**, but it carries a **19 cm right-hand and stick pop at the Stride loop wrap** | **Use `export2/P4B_ctl_p2/` now** (the files are still named `SK_Skater_P4B`; same skeleton). Switch to `p4_broadcast` when SKF removes the flip. |

The P4B pop comes from the source: in `SK_Skater_p4_broadcast.fbx` `Stride`, `upperarm_r` is twisted 180 degrees about its own axis on frames 1 and 61 only. Its joints don't move, so SKF's own figure looks fine. Retargeted onto P4B's arm, it swings the hand 18 cm (`dbg3.py`, `flips.py`).

### Sheets (filed)

| Shot file | Candidate and source |
|---|---|
| `devlog/shots/20261112-160221-B-lab-skh2-r.png` | R from `p2_cel` |
| `devlog/shots/20261112-160221-B-lab-skh2-p3a.png` | P3A from `p3_clay` |
| `devlog/shots/20261112-160221-B-lab-skh2-p4b.png` | P4B from `p4_broadcast` |
| `devlog/shots/20261112-160221-B-lab-skh2-p4b-ctl-p2.png` | P4B from `p2_cel` |

Each sheet has these rows:
- Stride from 3/4, side and behind;
- the slap from the side;
- Goalie_Hold front and side;
- the four aim holds and the Poke at frame 10 from the side, zoomed out so the whole stick is in frame.

### What I could not verify

- **Anything in Unreal:** the animation-only re-import, whether UE accepts the takes onto the existing skeleton asset, and whether the texture path change matters.
- **Twist flips in the sources.** S4 and all three SKF sets carry 156 to 178 frame-to-frame 180-degree twists on arm bones (`flips.py`). On the retargeted figures they show as hand pops of up to 58 to 80 cm in one frame in GetUp, Fall and Stumble. **These were in the first-pass exports too**; they are not new, but they are not fixed. The fix belongs in the source solver (SKF/S4: keep IK twist continuous). My retarget-side untwist made things worse and is off.
- **Toe-standing** in kneels and lying poses on the generated meshes (9 to 20 cm): unchanged; it needs a foot-pitch limit.
- **Reach on aims seen from the side:** `Aim_Left` and `Aim_Right` are foreshortened in the side view. Their numbers come from `verify.py`, not the picture.

## Third pass: twist-fixed clips (16:33)

**Short answer:**
- **`export3` should replace `export2` in the engine for R, P3A and P4B.**
- **For P4B the matched source `p4_broadcast` now wins:** the Stride pop is gone (loop wrap 0.5 cm hand, 0.3 cm blade; was 18.5 and 15.1 cm).
- **Lying and kneeling skate lift is mostly fixed** by the source's foot-pitch limit.
- **Skeleton, bind and mesh equal `skh/export/`**, so only the animations change.

All of this is measured in Blender on the re-imported FBX. Unreal is unverified.

Paths:

| What | Where |
|---|---|
| Exports | `C:\Users\kazda\jam\skater-lab\skh\export3\{R,P3A,P4B,P4B_ctl_p2}\SK_Skater_<asset>.fbx` and `_loopcut.fbx`, each with its base-colour PNG |
| Measurements | `skh\out3\<folder>\`; the hand jumps in `skh\jump\` |
| The table | `skh\three_final.md` |
| Inputs | copies of `skf/export2` (16:17) in `skh\in\skf2\` |
| Configs | `cfg/3_*.json` |
| Rebuild one figure | `bash skh/build3.sh <folder>` |

### Incident: `export2` was overwritten for 91 s and restored

My first third-pass configs kept `"export_dir": "export2"` (a sed slip). From **16:21:53 to 16:22:05** the third-pass builds wrote the files listed below into `skh/export2/`, replacing the second-pass files there:
- `R/SK_Skater_R.fbx` and `_loopcut`
- `P3A/SK_Skater_P3A.fbx` and `_loopcut`
- `P4B/SK_Skater_P4B.fbx` and `_loopcut`
- `P4B_ctl_p2/SK_Skater_P4B.fbx` and `_loopcut`

I moved those files to `export3/` and rebuilt `export2/` from the unchanged second-pass configs. The rebuild finished at **16:23:24**.

The restored files are proven identical to the 16:00 files:
- **sizes:** byte counts identical;
- **verification:** re-verifying all 31 takes gives numbers identical to the 16:00 `out2/verify.json`.

**Any import from `skh/export2/` that read files between 16:21:53 and 16:23:24 should be re-run.**

### Did the source fix remove the jumps?

`handjump.py` measures the largest one-frame move of `hand_l` and `hand_r` and of the blade centre. It reads the main file, whose last step is the loop wrap.
- **Most of the remaining "over 15 cm" steps are real speed.** The fixed sources themselves have 120 to 124 such steps; a slap blade moves 80 to 90 cm in one frame at 60 fps.
- **The big hand pops were mine.** GetUp frame 3 (58 to 80 cm) and Fall frame 27 (55 to 66 cm) were in every pass. They were not source twists: the left-hand IK weight jumped from 1 to 0 in one frame, when S4's glove-to-shaft distance crossed the 5 to 12 cm band.
- **New rule, this pass:** the grip weight may now change by at most 0.10 per frame, forward and backward, cyclic on loops.
- **Result:** the largest hand step is now 31 / 32 / 38 cm (R / P3A / P4B), in Shot_Slap and Fall. The source's own largest is 40 cm, in Shot_Slap.
- **The price:** the left mitten blends onto the shaft over about 10 frames, so the two-handed grip means rise by 1 to 2 cm, and Stumble's maximum rises to 30 to 34 cm.

### Continuity fixes

- **Left on, but no longer needed for the pop they targeted:** the slide fade and the 5-frame slide median from the second pass. Their target, the P4B Stride flip, came from the source and is gone. I did not test whether they still change anything.
- **Left off:** the tilt median and the untwist pass.
- **R regression:** still 0.00000 cm from `s5/export` (`out_reg/compare_s5.json`).

### Bind check (`out3/<c>/bindcheck.json`, against `skh/export/`)

| Compared | Result |
|---|---|
| Bone names and parents | equal |
| Rest matrices | max difference 0.0 |
| Body and stick vertices | difference 0.0 |
| Weights, polygons | hashes equal |
| UVs | difference 0.0 |

This holds for R, P3A, P4B and the P4B control (against `export/P4B`).

### Old / second / third (re-imported main files)

How to read the columns:
- *Grip*: left mitten to shaft on two-handed frames, mean / max (take).
- *Lift*: skate lift on frames where S4's skate is on the ice, for Fall / Fall_Forward / GetUp / GetUp_Front / Goalie_Block_Left / Goalie_Block_Right.
- *Reach*: before the rule -> after.
- *Jump*: largest one-frame move of hand (take) / blade (take); then the number of steps over 15 cm; then Stride's own maximum hand and blade step / its loop wrap.

| Cand. | Pass (source) | Grip mean / max | Slap/wrist max | Stick/mitt in head | Lift: Fall / Fall_Fwd / GetUp / GetUp_Front / Block L / Block R | Goalie stick (S4 16-19) | Blade err max | Reach err: Front / Left / Right / FarReach / Poke | Largest jump: hand / blade | Steps > 15 cm | Stride max / wrap |
|---|---|---|---|---|---|---|---|---|---|---|---|
| R | export (S4) | 6.9 / 18.8 (GetUp) | 17.7 | 0 | 9.5 / 10.3 / 6.0 / 6.5 / 1.9 / 1.8 | 26.3 | 20.7 | 3.3 / 3.2 / 5.2 / 9.0 / 32.8 | 57.8 (GetUp) / 78.9 (Slap) | 130 | 4.0, 3.9 / 0.4, 0.2 |
| R | export2 (p2_cel) | 4.4 / 22.9 (GetUp) | 11.9 | 0 | 9.1 / 10.3 / 6.0 / 6.5 / 1.9 / 1.8 | 27.9 | 1.9 | 4.7 / 4.2 / 8.0->0.0 / 5.2->0.5 / 4.2 | 58.2 (GetUp) / 90.5 (Slap) | 119 | 3.5, 2.9 / 0.4, 0.2 |
| R | **export3** (p2_cel fixed) | 6.5 / 29.8 (Stumble) | 11.4 | 0 | **0.3 / 5.1 / 0.9 / 5.1 / 0.2 / 0.2** | 27.9 | 2.3 | 3.4 / 0.7 / 6.9->0.7 / 5.2->0.5 / 3.7 | **30.8 (Slap)** / 90.4 (Slap) | 121 | 3.2, 2.9 / 0.4, 0.2 |
| P3A | export (S4) | 13.2 / 28.4 (Shot_Charge) | 30.2 | 9 | 16.2 / 0.8 / 16.2 / 7.4 / 9.7 / 8.9 | 31.7 | 1.5 | -0.6 / -0.9 / -4.7 / 14.0 / 44.5 | 56.8 (GetUp) / 99.7 (Slap) | 168 | 8.2, 3.8 / 0.4, 0.2 |
| P3A | export2 (p3_clay) | 9.7 / 25.9 (Crossover_R) | 20.7 | 8 | 13.6 / 0.3 / 13.1 / 0.3 / 9.5 / 8.6 | 34.3 | 2.2 | 3.1 / 5.6->1.4 / -1.1 / 30.6->0.0 / 28.1->0.0 | 57.3 (GetUp) / 94.5 (Slap) | 144 | 8.3, 5.7 / 0.4, 0.2 |
| P3A | **export3** (p3_clay fixed) | 11.4 / 32.3 (Stumble) | 19.7 | 7 | **0.3 / 0.4 / 0.4 / 0.4 / 0.3 / 0.4** | 34.3 | 2.0 | 2.1 / 1.1 / -1.2 / 30.6->0.0 / 26.3->0.0 | **32.2 (Fall)** / 93.7 (Slap) | 147 | 4.7, 5.9 / 0.4, 0.2 |
| P4B | export (S4) | 4.4 / 18.9 (Crossover_R) | 10.6 | 0 | 0.5 / 18.2 / 10.9 / 18.2 / 16.8 / 16.7 | 19.7 | 1.6 | 10.9 / 9.3 / 14.3 / -1.0 / 36.1 | 78.6 (GetUp) / 78.3 (Slap) | 157 | 18.6, 3.7 / 0.5, 0.2 |
| P4B | export2 (p4_broadcast) | 1.3 / 15.7 (GetUp) | 13.0 | 0 | 0.4 / 14.9 / 13.5 / 17.3 / 20.0 / 19.9 | 16.5 | 1.2 | 6.7->0.8 / -10.8->0.0 / 10.4->0.0 / -4.4 / -8.7->0.0 | 80.2 (GetUp) / 87.4 (Slap) | 153 | 18.9, 15.1 / **18.5, 15.1** |
| P4B | **export3** (p4_broadcast fixed) | 3.6 / 33.7 (Stumble) | 14.5 | 0 | **1.3 / 14.9 / 0.2 / 14.9 / 0.2 / 0.2** | **16.8** | 1.9 | 8.1->1.7 / 8.9->0.8 / 11.5->0.8 / -4.4 / -8.0->0.0 | **38.2 (Slap)** / 87.5 (Slap) | 128 | 6.5, 3.8 / **0.5, 0.3** |
| P4B | export2 ctl (p2_cel) | 1.6 / 17.3 (GetUp) | 10.0 | 0 | 0.5 / 17.6 / 10.9 / 17.6 / 16.8 / 16.7 | 21.6 | 1.9 | 14.7->0.9 / 11.2->0.0 / 16.9->1.9 / -0.3 / -3.5 | 78.9 (GetUp) / 97.5 (Slap) | 147 | 18.8, 4.4 / 0.5, 0.2 |
| P4B | export3 ctl (p2_cel fixed) | 3.2 / 34.0 (Stumble) | 8.8 | 0 | 1.0 / 17.6 / 0.3 / 17.6 / 0.3 / 0.3 | 21.6 | 2.1 | 13.8->3.8 / 13.1->0.0 / 16.9->1.9 / -0.3 / -3.1 | 51.9 (Fall) / 93.0 (Slap) | 136 | 6.5, 3.6 / 0.5, 0.2 |

**On every `export3` file:**
- 31 takes; pelvis travel 0; root 0; loop seams 0.000 cm in the main file.
- Lowest stick vertex at least -0.01 cm.
- Standing skate error at most 0.5 cm on R and P3A, and 6.4 cm on P4B (Check).

### Skate lift: before against after

- **Kneels, the goalie blocks and GetUp:** now 0.2 to 0.9 cm on every figure. Before: 1.8 to 20 cm.
- **Fall:** 0.3 to 1.3 cm. Before: 0.4 to 16 cm.
- **Fall_Forward and GetUp_Front on the tall figures** still lift 5.1 cm (R) and 14.9 cm (P4B; 17.6 cm in the control). The chest and arms touch first when lying face down, so the skates are held up by the body, not by the foot. P3A is at 0.4 cm.

### Per candidate

| Candidate | Replace the engine's import with `export3`? |
|---|---|
| **R** | **Yes, from `export3/R/`.** Lying lift down from 10 cm to 5 cm and below; the hand pops are gone. The grip mean rises from 4.4 to 6.5 cm (the slower blend). |
| **P3A** | **Yes, from `export3/P3A/`.** Every lift 0.4 cm or less, hand pops gone, reaches fixed. The grip mean rises from 9.7 to 11.4 cm. |
| **P4B** | **Yes, from `export3/P4B/`. The matched source `p4_broadcast` now wins** over the `p2_cel` control: goalie stick 16.8 against 21.6 cm (inside S4's range), lying lift 14.9 against 17.6 cm, and no Stride pop. The control's remaining advantage is a lower slap and wrist maximum (8.8 against 14.5 cm). |

### Sheets (filed)

| Shot file | What it shows |
|---|---|
| `devlog/shots/20261112-163202-B-lab-skh3-r-falls.png` | R: Fall, Fall_Forward, GetUp, GetUp_Front from the side |
| `devlog/shots/20261112-163202-B-lab-skh3-p3a-falls.png` | P3A: the same four takes |
| `devlog/shots/20261112-163203-B-lab-skh3-p4b-falls.png` | P4B: the same four takes |
| `devlog/shots/20261112-163203-B-lab-skh3-p4b-stride.png` | P4B Stride from 3/4, side and behind |

### What I could not verify

- **Anything in Unreal**, including whether the importer read `export2` during the 16:21:53 to 16:23:24 window.
- **Whether the continuity fixes still change anything** (slide fade, slide median): left on, not re-tested against the fixed sources.
- **The grip-rate trade:** 0.10 per frame removes the pops but holds the left mitten off the shaft for longer. A rule that rate-limits the hand *target* instead of the weight might keep both; not tried.
