# Experiment E4: third-person camera, radar and far-net readability on a 120 x 60 m rink

Measured 2026-11-12 (about 01:00 to 02:00 CET, before the jam) on UE 5.9.0 with a throwaway content-only project (deleted afterwards), MCP port 8011,
`tools/run/editor.ps1`. Engine primitives only: a flat plane 120 x 60 m, board boxes, two box nets, six capsules (cylinder plus two spheres),
a puck cylinder (r 0.12 m, h 0.08 m, near-black), unlit flat-colour materials. Nothing here is kept as content (rule 7(h)); the recipe is
`tools/skills/camera-third-person.md`, the numbers and 14 small screenshots (all under 300 KB, `docs/experiments/img/cam-*.png`) are the result.

## Verdicts

| # | Question | Verdict | Short answer |
|---|---|---|---|
| 1 | Camera: distance, height, FOV, lag, pitch | PASS | 180 static combinations scored by projection, 20 screenshotted. Default `distance_m 6, height_m 3.5, fov_deg 70, lag_frames 4`; wide `distance_m 8, height_m 4, fov_deg 85, lag_frames 6`. Pitch is derived (look at a ground point 5 m ahead of the pawn), see below. |
| 2 | Far-net readability at 60-110 m | PASS | (b) screen-space marker with distance text reads at every distance and is the one to ship; (a) emissive beacon also reads (a 0.8 m x 9 m pillar against the black void), optional; (c) a brighter net frame does not read at 85-110 m. |
| 3 | Radar (UMG mini-rink) | PASS with a caveat | Drawn in PIE at 1920x1082, seven dots updated every tick from world positions (numbers below). The per-tick driver in this experiment was Python; the shipping driver (a C++ widget) was not built. 200 x 100 px, bottom-right. |
| 4 | Puck visibility | PASS | A plain near-black puck is findable on pale ice when still (30 px at 10 m, 13 px at 25 m, 8 px at 40 m) and lost when moving beyond 25 m. A short trail is the strongest aid; a shadow disc helps up to about 25 m; an emissive rim ring is marginal. M0: plain puck plus trail plus shadow disc. |
| 5 | Scale: does 120 m feel empty with six skaters? | PASS (finding: yes, empty) | One or two skaters visible at a time, the rest are specks. Try `rink.length_m` 90 (with `rink.width_m` 45) and `skater.speed` 11 first. |
| 6 | Determinism-friendly camera | PASS | Spring-arm lag uses DeltaSeconds (measured: the same per-frame motion gives 0.67 m at 60 Hz and 0.43 m at 30 Hz). A frame-based lag (absolute arm, one filter step per sim frame) gave identical rows at 60 and 30 Hz. |

## 1. Camera

Geometry (all in metres, rink along +X, pawn at the origin, camera looks along +X): the camera sits `distance_m` behind and `height_m` above the pawn's
feet. UE's `FOV` is the **horizontal** FOV (16:9 here, so fov 70 = 43 deg vertical). Verified on the real component chain: a `USpringArmComponent` pivoting at the
feet (relative z -90 cm under a 90 cm capsule), arm pitch = -atan(h/d), `TargetArmLength` = sqrt(d^2 + h^2), and a `UCameraComponent` whose relative pitch tilts it back up by
`atan(h/d) - atan(h/(d + look_ahead))`. For d 6, h 3.5, look-ahead 5 the engine placed the camera at (-6.000, 0.000, 3.500) m with pitch -17.65 deg, which is exactly the formula.
So `pitch_deg = -atan(height_m / (distance_m + look_ahead_m))`; `look_ahead_m` is not in the `game.camera` schema (see "Recommended `game.camera`").

### How the combinations were scored

Pawn at centre ice (0, 0) facing the far net at x = 58 m (60 m is the board). Opponents B1 (9, -3.5) and B2 (14, 5), the puck 15 m ahead, and a 6 m puck. A combination "passes"
when the puck at 15 m, the puck at 6 m, both nearest opponents and the far net all project inside the frame with a 6 % margin, via a pinhole projection that was
checked against the offscreen captures. Grid: distance 4/6/8 x height 2/3/4 x FOV 60/70/80/90 x pitch -4/-8/-12/-16/-20 = 180 cases.

Result: **170 of 180 pass, so static framing barely discriminates.** The 10 failures are 9 cases of pitch -20 with FOV 60 (the far net falls off the top edge) and one of pitch -4 (6 m puck lost). What discriminates is:

| Factor | Numbers (1080p) |
|---|---|
| Puck size at 15 m ahead (0.24 m across) | FOV 60: 17-21 px, FOV 70: 14-18 px, FOV 80: 12-15 px, FOV 90: 10-12 px. Distance 4: 12-22 px, distance 8: 10-18 px. |
| Opponent beside the pawn stays in frame | Only wide setups keep all three flank probes (1.5, 4.5), (-2, -5), (0, 6.5): distance 8 + FOV 85-90. FOV 70 at distance 6 keeps one of three. A close camera (distance 4, FOV 60) keeps none. |
| The pawn hides the puck on the stick | On-axis puck is hidden up to `1.8 * d / (h - 1.8)` m ahead of the pawn: 6.4 m (d 6, h 3.5), 4.9 m (d 6, h 4), 9.0 m (d 6, h 3), 36 m (d 4, h 2). Offset 0.7 m to the side and it is visible at every setting (`img/cam-stick-sheet.png`). |
| Far net near the top edge | Steep pitch pushes the net and its beacon off the frame (d 6, h 4, pitch -24: net at v = 12 px of 1080). |

### Sweep table (the screenshotted combinations, 1920x1080 captures)

Look-ahead 5 m (F1), 6 m (W), 4 m (T), 3 m (H); the others give their pitch. "Opp px" = pixel height of the two nearest opponents (1.8 m tall). All captures are in `img/`.

| Name | dist | height | FOV | pitch | puck 15 m | opp px | far net (58 m) | flank probes in frame | on-axis puck hidden to | Verdict |
|---|---|---|---|---|---|---|---|---|---|---|
| **F1 default** | 6 | 3.5 | 70 | -17.7 | 16 px | 164 / 124 | 89 px wide, v 167 | 1 of 3 | 6.4 m | best compromise; keep |
| **W wide** | 8 | 4 | 85 | -15.9 | 11 px | 110 / 86 | 65 px wide, v 295 | 3 of 3 | 6.5 m | wide option; puck smaller |
| T tight | 4 | 2.5 | 60 | -17.4 | 21 px | 232 / 169 | 111 px wide, v 71 | 0 of 3 | 10.3 m | skater fills 33 % of the height; flank blind |
| H high | 6 | 4 | 70 | -24.0 | 16 px | 165 / 126 | 92 px wide, v 12 | 1 of 3 | 4.9 m | net on the top edge, beacon clipped |
| c1 | 6 | 3 | 70 | -12 | 16 px | 163 / 123 | 87 px, v 298 | 1 of 3 | 9.0 m | too much black void above the boards |
| c3 | 8 | 3 | 90 | -12 | 10 px | 101 / 79 | 59 px, v 369 | 3 of 3 | 12 m | wide but low and flat, puck 10 px |
| c4 | 4 | 2 | 60 | -8 | 21 px | 230 / 166 | 108 px, v 340 | 0 of 3 | 36 m | pawn hides everything in front |
| d1 | 6 | 3 | 70 | -18 | 16 px | 165 / 125 | 89 px, v 147 | 1 of 3 | 9.0 m | like F1, lower |
| d2 | 6 | 4 | 70 | -22 | 16 px | 164 / 125 | 91 px, v 65 | 1 of 3 | 4.9 m | net near the top edge |
| d3 | 8 | 4 | 80 | -20 | 12 px | 121 / 95 | 72 px, v 186 | 1 of 3 | 6.5 m | middle ground |
| d4 | 8 | 3 | 90 | -18 | 10 px | 103 / 80 | 61 px, v 263 | 3 of 3 | 12 m | wide, puck smallest |
| d6 | 4 | 3 | 70 | -16 | 17 px | 189 / 138 | 91 px, v 200 | 0 of 3 | 6.0 m | close, flank blind |

(c2, c5, c6, c7, d5 are in the sheets but not tabulated; c2 = d8 h4 FOV 80 pitch -14, c5 = d6 h4 FOV 80 pitch -14, c6 = d8 h4 FOV 70 pitch -12, c7 = d6 h2 FOV 70 pitch -12, d5 = d6 h4 FOV 80 pitch -20.)

Screenshots: `img/cam-a-default-far.png` and `img/cam-a-default-p15.png` (F1; "far" has the puck 3 m ahead and 1 m to the side, "p15" has it 15 m ahead), `img/cam-b-wide-far.png` and `img/cam-b-wide-p15.png` (W),
`img/cam-sweep-sheet-1.png` (c1-c7, d1), `img/cam-sweep-sheet-2.png` (d1-d6), `img/cam-sweep-sheet-3.png` (F1, W, T, H, each far and 15 m).
Honest reading of the F1 shots: the puck at 15 m is a 16 px dark dot, readable because the ice is plain; the two nearest opponents are clear capsules; the far net is a pale
box on the far boards, 89 px wide, readable but low contrast against pale ice and boards (hence question 2). About a quarter of the frame above the far boards is black void; a lower
pitch would use it but pushes the net off the top.

### Lag (0, 0.1, 0.2 s) and pitch

Lag moves the camera back by about `speed * lag` and swings it behind a turning pawn. Measured with a pawn at 8 m/s making a 180 degree turn at 5 rad/s (38 frames),
frame-based lag as in section 6 (N sim frames = time constant N/60 s), camera F1:

| `lag_frames` | about | extra distance behind at 8 m/s | max camera yaw error in the U-turn |
|---|---|---|---|
| 0 | 0 s | 0 m | 0 deg |
| 3 | 0.05 s | 0.34 m | 12 deg |
| 6 | 0.1 s | 0.74 m | 26 deg |
| 12 | 0.2 s | 1.53 m | 53 deg |

At 12 frames the camera is 1.5 m further back and trails a quick turn by 53 degrees, which is more than the F1 flank margins absorb. Recommended default 4 frames
(about 0.5 m and about 18 deg by interpolation; 3 and 6 were measured, 4 was not), wide option 6. Pitch: shallow pitches (-4 to -12) waste the upper third on black void and shrink the
ice in front; steep ones (-22 and below) clip the far net. The look-at rule (ground point 5 m ahead of the pawn, which gives -17.7 deg for F1) keeps the net inside the frame
at 6 m distance and the puck 15 m ahead near the middle of the screen.

### Recommended `game.camera`

| Key | Default | Wide option | Note |
|---|---|---|---|
| `mode` | `third_person` | `third_person` | `arena` stays as the fixed camera |
| `distance_m` | 6 | 8 | |
| `height_m` | 3.5 | 4 | the sweep grid used 2, 3, 4; the F1 and W captures use 3.5 and 4 themselves |
| `fov_deg` | 70 (horizontal) | 85 | |
| `lag_frames` | 4 | 6 | sim frames of a first-order filter, see section 6 |
| `radar` | true | true | |

Not in the schema and needed by the recipe: **`look_ahead_m`** (5 default, 6 wide). If the contract is not extended, hard-code 5 in the camera component and list it as a constant.
Two gameplay consequences for the contract owner, no edits made here: (1) the puck must sit about 0.7 m to the side of the skater's axis, otherwise the pawn hides it for the first
6 m; (2) controls should be relative to the skater's heading, not to the lagged camera.

## 2. Far-net readability (60, 85, 110 m from the pawn)

Camera F1 throughout; the pawn moved so that the net is 60, 85 and 110 m away. `img/cam-net-sheet.png` (3 x 3: base, beacon, brighter frame; columns 60, 85, 110 m) and
`img/cam-net-marker-sheet.png` (the marker in PIE, crops of the top 330 px at 60, 85, 110 m).

| Option | 60 m | 85 m | 110 m | Clutter | Verdict |
|---|---|---|---|---|---|
| base (grey box, white posts) | pale box on the boards, 85 px wide | 60 px | 47 px | none | readable only because you know where to look |
| (a) beacon: 0.8 m x 9 m magenta emissive pillar on the net | pillar clearly above the boards, runs off the top edge | clear | clear, about 9 px wide, 100 px tall | a vertical bar at the far end; geometry in the world | reads at all distances; optional flag |
| (b) UMG marker: 22 px magenta diamond plus distance text at the net's projected position | clear | clear | clear (the text overlaps the net box at 110 m: put the text above the diamond) | two small HUD elements, clamped to the screen edge when the net is off screen | **ship this** |
| (c) brighter frame: 0.3 m yellow posts and bar | visible | marginal | not visible (the frame is 2 px) | none | does not solve the problem |

The marker is the only one that also answers "where is the net when it is behind me" (edge clamp; the clamp code is in the recipe, it was exercised only with the net in front). The beacon is cheap and works against the black void above the boards;
an earlier 0.4 m x 10 m beacon was a hairline at 110 m, hence the 0.8 m size. (Net pixel widths are projected from the pinhole model for a 4 m net at those distances; the screenshots agree by eye.)

## 3. Radar

UMG widget blueprint `WBP_Hud` (canvas panel root; `RadarBg` border 200 x 100 px, `RadarMid` centre-line border 2 x 100, `Dot0..Dot5` borders (player dot 12 px yellow, the other five 9 px red or blue), `PuckDot` 6 px white; the net marker lives in the same widget). It was created by the pawn blueprint's `BeginPlay` through the Graph DSL, drawn in PIE at
1920x1082 (`img/cam-radar-pie.png`, zoom `img/cam-radar-crop.png`) and updated every tick from the actors' world positions: radar x = (x + 60) / 120 * 200, y = (y + 30) / 60 * 100
(fixed orientation, +X to the right, whatever way the camera faces).

Proof of the per-tick update (log of the PIE run, pawn skating +X at 8 m/s, tick callback at 60 Hz): at frame 60 the player is at x = -7 m and the dot's render translation is (88.33, 58.33);
at frame 240 the player is at x = 9 m and it is (115.00, 58.33), which are the formula's values; the far-net marker followed the projection (827 px, 118 px) and its text read "49 m"
(58 - 9). Per-frame cost was not measured.

Recommendation: **200 x 100 px (2:1, the rink's ratio, 1.67 px per metre), bottom-right corner, 24 px margin**. Dots of 9-12 px read as skaters at once (a skater is 0.6 m = 1 px at true scale, so the dots are
deliberately oversized); the puck is a 6 px white dot. Bottom-right keeps clear of the far net and marker (top centre), the score (top centre) and the player's own skater (bottom centre).
Go to 240 x 120 if the player dot is not found at a glance in playtest (not tried). Caveat: the driver here was a Python tick; the same writes (`SetRenderTranslation` per dot) need a C++
widget (`NativeTick`), because the Graph DSL cannot author Widget Blueprint graphs (see Errors).

## 4. Puck visibility (0.12 m near-black cylinder on pale ice)

Camera F1, puck at 10, 25 and 40 m of horizontal distance from the camera (depth 11, 25, 39 m), 1080p crops 300 x 170 px at 1:1 in `img/cam-puck-sheet.png`.

| Variant | 10 m (30 px wide) | 25 m (13 px) | 40 m (8 px) |
|---|---|---|---|
| plain near-black | obvious | a clear dark dot | a dark speck, findable only if you know where to look; lost when moving |
| contact-shadow disc (0.6 m grey-blue disc under it) | reads as a puck with a shadow, obvious | a faint halo, no gain | invisible |
| emissive rim (0.34 m orange disc under, a 0.05 m ring) | a thin orange ring, clear | a single orange pixel | invisible |
| trail (6 orange spheres from 0.22 m to 0.07 m, 0.6 m apart) | far too heavy at 10 m (balls), but unmistakable | a bright streak, the strongest | still a streak about 8 px long |
| all three | trail dominates, shadow adds little | trail | trail |

These are still images; "lost when moving" is an inference from the 8 px size, not a measured motion test.
Recommendation for M0: plain near-black puck (Godot lesson: never grow it), **a trail only when speed exceeds about 8 m/s**, rendered as a ribbon no wider than the puck (spheres were a crude stand-in), orange,
and a 0.5 m contact-shadow disc at close range. Radar plus trail cover the far range; no ring.

## 5. Scale check: 120 m with six skaters

`img/cam-scale-sheet.png`: top-down (fov 90 from 90 m; six specks on a 7200 m2 sheet), then default and wide third-person views from mid-ice with the six skaters spread.

Honest read: **it feels empty.** 7200 m2 for six skaters is 1200 m2 each (an NHL rink in 3-on-3 overtime is about 260 m2 each, 4.5x denser; the NHL figure is from the nominal 61 x 26 m rink, not measured here); in the default view two to three skaters are in frame
and the other three are specks or behind the camera; the far boards are 15 s of skating away. The wide camera shows more but each skater is 100 px or less, and the 120 m length mostly shows empty ice
between the camera and the action. Without crowd or props the only depth cue is the ice lines (I added a red centre line, blue lines and a centre circle; without them it would be worse).
First two cartridge values to try: **`rink.length_m`** from 120 to 90 (and `rink.width_m` 60 to 45, same 2:1), which gives six skaters 4050 m2 each 675, and **`skater.speed`** from 8 to 11 m/s
(120 m in 11 s, 90 m in 8 s). Re-run the A3 balance sweep after either; shot speed 18 m/s stays above the new skater speed.

## 6. Determinism-friendly camera

The camera must read sim state and never write to it, and its pose should be a pure function of the sim frame so that a recorded run replays the same views.

Measured:

| Setup | Result |
|---|---|
| Spring arm with `bEnableCameraLag`, `CameraLagSpeed` 10, fixed frame rate 60, pawn moved 0.1333 m per frame | steady offset 0.667 m behind (6.67 m instead of 6.00); two runs identical in every logged value (200 frames each) |
| The same, engine at fixed 30 Hz, same 0.1333 m per frame | 0.43 m: same sim input, different camera. The arm uses `DeltaSeconds` (substepping on by default, max step 1/60) |
| Model of the arm (`VInterpTo`, substeps of 1/60) | 0.667 m at 60 Hz (matches the measurement), 0.744 m at 144 Hz for the same motion per second |
| Frame-based lag: arm set absolute, one filter step per sim frame, `alpha = 1 - exp(-1 / lag_frames)` on position and yaw | rows identical at 60 Hz and 30 Hz (`lag_frames` 6: 0.74 m, 26 deg), equal to the model (0.735 m) |

How to make `lag_frames` real: do not use the arm's own lag. Make the arm's location and rotation absolute (`SetAbsolute(true, true, false)`), then **in the sim step (60 Hz), not in a render tick**,
filter the pivot: `pivot += (skater_pos - pivot) * alpha; yaw += wrap(skater_yaw - yaw) * alpha`, and set the arm's world transform from `(pivot, yaw)` with the fixed arm pitch. If render-frame
smoothness matters, interpolate pivot and yaw between the previous and the current sim state with the sim accumulator's `Alpha` (the way pawns are interpolated); that interpolation is cosmetic. The camera component
and widgets may read the world; nothing in the sim may read the camera. A camera-relative stick would make recorded inputs depend on the camera: map the stick to the skater's heading, or to a camera yaw computed from sim state.
Not proven: packaged-build behaviour; the arm's own lag speed could not be varied in my harness (Errors 7).

## Errors and surprises (all of them)

1. **Editor crash on `StopPIE`.** The Python tick callback and a `builtins` dictionary still held references to PIE objects; at PIE shutdown the engine reported "This reference is preventing the old GameInstance from being GC'd" and exited (MCP then answered ECONNRESET and ECONNREFUSED). The editor restarted in 12 s (warm); the unsaved level and the scene had to be rebuilt (a 5 s script). Mitigation (not tested): `unregister_slate_post_tick_callback`, clear every global that holds PIE objects, `gc.collect()`, then `StopPIE`.
2. **`PythonScriptingTools.execute_python` returns only the last expression**, never `print`; all probes end with an expression such as `str([...])`.
3. **Python cannot reach the BP-created HUD directly**: `WidgetBlueprintLibrary` is not exposed, `get_widget_from_name` is not exposed, and a widget made with `unreal.new_object` has no widget tree. What worked: mark the widgets as variables (`UMGEditor.WidgetBlueprintEditorSubsystem.ToggleWidgetAsVariable`, needs `widgetBlueprint` and `widget`), create the HUD in the pawn blueprint's `BeginPlay` through the DSL, and find the instance as `<GameInstance path>.WBP_Hud_C_0` with `unreal.find_object`; variables are then properties, e.g. `hud.get_editor_property('Dot0')`. Calling a non-exposed Blueprint function from Python also works (`call_method`, see the recipe); it returned an initialised widget.
4. **The Graph DSL cannot author Widget Blueprint graphs** (`[unsupported-asset-class] "WidgetBlueprint" assets are not supported by the blueprint domain yet`); `BlueprintToolset.AddVariable` cannot create object-typed variables (primitives, a few structs, enums and structs only), so the pawn blueprint cannot store a reference to the HUD. The DSL has no `let`; use `(bind name (node ...))`. A `bind` in one event is not visible in another.
5. `unreal.Rotator(a, b, c)` takes **(roll, pitch, yaw)**: my first lag harness passed `(0, yaw, 0)` and tilted the pawn. Use keyword arguments.
6. **Stale component references**: components fetched with `get_component_by_class` before the first tick returned zero transforms and ignored `set_editor_property`; fetch them inside the tick callback each time.
7. **I could not vary the arm's own `CameraLagSpeed` in PIE**: `set_editor_property('camera_lag_speed', 3)` read back as 10 during the run, so only speed 10 was measured for the engine arm (the 3, 6, 12 frame sweep used my own frame-based lag). Rotation lag (`bEnableCameraRotationLag`) did not follow a teleported pawn's yaw in my harness (error stayed at 179 deg for 200 frames); not resolved, replaced by the frame-based yaw filter.
8. After one PIE session `unreal.EditorAssetLibrary.does_asset_exist` and `list_assets('/Game')` returned False/empty and `AssetToolset.SaveAsset` answered "Asset does not exist" while `load_asset` still worked and the files were on disk; not resolved (saves made earlier were fine). The map itself was never saved, only the assets.
9. Git Bash rewrote `/Game/...` arguments into `C:/Program Files/Git/Game/...` for a `node` child process; set `MSYS_NO_PATHCONV=1` (already in `tools/mcp/README.md`).
10. `unreal.WidgetLayoutLibrary.project_world_location_to_widget_position` returns the `Vector2D` (not a tuple) and returns `None` when the point cannot be projected; handle `None`.
11. Colour: the offscreen `Capture` in mode `Lit` came out lighter than the PIE viewport (PIE adds auto exposure and a vignette); decisions here use relative contrast within one image. `Capture` does not render UMG; the HUD needs `CaptureGameViewport` in PIE.
12. Quantising the screenshots to 32 colours turned the blue to grey; 256 colours at 1920x1080 is 65-80 KB.
13. A PIE-driven test left the pawn rotated 180 degrees by the previous harness; the next run looked broken until the pawn's rotation was reset. Reset position and rotation at the start of every PIE harness.

## Is the recipe agent-executable without a human?

Yes for everything that was exercised: project creation, level, materials, camera pawn, widgets, PIE, screenshots and logging were all done through MCP calls and `execute_python`; no dialog or click was needed.
Open items for the jam builder, none of which needs a human: the C++ radar/marker widget driver (the Python tick proves the layout and the numbers, not the shipping path), the PIE-teardown mitigation (Errors 1) and the arm's own lag speed (Errors 7, irrelevant once `lag_frames` is frame-based).
