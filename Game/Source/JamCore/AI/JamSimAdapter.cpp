// Lane A2: see JamSimAdapter.h.
#include "JamSimAdapter.h"

#include "JamEventBus.h"
#include "JamSim.h"

namespace JamAI
{
	namespace SimAdapterLocal
	{
		ERole RoleOf(const FName& R)
		{
			static const FName Player(TEXT("player")), Support(TEXT("support")), Back(TEXT("back"));
			if (R == Player) { return ERole::Player; }
			if (R == Support) { return ERole::Support; }
			if (R == Back) { return ERole::Back; }
			return ERole::Chaser;
		}
		FVec2 V(const FVector2D& P) { return FVec2(P.X, P.Y); }
	}
	using namespace SimAdapterLocal;

	FJamMatchConfig ToMatchConfig(const FJamGameConfig& G)
	{
		FJamMatchConfig C;
		C.LengthM = G.Rink.LengthM;
		C.WidthM = G.Rink.WidthM;
		C.CornerRadiusM = G.Rink.CornerRadiusM;
		C.BoardRestitution = G.Rink.BoardRestitution;
		C.IceFriction = G.Rink.IceFriction;
		C.GoalWidthM = G.Rink.GoalWidthM;
		C.Speed = G.Skater.Speed;
		C.Accel = G.Skater.Accel;
		C.TurnRate = G.Skater.TurnRate;
		C.LateralGrip = G.Skater.LateralGrip;
		C.BrakeDecel = G.Skater.BrakeDecel;
		C.PuckRadiusM = G.Puck.RadiusM;
		C.PuckMaxSpeed = G.Puck.MaxSpeed;
		C.PuckDrag = G.Puck.Drag;
		C.ShotReachM = G.Shot.ReachM;
		C.ShotSpeed = G.Shot.Speed;
		C.AimConeDeg = G.Shot.bHasAimCone ? G.Shot.AimConeDeg : 0.0;
		C.ShotCooldownFrames = G.Shot.CooldownFrames;
		C.bTimer = G.Rules.Mode == FName(TEXT("timer"));
		C.ScoreToWin = G.Rules.ScoreToWin;
		C.TimerFrames = G.Rules.TimerFrames;
		C.GoalPauseFrames = G.Rules.GoalPauseFrames;
		C.AI.YieldMarginM = G.Ai.YieldMarginM;
		C.AI.YieldRadiusM = G.Ai.YieldRadiusM;
		C.AI.HoldRadiusM = G.Ai.HoldRadiusM;
		C.AI.SpeedScale = G.Ai.SpeedScale;
		C.AI.RedSkill = G.Ai.RedSkill;
		C.NumSkaters = 0;
		for (int32 T = 0; T < G.Teams.Num() && T < 2; ++T)
		{
			for (const FJamSkaterSpawn& Sp : G.Teams[T].Skaters)
			{
				if (C.NumSkaters < kMaxSkaters)
				{
					FJamMatchSkaterCfg& S = C.Skaters[C.NumSkaters++];
					S.Team = T;
					S.Role = RoleOf(Sp.Role);
					S.Spawn = FVec2(Sp.X, Sp.Z);
				}
			}
		}
		return C;
	}

	FJamSimAdapter::FJamSimAdapter(const FJamGameConfig& InGame)
		: Game(InGame)
	{
		Sim.Reset(NewObject<UJamSim>());
	}

	FJamSimAdapter::~FJamSimAdapter() = default;

	void FJamSimAdapter::Reset(const FJamMatchConfig& C, uint64_t Seed)
	{
		(void)C;   // the sim reads the full cartridge config; C is the runner's view of the same cartridge
		Sim->Init(Game, Seed);
	}

	void FJamSimAdapter::BuildWorld(const UJamSim& S, FJamAIWorld& W)
	{
		const FJamGameConfig& G = S.GetConfig();
		W.Frame = S.GetFrame();
		W.bFrozen = S.GetPhase() != EJamSimPhase::Playing;
		W.PuckPos = V(S.GetPuck().Pos);
		W.PuckVel = V(S.GetPuck().Vel);
		W.PuckLastTouch = S.GetPuck().LastTouch;
		const TArray<FJamSkaterState>& Sk = S.GetSkaters();
		W.NumSkaters = FMath::Min(Sk.Num(), kMaxSkaters);
		W.PlayerIndex = S.GetHumanIndex();
		W.HalfLength = G.Rink.LengthM * 0.5;
		W.HalfWidth = G.Rink.WidthM * 0.5;
		W.CornerRadius = G.Rink.CornerRadiusM;
		W.GoalX = S.GoalLineX();
		W.GoalHalfWidth = G.Rink.GoalWidthM * 0.5;
		W.ShotReach = G.Shot.ReachM;
		W.bDrawOpen = S.IsDrawOpen();
		W.bFaceoffPhase = S.GetPhase() == EJamSimPhase::Faceoff;
		W.FaceoffCountdown = S.GetFaceoffCountdown();
		// R-0017: stick facts and the carrier
		{
			// the same rule as UJamSim::CarrierIndex (private): the last toucher, able to act, with the puck within shot reach
			const int32 Lt = S.GetPuck().LastTouch;
			const TArray<FJamSkaterState>& All = S.GetSkaters();
			W.Carrier = (All.IsValidIndex(Lt) && All[Lt].CanAct() && FVector2D::Distance(All[Lt].Pos, S.GetPuck().Pos) <= G.Shot.ReachM) ? Lt : -1;
		}
		W.SkaterRadius = UJamSim::SkaterRadius;
		W.StickReach = G.Stick.ReachM;
		W.LiftRange = G.Stick.LiftRangeM;
		const int64 NextFrame = S.GetFrame() + 1;   // UJamSim::Step increments the frame before shots are tried
		for (int32 i = 0; i < W.NumSkaters; ++i)
		{
			const FJamSkaterState& K = Sk[i];
			FJamAISkater& O = W.Skaters[i];
			O.Pos = V(K.Pos);
			O.Vel = V(K.Vel);
			O.Facing = V(K.Forward());
			O.Spawn = V(K.SpawnPos);
			O.Team = K.Team;
			O.Role = RoleOf(K.Role);
			O.bHasPuck = K.bHasPuck;
			O.bDown = K.IsDown();
			O.bCentre = K.bCentre;
			O.bCanPoke = K.CanAct() && NextFrame >= K.PokeReadyFrame && K.PokeWindow <= 0;
			O.bCanLift = K.CanAct() && NextFrame >= K.LiftReadyFrame;
			// the same gate as UJamSim::TryShoot
			const FVector2D To = S.GetPuck().Pos - K.Pos;
			const double D = To.Size();
			O.bCanShoot = !W.bFrozen && !K.IsDown() && NextFrame >= K.ShotReadyFrame && D <= G.Shot.ReachM
				&& (D < 0.05 || FVector2D::DotProduct(K.Forward(), To / D) >= UJamSim::ShotFrontDot);
		}
	}

	void FJamSimAdapter::View(FJamAIWorld& W) const
	{
		BuildWorld(*Sim, W);
		W.FramesSinceDrop = (W.bDrawOpen && LastDropFrame >= 0) ? static_cast<int>(W.Frame - LastDropFrame) : -1;
	}

	FJamAIStepEvents FJamSimAdapter::Step(const FJamAIInput* In)
	{
		FJamAIStepEvents Ev;
		const TArray<FJamSkaterState>& Before = Sim->GetSkaters();
		// A touch is a change of the puck's last toucher (UJamSim's own Touches counts every contact start, which is
		// hundreds per match while dribbling).
		const int32 LastBefore = Sim->GetPuck().LastTouch;
		int32 Shots[kMaxSkaters] = {};
		const int32 N = FMath::Min(Before.Num(), kMaxSkaters);
		for (int32 i = 0; i < N; ++i)
		{
			Shots[i] = Before[i].Shots;
		}
		const int32 S0 = Sim->GetScore(0), S1 = Sim->GetScore(1);
		Inputs.SetNum(N);
		for (int32 i = 0; i < N; ++i)
		{
			FJamInput& I = Inputs[i];
			I.Move = FVector2D(In[i].Move.X, In[i].Move.Y);
			I.bShoot = In[i].bShoot;
			I.bCheck = In[i].bCheck;
			I.bBrake = In[i].bBrake;
			I.bPass = In[i].bPass;
			I.Stick = FVector2D(In[i].Stick.X, In[i].Stick.Y);
			I.bPoke = In[i].bPoke;
			I.bLift = In[i].bLift;
		}
		Sim->Step(Inputs);
		const TArray<FJamSkaterState>& After = Sim->GetSkaters();
		for (int32 i = 0; i < N; ++i)
		{
			if (After[i].Shots > Shots[i] && Ev.Shot < 0) { Ev.Shot = i; }
		}
		const int32 LastAfter = Sim->GetPuck().LastTouch;
		if (LastAfter >= 0 && LastAfter < N && LastAfter != LastBefore) { Ev.TouchMask |= 1u << LastAfter; }
		if (Sim->GetScore(0) > S0) { Ev.GoalTeam = 0; }
		else if (Sim->GetScore(1) > S1) { Ev.GoalTeam = 1; }
		for (const FJamSimEvent& E : Sim->GetFrameEvents())
		{
			// R-0013: a faceoff is now a drop followed by won / lost events; count the drop (or a kindless faceoff) once
			if (E.Name == JamEvents::Faceoff && (E.Kind == NAME_None || E.Kind == FName(TEXT("drop"))))
			{
				Ev.bFaceoff = true;
				LastDropFrame = Sim->GetFrame();
			}
		}
		return Ev;
	}

	bool FJamSimAdapter::IsOver() const { return Sim->GetPhase() == EJamSimPhase::Over; }
	int FJamSimAdapter::Winner() const { return Sim->GetWinner(); }
	int FJamSimAdapter::Score(int Team) const { return Sim->GetScore(Team); }
	uint64_t FJamSimAdapter::StateHash() const { return Sim->StateHash(); }

	double FJamSimAdapter::BoardDistance(const FVec2& P) const
	{
		FVector2D N;
		return Sim->BoardDistance(FVector2D(P.X, P.Y), N);
	}
}
