# SK2 report: sticks in two hands, the C-0001 v3 calls and the puck on the blade, on all four skater figures (device B)

Lane SK2, 2026-11-12, 16:02 to 17:05 local. Tree `C:/Users/kazda/jam/wt/b-vfx`, editor on port 8002. No git command, no build.

**How it was verified**
- Everything ran in the **real game**: PIE with device A's game mode, the rink built by the game, the bot match running.
- `jam.Cut` was a scratch copy of `hockey.test.visual-b2` with `"visual": "humanoid_b1"` for T-B1, and `hockey.test.visual-b2` itself for T-B2.
- Eight test figures `SK2_*` stood beside the boards in the same PIE world for the controlled shots.

**Housekeeping**
- `/Game/Maps/Rink` was never saved: its mtime is still 12:40, and it was discarded through `new_blank_map`.
- No file outside `Plugins/JamAssets/Content/Skaters/` changed under `Game/`. `_Scratch/` does not exist.
- The editor was left through `ExitEditor`, with no dirty package. Port 8002 still shows `LISTENING` with owner pid 0 right after exit (the known ~100 s linger).
- No frame numbers: the port 8000 editor shared the GPU.
- Scratch scripts: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/sk2` (`$SK2`).

## Verdict per step and figure

| Step | T-B1 (p2_cel) | T-B2 hybrid (R) | P3 clay (P3A) | P4 broadcast (P4B) |
|---|---|---|---|---|
| 1 Import / re-point | **PASS** | n/a | n/a | n/a |
| 2 Stick reads as shaft and blade, hands, blade on ice | **PASS** (stride under the aim layer: blade to -7 cm) | **PASS** | **PASS** | **PARTIAL**: the left arm is thrown back in lift, pass and poke |
| 3/4/5 Nine v3 calls | **PASS** (in match + test figure) | **PASS** (in match + test figure) | **PASS** (test figure only, see "could not verify") | **PASS** (test figure only) |
| Puck rides the blade (match) | **PASS**: 5.5 to 5.7 cm in 12 of 12 frames | **PASS**: 4.7 to 6.2 cm in 11 of 12 frames (one 14.7 cm read, see below) | not testable in a match | not testable in a match |
| Numbers 1 7 42 99, crests | **PASS** | **PARTIAL**: the right mitt covers part of the number in the aim pose (known since B12) | **PARTIAL**: small but readable | **FAIL** in the aim pose: the arms cover the back |
| 6 Stills | **PASS** | **PASS** | **PASS** | **PASS** |

**Animation sources**
- T-B1: mesh from `skf/export/p2_cel`; all 31 takes from `skf/export2/p2_cel` (the twist-fixed clips).
- T-B2 (R), P3A and P4B: all 31 takes from **`skh/export3/`** (`R/`, `P3A/`, `P4B/`), animation-only onto the existing skeletons.
- The `export2/R` import that ran first was superseded by `export3` before any measurement.

**Structure (step 5):** `BP_SkaterVisual_TB2_P3/_P4` and `ABP_SkaterTB2_P3/_P4` are **re-derived duplicates** of the finished TB2 assets, not child classes. The route:
1. Delete the old ones and duplicate the TB2 assets.
2. Re-point the AnimBP's skeleton and every player by graph context (`$SK2/a3.py`).
3. Rename the Blueprint's names with a DSL edit (`$SK2/dsled.mjs`).
4. Set the mesh and the anim class (`$SK2/b3.py`).

Child classes would still need the anim interface that lane SKI described. The duplicates therefore have the numbers, crests, the nine calls and the puck socket. Repeat the four steps after any change to TB2.

## Stills (`devlog/shots/`, filed as JPEG)

| File | What |
|---|---|
| `20261112-170013-B-sk2-tb1-carry.jpg` | T-B1, 12-frame carry along the boards |
| `20261112-170013-B-sk2-tb2-carry.jpg` | T-B2, 12-frame carry through a corner |
| `...-sk2-{tb1,tb2,p3,p4}-aim.jpg` | aim sweep: left, front, right, far reach |
| `...-sk2-{tb1,tb2,p3,p4}-verbs.jpg` | row 1 poke, lift, pass, foul; row 2 charge 0 / 0.5 / 1, release; row 3 faceoff ready, faceoff draw, crease hold, block left |
| `20261112-170015-B-sk2-lineup-front.jpg` | all four figures holding sticks, both colours |
| `20261112-170015-B-sk2-lineup-back.jpg` | numbers 1 7 42 99, two rows |
| `20261112-170016-B-sk2-tb1-match.jpg` | T-B1 from the game camera |

## What each call does (all four figures, same graph)

- **`SetStickAim(Angle, Reach)`**: drives `BS_*_Aim`, a 2D blend space with X = angle and Y = reach.
  - Samples: `Aim_Right`, `Aim_Front` and `Aim_Left` at 1.0 m; at 1.6 m, `Aim_FarReach` at the front and the side holds repeated at the sides.
  - The angle is clamped to the figure's own hold angles: T-B1 -63.6/+66.3, T-B2 -63.1/+67.2, P3 -56.3/+63.1, P4 -67.9/+70.0. Reach is clamped to 1.0..1.6.
  - It is a Layered blend per bone from `spine_01` with mesh-space rotation, weighted by `AimW`. `AimW` eases 0.15 per frame to 1 after the first call, and to 0 while down, at a faceoff or in the goalie stance.
- **`PlayPoke(bClean)`**: `Poke`, full body. When not clean, `Stumble` follows 18 frames later (blend-out 0.2 s, which covers the SKF/SKH note about Stumble's end pose).
- **`PlayLift()`**: `Lift`. **`PlayPass(S)`**: `Pass` at rate 0.8 + 0.5·S. **`PlayFoulReaction()`**: `Foul_Reaction`.
  - All are full-body dynamic montages on `DefaultSlot`: blend in 0.08 s, out 0.2 s.
  - No upper-body slot: adding a slot name to the skeleton was not reachable from the tools.
- **`SetShotCharge(C)`**: a sequence evaluator on `Shot_Slap` at time C x 16/60 s (frame 0 to 16, the wind-up top). It feeds a two-way blend with alpha min(1, 5C), so 0 means no influence.
- **`PlayShoot(Charge)`**:
  - Charge > 0: `Shot_Slap` montage starting at min(Charge, last charge) x 16/60 s, the same pose the evaluator shows, so there is no pop (`*-verbs` row 2).
  - Charge 0: a one-frame deferred `Shot_Wrist`, dropped if `PlayPass` arrives in the same frame (the pawn sends `PlayShoot(0)` with every pass).
- **`PlayCheck()`**: `Check`, dynamic montage.
- **`SetFaceoff(b)`**: holds `Faceoff_Ready` through a nested Blend-by-bool chain after the locomotion state machine (0.2 s blends).
- **`PlayFaceoffDraw(bWon)`**: `Faceoff_Draw`; won at rate 1.0, lost at 0.7. There is only one draw take.
- **`SetGoalieStance(bOn, Side)`**: `Goalie_Hold`; Side -1 gives `Goalie_Block_Left` and +1 gives `Goalie_Block_Right`. The blocks hold their last frame. No extra mesh.

**Priorities**
- Knocked down blocks every one-shot, stops montages (0.15 s) and zeroes the aim and charge.
- The one-shot order is foul 5 > shoot and check 4 > poke, lift and pass 3 > faceoff draw 2. A one-shot may interrupt only an equal or lower one.
- For the stances, faceoff beats goalie, and both suppress the aim layer. Aim beats locomotion.

**Calls before BeginPlay** only store variables. `PushAnim` (a cast to the AnimBP) runs on every per-frame call, so nothing breaks before an anim instance exists. The existing eight calls are unchanged.

## Measured aim error (blade centre vs the asked point; test figure in PIE, cm; columns -63 / -30 / 0 / +35 / +70 degrees)

| Figure | 1.0 m | 1.3 m | 1.6 m |
|---|---|---|---|
| T-B1 | 1.4 / 1.9 / 1.3 / 0.6 / 7.8 | 29.9 / 8.7 / 4.5 / 29.4 / 31.3 | 60.0 / 33.3 / 1.1 / 33.5 / 60.9 |
| T-B2 | 2.7 / 1.3 / 1.2 / 1.7 / 2.5 | 29.6 / 9.2 / 6.1 / 29.9 / 30.0 | 59.6 / 36.7 / 2.5 / 35.3 / 60.0 |
| P3 | 12.9 / 4.4 / 4.4 / 4.8 / 14.0 | 32.9 / 3.6 / 1.2 / 25.3 / 32.6 | 61.6 / 31.9 / 2.1 / 34.2 / 61.1 |
| P4 | 1.6 / 0.3 / 0.2 / 4.2 / 2.1 | 30.0 / 29.6 / 12.1 / 7.3 / 29.9 | 61.3 / 45.2 / 9.8 / 35.5 / 59.9 |

**How to read the table**
- At 1.0 m every figure is within 3 cm inside its hold range.
- At the front, 1.6 m is within 1 to 10 cm.
- **Reach beyond 1.0 m at the sides is not covered**: no take has a far reach at an angle, so the side holds stay at 1.0 m (error = asked reach - 1.0).
- Blade height at 1.0 m is -3 to +5 cm. Between the front and the far reach (-30°, 1.3 m) the blend dips the blade **10 to 16 cm into the ice**.

**Measured in the match**
- The pawn sends 1.2 m at rest. Carriers sit at the 1.6 m clamp.
- Carry, puck to blade centre (design offset 5 cm):
  - T-B1: 5.5 to 5.7 cm in 12 of 12 frames.
  - T-B2: 4.7 to 6.2 cm in 11 of 12 frames. One frame read 14.7 cm; that read was taken during a 35° yaw jump, and the puck to `puck_socket` distance was 11.7 cm in that read only.
- Puck to `puck_socket`: 0.0 to 0.6 cm otherwise.
- Blade height while carrying: T-B1 -7.5 to +1.1 cm, T-B2 +3 to +8 cm.

## Hands on the stick (left wrist joint to the shaft line, cm, engine)

| Figure | Glide (no aim) | Stride (no aim) | Aim_Front | Aim_Left | Aim_Right | Far reach | Stride + aim layer | Pass |
|---|---|---|---|---|---|---|---|---|
| T-B1 | 6.8 | 80 (one-handed by design) | 5.7 | 6.2 | 5.1 | 70 (one-handed by design) | 5.7 | on the shaft (stills) |
| T-B2 | 11.5 | 65 (one hand) | 16.1 | 10.1 | 14.8 | 55 (one hand) | 16.1 | 14.5 |
| P3 | 14.0 | 60 (one hand) | 19.2 | 15.3 | 15.4 | 63 (one hand) | 19.1 | 18.3 |
| P4 | 11.1 | 70 (one hand) | 10.2 | 6.0 | 15.8 | 47 (one hand) | 10.2 | 10.2 (but the arm is thrown back in the stills) |

The distance is wrist joint to shaft, so it includes the glove radius. On T-B1, about 5 to 6 cm is "holding". The generated figures' mitts are bigger, and their stills show both mitts on the shaft in glide and in the aims (the line-up still).

In play, every figure carries the aim layer, so both hands are on the stick whenever it is not knocked down or in a one-handed take.

## Blender-side faults (for the Blender lanes)

1. **No far-reach holds at the sides** (all four figures).
   - Effect: 30 to 61 cm aim error at ±30..70° beyond 1.0 m, and the blade dips 10 to 16 cm under the ice between the front and far-reach holds.
   - Ask: `Aim_FarLeft` and `Aim_FarRight` at 1.6 m and about ±60°, and ideally a 1.3 m middle row.
2. **P3A's aim holds** sit at -56.3° and +63.1°, short of the pawn's -63 / +70: 13 to 14 cm error at the extremes.
3. **P4B `Lift`, `Pass`, `Poke` and glide** (`p4-verbs` row 1): the left arm swings back off the shaft, around frames 6 to 11. In the aim stills the arms cross the back and hide the jersey number.
4. **T-B1 `Stride`** under the aim layer: the stride's lower pelvis puts the blade at -7 cm. An engine-side workaround is possible (pelvis offset), but a stride whose pelvis height matches `Glide` fixes it at the source.
5. T-B2's right mitt over the back number in the aim and idle poses (already known from B12).

## Assets created or changed (all `Game/Plugins/JamAssets/Content/Skaters/`, bytes)

**Created**
- `SK_SkaterTB1v2` 1,065,176
- `SKEL_SkaterTB1v2` 11,809
- `SM_SkaterTB1v2_Stick` 80,353
- 31 x `AS_SkaterTB1v2_*`, 4,893,273 in total
- `BS_SkaterTB1v2_Skate` 27,913
- Aim blend spaces: `BS_SkaterTB1v2_Aim` 13,982, `BS_SkaterTB2_Aim` 14,224, `BS_SkaterTB2_P3_Aim` 13,907, `BS_SkaterTB2_P4_Aim` 14,152
- 15 new `AS_SkaterTB2_<stick verb>` takes for the hybrid

**Changed**

| Asset | Bytes | What changed |
|---|---|---|
| `ABP_SkaterTB1` | 527,760 | new skeleton, stick-verb layers, variables `AimAngle AimReach AimW ChargeTime ChargeAlpha bStLoco bStFace bStHold bStRight` |
| `ABP_SkaterTB2` | 528,153 | stick-verb layers |
| `ABP_SkaterTB2_P3` | 523,829 | re-derived |
| `ABP_SkaterTB2_P4` | 523,885 | re-derived |
| `BP_SkaterVisual_TB1` | 782,673 | new mesh; nine v3 events plus `PushAnim`, `PlayAct`, `SK2Tick`, `SK2Down`; 14 variables |
| `BP_SkaterVisual_TB2` | 757,392 | same as TB1 |
| `BP_SkaterVisual_TB2_P3` | 757,448 | re-derived |
| `BP_SkaterVisual_TB2_P4` | 757,612 | re-derived |
| `SK_SkaterTB2` | 3,744,737 | `puck_socket` offset (now 5 cm beyond the blade, on bone `stick_socket`) |
| `SK_SkaterTB2_P3` | 3,551,420 | same |
| `SK_SkaterTB2_P4` | 4,208,603 | same |
| `MI_SkaterTB2_P3` | 13,496 | number box Z 96, W 44, H 30, NumberWidth 0.42, NumberHeight 0.5; crest Y -6, Z 102, size 12 |
| `MI_SkaterTB2_P4` | 11,687 | number box Z 124, W 44, H 30, NumberWidth 0.42, NumberHeight 0.55; crest Y -10, Z 136, size 12 |
| `BS_SkaterTB2_Skate`, `BS_SkaterTB2_P3_Skate`, `BS_SkaterTB2_P4_Skate` | 28,045 / 27,857 / 28,116 | re-saved |
| `AS_SkaterTB2_*`, `AS_SkaterTB2_P3_*`, `AS_SkaterTB2_P4_*` | 93 files, 18,656,134 | contents re-imported from `skh/export3` |

**Unchanged:** `MI_SkaterTB1_JerseyDeco`. UV0 on the new mesh frames the back exactly as on the old one: u = 0.025y + 0.5, v = 4.2667 - 0.0333z, residual 1e-7.

**Ledger rows** (54, ready to paste): `$SK2/rows.md`, copied below.
- For changed files, update the notes only:
  - `AS_SkaterTB2*` rows: "re-imported from skh/export3 (lane SK2)".
  - `BP`/`ABP` rows: "C-0001 v3 calls (lane SK2)".
  - P3/P4 `BP`/`ABP` rows: "re-derived from TB2 by lane SK2".
  - `MI_SkaterTB2_P3/_P4` rows: "number box and crest set (lane SK2)".

```
| Game/Plugins/JamAssets/Content/Skaters/SK_SkaterTB1v2.uasset | mesh | 1.02 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export/p2_cel SK_Skater_p2_cel_loopcut.fbx (T-B1 v2); sockets stick/head on their bones, puck_socket on bone stick_socket 5 cm beyond the blade (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/SKEL_SkaterTB1v2.uasset | other | 0.01 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skeleton of the p2_cel import, root identity |
| Game/Plugins/JamAssets/Content/Skaters/SM_SkaterTB1v2_Stick.uasset | mesh | 0.08 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | skf SM_Stick_p2_cel.fbx (cm); imported, not attached (the stick is skinned into SK_SkaterTB1v2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Aim_FarReach.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Aim_Front.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Aim_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Aim_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Celebrate.uasset | anim | 0.19 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Crossover_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Crossover_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Faceoff_Draw.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Faceoff_Ready.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall_Forward.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Foul_Reaction.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp.uasset | anim | 0.19 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Front.uasset | anim | 0.19 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Glide.uasset | anim | 0.21 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Goalie_Block_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Goalie_Block_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Goalie_Hold.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Idle_Faceoff.uasset | anim | 0.21 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lean_Left.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lean_Right.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lift.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Pass.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Poke.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Shot_Charge.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Shot_Slap.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Shot_Wrist.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Stop.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Stride.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Stumble.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export2/p2_cel loopcut (twist-fixed clips), animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/BS_SkaterTB1v2_Skate.uasset | anim | 0.03 | original | UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | copy of BS_SkaterTB1_Skate's layout on the v2 takes (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/BS_SkaterTB1v2_Aim.uasset | anim | 0.01 | original | UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | 2D aim blend space AimAngle x AimReach from the figure's Aim_Right/Front/Left/FarReach holds (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/BS_SkaterTB2_Aim.uasset | anim | 0.01 | original | UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | 2D aim blend space AimAngle x AimReach from the figure's Aim_Right/Front/Left/FarReach holds (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/BS_SkaterTB2_P3_Aim.uasset | anim | 0.01 | original | UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | 2D aim blend space AimAngle x AimReach from the figure's Aim_Right/Front/Left/FarReach holds (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/BS_SkaterTB2_P4_Aim.uasset | anim | 0.01 | original | UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | 2D aim blend space AimAngle x AimReach from the figure's Aim_Right/Front/Left/FarReach holds (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Aim_FarReach.uasset | anim | 0.16 | original | Blender 4.2 Python script (lanes SKF, SKH) | | | 2026-11-12 | B | y | skf p2_cel clip retargeted onto R by skh retarget.py, skh/export3/R loopcut, animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Aim_Front.uasset | anim | 0.17 | original | Blender 4.2 Python script (lanes SKF, SKH) | | | 2026-11-12 | B | y | skf p2_cel clip retargeted onto R by skh retarget.py, skh/export3/R loopcut, animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Aim_Left.uasset | anim | 0.17 | original | Blender 4.2 Python script (lanes SKF, SKH) | | | 2026-11-12 | B | y | skf p2_cel clip retargeted onto R by skh retarget.py, skh/export3/R loopcut, animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Aim_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (lanes SKF, SKH) | | | 2026-11-12 | B | y | skf p2_cel clip retargeted onto R by skh retarget.py, skh/export3/R loopcut, animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Faceoff_Draw.uasset | anim | 0.16 | original | Blender 4.2 Python script (lanes SKF, SKH) | | | 2026-11-12 | B | y | skf p2_cel clip retargeted onto R by skh retarget.py, skh/export3/R loopcut, animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Faceoff_Ready.uasset | anim | 0.16 | original | Blender 4.2 Python script (lanes SKF, SKH) | | | 2026-11-12 | B | y | skf p2_cel clip retargeted onto R by skh retarget.py, skh/export3/R loopcut, animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Foul_Reaction.uasset | anim | 0.22 | original | Blender 4.2 Python script (lanes SKF, SKH) | | | 2026-11-12 | B | y | skf p2_cel clip retargeted onto R by skh retarget.py, skh/export3/R loopcut, animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Goalie_Block_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lanes SKF, SKH) | | | 2026-11-12 | B | y | skf p2_cel clip retargeted onto R by skh retarget.py, skh/export3/R loopcut, animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Goalie_Block_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lanes SKF, SKH) | | | 2026-11-12 | B | y | skf p2_cel clip retargeted onto R by skh retarget.py, skh/export3/R loopcut, animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Goalie_Hold.uasset | anim | 0.16 | original | Blender 4.2 Python script (lanes SKF, SKH) | | | 2026-11-12 | B | y | skf p2_cel clip retargeted onto R by skh retarget.py, skh/export3/R loopcut, animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Lift.uasset | anim | 0.15 | original | Blender 4.2 Python script (lanes SKF, SKH) | | | 2026-11-12 | B | y | skf p2_cel clip retargeted onto R by skh retarget.py, skh/export3/R loopcut, animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Pass.uasset | anim | 0.15 | original | Blender 4.2 Python script (lanes SKF, SKH) | | | 2026-11-12 | B | y | skf p2_cel clip retargeted onto R by skh retarget.py, skh/export3/R loopcut, animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Poke.uasset | anim | 0.17 | original | Blender 4.2 Python script (lanes SKF, SKH) | | | 2026-11-12 | B | y | skf p2_cel clip retargeted onto R by skh retarget.py, skh/export3/R loopcut, animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Shot_Charge.uasset | anim | 0.16 | original | Blender 4.2 Python script (lanes SKF, SKH) | | | 2026-11-12 | B | y | skf p2_cel clip retargeted onto R by skh retarget.py, skh/export3/R loopcut, animation-only import (lane SK2) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Stumble.uasset | anim | 0.16 | original | Blender 4.2 Python script (lanes SKF, SKH) | | | 2026-11-12 | B | y | skf p2_cel clip retargeted onto R by skh retarget.py, skh/export3/R loopcut, animation-only import (lane SK2) |
```

## Orphans for the lead to delete (nothing references them now)

- **Old T-B1** (3,899,403 bytes): `SK_SkaterTB1`, `SKEL_SkaterTB1`, `BS_SkaterTB1_Skate`, `AM_SkaterTB1_Shot_Slap`, `AM_SkaterTB1_Check`, `MI_SkaterTB1_Jersey`, and 17 `AS_SkaterTB1_*`.
- **Montages now replaced by dynamic montages** (53,571 bytes): `AM_SkaterTB2_Shot_Slap`, `AM_SkaterTB2_Check`, `AM_SkaterTB2_P3_Shot_Slap`, `AM_SkaterTB2_P3_Check`, `AM_SkaterTB2_P4_Shot_Slap`, `AM_SkaterTB2_P4_Check`.
- `SM_SkaterTB1v2_Stick` is imported but unused. Keep it only if A's toy figure wants the shape.

## For device A

- **Class names per look tier** (unchanged):
  - `humanoid_b1` → `/JamAssets/Skaters/BP_SkaterVisual_TB1.BP_SkaterVisual_TB1_C` (now the p2_cel figure).
  - `generated_b2` → `BP_SkaterVisual_TB2_C`.
  - Clay and broadcast tiers → `BP_SkaterVisual_TB2_P3_C` and `BP_SkaterVisual_TB2_P4_C`. These need A's tier-to-class lookup (lane SKI's proposal); without it they cannot appear in a match.
- **Calls that arrived** (polled on the match visuals):
  - `SetStickAim` every frame: rest 1.2 m, carriers at 1.6 m or more (B clamps, so raw values above 1.6 were not seen). Angles swing -62 to +47° in a carry.
  - `PlayShoot`, always with Charge 0.
  - pri-3 one-shots (poke, lift or pass; the poll cannot tell them apart).
  - `PlayFaceoffDraw` at the opening faceoff.
- **Not seen: `SetShotCharge` > 0** in about 40 s of bot play: do the bots ever charge? **Never called: `SetGoalieStance`**: the pawn does not call it; the brain's owner must.
- **`PlayShoot(0)` arrives with every pass**, in the same frame as `PlayPass`. B handles it by deferring a charge-0 shot one frame. A cleaner contract would send `PlayShoot` only for shots.
- The rest reach is 1.2 m on this tree, not 1.0 (R-0022 point 3 not yet applied here, or the test cartridge overrides it).

## What I could not verify

- P3 and P4 in a real match carry (no tier key exists yet). Their calls and stills come from test figures in the same PIE world.
- Test figures placed in the editor world lost their `JerseyMID` / `HelmetMID` variables in PIE, although the MIDs exist on the mesh. Game-spawned visuals are fine: numbers show in the match stills.
  - For the line-up I set the same parameters directly on those MIDs (team colour, crest, number).
  - Root cause not found. It does not affect the game path.
- The poll could not tell which pri-3 one-shot (poke, lift or pass) the pawn sent.
- Frame cost.
- The goalie block re-trigger: going hold → block → hold → block may start the block at its held last frame. The Blend-by-bool "reset child" option was not settable from Python.
- `Stumble` after a mistimed poke in a match (only on the test figure).

## What the lead must decide

1. Ask the Blender lanes for side far-reach holds (fault 1). Engine-side, the alternative is to yaw the whole figure toward the aim at reach above 1.0 m, which also turns the skates.
2. Duplicates vs child classes for P3 and P4. Child classes need the anim interface lane SKI proposed.
3. Delete the orphans above.
4. P4's number placement waits for fault 3. Until then P4 shows numbers only outside the aim pose (not verified).
5. A real upper-body slot for pass and lift (needs a skeleton slot group), so the legs keep striding during a pass.
