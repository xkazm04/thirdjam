// Lane A196 (track F3, evolution 9): the hits read (HitRead.h, docs/experiments/HITS.md). Presentation only.
#include "HitRead.h"
#include "JamPawns.h"
#include "JamEventBus.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "HAL/IConsoleManager.h"
#include "UnrealClient.h"
#include "Misc/AutomationTest.h"

DEFINE_LOG_CATEGORY_STATIC(LogJamHit, Log, All);

static TAutoConsoleVariable<int32> CVarJamHitTell(TEXT("jam.hit.Tell"), 1,
	TEXT("The tell before a hit (lane A196): 1 (default) a chevron of light on the ice 1.4 to 2.4 m ahead of a body checker through his wind-up and window (grey out on a miss), a streak across a cross-checker's stick, the held brace's streak and arc; 0 none (before). Presentation only."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitLean(TEXT("jam.hit.Lean"), 1,
	TEXT("The figure's lean through a check (lane A196): 1 (default) while the figure lacks device B's PlayCheckWindup / PlayCrossCheck / SetCrossCheckHold: back over the wind-up, forward through the window or the brace; 2 always; 0 none (before). Presentation only."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitBurst(TEXT("jam.hit.Burst"), 1,
	TEXT("The burst at contact (lane A196): 1 (default) a ring on the ice sized by the closing speed and shards away from the victim's fall; 0 none (before). Presentation only."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitDown(TEXT("jam.hit.Down"), 1,
	TEXT("The downed skater's countdown (lane A196): 1 (default) a ring of 12 bars on the ice round him that go out as his down frames run; 0 none (before). Presentation only."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitPuckFlash(TEXT("jam.hit.PuckFlash"), 1,
	TEXT("The stripped puck's flash (lane A196): 1 (default) a gold ring out from the puck a stick lift took; 0 none (before). Presentation only."), ECVF_Default);
static TAutoConsoleVariable<float> CVarJamHitScale(TEXT("jam.hit.Scale"), 1.f,
	TEXT("Size factor of the hits' marks (lane A196, tuning in a window): 1 (default)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitCapture(TEXT("jam.hit.Capture"), 0,
	TEXT("Proof (lane A196): N > 0 takes a still every sim frame from each of the human's presses (check, cross, brace) to 30 frames after it landed or missed, N strips a kind (run with a fixed 60 Hz step). 0 = off."), ECVF_Default);

// ---------------------------------------------------------------- the pure part

double FJamHitReadModel::BurstRadiusM(double ClosingMps, bool bBraced)
{
	const double C = FMath::Clamp(ClosingMps, 0.0, 14.0);
	const double R = 1.2 + (3.0 - 1.2) * (C / 14.0);
	return bBraced ? R * 1.2 : R;
}

bool FJamHitReadModel::ShardAllowed(const FVector2D& ShardDir, const FVector2D& FallDir)
{
	if (FallDir.IsNearlyZero()) return true;
	const FVector2D A = ShardDir.GetSafeNormal(), B = FallDir.GetSafeNormal();
	return FVector2D::DotProduct(A, B) < FMath::Cos(FMath::DegreesToRadians(55.0));
}

int32 FJamHitReadModel::DownBarsLit(int32 FramesLeft, int32 Total)
{
	if (FramesLeft <= 0 || Total <= 0) return 0;
	return FMath::Clamp(FMath::CeilToInt(static_cast<double>(DownSegments) * FramesLeft / Total), 1, DownSegments);
}

float FJamHitReadModel::CheckLeanDeg(int32 Since, int32 Windup, int32 Window, int32 EndedAt)
{
	// + = the top of the figure back. Loading over the wind-up (to +6), the drive through the window (-12, reached in 3 frames), upright again over
	// LeanBackFrames from the end (the contact, the miss, or the window's last frame).
	if (Since < 0) return 0.f;
	const int32 W = FMath::Max(Windup, 0);
	const int32 End = EndedAt >= 0 ? EndedAt : W + FMath::Max(Window, 1);
	const auto Drive = [W](int32 T) { const float K = FMath::Clamp((T - W + 1) / 3.f, 0.f, 1.f); return FMath::Lerp(W > 0 ? 6.f : 0.f, -12.f, K); };
	if (Since < W && Since < End) return 6.f * (Since + 1) / static_cast<float>(W);
	if (Since < End) return Drive(Since);
	const float At = End <= W ? (W > 0 ? 6.f * FMath::Min(End, W) / static_cast<float>(W) : 0.f) : Drive(End - 1);
	const float K = FMath::Clamp((Since - End) / static_cast<float>(LeanBackFrames), 0.f, 1.f);
	return FMath::Lerp(At, 0.f, K);
}

void FJamHitReadModel::Reset()
{
	Presses.Reset(); Bursts.Reset(); Downs.Reset(); Flashes.Reset(); FrameLogs.Reset();
	LastFrame = -1;
	Hits = Misses = Strips = 0;
}

void FJamHitReadModel::Step(int64 Frame, const TArray<FJamSkaterState>& Skaters, const TArray<FJamSimEvent>& Events, double SkaterSpeed, const FLinearColor TeamColours[2])
{
	if (Frame <= LastFrame) Reset();   // a restart or a new match: the frame went back
	LastFrame = Frame;
	FrameLogs.Reset();
	if (TeamColours) { Colours[0] = TeamColours[0]; Colours[1] = TeamColours[1]; }
	const int32 N = Skaters.Num();
	Braced.SetNum(N); BracedSpeed.SetNum(N); Teams.SetNum(N); Down.SetNum(N);
	for (int32 i = 0; i < N; ++i)
	{
		const FJamSkaterState& S = Skaters[i];
		Teams[i] = FMath::Clamp(S.Team, 0, 1);
		Down[i] = S.IsDown();
		Braced[i] = S.bCrossBraced;
		BracedSpeed[i] = S.Vel.Size();
	}
	auto Running = [this](int32 Who, bool bCross) -> FPress*
	{
		for (int32 k = Presses.Num() - 1; k >= 0; --k) if (Presses[k].Skater == Who && Presses[k].bCross == bCross && Presses[k].EndFrame < 0) return &Presses[k];
		return nullptr;
	};
	// The presses of this frame.
	for (int32 i = 0; i < N; ++i)
	{
		const FJamSkaterState& S = Skaters[i];
		for (int32 Pass = 0; Pass < 2; ++Pass)
		{
			const bool bCross = Pass == 1;
			if (!(bCross ? S.bLiftThisFrame : S.bCheckThisFrame)) continue;
			if (FPress* Old = Running(i, bCross)) { Old->EndFrame = Frame; Old->Result = EJamHitResult::None; }   // a new press replaces a running one quietly
			FPress P;
			P.Skater = i; P.Team = Teams[i]; P.bCross = bCross; P.bHuman = S.bHuman; P.PressFrame = Frame;
			P.Windup = S.WindupFrames > 0 ? S.WindupFrames : (bCross ? S.LiftWindupLeft : S.CheckWindupLeft);
			P.Window = bCross ? 0 : (S.bHuman ? UJamSim::HumanCheckWindowFrames : UJamSim::CheckWindowFrames);
			Presses.Add(P);
		}
	}
	// A stick lift that took the puck: the stripped puck's flash, and the lifter's press knows it stripped.
	for (const FJamSimEvent& E : Events)
	{
		if (E.Name != JamEvents::Lift || E.Strength < 0.5f) continue;
		FFlash Fl; Fl.Frame = Frame; Flashes.Add(Fl);
		++Strips;
		if (FPress* P = Running(E.Who, true)) P->bStripped = true;
	}
	// The hits of this frame: every victim whose read-outs name the man who hit him.
	TSet<int32> CheckersSeen;
	for (int32 v = 0; v < N; ++v)
	{
		const FJamSkaterState& V = Skaters[v];
		if (V.ContactBy < 0 || V.ContactOutcome <= 0 || !Skaters.IsValidIndex(V.ContactBy)) continue;
		const FJamSkaterState& A = Skaters[V.ContactBy];
		const bool bBraced = V.bCrossBraceHit;
		const bool bCross = bBraced || V.bContactCross;
		const FJamSimEvent* Ev = nullptr;
		for (const FJamSimEvent& E : Events) if (E.Name == JamEvents::Check && E.Who == A.Index) { Ev = &E; break; }
		FBurst B;
		B.Frame = Frame;
		B.bBraced = bBraced;
		B.DirM = (V.Pos - A.Pos).GetSafeNormal();
		if (B.DirM.IsNearlyZero()) B.DirM = FVector2D(1, 0);
		B.AtM = Ev ? Ev->Where : (A.Pos + V.Pos) * 0.5;
		const double Closing = Ev ? static_cast<double>(Ev->Strength) * SkaterSpeed : 1.5;   // a bump carries no event: the soft ring
		const EJamHitResult R = V.ContactOutcome >= UJamSim::ContactDown ? EJamHitResult::Down : V.ContactOutcome == UJamSim::ContactStumble ? EJamHitResult::Stumble : EJamHitResult::Bump;
		B.RadiusM = BurstRadiusM(R == EJamHitResult::Bump ? 0.0 : Closing, bBraced);
		B.FallM = R == EJamHitResult::Down ? V.KnockDir : FVector2D::ZeroVector;
		B.Colour = Colours[FMath::Clamp(A.Team, 0, 1)];
		Bursts.Add(B);
		CheckersSeen.Add(A.Index);
		++Hits;
		FHitLog L;
		L.Frame = Frame; L.Kind = bBraced ? TEXT("braced") : bCross ? TEXT("cross") : TEXT("check"); L.Result = R;
		L.Checker = A.Index; L.Victim = v; L.bHuman = A.bHuman || V.bHuman; L.ClosingMps = Ev ? Closing : 0.0; L.BurstM = B.RadiusM;
		if (FPress* P = bBraced ? nullptr : Running(A.Index, bCross))
		{
			P->EndFrame = Frame; P->Result = R;
			L.Windup = P->Windup; L.PressToContact = static_cast<int32>(Frame - P->PressFrame);
		}
		FrameLogs.Add(L);
	}
	// A check event no read-out names (the momentum model off, a hip check): its burst at the event's point.
	for (const FJamSimEvent& E : Events)
	{
		if (E.Name != JamEvents::Check || CheckersSeen.Contains(E.Who) || !Skaters.IsValidIndex(E.Who)) continue;
		CheckersSeen.Add(E.Who);
		FBurst B;
		B.Frame = Frame; B.AtM = E.Where; B.DirM = E.Dir.IsNearlyZero() ? FVector2D(1, 0) : E.Dir.GetSafeNormal();
		B.RadiusM = BurstRadiusM(static_cast<double>(E.Strength) * SkaterSpeed, false);
		B.Colour = Colours[FMath::Clamp(Skaters[E.Who].Team, 0, 1)];
		Bursts.Add(B);
		++Hits;
		FHitLog L;
		L.Frame = Frame; L.Kind = TEXT("check"); L.Result = E.Outcome == FName(TEXT("stumble")) ? EJamHitResult::Stumble : EJamHitResult::Down;
		L.Checker = E.Who; L.bHuman = Skaters[E.Who].bHuman; L.ClosingMps = E.Strength * SkaterSpeed; L.BurstM = B.RadiusM;
		if (FPress* P = Running(E.Who, false)) { P->EndFrame = Frame; P->Result = L.Result; L.Windup = P->Windup; L.PressToContact = static_cast<int32>(Frame - P->PressFrame); }
		FrameLogs.Add(L);
	}
	// The presses that came to nothing: a body check whose window ran out, a cross-check whose wind-up ran out with no shove landed.
	for (FPress& P : Presses)
	{
		if (P.EndFrame >= 0 || !Skaters.IsValidIndex(P.Skater)) continue;
		const FJamSkaterState& S = Skaters[P.Skater];
		if (S.IsDown() || S.InBox()) { P.EndFrame = Frame; P.Result = EJamHitResult::None; continue; }
		bool bOver = false;
		if (!P.bCross) bOver = Frame > P.PressFrame && S.CheckWindupLeft == 0 && S.CheckWindow == 0;
		else bOver = Frame >= P.PressFrame + P.Windup && S.LiftWindupLeft == 0;
		if (!bOver) continue;
		P.EndFrame = Frame;
		P.Result = P.bStripped ? EJamHitResult::Strip : EJamHitResult::Miss;
		if (P.Result == EJamHitResult::Miss) ++Misses;
		FHitLog L;
		L.Frame = Frame; L.Kind = P.bCross ? TEXT("cross") : TEXT("check"); L.Result = P.Result; L.Checker = P.Skater; L.bHuman = P.bHuman; L.Windup = P.Windup;
		FrameLogs.Add(L);
	}
	// The downs.
	for (int32 i = 0; i < N; ++i)
	{
		const FJamSkaterState& S = Skaters[i];
		const int32 k = Downs.IndexOfByPredicate([i](const FDown& D) { return D.Skater == i; });
		if (S.KnockFrames > 0)
		{
			if (k == INDEX_NONE) { FDown D; D.Skater = i; D.Total = S.KnockFrames; D.Left = S.KnockFrames; D.DirM = S.KnockDir; Downs.Add(D); }
			else { Downs[k].Left = S.KnockFrames; Downs[k].Total = FMath::Max(Downs[k].Total, S.KnockFrames); }
		}
		else if (k != INDEX_NONE) Downs.RemoveAt(k);
	}
	// Forget what has finished drawing.
	const int32 Keep = FMath::Max(MissFadeFrames, LeanBackFrames) + 2;
	Presses.RemoveAll([Frame, Keep](const FPress& P) { return P.EndFrame >= 0 && Frame - P.EndFrame > Keep; });
	Bursts.RemoveAll([Frame](const FBurst& B) { return Frame - B.Frame > BurstFrames; });
	Flashes.RemoveAll([Frame](const FFlash& F) { return Frame - F.Frame > FlashFrames; });
}

namespace
{
	FVector2D YawDir(float YawDeg) { const double R = FMath::DegreesToRadians(static_cast<double>(YawDeg)); return FVector2D(FMath::Cos(R), FMath::Sin(R)); }
	float EaseOut(float A) { const float K = FMath::Clamp(A, 0.f, 1.f); return 1.f - (1.f - K) * (1.f - K); }
	const FLinearColor Grey(0.32f, 0.32f, 0.34f);
	const FLinearColor Gold(1.f, 0.78f, 0.15f);
	// The ground marks stand from 3 cm (at 0 to 3 cm the look tier's ice covered the chevron and the downs' oval in the first strips).
	constexpr double GroundZ = 3.0;

	// An arc of Count bars on the ice round C (cm) facing Dir, radius R (cm), spanning HalfDeg either side.
	void Arc(TArray<FJamHitBar>& Out, const FVector& C, const FVector2D& Dir, double R, int32 Count, double HalfDeg, double Wide, double Tall, double Z,
		const FLinearColor& Col, float Em, EJamHitLayer Layer, double Fill = 0.75)
	{
		const double Base = FMath::Atan2(Dir.Y, Dir.X);
		const double Span = FMath::DegreesToRadians(HalfDeg * 2.0);
		const double Step = Count > 1 ? Span / (Count - 1) : 0.0;
		const double Along = FMath::Max(4.0, (Count > 1 ? Step : Span) * R * Fill);
		for (int32 i = 0; i < Count; ++i)
		{
			const double A = Base - Span * 0.5 + Step * i;
			FJamHitBar B;
			B.CentreCm = C + FVector(R * FMath::Cos(A), R * FMath::Sin(A), Z + Tall * 0.5);
			B.YawDeg = static_cast<float>(FMath::RadiansToDegrees(A) + 90.0);
			B.SizeCm = FVector(Along, Wide, Tall);
			B.Colour = Col; B.Emissive = Em; B.Layer = static_cast<uint8>(Layer);
			Out.Add(B);
		}
	}
	// A full ring of Count bars (the first at angle 0), only the first Lit of them.
	void Ring(TArray<FJamHitBar>& Out, const FVector& C, double R, int32 Count, int32 Lit, double Wide, double Tall, double Z, const FLinearColor& Col, float Em, EJamHitLayer Layer, double Fill = 0.6)
	{
		const double Along = FMath::Max(4.0, 2.0 * UE_DOUBLE_PI * R / Count * Fill);
		for (int32 i = 0; i < FMath::Min(Lit, Count); ++i)
		{
			const double A = UE_DOUBLE_PI * 0.5 - 2.0 * UE_DOUBLE_PI * i / Count;   // from 12 o'clock, clockwise seen from above
			FJamHitBar B;
			B.CentreCm = C + FVector(R * FMath::Cos(A), R * FMath::Sin(A), Z + Tall * 0.5);
			B.YawDeg = static_cast<float>(FMath::RadiansToDegrees(A) + 90.0);
			B.SizeCm = FVector(Along, Wide, Tall);
			B.Colour = Col; B.Emissive = Em; B.Layer = static_cast<uint8>(Layer);
			Out.Add(B);
		}
	}
}

void FJamHitReadModel::Draw(double FrameF, const TArray<FVector>& At, const TArray<float>& Yaw, const FVector& PuckAtCm, const FDrawSwitches& Sw, TArray<FJamHitBar>& Out) const
{
	const double Sc = FMath::Clamp(static_cast<double>(Sw.Scale), 0.25, 3.0);
	// BEFORE: the presses.
	if (Sw.bTell)
	{
		for (const FPress& P : Presses)
		{
			if (!At.IsValidIndex(P.Skater) || !Yaw.IsValidIndex(P.Skater) || (Down.IsValidIndex(P.Skater) && Down[P.Skater])) continue;
			const float Since = static_cast<float>(FrameF - static_cast<double>(P.PressFrame));
			const bool bEnded = P.EndFrame >= 0;
			const float AfterEnd = bEnded ? static_cast<float>(FrameF - static_cast<double>(P.EndFrame)) : -1.f;
			const bool bLanded = bEnded && (P.Result == EJamHitResult::Down || P.Result == EJamHitResult::Stumble || P.Result == EJamHitResult::Bump);
			const bool bMiss = bEnded && (P.Result == EJamHitResult::Miss || P.Result == EJamHitResult::Strip);
			if (bEnded && !bLanded && !bMiss) continue;   // replaced, fell, boxed: nothing
			const FLinearColor Team = Colours[P.Team];
			const FVector2D F = YawDir(Yaw[P.Skater]);
			const FVector C = At[P.Skater];
			if (!P.bCross)
			{
				if (bLanded) continue;   // the burst takes over on the contact frame
				float Scale = 1.f, Em = 6.f, Slide = 0.f;
				FLinearColor Col = Team;
				if (Since < P.Windup) { const float K = FMath::Clamp((Since + 1.f) / FMath::Max(P.Windup, 1), 0.f, 1.f); Scale = 0.55f + 0.45f * K; Em = 2.f + 3.f * K; }
				if (bMiss)
				{
					const float K = AfterEnd / MissFadeFrames;
					if (K >= 1.f) continue;
					Col = FMath::Lerp(Team, Grey, FMath::Min(1.f, K * 3.f));
					Em = FMath::Lerp(1.5f, 0.2f, K); Scale = 1.f - 0.45f * K; Slide = 0.35f * K;
				}
				// A chevron pointing the way of the lunge, TellNearM to TellFarM ahead (outside the HUD's rings round the feet), white-hot over the team
				// colour; it slides out from the body over the wind-up and stands at full length through the window.
				const FLinearColor Hot = bMiss ? Col : FMath::Lerp(Col, FLinearColor::White, 0.45f);
				const FVector2D Side(-F.Y, F.X);
				const double Tip = (TellNearM + (TellFarM - TellNearM) * Scale + Slide) * 100.0 * Sc;
				const double ArmLen = 95.0 * Scale * Sc, ArmDeg = 28.0;
				for (int32 k = -1; k <= 1; k += 2)
				{
					const double A = FMath::DegreesToRadians(180.0 - k * ArmDeg);   // from the tip back and out to each side
					const FVector2D ArmDir = F * FMath::Cos(A) + Side * FMath::Sin(A);
					const FVector2D Mid = F * (Tip * 1.0) + ArmDir * (ArmLen * 0.5);
					FJamHitBar B;
					B.CentreCm = C + FVector(Mid.X, Mid.Y, GroundZ + 2.0);
					B.YawDeg = static_cast<float>(FMath::RadiansToDegrees(FMath::Atan2(ArmDir.Y, ArmDir.X)));
					B.SizeCm = FVector(ArmLen, 18.0 * Sc, 4.0);
					B.Colour = Hot; B.Emissive = Em * 1.4f; B.Layer = static_cast<uint8>(EJamHitLayer::Tell);
					Out.Add(B);
				}
			}
			else
			{
				// The streak across the stick: loading over the wind-up, thrust 35 cm forward on the shove, then gone over 8 frames (grey on a miss).
				float Fwd = 45.f, Em = 6.f;
				FLinearColor Col = Team;
				if (Since < P.Windup) { const float K = FMath::Clamp((Since + 1.f) / FMath::Max(P.Windup, 1), 0.f, 1.f); Em = 2.f + 4.f * K; }
				if (bEnded)
				{
					const float K = AfterEnd / 8.f;
					if (K >= 1.f) continue;
					if (bLanded) { Fwd = 45.f + 35.f * EaseOut(AfterEnd / 3.f); Col = FMath::Lerp(FLinearColor::White, Team, K); Em = FMath::Lerp(9.f, 1.f, K); }
					else { Col = FMath::Lerp(Team, Grey, FMath::Min(1.f, K * 3.f)); Em = FMath::Lerp(1.5f, 0.2f, K); }
				}
				FJamHitBar B;
				B.CentreCm = C + FVector(F.X, F.Y, 0.0) * (Fwd * Sc) + FVector(0, 0, 105.0);
				B.YawDeg = Yaw[P.Skater] + 90.f;
				B.SizeCm = FVector(140.0 * Sc, 12.0, 12.0);
				if (!bEnded || bLanded) Col = FMath::Lerp(Col, FLinearColor::White, 0.4f);
				B.Colour = Col; B.Emissive = Em; B.Layer = static_cast<uint8>(EJamHitLayer::Streak);
				Out.Add(B);
			}
		}
		// The held brace: the streak across the chest and an arc on the ice ahead, brighter with speed.
		for (int32 i = 0; i < Braced.Num(); ++i)
		{
			if (!Braced[i] || !At.IsValidIndex(i) || !Yaw.IsValidIndex(i) || Down[i]) continue;
			const FVector2D F = YawDir(Yaw[i]);
			const float K = static_cast<float>(FMath::Clamp(BracedSpeed[i] / 8.0, 0.0, 1.0));
			const FLinearColor Team = Colours[Teams[i]];
			FJamHitBar B;
			B.CentreCm = At[i] + FVector(F.X, F.Y, 0.0) * (42.0 * Sc) + FVector(0, 0, 110.0);
			B.YawDeg = Yaw[i] + 90.f;
			B.SizeCm = FVector(140.0 * Sc, 12.0, 12.0);
			B.Colour = Team; B.Emissive = 3.f + 3.f * K; B.Layer = static_cast<uint8>(EJamHitLayer::Brace);
			Out.Add(B);
			Arc(Out, At[i], F, BraceRadiusM * 100.0 * Sc, 7, 42.0, 16.0 * Sc, 4.0, GroundZ, FMath::Lerp(Team, FLinearColor::White, 0.35f), 2.f + 5.f * K, EJamHitLayer::Brace);
		}
	}
	// AT CONTACT: the bursts.
	if (Sw.bBurst)
	{
		for (const FBurst& B : Bursts)
		{
			const float A = static_cast<float>((FrameF - static_cast<double>(B.Frame)) / BurstFrames);
			if (A < 0.f || A >= 1.f) continue;
			const FVector C(B.AtM.X * 100.0, B.AtM.Y * 100.0, 0.0);
			const FLinearColor Col = FMath::Lerp(FLinearColor::White, B.Colour, FMath::Min(1.f, A * 2.f));
			const float Em = FMath::Lerp(10.f, 1.f, A);
			const double R = B.RadiusM * 100.0 * Sc * (0.45 + 0.55 * EaseOut(A));
			Ring(Out, C, R, 16, 16, (26.0 * (1.0 - A) + 5.0) * Sc, 4.0, GroundZ, Col, Em * 1.5f, EJamHitLayer::Burst, 0.7);
			if (B.bBraced && A > 0.15f) Ring(Out, C, R * 0.62, 12, 12, (18.0 * (1.0 - A) + 4.0) * Sc, 4.0, GroundZ, Col, Em * 1.5f, EJamHitLayer::Burst, 0.7);
			if (B.RadiusM <= BurstRadiusM(0.0, B.bBraced) + 1e-6) continue;   // a bump: the soft ring only
			const int32 Count = ShardCount(B.bBraced);
			const double Base = FMath::Atan2(-B.DirM.Y, -B.DirM.X);
			for (int32 i = 0; i < Count; ++i)
			{
				const double Ang = Base + 2.0 * UE_DOUBLE_PI * (i + 0.5) / Count;
				const FVector2D D(FMath::Cos(Ang), FMath::Sin(Ang));
				if (!ShardAllowed(D, B.FallM)) continue;
				const double Dist = B.RadiusM * 100.0 * Sc * (0.25 + 0.6 * EaseOut(A));
				FJamHitBar S;
				// A victim who stays up: shards that rise (45 to 80 cm); one who goes down: ice spray that stays on the ice (nothing over the lying figure).
				const bool bLow = !B.FallM.IsNearlyZero();
				S.CentreCm = C + FVector(D.X, D.Y, 0.0) * Dist + FVector(0, 0, bLow ? GroundZ + 2.0 : 45.0 + 35.0 * A);
				S.YawDeg = static_cast<float>(FMath::RadiansToDegrees(Ang));
				S.PitchDeg = bLow ? 0.f : 25.f;
				S.SizeCm = FVector((60.0 * (1.0 - A) + 15.0) * Sc, bLow ? 14.0 : 10.0, bLow ? 4.0 : 10.0);
				S.Colour = Col; S.Emissive = Em; S.Layer = static_cast<uint8>(EJamHitLayer::Shard);
				Out.Add(S);
			}
		}
	}
	// AFTER: the downs' countdown and the stripped puck.
	if (Sw.bDown)
	{
		for (const FDown& D : Downs)
		{
			if (!At.IsValidIndex(D.Skater)) continue;
			const int32 Lit = DownBarsLit(D.Left, D.Total);
			const FVector2D Dir = D.DirM.IsNearlyZero() ? FVector2D(1, 0) : D.DirM.GetSafeNormal();
			const FVector C = At[D.Skater] + FVector(Dir.X, Dir.Y, 0.0) * (DownBodyM * 50.0);
			// His team's own colour, unlifted (lifted to white by a quarter it was faint on the pale clay ice in the strips).
			const FLinearColor Col = Colours[Teams.IsValidIndex(D.Skater) ? Teams[D.Skater] : 0];
			// An oval on the ice round the lying body (DownRingRadiusM along it, DownRingSideM across), lit bars from his head round clockwise.
			const FVector2D Side(-Dir.Y, Dir.X);
			const double Along = 2.0 * UE_DOUBLE_PI * DownRingSideM * 100.0 / DownSegments * 0.75;
			for (int32 i = 0; i < Lit; ++i)
			{
				const double A = 2.0 * UE_DOUBLE_PI * i / DownSegments;
				const FVector2D P = Dir * (FMath::Cos(A) * DownRingRadiusM) + Side * (FMath::Sin(A) * DownRingSideM);
				const FVector2D T = (-Dir * (FMath::Sin(A) * DownRingRadiusM) + Side * (FMath::Cos(A) * DownRingSideM)).GetSafeNormal();
				FJamHitBar B;
				B.CentreCm = FVector(C.X + P.X * 100.0, C.Y + P.Y * 100.0, GroundZ + 1.5);
				B.YawDeg = static_cast<float>(FMath::RadiansToDegrees(FMath::Atan2(T.Y, T.X)));
				B.SizeCm = FVector(Along, 15.0, 3.0);
				B.Colour = Col; B.Emissive = 3.5f; B.Layer = static_cast<uint8>(EJamHitLayer::Down);
				Out.Add(B);
			}
		}
	}
	if (Sw.bFlash)
	{
		for (const FFlash& Fl : Flashes)
		{
			const float A = static_cast<float>((FrameF - static_cast<double>(Fl.Frame)) / FlashFrames);
			if (A < 0.f || A >= 1.f) continue;
			Ring(Out, FVector(PuckAtCm.X, PuckAtCm.Y, 0.0), (45.0 + 55.0 * EaseOut(A)) * Sc, 10, 10, 8.0 * (1.0 - A) + 3.0, 3.0, GroundZ, FMath::Lerp(FLinearColor::White, Gold, FMath::Min(1.f, A * 3.f)),
				FMath::Lerp(9.f, 1.f, A), EJamHitLayer::PuckFlash);
		}
	}
}

float FJamHitReadModel::LeanFor(int32 I, double FrameF) const
{
	if (Down.IsValidIndex(I) && Down[I]) return 0.f;
	if (Braced.IsValidIndex(I) && Braced[I]) return -8.f;
	for (int32 k = Presses.Num() - 1; k >= 0; --k)
	{
		const FPress& P = Presses[k];
		if (P.Skater != I) continue;
		if (P.EndFrame >= 0 && P.Result == EJamHitResult::None) return 0.f;
		const int32 Since = static_cast<int32>(FMath::FloorToDouble(FrameF - static_cast<double>(P.PressFrame)));
		const int32 Ended = P.EndFrame >= 0 ? static_cast<int32>(P.EndFrame - P.PressFrame) : -1;
		// A cross-check: the same shape with a one-frame window (the shove's frame).
		return CheckLeanDeg(Since, P.Windup, P.bCross ? 1 : P.Window, P.bCross && Ended < 0 ? -1 : Ended) * (P.bCross ? 0.8f : 1.f);
	}
	return 0.f;
}

// ---------------------------------------------------------------- the actor

AJamHitRead::AJamHitRead()
{
	PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	Root->SetMobility(EComponentMobility::Movable);
}

int32 AJamHitRead::LeanMode() { return FMath::Clamp(CVarJamHitLean.GetValueOnGameThread(), 0, 2); }

bool AJamHitRead::FigureHas(UObject* Figure, const TCHAR* Function)
{
	return Figure && Figure->FindFunction(FName(Function)) != nullptr;
}

static const TCHAR* HitResultName(EJamHitResult R)
{
	switch (R)
	{
	case EJamHitResult::Bump: return TEXT("bump");
	case EJamHitResult::Stumble: return TEXT("stumble");
	case EJamHitResult::Down: return TEXT("down");
	case EJamHitResult::Miss: return TEXT("miss");
	case EJamHitResult::Strip: return TEXT("strip");
	default: return TEXT("none");
	}
}

void AJamHitRead::StepSim(UWorld* World, const UJamSim* Sim, const TArray<TObjectPtr<AJamSkater>>& Skaters, TObjectPtr<AJamHitRead>& Slot)
{
	if (!World || !Sim) return;
	if (!Slot)
	{
		FActorSpawnParameters Sp;
		Sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Slot = World->SpawnActor<AJamHitRead>(AJamHitRead::StaticClass(), FTransform::Identity, Sp);
		if (!Slot) return;
		// The pool made up front, hidden (the first hit of a match drew its first frame dark while its new bars' instances were made).
		TArray<FJamHitBar> Warm;
		Warm.SetNum(PrewarmBars);
		Slot->ApplyBars(Warm);
		Slot->ApplyBars(TArray<FJamHitBar>());
	}
	FLinearColor Cols[2] = { FLinearColor(1.f, 0.4f, 0.05f), FLinearColor(0.05f, 0.3f, 1.f) };
	for (int32 t = 0; t < 2; ++t) if (Sim->GetConfig().Teams.IsValidIndex(t)) Cols[t] = Sim->GetConfig().Teams[t].Colour;
	Slot->Model.Step(Sim->GetFrame(), Sim->GetSkaters(), Sim->GetFrameEvents(), Sim->GetConfig().Skater.Speed, Cols);
	for (const FJamHitReadModel::FHitLog& L : Slot->Model.GetFrameLogs())
	{
		UE_LOG(LogJamHit, Log, TEXT("JAMHIT frame=%lld kind=%s result=%s checker=%d victim=%d human=%d closing=%.1f burst_m=%.2f windup=%d press_to_contact=%d"),
			L.Frame, *L.Kind, HitResultName(L.Result), L.Checker, L.Victim, L.bHuman ? 1 : 0, L.ClosingMps, L.BurstM, L.Windup, L.PressToContact);
	}
	Slot->Capture(Sim);
}

void AJamHitRead::Capture(const UJamSim* Sim)
{
	const int32 N = CVarJamHitCapture.GetValueOnGameThread();
	if (N <= 0) return;
	const int64 F = Sim->GetFrame();
	const int32 H = Sim->GetHumanIndex();
	if (!Sim->GetSkaters().IsValidIndex(H)) return;
	const FJamSkaterState& S = Sim->GetSkaters()[H];
	if (CaptureFrom < 0)
	{
		static bool bWasBraced = false;
		FString Kind;
		int64 Until = F;
		if (S.bCheckThisFrame) { Kind = TEXT("check"); Until = F + FMath::Max(S.WindupFrames, 0) + UJamSim::HumanCheckWindowFrames + 30; }
		else if (S.bLiftThisFrame) { Kind = TEXT("cross"); Until = F + FMath::Max(S.WindupFrames, 0) + 30; }
		else if (S.bCrossBraced && !bWasBraced) { Kind = TEXT("brace"); Until = F + 240; }
		else for (const FJamHitReadModel::FHitLog& L : Model.GetFrameLogs()) if (L.Victim == H && (L.Result == EJamHitResult::Down || L.Result == EJamHitResult::Stumble)) { Kind = TEXT("taken"); Until = F + 40; }
		bWasBraced = S.bCrossBraced;
		if (Kind.IsEmpty()) return;
		int32& Taken = CapturesPerKind.FindOrAdd(Kind);
		if (Taken >= N) return;
		++Taken;
		CaptureFrom = F; CaptureUntil = Until; CaptureKind = Kind;
		UE_LOG(LogJamHit, Log, TEXT("JAMHIT capture start kind=%s frame=%lld"), *Kind, F);
	}
	// The human's press ended (a contact, a miss) or the brace ended: 30 frames more.
	if (CaptureKind != TEXT("taken")) for (const FJamHitReadModel::FHitLog& L : Model.GetFrameLogs()) if ((L.Checker == H || L.Victim == H) && L.Frame >= CaptureFrom) CaptureUntil = FMath::Min(CaptureUntil, L.Frame + 30);
	if (CaptureKind == TEXT("brace") && !S.bCrossBraced && CaptureUntil > F + 30) CaptureUntil = F + 30;
	FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("a196-%s-%06lld-f%06lld.png"), *CaptureKind, CaptureFrom, F), true, false);
	if (F >= CaptureUntil)
	{
		UE_LOG(LogJamHit, Log, TEXT("JAMHIT capture end kind=%s from=%lld to=%lld"), *CaptureKind, CaptureFrom, F);
		CaptureFrom = -1;
	}
}

void AJamHitRead::Render(const UJamSim* Sim, const TArray<TObjectPtr<AJamSkater>>& Skaters, const FVector& PuckAtCm, double FrameF)
{
	if (!Sim) return;
	const int32 N = Sim->GetSkaters().Num();
	TArray<FVector> At; At.Init(FVector::ZeroVector, N);
	TArray<float> Yaw; Yaw.Init(0.f, N);
	for (AJamSkater* Sk : Skaters)
	{
		if (!Sk || !At.IsValidIndex(Sk->GetSimIndex())) continue;
		const FVector L = Sk->GetActorLocation();
		At[Sk->GetSimIndex()] = FVector(L.X, L.Y, 0.0);
		Yaw[Sk->GetSimIndex()] = static_cast<float>(Sk->GetActorRotation().Yaw);
	}
	FJamHitReadModel::FDrawSwitches Sw;
	Sw.bTell = CVarJamHitTell.GetValueOnGameThread() != 0;
	Sw.bBurst = CVarJamHitBurst.GetValueOnGameThread() != 0;
	Sw.bDown = CVarJamHitDown.GetValueOnGameThread() != 0;
	Sw.bFlash = CVarJamHitPuckFlash.GetValueOnGameThread() != 0;
	Sw.Scale = CVarJamHitScale.GetValueOnGameThread();
	TArray<FJamHitBar> Bars;
	Model.Draw(FrameF, At, Yaw, PuckAtCm, Sw, Bars);
	ApplyBars(Bars);
	// The lean of the drawn figure (after its ApplyRender set this frame's transform; the next ApplyRender sets it afresh).
	const int32 Mode = LeanMode();
	if (Mode == 0) return;
	for (AJamSkater* Sk : Skaters)
	{
		if (!Sk) continue;
		const int32 I = Sk->GetSimIndex();
		if (!Sim->GetSkaters().IsValidIndex(I) || Sk->IsHitStopped()) continue;
		const float Lean = Model.LeanFor(I, FrameF);
		if (FMath::IsNearlyZero(Lean)) continue;
		if (Mode == 1)
		{
			// Device B's take stands the stand-in down: the brace's pose, the cross-check's, the check's wind-up.
			UObject* Fig = Sk->GetVisualObject();
			const FJamSkaterState& S = Sim->GetSkaters()[I];
			const bool bBraceLean = S.bCrossBraced;
			bool bCrossLean = false;
			for (const FJamHitReadModel::FPress& P : Model.GetPresses()) if (P.Skater == I) bCrossLean = P.bCross;
			const TCHAR* Take = bBraceLean ? TEXT("SetCrossCheckHold") : bCrossLean ? TEXT("PlayCrossCheck") : TEXT("PlayCheckWindup");
			if (FigureHas(Fig, Take)) continue;
		}
		Sk->AddActorLocalRotation(FRotator(Lean, 0.0, 0.0));
	}
}

void AJamHitRead::ApplyBars(const TArray<FJamHitBar>& Bars)
{
	static const TCHAR* const TrimPath = TEXT("/Game/Placeholders/M_Trim.M_Trim");   // A's own trim material (TeamColour, Emissive), as the boosts' beacon
	UStaticMesh* Cube = nullptr;
	UMaterialInterface* Base = nullptr;
	for (int32 i = Pool.Num(); i < Bars.Num(); ++i)
	{
		if (!Cube) Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		if (!Base)
		{
			Base = LoadObject<UMaterialInterface>(nullptr, TrimPath, nullptr, LOAD_NoWarn | LOAD_Quiet);
			if (!Base) Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		}
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this, NAME_None, RF_Transient);
		C->SetMobility(EComponentMobility::Movable);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->SetGenerateOverlapEvents(false);
		C->SetCastShadow(false);
		if (Cube) C->SetStaticMesh(Cube);
		C->SetupAttachment(Root);
		C->RegisterComponent();
		UMaterialInstanceDynamic* M = Base ? UMaterialInstanceDynamic::Create(Base, this) : nullptr;
		if (M) C->SetMaterial(0, M);
		Pool.Add(C);
		PoolMids.Add(M);
		PoolColour.Add(FLinearColor(-1, -1, -1));
		PoolEmissive.Add(-1.f);
	}
	for (int32 i = 0; i < Pool.Num(); ++i)
	{
		UStaticMeshComponent* C = Pool[i];
		if (!C) continue;
		if (i >= Bars.Num()) { if (C->IsVisible()) C->SetVisibility(false); continue; }
		const FJamHitBar& B = Bars[i];
		C->SetWorldLocationAndRotation(B.CentreCm, FRotator(B.PitchDeg, B.YawDeg, 0.f));
		C->SetWorldScale3D(B.SizeCm / 100.0);
		if (!C->IsVisible()) C->SetVisibility(true);
		if (UMaterialInstanceDynamic* M = PoolMids[i])
		{
			if (!PoolColour[i].Equals(B.Colour, 0.004f)) { PoolColour[i] = B.Colour; M->SetVectorParameterValue(TEXT("TeamColour"), B.Colour); M->SetVectorParameterValue(TEXT("Color"), B.Colour); }
			if (!FMath::IsNearlyEqual(PoolEmissive[i], B.Emissive, 0.01f)) { PoolEmissive[i] = B.Emissive; M->SetScalarParameterValue(TEXT("Emissive"), B.Emissive); }
		}
	}
	BarsDrawn = Bars.Num();
}

// ---------------------------------------------------------------- tests

#if WITH_DEV_AUTOMATION_TESTS

namespace JamHitTest
{
	TArray<FJamSkaterState> Two()
	{
		TArray<FJamSkaterState> S;
		S.SetNum(2);
		S[0].Index = 0; S[0].Team = 0; S[0].bHuman = true; S[0].Pos = FVector2D(0, 0);
		S[1].Index = 1; S[1].Team = 1; S[1].Pos = FVector2D(3, 0);
		return S;
	}
	void ClearFrame(TArray<FJamSkaterState>& S)
	{
		for (FJamSkaterState& X : S) { X.bCheckThisFrame = false; X.bLiftThisFrame = false; X.WindupFrames = 0; X.ContactBy = -1; X.ContactOutcome = 0; X.bContactCross = false; X.bCrossBraceHit = false; }
	}
	int32 Count(const TArray<FJamHitBar>& Bars, EJamHitLayer L) { int32 N = 0; for (const FJamHitBar& B : Bars) if (B.Layer == static_cast<uint8>(L)) ++N; return N; }
	const FLinearColor Cols[2] = { FLinearColor(1, 0.4f, 0.05f), FLinearColor(0.05f, 0.3f, 1) };
	void Draw(const FJamHitReadModel& M, double FrameF, const TArray<FJamSkaterState>& S, TArray<FJamHitBar>& Out)
	{
		TArray<FVector> At; TArray<float> Yaw;
		for (const FJamSkaterState& X : S) { At.Add(FVector(X.Pos.X * 100.0, X.Pos.Y * 100.0, 0.0)); Yaw.Add(static_cast<float>(FMath::RadiansToDegrees(X.Yaw))); }
		Out.Reset();
		M.Draw(FrameF, At, Yaw, FVector(500, 0, 0), FJamHitReadModel::FDrawSwitches(), Out);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHitBurstSize, "Jam.Hit.BurstSize", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHitBurstSize::RunTest(const FString&)
{
	double Prev = 0.0;
	for (int32 V = 0; V <= 16; ++V)
	{
		const double R = FJamHitReadModel::BurstRadiusM(V, false);
		TestTrue(FString::Printf(TEXT("the ring grows with the closing speed (%d m/s: %.2f m)"), V, R), R >= Prev - 1e-9);
		Prev = R;
	}
	TestEqual(TEXT("a soft touch: 1.2 m (outside the HUD's rings)"), FJamHitReadModel::BurstRadiusM(0.0, false), 1.2, 1e-6);
	TestEqual(TEXT("14 m/s and over: 3 m"), FJamHitReadModel::BurstRadiusM(20.0, false), 3.0, 1e-6);
	TestTrue(TEXT("a braced hit's ring is larger"), FJamHitReadModel::BurstRadiusM(6.0, true) > FJamHitReadModel::BurstRadiusM(6.0, false));
	TestTrue(TEXT("a braced hit throws more shards"), FJamHitReadModel::ShardCount(true) > FJamHitReadModel::ShardCount(false));
	// A landed check through the model: the press, four frames of wind-up, the contact at closing 6 m/s (Strength 0.75 of 8 m/s).
	using namespace JamHitTest;
	FJamHitReadModel M;
	TArray<FJamSkaterState> S = Two();
	TArray<FJamSimEvent> None;
	S[0].bCheckThisFrame = true; S[0].WindupFrames = 4; S[0].CheckWindupLeft = 4;
	M.Step(100, S, None, 8.0, Cols);
	for (int64 F = 101; F <= 105; ++F)
	{
		ClearFrame(S);
		S[0].CheckWindupLeft = FMath::Max(0, 104 - static_cast<int32>(F)); S[0].CheckWindow = S[0].CheckWindupLeft > 0 ? 0 : 12;
		TArray<FJamSimEvent> Ev;
		if (F == 106 - 1)
		{
			S[1].ContactBy = 0; S[1].ContactOutcome = UJamSim::ContactDown; S[1].KnockFrames = 90; S[1].KnockDir = FVector2D(1, 0);
			FJamSimEvent E; E.Name = JamEvents::Check; E.Who = 0; E.Where = FVector2D(1.5, 0); E.Strength = 0.75f; E.Dir = FVector2D(1, 0); Ev.Add(E);
		}
		M.Step(F, S, Ev, 8.0, Cols);
	}
	TestEqual(TEXT("one hit"), M.Hits, 1);
	TestEqual(TEXT("no miss"), M.Misses, 0);
	TestEqual(TEXT("one burst"), M.GetBursts().Num(), 1);
	if (M.GetBursts().Num() == 1) TestEqual(TEXT("its ring by the closing speed (6 m/s)"), M.GetBursts()[0].RadiusM, FJamHitReadModel::BurstRadiusM(6.0, false), 1e-6);
	TArray<FJamHitBar> Bars;
	Draw(M, 105.5, S, Bars);
	TestEqual(TEXT("the burst's ring is drawn"), Count(Bars, EJamHitLayer::Burst), 16);
	TestTrue(TEXT("shards are drawn"), Count(Bars, EJamHitLayer::Shard) > 0);
	TestEqual(TEXT("no tell once it landed"), Count(Bars, EJamHitLayer::Tell), 0);
	Draw(M, 105.0 + FJamHitReadModel::BurstFrames + 0.5, S, Bars);
	TestEqual(TEXT("the burst is over after its frames"), Count(Bars, EJamHitLayer::Burst), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHitMissNoBurst, "Jam.Hit.MissNoBurst", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHitMissNoBurst::RunTest(const FString&)
{
	using namespace JamHitTest;
	FJamHitReadModel M;
	TArray<FJamSkaterState> S = Two();
	S[1].Pos = FVector2D(9, 0);   // nobody within reach
	TArray<FJamSimEvent> None;
	S[0].bCheckThisFrame = true; S[0].WindupFrames = 4; S[0].CheckWindupLeft = 4;
	M.Step(10, S, None, 8.0, Cols);
	TArray<FJamHitBar> Bars;
	Draw(M, 10.0, S, Bars);
	TestEqual(TEXT("the wind-up shows the chevron (2 bars)"), Count(Bars, EJamHitLayer::Tell), 2);
	int64 MissAt = -1;
	for (int64 F = 11; F <= 30; ++F)
	{
		ClearFrame(S);
		const int32 Wu = FMath::Max(0, 14 - static_cast<int32>(F));
		S[0].CheckWindupLeft = Wu;
		S[0].CheckWindow = Wu > 0 ? 0 : FMath::Max(0, 26 - static_cast<int32>(F));   // the window: frames 14..25
		M.Step(F, S, None, 8.0, Cols);
		if (MissAt < 0 && M.Misses > 0) MissAt = F;
	}
	TestEqual(TEXT("one miss"), M.Misses, 1);
	TestEqual(TEXT("the miss when the window ran out"), MissAt, static_cast<int64>(26));
	TestEqual(TEXT("a miss gives no burst"), M.GetBursts().Num(), 0);
	TestEqual(TEXT("no hit"), M.Hits, 0);
	Draw(M, 27.0, S, Bars);
	TestEqual(TEXT("no burst drawn"), Count(Bars, EJamHitLayer::Burst) + Count(Bars, EJamHitLayer::Shard), 0);
	TestEqual(TEXT("the chevron fades out after the miss"), Count(Bars, EJamHitLayer::Tell), 2);
	bool bGrey = true;
	for (const FJamHitBar& B : Bars) if (B.Layer == static_cast<uint8>(EJamHitLayer::Tell)) bGrey &= B.Emissive < 3.f;   // the live chevron burns at 8.4 through the window
	TestTrue(TEXT("the missed chevron is dim"), bGrey);
	Draw(M, 26.0 + FJamHitReadModel::MissFadeFrames + 0.5, S, Bars);
	TestEqual(TEXT("and gone after its fade"), Count(Bars, EJamHitLayer::Tell), 0);
	// A cross-check whose stick took the puck and whose shove found nobody: the strip, the puck's flash, no burst.
	FJamHitReadModel X;
	TArray<FJamSkaterState> T = Two();
	T[0].bLiftThisFrame = true; T[0].WindupFrames = 4;
	TArray<FJamSimEvent> Lift;
	FJamSimEvent E; E.Name = JamEvents::Lift; E.Who = 0; E.Strength = 1.f; Lift.Add(E);
	X.Step(50, T, Lift, 8.0, Cols);
	for (int64 F = 51; F <= 56; ++F) { ClearFrame(T); X.Step(F, T, None, 8.0, Cols); }
	TestEqual(TEXT("the strip is counted"), X.Strips, 1);
	TestEqual(TEXT("a strip with no shove is no miss"), X.Misses, 0);
	TestEqual(TEXT("and gives no burst"), X.GetBursts().Num(), 0);
	Draw(X, 52.0, T, Bars);
	TestEqual(TEXT("the stripped puck flashes"), Count(Bars, EJamHitLayer::PuckFlash), 10);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHitDownClear, "Jam.Hit.DownClear", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHitDownClear::RunTest(const FString&)
{
	using namespace JamHitTest;
	TestEqual(TEXT("12 bars at the fall"), FJamHitReadModel::DownBarsLit(120, 120), 12);
	TestEqual(TEXT("6 at half way"), FJamHitReadModel::DownBarsLit(60, 120), 6);
	TestEqual(TEXT("1 on the last frame"), FJamHitReadModel::DownBarsLit(1, 120), 1);
	TestEqual(TEXT("none when up"), FJamHitReadModel::DownBarsLit(0, 120), 0);
	// The hit that downs skater 1 along +x, every direction of fall: nothing over the lying figure but the ice.
	for (int32 Deg = 0; Deg < 360; Deg += 30)
	{
		const FVector2D Fall(FMath::Cos(FMath::DegreesToRadians(Deg)), FMath::Sin(FMath::DegreesToRadians(Deg)));
		FJamHitReadModel M;
		TArray<FJamSkaterState> S = Two();
		S[1].Pos = FVector2D(1.2, 0);
		TArray<FJamSimEvent> Ev;
		S[1].ContactBy = 0; S[1].ContactOutcome = UJamSim::ContactDown; S[1].KnockFrames = 120; S[1].KnockDir = Fall;
		FJamSimEvent E; E.Name = JamEvents::Check; E.Who = 0; E.Where = FVector2D(0.6, 0); E.Strength = 1.f; E.Dir = FVector2D(1, 0); Ev.Add(E);
		M.Step(200, S, Ev, 8.0, Cols);
		const FVector2D Feet = S[1].Pos, Head = S[1].Pos + Fall * FJamHitReadModel::DownBodyM;
		for (double FrameF = 200.0; FrameF < 200.0 + FJamHitReadModel::BurstFrames; FrameF += 0.5)
		{
			TArray<FJamHitBar> Bars;
			Draw(M, FrameF, S, Bars);
			for (const FJamHitBar& B : Bars)
			{
				const FVector2D P(B.CentreCm.X / 100.0, B.CentreCm.Y / 100.0);
				const FVector2D Closest = FMath::ClosestPointOnSegment2D(P, Feet, Head);
				const double D = FVector2D::Distance(P, Closest);
				const double TopCm = B.CentreCm.Z + B.SizeCm.Z * 0.5;
				if (B.Layer == static_cast<uint8>(EJamHitLayer::Down))
				{
					TestTrue(TEXT("the countdown lies on the ice"), TopCm <= 8.0);
					TestTrue(FString::Printf(TEXT("the countdown is clear of the lying figure (fall %d deg, %.2f m)"), Deg, D), D > FJamHitReadModel::DownBodyHalfWideM + 0.2);
				}
				else if (TopCm > 10.0)
				{
					TestTrue(FString::Printf(TEXT("nothing above the ice over the lying figure (fall %d deg, layer %d, %.2f m)"), Deg, B.Layer, D), D > FJamHitReadModel::DownBodyHalfWideM);
				}
			}
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHitLean, "Jam.Hit.Lean", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHitLean::RunTest(const FString&)
{
	// Wind-up 4, window 12, no contact: back over frames 0..3, forward from 4, upright 8 frames after the window.
	TestTrue(TEXT("loading: leans back in the wind-up"), FJamHitReadModel::CheckLeanDeg(1, 4, 12, -1) > 0.f);
	TestEqual(TEXT("fully back on the wind-up's last frame"), FJamHitReadModel::CheckLeanDeg(3, 4, 12, -1), 6.f, 1e-4f);
	TestTrue(TEXT("the drive: forward in the window"), FJamHitReadModel::CheckLeanDeg(8, 4, 12, -1) < -10.f);
	TestEqual(TEXT("upright after the window and the ease"), FJamHitReadModel::CheckLeanDeg(16 + FJamHitReadModel::LeanBackFrames, 4, 12, -1), 0.f, 1e-4f);
	TestTrue(TEXT("a contact on frame 6 eases back from there"), FMath::Abs(FJamHitReadModel::CheckLeanDeg(10, 4, 12, 6)) < FMath::Abs(FJamHitReadModel::CheckLeanDeg(10, 4, 12, -1)));
	TestEqual(TEXT("no wind-up: nothing before the press"), FJamHitReadModel::CheckLeanDeg(-1, 0, 6, -1), 0.f, 1e-4f);
	TestTrue(TEXT("the lean stays small (a stand-in, not a take)"), FMath::Abs(FJamHitReadModel::CheckLeanDeg(5, 4, 12, -1)) <= 12.f);
	TestEqual(TEXT("the lean is on by default"), AJamHitRead::LeanMode(), 1);
	UObject* Plain = GetTransientPackage();
	TestFalse(TEXT("an object without device B's takes has none"), AJamHitRead::FigureHas(Plain, TEXT("PlayCheckWindup")));
	return true;
}

#endif
