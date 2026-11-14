// Lane A166 (track F2, evolution 7, docs/experiments/MOVEMENT-8.md): the two warnings on the ice. The parts of UJamHudWidget::UpdateMarks that need no world.
//   Jam.Hud.Marks   the rings' sizes, the breathing by the sim's frame, and the rule that keeps a mark off a downed figure.
// The rules that decide WHEN a mark shows are JamCarrierMarks.h's: Jam.Core.Movement.Marks (JamCore).
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "HUDWidget.h"
#include "JamCarrierMarks.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudMarks, "Jam.Hud.Marks", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudMarks::RunTest(const FString& Parameters)
{
	// The breathing: 1 to 1.2 and back every 20 sim frames, the same for the same frame, never outside.
	TestEqual(TEXT("frame 0: the ring's own size"), UJamHudWidget::MarkPulse(0), 1.f);
	TestEqual(TEXT("half a period on: 1.2 x"), UJamHudWidget::MarkPulse(UJamHudWidget::MarkPulseFrames / 2), 1.2f);
	TestEqual(TEXT("a period on: its own size again"), UJamHudWidget::MarkPulse(UJamHudWidget::MarkPulseFrames), 1.f);
	bool bInside = true;
	for (int64 F = -50; F < 500; ++F) { const float P = UJamHudWidget::MarkPulse(F); if (P < 1.f - 1e-4f || P > 1.2f + 1e-4f) bInside = false; }
	TestTrue(TEXT("never under 1 or over 1.2 (a restart's frames included)"), bInside);
	TestTrue(TEXT("three breaths a second: readable inside a third of a second"), UJamHudWidget::MarkPulseFrames == 20);
	// The sizes: the contested ring is wider than the carrier's (0.8 m) and the human's (1.1 m) rings; the lit stop mark is larger than the armed one.
	TestTrue(TEXT("the contested puck's ring is larger than the rings under skaters"), UJamHudWidget::ContestRingRadiusM > UJamHudWidget::YouRingRadiusM && UJamHudWidget::ContestRingRadiusM > UJamHudWidget::CarrierRingRadiusM);
	TestTrue(TEXT("the stop mark grows when it lights"), UJamHudWidget::StopMarkLitRadiusM > UJamHudWidget::StopMarkArmedRadiusM && UJamHudWidget::StopMarkArmedAlpha < 1.f);
	// The stop mark stands inside the window it lights for.
	{
		const JamCarrierMarks::FStopRule K;
		TestTrue(TEXT("the mark's 8 m is inside the 7 to 11 m window, and the armed range is outside it"), K.AheadM > K.LitNearM && K.AheadM < K.LitFarM && K.ArmFarM > K.LitFarM);
	}
	// Nothing on a downed figure.
	{
		TArray<FBox2D> Downs;
		TestFalse(TEXT("nobody down: a mark is drawn"), UJamHudWidget::MarkMeetsDown(FVector2D(100, 100), FVector2D(200, 150), Downs));
		Downs.Add(FBox2D(FVector2D(180, 120), FVector2D(300, 200)));
		TestTrue(TEXT("the mark's box meets a downed figure's: not drawn"), UJamHudWidget::MarkMeetsDown(FVector2D(100, 100), FVector2D(200, 150), Downs));
		TestTrue(TEXT("within the pad of it: not drawn"), UJamHudWidget::MarkMeetsDown(FVector2D(100, 100), FVector2D(176, 150), Downs));
		TestFalse(TEXT("clear of it by more than the pad: drawn"), UJamHudWidget::MarkMeetsDown(FVector2D(100, 100), FVector2D(170, 150), Downs));
		TestFalse(TEXT("above it: drawn"), UJamHudWidget::MarkMeetsDown(FVector2D(180, 20), FVector2D(300, 110), Downs));
		Downs.Add(FBox2D(FVector2D(0, 0), FVector2D(50, 50)));
		TestTrue(TEXT("any of several downed figures"), UJamHudWidget::MarkMeetsDown(FVector2D(40, 40), FVector2D(60, 60), Downs));
	}
	return true;
}

#endif
