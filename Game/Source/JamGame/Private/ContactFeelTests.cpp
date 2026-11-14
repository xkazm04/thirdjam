// Lane A101 (track F1, docs/experiments/CONTACT-2.md): every outcome reads. The parts of the presentation that need no world.
//   Jam.Camera.StumbleKick   a stumble kicks the camera by its ratio: 3 cm at bump_ratio to 6 cm at down_ratio, never more than a knockdown's smallest kick
//   Jam.Hud.ContactLine      the two contact lines of the HUD and which one wins
// Lane A132 (track F1, evolution 4, docs/experiments/CONTACT-5.md): the hit has a size.
//   Jam.Camera.DownKick      a knockdown kicks by the frames its victim lies: 6 cm at 60 frames to 12 cm at 150, never less than the hardest stumble; the human's
//                            own bounce kicks out and swings back past rest once; jam.hitkick 0 (a scale of 0) switches both off
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCameraDownKick, "Jam.Camera.DownKick", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamCameraDownKick::RunTest(const FString& Parameters)
{
	const FJamContactConfig K;   // down for 60 to 150 frames; bump 0.4, down 1
	// The offset along the push (the push is +x, the camera looks along +x) on frame FrameIndex of the kick, in cm; HitKick is jam.hitkick.
	auto Cm = [](int32 FrameIndex, float Strength, float Scale, bool bBounce, float HitKick = 1.f)
	{
		FVector Off;
		float Roll = 0.f;
		UJamFollowCamera::KickOffset(FrameIndex, Strength, FVector2D(1.0, 0.0), 0.f, Scale * HitKick, Off, Roll, bBounce);
		return static_cast<float>(Off.X);
	};
	auto Down = [&](int32 Frames) { return Cm(0, 1.f, UJamFollowCamera::DownKickScale(Frames, K.DownFramesMin, K.DownFramesMax), false); };
	const float Hard = Cm(0, UJamFollowCamera::StumbleKickStrength, UJamFollowCamera::StumbleKickScale(0.999f, static_cast<float>(K.BumpRatio), static_cast<float>(K.DownRatio)), false);
	AddInfo(FString::Printf(TEXT("JAMKICKTABLE knockdown %.2f cm at %d frames down, %.2f at 90, %.2f at 105, %.2f at 120, %.2f at %d; the hardest stumble %.2f cm"), Down(K.DownFramesMin), K.DownFramesMin, Down(90), Down(105),
		Down(120), Down(K.DownFramesMax), K.DownFramesMax, Hard));
	TestTrue(*FString::Printf(TEXT("the shortest knockdown (60 frames) kicks 6 cm (%.2f)"), Down(K.DownFramesMin)), FMath::IsNearlyEqual(Down(K.DownFramesMin), 6.f, 0.01f));
	TestTrue(*FString::Printf(TEXT("the middle (105 frames) kicks 9 cm (%.2f)"), Down(105)), FMath::IsNearlyEqual(Down(105), 9.f, 0.01f));
	TestTrue(*FString::Printf(TEXT("the longest knockdown (150 frames) kicks 12 cm (%.2f)"), Down(K.DownFramesMax)), FMath::IsNearlyEqual(Down(K.DownFramesMax), 12.f, 0.01f));
	TestTrue(TEXT("outside the band the kick is clamped (a mode's 250 frames kick 12 cm, 30 frames 6 cm)"), FMath::IsNearlyEqual(Down(250), 12.f, 0.01f) && FMath::IsNearlyEqual(Down(30), 6.f, 0.01f));
	TestTrue(TEXT("the hardest stumble never kicks more than the shortest knockdown"), Hard <= Down(K.DownFramesMin) + 1e-3f);
	float Last = 0.f;
	for (int32 F = 0; F <= 300; ++F)
	{
		const float C = Down(F);
		if (C + 1e-4f < Last) { AddError(FString::Printf(TEXT("the knockdown's kick shrinks at %d frames down"), F)); break; }
		Last = C;
	}
	TestTrue(TEXT("down_frames_min = down_frames_max: a finite scale"), FMath::IsFinite(UJamFollowCamera::DownKickScale(90, 90, 90)));
	TestTrue(TEXT("jam.hitkick 0: no kick for a knockdown"), Cm(0, 1.f, UJamFollowCamera::DownKickScale(150, K.DownFramesMin, K.DownFramesMax), false, 0.f) == 0.f);
	// The bounce: out as far as the hardest stumble, back past rest once, then still.
	FString Shape, Plain;
	int32 SignChanges = 0;
	float Deepest = 0.f, Prev = 0.f;
	for (int32 I = 0; I < UJamFollowCamera::KickFrames; ++I)
	{
		const float B = Cm(I, UJamFollowCamera::StumbleKickStrength, UJamFollowCamera::StumbleKickMax, true);
		const float P = Cm(I, UJamFollowCamera::StumbleKickStrength, UJamFollowCamera::StumbleKickMax, false);
		Shape += FString::Printf(TEXT("%s%.2f"), I ? TEXT(" ") : TEXT(""), B);
		Plain += FString::Printf(TEXT("%s%.2f"), I ? TEXT(" ") : TEXT(""), P);
		if (I > 0 && ((B < 0.f) != (Prev < 0.f))) ++SignChanges;
		if (P < 0.f) AddError(FString::Printf(TEXT("a hit's kick goes behind rest on frame %d"), I));
		Deepest = FMath::Min(Deepest, B);
		Prev = B;
	}
	AddInfo(FString::Printf(TEXT("JAMKICKTABLE bounce, cm along the push by frame: %s | a stumble of the same size: %s"), *Shape, *Plain));
	TestTrue(TEXT("the bounce starts 6 cm out, as the hardest stumble"), FMath::IsNearlyEqual(Cm(0, UJamFollowCamera::StumbleKickStrength, UJamFollowCamera::StumbleKickMax, true), 6.f, 0.01f));
	TestEqual(TEXT("the bounce crosses rest once"), SignChanges, 1);
	TestTrue(*FString::Printf(TEXT("the swing back is seen (%.2f cm behind rest) and smaller than the kick"), -Deepest), Deepest <= -1.f && Deepest >= -3.f);
	TestTrue(TEXT("the bounce is still by its last frame (under half a centimetre)"), FMath::Abs(Cm(UJamFollowCamera::KickFrames - 1, UJamFollowCamera::StumbleKickStrength, UJamFollowCamera::StumbleKickMax, true)) < 0.5f);
	TestTrue(TEXT("nothing after the last frame"), Cm(UJamFollowCamera::KickFrames, UJamFollowCamera::StumbleKickStrength, UJamFollowCamera::StumbleKickMax, true) == 0.f);
	TestTrue(TEXT("jam.hitkick 0: no kick for a bounce"), Cm(0, UJamFollowCamera::StumbleKickStrength, UJamFollowCamera::StumbleKickMax, true, 0.f) == 0.f);
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
