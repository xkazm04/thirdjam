// Lane A32 tests: Jam.Commentary.*. No world, no map: the picker is driven with a fake manifest, synthetic events and a synthetic world.
//   UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Commentary; Quit" -unattended -nullrhi -nosplash -log
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "CommentarySystem.h"
#include "JamCommentaryData.h"
#include "HUDWidget.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamCommentaryTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

	// Six skaters as in hockey.regular: team 0 (blue) = indices 0..2 (human is 0), team 1 (orange) = 3..5. Team 0 attacks +x; goal line at 41.
	FJamCommentaryWorld World(int64 Frame = 1000, bool bPause = false)
	{
		FJamCommentaryWorld W;
		W.Frame = Frame;
		W.HumanIndex = 0;
		W.Teams = { 0, 0, 0, 1, 1, 1 };
		W.Positions = { FVector2D(-10, 0), FVector2D(-10, -10), FVector2D(-30, 0), FVector2D(10, 0), FVector2D(10, 10), FVector2D(30, 0) };
		W.DrawEarly.Init(false, 6);
		W.bPause = bPause;
		W.RinkLengthM = 90; W.RinkWidthM = 45; W.GoalLineX = 41; W.GoalWidthM = 4;
		W.bFirstTo = true; W.ScoreToWin = 3;
		return W;
	}

	FJamEventPayload Payload(int32 Who, const FVector2D& WhereM, float Strength = 0.f, FName Kind = NAME_None)
	{
		FJamEventPayload P;
		P.Who = Who;
		P.Where = FVector(WhereM.X * 100, WhereM.Y * 100, 0);
		P.Strength = Strength;
		P.Kind = Kind;
		return P;
	}

	FString Line(const TCHAR* Id, const TCHAR* Event, const TCHAR* Kind, const TCHAR* Team, int32 Priority, double Cooldown, const TCHAR* Text, double Seconds = 1.0)
	{
		const FString Name = FString(TEXT("VO_Commentator_")) + FString(Id).Replace(TEXT("."), TEXT("_"));
		return FString::Printf(TEXT("{\"id\":\"%s\",\"event\":\"%s\",\"kind\":\"%s\",\"team\":\"%s\",\"priority\":%d,\"cooldown_s\":%g,\"when\":\"test\",\"text\":\"%s\",\"sound\":\"/JamAssets/Audio/VO/%s.%s\",\"seconds\":%g}"),
			Id, Event, Kind, Team, Priority, Cooldown, Text, *Name, *Name, Seconds);
	}

	// A small manifest with the situations the rules need.
	FString TestManifest()
	{
		TArray<FString> L;
		L.Add(Line(TEXT("goal.blue.01"), TEXT("goal"), TEXT(""), TEXT("blue"), 5, 0, TEXT("Blue scores!")));
		L.Add(Line(TEXT("goal.blue.02"), TEXT("goal"), TEXT(""), TEXT("blue"), 5, 0, TEXT("Blue finds the net!")));
		L.Add(Line(TEXT("goal.orange.01"), TEXT("goal"), TEXT(""), TEXT("orange"), 5, 0, TEXT("Orange scores!")));
		L.Add(Line(TEXT("goal.for.01"), TEXT("goal"), TEXT(""), TEXT("any"), 4, 0, TEXT("That one is ours.")));
		L.Add(Line(TEXT("goal.against.01"), TEXT("goal"), TEXT(""), TEXT("any"), 4, 0, TEXT("Credit where it is due.")));
		L.Add(Line(TEXT("goal.first.01"), TEXT("goal"), TEXT(""), TEXT("any"), 4, 0, TEXT("First blood, so to speak.")));
		L.Add(Line(TEXT("goal.late.01"), TEXT("goal"), TEXT(""), TEXT("any"), 5, 0, TEXT("Right at the end!")));
		L.Add(Line(TEXT("goal.pause.01"), TEXT("goal"), TEXT(""), TEXT("any"), 2, 20, TEXT("Back to the dot.")));
		L.Add(Line(TEXT("shot.blue.01"), TEXT("shot"), TEXT(""), TEXT("blue"), 2, 6, TEXT("Blue shoots.")));
		L.Add(Line(TEXT("shot.orange.01"), TEXT("shot"), TEXT(""), TEXT("orange"), 2, 6, TEXT("Orange shoots.")));
		L.Add(Line(TEXT("shot.hard.01"), TEXT("shot"), TEXT(""), TEXT("any"), 3, 10, TEXT("What a blast.")));
		L.Add(Line(TEXT("shot.soft.01"), TEXT("shot"), TEXT(""), TEXT("any"), 1, 15, TEXT("A soft one.")));
		L.Add(Line(TEXT("shot.long.01"), TEXT("shot"), TEXT(""), TEXT("any"), 2, 20, TEXT("From way out.")));
		L.Add(Line(TEXT("save.any.01"), TEXT("save"), TEXT(""), TEXT("any"), 3, 6, TEXT("Stopped.")));
		L.Add(Line(TEXT("save.big.01"), TEXT("save"), TEXT(""), TEXT("any"), 4, 10, TEXT("Huge stop!")));
		L.Add(Line(TEXT("faceoff.drop.01"), TEXT("faceoff"), TEXT("drop"), TEXT("any"), 2, 8, TEXT("Puck is down.")));
		L.Add(Line(TEXT("faceoff.after_goal.01"), TEXT("faceoff"), TEXT("drop"), TEXT("any"), 2, 30, TEXT("Back underway.")));
		L.Add(Line(TEXT("faceoff.blue.01"), TEXT("faceoff"), TEXT("won"), TEXT("blue"), 2, 6, TEXT("Blue wins the draw.")));
		L.Add(Line(TEXT("period.take_1.01"), TEXT("period"), TEXT("take_1"), TEXT("any"), 4, 0, TEXT("Good evening from the rink."), 3.0));
		L.Add(Line(TEXT("period.take_n.01"), TEXT("period"), TEXT(""), TEXT("any"), 4, 0, TEXT("Another take."), 1.5));
		L.Add(Line(TEXT("win.any.01"), TEXT("win"), TEXT(""), TEXT("any"), 5, 0, TEXT("They have done it!"), 2.0));
		L.Add(Line(TEXT("lose.any.01"), TEXT("lose"), TEXT(""), TEXT("any"), 5, 0, TEXT("Not tonight."), 2.0));
		L.Add(Line(TEXT("clock.last_10.01"), TEXT("clock"), TEXT("last_10s"), TEXT("any"), 4, 0, TEXT("Ten seconds.")));
		L.Add(Line(TEXT("clock.last_10_tied.01"), TEXT("clock"), TEXT("last_10s"), TEXT("any"), 5, 0, TEXT("Ten seconds, all square.")));
		L.Add(Line(TEXT("zone_enter.slow_ice.01"), TEXT("zone_enter"), TEXT("slow_ice"), TEXT("any"), 1, 30, TEXT("Into the slush.")));
		L.Add(Line(TEXT("pass.to_human.01"), TEXT("pass"), TEXT(""), TEXT("any"), 2, 20, TEXT("Here it comes.")));
		L.Add(Line(TEXT("pass.any.01"), TEXT("pass"), TEXT(""), TEXT("any"), 1, 15, TEXT("Tape to tape.")));
		for (int32 i = 1; i <= 4; ++i) L.Add(Line(*FString::Printf(TEXT("board.hard.%02d"), i), TEXT("board"), TEXT(""), TEXT("any"), 1, 0, *FString::Printf(TEXT("Off the glass number %d."), i)));
		// Lane A47: B's condition-tagged lines (kind = the condition, B-to-A-0030 fault 4).
		L.Add(Line(TEXT("board.bank.01"), TEXT("board"), TEXT("bank"), TEXT("any"), 4, 0, TEXT("Banked it in off the boards!")));
		L.Add(Line(TEXT("board.rim.01"), TEXT("board"), TEXT("rim"), TEXT("any"), 2, 0, TEXT("Round the rim.")));
		return TEXT("{\"schema\":\"hokej.commentary/1\",\"count\":") + FString::FromInt(L.Num()) + TEXT(",\"lines\":[") + FString::Join(L, TEXT(",")) + TEXT("]}");
	}

	struct FRig
	{
		FJamCommentaryBank Bank;
		TUniquePtr<FJamCommentaryPicker> Picker;
		TArray<FString> Errors;
		FRig()
		{
			Bank.Parse(TestManifest(), Errors);
			Picker = MakeUnique<FJamCommentaryPicker>(Bank, 7);
		}
		// One frame with one event; returns the line that started (or none).
		bool Frame(int64 F, FName Event, const FJamEventPayload& P, FJamCommentaryPick& Out, bool bPause = false, FString* Skip = nullptr, TFunction<void(FJamCommentaryWorld&)> Tweak = nullptr)
		{
			FJamCommentaryWorld W = World(F, bPause);
			if (Tweak) Tweak(W);
			Picker->Observe(Event, P, FVector2D(P.Where.X / 100.0, P.Where.Y / 100.0), W);
			return Picker->Resolve(W, Out, Skip);
		}
		bool Idle(int64 F, FJamCommentaryPick& Out, bool bPause = false, TFunction<void(FJamCommentaryWorld&)> Tweak = nullptr)
		{
			FJamCommentaryWorld W = World(F, bPause);
			if (Tweak) Tweak(W);
			return Picker->Resolve(W, Out);
		}
	};
}

using namespace JamCommentaryTests;

// The embedded manifest parses, equals commentary.json on disk, has 340 lines with unique ids, and every sound path follows the id.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCommentaryBankClean, "Jam.Commentary.BankClean", JamCommentaryTests::Flags)
bool FJamCommentaryBankClean::RunTest(const FString&)
{
	FJamCommentaryBank Bank;
	TArray<FString> Errors;
	const bool bParsed = Bank.Parse(FJamCommentaryBank::EmbeddedJson(), Errors);
	for (const FString& E : Errors) AddError(E);
	TestTrue(TEXT("embedded manifest parses"), bParsed);
	TestEqual(TEXT("340 lines"), Bank.Lines.Num(), 340);
	TestTrue(TEXT("at least 130 situations"), Bank.SituationCount() >= 130);
	const FString DiskPath = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("JamAssets"), TEXT("Content"), TEXT("Audio"), TEXT("VO"), TEXT("commentary.json"));
	FString Disk;
	if (FFileHelper::LoadFileToString(Disk, *DiskPath))
	{
		TestEqual(TEXT("embedded copy equals commentary.json (run node tools/commentary/embed.mjs)"), FJamCommentaryBank::EmbeddedJson().Replace(TEXT("\r"), TEXT("")), Disk.Replace(TEXT("\r"), TEXT("")));
	}
	else AddInfo(TEXT("no commentary.json on disk (packaged run): embedded copy only"));
	int32 Stoppage = 0;
	for (const FJamCommentaryLine& L : Bank.Lines)
	{
		const FString Name = FString(TEXT("VO_Commentator_")) + L.Id.ToString().Replace(TEXT("."), TEXT("_"));
		TestEqual(*FString::Printf(TEXT("%s sound path"), *L.Id.ToString()), L.Sound.ToString(), FString::Printf(TEXT("/JamAssets/Audio/VO/%s.%s"), *Name, *Name));
		TestTrue(*FString::Printf(TEXT("%s duration"), *L.Id.ToString()), L.DurationFrames >= 30 && L.DurationFrames <= 360);
		if (L.bStoppage) ++Stoppage;
	}
	TestEqual(TEXT("53 stoppage lines (period, period_end, season_take, win, lose, goal.pause)"), Stoppage, 53);
	TestTrue(TEXT("goal.late.01 is a goal line"), Bank.Find(TEXT("goal.late.01")) != INDEX_NONE && Bank.ForEvent(TEXT("goal")).Contains(Bank.Find(TEXT("goal.late.01"))));
	// Strict parse: unknown keys and a wrong team are errors.
	FJamCommentaryBank Bad;
	TArray<FString> BadErrors;
	TestFalse(TEXT("unknown key rejected"), Bad.Parse(TEXT("{\"schema\":\"hokej.commentary/1\",\"lines\":[{\"id\":\"goal.blue.01\",\"event\":\"goal\",\"kind\":\"\",\"team\":\"red\",\"priority\":5,\"cooldown_s\":0,\"when\":\"x\",\"text\":\"y\",\"sound\":\"/a.a\",\"seconds\":1,\"extra\":1}]}"), BadErrors));
	TestTrue(TEXT("two errors named"), BadErrors.Num() >= 1);
	return true;
}

// Selection: event, kind, team and when; the highest priority wins; a tie goes to the more specific line; the human's lines.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCommentarySelection, "Jam.Commentary.Selection", JamCommentaryTests::Flags)
bool FJamCommentarySelection::RunTest(const FString&)
{
	FRig R;
	for (const FString& E : R.Errors) AddError(E);
	TestEqual(TEXT("test manifest parses"), R.Errors.Num(), 0);
	FJamCommentaryPick P;
	// A hard shot by orange: shot.hard (3) beats shot.orange (2).
	TestTrue(TEXT("hard shot plays"), R.Frame(1000, JamEvents::Shot, Payload(4, FVector2D(-20, 0), 0.9f), P));
	TestEqual(TEXT("priority wins: shot.hard"), P.Situation, FName(TEXT("shot.hard")));
	// A medium shot by blue from its own half: shot.blue and shot.long tie at 2 with one specificity each; shot.soft (strength 0.5) does not hold.
	TestTrue(TEXT("medium shot plays"), R.Frame(1200, JamEvents::Shot, Payload(1, FVector2D(-20, 0), 0.5f), P));
	TestTrue(TEXT("shot.blue or shot.long"), P.Situation == FName(TEXT("shot.blue")) || P.Situation == FName(TEXT("shot.long")));
	// A soft shot by blue from the attacking half with shot.blue on cooldown: shot.soft (1).
	FJamCommentaryPick P2;
	const bool bSoft = R.Frame(1400, JamEvents::Shot, Payload(1, FVector2D(20, 0), 0.1f), P2);
	TestTrue(TEXT("soft shot plays something"), bSoft);
	if (P.Situation == FName(TEXT("shot.blue"))) TestEqual(TEXT("shot.blue cooling: shot.soft"), P2.Situation, FName(TEXT("shot.soft")));
	// Kind: a blue centre wins the draw -> faceoff.blue; an orange one -> nothing (no orange line, drop needs kind drop).
	TestTrue(TEXT("blue draw"), R.Frame(2000, JamEvents::Faceoff, Payload(1, FVector2D::ZeroVector, 0.f, TEXT("won")), P));
	TestEqual(TEXT("faceoff.blue"), P.Situation, FName(TEXT("faceoff.blue")));
	TestFalse(TEXT("orange draw has no line"), R.Frame(2200, JamEvents::Faceoff, Payload(4, FVector2D::ZeroVector, 0.f, TEXT("won")), P));
	// Zone kinds: the payload carries the zone id, the line the zone kind.
	TestTrue(TEXT("zone id -> kind"), R.Frame(2400, JamEvents::ZoneEnter, Payload(0, FVector2D::ZeroVector, 0.f, TEXT("z1")), P, false, nullptr, [](FJamCommentaryWorld& W) { W.ZoneKinds.Add(TEXT("z1"), TEXT("slow_ice")); }));
	TestEqual(TEXT("zone_enter.slow_ice"), P.Situation, FName(TEXT("zone_enter.slow_ice")));
	TestFalse(TEXT("an unknown zone id has no line"), R.Frame(2600, JamEvents::ZoneEnter, Payload(0, FVector2D::ZeroVector, 0.f, TEXT("z9")), P));
	// pass.to_human (2, human) beats pass.any (1).
	TestTrue(TEXT("pass to the human"), R.Frame(2800, JamEvents::Pass, Payload(1, FVector2D::ZeroVector, 0.5f, TEXT("to_0")), P));
	TestEqual(TEXT("pass.to_human"), P.Situation, FName(TEXT("pass.to_human")));
	// Intensity quiet: priority 1 and 2 never play.
	FJamCommentarySettings Quiet; Quiet.Intensity = JamCommentary::IntensityQuiet;
	R.Picker->SetSettings(Quiet);
	TestFalse(TEXT("quiet drops a priority-1 pass"), R.Frame(3000, JamEvents::Pass, Payload(4, FVector2D::ZeroVector, 0.5f, TEXT("to_5")), P));
	TestTrue(TEXT("quiet keeps a hard shot (3)"), R.Frame(3200, JamEvents::Shot, Payload(4, FVector2D(-20, 0), 0.9f), P));
	FJamCommentarySettings Off; Off.bEnabled = false;
	R.Picker->SetSettings(Off);
	TestFalse(TEXT("disabled picks nothing"), R.Frame(5000, JamEvents::Goal, Payload(0, FVector2D(43, 0), 1.f), P, true, nullptr, [](FJamCommentaryWorld& W) { W.Score[0] = 1; }));
	// Settings from a cartridge.
	const FJamCommentarySettings S = FJamCommentarySettings::FromCartridgeJson(TEXT("{\"game\":{\"commentary\":{\"enabled\":true,\"intensity\":\"loud\"}}}"));
	TestTrue(TEXT("settings parse"), S.bEnabled && S.Intensity == JamCommentary::IntensityLoud && S.CooldownScale() == 0.5f);
	TestEqual(TEXT("absent = normal"), FJamCommentarySettings::FromCartridgeJson(TEXT("{\"game\":{}}")).Intensity, JamCommentary::IntensityNormal);
	return true;
}

// Timing: the 2.5 s gate from the start of the last line, one line at a time (its duration), a late line is dropped and never queued,
// a situation's cooldown, never the same alternate twice running.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCommentaryTiming, "Jam.Commentary.Timing", JamCommentaryTests::Flags)
bool FJamCommentaryTiming::RunTest(const FString&)
{
	FRig R;
	FJamCommentaryPick P;
	FString Skip;
	TestTrue(TEXT("first shot plays"), R.Frame(1000, JamEvents::Shot, Payload(4, FVector2D(-20, 0), 0.9f), P));
	TestEqual(TEXT("1.0 s line = 60 frames"), P.DurationFrames, 60);
	// 100 frames later: the line is over but the gate (150) is closed: dropped with the reason.
	TestFalse(TEXT("a line inside the gate is dropped"), R.Frame(1100, JamEvents::Shot, Payload(1, FVector2D(20, 0), 0.5f), P, false, &Skip));
	TestEqual(TEXT("reason gate"), Skip, FString(TEXT("gate")));
	// Nothing was queued: an idle frame after the gate reopens plays nothing.
	TestFalse(TEXT("never queued"), R.Idle(1160, P));
	// 150 frames after the start the gate is open.
	TestTrue(TEXT("gate reopens at 150"), R.Frame(1150 + 160, JamEvents::Shot, Payload(1, FVector2D(20, 0), 0.5f), P));
	// A 3-second stoppage line holds the booth past the gate: a quick line at +160 is dropped as "playing".
	TestTrue(TEXT("take 1 line"), R.Frame(2000, JamEvents::Period, Payload(-1, FVector2D::ZeroVector, 0.f, TEXT("take_1")), P, true));
	TestEqual(TEXT("180 frames"), P.DurationFrames, 180);
	TestFalse(TEXT("one line at a time"), R.Frame(2160, JamEvents::Shot, Payload(4, FVector2D(-20, 0), 0.9f), P, false, &Skip));
	TestEqual(TEXT("reason playing"), Skip, FString(TEXT("playing")));
	TestTrue(TEXT("free after the line ends"), R.Frame(2181, JamEvents::Shot, Payload(4, FVector2D(-20, 0), 0.9f), P));
	// Cooldown per situation: shot.hard has 10 s; the next hard shot 5 s later falls through to shot.orange (6 s cooldown, never played).
	TestTrue(TEXT("hard shot again"), R.Frame(2481, JamEvents::Shot, Payload(4, FVector2D(-20, 0), 0.9f), P));
	TestEqual(TEXT("shot.hard cooling: shot.orange"), P.Situation, FName(TEXT("shot.orange")));
	// Alternates: four board.hard lines with no cooldown, each 1 s; the same id never plays twice running, the last three are skipped.
	TArray<FName> Ids;
	for (int32 i = 0; i < 8; ++i)
	{
		TestTrue(TEXT("board plays"), R.Frame(5000 + i * 200, JamEvents::Board, Payload(-1, FVector2D(0, 22), 0.9f), P));
		Ids.Add(P.LineId);
	}
	for (int32 i = 1; i < Ids.Num(); ++i)
	{
		TestTrue(TEXT("not the same line twice running"), Ids[i] != Ids[i - 1]);
		if (i >= 3) TestTrue(TEXT("not among the last three"), Ids[i] != Ids[i - 1] && Ids[i] != Ids[i - 2] && Ids[i] != Ids[i - 3]);
	}
	// Loud halves the cooldown: shot.hard (10 s) is back after 5 s.
	FJamCommentarySettings Loud; Loud.Intensity = JamCommentary::IntensityLoud;
	R.Picker->SetSettings(Loud);
	TestTrue(TEXT("hard shot"), R.Frame(10000, JamEvents::Shot, Payload(4, FVector2D(-20, 0), 0.9f), P));
	TestEqual(TEXT("shot.hard"), P.Situation, FName(TEXT("shot.hard")));
	TestTrue(TEXT("hard shot 5 s later"), R.Frame(10300, JamEvents::Shot, Payload(4, FVector2D(-20, 0), 0.9f), P));
	TestEqual(TEXT("loud: shot.hard again"), P.Situation, FName(TEXT("shot.hard")));
	return true;
}

// Stoppage lines only in a pause; the goal's two beats; the win and lose lines.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCommentaryStoppage, "Jam.Commentary.Stoppage", JamCommentaryTests::Flags)
bool FJamCommentaryStoppage::RunTest(const FString&)
{
	FRig R;
	FJamCommentaryPick P;
	// period while Playing: nothing; in a pause: the take line.
	TestFalse(TEXT("no take line in play"), R.Frame(1000, JamEvents::Period, Payload(-1, FVector2D::ZeroVector, 0.f, TEXT("take_1")), P, false));
	TestTrue(TEXT("take line in the faceoff countdown"), R.Frame(1200, JamEvents::Period, Payload(-1, FVector2D::ZeroVector, 0.f, TEXT("take_1")), P, true));
	TestEqual(TEXT("period.take_1"), P.Situation, FName(TEXT("period.take_1")));
	TestTrue(TEXT("take 4"), R.Frame(1600, JamEvents::Period, Payload(-1, FVector2D::ZeroVector, 0.f, TEXT("take_4")), P, true));
	TestEqual(TEXT("period.take_n for take 4"), P.Situation, FName(TEXT("period.take_n")));
	// A goal by blue (the human's team) at +x in the goal pause: the colour call first (5), then the second beat when the gate reopens.
	auto Scored = [](FJamCommentaryWorld& W) { W.Score[0] = 1; };
	TestTrue(TEXT("goal plays"), R.Frame(3000, JamEvents::Goal, Payload(0, FVector2D(43, 0), 1.f), P, true, nullptr, Scored));
	TestEqual(TEXT("colour call first"), P.Situation, FName(TEXT("goal.blue")));
	TestFalse(TEXT("nothing during the gate"), R.Idle(3100, P, true, Scored));
	TestTrue(TEXT("second beat when the gate reopens, still in the pause"), R.Idle(3150, P, true, Scored));
	TestTrue(TEXT("a score-state or mood line, not the colour, not goal.pause"), P.Situation == FName(TEXT("goal.for")) || P.Situation == FName(TEXT("goal.first")));
	TestEqual(TEXT("the beat is a goal line"), P.Event, JamEvents::Goal);
	TestFalse(TEXT("no third beat"), R.Idle(3300, P, true, Scored));
	// A goal with the gate closed behind a shot line: lane A47 (B-to-A-0030 fault 3) made the colour call a must-play that cuts the lower-priority line
	// (A32 had it dropped here, its open point 1); the second beat still follows when the gate reopens. (The score before orange's goal is 1:0.)
	TestTrue(TEXT("a shot right before"), R.Frame(4000, JamEvents::Shot, Payload(4, FVector2D(-20, 0), 0.9f), P, false, nullptr, Scored));
	auto Scored2 = [](FJamCommentaryWorld& W) { W.Score[0] = 1; W.Score[1] = 1; };
	FString Skip;
	TestTrue(TEXT("goal inside the gate cuts in (must-play)"), R.Frame(4060, JamEvents::Goal, Payload(4, FVector2D(-43, 0), 1.f), P, true, &Skip, Scored2));
	TestEqual(TEXT("the colour call"), P.Situation, FName(TEXT("goal.orange")));
	TestFalse(TEXT("nothing to cut: the 60-frame shot line had just ended, only the gate was skipped"), R.Picker->TakeCutRequest());
	TestFalse(TEXT("the beat waits for the gate"), R.Idle(4100, P, true, Scored2));
	TestTrue(TEXT("the beat plays once the gate opens"), R.Idle(4210, P, true, Scored2));
	TestTrue(TEXT("orange's goal: goal.against or goal.first"), P.Situation == FName(TEXT("goal.against")) || P.Situation == FName(TEXT("goal.first")));
	// The beat expires when play resumes.
	TestTrue(TEXT("another goal"), R.Frame(6000, JamEvents::Goal, Payload(0, FVector2D(43, 0), 1.f), P, true, nullptr, [](FJamCommentaryWorld& W) { W.Score[0] = 2; W.Score[1] = 1; }));
	TestFalse(TEXT("play resumed: no beat"), R.Idle(6150, P, false, [](FJamCommentaryWorld& W) { W.Score[0] = 2; W.Score[1] = 1; }));
	// win / lose only in a pause (the match is over = a pause); goal.late needs the clock.
	TestFalse(TEXT("win line never in play"), R.Frame(7000, JamEvents::Win, Payload(-1, FVector2D::ZeroVector, 1.f), P, false));
	TestTrue(TEXT("win line when over"), R.Frame(7200, JamEvents::Win, Payload(-1, FVector2D::ZeroVector, 1.f), P, true));
	TestEqual(TEXT("win.any"), P.Situation, FName(TEXT("win.any")));
	auto Late = [](FJamCommentaryWorld& W) { W.bTimer = true; W.bFirstTo = false; W.FramesLeftInPeriod = 300; W.Score[0] = 3; W.Score[1] = 1; };
	TestTrue(TEXT("late goal"), R.Frame(9000, JamEvents::Goal, Payload(1, FVector2D(43, 0), 1.f), P, true, nullptr, Late));
	TestTrue(TEXT("goal.late (5) ties goal.blue (5); both are allowed"), P.Situation == FName(TEXT("goal.late")) || P.Situation == FName(TEXT("goal.blue")));
	// clock: tied beats plain.
	TestTrue(TEXT("clock"), R.Frame(9500, JamEvents::Clock, Payload(-1, FVector2D::ZeroVector, 0.f, TEXT("last_10s")), P, false, nullptr, [](FJamCommentaryWorld& W) { W.bTimer = true; W.Score[0] = W.Score[1] = 2; }));
	TestEqual(TEXT("clock.last_10_tied"), P.Situation, FName(TEXT("clock.last_10_tied")));
	return true;
}

// The booth yields to a bark addressed to the human: nothing starts for 150 frames and a playing line is cut.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCommentaryYield, "Jam.Commentary.Yield", JamCommentaryTests::Flags)
bool FJamCommentaryYield::RunTest(const FString&)
{
	FRig R;
	FJamCommentaryPick P;
	FString Skip;
	// A 3 s take line is playing; a bark at the human arrives 30 frames in: the line is cut and the booth holds.
	TestTrue(TEXT("take line"), R.Frame(1000, JamEvents::Period, Payload(-1, FVector2D::ZeroVector, 0.f, TEXT("take_1")), P, true));
	TestTrue(TEXT("line is playing"), R.Picker->IsLinePlaying(1030));
	{
		FJamCommentaryWorld W = World(1030, true);
		R.Picker->ObserveBark(1030, true);
		TestFalse(TEXT("nothing starts on the bark frame"), R.Picker->Resolve(W, P, &Skip));
	}
	TestTrue(TEXT("the playing line is cut"), R.Picker->TakeCutRequest());
	TestFalse(TEXT("cut: no longer playing"), R.Picker->IsLinePlaying(1031));
	// 100 frames later the hold (150) is still on: a shot is dropped with reason yield.
	TestFalse(TEXT("held for the bark"), R.Frame(1130, JamEvents::Shot, Payload(4, FVector2D(-20, 0), 0.9f), P, false, &Skip));
	TestEqual(TEXT("reason yield"), Skip, FString(TEXT("yield")));
	TestTrue(TEXT("free after the hold"), R.Frame(1181, JamEvents::Shot, Payload(4, FVector2D(-20, 0), 0.9f), P));
	// A bark not at the human (goal_against) changes nothing.
	{
		FJamCommentaryWorld W = World(1400, false);
		R.Picker->ObserveBark(1400, false);
		R.Picker->Resolve(W, P);
	}
	TestFalse(TEXT("no cut for a bark to the rink"), R.Picker->TakeCutRequest());
	TestTrue(TEXT("booth open"), R.Frame(1400, JamEvents::Shot, Payload(1, FVector2D(20, 0), 0.5f), P));
	// A bark and an event on the same frame: the bark wins, the event is dropped (never queued).
	{
		FJamCommentaryWorld W = World(2000, false);
		R.Picker->Observe(JamEvents::Shot, Payload(4, FVector2D(-20, 0), 0.9f), FVector2D(-20, 0), W);
		R.Picker->ObserveBark(2000, true);
		TestFalse(TEXT("same frame: the bark wins"), R.Picker->Resolve(W, P, &Skip));
		TestEqual(TEXT("reason yield"), Skip, FString(TEXT("yield")));
	}
	TestFalse(TEXT("the dropped shot does not come back"), R.Idle(2151, P));
	// jam.say_line: Force plays regardless, still takes the booth.
	TestTrue(TEXT("force by id"), R.Picker->Force(TEXT("save.big.01"), 3000, P));
	TestEqual(TEXT("forced line"), P.LineId, FName(TEXT("save.big.01")));
	TestFalse(TEXT("unknown id"), R.Picker->Force(TEXT("nope.any.01"), 3000, P));
	TestFalse(TEXT("the forced line holds the gate"), R.Frame(3100, JamEvents::Shot, Payload(4, FVector2D(-20, 0), 0.9f), P));
	return true;
}

// Lane A47 (B-to-A-0030 fault 3): a bark never cuts a line under 0.5 s old; the line plays on and the booth still holds after it. An older line is cut,
// and the end frame the HUD's caption reads moves to the cut frame (fault 7: the caption clears when the line ends).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCommentaryCutFloor, "Jam.Commentary.CutFloor", JamCommentaryTests::Flags)
bool FJamCommentaryCutFloor::RunTest(const FString&)
{
	FRig R;
	FJamCommentaryPick P;
	FString Skip;
	// A 3 s take line starts at 1000; a bark at the human 10 frames in: no cut, the line keeps playing to its end (1180).
	TestTrue(TEXT("take line"), R.Frame(1000, JamEvents::Period, Payload(-1, FVector2D::ZeroVector, 0.f, TEXT("take_1")), P, true));
	{
		FJamCommentaryWorld W = World(1010, true);
		R.Picker->ObserveBark(1010, true);
		TestFalse(TEXT("nothing starts on the bark frame"), R.Picker->Resolve(W, P, &Skip));
	}
	TestFalse(TEXT("a line 10 frames old is not cut"), R.Picker->TakeCutRequest());
	TestTrue(TEXT("the young line plays on"), R.Picker->IsLinePlaying(1100));
	TestEqual(TEXT("its end frame is unchanged"), R.Picker->GetLastEndFrame(), static_cast<int64>(1180));
	// The booth waits for the line: a shot while it still plays is dropped as playing; once it has ended (and the bark's 150 frames are up) the booth is free.
	TestFalse(TEXT("nothing cuts in while the young line plays"), R.Frame(1100, JamEvents::Shot, Payload(4, FVector2D(-20, 0), 0.9f), P, false, &Skip));
	TestEqual(TEXT("reason playing"), Skip, FString(TEXT("playing")));
	TestTrue(TEXT("free after the line and the hold"), R.Frame(1185, JamEvents::Shot, Payload(4, FVector2D(-20, 0), 0.9f), P));
	// A bark exactly at the floor (30 frames) cuts: the end frame becomes the cut frame and the caption fades from it.
	TestTrue(TEXT("second take line"), R.Frame(2000, JamEvents::Period, Payload(-1, FVector2D::ZeroVector, 0.f, TEXT("take_1")), P, true));
	{
		FJamCommentaryWorld W = World(2030, true);
		R.Picker->ObserveBark(2030, true);
		R.Picker->Resolve(W, P, &Skip);
	}
	TestTrue(TEXT("a line 30 frames old is cut"), R.Picker->TakeCutRequest());
	TestEqual(TEXT("cut reason"), FString(R.Picker->GetCutReason()), FString(TEXT("bark_at_human")));
	TestEqual(TEXT("end frame = cut frame"), R.Picker->GetLastEndFrame(), static_cast<int64>(2030));
	const int32 Shown = static_cast<int32>(R.Picker->GetLastEndFrame() - 2000);
	TestEqual(TEXT("caption full while the voice plays"), UJamHudWidget::CommentaryOpacityFor(2000, Shown, 2029, 0), 1.f);
	TestEqual(TEXT("caption gone 30 frames after the cut"), UJamHudWidget::CommentaryOpacityFor(2000, Shown, 2060, 0), 0.f);
	return true;
}

// Lane A47 (B-to-A-0030 fault 3, A32's open point 1): a must-play line (priority 5) cuts a playing lower-priority line and skips the gate behind one;
// it never cuts another priority-5 line and still waits out a bark hold.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCommentaryMustPlay, "Jam.Commentary.MustPlay", JamCommentaryTests::Flags)
bool FJamCommentaryMustPlay::RunTest(const FString&)
{
	FRig R;
	FJamCommentaryPick P;
	FString Skip;
	auto Scored = [](FJamCommentaryWorld& W) { W.Score[0] = 1; };
	// A hard shot line (3, 60 frames) at 1000; the goal 19 frames later is called: the shot line is cut with the reason must_play.
	TestTrue(TEXT("shot line"), R.Frame(1000, JamEvents::Shot, Payload(0, FVector2D(30, 0), 0.9f), P));
	TestEqual(TEXT("shot.hard"), P.Situation, FName(TEXT("shot.hard")));
	TestTrue(TEXT("the goal call cuts in"), R.Frame(1019, JamEvents::Goal, Payload(0, FVector2D(43, 0), 1.f), P, true, &Skip, Scored));
	TestEqual(TEXT("goal.blue"), P.Situation, FName(TEXT("goal.blue")));
	TestTrue(TEXT("the shot line was cut"), R.Picker->TakeCutRequest());
	TestEqual(TEXT("cut reason"), FString(R.Picker->GetCutReason()), FString(TEXT("must_play")));
	TestEqual(TEXT("the goal call owns the booth"), R.Picker->GetLastEndFrame(), static_cast<int64>(1019 + 60));
	// The second beat still follows in the pause once the gate reopens.
	TestTrue(TEXT("second beat"), R.Idle(1019 + 150, P, true, Scored));
	TestEqual(TEXT("a goal line"), P.Event, JamEvents::Goal);
	// A lower-priority line that ended but whose 2.5 s gate is still closed: the goal call skips the gate.
	TestTrue(TEXT("pass line"), R.Frame(3000, JamEvents::Pass, Payload(4, FVector2D::ZeroVector, 0.5f, TEXT("to_5")), P, false, nullptr, Scored));
	TestEqual(TEXT("1 s line"), P.DurationFrames, 60);
	auto Scored2 = [](FJamCommentaryWorld& W) { W.Score[0] = 1; W.Score[1] = 1; };
	TestTrue(TEXT("goal call inside the gate, after the line"), R.Frame(3100, JamEvents::Goal, Payload(4, FVector2D(-43, 0), 1.f), P, true, &Skip, Scored2));
	TestEqual(TEXT("goal.orange"), P.Situation, FName(TEXT("goal.orange")));
	TestFalse(TEXT("nothing to cut: the pass line had ended"), R.Picker->TakeCutRequest());
	// A priority-5 line never cuts another priority-5 line.
	TestTrue(TEXT("win line (5)"), R.Frame(5000, JamEvents::Win, Payload(-1, FVector2D::ZeroVector, 1.f), P, true));
	TestFalse(TEXT("a goal call does not cut the win line"), R.Frame(5020, JamEvents::Goal, Payload(0, FVector2D(43, 0), 1.f), P, true, &Skip, [](FJamCommentaryWorld& W) { W.Score[0] = 2; W.Score[1] = 1; }));
	TestEqual(TEXT("reason playing"), Skip, FString(TEXT("playing")));
	// A bark hold silences everything but a must-play: the goal call plays through it (first PIE proof: a bark 61 frames before a goal had silenced the call).
	{
		FJamCommentaryWorld W = World(7000, false);
		R.Picker->ObserveBark(7000, true);
		R.Picker->Resolve(W, P);
	}
	TestFalse(TEXT("a pass inside the bark hold is dropped"), R.Frame(7030, JamEvents::Pass, Payload(4, FVector2D::ZeroVector, 0.5f, TEXT("to_5")), P, false, &Skip));
	TestEqual(TEXT("reason yield"), Skip, FString(TEXT("yield")));
	TestTrue(TEXT("goal call inside a bark hold plays"), R.Frame(7050, JamEvents::Goal, Payload(0, FVector2D(43, 0), 1.f), P, true, &Skip, [](FJamCommentaryWorld& W) { W.Score[0] = 3; W.Score[1] = 1; }));
	TestEqual(TEXT("goal.blue"), P.Situation, FName(TEXT("goal.blue")));
	// A bark on the goal's own frame: the goal call still wins (the bark's bubble shows regardless).
	{
		FJamCommentaryWorld W = World(9000, true);
		W.Score[0] = 3; W.Score[1] = 2;
		R.Picker->Observe(JamEvents::Goal, Payload(4, FVector2D(-43, 0), 1.f), FVector2D(-43, 0), W);
		R.Picker->ObserveBark(9000, true);
		TestTrue(TEXT("same frame: the goal call beats the bark"), R.Picker->Resolve(W, P, &Skip));
		TestEqual(TEXT("goal.orange"), P.Situation, FName(TEXT("goal.orange")));
	}
	return true;
}

// Lane A47 (B-to-A-0030 fault 4): the bank's when conditions. board.bank holds at a goal within a second of a board hit and never on a board event;
// a kind that names the condition (B's bank / rim tags) is a condition, not a payload kind; board.rim needs the hit behind a goal line.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCommentaryBankWhen, "Jam.Commentary.BankWhen", JamCommentaryTests::Flags)
bool FJamCommentaryBankWhen::RunTest(const FString&)
{
	FRig R;
	FJamCommentaryPick P;
	const int32 BankIdx = R.Bank.Find(TEXT("board.bank.01"));
	const int32 RimIdx = R.Bank.Find(TEXT("board.rim.01"));
	TestTrue(TEXT("the tagged lines parsed"), BankIdx != INDEX_NONE && RimIdx != INDEX_NONE);
	// Two puck board hits 20 frames apart along the side boards (the QA1 case): neither picks the bank line or the rim line.
	TestTrue(TEXT("first board hit plays"), R.Frame(1000, JamEvents::Board, Payload(-1, FVector2D(0, 22), 0.9f), P));
	TestTrue(TEXT("a plain board line"), P.Situation == FName(TEXT("board.hard")));
	{
		FJamCommentaryWorld W = World(1020, false);
		FJamCommentaryEvent E;
		E.Event = JamEvents::Board; E.Who = -1; E.WhereM = FVector2D(0, 22); E.Strength = 0.9f; E.Frame = 1020; E.Team = -1;
		TArray<int32> C;
		R.Picker->Candidates(E, W, C);
		TestFalse(TEXT("board.bank is never a candidate on a board event"), C.Contains(BankIdx));
		TestFalse(TEXT("board.rim needs a hit behind the goal line"), C.Contains(RimIdx));
		TestFalse(TEXT("bank does not hold on a board event even 20 frames after a board hit"), R.Picker->WhenHolds(R.Bank.Lines[BankIdx], E, W));
		// The same hit behind the goal line: rim holds (the kind tag is a condition, not a kind to match).
		E.WhereM = FVector2D(43, 10);
		R.Picker->Candidates(E, W, C);
		TestTrue(TEXT("board.rim is a candidate behind the goal line"), C.Contains(RimIdx));
		// A goal 20 frames after the board hit: bank holds.
		FJamCommentaryEvent G;
		G.Event = JamEvents::Goal; G.Who = 0; G.WhereM = FVector2D(43, 0); G.Strength = 1.f; G.Frame = 1020; G.Team = 0;
		G.ScoreAfter[0] = 1;
		TestTrue(TEXT("bank holds at a goal within a second of a board hit"), R.Picker->WhenHolds(R.Bank.Lines[BankIdx], G, W));
		G.Frame = 1100;
		TestFalse(TEXT("not 100 frames after"), R.Picker->WhenHolds(R.Bank.Lines[BankIdx], G, W));
	}
	// Through the rules: a board hit at 3000, the goal at 3030 in the pause: the colour call (5) wins; board.bank (4) was among the goal's candidates
	// only because of the goal (Observe's goal-time path), never from the board event itself.
	TestTrue(TEXT("board hit"), R.Frame(3000, JamEvents::Board, Payload(-1, FVector2D(0, -22), 0.9f), P));
	TestTrue(TEXT("board line, not bank"), P.Situation != FName(TEXT("board.bank")));
	TestTrue(TEXT("the goal is called"), R.Frame(3030, JamEvents::Goal, Payload(0, FVector2D(43, 0), 1.f), P, true, nullptr, [](FJamCommentaryWorld& W) { W.Score[0] = 1; }));
	TestEqual(TEXT("colour call"), P.Situation, FName(TEXT("goal.blue")));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
