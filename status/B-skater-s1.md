# Status B, lab lane S1 (figures), 2026-11-12

Lab folder `C:\Users\kazda\jam\skater-lab\s1\` (ledger `meshy-ledger.jsonl`; its `download` lines repeat the credits of the task they fetch, so do not sum the file). Total spent: **65 credits** of 150. Wall time about 10 minutes.

## Rubric rows (1 poor, 5 excellent)

| | A big-head mascot | B lean athlete | C wooden toy | Baseline fig. 1 |
|---|---|---|---|---|
| **Look fit** | **4.** Helmet plus head about 1/3 of height, mitten gloves, white stripe at the hem. Strong silhouette that survives the 40 px render (head and white gloves read). A baked soft face with eyes. | **2.** Reads as a real hockey player (cage, socks, skates), but at 40 px the head is 3 px and the thin limbs vanish; only a dark blob with a stripe. Two white stripes (chest and hem) rather than one. | **4.** Blocky torso, ball shoulders and elbows, knee and hip blocks. At 40 px it is the cleanest toy shape; the wood is only implied (skin-toned head, pale hands), the joints come from the mesh, not the texture. Slight banded shading on the torso in Workbench. | **4.** Chunky kid-proportioned toy, the same family as A but with a smaller head. **A "29" is printed on the sleeve (side view), which breaks the no-numbers rule.** Small marks on the socks too. |
| **Technical** | 20,796 tris, one 2048 base colour (JPG in FBX), 28 bones `pelvis`..`RightToe_end`, no root bone, A-pose, limbs separate. Bounds 1.37 x 0.56 x 1.7 m (wide hands). | 20,722 tris, one 2K, same 28 bones, no root, A-pose, limbs separate (arms hang close to the body). | 20,694 tris, one 2K, same 28 bones, no root, A-pose, limbs separate. Rigid parts skinned fine in the turntable (no tearing seen; no animation tested). | 20,733 tris, one 2K, same 28 bones, no root, A-pose. |
| **Motion** | Not tested (lane S2 owns clips). Only the rig's rest action exists (pelvis travel 0). Short stubby limbs with huge head will make stride readable only through the body lean. | Not tested. Longest limbs, so leg push is the most visible of the four. | Not tested. Rigid joints should suit a stiff stride but a skinned blocky mesh may crease at the elbows; unknown. | Known from MESHY.md (stride plausible, 0.38 m sideways drift). |
| **Cost** | 20 cr: 2 previews (one retry for head size), refine, rig. 5+5+10+5. About 3 min wall. | 20 cr: preview 5, refine 10, rig 5. | 20 cr: same. | Already paid by lane B5 (not on this ledger). |
| **Provenance** | `ai-generated`, Meshy meshy-6-lite. Preview 01a1cf31-3eb4-7248-af39-1504a29b84af, refine 01a1cf32-6706-7384-b4a0-f860a20732ce, rig 01a1cf33-3b37-7559-9f87-cea18ff7a346. Meshy licence; counts as a Meshy/partner experiment, not scored as our own art. | `ai-generated`. Preview 01a1cf30-0ea4-76da-95eb-b3e1904af014, refine 01a1cf31-3e93-7397-8a62-d4ac3a7f665c, rig 01a1cf32-6719-70bb-9e69-c26981e29390. | `ai-generated`. Preview 01a1cf30-0ea1-7400-b001-437670f1352b, refine 01a1cf31-3e83-731d-a761-4a35c19c8f77, rig 01a1cf32-676a-7466-95ff-6ea9ee1892d4. | `ai-generated`, from lane B5 (refine 01a1cf21-abca-71e0-8b89-ded695bb2f20, rig 01a1cf22-143c-7425-8d02-8ce566754261). |
| **Import risk** | Same skeleton and rig as baseline, so B5's import answers carry over. Extra: huge head means helmet/head overlap with any head-attached item; the wide mitten gloves may clip a stick. | Same skeleton. Thin arms close to the body: skinning weights near the torso may smear. | Same skeleton. Jointed rigid shapes may separate or intersect when bent past the A-pose; test early. | Known; but the printed number must be removed (re-texture or paint) before it is shown. |

Credit lines: 4 previews x5 = 20; 3 refines x10 = 30; 3 rigs x5 = 15. Total 65. Balance before 1144 (monthly 944 + permanent 200); after 960. The 184 difference includes the other three lanes' spending on the same account, so it is not ours.

## Prompts used (verbatim)

Preview, all with `--ai-model meshy-6-lite --pose a-pose --polycount 20000`:

- A (first try, rejected: head only about a quarter of the height, child toy): `a big-head mascot ice hockey skater: very large round helmeted head about one third of the total height, short chunky arms and legs, big round mitten gloves, hockey skates, deep blue jersey with one white stripe, A-pose, arms and legs clearly separated from the body, painted cel style, flat solid colours, no text, no logos` (task 01a1cf30-0e3b-77e9-a15c-bd20aad5ecc5, 5 cr).
- A (retry, kept): `a big-head mascot ice hockey skater, caricature proportions: gigantic oversized round helmeted head as big as the whole body, one third of the total height, tiny short stubby arms and legs, huge round mitten gloves, hockey skates, deep blue jersey with one white stripe, A-pose, arms and legs clearly separated from the body, painted cel style, flat solid colours, no text, no logos`
- B: `a lean stylised ice hockey athlete, heroic six-heads-tall proportions, full hockey gear: helmet with cage visor, shoulder pads under the jersey, shin guards, big hockey gloves, ice skates, simplified shapes, deep blue jersey with one white stripe, A-pose, arms and legs clearly separated from the body, painted cel style, flat solid colours, no text, no logos`
- C: `a wooden articulated toy ice hockey figure, visibly jointed with ball joints at shoulders, elbows, hips and knees, blocky torso, painted wooden hockey toy, helmet, skates, deep blue jersey with one white stripe, A-pose, arms and legs clearly separated from the body, painted cel style, flat solid colours, no text, no logos`

Refine (`--texture-res 2k`), with TP = `deep blue jersey with one white horizontal stripe, deep blue helmet, white gloves, black skates with silver blades, warm skin, flat solid colours, painted cel style, no text, no logos, no numbers`:
- A: `big-head mascot hockey skater: TP`
- B: `lean stylised hockey athlete: TP`
- C: `painted wooden toy hockey figure: TP, bare light wood arms and legs and joints`

Rig: `--template ue5_mannequin --height 1.7`. Thumbnails of the three first previews were read: B and C fine, A was a retry (not fused, but the head was too small for the brief; one retry only).

## Findings

- All four rigs are identical in skeleton (28 bones, `pelvis` root-less, same names), so any of them can take the same clips and the same retarget as figure 1.
- The tool ignored "one white stripe" in B (hem and chest stripes) and gave A a tiny collar button (not a logo). None of A, B, C shows letters, numbers or logos in the eight views.
- **Baseline figure 1 shows the number "29" on the sleeve** (visible in `detail`). That contradicts the "no numbers" rule and was not caught in B5's review.
- Texture is baked with soft shading on all four (the `remove_lighting` flag is not honoured by 6-lite, as in MESHY.md); C shows horizontal banding in the Workbench render.

## Contact sheets (filed with devlog/shot.mjs)

- `devlog/shots/20261112-110132-B-lab-s1-turntable.png` (4 rows x 8 views: A, B, C, figure 1)
- `devlog/shots/20261112-110132-B-lab-s1-small40.png` (about 40 px tall figures, front and side each, 4x nearest; order A, B, C, figure 1)
- `devlog/shots/20261112-110133-B-lab-s1-detail.png` (front/side 2x: A, figure 1 side, B, C)

Local files: `C:\Users\kazda\jam\skater-lab\s1\sheets\`, rigged FBXs in `s1\gA`, `gB`, `gC` (`rigged_character_fbx_url.fbx`), refine FBX and texture in `rA`, `rB`, `rC`, inspect output `insp_*.txt`.

## Verdicts and ranking

1. **C wooden toy: queue for import** (as the second figure). Cleanest, most toy-like silhouette at 40 px, close to T-A's look, no face detail to worry about. Risk: skinning on rigid joint blocks.
2. **A big-head mascot: queue for import, with C or instead of figure 1.** Best 40 px readability thanks to the head and white gloves; the child-like face is the same trade-off as figure 1. Better than the baseline because the large helmet is the strongest team-colour area at distance.
3. **Baseline figure 1: keep as fallback** (it is already importing) but retexture or paint out the "29" before it is shown.
4. **B lean athlete: keep as fallback / drop for gameplay.** Most realistic and the best candidate for a close-up or a menu, but at 40 px it is the weakest silhouette, which is the game's main viewing case. Not a gameplay figure at the camera distance used.

(Ranking is by the 40 px render and the turntable only; nothing was animated.)

## Could not verify

- Any motion: stride, shot, fall, skinning quality at bent joints, pelvis drift and foot slide for the three new figures (S2's clips were not applied to them).
- The look under the game's toon ramp plus outline (Workbench is only a stand-in), and the orange team variant.
- The Unreal import (28-bone root-less rig with no root; known risk from B5).
- Whether the "29" on figure 1 is on the final refine texture in the editor (it is in `REFINE/texture0.base_color.png` as rendered).
- Absolute 40 px sizes: I downsampled 192 px Workbench renders (area filter), not the engine at 1080p.

## Lead or operator must decide

- Which of A and C goes to lane B5 for the import trial (the lead queues one or two), and whether a figure other than figure 1 becomes the T-B2 default.
- Whether to spend the remaining 85 credits (S1 budget): a re-refine of figure 1 without numbers (10 cr), an orange team refine of A or C (10 cr each), and S2 stride/shot clips applied to the new rigs (3 cr per animate, 10 per motion already generated for figure 1 could be reused: they are bound to the same template).
- Figure 1 with a printed number: B5 and the dev log should know before it is shown.

## Orange team (follow-up from the lead; 25 credits)

Calls (same preview, same texture prompt with "warm orange jersey with one white horizontal stripe, warm orange helmet" in place of the blue, `--texture-res 2k`):
- A orange refine: task 01a1cf35-b8f7-7095-8f3d-04262a9c1d1a, 10 cr (preview 01a1cf31-3eb4-7248-af39-1504a29b84af).
- C orange refine: task 01a1cf35-b92f-7137-b5c8-b62618cae4d7, 10 cr (preview 01a1cf30-0ea1-7400-b001-437670f1352b).
- A orange rig (see answer 1): task 01a1cf36-efe9-7233-8092-72ccedd27992, 5 cr. Same 28 bones, 20,796 tris, 1.7 m.
- Total follow-up 25 cr; S1 total now 90 of 150. Files in `s1\oA`, `oC`, `gOA`.

**1. Same mesh and UVs? Same geometry, different UV layout and vertex order: the orange texture CANNOT simply be swapped onto the blue rigged mesh.**
How I compared: imported blue refine `model.fbx` and orange refine `model.fbx` in Blender 4.2 (blue A in `rA`, blue C in `rC`) and read the largest mesh. Vertex count: A 10,376 vs 10,376, C 10,341 vs 10,341; polygons A 20,796 vs 20,796, C 20,694 vs 20,694. The set of vertex positions rounded to 1 mm is identical in both pairs (A 10,376 of 10,376 shared, C 10,341 of 10,341), and the bounding boxes match. But the vertex and loop order differ (loop vertex indices are not equal), and when I matched every loop by position plus UV, 0 of 62,388 (A) and 0 of 62,082 (C) loops matched; the sorted UV lists also differ. So the retexture re-unwrapped the UVs: the surface shape is the same, the texture atlas is not.
Consequence: orange A needs its own rig, done (5 cr). Orange C would need its own rig too if used (5 cr, not done: the lead said only A).
The orange rigged FBX is in `s1\gOA\rigged_character_fbx_url.fbx`; its skeleton equals the blue one, so clips made for the blue A should play on it (not tested).

**2. Letters, numbers, logos in eight views:** none seen in either orange figure (full-size 192 px views and 2x crops). Small remarks: on C orange the backs of the gloves and the boot tops carry tiny dark marks (probably laces/straps, not letters); both orange figures have a tiny collar/chest mark. I cannot rule out detail below the 192 px render.

**Look problems you should know about (not asked, but important):**
- **Orange A has lost its face:** the head area is a flat pale-grey blank under the helmet (the face skin colour was not painted; it reads as a visor). Fine at 40 px, odd close up. Blue A has a normal face. A second orange refine (10 cr) with "warm skin tone face with eyes" might fix it; not spent.
- Orange C has no face either (blank skin head, as blue C has a small face), a sleeveless-looking orange torso, bare skin arms and legs: it reads as a plain wooden toy. The white stripe is there (once, chest).
- A blue vs orange at 40 px: both read clearly as two teams (orange clearly different hue from blue).

Contact sheet: `devlog/shots/20261112-110455-B-lab-s1-teams.png` (rows: A blue, A orange, C blue, C orange, 8 views each, then a small-size strip).
