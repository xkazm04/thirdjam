// Automation tests of lane A5 that need no world: camera lag, far-net readability, toy figure pose, input map, effects binder naming.
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "CameraFollow.h"
#include "HUDFeed.h"
#include "HUDWidget.h"
#include "RadarWidget.h"
#include "ToySkaterVisual.h"
#include "InputMapping.h"
#include "EffectsBinder.h"
#include "InputAction.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamA5Test
{
	// The path of a human skater: 8 m/s, a 180 degree turn at 5 rad/s starting at frame 120, then straight again.
	struct FPath
	{
		FVector2D Pos = FVector2D(-20.0, 5.0);
		float YawDeg = 0.f;
		void Step(int32 Frame)
		{
			const float YawRate = (Frame >= 120 && Frame < 120 + 36) ? 5.f : 0.f;  // rad/s, 36 frames = 0.6 s = 172 deg
			YawDeg = FMath::UnwindDegrees(YawDeg + FMath::RadiansToDegrees(YawRate) / 60.f);
			const float R = FMath::DegreesToRadians(YawDeg);
			Pos += FVector2D(FMath::Cos(R), FMath::Sin(R)) * (8.f / 60.f);
		}
	};

	// Runs N simulation frames with a render dt and an accumulator, returns the camera pose per SIM frame.
	TArray<FTransform> RunCamera(float RenderDt, int32 SimFrames, int32 Lag)
	{
		FJamCameraSettings C;
		C.LagFrames = Lag;
		FJamCameraFilter Filter;
		FPath Path;
		TArray<FTransform> Rows;
		float Acc = 0.f;
		int32 Frame = 0;
		while (Frame < SimFrames)
		{
			Acc += RenderDt;
			while (Acc >= 1.f / 60.f - 1e-5f && Frame < SimFrames)
			{
				++Frame;
				Path.Step(Frame);
				Filter.Step(Path.Pos, Path.YawDeg, C.LagFrames);
				Rows.Add(UJamFollowCamera::ComputeThirdPersonPose(C, Filter.PivotM, Filter.YawDeg));
				Acc -= 1.f / 60.f;
			}
		}
		return Rows;
	}

	// Projects a world point (cm) through a camera transform and horizontal fov at 16:9; false when behind the camera.
	bool Project(const FTransform& Cam, float FovDeg, const FVector& World, FVector2D& OutPx, float W = 1920.f, float H = 1080.f)
	{
		const FVector L = Cam.InverseTransformPosition(World);  // x forward, y right, z up
		if (L.X <= 1.f)
		{
			return false;
		}
		const float T = FMath::Tan(FMath::DegreesToRadians(FovDeg * 0.5f));
		const float Aspect = W / H;
		const float Nx = (L.Y / L.X) / T;
		const float Ny = (L.Z / L.X) / (T / Aspect);
		OutPx = FVector2D((Nx * 0.5f + 0.5f) * W, (0.5f - Ny * 0.5f) * H);
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCameraLagFrameBased, "Jam.Camera.CameraLagFrameBased", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamCameraLagFrameBased::RunTest(const FString&)
{
	const TArray<FTransform> A = JamA5Test::RunCamera(1.f / 60.f, 400, 4);
	const TArray<FTransform> B = JamA5Test::RunCamera(1.f / 30.f, 400, 4);
	const TArray<FTransform> C = JamA5Test::RunCamera(1.f / 144.f, 400, 4);
	TestEqual(TEXT("rows at 60 Hz"), A.Num(), 400);
	TestEqual(TEXT("rows at 30 Hz"), B.Num(), 400);
	TestEqual(TEXT("rows at 144 Hz"), C.Num(), 400);
	double MaxDiff = 0.0;
	for (int32 I = 0; I < FMath::Min(A.Num(), FMath::Min(B.Num(), C.Num())); ++I)
	{
		MaxDiff = FMath::Max(MaxDiff, (double)FVector::Dist(A[I].GetLocation(), B[I].GetLocation()));
		MaxDiff = FMath::Max(MaxDiff, (double)FVector::Dist(A[I].GetLocation(), C[I].GetLocation()));
		MaxDiff = FMath::Max(MaxDiff, (double)(FMath::Abs(A[I].GetRotation().X - B[I].GetRotation().X) + FMath::Abs(A[I].GetRotation().Y - B[I].GetRotation().Y) + FMath::Abs(A[I].GetRotation().Z - B[I].GetRotation().Z) + FMath::Abs(A[I].GetRotation().W - B[I].GetRotation().W)));
	}
	TestTrue(*FString::Printf(TEXT("camera positions identical for the same sim frames at 30, 60 and 144 Hz (max diff %g)"), MaxDiff), MaxDiff == 0.0);

	// The lag really lags: during the turn the camera trails the skater by more than at lag 0, and lag 0 is rigid.
	const TArray<FTransform> Rigid = JamA5Test::RunCamera(1.f / 60.f, 400, 0);
	TestTrue(TEXT("lag differs from rigid"), FVector::Dist(Rigid[130].GetLocation(), A[130].GetLocation()) > 1.f);
	TestEqual(TEXT("alpha at lag 0 is 1"), FJamCameraFilter::AlphaForLag(0), 1.f);
	TestEqual(TEXT("alpha at lag 4"), FJamCameraFilter::AlphaForLag(4), 1.f - FMath::Exp(-0.25f), 1e-6f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCameraDerived, "Jam.Camera.Derived", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamCameraDerived::RunTest(const FString&)
{
	FJamCameraSettings C;
	TestEqual(TEXT("pitch -17.65"), UJamFollowCamera::DerivedPitchDeg(C), -17.65f, 0.01f);
	const FTransform T = UJamFollowCamera::ComputeThirdPersonPose(C, FVector2D(0, 0), 0.f);
	TestEqual(TEXT("camera x = -6 m"), (float)T.GetLocation().X, -600.f, 0.01f);
	TestEqual(TEXT("camera y = +0.7 m off axis"), (float)T.GetLocation().Y, 70.f, 0.01f);
	TestEqual(TEXT("camera z = 3.5 m"), (float)T.GetLocation().Z, 350.f, 0.01f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFarNetReadable, "Jam.Camera.FarNetReadable", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamFarNetReadable::RunTest(const FString&)
{
	FJamCameraSettings C;
	for (float DistM : { 60.f, 85.f, 110.f })
	{
		FJamPresentationFrame F;
		F.RinkLengthM = 120.f;
		const FVector Net = F.GetFarNetWorldCm();
		const FVector2D SkaterM(Net.X / 100.f - DistM, 0.f);
		const FTransform Cam = UJamFollowCamera::ComputeThirdPersonPose(C, SkaterM, 0.f);
		FVector2D Px;
		const bool bFront = JamA5Test::Project(Cam, C.FovDeg, Net, Px);
		TestTrue(*FString::Printf(TEXT("net at %.0f m is in front of the camera"), DistM), bFront);
		TestTrue(*FString::Printf(TEXT("net at %.0f m projects inside 1920x1080 (%.0f, %.0f)"), DistM, Px.X, Px.Y), Px.X >= 0 && Px.X <= 1920 && Px.Y >= 0 && Px.Y <= 1080);
		// the text equals the distance (rounded)
		F.Skaters.Add(FJamPresentationSkater{ SkaterM, 0.f, 0, true });
		TestEqual(TEXT("distance text"), FString::Printf(TEXT("%d m"), FMath::RoundToInt32(FVector2D::Distance(SkaterM, FVector2D(Net.X / 100.f, 0.f)))), FString::Printf(TEXT("%d m"), FMath::RoundToInt32(DistM)));
	}

	// The radar shows 7 entries (6 skaters and the puck) and each follows the formula to 0.01 px.
	FJamPresentationFrame F;
	F.RinkLengthM = 120.f;
	F.RinkWidthM = 60.f;
	for (int32 I = 0; I < 6; ++I)
	{
		FJamPresentationSkater S;
		S.PosM = FVector2D(-50.0 + 20.0 * I, -25.0 + 9.0 * I);
		S.TeamId = I < 3 ? 0 : 1;
		S.bHuman = I == 0;
		F.Skaters.Add(S);
	}
	F.PuckM = FVector2D(3.0, -4.0);
	TArray<FJamRadarEntry> E;
	UJamRadarWidget::BuildEntries(F, E);
	TestEqual(TEXT("radar entries"), E.Num(), 7);
	for (int32 I = 0; I < 6 && E.Num() == 7; ++I)
	{
		TestEqual(*FString::Printf(TEXT("dot %d x"), I), (float)E[I].PosPx.X, (float)((F.Skaters[I].PosM.X + 60.0) / 120.0 * 200.0), 0.01f);
		TestEqual(*FString::Printf(TEXT("dot %d y"), I), (float)E[I].PosPx.Y, (float)((F.Skaters[I].PosM.Y + 30.0) / 60.0 * 100.0), 0.01f);
	}
	if (E.Num() == 7)
	{
		TestTrue(TEXT("last entry is the puck"), E[6].bPuck);
		TestEqual(TEXT("puck x"), (float)E[6].PosPx.X, (3.f + 60.f) / 120.f * 200.f, 0.01f);
	}

	// Arena mode: both goal triggers project inside the viewport.
	FJamCameraSettings Arena;
	Arena.Mode = EJamCameraMode::Arena;
	const FTransform AC = UJamFollowCamera::ComputeArenaPose(Arena, 120.f, 60.f);
	for (float Sgn : { -1.f, 1.f })
	{
		FVector2D Px;
		const FVector Goal(Sgn * 56.f * 100.f, 0.f, 75.f);
		TestTrue(TEXT("arena: goal in front"), JamA5Test::Project(AC, Arena.FovDeg, Goal, Px));
		TestTrue(*FString::Printf(TEXT("arena: goal %.0f inside viewport (%.0f, %.0f)"), Sgn, Px.X, Px.Y), Px.X >= 0 && Px.X <= 1920 && Px.Y >= 0 && Px.Y <= 1080);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamToyPose, "Jam.Skater.ToyPose", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamToyPose::RunTest(const FString&)
{
	using T = UToySkaterVisualComponent;
	TestEqual(TEXT("shot 0"), T::ShotStickYaw(0), 0.f, 0.01f);
	TestEqual(TEXT("shot 18"), T::ShotStickYaw(18), -100.f, 0.01f);
	TestEqual(TEXT("shot 25"), T::ShotStickYaw(25), 10.f, 0.01f);
	TestEqual(TEXT("shot 34"), T::ShotStickYaw(34), 75.f, 0.01f);
	TestEqual(TEXT("shot 40"), T::ShotStickYaw(40), 75.f, 0.01f);
	TestEqual(TEXT("shot 51 is over"), T::ShotStickYaw(51), 0.f, 0.01f);
	TestEqual(TEXT("knock down 0"), T::KnockTipDeg(0, true), 0.f, 0.01f);
	TestEqual(TEXT("knock down 14"), T::KnockTipDeg(14, true), 88.f, 0.01f);
	TestEqual(TEXT("knock down 17 bounce"), T::KnockTipDeg(17, true), 80.f, 0.01f);
	TestEqual(TEXT("knock down 20"), T::KnockTipDeg(20, true), 88.f, 0.01f);
	TestEqual(TEXT("get up 0"), T::KnockTipDeg(0, false), 88.f, 0.01f);
	TestEqual(TEXT("get up 20"), T::KnockTipDeg(20, false), 0.f, 0.01f);
	TestEqual(TEXT("lean is clamped at 35"), T::LeanTarget(8.f, 5.f), 35.f, 0.01f);
	TestEqual(TEXT("lean is clamped at -35"), T::LeanTarget(8.f, -5.f), -35.f, 0.01f);
	TestEqual(TEXT("lean 2.5 * speed * turn"), T::LeanTarget(4.f, 1.f), 10.f, 0.01f);

	FToyPoseState S;
	for (int32 I = 0; I < 300; ++I)
	{
		T::AdvanceState(S, 8.f, 5.f);
	}
	TestTrue(TEXT("lean follows to the clamp"), FMath::Abs(S.Lean - 35.f) < 0.5f);

	// deterministic: same inputs, identical bytes
	FToyInputs In;
	In.Speed = 6.f; In.TurnRate = 2.f; In.FramesSinceShoot = 18; In.FramesSinceCheck = 8; In.FramesSinceKnock = 14; In.bDown = true; In.Frame = 77; In.bBoost = true;
	const FToyPose A = T::ComputePose(S, In);
	const FToyPose B = T::ComputePose(S, In);
	TestEqual(TEXT("identical bytes"), FMemory::Memcmp(&A, &B, sizeof(FToyPose)), 0);
	TestEqual(TEXT("check pitch adds 30 degrees forward"), T::CheckCurve01(8), 1.f, 0.001f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamInputMapTest, "Jam.Input.Mapping", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamInputMapTest::RunTest(const FString&)
{
	UJamInputMap* M = UJamInputMap::Get();
	TestNotNull(TEXT("map"), M);
	if (!M)
	{
		return false;
	}
	TestTrue(TEXT("W moves"), M->IsKeyMapped(M->Move, EKeys::W));
	TestTrue(TEXT("left stick moves"), M->IsKeyMapped(M->Move, EKeys::Gamepad_Left2D));
	TestTrue(TEXT("space shoots"), M->IsKeyMapped(M->Shoot, EKeys::SpaceBar));
	TestTrue(TEXT("pad A shoots"), M->IsKeyMapped(M->Shoot, EKeys::Gamepad_FaceButton_Bottom));
	TestTrue(TEXT("shift checks"), M->IsKeyMapped(M->Check, EKeys::LeftShift));
	TestTrue(TEXT("pad B checks"), M->IsKeyMapped(M->Check, EKeys::Gamepad_FaceButton_Right));
	TestTrue(TEXT("R restarts"), M->IsKeyMapped(M->Restart, EKeys::R));
	TestTrue(TEXT("Esc pauses"), M->IsKeyMapped(M->Pause, EKeys::Escape));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFxNaming, "Jam.Fx.NamingAndLogOnce", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamFxNaming::RunTest(const FString&)
{
	TestEqual(TEXT("niagara path"), UJamEffectsBinder::NiagaraPathFor(TEXT("goal")).ToString(), FString(TEXT("/JamAssets/VFX/NS_goal.NS_goal")));
	TestEqual(TEXT("sound path"), UJamEffectsBinder::SoundPathFor(TEXT("pickup_take")).ToString(), FString(TEXT("/JamAssets/Audio/MS_pickup_take.MS_pickup_take")));
	UJamEffectsBinder::ResetMissingLogForTests();
	const FSoftObjectPath P(TEXT("/JamAssets/VFX/NS_a5_test_missing.NS_a5_test_missing"));
	TestNull(TEXT("missing asset returns null"), UJamEffectsBinder::LoadOrLogOnce(P));
	TestNull(TEXT("second call still null"), UJamEffectsBinder::LoadOrLogOnce(P));
	TestEqual(TEXT("one missing path is logged once"), UJamEffectsBinder::GetMissingLoggedCount(), 1);
	UJamEffectsBinder::ResetMissingLogForTests();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudText, "Jam.Hud.Text", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudText::RunTest(const FString&)
{
	TestEqual(TEXT("take"), UJamHudWidget::FormatTake(2), FString(TEXT("TAKE 2")));
	TestEqual(TEXT("timer 3:00"), UJamHudWidget::FormatTimer(180 * 60), FString(TEXT("3:00")));
	TestEqual(TEXT("timer rounds up"), UJamHudWidget::FormatTimer(59), FString(TEXT("0:01")));
	TestEqual(TEXT("win card"), UJamHudWidget::CardFor(EJamPhase::Win), FString(TEXT("THAT'S A WRAP")));
	TestEqual(TEXT("lose card"), UJamHudWidget::CardFor(EJamPhase::Lose), FString(TEXT("RETAKE?")));
	TestTrue(TEXT("no card in play"), UJamHudWidget::CardFor(EJamPhase::Play).IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCameraInsideBoards, "Jam.Camera.InsideBoards", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamCameraInsideBoards::RunTest(const FString&)
{
	FJamCameraSettings C;
	for (float Yaw : { 0.f, 30.f, -30.f, 180.f, 90.f })
	{
		for (float X : { -58.f, -40.f, 0.f, 58.f })
		{
			const FVector2D P(X, 25.0);
			const FTransform T = UJamFollowCamera::ComputeThirdPersonPose(C, P, Yaw);
			const FVector Focus(P.X * 100.0, P.Y * 100.0, 90.0);
			const FVector Cl = UJamFollowCamera::ClampInsideBoards(Focus, T.GetLocation(), 120.f, 60.f);
			TestTrue(*FString::Printf(TEXT("inside boards x=%.0f yaw=%.0f"), X, Yaw), FMath::Abs(Cl.X) <= 5970.01 && FMath::Abs(Cl.Y) <= 2970.01);
			if (FMath::Abs(T.GetLocation().X) < 5900 && FMath::Abs(T.GetLocation().Y) < 2900)
			{
				TestTrue(TEXT("untouched when already inside"), Cl.Equals(T.GetLocation(), 0.01));
			}
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudRules, "Jam.Hud.RuleAndHints", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudRules::RunTest(const FString&)
{
	TestEqual(TEXT("first to"), UJamHudWidget::FormatTake(1, 3), FString(TEXT("TAKE 1: first to 3")));
	TestEqual(TEXT("timer rule"), UJamHudWidget::FormatTake(1, 0), FString(TEXT("TAKE 1")));
	FJamPresentationFrame F;
	F.SimFrame = 100;
	TestTrue(TEXT("hint at 100"), UJamHudWidget::ShowControlsHint(F));
	F.SimFrame = 600;
	TestFalse(TEXT("no hint at 600"), UJamHudWidget::ShowControlsHint(F));
	F.bDirectorsCallReady = true;
	TestFalse(TEXT("no call before a goal"), UJamHudWidget::ShowDirectorsCall(F));
	F.ScoreAway = 1;
	TestTrue(TEXT("call after a goal"), UJamHudWidget::ShowDirectorsCall(F));
	UJamInputMap* M = UJamInputMap::Get();
	TestTrue(TEXT("ctrl brakes"), M->IsKeyMapped(M->Brake, EKeys::LeftControl));
	TestTrue(TEXT("left trigger brakes"), M->IsKeyMapped(M->Brake, EKeys::Gamepad_LeftTriggerAxis));
	return true;
}

#endif
