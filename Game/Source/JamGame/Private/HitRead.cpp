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
#include "Blueprint/WidgetLayoutLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Rendering/DrawElements.h"
#include "RadarWidget.h"
#include "HUDFeed.h"

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
static TAutoConsoleVariable<int32> CVarJamHitBTakes(TEXT("jam.hit.BTakes"), 2,
	TEXT("Device B's wind-up takes: SetWindupTakes(true, 8) by name on 2 (default from lane A210: every figure, the human's too: the figure does the hitting; at a 4-frame wind-up B's take starts inside its coil and still shows the drive and the follow-through), 1 the bots' figures only (package H34: the human on A's stand-in lean), 0 none. A's lean stands down exactly on the figures told."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitCaptureBots(TEXT("jam.hit.CaptureBots"), 0,
	TEXT("With jam.hit.Capture: 1 = a bot's check or cross-check pressed within 14 m of the human is a strip too (botcheck / botcross). Lane A210."), ECVF_Default);
static constexpr double CaptureBotsM = 14.0;
static constexpr float BurstFramesF = static_cast<float>(FJamHitReadModel::BurstFrames);
// Lane A210 (the night, R-0067: "technical blue rectangle"): the drawn bars of a cross-check, each behind its own switch.
static TAutoConsoleVariable<int32> CVarJamHitStreak(TEXT("jam.hit.Streak"), 0,
	TEXT("The bar of light across the stick of a pressed cross-check: 0 (default from lane A210: the operator read it as a bug, R-0067) not drawn, 1 drawn (package H34)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitBraceBar(TEXT("jam.hit.BraceBar"), 0,
	TEXT("The bar of light across the chest of a held cross-check, in the team's colour: 0 (default from lane A210: this is the technical blue rectangle of R-0067) not drawn, 1 drawn (package H34)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitQuiet(TEXT("jam.hit.Quiet"), 1,
	TEXT("The marks as quiet helpers (lane A210): 1 (default) the chevron and the brace's arc thin and in the ice's white, the contact ring small (jam.hit.RingScale), white and gone in 8 frames; 0 = package H34's marks (fat, in the team's colour, a ring to 3 m for 14 frames)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarJamHitRingScale(TEXT("jam.hit.RingScale"), 0.45f,
	TEXT("With jam.hit.Quiet 1: the contact ring's radius as a share of package H34's (0.8 to 3.0 m by closing speed), never under 0.95 m. Default 0.45."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitBraceArc(TEXT("jam.hit.BraceArc"), 1,
	TEXT("The arc on the ice ahead of a held cross-check (1 drawn, 0 not)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitOffScreen(TEXT("jam.hit.OffScreen"), 1,
	TEXT("A man down outside the frame (lane A196, evolution 10): 1 (default) an arrow on the screen's edge pointing at him (the human's mates a hollow diamond, opponents a solid triangle) and his mark on the radar; 0 none (before). Presentation only."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitCapture(TEXT("jam.hit.Capture"), 0,
	TEXT("Proof (lane A196): N > 0 takes a still every sim frame from each of the human's presses (check, cross, brace) to 30 frames after it landed or missed, N strips a kind (run with a fixed 60 Hz step). 0 = off."), ECVF_Default);

// ---------------------------------------------------------------- the pure part

double FJamHitReadModel::BurstRadiusM(double ClosingMps, bool bBraced)
{
	const double C = FMath::Clamp(ClosingMps, 0.0, 14.0);
	const double R = 1.2 + (3.0 - 1.2) * (C / 14.0);
	return bBraced ? R * 1.2 : R;
}

bool FJamHitReadModel::ClearOfBody(const FVector2D& P, const FVector2D& C, const FVector2D& Fall, double Margin)
{
	if (Fall.IsNearlyZero()) return FVector2D::Distance(P, C) > Margin;
	const FVector2D Head = C + Fall.GetSafeNormal() * DownBodyM;
	return FVector2D::Distance(P, FMath::ClosestPointOnSegment2D(P, C, Head)) > Margin;
}

double FJamHitReadModel::TipRemainFrames(double GapM, double ClosingMps, float Since, int32 Windup)
{
	// The frames to the contact as they look now: the bodies' gap over the closing speed, never before the wind-up ends (the window opens then).
	const double Bodies = FMath::Max(0.0, GapM - 2.0 * UJamSim::SkaterRadius);
	const double ByGap = Bodies / FMath::Max(ClosingMps, 0.5) * 60.0;
	return FMath::Clamp(FMath::Max(static_cast<double>(Windup) - Since, ByGap), 0.0, 30.0);
}

double FJamHitReadModel::TipDistM(double GapM, float Since, int32 Windup, double ClosingMps)
{
	if (GapM < 0.0) return TellNearM + (TellFarM - TellNearM) * FMath::Clamp((Since + 1.0) / FMath::Max(Windup, 1), 0.0, 1.0);
	const double R = UJamSim::SkaterRadius;
	const double Near = FMath::Max(GapM - R, R);   // the target's near side
	const double Rem = TipRemainFrames(GapM, ClosingMps, Since, Windup);
	const double Short = Rem / (Rem + FMath::Max(Since, 0.f) + 1.0);   // the share of the way still to go: 1 far off, 0 on him
	return R + (Near - R) * (1.0 - Short);
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
	Braced.SetNum(N); BracedSpeed.SetNum(N); Teams.SetNum(N); Down.SetNum(N); PosM.SetNum(N); FallM.SetNum(N); VelM.SetNum(N);
	for (int32 i = 0; i < N; ++i)
	{
		const FJamSkaterState& S = Skaters[i];
		Teams[i] = FMath::Clamp(S.Team, 0, 1);
		Down[i] = S.IsDown();
		Braced[i] = S.bCrossBraced;
		PosM[i] = S.Pos;
		VelM[i] = S.Vel;
		FallM[i] = S.IsDown() ? S.KnockDir : FVector2D::ZeroVector;
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
			// Evolution 10: the man the chevron points at: a cross-check's CrossTarget, else the nearest standing opponent within 7 m and 70 degrees of
			// the way he faces (presentation's own guess: no rule reads it).
			P.Target = bCross && Skaters.IsValidIndex(S.CrossTarget) ? S.CrossTarget : -1;
			if (P.Target < 0)
			{
				double Best = 7.0;
				const FVector2D Fw(FMath::Cos(S.Yaw), FMath::Sin(S.Yaw));
				for (const FJamSkaterState& O : Skaters)
				{
					if (O.Team == S.Team || O.IsDown() || O.InBox()) continue;
					const FVector2D D = O.Pos - S.Pos;
					const double Dist = D.Size();
					if (Dist < Best && Dist > 1e-3 && FVector2D::DotProduct(D / Dist, Fw) >= FMath::Cos(FMath::DegreesToRadians(70.0))) { Best = Dist; P.Target = O.Index; }
				}
			}
			Presses.Add(P);
		}
	}
	// Evolution 10: the tip's meeting with the target, judged on the sim's places each frame (the tip as Draw places it).
	for (FPress& P : Presses)
	{
		if (P.EndFrame >= 0 || P.TipMetFrame >= 0 || !Skaters.IsValidIndex(P.Target) || !Skaters.IsValidIndex(P.Skater)) continue;
		const FJamSkaterState& Cs = Skaters[P.Skater];
		const FJamSkaterState& Ts = Skaters[P.Target];
		const FVector2D D = Ts.Pos - Cs.Pos;
		const double Gap = D.Size();
		const double Closing = Gap > 1e-6 ? FVector2D::DotProduct(Cs.Vel - Ts.Vel, D / Gap) : 0.0;
		// Met: the tip's run is done (at most a frame to go): it stands on his near side.
		if (TipRemainFrames(Gap, Closing, static_cast<float>(Frame - P.PressFrame), P.Windup) <= 1.0) P.TipMetFrame = Frame;
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
		B.Weight = R == EJamHitResult::Bump ? 0.f : BurstWeight(Closing);
		B.Checker = A.Index; B.Victim = v;
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
			L.TipMet = P->TipMetFrame >= 0 ? static_cast<int32>(P->TipMetFrame - P->PressFrame) : -1;
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
		B.Weight = BurstWeight(static_cast<double>(E.Strength) * SkaterSpeed);
		B.Checker = E.Who;
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
		// Evolution 10 item 4: a body check's miss is the sim's own (ChecksMissed grew: the HUD's MISSED reads the same counter), so the two agree on
		// the frame; a window that closed with no miss counted (he took the puck, the press was spent) ends quietly. A man down keeps his press (the sim
		// still counts its window), drawn nothing while he is down.
		const bool bMissedNow = !P.bCross && PrevMissed.IsValidIndex(P.Skater) && S.ChecksMissed > PrevMissed[P.Skater];
		if ((P.bCross || !bMissedNow) && S.InBox()) { P.EndFrame = Frame; P.Result = EJamHitResult::None; continue; }
		if (P.bCross && S.IsDown()) { P.EndFrame = Frame; P.Result = EJamHitResult::None; continue; }
		bool bOver = false;
		if (!P.bCross) bOver = bMissedNow || (Frame > P.PressFrame && S.CheckWindupLeft == 0 && S.CheckWindow == 0);
		else bOver = Frame >= P.PressFrame + P.Windup && S.LiftWindupLeft == 0;
		if (!bOver) continue;
		P.EndFrame = Frame;
		if (!P.bCross && !bMissedNow) { P.Result = EJamHitResult::None; continue; }
		P.Result = P.bStripped ? EJamHitResult::Strip : EJamHitResult::Miss;
		if (P.Result == EJamHitResult::Miss) ++Misses;
		FHitLog L;
		L.Frame = Frame; L.Kind = P.bCross ? TEXT("cross") : TEXT("check"); L.Result = P.Result; L.Checker = P.Skater; L.bHuman = P.bHuman; L.Windup = P.Windup;
		FrameLogs.Add(L);
	}
	PrevMissed.SetNum(N);
	for (int32 i = 0; i < N; ++i) PrevMissed[i] = Skaters[i].ChecksMissed;
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
			FVector2D F = YawDir(Yaw[P.Skater]);
			const FVector C = At[P.Skater];
			double GapM = -1.0, ClosingMps = 0.0;
			if (!P.bCross && At.IsValidIndex(P.Target) && !(Down.IsValidIndex(P.Target) && Down[P.Target]))
			{
				const FVector2D ToT((At[P.Target].X - C.X) / 100.0, (At[P.Target].Y - C.Y) / 100.0);
				if (ToT.Size() > 0.05) { GapM = ToT.Size(); F = ToT / GapM; }
				if (GapM > 0.0 && VelM.IsValidIndex(P.Skater) && VelM.IsValidIndex(P.Target)) ClosingMps = FVector2D::DotProduct(VelM[P.Skater] - VelM[P.Target], F);
			}
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
				// Lane A210: the quiet chevron (Sw.bThinTell): the ice's own white, 7 cm wide, arms of 60 %: a direction cue, not a slab in the team's colour.
				const FLinearColor Hot = bMiss ? Col : Sw.bThinTell ? FLinearColor::White : FMath::Lerp(Col, FLinearColor::White, 0.45f);
				const FVector2D Side(-F.Y, F.X);
				// Evolution 10: the tip runs from his body's edge to the target's near side over the wind-up the call carried (TipDistM); no target:
				// TellNearM to TellFarM ahead as before. The arms shorten when the tip is close to him (never back over his own body).
				const double TipM = (bMiss ? TipDistM(GapM, static_cast<float>(P.EndFrame - P.PressFrame), P.Windup, ClosingMps) : TipDistM(GapM, Since, P.Windup, ClosingMps)) + Slide;
				const double Tip = TipM * 100.0 * Sc;
				const double ArmLen = FMath::Min(95.0 * Scale, FMath::Max(25.0, (TipM - UJamSim::SkaterRadius) * 100.0 * 0.9)) * Sc * (Sw.bThinTell ? 0.6 : 1.0), ArmDeg = 28.0;
				for (int32 k = -1; k <= 1; k += 2)
				{
					const double A = FMath::DegreesToRadians(180.0 - k * ArmDeg);   // from the tip back and out to each side
					const FVector2D ArmDir = F * FMath::Cos(A) + Side * FMath::Sin(A);
					const FVector2D Mid = F * (Tip * 1.0) + ArmDir * (ArmLen * 0.5);
					FJamHitBar B;
					B.CentreCm = C + FVector(Mid.X, Mid.Y, GroundZ + 2.0);
					B.YawDeg = static_cast<float>(FMath::RadiansToDegrees(FMath::Atan2(ArmDir.Y, ArmDir.X)));
					B.SizeCm = FVector(ArmLen, (Sw.bThinTell ? 7.0 : 18.0) * Sc, 4.0);
					B.Colour = Hot; B.Emissive = Em * (Sw.bThinTell ? 0.7f : 1.4f); B.Layer = static_cast<uint8>(EJamHitLayer::Tell);
					Out.Add(B);
				}
			}
			else if (Sw.bStreak)
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
			if (Sw.bBraceBar) Out.Add(B);
			if (Sw.bBraceArc) Arc(Out, At[i], F, BraceRadiusM * 100.0 * Sc, 7, 42.0, (Sw.bThinTell ? 7.0 : 16.0) * Sc, 4.0, GroundZ, Sw.bThinTell ? FLinearColor::White : FMath::Lerp(Team, FLinearColor::White, 0.35f), Sw.bThinTell ? 1.5f + 2.f * K : 2.f + 5.f * K, EJamHitLayer::Brace);
		}
	}
	// AT CONTACT: the bursts.
	if (Sw.bBurst)
	{
		for (const FBurst& B : Bursts)
		{
			// Lane A210: the small ring (Sw.RingScale < 1: the radius scaled, never under RingMinM so it clears the two bodies; Sw.RingLife: its share of
			// BurstFrames; Sw.bRingTeam false: the ice's own white all the way, no slab in the checker's colour).
			const float A = static_cast<float>((FrameF - static_cast<double>(B.Frame)) / (BurstFrames * FMath::Clamp(Sw.RingLife, 0.1f, 1.f)));
			if (A < 0.f || A >= 1.f) continue;
			const FVector C(B.AtM.X * 100.0, B.AtM.Y * 100.0, 0.0);
			const FLinearColor Col = Sw.bRingTeam ? FMath::Lerp(FLinearColor::White, B.Colour, FMath::Min(1.f, A * 2.f)) : FLinearColor::White;
			const double RingM = Sw.RingScale < 1.f ? FMath::Max(RingMinM, B.RadiusM * Sw.RingScale) : B.RadiusM;
			const float WideK = Sw.RingScale < 1.f ? 0.45f : 1.f;
			// Evolution 10: width, brightness and count by the hit's weight, all falling from the first frame; nothing within BodyClearM of either body.
			const float Em = BurstEmissive(B.Weight, A);
			const double R = RingM * 100.0 * Sc * (Sw.RingScale < 1.f ? 0.8 + 0.2 * EaseOut(A) : 0.45 + 0.55 * EaseOut(A));
			const int32 Segs = 10 + FMath::RoundToInt(6.f * B.Weight);
			const int32 First = Out.Num();
			Ring(Out, C, R, Segs, Segs, BurstBarWidthCm(B.Weight, A) * WideK * Sc, 4.0, GroundZ, Col, Em * WideK, EJamHitLayer::Burst, 0.6);
			if (B.bBraced && A > 0.15f && Sw.RingScale >= 1.f) Ring(Out, C, R * 0.62, 12, 12, BurstBarWidthCm(B.Weight, A) * 0.7f * Sc, 4.0, GroundZ, Col, Em, EJamHitLayer::Burst, 0.6);
			const auto ClearOfBoth = [&](const FVector& Pc)
			{
				const FVector2D P(Pc.X / 100.0, Pc.Y / 100.0);
				for (const int32 Who : { B.Checker, B.Victim })
				{
					if (!At.IsValidIndex(Who)) continue;
					const FVector2D Body(At[Who].X / 100.0, At[Who].Y / 100.0);
					if (!ClearOfBody(P, Body, FallM.IsValidIndex(Who) ? FallM[Who] : FVector2D::ZeroVector, BodyClearM)) return false;
				}
				return true;
			};
			for (int32 k = Out.Num() - 1; k >= First; --k) if (!ClearOfBoth(Out[k].CentreCm)) Out.RemoveAt(k);
			if (B.Weight < 0.25f) continue;   // a bump or a light hit: the ring only
			const int32 Count = ShardCount(B.bBraced);
			const double Base = FMath::Atan2(-B.DirM.Y, -B.DirM.X);
			for (int32 i = 0; i < Count; ++i)
			{
				const double Ang = Base + 2.0 * UE_DOUBLE_PI * (i + 0.5) / Count;
				const FVector2D D(FMath::Cos(Ang), FMath::Sin(Ang));
				if (!ShardAllowed(D, B.FallM)) continue;
				const double Dist = RingM * 100.0 * Sc * (Sw.RingScale < 1.f ? 0.9 + 0.5 * EaseOut(A) : 0.25 + 0.6 * EaseOut(A));
				FJamHitBar S;
				// A victim who stays up: shards that rise (45 to 80 cm); one who goes down: ice spray that stays on the ice (nothing over the lying figure).
				const bool bLow = !B.FallM.IsNearlyZero();
				S.CentreCm = C + FVector(D.X, D.Y, 0.0) * Dist + FVector(0, 0, bLow ? GroundZ + 2.0 : 45.0 + 35.0 * A);
				S.YawDeg = static_cast<float>(FMath::RadiansToDegrees(Ang));
				S.PitchDeg = bLow ? 0.f : 25.f;
				S.SizeCm = FVector((60.0 * (1.0 - A) + 15.0) * Sc * (Sw.RingScale < 1.f ? 0.6 : 1.0), (bLow ? 14.0 : 10.0) * WideK, bLow ? 4.0 : 10.0 * WideK);
				S.Colour = Col; S.Emissive = Em * WideK; S.Layer = static_cast<uint8>(EJamHitLayer::Shard);
				if (ClearOfBoth(S.CentreCm)) Out.Add(S);
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

void AJamHitRead::TellTakes(const UJamSim* Sim, const TArray<TObjectPtr<AJamSkater>>& Skaters)
{
	// PROPS READY (device B, dev-b 1da715c1): B's wind-up takes are behind its own switch, SetWindupTakes(bool bOn, int32 N) (N 0 = 8): told by name to each
	// figure on spawn and again when the figure or the cvar changes. jam.hit.BTakes: 1 (default) the bots' figures only (the human's 4-frame wind-up
	// starts B's take inside its coil: B and the strips read it as a dip, so the human keeps A's lean), 2 every figure, 0 none.
	const int32 Mode = FMath::Clamp(CVarJamHitBTakes.GetValueOnGameThread(), 0, 2);
	static const FName NameTakes(TEXT("SetWindupTakes"));
	for (AJamSkater* Sk : Skaters)
	{
		if (!Sk || !Sim->GetSkaters().IsValidIndex(Sk->GetSimIndex())) continue;
		const int32 I = Sk->GetSimIndex();
		if (TakesOn.Num() <= I) { TakesOn.SetNum(I + 1); TakesFig.SetNum(I + 1); TakesWant.SetNum(I + 1); }
		UObject* V = Sk->GetVisualObject();
		const bool bWant = Mode == 2 || (Mode == 1 && !Sim->GetSkaters()[I].bHuman);
		if (TakesFig[I].Get() == V && TakesWant[I] == bWant) continue;
		TakesFig[I] = V;
		TakesWant[I] = bWant;
		const bool bHas = FigureHas(V, TEXT("SetWindupTakes"));
		const bool bSent = bHas && AJamSkater::CallFigure(V, NameTakes, { bWant ? 1.0 : 0.0, 8.0 });
		TakesOn[I] = bSent && bWant;
		UE_LOG(LogJamHit, Log, TEXT("JAMHIT btakes frame=%lld skater=%d human=%d on=%d call=%s"), Sim->GetFrame(), I, Sim->GetSkaters()[I].bHuman ? 1 : 0, TakesOn[I] ? 1 : 0,
			bSent ? TEXT("SetWindupTakes") : TEXT("none"));
	}
}

bool AJamHitRead::OffScreenOn() { return CVarJamHitOffScreen.GetValueOnGameThread() != 0; }

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
	Slot->HumanTeam = Sim->GetHumanTeam();
	Slot->SimFrame = Sim->GetFrame();
	// Evolution 10: the edge arrows' widget, over the HUD, on the local player's screen (none in a headless run).
	if (!Slot->Edge)
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (PC->IsLocalController() && !IsRunningCommandlet() && World->GetGameViewport())
			{
				Slot->Edge = CreateWidget<UJamHitEdgeWidget>(PC, UJamHitEdgeWidget::StaticClass());
				if (Slot->Edge) { Slot->Edge->Source = Slot; Slot->Edge->SetVisibility(ESlateVisibility::HitTestInvisible); Slot->Edge->AddToViewport(40); }
			}
		}
	}
	for (const FJamHitReadModel::FHitLog& L : Slot->Model.GetFrameLogs())
	{
		UE_LOG(LogJamHit, Log, TEXT("JAMHIT frame=%lld kind=%s result=%s checker=%d victim=%d human=%d closing=%.1f burst_m=%.2f windup=%d press_to_contact=%d tip_met=%d"),
			L.Frame, *L.Kind, HitResultName(L.Result), L.Checker, L.Victim, L.bHuman ? 1 : 0, L.ClosingMps, L.BurstM, L.Windup, L.PressToContact, L.TipMet);
	}
	Slot->TellTakes(Sim, Skaters);
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
		// Lane A210 (jam.hit.Capture N with jam.hit.CaptureBots 1): a bot's press within CaptureBotsM of the human is a strip too (botcheck / botcross):
		// the bots' takes at their own wind-ups (8 / 12 frames) from the game camera.
		if (Kind.IsEmpty() && CVarJamHitCaptureBots.GetValueOnGameThread() != 0)
		{
			for (const FJamSkaterState& B : Sim->GetSkaters())
			{
				if (B.bHuman || !(B.bCheckThisFrame || B.bLiftThisFrame) || FVector2D::Distance(B.Pos, S.Pos) > CaptureBotsM) continue;
				Kind = B.bCheckThisFrame ? TEXT("botcheck") : TEXT("botcross");
				Until = F + FMath::Max(B.WindupFrames, 0) + (B.bCheckThisFrame ? 6 : 0) + 30;
				UE_LOG(LogJamHit, Log, TEXT("JAMHIT capture bot kind=%s frame=%lld skater=%d windup=%d dist_m=%.1f"), *Kind, F, B.Index, B.WindupFrames, FVector2D::Distance(B.Pos, S.Pos));
				break;
			}
		}
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
	// Lane A210: -JamHitShots=<absolute folder> on the command line sends the stills there (two capture runs side by side in one checkout
	// cannot share Saved/Screenshots); absent: the engine's own folder, as before.
	static FString ShotDir;
	static bool bShotDirRead = false;
	if (!bShotDirRead) { bShotDirRead = true; FParse::Value(FCommandLine::Get(), TEXT("-JamHitShots="), ShotDir); ShotDir.ReplaceInline(TEXT("\\"), TEXT("/")); }
	const FString ShotName = FString::Printf(TEXT("a196-%s-%06lld-f%06lld.png"), *CaptureKind, CaptureFrom, F);
	FScreenshotRequest::RequestScreenshot(ShotDir.IsEmpty() ? ShotName : ShotDir / ShotName, true, false);
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
	DrawnAtCm = At;
	FJamHitReadModel::FDrawSwitches Sw;
	Sw.bTell = CVarJamHitTell.GetValueOnGameThread() != 0;
	Sw.bBurst = CVarJamHitBurst.GetValueOnGameThread() != 0;
	Sw.bDown = CVarJamHitDown.GetValueOnGameThread() != 0;
	Sw.bFlash = CVarJamHitPuckFlash.GetValueOnGameThread() != 0;
	Sw.Scale = CVarJamHitScale.GetValueOnGameThread();
	Sw.bStreak = CVarJamHitStreak.GetValueOnGameThread() != 0;
	Sw.bBraceBar = CVarJamHitBraceBar.GetValueOnGameThread() != 0;
	Sw.bBraceArc = CVarJamHitBraceArc.GetValueOnGameThread() != 0;
	if (CVarJamHitQuiet.GetValueOnGameThread() != 0)
	{
		Sw.bThinTell = true; Sw.bRingTeam = false; Sw.RingLife = 8.f / BurstFramesF; Sw.RingScale = FMath::Clamp(CVarJamHitRingScale.GetValueOnGameThread(), 0.1f, 0.99f);
	}
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
			// B's wind-up takes play only on a figure told SetWindupTakes(true) (jam.hit.BTakes): the lean stands down there and nowhere else; the brace's
			// take (SetCrossCheckHold) plays on any figure that declares it.
			const bool bBTake = bBraceLean ? FigureHas(Fig, Take) : (TakesOn.IsValidIndex(I) && TakesOn[I] && FigureHas(Fig, Take));
			if (bBTake) continue;
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

// ---------------------------------------------------------------- evolution 10: a man down off screen

bool JamHitEdge::IsOffScreen(const FVector2D& P, bool bInFront, const FVector2D& Size)
{
	return !bInFront || P.X < 0.0 || P.Y < 0.0 || P.X > Size.X || P.Y > Size.Y;
}

FVector2D JamHitEdge::EdgePlace(const FVector2D& Size, const FVector2D& Dir, float Inset)
{
	const FVector2D C = Size * 0.5;
	FVector2D D = Dir.IsNearlyZero() ? FVector2D(0, 1) : Dir.GetSafeNormal();
	const double Hx = FMath::Max(1.0, C.X - Inset), Hy = FMath::Max(1.0, C.Y - Inset);
	const double Tx = FMath::Abs(D.X) > 1e-6 ? Hx / FMath::Abs(D.X) : 1e9;
	const double Ty = FMath::Abs(D.Y) > 1e-6 ? Hy / FMath::Abs(D.Y) : 1e9;
	return C + D * FMath::Min(Tx, Ty);
}

void JamHitEdge::Outline(const FJamEdgeArrow& A, TArray<FVector2f>& Out)
{
	Out.Reset();
	const FVector2D F(FMath::Cos(A.AngleRad), FMath::Sin(A.AngleRad)), S(-F.Y, F.X);
	const double L = ArrowPx;
	auto P = [&](double f, double s) { const FVector2D V = A.PosPx + F * f + S * s; Out.Add(FVector2f(V)); };
	if (A.bMate)
	{
		// A hollow diamond with its tip pulled out towards him.
		P(L * 0.75, 0); P(0, L * 0.4); P(-L * 0.45, 0); P(0, -L * 0.4); P(L * 0.75, 0);
	}
	else
	{
		// A solid triangle (drawn as nested outlines).
		for (double k = 1.0; k > 0.15; k -= 0.2) { P(L * 0.6 * k, 0); P(-L * 0.4 * k, L * 0.45 * k); P(-L * 0.4 * k, -L * 0.45 * k); P(L * 0.6 * k, 0); }
	}
}

void UJamHitEdgeWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	Arrows.Reset();
	AJamHitRead* H = Source.Get();
	APlayerController* PC = GetOwningPlayer();
	if (!H || !PC || !AJamHitRead::OffScreenOn() || !PC->PlayerCameraManager) return;
	const FVector2D Size = MyGeometry.GetLocalSize();
	if (Size.X < 10.0 || Size.Y < 10.0) return;
	const FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();
	const FRotationMatrix CamM(PC->PlayerCameraManager->GetCameraRotation());
	const FVector Fw = CamM.GetUnitAxis(EAxis::X), Rt = CamM.GetUnitAxis(EAxis::Y), Up = CamM.GetUnitAxis(EAxis::Z);
	const FJamHitReadModel& M = H->GetModel();
	for (const FJamHitReadModel::FDown& D : M.GetDowns())
	{
		if (!H->DrawnAtCm.IsValidIndex(D.Skater)) continue;
		// His body's middle (the oval's centre), 30 cm up.
		const FVector2D Dir2 = D.DirM.IsNearlyZero() ? FVector2D(1, 0) : D.DirM.GetSafeNormal();
		const FVector W = H->DrawnAtCm[D.Skater] + FVector(Dir2.X, Dir2.Y, 0.0) * (FJamHitReadModel::DownBodyM * 50.0) + FVector(0, 0, 30.0);
		FVector2D P;
		const FVector V = W - CamLoc;
		const bool bInFront = FVector::DotProduct(V, Fw) > 1.0 && UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, W, P, false);
		if (!JamHitEdge::IsOffScreen(P, bInFront, Size)) continue;
		// The way to him on the screen: from the camera's right and up (works behind the camera too).
		const FVector2D Way(FVector::DotProduct(V, Rt), -FVector::DotProduct(V, Up));
		FJamEdgeArrow A;
		A.PosPx = JamHitEdge::EdgePlace(Size, bInFront ? (P - Size * 0.5) : Way, JamHitEdge::EdgeInsetPx);
		const FVector2D Pt = bInFront ? (P - A.PosPx) : Way;
		A.AngleRad = static_cast<float>(FMath::Atan2(Pt.Y, Pt.X));
		const int32 Team = M.TeamOf(D.Skater);
		A.bMate = Team == H->HumanTeam;
		A.Colour = M.ColourOf(Team);
		Arrows.Add(A);
		const int64 DownAt = H->SimFrame - FMath::Max(0, D.Total - D.Left);
		if (const int64* Was = Logged.Find(D.Skater); !Was || *Was != DownAt)
		{
			Logged.Add(D.Skater, DownAt);
			UE_LOG(LogJamHit, Log, TEXT("JAMHIT offscreen frame=%lld skater=%d down_at=%lld delay=%lld mate=%d"), H->SimFrame, D.Skater, DownAt, H->SimFrame - DownAt, A.bMate ? 1 : 0);
		}
	}
}

int32 UJamHitEdgeWidget::NativePaint(const FPaintArgs& Args, const FGeometry& Geo, const FSlateRect& Cull, FSlateWindowElementList& Out, int32 Layer,
	const FWidgetStyle& Style, bool bParentEnabled) const
{
	const int32 L = Super::NativePaint(Args, Geo, Cull, Out, Layer, Style, bParentEnabled);
	TArray<FVector2f> Pts;
	for (const FJamEdgeArrow& A : Arrows)
	{
		JamHitEdge::Outline(A, Pts);
		// A dark under-stroke so the arrow reads on pale ice and on the crowd alike.
		FSlateDrawElement::MakeLines(Out, L + 1, Geo.ToPaintGeometry(), Pts, ESlateDrawEffect::None, FLinearColor(0, 0, 0, 0.7f), true, 7.f);
		FSlateDrawElement::MakeLines(Out, L + 2, Geo.ToPaintGeometry(), Pts, ESlateDrawEffect::None, A.Colour, true, 4.f);
	}
	return L + 2;
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
	TestTrue(TEXT("the burst's ring is drawn"), Count(Bars, EJamHitLayer::Burst) > 0);
	TestTrue(TEXT("shards are drawn"), Count(Bars, EJamHitLayer::Shard) > 0);
	// Evolution 10: nothing of the burst within BodyClearM of either body (the checker at 0 m, the victim at 3 m lying along +x).
	for (const FJamHitBar& B : Bars)
	{
		if (B.Layer != static_cast<uint8>(EJamHitLayer::Burst) && B.Layer != static_cast<uint8>(EJamHitLayer::Shard)) continue;
		const FVector2D P(B.CentreCm.X / 100.0, B.CentreCm.Y / 100.0);
		TestTrue(TEXT("the burst keeps clear of the checker"), FJamHitReadModel::ClearOfBody(P, S[0].Pos, FVector2D::ZeroVector, FJamHitReadModel::BodyClearM - 1e-6));
		TestTrue(TEXT("the burst keeps clear of the victim"), FJamHitReadModel::ClearOfBody(P, S[1].Pos, S[1].KnockDir, FJamHitReadModel::BodyClearM - 1e-6));
	}
	// Evolution 10: a light hit is a third of evolution 9's (width 8 cm against 31 at its first frame), and the weight falls from the first frame.
	TestTrue(TEXT("a light hit's bars a third of before"), FJamHitReadModel::BurstBarWidthCm(FJamHitReadModel::BurstWeight(1.4), 0.f) <= 31.f / 3.f + 0.5f);
	TestTrue(TEXT("a hard hit's bars wider than a light one's"), FJamHitReadModel::BurstBarWidthCm(1.f, 0.f) > 2.5f * FJamHitReadModel::BurstBarWidthCm(0.1f, 0.f));
	TestTrue(TEXT("dimmer from the first frame"), FJamHitReadModel::BurstEmissive(0.5f, 0.1f) < FJamHitReadModel::BurstEmissive(0.5f, 0.f));
	TestTrue(TEXT("narrower from the first frame"), FJamHitReadModel::BurstBarWidthCm(0.5f, 0.1f) < FJamHitReadModel::BurstBarWidthCm(0.5f, 0.f));
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
		if (F >= 26) S[0].ChecksMissed = 1;   // the sim counts the miss on the frame the window runs out (evolution 10: the model follows it)
		M.Step(F, S, None, 8.0, Cols);
		if (MissAt < 0 && M.Misses > 0) MissAt = F;
	}
	TestEqual(TEXT("one miss"), M.Misses, 1);
	{
		// Evolution 10 item 4: a window that closes with no miss counted by the sim (he took the puck) ends quietly: no miss, no grey.
		FJamHitReadModel Q;
		TArray<FJamSkaterState> T2 = Two();
		T2[0].bCheckThisFrame = true; T2[0].WindupFrames = 4; T2[0].CheckWindupLeft = 4;
		Q.Step(10, T2, None, 8.0, Cols);
		for (int64 F = 11; F <= 30; ++F)
		{
			ClearFrame(T2);
			const int32 Wu = FMath::Max(0, 14 - static_cast<int32>(F));
			T2[0].CheckWindupLeft = Wu;
			T2[0].CheckWindow = Wu > 0 ? 0 : FMath::Max(0, 26 - static_cast<int32>(F));
			Q.Step(F, T2, None, 8.0, Cols);
		}
		TestEqual(TEXT("no miss the sim did not count"), Q.Misses, 0);
	}
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHitOffScreen, "Jam.Hit.OffScreen", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHitOffScreen::RunTest(const FString&)
{
	const FVector2D Size(1920, 1080);
	TestFalse(TEXT("a man on screen: no arrow"), JamHitEdge::IsOffScreen(FVector2D(900, 500), true, Size));
	TestTrue(TEXT("a man past the right edge: an arrow"), JamHitEdge::IsOffScreen(FVector2D(2100, 500), true, Size));
	TestTrue(TEXT("a man behind the camera: an arrow"), JamHitEdge::IsOffScreen(FVector2D(900, 500), false, Size));
	TestEqual(TEXT("the switch is on by default"), AJamHitRead::OffScreenOn(), true);
	// Every direction: the arrow on the inset edge, inside the screen, pointing out at him, never over him (his point is off screen, the arrow's box on it).
	for (int32 Deg = 0; Deg < 360; Deg += 15)
	{
		const FVector2D D(FMath::Cos(FMath::DegreesToRadians(Deg)), FMath::Sin(FMath::DegreesToRadians(Deg)));
		const FVector2D Him = Size * 0.5 + D * 1400.0;   // off screen that way
		FJamEdgeArrow A;
		A.PosPx = JamHitEdge::EdgePlace(Size, Him - Size * 0.5, JamHitEdge::EdgeInsetPx);
		A.AngleRad = static_cast<float>(FMath::Atan2((Him - A.PosPx).Y, (Him - A.PosPx).X));
		const bool bOnInset = FMath::IsNearlyEqual(A.PosPx.X, JamHitEdge::EdgeInsetPx, 0.5) || FMath::IsNearlyEqual(A.PosPx.X, Size.X - JamHitEdge::EdgeInsetPx, 0.5)
			|| FMath::IsNearlyEqual(A.PosPx.Y, JamHitEdge::EdgeInsetPx, 0.5) || FMath::IsNearlyEqual(A.PosPx.Y, Size.Y - JamHitEdge::EdgeInsetPx, 0.5);
		TestTrue(FString::Printf(TEXT("%d deg: on the inset edge"), Deg), bOnInset);
		for (const bool bMate : { true, false })
		{
			A.bMate = bMate;
			TArray<FVector2f> Pts;
			JamHitEdge::Outline(A, Pts);
			FBox2D Box(ForceInit);
			for (const FVector2f& P : Pts) Box += FVector2D(P);
			TestTrue(FString::Printf(TEXT("%d deg: the arrow inside the screen"), Deg), Box.Min.X >= 0 && Box.Min.Y >= 0 && Box.Max.X <= Size.X && Box.Max.Y <= Size.Y);
			TestFalse(FString::Printf(TEXT("%d deg: the arrow not over him"), Deg), Box.IsInside(Him));
			// The arrow's tip is its farthest point towards him.
			const FVector2D Tip(Pts[0]);
			TestTrue(FString::Printf(TEXT("%d deg: it points at him"), Deg), FVector2D::Distance(Tip, Him) < FVector2D::Distance(A.PosPx, Him));
		}
	}
	// The two sides by shape: a mate's outline is one closed diamond (5 points), an opponent's nested triangles (more).
	FJamEdgeArrow M; M.bMate = true; FJamEdgeArrow O; O.bMate = false;
	TArray<FVector2f> Pm, Po;
	JamHitEdge::Outline(M, Pm); JamHitEdge::Outline(O, Po);
	TestEqual(TEXT("a mate: the hollow diamond"), Pm.Num(), 5);
	TestTrue(TEXT("an opponent: the solid triangle"), Po.Num() > 5);
	// The radar marks every downed man by side.
	FJamPresentationFrame F;
	F.RinkLengthM = 60.f; F.RinkWidthM = 30.f; F.HumanIndex = 0;
	FJamPresentationSkater Hs; Hs.TeamId = 0; Hs.bHuman = true; F.Skaters.Add(Hs);
	F.DownsM = { FVector2D(10, 5), FVector2D(-10, -5) };
	F.DownsTeam = { 0, 1 };
	TArray<FJamRadarEntry> E;
	UJamRadarWidget::BuildEntries(F, E);
	int32 Mates = 0, Opps = 0;
	for (const FJamRadarEntry& X : E) { Mates += X.DownMark == 1; Opps += X.DownMark == 2; }
	TestEqual(TEXT("the radar: one mate down"), Mates, 1);
	TestEqual(TEXT("the radar: one opponent down"), Opps, 1);
	return true;
}

// Lane A210 (the night, R-0067): "crosscheck has bug in animation displaying technical blue rectangle": the two bars over the figure ship off, each behind
// its own switch; the marks that stay are quiet (thin, the ice's white, a contact ring that keeps to 1.35 m and 8 frames). The struct's defaults are
// package H34's (the tests above draw with them); the game's are the cvars'.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHitQuiet, "Jam.Hit.Quiet", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHitQuiet::RunTest(const FString&)
{
	using namespace JamHitTest;
	TestEqual(TEXT("ships: no bar across the stick of a pressed cross-check"), CVarJamHitStreak.GetValueOnGameThread(), 0);
	TestEqual(TEXT("ships: no bar across the chest of a held cross-check"), CVarJamHitBraceBar.GetValueOnGameThread(), 0);
	TestEqual(TEXT("ships: the quiet marks"), CVarJamHitQuiet.GetValueOnGameThread(), 1);
	TestEqual(TEXT("ships: device B's takes on every figure, the human's too"), CVarJamHitBTakes.GetValueOnGameThread(), 2);
	const auto DrawWith = [](const FJamHitReadModel& M, double FrameF, const TArray<FJamSkaterState>& S, const FJamHitReadModel::FDrawSwitches& Sw, TArray<FJamHitBar>& Out)
	{
		TArray<FVector> At; TArray<float> Yaw;
		for (const FJamSkaterState& X : S) { At.Add(FVector(X.Pos.X * 100.0, X.Pos.Y * 100.0, 0.0)); Yaw.Add(static_cast<float>(FMath::RadiansToDegrees(X.Yaw))); }
		Out.Reset();
		M.Draw(FrameF, At, Yaw, FVector(500, 0, 0), Sw, Out);
	};
	const auto Above = [](const TArray<FJamHitBar>& Bars, double Cm) { int32 N = 0; for (const FJamHitBar& B : Bars) if (B.CentreCm.Z > Cm) ++N; return N; };
	TArray<FJamSimEvent> None;
	TArray<FJamHitBar> Bars;
	FJamHitReadModel::FDrawSwitches H34, Now;
	Now.bStreak = false; Now.bBraceBar = false; Now.bThinTell = true; Now.bRingTeam = false; Now.RingScale = 0.45f; Now.RingLife = 8.f / FJamHitReadModel::BurstFrames;
	{
		// A pressed cross-check, two frames into its wind-up.
		FJamHitReadModel M;
		TArray<FJamSkaterState> S = Two();
		S[0].bLiftThisFrame = true; S[0].WindupFrames = 8;
		M.Step(100, S, None, 8.0, Cols);
		ClearFrame(S);
		M.Step(101, S, None, 8.0, Cols);
		DrawWith(M, 101.5, S, H34, Bars);
		TestEqual(TEXT("package H34: one bar across the stick"), Count(Bars, EJamHitLayer::Streak), 1);
		TestTrue(TEXT("package H34: it hangs over the figure (chest height)"), Above(Bars, 50.0) >= 1);
		DrawWith(M, 101.5, S, Now, Bars);
		TestEqual(TEXT("now: no bar across the stick"), Count(Bars, EJamHitLayer::Streak), 0);
		TestEqual(TEXT("now: nothing drawn above the ice while he winds up"), Above(Bars, 50.0), 0);
	}
	{
		// A held cross-check at 7 m/s.
		FJamHitReadModel M;
		TArray<FJamSkaterState> S = Two();
		S[0].bCrossBraced = true; S[0].Vel = FVector2D(7, 0);
		M.Step(100, S, None, 8.0, Cols);
		DrawWith(M, 100.5, S, H34, Bars);
		TestEqual(TEXT("package H34: the bar across the chest and seven bars of arc"), Count(Bars, EJamHitLayer::Brace), 8);
		TestEqual(TEXT("package H34: the bar is over the figure"), Above(Bars, 50.0), 1);
		DrawWith(M, 100.5, S, Now, Bars);
		TestEqual(TEXT("now: the arc alone"), Count(Bars, EJamHitLayer::Brace), 7);
		TestEqual(TEXT("now: nothing over the figure while he holds"), Above(Bars, 50.0), 0);
		for (const FJamHitBar& B : Bars)
		{
			TestTrue(TEXT("now: the arc is thin (7 cm)"), B.SizeCm.Y <= 7.0 + 1e-6);
			TestTrue(TEXT("now: the arc is the ice's white, not the team's colour"), B.Colour.Equals(FLinearColor::White, 0.001f));
		}
		FJamHitReadModel::FDrawSwitches NoArc = Now; NoArc.bBraceArc = false;
		DrawWith(M, 100.5, S, NoArc, Bars);
		TestEqual(TEXT("the arc has its own switch"), Count(Bars, EJamHitLayer::Brace), 0);
	}
	{
		// A body check's wind-up: the chevron.
		FJamHitReadModel M;
		TArray<FJamSkaterState> S = Two();
		S[0].bCheckThisFrame = true; S[0].WindupFrames = 8; S[0].CheckWindupLeft = 8;
		M.Step(100, S, None, 8.0, Cols);
		DrawWith(M, 100.5, S, Now, Bars);
		TestEqual(TEXT("now: the chevron is still two bars"), Count(Bars, EJamHitLayer::Tell), 2);
		for (const FJamHitBar& B : Bars)
		{
			TestTrue(TEXT("now: the chevron is thin (7 cm against 18)"), B.SizeCm.Y <= 7.0 + 1e-6);
			TestTrue(TEXT("now: the chevron is white"), B.Colour.Equals(FLinearColor::White, 0.001f));
			TestTrue(TEXT("now: the chevron lies on the ice"), B.CentreCm.Z < 20.0);
		}
		// The contact at 11 m/s (Strength 11 / 8), the victim down.
		for (int64 F = 101; F <= 108; ++F)
		{
			ClearFrame(S);
			S[0].CheckWindupLeft = FMath::Max(0, 108 - static_cast<int32>(F)); S[0].CheckWindow = S[0].CheckWindupLeft > 0 ? 0 : 12;
			TArray<FJamSimEvent> Ev;
			if (F == 108)
			{
				S[1].ContactBy = 0; S[1].ContactOutcome = UJamSim::ContactDown; S[1].KnockFrames = 90; S[1].KnockDir = FVector2D(1, 0);
				FJamSimEvent E; E.Name = JamEvents::Check; E.Who = 0; E.Where = FVector2D(1.5, 0); E.Strength = 11.f / 8.f; E.Dir = FVector2D(1, 0); Ev.Add(E);
			}
			M.Step(F, S, Ev, 8.0, Cols);
		}
		TestEqual(TEXT("one burst"), M.GetBursts().Num(), 1);
		if (M.GetBursts().Num() == 1)
		{
			const FVector C(M.GetBursts()[0].AtM.X * 100.0, M.GetBursts()[0].AtM.Y * 100.0, 0.0);
			const auto Reach = [&C](const TArray<FJamHitBar>& In) { double R = 0.0; for (const FJamHitBar& B : In) if (B.Layer == static_cast<uint8>(EJamHitLayer::Burst)) R = FMath::Max(R, FVector::Dist2D(B.CentreCm, C)); return R / 100.0; };
			DrawWith(M, 116.0, S, H34, Bars);
			const double Before = Reach(Bars);
			const int32 BarsBefore = Count(Bars, EJamHitLayer::Burst);
			DrawWith(M, 112.0, S, Now, Bars);
			const double After = Reach(Bars);
			TestTrue(FString::Printf(TEXT("package H34: the ring of a hard hit reaches past 2 m (%.2f m)"), Before), Before > 2.0);
			TestTrue(FString::Printf(TEXT("now: the ring keeps to 1.35 m (%.2f m)"), After), After <= 1.35 + 1e-6 && After >= FJamHitReadModel::RingMinM * 0.8 - 1e-6);
			TestTrue(TEXT("now: the ring is drawn"), Count(Bars, EJamHitLayer::Burst) > 0 && BarsBefore > 0);
			for (const FJamHitBar& B : Bars)
			{
				if (B.Layer != static_cast<uint8>(EJamHitLayer::Burst) && B.Layer != static_cast<uint8>(EJamHitLayer::Shard)) continue;
				TestTrue(TEXT("now: the ring and its spray are white, never the checker's colour"), B.Colour.Equals(FLinearColor::White, 0.001f));
				TestTrue(TEXT("now: the ring's bars at most 12 cm wide (28 before)"), B.Layer != static_cast<uint8>(EJamHitLayer::Burst) || B.SizeCm.Y <= 12.0);
				const FVector2D P(B.CentreCm.X / 100.0, B.CentreCm.Y / 100.0);
				TestTrue(TEXT("now: still clear of the checker"), FJamHitReadModel::ClearOfBody(P, S[0].Pos, FVector2D::ZeroVector, FJamHitReadModel::BodyClearM - 1e-6));
				TestTrue(TEXT("now: still clear of the victim"), FJamHitReadModel::ClearOfBody(P, S[1].Pos, S[1].KnockDir, FJamHitReadModel::BodyClearM - 1e-6));
			}
			DrawWith(M, 108.0 + 8.5, S, Now, Bars);
			TestEqual(TEXT("now: the ring is gone 8 frames after the contact"), Count(Bars, EJamHitLayer::Burst) + Count(Bars, EJamHitLayer::Shard), 0);
			DrawWith(M, 108.0 + 8.5, S, H34, Bars);
			TestTrue(TEXT("package H34: it burned on to 14"), Count(Bars, EJamHitLayer::Burst) > 0);
			DrawWith(M, 112.0, S, Now, Bars);
			TestTrue(TEXT("the downed man's oval stays"), Count(Bars, EJamHitLayer::Down) > 0);
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
	// Evolution 10 item 3: the wind-up's length scales the lean and the chevron. At 4, 8 and 12 frames: fully back on the wind-up's last frame,
	// and the tip leaves his body at the press and reaches the target's near side (gap 3 m) exactly when the wind-up ends.
	for (const int32 W : { 4, 8, 12 })
	{
		TestEqual(FString::Printf(TEXT("back on frame %d of %d"), W - 1, W), FJamHitReadModel::CheckLeanDeg(W - 1, W, 12, -1), 6.f, 1e-4f);
		TestTrue(FString::Printf(TEXT("forward after a wind-up of %d"), W), FJamHitReadModel::CheckLeanDeg(W + 4, W, 12, -1) < -10.f);
		// Two men 3 m apart closing so the bodies meet W frames after the press (the bots press on that lead): the tip meets him within 2 frames of it.
		const double Closing = (3.0 - 2.0 * UJamSim::SkaterRadius) / (W / 60.0);
		int32 Met = -1;
		for (int32 Sn = 0; Sn <= W + 4 && Met < 0; ++Sn)
		{
			const double Gap = FMath::Max(2.0 * UJamSim::SkaterRadius, 3.0 - Closing * Sn / 60.0);
			if (FJamHitReadModel::TipRemainFrames(Gap, Closing, static_cast<float>(Sn), W) <= 1.0) Met = Sn;
			const double Near = FMath::Max(Gap - UJamSim::SkaterRadius, UJamSim::SkaterRadius);
			TestTrue(FString::Printf(TEXT("the tip never past him (%d, frame %d)"), W, Sn), FJamHitReadModel::TipDistM(Gap, static_cast<float>(Sn), W, Closing) <= Near + 1e-6);
		}
		TestTrue(FString::Printf(TEXT("a wind-up of %d: the tip meets him within 2 frames of the contact (met %d)"), W, Met), Met >= 0 && FMath::Abs(Met - W) <= 2);
		TestTrue(FString::Printf(TEXT("a wind-up of %d: the tip well short of him at the press"), W),
			(3.0 - UJamSim::SkaterRadius) - FJamHitReadModel::TipDistM(3.0, 0.f, W, Closing) > FJamHitReadModel::TipMeetM);
	}
	TestEqual(TEXT("no target: the tip at TellFarM"), FJamHitReadModel::TipDistM(-1.0, 20.f, 4, 0.0), FJamHitReadModel::TellFarM, 1e-6);
	UObject* Plain = GetTransientPackage();
	TestFalse(TEXT("an object without device B's takes has none"), AJamHitRead::FigureHas(Plain, TEXT("PlayCheckWindup")));
	return true;
}

#endif
