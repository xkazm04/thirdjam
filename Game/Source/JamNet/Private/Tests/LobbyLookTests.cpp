// Lane A195 (track U2, evolution 7): PLAY WITH A FRIEND is one product with the front end. The lobby lives in JamNet (JamGame's tests cannot reach it),
// so its two checks sit here beside the menu's (a name under another test's name hides that test): Jam.Menu.LobbyOneLook (rinks, not cuts; no Director) and
// Jam.Menu.LobbyFitsTheScreen (its panel inside 1920 x 1080 units, the three shipped sizes being that one layout: FitsTheScreen holds the DPI curve).
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "LobbyWidget.h"
#include "MenuWidget.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamLobbyLookTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

	// The lobby built in a test world (no game instance: its tick reads no subsystem, so the rows stand as built).
	struct FLobbyFixture
	{
		UWorld* World = nullptr;
		ULobbyWidget* Lobby = nullptr;
		FLobbyFixture()
		{
			if (!GEngine || !FSlateApplication::IsInitialized()) return;
			World = UWorld::CreateWorld(EWorldType::Game, false, TEXT("JamLobbyLookWorld"));
			if (!World) return;
			GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
			Lobby = CreateWidget<ULobbyWidget>(World, ULobbyWidget::StaticClass());
			if (Lobby) { Lobby->AddToRoot(); Lobby->TakeWidget(); }
		}
		~FLobbyFixture()
		{
			if (Lobby) { Lobby->ReleaseSlateResources(true); Lobby->RemoveFromRoot(); }
			if (World) { GEngine->DestroyWorldContext(World); World->DestroyWorld(false); }
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLobbyOneLook, "Jam.Menu.LobbyOneLook", JamLobbyLookTests::Flags)
bool FJamLobbyOneLook::RunTest(const FString&)
{
	JamLobbyLookTests::FLobbyFixture F;
	if (!TestNotNull(TEXT("the lobby builds in a test world"), F.Lobby)) return false;
	const TArray<FString> Words = F.Lobby->GetWordsForTest();
	TestTrue(TEXT("the lobby's fixed words are there"), Words.Num() >= 5);
	for (const FString& W : Words)
	{
		const FString U = W.ToUpper();
		TestFalse(*FString::Printf(TEXT("the lobby names no cut ('%s')"), *W.Left(60)), U.Contains(TEXT(" CUT")) || U.Contains(TEXT("CUT'S")));
		TestFalse(*FString::Printf(TEXT("the lobby names no Director ('%s')"), *W.Left(60)), U.Contains(TEXT("DIRECTOR")));
	}
	TestEqual(TEXT("a rink by its name, not its id"), ULobbyWidget::RinkFor(TEXT("hockey.regular")), FString(TEXT("REGULAR")));
	TestEqual(TEXT("the large rink"), ULobbyWidget::RinkFor(TEXT("hockey.arena.large")), FString(TEXT("LARGE RINK")));
	TestFalse(TEXT("no rink's name carries CUT"), ULobbyWidget::RinkFor(TEXT("hockey.chaos")).Contains(TEXT("CUT")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLobbyFits, "Jam.Menu.LobbyFitsTheScreen", JamLobbyLookTests::Flags)
bool FJamLobbyFits::RunTest(const FString&)
{
	JamLobbyLookTests::FLobbyFixture F;
	if (!TestNotNull(TEXT("the lobby builds in a test world"), F.Lobby)) return false;
	UWidget* Panel = F.Lobby->GetPanelForTest();
	if (!TestNotNull(TEXT("the lobby has its panel"), Panel)) return false;
	TSharedRef<SWidget> S = Panel->TakeWidget();
	S->SlatePrepass(1.f);
	const FVector2D Size = S->GetDesiredSize();
	const float Want = ULobbyWidget::ColumnW + 2.f * FJamMenuLook::CardPad;
	AddInfo(FString::Printf(TEXT("JAMMENU measure lobby %.0f x %.0f (every row shown) of 1920 x 1080"), Size.X, Size.Y));
	TestTrue(*FString::Printf(TEXT("the lobby panel is its column and padding wide (%.0f of %.0f)"), Size.X, Want), FMath::IsNearlyEqual(Size.X, Want, 1.f));
	TestTrue(*FString::Printf(TEXT("the lobby panel fits 1080 units high with every row shown (%.0f)"), Size.Y), Size.Y > 200.f && Size.Y <= 1000.f);
	return true;
}


// Lane A195 (FAULT 6, the hardening lane): the lobby is no dead end for a pad or a keyboard: it opens with the focus on its first button (HOST), Esc
// and pad B leave it by BACK's road (HOME, the focus back on PLAY WITH A FRIEND), and Enter or A on a button stay the buttons' own.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLobbyFocus, "Jam.Menu.LobbyFocus", JamLobbyLookTests::Flags)
bool FJamLobbyFocus::RunTest(const FString&)
{
	JamLobbyLookTests::FLobbyFixture F;
	if (!TestNotNull(TEXT("the lobby builds in a test world"), F.Lobby)) return false;
	UWidget* First = F.Lobby->GetFirstFocusable();
	TestTrue(*FString::Printf(TEXT("the lobby's first place is HOST (%s)"), First ? *First->GetName() : TEXT("none")), First && First->GetName() == TEXT("HostButton") && First->GetIsEnabled());
	TestTrue(TEXT("Esc leaves the lobby"), ULobbyWidget::IsLeaveKey(EKeys::Escape));
	TestTrue(TEXT("pad B leaves the lobby"), ULobbyWidget::IsLeaveKey(EKeys::Gamepad_FaceButton_Right));
	TestFalse(TEXT("Enter is a button's, not a leave"), ULobbyWidget::IsLeaveKey(EKeys::Enter));
	TestFalse(TEXT("pad A is a button's, not a leave"), ULobbyWidget::IsLeaveKey(EKeys::Gamepad_FaceButton_Bottom));
	return true;
}

#endif
