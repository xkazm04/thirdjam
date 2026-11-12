# Skater lab SKC: figures from a concept sheet through device B's skater pipeline (A-to-B-0009), report at 21:05

Lane ran 20:18 to 21:05. All three figures are done end to end, one after the other. None needed a retry of a paid step.

## Short answer

| Figure | Replaces | Look | Motion faults | Replace in the engine? |
|---|---|---|---|---|
| **P2C** cel toy | R (`skh/export5/R`) | **P2C wins close up**: cage helmet, a face with brows and a smile, chunky mittens, laced skates. **At 40 px, R wins slightly**: its skin-tone mittens and blue pants give more contrast than P2C's navy. | **P2C is better** on the aim grid, the side knockdowns and the goalie stick. **P2C is worse** on three get-up blade pops and standing skate error. | **Yes**, with three takes held back (below). |
| **P4C** broadcast | P4B | **A near tie.** P4C has a cage and a clean face, but the head is only 4 % bigger than P4B's (head radius 0.143 against 0.137 m). The "head far too small" verdict is **not** cured. P4B has the more defined shoulder pads. At 40 px they are indistinguishable. | **Mixed.** P4C is better on side knockdowns, face-down skates, grips and blend dips. It is worse on blade pops (5 takes) and falls (skate lift 5 cm). | **No, not on look grounds.** Re-do the sheet with a bigger head if P4 needs one. |
| **P3C** clay | P3A | **Split.** P3A reads more as a clay puppet: soft forms, white mittens, rosy face. P3C reads more as hockey: cage, stitched seams, laced skates. Its clay texture barely shows in Workbench. | **P3C is clearly better.** P3A has 39 stick-in-head frames; P3C has 0. P3A has two stick pops of 196 cm and 164 cm; P3C's worst is 31 cm. P3C's aim holds land within 0.7 cm (P3A: 7.4 cm). | **Yes on motion**; the look is the Director's call (an EEVEE pass is still needed). |

All of this is measured in Blender on the re-imported FBX files. **Nothing has been in Unreal.**

**Credits:**

| | Spent | Budget | Balance before / after |
|---|---|---|---|
| Codex images | 3 | 12 | n/a |
| Rodin | 1.5 credits | 6 | 403.5 / 402 |
| Meshy (shared account) | 15 credits | 60 | 570 / 555 |

## Method (same for all three)

1. **Concept sheet.**
   - One Codex image per figure through `skc/gen/gen.mjs`, a copy of `image-lab/gen.mjs` that runs one job per call.
   - Style line (A's line, plus "no numbers"): *Clean concept sheet for 3D modelling: one character shown twice side by side, a full front view on the left and a full side (profile) view on the right, same scale, feet on the same baseline, plain pure white background, soft even studio lighting, no shadows on the ground, no text, no labels, no numbers, no logos, no watermark, nothing else in the frame.*
   - The full text sent to Codex sits beside each sheet as `*.prompt.txt`.
   - All three sheets were clean on the first try: two views, no stick, feet in frame, no text.
   - `skc/crop.py` splits each sheet at the white gap between the two views and pads each view to a square.
2. **Mesh.** Rodin `image-to-3d` through `rodin-tools/rodin.mjs`:
   - Gen-2.5-Low, `mesh_mode` Quad, `quality_override` 18000, TAPose true, GLB;
   - images: the front crop then the side crop;
   - each prompt names the colours on the objects (S3's finding).
3. **Rig.** Meshy `rig --model-url <Rodin GLB link> --template ue5_mannequin`.
   - Heights from `skf/skf_style.py` `top`: p2_cel **1.73 m**, p4_broadcast **1.80 m**, p3_clay **1.66 m**.
   - The brief said 1.64 m for P3; the table says 1.66 m, so I used 1.66 m.
4. **Normalising: `retarget.py`'s own preparation, not A's `normalize_fbx.py`.**
   - `retarget.py` reads the raw Meshy FBX. It turns the figure to +X, applies the transforms, adds a real `root` bone at the origin as the parent of `pelvis`, adds the sockets, and exports a centimetre file.
   - Re-import check: armature object scale 0.01, `root` is the only top bone, root rest is identity.
   - A's script instead renames the armature *object* `root`, so UE invents the root bone. B's takes key a real `root` bone, so A's script would not match them.
5. **Retarget.**
   - Script: `skc/retarget.py`, a copy of `skh/retarget.py` (19:18) with one addition: config key `cont_takes`.
   - **No seventh-pass section existed** in `B-skater-skh.md` (it ends at the sixth pass, 18:50; `export7` was built at 19:20 but not reported). So I followed the sixth pass's per-take choice.
   - **Sixth-pass rules on:** settle (all its lying and kneeling takes), stick-head clearance (`Board_Pin`, `Knockdown_*`), reach rule and ice tilt.
   - **Stick continuity only on:** `Fall`, `Fall_Forward`, `GetUp`, `GetUp_Front`, `Knockdown_*`, `Trip` and `Board_Pin`. Off everywhere else, including `Stride` and `GetUp_Side_*`, as the sixth pass advised.
   - Settle stays on in `GetUp_Side_*` so that each knockdown ends exactly where its get-up starts (0.000 cm on every figure).
6. **Verify.**
   - The skh scripts, copied into `skc/`: `verify.py` (main and loopcut), `newtakes.py`, `gridcheck.py`, `rearhigh.py`, `handjump.py`, `lyingparts.py`.
   - **The same scripts were run on the current figures** (`skh/export5/{R,P4B,P3A}`, read only, into `skc/out/cmp_*`), so the tables below compare like with like.
   - `skc/summary.py` writes the tables.
7. **Sheets.** `skc/render_sheets.sh` (Workbench) renders:
   - a turntable;
   - Stride from 3/4, side and behind;
   - the slap from the side and 3/4;
   - the 24 aim holds from above;
   - six lying takes from the side;
   - a 40 px strip from the game camera (behind and above) in blue, then hue-keyed orange.

   `skc/ab.sh` renders the A/B pairs. A face-on 40 px pair was added for each figure.
8. **Orange.** `skc/tex/huekey.py` turns hues 190 to 260 degrees (saturation above 0.35, value above 0.30) to orange. Navy stays as it is.

## P2C "cel toy"

**Concept prompt (verbatim; the style line above follows it):** `A chunky stylized toy ice hockey player about three and a half heads tall: a big round head in a large deep blue hockey helmet with a white metal cage over the face, a friendly simple face with big dark eyes, eyebrows and a small smile that read clearly through the cage, short thick arms and legs, big rounded dark navy hockey mittens, a deep blue hockey jersey with white trim at the collar, cuffs and hem and a plain unmarked back, dark navy hockey pants, deep blue socks with one white band, black ice skates with silver blades. Empty hands: no hockey stick, nothing held. Painted cel-shaded toy look, large flat solid colour areas, matte plastic surfaces. A-pose: arms held straight and angled down away from the body with a clear gap under each arm, legs slightly apart, arms and legs clearly separated from the body. No numbers, no letters and no logos anywhere on the clothing or helmet.`

**Rodin prompt:** `stylized chunky toy ice hockey player from the concept sheet, deep blue jersey with white trim, dark navy mittens, deep blue helmet with a white metal cage, black skates with silver blades, empty hands, no stick, A-pose`

**Generation:**

| Step | Id | Credits | Time | Result |
|---|---|---|---|---|
| Rodin | `c439ca2c-510e-4831-8c7f-0a50b652d62e` | 0.5 | 61 s | 37,280 tris, 1.73 m as delivered |
| Meshy rig | `01a1d137-c464-71be-b85a-5eb778cf905a` | 5 | | |

**Turntable as generated:**
- Cage present.
- Arms clear of the body.
- Plain back.
- No fused parts.

**Texture:**
- The rig's base colour is 2048 px, 8-bit RGB. The normal and ORM maps are dropped.
- S6's detector flagged 40 to 150 pointer clusters per band, the same as on every other atlas.
- By eye (8 head and chest close-ups, 8 leg close-ups, the 1K atlas) **no letter-like marks**. The white laces are shapes.
- No cleaning was needed: the PNG is a straight copy.

**Measured** (main file; config `cfg/P2C.json`, override none, all values measured):

| Check | P2C | R (export5, same scripts) |
|---|---|---|
| K / head r / mitten r / head clear / stick-head clear (m) | 0.610 / 0.347 / 0.122 / 0.469 / 0.439 | 0.586 / 0.278 / 0.133 / 0.411 / 0.358 |
| takes / bones / pelvis travel / root / main seams | 70 / 32 / 0.000 / 0.000 / 0.000 | same |
| loopcut wraps over 3 cm | 0 (Stride 1.2) | 0 (Stride 1.9) |
| lowest body / stick (cm) | -0.92 / -0.98 (settle tolerance) | 0.00 / -0.01 |
| standing skate error worst (cm) | 2.4 (Check_Deliver_Left) | 0.3 |
| left mitten to shaft, mean of takes / worst (cm) | 6.7 / 29.6 | 8.6 / 38.4 |
| Goalie_Hold stick low (S4 16-19 cm) | **17.3** | 26.4 |
| aim grid worst position / angle | **0.8 cm / 0.22 deg** | 6.1 cm / 2.05 deg |
| aim holds blade low (0-2.5 cm) | 0.0 to 1.0 | 0.0 to 1.5 |
| blend: worst error / blade range / cases outside -1..+4 | 4.4 / -3.7..+2.4 / 20 | 6.5 / -4.8..+1.4 / 21 |
| rear-high back covered | 0 % except `Aim_P00_R130` 5 % | 0 % |
| side knockdowns rest, hip / skate (cm) | **0.1 / 0.1 and 0.0 / 0.0** | 13.6 / 0.3 and 5.2 / 0.3 |
| face-down rest, skate up (cm) | 6.6 | 5.2 |
| knockdown end to get-up start | 0.000 on all 5 pairs | same |
| blade pops (step > 1.5 x source) | **GetUp_Side_Left 36 vs 9, GetUp_Side_Right 32 vs 17, Goalie_Block_Left 23 vs 7** | none |
| stick or mitten inside the head | 0 | 0 |

**What is wrong:**
- **Three get-up blade pops.** One-frame blade jumps of 23 to 36 cm in `GetUp_Side_Left`, `GetUp_Side_Right` and `Goalie_Block_Left`. Blade contact error in `GetUp_Side_Left` is 36.6 cm.
  - The cause is the settle rule plus the ice-safe tilt.
  - A variant with stick continuity also on these three takes (`skc/exportv/P2C_cont`, not an export) gave **identical** numbers, so continuity does not fix it.
- **Face-down skates** are 6.6 cm up (R: 5.2 cm).
- **The big helmet sits low** on the upper back from behind. The jersey back from the shoulders to the hem is clear for a number.
- **At 40 px from behind,** the navy pants and mittens make the figure darker than R. In the hue-keyed orange, the helmet goes brown-orange.
- **At 40 px face-on,** the eyes read through the cage, but the cage greys the face (sheet `ab-face40`). From the game camera (behind), no face is visible on any figure.

**A/B against R:**
- Sheets: `devlog/shots/20261112-203521-B-lab-skc-p2-ab-p2.png` and `20261112-204044-B-lab-skc-p2-ab-face40.png`.
- **Look:** P2C close; R slightly at 40 px.
- **Motion:** P2C on holds, side knockdowns and the goalie stick; R on the get-up pops and standing skates.

**Replace?** Yes, from `skc/export/P2C/`.
- It has both things the bake-off split between the B figures (toy-ness and the cage hockey read).
- **Not good enough:** `GetUp_Side_Left`, `GetUp_Side_Right` and `Goalie_Block_Left` (blade pops).
- **Marginal:** face-down skates 6.6 cm up (`Fall_Forward`, `Knockdown_Back`, `Trip`, `GetUp_Front`).

## P4C "broadcast"

**Concept prompt (verbatim):** `A stylized but believable adult ice hockey player in full gear, about six and a half heads tall with a properly sized head (not small): a deep blue hockey helmet with a white metal cage over the face, a friendly clear face with visible eyes behind the cage, broad shoulder pads under a loose deep blue hockey jersey with white trim at the collar, cuffs and hem and a plain unmarked back, big padded deep blue hockey gloves, padded dark navy hockey pants, deep blue socks with one white band over shin guards, black ice skates with silver blades. Empty hands: no hockey stick, nothing held. Stylized sports video game character, clean simplified shapes, matte materials, large solid colour areas. A-pose: arms held straight and angled down away from the body with a clear gap under each arm, legs slightly apart, arms and legs clearly separated from the body. No numbers, no letters and no logos anywhere on the clothing or helmet.`

**Rodin prompt:** `stylized believable ice hockey player from the concept sheet, deep blue jersey with white trim over broad shoulder pads, big deep blue gloves, dark navy padded pants, deep blue socks with a white band, deep blue helmet with a white metal cage, black skates with silver blades, empty hands, no stick, A-pose`

**Generation:**

| Step | Id | Credits | Time | Result |
|---|---|---|---|---|
| Rodin | `0c3f767e-fddf-494f-9b91-4de4e678cea3` | 0.5 | 70 s | 34,914 tris, 1.72 m as delivered |
| Meshy rig | `01a1d144-8daa-70f4-9211-07b14eeb82dc` | 5 | | 1.80 m |

**The mesh as generated:**
- The cage is present but thin.
- Arms are clear of the body.
- The sheet itself is about 6.1 heads tall; the mesh is about 6.3 (head radius 0.143 m on 1.80 m).

**Texture:**
- 2048 px, 8-bit RGB. No marks by eye: the skate laces are X-crossings, and there is a black chin strap.
- No cleaning was needed: the PNG is a straight copy.

**Measured** (config `cfg/P4C.json`; boot re-weight on, measured calf share 0.087):

| Check | P4C | P4B (export5, same scripts) |
|---|---|---|
| K / head r / mitten r | 0.871 / 0.143 / 0.115 | 0.911 / 0.137 / 0.093 |
| travel, root, seams | 0 / 0 / 0 | same |
| loopcut wraps over 3 cm | Stride 4.9 | Stride 5.3 |
| lowest body / stick (cm) | -0.98 / 0.00 | 0.00 / 0.00 |
| standing skate worst (cm) | 3.9 | 6.3 |
| mitten to shaft, mean / worst (cm) | **5.2** / 33.0; Glide 0.1 | 8.5 / 33.7; Glide 4.2 |
| Goalie_Hold stick low | 16.7 | 17.3 |
| aim grid worst position / angle | 4.6 cm / 1.30 deg | 3.7 cm / 2.09 deg |
| blend worst / blade range / cases outside | 5.6 / -2.2..+2.2 / **1** | 6.1 / -2.4..+2.2 / 2 |
| rear-high | `P70_R130` 17 %, `P00_R130` 2 %, else 0 | `P70_R130` 13 % |
| side knockdowns rest, hip (cm) | **0.0 / 0.0** | 10.4 / 9.5 |
| face-down rest, skate / torso (cm) | 11.8 / 16.3 | 15.0 / 10.7 |
| skate lift Fall / GetUp (cm) | 5.1 / 4.5 | 1.4 / 0.2 |
| blade pops | GetUp_Side_Left 41 vs 9, GetUp_Side_Right 33 vs 19, Goalie_Block_Left 36 vs 7, Lift 34 vs 18, Lift_Hit 30 vs 16 | Lift_Hit 24 vs 16, Stride 16 vs 8 |

**What is wrong:**
- **The head is still small.** The concept route does not by itself fix "head far too small"; Rodin follows the sheet, and the sheet came out at about 6 heads. A larger head needs a sheet that asks for about 5 to 5.5 heads.
- **Five takes with blade pops.**
- **Falls lift the skate** 5 cm.
- **The face-down torso** rests 16 cm up.
- **At 40 px** it looks the same as P4B.

**A/B against P4B:**
- Sheets: `devlog/shots/20261112-204654-B-lab-skc-p4-ab-p4.png` and `-ab-face40.png`.
- **Look:** a tie close up (P4C has the cage and the cleaner face; P4B the pads) and a tie at 40 px.
- **Motion:** mixed (above).

**Replace? No.** Keep P4B. P4C is a valid alternative on the same skeleton type, but it is not the step up that A's toy Rodin2 was.

## P3C "clay"

**Concept prompt (verbatim):** `A hand-made clay miniature figure of a chunky ice hockey player about three and a half heads tall, sculpted in matte deep blue modelling clay with visible thumb dents and soft, slightly irregular rounded forms: a big round head in a large deep blue clay hockey helmet with a white clay cage over the face, a friendly simple clay face with dot eyes, eyebrows and a small smile visible through the cage, a deep blue clay jersey with a white stitched seam along the shoulders and sides, white trim at the cuffs and hem and a plain unmarked back, short thick arms with big rounded dark navy clay mittens, dark navy clay hockey pants, deep blue clay socks with one white band, black clay skates with pale grey blades. Empty hands: no hockey stick, nothing held. Stop-motion puppet look, matte clay surfaces with fingerprint texture. A-pose: arms held straight and angled down away from the body with a clear gap under each arm, legs slightly apart, arms and legs clearly separated from the body. No numbers, no letters and no logos anywhere on the clothing or helmet.`

**Rodin prompt:** `hand-made clay miniature of a chunky ice hockey player from the concept sheet, matte deep blue modelling clay with thumb dents, deep blue clay jersey with a white stitched seam, dark navy clay mittens, deep blue clay helmet with a white clay cage, black clay skates, empty hands, no stick, A-pose`

**Generation:**

| Step | Id | Credits | Time | Result |
|---|---|---|---|---|
| Rodin | `cb0f0a16-7ce4-4b84-87c7-78027c268ee8` | 0.5 | 50 s | 37,404 tris |
| Meshy rig | `01a1d14e-aaf9-736c-8ec3-ee298af3bd93` | 5 | | 1.66 m |

**Texture:**
- 2048 px RGB. Clean by eye: the stitches are white dashes along the seams, as asked.
- No cleaning was needed.

**One retry, of the retarget step (free):**
- The first build stopped. The boot is skinned asymmetrically: the left boot is calf-dominant down to the sole, the right one foot-dominant. So no calf-dominant vertex lay within 2 cm of the measured ankle height (0.182 m), and the boot band was empty.
- I set `override.ankle_z` to 0.21, read from a dominance-by-height probe (`skc/probe_ankle.py`): at 0.21 to 0.22 m both calves dominate. This is the "cannot be measured" value in the config.
- `grip_offset_m` is -0.10, as P3A's (the p3_clay child grip).

**Measured:**

| Check | P3C | P3A (export5, same scripts) |
|---|---|---|
| K / head r / mitten r | 0.622 / 0.307 / 0.115 | 0.738 / 0.295 / 0.140 |
| travel, root, seams; wraps over 3 cm | 0 / 0 / 0; none | same; none |
| lowest body / stick (cm) | -0.74 / -0.89 | 0.11 / -0.01 |
| standing skate worst (cm) | **0.6** | 3.0 |
| slap grip mean / max (cm) | **0.7 / 5.8** | 10.8 / 18.5 |
| stick or mitten in head (frames) | **0** | 39 (`Board_Pin`, `Board_Hit`) |
| Goalie_Hold stick low (16-19) | **19.7** | 33.6 |
| aim grid worst position / angle | **0.7 cm / 0.13 deg** | 7.4 cm / 2.81 deg |
| bottom mitten to shaft, two-handed holds | 0.1 to 5.7 cm | 0.2 to 10.7 cm |
| blend worst / blade range / cases outside | 3.5 / -2.7..+0.8 / 17 | 4.0 / -3.6..+1.9 / 18 |
| rear-high | 0 % everywhere | 0 % |
| side knockdowns rest (hip, cm) | 0.8 / 0.3 | 3.0 / 0.8 |
| face-down rest, torso / skate (cm) | 8.2 / 4.7 | 6.0 / 1.0 |
| blade pops | GetUp_Side_Left 28 vs 7, GetUp_Side_Right 31 vs 17 | **Board_Hit 196 vs 95, GetUp_Side_Left 164 vs 7**, GetUp_Front 20, GetUp_Side_Right 28 |

**What is wrong:**
- **Two get-up blade pops** (28 to 31 cm).
- **Face-down,** the torso rests 8 cm up and the skates 4.7 cm.
- **The look in Workbench** is "a toy with a clay texture" more than a clay puppet. The thumb dents are painted, not sculpted, and do not show at 40 px.
- I did not render an EEVEE clay pass (skh's `tex/render_clay.py`) for lack of time.

**A/B against P3A:**
- Sheets: `devlog/shots/20261112-210127-B-lab-skc-p3-ab-p3.png` and `-210128-…-ab-face40.png`.
- **Look:** split. P3A wins on clay-ness and the face at 40 px (no cage). P3C wins on the hockey read (cage, skates, seams).
- **Motion:** P3C clearly.

**Replace?**
- **Yes for motion.** P3C has none of P3A's held-back faults (`Board_Hit`, `GetUp_Side_Left`, `Board_Pin`).
- **The look is the Director's call** after an EEVEE or engine clay pass.
- **Not good enough:** `GetUp_Side_Left` and `GetUp_Side_Right`.

## What an importer must know (all three)

- **Files:**
  - `C:\Users\kazda\jam\skater-lab\skc\export\<P2C|P4C|P3C>\SK_Skater_<fig>.fbx` and `_loopcut.fbx`, 70 takes at 60 fps.
  - Beside them: `T_Skater_<fig>_BaseColor.png` (2048, sRGB, referenced by path, not embedded), the concept sheet, its `.prompt.txt`, both crops and the config `<fig>.json`.
- **Skeleton:**
  - 32 bones, `root` on top with an identity rest; the Meshy `ue5_mannequin` names; centimetres (armature scale 0.01 on re-import).
  - Sockets `stick_socket` (a bone, animated per take), `head_socket` and the others as in skh.
  - Bind pose: Meshy's A/T pose from Rodin's TAPose.
  - **It is a new skeleton asset per figure** (its own bone lengths), not the R, P3A or P4B skeleton.
- **Materials:** one body slot `M_Skater_<fig>`, plus the stick's two flat slots (`Trim`, `Stick`). The stick is 196 tris and part of the file.
- **Grip rotation:** not computed. A's `ue_grip.py` runs only in the UE editor and is meant for a separate stick on a socket. B's export carries the stick as a mesh on the animated `stick_socket` bone, so no socket rotation is needed.
- **Physics** (A's capsule note): not built here (it is an editor step). The toy figures have big heads (head radius 0.31 to 0.35 m), so use A's hand-made 13 capsules, not auto bodies.

## Provenance

| File | Origin | Tool |
|---|---|---|
| `p?-sheet-1.png`, the front and side crops | ai-generated (crops: our script) | Codex image generation |
| mesh, UVs, base colour in each `SK_Skater_<fig>*.fbx` and its PNG | ai-generated | Hyper3D Rodin Gen-2.5-Low (task ids above) |
| skeleton and skin weights | ai-generated, modified | Meshy rig (ids above); `root`, sockets, foot pivot and boot re-weight by `skc/retarget.py` |
| the 70 takes | original | SKF `build_clips.py` (`skf/export4`), retargeted by `skc/retarget.py` |
| the stick | original | S4's `build_skater.py` |
| `tex/<fig>/orange.png` (test only, not exported) | ai-generated, modified | `skc/tex/huekey.py` |

## Rebuild

- Everything after the paid calls, with no API calls: `bash C:/Users/kazda/jam/skater-lab/skc/run_all.sh`, about 25 min.
- One figure: `bash skc/build.sh <fig>`, then `bash skc/render_sheets.sh <fig>`.
- Paid steps: `node skc/gen/gen.mjs <job>`, then `rodin.mjs image-to-3d` and `meshy.mjs rig` as in `skc/ledger.md`.
- Ledgers: `skc/rodin-ledger.jsonl`, `meshy-ledger.jsonl`, `gen/gen.log`, `balances.jsonl`, `ledger.md`.
- `skh/`, `skf/` and `s*/` were not modified.

## Sheets (`devlog/shots/`)

| Figure | Files |
|---|---|
| P2C | `20261112-2035xx-B-lab-skc-p2-{concept,rodin-turntable,turntable,stride,slap,aimgrid-above,knockdowns,40px,ab-p2}.png`, `20261112-204044-B-lab-skc-p2-ab-face40.png` |
| P4C | `20261112-2046xx-B-lab-skc-p4-{concept,rodin-turntable,turntable,stride,slap,aimgrid-above,knockdowns,40px,ab-p4,ab-face40}.png` |
| P3C | `20261112-2101xx-B-lab-skc-p3-{concept,rodin-turntable,turntable,stride,slap,aimgrid-above,knockdowns,40px,ab-p3,ab-face40}.png` |

## What I could not verify

- **Anything in Unreal:** import, skeleton creation, sockets, physics, the grip, the toon material, the hue key on the engine's shader.
- **Rodin's multi-view mode:** the `condition_mode` field was not set, following A's recipe. I relied on the result (one body, both views honoured).
- **Clay look:** no EEVEE pass for P3C, and Workbench hides the painted dents.
- **The 40 px renders** are Workbench downsamples, not the engine at 1080p.
- **The seventh pass of the skh lane** (`skh7` code, `export7`) was not used, because it has no report section.
- **Hyper3D export rights** (S3's open question).
- **Eyes or Gemini A/B verdicts:** not run. The A/B verdicts above are my own reading of the sheets.

## Second pass (21:04 to 21:45): P4 with a bigger head, the 72 new takes, the stick prop rule

**Short answer:**
- **P4D (a new broadcast player with a bigger head) passes on look.** It is the clearest read of the three P4 figures close up, and slightly the best at 40 px. Two of its takes are not good enough (`Board_Hit`, `Lift`).
- **The prop rule works.** On all three figures, the get-up and goalie-block blade jumps are now at or under the source's own (target was at most 1.5 times the source). The stick never goes below the ice, and the hands stay on the shaft.
- **All exports now have the scripted family's 72 newest takes** (`skf/export5`). Every knockdown-to-get-up join is exact (0.000 cm), and `Stumble_Short` starts exactly on `Poke` frame 6.
- **Files:**
  - `C:\Users\kazda\jam\skater-lab\skc\export2\{P2C,P3C,P4D}\SK_Skater_<fig>.fbx` and `_loopcut.fbx`.
  - Beside each: the base-colour PNG (2048), the concept sheet and its prompt, both crops, and the config `<fig>.json`.
  - `skc/export/` is untouched.
- **P2C and P3C keep their pass-1 skeleton, bind pose and mesh.** `bindcheck`, export against export2: bone names and parents equal, rest-pose difference 0.0. Only the animations changed.
- All of this is measured in Blender on the re-imported FBX files. Nothing has been in Unreal.

**Credits this pass** (all 2026-11-12 UTC; rows in `skc/ledger.md`):

| | Spent | Second-pass budget | Balance before / after |
|---|---|---|---|
| Codex images | 2 | 9 | n/a |
| Rodin | 1.0 credit | 4.5 | 402 / 401 |
| Meshy (shared account) | 10 credits | 45 | 555 / 545 |

**Lane total:** 5 images, 2.5 Rodin credits, 25 Meshy credits.

### 1. P4D: the head fixed

**First sheet** (`p4d-sheet-1`; Rodin `4b694e2f-4ce7-4d8b-ad92-3849d4ee05e7`; Meshy rig `01a1d161-c606-75ce-9d8c-3fe4c9dcedd8`):
- It came out at about **3.7 heads tall**, helmet included (head radius 0.266 m on 1.80 m): a bobble-head, not the 5 to 5.5 heads asked for.
- I used the one allowed retry. The first mesh is kept in the lab only (`skc/export2v/P4Dtoy`, concept in `export2v/P4Dtoy_concept`) and is not an export.

**Retry, `p4d-sheet-2` = P4D.**
- Prompt verbatim; the pass-1 style line follows it: `A stylized broadcast-style ice hockey player in full gear, an adult athletic build about five and a half heads tall measured with the helmet: the head and deep blue hockey helmet about one quarter larger than on a real player (bigger than realistic, but not a toy or a child), a white metal cage over the face that shows a friendly adult face with clear eyes, broad shoulder pads under a loose deep blue hockey jersey with white trim at the collar, cuffs and hem and a plain unmarked back, slightly oversized padded deep blue hockey gloves, padded dark navy hockey pants, deep blue socks with one white band over shin guards, slightly oversized black ice skates with silver blades, long sturdy legs. Empty hands: no hockey stick, nothing held. Stylized sports video game character, clean simplified shapes, matte materials, large solid colour areas. A-pose: arms held straight and angled down away from the body with a clear gap under each arm, legs slightly apart, arms and legs clearly separated from the body. No numbers, no letters and no logos anywhere on the clothing or helmet.`
- Rodin `ca2526c2-a9c3-494e-a2fb-c29f0dde9609` (0.5 credit, 36,020 tris). Meshy rig `01a1d16a-4f72-748c-9bab-f5d2dbf213fe` (5 credits, 1.80 m).
- **Texture:** 2048 px. No marks in the head and leg close-ups. Straight copy, no cleaning.

**Head against height:**
- Head radius: the mean half-extent of the head-dominant vertices, helmet included (`skc/headratio.py`; the same value `retarget.py` measures).
- Sheet: helmet top to chin, measured on the concept image.

| Figure | Height | Head radius | Height / head diameter | Heads tall on the sheet |
|---|---|---|---|---|
| P4B (skh) | 1.80 m | 0.137 m | 6.6 | (text route, no sheet) |
| P4C (pass 1) | 1.80 m | 0.143 m | 6.3 | 6.1 |
| **P4D** | 1.80 m | **0.171 m** (+25 % on P4B) | **5.3** | **5.3** |
| (P4D first sheet, not used) | 1.80 m | 0.266 m | 3.4 | 3.7 |

**A/B, all three in the same pose and camera** (`devlog/shots/20261112-214200-B-lab-skc2-p4-ab-bcd.png`):
- **Close up** (Glide 3/4, `Idle_Faceoff` front): P4D's larger helmet and caged face read as a stylised game player; P4B and P4C read as small-headed adults.
- **At 40 px from the game camera:** the three are close. P4D's helmet is visibly the biggest dot.
- **Face-on at 40 px:** P4D's face and cage read best.
- **Verdict on look:** P4D wins, by a clear margin close up and a small one at game distance.

**P4D measured** (72 takes from `p4_broadcast`; config `export2/P4D/P4D.json`, measured values only):

| Check | Value |
|---|---|
| Takes, pelvis travel, root, main seams | 72; 0.000; 0.000; 0.000 |
| Loopcut wraps over 3 cm | `Stride` 5.2 cm |
| Lowest body / stick (cm) | -0.96 / -0.93 |
| Standing skate error | 2.0 cm |
| Goalie stick | 19.8 cm |
| Aim holds, worst | 3.6 cm / 1.85 deg |
| Blend test | worst 5.6 cm; blade -2.6 to +2.3 cm; 2 cases outside -1 to +4 |
| Bottom hand on the right-side holds | up to 20 cm off the shaft |
| Rear-high | `Aim_P70_R130` 26 % (right glove at the shoulder); 0 elsewhere |
| Stick or mitten inside the head | 0 frames |
| Side knockdowns and `Trip` rest | 0.1 cm |
| Face-down skates | 11.5 cm up |
| Skate lift in `Fall` | 6.7 cm |
| **Blade jumps more than 1.5 times the source's** | **`Board_Hit` 200 vs 108 cm, `Lift` 40 vs 22 cm** |

**Replace P4B?** On look, yes. Hold back `Board_Hit` and `Lift`.

### 2. The 72 takes (skf export5) on P2C, P3C and P4D

What changed in `skc/retarget.py` (copies, nothing in `skh/` touched):
- `Idle` is a loop.
- `Trip` is no longer face-down, so the face-down legs rule (unused, `skh7`) is off for it (config `trip_side`).
- **New start-match rule (`match_start`).** A take that must start exactly where another ends gets that take's final pose at frame 0, and the difference is blended out over 12 frames. The stick is floored after the blend.
  - Used for `Stumble_Short` from `Poke` frame 6, `GetUp_Side_Left` from `Knockdown_Left`'s end, and `GetUp_Side_Right` from `Knockdown_Right`'s end.
  - Why: the bodies already matched, but the hands and stick did not. On P2C, `Poke` frame 6 against `Stumble_Short` frame 0 was 23.6 cm (left hand), because the two takes have different grip weights and reach slides.

`newtakes.py` now checks `Trip -> GetUp_Side_Right` and `Poke@6 -> Stumble_Short`.

**Join checks** (bone for bone, re-imported main file), on P2C, P3C and P4D:

| Join | Result |
|---|---|
| `Knockdown_Left -> GetUp_Side_Left` | **0.000 cm** |
| `Knockdown_Right -> GetUp_Side_Right` | **0.000 cm** |
| `Trip -> GetUp_Side_Right` | **0.000 cm** |
| `Knockdown_Back -> GetUp_Front` | **0.000 cm** |
| `Fall_Forward -> GetUp_Front` | **0.000 cm** |
| `Poke` frame 6 -> `Stumble_Short` frame 0 | **0.000 cm** |

**The changed takes** (largest one-frame blade step, ours against the source; lowest stick; left mitten to shaft mean / max):

| Take | P2C | P3C | P4D |
|---|---|---|---|
| `Lift` | 18 vs 21; 0.7; 4.6 / 7.5 | 15 vs 20; 0.0; 4.2 / 6.2 | **40 vs 22**; 3.6; 0.5 / 1.5 |
| `Stumble_Short` | 55 vs 60; 0.0; 3.9 / 4.4 | 55 vs 56; 0.0; 3.9 / 4.6 | 73 vs 67; 0.0; 4.6 / 7.2 |
| `Idle` | static; 0.7 | static; 0.0 | static; 4.7 |
| `Check_Hit_Left/_Right/_Back` | 65-73 vs 79-106 | 65-76 vs 75-100 | 63-94 vs 82-114 |
| `Poke_Hit` | 110 vs 115 | 106 vs 115 | 132 vs 122 |
| `Lift_Hit` | 28 vs 31 | 28 vs 30 | 42 vs 32 |
| `Board_Hit` | 104 vs 100 | 99 vs 95 | **200 vs 108** |
| `Check_Deliver_Left` | 30 vs 31 | 27 vs 28 | 32 vs 36 |
| `Trip` (ends on its side) | 102 vs 136; rests at 0.0 / 0.0 cm | 104 vs 137; rests at 0.3 / 0.1 | 156 vs 146; rests at 0.1 / 0.1 |

**New faults from the stronger takes:**
- P2C has 6 frames with the stick inside the big helmet's radius (`Check_Hit_Right`, `Trip`).
- P3C has 5 frames (`Check_Hit_Right`).
- P4D has none.

**Unchanged from pass 1** on P2C and P3C: aim holds, blend test, rear-high, goalie stick, travel, seams, wraps.

### 3. The prop rule

**What it does:**
- **Takes:** `GetUp_Side_Left`, `GetUp_Side_Right`, `Goalie_Block_Left`, plus `Knockdown_Left`, `Knockdown_Right` and `Trip`. The last three are included because their end frames are the get-ups' first frames; without them the joins broke by 16 cm.
- **When:** on frames where the source stick's lowest vertex is within the threshold of the ice, blended over 4 frames.
- **The truth is the source stick's world pose:**
  - one anchor vertex per take (the one most often lowest) is placed at the source's position, scaled horizontally about the pelvis by the leg ratio K, at the source height;
  - the stick takes the source's orientation;
  - the right arm reaches the grip by two-bone IK;
  - the stick then pivots about the anchor so the shaft runs through the mitten the arm reached;
  - it is floored, and the left hand re-grips.
- **Frames outside the prop phase are untouched.**

**Three deviations from the brief, all measured:**
1. **Threshold 20 cm, not 3 cm, for `GetUp_Side_*`.** At 3 cm the prop phase began mid-take, and the 4-frame blend-in itself jumped the blade (21 to 24 cm). In those takes the source props the stick 4 to 10 cm above the ice for its first 13 frames.
2. **Threshold 25 cm for `Goalie_Block_Left`.** Its source stick is held 16 to 20 cm up and never comes within 3 cm. The jump there came from the settle rule's whole-body pitch swinging the stick, and making the source pose the truth removes it.
3. **One anchor vertex per take.** A per-frame lowest vertex switched between butt and blade, which moved the K-scaled anchor by (1 - K) times the stick length in one frame (the 15.6 cm step at frame 28 on P2C).

**Before (pass 1) -> after (pass 2)**, largest one-frame blade step, ours against the source; target at most 1.5 times the source:

| Take | P2C | P3C | P4D (no pass-1 version; P4C pass 1 for reference) |
|---|---|---|---|
| `GetUp_Side_Left` | 36.3 -> **8.5** (src 8.7) | 27.9 -> **7.2** (src 6.7) | **7.4** (src 9.0) |
| `GetUp_Side_Right` | 32.4 -> **14.9** (src 17.3) | 30.6 -> **15.8** (src 17.1) | **21.3** (src 18.6) |
| `Goalie_Block_Left` | 23.4 -> **4.3** (src 6.8) | 6.3 -> **4.2** (src 6.4) | **5.9** (src 7.2) |

The other targets, all met:

| Check | P2C | P3C | P4D |
|---|---|---|---|
| Stick never below the ice | lowest 0.0 cm | 0.0 cm | 0.0 cm |
| Left mitten to shaft in `GetUp_Side_*` (mean / max) | 2.1 / 2.9 cm | 2.3 / 3.7 cm | 1.9 / 2.5 cm |
| Mitten radius | 12.2 cm | 11.5 cm | 12.2 cm |

The right mitten is on the shaft by construction (the shaft is pivoted through it).

**The price: in `GetUp_Side_Left` the blade is held off the ice** where the source's blade is on it (28 / 23 / 17 cm on P2C / P3C / P4D). The figure props on the butt end, because the arm cannot reach the source grip while both ends stay on the ice. The sheets read it as leaning on the stick (`lab-skc2-p2-getup`, `-p3-getup`, `-p4-getup`).

**Extension to `GetUp`, `GetUp_Front`, `Knockdown_Back`, `Fall_Forward` and `Fall`: tested on P2C and not applied.** `GetUp_Front`'s blade step got worse (10.7 -> 21.7 cm against a source of 10.3). It shares joins with `Knockdown_Back` and `Fall_Forward`, so the group cannot be split. `GetUp` alone improved (lowest stick -0.5 -> 0.0 cm, grip mean 6.2 -> 3.8 cm), but it was left off to keep the rule to the asked takes. The test build is `export2v/P2C_ext`.

### Shippable takes per figure (export2)

| Figure | Ship | Hold back | Marginal (ship, but look in the engine) |
|---|---|---|---|
| **P2C** | 70 of 72 | none by the blade-jump measure | `Check_Hit_Right`, `Trip` (6 frames with the stick inside the big helmet's radius). Face-down skates 6.6 cm up (`Fall_Forward`, `Knockdown_Back`, `GetUp_Front`). `GetUp_Side_Left` props on the butt end. |
| **P3C** | 71 of 72 | none | `Check_Hit_Right` (5 frames with the stick in the helmet radius). Face-down skates 4.7 cm up. |
| **P4D** | 70 of 72 | **`Board_Hit`** (200 cm blade pop, source 108); **`Lift`** (40 cm against the source's 22) | Face-down skates 11.5 cm up. `Fall` skate lift 6.7 cm. `Aim_P70_R130` covers 26 % of the upper back. Bottom hand up to 20 cm off the shaft on the right-side holds. |

### What an importer must know (changes since the first report)

- **Import from `export2/`, not `export/`.**
  - 72 takes at 60 fps; new are `Idle` (a loop) and `Stumble_Short`.
  - `Trip` now ends on the left hip and must be followed by **`GetUp_Side_Right`**, not `GetUp_Front`.
  - `Stumble_Short` is cut in at `Poke` frame 6.
- **P2C and P3C:** same skeleton, bind pose, mesh and UVs as pass 1 (bindcheck: rest difference 0.0). An animation-only re-import is safe.
- **P4D is a new skeleton asset:** 32 bones, `root` on top, Meshy `ue5_mannequin` names, centimetres, 1.80 m, bind pose from Rodin's TAPose. It does not share bones with P4B or P4C.
- **Base colour** is still referenced by path (`export2/<fig>/T_Skater_<fig>_BaseColor.png`). If the importer copies the FBX elsewhere, it must copy the PNG with it.
- **Grip rotation:** still not needed (the stick rides the animated `stick_socket` bone). Physics: A's capsules (the P4D head radius is now 0.171 m).

### Provenance (new files)

| File | Origin | Tool |
|---|---|---|
| `p4d-sheet-1.png`, `p4d-sheet-2.png` and their crops | ai-generated | Codex image generation |
| P4D mesh and texture | ai-generated | Hyper3D Rodin `ca2526c2-a9c3-494e-a2fb-c29f0dde9609` |
| P4D skeleton and weights | ai-generated, modified | Meshy rig `01a1d16a-4f72-748c-9bab-f5d2dbf213fe`; root, sockets and boot re-weight by `retarget.py` |
| All 72 takes | original | SKF fifth pass (`skf/export5`), retargeted by `skc/retarget.py` |
| Prop rule, start match, `huekey`, `headratio` | original | our scripts |

### Rebuild

- No API calls, about 15 min: `bash C:/Users/kazda/jam/skater-lab/skc/run_all2.sh`.
- One figure: `bash skc/build2.sh 2_<fig>`, then `bash skc/sheets2.sh <fig> <before fbx>`.
- The pass-1 script is kept as `skc/retarget_pass1.py.bak`. The new keys (`prop_takes`, `prop_z`, `prop_blend`, `prop_fixed_anchor`, `match_start`, `match_frames`, `trip_side`) are off unless set, so `cfg/P2C.json` and the other pass-1 configs rebuild pass 1.

### Sheets (`devlog/shots/20261112-2142xx-B-lab-skc2-*`)

| Sheet | What it shows |
|---|---|
| `p4-ab-bcd` | P4B, P4C, P4D close up and at 40 px |
| `p2-getup`, `p3-getup` | Before and after rows of `GetUp_Side_Left`, `GetUp_Side_Right` and `Goalie_Block_Left` from the side |
| `p4-getup` | The same rows, P4C pass 1 against P4D |
| `p2-lift-stumble`, `p3-lift-stumble`, `p4-lift-stumble` | `Lift` and `Stumble_Short` from the side and from the game camera at 40 px |
| `p4d-concept`, `p4d-turntable` | P4D's concept sheet, and its Rodin mesh as generated (8 views) |

### What I could not verify (second pass)

- **Anything in Unreal.**
- **P4D's `Board_Hit` pop:** not diagnosed. It looks like P3A's old ice-tilt branch switch.
- **P4D at 40 px in orange:** not rendered (the hue-keyed texture is in `skc/tex/P4D2/orange.png`).
- **Look checks:** no eyes or Gemini verdict on P4D. The look verdict is mine, from the sheets.
- **Unchanged from pass 1:** P3C's 5.4 cm blade-contact error on standing takes. It was the same in pass 1; it comes from the p3 child grip, not from this pass.
