// Lane A227 (track U4, the last lane): five things a judge sees in the first minutes (lane A226's faults F3, F5, F6, F7, F8).
//   Jam.Hit.NoBurst         no Niagara burst at a check, cross-check, knockdown, foul or a skater's board hit (jam.fx.HitEffects 0); the goal, the shot,
//                           a pickup, an ability and a puck's board hit keep theirs; 1 gives every burst back.
//   Jam.Hud.SprintOffIce    the stamina bar is off the ice by default (jam.hud.SprintOnIce 0); the SPRINT row's spent legs: below a quarter, not sprinting.
//   Jam.Hud.EffectWords     the effect slot under the score reads SPEED  2.8 (jam.hud.EffectWords 1); 0 = speed 2.8s.
//   Jam.Hud.BoxPlate        IN THE BOX  <seconds> while he sits (jam.cam.BoxFollowsPlay 1), rounded up; nothing at 0 frames.
//   Jam.Hud.EdgeArrowOffPanel the man-down edge arrow is lifted over the controls panel (jam.hud.EdgeArrowOffPanel 1).
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "HAL/IConsoleManager.h"
#include "HUDWidget.h"
#include "EffectsBinder.h"
#include "CameraFollow.h"
#include "MenuStyle.h"

#if WITH_DEV_AUTOMATION_TESTS

static int32 A227CVar(const TCHAR* Name)
{
	IConsoleVariable* V = IConsoleManager::Get().FindConsoleVariable(Name);
	return V ? V->GetInt() : -1;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHitNoBurst, "Jam.Hit.NoBurst", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHitNoBurst::RunTest(const FString&)
{
	TestEqual(TEXT("jam.fx.HitEffects ships 0"), A227CVar(TEXT("jam.fx.HitEffects")), 0);
	FJamEventPayload P;
	FJamEventPayload Skater; Skater.Kind = FName(TEXT("skater"));
	FJamEventPayload Cross; Cross.Kind = FName(TEXT("cross_check"));
	TestTrue(TEXT("a check spawns nothing"), UJamEffectsBinder::SkipsBurst(JamEvents::Check, P, false));
	TestTrue(TEXT("a cross-check spawns nothing"), UJamEffectsBinder::SkipsBurst(JamEvents::Check, Cross, false));
	TestTrue(TEXT("a knockdown spawns nothing"), UJamEffectsBinder::SkipsBurst(JamEvents::Knockdown, P, false));
	TestTrue(TEXT("a foul spawns nothing"), UJamEffectsBinder::SkipsBurst(JamEvents::Foul, P, false));
	TestTrue(TEXT("a skater into the boards spawns nothing"), UJamEffectsBinder::SkipsBurst(JamEvents::Board, Skater, false));
	TestFalse(TEXT("the puck off the boards keeps its chips"), UJamEffectsBinder::SkipsBurst(JamEvents::Board, P, false));
	for (const FName& E : { JamEvents::Goal, JamEvents::Shot, JamEvents::PickupTake, JamEvents::PickupSpawn, JamEvents::Ability, JamEvents::Save, JamEvents::Post })
		TestFalse(*FString::Printf(TEXT("%s keeps its burst"), *E.ToString()), UJamEffectsBinder::SkipsBurst(E, P, false));
	TestFalse(TEXT("jam.fx.HitEffects 1: a check bursts as before"), UJamEffectsBinder::SkipsBurst(JamEvents::Check, P, true));
	TestFalse(TEXT("jam.fx.HitEffects 1: a knockdown bursts as before"), UJamEffectsBinder::SkipsBurst(JamEvents::Knockdown, P, true));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudSprintOffIce, "Jam.Hud.SprintOffIce", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudSprintOffIce::RunTest(const FString&)
{
	TestEqual(TEXT("jam.hud.SprintOnIce ships 0"), A227CVar(TEXT("jam.hud.SprintOnIce")), 0);
	TestTrue(TEXT("empty legs, resting: spent"), UJamHudWidget::LegsSpent(0.1f, false));
	TestFalse(TEXT("sprinting: never dimmed (the row is lit)"), UJamHudWidget::LegsSpent(0.1f, true));
	TestFalse(TEXT("a quarter back: not spent"), UJamHudWidget::LegsSpent(0.25f, false));
	TestFalse(TEXT("full: not spent"), UJamHudWidget::LegsSpent(1.f, false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudEffectWords, "Jam.Hud.EffectWords", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudEffectWords::RunTest(const FString&)
{
	TestEqual(TEXT("jam.hud.EffectWords ships 1"), A227CVar(TEXT("jam.hud.EffectWords")), 1);
	TestEqual(TEXT("the front end's case"), UJamHudWidget::EffectSlotText(FName(TEXT("speed")), 168, true), FString(TEXT("SPEED  2.8")));
	TestEqual(TEXT("underscores read as spaces"), UJamHudWidget::EffectSlotText(FName(TEXT("heavy_puck")), 60, true), FString(TEXT("HEAVY PUCK  1.0")));
	TestEqual(TEXT("0 = before"), UJamHudWidget::EffectSlotText(FName(TEXT("speed")), 168, false), FString(TEXT("speed 2.8s")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudBoxPlate, "Jam.Hud.BoxPlate", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudBoxPlate::RunTest(const FString&)
{
	TestEqual(TEXT("jam.cam.BoxFollowsPlay ships 1"), A227CVar(TEXT("jam.cam.BoxFollowsPlay")), 1);
	TestEqual(TEXT("ten seconds"), UJamHudWidget::BoxPlateText(600), FString(TEXT("IN THE BOX  10")));
	TestEqual(TEXT("rounded up"), UJamHudWidget::BoxPlateText(421), FString(TEXT("IN THE BOX  8")));
	TestEqual(TEXT("the last frame"), UJamHudWidget::BoxPlateText(1), FString(TEXT("IN THE BOX  1")));
	TestTrue(TEXT("out of the box: no plate"), UJamHudWidget::BoxPlateText(0).IsEmpty());
	TestEqual(TEXT("the camera eases over 20 frames"), UJamFollowCamera::BoxEaseFrames, 20);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudEdgeArrowOffPanel, "Jam.Hud.EdgeArrowOffPanel", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudEdgeArrowOffPanel::RunTest(const FString&)
{
	TestEqual(TEXT("jam.hud.EdgeArrowOffPanel ships 1"), A227CVar(TEXT("jam.hud.EdgeArrowOffPanel")), 1);
	// 1080 units high, the panel's reserve 24 + 300: an arrow at the left inset edge low on the screen stands over the panel.
	const float Top = FJamUiStyle::EdgePx + UJamHudWidget::ControlsPanelReserveH;
	const FVector2D Low(56.f, 980.f);
	const FVector2D Kept = UJamHudWidget::KeepOffPanel(Low, 1080.f, Top, 40.f, FJamUiStyle::EdgePx);
	TestTrue(TEXT("lifted over the panel"), Kept.Y <= 1080.f - Top - 40.f + 0.01f);
	TestEqual(TEXT("x kept"), Kept.X, Low.X);
	return true;
}

#endif
