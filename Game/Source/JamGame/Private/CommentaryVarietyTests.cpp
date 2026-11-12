// Lane A50 (R-0033 point 3): Jam.Commentary.Variety. The picker draws at random among a situation's alternates, never the line it played last for that
// situation, preferring lines not yet heard this match. No world: a synthetic bank with one situation of eight alternates. Run:
//   UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Commentary.Variety; Quit" -unattended -nullrhi -nosplash -log
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "CommentarySystem.h"
#include "JamEventBus.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamCommentaryVarietyTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;
	constexpr int32 Alternates = 8;

	FString Line(const FString& Id, const TCHAR* Event, const TCHAR* Text)
	{
		const FString Name = FString(TEXT("VO_Commentator_")) + Id.Replace(TEXT("."), TEXT("_"));
		return FString::Printf(TEXT("{\"id\":\"%s\",\"event\":\"%s\",\"kind\":\"\",\"team\":\"any\",\"priority\":1,\"cooldown_s\":0,\"when\":\"test\",\"text\":\"%s\",\"sound\":\"/JamAssets/Audio/VO/%s.%s\",\"seconds\":1}"),
			*Id, Event, Text, *Name, *Name);
	}

	// One situation, board.hard, with eight alternates at one priority and no cooldown.
	FString Manifest()
	{
		TArray<FString> L;
		for (int32 i = 1; i <= Alternates; ++i) L.Add(Line(FString::Printf(TEXT("board.hard.%02d"), i), TEXT("board"), *FString::Printf(TEXT("Off the glass number %d."), i)));
		return TEXT("{\"schema\":\"hokej.commentary/1\",\"count\":") + FString::FromInt(L.Num()) + TEXT(",\"lines\":[") + FString::Join(L, TEXT(",")) + TEXT("]}");
	}

	FJamCommentaryWorld World(int64 Frame)
	{
		FJamCommentaryWorld W;
		W.Frame = Frame;
		W.HumanIndex = 0;
		W.Teams = { 0, 0, 0, 1, 1, 1 };
		W.Positions = { FVector2D(-10, 0), FVector2D(-10, -10), FVector2D(-30, 0), FVector2D(10, 0), FVector2D(10, 10), FVector2D(30, 0) };
		W.DrawEarly.Init(false, 6);
		W.RinkLengthM = 90; W.RinkWidthM = 45; W.GoalLineX = 41; W.GoalWidthM = 4;
		W.bFirstTo = true; W.ScoreToWin = 3;
		return W;
	}

	// A hard board hit by the puck at frame F; returns the line that started.
	bool BoardHit(FJamCommentaryPicker& Picker, int64 F, FJamCommentaryPick& Out)
	{
		FJamEventPayload P;
		P.Who = -1;
		P.Where = FVector(0, 2200, 0);
		P.Strength = 0.9f;
		const FJamCommentaryWorld W = World(F);
		Picker.Observe(JamEvents::Board, P, FVector2D(0, 22), W);
		return Picker.Resolve(W, Out);
	}
}

using namespace JamCommentaryVarietyTests;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCommentaryVariety, "Jam.Commentary.Variety", JamCommentaryVarietyTests::Flags)
bool FJamCommentaryVariety::RunTest(const FString&)
{
	FJamCommentaryBank Bank;
	TArray<FString> Errors;
	Bank.Parse(Manifest(), Errors);
	for (const FString& E : Errors) AddError(E);
	if (Errors.Num() > 0) return false;
	// Three seeds, so the rule is not one stream's luck. Hits 200 frames apart (the gate is 150 frames, a line lasts 60).
	TSet<FName> FirstLines;
	for (uint64 Seed : { 7ull, 11ull, 23ull })
	{
		FJamCommentaryPicker Picker(Bank, Seed);
		TSet<FName> Heard;
		FName Prev;
		int64 F = 1000;
		// The first eight hits hear all eight alternates (unheard preferred): no repeat before the bank is exhausted.
		for (int32 n = 0; n < Alternates; ++n, F += 200)
		{
			FJamCommentaryPick P;
			TestTrue(*FString::Printf(TEXT("seed %llu hit %d plays"), Seed, n), BoardHit(Picker, F, P));
			TestEqual(TEXT("situation board.hard"), P.Situation, FName(TEXT("board.hard")));
			TestFalse(*FString::Printf(TEXT("seed %llu hit %d: %s not heard before the bank is exhausted"), Seed, n, *P.LineId.ToString()), Heard.Contains(P.LineId));
			Heard.Add(P.LineId);
			if (n == 0) FirstLines.Add(P.LineId);
			Prev = P.LineId;
		}
		TestEqual(*FString::Printf(TEXT("seed %llu: all %d alternates heard once"), Seed, Alternates), Heard.Num(), Alternates);
		// Afterwards every alternate has been heard: a random draw that is never the last line of the situation.
		TSet<FName> Later;
		for (int32 n = 0; n < 24; ++n, F += 200)
		{
			FJamCommentaryPick P;
			TestTrue(TEXT("a later hit plays"), BoardHit(Picker, F, P));
			TestNotEqual(*FString::Printf(TEXT("seed %llu later hit %d: never the last line"), Seed, n), P.LineId, Prev);
			Later.Add(P.LineId);
			Prev = P.LineId;
		}
		TestTrue(TEXT("the later draws use more than two alternates (a random pick, not a cycle of two)"), Later.Num() > 2);
	}
	// The stream is the seed's: two different seeds do not all open with the same line (three seeds, eight alternates).
	TestTrue(TEXT("different seeds do not all open with the same line"), FirstLines.Num() > 1);
	return true;
}

#endif
