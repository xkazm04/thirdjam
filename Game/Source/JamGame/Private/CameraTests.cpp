// Automation tests of lane A5 that need no world: camera lag, far-net readability, toy figure pose, input map, effects binder naming.
// Lane A40 adds Jam.Camera.SpringCriticallyDamped, BoardsClamp, EndBoardsLift, FarNetBias; Jam.Camera.InsideBoards tests the legacy (jam.camera.Legacy 1) pull-in.
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "CameraFollow.h"
#include "CameraProofScene.h"
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
	TestEqual(TEXT("take"), UJamHudWidget::FormatTake(2), FString(TEXT("PERIOD 2")));
	TestEqual(TEXT("timer 3:00"), UJamHudWidget::FormatTimer(180 * 60), FString(TEXT("3:00")));
	TestEqual(TEXT("timer rounds up"), UJamHudWidget::FormatTimer(59), FString(TEXT("0:01")));
	TestEqual(TEXT("win card"), UJamHudWidget::CardFor(EJamPhase::Win), FString(TEXT("YOU WIN")));
	TestEqual(TEXT("lose card"), UJamHudWidget::CardFor(EJamPhase::Lose), FString(TEXT("REMATCH?")));
	TestTrue(TEXT("no card in play"), UJamHudWidget::CardFor(EJamPhase::Play).IsEmpty());
	return true;
}

// Lane A42 (HUD pass 2): the words on the ice are hockey (R-0021 point 3) and the frame rules behind them.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudPass2, "Jam.Hud.YoursFaceoffMarkers", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudPass2::RunTest(const FString&)
{
	// The faceoff ring: HOLD while the countdown runs, DROP at the drop, nothing otherwise; WON / LOST after the draw.
	TestEqual(TEXT("hold while counting"), UJamHudWidget::FaceoffCue(90, false), FString(TEXT("HOLD")));
	TestEqual(TEXT("drop at zero"), UJamHudWidget::FaceoffCue(0, true), FString(TEXT("DROP")));
	TestTrue(TEXT("no cue in play"), UJamHudWidget::FaceoffCue(0, false).IsEmpty());
	TestEqual(TEXT("won"), UJamHudWidget::DrawResultText(true), FString(TEXT("WON")));
	TestEqual(TEXT("lost"), UJamHudWidget::DrawResultText(false), FString(TEXT("LOST")));
	TestEqual(TEXT("result frames"), UJamHudWidget::DrawResultFrames, 40);
	TestEqual(TEXT("hint wording"), UJamHudWidget::FaceoffHintText(), FString(TEXT("press shoot at the drop, not before")));
	TestEqual(TEXT("hint count"), UJamHudWidget::FaceoffHintCount, 3);
	// The YOU tag: the first 5 s, then 2 s after every drop.
	TestTrue(TEXT("you at the start"), UJamHudWidget::ShowYouTag(299, -1));
	TestFalse(TEXT("gone after 5 s"), UJamHudWidget::ShowYouTag(300, -1));
	TestTrue(TEXT("you after a drop"), UJamHudWidget::ShowYouTag(1000 + 119, 1000));
	TestFalse(TEXT("gone 2 s after the drop"), UJamHudWidget::ShowYouTag(1000 + 120, 1000));
	// The ring under the human fades from 20 m to the chevron at 24 m.
	TestEqual(TEXT("ring full at 10 m"), UJamHudWidget::RingAlphaAt(10.f), 1.f);
	TestEqual(TEXT("ring half at 22 m"), UJamHudWidget::RingAlphaAt(22.f), 0.5f);
	TestEqual(TEXT("ring gone at 24 m"), UJamHudWidget::RingAlphaAt(24.f), 0.f);
	// Markers stay 48 px inside the screen edge (the own-net marker sat flush on the right edge in the eyes pass).
	const FVector2D Right = UJamHudWidget::ClampMarker(FVector2D(1900, 500), 1920, 1080, 40, 150, 30);
	TestEqual(TEXT("right edge"), static_cast<float>(Right.X), 1920.f - 48.f - 40.f);
	const FVector2D Left = UJamHudWidget::ClampMarker(FVector2D(-200, 500), 1920, 1080, 40, 150, 30);
	TestEqual(TEXT("left edge"), static_cast<float>(Left.X), 48.f + 40.f);
	const FVector2D Low = UJamHudWidget::ClampMarker(FVector2D(960, 1200), 1920, 1080, 40, 150, 30);
	TestEqual(TEXT("bottom edge"), static_cast<float>(Low.Y), 1080.f - 48.f - 30.f);
	TestEqual(TEXT("inside untouched"), UJamHudWidget::ClampMarker(FVector2D(960, 500), 1920, 1080, 40, 150, 30), FVector2D(960, 500));
	// The commentator's strip: full for the line plus 90 frames, gone 30 frames later.
	TestEqual(TEXT("strip full"), UJamHudWidget::CommentaryOpacityFor(100, 120, 100 + 120 + 90), 1.f);
	TestEqual(TEXT("strip half"), UJamHudWidget::CommentaryOpacityFor(100, 120, 100 + 120 + 90 + 15), 0.5f);
	TestEqual(TEXT("strip gone"), UJamHudWidget::CommentaryOpacityFor(100, 120, 100 + 120 + 90 + 30), 0.f);
	TestEqual(TEXT("strip never shown"), UJamHudWidget::CommentaryOpacityFor(-1000000, 0, 50), 0.f);
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

// ---- lane A40: the board clamp, the end-board lift, the far-net bias, the spring (pure functions, no world)

namespace JamA40Test
{
	FJamPresentationFrame Frame(float L, float W, float Corner, const FVector2D& PosM, float YawDeg, int32 AttackSign = 1)
	{
		FJamPresentationFrame F;
		F.RinkLengthM = L;
		F.RinkWidthM = W;
		F.CornerRadiusM = Corner;
		F.AttackSign = AttackSign;
		F.HumanIndex = 0;
		F.Skaters.Add(FJamPresentationSkater{ PosM, YawDeg, 0, true });
		return F;
	}

	// 2D camera position (metres) of a pose.
	FVector2D Cam2D(const FTransform& T) { return FVector2D(T.GetLocation().X / 100.0, T.GetLocation().Y / 100.0); }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCameraSpring, "Jam.Camera.SpringCriticallyDamped", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamCameraSpring::RunTest(const FString&)
{
	FJamSpring S;
	S.Reset(0.f);
	float Prev = 0.f;
	int32 At95 = -1;
	for (int32 I = 1; I <= 60; ++I)
	{
		S.Step(1.f, 0.25f);
		TestTrue(*FString::Printf(TEXT("frame %d monotone (%g >= %g)"), I, S.X, Prev), S.X >= Prev - 1e-6f);
		TestTrue(*FString::Printf(TEXT("frame %d never overshoots (%g)"), I, S.X), S.X <= 1.f + 1e-6f);
		if (At95 < 0 && S.X >= 0.95f) { At95 = I; }
		Prev = S.X;
	}
	TestTrue(*FString::Printf(TEXT("95 %% within 4.7 / omega frames (at %d)"), At95), At95 > 0 && At95 <= 20);
	TestTrue(TEXT("settled"), FMath::Abs(S.X - 1.f) < 1e-3f);
	FJamSpring R;
	R.Step(3.f, 0.f);
	TestEqual(TEXT("omega 0 is rigid"), R.X, 3.f);
	// frame-based: the same steps give the same bytes whatever the render rate did in between (there is no dt to pass)
	FJamSpring A, B;
	for (int32 I = 0; I < 30; ++I) { A.Step((float)(I % 7), 0.2f); B.Step((float)(I % 7), 0.2f); }
	TestTrue(TEXT("identical values"), A.X == B.X && A.V == B.V);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCameraBoardsClamp, "Jam.Camera.BoardsClamp", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamCameraBoardsClamp::RunTest(const FString&)
{
	FJamCameraSettings C;
	const float Eps = 1e-3f;

	// 1. The inset geometry: inside / outside / nearest, with the corner arcs (90 x 45, corner 3.75, margin 0.8: half extents 44.2 x 21.7, arc 2.95).
	FJamCameraRink Rink;
	Rink.LengthM = 90.f; Rink.WidthM = 45.f; Rink.CornerRadiusM = 3.75f;
	TestTrue(TEXT("centre inside"), UJamFollowCamera::IsInsideBoards(FVector2D(0, 0), Rink, 0.8f));
	TestTrue(TEXT("0.81 m from a side is inside"), UJamFollowCamera::IsInsideBoards(FVector2D(10, 21.69), Rink, 0.8f));
	TestFalse(TEXT("0.79 m from a side is outside"), UJamFollowCamera::IsInsideBoards(FVector2D(10, 21.71), Rink, 0.8f));
	TestFalse(TEXT("the rectangle corner is outside the arc"), UJamFollowCamera::IsInsideBoards(FVector2D(44.1, 21.6), Rink, 0.8f));
	TestTrue(TEXT("on the arc is inside"), UJamFollowCamera::IsInsideBoards(FVector2D(44.2 - 2.95 + 2.95 * 0.7071, 21.7 - 2.95 + 2.95 * 0.7071), Rink, 0.8f));
	const FVector2D Np = UJamFollowCamera::NearestInsideBoards(FVector2D(60, 40), Rink, 0.8f);
	TestTrue(TEXT("nearest point of a far corner lies on the arc"), UJamFollowCamera::IsInsideBoards(Np, Rink, 0.8f) && FMath::Abs(FVector2D::Distance(Np, FVector2D(44.2 - 2.95, 21.7 - 2.95)) - 2.95) < 1e-3);

	// 2. A skater 1 m from each side and in each corner, every heading: the camera's position never leaves the inset region, in every one of 60 frames.
	int32 Clamped = 0;
	const FVector2D Spots[] = { { 0, 21.5 }, { 0, -21.5 }, { 44, 0 }, { -44, 0 }, { 20, 21.5 }, { -43, 20.5 }, { 43, -20.5 }, { 41.5, 19.5 }, { -41.5, -19.5 } };
	for (const FVector2D& Spot : Spots)
	{
		for (float Yaw = -180.f; Yaw < 180.f; Yaw += 30.f)
		{
			FJamCameraFilter State;
			for (int32 F = 0; F < 60; ++F)
			{
				const FTransform T = UJamFollowCamera::StepThirdPerson(C, State, JamA40Test::Frame(90, 45, 3.75f, Spot, Yaw));
				const FVector2D P = JamA40Test::Cam2D(T);
				if (!UJamFollowCamera::IsInsideBoards(P, Rink, 0.8f - Eps))
				{
					AddError(FString::Printf(TEXT("camera outside the boards at spot (%.1f, %.1f) yaw %.0f frame %d: (%.2f, %.2f)"), Spot.X, Spot.Y, Yaw, F, P.X, P.Y));
				}
				const float Z = T.GetLocation().Z / 100.f;
				if (!(Z >= C.HeightM - Eps && Z <= C.EndHeightM + Eps)) { AddError(FString::Printf(TEXT("height %.2f outside [height_m, end_height_m]"), Z)); }
				if (State.LastClampS > 0.01f) { ++Clamped; }
			}
		}
	}
	TestTrue(*FString::Printf(TEXT("the spots exercised the clamp (%d clamped frames)"), Clamped), Clamped > 1000);

	// 3. A lap hugging the boards at 11 m/s, 1.0 m inside, weaving +-35 degrees: inside every frame, no pops (position and yaw steps bounded), the clamp active.
	FJamCameraFilter State;
	FTransform Prev;
	int32 ClampedLap = 0;
	double MaxStepM = 0.0, MaxYawStep = 0.0;
	const int32 Frames = 2400;
	for (int32 F = 0; F < Frames; ++F)
	{
		FVector2D Pos;
		float Heading;
		AJamProofScene::LapPoint(90.f, 45.f, 3.75f, 1.0f, F * (11.f / 60.f), Pos, Heading);
		const FTransform T = UJamFollowCamera::StepThirdPerson(C, State, JamA40Test::Frame(90, 45, 3.75f, Pos, FMath::UnwindDegrees(Heading + 35.f * FMath::Sin(F / 25.f))));
		const FVector2D P = JamA40Test::Cam2D(T);
		if (!UJamFollowCamera::IsInsideBoards(P, Rink, 0.8f - Eps))
		{
			AddError(FString::Printf(TEXT("lap frame %d: camera outside the boards at (%.2f, %.2f), skater (%.2f, %.2f)"), F, P.X, P.Y, Pos.X, Pos.Y));
		}
		if (F > 0)
		{
			MaxStepM = FMath::Max(MaxStepM, (double)FVector::Dist(Prev.GetLocation(), T.GetLocation()) / 100.0);
			MaxYawStep = FMath::Max(MaxYawStep, (double)FMath::Abs(FMath::UnwindDegrees(T.Rotator().Yaw - Prev.Rotator().Yaw)));
		}
		if (State.LastClampS > 0.01f) { ++ClampedLap; }
		Prev = T;
	}
	TestTrue(*FString::Printf(TEXT("lap: the clamp was active (%d of %d frames)"), ClampedLap, Frames), ClampedLap > Frames / 10);
	// The skater moves 0.18 m and 1.4 degrees a frame here; the clamp is a projection, so the camera may not move more than its desired point plus the lift: under 1 m and 6 degrees a frame.
	TestTrue(*FString::Printf(TEXT("lap: no position pop (max %.3f m per frame)"), MaxStepM), MaxStepM < 1.0);
	TestTrue(*FString::Printf(TEXT("lap: no yaw pop (max %.2f deg per frame)"), MaxYawStep), MaxYawStep < 6.0);

	// 4. Open ice is untouched: at the centre the new step equals the open-ice rig once the lag has settled (bias 0 to isolate the clamp).
	FJamCameraSettings NoBias = C;
	NoBias.FarNetBias = 0.f;
	FJamCameraFilter Open;
	FTransform OpenT;
	for (int32 F = 0; F < 60; ++F) { OpenT = UJamFollowCamera::StepThirdPerson(NoBias, Open, JamA40Test::Frame(90, 45, 3.75f, FVector2D(5, -3), 40.f)); }
	const FTransform Rig = UJamFollowCamera::ComputeThirdPersonPose(NoBias, FVector2D(5, -3), 40.f);
	TestTrue(TEXT("open ice: same position as the F1 rig"), OpenT.GetLocation().Equals(Rig.GetLocation(), 0.5));
	TestTrue(TEXT("open ice: same rotation as the F1 rig"), OpenT.Rotator().Equals(Rig.Rotator(), 0.05f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCameraEndBoards, "Jam.Camera.EndBoardsLift", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamCameraEndBoards::RunTest(const FString&)
{
	FJamCameraSettings C;
	// The skater 1.2 m from his own end board (x = -45), skating away from it (yaw 35 degrees off the rink axis), attacking +X: the far net is at (41, 0, 3).
	const FJamPresentationFrame Fr = JamA40Test::Frame(90, 45, 3.75f, FVector2D(-43.8, 3.0), 35.f);
	FJamCameraFilter State;
	FTransform T;
	bool bNetAlways = true;
	for (int32 F = 0; F < 150; ++F)
	{
		T = UJamFollowCamera::StepThirdPerson(C, State, Fr);
		if (T.GetLocation().X < -4420.0 - 0.1) { AddError(FString::Printf(TEXT("frame %d past the end face minus the margin: x = %.1f cm"), F, T.GetLocation().X)); }
		FVector2D Px;
		if (F > 30 && !(JamA5Test::Project(T, C.FovDeg, Fr.GetFarNetWorldCm(), Px) && Px.X >= 0 && Px.X <= 1920 && Px.Y >= 0 && Px.Y <= 1080)) { bNetAlways = false; }
	}
	const float Z = T.GetLocation().Z / 100.f;
	TestTrue(*FString::Printf(TEXT("lifted toward end_height_m (%.2f m; 4.5 m of the 6 m arm displaced)"), Z), Z > 4.4f && Z <= C.EndHeightM + 1e-3f);
	TestTrue(*FString::Printf(TEXT("looks along the rink (yaw %.1f, skater 35)"), T.Rotator().Yaw), FMath::Abs(T.Rotator().Yaw) < 12.f);
	TestTrue(TEXT("the far net stays in frame from frame 30 on"), bNetAlways);
	FVector2D Px;
	TestTrue(TEXT("far net projects"), JamA5Test::Project(T, C.FovDeg, Fr.GetFarNetWorldCm(), Px));
	TestTrue(*FString::Printf(TEXT("far net inside 1920x1080 (%.0f, %.0f)"), Px.X, Px.Y), Px.X >= 0 && Px.X <= 1920 && Px.Y >= 0 && Px.Y <= 1080);

	// At a side board (far from the ends) the yaw stays the skater's: the along-the-rink rule is an end-board rule (far-net bias off to isolate it).
	FJamCameraSettings NoBias = C;
	NoBias.FarNetBias = 0.f;
	FJamCameraFilter Side;
	FTransform Ts;
	for (int32 F = 0; F < 150; ++F) { Ts = UJamFollowCamera::StepThirdPerson(NoBias, Side, JamA40Test::Frame(90, 45, 3.75f, FVector2D(0.0, 21.3), -60.f)); }
	// And a skater crossing his own end zone (heading -90, along the end board) keeps his framing: no whip toward the rink axis.
	FJamCameraFilter Cross;
	FTransform Tc;
	for (int32 F = 0; F < 150; ++F) { Tc = UJamFollowCamera::StepThirdPerson(NoBias, Cross, JamA40Test::Frame(90, 45, 3.75f, FVector2D(-43.8, 5.0), -90.f)); }
	TestTrue(*FString::Printf(TEXT("crossing the end zone: yaw stays near -90 (%.1f)"), Tc.Rotator().Yaw), FMath::Abs(FMath::UnwindDegrees(Tc.Rotator().Yaw + 90.f)) < 5.f);
	TestTrue(*FString::Printf(TEXT("side board: yaw stays near the skater's -60 (%.1f)"), Ts.Rotator().Yaw), FMath::Abs(FMath::UnwindDegrees(Ts.Rotator().Yaw + 60.f)) < 8.f);
	TestTrue(TEXT("side board: inside"), Ts.GetLocation().Y <= 2170.0 + 0.1);
	TestTrue(*FString::Printf(TEXT("side board: rose above the rig height (%.2f)"), Ts.GetLocation().Z / 100.f), Ts.GetLocation().Z > 360.0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCameraFarNetBias, "Jam.Camera.FarNetBias", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamCameraFarNetBias::RunTest(const FString&)
{
	// 90 x 45, attacking +X: the net mouth is at x = 41. The carrier at (17, -12) heading +X is 26.8 m from the net, 26.6 degrees off it.
	FJamCameraSettings C;
	const FVector2D Net(41.0, 0.0);
	const float Lead = UJamFollowCamera::FarNetLeadDeg(C, FVector2D(17, -12), 0.f, Net);
	TestEqual(TEXT("lead = bias * toward * band * delta"), Lead, 0.5f * FMath::Cos(FMath::DegreesToRadians(26.565f)) * 26.565f, 0.05f);
	TestEqual(TEXT("no lead when skating away"), UJamFollowCamera::FarNetLeadDeg(C, FVector2D(17, -12), 180.f, Net), 0.f);
	TestEqual(TEXT("no lead inside the near band"), UJamFollowCamera::FarNetLeadDeg(C, FVector2D(36, -3), 0.f, Net), 0.f);
	FJamCameraSettings Zero = C;
	Zero.FarNetBias = 0.f;
	TestEqual(TEXT("bias 0 = no lead"), UJamFollowCamera::FarNetLeadDeg(Zero, FVector2D(17, -12), 0.f, Net), 0.f);

	FJamCameraFilter A, B;
	FTransform Ta, Tb;
	for (int32 F = 0; F < 120; ++F)
	{
		Ta = UJamFollowCamera::StepThirdPerson(C, A, JamA40Test::Frame(90, 45, 3.75f, FVector2D(17, -12), 0.f));
		Tb = UJamFollowCamera::StepThirdPerson(Zero, B, JamA40Test::Frame(90, 45, 3.75f, FVector2D(17, -12), 0.f));
	}
	TestEqual(TEXT("settled yaw equals the lead"), (float)Ta.Rotator().Yaw, Lead, 0.3f);
	TestEqual(TEXT("bias 0 keeps the skater's yaw"), (float)Tb.Rotator().Yaw, 0.f, 0.01f);
	FVector2D PxA, PxB;
	const FVector NetCm(4100.f, 0.f, 300.f);
	TestTrue(TEXT("net in front (bias)"), JamA5Test::Project(Ta, C.FovDeg, NetCm, PxA));
	TestTrue(TEXT("net in front (no bias)"), JamA5Test::Project(Tb, C.FovDeg, NetCm, PxB));
	TestTrue(*FString::Printf(TEXT("the lead brings the net toward the centre (%.0f px vs %.0f px from centre)"), FMath::Abs(PxA.X - 960), FMath::Abs(PxB.X - 960)), FMath::Abs(PxA.X - 960) < FMath::Abs(PxB.X - 960) - 100);
	TestTrue(TEXT("net inside the frame with the lead"), PxA.X >= 0 && PxA.X <= 1920 && PxA.Y >= 0 && PxA.Y <= 1080);
	// The slot (4 m in front of the net mouth) is in frame too.
	FVector2D PxS;
	TestTrue(TEXT("slot projects"), JamA5Test::Project(Ta, C.FovDeg, FVector(3700.f, 0.f, 50.f), PxS) && PxS.X >= 0 && PxS.X <= 1920 && PxS.Y >= 0 && PxS.Y <= 1080);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudRules, "Jam.Hud.RuleAndHints", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudRules::RunTest(const FString&)
{
	TestEqual(TEXT("first to"), UJamHudWidget::FormatTake(1, 3), FString(TEXT("PERIOD 1: first to 3")));
	TestEqual(TEXT("timer rule"), UJamHudWidget::FormatTake(1, 0), FString(TEXT("PERIOD 1")));
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
