// Lane A15 (C-0005): Jam.Designer.RulesRoundTrip. Ten mock Designer replies that add hooks, variants, zones and obstacles over
// hockey.regular must parse as cartridges; a hook that could end the match before frame 600 must come back as the pointer error.
// The Node twin is schema/rules.test.mjs (DesignerRulesRoundTrip). Run:
//   UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Designer.RulesRoundTrip; Quit" -unattended -nullrhi -nosplash -log
#include "Misc/AutomationTest.h"
#include "JamDesignerPrompt.h"
#include "JamCartridge.h"
#include "JamJson.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamRulesDesignerTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;
	using FJPtr = TSharedPtr<FJamJson>;

	FJPtr Child(const FJPtr& O, const TCHAR* Key) { return O && O->IsObject() ? O->Get(Key) : FJPtr(); }

	// Sets (or replaces) Key under game.<Section> with the parsed Json snippet and returns the compact cartridge text.
	FString WithGameKey(const FString& BaseText, const TCHAR* Section, const TCHAR* Key, const FString& Json, FString& OutErr)
	{
		FJPtr Root = FJamJson::Parse(BaseText, OutErr);
		if (!Root) return FString();
		FJPtr Value = FJamJson::Parse(Json, OutErr);
		if (!Value) return FString();
		FJPtr Sec = Child(Child(Root, TEXT("game")), Section);
		if (!Sec) { OutErr = FString::Printf(TEXT("no game.%s"), Section); return FString(); }
		Sec->Object.RemoveAll([&](const TPair<FString, FJPtr>& P) { return P.Key == Key; });
		Sec->Object.Add(TPair<FString, FJPtr>(FString(Key), Value));
		return Root->ToCompact();
	}
}

using namespace JamRulesDesignerTests;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDesignerRulesRoundTrip, "Jam.Designer.RulesRoundTrip", JamRulesDesignerTests::Flags)
bool FJamDesignerRulesRoundTrip::RunTest(const FString& Parameters)
{
	TArray<FString> Errors;
	UJamCartridge* Base = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")), Errors);
	for (const FString& E : Errors) AddError(E);
	if (!Base) return false;
	const FString& B = Base->RawJson;

	struct FMock { const TCHAR* Sentence; const TCHAR* Section; const TCHAR* Key; const TCHAR* Json; };
	const FMock Mocks[] = {
		{ TEXT("the last goal counts double"), TEXT("rules"), TEXT("hooks"), TEXT("[{\"id\":\"last-goal-double\",\"when\":\"on_goal\",\"team\":\"scoring\",\"score\":{\"at_least\":3},\"then\":[{\"do\":\"add_score\",\"value\":1},{\"do\":\"announce\",\"text\":\"LAST GOAL COUNTS DOUBLE\"}]}]") },
		{ TEXT("the rink shrinks by 10 percent every goal"), TEXT("rules"), TEXT("variants"), TEXT("{\"shrink_per_goal\":0.1}") },
		{ TEXT("put a slow-ice circle at centre"), TEXT("rink"), TEXT("zones"), TEXT("[{\"id\":\"centre-slow\",\"shape\":\"circle\",\"x\":0,\"z\":0,\"r\":6,\"kind\":\"slow_ice\",\"strength\":0.6,\"visible\":true}]") },
		{ TEXT("sudden death after the first minute"), TEXT("rules"), TEXT("hooks"), TEXT("[{\"id\":\"sudden-death\",\"when\":\"on_goal\",\"after_frame\":3600,\"then\":[{\"do\":\"end_match\",\"team\":\"scoring\"},{\"do\":\"announce\",\"text\":\"SUDDEN DEATH\"}]}]") },
		{ TEXT("mercy rule at a four goal lead"), TEXT("rules"), TEXT("variants"), TEXT("{\"mercy_goals\":4}") },
		{ TEXT("swap the nets every minute"), TEXT("rules"), TEXT("hooks"), TEXT("[{\"id\":\"swap-minute\",\"when\":\"on_frame\",\"every_frames\":3600,\"team\":\"any\",\"then\":[{\"do\":\"swap_nets\"},{\"do\":\"announce\",\"text\":\"NETS SWAPPED\"}]}]") },
		{ TEXT("make the ice slippery when someone gets knocked down"), TEXT("rules"), TEXT("hooks"), TEXT("[{\"id\":\"slick-on-knockdown\",\"when\":\"on_knockdown\",\"then\":[{\"do\":\"set_param\",\"param\":\"ice_friction\",\"value\":0.005},{\"do\":\"announce\",\"text\":\"GLASS ICE\"}]}]") },
		{ TEXT("a fast lane down the middle and no shooting from the left corner"), TEXT("rink"), TEXT("zones"), TEXT("[{\"id\":\"fast-lane\",\"shape\":\"rect\",\"x\":0,\"z\":0,\"w\":60,\"d\":6,\"kind\":\"fast_ice\"},{\"id\":\"no-shoot-left\",\"shape\":\"rect\",\"x\":-35,\"z\":0,\"w\":14,\"d\":40,\"kind\":\"no_shoot\",\"visible\":false}]") },
		{ TEXT("bumpers in the corners"), TEXT("rink"), TEXT("obstacles"), TEXT("[{\"kind\":\"bumper\",\"x\":-30,\"z\":-12,\"size\":2},{\"kind\":\"bumper\",\"x\":30,\"z\":12,\"size\":2}]") },
		{ TEXT("goals count double once a team leads by two"), TEXT("rules"), TEXT("hooks"), TEXT("[{\"id\":\"double-points\",\"when\":\"on_goal\",\"score\":{\"lead_at_least\":2},\"then\":[{\"do\":\"set_score_multiplier\",\"team\":\"conceding\",\"value\":2},{\"do\":\"announce\",\"text\":\"COMEBACK: goals count double\"}]}]") },
	};
	for (const FMock& M : Mocks)
	{
		FString Err;
		const FString Text = WithGameKey(B, M.Section, M.Key, M.Json, Err);
		if (Text.IsEmpty()) { AddError(FString::Printf(TEXT("%s: %s"), M.Sentence, *Err)); continue; }
		const FJamDesignerParsedReply R = FJamDesignerPrompt::ParseReply(Text, B);
		TestTrue(*FString::Printf(TEXT("\"%s\" is a valid cut: %s"), M.Sentence, *FString::Join(R.Errors, TEXT(" | "))), R.Kind == EJamDesignerReplyKind::Cartridge);
	}

	// The guard: an unguarded end_match is rejected with the hook's pointer; the guarded forms pass.
	{
		FString Err;
		const FString Early = WithGameKey(B, TEXT("rules"), TEXT("hooks"), TEXT("[{\"id\":\"instant\",\"when\":\"on_goal\",\"then\":[{\"do\":\"end_match\",\"team\":\"scoring\"}]}]"), Err);
		const FJamDesignerParsedReply R = FJamDesignerPrompt::ParseReply(Early, B);
		TestTrue(TEXT("an instant end_match hook is invalid Designer output"), R.Kind == EJamDesignerReplyKind::Invalid);
		TestTrue(TEXT("the error names the hook and the frame"), R.Errors.ContainsByPredicate([](const FString& E) { return E.StartsWith(TEXT("at /game/rules/hooks/0:")) && E.Contains(TEXT("before frame 600")); }));
		TArray<FString> Plain;
		TestTrue(TEXT("but the cartridge itself is valid (a human may author it)"), UJamCartridge::Validate(Early, Plain));

		const FString Tick = WithGameKey(B, TEXT("rules"), TEXT("hooks"), TEXT("[{\"id\":\"tick\",\"when\":\"on_frame\",\"every_frames\":300,\"then\":[{\"do\":\"end_match\"}]}]"), Err);
		TestTrue(TEXT("a 5-second cadence end_match is invalid"), FJamDesignerPrompt::ParseReply(Tick, B).Kind == EJamDesignerReplyKind::Invalid);
		for (const TCHAR* Ok : {
			TEXT("[{\"id\":\"armed\",\"when\":\"on_goal\",\"after_frame\":600,\"then\":[{\"do\":\"end_match\",\"team\":\"scoring\"}]}]"),
			TEXT("[{\"id\":\"two\",\"when\":\"on_goal\",\"score\":{\"at_least\":2},\"then\":[{\"do\":\"end_match\",\"team\":\"scoring\"}]}]"),
			TEXT("[{\"id\":\"p\",\"when\":\"on_period_end\",\"then\":[{\"do\":\"end_match\"}]}]"),
			TEXT("[{\"id\":\"slow\",\"when\":\"on_frame\",\"every_frames\":600,\"then\":[{\"do\":\"end_match\"}]}]") })
		{
			const FString T = WithGameKey(B, TEXT("rules"), TEXT("hooks"), Ok, Err);
			const FJamDesignerParsedReply G = FJamDesignerPrompt::ParseReply(T, B);
			TestTrue(*FString::Printf(TEXT("guarded hook passes: %s | %s"), Ok, *FString::Join(G.Errors, TEXT(" | "))), G.Kind == EJamDesignerReplyKind::Cartridge);
		}
		const FString SD = WithGameKey(B, TEXT("rules"), TEXT("variants"), TEXT("{\"sudden_death\":true}"), Err);
		TestTrue(TEXT("the sudden_death variant is always allowed (armed at 600 by the loader)"), FJamDesignerPrompt::ParseReply(SD, B).Kind == EJamDesignerReplyKind::Cartridge);
	}

	// Still immutable: roles and the camera while editing hooks.
	{
		FString Err;
		FString T = WithGameKey(B, TEXT("rules"), TEXT("hooks"), TEXT("[{\"id\":\"x\",\"when\":\"on_goal\",\"then\":[{\"do\":\"swap_nets\"}]}]"), Err);
		T.ReplaceInline(TEXT("\"mode\":\"third_person\""), TEXT("\"mode\":\"arena\""));
		const FJamDesignerParsedReply R = FJamDesignerPrompt::ParseReply(T, B);
		TestTrue(TEXT("camera change still rejected"), R.Kind == EJamDesignerReplyKind::Invalid && R.Errors.ContainsByPredicate([](const FString& E) { return E.StartsWith(TEXT("at /game/camera/mode:")); }));
	}
	return true;
}

#endif
