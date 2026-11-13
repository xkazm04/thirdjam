// Binds gameplay events to Niagara and MetaSound assets by naming convention. Lane A5 (RULINGS R-0001 item 4); lane A21 (B-to-A-0012, 0016).
// Listens to UJamEventBus::OnJamEvent and, for each known event, spawns /JamAssets/VFX/NS_<event> and plays /JamAssets/Audio/MS_<event>.
// A missing asset is logged once ("JAMFX missing ...") and skipped. Cosmetic only: nothing here is read by the simulation.
// Parameters set on every spawn (B's names): Niagara User.Strength, User.Colour, User.Variant; MetaSound float Strength, Variant.
// Variant (B-to-A-0012): poke / lift clean 0, miss 1; faceoff drop 0, won 1, lost 2; foul foul_from_behind 0, foul_no_puck 1;
// signal defend 0, space 1, pass 2, praise 3 (EJamTeamIntent order); every other event 0. bark, signal, announce are 2D sounds.
// zone_enter / zone_exit carry the zone id in Kind; the binder looks the kind up in the game mode's sim config and sends the kind's
// colour as User.Colour; pickup_* and effect_* send the pickup kind's colour. shot_blocked reuses NS_foul / MS_foul at Variant 0
// until B makes its own (B-to-A-0016); period and carry are silent on purpose. Lane A47 (B-to-A-0029, 0031): check and knockdown send the sim's
// contact normal as User.Dir, board sends the board normal into the rink as User.Normal; period_end, clock, season_take are sound-only and 2D;
// clock Kind sudden_death is Variant 1; a goal's burst takes the scoring team's colour (the side of the net, not Who).
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/StreamableManager.h"
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
	// R-0033 point 4 (lane A50): MS_goal's Variant: 0 when the human's team scores, 1 when it concedes. Pure parts for the tests: the scoring team is the
	// one whose score grew between Before and After (the payload's Who is the last touch, the wrong team on an own goal); FallbackTeam (Who's team, -1
	// unknown) when neither grew; -1 when nothing tells, which plays Variant 1 (never the celebration for a goal that may not be ours).
	static int32 ResolveScoringTeam(const int32 Before[2], const int32 After[2], int32 FallbackTeam);
	static float GoalVariant(int32 ScoringTeam, int32 HumanTeam) { return ScoringTeam >= 0 && ScoringTeam == HumanTeam ? 0.f : 1.f; }

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

	// Lane A45 (R-0031 point 4, B-to-A-0028 ask 4): the glass hit. On a skater `board` event the rink glass's dynamic instance gets HitWobble = Strength, run down to 0 over
	// GlassWobbleFrames sim frames (the bus's frame, never wall clock), guarded by a parameter-exists check: a glass material without HitWobble (today's M_Glass) is left alone.
	// The rink's glass is one procedural mesh with one material slot, so "the nearest panel" is that mesh; HitPos (vector, if the material has it) carries the contact for a
	// material that localises the sweep. Pure for tests: the value at a frame since the hit.
	static constexpr int32 GlassWobbleFrames = 12;
	static float GlassWobbleAt(int64 FramesSinceHit, float Strength);
	UFUNCTION(BlueprintPure, Category = "Jam|Fx")
	int32 GetGlassWobbleCount() const { return GlassWobbles; }

	UJamEffectsBinder();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
	// Lane A45: the glass hook. Finds the rink's Glass mesh once, makes (or re-makes after a tier swap) its dynamic instance when the material carries HitWobble.
	void StartGlassWobble(const FJamEventPayload& Payload);
	class UMaterialInstanceDynamic* GlassMid();
	TWeakObjectPtr<class UMeshComponent> GlassMesh;
	UPROPERTY(Transient) TObjectPtr<class UMaterialInstanceDynamic> GlassInstance;
	bool bGlassLooked = false;
	int64 GlassHitFrame = -1;
	float GlassHitStrength = 0.f;
	int32 GlassWobbles = 0;
	UFUNCTION()
	void OnJamEvent(FName Event, const FJamEventPayload& Payload);

	// The goal's Variant from the owning game mode's sim (score delta since the last goal this binder saw) and the human's team.
	float GoalVariantFor(const FJamEventPayload& Payload);
	int32 PrevScore[2] = { 0, 0 };

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

// Lane A65 (PROFILING.md offenders 3 and 4): the match start loaded ~87 packages synchronously in the menu -> match frame (83 ms: every Niagara system and
// MetaSound of the binder, the figure class, the rink's look instances, the puck, the look collection). When the menu map is up this subsystem asks for
// all of them in one asynchronous request and holds the handle, so the loads at match start find the objects in memory. Nothing in the sim reads it.
// Missing packages are found once here (FPackageName::DoesPackageExist) so the binder's quiet loads skip their synchronous lookup.
// Diagnostics (not Shipping): every synchronous package load on the game thread from the menu on is logged as "JAMPRELOAD sync <package>", and the
// state of the request when the match map starts loading as "JAMPRELOAD match start ...".
UCLASS()
class JAMGAME_API UJamPreloadSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Every asset a match of this figure and tier loads at its start (existing or not; StartPreload filters).
	static TArray<FSoftObjectPath> MatchPaths(FName Visual, const FString& Tier);
	// The cut the next match will most likely play: -cut= on the command line, else jam.Cut (default hockey.regular).
	static FString LikelyNextCut();

	// Starts (once per cut) the asynchronous load of MatchPaths for the cut's figure and tier and holds it for the life of the game instance.
	void StartPreload(const FString& CutNameOrPath);

	// True when the preload looked the path's package up and it does not exist.
	static bool IsKnownMissing(const FSoftObjectPath& Path);

	int32 GetRequestedCount() const { return Requested; }
	bool IsPreloadComplete() const { return Handles.Num() > 0 && CompletedAt > 0.0; }

private:
	void HandlePostLoadMap(UWorld* World);
	void HandlePreLoadMap(const FString& MapName);
	void HandleSyncLoad(const FString& PackageName);

	// Lane A114: the match map's package, loaded asynchronously while the menu is up and held until the match map has loaded (UEngine::LoadMap finds it in
	// memory instead of loading it synchronously in the menu -> match frame). jam.PreloadMap=0: the map loads as before.
	void StartMapPreload();
	UPROPERTY(Transient)
	TObjectPtr<UPackage> MapPackage;
	// The world inside it: a package does not keep its objects alive, so LoadMap's garbage collection would take the world without this reference.
	UPROPERTY(Transient)
	TObjectPtr<UWorld> MapWorld;
	bool bMapRequested = false;
	int32 MapRequestId = INDEX_NONE;
	double MapRequestedAt = 0.0;
	double MapLoadedAt = 0.0;

	TArray<TSharedPtr<FStreamableHandle>> Handles;
	TSet<FString> PreloadedCuts;
	FDelegateHandle PostLoadMapHandle, PreLoadMapHandle, SyncLoadHandle;
	double RequestedAt = 0.0;
	double CompletedAt = 0.0;
	int32 Requested = 0;
	int32 SyncLoadsLogged = 0;
};
