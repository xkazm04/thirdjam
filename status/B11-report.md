# B11 report: the night rig re-laid for 90 x 45, and the second prop batch (device B)

Lane B11, device B, 2026-11-12, machine clock 14:40 to 15:10. Worktree `C:/Users/kazda/jam/wt/b-vfx`, branch `lane/b-vfx`. Editor on port 8002 (pid 3916), another editor on port 8000 was up the whole time ("unconfirmed, two editors up" applies to anything frame-related; I took no frame numbers).

## How it was run (read this first)

- **The build failed and was skipped** (lead's ruling). `build.ps1` stopped after 18 s at `JamEffectTypes.h(124): Unable to find ... 'EJamTarget'` (device A's Source, handoff B-to-A-0017). I did not touch `Game/Source`.
- The editor ran on the **old binaries** in the worktree (built 13:42). **No modal "rebuild modules" box appeared**; plain `editor.ps1 -WaitMcp -McpPort 8002` started it, MCP answered after 14 s, the process was `Responding` and the log kept moving. `-ExtraArgs "-unattended"` was not needed.
- Because the old code cannot build the rink, verification is on a **stand-in layout in memory on `Rink`, never saved**: a 90 x 45 m ice plane (`MI_Ice_Night`, stencil 2), four board boxes (1.1 m, `MI_Toon_Board`, stencil 1), two net boxes at x = +-42, a blue and an orange cylinder figure, `ArenaFloor_UnderIce` moved to z = -20 cm in memory, and `LR_L2_Night` streamed into the PIE world at the origin (`LevelStreamingDynamic.load_level_instance`). Stills are from a free camera (the PIE `GameNetworkManager` actor as view target) at 3.5 m, FOV 70. At the end `Rink` was discarded by opening a blank map and the editor was left through `ExitEditor` (it refused once because six `MI_*` packages were dirty; I saved them, then it exited). `git status` shows `Rink.umap` and everything under `Game/Content` untouched. Nothing is under `_Scratch`.
- A trap I hit: after Python imports the editor opens the **content drawer over the viewport**, and `CaptureGameViewport` photographs the drawer. A single click on the viewport closes it (Escape ends PIE).

## Part 1: the arena for 90 x 45

### What moved (rig `LR_L2_Night.umap`, saved and read back from disk)

Saved actor list by label prefix: `PPV` 1, `Sky` 1, `Spot` 4, `Room` 6, `Dress` 202, `Practical` 8, nothing else. `PPV_L2_Night`, `Sky_L2_Night`, `Spot_L2_Night_1..4` untouched. All Dress and Room components have `cast_shadow` False and all 8 practicals `cast_shadows` False (script check: empty list).

| Group | Before | After |
|---|---|---|
| Room (6) | inner faces x +-82, y +-47 | inner faces **x +-60, y +-37**, floor z -5 cm (plane 120 x 74), ceiling 28 m (read back from bounds) |
| Truss | 4 lamp heads (housing, lens, 2 yokes) at x +-45 / +-15, z 25 to 26.5 (on the spots) | long beams x +-50 (was +-80); 4 heads at **x -33.75, -11.25, 11.25, 33.75**, housing centre z 24.1, lens z 23.37 (0.2 m below the spot at 25 m), yokes z 24.95; 8 new hanger cubes tie each yoke to the beam at z 27 |
| Film set, hero group per end (crane, camera dolly aimed at the ice, big slate, director's chair, 2 lit film lights with lens discs) | at x 74 to 80 | at **x 49.6 to 57.8**: crane (+-54, -+8), dolly (+-50, -+6), chair (+-50.5, +-5.5), big slate (+-52, +-9.5), film lights (+-56.5, -+13.5 and +-13.5) aimed at (+-48, 0). Nothing of the group is nearer the long axis than |y| = 3.5 (the goal lights, at |y| 4, are the closest) |
| Goal lights (new) | - | (+49, -4) and (-49, +4), lamp to the ice |
| Long sides | |y| 37 to 46 | benches, chairs, slates, film lights, dolly and its track at **|y| 25.9 to 33.5**: film lights (+-22, +-32.5), dolly track y 29.5 (rails x +-52, was 130 m long, now 104 m), 2 dollies, 3 chairs and 2 slates at y -28.5 to -33.5, penalty box (0, 25.9) |
| Cases (65 cubes + 12 flight cases) and cables (10) | against the old walls | against the new walls: long-wall cases at |y| 35.0 to 36.0, end-wall cases at |x| 58.8 to 59.3; cables at |y| 36.2 and |x| 58 / 57.5 |
| Pass-1 cube benches | 8 cubes at y -38 | **removed** (replaced by the two generated benches) |
| Exit signs, truss lamps (generated) | x +-82 / +-76 | x +-59.7 / +-55 (z 9 and 20 m) |

New extents of everything outside the room (Dress and Practical bounds): x -59.97 to 59.92, y -36.32 to 36.65, z 0 to 27.25.

New Part 2 actors in the rig (10 meshes, 12 cubes, labels `Dress_L2_Night_335` and up): 2 `SM_Bench` at (+-9, -26.5) (backrest to the stand), 2 `SM_CoachStool` at (+-12.5, -27.0), 2 `SM_WaterCrate` at (+-13.8, -26.8), `SM_PenaltyBox` (0, 25.9) yaw 180 (open to the ice), 2 `SM_GoalLight` (above), `SM_ScoreboardHousing` at (0, 0, 10 m), yaw 90. All stencil 1, no shadow, no collision, folder `Dress/Gen/<Name>`. Scoreboard: the housing has two blank dark faces on opposite sides; yaw 90 turns them to the two ends. Its top is at 12.7 m (base z 10), two thin wires (x 0, y +-2.0) run to the truss beams at z 26.9.

### Practical lights (8, all shadowless, count unchanged)

| Label | Lights | Source (m) | Aim | cd | Attenuation | Source to nearest board point |
|---|---|---|---|---:|---:|---:|
| `Practical_L2_Night_4` / `_8` (+x end) | film lights 293 / 297 | (57.3, -13.5 / +13.5, 6.5) | (52, -6.5 / +6.5, 0), 10.9 m | 4000 (was 6000) | **12.5 m** (was 15) | 13.43 m |
| `_1` / `_5` (-x end) | 290 / 294 | (-57.3, -13.5 / +13.5, 6.5) | mirrored | 4000 | 12.5 m | 13.43 m |
| `_2`, `_3` (-y side) | 291, 292 | (-22 / +22, -31.5, 6.5) | the wall base at y -36.5, 8.2 m | 1500 | **9 m** (same) | 10.5 m |
| `_6`, `_7` (+y side) | 295, 296 | (-22 / +22, +31.5, 6.5) | y +36.5 | 1500 | 9 m | 10.5 m |

Every attenuation radius is shorter than the distance to the boards box (|x| <= 45, |y| <= 22.5, z <= 1.1), and the cones point at the group or the wall. Lens discs moved and turned with their film lights.

### Footprint check (script over every Dress, Practical actor bounds; |x| < 47 and |y| < 24.5)

**Violations: 0.** 33 actors are inside the footprint by design and are listed, not counted: the 32 truss pieces (z 19.8 to 27.2: beams, heads, yokes, hangers, 2 scoreboard wires) and `SM_ScoreboardHousing` (z 10 to 12.7, asked for in the brief). Closest approach of any Dress bounds to the boards box: 2.40 m (`SM_PenaltyBox`), then benches 3.42 m, goal lights 3.50 m. Closest practical: 8.29 m. The room's inner faces are 15 m (ends) and 14.5 m (sides) beyond the boards.

### Large cut (not run, lead's ruling), arithmetic instead

Rig and boards scale together by s about the origin (A's rule), so every distance from an actor to the boards is multiplied by s. Smallest distance from any actor's bounds to the boards, at scale 1.0 / 1.333 (large, 120 x 60) / 1.556 (wide, 140 x 70):

| Group | s = 1.0 | s = 1.333 | s = 1.556 |
|---|---:|---:|---:|
| Dress, closest (`SM_PenaltyBox`) | 2.40 m | 3.20 m | 3.73 m |
| Dress, then benches / goal lights | 3.42 / 3.50 m | 4.56 / 4.67 m | 5.32 / 5.45 m |
| Practical, closest (side lights) | 8.29 m | 11.05 m | 12.90 m |
| Room, x walls (boards at 45 s) | 15.0 m | 20.0 m | 23.3 m |
| Room, y walls | 14.5 m | 19.3 m | 22.6 m |

Nothing crosses the boards at either scale **if device A's scaling is in**: not verified (old binaries, no real game, no `SetTier`). If A's scaling is not in, the set stays at 49.6 to 59.9 m, which is inside the large cut's boards at x = 60: it would stand on the ice (end groups at x 50 to 58 against boards at 60) and the room walls at 60 would cut through the end zone. I could not show which case is true today.

### Brightness check (paired stills, identical cameras, stand-in, `signalstats` full range)

| View | Crop (w:h:x:y) | Before | After | Change |
|---|---|---:|---:|---:|
| centre (x -6, pitch -17.65) | pool 600:100:660:420 | 216.76 | 216.70 | -0.03 % |
| centre | dark band 250:80:40:430 | 61.97 | 61.78 | -0.31 % |
| game (x -36, y -3, pitch -17.65) | pool 800:150:300:620 | 191.86 | 191.03 | -0.43 % |
| game | band 500:50:60:350 | 60.67 | 60.41 | -0.43 % |
| sideS | pool 200:100:1700:650 | 174.62 | 173.55 | -0.61 % |
| sideS | band 500:60:600:700 | 202.80 | 202.36 | -0.22 % |
| sideN | pool / band | 176.46 / 198.40 | 175.43 / 197.81 | -0.58 / -0.30 % |
| own (x -39, looks at own end) | ice corner L 380:50:20:1005 | 103.73 | 93.57 | **-9.8 %** |
| own | ice corner R | 98.65 | 91.73 | **-7.0 %** |
| far (x 39, +x end) | ice corner L | 108.73 | 96.03 | **-11.7 %** |
| far | ice corner R | 108.23 | 97.97 | **-9.5 %** |

**Verdict: pools and the dark bands in the four game-style views PASS (largest 0.61 %); the ice at the end boards in the two end views FAILS the 3 % line (-7 to -12 %).** Two repeats of the same after-state differ by 0.01 %, so this is not noise. Isolation (PIE only, hiding groups in the streamed rig, all unconfirmed single shots): with every `Dress_` hidden the corner reads 102.5 (within 1.2 % of before); with only the truss visible 96.0; with only the cases 102.4; with only the hero group 103.0. So the whole drop comes from the **truss**: the lamp heads (emissive lenses) used to hang at x +-45 over the end boards and their light bounced onto the corner ice; they now hang over the pool centres as the brief asks. The pool plateau is saturated (216) so it cannot show the gain. Nothing to fix on my side unless the corner ice should stay as bright: the alternative is a faint emissive fill there, which the lead can order.

The strip of dark ice at the end boards in the end views also reflects the closer set (that is the intent).

### Stills (`devlog/shots/`, PNG 1920 x 1063, caption `.txt` beside each)

Before / after pairs, identical cameras: `b11-before-own` / `b11-after-own`, `-centre`, `-far`, `-game`, `-sideS`; after only: `b11-after-sideN`, `b11-after-bench`, `b11-after-penalty`, `b11-after-scoreboard`. They show: the end set now stands right behind the end boards with the net and its red goal light clear in front of it; the far net is clear of the dressing in `b11-after-game`; the scoreboard housing is above the top edge of the game-camera view (its bracket tips a few pixels into the top of `after-game`), so it does not hide the far net (arithmetic: from x -44, 3.5 m high, the housing at 10 to 12.7 m is 8 to 9 degrees above the horizon and the view's top edge is +4 degrees).

## Part 2: the second prop batch

All twelve GLBs imported by the B2b route (Interchange asset-import task, rename, delete the temporary `Imp_` folder, instance of `M_PropCel`), with one change in the pipeline: **I baked scale and pivot into a copy of each GLB with a small script (`scratchpad/b11/bake.py`) before import**, so the mesh arrives at its final size with the pivot at the base centre (x and z at the bounds centre, y at the bottom) and a 1K or 512 PNG of the base colour as the only texture. Nothing else in the GLB was changed. Rodin source files are untouched.

Texture review (each 2048 base colour opened at full size, plus a 2x crop of the water crate): no letters, digits or logo-like marks in any. **One retouch:** the water crate's bottle-cap UV islands held a blocky white "L" and an orange "+" shape; I painted those four cap discs flat orange in the texture before shrinking. No prop was skipped.

### Arena pieces (`Arena/`, Nanite on, 1K, `MI_*` with Emissive 2.5)

| Prop | Verdict | Baked scale | Size (cm, x y z) | Source tris | Nanite tris | Texture | Where |
|---|---|---:|---|---:|---:|---|---|
| `SM_Bench` | PASS | 2.2 | 413 x 116 x 152 | 9,634 | 4,912 | 1024, 522,888 B | 2, (+-9, -26.5) |
| `SM_PenaltyBox` | PASS | 1.6 | 302 x 200 x 243 | 9,824 | 5,036 | 1024, 657,702 B | (0, 25.9), yaw 180 |
| `SM_GoalLight` | PASS | 2.0 | 101 x 101 x 347 | 9,856 | 5,172 | 1024, 476,156 B | (+49, -4), (-49, +4) |
| `SM_ScoreboardHousing` | PASS | 2.4 | 435 x 267 x 274 | 9,414 | 4,912 | 1024, 609,973 B | (0, 0, 10 m), yaw 90 |
| `SM_WaterCrate` | PASS (retouched) | 0.27 | 51 x 51 x 45 | 9,740 | 3,652 (keep 0.3) | 1024, 652,272 B | 2, (+-13.8, -26.8) |
| `SM_CoachStool` | PASS | 0.57 | 37 x 59 x 100 | 9,556 | 3,778 (keep 0.3) | 1024, 619,595 B | 2, (+-12.5, -27.0) |

Nanite `keep_percent_triangles` is 0.5 except the two small pieces (0.3) and, to stay inside the 250k budget, **`SM_FlightCases` from 0.5 to 0.35 (4,914 to 3,782 each)**. Notes: the goal light is one mesh with one material, so its red lamp reads lit through the flat fill; to keep it dark device A would need an instance with the lamp not emissive, which this mesh cannot split. The scoreboard face is mottled (flat dark grey with blotches), not flat; one face has two bracket straps across it.

### Pickup props (`Props/`, Nanite off, 512 px, `MI_PickupProp_<kind>` = `M_PropCel` instance, Emissive 5.0)

| Asset | Source (task id in the ledger) | Verdict | Baked scale | Size (cm, x y z) | Tris | Texture | Attach |
|---|---|---|---:|---|---:|---|---|
| `SM_Pickup_speed` (film reel) | `reel3` | PASS | 0.302 | 22 x 54 x 55 | 4,726 | 512, 446,609 B | `PickupTop` |
| `SM_Pickup_precision` (camera lens) | `lens` | PASS | 0.300 | 53 x 51 x 55 | 4,836 | 512, 315,852 B | `PickupTop` |
| `SM_Pickup_heavy_puck` (film canister) | `canister3` | PASS | 0.298 | 55 x 55 x 24 | 4,732 | 512, 306,830 B | `PickupTop` |
| `SM_Pickup_big_puck` (spotlight head) | `spot2` | PASS | 0.315 | 52 x 53 x 55 | 4,762 | 512, 292,400 B | `PickupTop` |
| `SM_Pickup_reverse_other` (megaphone) | `megaphone` | PASS | 0.303 | 31 x 55 x 44 | 4,692 | 512, 349,374 B | `PickupTop` |
| `SM_Pickup_custom` (clapper) | `clapper` | PASS | 0.299 | 35 x 53 x 55 | 4,952 | 512, 288,529 B | `PickupTop` |

Sizes are x, y, z in UE axes (z up); the longest side is 55 cm on each. Pivot is at the base centre, so **device A attaches the mesh component at socket `PickupTop` of `/JamAssets/Props/SM_Pickup_Base` with relative location 0** (the prop's feet then stand on the socket at 45 cm), stencil 1, shadow as the token. No pickup is placed in the rig. The instance goes on the mesh already (slot 0); `M_PropCel` has no spin or bob, so A rotates the component (spin about Z works: pivot is on the axis). The standing reel is a wheel on its edge; the heavy puck is a flat tin (24 cm tall); both read in the stills.

Import rules (`apply_import_rules.py`, run with `ROOT` set to `/JamAssets/Arena` and `/JamAssets/Props`): 12 textures changed and saved (BC1 `CompressionNoAlpha`, `LossyCompressionAmount` Medium, VT off, `MaxTextureSize` 1024; the six pickups are 512 on disk so the cap is moot). Estimated cooked size 699,050 B per arena texture (1K) and 174,762 B per pickup texture (512), before the RDO switch.

### Verification of the pickups (in memory in PIE on `Rink`, next to B10's tokens)

Six `SM_Pickup_Base` with the six new props on them (front row) and six more bases with `SM_Pickup_Token` + `MI_Pickup_<kind>` (back row, 1 m to the side, 2.5 m behind). Stills: `b11-pickups-dark10`, `-dark30` (dark band at y -17), `-pool10`, `-pool30` (pool at x 6 to 16, y -1.5), each from the game-camera height. At 10 m every prop reads by silhouette and colour in both lighting conditions: yellow reel with black spokes, cyan barrel with a black lens, red flat tin, violet lamp with a cream lens, magenta horn, green slate with a striped stick; they are visibly larger than the generic tokens. At 30 m they read as small coloured shapes (roughly 15 to 25 px) with the black outline; in the dark band the six colours stay distinct. **Emissive 5 reads at 30 m in the dark band**; I did not try 4 or 6.

## Files created or changed, bytes on disk (`Game/Plugins/JamAssets/Content/`)

Changed: `Rigs/LR_L2_Night.umap` 468,713 (was 452,071); `Arena/SM_FlightCases.uasset` 408,616 (was 405,880).
New: see the ledger rows below (each row carries its size). Also under `devlog/shots/`: 18 PNGs of 1920 x 1063 and their captions, about 20 MB.

## Total triangles of the dressing

**238,758** (was 202.9k after B2c): 3 plane 6, 123 cubes 5,904, 35 cylinders 17,920, `SM_FilmLight` 8 x 5,038, `SM_DirectorChair` 5, `SM_Slate` 4, `SM_CameraDolly` 4, `SM_CameraCrane` 2, `SM_FlightCases` 12 x 3,782 (now), `SM_TrussLamps` 2, plus the new `SM_Bench` 2 x 4,912, `SM_CoachStool` 2 x 3,778, `SM_WaterCrate` 2 x 3,652, `SM_PenaltyBox` 5,036, `SM_GoalLight` 2 x 5,172, `SM_ScoreboardHousing` 4,912 (script sum over Dress and Room). Budget 250k. Without the flight-case, crate and stool reductions it was 257.4k.

## Ledger rows (paste into `ASSETS.md`; the umap row replaces the B2c row, the `SM_FlightCases` row only changes the size)

```
| Game/Plugins/JamAssets/Content/Arena/SM_Bench.uasset | mesh | 0.4 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | Rodin task 8c8c6999-4c51-4b7a-a002-5a9dd6779512 (status/B-props-n6.md), pivot and scale baked, Nanite keep 50% (lane B11) |
| Game/Plugins/JamAssets/Content/Arena/SM_PenaltyBox.uasset | mesh | 0.4 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | Rodin task 244171f5-05e1-4257-8b19-6fe726e53996, Nanite keep 50% (lane B11) |
| Game/Plugins/JamAssets/Content/Arena/SM_GoalLight.uasset | mesh | 0.4 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | Rodin task c8c954d0-08d0-49fb-afb8-340ebd7be770, Nanite keep 50% (lane B11) |
| Game/Plugins/JamAssets/Content/Arena/SM_ScoreboardHousing.uasset | mesh | 0.3 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | Rodin task f58cd4f7-ba92-4d7f-ab6e-2ed8f6968f82, Nanite keep 50% (lane B11) |
| Game/Plugins/JamAssets/Content/Arena/SM_WaterCrate.uasset | mesh | 0.4 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | Rodin task ce70c14e-640e-4aa5-bed2-95aa8263beb4, Nanite keep 30% (lane B11) |
| Game/Plugins/JamAssets/Content/Arena/SM_CoachStool.uasset | mesh | 0.4 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | Rodin task 88bf26e1-6637-454a-8318-2e14255c58dc, Nanite keep 30% (lane B11) |
| Game/Plugins/JamAssets/Content/Arena/SM_FlightCases.uasset | mesh | 0.4 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | Rodin task be52e7de-3e16-4984-ae1b-24929550b31e (B2b), Nanite keep 35% since lane B11 (408616 bytes) |
| Game/Plugins/JamAssets/Content/Arena/T_Bench_BaseColor.uasset | texture | 0.5 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | 1K base colour from Rodin task 8c8c6999-4c51-4b7a-a002-5a9dd6779512, no text (lane B11) |
| Game/Plugins/JamAssets/Content/Arena/T_PenaltyBox_BaseColor.uasset | texture | 0.7 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | 1K base colour from Rodin task 244171f5-05e1-4257-8b19-6fe726e53996, no text (lane B11) |
| Game/Plugins/JamAssets/Content/Arena/T_GoalLight_BaseColor.uasset | texture | 0.5 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | 1K base colour from Rodin task c8c954d0-08d0-49fb-afb8-340ebd7be770, no text (lane B11) |
| Game/Plugins/JamAssets/Content/Arena/T_ScoreboardHousing_BaseColor.uasset | texture | 0.6 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | 1K base colour from Rodin task f58cd4f7-ba92-4d7f-ab6e-2ed8f6968f82, blank faces, no text (lane B11) |
| Game/Plugins/JamAssets/Content/Arena/T_WaterCrate_BaseColor.uasset | texture | 0.7 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | 1K base colour from Rodin task ce70c14e-640e-4aa5-bed2-95aa8263beb4, four cap discs flat-filled to remove an L and a plus shape (lane B11) |
| Game/Plugins/JamAssets/Content/Arena/T_CoachStool_BaseColor.uasset | texture | 0.6 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | 1K base colour from Rodin task 88bf26e1-6637-454a-8318-2e14255c58dc, no text (lane B11) |
| Game/Plugins/JamAssets/Content/Arena/MI_Bench.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | M_PropCel instance (lane B11) |
| Game/Plugins/JamAssets/Content/Arena/MI_PenaltyBox.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | M_PropCel instance (lane B11) |
| Game/Plugins/JamAssets/Content/Arena/MI_GoalLight.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | M_PropCel instance (lane B11) |
| Game/Plugins/JamAssets/Content/Arena/MI_ScoreboardHousing.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | M_PropCel instance (lane B11) |
| Game/Plugins/JamAssets/Content/Arena/MI_WaterCrate.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | M_PropCel instance (lane B11) |
| Game/Plugins/JamAssets/Content/Arena/MI_CoachStool.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | M_PropCel instance (lane B11) |
| Game/Plugins/JamAssets/Content/Props/SM_Pickup_speed.uasset | mesh | 0.3 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | Rodin task 2bc1a8ef-c4f4-4793-a9ce-ce4dc834181d (reel3), 55 cm, Nanite off (lane B11) |
| Game/Plugins/JamAssets/Content/Props/SM_Pickup_precision.uasset | mesh | 0.2 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | Rodin task 2ee28217-51d5-4631-8f53-01cf0110d04d (lens), Nanite off (lane B11) |
| Game/Plugins/JamAssets/Content/Props/SM_Pickup_heavy_puck.uasset | mesh | 0.2 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | Rodin task 63df2422-2478-4fd5-8dd3-d076e02c9e00 (canister3), Nanite off (lane B11) |
| Game/Plugins/JamAssets/Content/Props/SM_Pickup_big_puck.uasset | mesh | 0.2 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | Rodin task 6bde7a6f-7440-441f-82e3-7e80ce8957b1 (spot2), Nanite off (lane B11) |
| Game/Plugins/JamAssets/Content/Props/SM_Pickup_reverse_other.uasset | mesh | 0.2 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | Rodin task 21e1de89-19df-4e74-82df-ec8c6123fa3e (megaphone), Nanite off (lane B11) |
| Game/Plugins/JamAssets/Content/Props/SM_Pickup_custom.uasset | mesh | 0.3 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | Rodin task 0b4839da-61c3-4f47-a812-a11577d15b7d (clapper), Nanite off (lane B11) |
| Game/Plugins/JamAssets/Content/Props/T_Pickup_speed_BaseColor.uasset | texture | 0.4 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | 512 base colour from Rodin task 2bc1a8ef-c4f4-4793-a9ce-ce4dc834181d, no text (lane B11) |
| Game/Plugins/JamAssets/Content/Props/T_Pickup_precision_BaseColor.uasset | texture | 0.3 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | 512 base colour from Rodin task 2ee28217-51d5-4631-8f53-01cf0110d04d, no text (lane B11) |
| Game/Plugins/JamAssets/Content/Props/T_Pickup_heavy_puck_BaseColor.uasset | texture | 0.3 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | 512 base colour from Rodin task 63df2422-2478-4fd5-8dd3-d076e02c9e00, no text (lane B11) |
| Game/Plugins/JamAssets/Content/Props/T_Pickup_big_puck_BaseColor.uasset | texture | 0.3 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | 512 base colour from Rodin task 6bde7a6f-7440-441f-82e3-7e80ce8957b1, no text (lane B11) |
| Game/Plugins/JamAssets/Content/Props/T_Pickup_reverse_other_BaseColor.uasset | texture | 0.3 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | 512 base colour from Rodin task 21e1de89-19df-4e74-82df-ec8c6123fa3e, no text (lane B11) |
| Game/Plugins/JamAssets/Content/Props/T_Pickup_custom_BaseColor.uasset | texture | 0.3 | ai-generated | Hyper3D Rodin |  |  | 2026-11-12 | B | y | 512 base colour from Rodin task 0b4839da-61c3-4f47-a812-a11577d15b7d, stripes only, no text (lane B11) |
| Game/Plugins/JamAssets/Content/Props/MI_PickupProp_speed.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | M_PropCel instance, Emissive 5 (lane B11) |
| Game/Plugins/JamAssets/Content/Props/MI_PickupProp_precision.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | M_PropCel instance, Emissive 5 (lane B11) |
| Game/Plugins/JamAssets/Content/Props/MI_PickupProp_heavy_puck.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | M_PropCel instance, Emissive 5 (lane B11) |
| Game/Plugins/JamAssets/Content/Props/MI_PickupProp_big_puck.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | M_PropCel instance, Emissive 5 (lane B11) |
| Game/Plugins/JamAssets/Content/Props/MI_PickupProp_reverse_other.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | M_PropCel instance, Emissive 5 (lane B11) |
| Game/Plugins/JamAssets/Content/Props/MI_PickupProp_custom.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | M_PropCel instance, Emissive 5 (lane B11) |
| Game/Plugins/JamAssets/Content/Rigs/LR_L2_Night.umap | other | 0.5 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | night rig re-laid for 90 x 45: room +-60 x +-37, truss over the pool centres, end sets at x 50-58, long sides at y 26-34, 8 shadowless practicals, 10 generated arena pieces (lanes B2, B2b, B2c, B1c, B11); replaces the B2c row (468713 bytes) |
```

Sizes in bytes for the new rows: meshes `SM_Bench` 367,287; `SM_PenaltyBox` 362,163; `SM_GoalLight` 372,433; `SM_ScoreboardHousing` 341,660; `SM_WaterCrate` 377,564; `SM_CoachStool` 397,329; `MI_Bench` 8,675; `MI_PenaltyBox` 10,181; `MI_GoalLight` 9,562; `MI_ScoreboardHousing` 11,340; `MI_WaterCrate` 9,665; `MI_CoachStool` 10,783; `MI_PickupProp_*` 9,135 to 11,221 (listed in git); `SM_Pickup_*` 211,626 to 274,838; `T_Pickup_*` 288,529 to 446,609.

## Commits

Two commits on `lane/b-vfx`: `68bd060` (part 1: rig, the six arena pieces with textures and instances, before/after stills); the second (pickup props, pickup stills, this report) follows it, see `git log`.

## What I could not verify

- **Anything on the real game** (old binaries cannot build the rink): the real follow camera, the real nets, boards, beacon posts (I assumed the beacon is within 1 m of the net at y = 0; the hero groups keep everything at |y| >= 3.5), A's `SetTier` scaling, the large cut. All stills use the stand-in.
- **Frame cost.** No frame numbers (two editors up). The dressing is 238.8k triangles of which 10 new Nanite instances are about 49k.
- **The ice-brightness drop at the end boards** is isolated to the truss by single-shot hide tests, not by a controlled A/B.
- **The real ice material at 90 x 45**: the stand-in ice uses `MI_Ice_Night` with its stored `RinkHalfLength`, so the painted lines on it are for a larger rink (A drives them on the real one).
- The goal lights are unlit only in intent (flat fill makes the lamp read red).
- Pickup scale on the real base in motion (A's spin and bob).

## What the lead must decide

1. **Corner ice darkening (-7 to -12 %) at the end boards** because the truss lamps left x +-45. Accept it (the pools are untouched), or have a faint blue emissive plane or an end-board practical added to put it back.
2. **Triangle budget.** 238.8k with `SM_FlightCases` reduced to Nanite 35 % and crate and stool at 30 %. If those should stay at 50 %, the total is 257.4k, over 250k.
3. **Pass-1 cube benches were removed** (8 actors, 96 triangles); say if they should come back on the +y side.
4. **Device A:** attach each pickup at `PickupTop` with relative location 0 and rotate the component; `M_PropCel` has no spin. B10's `M_Pickup` colours a mesh by a flat colour parameter, not a texture, so these meshes keep their own `M_PropCel` instances; if A wants bob and spin by WPO it needs a texture-driven variant of `M_Pickup` (not made).
5. **Device A:** the rig floor is at -5 cm as asked; A's floor should go to -20 cm (I put it there in the stand-in; I did not look for floor streaks).
6. **Build:** `JamEffectTypes.h(124) 'EJamTarget'` still breaks the editor build; the next lane on this tree needs it fixed first.
