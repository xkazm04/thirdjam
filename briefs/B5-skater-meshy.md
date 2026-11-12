# Lane B5: the generated skater, track T-B2 (device B, not before H8)

You are lane B5 on device B of the Third Jam (team Hokej). You produce a Meshy-generated, rigged, animated toy skater and put it behind
`IJamSkaterVisual` in a **test cartridge only**. It is an experiment: the game ships on device A's code-driven toy figure unless this track clears its
gate at H24 (`docs/HOCKEY.md` 5).

Repo `C:/Users/kazda/jam/thirdjam`, branch `lane/b-skater-tb2`, own worktree. Read first: `docs/experiments/MESHY.md` (all; section 6 is your command
list), `tools/meshy/README.md`, `tools/skills/skater-retarget.md` (steps 1-2 and 4-9), `tools/mcp/ANIMATION.md`, `contracts/skater-visual.md`,
`briefs/B0-lead-plan.md`. You own `Game/Plugins/JamAssets/Content/Skaters/TB2/**`.

## Preconditions (check each; if one fails, stop and report, do not improvise)

- [ ] `node tools/meshy/meshy.mjs balance` answers: the key is in `<repo>/.env` on this machine (it is gitignored; the operator places it). At least 120 credits.
- [ ] Blender 4.2 exists for `tools/meshy/inspect.py` (bones, triangles, pelvis travel). If it is not installed, skip the inspection and measure the same
      things after import in the editor (bone list, triangle count, root motion of the pelvis track); say which way you went.
- [ ] The seams are in: `IJamSkaterVisual`, `contracts/skater-visual.md`, the `JamEditor` helper `UTJAnimGraphEdLibrary`.
- [ ] The lead has given you the editor; no other generation, cook or capture is running.

## Steps

1. Run section 6 of `MESHY.md` with the scratch folder `C:\Users\kazda\jam\meshy-work` (outside the repo) and `MESHY_LEDGER` in it. One preview retry at
   most. **The pick of the preview is the operator's if they are awake; if not, take the first preview whose limbs are separate and record why.**
2. Import per `skater-retarget.md`: `SK_Skater_TB2` from the rig FBX, clips onto that skeleton with mesh import off, one 2K texture at most.
   Known risks from the experiment: the skeleton has 28 bones and no root (does the import add one); pelvis drift in the clips (strip it); no stick in
   the mesh (socket `stick_socket` on `hand_r`, plus `puck_socket`, `head_socket` as the contract names them).
3. Material: B1's `M_Toon` as parent with the generated texture as base colour times `TeamColour`, so the orange team is a parameter and not a second
   texture; stencil 1 on the mesh component.
4. `BP_SkaterVisual_TB2`: a component or actor implementing `IJamSkaterVisual`; the AnimBP through the `JamEditor` helper (states idle, stride, shot,
   down; blend by `Speed`). `SetNumber` may be a no-op with a note.
5. Swap it into the test cartridge the contract names; run A's bot tests if they exist on `main` by then.
6. Ledger every committed file: mesh, texture, rig and text-to-motion clips `ai-generated` (tool Meshy, task id in `note`); library clips (idle 0,
   Knock_Down 187) `sourced` with Meshy's licence and the preset id in `url` (Director ruling in `ASSETS.md`).

## Acceptance

- [ ] The figure skates, shoots and falls in PIE behind the interface; four stills via `node devlog/shot.mjs B tb2-<label> <file>`.
- [ ] Triangles under 25k, one texture at 2K or less, package size of `Skaters/TB2/` reported.
- [ ] Six figures on the ice: `gpuMs` and game-thread ms against six basic-shape stand-ins, on device B.
- [ ] Credits spent, from the ledger file.

## Report

PASS/FAIL per step, the numbers, what looked wrong (feet sliding, stick hand, loop seam), and your verdict on the promotion rule. Time box 4 hours
including generation; stop at the box.
