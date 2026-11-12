# Lane B1: painted-cel materials, post passes and the night rig (device B)

You are lane B1 on device B of the Third Jam (team Hokej, game `GAME.md`). You build the look the operator chose: **P2 painted cel, night arena**.
The jam is running: what you make is jam content.

Read first, in this order: `tools/skills/look-painted-cel.md` (the recipe; follow it), `contracts/look-tiers.md` (C-0002: rig map, actor labels),
`contracts/assets-naming.md` (C-0004: prefixes, folders), `contracts/skater-visual.md` (material parameter names), `contracts/RULINGS.md`,
`docs/experiments/LOOK-P2-NIGHT.md` (what it looked like and cost), `tools/mcp/README.md`, `handoff/B-to-A-0005.md` (what A will assign and drive).

## Where and how you work (this is not device A)

- Tree: `C:/Users/kazda/jam/thirdjam`, branch `dev-b`, the lead's own tree: this machine runs **one** editor, so there is no lane worktree.
  **Run no git command**; the lead commits what you report. Touch only `Game/Plugins/JamAssets/Content/Look/**` and `.../Rigs/LR_L2_Night.umap`,
  plus `devlog/shots/` through the helper and your report file `status/B1-report.md`.
- The editor is already running on the real project (`Game/ThirdJam.uproject`, MCP on `http://127.0.0.1:8000/mcp`). Drive it with `node tools/mcp/mcp.mjs`.
  You are its only driver. Do not stop it; if it dies, restart it with
  `. C:\Users\kazda\jam\ue-env.ps1; powershell -NoProfile -ExecutionPolicy Bypass -File tools\run\editor.ps1 -Project Game\ThirdJam.uproject -WaitMcp`
  (after a stop, port 8000 stays busy for about 100 s; after MCP answers, tools refuse for another 5 to 10 s with "running a slow task": retry).
- Python goes through `editor_toolset.toolsets.python_scripting.PythonScriptingTools.execute_python` with `{"script":{"content":"...","language":"Python"}}`;
  build the JSON args file with Node (`JSON.stringify`), pass it as `@file`. Keep scripts and args in your scratch folder, not in the repo.
- Never kill a process you did not start. Never save `/Game/Maps/Rink` or anything else under `Game/Content/` (device A's).
- Already checked by the lead on this GPU (NVIDIA N1X, x64 editor under emulation): the log says SM6; `r.CustomDepth` 3, Lumen GI and reflections 1,
  virtual shadow maps 1, mesh distance fields 1. Still do the GPU-profile check of recipe section 0 once your rig is up (`LumenSceneLighting`,
  `VirtualShadowMapMarkPages` present): if Lumen or VSM are not running on this GPU, stop and report.

## Deliverables, in order

1. **Masters** in `/JamAssets/Look/` (`ROOT = '/JamAssets/Look'`): `M_Toon`, `M_Trim`, `M_Ice`, and the two post-process masters named
   **`M_PP_ToonRamp`**, **`M_PP_Outline`** (the contract's prefix for a material is `M_`; the recipe's `PP_` names are not in C-0004). Graphs and values as in
   recipe section 3. Parameter names exactly `TeamColour`, `Emissive`, `Wet`, `Scuff` (C-0001). Every error list empty.
   `M_Ice`: the recipe hard-codes line positions for a 120 x 60 m rink. Replace the constants with scalar parameters `RinkHalfLength` (default 6000) and
   `RinkHalfWidth` (default 3000), in cm, and derive from them: goal lines at `RinkHalfLength - 700`, blue lines at `RinkHalfLength * 0.29`, centre circle
   radius 900, red centre line as is. Say in the report what you derived.
2. **Instances** (recipe section 4), same names, same folder, including the contract's `MI_PP_ToonRamp` and `MI_PP_Outline`. Add `MI_Trim_PuckHalo`
   (`M_Trim`, TeamColour 0.75, 1.0, 0.15, Emissive 18) for lane B3.
3. **`LR_L2_Night`**: open `/JamAssets/Rigs/LR_L2_Night` as the edited level, add and save. Actor labels from C-0002: `PPV_L2_Night` (unbound, values and both
   blendables from recipe section 5), `Sky_L2_Night`, `Spot_L2_Night_1` .. `_4` (x = -45, -15, 15, 45 m, z 25 m), and the dark room as static mesh actors of
   engine basic shapes labelled `Room_L2_Night_<n>`: a floor apron 5 cm below the ice, four walls, a ceiling at 28 m, all `MI_Toon_Dark`, no custom depth, no
   shadow, no collision. Size the room for the largest cut (rink 140 x 70 m) plus 12 m on every side, so no cut ever touches a wall.
   No `PlayerStart`, no gameplay actors, nothing that collides with the rink. **Do not** put board-top trim strips or net strips in the rig: they follow the
   rink size and are device A's (handoff 0005).
4. **Verify** on a throwaway layout of engine basic shapes: two capsule figures (stencil 1, `MI_Toon_Blue`, `MI_Toon_Orange`), an ice plane 120 x 60 m
   (stencil 2, `MI_Ice_Night`), a puck cylinder (stencil 3), a board segment and a net box (stencil 1), a camera 6 m behind and 3.5 m above a figure.
   Build it in an unsaved level or under `/JamAssets/_Scratch/`, with the rig map added as a streaming sub-level; **delete `_Scratch` before you report**
   (the ledger gate counts every file under the plugin's `Content/`). Screenshots only through PIE + `SceneCaptureToolset.SceneCaptureToolset.CaptureGameViewport`
   at 1920 x 1080 (the other capture tools skip post-process blendables): near figure; mid-ice; far net at about 100 m; `RampOn` 0 against 1; `OutlineOn` 0 against 1.
   File each with `node devlog/shot.mjs B <label> <file> "<caption>"`.
5. **Cost on this machine**: with the viewport realtime off, `PerfToolset.StatsToolset.GetFrameTimings` median of 60 samples, and one `TriggerGPUProfile`
   (`minInclusiveMs` 0): `gpuMs`, the Lumen nodes, the two `PostProcessMaterial` passes. These are device B numbers; the gate (4.5 ms for `l2_night`) is device A's.
   Record editor working set, free RAM and `nvidia-smi` memory used during PIE.
6. **Ledger rows** for the lead to paste: one line per file under `Game/Plugins/JamAssets/Content/Look/` and for the changed rig map, in the column order of
   `ASSETS.md` (`kind` material, or other for the map; `size_mb` with one decimal from the file on disk, write 0.1 for anything smaller; `origin` original;
   `tool` "UE 5.9 editor via MCP/Python"; date 2026-11-12; device B; shipped y).

## Not yours

Dressing meshes (B2), the puck kit and VFX (B3), the camera and everything under `Game/Content` or `Game/Source` (A), `LR_L1_Day`, `LR_L3_Neon`.

## Report

Write `status/B1-report.md`: PASS/FAIL per deliverable with the call that proves it; the asset list; frame and memory numbers; the stills you filed; every
name you chose that no contract gives; what in the recipe was wrong on this engine or this GPU; what is left. Save everything in the editor before you
finish and leave the editor running on `/Game/Maps/Rink`. Time box 2 hours: at the box, stop, save, and report what exists.
