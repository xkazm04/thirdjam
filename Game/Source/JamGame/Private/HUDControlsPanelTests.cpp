// Lane A222 (track U3; B-to-A-0044): the controls panel (HUDControlsPanel.cpp). The operator, 15 Nov 10:50: "Improve visually panel to show key
// bindings for player and keep it visible throughout the game, switchable in settings to take off permanently".
//   Jam.Hud.ControlsPanelRows     the rows by possession from the live bindings (with: SHOOT PASS DEKE SPIN; without: CROSS-CHECK CHECK POKE; always:
//                                 SKATE SPRINT BRAKE, ABILITY while he holds one), 6 to 8 of them; the row whose button is down lights; SAUCER / BRACED
//   Jam.Hud.ControlsPanelPad      the pad's names when the pad was touched last, the keys' when the keys were
//   Jam.Hud.ControlsPanelRebind   a rebind shows on the row (the pair splits as the CONTROLS screen splits it)
//   Jam.Hud.ControlsPanelOff      SETTINGS' CONTROLS PANEL: ON by default, OFF survives a save / load round trip (the test profile, never the player's
//                                 file), the row toggles and saves, OFF hides the panel, the strip and the pill; jam.hud.ControlsPanel 0 = the screen of before
//   Jam.Hud.ControlsPanelHidden   shown while play runs; hidden under the pause card (or a paused world), the GOAL banner and the end card; the swap
//                                 fades the possession half in over 6 frames
//   Jam.Hud.ControlsPanelHold    wave 2 (the Director, 13:26): the half does not blink: a possession flicker (2 frames each way for 60 frames)
//                                 draws at most one swap; a clean loss swaps 12 frames on; a press while without swaps on its frame; gaining at once
//   Jam.Hud.ControlsPanelClear    the panel's box at 1920 x 1080 and 1280 x 720 (at most 300 x ... px at 1080, in proportion at 720) meets none of the
//                                 radar's, the score's, the ability slot's, the commentary's, the stamina bar's or the call line's boxes; long key names fit
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "HAL/IConsoleManager.h"
#include "HUDWidget.h"
#include "InputMapping.h"
#include "MenuStyle.h"
#include "MenuWidget.h"
#include "SettingsWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/ScopeExit.h"
#include "Rendering/SlateRenderer.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamPanelTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;
	using W = UJamHudWidget;

	struct FRestore
	{
		FJamBindings Was = UJamInputMap::Get()->GetBindings();
		bool bPad = UJamInputMap::IsPadLast();
		bool bSetting = UJamHudWidget::IsControlsPanelSettingOn();
		~FRestore() { UJamInputMap::Get()->ApplyBindings(Was); UJamInputMap::NoteInputDevice(bPad); UJamHudWidget::SetControlsPanelSetting(bSetting); }
	};

	FString Line(const TArray<W::FControlsPanelRow>& Rows)
	{
		TArray<FString> P;
		for (const W::FControlsPanelRow& R : Rows) P.Add(FString::Printf(TEXT("%s %s%s"), *R.Key, *R.Word, R.bLit ? TEXT("*") : TEXT("")));
		return FString::Join(P, TEXT(" | "));
	}

	// A HUD in a test world (no game mode: ApplyControlsPanel takes the frame's facts straight in).
	struct FHud
	{
		UWorld* World = nullptr;
		UJamHudWidget* Hud = nullptr;
		FHud()
		{
			if (!GEngine || !FSlateApplication::IsInitialized()) return;
			World = UWorld::CreateWorld(EWorldType::Game, false, TEXT("JamPanelTestWorld"));
			if (!World) return;
			GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
			Hud = CreateWidget<UJamHudWidget>(World, UJamHudWidget::StaticClass());
			if (Hud) { Hud->AddToRoot(); Hud->TakeWidget(); }
		}
		~FHud()
		{
			if (Hud) { Hud->ReleaseSlateResources(true); Hud->RemoveFromRoot(); }
			if (World) { GEngine->DestroyWorldContext(World); World->DestroyWorld(false); }
		}
	};
}

using namespace JamPanelTests;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudControlsPanelRows, "Jam.Hud.ControlsPanelRows", JamPanelTests::Flags)
bool FJamHudControlsPanelRows::RunTest(const FString&)
{
	const FJamBindings D = FJamBindings::Defaults();
	const W::FControlsPanelHeld None;
	const FString With = Line(W::ControlsPanelRows(D, false, true, false, None));
	const FString Without = Line(W::ControlsPanelRows(D, false, false, false, None));
	AddInfo(FString::Printf(TEXT("JAMHUD panel with: %s"), *With));
	AddInfo(FString::Printf(TEXT("JAMHUD panel without: %s"), *Without));
	TestEqual(TEXT("with the puck"), With, FString(TEXT("LMB SHOOT | RMB PASS | WHEEL DEKE | E SPIN | WASD SKATE | SHIFT SPRINT | CTRL BRAKE")));
	TestEqual(TEXT("without it"), Without, FString(TEXT("LMB CROSS-CHECK | RMB CHECK | SPACE POKE | WASD SKATE | SHIFT SPRINT | CTRL BRAKE")));
	TestEqual(TEXT("an ability held: its row last"), Line(W::ControlsPanelRows(D, false, true, true, None)), With + TEXT(" | F ABILITY"));
	for (const bool bWith : { true, false })
		for (const bool bAb : { true, false })
		{
			const TArray<W::FControlsPanelRow> R = W::ControlsPanelRows(D, false, bWith, bAb, None);
			TestTrue(*FString::Printf(TEXT("six to eight rows (with=%d ability=%d: %d)"), bWith, bAb, R.Num()), R.Num() >= 6 && R.Num() <= W::ControlsPanelMaxRows);
			int32 Poss = 0;
			for (const W::FControlsPanelRow& X : R) Poss += X.bPossession ? 1 : 0;
			TestTrue(TEXT("the possession half fits its four slots"), Poss <= 4 && R.Num() - Poss <= 4);
		}
	// The row whose button is down lights; the held words are the pill's (SAUCER at a saucer's hold, BRACED at a brace's).
	W::FControlsPanelHeld H;
	H.Set(EJamVerb::Pass);
	TestEqual(TEXT("RMB held with the puck: PASS lights"), Line(W::ControlsPanelRows(D, false, true, false, H)), FString(TEXT("LMB SHOOT | RMB PASS* | WHEEL DEKE | E SPIN | WASD SKATE | SHIFT SPRINT | CTRL BRAKE")));
	H.bSaucer = true;
	TestTrue(TEXT("held to a saucer: the word is SAUCER"), Line(W::ControlsPanelRows(D, false, true, false, H)).Contains(TEXT("RMB SAUCER*")));
	W::FControlsPanelHeld C;
	C.Set(EJamVerb::CrossCheck);
	C.Set(EJamVerb::Sprint);
	C.Set(EJamVerb::SkateForward);
	TestEqual(TEXT("LMB held without the puck, skating, sprinting"), Line(W::ControlsPanelRows(D, false, false, false, C)), FString(TEXT("LMB CROSS-CHECK* | RMB CHECK | SPACE POKE | WASD SKATE* | SHIFT SPRINT* | CTRL BRAKE")));
	C.bBraced = true;
	TestTrue(TEXT("held to a brace: the word is BRACED"), Line(W::ControlsPanelRows(D, false, false, false, C)).Contains(TEXT("LMB BRACED*")));
	TestFalse(TEXT("a held cross-check does not light the puck's half"), Line(W::ControlsPanelRows(D, false, true, false, C)).Contains(TEXT("LMB SHOOT*")));
	// The live rows agree with the pill's words on the two-role buttons (the pause card keeps the pill's line).
	FRestore Restore;
	UJamInputMap::Get()->ApplyBindings(D);
	UJamInputMap::NoteInputDevice(false);
	TestTrue(TEXT("the pill's line names the panel's first two rows with the puck"), UJamHudWidget::ButtonsBadgeLine(true) == TEXT("LMB  SHOOT     RMB  PASS"));
	TestTrue(TEXT("the pill's line names the panel's first two rows without"), UJamHudWidget::ButtonsBadgeLine(false) == TEXT("LMB  CROSS-CHECK     RMB  CHECK"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudControlsPanelPad, "Jam.Hud.ControlsPanelPad", JamPanelTests::Flags)
bool FJamHudControlsPanelPad::RunTest(const FString&)
{
	const FJamBindings D = FJamBindings::Defaults();
	const W::FControlsPanelHeld None;
	TestEqual(TEXT("the pad with the puck"), Line(W::ControlsPanelRows(D, true, true, true, None)), FString(TEXT("RT SHOOT | RB PASS | Y DEKE | B SPIN | L-STICK SKATE | LT SPRINT | L3 BRAKE | R3 ABILITY")));
	TestEqual(TEXT("the pad without it"), Line(W::ControlsPanelRows(D, true, false, false, None)), FString(TEXT("RT CROSS-CHECK | RB CHECK | A POKE | L-STICK SKATE | LT SPRINT | L3 BRAKE")));
	// The live widget follows the device touched last (a pad press, then a mouse press: the resolver's own note).
	FRestore Restore;
	UJamInputMap::Get()->ApplyBindings(D);
	UJamHudWidget::SetControlsPanelSetting(true);
	FHud F;
	if (!F.Hud) { AddInfo(TEXT("no Slate: the widget part skipped")); return true; }
	UJamPlayerInput* Pi = NewObject<UJamPlayerInput>();
	Pi->DebugButton(EKeys::Gamepad_RightShoulder, true);
	Pi->DebugButton(EKeys::Gamepad_RightShoulder, false);
	W::FControlsPanelInput In;
	In.bWithPuck = true;
	F.Hud->ApplyControlsPanel(In);
	TestTrue(TEXT("after a pad press the panel says RT SHOOT"), Line(F.Hud->GetControlsPanelRowsShown()).StartsWith(TEXT("RT SHOOT | RB PASS")));
	Pi->DebugButton(EKeys::LeftMouseButton, true);
	Pi->DebugButton(EKeys::LeftMouseButton, false);
	In.SimFrame = 1;
	F.Hud->ApplyControlsPanel(In);
	TestTrue(TEXT("after a mouse press the panel says LMB SHOOT"), Line(F.Hud->GetControlsPanelRowsShown()).StartsWith(TEXT("LMB SHOOT | RMB PASS")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudControlsPanelRebind, "Jam.Hud.ControlsPanelRebind", JamPanelTests::Flags)
bool FJamHudControlsPanelRebind::RunTest(const FString&)
{
	FRestore Restore;
	FJamBindings B = FJamBindings::Defaults();
	TestTrue(TEXT("SHOOT onto G (its partner keeps LMB)"), B.Rebind(EJamVerb::Shoot, EKeys::G));
	TestTrue(TEXT("SPIN onto R-SHIFT"), B.Rebind(EJamVerb::Spin, EKeys::RightShift));
	TestTrue(TEXT("SKATE FORWARD onto I"), B.Rebind(EJamVerb::SkateForward, EKeys::I));
	UJamInputMap::Get()->ApplyBindings(B);
	UJamInputMap::NoteInputDevice(false);
	const W::FControlsPanelHeld None;
	const FString With = Line(W::ControlsPanelRows(UJamInputMap::Get()->GetBindings(), false, true, false, None));
	const FString Without = Line(W::ControlsPanelRows(UJamInputMap::Get()->GetBindings(), false, false, false, None));
	AddInfo(FString::Printf(TEXT("JAMHUD panel rebound with: %s / without: %s"), *With, *Without));
	TestTrue(TEXT("the rebound shot reads G"), With.StartsWith(TEXT("G SHOOT")));
	TestTrue(TEXT("the rebound spin reads R-SHIFT"), With.Contains(TEXT("R-SHIFT SPIN")));
	TestTrue(TEXT("the skate's keys read as rebound"), With.Contains(TEXT("IASD SKATE")));
	TestTrue(TEXT("the pair split: the cross-check keeps LMB"), Without.StartsWith(TEXT("LMB CROSS-CHECK")));
	// The widget reads the bindings as they are bound now, on its next frame.
	UJamHudWidget::SetControlsPanelSetting(true);
	FHud F;
	if (!F.Hud) { AddInfo(TEXT("no Slate: the widget part skipped")); return true; }
	W::FControlsPanelInput In;
	In.bWithPuck = true;
	F.Hud->ApplyControlsPanel(In);
	TestTrue(TEXT("the panel shows G SHOOT"), Line(F.Hud->GetControlsPanelRowsShown()).StartsWith(TEXT("G SHOOT")));
	UJamInputMap::Get()->ApplyBindings(FJamBindings::Defaults());
	In.SimFrame = 1;
	F.Hud->ApplyControlsPanel(In);
	TestTrue(TEXT("a reset shows on the next frame: LMB SHOOT"), Line(F.Hud->GetControlsPanelRowsShown()).StartsWith(TEXT("LMB SHOOT")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudControlsPanelOff, "Jam.Hud.ControlsPanelOff", JamPanelTests::Flags)
bool FJamHudControlsPanelOff::RunTest(const FString&)
{
	FRestore Restore;
	// The saved form: ON by default, OFF round-trips, a profile saved before the key existed reads ON.
	TestTrue(TEXT("ON by default"), FJamSettings().bControlsPanel);
	FJamSettings Off;
	Off.bControlsPanel = false;
	TestEqual(TEXT("saved as ControlsPanel=False"), Off.ToSaved().FindRef(TEXT("ControlsPanel")), FString(TEXT("False")));
	TestFalse(TEXT("OFF survives the round trip (a restart)"), FJamSettings::FromSaved(Off.ToSaved()).bControlsPanel);
	TestTrue(TEXT("ON survives the round trip"), FJamSettings::FromSaved(FJamSettings().ToSaved()).bControlsPanel);
	TMap<FString, FString> Old = FJamSettings().ToSaved();
	Old.Remove(TEXT("ControlsPanel"));
	TestTrue(TEXT("a profile from before the key reads ON"), FJamSettings::FromSaved(Old).bControlsPanel);
	// Save and Load through the test profile (FJamSettings::SetOverrideForTest: the player's file is never read or written).
	const FJamSettings Fresh;
	FJamSettings::SetOverrideForTest(&Fresh);
	ON_SCOPE_EXIT { FJamSettings::SetOverrideForTest(nullptr); };
	FJamSettings St = FJamSettings::Load();
	St.bControlsPanel = false;
	St.Save();
	TestFalse(TEXT("OFF saved and loaded"), FJamSettings::Load().bControlsPanel);
	TestFalse(TEXT("OFF loaded and saved again is still OFF (the level's save keeps it)"), FJamSettings::FromSaved(FJamSettings::Load().ToSaved()).bControlsPanel);
	FJamSettings::Load().Apply();
	TestFalse(TEXT("Apply() takes it to the HUD"), UJamHudWidget::IsControlsPanelSettingOn());
	// The legend: the panel; OFF = nothing (no strip, no pill either); the screen of before only with jam.hud.ControlsPanel 0.
	TestTrue(TEXT("rule on, setting on: the panel"), W::LegendFor(true, true) == W::ELegend::Panel);
	TestTrue(TEXT("rule on, setting off: no legend at all"), W::LegendFor(true, false) == W::ELegend::None);
	TestTrue(TEXT("the neutral value: the strip and the pill of before"), W::LegendFor(false, true) == W::ELegend::StripAndPill && W::LegendFor(false, false) == W::ELegend::StripAndPill);
	IConsoleVariable* Rule = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.hud.ControlsPanel"));
	TestNotNull(TEXT("jam.hud.ControlsPanel exists"), Rule);
	TestEqual(TEXT("ships: the panel rule on"), Rule ? Rule->GetInt() : -1, 1);
	TestTrue(TEXT("with OFF the legend now is none"), W::LegendNow() == W::ELegend::None);
	// The SETTINGS row: CONTROLS PANEL, its label ON / OFF, a press toggles, saves and applies on the press.
	UJamHudWidget::SetControlsPanelSetting(true);
	FJamSettings::SetOverrideForTest(&Fresh);
	FHud F;
	if (!F.Hud) { AddInfo(TEXT("no Slate: the widget part skipped")); return true; }
	UJamSettingsWidget* S = CreateWidget<UJamSettingsWidget>(F.World, UJamSettingsWidget::StaticClass());
	if (!TestNotNull(TEXT("SETTINGS builds"), S)) return false;
	S->AddToRoot();
	ON_SCOPE_EXIT { S->ReleaseSlateResources(true); S->RemoveFromRoot(); };
	S->TakeWidget();
	S->Refresh();
	TestNotNull(TEXT("SETTINGS has the CONTROLS PANEL row"), S->GetControlsPanelButtonForTest());
	TestTrue(TEXT("the pad walk reaches it"), S->GetFocusables().Contains(S->GetControlsPanelButtonForTest()));
	TestEqual(TEXT("it reads ON"), S->GetControlsPanelLabelForTest(), FString(TEXT("ON")));
	W::FControlsPanelInput In;
	In.bWithPuck = true;
	F.Hud->ApplyControlsPanel(In);
	TestTrue(TEXT("ON: the panel shows"), F.Hud->IsControlsPanelShown());
	S->PressControlsPanelForTest();
	TestEqual(TEXT("pressed: it reads OFF"), S->GetControlsPanelLabelForTest(), FString(TEXT("OFF")));
	TestFalse(TEXT("pressed: saved OFF"), FJamSettings::Load().bControlsPanel);
	In.SimFrame = 1;
	F.Hud->ApplyControlsPanel(In);
	TestFalse(TEXT("OFF: the panel is gone on the next frame"), F.Hud->IsControlsPanelShown());
	TestTrue(TEXT("OFF: no strip either (the strip's rule asks for the screen of before)"), W::LegendNow() != W::ELegend::StripAndPill);
	TestFalse(TEXT("OFF: no pill (the pill shows only in the screen of before)"), F.Hud->GetButtonsBadgeShown().Len() > 0);
	S->PressControlsPanelForTest();
	TestTrue(TEXT("pressed again: ON and saved"), FJamSettings::Load().bControlsPanel && S->GetControlsPanelLabelForTest() == TEXT("ON"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudControlsPanelHidden, "Jam.Hud.ControlsPanelHidden", JamPanelTests::Flags)
bool FJamHudControlsPanelHidden::RunTest(const FString&)
{
	TestTrue(TEXT("play: shown"), W::ShowControlsPanel(EJamPhase::Play, false, false));
	TestFalse(TEXT("the pause card (or a paused world): hidden"), W::ShowControlsPanel(EJamPhase::Play, true, false));
	TestFalse(TEXT("the paused phase: hidden"), W::ShowControlsPanel(EJamPhase::Paused, false, false));
	TestFalse(TEXT("the GOAL banner: hidden"), W::ShowControlsPanel(EJamPhase::Play, false, true));
	TestFalse(TEXT("the goal pause: hidden"), W::ShowControlsPanel(EJamPhase::GoalPause, false, false));
	TestFalse(TEXT("the end card (a win): hidden"), W::ShowControlsPanel(EJamPhase::Win, false, false));
	TestFalse(TEXT("the end card (a loss): hidden"), W::ShowControlsPanel(EJamPhase::Lose, false, false));
	FRestore Restore;
	UJamInputMap::Get()->ApplyBindings(FJamBindings::Defaults());
	UJamHudWidget::SetControlsPanelSetting(true);
	FHud F;
	if (!F.Hud) { AddInfo(TEXT("no Slate: the widget part skipped")); return true; }
	W::FControlsPanelInput In;
	const TPair<EJamPhase, const TCHAR*> Phases[] = { { EJamPhase::Paused, TEXT("paused") }, { EJamPhase::GoalPause, TEXT("goal") }, { EJamPhase::Win, TEXT("win") }, { EJamPhase::Lose, TEXT("lose") } };
	for (const TPair<EJamPhase, const TCHAR*>& P : Phases)
	{
		In.Phase = P.Key;
		F.Hud->ApplyControlsPanel(In);
		TestFalse(*FString::Printf(TEXT("the widget is hidden in %s"), P.Value), F.Hud->IsControlsPanelShown());
	}
	In.Phase = EJamPhase::Play;
	In.bPauseCardOrPaused = true;
	F.Hud->ApplyControlsPanel(In);
	TestFalse(TEXT("the widget is hidden under the pause card"), F.Hud->IsControlsPanelShown());
	In.bPauseCardOrPaused = false;
	In.bGoalBannerUp = true;
	F.Hud->ApplyControlsPanel(In);
	TestFalse(TEXT("the widget is hidden under the GOAL banner"), F.Hud->IsControlsPanelShown());
	In.bGoalBannerUp = false;
	// The swap: frame 100 with the puck, frame 101 without and LMB pressed (wave 2: a loss swaps at once only under a press, else after
	// jam.hud.PanelHoldFrames; Jam.Hud.ControlsPanelHold holds that rule): the possession half swaps on 101 and fades in over 6 frames.
	In.bWithPuck = true;
	for (int64 Fr = 90; Fr <= 100; ++Fr) { In.SimFrame = Fr; F.Hud->ApplyControlsPanel(In); }
	TestTrue(TEXT("shown in play"), F.Hud->IsControlsPanelShown());
	TestEqual(TEXT("settled with the puck: full"), F.Hud->GetControlsPanelSwapOpacity(), 1.f);
	In.bWithPuck = false;
	In.bPress = true;
	TArray<float> Fade;
	for (int64 Fr = 101; Fr <= 108; ++Fr)
	{
		In.SimFrame = Fr;
		F.Hud->ApplyControlsPanel(In);
		Fade.Add(F.Hud->GetControlsPanelSwapOpacity());
		if (Fr == 101) TestTrue(TEXT("the rows swap on the frame possession changes"), Line(F.Hud->GetControlsPanelRowsShown()).StartsWith(TEXT("LMB CROSS-CHECK")));
	}
	AddInfo(FString::Printf(TEXT("JAMHUD panel fade %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f"), Fade[0], Fade[1], Fade[2], Fade[3], Fade[4], Fade[5], Fade[6], Fade[7]));
	TestTrue(TEXT("the swap's first frame is faint"), Fade[0] > 0.f && Fade[0] < 0.5f);
	TestEqual(TEXT("full by the sixth frame (a fade of at most 6 frames)"), Fade[5], 1.f);
	for (int32 I = 1; I < Fade.Num(); ++I) TestTrue(TEXT("the fade only rises"), Fade[I] >= Fade[I - 1]);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudControlsPanelClear, "Jam.Hud.ControlsPanelClear", JamPanelTests::Flags)
bool FJamHudControlsPanelClear::RunTest(const FString&)
{
	FRestore Restore;
	UJamHudWidget::SetControlsPanelSetting(true);
	UJamInputMap::NoteInputDevice(false);
	FHud F;
	if (!F.Hud) { AddInfo(TEXT("no Slate: skipped")); return true; }
	TSharedRef<SWidget> Slate = F.Hud->TakeWidget();
	const FVector2D View(1920.f, 1080.f);   // 1280 x 720 and 1920 x 1080 are both 1920 x 1080 layout units (Jam.Menu.FitsTheScreen holds the DPI curve)
	// The largest panel: eight rows (with the puck and an ability), the defaults, then long rebound names.
	auto Measure = [&](const TCHAR* What) -> FBox2D
	{
		W::FControlsPanelInput In;
		In.bWithPuck = true;
		In.bAbility = true;
		static int64 Fr = 0;
		In.SimFrame = ++Fr;
		F.Hud->ApplyControlsPanel(In);
		In.bWithPuck = false;
		In.SimFrame = ++Fr;
		F.Hud->ApplyControlsPanel(In);
		Slate->SlatePrepass(1.f);
		const FBox2D Without = F.Hud->ControlsPanelBox(View);
		In.bWithPuck = true;
		In.SimFrame = ++Fr;
		F.Hud->ApplyControlsPanel(In);
		Slate->SlatePrepass(1.f);
		const FBox2D Box = F.Hud->ControlsPanelBox(View);
		AddInfo(FString::Printf(TEXT("JAMHUD panel box %s with the puck: x %.0f..%.0f y %.0f..%.0f (%.0f x %.0f units); without: %.0f x %.0f"), What,
			Box.Min.X, Box.Max.X, Box.Min.Y, Box.Max.Y, Box.GetSize().X, Box.GetSize().Y, Without.GetSize().X, Without.GetSize().Y));
		return Box;
	};
	UJamInputMap::Get()->ApplyBindings(FJamBindings::Defaults());
	const FBox2D Box = Measure(TEXT("defaults"));
	TestTrue(TEXT("the panel is built and measured"), Box.bIsValid && Box.GetSize().X > 100.f && Box.GetSize().Y > 100.f);
	TestTrue(*FString::Printf(TEXT("at most 300 units wide (%.0f)"), Box.GetSize().X), Box.GetSize().X <= W::ControlsPanelMaxW + 0.5f);
	TestTrue(*FString::Printf(TEXT("one width with the puck and without (%.0f)"), Box.GetSize().X), FMath::IsNearlyEqual(Box.GetSize().X, static_cast<double>(W::ControlsPanelMinW), 0.5));
	TestTrue(TEXT("on the safe margin, bottom left"), FMath::IsNearlyEqual(Box.Min.X, static_cast<double>(FJamUiStyle::EdgePx), 0.5) && FMath::IsNearlyEqual(Box.Max.Y, View.Y - static_cast<double>(FJamUiStyle::EdgePx), 0.5));
	TestTrue(*FString::Printf(TEXT("under half the screen's height (%.0f)"), Box.GetSize().Y), Box.GetSize().Y < 0.4f * View.Y);
	TestTrue(*FString::Printf(TEXT("the tallest panel inside the height kept for it (%.0f of %.0f): the hint over it never lies on it"), Box.GetSize().Y, W::ControlsPanelReserveH), Box.GetSize().Y <= W::ControlsPanelReserveH);
	for (const TPair<FString, FBox2D>& C : F.Hud->CornerBoxesForTest(View))
	{
		if (C.Key.StartsWith(TEXT("old-"))) continue;
		if (!C.Value.bIsValid || C.Value.GetSize().X <= 0.f) { AddInfo(FString::Printf(TEXT("JAMHUD %s: not measured (collapsed in a test world)"), *C.Key)); continue; }
		const bool bMeet = Box.Intersect(C.Value);
		AddInfo(FString::Printf(TEXT("JAMHUD %s box x %.0f..%.0f y %.0f..%.0f"), *C.Key, C.Value.Min.X, C.Value.Max.X, C.Value.Min.Y, C.Value.Max.Y));
		TestFalse(*FString::Printf(TEXT("the panel does not meet the %s (both sizes: one layout)"), *C.Key), bMeet);
	}
	// What the radar, the score and the slot take at their largest, by the HUD's own constants (collapsed widgets measure nothing in a test world).
	const FBox2D Radar(FVector2D(View.X - FJamUiStyle::EdgePx - 216.f, View.Y - FJamUiStyle::EdgePx - 116.f), FVector2D(View.X - FJamUiStyle::EdgePx, View.Y - FJamUiStyle::EdgePx));
	const FBox2D Slot(FVector2D(View.X - FJamUiStyle::EdgePx - 380.f, View.Y - FJamUiStyle::EdgePx - 100.f - 30.f - 46.f), FVector2D(View.X - FJamUiStyle::EdgePx, View.Y - FJamUiStyle::EdgePx - 100.f - 30.f));
	const FBox2D Score(FVector2D(View.X * 0.5f - 400.f, 0.f), FVector2D(View.X * 0.5f + 400.f, 160.f));
	const FBox2D Commentary(FVector2D(View.X * 0.5f - 470.f, View.Y - 200.f), FVector2D(View.X * 0.5f + 470.f, View.Y - 24.f));
	TestFalse(TEXT("clear of the radar's frame"), Box.Intersect(Radar));
	TestFalse(TEXT("clear of the ability slot"), Box.Intersect(Slot));
	TestFalse(TEXT("clear of the score"), Box.Intersect(Score));
	TestFalse(TEXT("clear of the commentary strip at its widest (880 + its pad, centred), docked or stepped up"), Box.Intersect(Commentary));
	// The pixels at both sizes, and the ice it covers against the old strip and pill together.
	for (const float Scale : { 1.f, 2.f / 3.f })
		AddInfo(FString::Printf(TEXT("JAMHUD panel at %.0f x %.0f: x %.0f..%.0f y %.0f..%.0f px (%.0f x %.0f px)"), View.X * Scale, View.Y * Scale,
			Box.Min.X * Scale, Box.Max.X * Scale, Box.Min.Y * Scale, Box.Max.Y * Scale, Box.GetSize().X * Scale, Box.GetSize().Y * Scale));
	const TSharedRef<FSlateFontMeasure> FM = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	const FVector2D PillText = FM->Measure(TEXT("LMB  CROSS-CHECK     RMB  CHECK"), FJamUiStyle::Font(FJamUiStyle::Body - 2, FName("Bold"), 20));
	const double PillArea = (PillText.X + 28.0) * (PillText.Y + 10.0), StripArea = (View.X - FJamUiStyle::EdgePx - UJamHudWidget::ControlsRightInsetPx) * 44.0;
	AddInfo(FString::Printf(TEXT("JAMHUD ice covered: the panel %.0f units^2 (all match); the old strip %.0f (8 s, first match) + the pill %.0f (all match, %.0f x %.0f) = %.0f"),
		Box.GetArea(), StripArea, PillArea, PillText.X + 28.0, PillText.Y + 10.0, StripArea + PillArea));
	// Long names: the widest keys a rebind can give the rows still fit the 300.
	FJamBindings Lg = FJamBindings::Defaults();
	Lg.Rebind(EJamVerb::Shoot, EKeys::BackSpace);
	Lg.Rebind(EJamVerb::Spin, EKeys::RightControl);
	Lg.Rebind(EJamVerb::Brake, EKeys::Insert);
	Lg.Rebind(EJamVerb::Ability, EKeys::PageDown);
	UJamInputMap::Get()->ApplyBindings(Lg);
	const FBox2D Long = Measure(TEXT("long names"));
	TestTrue(*FString::Printf(TEXT("long names: at most 300 units wide (%.0f)"), Long.GetSize().X), Long.GetSize().X <= W::ControlsPanelMaxW + 0.5f);
	for (const W::FControlsPanelRow& R : F.Hud->GetControlsPanelRowsShown())
	{
		const float Cap = static_cast<float>(FM->Measure(R.Key, FJamMenuLook::Font(FJamMenuLook::Caption, FName("Black"), 40)).X) + 2.f * FJamMenuLook::Unit;
		const float Word = static_cast<float>(FM->Measure(R.Word, FJamMenuLook::Font(FJamMenuLook::Caption + 2, FName("Bold"), FJamMenuLook::TrackLabel)).X);
		const float RowW = FMath::Max(Cap, W::ControlsPanelKeycapMinW) + 10.f + Word + 2.f * W::ControlsPanelPad;
		TestTrue(*FString::Printf(TEXT("the row '%s %s' fits whole (%.0f of %.0f)"), *R.Key, *R.Word, RowW, W::ControlsPanelMaxW), RowW <= W::ControlsPanelMaxW);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudControlsPanelHold, "Jam.Hud.ControlsPanelHold", JamPanelTests::Flags)
bool FJamHudControlsPanelHold::RunTest(const FString&)
{
	IConsoleVariable* V = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.hud.PanelHoldFrames"));
	TestNotNull(TEXT("jam.hud.PanelHoldFrames exists"), V);
	TestEqual(TEXT("ships at 12 frames"), V ? V->GetInt() : -1, 12);
	const int32 Hold = 12;
	// A scripted flicker: with / without alternating every 2 frames for 60 frames, after a settled half of either kind.
	for (const bool bSettledWith : { true, false })
	{
		W::FControlsPanelSwap S;
		for (int64 F = 0; F < 100; ++F) S.Step(bSettledWith, false, F, Hold);
		int32 Swaps = 0;
		for (int64 F = 100; F < 160; ++F) Swaps += S.Step(((F - 100) / 2) % 2 == (bSettledWith ? 1 : 0), false, F, Hold) ? 1 : 0;
		AddInfo(FString::Printf(TEXT("JAMHUD panel flicker after a settled %s half: %d swaps in 60 frames"), bSettledWith ? TEXT("WITH") : TEXT("WITHOUT"), Swaps));
		TestTrue(*FString::Printf(TEXT("a flicker after a settled %s half draws at most one swap (%d)"), bSettledWith ? TEXT("WITH") : TEXT("WITHOUT"), Swaps), Swaps <= 1);
		W::FControlsPanelSwap N;
		int32 NeutralSwaps = 0;
		for (int64 F = 0; F < 100; ++F) N.Step(bSettledWith, false, F, 0);
		for (int64 F = 100; F < 160; ++F) NeutralSwaps += N.Step(((F - 100) / 2) % 2 == (bSettledWith ? 1 : 0), false, F, 0) ? 1 : 0;
		TestTrue(*FString::Printf(TEXT("the neutral value (0) follows every flip, as PANEL READY (%d swaps)"), NeutralSwaps), NeutralSwaps >= 29);
	}
	// A clean loss at frame 100: the half swaps at 112, not before.
	{
		W::FControlsPanelSwap S;
		for (int64 F = 0; F < 100; ++F) S.Step(true, false, F, Hold);
		int64 At = -1;
		for (int64 F = 100; F < 140 && At < 0; ++F) if (S.Step(false, false, F, Hold)) At = F;
		TestEqual(TEXT("a clean loss swaps 12 frames on (frame 112)"), At, static_cast<int64>(112));
	}
	// A press while without: the swap is on the press's frame.
	{
		W::FControlsPanelSwap S;
		for (int64 F = 0; F < 100; ++F) S.Step(true, false, F, Hold);
		int64 At = -1;
		for (int64 F = 100; F < 140 && At < 0; ++F) if (S.Step(false, F == 103, F, Hold)) At = F;
		TestEqual(TEXT("an LMB / RMB press while without swaps on its frame (103)"), At, static_cast<int64>(103));
		TestEqual(TEXT("then shows WITHOUT"), S.Shown, 0);
		// Regaining within the hold: no swap without a press; with a press at once.
		TestFalse(TEXT("regaining 2 frames later: held"), S.Step(true, false, 105, Hold));
		TestTrue(TEXT("a press on the puck forces WITH"), S.Step(true, true, 106, Hold) && S.Shown == 1);
	}
	// Gaining after a settled WITHOUT: at once.
	{
		W::FControlsPanelSwap S;
		for (int64 F = 0; F < 100; ++F) S.Step(false, false, F, Hold);
		TestTrue(TEXT("gaining swaps on that frame"), S.Step(true, false, 100, Hold) && S.Shown == 1);
	}
	// A restart (the frame count going back) starts over on the truth.
	{
		W::FControlsPanelSwap S;
		for (int64 F = 0; F < 100; ++F) S.Step(true, false, F, Hold);
		S.Step(false, false, 0, Hold);
		TestEqual(TEXT("a restart shows the truth at once"), S.Shown, 0);
	}
	// The widget: the same flicker draws one row set, the lit row follows a press.
	FRestore Restore;
	UJamInputMap::Get()->ApplyBindings(FJamBindings::Defaults());
	UJamInputMap::NoteInputDevice(false);
	UJamHudWidget::SetControlsPanelSetting(true);
	FHud Fx;
	if (!Fx.Hud) { AddInfo(TEXT("no Slate: the widget part skipped")); return true; }
	W::FControlsPanelInput In;
	In.bWithPuck = true;
	for (int64 F = 0; F < 100; ++F) { In.SimFrame = F; Fx.Hud->ApplyControlsPanel(In); }
	int32 Changes = 0;
	FString Was = Line(Fx.Hud->GetControlsPanelRowsShown());
	for (int64 F = 100; F < 160; ++F)
	{
		In.SimFrame = F;
		In.bWithPuck = ((F - 100) / 2) % 2 == 1;
		Fx.Hud->ApplyControlsPanel(In);
		const FString Now = Line(Fx.Hud->GetControlsPanelRowsShown());
		Changes += Now != Was ? 1 : 0;
		Was = Now;
	}
	TestTrue(*FString::Printf(TEXT("the widget's rows change at most once in the flicker (%d)"), Changes), Changes <= 1);
	In.bWithPuck = false;
	In.bPress = true;
	In.Held = W::FControlsPanelHeld();
	In.Held.Set(EJamVerb::CrossCheck);
	In.SimFrame = 160;
	Fx.Hud->ApplyControlsPanel(In);
	TestTrue(TEXT("LMB pressed without the puck: the row reads CROSS-CHECK, lit, on that frame"), Line(Fx.Hud->GetControlsPanelRowsShown()).StartsWith(TEXT("LMB CROSS-CHECK*")));
	return true;
}

#endif
