// Lane A50 (R-0033 point 4): Jam.Fx.GoalVariant. MS_goal's Variant is 0 when the human's team scores and 1 when it concedes; the scoring team comes from
// the score delta (the goal payload's Who is the last touch, the wrong team on an own goal). Pure, no world. Run:
//   UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Fx.GoalVariant; Quit" -unattended -nullrhi -nosplash -log
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "EffectsBinder.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFxGoalVariant, "Jam.Fx.GoalVariant", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamFxGoalVariant::RunTest(const FString&)
{
	const int32 Zero[2] = { 0, 0 };
	const int32 Blue1[2] = { 1, 0 };
	const int32 Orange1[2] = { 0, 1 };
	const int32 Both[2] = { 1, 1 };
	// The team whose score grew scores, whatever Who's team says (an own goal: the last touch is the conceding team).
	TestEqual(TEXT("team 0 grew: team 0"), UJamEffectsBinder::ResolveScoringTeam(Zero, Blue1, 1), 0);
	TestEqual(TEXT("team 1 grew: team 1"), UJamEffectsBinder::ResolveScoringTeam(Zero, Orange1, 0), 1);
	TestEqual(TEXT("from 1-0 to 1-1: team 1"), UJamEffectsBinder::ResolveScoringTeam(Blue1, Both, -1), 1);
	// No growth (a forced event, a rule's AddScore elsewhere): Who's team, else unknown.
	TestEqual(TEXT("no growth: the fallback team"), UJamEffectsBinder::ResolveScoringTeam(Both, Both, 1), 1);
	TestEqual(TEXT("no growth, no fallback: unknown"), UJamEffectsBinder::ResolveScoringTeam(Both, Both, -1), -1);
	// Variant: 0 the human's team scored, 1 it conceded; unknown plays the concede cheer (never the celebration for a goal that may not be ours).
	TestEqual(TEXT("human team 0 scores: Variant 0"), UJamEffectsBinder::GoalVariant(0, 0), 0.f);
	TestEqual(TEXT("human team 0 concedes: Variant 1"), UJamEffectsBinder::GoalVariant(1, 0), 1.f);
	TestEqual(TEXT("human team 1 scores: Variant 0"), UJamEffectsBinder::GoalVariant(1, 1), 0.f);
	TestEqual(TEXT("unknown scorer: Variant 1"), UJamEffectsBinder::GoalVariant(-1, 0), 1.f);
	// Every other event keeps its B-to-A-0012 table: a goal with no Kind is Variant 0 there (the binder overrides it with GoalVariant at play time).
	TestEqual(TEXT("the static table's goal is 0"), UJamEffectsBinder::VariantFor(JamEvents::Goal, NAME_None), 0.f);
	return true;
}

#endif
