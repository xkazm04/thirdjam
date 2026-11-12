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
		// lane A3: deterministic value in [-1, 1] from two integers (splitmix64 finaliser); no state, no seed
		inline double Hash11(uint64_t A, uint64_t B)
		{
			uint64_t Z = A * 0x9E3779B97F4A7C15ull + B + 0x632BE59BD9B4E019ull;
			Z = (Z ^ (Z >> 30)) * 0xBF58476D1CE4E5B9ull;
			Z = (Z ^ (Z >> 27)) * 0x94D049BB133111EBull;
			Z ^= Z >> 31;
			return static_cast<double>(Z >> 11) * (2.0 / 9007199254740992.0) - 1.0;
		}
		inline FVec2 Rotate(const FVec2& V, double Rad) { const double C = std::cos(Rad), S = std::sin(Rad); return FVec2(V.X * C - V.Y * S, V.X * S + V.Y * C); }
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
			InReach[i] = 0;
			for (int k = 0; k <= kMaxReactionFrames; ++k)
			{
				Hist[i][k] = FJamAIInput();
			}
		}
		Tick = 0;
		TeamIntent = FJamAIIntent();
		PlayThinks = 0;
		Faceoffs = 0;
		FaceoffStart = 0;
		DrawJumps = 0;
		DrawPresses = 0;
		for (int i = 0; i < kMaxSkaters; ++i) { Pressed[i] = 0; Charging[i] = 0; bGoalieNow[i] = false; GoalieSideNow[i] = 0; }
		bWasFaceoff = false;
	}

	const FJamBotSkill& FJamBrain::SkillOf(const FJamAIWorld& W, int Id) const
	{
		const int PlayerTeam = W.PlayerIndex >= 0 ? W.Skaters[W.PlayerIndex].Team : 0;
		return W.Skaters[Id].Team != PlayerTeam ? Params.Opp : Params.Mate;
	}

	void FJamBrain::Think(const FJamAIWorld& W, FJamAIInput* Out)
	{
		TeamIntent = FJamAIIntent();
		if (W.IntentKind >= 1 && W.IntentKind <= 4 && W.IntentFramesLeft > 0)
		{
			TeamIntent.Kind = static_cast<ETeamIntent>(W.IntentKind);
			TeamIntent.IssuedFrame = W.Frame;
			TeamIntent.Frames = W.IntentFramesLeft;
			TeamIntent.Target = W.IntentTarget;
		}
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
		// lane A3: reaction delay. While frozen (goal pause) the ring is cleared, so after a faceoff a bot with delay N stands
		// for N frames; while playing, the input sent is the one decided N thinks ago.
		constexpr int Ring = kMaxReactionFrames + 1;
		if (W.bFaceoffPhase && !bWasFaceoff) { ++Faceoffs; FaceoffStart = W.Frame; }
		bWasFaceoff = W.bFaceoffPhase;
		if (W.bFrozen)
		{
			PlayThinks = 0;
			for (int i = 0; i < kMaxSkaters; ++i)
			{
				for (int k = 0; k < Ring; ++k)
				{
					Hist[i][k] = FJamAIInput();
				}
			}
		}
		else
		{
			++Tick;
			++PlayThinks;
		}
		const int Head = static_cast<int>(Tick % Ring);
		for (int i = 0; i < W.NumSkaters; ++i)
		{
			if (i == W.PlayerIndex)
			{
				continue;
			}
			FJamAIInput In;
			if (W.bFrozen || W.Skaters[i].bDown)
			{
				InReach[i] = 0;
				Charging[i] = 0;
				bGoalieNow[i] = false;
				// R-0013 draw: a centre that jumps early (pokes in the countdown) loses the draw; rookie does it at its rate
				const FJamBotSkill& K = SkillOf(W, i);
				if (W.bFaceoffPhase && W.Skaters[i].bCentre && W.FaceoffCountdown == 6 && K.DrawJumpPct > 0
					&& static_cast<int>((Hash11(static_cast<uint64_t>(FaceoffStart) * 7 + 1, static_cast<uint64_t>(i)) + 1.0) * 50.0) < K.DrawJumpPct)
				{
					In.bPoke = true;
					++DrawJumps;
				}
				Hist[i][Head] = In;
				Out[i] = In;
				continue;
			}
			// lane A3 (R-0009): the back is the net's last line (no goalies): it keeps full speed and reacts at once at every level
			const bool bBackRole = W.Skaters[i].Role == ERole::Back;
			double Sc = Params.SpeedScale * (W.Skaters[i].Team != PlayerTeam && !bBackRole ? Params.RedSkill : 1.0);
			if (W.Carrier == i) { Sc *= SkillOf(W, i).CarrySpeed; }
			// R-0007 TeamIntent praise: +10% for the praised teammate (the sim caps |Move| at 1, so it lifts scaled-down bots only)
			if (W.PlayerIndex >= 0 && W.Skaters[i].Team == PlayerTeam && TeamIntent.Kind == ETeamIntent::Praise && TeamIntent.Active(W.Frame)
				&& (TeamIntent.Target < 0 || TeamIntent.Target == i))
			{
				Sc *= 1.1;
			}
			bool bShoot = false, bAimed = false;
			FVec2 Dir = Decide(W, i, bShoot, bAimed);
			if (Dir.Length() > 1.0)
			{
				Dir = Dir.Normalized();
			}
			In.Move = Dir * Sc;
			In.bShoot = bShoot;
			StickPlay(W, i, In);
			// Lane A31 (B-to-A-0024 point 1): the carrier charges an aimed shot on the net. While the lined-up decision holds, shoot is held
			// (bShootHeld, never the edge) for the skill's window; at the window's end, or the frame the line-up is lost, the hold is dropped and
			// the sim fires the charged shot on the release (StepCharge). Rookie's window is 0: a tap, as before. Taps by StickPlay (the dump
			// under a forecheck) and Attack's board chip and pass-by-shot never charge.
			{
				const int Window = SkillOf(W, i).ShotChargeFrames;
				if (Window > 0 && W.Carrier == i && In.bShoot && bAimed && !W.Skaters[i].bNoShoot)
				{
					In.bShoot = false;
					if (Charging[i] < Window) { In.bShootHeld = true; ++Charging[i]; }
					else { Charging[i] = 0; }   // release
				}
				else
				{
					Charging[i] = 0;   // a hold in progress is released here (the sim fires whatever charge it has)
				}
			}
			In.bGoalie = bGoalieNow[i];
			In.GoalieSide = bGoalieNow[i] ? GoalieSideNow[i] : 0;
			// R-0013 draw: the opposing centre takes the draw early at the level's rate (else the sim's bot timing decides)
			{
				const FJamBotSkill& K = SkillOf(W, i);
				if (W.bDrawOpen && W.Skaters[i].bCentre && K.DrawPressPct > 0 && PlayThinks >= K.DrawPressFrames
					&& static_cast<int>((Hash11(static_cast<uint64_t>(FaceoffStart) * 7 + 3, static_cast<uint64_t>(i)) + 1.0) * 50.0) < K.DrawPressPct)
				{
					In.bPoke = true;
					++DrawPresses;
				}
			}
			Hist[i][Head] = In;
			int Delay = bBackRole ? 0 : SkillOf(W, i).ReactionFrames;
			if (TeamIntent.Kind == ETeamIntent::Praise && TeamIntent.Active(W.Frame) && W.PlayerIndex >= 0 && W.Skaters[i].Team == PlayerTeam
				&& (TeamIntent.Target < 0 || TeamIntent.Target == i))
			{
				Delay = 0;   // ACE.md 2: a praised teammate reacts at once
			}
			Delay = Delay < 0 ? 0 : (Delay > kMaxReactionFrames ? kMaxReactionFrames : Delay);
			// the steering is the one decided Delay thinks ago; the shot stays live (a delayed shot almost never meets the sim's
			// reach and facing gate: in the sweep any delay >= 2 frames stopped the AI from scoring at all)
			Out[i] = Hist[i][(Head - Delay + Ring) % Ring];
			Out[i].bShoot = In.bShoot;
			Out[i].bShootHeld = In.bShootHeld;   // lane A31: the charge is live like the shot
			Out[i].bPass = In.bPass;
			Out[i].Stick = In.Stick;
			Out[i].bPoke = In.bPoke;
			Out[i].bLift = In.bLift;
			Out[i].bGoalie = In.bGoalie;
			Out[i].GoalieSide = In.GoalieSide;
		}
	}

	FVec2 FJamBrain::Decide(const FJamAIWorld& W, int Id, bool& bShoot, bool& bAimed)
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
		bGoalieNow[Id] = false;
		GoalieSideNow[Id] = 0;

		// R-0017: a pickup within 8 m, nearer than the puck, while not carrying (all_star only)
		if (SkillOf(W, Id).bSeekPickups && W.Carrier != Id)
		{
			int Best = -1;
			double Bd = 8.0;
			for (int p = 0; p < W.NumPickups; ++p)
			{
				const double Pd = W.PickupPos[p].DistTo(Me);
				if (Pd < Bd && Pd < D) { Bd = Pd; Best = p; }
			}
			if (Best >= 0)
			{
				Used[Id] = S.Role;
				return Arrive(Me, W.PickupPos[Best], 0.5) + Separate(Me, W, Id, Params.SeparateRadius) * 1.2;
			}
		}

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
		// R-0007 TeamIntent defend / space for the human's teammates (pass is read in Attack)
		const bool bMate = W.PlayerIndex >= 0 && S.Team == W.Skaters[W.PlayerIndex].Team;
		if (bMate && TeamIntent.Active(W.Frame) && D > 2.5)
		{
			if (TeamIntent.Kind == ETeamIntent::Defend && S.Role != ERole::Back && Pp.X * OppGoal.X > 0.0)
			{
				// ACE.md 2: with the puck in our attacking half, fall back to 8 m in front of the own net on the puck's side
				FVec2 T(OwnGoal.X + Sign(-OwnGoal.X) * 8.0, Clamp(Pp.Y, -8.0, 8.0));
				Used[Id] = ERole::Back;
				return Arrive(Me, T - S.Vel * 0.25, Params.ArriveSlowRadius) + Separate(Me, W, Id, Params.SeparateRadius) * 1.2;
			}
			else if (TeamIntent.Kind == ETeamIntent::Space)
			{
				// free ice: the point ahead of the puck on the far side from the nearest opponent, kept inside the boards
				FVec2 Near = Pp;
				double Best = 1e18;
				for (int o = 0; o < W.NumSkaters; ++o)
				{
					if (W.Skaters[o].Team == S.Team) { continue; }
					const double Dd = W.Skaters[o].Pos.DistTo(Me);
					if (Dd < Best) { Best = Dd; Near = W.Skaters[o].Pos; }
				}
				FVec2 T = Pp + FVec2(Sign(OppGoal.X) * 12.0, (Near.Y > Me.Y ? -1.0 : 1.0) * 10.0);
				T.X = Clamp(T.X, -W.GoalX + 2.0, W.GoalX - 2.0);
				T.Y = Clamp(T.Y, -W.HalfWidth + 4.0, W.HalfWidth - 4.0);
				Used[Id] = ERole::Support;
				return Arrive(Me, T, Params.ArriveSlowRadius) + Separate(Me, W, Id, Params.SeparateRadius) * 1.2;
			}
		}
		Used[Id] = Role;

		if (Role == ERole::Chaser || (Role == ERole::Support && D < Params.PounceRadius))
		{
			Out = Attack(W, Id, Me, Pp, Dirg, D, bShoot, bAimed);
		}
		else if (Role == ERole::Back)
		{
			if (D < 2.5)
			{
				Out = Attack(W, Id, Me, Pp, Dirg, D, bShoot, bAimed);
			}
			else if (LastManGoalie(W, Id))
			{
				// R-0013 point 3: the carrier is inside our blue line and nobody stands between: hold the crease, squared to the
				// carrier (on the line goal centre -> carrier, 1.5 m out); the stick (StickPlay) pokes when the puck comes into reach
				const FVec2 C = W.Skaters[W.Carrier].Pos;
				FVec2 T = OwnGoal + (C - OwnGoal).Normalized() * 1.5;
				if (std::fabs(T.X) > W.GoalX) { T.X = OwnGoal.X; }
				Used[Id] = ERole::Back;
				Out = Arrive(Me, T - S.Vel * 0.25, 1.0);
				// Lane A31 (B-to-A-0024 point 3): the goalie intent for the visual. Side = where the carrier is, seen from the goalie facing
				// up-ice (Unreal frame: facing +x, left is -y; team 1 faces -x). Within a metre of the centre line: centre.
				bGoalieNow[Id] = true;
				const double Cy = C.Y;
				GoalieSideNow[Id] = std::fabs(Cy) < 1.0 ? 0 : static_cast<int>(Sign(Cy)) * (S.Team == 0 ? 1 : -1);
			}
			else
			{
				FVec2 T;
				const FJamBotSkill& Gk = SkillOf(W, Id);
				if (Pp.X * OwnGoal.X > 0.0 || Pp.DistTo(OwnGoal) < Gk.GuardRangeM)
				{
					// lane A3 (R-0009, feel QA): puck in our half or in shooting range: guard the net, on the line goal centre -> puck, at most
					// GuardRadiusM from the goal centre, so a long shot meets a body (no goalies in this game)
					const FVec2 ToPuck = Pp - OwnGoal;
					const double L = ToPuck.Length();
					const double R = L * 0.5 < Gk.GuardRadiusM ? L * 0.5 : Gk.GuardRadiusM;
					T = OwnGoal + ToPuck.Normalized() * R;
					if (std::fabs(T.X) > W.GoalX) { T.X = OwnGoal.X; }
				}
				else
				{
					const FVec2 Gl = OwnGoal + FVec2(Sign(-OwnGoal.X) * 3.0, 0.0);
					T = Gl + (Pp - Gl).LimitLength(Params.BackRadius);
				}
				Out = Arrive(Me, T - S.Vel * 0.25, Params.ArriveSlowRadius);   // damped: stop on the post, do not slide through it
				// lane A3 (R-0009): a shot on our net: step onto its path (the closest point of the puck's line)
				const double Pv = W.PuckVel.Length();
				if (SkillOf(W, Id).bBlockShots && Pp.DistTo(OwnGoal) >= SkillOf(W, Id).BlockMinDistM && Pv > 6.0 && W.PuckVel.Dot(OwnGoal - Pp) > 0.0)
				{
					const FVec2 Dir = W.PuckVel * (1.0 / Pv);
					const double Tg = std::fabs(Dir.X) > 1e-6 ? (OwnGoal.X - Pp.X) / Dir.X : -1.0;
					const double Yg = Pp.Y + Dir.Y * Tg;
					if (Tg > 0.0 && std::fabs(Yg) < W.GoalHalfWidth + 1.0)
					{
						double Along = (Me - Pp).Dot(Dir);
						Along = Along < 0.0 ? 0.0 : (Along > Tg ? Tg : Along);
						const FVec2 Block = Pp + Dir * Along;
						// damped: aim ahead of our own drift so the skater stops on the line instead of sliding through it
						Out = Arrive(Me, Block - S.Vel * 0.25, 1.0);
					}
				}
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
			// R-0017: a support that reads lanes stands in the pass lane from the opposing carrier to its nearest teammate
			const int C = W.Carrier;
			if (SkillOf(W, Id).bReadLanes && C >= 0 && W.Skaters[C].Team != S.Team)
			{
				int Mate = -1;
				double Best = 1e18;
				for (int o = 0; o < W.NumSkaters; ++o)
				{
					if (o == C || W.Skaters[o].Team != W.Skaters[C].Team || W.Skaters[o].bDown) { continue; }
					const double Dd = W.Skaters[o].Pos.DistTo(W.Skaters[C].Pos);
					if (Dd < Best) { Best = Dd; Mate = o; }
				}
				if (Mate >= 0 && Best < 30.0)
				{
					const FVec2 PassLane = LerpV(W.Skaters[C].Pos, W.Skaters[Mate].Pos, 0.45);
					Out = Arrive(Me, PassLane - S.Vel * 0.2, Params.ArriveSlowRadius);
				}
			}
		}
		Out += Separate(Me, W, Id, Params.SeparateRadius) * 1.2;
		return Out;
	}

	bool FJamBrain::LastManGoalie(const FJamAIWorld& W, int Id) const
	{
		const int C = W.Carrier;
		const FJamAISkater& S = W.Skaters[Id];
		if (C < 0 || W.Skaters[C].Team == S.Team)
		{
			return false;
		}
		const FVec2 OwnGoal = W.OwnGoal(S.Team);
		const FVec2 Cp = W.Skaters[C].Pos;
		// inside our blue line: the outer third of the rink on our side
		if (Cp.X * OwnGoal.X <= 0.0 || std::fabs(Cp.X) < W.HalfLength / 3.0)
		{
			return false;
		}
		const FVec2 Line = Cp - OwnGoal;
		const double L = Line.Length();
		if (L < 1e-6) { return false; }
		const FVec2 U = Line * (1.0 / L);
		for (int o = 0; o < W.NumSkaters; ++o)
		{
			if (o == Id || W.Skaters[o].Team != S.Team || W.Skaters[o].bDown) { continue; }
			const FVec2 Rel = W.Skaters[o].Pos - OwnGoal;
			const double Along = Rel.Dot(U);
			if (Along > 0.0 && Along < L && std::fabs(Rel.Cross(U)) < 2.5) { return false; }   // a teammate stands between
		}
		return true;
	}

	void FJamBrain::StickPlay(const FJamAIWorld& W, int Id, FJamAIInput& In)
	{
		const FJamAISkater& S = W.Skaters[Id];
		const FJamBotSkill& K = SkillOf(W, Id);
		const FVec2 To = W.PuckPos - S.Pos;
		const double D = To.Length();
		const double Blade = W.SkaterRadius + W.StickReach;
		// the blade follows the puck when it is near (zero aim = the sim's default, ahead at half reach)
		if (D > 1e-6 && D < Blade + 1.0)
		{
			In.Stick = To * (1.0 / D) * Clamp((D - W.SkaterRadius) / (W.StickReach > 0.01 ? W.StickReach : 0.01), 0.2, 1.0);
		}
		const int C = W.Carrier;
		const bool bOppCarrier = C >= 0 && W.Skaters[C].Team != S.Team;
		InReach[Id] = (bOppCarrier && D <= Blade) ? InReach[Id] + 1 : 0;
		// poke: the level's timing (frames the puck must sit inside reach); rookie never pokes
		if (K.PokeFrames >= 0 && bOppCarrier && S.bCanPoke && InReach[Id] > K.PokeFrames)
		{
			In.bPoke = true;
			InReach[Id] = 0;
		}
		// lift at contact
		if (K.bLift && bOppCarrier && S.bCanLift && W.Skaters[C].Pos.DistTo(S.Pos) <= 2.0 * W.SkaterRadius + W.LiftRange * 0.9)
		{
			In.bLift = true;
		}
		// pass out of pressure: a defender within 2 m and an open teammate (no opponent within 3 m of it, within 25 m)
		if (K.bPassOut && C == Id && !In.bShoot)
		{
			bool bPressure = false, bOpen = false;
			for (int o = 0; o < W.NumSkaters; ++o)
			{
				if (W.Skaters[o].Team != S.Team && !W.Skaters[o].bDown && W.Skaters[o].Pos.DistTo(S.Pos) < 2.0) { bPressure = true; }
			}
			for (int t = 0; t < W.NumSkaters && bPressure && !bOpen; ++t)
			{
				if (t == Id || W.Skaters[t].Team != S.Team || W.Skaters[t].bDown || W.Skaters[t].Pos.DistTo(S.Pos) > 25.0) { continue; }
				bool bFree = true;
				for (int o = 0; o < W.NumSkaters; ++o)
				{
					if (W.Skaters[o].Team != S.Team && W.Skaters[o].Pos.DistTo(W.Skaters[t].Pos) < 3.0) { bFree = false; }
				}
				bOpen = bFree;
			}
			In.bPass = bPressure && bOpen;
		}
		// Director 15:16: a carrier under the human's forecheck gives the puck up after the level's frames: pass, else dump it
		if (C == Id && W.PlayerIndex >= 0 && W.Skaters[W.PlayerIndex].Team != S.Team && !W.Skaters[W.PlayerIndex].bDown
			&& W.Skaters[W.PlayerIndex].Pos.DistTo(W.PuckPos) <= W.SkaterRadius + W.StickReach)
		{
			++Pressed[Id];
		}
		else
		{
			Pressed[Id] = 0;
		}
		if (K.ForecheckYieldFrames >= 0 && Pressed[Id] > K.ForecheckYieldFrames)
		{
			In.bPass = true;
			if (Pressed[Id] > K.ForecheckYieldFrames + 8 && S.bCanShoot) { In.bShoot = true; }
		}
		// bots never body-check (R-0013 fouls: no check from behind or on a skater without the puck is ever thrown)
		In.bCheck = false;
		// R-0019: never shoot from inside a no_shoot zone
		if (S.bNoShoot) { In.bShoot = false; }
	}

	FVec2 FJamBrain::Attack(const FJamAIWorld& W, int Id, const FVec2& Me, const FVec2& Pp, const FVec2& Dirg, double D, bool& bShoot, bool& bAimed) const
	{
		const FJamAISkater& S = W.Skaters[Id];
		const FJamBotSkill& K = SkillOf(W, Id);
		const FVec2 Fwd = S.Facing.Normalized();
		const FVec2 OppGoal = W.OppGoal(S.Team);
		const double Dgoal = Pp.DistTo(OppGoal);
		const double GeoTol = RadToDeg(std::atan2(1.4, Dgoal > 1.0 ? Dgoal : 1.0));
		double Tol = K.AimToleranceDeg < GeoTol ? K.AimToleranceDeg : GeoTol;
		// lane A3: aim error. The bot lines up on a point off the goal centre by up to AimErrorDeg; the error is a fixed
		// function of the skater and a 2-second window, so the approach does not jitter.
		FVec2 Aim = Dirg;
		if (K.AimErrorDeg > 0.0)
		{
			Aim = Rotate(Dirg, DegToRad(K.AimErrorDeg * Hash11(static_cast<uint64_t>(Id) + 1, static_cast<uint64_t>(W.Frame / 120))));
		}
		bool bFaceOk = Fwd.Dot(Aim) > std::cos(DegToRad(Tol));
		if (K.bUseAimCone && Params.AimConeDeg > 0.0 && K.AimErrorDeg <= 0.0)
		{
			// lane A3: the sim snaps a shot facing inside the cone onto the goal centre; a bot that knows it shoots there
			bFaceOk = bFaceOk || Fwd.Dot(Dirg) > std::cos(DegToRad(Params.AimConeDeg * 0.9));
		}
		const bool bNearWall = std::fabs(Pp.Y) > W.HalfWidth - 2.0 && Dgoal > 6.0;
		bool bOkShot = bFaceOk && Dgoal < W.ShotReach + K.ShootExtraM && !bNearWall;
		if (bOkShot && K.bClearLane)
		{
			// lane A3: no shot into a defender's body; keep carrying instead
			const FVec2 Lane = Fwd;
			for (int o = 0; o < W.NumSkaters && bOkShot; ++o)
			{
				if (W.Skaters[o].Team == S.Team) { continue; }
				const FVec2 Rel = W.Skaters[o].Pos - Pp;
				const double Along = Rel.Dot(Lane);
				if (Along > 0.0 && Along < Dgoal && std::fabs(Rel.Cross(Lane)) < 1.2) { bOkShot = false; }
			}
		}
		if (bStalled && Fwd.Dot(Dirg) > 0.0)
		{
			bOkShot = true;
		}
		if (D < W.ShotReach && bOkShot && S.bCanShoot)
		{
			bShoot = true;
			bAimed = true;   // lane A31: the one shot worth charging
		}
		// lane A3: a teammate that holds the puck and faces the human with a clear lane passes it (shot = pass, no pass button)
		const bool bPassIntent = TeamIntent.Kind == ETeamIntent::Pass && TeamIntent.Active(W.Frame);
		if ((K.bPassToHuman || bPassIntent) && W.PlayerIndex >= 0 && W.Skaters[W.PlayerIndex].Team == S.Team && S.bCanShoot && D < W.ShotReach)
		{
			const FJamAISkater& H = W.Skaters[W.PlayerIndex];
			const FVec2 To = H.Pos + H.Vel * 0.3 - Pp;
			const double Dh = To.Length();
			if (Dh > 5.0 && Dh < 25.0 && !H.bDown && Fwd.Dot(To * (1.0 / Dh)) > std::cos(DegToRad(15.0)))
			{
				bool bClear = true;
				for (int o = 0; o < W.NumSkaters && bClear; ++o)
				{
					if (W.Skaters[o].Team == S.Team) { continue; }
					const FVec2 Rel = W.Skaters[o].Pos - Pp;
					const double Along = Rel.Dot(To) / Dh;
					bClear = !(Along > 0.0 && Along < Dh && std::fabs(Rel.Cross(To) / Dh) < 2.0);
				}
				if (bClear) { bShoot = true; }
			}
		}
		// R-0007 pass intent: the carrier turns to face the human (lead 1.5 x velocity) so the pass above can fire
		if (bPassIntent && W.PlayerIndex >= 0 && W.Skaters[W.PlayerIndex].Team == S.Team && D < W.ShotReach + 0.5)
		{
			const FJamAISkater& H = W.Skaters[W.PlayerIndex];
			const FVec2 Lead = H.Pos + H.Vel * 1.5;
			if (Lead.DistTo(Pp) < 25.0) { return (Lead - Me).Normalized(); }
		}
		// lane A2: chip a puck off the boards (dribbling cannot: no skater fits between the board and the puck)
		FVec2 Bn;
		if (bBoardPeel && S.bCanShoot && BoardSdf(W, Pp, Bn) > -1.0 && Fwd.Dot(-Bn) > 0.6 && Fwd.X * (OppGoal.X >= 0.0 ? 1.0 : -1.0) > -0.2)
		{
			bShoot = true;
		}
		// Approach direction: towards the goal, or off the boards when the puck lies on them (lane A2 addition: the
		// Godot brain only aimed at the goal, which on an end board puts the approach point outside the rink).
		FVec2 Push = Aim;
		// R-0019: the carrier takes the puck round any slow-ice circle whose edge comes within 3 m of its straight line to the net
		if (W.Carrier == Id)
		{
			for (int z = 0; z < W.NumSlowIce; ++z)
			{
				const FVec2 Rel = W.SlowIceCentre[z] - Pp;
				const double Along = Rel.Dot(Dirg);
				if (Along <= 0.0 || Along > Dgoal) { continue; }
				const double Side = Rel.Cross(Dirg);   // > 0: the centre lies to one side of the line
				const double R = W.SlowIceRadius[z];
				if (std::fabs(Side) - R < 3.0)
				{
					const FVec2 Off = Dirg.Perp() * (Side > 0.0 ? 1.0 : -1.0);   // towards the far side from the centre
					const FVec2 Round = W.SlowIceCentre[z] + Off * (R + 3.5);
					Push = (Round - Pp).Normalized();
					break;
				}
			}
		}
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
		double Lead = Clamp(D / 8.0, 0.0, K.LeadFrames / 60.0);
		if (K.bIntercept)
		{
			// lane A3: earliest t with |Pp + V t - Me| = speed * t (speed = own top speed), capped like the plain lead
			const FVec2 R = Pp - Me;
			const FVec2 V = W.PuckVel.LimitLength(25.0);
			const double Sp = Params.InterceptSpeed * Params.SpeedScale;
			const double A = V.Dot(V) - Sp * Sp, B = 2.0 * R.Dot(V), C = R.Dot(R);
			double T = -1.0;
			if (std::fabs(A) < 1e-9)
			{
				T = std::fabs(B) > 1e-9 ? -C / B : -1.0;
			}
			else
			{
				const double Disc = B * B - 4.0 * A * C;
				if (Disc >= 0.0)
				{
					const double Sq = std::sqrt(Disc);
					const double T1 = (-B - Sq) / (2.0 * A), T2 = (-B + Sq) / (2.0 * A);
					const double Lo = T1 < T2 ? T1 : T2, Hi = T1 < T2 ? T2 : T1;
					T = Lo > 0.0 ? Lo : Hi;
				}
			}
			Lead = Clamp(T > 0.0 ? T : D / 8.0, 0.0, K.LeadFrames / 60.0);
		}
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
		// lane A3 (R-0013): a centre takes the draw with a person's reaction time after the drop
		if (W.bDrawOpen && Me.bCentre && W.FramesSinceDrop >= DrawReactionFrames)
		{
			In.bPoke = true;
		}
		// lane A3 (R-0013 / Director 14:45): forecheck: against an opposing carrier the blade follows the puck and pokes once it is in reach
		if (W.Carrier >= 0 && W.Skaters[W.Carrier].Team != Me.Team)
		{
			const FVec2 Tp = W.PuckPos - Me.Pos;
			const double Dp = Tp.Length();
			if (Dp > 1e-6 && Dp < W.SkaterRadius + W.StickReach + 1.0)
			{
				In.Stick = Tp * (1.0 / Dp) * (Dp - W.SkaterRadius < W.StickReach ? (Dp - W.SkaterRadius > 0.2 * W.StickReach ? (Dp - W.SkaterRadius) / W.StickReach : 0.2) : 1.0);
				if (Me.bCanPoke && Dp <= W.SkaterRadius + W.StickReach) { In.bPoke = true; }
				// and lifts the carrier's stick at contact range
				if (Me.bCanLift && W.Skaters[W.Carrier].Pos.DistTo(Me.Pos) <= 2.0 * W.SkaterRadius + W.LiftRange * 0.9) { In.bLift = true; }
			}
		}
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
		// lane A3 (forecheck): against an opposing carrier, skate to where the puck will be (a lead of up to 1 s on its velocity, in the
		// attacking frame) instead of lining up behind it; trailing a carrier at equal speed never closes the gap
		if (bForecheckLead && W.Carrier >= 0 && W.Skaters[W.Carrier].Team != Me.Team)
		{
			const FVec2 PuckV = W.PuckVel * Sg;
			const double Lead = Clamp(ToPuck.Length() / 10.0, 0.0, 1.0);
			FVec2 Tgt = Puck + PuckV * Lead;
			// the carrier's body is between us and the puck (riding the boards): come round it on the ice side
			const FVec2 Cp = W.Skaters[W.Carrier].Pos * Sg;
			const FVec2 ToC = Cp - Pos;
			if (ToC.Length() < ToPuck.Length() && ToC.Dot(ToPuck) > 0.0 && std::fabs(ToC.Cross(ToPuck.Normalized())) < 1.5)
			{
				FVec2 Side = ToPuck.Normalized().Perp();
				if (Side.Dot(FVec2(0.0, 0.0) - Cp) < 0.0) { Side = -Side; }   // the side that faces the middle of the rink
				Tgt = Cp + Side * 2.5 + PuckV * Lead;
			}
			// never chase into either net: a target behind a goal line in front of the net mouth stays a metre out
			// (the sim lets a skater into the net, where it got stuck for 400 frames in the 16:06 trace, regular pro seed 15)
			if (std::fabs(Tgt.Y) < W.GoalHalfWidth + 1.5 && std::fabs(Tgt.X) > Gx - 1.0) { Tgt.X = Sign(Tgt.X) * (Gx - 1.0); }
			Want = Tgt - Pos;
		}
		// lane A3 (16:12): never steer into either net mouth (the sim lets a skater into the net and it can stick there for many
		// seconds; the 16:11 trace scan had the scripted human inside a net up to 34 s per match on the large rink)
		{
			FVec2 T = Pos + Want;
			if (std::fabs(T.Y) < W.GoalHalfWidth + 1.0 && std::fabs(T.X) > Gx - 1.0)
			{
				T.X = Sign(T.X) * (Gx - 1.0);
				Want = T - Pos;
			}
			// behind a net (between its back and the end boards) and heading for the middle: go round the side of the net,
			// not into its back (the 16:12 trace: the human pushed into the back of the net for up to 2 minutes)
			if (std::fabs(Pos.X) > Gx + 0.3 && std::fabs(Pos.Y) < W.GoalHalfWidth + 1.2 && Want.X * Sign(Pos.X) < 0.0)
			{
				const double Sy = Pos.Y >= 0.0 ? 1.0 : -1.0;
				Want = FVec2(0.0, Sy);
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
