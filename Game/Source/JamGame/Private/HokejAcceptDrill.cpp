// Lane A87 (R-0043 point 1): the acceptance run's drill, jam.ctrl.Drill 20. The human plays through the real input path (ReadHumanInput: the cursor on the ice,
// the keys, the buttons); this drill only stages each step (SetSkater / SetPuck: the human, one mate, one opponent; every other skater parked in the human's own
// corner), scripts the mate and the opponent, tells the driver (tools/qa/accept/accept.ps1) where the cursor goes and what to press, and judges:
//   JAMACCEPT begin ...                                   the indices and the config numbers the pass rules use
//   JAMACCEPT cursor step=S k=K sx=<px> sy=<px> ...        the driver puts the cursor there (client pixels = viewport pixels) and keeps it there
//   JAMACCEPT act step=S k=K act=<what> ...                the driver presses (keys, lmb, pass, rmb, mmb, wheel, key, sweep, pause_resume, lmb_onetimer, probe_pass)
//   JAMACCEPT received step=6 k=K frame=F                  the one-timer's pass landed on the human (the driver releases the held button)
//   JAMACCEPT step=S verb=V ok=0|1 <facts>                one line per verb; tools/qa/accept/accept.mjs makes the table
//   JAMACCEPT summary ok=N fail=M                           the end
// Steps (docs/experiments/ACCEPT.md): 1 skate / sprint / sprint_carry / brake (lane A93, R-0045 laws 3 and 4), 2 aim, 3 carry, 4 shoot_tap / shoot_half / shoot_full, 5 pass / saucer, 6 onetimer,
// 7 check / crosscheck, 8 checked, 9 deke / spin / sweep, 10 resume_aim. jam.accept.From / jam.accept.To run a part. A QA drill: nothing here runs unless the cvar is 20.
#include "HokejGameMode.h"
#include "InputMapping.h"   // lane A191: JAM_INPUT_HAS_CROSS_HOLD (the held cross-check row)
#include "JamSim.h"
#include "JamPawns.h"
#include "JamEventBus.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformTime.h"

static TAutoConsoleVariable<int32> CVarJamAcceptFrom(TEXT("jam.accept.From"), 1, TEXT("Lane A87: the first step of the acceptance run (jam.ctrl.Drill 20)."));
static TAutoConsoleVariable<int32> CVarJamAcceptTo(TEXT("jam.accept.To"), 10, TEXT("Lane A87: the last step of the acceptance run (jam.ctrl.Drill 20)."));

#define ACC_LOG(Fmt, ...) UE_LOG(LogHokej, Log, TEXT("JAMACCEPT ") Fmt, ##__VA_ARGS__)

namespace JamAccept
{
	enum class EPh : uint8 { Begin, Stage, Settle, Hand, Measure, Gap, Done };

	struct FState
	{
		EPh Ph = EPh::Begin;
		int32 Step = 0, K = 0, Last = 10;
		int64 PhFrame = 0;          // the frame this phase began
		int32 Me = -1, Mate = -1, Opp = -1;
		TArray<int32> Parked;
		bool bMateLive = false, bOppLive = false, bPinPuck = true;
		FVector2D PuckPark = FVector2D::ZeroVector;
		double Dir = 1.0, Gx = 0.0;
		int32 NOk = 0, NFail = 0;
		// the human's own input this frame (read before the step)
		double MoveIn = 0.0;
		// measurement scratch (reset at each stage)
		int64 ActFrame = -1, MoveFrame = -1, EventFrame = -1, DownFrame = -1, UpFrame = -1, ShotFrame = -1, RecvFrame = -1, CueFrame = -1;
		double MaxSpeed = 0.0, Closing = 0.0, CamMaxM = 0.0, PuckDistAtAct = -1.0, LastWall = 0.0, GapS = 0.0;
		int32 SprintFrames = 0, SprintCarryFrames = 0, CarryFrames = 0, ChargeMax = 0, Sweeps0 = 0, Received0 = 0, MateRecv0 = 0, StickHit = 0;
		double PassV0 = 0.0;   // lane A207: the pass's release speed (step 5)
		// Lane A92 (R-0045 law 1): the hit by momentum as the sim resolved it: the victim's ContactOutcome (1 bump, 2 stumble, 3 down), its ratio, the frames it went down for.
		int32 HitOutcome = 0, KnockAt = 0;
		double HitRatio = 0.0;
		bool bLost = false, bShot = false, bGoal = false, bOneTimer = false, bSaucer = false, bPassed = false, bRecv = false, bCheck = false, bKnock = false;
		bool bStrip = false, bCamOff = false, bFired = false, bPaused = false, bMatePassed = false;
		int64 LostFrame = -1;
		// Lane A93 (R-0045 law 3), step 1 k=4 "brake": the human's move input this frame, the S line sent, the velocity's direction when W was released, the
		// frame the S input arrived against the velocity and the speed there, the frame the skater was still, the best speed backward after it, frames of the sim's reverse brake.
		FVector2D MoveVec = FVector2D::ZeroVector, Fwd0 = FVector2D::ZeroVector;
		bool bSAct = false;
		int64 RevFrame = -1, StopFrame = -1;
		double RevSpeed = 0.0, BackSpeed = 0.0;
		int32 BrakeFrames = 0;
		float ShotCharge = 0.f;
		FVector2D Target = FVector2D::ZeroVector;
		int64 ArriveFrame = -1, FollowFrame = -1;
		double ErrAtEnd = 0.0;
		// per-step aggregates
		double Speed1 = 0.0, Speed2 = 0.0;
		FString AimFacts;
		int32 AimOk = 0, OtOk = 0;
		FString OtFacts;
	};
	static FState G;

	static double YawOf(const FVector2D& D) { return FMath::Atan2(D.Y, D.X); }
	static FVector2D Unit(double Rad) { return FVector2D(FMath::Cos(Rad), FMath::Sin(Rad)); }
	static double AngDeg(const FVector2D& A, const FVector2D& B)
	{
		const FVector2D a = A.GetSafeNormal(), b = B.GetSafeNormal();
		if (a.IsNearlyZero() || b.IsNearlyZero()) return 180.0;
		return FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector2D::DotProduct(a, b), -1.0, 1.0)));
	}
	// Steps and the number of sub-attempts each has.
	static int32 SubsOf(int32 Step)
	{
		switch (Step) { case 1: return 4; case 2: return 4; case 3: return 1; case 4: return 3; case 5: return 3; case 6: return 2; case 7: return 3; case 8: return 1; case 9: return 3; case 10: return 2; default: return 0; }
	}
}

void AHokejGameMode::AcceptDrillInputs(TArray<FJamInput>& All)
{
	using namespace JamAccept;
	if (!Sim) return;
	if (Sim->GetPhase() == EJamSimPhase::Faceoff) Sim->SkipFaceoff();   // no faceoffs in the run: the scene is staged
	const TArray<FJamSkaterState>& Sk = Sim->GetSkaters();
	const int32 Me = Sim->GetHumanIndex();
	if (!Sk.IsValidIndex(Me) || !All.IsValidIndex(Me)) return;
	if (G.Ph == EPh::Begin)
	{
		G = FState();
		G.Me = Me;
		const int32 Team = Sk[Me].Team;
		for (const FJamSkaterState& S : Sk)
		{
			if (S.Index == Me) continue;
			if (S.Team == Team && G.Mate < 0 && S.Role != FName(TEXT("back"))) G.Mate = S.Index;
			else if (S.Team != Team && G.Opp < 0 && S.Role != FName(TEXT("back"))) G.Opp = S.Index;
		}
		for (const FJamSkaterState& S : Sk) if (S.Index != Me && S.Index != G.Mate && S.Index != G.Opp) G.Parked.Add(S.Index);
		G.Gx = Sim->AttackGoalX(Team);
		G.Dir = G.Gx > 0 ? 1.0 : -1.0;
		G.PuckPark = FVector2D(-G.Dir * 20.0, -18.0);
		G.Step = FMath::Clamp(CVarJamAcceptFrom.GetValueOnGameThread(), 1, 10);
		G.Last = FMath::Clamp(CVarJamAcceptTo.GetValueOnGameThread(), G.Step, 10);
		G.K = 1;
		G.Ph = EPh::Stage;
		G.PhFrame = Sim->GetFrame();
		G.LastWall = FPlatformTime::Seconds();
		const FJamGameConfig& C = Sim->GetConfig();
		ACC_LOG(TEXT("begin frame=%lld human=%d mate=%d opp=%d parked=%d attack_x=%.1f shot_reach=%.2f stick_reach=%.2f carry_reach=%.2f charge_max=%d saucer_half=%d onetimer_window=%d knock_frames=%d from=%d to=%d"),
			Sim->GetFrame(), Me, G.Mate, G.Opp, G.Parked.Num(), G.Gx, Sim->GetEffectiveStat(EJamStat::ShotReach, Me), C.Stick.ReachM, UJamSim::SkaterRadius + C.Stick.ReachM,
			C.Shot.ChargeFramesMax, C.Pass.SaucerHalfFrames, C.Shot.OneTimerWindowFrames, C.Skater.KnockdownFrames, G.Step, G.Last);
	}
	G.MoveIn = All[Me].Move.Size();
	G.MoveVec = All[Me].Move;
	// Everyone but the human: still, unless the step scripts the mate or the opponent below. Parked skaters sit in the human's own corner.
	for (int32 i = 0; i < All.Num(); ++i) if (i != Me) All[i] = FJamInput();
	int32 Slot = 0;
	auto Park = [&](int32 i) { if (Sk.IsValidIndex(i)) Sim->SetSkater(i, FVector2D(-G.Dir * 34.0, -16.0 + 6.5 * Slot++), FVector2D::ZeroVector, 0.0); };
	for (int32 i : G.Parked) Park(i);
	if (!G.bMateLive) Park(G.Mate);
	if (!G.bOppLive) Park(G.Opp);
	if (G.bPinPuck && Sim->GetPhase() == EJamSimPhase::Playing) Sim->SetPuck(G.PuckPark, FVector2D::ZeroVector);
	if (G.Ph != EPh::Measure && G.Ph != EPh::Hand && G.Ph != EPh::Settle) return;
	const int64 F = Sim->GetFrame();
	const FJamSkaterState& H = Sk[Me];
	// The mate (steps 5, 6): stands with its blade toward the human; in step 6 it carries and passes once the human is open.
	if (G.bMateLive && Sk.IsValidIndex(G.Mate) && All.IsValidIndex(G.Mate))
	{
		const FJamSkaterState& M = Sk[G.Mate];
		FJamInput& In = All[G.Mate];
		In.Stick = (H.Pos - M.Pos).GetSafeNormal() * 0.5;
		if (G.Step == 6 && G.Ph == EPh::Measure && !G.bMatePassed && Sim->CarrierIndex() == G.Mate && F >= M.ShotReadyFrame)
		{
			FVector2D Aim;
			const int32 To = Sim->BestPass(G.Mate, Aim);
			if (To == Me || F - G.PhFrame > 120)
			{
				In.bPass = true;
				G.bMatePassed = true;
				G.CueFrame = F;
				const double Dist = FVector2D::Distance(H.BladePos, Sim->GetPuck().Pos);
				const double Speed = FMath::Max(1.0, Sim->GetConfig().Skater.PassSpeed);
				const int32 Eta = FMath::CeilToInt32(Dist / (Speed * 0.85) / UJamSim::DT);
				ACC_LOG(TEXT("act step=6 k=%d act=lmb_onetimer eta_frames=%d best_pass=%d frame=%lld"), G.K, Eta, To, F);
			}
		}
	}
	// The opponent (steps 7, 8): in 7 it carries slowly at the human; in 8 it charges the carrying human and checks at contact (the A49 drill's rule).
	if (G.bOppLive && Sk.IsValidIndex(G.Opp) && All.IsValidIndex(G.Opp))
	{
		const FJamSkaterState& O = Sk[G.Opp];
		FJamInput& In = All[G.Opp];
		const FVector2D ToH = H.Pos - O.Pos;
		const double Dist = ToH.Size();
		// Lane A92: in the check sub-step the carrier skates at the human; in the cross-check sub-step it stands (the vision's case: the thrust from the front beats a
		// standing skater), a creep that keeps the puck on its blade. Under the neutral block both skate as before.
		// It holds the puck behind its body there (the stick back at full reach), out of the human's blade: the hit comes at its chest, the puck stays its own.
		const bool bStand = G.Step == 7 && G.K >= 2 && Sim->GetConfig().Contact.bMomentum;   // lane A191: k=3 (the held cross-check) the same
		if (G.Step == 7) { In.Move = ToH.GetSafeNormal() * (bStand ? 0.04 : 0.3); In.Stick = bStand ? -O.Forward() : O.Forward() * 0.5; }
		if (G.Step == 8 && G.Ph == EPh::Measure)
		{
			In.Move = ToH.GetSafeNormal();
			In.bSprint = true;
			const double Gap = Dist - UJamSim::SkaterRadius * 2.0;
			const double Closing = Dist > 1e-6 ? FVector2D::DotProduct(O.Vel - H.Vel, ToH / Dist) : 0.0;
			// Lane A192: the window opens game.contact.check_windup_frames after the press: the opponent presses that much earlier (as the brain does).
			const int32 Wu = Sim->GetConfig().Contact.CheckWindupFor(O.bHuman);   // lane A200: the presser's own (bot_check_windup_frames for a bot)
			if ((Gap <= 0.25 + Closing * Wu * UJamSim::DT || (Closing > 0.5 && Gap <= Closing * (UJamSim::CheckWindowFrames - 2 + Wu) * UJamSim::DT)) && !H.IsDown() && O.CanAct() && G.DownFrame < 0)
			{
				In.bCheck = true;
				if (G.CueFrame < 0) { G.CueFrame = F; G.Closing = Closing; }
			}
		}
	}
}

void AHokejGameMode::AcceptDrillObserve()
{
	using namespace JamAccept;
	if (!Sim || G.Ph == EPh::Begin || G.Ph == EPh::Done) return;
	const TArray<FJamSkaterState>& Sk = Sim->GetSkaters();
	const int32 Me = G.Me;
	if (!Sk.IsValidIndex(Me)) return;
	const int64 F = Sim->GetFrame();
	const FJamSkaterState& H = Sk[Me];
	const FJamPuckState& P = Sim->GetPuck();
	const FJamGameConfig& Cfg = Sim->GetConfig();
	const double Now = FPlatformTime::Seconds();
	const double WallGap = Now - G.LastWall;
	G.LastWall = Now;
	AJamSkater* HumanPawn = GetHumanSkater();
	APlayerController* PC = HumanPawn ? Cast<APlayerController>(HumanPawn->GetController()) : nullptr;
	const FVector2D Fwd(G.Dir, 0.0);
	const double FaceYaw = G.Dir > 0 ? 0.0 : PI;
	auto Screen = [&](const FVector2D& RinkM, FVector2D& Out) -> bool { return PC && PC->ProjectWorldLocationToScreen(UJamSim::ToWorld(RinkM), Out, false); };
	auto CursorLine = [&](const FVector2D& RinkM, const TCHAR* What)
	{
		FVector2D S2(-1, -1);
		const bool bOk = Screen(RinkM, S2);
		ACC_LOG(TEXT("cursor step=%d k=%d sx=%d sy=%d on_screen=%d what=%s rink=(%.2f,%.2f) frame=%lld"), G.Step, G.K, FMath::RoundToInt32(S2.X), FMath::RoundToInt32(S2.Y), bOk ? 1 : 0, What, RinkM.X, RinkM.Y, F);
	};
	// A cursor target along Dir from the skater: the farthest of Max..Min metres whose projection sits inside the viewport with a 24 px margin (the ice right
	// behind the skater lies under the third-person camera; a mate 8 m to the side lies past the screen's edge at 6 m).
	auto OnScreenTarget = [&](const FVector2D& From, const FVector2D& D, double MaxM, double MinM) -> FVector2D
	{
		int32 VX = 0, VY = 0;
		if (PC) PC->GetViewportSize(VX, VY);
		for (double M = MaxM; M >= MinM - 1e-6; M -= 0.25)
		{
			FVector2D S2;
			if (Screen(From + D * M, S2) && S2.X >= 24 && S2.Y >= 24 && S2.X <= VX - 24 && S2.Y <= VY - 24) return From + D * M;
		}
		return From + D * MinM;
	};
	auto CursorIce = [&](FVector2D& Out) -> bool
	{
		float MX = 0.f, MY = 0.f;
		FVector O, D;
		if (!PC || !PC->GetMousePosition(MX, MY) || !PC->DeprojectScreenPositionToWorld(MX, MY, O, D) || D.Z >= -1e-4) return false;
		const FVector Hit = O + D * (-O.Z / D.Z);
		Out = FVector2D(Hit.X / 100.0, Hit.Y / 100.0);
		return true;
	};
	auto Verdict = [&](const TCHAR* Verb, bool bOk, const FString& Facts)
	{
		(bOk ? G.NOk : G.NFail)++;
		ACC_LOG(TEXT("step=%d verb=%s ok=%d frame=%lld %s"), G.Step, Verb, bOk ? 1 : 0, F, *Facts);
	};
	auto Hand = [&](int32 To)
	{
		G.bPinPuck = false;
		const FJamSkaterState& T = Sk[To];
		Sim->SetPuck(T.Pos + T.Forward() * 0.45, T.Vel);
	};
	auto Next = [&]()
	{
		if (G.K < SubsOf(G.Step)) ++G.K;
		else
		{
			++G.Step; G.K = 1;
			if (G.Step > G.Last)
			{
				G.Ph = EPh::Done;
				ACC_LOG(TEXT("summary ok=%d fail=%d frame=%lld"), G.NOk, G.NFail, F);
				return;
			}
		}
		G.Ph = EPh::Gap;
		G.PhFrame = F;
	};
	const int64 InPh = F - G.PhFrame;

	if (G.Ph == EPh::Gap)
	{
		if (InPh >= 30 && Sim->GetPhase() == EJamSimPhase::Playing) { G.Ph = EPh::Stage; G.PhFrame = F; }
		return;
	}
	if (G.Ph == EPh::Stage)
	{
		if (Sim->GetPhase() != EJamSimPhase::Playing) return;
		// Director, 14 Nov 17:15 (the H33 release, second cut): step 7's rows share one opponent. With the cross-check's wind-up the pressed shove of k=2
		// lands 4 frames later, so its man was still down when k=3 was staged: the stage's ClearReleaseGrace came before his get-up, the get-up then gave
		// him his two seconds of grace, and the held cross-check at 14.7 m/s was a bump (JAMVIS grace grace_left=75). The stage waits for a man on his feet
		// (bounded: 300 frames), so the grace it clears is the one he would have carried.
		if (G.Step == 7 && G.K > 1 && G.Opp >= 0 && Sim->GetSkaters().IsValidIndex(G.Opp) && Sim->GetSkaters()[G.Opp].IsDown() && InPh <= 300) return;
		// Reset the scratch, stage the step's scene.
		const FState Keep = G;
		G = FState();
		G.Ph = EPh::Settle; G.Step = Keep.Step; G.K = Keep.K; G.Last = Keep.Last; G.Me = Keep.Me; G.Mate = Keep.Mate; G.Opp = Keep.Opp; G.Parked = Keep.Parked;
		G.PuckPark = Keep.PuckPark; G.Dir = Keep.Dir; G.Gx = Keep.Gx; G.NOk = Keep.NOk; G.NFail = Keep.NFail; G.LastWall = Keep.LastWall;
		G.Speed1 = Keep.Speed1; G.Speed2 = Keep.Speed2; G.AimFacts = Keep.AimFacts; G.AimOk = Keep.AimOk; G.OtOk = Keep.OtOk; G.OtFacts = Keep.OtFacts;
		if (G.K == 1) { G.AimFacts.Reset(); G.AimOk = 0; G.OtOk = 0; G.OtFacts.Reset(); }
		G.PhFrame = F;
		G.bPinPuck = true;
		// Lane A179 (R-0065 item 2): a cut without a mate (a two-a-side whose other skater is a back, the old Marathon) or without a non-back opponent has nothing for the
		// steps that stage one: G.Mate / G.Opp stay -1 and Sk[-1] crashed the game (3 of 3 matrix runs). The step is skipped and each of its verbs is recorded as
		// `skipped=no_mate` / `skipped=no_opp` (ok=0 but not a fail: tools/qa/accept/accept.mjs reads it as SKIP, mode-table.json can carry it); the drill goes on.
		{
			const bool bNeedMate = G.Step == 5 || G.Step == 6, bNeedOpp = G.Step == 7 || G.Step == 8;
			if ((bNeedMate && !Sk.IsValidIndex(G.Mate)) || (bNeedOpp && !Sk.IsValidIndex(G.Opp)))
			{
				const TCHAR* Why = bNeedMate ? TEXT("no_mate") : TEXT("no_opp");
				const TCHAR* V1 = G.Step == 5 ? TEXT("pass") : G.Step == 6 ? TEXT("onetimer") : G.Step == 7 ? TEXT("check") : TEXT("checked");
				const TCHAR* V2 = G.Step == 5 ? (Sim->PassPowerOn() ? TEXT("pass_power") : TEXT("saucer")) : G.Step == 7 ? TEXT("crosscheck") : nullptr;   // lane A207: under game.pass.power the held row is the power
				ACC_LOG(TEXT("step=%d verb=%s ok=0 frame=%lld skipped=%s (this cut has no %s for the step: not run, not a fail)"), G.Step, V1, F, Why, bNeedMate ? TEXT("mate") : TEXT("opponent"));
				if (V2) ACC_LOG(TEXT("step=%d verb=%s ok=0 frame=%lld skipped=%s (this cut has no %s for the step: not run, not a fail)"), G.Step, V2, F, Why, bNeedMate ? TEXT("mate") : TEXT("opponent"));
				if (G.Step == 7) ACC_LOG(TEXT("step=7 verb=crosscheck_held ok=0 frame=%lld skipped=%s (this cut has no opponent for the step: not run, not a fail)"), F, Why);   // lane A191
				G.K = SubsOf(G.Step);
				Next();
				return;
			}
		}
		const double Dir = G.Dir, Gx = G.Gx;
		FVector2D HPos(-Dir * 5.0, 0.0);
		switch (G.Step)
		{
		case 1: HPos = FVector2D(-Dir * 22.0, 0.0); break;
		case 4: HPos = FVector2D(Gx - Dir * 10.0, 0.0); break;
		case 5: HPos = FVector2D(Gx - Dir * 16.0, -4.0); G.bMateLive = true; Sim->SetSkater(G.Mate, FVector2D(Gx - Dir * 16.0, 4.0), FVector2D::ZeroVector, FaceYaw); break;
		case 6: HPos = FVector2D(Gx - Dir * 10.0, -2.0); G.bMateLive = true; Sim->SetSkater(G.Mate, FVector2D(Gx - Dir * 6.0, 6.0), FVector2D::ZeroVector, YawOf(HPos - FVector2D(Gx - Dir * 6.0, 6.0))); break;
		case 7: HPos = FVector2D(-Dir * 3.5, 0.0); G.bOppLive = true; Sim->SetSkater(G.Opp, FVector2D(Dir * 2.0, 0.0), FVector2D::ZeroVector, FaceYaw + PI); Sim->ClearReleaseGrace(G.Opp); break;   // the check row felled it: a new scene (the release grace would make the cross-check a bump)
		case 8: HPos = FVector2D(0.0, 0.0); G.bOppLive = true; Sim->SetSkater(G.Opp, FVector2D(Dir * 7.0, 0.0), FVector2D::ZeroVector, FaceYaw + PI); break;
		default: break;
		}
		// Lane A207 (track P1, game.pass.power): step 5's held row is the full pass: the human 28 m from the mate, both on the line y = -4 towards the net, the
		// mate facing him; nobody in the lane (the parked skaters sit in the human's own corner). A full hold must reach him at full power and be caught.
		if (G.Step == 5 && G.K == 3 && Sim->PassPowerOn() && Sk.IsValidIndex(G.Mate))
		{
			HPos = FVector2D(Gx - Dir * 40.0, -4.0);
			Sim->SetSkater(G.Mate, FVector2D(Gx - Dir * 12.0, -4.0), FVector2D::ZeroVector, FaceYaw + PI);
		}
		// Lane A191: the held cross-check (step 7 k=3) comes from 9 m at 5 m/s with the button held all the way: the speed brought is the hit.
		if (G.Step == 7 && G.K == 3) HPos = FVector2D(-Dir * 9.0, 0.0);
		Sim->SetSkater(Me, HPos, G.Step == 7 ? Fwd * (G.K == 3 ? 5.0 : 4.0) : FVector2D::ZeroVector, FaceYaw);
		ACC_LOG(TEXT("stage step=%d k=%d frame=%lld human=(%.1f,%.1f)"), G.Step, G.K, F, HPos.X, HPos.Y);
		if (G.Step == 7 && G.K == 3 && !JAM_INPUT_HAS_CROSS_HOLD)
		{
			// Lane A191: the row waits for A184's FJamInput::bCrossHold (InputMapping.h, JAM_INPUT_HAS_CROSS_HOLD): not run, not a fail.
			ACC_LOG(TEXT("step=7 verb=crosscheck_held ok=0 frame=%lld skipped=no_hold (FJamInput::bCrossHold is not on this build: JAM_INPUT_HAS_CROSS_HOLD 0)"), F);
			Next();
			return;
		}
		if (G.Step == 7)
		{
			// the opponent carries; the human comes in at 4 m/s with W held (camera-relative forward = the skater's facing after the camera's reset)
			G.bPinPuck = false;
			const FJamSkaterState& O = Sk[G.Opp];
			Sim->SetPuck(O.Pos + O.Forward() * (G.K == 2 && Sim->GetConfig().Contact.bMomentum ? -0.45 : 0.45), FVector2D::ZeroVector);   // lane A92: behind it in the cross-check sub-step
			// Lane A92: the cross-check sub-step glides in at its 4 m/s (W for a blink), so the hand's latency of one to six frames still presses inside cross_reach_m.
			ACC_LOG(TEXT("act step=7 k=%d act=keys keys=W ms=%d frame=%lld"), G.K, G.K == 2 && Sim->GetConfig().Contact.bMomentum ? 30 : 1500, F);
			// Lane A191: k=3: the cross-check button goes down now and stays down (LMB without the puck; the driver holds it for ms) while W carries him in.
			if (G.K == 3) ACC_LOG(TEXT("act step=7 k=3 act=lmb_hold ms=2200 frame=%lld"), F);
			G.ActFrame = F;
			G.Ph = EPh::Measure;
		}
		return;
	}
	if (G.Ph == EPh::Settle)
	{
		if (InPh < 40) return;
		// The camera has settled on the staged human: the cursor and the hand.
		const FVector2D Pos = H.Pos;
		switch (G.Step)
		{
		case 1:
			if (G.K == 3) Hand(Me);
			G.Ph = EPh::Hand; G.PhFrame = F; return;
		case 2: case 10:
		{
			if (G.Step == 10 && G.K == 1)
			{
				ACC_LOG(TEXT("act step=10 k=1 act=pause_resume frame=%lld"), F);
				G.ActFrame = F; G.Ph = EPh::Measure; G.PhFrame = F; return;
			}
			static const double Rel2[4] = { 0.0, 90.0, 180.0, -90.0 };
			static const double Rel10[2] = { 90.0, -90.0 };
			const double Rel = G.Step == 2 ? Rel2[(G.K - 1) % 4] : Rel10[(G.K - 1) % 2];
			G.Target = OnScreenTarget(Pos, Unit(FaceYaw + FMath::DegreesToRadians(Rel)), 2.5, 1.0);
			CursorLine(G.Target, TEXT("aim"));
			G.ActFrame = F; G.Ph = EPh::Measure; G.PhFrame = F; return;
		}
		case 3:
			Hand(Me);
			CursorLine(Pos + Fwd * 3.0, TEXT("ahead"));
			G.Ph = EPh::Hand; G.PhFrame = F; return;
		case 4:
			CursorLine(Pos + Fwd * 6.0, TEXT("net"));
			Hand(Me);
			G.Ph = EPh::Hand; G.PhFrame = F; return;
		case 5:
			if (G.K == 1)
			{
				ACC_LOG(TEXT("act step=5 k=1 act=probe_pass frame=%lld"), F);   // no puck yet: the driver finds which key raises JAMCTRL btn=pass
				G.ActFrame = F; G.Ph = EPh::Measure; G.PhFrame = F; return;
			}
			if (!Sk.IsValidIndex(G.Mate)) { G.K = SubsOf(G.Step); Next(); return; }   // A179: staged only with a mate (see the Stage guard); never Sk[-1]
			CursorLine(OnScreenTarget(Pos, (Sk[G.Mate].Pos - Pos).GetSafeNormal(), 6.0, 2.0), TEXT("mate"));
			Hand(Me);
			G.Ph = EPh::Hand; G.PhFrame = F; return;
		case 6:
			CursorLine(Pos + (FVector2D(G.Gx, 0.0) - Pos).GetSafeNormal() * 6.0, TEXT("net"));
			Hand(G.Mate);
			G.MateRecv0 = H.PassesReceived; G.Received0 = H.PassesReceived;
			G.Ph = EPh::Hand; G.PhFrame = F; return;
		case 8:
			CursorLine(Pos + Fwd * 3.0, TEXT("ahead"));
			Hand(Me);
			G.Ph = EPh::Hand; G.PhFrame = F; return;
		case 9:
			CursorLine(Pos + Fwd * 3.0, TEXT("ahead"));
			Hand(Me);
			G.Ph = EPh::Hand; G.PhFrame = F; return;
		default: G.Ph = EPh::Hand; G.PhFrame = F; return;
		}
	}
	if (G.Ph == EPh::Hand)
	{
		if (InPh < 45) return;   // the carry assist seats the handed puck on the blade the cursor holds
		const double PuckDist = FVector2D::Distance(H.Pos, P.Pos);
		G.PuckDistAtAct = PuckDist;
		G.ActFrame = F;
		G.Ph = EPh::Measure; G.PhFrame = F;
		G.Sweeps0 = H.Sweeps;
		G.MateRecv0 = Sk.IsValidIndex(G.Mate) ? Sk[G.Mate].PassesReceived : 0;
		const int32 C = Sim->CarrierIndex();
		const double Reach = Sim->GetEffectiveStat(EJamStat::ShotReach, Me);
		switch (G.Step)
		{
		case 1:
			ACC_LOG(TEXT("act step=1 k=%d act=keys keys=%s ms=2000 carrier=%d frame=%lld"), G.K, (G.K == 1 || G.K == 4) ? TEXT("W") : TEXT("W+SHIFT"), C, F); break;   // lane A93: k=4 (brake) skates on W, then the drill asks for S
		case 3:
		{
			const FVector2D L = H.Pos + Unit(FaceYaw + FMath::DegreesToRadians(75.0)) * 3.0, R = H.Pos + Unit(FaceYaw - FMath::DegreesToRadians(75.0)) * 3.0;
			FVector2D SL(-1, -1), SR(-1, -1); Screen(L, SL); Screen(R, SR);
			ACC_LOG(TEXT("act step=3 k=1 act=sweep lx=%d ly=%d rx=%d ry=%d n=3 ms=400 carrier=%d puck_dist=%.2f frame=%lld"), FMath::RoundToInt32(SL.X), FMath::RoundToInt32(SL.Y), FMath::RoundToInt32(SR.X), FMath::RoundToInt32(SR.Y), C, PuckDist, F);
			break;
		}
		case 4:
		{
			static const int32 Hold[3] = { 0, 8, 30 };
			ACC_LOG(TEXT("act step=4 k=%d act=lmb frames=%d carrier=%d puck_dist=%.2f reach=%.2f frame=%lld"), G.K, Hold[(G.K - 1) % 3], C, PuckDist, Reach, F);
			break;
		}
		case 5:
			// Lane A207: under game.pass.power the held row holds the full power plus 6 frames (36); before, the saucer's hold plus 6 (19).
			ACC_LOG(TEXT("act step=5 k=%d act=pass frames=%d carrier=%d puck_dist=%.2f reach=%.2f frame=%lld"), G.K, G.K == 2 ? 0 : (Sim->PassPowerOn() ? Cfg.Pass.PowerFullFrames + 6 : JamControls::SaucerHoldFrames(Cfg.Pass.SaucerHalfFrames) + 6), C, PuckDist, Reach, F); break;
		case 6:
			ACC_LOG(TEXT("wait step=6 k=%d mate_carrier=%d frame=%lld"), G.K, C == G.Mate ? 1 : 0, F); G.ActFrame = -1; break;   // the act line comes with the mate's pass
		case 8:
			ACC_LOG(TEXT("act step=8 k=1 act=none carrier=%d puck_dist=%.2f frame=%lld"), C, PuckDist, F); break;
		case 9:
		{
			if (G.K == 1) { ACC_LOG(TEXT("act step=9 k=1 act=wheel carrier=%d frame=%lld"), C, F); }
			else if (G.K == 2) { ACC_LOG(TEXT("act step=9 k=2 act=key key=E carrier=%d frame=%lld"), C, F); }
			else
			{
				const FVector2D L = H.Pos + Unit(FaceYaw + FMath::DegreesToRadians(75.0)) * 3.0, R = H.Pos + Unit(FaceYaw - FMath::DegreesToRadians(75.0)) * 3.0;
				FVector2D SL(-1, -1), SR(-1, -1); Screen(L, SL); Screen(R, SR);
				ACC_LOG(TEXT("act step=9 k=3 act=sweep lx=%d ly=%d rx=%d ry=%d n=1 ms=120 carrier=%d frame=%lld"), FMath::RoundToInt32(SL.X), FMath::RoundToInt32(SL.Y), FMath::RoundToInt32(SR.X), FMath::RoundToInt32(SR.Y), C, F);
			}
			break;
		}
		default: break;
		}
		return;
	}
	// EPh::Measure
	const int32 C = Sim->CarrierIndex();
	auto Events = [&](FName Name, int32 Who) { for (const FJamSimEvent& E : Sim->GetFrameEvents()) if (E.Name == Name && (Who < 0 || E.Who == Who)) return true; return false; };
	switch (G.Step)
	{
	case 1:
	{
		if (G.MoveFrame < 0 && G.MoveIn > 0.3) G.MoveFrame = F;
		if (G.K == 4)
		{
			// Lane A93 (R-0045 law 3): brake. W for 2 s, then S: still inside 40 frames of the S input arriving against the velocity, then moving backward.
			// The driver's key-ups are queued (Hold does not block), so the S line goes out only once W was released.
			if (G.MoveFrame >= 0 && !G.bSAct) G.MaxSpeed = FMath::Max(G.MaxSpeed, H.Vel.Size());
			if (G.MoveFrame >= 0 && !G.bSAct && G.MoveIn < 0.1)
			{
				G.bSAct = true;
				G.Fwd0 = H.Vel.GetSafeNormal();
				ACC_LOG(TEXT("act step=1 k=4 act=keys keys=S ms=1500 speed=%.2f frame=%lld"), H.Vel.Size(), F);
			}
			if (G.bSAct && G.RevFrame < 0 && G.MoveIn > 0.3 && FVector2D::DotProduct(G.MoveVec, H.Vel) < 0.0) { G.RevFrame = F; G.RevSpeed = H.Vel.Size(); }
			if (G.RevFrame >= 0 && G.StopFrame < 0 && H.Vel.Size() < 0.3) G.StopFrame = F;
			if (G.StopFrame >= 0) G.BackSpeed = FMath::Max(G.BackSpeed, -FVector2D::DotProduct(H.Vel, G.Fwd0));
			if (H.bReverseBraking) ++G.BrakeFrames;
			const bool bEndBrake = (G.RevFrame >= 0 && F - G.RevFrame > 90) || InPh > 420;
			if (!bEndBrake) return;
			const int64 StopFrames = (G.RevFrame >= 0 && G.StopFrame >= 0) ? G.StopFrame - G.RevFrame : -1;
			Verdict(TEXT("brake"), G.RevFrame >= 0 && G.StopFrame >= 0 && StopFrames <= 40 && G.BackSpeed > 1.0,
				FString::Printf(TEXT("max_speed=%.2f rev_speed=%.2f stop_frames=%lld back_speed=%.2f brake_frames=%d s_after=%lld"), G.MaxSpeed, G.RevSpeed, StopFrames, G.BackSpeed, G.BrakeFrames, G.RevFrame >= 0 ? G.RevFrame - G.ActFrame : -1));
			Next();
			return;
		}
		if (G.MoveFrame >= 0 && F - G.MoveFrame <= 120)
		{
			G.MaxSpeed = FMath::Max(G.MaxSpeed, H.Vel.Size());
			if (H.bSprinting) { ++G.SprintFrames; if (C == Me) ++G.SprintCarryFrames; }
			if (C == Me) ++G.CarryFrames;
		}
		const bool bEnd = (G.MoveFrame >= 0 && F - G.MoveFrame > 120) || InPh > 300;
		if (!bEnd) return;
		const FString Facts = FString::Printf(TEXT("max_speed=%.2f sprint_frames=%d sprint_carry_frames=%d carry_frames=%d move_after=%lld"), G.MaxSpeed, G.SprintFrames, G.SprintCarryFrames, G.CarryFrames, G.MoveFrame >= 0 ? G.MoveFrame - G.ActFrame : -1);
		if (G.K == 1) { G.Speed1 = G.MaxSpeed; Verdict(TEXT("skate"), G.MaxSpeed > 6.0, Facts); }
		else if (G.K == 2) { G.Speed2 = G.MaxSpeed; Verdict(TEXT("sprint"), G.SprintFrames > 0 && G.MaxSpeed > G.Speed1 + 0.2, Facts + FString::Printf(TEXT(" walk_speed=%.2f"), G.Speed1)); }
		else
		{
			// Lane A93 (R-0045 law 4, replacing R-0041 point 6): the carrier may sprint; its sprint raises its speed over the carrier's walk (the free walk of
			// k=1 x skater.carry_speed_mult) and stays under a free skater's sprint (k=2).
			// Lane A185 (R-0064 point 2): k=1's free walk has skater.free_speed_mult on it; the carrier's has not.
			const double CarryWalk = G.Speed1 * Cfg.Skater.CarrySpeedMult / FMath::Max(1.0, Cfg.Skater.FreeSpeedMult);
			Verdict(TEXT("sprint_carry"), G.CarryFrames >= 60 && G.SprintCarryFrames > 0 && G.MaxSpeed > CarryWalk + 0.2 && G.MaxSpeed < G.Speed2 - 0.2, Facts + FString::Printf(TEXT(" carry_walk=%.2f sprint_speed=%.2f"), CarryWalk, G.Speed2));
		}
		Next();
		return;
	}
	case 2: case 10:
	{
		if (G.Step == 10 && G.K == 1 && !G.bPaused)
		{
			// the driver pauses and resumes with Esc; the game mode does not tick while paused, so the pause shows as a wall-clock gap between two sim frames
			if (WallGap > 0.7) { G.bPaused = true; G.GapS = WallGap; ACC_LOG(TEXT("resumed step=10 gap_s=%.2f frame=%lld"), WallGap, F); G.PhFrame = F; return; }
			if (InPh > 900) { Verdict(TEXT("resume_aim"), false, FString::Printf(TEXT("pause_seen=0 waited_frames=%lld"), InPh)); G.K = SubsOf(10); Next(); }
			return;
		}
		if (G.Step == 10 && G.K == 1 && G.bPaused && G.Target.IsZero())
		{
			if (InPh < 30) return;
			G.Target = OnScreenTarget(H.Pos, Unit(FaceYaw + FMath::DegreesToRadians(90.0)), 2.5, 1.0);
			CursorLine(G.Target, TEXT("aim"));
			G.ActFrame = F; G.PhFrame = F; G.AimFacts = FString::Printf(TEXT("pause_gap_s=%.2f"), G.GapS);
			return;
		}
		FVector2D Ice;
		const bool bCursor = CursorIce(Ice);
		const double Err = AngDeg(H.BladePos - H.Pos, G.Target - H.Pos);
		if (G.ArriveFrame < 0 && bCursor && FVector2D::Distance(Ice, G.Target) < 0.6) G.ArriveFrame = F;
		if (G.ArriveFrame >= 0 && G.FollowFrame < 0 && Err <= 15.0) G.FollowFrame = F;
		const bool bEnd = (G.FollowFrame >= 0) || (G.ArriveFrame >= 0 && F - G.ArriveFrame > 60) || InPh > 240;
		if (!bEnd) return;
		const int64 Lat = G.FollowFrame >= 0 ? G.FollowFrame - G.ArriveFrame : -1;
		const bool bOk = G.ArriveFrame >= 0 && G.FollowFrame >= 0 && Lat <= 12;
		G.AimOk += bOk ? 1 : 0;
		G.AimFacts += FString::Printf(TEXT(" t%d=(arrive=%lld lat=%lld err=%.1f)"), G.K, G.ArriveFrame >= 0 ? G.ArriveFrame - G.ActFrame : -1, Lat, Err);
		if (G.K == SubsOf(G.Step)) Verdict(G.Step == 2 ? TEXT("aim") : TEXT("resume_aim"), G.AimOk == SubsOf(G.Step), FString::Printf(TEXT("followed=%d/%d "), G.AimOk, SubsOf(G.Step)) + G.AimFacts);
		if (G.Step == 10 && G.K == 1) { G.K = 2; G.Ph = EPh::Settle; G.PhFrame = F - 40; G.ArriveFrame = G.FollowFrame = -1; return; }   // the second target straight away (no re-stage)
		Next();
		return;
	}
	case 3:
	{
		if (!G.bLost && (P.LastTouch != Me || C != Me)) { G.bLost = true; G.LostFrame = F; }
		if (InPh < 300) return;
		Verdict(TEXT("carry"), !G.bLost, FString::Printf(TEXT("lost=%d lost_after=%lld sweeps=%d puck_dist=%.2f start_puck_dist=%.2f"), G.bLost ? 1 : 0, G.bLost ? G.LostFrame - G.ActFrame : -1, H.Sweeps - G.Sweeps0, FVector2D::Distance(H.Pos, P.Pos), G.PuckDistAtAct));
		Next();
		return;
	}
	case 4: case 6:
	{
		G.ChargeMax = FMath::Max(G.ChargeMax, H.ChargeFrames);
		if (G.Step == 6)
		{
			if (G.CueFrame < 0) { if (InPh > 300) { G.OtFacts += FString::Printf(TEXT(" k%d=(mate_pass=0)"), G.K); if (G.K == SubsOf(6)) Verdict(TEXT("onetimer"), G.OtOk > 0, FString::Printf(TEXT("ok_tries=%d"), G.OtOk) + G.OtFacts); Next(); } return; }
			if (G.RecvFrame < 0 && (H.PassesReceived > G.Received0 || (P.LastTouch == Me && C == Me)))
			{
				G.RecvFrame = F;
				ACC_LOG(TEXT("received step=6 k=%d frame=%lld after_pass=%lld puck_dist=%.2f"), G.K, F, F - G.CueFrame, FVector2D::Distance(H.Pos, P.Pos));
			}
		}
		if (H.bShotThisFrame && !G.bShot) { G.bShot = true; G.ShotFrame = F; G.ShotCharge = H.LastShotCharge; G.bOneTimer = H.bOneTimerThisFrame; }
		if (Events(JamEvents::Goal, -1)) G.bGoal = true;
		const int64 From = G.Step == 6 ? G.CueFrame : G.ActFrame;
		const bool bEnd = G.bGoal || (G.bShot && F - G.ShotFrame > 120) || (!G.bShot && F - From > 150) || Sim->GetPhase() != EJamSimPhase::Playing;
		if (!bEnd) return;
		if (G.Step == 4)
		{
			static const TCHAR* Verb[3] = { TEXT("shoot_tap"), TEXT("shoot_half"), TEXT("shoot_full") };
			const int32 i = (G.K - 1) % 3;
			// Lane A91 (R-0044 point 2): the tap's band is the sim's: a hold of game.shot.tap_frames (4) or fewer, released as charge 0 (flat). A click through the
			// real input is 2 to 4 frames long; the old band (<= 3) failed a 4-frame click that the sim fired as the flat tap the ruling asks for. The half band
			// starts above the tap's.
			const int32 TapMax = FMath::Max(3, Cfg.Shot.TapFrames);
			const bool bTapFlat = Cfg.Shot.TapFrames <= 0 || G.ShotCharge <= 1e-6f;
			const bool bBand = i == 0 ? (G.ChargeMax <= TapMax && bTapFlat) : (i == 1 ? (G.ChargeMax > TapMax && G.ChargeMax <= 12) : G.ChargeMax >= FMath::Min(Cfg.Shot.ChargeFramesMax, 30) - 4);
			Verdict(Verb[i], G.bShot && G.bGoal && bBand, FString::Printf(TEXT("shot=%d goal=%d charge_frames_max=%d charge=%.2f puck_dist_at_act=%.2f reach=%.2f"), G.bShot ? 1 : 0, G.bGoal ? 1 : 0, G.ChargeMax, G.ShotCharge, G.PuckDistAtAct, Sim->GetEffectiveStat(EJamStat::ShotReach, Me)));
		}
		else
		{
			const bool bOk = G.bGoal || (G.bShot && G.bOneTimer);
			G.OtOk += bOk ? 1 : 0;
			G.OtFacts += FString::Printf(TEXT(" k%d=(received=%d recv_after=%lld shot=%d one_timer=%d goal=%d charge_frames_max=%d)"), G.K, G.RecvFrame >= 0 ? 1 : 0, G.RecvFrame >= 0 ? G.RecvFrame - G.CueFrame : -1, G.bShot ? 1 : 0, G.bOneTimer ? 1 : 0, G.bGoal ? 1 : 0, G.ChargeMax);
			if (G.K == SubsOf(6)) Verdict(TEXT("onetimer"), G.OtOk > 0, FString::Printf(TEXT("ok_tries=%d"), G.OtOk) + G.OtFacts);
		}
		Next();
		return;
	}
	case 5:
	{
		if (G.K == 1) { if (InPh >= 150) { G.K = 1; G.Ph = EPh::Gap; G.PhFrame = F; G.K = 2; } return; }   // the probe: no verdict
		if (H.bPassThisFrame && !G.bPassed) { G.bPassed = true; G.EventFrame = F; G.bSaucer = H.bSaucerThisFrame; G.PassV0 = P.Vel.Size(); }
		if (G.bPassed && !G.bRecv && Sk.IsValidIndex(G.Mate) && (Sk[G.Mate].PassesReceived > G.MateRecv0 || C == G.Mate)) { G.bRecv = true; G.RecvFrame = F; }
		const bool bPower = G.K == 3 && Sim->PassPowerOn();   // lane A207
		const bool bEnd = (G.bPassed && (G.bRecv || F - G.EventFrame > (bPower ? 200 : 120))) || (!G.bPassed && InPh > (bPower ? 240 : 180));
		if (!bEnd) return;
		const FString Facts = FString::Printf(TEXT("pass=%d saucer=%d received=%d flight_frames=%lld puck_dist_at_act=%.2f reach=%.2f release_mps=%.1f full_mps=%.1f mate_dist=%.1f"), G.bPassed ? 1 : 0, G.bSaucer ? 1 : 0, G.bRecv ? 1 : 0, G.bRecv ? G.RecvFrame - G.EventFrame : -1, G.PuckDistAtAct, Sim->GetEffectiveStat(EJamStat::ShotReach, Me),
			G.PassV0, Sim->PassPowerOn() ? Sim->PassFullSpeed() : Cfg.Skater.PassSpeed, Sk.IsValidIndex(G.Mate) ? FVector2D::Distance(H.Pos, Sk[G.Mate].Pos) : -1.0);
		if (G.K == 2) Verdict(TEXT("pass"), G.bPassed && G.bRecv, Facts);
		else if (bPower) Verdict(TEXT("pass_power"), G.bPassed && G.bRecv && G.PassV0 >= Sim->PassFullSpeed() - 0.5, Facts);   // lane A207: the full pass, at full power, caught 28 m away
		else Verdict(TEXT("saucer"), G.bPassed && G.bSaucer, Facts);
		Next();
		return;
	}
	case 7:
	{
		const FJamSkaterState& O = Sk[G.Opp];
		const FVector2D ToO = O.Pos - H.Pos;
		const double Dist = ToO.Size();
		const double Gap = Dist - UJamSim::SkaterRadius * 2.0;
		const double Closing = Dist > 1e-6 ? FVector2D::DotProduct(H.Vel - O.Vel, ToO / Dist) : 0.0;
		// Lane A92: by momentum the press must come before the hit (a press after the first touch hits with the closing speed that is left): the RMB cue comes ten
		// frames before contact, inside the human's 12-frame window whatever the hand's latency; the MMB cue comes at cross_reach_m (the shove needs arm's length).
		const bool bCue = !Cfg.Contact.bMomentum ? (Gap <= 0.25 || (Closing > 0.5 && Gap <= Closing * (UJamSim::HumanCheckWindowFrames - 6) * UJamSim::DT))
			: G.K == 1 ? (Gap <= 0.25 || (Closing > 0.5 && Gap <= Closing * (UJamSim::HumanCheckWindowFrames - 2 + Cfg.Contact.CheckWindupFrames) * UJamSim::DT))   // lane A192: the cue comes the wind-up earlier
			: Gap <= Cfg.Contact.CrossReachM * 0.5 + FMath::Max(0.0, Closing) * Cfg.Contact.CrossWindupFrames * UJamSim::DT;   // lane A209: the shove lands the wind-up later, at half of arm's length (4 frames: 0.52 m as before; 8: 0.78 m)
		if (G.K == 3)
		{
			// Lane A191: the held cross-check. The button was pressed at the stage (act=lmb_hold); he skates in with it held: the man goes down by the speed brought
			// (A184's held cross-check: delivered on contact while bCrossHold is high). The verdict: the human's held level reached the sim (JAMCTRL btn=crosshold
			// held=1, from the log), the opponent went down, the closing speed at the first touch.
			if (G.CueFrame < 0 && Gap <= 0.25) { G.CueFrame = F; G.Closing = Closing; G.MaxSpeed = H.Vel.Size(); }
			if (O.ContactOutcome > G.HitOutcome) { G.HitOutcome = O.ContactOutcome; G.HitRatio = O.ContactRatio; }
			// Director, 14 Nov 16:35 (the H33 release, four tries): with the cross-check's wind-up the pressed shove of k=2 lands 4 frames later, so its man was still
			// down (2 frames left) when this stage began: the row read that old knockdown and ended untouched after 10 frames. A knockdown counts only once
			// the man has been seen on his feet in this stage (UpFrame marks it here).
			if (G.UpFrame < 0) { if (!O.IsDown()) G.UpFrame = F; else if (InPh <= 150) return; }
			if (G.DownFrame < 0 && G.UpFrame >= 0 && O.IsDown()) { G.DownFrame = F; G.bKnock = true; G.KnockAt = O.KnockFrames; }
			if (!((G.DownFrame >= 0 && F - G.DownFrame >= 10) || (G.CueFrame >= 0 && F - G.CueFrame > 60) || InPh > 300)) return;
			Verdict(TEXT("crosscheck_held"), G.bKnock && G.CueFrame >= 0,
				FString::Printf(TEXT("touched=%d down=%d knock_frames=%d outcome=%d ratio=%.2f closing=%.2f speed=%.2f"), G.CueFrame >= 0 ? 1 : 0, G.bKnock ? 1 : 0, G.KnockAt, G.HitOutcome, G.HitRatio, G.Closing, G.MaxSpeed));
			Next();
			return;
		}
		if (G.CueFrame < 0 && bCue)
		{
			G.CueFrame = F; G.Closing = Closing; G.MaxSpeed = H.Vel.Size();
			ACC_LOG(TEXT("act step=7 k=%d act=%s gap=%.2f closing=%.2f speed=%.2f frame=%lld"), G.K, G.K == 1 ? TEXT("rmb") : TEXT("mmb"), Gap, Closing, H.Vel.Size(), F);
		}
		if (Events(JamEvents::Check, Me)) G.bCheck = true;
		if (O.StickHitResult != 0) G.StickHit = O.StickHitResult;
		if (O.ContactOutcome > G.HitOutcome) { G.HitOutcome = O.ContactOutcome; G.HitRatio = O.ContactRatio; }   // lane A92
		if (G.DownFrame < 0 && O.IsDown()) { G.DownFrame = F; G.bKnock = true; G.KnockAt = O.KnockFrames; }
		if (G.DownFrame >= 0 && G.UpFrame < 0 && !O.IsDown()) G.UpFrame = F;
		if (G.DownFrame >= 0 && F - G.DownFrame == 5) G.bStrip = C != G.Opp;
		if (G.K == 2 && G.CueFrame >= 0 && F - G.CueFrame == 20) G.bStrip = C != G.Opp;
		const bool bEnd = G.K == 1 ? ((G.UpFrame >= 0) || (G.CueFrame >= 0 && F - G.CueFrame > 60 && G.DownFrame < 0) || InPh > 400) : ((G.CueFrame >= 0 && F - G.CueFrame > 40) || InPh > 300);
		if (!bEnd) return;
		if (G.K == 1)
		{
			const int64 Down = G.UpFrame >= 0 ? G.UpFrame - G.DownFrame : -1;
			// Lane A92 (R-0045 law 1): the check comes with closing speed (W held from 4 m/s at a carrier skating at the human), so by momentum it is a knockdown:
			// a check, down, the puck stripped, and the get-up after the frames the sim gave it (the ratio's, inside the model's range), +-10. Under the neutral
			// block the sim gives skater.knockdown_frames and the rule reads as before.
			const double Scale = Cfg.Skater.KnockdownFrames / 90.0;
			const bool bRange = !Cfg.Contact.bMomentum ? G.KnockAt == Cfg.Skater.KnockdownFrames
				: (G.KnockAt >= FMath::FloorToInt32(Cfg.Contact.DownFramesMin * Scale) - 1 && G.KnockAt <= FMath::CeilToInt32(FMath::Max(Cfg.Contact.DownFramesMin, Cfg.Contact.DownFramesMax) * Scale) + 1);
			Verdict(TEXT("check"), G.bCheck && G.bKnock && G.bStrip && Down >= 0 && FMath::Abs(Down - G.KnockAt) <= 10 && bRange,
				FString::Printf(TEXT("cued=%d check=%d knockdown=%d strip=%d down_frames=%lld knock_frames=%d range=%d..%d outcome=%d ratio=%.2f closing=%.2f speed=%.2f"), G.CueFrame >= 0 ? 1 : 0, G.bCheck ? 1 : 0,
					G.bKnock ? 1 : 0, G.bStrip ? 1 : 0, Down, G.KnockAt, FMath::RoundToInt32(Cfg.Contact.DownFramesMin * Scale), FMath::RoundToInt32(Cfg.Contact.DownFramesMax * Scale), G.HitOutcome, G.HitRatio, G.Closing, G.MaxSpeed));
		}
		// Lane A92: the cross-check comes from the front (the carrier skates at the human, facing it) with closing speed: the stick's thrust on top of it is a
		// stumble at least (the body part of the hit, the sim's ContactOutcome), and the puck is free 20 frames later. Under the neutral block: the lift as before.
		else Verdict(TEXT("crosscheck"), (Cfg.Contact.bMomentum ? G.HitOutcome >= UJamSim::ContactStumble : G.StickHit != 0) && G.bStrip,
			FString::Printf(TEXT("cued=%d stickhit=%d puck_free=%d outcome=%d ratio=%.2f knockdown=%d knock_frames=%d closing=%.2f"), G.CueFrame >= 0 ? 1 : 0, G.StickHit, G.bStrip ? 1 : 0, G.HitOutcome, G.HitRatio, G.bKnock ? 1 : 0, G.KnockAt, G.Closing));
		Next();
		return;
	}
	case 8:
	{
		if (H.ContactOutcome > G.HitOutcome) { G.HitOutcome = H.ContactOutcome; G.HitRatio = H.ContactRatio; }   // lane A92
		if (G.DownFrame < 0 && H.IsDown()) { G.DownFrame = F; G.KnockAt = H.KnockFrames; }
		if (G.DownFrame >= 0 && G.UpFrame < 0)
		{
			if (H.IsDown())
			{
				if (PC && PC->GetViewTarget() != HumanPawn) G.bCamOff = true;
				if (PC && PC->PlayerCameraManager && HumanPawn) G.CamMaxM = FMath::Max(G.CamMaxM, FVector::Dist2D(PC->PlayerCameraManager->GetCameraLocation(), HumanPawn->GetActorLocation()) / 100.0);
			}
			else if (H.CanAct()) G.UpFrame = F;
		}
		const bool bEnd = G.UpFrame >= 0 || InPh > 600 || (G.DownFrame >= 0 && F - G.DownFrame > FMath::Max(Cfg.Skater.KnockdownFrames, G.KnockAt) * 2);
		if (!bEnd) return;
		const int64 Down = G.UpFrame >= 0 ? G.UpFrame - G.DownFrame : -1;
		// Lane A92 (R-0045 law 1): the opponent sprints 7 m at the standing human and checks at contact: a full-speed hit beats the standing brace, so the human
		// goes down, for the frames the sim gave it by the ratio (+-15; skater.knockdown_frames under the neutral block).
		Verdict(TEXT("checked"), G.DownFrame >= 0 && !G.bCamOff && G.CamMaxM < 25.0 && Down >= 0 && FMath::Abs(Down - G.KnockAt) <= 15,
			FString::Printf(TEXT("down=%d view_target_human=%d cam_max_m=%.1f down_frames=%lld knock_frames=%d outcome=%d ratio=%.2f opp_cue=%lld closing=%.2f"), G.DownFrame >= 0 ? 1 : 0, G.bCamOff ? 0 : 1, G.CamMaxM, Down, G.KnockAt,
				G.HitOutcome, G.HitRatio, G.CueFrame >= 0 ? G.CueFrame - G.ActFrame : -1, G.Closing));
		Next();
		return;
	}
	case 9:
	{
		const bool bFire = G.K == 1 ? H.bDekeThisFrame : (G.K == 2 ? H.bSpinThisFrame : H.bSweepThisFrame);
		if (bFire && !G.bFired) { G.bFired = true; G.EventFrame = F; }
		if (G.bFired && !G.bLost && F - G.EventFrame <= 30 && (P.LastTouch != Me || C != Me)) { G.bLost = true; G.LostFrame = F; }
		const bool bEnd = (G.bFired && F - G.EventFrame > 30) || (!G.bFired && InPh > 120);
		if (!bEnd) return;
		static const TCHAR* Verb[3] = { TEXT("deke"), TEXT("spin"), TEXT("sweep") };
		Verdict(Verb[(G.K - 1) % 3], G.bFired && !G.bLost, FString::Printf(TEXT("fired=%d fired_after=%lld kept_30=%d lost_after=%lld puck_dist_at_act=%.2f"), G.bFired ? 1 : 0, G.bFired ? G.EventFrame - G.ActFrame : -1, G.bFired && !G.bLost ? 1 : 0, G.bLost ? G.LostFrame - G.EventFrame : -1, G.PuckDistAtAct));
		Next();
		return;
	}
	default: Next(); return;
	}
}
