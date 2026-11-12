// Lane A12 tests: Jam.Signals.*. No world: the routing from key slots to intents, the acknowledgement situations, the intent window and the key map.
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "SignalSystem.h"
#include "SignalVoice.h"
#include "BarkSystem.h"
#include "InputCoreTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamSignalsRouteToIntent, "Jam.Signals.SignalsRouteToIntent", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamSignalsRouteToIntent::RunTest(const FString&)
{
	TestEqual(TEXT("1 -> defend"), UJamSignalSystem::IntentForSlot(1), EJamTeamIntent::Defend);
	TestEqual(TEXT("2 -> space"), UJamSignalSystem::IntentForSlot(2), EJamTeamIntent::Space);
	TestEqual(TEXT("3 -> pass"), UJamSignalSystem::IntentForSlot(3), EJamTeamIntent::Pass);
	TestEqual(TEXT("4 -> praise"), UJamSignalSystem::IntentForSlot(4), EJamTeamIntent::Praise);
	TestEqual(TEXT("5 -> none"), UJamSignalSystem::IntentForSlot(5), EJamTeamIntent::None);

	TestEqual(TEXT("defend ack"), UJamSignalSystem::AckSituationFor(EJamTeamIntent::Defend), JamBarks::SignalAckDefend);
	TestEqual(TEXT("space ack"), UJamSignalSystem::AckSituationFor(EJamTeamIntent::Space), JamBarks::SignalAckSpace);
	TestEqual(TEXT("pass ack"), UJamSignalSystem::AckSituationFor(EJamTeamIntent::Pass), JamBarks::SignalAckPass);
	TestEqual(TEXT("praise reply"), UJamSignalSystem::AckSituationFor(EJamTeamIntent::Praise), JamBarks::PraiseReply);
	TestTrue(TEXT("none has no ack"), UJamSignalSystem::AckSituationFor(EJamTeamIntent::None).IsNone());

	// The intent record: default frames per intent, active from the issued frame for Frames frames, then over.
	const FJamTeamIntent D = UJamSignalSystem::Make(EJamTeamIntent::Defend, 1000, 0, -1);
	TestEqual(TEXT("defend lasts 300 frames"), D.Frames, 300);
	TestTrue(TEXT("active at issue"), D.IsActive(1000));
	TestTrue(TEXT("active at 1299"), D.IsActive(1299));
	TestFalse(TEXT("over at 1300"), D.IsActive(1300));
	TestFalse(TEXT("not active before issue"), D.IsActive(999));
	TestEqual(TEXT("frames left at 1100"), D.FramesLeft(1100), 200);
	const FJamTeamIntent P = UJamSignalSystem::Make(EJamTeamIntent::Pass, 0, 0, 2);
	TestEqual(TEXT("pass lasts 180 frames"), P.Frames, 180);
	TestEqual(TEXT("pass target kept"), P.TargetIndex, 2);
	TestEqual(TEXT("praise lasts 300 frames"), UJamSignalSystem::Make(EJamTeamIntent::Praise, 0, 0, 1).Frames, 300);
	TestFalse(TEXT("none is never active"), UJamSignalSystem::Make(EJamTeamIntent::None, 0, 0, -1).IsActive(0));

	// Wire names round-trip (the bus Kind and the bark situation suffix).
	for (EJamTeamIntent I : { EJamTeamIntent::Defend, EJamTeamIntent::Space, EJamTeamIntent::Pass, EJamTeamIntent::Praise })
	{
		TestEqual(TEXT("name round-trip"), FJamTeamIntent::FromName(FJamTeamIntent::NameOf(I)), I);
	}
	TestEqual(TEXT("unknown name -> none"), FJamTeamIntent::FromName(TEXT("attack")), EJamTeamIntent::None);

	// Voice phrases (Phase 2b): every grammar phrase routes to its intent, case and punctuation ignored; unknown text to none.
	for (const TPair<FString, EJamTeamIntent>& Ph : FJamSignalVoice::Phrases())
	{
		TestEqual(*FString::Printf(TEXT("phrase \"%s\""), *Ph.Key), FJamSignalVoice::IntentForPhrase(Ph.Key.ToUpper() + TEXT("!")), Ph.Value);
	}
	TestEqual(TEXT("\"Fall back.\" -> defend"), FJamSignalVoice::IntentForPhrase(TEXT("Fall back.")), EJamTeamIntent::Defend);
	TestEqual(TEXT("unknown phrase -> none"), FJamSignalVoice::IntentForPhrase(TEXT("shoot the puck")), EJamTeamIntent::None);
	TestEqual(TEXT("empty -> none"), FJamSignalVoice::IntentForPhrase(FString()), EJamTeamIntent::None);

	// Keys: 1-4 and the d-pad map to the four slots, nothing else does.
	UJamSignalInputMap* M = UJamSignalInputMap::Get();
	TestNotNull(TEXT("map"), M);
	if (M)
	{
		TestTrue(TEXT("1 -> slot 0"), M->IsKeyMapped(0, EKeys::One));
		TestTrue(TEXT("2 -> slot 1"), M->IsKeyMapped(1, EKeys::Two));
		TestTrue(TEXT("3 -> slot 2"), M->IsKeyMapped(2, EKeys::Three));
		TestTrue(TEXT("4 -> slot 3"), M->IsKeyMapped(3, EKeys::Four));
		TestTrue(TEXT("d-pad down -> defend"), M->IsKeyMapped(0, EKeys::Gamepad_DPad_Down));
		TestTrue(TEXT("d-pad up -> space"), M->IsKeyMapped(1, EKeys::Gamepad_DPad_Up));
		TestTrue(TEXT("d-pad left -> pass"), M->IsKeyMapped(2, EKeys::Gamepad_DPad_Left));
		TestTrue(TEXT("d-pad right -> praise"), M->IsKeyMapped(3, EKeys::Gamepad_DPad_Right));
		TestFalse(TEXT("space bar is not a signal"), M->IsKeyMapped(0, EKeys::SpaceBar));
		TestFalse(TEXT("1 is not praise"), M->IsKeyMapped(3, EKeys::One));
	}
	return true;
}

#endif
