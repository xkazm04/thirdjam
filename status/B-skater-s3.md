# B skater lab S3: Rodin route (mesh in Hyper3D Rodin, rig and clips in Meshy)

Verdict in one line: **the route works end to end in under 3 minutes of API time for 1.5 Rodin and 16 Meshy credits.** Rodin gives a usable, cheap, chunky
skater; Meshy rigs it from the raw Rodin link and the existing stride plays on it. Keep it as the **sponsor-track route and a second figure source**, not as a
replacement for the Meshy-only figure yet.

## Steps
| # | Step | Result | Evidence |
|---|---|---|---|
| 1 | Rodin skater, text, Gen-2.5-Low, 20k, GLB, 2K | **PASS** (first try, no retry needed) | task 931f440d-150e-4a5b-a627-802575d3ed10, 19,282 tris, 15.5k verts, 2048 diffuse+normal+orm, GLB 1.47 MB, 37 s, 0.5 credit |
| 1b | Control: same prompt without the TAPose flag | PASS | task c07557c7-cf01-462a-97a3-9f0336c0ff31, 19,748 tris, 26 s, 0.5 credit |
| 2 | Rodin prop: studio spotlight, tripod, barn doors, ~10k | **PASS** | task ac18133c-0104-4fa1-8c98-1a117ef75730, 9,438 tris, 26 s, 0.5 credit |
| 3 | Meshy rig of the Rodin mesh, `ue5_mannequin`, 1.7 m | **PASS** | the Rodin download link was accepted directly by `--model-url`; rig 01a1cf35-60be-755e-ad6a-8c15e288a42f, 22 s task time, 5 credits; 28 bones, same names as the baseline, no root, pelvis on top |
| 3b | Same for the no-TAPose mesh | PASS | rig 01a1cf39-74e0-7496-9b7c-cd8a09ffe4d3 |
| 4 | Apply lane B5's stride (motion 01a1cf22-5993-7679-bf8f-c65be3dbae5e, read from the name in its animate URL) | **PASS** | animate 01a1cf36-1cf2-71ae-8211-8c483f9c9f23 (TAPose mesh) and 01a1cf39-bd65-75b3-800b-5fb69ca0f91d (no TAPose), 3 credits each; 120 frames @30, 280 fcurves, same as the baseline |
| 5 | Compare with the Meshy-only figure | done, below | |

No data URI was needed: Meshy fetched `https://file.hyper3d.com/...` (signed, 7 days) directly. Nothing was uploaded to any file host.

## Verified facts about the Rodin API (also in `C:\Users\kazda\jam\rodin-tools\README.md`, heading "Verified 2026-11-12")
- Cost: `consumed` = **0.5 credit per generation** (skater, prop, control alike; the TAPose field adds nothing). Balance 421 -> 419.5.
- Time: 26 to 37 s per generation. Statuses seen: `Generating`, `Done`; 3 jobs per task. First poll after 5 s worked.
- Tier `Gen-2.5-Low` accepted (the only one tried). `quality_override` acts as a triangle budget (20000 gave 19.3k and 19.7k, 10000 gave 9.4k).
- Output: GLB, one file in the list (`base_basic_pbr.glb`), PBR with diffuse + normal + ORM at 2048 each (so "one texture set" is three 2K maps; the Meshy figure has base colour only).
- Link: host `file.hyper3d.com`, signed, `X-Tos-Expires=604800` = 7 days, no credentials needed.
- Pose: `TAPose=true` is accepted but barely changes the result (bounds width 1.19 vs 1.31 m); the prompt alone gives an A-pose. No Rodin rigging endpoint found in the docs; none called.
- Client fixes: none needed for submit, status, download. One addition: `download` and `--out` output now include the raw `links` list so a URL can be handed to Meshy.

## Rubric rows (1 poor, 5 excellent)
| Candidate | Look fit | Technical | Cost | Provenance | Import risk |
|---|---|---|---|---|---|
| Rodin skater, no TAPose (preferred) | 4: flat saturated deep blue, white stripe, chunky toy, blue skates with pale blades, skin-tone mitts; reads at 40 px | 3: 19.7k tris, 3 x 2K PBR maps, 28 bones `ue5_mannequin`, no root, A-pose, limbs separate; hands are big mittens without fingers | 5: 0.5 Rodin + 5 rig = 5.5 credits, about 1 minute | ai-generated (Hyper3D Rodin Gen-2.5-Low, task id above); counts for the Hyper3D pool; scored as ours only if rule 7(h) allows generated assets (to confirm) | 3: same Meshy rig as the baseline (UE retarget needed), plus ORM and normal maps to hook up or drop |
| Rodin skater, TAPose | 3: same shape but grey gloves and skates, duller; arms held forward | 3: 19.3k tris; gloves interpenetrate torso and knees in the crouch | 5: same | same | 3 |
| Meshy-only baseline (B5 figure) | 4: more face, glove and sock detail, but darker navy, and **a jersey number "29" visible in the render of `meshy-work/RIG`** (check against the no-numbers rule) | 4: 20.8k tris, 1 x 2K base colour, 28 bones | 3: 58 credits for figure plus clips | ai-generated (Meshy) | 3 |
| Rodin spotlight prop | 4: clean tripod and barn doors, readable from every side; **fully black** with a pale lens, not coloured | 4: 9.4k tris, 3 x 2K maps (heavy for a prop, within the cap) | 5: 0.5 credit, 26 s | ai-generated (Rodin) | 4: static mesh |

Motion (same stride clip, so the same faults; props not applicable): the loop seam is **not closed**: mean bone-position difference first to last frame
(relative to pelvis) 30.4 cm no-TAPose, 32.0 cm TAPose, 29.0 cm baseline (max 83 / 95 / 80 cm). Pelvis travel 2.0 m along Y plus 0.3 m sideways (baseline 1.8 m and
0.26 m). Lowest-foot height range 11.6 / 13.9 / 8.7 cm. Strip pelvis X/Y travel as in MESHY.md.

## How the rig deforms (contact sheets)
No-TAPose Rodin skater: clean through the crouch and arm swing; the helmet stays rigid; the mitten hands stay in front of the chest without cutting the body.
TAPose skater: the grey gloves penetrate torso and knees (worse). Baseline: slimmer limbs, more detailed hands, deforms well. At 40 px the Rodin figure is the
easier to read (more contrast against the ice); the baseline is darker.

## Contact sheets (filed in `devlog/shots/`, `20261112-1107xx-B-lab-s3-*`; sources in `C:\Users\kazda\jam\skater-lab\s3\`)
- `lab-s3-rodin-turntable`: Rodin (TAPose) over baseline, 8 turntable views
- `lab-s3-rodin-notapose`: Rodin without TAPose, 4 views
- `lab-s3-stride-tapose`: stride on the Rodin TAPose figure over the baseline
- `lab-s3-stride-notapose`: stride on the preferred Rodin figure
- `lab-s3-spotlight`: the prop, 4 views
- `lab-s3-40px`: 40 px tall, Rodin over baseline

Meshes: `skater1\base_basic_pbr.glb` (TAPose), `skater_notapose\base_basic_pbr.glb`, `spot\base_basic_pbr.glb`; rigs `RIG\`, `RIG2\`; clips `A_STRIDE\`, `A_STRIDE2\`.
Ledgers: `rodin-ledger.jsonl`, `meshy-ledger.jsonl` in `s3\`.

## Credits
- Rodin: 1.5 spent (3 x 0.5) of 10 allowed. Balance 421 -> 419.5.
- Meshy: 16 spent (2 rigs x 5, 2 animates x 3) of 40 allowed. The shared balance went 1129 (start) to 860 (end), mostly other lanes; not attributable to this lane.

## Verdict
- **Rodin skater: keep as fallback and as the Hyper3D prize-track route; queue for import only after the Meshy-only figure.** It is far cheaper (0.5 vs 15 credits
  for a textured mesh), 30 s per try, brighter and flatter, so it fits the painted cel look at distance. Weak points: blobby mitten hands, little face detail, dead
  grey/black colours it picks unprompted, three PBR maps where we want one. The rig, clips and UE retarget are identical to the baseline, so a swap is nearly free.
  Good as a **variant factory** (a batch of 5 tries costs 2.5 credits).
- **Rodin prop: worth it for hard-surface props** (spotlight, tripod, camera, crates): 0.5 credit, readable silhouette, under 10k tris. Recolour in the engine (it came out black).
- If the prize track counts "Rodin in the pipeline", this route qualifies with one figure and the props.

## Licence wording seen
Only the Terms of Service text recorded in the README (for Rodin "we will not limit your use of such Output"; private or commercial export "depending on your
subscription plan"; no resale of Content as such; no training competing models). I saw **no account-specific licence text** in any API response for this key.

## Not verified
- Anything in Unreal (import, retarget, a toon material on a PBR GLB). Whether `quality_override` is faces or triangles beyond three observed numbers.
- Other tiers, Quad mode, FBX output, a Rodin rigging endpoint (none documented). A second team colour for Rodin (not tried; hue swap is the plan).
- The "29" on the baseline back: seen in a Workbench render, worth a second look by lane B5.
- Stride loop quality was measured, not fixed.

## Decisions for the lead or operator
1. Written confirmation from Hyper3D that output may ship in the contest build (plan-dependent export rights), and whether the prize track needs Rodin output in the shipped game or only in the pipeline.
2. Whether to spend a few more Rodin credits on team variants or a prop batch (cheap).
3. Accept ORM and normal maps (3 x 2K) per figure, or strip to base colour to match the one-set budget.
4. Check the "29" jersey number on the Meshy-only figure against the no-numbers rule.

## Set props (follow-up, 2026-11-12)

Common suffix of every prompt (verbatim): `stylised, painted cel style, flat solid colours, mid-grey and warm accent colours (not black: it must read in a dark room), no text, no letters, no numbers, no logos`.
All: tier `Gen-2.5-Low`, GLB, `quality_override` 10000, 0.5 credit and 26 to 40 s each, three 2048 maps (diffuse, normal, orm; the API gave no 1K option I tried). Run as 7 parallel tasks: no parallelism error. **Rodin rescales every object to about 1.7 to 1.9 m on its longest axis** (bounds below are as delivered), so each prop needs a scale factor in the engine.
Folder: `C:\Users\kazda\jam\skater-lab\s3\props\<name>\base_basic_pbr.glb` (sheets `sheet_<name>.png`, frames `fr_<name>\`, logs `<name>.log`).
Text-like marks: none visible on any kept prop (checked in 4 views at 224 px; the clap-stick stripes are stripes, the board is blank). The ledger is `s3\rodin-ledger.jsonl`.

| Prop (folder) | Task id | Prompt start (then the common suffix) | Tris | Size m (x,y,z as delivered) | Look | Note |
|---|---|---|---|---|---|---|
| 1 spotlight (`spotlight`) | 26bba5b6-541f-4c75-8084-54003c8a41eb | a film studio spotlight on a tripod stand with barn doors, in mid-grey metal with a warm yellow lens rim, | 9,722 | 1.25 x 0.82 x 1.86 | 5 | mid-grey body and tripod, glowing warm lens; reads well; no marks |
| 2 chair (`chair`) | b439b854-b5f8-4514-8463-eece35718ed0 | a folding director's chair with a canvas seat and back in warm orange, wooden frame, | 9,374 | 1.13 x 0.87 x 1.75 | 5 | exactly as asked, orange canvas, brown frame, clean cross legs; blank back |
| 3 clapperboard (`clapper2`, kept; `clapper` rejected) | e453e553-94aa-40d9-a379-1459a919b6a0 | a film clapperboard standing open on a small stand, the top clap stick opened at an angle, thick chunky toy proportions, mid-grey blank board with no writing, diagonal black and white stripes only on the clap stick, warm orange hinge, | 9,948 | 0.47 x 1.20 x 1.79 | 4 | blank grey board, striped open stick, orange hinge and feet; the lower stick stripes are thin. First try f5ced869-67fb-43b1-8e98-56ac9af8b68a (9,502 tris) was rejected: slab with torn, noisy geometry and a flat sheet board, not an open slate |
| 4 camera on dolly (`camdolly`) | 1462aa6c-7e46-4089-9dbc-9f6c16a1a219 | a cinema camera on a wheeled dolly with a seat, chunky toy proportions, | 9,556 | 0.69 x 1.69 x 1.73 | 5 | grey body, orange seat and lens ring, yellow hubcaps, four wheels; very readable; small orange and white bits on the camera are shapes, not letters |
| 5 camera crane (`crane`) | e9a845c9-c386-4d72-b8f1-96249e6685a9 | a camera crane (jib arm) on a wheeled base with a small camera head at the end of the arm, | 9,938 | 0.89 x 1.44 x 1.78 | 4 | near-white arm and base with orange accents, camera head at the tip; very pale, low contrast on a light ground but fine in a dark room; thin arm |
| 6 flight cases (`flightcase`) | be52e7de-3e16-4984-ae1b-24929550b31e | a stack of two road flight cases with metal corners and latches, | 9,488 | 1.79 x 1.82 x 1.71 | 4 | two stacked cases, light-grey corners, latches, orange upper panel, grey lower; bounding box is far wider than the object reads, check scale after import |
| 7 light truss (`truss2`, kept; `truss` rejected) | ef087401-b9a3-4293-a769-3bd11edab124 | a tall studio light truss segment in light mid-grey painted metal (not black) with two hanging lamp heads with warm orange shades, chunky toy proportions, | 9,304 | 1.13 x 0.88 x 1.86 | 3 | grey lattice, orange shades that read; **three lamp heads instead of two**, and the truss is thin and pale. First try 96e87959-84ef-4d7a-afb8-9cf5d01df334 (9,642 tris) was rejected: all black, the colour instruction ignored |

Credits: 9 generations x 0.5 = **4.5 Rodin credits** for this follow-up (limit 6); two of them retries (clapperboard, truss). Whole lane: 6.0 Rodin credits, balance 421 -> 415.
Lessons: Rodin follows "mid-grey, warm accent" when the colour is attached to the object ("in light mid-grey painted metal (not black)"), and ignores it when it only sits in a generic suffix (truss, first try). Counts ("two lamps") are unreliable. Contact sheets filed as `lab-s3-prop-spotlight`, `-director-chair`, `-clapperboard`, `-camera-dolly`, `-camera-crane`, `-flight-cases`, `-light-truss` (devlog/shots, `20261112-111228-B-lab-s3-prop-*`). The spotlight row replaces the all-black one from the first run (look 4 there).
