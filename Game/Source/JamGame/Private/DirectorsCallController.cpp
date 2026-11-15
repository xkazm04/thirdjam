#include "DirectorsCallController.h"
#include "DirectorsCallWidget.h"
#include "MenuFlow.h"
#include "SettingsWidget.h"
#include "HokejGameMode.h"
#include "LookTierSwitcher.h"
#include "InputMapping.h"
#include "JamPawns.h"
#include "JamCartridge.h"
#include "JamDesignerSubsystem.h"
#include "JamRuleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogJamDirectorsCall);

// QA: jam.pausemenu open|close|call|apply <key> <value>|restart|quit (R-0003 4: name grepped across Game/Source first).
static FAutoConsoleCommandWithWorldAndArgs CmdJamPauseMenu(TEXT("jam.pausemenu"),
	TEXT("Pause menu QA: jam.pausemenu open | close | call|rule | controls | leave | no | yes | apply <key> <value> | restart | quit"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UJamDirectorsCallController* C = World ? World->GetSubsystem<UJamDirectorsCallController>() : nullptr;
		if (!C || !C->IsActive() || !Args.Num()) { UE_LOG(LogJamDirectorsCall, Warning, TEXT("JAMCALL no active controller in this world or no argument")); return; }
		const FString A = Args[0].ToLower();
		if (A == TEXT("open")) C->Pause();
		else if (A == TEXT("close")) C->Resume();
		else if (A == TEXT("call") || A == TEXT("rule")) { C->Pause(); if (C->GetMenu()) C->GetMenu()->ShowCall(); }   // lane A195: "rule", the item's name now (CHANGE A RULE)
		else if (A == TEXT("controls")) { C->Pause(); if (C->GetMenu()) C->GetMenu()->ShowControls(); }   // lane A195: the pause card's CONTROLS
		else if (A == TEXT("leave")) { C->Pause(); if (C->GetMenu()) C->GetMenu()->ShowLeaveConfirm(); }   // lane A195 (F6): QUIT TO MENU's question
		else if (A == TEXT("no") || A == TEXT("yes")) { if (C->GetMenu() && C->GetMenu()->IsOnLeaveConfirm()) C->GetMenu()->AnswerLeave(A == TEXT("yes")); }
		else if (A == TEXT("apply") && Args.Num() > 2) { C->Pause(); if (C->GetMenu()) { C->GetMenu()->ShowCall(); C->GetMenu()->ApplyValue(FName(*Args[1]), FCString::Atod(*Args[2])); } }
		else if (A == TEXT("restart")) C->RestartMatch();
		else if (A == TEXT("quit")) C->QuitToMenu();
		else UE_LOG(LogJamDirectorsCall, Warning, TEXT("JAMCALL unknown argument %s"), *Args[0]);
	}));

UJamDirectorsCallController* UJamDirectorsCallController::Get(const UObject* WorldContext)
{
	UWorld* W = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull) : nullptr;
	return W ? W->GetSubsystem<UJamDirectorsCallController>() : nullptr;
}

bool UJamDirectorsCallController::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

AHokejGameMode* UJamDirectorsCallController::GetGameMode() const
{
	UWorld* W = GetWorld();
	return W ? Cast<AHokejGameMode>(W->GetAuthGameMode()) : nullptr;
}

UJamDesignerSubsystem* UJamDirectorsCallController::GetDesigner() const
{
	UWorld* W = GetWorld();
	UGameInstance* GI = W ? W->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UJamDesignerSubsystem>() : nullptr;
}

APlayerController* UJamDirectorsCallController::GetPC() const
{
	UWorld* W = GetWorld();
	return W ? W->GetFirstPlayerController() : nullptr;
}

int64 UJamDirectorsCallController::CurrentSimFrame() const
{
	AHokejGameMode* GM = GetGameMode();
	return GM && GM->GetSim() ? GM->GetSim()->GetFrame() : 0;
}

void UJamDirectorsCallController::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	AHokejGameMode* GM = Cast<AHokejGameMode>(InWorld.GetAuthGameMode());
	if (!GM) return;   // the menu world, or a test world without the hockey game mode
	bActive = true;
	if (UJamPresentation* P = InWorld.GetSubsystem<UJamPresentation>()) FrameHandle = P->OnSimFrame.AddUObject(this, &UJamDirectorsCallController::OnSimFrame);
	UGameInstance* GI = InWorld.GetGameInstance();
	if (UJamEventBus* Bus = GI ? GI->GetSubsystem<UJamEventBus>() : nullptr)
	{
		Bus->OnJamEvent.AddUniqueDynamic(this, &UJamDirectorsCallController::OnJamEvent);
		bBusBound = true;
	}
	if (UJamDesignerSubsystem* D = GI ? GI->GetSubsystem<UJamDesignerSubsystem>() : nullptr)
	{
		D->OnDirectorsCall.AddUniqueDynamic(this, &UJamDirectorsCallController::OnDirectorsCall);
	}
	EnsureWidgets();
	EnsurePauseBound();
	// The opening TAKE card: when the match was built before this subsystem listened, its first "period" event is gone.
	if (Overlay && !bOpeningCardShown && GM->GetSim())
	{
		Overlay->ShowTake(GM->GetTake(), CurrentSimFrame() + TakeCardFrames);
		bOpeningCardShown = true;
	}
	UE_LOG(LogJamDirectorsCall, Log, TEXT("JAMCALL controller active take=%d"), GM->GetTake());
}

void UJamDirectorsCallController::Deinitialize()
{
	if (UWorld* W = GetWorld())
	{
		if (UJamPresentation* P = W->GetSubsystem<UJamPresentation>()) P->OnSimFrame.Remove(FrameHandle);
		if (UGameInstance* GI = W->GetGameInstance())
		{
			if (UJamEventBus* Bus = GI->GetSubsystem<UJamEventBus>()) Bus->OnJamEvent.RemoveDynamic(this, &UJamDirectorsCallController::OnJamEvent);
			if (UJamDesignerSubsystem* D = GI->GetSubsystem<UJamDesignerSubsystem>()) D->OnDirectorsCall.RemoveDynamic(this, &UJamDirectorsCallController::OnDirectorsCall);
		}
	}
	bActive = false;
	Super::Deinitialize();
}

void UJamDirectorsCallController::EnsureWidgets()
{
	APlayerController* PC = GetPC();
	if (!PC) return;
	if (!Overlay) Overlay = UJamDirectorsCallHudWidget::CreateAndAdd(PC);
	if (!Menu) Menu = UJamDirectorsCallWidget::CreateAndAdd(PC, this);
}

void UJamDirectorsCallController::EnsurePauseBound()
{
	AHokejGameMode* GM = GetGameMode();
	AJamSkater* Human = GM ? GM->GetHumanSkater() : nullptr;
	UJamPlayerInput* In = Human ? Human->FindComponentByClass<UJamPlayerInput>() : nullptr;
	if (In && In != BoundInput.Get())
	{
		In->OnPause.AddUniqueDynamic(this, &UJamDirectorsCallController::HandlePausePressed);
		BoundInput = In;
	}
}

void UJamDirectorsCallController::ApplySettingsForMatch()
{
	const FJamSettings S = FJamSettings::Load();
	S.Apply();
	if (!S.LookTierOverride.IsEmpty())
	{
		if (ULookTierSwitcher* L = GetWorld() ? GetWorld()->GetSubsystem<ULookTierSwitcher>() : nullptr)
		{
			AHokejGameMode* GM = GetGameMode();   // R-0016 point 5 (lane A15): the rig scales with the running rink
			L->SetTier(S.LookTierOverride, GM && GM->GetSim() ? static_cast<float>(GM->GetSim()->GetConfig().Rink.LengthM) : 90.f,
				GM && GM->GetSim() ? static_cast<float>(GM->GetSim()->GetConfig().Rink.WidthM) : 45.f);   // lane A230: the width too
			UE_LOG(LogJamDirectorsCall, Log, TEXT("JAMCALL look tier override %s"), *S.LookTierOverride);
		}
	}
}

void UJamDirectorsCallController::OnSimFrame(const FJamPresentationFrame& Frame)
{
	AHokejGameMode* GM = GetGameMode();
	if (!GM) return;
	EnsureWidgets();
	EnsurePauseBound();
	if (GM->GetCartridge() != LastCartridge.Get())
	{
		LastCartridge = GM->GetCartridge();
		ApplySettingsForMatch();
	}
	if (Overlay)
	{
		if (!bOpeningCardShown && GM->GetSim())
		{
			Overlay->ShowTake(GM->GetTake(), Frame.SimFrame + TakeCardFrames);
			bOpeningCardShown = true;
		}
		UJamDesignerSubsystem* D = GetDesigner();
		Overlay->TickFrame(Frame, D && D->IsDirectorsCallAvailable() && Frame.Phase == EJamPhase::Play);
	}
}

void UJamDirectorsCallController::OnJamEvent(FName Event, const FJamEventPayload& Payload)
{
	if (!bActive || !Overlay) return;
	if (Event == JamEvents::Period)
	{
		// Kind = "take_<n>".
		FString K = Payload.Kind.ToString();
		int32 Take = 1;
		if (K.RemoveFromStart(TEXT("take_"))) Take = FCString::Atoi(*K);
		if (AHokejGameMode* GM = GetGameMode()) Take = FMath::Max(Take, GM->GetTake());
		Overlay->ShowTake(Take, CurrentSimFrame() + TakeCardFrames);
		bOpeningCardShown = true;
		if (Menu && Menu->IsOpen()) Menu->ShowPause();
	}
	else if (Event == JamEvents::Announce)
	{
		// Lane A15 (C-0005): a rule spoke; the text rides the rule system's side channel (UJamRuleSystem::GetLastAnnouncement).
		AHokejGameMode* GM = GetGameMode();
		UJamRuleSystem* Rules = GM ? GM->GetRules() : nullptr;
		if (Rules && !Rules->GetLastAnnouncement().IsEmpty())
		{
			Overlay->ShowAnnouncement(UJamDirectorsCallWidget::RuleLine(Rules->GetLastAnnouncement()), CurrentSimFrame() + AnnouncementFrames);
			UE_LOG(LogJamDirectorsCall, Log, TEXT("JAMRULE announced \"%s\""), *Rules->GetLastAnnouncement());
		}
	}
}

void UJamDirectorsCallController::OnDirectorsCall(const FJamDirectorsCall& Call)
{
	if (!bActive || !Overlay || !Call.bApplied) return;
	Overlay->ShowAnnouncement(UJamDirectorsCallWidget::AnnouncementLine(Call.Announcement), CurrentSimFrame() + AnnouncementFrames);
	UE_LOG(LogJamDirectorsCall, Log, TEXT("JAMCALL announced \"%s\""), *Call.Announcement);
}

void UJamDirectorsCallController::HandlePausePressed()
{
	if (IsMenuOpen()) Resume();
	else Pause();
}

bool UJamDirectorsCallController::IsMenuOpen() const
{
	return Menu && Menu->IsOpen();
}

void UJamDirectorsCallController::Pause()
{
	if (!bActive) return;
	EnsureWidgets();
	if (!Menu) return;
	// Lane A204 (track N1, FAULT 4): a joined match refuses the world pause (ALobbyGameMode::AllowPausing): the card opens over a match that goes on, says so
	// in one line, and this seat sends neutral input while it is open. Single player pauses as before.
	const bool bWorldPaused = UGameplayStatics::SetGamePaused(GetWorld(), true);
	AHokejGameMode* GM = GetGameMode();
	const bool bGoesOn = !bWorldPaused && GM && !GM->AllowPausing(GetPC());
	Menu->Open();
	if (bGoesOn && Overlay) Overlay->ShowAnnouncement(TEXT("The match goes on"), CurrentSimFrame() + AnnouncementFrames);
	if (APlayerController* PC = GetPC())
	{
		PC->bShowMouseCursor = true;
		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(Menu->TakeWidget());
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(Mode);
	}
	UE_LOG(LogJamDirectorsCall, Log, TEXT("JAMCALL paused at frame %lld world_paused=%d%s"), CurrentSimFrame(), bWorldPaused ? 1 : 0, bGoesOn ? TEXT(" (online: the match goes on)") : TEXT(""));
}

void UJamDirectorsCallController::Resume()
{
	if (Menu) Menu->Close();
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	if (APlayerController* PC = GetPC())
	{
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}
}

void UJamDirectorsCallController::RestartMatch()
{
	Resume();
	if (AHokejGameMode* GM = GetGameMode()) GM->RestartMatch();
}

void UJamDirectorsCallController::QuitToMenu()
{
	if (Menu) Menu->Close();
	if (APlayerController* PC = GetPC()) PC->SetInputMode(FInputModeGameOnly());
	UJamMenuFlow::ReturnToMenu(GetWorld());
}
