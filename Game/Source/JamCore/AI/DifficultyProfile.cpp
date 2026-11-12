// Lane A3: see DifficultyProfile.h. The profile table is the one place the difficulty numbers live
// (docs/experiments/BALANCING.md repeats them with the sweep that chose them).
#include "DifficultyProfile.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace JamAI
{
	namespace DifficultyLocal
	{
		// pro is the FJamAIParams / FJamBotSkill defaults, so a cartridge without a level and code that never heard of
		// difficulties play the same game.
		FJamDifficultyProfile MakePro()
		{
			const FJamAIParams D;
			return FJamDifficultyProfile{"pro", D.SpeedScale, D.RedSkill, D.YieldMarginM, D.YieldRadiusM, D.HoldRadiusM, D.StallFrames, D.Opp, D.Mate.bPassToHuman};
		}

		FJamDifficultyProfile MakeRookie()
		{
			FJamBotSkill Opp;
			Opp.ReactionFrames = 2;
			Opp.AimErrorDeg = 3.0;
			Opp.ShootExtraM = 13.0;
			Opp.bBlockShots = true;
			Opp.BlockMinDistM = 35.0;   // only a shot from near centre ice (90 x 45: 41 m to the net) is blocked
			Opp.AimToleranceDeg = 20.0;
			Opp.LeadFrames = 15;
			Opp.bIntercept = false;
			Opp.bUseAimCone = false;
			return FJamDifficultyProfile{"rookie", 0.9, 0.85, 4.0, 25.0, 0.0, 180, Opp, true};
		}

		FJamDifficultyProfile MakeAllStar()
		{
			FJamBotSkill Opp;
			Opp.ReactionFrames = 0;
			Opp.AimErrorDeg = 0.0;
			Opp.ShootExtraM = 30.0;
			Opp.AimToleranceDeg = 20.0;
			Opp.LeadFrames = 45;
			Opp.bIntercept = true;
			Opp.bUseAimCone = true;
			return FJamDifficultyProfile{"all_star", 1.0, 1.0, 1.5, 12.0, 0.0, 90, Opp, false};
		}

		void SetErr(char* Err, int ErrLen, const char* Msg, const char* Pair)
		{
			if (Err && ErrLen > 0)
			{
				std::snprintf(Err, static_cast<size_t>(ErrLen), "%s: %s", Msg, Pair);
			}
		}

		bool SetSkill(FJamBotSkill& K, const char* Key, double V)
		{
			if (std::strcmp(Key, "reaction_frames") == 0) { K.ReactionFrames = static_cast<int>(V); return true; }
			if (std::strcmp(Key, "aim_error_deg") == 0) { K.AimErrorDeg = V; return true; }
			if (std::strcmp(Key, "shoot_extra_m") == 0) { K.ShootExtraM = V; return true; }
			if (std::strcmp(Key, "aim_tolerance_deg") == 0) { K.AimToleranceDeg = V; return true; }
			if (std::strcmp(Key, "lead_frames") == 0) { K.LeadFrames = static_cast<int>(V); return true; }
			if (std::strcmp(Key, "intercept") == 0) { K.bIntercept = V != 0.0; return true; }
			if (std::strcmp(Key, "use_aim_cone") == 0) { K.bUseAimCone = V != 0.0; return true; }
			if (std::strcmp(Key, "block_shots") == 0) { K.bBlockShots = V != 0.0; return true; }
			if (std::strcmp(Key, "block_min_dist_m") == 0) { K.BlockMinDistM = V; return true; }
			if (std::strcmp(Key, "clear_lane") == 0) { K.bClearLane = V != 0.0; return true; }
			if (std::strcmp(Key, "pass_to_human") == 0) { K.bPassToHuman = V != 0.0; return true; }
			return false;
		}
	}
	using namespace DifficultyLocal;

	const FJamDifficultyProfile& GetDifficultyProfile(EDifficulty Level)
	{
		static const FJamDifficultyProfile Rookie = MakeRookie();
		static const FJamDifficultyProfile Pro = MakePro();
		static const FJamDifficultyProfile AllStar = MakeAllStar();
		switch (Level)
		{
		case EDifficulty::Rookie: return Rookie;
		case EDifficulty::AllStar: return AllStar;
		default: return Pro;
		}
	}

	const char* DifficultyName(EDifficulty Level)
	{
		return GetDifficultyProfile(Level).Name;
	}

	bool ParseDifficulty(const char* Name, EDifficulty& Out)
	{
		if (!Name) { return false; }
		if (std::strcmp(Name, "rookie") == 0) { Out = EDifficulty::Rookie; return true; }
		if (std::strcmp(Name, "pro") == 0) { Out = EDifficulty::Pro; return true; }
		if (std::strcmp(Name, "all_star") == 0) { Out = EDifficulty::AllStar; return true; }
		return false;
	}

	void ApplyDifficultyProfile(const FJamDifficultyProfile& Profile, FJamAIParams& P)
	{
		P.SpeedScale = Profile.SpeedScale;
		P.RedSkill = Profile.RedSkill;
		P.YieldMarginM = Profile.YieldMarginM;
		P.YieldRadiusM = Profile.YieldRadiusM;
		P.HoldRadiusM = Profile.HoldRadiusM;
		P.StallFrames = Profile.StallFrames;
		P.Opp = Profile.Opp;
		P.Mate = FJamBotSkill();
		P.Mate.bPassToHuman = Profile.bMatesPass;
	}

	bool ApplyDifficultyTune(const char* Text, FJamAIParams& P, char* Err, int ErrLen)
	{
		const char* Cur = Text;
		char Pair[160];
		while (Cur && *Cur)
		{
			const char* Start = Cur;
			const char* End = std::strchr(Cur, ';');
			const size_t Len = End ? static_cast<size_t>(End - Cur) : std::strlen(Cur);
			Cur = End ? End + 1 : nullptr;
			if (Len == 0) { continue; }
			if (Len >= sizeof(Pair)) { SetErr(Err, ErrLen, "pair too long", ""); return false; }
			std::memcpy(Pair, Start, Len);
			Pair[Len] = 0;
			char* Eq = std::strchr(Pair, '=');
			if (!Eq) { SetErr(Err, ErrLen, "no '=' in pair", Pair); return false; }
			*Eq = 0;
			char* NumEnd = nullptr;
			const double V = std::strtod(Eq + 1, &NumEnd);
			if (NumEnd == Eq + 1 || *NumEnd != 0) { *Eq = '='; SetErr(Err, ErrLen, "not a number", Pair); return false; }
			const char* Key = Pair;
			bool bOk = true;
			if (std::strncmp(Key, "opp.", 4) == 0) { bOk = SetSkill(P.Opp, Key + 4, V); }
			else if (std::strncmp(Key, "mate.", 5) == 0) { bOk = SetSkill(P.Mate, Key + 5, V); }
			else if (std::strcmp(Key, "speed_scale") == 0) { P.SpeedScale = V; }
			else if (std::strcmp(Key, "red_skill") == 0) { P.RedSkill = V; }
			else if (std::strcmp(Key, "yield_margin_m") == 0) { P.YieldMarginM = V; }
			else if (std::strcmp(Key, "yield_radius_m") == 0) { P.YieldRadiusM = V; }
			else if (std::strcmp(Key, "hold_radius_m") == 0) { P.HoldRadiusM = V; }
			else if (std::strcmp(Key, "stall_frames") == 0) { P.StallFrames = static_cast<int>(V); }
			else if (std::strcmp(Key, "guard_range_m") == 0) { P.GuardRangeM = V; }
			else if (std::strcmp(Key, "guard_radius_m") == 0) { P.GuardRadiusM = V; }
			else { bOk = false; }
			if (!bOk) { *Eq = '='; SetErr(Err, ErrLen, "unknown tune key", Pair); return false; }
		}
		return true;
	}
}
