# Lane RAG report: a second engine without the GPU, and physics on the falls on a side copy

Tree `C:\Users\kazda\jam\wt\b-vfx`, fast-forwarded to `dev-b` 3e55f946 and built once (`ThirdJamEditor Win64 Development`, 111 actions, Result: Succeeded, 3 minutes). Port 8000. Lab folder `C:\Users\kazda\jam\rag-lab\`. My clock 01:41 to 02:25. One editor (pid 33564), left through `ExitEditor` and confirmed gone at 02:16:11. Nothing pushed. In the lead's tree only this report is mine.

## Evolution 0: a second engine without the GPU: it works, with one thing to know

**Command line that worked** (PowerShell, after `. C:\Users\kazda\jam\ue-env.ps1`, from `C:\Users\kazda\jam\wt\b-vfx`):

```
powershell -NoProfile -ExecutionPolicy Bypass -File tools\run\editor.ps1 -Project C:\Users\kazda\jam\wt\b-vfx\Game\ThirdJam.uproject -McpPort 8000 -ExtraArgs '-nullrhi -unattended'
```

which starts (from `Game\logs\editor.json`):

```
UnrealEditor.exe "C:\Users\kazda\jam\wt\b-vfx\Game\ThirdJam.uproject" -nosplash -ModelContextProtocolStartServer -ModelContextProtocolPort=8000 -abslog="C:\Users\kazda\jam\wt\b-vfx\Game\logs\editor-20261114-014447-0.log" -nullrhi -unattended
```

Proved in the brief's order (log `C:\Users\kazda\jam\wt\b-vfx\Game\logs\editor-20261114-014447-0.log`):

| Step | Result | Evidence |
|---|---|---|
| The null RHI is in use | yes | line 923 `LogModuleManager: InternalLoadLibrary: 'NullDrv' (.../UnrealEditor-NullDrv.dll)`; line 927 `Metadata set : rhiname="Null"`; line 930 `gpu=""`. **0 lines of `LogD3D12RHI`** in the whole log, counted again at the end of the lane. Windows' `GPU Process Memory` counter had **no instance for pid 33564**, while the other editor then running (pid 25556) showed 3.9 GB dedicated |
| MCP answers on 8000 | yes, within 28 s of the process start | `node tools/mcp/mcp.mjs wait --url http://127.0.0.1:8000/mcp`: `unreal-mcp 5.9.0-60351404`, protocol 2025-11-25 |
| A Python call runs | yes | `execute_python`: Python 3.11.8; the live engine has the fixed frame rate (True, 60) |
| An asset loads | yes | `BP_SkaterVisual_TB1_P3`, `SK_SkaterTB1_P3`, `SKEL_SkaterTB1_P3`; the editor world is `/Game/Maps/Rink` |
| A play session ticks | yes | `EditorToolset.EditorAppToolset.StartPIE {"options":{"playMode":"PlayMode_InEditorFloating","warmupSeconds":3}}` passes in 4.6 s: `UEDPIE_0_Rink`, `JAMCART loaded ... hockey.regular`, `JAMVISUAL humanoid_b1 tier=p3_clay`, 10 skaters, 10 figure actors, `HokejGameMode` |
| The figure's Blueprint logs as usual | yes | 104 `LogBlueprintUserMessages` lines in the first 40 s (`SK6RX hit`, `SK24 brk`, `SK22 brake`); a match knockdown at sim frame 521: `SK6KD down_f 113`, `SK4KD AS_SkaterTB1_P3_Fall`, `SK4UP ... GetUp`, `SK6REL ... end_minus_rel_f 0.000007`; the pawn's `JAMVIS knockdown / getup / hit / board` lines |

- **Tick rate: 60.2 sim frames per wall second** in the match (113 sim frames between log stamps 00.45.50:455 and 00.45.52:333). Memory 3.0 GB working set. 24 play sessions were started and stopped in this lane (counted in the log) with no hang, no assertion and a clean `LogExit: Exiting.` at the end.
- **The one thing to know: nothing is ever "rendered", so a figure's pose does not advance.** The figure meshes use `OnlyTickPoseWhenRendered`. Headless, a montage sat at position 0.0 and the pelvis stayed at z 57.33 on four figures. The Blueprint's time logic and its logs still run. One Python line per figure on the play-session instance fixes it (nothing saved): `mesh.set_editor_property('visibility_based_anim_tick_option', unreal.VisibilityBasedAnimTickOption.ALWAYS_TICK_POSE_AND_REFRESH_BONES)`. After it: pelvis z 72.8 to 74.6, `Stop_Hard` at 0.25 s.
- `ExitEditor` needs `{"bDiscardUnsavedChanges":true}` if anything is unsaved; the process was gone 6 s later.
- Not tried headless: a cook, `-run=pythonscript`, Niagara, sound. A modal dialog would have no window to click; none appeared.

**A second engine lane exists for everything that needs no picture:** Blueprint edits, physics assets, imports, bone and timing measurements, whole matches for their logs. My harness is reusable: `C:\Users\kazda\jam\rag-lab\py.mjs` (a Python file into the editor on 8000), `mc.sh` (one MCP call), `run.sh` (play session, test, dump, log slice, stop).

## 1. What is done, what is not

Evolution 0 is done and positive. One experiment evolution is done: a physics asset and a side copy of the clay figure exist under `Skaters/Lab/`, a powered-ragdoll schedule was run on 20 knockdowns of mixed lengths, directions and slide speeds beside the hand-made figure, in four settings, each measured per frame and two of them repeated. **Verdict: not worth shipping on the last day as the brief shapes it.** It never produced a NaN, it repeats exactly, and the get-up timing line is untouched (0.0), but no setting meets the bounds: the best one hands back within 3 cm in 12 of 20 knockdowns and it is nearly the hand-made take played one frame late. Not done: the schedule is driven from my test script's tick, not written into the copy Blueprint; no stick body; nothing was seen, by design. A film test for the GPU lane is ready and ran headless without error.

## 2. The experiment evolution

### What was built (in my tree, local branch `lab/ragdoll`, commit 2d4ac3eb = 3e55f946 plus these two files)

- `Game/Plugins/JamAssets/Content/Skaters/Lab/PHYS_SkaterTB1_P3_Rag.uasset` (25,939 bytes): 14 capsule bodies fitted to the clay mesh by `PhysicsAssetToolset.CreateFromMesh`: pelvis, spine_01, spine_03, head, upper and lower arms, thighs, calves, feet. The fitted stick body was removed. Seven radii were reduced so the hand-made takes do not sink the shapes (feet 13.8 to 6.0 cm, chest 23.8 to 19.1, pelvis 18.1 to 15.7). Limits: spine 50/50/40 degrees, head 60/60/60, shoulders free, elbows 120/120/90, hips 100/100/60, knees 110/110/45, ankles 50/50/40.
- `Game/Plugins/JamAssets/Content/Skaters/Lab/BP_SkaterVisual_TB1_P3_Rag.uasset` (52,297 bytes): **a child Blueprint of `BP_SkaterVisual_TB1_P3`, not a duplicate.** A duplicate does not compile (a cast to the figure's own class no longer fits the copy: "Can't connect pins Target and As BP Skater Visual TB1 P3"). The child inherits every graph and take. It adds `RagAnim` (a physical animation component) and `RagFloor` (an invisible box, top 2 cm under the ice, moving with the figure), and overrides on the inherited `Mesh`: the physics asset, collision on (bodies meet only `RagFloor`: no self-collision, no other figure, no boards), pose always ticked.
- The shipped figure assets are untouched: `git status` in my tree showed only `Skaters/Lab/` before the commit; `SK_SkaterTB1_P3` still has no physics asset.

### The schedule (per knockdown; frame 0 is the `SetKnockedDown(true)` call, H is the fall's hold frame read from the figure's `HoldAt`: 13, 18, 19 or 28)

The pelvis stays on the take. All bodies below `spine_01`, `thigh_l`, `thigh_r` are simulated and pulled to the take's pose in world space. Strong for 2 frames, weak after, a ramp back to strong, then the physics weight goes 1 to 0 over 7 frames and the bodies return to the animation.

| Setting | Hand-back starts | Weak motor (stiffness / damping) | Strong motor |
|---|---|---|---|
| A-soft (the brief's timing) | H - 7, so it ends on the lying loop's first frame | 400 / 30 | 5000 / 140 |
| A-stiff (the brief's timing) | H - 7 | 2500 / 75 | 12000 / 220 |
| B-soft (late) | H + 18, in the lying loop, after 8 ramp and 6 strong frames | 400 / 30 | 5000 / 140 |
| B-stiff (late) | H + 18 | 2500 / 75 | 12000 / 220 |

### Proving numbers: 20 knockdowns per setting (60 to 150 frames; falls `Fall`, `Knockdown_Back/Left/Right`, each in light, normal and heavy; slide 0, 3, 5 or 8 m/s slowing at 6 m/s2)

Measured in the headless play session, every frame, by reading the bones of the ragdoll figure and of a hand-made twin standing in the same spot and given the same calls in the same frames. "Shape" is a body's capsule; the ice is z 0.

| Bound from the brief | A-soft | A-stiff | B-soft | B-stiff | The hand-made twin |
|---|---|---|---|---|---|
| NaN in any body position | 0 | 0 | 0 | 0 | |
| Furthest any body got from its animated position (bound 60 cm) | 120.4 cm | 82.8 | 120.4 | 82.8 | |
| Knockdowns that stayed within 60 cm | 14 of 20 | 15 | 8 | 15 | |
| Lowest shape under the ice (bound 2 cm) | 11.5 cm | 10.7 | 24.0 | 10.7 | 8.1 by the same shapes |
| Knockdowns with no shape more than 2 cm under | 6 of 20 | 9 | 2 | 8 | 17 |
| Pose error when the hand-back starts, worst body (bound 3 cm) | up to 42.8 cm | 20.5 | 20.6 | 8.6 | |
| Knockdowns handed back within 3 cm | 0 of 20 | 3 | 8 | 12 | |
| Pose error two frames after the hand-back ends | 0.00 | 0.00 | 0.00 | 0.00 | |
| Get-up end minus release (`SK6REL`, 40 lines per run, both figures) | 0.0 (largest 0.000009) | 0.0 | 0.0 | 0.0 | 0.0 |
| Run-to-run spread, a second fresh play session | not repeated | not repeated | 0.000 cm over 31,486 samples | 0.000 cm over 31,486 samples | |
| Mean distance from the take during the fall, all 13 bodies | 11.2 cm | 7.1 | 15.9 | 8.9 | |
| The same for forearms and feet | 17 to 22 cm | 12 to 13 | 24 to 30 | 13 to 16 | |
| Lowest point of the stick's blade | 115.6 cm under the ice | 84.5 | 115.3 | 84.5 | 26.6 under (`Knockdown_Back_Heavy`) |

No still: this lane takes no pictures.

### Why it does not fit

- **The hand-made falls are stepped, like stop motion.** In `Knockdown_Back` the pelvis moves 12 and 17 cm in the first two frames, 2 to 3 cm a frame for the next five, then 10 and 16 cm again; the stick forearm moves 45 cm in one frame. Averaged over all bodies the take moves 10.0 cm per frame during a fall. A motor soft enough to let physics show cannot follow that: the ends trail 24 to 30 cm on average and feet got 97 to 120 cm from the take in the five heavy falls.
- **A motor stiff enough to come near the bounds is the take about one frame late.** B-stiff's mean distance from the take is 8.9 cm; from the take delayed by one frame it is 9.2 cm, by two frames 14.6. By arithmetic that is 5 to 6 pixels on a 110-pixel figure, for a third of a second.
- **The brief's timing leaves almost no physics.** A light fall holds from frame 13, so the hand-back starts at frame 6; 0 and 3 of 20 arrive within 3 cm because the take is still moving under the blend.
- **The pelvis drives a leg into the floor.** In B-soft, `Knockdown_Back_Light` put a foot 24 cm under the ice at frame 14 and 98 cm from the take. At the stiff motor the same thigh still goes 9 to 10 cm under.
- **The stick has no body.** It rides the simulated forearm, so a forearm 40 degrees off puts the blade a metre under the ice.

### Three engine findings worth keeping

1. **Simulated bodies ran exactly one frame behind the pelvis** while the mesh's collision was `PhysicsOnly`: spine_01 sat 16 and 22 cm from the take on frames 1 and 2 of `Knockdown_Back`, with no motor at all. With `QueryAndPhysics` the lag is gone (spine_01 0.0 cm, thighs under 1.5 cm). Measured both ways; the cause (a deferred kinematic update) is my inference.
2. **`PhysicsAssetToolset.SetConstraintLimits` does not reach the running figure.** `GetConstraints` read my new limits back, but the play-session figure still had the generated 45/45/45 on every joint, which held the forearms 16 to 29 cm off for the whole lying phase. Setting them through `PhysicsAsset.get_constraint_by_bone_names` and `ConstraintInstanceBlueprintLibrary.set_angular_limits` works (read back from the running figure). Seen on this one asset.
3. **Local-space motors were worse** than world-space (125.2 against 77.5 cm furthest, strong motor only), tried once, before findings 1 and 2 were fixed.

## 3. Not verified

- **Anything by eye.** Every ragdoll result is a number. Whether it reads more alive, mushy or broken at the game camera is unknown.
- The film scripts on the GPU editor. They ran headless only (ten pairs, 1,768 frames, no error); the camera positions below are arithmetic.
- A knockdown from the game (a figure attached to a pawn in a match). All 20 are test figures at mid-ice called from my script; the slide is my own.
- Wall falls, `Trip`, the spun falls, the night figure: none were in the 20, none was built.
- The Lab assets in a fresh editor start, in the lead's tree, in a cook or a packaged build.
- Whether any joint limit is ever reached. The limits are cones on generated frames, not anatomical.
- The cost in frame time of 14 bodies per figure; I did not time the wall clock during the runs.
- The repeat beyond 0.01 cm (the record's rounding), across an editor restart, or on the GPU editor.
- A-soft and A-stiff were run once each.
- The six `Error:` lines in my editor's log are all the abandoned duplicate's compile errors; I looked for no other warnings.

## 4. For the engine lane

No take was added or changed. If the film makes this worth another day, the behaviour to build into `BP_SkaterVisual_TB1_P3_Rag` is:

- **Trigger:** `SetKnockedDown(true, ...)`, frame 0. Read `HoldAt` after `FallPick` has run (H = `HoldAt` x 60).
- **Frames:** bodies below `spine_01`, `thigh_l`, `thigh_r` simulated from frame 1; motor strong on frames 0 and 1, weak from frame 2, ramp over 8 frames from H + 4, strong from H + 12, physics weight 1 to 0 over frames H + 18 to H + 25, simulation off at H + 25. The longest is 53 frames (heavy falls); the shortest knockdown (60 frames) is back on the animation at frame 38, before `GetUp_Snap` starts at 41.
- **Starts on** the standing or skating pose of frame 0; **ends on** the lying loop's own pose (error 0.00 two frames after the hand-back).
- **Must never:** move the actor or the `Mesh` component; change a montage's start, rate or length (get-up end minus release stays 0.0); run with `PhysicsOnly` collision on the mesh; still be simulating when the get-up starts; start on a wall fall until those are measured.
- **To film it now**, see section 7's first item for the steps.

## 5. For the other device

- Nothing is needed for the experiment. The bodies meet only the figure's own floor on the engine's unused `Destructible` channel, so `Game/Config` is untouched.
- If it ever ships: one named collision channel for "figure bodies against the ice" in `DefaultEngine.ini`, in place of `Destructible`.
- A warning for device A's `PA_P4_*` assets, made with the same toolset: limits set with `PhysicsAssetToolset.SetConstraintLimits` read back correctly and are not what a running figure has (finding 2).
- A headless editor needs the pose tick line from Evolution 0 for any bone measurement; `tools/run/editor.ps1` already forwards `-nullrhi -unattended` through `-ExtraArgs`.

## 6. Decisions for the operator

1. **Ship physics on the falls tomorrow? My answer is no.** The hand-made falls are stepped on purpose and physics either lags them visibly or copies them. The ten-minute film below would overturn this only if the soft setting plainly reads better at game size.
2. **If the film is liked:** the cheaper next form is physics on the ends only (forearms, head, feet), with the torso and thighs on the take. It is next evolution 1; it is a day's work with the stick body, not an hour's.
3. **The second engine:** worth using tonight for any lane that measures or edits without looking. It ran at 60 frames a second beside the GPU editor with no hang.

## 7. The next three evolutions

1. **The film (the lead's GPU lane, ten minutes), then ends-only physics.** Steps: copy the two files from `C:\Users\kazda\jam\wt\b-vfx\Game\Plugins\JamAssets\Content\Skaters\Lab\` into the same folder of the filming tree before its editor starts (lab, not shipped); start a play session on a quiet cartridge; run `C:\Users\kazda\jam\rag-lab\film\film_make_soft.py` through `execute_python` on that editor's port. Ten pairs stand on y 0 at x -1800 to +1800 every 400 cm, facing -Y: the hand-made figure in blue, its ragdoll twin 150 cm to its right in red. Pair i is knocked down at frame 90 + 150 i after the call (lengths 60, 74, 88, 96, 104, 113, 126, 138, 150, 150; pushed back, forward, left, right in turn); the whole run is 1,600 frames. A camera near (x of the pair + 75, -900, 450) pitched -22 degrees holds one pair. `film_status.py` reports `done`; `film_off.py` removes the figures; `film_make_stiff.py` is the same with the stiff motors. Then, ends-only: **proving number: 20 of 20 handed back within 3 cm, 20 of 20 within 60 cm, lowest shape no lower than the hand-made twin's, at the soft motor.**
2. **A stick body on the stick hand.** **Proving number: the blade's lowest point no more than 5 cm under the hand-made take's in all 20 knockdowns** (today the run's lowest point is 58 cm lower at the stiff motor and 89 cm at the soft one).
3. **The schedule inside the child Blueprint**, on `SetKnockedDown`, so a match drives it with no script. **Proving number: the same 20 knockdowns give positions equal to the script-driven run to 0.01 cm, and get-up end minus release 0.0 on every knockdown of two seeded matches.**

## State left behind

- My tree `C:\Users\kazda\jam\wt\b-vfx` is on the local branch `lab/ragdoll` (2d4ac3eb), not pushed; `lane/b-vfx` was fast-forwarded to 3e55f946. `dev-b` was not touched.
- The editor left through `ExitEditor` with `bDiscardUnsavedChanges` true: `/Game/Maps/Rink` was marked unsaved by a temporary actor I spawned and destroyed to read the skeleton; it was not saved. No process of mine is running.
- Lab folder: the harness (`py.mjs`, `mc.sh`, `run.sh`, `lib.py`, `rag.py`, `cases20.py`, `cfg*.py`), the analysis (`ana.mjs`, `vis.mjs`, `ts.mjs`, `low.mjs`), the records `runs\*.json` with a per-knockdown table in `runs\A1w.txt`, `A4.txt`, `C1.txt`, `C4.txt` (A-soft, A-stiff, B-soft, B-stiff), and `film\`.

## Ledger rows (only if the lead adopts the two files; the ledger's header has 11 fields, which is what these follow)

```
| Game/Plugins/JamAssets/Content/Skaters/Lab/PHYS_SkaterTB1_P3_Rag.uasset | other | 0.02 | original | UE 5.9 PhysicsAssetToolset.CreateFromMesh (MCP) + Python |  |  | 2026-11-14 | B | n | lab, not shipped: 14 capsule bodies for SK_SkaterTB1_P3 (stick body removed, seven radii reduced, limits set through the constraint accessor), used only by Lab/BP_SkaterVisual_TB1_P3_Rag (lane RAG) (25939 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Lab/BP_SkaterVisual_TB1_P3_Rag.uasset | other | 0.05 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | n | lab, not shipped: child Blueprint of BP_SkaterVisual_TB1_P3 with a physics asset override on Mesh, RagAnim (physical animation) and RagFloor (own floor box); the powered-ragdoll schedule is driven by rag-lab/rag.py, not by the Blueprint (lane RAG) (52297 bytes) |
```
