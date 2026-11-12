#include "JamSim.h"
#include "JamEventBus.h"
#include "JamZones.h"

DEFINE_LOG_CATEGORY(LogJamSim);

namespace
{
	const FName RolePlayer(TEXT("player"));
	const FName RuleTimer(TEXT("timer"));
	const FName CauseCheck(TEXT("check"));
	const FName KindClean(TEXT("clean"));
	const FName KindMiss(TEXT("miss"));
	const FName KindDrop(TEXT("drop"));
	const FName KindWon(TEXT("won"));
	const FName KindLost(TEXT("lost"));
	const FName FoulFromBehind(TEXT("foul_from_behind"));
	const FName FoulNoPuck(TEXT("foul_no_puck"));
	const FName KindPillar(TEXT("pillar"));
	const FName KindBumper(TEXT("bumper"));
	const FName KindGoalShield(TEXT("goal_shield"));

	constexpr double Gravity = 9.8;
	constexpr double SkaterBoardBounce = 0.3;
	constexpr double SkaterSkaterBounce = 0.2;
	constexpr double NetBounce = 0.3;
	constexpr double BoardTangentKeep = 0.9;
	constexpr double BoardEventMinSpeed = 3.0;   // m/s normal impact before a "board" event
	constexpr int32 BoardEventGapFrames = 6;
	constexpr double ReferenceIceFriction = 0.015; // the ice friction glide_drag was measured on

	double WrapPi(double A)
	{
		while (A > UE_DOUBLE_PI) A -= 2.0 * UE_DOUBLE_PI;
		while (A < -UE_DOUBLE_PI) A += 2.0 * UE_DOUBLE_PI;
		return A;
	}

	double RotateToward(double From, double To, double MaxStep)
	{
		const double D = WrapPi(To - From);
		if (FMath::Abs(D) <= MaxStep) return WrapPi(To);
		return WrapPi(From + (D > 0 ? MaxStep : -MaxStep));
	}

	double MoveToward(double A, double B, double MaxStep)
	{
		if (FMath::Abs(B - A) <= MaxStep) return B;
		return A + (B > A ? MaxStep : -MaxStep);
	}

	double Dot(const FVector2D& A, const FVector2D& B) { return A.X * B.X + A.Y * B.Y; }

	// Circle (centre P, radius R) against an axis-aligned box; pushes P out, returns the outward normal.
	bool PushOutOfBox(FVector2D& P, double R, const FVector2D& Min, const FVector2D& Max, FVector2D& OutN)
	{
		const FVector2D Q(FMath::Clamp(P.X, Min.X, Max.X), FMath::Clamp(P.Y, Min.Y, Max.Y));
		const FVector2D D = P - Q;
		const double Dist2 = D.SizeSquared();
		if (Dist2 >= R * R) return false;
		if (Dist2 > 1e-12)
		{
			const double Dist = FMath::Sqrt(Dist2);
			OutN = D / Dist;
			P = Q + OutN * R;
			return true;
		}
		// Centre inside the box: leave along the axis of least penetration.
		const double L = P.X - Min.X, Rr = Max.X - P.X, B = P.Y - Min.Y, T = Max.Y - P.Y;
		const double M = FMath::Min(FMath::Min(L, Rr), FMath::Min(B, T));
		if (M == L) { OutN = FVector2D(-1, 0); P.X = Min.X - R; }
		else if (M == Rr) { OutN = FVector2D(1, 0); P.X = Max.X + R; }
		else if (M == B) { OutN = FVector2D(0, -1); P.Y = Min.Y - R; }
		else { OutN = FVector2D(0, 1); P.Y = Max.Y + R; }
		return true;
	}

	struct FHasher
	{
		uint64 H = 1469598103934665603ull;
		void Add(uint64 V)
		{
			for (int32 i = 0; i < 8; ++i) { H ^= (V >> (i * 8)) & 0xFF; H *= 1099511628211ull; }
		}
		void AddI(int64 V) { Add(static_cast<uint64>(V)); }
		void AddQ(double V, double Scale) { AddI(static_cast<int64>(FMath::RoundToDouble(V * Scale))); }
		void AddV(const FVector2D& V, double Scale) { AddQ(V.X, Scale); AddQ(V.Y, Scale); }
	};
}

bool UJamSim::Init(const FJamGameConfig& InConfig, uint64 InSeed)
{
	Config = InConfig;
	Seed = InSeed;
	Skaters.Reset();
	HumanIndex = -1;
	int32 Index = 0;
	for (int32 T = 0; T < Config.Teams.Num() && T < 2; ++T)
	{
		for (const FJamSkaterSpawn& Sp : Config.Teams[T].Skaters)
		{
			FJamSkaterState S;
			S.Index = Index;
			S.Team = T;
			S.Role = Sp.Role;
			S.bHuman = (Sp.Role == RolePlayer);
			if (S.bHuman && HumanIndex < 0) HumanIndex = Index;
			S.SpawnPos = FVector2D(Sp.X, Sp.Z);
			S.SpawnYaw = (T == 0) ? 0.0 : UE_DOUBLE_PI;
			Skaters.Add(S);
			++Index;
		}
	}
	InitRink = Config.Rink;
	InitSpawns.Reset();
	for (const FJamSkaterState& S : Skaters) InitSpawns.Add(S.SpawnPos);
	if (HumanIndex < 0)
	{
		UE_LOG(LogJamSim, Error, TEXT("JAMSIM the config has no skater with role \"player\""));
		HumanIndex = 0;
		return false;
	}
	Restart();
	return true;
}

void UJamSim::Restart()
{
	Rng.Seed(Seed);
	Frame = 0;
	Phase = EJamSimPhase::Playing;
	Score[0] = Score[1] = 0;
	Winner = -1;
	PauseFrames = 0;
	bOvertime = false;
	BoardEventCooldown = 0;
	TimerLeft = (Config.Rules.Mode == RuleTimer) ? Config.Rules.TimerFrames : 0;
	for (FJamSkaterState& S : Skaters)
	{
		S.Shots = S.Touches = S.Goals = S.Passes = 0;
		S.DistanceM = 0;
		S.ShotReadyFrame = 0;
		S.ChargeFrames = 0;
		S.DeflectCooldown = 0;
		S.PokeReadyFrame = S.LiftReadyFrame = 0;
		S.PenaltyFrames = 0;
		S.StumbleFrames = 0;
	}
	PendingFaceoffDot = FVector2D::ZeroVector;
	Puck = FJamPuckState();
	FrameEvents.Reset();
	// C-0005: rule state back to defaults; a shrunk or grown rink goes back to the geometry of the cartridge.
	ScoreMultiplier[0] = ScoreMultiplier[1] = 1;
	Period = 1;
	bNetsSwapped = false;
	PendingRinkScale = 1.0;
	PickupRequests.Reset();
	ModifierRequests.Reset();
	RuleDepth = 0;
	if (Config.Rink.LengthM != InitRink.LengthM || Config.Rink.WidthM != InitRink.WidthM || Config.Rink.CornerRadiusM != InitRink.CornerRadiusM)
	{
		Config.Rink.LengthM = InitRink.LengthM;
		Config.Rink.WidthM = InitRink.WidthM;
		Config.Rink.CornerRadiusM = InitRink.CornerRadiusM;
		Config.Rink.Obstacles = InitRink.Obstacles;
		for (int32 i = 0; i < Skaters.Num() && i < InitSpawns.Num(); ++i) Skaters[i].SpawnPos = InitSpawns[i];
		++RinkRevision;
	}
	ActiveModifiers.Reset();
	ResolveStats();
	FaceoffReset();
	FireRule(EJamRuleTrigger::OnPeriodStart, -1, -1, NAME_None, 1.f);
}

TArray<FString> UJamSim::ApplyLiveConfig(const FJamGameConfig& N)
{
	TArray<FString> Held;
	const FJamRinkConfig& R = Config.Rink;
	if (N.Rink.LengthM != R.LengthM) Held.Add(TEXT("rink.length_m"));
	if (N.Rink.WidthM != R.WidthM) Held.Add(TEXT("rink.width_m"));
	if (N.Rink.CornerRadiusM != R.CornerRadiusM) Held.Add(TEXT("rink.corner_radius_m"));
	if (N.Rink.GoalWidthM != R.GoalWidthM) Held.Add(TEXT("rink.goal_width_m"));
	if (N.Rink.Obstacles.Num() != R.Obstacles.Num()) Held.Add(TEXT("rink.obstacles"));
	if (N.Teams.Num() != Config.Teams.Num()) Held.Add(TEXT("teams"));
	if (N.Rules.Mode != Config.Rules.Mode || N.Rules.ScoreToWin != Config.Rules.ScoreToWin || N.Rules.TimerFrames != Config.Rules.TimerFrames) Held.Add(TEXT("rules"));

	Config.Rink.IceFriction = N.Rink.IceFriction;
	Config.Rink.BoardRestitution = N.Rink.BoardRestitution;
	Config.Skater = N.Skater;
	Config.Stick = N.Stick;
	Config.Puck = N.Puck;
	Config.Shot = N.Shot;
	Config.Rules.GoalPauseFrames = N.Rules.GoalPauseFrames;
	Config.Rules.bPenalties = N.Rules.bPenalties;
	Config.Ai = N.Ai;
	Config.Pickups = N.Pickups;
	Config.Look = N.Look;
	Config.Camera = N.Camera;
	// A shorter cooldown or knockdown takes effect from the next shot or check; running counters are left alone.
	ResolveStats();
	UE_LOG(LogJamSim, Log, TEXT("JAMSIM live config at frame %lld: skater.speed=%g shot.speed=%g ice_friction=%g held back: %s"), Frame, Config.Skater.Speed, Config.Shot.Speed,
		Config.Rink.IceFriction, Held.Num() ? *FString::Join(Held, TEXT(", ")) : TEXT("none"));
	return Held;
}

// ---------------------------------------------------------------------------------------------------------------------
// C-0005 seam A: modifiers.
// ---------------------------------------------------------------------------------------------------------------------

double UJamSim::BaseStat(EJamStat Stat) const
{
	switch (Stat)
	{
	case EJamStat::SkaterSpeed: return Config.Skater.Speed;
	case EJamStat::SkaterAccel: return Config.Skater.Accel;
	case EJamStat::SkaterTurnRate: return Config.Skater.TurnRate;
	case EJamStat::SkaterGrip: return Config.Skater.LateralGrip;
	case EJamStat::SkaterMass: return Config.Skater.Mass;
	case EJamStat::ShotSpeed: return Config.Shot.Speed;
	case EJamStat::ShotAimConeDeg: return Config.Shot.bHasAimCone ? Config.Shot.AimConeDeg : 0.0;
	case EJamStat::ShotReach: return Config.Shot.ReachM;
	case EJamStat::CheckSpeedMin: return Config.Skater.CheckSpeedMin;
	case EJamStat::KnockdownFrames: return Config.Skater.KnockdownFrames;
	case EJamStat::PuckDrag: return Config.Puck.Drag;
	case EJamStat::PuckMaxSpeed: return Config.Puck.MaxSpeed;
	case EJamStat::PuckRadius: return Config.Puck.RadiusM;
	case EJamStat::IceFriction: return Config.Rink.IceFriction;
	case EJamStat::BoardRestitution: return Config.Rink.BoardRestitution;
	case EJamStat::SkaterControlsSign: return 1.0;
	default: return 0.0;
	}
}

void UJamSim::CollectModifiers()
{
	ActiveModifiers.Reset();
	if (ModifierSource) ModifierSource->CollectModifiers(static_cast<int32>(Frame), ActiveModifiers);
}

void UJamSim::ResolveStats()
{
	constexpr int32 N = static_cast<int32>(EJamStat::Count);
	const int32 Rows = Skaters.Num() + 1;
	EffStats.SetNumUninitialized(Rows * N);
	for (int32 Row = 0; Row < Rows; ++Row)
	{
		const bool bGlobalRow = Row == Skaters.Num();
		const int32 SkTeam = bGlobalRow ? -1 : Skaters[Row].Team;
		for (int32 St = 0; St < N; ++St)
		{
			const EJamStat Stat = static_cast<EJamStat>(St);
			const double Base = BaseStat(Stat);
			double Prod = 1.0, Add = 0.0;
			bool bTouched = false;
			for (const FJamActiveModifier& M : ActiveModifiers)
			{
				bool bCovers = false;
				if (bGlobalRow)
				{
					bCovers = M.Target == EJamTarget::Puck || M.Target == EJamTarget::Rink || M.Target == EJamTarget::Everyone;
				}
				else
				{
					const int32 SrcTeam = Skaters.IsValidIndex(M.SourceSkater) ? Skaters[M.SourceSkater].Team : M.Team;
					switch (M.Target)
					{
					case EJamTarget::Self: bCovers = M.SourceSkater == Row; break;
					case EJamTarget::OwnTeam: bCovers = SrcTeam >= 0 && SrcTeam == SkTeam; break;
					case EJamTarget::OtherTeam: bCovers = SrcTeam >= 0 && SrcTeam != SkTeam; break;
					case EJamTarget::Everyone: bCovers = true; break;
					default: break;
					}
				}
				if (!bCovers) continue;
				for (const FJamStatModifier& SM : M.Stats)
				{
					if (SM.Stat != Stat) continue;
					Prod *= static_cast<double>(SM.Multiply);
					Add += static_cast<double>(SM.Add);
					bTouched = true;
				}
			}
			double V = Base;
			if (bTouched)
			{
				double Lo, Hi;
				JamStats::Range(Stat, Lo, Hi);
				V = FMath::Clamp(Base * Prod + Add, Lo, Hi);
			}
			EffStats[Row * N + St] = V;
		}
	}
}

double UJamSim::GetEffectiveStat(EJamStat Stat, int32 Skater) const
{
	constexpr int32 N = static_cast<int32>(EJamStat::Count);
	if (Stat >= EJamStat::Count) return 0.0;
	if (EffStats.Num() != (Skaters.Num() + 1) * N) return BaseStat(Stat);
	const int32 Row = (JamStats::IsGlobal(Stat) || !Skaters.IsValidIndex(Skater)) ? Skaters.Num() : Skater;
	return EffStats[Row * N + static_cast<int32>(Stat)];
}

// ---------------------------------------------------------------------------------------------------------------------
// C-0005 seam B: rule hooks.
// ---------------------------------------------------------------------------------------------------------------------

void UJamSim::FireRule(EJamRuleTrigger Trigger, int32 Who, int32 Team, FName Kind, float Value)
{
	if (!RuleHook || RuleDepth >= 2) return;   // depth: an action may end a period (OnPeriodEnd / OnPeriodStart) but never recurse further
	FJamRuleEventIn In;
	In.Trigger = Trigger;
	In.Frame = static_cast<int32>(Frame);
	In.Who = Who;
	In.Team = Team;
	In.Kind = Kind;
	In.Value = Value;
	TArray<FJamRuleActionOut> Out;
	RuleHook->OnRuleEvent(In, Out);
	++RuleDepth;
	for (const FJamRuleActionOut& A : Out) ApplyRuleAction(In, A);
	--RuleDepth;
}

void UJamSim::ApplyRuleAction(const FJamRuleEventIn& In, const FJamRuleActionOut& A)
{
	switch (A.Action)
	{
	case EJamRuleAction::AddScore:
	{
		const int32 T = (A.Team == 0 || A.Team == 1) ? A.Team : In.Team;
		if (T != 0 && T != 1) return;
		Score[T] = FMath::Max(0, Score[T] + FMath::RoundToInt32(A.Value));
		if (Phase != EJamSimPhase::Over && Config.Rules.Mode != RuleTimer && Config.Rules.ScoreToWin > 0 && Score[T] >= Config.Rules.ScoreToWin) EndMatchNow(T);
		break;
	}
	case EJamRuleAction::SetScoreMultiplier:
	{
		const int32 M = FMath::Clamp(FMath::RoundToInt32(A.Value), 1, 9);
		if (A.Team == 0 || A.Team == 1) ScoreMultiplier[A.Team] = M;
		else ScoreMultiplier[0] = ScoreMultiplier[1] = M;
		break;
	}
	case EJamRuleAction::EndPeriod: EndPeriodNow(); break;
	case EJamRuleAction::EndMatch: EndMatchNow(A.Team); break;
	case EJamRuleAction::ShrinkRink:
	case EJamRuleAction::GrowRink:
	{
		double F = A.Value > 1e-6 ? static_cast<double>(A.Value) : 1.0;
		if (A.Action == EJamRuleAction::ShrinkRink && F > 1.0) F = 1.0 / F;
		if (A.Action == EJamRuleAction::GrowRink && F < 1.0) F = 1.0 / F;
		PendingRinkScale *= F;   // applied at the next faceoff, never mid-play
		break;
	}
	case EJamRuleAction::SetParam: SetParam(A.Param, A.Value); break;
	case EJamRuleAction::SpawnPickup: { FJamRuleRequest R; R.Cause = In; R.Action = A; PickupRequests.Add(R); break; }
	case EJamRuleAction::SwapNets: bNetsSwapped = !bNetsSwapped; break;
	case EJamRuleAction::Announce:
		Emit(JamEvents::Announce, -1, FVector2D::ZeroVector, 0.f, FName(TEXT("rule")));
		FrameEvents.Last().Text = A.Text;
		break;
	case EJamRuleAction::ApplyModifier: { FJamRuleRequest R; R.Cause = In; R.Action = A; ModifierRequests.Add(R); break; }
	default: break;
	}
}

bool UJamSim::SetParam(FName Key, double Value)
{
	FJamGameConfig N = Config;
	if (!UJamCartridge::ApplyValueKey(N, Key.ToString(), Value))
	{
		UE_LOG(LogJamSim, Warning, TEXT("JAMSIM SetParam: \"%s\" names no tunable; ignored"), *Key.ToString());
		return false;
	}
	ApplyLiveConfig(N);
	return true;
}

bool UJamSim::AllowsPickupAt(const FVector2D& PosM) const
{
	return ZoneSource ? ZoneSource->AllowsPickupAt(PosM) : true;
}

void UJamSim::NotifyPickupTaken(int32 Who, FName Kind)
{
	FireRule(EJamRuleTrigger::OnPickupTake, Who, Skaters.IsValidIndex(Who) ? Skaters[Who].Team : -1, Kind, 0.f);
}

void UJamSim::EndMatchNow(int32 Team)
{
	if (Phase == EJamSimPhase::Over) return;
	int32 W;
	if (Score[0] != Score[1]) W = Score[0] > Score[1] ? 0 : 1;
	else if (Team == 0 || Team == 1) W = Team;
	else W = Skaters.IsValidIndex(Puck.LastTouch) ? Skaters[Puck.LastTouch].Team : 0;
	Phase = EJamSimPhase::Over;
	Winner = W;
	PauseFrames = 0;
	Emit(Winner == GetHumanTeam() ? JamEvents::Win : JamEvents::Lose, -1, FVector2D::ZeroVector, 1.f);
}

void UJamSim::EndPeriodNow()
{
	if (Phase == EJamSimPhase::Over) return;
	if (Config.Rules.Mode == RuleTimer && Config.Rules.TimerFrames > 0 && !bOvertime)
	{
		if (Period >= 3)
		{
			TimerLeft = 0;
			if (Score[0] != Score[1]) { EndMatchNow(-1); return; }
			bOvertime = true;   // a tie at the horn: next goal wins, as the clock would have done
		}
		else
		{
			TimerLeft = Config.Rules.TimerFrames - (Period * Config.Rules.TimerFrames) / 3;
		}
		Phase = EJamSimPhase::GoalPause;
		PauseFrames = Config.Rules.GoalPauseFrames;
		UpdatePeriod();
		return;
	}
	// first_to: a period is whatever the rules say it is; the next one starts at the faceoff.
	Phase = EJamSimPhase::GoalPause;
	PauseFrames = Config.Rules.GoalPauseFrames;
	FireRule(EJamRuleTrigger::OnPeriodEnd, -1, -1, NAME_None, static_cast<float>(Period));
	++Period;
	FireRule(EJamRuleTrigger::OnPeriodStart, -1, -1, NAME_None, static_cast<float>(Period));
}

void UJamSim::UpdatePeriod()
{
	if (Config.Rules.Mode != RuleTimer || Config.Rules.TimerFrames <= 0) return;
	const int32 New = FMath::Clamp(1 + (GetTimerElapsed() * 3) / Config.Rules.TimerFrames, 1, 3);
	if (New == Period) return;
	FireRule(EJamRuleTrigger::OnPeriodEnd, -1, -1, NAME_None, static_cast<float>(Period));
	Period = New;
	FireRule(EJamRuleTrigger::OnPeriodStart, -1, -1, NAME_None, static_cast<float>(Period));
}

void UJamSim::ApplyPendingRink()
{
	if (PendingRinkScale == 1.0) return;
	const double L = FMath::Clamp(Config.Rink.LengthM * PendingRinkScale, 40.0, 160.0);
	const double W = FMath::Clamp(Config.Rink.WidthM * PendingRinkScale, 20.0, 80.0);
	const double Sx = L / Config.Rink.LengthM;
	const double Sz = W / Config.Rink.WidthM;
	Config.Rink.LengthM = L;
	Config.Rink.WidthM = W;
	Config.Rink.CornerRadiusM = FMath::Min(Config.Rink.CornerRadiusM, FMath::Min(L, W) * 0.5);
	for (FJamSkaterState& S : Skaters) { S.SpawnPos.X *= Sx; S.SpawnPos.Y *= Sz; }
	for (FJamObstacle& O : Config.Rink.Obstacles) { O.X *= Sx; O.Z *= Sz; }
	PendingRinkScale = 1.0;
	++RinkRevision;
	UE_LOG(LogJamSim, Log, TEXT("JAMSIM rink rebuilt at frame %lld: %g x %g (revision %d)"), Frame, L, W, RinkRevision);
}

// R-0013: a real faceoff. Everyone to the spawns shifted to the dot (penalised skaters stay in the box), the puck pinned at the dot, the two
// centres (per team the skater nearest the dot that is not a back) marked, then FaceoffCountdownFrames of countdown before the drop (StepFaceoff).
void UJamSim::FaceoffReset()
{
	ApplyPendingRink();
	FaceoffDot = PendingFaceoffDot;
	PendingFaceoffDot = FVector2D::ZeroVector;
	// A foul faceoff shifts the formation; keep everyone inside the boards.
	const double Shift = FaceoffDot.IsNearlyZero() ? 1.0 : 0.7;
	for (FJamSkaterState& S : Skaters)
	{
		S.CheckWindow = 0;
		S.KnockFrames = 0;
		S.StumbleFrames = 0;
		S.PokeWindow = 0;
		S.bPokeArmed = false;
		S.bTouchingPuck = false;
		S.bHasPuck = false;
		S.bCentre = false;
		S.DrawFrame = -1;
		S.Vel = FVector2D::ZeroVector;
		S.YawRate = 0;
		if (S.InBox()) { S.Pos = S.PrevPos = BoxPos(S.Team); continue; }
		S.Pos = FaceoffDot + S.SpawnPos * Shift;
		FVector2D N;
		const double D = BoardDistance(S.Pos, N) + SkaterRadius + 0.5;
		if (D > 0) S.Pos -= N * D;
		S.PrevPos = S.Pos;
		// Face the dot (the far net when the dot is the centre, as before).
		const FVector2D ToDot = FaceoffDot - S.Pos;
		S.Yaw = S.PrevYaw = ToDot.SizeSquared() > 1e-6 && !FaceoffDot.IsNearlyZero() ? FMath::Atan2(ToDot.Y, ToDot.X) : S.SpawnYaw;
	}
	for (int32 T = 0; T < 2; ++T)
	{
		int32 Best = -1;
		double BestD = 1e300;
		for (const FJamSkaterState& S : Skaters)
		{
			if (S.Team != T || S.InBox() || S.Role == FName(TEXT("back"))) continue;
			const double D = FVector2D::DistSquared(S.Pos, FaceoffDot);
			if (D < BestD) { BestD = D; Best = S.Index; }
		}
		if (Best < 0) for (const FJamSkaterState& S : Skaters) if (S.Team == T && !S.InBox()) { Best = S.Index; break; }
		if (Best >= 0) Skaters[Best].bCentre = true;
	}
	// The drop lands up to 0.5 m off the dot from the seeded stream, so every seed plays a different match (lane A2 asked).
	const FVector2D Dir = Rng.UnitVector2D();
	Puck.Pos = Puck.PrevPos = FaceoffDot + Dir * Rng.Range(0.0, FaceoffDropM);
	Puck.Vel = FVector2D::ZeroVector;
	Puck.LastTouch = -1;
	Phase = EJamSimPhase::Faceoff;
	FaceoffCountdown = FaceoffCountdownFrames;
	PauseFrames = 0;
	bDrawOpen = false;
	DropFrame = -1;
	BotDrawFrame[0] = BotDrawFrame[1] = -1;
}

void UJamSim::SkipFaceoff()
{
	if (Phase != EJamSimPhase::Faceoff) return;
	FaceoffCountdown = 0;
	Phase = EJamSimPhase::Playing;
	bDrawOpen = false;
	DropFrame = Frame;
	for (FJamSkaterState& S : Skaters) { S.bCentre = false; S.DrawFrame = -1; }
	Emit(JamEvents::Faceoff, -1, FaceoffDot, 0.f, KindDrop);
	FireRule(EJamRuleTrigger::OnFaceoff, -1, -1, NAME_None, 0.f);
}

// The countdown: everyone holds, the blade follows the aim, a centre that presses anything moves early and loses the draw.
// At zero the puck is live (event faceoff/drop) and the draw is open: the first clean stick input after the drop wins (ResolveDraw).
void UJamSim::StepFaceoff(const TArray<FJamInput>& Inputs)
{
	static const FJamInput Hold = [] { FJamInput I; I.bBrake = true; return I; }();
	for (FJamSkaterState& S : Skaters)
	{
		const FJamInput& In = Inputs.IsValidIndex(S.Index) ? Inputs[S.Index] : FJamInput();
		StepSkater(S, Hold);
		StepStick(S, In);
		CollideSkaterStatic(S);
		S.bAtFaceoff = S.bCentre;
		if (S.bCentre && S.DrawFrame == -1 && (In.bPoke || In.bLift || In.bShoot || In.bCheck)) S.DrawFrame = -2;   // early
	}
	CollideSkaters();
	Puck.Vel = FVector2D::ZeroVector;
	if (--FaceoffCountdown > 0) return;
	FaceoffCountdown = 0;
	Phase = EJamSimPhase::Playing;
	DropFrame = Frame;
	bDrawOpen = true;
	// Bot centres draw from the seeded stream: 6..18 frames after the drop, quicker at a higher red skill (lane A3's brain may press bPoke itself instead).
	for (int32 T = 0; T < 2; ++T)
	{
		BotDrawFrame[T] = -1;
		for (const FJamSkaterState& S : Skaters)
		{
			if (!S.bCentre || S.Team != T || S.bHuman) continue;
			const int32 Quick = FMath::Clamp(FMath::RoundToInt32(6.0 + (1.0 - FMath::Clamp(Config.Ai.RedSkill, 0.0, 1.0)) * 6.0), 6, 12);
			BotDrawFrame[T] = Frame + Quick + Rng.NextInt(7);
		}
	}
	Emit(JamEvents::Faceoff, -1, FaceoffDot, 0.f, KindDrop);
	FireRule(EJamRuleTrigger::OnFaceoff, -1, -1, NAME_None, 0.f);
}

void UJamSim::ResolveDraw(int32 WinnerIdx)
{
	bDrawOpen = false;
	int32 Loser = -1;
	for (FJamSkaterState& S : Skaters) if (S.bCentre && S.Index != WinnerIdx) Loser = S.Index;
	if (Skaters.IsValidIndex(WinnerIdx))
	{
		FJamSkaterState& W = Skaters[WinnerIdx];
		// The puck goes back towards the winner's side, a little off to the winner's stick hand (seeded).
		const FVector2D Back = FVector2D(AttackGoalX(W.Team) > 0 ? -1.0 : 1.0, 0.0);
		const FVector2D Side = Rng.UnitVector2D() * 1.5;
		Puck.Vel = Back * 2.5 + Side;   // a win is possession, not a clearance: the puck stays near the dot for the winner to collect
		Puck.LastTouch = WinnerIdx;
		Emit(JamEvents::Faceoff, WinnerIdx, Puck.Pos, 0.f, KindWon);
		if (Loser >= 0) Emit(JamEvents::Faceoff, Loser, Puck.Pos, 0.f, KindLost);
		W.DrawResult = 1;
		if (Loser >= 0) Skaters[Loser].DrawResult = -1;
	}
	for (FJamSkaterState& S : Skaters) { S.bCentre = false; S.DrawFrame = -1; }
}

FVector2D UJamSim::BoxPos(int32 Team) const
{
	return FVector2D(Team == 0 ? -3.0 : 3.0, -(Config.Rink.WidthM * 0.5 - SkaterRadius - 0.3));
}

int32 UJamSim::CarrierIndex() const
{
	const int32 C = Puck.LastTouch;
	if (!Skaters.IsValidIndex(C)) return -1;
	const FJamSkaterState& S = Skaters[C];
	if (!S.CanAct()) return -1;
	// Carried: within shot reach of the body, or on the blade (a long stick holds the puck beyond the body's reach).
	return (FVector2D::Distance(S.Pos, Puck.Pos) <= Eff(EJamStat::ShotReach, C) || FVector2D::Distance(S.BladePos, Puck.Pos) <= DeflectRangeM) ? C : -1;
}

void UJamSim::Emit(FName Name, int32 Who, const FVector2D& Where, float Strength, FName Kind)
{
	FJamSimEvent E;
	E.Name = Name;
	E.Who = Who;
	E.Where = Where;
	E.Strength = FMath::Clamp(Strength, 0.f, 1.f);
	E.Kind = Kind;
	E.Frame = Frame;
	FrameEvents.Add(E);
}

double UJamSim::BoardDistance(const FVector2D& P, FVector2D& OutNormal) const
{
	const double Hx = Config.Rink.LengthM * 0.5;
	const double Hz = Config.Rink.WidthM * 0.5;
	const double Rc = FMath::Clamp(Config.Rink.CornerRadiusM, 0.0, FMath::Min(Hx, Hz));
	const double Sx = P.X >= 0 ? 1.0 : -1.0;
	const double Sz = P.Y >= 0 ? 1.0 : -1.0;
	const double Qx = FMath::Abs(P.X) - (Hx - Rc);
	const double Qz = FMath::Abs(P.Y) - (Hz - Rc);
	if (Qx > 0 && Qz > 0)
	{
		const double Len = FMath::Sqrt(Qx * Qx + Qz * Qz);
		OutNormal = Len > 1e-12 ? FVector2D(Sx * Qx / Len, Sz * Qz / Len) : FVector2D(Sx, 0);
		return Len - Rc;
	}
	if (Qx > Qz) { OutNormal = FVector2D(Sx, 0); }
	else { OutNormal = FVector2D(0, Sz); }
	const double Outside = FMath::Sqrt(FMath::Square(FMath::Max(Qx, 0.0)) + FMath::Square(FMath::Max(Qz, 0.0)));
	const double Inside = FMath::Min(FMath::Max(Qx, Qz), 0.0);
	return Outside + Inside - Rc;
}

double UJamSim::AttackGoalX(int32 Team) const
{
	return ((Team == 0) != bNetsSwapped) ? GoalLineX() : -GoalLineX();
}

bool UJamSim::InGoal(const FVector2D& P, int32 Side) const
{
	const double X = P.X * Side;
	const double GL = GoalLineX();
	return X > GL + Eff(EJamStat::PuckRadius, -1) && X < GL + NetDepth && FMath::Abs(P.Y) < Config.Rink.GoalWidthM * 0.5;
}

void UJamSim::SetPuck(const FVector2D& Pos, const FVector2D& Vel)
{
	Puck.Pos = Puck.PrevPos = Pos;
	Puck.Vel = Vel;
}

void UJamSim::SetSkater(int32 Index, const FVector2D& Pos, const FVector2D& Vel, double Yaw)
{
	if (!Skaters.IsValidIndex(Index)) return;
	FJamSkaterState& S = Skaters[Index];
	S.Pos = S.PrevPos = Pos;
	S.Vel = Vel;
	S.Yaw = S.PrevYaw = Yaw;
}

void UJamSim::Step(const TArray<FJamInput>& Inputs)
{
	FrameEvents.Reset();
	++Frame;
	if (BoardEventCooldown > 0) --BoardEventCooldown;
	// C-0005: effective stats for this frame, before anything integrates.
	CollectModifiers();
	ResolveStats();

	for (FJamSkaterState& S : Skaters)
	{
		S.PrevPos = S.Pos;
		S.PrevYaw = S.Yaw;
		S.bShotThisFrame = false;
		S.bPassThisFrame = false;
		S.bCheckThisFrame = false;
		S.bPokeThisFrame = false;
		if (S.DeflectCooldown > 0) --S.DeflectCooldown;
		S.bLiftThisFrame = false;
		S.PokeResult = 0;
		S.DrawResult = 0;
		S.bFoulThisFrame = false;
		S.bAtFaceoff = false;
	}
	Puck.PrevPos = Puck.Pos;

	if (Phase == EJamSimPhase::Faceoff)
	{
		StepFaceoff(Inputs);
		return;
	}

	if (Phase == EJamSimPhase::Over)
	{
		// Frozen: everybody glides to a stop, nothing scores.
		static const FJamInput Hold = [] { FJamInput I; I.bBrake = true; return I; }();
		for (FJamSkaterState& S : Skaters) { StepSkater(S, Hold); CollideSkaterStatic(S); }
		CollideSkaters();
		return;
	}

	if (Phase == EJamSimPhase::GoalPause)
	{
		static const FJamInput Hold = [] { FJamInput I; I.bBrake = true; return I; }();
		for (FJamSkaterState& S : Skaters) { StepSkater(S, Hold); CollideSkaterStatic(S); }
		CollideSkaters();
		StepPuck();
		if (--PauseFrames <= 0)
		{
			FaceoffReset();
		}
		return;
	}

	// Playing. 0) the rule hook sees the frame, 1) shots and checks are decided on the positions the player saw, 2) skaters move, 3) the puck moves.
	FireRule(EJamRuleTrigger::OnFrame, -1, -1, NAME_None, static_cast<float>(Frame));
	if (Phase != EJamSimPhase::Playing) return;   // an OnFrame action ended the period or the match
	// R-0013 the draw: the first clean stick input after the drop wins; an early mover loses; bot centres press on their seeded frame.
	if (bDrawOpen)
	{
		for (FJamSkaterState& S : Skaters)
		{
			if (!S.bCentre || S.DrawFrame != -1) continue;
			const FJamInput& In = Inputs.IsValidIndex(S.Index) ? Inputs[S.Index] : FJamInput();
			bool bPress = In.bPoke || In.bLift || In.bShoot || In.bCheck;
			if (!bPress && !S.bHuman && BotDrawFrame[S.Team] >= 0 && Frame >= BotDrawFrame[S.Team])
			{
				bPress = true;
				// A human centre that has not moved yet gets a seeded coin against the bot's press (the brain patch, lane A3, will time the bots by
				// difficulty instead); a quick human press before this frame still wins outright, an early one still loses.
				for (FJamSkaterState& H : Skaters)
				{
					if (H.bCentre && H.bHuman && H.DrawFrame == -1 && Rng.NextInt(2) == 0) { H.DrawFrame = Frame; bPress = false; }
				}
			}
			if (bPress) S.DrawFrame = Frame;
		}
		int32 DrawWinner = -1, Early = -1, Pending = 0;
		int64 Best = INT64_MAX;
		for (const FJamSkaterState& S : Skaters)
		{
			if (!S.bCentre) continue;
			if (S.DrawFrame == -2) Early = S.Index;
			else if (S.DrawFrame == -1) ++Pending;
			else if (S.DrawFrame < Best) { Best = S.DrawFrame; DrawWinner = S.Index; }
		}
		if (DrawWinner >= 0 || Pending == 0 || Frame - DropFrame >= DrawWindowFrames)
		{
			if (DrawWinner < 0)
			{
				// Nobody clean: the non-early centre if there is one, else a seeded coin.
				TArray<int32> C;
				for (const FJamSkaterState& S : Skaters) if (S.bCentre && S.Index != Early) C.Add(S.Index);
				if (C.Num() == 0) for (const FJamSkaterState& S : Skaters) if (S.bCentre) C.Add(S.Index);
				if (C.Num()) DrawWinner = C[Rng.NextInt(C.Num())];
			}
			ResolveDraw(DrawWinner);
		}
		else
		{
			Puck.Vel = FVector2D::ZeroVector;   // pinned until a centre wins it
		}
	}

	for (FJamSkaterState& S : Skaters)
	{
		const FJamInput& In = Inputs.IsValidIndex(S.Index) ? Inputs[S.Index] : FJamInput();
		if (S.CheckWindow > 0) --S.CheckWindow;
		if (!S.CanAct()) { S.ChargeFrames = 0; continue; }
		if (In.bShoot) TryShoot(S, 0.f);
		StepCharge(S, In);
		if (In.bPass) TryPass(S);
		if (In.bCheck && Config.Skater.bCheckEnabled)
		{
			S.CheckWindow = CheckWindowFrames;
			S.bCheckThisFrame = true;
		}
		if (In.bPoke) TryPoke(S);
		if (In.bLift) TryLift(S);
	}
	for (FJamSkaterState& S : Skaters)
	{
		const FJamInput& In = Inputs.IsValidIndex(S.Index) ? Inputs[S.Index] : FJamInput();
		StepSkater(S, In);
		StepStick(S, In);
		CollideSkaterStatic(S);
	}
	CollideSkaters();
	StepPuck();
	ResolvePokes();

	if (Phase == EJamSimPhase::Playing && Config.Rules.Mode == RuleTimer && !bOvertime)
	{
		if (TimerLeft > 0) --TimerLeft;
		if (TimerLeft == 0)
		{
			if (Score[0] != Score[1])
			{
				Phase = EJamSimPhase::Over;
				Winner = Score[0] > Score[1] ? 0 : 1;
				Emit(Winner == GetHumanTeam() ? JamEvents::Win : JamEvents::Lose, -1, FVector2D::ZeroVector, 1.f);
			}
			else
			{
				bOvertime = true; // a tie at the horn: next goal wins
			}
		}
		UpdatePeriod();
	}

	// Possession flag for the visuals: last toucher with the puck in reach.
	for (FJamSkaterState& S : Skaters)
	{
		S.bHasPuck = (Puck.LastTouch == S.Index) && CarrierIndex() == S.Index;
	}
}

void UJamSim::StepSkater(FJamSkaterState& S, const FJamInput& In)
{
	const FJamSkaterConfig& C = Config.Skater;
	const double OldYaw = S.Yaw;
	// C-0005: per-skater effective stats (base values when no modifier touches them).
	const double Speed = Eff(EJamStat::SkaterSpeed, S.Index);
	const double Accel = Eff(EJamStat::SkaterAccel, S.Index);
	const double TurnRate = Eff(EJamStat::SkaterTurnRate, S.Index);
	const double Grip = Eff(EJamStat::SkaterGrip, S.Index);
	// Glide friction scales with the ice: glide_drag was measured at ice_friction 0.015 (DEFAULTS.md). R-0019: the zone under the skater scales speed and glide.
	const FJamIceFactors Ice = ZoneSource ? ZoneSource->IceAt(S.Pos) : FJamIceFactors();
	const double Glide = C.GlideDrag * (Eff(EJamStat::IceFriction, -1) / ReferenceIceFriction) * Ice.FrictionMul;

	if (S.InBox())
	{
		// R-0013: sitting out a penalty at the side boards; no input, no contact.
		--S.PenaltyFrames;
		S.Pos = S.PrevPos = BoxPos(S.Team);
		S.Vel = FVector2D::ZeroVector;
		S.YawRate = 0;
		return;
	}
	if (S.IsDown())
	{
		S.Vel *= FMath::Max(0.0, 1.0 - 2.0 * DT);
		--S.KnockFrames;
	}
	else if (S.IsStumbling())
	{
		// R-0013: a mistimed poke: the skater glides without thrust or steering until the stumble ends.
		--S.StumbleFrames;
		const FVector2D Fwd = S.Forward();
		double Along = Dot(S.Vel, Fwd);
		FVector2D Side = S.Vel - Fwd * Along;
		Along = MoveToward(Along, 0.0, (Glide + C.BrakeDecel * 0.5) * DT);
		Side *= FMath::Max(0.0, 1.0 - Grip * DT);
		S.Vel = Fwd * Along + Side;
	}
	else
	{
		// |Move| scales top speed and thrust. Humans clamp to 1; bots may send up to 1.5 (the brain folds ai.speed_scale into it).
		// R-0017: reverse_other multiplies the input by -1 through EJamStat::SkaterControlsSign.
		FVector2D Move = In.Move * Eff(EJamStat::SkaterControlsSign, S.Index);
		double Mag = Move.Size();
		const double MaxMag = S.bHuman ? 1.0 : 1.5;
		if (Mag > MaxMag) { Move *= MaxMag / Mag; Mag = MaxMag; }
		const double Top = Speed * Mag * Ice.SpeedMul;
		if (Mag > 0.05)
		{
			S.Yaw = RotateToward(S.Yaw, FMath::Atan2(Move.Y, Move.X), TurnRate * DT);
		}
		const FVector2D Fwd = S.Forward();
		double Along = Dot(S.Vel, Fwd);
		FVector2D Side = S.Vel - Fwd * Along;
		if (Mag > 0.05 && !In.bBrake && Along < Top)
		{
			Along = FMath::Min(Top, Along + Accel * Mag * DT);
		}
		Along = MoveToward(Along, 0.0, (Glide + (In.bBrake ? C.BrakeDecel : 0.0)) * DT);
		Side *= FMath::Max(0.0, 1.0 - (Grip + (In.bBrake ? C.BrakeDecel * 0.5 : 0.0)) * DT);
		S.Vel = Fwd * Along + Side;
	}
	const FVector2D Before = S.Pos;
	S.Pos += S.Vel * DT;
	S.DistanceM += FVector2D::Distance(Before, S.Pos);
	S.YawRate = WrapPi(S.Yaw - OldYaw) / DT;
}

// R-0013: the blade on the ice from the aim. No aim = ahead at half reach (the carry of the old body-push puck).
void UJamSim::StepStick(FJamSkaterState& S, const FJamInput& In)
{
	const double Mag = In.Stick.Size();
	FVector2D Dir = S.Forward();
	double Frac = 0.5;
	S.bStickAimed = Mag > 0.05;
	if (Mag > 0.05)
	{
		Dir = In.Stick / Mag;
		Frac = FMath::Min(Mag, 1.0);
	}
	S.BladePos = S.Pos + Dir * (SkaterRadius + Frac * Config.Stick.ReachM);
}

void UJamSim::TryPoke(FJamSkaterState& S)
{
	if (Frame < S.PokeReadyFrame || S.PokeWindow > 0) return;
	S.PokeWindow = FMath::Max(1, Config.Stick.PokeWindowFrames);
	S.PokeReadyFrame = Frame + Config.Stick.PokeWindowFrames + 6;
	S.bPokeThisFrame = true;
	// Armed when an opposing carrier is close: a miss then costs the stumble.
	const int32 C = CarrierIndex();
	S.bPokeArmed = C >= 0 && Skaters[C].Team != S.Team && FVector2D::Distance(Skaters[C].Pos, S.Pos) <= SkaterRadius * 2.0 + Config.Stick.ReachM + 1.0;
}

// After the puck moved: an open poke window whose blade reaches the puck takes it; a window that runs out while armed stumbles.
void UJamSim::ResolvePokes()
{
	const double R = Eff(EJamStat::PuckRadius, -1);
	for (FJamSkaterState& S : Skaters)
	{
		if (S.PokeWindow <= 0) continue;
		if (S.CanAct() && FVector2D::Distance(S.BladePos, Puck.Pos) <= R + PokeTakeM)
		{
			const int32 C = CarrierIndex();
			const bool bFromOpponent = C >= 0 && Skaters[C].Team != S.Team;
			// Clean take: the puck comes to the blade with the poker's motion, a touch towards the body.
			Puck.Vel = S.Vel * 0.6 + (S.Pos - Puck.Pos).GetSafeNormal() * 1.5;
			Puck.LastTouch = S.Index;
			++S.Touches;
			S.PokeWindow = 0;
			S.bPokeArmed = false;
			S.PokeResult = 1;
			Emit(JamEvents::Poke, S.Index, S.BladePos, bFromOpponent ? 1.f : 0.f, KindClean);
			continue;
		}
		if (--S.PokeWindow == 0 && S.bPokeArmed)
		{
			S.bPokeArmed = false;
			S.StumbleFrames = Config.Stick.StumbleFrames;
			S.PokeResult = -1;
			Emit(JamEvents::Poke, S.Index, S.BladePos, 0.f, KindMiss);
		}
	}
}

void UJamSim::TryLift(FJamSkaterState& S)
{
	if (Frame < S.LiftReadyFrame) return;
	S.LiftReadyFrame = Frame + 30;
	S.bLiftThisFrame = true;
	const int32 C = CarrierIndex();
	if (C >= 0 && Skaters[C].Team != S.Team && FVector2D::Distance(Skaters[C].Pos, S.Pos) <= SkaterRadius * 2.0 + Config.Stick.LiftRangeM)
	{
		FJamSkaterState& O = Skaters[C];
		Puck.Vel = (S.Pos - O.Pos).GetSafeNormal() * 3.0 + S.Vel * 0.5;
		Puck.LastTouch = S.Index;
		++S.Touches;
		Emit(JamEvents::Lift, S.Index, O.Pos, 1.f, KindClean);
		return;
	}
	Emit(JamEvents::Lift, S.Index, S.BladePos, 0.f, KindMiss);
}

// R-0013 hard foul: the fouler sits BoxFrames in the box; the fouled team gets a free faceoff at the fouler's end, on the side of the contact.
void UJamSim::CallFoul(FJamSkaterState& Fouler, const FJamSkaterState& Victim, const FVector2D& Contact, float Strength, FName Kind)
{
	Emit(JamEvents::Foul, Fouler.Index, Contact, Strength, Kind);
	Fouler.bFoulThisFrame = true;
	Fouler.PenaltyFrames = BoxFrames;
	Fouler.CheckWindow = 0;
	const double OwnEndX = -AttackGoalX(Fouler.Team) * 0.55;
	const double SideZ = (Contact.Y >= 0 ? 1.0 : -1.0) * Config.Rink.WidthM * 0.25;
	PendingFaceoffDot = FVector2D(OwnEndX, SideZ);
	if (Phase == EJamSimPhase::Playing)
	{
		Phase = EJamSimPhase::GoalPause;
		PauseFrames = FoulPauseFrames;
	}
}

void UJamSim::CollideSkaterStatic(FJamSkaterState& S)
{
	if (S.InBox()) return;
	const double R = SkaterRadius;
	FVector2D N;
	const double D = BoardDistance(S.Pos, N);
	if (D + R > 0)
	{
		S.Pos -= N * (D + R);
		const double Vn = Dot(S.Vel, N);
		if (Vn > 0) S.Vel -= N * ((1.0 + SkaterBoardBounce) * Vn);
	}
	// Nets as solid boxes behind each goal line. A skater is pushed out along the shortest axis; the speed it pressed into the hull with becomes a slide
	// along the face towards the nearer open side, so a skater steering straight into a net (A3's trace: pro seed 15, 400 frames at (42.9, -1.6)) goes round it.
	const double GL = GoalLineX();
	const double Hw = Config.Rink.GoalWidthM * 0.5;
	for (int32 Side = -1; Side <= 1; Side += 2)
	{
		const double X0 = Side > 0 ? GL : -GL - NetDepth;
		const double X1 = Side > 0 ? GL + NetDepth : -GL;
		if (PushOutOfBox(S.Pos, R, FVector2D(X0, -Hw), FVector2D(X1, Hw), N))
		{
			const double Vn = Dot(S.Vel, N);
			if (Vn < 0)
			{
				S.Vel -= N * Vn;
				const FVector2D Slide = FMath::Abs(N.X) > 0.5 ? FVector2D(0.0, S.Pos.Y >= 0.0 ? 1.0 : -1.0) : FVector2D(S.Pos.X >= 0.0 ? 1.0 : -1.0, 0.0);
				S.Vel += Slide * (-Vn * 0.8);
			}
		}
	}
	for (const FJamObstacle& O : Config.Rink.Obstacles)
	{
		const FVector2D C(O.X, O.Z);
		if (O.Kind == KindGoalShield)
		{
			const FVector2D Hs(JamZones::GoalShieldDepthM * 0.5, O.Size * 0.5);
			if (PushOutOfBox(S.Pos, R, C - Hs, C + Hs, N))
			{
				const double Vn = Dot(S.Vel, N);
				if (Vn < 0) S.Vel -= N * Vn;
			}
			continue;
		}
		if (O.Kind == KindPillar || O.Kind == KindBumper)
		{
			const FVector2D Dlt = S.Pos - C;
			const double Min = O.Size * 0.5 + R;
			const double Dist = Dlt.Size();
			if (Dist < Min)
			{
				N = Dist > 1e-9 ? Dlt / Dist : FVector2D(1, 0);
				S.Pos = C + N * Min;
				const double Vn = Dot(S.Vel, N);
				if (Vn < 0) S.Vel -= N * Vn;
			}
		}
		else
		{
			const FVector2D H(O.Size * 0.5, O.Size * 0.5);
			if (PushOutOfBox(S.Pos, R, C - H, C + H, N))
			{
				const double Vn = Dot(S.Vel, N);
				if (Vn < 0) S.Vel -= N * Vn;
			}
		}
	}
}

void UJamSim::CollideSkaters()
{
	const double R2 = SkaterRadius * 2.0;
	for (int32 i = 0; i < Skaters.Num(); ++i)
	{
		for (int32 j = i + 1; j < Skaters.Num(); ++j)
		{
			FJamSkaterState& A = Skaters[i];
			FJamSkaterState& B = Skaters[j];
			if (A.InBox() || B.InBox()) continue;
			const FVector2D Dlt = B.Pos - A.Pos;
			const double Dist = Dlt.Size();
			if (Dist >= R2) continue;
			const FVector2D N = Dist > 1e-9 ? Dlt / Dist : FVector2D(A.Team == 0 ? 1.0 : -1.0, 0.0);
			const FVector2D Contact = A.Pos + N * (Dist * 0.5);

			// Checks, decided on the velocities before the contact exchange.
			if (Config.Skater.bCheckEnabled && A.Team != B.Team)
			{
				const double ASpeed = Dot(A.Vel, N);
				const double BSpeed = Dot(B.Vel, -N);
				const float Strength = static_cast<float>(FMath::Clamp((ASpeed - Dot(B.Vel, N)) / Config.Skater.Speed, 0.0, 1.0));
				// R-0013: a check on a skater without the puck, or from behind (the contact normal runs along the victim's facing), is a foul when penalties are on.
				auto Landed = [&](FJamSkaterState& Checker, FJamSkaterState& Victim, const FVector2D& Dir)
				{
					Checker.CheckWindow = 0;
					Emit(JamEvents::Check, Checker.Index, Contact, Strength);
					FireRule(EJamRuleTrigger::OnCheck, Checker.Index, Checker.Team, NAME_None, Strength);
					const bool bVictimHasPuck = CarrierIndex() == Victim.Index;
					const bool bFromBehind = Dot(Dir, Victim.Forward()) > 0.5;
					KnockDown(Victim.Index, Dir, Strength, CauseCheck);
					if (Config.Rules.bPenalties && (!bVictimHasPuck || bFromBehind)) CallFoul(Checker, Victim, Contact, Strength, bVictimHasPuck ? FoulFromBehind : FoulNoPuck);
				};
				if (A.CheckWindow > 0 && A.CanAct() && !B.IsDown() && ASpeed > Eff(EJamStat::CheckSpeedMin, A.Index))
				{
					Landed(A, B, N);
				}
				else if (B.CheckWindow > 0 && B.CanAct() && !A.IsDown() && BSpeed > Eff(EJamStat::CheckSpeedMin, B.Index))
				{
					Landed(B, A, -N);
				}
			}

			const double Overlap = R2 - Dist;
			A.Pos -= N * (Overlap * 0.5);
			B.Pos += N * (Overlap * 0.5);
			const double Vn = Dot(B.Vel - A.Vel, N);
			if (Vn < 0)
			{
				const FVector2D J = N * ((1.0 + SkaterSkaterBounce) * Vn * 0.5);
				A.Vel += J;
				B.Vel -= J;
			}
		}
	}
}

void UJamSim::KnockDown(int32 Victim, const FVector2D& Dir, float Strength, FName Cause)
{
	FJamSkaterState& V = Skaters[Victim];
	V.KnockFrames = FMath::RoundToInt32(Eff(EJamStat::KnockdownFrames, Victim));
	V.KnockDir = Dir.GetSafeNormal();
	if (V.KnockDir.IsNearlyZero()) V.KnockDir = FVector2D(1, 0);
	Emit(JamEvents::Knockdown, V.Index, V.Pos, Strength, Cause);
	FireRule(EJamRuleTrigger::OnKnockdown, V.Index, V.Team, Cause, Strength);
	// C-0005: a check that lands on the carrier always strips the puck; it squirts off the victim's line, a little back towards the checker.
	if (Puck.LastTouch == V.Index && FVector2D::Distance(V.Pos, Puck.Pos) <= Eff(EJamStat::ShotReach, Victim))
	{
		Puck.Vel = V.Vel * 0.5 + Rng.UnitVector2D() * 1.0 - V.KnockDir * 1.0;
		Puck.LastTouch = -1;
	}
}

// C-0005 charged shot: the level input charges while the puck is in reach; release fires at speed + charge * bonus.
void UJamSim::StepCharge(FJamSkaterState& S, const FJamInput& In)
{
	const int32 Max = Config.Shot.ChargeFramesMax;
	if (In.bShootHeld)
	{
		if (Max <= 0) { if (S.ChargeFrames == 0) { TryShoot(S, 0.f); S.ChargeFrames = 1; } return; }   // charging off: a hold is one tap
		const bool bInReach = FVector2D::Distance(S.Pos, Puck.Pos) <= Eff(EJamStat::ShotReach, S.Index);
		if (bInReach && Frame >= S.ShotReadyFrame) S.ChargeFrames = FMath::Min(S.ChargeFrames + 1, Max);
		return;
	}
	if (S.ChargeFrames > 0)
	{
		const float Charge = Max > 0 ? static_cast<float>(S.ChargeFrames) / static_cast<float>(Max) : 0.f;
		S.ChargeFrames = 0;
		if (Max > 0) TryShoot(S, Charge);
	}
}

void UJamSim::TryShoot(FJamSkaterState& S, float Charge)
{
	if (Frame < S.ShotReadyFrame) return;
	const FVector2D ToPuck = Puck.Pos - S.Pos;
	const double Dist = ToPuck.Size();
	if (Dist > Eff(EJamStat::ShotReach, S.Index)) return;
	const FVector2D Fwd = S.Forward();
	if (Dist >= 0.05 && Dot(Fwd, ToPuck / Dist) < ShotFrontDot) return;
	// R-0019: a shot from inside a no_shoot zone is refused (the shooter keeps the puck; a short cooldown so the event does not spam).
	FName ZoneId;
	if (ZoneSource && ZoneSource->BlocksShot(Puck.Pos, ZoneId))
	{
		S.ShotReadyFrame = Frame + 12;
		Emit(JamEvents::ShotBlocked, S.Index, Puck.Pos, 0.f, ZoneId);
		return;
	}

	FVector2D Aim = Fwd;
	const double Cone = Eff(EJamStat::ShotAimConeDeg, S.Index);   // 0 = no aim help (absent aim_cone_deg)
	if (Cone > 0.0)
	{
		const FVector2D Target(AttackGoalX(S.Team), 0.0);
		const FVector2D ToGoal = (Target - Puck.Pos).GetSafeNormal();
		const double Cos = FMath::Clamp(Dot(Fwd, ToGoal), -1.0, 1.0);
		if (!ToGoal.IsNearlyZero() && FMath::RadiansToDegrees(FMath::Acos(Cos)) <= Cone)
		{
			Aim = ToGoal;
		}
	}
	const double MaxSpeed = Eff(EJamStat::PuckMaxSpeed, -1);
	const double Speed = FMath::Min(Eff(EJamStat::ShotSpeed, S.Index) + FMath::Clamp(static_cast<double>(Charge), 0.0, 1.0) * Config.Shot.ChargeSpeedBonus, MaxSpeed);
	Puck.Vel = Aim * Speed;
	Puck.LastTouch = S.Index;
	S.ShotReadyFrame = Frame + Config.Shot.CooldownFrames;
	S.bShotThisFrame = true;
	S.LastShotCharge = FMath::Clamp(Charge, 0.f, 1.f);
	++S.Shots;
	Emit(JamEvents::Shot, S.Index, Puck.Pos, static_cast<float>(Speed / MaxSpeed));
}

// C-0005 pass: the best teammate ahead of the puck with an open lane. The aim is where the receiver will be when the puck gets there
// (flight time solved against its velocity) plus pass_lead_frames of its motion, so it runs onto the puck instead of waiting for it.
int32 UJamSim::PickReceiver(const FJamSkaterState& S, FVector2D& OutAim) const
{
	const FVector2D Attack(AttackGoalX(S.Team) > 0 ? 1.0 : -1.0, 0.0);
	const double Lead = Config.Skater.PassLeadFrames * DT;
	const double Speed = FMath::Max(1.0, FMath::Min(Config.Skater.PassSpeed, Eff(EJamStat::PuckMaxSpeed, -1)));
	int32 Best = -1;
	double BestScore = -1e300;
	for (const FJamSkaterState& T : Skaters)
	{
		if (T.Team != S.Team || T.Index == S.Index || T.IsDown() || T.InBox()) continue;
		// The puck brakes under puck.drag: distance after t seconds is (v0 / k)(1 - e^(-kt)) (v0 t without drag). The first t within 4 s at which
		// that reaches the receiver's position P + V t is the flight time (bisection, deterministic); a receiver the puck cannot reach is no receiver.
		const FVector2D Rel = T.Pos - Puck.Pos;
		const double K = Eff(EJamStat::PuckDrag, -1);
		auto Flown = [&](double t) { return K > 1e-6 ? (Speed / K) * (1.0 - FMath::Exp(-K * t)) : Speed * t; };
		auto Gap = [&](double t) { return Flown(t) - (Rel + T.Vel * t).Size(); };
		double Lo = 0.0, Hi = 4.0;
		if (Gap(Hi) < 0.0)
		{
			// Not reachable at 4 s; try the nearest approach inside the window (the puck may pass the receiver early).
			bool bFound = false;
			for (int32 i = 1; i <= 40 && !bFound; ++i) { const double t = 0.1 * i; if (Gap(t) >= 0.0) { Hi = t; Lo = t - 0.1; bFound = true; } }
			if (!bFound) continue;
		}
		for (int32 i = 0; i < 32; ++i) { const double Mid = 0.5 * (Lo + Hi); if (Gap(Mid) >= 0.0) Hi = Mid; else Lo = Mid; }
		const double Tf = Hi;
		const FVector2D Aim = T.Pos + T.Vel * (Tf + Lead);
		const FVector2D To = Aim - Puck.Pos;
		const double D = To.Size();
		if (D < 2.0 || D > 45.0) continue;
		const FVector2D Dir = To / D;
		// Lane: the nearest opponent to the segment puck -> aim.
		double Lane = 1e300;
		for (const FJamSkaterState& O : Skaters)
		{
			if (O.Team == S.Team || O.InBox()) continue;
			const double T0 = FMath::Clamp(Dot(O.Pos - Puck.Pos, Dir), 0.0, D);
			Lane = FMath::Min(Lane, FVector2D::Distance(Puck.Pos + Dir * T0, O.Pos));
		}
		const double Open = FMath::Min(Lane, 4.0) / 4.0;          // 0 blocked .. 1 four metres clear
		const double Ahead = Dot(Dir, Attack);                    // -1 back .. 1 straight up the ice
		const double PassScore = Open * 2.0 + Ahead - D / 60.0;
		if (Lane < 0.9) continue;                                 // a stick in the lane: not a pass
		if (PassScore > BestScore) { BestScore = PassScore; Best = T.Index; OutAim = Aim; }
	}
	return Best;
}

void UJamSim::TryPass(FJamSkaterState& S)
{
	if (Frame < S.ShotReadyFrame) return;
	if (FVector2D::Distance(S.Pos, Puck.Pos) > Eff(EJamStat::ShotReach, S.Index)) return;
	FVector2D Aim;
	const int32 To = PickReceiver(S, Aim);
	if (To < 0) return;
	const double MaxSpeed = Eff(EJamStat::PuckMaxSpeed, -1);
	const double Speed = FMath::Min(Config.Skater.PassSpeed, MaxSpeed);
	Puck.Vel = (Aim - Puck.Pos).GetSafeNormal() * Speed;
	Puck.LastTouch = S.Index;
	S.ShotReadyFrame = Frame + PassCooldownFrames;
	S.ChargeFrames = 0;
	S.bPassThisFrame = true;
	S.LastShotCharge = 0.f;
	S.LastPassStrength = static_cast<float>(Speed / MaxSpeed);
	++S.Passes;
	Emit(JamEvents::Pass, S.Index, Puck.Pos, static_cast<float>(Speed / MaxSpeed), FName(*FString::Printf(TEXT("to_%d"), To)));
}

bool UJamSim::CollidePuckCircle(const FVector2D& C, double CR, double R, double E)
{
	const FVector2D Dlt = Puck.Pos - C;
	const double Min = CR + R;
	const double Dist = Dlt.Size();
	if (Dist >= Min) return false;
	const FVector2D N = Dist > 1e-9 ? Dlt / Dist : (Puck.PrevPos - C).GetSafeNormal();
	Puck.Pos = C + N * Min;
	const double Vn = Dot(Puck.Vel, N);
	if (Vn < 0) Puck.Vel -= N * ((1.0 + E) * Vn);
	return true;
}

bool UJamSim::CollidePuckBox(const FVector2D& Min, const FVector2D& Max, double R, double E)
{
	FVector2D N;
	if (!PushOutOfBox(Puck.Pos, R, Min, Max, N)) return false;
	const double Vn = Dot(Puck.Vel, N);
	if (Vn < 0) Puck.Vel -= N * ((1.0 + E) * Vn);
	return true;
}

// A thin two-sided wall. The side the puck was on at the start of the substep is kept, so a fast puck cannot cross it.
bool UJamSim::CollidePuckSegment(const FVector2D& A, const FVector2D& B, const FVector2D& PrevPos, double R, double E)
{
	const FVector2D Ab = B - A;
	const double Len2 = Ab.SizeSquared();
	if (Len2 < 1e-12) return false;
	const FVector2D N0 = FVector2D(-Ab.Y, Ab.X) / FMath::Sqrt(Len2);
	const double SPrev = Dot(PrevPos - A, N0);
	const double SNow = Dot(Puck.Pos - A, N0);
	const double T = Dot(Puck.Pos - A, Ab) / Len2;
	if (T > 0.0 && T < 1.0)
	{
		const double Sign = SPrev >= 0 ? 1.0 : -1.0;
		if (SNow * Sign < R)
		{
			const FVector2D N = N0 * Sign;
			Puck.Pos += N * (R - SNow * Sign);
			const double Vn = Dot(Puck.Vel, N);
			if (Vn < 0) Puck.Vel -= N * ((1.0 + E) * Vn);
			return true;
		}
		return false;
	}
	// Past an end: treat the end as a point.
	const FVector2D End = T <= 0.0 ? A : B;
	return CollidePuckCircle(End, 0.0, R, E);
}

void UJamSim::StepPuck()
{
	const double H = DT / PuckSubsteps;
	const double R = Eff(EJamStat::PuckRadius, -1);
	const double E = Eff(EJamStat::BoardRestitution, -1);
	const double GL = GoalLineX();
	const double Hw = Config.Rink.GoalWidthM * 0.5;
	const double Friction = Eff(EJamStat::IceFriction, -1) * Gravity;
	const double Drag = Eff(EJamStat::PuckDrag, -1);
	const double MaxSpeed = Eff(EJamStat::PuckMaxSpeed, -1);

	for (FJamSkaterState& S : Skaters) S.bTouchingPuck = false;
	TArray<bool, TInlineAllocator<12>> Touched;
	Touched.Init(false, Skaters.Num());

	for (int32 Sub = 0; Sub < PuckSubsteps; ++Sub)
	{
		Puck.Vel *= FMath::Max(0.0, 1.0 - Drag * H);
		const double Sp = Puck.Vel.Size();
		if (Sp > 1e-9)
		{
			const double FrictionHere = ZoneSource ? Friction * ZoneSource->IceAt(Puck.Pos).FrictionMul : Friction;   // R-0019 slow / fast ice under the puck
			const double NewSp = FMath::Max(0.0, Sp - FrictionHere * H);
			Puck.Vel *= NewSp / Sp;
		}
		const double Sp2 = Puck.Vel.Size();
		if (Sp2 > MaxSpeed) Puck.Vel *= MaxSpeed / Sp2;

		const FVector2D Prev = Puck.Pos;
		Puck.Pos += Puck.Vel * H;

		// Skater bodies push the puck (dribbling, as in the Godot pack); the carrier's blade pulls it (C-0005 carry assist, R-0013 stick).
		for (FJamSkaterState& S : Skaters)
		{
			if (S.InBox()) continue;
			// C-0005 deflection: an opponent's shot passing within DeflectRangeM of the blade bounces off it at 70 %; a teammate's puck (a pass)
			// is received instead: the blade takes most of its speed and the carry assist settles it.
			if (S.DeflectCooldown == 0 && Puck.LastTouch != S.Index && S.CanAct())
			{
				const FVector2D FromBlade = Puck.Pos - S.BladePos;
				const double Db = FromBlade.Size();
				const double SpB = Puck.Vel.Size();
				if (Db < DeflectRangeM && SpB > DeflectMinSpeed && Db > 1e-6 && Dot(Puck.Vel, FromBlade) < 0.0)
				{
					const bool bOwn = Skaters.IsValidIndex(Puck.LastTouch) && Skaters[Puck.LastTouch].Team == S.Team;
					if (bOwn)
					{
						// Reception: the pass is seated on the blade and moves with the receiver (R-0021: the puck stays on the stick).
						Puck.Pos = S.BladePos;
						Puck.Vel = S.Vel;
					}
					else if (Config.Shot.bDeflect && S.bStickAimed)   // a passive blade is no goalie: aim the stick to block
					{
						const FVector2D Nb = FromBlade / Db;
						Puck.Vel = (Puck.Vel - Nb * (2.0 * Dot(Puck.Vel, Nb))) * DeflectKeep;
					}
					else continue;
					Puck.LastTouch = S.Index;
					S.DeflectCooldown = 10;
					Touched[S.Index] = true;
					continue;
				}
			}
			const FVector2D Dlt = Puck.Pos - S.Pos;
			const double Min = SkaterRadius + R;
			const double Dist = Dlt.Size();
			if (Dist >= Min) continue;
			const FVector2D N = Dist > 1e-9 ? Dlt / Dist : S.Forward();
			Puck.Pos = S.Pos + N * Min;
			const double Vn = Dot(Puck.Vel - S.Vel, N);
			if (Vn < 0) Puck.Vel -= N * Vn;
			Touched[S.Index] = true;
			Puck.LastTouch = S.Index;
		}
		{
			const int32 Ci = CarrierIndex();
			const double Assist = Config.Puck.CarryAssist;
			// R-0021 (operator, feel.00): a carried puck stays on the blade through every turn and leaves only on a shot, a pass, a poke, a lift or a
			// check. Only a puck that rides with the carrier is carried: a shot or pass that just left the stick (fast relative to the body) is not pulled back.
			if (Ci >= 0 && Assist > 0.0 && !bDrawOpen && !Skaters[Ci].bShotThisFrame && !Skaters[Ci].bPassThisFrame && (Puck.Vel - Skaters[Ci].Vel).Size() < 4.0)
			{
				// The puck closes on the blade by a fraction per substep (assist 1 = on the blade within a frame, 0.6 = within three) and moves with the carrier.
				const FJamSkaterState& C = Skaters[Ci];
				const double K = FMath::Min(1.0, 100.0 * Assist * H);
				Puck.Pos += (C.BladePos - Puck.Pos) * K;
				Puck.Vel = C.Vel;
			}
		}

		// Boards.
		FVector2D N;
		const double D = BoardDistance(Puck.Pos, N);
		if (D + R > 0)
		{
			Puck.Pos -= N * (D + R);
			const double Vn = Dot(Puck.Vel, N);
			if (Vn > 0)
			{
				const FVector2D Normal = N * Vn;
				const FVector2D Tangent = Puck.Vel - Normal;
				Puck.Vel = Tangent * BoardTangentKeep - Normal * E;
				if (Vn >= BoardEventMinSpeed && BoardEventCooldown == 0)
				{
					Emit(JamEvents::Board, -1, Puck.Pos + N * R, static_cast<float>(Vn / MaxSpeed));
					BoardEventCooldown = BoardEventGapFrames;
				}
			}
		}

		// Nets: posts, side walls and back wall, both ends.
		for (int32 Side = -1; Side <= 1; Side += 2)
		{
			const double X0 = Side * GL;
			const double X1 = Side * (GL + NetDepth);
			CollidePuckCircle(FVector2D(X0, -Hw), PostRadius, R, NetBounce);
			CollidePuckCircle(FVector2D(X0, Hw), PostRadius, R, NetBounce);
			CollidePuckSegment(FVector2D(X0, -Hw), FVector2D(X1, -Hw), Prev, R, NetBounce);
			CollidePuckSegment(FVector2D(X0, Hw), FVector2D(X1, Hw), Prev, R, NetBounce);
			CollidePuckSegment(FVector2D(X1, -Hw), FVector2D(X1, Hw), Prev, R, NetBounce);
		}

		for (const FJamObstacle& O : Config.Rink.Obstacles)
		{
			const FVector2D C(O.X, O.Z);
			if (O.Kind == KindPillar) CollidePuckCircle(C, O.Size * 0.5, R, E);
			else if (O.Kind == KindBumper) CollidePuckCircle(C, O.Size * 0.5, R, JamZones::BumperRestitution);                       // R-0019: a lively pillar
			else if (O.Kind == KindGoalShield) CollidePuckBox(C - FVector2D(JamZones::GoalShieldDepthM * 0.5, O.Size * 0.5), C + FVector2D(JamZones::GoalShieldDepthM * 0.5, O.Size * 0.5), R, E);   // a wall across the rink
			else CollidePuckBox(C - FVector2D(O.Size * 0.5), C + FVector2D(O.Size * 0.5), R, E);
		}
		// R-0019 bumper zones: pushed out of the zone and thrown back.
		if (ZoneSource) ZoneSource->BumpPuck(Puck.Pos, Puck.Vel, R);

		// Last guard: whatever pushed it, the puck stays inside the boards.
		if (BoardDistance(Puck.Pos, N) + R > 0) Puck.Pos -= N * (BoardDistance(Puck.Pos, N) + R);

		Puck.PathM += FVector2D::Distance(Prev, Puck.Pos);

		if (Phase == EJamSimPhase::Playing)
		{
			if (InGoal(Puck.Pos, 1)) { ScoreGoal(bNetsSwapped ? 1 : 0); }
			else if (InGoal(Puck.Pos, -1)) { ScoreGoal(bNetsSwapped ? 0 : 1); }
		}
	}

	for (FJamSkaterState& S : Skaters)
	{
		if (Touched[S.Index] && !S.bTouchingPuck) ++S.Touches;
		S.bTouchingPuck = Touched[S.Index];
	}
}

void UJamSim::ScoreGoal(int32 ScoringTeam)
{
	Score[ScoringTeam] += ScoreMultiplier[ScoringTeam];
	const int32 Who = Puck.LastTouch;
	if (Skaters.IsValidIndex(Who) && Skaters[Who].Team == ScoringTeam) ++Skaters[Who].Goals;
	Emit(JamEvents::Goal, Who, Puck.Pos, 1.f);
	// C-0005: the rule hook sees the goal (and the score it reached) before the win check, so AddScore / EndMatch land in the same frame.
	FireRule(EJamRuleTrigger::OnGoal, Who, ScoringTeam, NAME_None, static_cast<float>(Score[ScoringTeam]));
	FireRule(EJamRuleTrigger::OnScoreReached, Who, ScoringTeam, NAME_None, static_cast<float>(Score[ScoringTeam]));
	if (Phase == EJamSimPhase::Over) return;

	const bool bFirstToDone = Config.Rules.Mode != RuleTimer && Config.Rules.ScoreToWin > 0 && Score[ScoringTeam] >= Config.Rules.ScoreToWin;
	if (bFirstToDone || bOvertime)
	{
		Phase = EJamSimPhase::Over;
		Winner = ScoringTeam;
		Emit(Winner == GetHumanTeam() ? JamEvents::Win : JamEvents::Lose, -1, FVector2D::ZeroVector, 1.f);
		return;
	}
	Phase = EJamSimPhase::GoalPause;
	PauseFrames = Config.Rules.GoalPauseFrames;
}

uint64 UJamSim::StateHash() const
{
	FHasher H;
	H.AddI(Frame);
	H.AddI(static_cast<int64>(Phase));
	H.AddI(Score[0]);
	H.AddI(Score[1]);
	H.AddI(Winner);
	H.AddI(PauseFrames);
	H.AddI(TimerLeft);
	H.AddI(bOvertime ? 1 : 0);
	H.Add(Rng.GetState());
	for (const FJamSkaterState& S : Skaters)
	{
		H.AddV(S.Pos, 1000.0);
		H.AddV(S.Vel, 1000.0);
		H.AddQ(S.Yaw, 1000.0);
		H.AddI(S.ShotReadyFrame);
		H.AddI(S.CheckWindow);
		H.AddI(S.KnockFrames);
		H.AddI(S.Shots);
		H.AddI(S.Touches);
		H.AddV(S.BladePos, 1000.0);
		H.AddI(S.PokeWindow);
		H.AddI(S.StumbleFrames);
		H.AddI(S.PenaltyFrames);
		H.AddI(S.PokeReadyFrame);
		H.AddI(S.LiftReadyFrame);
		H.AddI(S.bCentre ? 1 : 0);
		H.AddI(S.DrawFrame);
		H.AddI(S.ChargeFrames);
		H.AddI(S.DeflectCooldown);
	}
	H.AddI(FaceoffCountdown);
	H.AddV(FaceoffDot, 1000.0);
	H.AddV(PendingFaceoffDot, 1000.0);
	H.AddI(DropFrame);
	H.AddI(bDrawOpen ? 1 : 0);
	H.AddV(Puck.Pos, 1000.0);
	H.AddV(Puck.Vel, 1000.0);
	H.AddI(Puck.LastTouch);
	// C-0005: active modifier ids, and rule state only when it left its default (so a match without seams hashes as before).
	for (const FJamActiveModifier& M : ActiveModifiers) H.AddI(M.Id);
	if (ScoreMultiplier[0] != 1 || ScoreMultiplier[1] != 1) { H.AddI(ScoreMultiplier[0]); H.AddI(ScoreMultiplier[1]); }
	if (bNetsSwapped) H.AddI(1);
	if (Period != 1) H.AddI(Period);
	if (PendingRinkScale != 1.0) H.AddQ(PendingRinkScale, 1000.0);
	if (RinkRevision != 0) { H.AddI(RinkRevision); H.AddQ(Config.Rink.LengthM, 1000.0); H.AddQ(Config.Rink.WidthM, 1000.0); }
	return H.H;
}
