# B2c report: stage light on the real rig, and the atmosphere test (device B)

Lane B2c, device B, 2026-11-12, about 11:55 to 12:35 by the jam clock. No git command was run. I drove the lead's editor (port 8000) through `tools/mcp/mcp.mjs`.

At the end:
- The editor is open on `/Game/Maps/Rink`, with no dirty map or content packages and PIE stopped.
- `Rink` was never saved, and nothing under `/Game/` was written.
- `Game/Plugins/JamAssets/Content/_Scratch` does not exist on disk.

**Every frame number below is unconfirmed, because two editors were up** (port 8002 was rendering on the same GPU). Brightness values are full-range luma (0-255), measured the way lane B1b did: `ffmpeg crop,scale=out_range=pc,format=yuvj444p,signalstats`.

## Part A: applied to the real rig `LR_L2_Night`

### What changed

| Actor / asset | Property | Old | New |
|---|---|---|---|
| `Dress_L2_Night_293` (FilmLight) | location x,y / yaw | 61.79, -39.84 / 57.2 | 79.0, -24.0 / 70 (lens faces yaw 160, towards the ice and the group) |
| `Dress_L2_Night_297` (FilmLight) | location / yaw | 61.79, 39.84 / 122.8 | 79.0, 20.0 / -255 (faces -165) |
| `Dress_L2_Night_294` (FilmLight) | location / yaw | -61.79, 39.84 / -122.8 | -79.0, 24.0 / -110 (faces -20) |
| `Dress_L2_Night_290` (FilmLight) | location / yaw | -61.79, -39.84 / -57.2 | -79.0, -20.0 / -75 (faces 15) |
| `Dress_L2_Night_302` (DirectorChair) | location / yaw | 16.5, -38.3 / 180 | 74.0, 5.5 / 270 (faces the ice) |
| `Dress_L2_Night_299` (DirectorChair) | location / yaw | -16.5, -38.3 / 180 | -74.0, -5.5 / 90 |
| `Dress_L2_Night_307` (CameraDolly) | location / yaw | 75, 6 / 180 | 74.5, -5.0 / 180 (camera towards the ice) |
| `Dress_L2_Night_308` (CameraDolly) | location / yaw | -75, -6 / 0 | -74.5, 5.0 / 0 |
| `Dress_L2_Night_325`, `_326` | **new** big `SM_Slate` x1.678 (3 m), stencil 1, no shadow or collision, folder `Dress/Gen/Slate` | - | (76, 10, 1.52) yaw 190; (-76, -10, 1.52) yaw 10 |
| `Dress_L2_Night_327`..`_334` | **new** lens discs: engine `Cylinder` at scale 0.84 x 0.84 x 0.03, pitch 90, 0.45 m in front of each of the 8 film lights at z 3.3 m. Stencil 0, no shadow or collision, `MI_Trim_FilmLens`, folder `Dress/Lens` | - | one per film light |
| `Practical_L2_Night_1`..`_8` | **new** `SpotLight`, movable, `cast_shadows` **False**, colour (255,200,140), source radius 20 cm, folder `Practical` | - | see the next table |
| `MI_FilmLight` | `Emissive` | 1.5 | 2.5 |
| `MI_DirectorChair` | `Emissive` | 1.5 | 3.0 |
| `MI_Slate` | `Emissive` | 1.5 | 3.0 |
| `MI_CameraDolly` | `Emissive` | 1.5 | 3.0 |
| `MI_CameraCrane` | `Emissive` | 1.5 | 2.5 |
| `MI_FlightCases` | `Emissive` | 1.5 | 3.0 |
| `MI_TrussLamps` | `Emissive` | 1.5 | 2.5 |
| `/JamAssets/Look/MI_Trim_FilmLens` | **new** `M_Trim` instance: `TeamColour` (1.0, 0.72, 0.38), `Emissive` 30 (the beacon is 40) | - | - |

Practical lights, one per film light, all aimed away from the ice:

| Label | Film light | Source (m) | Aim | cd | Attenuation | Nearest board point |
|---|---|---|---|---:|---:|---:|
| `Practical_L2_Night_1` | 290 (-79,-20) | (-79.8, -20, 6.5) | down onto its half of the -x group, at (-75.5, -8, 0) | 6000 | 15 m | 20.5 m |
| `_4` | 293 (79,-24) | (79.8, -24, 6.5) | at (75.5, -9.6, 0) | 6000 | 15 m | 20.5 m |
| `_5` | 294 (-79,24) | (-79.8, 24, 6.5) | at (-75.5, 9.6, 0) | 6000 | 15 m | 20.5 m |
| `_8` | 297 (79,20) | (79.8, 20, 6.5) | at (75.5, 8, 0) | 6000 | 15 m | 20.5 m |
| `_2`, `_3`, `_6`, `_7` | 291, 292, 295, 296 (±29.9, ±39.8) | 1 m on the ice side of the lamp, z 6.5 | down and outwards, at 4 m behind the lamp towards the wall (pitch -52), cone 35/55 deg | 1500 | 9 m | 10.3 m |

No light can reach the boards or the ice: every attenuation radius is shorter than the distance to the nearest board point (last column), and the cones point away from the rink. I did not use light channels. A's `ArenaFloor_UnderIce` is on channel 0 and z-fights the rig floor, so a channel-1-only light would make that floor flicker between lit and unlit.

The end groups are point-symmetric. At the +x end, left to right in the far-end view: film light at y -24, crane at -14 (unchanged), dolly at -5, beacon and net at 0 (kept clear), chair at 5.5, big slate at 10, film light at 20. All pieces sit at |x| 74-79.8 m, inside the room's 82 m walls.

### Rig saved and read back (loaded fresh from disk)

210 actors: `PPV` 1, `Sky` 1, `Spot` 4, `Room` 6, `Dress` 190, `Practical` 8, and nothing else.

The Dress count is 180 plus 2 slates plus 8 lens discs. All 8 practicals read back with `cast_shadows` False.

The footprint check on every Dress and Practical actor flagged only `Dress_L2_Night_5/10/15/20`. Those are pass-1 truss pieces hanging at exactly 25 m over the spots, are not mine, and were untouched.

### Assets (bytes on disk, `Game/Plugins/JamAssets/Content/`)

| File | Before | After |
|---|---:|---:|
| `Rigs/LR_L2_Night.umap` | 408,404 | 451,276 |
| `Look/MI_Trim_FilmLens.uasset` (new) | - | 7,720 |
| `Arena/MI_CameraCrane.uasset` | 10,965 | 11,689 |
| `Arena/MI_CameraDolly.uasset` | 11,053 | 11,777 |
| `Arena/MI_DirectorChair.uasset` | 9,360 | 10,084 |
| `Arena/MI_FilmLight.uasset` | 9,454 | 8,677 |
| `Arena/MI_FlightCases.uasset` | 9,361 | 10,085 |
| `Arena/MI_Slate.uasset` | 11,241 | 11,965 |
| `Arena/MI_TrussLamps.uasset` | 9,874 | 10,598 |

Triangles added: 2 slates x 5,038 (Nanite) plus 8 cylinders x 512, about 14.2k in all. The dressing goes from about 188.7k to about 202.9k, against a 250k budget.

Ledger rows. The umap row replaces B2b's row of the same path; the seven `MI_*` rows only change their sizes, and their size column already reads 0.1 MB, so it stays the same.

```
| Game/Plugins/JamAssets/Content/Look/MI_Trim_FilmLens.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | M_Trim instance, warm lens glow for the film lights, Emissive 30 (lane B2c) |
| Game/Plugins/JamAssets/Content/Rigs/LR_L2_Night.umap | other | 0.5 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | night rig: B1's PPV, sky, 4 spots, dark room, dressing passes 1-2, end set groups, 8 lens discs, 8 shadowless practicals (lanes B2, B2b, B2c); replaces the B2b row |
```

### Ice-brightness check (paired stills, same camera, PIE on `Rink`, before 12:02 / after 12:12)

| View | Crop | Before | After | Change |
|---|---|---:|---:|---:|
| own (x -51, pitch -6) | pool 900x150@500,820 | 216.6 | 216.6 | 0.00 % |
| own | dark band 600x50@100,555 | 54.0 | 54.1 | +0.19 % |
| centre (x -9) | pool | 215.9 | 215.9 | 0.00 % |
| centre | band | 39.6 | 39.6 | 0.00 % |
| far (x 39, +x end) | pool | 203.2 | 203.2 | 0.00 % |
| far | band | 44.9 | 45.1 | +0.45 % |
| farN (x -39, -x end) | pool / band | 214.6 / 44.8 | 214.6 / 45.0 | 0.00 / +0.45 % |
| game-camera far (pitch -17.65) | pool / band | 193.4 / 66.1 | 193.4 / 66.0 | 0.00 / -0.15 % |
| game-camera mid (-15,-8) | pool / band | 126.3 / 87.8 | 126.7 / 87.9 | +0.32 / +0.11 % |
| game-camera -x end | pool / band | 212.6 / 61.2 | 212.6 / 61.4 | 0.00 / +0.33 % |

The largest change is 0.45 %, against a 3 % limit: **PASS**.

The strip of set behind the end boards (1920x130 px above the board line) went from YAVG 16.5 to 22.8 at the +x end and from 16.4 to 21.8 at the -x end. In the side view the brightest set pixel went from 172 to 252, which is the lens.

Frame cost: the GPU profile in the far view shows the `Lights` pass at 0.11 ms before and 0.16 ms after. That is unconfirmed (two editors up) and noisy.

### Stills (`devlog/shots/`, JPEG 1920x1085, each with a caption `.txt`)

- Before/after pairs from identical cameras:
  - `20261112-122652-B-b2c-before-far` / `-after-far`
  - `-before-farN` / `-after-farN` (the -x end)
  - `20261112-122653-B-b2c-before-gfar` / `-after-gfar` (game-camera geometry: 6 m back, 3.5 m up, pitch -17.65, FOV 70)
  - `-before-own` / `-after-own`
  - `-before-side` / `20261112-122654-B-b2c-after-side`
- Close views (after only): `20261112-122654-B-b2c-after-close-end`, `-after-close-light`.

### What still does not read

- **The long sides.** Chairs, flight cases, the dolly track and its dollies at |y| 38-46 m still read only at the frame edges from long-axis cameras. Each long side now has two lit film lights (±30 m), because the corner pair moved to the ends.
- **Small pieces behind the nets.** The dolly is 2 m tall and the chair 1.2 m. From the game camera the chair is mostly behind the board glass, and the dolly is a small grey shape next to the beacon. The slate, crane and two lit lenses carry each group.
- **Floor streaks.** On the lit floor under the end practicals there are horizontal streaks (`after-close-end`). They come from A's `ArenaFloor_UnderIce` z-fighting `Room_L2_Night_1`, both at z -5 cm (B1b decision 3). They are faint from the game camera and clear up close. I did not move the rig floor.
- **The truss lamps** above the end walls are unchanged (small, lamps pointing sideways).
- **Not checked:** the real follow camera in motion (the `game` still is the pawn's own view at one moment), and 90 x 45 or 140 x 70 rinks.

## Part B: plan test 2 on a scratch copy (not applied)

**Fixed script:** `C:\Users\kazda\AppData\Local\Temp\claude\6d8eaecb999746c2\1f5c7b1e-557b-4c70-b752-8370d9dbef0b\scratchpad\b2c\v1_t2_atmosphere_b2c.py`. The header lists every fix. Run it with that folder's `py.mjs` (`node py.mjs v1_t2_atmosphere_b2c.py 600 "ACTION='overlay_build'" "VER='b'"`).

What I changed in the script:
1. **The crash fix** (same as B1b's T1 fix): save the duplicated map, drop the reference, `gc` plus `collect_garbage`, then `load_level`. The duplicate-then-open step ran twice without a crash.
2. **Parameter injection** for the runner.
3. **New `overlay` mode.** The rig copy `/JamAssets/_Scratch/V1T2/L_V1T2` is stripped to the 15 test actors (4 cones, 2 flats, 8 frame cubes, and a second unbound volume `V1_PPV` at priority 10 that holds the haze and lens). It is saved and then **streamed into the real game** in PIE on `Rink` with `LevelStreamingDynamic.load_level_instance` (this works). All cases are switched in one PIE session with the game paused per camera, so the pairs are pixel-identical apart from the element under test.
4. **VER b: a camera-inside fade on the shaft material.** VER a failed the pop line, see below.
5. **New cases** b3, c3, c4, d3 and `g_ship`.

The original throwaway-layout path is kept (with fix 1) but was not run. The scratch folder was deleted (`cleanup`), and nothing is left on disk.

Cameras use the game-camera geometry: 3.5 m up, pitch -17.65, FOV 70:
- `near`: x -51, inside cone 1;
- `side`: (-30, 20), outside every cone;
- `far`: x 39, pitch -6, looking at the +x end.

| Element | Pass lines (plan) | Measured | Verdict | Recommendation, values to ship |
|---|---|---|---|---|
| Light-shaft cones, VER a (script as written) | visible from mid/end; no pop inside/outside a cone; far orange figure U/V shift < 10; `Translucency` <= 0.3 ms on B | Visible: the side cone region is 0 -> 90.6 luma at 0.6. **Pop:** from inside a cone the whole black room behind gets a veil (near background 0 -> 61.8, far 0 -> 66.2; at 1.2 it is 103-110), and outside there is none, so crossing a cone wall jumps about 62 levels. Orange figure: Y/V shift under 1 level. `Translucency` 0.10 -> 0.08-0.17 ms. The material renders `MTP_AFTER_DOF`, so neither the ramp nor the outline touches the cones (plan open question 3 answered). | **FAIL** (pop and wash) | Do not ship VER a. |
| Light-shaft cones, VER b (camera-inside fade, `InsideFade` 0.6, `ConeTan` 0.577) | same | Near (inside cone 1) background 0 -> 6.2; the cone ahead still reads as a cone; side cone 90.6 at 0.6, 52.2 at 0.3. Ice crop unchanged (203.3). The fade runs over 0.6 R to R instead of a step, but I did not shoot a crossing sequence. | **PASS** on the stills; pop crossing not filmed | Ship VER b at `Intensity` **0.3**, `InsideFade` 0.6, `FacingPow` 1.5, `ShaftColour` (1.0, 0.93, 0.80). Shafts are seen only from outside a cone, which is mostly the side and centre views. |
| Distance haze (post, on `V1_PPV`) | pass <= 0.05 ms; far-end black +10..+30; beacon still brightest; near figure unchanged | Script defaults (`HazeDensity` 0.00004): far end +1.0 (`HazeMax` 0.5) and +2.2 (0.9). That is too weak: at 45 m, h is only about 0.05. **`HazeDensity` 0.0003**: +13.7 (`HazeMax` 0.5) and +24.2 (0.8). Beacon 244.2 in every case. Ice and figures unchanged. The pass costs 0.04-0.08 ms in quiet samples, with outliers of 0.69-0.86 ms. It runs even with `HazeOn` 0. | **PASS** on look (0.0003 / 0.5); cost **unverified** (around the 0.05 line) | Ship `HazeDensity` **0.0003**, `HazeMax` **0.5**, `HazeStart` 1500, `HazeColour` (0.22, 0.30, 0.55), `HazeAbsorb` 0.3. Remove the blendable rather than set `HazeOn` 0 when it is off. |
| Painted backdrop flats (x ±81.6 m, 80 x 24 m, steel frame) | flat YAVG 25-70; beacon >= 100 above the flat behind it; reads as a painted flat | `Brightness` 5: wide flat 73.7 (just over). **4.5: 67.8.** 10: 120. Beacon 244 against 36 beside it at 4.5 (+208). It reads as a theatre night sky with a ridge and stars directly behind the end set groups, and the set reads against it. | **PASS** at 4.5 (FAIL at 5 and 10) | Ship `Brightness` **4.5**, or 4.0 if the haze ships too (with haze, the flat reads 75). It is the cheapest and most visible piece (one unlit opaque material, 2 planes and 8 cubes). |
| Film grain, fringe, lens flare (`film_grain_intensity` 0.3, `scene_fringe_intensity` 0.5, `lens_flare_intensity` 0.4, threshold 6) | <= 0.1 ms; taste | `LensFlares` pass 0.055-0.094 ms plus tonemap +0.04-0.11 ms, so about 0.1-0.2 ms. The visible change at these values is close to nil: the figure's luma moves 0.6 and nothing flares at threshold 6. | **FAIL** (cost at or over the line for no visible gain) | Do not ship as tested. If the operator wants grain, try grain alone (it lives in the tonemapper) and judge it on a moving frame. |
| All together, VER a `f_all` | <= +0.5 ms on B's profile | `Translucency` +0.08, haze pass about 0.08, lens about 0.1-0.15: about +0.25-0.3 ms. Looked washed out because of the VER a shafts. | cost **PASS** (unconfirmed); look FAIL (VER a) | Proposed ship set `g_ship` (VER b shafts 0.3, haze 0.0003/0.5, backdrop 4.5): far-end flat 75, beacon 244, ice unchanged. Not profiled separately. |

Part B stills (`devlog/shots/`):
- VER a: `20261112-12265x-B-b2c-t2-far-*` (baseline, shafts, haze strong, backdrop, backdrop bright, all), `-t2-near-b_shafts` (the veil), `-t2-side-b_shafts`, and `-t2-grid-far` (contact sheet of all nine cases).
- VER b: `20261112-123256-B-b2c-t2b-near-shafts-fade`, `-t2b-far-haze-dense`, `-t2b-far-backdrop-45`, `-t2b-far-ship`, `-t2b-side-ship`.

## What I could not verify

- **Every frame cost.** Two editors were up, and per-pass samples spread up to 10x (the haze pass ran 0.04-0.86 ms with identical content). The 4090 numbers need A's FrameBudget.
- **The shaft fade while crossing a cone wall.** I have only static stills inside and outside, not a moving sequence.
- **The order of the haze relative to the ramp and outline.** All three are Before DOF. The haze uses blendable priority 10 on a priority-10 volume, and I did not prove the sort order. The stills show no stepped haze on the ice, but the ice barely changes at these distances.
- **Lens flare at a lower threshold**, and grain on a moving frame.
- **The real follow camera across a whole match.** My game-camera stills use fixed positions with the game-camera height, pitch and FOV.

## Decisions

For the lead:
1. Paste the two ledger rows above and update the seven `MI_*` sizes.
2. The two corner film lights per long side now stand in the end groups. If the long sides look too empty, add two more film lights there. That costs 5k triangles each, and a practical each would take the count to 10, the cap.
3. The floor streaks under the end practicals are A's floor overlap (B1b decision 3): ask A to lower or hide `ArenaFloor_UnderIce` when a rig loads. The alternative on B's side is to raise `Room_L2_Night_1` to z -2 cm; I did not do it.
4. My fixed T2 script should be copied back to `skater-lab/v1/tests/` beside B1b's fixed T1 (I was not allowed to write there).

For the operator (tomorrow, D1):
1. **Backdrop flats:** recommend yes, at `Brightness` 4.5.
2. **Haze:** recommend yes, at 0.0003 / 0.5, if the 4090 confirms the pass at 0.05 ms or less.
3. **Shafts:** only the VER b material at 0.3. VER a should not ship.
4. **Lens flare, fringe and grain as tested:** recommend no.
