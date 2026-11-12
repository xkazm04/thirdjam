# Skater lab SKT: felt, stitches and clay maps for P3C (device B), report at 23:28

Lane ran 22:50 to 23:28 (machine clock). Blender 4.2 headless and Python (numpy, scipy, Pillow). No Unreal, no git, no paid calls. Everything is in `C:/Users/kazda/jam/skater-lab/skt/`; nothing outside it was modified (P3C's FBX files and PNG are unchanged, mtimes 21:39 / 21:06).

## Short answer
- **Three new maps on P3C's existing UVs**, at 2048 and 1024: base colour v2 (felt and cream stitches), a tangent-space normal map (felt fibres, raised stitches, clay thumb dents and fingerprints), and a part mask (R team colour, G cream, B clay, A felt). The mesh, UVs, skeleton and weights are untouched: only images are new.
- **Close up and at 2 to 3 m** the figure now has the concept's two materials: a fuzzy, mottled felt jersey with thick cream running stitches at the hem, collar, cuffs, side seams, shoulders and sock tops, cream felt bands on the upper sleeves and socks, and a clay helmet with soft thumb dents and visible fingerprint whorls.
- **At the game camera** (70 px figure from behind), the stitches read as a faint dotted cream line under the collar and across the lower back. They do not read as separate dashes. **At 40 px and at 8 m** they do not read; only the cream trim bands do.
- **The engine's hue key still works unchanged** on the new base colour: every team-cloth texel keys fully, and no cream texel keys at all.

## Outputs (`skt/out/`)

| File | Size | Holds |
|---|---|---|
| `T_SkaterGen_P3C_BaseColor_v2.png` | 2048, RGB, sRGB | The original colours, with felt mottling and fibres on the cloth, Rodin's thin old stitches painted out, the white trim turned warm cream, the cream bands, and the cream running stitches with a small shadow below each |
| `T_SkaterGen_P3C_Normal.png` | 2048, RGB, linear | Tangent space, **DirectX convention (green = down in the image, +V in Unreal)**, as Unreal expects: import with the default "Normalmap" compression and leave *Flip Green Channel* off. Blender needs the green channel inverted (`render.py` does this). |
| `T_SkaterGen_P3C_Mask.png` | 2048, RGBA, linear | **R** = takes the team colour (blue jersey, sleeves, socks, and the blue clay helmet; 0 under the stitches and bands). **G** = cream that must never be recoloured (stitches, sleeve and sock bands, cream trim at the collar, cuffs and hem). **B** = clay (helmet, face, cage, mittens, skates). **A** = felt amount (1 on the jersey, sleeves, pants, socks and trim). |
| `*_1024.png` for all three | 1024 | The game size. Base colour is box-averaged in linear space; normals are averaged then renormalised; the mask is averaged. Stitches stay whole at 1024 (sheet `lab-skt-1024`). |
| `stats.json` | | Hue-key check numbers (below) |

The gutters between UV islands are filled with the nearest island texel in every map, so mips do not bleed.

## How each map was derived (all from the mesh and the original texture; nothing hand-painted)
1. **`extract.py`** (Blender) reads `SK_Skater_P3C.fbx`. It dumps the bind-pose positions (cm), normals, the UVs and triangles (21,110 vertices, 37,404 triangles, one UV set), the 24 vertex groups and the bone heads.
2. **`raster.py`** rasterises the UV triangles at 2048 with numpy. It stores, for each texel, its 3D point, normal, bone weights and the world size of one texel step.
   - 65.2 % of the atlas is covered.
   - The median texel is 1.27 mm on the figure (5th to 95th percentile 0.93 to 1.87 mm).
3. **`parts.py`** builds the part map.
   - **Body region:** the largest bone-weight sum per texel. The spine and clavicle bones give the torso; pelvis is its own region; upper arm and forearm give the sleeves; hand gives the mittens; thigh, calf and foot give the legs; head gives the head.
   - **Colour class** of the original texel, in sRGB:
     - blue: b − max(r,g) > 0.18;
     - navy: 0.04 to 0.18, with value under 0.45;
     - light: saturation under 0.25 and value over 0.55;
     - dark: value under 0.22;
     - warm: hue under 60 degrees.
   - **Rules, region × colour:** blue on the torso is jersey; blue on an arm is sleeve; navy on the pelvis or thigh is pants; blue on the calf is sock; blue above 47 cm on a thigh is the jersey skirt; dark on the calf or foot is skate; light near the ankle is skate; and so on.
   - A 5×5 majority filter cleans the result.
   - **Two leaks fixed after the flat turntable:**
     - The hips first came out as "sock", because the jersey skirt is thigh-weighted. A 47 cm height split fixes it.
     - **The collar against the helmet (the known confusion):** the collar is head-weighted on this rig, so it first went to the helmet. Fix: blue below 108.5 cm and within 26 cm of the neck axis is jersey, and light below 112 cm within 22 cm is collar trim. This was measured: the helmet rim is about 30 cm out from the neck axis.
   - Sheet `lab-skt-parts` shows the result.
4. **`build_maps.py`** makes all three maps.
   - **Seams are found on the 3D surface, through the texels of every UV island at once:**
     - **hem:** jersey texels 3.5 cm from the hem trim, found with a KD-tree in 3D;
     - **collar:** 3.5 cm from the collar trim;
     - **cuffs:** sleeve texels 3.5 cm from the cuff trim;
     - **sock tops:** sock texels 2.0 cm from the pants;
     - **side seams:** the torso's front/back mid-plane on side-facing jersey, from 10 cm below the shoulder joint down to just above the hem stitches;
     - **shoulder seams:** the shoulder's mid-plane on top-facing jersey and sleeve, from past the collar stitches out to the shoulder joint.
   - **From texels to polylines:**
     - The iso-texels are binned by angle around a bone axis (rings: spine, neck, forearm, calf) or by height or width (open seams).
     - The per-bin median 3D point gives the polyline.
     - It is despiked against an 11-bin moving median, then smoothed.
   - **Stripe bands:** 3.6 cm wide at 62 % along the upper-arm bone, and 2.8 cm wide in the middle of the longest blue run on each sock.
   - **Running stitches:**
     - Dashes are placed once along each polyline in world centimetres. A closed ring gets a whole number of periods, so there is no short dash at the join.
     - Each dash is jittered from seed 7: length ±12 %, width ±8 %, angle σ 7 degrees, position σ 1.2 mm.
     - **Painting:** every covered texel whose 3D point falls inside a dash's rounded bar is painted cream. The texel must face the same way (normal dot over 0.3) and lie within 1.2 cm of the surface.
     - Because the test is in 3D, a dash that crosses a UV island border is painted on both islands as one dash. It does not double, and its size in centimetres is the same everywhere, whatever the UV scale.
     - **Height:** a 1.6 mm dome.
     - **Shadow:** the dash shifted 2.8 mm down the surface, darkening the base colour by 42 %.
   - **Felt base colour (cloth only):**
     - The colour is multiplied by a luminance factor, so hue and saturation are kept.
     - The factor comes from 3D value noise at 16, 6 and 2.5 mm (±9 %), dense short UV fibres (±6 %), and sparse lighter fibres (+22 %, about 9,000 strokes 10 to 26 px long).
     - There is no pattern larger than about 2 cm.
   - **Felt normal:** about 260,000 short random fibre strokes (5 to 13 px, curled), plus fuzzy 3D noise at 1.5 to 3.5 mm and a soft 12 mm undulation.
   - **Clay normal (clay parts):**
     - About 170 soft elliptical thumb dents, 1.6 to 3.6 cm across and 1.0 to 2.8 mm deep, at random points of the clay surface.
     - On the helmet, 80 % of the dents also carry fingerprint ridges: 4.2 mm spacing, 0.22 mm high, slightly whorled.
     - A faint 2 cm undulation is added.
     - The clay keeps its base colour, apart from a ±3 % 4 cm value drift.
   - **Height to normal:** the height field (cm) is differentiated in image space and divided by the world size of a texel step in x and y. That gives a normal along +u and +v, which matches the MikkTSpace tangent and bitangent directions; the green channel is written image-down (DirectX).

## Stitch dimensions that read (three iterations, sheets `lab-skt-iter-game` and `lab-skt-iter-close`)

| Pass | Dash length × width, gap | Distance from the trim | Close / 2 m | Game camera, 70 px | 40 px, 8 m |
|---|---|---|---|---|---|
| v1 (the brief's numbers) | 2.5 × 0.8 cm, 1.5 cm | 1.7 cm | Reads, but thin and fussy next to the concept | Lost in the cream hem trim | Nothing |
| v3 | 4.0 × 1.2, 2.4 | 2.4 | Close to the concept's proportion (measured on its foreground player: about 4 cm dashes, 1 to 1.2 cm wide, about 2.5 cm gaps) | Faint broken light line | Nothing |
| **final (v4 = v5)** | **4.0 × 1.4 cm, gap 2.4 cm, cream (0.97, 0.93, 0.82) × 1.04** | **3.5 cm** (sock tops 2.0) | **Reads like the concept:** thick, raised, slightly irregular cream dashes | **A dotted cream line under the collar and across the lower back,** separated from the trim by blue | Not as stitches; the cream bands carry the read |

Moving the line 3.5 cm off the trim helped the game-camera read more than making the dashes bigger. At 70 px one pixel is about 2.4 cm, so a 6.4 cm period cannot show as separate dots. 66 dashes in all (hem 95 cm, collar about 107 cm, cuffs about 58 cm each, sock tops 34 and 44 cm, sides 16 cm each, shoulders 6 to 7 cm each).

## Hue range kept for the engine's key
- **The engine's key** (`M_SkaterTB2`, in gamma space): `mask = saturate((b − max(r,g)) × KeyGain − KeyBias)`. P3C uses KeyGain 8, KeyBias 1.3, so the key starts at b − max(r,g) = 0.163 and is full at 0.288. The original jersey sits at 0.29 to 0.33.
- **The rule I enforced:** on every team texel (Mask.R), **b − max(r,g) ≥ 0.30**. Where the mottling or a stitch shadow pushed below it, blue was raised (411k texels touched, mostly shadow and dark fibre).
- **Measured on the new base colour** (team texels, 1st to 99th percentile):

  | | New | Original |
  |---|---|---|
  | Hue (degrees) | **218.4 to 224.1** | 217.4 to 222.6 |
  | Saturation | 0.78 to 0.96 | |
  | Value | 0.44 to 0.61 | |
  | b − max(r,g) | 0.300 to 0.361 | |

- **What keys:**
  - Key on team texels: mean 1.000, 1st percentile 1.000.
  - **Key on every cream texel (G): 0.000.**
  - Pants and mittens: 1,479 of 392k texels key above 0.1 (original texture: 787). These are boundary texels, and they stay navy in practice.

## How the engine should use the maps
- **Team colour:** gate with **Mask.R** instead of (or multiplied with) the hue key. The hue key also still works on its own (above). Keep `lum / JerseyLum` from the base colour so the felt mottling carries into the team colour. With JerseyLum near 0.19 (the jersey's mean gamma luminance) the mottle survives; with B5's 0.08 it saturates flat.
- **Mask.G** is cream: never recolour it. Optionally add a small rim or emissive lift on it at night so the stitches survive the dark tiers.
- **Mask.R × Mask.B** is the helmet: team-coloured clay, as in the concept's orange helmets.
- **Normal:** `T_SkaterGen_P3C_Normal`, TC_Normalmap, linear, DirectX green (no flip). It replaces SKI2's triplanar fibre bump on cloth. Gate `JerseyFelt` with **Mask.A** instead of the hue key and the 99 cm `FeltMaxZ` cut: A already excludes the helmet.
- **Suggested roughness:**

  | Part | Roughness |
  |---|---|
  | Felt (A) | 0.95, plus SKI2's rim or sheen boosted by A |
  | Cream stitches (G) | 0.85 |
  | Clay (B): helmet, face, mittens | 0.65 to 0.70 |
  | Skates | 0.6 |

- **Textures:** 1024 in game, like SKI2. Base colour BC1 sRGB. Normal BC5. **Mask:** BC3 or uncompressed, linear (sRGB off), because A is used. If BC1 is wanted, drop A: A equals the cloth parts, which is R + G + the pants.

## Sheets (`devlog/shots/20261112-2327xx-B-lab-skt-*`)

| Label | What |
|---|---|
| `parts` | Flat part colours, 8 views, Glide |
| `turntable` | Glide blue, Glide orange (through Mask.R, engine-like tint), Shot_Slap frame 22 blue; 8 views each |
| `close` | Back from behind and 3/4 in blue and orange, both forearms in blue and orange, the front and the helmet |
| `1024` | The 1024 maps close up |
| `game` | Game camera from behind (orthographic, as `skc`'s game mode): 40 px and 70 px figures, Glide and Shot_Slap, both colours, plus the 1024 maps at 70 px |
| `8m` | 8 m behind and above, perspective |
| `iter-game`, `iter-close` | v1, v3 and final, side by side |

**No reference pixels in any sheet.** The concept images were only looked at.

## Judged against the concept, honestly
- **Close up and at 2 m: close to the concept's intent.**
  - The jersey reads as soft wool felt (mottle, stray light fibres, fuzzy relief), and the helmet as thumbed clay with fingerprint whorls.
  - The hem stitches across the back and the sock-top stitches look like the concept's.
  - It does not match the concept's **hairy silhouette fuzz**: felt that thick is a mesh or shell effect, not a texture.
- **Weak spots:**
  - **The front of the jersey keeps Rodin's large cream V-collar patch.** It is classified as trim and turned cream; the concept has a plain collar.
  - **The collar stitch ring is irregular in places.** The jersey top is narrow and tilted on this mesh, so a few dashes sit at odd angles at the sides of the neck.
  - **Shoulder seams are short** (6 to 7 cm). The big helmet sits on the shoulders; they show mainly from above.
  - **The sleeve band can end raggedly** where the part map's sleeve and jersey meet on the upper back (the weights leak onto the torso sides). It was tightened to 12 cm from the arm axis.
  - **The concept's sleeves carry cream bands near the elbow and stitched cuffs.** P3C's sleeve already has Rodin's white cuff and forearm trims, so the arm shows more cream than the concept.
  - **The face keeps Rodin's baked blocky shading;** the original texture has the same blocks.
- **At the game camera (70 px):** a dotted light line under the collar and across the lower back, plus the strong cream trim bands. That is roughly what the concept's background players show. **At 40 px and at 8 m:** no stitch read at all; the figure reads as blue felt with cream bands and a blue helmet.

## Provenance

| File | Origin | Tool |
|---|---|---|
| P3C mesh, UVs, original base colour | ai-generated | Hyper3D Rodin Gen-2.5-Low, task `cb0f0a16-7ce4-4b84-87c7-78027c268ee8` (skeleton and weights: Meshy rig `01a1d14e-aaf9-736c-8ec3-ee298af3bd93`, modified by `skc/retarget.py`) |
| `T_SkaterGen_P3C_BaseColor_v2*`, `_Normal*`, `_Mask*` | **ai-generated, modified** | Derived from the mesh and the Rodin texture by device B's scripts (`skt/extract.py`, `raster.py`, `parts.py`, `build_maps.py`; seed 7) |
| Renders and sheets | original (of ai-generated assets) | Blender 4.2 EEVEE, `skt/render.py`, `sheet.py`, `gamesheet.py` |

## Rebuild
`bash C:/Users/kazda/jam/skater-lab/skt/run_all.sh` does extract, raster, parts, maps, renders and sheets, in about 3 minutes. Intermediate data is in `skt/work/`, renders in `skt/renders/final/`, sheets in `skt/sheets/`. `dbg_seams.py` plots the seam polylines in three projections (`work/seams_dbg.png`).

**Ledger rows when the maps are imported** (fill in the sizes on import):
```
| Game/Plugins/JamAssets/Content/Skaters/T_SkaterGen_P3C_BaseColor_v2.uasset | texture | | ai-generated, modified | Hyper3D Rodin + device B scripts | | | 2026-11-12 | B | n | P3C base colour with felt mottle, cream running stitches and bands; from Rodin cb0f0a16-7ce4-4b84-87c7-78027c268ee8 by skt/build_maps.py (lane SKT) |
| Game/Plugins/JamAssets/Content/Skaters/T_SkaterGen_P3C_Normal.uasset | texture | | ai-generated, modified | Hyper3D Rodin + device B scripts | | | 2026-11-12 | B | n | P3C tangent normal, DirectX green: felt fibres, raised stitches, clay thumb dents and fingerprints (lane SKT) |
| Game/Plugins/JamAssets/Content/Skaters/T_SkaterGen_P3C_Mask.uasset | texture | | ai-generated, modified | Hyper3D Rodin + device B scripts | | | 2026-11-12 | B | n | P3C mask: R team colour, G cream, B clay, A felt; from bone weights x base colour (lane SKT) |
```

## What I could not verify
- **Anything in Unreal:** the material hookup, BC compression of the mask, the normal's look under the toon or clay rig, the engine's depth of field at 8 m, and frame cost. The renders are EEVEE with a simple Principled material and a light sheen as a stand-in for the felt rim.
- **The normal map's tangent basis is matched to MikkTSpace by construction** (gradients along +u/+v scaled by the texel's world size), not checked against Unreal's own tangents. The bitangent is assumed orthogonal to the tangent; on strongly sheared islands the bump direction will be slightly off.
- **The orange tint in the sheets** is my numpy copy of the engine's formula (TeamColour (0.95, 0.30, 0.03), JerseyLum 0.20, saturate), not the engine's output. The helmet and jersey therefore look lighter orange than in the engine.
- **Every one of the 72 takes:** only Glide and Shot_Slap frame 22 were rendered. Stitch stretch under extreme poses was not checked; dashes are sized in the bind pose.
- **Other figures:** none; per the brief I stopped at P3C.
