# Lane B2b: dressing pass 2, the generated set props (device B, when an editor slot is free)

You are lane B2b on device B of the Third Jam (team Hokej, theme Director's Cut). Pass 1 dressed the night arena as a sound stage with 289 basic-shape
actors in the rig map `LR_L2_Night` (`status/B2-report.md`). Lab lane S3 has generated seven props with Hyper3D Rodin that are far better than the basic-shape
versions (`status/B-skater-s3.md`, section "Set props"; sheets `devlog/shots/*-B-lab-s3-prop-*.png`). You import them and replace the hero pieces.

Read first: `briefs/B-machine-notes.md`, `status/B2-report.md` (the label list and where each piece stands), `status/B-skater-s3.md` ("Set props"),
`contracts/assets-naming.md`, `tools/skills/look-painted-cel.md` section 1 (stencil). Tree: the lead's tree `C:/Users/kazda/jam/thirdjam`, editor on port 8000
(the lead starts it and tells you); **no git**. You write only `Game/Plugins/JamAssets/Content/Arena/**`, the rig map `LR_L2_Night.umap`, stills through
`node devlog/shot.mjs`, and `status/B2b-report.md`.

## Sources (read-only): `C:/Users/kazda/jam/skater-lab/s3/props/<name>/base_basic_pbr.glb`

| Prop folder | Asset name | Replaces in the rig (pass 1 labels from the report) | Target size in the arena |
|---|---|---|---|
| the mid-grey spotlight (the second one, not the all-black first) | `SM_FilmLight` | the eight film lights on stands | 4 m tall |
| director's chair | `SM_DirectorChair` | the chair | 1.2 m tall; but place two more beside each bench |
| `clapper2` | `SM_Slate` | the slate | 3 m tall prop "oversized on purpose" by the boards near centre, plus one at 1 m beside the chair |
| camera dolly | `SM_CameraDolly` | add: two on the dolly track along the long side, one behind each net | 2 m tall |
| camera crane | `SM_CameraCrane` | the crane silhouette behind the far net; add one behind the near net | 7 m reach |
| flight cases | `SM_FlightCases` | about a third of the 75 case cubes (keep the rest as cubes, they are fine as filler) | 1.2 m tall stack |
| `truss2` | `SM_TrussLamps` | nothing (the pass 1 truss stays); try one above each end wall and keep it only if it reads | 6 m long |

## Rules

- Import each GLB as one static mesh under `/JamAssets/Arena/`. Rodin scales every object to about 1.8 m on its longest axis: set the import or actor scale
  from the target size above, and check the bounding box after import (the flight cases' box is larger than the object reads).
- **Base colour only, 1K.** Each GLB carries three 2K maps (diffuse, normal, ORM). Import or keep only the diffuse as `T_<Name>_BaseColor` with
  `MaxTextureSize` 1024 (the cel look steps the light in a post pass and needs no normal or ORM detail); delete the other two textures. One material
  `M_PropCel` under `Arena/` (a copy of the `M_Toon` graph with a texture parameter `BaseColorTex` instead of the colour, same `Emissive` and rim
  parameters, flat-fill Emissive about 1.5), one instance per prop. Look at every texture once for letters, numbers or logo-like marks; if you find one,
  do not use that prop and say so.
- Stencil 1 on every prop (ramp and outline), no collision, `cast_shadow` off except the film lights and chairs. Everything stays outside the 140 x 70 m
  footprint plus 2 m, as in pass 1. Remove the pass 1 actors a prop replaces; keep labels `Dress_L2_Night_<n>` (continue the numbering).
- Triangles: each prop is about 9.5k. Count the total after placement (pass 1 was 48.8k; keep the whole dressing under 250k) and say whether Nanite is
  on for these meshes and what it does to the outline pass in a still (if Nanite breaks custom depth or the outline, turn it off for them).
- Verify with the three views of pass 1 (own end, centre faceoff, far end) plus two close views of the best props, through PIE and
  `CaptureGameViewport`, with A's real rink if it is in `/Game/Maps/Rink` by then (never save that map), else the throwaway layout. Nothing may cover
  the ice, the far net or its beacon. File stills as `b2b-<label>`.

## Ledger and report

Rows for the lead: meshes and textures `ai-generated`, tool "Hyper3D Rodin", the Rodin task id in `note` (ids are in `status/B-skater-s3.md`), sizes from
disk. Report `status/B2b-report.md`: per prop PASS/FAIL, scale used, triangles, texture size on disk, where placed; the actor-label list of the saved map
(B1's 12, pass 1's survivors, yours); total size of `Arena/`; stills; what you removed; what does not read from the game camera. Time box 90 minutes.
