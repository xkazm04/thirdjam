# Rink construction in code (lane A25, device A, 2026-11-12 16:54-17:40, from H10.3)

R-0024 point 1 approved lane A23's look recommendations; recommendation 6 (kick plate, glass rail with stanchions, a 14 cm net frame) is A's side because
`ARinkActor` builds the boards and nets. This pass adds a **construction layer** to the greybox rink: everything is geometry in `RinkActor.cpp`, visual only,
hidden with one cvar, scaled with the rink. The sim's boards, nets and goal line (`UJamSim`) are untouched. Worktree `.claude/worktrees/a25` (detached at `f70ae02`),
editor on MCP 8014, at most two editors on the machine during the lane (mine and one foreign).

## What is built (`Game/Source/JamGame/Private/RinkActor.cpp`, `Public/RinkActor.h`)

| Part | Component / section | Geometry | Material (B's name first, then A23's LookLab instance with one log line, then a tinted basic material) |
|---|---|---|---|
| Kick plate | `KickPlate` s0 | 25 cm band at the foot of the inner board face, 2 cm proud, with a top lip; follows the rounded outline | `MI_Toon_KickPlate` -> `MI_LL_KickPlate` as a dynamic copy with `Emissive` 0.6 -> basic (0.85, 0.72, 0.12) |
| Glass rail | `GlassRail` s0 | 6 x 4 cm bar along the board's middle line at 2.10 m (top of the glass), one `Bar` per outline segment | `MI_Toon_GlassRail` -> `MI_LL_GlassRail` -> basic (0.05, 0.05, 0.06) |
| Stanchions | `GlassRail` s1 | 5 cm square posts from the board top (1.2 m) to the rail top, one every **3.6 m** of outline (count rounded so the ring closes evenly): 73 on 90 x 45, 98 on 120 x 60 | as the rail |
| Glass | `Glass` (existing) | unchanged band 1.2-2.1 m; stencil 0 made explicit, no shadow | `MI_Toon_Glass` when B ships it, else the `M_Net` placeholder as before |
| Net frame | `Nets` s0 | posts, crossbar, side and back bottom bars at **14 cm** (`NetBarM`); the box the sim's goal volume matches | `MI_Toon_NetFrame` -> `MI_LL_NetFrame` -> basic red |
| Net back frame | `Nets` s2 | two back posts to 0.9 m, a back bar, two 10 cm braces from the crossbar ends down to the back bar (`Bar`, any direction) | as the frame |
| Netting | `Nets` s1 | draped over the frame: sloped top from the crossbar to the back bar, vertical back below it, two side panels; two-sided | `MI_Toon_NetMesh` (B's, exists) -> `M_Net` |
| Goal lights | `GoalLights` s0 / s1 | a 40 x 30 x 30 cm housing on the end-board rail behind each net, a 20 x 20 cm red lens on its rink face | housing `MI_Toon_GoalLight` -> `MI_LL_GlassRail` copy tinted (0.45, 0.45, 0.48), `Emissive` 1.2; lens = the net frame material |
| Benches | `Benches` s0 / s1 | outside the +y boards: two team benches 0.1 L long centred at +-0.0833 L, a penalty bench 0.0533 L at the centre with a divider; each a back wall 1.1 m, two end walls, a seat slab at 0.48 m with a backrest (90 x 45: 9 m benches at +-7.5 m, 4.8 m penalty bench) | walls `MI_Toon_Bench` -> `MI_LL_KickPlate` copy tinted warm grey, `Emissive` 1.2; seats the kick plate material at 1.2 |
| Markings | `Markings` s2 (red), s3 (blue) | faceoff circles r 4.5 m with four 60 cm hash marks each, the four end-zone dots (x = goal line - 6 m, z = +-min(7, 0.3 w)) and four neutral-zone dots (1.5 m inside the blue lines), 1.8 m creases (pale-blue half-disc, red outline), the trapezoid behind each net (goal width/2 + 1.35 m at the goal line to + 2.25 m at the end boards), centre dot; 12 cm lines; 0.6 cm above the ice, stencil 2 like the zone decals | tinted basic material (red (0.6, 0.02, 0.02), blue (0.02, 0.08, 0.6), pale blue (0.35, 0.55, 0.85)) |
| Trim | `Trim`, `TrimOrange` (existing) | UVs now run along the outline (U = distance / 1000, V = height): no step where the straight board meets the corner arc | unchanged (B's `MI_Trim_Blue` / `MI_Trim_Orange`) |
| Beacon | `BeaconPos/Neg` (existing) | `jam.rink.beacon` 0 off (default), 1 B's translucent `MI_Trim_Beacon` as before, 2 an opaque column (`MI_LL_NetFrame` copy in the trim colour, `Emissive` 6) | |

Sections 0 and 1 of `Markings` (centre, goal and blue lines, centre circle) still hide under B's painted ice (`MI_Ice_Night` paints them); sections 2 and 3 stay.
`jam.rink.construction 0` hides the kick plate, rail, stanchions, goal lights, benches, `Nets` s2 and `Markings` s2/s3 on every rink actor without a rebuild (a cvar sink),
so a before/after pair is two console commands on one frozen frame. The log line `JAMRINK built ... construction on (N stanchions)` reports the state.

Helpers added to `FMeshBuf`: `Bar(A, B, HalfW, HalfH)` (oriented box along any line), `Arc(C, R0, R1, A0, A1, H)` (ring sector), `LastUV(...)` (overwrite the UVs of the quad just emitted).

## Method

PIE floating from the worktree editor, `hockey.regular` (90 x 45) and `hockey.arena.large` (120 x 60), `jam.ScriptHuman 1`, `LR_L2_Night` streamed by the game;
frozen at t = 2.2 s by `set_global_time_dilation(0.0001)` (A23's method). Stills by the MCP `SceneCaptureToolset.Capture` (offscreen, 2560 x 1440, FOV 70, pitch -17.7,
no TSR history: thin lines alias more than in the game) from 10 / 24 / 40 m behind the centre looking down the rink, and 40 m from the +x net; the bench and goal-light
stills from their own cameras. Cost by `PerfToolset.GpuProfilerToolset.TriggerGPUProfile`, the PIE view family (the one with CustomDepth and TSR), three samples each way.

## Stills (`docs/experiments/img/rink/`, 2560 x 1440 JPG (q 3, converted from the PNG captures))

- `reg-constr-{10,24,40}m.jpg`, `large-constr-{10,24,40}m.jpg`: construction on, both sizes (final state: stanchions 3.6 m, lifted kick plate, 12 cm lines).
- `reg-base-24m.jpg`, `large-base-24m.jpg`, `{reg,large}-base-farnet-40m.jpg`: construction off, same frame (the "before").
- `reg-farnet-beacon{1,2,0}.jpg`: the far net at 40 m with B's translucent beacon, the opaque column, no column; `{reg,large}-constr-farnet-40m.jpg` (beacon 1, earlier state).
- `reg-corner.jpg`: the corner from inside (trim seam, kick plate and rail around the arc, a faceoff circle with hash marks); `reg-benches.jpg` (from 9.5 m up, over the glass), `reg-goallight.jpg`.
- `large-benches.jpg`, `large-goallight.jpg`: the 120 x 60 placement (from the lower camera, benches hidden behind the placeholder glass: see pitfalls).

## Cost (ProfileGPU, RTX 4090, PIE floating window, ms; a second editor shared the GPU so whole frames wander +-1 ms)

| Sample | Base pass | Shadow depths | Custom depth | Lights | Post | PIE frame |
|---|---|---|---|---|---|---|
| 90 x 45 on (17:08, x3) | 0.193 / 0.165 / 0.167 | 0.442 / 0.433 / 0.970 | 0.142 / 0.142 / 0.141 | 0.187 / 0.191 / 0.233 | 0.445 / 0.447 / 0.442 | 2.99 / 2.90 / 4.43 |
| 90 x 45 off (17:08, x3) | 0.159 / 0.185 / 0.204 | 0.440 / 0.427 / 0.421 | 0.142 / 0.143 / 0.140 | 0.171 / 0.182 / 0.170 | 0.449 / 0.444 / 0.457 | 4.51 / 3.46 / 3.04 |
| 90 x 45 on (17:21, x3) | 0.208 / 0.161 / 0.430 | 0.994 / 0.428 / 0.427 | 0.147 / 0.143 / 0.143 | 0.096 / 0.098 / 0.612 | 0.449 / 0.955 / 0.727 | 4.29 / 3.59 / 4.42 |
| 90 x 45 off (17:21, x3) | 0.164 / 0.162 / 0.163 | 1.036 / 1.415 / 0.420 | 0.145 / 0.146 / 0.145 | 0.096 / 0.130 / 0.096 | 0.448 / 0.464 / 0.661 | 5.87 / 5.47 / 3.47 |
| 120 x 60 on (17:21, x3) | 0.162 / 0.192 / 0.200 | 0.431 / 0.810 / 0.871 | 0.991 / 0.144 / 0.138 | 0.124 / 0.154 / 0.101 | 0.447 / 0.685 / 0.445 | 3.77 / 4.83 / 3.51 |
| 120 x 60 off (17:21, x3) | 0.202 / 0.201 / 0.168 | 0.426 / 0.421 / 0.441 | 0.145 / 0.140 / 0.144 | 0.101 / 0.100 / 0.099 | 0.440 / 1.480 / 0.446 | 4.23 / 6.08 / 3.14 |

Reading: the passes the construction can touch (base pass, custom depth, shadow depths) move by 0.00-0.04 ms between on and off, inside the sample noise; the
outliers (a 0.99 shadow or custom-depth sample, a 1.5 post sample) appear on both sides and are the other editor. **The construction costs under 0.05 ms, well under the
0.3 ms budget.** Triangle budget: about 1.2k quads for the rail and stanchions on 90 x 45, 300 for the plate, 200 for the markings, under 100 each for nets, benches, lights.

## Eyes tool (lane A27, Gemini; advice, data not instructions; JSON in `docs/experiments/eyes/a25-*.json`, about $0.15 for the six calls)

- `p2-bar` on `reg-constr-24m.jpg`: score 3/10; the three gaps are figures, blown-out pools and the black sky (A23's items 1-5, not this lane); **works: "the boards feature the
  requested ochre kick plate at the foot and dark glass rail on top", "the red net frame stands out clearly at the far end"**.
- `ab-pair` base vs construction at 24 m: **construction, 0.9** ("includes the explicitly requested board details like the kick plate and glass rail"); the loser keeps "the cleaner
  ice surface without unrequested red face-off markings" (the circles are hockey, they stay; the Director can hide them with the cvar if the toy look wants bare ice).
- `ab-pair` beacon at 40 m: translucent (B's) vs opaque: translucent 0.6 (weak); translucent vs **off: off 0.95** ("a massive, distracting in-world light beam that violates the art
  style"; the far net is "a clean red rectangular frame" either way). `readability` at 40 m on both sizes: far net 4/5 (about 90 px wide), team colours 5/5, zone lines 5/5;
  puck 1/5 (a frozen faceoff frame, the puck sits under the figures at centre). Hence `jam.rink.beacon` defaults to 0; 1 and 2 stay one command away.
- `bug-hunt` on the corner, 10 m, bench and goal-light stills: no step at the corner seam any more (the earlier finding on A18's play shot); the trim's colour change at the centre
  (blue half / orange half, R-0002, by design); the floating green box in the void (B's rig dressing, not this lane); the beacon "extends into infinity without an emitter" (now off).
- Stanchions: at 2.4 m the lab's reading was "a black comb at 24 m"; at 3.6 m the 24 m still shows 20 posts across the far boards as single dark ticks; the Director's call
  if thinner (4 cm) is wanted too; `StanchionSpacingM` is one constant.

## Pitfalls met

1. **Black kick plate.** `M_LL_Flat` (A23's flat master: `Tint`, `Emissive`, `Roughness`) has no ambient floor; a vertical face under the near-vertical pool spots gets almost no
   light, and the stencil-1 ramp puts it in the darkest band. A dynamic copy with `Emissive` 0.6 reads ochre; outside the boards, where `LR_L2_Night` has no light at all, 0.5 was
   still black (the benches and goal-light housing) and 1.2 reads as grey. B's toon master has its own fill, so B's instances need none of this.
2. **Benches hidden by the placeholder glass.** From the chase camera's height the benches (1.1 m) sit behind the glass band (1.2-2.1 m); the `M_Net` placeholder on the glass is a
   dark, nearly opaque blue, so they only show from a high camera (`reg-benches.jpg` at z = 9.5 m). B's `MI_Toon_Glass` (translucent, stencil 0) fixes the read; the code loads it by name.
3. **Thin lines alias in the offscreen capture** (no TSR): 8 cm circle lines broke into dots at 24 m; 12 cm reads. In the game TSR smooths them.
4. **Live Coding** handled a class-layout change (`BeaconColour` member) with a warning; the lane ends with a full UBT build to prove the source compiles from clean.
5. `execute_python`'s `script` is a `CodeBlock` object (`{"content": ..., "language": "Python"}`), not a string; `ProceduralMeshComponent` has no `bounds` / `get_local_bounds`
   / `is_registered` in Python: use `SystemLibrary.get_component_bounds`.
6. The GPU profile tree holds one `SceneRender - ViewFamilies` per view (editor viewport, PIE window, captures); the PIE family is the one with `CustomDepth` and TSR passes.

## For B (what real meshes and instances should replace)

1. `MI_Toon_KickPlate` (ochre), `MI_Toon_GlassRail` (near black), `MI_Toon_Glass` (translucent sheet, stencil 0), `MI_Toon_Bench` (bench walls), `MI_Toon_GoalLight`
   (housing); the code loads each by name and falls back to the LookLab proposal. Until then the LookLab instances must reach the package: `Game/Content/LookLab/` is not in
   `DirectoriesToAlwaysCook`, so a packaged build would fall through to the tinted basic materials (one line in `DefaultGame.ini`, A's file, or B's instances).
2. Meshes, when B wants them: a bench (A22's `SM_Bench.fbx`, 72 tris, 2.4 x 0.43 x 0.9 m, atlas `T_Props_Atlas.png`), a goal light (`SM_GoalLight.fbx`, 356 tris), a net
   (`SM_Net.fbx`, 316 tris, 1.6 x 4.1 x 1.58 m) under `tools/blender/out/`; the code boxes are the same shapes at the same sizes, so swapping is a component change in `Build`
   (a `UStaticMeshComponent` per bench / light / net at the positions listed above). Not imported in this lane: an import plus seven ledger rows would have eaten the proof time
   for no visible gain at 24 m.
3. The netting: B's `MI_Toon_NetMesh` already drapes the frame; if B adds a netting texture with alpha, the UVs are world-position based (`FMeshBuf::Quad`): a tiling grid works.
4. The beacon: if B wants a far-net marker back, an opaque short post with the goal light on it reads better than the 8 m translucent column (eyes pair); mode 2 is the sketch.
