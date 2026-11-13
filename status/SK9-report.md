# SK9 report: the P4 lab Blueprint on C-0001 v4, the pass-10 faceoff takes, the combined clay frames (device B)

Lane SK9 (continuing SK8), 2026-11-13, 05:05 to 05:40 machine clock. Tree `C:/Users/kazda/jam/wt/b-a45` (branch `dev-b-a45`, on bfc55af). Editor port 8000, mine:
pid 39016 (05:05 to 05:28) and pid 33016 (05:33 to 05:34, a cold start to check loading); both left through `ExitEditor` (`LogExit: Exiting.`, process gone, dirty lists
empty just before). One start of mine in between (pid 41428) hung on the "Waiting for ZenServer" box and was stopped with `editor.ps1 -Stop` (it had loaded nothing).
No build, no cook, no git write. `/Game/Maps/Rink` never saved (`Rink.umap` still 00:13, 18,966 bytes); no `_Scratch/` or `_SK8TMP/` left; `jam.Cut` back to
`hockey.regular`, `jam.ScriptHuman` 0 (read back). Port 8002 never touched.
Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/sk9` (the Blueprint scripts, `bk/` the pre-lane
copies of `BP_SkaterVisual_P4Lab` and of the 34 takes) and `.../scratchpad/sk8` (the PIE scripts, `img/`, `logs/`).

## Verdict

| # | Task | Result |
|---|---|---|
| 1 | `BP_SkaterVisual_P4Lab` on C-0001 v4 | **PASS**: compiles clean, also after a cold start; a test figure of it animates through every reaction and a timed knockdown |
| 2 | Pass-10 takes (3 night, 31 clay), by name | **Re-import PASS** (all 144 takes equal `export10` by take name, 0.0). **Faceoff PASS**: knees 70 -> 44 / 46 cm apart, reads as a crouch. **Clay aim: lean fixed, hands FAIL**: 3 of the 6 exact holds I measure and 9 of 15 cell centres are now one-handed (before: 0 and 6) |
| 3 | Combined clay frames, both figures, composite | **PASS** |
| 4 | All of `/JamAssets/Skaters/` loads and compiles | **PASS**: 731 assets, 16 Blueprints and AnimBPs `BS_UP_TO_DATE`, 16 blend spaces full |

## 1. `BP_SkaterVisual_P4Lab`

- **Before:** did not compile (the four "more than one function with the same name" errors).
- **What I did:** ran lane SK6's own scripts on it, unchanged (copies in the scratch): the four reactions as interface events with their old bodies, `SetKnockedDown` with `KnockdownFrames` and the get-up scheduled from it (the learned down time is no longer used), `SetTripped`, the shared 0.6 s reaction rule, the aim follow (0.06 s), the damped blade correction (step 0.2, dead zone 0.3 cm), the off-screen gate.
- **Result:** `BS_UP_TO_DATE`. Its 39 event blocks are, block for block, the shipped `BP_SkaterVisual_TB2` with the asset names swapped. Only two blocks differ, and they differed before: the order inside `ReceiveBeginPlay` and its own `SetBoost` values (1.0 / 0.5 against 1.75 / 1.25).
- **Driven in PIE** (a summoned test figure beside a `BP_SkaterVisual_TB2_P4`; the game cannot select it): it strides (joints travel up to 6.9 cm in a 0.65 s sample), and the log shows `Check_Hit_Back`, `Check_Hit_Left`, `Board_Hit`, `Board_Pin` on and off, `Lift_Hit` played, a second hit inside 0.6 s skipped. Knockdown with 90 frames: fall 0.5 s, get-up 0.9 s, get-up started on frame 37 and ended on frame 90.0. No runtime error line. Still: `sk9-p4lab-driven`.
- **For the landing:** this file must come from `dev-b-a45`. On `dev-b` it stays the v3 version, which does not compile against v4.
- **Seen, not looked into:** in my still the P4 lab figure shows no number on its back after `SetNumber(13)`; the `TB2_P4` figure beside it shows 13.
- 1,925,623 -> 2,786,178 bytes.

## 2. The pass-10 takes

- **`READY.txt` arrived at 05:10.** It names 3 night takes (`Faceoff_Draw`, `Faceoff_Ready`, `Idle_Faceoff`) and 31 clay takes (the same three, the 24 aim holds, the 4 aliases `Aim_Front`, `Aim_Left`, `Aim_Right`, `Aim_FarReach`).
- **How:** those 34 assets, and only those, re-imported in place through Interchange from `skf/export10/<style>/SK_Skater_<style>_loopcut.fbx`. None of them has a stale index. The two that do (`AS_SkaterTB1v2_Shot_Charge`, `_Stop`) are not in the list and were not touched.
- **Proof by name:** export10 imported once more as scratch assets and compared take by take with the project's assets, all 24 bones on every frame: night 72 of 72 equal, clay 72 of 72 equal, largest difference 0.0. Scratch deleted. Take settings (rate, loop, key count, length, compression, notifies, curves, tracks): no difference on any of the 144. `git status`: exactly these 34 take files changed.

**The faceoff stance** (a test figure held in the faceoff stance; export9 -> export10):

| | Night `TB1` | Clay `TB1_P3` |
|---|---|---|
| Thigh below horizontal, left / right (deg) | 29.3 / 28.5 -> **41.2 / 44.0** | 29.0 / 28.3 -> **41.3 / 44.0** |
| Knee spread (cm; hip joints 23 and 26 apart) | 70.0 -> **44.4** | 69.4 -> **46.3** |
| Knee outside its hip joint, each side (cm) | 23 -> 11 | 22 -> 10 |
| Skates apart (cm) | 56.3 -> 52.3 | 58.6 -> 54.9 |
| Pelvis / head height (cm) | 67.0 / 94.4 -> 74.0 / 105.6 | 63.7 / 89.9 -> 70.1 / 100.3 |
| Trunk from vertical (deg) | 55.2 -> 50.4 | 55.4 -> 50.5 |

From the game camera it now reads as a crouch over the dot, knees under the body (`sk9-faceoff-before-after`). It is still a deep crouch.

**Match log** (night, default figure, seed 1, `all_star`, 5400 sim frames): identical to both earlier runs. `JAMVIS` hit 76 = 52 played + 24 skipped; stick hit 74 = 27 + 47; board 41 = 30 + 11; pin on 41 = 41; pin off 41 = 37 + 4 ignored; no `JAMVIS` without a figure line; 5 knockdowns, get-up end minus release 0 frames each.

**Aim cells** (same rig as SK8; export9 -> export10):

| | Night `TB1` | Clay `TB1_P3` |
|---|---|---|
| Blade error, worst glide cell / stride cell / hold (cm) | 0.39 / 0.50 / 0.36 -> 0.39 / 0.56 / 0.36 | 0.38 / 0.50 / 0.42 -> 0.38 / 0.55 / 0.39 |
| Largest increase on any cell | +0.12 or less | +0.12 |
| Trunk from vertical, glide with aim (deg) | 27.2 -> 27.2 | 39.6 -> **28.4** |
| Head height, glide with aim (cm) | 121.9 -> 121.8 | 109.4 -> 115.3 |
| Bottom hand to the shaft, median over the glide cells (cm) | 4.4 -> 4.4 | 6.7 -> **18.4** |
| Cell centres with the hand more than 10 cm off (of 15) | 2 -> 2 | 6 -> **9** |
| Exact holds with the hand off (of the 6 measured) | 0 -> 0 | 0 -> **3** |

- **Clay hands, cell by cell (cm, export9 -> export10):** at -54.5 deg: 19 / 53 / 70 -> 58 / 62 / 67 (1.15 / 1.375 / 1.525 m); at -38 deg: under 7 / 21 / 53 -> 31 / 48 / 62; at -15 deg: 1.375 m under 7 -> 18, 1.525 m 20 -> 45; at +17.5 deg, 1.525 m: under 7 -> 16. Exact holds: -63 deg / 1.0 m 7.0 -> 55; -30 deg / 1.45 m under 7 -> 59; **0 deg / 1.6 m (straight ahead, far) 7.0 -> 51**.
- **What it looks like** (`sk9-clay-aim-hands`): the stick in the top hand only, the free glove beside the hip. SKF's note says 11 of the 24 clay holds are one-handed in export10 and names three as newly forced; against the export9 state adopted in SK8, the engine shows more than those three.
- **This breaks the first pass's rule** (hand no worse than before by more than 1 cm) on the clay figure. It is the trade SKF made for the lean (28.4 deg now, the night figure 27.2). I kept export10 as delivered, because the brief asked for these takes and the clay scripted figure is not selectable by the game. To go back on the aim only: restore the 28 files `AS_SkaterTB1_P3_Aim_*.uasset` from commit bfc55af (the faceoff takes are separate files); the lean then returns to 39.6 deg.
- Night figure: its aim holds are unchanged, its numbers are unchanged.

## 3. The combined clay frames

Real clay match, seed 1, frozen at the faceoff (game time 1.2 s) and in open skating (2.18 s, about 8 m/s), HUD removed, contact plane shown. `-p3c` is the game's own clay figure (`generated_b2`); `-tb1p3` is the scripted figure attached to each pawn and fed the game figure's motion, aim and faceoff state (the game's figure hidden). No reference pixels.

- **Composite: `sk9-clay-figures-compare`**: generated left, scripted right; rows low camera / game camera at the faceoff / game camera skating.
- Said plainly: both figures still stand on an amber stain (the contact plane is faint from the game camera). The generated figure is a bobble-head with a face; the scripted one is stockier with the felt number patch. In these frames the scripted figure's faceoff crouch and standing stance are the pass-10 ones.

## 4. Before exit

731 assets under `/JamAssets/Skaters/` load, 731 valid, 0 invalid, 0 warnings, no missing dependency. 16 blend spaces, none with an empty sample. All 16 Blueprints and AnimBPs compile to `BS_UP_TO_DATE`:
`ABP_SkaterGen_P2C`, `ABP_SkaterGen_P3C`, `ABP_SkaterGen_P4E`, `ABP_SkaterTB1`, `ABP_SkaterTB1_P3`, `ABP_SkaterTB2`, `ABP_SkaterTB2_P3`, `ABP_SkaterTB2_P4`, `BP_SkaterVisual_P4Lab`, `BP_SkaterVisual_TB1`, `BP_SkaterVisual_TB1_P3`, `BP_SkaterVisual_TB2`, `BP_SkaterVisual_TB2_P3`, `BP_SkaterVisual_TB2_P3_old`, `BP_SkaterVisual_TB2_P4`, `BP_SkaterVisual_TB2_old`. None fails.
After a cold editor start: the same 731 load, the same 16 are `BS_UP_TO_DATE` as loaded, and the log has no compile error line.

## What still looks wrong, said plainly

- The clay scripted figure aims one-handed over much of its right side and straight ahead at full reach.
- The faceoff is a deep crouch (thighs 41 to 44 deg below horizontal), knees still about 10 cm outside each hip.
- The two P4 figures (`BP_SkaterVisual_TB2_P4`, `BP_SkaterVisual_P4Lab`) keep the old faceoff squat; SKF did not build that style.
- The amber stain under the clay figures.

## Files changed (all under `Game/Plugins/JamAssets/Content/Skaters/`)

| What | Before (bytes) | After (bytes) |
|---|---:|---:|
| `BP_SkaterVisual_P4Lab.uasset` | 1,925,623 | 2,786,178 |
| 3 x `AS_SkaterTB1v2_{Faceoff_Draw, Faceoff_Ready, Idle_Faceoff}.uasset` (total) | 513,954 | 512,640 |
| 31 x `AS_SkaterTB1_P3_{Faceoff_Draw, Faceoff_Ready, Idle_Faceoff, Aim_*}.uasset` (total) | 4,114,809 | 4,116,582 |

Nothing else under `Game/` changed. New outside `Game/`: `status/SK9-checkpoint.md`, this report, 24 files in `devlog/shots/`.

## Ledger rows for `ASSETS.md` (replace the existing rows)

```
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_P4Lab.uasset | other | 2.66 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | copy of BP_SkaterVisual_TB2 with SKI2's gen.mjs graph on SK_SkaterGen_P4E / ABP_SkaterGen_P4E; not selected by the game yet (lane B21); on C-0001 v4: reactions on the interface, get-up timed by KnockdownFrames, 0.6 s reaction rule, aim follow 0.06 s, damped blade correction, off-screen gate (lane SK9, SK6's scripts) (2786178 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_FarReach.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127958 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_Front.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127606 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_Left.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127386 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N30_R100.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127513 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N30_R130.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127700 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N30_R145.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127881 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N30_R160.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127930 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N46_R100.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127567 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N46_R130.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127723 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N46_R145.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127815 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N46_R160.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127862 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N63_R100.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127630 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N63_R130.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127683 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N63_R145.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127696 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N63_R160.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127751 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P00_R100.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127712 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P00_R130.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127789 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P00_R145.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127887 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P00_R160.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (128019 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P35_R100.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127695 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P35_R130.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127716 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P35_R145.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127850 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P35_R160.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127870 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P70_R100.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127507 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P70_R130.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127522 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P70_R145.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127629 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P70_R160.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127657 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_Right.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 aim hold, less trunk lean), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (127585 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Faceoff_Draw.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 faceoff stance), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (150548 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Faceoff_Ready.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 faceoff stance), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (154638 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Idle_Faceoff.uasset | anim | 0.22 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p3_clay loopcut (pass 10 faceoff stance), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK9) (235257 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Faceoff_Draw.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p2_cel loopcut (pass 10 faceoff stance), animation-only re-import in place on SKEL_SkaterTB1v2, same asset (lanes SK8, SK9) (141284 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Faceoff_Ready.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p2_cel loopcut (pass 10 faceoff stance), animation-only re-import in place on SKEL_SkaterTB1v2, same asset (lanes SK8, SK9) (145361 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Idle_Faceoff.uasset | anim | 0.22 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export10/p2_cel loopcut (pass 10 faceoff stance), animation-only re-import in place on SKEL_SkaterTB1v2, same asset (lanes SK8, SK9) (225995 bytes) |
```

## Stills (`C:/Users/kazda/jam/wt/b-a45/devlog/shots/`, each with a one-line `.txt`)

For the operator:
- `20261113-052908-B-sk9-clay-figures-compare.png`
- `20261113-052908-B-sk9-faceoff-before-after.png`

The six named stills (JPEG, 1920 x 1085):
- `20261113-052904-B-sk9-clay-hero-low-p3c.jpg`, `20261113-052906-B-sk9-clay-hero-low-tb1p3.jpg`
- `20261113-052904-B-sk9-clay-hero-game-p3c.jpg`, `20261113-052906-B-sk9-clay-hero-game-tb1p3.jpg`
- `20261113-052905-B-sk9-clay-hero-skate-p3c.jpg`, `20261113-052907-B-sk9-clay-hero-skate-tb1p3.jpg`

Extra: `20261113-052905-B-sk9-clay-hero-low-skating-p3c.jpg`, `20261113-052907-B-sk9-clay-hero-low-skating-tb1p3.jpg` (the low camera at 2.18 s),
`20261113-052908-B-sk9-clay-aim-hands.png`, `20261113-052908-B-sk9-p4lab-driven.jpg`.

## For device A: what to take from `dev-b-a45` beyond the 17 files announced earlier

1. All 72 `AS_SkaterTB1v2_*` takes and all 72 `AS_SkaterTB1_P3_*` takes (34 of them are newer than commit bfc55af: take the branch's newest).
2. `M_SkaterContact.uasset` (new in SK8), with `BP_SkaterVisual_TB1_P3.uasset` and `BP_SkaterVisual_TB2_P3.uasset` again (both were in SK6's seven; they changed in SK8 and need the material).
3. `BP_SkaterVisual_P4Lab.uasset`: from `dev-b-a45`, not from `dev-b` (there it is the v3 version). Its mesh, AnimBP, takes and textures (`*SkaterGen_P4E*`) are B's art from `dev-b`, unchanged by me.

No interface, anim graph, mesh, skeleton, blend space or AnimBP changed in this pass. The only graph change is `BP_SkaterVisual_P4Lab`.

## What I could not verify

- **A cook.** Not run (the lead runs it). The cold start showing all 16 Blueprints up to date with no compile error is the nearest I have.
- `BP_SkaterVisual_P4Lab` in a match (the game cannot select it), and its blade error at the aim cells (not measured for this figure). Its knockdown was driven by my script, whose release call came late on purpose; the take ended on frame 90 as scheduled.
- Why the P4 lab figure shows no number.
- How often the clay figure is one-handed in real play (I measured the cells, not the share of match time).
- Motion by eye: everything is stills and per-frame numbers.
- The look on device A's GPU; frame time (none taken).
- The PIE window size: the saved editor setting was 1920 x 1080 when I started and still is (an untracked user setting under `Saved/`; setting it back does not persist from a script).

## What the lead must decide

1. The clay aim holds: keep export10 (upright, often one-handed) or restore the 28 `AS_SkaterTB1_P3_Aim_*` files from bfc55af (two-handed, bent 39.6 deg), or ask SKF for a middle setting.
2. Which figure the clay tier uses (`sk9-clay-figures-compare`); the scripted one needs device A's game mode to select it.
3. The P4 figures' faceoff squat: ask SKF, or leave.
4. Run the cook-only check; paste the 35 ledger rows.
