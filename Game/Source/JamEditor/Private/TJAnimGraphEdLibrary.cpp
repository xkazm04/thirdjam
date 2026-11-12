// Source: tools/mcp/ANIMATION.md section 2 (verbatim body; only the module export macro and includes adapted).
#include "TJAnimGraphEdLibrary.h"

#include "Animation/AnimBlueprint.h"
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
