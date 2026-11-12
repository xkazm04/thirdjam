// Lane A12 tests: Jam.Barks.*. No world, no map: the picker is driven with synthetic events and a synthetic world.
//   UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Barks; Quit" -unattended -nullrhi -nosplash -log
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "BarkSystem.h"
#include "BarkWidget.h"
#include "SignalIntent.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamBarkTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

	// Six skaters as in hockey.regular: team 0 = indices 0..2 (human is 0), team 1 = 3..5.
	FJamBarkWorld World()
	{
		FJamBarkWorld W;
		W.HumanIndex = 0;
		W.Teams = { 0, 0, 0, 1, 1, 1 };
		W.Positions = { FVector2D(-10, 0), FVector2D(-10, -10), FVector2D(-30, 0), FVector2D(10, 0), FVector2D(10, 10), FVector2D(30, 0) };
		return W;
	}

	FJamEventPayload Payload(int32 Who, const FVector2D& WhereM)
	{
		FJamEventPayload P;
		P.Who = Who;
		P.Where = FVector(WhereM.X * 100, WhereM.Y * 100, 0);
		return P;
	}

	// A small bank with two lines per combination the tests need.
	FString TestBankJson()
	{
		TArray<FString> Lines;
		const TCHAR* Situations[] = { TEXT("goal_for"), TEXT("goal_against"), TEXT("check_landed"), TEXT("check_received"), TEXT("faceoff"), TEXT("win"), TEXT("lose"), TEXT("directors_call"), TEXT("steal"), TEXT("stall"), TEXT("near_miss") };
		const TCHAR* OppTones[] = { TEXT("mock"), TEXT("complain"), TEXT("bravado") };
		const TCHAR* MateTones[] = { TEXT("encourage"), TEXT("praise"), TEXT("complain") };
		for (const TCHAR* S : Situations)
		{
			for (const TCHAR* T : OppTones) for (int32 i = 1; i <= 12; ++i) Lines.Add(FString::Printf(TEXT("{\"id\":\"%s.opponent.%s.%02d\",\"situation\":\"%s\",\"speaker\":\"opponent\",\"tone\":\"%s\",\"text\":\"opp %s %s %d\"}"), S, T, i, S, T, S, T, i));
			for (const TCHAR* T : MateTones) for (int32 i = 1; i <= 12; ++i) Lines.Add(FString::Printf(TEXT("{\"id\":\"%s.teammate.%s.%02d\",\"situation\":\"%s\",\"speaker\":\"teammate\",\"tone\":\"%s\",\"text\":\"mate %s %s %d\"}"), S, T, i, S, T, S, T, i));
		}
		for (const TCHAR* S : { TEXT("signal_ack_defend"), TEXT("signal_ack_space"), TEXT("signal_ack_pass"), TEXT("praise_reply") })
		{
			for (int32 i = 1; i <= 12; ++i) Lines.Add(FString::Printf(TEXT("{\"id\":\"%s.teammate.encourage.%02d\",\"situation\":\"%s\",\"speaker\":\"teammate\",\"tone\":\"encourage\",\"text\":\"ack %s %d\"}"), S, i, S, S, i));
		}
		return TEXT("{\"schema\":\"hokej.barks/1\",\"lines\":[") + FString::Join(Lines, TEXT(",")) + TEXT("]}");
	}
}

using namespace JamBarkTests;

// The embedded bank parses, matches the file on disk, has no duplicates, every line is short ASCII, and every situation x speaker x tone
// the design names has at least 12 lines (the word-list filter itself runs in Node: tools/barks/barks.test.mjs).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBarkBankClean, "Jam.Barks.BankClean", JamBarkTests::Flags)
bool FJamBarkBankClean::RunTest(const FString&)
{
	FJamBarkBank Bank;
	TArray<FString> Errors;
	const bool bParsed = Bank.Parse(FJamBarkBank::EmbeddedJson(), Errors);
	for (const FString& E : Errors) AddError(E);
	TestTrue(TEXT("embedded bank parses"), bParsed);
	TestTrue(TEXT("bank has at least 600 lines"), Bank.Lines.Num() >= 600);

	const FString DiskPath = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Barks"), TEXT("barks.json"));
	FString Disk;
	if (FFileHelper::LoadFileToString(Disk, *DiskPath))
	{
		TestEqual(TEXT("embedded copy equals Game/Content/Barks/barks.json (run node tools/barks/embed.mjs)"), FJamBarkBank::EmbeddedJson().Replace(TEXT("\r"), TEXT("")), Disk.Replace(TEXT("\r"), TEXT("")));
	}
	else AddInfo(TEXT("no barks.json on disk (packaged run): embedded copy only"));

	TSet<FString> Texts;
	for (const FJamBarkLine& L : Bank.Lines)
	{
		TestTrue(*FString::Printf(TEXT("%s is 6..60 characters"), *L.Id.ToString()), L.Text.Len() >= 6 && L.Text.Len() <= 60);
		const FString Norm = L.Text.ToLower();
		TestFalse(*FString::Printf(TEXT("%s duplicates another line"), *L.Id.ToString()), Texts.Contains(Norm));
		Texts.Add(Norm);
		for (TCHAR C : L.Text) if (C < 0x20 || C > 0x7E) { AddError(FString::Printf(TEXT("%s has a non-ASCII character"), *L.Id.ToString())); break; }
	}
	const FName OppSituations[] = { JamBarks::GoalFor, JamBarks::GoalAgainst, JamBarks::CheckLanded, JamBarks::CheckReceived, JamBarks::Steal, JamBarks::Stall, JamBarks::NearMiss, JamBarks::Faceoff, JamBarks::Win, JamBarks::Lose, JamBarks::DirectorsCall };
	const FName MateOnly[] = { JamBarks::SignalAckDefend, JamBarks::SignalAckSpace, JamBarks::SignalAckPass, JamBarks::PraiseReply };
	int32 Combos = 0;
	for (const FName& S : OppSituations)
	{
		for (const FName& T : { JamBarks::ToneMock, JamBarks::ToneComplain, JamBarks::ToneBravado }) { TestTrue(*FString::Printf(TEXT("%s/opponent/%s has >= 12 lines"), *S.ToString(), *T.ToString()), Bank.Count(S, JamBarks::SpeakerOpponent, T) >= 12); ++Combos; }
		for (const FName& T : { JamBarks::ToneEncourage, JamBarks::TonePraise, JamBarks::ToneComplain }) { TestTrue(*FString::Printf(TEXT("%s/teammate/%s has >= 12 lines"), *S.ToString(), *T.ToString()), Bank.Count(S, JamBarks::SpeakerTeammate, T) >= 12); ++Combos; }
	}
	for (const FName& S : MateOnly)
	{
		for (const FName& T : { JamBarks::ToneEncourage, JamBarks::TonePraise, JamBarks::ToneComplain }) { TestTrue(*FString::Printf(TEXT("%s/teammate/%s has >= 12 lines"), *S.ToString(), *T.ToString()), Bank.Count(S, JamBarks::SpeakerTeammate, T) >= 12); ++Combos; }
	}
	AddInfo(FString::Printf(TEXT("bank: %d lines, %d combinations checked"), Bank.Lines.Num(), Combos));
	return true;
}

// Events map to the expected situation and speaker side; the speaker is the nearest skater of that side; the human never speaks.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBarkPicksBySituation, "Jam.Barks.PicksBySituation", JamBarkTests::Flags)
bool FJamBarkPicksBySituation::RunTest(const FString&)
{
	FJamBarkBank Bank;
	TArray<FString> Errors;
	TestTrue(TEXT("test bank parses"), Bank.Parse(TestBankJson(), Errors));
	for (const FString& E : Errors) AddError(E);
	const FJamBarkWorld W = World();
	FJamBarkResolved R;

	// Human scores at +x (team 0 attacks +x): a teammate praises.
	TestTrue(TEXT("goal by human resolves"), FJamBarkPicker::Resolve(JamEvents::Goal, Payload(0, FVector2D(56, 0)), FVector2D(56, 0), W, R));
	TestEqual(TEXT("goal by human -> goal_for"), R.Situation, JamBarks::GoalFor);
	TestTrue(TEXT("goal by human -> teammate speaks"), R.Side == EJamBarkSpeaker::Teammate && W.IsTeammate(R.Speaker));
	// Bot teammate scores: the opponents complain.
	TestTrue(TEXT("goal by bot teammate resolves"), FJamBarkPicker::Resolve(JamEvents::Goal, Payload(1, FVector2D(56, 0)), FVector2D(56, 0), W, R));
	TestTrue(TEXT("goal by bot teammate -> goal_for, opponent speaks"), R.Situation == JamBarks::GoalFor && R.Side == EJamBarkSpeaker::Opponent && W.IsOpponent(R.Speaker));
	// Opponent 4 scores at -x: goal_against, the scorer speaks.
	TestTrue(TEXT("goal against resolves"), FJamBarkPicker::Resolve(JamEvents::Goal, Payload(4, FVector2D(-56, 0)), FVector2D(-56, 0), W, R));
	TestTrue(TEXT("goal against -> goal_against by the scorer"), R.Situation == JamBarks::GoalAgainst && R.Speaker == 4);
	// Checks: the human checks -> check_landed (nearest opponent to the contact point: index 3 at (10,0)); the human knocked down -> check_received.
	TestTrue(TEXT("human check resolves"), FJamBarkPicker::Resolve(JamEvents::Check, Payload(0, FVector2D(9, 0)), FVector2D(9, 0), W, R));
	TestTrue(TEXT("human check -> check_landed, nearest opponent 3"), R.Situation == JamBarks::CheckLanded && R.Speaker == 3);
	TestFalse(TEXT("a bot's check on a bot says nothing"), FJamBarkPicker::Resolve(JamEvents::Check, Payload(4, FVector2D(9, 0)), FVector2D(9, 0), W, R));
	FJamEventPayload Kd = Payload(0, FVector2D(-10, 0)); Kd.Kind = TEXT("check");
	TestTrue(TEXT("human knockdown resolves"), FJamBarkPicker::Resolve(JamEvents::Knockdown, Kd, FVector2D(-10, 0), W, R));
	TestTrue(TEXT("human knockdown -> check_received, an opponent"), R.Situation == JamBarks::CheckReceived && W.IsOpponent(R.Speaker));
	// Win: teammates; lose: opponents; faceoff: opponent; directors_call: nearest opponent to the human.
	TestTrue(TEXT("win"), FJamBarkPicker::Resolve(JamEvents::Win, Payload(-1, FVector2D::ZeroVector), FVector2D::ZeroVector, W, R) && R.Situation == JamBarks::Win && R.Side == EJamBarkSpeaker::Teammate);
	TestTrue(TEXT("lose"), FJamBarkPicker::Resolve(JamEvents::Lose, Payload(-1, FVector2D::ZeroVector), FVector2D::ZeroVector, W, R) && R.Situation == JamBarks::Lose && R.Side == EJamBarkSpeaker::Opponent);
	TestTrue(TEXT("faceoff"), FJamBarkPicker::Resolve(JamEvents::Faceoff, Payload(-1, FVector2D::ZeroVector), FVector2D::ZeroVector, W, R) && R.Situation == JamBarks::Faceoff);
	FJamEventPayload Dc; Dc.Kind = TEXT("skater_speed"); Dc.Strength = 0.5f;
	TestTrue(TEXT("directors_call"), FJamBarkPicker::Resolve(JamEvents::DirectorsCall, Dc, FVector2D::ZeroVector, W, R) && R.Situation == JamBarks::DirectorsCall && R.Speaker == 3);
	// Shots, boards, pickups and periods say nothing on their own.
	TestFalse(TEXT("shot"), FJamBarkPicker::Resolve(JamEvents::Shot, Payload(0, FVector2D::ZeroVector), FVector2D::ZeroVector, W, R));
	TestFalse(TEXT("board"), FJamBarkPicker::Resolve(JamEvents::Board, Payload(-1, FVector2D::ZeroVector), FVector2D::ZeroVector, W, R));
	TestFalse(TEXT("pickup_take"), FJamBarkPicker::Resolve(JamEvents::PickupTake, Payload(0, FVector2D::ZeroVector), FVector2D::ZeroVector, W, R));
	TestFalse(TEXT("period"), FJamBarkPicker::Resolve(JamEvents::Period, Payload(-1, FVector2D::ZeroVector), FVector2D::ZeroVector, W, R));

	// Picks: the tone follows the side and the intensity; the line comes from the right combination; quiet opponents never mock.
	FJamBarkPicker Picker(Bank, 7);
	FJamBarkPick P;
	TestTrue(TEXT("pick goal_against for opponent 4"), Picker.Pick(JamBarks::GoalAgainst, EJamBarkSpeaker::Opponent, 4, 0, P));
	TestTrue(TEXT("picked line is an opponent goal_against line"), P.Text.StartsWith(TEXT("opp goal_against")) && P.Speaker == 4 && P.Situation == JamBarks::GoalAgainst);
	TestTrue(TEXT("tone is an opponent tone"), P.Tone == JamBarks::ToneMock || P.Tone == JamBarks::ToneComplain || P.Tone == JamBarks::ToneBravado);
	FJamBarkSettings Quiet; Quiet.Intensity = JamBarks::IntensityQuiet;
	FJamBarkPicker QuietPicker(Bank, 3);
	QuietPicker.SetSettings(Quiet);
	for (int32 i = 0; i < 30; ++i)
	{
		FJamBarkPick Q;
		if (QuietPicker.Pick(JamBarks::Faceoff, EJamBarkSpeaker::Opponent, 3 + (i % 3), i * 2000, Q)) TestNotEqual(TEXT("quiet opponents never mock"), Q.Tone, JamBarks::ToneMock);
	}
	FJamBarkSettings Off; Off.bEnabled = false;
	FJamBarkPicker OffPicker(Bank, 3);
	OffPicker.SetSettings(Off);
	TestFalse(TEXT("disabled barks never pick"), OffPicker.Pick(JamBarks::Faceoff, EJamBarkSpeaker::Opponent, 3, 0, P));
	// Teammate-only situations with a forced tone (what the signal system will do).
	FJamBarkPicker Ack(Bank, 1);
	TestTrue(TEXT("signal ack picks a teammate encourage line"), Ack.PickWithTone(JamBarks::SignalAckDefend, EJamBarkSpeaker::Teammate, JamBarks::ToneEncourage, 1, 0, P) && P.Text.StartsWith(TEXT("ack signal_ack_defend")));
	TestEqual(TEXT("intent names round-trip"), FJamTeamIntent::FromName(FJamTeamIntent::NameOf(EJamTeamIntent::Space)), EJamTeamIntent::Space);
	// Settings from a cartridge: absent = enabled normal; explicit values are read; cooldown scale follows the intensity.
	const FJamBarkSettings Def = FJamBarkSettings::FromCartridgeJson(TEXT("{\"game\":{\"rink\":{}}}"));
	TestTrue(TEXT("absent game.barks = enabled normal"), Def.bEnabled && Def.Intensity == JamBarks::IntensityNormal && Def.CooldownScale() == 1.f);
	const FJamBarkSettings Loud = FJamBarkSettings::FromCartridgeJson(TEXT("{\"game\":{\"barks\":{\"enabled\":true,\"intensity\":\"loud\"}}}"));
	TestTrue(TEXT("loud read"), Loud.bEnabled && Loud.Intensity == JamBarks::IntensityLoud && Loud.CooldownScale() == 0.5f);
	const FJamBarkSettings Disabled = FJamBarkSettings::FromCartridgeJson(TEXT("{\"game\":{\"barks\":{\"enabled\":false}}}"));
	TestFalse(TEXT("enabled false read"), Disabled.bEnabled);
	return true;
}

// Global cooldown 240 frames, per-speaker 480 (normal), scaled by the intensity; no line repeats within 10 picks; the bubble fades by frames.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBarkCooldown, "Jam.Barks.Cooldown", JamBarkTests::Flags)
bool FJamBarkCooldown::RunTest(const FString&)
{
	FJamBarkBank Bank;
	TArray<FString> Errors;
	TestTrue(TEXT("test bank parses"), Bank.Parse(TestBankJson(), Errors));
	FJamBarkPicker Picker(Bank, 11);
	FJamBarkPick P;
	TestTrue(TEXT("first pick at frame 0"), Picker.Pick(JamBarks::Faceoff, EJamBarkSpeaker::Opponent, 3, 0, P));
	TestFalse(TEXT("another speaker at frame 100 is inside the global cooldown"), Picker.Pick(JamBarks::Faceoff, EJamBarkSpeaker::Opponent, 4, 100, P));
	TestFalse(TEXT("frame 239 still inside"), Picker.Pick(JamBarks::Faceoff, EJamBarkSpeaker::Opponent, 4, 239, P));
	TestTrue(TEXT("another speaker at frame 240 passes"), Picker.Pick(JamBarks::Faceoff, EJamBarkSpeaker::Opponent, 4, 240, P));
	TestFalse(TEXT("speaker 3 at frame 479 is inside its own 480-frame cooldown"), Picker.Pick(JamBarks::Faceoff, EJamBarkSpeaker::Opponent, 3, 479, P));
	// Frame 480: speaker 3's own 480 frames and the global 240 since frame 240 have both run out exactly.
	TestTrue(TEXT("speaker 3 at frame 480 passes"), Picker.Pick(JamBarks::Faceoff, EJamBarkSpeaker::Opponent, 3, 480, P));
	TestFalse(TEXT("speaker 5 at frame 480 is inside the global cooldown just set"), Picker.Pick(JamBarks::Faceoff, EJamBarkSpeaker::Opponent, 5, 480, P));
	TestTrue(TEXT("speaker 5 at frame 720 passes"), Picker.Pick(JamBarks::Faceoff, EJamBarkSpeaker::Opponent, 5, 720, P));
	TestFalse(TEXT("speaker 3 at frame 900 is inside its own cooldown from 480"), Picker.Pick(JamBarks::Faceoff, EJamBarkSpeaker::Opponent, 3, 900, P));
	TestTrue(TEXT("speaker 3 at frame 960 passes"), Picker.Pick(JamBarks::Faceoff, EJamBarkSpeaker::Opponent, 3, 960, P));

	// Loud halves the cooldowns, quiet doubles them.
	FJamBarkPicker LoudPicker(Bank, 2);
	FJamBarkSettings Loud; Loud.Intensity = JamBarks::IntensityLoud;
	LoudPicker.SetSettings(Loud);
	TestTrue(TEXT("loud first"), LoudPicker.Pick(JamBarks::Faceoff, EJamBarkSpeaker::Opponent, 3, 0, P));
	TestTrue(TEXT("loud: another speaker after 120 frames"), LoudPicker.Pick(JamBarks::Faceoff, EJamBarkSpeaker::Opponent, 4, 120, P));
	FJamBarkPicker QuietPicker(Bank, 2);
	FJamBarkSettings Quiet; Quiet.Intensity = JamBarks::IntensityQuiet;
	QuietPicker.SetSettings(Quiet);
	TestTrue(TEXT("quiet first"), QuietPicker.Pick(JamBarks::Faceoff, EJamBarkSpeaker::Opponent, 3, 0, P));
	TestFalse(TEXT("quiet: another speaker at 300 frames is still inside the doubled cooldown"), QuietPicker.Pick(JamBarks::Faceoff, EJamBarkSpeaker::Opponent, 4, 300, P));
	TestTrue(TEXT("quiet: another speaker at 480 frames passes"), QuietPicker.Pick(JamBarks::Faceoff, EJamBarkSpeaker::Opponent, 4, 480, P));

	// No repeats within 10 picks: 12 lines in the combination, 10 consecutive picks (one tone forced) are all different.
	FJamBarkPicker Rep(Bank, 5);
	TSet<FName> Seen;
	for (int32 i = 0; i < 10; ++i)
	{
		FJamBarkPick Q;
		TestTrue(TEXT("repeat pick"), Rep.PickWithTone(JamBarks::Lose, EJamBarkSpeaker::Opponent, JamBarks::ToneMock, 3 + (i % 3), i * 1000, Q));
		TestFalse(*FString::Printf(TEXT("pick %d repeats %s"), i, *Q.LineId.ToString()), Seen.Contains(Q.LineId));
		Seen.Add(Q.LineId);
	}
	TestEqual(TEXT("history holds the window"), Rep.GetHistory().Num(), 10);

	// Bubble timing: full for 150 frames, gone after 240.
	TestEqual(TEXT("opacity at show"), UJamBarkWidget::OpacityFor(100, 100), 1.f);
	TestEqual(TEXT("opacity at hold end"), UJamBarkWidget::OpacityFor(100, 250), 1.f);
	TestTrue(TEXT("opacity mid-fade"), FMath::IsNearlyEqual(UJamBarkWidget::OpacityFor(100, 295), 0.5f, 0.02f));
	TestEqual(TEXT("opacity after fade"), UJamBarkWidget::OpacityFor(100, 340), 0.f);
	TestEqual(TEXT("never shown"), UJamBarkWidget::OpacityFor(-1000000, 5), 0.f);
	return true;
}

#endif
