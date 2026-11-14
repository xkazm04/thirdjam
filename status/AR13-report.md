# AR13 report: the bigger cheer, a wave from the goal, the scoreboard hung, the stands' new colours (device B, no GPU)

Lane AR13, device B, machine clock 06:21 to 06:38 (2026-11-14). Tree `C:\Users\kazda\jam\wt\b-arena` at bcb81e3e. Two renderer-less editor processes of mine on port 8008 (`-nullrhi -unattended`, 0 `LogD3D12RHI` lines in both): pid 38408 did the work (4 play sessions) and pid 26696 widened the figures' bounds after I read my own numbers again. Both left through `ExitEditor` with nothing unsaved and are confirmed gone (06:29:46 and 06:32:12). No GPU editor, no commit, no push, no cook, no build. Console variables restored. 0 warnings naming B's arena, 0 Blueprint runtime errors, 0 `AttachTo` warnings. 60 and 59.9 ticks per wall second in the two goal tests.

**Nothing was seen, and the two masters are not changed:** a master cannot be compiled here. Their three changes are one script, `status/AR13-master.py`, for the GPU.

## Verdicts

| # | Job | Verdict |
|---|---|---|
| 1 | The bigger cheer | **DONE for everything but the look.** 12 figures and their far levels from AR12. **All eight UV channels survive**: in the source of both levels and in the built render data of both levels. Bounds raised, the far level's screen size set again |
| 2 | Target B | **Data done, blend in the script.** A sixth float per fan (index 5): half the fans on B, 97 % of them beside a nearest neighbour on the other target |
| 3 | The wave and the bounce | **Wave: data and Blueprint done, proven by the formula; bounce in the script.** The scored half rises from the net outward over 20 frames, each fan in 8; all at rest at frame 108. Your 60 % bar holds from frame 9 to 101 and not in the first 8 and last 6 frames, where the shared value is rest: that is the wave not yet arrived |
| 4 | The scoreboard hung | **DONE by numbers** on the 80 m, regular, large and wide rink (lowest point 9.2 to 18.2 m); left out on Pond, the 64 m and the 70 m rink |
| 5 | The stands' darker treads | **Meshes in; they do nothing yet.** `Arena/M_ArenaWood` does not read the vertex colour today: that is the script's third part |
| 6 | The film section | **DONE**: `status/AR13-film.md` and the script it calls |

## 1. The bigger cheer

- `SM_Fan_A` to `_L` reimported from `fbx-ar12`, each with its `_L1` as LOD 1 again. Triangles, slots and their order as before (520 to 620 full, 126 to 146 far).
- **Eight UV channels on both levels of all twelve**, read two ways: from the source model, and from the built render data (the buffers an instanced component draws from): 8 and 8. Full-precision UVs on both levels.
- **The offsets are in them** (largest per figure, read from the meshes):

| Figure | Full level: pose A / pose B, largest move (cm) | Far level: A / B | Up, to the side, toward the rink (full level, cm) |
|---|---|---|---|
| A | 142.7 / 142.0 | 153.3 / 151.9 | 138.2, 59.0, 33.5 |
| B | 121.1 / 120.6 | 124.8 / 122.6 | 118.3, 59.2, 40.1 |
| C | 137.8 / 142.8 | 148.7 / 154.8 | 141.5, 59.1, 32.1 |
| D | 84.2 / 87.5 | 91.0 / 90.6 | 76.5, 58.9, 1.2 |
| E | 140.5 / 140.5 | 151.2 / 151.2 | 137.6, 91.6, 33.0 |
| F | 90.1 / 107.3 | 90.0 / 108.3 | 107.0, 44.8, 20.8 |
| G | 143.8 / 140.7 | 153.8 / 150.6 | 129.4, 70.9, 52.2 |
| H | 144.5 / 148.7 | 155.4 / 159.7 | 144.7, 65.6, 16.7 |
| I | 138.4 / 137.8 | 147.3 / 147.3 | 131.8, 53.4, 17.2 |
| J | 125.4 / 130.7 | 134.9 / 140.4 | 124.8, 53.8, 15.2 |
| K | 151.3 / 156.1 | 163.6 / 168.2 | 156.0, 32.7, 22.5 |
| L | 109.2 / 130.7 | 112.1 / 126.5 | 123.3, 51.5, 8.2 |

- **AR12's "69 cm to each side" is exceeded by figure E** (its mug arm moves 91.6 cm sideways on the full level, 80.8 on the far level) and by G's far level (81.4). So I set the bounds to **172 cm up and 95 cm to each side** (60 cm toward the rink), not AR12's 168 and 69. For the Blender lane: say if E is meant to reach that far over a 58 cm seat pitch.
- The far level's screen size was set again for a switch at 30 m (0.1775 to 0.2000 per figure; the bounds' radius grew).
- Whether the instanced shader reads coordinates 5 to 7 on screen is step 0 of the film script.
- Not re-read: the vertex colour's alpha parts on AR12's files (AR12 says only the UVs changed; I checked the alpha on AR6's).

## 2 and 3. Target B, the wave, the bounce

**Per-instance data on the clay rig's fans, now six floats** (I added one; nothing is packed into another): 0 weight, 1 coat value, 2 accent value, 3 end, **4 delay** (new meaning: 0 at the scored net to 1 at the far corner, by the fan's rank in its half so the wave takes the same number of fans each frame, with 0.04 of jitter), **5 target** (0 A, 1 B; each fan takes the other target than its nearest placed neighbour).

| Layout | Fans = instances | On target B | Nearest neighbour on the other target |
|---|---:|---:|---:|
| 60 m | 491 | 49.5 % | 96.5 % |
| 90 m | 664 | 50.0 % | 97.3 % |
| 120 m | 830 | 50.2 % | 96.9 % |
| 140 m | 944 | 50.7 % | 97.4 % |

(64, 70 and 80 m: 491, 521 and 612 fans, 49 % on B, 97 to 98 %.) The seats and figures are the ones of AR5 (same seed); only the data changed.

**The Blueprint** (`BP_ArenaCrowd`, changed with the DSL tool in one write): the end's scalar now rises over **28 frames** instead of 8, holds, and falls from frame 88 to 108 as before.

**The formula the script puts into the master:** amount = smoothstep(saturate(E x (1 + `WaveK`) - `WaveK` x delay)) x weight x `PoseK`, with `WaveK` 2.5: every fan rises in 8 frames, started between frame 0 and 20 by its delay. The bounce: `BounceCm` (15) x amount x |sin(2 pi (2.2 t + phase))| straight up, the phase from the fan's coat value.

**Proof**, headless on the regular rink: a `goal` on the bus at +x, the scalar read every tick from the real Blueprint, the formula above evaluated on each fan's own data:

| Frame after the goal | 1 | 2 | 4 | 8 | 14 | 20 | 28 | 60 | 88 | 96 | 100 | 104 | 107 | 108 | 130 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| `CrowdCheerPlusX` | 0.036 | 0.071 | 0.143 | 0.286 | 0.5 | 0.714 | 1 | 1 | 1 | 0.6 | 0.4 | 0.2 | 0.05 | 0 | 0 |
| Fans of the +x half that have started (of 332) | | 31 | | 131 | 230 | 327 | 332 | 332 | 332 | | | | | 0 | 0 |
| Share of the half still at rest | 96 % | | 80 % | 61 % | 31 % | 2 % | 0 | 0 | 0 | 17 % | 44 % | 74 % | 94 % | 100 % | 100 % |
| Largest share on one blend value (bins of 0.02) | 98 % | 94 % | 83 % | 63 % | 34 % | 9 % | 11 % | 11 % | 11 % | 20 % | 48 % | 76 % | 97 % | at rest | at rest |

- **Your bar (no more than 60 % of the scored half on one blend value): met from frame 9 to frame 101.** In frames 1 to 8 and 102 to 107 the largest group is the fans the wave has not reached yet, or has already left: they share the value 0. A wave that takes 20 frames cannot have it otherwise; say if the bar meant something else.
- **At rest by frame 108** (your bar: 130). The -x half: 0 fans at every frame. A goal at -x mirrors it (31, 132, 230, 328, 332 fans; 0 of the +x half).
- The switch still works on this rig (`ke * ArenaCrowd 1`: 0 of 12 fan components visible; `0`: hidden; `2`: back).
- **Until the script has run** the old master is in force with the new data: the pose is AR12's A for every fan, the rise takes up to 28 frames with a shorter wave (a fan starts when E passes delay / (1 + delay)), no bounce. It works; it is not the design.

## 4. The scoreboard

`SM_Barn_ScoreboardHung` (508 triangles; it hangs 2.81 m under its origin) at (0, 0, eave) under the centre truss, at real size, as an instance on each rink's crowd owner, so the switch hides it with the rest. `ScoreFace` and `Cream` are the boards' cream (`MI_ClayCrowd_Cream`), the lamps a new low warm instance `Arena/MI_Barn_ScoreLamp`.

| Rink length | Scoreboard | Lowest point | In the clear zones under 7 m / meeting a stand |
|---|---|---:|---|
| 60, 64, 70 m | left out | (it would hang to 6.2, 6.8, 7.7 m) | |
| 80 m | 1 | 9.19 m | not run |
| 90 m | 1 | 10.69 m | 0 / 0 |
| 120 m | 1 | 15.19 m | 0 / 0 |
| 140 m | 1 | 18.19 m | 0 / 0 |

Fans, stands, clear zones and device A's pieces on the four rinks measured: unchanged (664 / 56 on the regular rink, 0 bounds in the zones, 0 meeting).

## 5. The stands

The three stand meshes are reimported with AR12's vertex colours (same triangles, slots and bounds). **`Arena/M_ArenaWood` has no vertex colour node among its 30 expressions**, so the darker treads do not show. The script multiplies the base colour by the vertex colour (decoded with power 2.2, as the crowd's is) and feeds the same product to the two Custom nodes that build the wash and the floor spill from the base. It will also darken whatever else on that master carries vertex colours: the barn's timber pieces, as the Blender lanes meant.

## 6. The script and the film section

- `status/AR13-master.py`: one call. I ran it in my editor on scratch copies of both masters with the compile switched off: it answered `built; 26 old nodes removed` and `built; base colour and 2 custom input(s) now read base x vertex colour: B26BarnWash.Base, B36FloorSpill.Base`, the graphs read back as written, a second run answered `already done` twice, and the copies were deleted unsaved. Stock nodes only; no Custom node added.
- `status/AR13-film.md`: the call, then the cheer at 15, 25 and 40 m, the risen fans against the row behind and the arms, the wave, the level switch during a cheer, the bounce, the scoreboard, the readability pairs, and the four values to set.

## Which files belong to which state

All in `C:\Users\kazda\jam\wt\b-arena`. **AR13 is the uncommitted difference in the tree**; job 2 (ea90e4ad) and AR11's night rig (bcb81e3e) are the commits under it and are not touched by it, except that they share files:

- **AR13 changed:** `Arena/SM_Fan_A.uasset` to `SM_Fan_L.uasset` (12), `Arena/SM_Stand_Mod.uasset`, `SM_Stand_End.uasset`, `SM_Stand_Corner.uasset`, `Arena/BP_ArenaCrowd.uasset`, `Rigs/LR_P3_Clay.umap`; **new:** `Arena/SM_Barn_ScoreboardHung.uasset`, `Arena/MI_Barn_ScoreLamp.uasset`, `status/AR13-master.py`, `status/AR13-film.md`, `status/AR13-report.md`.
- **Job 2's** clay rig is the committed one; AR13's rig is that rig with the fans' data rewritten and the scoreboard added, so AR13 cannot merge before job 2.
- **AR11's night rig (`Rigs/LR_L2_Night.umap`) is not changed.** It shares the figures and the Blueprint, so on night the pose is the bigger one and the rise takes 28 frames; its fans keep five floats (target A, the old random delay). To give night the wave is the same rig script on `LR_L2_Night`, after the GPU has seen AR11.
- Copies of the masters, the Blueprint and the rig as they were are in `scratchpad\ar13\bk`.

## Not verified

- The compile of the two masters and everything on screen: the size of the cheer, target B, the wave as motion, the bounce, hats in the air half way, arms through neighbours, risen fans against the row behind, culling, the level switch during a cheer, the scoreboard, the stands' new colours and the band's luma after them.
- Readability with a risen stand.
- The 64, 70 and 80 m rinks in play; night with the new figures.
- Frame time; the cost of eight UV channels at full precision on 664 to 944 instances (the fan meshes grew from about 138,000 to about 152,000 bytes each).
- A cook.

## For the other device

Nothing now. When AR13 merges: the cheer's scalar rises over 28 frames, not 8 (your goal light and caps run their own clocks; say if they should meet), and a board hangs over centre ice with a face waiting for a score: slot `ScoreFace` of `SM_Barn_ScoreboardHung`, on the live `BP_ArenaCrowd` actor's component `ScoreboardHung`.

## Decisions for the operator

1. **How big the cheer** (AR12's first): `PoseK` 1.0, or 0.6 for arms up on the bench.
2. **A wave from the goal, or everyone at once:** `WaveK` 2.5 or 0.
3. **The bounce:** 15 cm at 2.2 a second, or none.
4. **The scoreboard on Pond:** left out now; a flat one would have to be built.
5. **Darker stands:** keep the band's mean by lifting the wash, or let the stands recede.

## The next three evolutions, each with a proving number

1. **The GPU pass of `AR13-film.md`:** the cheer changing at least 9, 5 and 2 % of the frame at 15, 25 and 40 m; ice within 1 % and puck and jerseys within 3 with a risen stand; 0 fans vanishing at the frame's edge.
2. **The wave on night and on the builder:** the same data on `LR_L2_Night` (554 fans on the regular rink, 60 % bar from frame 9 to 101) and in `BP_ArenaCrowdOne`'s build.
3. **A score on the board** (with the other device): the face reads a number at the low camera, luma 60 to 90, 0 % of the game camera's frame in normal play.

## Ledger rows (11 columns)

Nineteen rows: seventeen replace the rows of the same path (twelve figures, three stands, the Blueprint, the clay rig as job 2 left it), two are new.

```
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_A.uasset | mesh | 0.15 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, seated, hands on knees, 608 tris and a far level (LOD 1) of 146 tris switching at about 30 m (screen size 0.1496), slots ClaySkin, ClayCoat, Scarf; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5); since AR13 lane AR12 files: eight UV channels on both levels (UV2 to UV4 the cheering pose A, UV5 to UV7 pose B), bounds raised 172 cm up and 95 cm to the sides, the far level screen size set again for 30 m (lane AR13) (152287 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_B.uasset | mesh | 0.15 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, seated, both arms up, 620 tris and a far level (LOD 1) of 138 tris switching at about 30 m (screen size 0.1562), slots ClaySkin, ClayCoat, Scarf; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5); since AR13 lane AR12 files: eight UV channels on both levels (UV2 to UV4 the cheering pose A, UV5 to UV7 pose B), bounds raised 172 cm up and 95 cm to the sides, the far level screen size set again for 30 m (lane AR13) (153531 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_C.uasset | mesh | 0.15 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, standing, one fist up, 540 tris and a far level (LOD 1) of 126 tris switching at about 30 m (screen size 0.147), slots ClaySkin, ClayCoat, Scarf; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5); since AR13 lane AR12 files: eight UV channels on both levels (UV2 to UV4 the cheering pose A, UV5 to UV7 pose B), bounds raised 172 cm up and 95 cm to the sides, the far level screen size set again for 30 m (lane AR13) (145424 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_D.uasset | mesh | 0.14 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, standing, leaning on a rail, 544 tris and a far level (LOD 1) of 126 tris switching at about 30 m (screen size 0.1504), slots ClaySkin, ClayCoat, Scarf; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5); since AR13 lane AR12 files: eight UV channels on both levels (UV2 to UV4 the cheering pose A, UV5 to UV7 pose B), bounds raised 172 cm up and 95 cm to the sides, the far level screen size set again for 30 m (lane AR13) (144992 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_E.uasset | mesh | 0.15 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, standing with a mug, 572 tris and a far level (LOD 1) of 134 tris switching at about 30 m (screen size 0.1464), slots Tin, ClaySkin, ClayCoat, Scarf; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5); since AR13 lane AR12 files: eight UV channels on both levels (UV2 to UV4 the cheering pose A, UV5 to UV7 pose B), bounds raised 172 cm up and 95 cm to the sides, the far level screen size set again for 30 m (lane AR13) (146038 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_F.uasset | mesh | 0.15 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, seated child, hands at the mouth, 604 tris and a far level (LOD 1) of 138 tris switching at about 30 m (screen size 0.1345), slots ClaySkin, ClayCoat, Scarf; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5); since AR13 lane AR12 files: eight UV channels on both levels (UV2 to UV4 the cheering pose A, UV5 to UV7 pose B), bounds raised 172 cm up and 95 cm to the sides, the far level screen size set again for 30 m (lane AR13) (146512 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_G.uasset | mesh | 0.15 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, seated, broad, arms crossed, tall floppy hat, 608 tris and a far level (LOD 1) of 138 tris switching at about 30 m (screen size 0.156), slots ClaySkin, ClayCoat; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5); since AR13 lane AR12 files: eight UV channels on both levels (UV2 to UV4 the cheering pose A, UV5 to UV7 pose B), bounds raised 172 cm up and 95 cm to the sides, the far level screen size set again for 30 m (lane AR13) (150742 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_H.uasset | mesh | 0.14 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, seated, slim and tall, hand at the chin, flat cap, 520 tris and a far level (LOD 1) of 126 tris switching at about 30 m (screen size 0.1462), slots ClaySkin, ClayCoat; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5); since AR13 lane AR12 files: eight UV channels on both levels (UV2 to UV4 the cheering pose A, UV5 to UV7 pose B), bounds raised 172 cm up and 95 cm to the sides, the far level screen size set again for 30 m (lane AR13) (142676 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_I.uasset | mesh | 0.15 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, seated, hands in the lap, hood up, 588 tris and a far level (LOD 1) of 138 tris switching at about 30 m (screen size 0.1451), slots ClaySkin, ClayCoat; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5); since AR13 lane AR12 files: eight UV channels on both levels (UV2 to UV4 the cheering pose A, UV5 to UV7 pose B), bounds raised 172 cm up and 95 cm to the sides, the far level screen size set again for 30 m (lane AR13) (148635 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_J.uasset | mesh | 0.15 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, seated, small, pointing at the ice, ear-flap hat, 620 tris and a far level (LOD 1) of 138 tris switching at about 30 m (screen size 0.1408), slots ClaySkin, ClayCoat; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5); since AR13 lane AR12 files: eight UV channels on both levels (UV2 to UV4 the cheering pose A, UV5 to UV7 pose B), bounds raised 172 cm up and 95 cm to the sides, the far level screen size set again for 30 m (lane AR13) (152592 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_K.uasset | mesh | 0.15 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, seated, turned to a neighbour, bare head with a bun, 596 tris and a far level (LOD 1) of 138 tris switching at about 30 m (screen size 0.1482), slots ClaySkin, ClayCoat; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5); since AR13 lane AR12 files: eight UV channels on both levels (UV2 to UV4 the cheering pose A, UV5 to UV7 pose B), bounds raised 172 cm up and 95 cm to the sides, the far level screen size set again for 30 m (lane AR13) (154188 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_L.uasset | mesh | 0.15 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, seated youngster, elbows on knees, 608 tris and a far level (LOD 1) of 138 tris switching at about 30 m (screen size 0.136), slots ClaySkin, ClayCoat; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5); since AR13 lane AR12 files: eight UV channels on both levels (UV2 to UV4 the cheering pose A, UV5 to UV7 pose B), bounds raised 172 cm up and 95 cm to the sides, the far level screen size set again for 30 m (lane AR13) (145338 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Stand_Mod.uasset | mesh | 0.13 | original | Blender 4 script (lane AR1, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | timber bleacher module, 4 rows, 4.89 x 4.01 x 3.99 m, 924 tris, slots Timber and TimberDark; no collision, vertex colours, materials bound to the slots (lane AR2); since AR13 lane AR12 vertex colours (darker treads), which show once M_ArenaWood multiplies by the vertex colour (lane AR13) (133232 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Stand_End.uasset | mesh | 0.11 | original | Blender 4 script (lane AR1, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | standing terrace for behind the nets, 2 levels, 4.89 x 2.40 x 2.25 m, 504 tris, slots Timber and TimberDark; no collision, vertex colours, materials bound to the slots (lane AR2); since AR13 lane AR12 vertex colours (darker treads), which show once M_ArenaWood multiplies by the vertex colour (lane AR13) (105928 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Stand_Corner.uasset | mesh | 0.14 | original | Blender 4 script (lane AR1, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | timber bleacher quarter turn, inner radius 6 m, 1344 tris, slots Timber and TimberDark; no collision, vertex colours, materials bound to the slots (lane AR2); since AR13 lane AR12 vertex colours (darker treads), which show once M_ArenaWood multiplies by the vertex colour (lane AR13) (141041 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Barn_ScoreboardHung.uasset | mesh | 0.10 | original | Blender 4 script (lane AR12, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | a scoreboard hung under a truss, 2.12 x 4.40 x 3.71 m, hanging 2.81 m under its origin, 508 tris, slots Timber, TimberDark, Tin, Cream, LampGlow, ScoreFace (the face bound to the boards cream until a score is drawn); no collision, vertex colours (lane AR13) (101335 bytes) |
| Game/Plugins/JamAssets/Content/Arena/MI_Barn_ScoreLamp.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_ClayCrowd for the hung scoreboard lamps (slot LampGlow): Tint (1.0, 0.55, 0.25), FillK 1.5, FillLow 1.0, PoseK 0; a low warm glow, unseen (lane AR13) (6068 bytes) |
| Game/Plugins/JamAssets/Content/Arena/BP_ArenaCrowd.uasset | other | 0.19 | original | UE 5.9 editor via MCP (Blueprint DSL) |  |  | 2026-11-14 | B | y | owner actor of one rink size crowd and stands (instanced components added per actor in the rig): at BeginPlay it destroys itself unless its world scale is 1 within 0.02 (the rig scale times its own 90 / length), else binds JamEventBus.OnJamEvent; on goal it notes the time for the end whose net was scored (Where.X sign) and every tick writes MPC_Crowd CrowdCheerPlusX / CrowdCheerMinusX: up over 8 frames, held, down over frames 88 to 108 (lane AR5); since AR8 a mode switch: the custom event ArenaCrowd(NewMode), called by the console line "ke * ArenaCrowd 0", "... 1" or "... 2" in builds that are not shipping builds: 0 hides the owner and every actor tagged B_ArenaDress, 1 hides the components tagged B_Fan, 2 shows everything; it reads no console variable (two unused variables Mode, HasVar are kept for that) (lanes AR5, AR8); since AR13 the scalar rises over 28 frames instead of 8, so that a wave has 20 frames to travel (lane AR13) (187551 bytes) |
| Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap | other | 1.64 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | P3 clay rig, barn with calm ice: PPV (EV100 4.2, SSR override, focus 16 m f/4, no depth blur, motion blur 0), sky light a movable real-time capture (no cubemap; intensity 0.35 and colour unchanged) of the capture dome SkyCap_P3_Clay (engine sphere, scale 700, MI_SkyAmbient_Clay, outside the barn; B34), 4 shadowed warm key spots (5200 cd, no specular, shadow resolution 1.0 and source radius 60 cm since B34), 8 shadowless lamps over the side lanes (2100 cd, broad beams, no specular; B28), 6 shadowless fills at lower intensity (B28), 18 lamp shades (6 of them decorative beyond the end boards, twice as wide since B29), LookParams (FeltRim 1.4); plank walls, posts, trusses, roof, floor; no bulb strings; all shadowless meshes, no collision; the old ten trusses replaced by ten roof bays (SM_Barn_TrussBay, height scale 0.82), 32 wall bays in front of the plank walls and 12 lengths of bunting hung at 13.05 m, all instances on one actor Dress_P3_Clay that scales with the rig; timber stands and a clay crowd as instances on seven actors Crowd_P3_Clay_k0667, k0711, k0778, k0889, k1000, k1333, k1556 (class BP_ArenaCrowd, one layout in real size per rink length 60, 64, 70, 80, 90, 120, 140 m, each scaled by 90 / length so that only the one whose rink is loaded has scale 1 and stays; the others destroy themselves at BeginPlay): on the regular rink 56 stand pieces and 664 fans (twelve figures with a far level, per-instance cheer weight, coat value, accent value, end and delay, front row empty, corner rows three quarters full), the ice gate, its props, a loft of two bays and the ice machine at the +x end wall; no light added, no shadows, no collision (lanes B9, B14, B18, B19, B22, B23, B27, B28, B29, B34, AR2, AR5); job 2 of AR8: 18 lamp arms on Dress_P3_Clay (6 SM_Barn_LampArm_075, 12 SM_Barn_LampArm_225, placed from the rig real lamp shades, each on its lamp rod) and a sliding door, a firewood stack and a skate bench on the -x end wall on each of the seven crowd owners (lane AR8 job 2); since AR13 six floats per fan (0 weight, 1 coat, 2 accent, 3 end, 4 delay by the distance to the end net, 5 target A or B) and the hung scoreboard on the owners of the 80, 90, 120 and 140 m rinks (lane AR13) (1635486 bytes) |
```
