# AR1 report: wooden stands and a clay crowd for the empty apron (device B, Blender only)

Lane AR1, device B, machine clock 20:42 to 21:15 (2026-11-13). First complete set at 21:08, 26 minutes in; the final scripts rebuilt everything once more at 21:12 to 21:15 (`READY.txt`).
Track: the arena and the clay look. Lab: `C:\Users\kazda\jam\arena-lab\` (outside the repository).

Housekeeping: no Unreal, no git write, no build. This report is the only file written in the repository. Nothing in `skater-lab\` was touched. No file, model, texture or image from outside was read: every mesh is built from primitives by `scripts/build.py`. The engine frames were looked at only.

## Verdict

**Built and exported: 12 FBX, all under budget, all read back equal.** The crowd reads as a crowd and stays a dark band (mean luma 21 against ice 163 and boards 149; no pixel brighter than the boards). **Not proven: the engine.** Nothing was imported, and the mock-up's light is my assumption.

Two things to decide before the import lane places anything:
1. **How near the stands stand.** At the brief's camera (12 m inside the boards) the crowd is calm. From 5 m inside, looking 30 degrees down (the corner-scrum frame), the front row is as large in frame as the players and sits right behind them (`ar1-gamecam-near-crowd.png`). The fix is placement, not mesh: leave row 0 empty, or move the stand line from 2.5 m to 4 or 5 m (the manifest gives the rule for any distance).
2. **How full the house is.** Three stills of the same view: full (about three seats in four), sparse (about one in three), empty.

## What was built

| Mesh | What | Size x, depth, z (m) | Triangles (budget) | Slots |
|---|---|---|---|---|
| `SM_Stand_Mod` | timber bleacher, 4 rows with treads at 1.00 / 1.65 / 2.30 / 2.95 m, a bench on each, plank front, rail, steps in the last 0.72 m | 4.89 x 4.01 x 3.99 | 924 (1,500) | Timber, TimberDark |
| `SM_Stand_Corner` | the same bleacher turned a quarter circle in 4 straight pieces, inner radius 6.0 m | 10.00 x 10.00 x 3.98 | 1,344 (1,500) | Timber, TimberDark |
| `SM_Stand_End` | standing terrace, 2 levels at 0.70 / 1.20 m, leaning rails, steps; for behind the nets | 4.89 x 2.40 x 2.25 | 504 (1,500) | Timber, TimberDark |
| `SM_BarnScoreboard` | timber and tin box 3.0 x 1.0 x 1.5 m on four chains under a beam, blank cream faces, two shaded lamps | 3.90 x 2.06 x 3.13 | 768 (1,200) | Timber, TimberDark, Tin, Cream, LampGlow |
| `SM_Fan_A` | seated, hands on knees, bobble hat | 0.68 x 0.69 x 1.52 | 608 (900) | ClaySkin, ClayCoat, Scarf |
| `SM_Fan_B` | seated, both arms up, ear-flap hat | 0.95 x 0.80 x 1.46 | 620 (900) | ClaySkin, ClayCoat, Scarf |
| `SM_Fan_C` | standing, one fist raised, flat cap | 0.74 x 0.65 x 1.42 | 540 (900) | ClaySkin, ClayCoat, Scarf |
| `SM_Fan_D` | standing, leaning forward on the rail, brimmed hat | 0.73 x 0.76 x 1.46 | 544 (900) | ClaySkin, ClayCoat, Scarf |
| `SM_Fan_E` | standing with a tin mug, beanie | 0.67 x 0.63 x 1.47 | 572 (900) | ClaySkin, ClayCoat, Scarf, Tin |
| `SM_Fan_F` | seated child, hands cupped at the mouth, bobble hat | 0.59 x 0.51 x 1.21 | 604 (900) | ClaySkin, ClayCoat, Scarf |
| `SM_FanRow_A` | 7 mixed fans along 4.06 m of bench | 4.12 x 0.83 x 1.52 | 4,160 (4,500) | ClaySkin, ClayCoat, Scarf |
| `SM_FanRow_B` | 7 mixed fans along 4.06 m of bench | 4.18 x 0.81 x 1.52 | 4,084 (4,500) | ClaySkin, ClayCoat, Scarf, Tin |

Twelve files, 15,272 triangles, 944 KB. All five items of the brief are there, including the optional end terrace.

- **Conventions are lane A103's** (`arena.py`): centimetres, forward -Z, up Y, a piece reaches Blender -Y so it reaches Unreal +Y, pivot on the floor in the middle of the rink-side edge, no collision, slot names only. The scoreboard's pivot is the middle of its beam's top; the corner's is the centre of its turn.
- **Fans:** the players' toy proportions (a 46 cm head on a 1.45 to 1.5 m figure), coat, hat, mittens, a scarf roll with one hanging end, two eye dots and a mouth line. `Scarf` is the one team-colour slot. The coat, hat and trouser colours are vertex colours on `ClayCoat` (browns, greys, dull green, mustard, rust, slate, plum).
- **UVs:** channel 0 is non-overlapping (Smart UV Project; 0 overlapping loops by Blender's own test, 0 of 1024 x 1024 sampled texels inside two triangles). Channel 1 is a world-scale box projection, 1 unit = 2 m, for tiling wood.
- **Sockets:** `Row0` to `Row3` on the module and `Row0`, `Row1` on the end terrace mark each row's seat line.
- **Read back:** each FBX was imported into a clean metres-scene Blender for the stills: triangle counts equal the build's, both UV channels, the colour attribute and the sockets are present.

One deviation from the brief: the steps are inside the module's 4.88 m, so a bench is 4.16 m long and a crowd row is 7 fans on 4.06 m, not on 4.9 m. This keeps modules butting end to end with no gap for stairs.

## How it reads in the mock-up

The view: ice, 1.3 m boards with red cap and yellow strip, six modules (29.3 m) 2.5 m beyond the boards, a plank wall 15.5 m out with a warm wash at its foot. Camera 3.5 m up, 12 m inside the boards, 20 degrees down, 1920 x 1080, Cycles. The rink's lamps reach only ice and boards; the stands get one dim warm light from above and a flat warm fill.

| Region (luma 0..255) | Share of frame | Mean | p99 | Max |
|---|---|---|---|---|
| ice | 59.4 % | 163.2 | 170.9 | 174.1 |
| boards, cream | 9.3 % | 144.2 | 151.4 | 153.1 |
| red cap | 1.0 % | 77.9 | 95.4 | 105.2 |
| stands + fans, full house | 18.6 % | 20.6 | 56.0 | 91.6 |
| stands + fans, sparse house | 17.1 % | 20.4 | 55.5 | 87.4 |
| stands only | 14.7 % | 20.4 | 52.8 | 81.0 |
| barn wall and floor | 11.7 % | 36.9 | 99.7 | 115.1 |

- **Does it read as a crowd?** Yes, at both distances. Bare, the stands read as bleachers and the wall behind stops being a blank.
- **Is it calm?** As a value band, yes: no stand or fan pixel is brighter than the boards' median, and 0.024 % are brighter than the red cap's median (hat tops and pale faces in the nearest module).
- **Squint:** in 16-pixel blocks the band's standard deviation is 9.2 with a full house, 7.7 sparse, 5.1 bare (the ice is 5.0). The crowd nearly doubles the band's blotchiness. What you see is a speckle of faces.
- **Too big or small?** Right at 12 m: heads are about 40 pixels in the nearest module (by eye). Too big in the near view, as said above.
- **Too detailed?** The eye dots are 2 to 3 pixels at 12 m. That is the size that shimmers under the game's upscaling (lane SK20). They are a vertex colour, so a material can drop them.
- **Too uniform?** A little. Two row meshes fill the back rows nine times in the still; the raised fist recurs if you look for it. Everyone but the child is the same height.

Three passes were made against the engine frames:
1. The first test had the band at luma 9 (a black hole, what B36 repaired) and the ice at 117. Lights were set to the engine's ice value; heads were enlarged from 41 to 46 cm and standing legs shortened and thickened (they read as stilts).
2. The two brightest coat colours came down, the preview skin was darkened, and the second sloping handrail at every module joint was removed (it read as a bright diagonal stripe).
3. Part of the key light was traded for flat fill: the band's deviation fell from 14.0 to 12.9 in the test view and its maximum from 92.7 to 82.8, with the mean nearly unchanged (22.4 to 21.1).

## What is weak

- **The mock-up's band is darker than the engine's apron today** (21 against B36's 30 to 45). Lit up to the apron's value, the faces would pass the red cap. The game's materials should compress the range: a darker skin, lifted timber shadows.
- **The engine has no light on the apron** (B36: a floor emissive only, and it warned that unlit shapes there read as flat cut-outs). The stands need a real fill light or a shaded material. That is the import lane's main job.
- **The timber is plain boxes.** "Rough-sawn" is a centimetre of jitter and a tone per plank; all wood character must come from the game's material.
- **The corner piece is a 4-piece polygon** and the row meshes do not fit it; single fans only there. Its UV islands fill only 15 % of the square.
- **`SM_Fan_D`'s hat brim leans 17 cm past the stand's front edge.** Harmless at 2.5 m from the boards.
- **The scoreboard's lamps are small** and it was judged only on the contact sheet, not from the game camera.
- **No LODs.** A full house on 90 x 45 is about 1,050 fans, about 620 thousand triangles in 8 instanced meshes, plus 52 thousand of timber.

## What I could not do

- **No Unreal import.** Slot binding, vertex colours and the `SOCKET_` empties are unverified in the engine.
- **The stills are not engine frames.** Blender's Standard view transform, flat stand-ins for ice and boards, an assumed lens (21 mm on 36 mm, about 81 degrees across). The luma numbers compare with each other, not with the engine's.
- **The near view has no mask or statistics**: judged by eye.
- **No still of the corner piece or the end terrace from the game camera.** Their placement is arithmetic (the manifest's table: corner clearance 1.96 to 3.55 m on the five rink sizes).
- EEVEE was not tried: Cycles on the CPU took 50 to 100 s per 1080p still.

## How to rebuild

```
cd /c/Users/kazda/jam/arena-lab
bash run.sh            # everything, 3 to 5 minutes: build, contact sheet, four game-camera stills, MANIFEST.md, READY.txt
bash run.sh build      # meshes and FBX only, 3 to 10 s
bash run.sh test       # build + a 960 x 540 game-camera test with luma table, about 20 s
```
Blender 4.2.16 headless (`-b --factory-startup`), ffmpeg for the sheet's labels. The mock-up's lights and density are environment variables (`AR1_STAND_W`, `AR1_AMB`, `AR1_DENSITY` and others, listed in `scripts/render.py`). The whole chain was run end to end with the final scripts (21:12 to 21:15, 3 minutes with the machine less loaded): the five stills came out the same size to the byte as the run before, and the luma table is the same.

## Outputs

- `C:\Users\kazda\jam\arena-lab\out\fbx\` : the 12 FBX and `MANIFEST.md` (sizes, slots, pivots, sockets, placement rule and table for 60, 70, 90, 120 and 140 m rinks, where not to put anything, cost, the luma table)
- `C:\Users\kazda\jam\arena-lab\out\renders\ar1-pieces.png` : contact sheet with a 1.45 m block and a 1.3 m board panel
- `C:\Users\kazda\jam\arena-lab\out\renders\ar1-gamecam-crowd.png` : the view that matters, full house
- `C:\Users\kazda\jam\arena-lab\out\renders\ar1-gamecam-empty-stands.png` : the same, stands only
- `C:\Users\kazda\jam\arena-lab\out\renders\ar1-gamecam-crowd-sparse.png` : the same, about one seat in three (extra)
- `C:\Users\kazda\jam\arena-lab\out\renders\ar1-gamecam-near-crowd.png` : 5 m inside the boards, 30 degrees down, two 1.45 m stand-in blocks at the boards (extra)
- `C:\Users\kazda\jam\arena-lab\out\READY.txt`
- `C:\Users\kazda\jam\arena-lab\scripts\build.py`, `render.py`, `manifest.py`, `C:\Users\kazda\jam\arena-lab\run.sh`
- `C:\Users\kazda\jam\arena-lab\work\` : `stats.json`, `fbx-readback.json`, `gamecam-stats.json`, `gamecam-stats-sparse.json`, masks, `ar1.blend`

Placement in one line (the manifest has the rest): front edge 2.5 m beyond the boards' outer face on every rink, corner pivots 3.3 m inside the board lines, whole modules between them stretched 0.96 to 1.02 along x: 11 / 17 / 27 modules per long side and 5 / 8 / 13 places per end on the 60 / 90 / 140 m rinks. That line leaves a 0.76 m walkway behind the other device's team benches (+y side, x within 0.134 L of centre) and penalty boxes (-y side, x within 5.3 m).

## Next three evolutions for this track, ranked

1. **Import, place and light the stands on the clay tier** (through the rig, so the 60, 90 and 140 m rinks follow). The number: in the QA corner-scrum frame (`qa3-clay-play-2`) the stands' band reads mean luma 30 to 45 with its maximum under the red cap (about 100); ice regions unchanged within noise (B36's 0.5); orange and blue figures against what is behind them at the boards still 45 or more apart; frame time at five a side no more than 0.3 ms above lane A106's measure. This is where the two decisions above get settled, in the engine.
2. **A crowd that does not shimmer and costs little**: distance levels for the fans (no face marks beyond the second row, one merged low mesh per back row) and a value-compressed crowd material. The number: over a 1 degree per frame pan along the boards, the crowd band's temporal deviation p99 at or under the boards' own (B36 measured 1.54); crowd triangles in view under 150 thousand from about 620 thousand; squint deviation of the band at or under 7 (9.2 today, bare stands 5.1).
3. **The crowd answers a goal without moving during play**: after the horn, with play dead, the scoring side's seated fans swap to the arms-up meshes for two seconds and the scoreboard's lamps warm up; nothing changes while the puck is live. The number: 0 changed crowd pixels across any 600 live-play frames; at least 30 % of the scoring side's fans swapped within 6 frames of the goal. This one needs the operator's ruling first, because it touches the no-motion rule.
