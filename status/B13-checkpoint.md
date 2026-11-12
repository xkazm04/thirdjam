# B13 checkpoint: props out of the arena (step A), device B

Lane B13, device B, 2026-11-12, machine clock 15:50 to 15:58. Main-tree editor on port 8000 (pid 29616); a second editor (port 8002) shared the GPU, so no frame numbers.
No git command run. `Rink` not saved, nothing under `/Game/` written, nothing under `_Scratch/`.

## The real rink runs again

PIE on `/Game/Maps/Rink` with the new binaries (tree built on `main` 6a2681e) builds the real game:

```
JAMCART loaded .../hockey.regular.cartridge.json id=hockey-regular ... rink=90x45 visual=toy tier=l2_night
JAMRINK built 90x45 m corner 3.75 goal 4 m, 0 obstacles, 0 zones (0 on B's strips), outline 52 points
JAMLOOK persistent lights hidden: 2 actors (sky light, directional light)
JAMLOOK floor StaticMeshActor_0 lowered by 15.0 cm to a top of -20.0 cm
JAMLOOK tier=l2_night map=/JamAssets/Rigs/LR_L2_Night.LR_L2_Night scale=1.000 (rink 90.0 m) trim_emissive=30.0
JAMLOOK rig loaded: 12 local lights scaled (intensity x 1.000, attenuation x 1.000)
```

PIE world: `RinkActor_0` (ice, boards, glass, trims, nets, beacons), 6 skaters, `JamPuck_0`, the rig streamed in. **Scale printed: 1.000 (rink 90.0 m).** Every still below is the real game.

## What was removed from `LR_L2_Night` (180 actors)

| Group (folder) | Removed |
|---|---:|
| `SM_FilmLight` (Dress/Gen/FilmLight) | 8 |
| their lens discs (Dress/Lens, `MI_Trim_FilmLens`) | 8 |
| `SM_DirectorChair` | 5 |
| `SM_Slate` | 4 |
| `SM_CameraDolly` | 4 |
| `SM_CameraCrane` | 2 |
| `SM_FlightCases` | 12 |
| `SM_TrussLamps` (the Rodin lamp heads) | 2 |
| batch 2: `SM_Bench` 2, `SM_CoachStool` 2, `SM_WaterCrate` 2, `SM_PenaltyBox` 1, `SM_GoalLight` 2, `SM_ScoreboardHousing` 1 | 10 |
| scoreboard wires (Dress/Truss) | 2 |
| dolly track, rails, sleepers, stands (Dress/Track) | 38 |
| cube cases (Dress/Cases) | 65 |
| cables (Dress/Cables) | 10 |
| exit signs (Dress/Signs) | 2 |
| `Practical_L2_Night_1..8` (the shadowless spots that lit the set) | 8 |

The assets stay in the plugin, unplaced (ledger note for the seven Rodin props and the six batch-2 pieces: "unplaced since H8.5, returns with the final art style").
`LR_P3_Clay` held no props (7 actors: `Key_P3_Clay`, `Fill_P3_Clay_1..3`, `Sky_P3_Clay`, `PPV_P3_Clay`, `LookParams_P3_Clay`): nothing to remove, the file is unchanged (32,591 bytes) and was not re-saved.

## What remains in `LR_L2_Night` (saved, 48 actors)

| Prefix | Count | What |
|---|---:|---|
| `PPV` | 1 | `PPV_L2_Night`, unchanged |
| `Sky` | 1 | `Sky_L2_Night`, unchanged |
| `Spot` | 4 | `Spot_L2_Night_1..4` at x -33.75, -11.25, 11.25, 33.75 m, z 25 m, unchanged (the only shadow casters) |
| `Room` | 6 | the dark volume, inner faces x +-60, y +-37, ceiling 28 m, unchanged |
| `Con` | 30 | the truss as bare structure, relabelled from `Dress_`: `Con_L2_Night_TrussBeam_1..2` (x -50..50, y +-2.2, z 27), `_TrussCross_1..4`, `_TrussHanger_1..8`; and per spot one lamp can from basic shapes: `Con_L2_Night_LampCan_n` (steel cylinder 1 m x 1.4 m, z 23.4 to 24.8), `_LampLens_n` (disc under it), `_LampYoke_n_a/_b`. All `cast_shadow` False, folders `Con/Truss` and `Con/LampCan` |

Nothing else (no `Dress_`, no `Practical_`). File `Game/Plugins/JamAssets/Content/Rigs/LR_L2_Night.umap`: **104,046 bytes** (was 468,713). Rig triangles now about 8.4k (16 engine cylinders, 19 cubes, 1 plane), was 238.8k.

## Brightness on the ice (paired real-game stills, same cameras, full-range luma)

| View | Crop | Before | After | Change |
|---|---|---:|---:|---:|
| own end (x -39, looks at -x end) | ice corner L / R | 111.7 / 109.9 | 112.1 / 109.6 | +0.3 / -0.2 % |
| far end (x 39, looks at +x end) | ice corner L / R | 115.0 / 116.7 | 114.2 / 116.3 | -0.6 / -0.3 % |
| game-camera geometry (x -36, y -3) | pool / dark band | 206.5 / 59.8 | 206.4 / 58.2 | -0.05 / -2.6 % * |

\* the band crop also holds the reflection of the end set in the ice (screen-space reflections), which is gone; under the 3 % line. **Confirmed: the practicals did not light the ice** (every ice crop within 0.7 % except the reflection one). Lane B11's -7 to -12 % at the end boards came from the truss lamps moving, not from the set, and is unchanged by step A; subject 4 of step B addresses it.

## Stills (`devlog/shots/`)

- `20261112-155815-B-b13-a-empty-wide24` (empty arena from 24 m), `-a-empty-game` (game camera), `-a-empty-over` (high corner view, truss and lamp cans visible)
- before: `20261112-155816-B-b13-a-before-over`, `-a-before-game`

## Ledger

Replace the `LR_L2_Night.umap` row (lane B11's) with:

```
| Game/Plugins/JamAssets/Content/Rigs/LR_L2_Night.umap | other | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | night rig: PPV, sky, 4 spots, dark room, bare truss with 4 basic-shape lamp cans; every prop and practical unplaced since H8.5 (lanes B1, B1c, B11, B13) |
```

Lane B13 goes on with step B (sky, ice, construction, lighting); the rig stays saved in this state until the next stop.
