# AR5 report: the crowd as instances in real size on every rink, a cheer that hears the goal, and the barn dressed by numbers (device B, an editor without a GPU)

Lane AR5, device B, machine clock 02:52 to 03:39 (2026-11-14). Tree `C:\Users\kazda\jam\wt\b-arena`, branch `lane/b-arena` at d217ceea, built first (`ThirdJamEditor Win64 Development`, exit 0, 02:53 to 03:00). Editor on port 8008 with `-nullrhi -unattended`: the log has **0 `LogD3D12RHI` lines**, `NullDrv` loaded, `rhiname="Null"`. Two editor processes, both mine: pid 28524 (03:00 to 03:31) did the work; pid 2828 (03:31 to 03:37) was started fresh to read every saved asset back from disk and to run the final measurements. Both left through `ExitEditor` with nothing unsaved (the dirty list was empty each time), `LogExit: Exiting.` in both logs, both processes confirmed gone (the second at 03:37:27). 27 play sessions, no hang, no assertion.

**Nothing in this lane was seen, and no material was compiled** (the editor refuses: "this editor has no renderer, so materials can't be compiled"). Every verdict below is a count or a measurement of geometry. The script for the GPU is `status/AR5-film.md`; its step 0 is whether the two changed masters compile.

Housekeeping:
- No commit, no push, no cook. One build of my own tree, as the brief said.
- Assets written only under `Arena/`, my crowd and stand materials under `Look/`, and `Rigs/LR_P3_Clay.umap`, all in my tree (37 files; `git status` shows nothing else but the two reports).
- The other editors (8002, 8004, 8000, 8006) were not touched.
- **The play world did not tick** until `Slate.bAllowThrottling 0` (world time stood at 0.0, as the figure lane found). Set for the measurements and put back to 1. Console variables at the end: `jam.Cut hockey.regular`, `jam.ScriptHuman 0`, `jam.ctrl.Drill 0`, `jam.ctrl.HandPuck 0`, `Slate.bAllowThrottling 1`.
- **Tick rate: 47 to 60 ticks per wall second** over eight goal tests of about four seconds each (game time against the wall clock, taken inside the editor's own tick; the last two: 60 and 53.3). Other lanes were building and rendering beside me.
- Scratch, scripts and the rig's backup: `C:\Users\kazda\AppData\Local\Temp\claude\6d8eaecb999746c2\1f5c7b1e-557b-4c70-b752-8370d9dbef0b\scratchpad\ar5`.

## Verdicts

| # | Job | Verdict |
|---|---|---|
| 1 | The crowd as instances with a far level | **DONE.** 293 actors become 1 in play (7 in the rig file, 6 remove themselves). 12 figures, each with its far level as LOD 1. Instances equal fans on four rinks. Triangles by the engine's LOD rule: 55 to 62 % of AR2's 339,596 on the regular rink, with 36 % more fans |
| 2 | Honest size on every rink | **DONE, with a small Blueprint of my own** (no code of the other device, no graph for the figure lane). Fans 0.96 to 1.04 of their built size and stands 3.99 m high on Pond, regular, large and wide; 0 bounds inside the clear zones; corner rows three quarters full |
| 3 | The cheer | **DONE as far as it can be without a picture.** A `goal` on the bus raises the scored end's scalar over 8 frames and drops it to 0 by frame 108; 332 and 332 fans of the two halves, only the scored end's half rises. The pose itself is in a material that was never compiled or seen |
| 4 | The barn's roof and walls | **DONE by numbers**: roof bays, wall bays, gate, props, loft, ice machine, bunting. 16,276 triangles, 1 actor, 0 lights, nothing in the clear zones, lowest point over the ice 8.97 m (Pond). AR6's lamp arms and far-end dressing: **not placed** (no time) |
| 5 | The filming and judging script | **DONE**: `status/AR5-film.md` |
| + | AR6's crowd colours (your addition) | **DONE**: the 12 figures and their far levels reimported from `fbx-ar6`; the colour's alpha came through; coat and accent values per instance; the scarf's tint white; the fill neutral |

## 1. The crowd as instances with a far level

- **Import.** `SM_Fan_A` to `_L` from AR6's files (first from AR3's and AR4's, then again from AR6's when your addition came), the `_L1` file of each as LOD 1. Read back on both levels of all twelve: **5 UV channels, the slots in the same order** (`ClaySkin, ClayCoat, Scarf`; `Tin` first on E; G to L without `Scarf`), 520 to 620 triangles full and 126 to 146 far. The pose channels hold numbers on both levels: the largest offset per figure is 34 to 97 cm on the full level and 36 to 105 cm on the far level. (AR3's table says 29 to 92 cm; I did not find out why mine read higher. Full-precision UVs are on for both levels.)
- **AR6's alpha.** Read back on `SM_Fan_G`, `SM_Fan_A` and `SM_Fan_A` LOD 1: exactly the six values 0.0, 0.2, 0.4, 0.6, 0.8, 1.0 (on A: 100 coat vertices, 48 hat, 34 scarf, 28 mittens, 52 skin, 84 other).
- **The far level's switch.** LOD 1 below screen size **0.1345 to 0.1562** (one value per figure = 2.539 x its bounds radius / 30 m; 2.539 is the engine's factor for a 70 degree view at 16:9, which is what the game camera reported in every sample). That is a switch at about **30 m**: full under 25 m, far beyond 35 m. Auto LOD distances are off on the twelve meshes.
- **Bounds raised** for the cheer: 95 cm up, 55 cm to each side on every figure.
- **Instances.** The components are `HierarchicalInstancedStaticMeshComponent` (per-cluster LOD by distance), one per figure mesh and one per stand mesh, no collision, no shadow, static. Five custom floats per fan: **0 cheer weight** (0.75 to 1.0), **1 coat value** (0.60 to 1.30), **2 accent value** (0.80 to 1.15: hat, scarf, mittens), 3 the end (+1 or -1), 4 a delay (0 to 0.5). Each seat's values come from one seed, so they are stable.
- In the editor the rig now shows all seven layouts over each other; only play sorts them out.
- **Actors.** AR2 had 237 crowd actors and 56 stand actors. Now the rig holds 7 owner actors (one per rink length) of which **1 is alive in play**; the rig goes from 416 to 121 actors.
- **Instances equal fans** on every layout (read back from the saved rig and again in play). The regular rink has **664** fans, not AR2's 489: the same 44 % on the sides and ends, plus the corners at three seats in four (215 of 272 corner seats; AR2 had 26 fans in the corners). To go back to 489 is one number.
- **No two neighbours alike:** 0 seated fans share a figure with any of their ten nearest on any layout. The standing terraces behind the nets have three figures to choose from and repeat there (0 to 8 fans a layout).

**Triangles.** No renderer, so nothing was drawn and no engine counter exists here. These are computed with the engine's rule (screen size from bounds radius, distance and the camera's real field of view) for six moments of the game camera in a real un-paused match per rink:

| Rink | Fans | Full figures / far level (six camera moments) | Crowd triangles if every fan were drawn | Crowd triangles inside the frame | Stands | Crowd + stands, worst moment |
|---|---:|---|---|---|---:|---:|
| Pond | 491 | 115 to 247 / 244 to 376 | 119,532 to 179,772 | 5,916 to 34,488 | 34,104 | 213,876 |
| Regular | 664 | 97 to 152 / 512 to 567 | 134,426 to 159,348 | 2,448 to 31,840 | 51,744 | 211,092 |
| Large | 830 | 37 to 164 / 666 to 793 | 130,078 to 187,816 | 0 to 32,946 | 69,216 | 257,032 |
| Wide | 944 | 0 to 151 / 793 to 944 | 129,036 to 197,578 | 0 to 57,258 | 80,304 | 277,882 |

- On the regular rink the stands and crowd are **186,170 to 211,092 triangles if every fan were drawn** (55 to 62 % of AR2's 339,596, with 175 more fans). Inside the game camera's frame the crowd is 2,448 to 31,840.
- The hierarchical component picks the level per cluster, not per fan, so the real split will differ a little. The stands have no far level (924 triangles a module).

## 2. Honest size on every rink

**The way chosen: a baked layout per rink length, and a Blueprint on the owner that reads the rig's scale.** `Arena/BP_ArenaCrowd` (written with the editor's Blueprint DSL tool, which my tools can use) is the owner of one layout. The rig holds seven owners, built for rink lengths 60, 64, 70, 80, 90, 120 and 140 m (every length in the shipped cartridges), each in real centimetres and scaled by 90 / length. When the switcher loads the rig at scale length / 90, exactly one owner has world scale 1; at BeginPlay every owner whose scale is not 1 within 0.02 destroys itself. It needs nothing from device A.

Each layout is built by the same rule: the stand line 4.0 m beyond the boards (sides and ends), as many modules as fit (stretched at most 6 % along their length, never in height or depth), corner pieces, terraces behind the nets.

| Rink length | Owner | Fans | Stand pieces (modules + terraces + corners) | All | Corner seats filled | Fans repeating a neighbour (terraces) | Measured in play |
|---|---|---:|---|---:|---|---:|---|
| 60 m | `Crowd_P3_Clay_k0667` | 491 | 30 + 2 + 4 | 36 | 211 of 272 | 0 | run |
| 64 m | `Crowd_P3_Clay_k0711` | 491 | 32 + 4 + 4 | 40 | 210 of 272 | 4 | not run |
| 70 m | `Crowd_P3_Clay_k0778` | 521 | 34 + 4 + 4 | 42 | 208 of 272 | 8 | not run |
| 80 m | `Crowd_P3_Clay_k0889` | 612 | 44 + 2 + 4 | 50 | 200 of 272 | 0 | not run |
| 90 m | `Crowd_P3_Clay_k1000` | 664 | 48 + 4 + 4 | 56 | 215 of 272 | 8 | run |
| 120 m | `Crowd_P3_Clay_k1333` | 830 | 68 + 2 + 4 | 74 | 199 of 272 | 0 | run |
| 140 m | `Crowd_P3_Clay_k1556` | 944 | 80 + 2 + 4 | 86 | 206 of 272 | 0 | run |

Measured in headless play (each rink's own match; bounds from the instances' world transforms, device A's pieces from its rink actor):

| Rink | Rig scale | Owner left alive | Fans = instances | Stand pieces | Fan scale (min to max) | Mean fan height | Stand height | Stand front, side / end (m) | Device A reaches to (m) | Bounds inside the clear zones | Bounds meeting device A's pieces |
|---|---:|---|---:|---:|---|---:|---:|---|---|---:|---:|
| Pond 60 x 30 | 0.667 | `Crowd_P3_Clay_k0667` | 491 | 36 | 0.96 to 1.04 | 1.40 m | 3.99 m | 19.0 / 34.0 | 16.98 / 30.86 | 0 | 0 |
| Regular 90 x 45 | 1.000 | `Crowd_P3_Clay_k1000` | 664 | 56 | 0.96 to 1.04 | 1.40 m | 3.99 m | 26.5 / 49.0 | 24.48 / 45.86 | 0 | 0 |
| Large 120 x 60 | 1.333 | `Crowd_P3_Clay_k1333` | 830 | 74 | 0.96 to 1.04 | 1.40 m | 3.99 m | 34.0 / 64.0 | 31.98 / 60.86 | 0 | 0 |
| Wide 140 x 70 | 1.556 | `Crowd_P3_Clay_k1556` | 944 | 86 | 0.96 to 1.04 | 1.40 m | 3.99 m | 39.0 / 74.0 | 36.98 / 70.86 | 0 | 0 |

- **Fan height:** the twelve figures are 1.21 to 1.66 m as built (a child, a tall hat); in play they are 1.17 to 1.73 m and 1.40 m in the mean on all four rinks. Against your 1.45 m the mean is 3.6 % under; the scale is within 4 % of 1 everywhere.
- **Clear zones** are taken as |y| under half-width + 2.1 m and |x| under half-length + 1.1 m (B41's numbers on the regular rink, carried to the others). 0 fan or stand bounds inside on the four rinks; the corner pieces' inner radius (6.0 m) clears the zone's corner (5.14 m from the turn's centre).
- **Where it does not hold:** a rink of any other length gets no stands and no crowd (every owner removes itself). The stand line assumes width = length / 2; the 60 x 28 rink gets a walkway 1 m wider. Cage (70), the 64 m and the 80 m rink were built and not run.

## 3. The cheer

- **In the material** (`M_ClayCrowd`, stock nodes, not compiled): world position offset = the mesh-space vector (UV2.u, UV3.u, UV4.u) turned to world, times `PoseK` x the fan's weight x a smoothed value of its end's scalar that starts later by the fan's delay.
- **The hook:** `Arena/MPC_Crowd` with `CrowdCheerPlusX` and `CrowdCheerMinusX`. (`MPC_Look` is not on my list to write, so the collection is my own.)
- **The binding, done here:** `BP_ArenaCrowd` binds `JamEventBus.OnJamEvent` at BeginPlay. On `goal` it notes the time for the end whose net was scored on (`Where.X` above 0: the +x end); every tick it writes both scalars: up over 8 frames, held, down from frame 88 to 108.

Proof in headless play on the regular rink, a `goal` sent on the bus at each end, the scalars read every tick:

| Frame after the goal | 1 | 4 | 8 | 60 | 88 | 98 | 107 | 108 | 130 |
|---|---|---|---|---|---|---|---|---|---|
| Goal at +x: `CrowdCheerPlusX` | 0.125 | 0.5 | 1 | 1 | 1 | 0.5 | 0.05 | 0 | 0 |
| Goal at +x: `CrowdCheerMinusX` | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| Goal at -x: `CrowdCheerMinusX` | 0.125 | 0.5 | 1 | 1 | 1 | 0.5 | 0.05 | 0 | 0 |
| Goal at -x: `CrowdCheerPlusX` | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |

- **Fans whose amount is above 0** (the material's own formula on each fan's data and the scalar read in play; frames 2, 8, 60, 100, 109, 130), goal at +x: `{"2": {"plus_x_half": 192, "minus_x_half": 0, "max_amount": 0.149}, "8": {"plus_x_half": 332, "minus_x_half": 0, "max_amount": 0.999}, "60": {"plus_x_half": 332, "minus_x_half": 0, "max_amount": 0.999}, "100": {"plus_x_half": 332, "minus_x_half": 0, "max_amount": 0.341}, "109": {"plus_x_half": 0, "minus_x_half": 0, "max_amount": 0}, "130": {"plus_x_half": 0, "minus_x_half": 0, "max_amount": 0}}`
- Goal at -x: `{"2": {"plus_x_half": 0, "minus_x_half": 201, "max_amount": 0.142}, "8": {"plus_x_half": 0, "minus_x_half": 332, "max_amount": 1}, "60": {"plus_x_half": 0, "minus_x_half": 332, "max_amount": 1}, "100": {"plus_x_half": 0, "minus_x_half": 332, "max_amount": 0.327}, "109": {"plus_x_half": 0, "minus_x_half": 0, "max_amount": 0}, "130": {"plus_x_half": 0, "minus_x_half": 0, "max_amount": 0}}`
- So: 332 fans of the scored half rise, 0 of the other half, all at rest again at frame 108 (your bar: 130). At frame 2 only 192 of the 332 have started: that is the delay.
- **It answers the match's own goals too.** In one run a real goal of the scripted match fell 80 frames after my staged one (log: `JAMEVENT goal who=8 where=(-4113.0,30.2,4.0)`), and the -x half rose for it while the +x half was still up.
- **Which half answers is my choice** (the end where the puck went in). If it should be the scoring team's own end, it is one comparison in the Blueprint.

## 4. The barn's roof and walls

All on instances, all matte, no light added. The roof, the walls and the bunting sit on one actor `Dress_P3_Clay` that scales with the rig like the barn; the gate, its props, the loft and the ice machine sit on each rink's crowd owner, placed at that rink's wall and scaled like the barn, so they can be left out where they do not fit.

| Piece | Count | Where (regular rink) | Triangles |
|---|---:|---|---:|
| `SM_Barn_TrussBay` | 10 | x = -60 + 12 k, eave 13.5 m, height scale 0.82 | 4,920 |
| `SM_Barn_WallBay` | 32 | 10 a long wall, 6 an end, 0.58 m in front of the old wall | 5,760 |
| `SM_Barn_Bunting` | 12 | six across each end at x = +-53 m, hung at 13.05 m | 2,784 |
| `SM_Barn_IceGate` | 1 | the +x end wall, 18.5 m off the centre line | 456 |
| `SM_Barn_Loft` | 2 | over the gate's bay and the next | 864 |
| `SM_Barn_GateProps` | 1 | the next bay toward the centre | 688 |
| `SM_Barn_IceMachine` | 1 | 2 m off the +x end wall, on the gate's other side | 804 |
| | | **Added** | **16,276** |

- **The old ten trusses are deleted, not kept beside** (10 actors, 720 triangles gone): the new bay carries its own truss on the same line, and two sets would double every chord. The bays' height is squeezed to 0.82 so their top (21.87 m) is the old trusses' top under the roof slabs. AR3's places put a truss every 12 m from x = -60; the old ones stood 6 m off that. The lamp shades hung between trusses before and still do.
- **The old walls and posts stay**; the wall bays stand in front of them.
- **`NightGlass`:** `Arena/MI_Barn_NightGlass`, `FillK` 0.5, a low even blue; for the picture lane to tune.
- **Bunting** is hung at 13.05 m so that the rig's scale leaves it at 8.70 m on Pond (lowest tip 7.58 m). In 24 sampled game-camera frames (six a rink) no length of it was in frame.

| Rink | Dressing scale | Lowest point over the ice (roof bays) | Bunting lowest tip | Pieces under 7 m inside the clear zones | Dressing bounds meeting a stand | Bunting in a sampled game-camera frame | Left out |
|---|---:|---:|---:|---:|---:|---|---|
| Pond | 0.667 | 8.97 m | 7.58 m | 0 | 0 | 0 of 6 | the loft |
| Regular | 1.000 | 13.46 m | 11.37 m | 0 | 0 | 0 of 6 | nothing |
| Large | 1.333 | 17.95 m | 15.16 m | 0 | 0 | 0 of 6 | nothing |
| Wide | 1.556 | 20.94 m | 17.68 m | 0 | 0 | 0 of 6 | nothing |

- "Meeting a stand" is bounds against bounds, with the corner stands tested as a ring. Before the loft was left out on the small rinks it met the +x end stand on Pond (its ladder's foot stood in the back row).
- Actors added: 1 (`Dress_P3_Clay`). Lights added: 0 (the rig has 18, as before).

**AR6's second part is not placed:** the three lamp arms (18 places from the rig's real lamp actors) and the far end's sliding door, firewood and skate bench are not imported. They are the next engine lane's first hour.

## 5. The filming script

`status/AR5-film.md`: step 0 the compile and a first look (2 minutes), step 1 the readability pairs with the hide and show code (4), step 2 the three things to tune with parameter names, values now and ranges, plus the Blender lane's two crowd measures (6), step 3 the list only eyes can pass (3).

## Not verified

Everything that needs eyes or a renderer:
- **Whether `M_ClayCrowd` and `Arena/M_ArenaWood` compile.** `M_ArenaWood` is a copy of `M_ClayBarn` with one usage flag more; `M_ClayCrowd` got about thirty stock nodes. A material that fails draws as the engine's grey default.
- How the crowd looks: brightness, colour, the far level against the full one, the switch at 30 m, whether fans sit on the benches (the seat numbers are AR1's; the corner seats are my reading of four straight benches).
- The cheering pose on screen, its share of the frame, arms through neighbours, culling at the raised bounds.
- Readability (ice, puck, jerseys) with the new crowd and the new colours; the band's luma; saturation against the jerseys.
- The roof bays against the roof slabs and the lamp cords, the wall bays against the old posts, the gate, loft, props and machine in place, the bunting from any camera, the windows' glow.
- Frame time, draw calls, memory, the rig's load time (the rig file is 1,538,671 bytes and holds seven layouts).
- A cook and a packaged build. In particular: a Blueprint in the plugin that binds the game's event bus, instanced components added per actor in a streamed level, and the two masters' instanced-mesh flag.
- The saved assets in an editor **with** a renderer. (In a fresh renderer-less process they were read back: 12 figures with 2 levels, 5 UV channels and their screen sizes; both masters with the instanced-mesh flag; the collection's two scalars; the Blueprint up to date; 7 owners with every component attached and the instance counts above.)
- Cage, the 64 m and the 80 m rink in play; a rink with a width other than half its length.
- The game camera's field of view on other settings than the 70 degrees it reported; a wider view moves the switch distance.
- The night and broadcast tiers: untouched (other rig files).
- `SM_FanRow_A`, `SM_FanRow_B` and `SM_BarnScoreboard` are still in the tree and no longer used by the rig.

## For the picture lane

1. Run `status/AR5-film.md` after the merge, step 0 first. If a master does not compile, the two suspects and how to cut each are written there.
2. The three numbers the lead asked for are in step 2 with the parameters: the band's mean 45 to 55, the windows 20 to 40, the cheer at 5 % of the frame.
3. To hide everything of this lane in play: the one live `BP_ArenaCrowd` actor and `Dress_P3_Clay` (code in step 1).

## For the figure lane

Nothing to author: the graph was written here. For review, the whole of `BP_ArenaCrowd` (two float variables `GoalTimePlusX`, `GoalTimeMinusX`):

```
(event `self.ReceiveBeginPlay`
  (set `self.GoalTimePlusX` -1000.0)
  (set `self.GoalTimeMinusX` -1000.0)
  (if (> (`KismetMathLibrary.Abs` (- (.x (`self.GetActorScale3D`)) 1.0)) 0.02)
    (`self.K2_DestroyActor`)
    (else
      (add `JamEventBus.OnJamEvent` (subsystem `JamEventBus`) (`|CreateDelegate` :!event `HandleJamEvent`)))))

(event `HandleJamEvent`
  (params (Event :type Name) (Payload :type "JamEventPayload Structure"))
  (bind (_who _where) (break `JamEventPayload` Payload))
  (if (== Event "goal")
    (if (> (.x _where) 0.0)
      (set `self.GoalTimePlusX` (`GameplayStatics.GetTimeSeconds`))
      (else
        (set `self.GoalTimeMinusX` (`GameplayStatics.GetTimeSeconds`))))))

(event `self.ReceiveTick`
  (bind fp (* (- (`GameplayStatics.GetTimeSeconds`) (`self.GoalTimePlusX`)) 60.0))
  (bind fm (* (- (`GameplayStatics.GetTimeSeconds`) (`self.GoalTimeMinusX`)) 60.0))
  (`KismetMaterialLibrary.SetScalarParameterValue` "/JamAssets/Arena/MPC_Crowd.MPC_Crowd" "CrowdCheerPlusX" (* (`KismetMathLibrary.FClamp` (/ fp 8.0)) (`KismetMathLibrary.FClamp` (/ (- 108.0 fp) 20.0))))
  (`KismetMaterialLibrary.SetScalarParameterValue` "/JamAssets/Arena/MPC_Crowd.MPC_Crowd" "CrowdCheerMinusX" (* (`KismetMathLibrary.FClamp` (/ fm 8.0)) (`KismetMathLibrary.FClamp` (/ (- 108.0 fm) 20.0)))))
```

Two traps for any lane that adds components to rig actors by script:
- An instanced component added to an actor that already has a scale keeps its world scale, not its relative one. My first rig had every layout at the rig's scale again until I reset the components' relative transforms; the measurement caught it (fans 0.93 m on Pond).
- A Blueprint actor's default root is movable. Static components added to it load with `AttachTo ... is not static ... Aborting` (260 warnings a rig load in my case) and stay unattached. The numbers were right all the same, by luck of the design (the live owner sits at the origin with scale 1). I made the Blueprint's root static, rebuilt the rig, and measured again: every component attached after a load from disk, no new warning in five more play sessions, the same numbers on four rinks.

## For the other device

1. **Please time the clay tier with and without this lane's actors**, five a side at 1920 x 1080, on the regular rink and on the wide arena: frame time on GPU and game thread, draw calls, triangles drawn, and the menu-to-match frame (the rig now loads seven layouts and removes six). Off switch: hide the live `BP_ArenaCrowd` actor and `Dress_P3_Clay`.
2. **The crowd exists for rink lengths 60, 64, 70, 80, 90, 120 and 140 m.** A new length needs a new layout from B (one line in the placing script); until then that rink has no stands.
3. **The cheer reads your `goal` event's `Where.X` sign** (centimetres in the events I saw; only the sign is used) as the scored end and the frame count as 60 a second. If either changes, tell B.
4. **The cook:** this lane adds a Blueprint, a parameter collection and instanced components to the clay rig. If the cook warns about `BP_ArenaCrowd` or `MPC_Crowd`, that is ours.
5. The clear zones are yours as B41 gave them; the stands stand 4.0 m beyond the boards on every rink now (Pond's walkway is 2.0 m, not AR2's 0.7 m).

## Decisions for the operator

1. **How full.** 664 fans on the regular rink (AR2: 489): the sides as before, the corners now three quarters full. Fewer is one number; I chose more because the corners were the critic's fault and the far level pays for them.
2. **Which half cheers:** the end where the puck went in (now), or the scoring team's own end.
3. **The old trusses are gone** from the clay rig in favour of AR3's bays. If the picture says no, the old rig file is one commit back.
4. **Small rinks** (60, 64, 70 m) have no loft: it does not fit behind an honest stand. Leave it out, or shorten the end stand there.
5. **The barn itself still scales** with the rink (a 13.5 m eave on the regular rink is 21 m on the wide arena) while the stands and people no longer do. On the wide arena that is a small crowd in a very large shed. Keep, or cap the barn's scale: that would be the next honest-size step and it touches the lamps.

## The next three evolutions, each with a proving number

1. **The picture lane's pass turned into values** (after `AR5-film.md`): crowd band mean 45 to 55 at the game camera, 0 % of crowd pixels more saturated than the jerseys, the cheer changing at least 5 % of the frame on the scored side, ice within 1 %.
2. **AR6's lamp arms and far end, and a far level for the stands:** 18 arms, each meeting its lamp's rod within 0.3 m (skipped otherwise, and counted); stands and crowd under 150,000 triangles at every sampled camera moment on the wide arena (now up to 277,882).
3. **One owner instead of seven, and the barn at an honest scale:** the rig file under 900,000 bytes (now 1,538,671), no actor destroyed at load, the eave between 12 and 15 m on all seven rink lengths.

## Ledger rows

37 rows in the ledger's 11 columns. Rows for the rig, `SM_Fan_A` to `_F`, `M_ClayCrowd`, the four `MI_ClayCrowd_*` and the two `MI_ClayBarn_Stand*` replace the rows of the same path; the rest are new.

```
| Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap | other | 1.54 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | P3 clay rig, barn with calm ice: PPV (EV100 4.2, SSR override, focus 16 m f/4, no depth blur, motion blur 0), sky light a movable real-time capture (no cubemap; intensity 0.35 and colour unchanged) of the capture dome SkyCap_P3_Clay (engine sphere, scale 700, MI_SkyAmbient_Clay, outside the barn; B34), 4 shadowed warm key spots (5200 cd, no specular, shadow resolution 1.0 and source radius 60 cm since B34), 8 shadowless lamps over the side lanes (2100 cd, broad beams, no specular; B28), 6 shadowless fills at lower intensity (B28), 18 lamp shades (6 of them decorative beyond the end boards, twice as wide since B29), LookParams (FeltRim 1.4); plank walls, posts, trusses, roof, floor; no bulb strings; all shadowless meshes, no collision; the old ten trusses replaced by ten roof bays (SM_Barn_TrussBay, height scale 0.82), 32 wall bays in front of the plank walls and 12 lengths of bunting hung at 13.05 m, all instances on one actor Dress_P3_Clay that scales with the rig; timber stands and a clay crowd as instances on seven actors Crowd_P3_Clay_k0667, k0711, k0778, k0889, k1000, k1333, k1556 (class BP_ArenaCrowd, one layout in real size per rink length 60, 64, 70, 80, 90, 120, 140 m, each scaled by 90 / length so that only the one whose rink is loaded has scale 1 and stays; the others destroy themselves at BeginPlay): on the regular rink 56 stand pieces and 664 fans (twelve figures with a far level, per-instance cheer weight, coat value, accent value, end and delay, front row empty, corner rows three quarters full), the ice gate, its props, a loft of two bays and the ice machine at the +x end wall; no light added, no shadows, no collision (lanes B9, B14, B18, B19, B22, B23, B27, B28, B29, B34, AR2, AR5) (1538671 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_A.uasset | mesh | 0.14 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, seated, hands on knees, 608 tris and a far level (LOD 1) of 146 tris switching at about 30 m (screen size 0.1496), slots ClaySkin, ClayCoat, Scarf; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5) (138449 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_B.uasset | mesh | 0.14 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, seated, both arms up, 620 tris and a far level (LOD 1) of 138 tris switching at about 30 m (screen size 0.1562), slots ClaySkin, ClayCoat, Scarf; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5) (138995 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_C.uasset | mesh | 0.13 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, standing, one fist up, 540 tris and a far level (LOD 1) of 126 tris switching at about 30 m (screen size 0.147), slots ClaySkin, ClayCoat, Scarf; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5) (130699 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_D.uasset | mesh | 0.13 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, standing, leaning on a rail, 544 tris and a far level (LOD 1) of 126 tris switching at about 30 m (screen size 0.1504), slots ClaySkin, ClayCoat, Scarf; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5) (130305 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_E.uasset | mesh | 0.13 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, standing with a mug, 572 tris and a far level (LOD 1) of 134 tris switching at about 30 m (screen size 0.1464), slots Tin, ClaySkin, ClayCoat, Scarf; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5) (133183 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_F.uasset | mesh | 0.13 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, seated child, hands at the mouth, 604 tris and a far level (LOD 1) of 138 tris switching at about 30 m (screen size 0.1345), slots ClaySkin, ClayCoat, Scarf; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5) (132959 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_G.uasset | mesh | 0.14 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, seated, broad, arms crossed, tall floppy hat, 608 tris and a far level (LOD 1) of 138 tris switching at about 30 m (screen size 0.156), slots ClaySkin, ClayCoat; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5) (135337 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_H.uasset | mesh | 0.13 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, seated, slim and tall, hand at the chin, flat cap, 520 tris and a far level (LOD 1) of 126 tris switching at about 30 m (screen size 0.1462), slots ClaySkin, ClayCoat; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5) (128216 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_I.uasset | mesh | 0.13 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, seated, hands in the lap, hood up, 588 tris and a far level (LOD 1) of 138 tris switching at about 30 m (screen size 0.1451), slots ClaySkin, ClayCoat; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5) (133781 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_J.uasset | mesh | 0.14 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, seated, small, pointing at the ice, ear-flap hat, 620 tris and a far level (LOD 1) of 138 tris switching at about 30 m (screen size 0.1408), slots ClaySkin, ClayCoat; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5) (137464 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_K.uasset | mesh | 0.14 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, seated, turned to a neighbour, bare head with a bun, 596 tris and a far level (LOD 1) of 138 tris switching at about 30 m (screen size 0.1482), slots ClaySkin, ClayCoat; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5) (136227 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_L.uasset | mesh | 0.13 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, seated youngster, elbows on knees, 608 tris and a far level (LOD 1) of 138 tris switching at about 30 m (screen size 0.136), slots ClaySkin, ClayCoat; five UV channels on both levels (UV2 to UV4 hold the offset in cm to the cheering pose, full-precision UVs), bounds raised 95 cm up and 55 cm to the sides for the pose; no collision, vertex colours of lane AR6 with the part in the alpha (1.0 coat, 0.8 hat, 0.6 scarf, 0.4 mittens, 0.2 skin, 0.0 the rest), materials bound to the slots (lanes AR2, AR5) (131223 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Barn_TrussBay.uasset | mesh | 0.09 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | one 74 m roof truss with a 12 m bay of purlins and rafters, 12.79 x 74.69 x 10.25 m, 492 tris, slots Timber, TimberDark, Tin; no collision, vertex colours, materials bound to the slots (lane AR5) (91359 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Barn_Bunting.uasset | mesh | 0.08 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | 12 m of plain cloth pennants on a cord, 232 tris, slot Cloth; no collision, vertex colours, materials bound to the slots (lane AR5) (83160 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Barn_IceGate.uasset | mesh | 0.10 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | timber double gate for the ice machine, 6.71 x 0.58 x 5.03 m, 456 tris, slots Timber, TimberDark, Tin; no collision, vertex colours, materials bound to the slots (lane AR5) (95244 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Barn_WallBay.uasset | mesh | 0.08 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | 12 m of wall framing 13.5 m high with one high window, 180 tris, slots Timber, TimberDark, NightGlass; no collision, vertex colours, materials bound to the slots (lane AR5) (78941 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Barn_Loft.uasset | mesh | 0.10 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | 12 m of plank gallery with hay bales and a ladder, 432 tris, slots Timber, TimberDark, Hay; no collision, vertex colours, materials bound to the slots (lane AR5) (96388 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Barn_IceMachine.uasset | mesh | 0.11 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | a clay ice resurfacer, parked, no text or mark, 804 tris, slots Tin, Paint, Rubber; no collision, vertex colours, materials bound to the slots (lane AR5) (107508 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Barn_GateProps.uasset | mesh | 0.11 | original | Blender 4 script (lanes AR1, AR3, AR4, AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | hose, stick rack, shovel, broom, buckets and spare board panels along 7.7 m of wall, 688 tris, slots Timber, TimberDark, Tin, Cream, Hay, Rubber; no collision, vertex colours, materials bound to the slots (lane AR5) (111746 bytes) |
| Game/Plugins/JamAssets/Content/Look/M_ClayCrowd.uasset | material | 0.03 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | master for the clay crowd and small parts: vertex colour (decoded) x Tint x a per-instance value chosen by the vertex alpha (coat: custom data 1; hat, scarf, mittens: data 2), muted by Sat, with a dim warm fill of its own colour (AR2CrowdBase, AR2CrowdFill); world position offset = the cheering pose from UV2 to UV4 x per-instance weight (data 0) x the scalar of the instance own end (data 3; MPC_Crowd CrowdCheerPlusX or CrowdCheerMinusX), started late by a per-instance delay (data 4), x PoseK; used with instanced static meshes; the AR5 part was saved without a compile (no renderer in that editor) (lanes AR2, AR5) (28933 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayCrowd_Fan.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_ClayCrowd for coats, hats and mittens (slot ClayCoat): Sat 0.8, FillK 1.4, FillLow 0.45, Rough 0.9, fill colour now a neutral (0.71, 0.71, 0.71) (lanes AR2, AR5) (5628 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayCrowd_Skin.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_ClayCrowd for faces (slot ClaySkin): Tint (0.55, 0.44, 0.36), FillK 0.4, fill colour now a neutral (0.71, 0.71, 0.71) (lanes AR2, AR5) (5634 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayCrowd_Scarf.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_ClayCrowd for scarves (slot Scarf): Tint now white (the colours are in the vertices since AR6), Sat 0.7, FillK 0.7, fill colour a neutral (0.71, 0.71, 0.71) (lanes AR2, AR5) (5640 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayCrowd_Tin.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_ClayCrowd for tin parts (slot Tin): Tint (0.30, 0.31, 0.33), fill colour now a neutral (0.71, 0.71, 0.71) (lanes AR2, AR5) (5628 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayBarn_Stand.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of Arena/M_ArenaWood (was M_ClayBarn; same values) for the stands planks (slot Timber): WoodLight (0.15, 0.09, 0.052), WashK 6.5, WashHeightCm 450 (lanes AR2, AR5) (9257 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayBarn_StandDark.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of Arena/M_ArenaWood (was M_ClayBarn; same values) for the stands posts and risers (slot TimberDark): WoodLight (0.08, 0.048, 0.028), WashK 6.5, WashHeightCm 450 (lanes AR2, AR5) (9281 bytes) |
| Game/Plugins/JamAssets/Content/Arena/M_ArenaWood.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | a copy of Look/M_ClayBarn with the instanced-static-mesh usage flag, parent of the stand woods and the barn dressing woods on instanced components; saved without a compile (no renderer in that editor) (lane AR5) (24975 bytes) |
| Game/Plugins/JamAssets/Content/Arena/MI_Barn_Timber.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_ArenaWood for the roof bays, wall bays, gate, loft and props (slot Timber): the stand plank wood with WashK 0 (matte, no wash) (lane AR5) (9241 bytes) |
| Game/Plugins/JamAssets/Content/Arena/MI_Barn_TimberDark.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_ArenaWood (slot TimberDark): the stand dark wood with WashK 0 (lane AR5) (9265 bytes) |
| Game/Plugins/JamAssets/Content/Arena/MI_Barn_NightGlass.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_ClayCrowd for the wall bays windows (slot NightGlass), the one emissive slot: Tint (0.10, 0.16, 0.34), FillK 0.5, FillLow 1.0, PoseK 0; left low for the picture lane to tune to luma 20 to 40 (lane AR5) (6074 bytes) |
| Game/Plugins/JamAssets/Content/Arena/MI_Barn_Cloth.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_ClayCrowd for the bunting (slot Cloth): Tint (0.45, 0.42, 0.36), Sat 0.7, FillK 0.5, PoseK 0 (lane AR5) (5711 bytes) |
| Game/Plugins/JamAssets/Content/Arena/MI_Barn_Hay.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_ClayCrowd for the hay bales (slot Hay): Tint (0.50, 0.42, 0.24), FillK 0.5, PoseK 0 (lane AR5) (5699 bytes) |
| Game/Plugins/JamAssets/Content/Arena/MI_Barn_Paint.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_ClayCrowd for the ice machine body (slot Paint): Tint (0.45, 0.43, 0.38), Sat 0.7, FillK 0.5, PoseK 0 (lane AR5) (5711 bytes) |
| Game/Plugins/JamAssets/Content/Arena/MI_Barn_Rubber.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_ClayCrowd for tyres and hose (slot Rubber): Tint (0.12, 0.12, 0.12), FillK 0.3, PoseK 0 (lane AR5) (5717 bytes) |
| Game/Plugins/JamAssets/Content/Arena/MPC_Crowd.uasset | material | 0.00 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | material parameter collection for the crowd cheer: scalars CrowdCheerPlusX and CrowdCheerMinusX (0 at rest, 0 to 1 while an end answers a goal), written every tick by BP_ArenaCrowd, read by M_ClayCrowd (lane AR5) (1802 bytes) |
| Game/Plugins/JamAssets/Content/Arena/BP_ArenaCrowd.uasset | other | 0.14 | original | UE 5.9 editor via MCP (Blueprint DSL) |  |  | 2026-11-14 | B | y | owner actor of one rink size crowd and stands (instanced components added per actor in the rig): at BeginPlay it destroys itself unless its world scale is 1 within 0.02 (the rig scale times its own 90 / length), else binds JamEventBus.OnJamEvent; on goal it notes the time for the end whose net was scored (Where.X sign) and every tick writes MPC_Crowd CrowdCheerPlusX / CrowdCheerMinusX: up over 8 frames, held, down over frames 88 to 108 (lane AR5) (144178 bytes) |
```
