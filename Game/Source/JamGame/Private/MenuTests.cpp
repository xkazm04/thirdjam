// Lane A8 tests: Jam.Menu.*. The first two need no world; MenuToMatchAndBack opens Menu.umap (PIE in the editor), picks the
// regular cut, waits for the rink's game mode, quits to the menu and waits for the menu's game mode. Run:
//   UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Menu; Quit" -unattended -nullrhi -nosplash -log
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Misc/Paths.h"
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

#endif
