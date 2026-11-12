# SK2 checkpoint (after step 3): T-B1 is the new scripted figure with the nine C-0001 v3 calls (device B)

Lane SK2, 2026-11-12, started 16:02, checkpoint 16:35. Tree `C:/Users/kazda/jam/wt/b-vfx`, editor on port 8002, no git, no build. `/Game/Maps/Rink` is
open in memory with test actors `SK2_*` and is never saved. Port 8000 editor shares the GPU: no frame numbers. Scratch: `.../scratchpad/sk2` (`$SK2`).
The final report follows in `status/SK2-report.md`. Everything below is verified in the real game (PIE, `jam.Cut` = a scratch copy of
`hockey.test.visual-b2` with `"visual": "humanoid_b1"`), the real match running, plus two test figures spawned beside the boards.

## What works on T-B1 (`BP_SkaterVisual_TB1`, class name unchanged)

- **New figure**: `skf/export2/p2_cel` (the twist-fixed clips; the mesh is from `skf/export/p2_cel`, identical bind). The figure has a face, mittens, a helmet, and a shaft-and-blade stick that is skinned into the mesh. Root identity, scale 1, 24 bones, 173 cm, faces +X. No socket scale compensation.
- **Sockets**: `stick_socket` on its bone; `head_socket` on its bone with roll -90, so it has world identity at rest; `puck_socket` on **bone `stick_socket`** at (6.20, -155.81, 10.39) cm. That puts it 5 cm beyond the blade centre, away from the skater, at puck height, so it **moves with the blade**.
- **Blade centre** in the `stick_socket` bone frame is (5.6, -149.57, 10.5) cm. I solved it from the four aim holds against lane SKF's reaches; it agrees within 0.15 cm across all four.
- **Jersey numbers**: UV0 on the new mesh is the same map as on the old one (u = 0.025 y + 0.5, v = 4.2667 - 0.0333 z, fit residual 1e-7), so `MI_SkaterTB1_JerseyDeco` works unchanged. "1" and "11" read on the back in match stills. 7, 42, 99 and the crest follow in the step 6 line-up.
- **Carried puck rides the blade**: in the match, 12 frames of a carry through a turn give puck to blade centre **5.5 to 5.7 cm** (the designed 5 cm offset), and puck to `puck_socket` 0.0 to 0.3 cm.
- **Aim** (`SetStickAim`; blend space `BS_SkaterTB1v2_Aim`, a mesh-space upper-body layer from `spine_01`): **error 0.7 to 1.9 cm at reach 1.0 m** for all angles, and 1.1 cm at the front at 1.6 m. The 1.6 m reach at the sides is not covered, because there is no side far-reach hold: the error is 56 to 59 cm at ±60° and 28 to 33 cm at ±30°. Blade height is -3 to +2 cm at 1.0 m. In the stride under the layer it reaches **-7 cm (into the ice)**.
- **Hands**: wrist-to-shaft 5.1 to 6.4 cm (the glove radius, i.e. holding) in glide, Aim_Front, Aim_Left, Aim_Right, and in stride with the aim layer. Stride without the layer and Aim_FarReach are one-handed by design.
- **Calls that arrived from the pawn** (polled on the six match visuals):
  - SetStickAim every frame. The reach is 1.2 m at rest.
  - PlayShoot, always with Charge 0, so the wrist shot plays.
  - pri-3 one-shots (poke, lift or pass).
  - A faceoff draw.
  - **Not seen: SetShotCharge > 0, SetGoalieStance** (A never calls the latter).
  - All nine calls were also driven on the test figure (stills in `$SK2/img/S_tb1_aim.png` and `S_tb1_verbs.png`).

## Assets created or changed (all under `Game/Plugins/JamAssets/Content/Skaters/`)

New:
- `SK_SkaterTB1v2` 1,065,176
- `SKEL_SkaterTB1v2` 11,809
- `SM_SkaterTB1v2_Stick` 80,353 (imported, not attached; the stick is skinned into the mesh)
- `BS_SkaterTB1v2_Skate` 27,913
- `BS_SkaterTB1v2_Aim` 13,982
- 31 x `AS_SkaterTB1v2_<Take>`, 130,793 to 221,363 each

Changed:
- `ABP_SkaterTB1` 527,760: skeleton re-targeted. New layers: four nested Blend Poses by bool (loco / Faceoff_Ready / Goalie_Hold / Goalie_Block_L/R), Layered blend (aim), Two-way blend (shot-charge evaluator on Shot_Slap). New variables: AimAngle, AimReach, AimW, ChargeTime, ChargeAlpha, bStLoco, bStFace, bStHold, bStRight.
- `BP_SkaterVisual_TB1` 782,673: mesh `SK_SkaterTB1v2`, the nine v3 events, `PushAnim`, `PlayAct`, `SK2Tick`, `SK2Down`, and 14 variables.
- `SK_SkaterTB1v2` slots use the existing `MI_SkaterTB1_*` by name. The Jersey slot uses `MI_SkaterTB1_JerseyDeco`.

Ledger rows: in the final report.

## Orphans for the lead to delete (nothing references them now; 3,899,403 bytes)

`SK_SkaterTB1`, `SKEL_SkaterTB1`, `BS_SkaterTB1_Skate`, `AM_SkaterTB1_Shot_Slap`, `AM_SkaterTB1_Check`, `MI_SkaterTB1_Jersey`, and the 17 `AS_SkaterTB1_*` (Celebrate, Check, Crossover_Left/Right, Fall, Fall_Forward,
GetUp, GetUp_Front, Glide, Idle_Faceoff, Lean_Left/Right, Shot_Slap, Shot_Wrist, Stop, Stride).
