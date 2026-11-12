# Lane A25: the rink's construction in code: kick plate, glass rail, net frame, markings (device A, from H10.3)

You are lane A25 of the Third Jam (team Hokej). R-0024 point 1 approves lane A23's look recommendations; the A-side parts are yours (B does the rig, ice and outline masters through
handoff A-to-B-0008). Read first: `docs/experiments/LOOK-LAB.md` (recommendation 6 and the construction sheets `docs/experiments/img/look-lab/sheet-constr*.jpg`),
`Game/Source/JamGame/Private/RinkActor.cpp` (+ header: how the boards, nets, lines and zone quads are built, `LoadLook`, stencils), `contracts/look-tiers.md` (C-0002: which materials
A may instance), `Game/Content/LookLab/` (A23's `MI_LL_KickPlate`, `MI_LL_GlassRail`, `MI_LL_NetFrame` as the proposal; B ships `MI_Toon_KickPlate` / `MI_Toon_GlassRail` later, so
load B's name first and fall back to the LookLab instance with one log line), `handoff/B-to-A-0007.md` and R-0016 (board and line geometry rules), R-0021 point 4.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a25` (the Director creates it; build it yourself; editor on MCP port 8014 only while proving; at most three editors on
the machine; wait for other UnrealBuildTool instances). Run no git command; never touch `Game/Plugins/JamAssets`.

You own: `Game/Source/JamGame/{Public,Private}/RinkActor.*` (construction only; zones and obstacles are A21's finished work, leave their functions alone), `docs/experiments/RINK.md`,
`status/A25.md`.

## Job
1. Boards: a 25 cm kick plate band at the bottom (ochre instance), the board face above it, a dark glass rail on top with stanchions every 2.4 m and a thin glass panel (translucent,
   stencil 0, no reflection cost beyond what the material has), all scaled with the rink (90 x 45 default, 120 x 60 large, R-0016's `BlueLineFrac` / `GoalLineOffsetCm` rules kept).
2. Nets: a 14 cm red tubular frame (posts, crossbar, back frame), a netting card with alpha (B's texture if it exists, else a procedural grid in the material), the goal-line box
   unchanged for the sim; the far net must read as a box at 40 m with the outline.
3. Markings: faceoff circles with the hash marks, centre dot, goal creases (blue half-discs), the trapezoid behind the nets: as decals or ice-material masks, whichever RinkActor does
   today, at the painted-line width the H7 stills use.
4. Benches and a penalty bench shape along one side (empty, crowdless), the goal lights as a small housing on each net; A22's props under `tools/blender/out/` (SM_Bench, SM_GoalLight,
   SM_Net) may be imported into `Game/Content/Rink/` if they are better than code geometry: ledger them.
5. Proof: stills at 10 / 24 / 40 m on the night rig for both rink sizes, the far-net readability pair, ProfileGPU before and after (the construction must add under 0.3 ms), and lane
   A27's eyes tool on the stills when it exists.

Report: the still paths, the cost, the file list for the Director, what B should replace with real meshes later. Time box 90 minutes; interim at 45 minutes.
