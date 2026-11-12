# Skater lab S2 (motions): which clips are usable, without Unreal

Lane S2, device B, 2026-11-12 about 11:05-11:20 local. Rig: lane B5's `ue5_mannequin` rig, task `01a1cf22-143c-7425-8d02-8ce566754261` (28 bones, no root, pelvis on top). Lab folder `C:\Users\kazda\jam\skater-lab\s2\` (ledger `meshy-ledger.jsonl`, FBX in `dl/`, per-clip `inspect.json` and `seam.json`, frames in `frames/`, sheets in `sheets/`, scripts `seam.py`, `post.sh`, `gen.sh`). `B-work` folder `meshy-work/` was only read.

**Credits: S2 spent 138 of 150** (9 text-to-motion prime x 10 = 90; 9 animate x 3 = 27; one 7-action library animate = 21). Account balance read **1129 before** my first call (10:5x) and **868 after** my last (the difference, 261, includes the other three lanes' spending; my own ledger sums to 138). Nothing was dropped from the list of nine needs. One mistake of mine: I started the generation script twice by accident (a `nohup &` start looked dead and I restarted it), so the stride prompt was generated twice (13 credits). I killed the second script after its stride; no other prompt was run twice. The two strides are kept as a free sample of how much the same prompt varies.

## How measured

- `tools/meshy/inspect.py` (pelvis travel, frames) and my own `s2/seam.py` (Blender, first versus last frame). Seam = the largest local bone rotation difference in degrees between first and last frame (pelvis excluded), the mean over bones, and the largest pelvis-relative position difference of feet, hands and head in cm. "Best trim" = the frame in the last 40 % of the clip whose pose is closest to frame 0 (a cut point), with its max joint difference.
- Pelvis drift = horizontal pelvis travel first to last frame, plus vertical change (dz). Fps 30 everywhere.
- Renders: `render.py` motion mode, 8 frames, Workbench, 192 px wide, tiled 8x1 with ffmpeg (largest sheet 332 KB). "Readable" is judged from the 8 stills only; I did not watch the clips play.

## Rows

TTM = Meshy text-to-motion `prime`, applied to the B5 rig with `animate` (13 credits per clip: 10 + 3). Library = Meshy preset (3 credits per action). Provenance: TTM = `ai-generated` (Meshy, paid plan, task ids below); library = `sourced` (Meshy preset id, Meshy's licence).

### B5's existing clips, re-measured (credits were B5's, not S2's)

| Need | Source | Task id | Frames / s | Pelvis drift | Loop seam | Readable | Class |
|---|---|---|---|---|---|---|---|
| 1 stride | TTM; prompt not recorded in B5's ledger, `MESHY.md` section 6 gives the intended text | animate `01a1cf22-c87c-77df-b384-91ea6830dcf4` | 120 / 4.0 | **1.82 m** horizontal (0.26 / -1.80 m), dz 0.6 cm | **85 deg** (thigh_r), mean 27, feet/hands 61 cm; best trim frame 105: 51 deg | yes: crouch, alternating pushes, arms swing; sheet `lab-s2-stride-b5` | ai-generated |
| 4 slap shot | TTM, as above | `01a1cf22-f99e-70ed-a4b5-24c4d37b01f8` | 90 / 3.0 | 8.6 cm | one-shot, n/a (136 deg hand_l first to last) | partly: wind-up and swing show, no stick so the hands read vaguely; `lab-s2-shot-b5` | ai-generated |
| 9 idle (library 0 "Idle") | library 0 | `01a1cf23-2a80-71d4-a100-9897e4a1d58e` (merged) | 121 / 4.03 | 0 | **0 deg**, perfect loop | yes as a plain standing idle, not a faceoff pose; `lab-s2-idle-lib0` | sourced |
| 7 fall (library 187 "Knock_Down") | library 187 | same merged task | 76 / 2.53 | 47 cm back, 45 cm down | one-shot, n/a | **yes: hit, falls back, ends flat on the back and holds**; `lab-s2-knockdown-lib187` | sourced |
| 4 fallback (library 323 "golf_drive") | library 323 | same | 93 / 3.1 | 9 cm | one-shot | a two-hand swing; `lab-s2-golf-lib323` | sourced |

Note: `MESHY.md` (lane E1's stride) records 0.91 m forward and 0.38 m sideways; B5's own stride measures 1.82 m. Every generation differs.

### New text-to-motion clips (S2)

| Need | Prompt verbatim | Motion task / animate task | Frames / s | Pelvis drift | Loop seam | Readable | Credits |
|---|---|---|---|---|---|---|---|
| 1 stride, wording 2 (run A) | "ice skating forward at speed: low crouch, torso pitched forward, each leg pushes out diagonally to the side then returns under the body, weight shifting from side to side, arms swinging opposite to the legs, smooth continuous cycle that repeats seamlessly" (4 s) | `01a1cf31-d95a-711f-867b-f18624465326` / `01a1cf32-3531-735b-8178-f6211b25ddfe` | 120 / 4.0 | **3.76 m** (-2.95, -2.32), dz -3 cm | **74 deg** (upperarm_r), mean 20, 39 cm; best trim 98/120: 55 deg | yes: crouched, diagonal pushes; `lab-s2-stride2a` | 13 |
| 1 stride, same prompt (run B) | same | `01a1cf31-78f9-7509-8281-bbf499e0f6f1` / `01a1cf31-fcdc-7474-a265-14da88efeee4` | 120 / 4.0 | **6.04 m** (-3.13, -5.16), dz -10 cm | **53 deg** (hand_l), mean 21, 47 cm; best trim 106/120: **27 deg**, 25 cm | yes: deepest lean, arms swept back, speed-skater look; `lab-s2-stride2b` | 13 |
| 2 glide | "gliding on ice with both skates together: standing in a low crouch with knees bent, torso leaning slightly forward, arms relaxed out in front, nearly motionless, coasting, subtle breathing sway" (3 s) | `01a1cf32-2f66-7333-b8e1-4a5dc6bbe2f4` / `01a1cf32-b421-7773-aaab-b9d316818782` | 90 / 3.0 | 1.5 cm | **4.1 deg** (foot_l), 1.4 cm: loops as is | yes: low crouch, forward lean, feet together; nearly static; `lab-s2-glide` | 13 |
| 3 hard stop | "hockey player skating fast then braking hard: turns hips sideways, both skates skid sideways, deep knee bend, leans back and away from the direction of travel, arms out for balance, comes to a stop" (3 s) | `01a1cf32-e692-70de-9e20-cfd48b01bc13` / `01a1cf33-4203-7059-b4f8-678e13ea735c` | 90 / 3.0 | 2.8 cm, dz +10 | one-shot (93 deg first to last, it ends upright) | partly: crouch, arms out, hips turn, then straightens; no skid or lean-back reads clearly; `lab-s2-stop` | 13 |
| 5 wrist shot | "ice hockey wrist shot: standing in a crouch holding a stick with both hands low at the right side, quick sweep of the stick forward with a flick of the wrists, short follow-through pointing toward the target, fast" (2 s) | `01a1cf33-746a-767f-b878-bdeabf80f7de` / `01a1cf33-a609-77a3-ab2c-966de66ec269` | 60 / 2.0 | 3.5 cm | 5 deg, nearly closed | **no**: small hand movement, almost no sweep, no stick; `lab-s2-wrist` | 13 |
| 6 body check | "ice hockey body check: skater in a crouch drives the right shoulder forward into an opponent, lunging a big step ahead, torso rotating and elbows tucked, then recovers to a skating stance" (2.5 s) | `01a1cf33-dcff-73cb-a88c-fccc56c1d15f` / `01a1cf34-1031-743c-9b08-f134cc8d7508` | 75 / 2.5 | 44 cm forward | 24 deg, recovers near the start | weak: a lunge with forward arms, the shoulder lead is not clear; `lab-s2-check` | 13 |
| 7 knocked down + get up (CORRECTION, see the engine section: the clip starts already lying on the ice, there is no fall in it; use library 187 for the fall) | "a skater is knocked down on the ice: falls backward hard, lands lying flat on the back with arms and legs spread, lies still for a moment, then rolls to the side, pushes up onto hands and knees, and stands up" (6 s) | `01a1cf34-42dc-73c5-b6c6-f45b0c6c433e` / `01a1cf34-9ec2-74d1-bf45-66ac25bddbd9` | 180 / 6.0 | 9 cm; pelvis dz +41 (it ends standing) | one-shot | **yes**: lies flat on the back (about 1.5 s), rolls, crouches, stands. Ends in a crouch, not a skating stance; `lab-s2-knocked` | 13 |
| 8 celebration | "ice hockey player celebrating a goal: stands up straight, raises both arms high above the head with fists clenched, jumps slightly and shouts with joy, shaking fists in the air" (3 s) | `01a1cf34-d1ca-7152-aef9-d20ff77e677b` / `01a1cf35-2d3a-77c3-aee0-23702ee1b226` | 90 / 3.0 | 16 cm, 34 cm vertical (jump) | 21 deg, one-shot | yes: arms rise, a small jump; short; `lab-s2-celebrate` | 13 |
| 9 faceoff idle | "ice hockey faceoff stance: bent over deeply at the waist with knees bent, feet wide apart, holding a stick low with both hands on the ice in front, head up looking forward, waiting still with tiny movements" (3 s) | `01a1cf35-5f26-75cb-9275-1aa837765138` / `01a1cf35-e387-7269-b50c-24ef65dc4f55` | 90 / 3.0 | 0.5 cm | **1.7 deg**: loops as is | yes: bent over, knees bent, nearly still; hands are empty (no stick); `lab-s2-faceoff` | 13 |

### New library presets (S2), one merged animate `01a1cf36-15e4-7091-94a8-7dd4f4914e80`, 7 actions x 3 = 21 credits

Library search (free) found nothing for skate, glide, coast, brake, tackle, shoulder check or celebrate; 678 presets.

| Need | Preset id and name | Frames / s | Pelvis drift | Loop seam | Readable | Sheet |
|---|---|---|---|---|---|---|
| 3 stop | 531 Sprint and Sudden Stop | 61 / 2.03 | 2.40 m | one-shot (116 deg) | a running stop, upright, not skating | `lab-s2-lib-sprintstop` |
| 3 stop | 260 Push Forward and Stop | 78 / 2.6 | 45 cm | one-shot | a push and walk-stop; no | `lab-s2-lib-pushstop` |
| 6 check | 512 Male Head-down Charge | 16 / 0.53 | 1.70 m | 19 deg | a head-down run, 0.5 s, loops like a run; not a shoulder check | `lab-s2-lib-headdown` |
| 7 get up | 344 Stand Up 1 | 249 / 8.3 | 46 cm, dz +49 | one-shot | lying to standing, works but 8.3 s is too slow; the first 1.5 s are lying | `lab-s2-lib-standup1` |
| 7 fall | 366 Falling Down | 69 / 2.3 | 57 cm, dz -49 | one-shot (118 deg) | ends curled up sitting, not flat: worse than 187 | `lab-s2-lib-fallingdown` |
| 8 celebrate | 298 Cheer with Both Hands Up | 57 / 1.9 | 0 | 1.2 deg | modest arm raise; reads as a cheer | `lab-s2-lib-cheer` |
| 8 celebrate | 403 Victory Fist Pump | 46 / 1.5 | 0 | 1.7 deg | hands go to the face and head; does not read as arms up | `lab-s2-lib-victory` |

## Best clip per need

| # | Need | Best | Why / caveat |
|---|---|---|---|
| 1 | skating stride that loops | **Stride run B** (anim `01a1cf31-fcdc-...`); alternates: run A, B5's stride | all three read as skating, none loops: 53 to 85 deg seam. Run B trimmed at frame 106 leaves 27 deg; a 10 to 15 frame cross-blend of the last frames into the first is needed anyway. Drift 1.8 to 6 m: strip it |
| 2 | glide / coast | **GLIDE** `01a1cf32-b421-...` | loops with no work; low crouch, feet together |
| 3 | hard stop or sharp turn lean | **STOP** (partial) | crouch and arms-out read; the skid does not. Code lean (`ANIMATION-SOURCES.md`) is likely the better stop and turn lean |
| 4 | slap shot | **B5's shot** `01a1cf22-f99e-...` | readable wind-up, no stick; a stick on `hand_r` will help |
| 5 | wrist shot | **none usable** | WRIST barely moves; use the slap shot sped up, or re-prompt with a bigger motion |
| 6 | body check | **none clearly usable**; CHECK as a weak stand-in | lunge reads, shoulder does not. Mannequin `MM_Attack_02` is the plan's alternative |
| 7 | knocked down, flat, get up | **KNOCKED** (fall, flat, roll, up in 6 s); fall alone **library 187 Knock_Down** | 187 is the cleaner fall and ends flat; KNOCKED includes the get-up |
| 8 | goal celebration | **CELEBRATE** (fallback library 298) | arms rise and a hop; short |
| 9 | faceoff idle | **FACEOFF** `01a1cf35-e387-...` | loops as is, bent over; no stick in hands |

## Contact sheets as filed (`devlog/shots/`, prefix `20261112-1106xx-B-lab-s2-`)

`stride-b5` (110621), `shot-b5` (110627), `idle-lib0` (110628), `knockdown-lib187` (110629), `golf-lib323` (110631), `stride2a` (110632), `stride2b` (110633), `glide` (110635), `stop` (110636), `wrist` (110637), `check` (110638), `knocked` (110640), `celebrate` (110641), `faceoff` (110642), `lib-sprintstop` (110644), `lib-pushstop` (110645), `lib-headdown` (110646), `lib-standup1` (110648), `lib-cheer` (110649), `lib-victory` (110650), `lib-fallingdown` (110651); each file name ends `.png`, with a caption sidecar. Local copies: `skater-lab\s2\sheets\`. The knocked and standup sheets show huge frames while lying: the render camera follows the pelvis, an artefact of `render.py`.

## What an AnimBP would need

- **Loop:** stride (after fixing the seam), glide, faceoff idle, library Idle. Glide, faceoff and Idle are closed already (0 to 4 deg); the stride is not.
- **One-shot:** slap shot, wrist shot, check, hard stop, celebration, knocked-down (hold, then get-up, or `SetKnockedDown` holds the last frame and a separate get-up plays), 187 fall.
- **Strip pelvis X/Y travel (no root bone, our sim owns movement):** every stride (1.8 to 6 m), the check (44 cm), library Sprint stop (2.4 m) and Head-down Charge (1.7 m), Knock_Down (47 cm back, keep the vertical drop). Idle, glide, faceoff, wrist, celebration and slap shot need nothing but the Z bob. In-place stride speed will not match the sim speed: scale play rate by `speed`.
- Blend glide to stride on `speed` (both crouch, so the pose is close); turn lean from code, not from a clip.
- All of these share one skeleton, so one AnimBP works; every clip FBX re-embeds the 3 MB mesh and texture, import with mesh off.

## Seen on the way

The B5 figure's jersey shows a number (a "29") and a small wordmark on the chest and a logo on the shin pads in several frames (for example `lab-s2-stride-b5`, frame 3). The shared rules say no text, numbers or logos in any kept asset. That is lane B5's texture, not mine; the lead should check the figure's texture before queueing it.

## Could not verify

- Playback: I judged from 8 stills per clip at 192 px; fast motion and foot slide are not checked (no foot-contact measure for ice).
- The seam metric is local bone rotation, no allowance for a half-cycle mirror: a stride that is a clean mirror at half length would show a large seam here and could loop on a half period. The "best trim" search looks only at whole-pose matches to frame 0.
- B5's exact stride and shot prompts (the ledger does not hold them).
- Anything in Unreal (retarget, root handling, import of 28 bones without a root). Text-to-motion variance: same prompt, two runs, 3.8 m and 6.0 m drift and different leans; a single run is not a promise.
- Stick: there is no stick in the mesh, so shots and checks were judged without it.
- Licence terms of the plan beyond what `MESHY.md` states.

## Lead or operator must decide

1. Which stride is the stride, and who builds the loop (a 10 to 15 frame blend or a cut at the cleanest frame, in Blender or in UE).
2. Whether stop, turn lean and check come from code (cheap, reads at 52 m) rather than clips: none of the stop and check clips read well here.
3. Whether another 25 to 40 credits goes into re-prompts for the wrist shot and the check (budget left in S2: 12 credits, so it needs more from the pool).
4. Whether to queue the clip set for import after the figure texture is cleaned of numbers and marks; the 3 shared library presets (187, Idle, Cheer 298) cost 9 credits to regenerate.
5. Confirm text-to-motion output is `ai-generated` and the library presets `sourced` for the ASSETS.md ledger (as `MESHY.md` open question 2).

---

# Engine-ready clips for mascot A and Rodin skater R

Follow-up from the lead (decisions: stride = stride B; stop, turn lean and check come from code; wrist shot = slap shot trimmed; figure 1 is test-only; ship mascot A; Rodin R became the lead's first pick, A second). Done about 11:10-11:20 local. No Unreal.

**Rigs.** A = S1 mascot "A", rig task `01a1cf33-3b37-7559-9f87-cea18ff7a346` (28 bones; pelvis 0.68 m high, standing head about 1.09 m, so a skater about 1.1 m tall and the cm figures below are for that size). R = Rodin no-TAPose, rig task `01a1cf39-74e0-7496-9b7c-cd8a09ffe4d3`. Both have the same 28 bone names, so the same trims and export settings apply.

## Credits

54 credits: per rig 6 animates x 3 (stride B, glide, B5's slap-shot motion, knocked-down, celebration, faceoff) plus one 3-action library animate (187, 298, 0) = 27, twice. S2 total in this report: 138 + 54 = **192** of the 215 allowance (150 + 35 + 30); 23 left. Account balance reads 806 at the end (shared with the other lanes).

Animate tasks, A: Stride `01a1cf3b-406b-72f6-8e32-cadb76853dc3`, Glide `01a1cf3b-7bae-74f0-b0d8-5f9204e834b9`, Shot `01a1cf3b-b65b-707a-a748-40304280b54c`, Knocked `01a1cf3b-f0d2-7080-bebb-28114e1554d2`, Celebrate `01a1cf3c-2b6a-722b-8686-ccb458b7ad83`, Faceoff `01a1cf3c-649e-7704-9f60-b1bf49d39110`, library 187/298/0 `01a1cf3c-c76e-7752-bd32-c73c9d3dfa37`. R, same order: `01a1cf3d-00c6-7157-8856-411d5bc66b29`, `01a1cf3d-3c65-7627-abdb-2f0dd0a9c489`, `01a1cf3d-76a8-70f8-bdd3-d31f7c09c724`, `01a1cf3d-b154-75e2-addf-ca1891dac672`, `01a1cf3d-ea98-722d-9c07-a4e497141449`, `01a1cf3e-2352-7158-b188-5f00899cde66`, `01a1cf3e-5c1e-7478-b2b5-fcd1f931f9a0`. Motion tasks are in the tables above: stride B `01a1cf31-78f9-7509-8281-bbf499e0f6f1`, glide `01a1cf32-2f66-7333-b8e1-4a5dc6bbe2f4`, slap shot `01a1cf22-8a91-7712-8132-b7f0bb2bd0b1`, knocked `01a1cf34-42dc-73c5-b6c6-f45b0c6c433e`, celebrate `01a1cf34-d1ca-7152-aef9-d20ff77e677b`, faceoff `01a1cf35-5f26-75cb-9275-1aa837765138`. Provenance: text-to-motion clips `ai-generated`; library 187 / 298 / 0 `sourced`.

## The script `skater-lab/s2/make_engine_clip.py`

```
B="C:/Program Files/Blender Foundation/Blender 4.2/blender.exe"
"$B" -b --factory-startup --python s2/make_engine_clip.py -- --in X.fbx --out Y.fbx [--action SUB] [--start F --end F | --auto-end]
     [--strip lock|linear|none] [--loop N] [--closed] [--name NAME] [--fps N] [--preview P.fbx] [--report R.json]
```
- Reads one action of the Meshy FBX and samples every frame (bone local location, rotation, scale). It strips the pelvis horizontal travel: default `lock` fixes the pelvis world X/Y to frame 1 and keeps the Z bob and the drop; `linear` removes only the straight-line drift and keeps the sway. Optional trim. Optional loop: the last N frames (N = 15 as used here) are cross-faded into the first: `out[i] = slerp(tail[i], head[i], i/N)` for i < N (rotations slerp, locations and scales lerp), so the clip becomes N frames shorter and the wrap is one natural frame step. `--closed` appends a copy of frame 1 (last frame equals first). `--auto-end` picks the end frame, within the last 40 % of the range, whose pose is closest to the first. It bakes a key on every bone at every frame at the clip's own rate (30 fps) into one action named `--name`.
- FBX export settings: armature only (`--preview` also writes a mesh version, used for the sheets); `use_armature_deform_only=False`; `add_leaf_bones=False` (the 28 bones, including `head_end`, `headfront`, `LeftHand_End`, `LeftToe_end`, are real bones in the source and keep their names); `bake_anim=True`; `bake_anim_use_all_bones=True`; `bake_anim_use_all_actions=False`; `bake_anim_use_nla_strips=False`; `bake_anim_force_startend_keying=True`; `bake_anim_step=1.0`; `bake_anim_simplify_factor=0`; axes -Z forward and Y up (the importer's defaults); `apply_unit_scale=True`; `apply_scale_options=FBX_SCALE_NONE`; `mesh_smooth_type=OFF`.
- After export it re-imports the file and prints `MEASURE_BEFORE` and `MEASURE_AFTER`: frames, seconds, pelvis horizontal travel first to last frame (cm), pelvis dz and Z range, seam (largest local joint rotation difference first to last, in degrees, with the bone), largest pelvis-relative position difference, and the median one-frame step. A JSON report sits next to each FBX.
- Batch: `s2/engine.sh A` and `s2/engine.sh R` (the exact trims are inside). `s2/prof.py` printed the per-frame motion profile I used to pick the trims.

## Engine clips

All at 30 fps. Files: `C:\Users\kazda\jam\skater-lab\s2\engine\A\<Clip>.fbx` and `...\engine\R\<Clip>.fbx` (armature only, 270 to 360 KB) with a `<Clip>.json` beside each. Numbers are A / R where they differ. Travel = pelvis horizontal travel first to last frame, before then after. Seam = largest joint rotation difference first to last frame. Step = median joint change between consecutive frames of that clip.

| Clip | Source | Trim / loop | Frames, seconds | Travel before -> after (cm) | Seam before -> after (deg) | Step |
|---|---|---|---|---|---|---|
| `Stride` | stride B | trim 1-106 (auto-end, nearest pose at 27 deg), 15-frame loop blend | 91, 3.03 | 597 -> **0** / 667 -> **0**; Z bob 9.6 / 11.3 kept | 52.8 -> **10.8** (calf_r) / 53.5 -> **11.0** | 12.2 / 11.6 |
| `Stride_Closed` | same | same plus `--closed` | 92, 3.07 | 0 / 0 | **0 / 0** | |
| `Glide` | glide | whole, 15-frame loop blend | 75, 2.5 | 1.5 -> **0** / 1.6 -> **0** | 4.1 -> **0.2** / 4.0 -> **0.2** | 0.1 |
| `Idle_Faceoff` | faceoff | whole, 15-frame loop blend | 75, 2.5 | 0.5 -> **0** / 0.5 -> **0** | 1.7 -> **0.3** / 1.8 -> **0.1** | 0.1 |
| `Shot_Slap` | B5 slap shot motion | trim 18-66 (dead time 1-17 and the recovery after 66 cut), no loop | 49, 1.63 | 8.2 -> **0** / 9.8 -> **0**; dz -10 | one-shot | 6.1 |
| `Shot_Wrist` | same | trim 33-45 (the swing only) | 13, 0.43 | 8.2 -> **0** / 9.8 -> **0** | one-shot | 15.4 |
| `Fall` | library 187 Knock_Down | whole (falls and ends flat) | 76, 2.53 | 43 -> **0** / 55 -> **0**; dz -46 / -49 kept | one-shot | 10.5 |
| `GetUp` | knocked-down text clip | trim 58-180 (lies still until about frame 58, then rolls, kneels, stands) | 123, 4.10 | 6.6 -> **0** / 12.5 -> **0**; dz +46 / +49 kept | one-shot | 6.4 |
| `Celebrate` | celebration text clip | whole | 90, 3.0 | 15.6 -> **0** / 17.4 -> **0**; jump Z range 33 / 38 kept | one-shot (21) | 8.6 |
| `Celebrate_Cheer298` (fallback) | library 298 | whole | 57, 1.9 | 0 | 1.2 | 10.6 |
| `Idle_Lib0` (spare) | library 0 | whole | 121, 4.03 | 0 | 0 | 2.9 |

All "after" numbers come from re-importing the exported FBX. Pelvis travel is 0 on every clip (target under 2 cm: met). **Seam target (under 3 degrees): met by `Glide`, `Idle_Faceoff` and `Stride_Closed`; not met by the open `Stride` (10.8 / 11.0 degrees).** That value is one ordinary frame step of a fast stride (median 12 degrees), so the wrap is a natural step, not a jump; the blend cannot go lower without a duplicated first frame. Choice for the lead: `Stride` (no repeated frame, right for an engine that loops by wrapping) or `Stride_Closed` (last frame equals the first, so one frame of about 33 ms repeats at the wrap). I would ship `Stride`. Only the horizontal pelvis travel is removed; yaw drift is not.

Correction to my first report: I wrote that the knocked-down text clip "falls backward". The measurements show it starts already lying on the ice (pelvis 43 cm high at frame 1) and lies still for about 2 s before getting up. So `Fall` is library 187 and `GetUp` is the second part of the knocked-down clip.

## Contact sheets as filed (8 frames each, `devlog/shots/20261112-1114xx-B-lab-s2-<A|R>-<clip>.png`)

`lab-s2-A-Stride`, `-Glide`, `-Idle_Faceoff`, `-Shot_Slap`, `-Shot_Wrist`, `-Fall`, `-GetUp`, `-Celebrate`, `-Celebrate_Cheer298`, `-Idle_Lib0`, and the same ten as `lab-s2-R-...`. Local copies: `skater-lab\s2\sheets_e\A\` and `\R\`. `Stride_Closed` has the same frames as `Stride`, so no separate sheet.

## By eye: do the big head, short limbs or mitten hands break a clip?

Judged from 8 stills per clip at 192 px; `render.py`'s camera follows the pelvis, so lying clips are cropped and zoomed.
- **A (mascot):** Stride, Glide and Faceoff read well. The big white gloves swing wide and stay readable, feet stay apart, no crossing. In the shots the gloves pass in front of the face and chin early on and end over the knees; no hand goes through the helmet. In Celebrate the arms are short, so the raised gloves end up against the sides of the helmet (frames 3 to 5), touching or slightly intersecting it: it reads as arms up, but not as a high raise. Fall and GetUp read.
- **R (Rodin):** the render camera shows R from higher up and closer than A, so the side-on checks are less certain. Stride: mitten fists swing in front and behind, legs short, skates do not cross; reads. Glide and Faceoff: the head covers the chest from this angle, fists sit under the chin. In both shots the fists vanish behind the helmet for frames 4 to 8; I cannot tell whether they intersect it. Celebrate: the fists end beside and slightly behind the helmet (frames 3 to 5), the same kind of touching as A. Fall and GetUp read.
- Neither figure has a stick, so the shots show only the hands.

## What still looks wrong

1. The open stride wrap is one frame step (10.8 / 11 degrees), not under 3 degrees (the closed variant is).
2. Celebrate: arms too short on both figures, gloves or fists touch the helmet. Fix with a code-added arm raise, or use the smaller `Celebrate_Cheer298`.
3. Shots: no stick, so the swing reads as hand waving; the wrist clip is only the 0.43 s swing.
4. R's shots could not be judged from this camera; render R from the side before promoting it.
5. `GetUp` ends in a crouch, not the glide pose: blend into Glide about 0.3 s. `Fall` ends flat and holds frame 76 until `GetUp` starts; the 2 s of lying in the text clip is cut.
6. Pelvis yaw drift stays; with `lock` the stride's sideways sway is gone. If the stride looks stiff, re-run with `--strip linear`.
7. Not verified: Unreal import (28 bones, no root), foot sliding against the ice, how the baked take names come in. The re-import in Blender gives the same 28 bones and one action per file.
