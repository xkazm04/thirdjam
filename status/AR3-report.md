# AR3 report: a crowd that answers a goal, and the barn above the ice (device B, Blender only)

Lane AR3, device B, machine clock 01:43 to 02:14 (2026-11-14). Track: the arena and the clay look. Lab: `C:\Users\kazda\jam\arena-lab\`.

Housekeeping: no Unreal, no editor, no git write. Written in the repository: this report and five stills with captions in `devlog/shots/`. **`out\fbx\` (lane AR1's files, which AR2 is importing) was read and not written**: all 12 checksums are the same before and after (`work\fbx-ar1.md5`). One Blender process at a time throughout. Nothing from outside was read; every mesh is built from primitives.

## Verdict

**Both evolutions are built, exported and proved in Blender: 31 FBX in `out\fbx-ar3\`, all under budget, all read back.** Neither is proved in the engine: nothing was imported.

1. **The crowd answers a goal.** Each of AR1's six figures and two rows, and six new seated figures, has a second pose that is the same mesh with its vertices moved. A half-full house laid out from eleven of the meshes has no repeat among any fan's ten nearest.
2. **The barn has a roof.** A 74 m timber truss with a 12 m bay of purlins, plain bunting for the ends and a gate for the ice machine. From the game camera at the side boards none of it is in frame; looking up the ice it takes 0.74 % of the frame; the ice's mean luma moves by 0.06 % at most.

## The render rule, and what it cost

The scripts were switched before the first render: Cycles on the CPU, 12 samples, 6 threads, 960 x 540, and `run.sh` no longer starts two Blenders side by side.

| | AR1 (yesterday) | AR3 (tonight) |
|---|---|---|
| Still | 1920 x 1080, 72 samples, all 20 threads | 960 x 540, 12 samples, 6 threads |
| Time per game-camera still | 69 to 108 s (6 stand modules, 87 fan instances) | 19 to 28 s (the whole barn: 41 stand pieces, 237 fans, 23 new pieces) |
| Piece-sheet still | 19 to 39 s | 7 to 13 s |

The cost is a quarter of the pixels and a sixth of the samples. At 12 samples the denoiser smears faces and scarves, so tonight's numbers are means and shares of the frame. The p99 and maximum I give for the new barn pieces are from denoised 12-sample stills and are not comparable with AR1's crowd figures.

`run.sh build` would rewrite `out\fbx\`, so it now refuses unless `AR1_REBUILD=1` is set.

## Evolution 1: a crowd that can answer a goal

| Pair | Seated | Second pose | Triangles (budget 900) | Vertices seated = cheering | Largest move (cm) |
|---|---|---|---|---|---|
| `SM_Fan_A` + `_Cheer` | AR1: hands on knees | half risen, both arms up | 608 | 346 = 346 | 90 |
| `SM_Fan_B` + `_Cheer` | AR1: both arms up | on the feet, both fists | 620 | 356 = 356 | 35 |
| `SM_Fan_C` + `_Cheer` | AR1: standing, one fist | both fists, leaning back | 540 | 308 = 308 | 92 |
| `SM_Fan_D` + `_Cheer` | AR1: leaning on the rail | upright, both arms up | 544 | 308 = 308 | 57 |
| `SM_Fan_E` + `_Cheer` | AR1: standing with a mug | mug raised, other fist up | 572 | 324 = 324 | 78 |
| `SM_Fan_F` + `_Cheer` | AR1: child, hands at the mouth | on the feet, both arms up | 604 | 344 = 344 | 29 |
| `SM_Fan_G` + `_Cheer` | new: broad, arms crossed, tall floppy hat | on the feet, both fists | 608 | 346 = 346 | 87 |
| `SM_Fan_H` + `_Cheer` | new: slim and tall, hand at the chin, flat cap | half risen, one arm up | 520 | 298 = 298 | 52 |
| `SM_Fan_I` + `_Cheer` | new: hands in the lap, hood up, thick wrap | still seated, turned to a neighbour, clapping | 588 | 332 = 332 | 29 |
| `SM_Fan_J` + `_Cheer` | new: small, pointing at the ice, ear-flap hat | half risen, both arms up | 620 | 356 = 356 | 74 |
| `SM_Fan_K` + `_Cheer` | new: turned to a neighbour, bare head with a bun | still seated, turned further, a hand out | 596 | 338 = 338 | 55 |
| `SM_Fan_L` + `_Cheer` | new: youngster, elbows on knees | on the feet, both arms up | 608 | 346 = 346 | 54 |
| `SM_FanRow_A` + `_Cheer` | AR1's row of 7 | each answers in its own way | 4,160 (4,500) | 2,374 = 2,374 | 90 |
| `SM_FanRow_B` + `_Cheer` | AR1's row of 7 | each answers in its own way | 4,084 (4,500) | 2,328 = 2,328 | 89 |

**The proving numbers**

- **Triangles per figure:** 520 to 620, inside AR1's 400 to 900.
- **Vertex count seated = cheering: equal on all 14 pairs.** Also equal: every face's vertex indices and slot, UV0, UV1, the vertex colours and the slot list. A fresh normal pass would turn 0 faces of any cheering mesh.
- **Longest run of identical neighbours along a row in the half-full layout: 1** (no two adjacent fans share a mesh). **Fans with their own mesh among their ten nearest: 0 of 231.** The nearest fan of the same mesh is 2.59 m away at the least, 3.72 m in the median.
- **The goal at the game camera:** 3.1 % of the frame changes by more than 8 luma when every fan answers. The ice does not: mean luma 167.51 seated and 167.51 cheering, 0 ice pixels changed.

**How the second pose is made.** For A to F and the rows the seated mesh is AR1's own file, read from `out\fbx\`; the cheering mesh is that mesh with other vertex positions. So `SM_Fan_A_Cheer` matches the `SM_Fan_A` the engine already has, and AR1's meshes need no re-import for a swap. One thing found on the way: rebuilding a figure from the recipe gives the same vertices in the same order (within 0.00002 cm) but cut some quads along the other diagonal on three of the six, which is why the file and not the rebuild is the base.

**The new figures.** Six seated builds: broad, slim and tall, small, a youngster; head tilts and turns; arms crossed, at the chin, in the lap, pointing, on the bench, elbows on knees; a tall floppy hat, a hood, a bare head with a bun. Coats are charcoal, taupe, moss, plum, oat and green; scarves and wraps cream, ochre, taupe, wine and charcoal. No text, no symbol, two dots and a line for a face.

## Evolution 2: the barn above the ice

| Mesh | What | Size x, y, z (m) | Triangles (budget) | Slots |
|---|---|---|---|---|
| `SM_Barn_TrussBay` | one 74 m truss (doubled bottom chord, king post, 8 posts, 8 struts, 3 iron straps) and one 12 m bay of 10 purlins, a ridge beam and 6 rafters | 12.79 x 74.69 x 10.25 | 492 (900) | Timber, TimberDark, Tin |
| `SM_Barn_Bunting` | 12 m of 14 plain cloth pennants on a sagging cord | 12.01 x 0.17 x 1.70 | 232 (400) | Cloth |
| `SM_Barn_IceGate` | a timber double gate, 5.2 x 4.4 m of leaves in a post-and-lintel frame, strap hinges, one leaf a hand ajar | 6.71 x 0.58 x 5.03 | 456 (700) | Timber, TimberDark, Tin |

**The proving numbers**

- **Triangles:** 492 per bay. A full barn is 10 bays (4,920) + 12 lengths of bunting (2,784) + the gate (456) = **8,160**. The ten trusses it replaces are 720.
- **Share of the frame and the ice**, in a mock-up of the clay barn round the regular rink (walls 120 x 74 m, eave 13.5 m, AR1's stands 4 m beyond the boards):

| View | New pieces' share of the frame | Their mean luma | Ice mean luma without | with | Change |
|---|---|---|---|---|---|
| AR1's game camera (12 m inside the side boards, 20 degrees down) | 0.00 % | not in frame | 167.51 | 167.45 | -0.04 % |
| The game camera up the ice, 38 m from the end boards | 0.74 % | 34.1 | 168.03 | 167.93 | -0.06 % |
| A low camera at centre ice (1.5 m, 9 degrees up) | 20.66 % | 20.0 | 165.97 | 165.92 | -0.03 % |

- **The ice is unchanged within 1 % in every view** (0.06 % at most).
- **Nothing is lower than 7 m over the ice:** the truss's lowest point is 5 cm under the eave (13.45 m; 9.0 m on Pond at rig scale 0.667). The bunting hangs over the end apron, not the ice, with its lowest tip 7.0 m up when hung at 8.7 m. The gate stands at the end wall.
- **Matte and dark:** from the low camera the new pieces read mean luma 20 against ice at 166.

## What the engine lane needs to know

**To swap or blend the crowd (three ways, pick one)**

1. **Swap the mesh** for the 108 frames: same pivot, same slots, same instance transform. On instanced components, move the instance from the seated component to the cheering one and back.
2. **Blend in the material, no swap.** Every seated mesh in `out\fbx-ar3\` carries three more UV channels: UV2 `PoseX`, UV3 `PoseY`, UV4 `PoseZ`. Their u is the offset in centimetres to the cheering pose along the mesh's local X, Y, Z as Unreal sees them; v is 0, so the importer's V flip cannot touch them. World position offset = the local-to-world transform of (UV2.u, UV3.u, UV4.u), times a 0 to 1 value per instance. Read back from the files, seated position + offset lands on the cheering vertex within 0.0001 cm. This way needs the `out\fbx-ar3\` copy of A to F (the same mesh plus the channels), and bounds raised by up to 92 cm.
3. Each `_Cheer` mesh carries the same channels leading back.

- **Slots:** A to F and the rows keep AR1's (`ClaySkin`, `ClayCoat`, `Scarf`, `Tin` on the mug figures). **G to L have only `ClaySkin` and `ClayCoat`**: their scarves and hats are muted vertex colours, so no team colour can reach them.
- **Reach:** a risen figure stands in its own footwell, nothing further forward than 0.53 m from the seat line. Raised hands reach up to 0.52 m to either side, so two neighbours answering together cross arms now and then.
- **The no-repeat rule:** give a fan any mesh that none of its ten nearest fans wears, then repair until none breaks it (`crowd_layout` in `scripts\render3.py`). It needs eleven meshes to always have a choice: A, B, F and G to L seated, C and E standing; D only at a rail.

**To place the barn pieces**

- **`SM_Barn_TrussBay`:** origin mid-span on the eave plane (the walls' top). Ten at x = -60 + 12 k (k = 0 to 9), y = 0, z = 13.5 m, yaw 0; the purlins reach +X to the next truss. They replace the ten `SM_Con_ClayTruss`. It follows the rig's uniform scale. The rig's lamps hang under the chord (keys at 13 m, side lamps at 11.5 m). Cast no shadow, like the rig's other pieces.
- **The roof's pitch is my guess** (a 10.2 m rise). The origin is on the eave plane so that a local Z scale changes only the rise: match it to the gables and roof slabs.
- **`SM_Barn_Bunting`:** origin at the middle of the span at the hanging points' height; the cloth hangs 1.68 m below. Six in a row across each end at x = +-(L/2 + 8), yaw 90, scaled 74/72 along X. Hang the origin at 8.7 m or more on every rink, by hand where the rig's scale would lower it. Keep clear of the end lamp shades, the goal lights (x 45.0 to 46.1) and the scoreboard. Never over the ice.
- **`SM_Barn_IceGate`:** origin on the floor at the middle of its rink-side face; its back, 0.58 m behind, meets the wall. One end wall only, 15 to 20 m off the centre line, in a gap between two wall posts (it is 6.7 m wide over the lintel). It needs no hole in the wall.
- The side zones (|y| under 24.6 m) and the goal-light zones are untouched: nothing of AR3's stands on the apron near the boards.

`out\fbx-ar3\MANIFEST.md` has every line in full.

## What I did not look at

- **The engine.** No import; the pose channels as world position offset are untested in Unreal.
- **The rig's real geometry.** The trusses' positions, the gables' rise, the roof slabs and the bulb strings are from B19's table, not from the map. AR2's actual stand line is from B41's sentence (4 m).
- **The blend half way.** A straight vertex blend between seated and risen shortens the arms mid-way. Over a few frames it will not show; held at 50 % it would. Not rendered.
- **Motion:** shimmer under upscaling, and the bunting, which is static.
- **Other rink sizes** in a still: the placement for Pond and the wide arena is arithmetic.
- **The mock-up's light** is AR1's assumption again (and its roof and far walls let the fill light through, so that AR1's light stays the same). The luma numbers compare with each other, not with the engine's.
- The row meshes are not in the half-full layout, and no row was made from the new figures.

## Decisions for the operator

1. **Who answers a goal.** In the stills every fan does. A real house splits: only the scoring side's half, or a random two in three, is one number for the engine lane.
2. **Scarves.** AR1's six carry a team-colour scarf slot (AR1's brief); tonight's six carry none (tonight's rule). The house is now mixed. If the crowd is to wear no team colour at all, bind `Scarf` to a plain muted clay, as the stills here do.
3. **The bunting at the top of the frame.** Looking up the ice, its tips sit in the top edge (0.74 % of the frame with the gate, mean luma 34). Keep, hang at 10.5 m and out of the frame, or drop.
4. **The gate:** which end, and whether the end stands leave a gap in front of it.
5. **The roof replaces the ten trusses the rig has.** Say if it should hang beside them instead.

## How to rebuild

```
cd /c/Users/kazda/jam/arena-lab
bash run3.sh            # about 5 minutes: build (6 s), piece sheet (35 s), nine views with masks (240 s), manifest
bash run3.sh build | sheet | views | manifest
```
The whole chain ran end to end once (02:01 to 02:05); the views and manifest once more after a low crowd view was added (02:05 to 02:09).

## Outputs

- `C:\Users\kazda\jam\arena-lab\out\fbx-ar3\` : 31 FBX (31,724 triangles, 2.5 MB) and `MANIFEST.md`
- `C:\Users\kazda\jam\arena-lab\out\renders-ar3\` : `ar3-pieces.png`, `ar3-goal-ab.png`, `ar3-goal-low-ab.png`, `ar3-low-ab.png`, `ar3-upice-ab.png`, `ar3-gamecam-barn-ab.png` and the single stills
- `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-021017-B-ar3-pieces.png`, `-goal-gamecam.png`, `-goal-low.png`, `-barn-low.png`, `-barn-upice.png`, each with a `.txt` caption
- `C:\Users\kazda\jam\arena-lab\scripts\build3.py`, `render3.py`, `manifest3.py`, `C:\Users\kazda\jam\arena-lab\run3.sh`
- `C:\Users\kazda\jam\arena-lab\work\stats3.json`, `views3.json`, `fbx3-readback.json`, `fbx-ar1.md5`, and AR1's scripts as they were (`*.ar1`)

## Next three evolutions, ranked

1. **The goal answer in play, in the engine:** the crowd blends to its second pose over the 108 frames together with device A's lamp and net. The number: mean ice shift under 1.0 in every channel across the 108 frames (B41's bar for the goal moment), 0 crowd pixels changed outside those frames, and frame time with every fan answering within 0.2 ms of AR2's measure.
2. **A far crowd that costs a quarter:** a second level of every figure at about 150 triangles with the same pose channels, for the far side and the ends. The number: far-side crowd triangles at 25 % or less of today's, with a mean difference under 2 luma in the crowd band from the game camera.
3. **The ice machine parked by its gate:** a clay resurfacer, the one prop every rink has. The number: under 2,500 triangles, under 1 % of the up-ice game-camera frame, outside the side and goal-light zones, ice mean luma unchanged within 1 %.
