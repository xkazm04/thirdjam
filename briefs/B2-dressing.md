# Lane B2: the arena around the rink, dressed as a sound stage (device B)

You are lane B2 on device B of the Third Jam (team Hokej, `GAME.md`, theme `docs/THEME.md`). The rink, boards and nets are built by device A from the
cartridge. You build **what stands around them**: the room and the dressing that makes the night arena read as a film set, because the theme is
Director's Cut. Arena choice: baseline L2-b "indoor, house lights down, spotlights on", **no crowd** (`docs/HOCKEY.md` 6).

Repo `C:/Users/kazda/jam/thirdjam`, branch `lane/b-dressing`, own worktree. Read first: `briefs/B0-lead-plan.md`, `tools/skills/look-painted-cel.md`
(1, 4, 5, 9), `contracts/assets-naming.md`, `contracts/look-tiers.md`, `prep-reference/STYLE-PATHS.md` (P2, T2) for the mood. **Reference stills are
never imported, traced or used as textures.** You own `Game/Plugins/JamAssets/Content/Arena/**`. You use B1's materials; you do not edit them.

Machine rules as in `briefs/B1-look.md`. You get the editor only when the lead hands it to you.

## The dressing list (first pass: engine basic shapes and procedural meshes only, nothing downloaded, nothing generated)

Everything sits outside the boards, never on the ice, never between the third-person camera (6 m behind, 3.5 m above the skater) and the play.
Dressing lives **inside the rig map** `/JamAssets/Rigs/LR_L2_Night` (C-0002 allows "look dressing" there; it is fixed in world space, rink centre at the
origin, ice top at Z = 0). The rink is 120 x 60 m by default and may load as 90 x 45 or 140 x 70, and the rig does not know which: so every piece
stands **outside the 140 x 70 footprint plus 2 m** (|x| > 72 m or |y| > 37 m) and inside B1's room (footprint plus 12 m). On the small cut the set
simply stands further back. Meshes go to `/JamAssets/Arena/` (`SM_`, `BP_`), labels in the map `Dress_L2_Night_<n>`.

| # | Piece | Asset | Built from | Stencil | Why it is here |
|---|---|---|---|---|---|
| 1 | Dark room: floor apron, four walls, ceiling at 28 m | **built by lane B1** as `Room_L2_Night_<n>` in the rig; you only check it and add to it | planes, cubes; `MI_Toon_Dark` | 0, no shadow | "house lights down"; hides the void, gives Lumen something to occlude the sky |
| 2 | Light truss: two beams along the long axis at 25 m with four lamp heads over the spot positions | `SM_Truss_Beam`, `SM_Lamp_Head` | cubes and cylinders; `MI_Toon_Dark`, lens disc `MI_Trim_Beacon` at low Emissive | 1 on the lamp heads | the spots must come from something; a lit lens disc sells it |
| 3 | Film lights on stands: 6 to 8 around the boards, 4 m tall, tripod legs, barn-door head aimed at the ice | `BP_FilmLight` (`SM_Stand`, `SM_LampHead`, `SM_BarnDoor`) | cylinders, cubes | 1 | the first thing that says "set" instead of "arena" |
| 4 | Director's chair at the centre line, outside the boards on the camera side | `BP_DirectorChair` | cubes, thin cylinders; seat and back in trim orange | 1 | the theme in one prop; visible at the faceoff |
| 5 | Slate (clapperboard) 3 m tall leaning by the boards near centre, sticks striped by material, **no text** | `BP_Slate` | two cubes, a hinge cylinder; stripes from a material function, not a texture | 1 | pairs with the clapperboard faceoff sound (B3) |
| 6 | Camera crane silhouette behind one net, dolly track along one long side | `BP_Crane`, `SM_DollyTrack` | cubes, cylinders | 1 | depth behind the far net; tells the eye where the far end is |
| 7 | Cable runs and flight cases along the walls | `SM_Case`, `SM_Cable` (splines optional) | cubes, flattened cylinders | 0 | fills the apron cheaply; no outline so it stays quiet |
| 8 | Player benches and two doors in the boards (visual only) | `BP_Bench` | cubes | 1 | scale reference for the figures |
| 9 | Exit signs: two small emissive rectangles high on the walls, **no letters** | `MI_Trim_*` on planes | planes | 0 | points of colour in the black, keeps the ceiling from reading as sky |

Rules for every piece: no text, letters, numbers, logos or brand shapes; no textures above 1K (none at all in pass 1); thin emissive parts at stencil 0
or the outline turns them black; nothing casts a shadow onto the ice except the boards and nets A owns (set `cast_shadow` off on the room and on
anything taller than the boards within 5 m of them).

## Second pass (needs the operator, after pass 1 is on screen)

Generated props through Meshy for pieces 3 to 6 if the basic-shape versions look poor in the stills: one prompt each, 20k triangles, one 1K texture,
"painted cel, flat colours, no text, no logos". The **pick among results is the operator's**; write the candidates and thumbnails into a handoff note
and keep going with pass 1 meanwhile. Every kept file gets an `ai-generated` ledger row with the task id.

## Acceptance

- [ ] The pieces above exist under `/JamAssets/Arena/` and are placed in `LR_L2_Night` outside the 140 x 70 m footprint plus 2 m.
- [ ] Three PIE screenshots at 1920x1080 through `CaptureGameViewport` from the third-person camera position: own end, centre faceoff, far end. In none
      of them does dressing cover the ice, the far net or its beacon.
- [ ] `GetFrameTimings` with and without the dressing visible: the difference is reported (target under 0.5 ms on device B; the gate is A's).
- [ ] Triangle count of all dressing reported (target under 150k); every committed file has an `ASSETS.md` row; `check-assets-ledger.mjs` passes.

## Report

The list above with PASS/FAIL per piece, the three stills (via `node devlog/shot.mjs B <label> <file>`), the numbers, and which pieces you would
replace with generated props and why. Time box 2 hours for pass 1.
