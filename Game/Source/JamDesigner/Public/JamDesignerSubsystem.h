// The Designer (docs/PLAN.md section 4, lane A7): D0 cuts (shipped presets + saved "My Director's Cut"), D1 live sentence -> cut
// through the Messages API (key typed by the player or ANTHROPIC_API_KEY; never bundled, never logged), the offline path
// (no key: the nearest cached cut by keyword), and the Director's Call (one tunable change per period).
// Every cartridge the Designer produces or loads goes through lane A1's UJamCartridge loader before it is used.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "JamDesignerSubsystem.generated.h"

class UJamCartridge;
class UJamEventBus;
class IJamDesignerTransport;

JAMDESIGNER_API DECLARE_LOG_CATEGORY_EXTERN(LogJamDesigner, Log, All);

UENUM(BlueprintType)
enum class EJamDesignerOutcome : uint8
{
	FirstTry,        // live: the first reply was valid
	Repaired,        // live: a repair attempt was valid
	Refused,         // live: the model (or the platform) declined; Refusal says why
	Failed,          // live: no valid reply after the chain, or the request could not be made; Errors say why
	Offline,         // no key (or the network failed): the nearest cached or saved cut
	OfflineNoMatch,  // no key and no cached sentence shares a word with the request; Message lists what to try
	Busy,            // a request is already running
};

USTRUCT(BlueprintType)
struct JAMDESIGNER_API FJamCutInfo
{
	GENERATED_BODY()
	// Shipped: "hockey.regular"; saved: the name it was saved under ("My Director's Cut").
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Designer") FString Name;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Designer") FString Path;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Designer") bool bSaved = false;
	// The sentence the cut was made from (saved cuts only).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Designer") FString Sentence;
};

USTRUCT(BlueprintType)
struct JAMDESIGNER_API FJamDesignerResult
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Designer") EJamDesignerOutcome Outcome = EJamDesignerOutcome::Failed;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Designer") FString Sentence;
	// The validated cartridge (FirstTry, Repaired, Offline); null otherwise.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Designer") TObjectPtr<UJamCartridge> Cartridge;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Designer") FString CartridgeJson;
	// The cut's display name (theme.name of the result, or the cached cut's name).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Designer") FString CutName;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Designer") FString Refusal;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Designer") TArray<FString> Errors;
	// One readable line for the UI.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Designer") FString Message;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Designer") int32 Attempts = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Designer") int64 TokensIn = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Designer") int64 TokensOut = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Designer") double Ms = 0;

	bool HasCartridge() const { return Cartridge != nullptr; }
};

USTRUCT(BlueprintType)
struct JAMDESIGNER_API FJamDirectorsCall
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Designer") bool bApplied = false;
	// Why it was refused (empty when applied).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Designer") FString Reason;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Designer") FName Key;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Designer") double OldValue = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Designer") double NewValue = 0;
	// What the scoreboard says: "Director's Call: ice is now glass".
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Designer") FString Announcement;
	// The new cartridge with values[Key] set (applied calls only).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Designer") TObjectPtr<UJamCartridge> Cartridge;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FJamDesignerCallback, const FJamDesignerResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FJamDesignerResultSignature, const FJamDesignerResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FJamCutChangedSignature, UJamCartridge*, Cartridge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FJamDirectorsCallSignature, const FJamDirectorsCall&, Call);

UCLASS()
class JAMDESIGNER_API UJamDesignerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// The event name broadcast on a Director's Call (proposed for contracts/events.md; Kind = the tunable key).
	static const FName DirectorsCallEvent;
	static const FString MyDirectorsCutName;

	UJamDesignerSubsystem();

	// ---- D0: cuts ----
	// Shipped presets (Content/Cartridges/*.cartridge.json, test cartridges excluded) then saved cuts (Saved/Cuts/*.cartridge.json).
	UFUNCTION(BlueprintCallable, Category = "Jam|Designer") TArray<FJamCutInfo> ListCuts() const;
	// Loads a shipped or saved cut by name (or a .json path), validates it with UJamCartridge, makes it current, fires
	// OnCutChanged and asks the running game mode to rebuild with it (AHokejGameMode::LoadCut, found by reflection).
	UFUNCTION(BlueprintCallable, Category = "Jam|Designer") bool LoadCut(const FString& Name, FString& OutError);
	// Validates Json and writes Saved/Cuts/<slug>.cartridge.json (+ <slug>.meta.json with the name and sentence). Returns the path or empty.
	UFUNCTION(BlueprintCallable, Category = "Jam|Designer") FString SaveCut(const FString& Name, const FString& Json, const FString& Sentence, FString& OutError);
	// Saves a Designer result as "My Director's Cut" and loads it.
	UFUNCTION(BlueprintCallable, Category = "Jam|Designer") bool ApplyResult(const FJamDesignerResult& Result, FString& OutError);

	UFUNCTION(BlueprintPure, Category = "Jam|Designer") UJamCartridge* GetCurrentCartridge() const { return Current; }
	// True between a LoadCut and the next frame, when the match rebuild and OnCutChanged run (R-0017: never inside a callback).
	UFUNCTION(BlueprintPure, Category = "Jam|Designer") bool IsCutApplyPending() const { return bApplyScheduled; }
	// Makes C current without loading a file (the game mode calls this with the cartridge it booted, so a Director's Call has a base).
	UFUNCTION(BlueprintCallable, Category = "Jam|Designer") void SetCurrentCartridge(UJamCartridge* C) { Current = C; }

	// ---- the Director's Call ----
	// One change per period: Key must be a number/integer tunable of the current cut, Value inside its min..max (integer
	// tunables need a whole number). Applied: values[Key] = Value, re-validated, made current, broadcast as directors_call.
	UFUNCTION(BlueprintCallable, Category = "Jam|Designer") FJamDirectorsCall DirectorsCall(FName Key, double Value);
	// The game mode calls this when a period (TAKE) starts; it gives the Director's Call back.
	UFUNCTION(BlueprintCallable, Category = "Jam|Designer") void NotifyPeriodStarted(int32 Period);
	UFUNCTION(BlueprintPure, Category = "Jam|Designer") bool IsDirectorsCallAvailable() const { return !bCallUsedThisPeriod; }

	// ---- D1: sentence -> cut ----
	// Live with a key (async, never blocks the game thread), offline from the cache without one. CurrentCartridgeJson empty =
	// the current cut. Done runs on the game thread exactly once.
	void RequestCut(const FString& Sentence, const FString& CurrentCartridgeJson, TFunction<void(const FJamDesignerResult&)> Done);
	UFUNCTION(BlueprintCallable, Category = "Jam|Designer", meta = (DisplayName = "Request Cut"))
	void RequestCutBP(const FString& Sentence, const FString& CurrentCartridgeJson, FJamDesignerCallback Done);
	UFUNCTION(BlueprintPure, Category = "Jam|Designer") bool IsRequestRunning() const { return bBusy; }

	// The cached sentences (Content/Designer/cached-cuts.json), for the offline UI.
	UFUNCTION(BlueprintCallable, Category = "Jam|Designer") TArray<FString> GetCachedSentences() const;
	// The cached cuts as name + sentence (Path empty), for the offline UI's list.
	UFUNCTION(BlueprintCallable, Category = "Jam|Designer") TArray<FJamCutInfo> GetCachedCuts() const;

	// ---- the key (lane A5's settings screen calls SetApiKey) ----
	// Stores the key in Saved/Config (GameUserSettings.ini via GConfig), never in content, never in a log. Empty clears it.
	UFUNCTION(BlueprintCallable, Category = "Jam|Designer") void SetApiKey(const FString& Key);
	UFUNCTION(BlueprintPure, Category = "Jam|Designer") bool HasApiKey() const { return !ResolveKey().IsEmpty(); }
	// True with a key (HTTP) or with the CLI transport available (R-0004: development and the operator's demo build).
	UFUNCTION(BlueprintPure, Category = "Jam|Designer") bool IsLiveAvailable() const { return HasApiKey() || !ResolveCliExe().IsEmpty(); }

	UPROPERTY(BlueprintAssignable, Category = "Jam|Designer") FJamDesignerResultSignature OnDesignerResult;
	UPROPERTY(BlueprintAssignable, Category = "Jam|Designer") FJamCutChangedSignature OnCutChanged;
	UPROPERTY(BlueprintAssignable, Category = "Jam|Designer") FJamDirectorsCallSignature OnDirectorsCall;

	// Per-request total timeout in seconds, and the delay before the one retry of a 429/5xx.
	UPROPERTY(EditAnywhere, Category = "Jam|Designer") float TimeoutSec = 60.f;
	// Per-attempt timeout of the CLI transport (process start plus the model call).
	UPROPERTY(EditAnywhere, Category = "Jam|Designer") float CliTimeoutSec = 120.f;
	UPROPERTY(EditAnywhere, Category = "Jam|Designer") float RetryDelaySec = 1.5f;

	// ---- test hooks ----
	void SetTransport(TSharedPtr<IJamDesignerTransport> InTransport);
	// Replaces config + environment as the key source (empty = no key). Nothing is written anywhere.
	void SetKeyOverrideForTest(const FString& Key) { bKeyOverride = true; KeyOverride = Key; }
	// Forces the CLI transport off, or on with Exe (empty Exe = look on PATH, ignoring the Shipping/config gate).
	void SetCliForTest(bool bEnable, const FString& Exe = FString()) { bCliOverride = true; bCliOverrideEnabled = bEnable; CliExeOverride = Exe; }
	// The claude executable the CLI transport would use; empty when the CLI route is off.
	FString ResolveCliExe() const;
	void SetEventBusForTest(UJamEventBus* Bus) { BusOverride = Bus; }
	void SetSavedCutsDirForTest(const FString& Dir) { SavedCutsDirOverride = Dir; }
	FString GetSavedCutsDir() const;

	// Offline resolution (exposed for tests and the console).
	FJamDesignerResult ResolveOffline(const FString& Sentence) const;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	FString ResolveKey() const;
	FString CurrentJsonOrDefault() const;
	UJamEventBus* GetBus() const;
	bool CallGameMode(const TCHAR* Function, const FString& StringArg, UObject* ObjectArg) const;
	void Finish(const FJamDesignerResult& Result, const TFunction<void(const FJamDesignerResult&)>& Done);
	void ScheduleCutApply(const FString& Path);

	friend struct FJamDesignerJob;

	UPROPERTY(Transient) TObjectPtr<UJamCartridge> Current;
	UPROPERTY(Transient) TObjectPtr<UJamEventBus> BusOverride;
	TSharedPtr<IJamDesignerTransport> Transport;
	FString SessionKey;
	FString KeyOverride;
	FString SavedCutsDirOverride;
	bool bKeyOverride = false;
	bool bCliOverride = false;
	bool bCliOverrideEnabled = false;
	FString CliExeOverride;
	bool bCallUsedThisPeriod = false;
	bool bBusy = false;
	bool bApplyScheduled = false;
	FString PendingApplyPath;
};
