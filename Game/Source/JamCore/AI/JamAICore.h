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
		bool bPassHeld = false;    // lane A77 (R-0036): pass held (FJamInput::bPassHeld) for the saucer: held W.SaucerHalfFrames then released; the sim lifts the pass
		bool bSprint = false;      // lane A72 (asks A2, A3): sprint held (FJamInput::bSprint); the brain with game.ai.sprint, the playtester with -playersprint
		bool bAbility = false;     // lane A84 (R-0041 point 3): the held ability on F (FJamInput::bAbility); the playtester sends it, the brain never does
	};

	// Lane A94 (R-0045 law 2, docs/VISION.md section 2, docs/experiments/BRAIN.md): the hockey principles. Every level knows them; the level decides how
	// well a bot executes them (ShapeShare: loosely; LateFrames: late). All off (the default) is the brain before A94, bit for bit. Only teams of five or
	// more play the shapes (the regular game); the carry-or-pass read runs for any carrier bot that has it on.
	struct FJamHockeySense
	{
		bool bCarryOrPass = false;     // the carrier reads the lane ahead and its teammates: carries while open, passes under pressure, to a better-placed teammate, or into a wall
		double PressureSec = 1.0;      // pass when the first opponent on the lane ahead arrives inside this many seconds
		int MinCarryFrames = 12;       // a fresh carrier carries at least this long before passing to a better-placed teammate (pressure and the wall pass at once)
		double BetterByM = 5.0;        // an open teammate this much nearer the opposing net than the carrier is better placed
		bool bAttackShape = false;     // without the puck in attack: a close support, a far-side / net-front skater, the backs at the blue line, out of the lanes
		bool bDefenceShape = false;    // one presses goal-side with gap control, the next covers the nearest pass, the rest slot and far side; nobody chases behind the play
		bool bTransition = false;      // on a change of possession the nearest two react first (the puck and the lane) for TransitionFrames
		int TransitionFrames = 90;
		double ShapeShare = 1.0;       // the share of two-second windows a bot keeps its shape job (a fixed coin per bot and window); the rest it plays the job loosely (an off spot, three quarters pace)
		int LateFrames = 0;            // the bot reads the change of possession this many frames late (0..kPhaseHist-1)
		bool bReadHumanPressure = true; // an opposing carrier passes away from the human's pressure too (false: the human's forecheck is left to the old yield rule)
		// Lane A115 (track Q2, evolution 2; docs/experiments/BRAIN-2.md): the brain plays hockey with the human. Every field off (the default) is the brain of
		// evolution 1, bit for bit (proved: the env switch JAM_BRAIN2_OFF=all replays 200 baseline matches hash for hash). ProHockeySense() turns them on.
		bool bPassRoom = false;        // a mate passes to the human where he has room (no opponent within 3 m; 2 m when he is set up for a one-timer), and turns to him when he is open and ahead of the play
		bool bSupportHuman = false;    // the human's side supports him: a mate offers an open lane when he carries, and does not stand off an opposing carrier he forechecks
		bool bChaseAhead = false;      // the loose puck's chaser is the skater that reaches where the puck will be first; the passer is not the chaser of its own pass; no yield to a human who is down
		bool bBrakeMeant = false;      // the reverse brake is a decision (a stop on the post, a turn back for a loose puck, a stop in front of a defender, a gap closed under control); any other reversal is carved into a turn
		bool bStopCut = false;         // (with bBrakeMeant) a carrier that runs at more than 5 m/s into a defender inside 4 m on its lane, with no pass on, stops before it cuts
		bool bCarrySprint = false;     // a bot carrier sprints while its lane is open (nobody on the lane 15 m ahead, nobody within 6 m)
		bool bGapControl = false;      // the presser closes under control: goal-side of a carrier that comes at it, it stops and waits on the line to the net, facing it, and steps up inside 3.5 m (the head-on charge is the old rule)
		// The stick of the human's mates (his side only; the opposing side keeps FJamBotSkill::PokeFrames / bLift): a mate whose skill has no poke pokes an
		// opposing carrier once its puck has sat this many frames inside the blade's reach, and lifts at contact at most every 90 frames. -1 = never (the
		// mates of evolution 1 have no stick verb: they won a puck only by the unpressed blade, which track F2's game.stick.passive_take false ends).
		int MateStickFrames = -1;
		bool bReadShield = false;      // reads FJamAIWorld::bBodyShield and FJamAISkater::TakeGraceFrames (track F2's body shield and take grace): no lift or poke the sim would refuse; the bot goes round the body, or contains during the grace
		// Lane A138 (track I1, R-0053: "a defender that contains goal-side wins when the carrier dawdles, not when he stops and goes"). The shield read's way
		// round the body (Attack): the point beside the carrier is led by the carrier's velocity for RoundLeadS seconds (0.3 = lane A115's rule: the bot stays
		// level with a carrier that goes and is never passed); and the bot goes round only while the carrier is slower than RoundMaxMps (0 = at any speed, lane
		// A115's rule): a carrier that goes is chased, not shadowed. Tune keys hk_round_lead_s, hk_round_max_mps.
		// Measured (INTEGRATION.md section 2): with 3 m/s (the Press job's own "slow carrier") a stop begun at 9.5 m passes a pro 97 % (72.5 % at 0) and one
		// begun at 11 m 86 % (25.5 %); the stop at 8 m and the straight charge do not move. Only read with bReadShield on.
		double RoundLeadS = 0.3;
		double RoundMaxMps = 3.0;
		// The spin's exposed end (lane A128: game.skater.spin.exposed_frames): the share of spins whose end this bot answers with its lift (a fixed coin per
		// bot and spin, as BodyCheckShare). 1 = every one (the spin with the cut from 6 m passes a pro 27 %), 0 = none (98 %: the spin's price unpaid).
		// Tune key hk_spin_end_share. Only read with bReadShield on (the brain of evolution 1 swings at a spin blind).
		double SpinEndShare = 0.5;
		// Lane A134 (track Q2, evolution 3; docs/experiments/BRAIN-3.md): the human's side takes a kept puck back. Every field at its default is the brain
		// of lane A138's tree, bit for bit (the tune key brain3=0; brain2=0 switches these off too). SetEvolution3() turns them on, per side.
		// YieldArriveS: a loose puck is left to the human by arrival, not by distance: a mate yields only while the human reaches the puck no later than
		// this many seconds after the mate itself would (ArriveSec: his real velocity and facing, a turn costs time). -1 = the distance rule (yield_margin_m).
		double YieldArriveS = -1.0;
		// BackcheckMps (a team without shape jobs: fewer than five): a bot that plays the puck against an opposing carrier faster than this, and is not
		// goal-side of it, skates for the carrier's lane to our net (the first point on it that it reaches before the carrier), not for the puck behind the
		// carrier's body, where the body shield refuses its stick. Goal-side again, it meets the carrier from the front. 0 = it chases the puck.
		double BackcheckMps = 0.0;
		// StepUpM (a team without shape jobs): the back goes at an opposing carrier in its own half that comes within this many metres of it, from the
		// front (it is goal-side by its post), instead of waiting for a puck inside 2.5 m. 0 = it holds its post.
		double StepUpM = 0.0;
		// MateCheckClosing: the human's mates body-check an opposing carrier from the front or the side at this closing speed and up (the rules of
		// FJamBotSkill::CheckMinClosing, which the mates' skill does not have). -1 = a mate never checks.
		double MateCheckClosing = -1.0;
		// ... only in a team of fewer than MateCheckMaxTeam skaters. 5 (as shipped): three and four a side, where no second defender's stick is near (the
		// big rinks); the regular game's five keep their mates' rule, so no cell of the ladder moves. 99 = every team (tune key hk_mate_check_max_team;
		// measured at five a side: regular at pro 67.8 -> 71.0 %, BRAIN-3.md).
		int MateCheckMaxTeam = 5;
		// ShotMaxAngleDeg: no aimed shot and no one-timer from further off the net's axis than this (the corner by the goal line, where the mouth is a
		// slit): the carrier takes the puck to the slot, or passes. 0 = a bot shoots from anywhere in range.
		double ShotMaxAngleDeg = 0.0;
		// PincerM (a team without shape jobs, the human's mates): while the human is within this many metres of an opposing carrier, the mate that plays
		// the puck comes at the carrier from the other side (the point 1.4 m beyond the carrier on the line human -> carrier), so the body cannot shield
		// the puck from both. 0 = it takes the puck's goal-side, wherever the human is.
		double PincerM = 0.0;
		// bPressInHunt (measured, ships off: BRAIN-3.md section 6): the presser of a five keeps its Press job (the gap in front of a carrier that goes) inside
		// its skill's HuntRadiusM. Off, as on every tree so far: the hunt takes every bot inside the radius off its shape job, the presser included, so the
		// hunt radius is also the distance at which the presser stops closing under control and charges (it moves the stop-and-go's window: R-0055 point 1).
		bool bPressInHunt = false;
		// Lane A153 (track Q2, evolution 4; docs/experiments/BRAIN-4.md). Both at their neutral value are the brain of evolution 3, bit for bit (the tune
		// key brain4=0, which also switches a level's bPressInHunt off: all_star has it on since this lane, R-0058 point 1).
		// MateFinishM (the human's mates): a mate that carries the puck inside this many metres of the opposing net, no further off its axis than 60
		// degrees, with an open lane (no standing opponent within 1.2 m of the line puck -> net up to 3 m in front of it: the last man in his crease does
		// not close a lane) finishes: it knows the sim's aim assist there (facing inside shot.aim_cone_deg counts as lined up) and it does not give the
		// puck to a better-placed mate or to the human by its coin; a human set up for the one-timer still gets it, and pressure still moves the puck.
		// 0 = a mate shoots only when lined up to atan(1.4 / distance) and inside its skill's range (REVIEW-2 item 10: 0.4 releases a match from inside 15 m).
		double MateFinishM = 0.0;
		// MateDriveM (the human's mates): the same open lane out to this distance: the mate takes the puck to the net itself. It does not hand the puck
		// to the human or to a better-placed mate on the way (a human set up for the one-timer still gets it, pressure still moves the puck); inside
		// MateFinishM it shoots. 0 = it gives the puck to an open human after MinCarryFrames wherever it is (measured: a mate is the carrier inside 15 m
		// of the net for 53 frames a match at pro, in three spells of a third of a second: it is never there long enough to finish).
		double MateDriveM = 0.0;
		// ScreenFrames (the human's mates): for this many frames after the human takes the puck, the mate nearest the opponent nearest him (within 8 m of
		// him) stands in that opponent's way, 1.2 m from it on the line to the human, instead of offering a lane: the human's first second with the puck
		// is covered (REVIEW-2: the puck is still his two seconds after a mate's pass in 19 % of the cases at pro). 0 = every mate offers a lane.
		int ScreenFrames = 0;
		// CarryClockS (a team without shape jobs: fewer than five): a carrier that has held the puck this many seconds attacks or moves it: it passes when
		// the sim's pass has a receiver, else shoots it at the net when it faces up the ice (as a stalled puck is shot away). 0 = it may carry for ever.
		double CarryClockS = 0.0;
		bool Any() const { return bCarryOrPass || bAttackShape || bDefenceShape || bTransition; }
	};
	// Lane A115: every rule of evolution 2 on (as ProHockeySense has them) or off (the brain of evolution 1). The tune key brain2 and the tests use it.
	inline void SetEvolution2(FJamHockeySense& H, bool bOn)
	{
		H.bPassRoom = H.bSupportHuman = H.bChaseAhead = H.bBrakeMeant = H.bStopCut = H.bCarrySprint = H.bReadShield = bOn;
		H.MateStickFrames = bOn ? 10 : -1;
		H.bGapControl = false;   // built and measured, not switched on by the profile (BRAIN-2.md): the tune keys opp.hk_gap_control=1;mate.hk_gap_control=1 play it
	}
	// Lane A134 (evolution 3, BRAIN-3.md): the rules of evolution 3 as shipped (ApplyDifficultyProfile calls it for each side) or off (lane A138's tree).
	// bMate: the human's side (the rules that take a kept puck back are his mates'; the opposing side keeps its approach: law 3's drill is measured on it).
	// The values are BRAIN-3.md's: a rule that is 0 / -1 here was measured and ships at its neutral value (its tune key plays it).
	constexpr double kEvo3YieldArriveS = -1.0;
	constexpr double kEvo3BackcheckMps = 0.0;
	constexpr double kEvo3StepUpM = 10.0;
	constexpr double kEvo3MateCheckClosing = 2.0;
	constexpr double kEvo3ShotMaxAngleDeg = 0.0;
	constexpr double kEvo3PincerM = 0.0;
	inline void SetEvolution3(FJamHockeySense& H, bool bOn, bool bMate)
	{
		H.YieldArriveS = -1.0;
		H.BackcheckMps = 0.0;
		H.StepUpM = 0.0;
		H.MateCheckClosing = -1.0;
		H.MateCheckMaxTeam = 5;
		H.ShotMaxAngleDeg = 0.0;
		H.PincerM = 0.0;
		// bPressInHunt is a level's switch (BRAIN-3.md section 6: it suits all_star's approach and breaks pro's): on, this leaves it as the level's
		// profile set it (FJamDifficultyProfile::Opp.Hk.bPressInHunt; no level sets it today); off (brain3=0) switches it off with the rest.
		if (!bOn) { H.bPressInHunt = false; return; }
		if (bMate)
		{
			H.YieldArriveS = kEvo3YieldArriveS;
			H.BackcheckMps = kEvo3BackcheckMps;
			H.StepUpM = kEvo3StepUpM;
			H.MateCheckClosing = kEvo3MateCheckClosing;
			H.PincerM = kEvo3PincerM;
		}
		H.ShotMaxAngleDeg = kEvo3ShotMaxAngleDeg;
	}
	// Lane A153 (evolution 4, BRAIN-4.md): the rules of evolution 4 as shipped (ApplyDifficultyProfile calls it for each side) or off (evolution 3).
	// A rule that is 0 here was measured and ships at its neutral value (its tune key plays it). Off also switches a level's bPressInHunt off.
	constexpr double kEvo4MateFinishM = 0.0;
	constexpr double kEvo4CarryClockS = 0.0;
	constexpr double kEvo4MateDriveM = 0.0;
	constexpr int kEvo4ScreenFrames = 0;   // neutral tonight (the Director, 03:36: the ladder is being measured on the landing). Recommended: 30 (half a second: rookie +1.5, pro +2.2, all_star -1.4 at 600; BRAIN-4.md section 3)
	inline void SetEvolution4(FJamHockeySense& H, bool bOn, bool bMate)
	{
		H.MateFinishM = 0.0;
		H.CarryClockS = 0.0;
		H.MateDriveM = 0.0;
		H.ScreenFrames = 0;
		if (!bOn) { H.bPressInHunt = false; return; }
		if (bMate) { H.MateFinishM = kEvo4MateFinishM; H.MateDriveM = kEvo4MateDriveM; H.ScreenFrames = kEvo4ScreenFrames; }
		else { H.CarryClockS = kEvo4CarryClockS; }
	}
	// Lane A94: the sense each level plays (DifficultyProfile.cpp sets the opponents' per level; the human's teammates play ProHockeySense at every level).
	inline FJamHockeySense ProHockeySense()
	{
		FJamHockeySense H;
		H.bCarryOrPass = true;
		H.bAttackShape = true;
		H.bDefenceShape = true;
		H.bTransition = true;
		H.ShapeShare = 0.85;
		H.LateFrames = 8;
		H.PressureSec = 1.4;   // measured (BRAIN.md): 1.0 s with better-placed 2 m gave 11.7 passes/min but the human's touches -2.3; 1.4 s and 3 m: 11.6 and -1.7
		H.BetterByM = 3.0;
		H.bReadHumanPressure = false;   // pro and rookie leave the human's forecheck to ForecheckYieldFrames (the human's touches, brief A94 point 5)
		// Lane A115 (evolution 2, BRAIN-2.md). The revert path: set these to false (each alone gives back its rule of evolution 1).
		// The same from a profile tune: brain2=0 (both sides), or one rule: opp.hk_pass_room=0, mate.hk_mate_stick_frames=-1, ... (DifficultyProfile.cpp).
		SetEvolution2(H, true);
		return H;
	}

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
		// R-0041 point 1 (lane A79): all_star bots hunt the carrier. A support bot within this many metres of an opposing carrier leaves its lane and goes at
		// it as a second chaser (stick, lift and body check as the chaser does); 0 = never (the brain's PounceRadius on the puck only), so rookie and pro do not move.
		double HuntRadiusM = 0.0;
		// Lane A94 (R-0045 law 2): the hockey principles (all off = the brain before A94)
		FJamHockeySense Hk;
		// Lane A95 (R-0045 law 4, VISION.md "a defender is always faster"): the share of two-second windows this side's bot sprints while it chases the puck or
		// an opposing carrier from more than 6 m without carrying it (A72's rule, per side and level now). 0 = never (the default: A72's absent ai.sprint).
		// game.ai.sprint, when the cartridge writes it, replaces it on both sides (FJamAIParams::SprintShare >= 0).
		double SprintShare = 0.0;
		// Lane A95 (B-to-A-0037 point 3): frames between two lift presses of one bot (B saw two figures trade cross-checks 2 to 10 frames apart for a whole match).
		// The sim's own lift cooldown is 30 frames; 0 = the sim's alone (the brain before A95). Per level: rookie 0 (never lifts), pro 90, all_star 45.
		int LiftCooldownFrames = 0;
	};
	constexpr int kMaxReactionFrames = 60;   // lane A72 (ask A4): game.ai.react_frames reaches one second (30 before; the ring is one entry longer than this)

	// lane A3: the opposing bots at pro (tools/sweep, docs/experiments/BALANCING.md): they shoot from reach + 30 m, know the
	// aim assist, and run to where the puck will be.
	inline FJamBotSkill ProOppSkill()
	{
		FJamBotSkill K;
		K.ShootExtraM = 30.0;  // lane A95 (CURVE-5.md): 13 -> 30, the pro bot shoots from reach + 30 m (A94's carry-or-pass halved its shots; the single biggest lever, regular 82 -> 67). 16:30 sweep: 13
		K.LeadFrames = 60;
		K.DrawJumpPct = 100;   // 16:18: the pro centre never wins the draw outright (the sim's own bot timing still can); with it the human touches the puck inside 600 frames after every faceoff
		K.PokeFrames = 2;      // lane A112 (CURVE-6.md): 6 -> 2 (the regular player on the kept puck 71.2 -> 60.8 alone). R-0017: pokes after the puck has sat 6 frames inside blade reach
		K.bLift = true;
		K.bPassOut = false;    // 15:13 sweep: pro passing kept the puck from the human past 600 frames after faceoffs (HumanHasSomethingToDo); all_star passes
		K.bReadLanes = true;
		K.ForecheckYieldFrames = 20;   // Director 15:16
		K.bIntercept = true;
		K.bUseAimCone = true;
		K.bOneTimer = true;    // R-0029 point 4 (lane A39): pro bots use the one-timer
		K.Hk = ProHockeySense();   // lane A94 (R-0045 law 2)
		// Lane A133 (CURVE-7.md): the pro carrier carries through the forecheck as all_star's does: it moves the puck under pressure only when the first
		// opponent arrives inside 0.3 s (1.4) and passes only to a mate 12 m nearer the net (3). The regular player 74.0 -> 51.0 (600; hunt 8); law 3's drill
		// does not see it (100 % at 8 m). The human's mates keep ProHockeySense's 1.4 / 3 (ApplyDifficultyProfile).
		K.Hk.PressureSec = 0.3;
		K.Hk.BetterByM = 12.0;
		K.SprintShare = 0.35;   // lane A95 (R-0045 law 4): a pro bot sprints in a third of its chases (rookie 0.1, all_star 0.7)
		K.LiftCooldownFrames = 90;   // lane A95 (B-to-A-0037 point 3): a pro bot lifts at most every 1.5 s
		K.HuntRadiusM = 8.0;   // lane A133 (CURVE-7.md, R-0055 point 1: the hunt is not a curve lever): 20 -> 8 m (law 3's drill at pro: the stop at 8 m 21.0 % at 20 m, 70.5 % at 10, 98.5 % at 9, 100 % at 8). Lane A112: 8 -> 20. (CURVE-6.md, R-0050: the human keeps the puck): 8 -> 20 m with the poke 2 and the checks from 1: the regular player 71.2 -> 50.2 (2,000; with sprint 0.7 and lift cooldown 45 too: 44.5). Lane A95: 0 -> 8. (R-0041 "pro bots check and lift as a matter of course", CURVE-5.md): the support goes at an opposing carrier within 8 m (0 before)
		K.CheckMinClosing = 1.0;   // lane A112 (CURVE-6.md): 2 -> 1 m/s. Lane A95 (CURVE-5.md): 4 -> 2 m/s: under A92's momentum model a low-speed press is a stumble, not a knockdown. R-0031 point 2 (lane A45): pro bots body-check the carrier from the front or the side at this closing speed and up (lane A70: 5 -> 4, knockdowns of the regular playtester 1.74 -> 1.96 a match, its win share unmoved, CURVE-2.md)
		return K;
	}

	// game.ai plus the Godot ai.json constants that the cartridge does not carry.
	struct FJamAIParams
	{
		double YieldMarginM = 2.0;     // game.ai.yield_margin_m
		double YieldRadiusM = 15.0;    // game.ai.yield_radius_m
		double HoldRadiusM = 0.0;      // game.ai.hold_radius_m (lane A3: pro profile 0 on puck.drag 0.5; A2's 4.5 handed the human the puck)
		double SpeedScale = 1.0;       // game.ai.speed_scale
		double MateSpeedScale = 0.0;   // lane A112 (R-0048 point 4): the human's teammates' top speed as a fraction of skater.speed when a human plays; 0 = SpeedScale (every bot's: the game before)
		double SprintShare = -1.0;     // lane A72 (ask A2): game.ai.sprint, the share of two-second windows a chasing bot sprints (0 = never); lane A95: -1 (absent) = each side's FJamBotSkill::SprintShare (the level's)
		double RedSkill = 1.07;        // lane A95 (CURVE-5.md): pro 0.95 -> 1.07. game.ai.red_skill (drives the side without the player); pro 0.95 by lane A70 (CURVE-2.md: regular playtester 50.6 % over 2,000 matches; 0.94 by R-0029 pass 2 measured 52.3 on main 7dfbeb5)
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
		int GraceFrames = 0;      // lane A113 (R-0049 point 1): frames left of its release grace (a hit on it is a bump at most); 0 = none
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
		// HOOK (lane A115 for track F2, lane A121's game.stick.take_grace_frames): frames left in which this skater's puck cannot be poked or lifted (it has
		// just won it: FJamSkaterState::TakeGraceLeft; the sim's refusal sits in IsPokeShielded). 0 = none. Nothing fills it in this tree:
		// FJamSimAdapter::BuildWorld sets it once the sim exposes it (the line is there, commented). The brain's read is StickCannotTake() below.
		int TakeGraceFrames = 0;
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
		// Lane A129 (track Q1): the second human, the guest of an online match (UJamSim::SeatGuest), or -1. The brain neither thinks for it nor gives it a
		// job (it plans with it as a human, as with PlayerIndex); offline it is -1 and nothing changes.
		int GuestIndex = -1;
		bool IsHumanSeat(int i) const { return i == PlayerIndex || (GuestIndex >= 0 && i == GuestIndex); }
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
		double LiftTapM = 0.0;    // R-0042 (lane A85): a tap stays on the ice
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
		int CarrierBestSaucer = -1;     // lane A77 (R-0036): the teammate a saucer from the carrier would go to (a blade may be in the lane); -1 nobody
		int SaucerHalfFrames = 8;       // lane A77: game.pass.saucer.half_frames (0 = the saucer is off)
		// Lane A94 (R-0045 law 2): the holder of the puck at the sim's own carrier reach (UJamSim::CarrierIndex: the blade's full length, where a carried puck
		// rides outside the shooting range) and the teammate its pass would go to; -1 none. Only the hockey sense reads them; W.Carrier is unchanged.
		int Holder = -1;
		int HolderBestPass = -1;
		int CarrierOrHolder() const { return Carrier >= 0 ? Carrier : Holder; }
		// R-0029 point 4 (lane A39): the goalie commit of the latest deke (UJamSim::GetDekeCommit): while it runs, the deker's opponents' blades, pokes
		// and shot blocks follow PuckPos - DekeCommitOffset (the puck's old side) instead of the puck.
		int DekeCommitFrames = 0;
		int DekeCommitSkater = -1;
		FVec2 DekeCommitOffset;
		double SkaterRadius = 0.4;
		double StickReach = 1.6;        // game.stick.reach_m
		// R-0042 (lane A85): the carrier's reach, UJamSim::CarrierReachM's rule: max(shot.reach_m, body radius + stick.reach_m) + 0.05. The brain's shot, one-timer
		// and pass-intent tests read it (the adapter's bCanShoot is the sim's own test). W.Carrier is still the adapter's shot.reach_m test: see StickPlay's cap.
		double CarrierReach() const { return (ShotReach > SkaterRadius + StickReach ? ShotReach : SkaterRadius + StickReach) + 0.05; }
		double LiftRange = 1.0;         // game.stick.lift_range_m
		double LiftConeDeg = 180.0;     // lane A95 (B-to-A-0037): game.stick.lift_cone_deg (the brain lifts only inside it)
		// Lane A95 (R-0045 law 3 / 4): the movement verbs the sim offers (A93's keys; 0 = the verb is off). The playtester uses a verb only when it exists.
		double ReverseBrakeDecel = 0.0; // game.skater.reverse_brake_decel
		int BurstFrames = 0;            // game.skater.burst.frames
		double SprintCarryBonus = 0.0;  // game.skater.sprint.carry_bonus (0 = the carrier cannot sprint)
		int SweepBotReactFrames = -1;   // lane A51: game.stick.sweep.bot_react_frames (-1 = absent: the level's FJamBotSkill::SweepReactFrames)
		// HOOK (lane A115 for track F2, lane A102's game.stick.body_shield): true when the sim refuses a poke or a lift while the carrier's body is between the
		// stick and the puck (UJamSim::IsBodyShielded). Nothing fills it in this tree (the key is not here yet): FJamSimAdapter::BuildWorld sets it from
		// G.Stick.bBodyShield once F2 has landed (the line is there, commented). The brain's read is BodyShields() below, the sim's rule on the brain's world.
		bool bBodyShield = false;
		// Lane A138 (track I1, R-0053): two more facts of the sim's stick rules, filled by FJamSimAdapter::BuildWorld (false / 0 = the brain before the lane).
		// bShieldShove: a lift the body shield or the take grace refuses is a cross-check's shove from any side (lane A125, R-0050 point 2). From behind that
		// shove is the from-behind foul where the whistle is on, and it is not a play the brain chooses where it is off (R-0053 point 3): StickPlay.
		// SpinExposedFrames: game.skater.spin.exposed_frames (lane A128): for the last frames of a spin neither the body nor a sweep shields the puck from a
		// stick. BodyShields() reads it, or the brain would hold its stick through the one moment the spin can be answered.
		bool bShieldShove = false;
		int SpinExposedFrames = 0;
		double BodyShieldRangeM = 2.0;   // UJamSim::BodyShieldRangeM
		double BodyShieldFarDeg = 60.0;  // UJamSim::BodyShieldFarDeg
		double BodyShieldFrontDeg = 60.0; // UJamSim::BodyShieldFrontDeg
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

	// Lane A94 (R-0045 law 2): the space read for a carrier (pure). Ahead is the way it means to carry (unit). The first opponent on the lane ahead (a cone
	// 2.5 m wide plus a fifth of the distance, out to 15 m), the seconds until it arrives (its distance less a metre over the closing speed, at least 1.5 m/s;
	// 0 when any opponent stands within 2.2 m), the opponents on that lane, and for each teammate its distance to the nearest opponent and whether the pass
	// lane is open (no opponent's body or blade within 1.5 m of the line), open (lane open, 3 m free, 4-30 m away) and better placed (open and BetterByM
	// nearer the opposing net than the carrier).
	struct FJamSpaceRead
	{
		int FirstOpp = -1;
		double FirstOppDist = 1e9;
		double ArriveSec = 1e9;
		int OnLane = 0;
		double MateFree[kMaxSkaters] = {};
		bool bLaneOpen[kMaxSkaters] = {};
		bool bOpen[kMaxSkaters] = {};
		bool bBetter[kMaxSkaters] = {};
		int NumOpen = 0;
		int NumBetter = 0;
	};
	JAMCORE_API FJamSpaceRead ReadSpace(const FJamAIWorld& W, int Carrier, const FVec2& Ahead, double BetterByM = 5.0, int IgnoreOpp = -1);

	// Lane A115 (evolution 2, BRAIN-2.md). Pure reads, exported for the tests.
	// The skater of Team (the human included, nobody who is down) that reaches a loose puck first: the earliest time, in steps of a tenth of a second up to
	// two seconds, at which it can be within a metre of where the puck will be (the puck slows as under puck.drag 0.5; a skater covers SkaterSpeed m/s after
	// a fifth of a second to turn). The last toucher of a puck that leaves it faster than 6 m/s (its own pass or shot) starts a second late: it is the chaser
	// only when nobody else of its team gets there. -1 = the team has nobody standing.
	JAMCORE_API int LoosePuckOwner(const FJamAIWorld& W, int Team, double SkaterSpeed);
	// Lane A134 (evolution 3, BRAIN-3.md). Seconds until skater i can be within a metre of where the loose puck will be (steps of a tenth up to two seconds,
	// the puck slowing as in LoosePuckOwner; 2.5 + distance / speed when not inside two). The skater first turns: 0.2 s when it already heads for the puck
	// (its velocity above 1 m/s, else its facing), up to 0.8 s for an about-turn, drifting on its velocity meanwhile; then it covers Speed m/s.
	JAMCORE_API double ArriveSec(const FJamAIWorld& W, int i, double Speed);
	// Lane A134: the point on an opposing carrier's lane to Team-of-Id's net that Id reaches before the carrier does (steps of 2 m from 2 m ahead of the
	// carrier; the carrier at its own speed, at least 3 m/s; Id at Speed), else the last point, 4 m in front of the net.
	JAMCORE_API FVec2 BackcheckPoint(const FJamAIWorld& W, int Id, int Carrier, double Speed);
	// Where a teammate of the carrier offers a pass lane: the best of six spots beside and ahead of the carrier (each side: level and wide, ahead, far ahead),
	// scored by an open lane from the carrier (no opponent within 2 m of it), the free ice round the spot, how far up the ice it is and how near the skater is
	// to it already; never within 4 m of another teammate.
	JAMCORE_API FVec2 OfferLane(const FJamAIWorld& W, int Id, int Carrier);
	// The sim's body shield on the brain's world (UJamSim::IsBodyShielded, lane A102): Poker within BodyShieldRangeM of Carrier, the puck on the far side of
	// Carrier's body, Carrier not facing Poker. False while W.bBodyShield is false.
	JAMCORE_API bool BodyShields(const FJamAIWorld& W, int Poker, int Carrier);
	// The carrier's puck cannot be taken by a stick right now: the carrier is inside its take grace (FJamAISkater::TakeGraceFrames) or its body shields the
	// puck from Poker (BodyShields). A poke or a lift would be a refused swing: the brain presses neither (StickPlay, FJamPlayerBot) and decides something
	// else (Attack): round the body when the body shields, close and contain goal-side at stick length during the grace; the body check stays.
	JAMCORE_API bool StickCannotTake(const FJamAIWorld& W, int Poker, int Carrier);

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
		int64_t LiftLast[kMaxSkaters] = {};   // lane A95 (B-to-A-0037 point 3): the frame of this bot's last lift press (FJamBotSkill::LiftCooldownFrames)
		int DrawJumps = 0;               // diagnostics: early draw presses sent
		int DrawPresses = 0;   // R-0017: frames an opposing carrier's puck has been inside this bot's blade reach
		// Lane A31: frames shoot has been held for the current aimed shot (0 = not charging); the goalie intent of the last Decide per skater.
		int Charging[kMaxSkaters] = {};
		int PassHold[kMaxSkaters] = {};   // lane A77 (R-0036): frames pass has been held for a saucer (0 = not holding)
		bool bGoalieNow[kMaxSkaters] = {};
		int GoalieSideNow[kMaxSkaters] = {};
		void StickPlay(const FJamAIWorld& W, int Id, FJamAIInput& In);
		// Lane A94 (R-0045 law 2): possession per frame (a ring, for the late read), the shape job per skater (AssignJobs, once per Think), and the frames
		// each skater has carried its current puck.
	public:
		enum class EJob : uint8_t { None, Puck, Support, Far, Point, Press, Cover, Weak };
		EJob LastJob(int i) const { return Job[i]; }
	private:
		static constexpr int kPhaseHist = 64;
		struct FPoss { int Team = -1; int64_t Since = 0; };
		FPoss PossHist[kPhaseHist] = {};
		int64_t PossFrame = -1;
		FPoss PossNow;
		EJob Job[kMaxSkaters] = {};
		bool JobLoose[kMaxSkaters] = {};  // the ShapeShare coin missed: the job is played loosely (an off spot, three quarters' pace)
		int JobOrd[kMaxSkaters] = {};
		int Owner[2] = {-1, -1};         // the skater (the human included) each attacking team has on the puck: the carrier, or the one nearest where a loose puck will be    // 0 for the first skater of a job on its team, 1 for the second (the second far / weak skater takes another spot)
		int CarryFrames[kMaxSkaters] = {};
		// Lane A115 (evolution 2): the brake on purpose. BrakeHold: frames left of a decided stop (the input is held against the velocity until the skater is
		// slow); BrakeCool: frames before the next decided stop; the wants are this frame's reasons (Decide, JobSteer and Attack set them; Think reads them).
		int BrakeHold[kMaxSkaters] = {};
		int BrakeCool[kMaxSkaters] = {};
		mutable bool bWantStop[kMaxSkaters] = {};      // the carrier stops in front of a defender (Attack), the presser closes under control (JobSteer)
		mutable bool bPlaysPuck[kMaxSkaters] = {};     // Attack ran for this skater this frame (it plays the puck)
		int MeantBrakes = 0, Carves = 0;               // diagnostics: decided stops and reversals carved into a turn since Reset
	public:
		int GetMeantBrakes() const { return MeantBrakes; }
		int GetCarves() const { return Carves; }
		bool IsBraking(int i) const { return BrakeHold[i] > 0; }
	private:
		void ReadPossession(const FJamAIWorld& W);
		FPoss PossAt(const FJamAIWorld& W, int Late) const;
		void AssignJobs(const FJamAIWorld& W);
		bool JobSteer(const FJamAIWorld& W, int Id, FVec2& Out, bool& bToPuck) const;
		// Lane A153 (evolution 4): where mate Id screens for the human in the first frames of his carry (FJamHockeySense::ScreenFrames); false = it does not
		bool ScreenPoint(const FJamAIWorld& W, int Id, FVec2& Out) const;
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
