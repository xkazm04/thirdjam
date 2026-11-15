// Lane A214 (track U1, evolution 8): the game camera at the boards, measured the way the eye counts it (EYE LIST 1 item 5: "near the boards the
// screen is wood and crowd or a straight-down view of his helmet"). Pure StepThirdPerson, no world. The bar of briefs/A214 item 2:
//   ice covers at least 55 % of the frame in 98 % of stills, never under 40 %; the pitch never steeper than 55 degrees down; no flip of the yaw;
//   the camera's eye never outside the boards' inner line nor inside the glass, nothing but air between it and him; he, the puck and 3 m of ice
//   in his direction of travel on screen; in open ice (more than 8 m from every board) the camera of main, to the bit.
// "Before" is the game's camera on main (jam.camera.CornerLift 3, KeepHuman 0.31, KeepNoYaw 2, RigYaw 1, PitchCap 0); "after" adds jam.camera.Boards 1.
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "CameraFollow.h"
#include "HUDFeed.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamA214Test
{
	constexpr float Fov = 70.f, Aspect = 16.f / 9.f;

	FJamCameraSettings Before()
	{
		FJamCameraSettings C;
		C.CornerLiftM = 3.f; C.KeepHumanFrac = 0.31f; C.KeepNoYaw = 2; C.KeepStepM = 0.6f; C.bRigYaw = true; C.PitchCapDeg = 0.f;
		return C;
	}
	FJamCameraSettings After() { FJamCameraSettings C = Before(); C.BoardsView = 1.f; C.BoardsMinHeightM = 2.f; return C; }   // the game's jam.camera.BoardsMinHeight

	FJamPresentationFrame Frame(const FJamCameraRink& R, const FVector2D& PosM, float YawDeg, const FVector2D& PuckM, int32 AttackSign = 1)
	{
		FJamPresentationFrame F;
		F.RinkLengthM = R.LengthM;
		F.RinkWidthM = R.WidthM;
		F.CornerRadiusM = R.CornerRadiusM;
		F.AttackSign = AttackSign;
		F.HumanIndex = 0;
		F.Phase = EJamPhase::Play;
		F.PuckM = PuckM;
		F.Skaters.Add(FJamPresentationSkater{ PosM, YawDeg, 0, true });
		return F;
	}

	// Metres from P to the nearest board's line (the straight lines; the corner arcs are further).
	double BoardDist(const FJamCameraRink& R, const FVector2D& P)
	{
		return FMath::Min(FMath::Min(R.LengthM * 0.5 - P.X, R.LengthM * 0.5 + P.X), FMath::Min(R.WidthM * 0.5 - P.Y, R.WidthM * 0.5 + P.Y));
	}

	bool OnScreen(const FTransform& T, const FVector& WorldCm, float Edge = 1.f)
	{
		const FVector2D F = UJamFollowCamera::FrameFraction(T, Fov, Aspect, WorldCm);
		return FMath::Abs(F.X) <= Edge && FMath::Abs(F.Y) <= Edge;
	}

	// The segment camera -> chest against both nets (a box 4 m wide, 1.1 m deep behind the goal line, 1.2 m high; the goal line 4 m from the end board).
	bool NetBetween(const FJamCameraRink& R, const FVector& A, const FVector& B)
	{
		for (int32 Side : { -1, 1 })
		{
			const double Gx = Side * (R.LengthM * 0.5 - 4.0);
			const FVector Lo(FMath::Min(Gx, Gx + Side * 1.1) * 100.0, -200.0, 0.0), Hi(FMath::Max(Gx, Gx + Side * 1.1) * 100.0, 200.0, 120.0);
			double T0 = 0.0, T1 = 1.0;
			bool bHit = true;
			for (int32 k = 0; k < 3 && bHit; ++k)
			{
				const double D = B[k] - A[k];
				if (FMath::Abs(D) < 1e-9) { if (A[k] < Lo[k] || A[k] > Hi[k]) bHit = false; continue; }
				double Ta = (Lo[k] - A[k]) / D, Tb = (Hi[k] - A[k]) / D;
				if (Ta > Tb) Swap(Ta, Tb);
				T0 = FMath::Max(T0, Ta); T1 = FMath::Min(T1, Tb);
				if (T0 > T1) bHit = false;
			}
			if (bHit) return true;
		}
		return false;
	}

	struct FRow
	{
		float Ice = 0.f, Down = 0.f;
		bool bHim = false, bChest = false, bAhead = false, bPuck = true, bInside = false, bAir = true;
	};
	FRow Look(const FTransform& T, const FJamCameraRink& R, const FVector2D& HumanM, const FVector2D& TravelDir, const FVector2D& PuckM, bool bPuckNear)
	{
		FRow Row;
		Row.Ice = UJamFollowCamera::IceShare(T, Fov, Aspect, R, 32, 18);
		Row.Down = -(float)T.Rotator().Pitch;
		const FVector Chest(HumanM.X * 100.0, HumanM.Y * 100.0, 100.0);
		Row.bChest = OnScreen(T, Chest);
		Row.bHim = Row.bChest || OnScreen(T, FVector(HumanM.X * 100.0, HumanM.Y * 100.0, 180.0)) || OnScreen(T, FVector(HumanM.X * 100.0, HumanM.Y * 100.0, 20.0));
		const FVector2D A = HumanM + TravelDir.GetSafeNormal() * 3.0;
		Row.bAhead = TravelDir.IsNearlyZero() || OnScreen(T, FVector(A.X * 100.0, A.Y * 100.0, 0.0));
		if (bPuckNear) Row.bPuck = OnScreen(T, FVector(PuckM.X * 100.0, PuckM.Y * 100.0, 5.0));
		const FVector Loc = T.GetLocation();
		// The eye never outside the inner line, and never in the glass (0.3 m: a camera's near plane and the glass' thickness).
		Row.bInside = UJamFollowCamera::IsInsideBoards(FVector2D(Loc.X / 100.0, Loc.Y / 100.0), R, 0.3f);
		Row.bAir = !NetBetween(R, Loc, Chest);
		return Row;
	}

	struct FTable
	{
		int32 N = 0, Ice55 = 0, Ice40 = 0, Steep55 = 0, Steep60 = 0, HimOff = 0, ChestOff = 0, AheadOff = 0, PuckN = 0, PuckOff = 0, Outside = 0, NetBlocks = 0;
		float MinIce = 1.f, MaxDown = -90.f, SumIce = 0.f;
		void Add(const FRow& Rw, bool bPuckNear)
		{
			++N; SumIce += Rw.Ice; MinIce = FMath::Min(MinIce, Rw.Ice); MaxDown = FMath::Max(MaxDown, Rw.Down);
			Ice55 += Rw.Ice < 0.55f; Ice40 += Rw.Ice < 0.40f; Steep55 += Rw.Down > 55.01f; Steep60 += Rw.Down > 60.f;
			HimOff += !Rw.bHim; ChestOff += !Rw.bChest; AheadOff += !Rw.bAhead; Outside += !Rw.bInside; NetBlocks += !Rw.bAir;
			if (bPuckNear) { ++PuckN; PuckOff += !Rw.bPuck; }
		}
		static FString Pc(int32 A, int32 B) { return FString::Printf(TEXT("%.1f %%"), B ? 100.0 * A / B : 0.0); }
		FString Line() const
		{
			return FString::Printf(TEXT("n %d | ice < 55 %%: %s | ice < 40 %%: %s | min ice %.2f, mean %.2f | pitch > 55: %s (> 60: %s), steepest %.1f | him off %s (chest off %s) | 3 m ahead off %s | puck off %s of %d | eye outside the line or in the glass %d | net between %d"),
				N, *Pc(Ice55, N), *Pc(Ice40, N), MinIce, N ? SumIce / N : 0.f, *Pc(Steep55, N), *Pc(Steep60, N), MaxDown, *Pc(HimOff, N), *Pc(ChestOff, N), *Pc(AheadOff, N), *Pc(PuckOff, PuckN), PuckN, Outside, NetBlocks);
		}
	};

	// The positions of the sweep: rounded rectangles D metres inside the boards' line, every Step metres of their perimeter.
	void Ring(const FJamCameraRink& R, double D, double Step, TArray<FVector2D>& Out)
	{
		const double Hx = R.LengthM * 0.5 - D, Hy = R.WidthM * 0.5 - D;
		const double Per = 4.0 * (Hx + Hy);
		const int32 N = FMath::Max(4, (int32)(Per / Step));
		for (int32 k = 0; k < N; ++k)
		{
			const double T = Per * k / N;
			FVector2D P;
			if (T < 2 * Hx) P = FVector2D(-Hx + T, Hy);
			else if (T < 2 * Hx + 2 * Hy) P = FVector2D(Hx, Hy - (T - 2 * Hx));
			else if (T < 4 * Hx + 2 * Hy) P = FVector2D(Hx - (T - 2 * Hx - 2 * Hy), -Hy);
			else P = FVector2D(-Hx, -Hy + (T - 4 * Hx - 2 * Hy));
			Out.Add(UJamFollowCamera::NearestInsideBoards(P, R, (float)D));
		}
	}

	// A135's modelled skater (the generator of Jam.Camera.KeepsHuman and BoundsCorrections, the same seeds): a path that hugs the boards.
	void Path(const FJamCameraRink& Rink, int32 Seed, TArray<FVector2D>& OutPos, TArray<float>& OutYaw)
	{
		FRandomStream Rng(Seed);
		const float L = Rink.LengthM, W = Rink.WidthM;
		FVector2D Pos(-7.5, 0.0), Vel(0.0, 0.0), Target(0.0, 0.0);
		float Yaw = 0.f, WantYaw = 0.f, WantSpeed = 0.f;
		int32 Hold = 0, Mode = 0;
		for (int32 i = 0; i < 6000; ++i)
		{
			if (i > 0 && i % 1500 == 0) { Pos = FVector2D(-7.5, 0.0); Vel = FVector2D::ZeroVector; Yaw = 0.f; Hold = 0; }
			if (--Hold <= 0)
			{
				Mode = Rng.RandRange(0, 9);
				Hold = Rng.RandRange(30, 150);
				const float Side = Rng.FRand() < 0.5f ? -1.f : 1.f;
				if (Mode <= 2) Target = FVector2D(Rng.FRandRange(-L * 0.5f, L * 0.5f), Side * W * 0.5f);
				else if (Mode <= 4) Target = FVector2D(Side * L * 0.5f, (Rng.FRand() < 0.5f ? -1.f : 1.f) * W * 0.5f);
				else if (Mode == 5) Target = FVector2D(Side * L * 0.5f, Rng.FRandRange(-W * 0.5f, W * 0.5f));
				else if (Mode <= 7) Target = FVector2D(0.0, 0.0);
				else if (Mode == 8) { WantYaw = Yaw + 180.f; }
				else { const float A = Rng.FRandRange(0.f, 2.f * PI); Vel += FVector2D(FMath::Cos(A), FMath::Sin(A)) * 6.0; }
				WantSpeed = (Mode == 6 || Mode == 7 || Mode == 8) ? 0.f : Rng.FRandRange(5.f, 12.65f);
			}
			if (Mode != 8 && Mode != 9)
			{
				const FVector2D To = Target - Pos;
				if (To.Size() > 0.5) WantYaw = FMath::RadiansToDegrees((float)FMath::Atan2(To.Y, To.X));
			}
			Yaw = FMath::UnwindDegrees(Yaw + FMath::Clamp(FMath::UnwindDegrees(WantYaw - Yaw), -4.77f, 4.77f));
			const FVector2D Fwd(FMath::Cos(FMath::DegreesToRadians(Yaw)), FMath::Sin(FMath::DegreesToRadians(Yaw)));
			const FVector2D Dv = Fwd * WantSpeed - Vel;
			const double Step = WantSpeed > 0.f ? 12.0 / 60.0 : 20.0 / 60.0;
			Vel += Dv.Size() > Step ? Dv.GetSafeNormal() * Step : Dv;
			Pos += Vel / 60.0;
			const FVector2D In = UJamFollowCamera::NearestInsideBoards(Pos, Rink, 0.45f);
			if (!In.Equals(Pos, 1e-9)) { Vel *= 0.5; Pos = In; }
			OutPos.Add(Pos); OutYaw.Add(Yaw);
		}
	}

	// The staged scenes of the brief, one path: he carries along the side board, into a corner, behind the net, stands in the corner facing out, skates
	// out and along the end board; then the same on the other side, mirrored. 9 m/s at most, turning at the sim's 4.77 degrees a frame.
	void Staged(const FJamCameraRink& Rink, TArray<FVector2D>& OutPos, TArray<float>& OutYaw)
	{
		const double Hx = Rink.LengthM * 0.5, Hy = Rink.WidthM * 0.5;
		for (int32 Mirror : { 1, -1 })
		{
			const FVector2D Way[] = { FVector2D(-10, Mirror * (Hy - 1.2)), FVector2D(Hx - 6, Mirror * (Hy - 1.2)), FVector2D(Hx - 1.2, Mirror * (Hy - 3)), FVector2D(Hx - 1.5, 0),
				FVector2D(Hx - 1.2, -Mirror * (Hy - 4)), FVector2D(Hx - 2.0, -Mirror * (Hy - 2.0)), FVector2D(Hx - 15, -Mirror * 5), FVector2D(-Hx + 12, Mirror * 2),
				FVector2D(-Hx + 1.2, Mirror * (Hy - 4)), FVector2D(-Hx + 2.2, -Mirror * 3), FVector2D(0, -Mirror * (Hy - 1.2)) };
			FVector2D Pos = Way[0];
			float Yaw = 0.f;
			for (int32 w = 1; w < UE_ARRAY_COUNT(Way); ++w)
			{
				for (int32 i = 0; i < 900; ++i)
				{
					const FVector2D To = Way[w] - Pos;
					if (To.Size() < 0.3) break;
					const float WantYaw = FMath::RadiansToDegrees((float)FMath::Atan2(To.Y, To.X));
					Yaw = FMath::UnwindDegrees(Yaw + FMath::Clamp(FMath::UnwindDegrees(WantYaw - Yaw), -4.77f, 4.77f));
					const double Speed = FMath::Min(9.0, 1.0 + To.Size() * 2.0);
					Pos += FVector2D(FMath::Cos(FMath::DegreesToRadians(Yaw)), FMath::Sin(FMath::DegreesToRadians(Yaw))) * (Speed / 60.0);
					Pos = UJamFollowCamera::NearestInsideBoards(Pos, Rink, 0.45f);
					OutPos.Add(Pos); OutYaw.Add(Yaw);
				}
				// stand there a second, turning round to face out (the corner, behind the net)
				for (int32 i = 0; i < 60; ++i)
				{
					const FVector2D ToC = -Pos;
					const float WantYaw = FMath::RadiansToDegrees((float)FMath::Atan2(ToC.Y, ToC.X));
					Yaw = FMath::UnwindDegrees(Yaw + FMath::Clamp(FMath::UnwindDegrees(WantYaw - Yaw), -4.77f, 4.77f));
					OutPos.Add(Pos); OutYaw.Add(Yaw);
				}
			}
		}
	}

	struct FDyn
	{
		FTable Tab;
		int32 YawDiffers = 0, FlipFrames = 0, OpenIce = 0, OpenDiffers = 0, OpenNotRig = 0;
		float MaxStepCm = 0.f, MaxPitchStep = 0.f, MaxYaw30 = 0.f, MaxOpenPx = 0.f;
		FString WorstStep;
		TArray<FString> AheadWhere;
		double Seconds = 0.0;
	};
	// Runs a path through both cameras; the after camera's table, and what it does that the before camera does not.
	void RunPath(const FJamCameraRink& R, const TArray<FVector2D>& Pos, const TArray<float>& Yaw, FDyn& OutBefore, FDyn& OutAfter, TArray<FString>* Strip = nullptr, int32 StripFrom = 0, int32 StripTo = 0)
	{
		const FJamCameraSettings Cb = Before(), Ca = After();
		FJamCameraFilter Sb, Sa;
		TArray<float> YawsA, YawsB;
		FTransform PrevA, PrevB;
		for (int32 i = 0; i < Pos.Num(); ++i)
		{
			const FVector2D Travel = i > 0 ? Pos[i] - Pos[i - 1] : FVector2D::ZeroVector;
			const FVector2D Dir = Travel.Size() > 0.5 / 60.0 ? Travel : FVector2D::ZeroVector;   // under 0.5 m/s he stands: no direction of travel
			const FVector2D Puck = Pos[i] + FVector2D(FMath::Cos(FMath::DegreesToRadians(Yaw[i])), FMath::Sin(FMath::DegreesToRadians(Yaw[i]))) * 0.9;   // on his blade
			const FJamPresentationFrame F = Frame(R, Pos[i], Yaw[i], Puck);
			const FTransform Tb = UJamFollowCamera::StepThirdPerson(Cb, Sb, F);
			const FTransform Ta = UJamFollowCamera::StepThirdPerson(Ca, Sa, F);
			OutBefore.Tab.Add(Look(Tb, R, Pos[i], Dir, Puck, true), true);
			const FRow RowA = Look(Ta, R, Pos[i], Dir, Puck, true);
			OutAfter.Tab.Add(RowA, true);
			if (!RowA.bAhead && OutAfter.AheadWhere.Num() < 12)
			{
				const double DirYaw = FMath::RadiansToDegrees(FMath::Atan2(Dir.Y, Dir.X));
				const FVector2D Fa = UJamFollowCamera::FrameFraction(Ta, Fov, Aspect, FVector((Pos[i].X + Dir.GetSafeNormal().X * 3.0) * 100.0, (Pos[i].Y + Dir.GetSafeNormal().Y * 3.0) * 100.0, 0.0));
				OutAfter.AheadWhere.Add(FString::Printf(TEXT("f%d pos (%.1f, %.1f) board %.1f m | facing %.0f, travel %.0f (off by %.0f) | ahead point at (%.2f, %.2f) | before on %d"), i, Pos[i].X, Pos[i].Y, BoardDist(R, Pos[i]),
					Yaw[i], DirYaw, FMath::Abs(FMath::UnwindDegrees((float)DirYaw - Yaw[i])), Fa.X, Fa.Y, Look(Tb, R, Pos[i], Dir, Puck, true).bAhead ? 1 : 0));
			}
			OutAfter.YawDiffers += FMath::Abs(FMath::UnwindDegrees(Ta.Rotator().Yaw - Tb.Rotator().Yaw)) > 1e-3f;
			for (int32 k = 0; k < 2; ++k)
			{
				FDyn& D = k ? OutAfter : OutBefore;
				const FTransform& T = k ? Ta : Tb;
				const FTransform& Pv = k ? PrevA : PrevB;
				TArray<float>& Ys = k ? YawsA : YawsB;
				Ys.Add((float)T.Rotator().Yaw);
				if (i > 0 && FVector2D::Distance(Pos[i], Pos[i - 1]) < 1.0)   // not across a teleport (the filter snaps there by A49's rule)
				{
					const float StepCm = (float)FVector::Dist(T.GetLocation(), Pv.GetLocation());
					if (StepCm > D.MaxStepCm) { D.MaxStepCm = StepCm; D.WorstStep = FString::Printf(TEXT("f%d pos (%.2f, %.2f) yaw %.1f prev (%.2f, %.2f) | cam (%.2f, %.2f, %.2f) <- (%.2f, %.2f, %.2f)"), i, Pos[i].X, Pos[i].Y, Yaw[i], Pos[i - 1].X, Pos[i - 1].Y,
						T.GetLocation().X / 100.0, T.GetLocation().Y / 100.0, T.GetLocation().Z / 100.0, Pv.GetLocation().X / 100.0, Pv.GetLocation().Y / 100.0, Pv.GetLocation().Z / 100.0); }
					D.MaxPitchStep = FMath::Max(D.MaxPitchStep, FMath::Abs((float)(T.Rotator().Pitch - Pv.Rotator().Pitch)));
				}
				if (i >= 30)
				{
					const float Y30 = FMath::Abs(FMath::UnwindDegrees(Ys[i] - Ys[i - 30]));
					const float Own30 = FMath::Abs(FMath::UnwindDegrees(Yaw[i] - Yaw[i - 30]));
					D.MaxYaw30 = FMath::Max(D.MaxYaw30, Y30 - Own30);   // the camera's own turn over his in 30 frames
					D.FlipFrames += Y30 > 90.f && Y30 > Own30 + 10.f;
				}
			}
			PrevA = Ta; PrevB = Tb;
			if (BoardDist(R, Pos[i]) > 8.0 && BoardDist(R, Sa.PivotM) > 8.0)
			{
				++OutAfter.OpenIce;
				if (!Ta.Equals(Tb, 1e-4)) ++OutAfter.OpenDiffers;
				if (!Ta.Equals(Sa.LastRigPose, 1e-4)) ++OutAfter.OpenNotRig;
				// How far apart in pixels (1920 wide) the two cameras put eight ice points round him.
				for (int32 a = 0; a < 8; ++a)
				{
					const double Ang = a * PI / 4;
					const FVector Wp((Pos[i].X + 3.0 * FMath::Cos(Ang)) * 100.0, (Pos[i].Y + 3.0 * FMath::Sin(Ang)) * 100.0, 0.0);
					const FVector2D Fa = UJamFollowCamera::FrameFraction(Ta, Fov, Aspect, Wp), Fb = UJamFollowCamera::FrameFraction(Tb, Fov, Aspect, Wp);
					if (FMath::Abs(Fa.X) < 2 && FMath::Abs(Fb.X) < 2) OutAfter.MaxOpenPx = FMath::Max(OutAfter.MaxOpenPx, (float)((Fa - Fb).Size() * 960.0));
				}
			}
			if (Strip && i >= StripFrom && i < StripTo)
			{
				const FRow Ra = Look(Ta, R, Pos[i], Dir, Puck, true);
				Strip->Add(FString::Printf(TEXT("JAMBOARDS strip f%d pos (%.2f, %.2f) yaw %.1f board %.2f m | before: ice %.2f down %.1f h %.2f | after: ice %.2f down %.1f h %.2f cam (%.2f, %.2f) w %.2f him %d ahead %d"),
					i, Pos[i].X, Pos[i].Y, Yaw[i], BoardDist(R, Pos[i]), UJamFollowCamera::IceShare(Tb, Fov, Aspect, R, 32, 18), -Tb.Rotator().Pitch, Tb.GetLocation().Z / 100.0,
					Ra.Ice, Ra.Down, Ta.GetLocation().Z / 100.0, Ta.GetLocation().X / 100.0, Ta.GetLocation().Y / 100.0, Sa.LastBoardsWeight, Ra.bHim ? 1 : 0, Ra.bAhead ? 1 : 0));
			}
		}
	}
}

// The sweep: he stands (the camera settled) at every place along the boards' line from 0.5 to 7 m in, facing every 15 degrees; and the bar on it.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCameraBoardsSweep, "Jam.Camera.BoardsSweep", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamCameraBoardsSweep::RunTest(const FString&)
{
	using namespace JamA214Test;
	TestEqual(TEXT("the boards view is off in the settings (the neutral value: main's camera)"), FJamCameraSettings().BoardsView, 0.f);
	struct FRk { const TCHAR* Name; float L, W, R; };
	for (const FRk& K : { FRk{ TEXT("regular 90 x 45"), 90.f, 45.f, 3.75f }, FRk{ TEXT("large 120 x 60"), 120.f, 60.f, 5.f } })
	{
		FJamCameraRink R; R.LengthM = K.L; R.WidthM = K.W; R.CornerRadiusM = K.R;
		FTable Tb, Ta;
		TMap<int32, FTable> ByD;
		for (double D : { 0.5, 1.0, 2.0, 3.0, 5.0, 7.0 })
		{
			TArray<FVector2D> Ps;
			Ring(R, D, 4.0, Ps);
			for (const FVector2D& P : Ps)
			{
				for (int32 Yd = -180; Yd < 180; Yd += 15)
				{
					const float Y = (float)Yd;
					const FVector2D Puck = P + FVector2D(FMath::Cos(FMath::DegreesToRadians(Y)), FMath::Sin(FMath::DegreesToRadians(Y))) * 0.9;
					const FJamPresentationFrame F = Frame(R, P, Y, Puck);
					FJamCameraFilter Sb, Sa;
					FTransform Pb, Pa;
					for (int32 i = 0; i < 60; ++i) { Pb = UJamFollowCamera::StepThirdPerson(Before(), Sb, F); Pa = UJamFollowCamera::StepThirdPerson(After(), Sa, F); }
					const FVector2D Facing(FMath::Cos(FMath::DegreesToRadians(Y)), FMath::Sin(FMath::DegreesToRadians(Y)));   // carrying: he skates where he faces
					Tb.Add(Look(Pb, R, P, Facing, Puck, true), true);
					const FRow Ra = Look(Pa, R, P, Facing, Puck, true);
					static int32 Dumped = 0;
					if ((!Ra.bHim || !Ra.bPuck) && Dumped < 40 && (Yd % 45) == 0)
					{
						++Dumped;
						const FVector2D Fc = UJamFollowCamera::FrameFraction(Pa, Fov, Aspect, FVector(P.X * 100.0, P.Y * 100.0, 100.0));
						const FVector2D Fh = UJamFollowCamera::FrameFraction(Pa, Fov, Aspect, FVector(P.X * 100.0, P.Y * 100.0, 180.0));
						const FVector2D Fp = UJamFollowCamera::FrameFraction(Pa, Fov, Aspect, FVector(Puck.X * 100.0, Puck.Y * 100.0, 5.0));
						AddInfo(FString::Printf(TEXT("JAMBOARDS off %s d %.1f pos (%.2f, %.2f) yaw %d | cam (%.2f, %.2f, %.2f) pitch %.1f yaw %.1f | chest (%.2f, %.2f) head (%.2f, %.2f) puck (%.2f, %.2f) w %.2f"),
							K.Name, D, P.X, P.Y, Yd, Pa.GetLocation().X / 100.0, Pa.GetLocation().Y / 100.0, Pa.GetLocation().Z / 100.0, Pa.Rotator().Pitch, Pa.Rotator().Yaw, Fc.X, Fc.Y, Fh.X, Fh.Y, Fp.X, Fp.Y, Sa.LastBoardsWeight));
					}
					Ta.Add(Ra, true);
					ByD.FindOrAdd((int32)(D * 10)).Add(Ra, true);
				}
			}
		}
		AddInfo(FString::Printf(TEXT("JAMBOARDS sweep %s BEFORE | %s"), K.Name, *Tb.Line()));
		AddInfo(FString::Printf(TEXT("JAMBOARDS sweep %s AFTER  | %s"), K.Name, *Ta.Line()));
		for (const TPair<int32, FTable>& It : ByD) AddInfo(FString::Printf(TEXT("JAMBOARDS sweep %s AFTER at %.1f m | %s"), K.Name, It.Key / 10.0, *It.Value.Line()));
		TestTrue(FString::Printf(TEXT("%s: the test has teeth (before, ice < 55 %% in %d of %d)"), K.Name, Tb.Ice55, Tb.N), Tb.Ice55 * 10 > Tb.N);
		TestTrue(FString::Printf(TEXT("%s: ice >= 55 %% in 98 %% of poses (%d under)"), K.Name, Ta.Ice55), Ta.Ice55 * 50 <= Ta.N);
		TestTrue(FString::Printf(TEXT("%s: ice never under 40 %% (min %.2f)"), K.Name, Ta.MinIce), Ta.MinIce >= 0.40f);
		TestEqual(FString::Printf(TEXT("%s: poses looking down steeper than 55 degrees"), K.Name), Ta.Steep55, 0);
		TestEqual(FString::Printf(TEXT("%s: poses with the eye outside the inner line or in the glass"), K.Name), Ta.Outside, 0);
		TestEqual(FString::Printf(TEXT("%s: poses without him on screen"), K.Name), Ta.HimOff, 0);
		TestTrue(FString::Printf(TEXT("%s: 3 m ahead of him on screen in 95 %% (%d off)"), K.Name, Ta.AheadOff), Ta.AheadOff * 20 <= Ta.N);
	}
	return true;
}

// The paths: A135's boards-hugging skater (two rinks), the staged scenes (corners, behind both nets, along both side boards, standing in a corner facing
// out), and the 120-frame strip from open ice into a corner and out. Per frame both cameras; the bar, no flip, no pop, the yaw of before to the bit.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCameraBoardsPaths, "Jam.Camera.BoardsPaths", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamCameraBoardsPaths::RunTest(const FString&)
{
	using namespace JamA214Test;
	struct FCase { const TCHAR* Name; float L, W, R; int32 Seed; };
	for (const FCase& K : { FCase{ TEXT("regular, A135 path"), 90.f, 45.f, 3.75f, 135 }, FCase{ TEXT("large, A135 path"), 120.f, 60.f, 5.f, 137 },
		FCase{ TEXT("regular, staged scenes"), 90.f, 45.f, 3.75f, 0 }, FCase{ TEXT("large, staged scenes"), 120.f, 60.f, 5.f, 0 } })
	{
		FJamCameraRink R; R.LengthM = K.L; R.WidthM = K.W; R.CornerRadiusM = K.R;
		TArray<FVector2D> Pos; TArray<float> Yaw;
		if (K.Seed) Path(R, K.Seed, Pos, Yaw); else Staged(R, Pos, Yaw);
		FDyn B, A;
		TArray<FString> Strip;
		RunPath(R, Pos, Yaw, B, A, K.Seed == 0 && K.L < 100.f ? &Strip : nullptr, 0, 240);
		for (const FString& S : Strip) AddInfo(S);
		for (const FString& S : A.AheadWhere) AddInfo(FString::Printf(TEXT("JAMBOARDS ahead off %s: %s"), K.Name, *S));
		// The camera's own cost: the same path through each camera alone, timed (the frame time's share of the camera).
		for (int32 k = 0; k < 2; ++k)
		{
			const FJamCameraSettings Cf = k ? After() : Before();
			FJamCameraFilter St;
			FTransform Sink;
			const double T0 = FPlatformTime::Seconds();
			for (int32 i = 0; i < Pos.Num(); ++i) Sink = UJamFollowCamera::StepThirdPerson(Cf, St, Frame(R, Pos[i], Yaw[i], Pos[i]));
			(k ? A : B).Seconds = FPlatformTime::Seconds() - T0;
			if (Sink.GetLocation().Z < -1e9) AddInfo(TEXT("."));
		}
		AddInfo(FString::Printf(TEXT("JAMBOARDS cost %s: StepThirdPerson %.2f us a frame before, %.2f us after (%d frames)"), K.Name, B.Seconds * 1e6 / Pos.Num(), A.Seconds * 1e6 / Pos.Num(), Pos.Num()));
		for (int32 k = 0; k < 2; ++k)
		{
			const FDyn& D = k ? A : B;
			AddInfo(FString::Printf(TEXT("JAMBOARDS path %s %s | %s | flips %d, camera's own turn in 30 frames %.1f deg | largest step %.0f cm, pitch step %.2f deg"),
				K.Name, k ? TEXT("AFTER ") : TEXT("BEFORE"), *D.Tab.Line(), D.FlipFrames, D.MaxYaw30, D.MaxStepCm, D.MaxPitchStep));
			AddInfo(FString::Printf(TEXT("JAMBOARDS worst step %s %s: %s"), K.Name, k ? TEXT("AFTER ") : TEXT("BEFORE"), *D.WorstStep));
		}
		AddInfo(FString::Printf(TEXT("JAMBOARDS path %s open ice | frames %d | after != before %d (worst %.2f px at 1920) | after != the open-ice rig %d"), K.Name, A.OpenIce, A.OpenDiffers, A.MaxOpenPx, A.OpenNotRig));
		TestEqual(FString::Printf(TEXT("%s: frames whose yaw is not the camera of before's (W and the blade read it)"), K.Name), A.YawDiffers, 0);
		TestEqual(FString::Printf(TEXT("%s: frames looking down steeper than 55 degrees"), K.Name), A.Tab.Steep55, 0);
		TestEqual(FString::Printf(TEXT("%s: frames with the eye outside the inner line or in the glass"), K.Name), A.Tab.Outside, 0);
		TestTrue(FString::Printf(TEXT("%s: ice >= 55 %% in 98 %% of frames (%d of %d under)"), K.Name, A.Tab.Ice55, A.Tab.N), A.Tab.Ice55 * 50 <= A.Tab.N);
		TestTrue(FString::Printf(TEXT("%s: ice never under 40 %% (min %.2f)"), K.Name, A.Tab.MinIce), A.Tab.MinIce >= 0.40f);
		TestTrue(FString::Printf(TEXT("%s: no flip the camera of before did not make (%d, before %d: the rig's lag behind his own half-turns)"), K.Name, A.FlipFrames, B.FlipFrames), A.FlipFrames <= B.FlipFrames);
		TestTrue(FString::Printf(TEXT("%s: 3 m ahead on screen in 95 %% (%d off)"), K.Name, A.Tab.AheadOff), A.Tab.AheadOff * 20 <= A.Tab.N);
		TestTrue(FString::Printf(TEXT("%s: him on screen in 99.5 %% of frames (%d off; the step limit's guard catches him within a frame or two)"), K.Name, A.Tab.HimOff), A.Tab.HimOff * 200 <= A.Tab.N);
		TestEqual(FString::Printf(TEXT("%s: open ice, frames off the open-ice rig"), K.Name), A.OpenNotRig, 0);
		TestTrue(FString::Printf(TEXT("%s: no pop: no step over main's bound of 1 m a frame (largest %.0f cm, before %.0f)"), K.Name, A.MaxStepCm, B.MaxStepCm), A.MaxStepCm <= FMath::Max(100.f, B.MaxStepCm));
	}
	return true;
}

#endif
