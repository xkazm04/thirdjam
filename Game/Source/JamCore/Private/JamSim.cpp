#include "JamSim.h"
#include "JamEventBus.h"

DEFINE_LOG_CATEGORY(LogJamSim);

namespace
{
	const FName RolePlayer(TEXT("player"));
	const FName RuleTimer(TEXT("timer"));
	const FName CauseCheck(TEXT("check"));

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
		S.Shots = S.Touches = S.Goals = 0;
		S.DistanceM = 0;
		S.ShotReadyFrame = 0;
	}
	Puck = FJamPuckState();
	FrameEvents.Reset();
	FaceoffReset();
}

void UJamSim::FaceoffReset()
{
	for (FJamSkaterState& S : Skaters)
	{
		S.Pos = S.PrevPos = S.SpawnPos;
		S.Vel = FVector2D::ZeroVector;
		S.Yaw = S.PrevYaw = S.SpawnYaw;
		S.YawRate = 0;
		S.CheckWindow = 0;
		S.KnockFrames = 0;
		S.bTouchingPuck = false;
		S.bHasPuck = false;
	}
	// The drop: up to 0.5 m off centre from the seeded stream, so every seed plays a different match (lane A2 asked).
	const FVector2D Dir = Rng.UnitVector2D();
	Puck.Pos = Puck.PrevPos = Dir * Rng.Range(0.0, FaceoffDropM);
	Puck.Vel = FVector2D::ZeroVector;
	Puck.LastTouch = -1;
	Phase = EJamSimPhase::Playing;
	PauseFrames = 0;
	Emit(JamEvents::Faceoff, -1, FVector2D::ZeroVector, 0.f);
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
	return Team == 0 ? GoalLineX() : -GoalLineX();
}

bool UJamSim::InGoal(const FVector2D& P, int32 Side) const
{
	const double X = P.X * Side;
	const double GL = GoalLineX();
	return X > GL + Config.Puck.RadiusM && X < GL + NetDepth && FMath::Abs(P.Y) < Config.Rink.GoalWidthM * 0.5;
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

	for (FJamSkaterState& S : Skaters)
	{
		S.PrevPos = S.Pos;
		S.PrevYaw = S.Yaw;
		S.bShotThisFrame = false;
		S.bCheckThisFrame = false;
	}
	Puck.PrevPos = Puck.Pos;

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

	// Playing. 1) shots and checks are decided on the positions the player saw, 2) skaters move, 3) the puck moves.
	for (FJamSkaterState& S : Skaters)
	{
		const FJamInput& In = Inputs.IsValidIndex(S.Index) ? Inputs[S.Index] : FJamInput();
		if (S.CheckWindow > 0) --S.CheckWindow;
		if (S.IsDown()) continue;
		if (In.bShoot) TryShoot(S);
		if (In.bCheck)
		{
			S.CheckWindow = CheckWindowFrames;
			S.bCheckThisFrame = true;
		}
	}
	for (FJamSkaterState& S : Skaters)
	{
		const FJamInput& In = Inputs.IsValidIndex(S.Index) ? Inputs[S.Index] : FJamInput();
		StepSkater(S, In);
		CollideSkaterStatic(S);
	}
	CollideSkaters();
	StepPuck();

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
	}

	// Possession flag for the visuals: last toucher with the puck in reach.
	for (FJamSkaterState& S : Skaters)
	{
		S.bHasPuck = (Puck.LastTouch == S.Index) && FVector2D::Distance(S.Pos, Puck.Pos) <= Config.Shot.ReachM && !S.IsDown();
	}
}

void UJamSim::StepSkater(FJamSkaterState& S, const FJamInput& In)
{
	const FJamSkaterConfig& C = Config.Skater;
	const double OldYaw = S.Yaw;
	// Glide friction scales with the ice: glide_drag was measured at ice_friction 0.015 (DEFAULTS.md).
	const double Glide = C.GlideDrag * (Config.Rink.IceFriction / ReferenceIceFriction);

	if (S.IsDown())
	{
		S.Vel *= FMath::Max(0.0, 1.0 - 2.0 * DT);
		--S.KnockFrames;
	}
	else
	{
		// |Move| scales top speed and thrust. Humans clamp to 1; bots may send up to 1.5 (the brain folds ai.speed_scale into it).
		FVector2D Move = In.Move;
		double Mag = Move.Size();
		const double MaxMag = S.bHuman ? 1.0 : 1.5;
		if (Mag > MaxMag) { Move *= MaxMag / Mag; Mag = MaxMag; }
		const double Top = C.Speed * Mag;
		if (Mag > 0.05)
		{
			S.Yaw = RotateToward(S.Yaw, FMath::Atan2(Move.Y, Move.X), C.TurnRate * DT);
		}
		const FVector2D Fwd = S.Forward();
		double Along = Dot(S.Vel, Fwd);
		FVector2D Side = S.Vel - Fwd * Along;
		if (Mag > 0.05 && !In.bBrake && Along < Top)
		{
			Along = FMath::Min(Top, Along + C.Accel * Mag * DT);
		}
		Along = MoveToward(Along, 0.0, (Glide + (In.bBrake ? C.BrakeDecel : 0.0)) * DT);
		Side *= FMath::Max(0.0, 1.0 - (C.LateralGrip + (In.bBrake ? C.BrakeDecel * 0.5 : 0.0)) * DT);
		S.Vel = Fwd * Along + Side;
	}
	const FVector2D Before = S.Pos;
	S.Pos += S.Vel * DT;
	S.DistanceM += FVector2D::Distance(Before, S.Pos);
	S.YawRate = WrapPi(S.Yaw - OldYaw) / DT;
}

void UJamSim::CollideSkaterStatic(FJamSkaterState& S)
{
	const double R = SkaterRadius;
	FVector2D N;
	const double D = BoardDistance(S.Pos, N);
	if (D + R > 0)
	{
		S.Pos -= N * (D + R);
		const double Vn = Dot(S.Vel, N);
		if (Vn > 0) S.Vel -= N * ((1.0 + SkaterBoardBounce) * Vn);
	}
	// Nets as solid boxes behind each goal line.
	const double GL = GoalLineX();
	const double Hw = Config.Rink.GoalWidthM * 0.5;
	for (int32 Side = -1; Side <= 1; Side += 2)
	{
		const double X0 = Side > 0 ? GL : -GL - NetDepth;
		const double X1 = Side > 0 ? GL + NetDepth : -GL;
		if (PushOutOfBox(S.Pos, R, FVector2D(X0, -Hw), FVector2D(X1, Hw), N))
		{
			const double Vn = Dot(S.Vel, N);
			if (Vn < 0) S.Vel -= N * Vn;
		}
	}
	for (const FJamObstacle& O : Config.Rink.Obstacles)
	{
		const FVector2D C(O.X, O.Z);
		if (O.Kind == FName(TEXT("pillar")))
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
				if (A.CheckWindow > 0 && !A.IsDown() && !B.IsDown() && ASpeed > Config.Skater.CheckSpeedMin)
				{
					A.CheckWindow = 0;
					Emit(JamEvents::Check, A.Index, Contact, Strength);
					KnockDown(B.Index, N, Strength, CauseCheck);
				}
				else if (B.CheckWindow > 0 && !B.IsDown() && !A.IsDown() && BSpeed > Config.Skater.CheckSpeedMin)
				{
					B.CheckWindow = 0;
					Emit(JamEvents::Check, B.Index, Contact, Strength);
					KnockDown(A.Index, -N, Strength, CauseCheck);
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
	V.KnockFrames = Config.Skater.KnockdownFrames;
	V.KnockDir = Dir.GetSafeNormal();
	if (V.KnockDir.IsNearlyZero()) V.KnockDir = FVector2D(1, 0);
	Emit(JamEvents::Knockdown, V.Index, V.Pos, Strength, Cause);
	// Drop the puck if the victim had it.
	if (Puck.LastTouch == V.Index && FVector2D::Distance(V.Pos, Puck.Pos) <= Config.Shot.ReachM)
	{
		Puck.Vel = V.Vel * 0.5 + Rng.UnitVector2D() * 1.0;
	}
}

void UJamSim::TryShoot(FJamSkaterState& S)
{
	if (Frame < S.ShotReadyFrame) return;
	const FVector2D ToPuck = Puck.Pos - S.Pos;
	const double Dist = ToPuck.Size();
	if (Dist > Config.Shot.ReachM) return;
	const FVector2D Fwd = S.Forward();
	if (Dist >= 0.05 && Dot(Fwd, ToPuck / Dist) < ShotFrontDot) return;

	FVector2D Aim = Fwd;
	if (Config.Shot.bHasAimCone)
	{
		const FVector2D Target(AttackGoalX(S.Team), 0.0);
		const FVector2D ToGoal = (Target - Puck.Pos).GetSafeNormal();
		const double Cos = FMath::Clamp(Dot(Fwd, ToGoal), -1.0, 1.0);
		if (!ToGoal.IsNearlyZero() && FMath::RadiansToDegrees(FMath::Acos(Cos)) <= Config.Shot.AimConeDeg)
		{
			Aim = ToGoal;
		}
	}
	const double Speed = FMath::Min(Config.Shot.Speed, Config.Puck.MaxSpeed);
	Puck.Vel = Aim * Speed;
	Puck.LastTouch = S.Index;
	S.ShotReadyFrame = Frame + Config.Shot.CooldownFrames;
	S.bShotThisFrame = true;
	++S.Shots;
	Emit(JamEvents::Shot, S.Index, Puck.Pos, static_cast<float>(Speed / Config.Puck.MaxSpeed));
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
	const double R = Config.Puck.RadiusM;
	const double E = Config.Rink.BoardRestitution;
	const double GL = GoalLineX();
	const double Hw = Config.Rink.GoalWidthM * 0.5;
	const double Friction = Config.Rink.IceFriction * Gravity;

	for (FJamSkaterState& S : Skaters) S.bTouchingPuck = false;
	TArray<bool, TInlineAllocator<12>> Touched;
	Touched.Init(false, Skaters.Num());

	for (int32 Sub = 0; Sub < PuckSubsteps; ++Sub)
	{
		Puck.Vel *= FMath::Max(0.0, 1.0 - Config.Puck.Drag * H);
		const double Sp = Puck.Vel.Size();
		if (Sp > 1e-9)
		{
			const double NewSp = FMath::Max(0.0, Sp - Friction * H);
			Puck.Vel *= NewSp / Sp;
		}
		const double Sp2 = Puck.Vel.Size();
		if (Sp2 > Config.Puck.MaxSpeed) Puck.Vel *= Config.Puck.MaxSpeed / Sp2;

		const FVector2D Prev = Puck.Pos;
		Puck.Pos += Puck.Vel * H;

		// Skater bodies push the puck (dribbling, as in the Godot pack; no possession glue).
		for (FJamSkaterState& S : Skaters)
		{
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
					Emit(JamEvents::Board, -1, Puck.Pos + N * R, static_cast<float>(Vn / Config.Puck.MaxSpeed));
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
			if (O.Kind == FName(TEXT("pillar"))) CollidePuckCircle(C, O.Size * 0.5, R, E);
			else CollidePuckBox(C - FVector2D(O.Size * 0.5), C + FVector2D(O.Size * 0.5), R, E);
		}

		// Last guard: whatever pushed it, the puck stays inside the boards.
		if (BoardDistance(Puck.Pos, N) + R > 0) Puck.Pos -= N * (BoardDistance(Puck.Pos, N) + R);

		Puck.PathM += FVector2D::Distance(Prev, Puck.Pos);

		if (Phase == EJamSimPhase::Playing)
		{
			if (InGoal(Puck.Pos, 1)) { ScoreGoal(0); }
			else if (InGoal(Puck.Pos, -1)) { ScoreGoal(1); }
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
	++Score[ScoringTeam];
	const int32 Who = Puck.LastTouch;
	if (Skaters.IsValidIndex(Who) && Skaters[Who].Team == ScoringTeam) ++Skaters[Who].Goals;
	Emit(JamEvents::Goal, Who, Puck.Pos, 1.f);

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
	}
	H.AddV(Puck.Pos, 1000.0);
	H.AddV(Puck.Vel, 1000.0);
	H.AddI(Puck.LastTouch);
	return H.H;
}
