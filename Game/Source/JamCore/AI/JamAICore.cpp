// Lane A2: brain and player bot (engine-free). See JamAICore.h.
#include "JamAICore.h"

namespace JamAI
{
	namespace AICoreLocal
	{
		inline double Clamp(double V, double Lo, double Hi) { return V < Lo ? Lo : (V > Hi ? Hi : V); }
		inline double Sign(double V) { return V > 0.0 ? 1.0 : (V < 0.0 ? -1.0 : 0.0); }
		inline double Lerp(double A, double B, double T) { return A + (B - A) * T; }
		inline FVec2 LerpV(const FVec2& A, const FVec2& B, double T) { return A + (B - A) * T; }
		inline double DegToRad(double D) { return D * kPi / 180.0; }
		inline double RadToDeg(double R) { return R * 180.0 / kPi; }
	}
	using namespace AICoreLocal;

	double BoardSdf(const FJamAIWorld& W, const FVec2& P, FVec2& OutNormal)
	{
		const double Rc = W.CornerRadius;
		const double Qx = std::fabs(P.X) - (W.HalfLength - Rc);
		const double Qy = std::fabs(P.Y) - (W.HalfWidth - Rc);
		const double Mx = Qx > 0.0 ? Qx : 0.0, My = Qy > 0.0 ? Qy : 0.0;
		const double Inner = Qx > Qy ? Qx : Qy;
		const double D = std::sqrt(Mx * Mx + My * My) + (Inner < 0.0 ? Inner : 0.0) - Rc;
		const double Sx = P.X >= 0.0 ? 1.0 : -1.0, Sy = P.Y >= 0.0 ? 1.0 : -1.0;
		if (Qx > 0.0 && Qy > 0.0) { OutNormal = FVec2(Sx * Qx, Sy * Qy).Normalized(); }
		else if (Qx > Qy) { OutNormal = FVec2(Sx, 0.0); }
		else { OutNormal = FVec2(0.0, Sy); }
		return D;
	}

	bool BoardPeel(const FJamAIWorld& W, const FVec2& Puck, double AttackSign, FVec2& OutDir)
	{
		constexpr double Inward = 1.0;   // 45 degrees off the board
		FVec2 N;
		if (BoardSdf(W, Puck, N) < -1.6)
		{
			return false;
		}
		FVec2 T = N.Perp();
		if (std::fabs(T.X) < 0.2)
		{
			// end board: slide it towards the nearer corner, where the board turns and it can come out
			if (T.Y * Puck.Y < 0.0) { T = -T; }
		}
		else if (T.X * AttackSign < 0.0)
		{
			T = -T;
		}
		OutDir = (T - N * Inward).Normalized();
		return true;
	}

	FVec2 Arrive(const FVec2& From, const FVec2& Target, double SlowRadius)
	{
		const FVec2 D = Target - From;
		const double Dist = D.Length();
		if (Dist < 0.0001)
		{
			return FVec2();
		}
		return D * (1.0 / Dist) * Clamp(Dist / (SlowRadius > 0.001 ? SlowRadius : 0.001), 0.0, 1.0);
	}

	FVec2 Separate(const FVec2& From, const FJamAIWorld& W, int Self, double Radius)
	{
		FVec2 Push;
		for (int i = 0; i < W.NumSkaters; ++i)
		{
			if (i == Self)
			{
				continue;
			}
			const FVec2 D = From - W.Skaters[i].Pos;
			const double Dist = D.Length();
			if (Dist > 0.0001 && Dist < Radius)
			{
				Push += D * (1.0 / Dist) * (1.0 - Dist / Radius);
			}
		}
		return Push.LimitLength(1.0);
	}

	void FJamBrain::Reset()
	{
		Anchor = FVec2(1e6, 0.0);
		AnchorFrame = 0;
		bStalled = false;
		Stalls = 0;
		StallChaser[0] = StallChaser[1] = -1;
		for (int i = 0; i < kMaxSkaters; ++i)
		{
			Used[i] = ERole::Chaser;
		}
	}

	void FJamBrain::Think(const FJamAIWorld& W, FJamAIInput* Out)
	{
		const FVec2 Pp = W.PuckPos;
		if (Pp.DistTo(Anchor) > Params.StallMoveM || W.Frame < AnchorFrame)
		{
			Anchor = Pp;
			AnchorFrame = W.Frame;
		}
		const bool bWas = bStalled;
		bStalled = !W.bFrozen && (W.Frame - AnchorFrame > Params.StallFrames);
		if (bStalled && !bWas)
		{
			++Stalls;
		}
		// Stall: the nearest support of each team becomes a chaser.
		StallChaser[0] = StallChaser[1] = -1;
		if (bStalled)
		{
			double Best[2] = {1e18, 1e18};
			for (int i = 0; i < W.NumSkaters; ++i)
			{
				const FJamAISkater& S = W.Skaters[i];
				if (i == W.PlayerIndex || S.Role != ERole::Support)
				{
					continue;
				}
				const double D = S.Pos.DistTo(Pp);
				if (D < Best[S.Team])
				{
					Best[S.Team] = D;
					StallChaser[S.Team] = i;
				}
			}
		}
		const int PlayerTeam = W.PlayerIndex >= 0 ? W.Skaters[W.PlayerIndex].Team : 0;
		for (int i = 0; i < W.NumSkaters; ++i)
		{
			if (i == W.PlayerIndex)
			{
				continue;
			}
			FJamAIInput In;
			if (W.bFrozen || W.Skaters[i].bDown)
			{
				Out[i] = In;
				continue;
			}
			const double Sc = Params.SpeedScale * (W.Skaters[i].Team != PlayerTeam ? Params.RedSkill : 1.0);
			bool bShoot = false;
			FVec2 Dir = Decide(W, i, bShoot);
			if (Dir.Length() > 1.0)
			{
				Dir = Dir.Normalized();
			}
			In.Move = Dir * Sc;
			In.bShoot = bShoot;
			Out[i] = In;
		}
	}

	FVec2 FJamBrain::Decide(const FJamAIWorld& W, int Id, bool& bShoot)
	{
		const FJamAISkater& S = W.Skaters[Id];
		const FVec2 Pp = W.PuckPos;
		const FVec2 Me = S.Pos;
		const FVec2 OppGoal = W.OppGoal(S.Team);
		const FVec2 OwnGoal = W.OwnGoal(S.Team);
		ERole Role = S.Role == ERole::Player ? ERole::Chaser : S.Role;
		const FVec2 Dirg = (OppGoal - Pp).Normalized();
		double D = Me.DistTo(Pp);
		FVec2 Out;

		bool bHasMateChaser = false;
		FVec2 MateChaser;
		for (int o = 0; o < W.NumSkaters; ++o)
		{
			if (o != Id && W.Skaters[o].Team == S.Team && W.Skaters[o].Role == ERole::Chaser)
			{
				MateChaser = W.Skaters[o].Pos;
				bHasMateChaser = true;
			}
		}

		bool bYield = false;
		if (W.PlayerIndex >= 0)
		{
			const FJamAISkater& P = W.Skaters[W.PlayerIndex];
			const double Db = P.Pos.DistTo(Pp);
			if (S.Team == P.Team)
			{
				// teammates yield when the human is nearer the puck (by the margin) and close to it
				bYield = Db <= D + Params.YieldMarginM && Db < Params.YieldRadiusM;
			}
			else if (Role != ERole::Back)
			{
				// opponents hold back while the human works the puck
				bYield = (Db < Params.HoldRadiusM && D > 3.0) || (Params.bFaceoffCourtesy && W.PuckLastTouch < 0);
			}
		}
		if (bStalled && Role == ERole::Support && StallChaser[S.Team] == Id)
		{
			Role = ERole::Chaser;
		}
		if (bYield && D > 2.0 && !bStalled)
		{
			Role = ERole::Support;
			D = 99.0;
		}
		Used[Id] = Role;

		if (Role == ERole::Chaser || (Role == ERole::Support && D < Params.PounceRadius))
		{
			Out = Attack(W, Id, Me, Pp, Dirg, D, bShoot);
		}
		else if (Role == ERole::Back)
		{
			if (D < 2.5)
			{
				Out = Attack(W, Id, Me, Pp, Dirg, D, bShoot);
			}
			else
			{
				const FVec2 Gl = OwnGoal + FVec2(Sign(-OwnGoal.X) * 3.0, 0.0);
				const FVec2 T = Gl + (Pp - Gl).LimitLength(Params.BackRadius);
				Out = Arrive(Me, T, Params.ArriveSlowRadius);
			}
		}
		else
		{
			const double Lane = S.Spawn.Y;
			FVec2 Between = LerpV(OwnGoal, Pp, 0.6);
			Between.X += Sign(OppGoal.X) * 4.0;
			Between.Y = Lerp(Between.Y, Lane, 0.5);
			if (bHasMateChaser && Between.DistTo(MateChaser) < Params.MinSeparation)
			{
				Between += (Between - MateChaser).Normalized() * Params.MinSeparation;
			}
			Out = Arrive(Me, Between, Params.ArriveSlowRadius);
		}
		Out += Separate(Me, W, Id, Params.SeparateRadius) * 1.2;
		return Out;
	}

	FVec2 FJamBrain::Attack(const FJamAIWorld& W, int Id, const FVec2& Me, const FVec2& Pp, const FVec2& Dirg, double D, bool& bShoot) const
	{
		const FJamAISkater& S = W.Skaters[Id];
		const FVec2 Fwd = S.Facing.Normalized();
		const FVec2 OppGoal = W.OppGoal(S.Team);
		const double Dgoal = Pp.DistTo(OppGoal);
		const double GeoTol = RadToDeg(std::atan2(1.4, Dgoal > 1.0 ? Dgoal : 1.0));
		const double Tol = Params.AimToleranceDeg < GeoTol ? Params.AimToleranceDeg : GeoTol;
		const bool bFaceOk = Fwd.Dot(Dirg) > std::cos(DegToRad(Tol));
		const bool bNearWall = std::fabs(Pp.Y) > W.HalfWidth - 2.0 && Dgoal > 6.0;
		bool bOkShot = bFaceOk && Dgoal < W.ShotReach + Params.ShootExtraM && !bNearWall;
		if (bStalled && Fwd.Dot(Dirg) > 0.0)
		{
			bOkShot = true;
		}
		if (D < W.ShotReach && bOkShot && S.bCanShoot)
		{
			bShoot = true;
		}
		// lane A2: chip a puck off the boards (dribbling cannot: no skater fits between the board and the puck)
		FVec2 Bn;
		if (bBoardPeel && S.bCanShoot && BoardSdf(W, Pp, Bn) > -1.0 && Fwd.Dot(-Bn) > 0.6 && Fwd.X * (OppGoal.X >= 0.0 ? 1.0 : -1.0) > -0.2)
		{
			bShoot = true;
		}
		// Approach direction: towards the goal, or off the boards when the puck lies on them (lane A2 addition: the
		// Godot brain only aimed at the goal, which on an end board puts the approach point outside the rink).
		FVec2 Push = Dirg;
		if (bBoardPeel)
		{
			BoardPeel(W, Pp, OppGoal.X >= 0.0 ? 1.0 : -1.0, Push);
		}
		const FVec2 ToPuck = Pp - Me;
		const double Lateral = ToPuck.X * Push.Y - ToPuck.Y * Push.X;
		if (ToPuck.Dot(Push) > 0.0 && std::fabs(Lateral) < 1.3)
		{
			return (Pp + Push * 3.0 - Me).Normalized();
		}
		const double Lead = Clamp(D / 8.0, 0.0, Params.LeadFrames / 60.0);
		const FVec2 Tgt = Pp + W.PuckVel.LimitLength(25.0) * Lead;
		FVec2 Behind = Tgt - Push * 1.2;
		// slow deterministic sway of the approach line so two pursuers do not lock the puck in a corner
		Behind += Push.Perp() * std::sin(static_cast<double>(W.Frame) * 0.015 + static_cast<double>(Id % 7)) * Params.SwayM;
		FVec2 Want = Behind - Me;
		if (ToPuck.Dot(Push) < 0.0 && D < 4.0)
		{
			FVec2 Side = Push.Perp() * (Lateral <= 0.0 ? 1.0 : -1.0);
			FVec2 SideN;
			if (bBoardPeel && BoardSdf(W, Behind + Side * 2.5, SideN) > -1.0)
			{
				Side = -Side;   // lane A2: swing round on the ice side, never through the boards
			}
			Want = Behind + Side * 2.5 - Me;
		}
		return Want.Normalized();
	}

	FJamAIInput FJamPlayerBot::Think(const FJamAIWorld& W) const
	{
		FJamAIInput In;
		if (W.bFrozen || W.PlayerIndex < 0)
		{
			return In;
		}
		const FJamAISkater& Me = W.Skaters[W.PlayerIndex];
		// Work in the attacking frame: the opponents' goal at +x (rotate 180 deg for team 1; keeps handedness).
		const double Sg = Me.Team == 0 ? 1.0 : -1.0;
		const double Gx = W.GoalX;
		const double Hz = W.HalfWidth;
		const FVec2 Pos = Me.Pos * Sg;
		const FVec2 Face = (Me.Facing * Sg).Normalized();
		const FVec2 Puck = W.PuckPos * Sg;
		FVec2 Others[kMaxSkaters];
		int NumOthers = 0;
		for (int i = 0; i < W.NumSkaters; ++i)
		{
			if (i != W.PlayerIndex)
			{
				Others[NumOthers++] = W.Skaters[i].Pos * Sg;
			}
		}
		const FVec2 Goal(Gx, 0.0);
		FVec2 Aim = Goal;
		const double Dgoal = (Goal - Puck).Length();
		FVec2 Dirg = (Aim - Puck).Normalized();
		// detour around a skater standing in the lane puck -> goal
		for (int k = 0; k < NumOthers; ++k)
		{
			const FVec2& O = Others[k];
			if (O.X < Gx - 6.0 && O.X > Puck.X - 0.5 && O.X < Puck.X + 8.0)
			{
				const FVec2 Rel = O - Puck;
				const double Along = Rel.Dot(Dirg);
				const double Side = Rel.Cross(Dirg);
				if (Along > 0.0 && std::fabs(Side) < 2.2)
				{
					const double Sgn = Puck.Y - O.Y >= 0.0 ? 1.0 : -1.0;
					Aim = FVec2(O.X + 3.0, O.Y + Sgn * 3.5);
					Dirg = (Aim - Puck).Normalized();
				}
			}
		}
		if (std::fabs(Puck.Y) > Hz - 1.6)
		{
			// pinned on a board: peel it off the wall
			Dirg = bPeelToMiddle ? FVec2(1.0, -Sign(Puck.Y) * 0.7).Normalized() : FVec2(1.0, 0.0);
		}
		if (bPeelToMiddle && Puck.X < -Gx + 2.0)
		{
			// behind the own goal line: bring it out to the middle
			Dirg = FVec2(1.0, -Puck.Y * 0.15).Normalized();
		}
		FVec2 Peel;
		if (bBoardPeel && BoardPeel(W, Puck, 1.0, Peel))
		{
			// lane A2: any board, end boards included (the rink is symmetric, so the attacking frame is fine)
			Dirg = Peel;
		}
		const FVec2 ToPuck = Puck - Pos;
		const FVec2 Behind = Puck - Dirg * 1.2;
		FVec2 Want;
		const double Lateral = ToPuck.Cross(Dirg);
		if (ToPuck.Dot(Dirg) > 0.0 && std::fabs(Lateral) < 1.3)
		{
			Want = Puck + Dirg * 3.0 - Pos;   // lined up behind it: push through
		}
		else
		{
			Want = Behind - Pos;
			if (ToPuck.Dot(Dirg) < 0.0 && ToPuck.Length() < 4.0)
			{
				// in front of the puck: swing wide first
				FVec2 Perp = Dirg.Perp() * (Lateral <= 0.0 ? 1.0 : -1.0);
				FVec2 SideN;
				if (bBoardPeel && BoardSdf(W, Behind + Perp * 2.5, SideN) > -1.0)
				{
					Perp = -Perp;   // lane A2: swing round on the ice side, never through the boards
				}
				Want = Behind + Perp * 2.5 - Pos;
			}
		}
		for (int k = 0; k < NumOthers; ++k)
		{
			const FVec2 Dd = Pos - Others[k];
			const double L = Dd.Length();
			if (L < 1.2 && L > 0.01)
			{
				Want += Dd.Normalized() * (1.2 - L) * 3.0;
			}
		}
		const FVec2 Dir = Want.Length() > 0.05 ? Want.Normalized() : FVec2();
		In.Move = Dir * Sg;
		// lane A2: chip a puck off the boards, facing inwards and not backwards
		FVec2 Bn;
		if (bBoardPeel && Me.bCanShoot && BoardSdf(W, Puck, Bn) > -1.0 && Face.Dot(-Bn) > 0.6 && Face.X > -0.2)
		{
			In.bShoot = true;
		}
		// shoot: puck close and in front, goal close enough, facing the goal mouth, clear lane
		const double FaceDot = bPeelToMiddle ? 0.97 : 0.95;
		if (Me.bCanShoot && Dgoal < ShootRangeM && Face.X > 0.3 && Face.Dot((Goal - Puck).Normalized()) > FaceDot)
		{
			const double Zg = Puck.Y + Face.Y / Face.X * (Gx - Puck.X);
			bool bClear = true;
			for (int k = 0; k < NumOthers; ++k)
			{
				const FVec2 Rel2 = Others[k] - Puck;
				if (Rel2.Dot(Face) > 0.0 && std::fabs(Rel2.Cross(Face)) < 1.0 && Others[k].X < Gx - 1.0)
				{
					bClear = false;
				}
			}
			if (std::fabs(Zg) < W.GoalHalfWidth + 0.5 && bClear)
			{
				In.bShoot = true;
			}
		}
		return In;
	}
}
