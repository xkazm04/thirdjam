// Binds gameplay events to Niagara and MetaSound assets by naming convention. Lane A5 (RULINGS R-0001 item 4); lane A21 (B-to-A-0012, 0016).
// Listens to UJamEventBus::OnJamEvent and, for each known event, spawns /JamAssets/VFX/NS_<event> and plays /JamAssets/Audio/MS_<event>.
// A missing asset is logged once ("JAMFX missing ...") and skipped. Cosmetic only: nothing here is read by the simulation.
// Parameters set on every spawn (B's names): Niagara User.Strength, User.Colour, User.Variant; MetaSound float Strength, Variant.
// Variant (B-to-A-0012): poke / lift clean 0, miss 1; faceoff drop 0, won 1, lost 2; foul foul_from_behind 0, foul_no_puck 1;
// signal defend 0, space 1, pass 2, praise 3 (EJamTeamIntent order); every other event 0. bark, signal, announce are 2D sounds.
// zone_enter / zone_exit carry the zone id in Kind; the binder looks the kind up in the game mode's sim config and sends the kind's
// colour as User.Colour; pickup_* and effect_* send the pickup kind's colour. shot_blocked reuses NS_foul / MS_foul at Variant 0
// until B makes its own (B-to-A-0016); period is silent on purpose.
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

	// B-to-A-0012: the Kind -> Variant table (Niagara User.Variant, MetaSound Variant). 0 for every event or kind not in the table.
	UFUNCTION(BlueprintPure, Category = "Jam|Fx")
	static float VariantFor(FName Event, FName Kind);

	// B-to-A-0012 point 1: bark, signal and announce play as 2D sounds, not at Where.
	UFUNCTION(BlueprintPure, Category = "Jam|Fx")
	static bool IsSound2D(FName Event);

	// B-to-A-0012 point 2: the zone kind's colour (slow_ice, fast_ice, bumper, no_shoot, boost_only); white for any other kind.
	UFUNCTION(BlueprintPure, Category = "Jam|Fx")
	static FLinearColor ZoneKindColour(FName Kind);

	// B-to-A-0016: the pickup kind's colour (speed, precision, heavy_puck, big_puck, reverse_other); white for custom or unknown.
	UFUNCTION(BlueprintPure, Category = "Jam|Fx")
	static FLinearColor PickupKindColour(FName Kind);

	// Events that never spawn or play anything (period: B-to-A-0012 asked; answer: silent).
	UFUNCTION(BlueprintPure, Category = "Jam|Fx")
	static bool IsSilentEvent(FName Event);

	// Events that have a sound and no burst by design (lose, bark, signal, announce): the Niagara lookup is skipped, so no "missing NS_" line.
	UFUNCTION(BlueprintPure, Category = "Jam|Fx")
	static bool IsSoundOnlyEvent(FName Event);

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

	// The colour an event's burst takes: zone kind, pickup kind, or the team colour of Who (white when none applies).
	FLinearColor ColourFor(FName Event, const FJamEventPayload& Payload) const;
	// zone id -> kind through the owning game mode's sim config; NAME_None when unknown.
	FName ZoneKindOf(FName ZoneId) const;

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
