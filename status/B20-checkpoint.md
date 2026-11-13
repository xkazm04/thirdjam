# B20 checkpoint: P4 lab after techniques 1 and 2 (device B)

Lane B20, device B, 2026-11-12, machine clock 22:54 to 23:22. Tree `wt/b-vfx`, editor on port 8002 (pid 41676). The 8000 editor shares the GPU: **no frame numbers**.

## Method

- Real game in PIE on `/Game/Maps/Rink`: scratch cartridge (QA1's `hockey.regular` copy with `game.look.tier p4_broadcast`, `game.skater.visual generated_b2`), `jam.Cut <path>`, `jam.ScriptHuman 1`, floating PIE, slomo 0.05 then 0.0001 at t = 2.17 s (frozen t 2.168 for the before set, 2.182 for the lab session).
- Cameras: the game's follow camera (faceoff frame: (-11.3, 0.8, 3.5) m, pitch -17.65, FOV 70), `face` (-7, 0.8, 1.9) pitch -7, `low` (-14, -3, 2.0) pitch -4 yaw 8, `8m` (-7.62, 0, 2.43) and `24m` (-22.87, 0, 7.30) at pitch -17.7, `corner` (-48, -30, 24) pitch -27 yaw 32.
- Python cannot spawn actors or add components in PIE in this build (`begin_deferred_actor_spawn_from_class` and `add_component_by_class` are not exposed), so the lab arena is built into `LR_P4_Broadcast` in the editor and tuned live in PIE (lights, rotations, MIDs). `Rink` was opened again from disk after the rig edit; it was never saved.

## Technique 1: dark arena and key light (verdict: toward)

| | |
|---|---|
| Built | B14's 8 wash rect lights, 2 key spots, 8 lamp boxes, 8 lamp faces, 2 key cans and 2 lamp rails removed from the rig. 14 `Lamp_P4_Broadcast_n` spots on a lab truss at 18 m (rows y = +-9 m, x = -54..54 every 18 m; the four at x = +-54 hang over the end stands so the far lamps come into a low camera's frame). Each head: dark housing (`MI_P4_Truss`) + four emissive lens discs (`MI_P4_LampLens`, `M_P4_Led`). Truss chords and cross members. Sky light 0.15 -> 0.05. B14's stands, roof trusses, dark volume kept. Exposure stays fixed at EV100 4.2. |
| Settings (live, final) | 3000 cd, inner 16 / outer 46 deg, attenuation 45 m, 5600 K, source radius 25 cm, aimed at (clamp(0.92 x, +-40), 0, 0); shadows on the 4 lamps at x = +-18 (resolution 0.5, no RT/contact shadows); lens Intensity 800, discs 42 cm |
| Measured (game camera, sRGB luma) | before (B14) the ice is 210-235 everywhere; after: centre ice 189, blue-line ice 179, board faces 33-34, sky band 32 |
| This machine | handled; no stall |
| Cost (estimate) | 14 movable spots, 4 with VSM shadows: B1c measured 4 shadowed spots at 0.84-1.0 ms of shadow depths on B; on a 4090 about 0.5-0.7 ms shadows + 0.3 ms lighting for 14 unshadowed-overlapping spots; mid-range (RTX 3060 class) 2-3x that. Cheaper than B14's 8 large rect lights covering the whole screen |
| Shadows | 4 shadowed lamps give the near figure a soft, weak shadow; all 14 shadowed gives a star of 3-5 soft shadows around each figure (`b20-t1-shadows`): grounded but busy, and 3.5x the shadow passes. The concept has one dark contact shadow: a contact shadow or one shadowed key per figure zone reads closer than more shadowed lamps |
| Out of reach | the concept's lit crowd (B14 stands are boxes; lit they would read as boxes) |

## Technique 2: reflective scratched ice (verdict: toward for the scratched surface, neutral for the reflections at the game camera)

| | |
|---|---|
| Built | `M_P4_Ice` (lit, opaque, default lit): blue-grey body, scratch mask `T_P4_IceScratch_M` (numpy, 1024, tileable: R fine skate scratches, G long scratches and gouges, B broad scuffs) sampled at 6 m, at 2.3 m rotated 30 deg, and at 23 m for the scuffs; `T_P4_IceScratch_N` grooves; `T_P4_Noise` 40 m body variation; roughness 0.02 clean, 0.18 scuff, 0.3 scratch. Markings: `M_Ice`'s own line code (centre red 60 cm, goal lines 12 cm, blue lines 60 cm, centre circle r 9 m) on the same parameters A's `RinkActor` sets (`RinkHalfLength` 4500, `RinkHalfWidth` 2250, `BlueLineFrac` 0.3333, `GoalLineOffsetCm` 400, read from A's ice MID and copied), drawn under the surface: the scratches and the gloss run over them. A's `Markings` sections 2 and 3 (faceoff circles, dots, creases) are A's geometry and stay on top, matte |
| Applied | PIE only: A's `RinkActor.Ice` (ProceduralMeshComponent) <- MID of `MI_P4_Ice` |
| Final values | IceBody (0.26, 0.31, 0.38), ScratchColour (0.92, 0.95, 0.98), ScratchBright 0.6, FineAmt 0.5, MedAmt 0.8, ScuffAmt 0.4, Bump 0.3, Specular 1.0 |
| This machine | handled |
| Cost (estimate) | 7 texture samples per pixel over the ice: under 0.1 ms on a 4090 and about 0.2 ms mid-range; the reflections are the cost (below) |

### Reflections compared (same frame, `b20-t2-reflections`)

| Method | Game camera | Low 2 m camera | Cost (estimate) |
|---|---|---|---|
| none | scratched ice, analytic lamp glints only | same | 0 |
| screen-space (quality 100, max roughness 0.4) | figures reflect faintly and sharper than Lumen; noisy streaks at the board line where the screen runs out | same; off-screen lamps can never appear | A23 measured +1.7 ms forced SSR on the 4090; mid-range 2-3 ms |
| Lumen software (quality 1, max roughness to trace 0.35) | **the LED trim along the boards reflects as an orange streak**; figures a faint soft smear | same, plus faint figure reflections | A23: +0.9 ms at Wet 0.6 on the 4090; the roughness cut at 0.35 keeps the boards and figures out of the trace; mid-range 1.5-2.5 ms |
| Lumen hardware RT | not run yet: `r.RayTracing` is not in `DefaultEngine.ini`; it needs an editor restart with ray tracing and skin cache on and a shader recompile. Planned at the end of the lab | | A23: +0.5-0.7 ms on top of software Lumen |

**Do the lamps, LED rails and figures appear in the ice?** LED trim: yes (Lumen). Figures: only faintly, even on a mirror test (`b20-t2-mirror-test`). Lamps: no, at either camera: with the lamps at 18 m, their mirror points lie between the camera and the lamp, close to the camera and out of frame. The concept's lamp streaks come from lamps that are low and far (at the far end and around the glass), and from **bright things near the horizon** to reflect: lit boards, glass, LED rails, lit stands. In this rig the surroundings are black, so the ice reflects black and reads as diffuse. Technique 4 (white boards, LED rails) is what will make the reflections read.

## Stills

`devlog/shots/20261112-232153-B-b20-t1-dark-arena`, `-t1-shadows`, `-t2-ice`, `-232154-B-b20-t2-reflections`, `-t2-mirror-test`.

## Scope change (23:40, operator through the coordinator)

Only **the ice surface** and **the skater on it** are chased now. Beams (technique 3) and boards/glass/LEDs (technique 4) were already run once before the message (`b20-t3-beams`, `b20-t4-boards-glass-led`); they are reported as found and not iterated.

## Ice, first good result (23:52)

- Added to `M_P4_Ice`: a **deep scratch layer** (the scratch tile sampled again with a view-dependent offset of 1.5 cm, bluish, so old scratches sit under the surface), **wet patches** (40 m noise, roughness 0.03, body 18 % darker), **turn arcs** (70 curved scratches, radius 0.6-1.5 m, in the medium channel), clean roughness 0.05, scratch 0.22, body (0.22, 0.27, 0.34).
- **The lamp streaks came from geometry, not from the material:** 48 emissive lens discs moved in PIE into a bank on the far end wall (x 62 m, z 9-12 m). With Lumen reflections they make the concept's vertical light streaks in the ice at the concept camera; at 18 m overhead the lamps never reflect toward a low or a game camera.
- Lumen software is the method that gives the streaks; SSR shows them faintly at the horizon only; none gives analytic glints only (`b20-ice-reflections-concept`). Lumen needs a few real frames to converge: at slomo 0.0001 captures need 5 s of settle.
- Post (technique 6, set once): bloom 0.9 threshold 1, filmic toe 0.65 slope 0.9 shoulder 0.24, contrast 1.08, saturation 1.06, vignette 0.35, no lens flare, no motion blur.
- Stills: `devlog/shots/20261112-235236-B-b20-ice-iterations`, `-ice-reflections-concept`.

## Skater, first result

- P4D re-imported with SKI2's `i1.body` and `mat.body` (mesh, its skeleton, 71 takes, `T_SkaterGen_P4D_BaseColor`, `MI_SkaterGen_P4D`). **Import is refused while PIE runs** ("Cannot import SkeletalMeshNode asset at runtime"): stop PIE first. Sockets, blend spaces, AnimBP and the Blueprint graph were **not** rebuilt: in PIE each of the six `BP_SkaterVisual_TB2_P4` visuals gets `SK_SkaterGen_P4D`, single-node `AS_SkaterGen_P4D_Stride` at 0.45 s, and a MID of `MI_P4_Figure` with the pawn's own `TeamColour`.
- `M_P4_Figure` (new, mine): hue key as SKI2's (gain 10, bias 2.3), team colour lifted (gamma 0.65), white yoke band by rest-pose height, dark gloves by rest-pose height and reach (z 55-98 cm, beyond 36 cm), dark skates under 22 cm except white laces/blades, roughness cloth 0.85 / skin 0.55 / helmet 0.22, specular 0.6 on the helmet; dark stick.
- All 14 lamps shadowed gives a soft multi-shadow that grounds the skater as in the concept (`b20-skater-p4d`, middle row).
- `NS_blade_spray` on this look: blue-grey cel discs with dark rims at chest height (`b20-blade-spray`): not snow.
- **Saving:** nothing created during PIE is saved (the editor refuses to save in PIE without an error through Python); all lab assets are saved at the end with PIE stopped.
