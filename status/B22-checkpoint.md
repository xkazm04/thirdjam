# B22 checkpoint after item 1: the bulbs' reflections in the near ice (device B)

Lane B22, device B, machine clock 01:21 to (see below). Main tree `C:/Users/kazda/jam/thirdjam` (branch `dev-b`), editor on port 8000 (mine). No git, no build, no frame numbers (another editor on the GPU).

## Item 1: done

**`M_Ice`** gains one Custom node, **`B22BulbRefl`** (group `Clay bulb reflection (B22)`), between `B19IceEmis` and the emissive output. It returns `Orig` unless `MPC_Look.Clay >= 0.5`, `ClayIceB19 > 0` and the new switch **`BulbReflB22 > 0`** (master default 0; `MI_Ice_Night` sets 1).

How it works, per ice pixel:
- the view ray is mirrored about the ice normal: B19's scratch normal (tangent to world through a `Transform` node), flattened by `BRNormalK` 0.35 so the scratches break the streaks without scattering them into dust;
- for each string (up to 12; positions and anchor heights from three vector parameters), the mirrored ray meets the string's vertical plane; the three nearest bulbs on the string's sag parabola (the same curve the mesh uses: `z = anchor - 260 (1 - (y / 3700)^2) - 21`, bulbs every 185 cm from -3520) are tested by their true perpendicular distance to the ray;
- each bulb adds a soft disc of radius 16 cm grown by roughness (`a = rough^2 x BRBlurK` + `BRStreak`) and by the pixel footprint of the unperturbed mirror ray (anti-aliasing: `fwidth`), 6 x longer along the reflection's vertical than across it (`BRStretch`), energy-normalised;
- times the ice's own Fresnel (Schlick, F0 = 0.08 x `ClayIceSpec`), times (1 - 0.6 x scratch mask) and (1 - snow), clamped at `BRMax` 40;
- where the bulb itself is on screen, `BRScreenFade` 0.3 lowers it (screen-space reflections show those as fine sparkle at the same positions: no double image seen);
- all lengths scale with `RinkHalfLength / BRLayoutHalfLen` (4500), so the wide and large rinks, whose rig is scaled, stay matched.

Instance values (`MI_Ice_Night`, all clay-only): `BulbReflB22` 1, `BRCount` 8, `BRYaw` 90 (strings run along world Y), `BRStrX_A` (-5250, -3750, -2250, -750), `BRStrX_B` (750, 2250, 3750, 5250), `BRStrZ_*` 880, `BRSag` 260, `BRHalfSpan` 3700, `BRBulbY0` -3520, `BRPitch` 185, `BRBulbCount` 39, `BRDrop` 21, `BRBulbR` 16, `BRColour` (1.0, 0.40, 0.11), `BRIntensity` 95, `BRStreak` 0.022, `BRBlurK` 0.5, `BRStretch` 6, `BRNormalK` 0.35, `BRFootK` 1, `BRScreenFade` 0.3, `BRScratchK` 0.6, `BRMax` 40.

Pixel shader instructions (`get_statistics`, the whole material): **1,072 before, 1,302 after** (the loop body is counted once; 8 strings x 3 bulbs run per clay ice pixel).

Saved: `M_Ice` 97,749 -> 120,018 bytes (01:45), `MI_Ice_Night` 22,439 -> 26,692 bytes (01:49). Rig not touched yet.

The key lamps' discs: not added. The four key spot lights already put their own specular hot spot (source radius 30 cm) in the ice where the lamp discs would reflect; an analytic disc would double them.

Stills (devlog/shots): see the list at the end of this file.

Against the estimate `est-p3-barn.png` from the same low camera: the near and middle ice now carry long warm vertical streaks of every bulb, broken by the scratches, as the estimate promises; they are thinner and less golden than the estimate's, and they fade into the bright key pool on the right of the frame, where the estimate keeps them. Not done in item 1: the estimate's open boards with a red rail (device A's glass and rail) and its standing glowing rings (item 5).

Stills (`devlog/shots/`): `20261113-015239-B-b22-low-before`, `-b22-low-after` (lane B19's low camera, zones cut, t 4.6), `-b22-game-before`, `-b22-game-after` (game camera at the faceoff, same frozen frame, analytic term off / on), `-b22-refl-views-ab` (8 m at the boards, 1.9 m face view, 10 m: left before, right after), `20261113-015240-B-b22-refl-yaw4` (four frames of a slow yaw: the streaks stay with their bulbs).

Next: item 2 (bulbs in the frame).
