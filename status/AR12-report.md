# AR12 report: a cheer that reads, lighter benches, the scoreboard hung (device B, Blender only)

Lane AR12, device B, machine clock 05:47 to 06:20 (2026-11-14). Track: the arena and the clay look. Lab: `C:\Users\kazda\jam\arena-lab\`.

Housekeeping: no Unreal, no editor, no git write. Written in the repository: this report and six stills with captions in `devlog/shots/`. **`out\fbx\`, `out\fbx-ar3\`, `out\fbx-ar4\` and `out\fbx-ar6\` were read and not written**: all 137 checksums are the same before and after. CPU renders only (Cycles, 12 samples, 6 threads, 960 x 540). One Blender process of this lane at a time; I checked the process list before each start (another lane's Blender was running beside mine, never two of mine). I looked at AR7's four stills and AR9's close view before starting.

## Verdict

**All four items are built, exported and proved in Blender: 88 FBX in `out\fbx-ar12\`.** None is proved in the engine.

1. **The cheer: 5 % or more at 15 m and at 25 m, not at 40 m.** With half the house answering, the share of the frame that changes is 9.4 %, 5.8 % and 2.5 % at 15, 25 and 40 m, against 4.0 %, 2.5 % and 1.0 % for the cheer the engine has now: 2.3 to 2.4 times at every distance. At the engine's own 31 m pose that is about 4.5 % by interpolation, not rendered.
2. **The stands:** with new vertex colours the figures stand 11 luma further from the stands (18 to 29 apart). Nothing in the stands is brighter than the boards.
3. **The scoreboard hangs** under the centre truss with its lowest point 10.7 m over the ice, outside both game-camera frames, two flat panels in a slot of their own for the score.
4. **The wide and large arena:** a line per piece in the manifest; summarised below.

**My mock-up reproduces the engine's number for today's cheer:** 2.5 % at 25 m and 1.0 % at 40 m bracket AR7's 2.0 to 2.2 % at 31 m. That is why I trust the ratio.

## 1. A cheer that reads at the game camera

**What changed.** Only the cheering targets. Every seated mesh is AR6's file with new values in the pose channels the engine already blends by (UV2 to UV4), plus a second target in three new channels (UV5 to UV7).

- A seated figure now **stands up on its bench**: the figure's top goes from 1.3 to 1.65 m over the tread to 1.9 to 2.3 m (AR3's cheer reached 1.3 to 1.8 m). The one figure at the rail has no bench under it and reaches 1.7 to 1.9 m.
- A standing figure steps up onto the bench (0.42 m).
- Both arms go straight up and wide.
- **The scarf is held overhead**, stretched to 0.95 m between both hands, or **the hat is held high** in one hand, crown to the rink. Both are the figure's own scarf and hat, moved: no vertex is added.
- A vertex moves 35 to 77 cm in the mean (AR3's cheer: 11 to 31 cm).

**The proving numbers.** The mock-up is laid out as the engine's rink: 663 fans in rows 1 to 3, the scored half 324, the far level beyond 30 m. It is lit to crowd band 43 and ice 163.3 at the side boards. The camera is AR7's pose (3.5 m up, 17.65 degrees down) on the centre line.

| Distance to the end stand | The cheer now | AR12's cheer | Ratio | Fans' share of the frame | Ice mean luma: seated / now / AR12 |
|---|---|---|---|---|---|
| 15 m | 3.97 % | **9.37 %** | x 2.36 | 7.9 % | 161.78 / 161.78 / 161.82 |
| 25 m | 2.49 % | **5.82 %** | x 2.34 | 4.4 % | 164.70 / 164.70 / 164.75 |
| 40 m | 1.03 % | **2.45 %** | x 2.38 | 1.8 % | 167.02 / 167.02 / 167.06 |

(Share of the frame changed by more than 8 luma.)

- **Vertex counts and channels:** read back, 0 of the 28 seated files differ from AR6's in vertices, faces, UV0, UV1 or colour. Seated + UV2 to UV4 lands on `_Cheer` and seated + UV5 to UV7 on `_Cheer2` within 0.0001 cm. Every target has its seated mesh's vertex count.
- **The ice's mean** moves by 0.05 luma at most (0.03 %), and 0 ice pixels change by more than 8.
- **Why 40 m falls short:** the fans are 1.8 % of the frame there. A pose can change each of their pixels at most about twice.
- **One thing that makes my figures conservative:** in these end-on views my crowd band reads 18 to 30, darker than the engine's 36 to 45. A lighter band would pass the 8-luma threshold more often.
- The brightest 1 % of the cheering fans reads 124 at most, against boards at 145.

**The second target (so neighbours do not rise alike).** Each figure has a target B: where A stretches the scarf, B holds the hat up, and so on; the two youngsters jump 12 to 14 cm further in B. It needs one more value per instance (0 or 1). In the stills, every second fan uses B.

**The jump.** A sine on Z is enough; no channel is needed. Suggested: Z offset of 15 cm x cheer x |sin|, about 2.2 times a second, phase and strength from the per-instance random. It is not in my numbers, so it comes on top.

## 2. The stands, lighter where people are not

`SM_Stand_Mod`, `SM_Stand_End` and `SM_Stand_Corner` are AR1's files with only the vertex colour changed; sockets kept. Seat planks 1.00; the aisle's steps 0.92 and 0.56 in turn; front planks 0.58; treads 0.50; posts 0.60; supports and side boards 0.46; risers 0.40.

| At the side-boards game camera | AR1's colours | AR12's colours |
|---|---|---|
| Figures' mean luma | 54.5 | 54.2 |
| Stands' mean luma | 36.9 | 25.2 |
| **Figures against stands (difference of the means)** | 17.6 | **29.0 (+11.4; bar +10)** |
| A figure pixel against the stand pixels within 8 pixels of it | 27.0 | 27.3 |
| Stands' brightest pixel, empty | | 88.7 (boards 145.5) |
| Crowd band's mean | 43.0 | 35.3 |

- **The bar is met on the means.** Right at a figure's outline the contrast is unchanged (27.0 to 27.3): the seat plank behind a seated figure is still the lightest timber.
- **The band's mean drops by 8.** The tones only go down from today's planks, so the seats keep their value and the rest darkens. If the band is to stay at 40, the stand material's wash comes up after.
- **It needs one thing from the engine lane:** the stands' material must multiply its wood by the vertex colour. I do not know whether `M_ArenaWood` reads it.
- Empty, the benches and steps read as lines against darker treads; the effect is modest (luma spread 11.7 to 12.4).

## 3. The scoreboard, hung

`SM_Barn_ScoreboardHung` is a new mesh, not AR1's: AR1's had six cream panels and no slot for a score. 508 triangles (bar 1,200).

- **What it is:** a timber and tin box 1.0 x 4.0 x 1.7 m on four chains under a hanger beam, strapped with two iron straps to the truss's doubled bottom chord. A tin roof and tray, a plain cream end either side, a shaded lamp over each long face.
- **The score's slot is `ScoreFace`:** two flat quads, one per long face, 3.72 x 1.38 m, facing the two ends of the rink. Their UV0 is the whole 0 to 1 square, u left to right as read from outside. They are plain until the other device binds a score material.
- **Where:** under AR3's truss at centre ice (x = 0), origin on the eave plane like the truss bay and the lamp arms.
- **No part under 8 m:** lowest point 10.69 m over the ice on the regular rink.
- **Out of the game camera's frame:** 0 % from the side boards and 0 % from one end looking up the ice. From a low camera looking up it takes 1.5 % of the frame at mean luma 47.
- **Ice mean:** -0.02 % and -0.04 % with it hung.
- **Not on Pond as it is:** the eave is 9.0 m there, and a life-size board would hang to 6.2 m.

## 4. The wide and the large arena

| Piece | Kind | At a larger barn |
|---|---|---|
| `SM_Barn_WallBay` | per 12 m bay | repeat more often at X = 1; **Z must follow the wall's height** |
| `SM_Barn_Bunting` | per 12 m length | life size; more lengths |
| `SM_Barn_Loft` | per 12 m bay | life size |
| `SM_Barn_TrussBay` | per 12 m bay, but it spans the barn | repeat at X = 1; **Y and Z must follow the barn** (its timbers thicken with it) |
| `SM_Barn_LampArm_*` | single, one per lamp | follows its truss's Z; stretch along X to the real gap to the nearest truss |
| `SM_Barn_IceGate`, `SM_Barn_SlidingDoor` | single | **counter-scale to 1** |
| `SM_Barn_IceMachine`, `SM_Barn_GateProps`, `SM_Barn_Firewood`, `SM_Barn_SkateBench` | single | **counter-scale to 1** (at 1.56 a hockey stick is 2.2 m long) |
| `SM_Barn_ScoreboardHung`, `SM_BarnScoreboard` | single | **counter-scale to 1**; the hanging point follows the truss |

## What the engine lane needs to know

- **The cheer:** reimport the 28 seated files over AR6's. The blend by UV2 to UV4 then leads to the new pose with no other change.
- **Bounds:** a vertex now moves up to 168 cm up, and the figure grows 69 cm to each side and 37 cm towards the rink. AR5 allowed 95 and 55.
- **Target B:** offset = lerp(UV2 to UV4, UV5 to UV7, a per-instance 0 or 1).
- **Eight UV channels** is Unreal's limit for a static mesh, and these files use all eight.
- **Half way through the blend** the hat is in the air and the scarf half stretched. Quick, it reads as a throw; held at 0.5 it would not.
- **The stands:** reimport the three; make the material multiply by the vertex colour.
- **The scoreboard:** place at (0, 0, eave), yaw 0; bind `ScoreFace` to the boards' cream until the score exists; `LampGlow` low; no shadow.

`out\fbx-ar12\MANIFEST.md` has every line.

## What I did not look at

- **The engine.** No import. Whether eight UV channels survive the import and the instanced path is untested.
- **The cheer at the engine's exact 31 m pose** (interpolated), from the side boards, or with the jump.
- **Fans standing on benches against the row behind:** a risen fan's head is level with the seated heads one row up. In the stills this reads as a crowd on its feet; I did not check it close.
- **Arms through neighbours:** raised hands and scarves reach 69 cm to the side, past the 58 cm seat pitch.
- **Motion:** the blend itself, shimmer, the level switch during a cheer.
- **The row meshes' new targets in a still:** they are built and checked, and on no view.
- **Pond, wide and large:** section 4 is arithmetic.

## Decisions for the operator

1. **How big the cheer should be.** This one fills the top of the frame with scarves and hats at 15 to 25 m. If that is too much for "calm behind the boards", the blend's strength scales it: 0.6 of the pose is still on the bench with arms up.
2. **Hats off and scarves up.** They are what makes the difference at distance. Say if either reads wrong for this crowd.
3. **Darker stands.** The benches now read, and the crowd band's mean drops by 8 unless the wood's wash is lifted. Keep the band at 40, or let the stands recede.
4. **The scoreboard on Pond:** leave it out, or have a flat one built that sits between the chords.

## How to rebuild

```
cd /c/Users/kazda/jam/arena-lab
bash run12.sh            # about 10 minutes: build (4 s), twenty views and the masks (about 480 s), piece sheet (50 s), labels, manifest
bash run12.sh build | views | sheet | stills | manifest | tune
```
The whole chain ran end to end at 06:09 to 06:18, after a first views run (05:57 to 06:08) that showed the end stands under-lit and the stands' contrast 0.8 short of the bar.

## Outputs

- `C:\Users\kazda\jam\arena-lab\out\fbx-ar12\` : 88 FBX (6.7 MB) and `MANIFEST.md`
- `C:\Users\kazda\jam\arena-lab\out\renders-ar12\` : `ar12-pieces.png`, `ar12-cheer-15.png`, `ar12-cheer-25.png`, `ar12-cheer-40.png`, `ar12-stands-crowd-ab.png`, `ar12-stands-empty-ab.png`, `ar12-score-low-ab.png`, `ar12-score-gamecams.png` and the single stills
- `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-061848-B-ar12-cheer-25m.png`, `-cheer-15m.png`, `-cheer-40m.png`, `-stands.png`, `-pieces.png`, `-scoreboard.png`, each with a `.txt` caption
- `C:\Users\kazda\jam\arena-lab\scripts\build12.py`, `render12.py`, `manifest12.py`, `C:\Users\kazda\jam\arena-lab\run12.sh`
- `C:\Users\kazda\jam\arena-lab\work\stats12.json`, `views12.json`, `fbx12-readback.json`

## Next three evolutions, ranked

1. **The cheer as a wave, not a switch.** Give each seat a delay from its distance to the scored net, so the stand rises from the goal outward over about 20 frames and sits down the same way. One float per instance, set at build time. The number: at no frame of the 108 does more than 60 % of the scored half share the same blend value; the peak share of the frame changed stays at 5 % or more at 25 m.
2. **A life-size roof for the wide arena.** A truss that spans 115 m with life-size timbers (a queen-post frame on two rows of posts beside the stands), so the bays repeat instead of stretching. The number: no member thicker than 0.9 m at the wide arena, under 900 triangles a bay, nothing lower than 7 m over the ice, ice mean unchanged within 1 %.
3. **The scoreboard for Pond.** A flat board that sits between the chords of the centre truss. The number: no part under 8 m at rig scale 0.667, the same `ScoreFace` slot and UVs, under 600 triangles.
