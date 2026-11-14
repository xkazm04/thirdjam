# AR5 film: fifteen minutes on the GPU for what lane AR5 built blind

Lane AR5 worked in an editor without a renderer (`-nullrhi`). Nothing in it was seen, and **no material was compiled**. This is the script for the picture lane, in the order that fails fastest. Everything is in branch `lane/b-arena` (tree `C:\Users\kazda\jam\wt\b-arena`); run it after the lead has merged it, in the GPU editor.

Names used below:
- the crowd and stands of the current rink: the one actor of class `/JamAssets/Arena/BP_ArenaCrowd` left alive in the play world (label `Crowd_P3_Clay_k<scale>`, for example `k1000` on the regular rink);
- the roof bays, wall bays and bunting: the actor labelled `Dress_P3_Clay` (a `StaticMeshActor` with three instanced components);
- materials: `/JamAssets/Look/M_ClayCrowd` with `MI_ClayCrowd_Fan`, `_Skin`, `_Scarf`, `_Tin`, `_Cream`; `/JamAssets/Arena/M_ArenaWood` (a copy of `M_ClayBarn` with the instanced-mesh flag) with `Look/MI_ClayBarn_Stand`, `_StandDark` and `Arena/MI_Barn_Timber`, `_TimberDark`; `Arena/MI_Barn_NightGlass`, `_Cloth`, `_Hay`, `_Paint`, `_Rubber` (on `M_ClayCrowd`);
- the cheer's scalars: `/JamAssets/Arena/MPC_Crowd`: `CrowdCheerPlusX`, `CrowdCheerMinusX`.

## Step 0 (2 minutes): do the two masters compile, and does the crowd draw

1. Open `M_ClayCrowd` and `M_ArenaWood` (or `unreal.MaterialEditingLibrary.recompile_material` on each) and read the log for `LogMaterial: Error`. The new part of `M_ClayCrowd` is stock nodes only, no new Custom node.
2. If `M_ClayCrowd` fails, the two new things in it are, in the order I would suspect them:
   - the coat and accent values: the vertex colour's alpha into two `If` nodes (over 0.9: `PerInstanceCustomData` 1; over 0.3: `PerInstanceCustomData` 2; else 1), through a `VertexInterpolator` into a `Multiply` with the `Tint` parameter, into input `Tint` of the Custom node `AR2CrowdBase`. To cut it: wire the `Tint` parameter straight into `AR2CrowdBase.Tint`.
   - the pose: `TextureCoordinate` 2, 3, 4 (their R) appended to a vector, `Transform` Local to World, times the amount, into World Position Offset. To cut it: disconnect World Position Offset.
3. Start a clay match on the regular rink. In the log: one `JAMLOOK tier=p3_clay ... scale=1.000`. In the world: exactly one `BP_ArenaCrowd` actor (the other six destroy themselves at BeginPlay), 19 instanced components on it, 664 fan instances.
4. Look once from the game camera and once from 8 m: fans sit on the benches, not in them or over them; no grey default material anywhere; the far side shows the far level (no faces) and the near side the full figures.

If step 0 fails and cannot be mended in five minutes: say so and stop; the rig of AR2 is the commit before the merge.

## Step 1 (4 minutes): the readability pairs

As AR2 (`scratchpad\ar2\pair.sh`, `pairm.py`): one clay match with the scripted human, frozen at 2.3 s, 6.0 s and 9.5 s (`slomo 0.0001`), the game camera's frame twice in the same state. Hide and show with:

```python
import unreal
W = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
cls = unreal.load_asset('/JamAssets/Arena/BP_ArenaCrowd').generated_class()
def crowd_hide(h):
    for a in unreal.GameplayStatics.get_all_actors_of_class(W, cls): a.set_actor_hidden_in_game(h)
    for a in unreal.GameplayStatics.get_all_actors_of_class(W, unreal.StaticMeshActor):
        if a.get_components_by_class(unreal.HierarchicalInstancedStaticMeshComponent): a.set_actor_hidden_in_game(h)
```

Pass: ice mean luma within 1 % (AR2: 0.03 %), puck against the ice within 3 (AR2: 1.5), each jersey within 3 (AR2: 2.4). Also write down the share of the frame the stands and crowd take (AR2: 8 to 11 %) and the band's mean luma (AR2: 28 to 41 at the game camera).

Then the same pair once on the wide arena (cut `c_wide.json`): the fans are now 1.4 m tall there, and the barn is much larger than the stands; this is the frame the operator has not seen.

## Step 2 (6 minutes): three things to tune by eye and number

### 2a. The crowd reads as people, not as a lattice

Target: band mean luma 45 to 55 against the ice's 165, 99th percentile under about 110, and the jerseys still the most saturated things in the frame (compare max minus min of RGB: the band's 95th percentile against the jerseys' median).

| What | Asset, parameter | Now | Range I expect |
|---|---|---|---|
| The fans' own light | `MI_ClayCrowd_Fan`, `FillK` | 1.4 | 1.4 to 3.0 |
| Coats lighter or darker | `MI_ClayCrowd_Fan`, `Tint` | (1, 1, 1) | 1.0 to 1.5 on all three |
| Coat colour | `MI_ClayCrowd_Fan`, `Sat` | 0.8 | 0.6 to 0.9; lower it if the band's colour comes near a jersey's |
| Faces | `MI_ClayCrowd_Skin`, `FillK`; `Tint` | 0.4; (0.55, 0.44, 0.36) | 0.4 to 0.8; faces were pale dots above this in AR2 |
| Scarves lighter | `MI_ClayCrowd_Scarf`, `Tint`; `FillK` | (1, 1, 1); 0.7 | leave the tint white (the colour is in the vertices); FillK 0.7 to 1.6 |
| The bench structure behind them | `MI_ClayBarn_StandDark`, `WashK` (and `MI_ClayBarn_Stand`, `WashK`) | 6.5 (6.5) | 2 to 6.5: lowering the dark wood's wash is what takes the lattice back; the fans then carry the band |

What changed with AR6's colours (the figures were reimported from `fbx-ar6`; the vertex colour's alpha came through with its six part values on the full and the far level):
- the colours are in the vertices now: `MI_ClayCrowd_Scarf`'s tint is white, and the fill colour of `MI_ClayCrowd_Fan`, `_Skin`, `_Scarf` and `_Tin` is a neutral (0.71, 0.71, 0.71) in place of the warm (1, 0.62, 0.36), so cream hats do not turn orange. `MI_ClayCrowd_Skin`'s tint is still AR2's darkening (0.55, 0.44, 0.36); AR6 says it can come back up: try toward (0.8, 0.7, 0.62) and watch for pale dots.
- each fan has a coat value in per-instance data 1 (0.60 to 1.30) and an accent value in data 2 (0.80 to 1.15, on hat, scarf and mittens), fixed per seat. The spread is two lines in the placing script (`scratchpad\ar5\layout.py`, `f['coat']`, `f['accent']`).

Two more numbers to take here, from the Blender lane:
- **the head-to-head luma spread** of the crowd at the game camera (the standard deviation of the heads' luma across the far band): Blender measured 24.8 with AR6's colours against 10.6 before, at a band mean of 48 and ice at 164. Say what the engine gives.
- **the share of crowd pixels more saturated than the jerseys** (max minus min of RGB above the jerseys' median): target 0.

### 2b. The windows

`Arena/MI_Barn_NightGlass`: `FillK` 0.5, `Tint` (0.10, 0.16, 0.34), `FillLow` 1.0 (so the light is even across the pane). Target luma 20 to 40 on screen, never above 60, no bloom. Expect `FillK` between 0.3 and 3. The windows sit above 9.75 m and are in neither game-camera frame: judge from a low camera at centre ice, 1.5 m up, looking 9 degrees up at a long wall.

### 2c. The cheer on a goal

Stage a goal on the bus (the rink's lamp answers too):

```python
gi = unreal.GameplayStatics.get_game_instance(W); bus = unreal.get_game_instance_subsystem(gi, unreal.JamEventBus)
p = unreal.JamEventPayload(); p.set_editor_property('who', 0); p.set_editor_property('where', unreal.Vector(4100.0, 0.0, 30.0)); p.set_editor_property('strength', 1.0)
bus.broadcast('goal', p)     # x > 0: the fans at x > 0 answer; x < 0: the other half
```

The scalar rises over 8 frames, holds, falls from frame 88 and is 0 at frame 108. Take the game camera's frame at rest and 30 frames after the goal, with the camera looking at the scored end, and count the pixels that change by more than 8 luma. Target: at least 5 % of the frame on the scoring side (AR3 measured 3.1 % with every fan answering).

| What | Asset, parameter | Now | Range |
|---|---|---|---|
| How far the pose goes | `PoseK` on `MI_ClayCrowd_Fan`, `_Skin`, `_Scarf`, `_Tin` | 1.0 | 1.0 to 1.5. **All four must hold the same value**, or coat, face and scarf of one fan part ways |
| Hold a pose to judge it | console: nothing; in Python `unreal.MaterialLibrary.set_scalar_parameter_value(W, unreal.load_asset('/JamAssets/Arena/MPC_Crowd'), 'CrowdCheerPlusX', 1.0)` is overwritten every tick by the crowd's Blueprint; freeze the match (`slomo 0.0001`) 30 frames after a staged goal instead | | |

If 5 % is not reached with `PoseK` 1.5, the next lever is not built: a lift of the fans' fill while the scalar is up (one `Lerp` on `FillK` in `M_ClayCrowd`, read from the same amount). Say what share you measured and I will add it.

Also look for: arms passing through a neighbour (AR3 says hands reach 0.52 m to the side), a fan cut off at the top by culling while cheering (the meshes' bounds were raised by 95 cm up and 55 cm to the sides for this), and whether the far level's cheer reads the same as the full figure's across the switch.

## Step 3 (3 minutes): what only eyes can pass

- The switch between the full figure and the far level (set for about 30 m at a 70 degree view): walk a camera from 20 m to 40 m toward the far side; say if it pops.
- The corner rows: 68 seats a corner, about three in four filled; seats follow the four straight benches by my reading of the mesh. Say if fans float or sink there.
- The roof: ten `SM_Barn_TrussBay` at x = -60 + 12 k, eave at 13.5 m, squeezed to 0.82 in height so the top meets the old trusses' top (21.87 m). The rig's ten old trusses are deleted. Say if rafters or purlins poke through the roof slabs or the lamp cords hang from nothing.
- The walls: 32 `SM_Barn_WallBay` stand 0.58 m in front of the old walls; the old walls and posts are still there. Say if the old posts and the new ones fight.
- Bunting: 12 lengths across the two ends at x = +-53 m, hung at 13.05 m (8.7 m on Pond). In 24 sampled game-camera frames on four rinks none was in frame; that is a sample, not a proof.
- The gate, the loft (two bays), the props and the ice machine at the +x end wall. On Pond, Cage and the 64 m rink the loft is left out (its ladder would stand in the end stand).
- One still per rink size, as AR2.

## What to send back

The three pairs' numbers, the band's mean and 99th percentile after tuning with the values you set, the head-to-head spread and the share of crowd pixels more saturated than the jerseys, the windows' luma and `FillK`, the cheer's share of the frame and `PoseK`, and a yes or no on each line of step 3.
