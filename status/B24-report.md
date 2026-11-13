# B24 report: posture-fixed P4E re-imported, the aim layer opened for the stride's lean, hero stills re-shot (device B)

Lane B24 (continuing B21), machine clock 03:26 to 04:00. Tree `wt/b-vfx` (dev-b ff42f7c), editor on **port 8000** (two starts: the second after deleting the old P4E set, for a clean import), left through `ExitEditor` with **no dirty package** (checked with PIE stopped). No git, no build. `/Game/Maps/Rink` never saved (18,966 bytes, 2026-11-12 12:40); `_Scratch/` does not exist; only lab assets touched. The B23 editor on 8002 shared the GPU: no frame numbers. Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b24` (`setlbb.py` = the aim-layer change, `meas.sh` + `lab.lean/blade/aimpt` = the measurements, `ab.py` = the A/B sheet).

## 1. Re-import (export3): PASS
Clean route: deleted `SK_SkaterGen_P4E`, its 75 takes, both blend spaces, `ABP_SkaterGen_P4E`, `BP_SkaterVisual_P4Lab` (skeleton and textures kept), restarted the editor, imported `export3/P4E/SK_Skater_P4E_loopcut.fbx` with the skeleton set to `SKEL_SkaterGen_P4E` (mesh + 75 takes, no physics asset), then SKI2's scripts: sockets, slots (`MI_P4_FigureE`, trim, `M_P4_Stick`), blend spaces (Skate 14 / Aim 24 samples, **0 empty**), AnimBP copy (no unmapped players), Blueprint duplicate + `gen.mjs` graph. AnimBP and `BP_SkaterVisual_P4Lab` compile `BS_UP_TO_DATE`; **all 726 assets under `/JamAssets/Skaters/` load**; save left no warnings. Textures kept from export2, as told.

## 2. The stride's lean through the aim layer: done, lean 37 -> 45 deg at full stride
Cause confirmed: `ABP_SkaterGen_P4E`'s `LayeredBoneBlend` took the aim hold **from `spine_01`, in mesh space**, so the trunk was the hold's whatever the stride did. Tried:

| Aim layer | Trunk lean (pelvis to neck_01 from vertical), rest / full stride (9 m/s) | Blade height at 3 aim cells, full stride | Verdict |
|---|---|---|---|
| spine_01, mesh space (as shipped) | 33 / **37 deg** | +5, +6, -2 cm | upright |
| spine_03, local space | 26 / 48 deg | **-31, -38, -11 cm** (blade through the ice) | rejected |
| **spine_03, mesh space (kept)** | 28 / **45 deg** | **-4, -2, -4 cm** | lean shows, blade stays on the ice |

The chest and arms still take the aim hold's orientation in component space; only spine_01-02 now come from locomotion, so the shoulders (and the stick) ride down and forward with the lean. Lean is stable over the stride cycle (45.1-45.4 deg in 8 phases).

**Blade against the aim point** (my own aim point: reach from the pawn origin along facing - aim angle; not SKI2's calibrated cells), horizontal error, cm:

| Aim cell (angle / reach) | Rest, before | Rest, after | Full stride, before | Full stride, after |
|---|---|---|---|---|
| 0 / 1.30 | 6 | 9 | 1 | 11 |
| -40 / 1.45 | 11 | 13 | 12 | 7 |
| 35 / 1.15 | 12 | 10 | 3 | 21 |

So the blade stays on the ice (height within -4..+1 cm at speed) but lands up to ~20 cm off its aim point at full stride on the forehand-wide cell. The puck is placed by the sim, so at speed the puck can sit up to a blade-width from the blade. Not fixed: the cure is a speed-scaled correction or re-baked holds with the lean.

## 3. What export3 claims, checked in the engine
- **Both hands on the stick:** **yes** in Glide and full Stride from the right side: top hand on the butt end (at the hip in Glide, behind the shoulder at full stride), bottom glove on the shaft; at 3 m the bottom glove reads as a **gripping glove**, not an open hand (`b24-hands`). From directly behind (concept framing) the top hand sits **behind and above the shoulder** at full stride and the stick above the bottom hand is hidden by the body: it reads as a dark lump on the upper back (visible in `b24-hero-concept-framing` and `b24-hero-best`). In `Stop_Hockey_Left` the top hand holds the butt end at chest height, the bottom hand is hidden behind the body from that camera.
- **Opponents' ready stance:** from the game camera in the match (Glide/Idle) the opponents stand in a forward-leaning ready stance (`b24-hero-game-camera`). The **wide squat remains only at the faceoff** (`Faceoff_Ready`, not changed), which is what the concept-framing frames show behind the hero.
- **Level head in `Stop_Hockey_Left`:** **yes**, the chin is level, eyes forward (`b24-hero-stop` vs B21's head-back in `b24-posture-ab`).
- **Face-down hem:** **fixed**: `Knockdown_Back` held face down shows no white triangle over the seat (`b24-facedown-hem`).

## 4. Stills (`C:/Users/kazda/jam/wt/b-vfx/devlog/shots/`)
- `20261113-034521-B-b24-hero-concept-framing.jpg`: same camera as `b21-hero-concept-framing`; lean 45 deg, push leg extended, powder off the left skate.
- `20261113-034653-B-b24-hero-stop.jpg`: same camera as `b21-hero-stop`; `Stop_Hockey_Left` 0.3 s, spray.
- `20261113-035136-B-b24-hero-game-camera.jpg`: game camera, running bot match, both teams leaning.
- `20261113-035137-B-b24-posture-ab.jpg`: B21 left, B24 right: concept framing, stop, game camera.
- `20261113-035618-B-b24-hero-best.jpg`: best frame tonight: low three-quarter (2 m behind-left, 0.75 m up), full stride with lean, blade by the puck, spray, banks and streaks.
- Checks: `20261113-034607-B-b24-hands.jpg`, `20261113-035215-B-b24-facedown-hem.jpg`.
- Housekeeping: one still was first filed under a `b21-` label by a stale helper (03:44); I deleted that file (mine, a minute old) and re-filed it as `b24-hero-concept-framing`. The B21 original `20261113-024341-B-b21-hero-concept-framing` is untouched.

## 5. Glints at the game camera: tried, not solved
The two wide bright bars beside the players at the game camera are mostly the **Lumen reflection of the side lamp banks** (two rows of heads), spread by the scratched ice, plus the glints' cores. In PIE I cut the side-bank emissive 400 -> 40 and the glints 4000 -> 800 cd, source 6 -> 1 cm: the bars got only slightly dimmer (the banks stay saturated through bloom at EV 4.2). Nothing kept: the bank instance was set back to 400 (saved, same value; rig not changed). Likely fixes for a next pass: one row instead of two on the side banks, banks tilted down or moved higher so the game camera's mirror path misses them, or lower specular at grazing-down angles only. Concept-framing streaks come from the far bank and are not affected by any of these.

## Still wrong, plainly
- The top hand at full stride from behind reads as a lump on the upper back; the stick's upper half is invisible from the concept framing.
- From the game camera height, the controlled player at mid speed reads as **sitting** (deep knee bend with the pelvis low, seen from above-behind) in some match frames (`gm1`-`gm3` in scratch); the hero match frame was chosen where it does not.
- Blade up to ~20 cm off its aim point at full stride (above).
- Opponents still in the faceoff squat at the faceoff.
- Cloth: procedural folds, no drape, faceted hem, soft lumps on the back in hard light.
- Side-bank smears at the game camera (item 5).

## Skater against `path-p4-t2`: about **70 %** (B21: 65-70 %)
Proportions, kit, gloves, visor, colours, the lean and the stop are now in the concept's class. **Posture is no longer the weakest part: the cloth is** (a jersey that hangs and folds is the biggest visible difference left at the concept framing), followed by the top hand/stick read from behind.

## Assets changed (bytes, `Game/Plugins/JamAssets/Content/`)
| File | Bytes | Change |
|---|---:|---|
| `Skaters/SK_SkaterGen_P4E` | 5,005,040 -> 5,003,644 | export3 (skirt back-panel weights) |
| `Skaters/AS_SkaterGen_P4E_*` (75) | 14,306,126 -> 14,387,685 total (137,351-356,398 each) | export3 takes (skf/export8 posture) |
| `Skaters/ABP_SkaterGen_P4E` | 644,922 -> 644,813 | aim layer from `spine_03`, mesh space |
| `Skaters/BS_SkaterGen_P4E_Skate` / `_Aim` | 27,620 -> 27,722 / 51,252 -> 51,331 | rebuilt on the new takes |
| `Skaters/BP_SkaterVisual_P4Lab` | 1,925,709 -> 1,925,623 | rebuilt (same graph) |
| `Skaters/SKEL_SkaterGen_P4E` | 14,022 | re-saved, unchanged |
| `Look/P4Lab/MI_P4_LampBank_Side` | 8,337 | re-saved, same values |

Ledger rows whose text must change (`shipped` n):
```
| Game/Plugins/JamAssets/Content/Skaters/SK_SkaterGen_P4E.uasset | mesh | 4.77 | ai-generated | Codex concept sheet + Hyper3D Rodin + Meshy rig; jersey garment by skc/p4/tools/build_jersey.py |  |  | 2026-11-13 | B | n | skc/p4/P4E/export3 loopcut FBX (export2 mesh, skirt back-panel weights to pelvis), loose jersey, 36.4k tris; Rodin a297c99b-3976-4baa-a8ee-17a77c59d0a2, Meshy rig 01a1d1f6-3b17-75d1-9dcd-afb5d0df719f; sockets stick/head/puck as SK_SkaterTB2 (lanes B21, B24) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterGen_P4E_<Take>.uasset | anim | 0.13-0.34 | original | Blender 4.2 script, retargeted |  |  | 2026-11-13 | B | n | skf/export8 take (posture fix) retargeted onto P4E (skc/p4/P4E/export3 loopcut), 75 takes (lanes B21, B24) |
| Game/Plugins/JamAssets/Content/Skaters/ABP_SkaterGen_P4E.uasset | other | 0.61 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | copy of ABP_SkaterTB2 on SKEL_SkaterGen_P4E; aim layer from spine_03 in mesh space so the stride's lean shows (lanes B21, B24) |
```

## What I could not verify
- Frame cost (shared GPU). Motion: lean and blade at speed in a played match beyond still frames.
- Blade error against SKI2's calibrated aim cells (mine use the pawn origin); the puck carry distance at full stride.
- `Board_Hit`'s 3 frames, `Stop_Hockey_Right` side hem (not stepped through).
- Glints/bank smears with a properly settled Lumen in every test frame (7-8 s waits).

## What the lead must decide
1. Keep the `spine_03` mesh-space aim layer (lean shows, blade up to ~20 cm off aim at full stride) or ask the figure lane to bake the lean into the aim holds instead.
2. Whether the next figure work is the cloth (now the weakest part) or the top-hand/stick read from behind.
3. Side-bank smears at the game camera: one row of side banks, or move them up (B, a short rig pass).
