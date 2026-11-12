# B20 report: the P4 lab, chasing the concept's ice and skater (device B)

Lane B20, device B, machine clock 2026-11-12 22:54 to 2026-11-13 00:06. Tree `wt/b-vfx`, editor on port 8002 (pid 41676, then pid 32716 for the ray-tracing run), left both times through `ExitEditor` with **no dirty package**. No git, no build. `/Game/Maps/Rink` never saved (mtime 2026-11-12 12:40, 18,966 bytes); it was opened again from disk after each rig edit, never reloaded while open. Nothing under `_Scratch/`. `jam.Cut` back to `hockey.regular`, `jam.ScriptHuman` 0. The 8000 editor shared the GPU all night: **no frame numbers**, costs below are estimates. Checkpoint: `status/B20-checkpoint.md`. Scratch (every script, every capture): `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b20` (`lab.py` = all PIE-side settings, `mk_mats.py` = the masters, `rig_build.py` = the rig, `mk_tex.py` = the textures).

**Scope:** techniques 1 and 2 were run as briefed; at 23:40 the operator narrowed the lab to **the ice surface and the skater on it**. Techniques 3 (beams) and 4 (boards, glass, LED rails) had each been run once before that message and are reported as found, not iterated. Technique 6 was set once (bloom and tone curve). Technique 7 became "the best ice and skater per camera".

## Method

- Real game in PIE on `Rink`: QA1's scratch cartridge (`hockey.regular`, `game.look.tier p4_broadcast`, `game.skater.visual generated_b2`), `jam.ScriptHuman 1`, frozen at the opening faceoff (slomo 0.05, then 0.0001 at t = 2.17 s; frozen t 2.168-2.194 across 5 sessions).
- Cameras: **concept** (3 m behind the blue skater at (-4.85, 0) m, 1.2 m up, pitch -6, FOV 70), **game** (the follow camera at the faceoff), **tq** (close three-quarter, (-3.0, -2.6, 1.3) m), the same two behind the orange skater, plus 8 m, 24 m, low 2 m, corner for techniques 1-4.
- **Python cannot spawn actors or add components in PIE in this build** (`begin_deferred_actor_spawn_from_class`, `add_component_by_class` are not exposed). So the lab arena is built into `LR_P4_Broadcast` in the editor and tuned live in PIE (light values, rotations, positions of movable actors, MIDs on A's components).
- **Two traps:** (1) the editor **does not save while PIE runs**, and `save_loaded_asset` from Python fails silently (`LogUtils: Error: The Editor is currently in a play mode`); everything was saved at the end with PIE stopped. (2) **FBX import is refused during PIE** ("Cannot import SkeletalMeshNode asset at runtime"). (3) Lumen reflections need a few real frames to converge after any change: at slomo 0.0001 a capture needs 5-8 s of settle, or the reflections are missing.

## Per technique

| # | Built | Settings | This machine | Expected cost (4090 / mid-range, estimate) | Verdict | Stills |
|---|---|---|---|---|---|---|
| 1 Dark arena, key light | B14's 8 wash rect lights, 2 keys, lamp boxes, key cans, lamp rails removed; 14 spots `Lamp_P4_Broadcast_n` on a lab truss at 18 m (rows y +-9, x -54..54 every 18 m); dark housings + 4 emissive lens discs each; a far-end **bank of 48 emissive lamp heads** at x 62 m, z 9-12 m; sky light 0.05; exposure fixed EV100 4.2 | 3000 cd, cones 18/50, att. 45 m, 5600 K, source 25 cm, rows cross-aimed (y x -1.4) so the far boards are lit; lens Intensity 800 | handled | 14 movable spots unshadowed ~0.3-0.5 ms; with all 14 VSM-shadowed ~1.5-2 ms on a 4090 (B1c: 4 spots 0.84-1.0 ms on B), 3-5 ms mid-range | **toward** | `b20-t1-dark-arena`, `-t1-shadows` |
| 1b Shadows | 4 shadowed lamps: weak soft shadow. **All 14 shadowed: a soft multi-shadow that grounds the skater as the concept does** | contact shadows 0 | handled | the 14-shadow row above | **toward** (cost is the question) | `b20-skater-p4d` middle row |
| 2 Ice | `M_P4_Ice` (lit, opaque): blue body, numpy scratch tiles at 6 m and 2.3 m rotated 30 deg (fine skate lines, long lines, gouges, **turn arcs** r 0.6-1.5 m), broad scuffs at 23 m, a **deep scratch layer** (same tile, view-offset 1.5 cm, bluish), **wet patches** (40 m noise, roughness 0.03), markings from `M_Ice`'s own line code on A's parameters, under the surface | body (0.24, 0.31, 0.42), roughness clean 0.05 / scratch 0.22 / scuff 0.18 / wet 0.03, specular 1.0, bump 0.3 | handled | 9 texture samples per ice pixel: < 0.1 ms / ~0.2 ms; reflections below | **toward** | `b20-t2-ice`, `-ice-iterations` |
| 3 Beams (run once) | (a) exponential height fog with volumetric fog, lights' scattering 4-25; (b) cone meshes `Beam_P4_Broadcast_n`, `M_P4_Beam` additive, depth-faded, noise-broken, height-faded, faded within 6 m of the camera | (a) density 0.012 / (b) Intensity 0.15 | both handled, no stall | (a) 0.5-1 ms / 1.5-3 ms; (b) ~0.1-0.2 ms translucency | (a) **away**: a grey haze over the whole dark, soft halos, no distinct shafts; (b) **toward** for the concept's shafts in low views, invisible from the game camera, and **no wash with the camera inside a cone** (the near/height fade solves A23's problem). Dropped by the scope change | `b20-t3-beams` |
| 4 Boards, glass, LED (run once) | `M_P4_Board` white plastic with low scuffs on `Boards`, `MI_P4_KickStrip` on `KickPlate`, `M_P4_Glass` on `Glass` + Lumen front-layer translucency reflections, `M_P4_Led` on A's `Trim`/`TrimOrange` (they are at the board top, blue/orange by half along x: where the concept has rails) | LED 60 | handled | ~0 / ~0 (front-layer translucency reflections ~0.3 ms) | boards and LEDs **toward** (the LED rails reflect in the ice as the concept's streaks); glass **neutral** (nothing lit behind it to show) | `b20-t4-boards-glass-led` |
| 5 Skater | P4D re-imported (below), `M_P4_Figure` on it in PIE | below | handled | same as any skinned mesh | **toward** | `b20-skater-p4d`, `-best-threequarter` |
| 6 Picture (set once) | on `PPV_P4_Broadcast` | bloom 0.9 threshold 1, film toe 0.65 slope 0.9 shoulder 0.24, contrast 1.08, saturation 1.06, vignette 0.35, lens flare 0, motion blur 0 | handled | ~0 | **toward** (lamp bloom, deeper blacks) | `b20-ice-iterations` panel 4 |

Which component got which material (PIE only, by `lab.py`): `RinkActor.Ice` <- MID of `MI_P4_Ice` (A's `RinkHalfLength` 4500, `RinkHalfWidth` 2250, `BlueLineFrac` 0.3333, `GoalLineOffsetCm` 400 copied from A's own ice MID); `Boards` <- `MI_P4_Board`; `KickPlate` <- `MI_P4_KickStrip`; `Glass` <- `MI_P4_Glass`; `Trim` <- `MI_P4_Led_Blue`; `TrimOrange` <- `MI_P4_Led_Orange`; `Markings` untouched (A's faceoff circles, dots, creases stay matte on top of the ice); each `BP_SkaterVisual_TB2_P4` visual's `SkeletalMeshComponent` <- `SK_SkaterGen_P4D`, single-node `AS_SkaterGen_P4D_Stride` at 0.45 s, slot 0 <- MID of `MI_P4_Figure` with that pawn's `TeamColour`, slot 2 (stick) <- a near-black MID of `M_P4_Board`.

## Reflections comparison

| Method | Concept camera (low, 1.2 m) | Game camera | Lamps / LED / skaters in the ice | Shippable? |
|---|---|---|---|---|
| none | analytic glints only | scratched ice | no / no / no | yes, 0 ms |
| SSR (quality 100) | lamp streaks faint, at the horizon only; noise at the board line | faint figure smears | partly / yes when on screen / faint | +1.7 ms on a 4090 (A23); worse than Lumen here |
| **Lumen software** (quality 1, max roughness to trace 0.4) | **vertical lamp streaks from the far bank, as in the concept** | LED rails as orange/blue streaks along the boards | **yes / yes / faint** | the one to ship: +0.5-0.9 ms on a 4090 (A23 +0.9 at Wet 0.6), 1.5-2.5 ms mid-range |
| Lumen hardware RT (editor restarted with `r.RayTracing` + skin cache; `r.Lumen.HardwareRayTracing 1`; also hit lighting) | **no visible change** (mean diff 2 levels) | no visible change | same | not worth it: A23 +0.5-0.7 ms; two project settings plus RT shader permutations for every material (A's). This machine runs it: RT enabled, editor up in 1 min, PIE in 1.7 min, no stall. In 5.9 `r.Lumen.Reflections.HardwareRayTracing` no longer exists; `r.Lumen.HardwareRayTracing` covers reflections |

**What made the reflections read was not the method but what there is to reflect:** lamps overhead at 18 m never reach a low or a game camera's mirror path; a bank of bright lamp heads low at the far end, and the LED rails at the board top, do. The skater's own reflection stays faint on every method: a dark-blue jersey reflected at 15-25 % Fresnel is lost on a lit, scratched ice; it shows on a clean mirror test only (`b20-t2-mirror-test`). The concept's player reflections need either a darker, glossier ice than its lamp-lit read allows, or are an artist's exaggeration.

## The ice, in depth

Iterations at the concept camera (`b20-ice-iterations`, `-best-concept-cam-blue`): scratches + deep layer + wet patches -> far lamp bank (the streaks appear) -> darker body (reflections stronger, too grey) -> body 0.22 + turn arcs + bloom/toe -> bluer body (0.24, 0.31, 0.42, final).

- **Matches the concept:** a blue-grey glossy sheet with dense white scratches at several scales that run over the painted lines; old scratches under the surface; vertical light streaks from far lamps; LED rails mirrored along the boards; dark arena behind; lamp bloom.
- **Still far:** the concept's ice is lighter overall because it reflects a **lit, busy arena** (stands, crowd, many lamps all around): ours reflects black except where a lamp is; the concept's reflections are broad, bright and everywhere, ours are a few streaks. The concept's scratches are crisper and more varied in brightness near the camera (a 1K tile at 6 m is soft at 3 m). Skater reflections (above). Faint tiling is not visible at any camera tried.
- **Snow spray:** `NS_blade_spray` (unchanged, B17) on this look gives **blue-grey cel discs with dark rims, 6-17 cm, flying to chest height** (`b20-blade-spray`): it reads as bubbles. The concept throws **fine white powder**: what it lacks is a lit, white, specular particle (not the tier's dark-rim chip), 1-3 cm particles in the hundreds, velocity-stretched streaks, a soft misty puff sprite, a flat fan that stays under the knee and leaves from the blade edge along the blade, and a few shavings left lying on the ice. That is a `VFX/` change (a broadcast branch in `M_Fx_Tier` and a spray emitter), not made.
- **How close:** concept t2 (dark arena): **about 60-65 % of the way**; t1 and t3 (lit, crowded arena; t3 neon): **about 45 %**, because their ice brightness comes from the arena it reflects. The rest is the arena (lit stands, many lamps) more than the ice material, plus spray and crisper close scratches.

## The skater, in depth

- **Import:** SKI2's `i1.body` and `mat.body` (ports and trees adapted, PIE stopped): `SK_SkaterGen_P4D`, its skeleton (left as `SK_SkaterGen_P4D_Skeleton`, SKI2's rename did not apply), 72 `AS_SkaterGen_P4D_*` takes, `T_SkaterGen_P4D_BaseColor`, `MI_SkaterGen_P4D` (M_SkaterTB2, key gain 10 bias 2.3). About 2 minutes. **Not done:** sockets, blend spaces, AnimBP copy and the Blueprint graph (`gen.mjs`), so `BP_SkaterVisual_TB2_P4` is unchanged (still P4B) and the game would not show P4D by itself; the stills swap the mesh into the six game visuals in PIE. Finishing the Blueprint is SKI2's remaining ~15 minutes of scripts.
- **Material `M_P4_Figure`** (mine, `Look/P4Lab/`): the P4D texture already carries the white sleeve, hem and sock stripes, the V-neck and white laces/silver blades; on top: the hue key re-coloured to the pawn's `TeamColour` (lifted with gamma 0.65 so orange reads orange, not maroon), a white **yoke band** by rest-pose height (134-140 cm), **dark gloves** by rest-pose height and reach (z 55-98 cm, beyond 36 cm from the axis), dark skates under 22 cm except the white parts, roughness cloth 0.85 / skin 0.55 / glove 0.6 / helmet 0.22 with specular 0.6 (a clear helmet highlight), a near-black stick. Ground shadow from all 14 lamps shadowed.
- **Pose:** `Stride` at 0.45 s gives the concept's forward lean with the free arm out.
- **How close (vs the foreground player in t1/t2/t3):** **about 50 %**. Colours, stripes, gloves, skates, helmet sheen and the grounding shadow are right in kind. What is far is mostly the mesh.

### What is wrong with the P4D mesh that materials cannot fix (for the parallel figure lane)

1. **Toy proportions:** the head and helmet are about 1/5 of the height (the concept's player about 1/7.5); legs short against the torso; the figure reads as a child or a bobblehead from every camera, worst at the concept's low framing.
2. **Cage, no visor:** a full white wire cage over a cartoon face; the concept has a clear half-visor over a realistic face seen from behind. A visor highlight needs visor geometry.
3. **Oversized mitten gloves** fused to the stick: no fingers or cuff break; the concept has slim hockey gloves with a cuff.
4. **No hockey pants silhouette:** short navy shorts with a hard step to the thigh pads; the concept has full padded pants down to the knee.
5. **Jersey has no cloth:** it is skin-tight on the torso, with no folds, no loose hem, no sleeve volume; the concept's jersey hangs off the shoulders with folds (cloth roughness cannot invent folds).
6. **Back of the jersey:** smudged white blobs where the concept sheet had a number: a texture artefact on the back, at the concept camera's centre of attention.
7. **Neck and shoulders:** thick neck and narrow, sloped shoulders under the big helmet; the concept's shoulders are broad with shoulder pads under the yoke.
8. **Skates:** chunky boots with flat soles; the concept's skates are slim with a visible blade holder.
9. **Stick:** the S4 stick is long and thin with a flat wooden blade; the concept's is a dark composite with a white-taped curved blade (material fixes colour, not shape).
10. **Pose library:** the concept's deep crossover/push lean and snow-throwing stop are not in the takes; `Stride` is the nearest.

## Joined result (`b20-best-*`)

At the concept framing the picture now has the concept's structure: dark arena, lamp bloom, glossy scratched blue ice with streaks, a skater in team colours with stripes and dark gloves grounded by soft shadows. Against each concept image:

| Concept | Ice | Skater | Gap is |
|---|---|---|---|
| t1 (lit arena, crowd) | ~45 % | ~50 % (orange weaker than blue) | arena (assets: stands, crowd, many lamps) for the ice; mesh for the skater |
| t2 (dark arena, spot rigs) | ~60-65 % | ~50 % | snow spray (technique, `VFX/`), crisper near scratches (technique/time), mesh |
| t3 (neon, pickups) | ~45 % | ~50 % | arena and neon dressing (assets), mesh |

**Readability:** at the game camera both teams read (blue jersey with white stripes, orange jersey, dark gloves). At 24 m (`fin-24m` in scratch) blue reads clearly; **orange reads as a dark red-brown**, weaker than the night tier's orange in SK5 (88 luma in the dark band): the jersey key keeps the texture's dark shading, so a lighter orange needs a higher `TeamHi` or a key that ignores the texture's shade. The puck keeps its halo at the faceoff dot. Not measured as luma boxes (frames moved between sessions).

**Two days on this pipeline?** The **ice**: yes, a shipped tier at this level (material, lamp bank, Lumen software reflections, post) is about a day of work across B and A. The **skater at concept level**: not with P4D; it needs a better mesh (the list above) and then SKI2's import, which is about 20 minutes per figure once a mesh exists. The skater gap is assets, not technique.

## What would have to change to make this a real tier

- **Ice:** a per-tier ice master is cleaner than another path in `M_Ice` (which already carries cel, clay and broadcast branches and another lane is editing it): A's `RinkActor` would load `MI_P4_Ice` for `p4_broadcast` by name (today it assigns `MI_Ice_Night` on every tier and writes `RinkHalfLength`, `RinkHalfWidth`, `BlueLineFrac`, `GoalLineOffsetCm`, `Wet`, `Scuff`; `M_P4_Ice` reads the first four under the same names). Alternatively B adds `MPC_Look.Clay == -1` branches to `M_Ice` (no A change).
- **Figure:** `BP_SkaterVisual_TB2_P4` rebuilt on P4D (or its successor) with SKI2's `gen.mjs`, its slot 0 MID from `M_P4_Figure` (or the figure master gains these paths); team colour still comes from the pawn.
- **Boards, kick plate, glass, trims:** A's code loads one instance name per part on every tier (`MI_Toon_Board`, `MI_Toon_KickPlate`, `MI_Toon_Glass`, `MI_Trim_Blue/Orange`); a tier needs A to choose by tier (B15 asked the same for the glass).
- **Lights:** the rig carries them (`LR_P4_Broadcast`, saved); shadowed lamp count is the cost lever (4 vs 14). A's switcher scales local lights with the rink; the far lamp bank must move with the rink length on large cuts.
- **Project settings (A's):** none required for the recommended set (software Lumen reflections are already the project default). Not recommended: `r.RayTracing` (no visible gain); volumetric fog (away). Optional: `r.Lumen.TranslucencyReflections.FrontLayer.EnableForProject 1` for glass reflections (only if the glass is pursued).
- **VFX (B's `VFX/` owner):** a broadcast snow spray (above).

## Assets created or changed (bytes on disk, `Game/Plugins/JamAssets/Content/`)

| File | Bytes |
|---|---:|
| `Rigs/LR_P4_Broadcast.umap` | 465,441 (B14) -> 843,777 |
| `Look/P4Lab/M_P4_Ice` | 41,354 |
| `Look/P4Lab/M_P4_Figure` | 36,475 |
| `Look/P4Lab/M_P4_Board` | 19,696 |
| `Look/P4Lab/M_P4_Beam` | 18,092 |
| `Look/P4Lab/M_P4_Glass` | 16,650 |
| `Look/P4Lab/M_P4_Led` | 11,275 |
| `Look/P4Lab/MI_P4_Ice` | 16,579 (final values saved) |
| `Look/P4Lab/MI_P4_Figure` | 13,930 |
| `Look/P4Lab/MI_P4_Glass` | 9,628 |
| `Look/P4Lab/MI_P4_KickStrip` | 9,392 |
| `Look/P4Lab/MI_P4_Board` | 9,345 |
| `Look/P4Lab/MI_P4_Led_Orange` | 8,800 |
| `Look/P4Lab/MI_P4_Led_Blue` | 8,431 |
| `Look/P4Lab/MI_P4_Truss` | 8,005 |
| `Look/P4Lab/MI_P4_LampLens` | 6,324 |
| `Look/P4Lab/MI_P4_Beam` | 5,958 |
| `Look/P4Lab/T_P4_IceScratch_M` | 1,564,874 |
| `Look/P4Lab/T_P4_IceScratch_N` | 1,613,409 |
| `Look/P4Lab/T_P4_Noise` | 763,279 |
| `Skaters/SK_SkaterGen_P4D` | 5,040,696 |
| `Skaters/SK_SkaterGen_P4D_Skeleton` | 13,819 |
| `Skaters/T_SkaterGen_P4D_BaseColor` | 2,643,612 |
| `Skaters/MI_SkaterGen_P4D` | 12,519 |
| `Skaters/AS_SkaterGen_P4D_*` (72 takes) | 136,571-363,157 each; the 76 P4D files total 21,050,988 |

Not changed: `M_Ice`, `M_Toon`, `M_Trim`, every other `Look/` asset, `LR_P3_Clay`, `LR_L2_Night`, `VFX/`, `Audio/`, `Props/`, `BP_SkaterVisual_TB2_P4` (1,688,460, untouched). B14's `MI_Bcast_*` instances are now referenced by nothing in the rig except `MI_Bcast_Stands`, `MI_Bcast_Truss`, `MI_Bcast_Volume` (kept stands, roof trusses, back volume). `LR_P4_Broadcast` now: PPV (B14's plus bloom/tone/Lumen/motion blur as above), sky 0.05, LookParams, B14 stands/trusses/back, 14 lamps (all shadowed), heads, lab truss, 48-head far lamp bank, 14 beam cones (hidden), 6 glass-top LED rails (hidden), the volumetric fog actor (invisible). B14's backup: scratch `backup_LR_P4_Broadcast_B14.umap`.

Ledger rows (`shipped` n). The `LR_P4_Broadcast` row **replaces** B14's.

```
| Game/Plugins/JamAssets/Content/Rigs/LR_P4_Broadcast.umap | other | 0.8 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | P4 lab arena: PPV (EV100 4.2, bloom 0.9, filmic toe 0.65, Lumen reflections, no motion blur), sky 0.05, LookParams (Clay -1), 14 shadowed spot lamps on a lab truss at 18 m with emissive heads, far-end bank of 48 emissive lamp heads, hidden beam cones, hidden glass-top LED rails, off volumetric fog; B14's stands, roof trusses and back volume kept (lanes B14, B20) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/M_P4_Ice.uasset | material | 0.04 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | lab ice master: scratch tiles at 3 scales + deep parallax layer + wet patches, M_Ice's line code on A's rink parameters under the surface (lane B20) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/M_P4_Figure.uasset | material | 0.04 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | lab skater master on the P4D texture: hue key to TeamColour, yoke, dark gloves and skates by rest-pose position, cloth/helmet roughness (lane B20) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/M_P4_Board.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | lab lit surface master: base, kick strip by height, low scuffs (lane B20) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/M_P4_Beam.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | lab additive light-cone master: depth fade, height fade, near-camera fade, noise (lane B20) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/M_P4_Glass.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | lab translucent glass master, Fresnel opacity, lit top edge (lane B20) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/M_P4_Led.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | lab unlit emissive master for LED rails and lamp lenses (lane B20) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/MI_P4_Ice.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | instance of M_P4_Ice, final lab values (lane B20) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/MI_P4_Figure.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | instance of M_P4_Figure, final lab values (lane B20) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/MI_P4_Glass.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | instance of M_P4_Glass (lane B20) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/MI_P4_KickStrip.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | instance of M_P4_Board, yellow kick strip (lane B20) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/MI_P4_Board.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | instance of M_P4_Board, white boards with scuffs (lane B20) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/MI_P4_Led_Orange.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | instance of M_P4_Led, orange (lane B20) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/MI_P4_Led_Blue.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | instance of M_P4_Led, blue (lane B20) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/MI_P4_Truss.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | instance of M_P4_Board, dark metal (lane B20) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/MI_P4_LampLens.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | instance of M_P4_Led, lamp lens 800 (lane B20) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/MI_P4_Beam.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | instance of M_P4_Beam (lane B20) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/T_P4_IceScratch_M.uasset | texture | 1.49 | original | Python script (numpy, scratchpad/b20/mk_tex.py) |  |  | 2026-11-13 | B | n | procedural tileable ice scratch mask 1024: fine lines, long lines + gouges + turn arcs, broad scuffs (lane B20) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/T_P4_IceScratch_N.uasset | texture | 1.54 | original | Python script (numpy, scratchpad/b20/mk_tex.py) |  |  | 2026-11-13 | B | n | procedural tileable scratch-groove normal map 1024 (lane B20) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/T_P4_Noise.uasset | texture | 0.73 | original | Python script (numpy, scratchpad/b20/mk_tex.py) |  |  | 2026-11-13 | B | n | procedural tileable value noise, 3 octave sets, 512 (lane B20) |
| Game/Plugins/JamAssets/Content/Skaters/SK_SkaterGen_P4D.uasset | mesh | 4.81 | ai-generated | Codex concept sheet + Hyper3D Rodin + Meshy rig |  |  | 2026-11-13 | B | n | skc/export2/P4D loopcut FBX; Rodin ca2526c2-a9c3-494e-a2fb-c29f0dde9609, Meshy rig 01a1d16a-4f72-748c-9bab-f5d2dbf213fe; root and boot weights by skc/retarget.py; S4 stick (original); no sockets yet (lane B20 with SKI2's i1.body) |
| Game/Plugins/JamAssets/Content/Skaters/SK_SkaterGen_P4D_Skeleton.uasset | other | 0.01 | ai-generated | Codex concept sheet + Hyper3D Rodin + Meshy rig |  |  | 2026-11-13 | B | n | own skeleton of P4D, 32 bones; Meshy rig 01a1d16a-4f72-748c-9bab-f5d2dbf213fe (lane B20) |
| Game/Plugins/JamAssets/Content/Skaters/T_SkaterGen_P4D_BaseColor.uasset | texture | 2.52 | ai-generated | Codex concept sheet + Hyper3D Rodin + Meshy rig |  |  | 2026-11-13 | B | n | P4D base colour, 2048 source, MaxTextureSize 1024, BC1 no alpha, Lossy Medium; Rodin ca2526c2-a9c3-494e-a2fb-c29f0dde9609 (lane B20 with SKI2's mat.body) |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterGen_P4D.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | instance of M_SkaterTB2, KeyGain 10, KeyBias 2.3 (lane B20 with SKI2's mat.body) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterGen_P4D_<Take>.uasset | anim | 0.13-0.35 | original | Blender 4.2 script, retargeted |  |  | 2026-11-13 | B | n | skf/export5 take retargeted onto P4D by skc/retarget.py (skc/export2/P4D loopcut), 72 takes (lane B20 with SKI2's i1.body) |
```

## Six stills for the operator (`devlog/shots/`)

1. `20261113-000510-B-b20-best-concept-cam-blue`: the best ice and skater at the concept's framing.
2. `20261112-235758-B-b20-before-after`: B14's broadcast vs the lab, low camera and game camera.
3. `20261112-235758-B-b20-best-game-cam`: the game camera.
4. `20261112-235758-B-b20-best-threequarter` (and `-best-orange`): the P4D skater close, blue and orange.
5. `20261112-235236-B-b20-ice-iterations`: how the ice got there (the lamp bank is the step that matters).
6. `20261112-235237-B-b20-blade-spray`: what the current snow spray looks like on this look.

Also filed: `b20-t1-dark-arena`, `-t1-shadows`, `-t2-ice`, `-t2-reflections`, `-t2-mirror-test`, `-t3-beams`, `-t4-boards-glass-led`, `-ice-reflections-concept`, `-skater-p4d`, `-lumen-sw-vs-hwrt`.

## What I could not verify

- Any frame cost (shared GPU all night); every ms above is an estimate from A23's and B1c's numbers.
- Motion: stills only; scratch shimmer, Lumen reflection lag and the lamp streaks in motion are unseen.
- P4D in the game through its Blueprint (not rebuilt); P4D's sockets, carry and reactions; numbers on P4D.
- The rig on large rinks (the far lamp bank is placed for 90 x 45).
- The orange team at the concept framing with a lamp bank behind it (the bank is at +x only).
- Skater readability as SK5-style luma boxes (frames moved between sessions).

## What the lead must decide

1. Whether the ice is worth a tier: about 60 % of the way to t2 for roughly a day of work and +0.5-1 ms of Lumen reflections (plus 1-1.5 ms if all 14 lamps keep shadows).
2. Whether to commission the figure lane against the mesh list above: the skater gap is the mesh, not the materials.
3. Hardware ray tracing: recommend no (no visible change on this content, as A23 found).
4. Snow spray: ask the `VFX/` owner for a broadcast spray branch, or accept the current chips.
5. Per-tier instance selection in A's `RinkActor` (ice, boards, kick plate, glass, trims) if broadcast proceeds.
6. Keep or delete the lab assets (all `shipped` n) and the 76 P4D files; paste the ledger rows if kept.
