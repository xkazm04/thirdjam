# SK7 report: the two clay figures as felt-and-stitch toys (device B)

Lane SK7, 2026-11-13, 02:29 to 03:20 machine clock. Tree `C:/Users/kazda/jam/wt/b-a45` (branch `dev-b-a45`, on ac86499), editor port 8000 (pid 16156, mine),
left through `ExitEditor` at 03:13 (`LogExit: Exiting.`, process gone, dirty lists empty just before). No build, no cook, no git write.
`/Game/Maps/Rink` never saved (`Rink.umap` still 00:13, 18,966 bytes); `_Scratch/` absent; `jam.Cut` back to `hockey.regular`, `jam.ScriptHuman` 0 (read back);
the PIE window size (an editor user setting under `Saved/`, 1280 x 720) was 1920 x 1080 during the lane and is set back. Port 8002 never touched.
Scratch `$SK7` = `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/sk7`
(scripts, `img/` every capture, `bk/` the pre-lane `.uasset` copies of every file I touched). Checkpoint: `status/SK7-checkpoint.md`.

## Verdict

| # | Task | Result |
|---|---|---|
| 1 | Before stills | **PASS**. P3C: the real clay match (`generated_b2` x `p3_clay`, seed 1) frozen at the faceoff (t 2.18). TB1_P3: six test figures summoned at the match figures' places in that same frozen match, because this tree's game mode gives `humanoid_b1` the night `BP_SkaterVisual_TB1` on every tier |
| 2 | P3C normal map | **PASS, with one honest gap**: the map is on and shades correctly (felt relief, raised stitches, thumb dents in the helmet). SK6's pastel wash did **not** reproduce, so its exact cause is not proven (below) |
| 3 | Felt at both distances | **PARTIAL**. Close up both read as wool felt with fat cream stitches and a clay helmet. At the game camera the stitches now read as separate dashes and the jersey is a deeper, mottled colour; there is still no fibre silhouette, so at 10 to 25 m "felt" comes from colour and stitches only |
| 4 | Team colours on clay | **PASS**: hue 18 to 26 (orange) against 223 to 248 (blue) at 7, 12 and 33 m; cream stitches and bands stay cream on both teams |
| 5 | Contact shadow / reflection | **REPORTED, not fixed** (no figure flag is at fault): the shadow under the skates is an amber patch, not a dark contact; reflections of the figures are faint |
| 6 | After stills, composites | **PASS**: same cameras and same frozen frame; `sk7-clay-figures-before-after` and `sk7-clay-hero-low` filed |
| | Asset check before exit | **PASS**: 645 assets under `/JamAssets/Skaters` loaded, 645 valid, 0 invalid, 0 warnings, no missing dependency, 14 blend spaces full, 14 Blueprints / AnimBPs `BS_UP_TO_DATE` (not recompiled, not saved) |

## 2. The normal map: what I found

- **The map was never wrong.** Checked one by one on SK6's own texture and SK6's own node code, before I changed anything:
  sRGB off, `TC_Normalmap`, no green flip; world-normal buffer continuous over the figure (no tangent break; the material goes local -> tangent -> output, which is exact for a flat map on any tangent basis);
  the relief lands exactly on the painted stitches (same UV set); each dash faces up on its upper side and down on its lower side (raised, so the green channel is right), dents read as dents
  (`devlog/shots/20261113-031356-B-sk7-p3c-normal-check.jpg`).
- **It was far too weak.** Median tilt 1 degree (90th percentile 3 degrees): at strength 1 the on / off difference was inside the frame-to-frame noise (3.8 against 3.1 mean absolute difference). It only showed from strength 4 to 8.
- **SK6's wash** (pastel jersey, grey pants, pale eyes) did not appear at strength 1, 3, 4, 8 or 40, from the back or the front, blue or orange, frozen or in running play. That wash is what the felt sheen does when the felt mask is 1 everywhere, and SK6 changed the felt mask wiring and `FeltRimBoost` (4 to 2) in the same half hour, on the older clay rig. My reading: SK6's still shows that sheen fault, not the normal map. **Not proven**: I could not recreate SK6's state.
- **Fix**: the maps are rebuilt with three times the felt relief, 2.5 times the clay dents and a 4 mm stitch dome (was 1.6 mm), and `MI_SkaterGen_P3C` uses the map at `NormalStrength` 3.5. The fallback (procedural detail normal) was not needed.

## 3. What changed on the figures

**P3C (`BP_SkaterVisual_TB2_P3`), textures rebuilt** (`skater-lab/skt/build_maps_sk7.py`, a copy of SKT's builder with switches; 40 s; outputs in `skt/out_sk7/`):
running stitches 8.0 x 2.6 cm with 4.4 cm gaps (was 4.0 x 1.4, gap 2.4), 31 dashes (was 66), 3.9 cm from the trim; a darker felt band (x 0.55, 2.4 cm) on the jersey beside the hem and cuff trims; relief as above.
**`M_SkaterTB2`**, new group "Felt (SK7)", every default equal to the old output: `SheenWhite` 0.5 and `SheenTint` (1,1,1) (the felt rim's colour, was fixed half white),
`FeltMottle` 0 and `FeltMottleTile` 8 (value-noise mottle on the felt, in pre-skinned space), `ClayBumpScale` 1 (scales the triplanar clay bump).
**`MI_SkaterGen_P3C`**: `NormalTex` = the map, `NormalStrength` 3.5, `JerseyLum` 0.19 -> 0.30 (deeper team colour, the baked mottle and fibres stop clipping), `FeltMottle` 0.7, `FeltMottleTile` 7,
`SheenWhite` 0.15, `SheenTint` (1.0, 0.82, 0.6), `FeltRimBoost` 2 -> 3.5, `ClayBumpScale` 0.4, `RoughClay` 0.70 -> 0.80. The pale lavender wash on the jersey edges of the before still is gone.

**TB1_P3 (`BP_SkaterVisual_TB1_P3`), mesh rebuilt** (`skater-lab/skf`, `SKF_SUFFIX=7c SKF_FUZZ=0 SKF_STITCH_SCALE=2.0`, build_skater + build_clips; `export7c/p3_clay`):
running stitches 8.0 x 2.8 cm (cuffs 5.2 x 1.8, a long flat dash does not sit on the sleeve), 38 dashes (was 71). `compare_anim.py` against export6: same 24 bones and parents, rest matrices and all 72 takes identical (max difference 0.0).
Mesh-only re-import (SK6's script): 8,697 -> 8,254 vertices, 9 slots, 3 sockets and the skeleton kept, vertex colours present, no new asset from the import.
**`M_SkaterTB1Jersey`**, group "Felt (SK7)", all behind `FeltMix` 0: felt amount = `FeltMix` x vertex colour R; the fibre detail normal of `M_SkaterTB2` (`FeltTile`, `FeltBump`); `ClayBumpScale`;
`FeltMottle` / `FeltMottleTile`; `FeltDarken`; a hem band by pre-skinned height (`HemZ0`, `HemZ1`, `HemK`); `SheenWhite`, `SheenTint`.
**Instances**: `_JerseyDeco` FeltMix 1, FeltBump 3, FeltTile 4, ClayBumpScale 0.35, FeltMottle 0.7, FeltDarken 0.62, hem band 86 to 93.5 cm x 0.62 (the cream stitches now sit on a darker band), warm sheen;
`_TrimDeco` cream (0.80, 0.70, 0.50) instead of white (0.9), FeltMix 1; **new `MI_SkaterTB1_P3_PadsFelt`** (instance of `M_SkaterTB1Jersey`, dark navy felt) on the mesh's `Pads` slot, so the breezers are felt, not lumpy black clay.
`MI_SkaterTB1_P3_Pads` is no longer referenced by the mesh; I left the asset in place (no deletion).
Not changed: the helmet, gloves and skates of TB1_P3 (instances of `M_Toon`), any Blueprint, any AnimBP, any take, the stick mesh.

**What I would have wanted in `M_Toon` / `M_Ice` (not mine, not touched):** a `ClayBumpScale`-style control and the vertex colours on TB1_P3's clay parts (the helmet stays crinkled, not softly dented);
and the ice's bulb-reflection term dimmed inside cast shadows (task 5).

**Cost:** pixel shader instructions `M_SkaterTB2` 591 -> 695, `M_SkaterTB1Jersey` 508 -> 652. These masters also serve the night, cel and broadcast figures; they pay the extra instructions too. No frame numbers taken.

## 4. Team colours (measured on the stills, mean of the jersey pixels)

| | P3C | TB1_P3 |
|---|---|---|
| Blue, 7 m (game camera) | hue 240, value 0.37 (before 0.49) | hue 239, value 0.42 (before 0.49) |
| Blue, 12 m / 33 m | hue 248 / 223, value 0.22 / 0.41 | hue 243 / 225, value 0.28 / 0.47 |
| Orange, 12 m / 33 m | hue 18 / 22, value 0.50 / 0.67 | hue 18 / 26, value 0.52 / 0.73 |

The felt is deliberately darker than before (about a quarter on the blue). In the barn's unlit corners a blue figure is now dark navy (value 0.22 at 12 m in one view): still blue, but nearer to the dark pants. If the lead wants it brighter: `JerseyLum` back toward 0.24 on `MI_SkaterGen_P3C`, `FeltDarken` toward 0.8 on `MI_SkaterTB1_P3_JerseyDeco`.
Cream: P3C gates the team colour by the mask (R x (1 - G)), TB1_P3 has the cream on its own slot; neither took team colour in any still.

## 5. Grounding

- Both figures cast shadows (`cast_shadow` true, opaque materials). **The shadow on the ice is an amber-brown patch, not a dark contact**: inside the shadow the key light is gone and what remains is the ice's analytic bulb reflection (lane B22's emissive term), which a figure cannot shadow. It reads as a warm stain under the figure. A fix belongs in `M_Ice` or the rig, not in the figure.
- Reflections of the figures are faint (SSR on scratched ice); nothing on the figures blocks them.
- P3C's lifted skate at the faceoff frame floats a few centimetres with its shadow detached (`sk7-clay-grounding`, left). That is the take's pose, not a material matter.

## What still looks wrong, said plainly

- **TB1_P3 from behind sits in a wide squat**; in the hero frame it reads as sitting on a stool (`sk7-clay-hero-low-tb1p3`). I used P3C for the operator's hero.
- P3C's collar stitches are a few slanted dashes (the seam finder breaks the collar ring into two pieces at this dash size); the front keeps Rodin's cream V patch.
- TB1_P3's stitches are still hard-edged boxes close up; its helmet is crinkled clay rather than softly thumbed.
- The amber shadows (above).

## Against the concepts (`path-p3-t1/t2/t3`) and the estimate (`est-p3-barn`)

- **Fibre silhouette:** the concept's jerseys have a hairy outline and stray fibres; ours have a clean outline. That needs shells or cards (SKF's fuzz variant was judged noise), not a material.
- **Proportions and pose:** the concept has stocky bodies, small helmets, low athletic strides and no faces toward the camera; P3C is a bobble-head with a cage and a smile, TB1_P3 squats.
- **Colour:** the concept's blue is a deep indigo; ours is the cartridge's royal blue (#1D4ED8) darkened by a quarter to a third.
- **Plain backs:** the concept has no numbers or crests; ours carry the game's number and crest.
- **Clay parts:** the concept's helmets have tooled ridges and vents, the mittens and skates are chunky clay; ours are closer on P3C than on TB1_P3.
- **Estimate:** its figures stand on clear mirror reflections with a dark contact, and its felt is hairier than real; we have faint reflections, amber shadows and matte wool.

## Assets created or changed (bytes), all under `Game/Plugins/JamAssets/Content/Skaters/`

| File | Before | After |
|---|---:|---:|
| `T_SkaterGen_P3C_BaseColor_v2.uasset` | 1,161,607 | 1,149,835 |
| `T_SkaterGen_P3C_Normal.uasset` | 1,028,141 | 1,352,804 |
| `T_SkaterGen_P3C_Mask.uasset` | 89,346 | 85,497 |
| `M_SkaterTB2.uasset` | 81,140 | 86,162 |
| `M_SkaterTB1Jersey.uasset` | 61,287 | 73,597 |
| `MI_SkaterGen_P3C.uasset` | 22,997 | 25,527 |
| `MI_SkaterTB1_P3_JerseyDeco.uasset` | 12,511 | 17,636 |
| `MI_SkaterTB1_P3_TrimDeco.uasset` | 13,121 | 15,681 |
| `SK_SkaterTB1_P3.uasset` | 1,990,157 | 1,890,974 |
| `MI_SkaterTB1_P3_PadsFelt.uasset` (new) | | 15,041 |

Nothing else under `Game/` changed (`git status`, `find -newer`). Outside the repo: `skater-lab/skt/build_maps_sk7.py`, `out_sk7/`, `work_sk7/` (new);
`skater-lab/skf/build_skater.py` edited in place (env `SKF_STITCH_SCALE`, default 1 = sixth pass; the file as found is `build_skater.pre_sk7.py`), `export7c/`, `work7c/`, `logs7c/` (new).

## Ledger rows for `ASSETS.md`

New:
```
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB1_P3_PadsFelt.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-13 | B | y | instance of M_SkaterTB1Jersey on the Pads slot of SK_SkaterTB1_P3: dark navy felt breezers (FeltMix 1, FeltBump 3, FeltMottle 0.6, crest and number off) (lane SK7) (15041 bytes) |
```
Replace the existing rows with:
```
| Game/Plugins/JamAssets/Content/Skaters/T_SkaterGen_P3C_BaseColor_v2.uasset | texture | 1.10 | original | Python (numpy, Blender 4.2) lane SKT + UE 5.9 via MCP/Python | | | 2026-11-13 | B | n | skt/out_sk7 base colour at 1024, BC1 sRGB: felt mottling, cream bands, running stitches 8.0 x 2.6 cm, darker felt band at hem and cuffs (lane SK6, rebuilt lane SK7) (1149835 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/T_SkaterGen_P3C_Normal.uasset | texture | 1.29 | original | Python (numpy, Blender 4.2) lane SKT + UE 5.9 via MCP/Python | | | 2026-11-13 | B | n | skt/out_sk7 tangent normal at 1024, TC_Normalmap, DirectX green: felt fibres x3, 4 mm stitch domes, clay thumb dents x2.5; used by MI_SkaterGen_P3C at NormalStrength 3.5 (lane SK6, rebuilt lane SK7) (1352804 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/T_SkaterGen_P3C_Mask.uasset | texture | 0.08 | original | Python (numpy, Blender 4.2) lane SKT + UE 5.9 via MCP/Python | | | 2026-11-13 | B | n | skt/out_sk7 part mask at 1024, linear BC3: R team, G cream, B clay, A felt (lane SK6, rebuilt lane SK7) (85497 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/M_SkaterTB2.uasset | material | 0.08 | original | UE 5.9 editor via MCP/Python | | | 2026-11-13 | B | y | hue-key team recolour; copy of M_Toon; glove key (B6s); WarmFill (B8); number and crest after the hue key (B12); SKT maps behind SKTMaps (SK6); group Felt (SK7): SheenWhite, SheenTint, FeltMottle, FeltMottleTile, ClayBumpScale, defaults = old output (86162 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/M_SkaterTB1Jersey.uasset | material | 0.07 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | M_Toon copy + JerseyDeco (B12) + vertex colour path behind VColMix (SK6) + felt path behind FeltMix (default 0): fibre normal, mottle, FeltDarken, hem band, sheen colour, ClayBumpScale (lane SK7) (73597 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterGen_P3C.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python | | | 2026-11-13 | B | n | instance of M_SkaterTB2: base colour v2 + mask + normal map (NormalStrength 3.5), SKTMaps 1, JerseyLum 0.30, FeltRimBoost 3.5, FeltMottle 0.7, SheenWhite 0.15 warm SheenTint, ClayBumpScale 0.4, RoughClay 0.8 (lanes SKI2, SK6, SK7) (25527 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB1_P3_JerseyDeco.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python | | | 2026-11-13 | B | y | instance of M_SkaterTB1Jersey: crest at Z 113, back number off, RimStrength 1.2, Roughness 1, VColMix 1 (SK6); felt: FeltMix 1, FeltBump 3, FeltMottle 0.7, FeltDarken 0.62, hem band 86 to 93.5 cm, warm sheen (lane SK7) (17636 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB1_P3_TrimDeco.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-13 | B | y | instance of M_SkaterTB1Jersey on the Trim slot: cream felt (0.80, 0.70, 0.50) patch, rolls, bands and stitches; back number via UV0 on the patch, crest off; VColMix 1, FeltMix 1 (lanes SKI3, SK6, SK7) (15681 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/SK_SkaterTB1_P3.uasset | mesh | 1.80 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export7c/p3_clay SK_Skater_p3_clay_loopcut.fbx, mesh only (12.0k tris, 8254 vertices): bands, running stitches 8.0 x 2.8 cm (38), vertex colours Col; own skeleton, sockets kept; Pads slot = MI_SkaterTB1_P3_PadsFelt (lanes SKI3, SK6, SK7) (1890974 bytes) |
```
Two notes on these rows: the existing `MI_SkaterTB1_P3_Pads` row stays (the asset stays, now unreferenced). The three texture rows say "original" as this tree's rows do; lane SKT's own report calls the base colour "ai-generated, modified" (derived from the Rodin texture). The lead should decide which is right for the contest record.

## Stills (`C:/Users/kazda/jam/wt/b-a45/devlog/shots/`, each with a one-line `.txt`)

For the operator:
- `20261113-031355-B-sk7-clay-hero-low.png`: P3C in the real clay match, low camera about 5 m behind the blue centre, both teams in view (beside `est-p3-barn.png`).
- `20261113-031355-B-sk7-clay-figures-before-after.png`: 2 x 3 grid, before above after; low camera / game camera / close-up; each cell P3C left, TB1_P3 right.

The twelve named stills (JPEG, 1920 x 1085), `<fig>` = `p3c`, `tb1p3`:
- `20261113-031353-B-sk7-p3c-low-before.jpg`, `-sk7-p3c-game-before.jpg`, `-sk7-p3c-close-before.jpg`, `-sk7-tb1p3-low-before.jpg`, `-sk7-tb1p3-game-before.jpg`, `20261113-031354-B-sk7-tb1p3-close-before.jpg`
- `20261113-031354-B-sk7-p3c-low-after.jpg`, `-sk7-p3c-game-after.jpg`, `-sk7-p3c-close-after.jpg`, `20261113-031355-B-sk7-tb1p3-low-after.jpg`, `-sk7-tb1p3-game-after.jpg`, `-sk7-tb1p3-close-after.jpg`

Extra: `-sk7-<fig>-close-orange-before/-after.jpg` (the orange figure from the front), `20261113-031356-B-sk7-clay-hero-low-tb1p3.png`, `-sk7-clay-low-b22cam-50mm-p3c.jpg` and `-tb1p3.jpg` (B22's position, 50 degree lens),
`-sk7-clay-figures-close-after.jpg` (both figures, both colours), `-sk7-clay-team-colours-far.jpg`, `-sk7-p3c-normal-check.jpg`, `-sk7-clay-grounding.jpg`, `20261113-031357-B-sk7-night-sanity.jpg`.
Cameras: low = lane B22's (x -17, y -7, z 2.2 m, pitch -10, yaw 25, fov 70); game = the game's own camera at the faceoff, HUD widgets removed; close = 2.8 m, fov 50, focus set on the figure (the tier's 16 m focus would blur it). No reference pixels in any still.

## What I could not verify

- **TB1_P3 in a real match**: this tree's `ResolveVisualClass` never selects it. It was seen only as test figures (summoned, team and number set through the interface, animated, then frozen).
- **The cause of SK6's wash** (above).
- **A cook.** Not run. This lane touched `Skaters/` (a mesh re-import, two master materials, a new instance), so the cook-only check is due before `dev-b-a45` is pushed.
- That the other figures on the two masters (night TB1 and TB2, P2C, P4D, TB2_P4, the `_old` copies) render pixel-identically: true by construction (every new parameter defaults to the old value), and a night match with TB1 and with TB2 looked normal (`sk7-night-sanity`), but nothing was pixel-diffed and the broadcast tier was not opened.
- Motion: shimmer of the stronger normal and the mottle in play (two frames of running play looked clean, no measurement), stitch stretch in the other takes, frame cost.
- The look on device A's GPU.

## For device A

1. Take **ten** files from `dev-b-a45`, together: the nine changed ones and the new `MI_SkaterTB1_P3_PadsFelt.uasset`. `SK_SkaterTB1_P3` references the new instance; without it the breezers fall back to the default material.
2. No interface, Blueprint, AnimBP, take, socket or skeleton changed. `M_SkaterTB2` and `M_SkaterTB1Jersey` gained parameters only (group "Felt (SK7)").
3. `humanoid_b1` x `p3_clay` shows `BP_SkaterVisual_TB1_P3` only if the game mode maps it; on this tree it does not.

## What the lead must decide

1. The blue's depth (section 4): keep, or lift `JerseyLum` / `FeltDarken`.
2. Which figure is the clay hero: P3C reads better from the game camera; TB1_P3's squat is the weak point and is an animation matter.
3. The amber shadows and the weak reflections: a look-lane item in `M_Ice` / the rig.
4. The origin wording of the three P3C texture rows.
5. Run the cook-only check, paste the ledger rows.
