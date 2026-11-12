# Look lab A23: what moves the night rink toward the P2 reference (painted cel, night arena, toy figures)

Run 2026-11-12, 15:58 to about 17:50, lane A23 on device A (RTX 4090, UE 5.9.0-60351404), in the worktree `.claude/worktrees/a23-look` (detached at 6a2681e),
its own editor on MCP port 8011, the only editor on the machine. Nothing under `Game/Plugins/JamAssets` was written; B's rig `LR_L2_Night` and B's materials were only
read and, in PIE, replaced in memory. Everything the lab made lives in `Game/Content/LookLab/` (six masters, instances) and in this file; the stills are in
`docs/experiments/img/look-lab/`. The lab's scripts (`mat.py`, `lab.py`, `batch*.sh`, `gpu.mjs`) are in the lane's scratch folder, not in the repo: every parameter they set is listed below.

## Method (so the pairs are comparable)

- The real game in PIE: `hockey.regular` (90 x 45 m, seed 1), `jam.ScriptHuman 1`, `LR_L2_Night` streamed by the game, HUD widgets removed, 2560 x 1440 floating window
  (captures 2560 x 1354 in session 1, 2560 x 1442 in session 2: the window's title bar).
- One frozen frame per session: the sim steps from the world delta, so `set_global_time_dilation(0.0001)` freezes it (one sim frame per ~170 s of real time) while the
  camera manager keeps updating. Session 1: t = 2.17 s (faceoff just dropped, six figures around centre ice); session 2: the same instant at t = 2.10 s, and a play frame at
  t = 18.62 s (puck in play near the blue line). Pausing the game does not work for this: the camera manager stops, so a moved camera still shows the game view.
- A lab `CameraActor` 10, 24 and 40 m behind the blue centre (pitch -17.7 deg, FOV 70: the game camera's own geometry), set as the view target; the far-net stills put the
  camera 40 m from the net at x = +1 m. Every "before" is B's `MI_PP_ToonRamp` + `MI_PP_Outline` on B's masters; every "after" swaps only the thing under test, in memory.
- Cost: `PerfToolset.GpuProfilerToolset.TriggerGPUProfile {"minInclusiveMs":0}`, median of three, the PIE view family; whole-frame numbers carry about +-0.5 ms of noise
  (one editor on the GPU), so per-pass numbers are what to read for anything under 0.5 ms.

## Verdict per experiment

| # | Experiment | What moved the picture | What did not | Stills |
|---|---|---|---|---|
| 1 | Toon ramp: 2 / 3 / 4 bands, softness 0.02 / 0.08, cool shadow tint vs neutral (24 m) | **nothing on the figures**: the toy capsule is lit straight from above by its pool and carries B's flat emissive fill (2.5 x TeamColour), so the received light is almost uniform and there is no edge to step; the 12 variants are indistinguishable at 24 m. The ramp's paint read is on the **ice pools** (banded concentric rings), not on the figures. Dropping the fill to 1.0 and using 4 bands shows a lit cap and a darker base: the only way to see a band on a top-lit capsule. | band count, softness, tint | `sheet-ramp.jpg`, `sheet-fill.jpg` |
| 2 | Outline: constant 2 px vs 1.5-3 px by distance; ink vs darkened base; inner lines from normals | **width by distance** (3 px at <= 10 m to 1.5 px at 40 m, `WidthMode 1`) keeps the far figure from going black and gives the near one a drawn edge; **ink** (a constant near-black) reads as a line, the darkened base reads as shading at 10 m; **inner lines** (normal edges on stencil 1) separate helmet, band and body at 10-24 m and cost nothing visible at 40 m. | constant 2 px is heavier than B's 1.5 px + fade at 40 m with no gain at 10 m | `sheet-outline.jpg`, `outline-v2-10m.jpg`, `outline-v2-40m.jpg` |
| 3 | Ice: painted scuff and frost tiles; reflections planar / SSR / Lumen; the puck's halo | **two procedural tiles** (`M_LL_Ice2`: a streak tile stretched 10:1 along the rink and a frost-patch tile, both posterised to 3 and 2 levels) turn the pools into brushed paint at every distance; B's dark-ice floor (`Ambient` = BaseColour x (0.25, 0.35, 0.65) x 0.9, emissive) must be kept or the gaps go black. **Wet 0.3-0.6 with Lumen reflections** adds faint lamp pools on the ice (reference-like), +0.9 ms. The puck's yellow-green halo stays the most visible mark on dark ice under every variant; on the lit, wet pool it is still found at 24 m. | **SSR forced** (Lumen reflections off, `r.SSR.Quality 3`): +1.7 ms and no better. **Planar reflection**: 4.7 ms for its own pass and the engine overlays "requires global clip plane project setting" (not set in `DefaultEngine.ini`): rejected. Wet 1.0: glass, the paint read goes. | `sheet-ice3.jpg`, `ice2-24m.jpg`, `ice2-wet06-24m.jpg`, `ice-lab-wet06-ssr-24m.jpg`, `ice2-wet06-planar-24m.jpg` |
| 4 | Night volume: black vs deep-blue gradient vs haze; pool falloff; rim | **The blue gradient dome** (`M_LL_Sky`, unlit two-sided sphere r = 300 m, room walls and ceiling hidden) is the single biggest move toward the reference: the void goes, the gaps between pools lighten, the dressing has a horizon; +0.0 ms. **Spot inner cone 5 deg** (outer 26.5) gives soft pools with wider painted rings; 25 deg gives hard discs and the bands vanish. **Sky light 2 -> 4** lifts dark ice (puck findable) at the price of contrast; with the dome, 2 is enough. **Haze cards** (`M_LL_Haze`, additive, dense at the ice, gone at 18 m): visible only at Strength >= 0.8 and only the two **end** cards are safe; the side cards wash the frame edge when the camera is within ~15 m of them (seen at 40 m in the proof). | **Light-shaft cones** (additive cone per spot): invisible below Strength 1, and at 3 they wash the whole frame whenever the camera stands inside a pool, which the follow camera does most of the time. Not for the chase camera; maybe for the arena camera. **Rim** (RimStrength 1.5 on the figure material): no change visible at 10 or 24 m under top-down pools (the Fresnel term needs a light from behind). | `sheet-night.jpg`, `sheet-night2.jpg`, `night-sky-24m.jpg`, `night-spot-inner5-24m.jpg`, `night-spot-inner25-24m.jpg`, `night-sky-haze08-shafts3-40m.jpg` |
| 5 | Construction: kick plate, glass line, net frame; the far net at 40 m | **Kick plate** (25 cm ochre band at the foot of the inner board face) and the **glass rail** (dark 4 cm rail at 2.1 m with 5 cm stanchions) give the boards the two lines the reference has, read at 10 and 24 m, a one-pixel line at 40 m; **thicker red net frame** (14 cm posts and crossbar, a back bar, two braces) makes the far net at 40 m a bolder box, and the outline still helps it: without any outline the frame edges go pink. +0.0 ms. | at 40 m the kick plate and rail are below one pixel: they are for the near third of the ice | `sheet-constr.jpg`, `sheet-constr-farnet.jpg`, `constr-farnet-40m-outline.jpg` |
| 6 | Cost | see the table: nothing the recommendation uses is over 1.5 ms; the two things marked are the paths it rejects | | |

## The recommended combination (the proof stills)

`proof-faceoff-{10,24,40}m.jpg` and `proof-play-{10,24,40}m.jpg` (2560 px), against `base2-b-*` and `play-b-*` (B's current look, same frames). Sheets: `sheet-proof-faceoff.jpg`, `sheet-proof-play.jpg`.

| Element | Value (lab asset in `/Game/LookLab/`) | For B's master |
|---|---|---|
| Night sky | `M_LL_Sky` (`MI_LL_Sky`): Horizon (0.05, 0.09, 0.22), Zenith (0.004, 0.006, 0.018), Brightness 0.5, CloudStrength 0.35, Radius 30000 cm; an engine sphere scaled 600, stencil 0, no shadow, `affect_dynamic_indirect_lighting` off | a `Sky_Dome_L2_Night` static mesh in the rig with a `M_Sky` unlit two-sided gradient; drop `Room_L2_Night_2..6` (walls, ceiling), keep the floor |
| Haze | `MI_LL_HazeCard` (`M_LL_Haze` Mode 0): Strength 0.8, TopZ 1800, HazeColour (0.75, 0.80, 1.0); two 70 x 18 m planes at x = +-62 m only (no side cards) | `M_Haze` additive unlit; the two end cards in the rig; no shaft cones |
| Pools | `Spot_L2_Night_1..4` inner cone 5 deg (outer 26.5, intensity unchanged) | one number in the rig |
| Ice | `M_LL_Ice2`: IceColour (0.72, 0.80, 0.86), ScuffColour (0.50, 0.56, 0.62), FrostColour (0.88, 0.91, 0.95), Scuff 0.7, Frost 0.5, Posterize 3, ScuffBias 0.45, FrostBias 0.58, Wet 0.3, IceAmbient 0.9, AmbientColour (0.25, 0.35, 0.65); A's `Markings` component drew the lines in the lab | add the two tiles (the `Tiles` custom node: streak noise on world position x 0.10 along x, scale 0.06, 3 levels, turbulence; fine noise scale 0.03; frost noise scale 0.006, 3 levels; posterise) in front of `M_Ice`'s `ScuffMask`, keep its lines and `Ambient`; `MI_Ice_Night` Wet 0.3 |
| Figures | emissive fill 1.5 (B's `Emissive` on the figure instances; A's toy sets `BaseEmissive` 2.5 at build) | `MI_Toon_Blue` / `MI_Toon_Orange` Emissive 2.5 -> 1.5, or A's `UToySkaterVisualComponent::BaseEmissive` |
| Ramp | `M_LL_Ramp`: FigureBands 3, Softness 0.08, TopStops 3.5, RangeStops 3.0, TintStrength 1, ShadowTint (0.78, 0.88, 1.22), IceStopsPerStep 1.0, IceSoftness 0.08, ShadowFloor 0.004 | keep `M_PP_ToonRamp` as it is: the bounded ramp changed nothing visible on the figures; the cool tint is optional |
| Outline | `M_LL_Outline`: WidthMode 1, WidthNear 3, WidthFar 1.5, NearCm 1000, FarCm 4000, ColourMode 0 (ink), InkColour (0.012, 0.012, 0.028), InnerLines 1, NormalThreshold 0.35, DepthThreshold 0.08, FadeStart 1500, FadeEnd 4000, FadeMin 0.3, PuckBoost 1 | `M_PP_Outline`: width by distance instead of a constant 1.5, ink colour as a constant instead of `OutlineColour * scene` |
| Construction | `MI_LL_KickPlate` (0.85, 0.72, 0.12), `MI_LL_GlassRail` (0.05, 0.05, 0.06), `MI_LL_NetFrame` (0.70, 0.04, 0.03, Emissive 2); kick plate 25 x 2 cm at the board foot, rail 6 x 4 cm at z 2.1 m, stanchions 5 cm every outline segment, net posts and bar 14 cm, back bar, two braces; all stencil 1 | A's `RinkActor` builds the boards and nets, so this is a handoff to A: the three bands as extra sections of `Boards` and `Nets` with B's instances `MI_Toon_KickPlate`, `MI_Toon_GlassRail`, and thicker `Nets` section 0 |

What the combination costs: 7.0 ms whole frame at 2560 x 1442 against 6.5 for B's look on the same frame (+0.5, inside the noise band; the wet ice's Lumen reflections are the
only measurable part, +0.9 ms at Wet 0.6, less at 0.3). The post passes together stay under 0.1 ms.

## Cost table (ProfileGPU, RTX 4090, 2560 x 1354 / 1442 PIE, median of three; ms)

| Variant | Frame | Ramp pass | Outline pass | VSM + shadows | Deferred lighting | Translucency | TSR | Note |
|---|---|---|---|---|---|---|---|---|
| B's post (session 1 / 2) | 6.48 / 6.55 | 0.055 | 0.038 | 1.47 / 1.51 | 0.97 / 0.94 | 0.25 | 1.73 / 1.80 | the "before" |
| lab ramp 3 bands + outline constant 2 px | 6.14 | 0.060 | 0.040 | 1.44 | 0.84 | 0.24 | 1.72 | |
| lab outline by distance + inner lines | 6.56 | 0.058 | 0.039 | 1.35 | 0.78 | 0.24 | 2.14 | TSR sample noise |
| lab outline by distance, no inner lines | 6.04 | 0.060 | 0.036 | 1.47 | 0.86 | 0.24 | 1.72 | |
| both passes switched off (`RampOn 0`, `OutlineOn 0`) | 5.93 | 0.039 | 0.031 | 1.41 | 0.82 | 0.23 | 1.71 | the pass still runs its early-out |
| lab ice, dry | 6.10 | 0.060 | 0.043 | 1.28 | 0.75 | 0.24 | 1.72 | |
| lab ice Wet 0.6, Lumen reflections (session 1 / 2) | 6.11 / 7.38 | 0.054 | 0.039 | 1.45 | 0.89 / 1.69 | 0.25 | 1.72 / 1.81 | Lumen reflection work shows under lighting and diffuse-indirect (+0.9) |
| lab ice Wet 0.6, Lumen reflections off, `r.SSR.Quality 3` | **8.24** | 0.057 | 0.042 | 2.60 | 1.88 | 0.25 | 2.26 | **marked**: +1.7 ms, no better |
| lab ice Wet 0.6, planar reflection (screen percentage 50) | **9.11** | 0.057 | 0.039 | 2.54 | 1.40 | 0.35 | 1.83 | **marked**: `PlanarReflection` pass 4.66 ms, and invalid without the global clip plane setting |
| sky dome (room hidden) | 6.13 | 0.057 | 0.037 | 1.45 | 0.76 | 0.24 | 1.71 | |
| sky dome + 4 haze cards | 6.02 | 0.057 | 0.037 | 1.35 | 0.78 | 0.23 | 1.70 | |
| sky dome + haze + 4 shaft cones | 6.13 | 0.059 | 0.039 | 1.45 | 0.97 | 0.37 | 1.72 | translucency +0.13 |
| construction (182 small static meshes) | 5.96 | 0.059 | 0.038 | 1.38 | 0.86 | 0.23 | 1.70 | |
| **recommended combination** (session 2) | **7.01** | 0.063 | 0.040 | 1.44 | 0.91 | 0.25 | 1.81 | against 6.55 for B's look on the same frame |

The whole frame moves by +-0.5 ms between captures of the same scene (first vs second session of B's post: 6.48 vs 6.55; TSR alone wanders 1.70 to 2.14), so only the
two marked rows are real frame-level costs; everything else is read from the per-pass columns.

## Ray tracing block (the Director's addition, 16:13): HWRT Lumen, ray-traced reflections, DLAA

Session 3, the lab editor restarted with `-ini:Engine:[/Script/Engine.RendererSettings]:r.RayTracing=True -ini:Engine:[/Script/Engine.RendererSettings]:r.SkinCache.CompileShaders=True`
(`r.RayTracing` is read at start-up and is not in `Game/Config/DefaultEngine.ini`; without the skin-cache line the editor aborts at start: "Ray tracing requires skin cache to be
enabled", so turning RT on for the shipped game is two project settings plus a global-shader recompile, A's). Frozen at t = 1.78 s (the faceoff countdown, figures at +-7.1 m).
`r.RayTracing` read back 1; toggled at run time: `r.Lumen.HardwareRayTracing`, `r.Lumen.Reflections.HardwareRayTracing`, `r.Lumen.HardwareRayTracing.LightingMode 2` (hit lighting).

| Case (recommended combination, ice Wet 0.6) | Frame ms | RT scene pass | Shadows | Lighting | Reading |
|---|---|---|---|---|---|
| software Lumen, B's post | 7.51 / 8.14 | 0.001 | 1.52 | 1.84 | the "before" of this session (the first sample ran while shaders still warmed) |
| HWRT Lumen, B's post | 8.15 / 7.57 | 0.40 | 2.82 | 2.90 | **no visible change** at 24 m: same pools, same figures, same ink lines |
| software Lumen, combination | 7.70 | 0.001 | 1.55 | 1.63 | |
| HWRT Lumen + HWRT reflections, combination | 8.24 | 0.37 | 1.95 | 1.96 | **no visible change** at 10 / 24 / 40 m (`sheet-rt2.jpg`): the wet ice's lamp pools look the same, the toon ramp and outline survive untouched (they key on custom stencil and depth, not on the GI path) |
| + hit lighting (`LightingMode 2`), combination | 6.78 (noise) | 0.45 | 1.94 | 1.33 | no visible change (`rt2-hwrt-hitlight-combo-wet06-24m.jpg`) |
| + DLAA (`jam.dlss dlaa`) | - | - | - | - | **not testable here**: `jam.dlss supported=0 reason='not in this build' saved=dlaa effective=off` (the DLSS plugin is not in the Development editor target; lane A11's path is the packaged build) |

Verdict for the P4 question: on a scene of basic shapes, flat ice and four spots, hardware ray tracing buys nothing the eye can find at 10, 24 or 40 m and costs about
0.5 to 0.7 ms (the RT scene update plus heavier shadow and lighting passes), so there is no reason to ship `r.RayTracing` on for P2; it does not break the painted post, so it
stays an option for a later P4 pass with real reflective materials and dressing. Stills: `rt-swlumen-bpost-24m.jpg`, `rt-hwrt-bpost-24m.jpg`, `rt2-{swlumen,hwrt}-combo-wet06-{10,24,40}m.jpg`, `sheet-rt.jpg`, `sheet-rt2.jpg`.

## What B needs to change (B owns the shipped masters; these are proposals, the values are above)

1. `LR_L2_Night`: replace the room's walls and ceiling with a sky dome (`M_Sky` from `M_LL_Sky`), add two end haze cards (`M_Haze` from `M_LL_Haze`, Strength 0.8), set
   the four pool spots' inner cone to 5 deg. No shaft cones, no side cards.
2. `M_Ice`: the two posterised tiles in front of `ScuffMask` (graph in `M_LL_Ice2`), `MI_Ice_Night` Wet 0.3. Keep `Ambient`.
3. `M_PP_Outline`: width by distance (3 px at 10 m to 1.5 px at 40 m), ink as a constant colour, keep the inner (normal) lines. `M_PP_ToonRamp`: unchanged.
4. `MI_Toon_Blue` / `MI_Toon_Orange` Emissive 2.5 -> 1.5 (or A lowers `BaseEmissive` on the toy figure); the ramp cannot read on a top-lit capsule with a 2.5 fill.
5. Two instances for A's construction bands: `MI_Toon_KickPlate` (ochre) and `MI_Toon_GlassRail` (near black); A adds the bands and the thicker net frame in `RinkActor`.

What stays with A: the rink bands (`RinkActor.cpp`: kick plate, glass rail and stanchions as extra sections; net posts 14 cm), the figure fill, and, if the sky dome ships,
`ULookTierSwitcher` must not hide it with the persistent lights (it is a mesh, not a light: nothing to do today).

## Errors and pitfalls met

1. `connect_material_expressions` into a Noise node needs the pin name `World Position`, not `Position` (cost 5 minutes).
2. `create_asset` returns `None` for a material deleted and re-created under the same name in one session (E3's pitfall 7 again): `M_LL_Ice2` is `M_LL_Ice` plus the ambient term.
3. Pausing the game freezes the camera manager too: a `CameraActor` moved while paused is not re-read, so the first 30 captures were all the game camera and were thrown away;
   the time-dilation freeze keeps the camera live and the sim still.
4. `unreal.Rotator(a, b, c)` is roll, pitch, yaw: pass keywords.
5. `set_vector_parameter_value_on_materials` takes a `Vector`, not a `LinearColor`; `GameplayStatics` has no deferred spawn in Python, so anything that must exist in PIE is
   placed in the editor world before PIE (the planar reflection, the dome, the cards, the bands); `Rink` was never saved.
6. The PIE floating window shrank from 2560 x 1354 to 1762 x 970 once in session 1 (after the far-net captures; cause not found); the affected captures (`night-rim15-*`,
   `night-sky-haze08-*`, `ice-lab2-*`) are kept at that size and the ice set was retaken in session 2 at full size.
7. `execute_python` returns only the last expression: results went through the return value or a file, never `print`.
