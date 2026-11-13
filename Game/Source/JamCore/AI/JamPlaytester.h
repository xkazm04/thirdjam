// Lane A34 (R-0028 point 3): a human-like playtester for the human's slot, and the fun metrics of one match.
//
// FJamPlaytester plays like a person would through a pad: it sees the world late (a reaction ring of N frames), aims with an error that
// drifts every two seconds, moves an analog stick that ramps instead of snapping, holds the shoot button to charge and lets go to shoot,
// and (the novice) stops pressing anything for half a second every ten seconds. Its decisions read only FJamAIWorld and its output is one
// FJamAIInput, which FJamSimAdapter::Step copies into FJamInput field by field: the same path the pawn's input takes in the game mode.
// No wall clock, no engine randomness: every "random" number is a hash of (seed, frame window, salt), so a match replays exactly.
//
// RunPlaytestMatch runs one match of the playtester against the brain on lane A1's UJamSim (through FJamSimAdapter: the checks, pokes and
// knockdowns come from UJamSim::GetFrameEvents, which IJamAISim does not carry) and fills FJamPlaytestStats, the fun metrics the
// tournament commandlet prints as thirdjam.playtest/1 JSON lines (-player=novice|regular|good) and tools/playtest/run.mjs tabulates.
// Nothing here changes a profile or a sim number: it measures.
#pragma once

#include "JamAICore.h"
#include "JamBotMatch.h"

namespace JamAI
{
	enum class EPlaytester : uint8_t { Novice, Regular, Good };

	// The person behind the pad. Numbers from briefs/A34-playtester.md (reaction 12 / 6 / 3 frames, aim noise 10 / 5 / 2 degrees, the novice's
	// lapse); the rest are this lane's reading of a first-time, a regular and a good player, listed in docs/experiments/PLAYTEST.md.
	struct FJamPlaytesterProfile
	{
		int ReactionFrames = 6;          // the world is seen this many frames late (0..kMaxReactionFrames)
		double AimNoiseDeg = 5.0;        // the aim point drifts up to this far off the goal centre; re-rolled every AimWindowFrames
		int AimWindowFrames = 120;
		int LapseEveryFrames = 0;        // every this many frames of play the player stops pressing for LapseFrames (0 = never)
		int LapseFrames = 30;
		double StickRamp = 0.2;          // how much |Move| may change per frame (an analog stick pushed over, not snapped)
		double ShootRangeM = 20.0;       // starts charging when the puck is this near the goal (metres from the goal centre)
		double ReleaseToleranceDeg = 6.0; // lets go when the facing is within this of the (noisy) aim
		int PatienceFrames = 40;         // ... or after holding this long, lined up or not (the shot still needs the sim's 60-degree front gate)
		double PassAheadM = 5.0;         // passes when a teammate is this much nearer the goal, open, and a defender closes
		double PressureM = 4.0;          // "a defender closes" = an opponent within this of me
		double CheckFromBehindDot = 0.5; // a check is attempted when my closing direction . the carrier's facing is below this (0.5 = the sim's foul line)
		int DrawReactionFrames = 12;     // presses for the draw this many frames after the drop
		double ChaseLeadS = 0.5;         // leads a moving puck / carrier by this many seconds of its velocity
		bool bGoalside = true;           // defends goalside of an opposing carrier that is nearer our net than we are
		// R-0029 point 4 (lane A39): how often the player uses the one-timer and the deke when the chance is there (per cent of 2-second windows:
		// the good player always, the regular one half the time, the novice never). The breakaway burst is the sim's: nobody presses for it.
		int OneTimerPct = 50;
		int DekePct = 50;
		// Lane A51 (R-0034): stick handling. The regular and good players carry the puck on the far side from a defender within SweepDefenderM in front
		// (a sweep when the side changes) and spin when a defender closes from behind at speed; the novice knows neither.
		bool bStickHandling = false;
		double SweepDefenderM = 2.5;
	};
	FJamPlaytesterProfile PlaytesterProfile(EPlaytester Kind);
	const char* PlaytesterName(EPlaytester Kind);
	bool ParsePlaytester(const char* Name, EPlaytester& Out);

	// What the playtester decided this frame (diagnostics; the match counts them).
	enum class EPlaytesterAct : uint8_t { Idle, Draw, Chase, Carry, Charge, Pass, Support, Forecheck, Poke, Check, Lapse, OneTimer, Deke, Sweep, Spin, Count };

	class JAMCORE_API FJamPlaytester   // lane A56: exported so the JamNet soak (Bar.Net.Soak*) can seat two playtesters as the two humans
	{
	public:
		FJamPlaytesterProfile P;
		int ChargeFramesMax = 30;        // game.shot.charge_frames_max (the sim's; FJamAIWorld does not carry it)

		void Reset(EPlaytester Kind, uint64_t Seed);
		// One frame: the decision on this frame's world, delayed by the reaction ring, then the analog ramp.
		FJamAIInput Think(const FJamAIWorld& W);
		EPlaytesterAct LastAct() const { return Act; }
		int ActCount(EPlaytesterAct A) const { return Acts[static_cast<int>(A)]; }

	private:
		uint64_t Seed = 0;
		int64_t Tick = 0;
		FJamAIInput Ring[kMaxReactionFrames + 1] = {};
		FVec2 MoveOut;                   // the stick as held after the ramp
		bool bHolding = false;           // shoot held
		int HoldFrames = 0;
		int64_t LastPassFrame = -1000;
		int64_t LastCheckFrame = -1000;
		int64_t LastDekeFrame = -1000;   // R-0029 point 4 (lane A39)
		int64_t PlayFrames = 0;          // unfrozen frames seen (the lapse clock)
		EPlaytesterAct Act = EPlaytesterAct::Idle;
		int Acts[static_cast<int>(EPlaytesterAct::Count)] = {};

		// A fixed number in [-1, 1] for (seed, window, salt).
		double Noise(int64_t Window, int Salt) const;
		FJamAIInput Decide(const FJamAIWorld& W);
	};

	// The fun metrics of one match (brief A34 point 2). Frames, never seconds; -1 = never happened.
	struct FJamPlaytestStats
	{
		uint64_t Seed = 0;
		int PlayerIndex = -1;
		int PlayerTeam = 0;
		int GoalsFor = 0, GoalsAgainst = 0;
		int GoalsByHuman = 0;            // goals whose last toucher was the human
		int64_t Frames = 0, PlayFrames = 0;
		bool bOver = false;
		int Winner = -1;
		uint64_t Hash = 0;
		int Touches = 0;                 // the human became the puck's last toucher
		int Carries3s = 0;               // spells as the carrier of at least 180 frames
		int64_t CarryFrames = 0;         // frames as the carrier
		int Shots = 0, ShotsOnTarget = 0; // on target: the puck's line at the shot crosses the goal line inside the mouth
		int ShotsOpp = 0;                // shots by the other team
		int ShotsMates = 0;
		int LeadChanges = 0;             // the leading team changes (ties in between allowed)
		int64_t FirstTouch = -1, FirstShot = -1, FirstGoalFor = -1, FirstGoalByHuman = -1, FirstGoalAgainst = -1;
		int Faceoffs = 0, FaceoffsWithoutTouch = 0, MaxFaceoffToTouch = 0;
		// lane A3 (R-0029): in the faceoff windows that ran out without a human touch, frames by who held the puck
		int64_t WoMateFrames = 0, WoOppFrames = 0, WoLooseFrames = 0;
		int ChecksLanded = 0, ChecksTaken = 0, Fouls = 0;
		int FoulsFromBehind = 0, FoulsNoPuck = 0;   // the two kinds of foul (contracts/events.md): from behind on the carrier; on a skater without the puck
		int64_t BoxFrames = 0;           // frames the human sat in the penalty box
		int Pokes = 0, PokesClean = 0, Lifts = 0, Passes = 0;
		int PassesReceived = 0;          // lane A46: teammates' passes seated on the human's blade (UJamSim's FJamSkaterState::PassesReceived)
		// R-0029 point 4 (lane A39): the payoffs the human used (one-timer shots, bursts, dekes) and the human's goals within 180 frames of one of them
		int OneTimers = 0, Breakaways = 0, Dekes = 0, PayoffGoals = 0;
		int Knockdowns = 0;              // the human knocked down (any cause)
		int64_t IdleFrames = 0;          // frames beyond 900 since the human's last touch (play frames only)
		int IdleSpells = 0;              // spells without a touch that reached 900 frames
		int64_t MaxNoTouch = 0;          // the longest spell without a touch
		int Acts[static_cast<int>(EPlaytesterAct::Count)] = {};
	};

	class FJamSimAdapter;
	// One match on UJamSim: the brain plays every other skater at the config's AI params, the playtester plays the human.
	// lane A71: the per-frame log (tools/soak/playtest/idle.mjs). Called after a frame's step every TraceEvery frames and on every frame the human touches the puck;
	// SinceTouch = play frames since the human's last touch. Null / 0 = no trace, and the match is the same either way.
	using FJamPlaytestTraceFn = void (*)(void* Ctx, const FJamAIWorld& W, const FJamAIInput* In, const FJamBrain& Brain, int64_t SinceTouch, bool bHumanTouch);
	FJamPlaytestStats RunPlaytestMatch(FJamSimAdapter& Sim, const FJamMatchConfig& C, uint64_t Seed, EPlaytester Kind, int64_t MaxFrames, int ChargeFramesMax,
		FJamPlaytestTraceFn Trace = nullptr, void* TraceCtx = nullptr, int TraceEvery = 0);
	// One thirdjam.playtest/1 JSON line (no newline) into Buf. Returns the length written.
	int FormatPlaytestJson(const FJamPlaytestStats& S, const char* Cartridge, const char* Difficulty, EPlaytester Kind, char* Buf, int BufLen);
}
