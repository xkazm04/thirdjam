// Lane A224 (leftover c: lane A223 saw a yellow engine line under the score on Pond in a Development package; the trailer is recorded from a package).
// Read in the engine (UE 5.9, UnrealEngine.cpp): a SHIPPING build draws no engine screen message: UEngine::AddOnScreenDebugMessage is compiled out of
// Shipping and Test, and the map warnings ("Preparing Shaders (N)", "LIGHTING NEEDS TO BE REBUILT") are drawn only inside DrawStatsHUD's
// #if !UE_BUILD_SHIPPING block. A Development or Test package draws them. So at the start of every PACKAGED game (cooked data, not the editor) of any
// configuration, this subsystem runs the engine's own DisableAllScreenMessages and sets bEnableOnScreenDebugMessages false.
// jam.ui.EngineScreenMessages 1 = leave them as the engine has them (the editor and -game on the editor binary are never touched).
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ScreenMessagesOff.generated.h"

UCLASS()
class JAMGAME_API UJamScreenMessagesOff : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// Pure, for the test: are the engine's screen messages switched off at start? (a cooked game, not the editor, the switch at 0)
	static bool ShouldSwitchOff(bool bCooked, bool bEditor, int32 Switch) { return bCooked && !bEditor && Switch == 0; }
};
