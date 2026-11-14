# AR4 report: a far crowd that costs a quarter, and the barn's walls and ends dressed (device B, Blender only)

Lane AR4, device B, machine clock 02:16 to 02:50 (2026-11-14). Track: the arena and the clay look. Lab: `C:\Users\kazda\jam\arena-lab\`.

Housekeeping: no Unreal, no editor, no git write. Written in the repository: this report and five stills with captions in `devlog/shots/`. **`out\fbx\` (AR1) and `out\fbx-ar3\` (AR3) were read and not written**: all 43 checksums are the same before and after (`work\fbx-ar1.md5`, `work\fbx-ar3.md5`). CPU renders only (Cycles, 12 samples, 6 threads, 960 x 540), one Blender process at a time. Every mesh is built from primitives.

## Verdict

**Both evolutions are built, exported and proved in Blender: 32 FBX in `out\fbx-ar4\`, all under budget, all read back.** Neither is proved in the engine.

1. **The far crowd costs 23 %.** Every figure and both rows have a second level at 126 to 146 triangles, with the full figure's slots, pivot and pose channels, so the answer to a goal still works. Seated and cheering vertex counts are equal on all 14 pairs. Over the crowd's band at the far side the picture differs by 0.95 luma. **One number misses the bar if read strictly:** on the figures' own pixels the difference is 3.1 luma, against a noise floor of 1.5.
2. **The barn is no longer empty.** Wall bays with posts, braces and a dim night window, a hay loft over the gate, the ice machine parked, props leaning by the gate: 8,116 triangles for a full barn. They take 3.0 % and 3.9 % of the two game-camera frames, all above the boards, and are darker than everything that has to read. The ice's mean luma moves by 0.11 % at most.

**One thing the engine lane must know first: use the far figures by distance, not by side.** The game camera follows the puck, so the "far side" is 37 m away when play is at the near boards and 16 m away when play crosses. At 16 to 20 m the far figures are visibly cruder.

## Evolution 1: a far crowd that costs a quarter

| Pair (`_L1` + `_L1_Cheer`) | Triangles far / full | Share | Vertices seated = cheering | Slots (as the full figure, same order) |
|---|---|---|---|---|
| `SM_Fan_A_L1` | 146 / 608 | 24.0 % | 105 = 105 | ClaySkin, ClayCoat, Scarf |
| `SM_Fan_B_L1` | 138 / 620 | 22.3 % | 99 = 99 | ClaySkin, ClayCoat, Scarf |
| `SM_Fan_C_L1` | 126 / 540 | 23.3 % | 91 = 91 | ClaySkin, ClayCoat, Scarf |
| `SM_Fan_D_L1` | 126 / 544 | 23.2 % | 91 = 91 | ClaySkin, ClayCoat, Scarf |
| `SM_Fan_E_L1` | 134 / 572 | 23.4 % | 99 = 99 | Tin, ClaySkin, ClayCoat, Scarf |
| `SM_Fan_F_L1` | 138 / 604 | 22.9 % | 99 = 99 | ClaySkin, ClayCoat, Scarf |
| `SM_Fan_G_L1` | 138 / 608 | 22.7 % | 99 = 99 | ClaySkin, ClayCoat |
| `SM_Fan_H_L1` | 126 / 520 | 24.2 % | 87 = 87 | ClaySkin, ClayCoat |
| `SM_Fan_I_L1` | 138 / 588 | 23.5 % | 99 = 99 | ClaySkin, ClayCoat |
| `SM_Fan_J_L1` | 138 / 620 | 22.3 % | 99 = 99 | ClaySkin, ClayCoat |
| `SM_Fan_K_L1` | 138 / 596 | 23.2 % | 99 = 99 | ClaySkin, ClayCoat |
| `SM_Fan_L_L1` | 138 / 608 | 22.7 % | 99 = 99 | ClaySkin, ClayCoat |
| `SM_FanRow_A_L1` | 962 / 4,160 | 23.1 % | 691 = 691 | ClaySkin, ClayCoat, Scarf |
| `SM_FanRow_B_L1` | 958 / 4,084 | 23.5 % | 691 = 691 | Tin, ClaySkin, ClayCoat, Scarf |

**The proving numbers**

- **Far-side crowd triangles: 23.1 % of today's** (bar: 25 % or less). In the mock-up's half-full far side, 217 fans go from 127,532 to 29,522 triangles. No single figure is over 24.2 %.
- **Vertex count seated = cheering: equal on all 14 pairs.** So are the faces, UV0 and slots: the cheering mesh is the seated mesh with its vertices moved.
- **Mean difference against the full figures, far side from the game camera (37 to 41 m):**

| Where | Mean absolute difference (luma) | The same picture with other noise | Mean luma full / far |
|---|---|---|---|
| The band of rows the far crowd stands in | **0.95** | 0.78 | 48.36 / 48.31 |
| The figures' own pixels (23 % of that band) | **3.14** | 1.51 | 23.18 / 22.94 |

- **Read as the band, the bar of 2 luma is met (0.95). Read as the figures' pixels alone, it is not (3.14).** What remains there is outline: at 20 pixels tall, a six-sided head and a three-sided arm move the edge by part of a pixel. The mean brightness of the figures is the same within 0.24.
- This measure needed more than one 12-sample still: two such stills of the same scene differ by 3.1 on the figures' pixels. The table is from the band's rows only, at 16 samples, four noise seeds averaged per picture (12 strips, 53 s in all).
- **The pose channels work on the far figures:** read back from the files, seated position + offset lands on the cheering vertex within 0.0001 cm.

**One pass was made after the first measure.** The first far figure read 3.33 on the figures' pixels and 0.62 luma darker than the full one. Widening the six-sided body and head by 5 to 6 % to fill the round outline, and easing the head's darkening from 7 % to 3 %, brought it to 3.14 and 0.24.

**What a far figure leaves out:** eye dots and mouth, mittens, boots, the scarf's hanging end, cap brims and ear flaps. Body and head are six-sided, arms three-sided, each pair of legs one slab.

**The limit.** Put on the near side at AR1's game camera (16 to 20 m), the far figures differ by 4.3 luma on their pixels and it shows by eye: no faces, pointed hats (`ar4-near-limit.png`). Use the full figures there.

## Evolution 2: the barn's walls and ends

| Mesh | What | Size x, y, z (m) | Triangles (budget) | Slots |
|---|---|---|---|---|
| `SM_Barn_WallBay` | 12 m of wall, 13.5 m high: a post, two girts and a top plate, a cross-brace, two knee braces, one high window (3.4 x 2.6 m at 9.75 to 12.35 m) in a timber reveal 0.5 m deep | 12.11 x 0.62 x 13.50 | 180 (400) | Timber, TimberDark, NightGlass |
| `SM_Barn_Loft` | 12 m of plank gallery: deck at 6.3 m on brackets, a plank rail, nine hay bales, a ladder | 12.01 x 4.18 x 7.56 | 432 (700) | Timber, TimberDark, Hay |
| `SM_Barn_IceMachine` | a clay resurfacer, parked: snow tank, seat and wheel, four fat tyres, the conditioner behind; no text, no mark | 4.71 x 2.42 x 2.52 | 804 (2,500) | Paint, Tin, Rubber |
| `SM_Barn_GateProps` | along 7.7 m of wall: a coiled hose, six sticks in a rack, a shovel and a broom, four buckets, three spare board panels | 7.67 x 1.15 x 1.99 | 688 (900) | Timber, TimberDark, Tin, Cream, Hay, Rubber |

**The proving numbers**

- **Triangles:** 180 per wall bay. A full barn is 32 bays (5,760) + 2 lofts (864) + the machine (804) + the props (688) = **8,116**.
- **Share of the frame and the ice** (the mock-up of AR3, with AR1's stands and AR3's roof, bunting and gate in both stills):

| View | AR4's pieces: share of frame | Their luma: mean / p99 / max | Ice mean luma without | with | Change |
|---|---|---|---|---|---|
| Game camera at the side boards | 3.03 % | 28.3 / 41.1 / 70.7 | 167.46 | 167.42 | -0.02 % |
| Game camera up the ice | 3.88 % | 27.6 / 46.9 / 81.8 | 167.94 | 167.87 | -0.04 % |
| Low camera at the gate's corner | 15.07 % | 21.7 / 44.6 / 70.5 | 158.14 | 157.96 | -0.11 % |

- **The ice machine alone: 0.19 % of the up-ice frame** (bar: under 1 %), mean luma 30.7. It is not in the side-boards frame.
- **The windows are in neither game-camera frame** (they sit above 9.75 m). From the low camera they take 2.2 % of the frame at mean luma 29.3, maximum 44.4.
- **The order is kept.** In the up-ice still: ice 167.9, boards 144.4, an orange jersey block 97.6, a blue jersey block 69.0, then AR4's pieces at mean 27.6 and p99 46.9. Their brightest pixel (81.8) is under the boards and the orange block; it is over the blue block's mean.
- **Not measured: the puck.** My stand-in disc covers 55 pixels that the denoiser blends with the ice.
- **Clear zones:** every piece stands at a wall (|y| over 33 m or x over 55 m on the regular rink). Nothing is within 8 m of the side zones or the goal-light zones.

## What the engine lane needs to know

**The far figures**

- **By distance.** Import each `_L1` as LOD 1 of its full figure, or keep two instanced components per figure and choose by distance to the camera. These numbers support a switch at 35 m or more and rule out 20 m or less; between the two is untested.
- Same pivot and frame as the full figure. Same slots in the same order. UV0, UV1, the vertex colours and the three pose channels (UV2 to UV4) are all there, so one material serves both levels.
- Some parts are open shells (hat, scarf ring, arms): keep the material one-sided; every part faces outwards.

**The wall pieces share one frame:** origin on the floor 0.58 m in front of the wall's face, local +Y at the wall, local +X along it. Yaw as the stands: 0 on the +y wall, 180 on -y, -90 on the +x end, +90 on the -x end.

- **`SM_Barn_WallBay`:** ten along each long wall (x = -60 + 12 k on +y; x = +60 - 12 k on -y), six along each end scaled 74/72 along X. It follows the rig's uniform scale. Below 4.4 m there is only the post. Cast no shadow.
- **The slot `NightGlass`** is the one emissive slot: one box per window behind the glazing bars. Colour about sRGB (89, 112, 158). **Tune it on screen to luma 20 to 40, never above 60**, no bloom. In the mock-up it reads 29.
- **A rig light at a window:** there is no hole in the wall. A shadowless rect light 3.2 x 2.4 m can sit in the reveal just in front of the glass (local x 6.0, y +0.45, z 11.05 m), facing local -Y. Keep its reach short of the ice.
- **`SM_Barn_Loft`:** on one end wall, same origin and yaw as that wall's bays, over the gate's bay and the next. The deck overhangs 2.0 m; the ladder's foot stands 4.2 m from the wall.
- **`SM_Barn_IceMachine`:** origin on the floor under its middle, nose along local +X. Parked parallel to the end wall about 2 m off it, on the side of the gate away from the props. No emissive.
- **`SM_Barn_GateProps`:** against the end wall beside the gate, between two posts. Bind `Cream` to the boards' material; the vertex colour keeps the panels 26 to 36 % darker.

`out\fbx-ar4\MANIFEST.md` has every line in full.

## What I did not look at

- **The engine.** No import. Whether Unreal keeps the slot order and five UV channels on an imported LOD 1 is untested.
- **The switch itself:** no still of a figure changing level, and nothing in motion (pop, shimmer).
- **Other rink sizes.** On Pond the walls stand closer and lower (rig scale 0.667), so the windows drop to 6.5 m. Whether they then enter a game-camera frame is not checked.
- **The rig's real geometry.** Wall positions are B19's table. The rig's own 34 posts are stood in for by plain posts in both stills.
- **The stand line.** The mock-up uses 4 m beyond the boards (B41's sentence); I have not seen AR2's result.
- **The mock-up's light** is still AR1's assumption. The luma numbers compare with each other, not with the engine's.
- **The gate's bay.** The wall bay's cross-brace passes behind the gate's lintel there; I did not make a bay without it.

## Decisions for the operator

1. **The windows glow blue.** It is the brief's "dim blue night outside", and blue is also a team colour. At luma 29 and out of both game-camera frames I think it is safe. Say if they should be a colder grey instead.
2. **Two post rhythms.** The wall bays bring a post every 12 m; the rig has its own 34. Replace the rig's, or accept both.
3. **The machine's corner.** A corner stand piece in the gate's corner would hide the machine. Leave that one corner open?
4. **The loft:** over the gate only (two lengths, as shown), or along the whole end (six).
5. **How strict "under 2 luma" is.** If it means the figures' own pixels, the far figures would need a rounder head and body: my estimate is about 26 more triangles each, which takes them to about 28 % of the full figures, and I have not tried whether that is enough.

## How to rebuild

```
cd /c/Users/kazda/jam/arena-lab
bash run4.sh            # about 7 minutes: build (2 s), piece sheet (30 s), eleven views, twelve band strips and masks (320 s), manifest
bash run4.sh build | sheet | views | manifest | stills
```
The whole chain ran end to end at 02:36 to 02:41; the views and manifest once more at 02:42 to 02:48, to count the ice machine and the windows apart. A first run stopped at the labelling step (a colon in a caption broke ffmpeg); fixed.

## Outputs

- `C:\Users\kazda\jam\arena-lab\out\fbx-ar4\` : 32 FBX (9,192 triangles, 1.2 MB) and `MANIFEST.md`
- `C:\Users\kazda\jam\arena-lab\out\renders-ar4\` : `ar4-pieces.png`, `ar4-far-crowd.png`, `ar4-near-limit.png`, `ar4-gamecam-ab.png`, `ar4-upice-ab.png`, `ar4-low-ab.png` and the single stills
- `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-024131-B-ar4-pieces.png`, `-barn-corner.png`, `-barn-upice.png`, `-barn-sideboards.png`, `-far-crowd.png`, each with a `.txt` caption
- `C:\Users\kazda\jam\arena-lab\scripts\build4.py`, `render4.py`, `manifest4.py`, `C:\Users\kazda\jam\arena-lab\run4.sh`
- `C:\Users\kazda\jam\arena-lab\work\stats4.json`, `views4.json`, `views4-pass1.json`, `fbx4-readback.json`

## Next three evolutions, ranked

1. **The lamps hang from the roof.** The rig's lamp shades, key lights and bulb strings float under AR3's trusses. A kit of hangers (a chain or rod to the nearest chord, a bracket where a bulb string meets a post). The number: every lamp and string end in the rig meets timber within 0.3 m; under 1,500 triangles for the barn; ice mean luma unchanged within 1 %.
2. **The other end.** The end without the gate has only wall bays. A closed sliding barn door, a firewood stack and a bench with skates on pegs. The number: under 1,500 triangles, under 1 % of the up-ice frame at that end, the order of values kept.
3. **The machine comes out between periods.** A clay driver on the seat (one of the crowd figures), wheels as separate meshes with pivots, and a lap drawn as a path for the engine. The number: under 3,500 triangles with the driver; four wheel pivots that the engine can spin; zero frames of it on the ice while the puck is live. This one needs the operator's ruling first: it is motion on the playing surface, in a stoppage only.
