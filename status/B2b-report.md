# B2b report: dressing pass 2, generated set props (device B)

Lane B2b, device B, 2026-11-12 about 11:45 to 11:55 by the machine clock. No git command was run. Everything went through `node tools/mcp/mcp.mjs` against the lead's editor on port 8000
(`execute_python`, `StartPIE`, `StopPIE`, `CaptureGameViewport`). At the end the editor is open on `/Game/Maps/Rink`, `get_dirty_map_packages` and `get_dirty_content_packages` are both
empty, `Rink` was never saved (my helper camera actor was put in it in memory and removed again; it holds its own 4 actors again), nothing is under `Content/_Scratch`.
Verified in PIE on `/Game/Maps/Rink` with device A's real rink, nets, puck, toy skaters and HUD, the night rig streamed in by itself.

## First: M_Ice blue line

`/JamAssets/Look/M_Ice`: the two Custom nodes that held `RinkHalfLength * 0.29` (the colour node and the roughness node) now read a new scalar parameter `BlueLineFrac`, default **0.3333**,
as an extra Custom input; recompiled with no error expressions, saved. PIE still `b2b-ice-blueline` (top-down, camera at x = 20 m, z = 25 m, pitch -89): the painted blue line is at the image
centre, i.e. at 20 m, which is 2.6 m further from centre than the old 17.4 m (0.29 x 60 m). I did not shoot a "before" still; the 2.6 m follows from the two constants. Ice parameter on the instance `MI_Ice_Night` is untouched, so `RinkHalfLength` there still gives 20 m as long as it is 6000.

## Import route that worked (first try, no Blender)

Interchange through the plain asset-import task, per prop, in the editor Python:

```python
t = unreal.AssetImportTask()
t.set_editor_property('filename', 'C:/Users/kazda/jam/skater-lab/s3/props/<folder>/base_basic_pbr.glb')
t.set_editor_property('destination_path', '/JamAssets/Arena/Imp_<Name>')
t.set_editor_property('automated', True); t.set_editor_property('replace_existing', True); t.set_editor_property('save', False)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
```

It gave per GLB: `StaticMeshes/base_basic_pbr`, `Materials/base_basic_pbr_material_0`, `Textures/diffuse`, `Textures/normal`, `Textures/orm`. Then `EditorAssetLibrary.rename_asset`
`.../Textures/diffuse` to `/JamAssets/Arena/T_<Name>_BaseColor` and `.../StaticMeshes/base_basic_pbr` to `/JamAssets/Arena/SM_<Name>`, and `delete_directory` on the `Imp_<Name>` folder
(normal, orm and the imported material went with it). The Rodin diffuse is 2048; to keep the files small I exported each to PNG (`unreal.Exporter.run_asset_export_task`), downsized to 1024 x 1024 with
Pillow, and re-imported over `T_<Name>_BaseColor` (`destination_path='/JamAssets/Arena'`, `destination_name`), then `max_texture_size = 1024`, sRGB on, default compression. Imported meshes: Nanite on by Interchange default,
pivot at the bounds centre (so z = scale x (half height - origin z) puts them on the floor), unit-scale (1 uu = 1 cm), one material slot each.

## Material

`/JamAssets/Arena/M_PropCel` (Default Lit, opaque, two-sided): `BaseColorTex` (TextureSampleParameter2D) to Base Color; Roughness 0.6, Specular 0.25; same Custom rim node as `M_Toon`
(`F * RimStrength * RimColour + Emissive * Tex`, `RimPower` 3, `RimStrength` 0.35, `RimColour` 0.55,0.7,1.0) with **Emissive 1.5** as flat fill. One instance per prop `MI_<Name>` with its texture; the mesh's slot 0 points at it. Compiled clean.

## Per prop

| Prop | PASS | Scale used | Nanite triangles (after) | Texture on disk | Mesh on disk | Where placed |
|---|---|---|---:|---:|---:|---|
| `SM_FilmLight` (spotlight, mid-grey) | PASS | 2.15 (4 m tall) | 5,038 | 480,793 | 402,960 | 8, at the pass 1 stand spots (+-30, +-40) and (+-62, +-40) m, lens aimed at the ice, shadows on |
| `SM_DirectorChair` | PASS | 0.686 (1.2 m) | 4,912 | 629,243 | 370,947 | 1 at (0, -41) m, 4 more at (+-7.5 and +-16.5, -38.3) = two beside each bench, facing the ice, shadows on |
| `SM_Slate` (clapper2) | PASS | 1.676 (3 m) and 0.559 (1 m) | 5,038 | 677,632 | 355,746 | big one at (10, -39.5) m, small one at (2.3, -41) beside the chair |
| `SM_CameraDolly` | PASS | 1.156 (2 m) | 4,912 | 778,820 | 403,300 | 2 on the track (x = -28 and +32, y = 44) and 1 behind each net (+-75, +-6) |
| `SM_CameraCrane` | PASS | 4.86 (7 m longest axis, 8.65 m tall) | 5,040 | 967,736 | 438,275 | (77.5, -14) replaces the pass 1 crane; new one at (-77.5, +14); arm along y, inside the |x| >= 72 line |
| `SM_FlightCases` | PASS | 0.70 (1.2 m stack, box 1.25 m) | 4,914 | 829,724 | 405,880 | 12 on the spots of 12 pass 1 stacked cube pairs (24 cubes removed, about a third of the 75 case stacks' cubes) |
| `SM_TrussLamps` (truss2) | PASS, marginal | 3.23, rolled 90 degrees to lie horizontal, 6 m long | 4,912 | 774,594 | 377,834 | one above each end wall at (+-76, 0), z 20 m |

Bounds read back after import (cm, x,y,z): FilmLight 125,82,186; DirectorChair 113,87,175; Slate 47,120,179; CameraDolly 69,169,173; CameraCrane 88,144,178; FlightCases 179,181,171 (the box is as wide as Rodin said, the object reads smaller inside it: the stack at 0.70 is about 1.25 m wide in the still, fine);
TrussLamps 113,88,186. Source triangles 9.3k to 9.9k as in `status/B-skater-s3.md`. Front axes found by test: light front = mesh +Y (yaw offset -90), chair front = mesh -Y (+90), slate faces along its short axis.

Truss: it reads as a pale lattice with two orange lamp shades above the far end wall in the own-end still, but it is small and the lamps point sideways (the model hangs them down when upright). Kept, as the brief allows, but it is the weakest piece; remove `Dress/Gen/TrussLamps` if the lead prefers.

## Rules checked

- Stencil 1 (`render_custom_depth` on, value 1) on every new actor, `NO_COLLISION`, static, `cast_shadow` off except film lights and chairs.
- Footprint: read back for all 35 new actors: none overlaps |x| < 72 m and |y| < 37 m below 25 m height (the trusses are at z 20 m, over the end wall, outside the footprint in x). Nothing in any still covers the ice, the far net or its beacon.
- **Nanite on**, and it does not break custom depth or the outline: the black cel outline and the stencil ramp are present on every prop in the stills (`b2b-close-*`), so Nanite stays on. To meet the triangle budget I set the Nanite build setting `keep_percent_triangles` to 0.5 on all seven meshes (about 4.9k to 5.0k each, read with `get_num_nanite_triangles`); at 25 m or more from the camera nothing is lost in the stills. The non-Nanite fallback meshes are 0.7k to 2.4k triangles.
- **Triangles of the whole dressing: 188,742** (35 props about 173k Nanite, rest basic shapes: 121 cubes x 12 is counted as 12 triangles each, cylinders 512, cone 284, planes 2; the room cubes are in the sum). Pass 1 was 48.8k; budget 250k. Without the 0.5 reduction it would have been about 335k.
- Frame time: not measured this time (two editors up, numbers would be noise). Nanite with 35 instances of seven meshes is expected to be cheap, but there is no measurement.

## What I removed (pass 1 actors)

144 actors: the 80 film-light actors (8 stands x 10), the 13 chair actors, the 14 slate actors, the 13 crane actors, and 24 case cubes (12 stacked pairs). Kept from pass 1: the truss (22), the dolly track and its cart (38), cables (10), benches (8), exit signs (2), the remaining 65 case cubes.

## Rig map `LR_L2_Night.umap`: saved and read back (loaded fresh from disk, opened directly as the edited level)

192 actors. By label prefix: `PPV_L2_Night` 1, `Sky_L2_Night` 1, `Spot_L2_Night_1..4` 4, `Room_L2_Night_1..6` 6, `Dress_L2_Night_*` 180 (numbering has gaps: pass 1 numbers that were removed are not reused; mine are `Dress_L2_Night_290` to `Dress_L2_Night_324`).
My 35 sit in the outliner folder `Dress/Gen/<Name>`: FilmLight 8, DirectorChair 5, Slate 2, CameraDolly 4, CameraCrane 2, FlightCases 12, TrussLamps 2.

## Textures: letters, numbers, logo-like marks

I looked at each 2048 base-colour image at full size. All seven are UV-atlas patchwork of flat colour islands (grey, orange, yellow, black, a few teal discs on the dolly) with lattice lines, tyre rings and hinge discs: **no letters, numbers or logo-like marks** in any. The slate has black and white diagonal stripes on the clap sticks only; the board is blank grey. No prop was dropped.

## Files created or changed (bytes on disk), `Game/Plugins/JamAssets/Content/`

| File | Bytes |
|---|---:|
| `Arena/M_PropCel.uasset` | 16,871 |
| `Arena/MI_CameraCrane.uasset` | 10,965 |
| `Arena/MI_CameraDolly.uasset` | 11,053 |
| `Arena/MI_DirectorChair.uasset` | 9,360 |
| `Arena/MI_FilmLight.uasset` | 9,454 |
| `Arena/MI_FlightCases.uasset` | 9,361 |
| `Arena/MI_Slate.uasset` | 11,241 |
| `Arena/MI_TrussLamps.uasset` | 9,874 |
| `Arena/SM_CameraCrane.uasset` | 438,275 |
| `Arena/SM_CameraDolly.uasset` | 403,300 |
| `Arena/SM_DirectorChair.uasset` | 370,947 |
| `Arena/SM_FilmLight.uasset` | 402,960 |
| `Arena/SM_FlightCases.uasset` | 405,880 |
| `Arena/SM_Slate.uasset` | 355,746 |
| `Arena/SM_TrussLamps.uasset` | 377,834 |
| `Arena/T_CameraCrane_BaseColor.uasset` | 967,736 |
| `Arena/T_CameraDolly_BaseColor.uasset` | 778,820 |
| `Arena/T_DirectorChair_BaseColor.uasset` | 629,243 |
| `Arena/T_FilmLight_BaseColor.uasset` | 480,793 |
| `Arena/T_FlightCases_BaseColor.uasset` | 829,724 |
| `Arena/T_Slate_BaseColor.uasset` | 677,632 |
| `Arena/T_TrussLamps_BaseColor.uasset` | 774,594 |
| `Look/M_Ice.uasset` (changed, `BlueLineFrac`) | 24,554 |
| `Rigs/LR_L2_Night.umap` (changed; replaces B2's 656,481 row) | 408,404 |

Total size of `Arena/`: **8,033,788 bytes** (pass 1 materials included). Props per prop on disk (mesh + texture): FilmLight 883,753; DirectorChair 1,000,190; Slate 1,033,378; CameraDolly 1,182,120; CameraCrane 1,406,011; FlightCases 1,235,604; TrussLamps 1,152,428. Under `devlog/shots/`: the stills below.

## Ledger rows (paste into `ASSETS.md`; the umap row REPLACES B2's row of the same path; the M_Ice row, if one exists, only changes its size)

```
| Game/Plugins/JamAssets/Content/Arena/SM_FilmLight.uasset | mesh | 0.4 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | Rodin task 26bba5b6-541f-4c75-8084-54003c8a41eb (status/B-skater-s3.md), spotlight on tripod, Nanite keep 50% (lane B2b) |
| Game/Plugins/JamAssets/Content/Arena/SM_DirectorChair.uasset | mesh | 0.4 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | Rodin task b439b854-b5f8-4514-8463-eece35718ed0 (lane B2b) |
| Game/Plugins/JamAssets/Content/Arena/SM_Slate.uasset | mesh | 0.4 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | Rodin task e453e553-94aa-40d9-a379-1459a919b6a0 (clapper2; first try f5ced869 rejected, not shipped) (lane B2b) |
| Game/Plugins/JamAssets/Content/Arena/SM_CameraDolly.uasset | mesh | 0.4 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | Rodin task 1462aa6c-7e46-4089-9dbc-9f6c16a1a219 (lane B2b) |
| Game/Plugins/JamAssets/Content/Arena/SM_CameraCrane.uasset | mesh | 0.4 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | Rodin task e9a845c9-c386-4d72-b8f1-96249e6685a9 (lane B2b) |
| Game/Plugins/JamAssets/Content/Arena/SM_FlightCases.uasset | mesh | 0.4 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | Rodin task be52e7de-3e16-4984-ae1b-24929550b31e (lane B2b) |
| Game/Plugins/JamAssets/Content/Arena/SM_TrussLamps.uasset | mesh | 0.4 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | Rodin task ef087401-b9a3-4293-a769-3bd11edab124 (truss2; first try 96e87959 rejected, not shipped) (lane B2b) |
| Game/Plugins/JamAssets/Content/Arena/T_FilmLight_BaseColor.uasset | texture | 0.5 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | 1K base colour from Rodin task 26bba5b6-541f-4c75-8084-54003c8a41eb, downsized from 2K, no text (lane B2b) |
| Game/Plugins/JamAssets/Content/Arena/T_DirectorChair_BaseColor.uasset | texture | 0.6 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | 1K base colour from Rodin task b439b854-b5f8-4514-8463-eece35718ed0, no text (lane B2b) |
| Game/Plugins/JamAssets/Content/Arena/T_Slate_BaseColor.uasset | texture | 0.7 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | 1K base colour from Rodin task e453e553-94aa-40d9-a379-1459a919b6a0, stripes only, no text (lane B2b) |
| Game/Plugins/JamAssets/Content/Arena/T_CameraDolly_BaseColor.uasset | texture | 0.8 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | 1K base colour from Rodin task 1462aa6c-7e46-4089-9dbc-9f6c16a1a219, no text (lane B2b) |
| Game/Plugins/JamAssets/Content/Arena/T_CameraCrane_BaseColor.uasset | texture | 1.0 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | 1K base colour from Rodin task e9a845c9-c386-4d72-b8f1-96249e6685a9, no text (lane B2b) |
| Game/Plugins/JamAssets/Content/Arena/T_FlightCases_BaseColor.uasset | texture | 0.8 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | 1K base colour from Rodin task be52e7de-3e16-4984-ae1b-24929550b31e, no text (lane B2b) |
| Game/Plugins/JamAssets/Content/Arena/T_TrussLamps_BaseColor.uasset | texture | 0.8 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | 1K base colour from Rodin task ef087401-b9a3-4293-a769-3bd11edab124, no text (lane B2b) |
| Game/Plugins/JamAssets/Content/Arena/M_PropCel.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | prop master: BaseColorTex, Emissive 1.5, rim as M_Toon (lane B2b) |
| Game/Plugins/JamAssets/Content/Arena/MI_FilmLight.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | M_PropCel instance (lane B2b) |
| Game/Plugins/JamAssets/Content/Arena/MI_DirectorChair.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | M_PropCel instance (lane B2b) |
| Game/Plugins/JamAssets/Content/Arena/MI_Slate.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | M_PropCel instance (lane B2b) |
| Game/Plugins/JamAssets/Content/Arena/MI_CameraDolly.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | M_PropCel instance (lane B2b) |
| Game/Plugins/JamAssets/Content/Arena/MI_CameraCrane.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | M_PropCel instance (lane B2b) |
| Game/Plugins/JamAssets/Content/Arena/MI_FlightCases.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | M_PropCel instance (lane B2b) |
| Game/Plugins/JamAssets/Content/Arena/MI_TrussLamps.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | M_PropCel instance (lane B2b) |
| Game/Plugins/JamAssets/Content/Rigs/LR_L2_Night.umap | other | 0.4 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | night rig: B1's PPV, sky, 4 spots, dark room, pass 1 dressing and 35 generated-prop actors (lanes B2, B2b); replaces the B2 row |
```

## Stills filed (`devlog/shots/`, PNG about 1920 x 1085, caption `.txt` beside each)

- `20261112-115044-B-b2b-own-end.png`, `20261112-115045-B-b2b-faceoff.png`, `20261112-115045-B-b2b-far-end.png` (a free `CameraActor`, FOV 70, 3.5 m high, pitch -6, at x = -51, -9 and +39 m on the long axis, y = 0)
- `20261112-115046-B-b2b-close-chairs-slate.png`, `20261112-115046-B-b2b-close-light-dolly.png`, `20261112-115046-B-b2b-close-crane.png`
- `20261112-115047-B-b2b-ice-blueline.png` (the M_Ice check)

The helper camera was a `CameraActor` placed in `Rink` in memory before PIE (the PIE world has no Python spawn), set as view target per shot; it was removed and `Rink` reloaded discarding changes.

## What does not read from the game camera

- Chairs, slates, the dolly track and its three dollies, the flight cases and the film lights at the long-side walls are 37 to 46 m off the long axis behind the 1 m boards: in the three long-axis views only the far crane, the lights near the corners, the small slate at far left and the truss are visible; the rest shows only in the close stills or from a camera on the long side.
- The flight cases, dolly and chairs are lit only by the fill emissive (1.5) and the sky: from the game camera they are dark silhouettes with a thin outline. The slate, crane and the chair canvas read best (pale grey and orange).
- The truss over each end wall is small, its lamps point sideways, and the scoreboard HUD overlaps it at the top of the faceoff view. The ice, the far net and the beacon are clear in all three views.
- Not done or not measured: frame cost, a view from the real in-game follow camera (I used a free camera at the same height), the 90 x 45 and 140 x 70 cuts.
