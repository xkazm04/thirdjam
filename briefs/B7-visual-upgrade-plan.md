# B7: the visual upgrade plan (lane V1, device B, written at H5, 2026-11-12 about 11:05-11:20)

Planning only: no editor was started, nothing was measured for this file, no credits were spent. Every number below is quoted from a named document
or still, or is marked **guess**. The three test scripts it names are written but **not run**: `C:\Users\kazda\jam\skater-lab\v1\tests\`.

The operator asked for "the scenario in which we upgrade to a higher visual tier tomorrow, so we have a clear plan based on tested directions".
This file gives five candidate directions (section 2), the experiments that decide them tonight (section 3), the timeline from tonight to the
H72 freeze with its decision points (section 4), and what the documents cannot tell us (section 5). Section 1 says where the look stands and how
much frame budget is left, because that number constrains everything else.

## 1. Where the look stands at H5, and the budget it has left

**What exists** (`status/B1-report.md`, `status/B2-report.md`, stills in `devlog/shots/`): P2 painted cel at T2 night. There are five masters
and seventeen instances. Toon ramp and ink outline are post passes by custom stencil. `LR_L2_Night` holds the PPV, the sky light, four
shadowed 20 000 cd spots at z 25 m, a dark room, and 289 sound-stage actors built from basic shapes. Everything was checked on a
throwaway layout only. A's rink, skaters and puck are not in `Rink` yet.

**What the stills show** (I looked at all nine B1/B2 stills and the P2-T2, P2-T3, P3-T3 and P4-T3 references):

| Weakness | Cause (mine, from the recipe values; treat as a hypothesis until test T1) |
|---|---|
| Ice between the pools is pure black (B1 stills). In B2's stills the same zone is dark navy with blotchy scuff, cause not determined. | Pools are 29 m across on a 60 m wide rink (recipe section 5), so everything at \|y\| > 14.5 m and \|x\| > 59.5 m is lit only by a 2.0 sky light that the ceiling occludes. The ramp then steps a tiny, noisy Lumen value. |
| The blue line blotches near the camera (B1 mid-ice still, bottom edge) | The line's base colour is constant, but its **roughness** still follows the scuff mask. The ramp divides by the diffuse colour (`sc / max(dc, 0.03)`). On a line with about 0.09 luminance, that divide amplifies specular differences about 10x more than on the ice, so the scuff pattern shows as stepped blotches. |
| The set dressing is basic shapes | Pass 1 by design. Chair, slate and benches are out of the gameplay frame (B2 report). |
| The far end is hard to read | The room behind the far boards is black. The net reads only as a red box against white boards. The beacon fixes the net (E3), but the far end has no shape. The P2-T2 reference has a painted sky behind the far boards. |
| Compared with the P2-T2 reference | The reference has lit blue ice everywhere, scratch lines, visible light shafts under each lamp and a painted backdrop. The build has none of these four, and they are most of the difference. |

**Frame budget: the L2 tier has almost no headroom.** This is the fact that shapes every direction below.

| Number | Value | Source |
|---|---|---|
| Gate | p99 frame under 8 ms, 1080p Shipping, A's RTX 4090, whole game | `docs/HOCKEY.md` 7 |
| Contract budget for lighting + post, `l2_night` | 4.5 ms | `contracts/look-tiers.md` |
| Measured, full P2 night look, throwaway with 143 actors | 4.31 ms median (repeat 3.80), standalone 4090; **no p99** | `docs/experiments/LOOK-P2-NIGHT.md` |
| Lumen part of that | 2.5-3 ms | same |
| Shadowed spots | about 0.6 ms (saving if off) | same, case c |
| Low profile `sg.ShadowQuality 1`, `sg.GlobalIlluminationQuality 1` | -1.0 to -1.3 ms | same, case d |
| Both post passes | 0.04 ms on the 4090, 0.08 ms on B | E3; B1 |
| One volumetric spot + height fog | +0.28-0.53 ms; turns far orange figures beige | E3 |
| Dressing, 289 actors, 48.8k triangles | about +0.3 ms on **B**, PIE, other editors running (noisy) | B2 |
| `LumenReflections` on B, PIE view at 1398x790, dry rough ice | **2.71 ms** of 3.62 ms deferred lighting | B1 profile |
| `RenderDeferredLighting` on the 4090 | 0.63 ms (reflections included) | E3 case c |

The L2 look has about **0.2-0.7 ms** left against its own 4.5 ms budget. That is before the dressing is counted on A and before anything
below is added. So every upgrade has to **pay for itself**. The levers we have measured are spot shadows (0.6 ms) and the GI quality profile (1.0-1.3 ms).
One lever is unmeasured and looks large on B: Lumen reflections on dry ice. Test T1 measures it.

Judge GPUs: nothing is measured. E3 guesses an RTX 3060 at 10-13 ms and a GTX 1660 at 20-25 ms for the full look. On B (NVIDIA N1X), PIE is
about 2x A's PIE number on the whole frame and about 5x on deferred lighting. B is a rough stand-in for a mid-range card at best (**guess**).

## 2. Directions (ranked by judge value per lane-hour, after paying for the frame)

Each direction is a different kind of step. **D0 is not an upgrade.** It is the floor that ships whatever happens, and it is built tonight.

### D0. The floor: P2 night with its known weaknesses fixed (always ships)

- **Changes:** three changes to `M_Ice`:
  - an `IceAmbient` emissive term so the dark ice reads dark navy, not black;
  - line roughness held constant (`LineFix`) or a lighter `LineBlue`;
  - `MI_Toon_NetMesh` darker (E3 already asks for this).

  Plus, if T1 shows it pays: the L2 PPV's reflection method set to screen space or none.
- **Judge value:** the first frame no longer looks unfinished. Puck and skaters are visible in the side bands.
- **Cost:** about 1 lane-hour after T1, one editor. **Frame:** expected under 0.05 ms (material ALU only, **guess**). The reflection change may *save* 0.3-2.7 ms (0.3 is A's bound, 2.7 is B's PIE).
- **Needs from A:** the answer to C-0002 open question 2. If the persistent `Rink` sky light stays, it adds to the rig's sky and the floor must be tuned
  in `Rink`, not on a throwaway. **Fallback:** sky light 3.5 (E3's own suggestion), or two unshadowed wide fill spots (both are cases in T1).

### D1. Stage light and atmosphere: "it is a film set" (rank 1)

- **Changes, all in `LR_L2_Night` plus three new materials under `/JamAssets/Look/`:**
  - additive light-shaft cones under the four spots (basic Cone, unlit additive two-sided material, depth fade);
  - a painted backdrop **flat** at each end wall: an unlit procedural night sky with a mountain ridge and stars, on a steel frame of three cubes. It is a theatre flat on a sound stage, and it gives the far end a shape;
  - a depth-haze post material (third blendable) that lifts the black with distance;
  - film grain, mild chromatic fringe and lens flare from the PPV's own settings.
- **Judge value, first 30 s:** highest of all five. Shafts from film lamps, a painted flat behind the far net and film grain say "director, film set" before
  anyone reads a menu. This is the P2-T2 reference's light shafts and painted backdrop, which the stills are missing.
- **Cost:** test 45 min + integration 1.5 h + taste tuning 1 h, so **about 3 lane-hours**, one editor. **Size:** under 0.1 MB (procedural, no textures).
- **Frame risk:**
  - Shafts: four full-height additive cones, translucency overdraw over much of the screen. **No measurement**; guess 0.1-0.3 ms on the 4090.
    The volumetric alternative is measured at 0.3-0.5 ms and washes out far figures, so cones are the cheaper and safer choice.
  - Haze post and grain/fringe: expected about 0.02-0.05 ms each, like the other post passes (**guess**).
  - Lens flare: unmeasured.
- **Needs from A:** nothing to build. The 4090 number comes from A's FrameBudget run on the next merged rig (handoff, section 4).
- **Fallback:** keep the backdrop flats (they cost almost nothing) and drop the shafts. Below that, drop the haze.

### D2. Ice surface quality (rank 2)

- **Changes in `M_Ice`, procedural only (we cannot paint textures):**
  - skate scratches: three families of long, slightly curved dashed lines, faded out past about 30 m so they do not shimmer under TSR;
  - the `LineFix` from D0;
  - `ScratchColour` and `Scratch` parameters, so the Designer's "ice is now glass" cut can drive `Scratch` and `Wet` (A drives the dynamic instance).

  Live skate trails (runtime marks behind each skater) need a per-skater component or a render target. That is A's code plus lane B3, and it is
  **not** in this direction.
- **Judge value:** medium. Scratched ice is the single most visible surface in every frame of the P2-T2 reference. Our ice is clean noise.
- **Cost:** about 2 lane-hours, shared with T1. **Frame:** ALU only on one material; under 0.05 ms (**guess**; T1 reads BasePass in the profiler).
  **Size:** 0. **Needs from A:** nothing. **Fallback:** `Scratch` 0, which is today's ice.

### D3. The L3 tier, built in P2 style, and switched live by Director's Call (rank 3)

`LR_L3_Neon` is empty and owned by B. Two variants compete for it:

- **D3a, "P2-T3 poster" (STYLE-PATHS T3, boost climax):**
  - warm saturated grade, bloom 1.4-1.6, warmer spots;
  - D1's shafts at higher intensity ("painted light rays");
  - glowing pickups (lane B3's props);
  - dry ice with **painted reflection streaks**: an emissive term in the ice under the trim, which the ramp turns into painted bands.

  No real reflections, so it is cheap. It is closest to what P2 is.
- **D3b, "wet neon" (HOCKEY-LADDER L3):**
  - cyan and magenta trim at Emissive 60;
  - `Wet` 1 (ice roughness 0.05) with Lumen reflections, falling back to screen-space reflections, then to none plus painted streaks;
  - a non-volumetric height fog (`Fog_L3_Neon`; E3 measured plain fog at 0.03 ms).

  Ray tracing is off in the project, so "RT reflections" is not an option unless A turns it on. Recommendation: do not.
- **Judge value:**
  - High when the judge sees it. The menu's "Choose your cut" lists the Boost Cut, which uses L3 (`docs/HOCKEY.md` 3, M3).
  - A Director's Call that switches the lights live is the theme made visible ("Director's Call: lights to neon"). `SetTier` on a Director's Call is
    already ruled (R-0001 point 5).
- **Cost:**
  - Test 60 min; rig 2 h; pickup glow coordination with B3 1 h; so **about 4 lane-hours**, one editor.
  - A: one FrameBudget run, and wiring a Director's Call option to the tier. **Size:** under 1 MB.
- **Frame risk:** budget 5.5 ms (C-0002), **not measured**. D3a costs about L2 plus bloom plus shafts. D3b adds reflections on a mirror-like plane. On the 4090,
  deferred lighting including reflections was 0.63 ms on rough ice; on wet ice it is unknown. On B, reflections were already 2.7 ms on rough ice, so
  D3b with Lumen reflections is the one option here that can fail on a judge's GPU (HOCKEY-LADDER: "the one tier that can embarrass the project").
- **Needs from A:**
  1. Per-tier material defaults. `Wet` and the streak strength must differ between L2 and L3, but a rig map holds no code and the masters read no
     collection. Proposal: B adds `MPC_Look` (scalars `Wet`, `Streak`, `Scratch`) to the masters, and `ULookTierSwitcher::SetTier` sets it from a small per-tier table
     (about ten lines of C++). Without it, D3 is limited to light, post and rig actors, which still covers D3a.
  2. A Designer or Director's Call preset that switches `l2_night` to `l3_neon`.
  3. The 4090 number.
- **Fallback:** the contract's own: "ships as `l2_night` with neon trim".

### D4. Hero set pieces from generated meshes (rank 4)

- **Changes:**
  - Meshy (or one Rodin try, since lane S3 is already testing that route) for four props: director's chair, slate, film light on a stand, and
    camera on a crane. Only the chair and slate are clearly worth it (B2's own advice).
  - A textured toon master `M_ToonTex` (base colour texture times tint, same rim and fill as `M_Toon`).
  - Placed at stencil 1 in `LR_L2_Night`, and **moved into the gameplay view**: chair and slate at about x ±74 near the long axis. Today they sit 41 m
    off-axis and no gameplay camera sees them (B2).

  The hero **skater** is the skater lab's question (`briefs/B-skater-lab.md`, lanes S1-S4 and B5). This plan does not duplicate it.
- **Judge value:** medium. The slate and chair are the most recognisable theme objects, but they sit at the far end at 30-100 m. Their silhouette matters more
  than their surface, and basic shapes already give a silhouette.
- **Cost:**
  - Generation runs with no editor, about 30 min of lab time. Meshy charges about 5 credits a preview and 10 a refine (`docs/experiments/MESHY.md`),
    so **about 60-90 credits** for four props with retries.
  - Import, material and placement: about 2.5 h of editor time, so **about 4 lane-hours**.
- **Size:** about 1 MB of mesh plus 0.7-5.6 MB of texture per prop at 1K-2K (MESHY.md figures). Four props come to about 10-25 MB of the 350 MB budget.
  **Frame:** about 4 x 20k triangles; negligible (**guess**).
- **Risks:**
  - A textured albedo under the ramp's divide is untested (specular noise may step, as on the blue line).
  - Generated props can carry brand-like marks; check every one.
- **Needs from A:** nothing (rig content is B's). **Fallback:** B2's basic-shape pieces moved into view.

### D5. A second look as a selectable cut: "P3-lite" clay in the empty `LR_L1_Day` slot (rank 5)

- **Changes:**
  - Fill `LR_L1_Day` with a warm, soft clay-miniature day look: STYLE-PATHS P3 at T1, "bright, cheerful".
  - Sun plus bright sky light, so the contract's `Sun_L1_Day` is needed.
  - A softer ramp: its own `MI_PP_ToonRamp_Day` with `StepSoftness` about 0.5 and the outline off.
  - Warm grade and a fixed-focus tilt-shift depth of field. The camera is always 6 m behind the skater, so a constant focal distance works without code.
  - With A's `MPC_Look` (D3 dependency 1): a procedural fingerprint normal and high roughness in `M_Toon`.

  Without the MPC it reads as "soft warm toy", not clay. P4 is not proposed: glossy PBR, reflections and stands are its whole cost, and "no crowd" was decided.
- **Judge value:** a second, visibly different look per cut is the theme ("your cut"). It only counts if the judge chooses that cut or a Director's Call.
- **Cost:** about 4-6 lane-hours plus A's MPC hook. **Size:** under 1 MB without textures.
- **Frame risk:**
  - Depth of field in UE5 desktop is the cinematic one: cost **not measured**, guess 0.3-0.8 ms.
  - Budget `l1_day` 4.0 ms, unmeasured. The sun's shadow over the 120 m rink is also unmeasured.
  - Depth of field blurs the far net by design (STYLE-PATHS P3). Cap it, or the cut fails readability.
- **Needs from A:** the MPC hook, a cartridge or preset that uses `l1_day`, and the 4090 number.
- **Fallback:** do not ship `l1_day`. This is the first direction to cut.

### Not proposed

- NvRTX or path tracing for play: off by guardrail. No 5.9 branch was found, and judges do not have a 4090 (`docs/NVRTX-NOTES.md`). It remains possible for trailer stills only.
- Turning hardware ray tracing on: device A's decision, and it fails the mid-range judge GPU (HOCKEY-LADDER L3).

## 3. Tests that decide (smallest experiment per direction, cheapest and most decisive first)

All editor tests run **one at a time in one editor slot**. While B3 (port 8001) and B5 (port 8002) hold both slots, the tests wait. A test takes a slot only
when a lane releases one: B3's box ends 14:30, B5's 14:45. Two editors are never exceeded. For numbers, the other editor must not be in PIE, and its viewport
must not be realtime (`briefs/B-machine-notes.md` rule 9). Whole-frame PIE noise on B is ±0.5-1 ms (E3 pitfall 10), so deltas under 0.5 ms are read from
`PerfToolset.GpuProfilerToolset.TriggerGPUProfile {"minInclusiveMs":0}`, not from `gpuMs`.

Common method (the scripts' headers give the exact calls):

1. **Build.** The script duplicates `LR_L2_Night` to `/JamAssets/_Scratch/V1Tn/`, opens the copy, and adds a throwaway rink, figures, pucks and a third-person camera
   (6 m back, 3.5 m up, pitch -17.65, FOV 70: `tools/skills/camera-third-person.md`).
2. **Measure each case.** Set the case, then:
   - `StartPIE`;
   - `GetFrameTimings` 60 times, median `gpuMs`;
   - `CaptureGameViewport` per shot;
   - one `TriggerGPUProfile`;
   - `StopPIE`.
3. **Pixel checks** with `ffmpeg ... crop=...,signalstats` on the saved PNGs.
4. **Clean up.** The cleanup action deletes the scratch folder. The real rig and `/JamAssets/Look/` are never written.

| # | Test (direction) | What is built | What is measured | Pass line | Box | Needs A's 4090? |
|---|---|---|---|---|---|---|
| **T1** | Ice floor, line fix, scratches, reflection lever (D0, D2) | rig copy, scratch `M_V1Ice_a` (recipe ice + `IceAmbient`, `LineFix`, `Scratch`), 2 optional fill spots; 13 cases: ambient 0.15 / 0.40, house fill, sky 3.5, line fix, lighter blue, scratches, reflections SSR / none / max roughness 0.2 | gpuMs per case; profile `LumenReflections`, `BasePass`; stills `near`, `side`, `line`, `far` | **Floor:** side-band ice YAVG 20-60 (0-255; baseline about 0-5) with pool centre YAVG within 5 % of baseline and bands still visible; dark-band puck at 24 m differs from its surround by 40+ luma levels. **Line:** YMAX-YMIN in the line crop 30 or less (baseline expected over 60). **Cost:** BasePass +0.05 ms or less. **Lever:** adopt SSR or none for L2 if it saves 0.5 ms or more on B with no visible change on dry ice. | 45 min | No for go/no-go. Yes later for the saving's size on the gate GPU. |
| **T2** | Stage light and atmosphere (D1) | rig copy, 4 shaft cones sized from the spots' cones, haze post as third blendable, 2 backdrop flats with frames; cases: shafts 0.6 / 1.2, haze / strong, backdrop, lens+grain, all | gpuMs; profile `Translucency`, haze `PostProcessMaterial`; stills `near` (camera inside a cone), `mid`, `far`, `end` | **Shafts:** visible from `mid` and `end`, no popping between inside and outside a cone, far orange figure's mean U/V shift under 10 levels (the volumetric fog failed this); `Translucency` 0.3 ms or less on B. **Haze:** pass 0.05 ms or less; far-end black lifted by +10 to +30 YAVG; beacon still brightest at the far end. **Backdrop:** YAVG 25-70, beacon 100+ levels above it. **All together:** +0.5 ms or less on B's profile. | 45 min | No (relative). Yes for the absolute at the next merge. |
| **T3** | L3 variants (D3) | rig copy restyled: cyan/magenta trim strips, recoloured spots, scratch `M_V1IceNeon_a` (Wet, painted `Streak`), 3 pickups, optional height fog; cases: L2 reference, neon + Lumen refl., neon + SSR, neon painted (no refl.), neon + fog, P2-T3 poster | gpuMs; profile `LumenReflections` / `ScreenSpaceReflections`, `Bloom`, fog; stills `near`, `mid`, `far` | **D3b goes ahead only if** neon+SSR or neon painted costs 1.0 ms or less over the L2 reference on B's profile, the puck halo and team colours stay separate (blue figure next to cyan trim is the case to watch), and the far beacon is findable. **D3a goes ahead if** poster costs 0.6 ms or less over the reference. Lumen-reflection wet ice is kept only if A's 4090 shows the L3 rig at 5.5 ms or less **and** B's profile is under +2 ms. | 60 min | Yes for the gate (handoff). B gives the mid-range proxy. |
| T4 | Hero prop (D4) | 2 Meshy props (chair, slate) by a lab lane, no editor; then import one into scratch with a scratch `M_ToonTex`, place at x 74 | credits, triangles, file size; one still at the in-game distance; ramp noise on the texture | reads as chair or slate at 30+ m at 1080p (the lead judges, with one still each with and without the prop); 20k triangles or fewer; 1K texture; under 5 MB on disk; no marks or letters; no ramp blotch | 30 min generation + 30 min editor | No |
| T5 | P3-lite day cut (D5) | rig copy with sun, sky 6, warm grade, a scratch ramp instance (`StepSoftness` 0.5, outline off), depth of field at 7 m | gpuMs; profile DOF passes; stills `near`, `far` | depth of field 0.5 ms or less on B; far net plus beacon still findable at 104 m; the operator says "clay-ish toy" and not "blurry" | 45 min | Yes for the DOF cost |

Scripts written (not run) for T1, T2 and T3: `C:\Users\kazda\jam\skater-lab\v1\tests\v1_t1_ice_floor.py`, `v1_t2_atmosphere.py`,
`v1_t3_neon.py`. T4 needs no script beyond the existing Meshy client and B5's import path. T5 can be derived from T2's script (same skeleton, different
cases) if D5 survives decision point DP2.

Order and why: T1 decides the floor that ships regardless, and it measures the one lever that could fund everything else. T2 is the best value per cost,
and its pieces fall back one by one. T3 is the most expensive and the riskiest, and it needs A's number anyway. T4 runs in parallel because generation
needs no editor. T5 runs last and only if the operator wants it.

## 4. The scenario: tonight to H72

Times are CET local, with H as hours after Thu 06:00. "Slot" means one of B's two editors.

| When | H | What | Who / slot |
|---|---|---|---|
| Thu 14:30-15:15 | H8.5-9.3 | **T1** in the first slot B3 or B5 releases. If neither releases by 15:30, the lead pauses whichever lane has the less critical next step. | V1 tests, slot 1 |
| Thu 15:15-16:00 | H9.3-10 | **T2** in the same slot | slot 1 |
| Thu, any time after 13:00 | - | **T4 generation** (no editor): chair and slate, at most 30 Meshy credits, inside the operator's credit approval | a lab lane |
| Thu 16:00-16:30 | H10-10.5 | **DP1** (lead, no operator needed): adopt the D0 floor, picking values by T1's pass lines; adopt the D1 pieces that passed. Integrate into the real masters and `LR_L2_Night`, each piece in its own outliner folder or blendable so it can be switched off alone. | lead, slot 1 |
| Thu 16:30-17:30 | H10.5-11.5 | integration, one PIE still set on the rig; push `dev-b` at the merge window with a handoff to A for the 4090 number | slot 1 |
| Thu 18:00-19:00 | H12-13 | **T3** (60 min); then T4's import and still (30 min) if B5 has freed its slot | slot 1 (+ slot 2) |
| Thu 20:00 | H14 | Epic livestream: the operator listens for Render Royalty, NVIDIA and Hyper3D criteria. They may reorder D3, D4 and D5. | operator |
| Thu 21:00 | H15 | **DP2, the operator** (15 minutes, on a contact sheet of T1-T4 stills with the numbers): (a) which L3 variant to build overnight, D3a or D3b; (b) the credit ceiling for D4; (c) D5 go or no-go for a T5 test in the morning; (d) taste vetoes on haze, grain and shafts. | operator |
| Thu 21:30-Fri 05:00 | H15.5-23 | overnight agents: build the chosen L3 in `LR_L3_Neon`; D4 props (if approved) generated, imported and placed; D2 scratches tuned. **Defaults if DP2 is not answered:** D3a (the cheaper one), no D4 spend beyond 30 credits, no D5. | slot 1 + slot 2 |
| Fri 06:00 | H24 | Gate 2 package (A). A's FrameBudget on `Rink` for `l2_night` and `l3_neon` (handoff B-to-A, below). | A |
| Fri 06:00-08:00 | H24-26 | **DP3** (lead, then the operator at wake-up). If `l2_night` is over 8 ms p99 on the 4090, or its lighting share is over 4.5 ms, switch off pieces in this order until it passes: lens flare, haze, shafts at half intensity, shafts, 2 of 4 spot shadows. If `l3_neon` is over 5.5 ms, drop reflections to SSR or none, then fog. | lead, operator |
| Fri 08:00-09:00 | H26-27 | **T5** (only if DP2 said go) | slot 1 |
| Fri 09:00-18:00 | H27-36 | second wave: D3 finish (pickup glow with B3, the Director's Call preset with A7); D4 placement; D5 build if T5 passed | slots 1-2 |
| Fri 18:00 | H36 | **DP4**: D5 kept only if it is presentable now **and** within budget; otherwise it is cut (it is cut first) | operator, 10 min |
| Sat 06:00 | H48 | Gate 3 (M2 on L2). The Render Royalty or Glass Box choice (PLAN 8) is made here. The L3 ship decision for M3 at H56. | operator |
| Sat 18:00 | H60 | **DP5**: the last look *change* accepted; after this, parameter tuning only | operator |
| Sun 06:00 | H72 | feature freeze | - |

**If every upgrade fails:** ship `LR_L2_Night` as it is now with D0 applied, which is about an hour of work tonight. That means dark-navy side ice, a clean
blue line, a darker net mesh, beacons (A spawns them, R-0002 point 5), and the reflection saving if T1 found one. Add the backdrop flats from D1 if T2
showed they cost nothing. The L3 slot ships as `l2_night` with neon trim (C-0002). `l1_day` does not ship.

**Handoffs to file once each test result exists (B to A):**

1. 4090 FrameBudget, standalone CSV or Shipping p99, for `l2_night` after D0/D1 and for `l3_neon` at the H24 package. Also with the low profile.
2. Answer C-0002 open question 2 (the persistent `Rink` sky light) before D0 values are final.
3. `MPC_Look` set by `ULookTierSwitcher::SetTier` from a per-tier table. Only if D3 needs `Wet` per tier or D5 is a go.
4. A Director's Call or Designer preset `l2_night` to `l3_neon`, for lane A7.
5. A cook-list line only if a fourth rig is ever proposed. This plan avoids that by using the existing three slots.

**What the operator must decide, and by when:**

| Decision | Deadline | Default if not answered |
|---|---|---|
| Approve this order (T1, T2 tonight in the first free slot; may pause a lane at 15:30 if no slot frees) | Thu 15:30 (H9.5) | tests wait for a free slot |
| Credit ceiling for D4 (proposed: 90 Meshy, 0-2 Rodin beyond lane S3's 10) | Thu 21:00 (H15) | 30 Meshy credits, two props, no Rodin |
| L3 variant: D3a poster or D3b wet neon | Thu 21:00 (H15) | D3a |
| D5 second cut (P3-lite day) test go or no-go | Thu 21:00 (H15) | no-go |
| Taste vetoes on haze, grain, shafts and blue-line style | Thu 21:00 (H15), then Fri 08:00 | lead's choice by pass lines |
| Keep or cut D5 | Fri 18:00 (H36) | cut |
| Modifier: Render Royalty or Glass Box | Sat 06:00 (H48, already in PLAN 8) | - |
| Last look change | Sat 18:00 (H60) | - |

## 5. What I could not determine, and where today's measurements contradict older documents

**Could not determine from the documents (each needs a measurement or an answer):**

1. The 4090 cost of `LR_L2_Night` **as it is now**, with 289 dressing actors on A's real rink, at p99. C-0002's 4.3 ms came from a throwaway with no dressing, no game and no p99.
2. Whether `FPostProcessSettings.reflection_method` and `lumen_max_roughness_to_trace_reflections` can be overridden per volume from Python in 5.9. I believe they
   exist, but this is unverified; T1's script reports which properties it could not set.
3. Whether translucent shafts render after the post-process passes (separate translucency, "after DOF"). If not, the ramp would step them and the outline might edge
   them. T2's stills show which.
4. The basic Cone's apex direction and pivot. T2 reads the bounds and has a flip constant; the first still settles it.
5. Why B1's gap ice is pure black while B2's is dark navy with blotches: board bounce, a different camera, or another editor's sky. Unknown.
6. What a mid-range judge GPU does with any of this. No measurement exists on any machine; B is a loose proxy.
7. The cost of depth of field (D5), Lumen reflections on wet ice (D3b), lens flare and four full-height additive cones (D1). None was measured anywhere.
8. Whether a Meshy-textured prop survives the ramp's divide without blotches (the same mechanism as the blue line).
9. How "B sets rig defaults" for `Wet` and `Scuff` per tier (C-0002 lists B as a producer but names no mechanism). That is the reason for handoff 3.
10. The Render Royalty, NVIDIA and Hyper3D prize criteria (`docs/PLAN.md` 12 is still open).

**Claims in older documents that today's facts contradict:**

| Document and claim | Today's evidence |
|---|---|
| `docs/TRACKS.md` 5: device B has 128 GB, a high-end discrete GPU, and up to 3 editors and 7 builders | B is Windows ARM64 with 64 GB unified memory and the x64 editor under emulation; 2 editors (operator, H5; `handoff/B-to-A-0006.md`). |
| `prep-reference/HOCKEY-LADDER.md` L3: "The RTX 4090 on device B does this comfortably" | The 4090 is on device A. B has an N1X, and ray tracing is off in the project (`status/B.md` H4). |
| `prep-reference/STYLE-PATHS.md`: P2 performance 5/5, "should hold 60 fps on a GTX 1660" | E3's own estimate is 20-25 ms on a 1660 (30-45 fps), because Lumen carries the look. |
| STYLE-PATHS: P2 costs "about 8 h" of the 20 | B1 built masters, post passes and the rig in about 12 minutes; B2 built 289 actors in about 13. Build time is not the cost; judging stills and the editor slot are. |
| STYLE-PATHS: "Drop Lumen to fixed ambient to be safer" | E3: without Lumen (SM5) the look goes flat; the ramp steps Lumen's light. The safe lever is the GI *quality* profile, not dropping Lumen. |
| `contracts/look-tiers.md`: `l2_night` "measured 4.3 ms" against a 4.5 ms budget | Measured on a 143-actor throwaway; the rig now has 301 actors, and B measured the dressing at about +0.3 ms (on B). The current rig is unmeasured on A and may already be at its budget. |
| `docs/HOCKEY.md` 3 (M1 "L1 daylight toy") and `docs/PLAN.md` 3 (Gate 2 "L1 look") | `l2_night` ships first (C-0002, HOCKEY.md 6, GAME.md: "Regular, M1, L2 night"). L1 is empty. |
| `tools/skills/look-painted-cel.md` camera "2.5 m up" and "capsule 1.6 m" | GAME.md and the camera skill say 3.5 m up (pitch -17.65); the engine has no basic capsule (B1). |
| look-painted-cel recipe: blue lines at \|x\| = 17.5 m | B1 derived them from `RinkHalfLength * 0.29` (17.4 m at the default). |
| `docs/experiments/LOOK-P2-NIGHT.md` describes the dark ice as "near black ... raise sky to 3-4 if the puck must be seen there" | B1 shipped sky 2.0, and the side bands of the rink (\|y\| > 14.5 m, most of the width) are dark by geometry, not just "between pools". The E3 advice was not applied. |
