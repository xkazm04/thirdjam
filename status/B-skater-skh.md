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
