// Lane A9: the match side of "Play with a friend". Rink.umap is opened with ?game=/Script/JamNet.LobbyGameMode by
// UJamLobbySubsystem; this subclass builds the match exactly as AHokejGameMode does (Super::StartPlay), then takes over the
// stepping: each engine tick it pumps the lockstep driver and advances the sim only for frames whose remote input is present.
// The host drives the cartridge's human, the guest drives team 1's first non-back skater (UJamLobbySubsystem::GetGuestIndex).
// With no lobby session active it behaves exactly like AHokejGameMode (single player).
//
// AHokejGameMode keeps StepOnce / GatherInputs / DeliverEvents / PushPresentation / ReadHumanInput private, so this class
// re-implements the step loop with the public API (seam request for lane A1 in the A9 report: make them protected virtual).
#pragma once

#include "CoreMinimal.h"
#include "HokejGameMode.h"
#include "LobbyGameMode.generated.h"

class UJamLobbySubsystem;
class UJamLockstepDriver;
class UJamBrain;
class AJamSkater;
class AJamPuck;
class ULobbyCardWidget;

UCLASS()
class JAMNET_API ALobbyGameMode : public AHokejGameMode
{
	GENERATED_BODY()

public:
	ALobbyGameMode();
	virtual void StartPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	UFUNCTION(BlueprintPure, Category = "Jam|Lobby") bool IsLockstep() const { return bLockstep; }
	UFUNCTION(BlueprintPure, Category = "Jam|Lobby") int32 GetLocalIndex() const { return LocalIndex; }

private:
	bool StepLockstep();
	FJamInput ReadLocalInput() const;
	FJamInput ScriptedInput(int32 Index) const;
	void DeliverSimEvents();
	void PushLockstepPresentation();
	void PossessLocal(APlayerController* PC);
	void EndMatch(const FString& Title, const FString& Subtitle, const FString& Reason);
	AJamSkater* SkaterActor(int32 SimIndex) const;

	UPROPERTY(Transient) TObjectPtr<UJamLobbySubsystem> Lobby;
	UPROPERTY(Transient) TObjectPtr<UJamLockstepDriver> Driver;
	UPROPERTY(Transient) TObjectPtr<UJamBrain> LockBrain;
	UPROPERTY(Transient) TArray<TObjectPtr<AJamSkater>> SkaterActors;
	UPROPERTY(Transient) TObjectPtr<AJamPuck> PuckActor;
	UPROPERTY(Transient) TObjectPtr<ULobbyCardWidget> Card;

	TArray<FJamInput> LockInputs;
	int32 LocalIndex = -1;
	int32 RemoteIndex = -1;
	double LockAccumulator = 0.0;
	bool bLockstep = false;
	bool bEnded = false;
	bool bShotTaken = false;
};
