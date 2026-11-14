# AR7 report: AR5 seen on the GPU, tuned, and a verdict for the 09:00 package (device B)

Lane AR7, device B, machine clock 03:45 to 04:21 (2026-11-14). Tree `C:\Users\kazda\jam\wt\b-arena`, branch `lane/b-arena` at f0127bab (the lead's commit of AR5). One GPU editor of mine on port 8008 (pid 21560, 58 `LogD3D12RHI` lines), 12 play sessions, left through `ExitEditor` at 04:19 and confirmed gone (04:19:34, `LogExit: Exiting.`). The only unsaved things at exit were the two masters marked by my compile call, with no change in them; they were discarded. No commit, no push, no build, no cook. Console variables read back restored (`jam.Cut hockey.regular`, `jam.ScriptHuman 0`, `jam.ctrl.Drill 0`, `jam.ctrl.HandPuck 0`, `Slate.bAllowThrottling 1`, `r.Editor.Viewport.OverridePIEScreenPercentage 1`). The play world ticked without the Slate setting. Stills were taken at 1442 x 883 with the override at 0.

## Verdict: (a) merge all of AR5 as tuned

| The line | Result |
|---|---|
| A crash | none: 12 play sessions on four rink sizes, 0 asserts, 0 GPU errors in the log |
| A checker material | none: both masters compile (0 material errors in the log), and no still shows the default material |
| A pop of the level switch | not seen: five stills 2 m apart across the switch distance show the same fans with the same outline and colour. **Not filmed in motion** |
| Readability | inside the line on seven frozen pairs: ice mean within 0.02 %, puck within 0.5, every jersey within 2.2 (tuned values: 0.01 %, 0.3, 0.6) |

What speaks for (a) over (b): the roof bays, wall bays and windows are what turns the far wall from a lit plank into a barn in every still, and I found no seam, doubled post or clipping in them at any distance I looked from. What is open, so that you can still choose (b) or (c): this branch has not been cooked; the cheer changes 2 % of the frame, not 5 %; the crowd band reaches the 45 to 55 target only in two of three views; on Pond the windows stand at the top edge of the game camera's frame. Each is below.

## 0. Do the masters compile

- **Yes, both, with no change needed.** `M_ClayCrowd`: 230 vertex and 217 pixel instructions; `Arena/M_ArenaWood`: 147 and 331. 0 `LogMaterial: Error` lines.
- The crowd draws with its own materials on every rink; the stands draw with the wood. 0 `AttachTo` warnings in this process (the root fix of AR5 holds).

## 1. Seen

Regular rink, un-paused clay match and frozen views (`ar7-game-play-1`, `-2`, `ar7-low`, `ar7-far-end`, `ar7-barn`):
- **The crowd reads as people.** Light hats, faces, scarves; no two neighbours alike that I could find. It is no longer a lattice: the benches show only in the empty front row and between groups.
- **Fans sit on the benches.** Seated figures rest on the treads, standing ones stand on them, in the modules, the corners and the terraces. I saw none floating or sunk.
- **The level switch:** no seam between near and far fans in any still; see the line above.
- **The roof and walls:** the wall bays' cross braces and posts read as framing in front of the planks; the old posts do not show as doubles from the distances I looked at (5 to 90 m). The roof bays sit under the slabs with the lamps hanging between them; the 0.82 squeeze does not show. I did not go closer than 5 m to a wall.
- **Bunting:** out of the game camera's frame on the regular rink in every still. From a low camera at an end it shows as a dark line of pennants under the eave.
- **Gate, loft, machine, props:** the loft and its hay show in the +x corner view; the gate, the machine and the props stand behind the end stand and are hidden from the game camera. I did not take a close still of them.

Other rinks (`ar7-pond`, `ar7-wide`, `ar7-large`):
- **Pond:** the crowd in real size behind the boards, close and readable; the loft is left out as built. **The windows are at the top edge of the game camera's frame** there (the barn is at scale 0.667, so they sit at 6.5 to 8.2 m).
- **Wide and large:** a real-size crowd in a very large shed; the band is thin and far, and reads as a crowd, not as giants. Nothing broken.

## 2. Tuned

Saved values (before in brackets): `MI_ClayCrowd_Fan` `FillK` 2.0 (1.4); `MI_ClayCrowd_Skin` `FillK` 0.5 (0.4); `MI_ClayCrowd_Scarf` `FillK` 0.9 (0.7); `MI_ClayBarn_Stand` and `_StandDark` `WashK` 8.5 (6.5); `Arena/MI_Barn_NightGlass` `FillK` 2.8 (0.5). The fill stays neutral.

The crowd band at the game camera (pixels that change when the crowd's owner alone is hidden, in the rows the fans occupy, frozen clay match):

| Moment | Band, share of frame | Mean luma | Median | 99th percentile | Over 140 | Ice in the same frame |
|---|---:|---:|---:|---:|---:|---:|
| 2.3 s (faceoff) | 7.1 % | **35.6** | 32 | 115 | 0.25 % | 162 |
| 6.0 s | 9.2 % | **44.3** | 36 | 156 | 2.0 % | 166 |
| 9.5 s | 8.6 % | **45.1** | 47 | 110 | 0.01 % | 150 |

- **The 45 to 55 target is met in one view, within 1 of it in another, and missed at the faceoff (35.6).** I stopped there: the band's top is already close to the ice (99th percentile 156 against 166 at 6.0 s), and the top is the light hats. More fill lifts hats and coats together.
- **Head-to-head spread:** the standard deviation of luma at the fans' heads (each fan's head projected to the screen) is **35.1 and 37.4** (144 and 166 heads), against Blender's 24.8. The crowd is more varied than the mock-up, not less.
- **More saturated than the jerseys:** of the fans' head and coat samples, **1.4 %, 0.3 % and 0.0 %** are above the jerseys' median chroma (target 0). Above the least saturated jersey in frame: 5 to 10 %.
- **`NightGlass`:** luma **29.6 in the mean, 36.6 at most** at `FillK` 2.8 (it was 3.7 at 0.5 and 55 at 6.0). The Pond and the `ar7-windows` stills were taken at 6.0, before the last value; the `ar7-level-switch` crops show 2.8.

## 3. The cheer, seen

`ar7-cheer-plus`, `ar7-cheer-minus`: a frozen clay match, a game-camera pose toward each end (3.5 m up, 17.65 degrees down, 31 m from the end stand), the end's scalar set to the value the Blueprint writes at each frame. Nothing else differs between the frames.

| Frame after the goal | +8 | +30 | +60 | +100 | +110 | +130 |
|---|---:|---:|---:|---:|---:|---:|
| +x end: share of the frame changing by more than 8 luma | 2.04 % | 2.10 % | 2.17 % | 0.87 % | 0.32 % | 0.39 % |
| -x end | 1.94 % | 1.92 % | 1.93 % | 0.58 % | 0.02 % | 0.03 % |

- **The scored half rises and it reads:** arms go up, seated fans stand, each in its own way; the terrace behind the net is the clearest. No fan tears, nothing pops, the far level cheers like the full one.
- **It is 2 % of the frame, not 5 %** (4.3 to 4.5 % of the upper 45 % of the frame). The pose alone will not reach 5 %: AR3 measured 3.1 % with every fan answering.
- **The ice does not change:** mean 169.51 against 169.52 to 169.55, at most 82 ice pixels of 1.27 million differ.
- The 0.3 to 0.4 % left at +110 and +130 on the +x strip is drift of the slowed match, not the crowd (the -x strip returns to 0.02 %).
- **Not seen together with the other device's red light and caps or the effects lane's dust.** I ran out of time for an un-paused goal at the game camera. In the headless run of AR5 the Blueprint answered the match's real goals, so all three will fire on the same frame; how they sit together is unjudged.

## 4. Readability

Frozen pairs at the game camera, one clay match with the scripted human, everything of AR5 hidden and shown in the same game state (`ar7-pairs`).

| Values | Moment | Ice mean luma, hidden → shown | Puck against the ice | Jerseys against the ice (hidden → shown) |
|---|---|---|---|---|
| as AR5 | 2.3 s | 163.98 → 163.94 (-0.02 %) | -150.5 → -150.6 | blue -93.3 → -93.8; orange -79.2 → -78.0, -89.4 → -89.3 |
| as AR5 | 6.0 s | 166.30 → 166.30 (0.00 %) | -157.9 → -157.9 | blue -117.9 → -117.9; orange -88.4 → -87.9 |
| as AR5 | 9.5 s | 152.24 → 152.20 (-0.02 %) | -144.7 → -144.8 | none on clear ice |
| as AR5 | 13.0 s | 175.89 → 175.85 (-0.02 %) | -181.6 → -182.1 | blue -136.4 → -135.9; orange -100.3 → -98.1 |
| **tuned** | 2.3 s | 163.71 → 163.73 (+0.01 %) | -148.6 → -148.3 | blue -94.0 → -94.0; orange -77.2 → -76.6, -90.3 → -90.6 |
| **tuned** | 6.0 s | 166.50 → 166.49 (-0.01 %) | -158.4 → -158.4 | blue -118.1 → -118.0; orange -88.0 → -87.8 |
| **tuned** | 9.5 s | 152.26 → 152.24 (-0.01 %) | -144.7 → -144.6 | none on clear ice |

- The line is 1 % for the ice and 3 for puck and jerseys. Largest moves with the tuned values: 0.01 %, 0.3 and 0.6.
- Everything of AR5 together takes 12.5 to 18.8 % of the game camera's frame (AR2: 8 to 11 %; the roof and wall bays are the difference).
- `ar7-ar2-side-by-side`: AR2's two un-paused stills from dev-b beside two moments of the same kind of match now (90 and 30 ticks after the first knockdown). As in AR2, two un-paused runs do not reach the same game state, so these are for the eye.

## Not verified

- **A cook and a packaged build of this branch.** This is the largest open item for a 09:00 package.
- Frame time, draw calls, load time.
- The level switch in motion (only stills 2 m apart).
- The cheer on a real un-paused goal, with the goal light, the caps and the dust.
- Cage, the 64 m and the 80 m rink (built, never run with a picture).
- Pond and the windows' still at the final `FillK` 2.8 (the Pond still shows 6.0).
- A close look (under 5 m) at the wall bays against the old posts, the gate, the machine and the props.
- The night and broadcast tiers: untouched, not looked at.
- `r.ForceLOD` did not move the instanced crowd, so I have no forced full-against-far measure.
- AR6's lamp arms and far-end dressing: still not imported (job 6 not reached).

## For the other device

1. **Time the clay tier with and without AR5**, five a side at 1920 x 1080, on the regular rink and on Pond (the crowd is nearest there): frame time on GPU and game thread, draw calls, the menu-to-match frame.
2. **The off switch**, no rebuild: hide the one live actor of class `/JamAssets/Arena/BP_ArenaCrowd` (crowd, stands, gate, loft, machine, props) and the `StaticMeshActor` labelled `Dress_P3_Clay` (roof bays, wall bays, bunting). In the rig file: delete the seven `Crowd_P3_Clay_k*` actors and `Dress_P3_Clay`; the old trusses are then missing too, which only shows from low cameras.
3. A cook warning that names `BP_ArenaCrowd`, `MPC_Crowd`, `M_ClayCrowd` or `M_ArenaWood` is ours.

## Decisions for the operator

1. **How light the crowd may be.** Now: band mean 36 to 45, the lightest hats at luma 150 and more against ice at 166. Stills: `ar7-game-play-1` (now) against `ar7-ar2-side-by-side` (AR2's darker crowd on the left). Lighter is `FillK` on `MI_ClayCrowd_Fan`; I would not go above 2.0.
2. **The windows.** A dim blue at luma 30. On Pond they are inside the game camera's frame. Stills: `ar7-pond` (windows at the top of the first tile, there still at the brighter 6.0) against `ar7-level-switch` (2.8). To take them out of the picture: `FillK` 0.5 on `MI_Barn_NightGlass` (luma 4).
3. **The cheer at 2 % of the frame.** Stills: `ar7-cheer-plus`, `ar7-cheer-minus`. Enough as an answer in the background, or build the lift of the fans' fill during the cheer (one node chain in `M_ClayCrowd`, after 10:00).
4. **Roof and walls in or out** (my (a) against (b)). Stills: `ar7-far-end` and `ar7-barn` (in) against AR2's `20261114-021041-B-ar2-far-end.png` on dev-b (out).

## Files changed since AR5 (f0127bab), all in `C:\Users\kazda\jam\wt\b-arena`

The tree is in the state to merge: everything saved, nothing half-tuned, the editor closed.

- `Game/Plugins/JamAssets/Content/Look/MI_ClayCrowd_Fan.uasset`
- `Game/Plugins/JamAssets/Content/Look/MI_ClayCrowd_Skin.uasset`
- `Game/Plugins/JamAssets/Content/Look/MI_ClayCrowd_Scarf.uasset`
- `Game/Plugins/JamAssets/Content/Look/MI_ClayBarn_Stand.uasset`
- `Game/Plugins/JamAssets/Content/Look/MI_ClayBarn_StandDark.uasset`
- `Game/Plugins/JamAssets/Content/Arena/MI_Barn_NightGlass.uasset`
- `status/AR7-report.md`
- `devlog/shots/20261114-041908-B-ar7-<what>.png` and `.txt`, 14 stills: `game-play-1`, `game-play-2`, `ar2-side-by-side`, `pairs`, `low`, `far-end`, `windows`, `barn`, `cheer-plus`, `cheer-minus`, `level-switch`, `pond`, `wide`, `large`

`status/AR5-film.md` is unchanged; where it says the scarf's and the fans' values, this report's are the current ones.

## Ledger rows

Six rows in the ledger's 11 columns; each replaces the row of the same path from AR5.

```
| Game/Plugins/JamAssets/Content/Look/MI_ClayCrowd_Fan.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_ClayCrowd for coats, hats and mittens (slot ClayCoat): Sat 0.8, FillK 2.0 (1.4 before AR7), FillLow 0.45, Rough 0.9, fill colour a neutral (0.71, 0.71, 0.71) (lanes AR2, AR5, AR7) (9393 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayCrowd_Skin.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_ClayCrowd for faces (slot ClaySkin): Tint (0.55, 0.44, 0.36), FillK 0.5 (0.4 before AR7), fill colour a neutral (0.71, 0.71, 0.71) (lanes AR2, AR5, AR7) (9321 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayCrowd_Scarf.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_ClayCrowd for scarves (slot Scarf): Tint white (the colours are in the vertices since AR6), Sat 0.7, FillK 0.9 (0.7 before AR7), fill colour a neutral (0.71, 0.71, 0.71) (lanes AR2, AR5, AR7) (9448 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayBarn_Stand.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of Arena/M_ArenaWood for the stands planks (slot Timber): WoodLight (0.15, 0.09, 0.052), WashK 8.5 (6.5 before AR7), WashHeightCm 450 (lanes AR2, AR5, AR7) (15118 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayBarn_StandDark.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of Arena/M_ArenaWood for the stands posts and risers (slot TimberDark): WoodLight (0.08, 0.048, 0.028), WashK 8.5 (6.5 before AR7), WashHeightCm 450 (lanes AR2, AR5, AR7) (14554 bytes) |
| Game/Plugins/JamAssets/Content/Arena/MI_Barn_NightGlass.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_ClayCrowd for the wall bays windows (slot NightGlass), the one emissive slot: Tint (0.10, 0.16, 0.34), FillK 2.8 (0.5 before AR7; measured luma 30 on screen, maximum 37), FillLow 1.0, PoseK 0 (lanes AR5, AR7) (10482 bytes) |
```
