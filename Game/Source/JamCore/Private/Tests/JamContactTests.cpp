// Lane A92 (R-0045, docs/VISION.md law 1; docs/experiments/CONTACT.md): contact by momentum.
//   Jam.Core.Contact.SameDirection   two skaters going the same way at the same speed: a pressed check is a bump at most
//   Jam.Core.Contact.HeadOn          head-on at full speed: down for down_frames_max
//   Jam.Core.Contact.StandingBrace   a standing skater: a side body check from 4 m/s stumbles it at most, a front cross-check from the same speed puts it down
//   Jam.Core.Contact.SprintFalls     a sprinting victim falls to the hit a walking one survives
//   Jam.Core.Contact.AttackerBounces a sprinting attacker into a braced skater is the one who stumbles
//   Jam.Core.Contact.CarrierRefused  the carrier's check press is refused; contact.carrier_can_hit true gives the press back
//   Jam.Core.Contact.Fouls           a hit from behind on the carrier and a hit on a skater without the puck still call the foul
//   Jam.Core.ContactNeutral          the neutral block (momentum false, carrier_can_hit true): a three-minute bot match hashes as main at 8f6b157 did
//   Jam.Core.ContactMatch            the same match at pro and at all_star under the model: the contacts by kind and by whom
//   Jam.Core.Contact.Levers          skater.knockdown_frames scales the frames down; skater.check_speed_min scales the hit (5 / value), not the frames
//   Jam.Core.ContactModes            the modes that wrote those two levers (and the raw cut), model against neutral: knockdowns and frames down (a measurement)
//   Jam.Core.ContactTakes            the rookie takes of Jam.Bot.SeasonTakes over ten seeds, model against neutral: the AI's shots and the score (a measurement)
// Lane A101 (track F1, docs/experiments/CONTACT-2.md): every outcome reads.
//   Jam.Core.Contact.Boards          the boards are a body: a hit that pushes a victim within 1.5 m of the boards into them counts x contact.board_bonus and fires the
//                                    board read-out and event; along the boards, away from them, beyond 1.5 m and at board_bonus 1 it is the open-ice hit
//   Jam.Core.Contact.HitStop         the hold by outcome and ratio: stumble 1 or 2 frames, knockdown 2 to 5, bump 0; jam.hitstop scales it
//   Jam.Core.ContactBoardMatch       three pro matches: outcomes by kind, the share of hits near the boards; board_bonus 1 hashes as main at ee14438 did
// Lane A113 (track F1, R-0049 point 1, docs/experiments/CONTACT-3.md): a tackle is an event, not a rhythm.
//   Jam.Core.Contact.ReleaseGrace    a skater released from a knockdown takes a bump at most for contact.release_grace_frames; after it the same hit lands;
//                                    0 = the hit lands at once; the counter is hashed while it runs; the soft hit and the whiff count for the HUD
//   Jam.Core.Contact.HitStopPick     the hit the hold is for: one that involves the human is never hidden by a knockdown between two bots
//   Jam.Core.ContactGraceMatch       release_grace_frames 0 hashes as the baseline (three pro seeds); ten pro matches of 100 s, grace 0 / 90 / 120: knockdowns
//                                    of the human, figures down again within 2 s of their release
// Run:
//   UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Core.Contact; Quit" -unattended -nullrhi -nosplash -log
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "JamBotMatch.h"
#include "DifficultyResolve.h"
#include "JamCartridge.h"
#include "JamEventBus.h"
#include "JamPawns.h"
#include "JamPlaytester.h"
#include "JamSim.h"
#include "JamSimAdapter.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamContactTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;
	const FName FoulEvent(TEXT("foul"));

	// ---- the three-minute bot match ------------------------------------------------------------------------------------------------------------
	struct FContactCount
	{
		JamAI::FJamSimAdapter* Sim = nullptr;
		int32 Human = -1;
		int32 Bumps[2] = { 0, 0 };        // [0] on a bot, [1] on the human: bBumpThisFrame frames (the figure's PlayHitReaction)
		int32 Checks[2] = { 0, 0 };       // check events by a bot / by the human
		int32 Downs[2] = { 0, 0 };        // knockdown events of a bot / of the human
		int32 BotOnBotDowns = 0;          // a bot knocked down by a bot
		int32 Lifts[2] = { 0, 0 };        // lift presses by a bot / by the human
		// The model's read-outs, by victim (bot / human): hits resolved as a bump, a stumble, a knockdown; the attacker's recoils and the carrier's refused presses.
		int32 HitBump[2] = { 0, 0 }, HitStumble[2] = { 0, 0 }, HitDown[2] = { 0, 0 }, Recoil[2] = { 0, 0 }, Refused[2] = { 0, 0 };
		int32 CrossStumble[2] = { 0, 0 }, CrossDown[2] = { 0, 0 };   // of those, by a cross-check (the rest by a body check)
		int32 DownFrames[2] = { 0, 0 };                               // frames down, summed (the model's knockdowns only)
		// Lane A101: hits (any outcome) on a victim within ContactBoardRangeM of the boards; of the landed ones, those the boards answered (bContactBoard), by outcome;
		// and the ones the bonus promoted (the ratio without the boards' factor was under the threshold the hit passed).
		int32 NearBoards[2] = { 0, 0 }, BoardStumble[2] = { 0, 0 }, BoardDown[2] = { 0, 0 }, BoardPromotedStumble[2] = { 0, 0 }, BoardPromotedDown[2] = { 0, 0 };
		// Every resolved hit by where and how: [0 = within 1.5 m of the boards, 1 = 1.5 to 3 m][0 = into them (dot of the push and the outward normal 0.5 and up),
		// 1 = glancing (0 to 0.5), 2 = along or away][bump, stumble, down]. The push of a bump is the victim's BumpDir (stale when the bump read-out was gapped).
		int32 Where[2][3][3] = {};
		int32 BoardEvents = 0;                                        // `board` events with Kind skater (the hit's and a skater's own slide into the boards)
		// Lane A113: the release grace. By victim (bot / human): knockdowns (any cause) that begin within ReDownWindow frames of the same skater's release, of
		// those the ones a hit caused (ContactOutcome down), stumbles by a hit in the same window, hits the grace made a bump (bContactGrace) and of those the
		// ones a bot threw; the shortest release-to-down gap of the match; the human's HUD counters at the end.
		static constexpr int32 ReDownWindow = 120, MaxSk = 16;
		int32 ReDown[2] = { 0, 0 }, ReDownByHit[2] = { 0, 0 }, ReStumble[2] = { 0, 0 }, Graced[2] = { 0, 0 }, GracedByBot = 0, ReHitInGrace = 0;
		int32 MinGap = 1000000, GraceAtRelease = -1;
		int32 HumanSoft = 0, HumanMissed = 0, HumanGraced = 0;
		bool bWasDown[MaxSk] = {};
		int64 ReleasedAt[MaxSk] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
		int32 GraceBefore[MaxSk] = {};   // GraceFrames after the previous frame: over 1 means the skater was inside its grace when this frame's contacts resolved
		int64 LastFrame = -1;
	};

	void Trace(void* Ctx, const JamAI::FJamAIWorld&, const JamAI::FJamAIInput*, const JamAI::FJamBrain&, int64_t, bool)
	{
		FContactCount& C = *static_cast<FContactCount*>(Ctx);
		const UJamSim* Sim = C.Sim->GetSim();
		if (!Sim || Sim->GetFrame() == C.LastFrame) return;   // the trace is called every frame and again on a touch frame
		C.LastFrame = Sim->GetFrame();
		for (const FJamSkaterState& S : Sim->GetSkaters())
		{
			const int32 H = S.Index == C.Human ? 1 : 0;
			if (S.bBumpThisFrame) ++C.Bumps[H];
			if (S.bLiftThisFrame) ++C.Lifts[H];
			if (S.ContactOutcome == UJamSim::ContactBump) ++C.HitBump[H];
			if (S.ContactOutcome == UJamSim::ContactStumble) { ++C.HitStumble[H]; if (S.bContactCross) ++C.CrossStumble[H]; }
			if (S.ContactOutcome == UJamSim::ContactDown) { ++C.HitDown[H]; C.DownFrames[H] += S.KnockFrames; if (S.bContactCross) ++C.CrossDown[H]; }
			if (S.bRecoilThisFrame) ++C.Recoil[H];
			if (S.bCheckRefused) ++C.Refused[H];
			// Lane A113: the release grace.
			if (S.Index < FContactCount::MaxSk)
			{
				const int32 I = S.Index;
				const bool bDown = S.IsDown();
				const bool bInWindow = C.ReleasedAt[I] >= 0 && Sim->GetFrame() - C.ReleasedAt[I] < FContactCount::ReDownWindow;   // inside 2 s: frames 0 to 119 after the release
				if (bDown && !C.bWasDown[I] && C.ReleasedAt[I] >= 0)
				{
					C.MinGap = FMath::Min(C.MinGap, static_cast<int32>(Sim->GetFrame() - C.ReleasedAt[I]));
					if (bInWindow) { ++C.ReDown[H]; if (S.ContactOutcome == UJamSim::ContactDown) ++C.ReDownByHit[H]; }
				}
				if (S.ContactOutcome == UJamSim::ContactStumble && bInWindow) ++C.ReStumble[H];
				if (S.ContactOutcome >= UJamSim::ContactStumble && C.GraceBefore[I] > 1) ++C.ReHitInGrace;   // the rule's own promise: never
				if (S.bContactGrace) { ++C.Graced[H]; if (S.ContactBy != C.Human) ++C.GracedByBot; }
				if (!bDown && C.bWasDown[I]) { C.ReleasedAt[I] = Sim->GetFrame(); if (C.GraceAtRelease < 0) C.GraceAtRelease = S.GraceFrames; }
				C.bWasDown[I] = bDown;
				C.GraceBefore[I] = S.GraceFrames;
				if (H == 1) { C.HumanSoft = S.ChecksSoft; C.HumanMissed = S.ChecksMissed; C.HumanGraced = S.ChecksGraced; }
			}
			if (S.ContactOutcome != UJamSim::ContactNone)
			{
				FVector2D N;
				const double Gap = -Sim->BoardDistance(S.Pos, N);
				if (Gap <= UJamSim::ContactBoardRangeM) ++C.NearBoards[H];
				if (Gap <= 2.0 * UJamSim::ContactBoardRangeM)
				{
					const double Into = FVector2D::DotProduct(S.ContactOutcome == UJamSim::ContactDown ? S.KnockDir : S.BumpDir, N);
					++C.Where[Gap <= UJamSim::ContactBoardRangeM ? 0 : 1][Into >= 0.5 ? 0 : Into > 0.0 ? 1 : 2][S.ContactOutcome - 1];
				}
				if (S.bContactBoard)
				{
					const FJamContactConfig& K = Sim->GetConfig().Contact;
					const double Free = S.ContactRatio / Sim->BoardFactor(S, S.ContactOutcome == UJamSim::ContactDown ? S.KnockDir : S.BumpDir);
					if (S.ContactOutcome == UJamSim::ContactDown) { ++C.BoardDown[H]; if (Free < K.DownRatio) ++C.BoardPromotedDown[H]; }
					else { ++C.BoardStumble[H]; if (Free < K.BumpRatio) ++C.BoardPromotedStumble[H]; }
				}
			}
		}
		int32 Checker = -1;
		for (const FJamSimEvent& E : Sim->GetFrameEvents())
		{
			if (E.Name == JamEvents::Check) { Checker = E.Who; ++C.Checks[E.Who == C.Human ? 1 : 0]; }
			if (E.Name == JamEvents::Board && E.Who >= 0) ++C.BoardEvents;
			if (E.Name == JamEvents::Knockdown)
			{
				++C.Downs[E.Who == C.Human ? 1 : 0];
				if (E.Who != C.Human && Checker >= 0 && Checker != C.Human) ++C.BotOnBotDowns;
			}
		}
	}

	// hockey.regular at a level, penalties off and no score limit inside the three minutes (A73's cut), 10 800 frames, the Regular playtester as the human.
	// File = another cartridge (its own rules kept, Level null = its own level).
	JamAI::FJamPlaytestStats Match(FAutomationTestBase& T, const TCHAR* Level, uint64 Seed, FContactCount& Count, TFunctionRef<void(FJamGameConfig&)> Edit, const FString& File = FString(), int64 Frames = 10800)
	{
		TArray<FString> Errors;
		UJamCartridge* C = UJamCartridge::LoadFromFile(File.IsEmpty() ? UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")) : File, Errors);
		for (const FString& E : Errors) T.AddError(FString::Printf(TEXT("%s: %s"), File.IsEmpty() ? TEXT("hockey.regular") : *File, *E));
		if (!C) return JamAI::FJamPlaytestStats();
		if (File.IsEmpty())
		{
			C->Game.Rules.ScoreToWin = 99;
			C->Game.Rules.bPenalties = false;
		}
		Edit(C->Game);
		JamAI::FJamSimAdapter Sim(*C);
		Count.Sim = &Sim;
		const JamAI::FJamMatchConfig M = JamAI::ToMatchConfigResolved(*C, Level);
		Count.Human = M.PlayerIndex();
		return JamAI::RunPlaytestMatch(Sim, M, Seed, JamAI::EPlaytester::Regular, Frames, C->Game.Shot.ChargeFramesMax, false, &Trace, &Count, 1);
	}

	FString Line(const TCHAR* Label, const JamAI::FJamPlaytestStats& S, const FContactCount& C)
	{
		return FString::Printf(TEXT("%s: frames %lld hash %016llx goals %d:%d | bump read-outs bot %d human %d | check events by bot %d by human %d | knockdowns of bot %d of human %d (bot on bot %d) | ")
			TEXT("lift presses bot %d human %d | hits taken (bump/stumble/down) bot %d/%d/%d human %d/%d/%d | by cross-check (stumble/down) bot %d/%d human %d/%d | down frames bot %d human %d | recoils bot %d human %d | refused carrying bot %d human %d"),
			Label, static_cast<long long>(S.Frames), static_cast<unsigned long long>(S.Hash), S.GoalsFor, S.GoalsAgainst, C.Bumps[0], C.Bumps[1], C.Checks[0], C.Checks[1], C.Downs[0], C.Downs[1],
			C.BotOnBotDowns, C.Lifts[0], C.Lifts[1], C.HitBump[0], C.HitStumble[0], C.HitDown[0], C.HitBump[1], C.HitStumble[1], C.HitDown[1], C.CrossStumble[0], C.CrossDown[0], C.CrossStumble[1], C.CrossDown[1], C.DownFrames[0], C.DownFrames[1], C.Recoil[0], C.Recoil[1], C.Refused[0], C.Refused[1]);
	}

	// ---- the scenes --------------------------------------------------------------------------------------------------------------------------
	// hockey.regular (speed 11, mass 80, knockdown_frames 90), penalties off unless a scene turns them on; faceoff skipped. A = the human's skater, V = the first
	// skater of the other team; everybody else parked along the side boards, the puck far away.
	struct FContactScene
	{
		UJamSim* Sim = nullptr;
		int32 A = -1, V = -1;
		FJamGameConfig Cfg;
		bool Ok() const { return Sim && A >= 0 && V >= 0; }
		const FJamSkaterState& Att() const { return Sim->GetSkaters()[A]; }
		const FJamSkaterState& Vic() const { return Sim->GetSkaters()[V]; }
	};

	FContactScene Make(FAutomationTestBase& T, TFunctionRef<void(FJamGameConfig&)> Edit)
	{
		FContactScene S;
		TArray<FString> Errors;
		UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")), Errors);
		for (const FString& E : Errors) T.AddError(FString::Printf(TEXT("hockey.regular: %s"), *E));
		if (!C) return S;
		S.Cfg = C->Game;
		S.Cfg.Rules.bPenalties = false;
		Edit(S.Cfg);
		S.Sim = NewObject<UJamSim>();
		S.Sim->Init(S.Cfg, 7);
		S.Sim->SkipFaceoff();
		S.A = S.Sim->GetHumanIndex();
		const int32 Team = S.Sim->GetSkaters()[S.A].Team;
		for (const FJamSkaterState& K : S.Sim->GetSkaters()) if (K.Team != Team) { S.V = K.Index; break; }
		const double Hx = S.Cfg.Rink.LengthM * 0.5 - 4.0, Hz = S.Cfg.Rink.WidthM * 0.5 - 3.0;
		int32 N = 0;
		for (const FJamSkaterState& K : S.Sim->GetSkaters())
		{
			if (K.Index == S.A || K.Index == S.V) continue;
			S.Sim->SetSkater(K.Index, FVector2D(-Hx + 2.5 * N, (N % 2 ? 1.0 : -1.0) * Hz), FVector2D::ZeroVector, 0.0);
			++N;
		}
		S.Sim->SetPuck(FVector2D(-20.0, 12.0), FVector2D::ZeroVector);
		return S;
	}

	struct FContactHit
	{
		int32 Outcome = 0;          // the victim's ContactOutcome on the frame of the hit
		float Ratio = 0.f;
		int32 KnockFrames = 0, StumbleFrames = 0;
		bool bCheck = false, bKnock = false, bLiftClean = false, bRecoil = false, bRefused = false, bWindow = false;
		FName Foul;
		float Strength = 0.f;
		int32 AttStumble = 0;
		double AttSpeedBefore = 0, AttSpeedAfter = 0, VicSpeedBefore = 0, VicSpeedAfter = 0;
		int32 Frame = -1;
	};

	// Steps up to Frames; Fill writes the inputs of frame F. Stops on the frame the victim takes a hit (ContactOutcome != 0), a foul or a knockdown.
	FContactHit Run(FContactScene& S, int32 Frames, TFunctionRef<void(TArray<FJamInput>&, int32)> Fill)
	{
		FContactHit H;
		const int32 N = S.Sim->GetSkaters().Num();
		for (int32 F = 0; F < Frames; ++F)
		{
			TArray<FJamInput> In;
			In.SetNum(N);
			Fill(In, F);
			H.AttSpeedBefore = S.Att().Vel.Size();
			H.VicSpeedBefore = S.Vic().Vel.Size();
			S.Sim->Step(In);
			for (const FJamSimEvent& E : S.Sim->GetFrameEvents())
			{
				if (E.Name == JamEvents::Check && E.Who == S.A) { H.bCheck = true; H.Strength = E.Strength; }
				if (E.Name == JamEvents::Knockdown && E.Who == S.V) H.bKnock = true;
				if (E.Name == JamEvents::Lift && E.Who == S.A && E.Strength > 0.5f) H.bLiftClean = true;
				if (E.Name == FoulEvent && E.Who == S.A) H.Foul = E.Kind;
			}
			H.bRefused |= S.Att().bCheckRefused;
			H.bWindow |= S.Att().CheckWindow > 0 || S.Att().bCheckThisFrame;
			if (S.Vic().ContactOutcome != 0 || H.bKnock || !H.Foul.IsNone())
			{
				H.Outcome = S.Vic().ContactOutcome;
				H.Ratio = S.Vic().ContactRatio;
				H.KnockFrames = S.Vic().KnockFrames;
				H.StumbleFrames = S.Vic().StumbleFrames;
				H.bRecoil = S.Att().bRecoilThisFrame;
				H.AttStumble = S.Att().StumbleFrames;
				H.AttSpeedAfter = S.Att().Vel.Size();
				H.VicSpeedAfter = S.Vic().Vel.Size();
				H.Frame = F;
				return H;
			}
		}
		return H;
	}

	// The victim takes the puck on its blade side and is the carrier (the puck 0.45 m ahead of it, as the acceptance drill hands it).
	bool HandPuck(FContactScene& S, int32 To)
	{
		for (int32 F = 0; F < 40; ++F)
		{
			const FJamSkaterState& K = S.Sim->GetSkaters()[To];
			if (S.Sim->CarrierIndex() == To) return true;
			if (F == 0) S.Sim->SetPuck(K.Pos + K.Forward() * 0.45, K.Vel);
			TArray<FJamInput> In;
			In.SetNum(S.Sim->GetSkaters().Num());
			S.Sim->Step(In);
		}
		return S.Sim->CarrierIndex() == To;
	}
	void Carry(FContactScene& S, int32 Who, const FVector2D& Pos, const FVector2D& Vel, double Yaw)
	{
		S.Sim->SetSkater(Who, Pos, Vel, Yaw);
		S.Sim->SetPuck(Pos + FVector2D(FMath::Cos(Yaw), FMath::Sin(Yaw)) * 0.45, Vel);
	}

	FString Facts(const FContactHit& H)
	{
		return FString::Printf(TEXT("outcome %d ratio %.2f knock %d stumble %d check %d strength %.2f recoil %d att %.2f -> %.2f m/s vic %.2f -> %.2f m/s frame %d"), H.Outcome, H.Ratio, H.KnockFrames,
			H.StumbleFrames, H.bCheck ? 1 : 0, H.Strength, H.bRecoil ? 1 : 0, H.AttSpeedBefore, H.AttSpeedAfter, H.VicSpeedBefore, H.VicSpeedAfter, H.Frame);
	}
	const auto NoEdit = [](FJamGameConfig&) {};

	// Lane A113: the grace part of a match line.
	FString GraceLine(const FContactCount& C)
	{
		return FString::Printf(TEXT("knockdowns of human %d of bot %d | down again within %d frames of release: human %d bot %d (by a hit: human %d bot %d) | stumbled by a hit in that window: human %d bot %d | ")
			TEXT("shortest release-to-down gap %d frames | hits the grace made a bump: on human %d on bot %d (thrown by a bot %d) | landed hits on a skater inside its grace %d | ")
			TEXT("grace at the first release %d | human's own: soft %d missed %d met-a-grace %d"),
			C.Downs[1], C.Downs[0], FContactCount::ReDownWindow, C.ReDown[1], C.ReDown[0], C.ReDownByHit[1], C.ReDownByHit[0], C.ReStumble[1], C.ReStumble[0],
			C.MinGap == 1000000 ? -1 : C.MinGap, C.Graced[1], C.Graced[0], C.GracedByBot, C.ReHitInGrace, C.GraceAtRelease, C.HumanSoft, C.HumanMissed, C.HumanGraced);
	}

	// Lane A101: the board part of a match line.
	FString BoardLine(const FContactCount& C)
	{
		const int32 Hits = C.HitBump[0] + C.HitBump[1] + C.HitStumble[0] + C.HitStumble[1] + C.HitDown[0] + C.HitDown[1];
		const int32 Near = C.NearBoards[0] + C.NearBoards[1];
		FString Bins;
		for (int32 B = 0; B < 2; ++B) Bins += FString::Printf(TEXT(" | %s (bump/stumble/down): into %d/%d/%d glancing %d/%d/%d along-or-away %d/%d/%d"), B == 0 ? TEXT("within 1.5 m") : TEXT("1.5 to 3 m"),
			C.Where[B][0][0], C.Where[B][0][1], C.Where[B][0][2], C.Where[B][1][0], C.Where[B][1][1], C.Where[B][1][2], C.Where[B][2][0], C.Where[B][2][1], C.Where[B][2][2]);
		return FString::Printf(TEXT("hits resolved %d, near the boards (1.5 m) %d = %.1f %% (bot %d human %d) | the boards answered: stumble bot %d human %d, down bot %d human %d | promoted by the bonus: to a stumble bot %d human %d, to a knockdown bot %d human %d | board events (skater) %d"),
			Hits, Near, Hits > 0 ? 100.0 * Near / Hits : 0.0, C.NearBoards[0], C.NearBoards[1], C.BoardStumble[0], C.BoardStumble[1], C.BoardDown[0], C.BoardDown[1],
			C.BoardPromotedStumble[0], C.BoardPromotedStumble[1], C.BoardPromotedDown[0], C.BoardPromotedDown[1], C.BoardEvents) + Bins;
	}
}

using namespace JamContactTests;

// ---- Lane A101 (track F1): every outcome reads -------------------------------------------------------------------------------------------------

// The boards are a body. One hit, five places: a side body check on a standing skater (facing along the boards or +x, the hit from its side, seen and braced).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamContactBoards, "Jam.Core.Contact.Boards", JamContactTests::Flags)
bool FJamContactBoards::RunTest(const FString& Parameters)
{
	struct FOut { FContactHit H; bool bBoardReadout = false, bContactBoard = false; int32 BoardEvents = 0, BoardHits = 0; float BoardStrength = -1.f; FVector2D BoardDir = FVector2D::ZeroVector; double BoardGap = -1.0; uint64 Hash = 0; };
	// Speed = the attacker's speed along Dir; the victim stands at VPos facing VYaw, the attacker starts 1 m away along -Dir.
	auto Hit = [&](const TCHAR* Name, double Bonus, const FVector2D& VPos, double VYaw, const FVector2D& Dir, double Speed) -> FOut
	{
		FOut O;
		FContactScene S = Make(*this, [&](FJamGameConfig& G) { G.Contact.BoardBonus = Bonus; });
		if (!S.Ok()) return O;
		S.Sim->SetSkater(S.V, VPos, FVector2D::ZeroVector, VYaw);
		S.Sim->SetSkater(S.A, VPos - Dir * 1.0, Dir * Speed, FMath::Atan2(Dir.Y, Dir.X));
		O.H = Run(S, 12, [&](TArray<FJamInput>& In, int32 F) { In[S.A].bCheck = F == 0; });
		O.bBoardReadout = S.Vic().bBoardHitThisFrame;
		O.bContactBoard = S.Vic().bContactBoard;
		O.BoardHits = S.Vic().ContactBoardHits;
		for (const FJamSimEvent& E : S.Sim->GetFrameEvents())
		{
			if (E.Name != JamEvents::Board || E.Who != S.V) continue;
			++O.BoardEvents;
			O.BoardStrength = E.Strength;
			O.BoardDir = E.Dir;
			FVector2D N;
			O.BoardGap = FMath::Abs(S.Sim->BoardDistance(E.Where, N));
		}
		// The echo: the victim's own slide into the boards inside half a second is the hit already announced.
		for (int32 F = 0; F < 30; ++F)
		{
			TArray<FJamInput> In;
			In.SetNum(S.Sim->GetSkaters().Num());
			S.Sim->Step(In);
			for (const FJamSimEvent& E : S.Sim->GetFrameEvents()) if (E.Name == JamEvents::Board && E.Who == S.V) ++O.BoardEvents;
		}
		O.Hash = S.Sim->StateHash();
		AddInfo(FString::Printf(TEXT("JAMBOARDS %s (board_bonus %.2f, %.1f m/s): %s | board read-out %d contact_board %d board events %d strength %.2f"), Name, Bonus, Speed, *Facts(O.H),
			O.bBoardReadout ? 1 : 0, O.bContactBoard ? 1 : 0, O.BoardEvents, O.BoardStrength));
		return O;
	};
	FJamGameConfig Cfg;
	{
		FContactScene S0 = Make(*this, NoEdit);
		if (!S0.Ok()) return false;
		Cfg = S0.Cfg;
	}
	TestEqual(TEXT("contact.board_bonus defaults to 1.25"), Cfg.Contact.BoardBonus, 1.25);
	const double Hz = Cfg.Rink.WidthM * 0.5;
	const FVector2D In(0.0, 1.0);   // into the +z side boards
	const double Bonus = Cfg.Contact.BoardBonus;
	// 1. Open ice: the reference hit (4 m/s: a stumble, CONTACT.md's 0.61).
	const FOut Open = Hit(TEXT("open ice"), Bonus, FVector2D::ZeroVector, 0.0, In, 4.0);
	TestEqual(TEXT("open ice, 4 m/s on a standing skater: a stumble"), Open.H.Outcome, UJamSim::ContactStumble);
	TestTrue(TEXT("open ice: no board read-out, no board event"), !Open.bBoardReadout && !Open.bContactBoard && Open.BoardEvents == 0);
	// 2. The same hit 1 m from the boards, straight into them: the ratio counts x board_bonus, the read-out and the event fire with the check's strength.
	const FOut Into = Hit(TEXT("1 m from the boards, into them"), Bonus, FVector2D(0.0, Hz - 1.0), 0.0, In, 4.0);
	TestTrue(*FString::Printf(TEXT("into the boards: ratio %.3f = open ice %.3f x %.2f"), Into.H.Ratio, Open.H.Ratio, Bonus), FMath::IsNearlyEqual(static_cast<double>(Into.H.Ratio), Open.H.Ratio * Bonus, 0.005));
	TestTrue(TEXT("into the boards: the victim's board read-out fires (PlayBoardHit) and the hit is counted"), Into.bBoardReadout && Into.bContactBoard && Into.BoardHits == 1);
	TestEqual(TEXT("into the boards: one board event for the victim, also through its slide into the boards (the echo is the same hit)"), Into.BoardEvents, 1);
	TestTrue(*FString::Printf(TEXT("... with the check's strength (%.2f = %.2f)"), Into.BoardStrength, Into.H.Strength), Into.H.bCheck && FMath::IsNearlyEqual(Into.BoardStrength, Into.H.Strength, 0.001f));
	TestTrue(TEXT("... on the boards, its Dir the normal into the rink"), Into.BoardGap < 0.01 && FVector2D::DotProduct(Into.BoardDir, In) < -0.99);
	// 3. Along the boards, away from them, and beyond the range: the open-ice hit.
	const FOut Along = Hit(TEXT("1 m from the boards, along them"), Bonus, FVector2D(0.0, Hz - 1.0), PI * 0.5, FVector2D(1.0, 0.0), 4.0);
	const FOut Away = Hit(TEXT("1.45 m from the boards, away from them"), Bonus, FVector2D(0.0, Hz - 1.45), 0.0, -In, 4.0);
	const FOut Far = Hit(TEXT("2 m from the boards, into them"), Bonus, FVector2D(0.0, Hz - 2.0), 0.0, In, 4.0);
	for (const FOut* O : { &Along, &Away, &Far })
	{
		TestTrue(*FString::Printf(TEXT("no boards in the hit: ratio %.3f = open ice %.3f"), O->H.Ratio, Open.H.Ratio), FMath::IsNearlyEqual(O->H.Ratio, Open.H.Ratio, 0.005f));
		TestTrue(TEXT("... and no board read-out at the hit"), !O->bBoardReadout && !O->bContactBoard);
	}
	// 4. Half the bonus at 75.5 degrees off straight in (dot 0.25): the factor is linear in 2 x dot up to 60 degrees.
	{
		const FVector2D Dir = FVector2D(FMath::Sqrt(1.0 - 0.25 * 0.25), 0.25);
		const FOut Half = Hit(TEXT("1 m from the boards, dot 0.25"), Bonus, FVector2D(0.0, Hz - 1.0), FMath::Atan2(Dir.Y, Dir.X) + PI * 0.5, Dir, 4.0);
		TestTrue(*FString::Printf(TEXT("dot 0.25: ratio %.3f = open ice x %.3f"), Half.H.Ratio, 1.0 + (Bonus - 1.0) * 0.5), FMath::IsNearlyEqual(static_cast<double>(Half.H.Ratio), Open.H.Ratio * (1.0 + (Bonus - 1.0) * 0.5), 0.01));
	}
	// 5. The hit the boards decide: 6.5 m/s stumbles a braced skater on open ice (0.89) and puts it down against the boards (1.11).
	const FOut OpenHard = Hit(TEXT("open ice"), Bonus, FVector2D::ZeroVector, 0.0, In, 6.5);
	const FOut IntoHard = Hit(TEXT("1 m from the boards, into them"), Bonus, FVector2D(0.0, Hz - 1.0), 0.0, In, 6.5);
	TestEqual(TEXT("6.5 m/s on open ice: a stumble"), OpenHard.H.Outcome, UJamSim::ContactStumble);
	TestEqual(TEXT("6.5 m/s into the boards: down"), IntoHard.H.Outcome, UJamSim::ContactDown);
	TestTrue(TEXT("... with the board read-out"), IntoHard.bBoardReadout && IntoHard.H.bKnock);
	// 6. The neutral value: board_bonus 1 is the open-ice hit at the boards: same ratio, no read-out, no event; and the state after it hashes as a sim that
	// never heard of the boards would (the same scene with the bonus left at its default but the victim's hit resolved 2 m out differs: so compare 1.0 against 1.0 twice
	// and against the default, which must differ once the knockdown lasts longer).
	const FOut Neutral = Hit(TEXT("1 m from the boards, into them"), 1.0, FVector2D(0.0, Hz - 1.0), 0.0, In, 6.5);
	TestTrue(*FString::Printf(TEXT("board_bonus 1: ratio %.3f = open ice %.3f"), Neutral.H.Ratio, OpenHard.H.Ratio), FMath::IsNearlyEqual(Neutral.H.Ratio, OpenHard.H.Ratio, 0.005f));
	TestEqual(TEXT("board_bonus 1: a stumble, as on open ice"), Neutral.H.Outcome, UJamSim::ContactStumble);
	TestTrue(TEXT("board_bonus 1: no board read-out and no board event at the hit"), !Neutral.bBoardReadout && !Neutral.bContactBoard && Neutral.BoardHits == 0);
	TestTrue(TEXT("board_bonus 1 and 1.25 end in different states (the bonus is a rule), two runs at 1 in the same"), Neutral.Hash != IntoHard.Hash
		&& Neutral.Hash == Hit(TEXT("1 m from the boards, into them (again)"), 1.0, FVector2D(0.0, Hz - 1.0), 0.0, In, 6.5).Hash);
	// 7. The key loads, its range holds.
	FString Json;
	if (TestTrue(TEXT("hockey.regular reads"), FFileHelper::LoadFileToString(Json, *UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")))))
	{
		TArray<FString> Errors;
		FString One = Json;
		TestEqual(TEXT("one game.skater anchor"), One.ReplaceInline(TEXT("\"skater\": {"), TEXT("\"contact\": { \"board_bonus\": 1 }, \"skater\": {")), 1);
		UJamCartridge* N = UJamCartridge::LoadFromString(One, Errors);
		for (const FString& E : Errors) AddError(FString::Printf(TEXT("board_bonus 1: %s"), *E));
		if (TestNotNull(TEXT("contact.board_bonus 1 loads"), N)) TestEqual(TEXT("... and reads back"), N->Game.Contact.BoardBonus, 1.0);
		FString Bad = Json;
		Bad.ReplaceInline(TEXT("\"skater\": {"), TEXT("\"contact\": { \"board_bonus\": 0.5 }, \"skater\": {"));
		Errors.Reset();
		UJamCartridge* B = UJamCartridge::LoadFromString(Bad, Errors);
		TestTrue(TEXT("contact.board_bonus under 1 is out of range"), B == nullptr || Errors.Num() > 0);
	}
	return true;
}

// ---- Lane A113 (track F1, R-0049 point 1): a tackle is an event, not a rhythm -------------------------------------------------------------------

// One head-on hit at full speed puts the victim down; it is released; the same hit comes again Wait frames after the release.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamContactReleaseGrace, "Jam.Core.Contact.ReleaseGrace", JamContactTests::Flags)
bool FJamContactReleaseGrace::RunTest(const FString& Parameters)
{
	struct FOut { bool bOk = false; FContactHit First, Second; int32 GraceAtRelease = -1, GraceAfter30 = -1, GraceAtHit = -1, DownFor = 0, AttGraced = 0, AttSoft = 0, VicGraced = 0, AttStumble = 0; bool bGraceFlag = false; uint64 Hash = 0; };
	auto Scene = [&](const TCHAR* Name, TFunctionRef<void(FJamGameConfig&)> Edit, int32 Wait) -> FOut
	{
		FOut O;
		FContactScene S = Make(*this, Edit);
		if (!S.Ok()) return O;
		const double Top = S.Cfg.Skater.Speed;
		auto Charge = [&]()
		{
			S.Sim->SetSkater(S.V, FVector2D(0.6, 0.0), FVector2D(-Top, 0.0), PI);
			S.Sim->SetSkater(S.A, FVector2D(-0.6, 0.0), FVector2D(Top, 0.0), 0.0);
			return Run(S, 12, [&](TArray<FJamInput>& In, int32 F) { In[S.A].bCheck = F == 0; });
		};
		O.First = Charge();
		if (!TestTrue(*FString::Printf(TEXT("%s: the first hit puts the victim down"), Name), O.First.bKnock && S.Vic().IsDown())) return O;
		TArray<FJamInput> None;
		None.SetNum(S.Sim->GetSkaters().Num());
		// Park the attacker away from the fallen victim, wait for the release.
		S.Sim->SetSkater(S.A, FVector2D(-12.0, 0.0), FVector2D::ZeroVector, 0.0);
		while (S.Vic().IsDown() && O.DownFor < 400) { S.Sim->Step(None); ++O.DownFor; }
		O.GraceAtRelease = S.Vic().GraceFrames;
		for (int32 F = 0; F < Wait; ++F)
		{
			S.Sim->Step(None);
			if (F == 29) O.GraceAfter30 = S.Vic().GraceFrames;
		}
		O.GraceAtHit = S.Vic().GraceFrames;
		O.Second = Charge();
		O.bGraceFlag = S.Vic().bContactGrace;
		O.AttGraced = S.Att().ChecksGraced;
		O.AttSoft = S.Att().ChecksSoft;
		O.VicGraced = S.Vic().ContactGraced;
		O.AttStumble = S.Att().StumbleFrames;
		O.Hash = S.Sim->StateHash();
		O.bOk = true;
		AddInfo(FString::Printf(TEXT("JAMGRACE scene %s: down for %d frames, grace at release %d, after 30 frames %d, at the second charge %d | second hit: %s | grace flag %d attacker graced %d soft %d"),
			Name, O.DownFor, O.GraceAtRelease, O.GraceAfter30, O.GraceAtHit, *Facts(O.Second), O.bGraceFlag ? 1 : 0, O.AttGraced, O.AttSoft));
		return O;
	};
	const FJamContactConfig K;
	TestEqual(TEXT("the default grace is 120 frames (R-0051)"), K.ReleaseGraceFrames, 120);
	// 1. Inside the grace: the biggest hit in the game is a bump.
	const FOut In = Scene(TEXT("grace 120, hit 10 frames after the release"), NoEdit, 10);
	if (!In.bOk) return false;
	TestEqual(TEXT("the release starts the grace at contact.release_grace_frames"), In.GraceAtRelease, 120);
	TestTrue(*FString::Printf(TEXT("the grace is running at the second charge (%d)"), In.GraceAtHit), In.GraceAtHit > 90 && In.GraceAtHit <= 110);
	TestEqual(TEXT("inside the grace: a bump"), In.Second.Outcome, UJamSim::ContactBump);
	TestTrue(TEXT("... no check event, no knockdown event, the victim stays up and in control"), !In.Second.bCheck && !In.Second.bKnock && In.Second.KnockFrames == 0 && In.Second.StumbleFrames == 0);
	TestTrue(*FString::Printf(TEXT("... the read-out keeps the hit's real ratio (%.2f, past down_ratio)"), In.Second.Ratio), In.Second.Ratio >= K.DownRatio);
	TestTrue(TEXT("... the victim's grace read-out and the attacker's counter say why (no TOO SOFT for it)"), In.bGraceFlag && In.VicGraced == 1 && In.AttGraced == 1 && In.AttSoft == 0);
	TestTrue(*FString::Printf(TEXT("... the same hit landed the first time (ratio %.2f, down %d frames)"), In.First.Ratio, In.First.KnockFrames), In.First.Outcome == UJamSim::ContactDown && In.First.bKnock);
	TestTrue(*FString::Printf(TEXT("... the attacker pays as for any bump: this full-speed charge bounces it (recoil, %d frames of stumble)"), In.AttStumble), In.Second.bRecoil && In.AttStumble > 0);
	// 2. The countdown: 30 frames on, 90 are left; after the grace the same hit lands again.
	const FOut After = Scene(TEXT("grace 120, hit 130 frames after the release"), NoEdit, 130);
	TestEqual(TEXT("30 frames after the release 90 frames of grace are left"), After.GraceAfter30, 90);
	TestEqual(TEXT("130 frames after the release the grace is over"), After.GraceAtHit, 0);
	TestTrue(TEXT("after the grace the same hit lands: down, with its events"), After.Second.Outcome == UJamSim::ContactDown && After.Second.bCheck && After.Second.bKnock && !After.bGraceFlag && After.AttGraced == 0);
	// 3. The neutral value: no grace, the hit 10 frames after the release lands (the rhythm device B played).
	const FOut Zero = Scene(TEXT("grace 0, hit 10 frames after the release"), [](FJamGameConfig& G) { G.Contact.ReleaseGraceFrames = 0; }, 10);
	TestTrue(TEXT("release_grace_frames 0: no grace starts"), Zero.GraceAtRelease == 0 && Zero.GraceAtHit == 0);
	TestTrue(TEXT("release_grace_frames 0: the hit right after the release lands"), Zero.Second.Outcome == UJamSim::ContactDown && Zero.Second.bKnock && !Zero.bGraceFlag);
	TestTrue(TEXT("grace 0 and the default grace end in different states (the grace is a rule)"), Zero.Hash != In.Hash);
	// 4. The grace belongs to the momentum model: lane A92's neutral block starts none (it hashes as the game before the model).
	const FOut Old = Scene(TEXT("contact.momentum false"), [](FJamGameConfig& G) { G.Contact.SetNeutral(); }, 10);
	TestTrue(TEXT("contact.momentum false: no grace starts, the old rule's hit lands"), Old.GraceAtRelease == 0 && Old.Second.bKnock);
	// 5. A longer grace is a longer grace.
	const FOut Long = Scene(TEXT("grace 200, hit 100 frames after the release"), [](FJamGameConfig& G) { G.Contact.ReleaseGraceFrames = 200; }, 100);
	TestTrue(TEXT("release_grace_frames 200: 100 frames after the release the hit is still a bump"), Long.GraceAtRelease == 200 && Long.Second.Outcome == UJamSim::ContactBump && !Long.Second.bKnock);
	// 6. The counter is state: the hash sees it (one field changed on a live sim, then put back).
	{
		FContactScene S = Make(*this, NoEdit);
		if (S.Ok())
		{
			FJamSkaterState& V = const_cast<FJamSkaterState&>(S.Vic());   // test only
			const uint64 H0 = S.Sim->StateHash();
			V.GraceFrames = 5;
			const uint64 H5 = S.Sim->StateHash();
			V.GraceFrames = 6;
			const uint64 H6 = S.Sim->StateHash();
			V.GraceFrames = 0;
			TestTrue(TEXT("StateHash() covers GraceFrames (0 / 5 / 6 differ) and gives the hash back at 0"), H0 != H5 && H5 != H6 && S.Sim->StateHash() == H0);
			V.bContactGrace = true; V.ChecksSoft = 3; V.ChecksMissed = 2; V.ChecksGraced = 1; V.ContactGraced = 4;
			TestEqual(TEXT("the read-outs (bContactGrace, ChecksSoft, ChecksMissed, ChecksGraced, ContactGraced) are not hashed"), S.Sim->StateHash(), H0);
		}
	}
	// 7. The soft hit and the whiff count on the attacker (the HUD's TOO SOFT and MISSED).
	{
		FContactScene S = Make(*this, NoEdit);
		if (S.Ok())
		{
			// Both gliding the same way at the same speed, the attacker behind: a bump (lane A92's SameDirection).
			S.Sim->SetSkater(S.V, FVector2D::ZeroVector, FVector2D(8.0, 0.0), 0.0);
			S.Sim->SetSkater(S.A, FVector2D(-0.78, 0.0), FVector2D(8.0, 0.0), 0.0);
			const FContactHit H = Run(S, 12, [&](TArray<FJamInput>& In2, int32 F) { In2[S.A].bCheck = F == 0; In2[S.A].Move = FVector2D(0.7, 0.0); In2[S.V].Move = FVector2D(0.7, 0.0); });
			AddInfo(FString::Printf(TEXT("JAMGRACE soft: %s | attacker soft %d missed %d graced %d"), *Facts(H), S.Att().ChecksSoft, S.Att().ChecksMissed, S.Att().ChecksGraced));
			TestTrue(TEXT("a pressed check that resolves as a bump counts as soft on the attacker, not as a miss"), H.Outcome == UJamSim::ContactBump && S.Att().ChecksSoft == 1 && S.Att().ChecksMissed == 0 && S.Att().ChecksGraced == 0);
		}
		FContactScene W = Make(*this, NoEdit);
		if (W.Ok())
		{
			// Nobody near: the window (12 frames for the human) runs out.
			W.Sim->SetSkater(W.V, FVector2D(15.0, 0.0), FVector2D::ZeroVector, PI);
			W.Sim->SetSkater(W.A, FVector2D(-15.0, 0.0), FVector2D(4.0, 0.0), 0.0);
			int32 MissedAt = -1;
			for (int32 F = 0; F < 30; ++F)
			{
				TArray<FJamInput> In2;
				In2.SetNum(W.Sim->GetSkaters().Num());
				In2[W.A].bCheck = F == 0;
				W.Sim->Step(In2);
				if (MissedAt < 0 && W.Att().ChecksMissed > 0) MissedAt = F;
			}
			AddInfo(FString::Printf(TEXT("JAMGRACE whiff: missed %d at frame %d soft %d"), W.Att().ChecksMissed, MissedAt, W.Att().ChecksSoft));
			TestTrue(TEXT("a press that meets nobody counts one miss when its window closes, and no soft hit"), W.Att().ChecksMissed == 1 && W.Att().ChecksSoft == 0);
			TestEqual(TEXT("... on the frame the human's window (12 frames) closes"), MissedAt, UJamSim::HumanCheckWindowFrames);
		}
	}
	// 8. The key loads, its range holds.
	FString Json;
	if (TestTrue(TEXT("hockey.regular reads"), FFileHelper::LoadFileToString(Json, *UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")))))
	{
		TArray<FString> Errors;
		FString One = Json;
		TestEqual(TEXT("one game.skater anchor"), One.ReplaceInline(TEXT("\"skater\": {"), TEXT("\"contact\": { \"release_grace_frames\": 0 }, \"skater\": {")), 1);
		UJamCartridge* N = UJamCartridge::LoadFromString(One, Errors);
		for (const FString& E : Errors) AddError(FString::Printf(TEXT("release_grace_frames 0: %s"), *E));
		if (TestNotNull(TEXT("contact.release_grace_frames 0 loads"), N)) TestEqual(TEXT("... and reads back"), N->Game.Contact.ReleaseGraceFrames, 0);
		for (const TCHAR* BadValue : { TEXT("-1"), TEXT("301"), TEXT("1.5") })
		{
			FString Bad = Json;
			Bad.ReplaceInline(TEXT("\"skater\": {"), *FString::Printf(TEXT("\"contact\": { \"release_grace_frames\": %s }, \"skater\": {"), BadValue));
			Errors.Reset();
			UJamCartridge* B = UJamCartridge::LoadFromString(Bad, Errors);
			TestTrue(*FString::Printf(TEXT("contact.release_grace_frames %s is refused"), BadValue), B == nullptr || Errors.Num() > 0);
		}
	}
	return true;
}

// The hit the hold is for (AJamSkater::HitStopPick; the game mode's HitStopRoute holds on it).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamContactHitStopPick, "Jam.Core.Contact.HitStopPick", JamContactTests::Flags)
bool FJamContactHitStopPick::RunTest(const FString& Parameters)
{
	const FName Cause(TEXT("check")), Front(TEXT("front")), SideL(TEXT("side_l"));
	const int32 Human = 0;
	auto Skaters = []() { TArray<FJamSkaterState> Sk; Sk.SetNum(6); for (int32 I = 0; I < 6; ++I) Sk[I].Index = I; return Sk; };
	auto Event = [](FName Name, int32 Who, float Strength, FName Kind) { FJamSimEvent E; E.Name = Name; E.Who = Who; E.Strength = Strength; E.Kind = Kind; return E; };
	const FJamContactConfig K;
	// 1. A stumble of the human, alone: its check event's strength (the log line used to print 0.00).
	{
		TArray<FJamSkaterState> Sk = Skaters();
		Sk[Human].ContactOutcome = UJamSim::ContactStumble; Sk[Human].ContactRatio = 0.8f; Sk[Human].ContactBy = 4;
		const TArray<FJamSimEvent> Ev = { Event(JamEvents::Check, 4, 0.45f, SideL) };
		const FJamHitStopPick P = AJamSkater::HitStopPick(Sk, Ev, Human, K.BumpRatio);
		TestTrue(TEXT("a stumble the human takes: victim, checker, outcome, ratio"), P.Victim == Human && P.Checker == 4 && P.Outcome == UJamSim::ContactStumble && FMath::IsNearlyEqual(P.Ratio, 0.8f));
		TestTrue(*FString::Printf(TEXT("... with its check event's strength (%.2f) and the kind stumble"), P.Strength), FMath::IsNearlyEqual(P.Strength, 0.45f) && P.Kind == FName(TEXT("stumble")));
		TestEqual(TEXT("... held 2 frames (ratio 0.8, past the middle of the band)"), AJamSkater::HitStopFramesFor(P.Outcome, P.Ratio, K, 3), 2);
	}
	// 2. The same frame also carries a knockdown between two bots: the human's stumble is still the hit (the route held nothing).
	{
		TArray<FJamSkaterState> Sk = Skaters();
		Sk[Human].ContactOutcome = UJamSim::ContactStumble; Sk[Human].ContactRatio = 0.5f; Sk[Human].ContactBy = 4;
		Sk[2].ContactOutcome = UJamSim::ContactDown; Sk[2].ContactRatio = 2.f; Sk[2].ContactBy = 5;
		const TArray<FJamSimEvent> Ev = { Event(JamEvents::Check, 4, 0.3f, SideL), Event(JamEvents::Check, 5, 0.9f, Front), Event(JamEvents::Knockdown, 2, 0.9f, Cause) };
		const FJamHitStopPick P = AJamSkater::HitStopPick(Sk, Ev, Human, K.BumpRatio);
		TestTrue(TEXT("a bot-on-bot knockdown and a stumble of the human in one frame: the stumble is picked"), P.Victim == Human && P.Checker == 4 && P.Outcome == UJamSim::ContactStumble);
		TestTrue(*FString::Printf(TEXT("... with its own strength (%.2f), 1 frame"), P.Strength), FMath::IsNearlyEqual(P.Strength, 0.3f) && AJamSkater::HitStopFramesFor(P.Outcome, P.Ratio, K, 3) == 1);
	}
	// 3. Two knockdowns in one frame, the human's fall listed first: the human's is the hit, with its own checker and kind.
	{
		TArray<FJamSkaterState> Sk = Skaters();
		Sk[Human].ContactOutcome = UJamSim::ContactDown; Sk[Human].ContactRatio = 1.4f; Sk[Human].ContactBy = 4;
		Sk[2].ContactOutcome = UJamSim::ContactDown; Sk[2].ContactRatio = 3.f; Sk[2].ContactBy = 5;
		const TArray<FJamSimEvent> Ev = { Event(JamEvents::Check, 4, 0.5f, SideL), Event(JamEvents::Knockdown, Human, 0.5f, Cause), Event(JamEvents::Check, 5, 1.f, Front), Event(JamEvents::Knockdown, 2, 1.f, Cause) };
		const FJamHitStopPick P = AJamSkater::HitStopPick(Sk, Ev, Human, K.BumpRatio);
		TestTrue(TEXT("two knockdowns in one frame: the human's fall is picked, not the last one"), P.Victim == Human && P.Checker == 4 && P.Kind == SideL && FMath::IsNearlyEqual(P.Strength, 0.5f) && FMath::IsNearlyEqual(P.Ratio, 1.4f));
		TestEqual(TEXT("... held 3 frames (ratio 1.4)"), AJamSkater::HitStopFramesFor(P.Outcome, P.Ratio, K, 3), 3);
	}
	// 4. A knockdown between two bots only: nothing to hold.
	{
		TArray<FJamSkaterState> Sk = Skaters();
		Sk[2].ContactOutcome = UJamSim::ContactDown; Sk[2].ContactRatio = 2.f; Sk[2].ContactBy = 5;
		const TArray<FJamSimEvent> Ev = { Event(JamEvents::Check, 5, 0.9f, Front), Event(JamEvents::Knockdown, 2, 0.9f, Cause) };
		TestEqual(TEXT("a knockdown between two bots holds nothing"), AJamSkater::HitStopPick(Sk, Ev, Human, K.BumpRatio).Victim, -1);
	}
	// 5. A knockdown the human lands; and a knockdown the momentum model did not resolve (the frame's check event names the checker; the base holds).
	{
		TArray<FJamSkaterState> Sk = Skaters();
		Sk[3].ContactOutcome = UJamSim::ContactDown; Sk[3].ContactRatio = 2.34f; Sk[3].ContactBy = Human;
		const TArray<FJamSimEvent> Ev = { Event(JamEvents::Check, Human, 0.8f, Front), Event(JamEvents::Knockdown, 3, 0.8f, Cause) };
		const FJamHitStopPick P = AJamSkater::HitStopPick(Sk, Ev, Human, K.BumpRatio);
		TestTrue(TEXT("a knockdown the human lands"), P.Victim == 3 && P.Checker == Human && P.Outcome == UJamSim::ContactDown && P.Kind == Front && AJamSkater::HitStopFramesFor(P.Outcome, P.Ratio, K, 3) == 4);
		TArray<FJamSkaterState> Old = Skaters();
		const FJamHitStopPick Q = AJamSkater::HitStopPick(Old, Ev, Human, K.BumpRatio);
		TestTrue(TEXT("contact.momentum false: the check event names the checker, outcome none, the base holds"), Q.Victim == 3 && Q.Checker == Human && Q.Outcome == UJamSim::ContactNone && AJamSkater::HitStopFramesFor(Q.Outcome, Q.Ratio, K, 3) == 3);
	}
	// 6. The human's own bounce (its hit was a bump, it stumbles): 1 frame; a knockdown by another cause (the shotgun) holds nothing.
	{
		TArray<FJamSkaterState> Sk = Skaters();
		Sk[Human].bRecoilThisFrame = true;
		Sk[3].ContactOutcome = UJamSim::ContactBump; Sk[3].ContactBy = Human;
		const FJamHitStopPick P = AJamSkater::HitStopPick(Sk, TArray<FJamSimEvent>(), Human, K.BumpRatio);
		TestTrue(TEXT("the human's bounce: the human holds with the skater it bounced off, 1 frame"), P.Victim == Human && P.Checker == 3 && P.Kind == FName(TEXT("bounce")) && AJamSkater::HitStopFramesFor(P.Outcome, P.Ratio, K, 3) == 1);
		const TArray<FJamSimEvent> Shot = { Event(JamEvents::Knockdown, Human, 1.f, FName(TEXT("shotgun"))) };
		TestEqual(TEXT("a knockdown by the shotgun is not a check: nothing picked (as before)"), AJamSkater::HitStopPick(Skaters(), Shot, Human, K.BumpRatio).Victim, -1);
	}
	return true;
}

// The neutral proof and the brief's measurement. Baseline = this track's evolution 1 (board_bonus 1.25): pro seeds 1 to 3, three minutes.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamContactGraceMatch, "Jam.Core.ContactGraceMatch", JamContactTests::Flags)
bool FJamContactGraceMatch::RunTest(const FString& Parameters)
{
	const uint64 Baseline[3] = { 0xb59b988423ca43cfull, 0x2333d70ef430d6f3ull, 0x4f3c95b28136e412ull };
	for (uint64 Seed = 1; Seed <= 3; ++Seed)
	{
		FContactCount Cn;
		const JamAI::FJamPlaytestStats Sn = Match(*this, TEXT("pro"), Seed, Cn, [](FJamGameConfig& G) { G.Contact.ReleaseGraceFrames = 0; G.Stick.bBodyShield = false; G.Stick.bPassiveTake = true; G.Stick.bTeammateTouchTakes = true; });   // lane A102's stick rules off: the baseline is lane A101's tree
		AddInfo(Line(*FString::Printf(TEXT("JAMGRACE match pro seed %llu release_grace_frames 0"), static_cast<unsigned long long>(Seed)), Sn, Cn));
		TestEqual(*FString::Printf(TEXT("pro seed %llu: release_grace_frames 0 hashes as the baseline did"), static_cast<unsigned long long>(Seed)), FString::Printf(TEXT("%016llx"), static_cast<unsigned long long>(Sn.Hash)),
			FString::Printf(TEXT("%016llx"), static_cast<unsigned long long>(Baseline[Seed - 1])));
		TestEqual(*FString::Printf(TEXT("pro seed %llu: at release_grace_frames 0 no hit meets a grace"), static_cast<unsigned long long>(Seed)), Cn.Graced[0] + Cn.Graced[1], 0);
		FContactCount C;
		const JamAI::FJamPlaytestStats S = Match(*this, TEXT("pro"), Seed, C, [](FJamGameConfig&) {});
		AddInfo(Line(*FString::Printf(TEXT("JAMGRACE match pro seed %llu release_grace_frames 120 (the default)"), static_cast<unsigned long long>(Seed)), S, C));
		AddInfo(FString::Printf(TEXT("JAMGRACE grace pro seed %llu three minutes, grace 120 (the default): %s"), static_cast<unsigned long long>(Seed), *GraceLine(C)));
		TestEqual(*FString::Printf(TEXT("pro seed %llu: on the default no hit lands on a skater inside its grace, three minutes"), static_cast<unsigned long long>(Seed)), C.ReHitInGrace, 0);
		AddInfo(FString::Printf(TEXT("JAMGRACE grace pro seed %llu three minutes, grace 0: %s"), static_cast<unsigned long long>(Seed), *GraceLine(Cn)));
	}
	// The brief's measurement: 100 s at pro, five a side, the scripted human (the Regular playtester), ten seeds, grace 0 (before) / 90 (the first ruling) / 120
	// (the default, R-0051).
	const int32 Arms[3] = { 0, 90, 120 };
	for (int32 A = 0; A < 3; ++A)
	{
		const int32 Grace = Arms[A];
		int32 HumanDowns = 0, HumanMax = 0, BotDowns = 0, ReDown = 0, ReDownByHit = 0, ReStumble = 0, Graced = 0, GracedByBot = 0, InGrace = 0, MinGap = 1000000, Over4 = 0, Soft = 0, Missed = 0;
		FString PerSeed;
		for (uint64 Seed = 1; Seed <= 10; ++Seed)
		{
			FContactCount C;
			const JamAI::FJamPlaytestStats S = Match(*this, TEXT("pro"), Seed, C, [Grace](FJamGameConfig& G) { G.Contact.ReleaseGraceFrames = Grace; }, FString(), 6000);
			AddInfo(FString::Printf(TEXT("JAMGRACE 100s pro seed %llu grace %d: frames %lld hash %016llx goals %d:%d | %s"), static_cast<unsigned long long>(Seed), Grace, static_cast<long long>(S.Frames),
				static_cast<unsigned long long>(S.Hash), S.GoalsFor, S.GoalsAgainst, *GraceLine(C)));
			HumanDowns += C.Downs[1]; HumanMax = FMath::Max(HumanMax, C.Downs[1]); BotDowns += C.Downs[0];
			ReDown += C.ReDown[0] + C.ReDown[1]; ReDownByHit += C.ReDownByHit[0] + C.ReDownByHit[1]; ReStumble += C.ReStumble[0] + C.ReStumble[1];
			Graced += C.Graced[0] + C.Graced[1]; GracedByBot += C.GracedByBot; InGrace += C.ReHitInGrace; MinGap = FMath::Min(MinGap, C.MinGap);
			if (C.Downs[1] >= 5) ++Over4;
			Soft += C.HumanSoft; Missed += C.HumanMissed;
			PerSeed += FString::Printf(TEXT("%s%d"), Seed > 1 ? TEXT(" ") : TEXT(""), C.Downs[1]);
		}
		AddInfo(FString::Printf(TEXT("JAMGRACE 100s pro TOTAL grace %d, ten seeds: knockdowns of the human %d (per seed: %s; most %d; matches with five or more %d) of a bot %d | figures down again within 120 frames of release %d (by a hit %d) | ")
			TEXT("stumbled by a hit in that window %d | shortest release-to-down gap %d frames | hits the grace made a bump %d (thrown by a bot %d) | landed hits inside a grace %d | human's soft hits %d misses %d"),
			Grace, HumanDowns, *PerSeed, HumanMax, Over4, BotDowns, ReDown, ReDownByHit, ReStumble, MinGap == 1000000 ? -1 : MinGap, Graced, GracedByBot, InGrace, Soft, Missed));
		if (Grace > 0) TestEqual(*FString::Printf(TEXT("grace %d: no hit lands (stumble or knockdown) on a skater inside its grace, ten matches"), Grace), InGrace, 0);
		if (Grace == 0) TestEqual(TEXT("grace 0: no hit meets a grace"), Graced, 0);
	}
	return true;
}

// The hold reads the hit (AJamSkater::HitStopFramesFor; the game mode's HitStopRoute calls it with jam.hitstop as the base).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamContactHitStop, "Jam.Core.Contact.HitStop", JamContactTests::Flags)
bool FJamContactHitStop::RunTest(const FString& Parameters)
{
	const FJamContactConfig K;   // the defaults: bump 0.4, down 1, full 3
	struct FRow { int32 Outcome; float Ratio; int32 Base; int32 Want; const TCHAR* Name; };
	const FRow Rows[] = {
		{ UJamSim::ContactBump, 0.30f, 3, 0, TEXT("a bump holds nothing") },
		{ UJamSim::ContactStumble, 0.40f, 3, 1, TEXT("the lightest stumble: 1") },
		{ UJamSim::ContactStumble, 0.69f, 3, 1, TEXT("a stumble under the middle of its band: 1") },
		{ UJamSim::ContactStumble, 0.70f, 3, 2, TEXT("a stumble from the middle of its band: 2") },
		{ UJamSim::ContactStumble, 0.99f, 3, 2, TEXT("the hardest stumble: 2") },
		{ UJamSim::ContactDown, 1.00f, 3, 2, TEXT("a knockdown at down_ratio: 2") },
		{ UJamSim::ContactDown, 1.40f, 3, 3, TEXT("a knockdown at 1.4 (a full-speed charge into a braced skater): 3") },
		{ UJamSim::ContactDown, 2.34f, 3, 4, TEXT("a knockdown at 2.34 (the acceptance drill's check): 4") },
		{ UJamSim::ContactDown, 3.00f, 3, 5, TEXT("a knockdown at down_full_ratio: 5") },
		{ UJamSim::ContactDown, 4.61f, 3, 5, TEXT("head-on at full speed: 5") },
		{ UJamSim::ContactNone, 0.00f, 3, 3, TEXT("a knockdown the model did not resolve (contact.momentum false): the base, as before") },
		{ UJamSim::ContactDown, 3.00f, 0, 0, TEXT("jam.hitstop 0: off") },
		{ UJamSim::ContactStumble, 0.90f, 0, 0, TEXT("jam.hitstop 0: off for a stumble too") },
		{ UJamSim::ContactDown, 3.00f, 6, 6, TEXT("jam.hitstop 6: the biggest hit holds 6 (the cap)") },
		{ UJamSim::ContactStumble, 0.45f, 6, 2, TEXT("jam.hitstop 6: the lightest stumble 2") },
		{ UJamSim::ContactStumble, 0.45f, 1, 1, TEXT("jam.hitstop 1: a landed hit never under 1") },
	};
	for (const FRow& R : Rows) TestEqual(R.Name, AJamSkater::HitStopFramesFor(R.Outcome, R.Ratio, K, R.Base), R.Want);
	// Never shorter for a harder hit, across the two bands.
	int32 Last = 0;
	for (int32 I = 40; I <= 500; ++I)
	{
		const float Ratio = I / 100.f;
		const int32 Frames = AJamSkater::HitStopFramesFor(Ratio >= K.DownRatio ? UJamSim::ContactDown : UJamSim::ContactStumble, Ratio, K, 3);
		if (Frames < Last) { AddError(FString::Printf(TEXT("the hold shrinks at ratio %.2f: %d after %d"), Ratio, Frames, Last)); break; }
		Last = Frames;
	}
	TestEqual(TEXT("the sweep ends on 5 frames"), Last, 5);
	return true;
}

// Three pro matches (the brief's measurement), the bonus against its neutral value; the neutral arm must hash as main at ee14438 did (the Director's gate log
// premerge-automation-ue-20261113-184648.log: Jam.Core.ContactMatch, pro, seeds 1 to 3).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamContactBoardMatch, "Jam.Core.ContactBoardMatch", JamContactTests::Flags)
bool FJamContactBoardMatch::RunTest(const FString& Parameters)
{
	const uint64 Main[3] = { 0x6ed8f16ab724086bull, 0xdc1a21fe22e0bc6full, 0x7d34793757849ca9ull };
	for (uint64 Seed = 1; Seed <= 3; ++Seed)
	{
		FContactCount C;
		const JamAI::FJamPlaytestStats S = Match(*this, TEXT("pro"), Seed, C, [](FJamGameConfig&) {});
		AddInfo(Line(*FString::Printf(TEXT("JAMBOARDS match pro seed %llu board_bonus 1.25"), static_cast<unsigned long long>(Seed)), S, C));
		AddInfo(FString::Printf(TEXT("JAMBOARDS boards pro seed %llu board_bonus 1.25: %s"), static_cast<unsigned long long>(Seed), *BoardLine(C)));
		FContactCount Cn;
		const JamAI::FJamPlaytestStats Sn = Match(*this, TEXT("pro"), Seed, Cn, [](FJamGameConfig& G) { G.Contact.BoardBonus = 1.0; G.Contact.ReleaseGraceFrames = 0; G.Stick.bBodyShield = false; G.Stick.bPassiveTake = true; G.Stick.bTeammateTouchTakes = true; });   // lanes A113 and A102: every later rule neutral = the game main had at ee14438
		AddInfo(Line(*FString::Printf(TEXT("JAMBOARDS match pro seed %llu board_bonus 1.00"), static_cast<unsigned long long>(Seed)), Sn, Cn));
		AddInfo(FString::Printf(TEXT("JAMBOARDS boards pro seed %llu board_bonus 1.00: %s"), static_cast<unsigned long long>(Seed), *BoardLine(Cn)));
		TestEqual(*FString::Printf(TEXT("pro seed %llu: board_bonus 1 hashes as main did"), static_cast<unsigned long long>(Seed)), FString::Printf(TEXT("%016llx"), static_cast<unsigned long long>(Sn.Hash)),
			FString::Printf(TEXT("%016llx"), static_cast<unsigned long long>(Main[Seed - 1])));
		TestEqual(*FString::Printf(TEXT("pro seed %llu: at board_bonus 1 the boards answer no hit"), static_cast<unsigned long long>(Seed)), Cn.BoardStumble[0] + Cn.BoardStumble[1] + Cn.BoardDown[0] + Cn.BoardDown[1], 0);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamContactSameDirection, "Jam.Core.Contact.SameDirection", JamContactTests::Flags)
bool FJamContactSameDirection::RunTest(const FString& Parameters)
{
	// Both glide along +x at 8 m/s; the attacker touches the victim from behind, then from the side, and presses check.
	struct FCase { const TCHAR* Name; FVector2D Offset; };
	for (const FCase& C : { FCase{ TEXT("from behind"), FVector2D(-0.78, 0.0) }, FCase{ TEXT("from the side"), FVector2D(0.0, -0.78) } })
	{
		FContactScene S = Make(*this, NoEdit);
		if (!S.Ok()) return false;
		S.Sim->SetSkater(S.V, FVector2D::ZeroVector, FVector2D(8.0, 0.0), 0.0);
		S.Sim->SetSkater(S.A, C.Offset, FVector2D(8.0, 0.0), 0.0);
		const FContactHit H = Run(S, 12, [&](TArray<FJamInput>& In, int32 F) { In[S.A].bCheck = F == 0; In[S.A].Move = FVector2D(0.7, 0.0); In[S.V].Move = FVector2D(0.7, 0.0); });
		AddInfo(FString::Printf(TEXT("same direction, same speed, %s: %s"), C.Name, *Facts(H)));
		TestEqual(*FString::Printf(TEXT("%s: the pressed check is a bump"), C.Name), H.Outcome, UJamSim::ContactBump);
		TestFalse(*FString::Printf(TEXT("%s: no check event, no knockdown"), C.Name), H.bCheck || H.bKnock);
		TestTrue(*FString::Printf(TEXT("%s: the victim keeps its feet and its control"), C.Name), S.Vic().CanAct());
		TestTrue(*FString::Printf(TEXT("%s: the victim keeps its speed (%.2f of %.2f m/s)"), C.Name, H.VicSpeedAfter, H.VicSpeedBefore), H.VicSpeedAfter > H.VicSpeedBefore - 0.5);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamContactHeadOn, "Jam.Core.Contact.HeadOn", JamContactTests::Flags)
bool FJamContactHeadOn::RunTest(const FString& Parameters)
{
	FContactScene S = Make(*this, NoEdit);
	if (!S.Ok()) return false;
	const double Top = S.Cfg.Skater.Speed;
	S.Sim->SetSkater(S.V, FVector2D(0.6, 0.0), FVector2D(-Top, 0.0), PI);
	S.Sim->SetSkater(S.A, FVector2D(-0.6, 0.0), FVector2D(Top, 0.0), 0.0);
	const FContactHit H = Run(S, 12, [&](TArray<FJamInput>& In, int32 F) { In[S.A].bCheck = F == 0; });
	AddInfo(FString::Printf(TEXT("head-on at %.1f m/s each: %s"), Top, *Facts(H)));
	TestEqual(TEXT("head-on at full speed: down"), H.Outcome, UJamSim::ContactDown);
	TestTrue(TEXT("the check and the knockdown events"), H.bCheck && H.bKnock);
	TestEqual(TEXT("down for down_frames_max (skater.knockdown_frames 90)"), H.KnockFrames, S.Cfg.Contact.DownFramesMax);
	TestTrue(*FString::Printf(TEXT("the ratio %.2f is past down_full_ratio %.1f: the biggest hit in the game"), H.Ratio, S.Cfg.Contact.DownFullRatio), H.Ratio >= S.Cfg.Contact.DownFullRatio);
	TestTrue(TEXT("strength 1 (closing speed over skater.speed, clamped)"), H.Strength > 0.99f);
	// The same press with the same closing speed halved lands a shorter knockdown: the frames follow the ratio.
	FContactScene S2 = Make(*this, NoEdit);
	S2.Sim->SetSkater(S2.V, FVector2D(0.6, 0.0), FVector2D(-3.0, 0.0), PI);
	S2.Sim->SetSkater(S2.A, FVector2D(-0.6, 0.0), FVector2D(4.0, 0.0), 0.0);
	const FContactHit H2 = Run(S2, 12, [&](TArray<FJamInput>& In, int32 F) { In[S2.A].bCheck = F == 0; });
	AddInfo(FString::Printf(TEXT("head-on at 4 and 3 m/s: %s"), *Facts(H2)));
	TestEqual(TEXT("head-on at 4 + 3 m/s: down"), H2.Outcome, UJamSim::ContactDown);
	TestTrue(*FString::Printf(TEXT("... for fewer frames (%d) than the full-speed hit (%d), at least down_frames_min"), H2.KnockFrames, H.KnockFrames),
		H2.KnockFrames < H.KnockFrames && H2.KnockFrames >= S2.Cfg.Contact.DownFramesMin);
	return true;
}

// The two older levers under the model. skater.knockdown_frames scales the frames down (value / 90): the head-on at full speed lies 250 at 150. skater.check_speed_min
// scales the hit (5 / value) for the outcome only: at 2.5 the side check from 4 m/s that stumbled a standing skater (0.61) puts it down (1.22), for down_frames_min
// (the physical ratio is under down_ratio), not for the maximum.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamContactLevers, "Jam.Core.Contact.Levers", JamContactTests::Flags)
bool FJamContactLevers::RunTest(const FString& Parameters)
{
	{
		FContactScene S = Make(*this, [](FJamGameConfig& G) { G.Skater.KnockdownFrames = 150; });
		if (!S.Ok()) return false;
		const double Top = S.Cfg.Skater.Speed;
		S.Sim->SetSkater(S.V, FVector2D(0.6, 0.0), FVector2D(-Top, 0.0), PI);
		S.Sim->SetSkater(S.A, FVector2D(-0.6, 0.0), FVector2D(Top, 0.0), 0.0);
		const FContactHit H = Run(S, 12, [&](TArray<FJamInput>& In, int32 F) { In[S.A].bCheck = F == 0; });
		AddInfo(FString::Printf(TEXT("knockdown_frames 150, head-on at full speed: %s"), *Facts(H)));
		TestEqual(TEXT("knockdown_frames 150: the biggest hit lies down_frames_max x 150 / 90 = 250"), H.KnockFrames, 250);
	}
	for (const double CheckMin : { 5.0, 2.5 })
	{
		FContactScene S = Make(*this, [&](FJamGameConfig& G) { G.Skater.CheckSpeedMin = CheckMin; });
		if (!S.Ok()) return false;
		S.Sim->SetSkater(S.V, FVector2D::ZeroVector, FVector2D::ZeroVector, 0.0);
		S.Sim->SetSkater(S.A, FVector2D(0.0, -1.0), FVector2D(0.0, 4.0), PI * 0.5);
		const FContactHit H = Run(S, 12, [&](TArray<FJamInput>& In, int32 F) { In[S.A].bCheck = F == 0; });
		AddInfo(FString::Printf(TEXT("check_speed_min %.1f, side check from 4 m/s on a standing skater: %s"), CheckMin, *Facts(H)));
		if (CheckMin > 4.0) TestEqual(TEXT("check_speed_min 5: a stumble"), H.Outcome, UJamSim::ContactStumble);
		else
		{
			TestEqual(TEXT("check_speed_min 2.5: the same hit weighs double and puts it down"), H.Outcome, UJamSim::ContactDown);
			TestEqual(TEXT("... for down_frames_min: the frames read the physical ratio"), H.KnockFrames, S.Cfg.Contact.DownFramesMin);
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamContactStandingBrace, "Jam.Core.Contact.StandingBrace", JamContactTests::Flags)
bool FJamContactStandingBrace::RunTest(const FString& Parameters)
{
	// 1. A side body check from 4 m/s on a standing skater (facing +x; the attacker comes along +z from its left): a stumble at most.
	{
		FContactScene S = Make(*this, NoEdit);
		if (!S.Ok()) return false;
		S.Sim->SetSkater(S.V, FVector2D::ZeroVector, FVector2D::ZeroVector, 0.0);
		S.Sim->SetSkater(S.A, FVector2D(0.0, -1.0), FVector2D(0.0, 4.0), PI * 0.5);
		const FContactHit H = Run(S, 12, [&](TArray<FJamInput>& In, int32 F) { In[S.A].bCheck = F == 0; });
		AddInfo(FString::Printf(TEXT("side body check from 4 m/s on a standing skater: %s"), *Facts(H)));
		TestTrue(TEXT("side body check from 4 m/s: the standing skater is hit"), H.Outcome != 0);
		TestTrue(TEXT("... and stumbles at most (the brace holds)"), H.Outcome <= UJamSim::ContactStumble && !H.bKnock);
		TestTrue(*FString::Printf(TEXT("... the ratio %.2f is under down_ratio"), H.Ratio), H.Ratio < S.Cfg.Contact.DownRatio);
	}
	// 2. A cross-check from the front at the same 4 m/s (the standing skater carries the puck: the cross-check is the lift at the carrier): down.
	// 3. The same cross-check from the side: the brace holds, a stumble.
	struct FCase { const TCHAR* Name; FVector2D From; bool bDown; };
	for (const FCase& C : { FCase{ TEXT("from the front"), FVector2D(1.0, 0.0), true }, FCase{ TEXT("from the side"), FVector2D(0.0, -1.0), false } })
	{
		FContactScene S = Make(*this, NoEdit);
		if (!S.Ok()) return false;
		S.Sim->SetSkater(S.V, FVector2D::ZeroVector, FVector2D::ZeroVector, 0.0);
		S.Sim->SetSkater(S.A, FVector2D(-10.0, -10.0), FVector2D::ZeroVector, 0.0);
		TestTrue(*FString::Printf(TEXT("%s: the standing skater carries the puck"), C.Name), HandPuck(S, S.V));
		Carry(S, S.V, FVector2D::ZeroVector, FVector2D::ZeroVector, 0.0);
		const FVector2D Dir = -C.From;
		S.Sim->SetSkater(S.A, C.From * 1.2, Dir * 4.0, FMath::Atan2(Dir.Y, Dir.X));   // a gap of 0.4 m: inside cross_reach_m 0.5
		const FContactHit H = Run(S, 6, [&](TArray<FJamInput>& In, int32 F) { In[S.A].bLift = F == 0; });
		AddInfo(FString::Printf(TEXT("cross-check from 4 m/s %s on a standing carrier: %s lift_clean %d"), C.Name, *Facts(H), H.bLiftClean ? 1 : 0));
		if (C.bDown)
		{
			TestEqual(TEXT("front cross-check from 4 m/s: the standing skater goes down (the thrust ignores the brace)"), H.Outcome, UJamSim::ContactDown);
			TestTrue(TEXT("... with the check and the knockdown events"), H.bCheck && H.bKnock);
		}
		else
		{
			TestEqual(TEXT("side cross-check from 4 m/s: the brace holds, a stumble"), H.Outcome, UJamSim::ContactStumble);
		}
		TestTrue(*FString::Printf(TEXT("%s: the carrier no longer has the puck"), C.Name), S.Sim->CarrierIndex() != S.V);
	}
	// 4. The same front cross-check pressed from the blade's length (a gap of 0.8 m, beyond cross_reach_m): the stick lift only, nobody is shoved.
	// 5. From behind at arm's length: the stick lift only (no shove, no foul).
	for (int32 Case = 0; Case < 2; ++Case)
	{
		FContactScene S = Make(*this, [](FJamGameConfig& G) { G.Rules.bPenalties = true; G.Stick.bBodyShield = false; });
		// Lane A102: stick.body_shield false here: this case is about the shove (none from behind); with the body shield a lift from behind does not take a puck held ahead (Jam.Core.Movement.BodyShield).
		if (!S.Ok()) return false;
		S.Sim->SetSkater(S.V, FVector2D::ZeroVector, FVector2D::ZeroVector, 0.0);
		S.Sim->SetSkater(S.A, FVector2D(-10.0, -10.0), FVector2D::ZeroVector, 0.0);
		TestTrue(TEXT("the standing skater carries the puck"), HandPuck(S, S.V));
		Carry(S, S.V, FVector2D::ZeroVector, FVector2D::ZeroVector, 0.0);
		const FVector2D From = Case == 0 ? FVector2D(1.6, 0.0) : FVector2D(-1.2, 0.0);
		S.Sim->SetSkater(S.A, From, -From.GetSafeNormal() * 4.0, FMath::Atan2(-From.Y, -From.X));
		const FContactHit H = Run(S, 1, [&](TArray<FJamInput>& In, int32 F) { In[S.A].bLift = F == 0; });
		bool bLift = false;
		for (const FJamSimEvent& E : S.Sim->GetFrameEvents()) if (E.Name == JamEvents::Lift && E.Who == S.A && E.Strength > 0.5f) bLift = true;
		TestTrue(Case == 0 ? TEXT("a lift from the blade's length: the puck is lifted") : TEXT("a lift from behind: the puck is lifted"), bLift);
		TestTrue(Case == 0 ? TEXT("... and nobody is shoved (beyond cross_reach_m)") : TEXT("... and nobody is shoved, no foul (a lift from behind stays a stick lift)"),
			S.Vic().ContactOutcome == 0 && !H.bCheck && !H.bKnock && H.Foul.IsNone() && S.Vic().CanAct());
	}
	// StabilityOf by the numbers (mass 80, stability_mps 6: 480 kg m/s neutral).
	{
		FContactScene S = Make(*this, NoEdit);
		const FJamContactConfig& K = S.Cfg.Contact;
		const double Base = S.Cfg.Skater.Mass * K.StabilityMps;
		S.Sim->SetSkater(S.V, FVector2D::ZeroVector, FVector2D::ZeroVector, 0.0);
		const FJamSkaterState& V = S.Vic();
		TestEqual(TEXT("standing, body hit from the side: x stand_brace"), S.Sim->StabilityOf(V, FVector2D(0, 1), UJamSim::EContactKind::Body), Base * K.StandBrace, 1e-6);
		TestEqual(TEXT("standing, body hit from the front: x stand_brace"), S.Sim->StabilityOf(V, FVector2D(-1, 0), UJamSim::EContactKind::Body), Base * K.StandBrace, 1e-6);
		TestEqual(TEXT("standing, cross-check from the front: the brace ignored"), S.Sim->StabilityOf(V, FVector2D(-1, 0), UJamSim::EContactKind::Cross), Base, 1e-6);
		TestEqual(TEXT("standing, cross-check from the side: braced"), S.Sim->StabilityOf(V, FVector2D(0, 1), UJamSim::EContactKind::Cross), Base * K.StandBrace, 1e-6);
		TestEqual(TEXT("standing, hit from behind: unbraced x unseen_stability"), S.Sim->StabilityOf(V, FVector2D(1, 0), UJamSim::EContactKind::Body), Base * K.UnseenStability, 1e-6);
		S.Sim->SetSkater(S.V, FVector2D::ZeroVector, FVector2D(S.Cfg.Skater.Speed, 0.0), 0.0);
		TestEqual(TEXT("at top speed, from the side: x speed_stability"), S.Sim->StabilityOf(S.Vic(), FVector2D(0, 1), UJamSim::EContactKind::Body), Base * K.SpeedStability, 1e-6);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamContactSprintFalls, "Jam.Core.Contact.SprintFalls", JamContactTests::Flags)
bool FJamContactSprintFalls::RunTest(const FString& Parameters)
{
	// The victim skates along +x at 5 m/s; the attacker hits it from its left at 3.5 m/s. Walking it stumbles, sprinting (Shift held, the same speed) it falls.
	FContactHit Hits[2];
	for (int32 Sprint = 0; Sprint < 2; ++Sprint)
	{
		FContactScene S = Make(*this, NoEdit);
		if (!S.Ok()) return false;
		// Two frames of the victim's own skating first, so the sprint flag is up when the hit comes; then both are placed for the contact.
		S.Sim->SetSkater(S.V, FVector2D(-0.2, 0.0), FVector2D(5.0, 0.0), 0.0);
		S.Sim->SetSkater(S.A, FVector2D(-12.0, -12.0), FVector2D::ZeroVector, 0.0);
		auto Skate = [&](TArray<FJamInput>& In) { In[S.V].Move = FVector2D(1.0, 0.0); In[S.V].bSprint = Sprint == 1; };
		Run(S, 2, [&](TArray<FJamInput>& In, int32) { Skate(In); });
		TestTrue(*FString::Printf(TEXT("%s: the sprint flag"), Sprint ? TEXT("sprinting") : TEXT("walking")), S.Vic().bSprinting == (Sprint == 1));
		S.Sim->SetSkater(S.V, FVector2D::ZeroVector, FVector2D(5.0, 0.0), 0.0);
		S.Sim->SetSkater(S.A, FVector2D(0.05, -0.9), FVector2D(5.0, 3.5), PI * 0.5);
		Hits[Sprint] = Run(S, 12, [&](TArray<FJamInput>& In, int32 F) { Skate(In); In[S.A].bCheck = F == 0; });
		AddInfo(FString::Printf(TEXT("side hit closing 3.5 m/s on a %s victim at 5 m/s: %s"), Sprint ? TEXT("sprinting") : TEXT("walking"), *Facts(Hits[Sprint])));
	}
	TestEqual(TEXT("the walking victim survives: a stumble"), Hits[0].Outcome, UJamSim::ContactStumble);
	TestEqual(TEXT("the sprinting victim falls to the same hit"), Hits[1].Outcome, UJamSim::ContactDown);
	TestTrue(*FString::Printf(TEXT("the sprint's ratio %.2f is the walk's %.2f over sprint_stability"), Hits[1].Ratio, Hits[0].Ratio), Hits[1].Ratio > Hits[0].Ratio * 1.2f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamContactAttackerBounces, "Jam.Core.Contact.AttackerBounces", JamContactTests::Flags)
bool FJamContactAttackerBounces::RunTest(const FString& Parameters)
{
	// A standing braced skater (facing +x). The attacker runs into its side at 5 m/s: sprinting it is the one who stumbles; walking in at 3 m/s it keeps its feet.
	FContactHit Hits[2];
	for (int32 Sprint = 0; Sprint < 2; ++Sprint)
	{
		FContactScene S = Make(*this, NoEdit);
		if (!S.Ok()) return false;
		const double V0 = Sprint ? 5.0 : 3.0;
		S.Sim->SetSkater(S.V, FVector2D::ZeroVector, FVector2D::ZeroVector, 0.0);
		S.Sim->SetSkater(S.A, FVector2D(0.0, -1.2), FVector2D(0.0, V0), PI * 0.5);
		Hits[Sprint] = Run(S, 14, [&](TArray<FJamInput>& In, int32 F) { In[S.A].Move = FVector2D(0.0, Sprint ? 1.0 : 0.3); In[S.A].bSprint = Sprint == 1; In[S.A].bCheck = F == 1; });
		AddInfo(FString::Printf(TEXT("%s into a standing braced skater's side: %s attacker stumble frames %d"), Sprint ? TEXT("sprinting at 5 m/s") : TEXT("walking at 3 m/s"), *Facts(Hits[Sprint]), Hits[Sprint].AttStumble));
		if (Sprint)
		{
			TestTrue(TEXT("the braced skater stays up"), Hits[1].Outcome != 0 && Hits[1].Outcome < UJamSim::ContactDown);
			TestTrue(TEXT("the sprinting attacker is the one who stumbles (recoil)"), Hits[1].bRecoil && Hits[1].AttStumble == S.Cfg.Contact.StaggerFrames && !S.Att().CanAct());
			TestTrue(*FString::Printf(TEXT("... and loses the speed it gave (%.2f -> %.2f m/s)"), Hits[1].AttSpeedBefore, Hits[1].AttSpeedAfter), Hits[1].AttSpeedAfter < Hits[1].AttSpeedBefore * 0.5);
		}
		else
		{
			TestTrue(TEXT("walking in at 3 m/s: no recoil, the attacker keeps its feet"), Hits[0].Outcome != 0 && !Hits[0].bRecoil && S.Att().CanAct());
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamContactCarrierRefused, "Jam.Core.Contact.CarrierRefused", JamContactTests::Flags)
bool FJamContactCarrierRefused::RunTest(const FString& Parameters)
{
	for (int32 CanHit = 0; CanHit < 2; ++CanHit)
	{
		FContactScene S = Make(*this, [&](FJamGameConfig& G) { G.Contact.bCarrierCanHit = CanHit == 1; });
		if (!S.Ok()) return false;
		S.Sim->SetSkater(S.A, FVector2D(-6.0, 0.0), FVector2D::ZeroVector, 0.0);
		S.Sim->SetSkater(S.V, FVector2D(10.0, 10.0), FVector2D::ZeroVector, PI);
		TestTrue(TEXT("the attacker carries the puck"), HandPuck(S, S.A));
		// Head-on at 8 m/s each with the puck on the attacker's stick: the hit that is the biggest in the game without the puck.
		Carry(S, S.A, FVector2D(-0.6, 0.0), FVector2D(8.0, 0.0), 0.0);
		S.Sim->SetSkater(S.V, FVector2D(0.6, 0.0), FVector2D(-8.0, 0.0), PI);
		const FContactHit H = Run(S, 14, [&](TArray<FJamInput>& In, int32 F) { In[S.A].bCheck = F == 0; });
		AddInfo(FString::Printf(TEXT("carrier_can_hit %d: refused %d window %d %s"), CanHit, H.bRefused ? 1 : 0, H.bWindow ? 1 : 0, *Facts(H)));
		if (!CanHit)
		{
			TestTrue(TEXT("the carrier's press is refused (bCheckRefused)"), H.bRefused);
			TestFalse(TEXT("... no check window opens, no check animation"), H.bWindow);
			TestTrue(TEXT("... the victim takes no hit: no check, no knockdown"), H.Outcome == 0 && !H.bCheck && !H.bKnock);
			TestEqual(TEXT("... counted once"), S.Att().ChecksRefused, 1);
		}
		else
		{
			TestFalse(TEXT("carrier_can_hit true: the press is not refused"), H.bRefused);
			TestTrue(TEXT("... and the carrier's check lands"), H.Outcome == UJamSim::ContactDown && H.bCheck && H.bKnock);
		}
	}
	// A take in the same breath finishes its hit: the victim carried the puck a moment ago (inside stick.foul_grace_frames), the attacker's body takes it at the
	// contact, and its check, pressed before the contact, still lands (the acceptance drill's step 7 found the refusal: A92's status, 16:17).
	{
		FContactScene S = Make(*this, NoEdit);
		S.Sim->SetSkater(S.V, FVector2D::ZeroVector, FVector2D::ZeroVector, PI);
		S.Sim->SetSkater(S.A, FVector2D(-10.0, -10.0), FVector2D::ZeroVector, 0.0);
		TestTrue(TEXT("the victim carries the puck"), HandPuck(S, S.V));
		Carry(S, S.V, FVector2D(0.7, 0.0), FVector2D(-2.5, 0.0), PI);   // the puck 0.45 m ahead of the victim: between the two bodies
		S.Sim->SetSkater(S.A, FVector2D(-0.7, 0.0), FVector2D(6.0, 0.0), 0.0);
		const FContactHit H = Run(S, 14, [&](TArray<FJamInput>& In, int32 F) { In[S.A].bCheck = F == 0; });
		AddInfo(FString::Printf(TEXT("head-on at a carrier with the puck between: refused %d %s"), H.bRefused ? 1 : 0, *Facts(H)));
		TestTrue(TEXT("a check at a carrier with the puck between the bodies lands (not refused)"), !H.bRefused && H.bCheck && H.Outcome >= UJamSim::ContactStumble);
	}
	// The same for the cross-check: the victim carried a moment ago and the puck is gone from it (parked here); the lift at arm's length from its front finds no
	// carrier, and the thrust still lands on the skater that just lost it. Once the grace is over the same press hits nobody.
	for (const int32 Wait : { 2, 30 })
	{
		FContactScene S = Make(*this, NoEdit);
		S.Sim->SetSkater(S.V, FVector2D::ZeroVector, FVector2D::ZeroVector, PI);
		S.Sim->SetSkater(S.A, FVector2D(-10.0, -10.0), FVector2D::ZeroVector, 0.0);
		TestTrue(TEXT("the victim carries the puck"), HandPuck(S, S.V));
		S.Sim->SetPuck(FVector2D(-20.0, 12.0), FVector2D::ZeroVector);
		Run(S, Wait, [](TArray<FJamInput>&, int32) {});
		S.Sim->SetSkater(S.V, FVector2D::ZeroVector, FVector2D::ZeroVector, PI);
		S.Sim->SetSkater(S.A, FVector2D(-1.2, 0.0), FVector2D(4.0, 0.0), 0.0);
		const FContactHit H = Run(S, 2, [&](TArray<FJamInput>& In, int32 F) { In[S.A].bLift = F == 0; });
		AddInfo(FString::Printf(TEXT("cross-check %d frames after the victim lost the puck: %s"), Wait, *Facts(H)));
		if (Wait <= 10) TestTrue(TEXT("a cross-check on the skater that just lost the puck still lands (inside the grace)"), H.bCheck && H.Outcome == UJamSim::ContactDown);
		else TestTrue(TEXT("... and hits nobody once the grace is over"), H.Outcome == 0 && !H.bCheck);
	}
	// The carrier's cross-check: a carrier's lift finds no opposing carrier, so it is a miss as before.
	{
		FContactScene S = Make(*this, NoEdit);
		S.Sim->SetSkater(S.A, FVector2D(-6.0, 0.0), FVector2D::ZeroVector, 0.0);
		S.Sim->SetSkater(S.V, FVector2D(10.0, 10.0), FVector2D::ZeroVector, PI);
		TestTrue(TEXT("the attacker carries the puck"), HandPuck(S, S.A));
		// the cross-check: a carrier's lift finds no opposing carrier, so it is a miss as before
		Carry(S, S.A, FVector2D(-0.7, 0.0), FVector2D(3.0, 0.0), 0.0);
		S.Sim->SetSkater(S.V, FVector2D(0.7, 0.0), FVector2D::ZeroVector, PI);
		const FContactHit H = Run(S, 8, [&](TArray<FJamInput>& In, int32 F) { In[S.A].bLift = F == 0; });
		TestTrue(TEXT("the carrier's cross-check hits nobody"), H.Outcome == 0 && !H.bCheck && !H.bKnock && !H.bLiftClean);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamContactFouls, "Jam.Core.Contact.Fouls", JamContactTests::Flags)
bool FJamContactFouls::RunTest(const FString& Parameters)
{
	auto Penalties = [](FJamGameConfig& G) { G.Rules.bPenalties = true; };
	// 1. From behind on the carrier, closing 6 m/s: the hit lands and the foul is called.
	{
		FContactScene S = Make(*this, Penalties);
		if (!S.Ok()) return false;
		S.Sim->SetSkater(S.V, FVector2D::ZeroVector, FVector2D::ZeroVector, 0.0);
		S.Sim->SetSkater(S.A, FVector2D(-10.0, -10.0), FVector2D::ZeroVector, 0.0);
		TestTrue(TEXT("the victim carries the puck"), HandPuck(S, S.V));
		Carry(S, S.V, FVector2D::ZeroVector, FVector2D(3.0, 0.0), 0.0);
		S.Sim->SetSkater(S.A, FVector2D(-0.9, 0.0), FVector2D(9.0, 0.0), 0.0);
		const FContactHit H = Run(S, 12, [&](TArray<FJamInput>& In, int32 F) { In[S.A].bCheck = F == 0; });
		AddInfo(FString::Printf(TEXT("from behind on the carrier: %s foul %s"), *Facts(H), *H.Foul.ToString()));
		TestTrue(TEXT("from behind on the carrier: the hit lands"), H.bCheck && H.Outcome >= UJamSim::ContactStumble);
		TestEqual(TEXT("... and the foul is foul_from_behind"), H.Foul.ToString(), FString(TEXT("foul_from_behind")));
		TestTrue(TEXT("... the fouler sits in the box"), S.Att().InBox());
	}
	// 2. From the front on a skater without the puck: foul_no_puck.
	{
		FContactScene S = Make(*this, Penalties);
		S.Sim->SetSkater(S.V, FVector2D(0.6, 0.0), FVector2D(-4.0, 0.0), PI);
		S.Sim->SetSkater(S.A, FVector2D(-0.6, 0.0), FVector2D(6.0, 0.0), 0.0);
		const FContactHit H = Run(S, 12, [&](TArray<FJamInput>& In, int32 F) { In[S.A].bCheck = F == 0; });
		AddInfo(FString::Printf(TEXT("from the front on a skater without the puck: %s foul %s"), *Facts(H), *H.Foul.ToString()));
		TestEqual(TEXT("a hit on a skater without the puck: foul_no_puck"), H.Foul.ToString(), FString(TEXT("foul_no_puck")));
	}
	// 3. A clean hit from the front on the carrier: no foul. And a bump from behind (same speed) is no foul either: nothing landed.
	{
		FContactScene S = Make(*this, Penalties);
		S.Sim->SetSkater(S.V, FVector2D::ZeroVector, FVector2D::ZeroVector, PI);
		S.Sim->SetSkater(S.A, FVector2D(-10.0, -10.0), FVector2D::ZeroVector, 0.0);
		TestTrue(TEXT("the victim carries the puck"), HandPuck(S, S.V));
		Carry(S, S.V, FVector2D(0.6, 0.0), FVector2D(-4.0, 0.0), PI);
		S.Sim->SetSkater(S.A, FVector2D(-0.6, 0.0), FVector2D(6.0, 0.0), 0.0);
		const FContactHit H = Run(S, 12, [&](TArray<FJamInput>& In, int32 F) { In[S.A].bCheck = F == 0; });
		TestTrue(TEXT("from the front on the carrier: the hit lands, no foul"), H.bCheck && H.Foul.IsNone());
		TestTrue(TEXT("... and the puck is loose"), S.Sim->CarrierIndex() != S.V);
	}
	{
		FContactScene S = Make(*this, Penalties);
		S.Sim->SetSkater(S.V, FVector2D::ZeroVector, FVector2D::ZeroVector, 0.0);
		S.Sim->SetSkater(S.A, FVector2D(-10.0, -10.0), FVector2D::ZeroVector, 0.0);
		TestTrue(TEXT("the victim carries the puck"), HandPuck(S, S.V));
		Carry(S, S.V, FVector2D::ZeroVector, FVector2D(8.0, 0.0), 0.0);
		S.Sim->SetSkater(S.A, FVector2D(-0.78, 0.0), FVector2D(8.0, 0.0), 0.0);
		const FContactHit H = Run(S, 12, [&](TArray<FJamInput>& In, int32 F) { In[S.A].bCheck = F == 0; In[S.A].Move = FVector2D(0.7, 0.0); In[S.V].Move = FVector2D(0.7, 0.0); });
		TestTrue(TEXT("a bump from behind at the same speed: no check, no foul"), H.Outcome == UJamSim::ContactBump && !H.bCheck && H.Foul.IsNone());
	}
	return true;
}

// The neutral block gives back the pre-vision game (re-pinned by lane A95 on the merged tree): with A92's block, A93's movement block and game.ai.sprint 0 written
// into the cartridge's text (and A93's one-sided sweep shield switch, which has no key), two regular-playtester matches replay main bb18439 (A94's brain, before
// A92 and A93; read from that commit's build, Jam.Core.MatchAlwaysEnds): hash for hash, and the model resolves nothing. A92's pins were main 8f6b157's.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamContactNeutral, "Jam.Core.ContactNeutral", JamContactTests::Flags)
bool FJamContactNeutral::RunTest(const FString& Parameters)
{
	struct FMain { const TCHAR* Cart; const TCHAR* Level; uint64 Seed; int64 Frames; uint64 Hash; };
	for (const FMain& M : { FMain{ TEXT("hockey.boost"), TEXT("pro"), 150, 8633, 0x1a33b22080d0f0dfull }, FMain{ TEXT("hockey.regular"), TEXT("all_star"), 387, 9475, 0x59762ff73c73f6e3ull } })
	{
		FString Raw;
		if (!TestTrue(*FString::Printf(TEXT("%s reads"), M.Cart), FFileHelper::LoadFileToString(Raw, *UJamCartridge::ResolveCartridgePath(M.Cart)))) continue;
		TestEqual(TEXT("one game.skater anchor"), Raw.ReplaceInline(TEXT("\"skater\": {"), TEXT("\"contact\": {\"momentum\": false, \"carrier_can_hit\": true}, \"stick\": {\"carry_reach_mult\": 1, \"lift_cone_deg\": 180, \"sweep\": {\"sprint_shield_mult\": 1}}, ")
			TEXT("\"skater\": {\"reverse_brake_decel\": 0, \"burst\": {\"frames\": 0, \"accel_mult\": 1}, \"sprint\": {\"carry_bonus\": 0}, ")), 1);
		TestEqual(TEXT("one game.ai anchor"), Raw.ReplaceInline(TEXT("\"ai\": {"), TEXT("\"ai\": {\"sprint\": 0, ")), 1);
		TArray<FString> Errors;
		UJamCartridge* C = UJamCartridge::LoadFromString(Raw, Errors);
		for (const FString& E : Errors) AddError(FString::Printf(TEXT("neutral blocks in %s: %s"), M.Cart, *E));
		if (!C) continue;
		C->Game.Stick.bSweepShieldOneSided = true;
		C->Game.Stick.bBodyShield = false; C->Game.Stick.bPassiveTake = true; C->Game.Stick.bTeammateTouchTakes = true;   // lane A102: the game before its two stick rules
		FContactCount Count;
		JamAI::FJamSimAdapter Sim(*C);
		Count.Sim = &Sim;
		JamAI::FJamMatchConfig Cfg = JamAI::ToMatchConfigResolved(*C, M.Level);
		JamAI::EDifficulty Lv = JamAI::EDifficulty::Pro;
		JamAI::ParseDifficulty(TCHAR_TO_ANSI(M.Level), Lv);
		char Err[200] = {};
		TestTrue(TEXT("the pre-A95 profile tune applies (lane A95's profile numbers back to bb18439's)"), JamAI::ApplyDifficultyTune(JamAI::PreA95Tune(Lv, C->Game.Rink.LengthM >= JamAI::LargeRinkM), Cfg.AI, Err, sizeof(Err)));
		Count.Human = Cfg.PlayerIndex();
		const JamAI::FJamPlaytestStats S = JamAI::RunPlaytestMatch(Sim, Cfg, M.Seed, JamAI::EPlaytester::Regular, 36000, C->Game.Shot.ChargeFramesMax, false, &JamContactTests::Trace, &Count, 1);
		AddInfo(Line(*FString::Printf(TEXT("JAMCONTACT neutral %s %s seed %llu (main bb18439: %lld frames, %016llx)"), M.Cart, M.Level, M.Seed, M.Frames, M.Hash), S, Count));
		TestEqual(*FString::Printf(TEXT("%s %s: ends on main's frame"), M.Cart, M.Level), static_cast<int64>(S.Frames), M.Frames);
		TestEqual(*FString::Printf(TEXT("%s %s: the neutral blocks' match hash is main's"), M.Cart, M.Level), FString::Printf(TEXT("%016llx"), static_cast<unsigned long long>(S.Hash)),
			FString::Printf(TEXT("%016llx"), static_cast<unsigned long long>(M.Hash)));
		TestEqual(*FString::Printf(TEXT("%s %s: the model resolved nothing"), M.Cart, M.Level), Count.HitBump[0] + Count.HitBump[1] + Count.HitStumble[0] + Count.HitStumble[1] + Count.HitDown[0] + Count.HitDown[1] + Count.Refused[0] + Count.Refused[1], 0);
	}
	// The neutral block as a cartridge text loads and reads back; an unknown key in game.contact is an error.
	FString Json;
	if (TestTrue(TEXT("hockey.regular reads"), FFileHelper::LoadFileToString(Json, *UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")))))
	{
		TArray<FString> Errors;
		FString Neutral = Json;
		TestEqual(TEXT("one game.pass / game.skater anchor"), Neutral.ReplaceInline(TEXT("\"skater\": {"), TEXT("\"contact\": { \"momentum\": false, \"carrier_can_hit\": true }, \"skater\": {")), 1);
		UJamCartridge* N = UJamCartridge::LoadFromString(Neutral, Errors);
		for (const FString& E : Errors) AddError(FString::Printf(TEXT("neutral block: %s"), *E));
		if (N) TestTrue(TEXT("the neutral block reads: momentum false, carrier_can_hit true"), !N->Game.Contact.bMomentum && N->Game.Contact.bCarrierCanHit);
		UJamCartridge* D = UJamCartridge::LoadFromString(Json, Errors);
		if (D) TestTrue(TEXT("absent game.contact: the model (momentum true, carrier_can_hit false, lunge 1.5, thrust 3)"), D->Game.Contact.bMomentum && !D->Game.Contact.bCarrierCanHit
			&& D->Game.Contact.LungeMps == 1.5 && D->Game.Contact.ThrustMps == 3.0);
		FString Tuned = Json;
		Tuned.ReplaceInline(TEXT("\"skater\": {"), TEXT("\"contact\": { \"stand_brace\": 2, \"down_frames_max\": 200 }, \"skater\": {"));
		UJamCartridge* Tn = UJamCartridge::LoadFromString(Tuned, Errors);
		if (TestNotNull(TEXT("a tuned block loads"), Tn)) TestTrue(TEXT("... and reads back"), Tn->Game.Contact.StandBrace == 2.0 && Tn->Game.Contact.DownFramesMax == 200);
		FString Bad = Json;
		Bad.ReplaceInline(TEXT("\"skater\": {"), TEXT("\"contact\": { \"brace\": 2 }, \"skater\": {"));
		Errors.Reset();
		UJamCartridge* B = UJamCartridge::LoadFromString(Bad, Errors);
		TestTrue(TEXT("an unknown key in game.contact is an error"), B == nullptr || Errors.Num() > 0);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamContactMatch, "Jam.Core.ContactMatch", JamContactTests::Flags)
bool FJamContactMatch::RunTest(const FString& Parameters)
{
	for (const TCHAR* Level : { TEXT("pro"), TEXT("all_star") })
	{
		FContactCount C;
		const JamAI::FJamPlaytestStats S = Match(*this, Level, 1, C, [](FJamGameConfig&) {});
		TestEqual(*FString::Printf(TEXT("%s: the match ran its three minutes"), Level), static_cast<int64>(S.Frames), static_cast<int64>(10800));
		AddInfo(Line(*FString::Printf(TEXT("JAMCONTACT match %s seed 1"), Level), S, C));
		for (uint64 Seed = 2; Seed <= 3; ++Seed)
		{
			FContactCount Cs;
			const JamAI::FJamPlaytestStats Ss = Match(*this, Level, Seed, Cs, [](FJamGameConfig&) {});
			AddInfo(Line(*FString::Printf(TEXT("JAMCONTACT match %s seed %llu"), Level, static_cast<unsigned long long>(Seed)), Ss, Cs));
			FContactCount Cn;
			const JamAI::FJamPlaytestStats Sn = Match(*this, Level, Seed, Cn, [](FJamGameConfig& G) { G.Contact.SetNeutral(); });
			AddInfo(Line(*FString::Printf(TEXT("JAMCONTACT neutral %s seed %llu"), Level, static_cast<unsigned long long>(Seed)), Sn, Cn));
		}
		// Two runs agree (the model draws from the sim's seeded generator only).
		FContactCount C2;
		const JamAI::FJamPlaytestStats S2 = Match(*this, Level, 1, C2, [](FJamGameConfig&) {});
		TestEqual(*FString::Printf(TEXT("%s: two runs of the match agree"), Level), FString::Printf(TEXT("%016llx"), static_cast<unsigned long long>(S.Hash)), FString::Printf(TEXT("%016llx"), static_cast<unsigned long long>(S2.Hash)));
	}
	return true;
}

// A measurement, no bar of its own: the cuts that wrote skater.check_speed_min or skater.knockdown_frames, three minutes each at their own level and rules (the
// match may end earlier by its own rules), the model against the neutral block: knockdowns, stumbles and frames down.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamContactModes, "Jam.Core.ContactModes", JamContactTests::Flags)
bool FJamContactModes::RunTest(const FString& Parameters)
{
	const TCHAR* Files[] = { TEXT("modes/hockey.mode.bruisers"), TEXT("modes/hockey.mode.fog-of-war"), TEXT("modes/hockey.mode.cage"), TEXT("modes/hockey.mode.glass-house"), TEXT("hockey.raw") };
	for (const TCHAR* F : Files)
	{
		const FString Path = FPaths::Combine(UJamCartridge::GetCartridgeDir(), FString(F) + TEXT(".cartridge.json"));
		for (int32 Neutral = 0; Neutral < 2; ++Neutral)
		{
			FContactCount C;
			int32 CheckMinX10 = 0, KnockCfg = 0;
			const JamAI::FJamPlaytestStats S = Match(*this, nullptr, 1, C, [&](FJamGameConfig& G) { if (Neutral) G.Contact.SetNeutral(); CheckMinX10 = FMath::RoundToInt32(G.Skater.CheckSpeedMin * 10.0); KnockCfg = G.Skater.KnockdownFrames; }, Path);
			AddInfo(FString::Printf(TEXT("JAMCONTACT mode %s (check_speed_min %.1f, knockdown_frames %d) %s: frames %lld goals %d:%d | knockdowns of bot %d of human %d (bot on bot %d) | stumbles bot %d human %d | ")
				TEXT("model's frames down bot %d human %d | check events by bot %d by human %d | fouls %d"), F, CheckMinX10 / 10.0, KnockCfg, Neutral ? TEXT("neutral") : TEXT("model"), static_cast<long long>(S.Frames), S.GoalsFor, S.GoalsAgainst,
				C.Downs[0], C.Downs[1], C.BotOnBotDowns, C.HitStumble[0], C.HitStumble[1], C.DownFrames[0], C.DownFrames[1], C.Checks[0], C.Checks[1], S.Fouls));
		}
	}
	return true;
}

// A measurement, no bar of its own: Jam.Bot.SeasonTakes wants the AI to shoot at least once in a match of a minute or more, on seeds 1, 2, 7. The rookie bots never
// check and never cross-check, so under the model they only take hits; this prints, for the two rookie takes over seeds 1..10, the AI's shots and the score under
// the model and under the neutral block (= main), with the same runner as that test (the scripted human, PlayerBot).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamContactTakes, "Jam.Core.ContactTakes", JamContactTests::Flags)
bool FJamContactTakes::RunTest(const FString& Parameters)
{
	for (int32 Take = 1; Take <= 2; ++Take)
	{
		const FString Name = FString::Printf(TEXT("take-%d"), Take);
		const FString Path = FPaths::Combine(UJamCartridge::GetCartridgeDir(), TEXT("ladder"), Name + TEXT(".cartridge.json"));
		for (int32 Neutral = 0; Neutral < 2; ++Neutral)
		{
			TArray<FString> Errors;
			UJamCartridge* Cart = UJamCartridge::LoadFromFile(Path, Errors);
			if (!TestNotNull(*FString::Printf(TEXT("%s loads"), *Name), Cart)) continue;
			if (Neutral) Cart->Game.Contact.SetNeutral();
			const JamAI::FJamMatchConfig C = JamAI::ToMatchConfigResolved(*Cart);
			const int32 Pi = C.PlayerIndex();
			const int32 Team = C.Skaters[Pi].Team;
			FString Row;
			int32 Zero = 0, Total = 0, Wins = 0, GoalsFor = 0, GoalsAgainst = 0;
			for (uint64 Seed = 1; Seed <= 10; ++Seed)
			{
				JamAI::FJamSimAdapter Sim(*Cart);
				const JamAI::FJamMatchStats A = JamAI::RunMatch(Sim, C, Seed, JamAI::EJamPlayerMode::PlayerBot, 36000, 60);
				int32 AiShots = 0;
				for (int32 i = 0; i < A.NumSkaters; ++i) AiShots += i == Pi ? 0 : A.Shots[i];
				Row += FString::Printf(TEXT(" s%llu %d:%d shots %d;"), static_cast<unsigned long long>(Seed), A.Goals[Team], A.Goals[1 - Team], AiShots);
				Zero += (AiShots == 0 && A.Frames >= 3600) ? 1 : 0;
				Total += AiShots;
				Wins += (A.bOver && A.Winner == Team) ? 1 : 0;
				GoalsFor += A.Goals[Team];
				GoalsAgainst += A.Goals[1 - Team];
			}
			AddInfo(FString::Printf(TEXT("JAMCONTACT takes %s %s: AI shots total %d, matches of a minute or more without an AI shot %d of 10, human wins %d of 10, goals %d:%d |%s"), *Name,
				Neutral ? TEXT("neutral") : TEXT("model"), Total, Zero, Wins, GoalsFor, GoalsAgainst, *Row));
		}
	}
	return true;
}


// Lane A95 (B-to-A-0037): the cone on the stick lift and on the cross-check's shove (game.stick.lift_cone_deg, default 70). A standing carrier with an attacker
// standing beside it at arm's length (0.3 m between the bodies: inside contact.cross_reach_m and stick.lift_range_m), the attacker's facing turned Theta off the
// line to the carrier; one lift press. Inside the cone the lift is clean (the event's Dir is the unit vector attacker -> victim) and the thrust lands; outside it
// the swing is a miss and nothing lands. With the cone at 180 (the key's "no cone", before A95) every angle lifts.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLiftCone, "Jam.Core.LiftCone", JamContactTests::Flags)
bool FJamLiftCone::RunTest(const FString& Parameters)
{
	using namespace JamContactTests;
	struct FOut { bool bClean = false; bool bShove = false; FVector2D Dir = FVector2D::ZeroVector; FVector2D Want = FVector2D::ZeroVector; };
	auto Try = [&](double ConeDeg, double ThetaDeg) -> FOut
	{
		FOut O;
		FContactScene S = Make(*this, [&](FJamGameConfig& G) { G.Stick.LiftConeDeg = ConeDeg; });
		if (!S.Ok()) return O;
		S.Sim->SetSkater(S.V, FVector2D::ZeroVector, FVector2D::ZeroVector, 0.0);
		S.Sim->SetSkater(S.A, FVector2D(0.0, -20.0), FVector2D::ZeroVector, 0.0);
		if (!HandPuck(S, S.V)) { AddError(TEXT("the victim does not carry")); return O; }
		const double R = S.Sim->SkaterRadius;
		const FVector2D APos(0.0, -(2.0 * R + 0.3));   // beside the carrier (not from behind: Dir . victim's facing = 0)
		const double ToV = UE_DOUBLE_PI * 0.5;         // the line attacker -> victim is +y
		S.Sim->SetSkater(S.A, APos, FVector2D::ZeroVector, ToV + FMath::DegreesToRadians(ThetaDeg));
		O.Want = (S.Vic().Pos - APos).GetSafeNormal();
		TArray<FJamInput> In;
		In.SetNum(S.Sim->GetSkaters().Num());
		In[S.A].bLift = true;
		S.Sim->Step(In);
		for (const FJamSimEvent& E : S.Sim->GetFrameEvents())
		{
			if (E.Name == JamEvents::Lift && E.Who == S.A && E.Strength > 0.5f) { O.bClean = true; O.Dir = E.Dir; }
		}
		O.bShove = S.Vic().ContactOutcome != 0;
		return O;
	};
	for (const double Theta : { 0.0, 30.0, 60.0, 80.0, 110.0, 160.0 })
	{
		const FOut D = Try(70.0, Theta);
		const FOut N = Try(180.0, Theta);
		AddInfo(FString::Printf(TEXT("theta %.0f deg: cone 70 lift %s shove %s dir (%.2f, %.2f) | cone 180 lift %s shove %s"), Theta, D.bClean ? TEXT("clean") : TEXT("miss"), D.bShove ? TEXT("yes") : TEXT("no"),
			D.Dir.X, D.Dir.Y, N.bClean ? TEXT("clean") : TEXT("miss"), N.bShove ? TEXT("yes") : TEXT("no")));
		const bool bInside = Theta <= 70.0;
		TestEqual(*FString::Printf(TEXT("cone 70, %.0f deg: clean lift only inside"), Theta), D.bClean, bInside);
		TestEqual(*FString::Printf(TEXT("cone 70, %.0f deg: the shove lands only inside"), Theta), D.bShove, bInside);
		if (D.bClean) TestTrue(*FString::Printf(TEXT("%.0f deg: the clean lift's Dir is attacker -> victim"), Theta), D.Dir.Equals(D.Want, 0.01));
		TestTrue(*FString::Printf(TEXT("cone 180 (no cone), %.0f deg: the lift is clean"), Theta), N.bClean);
	}
	return true;
}

#endif
