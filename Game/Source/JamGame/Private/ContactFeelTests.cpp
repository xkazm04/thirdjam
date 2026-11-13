// Lane A101 (track F1, docs/experiments/CONTACT-2.md): every outcome reads. The parts of the presentation that need no world.
//   Jam.Camera.StumbleKick   a stumble kicks the camera by its ratio: 3 cm at bump_ratio to 6 cm at down_ratio, never more than a knockdown's smallest kick
//   Jam.Hud.ContactLine      the two contact lines of the HUD and which one wins
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "CameraFollow.h"
#include "HUDWidget.h"
#include "JamCartridge.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCameraStumbleKick, "Jam.Camera.StumbleKick", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamCameraStumbleKick::RunTest(const FString& Parameters)
{
	const FJamContactConfig K;   // bump 0.4, down 1
	const float Bump = static_cast<float>(K.BumpRatio), Down = static_cast<float>(K.DownRatio);
	auto Cm = [&](float Strength, float Scale)
	{
		FVector Off;
		float Roll = 0.f;
		UJamFollowCamera::KickOffset(0, Strength, FVector2D(1.0, 0.0), 0.f, Scale, Off, Roll);
		return static_cast<float>(Off.Size());
	};
	const float S = UJamFollowCamera::StumbleKickStrength;
	const float Light = Cm(S, UJamFollowCamera::StumbleKickScale(Bump, Bump, Down));
	const float Mid = Cm(S, UJamFollowCamera::StumbleKickScale(0.7f, Bump, Down));
	const float Hard = Cm(S, UJamFollowCamera::StumbleKickScale(0.999f, Bump, Down));
	const float KnockMin = Cm(0.1f, 1.f), KnockMax = Cm(1.f, 1.f);
	AddInfo(FString::Printf(TEXT("JAMKICKTABLE stumble %.2f cm at ratio %.2f, %.2f cm at 0.70, %.2f cm at 0.999; knockdown %.2f to %.2f cm"), Light, Bump, Mid, Hard, KnockMin, KnockMax));
	TestTrue(*FString::Printf(TEXT("the lightest stumble kicks 3 cm (%.2f)"), Light), FMath::IsNearlyEqual(Light, 3.f, 0.01f));
	TestTrue(*FString::Printf(TEXT("the middle of the band kicks 4.5 cm (%.2f)"), Mid), FMath::IsNearlyEqual(Mid, 4.5f, 0.01f));
	TestTrue(*FString::Printf(TEXT("the hardest stumble kicks just under 6 cm (%.2f)"), Hard), Hard > 5.9f && Hard <= 6.f);
	TestTrue(TEXT("a stumble never kicks more than a knockdown's smallest kick"), Hard <= KnockMin + 1e-3f);
	TestTrue(TEXT("a knockdown kicks 6 to 12 cm, as before"), FMath::IsNearlyEqual(KnockMin, 6.f, 0.01f) && FMath::IsNearlyEqual(KnockMax, 12.f, 0.01f));
	TestTrue(TEXT("the scale clamps outside the band"), UJamFollowCamera::StumbleKickScale(0.1f, Bump, Down) == UJamFollowCamera::StumbleKickMin && UJamFollowCamera::StumbleKickScale(3.f, Bump, Down) == UJamFollowCamera::StumbleKickMax);
	float Last = 0.f;
	for (int32 I = 40; I <= 100; ++I)
	{
		const float C = Cm(S, UJamFollowCamera::StumbleKickScale(I / 100.f, Bump, Down));
		if (C + 1e-4f < Last) { AddError(FString::Printf(TEXT("the kick shrinks at ratio %.2f"), I / 100.f)); break; }
		Last = C;
	}
	// A degenerate band (a mode that set bump_ratio = down_ratio) divides by nothing.
	TestTrue(TEXT("bump_ratio = down_ratio: a finite scale"), FMath::IsFinite(UJamFollowCamera::StumbleKickScale(1.f, 1.f, 1.f)));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudContactLine, "Jam.Hud.ContactLine", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudContactLine::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("a refused press"), UJamHudWidget::ContactLineFor(true, false), FString(TEXT("NOT WITH THE PUCK")));
	TestEqual(TEXT("the human's own bounce"), UJamHudWidget::ContactLineFor(false, true), FString(TEXT("BOUNCED OFF")));
	TestEqual(TEXT("both in one frame: the bounce (it is the one that cost the player its feet)"), UJamHudWidget::ContactLineFor(true, true), FString(TEXT("BOUNCED OFF")));
	TestTrue(TEXT("neither: no line"), UJamHudWidget::ContactLineFor(false, false).IsEmpty());
	// Lane A113: the soft hit explains itself.
	TestEqual(TEXT("the human's own bump"), UJamHudWidget::ContactLineFor(false, false, true, false, false), FString(TEXT("TOO SOFT")));
	TestEqual(TEXT("a whiff"), UJamHudWidget::ContactLineFor(false, false, false, true, false), FString(TEXT("MISSED")));
	TestEqual(TEXT("a hit on a skater inside its release grace"), UJamHudWidget::ContactLineFor(false, false, false, false, true), FString(TEXT("JUST GOT UP")));
	TestEqual(TEXT("a soft hit that bounced the human: the bounce"), UJamHudWidget::ContactLineFor(false, true, true, false, false), FString(TEXT("BOUNCED OFF")));
	TestEqual(TEXT("the grace before the soft hit before the miss"), UJamHudWidget::ContactLineFor(false, false, true, true, true), FString(TEXT("JUST GOT UP")));
	TestEqual(TEXT("the soft hit before the miss"), UJamHudWidget::ContactLineFor(false, false, true, true, false), FString(TEXT("TOO SOFT")));
	TestEqual(TEXT("a refused press before the soft hit and the miss"), UJamHudWidget::ContactLineFor(true, false, true, true, false), FString(TEXT("NOT WITH THE PUCK")));
	TestEqual(TEXT("a charge into a skater that just got up bounced the human: the grace is the reason"), UJamHudWidget::ContactLineFor(false, true, false, false, true), FString(TEXT("JUST GOT UP")));
	TestTrue(TEXT("the line is up for a second and fades over its last quarter"), UJamHudWidget::ContactLineFrames == 60 && UJamHudWidget::ContactFadeFrames == 15);
	return true;
}

#endif
