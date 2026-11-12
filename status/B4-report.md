# B4 report: scripted original skater imported as track T-B1 (device B)

Lane B4, 2026-11-12, about 11:41 to 12:00 local (well inside the box). Branch `lane/b-skater`, worktree `C:/Users/kazda/jam/wt/b-skater`. Own editor on port 8002,
closed through `ExitEditor` at the end (the in-memory `Rink` edits were discarded by opening a blank map first; `Rink` was never saved). Another editor (port 8000)
was up on the same GPU the whole time: **no frame numbers were taken**. Scratch scripts: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b4/` (called `$S4B` below).
Source: `C:/Users/kazda/jam/skater-lab/s4/export/SK_Skater_S4_loopcut.fbx` (read-only). `SM_S4_Stick.fbx` was not needed: the stick is skinned into the main mesh.

## Verdict in one paragraph

The scripted figure imports in one pass with default FBX options, plays all fifteen clips on its own skeleton, takes team colour, and sits behind
`IJamSkaterVisual` in `BP_SkaterVisual_TB1` with an AnimBP that does idle, glide-to-stride, a turn lean with crossovers, both falls, get-up, and the
shot and check montages. Two things in lane S4's export did not survive Unreal as expected and were fixed on our side: the two **2-frame lean holds
imported as the wrong pose** (re-exported as 30-frame loops), and the **root bone carries scale 100 and a 90 degree roll** (sockets compensate). From the
game camera it reads cleaner than T-B2 and moves better; close up T-B2 has more charm.

## Deliverables

| # | Deliverable | Result | Evidence |
|---|---|---|---|
| 1 | Import: `SK_SkaterTB1`, `SKEL_SkaterTB1`, 15 `AS_SkaterTB1_<Clip>`, sockets | **PASS** (with the lean fix below) | One FBX import (`i1_import.py`), mesh + 15 takes; renamed by `i2_rename_inspect.py`. 24 bones, 9 slots, 0 textures; bounds z 0 to 172.7 cm; all 15 clips on `SKEL_SkaterTB1`, root motion off. Sockets `stick_socket`, `puck_socket`, `head_socket` are real `SkeletalMeshSocket`s on the bones of the same names; in PIE: all exist, world scale 1.0, `puck_socket` at z 0, 78 cm ahead and 10 cm left of the figure. Triangles: 5,932 per lane S4's Blender count (no triangle accessor found in UE Python or MCP; UE reports 3,893 render vertices). |
| 1b | Loop wrap of `Stride`, `Glide`, `Idle_Faceoff` | **PASS** (measured on the imported assets, plus PIE) | `i3_check.py` chains the bone poses of every key: max joint jump last key to first key vs the clip's median step per frame: Stride **0.611 cm** (median step 3.94), Glide **0.004** (0.089), Idle_Faceoff **0.004** (0.094), Crossover_Left 1.402 (4.25). Each wrap is below one frame step: no repeated frame, no jump. Same numbers as lane S4's Blender measurement. In PIE the stride loops without a visible pop (watched in stills only; no video). |
| 2 | Materials | **PASS** | `m1_materials.py`: nine constant instances `MI_SkaterTB1_<Slot>` of `/JamAssets/Look/M_Toon`, assigned by slot name. Jersey (blue default) Emissive 2.0, RimStrength 0.6; Helmet = team colour x 0.5; Trim white; Gloves and Pads near black, Emissive 0 (eyes stay untinted); Skin; Skate dark; Blade light steel; Stick wood. In the Blueprint: two dynamic instances (Jersey, Helmet) created at BeginPlay. Stencil 1 on the mesh component (checked in PIE: `render_custom_depth` true, stencil 1). Still `tb1-teams`. |
| 3 | `ABP_SkaterTB1` | **PASS** | Variables `Speed`, `TurnRate`, `bKnockedDown`, `bFallForward`; compiles `BS_UP_TO_DATE`. States Idle (`Idle_Faceoff`), Skate, Fall, FallForward, GetUp. Skate plays the 2D blend space `BS_SkaterTB1_Skate` (X Speed 0..8 m/s, Y TurnRate -3..3 rad/s): Glide at 0 and 1 m/s (rate 0.8), Stride at 8 m/s; Lean_Left at TurnRate +3 (0 and 3 m/s), Crossover_Left at (8, +3); mirrored for the right. Idle to Skate at Speed > 0.3 (0.3 s), back at < 0.25 (0.4 s); Idle/Skate to Fall on `bKnockedDown and not bFallForward`, to FallForward on `bKnockedDown and bFallForward` (0.15 s); Fall to GetUp (0.15 s) and FallForward to GetUp (**0.2 s**) when it clears; GetUp to Skate automatic at the end (0.3 s). Fall, FallForward, GetUp do not loop (hold the last frame). Then `Slot DefaultSlot`. Montages `AM_SkaterTB1_Shot_Slap` (PlayShoot), `AM_SkaterTB1_Check` (PlayCheck). `Stop`, `Shot_Wrist`, `Celebrate` imported, not wired; the crossovers are wired through the blend space. |
| 4 | `BP_SkaterVisual_TB1` | **PASS** | Actor in `/JamAssets/Skaters/`, implements `JamSkaterVisual` (Python `does_implement_interface` true). All eight calls run from Python on PIE actors without error. `SetTeam`: stores `PendingColour`/`bHasColour`, applies to the Jersey and (x 0.5) Helmet instances when they exist, else at BeginPlay; **before-BeginPlay path verified**: `SetTeam(orange)` on the editor-world actor, then PIE: the figure is orange (still `tb1-prebegin`). `UpdateMotion`: Speed and TurnRate to the AnimBP, mesh world yaw = Facing yaw. `SetKnockedDown(true, DirX, DirZ)`: `Fall` when (DirX, DirZ) is behind the facing (dot < 0), else `Fall_Forward`; the mesh yaw is set so the fall runs along the direction (measured: dir (-1,0) gives yaw 360 = facing, backward fall; (1,0) gives 0 and Fall_Forward; (-0.71,0.71) gives 315). `SetKnockedDown(false)`: GetUp; the mesh yaw eases from the down yaw back to facing over the next 72 `UpdateMotion` calls (call-counted, deterministic); after a forward fall the down yaw is turned 180 degrees first so the head stays where it lies. `PlayShoot`, `PlayCheck`: montages (Check montage reported playing). `SetBoost`: Jersey `Emissive` 3.0 on any kind, 2.0 on `None`. **No-ops: `SetNumber`, `SetHasPuck`.** |
| 5 | Verify in PIE with `LR_L2_Night` | **PASS** (stills), frame cost **not measured** | `Rink` (in memory, never saved) + `LR_L2_Night` as sub-level, current level set back to `Rink`; calls driven from Python (`d0.py` + per-shot `d1.py`). Stills at 1920 through `CaptureGameViewport`, listed below. The first capture after a camera or figure teleport shows TAA ghosting; every filed still is a second capture. |

## What Unreal did (lane S4 could not verify these)

- **Root**: Unreal kept `root` as the skeleton root and added **no** `Armature` bone (24 bones). But it folded the `Armature` object's transform into `root`:
  the reference pose of `root` has **roll 90 degrees and scale 100**, the other bones' local translations are in metres, so every bone has world scale 100.
  The mesh itself is right: mesh component scale 1, figure 172.7 cm tall. Anything attached to a bone would arrive 100 times too large and rolled;
  the three sockets therefore carry the inverse (relative scale 0.01 and the inverse bone rotation, `s2_socket_xf.py`): measured world scale 1.0 and
  identity rotation at the reference pose. **So the answer to "is the scale 1": the mesh yes, the skeleton root no.**
- **Axis**: the figure faces **+X**, its left hand on -Y (clavicle_l at y -4, `puck_socket` 78 cm ahead on +X, 10 cm to -Y). No yaw on the mesh component
  (unlike T-B2 phase 1).
- **Scale**: no 100x or 0.01x on the mesh; Import Uniform Scale 1.0, Force Front X Axis off, Convert Scene on (defaults).
- **Take names**: `Armature|Armature|<Clip>` became `SK_SkaterTB1Armature_<Clip>` (destination name + `Armature_` + clip); renamed to `AS_SkaterTB1_<Clip>`.
- **Loop wrap**: loopcut takes import as N keys and (N-1)/60 s (Stride 60 keys, 0.9833 s; Glide and Idle 120 keys, 1.9833 s; crossovers 40 keys, 0.65 s);
  the wrap is one frame step (numbers above).
- **Two body meshes**: body and stick became one skeletal mesh with 9 slots, as expected. UE slot order: Pads, Jersey, Trim, Skin, Helmet, Blade, Gloves, Skate, Stick
  (the Blueprint uses element indices 1 = Jersey and 4 = Helmet for its dynamic instances).
- **Lean holds broken (fixed)**: `Lean_Left` and `Lean_Right` (2-frame holds) imported with 3 keys, 17 tracks, and **identical poses equal to Glide's
  first frame** (the lean is lost). Fix in a copy of the export, `$S4B/lean_fix.py` (headless Blender 4.2, reads lane S4's `s4_skater_anim.blend`, writes only
  `$S4B/fbx/SK_Skater_S4_leans.fbx`): each lean is re-baked as a 30-frame loop (frames 0..29, no repeated last frame) holding the same pose plus a tiny
  sine wobble (0.001 on one quaternion component, about 0.1 degree; 0.5 mm on one location axis), all other export settings identical to `build_clips.py`.
  Re-imported onto `SKEL_SkaterTB1`: 30 keys, 24 tracks, left and right mirrored (pelvis roll -15.7 / +15.7 degrees). I did not isolate whether the
  length or the wobble is what fixed it. **For lane S4's script**: export holds as at least a 30-frame loop with a sub-degree wobble on every bone.
- **Anim-only re-import trap**: importing takes without the mesh (the lean fix) keys `root` at scale **1** instead of 100; the figure shrank to a 1.7 cm dot.
  Fixed by removing the `root` track from both lean clips (`i5_drop_root.py`, 23 tracks, root keeps its reference pose). Anyone re-importing a single
  clip must do the same.

## Stills (`devlog/shots/`, 1920 x 1085, PIE, `LR_L2_Night`)

- `20261112-115622-B-tb1-stride.png`: blue and orange Stride (7 and 8 m/s), game camera 6 m behind, 3.5 m up
- `20261112-115623-B-tb1-lean.png`: turn lean from behind, TurnRate +3 (left) and -3 (right) at 3 m/s
- `20261112-115623-B-tb1-slap.png`: slap shot frozen at wind-up (0.27 s) and contact (0.40 s)
- `20261112-115623-B-tb1-falls.png`: Fall (backward), Fall yawed 45 degrees, Fall_Forward
- `20261112-115623-B-tb1-getup.png`: after the get-up, skating again
- `20261112-115623-B-tb1-teams.png`: blue and orange side by side, faceoff
- `20261112-115623-B-tb1-six.png`: six figures from 10 to 60 m, all readable, team colours clear at 60 m
- `20261112-115624-B-tb1-vs-tb2.png`: **side by side with T-B2 from the game camera**, both Stride at 7 m/s (left T-B1, right T-B2)
- `20261112-115624-B-tb1-vs-tb2-front.png`: side by side from the front 3/4 (left T-B2, right T-B1)
- `20261112-115624-B-tb1-prebegin.png`: `SetTeam` before BeginPlay

## Comparison with T-B2 (my honest read of the stills)

- **From the game camera T-B1 reads better.** Its back is a clean block of team colour with a white hem stripe, the dark breezers separate it from the legs,
  the stick with its blade sits clearly to the right, and the striding legs read at 60 m. T-B2 from behind is a round helmet and a jersey with **orange-yellow
  mitts on the blue team** (the hue key leaves them), which muddies team reading, and its stick is a bladeless cylinder.
- **T-B1 moves better for this game**: exact loops, zero drift, both hands on a real stick, a slap wind-up you read at distance, leans and crossovers driven by
  `TurnRate`, falls in both directions, a get-up. T-B2's motion is more organic (mocap-like) but has one-handed shots, a fall that slides, and no forward fall.
- **Close up T-B2 has more appeal**: a face with eyes and a cartoon character, against T-B1's cage over a skin block ("programmer toy"). At the game camera's
  distance that difference mostly disappears.
- Cost: T-B1 0.94 MB mesh, no texture, about 5.9k triangles; T-B2 3.7 MB mesh + 2.5 MB texture, 19.7k triangles, with glyph-like texture marks lane B5 flagged.
  T-B1 is `original` throughout.

## What is weak

- The helmet at team colour x 0.5 still reads almost like the jersey under Emissive 1.5 (both bright); a white or near-black helmet might separate head and body better.
- Stride is only full at 8 m/s (as the brief asked); at 4 to 5 m/s the figure is half glide, half stride. Lane B5 put full stride at 4.5 m/s.
- Forward fall to get-up: `GetUp` starts lying on the back, so after `Fall_Forward` the body flips face-down to face-up in 0.2 s (the yaw turn keeps the head in place). A `GetUp_Front` clip would fix it.
- A knock-down during GetUp has no transition: the figure finishes the get-up, enters Skate, then falls.
- The lean from behind is subtle at low speed; the stick trails on the right in both leans (right-handed pose).
- The turn-lean sign (positive TurnRate = turning left = `Lean_Left`) follows the contract; not checked against A's pawn.
- Dark ice at grazing camera angles in the night rig (`tb1-slap`, `tb1-vs-tb2-front`): the figures still read, the ice does not.
- `SetNumber` and `SetHasPuck` are no-ops (no number surface, no marker).

## Not verified

- Frame cost of six figures (another editor rendered on the same GPU; nothing measured).
- The wrap "without a visible hitch" was judged from numbers and stills, not from video.
- `Celebrate`, `Stop`, `Shot_Wrist` (imported only); the check montage was seen to play, not looked at; crossovers at full speed only in a blurred still.
- Swap through the cartridge and bot tests (no consumer of `game.skater.visual` on this tree).
- Triangle count inside Unreal (taken from lane S4's Blender measurement).

## For the lead to decide

1. The Director's ruling on `B-to-A-0008` (redefine T-B1 as this figure); until then it lives beside T-B2 and nothing ships by default.
2. Whether lane S4 re-exports with the root at unit scale (lane B5's `rootfix.py` approach) so attachments need no socket compensation, or we keep the compensated sockets.
3. Lane S4 to adopt the lean-hold export change (30-frame loop with wobble), and whether to author `GetUp_Front`.
4. Helmet colour (team x 0.5, white, or dark) and where full stride should sit (4.5 or 8 m/s).

## Files (bytes), all under `Game/Plugins/JamAssets/Content/Skaters/`

| File | Bytes |
|---|---|
| SK_SkaterTB1.uasset | 940,746 |
| SKEL_SkaterTB1.uasset | 11,629 |
| AS_SkaterTB1_Celebrate.uasset | 206,051 |
| AS_SkaterTB1_Check.uasset | 167,631 |
| AS_SkaterTB1_Crossover_Left.uasset | 155,532 |
| AS_SkaterTB1_Crossover_Right.uasset | 159,403 |
| AS_SkaterTB1_Fall.uasset | 164,559 |
| AS_SkaterTB1_Fall_Forward.uasset | 165,850 |
| AS_SkaterTB1_GetUp.uasset | 212,975 |
| AS_SkaterTB1_Glide.uasset | 225,698 |
| AS_SkaterTB1_Idle_Faceoff.uasset | 225,697 |
| AS_SkaterTB1_Lean_Left.uasset | 188,966 |
| AS_SkaterTB1_Lean_Right.uasset | 188,978 |
| AS_SkaterTB1_Shot_Slap.uasset | 182,520 |
| AS_SkaterTB1_Shot_Wrist.uasset | 148,948 |
| AS_SkaterTB1_Stop.uasset | 176,558 |
| AS_SkaterTB1_Stride.uasset | 185,334 |
| BS_SkaterTB1_Skate.uasset | 19,606 |
| AM_SkaterTB1_Shot_Slap.uasset | 8,745 |
| AM_SkaterTB1_Check.uasset | 8,943 |
| ABP_SkaterTB1.uasset | 272,727 |
| BP_SkaterVisual_TB1.uasset | 320,406 |
| MI_SkaterTB1_Jersey.uasset | 9,245 |
| MI_SkaterTB1_Helmet.uasset | 8,966 |
| MI_SkaterTB1_Trim.uasset | 8,954 |
| MI_SkaterTB1_Gloves.uasset | 8,621 |
| MI_SkaterTB1_Pads.uasset | 8,954 |
| MI_SkaterTB1_Skin.uasset | 8,954 |
| MI_SkaterTB1_Skate.uasset | 8,960 |
| MI_SkaterTB1_Blade.uasset | 8,615 |
| MI_SkaterTB1_Stick.uasset | 8,960 |
| **Total, 31 files** | **4,417,731** |

Nothing under `_Scratch/`; no T-B2 file touched; no map saved.

## Ledger rows ready to paste

Columns as in lane B5's report (path | kind | version | source type | tool | url | (empty) | date | device | shipped | note); check the order against `ASSETS.md`.
`shipped = n` until the Director's ruling on T-B1.

```
| Game/Plugins/JamAssets/Content/Skaters/SK_SkaterTB1.uasset | mesh | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | skater-lab/s4 build_skater.py, SK_Skater_S4_loopcut.fbx; sockets added by lane B4 |
| Game/Plugins/JamAssets/Content/Skaters/SKEL_SkaterTB1.uasset | other | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | skeleton from the same import; root at scale 100 (see B4 report) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_Idle_Faceoff.uasset | anim | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | build_clips.py, loopcut |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_Stride.uasset | anim | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | build_clips.py, loopcut |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_Glide.uasset | anim | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | build_clips.py, loopcut |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_Stop.uasset | anim | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | build_clips.py; imported, unwired |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_Shot_Slap.uasset | anim | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | build_clips.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_Shot_Wrist.uasset | anim | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | build_clips.py; imported, unwired |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_Check.uasset | anim | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | build_clips.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_Fall.uasset | anim | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | build_clips.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_Fall_Forward.uasset | anim | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | build_clips.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_GetUp.uasset | anim | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | build_clips.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_Celebrate.uasset | anim | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | build_clips.py; imported, unwired |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_Lean_Left.uasset | anim | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | build_clips.py pose, re-exported as 30-frame loop by lane B4 lean_fix.py; root track removed |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_Lean_Right.uasset | anim | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | build_clips.py pose, re-exported as 30-frame loop by lane B4 lean_fix.py; root track removed |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_Crossover_Left.uasset | anim | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | build_clips.py, loopcut |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_Crossover_Right.uasset | anim | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | build_clips.py, loopcut |
| Game/Plugins/JamAssets/Content/Skaters/BS_SkaterTB1_Skate.uasset | anim | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | 2D blend space Speed x TurnRate (lane B4) |
| Game/Plugins/JamAssets/Content/Skaters/AM_SkaterTB1_Shot_Slap.uasset | anim | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | montage for PlayShoot |
| Game/Plugins/JamAssets/Content/Skaters/AM_SkaterTB1_Check.uasset | anim | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | montage for PlayCheck |
| Game/Plugins/JamAssets/Content/Skaters/ABP_SkaterTB1.uasset | other | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | AnimBP: idle, skate blend, lean, falls, get-up, DefaultSlot (lane B4) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | implements IJamSkaterVisual (lane B4) |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB1_Jersey.uasset | material | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | instance of M_Toon; team colour via dynamic instance |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB1_Helmet.uasset | material | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | instance of M_Toon; team colour x 0.5 via dynamic instance |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB1_Trim.uasset | material | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | instance of M_Toon, white |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB1_Gloves.uasset | material | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | instance of M_Toon, near black |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB1_Pads.uasset | material | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | instance of M_Toon, near black, also the eyes |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB1_Skin.uasset | material | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | instance of M_Toon |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB1_Skate.uasset | material | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | instance of M_Toon |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB1_Blade.uasset | material | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | instance of M_Toon, light steel |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB1_Stick.uasset | material | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | instance of M_Toon, wood |
```

## Re-runnable scripts (`$S4B`, run from the worktree with the editor on 8002; `run.sh <file.py>` wraps `execute_python`)

| # | Script | Does |
|---|---|---|
| 1 | `i1_import.py`, `i2_rename_inspect.py`, `i3_check.py` | import mesh + takes; rename skeleton and takes, list bones, slots, clip frames; root track and loop-wrap measurement |
| 2 | `m1_materials.py`, `s1_sockets.mjs`, `s2_socket_xf.py` | material instances and slot assignment; the three sockets; socket scale/rotation compensation |
| 3 | `lean_fix.py` (Blender), `i4_leans.py`, `i5_drop_root.py` | lean re-export, re-import and blend-space re-point, drop the root track |
| 4 | `a0_bs_montage.py` | blend space and the two montages (run before step 3 on a fresh build, then step 3) |
| 5 | `a1_create.py`, `a2_vars.mjs`, `a3_states.py`, `a4_contents.mjs`, `a5_run.mjs` (`a5_props.py`) | AnimBP; run once per fresh asset (a second `a4` duplicates nodes) |
| 6 | `b1_create.mjs`, `b2_components.py`, `dsl.mjs b3_bp.dsl`, `b4_compile.py` | visual Blueprint |
| 7 | `v1_setup.py`, `startpie.sh`, `d0.py` + `d1.py`, `cap.sh`, `z_save.py`, `z_discard.py` | test level, PIE, drive, capture, save, discard `Rink` before `ExitEditor` |

## Commits on `lane/b-skater`

- `d766df5` import: mesh, skeleton, 15 clips, 9 material instances, sockets
- `2c1c8bb` AnimBP, blend space, montages, `BP_SkaterVisual_TB1`, lean re-export, stills
- the commit holding this report (hash in the final message)

---

# Phase 2 (first note, written at 12:16 for the lead at once)

**Unreal rejects lane S4's track-stripped FBX.** Importing `C:/Users/kazda/jam/skater-lab/s4/export/SK_Skater_S4_loopcut.fbx` (12:09, after
`strip_tracks.py`) fails in the FBX SDK: `Cannot open FBX file ... FInterchangeFbxParser::LoadFbxFile: Cannot load the FBX file. There was an error
when parsing the file.` Nothing is imported. Fallback used (S4's own advice, not my phase 1 route): I ran unmodified copies of lane S4's
`build_skater.py` and `build_clips.py` with my scratch folder as the lab dir, **without** `strip_tracks.py`
(`$S4B/s4lab/export/SK_Skater_S4_loopcut.fbx`). That file imports cleanly: `root` at identity rotation and scale 1.0, every bone at world scale 1,
`root` keeps identity keys (harmless). **Lane S5 must not run the strip step on the hybrid file either.** Lane S4 should drop `strip_tracks.py`
from `run_all.sh` (or make it optional) so its published export is the importable one. T-B1 refresh committed as `131000e`.

## Phase 2 result (lead's follow-ups A and B, 12:12 to 12:30; editor on 8002 closed through `ExitEditor`, `Rink` discarded, never saved)

### Both new files: what Unreal did

| File | Result |
|---|---|
| Lane S4 `export/SK_Skater_S4_loopcut.fbx` (12:09, track-stripped) | **Rejected** by the FBX SDK (`Cannot load the FBX file. There was an error when parsing the file.`). Nothing imported. |
| Lane S5 `export/SK_Skater_S5_loopcut.fbx` (12:19, track-stripped) | **Rejected**, same error (copied to `$S4B/s5in/probe_1219.fbx` first, so not a half-written file). |
| S4 fallback: lane S4 `build_skater.py` + `build_clips.py` run unmodified into `$S4B/s4lab/`, no strip step | **Imports.** 24 bones, `root` the root, **identity rotation, scale 1.0**, every bone at world scale 1; faces +X, 172.7 cm; 16 takes (`GetUp_Front` new); `root` keeps identity keys (harmless). Leans arrive as 30-key loops with 24 tracks, mirrored: **`lean_fix.py` and `i5_drop_root.py` are no longer needed.** Loop wraps unchanged: Stride 0.611 cm (frame step 3.94), Glide 0.004, Idle 0.004, Crossover 1.402. |
| S5 fallback: lane S5 `retarget.py` (12:19:11, byte-identical) with its byte-identical `in/` inputs into `$S4B/s5lab/`, no strip step | **Imports.** 32 bones, `root` identity at scale 1, 170 cm, faces +X; 3 slots `Material_0`, `Trim`, `Stick`; 16 takes named `SK_SkaterTB2Armature_<Clip>` (renamed), 32 tracks each; loop wraps Stride 0.442 cm (step 2.87), Glide 0.005, Idle 0.005. No texture imported (none embedded). |

**The strip step is the only thing between the published files and Unreal.** Both labs should publish without `strip_tracks.py`.
Socket compensation: the scale part is gone (all bones at scale 1). I kept the rotation alignment (`s2_socket_xf.py`) because the socket bones
still carry Blender bone axes (`puck_socket` and `head_socket` have roll 90 at rest); measured socket world rotation identity and scale 1.0 on both meshes.

### A. T-B1 refresh: PASS (commit `131000e`)

Route: **delete and rebuild with my scripts** (`$S4B/p2/00_clean_tb1.py`, then `i1`/`i2` on the fallback file, `p2/m1_materials.py`, `s1`/`s2`,
`p2/a0_bs_montage.py`, `a1`, `a2`, `p2/a3_states.py`, `p2/a4_contents.mjs`, `p2/a5_run.mjs`, `b1`, `b2`, `p2/b3_bp.dsl`, `b4`).
- `GetUp_Front` wired after `Fall_Forward` (0.2 s blend), plain `GetUp` after `Fall`; both return to Skate automatically at the end. The phase 1
  "turn 180 degrees before the get-up" trick is gone.
- **A knock-down during a get-up interrupts it**: GetUp and GetUpFront each have transitions to Fall and FallForward (0.15 s); the Blueprint resets
  its yaw-easing counter on a new knock-down. Seen in `tb1-falls`: the leftmost figure was knocked down 0.4 s into its get-up and lies again.
- Blend space: Glide at 0 and 1 m/s, **full Stride at 5 m/s** (rate 1.0) and at 8 m/s (rate 1.3); crossovers at (5, +-3) and (8, +-3) with the same rates; leans at (0 to 3, +-3).
- Helmet: team colour x **0.75** (dynamic instance and the `MI_SkaterTB1_Helmet` default).
- Stills: `devlog/shots/20261112-121953-B-tb1-stride.png`, `20261112-121953-B-tb1-falls.png` (right to left: Fall, Fall_Forward, GetUp mid,
  GetUp_Front mid, knock-down during a get-up; game time frozen with global time dilation), `20261112-121954-B-tb1-teams.png`.

### B. T-B2 rebuilt on the hybrid: PASS (commit `d793aff`)

- Removed lane B5's set: `AM_SkaterTB2_Shoot` deleted; every other old TB2 clip, blend space, montage, mesh, skeleton, AnimBP and Blueprint was deleted
  and rebuilt under the same name from the hybrid (the old `AS_SkaterTB2_{Stride,Glide,Idle_Faceoff,Shot_Slap,Shot_Wrist,Fall,GetUp,Celebrate}` names
  now hold S4 clips retargeted by S5). `M_SkaterTB2` kept unchanged: KeyGain 6, KeyBias 0.15, JerseyLum 0.08 not retuned; the recolour of the clean
  texture looks right in `tb2h-teams`. `T_SkaterTB2_BaseColor` re-imported in place from `skater-lab/s6/T_SkaterTB2_BaseColor_clean.png` (2048 x 2048, sRGB).
- Slots: `Material_0` = `M_SkaterTB2`, `Trim` = `MI_SkaterTB2_Trim`, `Stick` = `MI_SkaterTB2_Stick` (copies of the TB1 instances).
- Same structure as T-B1 (scripts in `$S4B/tb2/` are the T-B1 scripts with the names changed): same states, blend space axes and samples, both
  montages, the fall logic, `SetTeam` before BeginPlay. One difference: a single dynamic instance (the hue-key material colours suit and helmet
  together); `SetBoost` sets its `Emissive` to 1.0, back to 0.5 on `None`.
- **Shins and skates** (`tb2h-shins`): the shins are clean. One small mark remains: three short dark diagonal strokes on the outer side of the
  right boot. It comes from the **texture** (dark strokes inside a flat colour patch, not on a silhouette or crease that the outline pass draws).
  The laces on the boot tops are texture too and read as laces.
- Stills: `20261112-122757-B-tb2h-stride.png`; `20261112-122757-B-tb2h-slap.png` (side view: in the wind-up both mitts are clear of the helmet by
  about one mitt width and the lower mitt sits on the shaft; at contact the figure has turned away); `20261112-122757-B-tb2h-falls.png` (same five
  states as T-B1); `20261112-122758-B-tb2h-teams.png`; `20261112-122758-B-tb2h-six.png`; `20261112-122758-B-tb2h-shins.png`;
  `20261112-122758-B-tb1-vs-tb2h.png` (game camera, both striding at 7 m/s: left T-B1, right T-B2 hybrid).

### Comparison as they now stand

- **Same motion now**, so the choice is the figure. From the game camera T-B1 still reads more crisply: a block back with a white hem, dark breezers
  splitting body and legs, a visible stick blade. The hybrid from behind is one smooth team-colour shape with white stripes: its team colour reads
  very well, better than T-B1 at 60 m in `tb2h-six`, but the leg push is less legible without the dark breezers.
- **Close up and from the front the hybrid wins clearly**: a face with eyes, a rounded helmet, a character (`tb2h-teams`). T-B1 is the cage-and-block toy.
- **Hybrid weak spots seen**: the **mitts stay yellow on both teams** (outside the hue key) and on the blue team read like an orange accent; the left
  mitt leaves the shaft in some two-handed frames (S5 measured 12 cm in the slap); in lying poses the skates float (S5: 6 to 10 cm), visible in
  `tb2h-falls`; the texture mark on one boot; the same TAA ghost halo as T-B1 in the first second after a teleport.
- **Cost**: T-B1 4.6 MB in 32 files, 5.9k triangles, no texture, all `original`. T-B2 10.4 MB in 27 files, 19.9k triangles, one 2K texture, mesh and
  texture `ai-generated`.
- My call: the hybrid as the shipped T-B2 for looks; T-B1 stays the all-`original` fallback. If the mitts join the hue key, the hybrid has no
  team-reading problem left.

### Not verified (phase 2)

- Frame cost (the port 8000 editor was up throughout).
- `Celebrate`, `Stop`, `Shot_Wrist` on both figures (imported, not wired); the turn-lean sign against A's pawn.
- That the labs' published files import once the strip step is dropped: I proved it on byte-identical reruns of their scripts and inputs, not on their own files.

### Commits (phase 2)

`131000e` T-B1 refresh; `86291ab` early note on the stripped FBX; `d793aff` T-B2 hybrid; this section in the commit after it.

### Files after phase 2 (bytes)

T-B1: 32 files, 4,629,488 bytes (`SK_SkaterTB1` 941,511; `SKEL_SkaterTB1` 11,633; `AS_SkaterTB1_GetUp_Front` 215,819; other clips 144,573 to 226,171;
`ABP_SkaterTB1` 350,219; `BP_SkaterVisual_TB1` 312,403; `BS_SkaterTB1_Skate` 24,880; montages about 8,800; instances about 9,000 each).
T-B2: 27 files, 10,446,401 bytes (`SK_SkaterTB2` 3,744,531; `T_SkaterTB2_BaseColor` 2,356,732; `SKEL_SkaterTB2` 13,398; clips 166,459 to 316,995;
`ABP_SkaterTB2` 350,554; `BP_SkaterVisual_TB2` 287,855; `BS_SkaterTB2_Skate` 25,144; `M_SkaterTB2` 22,040; `AM_SkaterTB2_Shot_Slap` 8,912;
`AM_SkaterTB2_Check` 9,108; `MI_SkaterTB2_Stick` 8,960; `MI_SkaterTB2_Trim` 8,954).

### Ledger rows (phase 2), ready to paste

Remove: lane B5's `AM_SkaterTB2_Shoot.uasset` row, and lane B5's phase 2 rows for `SK_SkaterTB2`, `SKEL_SkaterTB2`, `T_SkaterTB2_BaseColor`, the eight
`AS_SkaterTB2_*` clips, `BS_SkaterTB2_Skate`, `AM_SkaterTB2_Check`, `ABP_SkaterTB2`, `BP_SkaterVisual_TB2` (same paths, new content below). `M_SkaterTB2.uasset`
keeps lane B5's row. T-B1: one file added (`AS_SkaterTB1_GetUp_Front`); every other phase 1 T-B1 row stands, its note changed to "rebuilt from lane S4's
12:09 export (cm, root at unit scale), strip step removed"; the two lean rows lose "re-exported by lane B4 lean_fix.py; root track removed".

```
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_GetUp_Front.uasset | anim | 0.1 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | build_clips.py 12:09 export; get-up from the face-down fall |
| Game/Plugins/JamAssets/Content/Skaters/SK_SkaterTB2.uasset | mesh | 0.1 | ai-generated | Hyper3D Rodin (mesh, texture) + Meshy (rig) + Blender retarget script (lane S5) | | | 2026-11-12 | B | y | Rodin task c07557c7-cf01-462a-97a3-9f0336c0ff31, Meshy rig 01a1cf39-74e0-7496-9b7c-cd8a09ffe4d3; root, sockets, boot weights by s5 retarget.py; includes the S4 stick (original) |
| Game/Plugins/JamAssets/Content/Skaters/SKEL_SkaterTB2.uasset | other | 0.1 | ai-generated | Hyper3D Rodin (mesh, texture) + Meshy (rig) + Blender retarget script (lane S5) | | | 2026-11-12 | B | y | skeleton of the hybrid import |
| Game/Plugins/JamAssets/Content/Skaters/T_SkaterTB2_BaseColor.uasset | other | 0.1 | ai-generated | Hyper3D Rodin (mesh, texture) + Meshy (rig) + Blender retarget script (lane S5) | | | 2026-11-12 | B | y | texture: Rodin diffuse cleaned by lane S6 (T_SkaterTB2_BaseColor_clean.png), 2K |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Idle_Faceoff.uasset | anim | 0.1 | original | Blender 4.2 Python script (lanes S4, S5) | | | 2026-11-12 | B | y | S4 build_clips.py clip retargeted onto R by S5 retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Stride.uasset | anim | 0.1 | original | Blender 4.2 Python script (lanes S4, S5) | | | 2026-11-12 | B | y | S4 build_clips.py clip retargeted onto R by S5 retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Glide.uasset | anim | 0.1 | original | Blender 4.2 Python script (lanes S4, S5) | | | 2026-11-12 | B | y | S4 build_clips.py clip retargeted onto R by S5 retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Stop.uasset | anim | 0.1 | original | Blender 4.2 Python script (lanes S4, S5) | | | 2026-11-12 | B | y | S4 build_clips.py clip retargeted onto R by S5 retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Shot_Slap.uasset | anim | 0.1 | original | Blender 4.2 Python script (lanes S4, S5) | | | 2026-11-12 | B | y | S4 build_clips.py clip retargeted onto R by S5 retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Shot_Wrist.uasset | anim | 0.1 | original | Blender 4.2 Python script (lanes S4, S5) | | | 2026-11-12 | B | y | S4 build_clips.py clip retargeted onto R by S5 retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Check.uasset | anim | 0.1 | original | Blender 4.2 Python script (lanes S4, S5) | | | 2026-11-12 | B | y | S4 build_clips.py clip retargeted onto R by S5 retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Fall.uasset | anim | 0.1 | original | Blender 4.2 Python script (lanes S4, S5) | | | 2026-11-12 | B | y | S4 build_clips.py clip retargeted onto R by S5 retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Fall_Forward.uasset | anim | 0.1 | original | Blender 4.2 Python script (lanes S4, S5) | | | 2026-11-12 | B | y | S4 build_clips.py clip retargeted onto R by S5 retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_GetUp.uasset | anim | 0.1 | original | Blender 4.2 Python script (lanes S4, S5) | | | 2026-11-12 | B | y | S4 build_clips.py clip retargeted onto R by S5 retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_GetUp_Front.uasset | anim | 0.1 | original | Blender 4.2 Python script (lanes S4, S5) | | | 2026-11-12 | B | y | S4 build_clips.py clip retargeted onto R by S5 retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Celebrate.uasset | anim | 0.1 | original | Blender 4.2 Python script (lanes S4, S5) | | | 2026-11-12 | B | y | S4 build_clips.py clip retargeted onto R by S5 retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Lean_Left.uasset | anim | 0.1 | original | Blender 4.2 Python script (lanes S4, S5) | | | 2026-11-12 | B | y | S4 build_clips.py clip retargeted onto R by S5 retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Lean_Right.uasset | anim | 0.1 | original | Blender 4.2 Python script (lanes S4, S5) | | | 2026-11-12 | B | y | S4 build_clips.py clip retargeted onto R by S5 retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Crossover_Left.uasset | anim | 0.1 | original | Blender 4.2 Python script (lanes S4, S5) | | | 2026-11-12 | B | y | S4 build_clips.py clip retargeted onto R by S5 retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_Crossover_Right.uasset | anim | 0.1 | original | Blender 4.2 Python script (lanes S4, S5) | | | 2026-11-12 | B | y | S4 build_clips.py clip retargeted onto R by S5 retarget.py |
| Game/Plugins/JamAssets/Content/Skaters/BS_SkaterTB2_Skate.uasset | anim | 0.1 | original | Blender 4.2 Python script (lanes S4, S5) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | 2D blend space Speed x TurnRate (lane B4) |
| Game/Plugins/JamAssets/Content/Skaters/AM_SkaterTB2_Shot_Slap.uasset | anim | 0.1 | original | Blender 4.2 Python script (lanes S4, S5) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | montage for PlayShoot |
| Game/Plugins/JamAssets/Content/Skaters/AM_SkaterTB2_Check.uasset | anim | 0.1 | original | Blender 4.2 Python script (lanes S4, S5) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | montage for PlayCheck (S4 Check clip) |
| Game/Plugins/JamAssets/Content/Skaters/ABP_SkaterTB2.uasset | other | 0.1 | original | UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | AnimBP, same structure as ABP_SkaterTB1 (lane B4) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB2.uasset | other | 0.1 | original | UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | implements IJamSkaterVisual (lane B4) |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB2_Stick.uasset | material | 0.1 | original | UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | copy of MI_SkaterTB1_Stick |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB2_Trim.uasset | material | 0.1 | original | UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | copy of MI_SkaterTB1_Trim (stick tape) |
```
