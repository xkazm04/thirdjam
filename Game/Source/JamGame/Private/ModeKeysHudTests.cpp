// Lane A72 (R-0037, ask A1): rules.timer_frames reaches 36000 (ten minutes); the HUD clock reads it as 10:00.
//   UnrealEditor-Cmd ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Core.ModeKeys; Quit" -nullrhi -unattended -nopause -stdout
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "HUDWidget.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamModeKeysHudClock, "Jam.Core.ModeKeys.HudClock", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamModeKeysHudClock::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("36000 frames read 10:00"), UJamHudWidget::FormatTimer(36000), FString(TEXT("10:00")));
	TestEqual(TEXT("one frame gone still reads 10:00 (seconds round up)"), UJamHudWidget::FormatTimer(35999), FString(TEXT("10:00")));
	TestEqual(TEXT("one second gone reads 9:59"), UJamHudWidget::FormatTimer(35940), FString(TEXT("9:59")));
	TestEqual(TEXT("the old ceiling, 18000, reads 5:00"), UJamHudWidget::FormatTimer(18000), FString(TEXT("5:00")));
	return true;
}

#endif
