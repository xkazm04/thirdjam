// Lane A219 (track F3, R-0069 point 3 and R-0070, docs/experiments/BAR.md). The owner, 15 Nov: "Crosscheck still displays blue line/rectangle in front
// of player. This should player never see" and "no aim line needed, the player animation should reflect the crosscheck movement and that should be all".
//   Jam.Hud.NothingAtAHit   the HUD's marks of the game's mechanics that lay on the ice, across the figure or beside it at a hit ship OFF: the pass line
//                           (jam.hud.PassMeter), the shot's line (jam.hud.ShotLine), the stop's line (jam.hud.StopLine), the contact line's plate
//                           (jam.hud.ContactPlate: BRACED HIT, MISSED), the swell of the ring of a carrier who won the puck by his own hit
//                           (jam.hud.SwellAtHit). The rings under skaters, the puck's ring and the bottom-left pill are not touched.
//   Jam.Hit.TakeKept        the figure's own check or cross-check take is not replaced by the reaction to a mere collision (jam.vis.HitTakeGuard,
//                           JamPawns.cpp): from the press to ten frames after the wind-up, and through a held brace; a stumble still goes out.
//   Jam.Hit.HoldOneMove     wave 2: press and hold is one movement: the hold's pose is wanted from the end of the tap's wind-up while the button is
//                           still held (jam.vis.HoldEarly), dropped on the release or when the sim never braces; the braking pose is held back while
//                           the hold's pose is on (jam.vis.HoldOverBrake).
//   Jam.Hit.WhiffTake       lane A224: a miss is device B's whiff take (jam.vis.WhiffTake, JamPawns.cpp / HitRead.cpp): while it runs no
//                           collision reaction, braking pose or PlayLift of A's cuts it; a held cross-check let go on nobody is a whiff.
//   Jam.Hud.BoxedRing       lane A224: no ring under a skater who sits in the penalty box (jam.hud.BoxedRing 0).
//   Jam.Hud.ChevronsOffPanel lane A224: the net badges and their chevrons stand over the controls panel, never on it.
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHitHoldOneMove, "Jam.Hit.HoldOneMove", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHitHoldOneMove::RunTest(const FString&)
{
	const auto Value = [this](const TCHAR* Name) -> int32
	{
		IConsoleVariable* V = IConsoleManager::Get().FindConsoleVariable(Name);
		TestNotNull(FString::Printf(TEXT("%s exists"), Name), V);
		return V ? V->GetInt() : -99;
	};
	TestEqual(TEXT("ships: the early hold"), Value(TEXT("jam.vis.HoldEarly")), 1);
	TestEqual(TEXT("ships: called on the frame the wind-up ends"), Value(TEXT("jam.vis.HoldEarlyLead")), 0);
	TestEqual(TEXT("ships: the hold over the brake"), Value(TEXT("jam.vis.HoldOverBrake")), 1);
	// The hand's press of the films: f391, wind-up 8, the human's buttons hold from f394, the sim braces at f403, he releases at f490.
	const int64 Press = 391; const int32 Wu = 8, GiveUp = 20;
	int32 FirstOn = -1, Gaps = 0; bool bOn = false;
	for (int64 F = Press; F <= 490; ++F)
	{
		const int32 Held = F >= 394 ? FMath::Min(static_cast<int32>(F - 393), 10) : 0;
		const bool bWant = AJamSkater::HoldPoseWanted(true, F >= 403, Held, false, F, Press, Wu, 0, GiveUp);
		if (bWant && FirstOn < 0) FirstOn = static_cast<int32>(F);
		if (bOn && !bWant) ++Gaps;
		bOn = bWant;
	}
	TestEqual(TEXT("held through: the pose is called on the frame the wind-up ends (f399), four frames before the sim braces"), FirstOn, 399);
	TestEqual(TEXT("held through: once on it never drops before the release"), Gaps, 0);
	TestFalse(TEXT("before the wind-up ends: the tap's take alone"), AJamSkater::HoldPoseWanted(true, false, 4, false, 398, Press, Wu, 0, GiveUp));
	TestFalse(TEXT("a tap (the button released: no hold frames): no hold's pose"), AJamSkater::HoldPoseWanted(true, false, 0, false, 399, Press, Wu, 0, GiveUp));
	TestFalse(TEXT("held but the sim never braces: dropped 20 frames after the press"), AJamSkater::HoldPoseWanted(true, false, 10, false, Press + GiveUp + 1, Press, Wu, 0, GiveUp));
	TestTrue(TEXT("held, the sim not braced yet, inside the window: on"), AJamSkater::HoldPoseWanted(true, false, 10, false, Press + GiveUp, Press, Wu, 0, GiveUp));
	TestFalse(TEXT("down: no early pose"), AJamSkater::HoldPoseWanted(true, false, 5, true, 400, Press, Wu, 0, GiveUp));
	TestTrue(TEXT("the sim's brace always shows, with no press known (a hold that began without a tap)"), AJamSkater::HoldPoseWanted(true, true, 10, false, 5000, -1, 0, 0, GiveUp));
	TestFalse(TEXT("the switch off: nothing before the sim's brace (package H40)"), AJamSkater::HoldPoseWanted(false, false, 6, false, 400, Press, Wu, 0, GiveUp));
	TestTrue(TEXT("the switch off: the sim's brace as before"), AJamSkater::HoldPoseWanted(false, true, 10, false, 403, Press, Wu, 0, GiveUp));
	TestTrue(TEXT("a lead of -2: two frames before the contact frame"), AJamSkater::HoldPoseWanted(true, false, 4, false, 397, Press, Wu, -2, GiveUp));
	TestFalse(TEXT("no sim frame known: no early pose"), AJamSkater::HoldPoseWanted(true, false, 5, false, -1, Press, Wu, 0, GiveUp));
	// The brake.
	TestFalse(TEXT("braking with the hold's pose on: the braking pose is held back"), AJamSkater::BrakeLevelToSend(1, true, true));
	TestTrue(TEXT("braking without the hold: sent as before"), AJamSkater::BrakeLevelToSend(1, true, false));
	TestFalse(TEXT("not braking: nothing"), AJamSkater::BrakeLevelToSend(1, false, true));
	TestTrue(TEXT("mode 2: the brake goes out (the hold is told again after it)"), AJamSkater::BrakeLevelToSend(2, true, true));
	TestTrue(TEXT("mode 0: package H40"), AJamSkater::BrakeLevelToSend(0, true, true));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHitWhiffTake, "Jam.Hit.WhiffTake", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHitWhiffTake::RunTest(const FString&)
{
	IConsoleVariable* V = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.vis.WhiffTake"));
	TestNotNull(TEXT("jam.vis.WhiffTake exists"), V);
	TestEqual(TEXT("ships: the whiff is device B's take"), V ? V->GetInt() : -1, 1);
	TestTrue(TEXT("the reader agrees"), AJamSkater::WhiffTakeOn());
	// The takes' lengths (device B, E4 / H56.5): the cross-check's whiff frames 0 to 14; the body check's lunge recovers in 25.
	TestEqual(TEXT("cross-check whiff: 16 frames kept"), AJamSkater::WhiffTakeFrames(1), 16);
	TestEqual(TEXT("body check whiff: 25 frames kept"), AJamSkater::WhiffTakeFrames(0), 25);
	// A whiff sent at frame 1000 (cross-check): kept to 1016.
	const int64 Until = 1000 + AJamSkater::WhiffTakeFrames(1);
	TestTrue(TEXT("the frame it is sent: kept"), AJamSkater::WhiffTakeKeeps(true, 1000, Until));
	TestTrue(TEXT("its last frame: kept (no reaction, no brake, no lift over it)"), AJamSkater::WhiffTakeKeeps(true, Until, Until));
	TestFalse(TEXT("a frame after: A's calls go out as before"), AJamSkater::WhiffTakeKeeps(true, Until + 1, Until));
	TestFalse(TEXT("no whiff sent: nothing kept"), AJamSkater::WhiffTakeKeeps(true, 1000, -1));
	TestFalse(TEXT("the switch off: main 94bb5b48"), AJamSkater::WhiffTakeKeeps(false, 1000, Until));
	TestFalse(TEXT("no sim frame known: nothing kept"), AJamSkater::WhiffTakeKeeps(true, -1, Until));
	// The braking pose under the whiff: BrakeLevelToSend AND not kept (as ApplySimFrame composes them).
	TestFalse(TEXT("braking inside the whiff: the braking pose is held back"), AJamSkater::BrakeLevelToSend(1, true, false) && !AJamSkater::WhiffTakeKeeps(true, 1005, Until));
	TestTrue(TEXT("braking after it: sent"), AJamSkater::BrakeLevelToSend(1, true, false) && !AJamSkater::WhiffTakeKeeps(true, Until + 1, Until));
	// The held cross-check let go on nobody.
	TestTrue(TEXT("braced last frame, button up now, not down: a whiff"), AJamSkater::HoldReleasedOnNobody(true, false, 0, false));
	TestFalse(TEXT("still braced: no whiff"), AJamSkater::HoldReleasedOnNobody(true, true, 10, false));
	TestFalse(TEXT("the brace spent on a body (bCrossBraced drops while the button is down): no whiff"), AJamSkater::HoldReleasedOnNobody(true, false, 10, false));
	TestFalse(TEXT("never braced (a tap): no whiff here (the tap's own miss is the hit layer's)"), AJamSkater::HoldReleasedOnNobody(false, false, 0, false));
	TestFalse(TEXT("knocked down out of the hold: no whiff"), AJamSkater::HoldReleasedOnNobody(true, false, 0, true));
	TestEqual(TEXT("the release's whiff waits 3 frames for a new press (film f702 / f703: a check pressed a frame after the release wins)"), AJamSkater::HoldWhiffDelayFrames, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudBoxedRing, "Jam.Hud.BoxedRing", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudBoxedRing::RunTest(const FString&)
{
	IConsoleVariable* V = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.hud.BoxedRing"));
	TestNotNull(TEXT("jam.hud.BoxedRing exists"), V);
	TestEqual(TEXT("ships 0: no ring under a boxed skater"), V ? V->GetInt() : -1, 0);
	TestFalse(TEXT("the reader agrees"), UJamHudWidget::BoxedRingOn());
	TestFalse(TEXT("boxed (his or a bot's): no ring"), UJamHudWidget::RingOnIce(false, true));
	TestTrue(TEXT("on the ice: the ring as before"), UJamHudWidget::RingOnIce(false, false));
	TestTrue(TEXT("the switch at 1: main 94bb5b48 (the ring stayed in front of the box)"), UJamHudWidget::RingOnIce(true, true));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudChevronsOffPanel, "Jam.Hud.ChevronsOffPanel", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudChevronsOffPanel::RunTest(const FString&)
{
	IConsoleVariable* V = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.hud.ChevronsOffPanel"));
	TestNotNull(TEXT("jam.hud.ChevronsOffPanel exists"), V);
	TestEqual(TEXT("ships 1"), V ? V->GetInt() : -1, 1);
	// 1920 x 1080 layout units, the panel 240 tall at the bottom left, the edge 24.
	const float VH = 1080.f, Top = 240.f, Pad = 40.f, Edge = 24.f;
	const FVector2D OnPanel(44.f, 1016.f);   // the own-net badge clamped to the left edge, low: on the panel's box
	const FVector2D Lifted = UJamHudWidget::KeepOffPanel(OnPanel, VH, Top, Pad, Edge);
	TestTrue(TEXT("on the panel: lifted to stand over it (same x)"), FMath::IsNearlyEqual(Lifted.X, OnPanel.X));
	TestTrue(TEXT("on the panel: its top less the pad"), FMath::IsNearlyEqual(Lifted.Y, static_cast<double>(VH - Top - Pad)));
	TestTrue(TEXT("right of the panel: untouched"), FMath::IsNearlyEqual(UJamHudWidget::KeepOffPanel(FVector2D(Edge + UJamHudWidget::ControlsPanelMaxW + Pad + 1.f, 1016.f), VH, Top, Pad, Edge).Y, 1016.0));
	TestTrue(TEXT("above the panel: untouched"), FMath::IsNearlyEqual(UJamHudWidget::KeepOffPanel(FVector2D(44.f, 700.f), VH, Top, Pad, Edge).Y, 700.0));
	TestTrue(TEXT("no panel shown: untouched"), FMath::IsNearlyEqual(UJamHudWidget::KeepOffPanel(OnPanel, VH, 0.f, Pad, Edge).Y, OnPanel.Y));
	return true;
}

#endif
