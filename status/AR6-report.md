# AR6 report: a crowd that reads as people, the lamps hung from the roof, the other end (device B, Blender only)

Lane AR6, device B, machine clock 02:53 to 03:24 (2026-11-14). Track: the arena and the clay look. Lab: `C:\Users\kazda\jam\arena-lab\`.

Housekeeping: no Unreal, no editor, no git write. Written in the repository: this report and five stills with captions in `devlog/shots/`. **`out\fbx\`, `out\fbx-ar3\` and `out\fbx-ar4\` were read and not written**: all 75 checksums are the same before and after. CPU renders only (Cycles, 12 samples, 6 threads, 960 x 540), one Blender process at a time. I looked at AR2's four engine stills before starting.

## Verdict

**All three evolutions are built, exported and proved in Blender: 62 FBX in `out\fbx-ar6\`.** None is proved in the engine.

1. **The crowd reads as people.** All 56 crowd meshes are the same files with only the vertex colour changed. The spread of luma from head to head is 2.3 times what it was; no figure pixel is as colourful as a jersey; the cheer now changes 5.4 % of the frame. Two numbers are short of clean: 0.3 % of figure pixels are brighter than the boards' mean, and by HSV saturation (not by chroma) 0.4 % of figure pixels pass the jerseys.
2. **The lamps hang from the roof.** 18 timber arms, 1,440 triangles, every rod held with no gap, by positions that are partly assumed. From the ice they hardly show: 0.08 % of the low camera's frame.
3. **The other end has a door.** A sliding barn door with a hoist, a firewood stack and a skate bench: 1,314 triangles, 0.64 % of the up-ice frame. Only the door and hoist show over the end stands; the firewood and the bench are hidden behind them in play.

## Evolution 1: a crowd that reads as people

**What changed.** Vertex colours only. Hats, scarves and mittens are now light and varied (cream, straw, pale grey-blue, faded mustard, dusty rose, sage, oat, fog); coats run from charcoal and umber to cream; a few hats are dark on purpose, over light coats. No colour is near the teams' blue or orange.

**What did not change.** Vertices, faces, UV0, UV1, the three pose channels, slots, pivots and names. Read back, 0 of the 56 files differ from their AR3 or AR4 file in anything but colour (largest position difference 0.00003 cm). **Reimport them over the existing assets.**

**The proving numbers.** The mock-up is lit to AR2's engine measure: crowd band 48.3, ice 163.6 at the game camera.

| | Tonight's colours | AR6's colours | Bar |
|---|---|---|---|
| Standard deviation of luma across heads (hat and face), 82 heads at the game camera | 10.6 | **24.8 (x 2.3)** | at least doubled |
| The same in a framing like AR2's play still, 100 heads | 8.8 | **23.2 (x 2.6)** | |
| Mean luma of those heads; darkest to lightest | 52.9; 32 to 78 | 80.4; 41 to 126 | |
| Share of figure pixels more colourful than the jersey blocks (chroma) | | **0.000 %** | 0 |
| The same by HSV saturation | | 0.375 % | 0 |
| The cheer: share of the frame changed by more than 8 luma | 4.2 % | **5.4 %** | aim 5 % or more |

- **Saturation, plainly.** By chroma (how colourful a pixel looks: the largest channel minus the smallest) the figures reach 91 at most; the jersey blocks sit at 105 and 117. By HSV saturation 0.4 % of figure pixels pass the jerseys' 0.78: these are near-black pixels, where HSV saturation can be anything. The engine's own dark timber reads 0.9 by that measure in AR2's still.
- **The cheer.** The figures are 7.2 % of this frame; 55 % of their pixels change. The ice does not: mean 163.59 seated and cheering, 0 ice pixels changed. In AR3's darker still the same cheer changed 3.1 %.
- **Against the boards.** The figures' 99th percentile is 133 against boards at 147 and ice at 164. **0.29 % of figure pixels are over the boards' mean** (brightest 174): the tops of the lightest hats nearest the fill. If the rule is strict, the engine's crowd material needs a cap, or the lightest three clays come down a step.
- **Against the jerseys.** The figures' mean luma is 58; the blue jersey block's is 56 and the orange 93. The crowd is now about as light as a blue jersey on average. It stays far less colourful, and it is behind the boards.

**How I calibrated.** I measured regions of AR2's play still: ice about 176 there (AR2 reports 165 over its frames), boards 156 to 178, the stands' band 23 to 28. My mock-up's boards read 147, a little under the engine's. The crowd's light is a neutral fill that reaches only stands and crowd, to stand in for the engine's material fill, which keeps each colour's hue. An earlier pass with a warm light turned the cream hats orange and failed the saturation bar; that was the light, not the colours.

**The per-instance tint.** The colour's alpha now says what a vertex is:

| Alpha | Part |
|---|---|
| 1.0 | coat (body and upper arms) |
| 0.8 | hat, hood, hair, bobble |
| 0.6 | scarf or wrap |
| 0.4 | mittens (a far figure's forearm) |
| 0.2 | skin |
| 0.0 | trousers, boots, eye dots and mouth, the tin mug |

Suggested instance custom data: `CustomData0` the cheer (0 to 1); `CustomData1` a coat value of 0.6 to 1.3 on alpha 1.0; `CustomData2` an accent value of 0.8 to 1.15 on alpha 0.4 to 0.8. Skin and the alpha-0 parts take none. I did not make a second colour set: a static mesh imports one, and the alpha mask does the same job without another channel.

## Evolution 2: the lamps hang from the roof

| Mesh | What | Triangles (budget 90 each) |
|---|---|---|
| `SM_Barn_LampArm_075` | a timber arm on a truss's bottom chord reaching 0.75 m to the lamp's rod; an iron strap, an iron plate round the rod, three chain links | 72 |
| `SM_Barn_LampArm_150` | the same, reach 1.5 m, with a stay | 84 |
| `SM_Barn_LampArm_225` | the same, reach 2.25 m, with a stay | 84 |

**The proving numbers**

- **Every lamp meets timber: worst gap 0.00 m** (bar 0.3) over 18 lamps, by construction from the positions below.
- **1,440 triangles for the barn** (bar: under 1,500).
- **Ice mean luma unchanged:** -0.006 % at most across three views.
- From the game cameras the arms are not in frame. From the low camera they take 0.08 % of the frame at mean luma 18. Looking straight up from the ice, the chord and the shade hide most of each arm.

**What I assumed, where the reports give no number**

- **Given (B28):** 4 keys at x +-11.25 and +-33.75, y 0, 13 m; 8 side lamps at the same x, y +-13.5, 11.5 m. AR3's trusses stand at x = -60 + 12 k, so each lamp is 0.75 or 2.25 m from one.
- **Assumed: the rods.** B19 lists the shade mesh with a 4 m rod and B27 scaled the shades 2.2 times, so I took 8.8 m of rod above the lamp's height. The rod then passes through the arm's plate. If the real rod ends under 13.3 m, the chain does not reach it.
- **Assumed: the six end shades' places.** B29 gives 5.6 m high and "about 3 m outside the end boards" on Pond. I took x = +-49.5 m, y = -13.5 / 0 / +13.5 m, a rod to 13.6 m. Those x and y are guesses.
- **No strings:** B27 removed them all.

## Evolution 3: the other end

| Mesh | What | Size x, y, z (m) | Triangles (budget 600 each) |
|---|---|---|---|
| `SM_Barn_SlidingDoor` | a closed sliding door, two plank leaves on an iron rail under a timber header; a hoist beam with rope and hook at 7.6 m | 8.40 x 2.31 x 7.80 | 384 |
| `SM_Barn_Firewood` | 26 split logs stacked end-on between two posts; a chopping block | 4.57 x 0.93 x 1.78 | 414 |
| `SM_Barn_SkateBench` | a bench, a peg rail with four pairs of skates hung by their laces, one skate on the floor | 3.00 x 0.91 x 1.76 | 516 |

**The proving numbers**

- **1,314 triangles** (bar: under 1,500).
- **0.64 % of the up-ice frame at that end** (bar: under 1 %), at mean luma 17, brightest pixel 63.
- **The order is kept:** ice 169, boards 143, orange block 96, blue block 58, then these pieces at 17.
- **Ice mean luma:** 168.84 without and with.

## What the engine lane needs to know

- **The crowd:** reimport the 56 files over their assets. Set `MI_ClayCrowd_Scarf`'s tint to white: its oatmeal tint multiplies the `Scarf` slot, and the scarves' colours are now in the vertex colour. `MI_ClayCrowd_Skin` can take back some darkening. Vertex colour must import with alpha.
- **The lamp arms:** origin on the lamp's axis at the eave plane (13.5 m, the same height as the truss bay's origin). Place at (lamp x, lamp y, eave); yaw 0 when the nearest truss has the smaller x, 180 when the larger. The manifest lists all 18 with arm and yaw. They follow the rig's scale. Cast no shadow.
- **The other end:** all three use AR4's wall frame (origin on the floor 0.58 m in front of the wall, local +Y at the wall; yaw +90 on the -x end). The door goes in the middle of a wall bay; the firewood and the bench against the wall either side, not across a post.

`out\fbx-ar6\MANIFEST.md` has every line, the palette in sRGB bytes per figure, and the 18 lamps.

## What I did not look at

- **The engine.** No import. Whether Unreal's importer keeps the colour's alpha on these files is untested.
- **The crowd under the engine's own fill.** My head-to-head spread depends on how flat the light is; the ratio held in two framings (2.3 and 2.6) and in an earlier, warmer light (3.0).
- **The rows of seven in a game-camera still:** they are recoloured and on the piece sheet's files, not in the views.
- **The real lamp actors.** Rod length and the end shades' places are assumptions, above.
- **Motion and shimmer** of the lighter crowd under upscaling: light hats on dark coats are higher contrast than before.
- **Pond and the wide arena.**

## Decisions for the operator

1. **How light the crowd may be.** 99 % of figure pixels are under the boards; the lightest hat tops are not. Keep the wide value range, or cap it under the boards.
2. **The pale grey-blue.** It is one of the six light clays the brief names, at an HSV saturation of 0.14. If any blue in the crowd is unwanted, it swaps for fog grey in one line.
3. **The other end's props are hidden in play.** The firewood and the bench stand behind the end stands. Keep them for low cameras, or open a gap in the end stands in front of them.
4. **The lamp arms cost 1,440 triangles for something the game camera never sees.** They matter in the wide and far-end stills, where the rods now end in nothing.

## How to rebuild

```
cd /c/Users/kazda/jam/arena-lab
bash run6.sh            # about 6 minutes: build (3 s), twelve views and the masks (about 290 s), piece sheet (45 s), labels, manifest
bash run6.sh build | views | sheet | stills | manifest | tune
```
The whole chain ran end to end twice (03:11 to 03:16 and 03:18 to 03:22); the second run added the view under a lamp.

## Outputs

- `C:\Users\kazda\jam\arena-lab\out\fbx-ar6\` : 62 FBX (3.8 MB) and `MANIFEST.md`
- `C:\Users\kazda\jam\arena-lab\out\renders-ar6\` : `ar6-pieces.png`, `ar6-crowd-ab.png`, `ar6-cheer-ab.png`, `ar6-cheer-before-after.png`, `ar6-play-ab.png`, `ar6-upice2-ab.png`, `ar6-low-ab.png`, `ar6-lamps-ab.png` and the single stills
- `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-032301-B-ar6-crowd-gamecam.png`, `-cheer-gamecam.png`, `-crowd-play.png`, `-pieces.png`, `-other-end.png`, each with a `.txt` caption
- `C:\Users\kazda\jam\arena-lab\scripts\build6.py`, `render6.py`, `manifest6.py`, `C:\Users\kazda\jam\arena-lab\run6.sh`
- `C:\Users\kazda\jam\arena-lab\work\stats6.json`, `views6.json`, `fbx6-readback.json`

## Next three evolutions, ranked

1. **A crowd that keeps its people at 40 m.** At the far side a head is 6 to 8 pixels; the value pattern must survive there. Check AR6's colours on the far figures from the game camera and, if heads merge, enlarge the far figures' hats and lighten their shoulders. The number: head-to-head luma spread at the far side at least 60 % of the near side's, with the far figures still at 25 % of the triangles.
2. **The stands themselves, lighter where people are not.** In AR2's stills the empty benches and risers make the brown lattice. A second vertex-colour set for the stand module: seat planks a step lighter, risers and under-structure darker. The number: with the crowd hidden, the stands' band luma spread falls by a third at the game camera; with the crowd shown, figure-to-stand contrast rises by 10 luma.
3. **The scoreboard finally hung.** AR1's barn scoreboard is imported and not placed; hang it from a truss over centre ice on an arm of this kit. The number: no part under 8 m, not in the game camera's frame from the side boards, ice mean luma unchanged within 1 %.
