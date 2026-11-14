# AR2 report: the timber stands and the clay crowd in the engine (device B)

Lane AR2, device B, machine clock 01:39 to 02:16 (2026-11-14). Main tree `C:/Users/kazda/jam/thirdjam` (`dev-b`, read only; `git log` showed 3e55f946 at the start). Editor on port 8002 (pid 25556, mine, the only editor I started), left through `ExitEditor` at 02:10:17 (`LogExit: Exiting.`, no play session open, dirty list empty just before, process gone). No GPU hang, no restart.

**Two other editors are running at hand-back, neither mine, neither touched:** pid 33564 (worktree `wt/b-vfx`, port 8000, started 01:44:47, so it ran beside mine) and pid 33400 (worktree `wt/b-fig`, port 8004, started 02:12:37). Both were started with `-nullrhi -unattended`, so by their command lines they do not use the GPU.

Housekeeping:
- No commit, no push, no build, no cook. **No frame numbers.**
- Nothing under the other device's paths written. The twelve imported pieces are lane AR1's own (built from primitives on this machine).
- Console variables read back at the end: `r.Editor.Viewport.OverridePIEScreenPercentage 1`, `jam.Cut hockey.regular`, `jam.ScriptHuman 0`, `jam.ctrl.Drill 0`, `jam.ctrl.HandPuck 0`. Stills were taken with the override at 0 (1920 x 1085).
- **Tick rate:** with five Blender lanes on the CPU the un-paused matches ran at 8.1 and 16.7 ticks a second while my frames were being captured. I did not measure the rate without capturing.
- Scratch, with the rig's backup in `bk/`: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/ar2`.

## Verdicts

| # | Job | Verdict | Still |
|---|---|---|---|
| 1 | Import | **DONE**: 12 meshes, triangle counts equal the manifest's, no collision, clay materials bound. No Nanite, no LODs | `ar2-far-end` |
| 2 | Place by the clay rig | **DONE on the regular rink; follows Pond, the wide and the large arena with nothing inside the clear zones.** One thing to decide: the crowd scales with the rig, so fans are 1.0 m tall on Pond and 2.3 m on the wide arena | `ar2-side-corner-ab`, `ar2-pond`, `ar2-wide`, `ar2-large` |
| 3 | Light | **DONE, with no light added**: a warm fill in the materials; band mean luma 34 to 37 against ice 165 | `ar2-low`, `ar2-far-end` |
| 4 | Prove | **PASS on frozen pairs** (ice within 0.03 %, puck within 1.5, every jersey within 2.4). The un-paused before/after runs did not reach the same game state, so they are stills, not a measure | `ar2-game-ab`, `ar2-game-play-1`, `ar2-game-play-2` |
| 5 | The crowd answers a goal | **NOT DONE**: it needs one thing that hears the `goal` event; what it needs is below | none |

## 1. Import

- Twelve FBX from `arena-lab/out/fbx/` into `Game/Plugins/JamAssets/Content/Arena/`, at scale 1, normals from the file, no generated collision, no lightmap UVs, materials off. Read back: 924 / 1,344 / 504 / 768 triangles for the stand pieces and the scoreboard, 540 to 620 per fan, 4,160 and 4,084 per row of seven; every mesh has its vertex colours and 0 collision shapes.
- **Materials** (bound on the mesh assets, by slot):

| Slot | Material | What it is |
|---|---|---|
| `Timber` | `Look/MI_ClayBarn_Stand` (new, on `M_ClayBarn`) | the barn's plank wood, a step lighter than the floor |
| `TimberDark` | `Look/MI_ClayBarn_StandDark` (new, on `M_ClayBarn`) | the barn's dark post wood |
| `ClayCoat` | `Look/MI_ClayCrowd_Fan` (new) | the vertex colour (browns, greys, dull green, mustard, rust, slate, plum), muted to 0.8 saturation |
| `ClaySkin` | `Look/MI_ClayCrowd_Skin` (new) | the skin tones, darkened and with less fill so faces are not pale dots |
| `Scarf` | `Look/MI_ClayCrowd_Scarf` (new) | one neutral oatmeal wool. **Not a team colour**, by your rule; lane AR1 meant this slot for muted team colours |
| `Tin`, `Cream` | `Look/MI_ClayCrowd_Tin`, `_Cream` (new) | dull grey; the boards' cream |
| `LampGlow` | `Look/MI_ClayBulb_Lamp` (existing) | the scoreboard's two lamps |

- **`Look/M_ClayCrowd`** (new master, five instances above): base colour = vertex colour x `Tint`, muted by `Sat`; emissive = a dim warm fill of that colour, stronger on faces that look up.
- **Nanite and LODs: not done.** Neither was one click from the script; the fans are 540 to 620 triangles each.
- **`SM_BarnScoreboard` is imported and not placed.** The manifest wants it over centre ice against a truss; that was not in this lane's jobs.

## 2. Placement

All of it is actors in `LR_P3_Clay`, so it scales with the rink as the barn does. Layout on the regular rink:

| Where | Pieces | Position |
|---|---|---|
| Long sides | 18 modules a side, x scale 0.977 | front edge at \|y\| = 26.5 m (4.0 m beyond the boards), x from -43 to +43 m |
| Ends | 6 modules and 2 standing terraces (the middle two places) an end, scale 1.048 | front edge at \|x\| = 49.0 m |
| Corners | 4 quarter-turn pieces | turn centres at (43.0, 20.5), all four signs |
| Crowd | 195 single fans and 42 rows of seven = **489 fans** in 237 actors | rows 1 to 3 only; **row 0 and the terraces' lower level are empty**; about 45 % of the seats in rows 1 to 3 |

- **Clear zones:** the stands begin at |y| 26.5 m (zone to 24.6 m) and |x| 49.0 m (zone to 46.1 m). The walkway behind device A's boxes and benches is 2.1 m. Everything behind the table and boxes that the game camera hides is timber, not fans' faces: row 0 is empty.
- **Other rink sizes** (read from the running game; the stand line is the rig's scale times 26.5 and 49.0):

| Rink | Rig scale | Stand front, side / end (m) | Device A's bench backs / goal lights end at (m) | A fan's height |
|---|---:|---|---|---:|
| Pond 60 x 30 | 0.667 | 17.67 / 32.7 | 16.94 / 30.86 | 1.0 m |
| Regular 90 x 45 | 1.000 | 26.5 / 49.0 | 24.48 / 45.86 | 1.45 to 1.5 m |
| Large 120 x 60 | 1.333 | 35.3 / 65.3 | 31.94 / 60.86 | 2.0 m |
| Wide 140 x 70 | 1.556 | 41.2 / 76.2 | 36.98 / 70.86 | 2.3 m |

- Nothing crosses a piece on any of the four (`ar2-pond`, `ar2-wide`, `ar2-large`). On Pond the walkway behind the benches is 0.7 m.
- **The scale is the weak point.** The rig scales everything, so on the wide arena the fans are half again as tall as the players, and on Pond two thirds. It reads as a closer or a farther crowd, not as wrong at a glance, but it is not honest to size. A rig cannot hide or re-space its actors per rink size by itself; that needs either three baked layouts chosen by device A's switcher, or device A's rink actor placing the pieces.
- Cage (70 x 35) and Marathon were not run.

## 3. Light

No light actor was added. The stands use the barn wood's own warm wash (`WashK` 6.5) and the crowd the fill in `M_ClayCrowd`; neither casts a shadow, makes a highlight or reaches the ice.

| View (regular rink) | Stands and crowd, share of frame | Band luma: mean / median / 99th percentile | What the band covered before |
|---|---:|---|---:|
| Over the side boards, game height | 18.3 % | 36.5 / 32 / 91 | 48.1 |
| Into a corner, game height | 19.6 % | 34.2 / 31 / 68 | 45.4 |
| Low camera 1.7 m | 7.8 % | 29.6 / 28 / 86 | 42.5 |

- The band is **darker** than the lit wall foot and apron it replaces (48 → 36), the ice is 165 and an orange jersey about 120.
- The brightest things in the band are the scarves and faces; I lowered both once after the first look (faces were pale dots at 99th percentile 140).
- Chroma was not measured. The dark warm wood is saturated in the numbers, as the barn already is; the coats are muted to 0.8.

## 4. Proof

**The measure that holds: frozen pairs.** One clay match with the scripted human, frozen at four moments; at each the game camera's frame was taken twice in the same game state, stands and crowd hidden and then shown.

| Moment | Ice mean luma, hidden → shown | Puck against the ice | Jerseys against the ice (team: hidden → shown) | Stands and crowd in frame |
|---|---|---|---|---:|
| 2.3 s (faceoff) | 164.56 → 164.51 (-0.03 %) | -149.7 → -149.6 | blue -94.2 → -94.7; orange -81.9 → -81.5, -95.8 → -95.7, -104.6 → -104.8 | 8.1 % |
| 6.0 s | 168.50 → 168.48 (-0.01 %) | -161.5 → -161.7 | blue -109.6 → -110.5; orange -55.2 → -52.8, -48.0 → -47.9 | 10.7 % |
| 9.5 s | 173.24 → 173.22 (-0.01 %) | -172.2 → -172.2 | blue -159.7 → -159.8, -132.0 → -131.6; orange -97.5 → -97.3 | 8.2 % |
| 13.0 s (camera lifted at the boards) | 155.61 → 155.63 (+0.01 %) | -144.2 → -142.7 | orange -79.7 → -78.8 | 0.8 % |

Targets were the ice within 1 % and puck and jerseys within 3: the largest moves are 0.03 %, 1.5 and 2.4.

**What did not work as a measure.** I ran the same un-paused match before and after (frames 2, 30, 60 and 90 ticks after the first knockdown). The two runs did not reach the same game state (the puck is in different places from the first frame on), so frame-to-frame numbers between them mean nothing; the "after" frames are filed as the two stills in play.

**Counts.**

| | Added |
|---|---:|
| Actors in `LR_P3_Clay` | 293 (56 stand pieces, 237 crowd actors); the rig goes from 123 to 416 actors |
| Fans | 489 |
| Triangles | 339,596 (stands 51,744; crowd 287,852) |
| Lights | 0 |
| Materials | 1 master, 7 instances |
| Distinct meshes drawn | 11 (the engine can batch equal mesh and material pairs) |
| Rig file | 320,088 → 826,640 bytes |

## 5. The crowd answering a goal: what it would need

Not built. The cheapest honest version is a material pulse: `M_ClayCrowd` reads one scalar (say `MPC_Look.CrowdCheer`, 0 to 1) and lifts the fill and bobs the figures a few centimetres with it. That part is mine and small. What is missing is **something that sets the scalar for the 108 frames**:
- device A's rink actor already hears `goal` and already runs the lamp's curve: one line there writing that curve into `MPC_Look.CrowdCheer` would do it; or
- B's `BP_LookParams` binds the bus event itself: a Blueprint graph edit, which my tools in this lane cannot author.

## Not verified

- A cook and a packaged build (20 new assets, the rig more than twice its size).
- Frame time, draw calls, memory, and the rig's streaming time at match start. The first match waits for the rig, and the rig now loads 293 more actors.
- The saved assets in a fresh editor process (saves returned true, the dirty list was empty, the stills come from fresh play sessions after the saves).
- Readability in un-paused play as a measure (see above); motion over 30 frames; a goal moment with the crowd in frame.
- Chroma of the crowd against the jerseys.
- Cage and Marathon; the corner pieces against the rounder corners of other rinks beyond what the stills show.
- The night and broadcast tiers: untouched by construction (their rigs are separate); no still taken.
- Whether lane AR1's sockets came through the import (I placed by the manifest's numbers).

## For the other device

1. **Please time the clay tier with and without the stands** on your machine, five a side at 1920 x 1080: GPU and game-thread frame time, draw calls, and the menu-to-match frame. To switch them off without a rebuild: hide the actors whose labels start `Stand_P3_Clay` and `Fan_P3_Clay` in `LR_P3_Clay` (293 actors, 339,596 triangles, no lights).
2. **The stands follow your clear zones as B41 gave them**; on Pond the walkway behind your benches is 0.7 m. If a bench, box or light grows, tell B.
3. **A goal cheer needs one line of yours:** write your lamp curve (0 to 1 over the 108 frames) into a scalar of `/JamAssets/Look/MPC_Look` (B adds `CrowdCheer` on request); B's material does the rest.
4. **Fans' size on rinks other than 90 x 45** follows the rig's scale. If the crowd should keep its size, the placing has to move to your rink actor or your switcher has to choose between baked layouts; say which you prefer.

## Decisions for the operator

1. **How full:** 489 fans, about 45 % of the seats behind an empty front row. Fuller or sparser is two numbers in the placing script.
2. **How bright:** the band's mean is 35 against the ice's 165 and darker than the wall it covers. One value each: `FillK` on `MI_ClayCrowd_Fan` (1.4), `WashK` on the two stand woods (6.5).
3. **Scarves:** neutral now. Muted team colours (one end mostly blue, one mostly orange) is what lane AR1 drew; it would put team hues in the crowd.
4. **Other rink sizes:** keep the scaled crowd, or stands without fans there, or a lane to place per rink.
5. **The barn scoreboard:** hang it over centre ice, or leave it out.

## The next three evolutions, each with a proving number

1. **The crowd answers a goal** (with device A's one line): band mean luma up by 10 to 20 for the 108 frames and back to rest by frame 120, ice mean shift under 1.0 in every channel throughout.
2. **A crowd of honest size on every rink:** a fan's height within 5 % of 1.45 m on the five shipped rinks, zero overlap with the four clear zones on each.
3. **The cost, measured and cut if needed** (instancing by mesh, a far LOD for the rows): device A's frame time on clay within 0.3 ms of the tier without stands, at five a side.

## Assets (bytes on disk, `Game/Plugins/JamAssets/Content/`)

| File | Before | After |
|---|---:|---:|
| `Rigs/LR_P3_Clay.umap` | 320,088 | 826,640 |
| `Arena/SM_Stand_Mod`, `_Corner`, `_End`, `SM_BarnScoreboard` | new | 133,310; 138,855; 107,008; 120,252 |
| `Arena/SM_Fan_A` to `_F` | new | 107,628; 109,300; 103,814; 102,978; 105,572; 106,787 |
| `Arena/SM_FanRow_A`, `_B` | new | 339,020; 334,013 |
| `Look/M_ClayCrowd` | new | 16,173 |
| `Look/MI_ClayCrowd_Fan`, `_Skin`, `_Scarf`, `_Tin`, `_Cream` | new | 9,636; 9,737; 9,712; 9,671; 9,732 |
| `Look/MI_ClayBarn_Stand`, `_StandDark` | new | 12,663; 11,702 |

The twelve meshes are 1,808,537 bytes together.

## Ledger rows

The first replaces the row of the same path; the other twenty are new.

```
| Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap | other | 0.83 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | P3 clay rig, barn with calm ice: PPV (EV100 4.2, SSR override, focus 16 m f/4, no depth blur, motion blur 0), sky light a movable real-time capture (no cubemap; intensity 0.35 and colour unchanged) of the capture dome SkyCap_P3_Clay (engine sphere, scale 700, MI_SkyAmbient_Clay, outside the barn; B34), 4 shadowed warm key spots (5200 cd, no specular, shadow resolution 1.0 and source radius 60 cm since B34), 8 shadowless lamps over the side lanes (2100 cd, broad beams, no specular; B28), 6 shadowless fills at lower intensity (B28), 18 lamp shades (6 of them decorative beyond the end boards, twice as wide since B29), LookParams (FeltRim 1.4); plank walls, posts, trusses, roof, floor; no bulb strings; all shadowless meshes, no collision; timber stands and a clay crowd round the rink: 48 bleacher modules, 4 terraces behind the nets, 4 corner pieces (Stand_P3_Clay_01..56, front edge 4 m beyond the boards) and 237 crowd actors (Fan_P3_Clay_001..237: 195 single fans and 42 rows of seven, front row empty), no light added, no shadows, no collision (lanes B9, B14, B18, B19, B22, B23, B27, B28, B29, B34, AR2) (826640 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Stand_Mod.uasset | mesh | 0.13 | original | Blender 4 script (lane AR1, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | timber bleacher module, 4 rows, 4.89 x 4.01 x 3.99 m, 924 tris, slots Timber and TimberDark; no collision, vertex colours, materials bound to the slots (lane AR2) (133310 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Stand_Corner.uasset | mesh | 0.14 | original | Blender 4 script (lane AR1, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | timber bleacher quarter turn, inner radius 6 m, 1344 tris, slots Timber and TimberDark; no collision, vertex colours, materials bound to the slots (lane AR2) (138855 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Stand_End.uasset | mesh | 0.11 | original | Blender 4 script (lane AR1, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | standing terrace for behind the nets, 2 levels, 4.89 x 2.40 x 2.25 m, 504 tris, slots Timber and TimberDark; no collision, vertex colours, materials bound to the slots (lane AR2) (107008 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_BarnScoreboard.uasset | mesh | 0.12 | original | Blender 4 script (lane AR1, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | timber and tin scoreboard box on chains, 768 tris, slots Timber, TimberDark, Tin, Cream, LampGlow; imported, not placed in any rig; no collision, vertex colours, materials bound to the slots (lane AR2) (120252 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_A.uasset | mesh | 0.11 | original | Blender 4 script (lane AR1, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, seated, hands on knees, 608 tris, slots ClaySkin, ClayCoat, Scarf; no collision, vertex colours, materials bound to the slots (lane AR2) (107628 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_B.uasset | mesh | 0.11 | original | Blender 4 script (lane AR1, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, seated, arms up, 620 tris, slots ClaySkin, ClayCoat, Scarf; no collision, vertex colours, materials bound to the slots (lane AR2) (109300 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_C.uasset | mesh | 0.10 | original | Blender 4 script (lane AR1, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, standing, fist raised, 540 tris, slots ClaySkin, ClayCoat, Scarf; no collision, vertex colours, materials bound to the slots (lane AR2) (103814 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_D.uasset | mesh | 0.10 | original | Blender 4 script (lane AR1, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, standing, leaning on a rail, 544 tris, slots ClaySkin, ClayCoat, Scarf; no collision, vertex colours, materials bound to the slots (lane AR2) (102978 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_E.uasset | mesh | 0.11 | original | Blender 4 script (lane AR1, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, standing with a tin mug, 572 tris, slots Tin, ClaySkin, ClayCoat, Scarf; no collision, vertex colours, materials bound to the slots (lane AR2) (105572 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Fan_F.uasset | mesh | 0.11 | original | Blender 4 script (lane AR1, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | clay fan, seated child, 604 tris, slots ClaySkin, ClayCoat, Scarf; no collision, vertex colours, materials bound to the slots (lane AR2) (106787 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_FanRow_A.uasset | mesh | 0.34 | original | Blender 4 script (lane AR1, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | row of 7 clay fans along 4.06 m of bench, 4160 tris, slots ClaySkin, ClayCoat, Scarf; no collision, vertex colours, materials bound to the slots (lane AR2) (339020 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_FanRow_B.uasset | mesh | 0.33 | original | Blender 4 script (lane AR1, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | row of 7 clay fans along 4.06 m of bench, 4084 tris, slots Tin, ClaySkin, ClayCoat, Scarf; no collision, vertex colours, materials bound to the slots (lane AR2) (334013 bytes) |
| Game/Plugins/JamAssets/Content/Look/M_ClayCrowd.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | master for the clay crowd and small stand parts: vertex colour (decoded) x Tint, muted by Sat, with a dim warm fill of its own colour that is stronger on faces looking up (AR2CrowdBase, AR2CrowdFill); no light actor needed (lane AR2) (16173 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayCrowd_Fan.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_ClayCrowd, coats, hats, mittens, trousers (slot ClayCoat): Sat 0.8, FillK 1.4 (lane AR2) (9636 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayCrowd_Skin.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_ClayCrowd, faces (slot ClaySkin): Tint (0.55, 0.44, 0.36), FillK 0.4, kept darker than the boards (lane AR2) (9737 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayCrowd_Scarf.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_ClayCrowd, scarves (slot Scarf): neutral oatmeal Tint (0.36, 0.33, 0.26), FillK 0.7; never a team colour (lane AR2) (9712 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayCrowd_Tin.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_ClayCrowd, tin (slot Tin): Tint (0.30, 0.31, 0.33) (lane AR2) (9671 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayCrowd_Cream.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_ClayCrowd, the scoreboard panels (slot Cream): Tint (0.72, 0.69, 0.64) (lane AR2) (9732 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayBarn_Stand.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_ClayBarn for the stands planks (slot Timber): WoodLight (0.15, 0.09, 0.052), WashK 6.5, WashHeightCm 450 (lane AR2) (12663 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayBarn_StandDark.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_ClayBarn for the stands posts and risers (slot TimberDark): WoodLight (0.08, 0.048, 0.028), WashK 6.5, WashHeightCm 450 (lane AR2) (11702 bytes) |
```

## Stills (`C:/Users/kazda/jam/thirdjam/devlog/shots/`, each with a `.txt`)

- `20261114-021041-B-ar2-game-play-1.png`, `20261114-021041-B-ar2-game-play-2.png` (un-paused)
- `20261114-021041-B-ar2-game-ab.png` (frozen pairs)
- `20261114-021041-B-ar2-low.png`
- `20261114-021041-B-ar2-far-end.png`
- `20261114-021041-B-ar2-side-corner-ab.png`
- `20261114-021041-B-ar2-pond.png`, `20261114-021041-B-ar2-wide.png`, `20261114-021041-B-ar2-large.png`
