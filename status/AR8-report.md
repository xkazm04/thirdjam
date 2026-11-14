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
