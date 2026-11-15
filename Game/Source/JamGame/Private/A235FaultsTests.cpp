// Lane A235 (track S1): the walk of H45's faults W13, W10, W15, W6, W7, each behind a variable; the defaults and the neutral values.
//   Jam.Hit.ContactNoBurst   W13: no burst at a poke, a lift, the draw's won / lost or a shot blocked by a skater (jam.fx.HitEffects 0); 1 gives them back; a zone's blocked shot keeps its burst.
//   Jam.Hit.NoSlideTrail     W10: jam.fx.SlideTrail ships 0; the figure's NS_slide_trail is the component hidden (by asset name).
//   Jam.Hit.NoEdgeArrow      W15: jam.hit.EdgeArrow ships 0 (no man-down arrow); jam.hit.OffScreen (the radar's marks) untouched.
//   Jam.Hud.PromptWaitsForCard W6: the teaching prompt waits while a card is up (jam.hud.PromptWaitsForCard 1); 0 = before.
//   Jam.Hud.OwnLabelCaps     W11 (job 7): OWN  24 M in the one look; own 24 m under jam.hud.OneLook 0. Jam.Menu.SentencesFit W3: four sentences listed.
//   Jam.Hud.FaceoffCueInBox  W7: the faceoff's dial and hint are not drawn for a man in the box (jam.hud.FaceoffCueInBox 0); 1 = before.
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "HAL/IConsoleManager.h"
#include "HUDWidget.h"
#include "EffectsBinder.h"
#include "HitRead.h"
#include "JamPawns.h"
#include "MenuWidget.h"

#if WITH_DEV_AUTOMATION_TESTS

static int32 A235CVar(const TCHAR* Name)
{
	IConsoleVariable* V = IConsoleManager::Get().FindConsoleVariable(Name);
	return V ? V->GetInt() : -1;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHitContactNoBurst, "Jam.Hit.ContactNoBurst", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHitContactNoBurst::RunTest(const FString&)
{
	TestEqual(TEXT("jam.fx.HitEffects ships 0"), A235CVar(TEXT("jam.fx.HitEffects")), 0);
	FJamEventPayload P;
	FJamEventPayload Body; Body.Kind = FName(TEXT("skater"));
	FJamEventPayload Zone; Zone.Kind = FName(TEXT("no_shoot_1"));
	TestTrue(TEXT("a poke spawns nothing"), UJamEffectsBinder::SkipsBurst(JamEvents::Poke, P, false));
	TestTrue(TEXT("a lift spawns nothing"), UJamEffectsBinder::SkipsBurst(JamEvents::Lift, P, false));
	TestTrue(TEXT("a shot blocked by a skater spawns nothing"), UJamEffectsBinder::SkipsBurst(JamEvents::ShotBlocked, Body, false));
	FJamEventPayload Won; Won.Kind = FName(TEXT("won"));
	FJamEventPayload Lost; Lost.Kind = FName(TEXT("lost"));
	FJamEventPayload Drop; Drop.Kind = FName(TEXT("drop"));
	TestTrue(TEXT("the draw won spawns nothing"), UJamEffectsBinder::SkipsBurst(JamEvents::Faceoff, Won, false));
	TestTrue(TEXT("the draw lost spawns nothing"), UJamEffectsBinder::SkipsBurst(JamEvents::Faceoff, Lost, false));
	TestFalse(TEXT("the drop keeps its puff"), UJamEffectsBinder::SkipsBurst(JamEvents::Faceoff, Drop, false));
	TestFalse(TEXT("a shot refused in a no-shoot zone keeps its burst"), UJamEffectsBinder::SkipsBurst(JamEvents::ShotBlocked, Zone, false));
	for (const FName& E : { JamEvents::Goal, JamEvents::Shot, JamEvents::Pass, JamEvents::Save, JamEvents::Post, JamEvents::PickupTake })
		TestFalse(*FString::Printf(TEXT("%s keeps its burst"), *E.ToString()), UJamEffectsBinder::SkipsBurst(E, P, false));
	TestFalse(TEXT("1 = before: a poke bursts"), UJamEffectsBinder::SkipsBurst(JamEvents::Poke, P, true));
	TestFalse(TEXT("1 = before: a body-blocked shot bursts"), UJamEffectsBinder::SkipsBurst(JamEvents::ShotBlocked, Body, true));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHitNoSlideTrail, "Jam.Hit.NoSlideTrail", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHitNoSlideTrail::RunTest(const FString&)
{
	TestEqual(TEXT("jam.fx.SlideTrail ships 0"), A235CVar(TEXT("jam.fx.SlideTrail")), 0);
	TestTrue(TEXT("NS_slide_trail is the streak"), AJamSkater::IsSlideTrailAsset(TEXT("NS_slide_trail")));
	TestFalse(TEXT("the brake spray stays"), AJamSkater::IsSlideTrailAsset(TEXT("NS_brake")));
	TestFalse(TEXT("the blade spray stays"), AJamSkater::IsSlideTrailAsset(TEXT("NS_blade_spray")));
	TestEqual(TEXT("no figure: nothing hidden"), AJamSkater::HideSlideTrail(nullptr), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHitNoEdgeArrow, "Jam.Hit.NoEdgeArrow", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHitNoEdgeArrow::RunTest(const FString&)
{
	TestEqual(TEXT("jam.hit.EdgeArrow ships 0"), A235CVar(TEXT("jam.hit.EdgeArrow")), 0);
	TestFalse(TEXT("no man-down arrow by default"), UJamHitEdgeWidget::EdgeArrowOn());
	TestEqual(TEXT("jam.hit.OffScreen (the radar's down marks) untouched"), A235CVar(TEXT("jam.hit.OffScreen")), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudPromptWaitsForCard, "Jam.Hud.PromptWaitsForCard", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudPromptWaitsForCard::RunTest(const FString&)
{
	TestEqual(TEXT("jam.hud.PromptWaitsForCard ships 1"), A235CVar(TEXT("jam.hud.PromptWaitsForCard")), 1);
	TestTrue(TEXT("a card up: the prompt waits"), UJamHudWidget::PromptWaitsForCard(true, true));
	TestFalse(TEXT("no card: the prompt is drawn"), UJamHudWidget::PromptWaitsForCard(true, false));
	TestFalse(TEXT("0 = before: drawn under the card"), UJamHudWidget::PromptWaitsForCard(false, true));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudFaceoffCueInBox, "Jam.Hud.FaceoffCueInBox", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudFaceoffCueInBox::RunTest(const FString&)
{
	TestEqual(TEXT("jam.hud.FaceoffCueInBox ships 0"), A235CVar(TEXT("jam.hud.FaceoffCueInBox")), 0);
	TestTrue(TEXT("in the box: no dial, no hint"), UJamHudWidget::FaceoffCueHiddenInBox(false, 480));
	TestFalse(TEXT("out of the box: drawn"), UJamHudWidget::FaceoffCueHiddenInBox(false, 0));
	TestFalse(TEXT("1 = before: drawn in the box"), UJamHudWidget::FaceoffCueHiddenInBox(true, 480));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudOwnLabelCaps, "Jam.Hud.OwnLabelCaps", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudOwnLabelCaps::RunTest(const FString&)
{
	TestEqual(TEXT("the one look: capitals"), UJamHudWidget::OwnLabelText(24, true), FString(TEXT("OWN  24 M")));
	TestEqual(TEXT("jam.hud.OneLook 0: the words of before"), UJamHudWidget::OwnLabelText(24, false), FString(TEXT("own 24 m")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuSentencesFit, "Jam.Menu.SentencesFit", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamMenuSentencesFit::RunTest(const FString&)
{
	TestEqual(TEXT("MAKE YOUR OWN lists four sentences whole (the fifth was cut by the list's edge)"), UJamMenuWidget::MaxSentencesShown, 4);
	return true;
}

#endif
