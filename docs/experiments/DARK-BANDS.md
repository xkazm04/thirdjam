# Dark bands: figures readable in the side bands of the night rink (lane A43, 2026-11-12, device A)

Three lanes reported the same fault: figures in the dark side bands are black silhouettes and the team colours read 1 of 5 at 24 m (TACKLING.md section 5,
CUTS.md bug 2, A30's strips). This lane parked the human and an opponent in the band, measured them at 10 / 24 / 40 m on `humanoid_b1` and `generated_b2`,
tried the five levers of the brief as runtime parameters and dynamic instances only, and picked. Stills in `docs/experiments/img/dark-bands/`, eyes JSON in
`docs/experiments/eyes/a43-*.json`. No master of B's was saved; the scripts (`freeze.py`, `shot.py`, `var.py`, `luma.py`, `fig.py`) lived in the lane's scratch folder
and every value they set is in this file.

## 0. The short version

1. **On main fef6630 the fault is already gone for the team colours.** B15's star dome and sky-light change lifted the band from 43-53 luma (B13) to 82-96, SK3 put
   `RimNight` 1.3 on B's jersey master, and the shipped figure is `humanoid_b1` (R-0030). In the band at 24 m the eyes read **team colours 4-5 of 5 on every still
   of this lane**, baseline included (10 m 5, 24 m 4-5, 40 m 4-5). The 1-of-5 the other lanes saw came from the toy figure drawing the engine's default grid
   material (R-0030 point 2, B16's warning) on the pre-dome band.
2. **What is still weak is the blue jersey**, not the orange: at 24 m it is a dark navy (rgb 43, 48, 121; luma 46-52) on a lighter blue band (59, 90, 150; luma 88):
   CIE76 dE 21 against the band, against 75 for the orange. It reads by hue and by the ink line, not by luma.
3. **Recommendation, B's masters:** jersey `Emissive` 2.0 -> **2.5** on the night figures (`M_SkaterTB1Jersey` / `MI_SkaterTB1_JerseyDeco`, `M_SkaterTB2` on
   `generated_b2`): the blue jersey steps up one ramp band (lit pixels 95 -> 98 luma, rgb 95, 94, 108 -> 88, 98, 123; the early pairing 46 -> 64), the orange keeps its hue;
   eyes 24 m team colours 5 ("royal blue and warm orange"). Everything else stays: `MI_Toon_Blue` / `MI_Toon_Orange` as B15 left them (Emissive 1.5, RimStrength 0.6;
   A's figures set 2.2 / 1.3 on their own MIDs), `MI_Ice_Night` `IceAmbient` 0.9, `M_PP_Outline` ink (0.012, 0.012, 0.028), `RimNight` 1.3.
4. **Optional, A's runtime:** a shadowless warm point light on **lighting channel 1 only**, attached to the human's pawn 1.8 m up and 0.6 m behind, **25 cd**, radius 6 m,
   with the skater meshes on channels 0 + 1: the blue jersey's lit pixels go 98 -> 104 luma, the helmet and shoulders get a highlight, the band ice beside the figure
   moves +1 luma (Lumen bounce only), cost not measurable (the `Lights` row 0.15-0.18 ms with and without it). Above 40 cd the blue turns pastel cyan and the team
   read gets worse; at 100 cd the jersey is white (232). Not implemented in `LookTierSwitcher` in this lane: the gain over the master change is small and the Director should
   look at `sheet-b1-24m.jpg` (C1 against C2) before it ships.
5. **Rejected with numbers:** `RimStrength` / `RimNight` 1.2-1.5 (no change, +-1 luma, dE 21.5 -> 20.7); rim 3.0 with `RimPower` 1 (works, but the cyan `RimColour` pulls the
   blue toward the ice: dE 21 -> 9); the fill light on all channels (500 cd: the band under the figure goes to 247 luma); the ice `IceAmbient` floor (1.3: +0.2 luma, inside
   one ramp step; 2.0: the band 88 -> 139 and the pools flatten); the ink lift to (0.06, 0.08, 0.18) (the blue figure's edge becomes ice-coloured, dE 21 -> 7, and the orange
   loses saturation).
6. **What does not read at 24 m is the puck** (eyes 1-2 of 5 on most stills: luma 61-80 on a band of 88, "a tiny black cluster of 4 x 4 pixels") and with it the carrier
   (2-4). That is B-to-A-0023 point 6 (the halo flattens from the game camera) and not a figure lever; it is the open item for the Director.

## 1. Method

- Worktree a43 at main fef6630, `ThirdJamEditor Development` built with unity off (70.4 s, no other UBT); one editor on MCP 8024 from the worktree, the only
  `UnrealEditor.exe` on the machine from 20:25 to 20:48; stopped through `editor.ps1 -Stop` (own pid). `Rink.umap` and the rig were never saved.
- Frame: PIE floating 2560 x 1354, `hockey.test.figure-b1` then `-b2` (seed 1, `jam.ScriptHuman 1`), the sim frozen at t = 3.97 s (b2: 4.20 s) by global time dilation
  0.0001 **and the game mode's tick switched off** (`ApplyRender` would otherwise put a moved pawn back every tick). The human (blue 7, sim index 0) parked at (3, -15.5) m
  facing +x, the orange chaser (index 3) at (6, -14) m facing -x, the puck on the human's blade, the other four at the far end. The blue line at x = -15 and the red line at
  x = 0 are out of the box.
- Camera: a `CameraActor` placed in the editor world before PIE, set as the view target in PIE, 10 / 24 / 40 m behind the human along +x, pitch -17.7, FOV 70 (the
  geometry of A23 / A30 / B15). Capture: `HighResShot 2560x1354` (no HUD).
- Luma: Rec.709 from sRGB over 9 x 9 px boxes around projected world points (the jersey 0.95 m up the figure's axis, the band 1.5 m beside the human, a pool at (12, -7) m).
  Colour contrast: CIE76 dE in Lab between the jersey box and the band box. Second metric, pose-insensitive: inside a window of 2600 / distance px around the torso, the
  **lit figure pixels** (dE > 12 from the window's border median and luma > 30: no ink, no pants, no shadow): count, mean rgb, luma, dE against the ice (`fig.py`).
- Eyes (R-0024 point 2, advice not instructions): `readability` on `gemini-3.8-flash --no-think` at the still's distance; 15 calls, about 4 cents.
- Pose drift: B's animation blueprint keeps ticking under time dilation, so over the session the parked figures slid into other poses (stride at 20:31, a crouch from
  20:39). Point boxes are therefore only compared **inside one pairing** (the early set 20:31-20:38 and the paired set 20:42-20:45, each with its own baseline); the lit-pixel
  metric and the eyes are pose-tolerant.
- Cost: `ProfileGPU` x3 on the frozen frame, graphics-queue tree, `r.ProfileGPU.ShowUI 0`, read from the editor log.

## 2. The band today (ice survey from the 40 m camera, figures parked away, 9 x 9 px luma)

| y (m) | x = -21 | -18 | -15 (blue line) | -12 | -9 | -6 | -3 | 0 (red line) | 3 | 6 | 9 | 12 | 15 (blue line) | 18 | 21 | 24 | 27 | 30 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| -7 (pools) | 78 | 86 | 44 | 185 | 186 | 128 | 93 | 16 | 92 | 124 | 192 | 194 | 92 | 115 | 88 | 85 | 118 | 164 |
| -11 | 84 | 77 | 7 | 83 | 94 | 94 | 86 | 16 | 93 | 87 | 88 | 95 | 39 | 87 | 88 | 88 | 90 | 89 |
| -15 (the band) | 80 | 82 | 7 | 83 | 95 | 86 | 95 | 16 | 122 | 82 | 95 | 88 | 42 | 89 | 91 | 91 | 87 | 87 |
| -19 | 77 | 85 | 7 | 85 | 91 | 85 | 93 | 17 | 90 | 94 | 93 | 92 | 47 | 89 | 90 | 87 | 93 | 92 |
| -21 (board foot) | 80 | 84 | 13 | 86 | 86 | 91 | 90 | 23 | 120 | 105 | 115 | 110 | 63 | 141 | 138 | 137 | 140 | 138 |

B13 measured the y = -15 band at 43-53 before the dome; it is 77-96 now, the pools 185-194. There is no darker band left to stand in: the parked spot (band 82-96 at
the three cameras) is the worst case the rink has today. Still: `a43-survey.jpg`.

## 3. Luma table (9 x 9 px boxes; blue = the human's jersey, orange = the opponent's; band = the ice beside the human; dE = jersey box against band box)

Early pairing (stride pose, 20:31-20:38), `humanoid_b1`:

| Variant | 10 m blue / orange / band | 24 m blue / orange / band | 24 m dE blue / orange | 40 m blue / orange / band |
|---|---|---|---|---|
| base (jersey Emissive 2.0, RimNight 1.3, helmet 1.5 / 0.6) | 49 / 60 / 82 | 52 / 61 / 88 | 21.5 / 74.8 | 37 / 89 / 87 |
| RimNight + RimStrength 1.5 (jersey) | 46 / 60 / 82 | 53 / 62 / 88 | 20.7 / 75.8 | 37 / 90 / 87 |
| jersey Emissive 1.5 | 46 / 60 / 82 | 52 / 57 / 88 | 20.8 / 72.1 | 36 / 78 / 87 |
| jersey Emissive 2.5 | 51 / 128 / 82 | 70 / 97 / 88 | 15.6 / 90.1 | 67 / 116 / 87 |
| jersey Emissive 3.0 | 54 / 129 / 83 | 76 / 99 / 88 | 14.2 / 91.1 | 73 / 117 / 87 |
| rim 3.0 on every slot, RimPower 1 | | 78 / 109 / 88 | 9.3 / 79.4 | |
| fill 500 cd, all channels, 1.8 m over the pawn | 235 / 192 / **247** | 236 / 175 / **247** | 8.0 / 73.6 | 238 / 195 / **246** |
| fill 1500 cd, all channels | 249 / 213 / **254** | 249 / 205 / **254** | 3.6 / 72.3 | 250 / 218 / **254** |
| fill 300 cd, channel 1 only | 246 / 115 / 83 (front of the figure 69) | 249 / 162 / 88 (front 114) | 70.2 / 47.9 | 231 / 175 / 87 (front 111) |
| fill 100 cd, channel 1 only | 227 / 117 / 83 | 232 / 162 / 88 | 62.7 / 47.1 | 201 / 175 / 87 |
| `IceAmbient` 1.3 | 44 / 65 / 83 | 54 / 67 / 88 | 22.2 / 74.1 | 41 / 97 / 87 |

The dE of the blue jersey falls when the jersey gets lighter because the band is a lighter blue: the lit blue moves toward the ice in Lab while gaining luma
contrast against the ink line and the pants. Read the dE together with the luma, and with the eyes.

Paired set (crouch pose, 20:42-20:45), `humanoid_b1`, lit figure pixels (count / mean rgb / luma / dE against the ice around the figure):

| Variant | 24 m blue | 24 m orange | 40 m blue | 40 m orange | 10 m blue |
|---|---|---|---|---|---|
| base | 6450 px, (95, 94, 108), 95.5, dE 30 | 5579, (95, 74, 82), 79.4, dE 38 | 2493, (92, 94, 112), 94.6, dE 27 | 2100, (96, 72, 82), 78.2, dE 38 | 30158, (108, 109, 120), 109.7 |
| **C1: jersey Emissive 2.5** | 7637, (88, 98, 123), 97.7, dE 23 | 6244, (97, 84, 95), 87.4, dE 35 | 2898, (87, 98, 128), 98.1, dE 21 | 2450, (101, 85, 97), 89.2, dE 35 | 35474, (98, 113, 137), 111.5 |
| C2: C1 + fill 25 cd ch1 | 8278, (88, 106, 135), 104.1, dE 22 | 6892, (97, 92, 108), 94.2, dE 31 | 3035, (86, 104, 140), 103.1, dE 19 | 2612, (101, 92, 109), 95.4, dE 32 | 39258, (97, 120, 148), 116.8 |
| C3: C1 + fill 60 cd ch1 | 8897, (98, 117, 141), 114.5, dE 25 | 7235, (105, 102, 114), 103.5, dE 33 | 3077, (98, 119, 147), 116.3, dE 23 | 2625, (110, 103, 114), 105.4, dE 34 | 41333, (108, 131, 153), 127.5 |

Point boxes of the same set at 24 m (blue / orange / band): base 73 / 105 / 88, C1 80 / 103 / 88, C2 139 / 108 / 88, C3 165 / 169 / 88; at 40 m: base 45 / 113 / 87, C1 64 / 121 / 87,
C2 135 / 124 / 87, C3 153 / 158 / 87. Single points and the ink reset: `a43-b1-fillc15-24m` (15 cd: blue 146), `a43-b1-fillc40-24m` (40 cd: 203), `a43-b1-fillc800-24m`
(800 cd: 254 and the front ice 146), `a43-b1-ice20c-24m` (`IceAmbient` 2.0: band 139, blue 87, orange 97, pools 193), `a43-b1-ink-24m` (ink lift: blue 96 but dE 7, orange grey 111, 98, 105).

`generated_b2` (its own pose, 20:45; the MID is `MID_M_SkaterTB2_0`, RimNight 1.3, Emissive 0.5, WarmFill 1.25, JerseyLum 0.08):

| Variant | 10 m blue / orange / band | 24 m blue / orange / band | 40 m blue / orange / band |
|---|---|---|---|
| base | 56 / 76 / 82 | 30 / 103 / 87 | 27 / 104 / 87 |
| C2 (jersey Emissive 2.5 + fill 25 cd ch1) | 96 / 14 (the box on the black pants) / 82 | 117 / 78 / 87 | 113 / 114 / 86 |

On b2 the jersey Emissive 2.5 on a master that fills through `WarmFill` is a bigger step (blue 30 -> 117 at 24 m) and reads as "bright saturated cyan-blue" to the eyes;
B should scale it to that master (about 1.0-1.5 on `M_SkaterTB2`, to be measured by B against `sheet-b2.jpg`).

## 4. Eyes verdicts (`readability`, flash, no-think; puck / carrier / team colours / far net / zone strips, overall)

| Still | Distance | Scores | What it said about the colours |
|---|---|---|---|
| `a43-b1-base-10m` | 10 | 4 / 5 / **5** / 4 / 5, 4 | "deep electric blue ... bright warm orange" |
| `a43-b1-base-24m` | 24 | 1 / 2 / **5** / 4 / 5, 3 | "deep blue and warm orange jerseys are instantly distinguishable"; the puck "not visible" |
| `a43-b1-base-40m` | 40 | 1 / 3 / **4** / 5 / 5, 2 | "figures very small (about 35 px tall)" |
| `a43-b1-base2-24m` | 24 | 4 / 5 / **4** / - / -, 4 | "deep royal blue ... warm orange" (the puck and its guide line were in view on this frame) |
| `a43-b1-c1-24m` (jersey 2.5) | 24 | 2 / 4 / **5** / - / -, 4 | "royal blue skater and warm orange opponent contrast sharply against each other and against the blue ice" |
| `a43-b1-c2-24m` (+ fill 25) | 24 | 4 / 5 / **4** / - / -, 4 | "vibrant bright blue and warm orange" |
| `a43-b1-c3-24m` (+ fill 60) | 24 | 1 / 4 / **5** / - / -, 4 | "bright cyan / deep blue" |
| `a43-b1-c2-10m` | 10 | 1 / 4 / **5** / - / -, 4 | "bright cyan-blue and warm orange" |
| `a43-b1-c2-40m` | 40 | 2 / 4 / **5** / - / -, 3 | "vivid royal blue and bright warm orange" |
| `a43-b2-base-24m` | 24 | 2 / 3 / **5** / - / -, 4 | "vibrant solid blue and bright orange" |
| `a43-b2-c2-24m` | 24 | 1 / 3 / **5** / - / -, 3 | "bright saturated cyan-blue and warm orange" |
| `a43-b2-c2-40m` | 40 | 1 / 1 / **5** / - / -, 3 | "cyan / bright blue and warm orange" |

The team colours never go below 4 on this main. The puck is the element that fails (1-2 at 24 and 40 m on most frames; 4 only when the aim line and the glow disc
sat in view) and takes the carrier down with it. Gemini's "cyan" from C2 / C3 upward is the warm fill desaturating the navy: the sign to stop at 25 cd.

## 5. Cost (ProfileGPU, graphics queue, 2560 x 1354 PIE on the 4090, three samples each; ms)

| State | Frame | Scene | BasePass | ShadowDepths | Deferred lighting (`Lights` row) | PostProcessing |
|---|---|---|---|---|---|---|
| base (early pairing) | 4.09 / 4.56 | 3.08 / 3.44 | 0.29 / 0.35 | 0.27 / 0.31 | 0.27 / 0.31 (0.18) | 1.22 / 1.33 |
| fill 500 cd, all channels | 4.15 / 4.11 / 4.22 | 3.06 / 3.07 / 3.16 | 0.28 | 0.27 | 0.27 (0.15-0.17) | 1.22-1.24 |
| fill 300 cd, channel 1 | 4.70 / 3.89 / 5.69 | 3.66 / 2.88 / 4.71 | 0.26 | 0.24-0.80 | 0.23-0.26 (0.14-0.15) | 1.16-1.45 |
| C2 (jersey 2.5 + fill 25 cd ch1) | 4.42 / 5.08 / 4.19 | 3.41 / 4.12 / 3.15 | 0.26-0.48 | 0.24-0.25 | 0.25 (0.15) | 1.16-2.41 |

The whole frame wanders +-0.8 ms between samples of the same state (TSR and the post chain), so only the per-pass rows carry a signal: one unshadowed point light adds nothing
the `Lights` row can show (0.15-0.18 ms in every state; B13's two `Fill_L2_Night` spots are 0.024 ms of it), the material constant costs nothing, ToonRamp 0.056 and Outline
0.043 are unchanged. Nothing in this lane is over 0.3 ms; the two all-channel fills are rejected for lighting the ice, not for cost.

## 6. Recommendation (exact values)

B's masters (night tier):

| Asset | Parameter | Today | Recommended | Why |
|---|---|---|---|---|
| `M_SkaterTB1Jersey` (through `MI_SkaterTB1_JerseyDeco`; the BP's MID starts from the instance) | `Emissive` | 2.0 | **2.5** | the blue jersey steps up one ramp band in the band (lit pixels 95 -> 98 luma, point box 46 -> 64 early / 73 -> 80 paired, 40 m 37 -> 67 early); the orange keeps its hue (173, 83, 18); eyes 5 |
| `M_SkaterTB2` (`generated_b2`) | `Emissive` | 0.5 (with `WarmFill` 1.25) | **about 1.0-1.5**, B measures | 2.5 took the b2 blue from 30 to 117 at 24 m and the eyes called it cyan: too far on this master |
| `M_SkaterTB1Jersey`, `M_SkaterTB2`, `M_SkaterGen` | `RimNight` | 1.3 | **1.3** (keep) | 1.5 is invisible (+-1 luma); 3.0 with RimPower 1 shows but pulls the blue to the ice colour (dE 21 -> 9) |
| `MI_SkaterTB1_Helmet` / `_Trim` / `MI_SkaterTB2_Trim` (M_Toon) | `Emissive`, `RimStrength` | 1.5, 0.6 | keep | the helmet did not need more; the fill light is what lit it |
| `MI_Toon_Blue` / `MI_Toon_Orange` | `Emissive`, `RimStrength` | 1.5, 0.6 | **keep** | A's own figures set Emissive 2.2 and RimStrength 1.3 on their MIDs (ToySkaterVisual / SkeletalSkaterVisual); nothing measured here asks for a change |
| `MI_Ice_Night` | `IceAmbient` | 0.9 | **keep** | 1.3 is inside one ice ramp step (+0.2 luma); 2.0 lifts the band to 139 and flattens the painted pools |
| `M_PP_Outline` / `MI_PP_Outline` | `InkColour` | (0.012, 0.012, 0.028) | **keep** | the uniform lift to (0.06, 0.08, 0.18) makes the blue figure's edge ice-coloured (dE 21 -> 7) and greys the orange; a band-aware ink would be a master change for a gain the eyes do not need |

A's runtime (optional, Director's call after `sheet-b1-24m.jpg`): in `ULookTierSwitcher`, on `l2_night` only, one `UPointLightComponent` attached to the human's
`AJamSkater` at (-60, 0, 180) cm: 25 cd, attenuation 600 cm, colour (1.0, 0.92, 0.78), no shadows, `LightingChannels` channel 1 only; the skater visuals' mesh components set to
channels 0 + 1 (`USkeletalMeshComponent::SetLightingChannels(true, true, false)`, which for B's figures means the BP's mesh, reachable through `GetVisualObject`). Measured:
blue lit pixels 98 -> 104 luma, helmet and shoulder highlight, band +1 luma (Lumen bounce), cost below the noise. Do not go above 40 cd.

Not for this lane, for the Director: the puck at 24 m in the band (B-to-A-0023 point 6: a thicker halo disc or a vertical glow card in the puck kit); and the pose drift
under time dilation, which any future frozen-frame lane should know (freeze, then measure within a minute, or compare only inside one pairing).

## 7. Handoff draft for B (A-to-B, "the blue jersey in the dark bands")

> B: lane A43 parked the human and an opponent in the side band of `LR_L2_Night` (y = -15 m, the darkest ice the rink has since your dome: 82-96 luma) and measured
> `humanoid_b1` and `generated_b2` at 10 / 24 / 40 m (`docs/experiments/DARK-BANDS.md`, stills `docs/experiments/img/dark-bands/`). Since B15 and SK3 the team colours read
> 4-5 of 5 at every distance (eyes, 12 stills); the weak figure is the blue jersey, a navy (43, 48, 121) on a lighter blue band (59, 90, 150), dE 21 against 75 for the orange.
> One master value closes most of it: **`Emissive` 2.0 -> 2.5 on `M_SkaterTB1Jersey` / `MI_SkaterTB1_JerseyDeco`** (night gate only; the blue steps up one ramp band, the
> orange keeps its hue, eyes 5). On `M_SkaterTB2` the same 2.5 overshoots (the blue turns cyan at 24 m): try about 1.0-1.5 against `sheet-b2.jpg`. Keep `RimNight` 1.3 (1.5 is
> invisible, 3.0 pulls the blue to the ice colour), keep `MI_Ice_Night` `IceAmbient` 0.9 and the outline ink; the fill-light route only works on lighting channel 1 at 25 cd
> (all-channel fills light the band to 247, as yours did), and A keeps that as an optional runtime lever. What does not read at 24 m now is the puck (1-2 of 5): your
> B-to-A-0023 point 6 (thicker halo disc, vertical glow card) is the next lever for the band, not the figures.

## 8. Stills (`docs/experiments/img/dark-bands/`, 2560 x 1354 JPEG from the PNG captures)

- Baselines: `a43-b1-base-{10,24,40}m.jpg` (stride pose), `a43-b1-base2-{10,24,40}m.jpg` (crouch pose, the paired set), `a43-b2-base-{10,24,40}m.jpg`.
- Candidates: `a43-b1-c1-{10,24,40}m.jpg` (jersey Emissive 2.5), `a43-b1-c2-{10,24,40}m.jpg` (+ fill 25 cd ch1), `a43-b1-c3-{10,24,40}m.jpg` (+ fill 60), `a43-b2-c2-{10,24,40}m.jpg`.
- Single tests at 24 m: `a43-b1-rim15-24m`, `a43-b1-rim30p1-24m`, `a43-b1-emis{15,25,30}-24m`, `a43-b1-fill500-24m`, `a43-b1-fillc{15,40,100,300,800}-24m`, `a43-b1-ice13-24m`,
  `a43-b1-ice20c-24m`, `a43-b1-ink-24m`, `a43-b1-c1b-24m`; the ice survey `a43-survey.jpg`.
- Sheets: `sheet-b1-24m.jpg` (base, C1, C2, C3, the two rejections), `sheet-b1-distances.jpg` (base and C1 at the three distances), `sheet-b2.jpg`, and the zoomed crops
  `crops-b1-paired.jpg`, `crops-b1-early.jpg`, `crops-b2-paired.jpg`.

## 9. Errors and pitfalls met

1. `GameplayStatics.project_world_to_screen` returns a `Vector2D` or `None` in Python, not a (bool, vector) pair.
2. `unreal.KismetMaterialLibrary` is not exposed; a post-process MID is made through any mesh component's `create_dynamic_material_instance(slot, parent)` and the slot
   set back; the PPV's `weighted_blendables.array` is copied, edited and set back as a whole.
3. A moved pawn is put back by `ApplyRender` every tick: the game mode's tick must be off, not only time dilation.
4. Under time dilation B's animation blueprint still advances: the pose drifts over minutes (see Method). The first ink script also lifted `OutlineColour`; the reset
   put only `InkColour` back, so `a43-b1-c1-24m` of the first run and `a43-b1-ice20c-24m` carried a lifted outline colour until `a43-b1-c1b-24m` restored (0.01, 0.012, 0.03).
5. A point light with channel 1 only still lights the ice a little through Lumen's bounce off the lit figure (300 cd: the ice in front of the figure +25 luma; 25 cd: +1).
6. The toon ramp quantises: Emissive 1.5 and 2.0 land on the same band of the blue jersey, 2.5 and 3.0 on the next; `IceAmbient` 0.9 and 1.3 on the same ice step.
7. `get_components_by_class(MeshComponent)` on B's visual actor returns components named `TRASH_SkeletalMeshComponent_0` (B's BP rebuilds its mesh at BeginPlay); their
   MIDs are the live ones, and the lighting-channel change did reach the drawn mesh (the channel-1 light lit the figure).
