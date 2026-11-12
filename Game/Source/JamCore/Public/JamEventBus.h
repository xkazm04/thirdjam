// Gameplay events for VFX and audio hooks. Contract: contracts/events.md.
// Producer: gameplay (device A) calls Broadcast. Consumers: device B binds OnJamEvent and spawns NS_<event> / plays MS_<event>.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "JamEventBus.generated.h"

JAMCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogJamEvents, Log, All);

// Event names (contracts/events.md). Any other name is an error: Broadcast logs it and does not deliver it.
namespace JamEvents
{
	JAMCORE_API extern const FName Shot;         // "shot"
	JAMCORE_API extern const FName Board;        // "board"
	JAMCORE_API extern const FName Goal;         // "goal"
	JAMCORE_API extern const FName Faceoff;      // "faceoff"
	JAMCORE_API extern const FName Check;        // "check"
	JAMCORE_API extern const FName Knockdown;    // "knockdown"
	JAMCORE_API extern const FName PickupSpawn;  // "pickup_spawn"
	JAMCORE_API extern const FName PickupTake;   // "pickup_take"
	JAMCORE_API extern const FName Win;          // "win"
	JAMCORE_API extern const FName Lose;         // "lose"
	JAMCORE_API extern const FName DirectorsCall; // "directors_call" (R-0001 7, R-0003 3): Who -1, Where zero, Strength = new value's position in the tunable range 0..1, Kind = tunable key
	JAMCORE_API extern const FName Period;       // "period" (R-0003 3): a TAKE starts; Who -1, Where centre ice, Strength 0, Kind = "take_<n>"
	JAMCORE_API extern const FName Bark;         // "bark" (R-0007, lane A12): Who = speaker skater index, Where = speaker, Strength 0, Kind = situation (goal_for, steal, ...)
	JAMCORE_API extern const FName Signal;       // "signal" (R-0007, lane A12): Who = the human's skater, Where = human, Strength = frames/600, Kind = defend | space | pass | praise
	// C-0005 section 6 (lane A16 registered; producers: A15 rules, A14 effects and zones, A16 passes).
	JAMCORE_API extern const FName Announce;     // "announce": a rule's text on the scoreboard line; Who -1, Where zero, Strength 0, Kind = "rule"; the text rides FJamSimEvent::Text
	JAMCORE_API extern const FName Pass;         // "pass": Who = passer, Where = puck, Strength = pass speed / puck.max_speed
	JAMCORE_API extern const FName EffectStart;  // "effect_start": Who = target skater or -1, Where = that skater (or zero), Kind = effect id
	JAMCORE_API extern const FName EffectEnd;    // "effect_end": same payload as effect_start
	JAMCORE_API extern const FName ZoneEnter;    // "zone_enter": Who = skater, Where = skater, Kind = zone id
	JAMCORE_API extern const FName ZoneExit;     // "zone_exit": same payload as zone_enter
	// R-0013 (lane A16): the stick. Who = the actor; Kind = clean | miss for poke and lift, foul_from_behind | foul_no_puck for foul.
	JAMCORE_API extern const FName Poke;         // "poke": a poke check; Where = blade, Strength 0
	JAMCORE_API extern const FName Lift;         // "lift": a stick lift; Where = the carrier, Strength 0
	JAMCORE_API extern const FName Foul;         // "foul": a check that was a foul; Who = the fouler, Where = contact, Strength = check strength
	JAMCORE_API extern const FName ShotBlocked;  // "shot_blocked" (R-0019): a shot refused inside a no_shoot zone; Who = shooter, Where = puck, Strength 0, Kind = zone id
	// R-0013: `faceoff` gains Kind = drop (the puck is live, Who -1) | won (Who = the centre that won) | lost (Who = the centre that lost).
}

USTRUCT(BlueprintType)
struct JAMCORE_API FJamEventPayload
{
	GENERATED_BODY()

	// Skater index (0..N-1 in game.teams order, team 0's skaters first) or -1 when no skater is involved.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Events")
	int32 Who = -1;

	// World position in Unreal units (centimetres) where the event happened.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Events")
	FVector Where = FVector::ZeroVector;

	// Event-specific magnitude, 0..1 (contracts/events.md says what it means per event).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Events")
	float Strength = 0.f;

	// R-0001 item 3: pickup kind for pickup_spawn / pickup_take, cause ("check" | "boost") for knockdown; NAME_None otherwise.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Events")
	FName Kind = NAME_None;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FJamEventSignature, FName, Event, const FJamEventPayload&, Payload);

UCLASS()
class JAMCORE_API UJamEventBus : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Delivers a known event to every listener and logs it as
	// "JAMEVENT <name> who=<int> where=(x,y,z) strength=<float> frame=<uint64> kind=<name>". Returns false (and logs an error) for an unknown name.
	UFUNCTION(BlueprintCallable, Category = "Jam|Events")
	bool Broadcast(FName Event, const FJamEventPayload& Payload);

	// True when Event is one of JamEvents.
	UFUNCTION(BlueprintPure, Category = "Jam|Events")
	static bool IsKnownEvent(FName Event);

	// The known event names, in contract order (the ten of C-0003, then directors_call and period).
	UFUNCTION(BlueprintPure, Category = "Jam|Events")
	static TArray<FName> GetKnownEvents();

	// Number of events delivered since this game instance started (tests read it).
	UFUNCTION(BlueprintPure, Category = "Jam|Events")
	int32 GetBroadcastCount() const { return BroadcastCount; }

	UPROPERTY(BlueprintAssignable, Category = "Jam|Events")
	FJamEventSignature OnJamEvent;

	// Logging every event is on by default; long bot runs may turn it off.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Events")
	bool bLogEvents = true;

	// The simulation frame the game mode is delivering events for; the log line prints it as frame= (contracts/events.md open
	// question 3). -1 (default, no simulation running) prints the engine frame counter instead.
	UFUNCTION(BlueprintCallable, Category = "Jam|Events")
	void SetSimFrame(int64 InFrame) { SimFrame = InFrame; }

	UFUNCTION(BlueprintPure, Category = "Jam|Events")
	int64 GetSimFrame() const { return SimFrame; }

private:
	int32 BroadcastCount = 0;
	int64 SimFrame = -1;
};
