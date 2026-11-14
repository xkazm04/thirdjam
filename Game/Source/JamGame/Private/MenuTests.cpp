// Lane A8 tests: Jam.Menu.*. The first two need no world; MenuToMatchAndBack opens Menu.umap (PIE in the editor), picks the
// regular cut, waits for the rink's game mode, quits to the menu and waits for the menu's game mode. Run:
//   UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Menu; Quit" -unattended -nullrhi -nosplash -log
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "MenuFlow.h"
#include "MenuWidget.h"
#include "MenuGameMode.h"
#include "SettingsWidget.h"
#include "DirectorsCallWidget.h"
#include "DirectorsCallController.h"
#include "BarkWidget.h"
#include "HokejGameMode.h"
#include "JamCartridge.h"
#include "JamJson.h"
#include "JamDesignerSubsystem.h"
#include "InputMapping.h"
#include "DifficultyResolve.h"
#include "JamBrain.h"
#include "DifficultyProfile.h"
#include "InputMappingContext.h"
#include "MenuStyle.h"
#include "HUDWidget.h"
#include "Engine/UserInterfaceSettings.h"
#include "Fonts/FontMeasure.h"
#include "Rendering/SlateRenderer.h"
#include "Framework/Application/SlateApplication.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamMenuTests
{
	constexpr EAutomationTestFlags MenuTestFlags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

	static UJamDesignerSubsystem* MakeDesigner()
	{
		UJamDesignerSubsystem* S = NewObject<UJamDesignerSubsystem>(NewObject<UGameInstance>());
		S->SetKeyOverrideForTest(FString());
		const FString Dir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Automation"), TEXT("MenuCuts"));
		IFileManager::Get().DeleteDirectory(*Dir, false, true);
		S->SetSavedCutsDirForTest(Dir);
		return S;
	}

	static UWorld* GameWorld()
	{
		if (!GEngine) return nullptr;
		for (const FWorldContext& C : GEngine->GetWorldContexts())
		{
			if ((C.WorldType == EWorldType::PIE || C.WorldType == EWorldType::Game) && C.World()) return C.World();
		}
		return nullptr;
	}
}

using JamMenuTests::MakeDesigner;
using JamMenuTests::GameWorld;

// The menu lists the shipped presets by their slate names, each with a description and the key tunables.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuListsCuts, "Jam.Menu.MenuListsCuts", JamMenuTests::MenuTestFlags)
bool FJamMenuListsCuts::RunTest(const FString&)
{
	UJamDesignerSubsystem* S = MakeDesigner();
	const TArray<FJamMenuCutRow> Rows = UJamMenuFlow::BuildCutRows(S->ListCuts());
	TArray<FString> Displays;
	for (const FJamMenuCutRow& R : Rows) Displays.Add(R.Display);
	for (const TCHAR* Expected : { TEXT("Regular Cut"), TEXT("Raw Cut"), TEXT("Boost Cut"), TEXT("Large Rink Cut"), TEXT("Wide Cut") })
	{
		TestTrue(FString::Printf(TEXT("lists %s"), Expected), Displays.Contains(Expected));
	}
	TestTrue(TEXT("Regular Cut is first"), Rows.Num() > 0 && Rows[0].Display == TEXT("Regular Cut"));
	for (const FJamMenuCutRow& R : Rows)
	{
		TestFalse(FString::Printf(TEXT("%s is not a test cartridge"), *R.Name), R.Name.Contains(TEXT(".test.")));
		TestTrue(FString::Printf(TEXT("%s has a description"), *R.Name), R.Description.Len() > 10 && !R.Description.StartsWith(TEXT("cannot load")));
		TestTrue(FString::Printf(TEXT("%s has key tunables"), *R.Name), R.Tunables.Contains(TEXT("speed")) && R.Tunables.Contains(TEXT("shot")));
	}
	const FJamMenuCutRow* Raw = Rows.FindByPredicate([](const FJamMenuCutRow& R) { return R.Name == TEXT("hockey.raw"); });
	TestTrue(TEXT("Raw Cut says body checks"), Raw && Raw->Description.Contains(TEXT("body checks")));
	const FJamMenuCutRow* Large = Rows.FindByPredicate([](const FJamMenuCutRow& R) { return R.Name == TEXT("hockey.arena.large"); });
	TestTrue(TEXT("Large Rink Cut says 120 x 60, open ice (R-0010)"), Large && Large->Description.Contains(TEXT("120 x 60 m, open ice")));
	TestTrue(TEXT("Regular Cut says 90 x 45, fast (R-0010)"), Rows[0].Description.Contains(TEXT("90 x 45 m, fast")));

	// A saved cut keeps its own name and comes after the presets.
	FString Regular;
	FFileHelper::LoadFileToString(Regular, *UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")));
	FString Err;
	TestTrue(TEXT("saves My Director's Cut"), !S->SaveCut(UJamDesignerSubsystem::MyDirectorsCutName, Regular, TEXT("keep it regular"), Err).IsEmpty());
	const TArray<FJamMenuCutRow> Rows2 = UJamMenuFlow::BuildCutRows(S->ListCuts());
	TestEqual(TEXT("one more row"), Rows2.Num(), Rows.Num() + 1);
	TestTrue(TEXT("My Director's Cut is last and named"), Rows2.Num() && Rows2.Last().bSaved && Rows2.Last().Display == UJamDesignerSubsystem::MyDirectorsCutName);

	// Three biggest changes: speed doubled beats a small friction change beats a renamed cut. The numbers come from the shipped
	// regular cut (R-0010 changed them once already), so the text swaps are built from what the loader read.
	TArray<FString> LoadErrors;
	UJamCartridge* RegularCut = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")), LoadErrors, S);
	if (!TestNotNull(TEXT("regular cut loads"), RegularCut)) return false;
	const FString Speed = FJamJson::NumberToString(RegularCut->Game.Skater.Speed);
	const FString Speed2 = FJamJson::NumberToString(RegularCut->Game.Skater.Speed * 2);
	const FString Friction = FJamJson::NumberToString(RegularCut->Game.Rink.IceFriction);
	const FString Friction2 = FJamJson::NumberToString(FMath::RoundToDouble(RegularCut->Game.Rink.IceFriction * 0.8 * 1e6) / 1e6);
	FString Faster = Regular.Replace(*FString::Printf(TEXT("\"speed\": %s,"), *Speed), *FString::Printf(TEXT("\"speed\": %s,"), *Speed2))
		.Replace(*FString::Printf(TEXT("\"ice_friction\": %s"), *Friction), *FString::Printf(TEXT("\"ice_friction\": %s"), *Friction2))
		.Replace(*FString::Printf(TEXT("\"name\": \"%s\""), *RegularCut->ThemeName), TEXT("\"name\": \"Fast Night\""));
	const TArray<FString> Changes = UJamMenuFlow::BiggestChanges(Regular, Faster, 3);
	TestEqual(TEXT("three changes"), Changes.Num(), 3);
	TestTrue(TEXT("speed first"), Changes.Num() && Changes[0].Contains(FString::Printf(TEXT("skater speed %s -> %s"), *Speed, *Speed2)));
	TestTrue(TEXT("friction second"), Changes.Num() > 1 && Changes[1].Contains(TEXT("ice friction")));
	TestTrue(TEXT("rename last"), Changes.Num() > 2 && Changes[2].Contains(TEXT("Fast Night")));
	return true;
}

// The second Director's Call in the same TAKE is refused; the next TAKE allows one again. Plus the card's tunable pick and the
// scoreboard line.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamDirectorsCallOncePerPeriod, "Jam.Menu.DirectorsCallOncePerPeriod", JamMenuTests::MenuTestFlags)
bool FJamDirectorsCallOncePerPeriod::RunTest(const FString&)
{
	UJamDesignerSubsystem* S = MakeDesigner();
	TArray<FString> Errors;
	UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")), Errors, S);
	if (!TestNotNull(TEXT("regular cut loads"), C)) return false;
	S->SetCurrentCartridge(C);
	S->NotifyPeriodStarted(1);
	TestTrue(TEXT("call available at TAKE 1"), S->IsDirectorsCallAvailable());

	const double Speed0 = C->Game.Skater.Speed;
	const FJamDirectorsCall A = S->DirectorsCall(TEXT("skater_speed"), 10.0);
	TestTrue(TEXT("first call applied"), A.bApplied);
	TestEqual(TEXT("first call old value is the cut's skater speed"), A.OldValue, Speed0);
	TestFalse(TEXT("call used"), S->IsDirectorsCallAvailable());

	const FJamDirectorsCall B = S->DirectorsCall(TEXT("shot_speed"), 20.0);
	TestFalse(TEXT("second call in the same TAKE refused"), B.bApplied);
	TestTrue(TEXT("refusal names the period"), B.Reason.Contains(TEXT("period")));

	S->NotifyPeriodStarted(2);
	TestTrue(TEXT("call available again at TAKE 2"), S->IsDirectorsCallAvailable());
	const FJamDirectorsCall Cc = S->DirectorsCall(TEXT("shot_speed"), 20.0);
	TestTrue(TEXT("TAKE 2 call applied"), Cc.bApplied);
	TestTrue(TEXT("the speed call survived"), Cc.Cartridge && FMath::IsNearlyEqual(Cc.Cartridge->Game.Skater.Speed, 10.0));
	TestTrue(TEXT("the shot call applied"), Cc.Cartridge && FMath::IsNearlyEqual(Cc.Cartridge->Game.Shot.Speed, 20.0));

	// The card offers skater speed, shot power, ice friction and the opponents' skill (the regular cut has no pickup tunable).
	const TArray<FJamCallTunable> T = UJamDirectorsCallWidget::PickTunables(C->RawJson, 5);
	TestEqual(TEXT("five tunables"), T.Num(), 5);
	TestTrue(TEXT("skater speed first"), T.Num() && T[0].Key == TEXT("skater_speed"));
	TestTrue(TEXT("shot power second"), T.Num() > 1 && T[1].Key == TEXT("shot_speed"));
	TestTrue(TEXT("ice friction third"), T.Num() > 2 && T[2].Key == TEXT("ice_friction"));
	TestTrue(TEXT("opponents' skill offered"), T.ContainsByPredicate([](const FJamCallTunable& X) { return X.Key == TEXT("ai_red_skill"); }));
	TestTrue(TEXT("ranges come from the cut"), T.Num() && T[0].Min < T[0].Max && T[0].Current == Speed0 && T[0].Min <= Speed0 && Speed0 <= T[0].Max);
	const TArray<FJamCallTunable> Called = UJamDirectorsCallWidget::PickTunables(Cc.Cartridge->RawJson, 5);
	TestTrue(TEXT("current value follows values[]"), Called.Num() && Called[0].Current == 10.0);
	FJamCallTunable Pause;
	Pause.Key = TEXT("goal_pause_frames");
	Pause.Min = 30;
	Pause.Max = 600;
	Pause.Current = 120;
	Pause.bInteger = true;
	TestEqual(TEXT("integer tunable rounds"), Pause.FromNormalized(0.5f), 315.0);
	TestEqual(TEXT("normalized round trip"), Pause.ToNormalized(315.0), 0.5f);

	TestEqual(TEXT("scoreboard line"), UJamDirectorsCallWidget::AnnouncementLine(TEXT("Director's Call: ice is now glass")), FString(TEXT("DIRECTOR'S CALL: ice is now glass")));
	TestEqual(TEXT("scoreboard line without prefix"), UJamDirectorsCallWidget::AnnouncementLine(TEXT("shot power up (18 -> 20)")), FString(TEXT("DIRECTOR'S CALL: shot power up (18 -> 20)")));

	// Lane A42 (y-slots): the MATCH banner's band at 1080p and a bark bubble pushed out of it (above when it fits, else below).
	float Top = 0.f, Bottom = 0.f;
	UJamDirectorsCallHudWidget::BannerBandPx(1080.f, Top, Bottom);
	// A48's storyboard finding 3: the card lives in the top band under the scoreboard (0.12 to 0.42 of 1080), never over the ring at 0.66.
	TestTrue(TEXT("card band under the scoreboard"), Top >= 110.f && Top < UJamDirectorsCallHudWidget::MatchCardCentreY && Bottom > UJamDirectorsCallHudWidget::MatchCardCentreY && Bottom <= 0.42f * 1080.f);
	TestTrue(TEXT("card clear of the countdown ring"), Bottom < 0.66f * 1080.f - 110.f);
	TestEqual(TEXT("bubble above the card when there is room"), static_cast<float>(UJamBarkWidget::PushOutOfBand(FVector2D(600, Top + 40.f), 54.f, Top, Bottom, 60.f).Y), Top - 4.f);
	// With the card in the top band the bubbles' own floor (170) leaves no room above: they go under the card.
	TestEqual(TEXT("bubble below the card in play"), static_cast<float>(UJamBarkWidget::PushOutOfBand(FVector2D(600, Top + 40.f), 54.f, Top, Bottom, 170.f).Y), Bottom + 4.f + 54.f);
	TestEqual(TEXT("bubble clear of the banner stays"), UJamBarkWidget::PushOutOfBand(FVector2D(600, Bottom + 100.f), 54.f, Top, Bottom, 170.f), FVector2D(600, Bottom + 100.f));

	// Settings round trip of the difficulty strings (lane A3's contract).
	EJamMenuDifficulty D;
	TestTrue(TEXT("all_star parses"), FJamSettings::ParseDifficulty(TEXT("all_star"), D) && D == EJamMenuDifficulty::AllStar);
	TestEqual(TEXT("rookie string"), FJamSettings::DifficultyString(EJamMenuDifficulty::Rookie), FString(TEXT("rookie")));
	return true;
}

// ---- MenuToMatchAndBack: latent, needs the maps (PIE in the editor, or a game world).

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FJamMenuPickRegular, FAutomationTestBase*, Test, int32, Ticks);
bool FJamMenuPickRegular::Update()
{
	UWorld* W = GameWorld();
	AJamMenuGameMode* GM = W ? Cast<AJamMenuGameMode>(W->GetAuthGameMode()) : nullptr;
	if (GM && GM->GetMenu())
	{
		Test->TestTrue(TEXT("menu picks the regular cut"), GM->GetMenu()->ChooseCut(TEXT("hockey.regular")));
		return true;
	}
	if (++Ticks > 1200) { Test->AddError(TEXT("the menu map did not come up with AJamMenuGameMode and its widget (is Menu.umap's game mode set?)")); return true; }
	return false;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FJamWaitForMatch, FAutomationTestBase*, Test, int32, Ticks);
bool FJamWaitForMatch::Update()
{
	UWorld* W = GameWorld();
	AHokejGameMode* GM = W ? Cast<AHokejGameMode>(W->GetAuthGameMode()) : nullptr;
	if (GM && GM->GetSim() && GM->GetSim()->GetFrame() > 30)
	{
		Test->TestTrue(TEXT("match started on the regular cut"), GM->GetCartridge() && GM->GetCartridge()->Id == TEXT("hockey-regular"));
		UJamDirectorsCallController* Ctl = W->GetSubsystem<UJamDirectorsCallController>();
		Test->TestTrue(TEXT("Director's Call controller is active in the rink"), Ctl && Ctl->IsActive());
		Test->TestTrue(TEXT("MATCH 1 card shown"), Ctl && Ctl->GetOverlay() && Ctl->GetOverlay()->GetTakeCardText() == TEXT("MATCH 1"));
		if (Ctl)
		{
			Ctl->Pause();
			Test->TestTrue(TEXT("pause menu opens"), Ctl->IsMenuOpen());
			Test->TestTrue(TEXT("game paused"), W->IsPaused());
			Ctl->QuitToMenu();
		}
		return true;
	}
	if (++Ticks > 1800) { Test->AddError(TEXT("the rink did not start a match after the menu picked a cut")); return true; }
	return false;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FJamWaitForMenu, FAutomationTestBase*, Test, int32, Ticks);
bool FJamWaitForMenu::Update()
{
	UWorld* W = GameWorld();
	AJamMenuGameMode* GM = W ? Cast<AJamMenuGameMode>(W->GetAuthGameMode()) : nullptr;
	if (GM && GM->GetMenu())
	{
		Test->TestFalse(TEXT("menu world is not paused"), W->IsPaused());
		return true;
	}
	if (++Ticks > 1200) { Test->AddError(TEXT("quit to menu did not bring the menu back")); return true; }
	return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuToMatchAndBack, "Jam.Menu.MenuToMatchAndBack", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FJamMenuToMatchAndBack::RunTest(const FString&)
{
	if (!AutomationOpenMap(UJamMenuFlow::MenuMap))
	{
		AddError(FString::Printf(TEXT("cannot open %s"), UJamMenuFlow::MenuMap));
		return false;
	}
	ADD_LATENT_AUTOMATION_COMMAND(FJamMenuPickRegular(this, 0));
	ADD_LATENT_AUTOMATION_COMMAND(FJamWaitForMatch(this, 0));
	ADD_LATENT_AUTOMATION_COMMAND(FJamWaitForMenu(this, 0));
	ADD_LATENT_AUTOMATION_COMMAND(FExitGameCommand());
	return true;
}

// Lane A67 (R-0036): the six Director's choices are on disk, load through the game's loader, carry a pitch and three facts, and name
// themselves by cartridge id (the match card's line); the shipped cuts are not modes.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuDirectorsChoices, "Jam.Menu.DirectorsChoices", JamMenuTests::MenuTestFlags)
bool FJamMenuDirectorsChoices::RunTest(const FString&)
{
	const TArray<FJamModeCard> Cards = UJamMenuFlow::AvailableModeCards();
	TestEqual(TEXT("twelve Director's choices on disk (lane A81: two rows of six)"), Cards.Num(), 12);
	for (const FJamModeCard& M : Cards)
	{
		TArray<FString> Errors;
		UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(M.Name), Errors);
		for (const FString& E : Errors) AddError(FString::Printf(TEXT("%s: %s"), *M.Name, *E));
		if (!C) continue;
		TestEqual(*FString::Printf(TEXT("%s id"), *M.Name), C->Id, M.Id);
		TestEqual(*FString::Printf(TEXT("%s names itself"), *M.Name), UJamMenuFlow::ModeDisplayForId(C->Id), M.Display);
		TestEqual(*FString::Printf(TEXT("%s three facts"), *M.Name), M.Facts.Num(), 3);
		TestFalse(*FString::Printf(TEXT("%s pitch"), *M.Name), M.Pitch.IsEmpty());
	}
	TestTrue(TEXT("the regular cut is not a mode"), UJamMenuFlow::ModeDisplayForId(TEXT("hockey-regular")).IsEmpty());
	return true;
}

// ---------------------------------------------------------------- lane A144 (track U2): the first screen, the fit, the controls

namespace JamMenuTests
{
	// A menu widget in a game world of its own. No game instance, so no Designer and no season: the first screen, the mode cards (read from disk),
	// the settings, the credits and the controls build as they do in the game; the cut cards are checked from the rows.
	struct FMenuFixture
	{
		UWorld* World = nullptr;
		UJamMenuWidget* Menu = nullptr;

		FMenuFixture()
		{
			if (!GEngine || !FSlateApplication::IsInitialized()) return;
			World = UWorld::CreateWorld(EWorldType::Game, false, TEXT("JamMenuTestWorld"));
			if (!World) return;
			FWorldContext& Ctx = GEngine->CreateNewWorldContext(EWorldType::Game);
			Ctx.SetCurrentWorld(World);
			Menu = CreateWidget<UJamMenuWidget>(World, UJamMenuWidget::StaticClass());
			if (Menu) { Menu->AddToRoot(); Menu->TakeWidget(); Menu->ShowScreen(EJamMenuScreen::Main); }
		}
		~FMenuFixture()
		{
			if (Menu) { Menu->ReleaseSlateResources(true); Menu->RemoveFromRoot(); }
			if (World) { GEngine->DestroyWorldContext(World); World->DestroyWorld(false); }
		}
	};

	// The viewport in slate units at both shipped resolutions: the DPI curve scales by the short side over 1080 (1280 x 720 at 2/3 is 1920 x 1080).
	constexpr float ViewW = 1920.f;
	constexpr float ViewH = 1080.f;
}

// The first screen says what the game is and opens on PLAY NOW: one match on the regular cut, five a side as the line says.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuFirstScreen, "Jam.Menu.FirstScreen", JamMenuTests::MenuTestFlags)
bool FJamMenuFirstScreen::RunTest(const FString&)
{
	// A fresh profile, held in memory for the test (nothing of it reaches the player's settings file).
	const FJamSettings Fresh = FJamSettings();
	FJamSettings::SetOverrideForTest(&Fresh);
	ON_SCOPE_EXIT { FJamSettings::SetOverrideForTest(nullptr); };
	JamMenuTests::FMenuFixture F;
	if (!TestNotNull(TEXT("the menu builds in a test world"), F.Menu)) return false;

	// Evolution 3: the level is chosen where it is played. Three segments under PLAY NOW, the level in force lit, one line on what it means.
	const TArray<FString> Segments = F.Menu->GetSwitchLabels();
	TestTrue(TEXT("the switch reads ROOKIE / PRO / ALL-STAR"), Segments.Num() == 3 && Segments[0] == TEXT("ROOKIE") && Segments[1] == TEXT("PRO") && Segments[2] == TEXT("ALL-STAR"));
	TestEqual(TEXT("a fresh profile lights ROOKIE"), static_cast<int32>(F.Menu->GetSwitchLevel()), static_cast<int32>(EJamMenuDifficulty::Rookie));
	TestEqual(TEXT("PLAY NOW's line follows the switch"), F.Menu->GetFirstPressLine(), UJamMenuFlow::FirstMatchLine(TEXT("rookie")));
	TestEqual(TEXT("the line under the switch says what rookie means"), F.Menu->GetLevelMeaningLine(), UJamMenuFlow::LevelMeaning(TEXT("rookie")));
	F.Menu->PickLevel(EJamMenuDifficulty::AllStar);
	TestEqual(TEXT("a click on ALL-STAR lights it"), static_cast<int32>(F.Menu->GetSwitchLevel()), static_cast<int32>(EJamMenuDifficulty::AllStar));
	TestTrue(TEXT("PLAY NOW's line says ALL-STAR"), F.Menu->GetFirstPressLine().Contains(TEXT("ALL-STAR")));
	TestEqual(TEXT("the line says what all-star means"), F.Menu->GetLevelMeaningLine(), UJamMenuFlow::LevelMeaning(TEXT("all_star")));
	TestTrue(TEXT("the switch wrote the setting the SETTINGS button writes"), FJamSettings::Load().bDifficultyChosen && FJamSettings::Load().Difficulty == EJamMenuDifficulty::AllStar);
	F.Menu->StepLevel(-1);
	TestEqual(TEXT("left steps to PRO"), static_cast<int32>(F.Menu->GetSwitchLevel()), static_cast<int32>(EJamMenuDifficulty::Pro));
	F.Menu->StepLevel(-1);
	F.Menu->StepLevel(-1);
	TestEqual(TEXT("the left end holds at ROOKIE"), static_cast<int32>(F.Menu->GetSwitchLevel()), static_cast<int32>(EJamMenuDifficulty::Rookie));
	TestTrue(TEXT("a rookie picked on the switch is a pick"), FJamSettings::Load().bDifficultyChosen && FJamSettings::Load().EffectiveDifficulty() == EJamMenuDifficulty::Rookie);
	F.Menu->StepLevel(1);
	F.Menu->StepLevel(1);
	F.Menu->StepLevel(1);
	TestEqual(TEXT("the right end holds at ALL-STAR"), static_cast<int32>(F.Menu->GetSwitchLevel()), static_cast<int32>(EJamMenuDifficulty::AllStar));
	// The three meanings are three different lines, and none names a number the ladder does not hold.
	// Lane A176: the line says who the level is for, and carries no number the curve could make stale.
	TestTrue(TEXT("rookie is for players new to hockey games"), UJamMenuFlow::LevelMeaning(TEXT("rookie")).StartsWith(TEXT("New to hockey games")));
	TestTrue(TEXT("pro is for players who have played one"), UJamMenuFlow::LevelMeaning(TEXT("pro")).StartsWith(TEXT("Played one before")));
	TestTrue(TEXT("all-star is the wall"), UJamMenuFlow::LevelMeaning(TEXT("all_star")).StartsWith(TEXT("The wall")));
	for (const TCHAR* Level : { TEXT("rookie"), TEXT("pro"), TEXT("all_star") })
	{
		const FString L = UJamMenuFlow::LevelMeaning(Level);
		bool bDigit = false;
		for (const TCHAR C : L) bDigit |= FChar::IsDigit(C);
		TestFalse(*FString::Printf(TEXT("the %s line has no number that can go stale"), Level), bDigit || L.Contains(TEXT("%")));
	}
	TestTrue(TEXT("each level has its own line"), UJamMenuFlow::LevelMeaning(TEXT("rookie")) != UJamMenuFlow::LevelMeaning(TEXT("pro")) && UJamMenuFlow::LevelMeaning(TEXT("pro")) != UJamMenuFlow::LevelMeaning(TEXT("all_star")));
	const TArray<FString> Labels = F.Menu->GetMainLabels();
	// Lane A187 (R-0064 point 3): the front end of a sports game: no cut to direct on MAIN.
	const TArray<FString> Expected = { TEXT("PLAY NOW"), TEXT("SEASON"), TEXT("MODES"), TEXT("PLAY WITH A FRIEND"),
		TEXT("CONTROLS"), TEXT("SETTINGS"), TEXT("CREDITS"), TEXT("QUIT") };
	TestEqual(TEXT("eight buttons on MAIN"), Labels.Num(), Expected.Num());
	for (const FString& Lb : Labels) TestFalse(*FString::Printf(TEXT("MAIN does not present the game as cuts to direct (%s)"), *Lb), Lb.Contains(TEXT("CUT")) || Lb.Contains(TEXT("DIRECT")));
	for (int32 i = 0; i < Expected.Num() && i < Labels.Num(); ++i)
	{
		TestTrue(*FString::Printf(TEXT("button %d is %s (it reads %s)"), i + 1, *Expected[i], *Labels[i]), Labels[i].StartsWith(Expected[i]));
	}
	TestNotNull(TEXT("the screen has a first press"), F.Menu->GetFirstPress());
	TestEqual(TEXT("PLAY NOW starts the regular cut"), FString(UJamMenuFlow::FirstCut), FString(TEXT("hockey.regular")));

	// The line is true of the cut the first press starts.
	TArray<FString> Errors;
	const UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(UJamMenuFlow::FirstCut), Errors);
	if (!TestNotNull(TEXT("the first cut loads"), C)) return false;
	const FString What = UJamMenuFlow::WhatItIsLine();
	TestTrue(TEXT("the line says what the game is, in one line"), What.Contains(TEXT("hockey")) && What.Len() <= 64);
	TestTrue(TEXT("five a side, as the line says"), What.Contains(TEXT("Five-a-side")) && C->Game.Teams.Num() == 2 && C->Game.Teams[0].Skaters.Num() == 5 && C->Game.Teams[1].Skaters.Num() == 5);
	// R-0054 point 5: the whistle line only when the first cut has no whistle.
	TestTrue(TEXT("the brand line does not promise a whistle-free game the first cut does not play"), !UJamMenuFlow::BrandLine().Contains(TEXT("whistle")) || !C->Game.Rules.bPenalties);
	return true;
}

// Twelve cards, every name inside its card, and every screen inside the 720p viewport by the widget's own desired size.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuFitsTheScreen, "Jam.Menu.FitsTheScreen", JamMenuTests::MenuTestFlags)
bool FJamMenuFitsTheScreen::RunTest(const FString&)
{
	const FJamSettings Fresh = FJamSettings();
	FJamSettings::SetOverrideForTest(&Fresh);
	ON_SCOPE_EXIT { FJamSettings::SetOverrideForTest(nullptr); };
	JamMenuTests::FMenuFixture F;
	if (!TestNotNull(TEXT("the menu builds in a test world"), F.Menu)) return false;
	const TPair<EJamMenuScreen, const TCHAR*> Screens[] = {
		{ EJamMenuScreen::Main, TEXT("main") }, { EJamMenuScreen::Cuts, TEXT("cuts") }, { EJamMenuScreen::Direct, TEXT("direct") },
		{ EJamMenuScreen::Settings, TEXT("settings") }, { EJamMenuScreen::Credits, TEXT("credits") }, { EJamMenuScreen::Controls, TEXT("controls") },
		{ EJamMenuScreen::Landing, TEXT("landing") }, { EJamMenuScreen::Modes, TEXT("modes") }, { EJamMenuScreen::Season, TEXT("season") } };
	// Lane A187: 1280 x 720, 1920 x 1080 and 2560 x 1440 are all 1920 x 1080 slate units (the DPI curve scales by the short side), so this one measure
	// holds the layout at the three sizes.
	for (const TPair<EJamMenuScreen, const TCHAR*>& Sc : Screens)
	{
		const FVector2D Size = F.Menu->MeasureScreen(Sc.Key);
		// The test world has no Designer, so the cuts screen is measured without its cut cards: the "No cuts found" line (one body line and its
		// 6 units of padding: 35) stands where the two rows of cut cards (CutListHeight) are in the game.
		const float Extra = Sc.Key == EJamMenuScreen::Cuts ? UJamMenuWidget::CutListHeight() - 35.f : 0.f;
		AddInfo(FString::Printf(TEXT("JAMMENU measure %s %.0f x %.0f of %.0f x %.0f"), Sc.Value, Size.X, Size.Y + Extra, JamMenuTests::ViewW, JamMenuTests::ViewH));
		TestTrue(*FString::Printf(TEXT("%s is at most %.0f wide (%.0f)"), Sc.Value, JamMenuTests::ViewW, Size.X), Size.X > 100.f && Size.X <= JamMenuTests::ViewW);
		TestTrue(*FString::Printf(TEXT("%s is at most %.0f high (%.0f)"), Sc.Value, JamMenuTests::ViewH, Size.Y + Extra), Size.Y > 100.f && Size.Y + Extra <= JamMenuTests::ViewH);
		// Evolution 3 (the Director's bar): the first screen with its level switch stays at or under 1000 of the 1080 units.
		if (Sc.Key == EJamMenuScreen::Main) TestTrue(*FString::Printf(TEXT("the first screen is at most 1000 high (%.0f)"), Size.Y), Size.Y <= 1000.f);
	}

	// The twelve cards are on MODES (lane A187), and their words fit the card's own text width with the card's own type: the name on one line, the
	// pitch on two; the facts, one line under the grid for the focused card, fit that line.
	F.Menu->MeasureScreen(EJamMenuScreen::Modes);
	TestEqual(TEXT("twelve mode cards on MODES"), F.Menu->GetModeButtons().Num(), 12);
	// Director 13:35: every card's description inside its box, descenders included: its laid-out height (the wrapped lines at the card's type) against
	// the box, and two full lines of the face against the box. Slate units: the same at 1280 x 720 and 1920 x 1080.
	{
		const TArray<FVector2D> Pitches = F.Menu->GetModePitchSizes();
		TestEqual(TEXT("fourteen card descriptions laid out"), Pitches.Num(), 14);
		const float Box = UJamMenuWidget::ModePitchBoxHeight();
		const float LineH = static_cast<float>(FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->GetMaxCharacterHeight(UJamMenuWidget::ModePitchFont()));
		AddInfo(FString::Printf(TEXT("JAMMENU measure card description: line %.1f, box %.0f"), LineH, Box));
		TestTrue(*FString::Printf(TEXT("two full lines (%.1f) fit the description box (%.0f)"), 2.f * LineH, Box), LineH > 0.f && 2.f * LineH <= Box);
		for (int32 i = 0; i < Pitches.Num(); ++i)
			TestTrue(*FString::Printf(TEXT("card %d description %.0f x %.1f inside %.0f x %.0f"), i + 1, Pitches[i].X, Pitches[i].Y, UJamMenuWidget::ModeTextWidth(), Box),
				Pitches[i].Y > 0.f && Pitches[i].Y <= Box && Pitches[i].X <= UJamMenuWidget::ModeTextWidth() + 0.5f);
	}
	const TSharedRef<FSlateFontMeasure> Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	const float TextW = UJamMenuWidget::ModeTextWidth();
	float WidestName = 0.f, WidestPitch = 0.f, WidestFact = 0.f;
	FString WidestNameIs;
	for (const FJamModeCard& M : UJamMenuFlow::AvailableModeCards())
	{
		const float NameW = static_cast<float>(Measure->Measure(M.Display.ToUpper(), UJamMenuWidget::ModeNameFont()).X);
		const float PitchW = static_cast<float>(Measure->Measure(M.Pitch, UJamMenuWidget::ModePitchFont()).X);
		if (NameW > WidestName) { WidestName = NameW; WidestNameIs = M.Display; }
		WidestPitch = FMath::Max(WidestPitch, PitchW);
		TestTrue(*FString::Printf(TEXT("%s fits its card on one line (%.0f of %.0f)"), *M.Display, NameW, TextW), NameW > 0.f && NameW <= TextW);
		// Two lines hold at most 1.75 widths of words (the break between them costs up to a word).
		TestTrue(*FString::Printf(TEXT("%s pitch fits two lines (%.0f of %.0f)"), *M.Display, PitchW, 1.75f * TextW), PitchW > 0.f && PitchW <= 1.75f * TextW);
		{
			const FString Line = FString::Printf(TEXT("%s   /   %s"), *M.Display.ToUpper(), *FString::Join(M.Facts, TEXT("   /   ")));
			const float FactW = static_cast<float>(Measure->Measure(Line, FJamMenuLook::Font(FJamMenuLook::Body - 1, FName("Bold"), 20)).X);
			WidestFact = FMath::Max(WidestFact, FactW);
			TestTrue(*FString::Printf(TEXT("%s facts fit the line under the grid (%.0f of %.0f)"), *M.Display, FactW, UJamMenuWidget::ModeDetailWidth()), FactW > 0.f && FactW <= UJamMenuWidget::ModeDetailWidth());
			TestTrue(*FString::Printf(TEXT("%s sides read (%d v %d)"), *M.Display, M.HomeSkaters, M.AwaySkaters), M.HomeSkaters > 0 && M.AwaySkaters > 0);
		}
	}
	AddInfo(FString::Printf(TEXT("JAMMENU measure cards: widest name %.0f (%s), widest pitch %.0f, widest fact %.0f, text width %.0f"), WidestName, *WidestNameIs, WidestPitch, WidestFact, TextW));

	// Evolution 3: what a level means fits one line under the switch (816 units, the card's text width).
	for (const TCHAR* Level : { TEXT("rookie"), TEXT("pro"), TEXT("all_star") })
	{
		const float W = static_cast<float>(Measure->Measure(UJamMenuFlow::LevelMeaning(Level), FJamUiStyle::CaptionFont()).X);
		TestTrue(*FString::Printf(TEXT("the %s line fits one line (%.0f of 816)"), Level, W), W > 0.f && W <= 816.f);
	}

	// The cut cards: every shipped cut's name on one line and its line on two, inside a third of the list.
	for (const FJamMenuCutRow& R : UJamMenuFlow::BuildCutRows(MakeDesigner()->ListCuts()))
	{
		const float W = static_cast<float>(Measure->Measure(R.Display.ToUpper(), UJamMenuWidget::CutNameFont()).X);
		TestTrue(*FString::Printf(TEXT("%s fits a cut card (%.0f of %.0f)"), *R.Display, W, UJamMenuWidget::CutTextWidth()), W > 0.f && W <= UJamMenuWidget::CutTextWidth());
		TestFalse(*FString::Printf(TEXT("%s is described by its own facts, not by a theme name"), *R.Display), R.Description.Contains(TEXT("Night Rink")));
		const float LineW = static_cast<float>(Measure->Measure(UJamMenuFlow::CutCardLine(R), UJamMenuWidget::CutLineFont()).X);
		TestTrue(*FString::Printf(TEXT("%s line fits two lines of its card (%.0f of %.0f)"), *R.Display, LineW, 1.75f * UJamMenuWidget::CutTextWidth()), LineW > 0.f && LineW <= 1.75f * UJamMenuWidget::CutTextWidth());
	}

	// Lane A195 (U2 evolution 6): every screen a player meets, not only the menu's. The three shipped sizes are one layout because the DPI curve maps each
	// to 1920 x 1080 units: held here from the project's own curve, not assumed. Then the pause card (the pause menu, the rule card, CONTROLS) and the end
	// card (a match's, a season take's between-takes card, the season's last) measured in those units.
	if (const UUserInterfaceSettings* UI = GetDefault<UUserInterfaceSettings>())
	{
		for (const FIntPoint Res : { FIntPoint(1280, 720), FIntPoint(1920, 1080), FIntPoint(2560, 1440) })
		{
			const float Scale = UI->GetDPIScaleBasedOnSize(Res);
			const FVector2D Units = FVector2D(Res) / FMath::Max(Scale, 0.01f);
			AddInfo(FString::Printf(TEXT("JAMMENU dpi %dx%d scale %.3f = %.0f x %.0f units"), Res.X, Res.Y, Scale, Units.X, Units.Y));
			TestTrue(*FString::Printf(TEXT("%dx%d is 1920 x 1080 units (%.0f x %.0f)"), Res.X, Res.Y, Units.X, Units.Y), FMath::IsNearlyEqual(Units.X, JamMenuTests::ViewW, 2.f) && FMath::IsNearlyEqual(Units.Y, JamMenuTests::ViewH, 2.f));
		}
	}
	{
		UJamDirectorsCallWidget* P = CreateWidget<UJamDirectorsCallWidget>(F.World, UJamDirectorsCallWidget::StaticClass());
		if (TestNotNull(TEXT("the pause card builds"), P))
		{
			P->AddToRoot();
			ON_SCOPE_EXIT { P->ReleaseSlateResources(true); P->RemoveFromRoot(); };
			TSharedRef<SWidget> Slate = P->TakeWidget();
			const TPair<int32, const TCHAR*> PauseScreens[] = { { 0, TEXT("pause") }, { 1, TEXT("rule card") }, { 2, TEXT("pause controls") } };
			for (const TPair<int32, const TCHAR*>& Ps : PauseScreens)
			{
				if (Ps.Key == 0) P->ShowPause(); else if (Ps.Key == 1) P->ShowCall(); else P->ShowControls();
				Slate->SlatePrepass(1.f);
				const FVector2D Size = Slate->GetDesiredSize();
				AddInfo(FString::Printf(TEXT("JAMMENU measure %s %.0f x %.0f of %.0f x %.0f"), Ps.Value, Size.X, Size.Y, JamMenuTests::ViewW, JamMenuTests::ViewH));
				TestTrue(*FString::Printf(TEXT("the %s card is %.0f wide (%.0f)"), Ps.Value, UJamDirectorsCallWidget::PauseCardWidth, Size.X), FMath::IsNearlyEqual(Size.X, UJamDirectorsCallWidget::PauseCardWidth, 1.f));
				TestTrue(*FString::Printf(TEXT("the %s card fits 1080 units high with the header band clear (%.0f of 1000)"), Ps.Value, Size.Y), Size.Y > 100.f && Size.Y <= 1000.f);
				const float RowW = P->GetHintRowWidth();
				TestTrue(*FString::Printf(TEXT("the %s card's key hints fit inside it (%.0f of %.0f)"), Ps.Value, RowW, UJamDirectorsCallWidget::PauseCardInnerWidth), RowW > 100.f && RowW <= UJamDirectorsCallWidget::PauseCardInnerWidth);
			}
			P->ShowPause();
		}
	}
	{
		UJamHudWidget* H = CreateWidget<UJamHudWidget>(F.World, UJamHudWidget::StaticClass());
		if (TestNotNull(TEXT("the HUD builds"), H))
		{
			H->AddToRoot();
			ON_SCOPE_EXIT { H->ReleaseSlateResources(true); H->RemoveFromRoot(); };
			H->TakeWidget();
			const TPair<const TCHAR*, const TCHAR*> Ends[] = {
				{ TEXT("YOU WIN"), TEXT("R  REMATCH      ESC  MENU") },
				{ TEXT("BEATEN"), TEXT("R  NEXT MATCH: THE FINAL CUT      ESC  MENU") },   // the season's between-takes card, the longest next take name
				{ TEXT("YOU WIN"), TEXT("SEASON COMPLETE      R  MENU") } };
			for (const TPair<const TCHAR*, const TCHAR*>& E : Ends)
			{
				H->ShowEndCardForTest(E.Key, UJamHudWidget::EndCardScore(3, 1), E.Value);
				UWidget* Card = H->GetEndCardForTest();
				if (!TestNotNull(TEXT("the end card exists"), Card)) break;
				TSharedRef<SWidget> CS = Card->TakeWidget();
				CS->SlatePrepass(1.f);
				const FVector2D Size = CS->GetDesiredSize();
				AddInfo(FString::Printf(TEXT("JAMMENU measure end card '%s' %.0f x %.0f (card %.0f)"), E.Value, Size.X, Size.Y, UJamHudWidget::EndCardW));
				TestTrue(*FString::Printf(TEXT("the end card keeps its width with '%s' (%.0f)"), E.Value, Size.X), FMath::IsNearlyEqual(Size.X, UJamHudWidget::EndCardW, 1.f));
				TestTrue(*FString::Printf(TEXT("the end card fits 1080 units high (%.0f)"), Size.Y), Size.Y > 100.f && Size.Y <= 0.8f * JamMenuTests::ViewH);
				const float RowW = H->GetEndCardHintRowWidth(), Inner = UJamHudWidget::EndCardW - 2.f * FJamMenuLook::CardPad;
				TestTrue(*FString::Printf(TEXT("the end card's keys fit inside it with '%s' (%.0f of %.0f)"), E.Value, RowW, Inner), RowW > 50.f && RowW <= Inner);
			}
		}
	}
	return true;
}

// Lane A187 (track U2 evolution 5, R-0064 point 3): the front end. The landing's press goes to MAIN with PLAY NOW as its home; every screen has a
// header, live key hints, a home focus and a parent (Back from everywhere); MODES holds the twelve modes, ALL RINKS and MAKE YOUR OWN (the Designer's
// screen, last); the style table keeps its spacing on the 8-unit grid and its transition under 200 ms; nothing of the front end names a league.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuFrontEnd, "Jam.Menu.FrontEnd", JamMenuTests::MenuTestFlags)
bool FJamMenuFrontEnd::RunTest(const FString&)
{
	const FJamSettings Fresh = FJamSettings();
	FJamSettings::SetOverrideForTest(&Fresh);
	ON_SCOPE_EXIT { FJamSettings::SetOverrideForTest(nullptr); };
	JamMenuTests::FMenuFixture F;
	if (!TestNotNull(TEXT("the menu builds in a test world"), F.Menu)) return false;

	// The landing, then MAIN with PLAY NOW: the release row's two Returns.
	F.Menu->ShowScreen(EJamMenuScreen::Landing);
	TestEqual(TEXT("the landing is shown"), static_cast<int32>(F.Menu->GetScreen()), static_cast<int32>(EJamMenuScreen::Landing));
	TestTrue(TEXT("the landing's hints say what starts it"), F.Menu->GetHintLine().Contains(TEXT("ENTER / A START")));
	TestNotNull(TEXT("the landing has one place to stand"), F.Menu->GetHomeFocus(EJamMenuScreen::Landing));
	F.Menu->PressStart();
	TestEqual(TEXT("the landing's press opens MAIN"), static_cast<int32>(F.Menu->GetScreen()), static_cast<int32>(EJamMenuScreen::Main));
	TestTrue(TEXT("MAIN's home is PLAY NOW (a second Enter plays)"), F.Menu->GetHomeFocus(EJamMenuScreen::Main) == F.Menu->GetFirstPress());
	TestEqual(TEXT("MAIN's header"), F.Menu->GetHeaderTitle(), FString(TEXT("HOME")));
	F.Menu->PressStart();
	TestEqual(TEXT("a press on MAIN is not a landing press"), static_cast<int32>(F.Menu->GetScreen()), static_cast<int32>(EJamMenuScreen::Main));

	// Every screen: a header (but the landing), key hints, a home, and Back to its parent.
	const EJamMenuScreen All[] = { EJamMenuScreen::Main, EJamMenuScreen::Season, EJamMenuScreen::Modes, EJamMenuScreen::Cuts, EJamMenuScreen::Direct,
		EJamMenuScreen::Settings, EJamMenuScreen::Credits, EJamMenuScreen::Controls };
	for (const EJamMenuScreen Sc : All)
	{
		const FString Name = UJamMenuWidget::ScreenName(Sc);
		EJamMenuScreen Parsed;
		TestTrue(*FString::Printf(TEXT("%s parses back"), *Name), UJamMenuWidget::ParseScreen(Name, Parsed) && Parsed == Sc);
		F.Menu->ShowScreen(Sc);
		TestFalse(*FString::Printf(TEXT("%s has a header"), *Name), F.Menu->GetHeaderTitle().IsEmpty());
		TestTrue(*FString::Printf(TEXT("%s has key hints with Back or Title (%s)"), *Name, *F.Menu->GetHintLine()), F.Menu->GetHintLine().Contains(TEXT("ESC / B")));
		TestNotNull(*FString::Printf(TEXT("%s has a home focus"), *Name), F.Menu->GetHomeFocus(Sc));
		const EJamMenuScreen Parent = UJamMenuWidget::ParentOf(Sc);
		F.Menu->GoBack();
		TestEqual(*FString::Printf(TEXT("Back from %s"), *Name), static_cast<int32>(F.Menu->GetScreen()), static_cast<int32>(Parent));
	}
	TestEqual(TEXT("MAKE YOUR OWN goes back to MODES"), static_cast<int32>(UJamMenuWidget::ParentOf(EJamMenuScreen::Direct)), static_cast<int32>(EJamMenuScreen::Modes));
	TestEqual(TEXT("ALL RINKS goes back to MODES"), static_cast<int32>(UJamMenuWidget::ParentOf(EJamMenuScreen::Cuts)), static_cast<int32>(EJamMenuScreen::Modes));
	TestEqual(TEXT("the landing stays"), static_cast<int32>(UJamMenuWidget::ParentOf(EJamMenuScreen::Landing)), static_cast<int32>(EJamMenuScreen::Landing));

	// MODES: the twelve, then ALL RINKS, then MAKE YOUR OWN; MAKE YOUR OWN opens the Designer's screen.
	F.Menu->ShowScreen(EJamMenuScreen::Modes);
	const TArray<FString> Cards = F.Menu->GetModeCardNames();
	TestEqual(TEXT("fourteen cards on MODES"), Cards.Num(), 14);
	if (Cards.Num() >= 2)
	{
		TestEqual(TEXT("the last card is MAKE YOUR OWN"), Cards.Last(), FString(TEXT("MAKE YOUR OWN")));
		TestEqual(TEXT("ALL RINKS before it"), Cards[Cards.Num() - 2], FString(TEXT("ALL RINKS")));
	}
	F.Menu->OnRowClicked(EJamMenuRowKind::Open, TEXT("direct"));
	TestEqual(TEXT("MAKE YOUR OWN opens the Designer's screen"), static_cast<int32>(F.Menu->GetScreen()), static_cast<int32>(EJamMenuScreen::Direct));
	F.Menu->GoBack();
	F.Menu->OnRowClicked(EJamMenuRowKind::Open, TEXT("cuts"));
	TestEqual(TEXT("ALL RINKS opens the rinks"), static_cast<int32>(F.Menu->GetScreen()), static_cast<int32>(EJamMenuScreen::Cuts));

	// SEASON without a game instance says so and still has its Back.
	F.Menu->ShowScreen(EJamMenuScreen::Season);
	TestNotNull(TEXT("SEASON without a season: the home is BACK"), F.Menu->GetHomeFocus(EJamMenuScreen::Season));

	// The style table.
	for (const float V : FJamMenuLook::GridValues())
		TestTrue(*FString::Printf(TEXT("%.0f is on the 8-unit grid"), V), FMath::IsNearlyZero(FMath::Fmod(V, FJamMenuLook::Unit)));
	TestTrue(TEXT("a screen comes in under 200 ms"), FJamMenuLook::TransitionSeconds > 0.f && FJamMenuLook::TransitionSeconds < 0.2f);
	TestTrue(TEXT("the focus colour is not the chosen colour"), !FJamMenuLook::Focus().Equals(FJamMenuLook::Chosen()));
	TestTrue(TEXT("device B's names are listed"), FJamMenuLook::KitNames().Num() >= 20);

	// No league, team or broadcaster: the words of MAIN, MODES and the hints carry none of the marks a sports game's menu would borrow.
	TArray<FString> Words = F.Menu->GetMainLabels();
	Words.Append(Cards);
	for (const FString& W : Words)
		for (const TCHAR* Mark : { TEXT("NHL"), TEXT("NATIONAL HOCKEY"), TEXT("EA SPORTS"), TEXT("ESPN"), TEXT("STANLEY") })
			TestFalse(*FString::Printf(TEXT("'%s' carries no mark (%s)"), *W, Mark), W.ToUpper().Contains(Mark));
	return true;
}

// The CONTROLS screen against the bindings: every key UJamInputMap binds is on a row, every key a row names is bound, and the rows say the
// things a hand cannot guess (a tap is a flat shot, a held pass is a saucer, S against your motion brakes).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuControlsComplete, "Jam.Menu.ControlsComplete", JamMenuTests::MenuTestFlags)
bool FJamMenuControlsComplete::RunTest(const FString&)
{
	UJamInputMap* Map = UJamInputMap::Get();
	if (!TestNotNull(TEXT("the input map"), Map) || !TestNotNull(TEXT("its context"), Map->Context.Get())) return false;
	const TArray<FJamControlRow>& Rows = UJamMenuFlow::ControlRows();
	TSet<FKey> Listed;
	for (const FJamControlRow& R : Rows)
	{
		TestTrue(*FString::Printf(TEXT("%s has keys, a pad button and a line"), *R.Verb), !R.Verb.IsEmpty() && !R.Keys.IsEmpty() && !R.Pad.IsEmpty() && !R.Note.IsEmpty());
		for (const FKey& K : R.Bound)
		{
			Listed.Add(K);
			const bool bBound = Map->Context->GetMappings().ContainsByPredicate([&](const FEnhancedActionKeyMapping& M) { return M.Key == K; });
			TestTrue(*FString::Printf(TEXT("%s names %s, which the game binds"), *R.Verb, *K.ToString()), bBound);
		}
	}
	int32 Bound = 0;
	for (const FEnhancedActionKeyMapping& M : Map->Context->GetMappings())
	{
		++Bound;
		TestTrue(*FString::Printf(TEXT("the bound key %s is on the controls screen"), *M.Key.ToString()), Listed.Contains(M.Key));
	}
	AddInfo(FString::Printf(TEXT("JAMMENU controls: %d rows, %d keys listed, %d mappings bound"), Rows.Num(), Listed.Num(), Bound));

	auto Row = [&](const TCHAR* Verb) -> const FJamControlRow*
	{
		return Rows.FindByPredicate([&](const FJamControlRow& R) { return R.Verb.StartsWith(Verb); });
	};
	const FJamControlRow* Shoot = Row(TEXT("SHOOT"));
	TestTrue(TEXT("shoot: left mouse, a tap is a flat shot"), Shoot && Shoot->Keys.Contains(TEXT("left mouse")) && Shoot->Note.Contains(TEXT("tap is a flat shot")) && Map->IsKeyMapped(Map->Shoot, EKeys::LeftMouseButton));
	const FJamControlRow* Check = Row(TEXT("CHECK"));
	TestTrue(TEXT("check: right mouse"), Check && Check->Keys.Contains(TEXT("right mouse")) && Map->IsKeyMapped(Map->Check, EKeys::RightMouseButton));
	const FJamControlRow* Lift = Row(TEXT("CROSS-CHECK"));
	// Lane A183 (R-0064 point 1): the cross-check is LMB without the puck, MMB and mouse 4 its second bindings; the pass is RMB with the puck, Q its second.
	TestTrue(TEXT("cross-check: left mouse without the puck, middle mouse too"), Lift && Lift->Verb.Contains(TEXT("WITHOUT")) && Lift->Keys.Contains(TEXT("left mouse")) && Lift->Keys.Contains(TEXT("middle mouse")) && Map->IsKeyMapped(Map->Lift, EKeys::MiddleMouseButton));
	const FJamControlRow* Pass = Row(TEXT("PASS"));
	TestTrue(TEXT("pass: right mouse with the puck or Q, held is a saucer"), Pass && Pass->Keys.Contains(TEXT("right mouse")) && Pass->Keys.Contains(TEXT("Q")) && Pass->Note.Contains(TEXT("saucer")) && Map->IsKeyMapped(Map->Pass, EKeys::Q));
	const FJamControlRow* Ability = Row(TEXT("ABILITY"));
	TestTrue(TEXT("ability: F"), Ability && Ability->Keys == TEXT("F") && Map->IsKeyMapped(Map->Ability, EKeys::F));
	const FJamControlRow* Spin = Row(TEXT("SPIN"));
	TestTrue(TEXT("spin: E"), Spin && Spin->Keys == TEXT("E") && Map->IsKeyMapped(Map->Spin, EKeys::E));
	const FJamControlRow* Deke = Row(TEXT("DEKE"));
	TestTrue(TEXT("deke: the wheel"), Deke && Deke->Keys.Contains(TEXT("wheel")) && Map->IsKeyMapped(Map->Deke, EKeys::MouseScrollUp));
	const FJamControlRow* Sprint = Row(TEXT("SPRINT"));
	TestTrue(TEXT("sprint: Shift"), Sprint && Sprint->Keys == TEXT("Shift") && Map->IsKeyMapped(Map->Sprint, EKeys::LeftShift));
	const FJamControlRow* Brake = Row(TEXT("BRAKE"));
	TestTrue(TEXT("brake: S against your motion"), Brake && Brake->Keys.Contains(TEXT("S against your motion")) && Map->IsKeyMapped(Map->Brake, EKeys::LeftControl));
	const FJamControlRow* Poke = Row(TEXT("POKE"));
	TestTrue(TEXT("poke: Space"), Poke && Poke->Keys == TEXT("Space") && Map->IsKeyMapped(Map->Poke, EKeys::SpaceBar));
	const FJamControlRow* Aim = Row(TEXT("AIM"));
	TestTrue(TEXT("the mouse aims"), Aim && Aim->Keys.Contains(TEXT("mouse")) && Map->IsKeyMapped(Map->StickMouse, EKeys::Mouse2D));
	return true;
}

// ---------------------------------------------------------------- lane A144: the player's level (evolution 2, then the level wire)

// A fresh profile is rookie; a picked level is the player's; the rule says which cut gets the player's level; the option the rink reads.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuFirstMatchLevel, "Jam.Menu.FirstMatchLevel", JamMenuTests::MenuTestFlags)
bool FJamMenuFirstMatchLevel::RunTest(const FString&)
{
	// What counts as a pick: the saved key, or a saved level only the button could have written.
	TestFalse(TEXT("nothing saved is no pick"), FJamSettings::IsDifficultyChosen(false, false, EJamMenuDifficulty::Pro));
	TestFalse(TEXT("a saved pro without the key is no pick (every volume change wrote it)"), FJamSettings::IsDifficultyChosen(false, true, EJamMenuDifficulty::Pro));
	TestTrue(TEXT("a saved all-star is a pick"), FJamSettings::IsDifficultyChosen(false, true, EJamMenuDifficulty::AllStar));
	TestTrue(TEXT("the saved key is a pick"), FJamSettings::IsDifficultyChosen(true, true, EJamMenuDifficulty::Pro));

	// A fresh profile starts at rookie: the level PLAY NOW plays, the button's line, the settings button.
	FJamSettings Fresh;
	TestFalse(TEXT("a fresh profile has no pick"), Fresh.bDifficultyChosen);
	TestEqual(TEXT("a fresh profile is rookie"), FJamSettings::DifficultyString(Fresh.EffectiveDifficulty()), FString(TEXT("rookie")));
	TestEqual(TEXT("a fresh profile plays rookie"), UJamMenuFlow::FirstMatchLevel(Fresh), FString(TEXT("rookie")));
	TestEqual(TEXT("the button says rookie"), UJamMenuFlow::FirstMatchLine(TEXT("rookie")), FString(TEXT("ONE MATCH, ROOKIE.  ENTER / A, OR CLICK.")));

	for (const EJamMenuDifficulty D : { EJamMenuDifficulty::Rookie, EJamMenuDifficulty::Pro, EJamMenuDifficulty::AllStar })
	{
		FJamSettings Picked;
		Picked.Difficulty = D;
		Picked.bDifficultyChosen = true;
		const FString Level = FJamSettings::DifficultyString(D);
		TestEqual(*FString::Printf(TEXT("a picked %s plays %s"), *Level, *Level), UJamMenuFlow::FirstMatchLevel(Picked), Level);
		TestTrue(*FString::Printf(TEXT("the button names %s"), *Level), UJamMenuFlow::FirstMatchLine(Level).Contains(FJamSettings::DifficultyLabel(D)));
		// THE RULE: the regular cuts play the player's level; a mode, a take, a saved cut and a path play their own (an empty level).
		TestEqual(*FString::Printf(TEXT("%s: PLAY NOW's cut plays the player's level"), *Level), UJamMenuFlow::PlayerLevelFor(UJamMenuFlow::FirstCut, false, Picked), Level);
		TestEqual(*FString::Printf(TEXT("%s: a cut card plays the player's level"), *Level), UJamMenuFlow::PlayerLevelFor(TEXT("hockey.raw"), false, Picked), Level);
		TestEqual(*FString::Printf(TEXT("%s: a Director's mode keeps its own"), *Level), UJamMenuFlow::PlayerLevelFor(TEXT("modes/hockey.mode.sudden-death"), false, Picked), FString());
		TestEqual(*FString::Printf(TEXT("%s: a saved cut or a played take keeps its own"), *Level), UJamMenuFlow::PlayerLevelFor(TEXT("My Director's Cut"), true, Picked), FString());
		TestEqual(*FString::Printf(TEXT("%s: a path (a season take) keeps its own"), *Level), UJamMenuFlow::PlayerLevelFor(TEXT("C:/x/ladder/take-1.cartridge.json"), false, Picked), FString());
		// The option the rink reads back.
		TestEqual(*FString::Printf(TEXT("%s travels as an option"), *Level), AHokejGameMode::LevelFromOptions(FString::Printf(TEXT("?cut=hockey.regular?level=%s"), *Level)), Level);
	}
	TestEqual(TEXT("no option, no level"), AHokejGameMode::LevelFromOptions(TEXT("?cut=hockey.regular")), FString());
	TestEqual(TEXT("no such level, no level"), AHokejGameMode::LevelFromOptions(TEXT("?cut=hockey.regular?level=legend")), FString());

	// The brain itself takes each level over a cartridge's own (the call the game mode makes), by its profile name.
	TArray<FString> Errors;
	const UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(UJamMenuFlow::FirstCut), Errors);
	if (!TestNotNull(TEXT("the first cut loads"), C)) return false;
	for (const TCHAR* Level : { TEXT("rookie"), TEXT("pro"), TEXT("all_star") })
	{
		UJamBrain* B = NewObject<UJamBrain>();
		B->ApplyCartridge(*C, Level);
		TestEqual(*FString::Printf(TEXT("the brain's profile is %s"), Level), B->Difficulty.ToString(), FString(Level));
	}
	UJamBrain* Own = NewObject<UJamBrain>();
	Own->ApplyCartridge(*C);
	TestEqual(TEXT("without a level the brain plays the cartridge's own"), Own->Difficulty.ToString(), UJamMenuFlow::CutLevel(*C));
	return true;
}

// ---- Jam.Menu.LevelReachesTheBrain: the whole wire, in the game. The menu starts a match, the rink's game mode gives the brain its level.
// (Evolution 3 adds two: the first screen's switch clicked and stepped, then PLAY NOW.) Five matches: PLAY NOW on a fresh profile (rookie), a regular cut with pro picked, with all-star picked (the player's level each time, the
// brain's profile name), the season's take with all-star picked (the take's own level), a Director's mode with rookie picked (the mode's own).

namespace JamMenuTests
{
	struct FLevelStep
	{
		const TCHAR* What;
		int32 Kind;                 // 0 PLAY NOW, 1 a regular cut card, 2 the season, 3 a Director's mode
		bool bChosen;
		EJamMenuDifficulty Picked;
		const TCHAR* Expect;        // the level the brain must play; null = the cartridge's own
	};
	static const FLevelStep LevelSteps[] = {
		{ TEXT("PLAY NOW on a fresh profile"), 0, false, EJamMenuDifficulty::Pro, TEXT("rookie") },
		{ TEXT("the regular cut with pro picked"), 1, true, EJamMenuDifficulty::Pro, TEXT("pro") },
		{ TEXT("PLAY NOW with all-star picked"), 0, true, EJamMenuDifficulty::AllStar, TEXT("all_star") },
		{ TEXT("the season's take with all-star picked"), 2, true, EJamMenuDifficulty::AllStar, nullptr },
		{ TEXT("Sudden Death with rookie picked"), 3, true, EJamMenuDifficulty::Rookie, nullptr },
		// Evolution 3: the first screen's switch, then PLAY NOW. A fresh profile each time.
		{ TEXT("the switch clicked to ALL-STAR, then PLAY NOW"), 4, false, EJamMenuDifficulty::Pro, TEXT("all_star") },
		{ TEXT("the switch stepped right from ROOKIE, then PLAY NOW"), 5, false, EJamMenuDifficulty::Pro, TEXT("pro") },
	};
	static const int32 NumLevelSteps = UE_ARRAY_COUNT(LevelSteps);
}

class FJamLevelWireCommand : public IAutomationLatentCommand
{
public:
	explicit FJamLevelWireCommand(FAutomationTestBase* InTest) : Test(InTest) {}

	virtual bool Update() override
	{
		using namespace JamMenuTests;
		UWorld* W = GameWorld();
		if (++Ticks > 2400) { Test->AddError(FString::Printf(TEXT("step %d (%s) timed out in phase %d"), Step, Step < NumLevelSteps ? LevelSteps[Step].What : TEXT("end"), Phase)); return Finish(); }
		if (Step >= NumLevelSteps)
		{
			// Back at the menu once more, then done.
			AJamMenuGameMode* Menu = W ? Cast<AJamMenuGameMode>(W->GetAuthGameMode()) : nullptr;
			return Menu && Menu->GetMenu() ? Finish() : false;
		}
		const FLevelStep& S = LevelSteps[Step];
		if (Phase == 0)
		{
			AJamMenuGameMode* GM = W ? Cast<AJamMenuGameMode>(W->GetAuthGameMode()) : nullptr;
			if (!GM || !GM->GetMenu()) return false;
			FJamSettings Settings;
			Settings.Difficulty = S.Picked;
			Settings.bDifficultyChosen = S.bChosen;
			FJamSettings::SetOverrideForTest(&Settings);
			bool bStarted = false;
			switch (S.Kind)
			{
			case 0: bStarted = GM->GetMenu()->PlayNow(); break;
			case 4:
			case 5:
				// The switch writes the setting (into the test's in-memory profile), PLAY NOW's line follows it, and the match plays it.
				if (S.Kind == 4) GM->GetMenu()->PickLevel(EJamMenuDifficulty::AllStar); else GM->GetMenu()->StepLevel(1);
				Test->TestEqual(*FString::Printf(TEXT("%s: PLAY NOW's line names the level"), S.What), GM->GetMenu()->GetFirstPressLine(), UJamMenuFlow::FirstMatchLine(S.Expect));
				Test->TestTrue(*FString::Printf(TEXT("%s: the switch is the player's pick"), S.What), FJamSettings::Load().bDifficultyChosen);
				bStarted = GM->GetMenu()->PlayNow();
				break;
			case 1: bStarted = GM->GetMenu()->ChooseCut(TEXT("hockey.regular")); break;
			case 2:
				// The season's own start; where this machine's season cannot start (wrapped, no progress file), the call the season makes for a
				// take (UJamProgression::StartTake: StartMatch with the take's path, no level).
				bStarted = GM->GetMenu()->PlaySeason();
				if (!bStarted)
				{
					Test->AddInfo(TEXT("the season did not start on this machine: take 1 is started by its path, as the season starts a take"));
					UJamMenuFlow::StartMatch(W, UJamCartridge::ResolveCartridgePath(TEXT("ladder/take-1")));
					bStarted = true;
				}
				break;
			default: bStarted = GM->GetMenu()->ChooseCut(TEXT("modes/hockey.mode.sudden-death")); break;
			}
			if (!bStarted) { Test->AddError(FString::Printf(TEXT("%s: the menu did not start a match"), S.What)); return Finish(); }
			Phase = 1;
			Ticks = 0;
			return false;
		}
		AHokejGameMode* GM = W ? Cast<AHokejGameMode>(W->GetAuthGameMode()) : nullptr;
		if (!GM || !GM->GetSim() || !GM->GetBrain() || !GM->GetCartridge() || GM->GetSim()->GetFrame() < 30) return false;
		const FString Own = UJamMenuFlow::CutLevel(*GM->GetCartridge());
		const FString Brain = GM->GetBrain()->Difficulty.ToString();
		Test->AddInfo(FString::Printf(TEXT("JAMMENU level wire: %s -> brain=%s source=%s cartridge_own=%s cut=%s"), S.What, *Brain,
			GM->IsLevelFromSettings() ? TEXT("settings") : TEXT("cartridge"), *Own, *GM->GetCartridge()->Id));
		Test->TestEqual(*FString::Printf(TEXT("%s: the game mode names the brain's profile"), S.What), GM->GetLevelInForce(), Brain);
		if (S.Expect)
		{
			Test->TestEqual(*FString::Printf(TEXT("%s: the brain's profile"), S.What), Brain, FString(S.Expect));
			Test->TestTrue(*FString::Printf(TEXT("%s: the level is the player's"), S.What), GM->IsLevelFromSettings());
			Test->TestEqual(*FString::Printf(TEXT("%s: it is the regular cut"), S.What), GM->GetCartridge()->Id, FString(TEXT("hockey-regular")));
		}
		else
		{
			Test->TestEqual(*FString::Printf(TEXT("%s: the brain plays the cartridge's own level"), S.What), Brain, Own);
			Test->TestFalse(*FString::Printf(TEXT("%s: the setting is ignored"), S.What), GM->IsLevelFromSettings());
			const FString Picked = FJamSettings::DifficultyString(S.Picked);
			if (Own == Picked) Test->AddInfo(FString::Printf(TEXT("%s: the cartridge's own level happens to be the picked one (%s): this step proves the source only"), S.What, *Own));
		}
		if (UJamDirectorsCallController* Ctl = W->GetSubsystem<UJamDirectorsCallController>())
		{
			if (Ctl->GetOverlay())
			{
				const FString Sub = Ctl->GetOverlay()->GetTakeSubText();
				const FString Label = UJamDirectorsCallHudWidget::LevelLabel(Brain);
				Test->TestTrue(*FString::Printf(TEXT("%s: the match card shows the level (%s)"), S.What, *Sub), Sub.Contains(TEXT("COACH'S NOTE")) || Sub.EndsWith(Label));
			}
		}
		UJamMenuFlow::ReturnToMenu(W);
		++Step;
		Phase = 0;
		Ticks = 0;
		return false;
	}

private:
	bool Finish()
	{
		FJamSettings::SetOverrideForTest(nullptr);
		return true;
	}

	FAutomationTestBase* Test;
	int32 Step = 0;
	int32 Phase = 0;
	int32 Ticks = 0;
};

// ---- Jam.Menu.LaunchCutWins (Director, 14 Nov): with -cut=<cut> on the command line PLAY NOW starts that cut, not the regular one, and the match
// plays the cartridge's own level whatever the settings say (the acceptance run launches the package with its own cut and presses Return on the
// first screen). The process's own command line is the seam: the launch argument is appended for the test and the line is put back after it.
class FJamLaunchCutCommand : public IAutomationLatentCommand
{
public:
	FJamLaunchCutCommand(FAutomationTestBase* InTest, const FString& InCut) : Test(InTest), Cut(InCut), SavedCommandLine(FCommandLine::Get()) {}

	virtual bool Update() override
	{
		UWorld* W = GameWorld();
		if (++Ticks > 2400) { Test->AddError(FString::Printf(TEXT("the launch cut test timed out in phase %d"), Phase)); return Finish(); }
		if (Phase == 0)
		{
			AJamMenuGameMode* GM = W ? Cast<AJamMenuGameMode>(W->GetAuthGameMode()) : nullptr;
			if (!GM || !GM->GetMenu()) return false;
			// The player picked all-star: the launch cut must not take it.
			FJamSettings Settings;
			Settings.Difficulty = EJamMenuDifficulty::AllStar;
			Settings.bDifficultyChosen = true;
			FJamSettings::SetOverrideForTest(&Settings);
			FCommandLine::Append(*FString::Printf(TEXT(" -cut=\"%s\""), *Cut));
			bAppended = true;
			if (!Test->TestTrue(TEXT("PLAY NOW starts a match"), GM->GetMenu()->PlayNow())) return Finish();
			Phase = 1;
			Ticks = 0;
			return false;
		}
		if (Phase == 1)
		{
			AHokejGameMode* GM = W ? Cast<AHokejGameMode>(W->GetAuthGameMode()) : nullptr;
			if (!GM || !GM->GetSim() || !GM->GetBrain() || !GM->GetCartridge() || GM->GetSim()->GetFrame() < 30) return false;
			TArray<FString> Errors;
			const UJamCartridge* Want = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(Cut), Errors);
			if (Test->TestNotNull(TEXT("the launch cut loads"), Want))
			{
				Test->TestEqual(TEXT("PLAY NOW started the launch cut"), GM->GetCartridge()->Id, Want->Id);
				Test->TestNotEqual(TEXT("and not the regular cut"), GM->GetCartridge()->Id, FString(TEXT("hockey-regular")));
				Test->TestEqual(TEXT("the brain plays the launch cut's own level"), GM->GetBrain()->Difficulty.ToString(), UJamMenuFlow::CutLevel(*Want));
			}
			Test->TestFalse(TEXT("the settings' level is ignored"), GM->IsLevelFromSettings());
			Test->TestEqual(TEXT("the source is the launch"), GM->GetLevelSource(), FString(TEXT("launch")));
			Test->AddInfo(FString::Printf(TEXT("JAMMENU launch cut: -cut=%s -> id=%s brain=%s source=%s (settings: all_star picked)"), *Cut, *GM->GetCartridge()->Id,
				*GM->GetBrain()->Difficulty.ToString(), *GM->GetLevelSource()));
			// The command line goes back before the menu returns, so no later match of this process sees the argument.
			Restore();
			UJamMenuFlow::ReturnToMenu(W);
			Phase = 2;
			Ticks = 0;
			return false;
		}
		AJamMenuGameMode* Menu = W ? Cast<AJamMenuGameMode>(W->GetAuthGameMode()) : nullptr;
		return Menu && Menu->GetMenu() ? Finish() : false;
	}

private:
	void Restore()
	{
		if (bAppended) { FCommandLine::Set(*SavedCommandLine); bAppended = false; }
		FJamSettings::SetOverrideForTest(nullptr);
	}
	bool Finish()
	{
		Restore();
		return true;
	}

	FAutomationTestBase* Test;
	FString Cut;
	FString SavedCommandLine;
	bool bAppended = false;
	int32 Phase = 0;
	int32 Ticks = 0;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuLaunchCutWins, "Jam.Menu.LaunchCutWins", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FJamMenuLaunchCutWins::RunTest(const FString&)
{
	FString Already;
	if (FParse::Value(FCommandLine::Get(), TEXT("-cut="), Already))
	{
		AddInfo(TEXT("this process was itself started with -cut=: the test would not prove its own argument; skipped"));
		return true;
	}
	// The acceptance run's own cut when the tree has it (rookie bots on purpose), else a shipped cut that is not the regular one.
	FString Cut = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("../tools/qa/accept/accept-cut.json")));
	if (!FPaths::FileExists(Cut)) Cut = TEXT("hockey.raw");
	if (!AutomationOpenMap(UJamMenuFlow::MenuMap))
	{
		AddError(FString::Printf(TEXT("cannot open %s"), UJamMenuFlow::MenuMap));
		return false;
	}
	ADD_LATENT_AUTOMATION_COMMAND(FJamLaunchCutCommand(this, Cut));
	ADD_LATENT_AUTOMATION_COMMAND(FExitGameCommand());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuLevelReachesTheBrain, "Jam.Menu.LevelReachesTheBrain", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FJamMenuLevelReachesTheBrain::RunTest(const FString&)
{
	if (!AutomationOpenMap(UJamMenuFlow::MenuMap))
	{
		AddError(FString::Printf(TEXT("cannot open %s"), UJamMenuFlow::MenuMap));
		return false;
	}
	ADD_LATENT_AUTOMATION_COMMAND(FJamLevelWireCommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FExitGameCommand());
	return true;
}

// The brand line follows the first cut at run time: the whistle line where the cut has no whistle, the law where it has one. Both trees pass:
// the rule is tested for both values, the line on screen against the cut on disk.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuBrandLine, "Jam.Menu.BrandLineFollowsTheCut", JamMenuTests::MenuTestFlags)
bool FJamMenuBrandLine::RunTest(const FString&)
{
	TestEqual(TEXT("no whistle in the first cut: the banner"), UJamMenuFlow::BrandLineFor(false), FString(TEXT("Hockey with the whistle taken out.")));
	TestEqual(TEXT("a whistle in the first cut: the law"), UJamMenuFlow::BrandLineFor(true), FString(TEXT("The speed you bring is the hit you get.")));
	TArray<FString> Errors;
	const UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(UJamMenuFlow::FirstCut), Errors);
	if (!TestNotNull(TEXT("the first cut loads"), C)) return false;
	TestEqual(TEXT("the line on the first screen is the one for the cut on disk"), UJamMenuFlow::BrandLine(), UJamMenuFlow::BrandLineFor(C->Game.Rules.bPenalties));
	AddInfo(FString::Printf(TEXT("JAMMENU brand line: rules.penalties %s -> %s"), C->Game.Rules.bPenalties ? TEXT("true") : TEXT("false"), *UJamMenuFlow::BrandLine()));
	return true;
}

// ---------------------------------------------------------------- lane A144, evolution 3

// A profile saved before the level wire (Save() wrote "pro" with every volume change, and no pick key) shows ROOKIE; the player who then picks PRO
// keeps PRO across restarts. The saved form is the one Load() reads and Save() writes (FromSaved / ToSaved): a restart is a round trip through it.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuOldProfileThenPro, "Jam.Menu.OldProfileThenPro", JamMenuTests::MenuTestFlags)
bool FJamMenuOldProfileThenPro::RunTest(const FString&)
{
	TMap<FString, FString> Old;
	Old.Add(TEXT("Difficulty"), TEXT("pro"));
	Old.Add(TEXT("Volume"), TEXT("0.650000"));
	Old.Add(TEXT("Barks"), TEXT("-1"));
	const FJamSettings Met = FJamSettings::FromSaved(Old);
	TestFalse(TEXT("an old profile's saved pro is no pick"), Met.bDifficultyChosen);
	TestEqual(TEXT("the old profile shows ROOKIE"), FJamSettings::DifficultyLabel(Met.EffectiveDifficulty()), FString(TEXT("ROOKIE")));
	TestEqual(TEXT("the old profile plays rookie"), UJamMenuFlow::FirstMatchLevel(Met), FString(TEXT("rookie")));
	TestTrue(TEXT("its volume is kept"), FMath::IsNearlyEqual(Met.Volume, 0.65f, 0.001f));

	// The player picks PRO (the switch and the SETTINGS button write the same two fields), the game saves, the game restarts.
	FJamSettings Picked = Met;
	Picked.Difficulty = EJamMenuDifficulty::Pro;
	Picked.bDifficultyChosen = true;
	const TMap<FString, FString> Saved = Picked.ToSaved();
	TestTrue(TEXT("the pick is saved with its key"), Saved.Contains(TEXT("DifficultyChosen")) && Saved[TEXT("DifficultyChosen")].ToBool() && Saved[TEXT("Difficulty")] == TEXT("pro"));
	FJamSettings After = FJamSettings::FromSaved(Saved);
	for (int32 Restart = 1; Restart <= 3; ++Restart)
	{
		TestTrue(*FString::Printf(TEXT("restart %d: PRO is still the player's pick"), Restart), After.bDifficultyChosen && After.EffectiveDifficulty() == EJamMenuDifficulty::Pro);
		TestEqual(*FString::Printf(TEXT("restart %d: PLAY NOW plays pro"), Restart), UJamMenuFlow::FirstMatchLevel(After), FString(TEXT("pro")));
		TestTrue(*FString::Printf(TEXT("restart %d: the volume is kept"), Restart), FMath::IsNearlyEqual(After.Volume, 0.65f, 0.001f));
		After = FJamSettings::FromSaved(After.ToSaved());
	}
	// An old profile that had all-star saved could only have got it from the button: it stays the player's.
	TMap<FString, FString> OldStar;
	OldStar.Add(TEXT("Difficulty"), TEXT("all_star"));
	TestTrue(TEXT("an old all-star stays all-star"), FJamSettings::FromSaved(OldStar).EffectiveDifficulty() == EJamMenuDifficulty::AllStar);
	TestTrue(TEXT("nothing saved at all is a fresh profile: rookie"), FJamSettings::FromSaved(TMap<FString, FString>()).EffectiveDifficulty() == EJamMenuDifficulty::Rookie);
	return true;
}

// The same through the game's own Save() and Load() and the config system they use, in a scratch section of the settings file's cache (emptied
// after; the player's section is not touched): the old profile is the three lines an old package wrote, the pick is saved under its key and read
// back. What this cannot show is the engine writing the file to disk: a process started with -Multiprocess (the gate, the off-screen stills)
// writes no config file at all; a package does (its [Jam.Settings] section is on this machine from earlier packages).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuPickSurvivesSaveAndLoad, "Jam.Menu.PickSurvivesSaveAndLoad", JamMenuTests::MenuTestFlags)
bool FJamMenuPickSurvivesSaveAndLoad::RunTest(const FString&)
{
	if (!GConfig) { AddError(TEXT("no config system")); return false; }
	const TCHAR* RealSection = FJamSettings::Section;
	FJamSettings::Section = TEXT("Jam.Settings.A144Test");
	FJamSettings::SetOverrideForTest(nullptr);
	ON_SCOPE_EXIT
	{
		GConfig->EmptySection(FJamSettings::Section, GGameUserSettingsIni);
		GConfig->Flush(false, GGameUserSettingsIni);   // where this process writes config files at all, the scratch section leaves the file too
		FJamSettings::Section = RealSection;
	};
	GConfig->EmptySection(FJamSettings::Section, GGameUserSettingsIni);
	TestFalse(TEXT("nothing saved: a fresh profile, rookie"), FJamSettings::Load().bDifficultyChosen);
	TestTrue(TEXT("nothing saved plays rookie"), FJamSettings::Load().EffectiveDifficulty() == EJamMenuDifficulty::Rookie);

	// What a package of before tonight left behind.
	GConfig->SetString(FJamSettings::Section, TEXT("Difficulty"), TEXT("pro"), GGameUserSettingsIni);
	GConfig->SetString(FJamSettings::Section, TEXT("Volume"), TEXT("1"), GGameUserSettingsIni);
	GConfig->SetString(FJamSettings::Section, TEXT("Barks"), TEXT("-1"), GGameUserSettingsIni);
	FJamSettings Met = FJamSettings::Load();
	TestFalse(TEXT("the old profile's pro is no pick"), Met.bDifficultyChosen);
	TestTrue(TEXT("the old profile shows ROOKIE"), Met.EffectiveDifficulty() == EJamMenuDifficulty::Rookie);

	// The player picks PRO; the game saves; the next start loads.
	Met.Difficulty = EJamMenuDifficulty::Pro;
	Met.bDifficultyChosen = true;
	Met.Volume = 0.7f;
	Met.Save();
	FString Key;
	TestTrue(TEXT("Save() writes the pick's key"), GConfig->GetString(FJamSettings::Section, TEXT("DifficultyChosen"), Key, GGameUserSettingsIni) && Key.ToBool());
	const FJamSettings Next = FJamSettings::Load();
	TestTrue(TEXT("Load() reads the pick back: PRO stays PRO"), Next.bDifficultyChosen && Next.EffectiveDifficulty() == EJamMenuDifficulty::Pro);
	TestTrue(TEXT("and the rest of the profile with it"), FMath::IsNearlyEqual(Next.Volume, 0.7f, 0.001f) && Next.Barks == -1);
	TestEqual(TEXT("PLAY NOW then plays pro"), UJamMenuFlow::FirstMatchLevel(Next), FString(TEXT("pro")));
	return true;
}

// The pause card's controls table stays inside the card: every line, with the card's own font, within its inner width; and it still names every
// verb of the binding table.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuPauseCardFits, "Jam.Menu.PauseCardFits", JamMenuTests::MenuTestFlags)
bool FJamMenuPauseCardFits::RunTest(const FString&)
{
	if (!FSlateApplication::IsInitialized()) { AddError(TEXT("no Slate application to measure with")); return false; }
	const TSharedRef<FSlateFontMeasure> Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	const FString Table = UJamDirectorsCallWidget::PauseControlsText();
	TArray<FString> Lines;
	Table.ParseIntoArrayLines(Lines);
	TestEqual(TEXT("five lines"), Lines.Num(), 5);
	float Widest = 0.f;
	for (const FString& Line : Lines)
	{
		const float W = static_cast<float>(Measure->Measure(Line, FJamUiStyle::CaptionFont()).X);
		Widest = FMath::Max(Widest, W);
		TestTrue(*FString::Printf(TEXT("'%s' fits the card (%.0f of %.0f)"), *Line.Left(40), W, UJamDirectorsCallWidget::PauseCardInnerWidth), W > 0.f && W <= UJamDirectorsCallWidget::PauseCardInnerWidth);
	}
	AddInfo(FString::Printf(TEXT("JAMMENU pause card: widest controls line %.0f of %.0f"), Widest, UJamDirectorsCallWidget::PauseCardInnerWidth));
	TArray<FString> Rows;
	UJamInputMap::BindingTableText(false).ParseIntoArrayLines(Rows);
	TestTrue(TEXT("the binding table has rows"), Rows.Num() >= 10);
	for (const FString& Row : Rows)
	{
		FString Verb, Keys;
		if (!Row.Split(TEXT("  "), &Verb, &Keys)) continue;
		TestTrue(*FString::Printf(TEXT("the pause card names %s"), *Verb), Table.Contains(Verb));
	}
	return true;
}

// ---------------------------------------------------------------- lane A164, evolution 4: the first screen has no dead key

namespace JamMenuTests
{
	// The profile as the game saved it: both keys of the level, read from the config system (the scratch section this test saves into).
	static FString SavedLevelKeys()
	{
		FString D, C;
		if (GConfig)
		{
			GConfig->GetString(FJamSettings::Section, TEXT("Difficulty"), D, GGameUserSettingsIni);
			GConfig->GetString(FJamSettings::Section, TEXT("DifficultyChosen"), C, GGameUserSettingsIni);
		}
		return FString::Printf(TEXT("Difficulty=%s DifficultyChosen=%s"), *D, *C);
	}

	static void FocusPlayNow(UJamMenuWidget* Menu)
	{
		// What the first screen's first tick does (ShowScreen -> PendingFocus); a null-RHI game ticks no widget, so the test does it.
		if (Menu && Menu->GetFirstPress()) Menu->GetFirstPress()->SetFocus();
	}

	static void FocusMenuItself(UJamMenuWidget* Menu)
	{
		// Where Slate leaves the keyboard focus after a click on something that takes none: the menu widget itself (the fault's state).
		if (Menu && Menu->GetCachedWidget().IsValid()) FSlateApplication::Get().SetUserFocus(0, Menu->GetCachedWidget(), EFocusCause::Mouse);
	}
}

// "Click PRO, press Return" plays pro; every way a level is picked saves both keys at once; a start reads them back. Two matches through the menu.
// The profile is the game's own Save() and Load() through the config system in a scratch section (emptied after; the player's section and file
// are never touched), so the keys this test reads are the keys a package writes.
class FJamClickThenReturnCommand : public IAutomationLatentCommand
{
public:
	explicit FJamClickThenReturnCommand(FAutomationTestBase* InTest) : Test(InTest), RealSection(FJamSettings::Section) {}

	virtual bool Update() override
	{
		using namespace JamMenuTests;
		UWorld* W = GameWorld();
		if (++Ticks > 2400) { Test->AddError(FString::Printf(TEXT("timed out in phase %d"), Phase)); return Finish(); }
		AJamMenuGameMode* MenuGM = W ? Cast<AJamMenuGameMode>(W->GetAuthGameMode()) : nullptr;
		UJamMenuWidget* Menu = MenuGM ? MenuGM->GetMenu() : nullptr;
		switch (Phase)
		{
		case 0:
			if (!Menu) return false;
			FJamSettings::Section = TEXT("Jam.Settings.A164Test");
			FJamSettings::SetOverrideForTest(nullptr);
			bScratch = true;
			GConfig->EmptySection(FJamSettings::Section, GGameUserSettingsIni);
			Menu->ShowScreen(EJamMenuScreen::Main);
			return Next();
		case 1:
		{
			if (!Menu || Ticks < 10) return false;
			FocusPlayNow(Menu);
			Test->TestTrue(TEXT("the first screen: the focus on PLAY NOW"), Menu->IsFocusOnPlayNow());
			Test->TestTrue(TEXT("a fresh profile: the switch shows ROOKIE"), Menu->GetSwitchLevel() == EJamMenuDifficulty::Rookie);
			// Click PRO by its own click path (Slate's pointer routing down the segment's path).
			FString Why;
			if (!Menu->ClickForQa(Menu->GetLevelButton(EJamMenuDifficulty::Pro), Why))
			{
				Test->AddError(FString::Printf(TEXT("cannot click the PRO segment: %s"), *Why));
				return Finish();
			}
			Test->AddInfo(FString::Printf(TEXT("after the mouse-down Slate gave the focus to %s (the cause of the dead Return when it is the menu itself)"),
				Menu->WasLastClickDownOnMenu() ? TEXT("the menu widget itself") : TEXT("something else")));
			Test->TestTrue(TEXT("the click picked PRO"), Menu->GetSwitchLevel() == EJamMenuDifficulty::Pro);
			Test->TestEqual(TEXT("the click saved both keys in one save"), SavedLevelKeys(), FString(TEXT("Difficulty=pro DifficultyChosen=True")));
			Test->TestTrue(TEXT("after the click the focus is back on PLAY NOW"), Menu->IsFocusOnPlayNow());
			// Return, as a key given to Slate for the focused widget, in the same frame as the click.
			Menu->PressKeyForQa(EKeys::Enter);
			Expect = TEXT("pro");
			What = TEXT("click PRO, then Return");
			Phase = 3;
			return Next();
		}
		case 4:
		case 9:
			if (!CheckMatch(W)) return false;
			return Next();
		case 5:
		case 10:
			if (!Menu) return false;
			if (Phase == 10) return Finish();
			Menu->ShowScreen(EJamMenuScreen::Settings);
			return Next();
		case 6:
		{
			if (!Menu || Ticks < 6) return false;
			// The settings screen's level button: PRO -> ALL-STAR.
			FString Why;
			UJamSettingsWidget* Panel = Menu->GetSettingsPanel();
			if (!Panel || !Menu->ClickForQa(Panel->GetFirstFocusable(), Why))
			{
				Test->AddError(FString::Printf(TEXT("cannot click the settings' level button: %s"), Panel ? *Why : TEXT("no settings panel")));
				return Finish();
			}
			Test->TestEqual(TEXT("the settings' button saved both keys (all-star)"), SavedLevelKeys(), FString(TEXT("Difficulty=all_star DifficultyChosen=True")));
			// The QA hook (jam.menuscreen level rookie), the console's path.
			GEngine->Exec(W, TEXT("jam.menuscreen level rookie"));
			Test->TestEqual(TEXT("jam.menuscreen level saved both keys (rookie)"), SavedLevelKeys(), FString(TEXT("Difficulty=rookie DifficultyChosen=True")));
			Menu->ShowScreen(EJamMenuScreen::Main);
			return Next();
		}
		case 7:
			if (!Menu || Ticks < 6) return false;
			Test->TestTrue(TEXT("the first screen re-reads the profile: ROOKIE"), Menu->GetSwitchLevel() == EJamMenuDifficulty::Rookie);
			// The pad: D-pad right on PLAY NOW steps the level and keeps the focus.
			FocusPlayNow(Menu);
			Menu->PressKeyForQa(EKeys::Gamepad_DPad_Right);
			Test->TestEqual(TEXT("the pad's right saved both keys (pro)"), SavedLevelKeys(), FString(TEXT("Difficulty=pro DifficultyChosen=True")));
			Test->TestTrue(TEXT("the pad's right keeps the focus on PLAY NOW"), Menu->IsFocusOnPlayNow());
			{
				FString Why;
				if (!Menu->ClickForQa(Menu->GetLevelButton(EJamMenuDifficulty::AllStar), Why))
				{
					Test->AddError(FString::Printf(TEXT("cannot click the ALL-STAR segment: %s"), *Why));
					return Finish();
				}
			}
			Test->TestEqual(TEXT("the ALL-STAR click saved both keys"), SavedLevelKeys(), FString(TEXT("Difficulty=all_star DifficultyChosen=True")));
			// The pad's A reaching the menu widget itself (nothing of the screen focused) starts the match: no tick between, so the guard is not what helps.
			FocusMenuItself(Menu);
			Menu->PressKeyForQa(EKeys::Gamepad_FaceButton_Bottom);
			Expect = TEXT("all_star");
			What = TEXT("click ALL-STAR, focus on the menu itself, pad A");
			Phase = 8;
			return Next();
		default:
			return Finish();
		}
	}

private:
	bool Next() { ++Phase; Ticks = 0; return false; }

	bool CheckMatch(UWorld* W)
	{
		AHokejGameMode* GM = W ? Cast<AHokejGameMode>(W->GetAuthGameMode()) : nullptr;
		if (!GM || !GM->GetSim() || !GM->GetBrain() || !GM->GetCartridge() || GM->GetSim()->GetFrame() < 30) return false;
		const FString Brain = GM->GetBrain()->Difficulty.ToString();
		Test->AddInfo(FString::Printf(TEXT("JAMMENU click then key: %s -> brain=%s source=%s cut=%s saved: %s"), *What, *Brain, *GM->GetLevelSource(), *GM->GetCartridge()->Id,
			*JamMenuTests::SavedLevelKeys()));
		Test->TestEqual(*FString::Printf(TEXT("%s: the match plays the picked level"), *What), Brain, Expect);
		Test->TestEqual(*FString::Printf(TEXT("%s: the level is the player's"), *What), GM->GetLevelSource(), FString(TEXT("settings")));
		Test->TestEqual(*FString::Printf(TEXT("%s: PLAY NOW's cut"), *What), GM->GetCartridge()->Id, FString(TEXT("hockey-regular")));
		UJamMenuFlow::ReturnToMenu(W);
		return true;
	}

	bool Finish()
	{
		if (bScratch)
		{
			GConfig->EmptySection(FJamSettings::Section, GGameUserSettingsIni);
			GConfig->Flush(false, GGameUserSettingsIni);
			FJamSettings::Section = RealSection;
			bScratch = false;
		}
		return true;
	}

	FAutomationTestBase* Test;
	const TCHAR* RealSection;
	bool bScratch = false;
	int32 Phase = 0;
	int32 Ticks = 0;
	FString Expect;
	FString What;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuClickThenReturn, "Jam.Menu.ClickThenReturn", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FJamMenuClickThenReturn::RunTest(const FString&)
{
	if (!AutomationOpenMap(UJamMenuFlow::MenuMap))
	{
		AddError(FString::Printf(TEXT("cannot open %s"), UJamMenuFlow::MenuMap));
		return false;
	}
	ADD_LATENT_AUTOMATION_COMMAND(FJamClickThenReturnCommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FExitGameCommand());
	return true;
}

// Lane A195 (U2 evolution 6, R-0064 point 3): every screen a player meets is one product. The pause card, the end card, SETTINGS and CONTROLS are drawn
// from the front end's style table: the items in its look (the focus fill), key hints in its caps, and the words of the front end (no film words: no
// "cut", no "Director" outside MAKE YOUR OWN). The end card's restart key follows the bindings (A183: it is rebindable).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMenuEveryScreenOneLook, "Jam.Menu.EveryScreenOneLook", JamMenuTests::MenuTestFlags)
bool FJamMenuEveryScreenOneLook::RunTest(const FString&)
{
	const FJamSettings Fresh = FJamSettings();
	FJamSettings::SetOverrideForTest(&Fresh);
	ON_SCOPE_EXIT { FJamSettings::SetOverrideForTest(nullptr); };
	const FJamBindings Before = UJamInputMap::Get()->GetBindings();
	UJamInputMap::Get()->ApplyBindings(FJamBindings::Defaults());
	ON_SCOPE_EXIT { UJamInputMap::Get()->ApplyBindings(Before); };
	JamMenuTests::FMenuFixture F;
	if (!TestNotNull(TEXT("the menu builds in a test world"), F.Menu)) return false;
	auto NoFilmWords = [this](const FString& Where, const FString& Text)
	{
		const FString U = Text.ToUpper();
		TestFalse(*FString::Printf(TEXT("%s names no cut ('%s')"), *Where, *Text.Left(60)), U.Contains(TEXT("CUT'S")) || U.Contains(TEXT(" CUTS")) || U.Contains(TEXT("CUT ")));
		TestFalse(*FString::Printf(TEXT("%s names no Director ('%s')"), *Where, *Text.Left(60)), U.Contains(TEXT("DIRECTOR")));
	};

	// The pause card: the five items in their order and words, the hints in caps, the item look.
	UJamDirectorsCallWidget* P = CreateWidget<UJamDirectorsCallWidget>(F.World, UJamDirectorsCallWidget::StaticClass());
	if (TestNotNull(TEXT("the pause card builds"), P))
	{
		P->AddToRoot();
		ON_SCOPE_EXIT { P->ReleaseSlateResources(true); P->RemoveFromRoot(); };
		P->TakeWidget();
		P->ShowPause();
		const TArray<FString> Labels = P->GetPauseLabels();
		// The test world has no Designer, so the rule item reads "CHANGE A RULE  (USED THIS PERIOD)" here: its first words are held.
		TestTrue(TEXT("the pause items"), Labels.Num() == 5 && Labels[0] == TEXT("RESUME") && Labels[1].StartsWith(TEXT("CHANGE A RULE")) && Labels[2] == TEXT("RESTART") && Labels[3] == TEXT("CONTROLS") && Labels[4] == TEXT("QUIT TO MENU"));
		AddInfo(FString::Printf(TEXT("JAMMENU pause items: %s"), *FString::Join(Labels, TEXT(" | "))));
		for (const FString& L : Labels) NoFilmWords(TEXT("the pause card"), L);
		TestEqual(TEXT("the pause hints"), P->GetHintLine(), FString(TEXT("ESC / START RESUME   ENTER / A SELECT   R / VIEW RESTART")));
		TestEqual(TEXT("RESUME is in the item look at rest before a tick"), P->GetPaintedState(P->GetResumeButton()), -1);
		P->ShowCall();
		TestTrue(TEXT("the rule card's hints say how to slide"), P->GetHintLine().Contains(TEXT("LEFT / RIGHT SLIDE")));
		P->ShowControls();
		TestTrue(TEXT("CONTROLS in the pause card says how to rebind"), P->GetHintLine().Contains(TEXT("ENTER / A REBIND")));
		P->ShowPause();
	}

	// The end card: the key line as caps, the restart key from the bindings; the season's lines keep their words, the R becomes the bound key.
	{
		FString Note;
		TArray<TPair<FString, FString>> H = UJamHudWidget::EndCardHints(UJamHudWidget::EndCardKeys(), Note);
		TestEqual(TEXT("the match's end keys"), FJamMenuLook::HintsLine(H), FString(TEXT("R / VIEW REMATCH   ESC / START MENU")));
		TestTrue(TEXT("no note on a match's end card"), Note.IsEmpty());
		H = UJamHudWidget::EndCardHints(TEXT("R  NEXT MATCH: THE BIG ROOM      ESC  MENU"), Note);
		TestEqual(TEXT("the between-takes card's keys"), FJamMenuLook::HintsLine(H), FString(TEXT("R / VIEW NEXT MATCH: THE BIG ROOM   ESC / START MENU")));
		H = UJamHudWidget::EndCardHints(TEXT("SEASON COMPLETE      R  MENU"), Note);
		TestEqual(TEXT("the season's last card: its line"), Note, FString(TEXT("SEASON COMPLETE")));
		TestEqual(TEXT("the season's last card: its key"), FJamMenuLook::HintsLine(H), FString(TEXT("R / VIEW MENU")));
		FJamBindings Re = FJamBindings::Defaults();
		Re.Rebind(EJamVerb::Restart, EKeys::T);
		UJamInputMap::Get()->ApplyBindings(Re);
		TestEqual(TEXT("a rebound restart: the end card's line follows"), UJamHudWidget::EndCardKeys(), FString(TEXT("T  REMATCH      ESC  MENU")));
		H = UJamHudWidget::EndCardHints(TEXT("R  NEXT MATCH: THE BIG ROOM      ESC  MENU"), Note);
		TestTrue(TEXT("a rebound restart: the season's card shows the bound key"), H.Num() == 2 && H[0].Key == TEXT("T / VIEW"));
		UJamInputMap::Get()->ApplyBindings(FJamBindings::Defaults());
	}

	// SETTINGS and CONTROLS: the words of the front end, and the buttons in the item look.
	NoFilmWords(TEXT("the look's label"), FJamSettings::TierLabel(FString()));
	NoFilmWords(TEXT("the barks' label"), FJamSettings::BarksLabel(-1, FString()));
	NoFilmWords(TEXT("the barks' label with an intensity"), FJamSettings::BarksLabel(-1, TEXT("loud")));
	const FButtonStyle Focus = FJamMenuLook::ItemStyle(2);
	TestTrue(TEXT("the focus look is the away orange fill (or device B's focused plate)"), Focus.Normal.TintColor.GetSpecifiedColor().Equals(FJamMenuLook::Focus(), 0.01f) || Focus.Normal.GetResourceObject() != nullptr || Focus.Normal.DrawAs == ESlateBrushDrawType::RoundedBox);
	F.Menu->ShowScreen(EJamMenuScreen::Settings);
	if (UJamSettingsWidget* S = F.Menu->GetSettingsPanel())
	{
		TestEqual(TEXT("SETTINGS: seven places to stand"), S->GetFocusables().Num(), 7);
		TestTrue(TEXT("SETTINGS opens on the level"), S->GetFirstFocusable() == S->GetFocusables()[0]);
	}
	return true;
}

#endif
