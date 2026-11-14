#include "MenuGameMode.h"
#include "MenuWidget.h"
#include "SettingsWidget.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpectatorPawn.h"
#include "HAL/IConsoleManager.h"
#include "Engine/World.h"

// QA: jam.menuscreen <main|cuts|direct|settings|credits|controls> shows a screen; jam.menuscreen choose <cut name> picks a cut;
// jam.menuscreen say <sentence...> directs one; jam.menuscreen play presses PLAY NOW (lane A144). (R-0003 4: names grepped across Game/Source first.)
static FAutoConsoleCommandWithWorldAndArgs CmdJamMenuScreen(TEXT("jam.menuscreen"),
	TEXT("Menu QA: jam.menuscreen main|cuts|direct|settings|credits|controls | play | walk | choose <cut> | say <sentence>"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		AJamMenuGameMode* GM = World ? Cast<AJamMenuGameMode>(World->GetAuthGameMode()) : nullptr;
		UJamMenuWidget* Menu = GM ? GM->GetMenu() : nullptr;
		if (!Menu || !Args.Num()) { UE_LOG(LogTemp, Warning, TEXT("JAMMENU no menu in this world or no argument")); return; }
		const FString A = Args[0].ToLower();
		if (A == TEXT("main")) Menu->ShowScreen(EJamMenuScreen::Main);
		else if (A == TEXT("cuts")) Menu->ShowScreen(EJamMenuScreen::Cuts);
		else if (A == TEXT("direct")) Menu->ShowScreen(EJamMenuScreen::Direct);
		else if (A == TEXT("settings")) Menu->ShowScreen(EJamMenuScreen::Settings);
		else if (A == TEXT("credits")) Menu->ShowScreen(EJamMenuScreen::Credits);
		else if (A == TEXT("controls")) Menu->ShowScreen(EJamMenuScreen::Controls);
		else if (A == TEXT("play")) Menu->PlayNow();
		else if (A == TEXT("walk")) Menu->StartPadWalk();   // lane A144: the pad walk (JAMMENU walk lines)
		else if (A == TEXT("choose") && Args.Num() > 1) Menu->ChooseCut(Args[1]);
		else if (A == TEXT("say") && Args.Num() > 1)
		{
			TArray<FString> Rest(Args.GetData() + 1, Args.Num() - 1);
			Menu->ShowScreen(EJamMenuScreen::Direct);
			Menu->Direct(FString::Join(Rest, TEXT(" ")));
		}
		else UE_LOG(LogTemp, Warning, TEXT("JAMMENU unknown argument %s"), *Args[0]);
	}));

AJamMenuGameMode::AJamMenuGameMode()
{
	DefaultPawnClass = nullptr;
	bStartPlayersAsSpectators = true;
	SpectatorClass = ASpectatorPawn::StaticClass();
}

void AJamMenuGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (!NewPlayer || !NewPlayer->IsLocalController()) return;
	FJamSettings::Load().Apply();
	if (!Menu) Menu = UJamMenuWidget::CreateAndAdd(NewPlayer);
	NewPlayer->bShowMouseCursor = true;
	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	if (Menu) Mode.SetWidgetToFocus(Menu->TakeWidget());
	NewPlayer->SetInputMode(Mode);
}
