// Settings (lane A8): the API key (through the Designer's SetApiKey, never in content, never logged), AI difficulty, a look-tier
// override and the volume, stored in Saved/Config GameUserSettings.ini under [Jam.Settings]. The widget is C++-drawn UMG hosted by
// the main menu's Settings screen.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsWidget.generated.h"

class UTextBlock;
class UButton;
class USlider;
class UEditableTextBox;
class UVerticalBox;

// Seam for lane A3: the three AI difficulties of game.ai.difficulty. The strings "rookie" | "pro" | "all_star" are the contract
// (briefs/A3-ai-difficulty.md); when A3's enum lands this one maps onto it through DifficultyString().
UENUM(BlueprintType)
enum class EJamMenuDifficulty : uint8
{
	Rookie,
	Pro,
	AllStar
};

USTRUCT(BlueprintType)
struct JAMGAME_API FJamSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Jam|Settings") EJamMenuDifficulty Difficulty = EJamMenuDifficulty::Pro;
	// Lane A144 (evolution 2): true once the player has picked a level in the settings. Until then PLAY NOW plays rookie (a first run is a
	// newcomer's): EffectiveDifficulty() is what PLAY NOW plays and what the settings button shows.
	UPROPERTY(BlueprintReadWrite, Category = "Jam|Settings") bool bDifficultyChosen = false;
	EJamMenuDifficulty EffectiveDifficulty() const { return bDifficultyChosen ? Difficulty : EJamMenuDifficulty::Rookie; }
	// The saved DifficultyChosen key says so; so does a saved level other than the default (Save() wrote "pro" on every volume change before
	// this key existed, so a saved "pro" alone is no pick, while a saved rookie or all-star could only come from the button).
	static bool IsDifficultyChosen(bool bChosenKey, bool bHasSavedLevel, EJamMenuDifficulty Saved) { return bChosenKey || (bHasSavedLevel && Saved != EJamMenuDifficulty::Pro); }
	// Empty = the cut's own look tier; else "l1_day" | "l2_night" | "l3_neon" (ULookTierSwitcher::GetKnownTiers).
	UPROPERTY(BlueprintReadWrite, Category = "Jam|Settings") FString LookTierOverride;
	// 0..1, applied through FApp::SetVolumeMultiplier.
	UPROPERTY(BlueprintReadWrite, Category = "Jam|Settings") float Volume = 1.f;
	// Barks (lane A12): -1 = the cut's own game.barks.enabled, 0 = off, 1 = on; applied through the jam.Barks console variable.
	UPROPERTY(BlueprintReadWrite, Category = "Jam|Settings") int32 Barks = -1;
	// Bark intensity override: "" = the cut's own, else "quiet" | "normal" | "loud". Pushed to a jam.BarkIntensity console variable
	// when lane A12 provides one; until then stored only (the report names the patch).
	UPROPERTY(BlueprintReadWrite, Category = "Jam|Settings") FString BarkIntensity;

	static const TCHAR* Section;   // "Jam.Settings"

	static FJamSettings Load();
	// Tests: Load() returns this instead of the saved file while it is set (null clears it). Nothing is written.
	static void SetOverrideForTest(const FJamSettings* Settings);
	void Save() const;
	// Applies what applies anywhere (the volume). The look tier is applied per match by UJamDirectorsCallController.
	void Apply() const;

	static FString DifficultyString(EJamMenuDifficulty D);
	static bool ParseDifficulty(const FString& S, EJamMenuDifficulty& Out);
	static FString DifficultyLabel(EJamMenuDifficulty D);   // "ROOKIE" | "PRO" | "ALL-STAR"
	static FString TierLabel(const FString& Tier);          // "" -> "CUT'S OWN", "l2_night" -> "NIGHT (L2)"
	static FString BarksLabel(int32 Barks, const FString& Intensity);  // "OFF" | "CUT'S OWN" | "ON, QUIET" ...
};

UCLASS()
class JAMGAME_API UJamSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Re-reads the settings and the key state (the menu calls it when the screen opens).
	void Refresh();
	UWidget* GetFirstFocusable() const;
	// Lane A144 (the pad walk): everything on the panel a pad can stand on, in reading order.
	TArray<UWidget*> GetFocusables() const;

	UFUNCTION(BlueprintPure, Category = "Jam|Settings") FJamSettings GetSettings() const { return Settings; }

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void BuildTree();
	void RefreshLabels();

	UFUNCTION() void OnKeyCommitted(const FText& Text, ETextCommit::Type Commit);
	UFUNCTION() void OnClearKey();
	UFUNCTION() void OnDifficultyClicked();
	UFUNCTION() void OnTierClicked();
	UFUNCTION() void OnVolumeChanged(float V);
	UFUNCTION() void OnVoiceClicked();
	UFUNCTION() void OnBarksClicked();
	FString VoiceStatus() const;

	FJamSettings Settings;

	UPROPERTY(Transient) TObjectPtr<UVerticalBox> Root;
	UPROPERTY(Transient) TObjectPtr<UEditableTextBox> KeyBox;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> KeyState;
	UPROPERTY(Transient) TObjectPtr<UButton> ClearKeyButton;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> ClearKeyText;
	UPROPERTY(Transient) TObjectPtr<UButton> DifficultyButton;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> DifficultyText;
	UPROPERTY(Transient) TObjectPtr<UButton> TierButton;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TierText;
	UPROPERTY(Transient) TObjectPtr<USlider> VolumeSlider;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> VolumeText;
	UPROPERTY(Transient) TObjectPtr<UButton> VoiceButton;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> VoiceText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> VoiceStatusText;
	UPROPERTY(Transient) TObjectPtr<UButton> BarksButton;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> BarksText;
	int32 VoiceStatusTick = 0;
};
