// Lane A2: brain and player bot (engine-free). See JamAICore.h.
#include "JamAICore.h"

namespace JamAI
{
	namespace AICoreLocal
	{
		using namespace AIMath;   // lane A55: Clamp, Sign, DegToRad, RadToDeg, Rotate live in JamAICore.h (shared with JamPlaytester.cpp)
		inline double Lerp(double A, double B, double T) { return A + (B - A) * T; }
		inline FVec2 LerpV(const FVec2& A, const FVec2& B, double T) { return A + (B - A) * T; }
		// lane A3: deterministic value in [-1, 1] from two integers (splitmix64 finaliser); no state, no seed
		inline double Hash11(uint64_t A, uint64_t B)
		{
			uint64_t Z = A * 0x9E3779B97F4A7C15ull + B + 0x632BE59BD9B4E019ull;
			Z = (Z ^ (Z >> 30)) * 0xBF58476D1CE4E5B9ull;
			Z = (Z ^ (Z >> 27)) * 0x94D049BB133111EBull;
			Z ^= Z >> 31;
			return static_cast<double>(Z >> 11) * (2.0 / 9007199254740992.0) - 1.0;
		}
		// R-0035 (lane A52): the shooter's charge choice (ChargeWindowFor): inside ChargeCloseM of the net a tap; a lift level must clear the bar by ChargeBarMarginM.
		constexpr double ChargeCloseM = 6.0;
		constexpr double ChargeBarMarginM = 0.1;
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
			CheckCool[i] = 0;
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
		PuckHistFrame = -1;   // lane A51
		PuckHistCount = 0;
	}

	// Lane A51 (R-0034): one entry per frame: the puck's offset from the carrier's body (the carrier's index, -1 loose).
	void FJamBrain::RecordPuck(const FJamAIWorld& W)
	{
		if (W.Frame == PuckHistFrame) { return; }
		if (PuckHistFrame >= 0 && W.Frame != PuckHistFrame + 1) { PuckHistCount = 0; }   // a jump (restart, a skipped frame): the history starts over
		FPuckHist& H = PuckHist[static_cast<int>(W.Frame % kPuckHist)];
		H.Carrier = W.Carrier;
		H.Rel = (W.Carrier >= 0 && W.Carrier < W.NumSkaters) ? W.PuckPos - W.Skaters[W.Carrier].Pos : W.PuckPos;
		PuckHistFrame = W.Frame;
		if (PuckHistCount < kPuckHist) { ++PuckHistCount; }
	}

	FVec2 FJamBrain::PuckSeen(const FJamAIWorld& W, int Id) const
	{
		const int C = W.Carrier;
		if (C < 0 || C >= W.NumSkaters || W.Skaters[C].Team == W.Skaters[Id].Team) { return W.PuckPos; }   // only an opposing carrier is read late
		const int React = W.SweepBotReactFrames >= 0 ? W.SweepBotReactFrames : SkillOf(W, Id).SweepReactFrames;
		if (React <= 0 || React >= kPuckHist || PuckHistCount <= React) { return W.PuckPos; }
		const FPuckHist& H = PuckHist[static_cast<int>((W.Frame - React) % kPuckHist)];
		if (H.Carrier != C) { return W.PuckPos; }   // a fresh carrier is read live
		return W.Skaters[C].Pos + H.Rel;
	}

	const FJamBotSkill& FJamBrain::SkillOf(const FJamAIWorld& W, int Id) const
	{
		const int PlayerTeam = W.PlayerIndex >= 0 ? W.Skaters[W.PlayerIndex].Team : 0;
		return W.Skaters[Id].Team != PlayerTeam ? Params.Opp : Params.Mate;
	}

	// R-0035 (lane A52): the shooter's charge reads the net's height. A bot whose profile charges at all (ShotChargeFrames > 0) holds for the window that
	// lands the puck under the bar: the full charge when full_m clears the bar by the margin (a real 1.22 net), half_frames when half_m does (the 0.61
	// bar), a tap from close (inside ChargeCloseM: the lob is not worth the hold) or when nothing fits. A bot with aim error (rookie's profile when it
	// charges) mis-charges by up to that error in frames (a fixed function of the skater and the 2-second window, like the aim). Pure.
	// UJamSim::LiftForCharge's rule on the brain's copy of the keys: the height a hold of F frames arrives at over the goal line.
	double LiftAtFrames(const FJamAIWorld& W, int F)
	{
		const int Max = W.ChargeFramesMax;
		const int Half = W.LiftHalfFrames;
		if (Max <= 0) { return W.LiftTapM; }
		const double C = static_cast<double>(F < 0 ? 0 : (F > Max ? Max : F));
		if (Half <= 0 || Half >= Max) { return Lerp(W.LiftTapM, W.LiftFullM, C / Max); }
		if (C <= Half) { return Lerp(W.LiftTapM, W.LiftHalfM, C / Half); }
		return Lerp(W.LiftHalfM, W.LiftFullM, (C - Half) / (Max - Half));
	}

	int ChargeWindowFor(const FJamAIWorld& W, int Id, const FJamBotSkill& K)
	{
		if (K.ShotChargeFrames <= 0 || W.ChargeFramesMax <= 0) { return 0; }
		const FJamAISkater& S = W.Skaters[Id];
		const double Dgoal = W.PuckPos.DistTo(W.OppGoal(S.Team));
		if (Dgoal < ChargeCloseM) { return 0; }
		// The longest hold whose arrival stays under the bar by the margin (the lift grows with the hold).
		int Best = -1;
		for (int F = 0; F <= W.ChargeFramesMax; ++F)
		{
			if (LiftAtFrames(W, F) + ChargeBarMarginM < W.GoalHeightM) { Best = F; } else { break; }
		}
		if (Best <= 0) { return 0; }
		// A real net takes the full charge; a low bar takes the profile's wind-up (pro 10, all_star 18) cut to what clears it (0.61: 10 frames, 0.495 m).
		int Window = Best >= W.ChargeFramesMax ? W.ChargeFramesMax : (Best < K.ShotChargeFrames ? Best : K.ShotChargeFrames);
		if (K.AimErrorDeg > 0.0 && Window > 0)
		{
			Window += static_cast<int>(std::lround(K.AimErrorDeg * Hash11(static_cast<uint64_t>(Id) + 17, static_cast<uint64_t>(W.Frame / 120))));
		}
		return Window < 0 ? 0 : (Window > W.ChargeFramesMax ? W.ChargeFramesMax : Window);
	}

	void FJamBrain::Think(const FJamAIWorld& W, FJamAIInput* Out)
	{
		RecordPuck(W);   // lane A51: before any bot reads the puck
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
			bool bShoot = false, bAimed = false, bDeke = false, bPass = false;
			FVec2 Dir = Decide(W, i, bShoot, bAimed, bDeke, bPass);
			if (Dir.Length() > 1.0)
			{
				Dir = Dir.Normalized();
			}
			In.Move = Dir * Sc;
			In.bShoot = bShoot;
			In.bDeke = bDeke;   // R-0029 point 4 (lane A39)
			In.bPass = bPass;   // lane A46: the pass to the human
			StickPlay(W, i, In);
			// Lane A77 (R-0036): the saucer. A carrier that charges its shots (pro, all_star; rookie's window is 0) whose pass has no flat lane but a saucer lane
			// (W.CarrierBestPass < 0, W.CarrierBestSaucer >= 0) holds pass for W.SaucerHalfFrames and releases it: the sim lifts the puck over the blade in the lane.
			// While it holds, StickPlay keeps the puck inside shot reach (A66's cap). The puck gone, the hold is dropped (the sim fires nothing out of reach).
			if (W.Carrier == i && W.SaucerHalfFrames > 0 && SkillOf(W, i).ShotChargeFrames > 0)
			{
				const bool bWantSaucer = In.bPass && W.CarrierBestPass < 0 && W.CarrierBestSaucer >= 0;
				if (bWantSaucer || PassHold[i] > 0)
				{
					In.bPass = false;
					if (PassHold[i] < W.SaucerHalfFrames) { In.bPassHeld = true; ++PassHold[i]; }
					else { PassHold[i] = 0; }   // the release: the sim's StepPassCharge passes the saucer
				}
			}
			else { PassHold[i] = 0; }
			// Lane A31 (B-to-A-0024 point 1): the carrier charges an aimed shot on the net. While the lined-up decision holds, shoot is held
			// (bShootHeld, never the edge) for the skill's window; at the window's end, or the frame the line-up is lost, the hold is dropped and
			// the sim fires the charged shot on the release (StepCharge). Rookie's window is 0: a tap, as before. Taps by StickPlay (the dump
			// under a forecheck) and Attack's board chip and pass-by-shot never charge.
			{
				const int Window = ChargeWindowFor(W, i, SkillOf(W, i));   // R-0035 (lane A52): the hold picks the height under the bar (the profile's frames before)
				// Lane A47 (B-to-A-0030 fault 9): QA1 saw pro charges of 0.00 to 0.07 with the window at 10. Two causes. (1) The hold asked for
				// W.Carrier == i, and the carrier of record is the last toucher with the puck in reach: a bot arriving on a loose puck or a pass is aimed
				// and in reach before it has touched it, so its first shot went out as a tap through In.bShoot. (2) A hold in progress was released the
				// first frame the line-up or the carrier test flickered (the puck a few centimetres past reach on a fast carry), after one or two frames.
				// Now an aimed shot in reach charges whether or not the bot is the carrier of record, and a hold in progress rides out a frame without
				// the line-up while the bot can still shoot (puck in reach, cooldown over); it releases at the window's end or when the puck is gone.
				const bool bWantCharge = Window > 0 && In.bShoot && bAimed && !W.Skaters[i].bNoShoot;
				const bool bKeepCharge = Window > 0 && Charging[i] > 0 && W.Skaters[i].bCanShoot && !W.Skaters[i].bNoShoot;
				if (bWantCharge || bKeepCharge)
				{
					In.bShoot = false;
					if (Charging[i] < Window) { In.bShootHeld = true; ++Charging[i]; }
					else { Charging[i] = 0; }   // release: the sim fires the charged shot
				}
				else
				{
					Charging[i] = 0;   // a hold in progress is released here (the sim fires whatever charge it has)
				}
			}
			// R-0033 point 2 (lane A50): any skater standing in its own crease takes the stance, not only the back on its LastManGoalie rule. The
			// steering above is untouched: this is the visual intent only (the game mode sends SetGoalieStance on a change).
			if (!bGoalieNow[i])
			{
				int Side = 0;
				if (CreaseStance(W, i, Side)) { bGoalieNow[i] = true; GoalieSideNow[i] = Side; }
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
			Out[i].bPassHeld = In.bPassHeld;   // lane A77: the saucer hold is live like the charge
			Out[i].Stick = In.Stick;
			Out[i].bPoke = In.bPoke;
			Out[i].bCheck = In.bCheck;   // R-0031 point 2 (lane A45): the check press is live like the shot (a delayed press misses the 6-frame window)
			Out[i].bLift = In.bLift;
			Out[i].bGoalie = In.bGoalie;
			Out[i].GoalieSide = In.GoalieSide;
			Out[i].bDeke = In.bDeke;   // lane A39: live, like the shot: a delayed deke would miss the moment
			// lane A72 (ask A2): game.ai.sprint. A bot sprints while it chases the puck from more than 6 m without carrying it, in a share of the two-second
			// windows (a fixed coin per bot); the sim spends the same stamina bar as the human's. Absent = 0: no bot ever sets it.
			Out[i].bSprint = false;
			if (Params.SprintShare > 0.0 && W.Carrier != i && !W.bDrawOpen)
			{
				const FVec2 ToPuck = W.PuckPos - W.Skaters[i].Pos;
				const bool bChase = ToPuck.Length() > 6.0 && Out[i].Move.Dot(ToPuck) > 0.0;
				Out[i].bSprint = bChase && (Params.SprintShare >= 1.0
					|| (Hash11(static_cast<uint64_t>(i) + 401, static_cast<uint64_t>(W.Frame / 120)) + 1.0) * 0.5 < Params.SprintShare);
			}
		}
	}

	FVec2 FJamBrain::Decide(const FJamAIWorld& W, int Id, bool& bShoot, bool& bAimed, bool& bDeke, bool& bPass)
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

		// R-0041 point 1 (lane A79): a hunting support goes at an opposing carrier within its skill's HuntRadiusM (a yielding bot's D is 99, so it stays out)
		const FJamBotSkill& Hk = SkillOf(W, Id);
		const bool bHunt = Hk.HuntRadiusM > 0.0 && D < 90.0 && W.Carrier >= 0 && W.Skaters[W.Carrier].Team != S.Team && !W.Skaters[W.Carrier].bDown
			&& W.Skaters[W.Carrier].Pos.DistTo(Me) < Hk.HuntRadiusM;
		if (Role == ERole::Chaser || (Role == ERole::Support && (D < Params.PounceRadius || bHunt)))
		{
			Out = Attack(W, Id, Me, Pp, Dirg, D, bShoot, bAimed, bDeke, bPass);
		}
		else if (Role == ERole::Back)
		{
			if (D < 2.5)
			{
				Out = Attack(W, Id, Me, Pp, Dirg, D, bShoot, bAimed, bDeke, bPass);
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
				// lane A3 (R-0009): a shot on our net: step onto its path (the closest point of the puck's line). R-0029 point 4 (lane A39): a back
				// committed by an opponent's deke steps onto the line the puck's old side would have taken (it is beaten to the other post).
				const double Pv = W.PuckVel.Length();
				const bool bCommitted = W.DekeCommitFrames > 0 && W.DekeCommitSkater >= 0 && W.DekeCommitSkater < W.NumSkaters && W.Skaters[W.DekeCommitSkater].Team != S.Team;
				const FVec2 Pb = bCommitted ? Pp - W.DekeCommitOffset : Pp;
				if (SkillOf(W, Id).bBlockShots && Pp.DistTo(OwnGoal) >= SkillOf(W, Id).BlockMinDistM && Pv > 6.0 && W.PuckVel.Dot(OwnGoal - Pp) > 0.0)
				{
					const FVec2 Dir = W.PuckVel * (1.0 / Pv);
					const double Tg = std::fabs(Dir.X) > 1e-6 ? (OwnGoal.X - Pb.X) / Dir.X : -1.0;
					const double Yg = Pb.Y + Dir.Y * Tg;
					if (Tg > 0.0 && std::fabs(Yg) < W.GoalHalfWidth + 1.0)
					{
						double Along = (Me - Pb).Dot(Dir);
						Along = Along < 0.0 ? 0.0 : (Along > Tg ? Tg : Along);
						const FVec2 Block = Pb + Dir * Along;
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

	bool CreaseStance(const FJamAIWorld& W, int Id, int& OutSide)
	{
		OutSide = 0;
		if (Id < 0 || Id >= W.NumSkaters || W.bFrozen) { return false; }
		const FJamAISkater& S = W.Skaters[Id];
		if (S.bDown || W.Carrier == Id) { return false; }
		const int C = W.Carrier;
		if (C < 0 || W.Skaters[C].Team == S.Team) { return false; }
		const FVec2 OwnGoal = W.OwnGoal(S.Team);
		const FVec2 Cp = W.Skaters[C].Pos;
		// the carrier inside our blue line: the outer third of the rink on our side (LastManGoalie's rule)
		if (Cp.X * OwnGoal.X <= 0.0 || std::fabs(Cp.X) < W.HalfLength / 3.0) { return false; }
		// in the crease: within CreaseStanceM of the goal centre, on the ice side of the goal line
		if (S.Pos.DistTo(OwnGoal) > CreaseStanceM || std::fabs(S.Pos.X) > W.GoalX) { return false; }
		OutSide = std::fabs(Cp.Y) < 1.0 ? 0 : static_cast<int>(AICoreLocal::Sign(Cp.Y)) * (S.Team == 0 ? 1 : -1);
		return true;
	}

	void FJamBrain::StickPlay(const FJamAIWorld& W, int Id, FJamAIInput& In)
	{
		const FJamAISkater& S = W.Skaters[Id];
		const FJamBotSkill& K = SkillOf(W, Id);
		const int C = W.Carrier;
		const bool bOppCarrier = C >= 0 && W.Skaters[C].Team != S.Team;
		// R-0029 point 4 (lane A39): during an opponent's deke commit the blade and the poke follow the puck's old side: the back reads where the
		// puck was, not where it went, for deke.commit_frames (and the flight of a shot released inside the window).
		const bool bCommitted = W.DekeCommitFrames > 0 && W.DekeCommitSkater >= 0 && W.DekeCommitSkater < W.NumSkaters && W.Skaters[W.DekeCommitSkater].Team != S.Team;
		// Lane A51 (R-0034): against an opposing carrier the blade and the poke read the puck's place on the body SweepReactFrames late (PuckSeen), so a
		// sweep inside the stick's reach beats the poke; the deke commit (A39) wins over it while it runs.
		const FVec2 PuckRef = bCommitted ? W.PuckPos - W.DekeCommitOffset : PuckSeen(W, Id);
		const FVec2 To = PuckRef - S.Pos;
		const double D = To.Length();
		const double Blade = W.SkaterRadius + W.StickReach;
		// the blade follows the puck when it is near (zero aim = the sim's default, ahead at half reach)
		if (D > 1e-6 && D < Blade + 1.0)
		{
			// Lane A66: on its own puck inside its shooting range the bot draws the blade inside shot reach. Since A51 the sim carries the puck out to
			// the blade's full reach (0.4 + stick.reach_m 1.2 = 1.6 m, past shot.reach_m 1.5), and a blade aimed at the puck's own distance held it
			// there: carried, but beyond TryShoot's and the charge's reach, and the bot never shot (Jam.Bot.SeasonTakes take-3 / take-4 seed 1,
			// docs/experiments/STICK.md section 8). Outside the range the long carry stays as A51 made it (the passes to the human come from there).
			double MaxFrac = 1.0;
			if (W.PuckLastTouch == Id && !bOppCarrier && W.StickReach > 0.01 && (W.PuckPos.DistTo(W.OppGoal(S.Team)) < W.ShotReach + K.ShootExtraM || PassHold[Id] > 0))   // PassHold: lane A77, the saucer's hold needs the puck in pass reach
			{
				const double Cap = (W.ShotReach - 0.1 - W.SkaterRadius) / W.StickReach;
				if (Cap > 0.2 && Cap < MaxFrac) { MaxFrac = Cap; }
			}
			In.Stick = To * (1.0 / D) * Clamp((D - W.SkaterRadius) / (W.StickReach > 0.01 ? W.StickReach : 0.01), 0.2, MaxFrac);
		}
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
			In.bPass = In.bPass || (bPressure && bOpen);   // lane A46: Attack's pass to the human stays
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
		// R-0031 point 2 (lane A45; ends R-0017 point 4 for pro and all_star): the body check. Only on the opposing carrier, only when the closing speed along the
		// line to it and the bot's own speed along that line are both at least the skill's CheckMinClosing (the sim lands on the checker's own speed above
		// check_speed_min; the hit's strength is the closing speed), only from the front or the side (the push along the carrier's facing above 0.35 is left alone:
		// the sim's foul_from_behind band is 0.5, the margin covers the frames to contact), only when standing. One press per CheckCoolFrames: bCheck is a level in
		// the sim (held = re-armed and PlayCheck every frame). Pressed inside CheckArmGapM of contact: at 5 m/s that is the sim's 6-frame window. A skill with
		// CheckMinClosing < 0 (rookie) never presses, so no bot earns the box for a check on a skater without the puck or from behind.
		In.bCheck = false;
		if (CheckCool[Id] > 0) { --CheckCool[Id]; }
		if (K.CheckMinClosing >= 0.0 && bOppCarrier && !S.bDown && CheckCool[Id] <= 0 && !W.bFaceoffPhase)
		{
			constexpr int CheckCoolFrames = 30;
			constexpr double CheckArmGapM = 0.5;
			const FJamAISkater& T = W.Skaters[C];
			const FVec2 Rel = T.Pos - S.Pos;
			const double Dist = Rel.Length();
			if (Dist > 1e-6 && Dist <= 2.0 * W.SkaterRadius + CheckArmGapM && !T.bDown)
			{
				const FVec2 N = Rel * (1.0 / Dist);   // the push: checker towards the carrier
				const double Closing = (S.Vel - T.Vel).Dot(N);
				const double Own = S.Vel.Dot(N);
				const double Along = N.Dot(T.Facing.Normalized());
				// lane A72 (ask A5): game.ai.body_check takes a share of the chances; the coin is only thrown below 1, so a level's own rule is unchanged
				const bool bTake = K.BodyCheckShare >= 1.0
					|| (K.BodyCheckShare > 0.0 && (Hash11(static_cast<uint64_t>(Id) + 307, static_cast<uint64_t>(W.Frame / 60)) + 1.0) * 0.5 < K.BodyCheckShare);
				if (bTake && Closing >= K.CheckMinClosing && Own > K.CheckMinClosing && (K.bCheckFromBehind || Along <= 0.35))
				{
					In.bCheck = true;
					CheckCool[Id] = CheckCoolFrames;
				}
			}
		}
		// R-0019: never shoot from inside a no_shoot zone
		if (S.bNoShoot) { In.bShoot = false; }
	}

	FVec2 FJamBrain::Attack(const FJamAIWorld& W, int Id, const FVec2& Me, const FVec2& Pp, const FVec2& Dirg, double D, bool& bShoot, bool& bAimed, bool& bDeke, bool& bPass) const
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
		// R-0029 point 4 (lane A39): the one-timer. A pass just landed on the blade: shoot at once while the window is open, the net is in range and the
		// facing is inside the widened cone (the sim adds one_timer.cone_bonus_deg; the brain assumes the default 8 on top of the aim cone).
		if (K.bOneTimer && S.OneTimerFrames > 0 && S.bCanShoot && D < W.ShotReach && Dgoal < W.ShotReach + K.ShootExtraM + 10.0 && !bNearWall
			&& Fwd.Dot(Dirg) > std::cos(DegToRad((Params.AimConeDeg > 0.0 ? Params.AimConeDeg : 0.0) + 8.0)))
		{
			bShoot = true;
		}
		// The deke against the last man: the carrier inside 12 m of the net, facing it, with an opponent holding the crease on its line: one tap, no shot
		// until the puck has moved, then the shot (the sim aims it at the open post).
		if (K.bDeke && W.Carrier == Id)
		{
			if (S.DekeFrames > 0)
			{
				bShoot = S.bDekeSettled && S.bCanShoot && Fwd.Dot(Dirg) > std::cos(DegToRad(20.0));
			}
			else if (S.bCanDeke && Dgoal > 5.0 && Dgoal < 12.0 && Fwd.Dot(Dirg) > std::cos(DegToRad(20.0)))
			{
				for (int o = 0; o < W.NumSkaters; ++o)
				{
					const FJamAISkater& O = W.Skaters[o];
					if (O.Team == S.Team || O.bDown) { continue; }
					const FVec2 Rel = O.Pos - Pp;
					const double Along = Rel.Dot(Dirg);
					if (Along > 0.0 && Along < Dgoal && std::fabs(Rel.Cross(Dirg)) < 2.0 && O.Pos.DistTo(OppGoal) < 5.0) { bDeke = true; bShoot = false; break; }
				}
			}
		}
		// lane A3 / lane A46 (PAYOFFS.md section 6): a teammate carrying the puck passes to the human in a share of the chances when the human is open: standing,
		// 5-25 m away, ahead of or level with the puck up the ice, no opponent within 2 m of the lane (lane A3's lane check). The share is mate.pass_to_human
		// (rookie 0.6, pro 0.4, all_star 0.2); the human's `pass` signal (R-0007) makes it 0.9 while it runs. The coin is a fixed function of the skater and the
		// 2-second window (no randomness). The chaser and the support pass; the back only while the human calls `space` or `pass`. The pass itself is the sim's
		// (bPass: the lead solved for the receiver) when the sim would pick the human (W.CarrierBestPass); else lane A3's aimed tap at the human, from 8 m or more
		// (17.9 m/s minus puck.drag 0.5 per metre arrives under the 14 m/s cap of the body reception). A shot decided above (one-timer, aimed shot) comes first.
		const bool bPassIntent = TeamIntent.Kind == ETeamIntent::Pass && TeamIntent.Active(W.Frame);
		const bool bSpaceIntent = TeamIntent.Kind == ETeamIntent::Space && TeamIntent.Active(W.Frame);
		const double Share = bPassIntent ? 0.9 : K.PassToHuman;
		if (Share > 0.0 && W.PlayerIndex >= 0 && W.Skaters[W.PlayerIndex].Team == S.Team && W.Carrier == Id && S.bCanShoot && !bShoot
			&& (S.Role != ERole::Back || bPassIntent || bSpaceIntent))
		{
			const FJamAISkater& H = W.Skaters[W.PlayerIndex];
			const FVec2 To = H.Pos + H.Vel * 0.3 - Pp;
			const double Dh = To.Length();
			const double Coin = (Hash11(static_cast<uint64_t>(Id) + 101, static_cast<uint64_t>(W.Frame / 120)) + 1.0) * 0.5;   // [0, 1) per 2-second window
			const bool bAheadOrLevel = (H.Pos.X - Pp.X) * (OppGoal.X >= 0.0 ? 1.0 : -1.0) > -1.0;
			// set up for a shot: inside 26 m of the net and squared to it within 15 degrees (a one-timer position: PASSING.md section 2)
			const FVec2 HToGoal = OppGoal - H.Pos;
			const bool bSetUp = HToGoal.Length() < 26.0 && H.Facing.Normalized().Dot(HToGoal.Normalized()) > std::cos(DegToRad(15.0));
			// A set-up human is passed to in every chance (the pass every player makes); the share decides the other open chances.
			if ((bSetUp || (Coin < Share && bAheadOrLevel)) && Dh > 5.0 && Dh < 25.0 && !H.bDown)
			{
				bool bClear = true;
				for (int o = 0; o < W.NumSkaters && bClear; ++o)
				{
					if (W.Skaters[o].Team == S.Team) { continue; }
					const FVec2 Rel = W.Skaters[o].Pos - Pp;
					const double Along = Rel.Dot(To) / Dh;
					bClear = !(Along > 0.0 && Along < Dh && std::fabs(Rel.Cross(To) / Dh) < 2.0);
				}
				if (bClear)
				{
					if (W.CarrierBestPass == W.PlayerIndex) { bPass = true; }
					else if (Dh > 8.0 && Fwd.Dot(To * (1.0 / Dh)) > std::cos(DegToRad(15.0))) { bShoot = true; }
				}
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
