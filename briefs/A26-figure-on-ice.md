# Lane A26: the Blender toy skater on the ice as a skeletal visual (device A, from H10.3)

You are lane A26 of the Third Jam (team Hokej). R-0024 point 1: A22's Blender toy figure goes on the ice as a skeletal visual next to the code-built one; the better one ships after a
PIE comparison. Read first: `contracts/skater-visual.md` (C-0001 v3: every call a visual implements; `puck_socket`), `docs/experiments/BLENDER.md` (section 5 maps each call to a
sequence; section 6 the risks: bone-scale arm stretch, sequence time from the sim frame, determinism), `Game/Source/JamGame/Private/ToySkaterVisual.cpp` (the code-built figure, the
calls it implements, how the pawn drives it), `Game/Source/JamGame/Private/JamPawns.cpp` (ApplySimFrame), `Game/Content/Figures/` (SK_SkaterToy, SKEL_SkaterToy, AS_SkaterToy_*),
`Game/Source/JamCore/Tests/VisualSwapTests.cpp` (Jam.Bot.VisualSwap: a visual never changes the hash), `docs/experiments/FEEL.md`, R-0021 to R-0024.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a26` (the Director creates it at the current main; build it yourself, unity is off; your editor from it on MCP port
8013, only while proving, at most three UnrealEditor.exe on the machine; wait for other UnrealBuildTool instances). Run no git command; never touch `Game/Plugins/JamAssets`.

You own: `Game/Source/JamGame/{Public,Private}/SkeletalSkaterVisual.*` (new), the `game.skater.visual` value `toy_skeletal` (schema, limits, README, the game mode's visual factory:
one line each), `Game/Content/Figures/**` (materials for the figure: team colour instances through the same parameter names the toy figure uses, see `contracts/skater-visual.md`),
`docs/experiments/FIGURE.md`, `status/A26.md`.

## Job
1. `USkeletalSkaterVisual` (C++, a USkeletalMeshComponent child or an actor component on the pawn as the toy visual is): implements every C-0001 v3 call by selecting a sequence and
   a position **from the sim frame** (frame counts, never delta time): UpdateMotion -> idle / skate (cycle phase = distance travelled), carry, SetStickAim -> per-bone blend of
   `stick_root` between aim_left / aim_right (never a whole-action blend, see BLENDER.md risk 2), PlayPoke / PlayLift / PlayPass / shoot wind-up and release from SetShotCharge +
   PlayShoot, PlayCheck, SetKnockedDown (knocked_down then get_up), SetFaceoff (faceoff_ready), PlayFaceoffDraw (draw), SetGoalieStance (three stances), PlayFoulReaction, SetHasPuck
   (the puck draws at `puck_socket`, already done by the pawn when the socket exists), SetBoost. Team colours and the number through material parameters; stencil 1 for the outline;
   the toy emissive 2.2 kept (R-0016).
2. `game.skater.visual: "toy_skeletal"` selects it; `hockey.test.visual-a26.cartridge.json` under `Game/Content/Cartridges/` for the proof (do not touch the shipped cuts).
3. Proof: `Jam.Bot.VisualSwap` green with the new value (identical hash); a PIE bot match on the night rig with stills at 10 / 24 / 40 m and the three distances for the code-built
   figure on the same frozen frame (lane A23's time-dilation freeze trick is in `status/A23.md`); the stick verbs visible: a poke, a faceoff, a goalie stance, a knocked-down figure.
   Then run lane A27's eyes tool on the pairs when it exists (`tools/eyes/README.md`), otherwise state the comparison yourself.
4. `docs/experiments/FIGURE.md`: the mapping table (call -> sequence -> frames), the cost (ms per figure, 6 figures, ProfileGPU), what reads better and worse than the code figure
   at each distance, and your recommendation for the shipped default.

Report: the recommendation with the still paths, the VisualSwap result, the cost, the file list for the Director. Time box 2 hours; interim at 45 minutes with the figure skating in PIE.
