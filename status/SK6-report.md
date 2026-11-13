# SK6 report: the figure Blueprints on C-0001 v4, then the clay figures' surfaces (device B)

Lane SK6, 2026-11-13, 00:22 to 01:15 machine clock. Tree `C:/Users/kazda/jam/wt/b-a45` (for branch `dev-b-a45`), editor port 8000 (pid 37264, mine), left
through `ExitEditor` with no dirty package (dirty lists empty just before). No build, no git write. `/Game/Maps/Rink` never saved (`Rink.umap` still 00:13);
`_Scratch/` absent; `jam.Cut` back to `hockey.regular`, `jam.ScriptHuman` 0. Port 8002 was idle during my runs, but no frame numbers were taken.
Scratch `$SK6` = `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/sk6` (scripts, logs, DSL
backups `bk/`, the pre-lane `.uasset` copies in `bk/`, full-size images `img/`). Checkpoint: `status/SK6-checkpoint.md` (part 1 in full detail).

## Verdict

| Part / item | Result |
|---|---|
| 1.1 List and compile state | **PASS**: 7 Blueprints, all `BS_ERROR` with the 4 "more than one function with the same name" errors each |
| 1.2 Events onto the interface, `KnockdownFrames` timing | **PASS**: all 7 compile clean |
| 1.3 Match on the default figure (`all_star`) | **PASS**: JAMVIS and figure lines 1:1; 5 knockdowns, get-up ends **0 frames** from the release, first knockdown included |
| 1.3 Twitch / cooldown | twitched: **0.6 s cooldown** set (hit, board and stick hit) |
| 1.4 `generated_b2` night and clay | **PASS** (TB2, TB2_P3); `_TB1_P3` not selected by the game in this tree (test figures instead) |
| R-0032 stick blink (coordinator) | **FAIL before, PASS after** except the 30 Hz noise test (blade p95 2.45 cm, limit 2) |
| 2.5 Off-screen correction gate | **PASS** (all 7) |
| 2.6 P3C felt and stitches | **PARTIAL**: base colour v2 + mask (team colour, felt, roughness per part) in; **normal map imported but left off** (it breaks the shading) |
| 2.7 TB1_P3 export6 mesh | **PASS**: mesh re-imported, vertex colours present, jersey master reads them; clay parts (M_Toon) do not |
| Validation | **PASS**: 568 loaded, 568 valid, 0 invalid, 0 warnings, 14 blend spaces full, 14 Blueprints/AnimBPs `BS_UP_TO_DATE` |

## Part 1 (summary; numbers and tables in the checkpoint)

**Each Blueprint implements now** (all seven: `TB1`, `TB1_P3`, `TB2`, `TB2_P3`, `TB2_P4`, `TB2_old`, `TB2_P3_old`):
the four v4 calls as interface events with the old bodies, `SetKnockedDown` with `KnockdownFrames`, `SetTripped` as a Blueprint event, the 0.6 s reaction
cooldown, aim smoothing and damped correction (R-0032), the off-screen gate. `_TB2_P4` and the `_old` copies use their older takes (P3_old's board hit is
`Check_Hit_Back`, face-down falls are `Fall_Forward`) and no knockdown-strength binding. All seven were driven as test figures in PIE: each played the matching
take for hit, board, pin on/off, stick hit, knockdown (90 frames) and get-up, and the 0-frame fallback gets up on the release call.

**Get-up timing:** fall F and get-up G read from the takes, D = KnockdownFrames / 60; scale s = max(1, (F + G) / D); get-up starts at D - G/s at rate s (late
ticks caught up through the montage start). Measured, 5 + 5 + 5 knockdowns on three tiers: get-up end minus release **0.0 frames** (the estimate from the
montage position), -0.08 frames under slomo. The learned gap and its divide guard are deleted.

**Reaction counts** (126 s, seed 1, `all_star`, scripted human; identical sim on night TB1, night TB2 and clay TB2_P3):
JAMVIS hit 106 / board 54 / pin on 42, off 42 / stickhit 81; played after the cooldown 72 / 40 / 42 on / 32; skipped 34 / 14 / - / 49.
Before the cooldown 114 of 229 reactions came within 0.6 s of the previous one on the same figure; stick hits came in 2-frame bursts.

**Strips** (`devlog/shots/20261113-010410-B-sk6-hit-strip.jpg`, `-010411-B-sk6-board-strip.jpg`, `-sk6-stickhit-strip.jpg`, `-sk6-knockdown-strip.jpg`,
`-sk6-tiers-figures.jpg`): knockdown and get-up read; board hit reads; hit reaction reads at 9 m; `Lift_Hit` is subtle.

**R-0032 stick blink** (default figure; before -> after): 60-degree step: correction peaked at its clamps (turn 12 deg, slide 15.6 cm), settled in 7 to 12
frames with 1 to 2 sign flips -> peaks 4.1 deg / 1.5 cm, no flip on tilt and slide (turn crosses its final value once by 0.2 deg while following the smoothed
aim). Noise +-8 deg at 30 Hz: blade 6.1 / 37.0 / 57.7 cm per frame (p50 / p95 / max) -> 1.07 / 2.45 / 6.97 cm. Human in the match, on the ice: p95 3.6 ->
0.5 cm. No two-solution flips or re-grips seen. Change: critically damped aim follow (0.06 s), correction step 0.2 of the error (was 0.6) with a 0.3 cm dead zone.
The cause of the ringing: the correction reads the blade one frame late, and a one-frame-lag loop rings at a gain above 0.25.

## Part 2

### 5. Off-screen correction: PASS
`SK3Fix` in all seven: `FixW` = 0 while `WasRecentlyRendered(0.2)` is false, and the three corrections are reset to 0; on return `FixW` ramps to 1 over 0.15 s
and multiplies the applied correction. Proof (TB1 test figure, fixed camera, moved 40 m behind the camera for 1.5 s): off screen `StickCorr / YawCorr / RadCorr`
= 0 / 0 / 0 (SKI3 saw them run into 12 deg / 12 deg / 15 cm), back in view `FixW` 0.11, 0.22 ... 1.0 in 9 frames, corrections -0.04 -> -0.21 deg, no jump
(`$SK6/logs/off_final.json`).

### 6. P3C felt and stitches: PARTIAL
- Imported at 1024 from `skt/out/*_1024.png`: `T_SkaterGen_P3C_BaseColor_v2` (BC1 sRGB), `T_SkaterGen_P3C_Normal` (TC_Normalmap, linear, no green flip),
  `T_SkaterGen_P3C_Mask` (TC_Default with alpha = BC3, linear).
- `M_SkaterTB2`, group "SKT maps (SK6)", all behind **`SKTMaps` (default 0 = every code path returns today's value, by construction; not pixel-diffed)**:
  `MaskTex`, `NormalTex` (default FlatNormal), `RoughFelt` 0.95, `RoughStitch` 0.85, `RoughClay` 0.68, `NormalStrength`. With `SKTMaps` 1: team colour gated by
  mask R x (1 - G) instead of the hue key; felt by mask A x saturate(R + G) instead of the hue key and height cut (A alone also covers the navy pants, which the
  felt sheen turned grey); roughness per part; the normal map blended (whiteout, tangent space) over the clay normal.
- `MI_SkaterGen_P3C`: base colour v2, mask, `SKTMaps` 1, `JerseyLum` 0.19, `FeltRimBoost` 2 (was 4), `RoughClay` 0.70, `FeltBump` 2 (SKI2's fibre bump kept),
  **`NormalTex` = FlatNormal**.
- **The normal map is not used**: at any strength (0.3 and 1.0 tried, with and without reconstructing z) it turns the jersey pastel, the pants grey in patches
  (one leg grey, one dark) and the eyes pale under the clay rig (`devlog/shots/20261113-011436-B-sk6-p3c-normal-problem.jpg`). The map itself is mild
  (mean blue 254.9 / 255). Not found in the time: a tangent-basis mismatch between the lab's MikkTSpace assumption and the imported mesh, or my blend.
- Stills (`-011436-B-sk6-p3c-skt-maps.jpg`, 8 m / game camera / close, blue and orange, old left, new right): cream sleeve and sock bands and the stitch rows
  now read at 8 m and at the game camera; the jersey is a little lighter and less saturated than before; numbers read. The tier's depth of field blurs the close-up.

### 7. TB1_P3 export6 mesh: PASS (jersey and trim), not done on the clay parts
- `SK_SkaterTB1_P3` re-imported from `skf/export6/p3_clay/SK_Skater_p3_clay_loopcut.fbx`, mesh only (Interchange; skeleton `SKEL_SkaterTB1_P3` kept, no takes
  touched, no new asset created): 8,651 -> 8,697 vertices, 9 slots and their materials unchanged, the three sockets kept, **`has_vertex_colors()` true**.
  The separate stick mesh (`SM_SkaterTB1_P3_Stick`) was not re-imported (the blade tape is in the skeletal mesh's slots).
- `M_SkaterTB1Jersey`, group "Vertex colour (SK6)", behind **`VColMix` (default 0)**: base colour x (1 + (G - 0.5) x 2 x `VColJitter` 0.04) x (0.6 + 0.4 x B);
  roughness toward `VColFeltRough` 0.97 by R; the felt rim x R. Set to 1 on `MI_SkaterTB1_P3_JerseyDeco` and `_TrimDeco`.
- **The clay parts (Helmet, Gloves, Pads, Skate) are `M_Toon` instances** (`Look/` master, not mine): they do not read the vertex colours.
- Stills (`-011436-B-sk6-tb1p3-export6.jpg`): bands, stitches and the darker folds read at 8 m and the game camera.

## Assets created or changed (bytes)

| File (`Game/Plugins/JamAssets/Content/Skaters/`) | Before | After |
|---|---:|---:|
| `BP_SkaterVisual_TB1.uasset` | 1,940,533 | 2,804,420 |
| `BP_SkaterVisual_TB1_P3.uasset` | 1,994,837 | 2,859,016 |
| `BP_SkaterVisual_TB2.uasset` | 1,915,955 | 2,777,253 |
| `BP_SkaterVisual_TB2_P3.uasset` | 1,915,742 | 2,777,372 |
| `BP_SkaterVisual_TB2_P4.uasset` | 1,688,460 | 2,551,602 |
| `BP_SkaterVisual_TB2_old.uasset` | 1,688,227 | 2,551,478 |
| `BP_SkaterVisual_TB2_P3_old.uasset` | 1,687,984 | 2,551,279 |
| `M_SkaterTB2.uasset` | 74,020 | 81,140 |
| `MI_SkaterGen_P3C.uasset` | 19,732 | 22,997 |
| `T_SkaterGen_P3C_BaseColor_v2.uasset` (new) | | 1,161,607 |
| `T_SkaterGen_P3C_Normal.uasset` (new) | | 1,028,141 |
| `T_SkaterGen_P3C_Mask.uasset` (new) | | 89,346 |
| `SK_SkaterTB1_P3.uasset` | 1,928,839 | 1,990,157 |
| `M_SkaterTB1Jersey.uasset` | 55,212 | 61,287 |
| `MI_SkaterTB1_P3_JerseyDeco.uasset` | 12,074 | 12,511 |
| `MI_SkaterTB1_P3_TrimDeco.uasset` | 12,684 | 13,121 |
Nothing else under `Game/` changed (`find -newer`).

Ledger rows to add (format of `ASSETS.md`):
```
| Game/Plugins/JamAssets/Content/Skaters/T_SkaterGen_P3C_BaseColor_v2.uasset | texture | 1.11 | original | Python (numpy, Blender 4.2) lane SKT + UE 5.9 via MCP/Python | | | 2026-11-13 | B | n | skt/out base colour v2 at 1024, BC1 sRGB: felt mottling, cream bands and running stitches (lane SK6) |
| Game/Plugins/JamAssets/Content/Skaters/T_SkaterGen_P3C_Normal.uasset | texture | 0.98 | original | Python (numpy, Blender 4.2) lane SKT + UE 5.9 via MCP/Python | | | 2026-11-13 | B | n | skt/out tangent normal at 1024, TC_Normalmap, DirectX green; imported, not used by MI_SkaterGen_P3C (lane SK6) |
| Game/Plugins/JamAssets/Content/Skaters/T_SkaterGen_P3C_Mask.uasset | texture | 0.09 | original | Python (numpy, Blender 4.2) lane SKT + UE 5.9 via MCP/Python | | | 2026-11-13 | B | n | skt/out part mask at 1024, linear BC3: R team, G cream, B clay, A felt (lane SK6) |
```
Ledger rows to update (notes and sizes):
- `BP_SkaterVisual_TB1` 2.67, `_TB1_P3` 2.73, `_TB2` 2.65, `_TB2_P3` 2.65, `_TB2_P4` 2.43, `_TB2_old` 2.43, `_TB2_P3_old` 2.43 MB: "+ C-0001 v4: reactions on the
  interface, get-up timed by KnockdownFrames, 0.6 s reaction cooldown, aim follow 0.06 s, damped blade correction (0.2, dead zone 0.3 cm), off-screen gate (lane SK6)".
- `M_SkaterTB2` 0.08 MB: "+ SKT maps behind SKTMaps (default 0): mask-gated team colour, felt by mask, roughness per part, normal blend (lane SK6)".
  `MI_SkaterGen_P3C` 0.02 MB: "base colour v2 + mask, SKTMaps 1, JerseyLum 0.19, FeltRimBoost 2, normal off (lane SK6)".
- `SK_SkaterTB1_P3` 1.90 MB: "re-imported mesh only from skf/export6/p3_clay (bands, stitches, tape, vertex colours Col) (lane SK6)".
  `M_SkaterTB1Jersey` 0.06 MB: "+ vertex colour path behind VColMix (default 0) (lane SK6)". `MI_SkaterTB1_P3_JerseyDeco`, `_TrimDeco`: "VColMix 1 (lane SK6)".

## For device A, the landing window

Take from `dev-b-a45`, exactly: the seven `Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_*.uasset` above (they need v4's `JamSkaterVisual.h`; on v3
they fail to compile). The nine part-2 files (`M_SkaterTB2`, `MI_SkaterGen_P3C`, the three `T_SkaterGen_P3C_*`, `SK_SkaterTB1_P3`, `M_SkaterTB1Jersey`,
`MI_SkaterTB1_P3_JerseyDeco`, `_TrimDeco`) do not depend on v4; take them from `dev-b-a45` too (`dev-b` does not have them). Everything else of B's from `dev-b`.
Notes for A: hit reactions also fire for teammates bumping each other (the sim's bump rule does not look at the team); stick hits come in 2-frame bursts when the
puck changes hands repeatedly; the figures now ignore reactions within 0.6 s of the last one, so the sim may keep sending them.

## What I could not verify
- The normal map's failure cause (tangent basis or my blend).
- That `M_SkaterTB2` and `M_SkaterTB1Jersey` render pixel-identically for the other figures with the switches at 0 (true by construction of the code; not diffed).
- `_TB1_P3` in a real match (the game does not select it); the reactions on the night and clay matches came from the same seed as on TB1, so they are the same events.
- The get-up end is measured from the montage position, not from rendered frames; frame cost (none taken).
- The noisy-aim criterion: blade p95 2.45 cm a frame against 2 cm; a longer aim time constant would meet it at the cost of lag.

## What the lead must decide
1. Keep the shared 0.6 s cooldown on board and stick hits too (the brief named only hits), or ask A to rate-limit stick hits in the sim.
2. The R-0032 residual: accept 0.06 s and 2.45 cm p95 under +-8 deg / 30 Hz noise, or raise the aim time constant (say 0.09 s).
3. P3C's normal map: a lab or engine follow-up (tangent check), or drop it.
4. Vertex colours on TB1_P3's clay parts need a `VColMix`-style switch in `Look/M_Toon` (not B-skater's master): ask the look lane.
5. Remove the `SK6RX` / `SK6KD` / `SK6REL` / `SK6UPEND` / `SK4*` log lines before shipping if a quiet log is wanted.
6. Paste the ledger rows.
