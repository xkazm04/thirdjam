# SK10 report: the night's figure files on `dev-b` with the v3 Blueprints (device B)

Lane SK10 (the SK8/SK9 lane), 2026-11-13, 09:36 to 09:58 machine clock. Tree `C:/Users/kazda/jam/wt/b-vfx` (branch `lane/b-vfx`, on 43f8877, with the 155 staged files).
Editor port 8000, mine: pid 13428, 09:36 to 09:55, left through `ExitEditor` (`LogExit: Exiting.`, process gone, dirty lists empty just before). **Nothing saved**: `git status`
shows the same 156 entries as at the start, plus 18 new files in `devlog/shots/` and this report. No build was needed, no cook, no git write. `jam.Cut` back to `hockey.regular`,
`jam.ScriptHuman` 0. Another `UnrealEditor.exe` (pid 14144) appeared at about 09:55; it is not mine and I did not touch it.
Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/sk10` (scripts, `img/`, `logs/`).

## Verdict per check

| # | Check | Verdict |
|---|---|---|
| 1 | Everything loads and compiles | **pass** |
| 2 | Night tier, default figure | **pass** (knockdowns and reactions on test figures only: this tree's sim produced none in the match) |
| 3 | Clay tier, default figure (game-selected `BP_SkaterVisual_TB1_P3`) | **pass**; no slot wrong or default |
| 4 | Clay tier, `generated_b2`: the felt maps | **pass** |
| 5 | Night `BP_SkaterVisual_TB2` and lab `BP_SkaterVisual_TB2_P4` under the changed `M_SkaterTB2` | **partial**: by parameters both pass; night also by eye against SK6's still; **no earlier still of `TB2_P4` found to compare** |
| 6 | Night jersey under the changed `M_SkaterTB1Jersey` | **pass** by parameters and against the branch; not pixel-compared with a frame from before SK7 (none exists at the same frame) |

**Answer to the question:** yes. The 155 files behave with the v3 Blueprints as they do on the branch. The only differences I found live in the Blueprints: no contact plane, the get-up starts on the release call instead of ending on it, and the blade correction has no dead zone (so the blade error is smaller here).

## 1. Loading

- 731 assets under `/JamAssets/Skaters/` load; 731 valid, 0 invalid, 0 warnings; no missing dependency; 16 blend spaces, none with an empty sample.
- All 16 Blueprints and AnimBPs are `BS_UP_TO_DATE` as loaded, and again after compiling each in memory (not saved): `ABP_SkaterGen_P2C`, `ABP_SkaterGen_P3C`, `ABP_SkaterGen_P4E`, `ABP_SkaterTB1`, `ABP_SkaterTB1_P3`, `ABP_SkaterTB2`, `ABP_SkaterTB2_P3`, `ABP_SkaterTB2_P4`, `BP_SkaterVisual_P4Lab`, `BP_SkaterVisual_TB1`, `BP_SkaterVisual_TB1_P3`, `BP_SkaterVisual_TB2`, `BP_SkaterVisual_TB2_P3`, `BP_SkaterVisual_TB2_P3_old`, `BP_SkaterVisual_TB2_P4`, `BP_SkaterVisual_TB2_old`.
- The session log has no warning or error line naming a figure asset, a Blueprint compile, a missing reference or a material compile. No "Accessed None" or runtime error in any of the five matches.
- `git diff --stat origin/dev-b-a45 -- Game/Plugins/JamAssets/Content/Skaters/` lists only the eight `BP_*` files: the 155 files are the branch's, byte for byte.

## 2. Night tier, default figure (`BP_SkaterVisual_TB1`)

The game logs `JAMVISUAL humanoid_b1 tier=l2_night class=.../BP_SkaterVisual_TB1_C`.

| | This tree (v3) | Branch (SK9, v4) |
|---|---|---|
| Faceoff: knee spread (cm; hip joints 23 apart) | **44.4** | 44.4 |
| Faceoff: thigh below horizontal, left / right (deg) | 41.2 / 44.0 | 41.2 / 44.0 |
| Faceoff: pelvis height / trunk from vertical | 74.0 cm / 50.3 deg | 74.0 / 50.4 |
| Standing, no aim: knee spread / pelvis / trunk | 35.6 / 78.5 / 20.6 | 35.6 / 78.6 / 20.6 |
| Glide with aim: trunk / knees apart / head height | 27.2 deg / 35.6 / 121.8 cm | 27.2 / 35.7 / 121.8 |
| Stride (6 m/s) with aim: trunk / knees apart | 29.1 deg / 49.6 | 29.0 / 50.5 |
| Blade error, worst glide cell / stride cell / hold (cm) | **0.01 / 0.17 / 0.01** | 0.39 / 0.56 / 0.36 |
| Blade centre height, all cells (cm) | 4.49 to 4.52 | 4.13 to 4.80 |
| Bottom hand to the shaft, median (cm) | 4.4 | 4.4 |
| Cells with the hand more than 10 cm off (of 15) | 2 (44.1 and 21.2 cm) | 2 (44.1 and 21.2) |
| Correction in use, largest (tilt / turn / slide; clamps 12 / 12 / 15) | 3.1 deg / 3.5 deg / 4.5 cm | 2.9 / 2.9 / 4.4 |

- The blade error is smaller here because the v3 correction has no 0.3 cm dead zone and a larger step; that is in the Blueprint, not in the files.
- **The two repaired takes:** all 72 night takes in this tree, `Shot_Charge` and `Stop` among them, equal `export10` by take name on every frame of all 24 bones (largest difference 0.0). Played on test figures in slow motion: `Shot_Charge` is the stick-up wind-up, `Stop` is the braking lean; neither is the neighbour take. `SetShotCharge(1.0)` raises the stick (`sk10-shot-stop`).
- **Stop in play:** I played the take directly. I did not see the AnimBP trigger it in a match (not checked).
- **90 s match** (seed 1, `all_star`, scripted human, 5400 sim frames): no error line. This tree's sim sent 58 board, 42 poke, 28 lift, 8 pass, 2 shot and 1 goal events, and **no knockdown and no hit** (v3: no `JAMVIS` lines exist). The figure's poke line appears 42 times for 42 poke events.
- **Knockdown and get-up, on test figures:** `Knockdown_Left` then `GetUp_Side_Left`, `Knockdown_Back` then `GetUp_Front`, on both scripted figures. The v3 Blueprint starts the get-up when the release call arrives (`SK4UP ... gap 1.583`, my hold time). Ending the get-up on the sim's release frame is the v4 Blueprint's `KnockdownFrames` schedule and is not here, as expected.
- **Reactions, on test figures** (the v3 custom events called directly, recorded every frame): `Check_Hit_Back`, `Check_Hit_Left`, `Board_Hit`, `Board_Pin` on and off, `Lift_Hit` each played on both figures.
- **Against the branch, same frames:** night faceoff still: mean difference 0.75 of 255, 0.29 % of pixels differ by more than 24; skating still: 2.63 and 1.19 % (the frame is in motion).

## 3. Clay tier, default figure

- **The game selects it:** `JAMVISUAL humanoid_b1 tier=p3_clay class=.../BP_SkaterVisual_TB1_P3_C`, six figures, mesh `SK_SkaterTB1_P3`, `ABP_SkaterTB1_P3`. Real match, no call copying.
- **Materials, all nine slots, as read from a figure in the match:** Pads `MI_SkaterTB1_P3_PadsFelt`; Jersey a dynamic instance of `MI_SkaterTB1_P3_JerseyDeco`; Trim a dynamic instance of `MI_SkaterTB1_P3_TrimDeco`; Skin `MI_SkaterTB1_Skin`; Helmet a dynamic instance of `MI_SkaterTB1_P3_Helmet`; Gloves `MI_SkaterTB1_P3_Gloves`; Skate `MI_SkaterTB1_P3_Skate`; Blade `MI_SkaterTB1_Blade`; Stick `MI_SkaterTB1_Stick`. Each equals the mesh's own slot. **No slot is wrong or default.** The old Blueprint builds its dynamic instances from the mesh's slots, and SK7 put the felt pads on the mesh, so nothing was lost.
- **Seen in the stills:** felt jersey with the big cream stitches, dark felt pads, the cream number patch with the number, both team colours.
- **Faceoff stance:** knee spread **46.3 cm** (hips 26), thighs 41.3 / 44.0 deg below horizontal, pelvis 70.1 cm (SK9: 46.3, 70.1).
- **Aim, two-handed as intended:** hand to shaft median 6.7 cm; the six far-right cells one-handed (19 to 70 cm); the six exact holds all at 7.0 cm or less. Cell for cell within 0.1 cm of my SK8 numbers. Blade error 0.01 / 0.17 / 0.01 cm. Trunk with aim 39.6 deg (the bent export9 holds).
- **By take name:** the clay takes equal `export9` on 69 of 72; the three that differ are `Faceoff_Draw`, `Faceoff_Ready`, `Idle_Faceoff`, and those equal `export10`. That is the mix described in the brief.
- **Missing because the Blueprint is the old one:** the contact plane only (`ContactShadow` component with `M_SkaterContact` in `BP_SkaterVisual_TB1_P3` and `BP_SkaterVisual_TB2_P3`). `M_SkaterContact` loads but nothing uses it in this tree.

## 4. Clay tier, `generated_b2`

`JAMVISUAL generated_b2 tier=p3_clay class=.../BP_SkaterVisual_TB2_P3_C`. `MI_SkaterGen_P3C` carries `T_SkaterGen_P3C_BaseColor_v2` (1024, sRGB), `T_SkaterGen_P3C_Mask` (1024, linear), `T_SkaterGen_P3C_Normal` (1024, normal map), `SKTMaps` 1, `NormalStrength` 3.5, `JerseyLum` 0.30, `FeltMottle` 0.7. In the stills: cream stitch rows, bands, felt relief and mottle, the number.

## 5. Night `BP_SkaterVisual_TB2` and lab `BP_SkaterVisual_TB2_P4`

- Both are selected by the game (`generated_b2` on `l2_night` and on `p4_broadcast`) and render with three slots each, none default.
- **By parameters:** `MI_SkaterGen_P2C`, `MI_SkaterTB2_P4` and `MI_SkaterGen_P4D` hold every parameter SK6 and SK7 added at the master's default: `SKTMaps` 0, `FeltMottle` 0, `SheenWhite` 0.5, `ClayBumpScale` 1, `NormalStrength` 1, `NormalTex` the flat normal. With `SKTMaps` 0 the old code path runs.
- **Night `TB2` by eye** against SK6's still of 00:51 (before SK7 edited the master): the same bobble-head cel figure, outline, number and colours; blue jersey hue 218.9 here against 217.4 there (different frame and light).
- **Lab `TB2_P4`:** it renders cleanly on the broadcast tier (solid team colours, number on the back). **I found no earlier engine still of it to set beside this one**, so "as before" rests on the parameters only.
- Still: `sk10-night-b2` (SK6's still, this tree night, this tree broadcast).

## 6. Night jersey under `M_SkaterTB1Jersey`

- `MI_SkaterTB1_JerseyDeco` holds every added parameter at the master's default: `VColMix` 0, `FeltMix` 0, `FeltMottle` 0, `FeltDarken` 1, `ClayBumpScale` 1, `SheenWhite` 0.5. Both new paths are off.
- Blue jersey of the human, same frame: hue 218.6, value 0.64 here; 218.7 and 0.62 on the branch.
- There is no frame from before SK7 at the same game time, so this is not a pixel comparison with the old material. SK7's own check was the same kind (by construction and by eye).

## Stills (`C:/Users/kazda/jam/wt/b-vfx/devlog/shots/`, each with a one-line `.txt`)

- `20261113-095648-B-sk10-night-faceoff.png` (game camera and side)
- `20261113-095648-B-sk10-clay-default-game.jpg`
- `20261113-095649-B-sk10-clay-default-low.jpg`
- `20261113-095649-B-sk10-clay-b2-low.jpg`
- `20261113-095650-B-sk10-night-b2.png`
- `20261113-095650-B-sk10-v3-vs-branch.png`
- Extra: `20261113-095650-B-sk10-clay-default-skate.jpg`, `20261113-095651-B-sk10-clay-default-close.jpg`, `20261113-095652-B-sk10-shot-stop.jpg`

All from real matches, seed 1, frozen at game time 1.2 s (faceoff) or 2.18 s (skating), HUD removed. In `sk10-v3-vs-branch` the clay rows differ in the rink (pass 6 here, pass 5 on the branch) and in the contact plane; the figures match.

## What I could not verify

- **A cook.** Not run (yours).
- **Knockdowns and hit reactions in a real match in this tree:** the v3 sim sent none in 90 s. They were driven on test figures.
- **`Stop` triggered by the game's own logic** (I played the take directly).
- **`BP_SkaterVisual_TB2_P4` against an earlier still:** none found.
- **The night jersey and the generated figures pixel for pixel against the pre-SK7 materials:** no same-frame still exists.
- **`BP_SkaterVisual_P4Lab` and the two `_old` Blueprints in play:** they load and compile; I did not drive them here.
- **Motion by eye:** stills, slow-motion stills and per-frame numbers only. No frame numbers taken.
- The match ran slower than real time during my captures (90 s of sim took about 170 s); I did not look into why.
