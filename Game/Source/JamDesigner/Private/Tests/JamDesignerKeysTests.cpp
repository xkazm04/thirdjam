// Lane A44: Jam.Designer.KeyFamilies. Tonight's keys are the Designer's: the three payoffs (shot.one_timer, skater.breakaway, stick.deke;
// lane A39), stick.foul_grace_frames (A16), commentary (A32), look.trim_emissive and the p4_broadcast tier (A21 / A31), the three
// chase-camera numbers (A40). One mock reply per family over hockey.regular must parse as a cartridge and change nothing outside its
// family; an out-of-range value must come back as the pointer error; the rest of camera, skater, shot and stick stay protected; the
// system prompt names every family and stays under 70 KB (the game schema excerpt is trimmed to the editable paths).
// The live section (TJ_DESIGNER_CLI_LIVE=1, spends the operator's CLI subscription) sends the brief's seven sentences through the
// game's own CLI transport and applies the same check. The Node twin is Game/Source/JamDesigner/Tools/paraphrase-keys.mjs. Run:
//   UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Designer.KeyFamilies; Quit" -unattended -nullrhi -nosplash -log
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "Async/TaskGraphInterfaces.h"
#include "Engine/GameInstance.h"
#include "JamDesignerPrompt.h"
#include "JamDesignerSubsystem.h"
#include "JamCartridge.h"
#include "JamJson.h"

#include "../JamDesignerData.inl"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamDesignerKeysTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;
	using FJPtr = TSharedPtr<FJamJson>;

	struct FFamily
	{
		const TCHAR* Id;
		const TCHAR* Sentence;      // the brief's paraphrase
		const TCHAR* Path;          // dotted path under game, the one the family owns
		const TCHAR* Good;          // a sensible value (JSON)
		const TCHAR* Bad;           // out of range (JSON)
		const TCHAR* BadPointer;    // where the validator must point
		const TCHAR* PromptWord;    // what the system prompt must say about it
	};
	static const FFamily Families[] = {
		{ TEXT("one_timer"), TEXT("make one-timers deadly"), TEXT("shot.one_timer"), TEXT("{\"window_frames\":24,\"cone_bonus_deg\":16}"), TEXT("{\"window_frames\":90,\"cone_bonus_deg\":16}"), TEXT("/game/shot/one_timer/window_frames"), TEXT("`shot.one_timer`") },
		{ TEXT("breakaway"), TEXT("give breakaways a burst"), TEXT("skater.breakaway"), TEXT("{\"speed_bonus\":0.4,\"frames\":120}"), TEXT("{\"speed_bonus\":1.5,\"frames\":120}"), TEXT("/game/skater/breakaway/speed_bonus"), TEXT("`skater.breakaway`") },
		{ TEXT("deke"), TEXT("let me deke the last man"), TEXT("stick.deke"), TEXT("{\"offset_m\":1.5,\"commit_frames\":30}"), TEXT("{\"offset_m\":3,\"commit_frames\":30}"), TEXT("/game/stick/deke/offset_m"), TEXT("`stick.deke`") },
		{ TEXT("commentary"), TEXT("a quiet commentator"), TEXT("commentary"), TEXT("{\"enabled\":true,\"intensity\":\"quiet\"}"), TEXT("{\"intensity\":\"silent\"}"), TEXT("/game/commentary/intensity"), TEXT("`commentary`") },
		{ TEXT("camera"), TEXT("camera higher at the end boards"), TEXT("camera.end_height_m"), TEXT("12"), TEXT("40"), TEXT("/game/camera/end_height_m"), TEXT("`camera.end_height_m`") },
		{ TEXT("foul_grace"), TEXT("no cheap fouls after a strip"), TEXT("stick.foul_grace_frames"), TEXT("75"), TEXT("600"), TEXT("/game/stick/foul_grace_frames"), TEXT("`stick.foul_grace_frames`") },
		{ TEXT("broadcast"), TEXT("the broadcast look"), TEXT("look.tier"), TEXT("\"p4_broadcast\""), TEXT("\"p5_imax\""), TEXT("/game/look/tier"), TEXT("`p4_broadcast`") },
	};

	static FString Regular()
	{
		FString T;
		FFileHelper::LoadFileToString(T, *UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")));
		return T;
	}

	// The cartridge text with game.<Path> set to the parsed Json, creating the objects on the way (a payoff key is absent in hockey.regular).
	static FString WithGamePath(const FString& BaseText, const FString& Path, const FString& Json, FString& OutErr)
	{
		FJPtr Root = FJamJson::Parse(BaseText, OutErr);
		if (!Root) return FString();
		FJPtr Value = FJamJson::Parse(Json, OutErr);
		if (!Value) return FString();
		TArray<FString> Parts;
		Path.ParseIntoArray(Parts, TEXT("."));
		FJPtr Node = Root->Get(TEXT("game"));
		for (int32 i = 0; i + 1 < Parts.Num(); ++i)
		{
			FJPtr Next = Node->Get(Parts[i]);
			if (!Next)
			{
				Next = MakeShared<FJamJson>();
				Next->Type = FJamJson::EType::Object;
				Node->Object.Add(TPair<FString, FJPtr>(Parts[i], Next));
			}
			Node = Next;
		}
		Node->Object.RemoveAll([&](const TPair<FString, FJPtr>& P) { return P.Key == Parts.Last(); });
		Node->Object.Add(TPair<FString, FJPtr>(Parts.Last(), Value));
		return Root->ToCompact();
	}

	// Leaf differences between two JSON values as pointers. An object added where the other side has none (hockey.regular has no
	// `stick`, no `one_timer`) is walked leaf by leaf, so the pointers name the keys that appeared.
	static void Diffs(const FJamJson* X, const FJamJson* Y, const FString& Ptr, TArray<FString>& Out)
	{
		const bool bXObj = X && X->IsObject(), bYObj = Y && Y->IsObject();
		const bool bObjs = (bXObj || !X) && (bYObj || !Y) && (bXObj || bYObj);
		const bool bArrs = X && Y && X->IsArray() && Y->IsArray() && X->Array.Num() == Y->Array.Num();
		if (bObjs)
		{
			TArray<FString> Keys;
			if (X) for (const auto& P : X->Object) Keys.AddUnique(P.Key);
			if (Y) for (const auto& P : Y->Object) Keys.AddUnique(P.Key);
			for (const FString& K : Keys) Diffs(X ? X->Get(K).Get() : nullptr, Y ? Y->Get(K).Get() : nullptr, FJamJson::PointerChild(Ptr, K), Out);
		}
		else if (bArrs)
		{
			for (int32 i = 0; i < X->Array.Num(); ++i) Diffs(X->Array[i].Get(), Y->Array[i].Get(), FJamJson::PointerChild(Ptr, i), Out);
		}
		else if (!FJamDesignerPrompt::DeepEqual(X, Y)) Out.Add(Ptr);
	}

	static TArray<FString> ChangedPaths(const FString& BaseText, const FString& OutText)
	{
		FString Err;
		FJPtr A = FJamJson::Parse(BaseText, Err), B = FJamJson::Parse(OutText, Err);
		TArray<FString> Out;
		Diffs(A.Get(), B.Get(), FString(), Out);
		return Out;
	}

	// True when every changed pointer is inside the family's path (a theme change is tolerated: the prompt lets the model rename the cut).
	static bool OnlyInside(const TArray<FString>& Changed, const FString& Path, TArray<FString>& OutForeign)
	{
		const FString Own = TEXT("/game/") + Path.Replace(TEXT("."), TEXT("/"));
		for (const FString& P : Changed)
		{
			if (P == Own || P.StartsWith(Own + TEXT("/")) || P == TEXT("/theme") || P.StartsWith(TEXT("/theme/"))) continue;
			OutForeign.Add(P);
		}
		return OutForeign.Num() == 0;
	}
}

using namespace JamDesignerKeysTests;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDesignerKeyFamilies, "Jam.Designer.KeyFamilies", JamDesignerKeysTests::Flags)
bool FJamDesignerKeyFamilies::RunTest(const FString& Parameters)
{
	const FString Base = Regular();
	if (!TestFalse(TEXT("hockey.regular loads"), Base.IsEmpty())) return false;

	// The list the C++ mirror reads (generated from validate.mjs) carries every family.
	TArray<FString> Editable;
	for (const TCHAR* P : JamDesignerGameEditable) Editable.Add(P);
	for (const TCHAR* P : { TEXT("shot.one_timer"), TEXT("skater.breakaway"), TEXT("stick.deke"), TEXT("stick.foul_grace_frames"), TEXT("commentary"), TEXT("look.trim_emissive"), TEXT("camera.boards_margin_m"), TEXT("camera.end_height_m"), TEXT("camera.far_net_bias") })
	{
		TestTrue(*FString::Printf(TEXT("%s is Designer-editable"), P), Editable.Contains(P));
	}
	TestFalse(TEXT("camera.mode is not"), Editable.Contains(TEXT("camera.mode")) || Editable.Contains(TEXT("camera")));

	// The system prompt: names every family, is trimmed to the editable game paths, stays under 70 KB.
	const FString& System = FJamDesignerPrompt::GetSystemText();
	for (const FFamily& F : Families) TestTrue(*FString::Printf(TEXT("the prompt explains %s"), F.Id), System.Contains(F.PromptWord));
	TestTrue(TEXT("the prompt names trim_emissive"), System.Contains(TEXT("`look.trim_emissive`")));
	const int32 Bytes = FTCHARToUTF8(*System, System.Len()).Length();
	TestTrue(*FString::Printf(TEXT("system prompt %d bytes is under 70 KB"), Bytes), Bytes < 70 * 1024);
	TestFalse(TEXT("the schema excerpt has no teams (not editable)"), System.Contains(TEXT("\"skaters\": {")));
	TestFalse(TEXT("the schema excerpt has no camera.fov_deg (not editable)"), System.Contains(TEXT("\"fov_deg\": {")));
	TestTrue(TEXT("the schema excerpt keeps camera.end_height_m"), System.Contains(TEXT("\"end_height_m\": {")));
	TestTrue(TEXT("the schema excerpt keeps shot.one_timer"), System.Contains(TEXT("\"one_timer\": {")));
	TestTrue(TEXT("the schema excerpt keeps rules.hooks"), System.Contains(TEXT("\"hooks\": {")));

	for (const FFamily& F : Families)
	{
		FString Err;
		const FString Good = WithGamePath(Base, F.Path, F.Good, Err);
		if (!TestFalse(*FString::Printf(TEXT("%s: mock built (%s)"), F.Id, *Err), Good.IsEmpty())) continue;
		const FJamDesignerParsedReply R = FJamDesignerPrompt::ParseReply(Good, Base);
		TestTrue(*FString::Printf(TEXT("%s: \"%s\" -> %s is a valid cut: %s"), F.Id, F.Sentence, F.Path, *FString::Join(R.Errors, TEXT(" | "))), R.Kind == EJamDesignerReplyKind::Cartridge);
		const TArray<FString> Changed = ChangedPaths(Base, Good);
		TArray<FString> Foreign;
		TestTrue(*FString::Printf(TEXT("%s: only %s changed (%s)"), F.Id, F.Path, *FString::Join(Changed, TEXT(", "))), Changed.Num() > 0 && OnlyInside(Changed, F.Path, Foreign));
		TArray<FString> LoaderErrors;
		TestTrue(*FString::Printf(TEXT("%s: the loader takes it"), F.Id), UJamCartridge::LoadFromString(Good, LoaderErrors, GetTransientPackage()) != nullptr);
		for (const FString& E : LoaderErrors) AddError(E);

		const FString Bad = WithGamePath(Base, F.Path, F.Bad, Err);
		const FJamDesignerParsedReply B = FJamDesignerPrompt::ParseReply(Bad, Base);
		TestTrue(*FString::Printf(TEXT("%s: out of range is rejected"), F.Id), B.Kind == EJamDesignerReplyKind::Invalid);
		TestTrue(*FString::Printf(TEXT("%s: the error points at %s (%s)"), F.Id, F.BadPointer, *FString::Join(B.Errors, TEXT(" | "))),
			B.Errors.ContainsByPredicate([&](const FString& E) { return E.StartsWith(FString(TEXT("at ")) + F.BadPointer); }));
	}

	// Two families in one reply (a sentence may mean both) still pass; the rest of camera, shot, skater and stick stay the host's.
	{
		FString Err;
		FString Both = WithGamePath(Base, TEXT("stick.deke"), TEXT("{\"offset_m\":1.4,\"commit_frames\":28}"), Err);
		Both = WithGamePath(Both, TEXT("stick.foul_grace_frames"), TEXT("60"), Err);
		TestTrue(TEXT("deke + foul grace together"), FJamDesignerPrompt::ParseReply(Both, Base).Kind == EJamDesignerReplyKind::Cartridge);
		FString Cam = WithGamePath(Base, TEXT("camera.boards_margin_m"), TEXT("0.5"), Err);
		Cam = WithGamePath(Cam, TEXT("camera.far_net_bias"), TEXT("0.8"), Err);
		TestTrue(TEXT("margin + far net bias together"), FJamDesignerPrompt::ParseReply(Cam, Base).Kind == EJamDesignerReplyKind::Cartridge);
		const FString Trim = WithGamePath(WithGamePath(Base, TEXT("look.tier"), TEXT("\"p4_broadcast\""), Err), TEXT("look.trim_emissive"), TEXT("60"), Err);
		TestTrue(TEXT("broadcast tier + trim glow together"), FJamDesignerPrompt::ParseReply(Trim, Base).Kind == EJamDesignerReplyKind::Cartridge);
		const TPair<const TCHAR*, const TCHAR*> Protected[] = {
			{ TEXT("camera.fov_deg"), TEXT("90") }, { TEXT("camera.height_m"), TEXT("8") }, { TEXT("camera.mode"), TEXT("\"arena\"") },
			{ TEXT("shot.speed"), TEXT("25") }, { TEXT("shot.charge_speed_bonus"), TEXT("12") }, { TEXT("skater.speed"), TEXT("13") },
			{ TEXT("skater.pass_speed"), TEXT("20") }, { TEXT("stick.reach_m"), TEXT("2") }, { TEXT("look.fog_density"), TEXT("0.05") },
		};
		for (const auto& P : Protected)
		{
			const FJamDesignerParsedReply R = FJamDesignerPrompt::ParseReply(WithGamePath(Base, P.Key, P.Value, Err), Base);
			const FString Ptr = FString(TEXT("at /game/")) + FString(P.Key).Replace(TEXT("."), TEXT("/")) + TEXT(":");
			TArray<FString> Parts;
			FString(P.Key).ParseIntoArray(Parts, TEXT("."));
			const FString Parent = FString(TEXT("at /game/")) + Parts[0] + TEXT(":");   // hockey.regular has no `stick`: the diff is the object itself
			TestTrue(*FString::Printf(TEXT("%s stays protected (%s)"), P.Key, *FString::Join(R.Errors, TEXT(" | "))), R.Kind == EJamDesignerReplyKind::Invalid && R.Errors.ContainsByPredicate([&](const FString& E) { return E.StartsWith(Ptr) || E.StartsWith(Parent); }));
		}
	}

	// Live (opt-in, spends the operator's CLI subscription: at most 21 calls): the seven sentences through the game's own CLI transport.
	if (FPlatformMisc::GetEnvironmentVariable(TEXT("TJ_DESIGNER_CLI_LIVE")) == TEXT("1"))
	{
		UJamDesignerSubsystem* S = NewObject<UJamDesignerSubsystem>(NewObject<UGameInstance>());
		S->SetKeyOverrideForTest(FString());
		S->SetCliForTest(true);
		if (!TestFalse(TEXT("claude on PATH"), S->ResolveCliExe().IsEmpty())) return false;
		for (const FFamily& F : Families)
		{
			FJamDesignerResult Out;
			bool bDone = false;
			S->RequestCut(F.Sentence, Base, [&](const FJamDesignerResult& R) { Out = R; bDone = true; });
			const double Start = FPlatformTime::Seconds();
			while (!bDone && FPlatformTime::Seconds() - Start < 400.0)
			{
				FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);
				FPlatformProcess::Sleep(0.05f);
			}
			if (!TestTrue(*FString::Printf(TEXT("live %s finished"), F.Id), bDone)) continue;
			TestTrue(*FString::Printf(TEXT("live %s: a validated cut (%s)"), F.Id, *Out.Message), Out.HasCartridge());
			if (!Out.HasCartridge()) continue;
			const TArray<FString> Changed = ChangedPaths(Base, Out.CartridgeJson);
			TArray<FString> Foreign;
			const bool bOwn = Changed.ContainsByPredicate([&](const FString& P) { return P.StartsWith(TEXT("/game/") + FString(F.Path).Replace(TEXT("."), TEXT("/"))); });
			TestTrue(*FString::Printf(TEXT("live %s: %s changed"), F.Id, F.Path), bOwn);
			TestTrue(*FString::Printf(TEXT("live %s: nothing outside the family (%s)"), F.Id, *FString::Join(Foreign, TEXT(", "))), OnlyInside(Changed, F.Path, Foreign));
			AddInfo(FString::Printf(TEXT("live %s: outcome=%s attempts=%d tokens_in=%lld tokens_out=%lld ms=%.0f changed=%s"), F.Id,
				*StaticEnum<EJamDesignerOutcome>()->GetNameStringByValue((int64)Out.Outcome), Out.Attempts, Out.TokensIn, Out.TokensOut, Out.Ms, *FString::Join(Changed, TEXT(", "))));
		}
	}
	return true;
}

#endif
