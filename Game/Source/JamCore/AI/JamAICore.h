// Lane A2: the bot brain and the scripted player bot, engine-free (plain C++, no UObject) so the same source
// compiles inside JamCore and in the native tournament driver (tools/tournament/native). Port of the Godot pack
// third-life/site/ice-hockey-three-against-20261111-1 (scripts/ai/brain.gd, tests/bot_hockey*.gd).
//
// Laws: decisions read only the sim state handed in (FJamAIWorld); output is one FJamAIInput per skater; no
// randomness at all (the approach sway is a fixed function of frame and skater index); no wall clock.
// The only state the brain keeps is the stall detector's puck anchor, as in Godot.
//
// SEAM (lane A1): FJamAIWorld / FJamAIInput are this lane's view of UJamSim / FJamInput. JamBrainSimAdapter
// (to write when UJamSim lands) fills FJamAIWorld from the sim each frame and copies FJamAIInput into FJamInput.
#pragma once

#include <cmath>
#include <cstdint>

namespace JamAI
{
	constexpr int kMaxSkaters = 10;
	constexpr double kPi = 3.14159265358979323846;

	struct FVec2
	{
		double X = 0.0;
		double Y = 0.0;
		FVec2() = default;
		FVec2(double InX, double InY) : X(InX), Y(InY) {}
		FVec2 operator+(const FVec2& O) const { return FVec2(X + O.X, Y + O.Y); }
		FVec2 operator-(const FVec2& O) const { return FVec2(X - O.X, Y - O.Y); }
		FVec2 operator*(double S) const { return FVec2(X * S, Y * S); }
		FVec2 operator-() const { return FVec2(-X, -Y); }
		FVec2& operator+=(const FVec2& O) { X += O.X; Y += O.Y; return *this; }
		FVec2& operator-=(const FVec2& O) { X -= O.X; Y -= O.Y; return *this; }
		double Dot(const FVec2& O) const { return X * O.X + Y * O.Y; }
		// Godot Vector2.cross: a.x * b.y - a.y * b.x
		double Cross(const FVec2& O) const { return X * O.Y - Y * O.X; }
		double Length() const { return std::sqrt(X * X + Y * Y); }
		double DistTo(const FVec2& O) const { return (*this - O).Length(); }
		FVec2 Normalized() const { const double L = Length(); return L > 1e-12 ? FVec2(X / L, Y / L) : FVec2(); }
		FVec2 LimitLength(double M) const { const double L = Length(); return (L > M && L > 1e-12) ? *this * (M / L) : *this; }
		FVec2 Perp() const { return FVec2(-Y, X); }
	};

	enum class ERole : uint8_t { Player, Chaser, Support, Back };

	// What one skater sends to the sim for one frame. Move is in the sim plane (x along the rink, y = cartridge z), |Move| <= 1.
	struct FJamAIInput
	{
		FVec2 Move;
		bool bShoot = false;
		bool bCheck = false;
		bool bBrake = false;
	};

	// lane A3: how well one side's bots play. The defaults are A2's brain, which is what the human's teammates play at every
	// level; the opposing side's pro skill is ProOppSkill() below (DifficultyProfile.h holds the other levels).
	struct FJamBotSkill
	{
		int ReactionFrames = 0;        // the bot's steering is applied this many frames late (0..kMaxReactionFrames); shots stay live
		double AimErrorDeg = 0.0;      // the bot aims this far (at most) off the goal centre; a fixed function of skater and 2-second window
		double ShootExtraM = 13.0;     // shoot when dist_goal < reach + this (brief A2: reach + 13 m)
		double AimToleranceDeg = 20.0; // facing tolerance cap; the geometric one is atan(1.4 / dist_goal)
		int LeadFrames = 30;           // cap of the lead on a moving puck
		bool bIntercept = false;       // lead by an intercept solve (puck path vs own top speed) instead of dist / 8
		bool bUseAimCone = false;      // knows the sim's aim assist: facing inside shot.aim_cone_deg counts as on target
		bool bBlockShots = true;       // the back steps onto the path of a shot on its net
		double BlockMinDistM = 0.0;    // ... only of a shot taken at least this far from the net (rookie: long shots only)
		bool bClearLane = false;       // holds a shot while a defender stands in the lane puck -> goal (within 1.2 m of the line)
		bool bPassToHuman = false;     // (the human's teammates only) with the puck and facing a free human 5-25 m away: pass by shooting
	};
	constexpr int kMaxReactionFrames = 30;

	// lane A3: the opposing bots at pro (tools/sweep, docs/experiments/BALANCING.md): they shoot from reach + 30 m, know the
	// aim assist, and run to where the puck will be.
	inline FJamBotSkill ProOppSkill()
	{
		FJamBotSkill K;
		K.ShootExtraM = 30.0;
		K.LeadFrames = 60;
		K.bIntercept = true;
		K.bUseAimCone = true;
		return K;
	}

	// game.ai plus the Godot ai.json constants that the cartridge does not carry.
	struct FJamAIParams
	{
		double YieldMarginM = 2.0;     // game.ai.yield_margin_m
		double YieldRadiusM = 15.0;    // game.ai.yield_radius_m
		double HoldRadiusM = 0.0;      // game.ai.hold_radius_m (lane A3: pro profile 0 on puck.drag 0.5; A2's 4.5 handed the human the puck)
		double SpeedScale = 1.0;       // game.ai.speed_scale
		double RedSkill = 1.0;         // game.ai.red_skill (drives the side without the player)
		// lane A3: skill per side. Opp = the side without the human (team 1 in pure bot-vs-bot); Mate = the human's side.
		FJamBotSkill Mate;
		FJamBotSkill Opp = ProOppSkill();
		// lane A3: sim facts the skills read (filled from the cartridge by the match config / resolver)
		double InterceptSpeed = 8.0;   // skater.speed (m/s) for the intercept solve
		double AimConeDeg = 0.0;       // shot.aim_cone_deg, 0 = no assist
		// Godot ai.json, not in the cartridge:
		double MinSeparation = 4.0;
		double PounceRadius = 5.0;
		double BackRadius = 6.0;
		double GuardRangeM = 45.0;     // lane A3: ... and also when the puck is this near its net (a centre-ice shot on 90 x 45)
		double GuardRadiusM = 5.0;     // lane A3: the back's guard post when the puck is in its half (<= ~6 m from its own net)
		double SwayM = 1.5;
		double SeparateRadius = 1.5;
		double ArriveSlowRadius = 1.5;
		int StallFrames = 60;          // lane A3: pro profile (A2: 120)
		double StallMoveM = 2.0;
		// lane A2: until anyone touches the puck after a faceoff, opposing non-backs hold (the human takes the opening touch)
		bool bFaceoffCourtesy = true;
	};

	struct FJamAISkater
	{
		FVec2 Pos;
		FVec2 Vel;
		FVec2 Facing{1.0, 0.0};   // unit
		FVec2 Spawn;
		int Team = 0;             // 0 or 1; team 0 defends -x
		ERole Role = ERole::Chaser;
		bool bCanShoot = false;   // the sim's shot gate: in reach, facing within 60 deg, cooldown over
		bool bHasPuck = false;
		bool bDown = false;       // knocked down
	};

	// Read-only view of the simulation for one frame.
	struct FJamAIWorld
	{
		int64_t Frame = 0;
		bool bFrozen = false;     // goal pause, match over
		FVec2 PuckPos;
		FVec2 PuckVel;
		int PuckLastTouch = -1;   // skater that touched the puck last, -1 since the faceoff
		int NumSkaters = 0;
		int PlayerIndex = -1;     // the human's skater (-1 = none)
		FJamAISkater Skaters[kMaxSkaters];
		double HalfLength = 60.0;
		double HalfWidth = 30.0;
		double CornerRadius = 5.0;
		double GoalX = 56.0;      // |x| of the goal lines (length/2 - 4)
		double GoalHalfWidth = 2.0;
		double ShotReach = 1.5;
		// R-0007 / R-0009 (lane A12's signal seam): the human's active team signal, copied in by the game mode from
		// UJamSignalSystem::GetActiveIntent after BuildWorld; the brain reads only these. 0 none, 1 defend, 2 space, 3 pass, 4 praise.
		int32_t IntentKind = 0;
		int32_t IntentTarget = -1;      // skater index the intent is aimed at, -1 = every teammate
		int32_t IntentFramesLeft = 0;
		// Own goal of team t: (t == 0 ? -GoalX : +GoalX, 0).
		FVec2 OwnGoal(int Team) const { return FVec2(Team == 0 ? -GoalX : GoalX, 0.0); }
		FVec2 OppGoal(int Team) const { return OwnGoal(1 - Team); }
	};

	// R-0007: the brain's view of the human's team signal (FJamTeamIntent in JamGame/Public/SignalIntent.h, lane A12), rebuilt
	// every Think from FJamAIWorld::IntentKind / IntentTarget / IntentFramesLeft. Consumption rules: docs/experiments/ACE.md section 2.
	enum class ETeamIntent : uint8_t { None, Defend, Space, Pass, Praise };
	struct FJamAIIntent
	{
		ETeamIntent Kind = ETeamIntent::None;
		int64_t IssuedFrame = 0;
		int Frames = 0;
		int Target = -1;          // praise: the praised teammate (skater index), -1 = every teammate
		bool Active(int64_t Frame) const { return Kind != ETeamIntent::None && Frame >= IssuedFrame && Frame < IssuedFrame + Frames; }
	};

	// The brain for every non-player skater. One instance per match (holds only the stall anchor).
	class FJamBrain
	{
	public:
		FJamAIParams Params;
		bool bBoardPeel = true;   // false = the Godot brain exactly

		void Reset();
		// R-0007: the intent read from the world on the last Think (search "TeamIntent" in JamAICore.cpp): defend = teammates fall back
		// in front of the own net, space = spread to free ice, pass = the carrier passes to the human, praise = +10% speed, no reaction delay.
		const FJamAIIntent& GetTeamIntent() const { return TeamIntent; }
		// Fills Out[i] for every skater that is not the player; Out[PlayerIndex] is left untouched.
		void Think(const FJamAIWorld& W, FJamAIInput* Out);
		bool IsStalled() const { return bStalled; }
		int StallCount() const { return Stalls; }
		// Role the brain used for skater i on the last Think (after yields and stall promotion).
		ERole LastRole(int i) const { return Used[i]; }

	private:
		FVec2 Anchor{1e6, 0.0};
		int64_t AnchorFrame = 0;
		bool bStalled = false;
		int Stalls = 0;
		int StallChaser[2] = {-1, -1};
		ERole Used[kMaxSkaters] = {};

		// lane A3: reaction delay ring (input of the last kMaxReactionFrames + 1 thinks per skater)
		FJamAIInput Hist[kMaxSkaters][kMaxReactionFrames + 1] = {};
		int64_t Tick = 0;
		FJamAIIntent TeamIntent;

		const FJamBotSkill& SkillOf(const FJamAIWorld& W, int Id) const;
		FVec2 Decide(const FJamAIWorld& W, int Id, bool& bShoot);
		FVec2 Attack(const FJamAIWorld& W, int Id, const FVec2& Me, const FVec2& Pp, const FVec2& Dirg, double D, bool& bShoot) const;
	};

	// Scripted input for the human's skater, used only by tests and tournaments (Godot bot_hockey.gd).
	struct FJamPlayerBot
	{
		double ShootRangeM = 40.0;   // bot_hockey.gd 40; bot_hockey_ai.gd 22
		bool bPeelToMiddle = true;   // bot_hockey.gd peels off the board toward the middle; bot_hockey_ai.gd pushes straight
		bool bBoardPeel = true;      // lane A2: peel off any board (end boards too); false = the Godot bot exactly
		FJamAIInput Think(const FJamAIWorld& W) const;
	};

	// Rounded-rectangle board distance (inside negative) and outward normal, as UJamSim::BoardDistance.
	double BoardSdf(const FJamAIWorld& W, const FVec2& P, FVec2& OutNormal);
	// Direction that takes a puck lying within 1.6 m of the boards off them: along the board (towards +x when
	// AttackSign > 0, towards the nearer corner on an end board) and 45 degrees inwards. Returns false when the puck is free.
	bool BoardPeel(const FJamAIWorld& W, const FVec2& Puck, double AttackSign, FVec2& OutDir);

	// Steering helpers (TLSteer.arrive / separate).
	FVec2 Arrive(const FVec2& From, const FVec2& Target, double SlowRadius);
	FVec2 Separate(const FVec2& From, const FJamAIWorld& W, int Self, double Radius);
}
