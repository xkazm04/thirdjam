// Lane A11: NVIDIA DLSS Super Resolution / DLAA switch (docs/experiments/DLSS.md).
// Settings UI (lane A8) calls the static functions of UJamDlssSettings; UJamDlssSubsystem applies the saved mode at start.
// Without the DLSS plugin (non-Win64, or a checkout that did not run tools/dlss/fetch-dlss.ps1) everything compiles
// and IsSupported() is false. On non-NVIDIA GPUs the plugin reports unsupported and the engine keeps TSR.
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DlssJam.generated.h"

UENUM(BlueprintType)
enum class EJamDlssMode : uint8
{
	Auto        UMETA(DisplayName = "Auto"),
	Quality     UMETA(DisplayName = "Quality"),
	Balanced    UMETA(DisplayName = "Balanced"),
	Performance UMETA(DisplayName = "Performance"),
	DLAA        UMETA(DisplayName = "DLAA"),
	Off         UMETA(DisplayName = "Off")
};

UCLASS()
class JAMGAME_API UJamDlssSettings : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// True when the DLSS plugin is loaded and NGX reports Super Resolution supported on this GPU and driver.
	UFUNCTION(BlueprintPure, Category = "Jam|DLSS")
	static bool IsSupported();

	// Short reason for the Settings screen when IsSupported() is false ("not an NVIDIA RTX GPU", "driver too old", ...); empty when supported.
	UFUNCTION(BlueprintPure, Category = "Jam|DLSS")
	static FString GetUnsupportedReason();

	// Applies the mode now and saves it in GameUserSettings.ini. Unsupported hardware: any mode applies as Off (TSR); the choice is still saved.
	UFUNCTION(BlueprintCallable, Category = "Jam|DLSS")
	static void SetMode(EJamDlssMode Mode);

	// The saved choice (default Auto).
	UFUNCTION(BlueprintPure, Category = "Jam|DLSS")
	static EJamDlssMode GetMode();

	// What is running now: Off on unsupported hardware, otherwise the saved choice.
	UFUNCTION(BlueprintPure, Category = "Jam|DLSS")
	static EJamDlssMode GetEffectiveMode();

	// The modes to offer: all six when supported, only Off otherwise.
	UFUNCTION(BlueprintPure, Category = "Jam|DLSS")
	static TArray<EJamDlssMode> GetAvailableModes();

	// "auto" | "quality" | "balanced" | "performance" | "dlaa" | "off" (the saved ini value and the console command argument).
	UFUNCTION(BlueprintPure, Category = "Jam|DLSS")
	static FString ModeToString(EJamDlssMode Mode);

	UFUNCTION(BlueprintPure, Category = "Jam|DLSS")
	static bool ModeFromString(const FString& In, EJamDlssMode& OutMode);

	// Re-applies the saved mode (called by UJamDlssSubsystem; call again after a resolution change so Auto re-picks).
	UFUNCTION(BlueprintCallable, Category = "Jam|DLSS")
	static void ApplySaved();

private:
	static void Apply(EJamDlssMode Mode);
};

// Applies the saved DLSS mode once per game instance (after PostEngineInit, which the plugin's library needs).
UCLASS()
class JAMGAME_API UJamDlssSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
};
