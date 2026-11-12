# Lane B4: import the scripted original skater as track T-B1 (device B, next free editor slot)

You are lane B4 on device B of the Third Jam (team Hokej). Lab lane S4 built an **original** articulated toy hockey skater and fifteen clips with a Blender
script (`status/B-skater-s4.md`, including the section "Polish pass"; sheets `devlog/shots/*-B-lab-s4b-*.png`). It has the best motion of every candidate: exact
loops, zero pelvis travel, a real stick. You import it and put it behind `IJamSkaterVisual` as **`BP_SkaterVisual_TB1`** (handoff `B-to-A-0008` asks the Director to
redefine track T-B1 as this figure; until the ruling, it lives beside T-B2 and nothing ships by default).

Read first: `briefs/B-machine-notes.md`; `status/B-skater-s4.md` (bones, clips, export settings, what Unreal is expected to do); `contracts/skater-visual.md`;
`contracts/assets-naming.md` with R-0001 point 6 (prefixes `SK_ SKEL_ AS_ ABP_`); lane B5's report `C:/Users/kazda/jam/wt/b-skater/status/B5-report.md`
(sections on the AnimBP, the visual Blueprint, the pitfalls, and "Re-runnable scripts": the scripts themselves are in
`C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b5/`; copy and adapt, do not edit them in place);
`tools/mcp/ANIMATION.md` (the `UTJAnimGraphEdLibrary` helper for states and transitions).

Tree and editor: as the lead's prompt says. You write only `Game/Plugins/JamAssets/Content/Skaters/` files whose names contain `TB1` (lane B5 owns the `TB2`
files in the same folder in another worktree), stills through `node devlog/shot.mjs B tb1-<label> <file>`, and `status/B4-report.md`. **No git** unless
the lead's prompt gives you a lane branch.

## Source: `C:/Users/kazda/jam/skater-lab/s4/export/SK_Skater_S4_loopcut.fbx` (loops without the repeated last frame), `SM_S4_Stick.fbx`

## Deliverables

1. **Import**: `SK_SkaterTB1` (5.9k triangles, 9 material slots, no textures), `SKEL_SkaterTB1` (24 bones; say whether Unreal kept `root` as the root or added
   an `Armature` bone, which way the figure faces, its height in cm and whether the scale is 1), fifteen `AS_SkaterTB1_<Clip>` sequences named after the takes
   (`Idle_Faceoff`, `Stride`, `Glide`, `Stop`, `Shot_Slap`, `Shot_Wrist`, `Check`, `Fall`, `Fall_Forward`, `GetUp`, `Celebrate`, `Lean_Left`, `Lean_Right`,
   `Crossover_Left`, `Crossover_Right`). Check in the editor that `Stride`, `Glide` and `Idle_Faceoff` wrap without a visible hitch (the first and last frame poses
   differ by one frame step, not zero and not a jump). Real sockets `stick_socket`, `puck_socket`, `head_socket` on the bones of the same names.
2. **Materials**: no texture. One dynamic instance of `/JamAssets/Look/M_Toon` for the `Jersey` slot (`TeamColour` from `SetTeam`, `Emissive` 2.0, `RimStrength`
   0.6), and constant instances `MI_SkaterTB1_<Slot>` for the others in the same folder: `Trim` white, `Helmet` (the team colour too, darkened by half: a second
   dynamic instance), `Gloves` and `Pads` near black (the eyes use `Pads`: keep it untinted), `Skin`, `Skate`, `Blade` light steel, `Stick` wood. Stencil 1 on the mesh.
3. **`ABP_SkaterTB1`**: variables `Speed` (m/s), `TurnRate` (rad/s), `bKnockedDown`, `bFallForward`. Locomotion: blend `Idle_Faceoff` (speed under 0.3) ->
   `Glide` -> `Stride` (full at 8 m/s, play rate scaled by speed); a lean blend on top by `TurnRate` towards `Lean_Left` / `Lean_Right` (full lean at 3 rad/s; if a
   blend space is hard to author through MCP, a two-way blend by a normalised alpha is enough); `Down`: `Fall` or `Fall_Forward`, hold the last frame, then
   `GetUp` (from the face-down fall blend 0.2 s into `GetUp`). Montages: `Shot_Slap` for `PlayShoot`, `Check` for `PlayCheck`; `Celebrate`, `Stop`, `Shot_Wrist`
   and the crossovers imported, wired only if time allows.
4. **`BP_SkaterVisual_TB1`** (Actor, `/JamAssets/Skaters/`): implements all eight calls of `JamSkaterVisual`. `SetKnockedDown(true, DirX, DirZ)`: choose
   `Fall` when the direction is within 90 degrees of the figure's backward axis, else `Fall_Forward`, and yaw the mesh so the fall goes along the direction.
   `SetTeam` safe before BeginPlay. `SetNumber`, `SetHasPuck`, `SetBoost` may be no-ops; say which.
5. **Verify** in PIE with `LR_L2_Night` streamed in: stills at 1920 through `CaptureGameViewport`: from the game camera (6 m behind, 3.5 m above), stride,
   lean, slap shot, both falls, blue and orange side by side, six figures from 10 to 60 m. Drive the calls from Python as lane B5 did.
   Measure nothing while another editor renders, or say "unconfirmed, two editors up" beside every number.

## Report

`status/B4-report.md`: PASS/FAIL per deliverable with evidence; what Unreal did with the root, axis, scale and take names (lane S4 could not verify these);
files with sizes and ledger rows (`original`, tool "Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python"; kinds mesh / anim / material / other); stills;
your honest comparison with the T-B2 figure's stills if they exist by then; what is weak; what is not verified. Time box 90 minutes.
