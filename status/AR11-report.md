# AR11 report: the night tier gets the stands and the crowd; the wall pieces at real size on clay (device B, no GPU)

Lane AR11, device B, machine clock 05:46 to 06:00 (2026-11-14). Tree `C:\Users\kazda\jam\wt\b-arena` at ea90e4ad. One renderer-less editor of mine on port 8008 (pid 25820, `-nullrhi -unattended`, 0 `LogD3D12RHI` lines), 10 play sessions, left through `ExitEditor` with nothing unsaved and confirmed gone (05:55:34, `LogExit: Exiting.`). No GPU editor, no commit, no push, no cook, no build (the tree was built at a5c4ba47 and your commit on top carries no code). Console variables restored (`Slate.bAllowThrottling 1`, `jam.Cut hockey.regular`, `jam.ScriptHuman 0`). 59.6 and 60 ticks per wall second in the two goal tests. 0 warnings naming B's arena, 0 Blueprint runtime errors, 0 `AttachTo` warnings, 0 asserts in the log.

**Nothing in this lane was seen.** Every verdict is a count. Nothing of it is for the 10:00 package.

## Verdicts

| # | Job | Verdict |
|---|---|---|
| 1 | Stands and crowd on the night rig | **DONE by counts.** The same `BP_ArenaCrowd`, seven baked layouts, six new night instances as component overrides. Switch and cheer work as on clay. Two things the night rig forced: no modules where its three low fills point (10 a rink), and no corner stands on rinks under 90 m (its towers and mounds stand there) |
| 2 | Roof and walls on night | **Left out, on purpose.** The night tier already has its own room, sky and a lighting truss on four towers; a timber barn would stand inside them. What belongs there is below |
| 3 | Proof without a picture | **DONE**, four rink sizes, with one hole: the `blocking load` line is not written by a renderer-less editor (0 lines in 6 night sessions), so before and after are in the film script for the GPU |
| 4 | The filming script | **DONE**: `status/AR11-film.md` |
| 5 | Wall pieces at real size on clay | **DONE as a separate state**, proven by counts on four rinks; not in the tree's rig |

## 1. Stands and crowd on `LR_L2_Night`

**What the night rig is** (read in a headless night session): a dark room 120 x 74 x 28 m of `MI_Toon_Dark` (the star dome and the dark-volume walls are in the file, the walls hidden), a lighting truss at 27 m on four towers at (+-52, +-30) m with four spots, six snow banks along the boards (ends 45.7 to 47.8 m, sides 23.2 to 25.3 m, 0.6 m high, split round the boxes and benches), four snow mounds under the towers, two end fills (spots, 12 m up at x = +-50 m, 700 cd, a 75 degree cone, pointing down and in), and the three low rect fills behind the boards (450 cd, 7 m reach, pointing **away** from the ice at the boxes and the benches). All of it scales with the rink.

**What I placed:** seven owners `Crowd_L2_Night_k0667` to `k1556` of class `BP_ArenaCrowd` (the same Blueprint: same switch, same cheer, same rule that only the loaded rink's owner stays), each with the three stand meshes and the twelve figures as instanced components, tags `B_Arena` and `B_Fan`. No gate, loft, machine, props, door, firewood or bench (they need a wall). No dressing actor.

**The layout is clay's, with three rules for the night rig's own pieces:**
- **Banks:** the stand line is 4.0 m beyond the boards, or 0.6 m beyond the bank if that is further (the banks grow with the rink, the stands do not): on the large and wide arena the line moves out by 0.3 and 1.0 m.
- **Low fills:** no module stands where a rect fill points within its reach. That takes 4 modules behind the boxes and 6 behind the benches on most rinks (8 on Pond and the 70 m rink): **a gap in the crowd behind each.** The alternative (stands in a 450 cd fill at 3.7 m) is what your rule forbids.
- **Towers and mounds:** on rinks under 90 m they stand inside the ring a corner stand would take, so those four layouts have **no corner stands**.

**The night materials** (new instances of compiled masters; the masters are untouched; bound as overrides on the night rig's components, so the clay rig and the mesh assets are unchanged):

| Instance (all `Arena/`) | Parent | Values |
|---|---|---|
| `MI_NightCrowd_Fan` | `M_ClayCrowd` | Tint (0.30, 0.36, 0.50), Sat 0.35, FillK 0.35 |
| `MI_NightCrowd_Skin` | `M_ClayCrowd` | Tint (0.34, 0.33, 0.36), Sat 0.5, FillK 0.45 |
| `MI_NightCrowd_Scarf` | `M_ClayCrowd` | Tint (0.45, 0.50, 0.60), Sat 0.3, FillK 0.4 |
| `MI_NightCrowd_Tin` | `M_ClayCrowd` | Tint (0.20, 0.22, 0.26), FillK 0.3 |
| `MI_NightBarn_Stand` | `Arena/M_ArenaWood` | WoodDark (0.012, 0.016, 0.024), WoodLight (0.040, 0.050, 0.070), WashK 1.2, wash (0.45, 0.60, 1.0) |
| `MI_NightBarn_StandDark` | `Arena/M_ArenaWood` | WoodDark (0.008, 0.010, 0.016), WoodLight (0.022, 0.028, 0.040), WashK 1.2 |

The crowd's fill is a cool (0.55, 0.70, 1.0). Both parents carry the instanced-mesh flag. **These numbers are guesses for "dark, cool, low value"; nobody has seen them.** The crowd's components do not render custom depth, so by the night tier's own rule they get no ramp and no ink; that too is for the eye (film script, step 0).

## 2. The roof and the walls on night: left out

A timber roof and plank walls would stand inside the night tier's own dark room and under its truss and towers, two buildings in one place, and the tier's voice is a dark open hall with snow at the boards. I placed none of them, no bunting, no lamp arms (the night lamps hang from the truss).

What belongs there instead, as I read the tier: the ring of dark stands now placed; **low lamp posts along the stand fronts** (a dim cool pool on the snow between bank and stand, so the crowd's feet and a few hats are lit from below the game camera's frame) in place of fills; and at the ends, where the towers are, **a rail of snow-covered boards** instead of corner stands on the small rinks. Neither is built.

## 3. Proof

Each rink's own night match, headless; bounds from world transforms; the night rig's pieces and device A's from their own actors:

| Rink | Owner alive | Fans = instances | Stand pieces | Left out | Stand front, side / end (m) | Bounds in the clear zones | Meeting device A's pieces | Meeting a bank, mound, tower or the room's walls (6 + 4 + 96 + 4 pieces) | Nearest bank to a stand | Stands / fans in a low fill's reach |
|---|---|---:|---:|---|---|---:|---:|---:|---:|---|
| Pond 60 x 30 | `Crowd_L2_Night_k0667` | 208 | 24 | 8 modules, 4 corners | 19.0 / 34.0 | 0 | 0 | 0 | 2.13 m | 0 / 0 |
| Regular 90 x 45 | `Crowd_L2_Night_k1000` | 554 | 46 | 10 modules | 26.5 / 49.0 | 0 | 0 | 0 | 1.20 m | 0 / 0 |
| Large 120 x 60 | `Crowd_L2_Night_k1333` | 734 | 66 | 10 modules | 34.3 / 64.3 | 0 | 0 | 0 | 0.60 m | 0 / 0 |
| Wide 140 x 70 | `Crowd_L2_Night_k1556` | 861 | 78 | 10 modules | 40.0 / 75.0 | 0 | 0 | 0 | 0.60 m | 0 / 0 |

- Fan scale 0.96 to 1.04 and mean fan height 1.40 m on all four. The 64, 70 and 80 m layouts are built (194, 241, 322 fans) and were not run.
- **The two end fills do reach the end stands:** on the regular rink 83 and 72 fans are inside their cones, the nearest 9.9 m from the lamp (Pond 16 and 18 at 6.3 m; large 128 and 119 at 13.1 m; wide 147 and 144 at 15.9 m). They point down and in over the stands, not at them, and are unshadowed; I left the stands under them. How lit the end terraces are is for the eye.
- **The switch** (regular rink): as loaded 12 of 12 fan components visible (554), stands 3 of 3 (46); `ke * ArenaCrowd 1`: fans 0 of 12; `0`: owner hidden; `2`: all back. There is nothing tagged `B_ArenaDress` on night.
- **The cheer:** a `goal` at +x: `CrowdCheerPlusX` 0.125, 0.5, 1 at frames 1, 4, 8, held to 88, 0.5 at 98, 0 at 108; 287 fans of the +x half rise, 0 of the other. At -x the same for `CrowdCheerMinusX` and 267 fans.
- **The rig loads clean:** 0 warnings naming B's arena in 6 night sessions.
- **Size:** `LR_L2_Night.umap` 429,073 → 1,346,393 bytes; 169 → 176 actors (6 of the 7 new ones remove themselves at load, as on clay).
- **The blocking-load line: not available here.** The other device's `blocking load N ms` is written only with a renderer. On clay, on the GPU, the same seven-owner scheme read 39 to 52 ms after the first session (AR10).

## 5. The wall pieces at real size on clay (a separate state)

On the big rinks the gate, its props, the ice machine, the sliding door, the firewood and the bench stood at the barn's scale (1.56 times life size on the wide arena). In this state they are **at scale 1 on every rink**, their backs on the barn's wall wherever that wall is; the loft stays at the barn's scale (it spans between the wall's posts).

| Rink | Gate, props, firewood, bench | Ice machine | Sliding door | Loft | Their bounds meeting a stand / in the clear zones |
|---|---|---|---|---|---|
| Pond | scale 1.0 | left out (it would stand in the corner stand) | left out (its hoist reaches over the end stand's back) | none (as before) | 0 / 0 |
| Regular | 1.0 | 1.0 | 1.0 | barn scale 1.0 | 0 / 0 |
| Large | 1.0 | 1.0 | 1.0 | barn scale 1.333 | 0 / 0 |
| Wide | 1.0 | 1.0 | 1.0 | barn scale 1.556 | 0 / 0 |

- Fans and stands unchanged (664 / 56 on the regular rink).
- **Where it is:** `Game/Saved/AR11/LR_P3_Clay.job5.umap` (1,604,244 bytes; a second copy in `scratchpad\ar11\bk`). It is job 2's rig with these six components re-placed. To try it: with the editor closed, copy it over `Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap`. **The tree's clay rig is the committed job 2 state** (`git status` shows it unchanged).
- On a small rink the real-size pieces are larger than before (Pond: 1.5 times), which is why two are left out there.

## Not verified

All of the look:
- The night crowd and stands on screen: value, colour, whether silhouettes with a few lit hats, whether brighter than the boards' top, ink or no ink.
- `M_ArenaWood` under the night tier (it was built for the clay barn; its wash on night is a guess).
- Readability on night with the crowd in: ice, puck, jerseys.
- The gaps behind the boxes and benches; the small rinks without corner stands; the end terraces under the end fills.
- The cheer on night, with the red goal light.
- Frame time, the blocking-load line, a cook.
- The 64, 70 and 80 m rinks on night.
- Job 5: any of it on screen (the real-size gate in a wall bay 1.56 times its width on the wide arena may look small).

## For the other device

1. Nothing for the 10:00 package.
2. When the night stands are merged: the night rig's file triples (1.35 MB) and loads seven owners, six of which remove themselves. Please time a night match with and without (`ke * ArenaCrowd 0` after the match has started), as you did on clay, and read me your `blocking load` line for night.
3. The stands on night keep clear of your benches, boxes, table and goal lights by the same zones as on clay (0 bounds meeting them on four rinks). If the three low fills (B41, your ask) may be shortened or turned, the ten modules can come back: say.

## Decisions for the operator

1. **Gaps or fills:** ten modules are missing where the three low fills point. Keep the gaps, or shorten the fills' reach from 7 m to about 3 m and fill the rows.
2. **Small rinks on night:** no corner stands (towers and mounds). Keep, or move the towers out on rinks under 90 m.
3. **Ink:** the crowd without outlines (now), or in the night tier's ink.
4. **Real-size wall pieces on clay** (job 5): adopt after a look, or keep the barn-scale pieces.
5. **Lamp posts** in place of darkness between banks and stands on night: wanted or not.

## The next three evolutions, each with a proving number

1. **The GPU pass on night** (`status/AR11-film.md`): ice within 1 %, puck and jerseys within 3, the crowd band's 99th percentile under the boards' top and its mean at most two fifths of it, 0 crowd samples more saturated than the jerseys.
2. **Low lamp posts for the night crowd** (a mesh from the Blender lane, lit by material, no light actor): the lit share of the frame under 2 % at the game camera, ice mean within 0.5 %, 0 bounds in the clear zones or on a bank on seven rink lengths.
3. **The builder made cheap** (job 3's single owner with batched adds), then used on both tiers: the rig's blocking load within 20 ms of the baked rig's on four rinks, both rig files under 500,000 bytes, 0 actors destroyed at load.

## Files

Job 1 to 4 (in the tree, uncommitted):
- `Game/Plugins/JamAssets/Content/Rigs/LR_L2_Night.umap` (429,073 → 1,346,393 bytes; a copy of the file before in `scratchpad\ar11\bk`)
- `Game/Plugins/JamAssets/Content/Arena/MI_NightCrowd_Fan.uasset`, `MI_NightCrowd_Skin.uasset`, `MI_NightCrowd_Scarf.uasset`, `MI_NightCrowd_Tin.uasset`, `MI_NightBarn_Stand.uasset`, `MI_NightBarn_StandDark.uasset`
- `status/AR11-film.md`, `status/AR11-report.md`

Job 5 (apart, not in the tree's content): `Game/Saved/AR11/LR_P3_Clay.job5.umap`

## Ledger rows (11 columns)

Jobs 1 to 4: the night rig's row (replaces the row of the same path) and six new rows:

```
| Game/Plugins/JamAssets/Content/Rigs/LR_L2_Night.umap | other | 1.35 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | L2 night rig as before; sky light a movable real-time capture of the capture dome SkyCap_L2_Night (lane B34); side snow banks split into four pieces (Snow_L2_Night_Bank_3, _3b, _4, _4b on SM_Con_SnowBank_Side) leaving device A's penalty boxes, scorer's table and benches clear on every rink size; three shadowless rect fills (Fill_L2_Night_Box, Fill_L2_Night_Bench_1, _2: 450 cd, 7 m, facing away from the rink above the board line) for the pieces behind the boards (lane B41); since AR11 the stands and the crowd of the clay tier on seven actors Crowd_L2_Night_k0667 to k1556 (class BP_ArenaCrowd, one layout in real size per rink length, only the loaded rink one stays), with night instances as component overrides (MI_NightCrowd_*, MI_NightBarn_*), tags B_Arena and B_Fan; the stand line clear of the snow banks, no modules before the three low fills, no corner pieces under 90 m (towers and mounds stand there); no roof, walls or props; no light added, no shadows, no collision; look unseen (lane AR11) (1346393 bytes) |
| Game/Plugins/JamAssets/Content/Arena/MI_NightCrowd_Fan.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | night instance of M_ClayCrowd for coats, hats and mittens: Tint (0.30, 0.36, 0.50), Sat 0.35, FillK 0.35, a cool fill (0.55, 0.70, 1.0); bound as an override on the night rig components only; values set without a picture (lane AR11) (6068 bytes) |
| Game/Plugins/JamAssets/Content/Arena/MI_NightCrowd_Skin.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | night instance of M_ClayCrowd for faces: Tint (0.34, 0.33, 0.36), Sat 0.5, FillK 0.45, cool fill; bound as an override on the night rig components only; values set without a picture (lane AR11) (6074 bytes) |
| Game/Plugins/JamAssets/Content/Arena/MI_NightCrowd_Scarf.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | night instance of M_ClayCrowd for scarves: Tint (0.45, 0.50, 0.60), Sat 0.3, FillK 0.4, cool fill; bound as an override on the night rig components only; values set without a picture (lane AR11) (6080 bytes) |
| Game/Plugins/JamAssets/Content/Arena/MI_NightCrowd_Tin.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | night instance of M_ClayCrowd for tin parts: Tint (0.20, 0.22, 0.26), FillK 0.3, cool fill; bound as an override on the night rig components only; values set without a picture (lane AR11) (6068 bytes) |
| Game/Plugins/JamAssets/Content/Arena/MI_NightBarn_Stand.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | night instance of Arena/M_ArenaWood for the stands planks: WoodDark (0.012, 0.016, 0.024), WoodLight (0.040, 0.050, 0.070), WashK 1.2, a cool wash (0.45, 0.60, 1.0); bound as an override on the night rig components only; values set without a picture (lane AR11) (5991 bytes) |
| Game/Plugins/JamAssets/Content/Arena/MI_NightBarn_StandDark.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | night instance of Arena/M_ArenaWood for the stands posts and risers: WoodDark (0.008, 0.010, 0.016), WoodLight (0.022, 0.028, 0.040), WashK 1.2, cool wash; bound as an override on the night rig components only; values set without a picture (lane AR11) (6015 bytes) |
```

Job 5, only if it is adopted (replaces the clay rig's row as job 2 left it):

```
| Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap | other | 1.60 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | P3 clay rig, barn with calm ice: PPV (EV100 4.2, SSR override, focus 16 m f/4, no depth blur, motion blur 0), sky light a movable real-time capture (no cubemap; intensity 0.35 and colour unchanged) of the capture dome SkyCap_P3_Clay (engine sphere, scale 700, MI_SkyAmbient_Clay, outside the barn; B34), 4 shadowed warm key spots (5200 cd, no specular, shadow resolution 1.0 and source radius 60 cm since B34), 8 shadowless lamps over the side lanes (2100 cd, broad beams, no specular; B28), 6 shadowless fills at lower intensity (B28), 18 lamp shades (6 of them decorative beyond the end boards, twice as wide since B29), LookParams (FeltRim 1.4); plank walls, posts, trusses, roof, floor; no bulb strings; all shadowless meshes, no collision; the old ten trusses replaced by ten roof bays (SM_Barn_TrussBay, height scale 0.82), 32 wall bays in front of the plank walls and 12 lengths of bunting hung at 13.05 m, all instances on one actor Dress_P3_Clay that scales with the rig; timber stands and a clay crowd as instances on seven actors Crowd_P3_Clay_k0667, k0711, k0778, k0889, k1000, k1333, k1556 (class BP_ArenaCrowd, one layout in real size per rink length 60, 64, 70, 80, 90, 120, 140 m, each scaled by 90 / length so that only the one whose rink is loaded has scale 1 and stays; the others destroy themselves at BeginPlay): on the regular rink 56 stand pieces and 664 fans (twelve figures with a far level, per-instance cheer weight, coat value, accent value, end and delay, front row empty, corner rows three quarters full), the ice gate, its props, a loft of two bays and the ice machine at the +x end wall; no light added, no shadows, no collision (lanes B9, B14, B18, B19, B22, B23, B27, B28, B29, B34, AR2, AR5); job 2 of AR8: 18 lamp arms on Dress_P3_Clay (6 SM_Barn_LampArm_075, 12 SM_Barn_LampArm_225, placed from the rig real lamp shades, each on its lamp rod) and a sliding door, a firewood stack and a skate bench on the -x end wall on each of the seven crowd owners (lane AR8 job 2); job 5 of AR11 (a separate state, only if adopted): the gate, its props, the ice machine, the sliding door, the firewood and the bench at real size on every rink with their backs on the barn wall (the loft stays at the barn scale); on the 60 m and 64 m rinks the machine and the door are left out (lane AR11 job 5) (1604244 bytes) |
```
