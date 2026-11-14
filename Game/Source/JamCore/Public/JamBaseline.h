// Lane A138 (track I1, R-0053 point 5): ONE shared baseline for the pinned-hash neutral tests.
//
// Every lane used to pin "my rule at its neutral value replays my baseline" from its own tree, with the other lanes' neutral values written out by hand in
// each test; five landings on 13 Nov stopped on each other's pins. This header is the one place that knows what "neutral" is:
//
//   JamBaseline::MakeNeutral(Game)   every sim rule added since the vision's commit (main ee14438: lanes A92, A93, A94 and the curve of A95) at its neutral value
//   JamBaseline::BrainTune()         the same for the brain: a profile tune (JamAI::ApplyDifficultyTune) that switches off every brain evolution since
//   JamBaseline::HooksDead(World)    the brain's reads of sim rules that an older commit's brain did not have (call after BuildWorld when replaying such a commit)
//
// A neutral test calls these and then un-neutralises ONLY what its pinned commit had on (the With...() helpers below name the steps of the evening in the
// order they landed, so "the tree before lane X" is one call). A pin that still does not hold after that is a real change of the game, not a neighbour's key.
//
// THE NEXT LANE: a new rule goes in with its neutral value in MakeNeutral (one line), a new brain rule with its key in BrainTune (one pair), and, if a
// later test must replay the tree that had your rule on, one With...() step. Nothing else in any test changes.
// Header only; no state. docs/experiments/INTEGRATION.md section 1.
#pragma once

#include "CoreMinimal.h"
#include "JamCartridge.h"

namespace JamBaseline
{
	// Every rule since the vision's commit at its neutral value, in the order the rules landed.
	inline void MakeNeutral(FJamGameConfig& G)
	{
		G.Contact.BoardBonus = 1.0;               // lane A101 (track F1): game.contact.board_bonus (default 1.25)
		G.Contact.ReleaseGraceFrames = 0;         // lane A113 (track F1, R-0049 / R-0051): game.contact.release_grace_frames (default 120)
		G.Stick.bPassiveTake = true;              // lane A102 (track F2, R-0048): game.stick.passive_take (default false) ...
		G.Stick.bTeammateTouchTakes = true;       // ... with the teammate-touch switch (a rule, no key)
		G.Stick.bBodyShield = false;              // lane A102: game.stick.body_shield (default true)
		G.Stick.TakeGraceFrames = 0;              // lane A121 (track F2, R-0049 point 2): game.stick.take_grace_frames (default 30)
		G.Skater.ReverseBrakeDecel = 20;          // lane A121: game.skater.reverse_brake_decel (default 26)
		G.Skater.bBurstOwnStop = false;           // lane A121: game.skater.burst.own_stop (default true)
		G.Contact.CheckReachM = 0.0;              // lane A125 (track F1): game.contact.check_reach_m (ships at 0)
		G.Contact.bShieldShove = false;           // lane A125 / A138 (R-0050 point 2): the shield refuses the stick, never the shove (a rule, no key)
		G.Skater.BurstRestFrames = 0;             // lane A128 (track F2): game.skater.burst.rest_frames (default 6)
		G.Skater.SpinExposedTail = 0;             // lane A128: game.skater.spin.exposed_frames (default 5)
		G.Contact.ShoveBehindMatchMps = 0.0;      // lane A138 (track I1, R-0053 point 3): a push in the back at matched speed is a bump (a rule, no key; 0 = off)
		G.Stick.bTakeGraceSameFrame = false;      // lane A138 (track I1): the take grace covers the frame the puck is won in (a rule, no key)
		G.Contact.bPlaytesterClosingPress = false;   // lane A132 (track F1): the scripted human presses its check on closing speed (the instrument, no key; false = its own speed, as the pins were played)
		G.Skater.BurstRestKeptMps = 0.0;          // lane A147 (track F2): game.skater.burst.rest_kept_mps (default 3)
		// THE NEXT LANE: your rule's neutral value here, one line.
	}

	// Every brain evolution since the vision's commit switched off, as a profile tune (append it to any other tune with ';').
	// brain2=0: lane A115 (track Q2, evolution 2), both sides, every rule. Lane A138's three brain rules need no pair: the way round the body
	// (hk_round_max_mps) and the spin's exposed end (hk_spin_end_share) are read only with the shield read on, which brain2=0 switches off, and the
	// no-shove-from-behind rule is dead wherever MakeNeutral has switched the sim's shove rule off (or HooksDead was called).
	// THE NEXT LANE: your brain rule's off-switch here, one pair.
	// Level ("rookie", "pro", "all_star"; every pin so far is a pro match): THE CURVE LANE: when you move a level's profile numbers, the numbers that level
	// had before you go here for that level (as JamAI::PreA95Tune carries the levels of the commit before the vision), and the pins stay where they are.
	// Lanes A112 / A133 (track C0, CURVE-6.md, CURVE-7.md): each level's profile numbers those lanes moved, back to the values main had before them
	// (rookie: poke never, shot reach + 5, mates at speed_scale; pro: poke after 6 frames, hunt 8 m, checks from 2 m/s, pressure 1.4 s / better by 3 m; all_star: hunt 20, shot reach + 40,
	// mates at speed_scale). The large-rink offsets are not tune keys: every pin is a hockey.regular match.
	// bLargeRink: a rink of JamAI::LargeRinkM or longer, whose red skill offsets the curve lanes also moved (pro 1.04, rookie 0.9; all_star's large speed is
	// not a tune key: no pin is an all_star match on the large rink).
	inline const char* BrainTune(const char* Level = "pro", bool bLargeRink = false)
	{
		if (Level && Level[0] == 'r') return bLargeRink ? "brain2=0;opp.poke_frames=-1;opp.shoot_extra_m=5;mate_speed_scale=0;red_skill=0.9" : "brain2=0;opp.poke_frames=-1;opp.shoot_extra_m=5;mate_speed_scale=0";
		if (Level && Level[0] == 'a') return "brain2=0;opp.hunt_radius_m=20;opp.shoot_extra_m=40;mate_speed_scale=0";
		return bLargeRink ? "brain2=0;opp.poke_frames=6;opp.hunt_radius_m=8;opp.check_min_closing=2;opp.hk_pressure_s=1.4;opp.hk_better_by_m=3;mate_speed_scale=0;red_skill=1.04"
			: "brain2=0;opp.poke_frames=6;opp.hunt_radius_m=8;opp.check_min_closing=2;opp.hk_pressure_s=1.4;opp.hk_better_by_m=3;mate_speed_scale=0";
	}

	// Lane A133 (track C0): the shipped cuts' whistle. Lane A112 took it out of hockey.regular, hockey.arena.wide, hockey.arena.large and the season's takes
	// (rules.penalties false: content, not a rule, so not in MakeNeutral, whose callers may switch the whistle off themselves). A pin made on one of those cuts
	// before lane A112, with the cut's own rules, calls this after MakeNeutral.
	inline void WithWhistle(FJamGameConfig& G) { G.Rules.bPenalties = true; }

	// The brain's reads of track F2's shields (lane A115's two hooks, wired at the landing of 22:20): dead, as on any commit before them. Call on the world
	// right after FJamSimAdapter::BuildWorld when replaying such a commit with the body shield or the take grace on. (A template: no brain header here.)
	template <typename TWorld> inline void HooksDead(TWorld& W)
	{
		W.bBodyShield = false;
		W.bShieldShove = false; W.SpinExposedFrames = 0;   // lane A138's two world facts
		for (int i = 0; i < W.NumSkaters; ++i) W.Skaters[i].TakeGraceFrames = 0;
	}

	// The steps of 13 Nov, each on top of MakeNeutral, for a test whose pinned commit already had them on. They stack: call the ones the commit had.
	// Each writes the values that commit had, not today's defaults: a later lane that moves a default (the curve, a re-tuned grace) does not move a pin.
	inline void WithBoards(FJamGameConfig& G) { G.Contact.BoardBonus = 1.25; }                 // lane A101
	inline void WithReleaseGrace(FJamGameConfig& G) { G.Contact.ReleaseGraceFrames = 120; }    // lane A113 (R-0051)
	inline void WithKeptPuck(FJamGameConfig& G)                                                // lane A102 (R-0048, R-0050)
	{
		G.Stick.bPassiveTake = false; G.Stick.bTeammateTouchTakes = false; G.Stick.bBodyShield = true;
	}
	inline void WithCarrierAnswer(FJamGameConfig& G)                                           // lane A121
	{
		G.Stick.TakeGraceFrames = 30; G.Skater.ReverseBrakeDecel = 26; G.Skater.bBurstOwnStop = true;
	}
}
