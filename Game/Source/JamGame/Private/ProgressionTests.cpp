// Lane A17 tests: Jam.Progression.*. No world, no map:
//   UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Progression; Quit" -unattended -nullrhi -nosplash -log
// LadderValidates mirrors schema/ladder.test.mjs (same pointers); ProgressionUnlocks walks win -> next, loss -> same, resume from
// Progress.json in a temp folder; SeasonRoundTrip loads all eight takes and wins the season through the subsystem.
// The bot suite on takes 1-4 is Jam.Bot.SeasonTakes (JamCore/Tests/SeasonBotTests.cpp: the bot match runner is JamCore-internal).
#include "CoreMinimal.h"
#include "UObject/Package.h"   // GetTransientPackage() outside the unity blob (lane A18, R-0021 pass)
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Engine/GameInstance.h"
#include "Progression.h"
#include "JamCartridge.h"
#include "DifficultyResolve.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamProgressionTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

	FString ShippedLadderText()
	{
		FString T;
		FFileHelper::LoadFileToString(T, *UJamProgression::DefaultLadderPath());
		return T;
	}

	bool HasError(const TArray<FString>& Errors, const FString& Pointer, const FString& Fragment)
	{
		return Errors.ContainsByPredicate([&](const FString& E) { return E.StartsWith(TEXT("at ") + Pointer + TEXT(":")) && E.Contains(Fragment); });
	}

	// A scratch folder with a copy of the shipped ladder's location (the ladder path stays the shipped one; only Progress.json moves).
	FString TempProgressPath(const TCHAR* Name)
	{
		const FString Dir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Automation"), TEXT("Progression"));
		IFileManager::Get().MakeDirectory(*Dir, true);
		const FString P = FPaths::Combine(Dir, FString(Name) + TEXT(".json"));
		IFileManager::Get().Delete(*P, false, true, true);
		return P;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLadderValidates, "Jam.Progression.LadderValidates", JamProgressionTests::Flags)

bool FJamLadderValidates::RunTest(const FString& Parameters)
{
	using namespace JamProgressionTests;
	const FString Text = ShippedLadderText();
	TestFalse(TEXT("the shipped ladder file exists"), Text.IsEmpty());
	FJamLadder L;
	TArray<FString> Errors;
	const bool bOk = UJamProgression::ParseLadder(Text, L, Errors);
	for (const FString& E : Errors) AddError(TEXT("shipped ladder: ") + E);
	TestTrue(TEXT("the shipped ladder parses"), bOk);
	TestEqual(TEXT("eight takes"), L.Takes.Num(), 8);
	TestTrue(TEXT("take 1 is open"), L.Takes.Num() > 0 && L.Takes[0].bUnlockAlways);
	for (int32 i = 0; i < L.Takes.Num(); ++i)
	{
		TestFalse(*FString::Printf(TEXT("take %d has a note"), i + 1), L.Takes[i].TwistText.IsEmpty());
		TestTrue(*FString::Printf(TEXT("take %d after 1 needs the previous win"), i + 1), i == 0 || !L.Takes[i].bUnlockAlways);
	}

	auto Bad = [&](const FString& From, const FString& To, const FString& Pointer, const FString& Fragment)
	{
		FString T = Text;
		if (!T.Contains(From)) { AddError(FString::Printf(TEXT("mutation source not found: %s"), *From)); return; }
		T = T.Replace(*From, *To);
		FJamLadder Out;
		TArray<FString> E;
		const bool bAccepted = UJamProgression::ParseLadder(T, Out, E);
		if (bAccepted || !HasError(E, Pointer, Fragment))
		{
			AddError(FString::Printf(TEXT("expected \"%s\" at %s, got: %s"), *Fragment, *Pointer, E.Num() ? *FString::Join(E, TEXT(" | ")) : TEXT("accepted")));
		}
	};
	Bad(TEXT("\"unlock\": \"always\""), TEXT("\"unlock\": \"always\", \"fall_back\": \"take-2\""), TEXT("/ladder/takes/0/fall_back"), TEXT("unknown key"));
	Bad(TEXT("\"unlock\": \"always\""), TEXT("\"unlock\": \"always\", \"fallback\": \"take-2\""), TEXT("/ladder/takes/1/cartridge"), TEXT("is already used by take 1"));
	Bad(TEXT("\"unlock\": \"always\""), TEXT("\"unlock\": \"previous_won\""), TEXT("/ladder/takes/0/unlock"), TEXT("must unlock \"always\""));
	Bad(TEXT("\"cartridge\": \"take-4\""), TEXT("\"cartridge\": \"take-1\""), TEXT("/ladder/takes/3/cartridge"), TEXT("is already used by take 1"));
	Bad(TEXT("\"cartridge\": \"take-2\""), TEXT("\"cartridge\": null"), TEXT("/ladder/takes/1/cartridge"), TEXT("expected a string"));
	Bad(TEXT("\"schema\": \"tj.ladder/1\""), TEXT("\"schema\": \"tj.ladder/2\""), TEXT("/schema"), TEXT("tj.ladder/1"));
	Bad(TEXT("\"previous_won\""), TEXT("\"stars\""), TEXT("/ladder/takes/1/unlock"), TEXT("\"stars\" is not allowed"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamProgressionUnlocks, "Jam.Progression.ProgressionUnlocks", JamProgressionTests::Flags)

bool FJamProgressionUnlocks::RunTest(const FString& Parameters)
{
	using namespace JamProgressionTests;
	const FString PP = TempProgressPath(TEXT("unlocks"));
	UJamProgression* P = NewObject<UJamProgression>(NewObject<UGameInstance>());
	P->SetPathsForTest(FString(), PP);
	FJamSeasonState S = P->GetSeasonState();
	TestTrue(TEXT("ladder loaded"), S.bLoaded);
	TestEqual(TEXT("absent progress = take 1"), S.CurrentTake, 0);
	TestTrue(TEXT("take 1 unlocked"), S.Takes.Num() == 8 && S.Takes[0].bUnlocked);
	TestFalse(TEXT("take 2 locked"), S.Takes.Num() == 8 && S.Takes[1].bUnlocked);
	TestEqual(TEXT("menu line"), S.MenuLine, FString(TEXT("Take 1 of 8: The Read-Through")));

	// Loss -> same take (RETAKE), the restart key is not taken by the season.
	P->BeginTake(0);
	const FString Take1 = P->TakePath(0);
	TestTrue(TEXT("take 1 is the season match"), P->IsSeasonMatch(Take1));
	TestFalse(TEXT("another cut is not"), P->IsSeasonMatch(UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular"))));
	P->RecordResult(Take1, false);
	TestEqual(TEXT("loss keeps take 1"), P->GetSeasonState().CurrentTake, 0);
	TestEqual(TEXT("loss hint"), P->EndHint(Take1), FString(TEXT("R  REMATCH      ESC  PAUSE")));
	TestFalse(TEXT("loss: R restarts the same cut"), P->HandleRestart(nullptr));
	TestFalse(TEXT("take 2 still locked"), P->GetSeasonState().Takes[1].bUnlocked);

	// Win -> next take.
	P->BeginTake(0);
	P->RecordResult(Take1, true);
	S = P->GetSeasonState();
	TestEqual(TEXT("win moves to take 2"), S.CurrentTake, 1);
	TestTrue(TEXT("take 2 unlocked"), S.Takes[1].bUnlocked);
	TestEqual(TEXT("two attempts on take 1"), S.Takes[0].Attempts, 2);
	TestEqual(TEXT("win hint"), P->EndHint(Take1), FString(TEXT("R  NEXT MATCH: THE BIG ROOM      ESC  PAUSE")));

	// A result for a cut that is not the running take is ignored.
	P->RecordResult(P->TakePath(5), true);
	TestFalse(TEXT("take 6 not won by a stray result"), P->GetSeasonState().Takes[5].bWon);

	// Resume: a fresh subsystem reads Saved/Progress.json.
	FString Saved;
	TestTrue(TEXT("progress written"), FFileHelper::LoadFileToString(Saved, *PP));
	TestTrue(TEXT("absent = omit: no null, no zero wins"), !Saved.Contains(TEXT("null")) && !Saved.Contains(TEXT("\"wins\": 0")));
	UJamProgression* Q = NewObject<UJamProgression>(NewObject<UGameInstance>());
	Q->SetPathsForTest(FString(), PP);
	TestEqual(TEXT("resume at take 2"), Q->GetSeasonState().CurrentTake, 1);
	TestEqual(TEXT("resume keeps attempts"), Q->GetSeasonState().Takes[0].Attempts, 2);
	TestTrue(TEXT("resume: take 1 listed in Choose your cut"), Q->AppendPlayedTakes({}).ContainsByPredicate([](const FJamCutInfo& I) { return I.Name == TEXT("Take 1: The Read-Through") && I.bSaved; }));
	TestEqual(TEXT("unplayed takes are not listed"), Q->AppendPlayedTakes({}).Num(), 1);

	// Unknown keys are errors: a rejected file is the start of the season.
	FFileHelper::SaveStringToFile(Saved.Replace(TEXT("\"attempts\""), TEXT("\"stars\": 3, \"attempts\"")), *PP);
	FJamSeasonProgress Bad;
	TArray<FString> Errors;
	TestFalse(TEXT("unknown key rejected"), UJamProgression::ParseProgress(Saved.Replace(TEXT("\"attempts\""), TEXT("\"stars\": 3, \"attempts\"")), Bad, Errors));
	TestTrue(TEXT("pointer names the key"), HasError(Errors, TEXT("/takes/0/stars"), TEXT("unknown key")));
	Q->Reload();
	TestEqual(TEXT("rejected progress = take 1"), Q->GetSeasonState().CurrentTake, 0);
	IFileManager::Get().Delete(*PP, false, true, true);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamSeasonRoundTrip, "Jam.Progression.SeasonRoundTrip", JamProgressionTests::Flags)

bool FJamSeasonRoundTrip::RunTest(const FString& Parameters)
{
	using namespace JamProgressionTests;
	const FString PP = TempProgressPath(TEXT("roundtrip"));
	UJamProgression* P = NewObject<UJamProgression>(NewObject<UGameInstance>());
	P->SetPathsForTest(FString(), PP);
	const FJamLadder& L = P->GetLadder();
	TestEqual(TEXT("eight takes"), L.Takes.Num(), 8);
	using JamAI::EDifficulty;
	static const EDifficulty Levels[] = { EDifficulty::Rookie, EDifficulty::Rookie, EDifficulty::Rookie, EDifficulty::Rookie,   // lane A112 (R-0048 point 3): take-3 is rookie; lane A133 (R-0057): take-4 too (the season's ramp)
		 EDifficulty::Pro, EDifficulty::Pro, EDifficulty::Pro, EDifficulty::AllStar };
	for (int32 i = 0; i < L.Takes.Num(); ++i)
	{
		TArray<FString> Errors;
		UJamCartridge* C = UJamCartridge::LoadFromFile(P->TakePath(i), Errors, GetTransientPackage());
		if (!C) { AddError(FString::Printf(TEXT("take %d (%s) does not load: %s"), i + 1, *L.Takes[i].Cartridge, *FString::Join(Errors, TEXT("; ")))); continue; }
		TestEqual(*FString::Printf(TEXT("take %d id"), i + 1), C->Id, L.Takes[i].Cartridge);
		TestEqual(*FString::Printf(TEXT("take %d title"), i + 1), C->Title, L.Takes[i].Title);
		JamAI::FJamDifficultyResolution R;
		JamAI::ResolveDifficulty(*C, R);
		TestTrue(*FString::Printf(TEXT("take %d writes its level"), i + 1), R.bLevelWritten);
		if (i < UE_ARRAY_COUNT(Levels)) TestEqual(*FString::Printf(TEXT("take %d level"), i + 1), static_cast<int32>(R.Level), static_cast<int32>(Levels[i]));
		// Walk the season: a loss, then the win that opens the next take.
		TestTrue(*FString::Printf(TEXT("take %d unlocked in turn"), i + 1), P->GetSeasonState().Takes[i].bUnlocked);
		TestEqual(*FString::Printf(TEXT("take %d is current"), i + 1), P->GetSeasonState().CurrentTake, i);
		P->BeginTake(i);
		P->RecordResult(C->SourcePath, false);
		P->BeginTake(i);
		P->RecordResult(C->SourcePath, true);
	}
	const FJamSeasonState S = P->GetSeasonState();
	TestTrue(TEXT("season complete"), S.bComplete);
	TestEqual(TEXT("eight won"), S.TakesWon, 8);
	TestEqual(TEXT("wrapped season replays the final take"), S.CurrentTake, 7);
	TestEqual(TEXT("wrapped hint"), P->EndHint(P->TakePath(7)), FString(TEXT("SEASON COMPLETE      R  MENU")));
	// Text round trip of the progress file.
	FJamSeasonProgress Back;
	TArray<FString> Errors;
	TestTrue(TEXT("progress round-trips"), UJamProgression::ParseProgress(UJamProgression::ProgressToJson(P->GetProgress()), Back, Errors));
	TestEqual(TEXT("same takes"), Back.Takes.Num(), 8);
	TestEqual(TEXT("same attempts"), Back.Takes.Num() == 8 ? Back.Takes[7].Attempts : -1, 2);
	IFileManager::Get().Delete(*PP, false, true, true);

	// R-0014: a designed take the loader rejects plays its fallback.
	const FString Dir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Automation"), TEXT("ProgressionFallback"));
	IFileManager::Get().DeleteDirectory(*Dir, false, true);
	IFileManager::Get().MakeDirectory(*Dir, true);
	const FString Shipped = FPaths::GetPath(UJamProgression::DefaultLadderPath());
	IFileManager::Get().Copy(*FPaths::Combine(Dir, TEXT("take-1.cartridge.json")), *FPaths::Combine(Shipped, TEXT("take-1.cartridge.json")));
	IFileManager::Get().Copy(*FPaths::Combine(Dir, TEXT("take-2.cartridge.json")), *FPaths::Combine(Shipped, TEXT("take-2.cartridge.json")));
	FFileHelper::SaveStringToFile(TEXT("{ \"schema\": \"tj.cartridge/1\" }"), *FPaths::Combine(Dir, TEXT("not-landed.cartridge.json")));
	FFileHelper::SaveStringToFile(TEXT("{ \"schema\": \"tj.ladder/1\", \"ladder\": { \"id\": \"fallback-test\", \"takes\": [ ")
		TEXT("{ \"title\": \"One\", \"cartridge\": \"take-1\", \"twist_text\": \"a\", \"unlock\": \"always\" }, ")
		TEXT("{ \"title\": \"Two\", \"cartridge\": \"not-landed\", \"twist_text\": \"b\", \"unlock\": \"always\", \"fallback\": \"take-2\" } ] } }"),
		*FPaths::Combine(Dir, TEXT("test.ladder.json")));
	UJamProgression* F = NewObject<UJamProgression>(NewObject<UGameInstance>());
	F->SetPathsForTest(FPaths::Combine(Dir, TEXT("test.ladder.json")), FPaths::Combine(Dir, TEXT("progress.json")));
	bool bFallback = false;
	const FString Played = F->TakePath(1, &bFallback);
	TestTrue(TEXT("rejected designed take -> fallback"), bFallback && Played.EndsWith(TEXT("take-2.cartridge.json")));
	F->BeginTake(1);
	TestTrue(TEXT("the fallback is the season match"), F->IsSeasonMatch(Played));
	TestEqual(TEXT("the take keeps its title and note"), F->ActiveNote(Played), FString(TEXT("b")));
	bool bFallback0 = true;
	F->TakePath(0, &bFallback0);
	TestFalse(TEXT("a take without a fallback plays its cut"), bFallback0);
	IFileManager::Get().DeleteDirectory(*Dir, false, true);
	return true;
}

#endif
