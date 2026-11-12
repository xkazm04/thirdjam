// Lane A3: the three AI difficulties (game.ai.difficulty rookie | pro | all_star, absent = pro). Engine-free, like JamAICore,
// so the native tournament driver compiles it too. The numbers are written to docs/experiments/BALANCING.md.
//
// Resolution order (brief A3): the level's profile -> explicit game.ai numbers in the cartridge -> Designer `values`
// (ai_speed_scale, ai_red_skill). The engine side of that order (reading which keys the cartridge wrote) is
// DifficultyResolve.h; this file holds the data and the parts that need no engine.
//
// What a level changes: the opposing bots' skill (reaction delay, shot accuracy, shot range, lead / intercept, knowledge
// of the aim assist), how much room the bots give the human (yield and hold radii), stall tolerance and bot speed.
// The human's own teammates keep A2's skill at every level (they pass to the human at rookie): a level is how good the other team is.
#pragma once

#include "JamAICore.h"

namespace JamAI
{
	enum class EDifficulty : uint8_t { Rookie, Pro, AllStar };

	struct FJamDifficultyProfile
	{
		const char* Name;
		double SpeedScale;        // game.ai.speed_scale: every bot's top speed as a fraction of skater.speed
		double RedSkill;          // game.ai.red_skill: the opposing side's extra speed factor
		double YieldMarginM;      // game.ai.yield_margin_m: teammates leave the puck to the human when the human is this much nearer
		double YieldRadiusM;      // game.ai.yield_radius_m
		double HoldRadiusM;       // game.ai.hold_radius_m: opponents (not the back) hold off while the human is this near the puck
		int StallFrames;          // stall tolerance: frames the puck may sit (2 m) before a support joins the chase
		FJamBotSkill Opp;         // the opposing bots' skill
		bool bMatesPass;          // the human's teammates pass to the human (FJamBotSkill::bPassToHuman)
	};

	// The three profiles. Never null; an out-of-range level returns pro.
	const FJamDifficultyProfile& GetDifficultyProfile(EDifficulty Level);
	const char* DifficultyName(EDifficulty Level);
	// "rookie" | "pro" | "all_star" (exact, case-sensitive, as the schema). False on anything else.
	bool ParseDifficulty(const char* Name, EDifficulty& Out);

	// Writes every profile number into P (the explicit cartridge numbers are applied after this by the caller).
	// The teammates' skill (P.Mate) is FJamBotSkill() (A2's brain) plus the level's bMatesPass.
	void ApplyDifficultyProfile(const FJamDifficultyProfile& Profile, FJamAIParams& P);

	// Tuning hook for tools/sweep (not a cartridge path): "opp.reaction_frames=6;opp.aim_error_deg=4;hold_radius_m=3;...".
	// Keys: speed_scale red_skill yield_margin_m yield_radius_m hold_radius_m stall_frames guard_range_m guard_radius_m and opp.* / mate.*
	// reaction_frames aim_error_deg shoot_extra_m aim_tolerance_deg lead_frames intercept(0|1) use_aim_cone(0|1) clear_lane(0|1) pass_to_human(0|1).
	// Returns false and writes the first bad pair into Err.
	bool ApplyDifficultyTune(const char* Text, FJamAIParams& P, char* Err, int ErrLen);
}
