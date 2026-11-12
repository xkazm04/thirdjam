// In-match framing and the Director's Call (lane A8): a world subsystem that lives only in rink worlds (AHokejGameMode).
// It binds the pause key (Esc / Start, lane A5's UJamPlayerInput::OnPause), pauses the game and shows the pause menu with the
// Director's Call card (UJamDirectorsCallWidget), and draws the period cards "TAKE n" and the scoreboard line
// "DIRECTOR'S CALL: ..." in its own overlay (UJamDirectorsCallHudWidget) from the event bus ("period", "directors_call").
// Time is simulation frames: cards count UJamPresentation::OnSimFrame, never wall clock.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "JamEventBus.h"
#include "JamDesignerSubsystem.h"
#include "JamCartridge.h"
#include "InputMapping.h"
#include "HUDFeed.h"
#include "DirectorsCallController.generated.h"

class AHokejGameMode;
class UJamDirectorsCallWidget;
class UJamDirectorsCallHudWidget;
class APlayerController;

JAMGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogJamDirectorsCall, Log, All);

UCLASS()
class JAMGAME_API UJamDirectorsCallController : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Frames a TAKE card stays up (2.5 s) and a Director's Call line stays on the scoreboard (7 s).
	static constexpr int64 TakeCardFrames = 150;
	static constexpr int64 AnnouncementFrames = 420;

	static UJamDirectorsCallController* Get(const UObject* WorldContext);

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	// Pause menu.
	UFUNCTION(BlueprintCallable, Category = "Jam|DirectorsCall") void Pause();
	UFUNCTION(BlueprintCallable, Category = "Jam|DirectorsCall") void Resume();
	UFUNCTION(BlueprintCallable, Category = "Jam|DirectorsCall") void RestartMatch();
	UFUNCTION(BlueprintCallable, Category = "Jam|DirectorsCall") void QuitToMenu();
	UFUNCTION(BlueprintPure, Category = "Jam|DirectorsCall") bool IsMenuOpen() const;

	UFUNCTION(BlueprintPure, Category = "Jam|DirectorsCall") AHokejGameMode* GetGameMode() const;
	UFUNCTION(BlueprintPure, Category = "Jam|DirectorsCall") UJamDesignerSubsystem* GetDesigner() const;
	UFUNCTION(BlueprintPure, Category = "Jam|DirectorsCall") UJamDirectorsCallWidget* GetMenu() const { return Menu; }
	UFUNCTION(BlueprintPure, Category = "Jam|DirectorsCall") UJamDirectorsCallHudWidget* GetOverlay() const { return Overlay; }

	// True once the rink world has a game mode this controller drives (false in the menu world).
	bool IsActive() const { return bActive; }

	// The pause key handler (bound to UJamPlayerInput::OnPause of the human's skater).
	UFUNCTION() void HandlePausePressed();

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	UFUNCTION() void OnJamEvent(FName Event, const FJamEventPayload& Payload);
	UFUNCTION() void OnDirectorsCall(const FJamDirectorsCall& Call);
	void OnSimFrame(const FJamPresentationFrame& Frame);
	void EnsureWidgets();
	void EnsurePauseBound();
	void ApplySettingsForMatch();
	APlayerController* GetPC() const;
	int64 CurrentSimFrame() const;

	UPROPERTY(Transient) TObjectPtr<UJamDirectorsCallWidget> Menu;
	UPROPERTY(Transient) TObjectPtr<UJamDirectorsCallHudWidget> Overlay;
	TWeakObjectPtr<UJamPlayerInput> BoundInput;
	TWeakObjectPtr<UJamCartridge> LastCartridge;
	FDelegateHandle FrameHandle;
	bool bActive = false;
	bool bBusBound = false;
	bool bOpeningCardShown = false;
};
