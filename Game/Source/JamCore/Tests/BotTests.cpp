// Lane A2: the milestone bot tests (docs/HOCKEY.md 7). Headless, no world, no map:
//   UnrealEditor-Cmd ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Bot; Quit" -nullrhi -unattended -nopause -stdout
// They run on lane A1's UJamSim (through JamAI::FJamSimAdapter) with the shipped cartridge files hockey.regular and
// hockey.arena.large as UJamCartridge loads them (R-0010: regular is 90 x 45 speed 11, large is the old 120 x 60 speed 8).
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "../AI/JamBotMatch.h"
#include "../AI/JamSimAdapter.h"
#include "../AI/DifficultyResolve.h"
#include "../AI/JamPlaytester.h"
#include "JamCartridge.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamBotTestsLocal
{
	constexpr int64 MaxFrames = 36000;   // 10 minutes; a first-to-3 match that has not ended by then fails

	const char* PresetName(int32 Which) { return Which == 0 ? "hockey.regular" : "hockey.arena.large"; }

	UJamCartridge* Load(FAutomationTestBase& T, int32 Which)
	{
		TArray<FString> Errors;
		UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(UTF8_TO_TCHAR(PresetName(Which))), Errors);
		if (!C)
		{
			T.AddError(FString::Printf(TEXT("FAIL %hs does not load: %s"), PresetName(Which), *FString::Join(Errors, TEXT("; "))));
		}
		return C;
	}

	// lane A3: the bots play the cartridge's game.ai.difficulty (every preset is pro) unless Level names another level.
	JamAI::FJamMatchStats Run(const UJamCartridge& Cart, uint64 Seed, JamAI::EJamPlayerMode Mode, const TCHAR* Level = nullptr)
	{
		JamAI::FJamSimAdapter Sim(Cart);   // lane A41: with the game mode's rule, zone and effect systems
		return JamAI::RunMatch(Sim, JamAI::ToMatchConfigResolved(Cart, Level), Seed, Mode, MaxFrames, 60);
	}

	FString Line(const JamAI::FJamMatchStats& S, const char* Cart, JamAI::EJamPlayerMode Mode)
	{
		static char Buf[8192];
		JamAI::FormatStatsJson(S, Cart, JamAI::ModeName(Mode), Buf, sizeof(Buf));
		return UTF8_TO_TCHAR(Buf);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotPlayerBotWins, "Jam.Bot.PlayerBotWins",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::CommandletContext | EAutomationTestFlags::EngineFilter)

bool FJamBotPlayerBotWins::RunTest(const FString& Parameters)
{
	// R-0011 point 3: the win gate is hockey.regular at rookie (seeds 1, 2, 7: WIN, >= 3 shots, identical hashes), no first-goal clause;
	// at pro, where the backs block every shot, the first goal comes after frame 600 in at least 2 of the 3 seeds.
	using namespace JamBotTestsLocal;
	bool bOk = true;
	UJamCartridge* Cart = Load(*this, 0);
	if (!Cart) { return false; }
	const JamAI::FJamMatchConfig C = JamAI::ToMatchConfig(Cart->Game);
	const int32 Pi = C.PlayerIndex();
	const int32 PlayerTeam = C.Skaters[Pi].Team;
	int32 NotInstant = 0;
	for (const uint64 Seed : {1ull, 2ull, 7ull})
	{
		const JamAI::FJamMatchStats A = Run(*Cart, Seed, JamAI::EJamPlayerMode::PlayerBot, TEXT("rookie"));
		const JamAI::FJamMatchStats B = Run(*Cart, Seed, JamAI::EJamPlayerMode::PlayerBot, TEXT("rookie"));
		AddInfo(Line(A, PresetName(0), JamAI::EJamPlayerMode::PlayerBot));
		bool bSame = A.Hash == B.Hash && A.NumHashes == B.NumHashes && A.Frames == B.Frames;
		for (int32 h = 0; bSame && h < A.NumHashes; ++h)
		{
			bSame = A.Hashes[h] == B.Hashes[h];
		}
		const bool bWin = A.bOver && A.Winner == PlayerTeam;
		const bool bShots = A.Shots[Pi] >= 3;
		const FString Msg = FString::Printf(TEXT("%hs seed %llu (rookie): %d:%d in %lld frames, player shots %d, hash %016llx (%s on the second run)"),
			PresetName(0), Seed, A.Goals[PlayerTeam], A.Goals[1 - PlayerTeam], A.Frames, A.Shots[Pi], A.Hash, bSame ? TEXT("same") : TEXT("DIFFERENT"));
		if (bWin && bShots && bSame) { AddInfo(TEXT("PASS ") + Msg); }
		else { AddError(TEXT("FAIL ") + Msg); bOk = false; }

		const JamAI::FJamMatchStats Pro = Run(*Cart, Seed, JamAI::EJamPlayerMode::PlayerBot);   // the preset's level: pro
		const int32 FirstGoal = Pro.Goals[0] + Pro.Goals[1] > 0 ? Pro.GoalFrames[0] : -1;
		NotInstant += (FirstGoal < 0 || FirstGoal > 600) ? 1 : 0;
		AddInfo(FString::Printf(TEXT("%hs seed %llu (pro): %d:%d, first goal at frame %d"), PresetName(0), Seed, Pro.Goals[PlayerTeam], Pro.Goals[1 - PlayerTeam], FirstGoal));
	}
	const FString Msg = FString::Printf(TEXT("%hs (pro): first goal after frame 600 in %d of 3 seeds (need 2)"), PresetName(0), NotInstant);
	if (NotInstant >= 2) { AddInfo(TEXT("PASS ") + Msg); }
	else { AddError(TEXT("FAIL ") + Msg); bOk = false; }
	return bOk;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotAiBotPlays, "Jam.Bot.AiBotPlays",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::CommandletContext | EAutomationTestFlags::EngineFilter)

bool FJamBotAiBotPlays::RunTest(const FString& Parameters)
{
	using namespace JamBotTestsLocal;
	bool bOk = true;
	for (int32 P = 0; P < 2; ++P)
	{
		UJamCartridge* Cart = Load(*this, P);
		if (!Cart) { bOk = false; continue; }
		const int32 Pi = JamAI::ToMatchConfig(Cart->Game).PlayerIndex();
		const JamAI::FJamMatchStats S = Run(*Cart, 1, JamAI::EJamPlayerMode::PlayerBotAI);
		AddInfo(Line(S, PresetName(P), JamAI::EJamPlayerMode::PlayerBotAI));
		bool bMoved = true;
		double MinMoved = 1e9;
		int32 AiShots = 0;
		for (int32 i = 0; i < S.NumSkaters; ++i)
		{
			MinMoved = FMath::Min(MinMoved, S.MovedMax[i]);
			bMoved &= S.MovedMax[i] >= 3.0;
			AiShots += i == Pi ? 0 : S.Shots[i];
		}
		const bool bInside = S.MaxBoardDistance <= 0.01;
		// R-0003 / R-0011: hockey.regular (90 x 45) must end within 36,000 frames; hockey.arena.large has no goal clause.
		// R-0011 point 3: on hockey.arena.large (timer mode, guarding backs) movement, puck path and shots only, no goal clause
		const bool bFinish = P == 1 ? true : S.bOver;
		const FString Msg = FString::Printf(TEXT("%hs seed 1: least moved %.1f m, puck path %.1f m, AI shots %d, worst board distance %.3f m, over %s, first goal at frame %d"),
			PresetName(P), MinMoved, S.PuckPathM, AiShots, S.MaxBoardDistance, S.bOver ? TEXT("yes") : TEXT("no"), S.Goals[0] + S.Goals[1] > 0 ? S.GoalFrames[0] : -1);
		if (bMoved && S.PuckPathM > 30.0 && AiShots >= 1 && bInside && bFinish) { AddInfo(TEXT("PASS ") + Msg); }
		else { AddError(TEXT("FAIL ") + Msg); bOk = false; }
	}
	return bOk;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBotHumanHasSomethingToDo, "Jam.Bot.HumanHasSomethingToDo",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::CommandletContext | EAutomationTestFlags::EngineFilter)

bool FJamBotHumanHasSomethingToDo::RunTest(const FString& Parameters)
{
	using namespace JamBotTestsLocal;
	bool bOk = true;
	for (int32 P = 0; P < 2; ++P)
	{
		UJamCartridge* Cart = Load(*this, P);
		if (!Cart) { bOk = false; continue; }
		for (const uint64 Seed : {1ull, 2ull, 7ull})
		{
			const JamAI::FJamMatchStats S = Run(*Cart, Seed, JamAI::EJamPlayerMode::PlayerBot);
			const FString Msg = FString::Printf(TEXT("%hs seed %llu: %d faceoffs, %d without a human touch inside 600 frames, worst %d frames, over %s"),
				PresetName(P), Seed, S.Faceoffs, S.FaceoffsWithoutPlayerTouch, S.MaxFaceoffToTouch, S.bOver ? TEXT("yes") : TEXT("no (10-minute window)"));
			// lane A3: at pro some 120 x 60 matches run the full 10-minute window; the window is the "full match" here
			if ((S.bOver || S.Frames >= MaxFrames) && S.FaceoffsWithoutPlayerTouch == 0) { AddInfo(TEXT("PASS ") + Msg); }
			else { AddError(TEXT("FAIL ") + Msg); bOk = false; }
		}
	}
	// R-0029 point 5 (amended): the same window for lane A34's novice playtester at rookie on hockey.regular, 20 seeds, with a budget of
	// 10 % of the faceoffs without a human touch inside 600 frames (frames in the penalty box do not count: nobody touches the puck from there).
	// Measured 19:53: 12 of 126, 74 % of the lost windows a loose-puck chase.
	if (UJamCartridge* Cart = Load(*this, 0))
	{
		int32 Without = 0, Faceoffs = 0;
		for (uint64 Seed = 1; Seed <= 20; ++Seed)
		{
			JamAI::FJamSimAdapter Sim(*Cart);
			const JamAI::FJamPlaytestStats S = JamAI::RunPlaytestMatch(Sim, JamAI::ToMatchConfigResolved(*Cart, TEXT("rookie")), Seed, JamAI::EPlaytester::Novice,
				MaxFrames, Cart->Game.Shot.ChargeFramesMax);
			Without += S.FaceoffsWithoutTouch;
			Faceoffs += S.Faceoffs;
		}
		const int32 Budget = Faceoffs / 10;
		const FString Msg = FString::Printf(TEXT("hockey.regular rookie novice, seeds 1-20: %d of %d faceoffs without a human touch inside 600 frames (budget 10 %%: %d)"), Without, Faceoffs, Budget);
		if (Without <= Budget) { AddInfo(TEXT("PASS ") + Msg); } else { AddError(TEXT("FAIL ") + Msg); bOk = false; }
	}
	else { bOk = false; }
	return bOk;
}

#endif
