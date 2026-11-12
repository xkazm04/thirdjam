// The live Designer (docs/PLAN.md section 4): sentence -> prompt -> model -> validated cartridge.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "JamDesignerSubsystem.generated.h"

UCLASS()
class JAMDESIGNER_API UJamDesignerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// TODO(Designer lane): D0 preset list, D1 live generation (HTTP client, user-supplied key never logged or bundled),
	// schema validation against schema/cartridge.schema.json, cache, prompt assembly from prompts/. No network code yet.
	UFUNCTION(BlueprintPure, Category = "Jam|Designer")
	bool IsLiveAvailable() const { return false; }
};
