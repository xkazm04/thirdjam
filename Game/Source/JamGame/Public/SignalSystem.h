// Lane A12 (ruling R-0007 point 3), Phase 2: shouted team signals. Keys 1-4 and the gamepad d-pad issue an FJamTeamIntent (defend, space,
// pass, praise); the system keeps the active intent for the brain (lane A3 reads it through the game mode's world view), fires OnIntent, broadcasts
// "signal" on the bus once the contract knows the name, and has a teammate answer with a bark ("got it", "on him"). Phase 2b adds Windows speech
// recognition (SAPI) as a second source that calls the same Issue(). Frames, never seconds.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SignalIntent.h"
#include "SignalVoice.h"
#include "SignalSystem.generated.h"

class UInputAction;
class UJamSim;
class UInputMappingContext;
class APlayerController;
struct FJamPresentationFrame;

JAMGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogJamSignals, Log, All);

namespace JamSignals
{
	JAMGAME_API extern const FName BusEvent;   // "signal": Who = the human, Kind = intent name, Strength = frames / 600 (proposed for contracts/events.md)
	constexpr int32 RepeatCooldownFrames = 60;  // a second signal within a second is ignored (no spam, no double acks)
}

// The four actions and their mapping context (keys 1-4, d-pad), created once per process like lane A5's UJamInputMap.
UCLASS()
class JAMGAME_API UJamSignalInputMap : public UObject
{
	GENERATED_BODY()

public:
	static UJamSignalInputMap* Get();

	UPROPERTY() TArray<TObjectPtr<UInputAction>> Actions;   // slot 0..3 = defend, space, pass, praise
	UPROPERTY() TObjectPtr<UInputMappingContext> Context;

	bool IsKeyMapped(int32 Slot, const FKey& Key) const;

private:
	void Build();
};

DECLARE_MULTICAST_DELEGATE_OneParam(FJamIntentDelegate, const FJamTeamIntent&);

UCLASS()
class JAMGAME_API UJamSignalSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UJamSignalSystem* Get(const UObject* WorldContext);

	// Slot 1..4 (the key number) -> intent; anything else None.
	static EJamTeamIntent IntentForSlot(int32 Slot);
	// The bark situation a teammate answers with: signal_ack_defend / _space / _pass, praise_reply for praise, NAME_None for None.
	static FName AckSituationFor(EJamTeamIntent Intent);
	// The intent record for a signal given now (Frames = the default per intent).
	static FJamTeamIntent Make(EJamTeamIntent Intent, int64 Frame, int32 Issuer, int32 Target);
	// Lane A129 (track Q1): the bot teammate nearest Seat that answers its signal (never a human seat), or -1.
	static int32 AnswererFor(const UJamSim& Sim, int32 Seat);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	// Gives the signal: picks the target teammate (praise and pass: the teammate nearest the human), records it, fires OnIntent, broadcasts
	// the bus event when known, and has the target answer with a bark. False when no sim, no teammate, or inside the repeat cooldown.
	UFUNCTION(BlueprintCallable, Category = "Jam|Signals")
	bool Issue(EJamTeamIntent Intent);

	// The intent the brain should honour this frame; false when none is active.
	UFUNCTION(BlueprintPure, Category = "Jam|Signals")
	bool GetActiveIntent(FJamTeamIntent& Out) const;

	UFUNCTION(BlueprintPure, Category = "Jam|Signals") int32 GetSignalCount() const { return SignalCount; }
	UFUNCTION(BlueprintPure, Category = "Jam|Signals") const FJamTeamIntent& GetLastIntent() const { return Last; }
	UFUNCTION(BlueprintPure, Category = "Jam|Signals") bool IsInputBound() const { return bBound; }

	// ---- Phase 2b: voice signals (Windows speech recognition, SAPI). For lane A8's settings row "Voice signals". ----
	// Persisted in GameUserSettings.ini ([/Script/JamGame.JamSignalSystem] bVoiceSignals); the console variable jam.VoiceSignals overrides (-1 = setting).
	UFUNCTION(BlueprintCallable, Category = "Jam|Signals") static void SetVoiceSignalsEnabled(bool bEnabled);
	UFUNCTION(BlueprintPure, Category = "Jam|Signals") static bool IsVoiceSignalsEnabled();
	// True while the recognizer listens; GetVoiceStatus is "listening" | "off" | the reason it could not start (no recognizer, no microphone).
	UFUNCTION(BlueprintPure, Category = "Jam|Signals") bool IsVoiceListening() const;
	UFUNCTION(BlueprintPure, Category = "Jam|Signals") FString GetVoiceStatus() const;
	// The last phrase the recognizer heard (for the HUD or the settings row), empty when none.
	UFUNCTION(BlueprintPure, Category = "Jam|Signals") const FString& GetLastHeard() const { return LastHeard; }
	UFUNCTION(BlueprintPure, Category = "Jam|Signals") int32 GetHeardCount() const { return HeardCount; }

	FJamIntentDelegate OnIntent;

private:
	// Test hook: jam.SignalScript "<frame>:<slot>;..." presses key <slot> (1-4) through the real input stack at that sim frame (PIE proofs).
	void RunSignalScript();
	TArray<TPair<int64, int32>> Script;
	bool bScriptParsed = false;
	int32 PendingRelease = -1;
	void HandleSimFrame(const FJamPresentationFrame& Frame);
	void UpdateVoice();
	void TryBind();
	void OnSlot1() { Issue(EJamTeamIntent::Defend); }
	void OnSlot2() { Issue(EJamTeamIntent::Space); }
	void OnSlot3() { Issue(EJamTeamIntent::Pass); }
	void OnSlot4() { Issue(EJamTeamIntent::Praise); }
	int64 CurrentFrame() const;

	FDelegateHandle FrameHandle;
	TWeakObjectPtr<APlayerController> BoundPC;
	TUniquePtr<FJamSignalVoice> Voice;
	FJamTeamIntent Last;
	FString LastHeard;
	int64 Frame = 0;
	int32 SignalCount = 0;
	int32 HeardCount = 0;
	bool bBound = false;
	bool bVoiceWanted = false;
};
