// Lane A224: see ScreenMessagesOff.h.
#include "ScreenMessagesOff.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformProperties.h"
#include "Misc/AutomationTest.h"

static TAutoConsoleVariable<int32> CVarJamUiEngineScreenMessages(TEXT("jam.ui.EngineScreenMessages"), 0,
	TEXT("Lane A224: 0 (default) a packaged game of any configuration switches the engine's on-screen messages off at start (DisableAllScreenMessages; bEnableOnScreenDebugMessages false): no yellow engine line under the score; 1 = as the engine has them (main 94bb5b48). The editor is never touched."), ECVF_Default);

void UJamScreenMessagesOff::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	if (!GEngine || !ShouldSwitchOff(FPlatformProperties::RequiresCookedData(), GIsEditor, CVarJamUiEngineScreenMessages.GetValueOnGameThread())) return;
	GEngine->Exec(nullptr, TEXT("DisableAllScreenMessages"));
	GEngine->bEnableOnScreenDebugMessages = false;
	UE_LOG(LogTemp, Log, TEXT("JAMUI engine screen messages off (packaged game: DisableAllScreenMessages, bEnableOnScreenDebugMessages false; jam.ui.EngineScreenMessages 1 keeps them)"));
}

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamUiEngineMessagesOff, "Jam.Hud.EngineMessagesOff", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamUiEngineMessagesOff::RunTest(const FString&)
{
	IConsoleVariable* V = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.ui.EngineScreenMessages"));
	TestNotNull(TEXT("jam.ui.EngineScreenMessages exists"), V);
	TestEqual(TEXT("ships 0: off in a packaged game"), V ? V->GetInt() : -1, 0);
	TestTrue(TEXT("a packaged game (cooked, not the editor): switched off"), UJamScreenMessagesOff::ShouldSwitchOff(true, false, 0));
	TestFalse(TEXT("the editor: never touched"), UJamScreenMessagesOff::ShouldSwitchOff(false, true, 0));
	TestFalse(TEXT("-game on the editor binary (uncooked): not touched"), UJamScreenMessagesOff::ShouldSwitchOff(false, false, 0));
	TestFalse(TEXT("the switch at 1: as the engine has them"), UJamScreenMessagesOff::ShouldSwitchOff(true, false, 1));
	return true;
}
#endif
