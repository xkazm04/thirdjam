# B2 report: arena dressing, pass 1 (device B)

Lane B2, device B, 2026-11-12 about 10:45 to 10:58 by the machine clock (box ended 12:45). No git command was run. Everything went through `node tools/mcp/mcp.mjs`
(`execute_python`, `StartPIE`, `StopPIE`, `CaptureGameViewport`, `GetFrameTimings`) against the lead's editor on port 8000. At the end the editor is open on
`/Game/Maps/Rink`, `get_dirty_map_packages` and `get_dirty_content_packages` are both empty, nothing under `/Game/` was saved, and the plugin `Content/` holds only
`Arena/`, `Look/`, `Rigs/` (no `_Scratch`).

Method: pass 1 only, engine basic shapes (`Cube`, `Cylinder`, `Cone`, `Plane`), nothing generated or downloaded. 289 plain `StaticMeshActor`s labelled
`Dress_L2_Night_1..289`, grouped in outliner folders `Dress/Truss`, `Dress/FilmLights`, `Dress/Chair`, `Dress/Slate`, `Dress/Crane`, `Dress/Track`, `Dress/Cases`,
`Dress/Cables`, `Dress/Bench`, `Dress/Signs`. No `SM_`/`BP_` asset was authored (the lead's decision: plain placed actors; I did not try Blueprints). All actors: Static
mobility, `NO_COLLISION`, stencil 1 only where the brief's table says 1, `cast_shadow` off except the film-light stands (pole, legs, head) and the chair. Every piece
was checked by script against the footprint rule: bounds read back, zero violations (|x| >= 72 m or |y| >= 37 m, except the truss at z 24 to 27.5 m, which is overhead).

## PASS / FAIL per piece of the brief's table

| # | Piece | Result | Evidence |
|---|---|---|---|
| 1 | Dark room | PASS (checked, not edited) | B1's `Room_L2_Night_1..6` read back unchanged from the saved map; nothing of mine is inside the 140 x 70 m footprint plus 2 m; the stills show the room black behind the dressing. |
| 2 | Light truss | PASS, one deviation | 2 beams 160 m long at z 27 (y = -2.2 and +2.2), 4 cross beams, 4 lamp heads (cylinder 1.0 x 1.4 m, stencil 1) over the spot positions x = -45, -15, 15, 45, a lens disc under each (`MI_Dress_Lens`, stencil 0). Heads, lens discs and beams are in the own-end still (top of frame) and in a looking-up capture. Deviation: beams use `MI_Dress_Case`, not `MI_Toon_Dark`, because dark on the black room was invisible in the first capture. No spot was touched. |
| 3 | Film lights on stands | PASS | 8 stands at (+-30, +-40) and (+-62, +-40): centre pole, 3 tripod legs, head box aimed at the ice, lens disc, 4 barn-door plates (10 actors each, 80 in all). Four are visible in the own-end still and two in the faceoff still. Plain actors, not `BP_FilmLight`. |
| 4 | Director's chair | PASS, not in the three required stills | At (0, -41), facing the ice, 1.8 x life size, seat and back `MI_Toon_Orange`, wood parts `MI_Toon_Stick`, scissor legs, arms, a megaphone cone on the seat (13 actors). It is 41 m off the long axis, so it is outside the frame of the three required views; shown in a side capture (scratch only, not filed). |
| 5 | Slate | PASS, one deviation | At (10, -39.5), turned 120 degrees, leaning back 8 degrees, about 2.9 m tall with the clapper open 18 degrees on a hinge cylinder, 8 alternating cream/ink segments on the stick, three cream lines on the board, no text (14 actors). Visible at the left edge of the own-end still. Deviation: stripes are separate cube segments in two materials, not a material function. |
| 6 | Crane and dolly track | PASS | Crane at (80, -14), arm front end at x = 72.5 so it stays outside the 72 m line: wheeled base, mast, pivot, counterweight, camera box, lens, two cables (13 actors); beside the far net, not over it. Dolly track along +y at y = 43.3 and 44.7: two 130 m rails, 33 sleepers, a cart with a post and camera box (38 actors). Crane is in the faceoff and far-end stills; the track is only checked by position. |
| 7 | Cases and cables | PASS | 75 cases (`MI_Dress_Case`, stencil 0, some stacked) along both long walls (y about -45.2 and +46) and both end walls (x about +-80), 10 flattened cylinders as cable runs at |y| 37.7 to 38.5 and |x| 73 to 75 (`MI_Toon_Dark`, stencil 0). Cases are the dark blue boxes along the horizon in all three stills. Cables are black on black and effectively invisible. |
| 8 | Player benches and doors | PARTIAL | Two benches (6 m, seat, back, two legs, stencil 1) at (+-12, -38). Doors NOT done: the boards belong to A and stand inside the footprint, so no door can be placed under the |y| > 37 m rule. |
| 9 | Exit signs | PASS | Two planes 2.4 x 0.8 m, `MI_Dress_ExitGreen` (M_Trim, Emissive 8), stencil 0, no letters: one on the +x wall at (81.9, 28, 9), one on the -x wall at (-81.9, -28, 9). Visible green rectangle top right in the own-end and far-end stills. |

Acceptance: pieces placed in `LR_L2_Night` outside the footprint: PASS. Three 1920 x 1085 stills with the third-person camera 6 m behind and 3.5 m above a figure: PASS; in none
does dressing cover the ice, the far net or its beacon (the beacon cube stays clear in the far-end still). Cost numbers: below. Ledger: rows below, `check-assets-ledger.mjs` not run (not mine to run).

## Rig map: actor labels (read back after discarding in-memory state and reloading the saved file)

301 actors: B1's 12 and 289 of mine, nothing else.

`PPV_L2_Night`, `Sky_L2_Night`, `Spot_L2_Night_1`, `Spot_L2_Night_2`, `Spot_L2_Night_3`, `Spot_L2_Night_4`, `Room_L2_Night_1`, `Room_L2_Night_2`, `Room_L2_Night_3`,
`Room_L2_Night_4`, `Room_L2_Night_5`, `Room_L2_Night_6`, and `Dress_L2_Night_1` through `Dress_L2_Night_289` (consecutive, no gaps).

## Files created or changed under `Game/Plugins/JamAssets/Content/` (bytes on disk)

| File | Bytes |
|---|---:|
| `Arena/MI_Dress_Case.uasset` | 8,844 |
| `Arena/MI_Dress_Cream.uasset` | 8,625 |
| `Arena/MI_Dress_ExitGreen.uasset` | 8,464 |
| `Arena/MI_Dress_Ink.uasset` | 9,354 |
| `Arena/MI_Dress_Lens.uasset` | 8,019 |
| `Arena/MI_Dress_Steel.uasset` | 8,819 |
| `Rigs/LR_L2_Night.umap` (changed; B1 left it at 43,084) | 656,481 |

The five `M_Toon` instances: `MI_Dress_Ink` (0.12, 0.14, 0.20, Emissive 1.6, Rim 1.0), `MI_Dress_Cream` (0.85, 0.80, 0.68, 1.5, 0.6), `MI_Dress_Steel` (0.22, 0.25, 0.30, 1.3, 0.6),
`MI_Dress_Case` (0.10, 0.14, 0.22, 0.9, 0). `M_Trim` instances: `MI_Dress_Lens` (1.0, 0.85, 0.6, Emissive 10), `MI_Dress_ExitGreen` (0.1, 1.0, 0.3, Emissive 8).
Under `devlog/shots/`: the three stills and their caption files (below). No texture, mesh or sound file.

## Ledger rows (paste into `ASSETS.md`; the umap row REPLACES B1's row of the same path)

```
| Game/Plugins/JamAssets/Content/Arena/MI_Dress_Case.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | flight cases and truss beams, M_Toon instance (lane B2) |
| Game/Plugins/JamAssets/Content/Arena/MI_Dress_Cream.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | slate stripes and lines (lane B2) |
| Game/Plugins/JamAssets/Content/Arena/MI_Dress_ExitGreen.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | exit signs, M_Trim instance (lane B2) |
| Game/Plugins/JamAssets/Content/Arena/MI_Dress_Ink.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | dark parts with fill so they read at stencil 1 (lane B2) |
| Game/Plugins/JamAssets/Content/Arena/MI_Dress_Lens.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | lamp lens discs, M_Trim instance (lane B2) |
| Game/Plugins/JamAssets/Content/Arena/MI_Dress_Steel.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | stands, heads, benches, crane (lane B2) |
| Game/Plugins/JamAssets/Content/Rigs/LR_L2_Night.umap | other | 0.7 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | night rig: B1's PPV, sky, 4 spots, dark room plus 289 Dress_L2_Night actors from basic shapes (lane B2); replaces the B1 row |
```

## Stills (`devlog/shots/`, PNG 1920 x 1085, each with a caption `.txt`)

- `20261112-104742-B-b2-dressing-own-end.png` (figure at x = -45)
- `20261112-104743-B-b2-dressing-faceoff.png` (figure at x = -3)
- `20261112-104743-B-b2-dressing-far-end.png` (figure at x = 45)

The ice, figures, net boxes and beacon cube in them are a throwaway layout (ice plane 120 x 60 m `MI_Ice_Night` stencil 2, cylinder figures stencil 1, net cube, beacon
cube, board strips), added in memory to the rig map after the dressing was saved and removed again; the saved map was reloaded and read back to confirm. They are not A's
rink, nets or skaters. `StartPIE` in `PlayMode_InEditorFloating`, window 1920 x 1080, a `CameraActor` auto-activated for player 0 (FOV 70), moved with Python per shot.

## Numbers (device B, PIE)

- Triangles of all dressing: **48,816** (211 cubes x 48, 75 cylinders x 512, 1 cone x 284, 2 planes x 2, read from the basic shapes' mesh descriptions). Target was under 150k. Cylinders are 79% of it.
- Frame time with and without the dressing, `GetFrameTimings` x 60 per run, PIE floating window, same camera (own-end view), dressing hidden with `set_actor_hidden_in_game` and hidden in the editor viewport:

| Pair | gpuMs median without | with | difference |
|---|---:|---:|---:|
| 1 | 16.58 | 16.88 | +0.30 |
| 2 | 28.13 | 28.47 | +0.34 |

  Difference about **+0.3 ms** (target under 0.5). **Other editors were running on the machine** (the other lanes' editors; I did not check how many), and the absolute level moved from 16.6 to 28 ms between pairs
  for that reason; read the difference inside a pair, not the absolute value. A third "with" run in between gave 26.8. Game thread about 15 ms in all runs, so these are GPU-bound
  numbers dominated by Lumen (B1 measured 10.5 ms on a quiet machine). 289 actors of 5 meshes: instancing was not checked, the draw call count was not read.

## What I did not finish or could not verify

- Doors in the boards (item 8): not done, see above. Benches are 8 m behind the boards, not beside them.
- Blueprints and `SM_` assets: none made; the brief's asset names (`BP_FilmLight`, `BP_DirectorChair`, `BP_Slate`, `BP_Crane`, `BP_Bench`, `SM_Truss_Beam`, ...) do not exist. A second pass or A's tooling that expects them will not find them.
- The chair, the benches and the dolly track are outside the frame of every view along the long axis; I saw them only in scratch captures from the side.
- Checked with a throwaway 120 x 60 m layout only; not with A's real rink, nets and skaters, and not on the 90 x 45 or 140 x 70 cut.
- Slate stripes are cube segments, not a material function.
- Draw-call cost, shadow cost of the 8 stands under real gameplay, and `check-assets-ledger.mjs` were not measured or run.
- Dressing outside the spot cones is lit only by the sky light and the fill of the materials: stencil 0 pieces (cases, cables, truss beams, track sleepers) read as dark navy shapes or not at all. That is the "quiet" the brief asks for, but it is a taste call.

## For the lead to decide

1. Doors and benches: allow a piece inside the |y| > 37 m line (benches and doors need to be at the boards, about |y| 31 to 33 m) or accept benches far back and no doors.
2. Keep plain actors, or have `BP_*` wrappers made in pass 2 (Blueprint authoring was not tried).
3. Chair and slate placement: move them toward an end (x about +-74, near the long axis) if they must show in the default gameplay view.
4. Pieces I would replace with generated props, if the operator wants a second pass: the director's chair and the slate (most recognisable, the cube versions are crude), then the film lights; the crane and cases are fine as silhouettes.
5. Paste the ledger rows above, replacing B1's umap row.
