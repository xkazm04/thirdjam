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
			// Lane A46 (PAYOFFS.md section 6): the human's teammates pass to the open human in 0.4 of the chances at pro (0 before: the one-timer never reached the human).
			FJamDifficultyProfile P{"pro", D.SpeedScale, D.RedSkill, D.YieldMarginM, D.YieldRadiusM, D.HoldRadiusM, D.StallFrames, D.Opp, 0.4};
			P.LargeRedSkill = 0.97;   // lane A70 (CURVE-2.md, 2,000 matches): the regular playtester on 120 x 60 71.0 % at 0.9 -> 51.1 at 0.97 (0.95 55.6, 1.0 45.5); R-0029 pass 2 had 0.9
			return P;
		}

		FJamDifficultyProfile MakeRookie()
		{
			FJamBotSkill Opp;
			Opp.ReactionFrames = 0;
			Opp.AimErrorDeg = 0.0;
			Opp.ShootExtraM = 5.0;     // R-0029 P3 (13 before): shoot from reach + 5 m
			Opp.GuardRadiusM = 10.0;   // R-0029 P1 (5 / 45 before): the rookie back's guard post 10 m out, only within 20 m of its net
			Opp.GuardRangeM = 20.0;
			Opp.bBlockShots = true;
			Opp.BlockMinDistM = 35.0;   // only a shot from near centre ice (90 x 45: 41 m to the net) is blocked
			Opp.AimToleranceDeg = 20.0;
			Opp.LeadFrames = 15;
			Opp.bIntercept = false;
			Opp.bUseAimCone = true;
			Opp.PokeFrames = -1;   // R-0017: rookie bots never poke
			Opp.SweepReactFrames = 14;   // lane A51 (R-0034): the rookie's blade reads the carried puck 14 frames late
			Opp.ShotChargeFrames = 0;   // lane A31 (B-to-A-0024): rookie taps its shots
			Opp.DrawJumpPct = 100; // 16:23: the rookie centre always jumps early (the human takes the opening touch)
			// R-0029 P2: speed 0.85 / red skill 0.75 (0.96 / 0.88 before, tuned against the scripted human); measured with lane A34's novice.
			// Stall 60: the stall rule is what lets a rookie shoot at all (SeasonTakes needs one AI shot per match).
			// Lane A70 (CURVE-2.md): red skill 0.75 -> 0.8, the novice 90.8 -> 85.8 % over 2,000 matches (speed 0.9 moved nothing).
			FJamDifficultyProfile P{"rookie", 0.85, 0.8, 4.0, 25.0, 0.0, 60, Opp, 0.6};   // lane A46: mates pass to the open human in 0.6 of the chances (every chance before)
			P.LargeSpeedScale = 0.9;   // R-0029: large-rink rookie offsets from the playtester grid (novice 85 %, 19:43); 1.0 / 0.93 before
			P.LargeRedSkill = 0.97;   // lane A70 (CURVE-2.md, 2,000 matches): the novice on 120 x 60 97.7 % at 0.8 (0.9 93.3, 0.95 89.0, 1.0 81.8)
			return P;
		}

		FJamDifficultyProfile MakeAllStar()
		{
			FJamBotSkill Opp;
			Opp.ReactionFrames = 0;
			Opp.AimErrorDeg = 0.0;
			Opp.ShootExtraM = 40.0;
			Opp.AimToleranceDeg = 20.0;
			Opp.LeadFrames = 60;
			Opp.bClearLane = true;
			Opp.PokeFrames = 0;   // lane A79 (R-0041, CURVE-3.md): 2 -> 0, all_star pokes the moment the puck is in reach (with the hunt, below)
			Opp.SweepReactFrames = 5;   // lane A51 (R-0034): all_star reads the carried puck 5 frames late (pro 8, the FJamBotSkill default)
			Opp.ShotChargeFrames = 18;   // lane A31 (B-to-A-0024): all_star winds its shots up (pro 10, the FJamBotSkill default)
			Opp.bLift = true;
			Opp.bPassOut = true;
			Opp.bReadLanes = true;
			Opp.bSeekPickups = true;
			Opp.DrawPressPct = 0;  // 16:18: no early press; the centre jumps early in half the faceoffs instead (faceoffs without a human touch 2 per 40 matches)
			Opp.DrawJumpPct = 75;   // 16:25 worktree sweep: faceoffs without a human touch at most 2 per 40 matches
			Opp.DrawPressFrames = 8;
			Opp.ForecheckYieldFrames = 12;   // Director 15:16
			Opp.bIntercept = true;
			Opp.bUseAimCone = true;
			Opp.bOneTimer = true;   // R-0029 point 4 (lane A39): all_star bots use the one-timer and the deke; pro the one-timer (ProOppSkill); rookie neither
			Opp.bDeke = true;
			Opp.CheckMinClosing = 3.0;   // R-0031 point 2 (lane A45): all_star body-checks the carrier (front or side, never from behind); rookie keeps -1 (never). Lane A70: 5 -> 3 (knockdowns of the good playtester 2.73 -> 3.00 a match at speed 1.3, its win share 26.5 -> 26.8)
			Opp.HuntRadiusM = 10.0;   // lane A79 (R-0041 point 1, CURVE-3.md): the support goes at an opposing carrier within 10 m (good 26.8 -> 21.4, regular 13.3 -> 11.8 at speed 1.3)
			// R-0029 pass 2 (with A31's charge): speed 1.08 (bots may push past 1, the sim clamps them at 1.5): the good playtester 30 % / 25 %
			// R-0029 with A39's payoffs on (one-timer, burst, deke): speed 1.15 brings the good playtester back to 23 % at 40 seeds (1.08 measured 38 %, 1.12 33 %); A39 section 9.
			// Lane A70 (CURVE-2.md, 2,000 matches per number): speed 1.15 -> 1.3, the good playtester 40.5 -> 26.5 % (1.2 34.5, 1.32 23.9, 1.4 14.8; checks from
			// 3 or 4 m/s and bot_react_frames 3 moved nothing).
			// Lane A79 (R-0041, CURVE-3.md): the hunt and the instant poke take the win rate, so speed comes back 1.3 -> 1.2 (good 27.0, regular 14.4, novice 3.4).
			return FJamDifficultyProfile{"all_star", 1.2, 1.0, 4.0, 25.0, 0.0, 60, Opp, 0.2};   // lane A46: mates pass to the open human in 0.2 of the chances
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
			if (std::strcmp(Key, "poke_frames") == 0) { K.PokeFrames = static_cast<int>(V); return true; }
			if (std::strcmp(Key, "lift") == 0) { K.bLift = V != 0.0; return true; }
			if (std::strcmp(Key, "pass_out") == 0) { K.bPassOut = V != 0.0; return true; }
			if (std::strcmp(Key, "read_lanes") == 0) { K.bReadLanes = V != 0.0; return true; }
			if (std::strcmp(Key, "seek_pickups") == 0) { K.bSeekPickups = V != 0.0; return true; }
			if (std::strcmp(Key, "draw_jump_pct") == 0) { K.DrawJumpPct = static_cast<int>(V); return true; }
			if (std::strcmp(Key, "draw_press_pct") == 0) { K.DrawPressPct = static_cast<int>(V); return true; }
			if (std::strcmp(Key, "draw_press_frames") == 0) { K.DrawPressFrames = static_cast<int>(V); return true; }
			if (std::strcmp(Key, "guard_radius_m") == 0) { K.GuardRadiusM = V; return true; }
			if (std::strcmp(Key, "guard_range_m") == 0) { K.GuardRangeM = V; return true; }
			if (std::strcmp(Key, "carry_speed") == 0) { K.CarrySpeed = V; return true; }
			if (std::strcmp(Key, "forecheck_yield_frames") == 0) { K.ForecheckYieldFrames = static_cast<int>(V); return true; }
			if (std::strcmp(Key, "pass_to_human") == 0) { K.PassToHuman = V; return true; }   // lane A46: a 0..1 share (was 0 | 1)
			if (std::strcmp(Key, "shot_charge_frames") == 0) { K.ShotChargeFrames = static_cast<int>(V); return true; }   // lane A31
			if (std::strcmp(Key, "one_timer") == 0) { K.bOneTimer = V != 0.0; return true; }   // R-0029 point 4 (lane A39)
			if (std::strcmp(Key, "deke") == 0) { K.bDeke = V != 0.0; return true; }
			if (std::strcmp(Key, "check_min_closing") == 0) { K.CheckMinClosing = V; return true; }   // R-0031 point 2 (lane A45); -1 = never
			if (std::strcmp(Key, "check_from_behind") == 0) { K.bCheckFromBehind = V != 0.0; return true; }
			if (std::strcmp(Key, "hunt_radius_m") == 0) { K.HuntRadiusM = V; return true; }   // R-0041 point 1 (lane A79)
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
		P.Mate.PassToHuman = Profile.MatesPassShare;
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
			else if (std::strcmp(Key, "guard_range_m") == 0) { P.GuardRangeM = V; P.Opp.GuardRangeM = V; P.Mate.GuardRangeM = V; }   // both sides (opp.* / mate.* for one)
			else if (std::strcmp(Key, "guard_radius_m") == 0) { P.GuardRadiusM = V; P.Opp.GuardRadiusM = V; P.Mate.GuardRadiusM = V; }
			else { bOk = false; }
			if (!bOk) { *Eq = '='; SetErr(Err, ErrLen, "unknown tune key", Pair); return false; }
		}
		return true;
	}
}
