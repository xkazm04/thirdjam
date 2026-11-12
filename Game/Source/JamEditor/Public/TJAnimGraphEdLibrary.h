// Editor helper from tools/mcp/ANIMATION.md section 2: adds states and transitions to an Animation Blueprint state machine,
// which no MCP tool, Python API or Graph DSL can do. Python: unreal.TJAnimGraphEdLibrary.add_state / add_transition / get_bound_graph.
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TJAnimGraphEdLibrary.generated.h"

class UAnimBlueprint;
class UEdGraph;
class UEdGraphNode;

UCLASS()
class JAMEDITOR_API UTJAnimGraphEdLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Jam|AnimGraph")
	static UEdGraphNode* AddState(UAnimBlueprint* BP, const FString& SMName, const FString& StateName, FVector2D Pos);

	UFUNCTION(BlueprintCallable, Category = "Jam|AnimGraph")
	static UEdGraphNode* AddTransition(UAnimBlueprint* BP, const FString& SMName, const FString& From, const FString& To);

	UFUNCTION(BlueprintCallable, Category = "Jam|AnimGraph")
	static UEdGraph* GetBoundGraph(UEdGraphNode* Node);
};
