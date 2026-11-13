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
// Run:
//   UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Core.Contact; Quit" -unattended -nullrhi -nosplash -log
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "JamBotMatch.h"
#include "DifficultyResolve.h"
#include "JamCartridge.h"
#include "JamEventBus.h"
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
		}
		int32 Checker = -1;
		for (const FJamSimEvent& E : Sim->GetFrameEvents())
		{
			if (E.Name == JamEvents::Check) { Checker = E.Who; ++C.Checks[E.Who == C.Human ? 1 : 0]; }
			if (E.Name == JamEvents::Knockdown)
			{
				++C.Downs[E.Who == C.Human ? 1 : 0];
				if (E.Who != C.Human && Checker >= 0 && Checker != C.Human) ++C.BotOnBotDowns;
			}
		}
	}

	// hockey.regular at a level, penalties off and no score limit inside the three minutes (A73's cut), 10 800 frames, the Regular playtester as the human.
	// File = another cartridge (its own rules kept, Level null = its own level).
	JamAI::FJamPlaytestStats Match(FAutomationTestBase& T, const TCHAR* Level, uint64 Seed, FContactCount& Count, TFunctionRef<void(FJamGameConfig&)> Edit, const FString& File = FString())
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
		return JamAI::RunPlaytestMatch(Sim, M, Seed, JamAI::EPlaytester::Regular, 10800, C->Game.Shot.ChargeFramesMax, false, &Trace, &Count, 1);
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
}

using namespace JamContactTests;

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
		FContactScene S = Make(*this, [](FJamGameConfig& G) { G.Rules.bPenalties = true; });
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

#endif

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
