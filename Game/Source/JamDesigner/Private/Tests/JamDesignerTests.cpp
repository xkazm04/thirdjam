// Lane A7 tests (headless, no world, no network): Jam.Designer.*. Run:
//   UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Designer; Quit" -unattended -nullrhi -nosplash -log
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "JamDesignerSubsystem.h"
#include "JamDesignerPrompt.h"
#include "JamDesignerTransport.h"
#include "JamCartridge.h"
#include "JamEventBus.h"
#include "JamJson.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "Async/TaskGraphInterfaces.h"

#include "../JamDesignerData.inl"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamDesignerTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;
	// Not a key: a placeholder so the subsystem takes the live path against the mock transport.
	static const TCHAR* FakeKey = TEXT("placeholder-not-a-key");

	// Counts every Post and answers from a queue of canned model texts (the last one repeats). Never touches the network.
	class FMockTransport : public IJamDesignerTransport
	{
	public:
		int32 Calls = 0;
		TArray<FString> Bodies;
		TArray<FString> Replies;
		int32 Status = 200;
		virtual void Post(const FString&, const TArray<TPair<FString, FString>>&, const FString& Body, float, FJamDesignerHttpDone Done) override
		{
			Bodies.Add(Body);
			const FString Text = Replies.Num() ? Replies[FMath::Min(Calls, Replies.Num() - 1)] : FString();
			++Calls;
			// The reply text goes into a Messages API envelope exactly as the API sends it.
			TSharedPtr<FJamJson> S = MakeShared<FJamJson>();
			S->Type = FJamJson::EType::String;
			S->String = Text;
			FJamDesignerHttpReply R;
			R.bConnected = true;
			R.Status = Status;
			R.Ms = 5;
			R.Body = FString::Printf(TEXT("{\"content\":[{\"type\":\"text\",\"text\":%s}],\"stop_reason\":\"end_turn\",\"usage\":{\"input_tokens\":100,\"cache_read_input_tokens\":4000,\"output_tokens\":900}}"), *S->ToCompact());
			Done(R);
		}
	};

	static FString Regular()
	{
		FString T;
		FFileHelper::LoadFileToString(T, *UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")));
		return T;
	}

	static UJamDesignerSubsystem* Make(TSharedPtr<FMockTransport> Mock, const FString& Key)
	{
		// A subsystem must live inside a game instance; a bare one (never Init) is enough here.
		UJamDesignerSubsystem* S = NewObject<UJamDesignerSubsystem>(NewObject<UGameInstance>());
		S->SetKeyOverrideForTest(Key);
		S->SetCliForTest(false);
		S->RetryDelaySec = 0.f;
		if (Mock) S->SetTransport(Mock);
		S->SetSavedCutsDirForTest(FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Automation"), TEXT("DesignerCuts")));
		return S;
	}

	static FJamDesignerResult RunSync(UJamDesignerSubsystem* S, const FString& Sentence, const FString& Base)
	{
		FJamDesignerResult Out;
		bool bDone = false;
		S->RequestCut(Sentence, Base, [&](const FJamDesignerResult& R) { Out = R; bDone = true; });
		check(bDone); // the mock and the offline path both answer in the same frame
		return Out;
	}

	static FString Edit(const FString& Json, const TFunction<void(FJamJson&)>& F)
	{
		FString Err;
		TSharedPtr<FJamJson> Root = FJamJson::Parse(Json, Err);
		F(*Root);
		return Root->ToCompact();
	}

	static void SetNum(FJamJson& Obj, const FString& K, double V)
	{
		TSharedPtr<FJamJson> N = MakeShared<FJamJson>();
		N->Type = FJamJson::EType::Number;
		N->Number = V;
		Obj.Object.RemoveAll([&](const TPair<FString, TSharedPtr<FJamJson>>& P) { return P.Key == K; });
		Obj.Object.Add({ K, N });
	}
}

using namespace JamDesignerTests;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDesignerOfflineResolves, "Jam.Designer.DesignerOfflineResolves", JamDesignerTests::Flags)
bool FJamDesignerOfflineResolves::RunTest(const FString& Parameters)
{
	TSharedPtr<FMockTransport> Mock = MakeShared<FMockTransport>();
	UJamDesignerSubsystem* S = Make(Mock, FString());
	const FJamDesignerResult R = RunSync(S, TEXT("make it faster"), FString());
	TestTrue(TEXT("outcome"), R.Outcome == EJamDesignerOutcome::Offline);
	TestTrue(TEXT("has a validated cartridge"), R.HasCartridge());
	TestEqual(TEXT("cut"), R.CutName, FString(TEXT("Fast Cut")));
	TArray<FString> RegErrors;
	const UJamCartridge* Reg = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")), RegErrors);
	if (R.Cartridge && Reg) TestTrue(TEXT("faster skaters than hockey.regular"), R.Cartridge->Game.Skater.Speed > Reg->Game.Skater.Speed);
	TestEqual(TEXT("no transport call"), Mock->Calls, 0);

	const FJamDesignerResult Calm = RunSync(S, TEXT("something calm for a kid please"), FString());
	TestEqual(TEXT("calm cut"), Calm.CutName, FString(TEXT("Family Cut")));
	// Paraphrases a judge might type reach the right cut through the keywords.
	const TPair<const TCHAR*, const TCHAR*> Paraphrases[] = {
		{ TEXT("harder bots please"), TEXT("All-Star Cut") }, { TEXT("slippery ice"), TEXT("Glass Ice Cut") }, { TEXT("halloween"), TEXT("Midnight Cut") },
		{ TEXT("turbo"), TEXT("Turbo Cut") }, { TEXT("a huge arena"), TEXT("Big Glass Cut") }, { TEXT("first goal wins"), TEXT("Sudden Death Cut") },
	};
	for (const auto& P : Paraphrases) TestEqual(FString::Printf(TEXT("\"%s\""), P.Key), S->ResolveOffline(P.Key).CutName, FString(P.Value));
	const FJamDesignerResult None = RunSync(S, TEXT("zebra xylophone"), FString());
	TestTrue(TEXT("no match"), None.Outcome == EJamDesignerOutcome::OfflineNoMatch);
	TestFalse(TEXT("no match lists the cached sentences"), None.Message.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDesignerValidatesOutput, "Jam.Designer.DesignerValidatesOutput", JamDesignerTests::Flags)
bool FJamDesignerValidatesOutput::RunTest(const FString& Parameters)
{
	const FString Base = Regular();
	const FString Bad = Edit(Base, [](FJamJson& R) { SetNum(*R.Get(TEXT("values")), TEXT("skater_speed"), 99); });
	const FString Good = Edit(Base, [](FJamJson& R) { SetNum(*R.Get(TEXT("values")), TEXT("skater_speed"), 13); });

	// A canned bad JSON every time: the chain repairs (haiku), retries (sonnet), then rejects. Nothing is applied.
	{
		TSharedPtr<FMockTransport> Mock = MakeShared<FMockTransport>();
		Mock->Replies = { Bad };
		UJamDesignerSubsystem* S = Make(Mock, FakeKey);
		const FJamDesignerResult R = RunSync(S, TEXT("make it faster"), Base);
		TestTrue(TEXT("rejected"), R.Outcome == EJamDesignerOutcome::Failed);
		TestFalse(TEXT("no cartridge"), R.HasCartridge());
		TestEqual(TEXT("three attempts"), Mock->Calls, 3);
		TestTrue(TEXT("error names the pointer"), R.Errors.Num() > 0 && R.Errors[0].StartsWith(TEXT("at /values/skater_speed")));
		if (Mock->Bodies.Num() == 3)
		{
			TestTrue(TEXT("attempt 1 is haiku, fresh"), Mock->Bodies[0].Contains(TEXT("\"model\":\"claude-haiku-5-5\"")) && Mock->Bodies[0].Contains(TEXT("<player_sentence>")) && !Mock->Bodies[0].Contains(TEXT("<problems>")));
			TestTrue(TEXT("attempt 2 is a haiku repair carrying the error"), Mock->Bodies[1].Contains(TEXT("\"model\":\"claude-haiku-5-5\"")) && Mock->Bodies[1].Contains(TEXT("<problems>")) && Mock->Bodies[1].Contains(TEXT("- at /values/skater_speed")));
			TestTrue(TEXT("attempt 3 is a sonnet repair"), Mock->Bodies[2].Contains(TEXT("\"model\":\"claude-sonnet-5-5\"")) && Mock->Bodies[2].Contains(TEXT("<previous_reply>")));
			for (const FString& B : Mock->Bodies)
			{
				TestTrue(TEXT("max_tokens 8000, effort low"), B.Contains(TEXT("\"max_tokens\":8000")) && B.Contains(TEXT("\"output_config\":{\"effort\":\"low\"}")));
				TestFalse(TEXT("no temperature"), B.Contains(TEXT("temperature")));
				TestFalse(TEXT("the key is never in the body"), B.Contains(FakeKey));
			}
		}
		TestEqual(TEXT("tokens in counted with cache"), R.TokensIn, (int64)(3 * 4100));
		TestEqual(TEXT("tokens out"), R.TokensOut, (int64)(3 * 900));
	}
	// One bad reply, then a good one: one repair, then applied.
	{
		TSharedPtr<FMockTransport> Mock = MakeShared<FMockTransport>();
		Mock->Replies = { Bad, Good };
		UJamDesignerSubsystem* S = Make(Mock, FakeKey);
		const FJamDesignerResult R = RunSync(S, TEXT("make it faster"), Base);
		TestTrue(TEXT("repaired"), R.Outcome == EJamDesignerOutcome::Repaired);
		TestEqual(TEXT("two attempts"), Mock->Calls, 2);
		TestTrue(TEXT("validated cartridge"), R.Cartridge && R.Cartridge->Game.Skater.Speed == 13.0);
	}
	// Prose around the JSON, a protected change, and a refusal.
	{
		const FJamDesignerParsedReply P = FJamDesignerPrompt::ParseReply(TEXT("Sure! ") + Good, Base);
		TestTrue(TEXT("prose rejected"), P.Kind == EJamDesignerReplyKind::Invalid && P.Errors.Num() == 1 && P.Errors[0].Contains(TEXT("text outside the JSON object")));
		const FString Cam = Edit(Base, [](FJamJson& R) { SetNum(*R.Get(TEXT("game"))->Get(TEXT("camera")), TEXT("fov_deg"), 90); });
		const FJamDesignerParsedReply C = FJamDesignerPrompt::ParseReply(Cam, Base);
		TestTrue(TEXT("camera change rejected with its pointer"), C.Kind == EJamDesignerReplyKind::Invalid && C.Errors.Num() == 1 && C.Errors[0].StartsWith(TEXT("at /game/camera/fov_deg:")));
		const FString Rink = Edit(Base, [](FJamJson& R) { SetNum(*R.Get(TEXT("game"))->Get(TEXT("rink")), TEXT("length_m"), 100); });
		TestTrue(TEXT("rink length may change"), FJamDesignerPrompt::ParseReply(Rink, Base).Kind == EJamDesignerReplyKind::Cartridge);

		TSharedPtr<FMockTransport> Mock = MakeShared<FMockTransport>();
		Mock->Replies = { TEXT("{\"refusal\": \"I can't add online play, but I can make the rink bigger.\"}") };
		UJamDesignerSubsystem* S = Make(Mock, FakeKey);
		const FJamDesignerResult R = RunSync(S, TEXT("online play please"), Base);
		TestTrue(TEXT("refused gracefully"), R.Outcome == EJamDesignerOutcome::Refused);
		TestEqual(TEXT("one call"), Mock->Calls, 1);
		TestFalse(TEXT("refusal text"), R.Refusal.IsEmpty());
	}
	// A fatal HTTP status stops the chain at once.
	{
		TSharedPtr<FMockTransport> Mock = MakeShared<FMockTransport>();
		Mock->Status = 401;
		UJamDesignerSubsystem* S = Make(Mock, FakeKey);
		const FJamDesignerResult R = RunSync(S, TEXT("zebra xylophone"), Base);
		TestEqual(TEXT("401 stops after one call"), Mock->Calls, 1);
		TestTrue(TEXT("failed"), R.Outcome == EJamDesignerOutcome::Failed);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDesignerDirectorsCallRange, "Jam.Designer.DirectorsCallRange", JamDesignerTests::Flags)
bool FJamDesignerDirectorsCallRange::RunTest(const FString& Parameters)
{
	UJamDesignerSubsystem* S = Make(nullptr, FString());
	UJamEventBus* Bus = NewObject<UJamEventBus>(S->GetOuter());
	Bus->bLogEvents = false;
	S->SetEventBusForTest(Bus);
	TArray<FString> Errors;
	UJamCartridge* Base = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")), Errors);
	if (!TestNotNull(TEXT("regular loads"), Base)) return false;
	S->SetCurrentCartridge(Base);

	FJamDirectorsCall C = S->DirectorsCall(TEXT("ice_friction"), 0.5);
	TestFalse(TEXT("out of range refused"), C.bApplied);
	TestTrue(TEXT("reason names the range"), C.Reason.Contains(TEXT("0..0.1")));
	TestTrue(TEXT("current unchanged"), S->GetCurrentCartridge() == Base);
	TestFalse(TEXT("unknown key refused"), S->DirectorsCall(TEXT("number_of_goalies"), 1).bApplied);
	TestFalse(TEXT("integer tunable refuses a fraction"), S->DirectorsCall(TEXT("goal_pause_frames"), 60.5).bApplied);
	TestEqual(TEXT("nothing broadcast"), Bus->GetBroadcastCount(), 0);

	C = S->DirectorsCall(TEXT("ice_friction"), 0.001);
	TestTrue(TEXT("in range applied"), C.bApplied);
	TestEqual(TEXT("announcement"), C.Announcement, FString(TEXT("Director's Call: ice is now glass")));
	TestTrue(TEXT("new cut is current"), S->GetCurrentCartridge() != Base && S->GetCurrentCartridge() == C.Cartridge);
	if (C.Cartridge) TestEqual(TEXT("game field follows the value"), C.Cartridge->Game.Rink.IceFriction, 0.001);
	TestEqual(TEXT("old value"), C.OldValue, 0.015);
	if (UJamEventBus::IsKnownEvent(UJamDesignerSubsystem::DirectorsCallEvent)) TestEqual(TEXT("broadcast on the bus"), Bus->GetBroadcastCount(), 1);
	else AddInfo(TEXT("seam: directors_call is not in JamEvents yet; the call was delivered through OnDirectorsCall only"));

	TestFalse(TEXT("second call in the same period refused"), S->DirectorsCall(TEXT("shot_speed"), 20).bApplied);
	S->NotifyPeriodStarted(2);
	C = S->DirectorsCall(TEXT("goal_pause_frames"), 60);
	TestTrue(TEXT("next period allows a call (integer)"), C.bApplied);
	if (C.Cartridge) TestEqual(TEXT("both calls kept"), C.Cartridge->Game.Rink.IceFriction, 0.001);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDesignerNoKeyNoNetwork, "Jam.Designer.NoKeyNoNetwork", JamDesignerTests::Flags)
bool FJamDesignerNoKeyNoNetwork::RunTest(const FString& Parameters)
{
	TSharedPtr<FMockTransport> Mock = MakeShared<FMockTransport>();
	UJamDesignerSubsystem* S = Make(Mock, FString());
	TestFalse(TEXT("no key"), S->HasApiKey());
	for (const TCHAR* Sentence : { TEXT("make it faster"), TEXT("zebra"), TEXT("online play") }) RunSync(S, Sentence, FString());
	TestEqual(TEXT("mock transport never called"), Mock->Calls, 0);

	// The real HTTP transport too: no request object is created without a key.
	UJamDesignerSubsystem* Real = Make(nullptr, FString());
	const int32 Before = FJamDesignerHttpTransport::GetRequestsCreated();
	const int32 CliBefore = FJamCliTransport::GetProcessesStarted();
	RunSync(Real, TEXT("make it faster"), FString());
	TestEqual(TEXT("no IHttpRequest created"), FJamDesignerHttpTransport::GetRequestsCreated(), Before);
	TestEqual(TEXT("no CLI process started"), FJamCliTransport::GetProcessesStarted(), CliBefore);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDesignerPromptParity, "Jam.Designer.PromptParity", JamDesignerTests::Flags)
bool FJamDesignerPromptParity::RunTest(const FString& Parameters)
{
	const FString& System = FJamDesignerPrompt::GetSystemText();
	TestEqual(TEXT("system bytes = assemble.mjs"), (int32)FTCHARToUTF8(*System, System.Len()).Length(), JamDesignerParitySystemBytes);
	TestTrue(TEXT("system hash = assemble.mjs"), FJamDesignerPrompt::Fnv1a64(System) == JamDesignerParitySystemHash);
	TestFalse(TEXT("no placeholder left"), System.Contains(TEXT("{{")));
	TestFalse(TEXT("blocked_terms stay out"), System.Contains(TEXT("blocked_terms")));

	FString Body, Err;
	TestTrue(TEXT("first assembles"), FJamDesignerPrompt::AssembleBody(JamDesignerParitySentence, Regular(), JamDesignerModels::First, nullptr, {}, Body, Err));
	TestEqual(TEXT("first body bytes = assemble.mjs"), (int32)FTCHARToUTF8(*Body, Body.Len()).Length(), JamDesignerParityFirstBytes);
	TestTrue(TEXT("first body hash = assemble.mjs"), FJamDesignerPrompt::Fnv1a64(Body) == JamDesignerParityFirstHash);

	const FString Prev = JamDesignerParityPrevious;
	TArray<FString> Errs;
	for (const TCHAR* E : JamDesignerParityErrors) Errs.Add(E);
	TestTrue(TEXT("repair assembles"), FJamDesignerPrompt::AssembleBody(JamDesignerParitySentence, Regular(), JamDesignerModels::Retry, &Prev, Errs, Body, Err));
	TestEqual(TEXT("repair body bytes = assemble.mjs"), (int32)FTCHARToUTF8(*Body, Body.Len()).Length(), JamDesignerParityRepairBytes);
	TestTrue(TEXT("repair body hash = assemble.mjs"), FJamDesignerPrompt::Fnv1a64(Body) == JamDesignerParityRepairHash);

	TestFalse(TEXT("empty sentence refused"), FJamDesignerPrompt::AssembleBody(TEXT("   "), Regular(), JamDesignerModels::First, nullptr, {}, Body, Err));
	TestFalse(TEXT("long sentence refused"), FJamDesignerPrompt::AssembleBody(FString::ChrN(301, 'a'), Regular(), JamDesignerModels::First, nullptr, {}, Body, Err));

	// The embedded copies match the files on disk (run gen-designer-data.mjs after a change).
	const FString Repo = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("..")));
	const TPair<const TCHAR*, FString> Files[] = {
		{ TEXT("designer.system.md"), FPaths::Combine(Repo, TEXT("prompts/designer.system.md")) },
		{ TEXT("designer.repair.md"), FPaths::Combine(Repo, TEXT("prompts/designer.repair.md")) },
		{ TEXT("game.schema.json"), FPaths::Combine(Repo, TEXT("schema/game.schema.json")) },
		{ TEXT("limits.json"), FPaths::Combine(Repo, TEXT("schema/limits.json")) },
		{ TEXT("cached-cuts.json"), FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Designer/cached-cuts.json")) },
	};
	for (const auto& F : Files)
	{
		FString Disk;
		if (FFileHelper::LoadFileToString(Disk, *F.Value)) TestTrue(FString::Printf(TEXT("embedded %s is current"), F.Key), Disk == FJamDesignerPrompt::GetEmbeddedFile(F.Key));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDesignerCutsRoundTrip, "Jam.Designer.CutsRoundTrip", JamDesignerTests::Flags)
bool FJamDesignerCutsRoundTrip::RunTest(const FString& Parameters)
{
	UJamDesignerSubsystem* S = Make(nullptr, FString());
	IFileManager::Get().DeleteDirectory(*S->GetSavedCutsDir(), false, true);
	const FString Base = Regular();

	// Every cached sentence resolves to its own cut, and every cached cut is a legal Designer edit of hockey.regular.
	const TArray<FString> Sentences = S->GetCachedSentences();
	TestTrue(TEXT("24 to 40 cached cuts (R-0004)"), Sentences.Num() >= 24 && Sentences.Num() <= 40);
	for (const FString& Sentence : Sentences)
	{
		const FJamDesignerResult R = S->ResolveOffline(Sentence);
		TestTrue(FString::Printf(TEXT("\"%s\" resolves"), *Sentence), R.Outcome == EJamDesignerOutcome::Offline);
		const TArray<FJamCutInfo> Cached = S->GetCachedCuts();
		const FJamCutInfo* Own = Cached.FindByPredicate([&](const FJamCutInfo& I) { return I.Sentence == Sentence; });
		TestTrue(FString::Printf(TEXT("\"%s\" resolves to its own cut"), *Sentence), Own && Own->Name == R.CutName);
		const FJamDesignerParsedReply P = FJamDesignerPrompt::ParseReply(R.CartridgeJson, Base);
		TestTrue(FString::Printf(TEXT("\"%s\" is a legal Designer edit"), *Sentence), P.Kind == EJamDesignerReplyKind::Cartridge);
		for (const FString& E : P.Errors) AddError(E);
	}

	const TArray<FJamCutInfo> Before = S->ListCuts();
	TestTrue(TEXT("shipped presets listed"), Before.ContainsByPredicate([](const FJamCutInfo& I) { return I.Name == TEXT("hockey.regular") && !I.bSaved; }));
	TestFalse(TEXT("test cartridges hidden"), Before.ContainsByPredicate([](const FJamCutInfo& I) { return I.Name.Contains(TEXT(".test.")); }));

	const FJamDesignerResult Neon = S->ResolveOffline(TEXT("a neon party"));
	FString Err;
	TestTrue(TEXT("apply saves and loads My Director's Cut"), S->ApplyResult(Neon, Err));
	const TArray<FJamCutInfo> After = S->ListCuts();
	const FJamCutInfo* Mine = After.FindByPredicate([](const FJamCutInfo& I) { return I.bSaved; });
	if (TestNotNull(TEXT("saved cut listed"), Mine))
	{
		TestEqual(TEXT("saved name"), Mine->Name, UJamDesignerSubsystem::MyDirectorsCutName);
		TestEqual(TEXT("saved sentence"), Mine->Sentence, FString(TEXT("a neon party")));
	}
	TestTrue(TEXT("load by name"), S->LoadCut(UJamDesignerSubsystem::MyDirectorsCutName, Err));
	TestTrue(TEXT("neon look"), S->GetCurrentCartridge() && S->GetCurrentCartridge()->Game.Look.Tier == TEXT("l3_neon"));
	TestTrue(TEXT("load a preset"), S->LoadCut(TEXT("hockey.raw"), Err));
	TestFalse(TEXT("unknown cut"), S->LoadCut(TEXT("no-such-cut"), Err));
	TestFalse(TEXT("an invalid cut is not saved"), !S->SaveCut(TEXT("Bad"), TEXT("{\"schema\":\"tj.cartridge/1\"}"), FString(), Err).IsEmpty());
	IFileManager::Get().DeleteDirectory(*S->GetSavedCutsDir(), false, true);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDesignerCliTransportParsesResult, "Jam.Designer.CliTransportParsesResult", JamDesignerTests::Flags)
bool FJamDesignerCliTransportParsesResult::RunTest(const FString& Parameters)
{
	// A fake `claude` on PATH: records its arguments and stdin, answers with reply.json. No real CLI, no network.
	const FString Dir = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Automation"), TEXT("FakeClaude")));
	IFileManager::Get().DeleteDirectory(*Dir, false, true);
	IFileManager::Get().MakeDirectory(*Dir, true);
	FFileHelper::SaveStringToFile(TEXT("@echo off\r\necho %*> \"%~dp0args.txt\"\r\nfindstr \"^\" > \"%~dp0stdin.txt\"\r\ntype \"%~dp0reply.json\"\r\n"),
		*FPaths::Combine(Dir, TEXT("claude.cmd")), FFileHelper::EEncodingOptions::ForceAnsi);
	const FString Good = Edit(Regular(), [](FJamJson& R) { SetNum(*R.Get(TEXT("values")), TEXT("skater_speed"), 13); });
	TSharedPtr<FJamJson> Text = MakeShared<FJamJson>();
	Text->Type = FJamJson::EType::String;
	Text->String = Good;
	auto WriteReply = [&](const FString& Json) { FFileHelper::SaveStringToFile(Json, *FPaths::Combine(Dir, TEXT("reply.json")), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM); };
	WriteReply(FString::Printf(TEXT("{\"type\":\"result\",\"subtype\":\"success\",\"is_error\":false,\"result\":%s,\"stop_reason\":\"end_turn\",\"usage\":{\"input_tokens\":3,\"cache_creation_input_tokens\":9000,\"output_tokens\":1200}}"), *Text->ToCompact()));

	const FString OldPath = FPlatformMisc::GetEnvironmentVariable(TEXT("PATH"));
	FPlatformMisc::SetEnvironmentVar(TEXT("PATH"), *(Dir + TEXT(";") + OldPath));
	const FString Found = FJamCliTransport::FindClaudeOnPath();
	TestEqual(TEXT("fake claude found first on PATH"), FPaths::GetCleanFilename(Found), FString(TEXT("claude.cmd")));

	auto Run = [&](UJamDesignerSubsystem* S, const FString& Sentence)
	{
		FJamDesignerResult Out;
		bool bDone = false;
		S->RequestCut(Sentence, Regular(), [&](const FJamDesignerResult& R) { Out = R; bDone = true; });
		const double Start = FPlatformTime::Seconds();
		while (!bDone && FPlatformTime::Seconds() - Start < 30.0)
		{
			FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);
			FPlatformProcess::Sleep(0.01f);
		}
		TestTrue(TEXT("the CLI request finished"), bDone);
		return Out;
	};

	UJamDesignerSubsystem* S = Make(nullptr, FString());
	S->SetCliForTest(true);
	TestTrue(TEXT("live available through the CLI"), S->IsLiveAvailable());
	const int32 Before = FJamCliTransport::GetProcessesStarted();
	const FJamDesignerResult R = Run(S, TEXT("make it faster"));
	TestTrue(TEXT("first try"), R.Outcome == EJamDesignerOutcome::FirstTry);
	TestTrue(TEXT("validated cartridge"), R.Cartridge && R.Cartridge->Game.Skater.Speed == 13.0);
	TestEqual(TEXT("one process"), FJamCliTransport::GetProcessesStarted(), Before + 1);
	TestEqual(TEXT("tokens in (with cache)"), R.TokensIn, (int64)9003);
	TestEqual(TEXT("tokens out"), R.TokensOut, (int64)1200);

	FString Args, Stdin;
	FFileHelper::LoadFileToString(Args, *FPaths::Combine(Dir, TEXT("args.txt")));
	FFileHelper::LoadFileToString(Stdin, *FPaths::Combine(Dir, TEXT("stdin.txt")));
	TestTrue(TEXT("argv: print, haiku, json, one turn, system prompt file"), Args.Contains(TEXT("-p --model claude-haiku-5-5 --output-format json --max-turns 1 --system-prompt-file")));
	TestTrue(TEXT("argv: no tools, effort low"), Args.Contains(TEXT("--tools \"\"")) && Args.Contains(TEXT("--effort low")));
	TestFalse(TEXT("the sentence is never on argv"), Args.Contains(TEXT("faster")));
	TestTrue(TEXT("the sentence is on stdin"), Stdin.Contains(TEXT("<player_sentence>")) && Stdin.Contains(TEXT("make it faster")));

	// An error result with a fatal status stops the chain after one process.
	WriteReply(TEXT("{\"type\":\"result\",\"is_error\":true,\"result\":\"Invalid API key\",\"api_error_status\":401}"));
	const int32 Before2 = FJamCliTransport::GetProcessesStarted();
	const FJamDesignerResult E = Run(S, TEXT("zebra xylophone"));
	TestTrue(TEXT("failed"), E.Outcome == EJamDesignerOutcome::Failed);
	TestEqual(TEXT("fatal error: one process"), FJamCliTransport::GetProcessesStarted(), Before2 + 1);

	int32 Status = 0;
	FJamCliTransport::ToApiBody(TEXT("not json"), 1, Status);
	TestEqual(TEXT("garbage stdout is a 502"), Status, 502);
	const FJamDesignerApiReply Api = FJamDesignerPrompt::ParseApiResponse(FJamCliTransport::ToApiBody(TEXT("{\"is_error\":false,\"result\":\"{}\",\"stop_reason\":\"refusal\"}"), 0, Status));
	TestTrue(TEXT("result and stop_reason carried"), Status == 200 && Api.Text == TEXT("{}") && Api.StopReason == TEXT("refusal"));

	FPlatformMisc::SetEnvironmentVar(TEXT("PATH"), *OldPath);
	IFileManager::Get().DeleteDirectory(*Dir, false, true);
	return true;
}

// Opt-in (spends the operator's CLI subscription): set TJ_DESIGNER_CLI_LIVE=1. The real claude executable through CreateProc.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDesignerCliLive, "Jam.Designer.CliLive", JamDesignerTests::Flags)
bool FJamDesignerCliLive::RunTest(const FString& Parameters)
{
	if (FPlatformMisc::GetEnvironmentVariable(TEXT("TJ_DESIGNER_CLI_LIVE")) != TEXT("1")) { AddInfo(TEXT("skipped: set TJ_DESIGNER_CLI_LIVE=1 to run")); return true; }
	UJamDesignerSubsystem* S = Make(nullptr, FString());
	S->SetCliForTest(true);
	if (!TestFalse(TEXT("claude on PATH"), S->ResolveCliExe().IsEmpty())) return false;
	FJamDesignerResult Out;
	bool bDone = false;
	S->RequestCut(TEXT("make it faster"), Regular(), [&](const FJamDesignerResult& R) { Out = R; bDone = true; });
	const double Start = FPlatformTime::Seconds();
	while (!bDone && FPlatformTime::Seconds() - Start < 400.0)
	{
		FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);
		FPlatformProcess::Sleep(0.05f);
	}
	TestTrue(TEXT("finished"), bDone);
	TestTrue(TEXT("a validated cut from the real CLI"), Out.HasCartridge());
	AddInfo(FString::Printf(TEXT("outcome=%s attempts=%d tokens_in=%lld tokens_out=%lld ms=%.0f"), *StaticEnum<EJamDesignerOutcome>()->GetNameStringByValue((int64)Out.Outcome), Out.Attempts, Out.TokensIn, Out.TokensOut, Out.Ms));
	for (const FString& E : Out.Errors) AddInfo(E);
	return true;
}

#endif
