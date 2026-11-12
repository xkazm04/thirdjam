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
