// Lane A33 (R-0028 point 3): tackling that reads. Pure-function tests of the reaction layer (the skeletal figure's hit kind and reaction curves),
// the camera punch and the puck pop. No world. Run:
//   UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Tackling; Quit" -unattended -nullrhi -nosplash -log
#include "Misc/AutomationTest.h"
#include "SkeletalSkaterVisual.h"
#include "CameraFollow.h"
#include "JamPawns.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamTacklingHitKind, "Jam.Tackling.HitKind", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamTacklingHitKind::RunTest(const FString&)
{
	using V = USkeletalSkaterVisual;
	// Facing +x (yaw 0). The push runs from the checker to the victim.
	TestEqual(TEXT("pushed along the facing: hit from behind"), (int32)V::HitKindFor(1.f, 0.f, 0.f), (int32)ESkaterHitKind::Behind);
	TestEqual(TEXT("pushed against the facing: hit from the front"), (int32)V::HitKindFor(-1.f, 0.f, 0.f), (int32)ESkaterHitKind::Front);
	TestEqual(TEXT("pushed to the right (+z): the checker came from the left"), (int32)V::HitKindFor(0.f, 1.f, 0.f), (int32)ESkaterHitKind::SideL);
	TestEqual(TEXT("pushed to the left (-z): the checker came from the right"), (int32)V::HitKindFor(0.f, -1.f, 0.f), (int32)ESkaterHitKind::SideR);
	// The sim's thresholds: 0.5 along the facing (60 degrees off the axis is still front / behind).
	TestEqual(TEXT("55 degrees off the back is behind"), (int32)V::HitKindFor(FMath::Cos(FMath::DegreesToRadians(55.f)), FMath::Sin(FMath::DegreesToRadians(55.f)), 0.f), (int32)ESkaterHitKind::Behind);
	TestEqual(TEXT("65 degrees off the back is a side"), (int32)V::HitKindFor(FMath::Cos(FMath::DegreesToRadians(65.f)), FMath::Sin(FMath::DegreesToRadians(65.f)), 0.f), (int32)ESkaterHitKind::SideL);
	// Facing -z (yaw -90): the right vector is (1, 0), so a push toward +x comes from a checker standing on the left.
	TestEqual(TEXT("facing -z, pushed +x: checker on the left"), (int32)V::HitKindFor(1.f, 0.f, -90.f), (int32)ESkaterHitKind::SideL);
	TestEqual(TEXT("facing -z, pushed -x: checker on the right"), (int32)V::HitKindFor(-1.f, 0.f, -90.f), (int32)ESkaterHitKind::SideR);
	TestEqual(TEXT("zero direction is front"), (int32)V::HitKindFor(0.f, 0.f, 0.f), (int32)ESkaterHitKind::Front);
	TestEqual(TEXT("names"), FString(V::HitKindName(ESkaterHitKind::SideL)), FString(TEXT("side_l")));

	// The layer and the reaction by kind.
	FSkelInputs In;
	In.bDown = true;
	In.SinceKnock = 0;
	In.HitKind = ESkaterHitKind::Front;
	In.HitStrength = 0.5f;
	FSkelLayer L = V::ChooseLayer(In);
	TestEqual(TEXT("front fall plays the authored fall"), (int32)L.Seq, (int32)ESkaterSeq::KnockedDown);
	TestEqual(TEXT("front fall has no tip"), V::ReactionFor(In).TipDeg, 0.f, 0.01f);
	In.SinceKnock = 20;
	In.HitStrength = 1.f;
	TestTrue(TEXT("a harder hit runs the fall faster (frame 20 x 1.15 = 23)"), FMath::IsNearlyEqual(V::ChooseLayer(In).Frame, 23.f, 0.01f));
	In.HitStrength = 0.f;
	TestTrue(TEXT("a soft hit runs it slower (frame 20 x 0.85 = 17)"), FMath::IsNearlyEqual(V::ChooseLayer(In).Frame, 17.f, 0.01f));
	In.SinceKnock = 200;
	TestEqual(TEXT("holds the last frame while down"), V::ChooseLayer(In).Frame, 39.f, 0.01f);

	In.HitKind = ESkaterHitKind::SideL;
	In.SinceKnock = 0;
	L = V::ChooseLayer(In);
	TestEqual(TEXT("side fall holds the idle pose"), (int32)L.Seq, (int32)ESkaterSeq::Idle);
	TestEqual(TEXT("side fall, frame 0: no tip yet"), V::ReactionFor(In).TipDeg, 0.f, 0.01f);
	In.SinceKnock = 14; In.HitStrength = 0.5f;
	TestTrue(TEXT("side fall tips to 88 degrees by frame 14 at strength 0.5"), V::ReactionFor(In).TipDeg >= 80.f);
	In.SinceKnock = 60;
	TestEqual(TEXT("side fall holds flat"), V::ReactionFor(In).TipDeg, 88.f, 0.01f);
	In.bDown = false; In.SinceKnock = 0;
	TestEqual(TEXT("getting up from a side fall starts flat"), V::ReactionFor(In).TipDeg, 88.f, 0.01f);
	In.SinceKnock = 20;
	TestEqual(TEXT("up after 20 frames"), V::ReactionFor(In).TipDeg, 0.f, 0.01f);
	In.HitKind = ESkaterHitKind::Behind; In.bDown = true; In.SinceKnock = 14; In.KnockHold = 3;
	TestTrue(TEXT("the hold delays the tip (frame 14 - 3 = 11 of 14)"), V::ReactionFor(In).TipDeg < 80.f && V::ReactionFor(In).TipDeg > 30.f);

	// The board lean: 12 frames, peak at frame 3, nothing while down.
	FSkelInputs B;
	B.SinceBoard = 3; B.BoardStrength = 1.f;
	TestEqual(TEXT("board lean peaks at 16 degrees at strength 1"), V::ReactionFor(B).BoardLeanDeg, 16.f, 0.01f);
	B.BoardStrength = 0.f;
	TestEqual(TEXT("board lean 10 degrees at strength 0"), V::ReactionFor(B).BoardLeanDeg, 10.f, 0.01f);
	B.SinceBoard = 12;
	TestEqual(TEXT("board lean is over at frame 12"), V::ReactionFor(B).BoardLeanDeg, 0.f, 0.01f);
	B.SinceBoard = 3; B.bDown = true;
	TestEqual(TEXT("no board lean while down"), V::ReactionFor(B).BoardLeanDeg, 0.f, 0.01f);

	// The stumble after a mistimed poke: a 6-frame jab, then 20 frames with the stick low; a clean poke has none of it.
	FSkelInputs P;
	P.bPokeClean = false; P.SincePoke = 2;
	TestEqual(TEXT("miss: the jab at rate 3"), (int32)V::ChooseLayer(P).Seq, (int32)ESkaterSeq::Poke);
	TestEqual(TEXT("miss: jab frame 6"), V::ChooseLayer(P).Frame, 6.f, 0.01f);
	TestEqual(TEXT("miss: no stick drop during the jab"), V::ReactionFor(P).StickDropCm, 0.f, 0.01f);
	P.SincePoke = V::MissPokeFrames + V::StumbleFrames / 2;
	TestEqual(TEXT("miss: stumble pose is get_up"), (int32)V::ChooseLayer(P).Seq, (int32)ESkaterSeq::GetUp);
	TestEqual(TEXT("miss: stick 25 cm low mid-stumble"), V::ReactionFor(P).StickDropCm, 25.f, 0.01f);
	TestEqual(TEXT("miss: 12 degrees forward mid-stumble"), V::ReactionFor(P).StumblePitchDeg, 12.f, 0.01f);
	P.SincePoke = V::MissPokeFrames + V::StumbleFrames;
	TestEqual(TEXT("miss: over after the stumble"), V::ReactionFor(P).StickDropCm, 0.f, 0.01f);
	TestTrue(TEXT("miss: layer back to idle after the stumble"), V::ChooseLayer(P).Seq == ESkaterSeq::Idle);
	P.bPokeClean = true; P.SincePoke = 10;
	TestEqual(TEXT("clean: the whole poke"), V::ChooseLayer(P).Frame, 10.f, 0.01f);
	TestEqual(TEXT("clean: no stumble"), V::ReactionFor(P).StickDropCm, 0.f, 0.01f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamTacklingCameraPunch, "Jam.Tackling.CameraPunch", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamTacklingCameraPunch::RunTest(const FString&)
{
	float Cm = 0.f, Roll = 0.f;
	UJamFollowCamera::PunchOffset(0, 1.f, 1.f, Cm, Roll);
	TestEqual(TEXT("frame 0 at strength 1: 2 cm"), Cm, 2.f, 0.001f);
	TestEqual(TEXT("frame 0 at strength 1: 0.6 deg"), Roll, 0.6f, 0.001f);
	UJamFollowCamera::PunchOffset(3, 1.f, -1.f, Cm, Roll);
	TestEqual(TEXT("frame 3 left: -1 cm"), Cm, -1.f, 0.001f);
	TestEqual(TEXT("frame 3 left: -0.3 deg"), Roll, -0.3f, 0.001f);
	UJamFollowCamera::PunchOffset(5, 0.f, 1.f, Cm, Roll);
	TestEqual(TEXT("frame 5 at strength 0: half amplitude floor, 1/6 left"), Cm, 2.f * 0.5f / 6.f, 0.001f);
	UJamFollowCamera::PunchOffset(6, 1.f, 1.f, Cm, Roll);
	TestEqual(TEXT("frame 6: nothing"), Cm, 0.f, 0.001f);
	TestEqual(TEXT("frame 6: no roll"), Roll, 0.f, 0.001f);
	UJamFollowCamera::PunchOffset(-1, 1.f, 1.f, Cm, Roll);
	TestEqual(TEXT("before the punch: nothing"), Cm, 0.f, 0.001f);
	// Monotone decay.
	float Prev = 1e9f;
	for (int32 I = 0; I < UJamFollowCamera::PunchFrames; ++I)
	{
		UJamFollowCamera::PunchOffset(I, 0.7f, 1.f, Cm, Roll);
		TestTrue(*FString::Printf(TEXT("frame %d decays"), I), Cm < Prev && Cm > 0.f);
		Prev = Cm;
	}
	return true;
}

// Lane A33's Jam.Tackling.PuckPop (the cosmetic hop AJamPuck::PopLiftM) is gone with R-0035 (lane A52): the drawn puck follows the sim's height (Jam.Core.LiftedShot).

#endif
