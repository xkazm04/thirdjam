# B15 checkpoint: step 0 (construction, one source), device B

Lane B15, device B, 2026-11-12, machine clock 17:40 to 17:58. Editor on port 8002 (tree `wt/b-vfx`, pid 3808). No git command, no build, `Rink` not saved.

## What device A's code loads (`RinkActor.cpp`, 547699c) and whether it exists now

`LoadLook(name)` loads `/JamAssets/Look/<name>.<name>`; `LoadConstructionMat` falls back to `/Game/LookLab/MI_LL_*` (a dynamic copy with an emissive lift), then a tinted basic material.

| Name the code asks for | Component | Fallback before B15 | Now |
|---|---|---|---|
| `MI_Toon_KickPlate` | `KickPlate` s0, `Benches` s1 (seats) | `MI_LL_KickPlate` MID, Emissive 0.6 | **created** (M_Toon, TeamColour (0.85, 0.72, 0.12), Emissive 0.8, Roughness 0.5, Rim 0) |
| `MI_Toon_GlassRail` | `GlassRail` s0 (rail) and s1 (stanchions) | `MI_LL_GlassRail` | **created** (M_Toon, (0.05, 0.05, 0.06), Emissive 0, Roughness 0.4, Rim 0) |
| `MI_Toon_Glass` | `Glass` | `/Game/Placeholders/M_Net` | **created**, a child of `MI_Glass` (B13's glass): the glass is now B's on every tier |
| `MI_Toon_Bench` | `Benches` s0 | `MI_LL_KickPlate` MID tinted, Emissive 1.2 | **created** (M_Toon, (0.62, 0.60, 0.56), Emissive 1.0) |
| `MI_Toon_GoalLight` | `GoalLights` s0 (housing) | `MI_LL_GlassRail` MID tinted, Emissive 1.2 | **created** (M_Toon, (0.45, 0.45, 0.48), Emissive 1.2) |
| `MI_Toon_NetFrame`, `MI_Toon_NetMesh`, `MI_Toon_Board`, `MI_Ice_Night`, `MI_Trim_Blue/Orange`, `MI_Trim_Beacon`, `MI_Toon_Bumper`, `MI_Toon_GoalShield`, `MI_Zone_*` | | | existed already |
| `MI_Trim_BeaconOpaque` | beacon mode 2 only (`jam.rink.beacon 2`; default 0 = off) | `MI_LL_NetFrame` MID | not created (mode 2 is a sketch; the code would use one fixed colour for both teams) |

PIE listing on `hockey.regular` after the change: every construction component carries a `/JamAssets/Look/` instance, no `/Game/LookLab/` material is left on the rink.

## Duplicates removed

- `MI_Toon_Board`: B13's painted kick plate (22 cm) and rail band (10 cm) are off **under the cel tier only**. A literal `KickHeightCm 0` / `RailBandCm 0` would have removed B14's broadcast kick and rail paint (`BcastToonBase` reads the same parameters), so `M_Toon` gained `CelConstructOff` (default 0) and a `CelConstructGate` node after `Construct`: when `CelConstructOff` is 1 and `MPC_Look.Clay` is 0 the base is the plain `TeamColour`; at Clay 1 / -1 the node returns its input. `MI_Toon_Board` `CelConstructOff` 0 -> 1; `KickHeightCm` 22 and `RailBandCm` 10 stay for clay and broadcast. The feature stays in the master.
- `LR_L2_Night`: the 66 `Con_L2_Night_Stanchion_*` (folder `Con/GlassLine`) deleted; rig 218 -> 152 actors, saved. A's 73 stanchions (98 on 120 x 60) are the only ones.
- Net grid: **kept, single source**. A's netting (`Nets` s1) is drawn with B's `MI_Toon_NetMesh`, so the 10 cm grid is the material on A's geometry, not a second net; A draws no grid of its own.

## Glass

A's `Glass` component now loads `MI_Toon_Glass` by name (lane A25), so B creating that name closes the glass on B's side: no PIE swap needed. Open for the lead: `MI_Toon_Glass` is used on every tier (A's code has one name), so clay and broadcast now show the night glass where B14 wanted `MI_Bcast_Glass` under broadcast.

## Checks

- PIE `hockey.regular`: one kick plate, one rail, one stanchion set, one net, glass on B's instance (stills `s0_board`, `s0_netclose` against `before_board`).
- PIE `hockey.arena.large` (rig scale 1.333): rig 128 `Con_` (194 - 66), nothing doubled; towers and headers stand outside A's 120 x 60 boards.

Next: step 1 (sky), 2 (spots), 3 (ice), 4 (outline), 5 (fill), 6 (haze cards, saved hidden).
