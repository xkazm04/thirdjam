# B15 report: device A's measured look applied to the night rig, one construction source (device B)

Lane B15, device B, 2026-11-12. Machine clock 17:40 to 18:25.

Setup:
- Editor on port 8002, tree `wt/b-vfx` (dev-b + main 547699c), pid 3808. It exited through `ExitEditor` with no dirty package and PIE stopped.
- No git command, no build. `/Game/Maps/Rink` and `/Game/` were never saved. Nothing is under `_Scratch/`.
- Not touched: `Skaters/`, `Props/`, `VFX/`, `Audio/`, `LR_P3_Clay`, `LR_P4_Broadcast`.
- Cvars restored: `jam.ScriptHuman` 0, `jam.Cut` hockey.regular, `jam.rink.construction` 1.
- The editor on port 8000 shared the GPU, so I took **no frame numbers**.
- The step 0 checkpoint is `status/B15-checkpoint.md`.

## Method

I reproduced A23's frozen frame:
1. `jam.Cut hockey.regular` (seed 1) and `jam.ScriptHuman 1`.
2. PIE floating, warm-up 0, then `slomo 0.05`.
3. I polled the game time and set `slomo 0.0001` at t = 2.17. Frozen t was 2.176 to 2.198 across sessions.
4. HUD widgets removed.
5. Camera 10 / 24 / 40 m behind centre ice along the view ray: (-9.53, 0, 3.04), (-22.87, 0, 7.30), (-38.11, 0, 12.16) m. Pitch -17.7, yaw +x, FOV 70.
6. Capture: `CaptureGameViewport` at 1920 x 1085. A23's was 2560 x 1354, which is a slightly wider frame.

**Framing check:** my framing matches A23's `base2-b-*` (centre circle 44 % of the frame width against 48 %). The `proof-*` circle looks smaller because A's markings draw a 4.5 m centre circle there, while B's ice paints 9 m.

**`jam.` commands used:**
- `jam.Cut`, `jam.ScriptHuman`, `jam.rink.construction`.
- Present but not used: `jam.playcut`, `jam.rink.beacon`.

**Luma:** sRGB Rec.709 mean over fixed crops.

## Per step

### Step 0: construction, one source. PASS

**(a) Instances A's code loads.** Code reference: `RinkActor.cpp` `LoadLook` / `LoadConstructionMat`. The fallback order is `/JamAssets/Look/<name>`, then `/Game/LookLab/MI_LL_*` as a lifted MID, then a tinted basic material.

| Name | Used for | Fallback before | Now |
|---|---|---|---|
| `MI_Toon_KickPlate` | kick plate, bench seats | `MI_LL_KickPlate` MID | **created**: M_Toon, TeamColour (0.85, 0.72, 0.12), Emissive 0.8, Roughness 0.5, Rim 0 |
| `MI_Toon_GlassRail` | rail and stanchions | `MI_LL_GlassRail` | **created**: (0.05, 0.05, 0.06), Emissive 0, Roughness 0.4, Rim 0 |
| `MI_Toon_Glass` | `Glass` | `/Game/Placeholders/M_Net` | **created**: child of `MI_Glass` |
| `MI_Toon_Bench` | bench walls | `MI_LL_KickPlate` MID tinted | **created**: (0.62, 0.60, 0.56), Emissive 1.0 |
| `MI_Toon_GoalLight` | goal-light housing | `MI_LL_GlassRail` MID tinted | **created**: (0.45, 0.45, 0.48), Emissive 1.2 |
| `MI_Ice_Night`, `MI_Toon_Board`, `MI_Toon_NetFrame`, `MI_Toon_NetMesh`, `MI_Trim_Blue/Orange`, `MI_Trim_Beacon`, `MI_Toon_Bumper`, `MI_Toon_GoalShield`, `MI_Zone_*` | | | existed |
| `MI_Trim_BeaconOpaque` | beacon mode 2 only (default 0 = off) | `MI_LL_NetFrame` MID | not created: a fixed instance cannot take the per-game beacon colour |

The PIE listing on `hockey.regular` now shows no `/Game/LookLab/` material on the rink.

**(b) Duplicates removed.**

*The board paint.* `MI_Toon_Board` kick plate and rail band are off **under the cel tier only**:
- Setting `KickHeightCm` / `RailBandCm` to 0 would have removed B14's broadcast kick and rail paint, because `BcastToonBase` reads the same parameters.
- So `M_Toon` gained `CelConstructOff` (default 0) and a `CelConstructGate` node after `Construct`. With the parameter at 1 and Clay 0 the base is the plain `TeamColour`.
- `MI_Toon_Board` `CelConstructOff` 0 → 1. `KickHeightCm` 22 and `RailBandCm` 10 stay, and the feature stays in the master.

*The stanchions.* The 66 `Con_L2_Night_Stanchion_*` are deleted from the rig (218 → 152 actors).

*The net grid* is **kept: it is already single-source.** A's netting (`Nets` section 1) is drawn with B's `MI_Toon_NetMesh`, so the grid is the material on A's geometry. A draws none of its own (`s0-netclose`).

**(c) Glass.** A's `Glass` component asks for `MI_Toon_Glass`. Creating that name closes B-to-A-0023 point 2 on B's side, so no PIE swap was needed.

**Doubling check (PIE actor listing):**
- `hockey.regular`: one kick plate, one rail, A's 73 stanchions.
- `hockey.arena.large` (log `scale=1.333`): A's 98 stanchions, rig `Con_` 128 (was 194).
- Towers, headers and fills stand outside A's 120 x 60 boards. Nothing is doubled.

**Verdict:** A25's construction reads at 10 and 24 m: an ochre plate, a dark rail and thin posts, all on B's instances. B13's lighter 7 cm steel posts showed as a second comb, and they are gone.

Stills:

| Mine | Before |
|---|---|
| `b15-s0-board` | `b15-before-board` |
| `b15-s0-netclose` | none |

### Step 1: sky dome. PASS

**Values changed on `MI_NightSky_Stars`:**

| Parameter | Old | New |
|---|---|---|
| `HorizonColour` | (0.10, 0.16, 0.35) | (0.05, 0.09, 0.22) |
| `ZenithColour` | (0.004, 0.007, 0.020) | (0.004, 0.006, 0.018) |
| `SkyGlow` | 1.0 | 0.5 (A23's Brightness) |
| `GradSpread` | 0.35 | 1.0 |

`GradSpread` 1.0 matches A23's height curve: `M_LL_Sky` is t = (z / R)^0.6, and B's is sqrt(e / GradSpread).

**Rig:** `Room_L2_Night_2..6` `affect_indirect_lighting_while_hidden` and `affect_distance_field_lighting` True → False. The walls still exist, hidden and now inert.

This lighting change is what lightens the dark ice. With only the colours changed, the dark band stays at 48 luma and the sky turns near black (2.4). B13 had kept the hidden walls in Lumen, which is exactly what keeps the sky light off the ice.

**Dark-band luma at 24 m** (same crop in all four):

| Image | Dark band |
|---|---:|
| before | 48.2 |
| colours only | 48.2 |
| colours + walls out | **79.9** |
| A23 `proof-faceoff-24m` | 81.8 (`base2-b` 63.6) |

**Other measurements:** pools unchanged (217.8). Board face (lit by Lumen) 64.6 → 81.5.

**Stars:** they stay. They are clean 1-3 px points at 10, 24 and 40 m, never outlined or stepped (stencil 0).

**Verdict:** this is the largest move toward the proof. The void colour, gap ice and board face all match A23's levels.

**Rejected:**
- A23's colours at B13's spread 0.35: a black sky.
- A23's `CloudStrength` 0.35: not built.
- The band under the horizon (the SkyB ground planes) stays near black, where A23 had the room floor.

Stills: `b15-s1-sky-{10,24,40}` next to `proof-faceoff-{10,24,40}m`.

### Step 2: spots. PASS

**Change:** `Spot_L2_Night_1..4` inner cone 19 → 5 degrees. Outer stays 26.5. Intensity, attenuation and shadow settings unchanged, so the B1c hardening is intact.

**Pool centres are unchanged:**
- 24 m: 219.5 → 219.5.
- Top view, 5 m off-centre: 220.3 → 220.2.
- Both are within 0.1 %, well inside 5 %.

**Pool shoulders now fall off over a wider ring:** -12 to -40 % at 9-12 m from a pool centre. This is A23's "soft pools with wider painted rings".

**Dark bands:**
- Top view: gap at x = 0 is 46.2 → 46.2, gap at x = 22.5 is 87.0 → 87.0.
- In the 24 m camera crop: 91.9 → 80.9, because that crop holds pool fringe.

**End-board ice restored by B13's fills: unchanged** (top view 124.6 to 129.9, +0.1 %). The fills are outside every spot's cone change.

Stills: `b15-s2-spots-{10,24,40}` next to `proof-faceoff-*`.

### Step 3: ice. PASS

**`M_Ice`, new nodes** (group `Night (B15)`). Each returns its input when its switch is 0 or when |`MPC_Look.Clay`| > 0.5.

*`PaintTileScuff`*, after `ClayScuffGate` and before every consumer of the mask:
- A23's streak tile: simplex 0.06, 3 levels turbulent on world position x 0.10 (10:1 along the rink), plus fine noise 0.03, posterised to 3 levels.
- Output = lerp(M, max(M · `TileBaseKeep`, s · `TileScuffAmount`), `TileScuff`).

*`PaintTileFrost`*, after `FrostMask`:
- Frost patches: simplex 0.006, 3 levels, posterised to 2 levels.
- Output = lerp(F, max(F, fr · `TileFrostAmount`), `TileFrost`).

*`NightWetGate`*, in front of `Roughness`:
- Output = max(`Wet`, `NightWet`), cel tier only.
- **A's `RinkActor` overwrites `Wet` on the ice MID** (0, or 1 from the cartridge's `wet_ice`). An instance `Wet` 0.3 would never reach the game, so `NightWet` is the floor.

**Parameters and defaults:** `TileScuff` 0, `TileFrost` 0, `NightWet` 0, `TilePosterize` 3, `TileScuffBias` 0.45, `TileScuffAmount` 0.7, `TileBaseKeep` 1, `TileFrostBias` 0.58, `TileFrostAmount` 0.5.

**`MI_Ice_Night` values:**

| Parameter | Old | New |
|---|---|---|
| `TileScuff` | 0 | 1 |
| `TileFrost` | 0 | 1 |
| `TileBaseKeep` | 1 | **0.3** |
| `TileFrostAmount` | 0.5 | **0.35** |
| `NightWet` | 0 | 0.3 |

`IceAmbient` 0.9 is kept, so the dark-ice floor stays. The two remaining dark-band crops read 83.2 / 83.4.

**Which features fought:**
- **B13's blotchy base scuff noise against the streaks.** At full strength, B13's blotches read at 24 m and the streaks did not. I kept the streaks (A23's look; `s3-ice-24` reads like `proof-faceoff-24m`) and turned B13's whole scuff mask down to 30 % (`TileBaseKeep` 0.3). That includes the zones and scratches, which are still visible at 10 m.
- **The frost patches against B13's frost band.** With B13's `FrostColour` (1, 1, 1), the patches were white blotches at 0.5, so they are at 0.35. B13's frost band along the boards stays at full strength.

**`NightWet` 0.3:** almost invisible through the ramp (gap +1.7 %, everything else ±0.1).

Stills: `b15-s3-ice-{10,24,40}` next to `proof-faceoff-*`.

### Step 4: outline. PASS

**`M_PP_Outline`:** the old `Outline` node is replaced by `OutlineB15`, which carries every old input plus:

| Parameter | Default |
|---|---|
| `WidthMode` | 0 |
| `WidthNear` | 3 |
| `WidthFar` | 1.5 |
| `NearCm` | 1000 |
| `FarCm` | 4000 |
| `ColourMode` | 1 (the old darkened base) |
| `InkColour` | (0.012, 0.012, 0.028) |
| `InnerLines` | 1 |
| `PuckBoost` | 1 |
| `MPC_Look.Clay` | input |

The new terms act only when Clay is 0. At the defaults, the 24 m still differs from the old node by mean 0.57 levels, which is the frozen-frame noise between two PIE sessions.

**`MI_PP_Outline` values:** `WidthMode` 0 → 1, `ColourMode` 1 → 0 (ink), `InnerLines` 1.

**Stencil classes** (checked at 10, 24 and 40 m and in the game camera):
- Figures, boards, nets and A's construction (stencil 1) are inked.
- Ice, markings, trims, glass, haze and the star dome (stencil 0/2) are not.
- The puck's halo still reads at the game camera.

**Verdict:**
- At 10 m the figure gets a drawn ink edge. At 40 m it no longer goes black.
- **Side effect:** at 3 px the thin toy stick is inked solid black at 10 m (it was pale wood). It still reads as a stick, but its colour is lost near the camera.

Stills: `b15-s4-outline-{10,24,40}` next to `proof-faceoff-*`.

### Step 5: figure fill. PASS (instances); the toy is A's value

**Instances:** `MI_Toon_Blue` Emissive 2.5 → 1.5, and `MI_Toon_Orange` 2.0 → 1.5.

**The game does not use these instances for the toy.** In PIE every toy MID (body `M_Toon`, `MI_Toon_Helmet`, `MI_Toon_Stick`) carries **Emissive 2.2**, from `UToySkaterVisualComponent::BaseEmissive`, set at build and on every `SetBoost(None)`. The instance default does not matter for the toy.

**Stills:** I set 1.5 on the PIE MIDs for the stills only.

| Figure (24 m, game frame) | Luma at 2.2 | Luma at 1.5 |
|---|---:|---:|
| Blue in the pool | 79.5 | 78.4 |
| Blue in the dark band | 65.2 | 51.3 |
| Orange in the ring | 102.7 | 64.6 (reads red-brown) |

At 24 m both teams stay distinct, but orange loses its punch. No ramp band appears on the capsule at either fill.

**Value for the lead:** the other lane's four B figures should get fill 1.5 to match (I did not touch them).

Stills: `b15-s5-fill-{10,24,40}` next to `proof-faceoff-*`.

### Step 6: haze. PASS (saved hidden)

**New material `Look/M_HazeCard`:** unlit, additive, two-sided. A23's mode-0 term (dense at the ice, gone at `TopZ`) plus **`SideFade`** 0.25.

`SideFade` is my addition. Without it the 70 m card had hard vertical edges and read as a lit screen behind the far net.

**Instance `MI_HazeCard_End`:** Strength 0.8, TopZ 1800, EdgeSoft 0, HazeColour (0.75, 0.80, 1.0).

**Rig:** `Haze_L2_Night_1..2`, engine planes 70 x 18 m at x = ±62 m, z 0-18 m, folder `Haze`.
- Shadowless, no collision, no custom depth, no distance-field or indirect lighting.
- **Saved hidden in game.**
- No shaft cones and nothing at the sides.
- On the large cut they scale to x = ±82.7 m.

**Measured with the cards shown:** the far board face goes 54.7 → 115.6 and the band above it 0.7 → 41. Ice and figures are unchanged.

**The switch** (open `/JamAssets/Rigs/LR_L2_Night` as the editor world, run in editor Python; `True` hides them again):

```
[a.set_actor_hidden_in_game(False) for a in unreal.EditorLevelLibrary.get_all_level_actors() if a.get_actor_label().startswith('Haze_L2_Night_')]; unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
```

Stills: `b15-s6-haze-24`, `b15-final-{10,24,40,game}` (shown), `b15-final-nohaze-*` (as saved).

## Tier stability (clay and broadcast)

**Method:** one game-camera still of each tier before step 1 and after step 6, plus a fixed 24 m still.
- Construction off (`jam.rink.construction 0`) and A's glass hidden, so only the masters are compared.
- Clay through a scratch cartridge copy (`p3_clay`; Clay read 1).
- Broadcast through B14's rig swap (Clay read -1).

| Still | Mean \|d\| | Tile max | Frame luma |
|---|---:|---:|---|
| clay, 24 m fixed camera | 0.44 | 3.1 | 175.12 → 175.11 |
| broadcast, 24 m fixed camera | 0.39 | 6.6 | 133.43 → 133.44 |
| clay, game camera | 2.16 | 5.7 | 192.47 → 192.42 |
| broadcast, game camera | 3.15 | 10.3 | 170.54 → 170.37 |

**The game-camera stills are over 1 level, but this is motion, not a leak.** The difference map shows only doubled edges: lines, figure outlines and dent-texture edges. The follow camera moved between the two frozen frames (t 2.183 against 2.195). There is no area change: open ice in broadcast is 0.72, and frame luma is within 0.2.

The fixed-camera stills are inside B14's noise floor (0.5). **No leak.**

Every new path is gated:
- `CelConstructGate`, `PaintTileScuff`, `PaintTileFrost` and `NightWetGate` return their input at |Clay| > 0.5.
- `OutlineB15` uses the old path when Clay is not 0.

Stills: `b15-tier-{clay,bcast}-{before,after}`.

**Not master output, but it does change those tiers:** A's construction components and its glass now get B's new instances on every tier, because A's code has one name per part. Under clay and broadcast the kick plate, rail, benches, goal lights and glass show `MI_Toon_*` through `M_Toon`'s clay/broadcast paths, and `MI_Toon_Glass` instead of `M_Net`. B14 wanted `MI_Bcast_Glass` under broadcast.

## What changed

**Rig `LR_L2_Night`:** 218 → 154 actors.
- Removed: `Con_L2_Night_Stanchion_*` 66.
- Added: `Haze_L2_Night_` 2 (hidden).
- Changed: `Spot_L2_Night_1..4` inner cone 19 → 5. `Room_L2_Night_2..6` affect-lighting-while-hidden and distance-field lighting True → False.
- Unchanged: `PPV`, `Sky`, `Fill`, `SkyA`, `SkyB`, the other 128 `Con_`.
- Static-mesh shadow casters: still 0.

**Assets (bytes on disk, `Game/Plugins/JamAssets/Content/`):**

| File | Before | After |
|---|---:|---:|
| `Rigs/LR_L2_Night.umap` | 568,857 | 398,745 |
| `Look/M_Toon.uasset` | 52,257 | 53,109 |
| `Look/M_Ice.uasset` | 61,137 | 72,494 |
| `Look/M_PP_Outline.uasset` | 14,761 | 21,731 |
| `Look/MI_Toon_Board.uasset` | 14,514 | 14,144 |
| `Look/MI_Ice_Night.uasset` | 16,697 | 19,110 |
| `Look/MI_NightSky_Stars.uasset` | 8,968 | 9,341 |
| `Look/MI_PP_Outline.uasset` | 4,832 | 6,727 |
| `Look/MI_Toon_Blue.uasset` | 9,063 | 9,339 |
| `Look/MI_Toon_Orange.uasset` | 9,124 | 9,329 |
| `Look/MI_Toon_KickPlate.uasset` | new | 8,981 |
| `Look/MI_Toon_GlassRail.uasset` | new | 8,927 |
| `Look/MI_Toon_Glass.uasset` | new | 7,025 |
| `Look/MI_Toon_Bench.uasset` | new | 8,653 |
| `Look/MI_Toon_GoalLight.uasset` | new | 8,635 |
| `Look/M_HazeCard.uasset` | new | 17,474 |
| `Look/MI_HazeCard_End.uasset` | new | 7,971 |

## Ledger rows

**Rows to replace** (match by path):
- The `LR_L2_Night` row.
- The notes of `M_Toon`, `M_Ice`, `M_PP_Outline`, `MI_Toon_Board`, `MI_Ice_Night`, `MI_NightSky_Stars`, `MI_PP_Outline`, `MI_Toon_Blue`.

**Add:** `MI_Toon_Orange` (if no row exists) and the seven new files. **No rows to remove.**

```
| Game/Plugins/JamAssets/Content/Rigs/LR_L2_Night.umap | other | 0.4 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | night rig: PPV, sky, 4 spots (inner cone 5), hidden room walls out of Lumen, open night sky (SkyB, A23 gradient) and hidden dark-volume variant (SkyA), bare truss on 4 towers, lamp cans, 2 shadowless end fills, 2 end haze cards saved hidden; stanchions now A's RinkActor; props unplaced since H8.5 (lanes B1, B1c, B11, B13, B15) |
| Game/Plugins/JamAssets/Content/Look/M_Toon.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | master material (painted-cel look) + Construct (B13), clay pass 2 / broadcast mode (B14), CelConstructGate (cel tier only, B15) |
| Game/Plugins/JamAssets/Content/Look/M_Ice.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | master material (painted-cel look) + scuff zones, scratches, frost band (B13), clay pass 2 / broadcast mode (B14), streak and frost tiles and NightWet (cel tier only, A23 lab, B15) |
| Game/Plugins/JamAssets/Content/Look/M_PP_Outline.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | post-process master (painted-cel look) + width by distance, ink colour, inner-line switch (cel tier only, A23 lab, B15) |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Board.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance; painted kick plate and rail band off under the cel tier (A's RinkActor draws them), kept for clay and broadcast (B13, B14, B15) |
| Game/Plugins/JamAssets/Content/Look/MI_Ice_Night.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance (B13 surface, B14 clay/broadcast, B15 tiles, TileBaseKeep 0.3, NightWet 0.3) |
| Game/Plugins/JamAssets/Content/Look/MI_NightSky_Stars.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance, sky variant B, A23 gradient (lanes B13, B15) |
| Game/Plugins/JamAssets/Content/Look/MI_PP_Outline.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance, outline 3 to 1.5 px by distance, ink (B15) |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Blue.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance, Emissive 1.5 (B15) |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Orange.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance, Emissive 1.5 (B15) |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_KickPlate.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance of M_Toon, ochre kick plate for A's RinkActor (lane B15) |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_GlassRail.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance of M_Toon, near-black glass rail and stanchions for A's RinkActor (lane B15) |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Glass.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | child of MI_Glass, the name A's Glass component loads (lane B15) |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Bench.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance of M_Toon, bench walls for A's RinkActor (lane B15) |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_GoalLight.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance of M_Toon, goal-light housing for A's RinkActor (lane B15) |
| Game/Plugins/JamAssets/Content/Look/M_HazeCard.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | unlit additive haze card master: height falloff, side fade, no texture (lane B15, from A23's M_LL_Haze) |
| Game/Plugins/JamAssets/Content/Look/MI_HazeCard_End.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance, end haze cards Strength 0.8 (lane B15) |
```

## Six stills for the operator (`devlog/shots/`)

1. `20261112-181946-B-b15-final-24`: everything applied, haze shown. Next to `docs/experiments/img/look-lab/proof-faceoff-24m.jpg`.
2. `20261112-181945-B-b15-before-24`: the same frame before.
3. `20261112-181949-B-b15-final-nohaze-game`: the game camera as saved.
4. `20261112-181950-B-b15-final-board`: the one-source construction (kick plate, rail, stanchions, glass).
5. `20261112-181949-B-b15-s6-haze-24`: the haze cards, for the 21:00 decision.
6. `20261112-181946-B-b15-s5-fill-24`: fill 1.5 (orange darkens). Compare with `-s4-outline-24` at 2.2.

## Expected to cost more than A23's lab

No frame numbers were taken on B.
- **Ice:** `M_Ice` now runs B13's scuff zones, arc scratches, frost band and B14's clay/broadcast gates **plus** A23's three noise nodes (3 + 2 + 3 simplex levels) over the full-screen ice. The lab's `M_LL_Ice2` had only the tiles. That is more ALU than A23 measured.
- **`NightWet` 0.3:** turns on Lumen reflections on the ice (A23: up to +0.9 ms at 0.6, less at 0.3), for an almost invisible gain behind the ramp.
- **Lighting:** B13's two fills, which the lab did not have (shadowless).
- **The star dome:** a hash per pixel over the sky.
- **Haze cards:** large additive translucent quads at the far end (hidden for now).
- **Unchanged against B13:** the sky light now reaches the ice through the hidden walls (no new pass), the outline (same as the lab's), and the construction (A measured under 0.05 ms).

## What device A must know

1. **Instance names.** `MI_Toon_KickPlate`, `MI_Toon_GlassRail`, `MI_Toon_Glass`, `MI_Toon_Bench` and `MI_Toon_GoalLight` now exist, so `/Game/LookLab/` is no longer needed by the rink.
   - **The glass is B's `MI_Toon_Glass` on every tier.** If broadcast wants `MI_Bcast_Glass`, the code has to choose by tier.
   - `MI_Trim_BeaconOpaque` was not created.
2. **`Wet`.** `RinkActor` sets `Wet` on the ice MID from `wet_ice` (0 or 1), which overrides the instance. B's night wetness is the new `NightWet` floor. If `wet_ice` should mean "a little wet", set 0.3 there instead of 1.
3. **Figure fill.** The toy figure's MIDs carry `BaseEmissive` 2.2, so `MI_Toon_Blue/Orange` 1.5 has no effect on them. A23's 1.5 needs `UToySkaterVisualComponent::BaseEmissive` = 1.5. At 1.5 the orange team reads red-brown in the ring zone (-37 % luma at 24 m); 1.8 is a middle way.
4. **Construction next to the rig.**
   - The glass's lit top edge (`GlassTopCm` 210) sits just under A's dark rail at 2.1 m, so the rail reads as a dark line over a light line. Fine at 24 m; at 10 m it reads as a double line.
   - The goal-light housing stands on the rail behind each net and reads as a small floating box at 10 m (`s0-netclose`).
   - The netting's sloped top panel shows almost no grid, because the grid picks the dominant axis.
5. **Large cut (scale 1.333).**
   - Rig `Con_` towers and headers stand outside A's 120 x 60 boards. The end haze cards scale to x = ±82.7 m, still outside.
   - The last 14 m at each end are still outside the pools (B1c).
   - Nothing is doubled.
6. **Stick at 10 m.** The 3 px near outline inks the thin toy stick solid black at 10 m.

## What I could not verify

- Frame cost: the 8000 editor shared the GPU.
- The 4090.
- Motion and TSR shimmer of the streak tiles and stars: stills only.
- `hockey.arena.wide`.
- The haze cards' sorting against the post ramp and outline in motion.
- The B figures' fill (another lane's).
- Clay and broadcast with construction **on** under the new instances: I compared masters with construction off and the glass hidden.

## What the lead must decide

1. **Haze cards after 21:00:** on (the one-line switch above) or keep hidden.
2. **Figure fill:** whether A sets `BaseEmissive` 1.5 (A23) or about 1.8, given the orange team's loss.
3. **Ice scuff balance:** whether B13's scuff zones and scratches at 30 % (`TileBaseKeep` 0.3) are acceptable. Raise toward 0.6 if the slots should read more scuffed.
4. **Glass per tier** (a request to A); and whether to delete the now-inert `Room_L2_Night_2..6`.
5. **Sky gradient:** `GradSpread` 1.0 is my reading of A23's dome curve. B13's 0.35 with A23's colours is near black.
6. Paste the ledger rows.
