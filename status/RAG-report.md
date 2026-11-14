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
- **The one thing to know: nothing is ever "rendered", so a figure's pose does not advance.** The figure meshes use `OnlyTickPoseWhenRendered`. Headless, a montage sat at position 0.0 and the pelvis stayed at z 57.33 on four figures. The Blueprint's time logic and its logs still run. One Python line per figure on the play-session instance fixes it (nothing saved), **and it must be set without change notification**: `mesh.set_editor_property('visibility_based_anim_tick_option', unreal.VisibilityBasedAnimTickOption.ALWAYS_TICK_POSE_AND_REFRESH_BONES, unreal.PropertyAccessChangeNotifyMode.NEVER)`. **Corrected 04:20 (the figure lane's finding, confirmed here):** the line as I first gave it, without the third argument, re-runs the figure's construction and resets its Blueprint variables to the class defaults (`HoldAt`, `StumF`, `RxGap` read 0 afterwards instead of 9.0, 16, 0.35). Part 4 says which of this lane's runs were made that way and what it changed. After it: pelvis z 72.8 to 74.6, `Stop_Hard` at 0.25 s.
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
- A headless editor needs the pose tick line from Evolution 0 (in its corrected form, with `PropertyAccessChangeNotifyMode.NEVER`) for any bone measurement; `tools/run/editor.ps1` already forwards `-nullrhi -unattended` through `-ExtraArgs`.

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

---

# Part 2 (second loop, 02:20 to 03:15): ends-only physics, a stick body, the schedule inside the child Blueprint

Same tree (`C:\Users\kazda\jam\wt\b-vfx`, branch `lab/ragdoll`, now at 7c92338c, local, not pushed), port 8000, every editor started with `-nullrhi -unattended`. Four editor sessions (pids 15912, 38424, 36436, 28408), each left through `ExitEditor` with the discard flag and confirmed gone, the last at 03:02:33. `LogD3D12RHI` lines: 0 in each of the four logs, `Error:` lines 0, 44 play sessions started and stopped. In the lead's tree only this report is mine.

## 1. What is done, what is not

All four items are done and measured; one bound is not met. **Ends-only physics hands back within 3 cm in 20 of 20 knockdowns at every motor tried, keeps the timing line at 0.0, repeats exactly, and with the stick body the blade stays within 1.5 cm of the hand-made twin's lowest point.** The child Blueprint now runs the schedule itself: its positions equal the script-driven run to 0.01 cm, and two seeded matches ran with it as the game's figure (11 knockdowns, end minus release 0.0 on all, 4 wall falls skipped). **Not met: "never more than 25 cm from the take".** No motor holds it, because the hand-made take itself moves a hand or the head more than 25 cm in a single frame on 13 to 24 % of the fall's frames. The softest motor that keeps every end within 60 cm is 5000 / 106. Nothing was seen; the film scripts are updated.

## 2. Per evolution

### Evolution 1: ends-only physics

**What changed.** The pelvis, spine, upper arms and thighs stay on the take. Simulated: both forearms (they carry the hands), neck and head, shins and feet, and the stick. Five things made it work:

- **The physics starts 3 frames after the call, not on it.** The figure cuts from its skating pose into the fall with no blend: within the first three frames a hand moves up to 86 cm in a single frame, the head's top 108 cm, the blade 272 cm. Bodies switched on across that cut were flung.
- **Velocities are reset to the root's own at the switch-on.** Without it the stick kept the cut's spin: in the knockdown I followed frame by frame the blade sat 30 to 75 cm off for 14 frames.
- **A neck body between chest and head.** With the head jointed straight to the chest the joint sat 2.5 to 3 cm off wherever the take bent the neck, and 9 or 10 of 20 hand-backs missed 3 cm by that alone.
- **The hand-back motor is 25000 / 237** (was 12000 / 220): with 12000, 3 of 20 missed at 3.94 cm.
- **The floor is on the ice plane (was 2 cm under) with an ice-like material** (friction 0.05, `Lab/PM_RagIce`).

**Schedule** (frame 0 is `SetKnockedDown(true)`; H is the fall's hold frame, 13 to 28): simulated from frame 4 at the weak motor; ramp to the hand-back motor over frames H+4 to H+12; physics weight 1 to 0 over H+18 to H+25; off at H+25. A 60-frame knockdown is back on the take at frame 38, `GetUp_Snap` starts at 41; the longest is frame 53.

**Proving numbers** (the same 20 knockdowns as part 1, driven by the child Blueprint, measured against the hand-made twin in the same spot; "ends" are both hands, both feet, the head's top and the stick's grip):

| Weak motor (stiffness / damping) | 800 / 42 | 2500 / 75 | 5000 / 106 | 12000 / 164 | 25000 / 237 |
|---|---|---|---|---|---|
| Handed back within 3 cm | 20 of 20 | 20 | 20 | 20 | 20 |
| Worst end when the hand-back starts | 2.17 cm | 2.17 | 2.17 | 2.17 | 2.17 |
| Knockdowns with every end within 60 cm of the take | 11 | 17 | **20** | 20 | 20 |
| Knockdowns with every end within 25 cm | 0 | 2 | 3 | 10 | 13 |
| Furthest any end got | 72.0 cm | 65.5 | 57.8 | 48.1 | 37.3 |
| **Lag of hands and head behind the take during the fall, mean** | **12.8 cm** | **9.2** | **7.3** | **5.2** | **3.8** |
| the same, peak | 72.0 cm | 58.3 | 50.0 | 39.2 | 30.8 |
| mean by point: left hand, stick hand, head's top | 8.1, 11.0, 19.3 | 6.0, 7.5, 14.1 | 4.8, 5.5, 11.5 | 3.5, 3.7, 8.4 | 2.6, 2.6, 6.3 |
| Knockdowns where no simulated shape went lower than the twin's own lowest | 14 | 14 | 14 | 14 | 17 |
| Pose error two frames after the hand-back | 0.00 | 0.00 | 0.00 | 0.00 | 0.00 |
| NaN | 0 | 0 | 0 | 0 | 0 |
| Get-up end minus release (40 `SK6REL` lines per run) | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 |

- **Run-to-run spread: 0.000 cm** at the record's 0.01 cm rounding, over 33,908 samples each time: 2500 twice across an editor restart, 25000 twice, 5000 three times, the last from a clean start on the committed files.
- **The 25 cm bound.** After frame 3 the hand-made take's own step from one frame to the next is over 25 cm in 42 to 75 of 318 fall frames for each hand and the head's top (peaks 52 to 71 cm; the root's slide of up to 13 cm a frame is included). A body cannot be nearer than that without moving inside the same frame. At 100000 / 474 (run once, script-driven, before the floor moved) 18 of 20 held it, with a mean lag of 2.3 cm, which is the take.
- **The lowest shape.** The twin itself reads 26.5 cm under the ice by these shapes (the stick, frame 1 of `Knockdown_Back_Heavy`), so I compared per knockdown instead of fixing the shapes to read 0. Where the ragdoll goes lower than both the ice and its twin, it is the stick forearm or a foot at a landing: 6 knockdowns, at most 6 frames, at most 8.5 cm at 2500; 1 knockdown, 1 frame, 3.4 cm at 25000.
- **Is it doing something?** At 5000 the hands trail 5 cm and the head's top 11.5 cm on average through the fall, with single-frame peaks of 35 to 50 cm at the landing; by arithmetic that is 3 to 7 pixels on a 110-pixel figure. At 25000 it is 2.6 and 6.3 cm: close to the hand-made take and probably not worth its cost.

Per-knockdown tables: `C:\Users\kazda\jam\rag-lab\runs\P08.txt`, `P25.txt`, `P50.txt`, `P120.txt`, `P250.txt`. No still.

### Evolution 2: the stick body

**What changed.** A capsule of 3.5 cm radius along the shaft from the grip to 3 cm short of the blade's tip, mass scale 0.2, jointed to the stick forearm, simulated with the ends and held up by the floor.

**Proving number: the blade's lowest point is within 5 cm of the hand-made twin's in 20 of 20 knockdowns at every motor in the table** (worst 0.8 to 1.5 cm under the twin's). Whole-body, without a stick body, it was 58 to 89 cm lower. The blade's error when the hand-back starts is 1.7 cm at worst.

Two things to know: where the take itself puts the blade under the ice (5 to 27 cm: at frame 1 of the back falls, later in the side falls) the ragdoll is under the ice with it only on frames when the stick is not simulated; and the stick only stays in the top hand because of the joint, the lower hand is not tied to it.

### Evolution 3: the schedule inside the child Blueprint

**What changed** in `Lab/BP_SkaterVisual_TB1_P3_Rag` (still a child of the shipped figure; 529,350 bytes):

- `SetKnockedDown` is overridden: it calls the parent's, then arms the schedule on `true` and stops it on `false`.
- `ReceiveTick` is overridden: it calls the parent's, then runs the schedule by game time since the call, so a hit-stop does not shift it.
- Four helper events (`RagSim`, `RagWeight`, `RagDrive`, `RagStop`) and sixteen variables. Ten are editable per figure: `RagEnabled`, `RagS0` 3, `RagLate` 18, `RagRu` 8, `RagHs` 6, `RagBl` 7, `RagKw` 5000, `RagCw` 106, `RagKs` 25000, `RagCs` 237.
- **Wall falls are skipped** (`DownKind` 5 to 8 when the physics would start). The schedule also stops at once if the get-up has started.
- The figure's tick is made to run before its mesh's and the motor component's, so an order of ticks cannot change the result.

**Proving numbers.**

- **Blueprint against script: 0.000 cm** largest difference over 33,908 samples, at 5000 / 106, in two editor sessions.
- **Two seeded matches** (my cartridges `C:\Users\kazda\jam\rag-lab\rag.s1.cartridge.json` and `rag.s7920.cartridge.json`: the regular cartridge with the seed set, `pro`, five a side, `jam.ScriptHuman 1`), the child as every pawn's figure, called by the pawn:

| | Seed 1 | Seed 7920 |
|---|---|---|
| Last sim frame | 6355 | 6366 |
| Figures of the child's class, with the motor component | 10 of 10 | 10 of 10 |
| Knockdowns | 6 (the human) | 5 (human 4, bot 2 once) |
| Physics ran | 4 (35, 35, 40, 43 frames) | 3 (40, 40, 50 frames) |
| Wall falls skipped, 0 frames simulated | 2 | 2 |
| Frames between physics off and the get-up's start, smallest | 6 (a 64-frame knockdown) | 19 |
| Still simulating at the release | 0 | 0 |
| Get-up end minus release | 0.0 on all 6 | 0.0 on all 5 |
| NaN at the ends | 0 | 0 |
| `Error:` lines, Blueprint runtime warnings | 0, 0 | 0, 0 |

- **How it was swapped in, in my tree only.** The game picks the figure class from a path written in its code; no cartridge key or console variable names a class. A cartridge can pick the other track (`"visual": "generated_b2"`), which on clay loads `/JamAssets/Skaters/BP_SkaterVisual_TB2_P3`. I duplicated the child under that asset name in a lab folder, closed the editor, put that file in the place of `Skaters\BP_SkaterVisual_TB2_P3.uasset` in my tree for one session, and put the original back afterwards (its SHA-256 equals the repository's LFS id again; `git status` shows nothing). The parent of the swapped figure was the real, untouched `BP_SkaterVisual_TB1_P3`. Device A's cartridges were not touched.
- A dead end worth a line: renaming the two generated classes in memory leaves redirectors and cannot be undone in the session; I restarted the editor without saving.

### Evolution 4: the film scripts

All in `C:\Users\kazda\jam\rag-lab\film\`. For the GPU lane, in three lines:

1. Copy the three files in `film\loop2\` (the child Blueprint, the physics asset, `PM_RagIce`) into `Game\Plugins\JamAssets\Content\Skaters\Lab\` of the filming tree before its editor starts.
2. In a play session run `film_make_ends.py` through `execute_python`: the same ten pairs and timeline as before (hand-made in blue at x -1800 to +1800, the ends-only twin 150 cm to its right in red, pair i knocked down at frame 90 + 150 i, about 1,800 frames), weak motor 2500 / 75; `film_ends_status.py` until `done`, then `film_ends_off.py`.
3. `film_make_ends_stiff.py` is the same at 25000 / 237, the setting nearest the bounds.

**For the whole-body film already planned:** use the two files in `film\loop1\` (they are the ones measured in part 1) with `film_make_soft.py`. The files now in my tree's `Lab` folder are the ends-only ones and carry their own schedule; I made `film_make_soft.py` switch that off if it meets them, but that pairing was not run.

## 3. Not verified

- **Anything by eye**, again. Whether 5 to 12 cm of trailing hands and head reads as life or as mush at game size is the film's question.
- The film scripts on the GPU editor: `film_make_ends.py` ran headless (ten pairs, 1,791 frames, no error); the whole-body scripts were only rebuilt and parsed.
- In the matches there is no twin, so no distance from the take was measured there: only timing, NaN, errors and whether the physics ran. Seven knockdowns with physics is a small sample; `Trip` never occurred; no second knockdown arrived while one was simulated.
- The match figure was a duplicate of the child made before the last edit (the stop on a started get-up). The 20-knockdown run after that edit equals the one before to 0.01 cm.
- The matches used 5000 / 106 only.
- Whether the lower hand leaves the stick while both are simulated: not measured.
- Wall falls with physics, the night figure, a cook, a packaged build, frame-time cost.
- The figure lane's whip is not in my tree; what I say about it below is reasoning, not a test.

## 4. For the engine lane

- **Use the child as it is:** `Lab/BP_SkaterVisual_TB1_P3_Rag` needs `Lab/PHYS_SkaterTB1_P3_Rag` and `Lab/PM_RagIce`, and nothing else. It changes nothing while `RagEnabled` is false.
- **Trigger:** the game's `SetKnockedDown(true)`. **Starts on** the take's pose at frame 3. **Ends on** the lying loop's own pose (0.00 cm two frames after). **Lengths:** physics from frame 4 to H+25, so 38 to 53 frames.
- **Must never:** run on a wall fall; be on when the get-up starts (a knockdown under about 58 frames would cut it short by the guard); use `PhysicsOnly` collision on the mesh; start on frame 0.
- **The whip and the physics on the same bones.** During a knockdown the whip should yield on the head, the neck and the stick arm from the frame the physics starts until it is off, and keep the spine, which the physics inherits through the take. The physics is pulled toward whatever pose the animation gives, so a whip that changes the pose is followed, not fought; the fight would be a whip that also moves those bones after the physics has. **How each side knows, with no shared variable:** the whip can ask the mesh `IsSimulatingPhysics("head")` (or `"lowerarm_r"`) and take its weight on those bones to 0 while true. On frames 0 to 3, before the physics, the whip owns everything. The hit's direction and strength reach the child in the same `SetKnockedDown` call if the physics should ever take the hit as an impulse instead.
- **Three tool traps** (PhysicsAssetToolset through MCP): `SetConstraintLimits` does not reach a running figure (part 1); `AddConstraint` leaves the parent's anchor at the parent bone's origin (the stick grip sat 30 cm off in all 20 until I set `Pos2` on the template's `DefaultInstance`); a bone that animates between two bodies needs a body of its own (the neck).
- After `ExitEditor`, port 8000 can show as taken by "pid 0" for up to about a minute and `editor.ps1` refuses to start; retry.

## 5. For the other device

- **To let a figure class be swapped without replacing a file:** a console variable read in `AHokejGameMode::ResolveVisualClass`, for example `jam.vis.ClassOverride <class path>`, tried before the built-in paths. Input: one class path implementing `IJamSkaterVisual`; empty = today's behaviour.
- If ends-only ships: the child's class path in `VisualClassPathsFor` for `humanoid_b1` on `p3_clay` (or the child's parts folded into the shipped Blueprint by B), and one named collision channel for figure bodies against the ice in place of `Destructible`.
- Nothing else. The pawn's calls were enough; no new call or event is needed.

## 6. Decisions for the operator

1. **Ends-only at 5000 / 106 is the candidate if the film is liked.** It holds the hand-back, the timing, the stick and 60 cm; it does not hold 25 cm and no setting can. My view: decide by the film, red against blue, not by the 25 cm number.
2. **Softer or stiffer.** 2500 / 75 shows more (9 cm mean lag) and lets 3 of 20 knockdowns past 60 cm; 25000 / 237 is nearly the hand-made take.
3. **Clay only.** There is no night figure with physics; building one is the same steps on the other mesh, about an hour, unmeasured.
4. **Feet and the stick forearm dip into the ice for a few frames at the landing in 6 of 20 falls** (up to 8.5 cm at 2500). Accept for a toy, or hold it back until evolution 2 below.

## 7. The next three evolutions

1. **The landing dips.** More solver iterations on the physics asset, or shins kept on the take with only the feet simulated. **Proving number: 20 of 20 knockdowns with no simulated shape lower than its twin's lowest, at 5000 / 106** (14 today).
2. **The night figure and the wall falls.** The same physics asset and child on `SK_SkaterTB1v2`; wall falls with the boards as a second floor. **Proving number: the part 2 table for the night figure, and 0 bodies through the boards in the eight wall takes.**
3. **The hit as an impulse.** The knockdown's direction and strength pushed into the head and stick arm at switch-on, with the whip yielding as in section 4. **Proving number: the head's lag peak moves with the hit's strength (two strengths, at least 10 cm apart) while the hand-back stays 20 of 20 within 3 cm.**

## State left behind

- My tree: `lab/ragdoll` at 7c92338c (3e55f946, then 2d4ac3eb, then this), three Lab files, nothing else changed: `BP_SkaterVisual_TB1_P3.uasset` and `BP_SkaterVisual_TB2_P3.uasset` both hash to their LFS ids. No process of mine is running.
- Console variables were set back (`jam.Cut hockey.regular`, `jam.ScriptHuman 0`, `jam.vis.lowlog 0`) before the session ended.
- Lab folder additions: `rag2.py`, `run2.sh`, `ana2.mjs`, `lag2.mjs`, `lowx.mjs`, `rag_match.py`, `match.sh`, `anam.mjs`, `child.dsl` (the Blueprint's schedule as written), the two cartridges, `runs\P*.json`, `runs\M1.json`, `runs\M2.json` with their logs, `backup\` (the swapped file and the original), `film\`.

## Ledger rows for part 2 (11 fields, as the ledger's header; these replace part 1's two rows if the files are adopted)

```
| Game/Plugins/JamAssets/Content/Skaters/Lab/BP_SkaterVisual_TB1_P3_Rag.uasset | other | 0.50 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) |  |  | 2026-11-14 | B | n | lab, not shipped: child Blueprint of BP_SkaterVisual_TB1_P3; overrides SetKnockedDown and ReceiveTick (parent called first) to run ends-only physics on a fall: forearms, neck and head, shins and feet and the stick simulated from frame 4 to hold frame + 25, wall falls skipped; RagAnim (physical animation), RagFloor (own floor box on the ice plane), physics asset override on Mesh (lane RAG) (529350 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Lab/PHYS_SkaterTB1_P3_Rag.uasset | other | 0.03 | original | UE 5.9 PhysicsAssetToolset (MCP) + Python |  |  | 2026-11-14 | B | n | lab, not shipped: 16 bodies for SK_SkaterTB1_P3 (14 fitted capsules with seven radii reduced, a neck sphere, a thin stick capsule at mass scale 0.2), limits and the neck and stick joint anchors set by Python, used only by Lab/BP_SkaterVisual_TB1_P3_Rag (lane RAG) (28883 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Lab/PM_RagIce.uasset | other | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | n | lab, not shipped: physical material of the child figure's own floor (friction 0.05, restitution 0, combine min) (lane RAG) (1795 bytes) |
```

---

# Part 3 (third loop, 03:07 to 03:50): the merged figure with its whip, the landing dips, the night figure, the wall falls

Same tree (`C:\Users\kazda\jam\wt\b-vfx`, branch `lab/ragdoll`, now at 911a334a: the merge of local `dev-b` b65f7525 as 3ee31df1, then one commit of four Lab files; local, not pushed). The merge had no conflict; the rebuild passed in 129 s. Port 8000, every editor `-nullrhi -unattended`: three sessions (pids 17732, 24720, 21548), each left through `ExitEditor` with the discard flag and confirmed gone, the last at 03:42:00. `LogD3D12RHI` lines 0 and `Error:` lines 0 in the three logs. The play world's time advanced in every session; `Slate.bAllowThrottling 0` was not needed. In the lead's tree only this report is mine; `film\loop2\` was not touched.

## 1. What is done, what is not

All five items are done and measured on the merged figure. **Physics alone hands back within 3 cm in 20 of 20 knockdowns on both figures at 5000 / 106; the eight wall takes hand back 8 of 8 with nothing more than 0.2 cm further past the boards than the hand-made twin; two seeded matches on the merged build ran clean with wall falls simulated.** The child now yields to the parent's whip on the head and the stick arm while it simulates and keeps the spine. Not met, with the reasons in numbers below: with the whip on, the hand-back is 19 of 20 on clay and 18 of 20 on night, and every miss is a back fall where the hand-made twin's own left hand flickers; the landing dips reach 20 of 20 on clay only with two physics sub-steps, which is a project setting; 25 cm still cannot be held. I also found and fixed a fault of my own from part 2 (the tick order). Nothing was seen.

## 2. Per evolution

### Evolution 0: onto the merged figure

**A fault of mine that the merge exposed.** In part 2 I made the mesh tick after the figure. That shifted the parent's own lying-loop start by one frame on the child. On the old figure it did not show; on the new one, whose loop start is computed from the frames left, 5 of 20 knockdowns sat 1.4 to 3.8 cm off the twin after the physics was over. The child's tick now runs after its mesh's, as the parent's does. **With the physics switched off the child equals the twin on every point and frame, 0.00 cm, on both figures.** Part 2's "equal to the script-driven run" was measured with the old order and was not repeated.

**Two more changes that the new measure asked for.** The limits on the simulated joints are free now: the ankle cone was turning the feet, so a foot's shape sat up to 5.8 cm off at the hand-back while the ankle point read within 3. The table has a row for it. Continuous collision is on for the simulated bodies.

**`SetTestMode(16)` still exists and switches the whip off. It also forces the lying rule B** (the Blueprint tests `UpMode >= 8`), so a whip-off figure is not a clean twin for a whip-on one after the hold frame. I compare off against off and on against on.

**The table, clay, 5000 / 106, the same 20 knockdowns** (whip on = Strength 1.0 sent to both figures, of which a knockdown takes half):

| | Whip off on both | Whip on on both | Off, 2 sub-steps | On, 2 sub-steps |
|---|---|---|---|---|
| Handed back within 3 cm (worst end) | 20 (2.24 cm) | 19 (4.06) | 20 (1.13) | 20 (2.65) |
| Shape heights within 3 cm at the hand-back (worst) | 20 (0.8) | 19 (3.2) | 20 (0.5) | 20 (2.0) |
| Every end within 60 cm (furthest) | 20 (50.3) | 20 (50.6) | 20 (46.5) | 20 (46.5) |
| Every end within 25 cm | 3 | 4 | 10 | 10 |
| Lag of hands and head during the fall, mean / peak | 6.2 / 50.3 cm | 6.1 / 50.6 | 5.1 / 40.5 | 5.1 / 38.8 |
| mean by point: left hand, stick hand, head's top | 3.7, 3.9, 11.0 | 3.7, 3.9, 10.8 | 3.2, 3.5, 8.5 | 3.2, 3.5, 8.6 |
| Knockdowns with a simulated shape under the ice and lower than its twin | 4 (1 frame each, 3.3 cm at most) | 1 (1 frame, 0.9 cm) | **0** | **0** |
| Blade within 5 cm of the twin's lowest (worst under it) | 19 (5.3) | 20 (3.8) | 20 (1.8) | 20 (3.1) |
| Pose error two frames after the hand-back, NaN | 0.00, 0 | 0.00, 0 | 0.00, 0 | 0.00, 0 |
| Get-up end minus release (40 lines) | 0.0 | 0.0 | 0.0 | 0.0 |

- Run-to-run spread: 0.000 cm over 33,908 samples (whip on, twice in one session; night, whip off, across an editor restart from the committed files).
- Against part 2: the hands lag less (3.7 and 3.9 cm, were 4.8 and 5.5) because the elbows are free; the head is the same.
- **The one miss with the whip on is the twin, not the physics.** In every back fall, with the figure's defaults, the hand-made twin's left hand alternates about 2 cm up and down on consecutive frames for nine frames in `Lie_Loop_Front` (frames 24 to 32 of a 60-frame knockdown, 54 to 62 of a 113-frame one; six of six back falls, both figures). With `SetTestMode(16)` it does not. In the 60-frame knockdown that window lies on the hand-back and the ragdoll's hand is 4.06 cm from a hand that is jumping.

### Evolution 1: the whip against the physics

**How.** The parent offers no weight to set, so the child does it by override: after the parent's tick has written the whip into the animation Blueprint, and only while the child is simulating, the child sets that Blueprint's `WhipHead` and `WhipArm` back to zero. `WhipRot` (the two spine bones) is not touched. A switch, `RagWhipYield`, default on.

**Proving numbers.**

- Hand-back with whip and physics both on: 19 of 20 on clay (the miss above), 20 of 20 with two sub-steps.
- **The head's peak lag: 50.6 cm with the whip on against 50.3 physics-only, 0.6 % apart** (38.8 against 40.5 with sub-steps).
- The spine keeps its whip: `spine_03` is 0.00 cm apart between the child with the yield and without.
- What the yield removes: up to 4.9 cm at the head's top and 4.1 cm at the stick hand (means 0.9 and 0.2 cm) over the whip's frames.

**What I would ask the figure lane for:** `WhipHeadW` and `WhipArmW`, floats, default 1, multiplied into `WhipHead` and `WhipArm` inside `WhipTick`. The child then sets them to 0 at switch-on and 1 at stop, and no longer needs to name the animation Blueprint (today that one line differs between the clay and the night child).

### Evolution 2: the landing dips

| Step (clay, whip off) | Knockdowns with a shape under the ice and lower than its twin | Frames in all, deepest |
|---|---|---|
| The merged figure, part 2's physics asset | 8 of 20 | 16, 7.4 cm |
| Free limits and continuous collision | 4 of 20 | 4, 3.3 cm |
| More solver iterations on the bodies | no change | |
| World solver at 32 iterations | 5 of 20, and the motors feel different | 15, 7.8 cm |
| **Two physics sub-steps (1/120 s)** | **0 of 20** | 0 |

- **20 of 20 is reached on clay, with the motor unchanged, only with sub-stepping.** That is `bSubstepping`, `MaxSubstepDeltaTime` 0.008333, `MaxSubsteps` 2 in the project's physics settings, device A's file. I set it in my session only; nothing is saved. Its cost in frame time was not measured.
- Read strictly ("no simulated shape lower than its twin's lowest") it is 13 of 20 even then: the stick lies on the ice at 0.0 to 0.1 cm where the twin's hovers at 1.0 to 1.3. On the ice, not under it.
- Without sub-stepping: 16 of 20 on clay, 15 of 20 on night; every dip lasts one or two frames and is 4.5 cm at most.

### Evolution 3: the night figure and the wall falls

**The night figure.** `Lab/PHYS_SkaterTB1v2_Rag` (16 bodies fitted to `SK_SkaterTB1v2`; head radius 25.5 to 17 and feet 11.6 to 6 so the twin does not sink them; the same neck, stick, free limits and continuous collision) and `Lab/BP_SkaterVisual_TB1_Rag`, a child of `BP_SkaterVisual_TB1` with the same schedule.

| Night, 5000 / 106 | Whip off | Whip on | Off, 2 sub-steps | On, 2 sub-steps |
|---|---|---|---|---|
| Handed back within 3 cm (worst) | 20 (1.88) | 18 (5.15) | 20 (1.02) | 18 (3.46) |
| Every end within 60 cm (furthest) | 20 (45.9) | 20 (45.9) | 20 (42.7) | 19 (61.1) |
| Every end within 25 cm | 7 | 8 | 11 | 10 |
| Lag of hands and head, mean / peak | 6.0 / 45.7 | 6.0 / 45.5 | 5.0 / 35.4 | 5.4 / 48.9 |
| Under the ice and lower than its twin | 5 (2 frames, 4.5 cm at most) | 0 | 2 (1 frame, 1.5 cm) | 0 |
| Blade within 5 cm of the twin's (worst under) | 19 (7.5) | 20 (2.1) | 20 (2.5) | 20 (2.5) |
| NaN, end minus release | 0, 0.0 | 0, 0.0 | 0, 0.0 | 0, 0.0 |

The two misses with the whip on are back falls again, the left hand, the twin's flicker.

**The wall falls.** With `RagWalls` on the child no longer skips them. A second box, `RagWall`, stands at the figure's own wall plane (from the parent's `WallYaw` and `WallD`) while the physics runs, set back 6 cm because the twin's own shapes reach that far. The mesh's 30 cm shift toward the boards moves the motors' targets, so the bodies follow it. And the hand-back now starts no later than 9 frames before the get-up, whatever the fall: a 75-frame wall knockdown gets up at frame 40.

| Eight wall takes (four sides, clay and night, 66 cm from a straight board, shift 30, whip off) | Result |
|---|---|
| Handed back within 3 cm | **8 of 8**, worst 0.33 cm |
| A simulated shape further past the boards plane than its twin | **0 of 8** (largest excess 0.2 cm; the twin's own shapes are up to 5.5 cm past it, its stick on frame 1 of the back fall aside) |
| Furthest any end got from the twin while the mesh shifted | 13.7 to 23.9 cm |
| Lowest simulated shape | 0.1 to 2.7 cm under the ice in the eight (the twin 2.2 to 3.3 above); with two sub-steps 0.0 to 0.8 |
| NaN, get-up end minus release | 0, 0.0 |

**Two seeded matches on the merged build** (`pro`, five a side, scripted human, the final child as every pawn's figure with `RagWalls` on, swapped in as in part 2 and restored the same way):

| | Seed 1 | Seed 7920 |
|---|---|---|
| Last sim frame | 6623 | 6360 |
| Knockdowns | 3 (the human) | 5 (human 4, bot 4 once) |
| Physics ran | 3 | 5, two of them wall falls |
| Frames between physics off and the get-up's start, smallest | 30 | 2 (an 83-frame wall fall, by the new rule) |
| Still simulating at the release | 0 | 0 |
| Get-up end minus release | 0.0 on 3 | 0.0 on 5 |
| NaN, `Error:` lines, Blueprint runtime warnings | 0, 0, 0 | 0, 0, 0 |

### Evolution 4: the film scripts

`C:\Users\kazda\jam\rag-lab\film\loop3\` holds the five asset files and its own scripts; `loop2\` and the scripts beside it are as the picture lane has them. For the GPU lane:

1. Copy the five `.uasset` files in `film\loop3\` into `Skaters\Lab\` of the filming tree before its editor starts (they replace loop 2's three).
2. In a play session run `film\loop3\film3_make_clay.py` (ten pairs as before, blue hand-made, red ends-only, both whipped at Strength 1.0), then `film3_status.py` until `done`, then `film3_off.py`; `film3_make_night.py` is the same on the night figure.
3. `film3_make_walls_clay.py`: four pairs 66 cm from the +Y boards at x -1500 to +1500, wall ahead, behind, right, left, one every 150 frames from frame 90.

All three ran headless in a clean session (10, 10 and 4 pairs, no error). After that run I added one clearer error message and rebuilt them; the rebuilt files were parsed, not run again. If a make script answers "no player controller yet", the play session was still starting: run it again.

**About the loop 2 film on the merged figure:** it runs, but that child was built before the merge. Its lying loop starts one frame late against the twin (the fault above) and it does not yield to the whip. Loop 3 is the matching set.

## 3. Not verified

- **Anything by eye.**
- The film scripts on a GPU editor.
- Clay from a clean start: the first run of the clean session failed to spawn (the play session was not ready), so the across-restart repeat is night's only; clay repeated within one session.
- The whip at the wall: the wall takes ran with the whip off on both. Corners, and distances other than 66 cm, were not measured against a twin; two wall falls ran in a match without one.
- The matches used a duplicate of the final clay child with `RagWalls` on. No night match. No twin in a match, so no distances there.
- Sub-stepping's cost, and whether it changes anything else in the game that uses physics.
- The frame-time cost of the bodies, a cook, a packaged build.
- `Trip` and a second knockdown during a simulated one still never occurred.

## 4. For the engine lane

- **The children as they stand** (`RagEnabled` on, `RagWhipYield` on, `RagWalls` off, 5000 / 106, hand-back motor 25000 / 237): physics from frame 4, the hand-back from hold frame + 18 or 9 frames before the get-up, whichever is earlier, over 7 frames.
- **For the figure lane, three things in the parent:**
  1. `WhipHeadW` and `WhipArmW` as above.
  2. The left hand's nine-frame flicker in `Lie_Loop_Front` with the default lying rule (not with `SetTestMode(16)`), in every back fall, both figures.
  3. On frame 1 of `Board_Fall_Back` the twin's stick reaches 91 cm (clay) and 105 cm (night) past the boards plane, by my stick shape.
- `SetTestMode(16)` changes the lying rule as well as the whip; a test switch for the whip alone would make clean twins.
- A fourth tool trap: `summon` in a play session that has no player controller yet spawns nothing and says nothing.

## 5. For the other device

- **To use the children in the game:** in `AHokejGameMode::VisualClassPathsFor`, for `humanoid_b1`: on `p3_clay` first `/JamAssets/Skaters/Lab/BP_SkaterVisual_TB1_P3_Rag.BP_SkaterVisual_TB1_P3_Rag_C`, then today's path; on the other tiers first `/JamAssets/Skaters/Lab/BP_SkaterVisual_TB1_Rag.BP_SkaterVisual_TB1_Rag_C`, then today's. If a child is absent the game falls back as it does now.
- **Collision channel:** nothing is required. The bodies meet only the child's own floor and wall on the engine's `Destructible` channel, which nothing else here uses. A named channel is tidier, not needed.
- **Optional, for the dips:** `[/Script/Engine.PhysicsSettings]` `bSubstepping=True`, `MaxSubstepDeltaTime=0.008333`, `MaxSubsteps=2`.
- Still wanted for testing: a console variable that names a figure class, so no file has to be swapped.

## 6. Decisions for the operator

1. **Ship it or not is the film's call.** Everything a number can show is in place on both figures; what a number cannot show is whether 4 to 11 cm of trailing hands and head reads as life.
2. **Sub-stepping.** Without it a forearm or the stick is under the ice for one frame in about one fall in four or five. With it, none on clay. It is a global physics setting with an unmeasured cost.
3. **Wall falls with physics** pass 8 of 8 and ran in a match, but are off by default until they are filmed; one variable turns them on.
4. **The whip under the physics.** The yield removes at most 5 cm at the head. If the film shows no difference, the yield can stay or go without touching a bound.

## 7. The next three evolutions

1. **The weights in the parent** (`WhipHeadW`, `WhipArmW`) and the child using them. **Proving number: the child's graph names no animation Blueprint, and the 20 knockdowns give the same positions as today's override to 0.01 cm.**
2. **The cost.** Frame time of ten figures all knocked down at once, with and without two sub-steps, on the GPU editor. **Proving number: the added milliseconds per frame, and the frame rate of a 100 s match with the children against without.**
3. **Corners and other distances at the wall, with the whip on.** **Proving number: 0 shapes further past the boards than the twin and the hand-back within 3 cm in 12 of 12 (three distances, straight board and corner arc, two figures).**

## Would I ship it

**Yes, if the film is liked, and only then.** The list the lead would adopt, as they are and where they are (the files name each other by path, so they stay under `Skaters/Lab/` or are moved in the editor, not on disk):

- `Game/Plugins/JamAssets/Content/Skaters/Lab/BP_SkaterVisual_TB1_P3_Rag.uasset`
- `Game/Plugins/JamAssets/Content/Skaters/Lab/BP_SkaterVisual_TB1_Rag.uasset`
- `Game/Plugins/JamAssets/Content/Skaters/Lab/PHYS_SkaterTB1_P3_Rag.uasset`
- `Game/Plugins/JamAssets/Content/Skaters/Lab/PHYS_SkaterTB1v2_Rag.uasset`
- `Game/Plugins/JamAssets/Content/Skaters/Lab/PM_RagIce.uasset`

The other device changes the two class paths in section 5; the collision channel needs no change; sub-stepping is its choice. Before a push: a cook check (five new assets under `Skaters/`), which I did not run. What would make me say no: the film reading as mush, or the cost (unmeasured) showing in the frame rate.

## State left behind

- My tree: `lab/ragdoll` at 911a334a, clean. The shipped `BP_SkaterVisual_TB1_P3`, `BP_SkaterVisual_TB1`, both animation Blueprints and `BP_SkaterVisual_TB2_P3` hash to their LFS ids. The lead's exclude rule hides `Skaters/Lab/`, so the two new files were added with `git add -f`.
- Console variables and the physics settings were set back before each session ended; none was saved. No process of mine is running.
- Lab folder additions: `child_tpl3.dsl` (the Blueprint's schedule as written, for both children), `mk_child.py`, `mkvars.sh`, `mk_defaults.py`, `n2_frames.py`, `q3_body.py`, `q5_free.py`, `anaw.mjs`, `casesW.py`, `shapes_night.json`, `runs\F_*.txt` and `runs\S_*.txt` (per-knockdown tables without and with sub-steps), `runs\M3.json`, `runs\M4.json`, `film\loop3\`.

## Ledger rows for part 3 (11 fields; these replace part 2's rows if the files are adopted)

```
| Game/Plugins/JamAssets/Content/Skaters/Lab/BP_SkaterVisual_TB1_P3_Rag.uasset | other | 0.65 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) |  |  | 2026-11-14 | B | n | lab, not shipped: child Blueprint of BP_SkaterVisual_TB1_P3; ends-only physics on a fall run from SetKnockedDown and its tick (parent called first; ticks after its mesh): forearms, neck and head, shins and feet and the stick from frame 4 to the hand-back (hold frame + 18 or 9 frames before the get-up), zeroes the whip on head and stick arm while simulating (RagWhipYield), wall falls with a wall box behind RagWalls (off); RagAnim, RagFloor, RagWall, physics asset override on Mesh (lane RAG) (683165 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Lab/BP_SkaterVisual_TB1_Rag.uasset | other | 0.65 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) |  |  | 2026-11-14 | B | n | lab, not shipped: child Blueprint of BP_SkaterVisual_TB1 (the night figure), the same schedule and parts as BP_SkaterVisual_TB1_P3_Rag on PHYS_SkaterTB1v2_Rag (lane RAG) (683039 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Lab/PHYS_SkaterTB1_P3_Rag.uasset | other | 0.03 | original | UE 5.9 PhysicsAssetToolset (MCP) + Python |  |  | 2026-11-14 | B | n | lab, not shipped: 16 bodies for SK_SkaterTB1_P3 (14 fitted capsules with seven radii reduced, a neck sphere, a thin stick capsule at mass scale 0.2), limits free and continuous collision on the simulated bodies, neck and stick joint anchors set by Python (lane RAG) (28270 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Lab/PHYS_SkaterTB1v2_Rag.uasset | other | 0.03 | original | UE 5.9 PhysicsAssetToolset (MCP) + Python |  |  | 2026-11-14 | B | n | lab, not shipped: 16 bodies for SK_SkaterTB1v2 (15 fitted capsules with head and feet reduced, a neck sphere, the stick thinned at mass scale 0.2), limits free and continuous collision on the simulated bodies, neck joint anchors set by Python (lane RAG) (28171 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Lab/PM_RagIce.uasset | other | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | n | lab, not shipped: physical material of the children's own floor and wall (friction 0.05, restitution 0, combine min) (lane RAG) (1795 bytes) |
```

---

# Part 4 (fourth loop, 03:45 to 04:35): the cost, the wall at three distances and in the corners, the adoption note

Same tree (`C:\Users\kazda\jam\wt\b-vfx`, branch `lab/ragdoll`, now at c1e028af: one more local commit of the two children; not pushed), port 8000, every editor `-nullrhi -unattended`. Three sessions (pids 37656, 36360, 39204), each left through `ExitEditor` with the discard flag and confirmed gone, the last at 04:25:00. `LogD3D12RHI` lines 0 and `Error:` lines 0 in their logs. In the lead's tree only this report is mine; `film\loop2\` and `film\loop3\` were not touched.

## 1. What is done, what is not

The cost is measured, the twelve wall cases pass, and `C:\Users\kazda\jam\rag-lab\ADOPT.md` is written. **The measurement found a cost I had not seen and this loop removed it:** the children as filmed in loop 3 kept their physics bodies alive all match long, about 2 ms of game-thread time a frame for ten figures with nobody down. The children now make their bodies at the knockdown and remove them at the hand-back: the positions are the same to 0.01 cm and a figure nobody knocks down costs nothing extra. Ten down at once could not be staged with pawns; I used ten test figures beside the match's ten, as you allowed. The GPU side is not in any number here. **And the figure lane's finding is true of this lane's runs too:** the recipe line I gave in part 1 wiped the hand-made twin in every twin test; the section after the cost says what that changed (nothing in the 20-knockdown table re-run with the cure, one wall case's first eleven frames) and part 1's recipe is corrected in place.

## 2. Per evolution

### Evolution 1: the cost

**How it was measured.** The engine's CSV profiler (`CsvProfile Frames=N`) in a headless play session at the fixed 60 ticks a second. `stat unit`'s `GameThreadTime` reads 0 without a viewport, so **game-thread time here is the sum of the profiler's exclusive game-thread timers per frame** (`Exclusive/GameThread/*`, less the profiler's own). The physics part is three of them (`Physics`, `EventWait/EndPhysics`, `SyncBodies`); the workers' share is `Exclusive/AllWorkers/Physics`.

**The scene.** The regular match (its ten pawn figures, shipped) plus ten test figures of the class under test standing at mid-ice, so twenty figures; every figure's pose forced to tick, as rendering would. "Nobody down": 840 frames per run. "Ten down": all ten test figures knocked down in the same frame, six times per run, frames 4 to 53 after each call, 300 frames per run. Three other editors were running on the machine throughout.

| Mean ms per frame (each run listed) | Game thread | Of it physics | Workers' physics | 95th percentile of the game thread |
|---|---|---|---|---|
| (a) shipped figures, nobody down | 6.25, 7.15, 7.50 | 0.41 | 0.28 | 9.4 to 10.9 |
| shipped figures, ten down (hand-made falls) | 9.06, 7.55, 9.16 | 0.43 | 0.29 | 11.6 to 14.3 |
| (b) children as filmed in loop 3, nobody down | 8.38, 9.71 | 1.30 | 1.02 | 11.8 to 13.2 |
| **(b) children with bodies on demand, nobody down** | **7.09** | **0.38** | 0.27 | 10.7 |
| (c) children, ten down, no sub-steps | 12.50, 12.69, 13.10 | 3.50 | 2.62 | 16.6 to 18.6 |
| (d) children, ten down, two sub-steps | 14.11, 14.79, 14.70 | 5.18 | 4.28 | 18.4 to 20.3 |
| children with bodies on demand, nobody down, two sub-steps | 7.20 | 0.55 | 0.40 | 10.3 |
| *the cure applied (figures whole):* shipped, nobody down | 10.55, 5.90 | 0.34 | 0.25 | 9.9 to 16.4 |
| shipped, ten down (hand-made falls) | 9.53, 7.10 | 0.35 | 0.25 | 11.4 to 13.1 |
| children with bodies on demand, nobody down | 5.09, 9.78 | 0.32 | 0.24 | 8.0 to 11.3 |
| children with bodies on demand, ten down, no sub-steps | 8.20, 15.13 | 2.66 | 1.98 | 12.8 to 18.3 |

**How far to trust it.** Inside a run the frames' standard deviation is 1.3 to 3.2 ms; five runs, two of them with shipped figures, contain stalls of 67 to 133 ms and read 4.7 to 9.1. **The game-thread totals follow the machine's other load more than my scene:** the same case read 5.1 and 9.8 ms nobody down, and 8.2 and 15.1 ms ten down, in two runs minutes apart, while three other editors worked. **The physics columns are the numbers to use:** they repeat to 0.1 ms with nobody down and to 0.3 ms with ten down within one kind of run. The upper rows were measured with every figure wiped (below); the four cured rows are with whole figures.

**Added milliseconds per frame, against (a):**

| | Game thread | Physics alone |
|---|---|---|
| (b) nobody down, loop 3's children | +2.1 | +0.89 |
| (b) nobody down, bodies on demand | +0.1 (inside the spread) | 0.0 |
| (c) ten down at once | +5.8 (+4.2 against shipped figures falling); with the cure anything from -1.3 to +8.0 against shipped figures falling, which is the machine's load, not a measurement | +3.1; with the cure +2.3 |
| (d) ten down at once, two sub-steps | +7.6 (+5.9 against shipped figures falling) | +4.8 |
| two sub-steps with nobody down | +0.2 | +0.14 |

- **Per figure that is down: +0.23 to +0.31 ms of physics a frame** (whole and wiped figures), for the 35 to 50 frames the physics runs, and about +0.4 ms of game thread where two adjacent runs agreed (+0.48 and +0.59 with sub-steps).
- **Making the bodies costs about 0.5 to 1.1 ms per figure, once, in the frame of the call**: ten knockdown calls took 9.8 to 24.1 ms (median 16.0) on the children against 4.4 to 12.1 (median 5.0) on shipped figures, by Python's clock around the ten calls; with the cure, in the quieter pair of runs, 7.1 to 11.5 (median 8.2) against 3.0 to 12.7 (median 3.5).

**My verdict against a 16.7 ms frame.** In play it fits. My four matches had 3 to 6 knockdowns in 100 s, so the cost is a quarter to half a millisecond on the frames of a fall, up to a millisecond once at the hit, and nothing otherwise. If the other device holds its frame with more than a millisecond of game-thread headroom, this does not break it. Ten down at once is another matter: +2.3 to +3.1 ms of physics, and game-thread totals of 8 to 15 ms with up to one frame in twenty over 16.7 ms, on this machine with three other editors running; that case was staged and does not occur in a match. Sub-stepping costs little in play (+0.14 ms always, +0.17 ms more per figure down); I would still leave it off unless the film shows the one-frame dips.

**What changed in the children to remove the idle cost.** The mesh no longer carries the physics asset. At `SetKnockedDown(true)` the child gives it to the mesh (variable `RagPA`) and binds the motor component; when the physics stops it unbinds and takes it away. **Positions against loop 3's children: 0.000 cm** on the 20 clay knockdowns with the whip off and with it on (33,908 samples each), the 20 night knockdowns, and the 12 wall cases (30,324 coordinates per figure).

### Evolution 2: the wall at three distances and in the corners, whip on

Twelve cases, six per figure: a straight board at 40, 66 and 90 cm (mesh shift 5, 30, 30 cm) and three corner arcs at 40, 66 and 90 cm from the arc; the wall ahead, behind and to each side; Strength 1.0 sent to both figures; `RagWalls` on.

| | Clay (6) | Night (6) |
|---|---|---|
| Handed back within 3 cm | **6 of 6** (0.09 to 0.82 cm, and 2.34 in a corner at 66 cm) | **6 of 6** (0.15 to 0.27, and 2.28 in the same corner case) |
| A simulated shape further past the boards than its twin ever is in that take | **0 of 6** | **0 of 6** |
| Furthest a simulated shape is past the boards plane | 6.3 cm (the head; the wall box stands 6 cm behind the plane) | 0.9 cm |
| Further past the plane than the twin on the same frame, at most | 3.4 cm | 0.0 cm |
| Lowest simulated shape (no sub-steps) | 0.1 to 1.0 cm under the ice | 1.6 to 2.7 cm under |
| NaN, get-up end minus release (12 lines per figure) | 0, 0.0 | 0, 0.0 |

So 12 of 12 on both counts as I defined them, with one honest caveat: on the same frame the clay ragdoll's head can be up to 3.4 cm nearer the boards than the twin's, inside the 6 cm I set the wall back.

For the figure lane, from the twin alone: the stick is 14 to 18 cm past the boards plane in the first three frames of `Board_Fall_Front` at 40 and 66 cm, and 74 to 105 cm past it on frame 1 of `Board_Fall_Back` at 66 and 90 cm.

### Evolution 3: the adoption note

`C:\Users\kazda\jam\rag-lab\ADOPT.md`: the five files and where they go, that they name each other by path, the checks before a push with the cook check, the two class paths, the switches and their defaults, three ways to turn it off, and the steps for when the parent gains `WhipHeadW` and `WhipArmW`.

### The film folders

`film\loop4\` holds the five files as they now are (bodies on demand) and its own scripts (`film4_make_clay.py`, `film4_make_night.py`, `film4_make_walls_clay.py`, `film4_status.py`, `film4_off.py`). `loop3\` is as it was. **The picture lane can film either: the motion is the same to 0.01 cm.** Adopt from `loop4`.

**From a clean start on the committed files** (a fresh editor, the fourth commit): the 20 clay knockdowns with the whip on and the 20 night knockdowns give the positions of part 3 to 0.000 cm (33,908 samples each, across the restart), and the three `film4` scripts ran headless (10, 10 and 4 pairs, no error). After that I rebuilt the `film4` scripts once more with the cure below; the rebuilt files were parsed, not run again.

### The headless recipe wiped the twin: what that did to this lane's numbers

**Confirmed here.** A plain `set_editor_property` of the mesh's pose-tick option on a play-session figure resets the figure's Blueprint variables: in my harness the hand-made twin read `HoldAt` 0, `StumF` 0, `RxGap` 0 afterwards; with `unreal.PropertyAccessChangeNotifyMode.NEVER` it reads 9.0, 16, 0.35 and the pose still ticks.

**Which of my runs set it the plain way:**

| Runs | Who was wiped | What it changed |
|---|---|---|
| Every 20-knockdown table of parts 2, 3 and 4, both figures | the hand-made twin, always. The child's mesh already had the option, so that line changed nothing on it; I also set the child's own variables the plain way and cannot say now whether that wiped it | **Nothing I measure.** Re-run with the cure, clay, whip on, 5000 / 106: every number of the table is the same (19 of 20, worst 4.06 cm, lag 6.1 / 50.6 cm, blade 20 of 20) and every recorded value of twin and child equals the wiped run's: 0.000 cm over 135,632 values |
| The wall cases of parts 3 and 4 | the same | Clay re-run with the cure: the child's 50,540 recorded values are unchanged and five of six twins are unchanged. In the sixth (`Board_Fall_Front` at 40 cm) the whole twin's first eleven frames differ from the wiped twin's by up to 20.4 cm at the blade. Hand-back 6 of 6 with the same values, nothing further past the boards than the twin. Night was not re-run |
| The cost runs of the table's upper rows | all twenty figures, the match's pawn figures included | Re-measured with whole figures in the four cured rows: the physics timers moved by a few tenths, the totals by the machine's load |
| The seeded matches of parts 2 and 3 | nobody: my recorder only reads, and the child's own default already ticks its pose | nothing |
| Part 1's demonstration on the ten game figures | all ten | only that demonstration |

**Why it did not matter for the falls:** each fall writes its own hold frame before the child reads it (the hold frames in my records are 13 to 28, never 0), and get-up end minus release comes from the call's own length. What the wipe zeroes is what begin-play sets: reaction gaps, the stumble and brake gates.

**Corrected:** part 1's recipe (in place, marked), my harness (`rag2.py`, `cost.py`), and the `film4` scripts, which now also set the pose option before the team colour. **Not corrected, because the picture lane is using them:** the scripts beside `loop2` and in `loop3` set the option the plain way, after the team colour. The motion they film is the measured one. Whether the wipe also resets the team colour I could not see; if blue and red look alike in that film, that is why, and the child is the figure 150 cm to the twin's right.

**No match was run with the on-demand children.** I queued one (seed 7920, swapped in as before); its first step was refused because the freshly started editor was still busy, the script's guard then skipped the swap, and I did not retry in the time left. The swapped file was never replaced (its hash is the LFS id). Parts 2 and 3's matches ran with the earlier children.

## 3. Not verified

- A match with the on-demand children (above); the night wall cases with the cure; the other 20-knockdown tables with the cure (only clay with the whip on was re-run).
- **Anything by eye**, and anything on the GPU side: these are game-thread and physics timers of a headless editor, not frame times of the game.
- Ten pawns down at once: the ten were test figures, standing still, beside the match's ten shipped figures.
- The idle cost of the children as pawn figures in a match was not timed; the "nothing extra" is from ten test children.
- The cost of making the bodies inside one frame: timed by Python's clock around ten calls, not by the profiler.
- The night figure's cost (clay only).
- The game-thread totals: 5 to 10.5 ms for the same shipped scene depending on the minute, so they are not the game's real game-thread time; only the physics timers repeat.
- The film scripts on a GPU editor; a cook; a packaged build.

## 4. For the engine lane

- **The children as they now stand:** no bodies until a knockdown; bodies and motors made in the frame of `SetKnockedDown(true)`; physics from frame 4; hand-back from hold frame + 18 or 9 frames before the get-up; bodies removed when the physics stops. `RagEnabled` on, `RagWhipYield` on, `RagWalls` off, 5000 / 106.
- **Must never:** give the mesh a physics asset in the Blueprint's defaults again (that is the idle cost); call anything on the mesh's bodies outside a fall (there are none).
- For the figure lane: the stick past the boards in `Board_Fall_Front` and `Board_Fall_Back`, above.
- A fifth tool trap: `execute_python` refuses an `import` inside an `if`; the whole script fails with "imports must be at the top level". A sixth: an MCP call made while a freshly started editor is still loading answers "the Unreal Editor is running a slow task" and does nothing.
- **The figure lane's cure is the rule for every script here:** properties of a play-session figure are set with `PropertyAccessChangeNotifyMode.NEVER`.
- When the parent's `WhipHeadW`, `WhipArmW` and `SetTestMode(32)` reach `dev-b`: the children need no change to keep working; `ADOPT.md` section 7 has the check to run first and the steps to move the yield onto the weights.

## 5. For the other device

- The two class paths, as in part 3 and in `ADOPT.md` section 4. No collision channel change.
- **The cost to expect on the game thread:** nothing with nobody down; +0.2 to +0.4 ms a frame per figure that is down, for under a second; +0.5 to +1 ms once in the frame of the knockdown. With `bSubstepping` and two sub-steps: +0.14 ms always and +0.6 ms per figure down.

## 6. Decisions for the operator

1. **Ship or not is still the film's call**; the cost no longer argues against it.
2. **Sub-stepping** is cheap in play. It is still a global physics setting; take it only if the one-frame dips show.
3. **Wall falls with physics** now pass 12 of 12 at three distances and in corners with the whip on. They are still off by default until filmed.

## 7. The next three evolutions

1. **The weights in the parent** when the figure lane has them (`ADOPT.md` section 7). **Proving number: the child names no animation Blueprint and the 20 knockdowns keep their positions to 0.01 cm.**
2. **The cost on the GPU machine with pawns.** The children as the game's figures in a 100 s match, the profiler's physics timers against the shipped figures. **Proving number: the mean and 95th-percentile game-thread time of the two matches within 0.3 ms of each other outside falls.**
3. **The knockdown frame.** Make the bodies one frame after the call, spread over the three frames before the physics starts, if the one-off millisecond shows. **Proving number: no frame of a knockdown more than 0.5 ms above its neighbours in the profiler.**

## Would I ship it

Unchanged: **yes, if the film is liked, and only then**; the list is the five files in `film\loop4\`, and `ADOPT.md` is the procedure. The cost was my open reason to say no; measured, it is a quarter to half a millisecond during a fall and nothing otherwise.

## State left behind

- My tree: `lab/ragdoll` at c1e028af, clean. The shipped figure Blueprints, both animation Blueprints and `BP_SkaterVisual_TB2_P3` hash to their LFS ids.
- The physics settings and console variables were set back before each session ended; none was saved. No process of mine is running.
- Lab folder additions: `ADOPT.md`, `child_tpl4.dsl` (the schedule as now written into both children), `lazy.py`, `cost.py`, `cost.sh`, `costsum.mjs`, `csvstat.mjs`, `casesW12.py`, `anaw2.mjs`, `runs\cost_*\` (the profiler captures), `runs\W12C.json`, `runs\W12N.json`, `runs\L_*.json`, `runs\CWc.json` and `runs\W12Cc.json` (the cured re-runs), `film\loop4\`.

## Ledger rows for part 4 (11 fields; the two children replace part 3's rows, the other three rows of part 3 stand)

```
| Game/Plugins/JamAssets/Content/Skaters/Lab/BP_SkaterVisual_TB1_P3_Rag.uasset | other | 0.67 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) |  |  | 2026-11-14 | B | n | lab, not shipped: child Blueprint of BP_SkaterVisual_TB1_P3; ends-only physics on a fall run from SetKnockedDown and its tick (parent called first; ticks after its mesh): the physics bodies are made at the knockdown (RagPA) and removed at the hand-back; forearms, neck and head, shins and feet and the stick from frame 4 to the hand-back (hold frame + 18 or 9 frames before the get-up); zeroes the whip on head and stick arm while simulating (RagWhipYield); wall falls with a wall box behind RagWalls (off); RagAnim, RagFloor, RagWall (lane RAG) (704796 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Lab/BP_SkaterVisual_TB1_Rag.uasset | other | 0.67 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) |  |  | 2026-11-14 | B | n | lab, not shipped: child Blueprint of BP_SkaterVisual_TB1 (the night figure), the same schedule and parts as BP_SkaterVisual_TB1_P3_Rag on PHYS_SkaterTB1v2_Rag (lane RAG) (704662 bytes) |
```
