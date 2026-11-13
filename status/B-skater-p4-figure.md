# Skater lab SKC, third pass: chasing the P4 concept player overnight (report at 00:40)

Lane ran from 23:41 to 00:40 by the machine clock. I worked in Blender 4.2 and called Codex image generation, Hyper3D Rodin, Meshy and Tripo. Nothing has been in Unreal.

Files are in `C:\Users\kazda\jam\skater-lab\skc\p4\`. The running notes are in `p4/NOTES.md`, the sheets in `p4/sheets/`, the concept-framing renders in `p4/hero/`, and the ledgers in `p4/*-ledger.jsonl` and `p4/balances.jsonl`.

`skc/export/` and `skc/export2/` were not touched.

## Short answer

**The winner is P4E.** It is my own concept sheet, meshed by Rodin Gen-2.5-High, rigged by Meshy, with a clean kit texture painted in Blender. It gives a natural adult player of about 7.4 heads with a clear half visor, a white shoulder yoke, two sleeve stripes, a hem stripe, two sock bands, dark cuffed gloves and slim skates on white blade holders. The kit is crisp and symmetric because the bands are repainted as measured shapes, not left as the generator's smears. The back is clean.

**Rig and takes are ready for the engine.**
- 72 takes, all with the second pass's rules: start match, stick prop, settle.
- The other retarget lane's face-down legs rule is ported: face-down skates rest at 0.5 cm, against 11.5 cm on P4D.
- A jersey-skirt re-weight keeps the hem level in the stride.
- The rest heights are measured in `p4/P4E/out/P4E/summary.md`; summary below.

**Against the concept:** about two thirds of the way there.
- **Closed:** proportions, helmet and visor, the kit layout and the team-colour key.
- **Still open:**
  - cloth that hangs and folds like the concept's loose jersey (the generator makes a fairly tight, smooth shell);
  - the scripted P4 stride, a wide, deep squat where the concept shows a forward lunge;
  - a slightly photo-real face that can read uncanny close up;
  - glossy-plastic shading in my renders. That is my lighting, not the asset.

## Candidates

All candidates use the same Codex style line (front and side views, white background, no text). Prompts are verbatim in `p4/<cand>/concept/*.prompt.txt`.

| Candidate | Source | Settings | Cost | Tris | Textures | Verdict |
|---|---|---|---|---|---|---|
| E Rodin base | P4E sheet: ~7 heads, visor, yoke, stripes, socks; A-pose | Rodin Gen-2.5-Low, Quad 18k, TAPose | 0.5 Rodin | 34,061 | 3 x 2K | Natural proportions and a clean kit. Superseded by High. |
| F Rodin base | P4F sheet: the same, arms at 45 degrees, open hands | same | 0.5 Rodin | 32,804 | 3 x 2K | **Rejected:** a white slab through the visor, and splayed gloves. |
| F Rodin High | P4F sheet | Gen-2.5-High, texture high, `uhd` (4K) | 0.5 Rodin | 33,758 | diffuse 4K; normal and ORM 2K | Visor fixed, a cleaner face. Second best. |
| **E Rodin High (WINNER)** | P4E sheet | Gen-2.5-High, texture high, uhd 4K, Quad 18k, TAPose | 0.5 Rodin | **33,512** | diffuse 4K; normal and ORM 2K | The best face and visor, a clean kit, natural proportions. |
| E Rodin Extreme-High | P4E sheet | Gen-2.5-Extreme-High, Raw 50k, texture high, uhd | 1.0 Rodin | 50,000 | 4K / 2K | **Rejected:** a skin-coloured smear across the visor and cheek; no gain at game size. |
| E Rodin Gen-2 | P4E sheet | tier `Gen-2`, texture high, uhd | 0.5 Rodin | 32,548 | 4K / 2K | **Rejected:** the same class as High, with the visor barely there. |
| F Meshy | P4F front crop | image-to-3d `latest`, quad remesh 30k, PBR 2k | 30 Meshy | 55,336 | 3 x 2K | **Rejected:** the sharpest face, but **bare hands** (the gloves are lost) and spiky blades. |
| F Tripo | P4F front + left | `multiview_to_model` v2.5, PBR, texture detailed, face_limit 30000 | 40 Tripo | 29,938 | Color, NormalGL, ORM at 4K | The cleanest cloth surface, but the face is soft and no visor reads. Third. |
| E Tripo | P4E front + left | same | 40 Tripo | 29,952 | 4K | **Rejected:** a white blob on the back of the helmet. |
| G Rodin High | P4G sheet: loose jersey, shoulder pads, breezers to the knee, cuffed gloves | Gen-2.5-High, uhd | 0.5 Rodin | 34,184 | 4K / 2K | **Rejected:** the visor became round goggle lenses; the jersey is no looser in the mesh than E. |
| G Tripo | P4G sheet | `multiview_to_model` | 40 Tripo | 29,977 | 4K | **Rejected:** **letter-like marks across the upper back**, and a blob on the helmet. |

**API findings (measured):**
- **Rodin:** this key may use Gen-2.5-High and Gen-2 at 0.5 credit, the same as Low. `texture_mode high` and `uhd_texture` (4K diffuse) cost nothing extra. Gen-2.5-Extreme-High is charged 1.0.
- **Meshy:** the first image-to-3d call was refused at no cost, because `texture_resolution` must be `2k`, `4k` or `8k`, not 2048.
- **Tripo:** the key works. I wrote a minimal client, `p4/tools/tripo.mjs` (balance, upload, task, status, download; the key is never printed). Each multiview task cost 40 Tripo credits.

**Comparison sheets** (`devlog/shots/`):

| Sheet | What it shows |
|---|---|
| `20261113-000611-B-lab-skc3-cmp-generators` | One sheet through Rodin Low, Rodin High, Meshy and Tripo |
| `-cmp-rodin-e` | P4E through Rodin Low, High and Extreme-High, and Tripo |
| `20261113-003836-B-lab-skc3-cmp-more` | P4G Rodin, P4G Tripo, Gen-2 |
| `-concept-e`, `-concept-f`, `-concept-g` | The three concept sheets |

## The engine lab's list, judged on P4E

1. **Toy proportions: cleared.** Head radius 0.121 m on 1.80 m, about 7.4 heads. P4D is 0.171 m.
2. **Cage instead of a visor: cleared.** A clear half visor over a real face.
3. **Mitten gloves with no cuff: cleared.** Dark navy hockey gloves with a cuff and moulded fingers.
4. **Short shorts: partly cleared.** Padded blue pants to just above the knee, but slimmer than real breezers. P4G asked for breezers and Rodin did not deliver them.
5. **Skin-tight jersey: not cleared.** The jersey is a smooth shell with mild folds and does not hang loose. The hem now stays level and covers the pants in the stride (the skirt re-weight), but there is no hanging cloth.
6. **Marks on the back: cleared.** Clean team blue on the back. My texture pass found nothing to remove there, and the helmet's light vent marks were painted to helmet blue.
7. **Thick neck, sloped shoulders: partly cleared.** A normal neck and shoulders of normal width, but no visible shoulder-pad bulk.
8. **Chunky flat skates: cleared.** Slim black boots on white holders with steel runners.
9. The stick is the engine's.
10. **No deep push or snow-throwing stop among the takes:** noted, nothing added. The scripted P4 Stride is a very wide, deep squat, the main reason the concept-framing renders look less athletic than the concept.

## Texture pass (Blender only; `p4/tools/tex_p4.py`)

**Inputs.** The mesh and UVs are extracted and rasterised at 2048 with the texture lane's `skt/extract.py` and `skt/raster.py` (67 % coverage, about 1 mm per texel).

**Part map.** Regions come from bone weights: torso, pelvis, upper and lower arms, hands, thighs, calves, feet and head. Colour classes come from the generator's base colour: blue, white, skin, near-black.

**Bands, measured from the generator's own white and then repainted as clean shapes:**

| Band | Placement |
|---|---|
| Yoke | above 146.5 cm |
| Hem stripe | 96 to 99.5 cm |
| Sleeve stripes | two, at 17.4 to 20.6 cm and 23.6 to 26.9 cm from the shoulder along the arm, equal widths, mirrored left and right |
| Sock bands | 37 to 41 cm and 43.5 to 47.5 cm |

**Colours.**
- **Cloth:** every jersey, sleeve, pants and sock texel is repainted with a **saturated royal blue, sRGB (0.05, 0.17, 0.74)** or trim white, keeping 25 % of the generator's shading.
- **Hue key:** linear b - max(r, g) is at least 0.425 on 95 % of team cloth (median 0.483) and at least 0.29 on 100 % of it. No white texel keys.
- The generator's own blue was sRGB (0.063, 0.192, 0.533), which keys at only about 0.22, below the engine's 0.29.
- Gloves and skates keep their dark neutral colours.

**Other maps.**
- **Normal (DirectX):** a 3.2 mm cloth weave on all cloth, and leather noise on the gloves.
- **Mask RGBA:**

  | Channel | Holds |
  |---|---|
  | R | team cloth and the blue helmet shell |
  | G | white trim, never recoloured |
  | B | gloss: helmet 0.85, skates 0.6, gloves 0.3, cloth 0.12 |
  | A | cloth amount |
- **AO:** baked in Cycles at 2048 (32 samples) and multiplied in softly (0.8 + 0.2 x AO).

**Output.** 2048 and 1024 versions, with the UV gutters filled with the nearest island texel.

**Sheets:** `20261113-000611-B-lab-skc3-tex-before-after` and `20261113-003837-B-lab-skc3-closeups`.

## Winner P4E: rig, takes and checks

| Item | Value |
|---|---|
| Rodin mesh | `a297c99b-3976-4baa-a8ee-17a77c59d0a2` |
| Meshy rig | `01a1d1f6-3b17-75d1-9dcd-afb5d0df719f`, `ue5_mannequin`, 1.80 m |
| Config | `skc/cfg/3_P4E.json`, a copy beside the export |
| Clips | `skf/export5/p4_broadcast` |

The config carries the second pass's keys plus four new ones:
- `facedown_legs`: the seventh pass's hip, knee and ankle rule from `skh`, ported as a switch.
- `skirt_z` [0.965, 1.15, 0.86]: the thigh weight on the jersey skirt moves to the pelvis above the hem, blended from 86 cm. Without it the hem split into a V and the seat bulged in the stride (sheet `hero-stride`).
- **Stick continuity also on `Lift` and `Board_Hit`:** Lift's blade step drops from 40 to 24 cm (the source's own largest step is 22 cm).
- `hand_pop_filter`: an isolated left-arm IK switch is eased over at most 8 frames. Stop's left hand step drops from 24 cm to 8.4 cm.

**Measured** (re-imported main file):

| Check | Value |
|---|---|
| Takes, pelvis travel, root, loop seams | 72; 0.000; 0.000; 0.000 cm |
| Loopcut wraps over 3 cm | `Stride` 4.6 cm |
| Lowest body / stick | -0.97 / -0.50 cm |
| Standing skate error | 2.0 cm |
| Stick or mitten inside the head | 0 frames |
| Joins | Knockdowns to get-ups, `Trip -> GetUp_Side_Right` and `Poke@6 -> Stumble_Short`: all 0.000 cm |
| Prop takes, largest blade step (source) | `GetUp_Side_Left` 8.2 (9.0), `GetUp_Side_Right` 18.4 (18.6), `Goalie_Block_Left` 6.2 (7.2) cm |
| Takes with a blade step over 1.5 times the source's | none (`Board_Hit` 160 against 108 cm, under 1.5 times) |
| Takes with a hand step over 1.5 times the source's | `GetUp` 21 vs 6, `Goalie_Block_Left` 22 vs 7, `Lift` 30 vs 14 cm |
| Aim grid | worst 4.6 cm / 2.4 deg; blade 0.7 to 2.4 cm up; blend test worst 6.0 cm, 2 cases outside -1 to +4 cm |
| Rear-high back cover | 0 % in Glide and Stride; 13 % on `Aim_P00_R130`; 31 % on `Aim_P70_R130` |
| Face-down skates | **0.5 cm** (torso 12.7 cm, knees 17.9 cm up) |
| Lying rest | `Knockdown_Left` 0.3 cm, `Knockdown_Back` -0.5 cm, `Knockdown_Right` / `Trip` hip **8.6 cm** (the skirt re-weight changed the side-lying settle) |

**Shippable:** 69 of 72 takes ship without remarks.

**Ship, but look at them in the engine:**
- `GetUp`, `Goalie_Block_Left` and `Lift` (a hand step of 2 to 3 times the source's);
- `Knockdown_Right` and `Trip` (the hip rests 8.6 cm up);
- `GetUp_Side_Left` (it props on the butt end of the stick; blade 22 cm off the ice where the source's touches it).

None is a hard pop.

**What an importer needs:**
- **Files**, in `C:\Users\kazda\jam\skater-lab\skc\p4\P4E\export\P4E\`:
  - `SK_Skater_P4E.fbx` and `_loopcut.fbx`: 72 takes at 60 fps, a new skeleton asset (32 bones, `root` on top, centimetres, 1.80 m, bind pose from Rodin's TAPose, the stick on the animated `stick_socket` bone);
  - `T_Skater_P4E_BaseColor.png` (2048; `_1024` too; the original 4K generator texture is `_4k`);
  - `T_Skater_P4E_Normal.png` (DirectX, Non-Color), `T_Skater_P4E_Mask.png` (RGBA, linear) and `T_Skater_P4E_AO.png`;
  - `P4E.json`, the concept sheet, its prompt and the two crops.
- **The base colour is referenced by path**, so copy it with the FBX.
- **Physics:** A's capsules. Head radius 0.121 m, mitten radius 0.106 m.

## Concept-framing renders (the picture to judge)

EEVEE, low behind the player (about 2.7 m back, 1.2 m up), a glossy grey-blue floor and five overhead area lights. Each row shows P4E with the clean maps, P4E with the generator texture, P4D and P4B, in blue and then orange:

| Sheet | Pose |
|---|---|
| `devlog/shots/20261113-003837-B-lab-skc3-hero-stride` | Stride frame 50 |
| `-hero-glide` | the Glide carry |
| `-strips` | game camera at 40 and 70 px |
| `-winner` | turntable, face, back |

**What they show:**
- P4E is the only figure that reads as a player rather than a toy: helmet and visor in proportion, a crisp yoke, stripes and socks, slim skates.
- At the game camera it is the brightest and cleanest of the three, in both colours.
- The pose is the weakest part: the P4 Stride's wide, deep squat with the arms out.

## How close to the concept player, honestly

**Look, at 2 to 4 m: about 65 %.**
- The silhouette, proportions, helmet, visor, kit layout and skates match.

**What remains:**
1. **Cloth.** The concept's jersey is loose, hangs over the pants and folds at the waist and sleeves. Ours is a tight shell with baked shading. No generator produced real cloth volume: P4G asked for it, and the mesh came back no looser. It needs either a hand-modelled or simulated jersey shell skinned over the body, or a generator with better clothing.
2. **Pose and deformation.** The scripted P4 stride is a symmetric deep squat. The concept's player lunges with one leg extended and the chest forward. Even with the skirt re-weight, Meshy's auto-weights crease the seat and the shoulders in extreme poses, and there are no twist bones, so forearms candy-wrap in some turns.
3. **Face.** A realistic, slightly stiff face, readable behind the visor close up, and mildly uncanny at 1 m. It is irrelevant at game distance.
4. **Texture resolution.** It is enough: 2048 gives about 1 mm texels, and the generator's 4K source is kept. Fine detail is not the gap.
5. **Shading.** The concept's cloth is matte with soft sheen. My renders read glossy; the engine's material owns that (the mask gives the gloss).
6. **Time.** The mesh, rig, retarget and texture loop is about 25 minutes per candidate. Closing gaps 1 and 2 is days of manual work, not another overnight run.

## Credits and balances (third pass)

| Service | Spent | Budget | Balance before -> after |
|---|---|---|---|
| Codex images | 3 (p4e, p4f, p4g) | 15 | n/a |
| Rodin | **4.0**: 6 x 0.5 + 1 x 1.0 | 30 | 401 -> 397 |
| Meshy (shared account) | **35**: image-to-3d 30, one rig 5 | 150 | 545 -> 510 |
| Tripo | **120**: 3 x 40 | no cap given | 810 -> 690 |

**Lane total** (all three passes): 8 Codex images, Rodin 6.5, Meshy 60, Tripo 120.

## Provenance

| File | Origin | Tool |
|---|---|---|
| `p4/P4{E,F,G}/concept/*` sheets and crops | ai-generated (crops: our script) | Codex image generation |
| P4E mesh, UVs and the 4K generator base colour | ai-generated | Hyper3D Rodin `a297c99b-…` |
| P4E skeleton and weights | ai-generated, modified | Meshy rig `01a1d1f6-…`; root, sockets, boot and jersey-skirt re-weights by `skc/retarget.py` |
| `T_Skater_P4E_BaseColor` (2048 / 1024) | ai-generated, modified | Rodin texture repainted by `p4/tools/tex_p4.py` |
| `T_Skater_P4E_Normal`, `_Mask`, `_AO` | original | `tex_p4.py`; AO baked in Blender Cycles |
| The 72 takes | original | `skf/export5`, retargeted by `skc/retarget.py` |
| The stick | original | S4 |
| Rejected candidate meshes in `p4/P4*/` | ai-generated | Rodin, Meshy, Tripo; not exported |

## Rebuild (no API calls)

```
bash skc/build2.sh 3_P4E
python skc/p4/tools/tex_p4.py skc/p4/P4E/tex skc/p4/P4E/export/P4E/T_Skater_P4E_BaseColor_4k.png skc/p4/P4E/tex/ao.png skc/p4/P4E/tex/out P4E
bash skc/p4/tools/hero_all.sh Stride 50
bash skc/p4/tools/strips.sh <fbx> <name> <orange png>
bash skc/p4/tools/evalcand.sh <model> <name> "<label>" [front yaw]
```

`p4/tools/bake_ao.py` bakes the AO. Run the texture step before the build: the build references the PNG by path.

## What I could not verify

- **Anything in Unreal:** the material, the hue key on the engine's shader, the import, sockets and physics.
- **Deformation:** checked only on Workbench and EEVEE sheets at a few frames, not played through at speed.
- **The cloth weave normal at game distance:** it is invisible in my renders.
- **Hyper3D and Tripo export rights.**
- **Look checks:** no eyes or Gemini verdict. All look judgements above are mine, from the sheets.

## Fourth pass: the jersey and the stride (00:41 to 02:30)

**Short answer.**
- **Deliverable:** `C:\Users\kazda\jam\skater-lab\skc\p4\P4E\export2\P4E\`. It is P4E with the loose jersey, retargeted from `skf/export7/p4_broadcast` exactly as written at 00:43. On the lead's word, that export is final: lane SKF stopped at 00:49 without a report.
- **Ready for import:** `READY.txt` beside the folder lists the exact files for lane B21.
- **A fallback** on the older takes (`skf/export5`, 72 takes) is kept in `skc/p4/P4E/export2_skf5/P4E/`.
- `export/` is untouched.

**The lead's three questions:**
1. **Which takes come from export7.** All of them. The file has **75 takes, every one retargeted from `skf/export7`**. That includes the reworked Stride, Glide and crossovers, and three new takes: `Push_Start`, `Stop_Hockey_Left` and `Stop_Hockey_Right`. No take is carried over from the older export5; the other 72 names are export7's versions of the same takes. If an importer wants the old 72 by name, they are in `export2_skf5`.
2. **The white "shorts" on orange in `lab-skc4-hero-stride-v2`: not intended.**
   - The shorts are team colour. The mask's R channel covers them, so they turn orange like the jersey.
   - The white is the **hem band of the new hanging skirt** (the lower 55 % of a 9 cm skirt). Seen from low behind in the stride's crouch, it covered the seat.
   - On orange it read as white pants for two reasons:
     - my preview's orange recolour pushed the value up 1.6 times, which washed the orange out;
     - the band was too tall.
   - **Fixed in the final files:**
     - the band is now the lower 38 % of the skirt (about 3.5 cm);
     - the preview recolour now keeps the texel's value (times 1.1) at saturation 0.95.
   - The re-rendered sheet is `lab-skc4-hero-stride-v2-final`. In the engine, the team colour comes from the engine's own hue key, not from my preview.
3. **Jersey clipping in knockdown and stop poses** (sheet `lab-skc4-jersey-clipping`, close at 480 px):
   - `Knockdown_Left`, `Knockdown_Right`, `Stop_Hockey_Left`, `Stop_Hockey_Right`, `GetUp_Side_Left`: **no poke-through visible**.
   - **Face-down poses (`Knockdown_Back`, and the same lying pose in `Fall_Forward` and `GetUp_Front`):** the skirt's back panel is pulled up by the thigh weights. It shows as a white triangle of hem band lying over the seat of the pants, with the pants visible through the gap at its edges. This is the one clipping-like fault. It is the skirt being stretched, not the body poking through the jersey.
   - **Stops:** a thin grey sliver of the skirt's inner surface shows at the side hem in `Stop_Hockey_Right`.

### 1. The loose jersey (Blender only; `skc/p4/tools/build_jersey.py`)

**Selecting the jersey.** It is built from P4E's own mesh. The jersey faces are:
- the torso above the hem at 96.5 cm, including the part of the jersey skirt the auto-rig weighted to the thighs;
- the sleeves (upper and lower arm), chosen by bone weight.

Head, neck and gloves are excluded. A 3-round closing fills the small holes. The FBX had split vertices at UV seams, so the jersey (and only the jersey) is welded first. Without that weld, the hem boundary came out as dozens of fragments.

**Building the garment.** The jersey faces are duplicated into a garment surface, keeping the same UVs and weights. The garment is pushed out along the normals:
- 1.2 cm everywhere;
- up to +2.2 cm of room at the waist;
- sleeves up to +1.6 cm at mid-arm, tapering to 0.4 cm at the cuff;
- a broader, squarer shoulder line (pads): 2.2 cm out and 1.2 cm up around each shoulder joint;
- the offset fades to zero at the collar and the cuffs, so there is no gap.

**Folds: procedural.**
- Two crossing families of large diagonal folds from the armpits towards the hem (17 cm wavelength, 3 mm, seeded). They are damped to 35 % on the back, which stays smooth for the engine's number.
- Rings at the elbows (3.5 cm wavelength, 4 mm).

**Cloth simulation: tried and dropped after about 5 minutes.** I separated the garment, pinned it at the collar and shoulders (1.28 to 1.36 m), collided it with the body and ran 40 frames. The skirt fell through to the ice.

**The free skirt.**
- The hem loop (62 vertices) is levelled by an 11-vertex moving average, then extruded 9 cm down and 3 cm out in two rings, with an inner copy 3 mm in for the inside.
- It gets its own UV strip in an unused corner of the atlas (x 1040 to 1400, y 4 to 60 px): the top 45 % is team blue, the bottom 55 % is the white hem band.
- The old white stripe on the jersey body is painted blue, so the hem band now sits at the bottom of the hanging skirt, as in the concept.

**Hidden body faces.** 7,762 body faces under the garment are deleted: the torso and sleeves above 1.02 m. The last 11 cm at the cuffs and 10 cm round the neck are kept.

**Weights.**
- The garment keeps the body's weights.
- Hand and head weights on garment vertices move to the forearm and neck, so the cuffs and collar ride the arm and neck.
- The retarget's skirt re-weight (`skirt_z` [0.965, 1.15, 0.86]) blends the thighs into the pelvis on the skirt, so it swings with the hips and does not split.

**Triangles:** 36,400 for the body (the old figure had 33,512) plus 196 for the stick. That is under 40k.

**Maps.** Re-baked for the new surface: rasterised at 2048, AO baked in Cycles, and `tex_p4.py` re-run with the hem moved to the skirt (`HEM_CM`, `SKIRT_RECT`).
- **Hue key:** linear b - max(r, g) at or above 0.29 on 100 % of team cloth (5th percentile 0.425). No white texel keys.
- **Back:** clean team blue.
- **Deliverables:** base colour, normal (weave, DirectX), mask and AO, at 2048 plus `_1024`.

**Checks** (sheets `lab-skc4-jersey-checks`, `-jersey-close`): Stride, Shot_Slap, Knockdown_Left, Fall_Forward, Stop_Hockey_Left and Push_Start.
- No poke-through or tearing visible at 480 to 640 px.
- The skirt hangs over the pants top in the stride and flips up on the back when lying face down.
- Earlier faults that are now fixed:
  - **A white sawtooth along the hem band:** the UVs were written before a face reversal; they are now set per vertex. The unlevelled hem loop also contributed, and is now levelled.
  - **Rest pose 5.4 mm off at `stick_socket` and 2 mm at the foot pivots:** the retarget measures these from the mesh. Fixed by welding only the jersey and moving the garment's hand and head weights.

**What remains on the jersey:**
- The skirt has 62 segments and reads slightly faceted at 640 px.
- The folds are procedural: plausible, but regular rather than draped.
- A small grey fleck at the V-neck: the generator's collar trim, left as is.
- There is no real cloth motion; the skirt is skinned, not simulated.

### 2. The new takes

**I built both** (same jersey, same rules):

| Build | Takes | Folder |
|---|---|---|
| **Delivered** | `skf/export7`, 75 takes, adding `Push_Start`, `Stop_Hockey_Left` and `Stop_Hockey_Right` | `skc/p4/P4E/export2/P4E/` |
| Fallback | `skf/export5`, 72 takes | `skc/p4/P4E/export2_skf5/P4E/` |

**What the new Stride changes.** The stance is narrower, a leg pushes back, and the stick is carried in front of the body (from behind it is hidden). The wide symmetric squat is mostly gone. `Stop_Hockey_*` reads as a sideways stop with the shoulders turned; `Push_Start` is a crouched first push.

**Retarget rules added in this pass:**
- **The prop rule now covers all of `Board_Hit` and `Lift_Hit`:** the source stick pose is the truth in every frame. On the new takes, `Board_Hit`'s blade step falls from 228 to 99 cm (source 108) and `Lift_Hit`'s from 47 to 30 cm (source 32).
- **The price:** 3 frames of `Board_Hit` with a mitten inside the head radius on the new takes, because the prop pose skips the head clamp. The current takes have none.

**Measured, re-imported main files:**

| | export2_skf5 (older takes, 72) | **export2 (export7 takes, 75, delivered)** |
|---|---|---|
| Pelvis travel, root, main seams | 0 / 0 / 0 | 0 / 0 / 0 |
| Loopcut wraps over 3 cm | `Stride` 4.6 cm | none (largest `Crossover_Right` 1.8 cm) |
| Joins (knockdowns to get-ups, Trip, Poke frame 6 to Stumble_Short) | all 0.000 cm | all 0.000 cm |
| Takes with a blade step over 1.5 times the source's | none | none |
| Hand steps over 1.5 times the source's (ours / source, cm) | GetUp 21/6, Goalie_Block_Left 22/7, Lift 30/14 | Celebrate 26/18, Foul_Reaction 29/16, GetUp 21/6, Goalie_Block_Left 22/7 |
| Stick or mitten inside the head | 0 | 3 frames (Board_Hit) |
| Lying rest (side knockdowns, Trip) | 0.1 to 0.2 cm | 0.1 to 0.2 cm |
| Face-down skates / torso | 0.5 / 8.1 cm | 0.5 / 8.1 cm |
| Aim grid worst; blend test | 4.6 cm / 2.4 deg; 2 cases outside -1..+4 cm | 5.5 cm / 1.9 deg; 5 cases (blade -1.3 to +2.0 cm) |
| Rear-high back covered | up to 26 % (Aim_P70_R130) | Stride 2 to 9 %, Aim_P00_R130 21 %, Aim_P70_R130 26 % |
| New takes, blade step (source), cm | n/a | Stride 1.9 (0.7), Push_Start 4.0 (3.5), Stop_Hockey_Left 17 (16), Stop_Hockey_Right 29 (22) |

**Shippable:**
- **export2_skf5 (fallback):** all 72 takes. Look in the engine at GetUp, Goalie_Block_Left and Lift (the hand moves 2 to 3 times faster than in the source).
- **export2 (delivered):** all 75 takes. Look at Board_Hit (3 frames of mitten in the head radius), plus Celebrate, Foul_Reaction, GetUp and Goalie_Block_Left (hand speed).

### 3. Renders (`devlog/shots/20261113-0229xx-B-lab-skc4-*-final`; the first set at `0205xx` is superseded)

Final sheets (narrower hem band, corrected preview orange) are filed as `lab-skc4-*-final`. The first set, filed at 02:05, is superseded.

| Sheet | What it shows |
|---|---|
| `hero-stride-v2` | The new stride, 8 phases from behind at the concept's framing, blue and orange, with tonight's P4E below |
| `jersey-clipping` | Close views of the knockdowns, stops and a side get-up |
| `hero-stride-v2-current-takes` | The jersey on the old stride |
| `hero-stop` | Stop_Hockey_Left, 6 phases |
| `hero-glide` | Glide frame 1 |
| `jersey-close` | Close 3/4 views of the folds, shoulders and hem |
| `jersey-rest` | The jersey at rest |
| `jersey-checks` | The garment in six poses |
| `strips` | 40 and 70 px from the game camera, blue and orange, against tonight's P4E and P4D |

### How far this moved the figure, honestly

**The jersey moved it the most.** From behind and in 3/4 the figure now has the concept's garment: a boxy torso with room, a squarer shoulder line, wider sleeves, and a skirt hanging over the pants with the white hem band at its edge. The engine lab's item 5 (skin-tight jersey) and item 7 (no pad bulk) are now partly cleared.

**The new stride moved it a little.** The wide squat is gone and a push leg extends behind, but the torso is still upright and compact compared with the concept's forward lunge.

**What is left:**
- real cloth (drape, motion and a softer hem; this needs a simulated or hand-modelled jersey);
- the forward-leaning chest in the stride, which belongs to the takes;
- faceting on the skirt;
- the generator's baked shading under my repaint, which shows as soft lumps on the back in hard light;
- the face, which is still a little stiff.

At the game camera the change is modest: a wider, boxier jersey silhouette and a visible white hem edge. Up close it is clear.

With the narrower hem band, the orange team reads as orange shorts under an orange jersey with a white hem edge.

My estimate: about 75 % of the way to the concept player, up from about 65 %.

### What an importer must know (changed since the third pass)

- **The mesh changed:** a new skeletal-mesh import on the same skeleton.
  - Bone names, hierarchy and rest pose are untouched: every bone matches `export/` to 0.0 (checked on the re-imported files).
  - Vertex count, triangles (36,400), UVs (one new strip) and weights on the garment are new.
  - Takes re-import onto the same skeleton.
- **Back faces:** the skirt has an inner surface, so no two-sided material is needed.
- **Files:** maps by path as before, in the export folder. `T_Skater_P4E_BaseColor.png`, `_Normal` (DirectX), `_Mask` (RGBA) and `_AO`, each at 2048 plus `_1024`.
  - The generator's 4K texture is **not** valid for this mesh, because the skirt strip is new; it is left out.
- **Configs:** `skc/cfg/5_P4E.json` (export2, delivered) and `skc/cfg/4_P4E.json` (the export5 fallback); copies beside the exports.

**Rebuild:** `bash skc/p4/tools/jersey_all.sh 5_P4E` (garment, maps, retarget and rest-pose check).

The delivered maps were made with the final hem band; rebuild them with:

```
HEM_CM=96,96 SKIRT_RECT=1040,4,1400,60 SKIRT_SPLIT=0.62 python skc/p4/tools/tex_p4.py ...
```

**Renders:**
- `bash skc/p4/tools/hero_v2.sh <tag> <stop take>` (set `NEWDIR` to choose the export);
- `bash skc/p4/tools/checks_close.sh <export dir> <tag>`;
- `bash skc/p4/tools/strips.sh <fbx> <name> <orange png>`.

**Credits this pass:** none (Blender only). The balances are unchanged from the third pass: Rodin 397, Meshy 510, Tripo 690.

**Provenance:** the garment geometry, folds, skirt and repainted maps are `original` (our scripts), derived from the ai-generated Rodin mesh and texture. The takes are `original` (`skf`).

**Could not verify:** anything in Unreal; cloth behaviour at game speed (judged from stills only).
