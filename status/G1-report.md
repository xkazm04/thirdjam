# G1 report: the GPU lane (device B), hand-back 1: the puck's ring

Lane G1, device B, machine clock 14:15 to 14:37. Tree `C:\Users\kazda\jam\wt\b-arena` (`lane/b-arena` at 68bf1cb1, clean at the start). Build `ThirdJamEditor Win64 Development` PASS (366 s). One GPU editor, port 8008, pid 32208, **left running and idle** (no play session, no dirty package) for jobs 2 and 3; the editors on 8004 and 8006 were not touched. Cvars put back: `jam.Cut hockey.regular`, `r.Editor.Viewport.OverridePIEScreenPercentage 1`. No commit, no push, no cook. Scratch (scripts, every capture): `C:\Users\kazda\AppData\Local\Temp\claude\6d8eaecb999746c2\1f5c7b1e-557b-4c70-b752-8370d9dbef0b\scratchpad\g1` (`halve.py`, `ab.sh`, `meas.py`, `bk\SM_PuckDisc_before.uasset` = the HEAD file).

## 1. Done / not done
- **Done: job 1, the puck's white outer ring half as thick.** One file changed: `Props/SM_PuckDisc.uasset`.
- Not done yet: job 2 (menu backdrop), job 3 (neon arena), job 4 (menu scene).

## 2. The item
**Which part draws the white ring.** The pale ring at the game camera is `SM_PuckDisc`'s `Halo` section (flat disc, radius 0 to 30 cm, `MI_Trim_PuckHalo`, overlay `M_PuckGlow` on clay); the puck itself is A's cylinder at scale 0.24 (radius 12 cm). So the white band seen round the puck was 30 - 12 = **18 cm** wide. The `Ring` section (radius 25 to 35 cm, `MI_Fx_PuckRing`) draws nothing visible at these views. Materials untouched.

**What changed.** Geometry only: every vertex of the disc outside the puck's radius moved inward, r' = 12 + (r - 12) / 2 (448 of 546 vertices; z, UVs, slots, materials and triangle count 704 unchanged). Halo rings now at 0 / 6 / 12 / 15 / 18 / **21 cm** (were 0 / 6 / 12 / 18 / 24 / 30); the `Ring` section now 18.5 to 23.5 cm (was 25 to 35), keeping its place relative to the halo's edge. Bounds 47 x 47 x 2.9 cm (were 70 x 70 x 2.9). A sets nothing on the disc (A-to-B-0028: A's flattening is gone from main; `JamPawns.cpp` keeps the disc at world scale 1), so the asset alone carries the change.

**Proving numbers** (same frozen frame, only the disc mesh swapped, GPU, clay and night):
- Ring width: **18 cm before, 9 cm after.** Close camera (3 m, FOV 30): halo 525 px -> 367 px across, puck 210 px = 24 cm, so the pale band is 157 px -> 78 px each side (8.75 px/cm), on clay and night alike.
- Pale halo pixels at the game camera, faceoff crop: clay 395 -> 174, night 338 -> 149. At 30 m (crop round the puck): clay 58 -> 31, night 46 -> 25.
- **Readable at 30 m**: yes on both tiers: the dark puck on a thinner pale strip still reads at the frame centre (clay the weaker of the two: pale ring on pale ice, the dark puck carries it).

**Stills** (all in `C:\Users\kazda\jam\wt\b-arena\devlog\shots\`, each with a `.txt` caption):
- `C:\Users\kazda\jam\wt\b-arena\devlog\shots\20261114-143500-B-g1-puckring-ab-clay.png` (A/B sheet: faceoff, 30 m, close)
- `C:\Users\kazda\jam\wt\b-arena\devlog\shots\20261114-143501-B-g1-puckring-ab-night.png`
- `C:\Users\kazda\jam\wt\b-arena\devlog\shots\20261114-143502-B-g1-puckring-after-clay-faceoff.png`
- `C:\Users\kazda\jam\wt\b-arena\devlog\shots\20261114-143503-B-g1-puckring-after-night-faceoff.png`
- `C:\Users\kazda\jam\wt\b-arena\devlog\shots\20261114-143504-B-g1-puckring-after-clay-30m.png`
- `C:\Users\kazda\jam\wt\b-arena\devlog\shots\20261114-143505-B-g1-puckring-after-night-30m.png`

**What I looked at (Read tool):** both A/B sheets (the band visibly half as wide in every row, the puck unchanged, the clay disc keeps the same cream colour so the `M_PuckGlow` overlay still draws); the full before stills at the game camera on clay and night and the night close-up; the full after 30 m clay frame (the puck reads as a small dark dot with a thin pale rim beside the centre circle) and the full after night 30 m frame.

How measured: PIE on `Rink`, cut = `hockey.regular` with the tier forced to `p3_clay` / `l2_night` by a copy of the cartridge passed to `jam.Cut` (scratch `clay.cartridge.json`, `night.cartridge.json`); world slowed at load, frozen with `slomo 0.0001` at t = 2.02 to 2.04 s (just after the centre drop); stills through `CaptureGameViewport` (1536 x 883), the first capture after each camera move thrown away. 30 m camera: 30 m from the puck, 25 degrees off the long axis (straight behind, the human's own figure covers the puck), 3.5 m up, FOV 70. The "before" mesh for the same-frame A/B was a temporary duplicate (`SM_PuckDisc_G1Before`, never saved, deleted; not on disk).

## 3. Not verified
- A fast-moving puck (the B38 crescent/ladder question on clay): the halo is smaller, the overlay unchanged; not re-run.
- Broadcast tier and any other tier: not looked at.
- The faceoff stills carry the drop's blue puffs over the puck (frozen 0.04 s after the drop); no still of the halved ring on an open-play puck far from the puffs.
- Cook: not run by me.

## 4. For device A
Nothing to wire: `/JamAssets/Props/SM_PuckDisc` keeps its path, slots (`Halo`, `Beacon`, `Ring`, `Marker`) and materials; at scale 1 it is now 47 cm across with a 9 cm pale band round the 24 cm puck. If the cut's `puck_radius` stat changes the puck's size, the band stays 9 cm outside a 12 cm puck only: a bigger puck covers more of it (as before).

## 5. Decisions for the operator
- Half as thick is exact (18 -> 9 cm). If on clay at a distance it now reads too faint, the next step is brightness, not width (`M_PuckGlow` `Glow` 18 is unchanged).

## 6. Next three steps
1. Job 2: the menu backdrop `UI/T_UI_MenuBackdrop` (look at device A's `docs\experiments\menu-2\*.png` first), two or three candidates at 1920 x 1080, import, choose.
2. Job 3: `Rigs/LR_N1_Neon` after reading how `LR_P3_Clay` / `LR_L2_Night` load by tier.
3. Job 4 if time before 17:30.

## 7. Ledger row
| Game/Plugins/JamAssets/Content/Props/SM_PuckDisc.uasset | mesh | 0.03 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | puck halo disc, 47 x 47 x 2.9 cm (bounds -0.2 to 2.7 cm), 704 triangles, no collision; slots Halo (MI_Trim_PuckHalo, overlay M_PuckGlow), Beacon, Ring, Marker; the pale Halo ring outside the 12 cm puck halved from 18 to 9 cm (vertices outside r 12 cm moved to 12 + (r - 12) / 2: halo radius 30 -> 21 cm, Ring section 25-35 -> 18.5-23.5 cm), seen on the GPU on clay and night (lanes B38, B44, G1) (28360 bytes) |
