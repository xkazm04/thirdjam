// Lane A219 (track F3, R-0069 point 3 and R-0070, docs/experiments/BAR.md). The owner, 15 Nov: "Crosscheck still displays blue line/rectangle in front
// of player. This should player never see" and "no aim line needed, the player animation should reflect the crosscheck movement and that should be all".
//   Jam.Hud.NothingAtAHit   the HUD's marks of the game's mechanics that lay on the ice, across the figure or beside it at a hit ship OFF: the pass line
//                           (jam.hud.PassMeter), the shot's line (jam.hud.ShotLine), the stop's line (jam.hud.StopLine), the contact line's plate
//                           (jam.hud.ContactPlate: BRACED HIT, MISSED), the swell of the ring of a carrier who won the puck by his own hit
//                           (jam.hud.SwellAtHit). The rings under skaters, the puck's ring and the bottom-left pill are not touched.
//   Jam.Hit.TakeKept        the figure's own check or cross-check take is not replaced by the reaction to a mere collision (jam.vis.HitTakeGuard,
//                           JamPawns.cpp): from the press to ten frames after the wind-up, and through a held brace; a stumble still goes out.
// The hit layer's own bars are Jam.Hit.NoBars (HitRead.cpp).
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "HAL/IConsoleManager.h"
#include "HUDWidget.h"
#include "JamPawns.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudNothingAtAHit, "Jam.Hud.NothingAtAHit", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudNothingAtAHit::RunTest(const FString&)
{
	const auto Value = [this](const TCHAR* Name) -> int32
	{
		IConsoleVariable* V = IConsoleManager::Get().FindConsoleVariable(Name);
		TestNotNull(FString::Printf(TEXT("%s exists"), Name), V);
		return V ? V->GetInt() : -1;
	};
	TestEqual(TEXT("ships: no pass line on the ice or across the figure (jam.hud.PassMeter)"), Value(TEXT("jam.hud.PassMeter")), 0);
	TestEqual(TEXT("ships: no shot line on the ice (jam.hud.ShotLine)"), Value(TEXT("jam.hud.ShotLine")), 0);
	TestEqual(TEXT("ships: no stop line across his path (jam.hud.StopLine)"), Value(TEXT("jam.hud.StopLine")), 0);
	TestEqual(TEXT("ships: no plate at a hit (jam.hud.ContactPlate)"), Value(TEXT("jam.hud.ContactPlate")), 0);
	TestEqual(TEXT("ships: no swell of the hitter's ring (jam.hud.SwellAtHit)"), Value(TEXT("jam.hud.SwellAtHit")), 0);
	// What stays: the mate's ring of a held pass, the contested puck's rings, possession's rings.
	TestEqual(TEXT("stays: the held pass's mate ring (jam.hud.ChargeLook)"), Value(TEXT("jam.hud.ChargeLook")), 1);
	TestEqual(TEXT("stays: the contested puck's rings (jam.hud.Marks)"), Value(TEXT("jam.hud.Marks")), 1);
	TestEqual(TEXT("stays: possession's rings (jam.hud.Possess)"), Value(TEXT("jam.hud.Possess")), 1);
	TestFalse(TEXT("the plate's reader is off"), UJamHudWidget::ContactPlateOn());
	TestFalse(TEXT("the swell's reader is off"), UJamHudWidget::SwellAtHitOn());
	// The grace ring: won by a hit it keeps its own size through the whole grace; won any other way it closes from 1.5 x as before.
	for (int32 Left = 30; Left >= 0; --Left)
	{
		TestEqual(FString::Printf(TEXT("a grace won by his hit, %d frames left: the ring's own size"), Left), UJamHudWidget::GraceRingScaleFor(true, false, Left, 30), 1.f);
		TestEqual(FString::Printf(TEXT("a grace won without a hit, %d frames left: the closing ring as before"), Left), UJamHudWidget::GraceRingScaleFor(false, false, Left, 30), UJamHudWidget::TakeGraceRingScale(Left, 30));
		TestEqual(FString::Printf(TEXT("the switch on, %d frames left: package H39's swell"), Left), UJamHudWidget::GraceRingScaleFor(true, true, Left, 30), UJamHudWidget::TakeGraceRingScale(Left, 30));
	}
	TestEqual(TEXT("the grace begins without a hit: 1.5 x"), UJamHudWidget::GraceRingScaleFor(false, false, 30, 30), 1.5f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHitTakeKept, "Jam.Hit.TakeKept", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHitTakeKept::RunTest(const FString&)
{
	IConsoleVariable* V = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.vis.HitTakeGuard"));
	TestNotNull(TEXT("jam.vis.HitTakeGuard exists"), V);
	TestEqual(TEXT("ships: the guard on"), V ? V->GetInt() : -1, 1);
	// A cross-check pressed at frame 1338 with a wind-up of 8: the take runs to 1338 + 8 + 10 = 1356.
	const int64 Until = 1338 + 8 + 10;
	TestTrue(TEXT("a collision one frame after the press (package H39, the drill: the reaction replaced the take): kept off"), AJamSkater::OwnHitTakeKeeps(true, 0, 1339, Until, false));
	TestTrue(TEXT("a collision four frames after the press (the hand, f395): kept off"), AJamSkater::OwnHitTakeKeeps(true, 0, 1342, Until, false));
	TestTrue(TEXT("on the take's last frame: kept off"), AJamSkater::OwnHitTakeKeeps(true, 0, Until, Until, false));
	TestFalse(TEXT("a frame after the take: the reaction goes out as before"), AJamSkater::OwnHitTakeKeeps(true, 0, Until + 1, Until, false));
	TestFalse(TEXT("no take ever thrown: the reaction goes out"), AJamSkater::OwnHitTakeKeeps(true, 0, 500, -1, false));
	TestFalse(TEXT("a stumble a hit caused is never held back (15 frames without control)"), AJamSkater::OwnHitTakeKeeps(true, 15, 1339, Until, false));
	TestTrue(TEXT("holding the brace: a collision does not drop the stick"), AJamSkater::OwnHitTakeKeeps(true, 0, 9000, Until, true));
	TestFalse(TEXT("holding the brace, hit into a stumble: the stumble goes out"), AJamSkater::OwnHitTakeKeeps(true, 12, 9000, Until, true));
	TestFalse(TEXT("the switch off: package H39"), AJamSkater::OwnHitTakeKeeps(false, 0, 1339, Until, false));
	TestFalse(TEXT("no sim frame known (no event bus): no guard"), AJamSkater::OwnHitTakeKeeps(true, 0, -1, Until, false));
	return true;
}

#endif
