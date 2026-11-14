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
#include "HokejGameMode.h"
#include "BarkWidget.h"
#include "DirectorsCallWidget.h"
#include "JamSim.h"
#include "JamCartridge.h"

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
		FVector2D Px = FVector2D::ZeroVector;   // A55: read below even when Project() returns false
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
		FVector2D Px = FVector2D::ZeroVector;   // A55: read below even when Project() returns false
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
	// Lane A49: the operator's mapping (CONTROLS.md).
	TestTrue(TEXT("left mouse shoots"), M->IsKeyMapped(M->Shoot, EKeys::LeftMouseButton));
	TestTrue(TEXT("RT shoots"), M->IsKeyMapped(M->Shoot, EKeys::Gamepad_RightTriggerAxis));
	TestFalse(TEXT("space no longer shoots"), M->IsKeyMapped(M->Shoot, EKeys::SpaceBar));
	TestTrue(TEXT("right mouse checks"), M->IsKeyMapped(M->Check, EKeys::RightMouseButton));
	TestTrue(TEXT("RB checks"), M->IsKeyMapped(M->Check, EKeys::Gamepad_RightShoulder));
	TestFalse(TEXT("shift no longer checks"), M->IsKeyMapped(M->Check, EKeys::LeftShift));
	TestTrue(TEXT("shift sprints"), M->IsKeyMapped(M->Sprint, EKeys::LeftShift));
	TestTrue(TEXT("LT sprints"), M->IsKeyMapped(M->Sprint, EKeys::Gamepad_LeftTriggerAxis));
	TestTrue(TEXT("middle mouse lifts"), M->IsKeyMapped(M->Lift, EKeys::MiddleMouseButton));
	TestTrue(TEXT("mouse 4 lifts"), M->IsKeyMapped(M->Lift, EKeys::ThumbMouseButton));
	TestTrue(TEXT("LB lifts"), M->IsKeyMapped(M->Lift, EKeys::Gamepad_LeftShoulder));
	TestTrue(TEXT("space pokes"), M->IsKeyMapped(M->Poke, EKeys::SpaceBar));
	TestTrue(TEXT("pad A pokes"), M->IsKeyMapped(M->Poke, EKeys::Gamepad_FaceButton_Bottom));
	TestTrue(TEXT("wheel dekes"), M->IsKeyMapped(M->Deke, EKeys::MouseScrollUp) && M->IsKeyMapped(M->Deke, EKeys::MouseScrollDown));
	// Lane A84 (R-0041 point 3): the pass on Q / X, the ability on F / R3, Q off the deke.
	TestFalse(TEXT("Q no longer dekes"), M->IsKeyMapped(M->Deke, EKeys::Q));
	TestTrue(TEXT("pad Y dekes"), M->IsKeyMapped(M->Deke, EKeys::Gamepad_FaceButton_Top));
	TestTrue(TEXT("Q passes"), M->IsKeyMapped(M->Pass, EKeys::Q));
	TestFalse(TEXT("F no longer passes"), M->IsKeyMapped(M->Pass, EKeys::F));
	TestTrue(TEXT("F uses the ability"), M->IsKeyMapped(M->Ability, EKeys::F));
	TestTrue(TEXT("R3 uses the ability"), M->IsKeyMapped(M->Ability, EKeys::Gamepad_RightThumbstick));
	// Lane A51 (R-0034): the spin on E / B, the brake off B.
	TestFalse(TEXT("E no longer passes"), M->IsKeyMapped(M->Pass, EKeys::E));
	TestTrue(TEXT("pad X passes"), M->IsKeyMapped(M->Pass, EKeys::Gamepad_FaceButton_Left));
	TestTrue(TEXT("E spins"), M->IsKeyMapped(M->Spin, EKeys::E));
	TestTrue(TEXT("pad B spins"), M->IsKeyMapped(M->Spin, EKeys::Gamepad_FaceButton_Right));
	TestFalse(TEXT("pad B no longer brakes"), M->IsKeyMapped(M->Brake, EKeys::Gamepad_FaceButton_Right));
	TestTrue(TEXT("left stick click brakes"), M->IsKeyMapped(M->Brake, EKeys::Gamepad_LeftThumbstick));
	TestTrue(TEXT("mouse aims"), M->IsKeyMapped(M->StickMouse, EKeys::Mouse2D));
	TestTrue(TEXT("right stick aims"), M->IsKeyMapped(M->StickAim, EKeys::Gamepad_Right2D));
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
	// Lane A144 (evolution 2): the end card says what happened: the word, the score with the player's side first, the next keys.
	TestEqual(TEXT("lose card"), UJamHudWidget::CardFor(EJamPhase::Lose), FString(TEXT("BEATEN")));
	TestEqual(TEXT("win word"), UJamHudWidget::EndCardWord(EJamPhase::Win, 3, 1), FString(TEXT("YOU WIN")));
	TestEqual(TEXT("loss word"), UJamHudWidget::EndCardWord(EJamPhase::Lose, 1, 3), FString(TEXT("BEATEN")));
	TestEqual(TEXT("level at the horn is no loss"), UJamHudWidget::EndCardWord(EJamPhase::Lose, 1, 1), FString(TEXT("NO WINNER")));
	TestEqual(TEXT("no word in play"), UJamHudWidget::EndCardWord(EJamPhase::Play, 1, 1), FString());
	TestEqual(TEXT("score line, the player's side first"), UJamHudWidget::EndCardScore(1, 3), FString(TEXT("YOU  1 : 3  THEM")));
	TestEqual(TEXT("next keys"), UJamHudWidget::EndCardKeys(), FString(TEXT("R  REMATCH      ESC  MENU")));
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
				if (!(Z >= C.HeightM - Eps && Z <= FMath::Max(C.EndHeightM, C.HeightM) + Eps)) { AddError(FString::Printf(TEXT("height %.2f outside [height_m, max(height_m, end_height_m)]"), Z)); }
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
	C.EndHeightM = 5.f;   // A40's lift mechanism under test with A40's value (the A49 default of 3 m lifts nothing: the budget keeps the human framed instead)
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
	// A40 measured 4.5 m of the 6 m arm displaced; with A49's bounded yaw target the boards take a little less of the arm (3.98 m): still a lift above the rig.
	TestTrue(*FString::Printf(TEXT("lifted toward end_height_m (%.2f m; above the 3.5 m rig, at most end_height_m)"), Z), Z > 3.8f && Z <= C.EndHeightM + 1e-3f);
	AddInfo(FString::Printf(TEXT("end board yaw %.1f (skater 35; A40 looked along the rink, A49 keeps the human in the middle third)"), T.Rotator().Yaw));
	{
		const FVector2D Fh = UJamFollowCamera::FrameFraction(T, C.FovDeg, 16.f / 9.f, FVector(-43.8 * 100.0, 3.0 * 100.0, C.ChestM * 100.0));
		TestTrue(*FString::Printf(TEXT("end board: the human in the middle third (x %.3f, y %.3f)"), Fh.X, Fh.Y), FMath::Abs(Fh.X) <= 1.f / 3.f && FMath::Abs(Fh.Y) <= 1.f / 3.f);
	}
	// A49: the end-board rule now works inside the middle-third budget; the far net may leave the frame, the human may not (Jam.Camera.MiddleThird).
	AddInfo(FString::Printf(TEXT("far net in frame from frame 30 on: %s (informational since A49)"), bNetAlways ? TEXT("yes") : TEXT("no")));
	FVector2D Px = FVector2D::ZeroVector;   // A55: read below even when Project() returns false
	TestTrue(TEXT("far net projects"), JamA5Test::Project(T, C.FovDeg, Fr.GetFarNetWorldCm(), Px));
	AddInfo(FString::Printf(TEXT("far net at (%.0f, %.0f) of 1920x1080 (informational since A49)"), Px.X, Px.Y));

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
	AddInfo(FString::Printf(TEXT("side board: yaw %.1f (skater -60; informational since A49, the human's framing rules)"), Ts.Rotator().Yaw));
	{
		const FVector2D Fs = UJamFollowCamera::FrameFraction(Ts, NoBias.FovDeg, 16.f / 9.f, FVector(0.0, 21.3 * 100.0, NoBias.ChestM * 100.0));
		TestTrue(*FString::Printf(TEXT("side board: the human in the middle third (x %.3f, y %.3f)"), Fs.X, Fs.Y), FMath::Abs(Fs.X) <= 1.f / 3.f && FMath::Abs(Fs.Y) <= 1.f / 3.f);
	}
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
	TestEqual(TEXT("lead = bias * toward * band * delta"), Lead, C.FarNetBias * FMath::Cos(FMath::DegreesToRadians(26.565f)) * 26.565f, 0.05f);
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
	FVector2D PxA = FVector2D::ZeroVector, PxB = FVector2D::ZeroVector;   // A55: read below even when Project() returns false
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
	// Lane A111 (track U1): the strip belongs to the first match of a session; spent, it stays away unless the neutral value asks for every match.
	TestTrue(TEXT("hint live in the first match"), UJamHudWidget::ShowControlsHintLive(F, false, false));
	TestFalse(TEXT("no hint once spent"), UJamHudWidget::ShowControlsHintLive(F, true, false));
	TestTrue(TEXT("every match: the neutral value"), UJamHudWidget::ShowControlsHintLive(F, true, true));
	TestTrue(TEXT("the hint names the brake"), UJamHudWidget::ControlsHintText().Contains(TEXT("BRAKE")));
	// Lane A171 (REVIEW-3 item 8): the poke and the ability are in the first strip, by the keys the input map binds.
	TestTrue(TEXT("the hint names the poke and its key"), UJamHudWidget::ControlsHintText().Contains(TEXT("POKE  SPACE")));
	TestTrue(TEXT("the hint names the ability and its key"), UJamHudWidget::ControlsHintText().Contains(TEXT("ABILITY  F")));
	TestTrue(TEXT("every verb of before is still in it"), UJamHudWidget::ControlsHintText().Contains(TEXT("SKATE  WASD")) && UJamHudWidget::ControlsHintText().Contains(TEXT("AIM  MOUSE"))
		&& UJamHudWidget::ControlsHintText().Contains(TEXT("SHOOT / CROSS-CHECK  LMB")) && UJamHudWidget::ControlsHintText().Contains(TEXT("PASS / CHECK  RMB"))   // lane A183 (R-0064 point 1): the two-role buttons
		&& UJamHudWidget::ControlsHintText().Contains(TEXT("SPRINT  SHIFT")));
	TestTrue(TEXT("the hint is shorter than the ten-verb line (146 characters)"), UJamHudWidget::ControlsHintText().Len() < 146);
	TestEqual(TEXT("the hint window is 8 s"), UJamHudWidget::ControlsHintFrames, static_cast<int64>(480));
	// The marker over the human: always on; off, lane A42's fade rule.
	TestEqual(TEXT("marker on at the faceoff (ring full)"), UJamHudWidget::YouMarkerAlpha(true, 1.f), 1.f);
	TestEqual(TEXT("A42: no chevron while the ring is full"), UJamHudWidget::YouMarkerAlpha(false, 1.f), 0.f);
	TestEqual(TEXT("A42: chevron once the ring has faded"), UJamHudWidget::YouMarkerAlpha(false, 0.f), 1.f);
	// The commentator's strip: docked on the edge, over the controls strip and the call line while they show; undocked, the earlier places.
	TestEqual(TEXT("docked on the edge"), UJamHudWidget::CommentaryStripY(true, false, false), -24.f);
	TestEqual(TEXT("over the controls strip while it shows"), UJamHudWidget::CommentaryStripY(true, true, false), -150.f);
	TestEqual(TEXT("docked over the call line"), UJamHudWidget::CommentaryStripY(true, false, true), -76.f);
	TestEqual(TEXT("undocked as before"), UJamHudWidget::CommentaryStripY(false, false, true), -110.f);
	TestEqual(TEXT("undocked over the strip as before"), UJamHudWidget::CommentaryStripY(false, true, false), -150.f);
	// The net badges give way to the goal; a plate overhangs its label by the pad under any alignment.
	TestFalse(TEXT("no net badge in the goal pause"), UJamHudWidget::ShowNetBadges(EJamPhase::GoalPause));
	TestTrue(TEXT("net badge in play"), UJamHudWidget::ShowNetBadges(EJamPhase::Play));
	TestTrue(TEXT("neutral: badge in the goal pause"), UJamHudWidget::ShowNetBadges(EJamPhase::GoalPause, false));
	TestEqual(TEXT("plate of a left-aligned label"), UJamHudWidget::LabelPlatePos(FVector2D(100, 50), FVector2D(0, 0.5)), FVector2D(100 - UJamHudWidget::LabelPlatePadX, 50));
	TestEqual(TEXT("plate of a right-aligned label"), UJamHudWidget::LabelPlatePos(FVector2D(100, 50), FVector2D(1, 0.5)), FVector2D(100 + UJamHudWidget::LabelPlatePadX, 50));
	// The light hold (AHokejGameMode): released by the rig once it has been visible for three ticks, or by the 2 s bound, whichever comes first.
	TestFalse(TEXT("held while the rig loads"), AHokejGameMode::LightHoldDone(false, 0, 0.5));
	TestFalse(TEXT("held for the settle ticks"), AHokejGameMode::LightHoldDone(true, 2, 0.5));
	TestTrue(TEXT("released by the rig"), AHokejGameMode::LightHoldDone(true, 3, 0.5));
	TestTrue(TEXT("released by the bound"), AHokejGameMode::LightHoldDone(false, 0, 2.0));
	F.SimFrame = 600;
	TestFalse(TEXT("no hint at 600"), UJamHudWidget::ShowControlsHint(F));
	F.bDirectorsCallReady = true;
	TestFalse(TEXT("no call before a goal"), UJamHudWidget::ShowDirectorsCall(F));
	F.ScoreAway = 1;
	TestTrue(TEXT("call after a goal"), UJamHudWidget::ShowDirectorsCall(F));
	UJamInputMap* M = UJamInputMap::Get();
	TestTrue(TEXT("ctrl brakes"), M->IsKeyMapped(M->Brake, EKeys::LeftControl));
	TestTrue(TEXT("left stick click brakes (A51: B spins; A49: the left trigger sprints)"), M->IsKeyMapped(M->Brake, EKeys::Gamepad_LeftThumbstick));
	return true;
}

// ---- lane A49: the human never leaves the frame's middle third (the lead, the end boards, a corner), pure StepThirdPerson + FrameFraction.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCameraMiddleThird, "Jam.Camera.MiddleThird", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamCameraMiddleThird::RunTest(const FString&)
{
	FJamCameraSettings C;
	FJamCameraFilter State;
	int32 Frames = 0, Outside = 0;
	float MaxX = 0.f, MaxY = 0.f;
	auto Run = [&](FVector2D Pos, const FVector2D& VelMs, float YawDeg, int32 N)
	{
		for (int32 i = 0; i < N; ++i)
		{
			Pos += VelMs / 60.0;
			const FJamPresentationFrame F = JamA40Test::Frame(120.f, 60.f, 8.5f, Pos, YawDeg);
			const FTransform T = UJamFollowCamera::StepThirdPerson(C, State, F);
			const FVector2D Fr = UJamFollowCamera::FrameFraction(T, C.FovDeg, 16.f / 9.f, FVector(Pos.X * 100.0, Pos.Y * 100.0, C.ChestM * 100.0));
			++Frames;
			MaxX = FMath::Max(MaxX, FMath::Abs((float)Fr.X));
			MaxY = FMath::Max(MaxY, FMath::Abs((float)Fr.Y));
			if (FMath::Abs(Fr.X) > 1.f / 3.f || FMath::Abs(Fr.Y) > 1.f / 3.f) ++Outside;
		}
	};
	// Paths a skater can take (inside the boards and the 8.5 m corner arcs): across the slot toward the far net, into the end boards, along them, out of a corner.
	Run(FVector2D(10, -20), FVector2D(0, 8), 90.f, 300);     // crossing the slot sideways, 30 m from the far net: the lead wants to turn toward it
	Run(FVector2D(30, 4), FVector2D(8, 0), 0.f, 200);        // straight into the end boards (stops 0.7 m short of the glass)
	Run(FVector2D(55, -18), FVector2D(0, 8), 90.f, 270);     // along the end boards from arc to arc
	Run(FVector2D(48, 24), FVector2D(-8, 0), 180.f, 150);    // out of the corner, back up the rink
	// The same with the old defaults (lead 0.5, lift 5 m): the budget must hold there too.
	FJamCameraSettings Old = C;
	Old.FarNetBias = 0.5f;
	Old.EndHeightM = 5.f;
	FJamCameraSettings Keep = C;
	C = Old;
	State = FJamCameraFilter();
	Run(FVector2D(10, -20), FVector2D(0, 8), 90.f, 300);
	Run(FVector2D(30, 4), FVector2D(8, 0), 0.f, 200);
	Run(FVector2D(55, -18), FVector2D(0, 8), 90.f, 270);
	C = Keep;
	TestEqual(TEXT("frames with the chest outside the middle third"), Outside, 0);
	AddInfo(FString::Printf(TEXT("%d frames; max |x| %.3f, max |y| %.3f (edge 0.333)"), Frames, MaxX, MaxY));
	// Open ice: the F1 rig is untouched by the budget (the pitch is the derived pitch, the yaw the skater's).
	FJamCameraFilter S2;
	FTransform T2;
	for (int32 i = 0; i < 120; ++i) T2 = UJamFollowCamera::StepThirdPerson(Keep, S2, JamA40Test::Frame(120.f, 60.f, 8.5f, FVector2D(-10.0 + i * 0.1, 0.0), 0.f));
	TestEqual(TEXT("open ice yaw = the skater's"), (float)T2.Rotator().Yaw, 0.f, 0.05f);
	TestEqual(TEXT("open ice pitch = the derived pitch"), (float)T2.Rotator().Pitch, UJamFollowCamera::DerivedPitchDeg(Keep), 0.05f);
	return true;
}

// Lane A111 (track U1, B-to-A-0038 fault 5): the corner lift. At a board the camera looks at, more of the frame is ice; the yaw is untouched (the cursor's
// point keeps its bearing), the chest stays in the middle third, the move is eased, open ice is the rig of before, and CornerLiftM 0 is the camera of before.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCameraCornerLift, "Jam.Camera.CornerLift", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamCameraCornerLift::RunTest(const FString&)
{
	const float L = 90.f, W = 45.f, R = 8.5f, Fov = 70.f, Aspect = 16.f / 9.f;
	FJamCameraRink Rink; Rink.LengthM = L; Rink.WidthM = W; Rink.CornerRadiusM = R;
	FJamCameraSettings Off;
	TestEqual(TEXT("the settings default is off (the tests' rig, the neutral value)"), Off.CornerLiftM, 0.f);
	FJamCameraSettings On = Off;
	On.CornerLiftM = 3.f;
	// The weight: nearness times facing, the largest over the boards.
	TestEqual(TEXT("open ice"), UJamFollowCamera::CornerWeight(On, FVector2D(0, 0), 90.f, Rink), 0.f);
	TestEqual(TEXT("2 m from the side board, facing it"), UJamFollowCamera::CornerWeight(On, FVector2D(0, 20.5), 90.f, Rink), 1.f, 1e-4f);
	TestEqual(TEXT("half way in"), UJamFollowCamera::CornerWeight(On, FVector2D(0, 19.0), 90.f, Rink), 0.5f, 1e-3f);
	TestEqual(TEXT("5 m out: nothing yet"), UJamFollowCamera::CornerWeight(On, FVector2D(0, 17.5), 90.f, Rink), 0.f, 1e-4f);
	TestEqual(TEXT("along the board"), UJamFollowCamera::CornerWeight(On, FVector2D(0, 20.5), 0.f, Rink), 0.f);
	TestEqual(TEXT("away from the board"), UJamFollowCamera::CornerWeight(On, FVector2D(0, 20.5), -90.f, Rink), 0.f);
	TestEqual(TEXT("in the corner arc, facing the corner"), UJamFollowCamera::CornerWeight(On, FVector2D(41.2, 18.7), 45.f, Rink), 1.f, 1e-3f);
	TestEqual(TEXT("at the end board behind the net"), UJamFollowCamera::CornerWeight(On, FVector2D(43.5, 0), 0.f, Rink), 1.f, 1e-4f);

	auto Settle = [&](const FJamCameraSettings& C, const FVector2D& Pos, float Yaw)
	{
		FJamCameraFilter St;
		FTransform T;
		for (int32 i = 0; i < 300; ++i) T = UJamFollowCamera::StepThirdPerson(C, St, JamA40Test::Frame(L, W, R, Pos, Yaw));
		return T;
	};
	struct FScene { const TCHAR* Name; FVector2D Pos; float Yaw; };
	const FScene Scenes[] = {
		{ TEXT("side board, 3 m out, facing it"), FVector2D(5, 19.5), 90.f },
		{ TEXT("side board, 1.5 m out, facing it"), FVector2D(-12, 21.0), 90.f },
		{ TEXT("corner, facing it"), FVector2D(40.5, 18.0), 45.f },
		{ TEXT("corner, facing the end board"), FVector2D(42.5, 15.0), 0.f },
		{ TEXT("side board, facing it at 45 degrees"), FVector2D(10, 20.0), 45.f },
	};
	for (const FScene& S : Scenes)
	{
		const FTransform A = Settle(Off, S.Pos, S.Yaw), B = Settle(On, S.Pos, S.Yaw);
		const float IceA = UJamFollowCamera::IceShare(A, Fov, Aspect, Rink, 32, 18), IceB = UJamFollowCamera::IceShare(B, Fov, Aspect, Rink, 32, 18);
		const FVector Chest(S.Pos.X * 100.0, S.Pos.Y * 100.0, 100.0);
		const FVector2D Fr = UJamFollowCamera::FrameFraction(B, Fov, Aspect, Chest);
		AddInfo(FString::Printf(TEXT("JAMSCREEN corner %s: ice share %.0f %% -> %.0f %% of the frame; height %.2f -> %.2f m, pitch %.1f -> %.1f deg, yaw %.1f -> %.1f; chest at (%.2f, %.2f)"),
			S.Name, IceA * 100.f, IceB * 100.f, A.GetLocation().Z / 100.0, B.GetLocation().Z / 100.0, A.Rotator().Pitch, B.Rotator().Pitch, A.Rotator().Yaw, B.Rotator().Yaw, Fr.X, Fr.Y));
		TestTrue(FString::Printf(TEXT("%s: at least 10 points more of the frame is ice"), S.Name), IceB >= IceA + 0.10f);
		TestEqual(FString::Printf(TEXT("%s: the yaw is the yaw of before"), S.Name), (float)B.Rotator().Yaw, (float)A.Rotator().Yaw, 0.01f);
		TestTrue(FString::Printf(TEXT("%s: the chest stays in the middle third"), S.Name), FMath::Abs(Fr.X) <= 1.f / 3.f + 1e-3f && FMath::Abs(Fr.Y) <= 1.f / 3.f + 1e-3f);
	}
	// Open ice and along the boards: the pose of before, to the bit.
	for (const FScene& S : { FScene{ TEXT("centre ice"), FVector2D(0, 0), 30.f }, FScene{ TEXT("along the side board"), FVector2D(0, 20.5), 0.f }, FScene{ TEXT("leaving the corner"), FVector2D(40.5, 18.0), -135.f } })
	{
		const FTransform A = Settle(Off, S.Pos, S.Yaw), B = Settle(On, S.Pos, S.Yaw);
		TestTrue(FString::Printf(TEXT("%s: untouched"), S.Name), A.Equals(B, 1e-4));
	}
	// Eased: a skater runs at the side board at 9 m/s and stops 1 m from it; no frame moves the camera's height by more than 12 cm or its pitch by more than 1.2 degrees.
	{
		FJamCameraFilter St;
		FTransform Prev;
		float MaxDz = 0.f, MaxDp = 0.f, EndZ = 0.f;
		for (int32 i = 0; i < 360; ++i)
		{
			const double Y = FMath::Min(8.0 + i * 0.15, 21.5);
			const FTransform T = UJamFollowCamera::StepThirdPerson(On, St, JamA40Test::Frame(L, W, R, FVector2D(0, Y), 90.f));
			if (i > 0)
			{
				MaxDz = FMath::Max(MaxDz, FMath::Abs((float)(T.GetLocation().Z - Prev.GetLocation().Z)) / 100.f);
				MaxDp = FMath::Max(MaxDp, FMath::Abs((float)(T.Rotator().Pitch - Prev.Rotator().Pitch)));
			}
			Prev = T;
			EndZ = (float)T.GetLocation().Z / 100.f;
		}
		AddInfo(FString::Printf(TEXT("JAMSCREEN corner ease: running at the board at 9 m/s: largest step %.3f m of height, %.2f deg of pitch per frame; settles at %.2f m"), MaxDz, MaxDp, EndZ));
		TestTrue(TEXT("no snap in height"), MaxDz <= 0.12f);
		TestTrue(TEXT("no snap in pitch"), MaxDp <= 1.2f);
		TestEqual(TEXT("settles at the height plus the lift"), EndZ, On.HeightM + On.CornerLiftM, 0.02f);
	}
	return true;
}

// ---- lane A135 (track U1, evolution 3): the camera never loses the player. A modelled skater works the boards of both rinks for 6,000 frames each (runs at
// the glass, turns his back to it, stops, spins, goes into the corners, is knocked sideways, is put back on a dot); with FJamCameraSettings::KeepHumanFrac the
// chest is inside the frame's middle third on every frame, the camera stays inside the boards' inset, the yaw is the yaw of before wherever the rig of before
// held him (and in every settled pose), no frame snaps, and open ice is the rig of before to the bit. Pure StepThirdPerson + FrameFraction, no world.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCameraKeepsHuman, "Jam.Camera.KeepsHuman", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamCameraKeepsHuman::RunTest(const FString&)
{
	const float Fov = 70.f, Aspect = 16.f / 9.f;
	TestEqual(TEXT("the settings default is off (the tests' rig, the neutral value)"), FJamCameraSettings().KeepHumanFrac, 0.f);
	FJamCameraSettings Off;
	Off.CornerLiftM = 3.f;   // the game camera of before this lane: the corner lift on
	FJamCameraSettings On = Off;
	On.KeepHumanFrac = 0.31f;   // the game's value (jam.camera.KeepHuman)
	TestEqual(TEXT("the hold never turns the camera (evolution 4's default)"), On.KeepNoYaw, 2);
	FJamCameraSettings Ev3 = On;
	Ev3.KeepNoYaw = 0;          // evolution 3's hold: one slide, then the yaw (jam.camera.KeepNoYaw 0)

	struct FRun
	{
		int32 Frames = 0, OutThird = 0, OutFrame = 0, OutsideBoards = 0;
		float MaxX = 0.f, MaxY = 0.f, MaxStepCm = 0.f, MaxPitchStep = 0.f, MaxYawStep = 0.f, MaxH = 0.f;
		int32 Over55 = 0, Over60 = 0, Over70 = 0, Near5 = 0;   // lane A158: frames looking down steeper than 55.5 / 60 / 70 degrees, frames nearer than 4.99 m to the chest
		float MaxDown = 0.f, MinDistM = 1e9f;
		TArray<float> Yaw;
		TArray<bool> In30;   // the chest inside 0.31 of the half frame on this frame (the stage's own fraction)
	};
	// The same path for every camera: the skater reads nothing of the camera.
	auto Skate = [&](float L, float W, float R, int32 Seed, const FJamCameraSettings& C) -> FRun
	{
		FRun Out;
		FRandomStream Rng(Seed);
		FJamCameraRink Rink; Rink.LengthM = L; Rink.WidthM = W; Rink.CornerRadiusM = R;
		FJamCameraFilter St;
		FVector2D Pos(-7.5, 0.0), Vel(0.0, 0.0), Target(0.0, 0.0);
		float Yaw = 0.f, WantYaw = 0.f, WantSpeed = 0.f;
		int32 Hold = 0, Mode = 0;
		FTransform Prev;
		bool bPrev = false;
		for (int32 i = 0; i < 6000; ++i)
		{
			bool bTeleport = false;
			if (i > 0 && i % 1500 == 0) { Pos = FVector2D(-7.5, 0.0); Vel = FVector2D::ZeroVector; Yaw = 0.f; Hold = 0; bTeleport = true; }   // a faceoff after a goal
			if (--Hold <= 0)
			{
				Mode = Rng.RandRange(0, 9);
				Hold = Rng.RandRange(30, 150);
				const float Side = Rng.FRand() < 0.5f ? -1.f : 1.f;
				if (Mode <= 2) Target = FVector2D(Rng.FRandRange(-L * 0.5f, L * 0.5f), Side * W * 0.5f);            // run at a side board
				else if (Mode <= 4) Target = FVector2D(Side * L * 0.5f, (Rng.FRand() < 0.5f ? -1.f : 1.f) * W * 0.5f);   // into a corner
				else if (Mode == 5) Target = FVector2D(Side * L * 0.5f, Rng.FRandRange(-W * 0.5f, W * 0.5f));           // at an end board
				else if (Mode <= 7) Target = FVector2D(0.0, 0.0);                                                    // turn the back to the glass: face centre ice and stop
				else if (Mode == 8) { WantYaw = Yaw + 180.f; }                                                          // spin where he stands
				else { const float A = Rng.FRandRange(0.f, 2.f * PI); Vel += FVector2D(FMath::Cos(A), FMath::Sin(A)) * 6.0; }   // knocked sideways: he slides, facing as he was
				WantSpeed = (Mode == 6 || Mode == 7 || Mode == 8) ? 0.f : Rng.FRandRange(5.f, 12.65f);
			}
			if (Mode != 8 && Mode != 9)
			{
				const FVector2D To = Target - Pos;
				if (To.Size() > 0.5) WantYaw = FMath::RadiansToDegrees((float)FMath::Atan2(To.Y, To.X));
			}
			Yaw = FMath::UnwindDegrees(Yaw + FMath::Clamp(FMath::UnwindDegrees(WantYaw - Yaw), -4.77f, 4.77f));   // 5 rad/s
			const FVector2D Fwd(FMath::Cos(FMath::DegreesToRadians(Yaw)), FMath::Sin(FMath::DegreesToRadians(Yaw)));
			const FVector2D WantVel = Fwd * WantSpeed;
			const FVector2D Dv = WantVel - Vel;
			const double Step = WantSpeed > 0.f ? 12.0 / 60.0 : 20.0 / 60.0;   // accelerate at 12, stop at 20 m/s^2
			Vel += Dv.Size() > Step ? Dv.GetSafeNormal() * Step : Dv;
			Pos += Vel / 60.0;
			const FVector2D In = UJamFollowCamera::NearestInsideBoards(Pos, Rink, 0.45f);   // the skater's own radius from the glass
			if (!In.Equals(Pos, 1e-9)) { Vel *= 0.5; Pos = In; }
			const FTransform T = UJamFollowCamera::StepThirdPerson(C, St, JamA40Test::Frame(L, W, R, Pos, Yaw));
			const FVector2D Fr = UJamFollowCamera::FrameFraction(T, Fov, Aspect, FVector(Pos.X * 100.0, Pos.Y * 100.0, C.ChestM * 100.0));
			++Out.Frames;
			Out.MaxX = FMath::Max(Out.MaxX, FMath::Abs((float)Fr.X));
			Out.MaxY = FMath::Max(Out.MaxY, FMath::Abs((float)Fr.Y));
			if (FMath::Abs(Fr.X) > 1.f / 3.f || FMath::Abs(Fr.Y) > 1.f / 3.f) ++Out.OutThird;
			if (FMath::Abs(Fr.X) > 1.f || FMath::Abs(Fr.Y) > 1.f) ++Out.OutFrame;
			if (!UJamFollowCamera::IsInsideBoards(JamA40Test::Cam2D(T), Rink, C.BoardsMarginM - 1e-3f)) ++Out.OutsideBoards;
			Out.MaxH = FMath::Max(Out.MaxH, (float)T.GetLocation().Z / 100.f);
			{
				const float Down = -(float)T.Rotator().Pitch;
				const float DistM = (float)FVector::Dist(T.GetLocation(), FVector(Pos.X * 100.0, Pos.Y * 100.0, C.ChestM * 100.0)) / 100.f;
				Out.MaxDown = FMath::Max(Out.MaxDown, Down);
				Out.MinDistM = FMath::Min(Out.MinDistM, DistM);
				if (Down > 55.5f) ++Out.Over55;
				if (Down > 60.f) ++Out.Over60;
				if (Down > 70.f) ++Out.Over70;
				if (DistM < 4.99f) ++Out.Near5;
			}
			if (bPrev && !bTeleport)
			{
				Out.MaxStepCm = FMath::Max(Out.MaxStepCm, (float)FVector::Dist(T.GetLocation(), Prev.GetLocation()));
				Out.MaxPitchStep = FMath::Max(Out.MaxPitchStep, FMath::Abs((float)(T.Rotator().Pitch - Prev.Rotator().Pitch)));
				Out.MaxYawStep = FMath::Max(Out.MaxYawStep, FMath::Abs(FMath::UnwindDegrees((float)(T.Rotator().Yaw - Prev.Rotator().Yaw))));
			}
			Prev = T; bPrev = true;
			Out.Yaw.Add((float)T.Rotator().Yaw);
			Out.In30.Add(FMath::Abs(Fr.X) <= 0.31f && FMath::Abs(Fr.Y) <= 0.31f);
		}
		return Out;
	};
	struct FRinkCase { const TCHAR* Name; float L, W, R; int32 Seed; };
	for (const FRinkCase& K : { FRinkCase{ TEXT("90 m"), 90.f, 45.f, 3.75f, 135 }, FRinkCase{ TEXT("140 m"), 140.f, 70.f, 5.83f, 136 }, FRinkCase{ TEXT("120 m, 8.5 m arcs"), 120.f, 60.f, 8.5f, 137 } })
	{
		const FRun A = Skate(K.L, K.W, K.R, K.Seed, Off), B = Skate(K.L, K.W, K.R, K.Seed, On);
		int32 YawDiff = 0, YawDiffHeld = 0;
		float YawDiffMax = 0.f;
		for (int32 i = 0; i < B.Yaw.Num(); ++i)
		{
			const float D = FMath::Abs(FMath::UnwindDegrees(B.Yaw[i] - A.Yaw[i]));
			if (D <= 0.01f) continue;
			++YawDiff;
			YawDiffMax = FMath::Max(YawDiffMax, D);
			if (A.In30[i]) ++YawDiffHeld;
		}
		{
			const FRun E = Skate(K.L, K.W, K.R, K.Seed, Ev3);
			int32 EYaw = 0;
			float EYawMax = 0.f;
			for (int32 i = 0; i < E.Yaw.Num(); ++i) { const float D = FMath::Abs(FMath::UnwindDegrees(E.Yaw[i] - A.Yaw[i])); if (D > 0.01f) { ++EYaw; EYawMax = FMath::Max(EYawMax, D); } }
			AddInfo(FString::Printf(TEXT("JAMSCREEN keep %s, the yaw out of the hold: frames whose yaw is not the yaw of before: evolution 3's hold %d (at most %.1f deg) -> %d (at most %.1f deg); chest outside the middle third %d -> %d; largest camera step %.0f -> %.0f cm"),
				K.Name, EYaw, EYawMax, YawDiff, YawDiffMax, E.OutThird, B.OutThird, E.MaxStepCm, B.MaxStepCm));
			TestEqual(FString::Printf(TEXT("%s: evolution 3's hold still holds him (the neutral value)"), K.Name), E.OutThird, 0);
			FJamCameraSettings Guard = On;
			Guard.KeepNoYaw = 1;
			TestEqual(FString::Printf(TEXT("%s: with the yaw as a guard (KeepNoYaw 1) the chest is never outside the middle third"), K.Name), Skate(K.L, K.W, K.R, K.Seed, Guard).OutThird, 0);
		}
		AddInfo(FString::Printf(TEXT("JAMSCREEN keep %s: chest outside the middle third on %d -> %d of %d frames (outside the frame %d -> %d); max |x| %.3f -> %.3f, max |y| %.3f -> %.3f; largest step a frame: camera %.0f -> %.0f cm, pitch %.2f -> %.2f deg, yaw %.2f -> %.2f deg; highest %.2f -> %.2f m; the yaw is not the yaw of before on %d frames (at most %.1f deg), %d of them frames the rig of before held him inside 0.31"),
			K.Name, A.OutThird, B.OutThird, B.Frames, A.OutFrame, B.OutFrame, A.MaxX, B.MaxX, A.MaxY, B.MaxY, A.MaxStepCm, B.MaxStepCm, A.MaxPitchStep, B.MaxPitchStep, A.MaxYawStep, B.MaxYawStep, A.MaxH, B.MaxH, YawDiff, YawDiffMax, YawDiffHeld));
		// Evolution 4: the camera is never turned; the frames its limited move does not yet hold him are few, short and inside the picture.
		TestEqual(FString::Printf(TEXT("%s: frames whose yaw is not the yaw of before"), K.Name), YawDiff, 0);
		TestTrue(FString::Printf(TEXT("%s: frames with the chest outside the middle third: %d (at most 20 of 6,000)"), K.Name, B.OutThird), B.OutThird <= 20);
		TestEqual(FString::Printf(TEXT("%s: frames with the chest outside the frame"), K.Name), B.OutFrame, 0);
		TestTrue(FString::Printf(TEXT("%s: he is never past 0.6 of the half frame (%.3f, %.3f)"), K.Name, B.MaxX, B.MaxY), B.MaxX <= 0.6f && B.MaxY <= 1.f / 3.f);
		TestTrue(FString::Printf(TEXT("%s: the camera of before loses him on this path (the test has teeth)"), K.Name), A.OutThird > 50);
		TestEqual(FString::Printf(TEXT("%s: the camera never crosses the boards' inset"), K.Name), B.OutsideBoards, 0);
		// The yaw is touched only where the rig of before did not hold him: never on a frame it had him inside 0.30.
		TestTrue(FString::Printf(TEXT("%s: frames the rig of before held him whose yaw is not the yaw of before: %d (at most 30 of 6000)"), K.Name, YawDiffHeld), YawDiffHeld <= 30);
		// No snap. The skater's own spin swings the picture by 4.8 degrees of yaw a frame and the rig's foot by 70 cm; holding him while the boards take the
		// arm, no frame moves the camera by more than a metre and a half, tilts it by more than 15 degrees or turns it by more than 25 (the model's worst
		// turn is 19.5 degrees, one frame of a spin at the glass on the 90 m rink; the largest steps are printed above).
		TestTrue(FString::Printf(TEXT("%s: no snap in pitch (%.2f deg per frame)"), K.Name, B.MaxPitchStep), B.MaxPitchStep <= 15.f);
		TestTrue(FString::Printf(TEXT("%s: no snap in yaw (%.2f deg per frame)"), K.Name, B.MaxYawStep), B.MaxYawStep <= 15.f);
		TestTrue(FString::Printf(TEXT("%s: no snap in position (%.0f cm per frame)"), K.Name, B.MaxStepCm), B.MaxStepCm <= 150.f);
	}

	// The faults of the track's stills, settled: his back to the side board, his back to the corner, beside the end board.
	auto Settle = [&](const FJamCameraSettings& C, const FVector2D& Pos, float Yaw, float L, float W, float R)
	{
		FJamCameraFilter St;
		FTransform T;
		for (int32 i = 0; i < 300; ++i) T = UJamFollowCamera::StepThirdPerson(C, St, JamA40Test::Frame(L, W, R, Pos, Yaw));
		return T;
	};
	struct FScene { const TCHAR* Name; FVector2D Pos; float Yaw; bool bLostBefore; };
	for (const FScene& S : { FScene{ TEXT("back to the side board, 0.5 m from it"), FVector2D(8, -22.0), 90.f, false }, FScene{ TEXT("back to the side board, 2 m from it"), FVector2D(-20, 20.5), -90.f, false },
		FScene{ TEXT("back to the corner"), FVector2D(-43.0, -20.6), 45.f, true }, FScene{ TEXT("along the end board in the corner"), FVector2D(43.6, 16.0), 90.f, false },
		FScene{ TEXT("back to the end board behind the net"), FVector2D(44.4, 1.0), 180.f, true } })
	{
		const FTransform A = Settle(Off, S.Pos, S.Yaw, 90.f, 45.f, 3.75f), B = Settle(On, S.Pos, S.Yaw, 90.f, 45.f, 3.75f);
		const FVector Chest(S.Pos.X * 100.0, S.Pos.Y * 100.0, 100.0);
		const FVector2D Fa = UJamFollowCamera::FrameFraction(A, Fov, Aspect, Chest), Fb = UJamFollowCamera::FrameFraction(B, Fov, Aspect, Chest);
		AddInfo(FString::Printf(TEXT("JAMSCREEN keep scene %s: chest at (%.2f, %.2f) -> (%.2f, %.2f); height %.2f -> %.2f m, pitch %.1f -> %.1f deg, yaw %.1f -> %.1f, %.2f -> %.2f m from the chest"),
			S.Name, Fa.X, Fa.Y, Fb.X, Fb.Y, A.GetLocation().Z / 100.0, B.GetLocation().Z / 100.0, A.Rotator().Pitch, B.Rotator().Pitch, A.Rotator().Yaw, B.Rotator().Yaw,
			FVector::Dist(A.GetLocation(), Chest) / 100.0, FVector::Dist(B.GetLocation(), Chest) / 100.0));
		TestTrue(FString::Printf(TEXT("%s: the chest is in the middle third"), S.Name), FMath::Abs(Fb.X) <= 1.f / 3.f && FMath::Abs(Fb.Y) <= 1.f / 3.f);
		if (S.bLostBefore) TestTrue(FString::Printf(TEXT("%s: it was not before"), S.Name), FMath::Abs(Fa.X) > 1.f / 3.f || FMath::Abs(Fa.Y) > 1.f / 3.f);
		if (!S.bLostBefore) TestEqual(FString::Printf(TEXT("%s: the yaw is the yaw of before"), S.Name), (float)B.Rotator().Yaw, (float)A.Rotator().Yaw, 0.01f);
		TestTrue(FString::Printf(TEXT("%s: the figure is no nearer than 5 m (it does not fill the frame)"), S.Name), FVector::Dist(B.GetLocation(), Chest) >= 500.0);
		TestTrue(FString::Printf(TEXT("%s: the camera is inside the boards' inset"), S.Name), UJamFollowCamera::IsInsideBoards(JamA40Test::Cam2D(B), FJamCameraRink{ 90.f, 45.f, 3.75f }, Off.BoardsMarginM - 1e-3f));
	}

	// Open ice at a steady pace, straight and through a wide turn, attacking either way (with and without lane A40's far-net lead in play): the rig of
	// before to the bit. The stage's fraction stands outside lane A49's settled bounds, so a rig that holds him is not touched.
	for (int32 Sign : { -1, 1 })
	{
		FJamCameraFilter Sa, Sb;
		int32 Differ = 0;
		float MaxYawDiff = 0.f, MaxPosDiffCm = 0.f, MaxPitchDiff = 0.f, MaxRollDiff = 0.f;
		int32 FirstDiffer = -1, LastDiffer = -1;
		FVector2D Pos(-30.0, -20.0);
		float Yaw = 20.f;
		for (int32 i = 0; i < 360; ++i)   // 54 m of skating that stays 15 m and more from every board
		{
			if (i >= 150 && i < 300) Yaw += 0.6f;   // 36 degrees a second
			Pos += FVector2D(FMath::Cos(FMath::DegreesToRadians(Yaw)), FMath::Sin(FMath::DegreesToRadians(Yaw))) * (9.0 / 60.0);
			const FJamPresentationFrame F = JamA40Test::Frame(140.f, 70.f, 5.83f, Pos, Yaw, Sign);
			const FTransform Ta = UJamFollowCamera::StepThirdPerson(Off, Sa, F), Tb = UJamFollowCamera::StepThirdPerson(On, Sb, F);
			if (!Ta.Equals(Tb, 1e-6)) { ++Differ; if (FirstDiffer < 0) FirstDiffer = i; LastDiffer = i; }
			MaxPitchDiff = FMath::Max(MaxPitchDiff, FMath::Abs((float)(Tb.Rotator().Pitch - Ta.Rotator().Pitch)));
			MaxRollDiff = FMath::Max(MaxRollDiff, FMath::Abs((float)(Tb.Rotator().Roll - Ta.Rotator().Roll)));
			MaxYawDiff = FMath::Max(MaxYawDiff, FMath::Abs(FMath::UnwindDegrees((float)(Tb.Rotator().Yaw - Ta.Rotator().Yaw))));
			MaxPosDiffCm = FMath::Max(MaxPosDiffCm, (float)FVector::Dist(Ta.GetLocation(), Tb.GetLocation()));
		}
		AddInfo(FString::Printf(TEXT("JAMSCREEN keep open ice, far net at %s: the pose is not the pose of before on %d of 360 frames (yaw by at most %.6f deg, pitch %.6f deg, roll %.6f deg, the camera by at most %.6f cm; frames %d to %d)"),
			Sign > 0 ? TEXT("+X") : TEXT("-X"), Differ, MaxYawDiff, MaxPitchDiff, MaxRollDiff, MaxPosDiffCm, FirstDiffer, LastDiffer));
		TestEqual(FString::Printf(TEXT("open ice at 9 m/s, straight and turning, far net at %s: frames whose pose is not the pose of before"), Sign > 0 ? TEXT("+X") : TEXT("-X")), Differ, 0);
	}
	// The choices behind evolution 4's default, on the 90 m path: the yaw as a guard (1) or never (2), and the limit on the hold's own move of the foot.
	{
		const FRun A0 = Skate(90.f, 45.f, 3.75f, 135, Off);
		struct FChoice { int32 NoYaw; float StepM; };
		for (const FChoice& Ch : { FChoice{ 1, 0.f }, FChoice{ 1, 0.3f }, FChoice{ 1, 0.6f }, FChoice{ 1, 1.0f }, FChoice{ 1, 1.5f }, FChoice{ 2, 0.3f }, FChoice{ 2, 0.6f }, FChoice{ 2, 1.0f }, FChoice{ 2, 1.5f } })
		{
			FJamCameraSettings V = On;
			V.KeepNoYaw = Ch.NoYaw;
			V.KeepStepM = Ch.StepM;
			const FRun R = Skate(90.f, 45.f, 3.75f, 135, V);
			int32 Yd = 0;
			float YdMax = 0.f;
			for (int32 i = 0; i < R.Yaw.Num(); ++i) { const float D = FMath::Abs(FMath::UnwindDegrees(R.Yaw[i] - A0.Yaw[i])); if (D > 0.01f) { ++Yd; YdMax = FMath::Max(YdMax, D); } }
			AddInfo(FString::Printf(TEXT("JAMSCREEN keep choice 90 m: KeepNoYaw %d, KeepStepM %.1f: yaw not the yaw of before on %d frames (at most %.1f deg); chest outside the middle third %d (outside the frame %d, largest %.3f, %.3f); largest step a frame: camera %.0f cm, pitch %.2f deg, yaw %.2f deg"),
				Ch.NoYaw, Ch.StepM, Yd, YdMax, R.OutThird, R.OutFrame, R.MaxX, R.MaxY, R.MaxStepCm, R.MaxPitchStep, R.MaxYawStep));
		}
	}
	// Lane A158 (evolution 5): the pitch cap on the same three paths. Height and pitch alone: the yaw of every frame is the yaw without the cap, the foot
	// stays inside the boards, he is held as before, nothing snaps; the frames seen from steeply above fall.
	TestEqual(TEXT("the pitch cap is off in the settings (the neutral value)"), FJamCameraSettings().PitchCapDeg, 0.f);
	for (const FRinkCase& K : { FRinkCase{ TEXT("90 m"), 90.f, 45.f, 3.75f, 135 }, FRinkCase{ TEXT("140 m"), 140.f, 70.f, 5.83f, 136 }, FRinkCase{ TEXT("120 m, 8.5 m arcs"), 120.f, 60.f, 8.5f, 137 } })
	{
		const FRun B = Skate(K.L, K.W, K.R, K.Seed, On);
		// Lane A171 (evolution 6): the cap read again from the foot's final place, beside evolution 5's cap at 55.
		{
			FJamCameraSettings Ev5 = On;
			Ev5.PitchCapDeg = 55.f;
			FJamCameraSettings After = Ev5;
			After.bPitchCapAfterKeep = true;
			const FRun E = Skate(K.L, K.W, K.R, K.Seed, Ev5), R = Skate(K.L, K.W, K.R, K.Seed, After);
			int32 YawDiff = 0;
			for (int32 i = 0; i < R.Yaw.Num(); ++i) if (FMath::Abs(FMath::UnwindDegrees(R.Yaw[i] - B.Yaw[i])) > 0.01f) ++YawDiff;
			AddInfo(FString::Printf(TEXT("JAMSCREEN pitchcap-after %s cap 55, no cap -> evolution 5 -> read after the stage: frames steeper than 55.5 deg %d -> %d -> %d, than 60 deg %d -> %d -> %d, than 70 deg %d -> %d -> %d; steepest %.1f -> %.1f -> %.1f deg; highest %.2f -> %.2f -> %.2f m; nearest to the chest %.2f -> %.2f -> %.2f m (frames under 4.99 m: %d -> %d -> %d); chest outside the middle third %d -> %d -> %d (outside the frame %d, largest %.3f, %.3f); yaw not the yaw without the cap on %d frames; largest step a frame: camera %.0f -> %.0f -> %.0f cm, pitch %.2f -> %.2f -> %.2f deg"),
				K.Name, B.Over55, E.Over55, R.Over55, B.Over60, E.Over60, R.Over60, B.Over70, E.Over70, R.Over70, B.MaxDown, E.MaxDown, R.MaxDown, B.MaxH, E.MaxH, R.MaxH, B.MinDistM, E.MinDistM, R.MinDistM,
				B.Near5, E.Near5, R.Near5, B.OutThird, E.OutThird, R.OutThird, R.OutFrame, R.MaxX, R.MaxY, YawDiff, B.MaxStepCm, E.MaxStepCm, R.MaxStepCm, B.MaxPitchStep, E.MaxPitchStep, R.MaxPitchStep));
			TestEqual(FString::Printf(TEXT("%s, the cap read after the stage: frames whose yaw is not the yaw without the cap"), K.Name), YawDiff, 0);
			TestEqual(FString::Printf(TEXT("%s, the cap read after the stage: frames with the chest outside the frame"), K.Name), R.OutFrame, 0);
			TestEqual(FString::Printf(TEXT("%s, the cap read after the stage: the camera never crosses the boards' inset"), K.Name), R.OutsideBoards, 0);
			TestTrue(FString::Printf(TEXT("%s, the cap read after the stage: no more frames steeper than 60 degrees than evolution 5's cap (%d, %d)"), K.Name, R.Over60, E.Over60), R.Over60 <= E.Over60);
			TestTrue(FString::Printf(TEXT("%s, the cap read after the stage: he is held as without the cap (outside the middle third on %d frames, %d without)"), K.Name, R.OutThird, B.OutThird), R.OutThird <= FMath::Max(B.OutThird + 4, 20) && R.OutThird <= 24);
			TestTrue(FString::Printf(TEXT("%s, the cap read after the stage: no snap (camera %.0f cm, pitch %.2f deg a frame)"), K.Name, R.MaxStepCm, R.MaxPitchStep), R.MaxStepCm <= 150.f && R.MaxPitchStep <= 15.f);
			TestTrue(FString::Printf(TEXT("%s, the cap read after the stage: the camera is never higher than with evolution 5's cap (%.2f, %.2f m)"), K.Name, R.MaxH, E.MaxH), R.MaxH <= E.MaxH + 0.01f);
		}
		for (const float Cap : { 50.f, 55.f, 60.f })
		{
			FJamCameraSettings Capped = On;
			Capped.PitchCapDeg = Cap;
			const FRun R = Skate(K.L, K.W, K.R, K.Seed, Capped);
			int32 YawDiff = 0;
			for (int32 i = 0; i < R.Yaw.Num(); ++i) if (FMath::Abs(FMath::UnwindDegrees(R.Yaw[i] - B.Yaw[i])) > 0.01f) ++YawDiff;
			AddInfo(FString::Printf(TEXT("JAMSCREEN pitchcap %s cap %.0f: frames steeper than 55.5 deg %d -> %d, than 60 deg %d -> %d, than 70 deg %d -> %d; steepest %.1f -> %.1f deg; highest %.2f -> %.2f m; nearest to the chest %.2f -> %.2f m (frames under 4.99 m: %d -> %d); chest outside the middle third %d -> %d (outside the frame %d -> %d, largest %.3f, %.3f); yaw not the yaw without the cap on %d frames; largest step a frame: camera %.0f -> %.0f cm, pitch %.2f -> %.2f deg"),
				K.Name, Cap, B.Over55, R.Over55, B.Over60, R.Over60, B.Over70, R.Over70, B.MaxDown, R.MaxDown, B.MaxH, R.MaxH, B.MinDistM, R.MinDistM, B.Near5, R.Near5, B.OutThird, R.OutThird, B.OutFrame, R.OutFrame, R.MaxX, R.MaxY, YawDiff,
				B.MaxStepCm, R.MaxStepCm, B.MaxPitchStep, R.MaxPitchStep));
			TestEqual(FString::Printf(TEXT("%s, cap %.0f: frames whose yaw is not the yaw without the cap"), K.Name, Cap), YawDiff, 0);
			TestEqual(FString::Printf(TEXT("%s, cap %.0f: frames with the chest outside the frame"), K.Name, Cap), R.OutFrame, 0);
			TestEqual(FString::Printf(TEXT("%s, cap %.0f: the camera never crosses the boards' inset"), K.Name, Cap), R.OutsideBoards, 0);
			TestTrue(FString::Printf(TEXT("%s, cap %.0f: the camera is never higher than without the cap (%.2f, %.2f m)"), K.Name, Cap, R.MaxH, B.MaxH), R.MaxH <= B.MaxH + 0.01f);
			TestTrue(FString::Printf(TEXT("%s, cap %.0f: no more frames steeper than 60 degrees than without the cap (%d, %d)"), K.Name, Cap, R.Over60, B.Over60), R.Over60 <= B.Over60);
			TestTrue(FString::Printf(TEXT("%s, cap %.0f: he is held as without the cap (outside the middle third on %d frames, %d without; at most 20)"), K.Name, Cap, R.OutThird, B.OutThird), R.OutThird <= FMath::Max(B.OutThird + 4, 20) && R.OutThird <= 24);
			TestTrue(FString::Printf(TEXT("%s, cap %.0f: no snap (camera %.0f cm, pitch %.2f deg a frame)"), K.Name, Cap, R.MaxStepCm, R.MaxPitchStep), R.MaxStepCm <= 150.f && R.MaxPitchStep <= 15.f);
		}
	}
	// For the next lane: the same model with the crane rising at 0.4 m a frame (the default is 0.2).
	{
		FJamCameraSettings Fast = On;
		Fast.KeepRiseM = 0.4f;
		const FRun R = Skate(90.f, 45.f, 3.75f, 135, Fast);
		AddInfo(FString::Printf(TEXT("JAMSCREEN keep 90 m with KeepRiseM 0.4: outside the middle third %d; largest step a frame: camera %.0f cm, pitch %.2f deg, yaw %.2f deg"), R.OutThird, R.MaxStepCm, R.MaxPitchStep, R.MaxYawStep));
	}
	return true;
}

// ---- lane A158 (track U1, evolution 5): nothing is drawn on top of the key feature. Pure rules, no world.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudClearDown, "Jam.Hud.ClearDown", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudClearDown::RunTest(const FString&)
{
	TestTrue(TEXT("the rule is on by default"), UJamHudWidget::ClearDownOn());
	// The ring: gone while he is down, back over RingBackFrames from the frame he is up; the neutral value draws it always.
	TestEqual(TEXT("ring: never down"), UJamHudWidget::RingDownAlpha(true, false, 500, -1), 1.f);
	TestEqual(TEXT("ring: down"), UJamHudWidget::RingDownAlpha(true, true, 500, -1), 0.f);
	TestEqual(TEXT("ring: down, whatever the memory says"), UJamHudWidget::RingDownAlpha(true, true, 500, 400), 0.f);
	TestEqual(TEXT("ring: the first frame up"), UJamHudWidget::RingDownAlpha(true, false, 600, 600), 1.f / UJamHudWidget::RingBackFrames);
	TestEqual(TEXT("ring: half way back"), UJamHudWidget::RingDownAlpha(true, false, 605, 600), 0.5f);
	TestEqual(TEXT("ring: back"), UJamHudWidget::RingDownAlpha(true, false, 600 + UJamHudWidget::RingBackFrames, 600), 1.f);
	TestEqual(TEXT("ring: a restart (the frame count went back)"), UJamHudWidget::RingDownAlpha(true, false, 10, 600), 1.f);
	TestEqual(TEXT("ring: the neutral value draws it on a downed human"), UJamHudWidget::RingDownAlpha(false, true, 500, -1), 1.f);
	// The ring beside a downed figure: out over RingYieldFrames, back the same way.
	{
		float Share = 1.f;
		for (int32 i = 0; i < UJamHudWidget::RingYieldFrames; ++i) Share = UJamHudWidget::RingYieldStep(Share, true);
		TestEqual(TEXT("ring: gone after RingYieldFrames on a downed figure"), Share, 0.f);
		TestEqual(TEXT("ring: it stays gone"), UJamHudWidget::RingYieldStep(Share, true), 0.f);
		Share = UJamHudWidget::RingYieldStep(Share, false);
		TestTrue(TEXT("ring: it starts back once clear"), Share > 0.f && Share < 1.f);
		for (int32 i = 0; i < UJamHudWidget::RingYieldFrames; ++i) Share = UJamHudWidget::RingYieldStep(Share, false);
		TestEqual(TEXT("ring: back"), Share, 1.f);
	}
	// Boxes.
	TestTrue(TEXT("boxes that overlap meet"), UJamHudWidget::BoxesMeet(FVector2D(0, 0), FVector2D(10, 10), FVector2D(9, 9), FVector2D(20, 20)));
	TestFalse(TEXT("boxes that touch do not"), UJamHudWidget::BoxesMeet(FVector2D(0, 0), FVector2D(10, 10), FVector2D(10, 0), FVector2D(20, 10)));
	TestTrue(TEXT("a pad makes neighbours meet"), UJamHudWidget::BoxesMeet(FVector2D(0, 0), FVector2D(10, 10), FVector2D(14, 0), FVector2D(20, 10), 6.f));
	// The caption: its dock, the slot under the scoreboard when the dock is taken, no hop back, nothing when both are taken.
	TestEqual(TEXT("caption: the dock when it is free"), UJamHudWidget::CaptionPlace(0, true, true), 0);
	TestEqual(TEXT("caption: the top when a figure is under the dock"), UJamHudWidget::CaptionPlace(0, false, true), 1);
	TestEqual(TEXT("caption: it stays at the top while the top is free (no hop back)"), UJamHudWidget::CaptionPlace(1, true, true), 1);
	TestEqual(TEXT("caption: back to the dock when the top is taken"), UJamHudWidget::CaptionPlace(1, true, false), 0);
	TestEqual(TEXT("caption: not drawn when both are taken"), UJamHudWidget::CaptionPlace(0, false, false), -1);
	TestEqual(TEXT("caption: not drawn when both are taken (from the top)"), UJamHudWidget::CaptionPlace(1, false, false), -1);
	TestEqual(TEXT("caption: a hidden frame keeps the dock as its place"), UJamHudWidget::CaptionPlace(-1, true, false), 0);
	// The contact line's step along its row (1920 wide, the row 850..888, the line 300 wide).
	{
		const float VW = 1920.f, HalfW = 150.f, Y0 = 850.f, Y1 = 888.f, Pad = 6.f, MinX = 48.f + HalfW, MaxX = VW - 430.f - HalfW;
		float X = 0.f;
		TArray<FBox2D> None;
		TestTrue(TEXT("contact: a free row"), UJamHudWidget::SideStep(960.f, HalfW, Y0, Y1, None, MinX, MaxX, Pad, X));
		TestEqual(TEXT("contact: it stays at the centre"), X, 960.f);
		TArray<FBox2D> Lying = { FBox2D(FVector2D(900, 800), FVector2D(1220, 900)) };   // a figure lying under the centre, more of it to the right
		TestTrue(TEXT("contact: a figure under the centre"), UJamHudWidget::SideStep(960.f, HalfW, Y0, Y1, Lying, MinX, MaxX, Pad, X));
		TestTrue(FString::Printf(TEXT("contact: it steps to the nearer side, clear of the figure (%.0f)"), X), X + HalfW + Pad <= 900.f && X < 960.f);
		TestFalse(TEXT("contact: its box there meets nothing"), UJamHudWidget::BoxMeetsAny(FVector2D(X - HalfW, Y0), FVector2D(X + HalfW, Y1), Lying, Pad));
		float X2 = 0.f;
		TestTrue(TEXT("contact: from where it stepped to"), UJamHudWidget::SideStep(X, HalfW, Y0, Y1, Lying, MinX, MaxX, Pad, X2));
		TestEqual(TEXT("contact: it stays there (no hop)"), X2, X);
		TArray<FBox2D> Above = { FBox2D(FVector2D(900, 500), FVector2D(1220, 700)) };   // a figure over the row: not on it
		TestTrue(TEXT("contact: a figure above the row"), UJamHudWidget::SideStep(960.f, HalfW, Y0, Y1, Above, MinX, MaxX, Pad, X));
		TestEqual(TEXT("contact: the centre"), X, 960.f);
		TArray<FBox2D> Wall = { FBox2D(FVector2D(0, 800), FVector2D(1920, 900)) };       // the row is taken from edge to edge
		TestFalse(TEXT("contact: no place on the row"), UJamHudWidget::SideStep(960.f, HalfW, Y0, Y1, Wall, MinX, MaxX, Pad, X));
		TArray<FBox2D> Left = { FBox2D(FVector2D(100, 800), FVector2D(1000, 900)) };     // only the right is free, short of the ability plate
		TestTrue(TEXT("contact: only the right is free"), UJamHudWidget::SideStep(960.f, HalfW, Y0, Y1, Left, MinX, MaxX, Pad, X));
		TestTrue(FString::Printf(TEXT("contact: right of the figure, left of the ability plate (%.0f)"), X), X - HalfW - Pad >= 1000.f && X <= MaxX);
	}
	// The own-net marker: not on a net that is in the frame; the edge chevron when it is not; the neutral value draws it either way.
	TestFalse(TEXT("own marker: not drawn on a net in the frame"), UJamHudWidget::ShowOwnMarker(true, false));
	TestTrue(TEXT("own marker: drawn for a net out of the frame"), UJamHudWidget::ShowOwnMarker(false, false));
	TestTrue(TEXT("own marker: the neutral value"), UJamHudWidget::ShowOwnMarker(true, true));
	return true;
}

// ---- lane A168's Jam.Hud.PuckPostFlat went with jam.puck.Post (lane A188): device B's disc no longer carries the upright marker.

// ---- lane A171 (track U1, evolution 6): TOO SOON, and the last overlaps. Pure rules and a staged sim, no world.
//   Jam.Hud.TooSoon     the contact line's word for a press the check cooldown dropped, where it stands among the others, and on the sim with
//                       game.contact.check_cooldown_frames on: a mashed button raises the read-out the line reads; with the key off (as shipped) never.
//   Jam.Hud.ClearEdges  the caption's row under the scoreboard (clear of the MATCH card, the prompt, a net's pill), the prompt under the card, the
//                       own-net marker above the radar; the marks' pad over a downed figure.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudTooSoon, "Jam.Hud.TooSoon", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudTooSoon::RunTest(const FString&)
{
	// The word and its place: after everything that landed or was refused, before MISSED.
	TestEqual(TEXT("a press the cooldown dropped"), UJamHudWidget::ContactLineFor(false, false, false, false, false, true), FString(TEXT("TOO SOON")));
	TestEqual(TEXT("with the last window running out on the same frame: the press he just made"), UJamHudWidget::ContactLineFor(false, false, false, true, false, true), FString(TEXT("TOO SOON")));
	TestEqual(TEXT("a soft hit on the same frame wins"), UJamHudWidget::ContactLineFor(false, false, true, false, false, true), FString(TEXT("TOO SOFT")));
	TestEqual(TEXT("a refused press wins"), UJamHudWidget::ContactLineFor(true, false, false, false, false, true), FString(TEXT("NOT WITH THE PUCK")));
	TestEqual(TEXT("his own bounce wins"), UJamHudWidget::ContactLineFor(false, true, false, false, false, true), FString(TEXT("BOUNCED OFF")));
	TestEqual(TEXT("the grace wins"), UJamHudWidget::ContactLineFor(false, false, false, false, true, true), FString(TEXT("JUST GOT UP")));
	TestEqual(TEXT("the lines of before are the lines of before"), UJamHudWidget::ContactLineFor(false, false, false, true), FString(TEXT("MISSED")));
	TestTrue(TEXT("nothing: no line"), UJamHudWidget::ContactLineFor(false, false, false, false, false, false).IsEmpty());

	// The read-out the line reads, on the sim: the human stands alone in open ice and presses check every 12 frames for 60 frames.
	TArray<FString> Errors;
	UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")), Errors);
	for (const FString& E : Errors) AddError(E);
	if (!C) return false;
	TestEqual(TEXT("hockey.regular ships the cooldown off (the line cannot show in the shipped game)"), C->Game.Contact.CheckCooldownFrames, 0);
	auto Mash = [&](int32 Cooldown, bool bRestart, int32& OutLines) -> int32
	{
		FJamGameConfig G = C->Game;
		G.Contact.CheckCooldownFrames = Cooldown;
		G.Contact.bCheckCooldownRestart = bRestart;
		UJamSim* Sim = NewObject<UJamSim>();
		Sim->Init(G, 1);
		Sim->SkipFaceoff();
		const int32 Me = Sim->GetHumanIndex();
		const int32 N = Sim->GetSkaters().Num();
		for (int32 i = 0; i < N; ++i) Sim->SetSkater(i, FVector2D(-40.0 + i * 1.5, 19.0), FVector2D::ZeroVector, 0);   // everybody else lined up far away
		Sim->SetSkater(Me, FVector2D(10.0, -10.0), FVector2D::ZeroVector, 0);
		Sim->SetPuck(FVector2D(-30.0, -15.0), FVector2D::ZeroVector);
		// The HUD's own rule, frame by frame: a line when the counter rises (UpdateContactLine's ContactSeenCooling).
		int32 Seen = 0;
		OutLines = 0;
		for (int32 F = 0; F < 60; ++F)
		{
			TArray<FJamInput> In;
			In.SetNum(N);
			In[Me].bCheck = F % 12 == 0;
			Sim->Step(In);
			const FJamSkaterState& Hu = Sim->GetSkaters()[Me];
			const bool bCooling = Hu.ChecksCooling > Seen;
			Seen = Hu.ChecksCooling;
			if (UJamHudWidget::ContactLineFor(false, false, false, false, false, bCooling) == TEXT("TOO SOON")) ++OutLines;
		}
		return Sim->GetSkaters()[Me].ChecksCooling;
	};
	int32 LinesOff = 0, Lines40 = 0, LinesRestart = 0;
	const int32 Off = Mash(0, false, LinesOff), On40 = Mash(40, false, Lines40), Restart = Mash(36, true, LinesRestart);
	AddInfo(FString::Printf(TEXT("JAMSCREEN toosoon five presses 12 frames apart: the cooldown dropped %d with the key off, %d at check_cooldown_frames 40 (TOO SOON lines %d), %d at 36 with check_cooldown_restart (lines %d)"),
		Off, On40, Lines40, Restart, LinesRestart));
	TestEqual(TEXT("the key off (as shipped): no press is dropped"), Off, 0);
	TestEqual(TEXT("the key off: no TOO SOON line"), LinesOff, 0);
	TestTrue(FString::Printf(TEXT("check_cooldown_frames 40: the mash has presses dropped (%d)"), On40), On40 >= 2);
	TestEqual(TEXT("one TOO SOON line a dropped press"), Lines40, On40);
	TestTrue(FString::Printf(TEXT("check_cooldown_restart: every press after the first is dropped (%d of 4)"), Restart), Restart == 4 && LinesRestart == 4);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudClearEdges, "Jam.Hud.ClearEdges", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudClearEdges::RunTest(const FString&)
{
	TestFalse(TEXT("the rule is off by default (it follows the package; one number turns it on)"), UJamHudWidget::ClearEdgesOn());
	const float VW = 1920.f, VH = 1080.f, Top = UJamHudWidget::PromptSlotY, H = 44.f, X0 = 660.f, X1 = 1260.f, Gap = 8.f, MaxY = VH / 3.f;
	// The caption's row under the scoreboard.
	{
		TArray<FBox2D> None;
		TestEqual(TEXT("an empty band: the first row"), UJamHudWidget::FirstClearY(Top, H, X0, X1, None, Gap, MaxY), Top);
		// The far net's distance pill straight ahead (the window lane's 081-knock: "49 m" under "Flattened!").
		TArray<FBox2D> Pill = { FBox2D(FVector2D(960, 128), FVector2D(1050, 164)) };
		const float UnderPill = UJamHudWidget::FirstClearY(Top, H, X0, X1, Pill, Gap, MaxY);
		TestEqual(TEXT("a net's pill in the first row: the row under it"), UnderPill, 172.f);
		TestEqual(TEXT("the row it took is kept while it is clear (the same answer from there)"), UJamHudWidget::FirstClearY(UnderPill, H, X0, X1, Pill, Gap, MaxY), UnderPill);
		TArray<FBox2D> PillAside = { FBox2D(FVector2D(1400, 128), FVector2D(1490, 164)) };
		TestEqual(TEXT("a pill beside the row: the first row"), UJamHudWidget::FirstClearY(Top, H, X0, X1, PillAside, Gap, MaxY), Top);
		// The MATCH card (its band 121..295) with the prompt in the row under it.
		float BandTop = 0.f, BandBottom = 0.f;
		UJamDirectorsCallHudWidget::BannerBandPx(VH, BandTop, BandBottom);
		TArray<FBox2D> Card = { FBox2D(FVector2D(VW * 0.5f - 232.f, BandTop), FVector2D(VW * 0.5f + 232.f, BandBottom)) };
		const float UnderCard = UJamHudWidget::FirstClearY(Top, H, X0, X1, Card, Gap, MaxY);
		TestEqual(TEXT("the MATCH card up: the row under its band"), UnderCard, BandBottom + Gap);
		TestTrue(TEXT("that row ends above the middle third"), UnderCard + H <= MaxY);
		Card.Add(FBox2D(FVector2D(700, BandBottom + 8.f), FVector2D(1220, BandBottom + 8.f + UJamHudWidget::PromptSlotH)));
		TestEqual(TEXT("the card and the prompt under it: no row above the middle third (the caption keeps its dock or is not drawn)"), UJamHudWidget::FirstClearY(Top, H, X0, X1, Card, Gap, MaxY), -1.f);
		// Two things stacked: the answer clears both whatever their order.
		TArray<FBox2D> Two = { FBox2D(FVector2D(900, 160), FVector2D(1000, 200)), FBox2D(FVector2D(800, 110), FVector2D(1100, 156)) };
		TestEqual(TEXT("a pill under a prompt: under both"), UJamHudWidget::FirstClearY(Top, H, X0, X1, Two, Gap, MaxY), 208.f);
	}
	// The prompt under the MATCH card.
	{
		float BandTop = 0.f, BandBottom = 0.f;
		UJamDirectorsCallHudWidget::BannerBandPx(VH, BandTop, BandBottom);
		TestEqual(TEXT("the prompt's first row lies in the card's band: under the band"), UJamHudWidget::UnderBand(UJamHudWidget::PromptSlotY, UJamHudWidget::PromptSlotH, BandTop, BandBottom), BandBottom + 8.f);
		TestEqual(TEXT("its lower row too"), UJamHudWidget::UnderBand(UJamHudWidget::PromptSlotLowY, UJamHudWidget::PromptSlotH, BandTop, BandBottom), BandBottom + 8.f);
		TestTrue(TEXT("under the band is above the middle third"), BandBottom + 8.f + UJamHudWidget::PromptSlotH <= VH / 3.f);
		TestEqual(TEXT("a row clear of the band stays"), UJamHudWidget::UnderBand(320.f, UJamHudWidget::PromptSlotH, BandTop, BandBottom), 320.f);
	}
	// The own-net marker above the radar and the ability plate.
	{
		const FVector2D InCorner = UJamHudWidget::KeepOffRadar(FVector2D(VW - 88.f, VH - 104.f), VW, VH, 60.f, 40.f);
		TestTrue(FString::Printf(TEXT("a marker in the bottom right corner is lifted: its label ends above the ability plate (y %.0f)"), InCorner.Y), InCorner.Y + 40.f <= VH - 240.f && InCorner.X == VW - 88.f);
		const FVector2D Left = FVector2D(88.f, VH - 104.f);
		TestTrue(TEXT("the bottom left corner is not the radar's"), UJamHudWidget::KeepOffRadar(Left, VW, VH, 60.f, 40.f) == Left);
		const FVector2D High = FVector2D(VW - 88.f, 500.f);
		TestTrue(TEXT("the right edge above the corner is free"), UJamHudWidget::KeepOffRadar(High, VW, VH, 60.f, 40.f) == High);
		const FVector2D Mid = FVector2D(VW - 500.f, VH - 104.f);
		TestTrue(TEXT("the bottom edge left of the plate is free"), UJamHudWidget::KeepOffRadar(Mid, VW, VH, 60.f, 40.f) == Mid);
	}
	// The stop's line on the screen (lane A171): its length, its thickness across itself and never under the minimum, its turn, the box it covers.
	{
		FVector2D C, Sz, Min, Max;
		float Deg = 0.f;
		TestTrue(TEXT("a line lying level"), UJamHudWidget::StopBarLayout(FVector2D(400, 300), FVector2D(600, 300), FVector2D(500, 294), FVector2D(500, 306), 4.f, C, Sz, Deg, Min, Max));
		TestTrue(FString::Printf(TEXT("level: 200 long, 12 thick, not turned, its box 400..600 x 294..306 (%.0f x %.0f, %.1f deg)"), Sz.X, Sz.Y, Deg),
			FMath::IsNearlyEqual(Sz.X, 200.0, 0.01) && FMath::IsNearlyEqual(Sz.Y, 12.0, 0.01) && FMath::IsNearlyZero(Deg, 0.01f) && C.Equals(FVector2D(500, 300), 0.01) && Min.Equals(FVector2D(400, 294), 0.01) && Max.Equals(FVector2D(600, 306), 0.01));
		TestTrue(TEXT("a far line (its thickness a pixel on the screen)"), UJamHudWidget::StopBarLayout(FVector2D(400, 300), FVector2D(600, 300), FVector2D(500, 299.5), FVector2D(500, 300.5), 4.f, C, Sz, Deg, Min, Max));
		TestEqual(TEXT("... is never thinner than the minimum"), static_cast<float>(Sz.Y), 4.f);
		TestTrue(TEXT("a line seen at a slant"), UJamHudWidget::StopBarLayout(FVector2D(400, 300), FVector2D(500, 400), FVector2D(445, 355), FVector2D(455, 345), 4.f, C, Sz, Deg, Min, Max));
		TestTrue(FString::Printf(TEXT("slanted: turned 45 degrees, 141 long, 14 thick (%.1f deg, %.1f x %.1f)"), Deg, Sz.X, Sz.Y), FMath::IsNearlyEqual(Deg, 45.f, 0.01f) && FMath::IsNearlyEqual(Sz.X, 141.42, 0.01) && FMath::IsNearlyEqual(Sz.Y, 14.14, 0.01));
		TestTrue(TEXT("... and its box holds both ends and the thickness"), Min.X < 400.0 && Min.Y < 300.0 && Max.X > 500.0 && Max.Y > 400.0 && Max.X - Min.X < 112.0);
		TestFalse(TEXT("a line with no length is not drawn"), UJamHudWidget::StopBarLayout(FVector2D(400, 300), FVector2D(400.2, 300), FVector2D(400, 299), FVector2D(400, 301), 4.f, C, Sz, Deg, Min, Max));
		TestTrue(TEXT("two stick lengths wide in all; lit it is thicker than armed"), UJamHudWidget::StopBarHalfWidthM * 2.f == 3.2f && UJamHudWidget::StopBarLitThickM > UJamHudWidget::StopBarArmedThickM);
	}
	// The marks yield to the same boxes as the markers, with the markers' pad (lane A166's rings; lane A158's boxes).
	{
		TArray<FBox2D> Down = { FBox2D(FVector2D(200, 100), FVector2D(300, 160)) };
		TestTrue(TEXT("a mark 10 px from a downed figure's box is not drawn (the pad is 16)"), UJamHudWidget::MarkMeetsDown(FVector2D(100, 100), FVector2D(190, 150), Down, UJamHudWidget::MarkDownPadPx));
		TestFalse(TEXT("20 px away: drawn"), UJamHudWidget::MarkMeetsDown(FVector2D(100, 100), FVector2D(180, 150), Down, UJamHudWidget::MarkDownPadPx));
	}
	return true;
}

// ---- lane A135 (track U1, evolution 3): bark text on a fixed lane, and the brake prompt in the player's own input. Pure rules, no world.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudBarkLane, "Jam.Hud.BarkLane", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudBarkLane::RunTest(const FString&)
{
	// Every bubble width on three frames (Slate units: 1280 x 720 unscaled, the 1080p reference, an ultrawide), both sides.
	int32 Places = 0, Bad = 0;
	float MinSkates = 1e9f;
	const float BubH = 54.f;   // the tallest bubble (text, padding and the tail's band)
	for (const FVector2D& V : { FVector2D(1280, 720), FVector2D(1920, 1080), FVector2D(2580, 1080) })
	{
		const float VW = (float)V.X, VH = (float)V.Y;
		for (int32 Team = 0; Team < 2; ++Team) for (float HalfW = 60.f; HalfW <= 0.5f * (VW * 0.5f - UJamBarkWidget::LaneGapX - UJamBarkWidget::LaneEdgePx); HalfW += 7.f)
		{
			const FVector2D P = UJamBarkWidget::LanePos(Team, HalfW, VW);
			const float L = (float)P.X - HalfW, R = (float)P.X + HalfW, T = (float)P.Y - BubH, B = (float)P.Y;
			++Places;
			bool bOk = L >= 0.f && R <= VW && T >= 0.f;                                            // inside the frame
			bOk &= Team == 0 ? R <= VW * 0.5f - UJamBarkWidget::LaneGapX + 0.01f : L >= VW * 0.5f + UJamBarkWidget::LaneGapX - 0.01f;   // beside the board, on its team's side
			bOk &= B <= 104.f;                                                                     // in the scoreboard's row: over every net label and the puck's arrow (their clamps: 120 - 15, 150)
			bOk &= B < UJamHudWidget::PromptSlotY;                                                 // over the prompt's slots
			bOk &= B < VH / 3.f;                                                                   // over the middle third
			// The human's skates: his chest is inside the middle third (the camera's rule), his skates under it; 100 px of a 720-line frame is VH * 100 / 720.
			const float Skates = VH / 3.f - B;
			MinSkates = FMath::Min(MinSkates, Skates * 720.f / VH);
			bOk &= Skates >= VH * 100.f / 720.f;
			if (!bOk) ++Bad;
		}
	}
	AddInfo(FString::Printf(TEXT("JAMSCREEN bark lane: %d places on three frames; %d outside the lane's rules; the human's skates are at least %.0f px (of 720 lines) under the lane"), Places, Bad, MinSkates));
	TestTrue(TEXT("places were tried"), Places > 100);
	TestEqual(TEXT("lane places that break a rule"), Bad, 0);
	TestTrue(TEXT("the human's team is on the left, beside its plate"), UJamBarkWidget::LanePos(0, 100.f, 1920.f).X < 960.f && UJamBarkWidget::LanePos(1, 100.f, 1920.f).X > 960.f);
	// The lane yields to a figure under it (the camera over the boards looks down on ice in that row): standing in it, lying beside it; not one under it.
	const FVector2D LMin(1222, 30), LMax(1600, 84);
	TestTrue(TEXT("a figure standing in the lane's box"), UJamBarkWidget::LaneMeetsFigure(LMin, LMax, FVector2D(1400, 120), FVector2D(1400, 40)));
	TestTrue(TEXT("a figure whose head reaches the lane"), UJamBarkWidget::LaneMeetsFigure(LMin, LMax, FVector2D(1300, 160), FVector2D(1300, 88)));
	TestTrue(TEXT("a figure beside the lane, within its own width"), UJamBarkWidget::LaneMeetsFigure(LMin, LMax, FVector2D(1640, 110), FVector2D(1640, 40)));
	TestFalse(TEXT("a figure under the lane"), UJamBarkWidget::LaneMeetsFigure(LMin, LMax, FVector2D(1400, 260), FVector2D(1400, 150)));
	TestFalse(TEXT("a figure across the board"), UJamBarkWidget::LaneMeetsFigure(LMin, LMax, FVector2D(600, 120), FVector2D(600, 40)));
	// A line too long for its side stays in the frame.
	TestEqual(TEXT("a long line keeps its far end in the frame"), (float)UJamBarkWidget::LanePos(0, 400.f, 1280.f).X, UJamBarkWidget::LaneEdgePx + 400.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudPromptKeys, "Jam.Hud.PromptKeys", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudPromptKeys::RunTest(const FString&)
{
	const UJamInputMap* M = UJamInputMap::Get();
	FKey Back, Brake;
	// The keyboard's binding.
	TestTrue(TEXT("the map has the keyboard's two controls"), UJamHudWidget::BrakeKeysFor(false, Back, Brake));
	TestTrue(TEXT("keys: the back control is bound to the move"), M->IsKeyMapped(M->Move, Back) && !Back.IsGamepadKey());
	TestTrue(TEXT("keys: the brake control is bound to the brake"), M->IsKeyMapped(M->Brake, Brake) && !Brake.IsGamepadKey());
	TestTrue(TEXT("keys: S pulls back"), Back == EKeys::S);
	const FString Keys = UJamHudWidget::BrakePromptText(true, false);
	TestEqual(TEXT("keys: the words"), Keys, FString(TEXT("S AGAINST YOUR MOTION, OR HOLD CTRL: STOP, THEN GO")));
	TestTrue(TEXT("keys: names both bound controls"), Keys.Contains(UJamHudWidget::KeyLabel(Back)) && Keys.Contains(UJamHudWidget::KeyLabel(Brake)));
	// The pad's binding.
	TestTrue(TEXT("the map has the pad's two controls"), UJamHudWidget::BrakeKeysFor(true, Back, Brake));
	TestTrue(TEXT("pad: the back control is bound to the move"), M->IsKeyMapped(M->Move, Back) && Back.IsGamepadKey());
	TestTrue(TEXT("pad: the brake control is bound to the brake"), M->IsKeyMapped(M->Brake, Brake) && Brake.IsGamepadKey());
	const FString Pad = UJamHudWidget::BrakePromptText(true, true);
	TestEqual(TEXT("pad: the words"), Pad, FString(TEXT("LEFT STICK AGAINST YOUR MOTION, OR HOLD L3: STOP, THEN GO")));
	TestFalse(TEXT("pad: no keyboard key in the words"), Pad.Contains(TEXT("CTRL")) || Pad.StartsWith(TEXT("S ")));
	TestFalse(TEXT("keys: no pad control in the words"), Keys.Contains(TEXT("STICK")) || Keys.Contains(TEXT("L3")));
	// A rebinding changes the words; a brake with no control of its own leaves the pull alone; the neutral value is the line of before.
	TestEqual(TEXT("rebound"), UJamHudWidget::BrakePromptFor(EKeys::Down, EKeys::SpaceBar), FString(TEXT("DOWN ARROW AGAINST YOUR MOTION, OR HOLD SPACE: STOP, THEN GO")));
	TestEqual(TEXT("no brake control"), UJamHudWidget::BrakePromptFor(EKeys::S, FKey()), FString(TEXT("S AGAINST YOUR MOTION: STOP, THEN GO")));
	TestEqual(TEXT("the neutral value"), UJamHudWidget::BrakePromptText(false, true), UJamHudWidget::PromptText(UJamHudWidget::EPrompt::Brake));
	for (const FString& Line : { Keys, Pad }) TestTrue(TEXT("one line of the pill (60 characters at most)"), Line.Len() <= 60 && Line.EndsWith(TEXT(": STOP, THEN GO")));
	AddInfo(FString::Printf(TEXT("JAMSCREEN prompt keys: keys \"%s\"; pad \"%s\""), *Keys, *Pad));
	return true;
}

// ---- lane A135, evolution 4: the prompt keeps off the attacked net. Pure rule.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudPromptOffNet, "Jam.Hud.PromptOffNet", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudPromptOffNet::RunTest(const FString&)
{
	const float VW = 1920.f, HalfW = 300.f, Top = UJamHudWidget::PromptSlotY, Low = UJamHudWidget::PromptSlotLowY;
	const FVector2D None(0, 0);
	TestTrue(TEXT("no net in the picture: top centre"), UJamHudWidget::PromptPlace(VW, HalfW, false, false, None, None).Equals(FVector2D(0, Top)));
	TestTrue(TEXT("no net, the top row taken: lower centre (evolution 3's rule)"), UJamHudWidget::PromptPlace(VW, HalfW, true, false, None, None).Equals(FVector2D(0, Low)));
	// The fault's still: the net straight ahead, its badge at the top centre, its mouth just under it.
	const FVector2D NetMin(870, 150), NetMax(1050, 270);
	const FVector2D P1 = UJamHudWidget::PromptPlace(VW, HalfW, false, true, NetMin, NetMax);
	TestTrue(TEXT("evolution 3's lower slot lay on this net"), UJamHudWidget::PromptBoxMeets(FVector2D(0, Low), VW, HalfW, NetMin, NetMax));
	TestFalse(TEXT("the place chosen is clear of the net"), UJamHudWidget::PromptBoxMeets(P1, VW, HalfW, NetMin, NetMax));
	TestEqual(TEXT("... in the top row"), (float)P1.Y, Top);
	TestTrue(TEXT("... beside the net, on its nearer side, whole in the frame"), P1.X < 0.f && VW * 0.5f + P1.X - HalfW >= 8.f && VW * 0.5f + P1.X + HalfW <= NetMin.X - 12.f);
	// A net low in the picture leaves the top centre alone; the top row taken sends the prompt to the lower row beside the net.
	TestTrue(TEXT("a net under both rows: top centre"), UJamHudWidget::PromptPlace(VW, HalfW, false, true, FVector2D(870, 400), FVector2D(1050, 520)).Equals(FVector2D(0, Top)));
	TestTrue(TEXT("a net in the top row only (far, small): the lower row centre, which does not move with the net"), UJamHudWidget::PromptPlace(VW, HalfW, false, true, FVector2D(1000, 109), FVector2D(1130, 182)).Equals(FVector2D(0, Low)));
	const FVector2D P2 = UJamHudWidget::PromptPlace(VW, HalfW, true, true, NetMin, NetMax);
	TestTrue(TEXT("the top row taken: the lower row, clear of the net"), P2.Y == Low && !UJamHudWidget::PromptBoxMeets(P2, VW, HalfW, NetMin, NetMax));
	// Every net box up to 400 wide anywhere in the upper half of two frame shapes, three pill widths, the top row free or taken: never on the net.
	int32 Tried = 0, OnNet = 0, Outside = 0;
	for (float W : { 1920.f, 2580.f }) for (float Hw : { 240.f, 300.f, 340.f }) for (int32 Taken = 0; Taken < 2; ++Taken)
		for (float Nw = 60.f; Nw <= 400.f; Nw += 85.f) for (float Nx = -100.f; Nx <= W - 60.f; Nx += 47.f) for (float Ny = 60.f; Ny <= 500.f; Ny += 31.f)
		{
			const FVector2D Mn(Nx, Ny), Mx(Nx + Nw, Ny + Nw * 0.45f + 36.f);
			const FVector2D P = UJamHudWidget::PromptPlace(W, Hw, Taken != 0, true, Mn, Mx);
			++Tried;
			if (UJamHudWidget::PromptBoxMeets(P, W, Hw, Mn, Mx)) ++OnNet;
			if (W * 0.5f + P.X - Hw < 0.f || W * 0.5f + P.X + Hw > W) ++Outside;
		}
	AddInfo(FString::Printf(TEXT("JAMSCREEN prompt off net: %d net boxes tried; the prompt on the net %d; outside the frame %d"), Tried, OnNet, Outside));
	TestEqual(TEXT("places on the net"), OnNet, 0);
	TestEqual(TEXT("places outside the frame"), Outside, 0);
	return true;
}

// ---- lane A123 (track U1, evolution 2): nothing sits on the play, and the screen teaches the two new verbs. Pure rules, no world.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudClearScreen, "Jam.Hud.ClearScreen", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudClearScreen::RunTest(const FString&)
{
	const float VW = 1920.f, VH = 1080.f;
	// The middle third is the centre rectangle.
	TestTrue(TEXT("a box at the centre is in the middle third"), UJamHudWidget::InMiddleThird(FVector2D(900, 500), FVector2D(1000, 560), VW, VH));
	TestFalse(TEXT("the scoreboard is not"), UJamHudWidget::InMiddleThird(FVector2D(700, 14), FVector2D(1220, 104), VW, VH));
	TestFalse(TEXT("the commentary strip is not"), UJamHudWidget::InMiddleThird(FVector2D(600, 1010), FVector2D(1320, 1056), VW, VH));
	TestFalse(TEXT("a label on the side edge is not"), UJamHudWidget::InMiddleThird(FVector2D(1700, 500), FVector2D(1820, 530), VW, VH));
	TestFalse(TEXT("the prompt's slot is not"), UJamHudWidget::InMiddleThird(FVector2D(560, UJamHudWidget::PromptSlotY), FVector2D(1360, UJamHudWidget::PromptSlotY + UJamHudWidget::PromptSlotH), VW, VH));
	TestFalse(TEXT("the prompt's lower slot is not"), UJamHudWidget::InMiddleThird(FVector2D(560, UJamHudWidget::PromptSlotLowY), FVector2D(1360, UJamHudWidget::PromptSlotLowY + UJamHudWidget::PromptSlotH), VW, VH));
	// The bark bubble: wherever its speaker stands, the bubble's box ends outside the middle third and inside the frame, by the shortest way.
	int32 Moved = 0, Checked = 0;
	float Farthest = 0.f;
	for (float X = 160.f; X <= VW - 160.f; X += 40.f)
	{
		for (float Y = 170.f; Y <= VH - 60.f; Y += 20.f)
		{
			for (const float HalfW : { 120.f, 220.f, 330.f })
			{
				if (X - HalfW < 0.f || X + HalfW > VW) continue;
				const FVector2D P(X, Y), Q = UJamBarkWidget::PushOutOfMiddle(P, HalfW, 54.f, VW, VH);
				++Checked;
				if (!Q.Equals(P)) ++Moved;
				Farthest = FMath::Max(Farthest, static_cast<float>(FVector2D::Distance(P, Q)));
				if (UJamHudWidget::InMiddleThird(FVector2D(Q.X - HalfW, Q.Y - 54.f), FVector2D(Q.X + HalfW, Q.Y), VW, VH) || Q.X - HalfW < 0.f || Q.X + HalfW > VW || Q.Y - 54.f < 0.f || Q.Y > VH)
				{
					AddError(FString::Printf(TEXT("a bubble at (%.0f, %.0f) half width %.0f ends at (%.0f, %.0f): in the middle third or out of the frame"), X, Y, HalfW, Q.X, Q.Y));
				}
			}
		}
	}
	AddInfo(FString::Printf(TEXT("JAMSCREEN bark push: %d of %d bubble places moved, the farthest by %.0f Slate units"), Moved, Checked, Farthest));
	TestTrue(TEXT("bubbles over the middle were moved"), Moved > 0);
	TestTrue(TEXT("no bubble moves farther than half the rectangle's height plus its own"), Farthest <= VH / 6.f + 54.f + 8.f + 1.f);
	TestEqual(TEXT("a bubble outside stays"), UJamBarkWidget::PushOutOfMiddle(FVector2D(300, 300), 120.f, 54.f, VW, VH), FVector2D(300, 300));
	TestEqual(TEXT("a bubble just under the top edge of the rectangle goes up"), UJamBarkWidget::PushOutOfMiddle(FVector2D(960, 380), 120.f, 54.f, VW, VH), FVector2D(960, VH / 3.f - 8.f));
	TestEqual(TEXT("a bubble near the bottom edge goes down"), UJamBarkWidget::PushOutOfMiddle(FVector2D(960, 730), 120.f, 54.f, VW, VH), FVector2D(960, VH * 2.f / 3.f + 8.f + 54.f));
	// The faceoff ring (220 px at 0.66 of the height): at 260 right of centre it reached into the middle third; at 440 it stands outside.
	{
		const FVector2D RingC(VW * 0.5f + UJamHudWidget::RingOffsetClearXPx, VH * 0.66f), Old(VW * 0.5f + UJamHudWidget::RingOffsetXPx, VH * 0.66f), Half(UJamHudWidget::RingBoxPx * 0.5f, UJamHudWidget::RingBoxPx * 0.5f);
		TestFalse(TEXT("the ring stands outside the middle third"), UJamHudWidget::InMiddleThird(RingC - Half, RingC + Half, VW, VH));
		TestTrue(TEXT("at its place of before it reached in"), UJamHudWidget::InMiddleThird(Old - Half, Old + Half, VW, VH));
	}
	// The ready pill: by the HUD's own rule, after a goal.
	TestFalse(TEXT("no pill at 0:0"), UJamDirectorsCallHudWidget::ShowReadyPill(true, 0, false, false));
	TestTrue(TEXT("the pill after a goal"), UJamDirectorsCallHudWidget::ShowReadyPill(true, 1, false, false));
	TestFalse(TEXT("no pill on the end card"), UJamDirectorsCallHudWidget::ShowReadyPill(true, 1, true, false));
	TestFalse(TEXT("no pill under the line"), UJamDirectorsCallHudWidget::ShowReadyPill(true, 1, false, true));
	TestFalse(TEXT("no pill when no call is ready"), UJamDirectorsCallHudWidget::ShowReadyPill(false, 3, false, false));
	TestTrue(TEXT("neutral: the pill from the first faceoff"), UJamDirectorsCallHudWidget::ShowReadyPill(true, 0, false, false, false));
	// The YOU tag: at the faceoff count only; the neutral value is A42's window.
	TestTrue(TEXT("tag while the faceoff counts"), UJamHudWidget::ShowYouTagLive(true, true, 100, -1));
	TestFalse(TEXT("no tag once the puck has dropped"), UJamHudWidget::ShowYouTagLive(true, false, 200, 180));
	TestTrue(TEXT("neutral: the tag 2 s after a drop"), UJamHudWidget::ShowYouTagLive(false, false, 1000, 950));
	// A net's label gives way in the middle third; the neutral value keeps it.
	TestFalse(TEXT("no net label in the middle third"), UJamHudWidget::ShowNetLabel(true, FVector2D(980, 400), FVector2D(1060, 430), VW, VH));
	TestTrue(TEXT("the label on the edge stays"), UJamHudWidget::ShowNetLabel(true, FVector2D(1700, 400), FVector2D(1780, 430), VW, VH));
	TestTrue(TEXT("neutral: the label stays"), UJamHudWidget::ShowNetLabel(false, FVector2D(980, 400), FVector2D(1060, 430), VW, VH));
	// The centre plate's two lines.
	TestEqual(TEXT("the period line"), UJamHudWidget::FormatPeriod(1), FString(TEXT("PERIOD 1")));
	TestEqual(TEXT("the rule line"), UJamHudWidget::FormatGoalRule(3), FString(TEXT("FIRST TO 3")));
	TestTrue(TEXT("no rule line without a rule"), UJamHudWidget::FormatGoalRule(0).IsEmpty());
	// The marker yields to the puck: a ghost while the puck is inside the chevron's box, no tag while it is inside the tag's.
	const FVector2D Tip = UJamHudWidget::YouMarkerTip(FVector2D(800, 500));
	TestEqual(TEXT("the tip stands the gap over the head"), Tip, FVector2D(800, 500 - UJamHudWidget::YouMarkerGapPx));
	FVector2D CMin, CMax, TMin, TMax;
	UJamHudWidget::YouChevronBox(Tip, CMin, CMax);
	UJamHudWidget::YouTagBox(Tip, TMin, TMax);
	TestTrue(TEXT("the tag stands on the chevron"), TMax.Y <= CMin.Y && TMin.Y < TMax.Y);
	TestTrue(TEXT("a puck inside the chevron's box"), UJamHudWidget::MarkerBoxHasPuck(CMin, CMax, FVector2D(805, 470)));
	TestTrue(TEXT("a puck at its edge counts (the pad)"), UJamHudWidget::MarkerBoxHasPuck(CMin, CMax, FVector2D(CMax.X + 8, 470)));
	TestFalse(TEXT("a puck beside it does not"), UJamHudWidget::MarkerBoxHasPuck(CMin, CMax, FVector2D(CMax.X + 30, 470)));
	TestFalse(TEXT("a puck at the feet does not"), UJamHudWidget::MarkerBoxHasPuck(CMin, CMax, FVector2D(800, 640)));
	TestEqual(TEXT("ghost over the puck"), UJamHudWidget::MarkerYieldAlpha(true), UJamHudWidget::MarkerGhostAlpha);
	TestEqual(TEXT("full otherwise"), UJamHudWidget::MarkerYieldAlpha(false), 1.f);
	return true;
}

// The two one-time prompts: when each is due, once, in order, never over the controls strip's window or a faceoff count (the caller's bPlayLive).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudPrompts, "Jam.Hud.Prompts", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudPrompts::RunTest(const FString&)
{
	using W = UJamHudWidget;
	TestEqual(TEXT("the brake prompt's words"), W::PromptText(W::EPrompt::Brake), FString(TEXT("S AGAINST YOUR MOTION: STOP, THEN GO")));
	TestEqual(TEXT("the kept puck's words"), W::PromptText(W::EPrompt::Kept), FString(TEXT("THE PUCK IS YOURS UNTIL SOMEONE PRESSES FOR IT")));
	TestTrue(TEXT("no words for none"), W::PromptText(W::EPrompt::None).IsEmpty());
	// The trigger: PromptDue(on, play live, human carries, nearest opponent m, closing, kept spent, brake spent, the puck-is-kept rule).
	TestTrue(TEXT("nothing without the puck"), W::PromptDue(true, true, false, 4.f, true, false, false, true) == W::EPrompt::None);
	TestTrue(TEXT("nothing outside live play (a faceoff count, the controls strip)"), W::PromptDue(true, false, true, 4.f, true, false, false, true) == W::EPrompt::None);
	TestTrue(TEXT("the first carry: the kept puck"), W::PromptDue(true, true, true, 30.f, false, false, false, true) == W::EPrompt::Kept);
	TestTrue(TEXT("kept spent, nobody near: nothing"), W::PromptDue(true, true, true, 30.f, false, true, false, true) == W::EPrompt::None);
	TestTrue(TEXT("a defender at 10.5 m closing: not yet"), W::PromptDue(true, true, true, 10.5f, true, true, false, true) == W::EPrompt::None);
	TestTrue(TEXT("a defender inside 10 m but not closing: not yet"), W::PromptDue(true, true, true, 6.f, false, true, false, true) == W::EPrompt::None);
	TestTrue(TEXT("a defender closing inside 10 m: the brake"), W::PromptDue(true, true, true, 9.9f, true, true, false, true) == W::EPrompt::Brake);
	TestTrue(TEXT("both spent: never again"), W::PromptDue(true, true, true, 3.f, true, true, true, true) == W::EPrompt::None);
	TestTrue(TEXT("a cartridge that does not keep the puck gets the brake prompt only"), W::PromptDue(true, true, true, 9.f, true, false, false, false) == W::EPrompt::Brake);
	TestTrue(TEXT("and no kept prompt"), W::PromptDue(true, true, true, 30.f, false, false, false, false) == W::EPrompt::None);
	TestTrue(TEXT("the neutral value: none"), W::PromptDue(false, true, true, 3.f, true, false, false, true) == W::EPrompt::None);
	TestEqual(TEXT("the range is the brief's 10 m"), W::BrakePromptRangeM, 10.f);
	// A session: frame by frame through the rule as the widget drives it (one prompt at a time, each spent when shown).
	{
		bool bKept = false, bBrake = false;
		int64 Shown = -1;
		W::EPrompt Now = W::EPrompt::None;
		int32 KeptShows = 0, BrakeShows = 0;
		int64 KeptAt = -1, BrakeAt = -1;
		float Last = -1.f;
		for (int64 Fr = 0; Fr < 3600; ++Fr)
		{
			const bool bCarry = (Fr >= 200 && Fr < 900) || (Fr >= 1500 && Fr < 2400) || Fr >= 3000;   // three carries
			const float Nearest = bCarry ? FMath::Max(1.5f, 14.f - 0.02f * static_cast<float>(Fr % 700)) : -1.f;   // a defender closing from 14 m
			const bool bClosing = bCarry && Last >= 0.f && Nearest < Last - 1e-3f;
			Last = bCarry ? Nearest : -1.f;
			if (Now == W::EPrompt::None || W::PromptOpacity(Shown, Fr) <= 0.f)
			{
				Now = W::PromptDue(true, true, bCarry, Nearest, bClosing, bKept, bBrake, true);
				if (Now == W::EPrompt::Kept) { bKept = true; Shown = Fr; ++KeptShows; KeptAt = Fr; }
				else if (Now == W::EPrompt::Brake) { bBrake = true; Shown = Fr; ++BrakeShows; BrakeAt = Fr; }
			}
		}
		TestEqual(TEXT("the kept puck shows once"), KeptShows, 1);
		TestEqual(TEXT("the brake shows once"), BrakeShows, 1);
		TestEqual(TEXT("the kept puck at the first carry's first frame"), KeptAt, static_cast<int64>(200));
		TestTrue(TEXT("the brake after the kept prompt has gone"), BrakeAt >= KeptAt + W::PromptFrames);
		AddInfo(FString::Printf(TEXT("JAMSCREEN prompts in a modelled session: kept at frame %lld, brake at frame %lld"), KeptAt, BrakeAt));
	}
	// The slot: under the scoreboard; one lower while a net badge or the puck's arrow (or the overlay's line) stands there.
	TestEqual(TEXT("the top slot"), W::PromptSlotFor(false), W::PromptSlotY);
	TestEqual(TEXT("the lower slot"), W::PromptSlotFor(true), W::PromptSlotLowY);
	TestTrue(TEXT("a far-net badge beside the scoreboard holds the top slot"), W::InPromptSlot(FVector2D(1155, 120), 1920.f, 330.f));
	TestFalse(TEXT("a badge on the side edge does not"), W::InPromptSlot(FVector2D(1840, 120), 1920.f, 330.f));
	TestFalse(TEXT("a marker lower in the frame does not"), W::InPromptSlot(FVector2D(960, 400), 1920.f, 330.f));
	// The hold and the fade.
	TestEqual(TEXT("full while held"), W::PromptOpacity(100, 100 + W::PromptFrames - W::PromptFadeFrames - 1), 1.f);
	TestEqual(TEXT("half way through the fade"), W::PromptOpacity(100, 100 + W::PromptFrames - W::PromptFadeFrames / 2), 0.5f, 1e-4f);
	TestEqual(TEXT("gone after its frames"), W::PromptOpacity(100, 100 + W::PromptFrames), 0.f);
	TestEqual(TEXT("never shown"), W::PromptOpacity(-1, 50), 0.f);
	// The session's memory (statics; restored).
	const bool bK = W::IsPromptSpent(W::EPrompt::Kept), bB = W::IsPromptSpent(W::EPrompt::Brake);
	W::SetPromptsSpent(true, false);
	TestTrue(TEXT("kept spent"), W::IsPromptSpent(W::EPrompt::Kept));
	TestFalse(TEXT("brake due"), W::IsPromptSpent(W::EPrompt::Brake));
	W::SetPromptsSpent(bK, bB);
	return true;
}

// The marker's tip against the helmet's top, from the projection, on every frame of 100 s (6000 frames) of a modelled human: skating a figure over the rink
// through the corners, leaning with speed, crouching, knocked down four times (falling over 12 frames to lie 1.2 m from the feet, getting up over 30).
// The model stands in for the figure's head socket (the real figure is measured by jam.hud.MarkerProbe in a game process: docs/experiments/SCREEN.md part 2).
// The rule of this lane keeps the tip the gap over the helmet on every frame; the rule of before (1.8 m over the feet) is measured beside it.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudMarkerFollowsHead, "Jam.Hud.MarkerFollowsHead", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudMarkerFollowsHead::RunTest(const FString&)
{
	const float L = 90.f, Wd = 45.f, R = 3.75f, PxW = 1280.f, PxH = 720.f, SlateToPx = PxH / 1080.f;
	FJamCameraSettings C;
	C.CornerLiftM = 3.f;
	FJamCameraFilter St;
	struct FDown { int32 Start, Frames; float DirDeg; };
	const FDown Downs[] = { { 900, 150, 30.f }, { 2400, 90, 200.f }, { 3900, 120, -80.f }, { 5200, 60, 120.f } };
	FVector2D Pos(-7.5, 0.0);
	float Yaw = 0.f;
	int32 Projected = 0, DownFrames = 0, NewOver = 0, OldOver = 0;
	float NewMax = 0.f, OldMaxUp = 0.f, OldMaxDown = 0.f;
	for (int32 Fr = 0; Fr < 6000; ++Fr)
	{
		// Down: the feet stand still, the head falls toward DirDeg and lies at 0.25 m; else skate the figure.
		float Fall = 0.f, FallDir = 0.f;
		for (const FDown& D : Downs)
		{
			if (Fr < D.Start || Fr >= D.Start + D.Frames) continue;
			const int32 T = Fr - D.Start, Left = D.Start + D.Frames - Fr;
			Fall = FMath::Min(1.f, FMath::Min(static_cast<float>(T) / 12.f, static_cast<float>(Left) / 30.f));
			FallDir = FMath::DegreesToRadians(D.DirDeg);
			++DownFrames;
		}
		float Speed = 0.f;
		if (Fall <= 0.f)
		{
			const double Tm = Fr / 60.0;
			const FVector2D Next(40.0 * FMath::Sin(0.21 * Tm + 0.4), 20.0 * FMath::Sin(0.34 * Tm));
			const FVector2D Step = Next - Pos;
			Speed = FMath::Min(11.f, static_cast<float>(Step.Size()) * 60.f);
			if (Speed > 0.5f) Yaw = FMath::RadiansToDegrees(FMath::Atan2(Step.Y, Step.X));
			Pos += Step.GetSafeNormal() * (Speed / 60.f);
		}
		const FTransform Cam = UJamFollowCamera::StepThirdPerson(C, St, JamA40Test::Frame(L, Wd, R, Pos, Yaw));
		// The helmet's top: 1.65 m upright; the lean with speed takes it up to 0.35 m forward and 0.2 m down (a crouch); down, it lies 1.2 m from the feet at 0.25 m.
		const float YawR = FMath::DegreesToRadians(Yaw), Lean = Speed / 11.f;
		const FVector Up(Pos.X * 100.0 + FMath::Cos(YawR) * 35.0 * Lean, Pos.Y * 100.0 + FMath::Sin(YawR) * 35.0 * Lean, 165.0 - 20.0 * Lean);
		const FVector Lying(Pos.X * 100.0 + FMath::Cos(FallDir) * 120.0, Pos.Y * 100.0 + FMath::Sin(FallDir) * 120.0, 25.0);
		const FVector Helmet = FMath::Lerp(Up, Lying, Fall);
		FVector2D HelmetPx, OldPx;
		if (!JamA5Test::Project(Cam, C.FovDeg, Helmet, HelmetPx, PxW, PxH) || !JamA5Test::Project(Cam, C.FovDeg, FVector(Pos.X * 100.0, Pos.Y * 100.0, UJamHudWidget::YouMarkerHeadM * 100.0), OldPx, PxW, PxH)) continue;
		++Projected;
		// The tip in pixels: the gap is Slate units (1080 high), the frame 720.
		const FVector2D Gap(0.f, -UJamHudWidget::YouMarkerGapPx * SlateToPx);
		const float NewD = static_cast<float>(FVector2D::Distance(HelmetPx + Gap, HelmetPx)), OldD = static_cast<float>(FVector2D::Distance(OldPx + Gap, HelmetPx));
		NewMax = FMath::Max(NewMax, NewD);
		if (Fall > 0.f) OldMaxDown = FMath::Max(OldMaxDown, OldD); else OldMaxUp = FMath::Max(OldMaxUp, OldD);
		if (NewD > 20.f) ++NewOver;
		if (OldD > 20.f) ++OldOver;
	}
	AddInfo(FString::Printf(TEXT("JAMSCREEN marker model: %d of 6000 frames projected, %d down; tip to the helmet's top at 1280 x 720: this lane %.1f px at most (0 frames over 20); the rule of before %.1f px upright, %.1f px down (%d frames over 20)"),
		Projected, DownFrames, NewMax, OldMaxUp, OldMaxDown, OldOver));
	TestEqual(TEXT("the human projects on every frame"), Projected, 6000);
	TestEqual(TEXT("four knockdowns were in the 100 s"), DownFrames, 150 + 90 + 120 + 60);
	TestEqual(TEXT("the tip within 20 px of the helmet's top on every frame"), NewOver, 0);
	TestTrue(TEXT("the tip stands the gap over the helmet"), NewMax <= UJamHudWidget::YouMarkerGapPx);
	TestTrue(TEXT("the rule of before leaves the helmet by more than 20 px when the human is down"), OldMaxDown > 20.f);
	return true;
}

#endif
