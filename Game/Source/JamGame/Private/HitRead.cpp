// Lane A196 (track F3, evolution 9): the hits read (HitRead.h, docs/experiments/HITS.md). Presentation only.
#include "HitRead.h"
#include "JamPawns.h"
#include "JamSkaterVisual.h"
#include "JamEventBus.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
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
#include "HUDWidget.h"   // lane A227: KeepOffPanel
#include "MenuStyle.h"

DEFINE_LOG_CATEGORY_STATIC(LogJamHit, Log, All);

// Lane A219 (R-0069 point 3, 15 Nov: "Crosscheck still displays blue line/rectangle in front of player. This should player never see"): NOTHING of this
// file's bar meshes is drawn at a check or a cross-check by default, for the human and for the bots, on every look: no chevron, streak, chest bar, arc of
// dashes, burst of rectangles, countdown of rectangles round a downed man, ring of rectangles round a stripped puck. The figure and its stick say it
// (device B's takes), the hitter's own ring swells (the HUD's). Each layer keeps its switch: 1 on all five below (jam.hit.Tell, BraceArc, Burst, Down,
// PuckFlash) is package H39's picture. AJamHitRead::GameSwitches() is what the game draws with; Jam.Hit.NoBars holds it at zero bars.
static TAutoConsoleVariable<int32> CVarJamHitTell(TEXT("jam.hit.Tell"), 0,
	TEXT("The tell before a hit (lane A196): 0 (default from lane A219, R-0069 point 3: nothing technical is drawn at a check or a cross-check) none; 1 (package H39) a chevron of light on the ice 1.4 to 2.4 m ahead of a body checker through his wind-up and window (grey out on a miss), a streak across a cross-checker's stick, the held brace's streak and arc. Presentation only."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitLean(TEXT("jam.hit.Lean"), 1,
	TEXT("The figure's lean through a check (lane A196): 1 (default) while the figure lacks device B's PlayCheckWindup / PlayCrossCheck / SetCrossCheckHold: back over the wind-up, forward through the window or the brace; 2 always; 0 none (before). Presentation only."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitBurst(TEXT("jam.hit.Burst"), 0,
	TEXT("The burst at contact (lane A196): 0 (default from lane A219, R-0069: its ring of dashes and its shards are rectangles lying on the ice in front of him) none; 1 (package H39) a ring on the ice sized by the closing speed and shards away from the victim's fall. Presentation only."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitDown(TEXT("jam.hit.Down"), 0,
	TEXT("The downed skater's countdown (lane A196): 0 (default from lane A219, R-0069: a ring of rectangles in his team's colour, blue round the human's side) none: the figure lying on the ice says it; 1 (package H39) a ring of 12 bars on the ice round him that go out as his down frames run. Presentation only."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitPuckFlash(TEXT("jam.hit.PuckFlash"), 0,
	TEXT("The stripped puck's flash (lane A196): 0 (default from lane A219, R-0069: a ring of ten gold rectangles at a cross-check's strip) none; 1 (package H39) a gold ring out from the puck a stick lift took. Presentation only."), ECVF_Default);
static TAutoConsoleVariable<float> CVarJamHitScale(TEXT("jam.hit.Scale"), 1.f,
	TEXT("Size factor of the hits' marks (lane A196, tuning in a window): 1 (default)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitBTakes(TEXT("jam.hit.BTakes"), 2,
	TEXT("Device B's wind-up takes: SetWindupTakes(true, 8) by name on 2 (default from lane A210: every figure, the human's too: the figure does the hitting; at a 4-frame wind-up B's take starts inside its coil and still shows the drive and the follow-through), 1 the bots' figures only (package H34: the human on A's stand-in lean), 0 none. A's lean stands down exactly on the figures told."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitCaptureBots(TEXT("jam.hit.CaptureBots"), 0,
	TEXT("With jam.hit.Capture: 1 = a bot's check or cross-check pressed inside the distance band (jam.hit.CaptureBotsMinM .. CaptureBotsM from the human) is a strip too (botcheck / botcross), and a hit between two bots that lands (bothit, from the contact); 2 = the same, only when the man is inside the picture (20 % in from every edge). Lane A210."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitCaptureBoards(TEXT("jam.hit.CaptureBoards"), 0,
	TEXT("With jam.hit.Capture: 1 = each of the human's board hits is a strip of 24 frames (kind board): the board caps' flash and the board take. Lane A210."), ECVF_Default);
static TAutoConsoleVariable<float> CVarJamHitCaptureBotsM(TEXT("jam.hit.CaptureBotsM"), 14.f,
	TEXT("With jam.hit.CaptureBots 1: a bot's press is filmed when it is between jam.hit.CaptureBotsMinM and this many metres from the human (14: near; 40 with a minimum of 18: the hits at 20 and 35 m). Lane A210."), ECVF_Default);
static TAutoConsoleVariable<float> CVarJamHitCaptureBotsMinM(TEXT("jam.hit.CaptureBotsMinM"), 0.f,
	TEXT("With jam.hit.CaptureBots 1: the nearest a filmed bot press may be to the human, in metres (0). Lane A210."), ECVF_Default);
static TAutoConsoleVariable<float> CVarJamHitBraceSet(TEXT("jam.hit.BraceSet"), 0.f,
	TEXT("The braced figure sets itself (lane A210: a bridge on the FIGURE while device B's hold cannot be seen from behind; set it to 0 when B's pose reads): the drawn figure (never the sim) sinks and widens over 6 frames while the cross-check is held and rises again over 6: height x (1 - 0.10 x this), width x (1 + 0.06 x this). 0 (default since device B's H56.5 hold: the stick past both gloves and the elbows out read from the game camera on their own, and the set at 1.5 squashed the figure beside it); 1.5 = PUCK READY 1."), ECVF_Default);
static TAutoConsoleVariable<float> CVarJamHitBraceLean(TEXT("jam.hit.BraceLean"), 12.f,
	TEXT("With the braced figure's set: degrees the drawn figure pitches forward into the hold, on top of device B's take (12 by default; 0 = none)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarJamHitWhiff(TEXT("jam.hit.Whiff"), 0.8f,
	TEXT("A miss looks like a miss (lane A210, the eye's EYE 7): with jam.hit.Pop, after a check or a cross-check that met nobody the drawn figure tips into a dip (this share of the coil's depth) 4 frames after the miss and is back at its own size 14 frames after it; the figure is also told PlayWhiff(Kind) by name (device B's take, when it exists). 0.8 (default); 0 = the plain settle (READY 3)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitRetimeTest(TEXT("jam.hit.RetimeTest"), 0,
	TEXT("Proof only (lane A210): N > 0 sends PlayCheckWindup(1) to the human's figure N frames after each of his check presses, as jam.hit.Retime would on a contact inside the wind-up (to see what device B's running take does with it before lane A209's sim is here). 0 = off."), ECVF_Default);
static TAutoConsoleVariable<float> CVarJamHitPop(TEXT("jam.hit.Pop"), 0.f,
	TEXT("The hit's coil and drive on the drawn figure (lane A210; never the sim): through a check's or a cross-check's wind-up the figure sinks and widens (height down to x (1 - 0.10 x this)), on the frame before the window opens it springs tall and narrow (x (1 + 0.07 x this)) through the contact, then settles over 8 frames: device B's take, made larger (the helmet's travel from coil to spring at the game camera: 33 px without, 55 px with, at 960 x 540). 0 (default since device B's H56.0 takes: the sink 1.45 times deeper and the stick over the helmet read from behind on their own; the coil on top lowered the raised arms of B's cross-check); 1 = PUCK READY 1. The dip after a miss (jam.hit.Whiff) rides on this and is off with it: the miss is device B's PlayCheckWhiff()."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitRetime(TEXT("jam.hit.Retime"), 1,
	TEXT("A body check that lands before its wind-up is over (lane A209's rule: a meeting inside the wind-up is the hit): 1 (default) the checker's figure is run forward for one sim frame (its time dilation = the frames its take is short of the contact pose), so device B's take stands on its contact pose when the man falls instead of in its coil; 2 = by the call PlayCheckWindup(1) instead (device B's figure ignores it on a running take today); 0 = the take plays on from where it was."), ECVF_Default);
static constexpr float BurstFramesF = static_cast<float>(FJamHitReadModel::BurstFrames);
// Lane A210 (the night, R-0067: "technical blue rectangle"): the drawn bars of a cross-check, each behind its own switch.
static TAutoConsoleVariable<int32> CVarJamHitStreak(TEXT("jam.hit.Streak"), 0,
	TEXT("The bar of light across the stick of a pressed cross-check: 0 (default from lane A210: the operator read it as a bug, R-0067) not drawn, 1 drawn (package H34)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitBraceBar(TEXT("jam.hit.BraceBar"), 0,
	TEXT("The bar of light across the chest of a held cross-check, in the team's colour: 0 (default from lane A210: this is the technical blue rectangle of R-0067) not drawn, 1 drawn (package H34)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitQuiet(TEXT("jam.hit.Quiet"), 1,
	TEXT("The marks as quiet helpers (lane A210): 1 (default) the chevron and the brace's arc thin and in the ice's white, the contact ring small (jam.hit.RingScale), white and gone in 8 frames; 0 = package H34's marks (fat, in the team's colour, a ring to 3 m for 14 frames)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitRingFar(TEXT("jam.hit.RingFar"), 1,
	TEXT("With jam.hit.Quiet 1: 1 (default) the contact ring grows back with the distance from the camera: the small ring within 10 m (the human's own hits), package H34's size and 14 frames from 30 m (a hit across the rink is 30 px of bodies: the ring says where), white at any distance; 0 = the small ring everywhere."), ECVF_Default);
static TAutoConsoleVariable<float> CVarJamHitRingScale(TEXT("jam.hit.RingScale"), 0.45f,
	TEXT("With jam.hit.Quiet 1: the contact ring's radius as a share of package H34's (0.8 to 3.0 m by closing speed), never under 0.95 m. Default 0.45."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHitBraceArc(TEXT("jam.hit.BraceArc"), 0,
	TEXT("The arc of seven dashes on the ice ahead of a held cross-check: 0 (default from lane A219, R-0069: the grey-white rectangles in front of him through every frame of a hold) not drawn; 1 drawn (package H39; it also needs jam.hit.Tell 1)."), ECVF_Default);
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
	Presses.Reset(); Bursts.Reset(); Downs.Reset(); Flashes.Reset(); FrameLogs.Reset(); LastCrossPress.Reset();
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
	if (LastCrossPress.Num() != N) LastCrossPress.Init(-100000, N);
	for (int32 i = 0; i < N; ++i) if (Skaters[i].bLiftThisFrame) LastCrossPress[i] = Frame;
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
		else bOver = Frame >= P.PressFrame + P.Windup && S.LiftWindupLeft == 0 && S.CrossLiveLeft == 0;   // lane A215: the miss is the live window's end (contact.cross_live_frames)
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
			if (Sw.bThinTell && LastCrossPress.IsValidIndex(i) && FrameF - static_cast<double>(LastCrossPress[i]) < ArcAfterTapFrames) continue;   // never after a tap
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
			const FVector C(B.AtM.X * 100.0, B.AtM.Y * 100.0, 0.0);
			// Lane A210 (the hit at 20 and 35 m): the small ring is for a hit NEAR the eye, where it covered the two bodies; far off the bodies are 30 px and
			// the ring is what says 'a hit, there'. Far01: 0 within RingNearM of the eye (the quiet ring), 1 from RingFarM (package H34's size and life,
			// still white): about the same size on the screen at any distance. Sw.bRingByDistance false: near everywhere.
			const float Far01 = Sw.bRingByDistance && Sw.RingScale < 1.f ? static_cast<float>(FMath::Clamp((FVector::Dist(C, Sw.EyeAtCm) / 100.0 - RingNearM) / (RingFarM - RingNearM), 0.0, 1.0)) : 0.f;
			const bool bQuiet = Sw.RingScale < 1.f && Far01 < 0.999f;
			const float A = static_cast<float>((FrameF - static_cast<double>(B.Frame)) / (BurstFrames * FMath::Lerp(FMath::Clamp(Sw.RingLife, 0.1f, 1.f), 1.f, Far01)));
			if (A < 0.f || A >= 1.f) continue;
			const FLinearColor Col = Sw.bRingTeam ? FMath::Lerp(FLinearColor::White, B.Colour, FMath::Min(1.f, A * 2.f)) : FLinearColor::White;
			const double RingM = Sw.RingScale < 1.f ? FMath::Lerp(FMath::Max(RingMinM, B.RadiusM * Sw.RingScale), FMath::Max(RingMinM, B.RadiusM), static_cast<double>(Far01)) : B.RadiusM;
			const float WideK = Sw.RingScale < 1.f ? FMath::Lerp(0.45f, 1.f, Far01) : 1.f;
			// Evolution 10: width, brightness and count by the hit's weight, all falling from the first frame; nothing within BodyClearM of either body.
			const float Em = BurstEmissive(B.Weight, A);
			const double R = RingM * 100.0 * Sc * (bQuiet ? FMath::Lerp(0.8 + 0.2 * EaseOut(A), 0.45 + 0.55 * EaseOut(A), static_cast<double>(Far01)) : 0.45 + 0.55 * EaseOut(A));
			const int32 Segs = 10 + FMath::RoundToInt(6.f * B.Weight);
			const int32 First = Out.Num();
			Ring(Out, C, R, Segs, Segs, BurstBarWidthCm(B.Weight, A) * WideK * Sc, 4.0, GroundZ, Col, Em * WideK, EJamHitLayer::Burst, 0.6);
			if (B.bBraced && A > 0.15f && !bQuiet) Ring(Out, C, R * 0.62, 12, 12, BurstBarWidthCm(B.Weight, A) * 0.7f * Sc, 4.0, GroundZ, Col, Em, EJamHitLayer::Burst, 0.6);
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
				const double Dist = RingM * 100.0 * Sc * (bQuiet ? FMath::Lerp(0.9 + 0.5 * EaseOut(A), 0.25 + 0.6 * EaseOut(A), static_cast<double>(Far01)) : 0.25 + 0.6 * EaseOut(A));
				FJamHitBar S;
				// A victim who stays up: shards that rise (45 to 80 cm); one who goes down: ice spray that stays on the ice (nothing over the lying figure).
				const bool bLow = !B.FallM.IsNearlyZero();
				S.CentreCm = C + FVector(D.X, D.Y, 0.0) * Dist + FVector(0, 0, bLow ? GroundZ + 2.0 : 45.0 + 35.0 * A);
				S.YawDeg = static_cast<float>(FMath::RadiansToDegrees(Ang));
				S.PitchDeg = bLow ? 0.f : 25.f;
				S.SizeCm = FVector((60.0 * (1.0 - A) + 15.0) * Sc * (bQuiet ? FMath::Lerp(0.6, 1.0, static_cast<double>(Far01)) : 1.0), (bLow ? 14.0 : 10.0) * WideK, bLow ? 4.0 : 10.0 * WideK);
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

// Lane A210: the coil and the drive as one number, -1 (sunk: the coil) .. +1 (sprung: the drive), Since frames after a press with Windup frames of
// wind-up; End = the frame it ended (the contact, the miss) or would at the latest; bLanded: the contact came (the drive whatever the frame: lane A209's
// rule lets a body meet inside the wind-up). Sinks over the wind-up (eased, fully sunk three frames before its end), springs from -1 to +1 over the two
// frames up to the window (sprung on the frame it opens), holds to End, settles over PopSettleFrames.
float FJamHitReadModel::CheckPop(float Since, int32 Windup, float End, bool bLanded, float WhiffDip)
{
	if (Since < 0.f) return 0.f;
	const float W = static_cast<float>(FMath::Max(Windup, 1));
	const float Launch = FMath::Max(W - 2.f, 0.f);
	const auto Shape = [Launch](float T)
	{
		if (T < Launch) { const float K = FMath::Clamp(T / FMath::Max(Launch - 1.f, 1.f), 0.f, 1.f); return -(1.f - (1.f - K) * (1.f - K)); }
		return FMath::Clamp(-1.f + (T - Launch) * 1.f, -1.f, 1.f);
	};
	if (Since <= End) return Shape(Since);
	const float At = bLanded ? 1.f : Shape(End);
	if (!bLanded && WhiffDip > 0.f)
	{
		// A miss: from where the lunge ended down into the dip, then up to his own size.
		const float U = Since - End;
		if (U <= WhiffDipFrames) return FMath::Lerp(At, -WhiffDip, U / WhiffDipFrames);
		return -WhiffDip * (1.f - FMath::Clamp((U - WhiffDipFrames) / (WhiffFrames - WhiffDipFrames), 0.f, 1.f));
	}
	return At * (1.f - FMath::Clamp((Since - End) / PopSettleFrames, 0.f, 1.f));
}

float FJamHitReadModel::PopFor(int32 I, double FrameF, float WhiffDip) const
{
	if (Down.IsValidIndex(I) && Down[I]) return 0.f;
	for (int32 k = Presses.Num() - 1; k >= 0; --k)
	{
		const FPress& P = Presses[k];
		if (P.Skater != I) continue;
		if (P.EndFrame >= 0 && P.Result == EJamHitResult::None) return 0.f;
		const bool bLanded = P.EndFrame >= 0 && (P.Result == EJamHitResult::Down || P.Result == EJamHitResult::Stumble || P.Result == EJamHitResult::Bump);
		// A miss holds the drive four frames into the window at most (the lunge on air), a cross-check one frame past its shove.
		const float Latest = static_cast<float>(FMath::Max(P.Windup, 1) + (P.bCross ? 1 : FMath::Min(P.Window, 4)));
		const float End = P.EndFrame >= 0 ? FMath::Min(static_cast<float>(P.EndFrame - P.PressFrame), bLanded ? 1e6f : Latest) : Latest;
		return CheckPop(static_cast<float>(FrameF - static_cast<double>(P.PressFrame)), P.Windup, End, bLanded, WhiffDip) * (P.bCross ? 0.7f : 1.f);
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
	Slot->RetimeEarly(Sim, Skaters);
	Slot->StepBraceSet(Sim);
	Slot->LogStick(Sim, Skaters);
	Slot->Capture(Sim);
}

// Lane A210: a body check that lands RetimeLeadFrames or more before its wind-up ends finds device B's take still in its coil (the take puts contact on
// its frame 10 and was started at 10 - wind-up). The checker's figure is told PlayCheckWindup(1) by name on that frame: the take jumps to the frame
// before its contact pose, the hit-stop holds it there, the follow-through plays on. Only on figures told SetWindupTakes (TakesOn). Presentation only.
int32 AJamHitRead::RetimeFor(int32 PressToContact, int32 Windup)
{
	return PressToContact >= 0 && Windup > 0 && Windup - PressToContact >= RetimeLeadFrames ? 1 : -1;
}

// The re-time, two ways. Mode 1 (default): A's own: device B's figure does nothing with PlayCheckWindup on a running take (strips of 22:17), so the
// figure's clock is run forward instead: for ONE sim frame the figure actor's time dilation is the frames its take is short of its contact pose
// (wind-up - press to contact), then it goes back to the hit-stop's hold (or to 1): the take stands on its contact pose from the first held frame.
// Mode 2: the call PlayCheckWindup(1) by name (for the day B's figure honours it). Presentation only: the sim never sees the figure's clock.
static void JamHitSetFigureDilation(AJamSkater* Sk, float Dilation)
{
	if (!Sk) return;
	Sk->CustomTimeDilation = Dilation;
	if (AActor* Fig = Cast<AActor>(Sk->GetVisualObject())) Fig->CustomTimeDilation = Dilation;
}

void AJamHitRead::RetimeEarly(const UJamSim* Sim, const TArray<TObjectPtr<AJamSkater>>& Skaters)
{
	static const FName NameWindup(TEXT("PlayCheckWindup"));
	// Last frame's fast-forwards end here: back to the hold's stand-still while the hit-stop runs, else to the figure's own clock.
	FastFwdRenderLeft = 0;
	for (AJamSkater* Sk : Skaters)
	{
		if (!Sk || !FastFwd.IsValidIndex(Sk->GetSimIndex()) || !FastFwd[Sk->GetSimIndex()]) continue;
		FastFwd[Sk->GetSimIndex()] = false;
		JamHitSetFigureDilation(Sk, Sk->IsHitStopped() ? 0.0001f : 1.f);
	}
	const int32 Mode = CVarJamHitRetime.GetValueOnGameThread();
	const auto Send = [&](int32 Who, int32 ShortFrames, const TCHAR* Why, int32 Windup, int32 PressToContact)
	{
		for (AJamSkater* Sk : Skaters)
		{
			if (!Sk || Sk->GetSimIndex() != Who) continue;
			const TCHAR* How = TEXT("none");
			if (Mode >= 2) { if (AJamSkater::CallFigure(Sk->GetVisualObject(), NameWindup, { 1.0 })) How = TEXT("PlayCheckWindup(1)"); }
			else
			{
				if (FastFwd.Num() <= Who) FastFwd.SetNumZeroed(Who + 1);
				FastFwd[Who] = true;
				FastFwdRenderLeft = FastFwdRenderMax;
				JamHitSetFigureDilation(Sk, static_cast<float>(FMath::Clamp(ShortFrames, 1, 10)));
				How = TEXT("fast_forward");
			}
			UE_LOG(LogJamHit, Log, TEXT("JAMHIT retime frame=%lld checker=%d why=%s windup=%d press_to_contact=%d short_frames=%d how=%s (the take to its contact pose)"),
				Sim->GetFrame(), Who, Why, Windup, PressToContact, ShortFrames, How);
			return;
		}
	};
	if (const int32 TestAt = CVarJamHitRetimeTest.GetValueOnGameThread(); TestAt > 0)
	{
		for (const FJamHitReadModel::FPress& P : Model.GetPresses())
			if (P.bHuman && !P.bCross && Sim->GetFrame() - P.PressFrame == TestAt && P.Windup > TestAt) Send(P.Skater, P.Windup - TestAt, TEXT("test"), P.Windup, TestAt);
	}
	// Wave 4: a miss is told to the figure by name: a body check device B's PlayCheckWhiff() (H56.5), else PlayWhiff(0); a cross-check PlayWhiff(1)
	// (B is asked in handoff 0037 whether a cross-check's whiff has its own call). A figure without the call gets nothing.
	// Lane A224 (jam.vis.WhiffTake 1): PlayWhiff(Kind) first (device B's H59.5: 0 body check, 1 cross-check), PlayCheckWhiff only for a figure without it;
	// no stick-to-front over B's take; the pawn keeps A's other calls off the take while it runs (NoteWhiffTake). A cross-check whose live window ended
	// because he braces (or still holds the button) is not a miss yet: the hold goes on, and its whiff is the release on nobody (AJamSkater::ApplySimFrame).
	if (AJamSkater::WhiffTakeOn())
	{
		static const FName NameWhiff(TEXT("PlayWhiff")), NameCheckWhiff(TEXT("PlayCheckWhiff"));
		const TArray<FJamSkaterState>& SkS = Sim->GetSkaters();
		for (const FJamHitReadModel::FHitLog& L : Model.GetFrameLogs())
		{
			if (L.Result != EJamHitResult::Miss) continue;
			const bool bCross = L.Kind == TEXT("cross");
			if (bCross && SkS.IsValidIndex(L.Checker) && (SkS[L.Checker].bCrossBraced || SkS[L.Checker].CrossHoldFrames > 0))
			{
				if (L.bHuman) UE_LOG(LogJamHit, Log, TEXT("JAMHIT whiff frame=%lld checker=%d kind=cross call=none (held: the hold goes on; its whiff is the release on nobody)"), Sim->GetFrame(), L.Checker);
				continue;
			}
			for (AJamSkater* Sk : Skaters) if (Sk && Sk->GetSimIndex() == L.Checker)
			{
				const bool bSent = AJamSkater::CallFigure(Sk->GetVisualObject(), NameWhiff, { bCross ? 1.0 : 0.0 });
				const bool bSentCheck = !bSent && !bCross && AJamSkater::CallFigure(Sk->GetVisualObject(), NameCheckWhiff, {});
				if (bSent || bSentCheck) Sk->NoteWhiffTake(bCross ? 1 : 0);
				if (L.bHuman) UE_LOG(LogJamHit, Log, TEXT("JAMHIT whiff frame=%lld checker=%d kind=%s call=%s take=1"), Sim->GetFrame(), L.Checker, *L.Kind, bSent ? (bCross ? TEXT("PlayWhiff(1)") : TEXT("PlayWhiff(0)")) : bSentCheck ? TEXT("PlayCheckWhiff") : TEXT("none"));
			}
		}
	}
	else if (CVarJamHitWhiff.GetValueOnGameThread() > 0.f)
	{
		static const FName NameWhiff(TEXT("PlayWhiff")), NameCheckWhiff(TEXT("PlayCheckWhiff"));
		for (const FJamHitReadModel::FHitLog& L : Model.GetFrameLogs())
		{
			if (L.Result != EJamHitResult::Miss) continue;
			for (AJamSkater* Sk : Skaters) if (Sk && Sk->GetSimIndex() == L.Checker)
			{
				const bool bCross = L.Kind == TEXT("cross");
				const bool bSentCheck = !bCross && AJamSkater::CallFigure(Sk->GetVisualObject(), NameCheckWhiff, {});
				const bool bSent = bSentCheck || AJamSkater::CallFigure(Sk->GetVisualObject(), NameWhiff, { bCross ? 1.0 : 0.0 });
				if (L.Kind == TEXT("cross")) Sk->SetAimRecover(WhiffAimFrames);   // the stick back to the front while the take runs out
				if (L.bHuman) UE_LOG(LogJamHit, Log, TEXT("JAMHIT whiff frame=%lld checker=%d kind=%s call=%s"), Sim->GetFrame(), L.Checker, *L.Kind, bSentCheck ? TEXT("PlayCheckWhiff") : bSent ? TEXT("PlayWhiff") : TEXT("none"));
			}
		}
	}
	if (Mode == 0) return;
	for (const FJamHitReadModel::FHitLog& L : Model.GetFrameLogs())
	{
		if (L.Kind != TEXT("check") || !(L.Result == EJamHitResult::Bump || L.Result == EJamHitResult::Stumble || L.Result == EJamHitResult::Down)) continue;
		if (RetimeFor(L.PressToContact, L.Windup) < 0 || !TakesOn.IsValidIndex(L.Checker) || !TakesOn[L.Checker]) continue;
		Send(L.Checker, L.Windup - L.PressToContact, TEXT("contact"), L.Windup, L.PressToContact);
	}
}

// Lane A210: how far each figure is into its braced set (0..1), 6 sim frames in and out (the length of device B's CrossCheck_Hold_In / _Out).
void AJamHitRead::StepBraceSet(const UJamSim* Sim)
{
	const TArray<FJamSkaterState>& S = Sim->GetSkaters();
	if (BraceSetK.Num() != S.Num()) BraceSetK.Init(0.f, S.Num());
	for (int32 i = 0; i < S.Num(); ++i) BraceSetK[i] = FMath::Clamp(BraceSetK[i] + (S[i].bCrossBraced && !S[i].IsDown() ? 1.f : -1.f) / BraceSetFrames, 0.f, 1.f);
}

// Lane A210 (with jam.hit.Capture on): where device B's take holds the human's stick, every 10th sim frame of a hold and on the frames 2, 6 and 10 of a
// press: the stick_socket bone, the two hands and the head in the figure's own frame (x ahead, y to his right, z up, cm) and the shaft's direction.
// For the FOR B list: numbers instead of adjectives. Nothing reads it back.
void AJamHitRead::LogStick(const UJamSim* Sim, const TArray<TObjectPtr<AJamSkater>>& Skaters)
{
	if (CVarJamHitCapture.GetValueOnGameThread() <= 0) return;
	const int32 H = Sim->GetHumanIndex();
	if (!Sim->GetSkaters().IsValidIndex(H)) return;
	const FJamSkaterState& S = Sim->GetSkaters()[H];
	const int64 F = Sim->GetFrame();
	if (S.bCheckThisFrame || S.bLiftThisFrame) StickPressFrame = F;
	if (S.bCheckThisFrame || S.bLiftThisFrame) StickPressWindup = FMath::Max(S.WindupFrames, 0);
	const int64 Since = StickPressFrame >= 0 ? F - StickPressFrame : -1;
	// The human's step on each frame of his wind-up and two after (lane A209's pull toward the man rides on it): along and across his facing, in cm.
	if (Since >= 0 && Since <= StickPressWindup + 2)
	{
		const FVector2D D = (S.Pos - S.PrevPos) * 100.0, Fw(FMath::Cos(S.Yaw), FMath::Sin(S.Yaw));
		UE_LOG(LogJamHit, Log, TEXT("JAMHIT step frame=%lld since_press=%lld windup=%d along_cm=%.1f across_cm=%.1f speed_mps=%.2f yaw_step_deg=%.1f"), F, Since, StickPressWindup,
			FVector2D::DotProduct(D, Fw), FVector2D::DotProduct(D, FVector2D(-Fw.Y, Fw.X)), S.Vel.Size(), FMath::RadiansToDegrees(FMath::UnwindRadians(S.Yaw - S.PrevYaw)));
	}
	const bool bPressFrame = Since == 2 || Since == 6 || Since == 8 || Since == 10 || Since == 14;
	if (!(S.bCrossBraced && F % 10 == 0) && !bPressFrame) return;
	for (AJamSkater* Sk : Skaters)
	{
		if (!Sk || Sk->GetSimIndex() != H) continue;
		const AActor* Fig = Cast<AActor>(Sk->GetVisualObject());
		const USkeletalMeshComponent* Mesh = Fig ? Fig->FindComponentByClass<USkeletalMeshComponent>() : nullptr;
		if (!Mesh) return;
		const FTransform Inv = Sk->GetActorTransform().Inverse();
		const auto Local = [&](const TCHAR* Bone) { return Inv.TransformPosition(Mesh->GetSocketLocation(FName(Bone))); };
		const FVector St = Local(TEXT("stick_socket")), Hr = Local(TEXT("hand_r")), Hl = Local(TEXT("hand_l")), Hd = Local(TEXT("head"));
		const FVector Shaft = Inv.TransformVectorNoScale(Mesh->GetSocketTransform(FName(TEXT("stick_socket"))).GetUnitAxis(EAxis::Z));
		UE_LOG(LogJamHit, Log, TEXT("JAMHIT stick frame=%lld braced=%d since_press=%lld stick=(%.0f, %.0f, %.0f) hand_r=(%.0f, %.0f, %.0f) hand_l=(%.0f, %.0f, %.0f) head=(%.0f, %.0f, %.0f) shaft=(%.2f, %.2f, %.2f) (figure frame: x ahead, y right, z up, cm)"),
			F, S.bCrossBraced ? 1 : 0, Since, St.X, St.Y, St.Z, Hr.X, Hr.Y, Hr.Z, Hl.X, Hl.Y, Hl.Z, Hd.X, Hd.Y, Hd.Z, Shaft.X, Shaft.Y, Shaft.Z);
		return;
	}
}

// Lane A210 (jam.hit.CaptureBots 2): the rink point P (metres) is inside the picture, Margin (a share of the size) in from every edge.
static bool JamHitOnScreen(const UWorld* World, const FVector2D& PM, float Margin)
{
	const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	int32 W = 0, H = 0;
	FVector2D Px;
	if (!PC) return false;
	PC->GetViewportSize(W, H);
	if (W <= 0 || H <= 0 || !PC->ProjectWorldLocationToScreen(FVector(PM.X * 100.0, PM.Y * 100.0, 90.0), Px)) return false;
	return Px.X > W * Margin && Px.X < W * (1.f - Margin) && Px.Y > H * Margin && Px.Y < H * (1.f - Margin);
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
		else if (S.bBoardHitThisFrame && CVarJamHitCaptureBoards.GetValueOnGameThread() != 0) { Kind = TEXT("board"); Until = F + 24; }   // lane A210: his own board hits
		else if (S.bLiftThisFrame) { Kind = TEXT("cross"); Until = F + FMath::Max(S.WindupFrames, 0) + 30; }
		else if (S.bCrossBraced && !bWasBraced) { Kind = TEXT("brace"); Until = F + 240; }
		else for (const FJamHitReadModel::FHitLog& L : Model.GetFrameLogs()) if (L.Victim == H && (L.Result == EJamHitResult::Down || L.Result == EJamHitResult::Stumble)) { Kind = TEXT("taken"); Until = F + 40; }
		// Lane A210 (jam.hit.Capture N with jam.hit.CaptureBots 1): a bot's press within CaptureBotsM of the human is a strip too (botcheck / botcross):
		// the bots' takes at their own wind-ups (8 / 12 frames) from the game camera.
		if (Kind.IsEmpty() && CVarJamHitCaptureBots.GetValueOnGameThread() != 0)
		{
			for (const FJamSkaterState& B : Sim->GetSkaters())
			{
				if (B.bHuman || !(B.bCheckThisFrame || B.bLiftThisFrame) || FVector2D::Distance(B.Pos, S.Pos) > CVarJamHitCaptureBotsM.GetValueOnGameThread() || FVector2D::Distance(B.Pos, S.Pos) < CVarJamHitCaptureBotsMinM.GetValueOnGameThread()) continue;
				if (CVarJamHitCaptureBots.GetValueOnGameThread() >= 2 && !JamHitOnScreen(GetWorld(), B.Pos, 0.2f)) continue;
				Kind = B.bCheckThisFrame ? TEXT("botcheck") : TEXT("botcross");
				Until = F + FMath::Max(B.WindupFrames, 0) + (B.bCheckThisFrame ? 6 : 0) + 30;
				UE_LOG(LogJamHit, Log, TEXT("JAMHIT capture bot kind=%s frame=%lld skater=%d windup=%d dist_m=%.1f"), *Kind, F, B.Index, B.WindupFrames, FVector2D::Distance(B.Pos, S.Pos));
				break;
			}
		}
		// Lane A210: a hit between two bots that put a man down or made him stumble, inside the same distance band from the human: a strip from the
		// contact on (bothit): what a knockdown at 20 and 35 m looks like from the game camera (the wind-up is not to be seen at that size anyway).
		if (Kind.IsEmpty() && CVarJamHitCaptureBots.GetValueOnGameThread() != 0)
		{
			for (const FJamHitReadModel::FHitLog& L : Model.GetFrameLogs())
			{
				if (L.Checker == H || L.Victim == H || !Sim->GetSkaters().IsValidIndex(L.Victim) || !(L.Result == EJamHitResult::Down || L.Result == EJamHitResult::Stumble)) continue;
				const double D = FVector2D::Distance(Sim->GetSkaters()[L.Victim].Pos, S.Pos);
				if (D > CVarJamHitCaptureBotsM.GetValueOnGameThread() || D < CVarJamHitCaptureBotsMinM.GetValueOnGameThread()) continue;
				if (CVarJamHitCaptureBots.GetValueOnGameThread() >= 2 && !JamHitOnScreen(GetWorld(), Sim->GetSkaters()[L.Victim].Pos, 0.2f)) continue;
				Kind = TEXT("bothit"); Until = F + 45;
				UE_LOG(LogJamHit, Log, TEXT("JAMHIT capture bot kind=bothit frame=%lld checker=%d victim=%d result=%s closing=%.1f dist_m=%.1f"), F, L.Checker, L.Victim, HitResultName(L.Result), L.ClosingMps, D);
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

// Lane A219: the switches the game draws with (the cvars; the camera's place for the far ring is added in Render). The tests read the same function.
FJamHitReadModel::FDrawSwitches AJamHitRead::GameSwitches()
{
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
	return Sw;
}

bool AJamHitRead::AnyBarLayer(const FJamHitReadModel::FDrawSwitches& Sw)
{
	// The Tell layer holds the chevron, the streak, the chest bar and the arc: with bTell off none of the four is reached, whatever their own switches say.
	return Sw.bTell || Sw.bBurst || Sw.bDown || Sw.bFlash;
}

void AJamHitRead::Render(const UJamSim* Sim, const TArray<TObjectPtr<AJamSkater>>& Skaters, const FVector& PuckAtCm, double FrameF)
{
	if (!Sim) return;
	// Lane A210: a fast-forward ends at the next sim step (RetimeEarly). Should no step come (the sim stood still), it ends here after FastFwdRenderMax
	// drawn frames, so no figure is left running fast.
	if (FastFwdRenderLeft > 0 && --FastFwdRenderLeft == 0)
	{
		for (AJamSkater* Sk : Skaters)
		{
			if (!Sk || !FastFwd.IsValidIndex(Sk->GetSimIndex()) || !FastFwd[Sk->GetSimIndex()]) continue;
			FastFwd[Sk->GetSimIndex()] = false;
			JamHitSetFigureDilation(Sk, Sk->IsHitStopped() ? 0.0001f : 1.f);
		}
	}
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
	FJamHitReadModel::FDrawSwitches Sw = GameSwitches();
	if (Sw.bThinTell)
	{
		const APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
		if (PC && PC->PlayerCameraManager && CVarJamHitRingFar.GetValueOnGameThread() != 0) { Sw.bRingByDistance = true; Sw.EyeAtCm = PC->PlayerCameraManager->GetCameraLocation(); }
	}
	// Lane A219: with no layer switched on (the default) the model is not asked and no bar mesh is ever made (ApplyBars grows its pool only for a bar).
	TArray<FJamHitBar> Bars;
	if (AnyBarLayer(Sw)) Model.Draw(FrameF, At, Yaw, PuckAtCm, Sw, Bars);
	ApplyBars(Bars);
	// Lane A210: the drawn figure's own size: the braced set (jam.hit.BraceSet / jam.hit.BraceLean) and the hit's coil and drive (jam.hit.Pop). All 0:
	// nothing is touched. The scale is set on every frame a figure is off its own size and put back to 1 once; through a hit-stop it keeps the contact
	// frame's; the pitch rides on the transform ApplyRender set this frame.
	{
		const float SetAmt = FMath::Clamp(CVarJamHitBraceSet.GetValueOnGameThread(), 0.f, 2.f), SetLean = FMath::Clamp(CVarJamHitBraceLean.GetValueOnGameThread(), 0.f, 30.f);
		const float PopAmt = FMath::Clamp(CVarJamHitPop.GetValueOnGameThread(), 0.f, 2.f);
		if (BraceScaled.Num() != N) { BraceScaled.Init(false, N); PopHeld.Init(0.f, N); }
		for (AJamSkater* Sk : Skaters)
		{
			if (!Sk || !BraceSetK.IsValidIndex(Sk->GetSimIndex())) continue;
			const int32 I = Sk->GetSimIndex();
			const float K = SetAmt > 0.f ? FMath::InterpEaseInOut(0.f, 1.f, BraceSetK[I], 2.f) : 0.f;
			if (!Sk->IsHitStopped()) PopHeld[I] = PopAmt > 0.f ? Model.PopFor(I, FrameF, AJamSkater::WhiffTakeOn() ? 0.f : FMath::Clamp(CVarJamHitWhiff.GetValueOnGameThread(), 0.f, 1.5f)) * PopAmt : 0.f;
			const FVector Scale = BraceSetScale(K, SetAmt) * PopScale(PopHeld[I]);
			if (!Scale.Equals(FVector::OneVector, 1e-4)) { Sk->SetActorScale3D(Scale); BraceScaled[I] = true; }
			else if (BraceScaled[I]) { Sk->SetActorScale3D(FVector::OneVector); BraceScaled[I] = false; }
			// Through a hit-stop too: ApplyRender sets the held transform afresh each frame, and a braced figure that stood upright for the hold's frames
			// would pop at the very moment its hit lands.
			if (K > 0.f && SetLean > 0.f) Sk->AddActorLocalRotation(FRotator(-SetLean * K, 0.0, 0.0));
		}
	}
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

static TAutoConsoleVariable<int32> CVarJamHudEdgeArrowOffPanel(TEXT("jam.hud.EdgeArrowOffPanel"), 1,
	TEXT("Lane A227: 1 (default) a man-down edge arrow stands over the controls panel, never on it (as the puck arrow and the net chevrons); 0 = before."), ECVF_Default);

// Lane A235 (the walk of H45, W15): the man-down arrow stood in the picture beside the man just sent flying (Sniper f849) or alone on the ice (Power Play f800):
// lane A227's lift over the controls panel moves an arrow at the left edge up into the frame, and a man who has just gone off its edge is still on the camera's
// way. A mark at a hit (R-0069, R-0070), and the camera shows the man anyway: 0 (default) the arrow is not drawn (the radar's down marks stay with jam.hit.OffScreen);
// 1 = before (drawn as jam.hit.OffScreen says).
static TAutoConsoleVariable<int32> CVarJamHitEdgeArrow(TEXT("jam.hit.EdgeArrow"), 0,
	TEXT("Lane A235: 0 (default) no man-down arrow at the screen's edge (UJamHitEdgeWidget); 1 = before (drawn when jam.hit.OffScreen is 1). The radar's down marks follow jam.hit.OffScreen alone."), ECVF_Default);

bool UJamHitEdgeWidget::EdgeArrowOn() { return CVarJamHitEdgeArrow.GetValueOnGameThread() != 0; }

void UJamHitEdgeWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	Arrows.Reset();
	AJamHitRead* H = Source.Get();
	APlayerController* PC = GetOwningPlayer();
	if (!H || !PC || !AJamHitRead::OffScreenOn() || !EdgeArrowOn() || !PC->PlayerCameraManager) return;
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
		// Lane A227 (lane A226's F7): the arrow stands over the controls panel, never on it (the clamp of the puck arrow and the chevrons).
		if (CVarJamHudEdgeArrowOffPanel.GetValueOnGameThread() != 0) A.PosPx = UJamHudWidget::KeepOffPanel(A.PosPx, static_cast<float>(Size.Y), UJamHudWidget::ControlsPanelTopNow(), 40.f, FJamUiStyle::EdgePx);
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
	// Lane A219 (R-0069 point 3): the same frame as the GAME draws it: the cvars' switches, and the model asked only when a layer is on (Render's path).
	void DrawGame(const FJamHitReadModel& M, double FrameF, const TArray<FJamSkaterState>& S, TArray<FJamHitBar>& Out)
	{
		TArray<FVector> At; TArray<float> Yaw;
		for (const FJamSkaterState& X : S) { At.Add(FVector(X.Pos.X * 100.0, X.Pos.Y * 100.0, 0.0)); Yaw.Add(static_cast<float>(FMath::RadiansToDegrees(X.Yaw))); }
		Out.Reset();
		const FJamHitReadModel::FDrawSwitches Sw = AJamHitRead::GameSwitches();
		if (AJamHitRead::AnyBarLayer(Sw)) M.Draw(FrameF, At, Yaw, FVector(500, 0, 0), Sw, Out);
	}
	// Package H39's switches (the quiet marks of lane A210: every layer on, no streak, no chest bar, thin and white).
	FJamHitReadModel::FDrawSwitches H39()
	{
		FJamHitReadModel::FDrawSwitches Sw;
		Sw.bStreak = false; Sw.bBraceBar = false; Sw.bThinTell = true; Sw.bRingTeam = false; Sw.RingScale = 0.45f; Sw.RingLife = 8.f / FJamHitReadModel::BurstFrames;
		return Sw;
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
	TestEqual(TEXT("the layer on: the wind-up shows the chevron (2 bars)"), Count(Bars, EJamHitLayer::Tell), 2);
	DrawGame(M, 10.0, S, Bars);
	TestEqual(TEXT("the game (R-0069): nothing is drawn through a check's wind-up"), Bars.Num(), 0);
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
	TestEqual(TEXT("the layer on: the chevron fades out after the miss"), Count(Bars, EJamHitLayer::Tell), 2);
	bool bGrey = true;
	for (const FJamHitBar& B : Bars) if (B.Layer == static_cast<uint8>(EJamHitLayer::Tell)) bGrey &= B.Emissive < 3.f;   // the live chevron burns at 8.4 through the window
	TestTrue(TEXT("the layer on: the missed chevron is dim"), bGrey);
	DrawGame(M, 27.0, S, Bars);
	TestEqual(TEXT("the game (R-0069): no grey mark after a miss"), Bars.Num(), 0);
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
	TestEqual(TEXT("the layer on: the stripped puck flashes"), Count(Bars, EJamHitLayer::PuckFlash), 10);
	DrawGame(X, 52.0, T, Bars);
	TestEqual(TEXT("the game (R-0069): no ring of bars round a stripped puck"), Bars.Num(), 0);
	return true;
}

// Lane A219 (R-0069 point 3: "Crosscheck still displays blue line/rectangle in front of player. This should player never see"). The game's own switches
// (the cvars' defaults, through AJamHitRead::GameSwitches as Render reads them) draw NO bar at any frame of: a body check that misses, a cross-check
// that strips the puck and meets nobody, a hold of 60 frames at speed (the arc's case, with and without a tap before it), a braced hit that floors the
// man and his 90 frames down; thrown by the human and thrown by a bot. The same frames drawn with package H39's switches show every one of the layers:
// the film's frames are what this test walks, so a layer switched back on by default fails here by name.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHitNoBars, "Jam.Hit.NoBars", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHitNoBars::RunTest(const FString&)
{
	using namespace JamHitTest;
	TestEqual(TEXT("ships: no tell (chevron, streak, chest bar, arc)"), CVarJamHitTell.GetValueOnGameThread(), 0);
	TestEqual(TEXT("ships: no arc of dashes ahead of a held cross-check"), CVarJamHitBraceArc.GetValueOnGameThread(), 0);
	TestEqual(TEXT("ships: no bar across the stick"), CVarJamHitStreak.GetValueOnGameThread(), 0);
	TestEqual(TEXT("ships: no bar across the chest"), CVarJamHitBraceBar.GetValueOnGameThread(), 0);
	TestEqual(TEXT("ships: no burst of rectangles at contact"), CVarJamHitBurst.GetValueOnGameThread(), 0);
	TestEqual(TEXT("ships: no countdown of rectangles round a downed man"), CVarJamHitDown.GetValueOnGameThread(), 0);
	TestEqual(TEXT("ships: no ring of rectangles round a stripped puck"), CVarJamHitPuckFlash.GetValueOnGameThread(), 0);
	TestFalse(TEXT("the game's switches: no layer of bars is on"), AJamHitRead::AnyBarLayer(AJamHitRead::GameSwitches()));
	TestTrue(TEXT("package H39's switches: layers of bars are on"), AJamHitRead::AnyBarLayer(H39()));
	// A switch of the Tell layer left on alone reaches nothing while the layer is off (the streak, the chest bar and the arc live inside it).
	{
		FJamHitReadModel::FDrawSwitches Only;
		Only.bTell = false; Only.bBurst = false; Only.bDown = false; Only.bFlash = false; Only.bStreak = true; Only.bBraceBar = true; Only.bBraceArc = true;
		TestFalse(TEXT("streak, chest bar and arc switched on with the Tell layer off: still no layer"), AJamHitRead::AnyBarLayer(Only));
		FJamHitReadModel M;
		TArray<FJamSkaterState> S = Two();
		TArray<FJamSimEvent> None;
		S[0].bLiftThisFrame = true; S[0].WindupFrames = 8;
		M.Step(100, S, None, 8.0, Cols);
		ClearFrame(S);
		S[0].bCrossBraced = true; S[0].Vel = FVector2D(7, 0);
		M.Step(160, S, None, 8.0, Cols);
		TArray<FVector> At; TArray<float> Yaw;
		for (const FJamSkaterState& X : S) { At.Add(FVector(X.Pos.X * 100.0, X.Pos.Y * 100.0, 0.0)); Yaw.Add(0.f); }
		TArray<FJamHitBar> Bars;
		M.Draw(160.5, At, Yaw, FVector(500, 0, 0), Only, Bars);
		TestEqual(TEXT("and the model itself draws nothing for them"), Bars.Num(), 0);
	}
	const FJamHitReadModel::FDrawSwitches Old = H39();
	for (int32 Who = 0; Who < 2; ++Who)
	{
		const int32 Other = 1 - Who;
		const TCHAR* Name = Who == 0 ? TEXT("the human") : TEXT("a bot");
		FJamHitReadModel M;
		TArray<FJamSkaterState> S = Two();
		TArray<FJamSimEvent> None;
		TArray<FVector> At; TArray<float> Yaw;
		int32 GameBars = 0, Frames = 0;
		int32 OldTell = 0, OldBrace = 0, OldBurst = 0, OldDown = 0, OldFlash = 0, OldOverFigure = 0;
		const auto Look = [&](double FrameF)
		{
			TArray<FJamHitBar> Bars;
			DrawGame(M, FrameF, S, Bars);
			GameBars += Bars.Num(); ++Frames;
			At.Reset(); Yaw.Reset();
			for (const FJamSkaterState& X : S) { At.Add(FVector(X.Pos.X * 100.0, X.Pos.Y * 100.0, 0.0)); Yaw.Add(static_cast<float>(FMath::RadiansToDegrees(X.Yaw))); }
			Bars.Reset();
			M.Draw(FrameF, At, Yaw, FVector(500, 0, 0), Old, Bars);
			OldTell += Count(Bars, EJamHitLayer::Tell); OldBrace += Count(Bars, EJamHitLayer::Brace);
			OldBurst += Count(Bars, EJamHitLayer::Burst) + Count(Bars, EJamHitLayer::Shard); OldDown += Count(Bars, EJamHitLayer::Down); OldFlash += Count(Bars, EJamHitLayer::PuckFlash);
			for (const FJamHitBar& B : Bars) if (B.CentreCm.Z > 50.0) ++OldOverFigure;
		};
		const auto StepLook = [&](int64 F, const TArray<FJamSimEvent>& Ev) { M.Step(F, S, Ev, 8.0, Cols); Look(static_cast<double>(F)); Look(F + 0.5); };
		// 1. A body check into open ice: 8 frames of wind-up, a window of 12, the miss, its fade.
		S[Other].Pos = S[Who].Pos + FVector2D(9, 0);
		S[Who].bCheckThisFrame = true; S[Who].WindupFrames = 8; S[Who].CheckWindupLeft = 8;
		StepLook(100, None);
		for (int64 F = 101; F <= 140; ++F)
		{
			ClearFrame(S);
			const int32 Wu = FMath::Max(0, 108 - static_cast<int32>(F));
			S[Who].CheckWindupLeft = Wu; S[Who].CheckWindow = Wu > 0 ? 0 : FMath::Max(0, 120 - static_cast<int32>(F));
			if (F >= 120) S[Who].ChecksMissed = 1;
			StepLook(F, None);
		}
		// 2. A cross-check tapped with the man in reach: the stick takes the puck, the shove meets nobody.
		S[Other].Pos = S[Who].Pos + FVector2D(1.6, 0);
		ClearFrame(S);
		S[Who].bLiftThisFrame = true; S[Who].WindupFrames = 8;
		{
			TArray<FJamSimEvent> Lift;
			FJamSimEvent E; E.Name = JamEvents::Lift; E.Who = Who; E.Strength = 1.f; Lift.Add(E);
			StepLook(200, Lift);
		}
		for (int64 F = 201; F <= 230; ++F) { ClearFrame(S); StepLook(F, None); }
		// 3. The hold: braced by the sim 13 frames after a tap, held for 60 frames at 7 m/s (the arc's frames: from 40 after the tap).
		ClearFrame(S);
		S[Who].bLiftThisFrame = true; S[Who].WindupFrames = 8;
		StepLook(300, None);
		for (int64 F = 301; F <= 372; ++F)
		{
			ClearFrame(S);
			S[Who].bCrossBraced = F >= 313; S[Who].Vel = FVector2D(7, 0);
			StepLook(F, None);
		}
		// 4. The braced hit: the man goes down for 90 frames.
		ClearFrame(S);
		S[Who].bCrossBraced = false;
		S[Other].Pos = S[Who].Pos + FVector2D(1.2, 0);
		S[Other].ContactBy = Who; S[Other].ContactOutcome = UJamSim::ContactDown; S[Other].bContactCross = true; S[Other].bCrossBraceHit = true;
		S[Other].KnockFrames = 90; S[Other].KnockDir = FVector2D(1, 0);
		{
			TArray<FJamSimEvent> Hit;
			FJamSimEvent E; E.Name = JamEvents::Check; E.Who = Who; E.Where = S[Who].Pos + FVector2D(0.6, 0); E.Strength = 11.f / 8.f; E.Dir = FVector2D(1, 0); Hit.Add(E);
			StepLook(373, Hit);
		}
		for (int64 F = 374; F <= 470; ++F)
		{
			ClearFrame(S);
			S[Other].KnockFrames = FMath::Max(0, 90 - static_cast<int32>(F - 373));
			StepLook(F, None);
		}
		TestEqual(FString::Printf(TEXT("%s: the game draws no bar in any of the %d frames looked at"), Name, Frames), GameBars, 0);
		TestTrue(FString::Printf(TEXT("%s: the frames are the chevron's (package H39 drew %d bars of it)"), Name, OldTell), OldTell > 0);
		TestTrue(FString::Printf(TEXT("%s: the frames are the arc's (package H39 drew %d dashes)"), Name, OldBrace), OldBrace > 0);
		TestTrue(FString::Printf(TEXT("%s: the frames are the burst's (package H39 drew %d rectangles)"), Name, OldBurst), OldBurst > 0);
		TestTrue(FString::Printf(TEXT("%s: the frames are the countdown's (package H39 drew %d rectangles round the downed man)"), Name, OldDown), OldDown > 0);
		TestTrue(FString::Printf(TEXT("%s: the frames are the stripped puck's (package H39 drew %d rectangles)"), Name, OldFlash), OldFlash > 0);
		TestEqual(FString::Printf(TEXT("%s: package H39 already drew nothing over the figure"), Name), OldOverFigure, 0);
	}
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
	TestEqual(TEXT("ships: the quiet marks (for a layer someone switches back on)"), CVarJamHitQuiet.GetValueOnGameThread(), 1);
	// Lane A219 (R-0069 point 3): the layers themselves ship OFF: "Now" below is package H39's picture, kept as the model's test; Jam.Hit.NoBars is the game's.
	TestEqual(TEXT("ships: no tell"), CVarJamHitTell.GetValueOnGameThread(), 0);
	TestEqual(TEXT("ships: no arc ahead of a held cross-check"), CVarJamHitBraceArc.GetValueOnGameThread(), 0);
	TestEqual(TEXT("ships: no burst of bars"), CVarJamHitBurst.GetValueOnGameThread(), 0);
	TestEqual(TEXT("ships: no countdown of bars"), CVarJamHitDown.GetValueOnGameThread(), 0);
	TestEqual(TEXT("ships: device B's takes on every figure, the human's too"), CVarJamHitBTakes.GetValueOnGameThread(), 2);
	// The bridge for the brace: the figure itself sets (nothing drawn on it).
	TestEqual(TEXT("ships: no set on the braced figure (device B's H56.5 hold reads from behind)"), CVarJamHitBraceSet.GetValueOnGameThread(), 0.f, 1e-6f);
	TestEqual(TEXT("ships: no coil of mine on the figure (device B's H56.0 takes read from behind)"), CVarJamHitPop.GetValueOnGameThread(), 0.f, 1e-6f);
	TestTrue(TEXT("not set: the figure's own size"), AJamHitRead::BraceSetScale(0.f, 1.f).Equals(FVector::OneVector, 1e-6));
	TestTrue(TEXT("set: a tenth lower, 6 % wider"), AJamHitRead::BraceSetScale(1.f, 1.f).Equals(FVector(1.06, 1.06, 0.90), 1e-5));
	TestTrue(TEXT("half way in: between the two"), AJamHitRead::BraceSetScale(0.5f, 1.f).Z < 1.f && AJamHitRead::BraceSetScale(0.5f, 1.f).Z > 0.90f);
	TestTrue(TEXT("the switch at 0: untouched"), AJamHitRead::BraceSetScale(1.f, 0.f).Equals(FVector::OneVector, 1e-6));
	// The coil and the drive (jam.hit.Pop): sunk through the wind-up, sprung at the window, settled 8 frames after the end.
	TestEqual(TEXT("the press frame: his own size"), FJamHitReadModel::CheckPop(0.f, 8, 12.f, false), 0.f, 1e-6f);
	TestTrue(TEXT("half way through the wind-up: sinking"), FJamHitReadModel::CheckPop(3.f, 8, 12.f, false) < -0.5f);
	TestEqual(TEXT("three frames before the window: fully sunk"), FJamHitReadModel::CheckPop(5.f, 8, 12.f, false), -1.f, 1e-6f);
	TestEqual(TEXT("a frame before the window: half sprung"), FJamHitReadModel::CheckPop(7.f, 8, 12.f, false), 0.f, 1e-6f);
	TestEqual(TEXT("the frame the window opens: fully sprung"), FJamHitReadModel::CheckPop(8.f, 8, 12.f, false), 1.f, 1e-6f);
	TestEqual(TEXT("settled 8 frames after the end"), FJamHitReadModel::CheckPop(12.f + FJamHitReadModel::PopSettleFrames, 8, 12.f, false), 0.f, 1e-6f);
	// Wave 4 (EYE 7): a miss with the whiff's dip: down to -0.8 four frames after it, his own size 14 frames after it; a landed hit is untouched by it.
	TestEqual(TEXT("ships: the whiff dips"), CVarJamHitWhiff.GetValueOnGameThread(), 0.8f, 1e-6f);
	TestEqual(TEXT("a miss: in the dip 4 frames after"), FJamHitReadModel::CheckPop(12.f + FJamHitReadModel::WhiffDipFrames, 8, 12.f, false, 0.8f), -0.8f, 1e-5f);
	TestTrue(TEXT("a miss: on the way down 2 frames after"), FJamHitReadModel::CheckPop(14.f, 8, 12.f, false, 0.8f) < 0.5f && FJamHitReadModel::CheckPop(14.f, 8, 12.f, false, 0.8f) > -0.8f);
	TestEqual(TEXT("a miss: his own size 14 frames after"), FJamHitReadModel::CheckPop(12.f + FJamHitReadModel::WhiffFrames, 8, 12.f, false, 0.8f), 0.f, 1e-6f);
	TestEqual(TEXT("a landed hit: the dip does not touch it"), FJamHitReadModel::CheckPop(11.f, 8, 9.f, true, 0.8f), FJamHitReadModel::CheckPop(11.f, 8, 9.f, true, 0.f), 1e-6f);
	TestEqual(TEXT("a contact inside the wind-up: the drive from that frame"), FJamHitReadModel::CheckPop(3.5f, 8, 3.f, true), 1.f - 0.5f / FJamHitReadModel::PopSettleFrames, 1e-5f);
	TestTrue(TEXT("sunk: lower and wider"), AJamHitRead::PopScale(-1.f).Z < 1.f && AJamHitRead::PopScale(-1.f).X > 1.f);
	TestTrue(TEXT("sprung: taller and narrower"), AJamHitRead::PopScale(1.f).Z > 1.f && AJamHitRead::PopScale(1.f).X < 1.f);
	TestTrue(TEXT("at rest: his own size"), AJamHitRead::PopScale(0.f).Equals(FVector::OneVector, 1e-6));
	// A contact inside the wind-up: the take is sent to its contact pose; a contact at the wind-up's end or in the window is left alone.
	TestEqual(TEXT("contact on frame 2 of 8: re-timed to one frame before the take's contact"), AJamHitRead::RetimeFor(2, 8), 1);
	TestEqual(TEXT("contact on frame 6 of 8: re-timed"), AJamHitRead::RetimeFor(6, 8), 1);
	TestEqual(TEXT("contact on frame 7 of 8: within a frame, left alone"), AJamHitRead::RetimeFor(7, 8), -1);
	TestEqual(TEXT("contact at the wind-up's end: left alone"), AJamHitRead::RetimeFor(8, 8), -1);
	TestEqual(TEXT("contact in the window: left alone"), AJamHitRead::RetimeFor(13, 8), -1);
	TestEqual(TEXT("no wind-up: nothing to re-time"), AJamHitRead::RetimeFor(0, 0), -1);
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
		TestEqual(TEXT("package H39: no bar across the stick"), Count(Bars, EJamHitLayer::Streak), 0);
		TestEqual(TEXT("package H39: nothing drawn above the ice while he winds up"), Above(Bars, 50.0), 0);
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
		TestEqual(TEXT("package H39: the arc alone"), Count(Bars, EJamHitLayer::Brace), 7);
		// The eye's third list: a tap that ran on into a brace shows no dashes for ArcAfterTapFrames after the press; a hold that began without a tap does.
		{
			FJamHitReadModel T;
			TArray<FJamSkaterState> S2 = Two();
			S2[0].bLiftThisFrame = true; S2[0].WindupFrames = 8;
			T.Step(100, S2, None, 8.0, Cols);
			ClearFrame(S2);
			S2[0].bCrossBraced = true; S2[0].Vel = FVector2D(7, 0);
			T.Step(114, S2, None, 8.0, Cols);
			TArray<FJamHitBar> B2;
			DrawWith(T, 114.5, S2, Now, B2);
			TestEqual(TEXT("14 frames after a tap, braced by the sim: no dashes"), Count(B2, EJamHitLayer::Brace), 0);
			T.Step(100 + FJamHitReadModel::ArcAfterTapFrames, S2, None, 8.0, Cols);
			DrawWith(T, 100.5 + FJamHitReadModel::ArcAfterTapFrames, S2, Now, B2);
			TestEqual(TEXT("still held 40 frames after the tap: the dashes"), Count(B2, EJamHitLayer::Brace), 7);
		}
		TestEqual(TEXT("package H39: nothing over the figure while he holds"), Above(Bars, 50.0), 0);
		for (const FJamHitBar& B : Bars)
		{
			TestTrue(TEXT("package H39: the arc is thin (7 cm)"), B.SizeCm.Y <= 7.0 + 1e-6);
			TestTrue(TEXT("package H39: the arc is the ice's white, not the team's colour"), B.Colour.Equals(FLinearColor::White, 0.001f));
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
		TestEqual(TEXT("package H39: the chevron is still two bars"), Count(Bars, EJamHitLayer::Tell), 2);
		for (const FJamHitBar& B : Bars)
		{
			TestTrue(TEXT("package H39: the chevron is thin (7 cm against 18)"), B.SizeCm.Y <= 7.0 + 1e-6);
			TestTrue(TEXT("package H39: the chevron is white"), B.Colour.Equals(FLinearColor::White, 0.001f));
			TestTrue(TEXT("package H39: the chevron lies on the ice"), B.CentreCm.Z < 20.0);
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
			TestTrue(FString::Printf(TEXT("package H39: the ring keeps to 1.35 m (%.2f m)"), After), After <= 1.35 + 1e-6 && After >= FJamHitReadModel::RingMinM * 0.8 - 1e-6);
			TestTrue(TEXT("package H39: the ring is drawn"), Count(Bars, EJamHitLayer::Burst) > 0 && BarsBefore > 0);
			for (const FJamHitBar& B : Bars)
			{
				if (B.Layer != static_cast<uint8>(EJamHitLayer::Burst) && B.Layer != static_cast<uint8>(EJamHitLayer::Shard)) continue;
				TestTrue(TEXT("package H39: the ring and its spray are white, never the checker's colour"), B.Colour.Equals(FLinearColor::White, 0.001f));
				TestTrue(TEXT("package H39: the ring's bars at most 12 cm wide (28 before)"), B.Layer != static_cast<uint8>(EJamHitLayer::Burst) || B.SizeCm.Y <= 12.0);
				const FVector2D P(B.CentreCm.X / 100.0, B.CentreCm.Y / 100.0);
				TestTrue(TEXT("package H39: still clear of the checker"), FJamHitReadModel::ClearOfBody(P, S[0].Pos, FVector2D::ZeroVector, FJamHitReadModel::BodyClearM - 1e-6));
				TestTrue(TEXT("package H39: still clear of the victim"), FJamHitReadModel::ClearOfBody(P, S[1].Pos, S[1].KnockDir, FJamHitReadModel::BodyClearM - 1e-6));
			}
			DrawWith(M, 108.0 + 8.5, S, Now, Bars);
			TestEqual(TEXT("package H39: the ring is gone 8 frames after the contact"), Count(Bars, EJamHitLayer::Burst) + Count(Bars, EJamHitLayer::Shard), 0);
			DrawWith(M, 108.0 + 8.5, S, H34, Bars);
			TestTrue(TEXT("package H34: it burned on to 14"), Count(Bars, EJamHitLayer::Burst) > 0);
			// The same hit 40 m from the eye: the ring at package H34's size and life again, still white; 5 m from the eye: the small one.
			FJamHitReadModel::FDrawSwitches FarSw = Now; FarSw.bRingByDistance = true; FarSw.EyeAtCm = C + FVector(-4000.0, 0.0, 350.0);
			DrawWith(M, 116.0, S, FarSw, Bars);
			TestTrue(FString::Printf(TEXT("40 m from the eye: the ring reaches past 2 m again (%.2f m)"), Reach(Bars)), Reach(Bars) > 2.0);
			for (const FJamHitBar& B : Bars) if (B.Layer == static_cast<uint8>(EJamHitLayer::Burst)) TestTrue(TEXT("40 m from the eye: still white"), B.Colour.Equals(FLinearColor::White, 0.001f));
			FarSw.EyeAtCm = C + FVector(-500.0, 0.0, 350.0);
			DrawWith(M, 112.0, S, FarSw, Bars);
			TestTrue(FString::Printf(TEXT("5 m from the eye: the small ring (%.2f m)"), Reach(Bars)), Reach(Bars) <= 1.35 + 1e-6);
			FarSw.EyeAtCm = C + FVector(-2000.0, 0.0, 350.0);
			DrawWith(M, 112.0, S, FarSw, Bars);
			TestTrue(FString::Printf(TEXT("20 m from the eye: between the two (%.2f m)"), Reach(Bars)), Reach(Bars) > 1.35 && Reach(Bars) < 2.3);
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
