// Lane A34: see JamPlaytester.h. The controller is engine-free; the match runner and the tests below need UJamSim.
#include "JamPlaytester.h"
#include "JamSimAdapter.h"
#include "JamEventBus.h"
#include "JamSim.h"

#include <cstdio>
#include <cstring>

namespace JamAI
{
	namespace PlaytesterLocal
	{
		inline double Clamp(double V, double Lo, double Hi) { return V < Lo ? Lo : (V > Hi ? Hi : V); }
		inline double Sign(double V) { return V > 0.0 ? 1.0 : (V < 0.0 ? -1.0 : 0.0); }
		inline double DegToRad(double D) { return D * kPi / 180.0; }
		inline double RadToDeg(double R) { return R * 180.0 / kPi; }
		inline FVec2 Rotate(const FVec2& V, double Rad) { const double C = std::cos(Rad), S = std::sin(Rad); return FVec2(V.X * C - V.Y * S, V.X * S + V.Y * C); }
		inline double AngleBetweenDeg(const FVec2& A, const FVec2& B) { return RadToDeg(std::acos(Clamp(A.Dot(B), -1.0, 1.0))); }

		// splitmix64: a fixed, well-mixed 64-bit function of its input (no state).
		inline uint64_t Mix(uint64_t Z)
		{
			Z += 0x9E3779B97F4A7C15ull;
			Z = (Z ^ (Z >> 30)) * 0xBF58476D1CE4E5B9ull;
			Z = (Z ^ (Z >> 27)) * 0x94D049BB133111EBull;
			return Z ^ (Z >> 31);
		}

		// Is any other skater inside the corridor From -> From + Dir * Length, within HalfWidth of its line? (bodies block pucks)
		bool Corridor(const FJamAIWorld& W, int Self, const FVec2& From, const FVec2& Dir, double Length, double HalfWidth, bool bOpponentsOnly, int Team)
		{
			for (int i = 0; i < W.NumSkaters; ++i)
			{
				if (i == Self || W.Skaters[i].bDown) { continue; }
				if (bOpponentsOnly && W.Skaters[i].Team == Team) { continue; }
				const FVec2 Rel = W.Skaters[i].Pos - From;
				const double Along = Rel.Dot(Dir);
				if (Along <= 0.0 || Along >= Length) { continue; }
				if (std::fabs(Rel.Cross(Dir)) < HalfWidth) { return true; }
			}
			return false;
		}
	}
	using namespace PlaytesterLocal;

	FJamPlaytesterProfile PlaytesterProfile(EPlaytester Kind)
	{
		FJamPlaytesterProfile P;
		switch (Kind)
		{
		case EPlaytester::Novice:
			P.ReactionFrames = 12;
			P.AimNoiseDeg = 10.0;
			P.LapseEveryFrames = 600;
			P.LapseFrames = 30;
			P.StickRamp = 0.12;
			P.ShootRangeM = 14.0;        // shoots from close: has not learnt the range
			P.ReleaseToleranceDeg = 8.0;
			P.PatienceFrames = 24;       // lets go early, charged or not
			P.PassAheadM = 8.0;          // passes only when a teammate is obviously ahead
			P.PressureM = 3.0;
			P.CheckFromBehindDot = 0.75; // checks whenever close and fast, often from behind (a foul with rules.penalties)
			P.DrawReactionFrames = 6;    // plus the 12 frames of reaction: 18 after the drop
			P.ChaseLeadS = 0.0;          // chases where the puck is, not where it goes
			P.bGoalside = false;
			break;
		case EPlaytester::Regular:
			P.ReactionFrames = 6;
			P.AimNoiseDeg = 5.0;
			P.StickRamp = 0.2;
			P.ShootRangeM = 20.0;
			P.ReleaseToleranceDeg = 6.0;
			P.PatienceFrames = 40;
			P.PassAheadM = 5.0;
			P.PressureM = 4.0;
			P.CheckFromBehindDot = 0.5;
			P.DrawReactionFrames = 4;
			P.ChaseLeadS = 0.5;
			P.bGoalside = true;
			break;
		default:
			P.ReactionFrames = 3;
			P.AimNoiseDeg = 2.0;
			P.StickRamp = 0.35;
			P.ShootRangeM = 26.0;
			P.ReleaseToleranceDeg = 4.0;
			P.PatienceFrames = 60;       // waits for the full charge when the lane stays open
			P.PassAheadM = 3.0;
			P.PressureM = 5.0;
			P.CheckFromBehindDot = 0.2;  // checks only side-on or head-on
			P.DrawReactionFrames = 2;
			P.ChaseLeadS = 0.8;
			P.bGoalside = true;
			break;
		}
		return P;
	}

	const char* PlaytesterName(EPlaytester Kind)
	{
		switch (Kind)
		{
		case EPlaytester::Novice: return "novice";
		case EPlaytester::Regular: return "regular";
		default: return "good";
		}
	}

	bool ParsePlaytester(const char* Name, EPlaytester& Out)
	{
		if (std::strcmp(Name, "novice") == 0) { Out = EPlaytester::Novice; return true; }
		if (std::strcmp(Name, "regular") == 0) { Out = EPlaytester::Regular; return true; }
		if (std::strcmp(Name, "good") == 0) { Out = EPlaytester::Good; return true; }
		return false;
	}

	void FJamPlaytester::Reset(EPlaytester Kind, uint64_t InSeed)
	{
		P = PlaytesterProfile(Kind);
		Seed = InSeed;
		Tick = 0;
		for (FJamAIInput& R : Ring) { R = FJamAIInput(); }
		MoveOut = FVec2();
		bHolding = false;
		HoldFrames = 0;
		LastPassFrame = -1000;
		LastCheckFrame = -1000;
		PlayFrames = 0;
		Act = EPlaytesterAct::Idle;
		for (int& A : Acts) { A = 0; }
	}

	double FJamPlaytester::Noise(int64_t Window, int Salt) const
	{
		const uint64_t H = Mix(Seed ^ Mix(static_cast<uint64_t>(Window) * 0x100000001B3ull + static_cast<uint64_t>(Salt)));
		return static_cast<double>(H >> 11) * (2.0 / 9007199254740992.0) - 1.0;   // 53 bits -> [-1, 1)
	}

	FJamAIInput FJamPlaytester::Think(const FJamAIWorld& W)
	{
		const FJamAIInput D = Decide(W);
		++Acts[static_cast<int>(Act)];
		constexpr int RingLen = kMaxReactionFrames + 1;
		Ring[Tick % RingLen] = D;
		const int Delay = P.ReactionFrames < 0 ? 0 : (P.ReactionFrames > kMaxReactionFrames ? kMaxReactionFrames : P.ReactionFrames);
		FJamAIInput Out = Tick >= Delay ? Ring[(Tick - Delay) % RingLen] : FJamAIInput();
		++Tick;
		// the analog stick: pushed over at a finite rate, never snapped
		FVec2 Dm = Out.Move - MoveOut;
		const double L = Dm.Length();
		if (L > P.StickRamp && L > 1e-9) { Dm = Dm * (P.StickRamp / L); }
		MoveOut += Dm;
		Out.Move = MoveOut.LimitLength(1.0);
		return Out;
	}

	FJamAIInput FJamPlaytester::Decide(const FJamAIWorld& W)
	{
		FJamAIInput In;
		Act = EPlaytesterAct::Idle;
		if (W.bFrozen || W.PlayerIndex < 0 || W.bFaceoffPhase)
		{
			// goal pause, match over, or the faceoff countdown: hands off (a centre that presses anything in the countdown loses the draw)
			bHolding = false;
			HoldFrames = 0;
			return In;
		}
		const int Pi = W.PlayerIndex;
		const FJamAISkater& Me = W.Skaters[Pi];
		if (W.bDrawOpen)
		{
			// the draw: the centre presses after a person's reaction; anyone else leans toward the dot
			if (Me.bCentre)
			{
				if (W.FramesSinceDrop >= P.DrawReactionFrames) { In.bPoke = true; Act = EPlaytesterAct::Draw; }
			}
			else
			{
				In.Move = (W.PuckPos - Me.Pos).Normalized() * 0.5;
				Act = EPlaytesterAct::Chase;
			}
			return In;
		}
		++PlayFrames;
		if (P.LapseEveryFrames > 0 && PlayFrames > P.LapseEveryFrames && (PlayFrames % P.LapseEveryFrames) < P.LapseFrames)
		{
			// stamina of attention: the novice stops pressing (a held shot is let go: that fires it, as it would for a person)
			bHolding = false;
			HoldFrames = 0;
			Act = EPlaytesterAct::Lapse;
			return In;
		}
		if (Me.bDown)
		{
			// knocked down or stumbling: a person keeps pushing the stick where he wanted to go; the sim ignores it
			Act = EPlaytesterAct::Idle;
			return In;
		}

		// Work in the attacking frame: the opponents' goal at +x (rotate 180 degrees for team 1; keeps handedness).
		const double Sg = Me.Team == 0 ? 1.0 : -1.0;
		const FVec2 Pos = Me.Pos * Sg;
		const FVec2 Face = (Me.Facing * Sg).Normalized();
		const FVec2 Puck = W.PuckPos * Sg;
		const FVec2 PuckV = W.PuckVel * Sg;
		const double Gx = W.GoalX;
		const double Hz = W.HalfWidth;
		const FVec2 Goal(Gx, 0.0);
		const FVec2 OwnGoal(-Gx, 0.0);
		const double MySpeed = Me.Vel.Length();
		const FVec2 ToGoalFromPuck = (Goal - Puck).Normalized();
		// the aim: the goal centre, off by the aim noise of this two-second window
		const double NoiseDeg = P.AimNoiseDeg * Noise(W.Frame / (P.AimWindowFrames > 0 ? P.AimWindowFrames : 1), 1);
		const FVec2 AimDir = Rotate(ToGoalFromPuck, DegToRad(NoiseDeg));

		int NearOpp = -1;
		double NearOppDist = 1e9;
		for (int i = 0; i < W.NumSkaters; ++i)
		{
			if (i == Pi || W.Skaters[i].Team == Me.Team) { continue; }
			const double D = W.Skaters[i].Pos.DistTo(Me.Pos);
			if (D < NearOppDist) { NearOppDist = D; NearOpp = i; }
		}

		FVec2 Target = Puck;
		const bool bCarrying = W.Carrier == Pi;
		const bool bMateCarries = W.Carrier >= 0 && W.Carrier != Pi && W.Skaters[W.Carrier].Team == Me.Team;
		const bool bOppCarries = W.Carrier >= 0 && W.Skaters[W.Carrier].Team != Me.Team;

		if (bCarrying)
		{
			Act = EPlaytesterAct::Carry;
			const double Dgoal = (Goal - Puck).Length();
			const bool bLaneBlocked = Corridor(W, Pi, W.PuckPos, AimDir * Sg, Dgoal, 1.2, false, Me.Team);
			const bool bPressured = NearOppDist < P.PressureM;
			// pass: a teammate clearly ahead and open, while a defender closes or the lane to the goal is blocked
			if (W.Frame - LastPassFrame > 90 && (bPressured || bLaneBlocked))
			{
				for (int i = 0; i < W.NumSkaters; ++i)
				{
					const FJamAISkater& T = W.Skaters[i];
					if (i == Pi || T.Team != Me.Team || T.bDown) { continue; }
					const FVec2 Tp = T.Pos * Sg;
					if (Tp.X < Pos.X + P.PassAheadM) { continue; }
					bool bOpen = true;
					for (int k = 0; k < W.NumSkaters && bOpen; ++k)
					{
						if (W.Skaters[k].Team != Me.Team && W.Skaters[k].Pos.DistTo(T.Pos) < 3.0) { bOpen = false; }
					}
					const FVec2 ToT = T.Pos - W.PuckPos;
					const double Dt = ToT.Length();
					if (!bOpen || Dt < 3.0 || Dt > 40.0 || Corridor(W, Pi, W.PuckPos, ToT * (1.0 / Dt), Dt, 1.0, true, Me.Team)) { continue; }
					In.bPass = true;
					LastPassFrame = W.Frame;
					bHolding = false;
					HoldFrames = 0;
					Act = EPlaytesterAct::Pass;
					break;
				}
			}
			// steer: at the goal, round a defender standing in the way
			Target = Goal;
			for (int i = 0; i < W.NumSkaters; ++i)
			{
				if (i == Pi) { continue; }
				const FVec2 O = W.Skaters[i].Pos * Sg;
				const FVec2 Rel = O - Puck;
				const double Along = Rel.Dot(ToGoalFromPuck);
				const double Side = Rel.Cross(ToGoalFromPuck);
				if (Along > 0.0 && Along < 10.0 && std::fabs(Side) < 2.5 && O.X < Gx - 3.0)
				{
					const FVec2 Perp = ToGoalFromPuck.Perp();
					const FVec2 L = O + Perp * 3.5;
					const FVec2 R = O - Perp * 3.5;
					Target = (std::fabs(L.Y) < std::fabs(R.Y)) ? L : R;   // the side nearer the middle of the ice
					break;
				}
			}
			if (Act != EPlaytesterAct::Pass && Dgoal < P.ShootRangeM && (!bLaneBlocked || bPressured))
			{
				// in range: line up on the aim while holding the button, let go when lined up (or when patience runs out)
				Target = Puck + AimDir * 10.0;
				if (!bHolding) { bHolding = true; HoldFrames = 0; }
				++HoldFrames;
				const double Err = AngleBetweenDeg(Face, AimDir);
				const bool bLined = Err <= P.ReleaseToleranceDeg;
				const bool bFull = HoldFrames >= ChargeFramesMax;
				const bool bRelease = (bLined && (bFull || bPressured || HoldFrames >= P.PatienceFrames / 2)) || HoldFrames >= P.PatienceFrames;
				if (bRelease)
				{
					In.bShootHeld = false;   // the release fires the shot
					bHolding = false;
					HoldFrames = 0;
				}
				else
				{
					In.bShootHeld = true;
				}
				Act = EPlaytesterAct::Charge;
			}
			else if (bHolding)
			{
				// out of range again or passing: let go (fires whatever charge there is; a person does the same)
				bHolding = false;
				HoldFrames = 0;
			}
		}
		else
		{
			bHolding = false;
			HoldFrames = 0;
			if (bMateCarries)
			{
				// support: get ahead of the carrier, on the other side of the ice, and wait for the pass
				Act = EPlaytesterAct::Support;
				const FVec2 Cp = W.Skaters[W.Carrier].Pos * Sg;
				const double Side = Cp.Y >= 0.0 ? -1.0 : 1.0;
				Target = FVec2(Clamp(Cp.X + 12.0, -Gx + 6.0, Gx - 6.0), Side * Clamp(Hz - 5.0, 0.0, 8.0));
			}
			else if (bOppCarries)
			{
				// forecheck / defend: close on the carrier, goalside when it is nearer our net than we are
				Act = EPlaytesterAct::Forecheck;
				const FJamAISkater& C = W.Skaters[W.Carrier];
				const FVec2 Cp = C.Pos * Sg;
				const FVec2 Cv = C.Vel * Sg;
				if (P.bGoalside && Cp.X < Pos.X - 1.0)
				{
					Target = Cp + (OwnGoal - Cp).Normalized() * 2.5 + Cv * 0.3;
				}
				else
				{
					const double Lead = Clamp(Pos.DistTo(Puck) / 10.0, 0.0, P.ChaseLeadS);
					Target = Puck + PuckV * Lead;
				}
				// the blade follows the puck; poke once it is in reach
				const FVec2 Tp = W.PuckPos - Me.Pos;
				const double Dp = Tp.Length();
				if (Dp > 1e-6 && Dp < W.SkaterRadius + W.StickReach + 1.0)
				{
					const double Reach = Dp - W.SkaterRadius;
					In.Stick = Tp * (1.0 / Dp) * (Reach < W.StickReach ? Clamp(Reach / W.StickReach, 0.2, 1.0) : 1.0);
					if (Me.bCanPoke && Dp <= W.SkaterRadius + W.StickReach) { In.bPoke = true; Act = EPlaytesterAct::Poke; }
					if (Me.bCanLift && C.Pos.DistTo(Me.Pos) <= 2.0 * W.SkaterRadius + W.LiftRange * 0.9) { In.bLift = true; }
				}
				// a body check when closing fast and not from behind (the sim's foul line is 0.5 on the contact normal . the victim's facing)
				const double Dc = C.Pos.DistTo(Me.Pos);
				if (Dc < 1.8 && MySpeed > 5.0 && W.Frame - LastCheckFrame > 30)
				{
					const FVec2 N = (C.Pos - Me.Pos).Normalized();
					if (N.Dot(C.Facing) < P.CheckFromBehindDot && Me.Vel.Dot(N) > 4.0)
					{
						In.bCheck = true;
						LastCheckFrame = W.Frame;
						Act = EPlaytesterAct::Check;
					}
				}
			}
			else
			{
				// a loose puck: go where it is (the novice) or where it will be
				Act = EPlaytesterAct::Chase;
				const double Lead = Clamp(Pos.DistTo(Puck) / 10.0, 0.0, P.ChaseLeadS);
				Target = Puck + PuckV * Lead;
				FVec2 N;
				if (BoardSdf(W, W.PuckPos, N) > -1.6)
				{
					Target -= (N * Sg) * 1.0;   // a puck on the boards: come at it from the ice side
				}
			}
		}

		// never steer into a net mouth (the sim lets a skater into the net, where the scripted human got stuck; BALANCING.md)
		if (std::fabs(Target.Y) < W.GoalHalfWidth + 1.0 && std::fabs(Target.X) > Gx - 1.0 && !bCarrying)
		{
			Target.X = Sign(Target.X) * (Gx - 1.0);
		}
		if (bCarrying && Target.X > Gx - 1.5 && std::fabs(Target.Y) < W.GoalHalfWidth + 1.0)
		{
			Target.X = Gx - 1.5;   // carrying into the net: stop at the crease and let the shot go
		}
		// behind a net and heading for the middle: round its side
		if (std::fabs(Pos.X) > Gx + 0.3 && std::fabs(Pos.Y) < W.GoalHalfWidth + 1.2 && (Target.X - Pos.X) * Sign(Pos.X) < 0.0)
		{
			Target = Pos + FVec2(0.0, Pos.Y >= 0.0 ? 3.0 : -3.0);
		}
		FVec2 Want = Target - Pos;
		const double Dist = Want.Length();
		if (Dist < 0.3)
		{
			Want = FVec2();
		}
		else
		{
			// arrive: slow down inside 2 m of a support spot; full stick otherwise
			const double Scale = (Act == EPlaytesterAct::Support && Dist < 2.0) ? Dist / 2.0 : 1.0;
			Want = Want * (Scale / Dist);
		}
		In.Move = Want * Sg;
		return In;
	}

	FJamPlaytestStats RunPlaytestMatch(FJamSimAdapter& Sim, const FJamMatchConfig& C, uint64_t Seed, EPlaytester Kind, int64_t MaxFrames, int ChargeFramesMax)
	{
		FJamPlaytestStats St;
		St.Seed = Seed;
		St.PlayerIndex = C.PlayerIndex();
		const int Pi = St.PlayerIndex;
		St.PlayerTeam = Pi >= 0 ? C.Skaters[Pi].Team : 0;
		Sim.Reset(C, Seed);
		FJamBrain Brain;
		Brain.Reset();
		Brain.Params = C.AI;
		Brain.Params.InterceptSpeed = C.Speed;
		Brain.Params.AimConeDeg = C.AimConeDeg;
		FJamPlaytester Tester;
		Tester.Reset(Kind, Seed);
		Tester.ChargeFramesMax = ChargeFramesMax;
		FJamAIWorld W;
		FJamAIInput In[kMaxSkaters];
		int64_t SinceFaceoff = 0, SinceTouch = 0, CarrySpell = 0;
		int64_t WinMate = 0, WinOpp = 0, WinLoose = 0;   // this window's frames by holder
		auto CountWindow = [&]() { St.WoMateFrames += WinMate; St.WoOppFrames += WinOpp; St.WoLooseFrames += WinLoose; };
		bool bWaitingTouch = Pi >= 0;
		int LastLead = 0;
		St.Faceoffs = 1;
		const int TouchWindow = 600;
		const int IdleWindow = 900;
		const FName KindCheck(TEXT("check"));
		const FName KindFoulBehind(TEXT("foul_from_behind"));
		for (int64_t F = 0; F < MaxFrames && !Sim.IsOver(); ++F)
		{
			Sim.View(W);
			const int PreLastTouch = W.PuckLastTouch;
			for (int i = 0; i < kMaxSkaters; ++i) { In[i] = FJamAIInput(); }
			Brain.Think(W, In);
			if (Pi >= 0) { In[Pi] = Tester.Think(W); ++St.Acts[static_cast<int>(Tester.LastAct())]; }
			const FJamAIStepEvents Ev = Sim.Step(In);
			++St.Frames;
			Sim.View(W);
			const bool bPlay = !W.bFrozen;
			if (bPlay) { ++St.PlayFrames; }
			if (Pi < 0) { continue; }
			// touches and the two waits (after a faceoff, since the last touch)
			if ((Ev.TouchMask >> Pi) & 1u)
			{
				++St.Touches;
				if (St.FirstTouch < 0) { St.FirstTouch = St.Frames; }
				if (SinceTouch > St.MaxNoTouch) { St.MaxNoTouch = SinceTouch; }
				SinceTouch = 0;
				if (bWaitingTouch)
				{
					bWaitingTouch = false;
					if (SinceFaceoff > St.MaxFaceoffToTouch) { St.MaxFaceoffToTouch = static_cast<int>(SinceFaceoff); }
					if (SinceFaceoff > TouchWindow) { ++St.FaceoffsWithoutTouch; CountWindow(); }
				}
			}
			else if (bPlay)
			{
				++SinceTouch;
				if (SinceTouch == IdleWindow) { ++St.IdleSpells; }
				if (SinceTouch > IdleWindow) { ++St.IdleFrames; }
			}
			// carries
			if (W.Carrier == Pi && bPlay) { ++CarrySpell; ++St.CarryFrames; }
			else { if (CarrySpell >= 180) { ++St.Carries3s; } CarrySpell = 0; }
			// shots
			if (Ev.Shot == Pi)
			{
				++St.Shots;
				if (St.FirstShot < 0) { St.FirstShot = St.Frames; }
				const double Gx = St.PlayerTeam == 0 ? W.GoalX : -W.GoalX;
				const FVec2 V = W.PuckVel;
				if ((Gx - W.PuckPos.X) * V.X > 1e-6)
				{
					const double T = (Gx - W.PuckPos.X) / V.X;
					const double Y = W.PuckPos.Y + V.Y * T;
					if (std::fabs(Y) <= W.GoalHalfWidth + 0.12) { ++St.ShotsOnTarget; }
				}
			}
			else if (Ev.Shot >= 0 && Ev.Shot < C.NumSkaters)
			{
				if (C.Skaters[Ev.Shot].Team == St.PlayerTeam) { ++St.ShotsMates; } else { ++St.ShotsOpp; }
			}
			// goals and the lead
			if (Ev.GoalTeam >= 0)
			{
				const int Scorer = W.PuckLastTouch >= 0 ? W.PuckLastTouch : PreLastTouch;
				if (Ev.GoalTeam == St.PlayerTeam)
				{
					++St.GoalsFor;
					if (St.FirstGoalFor < 0) { St.FirstGoalFor = St.Frames; }
					if (Scorer == Pi)
					{
						++St.GoalsByHuman;
						if (St.FirstGoalByHuman < 0) { St.FirstGoalByHuman = St.Frames; }
					}
				}
				else
				{
					++St.GoalsAgainst;
					if (St.FirstGoalAgainst < 0) { St.FirstGoalAgainst = St.Frames; }
				}
				const int Lead = St.GoalsFor > St.GoalsAgainst ? 1 : (St.GoalsFor < St.GoalsAgainst ? -1 : 0);
				if (Lead != 0 && LastLead != 0 && Lead != LastLead) { ++St.LeadChanges; }
				if (Lead != 0) { LastLead = Lead; }
				if (bWaitingTouch)
				{
					if (SinceFaceoff > TouchWindow) { ++St.FaceoffsWithoutTouch; CountWindow(); }
					if (SinceFaceoff > St.MaxFaceoffToTouch) { St.MaxFaceoffToTouch = static_cast<int>(SinceFaceoff); }
					bWaitingTouch = false;
				}
			}
			if (Ev.bFaceoff)
			{
				++St.Faceoffs;
				SinceFaceoff = 0;
				bWaitingTouch = true;
				WinMate = WinOpp = WinLoose = 0;
			}
			else if (bPlay)
			{
				// lane A3 (R-0029 point 5): frames the human sits in the box do not count against the touch window (he cannot touch the puck from there)
				const UJamSim* BoxSim = Sim.GetSim();
				const bool bHumanInBox = BoxSim && BoxSim->GetSkaters().IsValidIndex(Pi) && BoxSim->GetSkaters()[Pi].InBox();
				if (!bHumanInBox) { ++SinceFaceoff; }
				if (bWaitingTouch)
				{
					if (W.Carrier < 0) { ++WinLoose; } else if (W.Skaters[W.Carrier].Team == W.Skaters[Pi].Team) { ++WinMate; } else { ++WinOpp; }
				}
			}
			// the sim's own events: checks, knockdowns, fouls, pokes, lifts, passes of the human
			if (const UJamSim* S = Sim.GetSim())
			{
				for (const FJamSimEvent& E : S->GetFrameEvents())
				{
					if (E.Who != Pi) { continue; }
					if (E.Name == JamEvents::Check) { ++St.ChecksLanded; }
					else if (E.Name == JamEvents::Knockdown) { ++St.Knockdowns; if (E.Kind == KindCheck) { ++St.ChecksTaken; } }
					else if (E.Name == JamEvents::Foul) { ++St.Fouls; if (E.Kind == KindFoulBehind) { ++St.FoulsFromBehind; } else { ++St.FoulsNoPuck; } }
					else if (E.Name == JamEvents::Poke) { ++St.Pokes; if (E.Strength > 0.5f) { ++St.PokesClean; } }
					else if (E.Name == JamEvents::Lift) { if (E.Strength > 0.5f) { ++St.Lifts; } }
					else if (E.Name == JamEvents::Pass) { ++St.Passes; }
				}
				const TArray<FJamSkaterState>& Sk = S->GetSkaters();
				if (Sk.IsValidIndex(Pi) && Sk[Pi].InBox()) { ++St.BoxFrames; }
			}
		}
		if (CarrySpell >= 180) { ++St.Carries3s; }
		if (SinceTouch > St.MaxNoTouch) { St.MaxNoTouch = SinceTouch; }
		if (bWaitingTouch && SinceFaceoff > TouchWindow)
		{
			++St.FaceoffsWithoutTouch;
			CountWindow();
			if (SinceFaceoff > St.MaxFaceoffToTouch) { St.MaxFaceoffToTouch = static_cast<int>(SinceFaceoff); }
		}
		St.bOver = Sim.IsOver();
		St.Winner = Sim.Winner();
		St.Hash = Sim.StateHash();
		return St;
	}

	int FormatPlaytestJson(const FJamPlaytestStats& S, const char* Cartridge, const char* Difficulty, EPlaytester Kind, char* Buf, int BufLen)
	{
		const char* Result = !S.bOver ? "unfinished" : (S.Winner < 0 ? "draw" : (S.Winner == S.PlayerTeam ? "win" : "loss"));
		const double Minutes = S.PlayFrames > 0 ? static_cast<double>(S.PlayFrames) / 3600.0 : 0.0;
		int N = std::snprintf(Buf, static_cast<size_t>(BufLen),
			"{\"tool\":\"thirdjam.playtest/1\",\"cartridge\":\"%s\",\"difficulty\":\"%s\",\"player\":\"%s\",\"seed\":%llu,\"result\":\"%s\",\"over\":%s,"
			"\"goals\":[%d,%d],\"goals_by_human\":%d,\"frames\":%lld,\"play_frames\":%lld,\"touches\":%d,\"touches_per_min\":%.1f,\"carries_3s\":%d,\"carry_frames\":%lld,"
			"\"shots\":%d,\"shots_on_target\":%d,\"shots_mates\":%d,\"shots_opp\":%d,\"lead_changes\":%d,"
			"\"first_touch\":%lld,\"first_shot\":%lld,\"first_goal_for\":%lld,\"first_goal_by_human\":%lld,\"first_goal_against\":%lld,"
			"\"faceoffs\":%d,\"faceoffs_without_touch\":%d,\"max_faceoff_to_touch\":%d,\"wo_frames\":[%lld,%lld,%lld],"
			"\"checks_landed\":%d,\"checks_taken\":%d,\"knockdowns\":%d,\"fouls\":%d,\"fouls_from_behind\":%d,\"fouls_no_puck\":%d,\"box_frames\":%lld,\"pokes\":%d,\"pokes_clean\":%d,\"lifts\":%d,\"passes\":%d,"
			"\"idle_frames\":%lld,\"idle_spells\":%d,\"max_no_touch\":%lld,\"hash\":\"%016llx\",\"player_index\":%d",
			Cartridge, Difficulty, PlaytesterName(Kind), static_cast<unsigned long long>(S.Seed), Result, S.bOver ? "true" : "false",
			S.GoalsFor, S.GoalsAgainst, S.GoalsByHuman, static_cast<long long>(S.Frames), static_cast<long long>(S.PlayFrames), S.Touches,
			Minutes > 0.0 ? S.Touches / Minutes : 0.0, S.Carries3s, static_cast<long long>(S.CarryFrames),
			S.Shots, S.ShotsOnTarget, S.ShotsMates, S.ShotsOpp, S.LeadChanges,
			static_cast<long long>(S.FirstTouch), static_cast<long long>(S.FirstShot), static_cast<long long>(S.FirstGoalFor), static_cast<long long>(S.FirstGoalByHuman),
			static_cast<long long>(S.FirstGoalAgainst), S.Faceoffs, S.FaceoffsWithoutTouch, S.MaxFaceoffToTouch,
			static_cast<long long>(S.WoMateFrames), static_cast<long long>(S.WoOppFrames), static_cast<long long>(S.WoLooseFrames),
			S.ChecksLanded, S.ChecksTaken, S.Knockdowns, S.Fouls, S.FoulsFromBehind, S.FoulsNoPuck, static_cast<long long>(S.BoxFrames), S.Pokes, S.PokesClean, S.Lifts, S.Passes,
			static_cast<long long>(S.IdleFrames), S.IdleSpells, static_cast<long long>(S.MaxNoTouch), static_cast<unsigned long long>(S.Hash), S.PlayerIndex);
		static const char* ActNames[] = {"idle", "draw", "chase", "carry", "charge", "pass", "support", "forecheck", "poke", "check", "lapse"};
		N += std::snprintf(Buf + N, static_cast<size_t>(BufLen - N), ",\"acts\":{");
		for (int a = 0; a < static_cast<int>(EPlaytesterAct::Count) && N < BufLen; ++a)
		{
			N += std::snprintf(Buf + N, static_cast<size_t>(BufLen - N), "%s\"%s\":%d", a ? "," : "", ActNames[a], S.Acts[a]);
		}
		N += std::snprintf(Buf + N, static_cast<size_t>(BufLen - N), "}}");
		return N;
	}
}

// ---- Jam.Playtest (brief A34 point 4), headless: UnrealEditor-Cmd ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Playtest; Quit" -nullrhi -unattended -nopause -stdout
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "JamCartridge.h"
#include "DifficultyResolve.h"

namespace JamPlaytestTestsLocal
{
	constexpr int64 MaxFrames = 36000;

	UJamCartridge* Load(FAutomationTestBase& T, const TCHAR* Name)
	{
		TArray<FString> Errors;
		UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(Name), Errors);
		if (!C) { T.AddError(FString::Printf(TEXT("FAIL %s does not load: %s"), Name, *FString::Join(Errors, TEXT("; ")))); }
		return C;
	}

	JamAI::FJamPlaytestStats Run(const UJamCartridge& Cart, uint64 Seed, JamAI::EPlaytester Kind, const TCHAR* Level)
	{
		JamAI::FJamSimAdapter Sim(Cart);   // lane A41: with the game mode's systems
		return JamAI::RunPlaytestMatch(Sim, JamAI::ToMatchConfigResolved(Cart, Level), Seed, Kind, MaxFrames, Cart.Game.Shot.ChargeFramesMax);
	}

	FString Line(const JamAI::FJamPlaytestStats& S, const char* Level, JamAI::EPlaytester Kind)
	{
		static char Buf[8192];
		JamAI::FormatPlaytestJson(S, "hockey.regular", Level, Kind, Buf, sizeof(Buf));
		return UTF8_TO_TCHAR(Buf);
	}
}

// R-0029 point 5 (lane A3): the novice bar is a gate in its 20-seed form; GoodNeverIdlesAtAllStar stays a "Bar." until it passes.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPlaytestNoviceScoresAtRookie, "Jam.Playtest.NoviceScoresAtRookie",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::CommandletContext | EAutomationTestFlags::EngineFilter)

bool FJamPlaytestNoviceScoresAtRookie::RunTest(const FString& Parameters)
{
	// The M1 bar (R-0029): a fresh human scores within two minutes (7,200 frames) at rookie on hockey.regular in at least 10 of 20 seeds; seed 1 replays (same hash twice).
	using namespace JamPlaytestTestsLocal;
	UJamCartridge* Cart = Load(*this, TEXT("hockey.regular"));
	if (!Cart) { return false; }
	bool bOk = true;
	int32 Scored = 0;
	for (uint64 Seed = 1; Seed <= 20; ++Seed)
	{
		const JamAI::FJamPlaytestStats A = Run(*Cart, Seed, JamAI::EPlaytester::Novice, TEXT("rookie"));
		const bool bScored = A.FirstGoalByHuman >= 0 && A.FirstGoalByHuman <= 7200;
		Scored += bScored ? 1 : 0;
		AddInfo(FString::Printf(TEXT("%s hockey.regular rookie novice seed %llu: first goal by the human at frame %lld, %d:%d, %s"), bScored ? TEXT("PASS") : TEXT("INFO"),
			Seed, static_cast<long long>(A.FirstGoalByHuman), A.GoalsFor, A.GoalsAgainst, A.bOver ? TEXT("over") : TEXT("unfinished")));
		if (Seed == 1)
		{
			const JamAI::FJamPlaytestStats B = Run(*Cart, Seed, JamAI::EPlaytester::Novice, TEXT("rookie"));
			const bool bSame = A.Hash == B.Hash && A.Frames == B.Frames;
			const FString Msg = FString::Printf(TEXT("seed 1 replays: hash %016llx / %016llx, %lld / %lld frames"), static_cast<unsigned long long>(A.Hash), static_cast<unsigned long long>(B.Hash), static_cast<long long>(A.Frames), static_cast<long long>(B.Frames));
			if (bSame) { AddInfo(TEXT("PASS ") + Msg); } else { AddError(TEXT("FAIL ") + Msg); bOk = false; }
		}
	}
	const FString Msg = FString::Printf(TEXT("the novice scores within 7,200 frames at rookie in %d of 20 seeds (need 10)"), Scored);
	if (Scored >= 10) { AddInfo(TEXT("PASS ") + Msg); } else { AddError(TEXT("FAIL ") + Msg); bOk = false; }
	return bOk;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPlaytestGoodNeverIdlesAtAllStar, "Bar.Playtest.GoodNeverIdlesAtAllStar",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::CommandletContext | EAutomationTestFlags::EngineFilter)

bool FJamPlaytestGoodNeverIdlesAtAllStar::RunTest(const FString& Parameters)
{
	// The good player never goes 15 s (900 frames of play) without a touch at all_star on hockey.regular, seeds 1, 2, 7.
	using namespace JamPlaytestTestsLocal;
	UJamCartridge* Cart = Load(*this, TEXT("hockey.regular"));
	if (!Cart) { return false; }
	bool bOk = true;
	for (const uint64 Seed : {1ull, 2ull, 7ull})
	{
		const JamAI::FJamPlaytestStats S = Run(*Cart, Seed, JamAI::EPlaytester::Good, TEXT("all_star"));
		AddInfo(Line(S, "all_star", JamAI::EPlaytester::Good));
		const FString Msg = FString::Printf(TEXT("hockey.regular all_star good seed %llu: %d spells of 15 s without a touch (longest %lld frames), %d:%d in %lld frames, %s"),
			Seed, S.IdleSpells, static_cast<long long>(S.MaxNoTouch), S.GoalsFor, S.GoalsAgainst, static_cast<long long>(S.Frames), S.bOver ? TEXT("over") : TEXT("unfinished"));
		if (S.IdleSpells == 0 && (S.bOver || S.Frames >= MaxFrames)) { AddInfo(TEXT("PASS ") + Msg); } else { AddError(TEXT("FAIL ") + Msg); bOk = false; }
	}
	return bOk;
}
#endif
