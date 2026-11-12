# SK3 checkpoint (after item 2): the aim grid on the scripted figure (device B)

Lane SK3, 2026-11-12, started 17:26, checkpoint 18:12. Main tree, editor on port 8000, no git, no build. `/Game/Maps/Rink` is open in memory with test
actors `SK3_*` (never saved). A second editor may share the GPU: no frame numbers. Scratch: `.../scratchpad/sk3` (`$SK3`). Final report follows in
`status/SK3-report.md`. Everything measured in PIE, the real game (`jam.Cut` = a scratch copy of the SK2 cartridge, `visual: humanoid_b1`).

## Item 1, `BP_SkaterVisual_TB1`: PASS (with an engine-side stick correction)

- **Re-import:** 55 takes from `skf/export3/p2_cel/SK_Skater_p2_cel_loopcut.fbx`, animation-only onto `SKEL_SkaterTB1v2` (31 replaced in place, 24 new
  `AS_SkaterTB1v2_Aim_<N|P><angle>_R<reach>`). FK of the imported holds: every hold within 0.0 cm of its point.
- **Blend space** `BS_SkaterTB1v2_Aim`: 24 samples at the true positions (X angle -63/-46/-30/0/35/70, Y reach 1.00/1.30/1.45/1.60), no grid
  interpolation, no snap. UE triangulates the samples (Delaunay); every rectangular cell becomes two triangles, so a cell centre is a 50/50 blend of two
  diagonal holds, not the four. Measured at the 15 cell centres: 15.3 cm worst (tangential diagonal), 8.8 cm (radial). The blend space blends in
  local space per bone (mesh-space blending only exists with per-bone smoothing overrides).
- **So the AnimBP now does the bilinear blend itself:** four players of the same 2D blend space, each pinned to one reach row (Y = 1.0, 1.3, 1.45, 1.6;
  on a row the triangulation is linear between the two holds of that row), X = `AimAngle`, play rate 0 (frame 0 of each hold, per the SKH note), chained by
  three Two-way blends with alphas `T1 = clamp((r-1)/0.3)`, `T2 = clamp((r-1.3)/0.15)`, `T3 = clamp((r-1.45)/0.15)` written by `PushAnim`. Cells: 6.7 cm
  worst, blade down to -6.5 cm under the ice.
- **Blade-on-ice correction** (asked by the coordinator; also cures the hip-height dips): a `Transform (Modify) Bone` on `stick_socket` after the aim
  layer (additive component-space rotation and translation, `StickRot`, `StickOff`), driven from the Blueprint (`SK3Fix`, once per frame from `SK2Tick`):
  three integrators read the blade centre each frame and turn the stick about the grip (pitch: height to `H0`; yaw: sideways error) and slide it along
  the shaft (reach error); clamps ±12°, ±12°, ±15 cm; gain `FixGain` 0.6; scaled by `AimW`; decays to 0 while a montage plays or the aim layer is off.
- **Clamp:** `SetStickAim` angle -63..+70 (was -63.6..+66.3), reach 1.0..1.6.

Test figure (39 figures, one per point; blade bottom = blade-centre height - 2.9 cm, calibrated on the exact holds):

| State | grid: err max / bottom | cells: err max / bottom | correction used (pitch / yaw / slide) |
|---|---|---|---|
| Glide | 0.3 / 0.9..1.2 | 0.3 / 0.9..1.2 | 3.5° / 3.1° / 2.4 cm |
| Stride (4 phases) | 2.8 / 0.4..2.6 | 2.8 / 0.5..2.5 | 4.0° / 3.7° / 4.7 cm |
| Crossover L | 0.8 / -1.7..3.3 | 0.8 / -1.7..3.3 | 5.8° / 4.2° / 6.6 cm |
| Crossover R | 1.0 / -1.4..1.1 | 0.9 / -1.4..1.1 | 5.6° / 4.1° / 6.0 cm |
| Lean L | 0.0 / 1.6 | 0.0 / 1.6 | 5.8° / 4.2° / 6.7 cm |
| Idle (speed 0 plays Idle_Faceoff, hips 10 cm low) | 0.0 / 2.2..2.4 | 0.0 / 2.2..2.4 | 6.6° / 3.2° / 10.2 cm |

Without the correction (same rig): Glide cells 6.7 cm / -6.5 cm; crossovers cells 9.2 cm / blade to -14 cm; idle 11.9 cm / -20 cm.
Left wrist to shaft on two-handed holds: 4.3 to 7.0 cm (holding) in every state.

Bot match (240 samples of six visuals, aim layer on, no montage): blade bottom p5 -0.7, median 1.6, p95 3.1 cm, 93 % within -1..+4 (correction off:
median -5.2, p5 -14.7, 5 % within). Position error median 1.1 cm; the p90 of 10 cm is the one-frame lag of a turning skater (constant 10 cm at turn rate 5
and reach 1.2 m), plus carriers whose asked angle swings 30-130° between frames.

## Item 2: PASS

- Nine calls on the test figure: poke (clean; mistimed queues Stumble in 18), lift, pass (rate 0.8 / 1.3), foul (pri 5), charge (ChargeAlpha 1 at 0.6),
  shoot 0.6 -> `Shot_Slap`, shoot 0 -> `Shot_Wrist` one frame later, check, faceoff ready (bStFace), draw won 1.0 / lost 0.7, goalie hold / block L / R.
- Back number readable from game-camera height (6 m behind, 3.5 m up) in Glide, Stride, aim left 70/1.6 and aim right -63/1.6 (`$SK3/img/tb1_back_strip.png`).
- Carry in the match: puck to blade centre 3.0 to 6.5 cm over 17 carried polls through turns (yaw -165 -> -99 -> 0), puck to socket 0.0 to 2.0 cm.

## Changed so far (`Game/Plugins/JamAssets/Content/Skaters/`)

`ABP_SkaterTB1` 649,067; `BP_SkaterVisual_TB1` 1,132,056; `BS_SkaterTB1v2_Aim` 51,672; 24 new `AS_SkaterTB1v2_Aim_*` (3,426,215 total); 31
`AS_SkaterTB1v2_*` re-imported in place.
