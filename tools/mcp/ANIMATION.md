# Animation, control rig, retarget, Blueprint, Niagara through MCP

Measured 2026-11-11/12 against UE 5.9.0 (launcher install) on a throwaway C++ project made the `smoke.ps1` way
(blank C++ module + `ModelContextProtocol`, `ToolsetRegistry`, `AllToolsets`), deleted afterwards. Tooling only:
engine mannequins, basic shapes, a throwaway map. Physics determinism is in `PHYSICS.md`.

All calls went through `node tools/mcp/mcp.mjs call <Module.Toolset.Tool> <json|@file>`. "Python" means
`editor_toolset.toolsets.python_scripting.PythonScriptingTools.execute_python` with
`{"script":{"content":"<python>","language":"Python"}}` (also an MCP call; no editor UI involved). Nothing needed a human.

| # | What | Verdict | Path | Wall | MCP calls (incl. exploration) / minimal recipe |
|---|---|---|---|---|---|
| 1 | project + mannequins | PASS | file copy from the engine's template resources | build 66 s, editor to MCP 51 s (first) / 15 s (warm) | - |
| 2 | AnimBP, state machine, 2 states, float transition, compile, save | PASS | MCP Blueprint toolset + a 80-line C++ editor helper for states/transitions + 1 Python line for the skeleton | 9 min first time (incl. 26 s rebuild + 15 s restart for the helper) | 96 / about 20 |
| 3 | Control Rig + procedural node, compile, save | PASS | MCP only (`ControlRigTools`, `CompileBlueprint`, `SaveAsset`) | 2 min | 31 / 11 |
| 4 | IK Retargeter UE4 -> UE5 mannequin, one clip, saved | PASS (NOT POSSIBLE via a dedicated tool) | Python only (no retarget toolset exists) | under 1 min | 4 / 1 script |
| 6 | Blueprint pawn: capsule + code component, lean from Tick, compile, PIE | PASS | MCP only (`ActorToolset`, `ObjectToolset`, `BlueprintDslToolset`) | 2 min | 29 / 12 |
| 7 | Niagara system from template on an actor, PIE, screenshot | PASS | MCP only (`NiagaraToolset_*`, `LevelToolset`, `SceneCaptureToolset`) | 1.5 min | 20 / 8 |

Per-call latency: about 1.1-1.7 s while the editor ran in the background (throttled), 0.06-0.3 s after the live
engine got `bUseFixedFrameRate=True` (see `PHYSICS.md`, "Side effect").

## 1. Project and mannequin content

No download, no Fab: the template feature packs ship inside the engine install and are plain `.uasset` folders.
Copy them (read-only source) into the project's `Content/` under the mount names the templates use:

| Copy from (`C:\Program Files\Epic Games\UE_5.9\Templates\TemplateResources\High\`) | To | Gives |
|---|---|---|
| `Characters\Content\*` | `Content\Characters\` | UE5 skeleton `/Game/Characters/Mannequins/Meshes/SK_Mannequin`, `SKM_Manny_Simple`, `SKM_Quinn_Simple`, `CR_Mannequin_Body/FootIK/Procedural`, `PA_Mannequin`, about 100 clips (`MM_Idle`, `MF_Unarmed_Jog_Fwd`, ...), 126 MB |
| `Mannequin\Content\*` | `Content\Mannequin\` | UE4 skeleton `/Game/Mannequin/Character/Mesh/UE4_Mannequin_Skeleton`, `SK_Mannequin`, `ThirdPersonIdle/Walk/Run/Jump*`, 40 MB |

The paths must be exactly those: the assets reference `/Game/Characters/Mannequins/...` and `/Game/Mannequin/...`
(`TP_ThirdPerson/Config/TemplateDefs.ini` lists `SharedContentPacks=(MountName="Characters")`). Other copies of the UE4
mannequin live in engine plugins (`NetworkPredictionExtras/.../UE4_Guy`, `AnimGenOffline/.../UE4_Mannequin`) but they
reference their plugin mount, so they only work with that plugin enabled. Project path used:
`...\scratchpad\anim-smoke\` (110 characters); no DDC path dialog appeared.

## 2. Animation Blueprint with a state machine: PASS (MCP + C++ helper)

What MCP can do by itself: create the AnimBP, add the State Machine node, variables, everything inside a state and
inside a transition rule, wire the pose to the output, compile, save. What it cannot: **add states or transitions**.
The Blueprint toolset palette (`FindNodeTypes`) in a state machine graph offers only comments and generic K2 macros;
"Add State", "Add Conduit" and transitions are schema actions of `UAnimationStateMachineSchema`, not palette entries,
and `CreateNode {"typeId":"|AddState"}` fails (`Could not create a node of type "|AddState"`). Python cannot either:
`unreal.BlueprintEditorLibrary.create_node_from_reference` and `find_node_references` use the same palette, and
`AnimStateNode`, `EdGraph` expose no way to add nodes. The Blueprint Graph DSL covers only event and function graphs
(`BLUEPRINT SCOPES: every event and function graph`).

Fix that worked: an editor module in the project with one `UBlueprintFunctionLibrary` (below), callable from Python as
`unreal.TJAnimGraphEdLibrary`. It needs a rebuild with the editor closed (26 s incremental) and a restart (15 s);
Live Coding cannot add a new module. A jam project should carry this helper from H0.

Sequence that worked (graph refPaths are `<asset>.<asset>:AnimGraph...`, see `ListGraphs`):

| Step | Call |
|---|---|
| create | `BlueprintToolsets.BlueprintToolset.Create {"folderPath":"/Game/Smoke","assetName":"ABP_X","assetType":{"refPath":"/Script/Engine.AnimInstance"}}` |
| skeleton | Python: `bp=unreal.load_asset('/Game/Smoke/ABP_X'); bp.set_editor_property('target_skeleton', unreal.load_asset('/Game/Characters/Mannequins/Meshes/SK_Mannequin'))` (Create leaves it `None`; `ObjectToolset.SetProperties` resolves a Blueprint to its CDO and answers `no such property ... 'TargetSkeleton'`) |
| state machine node | `CreateNode {"graph":{"refPath":"/Game/Smoke/ABP_X.ABP_X:AnimGraph"},"typeId":"Animation\|StateMachines\|StateMachine","pos":{"x":-300,"y":0}}` -> `...:AnimGraph.AnimGraphNode_StateMachine_0`, inner graph `...AnimGraphNode_StateMachine_0.New State Machine` |
| states + transitions | Python with the helper: `L=unreal.TJAnimGraphEdLibrary; L.add_state(bp,'New State Machine','Idle',unreal.Vector2D(300,0)); L.add_state(bp,'New State Machine','Moving',unreal.Vector2D(700,0)); L.add_transition(bp,'New State Machine','Entry','Idle'); t=L.add_transition(bp,'New State Machine','Idle','Moving')`; state graphs `...New State Machine.AnimStateNode_0.Idle`, rule graph `...AnimStateTransitionNode_0.Transition` |
| float variable | `AddVariable {"blueprint":{...},"name":"Speed","typeName":"float"}` (becomes double) |
| rule | in the rule graph: `CreateNode "Variables\|Default\|GetSpeed"`, `CreateNode "Utilities\|Operators\|Greater(>)"`, `ConnectPins` get(out 0) -> Greater(in 0), Greater(out 0) -> `AnimGraphNode_TransitionResult_0`(in 0 `bCanEnterTransition`), `SetPinValue` Greater in 1 = `"10.0"` |
| state contents | in each state graph: `CreateNode "Animation\|Sequences\|Play'MM_Idle'"` (one palette entry per compatible clip; `FindNodeTypes` filter `Animation\|Sequences\|Play'MF_` etc.), `ConnectPins` player out 0 -> `AnimGraphNode_StateResult_0` in 0 |
| output | `ConnectPins` `AnimGraphNode_StateMachine_0` out 0 -> `AnimGraphNode_Root_0` in 0 |
| compile, save | `CompileBlueprint {"blueprint":{...}}`, `EditorToolset.AssetToolset.SaveAsset {"assetPath":"/Game/Smoke/ABP_X"}`; Python `bp.get_editor_property('status')` = `BS_UP_TO_DATE` |

Runtime check (Simulate, Python on the PIE world): `Speed=0` keeps `foot_l` still (Idle), `Speed=20` swings it
(z 6.6 -> 46.2 cm, jog). Needed: `SetVariableInstanceEditable Speed true` (else `set_editor_property` raises
`cannot be edited on instances`) and `VisibilityBasedAnimTickOption = AlwaysTickPoseAndRefreshBones` on an off-camera
mesh (else bones never refresh and every sample is identical).

The helper (editor module `Type: Editor`, deps `Core, CoreUObject, Engine, UnrealEd, BlueprintGraph, AnimGraph,
AnimGraphRuntime, Kismet`; add the module to the editor target's `ExtraModuleNames` and to `.uproject` `Modules`):

```cpp
// TJAnimGraphEdLibrary.h: UCLASS() class UTJAnimGraphEdLibrary : public UBlueprintFunctionLibrary with three
// static UFUNCTION(BlueprintCallable) members: AddState, AddTransition, GetBoundGraph (signatures as below).
#include "AnimationStateMachineGraph.h"
#include "AnimationStateMachineSchema.h"
#include "AnimStateNode.h"
#include "AnimStateEntryNode.h"
#include "AnimStateTransitionNode.h"
#include "Kismet2/BlueprintEditorUtils.h"

static UAnimationStateMachineGraph* FindSMGraph(UAnimBlueprint* BP, const FString& Name)
{
    TArray<UEdGraph*> Graphs; BP->GetAllGraphs(Graphs);
    for (UEdGraph* G : Graphs) if (auto* SM = Cast<UAnimationStateMachineGraph>(G)) if (SM->GetName() == Name) return SM;
    return nullptr;
}
static UAnimStateNodeBase* FindState(UAnimationStateMachineGraph* SM, const FString& Name)
{
    for (UEdGraphNode* N : SM->Nodes) if (auto* S = Cast<UAnimStateNodeBase>(N))
        if (!S->IsA<UAnimStateTransitionNode>() && S->GetStateName() == Name) return S;
    return nullptr;
}
UEdGraphNode* UTJAnimGraphEdLibrary::AddState(UAnimBlueprint* BP, const FString& SMName, const FString& StateName, FVector2D Pos)
{
    UAnimationStateMachineGraph* SM = FindSMGraph(BP, SMName); if (!SM) return nullptr;
    UAnimStateNode* Node = FEdGraphSchemaAction_NewStateNode::SpawnNodeFromTemplate<UAnimStateNode>(SM, NewObject<UAnimStateNode>(), FVector2f(Pos), false);
    if (Node && Node->GetBoundGraph()) FBlueprintEditorUtils::RenameGraph(Node->GetBoundGraph(), StateName); // state name = graph name
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);
    return Node;
}
UEdGraphNode* UTJAnimGraphEdLibrary::AddTransition(UAnimBlueprint* BP, const FString& SMName, const FString& From, const FString& To)
{
    UAnimationStateMachineGraph* SM = FindSMGraph(BP, SMName); if (!SM) return nullptr;
    UAnimStateNodeBase* B = FindState(SM, To); if (!B) return nullptr;
    if (From == TEXT("Entry")) {            // wire the entry node
        SM->GetSchema()->TryCreateConnection(SM->EntryNode->GetOutputPin(), B->GetInputPin());
        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP); return SM->EntryNode;
    }
    UAnimStateNodeBase* A = FindState(SM, From); if (!A) return nullptr;
    const FVector2f Loc = (FVector2f(A->NodePosX, A->NodePosY) + FVector2f(B->NodePosX, B->NodePosY)) * 0.5f;
    UAnimStateTransitionNode* T = FEdGraphSchemaAction_NewStateNode::SpawnNodeFromTemplate<UAnimStateTransitionNode>(SM, NewObject<UAnimStateTransitionNode>(), Loc, false);
    T->CreateConnections(A, B);
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);
    return T;
}
UEdGraph* UTJAnimGraphEdLibrary::GetBoundGraph(UEdGraphNode* Node)
{ if (auto* S = Cast<UAnimStateNodeBase>(Node)) return S->GetBoundGraph(); return nullptr; }
```

Alternative without C++ (not tried): duplicate an engine AnimBP that already has a state machine
(`/Game/Mannequin/Animations/ThirdPerson_AnimBP`, `/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed`) and edit
its states' contents and rules through MCP; you cannot add a third state that way.

## 3. Control Rig with a procedural node: PASS (MCP only)

`animation_toolset.toolsets.controlrig.ControlRigTools` (prefix `CR.` below):

| Step | Call |
|---|---|
| create | `CR.create {"path":"/Game/Smoke/CR_X"}` -> `/Game/Smoke/CR_X.CR_X` |
| bones | `CR.import_bones_from_asset {"control_rig":{"refPath":"/Game/Smoke/CR_X.CR_X"},"skeletal_mesh":{"refPath":"/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"}}` (returns the element list) |
| graph | `CR.get_forward_solve_graph {"control_rig":{...}}` -> `...CR_X:RigVMModel` |
| event | `CR.add_event_node {..., "graph":{...},"event_type":"FORWARD_SOLVE","position":{"x":0,"y":0}}` -> `...RigVMModel.RigUnit_BeginExecution` (a new rig has **no** Forwards Solve node; `list_nodes` returns `[]`) |
| node | `CR.create_node {..., "node_type":"RigUnit_OffsetTransformForItem"}` |
| pins | `CR.list_pins {"control_rig":{...},"node":{"refPath":"...RigVMModel.RigUnit_OffsetTransformForItem"}}` -> `.ExecutePin .Item .OffsetTransform .Weight .bPropagateToChildren` |
| wire | `CR.connect_pins {..., "source_pin":{"refPath":"...RigUnit_BeginExecution.ExecutePin"},"target_pin":{"refPath":"...RigUnit_OffsetTransformForItem.ExecutePin"}}` |
| values | `CR.set_pin_value ... .Item = "(Type=Bone,Name=\"spine_03\")"`; `.OffsetTransform = "(Rotation=(X=0.130526,Y=0,Z=0,W=0.991445),Translation=(X=0,Y=0,Z=0),Scale3D=(X=1,Y=1,Z=1))"` (15 deg roll); read back with `CR.get_pin_value` |
| compile, save | `BlueprintToolsets.BlueprintToolset.CompileBlueprint {"blueprint":{"refPath":"/Game/Smoke/CR_X.CR_X"}}`, `EditorToolset.AssetToolset.SaveAsset {"assetPath":"/Game/Smoke/CR_X"}` (ControlRigTools has no compile/save of its own). Python check: `type=ControlRigBlueprint`, status `BS_UP_TO_DATE`, model nodes `["Offset Transform","Forwards Solve"]` |

Pass string values through `@file.json` (nested quotes). Not tested: wiring the rig into an AnimBP
(`Animation|ControlRig|...` palette entries exist in the AnimGraph) or evaluating its effect at runtime.

## 4. IK Retargeter UE4 -> UE5: PASS through Python (no MCP toolset for retargeting)

`TOOLS.md` has no retarget or IK Rig tool (only `BlueprintToolset.RetargetNodeClass`, unrelated). One Python call did
the whole chain in 4.1 s:

```python
import unreal
at = unreal.AssetToolsHelpers.get_asset_tools()
SRC_MESH = unreal.load_asset('/Game/Mannequin/Character/Mesh/SK_Mannequin')           # UE4 mannequin
TGT_MESH = unreal.load_asset('/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple')   # UE5 Manny
ANIM = unreal.load_asset('/Game/Mannequin/Animations/ThirdPersonRun')
def make_rig(name, mesh):
    rig = at.create_asset(name, '/Game/Smoke', unreal.IKRigDefinition, unreal.IKRigDefinitionFactory())
    c = unreal.IKRigController.get_controller(rig)
    c.set_skeletal_mesh(mesh)
    c.apply_auto_generated_retarget_definition()   # True for both mannequins (21 / 29 chains)
    c.apply_auto_fbik()                             # True
    return rig
src = make_rig('IK_Src', SRC_MESH); tgt = make_rig('IK_Tgt', TGT_MESH)
rtg = at.create_asset('RTG_Src_Tgt', '/Game/Smoke', unreal.IKRetargeter, unreal.IKRetargetFactory())
rc = unreal.IKRetargeterController.get_controller(rtg)
S, T = unreal.RetargetSourceOrTarget.SOURCE, unreal.RetargetSourceOrTarget.TARGET
rc.set_ik_rig(S, src); rc.set_ik_rig(T, tgt)
rc.set_preview_mesh(S, SRC_MESH); rc.set_preview_mesh(T, TGT_MESH)
rc.add_default_ops()                                # Pelvis Motion, FK Chains, IK Chains, IK Solve, Root Motion
rc.assign_ik_rig_to_all_ops(S, src); rc.assign_ik_rig_to_all_ops(T, tgt)
rc.auto_map_chains(unreal.AutoMapChainType.FUZZY, True)
rc.auto_align_all_bones(S); rc.auto_align_all_bones(T)
out = unreal.IKRetargetBatchOperation.duplicate_and_retarget(
    [unreal.EditorAssetLibrary.find_asset_data(ANIM.get_path_name())],
    SRC_MESH, TGT_MESH, rtg, prefix='RTG_', target_path='/Game/Smoke/Retargeted')
for p in ['/Game/Smoke/IK_Src', '/Game/Smoke/IK_Tgt', '/Game/Smoke/RTG_Src_Tgt']:
    unreal.EditorAssetLibrary.save_asset(p, only_if_is_dirty=False)
unreal.EditorAssetLibrary.save_directory('/Game/Smoke/Retargeted')
[str(a.package_name) for a in out]                  # ['/Game/Smoke/Retargeted/RTG_ThirdPersonRun']
```

Check: the result's skeleton is `/Game/Characters/Mannequins/Meshes/SK_Mannequin`, 17 frames / 0.6 s like the source,
and `unreal.AnimationLibrary.get_bone_pose_for_time(anim,'thigh_l',t,False)` changes over t (the clip moves).
Quality was not judged by eye. `apply_auto_generated_retarget_definition` returns False when a skeleton matches no
known template; then chains need `add_retarget_chain` / `set_retarget_root` by hand (not exercised here).

## 6. Blueprint pawn with a capsule and a code component leaning from Tick: PASS (MCP only)

The "code-driven component" is a 20-line C++ `USceneComponent` in the runtime module (`UTJLeanComponent`, `UFUNCTION
SetLean(float Lean)` sets relative roll = clamp(Lean,-1,1) * `MaxLeanDegrees` 30). Everything else through MCP:

| Step | Call |
|---|---|
| create | `BlueprintToolset.Create {"folderPath":"/Game/Smoke","assetName":"BP_Lean","assetType":{"refPath":"/Script/Engine.Pawn"}}` |
| capsule | `EditorToolset.ActorToolset.AddComponent {"owner":{"refPath":"/Game/Smoke/BP_Lean.BP_Lean"},"componentType":{"refPath":"/Script/Engine.CapsuleComponent"},"name":"Body"}` -> `/Game/Smoke/BP_Lean.BP_Lean_C:Body_GEN_VARIABLE` |
| make it root | `ActorToolset.SetParentComponent {"component":{"refPath":"...BP_Lean_C:DefaultSceneRoot_GEN_VARIABLE"},"parent":{"refPath":"...BP_Lean_C:Body_GEN_VARIABLE"}}` (the default root is removed) |
| child components | `AddComponent {"owner":{"refPath":"...:Body_GEN_VARIABLE"},"componentType":{"refPath":"/Script/AnimSmoke.TJLeanComponent"},"name":"Lean"}`, then a `StaticMeshComponent` "Visual" with owner `...:Lean_GEN_VARIABLE` |
| visual | `EditorToolset.ObjectToolset.SetProperties {"instance":{"refPath":"...:Visual_GEN_VARIABLE"},"values":"{\"StaticMesh\":{\"refPath\":\"/Engine/BasicShapes/Cylinder.Cylinder\"},\"RelativeLocation\":{\"X\":0,\"Y\":0,\"Z\":50}}"}` |
| variables | `AddVariable LeanInput float`, `AddVariable Frame int` |
| graph | `BlueprintToolsets.BlueprintDslToolset.Write {"assetPath":"/Game/Smoke/BP_Lean","dsl":<below>}` |
| compile, save, place | `CompileBlueprint`, `AssetToolset.SaveAsset`, `LevelToolset.AddToSceneFromClass {"actorType":{"refPath":"/Game/Smoke/BP_Lean.BP_Lean_C"},...}` |
| run | `EditorAppToolset.StartPIE {}` (and `{"options":{"bSimulate":true}}` for a screenshot of the level camera), `ConsoleToolset.LogsToolset.GetLogEntries {"logCategory":"LogBlueprintUserMessages","pattern":"TJLEAN"}`, `StopPIE {}` |

```lisp
(event `self.ReceiveTick`
  (params (DeltaSeconds))
  (set `self.Frame` (+ (`self.Frame`) 1))
  (set `self.LeanInput` (`KismetMathLibrary.Sin` (* (`self.Frame`) 0.05)))
  (`TJLeanComponent.SetLean` (`self.Lean`) (`self.LeanInput`))
  (if (== (mod (`self.Frame`) 30) 0)
    (`KismetSystemLibrary.PrintString` (`KismetStringLibrary.JoinStringArray`
      (|MakeArray "TJLEAN" (`KismetStringLibrary.Conv_IntToString` (`self.Frame`))
        (`KismetStringLibrary.Conv_DoubleToString` (`self.LeanInput`))
        (`KismetStringLibrary.Conv_DoubleToString` (.roll (`SceneComponent.K2_GetComponentRotation` (`self.Visual`))))
        :!type "String")))))
```

Result: `TJLEAN 60 0.14112 4.2336`, `TJLEAN 150 0.938 28.139999`, `TJLEAN 210 -0.879696 -26.390873` (roll = 30 x
lean) in PIE and Simulate; `SceneCaptureToolset.CaptureActiveEditorView` showed the tilted cylinder.

DSL pitfalls: `PrintString` has a hidden world-context pin, so do not pass `self` first (error `[connect-failed] ...
to "PrintString".input[2] bPrintToScreen`); parse errors only give a line (`[parse-failed] [) line 15]`): bind
intermediate values instead of deep nesting; `Write` returns the canonical text and warns about kept entries (writes
upsert); `params` gets its type from the overridden event.

## 7. Niagara from a template: PASS (MCP only)

| Step | Call |
|---|---|
| templates | Python `unreal.EditorAssetLibrary.list_assets('/Niagara/DefaultAssets/Templates/Systems')`: `AttributeReaderTrails, DirectionalBurst(Lightweight), FountainLightweight, MinimalLightweight, RadialBurst, SimpleExplosion` (`NiagaraToolset_Assets.GetAssetDiscoveryInfo {}` returned `[]`) |
| create | `NiagaraToolsets.NiagaraToolset_System.CreateNiagaraSystem {"assetName":"NS_X","assetPath":"/Game/Smoke","templateSystem":{"refPath":"/Niagara/DefaultAssets/Templates/Systems/FountainLightweight.FountainLightweight"}}` |
| check | `NiagaraToolset_System.GetSystemCompileState {"system":{"refPath":"/Game/Smoke/NS_X.NS_X"}}` -> `aggregateStatus: UpToDate`, no errors |
| save | `EditorToolset.AssetToolset.SaveAsset {"assetPath":"/Game/Smoke/NS_X"}` |
| actor | `LevelToolset.AddToSceneFromClass {"actorType":{"refPath":"/Script/Niagara.NiagaraActor"},"name":"TJ_Fx",...}`; `ActorToolset.GetComponents` -> `...NiagaraActor_0.NiagaraComponent0` |
| assign | `NiagaraToolset_Component.SetSystem {"niagaraComponent":{"refPath":"...NiagaraActor_0.NiagaraComponent0"},"system":{"refPath":"/Game/Smoke/NS_X.NS_X"},"bResetExistingOverrideParameters":true}` |
| see it | `EditorAppToolset.SetCameraTransform {"translation":{"x":-700,"y":300,"z":250},"rotation":{"pitch":-12,"yaw":0,"roll":0}}`, `StartPIE {"options":{"bSimulate":true}}`, wait 2 s, `SceneCaptureToolset.SceneCaptureToolset.CaptureActiveEditorView {"mode":"Lit","options":{"width":960,"height":540}}` with `call --save-images <dir>`: sprites visible above the actor; `StopPIE` |

A particle count was not read (no tool returns live particle counts); the screenshot is the evidence.

## Pitfalls collected

1. `PythonScriptingTools.search_symbols` takes `pattern` (plus `kinds, match, max_results, accepts, returns`), not `query`.
   Faster in practice: `execute_python` with `[n for n in dir(unreal) if ...]` and `f.__doc__`.
2. Wrong parameter names are cheap to fix: the error lists the valid ones (`GetGraph` wants `blueprint, graphName`;
   `GetNodeInfos` wants `nodes`; `GetProperties` wants `properties`; `SaveAsset` wants `assetPath`).
3. `ObjectToolset` resolves a Blueprint asset to its CDO, so asset-level properties (AnimBP `TargetSkeleton`) are out of
   reach; use Python on the asset.
4. `execute_python` returns the last expression; `print` goes to the Output Log (`GetLogEntries logCategory=LogPython`).
   The script may `await asyncio.sleep(x)` to let the editor tick (used to sample a PIE world over 1.2 s).
5. PIE world from Python: `unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()`.
6. `unreal.AnimInstance` exposes no `get_current_state_name`; check states by their effect (bone motion) or log from the
   AnimBP.
7. `unreal.Engine` is not exposed as a class; the live editor engine is
   `unreal.find_object(None, '/Engine/Transient.UnrealEdEngine_0')`, and its properties take the C++ names
   (`bUseFixedFrameRate`), not snake case.

## What an agent can do alone, per skater track (`docs/HOCKEY.md` section 5)

**T-A toy figure by code.** Fully alone, and this is the path with the fewest surprises: Blueprint pawns, components,
mesh assignment, Tick logic (lean, bob, stick swing, tip-over as transforms) through `ActorToolset`,
`ObjectToolset` and the Graph DSL, with C++ components compiled by `build.ps1` or Live Coding; verification by log
lines from PIE and `CaptureActiveEditorView`. Proven here end to end in 2 minutes and 29 calls; combined with the fixed
frame rate (`PHYSICS.md`) the motion is frame-deterministic.

**T-B1 humanoid, retargeted clips.** Mostly alone. Retargeting a clip between template-shaped skeletons is one Python
script (auto chains, auto FBIK, auto map, batch retarget). The AnimBP needs the C++ state helper above (or a
duplicated engine AnimBP) because MCP cannot add states; everything else in the AnimBP is MCP. Not proven here:
importing an FBX from an outside source (Interchange import via Python exists but was not run), a skeleton that
matches no template (chains by hand), and judging retarget quality, which needs eyes on screenshots.

**T-B2 generated rigged figure (Meshy, Tripo, Rodin).** Same pipeline as T-B1 after import, with more risk: generated
rigs often use non-standard bone names, so `apply_auto_generated_retarget_definition` may return False and the agent
must define chains bone by bone (`IKRigController.add_retarget_chain`, `set_retarget_root`); nothing here exercised that. Plan for a human glance at
a screenshot sequence before promotion, and keep T-A as the fallback the brief already assumes.
