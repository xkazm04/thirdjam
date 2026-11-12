# B13 report: the arena without props (night sky, ice, construction, lighting), device B

Lane B13, device B, 2026-11-12, machine clock 15:50 to 16:45. Main-tree editor on port 8000 (pid 29616). A second editor (port 8002) shared the GPU, so I took no frame numbers.
No git command, no build. `Rink` was never saved, nothing under `/Game/` was written, nothing is under `_Scratch/`. The editor is left on `/Game/Maps/Rink` with no dirty package and PIE stopped.
Step A's checkpoint is `status/B13-checkpoint.md`.

## 0. The real rink ran

PIE on `/Game/Maps/Rink` builds the real game:
- `JAMCART ... rink=90x45 visual=toy tier=l2_night`
- `JAMRINK built 90x45 m corner 3.75 goal 4 m`
- `JAMLOOK tier=l2_night map=/JamAssets/Rigs/LR_L2_Night.LR_L2_Night scale=1.000 (rink 90.0 m) trim_emissive=30.0`
- `JAMLOOK rig loaded: N local lights scaled (intensity x 1.000, attenuation x 1.000)`. N was 12 before step A and is 6 now: the 4 spots and 2 fills.

**Scale printed: 1.000.** Every still is from the real game in PIE plus `CaptureGameViewport`, with the HUD widgets removed and `slomo 0.0001` to freeze the faceoff.

## 1. Step A: props out

**`LR_L2_Night`: 180 actors removed.**
- The Rodin set: `SM_FilmLight` 8 plus their 8 lens discs, `SM_DirectorChair` 5, `SM_Slate` 4, `SM_CameraDolly` 4, `SM_CameraCrane` 2, `SM_FlightCases` 12, `SM_TrussLamps` 2.
- Batch 2: bench 2, coach stool 2, water crate 2, penalty box 1, goal light 2, scoreboard housing 1, plus its 2 wires.
- Track 38, cube cases 65, cables 10, exit signs 2.
- `Practical_L2_Night_1..8`.

What was kept: `PPV` 1, `Sky` 1, `Spot` 4, `Room` 6. The truss is now 30 bare-structure `Con_` actors:
- beams, cross pieces and hangers;
- per spot, one lamp can built from basic shapes (`Con_L2_Night_LampCan_n`, `_LampLens_n`, `_LampYoke_n_a/_b`).

All of them are shadowless.

**`LR_P3_Clay`** held no props (7 light/post/params actors). It is unchanged and was not re-saved.

**Effect on the ice: the practicals did not light it.** Paired real-game crops changed by at most 0.7 %. The only larger change, -2.6 %, is in a crop that held the end set's reflection.

Stills: `20261112-155815-B-b13-a-empty-wide24`, `-a-empty-game`, `-a-empty-over`; before: `20261112-155816-B-b13-a-before-over`, `-a-before-game`.

## 2. Subject 1: night sky or dark volume

**Master `Look/M_NightSky`** is new: unlit, two-sided, opaque, no texture, stencil 0, so it gets no ramp and no ink. One Custom node does two modes:
- `SkyMode` 0, the dark volume: a cool gradient by surface height that fades to black by `GradHeight` 14 m, with a faint column rhythm every 12 m. The ceiling stays black.
- `SkyMode` 1, the open sky: a horizon-to-zenith gradient, a dim ground band below the horizon, and a procedural star field. The stars come from a hash per direction cell (`StarScale` 300 cells per radian) and are 2-3 px points. They depend on view direction only, so they do not swim with the geometry.

Instances:
- `MI_NightSky_DarkVolume`: `WallGlow` 0.35, `ColStrength` 0.4.
- `MI_NightSky_Stars`: `StarScale` 300, `StarSize` 0.3, `StarDensity` 0.05, `StarBright` 40.

**How the lighting is kept.** `Room_L2_Night_2..6` (the walls and ceiling) are now invisible, with `affect_indirect_lighting_while_hidden` True, so Lumen still treats the room as closed and the sky light still does not reach the ice. Measured: ice crops in variants A and B are within 1 % of the old room (pool 162.9 / 161.8 / 162.1, side band 106.1 / 106.9 / 106.7, graze band 38.9 / 38.8 / 38.8). `render_in_main_pass` False was tried first and failed: the walls still wrote depth and the sky stayed black.

**Variant A, dark volume** (hidden in the saved rig): `SkyA_L2_Night_Wall_1..4` and `SkyA_L2_Night_Ceiling`, cube shells 5 cm inside the old room faces.

**Variant B, open night sky** (**saved as the default**):
- `SkyB_L2_Night_Dome`: an engine sphere at scale 600, so 300 m radius.
- `SkyB_L2_Night_Ground_1..4`: planes at z -3 cm outside the room footprint. They hide A's under-ice floor, which the sky light lights once it is outside the walls; that floor showed as a blue ledge in the first try.

All Sky actors are shadowless, have `affect_dynamic_indirect_lighting` False, stencil 0 and no collision.

**Checks:**
- Stars: not outlined and not stepped (stencil 0).
- Screen-space reflections: no sky or wall line appears in the ice in any still. The boards hide the horizon from every in-rink camera.
- Game camera: the sky sits above the far boards, and the far net reads against the boards.

**Verdict: B is the better picture at the same lighting and readability, so B is saved.** A is the safe fallback.

**The switch.** Open `/JamAssets/Rigs/LR_L2_Night` as the editor world and run this one line in editor Python (it shows A and hides B; swap `SkyA_` and `SkyB_` to go back):

```
[a.set_actor_hidden_in_game(not a.get_actor_label().startswith('SkyA_')) for a in unreal.EditorLevelLibrary.get_all_level_actors() if a.get_actor_label()[:5] in ('SkyA_','SkyB_')]; unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
```

**Also changed:** `Room_L2_Night_1` (the rig floor) moved from z -5 to -3 cm. Reason: A's floor lowering does not work (see section 9), so A's floor still sits at -5 cm and z-fought the rig floor.

Stills:
- `20261112-161203-B-b13-sky-before-over`, `-sky-a-over`, `-sky-b-over`
- `20261112-161204-B-b13-sky-a-wide`, `-sky-b-wide`, `-sky-a-game`, `-sky-b-game`
- `20261112-161205-B-b13-sky-a-up`, `-sky-b-up`

## 3. Subject 2: ice

**Added to `M_Ice`.** Every existing parameter and default is kept. With the new defaults the old picture is reproduced exactly.

| New parameter | Default | `MI_Ice_Night` |
|---|---:|---:|
| `ScuffZones` (scuff denser in the slots and along the boards, thinner in open ice) | 0 | 1.0 |
| `ScratchAmount` (arc-shaped skate scratches) | 0 | 0.8 |
| `FrostBand` | 0 | 1.0 |
| `FrostWidthCm` | 140 | 160 |
| `FrostColour` | 0.86, 0.91, 0.96 | 1, 1, 1 |
| `IceRoughMin` (replaces the constant 0.28) | 0.28 | 0.34 |
| `CornerRadiusCm` | 375 | not set |

Unchanged: `Scuff` 0.7, `Wet` 0, `IceAmbient` 0.9.

How the graph changed:
- New Custom node `FrostMask`: a rounded-rectangle distance to the boards built from `RinkHalfLength`, `RinkHalfWidth` and `CornerRadiusCm`, broken up by value noise.
- `ScuffMask` gained the zone mask and the scratches. The scratches are arcs around jittered centres, one per 7 m cell. Their lanes fade by screen-space derivative, so they do not alias at 24 m.
- `IceColour+Lines` lerps to the frost colour before the lines are painted.
- `Roughness`: frost goes matte (0.85).

Verdict:
- The markings stay crisp at the game camera, and nothing aliases at 24 m.
- The scratches show faintly in the dark bands and slots. The frost band is subtle because the ramp quantises it.
- The spot specular is softer (`IceRoughMin` 0.34), and nothing reads as a mirror disc.
- The puck stays black against the scuff, and no banding was added in the dark bands.

Rejected:
- Scratches whose direction came from noise: the derivative of the noise made them vanish entirely.
- A rougher `MI_Toon_Dark`: no effect; reverted. The file was re-saved with the same values, 6,995 → 7,118 bytes.

Stills:
- `20261112-161837-B-b13-ice-before-game` / `-ice-after-game`, `-ice-before-close`
- `20261112-161838-B-b13-ice-after-close`, `-ice-before-graze` / `-ice-after-graze`, `-ice-before-slot`
- `20261112-161839-B-b13-ice-after-slot`, `-ice-after-wide24`

## 4. Subject 3: construction

From the PIE actor listing:
- `Boards` is `MI_Toon_Board` (z 0 to 120 cm).
- `Nets` are `MI_Toon_NetFrame` and `MI_Toon_NetMesh`.
- `Glass` is **`/Game/Placeholders/M_Net`**: A's asset, translucent and lit, z 120 to 210 cm. It reads as a beige band.
- The trim strips sit at z 120 to 130 cm.

**`M_Toon`: new Custom node `Construct`**, between `TeamColour` and its three consumers. With every new parameter at its default 0, the output equals `TeamColour`, so the figures are unaffected. New parameters, group `Construct`:
- `KickHeightCm` 0, `KickColour` (0.08, 0.09, 0.12)
- `RailTopCm` 120, `RailBandCm` 0, `RailColour` (0.30, 0.33, 0.40)
- `GridCm` 0, `GridLine` 0.12, `GridHole` 0.35

Instances changed:
- `MI_Toon_Board`: `KickHeightCm` 0 → 22 (a dark kick plate), `RailBandCm` 0 → 10 (a darker top-rail band under the trim).
- `MI_Toon_NetMesh`: `GridCm` 0 → 10, giving a net mesh of light strings and dark holes. It fades to the average at distance, so it does not alias.
- No name or parameter that A sets was changed.

**New `Look/M_Glass` and `MI_Glass`** (A has to switch the material; I applied it in PIE only):
- unlit translucent, Fresnel, opacity 0.06 plus Fresnel, and a lit top edge at `GlassTopCm` 210;
- vertical only, so nothing is sorted over the ice.
- Other defaults: `GlassTint` (0.55, 0.75, 1.0), `GlassGlow` 0.25, `FresnelGlow` 2, `EdgeCm` 4, `EdgeGlow` 4.

**`Con_` actors added to the rig.** All are engine cubes, shadowless, stencil 1, no collision, outside the boards:
- 66 glass stanchions `Con_L2_Night_Stanchion_n`, 7 cm, z 1.2 to 2.15 m, every 4 m along the long sides, at |y| 6, 10, 14 and 18 m on the end boards (none behind the nets), and 2 per corner arc. Material `MI_Dress_Steel`, folder `Con/GlassLine`.
- 4 lattice towers `Con_L2_Night_Tower_n_*` at (±52, ±30) m, 28 m tall: 4 legs plus 20 braces each, `MI_Dress_Case`.
- 2 end headers `Con_L2_Night_Header_1..2` at x ±52, z 27 m.
- The truss beams were lengthened (x scale 100 → 104.5) to meet the headers.

Neither net is blocked from the game camera: the towers show only at the top corners, and the headers are above the frame.

Stills:
- `20261112-162426-B-b13-con-before-net` / `-con-after-net`
- `20261112-162427-B-b13-con-before-board` / `-con-after-board`, `-con-after-over`, `-con-after-game`
- `20261112-162428-B-b13-con-after-own`, `-con-after-far`

**Verdict:** the boards, net and glass line read as built hockey construction. The truss now stands on towers instead of floating.

## 5. Subject 4: lighting

**Luminance profiles** come from a top view with the figures hidden (full range, 9 × 9 px means, every 3 m).

| Line | x (m) | Luma |
|---|---|---:|
| y -7 (through the pools) | pool plateaus | 212-221 |
| y -7 | gaps at ±21 to ±24 | 50-62 |
| y -7 | ±3 | 98-106 |
| y -7 | centre red line | 8 |
| y -15 (the dark band) | the whole length | 43-53 |
| y -20 (along the board) | most of it | 74-90 (the trims tint it through Lumen) |
| y -20 | blue lines | 38 |

**Where it is hard to read:**
- Blue figures in the dark band: blue on dark-blue ice. They read only through their emissive fill and the ink outline.
- The puck at 24 m in the dark band: 4-5 px.

**End-board ice restored.** Two new spot lights, `Fill_L2_Night_1..2`:
- at (±50, 0, 12) m, aimed at (±42, 0, 0), pitch -56.3;
- 700 cd, attenuation 26 m, cones 25 / 75 degrees, colour (1.0, 0.96, 0.9);
- shadowless, movable.

Measured against no fill, same cameras:

| Area | Change |
|---|---:|
| end-board corner ice | +11.5 %, +11.4 % (own end); +9.0 %, +9.4 % (far end) |
| ice behind the nets | +4 to +6 % |
| pools | unchanged |

That puts back lane B11's -7 to -12 %. At 600 cd and above, one patch at (±39, ±15) steps up one ramp band (46 → 79 luma). It reads as light spill at the ends.

**Rim on the figures: rejected, and the rim lights were removed.**
- Four side spots at 1.8 m, 3000 cd, diffuse: lit the dark-band ice by +60 to +180 % near the boards.
- Specular-only (`diffuse_scale` 0, 8000 cd): still +45 % on the ice near the lights.
- On lighting channel 1 only: the ice is untouched, but they do nothing unless A puts the skater meshes on channel 1 too. With that done in PIE, 6000 cd was barely visible on the emissive-filled toon figures.
- The material rim works better. `RimStrength` 1.5 on the skater MIDs (A sets 0.6) gives a visibly lighter cyan edge on the blue figure in the dark band. That is A's value; see section 9.

**Puck in the darkest band.** Puck and two figures were moved to y -15 while paused.

| Distance | Halo on | Halo off |
|---|---|---|
| 10 m | ring up to 249 against 51 | black (0) against 49 |
| 24 m | from a 3.5 m camera height the halo flattens to a 2 px bright line and covers the puck's dark top (window min 39) | 4-5 px black against 50 |

Both read.

**Atmosphere variant (stills only, not saved; the veto window runs until 21:00):**
- haze post 0.0003 / 0.5 (start 1500, colour 0.22, 0.30, 0.55, absorb 0.3);
- lane B2c VER b shafts at 0.3 (inside fade 0.6) on the four spots;
- built as transient materials in `/Engine/Transient` and applied to PIE actors only. Nothing was written.

It reads well under the open sky. The game camera sits in the first shaft's fade zone, so there is no veil, and the ice is unchanged.

Stills:
- `20261112-163703-B-b13-atmo-variant-over`, `-atmo-variant-wide`, `-atmo-variant-game`
- `20261112-163704-B-b13-atmo-off-wide`
- `20261112-163704-B-b13-light-darkband-puck-10m-halo` / `-10m-nohalo` / `-24m-halo`
- `20261112-163705-B-b13-light-darkband-puck-24m-nohalo`

Final saved state:
- `20261112-163900-B-b13-final-game`, `-final-wide24`, `-final-close6`
- `20261112-163901-B-b13-final-own`, `-final-far`, `-final-over`, `-final-variantA-over`
- `20261112-163902-B-b13-final-variantA-game`, `-final-withglass-game`

## 6. The rig as saved

`LR_L2_Night`, 218 actors:

| Prefix | Count |
|---|---:|
| `PPV` | 1 |
| `Sky` | 1 |
| `Spot` | 4 (unchanged) |
| `Room` | 6 |
| `Con` | 194 |
| `SkyA` | 5 (hidden) |
| `SkyB` | 5 |
| `Fill` | 2 |

- Static-mesh shadow casters: **0**.
- Local lights: 4 shadowed spots (unchanged) and 2 shadowless fills.
- No translucency over the ice.
- Meshes: 188 cubes, 16 cylinders, 5 planes, 1 sphere, about 12k triangles (was 238.8k).

**GPU-hang hardening:** every rule is kept. No new shadow caster, and the spots are untouched.

## 7. Assets created or changed (bytes on disk, `Game/Plugins/JamAssets/Content/`)

| File | Before | After |
|---|---:|---:|
| `Rigs/LR_L2_Night.umap` | 468,713 | 568,857 |
| `Look/M_Ice.uasset` | 37,335 | 45,547 |
| `Look/MI_Ice_Night.uasset` | 9,852 | 12,588 |
| `Look/M_Toon.uasset` | 31,139 | 38,181 |
| `Look/MI_Toon_Board.uasset` | 8,595 | 10,298 |
| `Look/MI_Toon_NetMesh.uasset` | 8,402 | 15,252 |
| `Look/MI_Toon_Dark.uasset` (re-saved, same values) | 6,995 | 7,118 |
| `Look/M_NightSky.uasset` | new | 20,965 |
| `Look/MI_NightSky_DarkVolume.uasset` | new | 7,994 |
| `Look/MI_NightSky_Stars.uasset` | new | 8,968 |
| `Look/M_Glass.uasset` | new | 19,728 |
| `Look/MI_Glass.uasset` | new | 9,205 |

Ledger rows. The first replaces the current `LR_L2_Night` row. `M_Ice`, `M_Toon`, `MI_Ice_Night`, `MI_Toon_Board` and `MI_Toon_NetMesh` replace their existing rows (notes only; sizes stay 0.0).

```
| Game/Plugins/JamAssets/Content/Rigs/LR_L2_Night.umap | other | 0.6 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | night rig: PPV, sky, 4 spots, invisible room kept for Lumen, open night sky (SkyB) and hidden dark-volume variant (SkyA), bare truss on 4 towers, lamp cans, 66 glass stanchions, 2 shadowless end fills; props unplaced since H8.5 (lanes B1, B1c, B11, B13) |
| Game/Plugins/JamAssets/Content/Look/M_NightSky.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | unlit sky master: dark-volume gradient (SkyMode 0) or procedural star dome and horizon glow (SkyMode 1), no texture (lane B13) |
| Game/Plugins/JamAssets/Content/Look/MI_NightSky_DarkVolume.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance, sky variant A (lane B13) |
| Game/Plugins/JamAssets/Content/Look/MI_NightSky_Stars.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance, sky variant B (lane B13) |
| Game/Plugins/JamAssets/Content/Look/M_Glass.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | unlit translucent rink glass with Fresnel and lit top edge (lane B13) |
| Game/Plugins/JamAssets/Content/Look/MI_Glass.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance for A's Glass component (lane B13) |
| Game/Plugins/JamAssets/Content/Look/M_Ice.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | master material (painted-cel look) + scuff zones, arc scratches, frost band, IceRoughMin (lane B13) |
| Game/Plugins/JamAssets/Content/Look/M_Toon.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | master material (painted-cel look) + Construct: kick plate, rail band, net grid, all off by default (lane B13) |
| Game/Plugins/JamAssets/Content/Look/MI_Ice_Night.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance (lane B13 surface values) |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Board.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance, kick plate 22 cm and rail band (lane B13) |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_NetMesh.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-12 | B | y | instance, 10 cm net grid (lane B13) |
```

Per A-to-B-0007, append the note "unplaced since H8.5, returns with the final art style" to these rows (shipped stays `y`):
- the seven Rodin props: `SM_FilmLight`, `SM_DirectorChair`, `SM_Slate`, `SM_CameraDolly`, `SM_CameraCrane`, `SM_FlightCases`, `SM_TrussLamps`;
- the six batch-2 pieces;
- `MI_Trim_FilmLens`.

## 8. For the operator: six stills to look at

1. `20261112-163900-B-b13-final-game`: the shipped look from the game camera.
2. `20261112-163901-B-b13-final-over`: the whole arena under the open sky.
3. `20261112-163901-B-b13-final-variantA-over`: the dark-volume alternative (the sky choice).
4. `20261112-163902-B-b13-final-withglass-game`: with A's glass switched to `MI_Glass`.
5. `20261112-163703-B-b13-atmo-variant-wide`: the haze and shafts variant (veto until 21:00).
6. `20261112-161839-B-b13-ice-after-slot`: the ice in the slot (scratches, frost, scuff).

## 9. What I could not verify, and what others must decide or know

**What I could not verify:**
- Frame cost: two editors were up.
- The 4090.
- The large and wide cuts with the new `Con_`, `Sky` and `Fill` actors.
- Stars and scratches in motion (TSR); I have stills only.
- The haze's order against the ramp and outline.
- `M_Toon` `Construct` under the clay tier (the boards would get the kick plate there too).
- TB1 and TB2 figures: they use their own masters, so they are unaffected.

**The lead decides:**
1. Sky: B (saved) or A (the one-line switch above). This is the operator's choice.
2. Atmosphere: yes or no by 21:00.
3. Send A the glass switch (item 2 below).
4. Rim approach: A's `RimStrength`, or a lighting-channel rim.
5. Paste the ledger rows.

**Device A must know:**
1. **`LookTierSwitcher::LowerFloor` does not move the floor.** The log says `JAMLOOK floor StaticMeshActor_0 lowered by 15.0 cm to a top of -20.0 cm`, but the same frame warns `Mobility of ...StaticMeshActor_0 : StaticMeshComponent0 has to be 'Movable' if you'd like to move`, and its bounds still read z -5 cm. Set the floor Movable first, or hide it when a rig loads. I raised the rig floor to -3 cm as a workaround.
2. **Glass:** switch the `Glass` component from `/Game/Placeholders/M_Net` to `/JamAssets/Look/MI_Glass`. `GlassTopCm` 210 matches today's glass top; if the glass height changes, set `GlassTopCm` on a MID.
3. **Uniform rig scaling and the glass line.** The `Con_L2_Night_Stanchion_*` stand on the 1.2 m board. Under the uniform rig scale s they grow to 2.15·s m (2.87 m at 1.333) while A's glass stays at 2.1 m. Either scale the `Con/GlassLine` folder by (s, s, 1), or I move them into A's spawn.
4. **Rim.** For a light rim that leaves the ice alone, the skater meshes need lighting channel 1 (rim lights on channel 1 only). The cheap alternative is `RimStrength` 1.2-1.5 instead of 0.6 on the skater MIDs.
5. **`M_Ice` `CornerRadiusCm`** (default 375 = today's 3.75 m corner) shapes the frost band. Drive it from the cartridge's corner radius on larger rinks if their corners differ.
6. The puck halo flattens into a 2 px line at 24 m from game-camera height. Lane B3 or A could thicken or tilt it.
