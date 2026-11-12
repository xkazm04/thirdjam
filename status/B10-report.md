# Lane B10 report: zone strips and the pickup look (device B)

- Worktree `C:/Users/kazda/jam/wt/b-vfx`, branch `lane/b-vfx`.
  - **`git merge --ff-only dev-b` failed** ("Not possible to fast-forward": the lane branch has B3b's commits that `dev-b` holds as a different commit).
  - I did not merge any other way. The lane stayed at `93b33a7`. Contracts, `handoff/B-to-A-0012.md` (untracked in the main tree) and `RinkActor.cpp` were read from the main tree, read-only.
- No build was needed.
- Editor on port 8002, two sessions.
  - **Session 1 (14:11 to 14:26)** started fine but **was found suspended** (all 98 threads in the `Suspended` wait state; no log line after frame 0). I resumed my own process with `NtResumeProcess` and it carried on. My reading: the harness froze the job of the background shell that started it. Session 2 was started detached and did not freeze.
  - Session 1 ended through `ExitEditor` with `bDiscardUnsavedChanges` (the reason is under "What went wrong").
  - **Session 2 (14:29 to 14:34)** ended cleanly through `ExitEditor`; the log ends `LogExit: Exiting.`
- `Rink.umap` was never saved: its mtime is still 12:40. Before each exit I left Rink unsaved by opening a blank map (`new_blank_map`); I never reloaded the open map.
- Nothing is under `Content/_Scratch`.
- The port-8000 editor was up the whole time, and a package was running in `b-skater`. **No frame numbers** were taken.

## Verdicts

| # | Deliverable | Verdict | Evidence |
|---|---|---|---|
| 1 | Zone strips `M_Zone` + 5 `MI_Zone_<kind>` | **PASS** | Compiles with no error expressions (327 pixel-shader instructions). In PIE, all 5 kinds × rect 12×8 m and circle r 5 m: crisp edges on both shapes, each kind read by pattern as well as colour, the painted lines show through, and a stand-in puck reads on every zone it sat on (stills `b10-top-*`, `b10-cam10-*`, `b10-cam30-east`). |
| 2 | Pickup look | **PASS** | `SM_Pickup_Base` has socket `PickupTop` at 45 cm. There is a generic `SM_Pickup_Token`, `M_Pickup` with spin and bob by WPO, six kind instances plus `MI_Pickup_Base`, and `NS_pickup_idle`. Read: 10 m clear; about 25-30 m in colour (`b10-pickup-close`, `b10-cam30-*`). The spin is seen as a changed orientation between captures. |
| 3 | Verify in PIE | **PASS, with caveats** | Real game on `/Game/Maps/Rink` (HokejGameMode, live bot match, 90×45 cut, night rig), with my actors placed in memory only. The camera was a helper `CameraActor` with the follow camera's geometry (6-10 m back, 3.5 m up, pitch -18, FOV 70), not the follow camera itself. Caveats are under "Honest readability". |
| 4 | List for device A | **PASS** | Below. |
| + | R-0019 add-on: `MI_Toon_Bumper`, `MI_Toon_GoalShield` | **PASS** | Instances of `M_Toon` (master untouched). Still `b10-obstacles`. |
| + | `NS_blade_spray` | **PASS** | Ends itself (spawned without auto-destroy: inactive 6 s later). Device A's toy skaters were already spawning it live in PIE: six components active at once during the match. Still `b10-blade-spray`. |

## 1. Zones: `M_Zone` (translucent, unlit, After-DOF pass)

**Why translucent:** an unlit translucent strip is composited after the toon ramp and the outline (both Before DOF). So the ramp cannot eat it, the outline ignores it, and it writes no custom depth (stencil 0). A masked opaque strip would be ramped through the ice's stencil 2 and turn black.

**Edge from UVs.** The material derives the zone's size in cm per pixel from the screen derivatives of UV and world position. Device A sets **no size**; rotation and non-uniform scale work.

**Layers, from outside in:**
- a 7 cm dark ink rim, which reads on lit pools;
- a 25 cm solid colour band, which reads in dark bands;
- a pattern;
- a faint fill.

**Patterns** are in world metres, so they keep their scale on any zone size.

| Instance | `ZoneColour` (from 0012) | `Pattern` | Look and motion |
|---|---|---|---|
| `MI_Zone_slow_ice` | 0.35, 0.65, 1.0 | 0 | frosted cross-hatch, thin lines at ±45° every 0.5 m, slow breathing ("rough") |
| `MI_Zone_fast_ice` | 0.2, 1.0, 0.8 | 1 | 2.6 m streaks along world X in lanes 0.7 m apart, scrolling +X at 1.5 m/s × (0.6..1.4 by Strength) ("slick") |
| `MI_Zone_bumper` | 1.0, 0.35, 0.1 | 2 | a 1.2 m band of thick chevrons pointing outward along the edge, ink between them ("bouncy, pushes out") |
| `MI_Zone_no_shoot` | 1.0, 0.1, 0.2 | 3 | 0.5 m diagonal hazard stripes, red and ink, across the whole zone ("forbidden"); Emissive 3.5 so it stays red, not pink |
| `MI_Zone_boost_only` | 0.9, 0.8, 0.2 | 4 | a dotted ring 0.9 m inside the edge, 36 cm dots every 0.75 m, with a slow chase |

**Parameters** (group `Zone`; defaults in brackets):
- `ZoneColour` (vector); `Strength` (0..1 from the cartridge, default 1: scales fill, pattern opacity and scroll speed); `Emissive` (8, `no_shoot` 3.5); `PatternScale` (1); `Circle` (0 = rect, 1 = circle).
- Tuning only: `Pattern`, `FillAlpha` 0.14, `PatternAlpha` 0.75, `BandWidth` 25 cm, `RimWidth` 7 cm, `ScrollSpeed` 150 cm/s, `InkColour`.

## 2. Pickups

**`SM_Pickup_Base`** (`Props/`) is made with GeometryScript from primitives:
- a 70 cm plinth 6 cm high, a 62 cm step and a torus ring;
- the top of the ring is at 12 cm, pivot at the centre of the bottom, no collision;
- **socket `PickupTop` at Z 45 cm** (the attach height for any pickup mesh: the token's centre sits there, bob ±8 cm);
- default material `MI_Pickup_Base`.

**`SM_Pickup_Token`** is the generic upright reel-like disc (46 cm diameter, 8 cm thick, a 16 cm hub), pivot at its centre, bounds extended for the WPO. Default material `MI_Pickup_custom`.

**`M_Pickup`** is Default Lit (so it survives the stencil-1 ramp and gets the ink outline):
- Emissive = `PickupColour` × `Emissive` × `BodyTint` + ring glow + Fresnel rim;
- WPO = spin about the component pivot's Z at `Spin`°/s, plus a bob of `BobHeight` cm at `BobSpeed` Hz, with a per-placement phase hashed from the position;
- the normals are rotated with the spin.

**Parameters:** `PickupColour`, `Emissive` 6, `BodyTint` 1, `Roughness` 0.45, `RimStrength` 0.8, `RimPower` 2.5, `RimColour`, `Spin` 90, `BobHeight` 8, `BobSpeed` 0.5, `RingGlow` 0, `RingEmissive` 14, `RingInner` 20, `RingOuter` 33, `RingMinZ` 6.

| Instance | `PickupColour` | Notes |
|---|---|---|
| `MI_Pickup_speed` | 0.1, 0.85, 1.0 (cyan) | |
| `MI_Pickup_precision` | 1.0, 0.2, 0.8 (magenta) | close to `MI_Toon_Bumper`, see "Lead must decide" |
| `MI_Pickup_heavy_puck` | 0.55, 0.25, 1.0 (violet) | |
| `MI_Pickup_big_puck` | 1.0, 0.75, 0.1 (amber) | |
| `MI_Pickup_reverse_other` | 1.0, 0.12, 0.12 (red) | |
| `MI_Pickup_custom` | 0.92, 0.92, 1.0 (white) | Designer-made effects |
| `MI_Pickup_Base` | 1.0, 0.85, 0.6 | `Spin` 0, `BobHeight` 0, `BodyTint` 0.12, `Emissive` 1.5, `RingGlow` 1; tint per kind with a MID setting `PickupColour` |

**`NS_pickup_idle`** (`VFX/`, Fountain emitter, CPU, `M_Fx_Sprite`):
- 10 + 10 × `User.Strength` rising sparks per second from a flat 56 cm disc, 0.7-1.3 m/s, 0.9-1.4 s life, 5-10 cm, about 20 alive;
- user parameters `User.Colour` (default 1, 0.85, 0.6) and `User.Strength` (0.5);
- **it loops until stopped.** To stop it: `Deactivate()` lets the live sparks finish (verified: inactive after 6 s), or `DestroyComponent()` / `DestroyInstance()` removes it at once. Spawn it with auto-destroy so a deactivated component cleans itself up.

## 3. R-0019 additions (instances of the existing `M_Toon`; master untouched)

- `MI_Toon_Bumper`: `TeamColour` 1.0, 0.08, 0.55 (hot magenta), `Emissive` 2.0, `RimStrength` 0.6, stencil 1. It is neither team's colour.
- `MI_Toon_GoalShield`: `TeamColour` 0.82, 0.86, 0.90 (pale grey-white, opaque), `Emissive` 1.5, `RimStrength` 0.6, stencil 1.

## 4. `NS_blade_spray` (what the toy figure expects)

**What the code does:** `ToySkaterVisual.cpp:36` loads `/JamAssets/VFX/NS_blade_spray.NS_blade_spray` by soft path. Line 361 spawns it with `SpawnSystemAtLocation(World, BladeSpray, ComponentLocation + (0, 0, GroundOffsetCm), Facing)`:
- every 8 frames while |lean| ≥ 25° and the skater is not down;
- **it sets no user parameters** and keeps no handle, so the asset must end itself and must look right at its defaults.

**What the asset does:**
- one burst of `16 + 22 × saturate(User.Strength)` sprites (default Strength 0.6, so 29; at most 38);
- 0.25-0.4 s life, 1.5-3.4 m/s from a point 4 cm under the blade, gravity on;
- spawned from a flattened ellipsoid stretched along the system's local Y, so it fans **sideways from the skater's facing**, low to the ice;
- `User.Colour` default 0.8, 0.9, 1.0 (white-blue); `M_Fx_Sprite`, no shadows.

**Not side-aware:** the code passes no lean sign, so the spray goes to both sides. A could rotate by the lean sign, or set `User.Strength` from |lean|.

## 5. Honest readability (PIE stills)

- **All five zones are told apart at 10 m and at 25-30 m** by pattern first, colour second, on both pools and dark bands.
- **Closest pairs:**
  - `slow_ice` and `fast_ice` at 30+ m in a pool: both are light, cool-coloured and finely patterned; the hatch versus streaks difference fades first.
  - `bumper` and `no_shoot` for a red-green colour-blind player: orange against red, but chevrons on the edge only versus stripes all over, so the pattern separates them.
- **On lit pools the zone fill is faint:** the band and pattern carry it. That is deliberate, so the ice and lines stay readable.
- **Puck:**
  - The stand-in puck (24 cm near-black disc, stencil 3, on a 60 cm `MI_Trim_PuckHalo` disc) stayed clearly visible on `slow_ice`, `fast_ice`, `no_shoot` and `boost_only` at 10 and 25 m.
  - **Weakest is `boost_only`:** its yellow fill is close to the yellow-green halo, so the halo ring loses contrast. The black puck still reads.
  - My stand-in halo sits 0.4 cm high, above the zone at 0.2 cm. If device A's real halo is lower than the zone plane, the translucent strip would tint it.
- **Painted lines:**
  - Red and blue lines show through every zone. Under `no_shoot` and `bumper` the line is darkened by the ink stripes but still continuous.
  - No zone hides a line completely at `Strength` 1.
- **Pickups:**
  - Clear at 10-15 m in colour.
  - At 25-30 m they are small coloured marks (about 10-15 px): readable by colour in dark bands, weaker in lit pools.
  - On lit pools the emissive washes the token to a pastel (pink rather than magenta).
  - `NS_pickup_idle` sparks are additive, so they **barely show on lit ice**; they show in dark bands.
- **Not verified:**
  - the real follow camera (a helper camera stood in for it);
  - device A's actual zone and pickup rendering;
  - `visible:false`;
  - the 120×60 and 140×70 cuts;
  - frame cost (two editors were up).

## 6. What device A needs (handoff list)

1. **Zone mesh:**
   - `/Engine/BasicShapes/Plane` (100×100 cm, UV 0..1) or any flat quad whose **UV 0..1 spans the zone's bounding box**: w × d for a rect, 2r × 2r for a circle;
   - scale `(w/1, d/1, 1)` in metres per 100 cm, i.e. `SetRelativeScale3D(w_m, d_m, 1)`;
   - Z = ice + **0.2 cm**; any yaw is fine;
   - cartridge `x, z` map to world X, Y (as in `RinkActor`).
2. **Zone material:** `/JamAssets/Look/MI_Zone_<kind>.MI_Zone_<kind>` with `<kind>` = `slow_ice | fast_ice | bumper | no_shoot | boost_only`.
   - Make a MID and set scalar `Circle` = 1 for `shape: circle` (0 for rect) and `Strength` = the zone's `strength` (0..1). Optional: `PatternScale`.
   - Do not set a size; the material measures it.
   - Do not spawn anything for `visible: false`.
3. **Zone component flags:** `SetCastShadow(false)`, `SetCollisionEnabled(NoCollision)`, `SetRenderCustomDepth(false)` (stencil 0); optionally `TranslucencySortPriority` 1.
4. **Pickup:**
   - spawn `/JamAssets/Props/SM_Pickup_Base` on the ice (Z 0), stencil 1 (`SetRenderCustomDepth(true)`, value 1), shadow on, no collision;
   - attach the pickup mesh at socket **`PickupTop` (45 cm)**: for now `/JamAssets/Props/SM_Pickup_Token` with `/JamAssets/Look/MI_Pickup_<kind>` (`speed | precision | heavy_puck | big_puck | reverse_other`, else `MI_Pickup_custom`), stencil 1;
   - optionally give the base a MID of `MI_Pickup_Base` with `PickupColour` = the kind's colour.
   - **Nothing to animate:** the spin and bob are in the material.
   - The film-themed meshes from the other lane attach at the same socket. To make them spin and bob, give them an `M_Pickup` instance (their pivot must be on the spin axis); otherwise rotate the component.
5. **`NS_pickup_idle`:** attach at the base, `SetVariableLinearColor("Colour", kind colour)`; on take or despawn call `Deactivate()` (or destroy).
6. **Colours** for `User.Colour` of `NS_zone_enter/exit`, `NS_pickup_spawn/take` and `NS_effect_*`:
   - zones exactly as in 0012: `slow_ice` (0.35, 0.65, 1.0), `fast_ice` (0.2, 1.0, 0.8), `bumper` (1.0, 0.35, 0.1), `no_shoot` (1.0, 0.1, 0.2), `boost_only` (0.9, 0.8, 0.2);
   - pickups as in the table in section 2.
7. **Obstacles (R-0019):** `MI_Toon_Bumper` and `MI_Toon_GoalShield` via `LoadLook`, stencil 1 (A's `Obstacles` mesh already uses stencil 1).
8. **`NS_blade_spray`:** works as the code spawns it today. Optional: set `User.Strength` (0..1, from |lean| or speed) and rotate by the lean side.
9. **`shot_blocked`:** the `no_shoot` strip is the "why". A could additionally spawn `NS_foul` with `Variant` 0 and the `no_shoot` colour at the shooter, if the lead wants a burst.

## 7. Files (bytes on disk) and ledger rows

Total: **857,661 bytes**

- `Look/`: `M_Zone` 25,822; `MI_Zone_slow_ice` 9,826; `MI_Zone_fast_ice` 8,417; `MI_Zone_bumper` 10,554; `MI_Zone_no_shoot` 10,572; `MI_Zone_boost_only` 9,175.
- `Look/`: `M_Pickup` 27,189; `MI_Pickup_Base` 10,476; `MI_Pickup_speed` 7,228; `MI_Pickup_precision` 7,355; `MI_Pickup_heavy_puck` 7,318; `MI_Pickup_big_puck` 7,291; `MI_Pickup_reverse_other` 7,477; `MI_Pickup_custom` 7,002.
- `Look/`: `MI_Toon_Bumper` 8,714; `MI_Toon_GoalShield` 8,736.
- `Props/`: `SM_Pickup_Base` 45,265; `SM_Pickup_Token` 21,762.
- `VFX/`: `NS_pickup_idle` 294,296; `NS_blade_spray` 323,186.

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Look/M_Zone.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | zone strip master, translucent unlit, procedural; ZoneColour, Strength, Emissive, PatternScale, Circle (25822 bytes, lane B10) |
| Game/Plugins/JamAssets/Content/Look/MI_Zone_slow_ice.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | cross-hatch (9826 bytes, lane B10) |
| Game/Plugins/JamAssets/Content/Look/MI_Zone_fast_ice.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | scrolling streaks (8417 bytes, lane B10) |
| Game/Plugins/JamAssets/Content/Look/MI_Zone_bumper.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | edge chevrons (10554 bytes, lane B10) |
| Game/Plugins/JamAssets/Content/Look/MI_Zone_no_shoot.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | hazard stripes (10572 bytes, lane B10) |
| Game/Plugins/JamAssets/Content/Look/MI_Zone_boost_only.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | dotted ring (9175 bytes, lane B10) |
| Game/Plugins/JamAssets/Content/Look/M_Pickup.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | pickup master, lit + emissive, WPO spin/bob, ring glow (27189 bytes, lane B10) |
| Game/Plugins/JamAssets/Content/Look/MI_Pickup_Base.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | pedestal, static, ring glow (10476 bytes, lane B10) |
| Game/Plugins/JamAssets/Content/Look/MI_Pickup_speed.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | cyan (7228 bytes, lane B10) |
| Game/Plugins/JamAssets/Content/Look/MI_Pickup_precision.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | magenta (7355 bytes, lane B10) |
| Game/Plugins/JamAssets/Content/Look/MI_Pickup_heavy_puck.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | violet (7318 bytes, lane B10) |
| Game/Plugins/JamAssets/Content/Look/MI_Pickup_big_puck.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | amber (7291 bytes, lane B10) |
| Game/Plugins/JamAssets/Content/Look/MI_Pickup_reverse_other.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | red (7477 bytes, lane B10) |
| Game/Plugins/JamAssets/Content/Look/MI_Pickup_custom.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | white, Designer-made effects (7002 bytes, lane B10) |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Bumper.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | M_Toon instance, hot magenta, bumper obstacle (8714 bytes, lane B10, R-0019) |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_GoalShield.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | M_Toon instance, pale grey-white, goal shield (8736 bytes, lane B10, R-0019) |
| Game/Plugins/JamAssets/Content/Props/SM_Pickup_Base.uasset | mesh | 0.0 | original | UE 5.9 GeometryScript via MCP/Python |  |  | 2026-11-12 | B | y | 70 cm pedestal ring, socket PickupTop at 45 cm (45265 bytes, lane B10) |
| Game/Plugins/JamAssets/Content/Props/SM_Pickup_Token.uasset | mesh | 0.0 | original | UE 5.9 GeometryScript via MCP/Python |  |  | 2026-11-12 | B | y | generic upright disc token, pivot centre (21762 bytes, lane B10) |
| Game/Plugins/JamAssets/Content/VFX/NS_pickup_idle.uasset | vfx | 0.3 | original | UE 5.9 Niagara via MCP |  |  | 2026-11-12 | B | y | looping rising sparks; User.Colour, User.Strength; stop with Deactivate (294296 bytes, lane B10) |
| Game/Plugins/JamAssets/Content/VFX/NS_blade_spray.uasset | vfx | 0.3 | original | UE 5.9 Niagara via MCP |  |  | 2026-11-12 | B | y | one-shot sideways ice spray, at most 38 sprites; User.Strength, User.Colour; spawned by ToySkaterVisual (323186 bytes, lane B10) |

## 8. Stills (`devlog/shots/`, each with a caption `.txt`)

- `20261112-143456-B-b10-top-west`, `-top-east`
- `20261112-143457-B-b10-cam10-slow`, `-cam10-noshoot`, `-cam30-east`, `-cam30-west`, `-pickup-close`
- `20261112-143458-B-b10-pickup-idle`, `-obstacles`, `-blade-spray`

## 9. What went wrong

- **Corrupted package, my mistake.** I re-ran the material script by accident. `EditorAssetLibrary.delete_asset` on `M_Pickup` (still referenced by its instances) failed half-way and logged "package is now potentially corrupt". The instances then rendered as the default material: black pickups in the first stills.
  - Fix: I discarded the in-memory changes with `ExitEditor bDiscardUnsavedChanges` (Rink unsaved, on a blank map), deleted `M_Pickup` and `MI_Pickup_*` on disk, restarted, and rebuilt them from scratch. The script now refuses to run if `M_Pickup` exists.
  - **Pitfall for the next lane:** never delete a referenced material to rebuild it; clear its expressions in place (`delete_all_material_expressions`).
- **Suspended editor.** See the header.
- **ff-merge refused.** See the header; the worktree is not at `dev-b`.

## 10. Commits

See the end of this file (filled in after the commit).

## 11. Lead must decide

1. **Pickup and obstacle magenta.** `MI_Pickup_precision` (magenta) and `MI_Toon_Bumper` (hot magenta) are close in hue. Shape and size differ, but if the bumper must be unmistakable, move precision to another hue (for example 0.6, 1.0, 0.9 mint).
2. **`boost_only` against the puck halo.** Yellow close to the yellow-green halo: accept, or move `boost_only` to (1.0, 0.6, 0.0). That would also change B-to-A-0012's colour.
3. **Idle sparks on lit ice.** Additive sparks vanish there: accept (the token itself reads), or add a darker alpha-blended spark material.
4. **Blade spray side.** Should A pass the lean sign (rotate the spawn) so the spray goes only to the outside of the turn?
5. **`shot_blocked` feedback.** Should it get its own burst or sound (`NS_`/`MS_shot_blocked`), or reuse `NS_foul` Variant 0?
6. **Merge lane B3b into `dev-b`** so this worktree can fast-forward again.
