# Lane A28: the P4 figure slice: a generated hockey player that plays (device A, from H10.4)

You are lane A28 of the Third Jam (team Hokej). Ruling R-0025 point 3 in `contracts/RULINGS.md`: P4 is a masterclass in graphic design from commercial APIs and free assets, able to
play hockey; the toolkit-fit slice comes first: the recurring animated character, in motion, at the game camera. Read first: R-0025, `docs/experiments/MESHY.md` and
`docs/experiments/ANIMATION-SOURCES.md` (what was proven on Meshy, Rodin, Tripo and the Fab pack earlier today), `contracts/skater-visual.md` (C-0001 v3, `puck_socket`),
`contracts/assets-naming.md` (C-0004 ledger), `docs/experiments/BLENDER.md` (the toy figure as the scale reference: 1.6 m toy, 1.85 m broadcast, stick 1.3 m), the registry notes below,
`briefs/A26-figure-on-ice.md` (the skeletal visual that will drive your figure; coordinate by status file, do not edit A26's files), `status/B-skater-s3.md` on `origin/dev-b`
(`git show`, read-only: how B made T-B2 from a Rodin mesh and a Meshy rig).

Keys in `.env`: `MESHI_API_KEY` (Meshy), `RODIN_API_KEY` (Hyper3D Rodin), `FAL_API_KEY` (fal.ai, which hosts Tripo3D; use it for Tripo unless `TRIPO_API_KEY` appears). Never print a
key, never write one to a file, never send one anywhere but the vendor's API. Ledger every generation (tool, task id, credits or dollars, size) in `ASSETS.md` and in your status.

You own: `tools/p4/**` (generation scripts and job lists), `Game/Content/P4/**` (imports: `SK_P4_*`, `T_P4_*`, physics assets), `docs/experiments/P4-FIGURE.md`, `status/A28.md`.
No editor of your own unless needed for a retarget (then MCP port 8015, only while working, at most three editors on the machine; headless `UnrealEditor-Cmd -run=ImportAssets`
preferred, as lane A22 did: see `tools/blender/import_toy.json` and `verify_toy.py`). Run no git command but the read-only `git show origin/dev-b:<path>`; never touch
`Game/Plugins/JamAssets`.

## Registry rules that bind this lane
- Rig binding is three things: a named target skeleton (UE5 Manny for P4, state it with the asset), an explicit source-to-target bone table owned in one place
  (`tools/p4/bonemap-<vendor>.json`), and a totality check (every bone of the Manny chains spine, arms, legs is the target of a row; report the unmapped set, not a boolean).
  Structural success is not a rig test: a full clip must be played and watched.
- Scale: a generated humanoid is judged by a reference-skeleton size check (pelvis height, arm span, hand size against Manny), not by its bounding box.
- One character end to end before any batch.

## Job
1. **Three vendors, one brief, one judgement.** The same prompt family ("stylized hockey player, toy-like proportions, helmet with visor, blue jersey with white trim, number 7, hockey
   gloves, no stick, A-pose, game-ready low poly, clean topology") to Meshy (text-to-3D, then its rig with preset `ue5_mannequin`), Rodin (mesh, then Meshy's rig on it, B's route),
   Tripo through fal (text-to-3D with rig if the endpoint offers it). Record per vendor: wall time, credits, triangle count, texture sizes, rig preset and bone count, mapping totality,
   the three hardest things the mesh got wrong (hands, helmet, skates are the usual ones). A separate stick mesh (A22's `tools/blender/out/` stick or a Meshy one) attaches at the
   hand socket; the puck at `puck_socket` (add the socket to the skeleton in the import step).
2. **Play a full clip on each**: the Manny `MM_Walk_Fwd` / `MM_Jog_Fwd` as the smoke test, then the Fab Hockey Player skate clips if the operator has bought the pack
   (`Game/Content/P4/Fab/` if present; say so in status if not), through an IK Retargeter from the vendor skeleton to Manny. Watch it: a 2-second viewport capture per figure
   (`devlog/shots/`), judged with lane A27's eyes tool (`tools/eyes/README.md`, preset `figure`) when it exists.
3. **Collision as a cosmetic layer**: a physics asset per figure (capsules on pelvis, spine, head, limbs); a knocked_down that blends to ragdoll for 20 frames then back to the sequence,
   driven only by the sim's SetKnockedDown (never feeding back). Prove in a bot match through A26's skeletal visual once A26 reports it works (its status file says when), else note it
   as the next step.
4. **Free assets**: list what Fab offers free under the Standard licence for a hockey rink, stands, benches, lights (names, sizes, licence lines) without downloading anything that
   needs a sign-in; the operator does sign-ins.
5. `docs/experiments/P4-FIGURE.md`: the vendor table, stills, the recommendation (which vendor, which rig, what it costs per figure and per team), and the three blockers in order.

Report: the table, the still paths, the recommendation, the blockers, the ledger rows added. Time box 2 hours; interim at 45 minutes with the first figure imported and walking.
