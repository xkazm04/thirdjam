// Binds gameplay events to Niagara and MetaSound assets by naming convention. Lane A5 (RULINGS R-0001 item 4).
// Listens to UJamEventBus::OnJamEvent and, for each known event, spawns /JamAssets/VFX/NS_<event> and plays /JamAssets/Audio/MS_<event>.
// A missing asset is logged once ("JAMFX missing ...") and skipped. Cosmetic only: nothing here is read by the simulation.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JamEventBus.h"
#include "EffectsBinder.generated.h"

class UNiagaraSystem;
class USoundBase;

JAMGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogJamFx, Log, All);

UCLASS(ClassGroup = (Jam), meta = (BlueprintSpawnableComponent))
class JAMGAME_API UJamEffectsBinder : public UActorComponent
{
	GENERATED_BODY()

public:
	// Soft object paths by convention (the suffix is the event name exactly).
	static FSoftObjectPath NiagaraPathFor(FName Event);
	static FSoftObjectPath SoundPathFor(FName Event);

	// Loads (sync, quiet) the asset at Path, or logs "JAMFX missing <path>" ONCE per path and returns null.
	// Shared with other cosmetic code (the toy figure's blade spray) so one missing asset is one log line.
	static UObject* LoadOrLogOnce(const FSoftObjectPath& Path);

	// Number of distinct missing assets logged so far in this process (tests read it).
	static int32 GetMissingLoggedCount();
	static void ResetMissingLogForTests();

	UFUNCTION(BlueprintCallable, Category = "Jam|Fx")
	void HandleEvent(FName Event, const FJamEventPayload& Payload);

	// Counts of effects actually spawned or played since BeginPlay (PIE proof reads them).
	UFUNCTION(BlueprintPure, Category = "Jam|Fx")
	int32 GetSpawnedNiagaraCount() const { return SpawnedNiagara; }

	UFUNCTION(BlueprintPure, Category = "Jam|Fx")
	int32 GetPlayedSoundCount() const { return PlayedSounds; }

	UFUNCTION(BlueprintPure, Category = "Jam|Fx")
	int32 GetHandledEventCount() const { return HandledEvents; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
	UFUNCTION()
	void OnJamEvent(FName Event, const FJamEventPayload& Payload);

	// Assets resolved at BeginPlay (null = missing, already logged) so no event hitches on a load.
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UNiagaraSystem>> Niagara;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<USoundBase>> Sounds;

	UPROPERTY(Transient)
	TObjectPtr<UJamEventBus> Bus;

	int32 SpawnedNiagara = 0;
	int32 PlayedSounds = 0;
	int32 HandledEvents = 0;
};
