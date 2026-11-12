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

## Fourth pass: aim grid (17:37)

**Short answer:**
- **`export4` should replace `export3` in the engine for all three**, because only it has the 24-hold aim grid.
- On every figure:
  - every hold is on its point within 6.1 cm and 2.1 degrees;
  - the blade sits 0.0 to 2.3 cm up, never below the ice;
  - the pelvis and legs equal the candidate's own Glide exactly (0.000 cm);
  - the blend test never misses by more than 8 cm (worst 6.5 cm).
- **But the blade dips below -1 cm in some blends:** R 24 of 53, P3A 15, P4B 4.
- **Watch these:**
  - P3A's stick-in-head frames rose from 7 to 21;
  - the bottom hand on P4B's right-side holds is 2 to 10 cm off the shaft;
  - the one-handed holds have a 13 to 16 cm free-hand jump at frame 8, which comes from the source (see below).
- **Skeleton, bind and mesh equal `skh/export/`** (zero difference), so only the animations change.

Paths:

| What | Where |
|---|---|
| Exports | `C:\Users\kazda\jam\skater-lab\skh\export4\{R,P3A,P4B}\SK_Skater_<c>.fbx` and `_loopcut.fbx` (55 takes), with the base-colour PNG |
| Measurements | `skh\out4\<c>\` (`grid.json`, `rear.json`, `verify*.json`, `bindcheck.json`) |
| Hand jumps | `skh\jump\<c>_4.json` |
| The table | `skh\four.md` |
| Inputs | copies of `skf/export3` (17:19) in `skh\in\skf3\` |
| Configs | `cfg/4_*.json`: `export_dir` "export4", checked before the first build |
| Rebuild one figure | `bash skh/build4.sh <c>` |

`export/`, `export2/` and `export3/` were not touched; their newest file is still from 16:30.

### New rules (second-pass configs and later only; R's S5 regression is still 0.00000 cm)

- **The grid holds** `Aim_<N|P><angle>_R<cm>`:
  - they are loops, measured at frame 1, with the reach rule aimed at their own reach;
  - the blade target is the ice (1.5 cm) whatever the source stick length gives. P4B's blades floated 4 to 5 cm before this, because the source stick is 6 cm longer than the S4 stick on our figures.
- **Sideways rule.** If a hold's angle is more than 3 degrees off, the stick turns about the grip, around the vertical axis, iterated together with the reach slide. Then the left hand re-grips.
  - Before the rule: P4B up to -9.6 degrees, P3A up to -42.6 degrees.
  - After: within 0.4 degrees on every corrected hold.
- **Grip weight constant within a hold.** It did not change the frame-8 jump (below), but it is harmless.

### 1. Every hold (re-imported main file, frame 1)

How to read the columns:
- *Position error*: blade centre against the asked point (angle and reach from the pelvis), worst.
- *Blade lowest*: blade bottom above the ice; the target is 0 to 2.5 cm.
- *Mitten centre to shaft*: the shaft runs inside the mitten while this is under the mitten radius.

| | R | P3A | P4B |
|---|---|---|---|
| Position error, worst | 6.1 cm (`N30_R130`; under the 3-degree and 5 cm thresholds, so no rule) | 1.4 cm | 3.6 cm (`P70_R100`) |
| Angle error, worst | 2.05 deg | 0.27 deg | 2.08 deg |
| Reach error, worst | 4.1 cm | 1.4 cm | 1.5 cm |
| Blade lowest (0 to 2.5) | 0.18 to 1.32 | 0.01 to 2.09 | 0.64 to 2.28 |
| Pelvis and legs against Glide | 0.000 cm | 0.000 | 0.000 |
| Mitten radius | 13.3 cm | 14.0 cm | 9.3 cm |
| Mitten centre to shaft, two-handed holds | 5.5 to 13.9 cm | 6.4 to 13.3 cm | 5.3 to 18.8 cm: **off the shaft by 2 to 10 cm** on the -30 / -46 / -63 columns |
| One-handed holds (source list) | `N63_R160` 82.6 cm, `N46_R160` 80.4 cm: free hand, as in the source | `N63_R145`, `N63_R160`, `N46_R160`, `N30_R160`: 62 to 64 cm, free | `N63_R160` 113 cm: free |

### 2. Blend test on the retargeted skeletons

Same method as SKF:
- 53 poses: the midpoints of neighbouring holds and the cell centres;
- `spine_01` and below blended as sign-aligned, normalised quaternion sums of mesh-space rotations;
- the bones' parent-space translations blended linearly, because the stick slides through the mitten;
- pelvis and legs from Glide.

| | Worst position error | Over 8 cm | Blade height range | Cases outside -1 to +4 cm |
|---|---|---|---|---|
| R | 6.5 cm (`N46_R160` / `N30_R160`) | 0 | **-4.83** to +1.22 | **24**: all dips, mostly -1.0 to -2.4; worst `N46_R160`/`N30_R160` -4.83 |
| P3A | 4.2 cm (`P70_R100` / `P70_R130`) | 0 | **-6.50 to +6.27** | **15**: worst `P35_R145`/`P70_R145`/`P35_R160`/`P70_R160` cell -6.50, `P35_R160`/`P70_R160` -6.46, `N30_R160`/`P00_R160` +6.27 |
| P4B | 6.2 cm (cell `N30`/`P00` x `R145`/`R160`) | 0 | -3.08 to +2.15 | **4**: `N30_R160`/`P00_R160` -3.08, that cell -1.40, `P00_R160`/`P35_R160` -1.26, the `P00`/`P35` x `R145`/`R160` cell -1.03 |

The full list is in `out4/<c>/grid.json`, key `blend_flags`.

**Why the dips:** the ice tilt and the slide differ from hold to hold, so the blended stick is not where either hold put it. **The engine needs a blade-on-ice correction after the blend**, or a denser grid where P3A goes one-handed.

### 3. Rear-high (behind, 30 deg up; sheet `lab-skh4-rearhigh`)

**Number:** the share of upper-back points (body vertices of `spine_02`/`spine_03` facing the camera, inside a 40 x 30 cm box) that an arm, a mitten or the stick hides.

| | Glide | Stride f1 / f15 / f30 | `Aim_P00_R130` | `Aim_N63_R160` | `Aim_P70_R130` |
|---|---|---|---|---|---|
| R, `export4` | 0 % | 0 / 0 / 0 | 0 | 0 | 0 |
| P3A, `export4` | 0 | 0 / 0 / 0 | 0 | 0 | 0 |
| P4B, `export4` | 0 | 0 / 0 / 0 | 0 | 0 | **13 %** (right glove at the right shoulder) |

For comparison, the `export3` aliases: `Aim_Right` covered **56 %** on R and **18 %** on P4B; everything else was 0 %.

**How to read it:**
- The samples are coarse: 5 to 17 points per view.
- On the sheet, R's and P3A's top-hand mitten shows over the **lower back and hip** in Glide and the holds. That is below the 40 x 30 cm box, so a number low on the back could still be partly hidden.
- I judged by eye only for that region.

### 4. P4B `Lift`, `Pass`, `Poke` frames 6 to 11: left mitten centre to the shaft (cm)

| Take | `export3` | `export4` |
|---|---|---|
| Lift | 0.3 0.3 0.4 0.4 0.4 0.4 | 2.7 2.7 2.0 0.1 0.1 0.1 |
| Pass | 0.4 0.4 0.3 0.3 0.2 0.2 | 2.8 1.3 3.2 3.3 4.5 1.9 |
| Poke | 0.3 2.7 1.6 3.5 6.4 9.7 | 3.0 5.3 7.2 15.9 20.6 42.6 (one-handed by design; the free arm goes out for balance) |

All of these are inside P4B's 9.3 cm mitten radius except Poke, so both hands stay on the shaft in Lift and Pass.

### 5. Third-pass checks again

| | Value |
|---|---|
| Takes | 55 |
| Pelvis travel, root | 0.000 |
| Stick lowest | -0.01 cm |
| Skate lift | unchanged from `export3`: R 0.2 to 5.1 cm, P3A at most 0.4 cm, P4B 0.2 to 14.9 cm (face down) |
| Bind check | names, parents, rest (0.0), vertices (0.0), weights, polygons and UVs equal `skh/export/` for R, P3A, P4B |

`export3` against `export4` (main file):

| | Left hand to shaft, two-handed, mean / max | Stick or mitt in head (frames) | Goalie stick | Blade err max | Reach err (aliases) Front / Left / Right / FarReach / Poke | Largest hand / blade step | Steps > 15 cm | Stride max / wrap |
|---|---|---|---|---|---|---|---|---|
| R 3 -> 4 | 6.5 / 29.8 -> 7.0 / 29.6 | 0 -> 0 | 27.9 -> 26.4 | 2.3 -> 3.5 (Stride) | 3.4 / 0.7 / 0.7 / 0.5 / 3.7 -> 2.6 / -0.6 / 0.8 / 0.0 / -0.7 | 30.8 / 90.4 -> 30.1 / 88.1 | 121 -> 120 | 3.2, 2.9 / 0.4, 0.2 -> 4.5, 9.4 / 0.4, 0.2 |
| P3A 3 -> 4 | 11.4 / 32.3 -> 8.2 / 33.7 | **7 -> 21** | 34.3 -> 32.5 | 2.0 -> 3.4 (Stride) | 2.1 / 1.1 / -1.2 / 0.0 / 0.0 -> 0.6 / 0.0 / 0.8 / 0.0 / 0.0 | 32.2 / 93.7 -> 32.5 / 89.4 | 147 -> 144 | 4.7, 5.9 / 0.4, 0.2 -> 6.7, 12.5 / 0.7, 0.6 |
| P4B 3 -> 4 | 3.6 / 33.7 -> 6.9 / 34.0 | 0 -> 0 | 16.8 -> 17.3 | 1.9 -> 1.9 | 1.7 / 0.8 / 0.8 / -4.4 / 0.0 -> 0.0 / 0.1 / 1.6 / 0.0 / 0.0 | 38.2 / 87.5 -> 40.9 / 85.1 | 128 -> 131 | 6.5, 3.8 / 0.5, 0.3 -> 9.1, 18.2 / 4.9, 6.9 |

**Loop wraps (loopcut file).** Most are a frame step. These are larger:

| Wrap | Size | Cause |
|---|---|---|
| One-handed holds (R `N46`/`N63_R160`; P3A `N63_R145`, `N63`/`N46`/`N30_R160`; P4B `N63_R160`) | 13 to 16 cm | `LeftHand_End` jumps at **frame 8 inside the hold**. The **source** has the same jump: `hand_l` moves 5.5 cm at frames 7 and 8 in `SK_Skater_p2_cel.fbx` `Aim_N63_R160` and `Aim_N46_R160`. Our figures' longer forearm and mitten amplify it. **For SKF:** a one-handed hold's free hand should not move. |
| P4B `Stride` | 8.2 cm (main seam 0.85 cm) | SKF reports its own P4 Stride wrap as 4.5 cm. Ours is larger. Not a pop at 60 fps, but visible. Not fixed. |
| P3A `Crossover_Right` | 21.5 cm | Already 19.3 cm in `export3`: **I missed it in the third pass**. Not investigated. |

### Replace?

| Candidate | Replace with `export4`? |
|---|---|
| **R** | **Yes.** The grid is clean per hold. Note the blade dipping in 24 blends (to -4.8 cm). |
| **P3A** | **Yes, for the grid**, but the engine should clamp the blade height after the blend (-6.5 to +6.3 cm), and stick-in-head frames tripled (7 to 21). |
| **P4B** | **Yes.** The most robust blend (4 cases, worst -3.1 cm). The weakest point is the bottom hand on the right-side holds (2 to 10 cm off the shaft). |

### Sheets (filed)

| Shot file | What it shows |
|---|---|
| `devlog/shots/20261112-173639-B-lab-skh4-r-aimgrid-above.png` | R's aim grid from above |
| `devlog/shots/20261112-173639-B-lab-skh4-p3a-aimgrid-above.png` | P3A's aim grid from above |
| `devlog/shots/20261112-173640-B-lab-skh4-p4b-aimgrid-above.png` | P4B's aim grid from above |
| `devlog/shots/20261112-173640-B-lab-skh4-rearhigh.png` | rows R, P3A, P4B: Glide, Stride frames 1 / 15 / 30, `Aim_P00_R130`, `Aim_N63_R160`, `Aim_P70_R130` |

In the grid sheets, columns run +70 to -63 degrees with the skater's left on the image left; rows run 1.00 to 1.60 m. The orthographic top camera crops the blade tip at 1.6 m on the outer columns.

### What I could not verify

- **The engine's real blend space:** its triangulation may differ from my linear cells.
- **Back occlusion low on the back:** the samples are coarse, and the hip and lower back were judged by eye.
- **Two wraps:** why P4B Stride's main seam is 0.85 cm, and the P3A Crossover_Right wrap.
- **Anything in Unreal.**

## Fifth pass: fixes, falls and collisions (18:20)

**Short answer:**
- **R: `export5` replaces `export4`.**
- **P4B: `export5` replaces `export4`, with four takes held back:** `Knockdown_Left`, `Knockdown_Right`, `Knockdown_Back`, `Trip`.
- **P3A: `export5` should replace `export4` only for the 55 old takes plus the takes that pass below.** `Board_Hit`, `GetUp_Side_Left` and `Board_Pin` are not good enough.
- **Skeleton, bind and mesh equal `skh/export/` on all three** (zero difference), so only the animations change.

All of this is measured in Blender on the re-imported FBX. Unreal is unverified.

Paths:

| What | Where |
|---|---|
| Exports | `C:\Users\kazda\jam\skater-lab\skh\export5\{R,P3A,P4B}\SK_Skater_<c>.fbx` and `_loopcut.fbx` (70 takes), with the base-colour PNG |
| Measurements | `skh\out5\<c>\` (`grid.json`, `rear.json`, `newtakes.json`, `verify*.json`, `bindcheck.json`) |
| Hand jumps | `skh\jump\<c>_5.json`; the sources' own jumps in `skh\jump\src4_<style>.json` |
| The table | `skh\five.md` |
| Inputs | copies of `skf/export4` (17:57) in `skh\in\skf4\` |
| Configs | `cfg/5_*.json`: `export_dir` "export5", checked before the build |
| Rebuild one figure | `bash skh/build5.sh <c>` |

`export/` to `export4/` were not touched; their newest file is still from 17:35.

### New rules this pass (second-pass configs and later; R's S5 regression is still 0.00000 cm)

- **P3 child grip, `grip_offset_m` -0.10 in `cfg/5_P3A.json`.** SKF's P3 now holds the stick 10 cm below the knob (grip to heel 1.30 m). Our mesh keeps the 1.40 m S4 stick, so the bind stays identical. The stick therefore sits 10 cm further up through the top hand on every frame.
- **Loop seam:** a loop's last frame is now a copy of its first when the source's are equal. The per-frame tilt continuity had left P4B Stride's main seam at 0.85 cm and P3A Board_Pin's at 0.65 cm; both are now 0.
- **Skate-turn clamp, `FOOT_STEP_MAX` 25 degrees per frame.** A bigger frame-to-frame turn of a foot is cut to 25 degrees by a slerp about the ankle, and caught up over the following frames.
  - It fires on 23 to 25 frames per figure, almost all in `GetUp_Side_*` and `Trip` frame 1.
  - Before: `GetUp_Side_*` turned the skate 46 to 54 degrees in one frame on R, 50 to 63 on P3A and **80 to 97 on P4B**.
  - After: 25.0 degrees on every figure.
  - **Is it safe?** The skate stays on the ice: lowest skate 0.0 cm, lowest stick -0.01 cm. The knockdown-to-get-up joins stay at 0.000 cm.
  - One cost: in those 2 to 4 frames the boot lags the shin by up to about 70 degrees on P4B, so the ankle visibly kinks. I judged it from the side sheets only.

### 1. The fourth-pass checks again (`export4` -> `export5`)

| | R | P3A | P4B |
|---|---|---|---|
| Holds: position / angle error, worst | 6.1 cm / 2.05 deg -> **same** | 1.4 / 0.27 -> 7.4 cm / 2.8 deg (`grip_offset_m` moves the stick; still under the rule thresholds) | 3.6 / 2.1 -> 3.7 / 2.1 |
| Holds: blade lowest (0 to 2.5) | 0.0 to 1.5 | 0.0 to 1.0 | 0.8 to 2.3 |
| Holds: pelvis and legs against Glide | 0.000 | 0.000 | 0.000 |
| Holds: bottom-hand mitten centre to shaft, two-handed (mitten radius) | 3.1 to 13.9 (13.3) | **0.2 to 10.7** (14.0); free in `N63_R130/R145/R160`, `N46_R145/R160`, `N30_R160` as in the source | 0.4 to 19.0 (9.3): **still 2 to 10 cm off on the right-side columns** |
| Blend: worst position error | 6.5 -> 6.45 cm | 4.2 -> 4.0 | 6.2 -> 6.1 |
| Blend: blade height | **-4.83 -> -4.82** | **-6.50 -> -3.63** | **-3.08 -> -2.38** |
| Blend: cases outside -1 to +4 cm | 24 -> **21** | 15 -> **18** | 4 -> **2** |
| Rear-high, upper back covered (Glide, Stride 1/15/30, `P00_R130`, `N63_R160`, `P70_R130`) | 0 % everywhere | 0 % everywhere | 0 % except `P70_R130` 13 % (right glove at the shoulder; unchanged) |
| Stick or mitt in head (frames) | 0 | 21 -> **39**: `Board_Pin` 36, `Board_Hit` 3; the old takes are now 0 | 0 |
| Skate lift Fall / Fall_Fwd / GetUp / GetUp_Front / Block L / R | 0.3 / 5.2 / 0.7 / 5.2 / 0.2 / 0.2 | 0.3 / 0.4 / 0.4 / 0.4 / 0.3 / 0.4 | 1.4 / 15.0 / 0.2 / 15.0 / 0.2 / 0.2 |
| Pelvis travel, root, stick lowest | 0, 0, -0.01 | 0, 0, -0.01 | 0, 0, 0.00 |

**Does the static, hip-level source reduce the blend dips?**
- **P3A: yes,** from -6.5 to -3.6 cm.
- **P4B: yes,** from -3.1 to -2.4 cm, with 2 cases left.
- **R: no.** Its -4.8 cm dip is between `N46_R160` and `N30_R160`, where the retarget's own per-hold stick slide and tilt differ. **The engine still needs a blade-on-ice correction after the blend** for R and P3A.

### Loop wraps (loopcut file) and hand jumps

- **P3A `Crossover_Right`: gone.** The wrap is 0.2 / 0.1 cm (hand / blade), down from 21.5 cm. SKF's leg reach guard and hips at Glide height fixed the source. I never traced which bone made the 21.5 cm; it no longer matters.
- **One-handed holds: gone.** The 13 to 16 cm frame-8 jump is 0, because the source holds are now static.
- **P4B `Stride`: smaller, not gone.**
  - Main seam 0.85 -> 0.00 cm (the new loop-seam rule).
  - Loopcut wrap 8.2 -> 4.6 cm.
  - Wrap step, hand / blade: 2.5 / 7.8 cm. The take's largest within-cycle steps are 6.5 / 16.3 cm, so the wrap is a frame of normal stride speed, not a pop. SKF fixed the source wrap: 4.5 cm hand, below 1.2 times the mean.
- **Hand and blade steps.** Steps over 15 cm grew from 120 / 144 / 131 to 178 / 218 / 187 (R / P3A / P4B). The new hits and knockdowns are fast arcade swings at frames 1 to 6. The fourth-pass sources have about the same steps (e.g. P4 `Trip` blade 128 cm, `Knockdown_Back` hand 75 cm at frame 1).
- **Retarget-only pops (the step is more than 130 cm and the source has none):**
  - **P3A `Board_Hit`, 196 cm at frame 11.** The ice tilt jumps from 34 degrees to -2.5 degrees as the source blade comes down to the ice.
  - **P3A `GetUp_Side_Left`, 164 cm at frame 18.** The stick slide flips from -25 to +25 cm while the stick is planted as a prop, below the ice in the source.
  - R and P4B have none.

### 2. The new takes on the skinned figures (`newtakes.json`; lowest points over the take, cm)

| Take | R: lowest non-skate part / skate / stick | P3A | P4B | Hand on shaft, mean / max (R / P3A / P4B) |
|---|---|---|---|---|
| `Check_Hit_Left/_Right/_Back` | thighs or shins 29 / 0.0 / 0.0 to 0.2 | shins 22 / 0.1 / 0.0 | shins 18 to 19 / 0.0 / 3.8 to 4.3 | 11 to 16 / 10 to 14 / 10 to 12; free arm thrown out at the extreme (max 26 to 38) |
| `Knockdown_Left` | thighs 10.2 / 0.0 / 0.0 | thighs 0.8 / 0.1 / 0.0 | hands 5.7 / 0.0 / 0.2 | 24 / 23 / 22 |
| `Knockdown_Right` | thighs 3.1 / 0.0 / 0.7 | thighs 0.8 / 0.1 / 0.0 | shins 9.3 / 0.0 / 4.6 | 18 / 23 / 16 |
| `Knockdown_Back` | hands 0.1 / 0.0 / 0.7 | torso 6.0 / 0.1 / 0.0 | hands 0.1 / 0.0 / 2.2 | 22 / 22 / 20 |
| `Trip` | hands 0.1 / 0.0 / 0.7 | torso 6.0 / 0.1 / 0.0 | hands 0.1 / 0.0 / 4.6 | 24 / 29 / 17 |
| `GetUp_Side_Left/_Right` | thighs 4.5 to 10.9 / 0.0 / 0.0 | thighs 0.7 to 2.6 / 0.1 / 0.0 | thighs 8 to 9 / 0.0 / 0.0 to 0.2 | 4.1 / 1.2 / 9.3 |
| `Board_Hit` | thighs 29 / 0.0 / 0.0 | shins 21 / 0.1 / 0.0 | shins 18 / 0.0 / 4.5 | 7.2 / 7.2 / 6.8 |
| `Board_Pin` (loop) | shins 28.6 / 0.1 / 6.7 | shins 19.2 / 1.1 / 0.0 | thighs 22 / 0.1 / 60.7 | one hand |
| `Check_Deliver_Left/_Right` | shins 25 / 0.0 / 0.0 | shins 20 to 22 / 0.1 / 0.0 | shins 18 / 0.0 / 2.5 to 3.0 | 7 to 9 / 7 to 8 / 5.5 to 7 |
| `Poke_Hit` | thighs 29 / 0.0 / 0.7 | shins 22 / 0.1 / 0.0 | shins 19 / 0.0 / 4.5 | 6.3 / 1.8 / 6.9 |
| `Lift_Hit` | thighs 29 / 0.0 / 0.7 | shins 22 / 0.1 / 0.0 | shins 19 / 0.0 / 4.5 | 4.1 / 1.0 / 2.7 |

**Nothing goes below the ice by more than 1 cm on any figure** (lowest body point -0.0, lowest stick -0.01 cm).

**Lying rest** (the held end pose; lowest non-skate part / lowest skate, cm; the target is the body within 3 cm of the ice):

| Take | R | P3A | P4B |
|---|---|---|---|
| `Knockdown_Left` | **13.6** / 0.3 | 3.0 / 0.8 | **10.4** / 0.4 |
| `Knockdown_Right` | **5.2** / 0.3 | 0.8 / 1.1 | **9.5** / 0.4 |
| `Knockdown_Back` | 0.1 / **5.2** | **6.0** / 1.0 | 0.1 / **15.0** |
| `Trip` | 0.1 / **5.2** | **6.0** / 1.0 | 0.1 / **15.0** |

- **Side knockdowns on R and P4B:** the skate is the lowest point and the hip floats 5 to 14 cm. Their long skate and boot stick out below the hip in the source's stretched side pose. The sheets read as a figure curled on its side, not sliding on its hip.
- **`Knockdown_Back` and `Trip`:** the face-down lift is the same as `Fall_Forward`: the skates are 5.2 cm up on R and 15 cm on P4B. P3A rests on its skates and hands, with the big head and torso 6 cm up.
- **Knockdown end against get-up start, bone for bone:** 0.000 cm for all four pairs on every figure (Left -> `GetUp_Side_Left`, Right -> `GetUp_Side_Right`, Back and Trip -> `GetUp_Front`, and `Fall_Forward` -> `GetUp_Front`).
- **Self-intersection** (from the side sheets):
  - P3A `Board_Pin`: the mittens and stick sit at the big helmet (36 frames inside the head radius). It reads as the arms through the helmet.
  - P3A's side knockdowns: the mitten is pressed into the face.
  - R and P4B: no arm through the torso or the helmet that I could see at 160 px.
  - At the game camera's 40 px nothing reads as intersecting.

### 3. Sheets (filed)

| Shot file | What it shows |
|---|---|
| `devlog/shots/20261112-181820-B-lab-skh5-r-new-side.png` | R: the 12 new takes from the side, 6 frames each |
| `devlog/shots/20261112-181820-B-lab-skh5-r-new-game.png` | R: the same 12 takes from the game camera, 40 px shown 2x |
| `devlog/shots/20261112-181820-B-lab-skh5-r-knock40.png` | R: `Knockdown_Left`, `_Right`, `_Back` at 40 px in deep blue |
| `-p3a-new-side`, `-p3a-new-game`, `-p3a-knock40` (181821) | P3A, same three sheets |
| `-p4b-new-side`, `-p4b-new-game`, `-p4b-knock40` (181822) | P4B, same three sheets |

Row order on the new-take sheets: `Check_Hit_Left`, `Check_Hit_Back`, `Knockdown_Left`, `Knockdown_Right`, `Knockdown_Back`, `Trip`, `GetUp_Side_Left`, `GetUp_Side_Right`, `Board_Hit`, `Board_Pin`, `Check_Deliver_Left`, `Poke_Hit`.

At 40 px the three knockdowns read as different directions on all three figures.

### 4. Replace? And the takes not good enough to ship

| Candidate | `export5` replaces `export4`? | Not good enough |
|---|---|---|
| **R** | **Yes.** | `Knockdown_Left` (hip 13.6 cm above the ice, reads curled up) and `Knockdown_Right` (5.2 cm). `Knockdown_Back` and `Trip`: skates 5.2 cm up when lying, the same as `Fall_Forward`, which is already in the game. |
| **P3A** | **Yes for the old 55 takes and for** `Check_Hit_*`, `Knockdown_Left/_Right`, `GetUp_Side_Right`, `Check_Deliver_*`, `Poke_Hit`, `Lift_Hit`. | **`Board_Hit`** (196 cm stick pop at frame 11); **`GetUp_Side_Left`** (164 cm stick pop at frame 18); **`Board_Pin`** (mittens and stick through the big helmet, 36 frames); `Knockdown_Back` / `Trip` (torso 6 cm up when lying: marginal). |
| **P4B** | **Yes.** | `Knockdown_Left` (10.4 cm) and `Knockdown_Right` (9.5 cm): the hip floats over the skate. `Knockdown_Back` / `Trip`: skates 15 cm up when lying, as `Fall_Forward`. `GetUp_Side_*`: the skate-turn clamp leaves a visible ankle kink for 2 to 4 frames. |

### What I could not verify

- **Anything in Unreal.**
- **Whether the 25-degree skate clamp looks right at game speed:** it is judged from 6-frame sheets only.
- **Self-intersection:** judged by eye at 160 px, not computed.
- **The P3A `Board_Hit` and `GetUp_Side_Left` pops:** I found the cause (the tilt and slide branch switches) but did not fix it in this pass.

## Sixth pass: settle, stick continuity, blend-safe holds (18:50)

**Where each item stands:**

| Item | State |
|---|---|
| 1. Settle | **Done for the side lying poses.** Partial for face-down on the tall figures. |
| 2. Stick continuity | **Implemented, but it does not remove the pops.** |
| 3. Blend-safe holds | **Not started.** |
| 4. Stick out of P3A's helmet | **Done.** |

**`export6` is complete and consistent** for what is done: 70 takes per figure in `C:\Users\kazda\jam\skater-lab\skh\export6\{R,P3A,P4B}\`. The configs are `cfg/6_*.json` (`export_dir` "export6", `skh6`: true), checked before the first build. Rebuild with `bash skh/build6.sh <c>`.

`export/` to `export5/` were not touched. R's S5 regression was not re-run after these edits; every new rule is behind `skh6`, which R's regression config does not set.

**Bind check:** on all three figures, bone names, parents and rest pose (0.0), vertices (0.0), weights, polygons and UVs equal `skh/export/`. I ran it on the 18:40 build. The last rebuild changed animation code only.

### 1. Settle (lying and kneeling takes)

**What it does:**
- **Takes:** `Fall`, `Fall_Forward`, `GetUp`, `GetUp_Front`, `Knockdown_*`, `Trip`, `GetUp_Side_*` and the goalie blocks.
- **Per frame, measured on the skinned mesh:** a vertical pelvis move, plus a pitch or roll of the whole body about the pelvis (up to 15 degrees per pass, 3 passes). Together they bring the lowest torso/hip point and the lowest skate point to the ice.
- **Weight:** it follows how far the source pelvis has dropped (fully on when the drop is more than 0.60 of the rest height, off below 0.35), so standing frames are untouched.
- **Guards:**
  - a move is kept only if it brings the worse contact closer and never lifts a skate;
  - nothing in the body or stick goes below -1 cm.
- **Unchanged:** horizontal pelvis and root travel stay 0.000.
- **Not done:** the knee bend and ankle extension for the face-down poses.

Lying rest in the held end pose (lowest torso/hip / lowest skate, cm), `export5` -> `export6`:

| Take | R | P3A | P4B |
|---|---|---|---|
| `Knockdown_Left` | 13.6 / 0.3 -> **0.0 / 0.0** | 3.0 / 0.8 -> **0.2 / 0.1** | 10.4 / 0.4 -> **0.1 / -0.6** |
| `Knockdown_Right` | 5.2 / 0.3 -> **0.0 / 0.0** | 0.8 / 1.1 -> **0.2 / 0.2** | 9.5 / 0.4 -> **-0.9 / 0.6** |
| `Knockdown_Back`, `Trip`, `Fall_Forward` | 0.1 / 5.2 -> -0.5 / **4.9** | 6.0 / 1.0 -> 6.0 / 1.0 (unchanged: no move improved it) | 0.1 / 15.0 -> -0.5 / **10.4** |

- **Knockdown last frame against its get-up's first frame:** still 0.000 cm, bone for bone, for all five pairs on all three figures.
- **Side knockdowns:** now rest on the hip with the skate on the ice, on all three.
- **Face-down on the tall figures:** the skates are still 4.9 cm up (R) and 10.4 cm up (P4B). Pitch alone cannot bring them down without the chest and hands going through the ice; it needs the knee and ankle rule.
- **Cost of the guards elsewhere:** skate lift is otherwise at or under 1.4 cm. The exception is P4B `GetUp_Front` at 10.7 cm, from the same face-down start.

### 2. Stick continuity

**What it does:**
- The per-frame slide and ice tilt are rate-limited to 4 cm and 4 degrees per frame: forward and backward sweeps, three times, with loops warmed up over three laps.
- The tilt then takes the nearest ice-safe angle to the limited one.
- Static holds are excluded. On them the replayed tilt fought the reach rule, which lifted P3A's blades to 80 cm, so I took them out.

**Result: it does not remove the pops.** The ice-safe override jumps when the floor demands it.

Takes whose largest blade step is more than 1.5 times the source's largest blade step:

| Figure | Takes (ours against the source, cm) |
|---|---|
| R | `GetUp_Side_Left` 24 against 9 |
| P3A | `Board_Hit` **172** against 95 (was 196); `GetUp_Side_Left` **132** against 7 (was 164); `Goalie_Block_Left` 15 against 6 |
| P4B | `GetUp_Side_Left` 32 against 9, `GetUp_Side_Right` 39 against 19, `Goalie_Block_Left` 23 against 7, `Stride` 14 against 9 |

- **How it was measured:** I compared each take's largest step with the source take's largest step, not frame by frame.
- **Blade contact error:** it rose in the get-ups, to 25 to 33 cm (was 2 to 7 cm), because the limited tilt lags the contact.
- **Verdict:** this rule needs a different approach, solving slide and tilt jointly over time with the ice as a constraint. It is in `export6` as built.

### 3. Blend-safe holds: not started

The holds and the blend test are as in `export5`:

| Figure | Worst blend dip | Cases outside -1 to +4 cm |
|---|---|---|
| R | -4.82 cm | 21 |
| P3A | -3.63 cm | 18 |
| P4B | -2.38 cm | 2 |

Hold positions and blade heights are unchanged. No aim-grid sheet was made, because nothing changed there.

### 4. P3A stick through the helmet

**What it does:** in `Board_Pin`, `Knockdown_Left` and `Knockdown_Right`, when a stick vertex comes within the head radius plus 3 cm, the stick turns about the grip by the smallest angle that clears it, and the hands re-grip. I first ran it in every take; it lifted the blades of the holds, so it is now limited to these three.

**Result:**
- P3A stick-or-mitt-in-head frames: 39 -> **3**. `Board_Pin` 36 -> 0.
- The 3 left are in `Board_Hit`, which the rule does not cover.
- R and P4B: 0.
- The mitten pressed into the face in P3A's side knockdowns was not changed. It is a mitten, not the stick, and it is not inside the head radius.

### `export5` -> `export6`, other checks

| | R | P3A | P4B |
|---|---|---|---|
| Takes, pelvis and root travel | 70, 0, 0 | 70, 0, 0 | 70, 0, 0 |
| Lowest body / stick | -0.98 / -0.5 cm | -0.8 / -0.85 | -0.93 / 0.0 |
| Largest hand / blade step | 44.8 / 108.5 cm (= export5) | 50.0 / 172.3 (was 195.9) | 57.9 / 104.8 |
| Loop wraps over 3 cm | none | none | `Stride` 6.7 cm (was 4.6) |
| Rear-high upper back covered | 0 % | 0 % | 0 %, except `P70_R130` 13 % |

### Sheets (filed)

- `devlog/shots/20261112-184639-B-lab-skh6-r-lying.png`
- `devlog/shots/20261112-184640-B-lab-skh6-p3a-lying.png`
- `devlog/shots/20261112-184640-B-lab-skh6-p4b-lying.png`

Each shows `Knockdown_Left`, `Knockdown_Right`, `Knockdown_Back`, `Fall_Forward` and `Goalie_Block_Left` from the side.

These sheets were rendered from the 18:40 build. That build differs from the final one only in the static holds, which are not shown on them.

I did not look at these sheets before filing them: the time ran out.

### Replace? And what is still not good enough

| Candidate | `export6` replaces `export5`? | Still not good enough |
|---|---|---|
| **R** | **Yes for the lying takes.** The side knockdowns now rest on the ice. **Keep `export5` for `GetUp_Side_*`**: the continuity rule made their blade lag (blade error 25 cm, step 24 cm). | `Knockdown_Back`, `Trip`, `Fall_Forward`: skates 4.9 cm up. Blend dips: 21 cases. |
| **P3A** | **Yes for `Board_Pin` and the knockdowns.** | `Board_Hit` (172 cm stick pop), `GetUp_Side_Left` (132 cm pop), the blend dips (18 cases), the face-down torso 6 cm up. |
| **P4B** | **Yes for the knockdowns.** **Keep `export5` for `GetUp_Side_*` and `Stride`**: the continuity rule made `Stride`'s wrap and blade step worse. | `Knockdown_Back`, `Trip`, `Fall_Forward`, `GetUp_Front`: skates 10.4 to 10.7 cm up. |

**One concrete simplification:** the importer re-imports animations onto the same skeleton, so takes can be mixed per take. Take the knockdowns, `Board_Pin` and the falls from `export6`, and everything else from `export5`.

### What I could not verify

- **Looking at the sheets:** I did not review them before filing.
- **Face-down knee and ankle rule:** not implemented.
- **Item 3:** not started.
- **A frame-by-frame comparison of the blade against the source:** I compared per-take maxima only.
- **Anything in Unreal.**
