# AR14 report: the masters' script, the bigger cheer, the treads and the night, seen on the GPU (device B)

Lane AR14, device B, machine clock 06:43 to 07:27 (2026-11-14). Tree `C:\Users\kazda\jam\wt\b-arena` over f42515a0. Two GPU editor processes of mine on port 8008, one after the other: pid 31292 (the work; log `Game/logs/editor-20261114-064356-0.log`) and pid 31332 (a restart to read the saved state back from disk; log `editor-20261114-071821-0.log`). Both left through `ExitEditor`, both confirmed gone. Another `UnrealEditor.exe` (pid 27948) is running on this machine; it is not mine and I did not touch it. No commit, no push, no cook, no build, nothing under the other device's paths written. `r.Editor.Viewport.OverridePIEScreenPercentage` was 0 for the stills and is back at 1.

## Verdicts

| State | Verdict | In one line |
|---|---|---|
| Job 2 (lamp arms, far end; ea90e4ad) | **MERGE as is** | Unchanged since AR10; it is in every clay still of this lane and nothing of it shows a fault. |
| AR11 night (bcb81e3e) | **MERGE AS TUNED** | As built the night crowd was black shapes. Tuned (crowd fill about 4.5 times up, stands' wash 1.2 to 6.0) it reads as dim blue people, far darker than the boards. Ice unchanged with it on. One named gap: no wave and no second pose on night (old rig data). |
| AR13 cheer, treads, scoreboard (f42515a0) | **MERGE AS TUNED** | Both masters compile with the script, no checker anywhere, UV channels 5 to 7 are read on the instanced path. One change to the script: the wood takes the vertex colour as it is (the 2.2 decode made the stands nearly black). Stands' wash 8.5 to 20. |

**What "as tuned" means in files:** the ten assets and one script changed in this lane (listed under "Files"). They are saved and were read back from disk in a fresh editor.

**If a later state does not merge:**
- Job 2 alone: ea90e4ad as committed. Needs nothing from this lane.
- Job 2 and AR11 without AR13: bcb81e3e as committed, plus only the four `Arena/MI_NightCrowd_*.uasset` of this lane. Leave the two `Arena/MI_NightBarn_Stand*.uasset` at their committed bytes: their new wash was chosen against the wood master with the vertex-colour multiply, which is AR13's. **Not checked:** the four tuned crowd instances against the older crowd master (I only saw them with AR13's).
- All three: f42515a0 plus everything under "Files".

## Files

Assets saved by this lane (bytes before to after):

| Asset | Before | After | What changed |
|---|---|---|---|
| `Look/M_ClayCrowd` | 28,933 | 44,671 | The script's crowd part: pose B, the wave, the bounce. Compiled. |
| `Arena/M_ArenaWood` | 24,975 | 31,499 | The script's wood part, with power 1.0 in place of 2.2. Compiled. |
| `Look/MI_ClayBarn_Stand` | 15,118 | 15,148 | WashK 8.5 to 20 |
| `Look/MI_ClayBarn_StandDark` | 14,554 | 14,736 | WashK 8.5 to 20 |
| `Arena/MI_NightCrowd_Fan` | 6,068 | 9,463 | FillK 0.35 to 1.6 |
| `Arena/MI_NightCrowd_Skin` | 6,074 | 9,431 | FillK 0.45 to 2.0 |
| `Arena/MI_NightCrowd_Scarf` | 6,080 | 9,484 | FillK 0.4 to 1.8 |
| `Arena/MI_NightCrowd_Tin` | 6,068 | 9,279 | FillK 0.3 to 1.4 |
| `Arena/MI_NightBarn_Stand` | 5,991 | 11,713 | WashK 1.2 to 6.0 |
| `Arena/MI_NightBarn_StandDark` | 6,015 | 10,874 | WashK 1.2 to 6.0 |

Also changed: `status/AR13-master.py` (the wood's power constant 2.2 to 1.0 and its comment, so the script builds what is saved). New: this report, 25 stills with captions in `devlog/shots/20261114-0724-B-ar14-*.png`.

Not changed: both rigs, `BP_ArenaCrowd`, every mesh, the four `Look/MI_ClayCrowd_*`, `MPC_Crowd`, `Look/M_Toon`.

## 1. The masters' script

- Run in the GPU editor: `M_ClayCrowd` "built; 26 old nodes removed", 248 vertex and 217 pixel instructions (230 and 217 before). `M_ArenaWood` "built; base colour and 2 custom input(s) now read base x vertex colour", 152 and 339 (147 and 331 before).
- **Neither master failed. No copy was taken back.**
- **No default checker** on clay or night in any still. Both logs: 0 "Default Material" lines, 0 "usage flag" lines, 0 material compile errors.
- **The instanced path reads UV channels 5 to 7.** Risen, every fan on pose A against every fan on pose B: 2.84 % of the frame differs. The shipped mix against all-A 1.49 %, against all-B 1.61 %. Still: `target-a-b`.
- **One change to the script.** As written (vertex colour decoded with power 2.2) the stands went nearly black: crowd band mean 24.0, a close stand view fell from 47.4 to 14.5 where it changed. With power 1.0 the treads are dark and the seat planks are not. Still: `masters-before-after`.

## 2. The bigger cheer on clay

What a viewer sees: at a goal the fans stand, arms and scarves go overhead, and neighbours do different things. From the game camera it is a clear lift of the whole band behind the boards; the ice does not change.

| Distance from the end stand | Blender (AR12) | Before AR13 | Measured now (share of the frame changing by more than 8 luma) |
|---|---|---|---|
| 15 m | 9.4 % | 4.0 % | **6.46 %** |
| 25 m | 5.8 % | 2.5 % | **4.76 %** (5.24 % at the shipped wash, and again from disk) |
| 40 m | 2.5 % | 1.0 % | **4.06 %** |

- **Ice unchanged:** mean 182.93 to 182.94; 4, 73 and 660 ice pixels changed at the three distances.
- **The wave, end stand** (25 m, share of the frame): +8: 4.19 %, +14: 4.75 %, +20: 4.76 %, +28: 4.75 %, +60: 4.75 %, +108: 0.03 %, +130: 0.02 %. The end stand is nearest the net, so it is nearly all up by frame 8. Still: `wave-end`.
- **The wave, side stand** (seen from across the rink, net to the right; share of the stand rows changed, by sixths of the width, left to right): +8: nothing; +14: 0, 0, 0.1, 0, 0, **6.8** %; +20: 0, 0, 0, 6.9, 15.5, 20.0 %; +28: 0, 0, 0, 19.8, 20.9, 20.8 %. It starts at the net and reaches centre ice by frame 28; the other half of the building stays seated. Still: `wave-side`.
- **The bounce** (un-paused, cheer held): 1.1 to 2.2 % of the frame differs between two captures with BounceCm 15, 0.02 % with 0. Still: `bounce-frames`.
- **Neighbours, A against B:** they differ, visibly, in the close view. Still: `neighbours`.
- **Arms through neighbours:** yes, in places. Raised scarves and arms cross into the next seat in the close view. From the game camera I cannot see it. Risen fans cover the legs of the row behind, as a standing crowd does; I saw no figure cut through a bench or a rail. Stills: `neighbours`, `rows`.
- **The level switch during a cheer:** the far side stand risen at 26, 30 and 34 m shows the same poses; I see no step. Still: `level-switch`.
- **Readability pairs** (frozen at 2.3 s, all of B off against on):

| Pair | Ice luma | Puck against ice | Jerseys against ice |
|---|---|---|---|
| Crowd at rest | 164.12 to 164.10 (-0.01 %) | -151.1 to -151.3 | within 0.3 |
| Both ends risen behind the play | 164.12 to 164.13 (+0.00 %) | -151.1 to -151.0 | within 0.8 |

**Values I would ship (they are the master's defaults, nothing to save):** PoseK 1.0, WaveK 2.5, BounceCm 15.

## 3. The darker treads and the scoreboard

- **Empty benches read as benches now:** light seat planks on darker treads and risers. Stills: `wash-steps`, `clay-game-rest`.
- **The barn's other timber did darken a little.** Same frame, same wash, far-end camera, top fifth of the frame (roof and end wall): 4.80 before, 3.63 with the script as written, 4.27 with power 1.0. The roof seen from below with the script as written: 10.65 to 8.79. Not checked from below with power 1.0. At the shipped wash the top fifth reads 5.48.
- **The wash**, crowd band mean at the game camera:

| WashK | 8.5 (power 2.2) | 8.5 | 11 | 14 | 16 | **20** | 24 |
|---|---|---|---|---|---|---|---|
| Band mean | 24.0 | 28.8 | 31.2 | 33.5 | 34.7 | **38.4** | 41.9 |

Shipped: 20. Read back from disk: **38.7** (the same pixels with the crowd hidden: 41.2; AR7's band was 35.6).
- **The scoreboard:** seen from the low camera looking up 38 degrees (a cream face under the truss), small from the far end. It is in neither of two un-paused game-camera frames nor in the frozen one. Still: `scoreboard`. Regular rink only.

## 4. Night

**As AR11 built it:** stands and crowd are black shapes against the star sky, on every camera. Median luma of B's pixels 6.2 and 1.9 at the two moments; the boards' top under them 89 and 92. Darker than the boards, yes; people only as outlines. Stills: `night-as-built`, `night-rinks`.

- **Seen:** two un-paused moments, the low camera, the far end, the bench side, the box side, the end terrace, Pond and the wide arena (game camera and a high corner each). 554 fans and 46 stands on the regular rink, 208 and 24 on Pond, 861 and 78 on wide.
- **The gaps behind the benches and boxes** are there: the stands stop each side of them and the lit benches stand clear.
- **The end terraces:** two blocks with the red goal lamp between them; they read under the end fills.
- **Readability pairs.** My measure on dark ice: the lower 58 % of the frame, figures and every changed pixel cut out, mean luma; the puck as its glow against a ring beside it.

| Moment | Ice floor | Puck glow against ring |
|---|---|---|
| 2.3 s | 121.68 to 121.64 (-0.03 %) | 66.3 to 66.1 |
| 6.0 s | 84.86 to 84.87 (+0.01 %) | 158.3 to 158.2 |

Jerseys: within 1.2, except two far orange skaters at 2.3 s that moved by about 11 (-55.3 to -66.0, -63.8 to -75.5). The difference picture shows a one-pixel shimmer on outlines (the frame is not fully still). **I did not trace those two further.** Still: `night-pair`.

- **Tuning, four steps** (crowd FillK fan / skin / scarf / tin, stands' WashK). The crowd's Tint is not changed.

| Step | Values | End terrace block: mean, median | Bench-side stand band: mean | Lower 55 % of the game frame |
|---|---|---|---|---|
| As built | 0.35 / 0.45 / 0.4 / 0.3, 1.2 | 20.5, 1.1 | 5.4 | 114.26 |
| 2 | 0.9 / 1.1 / 1.0 / 0.8, 3 | 22.2, 3.4 | 6.8 | 114.23 |
| **3, shipped** | **1.6 / 2.0 / 1.8 / 1.4, 6** | **24.8, 7.4** | **8.8** | **114.21** |
| 4 | 2.6 / 3.2 / 3.0 / 2.3, 10 | 28.3, 13.5 | 11.6 | 114.21 |

The boards under them read 156 to 158 at every step; the lower frame with the stands off is 114.25. At step 3 the crowd is dim blue people with pale faces. At step 4 the faces start to read as pale dots from the game camera. From disk after the restart: block mean 24.8, median 7.4; lower frame 114.36 on and off. Stills: `night-tune`, `night-from-disk`.

- **The blocking-load line** (ms, as logged):

| | First night load in a process | Repeats |
|---|---|---|
| Before (old night rig, main tree logs of 03:11 and 05:06) | 91, 104 | 23 |
| After (this lane) | 145, 100 | 38, 38 (Pond), 54 (wide), 49 |

Clay in the same logs for comparison: 755 and 213 on a process's first load, then 45, 51, 39, 45.

- **The cheer on night:** 3.24 % of the frame at 25 m as built, **4.43 %** as tuned, from disk. Still: `night-cheer`. **The night rig still has AR11's data:** every fan takes pose A, and the delay is the old random one, so there is no wave from the net and no A/B mix on night.

## Decisions for the operator

1. **The size of the cheer.** PoseK 1.0 (5.24 % of the frame at 25 m; shipped) or 0.6 (4.17 %). Stills: `20261114-0724-B-ar14-posek-10.png`, `...-posek-06.png`.
2. **The wave.** WaveK 2.5, a front running from the net (shipped), or 1.0, a soft swell where the scored half rises almost together. Stills: `...-wavek-25.png`, `...-wavek-10.png`.
3. **The night crowd.** Black shapes against the sky (as built) or dim blue people (shipped). Stills: `...-night-crowd-shapes.png`, `...-night-crowd-people.png`.

The bounce (15 cm or none) is a fourth choice, but a still cannot show it; it needs a look in motion.

## Not verified

- **Nothing was watched in motion.** The cheer, the wave and the bounce are judged from stills and differences between captures.
- **Clay after the masters' change: regular rink only.** Pond, the wide arena and the other sizes were not looked at in this lane. The scoreboard's height on the other rinks is AR13's numbers, not seen.
- **Tuned night: regular rink only.** Pond and wide were seen as built.
- PoseK 0.6, WaveK 1.0 and 4.0 were each seen at one frame from one camera. Other bounce heights were not tried.
- WashK 22 would land nearer 40; I shipped a measured step (20), not an interpolated one.
- The level switch was checked with the cheer held at three distances, not by moving a camera through 30 m while the crowd rises.
- The barn dressing's wood instances (`Arena/MI_Barn_*`) were not saved again; they loaded and drew from disk.
- No cook, no package: nothing here is checked in a cooked game.
- **Job 6 (AR11's job 5, the wall pieces at real size on the wide arena) was not done.**

## For the other device

- Nothing under your paths was written. The switch (`ke * ArenaCrowd 0`, `1`, `2`) is unchanged.
- The goal cheer now moves 4 to 6.5 % of the frame at the scored end (2.1 % at AR7). Ice, puck and jersey numbers hold with it risen behind the play, on clay and on night.
- Night loads with stands and crowd log 38 to 54 ms on a repeat against 23 before.
- If you time the stands again, the crowd master is 248 vertex instructions against 230.

## Ledger rows (full, 11 columns)

| Game/Plugins/JamAssets/Content/Look/M_ClayCrowd.uasset | material | 0.04 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | master for the clay crowd and small parts: vertex colour (decoded) x Tint x a per-instance value chosen by the vertex alpha (coat: custom data 1; hat, scarf, mittens: data 2), muted by Sat, with a dim fill of its own colour; world position offset = the cheering pose (pose A from UV2 to UV4, pose B from UV5 to UV7, chosen per instance by data 5) x per-instance weight (data 0) x PoseK x a smoothed rise of the scalar of the instance own end (data 3; MPC_Crowd CrowdCheerPlusX or CrowdCheerMinusX) held back by WaveK x the delay from the net (data 4), plus a bounce of BounceCm straight up with a phase from data 1; defaults PoseK 1.0, WaveK 2.5, BounceCm 15; built by status/AR13-master.py and compiled on the GPU; used with instanced static meshes (lanes AR2, AR5, AR13, AR14) (44671 bytes) |
| Game/Plugins/JamAssets/Content/Arena/M_ArenaWood.uasset | material | 0.03 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | a copy of Look/M_ClayBarn with the instanced-static-mesh usage flag, parent of the stand woods and the barn dressing woods on instanced components; base colour and the wash and spill made from it multiplied by the vertex colour taken as it is (power 1.0; the 2.2 decode made the stands nearly black); built by status/AR13-master.py and compiled on the GPU (lanes AR5, AR13, AR14) (31499 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayBarn_Stand.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of Arena/M_ArenaWood for the stands planks (slot Timber): WoodLight (0.15, 0.09, 0.052), WashK 20 (8.5 before AR14, 6.5 before AR7; lifted to hold the crowd band near 40 under the vertex-colour multiply), WashHeightCm 450 (lanes AR2, AR5, AR7, AR14) (15148 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayBarn_StandDark.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of Arena/M_ArenaWood for the stands posts and risers (slot TimberDark): WoodLight (0.08, 0.048, 0.028), WashK 20 (8.5 before AR14, 6.5 before AR7), WashHeightCm 450 (lanes AR2, AR5, AR7, AR14) (14736 bytes) |
| Game/Plugins/JamAssets/Content/Arena/MI_NightCrowd_Fan.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | night instance of M_ClayCrowd for coats, hats and mittens: Tint (0.30, 0.36, 0.50), Sat 0.35, FillK 1.6 (0.35 before AR14: black shapes on screen), a cool fill (0.55, 0.70, 1.0); bound as an override on the night rig components only; tuned on the GPU (lanes AR11, AR14) (9463 bytes) |
| Game/Plugins/JamAssets/Content/Arena/MI_NightCrowd_Skin.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | night instance of M_ClayCrowd for faces: Tint (0.34, 0.33, 0.36), Sat 0.5, FillK 2.0 (0.45 before AR14), cool fill; bound as an override on the night rig components only; tuned on the GPU (lanes AR11, AR14) (9431 bytes) |
| Game/Plugins/JamAssets/Content/Arena/MI_NightCrowd_Scarf.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | night instance of M_ClayCrowd for scarves: Tint (0.45, 0.50, 0.60), Sat 0.3, FillK 1.8 (0.4 before AR14), cool fill; bound as an override on the night rig components only; tuned on the GPU (lanes AR11, AR14) (9484 bytes) |
| Game/Plugins/JamAssets/Content/Arena/MI_NightCrowd_Tin.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | night instance of M_ClayCrowd for tin parts: Tint (0.20, 0.22, 0.26), FillK 1.4 (0.3 before AR14), cool fill; bound as an override on the night rig components only; tuned on the GPU (lanes AR11, AR14) (9279 bytes) |
| Game/Plugins/JamAssets/Content/Arena/MI_NightBarn_Stand.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | night instance of Arena/M_ArenaWood for the stands planks: WoodDark (0.012, 0.016, 0.024), WoodLight (0.040, 0.050, 0.070), WashK 6.0 (1.2 before AR14), a cool wash (0.45, 0.60, 1.0); bound as an override on the night rig components only; tuned on the GPU against the wood master with the vertex-colour multiply (lanes AR11, AR14) (11713 bytes) |
| Game/Plugins/JamAssets/Content/Arena/MI_NightBarn_StandDark.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | night instance of Arena/M_ArenaWood for the stands posts and risers: WoodDark (0.008, 0.010, 0.016), WoodLight (0.022, 0.028, 0.040), WashK 6.0 (1.2 before AR14), cool wash; bound as an override on the night rig components only; tuned on the GPU against the wood master with the vertex-colour multiply (lanes AR11, AR14) (10874 bytes) |
