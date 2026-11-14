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
	TEXT("Menu QA: jam.menuscreen main|cuts|direct|settings|credits|controls | play | walk | where | click <rookie|pro|all_star|play|settingslevel> | level <rookie|pro|all_star|left|right> | key <left|right|up|down|arrowleft|arrowright|enter|a|b|esc> | choose <cut> | say <sentence>"),
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
		else if (A == TEXT("where")) Menu->PrintPlacesForQa();
		else if (A == TEXT("click") && Args.Num() > 1) Menu->ClickPlaceForQa(Args[1].ToLower());   // lane A164: a Slate-routed click, no cursor   // lane A164: the shown screen's places from the widgets' own geometry (JAMMENU place lines)
		else if (A == TEXT("key") && Args.Num() > 1)
		{
			// Lane A144 (evolution 3): a real key or pad press through Slate to the focused widget (no cursor, no window focus).
			static const TMap<FString, FKey> Keys = {
				{ TEXT("left"), EKeys::Gamepad_DPad_Left }, { TEXT("right"), EKeys::Gamepad_DPad_Right }, { TEXT("up"), EKeys::Gamepad_DPad_Up }, { TEXT("down"), EKeys::Gamepad_DPad_Down },
				{ TEXT("arrowleft"), EKeys::Left }, { TEXT("arrowright"), EKeys::Right }, { TEXT("enter"), EKeys::Enter }, { TEXT("a"), EKeys::Gamepad_FaceButton_Bottom },
				{ TEXT("b"), EKeys::Gamepad_FaceButton_Right }, { TEXT("esc"), EKeys::Escape } };
			if (const FKey* K = Keys.Find(Args[1].ToLower())) Menu->PressKeyForQa(*K);
			else UE_LOG(LogTemp, Warning, TEXT("JAMMENU unknown key %s"), *Args[1]);
		}
		else if (A == TEXT("level") && Args.Num() > 1)
		{
			// Lane A144 (evolution 3): the first screen's level switch: a segment by name (the click), or one step (left / right on PLAY NOW).
			const FString L = Args[1].ToLower();
			EJamMenuDifficulty D;
			if (L == TEXT("left")) Menu->StepLevel(-1);
			else if (L == TEXT("right")) Menu->StepLevel(1);
			else if (FJamSettings::ParseDifficulty(L, D)) Menu->PickLevel(D);
			else UE_LOG(LogTemp, Warning, TEXT("JAMMENU unknown level %s"), *Args[1]);
		}
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
