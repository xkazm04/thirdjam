# SK3 report: the aim grid in the engine, and the figures' finish per look tier (device B)

Lane SK3, 2026-11-12, 17:26 to 18:50 local. Main tree `C:/Users/kazda/jam/thirdjam`, editor on port 8000, left through `ExitEditor` with no dirty package.
No git command, no build. Checkpoint: `status/SK3-checkpoint.md`.

**Housekeeping**
- `/Game/Maps/Rink` was never saved (mtime still 11:10). It was discarded through `new_blank_map`.
- The only files changed under `Game/` are in `Plugins/JamAssets/Content/Skaters/`. `Props/` and `VFX/` are untouched, because item 6 was not done.
- `_Scratch/` does not exist. `jam.Cut` is back to `hockey.regular`.
- No frame numbers: another editor may have shared the GPU.
- Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/sk3` (`$SK3`). It holds every script, the raw `res_*.json` and `m_*.json` measurements, and the full-size images in `img/`.

**How it was verified:** everything ran in PIE, the real game.
- Night: `jam.Cut` was a scratch copy of the SK2 cartridge (`visual: humanoid_b1`, `l2_night`).
- Clay and broadcast: lane B14's cartridges and in-place figure swap.
- Test figures were spawned in the unsaved editor world: 39 per figure, one for each aim point.

## Verdict

| Item | T-B1 (scripted) | T-B2 hybrid (R) | P3 clay | P4 broadcast |
|---|---|---|---|---|
| 1 / 3 Re-import + 24-hold blend + measurements | **PASS** | **PASS** | **PASS** | **PASS**, but crossovers dip to -7 cm on the slow test rig (see the tables) |
| 1 Crossover / lean / idle cure | **PASS** (stick correction) | **PASS** | **PASS** | **PARTIAL** (crossover, test rig only) |
| 2 Nine calls | **PASS** | not re-driven (same graph and take names) | not re-driven | not re-driven |
| 2 Back number from the game camera (Glide, Stride, aim) | **PASS** | **PASS** | **PASS** | **PASS**: the left glove covers part of it at aim +70 / 1.6 |
| 2 Carry | **PASS**: 3.0 to 6.5 cm | not measured | not measured | not measured |
| 3 P4: left hand in Lift / Pass | – | – | – | **PARTIAL**: on the shaft (8 to 11 cm) at the one frame sampled early in the take; mid-take frames not sampled |
| 4a Night rim | **PASS**: RimNight 1.3 | **PASS** | – | – |
| 4b Broadcast via `MPC_Look` | **PASS**: masters gate on Clay < -0.5 | **PASS** | **PASS** | **PASS**: verified in the broadcast rig (Clay read -1.0) |
| 4c Clay stick | **PASS**: darker, warmer wood, all figures | | | |
| 5 Orphans | **PASS**: 29 deleted, none kept | | | |
| 6 Puck readability | **NOT DONE** (time) | | | |
| 7 Stills | **PASS** (puck comparison missing, see item 6) | | | |

## How the aim layer is built now (same on all four AnimBPs, so it can be rebuilt)

**1. Blend space `BS_<fig>_Aim`**
- 24 samples at their true positions:
  - X = `AimAngle`: -63, -46, -30, 0, 35, 70 (negative = the skater's right = `N`);
  - Y = `AimReach`: 1.00, 1.30, 1.45, 1.60;
  - sample `Aim_<N|P><|angle|>_R<cm>`.
- Axis ranges -63..70 and 1.0..1.6. Grid lines 7 and 4 are for display only.
- `interpolate_using_grid` false, `snap_to_grid` false, input smoothing 0.

**Why one 2D blend space player is not enough**
- UE triangulates the samples. Each rectangular cell becomes two triangles, so a cell centre is a 50/50 blend of two diagonal holds.
- It blends local-space rotations per bone. Mesh-space blending only exists with per-bone smoothing overrides.
- Measured at the 15 cell centres on T-B1:
  - 15.3 cm worst with the default (TANGENTIAL) diagonal;
  - 8.8 cm with RADIAL.

**2. AnimGraph (bilinear)**
- Four Blend Space Players of that same asset:
  - Y pinned to 1.0 / 1.3 / 1.45 / 1.6. A row is always a triangle edge, so each player is linear between the two holds of its row.
  - X = `AimAngle`.
  - Play rate 0, so each hold is evaluated at frame 0 (this avoids the frame-8 free-hand jump of the one-handed holds).
- They are chained through three Two-way blends with alphas `T1`, `T2`, `T3`. `PushAnim` writes them:
  - `T1 = clamp((r - 1.0) / 0.3)`
  - `T2 = clamp((r - 1.3) / 0.15)`
  - `T3 = clamp((r - 1.45) / 0.15)`
- The result feeds the existing Layered blend per bone (from `spine_01`, mesh-space rotation, weight `AimW`) over locomotion.

**3. Blade-on-ice correction, after the layer**
- Node chain: Local→Component, `Transform (Modify) Bone` on bone `stick_socket` (rotation and translation additive, in component space, from ABP variables `StickRot` and `StickOff`), Component→Local, then the existing charge blend.
- It is driven by the Blueprint event `SK3Fix`, called from `SK2Tick` once per game time. Each frame it reads the blade centre (the socket transform plus the figure's blade offset) and updates three integrators:
  - **pitch** about the grip (horizontal axis ⟂ shaft): height to `H0`;
  - **yaw** about the grip: sideways error to the asked point;
  - **slide** along the shaft: reach error.
- Settings:
  - gain `FixGain` 0.6;
  - clamps ±12°, ±12°, ±15 cm;
  - scaled by `AimW`;
  - decays ×0.8 per frame while a montage plays or `AimW` < 0.5.
- `H0` (blade-centre height target): T-B1 4.5, R 5.8, P3 5.2, P4 6.7 cm. This puts the blade bottom about 0.5 to 1 cm above the ice.

**4. Clamp:** `SetStickAim` angle is now -63..+70 on all four (was -63.6/+66.3, -63.1/+67.2, -56.3/+63.1, -67.9/+70.0). Reach is 1.0..1.6.

**Cure chosen for crossovers, leans and idle:** the same correction. The cheaper options (scaling the layer weight down, or an additive lift of the stick hand) were not needed.

## Aim error tables (test figures, cm)

How to read them:
- "err": horizontal distance from the blade centre to the asked point.
- "bottom": blade bottom above the ice, calibrated on the exact holds (blade centre minus 2.9 cm on T-B1, 5.3 on R, 4.7 on P3, 5.6 on P4).
- Stride, crossover and lean sample 2 to 4 cycle phases.
- On the test rig the feedback is stepped only about 3 times a second (one Python call per step), so in moving states the correction lags a cycle that the game updates every frame. Moving-state numbers are therefore pessimistic.

**Before the correction (bilinear blend only)**

| Figure | Glide grid err / bottom | Glide cells err / bottom |
|---|---|---|
| T-B1 | 0.2 / 1.5..2.5 | **6.7** / **-6.5**..2.1 |
| R | 9.7 (the holds themselves, Blender says 6.1) / 0.2..1.7 | **11.1** / **-7.2**..0.8 |
| P3 | 2.6 / 0.1..3.8 | 6.7 / **-4.8..8.7** |
| P4 | 4.5 / 0.8..2.5 | **10.8** / **-4.9**..1.2 |

T-B1 without the correction, other states:

| State | Cells err | Blade |
|---|---|---|
| Stride | 7.9 | to -5.2 + 2.9 offset → -8.1 |
| Crossover | 9.2 | -17 |
| Idle (speed 0 plays `Idle_Faceoff`, hips 10 cm low) | 11.9 | -20 |

**With the correction**

| Figure | State | Grid err / bottom | Cells err / bottom | Most correction used (pitch / yaw / slide) |
|---|---|---|---|---|
| T-B1 | Glide | 0.3 / 0.9..1.2 | 0.3 / 0.9..1.2 | 3.5° / 3.1° / 2.4 |
| | Stride | 2.8 / 0.4..2.6 | 2.8 / 0.5..2.5 | 4.0° / 3.7° / 4.7 |
| | Crossover L | 0.8 / -1.7..3.3 | 0.8 / -1.7..3.3 | 5.8° / 4.2° / 6.6 |
| | Crossover R | 1.0 / -1.4..1.1 | 0.9 / -1.4..1.1 | 5.6° / 4.1° / 6.0 |
| | Lean L | 0.0 / 1.6 | 0.0 / 1.6 | 5.8° / 4.2° / 6.7 |
| | Idle | 0.0 / 2.2..2.4 | 0.0 / 2.2..2.4 | 6.6° / 3.2° / 10.2 |
| R | Glide | 0.1 / 0.3..0.5 | 0.1 / 0.3..0.5 | 4.0° / 3.8° / 6.2 |
| | Stride | 2.9 / 0.7..2.7 | 2.8 / 0.8..2.7 | 3.2° / 3.3° / 6.9 |
| | Crossover L | 2.1 / -2.1..2.3 | 1.9 / -2.1..2.3 | 4.3° / 3.8° / 12.0 |
| | Crossover R | 1.8 / 1.9..2.7 | 1.6 / 1.9..2.6 | 2.9° / 5.0° / 10.3 |
| | Idle | 0.2 / 0.8 | 0.2 / 0.8 | 6.1° / 3.5° / 5.2 |
| P3 | Glide | 0.0 / -0.2..0.1 | 0.0 / -0.2..0.1 | 3.8° / 3.0° / 2.9 |
| | Stride | 2.0 / -1.9..1.1 | 1.8 / -1.8..0.9 | 4.1° / 3.1° / 3.9 |
| | Crossover L | 1.9 / -3.2..1.7 | 1.5 / -3.2..1.7 | 6.6° / 3.0° / 7.4 |
| | Crossover R | 2.3 / -0.3..2.6 | 2.0 / -0.3..2.6 | 8.8° / 4.6° / 10.0 |
| | Idle | 0.0 / 0.7 | 0.0 / 0.7 | 3.9° / 2.7° / 8.0 |
| P4 | Glide | 0.6 / 0.2..2.0 | 0.5 / 0.2..2.0 | 3.6° / 2.0° / 7.4 |
| | Stride | 4.8 / -2.5..5.5 | 4.6 / -2.4..5.2 | 3.1° / 2.5° / 6.4 |
| | Crossover L | 1.1 / **-7.0**..4.9 | 1.0 / **-6.9**..4.9 | 3.0° / 3.1° / 13.0 |
| | Crossover R | 4.2 / **-5.2**..2.9 | 4.0 / -5.1..2.9 | 6.0° / 3.9° / 14.9 |
| | Idle | 0.3 / 2.1 | 0.3 / 2.1 | 7.3° / 1.9° / 8.2 |

**Hands:** the correction does not move them. Left wrist to the shaft line on two-handed holds, with and without it:
- T-B1 4.3 to 7.0 cm in every state (glove radius about 5 to 6 cm).
- R mean 17, P3 16, P4 19 cm (mitten radii 13.3 / 14.0 / 9.3 cm).
- The generated figures' "shaft line" is approximate (the blade-offset direction), so treat their absolute values as rough.

**Bot match, T-B1** (240 polls of the six visuals with the aim layer on and no montage):

| | Correction on | Correction off |
|---|---|---|
| Blade bottom p5 / median / p95 | -0.7 / 1.6 / 3.1 cm | -14.7 / -5.2 / -1.3 cm |
| Share within -1..+4 cm | **93 %** | 5 % |
| Position error median | 1.1 cm | |

- The position-error p90 of about 10 cm is the measurement's one-frame lag on a turning skater (constant 10 cm at turn rate 5 and reach 1.2 m), plus carriers whose asked angle swings 30 to 130° between frames.
- Single outliers to +14 / -9.5 cm blade height occur in Stride for fast-swinging carriers.
- The generated figures were not measured in a match: they cannot be selected through the game yet.

## Item 2

- **Nine calls, T-B1 test figure:**
  - `PlayPoke`: clean → `Poke`; mistimed → `Poke`, then `StumbleIn` 18.
  - `PlayLift` → `Lift`. `PlayPass` → `Pass` at rate 0.8 (S = 0) / 1.3 (S = 1).
  - `PlayFoulReaction` → `Foul_Reaction`, priority 5.
  - `SetShotCharge(0.6)` → ChargeAlpha 1.
  - `PlayShoot(0.6)` → `Shot_Slap`. `PlayShoot(0)` → `Shot_Wrist` one frame later.
  - `PlayCheck` → `Check`, priority 4.
  - `SetFaceoff` → bStFace. `PlayFaceoffDraw` → won rate 1.0, lost 0.7.
  - `SetGoalieStance(-1 / +1)` → block left / right.
- **Back numbers:** readable from game-camera height (6 m behind, 3.5 m up) in Glide, Stride, aim +70 / 1.6 and aim -63 / 1.6 on all four figures (`sk3-back-*`). The test figures need `deco()` (SK2's known MID issue). The game path is unaffected.
- **Carry (T-B1, match):** puck to blade centre 3.0 to 6.5 cm over 17 carried polls through turns of yaw -165 → -99 → 0. Puck to socket 0.0 to 2.0 cm.

## Item 4: what the masters read from `MPC_Look`

`M_SkaterTB2`, `M_SkaterGen` and `M_SkaterTB1Jersey` each gained Custom gates in group "Tiers (SK3)", using the existing `Clay` collection parameter:

| Gate | What it does | Masters |
|---|---|---|
| `SK3RimGate` | The cel rim strength becomes 0 if Clay < -0.5. It becomes `RimNight` if \|Clay\| < 0.5 and `RimNight` > 0. Otherwise it stays `RimStrength`. The clay felt path is untouched. | all three |
| `SK3EmissiveGate` | The emissive fill (including `SetBoost`'s value) becomes 0 if Clay < -0.5. | all three |
| `SK3JerseyLumGate` | Under broadcast, `JerseyLum` becomes `BcastJerseyLum` (default 0.25). | `M_SkaterTB2`, `M_SkaterGen` |

- **Rim value chosen: `RimNight` 1.3** (master default). Compared from stills at 0.6, 1.0, 1.3 and 1.6, in the dark band at the game camera and at 30 m, and in a pool:
  - 1.3 and 1.6 give a lighter edge on the blue figure against the band;
  - 1.6 does not wash out in the pool, but adds nothing over 1.3;
  - the measured change is small (blue figure mean luma 34 → 35 → 35 → 36).
  - Clay and broadcast are unaffected (gated).
- **T-B1's `M_Toon` slots** (helmet, pads, gloves, stick) keep their instances' rim 0.6. `M_Toon` already turns rim and fill off under broadcast (B14).
- **Broadcast, verified in `LR_P4_Broadcast`** (Clay read -1.0): P4 figures with the plain `MI_SkaterTB2_P4` show no fill and no rim, with cloth shading (`sk3-tiers`, right). `MI_Bcast_SkaterP4` is no longer needed.
- **Clay stick:** `MI_SkaterTB1_Stick` and `MI_SkaterTB2_Stick` `TeamColour` changed from (0.55, 0.40, 0.20) to **(0.30, 0.15, 0.05)**, a darker, warmer wood. It reads on cream ice (`sk3-tiers`, middle) and still reads on night ice (`sk3-rim-*`). This applies to every tier, because the stick instances have no per-tier switch.

## Item 5: orphans

Each was reference-checked through the asset registry; every referencer was inside the set itself. A grep of `Game/Source`, `Config` and `Content` found no name. All were deleted through `EditorAssetLibrary.delete_asset`. **None kept.** 29 files, 4,033,327 bytes, all under `Game/Plugins/JamAssets/Content/Skaters/`:

| File | Bytes |
|---|---:|
| `SK_SkaterTB1.uasset` | 941,463 |
| `SKEL_SkaterTB1.uasset` | 11,633 |
| `BS_SkaterTB1_Skate.uasset` | 29,139 |
| `MI_SkaterTB1_Jersey.uasset` | 9,245 |
| `SM_SkaterTB1v2_Stick.uasset` | 80,353 |
| `AM_SkaterTB1_Check.uasset` | 8,921 |
| `AM_SkaterTB1_Shot_Slap.uasset` | 8,719 |
| `AM_SkaterTB2_Check.uasset` | 9,108 |
| `AM_SkaterTB2_Shot_Slap.uasset` | 8,912 |
| `AM_SkaterTB2_P3_Check.uasset` | 8,997 |
| `AM_SkaterTB2_P3_Shot_Slap.uasset` | 8,654 |
| `AM_SkaterTB2_P4_Check.uasset` | 9,132 |
| `AM_SkaterTB2_P4_Shot_Slap.uasset` | 8,768 |
| `AS_SkaterTB1_Celebrate.uasset` | 206,216 |
| `AS_SkaterTB1_Check.uasset` | 168,112 |
| `AS_SkaterTB1_Crossover_Left.uasset` | 155,707 |
| `AS_SkaterTB1_Crossover_Right.uasset` | 159,568 |
| `AS_SkaterTB1_Fall.uasset` | 166,233 |
| `AS_SkaterTB1_Fall_Forward.uasset` | 167,522 |
| `AS_SkaterTB1_GetUp.uasset` | 215,483 |
| `AS_SkaterTB1_GetUp_Front.uasset` | 215,819 |
| `AS_SkaterTB1_Glide.uasset` | 226,168 |
| `AS_SkaterTB1_Idle_Faceoff.uasset` | 226,171 |
| `AS_SkaterTB1_Lean_Left.uasset` | 144,573 |
| `AS_SkaterTB1_Lean_Right.uasset` | 145,347 |
| `AS_SkaterTB1_Shot_Slap.uasset` | 182,985 |
| `AS_SkaterTB1_Shot_Wrist.uasset` | 149,441 |
| `AS_SkaterTB1_Stop.uasset` | 177,030 |
| `AS_SkaterTB1_Stride.uasset` | 183,908 |

SK2 counted 17 old `AS_SkaterTB1_*` takes; 16 existed.

**Ledger rows to remove:** the rows of these 29 files.

## Assets created or changed (`Game/Plugins/JamAssets/Content/Skaters/`, bytes)

**Created:** 96 animation sequences, the 24 grid holds per figure:

| Set | Bytes |
|---|---:|
| `AS_SkaterTB1v2_Aim_<N|P>xx_Rxxx` | 3,426,215 |
| `AS_SkaterTB2_Aim_*` | 4,206,366 |
| `AS_SkaterTB2_P3_Aim_*` | 4,236,173 |
| `AS_SkaterTB2_P4_Aim_*` | 4,242,642 |

**Changed:**

| Asset | Bytes | What changed |
|---|---:|---|
| `ABP_SkaterTB1` | 649,067 | bilinear aim (4 players, 3 two-way blends, variables T1..T3), stick correction node, variables `StickRot`, `StickOff` |
| `ABP_SkaterTB2` | 649,480 | same |
| `ABP_SkaterTB2_P3` | 645,166 | same |
| `ABP_SkaterTB2_P4` | 645,187 | same |
| `BP_SkaterVisual_TB1` | 1,132,056 | clamp -63..70; T1..T3 in `PushAnim`; event `SK3Fix`; variables `FixT StickCorr YawCorr RadCorr BladeH H0 FixGain` |
| `BP_SkaterVisual_TB2` | 1,106,668 | same |
| `BP_SkaterVisual_TB2_P3` | 1,106,753 | same |
| `BP_SkaterVisual_TB2_P4` | 1,106,935 | same |
| `BS_SkaterTB1v2_Aim` | 51,672 | 24 samples at the true positions |
| `BS_SkaterTB2_Aim` | 51,573 | same |
| `BS_SkaterTB2_P3_Aim` | 51,367 | same |
| `BS_SkaterTB2_P4_Aim` | 51,662 | same |
| `BS_SkaterTB2_Skate` / `_P3_Skate` / `_P4_Skate` | 28,145 / 27,873 / 28,115 | re-saved by the import |
| `M_SkaterTB2` | 66,746 | tier gates, `RimNight` 1.3, `BcastJerseyLum` 0.25 |
| `M_SkaterGen` | 48,191 | same |
| `M_SkaterTB1Jersey` | 54,536 | rim and emissive gates, `RimNight` 1.3 |
| `MI_SkaterTB1_Stick` | 9,367 | wood (0.30, 0.15, 0.05) |
| `MI_SkaterTB2_Stick` | 9,088 | wood (0.30, 0.15, 0.05) |
| `AS_SkaterTB1v2_*` (31) | 4,903,410 | re-imported in place from `skf/export3/p2_cel` |
| `AS_SkaterTB2_*` (31) | 6,132,157 | re-imported in place from `skh/export4/R` |
| `AS_SkaterTB2_P3_*` (31) | 6,300,677 | re-imported in place from `skh/export4/P3A` |
| `AS_SkaterTB2_P4_*` (31) | 6,268,077 | re-imported in place from `skh/export4/P4B` |

**Ledger, rows to update (notes only):**
- `AS_SkaterTB1v2_*` (31): "re-imported from skf/export3/p2_cel (lane SK3)".
- `AS_SkaterTB2*` (93): "re-imported from skh/export4 (lane SK3)".
- `ABP_*` and `BP_SkaterVisual_*`: "+ bilinear 24-hold aim and blade-on-ice correction (lane SK3)".
- `BS_*_Aim`: "24-hold grid at true sample positions (lane SK3)".
- `M_SkaterTB2`, `M_SkaterGen`, `M_SkaterTB1Jersey`: "+ tier gates from MPC_Look (RimNight, broadcast fill/rim off, BcastJerseyLum) (lane SK3)".
- `MI_SkaterTB1_Stick`, `MI_SkaterTB2_Stick`: "darker wood (lane SK3)".

**Ledger rows to add (96).** The full list is in `$SK3/rows.md`, one row per hold. Pattern, with the first and the per-set form:

```
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Aim_N63_R100.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export3/p2_cel loopcut, 24-hold aim grid, animation-only import (lane SK3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Aim_<hold>.uasset | anim | 0.17-0.18 | original | Blender 4.2 Python script (lanes SKF, SKH) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf p2_cel grid retargeted onto R, skh/export4/R loopcut, animation-only import (lane SK3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_Aim_<hold>.uasset | anim | 0.17-0.18 | original | ... | | | 2026-11-12 | B | n | skf p3_clay grid retargeted onto P3A, skh/export4/P3A loopcut, animation-only import (lane SK3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_Aim_<hold>.uasset | anim | 0.18 | original | ... | | | 2026-11-12 | B | n | skf p4_broadcast grid retargeted onto P4B, skh/export4/P4B loopcut, animation-only import (lane SK3) |
```

## Stills (`devlog/shots/`)

| File | What |
|---|---|
| `20261112-184540-B-sk3-aim-tb1` | aim sweep from above at 1.6 m, angles -63 → +70 |
| `...-184541-B-sk3-aim-tb2`, `-aim-p3`, `-aim-p4` | the same on the generated figures |
| `...-184541-B-sk3-tiers` | bot match from the game camera: night with T-B1, clay with P3, broadcast with P4 |
| `...-184542-B-sk3-rim-game`, `-rim-far`, `-rim-pool` | rim comparison 0.6 / 1.0 / 1.3 / 1.6 |
| `...-184542/3-B-sk3-back-tb1`, `-back-p4`, `-back-p3`, `-back-tb2` | back numbers: Glide, Stride, aim +70, aim -63 |

The aim stills were taken outside the room, so their background is black and the HUD shows. There is no puck comparison.

## Blender-side faults (take, frame, what is wrong)

1. **R: the holds themselves miss their point.** The engine measures up to 9.7 cm at grid points without correction; Blender says 6.1 cm at `Aim_N30_R130`. The correction absorbs it.
2. **Crossovers and leans: hips 4 to 10 cm low** on all figures, all frames. `Idle_Faceoff`, which locomotion plays at speed 0, is about 10 cm low as well.
   - The correction cures it, at the cost of up to 10 to 15 cm of stick slide and 7 to 9° of tilt.
   - The source fix: crossover, lean and idle pelvis height nearer Glide, as was done for Stride.
3. **P4B `Stride`:** the loopcut wrap is 8 cm. I imported the loopcut file; the main file's 0.85 cm take was not swapped in (time).
4. **P3A `Crossover_Right`:** a 21 cm wrap jump (SKH's note). Not checked in the engine.
5. **P4B two-handed right-side holds:** the bottom hand is 2 to 10 cm off the shaft (SKH's numbers). The engine wrist-to-shaft is consistent (mean 19 cm against the 9.3 cm mitten radius).

## What device A must know

- **Visual-side frame cost:** two extra per-frame socket reads per skater in Blueprint (`SK3Fix`) plus one Modify Bone node. Not measured (shared GPU).
- **Constant `PlayShoot(0)` with every pass:** unchanged, still handled.
- **The swap for tiers** is still in-place only. `SK3Fix` casts to its own Blueprint's AnimBP, so a figure swapped in place (B14's route) has no stick correction. With A's class-per-tier (B-to-A-0021) each class has its own.
- **`MPC_Look.Clay` = -1** now also drives the skaters' broadcast finish. No A change is needed; `MI_Bcast_SkaterP4` is no longer needed.

## What I could not verify

- **Item 6 (puck readability):** not started, for time. The halo still flattens at 24 m.
- **Generated figures in a bot match:** they cannot be selected yet, so no match numbers.
- **Moving states at the game's per-frame update rate** on the test rig. The rig steps the feedback about 3 times a second, so the Stride and crossover numbers are pessimistic. The match numbers (T-B1) are per frame.
- **P4 left hand through the whole of `Lift` / `Pass`:** only the first sampled frame (on the shaft). The montages finished before more polls.
- **Calls on P3 / P4 / R** were not re-driven. The take names and the graph are unchanged since SK2.
- **Frame cost, and the rim in motion.**

## What the lead must decide

1. **Keep the engine-side stick correction**, or ask Blender for crossover, lean and idle hips at Glide height and denser holds, and then lower `FixGain`. It is the only thing keeping the blade out of the ice in 93 % of match frames on T-B1 (5 % without it).
2. **The darker stick applies to all tiers.** A clay-only stick needs a per-tier switch in `M_Toon` (a `Look/` master; not mine to change).
3. **`RimNight` 1.3** is a small visible gain. If more separation in the dark band is wanted, try 1.6, or a lighting-channel rim (B13).
4. **Item 6 (puck kit):** schedule it for a later lane.
5. Paste the ledger rows and remove the 29 orphan rows.
