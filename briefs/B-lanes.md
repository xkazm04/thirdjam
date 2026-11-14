# Device B lane board (the lead rewrites this file whenever a lane starts or reports)

Updated 2026-11-14 02:47 CET (H44.8). **The operator's night plan (01:30): nine lanes in parallel until 10:00, each an evolution loop; he plays at 10:00 and sets the last day's priorities.** B stays on its side of the line (no game code); new moves go to device A as wiring asks (B-to-A-0040). One GPU lane (the editor on 8002); everything else runs on the CPU (Blender renders with Cycles on the CPU only). dev-b = main 322e6c5 + B, builds. Lane briefs: the lead's scratch folder; reports under `status/`.

## Running (state at 02:47; one GPU editor, three editors without a GPU, Blender on the CPU)

| # | Lane | Loop now | Where | Model | Report |
|---|---|---|---|---|---|
| 1 | **Picture (the GPU)**: B42 | effects that read at game size (the stop's fan, abilities by kind, chips as streaks, the slide trail capped, the goal at the net); next: films of the figure and physics lanes' work, the stands' light and the cheer, the play-through | main tree, port 8002 | Fable | `status/B42-report.md` |
| 2 | **Figure, no GPU**: SK25 | `skf/export29` and `30`: lying loops by start rule, the reactive whip on contact, hit sizes by Strength; then the new sets with test triggers | `wt/b-fig`, port 8004, branch `lane/b-fig` | Fable | `status/SK25-report.md` (in that tree) |
| 3 | **Physics, no GPU**: RAG 2 | ends-only physics on the fall, a stick body, the schedule in the child Blueprint (side copy only) | `wt/b-vfx`, port 8000, branch `lab/ragdoll` (never pushed) | Fable | `status/RAG-report.md` |
| 4 | **Sound, no GPU**: AU14 | does sound work without a GPU; the fifteen sounds of the new sets as MetaSounds ready to wire | `wt/b-aud`, port 8006, branch `lane/b-aud` | Fable | `status/AU14-report.md` (in that tree) |
| 5 | **Figure, Blender**: SKF31 | the export gate (usable: `skater-lab/skf/gate.sh`), hit and fall as one event, the right-wall pose | `skater-lab/skf` | Fable | `status/B-skater-skf.md` |
| 6 | **Set: dive block, slide tackle**, loop 2 | the stick's path, a block that reads as an act, a chest-first block, a slide tackle that reads from behind | `sets-lab/dive` | Opus | `status/SET-dive-report.md` |
| 7 | **Set: dash**, loop 2 | the stick's swing, a silhouette unlike the sprint from above, mirrors | `sets-lab/dash` | Opus | `status/SET-dash-report.md` |
| 8 | **Set: toy weapons**, rework | the cannon as an unmistakable toy at the hip, the mallet's blow off the head | `sets-lab/weapons` | Opus | `status/SET-weapons-report.md` |
| 9 | **Arena**: AR4 | a far crowd at a quarter of the cost, the ice machine, the barn's walls dressed | `arena-lab` | Fable | `status/AR4-report.md` |
| – | resting | Trailer (three scenarios, the edit command, the operator's page: until there is footage), Eye (two passes done: again when the engine lanes land), sound generation (AU13 done) | | Opus / Fable | `status/TRAILER-report.md`, `EYE-report.md`, `AU13-report.md` |

