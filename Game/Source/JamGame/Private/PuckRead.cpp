// Lane A210 (track F3, wave 4): see PuckRead.h.
#include "PuckRead.h"
#include "JamPawns.h"
#include "HAL/IConsoleManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UnrealClient.h"

DEFINE_LOG_CATEGORY_STATIC(LogJamPuckShow, Log, All);

static TAutoConsoleVariable<int32> CVarJamPuckShow(TEXT("jam.puck.Show"), 1,
	TEXT("The carrier the screen shows (lane A210): 1 (default) the figure's has-puck flag and the puck's drawn place both follow the sim's carrier of record after the step, kept over single-frame drop-outs (jam.puck.Keep), with a short blend at a change of hands; 0 = package H34 (the flag from the skater state's bHasPuck, the drawn place from CarrierIndex, no blend). Presentation only."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamPuckKeep(TEXT("jam.puck.Keep"), 6,
	TEXT("With jam.puck.Show 1: the most sim frames the shown carrier is kept while the sim's carrier of record names nobody, the puck is still his last touch, he can act, he did not shoot or pass, and the puck is within 1.35 of his reach. 0 = the sim's carrier exactly. Lane A210."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamPuckCalls(TEXT("jam.puck.Calls"), 1,
	TEXT("Device B's play calls by name (lane A210): 1 (default) PlayReceive, SetPassCharge, PlayCallForPass, SetLookAtPuck(false) on the human, the trail's User.Carried; 0 = none (package H34)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamPuckFollow(TEXT("jam.puck.Follow"), 1,
	TEXT("The carried puck after the frame's animation (lane A210): 1 (default) the puck takes the carrier's puck socket again once the figures' poses of the frame are evaluated (it was placed on last frame's pose: a moving stick left it a frame behind the blade); 0 = placed once, before the animation (package H34; the probe still measures the lag)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamPuckProbe(TEXT("jam.puck.Probe"), 0,
	TEXT("Proof (lane A210): N > 0 logs one JAMPUCK show line every N sim frames: of the sim's carry frames, how many drew the puck within 10 cm of the carrier's puck socket (the human, the bots), and the flagged frames that disagree with the sim's carrier. 0 = off."), ECVF_Default);

static TAutoConsoleVariable<int32> CVarJamPuckCapture(TEXT("jam.puck.Capture"), 0,
	TEXT("Proof (lane A210): N > 0 takes a still every sim frame of the human's play with the puck, N strips a kind: carry (from the frame he becomes the shown carrier, 90 frames), pass (from the first frame of his held pass to 45 after the release), shot (from the first frame of his charge to 40 after the release), receive (a mate's pass seated on him, 10 before is not possible: from that frame, 40). Run with a fixed 60 Hz step and jam.hit.Capture 0 (one still a frame). Stills go to -JamHitShots=<folder>."), ECVF_Default);

bool FJamPuckShow::ShowOn() { return CVarJamPuckShow.GetValueOnGameThread() != 0; }

int32 FJamPuckShow::ShownCarrier(int32 Prev, int32 SimCarrier, int32 LastTouch, bool bPrevCanAct, bool bPrevReleased, double PrevDistM, double PrevReachM, int32 KeepFrames, int32& InOutKept)
{
	if (SimCarrier >= 0) { InOutKept = 0; return SimCarrier; }
	if (Prev >= 0 && KeepFrames > 0 && InOutKept < KeepFrames && LastTouch == Prev && bPrevCanAct && !bPrevReleased && PrevDistM <= PrevReachM * KeepReachK)
	{
		++InOutKept;
		return Prev;
	}
	InOutKept = 0;
	return -1;
}

FVector FJamPuckShow::Drawn(const FVector& TargetCm, const FVector& OffsetAtChangeCm, float SinceChange, int32 BlendFrames)
{
	const float K = BlendFrames <= 0 ? 1.f : FMath::Clamp(SinceChange / static_cast<float>(BlendFrames), 0.f, 1.f);
	const float Left = (1.f - K) * (1.f - K);   // eased out: most of the way in the first frame
	return TargetCm + OffsetAtChangeCm * Left;
}

void FJamPuckShow::Reset()
{
	*this = FJamPuckShow();
}

void FJamPuckShow::StepSim(const UJamSim* Sim, const TArray<TObjectPtr<AJamSkater>>& Skaters, AJamPuck* Puck)
{
	if (!Sim) return;
	const int64 F = Sim->GetFrame();
	if (F <= LastFrame) Reset();   // a restart or a new match: the frame went back
	LastFrame = F;
	const TArray<FJamSkaterState>& Sk = Sim->GetSkaters();
	const FJamPuckState& P = Sim->GetPuck();
	const int32 SimCarrier = Sim->CarrierIndex();
	if (ShowOn())
	{
		bool bCan = false, bReleased = false;
		double Dist = 0.0, Reach = 0.0;
		if (Sk.IsValidIndex(Shown))
		{
			const FJamSkaterState& S = Sk[Shown];
			bCan = S.CanAct();
			bReleased = S.bShotThisFrame || S.bPassThisFrame;
			Dist = FVector2D::Distance(S.Pos, P.Pos);
			Reach = Sim->CarrierReachM(Shown);
		}
		Shown = ShownCarrier(Shown, SimCarrier, P.LastTouch, bCan, bReleased, Dist, Reach, FMath::Max(0, CVarJamPuckKeep.GetValueOnGameThread()), Kept);
		if (Kept > 0) ++KeptN;
		for (AJamSkater* A : Skaters) if (A) A->SetShownCarrier(A->GetSimIndex() == Shown ? 1 : 0);
	}
	else
	{
		Shown = SimCarrier;
		Kept = 0;
		for (AJamSkater* A : Skaters) if (A) A->SetShownCarrier(-1);   // the pawn decides by the state's bHasPuck, as before
	}
	Capture(Sim);
	if (CVarJamPuckCalls.GetValueOnGameThread() == 0) return;
	// Device B's play calls (H55.6), by name: a figure without one gets nothing.
	static const FName NameReceive(TEXT("PlayReceive")), NameCharge(TEXT("SetPassCharge")), NameCall(TEXT("PlayCallForPass")), NameLook(TEXT("SetLookAtPuck"));
	const int32 N = Sk.Num();
	if (PrevReceived.Num() != N) { PrevReceived.Init(0, N); for (int32 i = 0; i < N; ++i) PrevReceived[i] = Sk[i].PassesReceived; ChargeSent.Init(false, N); }
	const int32 Half = FMath::Max(0, Sim->GetConfig().Pass.SaucerHalfFrames);
	const int32 Full = Sim->GetConfig().Pass.PowerFullFrames > 0 ? FMath::Max(Sim->GetConfig().Pass.PowerFullFrames, Half) : FMath::Max(Half, 1);
	const int32 Human = Sim->GetHumanIndex();
	for (AJamSkater* A : Skaters)
	{
		if (!A || !Sk.IsValidIndex(A->GetSimIndex())) continue;
		const int32 I = A->GetSimIndex();
		const FJamSkaterState& S = Sk[I];
		UObject* V = A->GetVisualObject();
		// SetLookAtPuck(false) once on the human's figure (his helmet turning over his shoulder reads as looking away); again when the figure is respawned.
		if (I == Human && V && LookFigure.Get() != V)
		{
			LookFigure = V;
			const bool bSent = AJamSkater::CallFigure(V, NameLook, { 0.0 });
			UE_LOG(LogJamPuckShow, Log, TEXT("JAMPUCK look frame=%lld skater=%d human=1 call=%s"), F, I, bSent ? TEXT("SetLookAtPuck(false)") : TEXT("none"));
		}
		// SetPassCharge: every frame a pass is held with the puck, 0 once when the hold ends (a release, a cancel, a lost puck).
		const float Charge = PassCharge01(S.PassChargeFrames, Full);
		if (Charge > 0.f)
		{
			const bool bSent = AJamSkater::CallFigure(V, NameCharge, { static_cast<double>(Charge) });
			if (!ChargeSent[I]) UE_LOG(LogJamPuckShow, Log, TEXT("JAMPUCK charge frame=%lld skater=%d human=%d on=1 call=%s"), F, I, S.bHuman ? 1 : 0, bSent ? TEXT("SetPassCharge") : TEXT("none"));
			ChargeSent[I] = true;
		}
		else if (ChargeSent[I])
		{
			ChargeSent[I] = false;
			AJamSkater::CallFigure(V, NameCharge, { 0.0 });
			UE_LOG(LogJamPuckShow, Log, TEXT("JAMPUCK charge frame=%lld skater=%d human=%d on=0 passed=%d"), F, I, S.bHuman ? 1 : 0, S.bPassThisFrame ? 1 : 0);
		}
		// PlayReceive: the frame a mate's pass is seated on this skater (the sim's own count of passes received: blade, body and the catch).
		if (S.PassesReceived > PrevReceived[I])
		{
			const bool bSent = AJamSkater::CallFigure(V, NameReceive, TConstArrayView<double>());
			++ReceivesN;
			UE_LOG(LogJamPuckShow, Log, TEXT("JAMPUCK receive frame=%lld skater=%d team=%d human=%d from=%d call=%s"), F, I, S.Team, S.bHuman ? 1 : 0, P.PassBy, bSent ? TEXT("PlayReceive") : TEXT("none"));
		}
		PrevReceived[I] = S.PassesReceived;
	}
	// PlayCallForPass: while the human carries, the mate his pass would go to now (the sim's own read of his stick's line) asks for it, if he is ahead of
	// where the human looks and no opponent stands within OpenM of him. (The brain's own "I am open" would be better: asked of lane A206.)
	if (Sk.IsValidIndex(Human) && Shown == Human && Sk[Human].CanAct())
	{
		const FJamSkaterState& H = Sk[Human];
		const FVector2D Stick = (H.BladePos - H.Pos).GetSafeNormal();
		FVector2D Aim;
		const int32 T = Stick.IsNearlyZero() ? -1 : Sim->HumanPassTarget(Human, Stick, FMath::Max(0, H.PassChargeFrames), Aim);
		if (Sk.IsValidIndex(T) && T != Human && Sk[T].Team == H.Team && Sk[T].CanAct())
		{
			constexpr double OpenM = 2.5, AheadM = 3.0;
			const FVector2D Fw(FMath::Cos(H.Yaw), FMath::Sin(H.Yaw));
			bool bOpen = FVector2D::DotProduct(Sk[T].Pos - H.Pos, Fw) >= AheadM;
			for (const FJamSkaterState& O : Sk) if (bOpen && O.Team != H.Team && !O.IsDown() && FVector2D::Distance(O.Pos, Sk[T].Pos) < OpenM) bOpen = false;
			if (bOpen && MayCall(F, LastCallFrame, T, LastCallMate))
			{
				for (AJamSkater* A : Skaters)
				{
					if (!A || A->GetSimIndex() != T) continue;
					const bool bSent = AJamSkater::CallFigure(A->GetVisualObject(), NameCall, TConstArrayView<double>());
					LastCallFrame = F; LastCallMate = T; ++CallsN;
					UE_LOG(LogJamPuckShow, Log, TEXT("JAMPUCK call frame=%lld mate=%d dist_m=%.1f call=%s"), F, T, FVector2D::Distance(Sk[T].Pos, H.Pos), bSent ? TEXT("PlayCallForPass") : TEXT("none"));
					break;
				}
			}
		}
	}
	// Device B's optional User.Carried on the puck's trail: on each change of the shown possession.
	const int32 Carried = Shown >= 0 ? 1 : 0;
	if (Puck && Carried != TrailCarried) { TrailCarried = Carried; Puck->SetTrailCarried(Carried != 0); }
}

// The strips of the play (jam.puck.Capture): see the cvar. One still a sim frame while a strip runs; the kind and the frame it began in the name.
void FJamPuckShow::Capture(const UJamSim* Sim)
{
	const int32 N = CVarJamPuckCapture.GetValueOnGameThread();
	const int32 H = Sim->GetHumanIndex();
	if (N <= 0 || !Sim->GetSkaters().IsValidIndex(H)) return;
	const FJamSkaterState& S = Sim->GetSkaters()[H];
	const int64 F = Sim->GetFrame();
	const bool bCarry = Shown == H, bPassHold = S.PassChargeFrames > 0, bShotHold = S.ChargeFrames > 0;
	if (CapFrom < 0)
	{
		const TCHAR* Kind = nullptr;
		int64 Until = F;
		if (bShotHold && !bCapShotHold) { Kind = TEXT("shot"); Until = F + 200; }
		else if (bPassHold && !bCapPassHold) { Kind = TEXT("pass"); Until = F + 200; }
		else if (S.PassesReceived > CapReceived) { Kind = TEXT("receive"); Until = F + 40; }
		else if (bCarry && !bCapCarry) { Kind = TEXT("carry"); Until = F + 90; }
		if (Kind)
		{
			int32& Taken = CapPerKind.FindOrAdd(Kind);
			if (Taken < N) { ++Taken; CapFrom = F; CapUntil = Until; CapKind = Kind; UE_LOG(LogJamPuckShow, Log, TEXT("JAMPUCK capture start kind=%s frame=%lld"), Kind, F); }
		}
	}
	else
	{
		// The release ends a held strip's wait: 45 frames more for a pass (the catch), 40 for a shot (the flight).
		if (CapKind == TEXT("pass") && S.bPassThisFrame) { CapUntil = FMath::Min(CapUntil, F + 45); UE_LOG(LogJamPuckShow, Log, TEXT("JAMPUCK capture pass released frame=%lld strength=%.2f"), F, S.LastPassStrength); }
		if (CapKind == TEXT("shot") && S.bShotThisFrame) { CapUntil = FMath::Min(CapUntil, F + 40); UE_LOG(LogJamPuckShow, Log, TEXT("JAMPUCK capture shot released frame=%lld charge=%.2f puck_mps=%.1f"), F, S.LastShotCharge, Sim->GetPuck().Vel.Size()); }
		if ((CapKind == TEXT("pass") && !bPassHold && !S.bPassThisFrame && CapUntil > F + 45) || (CapKind == TEXT("shot") && !bShotHold && !S.bShotThisFrame && CapUntil > F + 40)) CapUntil = F;   // a hold that ended with nothing
	}
	bCapCarry = bCarry; bCapPassHold = bPassHold; bCapShotHold = bShotHold; CapReceived = S.PassesReceived;
	if (CapFrom < 0) return;
	static FString ShotDir;
	static bool bShotDirRead = false;
	if (!bShotDirRead) { bShotDirRead = true; FParse::Value(FCommandLine::Get(), TEXT("-JamHitShots="), ShotDir); ShotDir.ReplaceInline(TEXT("\\"), TEXT("/")); }
	const FString Name = FString::Printf(TEXT("a196-%s-%06lld-f%06lld.png"), *CapKind, CapFrom, F);
	FScreenshotRequest::RequestScreenshot(ShotDir.IsEmpty() ? Name : ShotDir / Name, true, false);
	if (F >= CapUntil) { UE_LOG(LogJamPuckShow, Log, TEXT("JAMPUCK capture end kind=%s from=%lld to=%lld"), *CapKind, CapFrom, F); CapFrom = -1; }
}

void FJamPuckShow::Render(const UJamSim* Sim, const TArray<TObjectPtr<AJamSkater>>& Skaters, AJamPuck* Puck, float Alpha)
{
	if (!Sim || !Puck) return;
	const FJamPuckState& P = Sim->GetPuck();
	const double FrameF = static_cast<double>(Sim->GetFrame()) + Alpha;
	FVector SocketCm;
	// The probe's lag: what the puck's late tick measured on the frame just drawn (how far the blade's socket had moved from where the puck was placed).
	if (const double Lag = Puck->GetLastLagCm(); Lag >= 0.0 && CVarJamPuckProbe.GetValueOnGameThread() > 0)
	{
		++LagN; LagSumCm += Lag; LagMaxCm = FMath::Max(LagMaxCm, Lag); if (Lag > 1.0) ++LagOver1N; if (Lag > OnSocketCm) ++LagOver10N;
	}
	if (!ShowOn())
	{
		// Package H34's block: at the sim's carrier's socket when the visual has one, else where the sim has it.
		const int32 Carrier = Sim->CarrierIndex();
		if (Skaters.IsValidIndex(Carrier) && Skaters[Carrier] && Skaters[Carrier]->GetPuckSocketWorld(SocketCm)) { Puck->ApplyRenderAt(SocketCm); LastDrawn = FVector(SocketCm.X, SocketCm.Y, Puck->GetHalfHeightCm()); }
		else { Puck->ApplyRender(P, Alpha); LastDrawn = Puck->LiveRenderCm(P, Alpha); }
		DrawnCarrier = Carrier;
		bEverDrawn = true;
		bool bIsSocket = false;
		USceneComponent* Comp = Skaters.IsValidIndex(Carrier) && Skaters[Carrier] ? Skaters[Carrier]->FindPuckSocket(bIsSocket) : nullptr;
		Puck->SetFollow(Comp, bIsSocket, CVarJamPuckFollow.GetValueOnGameThread() != 0);
	}
	else
	{
		const FVector SimCm = Puck->LiveRenderCm(P, Alpha);
		const bool bSocket = Skaters.IsValidIndex(Shown) && Skaters[Shown] && Skaters[Shown]->GetPuckSocketWorld(SocketCm);
		const int32 Now = bSocket ? Shown : -1;
		const FVector Target = bSocket ? FVector(SocketCm.X, SocketCm.Y, Puck->GetHalfHeightCm()) : SimCm;
		if (Now != DrawnCarrier)
		{
			// A change of hands: what the drawn puck is off its new place by runs out over the blend. A placed puck (a faceoff, a restart: metres away) cuts.
			OffsetAtChange = bEverDrawn ? LastDrawn - Target : FVector::ZeroVector;
			if (OffsetAtChange.SizeSquared() > 300.0 * 300.0) OffsetAtChange = FVector::ZeroVector;
			ChangeFrameF = FrameF;
			ChangeBlend = Now >= 0 ? BlendInFrames : BlendOutFrames;
			DrawnCarrier = Now;
		}
		LastDrawn = Drawn(Target, OffsetAtChange, static_cast<float>(FrameF - ChangeFrameF), ChangeBlend);
		Puck->ApplyRenderBlend(LastDrawn, Alpha);
		bEverDrawn = true;
		// On the blade (the blend done): the puck follows the socket through this frame's animation too.
		bool bIsSocket = false;
		USceneComponent* Comp = bSocket && FrameF - ChangeFrameF >= ChangeBlend ? Skaters[Shown]->FindPuckSocket(bIsSocket) : nullptr;
		Puck->SetFollow(Comp, bIsSocket, CVarJamPuckFollow.GetValueOnGameThread() != 0);
	}
	// The probe: once per sim frame, at the first drawn frame after the step.
	const int32 Every = CVarJamPuckProbe.GetValueOnGameThread();
	if (Every <= 0 || Sim->GetFrame() == ProbedFrame) return;
	ProbedFrame = Sim->GetFrame();
	const int32 Carrier = Sim->CarrierIndex();
	const int32 Human = Sim->GetHumanIndex();
	if (Skaters.IsValidIndex(Carrier) && Skaters[Carrier] && Skaters[Carrier]->GetPuckSocketWorld(SocketCm))
	{
		const int32 G = Carrier == Human ? 0 : 1;
		const double D = FVector::Dist2D(LastDrawn, SocketCm);
		++CarryN[G];
		if (D <= OnSocketCm) ++OnSocketN[G];
		if (ShowOn() && DrawnCarrier == Carrier && FrameF - ChangeFrameF < ChangeBlend) ++BlendN[G];   // inside a pickup's blend
		WorstCm[G] = FMath::Max(WorstCm[G], D);
		// The drawn puck against the sim's (what a poke aimed by eye is off by), and every 10th frame of the human's carry where the socket is in his frame.
		const FVector SimCm(P.Pos.X * 100.0, P.Pos.Y * 100.0, 0.0);
		const double ToSim = FVector::Dist2D(LastDrawn, SimCm);
		ToSimSumCm[G] += ToSim; ToSimMaxCm[G] = FMath::Max(ToSimMaxCm[G], ToSim);
		if (G == 0 && Sim->GetFrame() % 10 == 0)
		{
			const FTransform Inv = Skaters[Carrier]->GetActorTransform().Inverse();
			const FVector Sl = Inv.TransformPosition(SocketCm), Pl = Inv.TransformPosition(SimCm);
			UE_LOG(LogJamPuckShow, Log, TEXT("JAMPUCK socket frame=%lld socket=(%.0f, %.0f) sim_puck=(%.0f, %.0f) apart_cm=%.0f sent_aim_deg=%.1f reach_m=%.2f speed_mps=%.1f (the human's frame: x ahead, y right, cm)"),
				Sim->GetFrame(), Sl.X, Sl.Y, Pl.X, Pl.Y, ToSim, Skaters[Carrier]->GetSentAimDeg(), Skaters[Carrier]->GetSentReachM(), Sim->GetSkaters()[Carrier].Vel.Size());
		}
	}
	for (const AJamSkater* A : Skaters)
	{
		if (!A || !A->IsToldHasPuck()) continue;
		const int32 G = A->GetSimIndex() == Human ? 0 : 1;
		++FlagN[G];
		if (A->GetSimIndex() != Carrier) ++FlagNotCarrierN[G];
	}
	if (++ProbeFrames < Every) return;
	const auto Pct = [](int32 A, int32 B) { return B > 0 ? 100.0 * A / B : 0.0; };
	UE_LOG(LogJamPuckShow, Log, TEXT("JAMPUCK show frame=%lld frames=%d show=%d keep=%d human: carry=%d on_socket=%d (%.1f %%) in_blend=%d worst_cm=%.0f flagged=%d flagged_not_carrier=%d (%.1f %%) bots: carry=%d on_socket=%d (%.1f %%) in_blend=%d worst_cm=%.0f flagged=%d flagged_not_carrier=%d (%.1f %%) kept_frames=%d receives=%d calls=%d | drawn to sim puck: human mean_cm=%.0f max_cm=%.0f bots mean_cm=%.0f max_cm=%.0f | blade lag before the follow: frames=%d mean_cm=%.1f max_cm=%.1f over_1cm=%d (%.1f %%) over_10cm=%d follow=%d"),
		Sim->GetFrame(), ProbeFrames, ShowOn() ? 1 : 0, CVarJamPuckKeep.GetValueOnGameThread(),
		CarryN[0], OnSocketN[0], Pct(OnSocketN[0], CarryN[0]), BlendN[0], WorstCm[0], FlagN[0], FlagNotCarrierN[0], Pct(FlagNotCarrierN[0], FlagN[0]),
		CarryN[1], OnSocketN[1], Pct(OnSocketN[1], CarryN[1]), BlendN[1], WorstCm[1], FlagN[1], FlagNotCarrierN[1], Pct(FlagNotCarrierN[1], FlagN[1]), KeptN, ReceivesN, CallsN,
		CarryN[0] > 0 ? ToSimSumCm[0] / CarryN[0] : 0.0, ToSimMaxCm[0], CarryN[1] > 0 ? ToSimSumCm[1] / CarryN[1] : 0.0, ToSimMaxCm[1],
		LagN, LagN > 0 ? LagSumCm / LagN : 0.0, LagMaxCm, LagOver1N, Pct(LagOver1N, LagN), LagOver10N, CVarJamPuckFollow.GetValueOnGameThread());
	ProbeFrames = 0;
	for (int32 g = 0; g < 2; ++g) { CarryN[g] = OnSocketN[g] = FlagN[g] = FlagNotCarrierN[g] = BlendN[g] = 0; ToSimSumCm[g] = ToSimMaxCm[g] = 0.0; WorstCm[g] = 0.0; }
	KeptN = ReceivesN = CallsN = 0;
	LagN = LagOver1N = LagOver10N = 0; LagSumCm = LagMaxCm = 0.0;
}

#if WITH_DEV_AUTOMATION_TESTS

// Jam.Puck.Show: the carrier the screen shows, the drawn place's blend, the play calls' rules. Pure: no world.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPuckShowTest, "Jam.Puck.Show", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamPuckShowTest::RunTest(const FString&)
{
	TestEqual(TEXT("ships: the shown carrier follows the sim's"), CVarJamPuckShow.GetValueOnGameThread(), 1);
	TestEqual(TEXT("ships: the play calls"), CVarJamPuckCalls.GetValueOnGameThread(), 1);
	TestEqual(TEXT("ships: the carried puck follows the blade after the animation"), CVarJamPuckFollow.GetValueOnGameThread(), 1);
	int32 Kept = 0;
	// The sim names a carrier: he is shown, whoever was before.
	TestEqual(TEXT("the sim's carrier is the shown one"), FJamPuckShow::ShownCarrier(-1, 3, 3, false, false, 0.0, 0.0, 6, Kept), 3);
	TestEqual(TEXT("a change of hands the sim names is taken at once"), FJamPuckShow::ShownCarrier(3, 7, 7, true, false, 1.0, 1.6, 6, Kept), 7);
	TestEqual(TEXT("nothing kept while the sim names him"), Kept, 0);
	// The sim names nobody for a frame, the puck still his and inside 1.35 of his reach: kept.
	TestEqual(TEXT("a one-frame drop-out is bridged"), FJamPuckShow::ShownCarrier(7, -1, 7, true, false, 1.9, 1.6, 6, Kept), 7);
	TestEqual(TEXT("one frame kept"), Kept, 1);
	for (int32 i = 0; i < 5; ++i) FJamPuckShow::ShownCarrier(7, -1, 7, true, false, 1.9, 1.6, 6, Kept);
	TestEqual(TEXT("six frames kept"), Kept, 6);
	TestEqual(TEXT("and no longer: the seventh frame shows nobody"), FJamPuckShow::ShownCarrier(7, -1, 7, true, false, 1.9, 1.6, 6, Kept), -1);
	// Never kept: down or hit, a shot or a pass, another man's touch, the puck out of reach, the switch at 0.
	Kept = 0;
	TestEqual(TEXT("down or hit: not kept"), FJamPuckShow::ShownCarrier(7, -1, 7, false, false, 1.0, 1.6, 6, Kept), -1);
	TestEqual(TEXT("he shot or passed: not kept"), FJamPuckShow::ShownCarrier(7, -1, 7, true, true, 1.0, 1.6, 6, Kept), -1);
	TestEqual(TEXT("another man touched it: not kept"), FJamPuckShow::ShownCarrier(7, -1, 2, true, false, 1.0, 1.6, 6, Kept), -1);
	TestEqual(TEXT("the puck beyond 1.35 of his reach: not kept"), FJamPuckShow::ShownCarrier(7, -1, 7, true, false, 1.6 * FJamPuckShow::KeepReachK + 0.01, 1.6, 6, Kept), -1);
	TestEqual(TEXT("jam.puck.Keep 0: the sim's carrier exactly"), FJamPuckShow::ShownCarrier(7, -1, 7, true, false, 1.0, 1.6, 0, Kept), -1);
	TestEqual(TEXT("nobody before, nobody now"), FJamPuckShow::ShownCarrier(-1, -1, 7, true, false, 1.0, 1.6, 6, Kept), -1);
	// The drawn place: the whole offset on the frame of the change, none after the blend, on the blade exactly from then on.
	const FVector Blade(100, 0, 4), Off(60, 0, 0);
	TestTrue(TEXT("the frame of the pickup: still where it was drawn"), FJamPuckShow::Drawn(Blade, Off, 0.f, FJamPuckShow::BlendInFrames).Equals(Blade + Off, 1e-3));
	TestTrue(TEXT("a frame later: most of the way"), FVector::Dist(FJamPuckShow::Drawn(Blade, Off, 1.f, FJamPuckShow::BlendInFrames), Blade) < 0.5 * Off.Size());
	TestTrue(TEXT("after the blend: on the blade"), FJamPuckShow::Drawn(Blade, Off, static_cast<float>(FJamPuckShow::BlendInFrames), FJamPuckShow::BlendInFrames).Equals(Blade, 1e-3));
	TestTrue(TEXT("no blend frames: on the blade at once"), FJamPuckShow::Drawn(Blade, Off, 0.f, 0).Equals(Blade, 1e-3));
	TestTrue(TEXT("the blend is two frames each way"), FJamPuckShow::BlendInFrames == 2 && FJamPuckShow::BlendOutFrames == 2);
	// The human's carried stick out of the zone his hips hide (AJamSkater::CarryAimDeg; floor 30 degrees).
	TestEqual(TEXT("ships: the carry floor is 30 degrees"), AJamSkater::CarryAimFloorDeg(), 30.f, 1e-4f);
	TestEqual(TEXT("not carrying: the aim as it is"), AJamSkater::CarryAimDeg(5.f, 0.f, false, 30.f), 5.f, 1e-4f);
	TestEqual(TEXT("carrying a hair to the forehand: the forehand floor (45)"), AJamSkater::CarryAimDeg(1.f, 0.f, true, 30.f), 45.f, 1e-4f);
	TestEqual(TEXT("carrying a hair to the backhand: the backhand floor (-30)"), AJamSkater::CarryAimDeg(-1.f, 0.f, true, 30.f), -30.f, 1e-4f);
	TestEqual(TEXT("on the forehand, a little to the backhand: stays"), AJamSkater::CarryAimDeg(-8.f, 45.f, true, 30.f), 45.f, 1e-4f);
	TestEqual(TEXT("on the forehand, clearly to the backhand: crosses"), AJamSkater::CarryAimDeg(-15.f, 45.f, true, 30.f), -30.f, 1e-4f);
	TestEqual(TEXT("on the backhand, a little to the forehand: stays"), AJamSkater::CarryAimDeg(10.f, -30.f, true, 30.f), -30.f, 1e-4f);
	TestEqual(TEXT("on the backhand, clearly to the forehand: crosses"), AJamSkater::CarryAimDeg(20.f, -30.f, true, 30.f), 45.f, 1e-4f);
	TestEqual(TEXT("outside the forehand floor: the aim as it is"), AJamSkater::CarryAimDeg(47.f, 45.f, true, 30.f), 47.f, 1e-4f);
	TestEqual(TEXT("outside the backhand floor: the aim as it is"), AJamSkater::CarryAimDeg(-33.f, -30.f, true, 30.f), -33.f, 1e-4f);
	TestEqual(TEXT("the floor at 0: off"), AJamSkater::CarryAimDeg(0.f, 0.f, true, 0.f), 0.f, 1e-4f);
	// SetPassCharge: 0 without a hold, 1 at full_frames, never over.
	TestEqual(TEXT("no hold: 0"), FJamPuckShow::PassCharge01(0, 30), 0.f, 1e-6f);
	TestEqual(TEXT("a lost puck's negative count: 0"), FJamPuckShow::PassCharge01(-1, 30), 0.f, 1e-6f);
	TestEqual(TEXT("half the hold: 0.5"), FJamPuckShow::PassCharge01(15, 30), 0.5f, 1e-6f);
	TestEqual(TEXT("the full hold: 1"), FJamPuckShow::PassCharge01(30, 30), 1.f, 1e-6f);
	TestEqual(TEXT("past it: 1"), FJamPuckShow::PassCharge01(45, 30), 1.f, 1e-6f);
	// PlayCallForPass: at most every 0.5 s, the same mate only after 2.5 s.
	TestTrue(TEXT("the first ask"), FJamPuckShow::MayCall(1000, -100000, 3, -1));
	TestFalse(TEXT("another mate 10 frames later: too soon"), FJamPuckShow::MayCall(1010, 1000, 4, 3));
	TestTrue(TEXT("another mate half a second later"), FJamPuckShow::MayCall(1000 + FJamPuckShow::CallEveryFrames, 1000, 4, 3));
	TestFalse(TEXT("the same mate a second later: not yet"), FJamPuckShow::MayCall(1060, 1000, 3, 3));
	TestTrue(TEXT("the same mate after 2.5 s"), FJamPuckShow::MayCall(1000 + FJamPuckShow::CallSameMateFrames, 1000, 3, 3));
	TestFalse(TEXT("nobody to call"), FJamPuckShow::MayCall(5000, 1000, -1, 3));
	return true;
}

#endif
