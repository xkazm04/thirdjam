# Lane B9: the P3 clay-miniature look as a second tier on the same geometry (device B, from H9)

You are lane B9 on device B of the Third Jam (team Hokej). The shipped look is P2 painted cel at night (`LR_L2_Night`). The operator wants **P3, a clay
miniature**, in parallel as a second look tier (`handoff/A-to-B-0006.md` point 2; ruling R-0012): the same rink, nets, puck and figures, switched by
`game.look.tier` = `p3_clay`. Target: pass 1 on screen today, complete with the same readability tests as L2 by H36 (Fri 18:00).

Read first: `briefs/B-machine-notes.md`; `prep-reference/STYLE-PATHS.md` (sections "P3 Clay miniature": the prompts and the feasibility notes; look at the
three stills `prep-reference/images/path-p3-*.png`: reference only, never imported or traced); `tools/skills/look-painted-cel.md` (how the P2 masters,
the stencil convention and the rig are built: P3 reuses the masters); `contracts/look-tiers.md`; `status/B1-report.md`, `status/B1b-report.md`,
`status/B1c-report.md` (the rig today, single-editor frame numbers, the 90 x 45 pool layout); `handoff/B-to-A-0005.md` and R-0002 point 5 (device A assigns
B's material instances **by path** and sets stencils: so P3 must work **without** A swapping materials); `handoff/A-to-B-0004.md` (texture rules).

Tree and editor: as the lead's prompt says. You write `Game/Plugins/JamAssets/Content/Look/**` and a new rig map `Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap`
(a new file: say so in the report, it needs a cook-list line and the tier value from device A). You do not touch `LR_L2_Night`.

## The design constraint that shapes everything

Device A's code assigns `/JamAssets/Look/MI_Ice_Night`, `MI_Toon_Board`, `M_Toon` dynamic instances and so on, whatever the tier. So a tier can change the
picture only through (1) what its **rig map** contains: lights, sky, the post-process volume and its blendables, dressing; and (2) **global material
parameters** the masters read. P3 therefore is:

1. **A Material Parameter Collection** `MPC_Look` in `Look/` with scalars `Clay` (0 = cel, 1 = clay), `ClayBump`, `FeltRim`, and a vector `LookTint`. The masters
   `M_Toon`, `M_Ice`, `M_Trim`, `M_PropCel`, `M_SkaterTB2` read it: at `Clay` 1 the surface gets a fingerprint and thumb-dent normal, roughness about 0.85,
   a soft felt rim on stencil-1 surfaces instead of the cel rim, slightly desaturated warm colours; at `Clay` 0 every pixel is what it is today (prove it:
   the L2 stills must not change by more than 1 level).
2. **A tiny Blueprint actor** `BP_LookParams` in `Look/`, placed in the rig map, that writes its exposed values into `MPC_Look` on BeginPlay and restores the
   defaults on EndPlay. `LR_P3_Clay` carries one set to `Clay` 1; `LR_L2_Night` carries none (defaults). If two rigs are ever loaded, the last one wins: say so.
3. **The rig `LR_P3_Clay`**: no toon-ramp and no outline blendable (clay is lit, not inked); a warm key from high on one side with soft shadows, a warm
   bounce fill, a cool rim from behind; a post volume with a gentle **depth-of-field band** (tilt-shift) whose focus sits between the player and the puck
   and whose blur is capped so the far net stays readable (the feasibility note warns about exactly this); mild bloom, warm grade, fixed exposure. Lights
   laid out for the default 90 x 45 m rink, actor labels `PPV_P3_Clay`, `Sky_P3_Clay`, `Key_P3_Clay`, `Fill_P3_Clay_<n>`, `LookParams_P3_Clay`. A miniature set
   reads as a tabletop: a warm wooden "table" plane far below the room floor and a blurred "workshop" backdrop are pass 2.

The clay detail is **texture work**: one tileable fingerprint/thumb-dent height map and one felt-fibre map, 1K each, generated with Codex (ruling R-0008;
`codex` is on PATH; generator pattern `prep-reference/tools/gen.mjs`: copy it to your scratch folder, one image per run, keep the prompt next to the output,
ledger as `ai-generated`, tool "Codex image generation") or, if generation stalls for 20 minutes, procedural noise in the material. Convert height to normal
in the material or on import (BC5, `LossyCompressionAmount` Medium, no virtual texture, per A-to-B-0004).

## Deliverables, pass 1 (today)

1. `MPC_Look`, the masters reading it, the proof that `Clay` 0 leaves L2 unchanged (before/after stills of the real game on `l2_night`, measured).
2. `BP_LookParams`, `LR_P3_Clay` with the lights and post volume, the two textures.
3. The real game in PIE with the clay rig streamed in place of the night rig (load it by hand in PIE and unload `LR_L2_Night`: the tier value does not
   exist yet; never save `Rink`): stills from the game camera at the faceoff, mid-ice and looking at the far net, with the toy figures and with the two
   humanoid visuals (`jam.Cut hockey.test.visual-b2`).
4. Readability, same tests as L2: puck findable at 10, 24 and 40 m; blue and orange separate at 60 m; the far net and its marker readable **with the depth
   of field on**; say plainly where clay loses to cel.
5. Frame cost against `l2_night` (pairs; mark "unconfirmed" unless you are the only editor), and what the depth of field alone costs.
6. What device A must add: the tier value `p3_clay`, the map path `/JamAssets/Rigs/LR_P3_Clay.LR_P3_Clay`, a cook-list line; and whether the trim strips and
   beacon (A's, emissive) need a different `Emissive` at this exposure.

## Report

`status/B9-report.md`: PASS/FAIL per deliverable with evidence; every new parameter name; files with sizes and ledger rows; stills (`b9-<label>`); the honest
verdict on whether this reads as clay or as "cel without ink"; what pass 2 needs. Time box 2.5 hours for pass 1.
