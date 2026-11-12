// Lane A2: the milestone bot tests (docs/HOCKEY.md 7). Headless, no world, no map:
//   UnrealEditor-Cmd ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Bot; Quit" -nullrhi -unattended -nopause -stdout
// They run on lane A1's UJamSim (through JamAI::FJamSimAdapter) with the shipped cartridge files hockey.regular and
// hockey.arena.small as UJamCartridge loads them.
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "../AI/JamBotMatch.h"
#include "../AI/JamSimAdapter.h"
#include "JamCartridge.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamBotTestsLocal
{
	constexpr int64 MaxFrames = 36000;   // 10 minutes; a first-to-3 match that has not ended by then fails

	const char* PresetName(int32 Which) { return Which == 0 ? "hockey.regular" : "hockey.arena.small"; }

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

	JamAI::FJamMatchStats Run(const UJamCartridge& Cart, uint64 Seed, JamAI::EJamPlayerMode Mode)
	{
		JamAI::FJamSimAdapter Sim(Cart.Game);
		return JamAI::RunMatch(Sim, JamAI::ToMatchConfig(Cart.Game), Seed, Mode, MaxFrames, 60);
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
	using namespace JamBotTestsLocal;
	bool bOk = true;
	for (int32 P = 0; P < 2; ++P)
	{
		UJamCartridge* Cart = Load(*this, P);
		if (!Cart) { bOk = false; continue; }
		const JamAI::FJamMatchConfig C = JamAI::ToMatchConfig(Cart->Game);
		const int32 Pi = C.PlayerIndex();
		const int32 PlayerTeam = C.Skaters[Pi].Team;
		for (const uint64 Seed : {1ull, 2ull, 7ull})
		{
			const JamAI::FJamMatchStats A = Run(*Cart, Seed, JamAI::EJamPlayerMode::PlayerBot);
			const JamAI::FJamMatchStats B = Run(*Cart, Seed, JamAI::EJamPlayerMode::PlayerBot);
			AddInfo(Line(A, PresetName(P), JamAI::EJamPlayerMode::PlayerBot));
			bool bSame = A.Hash == B.Hash && A.NumHashes == B.NumHashes && A.Frames == B.Frames;
			for (int32 h = 0; bSame && h < A.NumHashes; ++h)
			{
				bSame = A.Hashes[h] == B.Hashes[h];
			}
			const bool bWin = A.bOver && A.Winner == PlayerTeam;
			const bool bShots = A.Shots[Pi] >= 3;
			const FString Msg = FString::Printf(TEXT("%hs seed %llu: %d:%d in %lld frames, player shots %d, hash %016llx (%s on the second run)"),
				PresetName(P), Seed, A.Goals[PlayerTeam], A.Goals[1 - PlayerTeam], A.Frames, A.Shots[Pi], A.Hash, bSame ? TEXT("same") : TEXT("DIFFERENT"));
			// the gate is hockey.regular; the small rink is reported, and fails the test too (it is a shipped cartridge)
			if (bWin && bShots && bSame) { AddInfo(TEXT("PASS ") + Msg); }
			else { AddError(TEXT("FAIL ") + Msg); bOk = false; }
		}
	}
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
		const FString Msg = FString::Printf(TEXT("%hs seed 1: least moved %.1f m, puck path %.1f m, AI shots %d, worst board distance %.3f m, over %s"),
			PresetName(P), MinMoved, S.PuckPathM, AiShots, S.MaxBoardDistance, S.bOver ? TEXT("yes") : TEXT("no"));
		if (bMoved && S.PuckPathM > 30.0 && AiShots >= 1 && bInside && S.bOver) { AddInfo(TEXT("PASS ") + Msg); }
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
			const FString Msg = FString::Printf(TEXT("%hs seed %llu: %d faceoffs, %d without a human touch inside 600 frames, worst %d frames"),
				PresetName(P), Seed, S.Faceoffs, S.FaceoffsWithoutPlayerTouch, S.MaxFaceoffToTouch);
			if (S.bOver && S.FaceoffsWithoutPlayerTouch == 0) { AddInfo(TEXT("PASS ") + Msg); }
			else { AddError(TEXT("FAIL ") + Msg); bOk = false; }
		}
	}
	return bOk;
}

#endif
