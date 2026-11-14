# AR8 report: a switch for B's arena (job 1 only; jobs 2 and 3 not started)

Lane AR8, device B, machine clock 04:27 to 04:43 (2026-11-14). Tree `C:\Users\kazda\jam\wt\b-arena` at dev-b b25aa3ce, rebuilt first (exit 0). Renderer-less editor on port 8008 (`-nullrhi -unattended`, 0 `LogD3D12RHI` lines), never a GPU editor. No commit, no push, no cook. Console variables read back restored (`jam.Cut hockey.regular`, `jam.ScriptHuman 0`, `Slate.bAllowThrottling 1`).

**I hand back job 1 alone, now, so that your merge, build and cook have the time.** Jobs 2 (lamp arms, far end) and 3 (one owner, honest barn) are not started: both rewrite the same rig file, so doing them in this tree before you have taken job 1 would tie the switch to things no GPU has seen. Send me on and I start job 2 at once.

**One editor process of mine died, not through `ExitEditor`:** pid 25628 hit an engine assertion inside the Blueprint DSL tool (`Assertion failed: Outer != nullptr`, `UObjectHash.cpp` line 1587, script stack `BlueprintDslToolset.Write`) on the write that followed a refused write. Nothing had been saved in it; the tree was clean. Its port stayed in a listening state with no owner for about a minute. The second process (pid 36136) built the switch and the third (pid 33040) took the variable's read out again and proved the result; both left through `ExitEditor` with nothing unsaved and are confirmed gone (04:37:04 and 04:40, `LogExit: Exiting.` in both logs). After each exit the port showed leftover connections for up to a minute and the start tool refused it; I waited until the port was empty. For the other lanes: after the DSL tool refuses a write, restart the editor before the next write, or ask it for the node's schema first.

## 1. The switch

### The honest answer about the console variable

**A Blueprint cannot define a console variable in this engine, and cannot read one that only an ini file made.** I read the engine's source: the Blueprint node reads through `FindConsoleVariable`, which returns nothing for a variable created from an ini (`ECVF_CreatedFromIni`). So `jam.arena.crowd` works only once code registers it, and that code is the other device's.

What is built, and works today without their code:

**The console line for the other device: `ke * ArenaCrowd 0`** (or `1`, or `2`).

| Value | What it does |
|---|---|
| 0 | nothing of B's arena: the live crowd owner (stands, fans, gate, loft, machine, props) and `Dress_P3_Clay` (roof bays, wall bays, bunting) are hidden |
| 1 | the stands and the barn without the fans: the twelve fan components are hidden |
| 2 | everything (the default) |

- `ke` is the engine's own command that calls a Blueprint event by name on every object; `ArenaCrowd` is a new custom event of `BP_ArenaCrowd` with one integer.
- **Send it after the match has started.** The rig streams in a few seconds after the play session starts; a line sent before that reaches nobody (my first try did exactly that).
- **It is compiled out of shipping builds** (`#if !UE_BUILD_SHIPPING` around it in `UnrealEngine.cpp`). Development and test builds have it.
- It hides, it does not unload: the instances stay in memory and the owner still ticks. For frame time and draw calls that is the right comparison; for load time it is not.

**The variable's read is taken out again (your call, 04:38):** it logged 7 `Failed to find console variable 'jam.arena.crowd'` warnings on every rig load. The Blueprint now reads no console variable. To bring it back in one edit when the other device has registered `jam.arena.crowd` (for example `static TAutoConsoleVariable<int32> CVarArenaCrowd(TEXT("jam.arena.crowd"), 2, TEXT("0: nothing of B's arena, 1: stands and barn without fans, 2: everything."));`), write these two entries over the Blueprint's (the variables `Mode` and `HasVar` are still in it, unused until then); this is the text that compiled and ran at 04:34, in the DSL tool's own form:

```
(event `self.ReceiveBeginPlay`
  (set `self.GoalTimePlusX` -1000.0)
  (set `self.GoalTimeMinusX` -1000.0)
  (set `self.Mode` 2)
  (bind s (`KismetSystemLibrary.GetConsoleVariableStringValue` "jam.arena.crowd"))
  (set `self.HasVar` (!= s ""))
  (if (`self.HasVar`) (set `self.Mode` (`KismetStringLibrary.Conv_StringToInt` s)))
  (bind dress (`GameplayStatics.GetAllActorsWithTag` "B_ArenaDress"))
  (for a dress (`self.SetActorHiddenInGame` a (== (`self.Mode`) 0)))
  (if (> (`KismetMathLibrary.Abs` (- (.x (`self.GetActorScale3D`)) 1.0)) 0.02)
    (`self.K2_DestroyActor`)
    (else
      (add `JamEventBus.OnJamEvent` (subsystem `JamEventBus`) (`|CreateDelegate` :!event `HandleJamEvent`))
      (`self.ArenaCrowd` (`self.Mode`)))))

(event `self.ReceiveTick`
  (bind fp (* (- (`GameplayStatics.GetTimeSeconds`) (`self.GoalTimePlusX`)) 60.0))
  (bind fm (* (- (`GameplayStatics.GetTimeSeconds`) (`self.GoalTimeMinusX`)) 60.0))
  (if (`self.HasVar`)
    (bind s2 (`KismetSystemLibrary.GetConsoleVariableStringValue` "jam.arena.crowd"))
    (if (!= (`KismetStringLibrary.Conv_StringToInt` s2) (`self.Mode`)) (`self.ArenaCrowd` (`KismetStringLibrary.Conv_StringToInt` s2))))
  (`KismetMaterialLibrary.SetScalarParameterValue` "/JamAssets/Arena/MPC_Crowd.MPC_Crowd" "CrowdCheerPlusX" (* (`KismetMathLibrary.FClamp` (/ fp 8.0)) (`KismetMathLibrary.FClamp` (/ (- 108.0 fp) 20.0))))
  (`KismetMaterialLibrary.SetScalarParameterValue` "/JamAssets/Arena/MPC_Crowd.MPC_Crowd" "CrowdCheerMinusX" (* (`KismetMathLibrary.FClamp` (/ fm 8.0)) (`KismetMathLibrary.FClamp` (/ (- 108.0 fm) 20.0)))))
```

With that text the owner takes the variable's value at BeginPlay (also for `Dress_P3_Clay`) and follows it when it changes. It ran here only with the variable missing.

**Tags, as you asked for the fallback:** `B_Arena` on the seven crowd owners, on `Dress_P3_Clay` and on all their instanced components; `B_ArenaDress` on `Dress_P3_Clay`; `B_Fan` on the fan components. I know no stock console command that hides by tag; the tags are what the event itself uses, and they are there for any code that wants them.

### Proof, headless (the first table is from 04:35, with the variable's read still in; the saved Blueprint without it is proven below)

A real match on the regular rink and on Pond, the line typed through the console, then the owner, its components and the dressing actor read back:

| Rink | Value | Owner hidden | Fan components visible (instances) | Stand components visible (instances) | Gate, loft, machine, props visible | `Dress_P3_Clay` hidden (instances) |
|---|---|---|---|---|---|---|
| Regular | as loaded (2) | no | 12 of 12 (664) | 3 of 3 (56) | 4 of 4 | no (54) |
| Regular | 1 | no | **0 of 12** | 3 of 3 | 4 of 4 | no |
| Regular | 0 | **yes** | 0 of 12 | (owner hidden) | (owner hidden) | **yes** |
| Regular | 2 | no | 12 of 12 (664) | 3 of 3 (56) | 4 of 4 | no |
| Pond | as loaded (2) | no | 12 of 12 (491) | 3 of 3 (36) | 3 of 3 | no (54) |
| Pond | 1 | no | 0 of 12 | 3 of 3 | 3 of 3 | no |
| Pond | 0 | yes | 0 of 12 | (owner hidden) | (owner hidden) | yes |

- **The cheer still answers at 2:** a `goal` on the bus at +x: `CrowdCheerPlusX` 0.125, 0.5, 1 at frames 1, 4, 8; 1 to frame 88; 0.5 at 98; 0 at 108; 332 fans of the +x half rise, 0 of the other. 60 ticks per wall second.
- **No error at 0:** with everything hidden a `goal` at -x still runs its 108 frames in the scalar (nothing shows); 0 Blueprint runtime errors, 0 `AttachTo` warnings, 0 warnings naming B's arena in the log.
- Instance counts are unchanged by the switch at every value (664 fans, 56 stand pieces on the regular rink; nothing is removed).
- The rig's geometry is untouched: the tagging script moved nothing; after it the regular rink measures as in AR5 (664 fans, scale 0.96 to 1.04, 0 bounds in the clear zones, 0 bounds meeting device A's pieces).

### The saved Blueprint, without the variable's read (04:40, a fresh editor process, the regular rink)

- **The rig loads clean:** 0 `Failed to find console variable` lines, 0 warnings or errors naming B's arena (the Blueprint, the collection, the masters, the owners, the meshes, `ArenaCrowd`, `jam.arena`), 0 Blueprint runtime errors, 0 `AttachTo` warnings in the whole log.
- **The line gives the same table:** as loaded 12 of 12 fan components visible (664), stands 3 of 3 (56), gate, loft, machine, props 4 of 4, `Dress_P3_Clay` shown (54 instances); `ke * ArenaCrowd 1`: fans 0 of 12, the rest as before; `ke * ArenaCrowd 0`: owner hidden, `Dress_P3_Clay` hidden; `ke * ArenaCrowd 2`: everything back.
- **The cheer answers:** a `goal` at +x: `CrowdCheerPlusX` 0.125, 0.5, 1 at frames 1, 4, 8; 1 to frame 88; 0.5 at 98; 0 at 108; 332 fans of the +x half, 0 of the other; 59.9 ticks per wall second. (At frame 140 the match scored a real goal at -x and that half started up.)
- The regular rink still measures 664 fans, scale 0.96 to 1.04, 0 bounds in the clear zones, 0 meeting device A's pieces.

### What I found on the way, not mine

In my renderer-less editor the new sounds log errors during a match: `Failed to create CompressedAudioInfo` for `/JamAssets/Audio/SFX/SFX_Brake_a`, `SFX_PuckBoards*`, `SFX_SkateCarve*` (9 lines) and one handled ensure `Failed to parse seektable in 'SFX_SkateCarve_02' chunk=1`. It may be the missing audio device of a headless editor; the audio lane should look once in a normal one.

## Not verified (what the GPU must look at)

- **That hidden means not drawn**, on a picture: one still at each value.
- The variable's path (now out of the Blueprint, as text above): it never ran with a value, because nothing registers `jam.arena.crowd`.
- A cook and a packaged build with the changed Blueprint and rig.
- `ke` in the other device's packaged build configuration (it is absent from shipping builds).
- A rink length without a layout: no owner is alive there, so the line reaches nobody and `Dress_P3_Clay` stays.

## Not done

- Job 2: AR6's lamp arms and the far end's dressing.
- Job 3: one owner instead of seven and the barn at honest scale.

## Files: job 1 (saved, in the state to merge) and job 2 (not started)

**Job 1 changed exactly these, all in `C:\Users\kazda\jam\wt\b-arena`:**
- `Game/Plugins/JamAssets/Content/Arena/BP_ArenaCrowd.uasset` (the `ArenaCrowd` event; two unused variables)
- `Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap` (tags only; nothing moved, added or removed)
- `status/AR8-report.md`

**Job 2 has changed nothing.** When you resume me it will add new files under `Arena/` (three lamp arm meshes, the door, the firewood, the bench, and what materials they need) and change `Rigs/LR_P3_Clay.umap` again; I will list them apart from job 1's and keep a copy of job 1's rig file (`scratchpad\ar8\bk`) so the two states stay separable.

## Ledger rows

Two rows in the ledger's 11 columns; each replaces the row of the same path.

```
| Game/Plugins/JamAssets/Content/Arena/BP_ArenaCrowd.uasset | other | 0.19 | original | UE 5.9 editor via MCP (Blueprint DSL) |  |  | 2026-11-14 | B | y | owner actor of one rink size crowd and stands (instanced components added per actor in the rig): at BeginPlay it destroys itself unless its world scale is 1 within 0.02 (the rig scale times its own 90 / length), else binds JamEventBus.OnJamEvent; on goal it notes the time for the end whose net was scored (Where.X sign) and every tick writes MPC_Crowd CrowdCheerPlusX / CrowdCheerMinusX: up over 8 frames, held, down over frames 88 to 108 (lane AR5); since AR8 a mode switch: the custom event ArenaCrowd(NewMode), called by the console line "ke * ArenaCrowd 0", "... 1" or "... 2" in builds that are not shipping builds: 0 hides the owner and every actor tagged B_ArenaDress, 1 hides the components tagged B_Fan, 2 shows everything; it reads no console variable (two unused variables Mode, HasVar are kept for that) (lanes AR5, AR8) (187605 bytes) |
| Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap | other | 1.55 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | P3 clay rig, barn with calm ice: PPV (EV100 4.2, SSR override, focus 16 m f/4, no depth blur, motion blur 0), sky light a movable real-time capture (no cubemap; intensity 0.35 and colour unchanged) of the capture dome SkyCap_P3_Clay (engine sphere, scale 700, MI_SkyAmbient_Clay, outside the barn; B34), 4 shadowed warm key spots (5200 cd, no specular, shadow resolution 1.0 and source radius 60 cm since B34), 8 shadowless lamps over the side lanes (2100 cd, broad beams, no specular; B28), 6 shadowless fills at lower intensity (B28), 18 lamp shades (6 of them decorative beyond the end boards, twice as wide since B29), LookParams (FeltRim 1.4); plank walls, posts, trusses, roof, floor; no bulb strings; all shadowless meshes, no collision; the old ten trusses replaced by ten roof bays (SM_Barn_TrussBay, height scale 0.82), 32 wall bays in front of the plank walls and 12 lengths of bunting hung at 13.05 m, all instances on one actor Dress_P3_Clay that scales with the rig; timber stands and a clay crowd as instances on seven actors Crowd_P3_Clay_k0667, k0711, k0778, k0889, k1000, k1333, k1556 (class BP_ArenaCrowd, one layout in real size per rink length 60, 64, 70, 80, 90, 120, 140 m, each scaled by 90 / length so that only the one whose rink is loaded has scale 1 and stays; the others destroy themselves at BeginPlay): on the regular rink 56 stand pieces and 664 fans (twelve figures with a far level, per-instance cheer weight, coat value, accent value, end and delay, front row empty, corner rows three quarters full), the ice gate, its props, a loft of two bays and the ice machine at the +x end wall; no light added, no shadows, no collision (lanes B9, B14, B18, B19, B22, B23, B27, B28, B29, B34, AR2, AR5); since AR8 the tag B_Arena on the seven crowd owners, on Dress_P3_Clay and on all their instanced components, B_ArenaDress on Dress_P3_Clay, B_Fan on the fans components (nothing moved) (lane AR8) (1546530 bytes) |
```

---

# Part two: jobs 2 and 3, as built and counted without a GPU (04:43 to 04:58)

Written at 05:25, after the two urgent lanes. Both jobs were then put in front of the GPU in lane AR10: what was seen is in `status/AR10-report.md`; this part is what was built and counted. **Nothing here is in the operator's package.**

Editor processes of this part, all renderer-less on port 8008: pid 32004 **died** on my own script (an import and a level swap in one call: the engine stops with "World Memory Leaks" when a Python variable still holds the rig's package while the editor swaps worlds; nothing had been saved; I now drop every reference and collect garbage before the swap); pid 35516 did jobs 2 and 3 and left through `ExitEditor` (04:57:01, nothing unsaved, confirmed gone).

## Which files belong to which job

All under `C:\Users\kazda\jam\wt\b-arena`. The tree's rig is in **job 2's state**; job 3's rig is kept beside it and is not in use.

**Job 2 (in the tree, uncommitted):**
- `Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap` (1,607,256 bytes; job 1's was 1,546,530)
- `Game/Plugins/JamAssets/Content/Arena/SM_Barn_LampArm_075.uasset`, `SM_Barn_LampArm_150.uasset`, `SM_Barn_LampArm_225.uasset`
- `Game/Plugins/JamAssets/Content/Arena/SM_Barn_SlidingDoor.uasset`, `SM_Barn_Firewood.uasset`, `SM_Barn_SkateBench.uasset`

**Job 3:**
- `Game/Plugins/JamAssets/Content/Arena/BP_ArenaCrowdOne.uasset` (931,028 bytes; in the tree, referenced by nothing in job 2's rig)
- its rig, **not in the tree's content**: `Game/Saved/AR8/LR_P3_Clay.job3.umap` (363,270 bytes; `Game/Saved` is ignored by git; a second copy in `scratchpad\ar8\bk`). To try job 3: with the editor closed, copy it over `Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap`. Job 3's rig holds job 2's lamp arms and far end too.
- Copies of job 1's and job 2's rig files are in `scratchpad\ar8\bk` (`LR_P3_Clay.job1.umap`, `LR_P3_Clay.job2.umap`).

## Job 2: the lamp arms and the far end

**Lamp arms: all 18 placed, none skipped.** Read from the rig's real lamp shades (their bounds give the rod's axis and top) and from the roof bays' real places:

| Lamp | x, y (m) | Rod top (m) | Nearest truss at x (m) | Distance | Arm | Arm length scale | Gap |
|---|---|---:|---:|---:|---|---:|---:|
| `Shade_1` | -33.75, 0.00 | 22.94 | -36 | 2.25 m | `225` | 1.000 | 0.00 m |
| `Shade_2` | -11.25, 0.00 | 22.94 | -12 | 0.75 m | `075` | 1.000 | 0.00 m |
| `Shade_3` | 11.25, 0.00 | 22.94 | 12 | 0.75 m | `075` | 1.000 | 0.00 m |
| `Shade_4` | 33.75, 0.00 | 22.94 | 36 | 2.25 m | `225` | 1.000 | 0.00 m |
| `Shade_5` | -33.75, -13.50 | 21.44 | -36 | 2.25 m | `225` | 1.000 | 0.00 m |
| `Shade_6` | -11.25, -13.50 | 21.44 | -12 | 0.75 m | `075` | 1.000 | 0.00 m |
| `Shade_7` | 11.25, -13.50 | 21.44 | 12 | 0.75 m | `075` | 1.000 | 0.00 m |
| `Shade_8` | 33.75, -13.50 | 21.44 | 36 | 2.25 m | `225` | 1.000 | 0.00 m |
| `Shade_9` | -33.75, 13.50 | 21.44 | -36 | 2.25 m | `225` | 1.000 | 0.00 m |
| `Shade_10` | -11.25, 13.50 | 21.44 | -12 | 0.75 m | `075` | 1.000 | 0.00 m |
| `Shade_11` | 11.25, 13.50 | 21.44 | 12 | 0.75 m | `075` | 1.000 | 0.00 m |
| `Shade_12` | 33.75, 13.50 | 21.44 | 36 | 2.25 m | `225` | 1.000 | 0.00 m |
| `Shade_13` | -50.00, -12.00 | 14.58 | -48 | 2.00 m | `225` | 0.889 | 0.00 m |
| `Shade_14` | -50.00, 0.00 | 14.58 | -48 | 2.00 m | `225` | 0.889 | 0.00 m |
| `Shade_15` | -50.00, 12.00 | 14.58 | -48 | 2.00 m | `225` | 0.889 | 0.00 m |
| `Shade_16` | 50.00, -12.00 | 14.58 | 48 | 2.00 m | `225` | 0.889 | 0.00 m |
| `Shade_17` | 50.00, 0.00 | 14.58 | 48 | 2.00 m | `225` | 0.889 | 0.00 m |
| `Shade_18` | 50.00, 12.00 | 14.58 | 48 | 2.00 m | `225` | 0.889 | 0.00 m |

- The 12 lamps over the ice stand exactly 0.75 or 2.25 m from a truss: the arm fits as built.
- **The 6 end lamps stand at x = +-50.0 m, y = -12 / 0 / +12 m** (AR6 assumed 49.5 and 13.5), 2.0 m from the truss at +-48 m. No arm is 2.0 m long: I took the 2.25 m arm at 0.889 of its length so its plate sits on the rod. Unscaled it would miss the rod by 0.25 m, inside your 0.3 m. Their rods end at 14.58 m, inside the plate's height (14.3 to 14.6 m), with 2 cm to spare.
- `SM_Barn_LampArm_150` is imported and not used.
- The arms are instances on `Dress_P3_Clay` (two components, 6 + 12), so they scale with the rig like the trusses and lamps: 1,440 triangles.
- The roof bays are squeezed to 0.82 in height, the arms are not: an arm lies about 4 cm above the squeezed chord by my arithmetic. For the eye.

**The far end:** a sliding door in the middle of the fourth bay of the -x end wall (y = +6.2 m), the firewood in the next bay (y = +18.5 m), the skate bench on the door's other side (y = -6.2 m), in AR4's wall frame, at the barn's scale, on each of the seven crowd owners. 1,314 triangles.

Counted in headless play (each rink's own match; bounds from world transforms):

| Rink | Lamp arms (instances) | Lowest point of an arm over the ice | Door, firewood, bench | Bounds under 7 m in the clear zones | New pieces' bounds meeting a stand | Fans / stand pieces (unchanged) |
|---|---:|---:|---|---:|---:|---|
| Regular | 18 | 13.59 m | 1, 1, 1 at scale 1.000 | 0 | 0 | 664 / 56 |
| Pond | 18 | 9.06 m | 1, 1, 1 at scale 0.667 | 0 | 0 | 491 / 36 |
| Large | 18 | 18.12 m | 1, 1, 1 at scale 1.333 | 0 | 0 | 830 / 74 |
| Wide | 18 | 21.14 m | 1, 1, 1 at scale 1.556 | 0 | 0 | 944 / 86 |

- The switch of job 1 still works with them (`ke * ArenaCrowd 0 / 1 / 2`: the new pieces hide with their owners; 72 instances on `Dress_P3_Clay`).
- 0 warnings naming B's arena in the log; 0 Blueprint runtime errors.

## Job 3: one owner instead of seven

**Built as a second Blueprint, `Arena/BP_ArenaCrowdOne`, so that the merged `BP_ArenaCrowd` and its rig are untouched.** The rig holds one actor `Crowd_P3_Clay` with 15 empty instanced components (3 stand meshes, 12 figures) and the seven wall pieces baked at the barn's scale. At BeginPlay the Blueprint reads the rig's scale and lays out stands and fans in real size for that rink length: the same rule as the baked layouts (stand line 4.0 m beyond the boards, as many modules as fit, terraces behind the nets, corner pieces, 44 % of the seats, three corner seats in four), with its own cheap random numbers. It hides the loft where it does not fit. It carries the cheer and the `ArenaCrowd` switch of `BP_ArenaCrowd`, and no scale test: it never destroys itself.

| Rink length | Owners alive / destroyed at load | Stand pieces (the baked layout had) | Fans built (baked) | Fan scale | Mean fan height | Stand front, side / end (m) | Bounds in the clear zones | Bounds meeting device A's pieces | Wall pieces meeting a stand |
|---|---|---|---|---|---:|---|---:|---:|---|
| 60 m | 1 / 0 | 36 (36) | 476 (491) | 0.96 to 1.04 | 1.41 m | 19.0 / 34.0 | 0 | 0 | 0 (the loft, hidden by the Blueprint there, would meet 3) |
| 90 m | 1 / 0 | 56 (56) | 642 (664) | 0.96 to 1.04 | 1.41 m | 26.5 / 49.0 | 0 | 0 | 0 |
| 100 m (no baked layout) | 1 / 0 | 62 (62) | 722 (703) | 0.96 to 1.04 | 1.41 m | 29.0 / 54.0 | 0 | 0 | 0 |
| 120 m | 1 / 0 | 74 (74) | 834 (830) | 0.96 to 1.04 | 1.41 m | 34.0 / 64.0 | 0 | 0 | 0 |
| 140 m | 1 / 0 | 86 (86) | 932 (944) | 0.96 to 1.04 | 1.41 m | 39.0 / 74.0 | 0 | 0 | 0 |

| | Seven owners (jobs 1 and 2) | One owner (job 3) |
|---|---:|---:|
| Rig file | 1,607,256 bytes | **363,270 bytes** |
| Rig actors | 121 | 115 |
| Owners destroyed at load | 6 | **0** |
| Rink lengths with a crowd | the seven baked | **any** (100 m built and measured) |
| Blueprint | 187,605 bytes | 931,028 bytes |

- **The stands are the same pieces in the same places** as the baked layouts on every length (36, 56, 74, 86 pieces; fronts at the same lines). **The fans differ**: other seats are taken and other figures sit in them (the random numbers are the Blueprint's own), 476 to 932 against 491 to 944.
- **No two neighbours alike is weaker here:** the figure comes from a counter (every seventh of eleven), which keeps neighbours in a row apart but was not tested against the ten-nearest rule.
- The switch reaches the one owner (`ke * ArenaCrowd 1`: 0 of 12 fan components visible; `0`: owner and `Dress_P3_Clay` hidden; `2`: all back). The cheer answers: 319 fans of the +x half on a `goal` at +x, 0 of the other; in the same run the match's own goal at -x raised 323 there.
- 0 warnings naming B's arena, 0 Blueprint runtime errors, 0 `AttachTo` warnings in 10 play sessions.
- **Cost of building at BeginPlay was not measured** (about 650 to 950 fans, each added with five data values, in Blueprint).
- **The barn at honest scale was not attempted without eyes**; see AR10.

## Ledger rows (11 columns)

Job 2: six new rows and the rig's row (replaces the row of the same path):

```
| Game/Plugins/JamAssets/Content/Arena/SM_Barn_LampArm_075.uasset | mesh | 0.07 | original | Blender 4 script (lane AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | a timber arm from a truss chord to a lamp rod, reach 0.75 m, with an iron strap, plate and chain, 72 tris, slots Timber, Tin; in the clay rig 6 times (the lamps 0.75 m from a truss); no collision, vertex colours, materials bound to the slots (lane AR8 job 2) (72813 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Barn_LampArm_150.uasset | mesh | 0.07 | original | Blender 4 script (lane AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | the same arm with a stay, reach 1.50 m, 84 tris, slots Timber, TimberDark, Tin; imported, not placed (no lamp of the rig stands 1.5 m from a truss); no collision, vertex colours, materials bound to the slots (lane AR8 job 2) (74436 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Barn_LampArm_225.uasset | mesh | 0.07 | original | Blender 4 script (lane AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | the same arm with a stay, reach 2.25 m, 84 tris, slots Timber, TimberDark, Tin; in the clay rig 12 times (6 lamps 2.25 m from a truss, and the 6 end lamps 2.0 m from one with the arm at 0.889 of its length); no collision, vertex colours, materials bound to the slots (lane AR8 job 2) (74455 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Barn_SlidingDoor.uasset | mesh | 0.09 | original | Blender 4 script (lane AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | a closed sliding barn door with a hoist beam, 8.40 x 2.31 x 7.80 m, 384 tris, slots Timber, TimberDark, Tin, Rubber; on the -x end wall of every rink; no collision, vertex colours, materials bound to the slots (lane AR8 job 2) (88646 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Barn_Firewood.uasset | mesh | 0.10 | original | Blender 4 script (lane AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | a stack of split logs on skids with a chopping block, 4.57 x 0.93 x 1.78 m, 414 tris, slots Timber, TimberDark; on the -x end wall; no collision, vertex colours, materials bound to the slots (lane AR8 job 2) (95497 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Barn_SkateBench.uasset | mesh | 0.10 | original | Blender 4 script (lane AR6, built from primitives), imported by UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | a bench under a peg rail with hung skates, 3.00 x 0.91 x 1.76 m, 516 tris, slots Timber, TimberDark, Tin, Rubber; on the -x end wall; no collision, vertex colours, materials bound to the slots (lane AR8 job 2) (103649 bytes) |
| Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap | other | 1.61 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | P3 clay rig, barn with calm ice: PPV (EV100 4.2, SSR override, focus 16 m f/4, no depth blur, motion blur 0), sky light a movable real-time capture (no cubemap; intensity 0.35 and colour unchanged) of the capture dome SkyCap_P3_Clay (engine sphere, scale 700, MI_SkyAmbient_Clay, outside the barn; B34), 4 shadowed warm key spots (5200 cd, no specular, shadow resolution 1.0 and source radius 60 cm since B34), 8 shadowless lamps over the side lanes (2100 cd, broad beams, no specular; B28), 6 shadowless fills at lower intensity (B28), 18 lamp shades (6 of them decorative beyond the end boards, twice as wide since B29), LookParams (FeltRim 1.4); plank walls, posts, trusses, roof, floor; no bulb strings; all shadowless meshes, no collision; the old ten trusses replaced by ten roof bays (SM_Barn_TrussBay, height scale 0.82), 32 wall bays in front of the plank walls and 12 lengths of bunting hung at 13.05 m, all instances on one actor Dress_P3_Clay that scales with the rig; timber stands and a clay crowd as instances on seven actors Crowd_P3_Clay_k0667, k0711, k0778, k0889, k1000, k1333, k1556 (class BP_ArenaCrowd, one layout in real size per rink length 60, 64, 70, 80, 90, 120, 140 m, each scaled by 90 / length so that only the one whose rink is loaded has scale 1 and stays; the others destroy themselves at BeginPlay): on the regular rink 56 stand pieces and 664 fans (twelve figures with a far level, per-instance cheer weight, coat value, accent value, end and delay, front row empty, corner rows three quarters full), the ice gate, its props, a loft of two bays and the ice machine at the +x end wall; no light added, no shadows, no collision (lanes B9, B14, B18, B19, B22, B23, B27, B28, B29, B34, AR2, AR5); job 2 of AR8: 18 lamp arms on Dress_P3_Clay (6 SM_Barn_LampArm_075, 12 SM_Barn_LampArm_225, placed from the rig real lamp shades, each on its lamp rod) and a sliding door, a firewood stack and a skate bench on the -x end wall on each of the seven crowd owners (lane AR8 job 2) (1607256 bytes) |
```

Job 3, only if it is adopted: one new row, and the rig's row in place of job 2's:

```
| Game/Plugins/JamAssets/Content/Arena/BP_ArenaCrowdOne.uasset | other | 0.93 | original | UE 5.9 editor via MCP (Blueprint DSL) |  |  | 2026-11-14 | B | y | job 3 of AR8, NOT in use until the GPU has seen it: one owner for the crowd and stands on any rink length: at BeginPlay it reads the rig scale and fills its empty stand and fan components in real size (events Build, PlaceModule, PlaceFan, CornerFan), hides the loft where it does not fit, and has the cheer and the ArenaCrowd switch of BP_ArenaCrowd (lane AR8 job 3) (931028 bytes) |
| Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap | other | 0.36 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | P3 clay rig, barn with calm ice: PPV (EV100 4.2, SSR override, focus 16 m f/4, no depth blur, motion blur 0), sky light a movable real-time capture (no cubemap; intensity 0.35 and colour unchanged) of the capture dome SkyCap_P3_Clay (engine sphere, scale 700, MI_SkyAmbient_Clay, outside the barn; B34), 4 shadowed warm key spots (5200 cd, no specular, shadow resolution 1.0 and source radius 60 cm since B34), 8 shadowless lamps over the side lanes (2100 cd, broad beams, no specular; B28), 6 shadowless fills at lower intensity (B28), 18 lamp shades (6 of them decorative beyond the end boards, twice as wide since B29), LookParams (FeltRim 1.4); plank walls, posts, trusses, roof, floor; no bulb strings; all shadowless meshes, no collision; the old ten trusses replaced by ten roof bays (SM_Barn_TrussBay, height scale 0.82), 32 wall bays in front of the plank walls and 12 lengths of bunting hung at 13.05 m, all instances on one actor Dress_P3_Clay that scales with the rig; timber stands and a clay crowd on ONE actor Crowd_P3_Clay (class BP_ArenaCrowdOne) with 15 empty instanced components that the Blueprint fills at BeginPlay for the loaded rink length, and the gate, props, loft, ice machine, sliding door, firewood and bench baked on it at the barn scale; 18 lamp arms on Dress_P3_Clay; tags B_Arena, B_ArenaDress, B_Fan; no light added, no shadows, no collision (lanes B9, B14, B18, B19, B22, B23, B27, B28, B29, B34, AR2, AR5, AR8 job 3; this state replaces the job 2 state only if adopted) (363270 bytes) |
```
