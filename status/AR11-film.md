# AR11 film: the stands and the crowd on the night tier, for the GPU

Lane AR11 put the arena kit on `LR_L2_Night` without a renderer. **Nothing of its look has been seen.** The six night materials are new instances of two masters that compile (`M_ClayCrowd`, `Arena/M_ArenaWood`); their values are guesses made in the dark. This is the script for the lane with the GPU, about twenty minutes.

Names:
- the crowd and stands of the current rink on night: the one live actor of class `/JamAssets/Arena/BP_ArenaCrowd` (label `Crowd_L2_Night_k<scale>`); there is no dressing actor on night (no roof, no walls, no props);
- hide and show: `ke * ArenaCrowd 0` and `ke * ArenaCrowd 2` in the console, after the match has started (`1`: the stands without the fans);
- night cuts with the tier set: `scratchpad\ar11\c_nreg.json` (90 x 45), `c41_npond.json` (60 x 30), `c_nlarge.json` (120 x 60), `c41_nwide.json` (140 x 70);
- materials, all under `/JamAssets/Arena/`: `MI_NightCrowd_Fan`, `_Skin`, `_Scarf`, `_Tin` (on `M_ClayCrowd`), `MI_NightBarn_Stand`, `_StandDark` (on `M_ArenaWood`). They are bound as overrides on the night rig's components, not on the meshes: the clay rig is untouched.

## Step 0 (2 minutes): does it draw, and in whose voice

1. A night match on the regular rink. In the log: no "missing usage flag", no "Default Material". In the world: one live `BP_ArenaCrowd`, 15 components, 554 fans, 46 stand pieces.
2. Look from the game camera and from 8 m. The crowd's components do **not** render custom depth, so by the tier's own rule (stencil 0: no ramp, no ink) they are plain lit shapes without outlines. Say which it should be. To try the ink on a running match:

```python
import unreal
W = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
cls = unreal.load_asset('/JamAssets/Arena/BP_ArenaCrowd').generated_class()
for a in unreal.GameplayStatics.get_all_actors_of_class(W, cls):
    for c in a.get_components_by_class(unreal.HierarchicalInstancedStaticMeshComponent):
        c.set_render_custom_depth(True); c.set_custom_depth_stencil_value(1)   # False to take it back
```

Six hundred inked fans may be too loud for a calm rink; my default is off.

## Step 1 (6 minutes): the views

As AR7's, on night:
- the game camera in an **un-paused** night match at two moments (30 and 90 ticks after the first knockdown, `scratchpad\ar7\ab.sh` with the night cut);
- a low camera (1.7 m), the far end from 4.5 m, the bench side from game height;
- Pond and the wide arena: game camera, side boards, an end, from 9 m.

What to look for, because a count cannot tell:
- **the gaps:** ten modules are left out on the long sides where the three low fills behind the boards point (four behind the boxes, six behind the benches), so the crowd has a hole behind each. Say if it reads as an aisle or as a mistake;
- **no corner stands on rinks under 90 m** (Pond, Cage, the 64 m and the 80 m): the light towers and the snow mounds stand where the corner pieces would;
- the stand fronts against the snow banks (1.2 m apart on the regular rink; on the large and wide arena the stand line was moved out by 0.3 and 1.0 m to keep 0.6 m);
- the two end fills (700 cd spots, 12 m up at each end): 83 and 72 fans of the end stands are inside their cones, the nearest 9.9 m from the lamp. Look for a lit patch on the end terraces.

## Step 2 (6 minutes): the readability pairs

Frozen pairs at the game camera in one night match (2.3 s, 6.0 s, 9.5 s), B's arena hidden and shown with the console line above. On night the ice is dark and the puck glows, so clay's "bright neutral pixels" mask does not find the ice. Use:
- **ice:** the medians of the 13 ice regions of `scratchpad\ar2\regs.py` (B41's measure for night); the mean of the 13 within 1 % between hidden and shown;
- **puck:** the mean of the 60 brightest pixels in a 44 x 32 box at the puck's projected place, minus the median of the ring beside it (on night the puck is brighter than the ice, so the number is positive); within 3;
- **jerseys:** the 22-pixel chest box minus the ring's median, as on clay (`scratchpad\ar7\pairm.py`); within 3.

Also write down the share of the frame B's arena takes and the band's mean luma hidden and shown.

## Step 3 (6 minutes): three things to tune

Target: silhouettes with a few lit hats and faces. The band's 99th percentile under the luma of the boards' top in the same frame; the band's mean at most two fifths of it; no crowd sample more saturated than the jerseys' median; never a team colour.

| What | Asset, parameter | Now | Range I expect |
|---|---|---|---|
| The crowd's fill on night | `MI_NightCrowd_Fan`, `FillK`; `Tint` | 0.35; (0.30, 0.36, 0.50) | 0.1 to 0.8; tint 0.2 to 0.5 on all three, keep it cool |
| Faces | `MI_NightCrowd_Skin`, `FillK`; `Tint` | 0.45; (0.34, 0.33, 0.36) | 0.2 to 0.9 |
| The stands' wash | `MI_NightBarn_Stand` and `_StandDark`, `WashK`; `WoodLight` | 1.2; (0.040, 0.050, 0.070) and (0.022, 0.028, 0.040) | 0 to 3; at 0 the stands are lit by the rig's lights alone |
| Windows | none on night | | there are no walls of B's on night |

The fill's colour is a cool (0.55, 0.70, 1.0) on all four crowd instances; scarves are `MI_NightCrowd_Scarf` (`Tint` (0.45, 0.50, 0.60), `FillK` 0.4).

## Step 4 (2 minutes): the cheer and the cost

- A `goal` on the bus at each end (code in `status\AR5-film.md`, step 2c): the scored half rises as on clay; the strip at +0, +8, +30, +60, +100, +110, +130. On night the other device's red goal light is stronger: say how they sit together.
- In the log of a night session: the line `blocking load N ms`, before this lane (the commit before) and after. My renderer-less editor does not write that line. The night rig's file went from 429,073 to 1,346,393 bytes and from 169 to 176 actors.

## What to send back

The pair numbers; the band's share, mean and 99th percentile with the values you set; ink on or off; a yes or no on the gaps, the missing corners on small rinks and the end terraces under the fills; the two blocking-load numbers.
