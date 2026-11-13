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

	// Lane A55: the scalar helpers JamAICore.cpp and JamPlaytester.cpp each kept a private copy of (same bodies, folded here).
	namespace AIMath
	{
		inline double Clamp(double V, double Lo, double Hi) { return V < Lo ? Lo : (V > Hi ? Hi : V); }
		inline double Sign(double V) { return V > 0.0 ? 1.0 : (V < 0.0 ? -1.0 : 0.0); }
		inline double DegToRad(double D) { return D * kPi / 180.0; }
		inline double RadToDeg(double R) { return R * 180.0 / kPi; }
		inline FVec2 Rotate(const FVec2& V, double Rad) { const double C = std::cos(Rad), S = std::sin(Rad); return FVec2(V.X * C - V.Y * S, V.X * S + V.Y * C); }
	}

	enum class ERole : uint8_t { Player, Chaser, Support, Back };

	// What one skater sends to the sim for one frame. Move is in the sim plane (x along the rink, y = cartridge z), |Move| <= 1.
	struct FJamAIInput
	{
		FVec2 Move;
		bool bShoot = false;
		bool bShootHeld = false;   // lane A34: shoot held (level) for the charged shot, as the pawn sends it (FJamInput::bShootHeld); bots send the edge above
		bool bCheck = false;
		bool bBrake = false;
		// R-0013 / R-0017 (lane A16's FJamInput): pass, stick aim (|Stick| <= 1 of stick reach, zero = no aim), poke, lift
		bool bPass = false;
		FVec2 Stick;
		bool bPoke = false;
		bool bLift = false;
		// Lane A31 (B-to-A-0024 point 1): the shot is charged. The brain holds this level for the skill's ShotChargeFrames while lined up on the
		// net with the puck, then drops it: the sim's StepCharge fires the charged shot on the release (bShoot stays false for that whole window).
		// Lane A31 (B-to-A-0024 point 3, R-0013 point 3): this bot is the last man holding the crease; GoalieSide -1 left, 0 centre, +1 right,
		// seen from the goalie facing up-ice (the side the carrier is on). The game mode turns it into C-0001 v3's SetGoalieStance.
		bool bGoalie = false;
		int GoalieSide = 0;
		bool bDeke = false;        // R-0029 point 4 (lane A39): the deke tap (FJamInput::bDeke); the adapter copies it like the others
		bool bSpin = false;        // lane A51 (R-0034): the spin tap (FJamInput::bSpin); the playtester sends it, the brain never does
		bool bSprint = false;      // lane A72 (asks A2, A3): sprint held (FJamInput::bSprint); the brain with game.ai.sprint, the playtester with -playersprint
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
		// R-0017 (phase 2): stick and pass use
		int PokeFrames = -1;           // poke once an opposing carrier's puck has been inside blade reach this many frames (-1 = never pokes)
		bool bLift = false;            // stick-lifts an opposing carrier at contact range
		bool bPassOut = false;         // passes (FJamAIInput::bPass) to an open teammate when a defender is within 2 m
		bool bReadLanes = false;       // the support cuts the pass lane between the opposing carrier and its nearest teammate
		bool bSeekPickups = false;
		// R-0013 draws (opposing centre only): DrawJumpPct of faceoffs it moves early in the countdown (loses the draw);
		// DrawPressPct of faceoffs it takes the draw DrawPressFrames after the drop (else the sim's own bot timing, 6-18 frames)
		int DrawJumpPct = 0;
		int DrawPressPct = 0;
		int DrawPressFrames = 8;
		// Director 15:16: an opposing carrier with the human's blade within reach this many consecutive frames gives the puck up
		// (a pass, else a dump shot a few frames later); -1 = never (rookie holds it and loses it to the forecheck)
		int ForecheckYieldFrames = -1;
		double CarrySpeed = 1.0;
		// R-0029 P1: the back's guard post, per side (it was FJamAIParams::GuardRadiusM / GuardRangeM for both teams): stand at most
		// GuardRadiusM out from the own net on the line to the puck, when the puck is in our half or within GuardRangeM of the net
		double GuardRadiusM = 5.0;
		double GuardRangeM = 45.0;       // top speed while carrying the puck, as a fraction (a carrier is slower than a chaser, so a forecheck closes)     // detours for a pickup within 8 m (never while carrying, never when the puck is nearer)
		bool bBlockShots = true;       // the back steps onto the path of a shot on its net
		double BlockMinDistM = 0.0;    // ... only of a shot taken at least this far from the net (rookie: long shots only)
		bool bClearLane = false;       // holds a shot while a defender stands in the lane puck -> goal (within 1.2 m of the line)
		// (the human's teammates only) lane A46 (mate.pass_to_human): the share of the chances the carrier passes to the human when the human is open (standing,
		// 5-25 m away, ahead of or level with the puck, no opponent within 2 m of the lane): rookie 0.6, pro 0.4, all_star 0.2; 0 = never. The human's `pass`
		// signal makes it 0.9 while it runs. The coin is a fixed function of the skater and the 2-second window. Lane A3's rule was a 0 | 1 flag.
		double PassToHuman = 0.0;
		// Lane A31 (B-to-A-0024 point 1): frames the carrier holds shoot before releasing an aimed shot on the net (the sim's charge rule
		// applies to every skater: speed + charge x bonus, charge = held / shot.charge_frames_max). 0 = a tap (rookie); pro 10, all_star 18.
		// Board chips, dumps under a forecheck and the pass-by-shot to the human stay taps.
		int ShotChargeFrames = 10;
		// R-0029 point 4 (lane A39): the payoffs the bots use. pro: the one-timer; all_star: the one-timer and the deke; rookie: neither.
		bool bOneTimer = false;        // shoots at once inside the one-timer window after a received pass (full charge, wider cone)
		bool bDeke = false;            // dekes the crease back inside 12 m, then shoots at the open post
		// Lane A51 (R-0034): the blade follows, and the poke aims at, where the puck sat on the carrier's body this many frames ago (the stick-handling
		// reaction; rookie 14, pro 8, all_star 5), unless game.stick.sweep.bot_react_frames names a number for every level. 0 = reads the puck live.
		int SweepReactFrames = 8;
		// R-0031 point 2 (lane A45): the body check. CheckMinClosing < 0 = this bot never body-checks (rookie, and R-0017 point 4 before this ruling); otherwise a one-frame
		// check press on the opposing carrier within reach when the closing speed along the line to it, and the bot's own speed along that line (the sim lands a check on
		// the checker's own speed above check_speed_min), are at least this (m/s; 5 = game.skater.check_speed_min's default). bCheckFromBehind false = never when the
		// push would run along the carrier's facing (the sim's foul_from_behind band, 0.5; the brain keeps a margin at 0.35), so no bot ever earns the box for it.
		double CheckMinClosing = -1.0;
		bool bCheckFromBehind = false;
		// Lane A72 (ask A5, game.ai.body_check): the share of the chances above that the bot takes; the coin is a fixed function of the skater and the
		// one-second window. 1 = every chance (the levels' own rule), 0 = never. Only a cartridge that writes game.ai.body_check changes it.
		double BodyCheckShare = 1.0;
	};
	constexpr int kMaxReactionFrames = 60;   // lane A72 (ask A4): game.ai.react_frames reaches one second (30 before; the ring is one entry longer than this)

	// lane A3: the opposing bots at pro (tools/sweep, docs/experiments/BALANCING.md): they shoot from reach + 30 m, know the
	// aim assist, and run to where the puck will be.
	inline FJamBotSkill ProOppSkill()
	{
		FJamBotSkill K;
		K.ShootExtraM = 13.0;  // 16:30 worktree sweep, 60 seeds: regular 55 / large 48 (30 before 16:18)
		K.LeadFrames = 60;
		K.DrawJumpPct = 100;   // 16:18: the pro centre never wins the draw outright (the sim's own bot timing still can); with it the human touches the puck inside 600 frames after every faceoff
		K.PokeFrames = 6;      // R-0017: pokes after the puck has sat 6 frames inside blade reach
		K.bLift = true;
		K.bPassOut = false;    // 15:13 sweep: pro passing kept the puck from the human past 600 frames after faceoffs (HumanHasSomethingToDo); all_star passes
		K.bReadLanes = true;
		K.ForecheckYieldFrames = 20;   // Director 15:16
		K.bIntercept = true;
		K.bUseAimCone = true;
		K.bOneTimer = true;    // R-0029 point 4 (lane A39): pro bots use the one-timer
		K.CheckMinClosing = 4.0;   // R-0031 point 2 (lane A45): pro bots body-check the carrier from the front or the side at this closing speed and up (lane A70: 5 -> 4, knockdowns of the regular playtester 1.74 -> 1.96 a match, its win share unmoved, CURVE-2.md)
		return K;
	}

	// game.ai plus the Godot ai.json constants that the cartridge does not carry.
	struct FJamAIParams
	{
		double YieldMarginM = 2.0;     // game.ai.yield_margin_m
		double YieldRadiusM = 15.0;    // game.ai.yield_radius_m
		double HoldRadiusM = 0.0;      // game.ai.hold_radius_m (lane A3: pro profile 0 on puck.drag 0.5; A2's 4.5 handed the human the puck)
		double SpeedScale = 1.0;       // game.ai.speed_scale
		double SprintShare = 0.0;      // lane A72 (ask A2): game.ai.sprint, the share of two-second windows a chasing bot sprints (0 = never)
		double RedSkill = 0.95;        // game.ai.red_skill (drives the side without the player); pro 0.95 by lane A70 (CURVE-2.md: regular playtester 50.6 % over 2,000 matches; 0.94 by R-0029 pass 2 measured 52.3 on main 7dfbeb5)
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
		bool bCanPoke = false;    // R-0017: poke ready (cooldown over, no open window, can act)
		bool bCanLift = false;    // R-0017: lift ready
		bool bCentre = false;     // R-0013: takes the faceoff draw
		bool bNoShoot = false;    // R-0019: standing inside a no_shoot zone (UJamZoneSystem::BlocksShot): never shoots from here
		// R-0029 point 4 (lane A39): the payoff windows (FJamSimAdapter::BuildWorld fills them from UJamSim's skater state)
		int OneTimerFrames = 0;   // frames left (including the next one) in which a shot is a one-timer; 0 = no received pass in the window
		bool bBreakaway = false;  // the burst runs
		int DekeFrames = 0;       // frames left of the deke commit window (0 = none)
		bool bDekeSettled = false; // the deked puck has moved to the side (the shot at the open post may go)
		bool bCanDeke = false;    // a deke tap would take (carrying, lockout over, the cut has the deke)
		FVec2 DekeOffset;         // the puck's sideways offset from the body line while deked; the crease back follows PuckPos - DekeOffset (the old side)
		// Lane A51 (R-0034): the spin: frames left (0 = none) and whether a spin tap would take (ready, standing, stamina left, the cut has the spin).
		int SpinFrames = 0;
		bool bCanSpin = false;
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
		// R-0035 (lane A52): the net's height and the lifted shot's rule (game.rink.goal_height_m, game.shot.lift, game.shot.charge_frames_max), so the
		// shooter's charge picks the height it arrives at: the full charge at a real net, half_frames under the 0.61 bar, a tap from close.
		double GoalHeightM = 0.61;
		double LiftTapM = 0.05;
		double LiftHalfM = 0.45;
		double LiftFullM = 0.95;
		int LiftHalfFrames = 8;
		int ChargeFramesMax = 30;
		// R-0007 / R-0009 (lane A12's signal seam): the human's active team signal, copied in by the game mode from
		// UJamSignalSystem::GetActiveIntent after BuildWorld; the brain reads only these. 0 none, 1 defend, 2 space, 3 pass, 4 praise.
		int32_t IntentKind = 0;
		int32_t IntentTarget = -1;      // skater index the intent is aimed at, -1 = every teammate
		int32_t IntentFramesLeft = 0;
		// R-0017: stick facts, the carrier, and the pickups lying on the ice (filled by the game mode from UJamEffectSystem; none headless)
		bool bFaceoffPhase = false;     // R-0013: the faceoff countdown runs (everyone holds; a centre that presses anything loses the draw)
		int FaceoffCountdown = 0;       // frames left of it
		bool bDrawOpen = false;         // R-0013: the faceoff draw is open (first clean stick input wins)
		int FramesSinceDrop = -1;       // frames since the drop while the draw is open (-1 unknown; set by FJamSimAdapter::View)
		int Carrier = -1;               // UJamSim::CarrierIndex: a skater that can act with the puck within shot reach, else -1
		int CarrierBestPass = -1;       // lane A46: the teammate UJamSim::BestPass would send the carrier's pass to right now (-1: no carrier or nobody open); the adapter fills it
		// R-0029 point 4 (lane A39): the goalie commit of the latest deke (UJamSim::GetDekeCommit): while it runs, the deker's opponents' blades, pokes
		// and shot blocks follow PuckPos - DekeCommitOffset (the puck's old side) instead of the puck.
		int DekeCommitFrames = 0;
		int DekeCommitSkater = -1;
		FVec2 DekeCommitOffset;
		double SkaterRadius = 0.4;
		double StickReach = 1.6;        // game.stick.reach_m
		double LiftRange = 1.0;         // game.stick.lift_range_m
		int SweepBotReactFrames = -1;   // lane A51: game.stick.sweep.bot_react_frames (-1 = absent: the level's FJamBotSkill::SweepReactFrames)
		static constexpr int kMaxPickups = 8;
		int NumPickups = 0;
		int PickupKind[kMaxPickups] = {};   // index of the kind in the cut's pickup list (the brain does not read it)
		FVec2 PickupPos[kMaxPickups];
		// R-0019: slow-ice zones as circles (rect zones by their half diagonal), from the game mode's UJamZoneSystem; none headless
		static constexpr int kMaxSlowIce = 8;
		int NumSlowIce = 0;
		FVec2 SlowIceCentre[kMaxSlowIce];
		double SlowIceRadius[kMaxSlowIce] = {};
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

	// R-0033 point 2 (lane A50): any skater covering its own crease takes the goalie stance, the human included (no dedicated goalies; the operator's
	// "last and any player acts like a goalie"). True when skater Id stands within CreaseStanceM of its own goal centre on the ice side of the goal line,
	// is not down and not the carrier, and an opposing carrier is inside its blue line (the outer third on its side). OutSide as the back's goalie intent:
	// where the carrier is, seen from the goalie facing up-ice (-1 left, 0 centre within a metre, +1 right). Pure: the brain applies it to every bot after
	// Decide (steering untouched), the game mode to the human on the same world.
	constexpr double CreaseStanceM = 3.0;
	JAMCORE_API bool CreaseStance(const FJamAIWorld& W, int Id, int& OutSide);
	// R-0035 (lane A52): frames a bot holds shoot for an aimed shot, from the net's height and the lift rule in W (0 = a tap): the full charge at a real net
	// (full_m clears the bar), else the profile's wind-up cut to the longest hold that clears the bar by 10 cm (0.61: 10 frames, 0.495 m), a tap inside 6 m;
	// a profile that never charges (ShotChargeFrames 0) taps; aim error mis-charges by up to that many frames. LiftAtFrames is UJamSim::LiftForCharge's rule.
	JAMCORE_API double LiftAtFrames(const FJamAIWorld& W, int F);
	JAMCORE_API int ChargeWindowFor(const FJamAIWorld& W, int Id, const FJamBotSkill& K);

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
		int GetDrawJumps() const { return DrawJumps; }
		int GetDrawPresses() const { return DrawPresses; }
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
		int InReach[kMaxSkaters] = {};
		int Pressed[kMaxSkaters] = {};   // frames the human's blade has been within reach of this carrier
		int CheckCool[kMaxSkaters] = {};   // R-0031 point 2 (lane A45): frames before this bot may press check again (one press arms the sim's 6-frame window)
		int DrawJumps = 0;               // diagnostics: early draw presses sent
		int DrawPresses = 0;   // R-0017: frames an opposing carrier's puck has been inside this bot's blade reach
		// Lane A31: frames shoot has been held for the current aimed shot (0 = not charging); the goalie intent of the last Decide per skater.
		int Charging[kMaxSkaters] = {};
		bool bGoalieNow[kMaxSkaters] = {};
		int GoalieSideNow[kMaxSkaters] = {};
		void StickPlay(const FJamAIWorld& W, int Id, FJamAIInput& In);
		bool LastManGoalie(const FJamAIWorld& W, int Id) const;
		// Lane A51 (R-0034): where the puck sat on the carrier's body, per frame (a ring of the last kPuckHist frames; recorded once per Think). A defending
		// bot reads the carrier's body live and the puck's offset on it SweepReactFrames late, so a sweep inside its stick reach beats the poke and a
		// straight carry is poked as before. PuckSeen() gives the delayed puck for Id's skill (the live puck when the carrier changed inside the delay).
		static constexpr int kPuckHist = 32;
		struct FPuckHist { FVec2 Rel; int Carrier = -1; };
		FPuckHist PuckHist[kPuckHist] = {};
		int64_t PuckHistFrame = -1;     // the frame of the latest entry (-1 none)
		int PuckHistCount = 0;
		void RecordPuck(const FJamAIWorld& W);
		FVec2 PuckSeen(const FJamAIWorld& W, int Id) const;

		// lane A3: reaction delay ring (input of the last kMaxReactionFrames + 1 thinks per skater)
		FJamAIInput Hist[kMaxSkaters][kMaxReactionFrames + 1] = {};
		int64_t Tick = 0;
		FJamAIIntent TeamIntent;
		int64_t PlayThinks = 0;       // thinks since play resumed (the drop)
		int Faceoffs = 0;             // faceoff countdowns seen
		int64_t FaceoffStart = 0;     // frame the current faceoff countdown began (the draw knobs hash on it, so they vary per match)
		bool bWasFaceoff = false;

		const FJamBotSkill& SkillOf(const FJamAIWorld& W, int Id) const;
		// bAimed (lane A31): the shot is the lined-up shot on the net (the one worth charging), not a board chip, a dump or a pass-by-shot.
		// bDeke (lane A39, R-0029 point 4): the carrier taps the deke this frame (live, like the shot).
		// bPass (lane A46): the carrier passes to the human this frame (FJamAIInput::bPass: the sim's pass with its lead solve).
		FVec2 Decide(const FJamAIWorld& W, int Id, bool& bShoot, bool& bAimed, bool& bDeke, bool& bPass);
		FVec2 Attack(const FJamAIWorld& W, int Id, const FVec2& Me, const FVec2& Pp, const FVec2& Dirg, double D, bool& bShoot, bool& bAimed, bool& bDeke, bool& bPass) const;
	};

	// Scripted input for the human's skater, used only by tests and tournaments (Godot bot_hockey.gd).
	struct FJamPlayerBot
	{
		double ShootRangeM = 40.0;   // bot_hockey.gd 40; bot_hockey_ai.gd 22
		bool bPeelToMiddle = true;   // bot_hockey.gd peels off the board toward the middle; bot_hockey_ai.gd pushes straight
		bool bBoardPeel = true;      // lane A2: peel off any board (end boards too); false = the Godot bot exactly
		int DrawReactionFrames = 12;
		bool bForecheckLead = false;  // lane A3: skate to where an opposing carrier's puck will be (16:08 sweep: off, it cost the human 25 points) // lane A3 (R-0013): the scripted human takes the draw this many frames after the drop (a person's reaction)
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
