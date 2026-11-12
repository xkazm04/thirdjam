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
| 7 knocked down + get up | "a skater is knocked down on the ice: falls backward hard, lands lying flat on the back with arms and legs spread, lies still for a moment, then rolls to the side, pushes up onto hands and knees, and stands up" (6 s) | `01a1cf34-42dc-73c5-b6c6-f45b0c6c433e` / `01a1cf34-9ec2-74d1-bf45-66ac25bddbd9` | 180 / 6.0 | 9 cm; pelvis dz +41 (it ends standing) | one-shot | **yes**: lies flat on the back (about 1.5 s), rolls, crouches, stands. Ends in a crouch, not a skating stance; `lab-s2-knocked` | 13 |
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
