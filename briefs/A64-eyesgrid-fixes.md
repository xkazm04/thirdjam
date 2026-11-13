# Lane A64: the eyes grid's A-side faults fixed in rank order (device A, from H14, Opus)

You are lane A64 of the Third Jam (team Hokej). Lane A54 photographed the H19 package from every cut, tier and camera and let the eyes tool rank what looks wrong
(`docs/experiments/EYES-GRID.md`, stills under `docs/experiments/img/eyesgrid/h19/`, named `<cut>-<tier>-<moment>.jpg`). Fix the faults on A's side, in rank order, and
prove each with a new still of the same cell.

Read first: `docs/experiments/EYES-GRID.md` (the 22 findings, the owner per row), `status/A54.md` (the premises that were wrong: the launcher stub, `-cut=<path>` with a
cartridge copy to reach a tier, the silent fallback to hockey.regular on a rejected cartridge), `tools/qa/eyesgrid/shoot.ps1` + `grid.mjs` (how a still is taken),
`Game/Source/JamGame/Private/{HUDWidget,RinkActor,CameraFollow,LookTierSwitcher,HokejGameMode}.cpp`, `docs/experiments/UI.md` section 7 (HUD pass 2), `docs/experiments/
CAMERA.md`, `docs/experiments/QA1-FIXES.md`, `Game/Content/Cartridges/hockey.chaos.cartridge.json`, `tools/eyes/README.md`, R-0021 point 3.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a64` (created at the current main; build it yourself, unity off; one editor on MCP port 8031 only while
proving; at most three editors on the machine). Run no git command; never touch `Game/Plugins/JamAssets` (a material or mesh fault is B's: write the exact asset and
what it needs into your report and work around it on A's side where a code path can). Presentation work never changes the sim hash. A foreign "Windows Security" dialog
may hold the foreground: drive the game with PostMessage or the remote console, as A54 and A58 did.

You own: `HUDWidget.*` (the controls strip, the match card, the YOU ring, the net-distance markers), `RinkActor.*` (the goal-light housing, the bumper of the chaos cut,
the glass line), `CameraFollow.*` (the puck and carrier in frame, the far net framing), `LookTierSwitcher.cpp` (the `l1_day` exposure), `HokejGameMode.cpp` (a log line
and a HUD toast when a `-cut` cartridge is rejected and the game falls back), `docs/experiments/EYES-GRID-FIXES.md`, `docs/experiments/img/a64/**`, `status/A64.md`.

## Job, in A54's rank order
1. Chaos cut: the bumper is a flat magenta box (severity 5): find what spawns it (a missing material reference, a `/Engine/BasicShapes` primitive without a material, a
   bypass) and give it a kit material through the look tier; if the material is B's, say which and set a neutral one from the engine's own set meanwhile.
2. The controls strip collides with the minimap and its "PAUSE ESC" text in 51 stills: move or shrink one of them so neither overlaps at 1920x1080 and 1280x720.
3. The puck and the carrier are not visible in 33 stills, the far net not in frame in 4, only one opponent visible in 3: the game camera's framing (A40/A49 bounds):
   find the moments (card, faceoff, far net) where the camera sits and keep the puck and the carrier inside the frame's middle two thirds; the far-net still must show
   the net.
4. The goal-light housing reads as a floating cube on the end boards in 10 stills: attach it to the glass line (A25's placement) or hide its box and keep the light.
5. `l1_day` is blown out: exposure or the sky's intensity in the day tier's switch on A's side (if it is B's rig `LR_L1_Day`, say so with the parameter).
6. The Match 1 card and the WON/YOU reticle cover the faceoff in 21 stills; the net-distance markers sit on boards and nets in 32: the card leaves the middle third free
   during the drop, the markers clip to the ice.
7. The silent fallback: a rejected `-cut` cartridge logs `JAMCUT rejected <path>: <reason>` in every configuration and shows a HUD toast for 4 s.
8. Proof: re-shoot the cells each fault came from with A54's `shoot.ps1` on a Development package of your worktree; eyes `bug-hunt` on each new still; a before/after
   table in `EYES-GRID-FIXES.md` with the severity per cell before and after.

Report: the fault table with the fix per row, what is B's, the still paths, the file list. Interim at 45 minutes with rows 1-3. Time box 100 minutes.
