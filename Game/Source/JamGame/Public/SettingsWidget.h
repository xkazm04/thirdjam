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
	// Lane A183 (R-0064 point 1): the player's bindings in their saved form (FJamBindings::ToSaved: verb id -> "Keys|Pads"); empty = the defaults.
	// Saved in the settings file's own section [Jam.Controls], in the same save and flush as [Jam.Settings]; ToSaved / FromSaved carry them as
	// "Controls.<verb>" keys so a test holds them across a "restart" like the level.
	UPROPERTY(BlueprintReadWrite, Category = "Jam|Settings") TMap<FString, FString> Controls;
	// Lane A222 (B-to-A-0044): SETTINGS' CONTROLS PANEL, ON by default: the keys beside the play all match; OFF takes the panel (and the strip and the
	// pill it replaced) away for good. Saved as [Jam.Settings] ControlsPanel=True|False; applied to the HUD by Apply() and by the row on its press.
	UPROPERTY(BlueprintReadWrite, Category = "Jam|Settings") bool bControlsPanel = true;

	static const TCHAR* Section;   // "Jam.Settings"
	static const TCHAR* ControlsSection;   // "Jam.Controls" (lane A183)

	static FJamSettings Load();
	// Lane A144 (evolution 3): the saved form, key by key, the one Load() reads and Save() writes (GameUserSettings.ini, [Jam.Settings]). A test
	// holds a profile across a "restart" with these two and no file: FromSaved(ToSaved()).
	static FJamSettings FromSaved(const TMap<FString, FString>& Saved);
	TMap<FString, FString> ToSaved() const;
	// Tests: Load() returns this instead of the saved file while it is set (null clears it). Nothing is written.
	static void SetOverrideForTest(const FJamSettings* Settings);
	void Save() const;
	// Lane A164: false when this process writes no config file (-Multiprocess without -MultiprocessSaveConfig, or -nowrite): a save lives in memory.
	static bool WritesReachDisk();
	// Applies what applies anywhere (the volume). The look tier is applied per match by UJamDirectorsCallController.
	void Apply() const;

	static FString DifficultyString(EJamMenuDifficulty D);
	static bool ParseDifficulty(const FString& S, EJamMenuDifficulty& Out);
	static FString DifficultyLabel(EJamMenuDifficulty D);   // "ROOKIE" | "PRO" | "ALL-STAR"
	static FString TierLabel(const FString& Tier);          // "" -> "EACH RINK'S OWN" (lane A195), "l2_night" -> "NIGHT (L2)"
	static FString BarksLabel(int32 Barks, const FString& Intensity);  // "OFF" | "MODE'S OWN" | "ON, QUIET" ...
};

class UJamControlsWidget;
class UBorder;
enum class EJamVerb : uint8;
struct FJamBindings;

UCLASS()
class JAMGAME_API UJamSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Lane A183: the CONTROLS screen (every verb rebindable), for the front end to show where it shows a screen. Outer: a widget, a player controller,
	// a world or a game instance (what CreateWidget takes). Its OnBack fires on BACK, or Esc / pad B when no row listens.
	static UJamControlsWidget* MakeControlsScreen(UObject* Outer);

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
public:
	// Lane A195 (FAULT 7): where the focus goes when the key box commits (Enter or Esc): never nowhere.
	UWidget* FocusAfterKeyBox() const;
	class UEditableTextBox* GetKeyBox() const { return KeyBox; }
private:
	UFUNCTION() void OnClearKey();
	UFUNCTION() void OnDifficultyClicked();
	UFUNCTION() void OnTierClicked();
	UFUNCTION() void OnVolumeChanged(float V);
	UFUNCTION() void OnVoiceClicked();
	UFUNCTION() void OnBarksClicked();
	UFUNCTION() void OnControlsPanelClicked();   // lane A222
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
	UPROPERTY(Transient) TObjectPtr<UButton> ControlsPanelButton;   // lane A222: CONTROLS PANEL  ON / OFF
	UPROPERTY(Transient) TObjectPtr<UTextBlock> ControlsPanelText;
public:
	UButton* GetControlsPanelButtonForTest() const;
	FString GetControlsPanelLabelForTest() const;
	void PressControlsPanelForTest() { OnControlsPanelClicked(); }
	static FString ControlsPanelLabel(bool bOn) { return bOn ? TEXT("ON") : TEXT("OFF"); }
private:
	int32 VoiceStatusTick = 0;
	// Lane A195: the buttons in the front end's item look (MenuWidget.h, FJamMenuPainter; made in BuildTree).
	TSharedPtr<struct FJamMenuPainter> Painter;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FJamControlsBackSignature);

// Lane A183 (R-0064 point 1): the CONTROLS screen. One row per verb (the two-role buttons as two rows: "SHOOT (WITH THE PUCK)", "CROSS-CHECK
// (WITHOUT)"), each with its keys and its pad buttons. Click a cell (or stand on it with the pad and press A) and it listens: the next key, mouse
// button, wheel or pad button goes on that verb (the keys cell takes keys and mouse, the pad cell pad buttons); Esc cancels. A key another verb held is
// swapped onto it (the screen says which). RESET TO DEFAULTS. Every change is saved with the profile at once ([Jam.Controls]) and applied to the game.
UCLASS()
class JAMGAME_API UJamControlsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Jam|Settings") FJamControlsBackSignature OnBack;

	// Re-reads the bindings into the cells (call when the screen opens).
	void Refresh();
	UWidget* GetFirstFocusable() const;
	TArray<UWidget*> GetFocusables() const;

	// The listening state and the press it takes (the native key and mouse handlers call these; tests call them directly).
	void BeginListen(EJamVerb Verb, bool bPad);
	bool IsListening() const { return ListenVerb >= 0; }
	// True: the key was taken (bound, or Esc cancelled). A key of the wrong kind (a pad button on the keys cell) is refused with a line.
	bool HandleListenedKey(const FKey& Key);
	void ResetToDefaults();
	// Lane A199 (item 4): what RESET TO DEFAULTS does: the first press asks (the button reads CONFIRM RESET for ResetConfirmSeconds), the second resets.
	void RequestReset();
	bool IsResetArmed() const { return bResetArmed; }
	static constexpr float ResetConfirmSeconds = 4.f;
	// Lane A199: a press while listening as the native handlers see it (a key repeat is ignored: a held Enter or A never binds itself).
	bool TakeListenPress(const FKey& Key, bool bRepeat);
	FString GetStatusLine() const { return StatusLine; }
	// Lane A199: public so the tests press through the same handlers Slate calls (a mouse down; a quick second click, which Slate sends as a double-click).
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	// The cell's text as drawn ("LMB, MMB, MOUSE4"), for tests.
	FString CellText(EJamVerb Verb, bool bPad) const;
	// Lane A203 (item 3): while a cell listens, a mouse press anywhere on the screen is the press (a Slate input pre-processor, registered by
	// BeginListen): it never reaches the menu behind the panel. True = registered (false where Slate is not up).
	bool IsCapturingOutside() const { return ListenCapture.IsValid(); }
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void BeginDestroy() override;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnAnalogValueChanged(const FGeometry& InGeometry, const FAnalogInputEvent& InAnalogEvent) override;

private:
	void BuildTree();
	void RefreshCells();
	void SaveAndApply(const FJamBindings& B);
	void OnCellClicked(int32 Cell);
	UFUNCTION() void OnResetClicked();
	UFUNCTION() void OnBackClicked();

	int32 ListenVerb = -1;
	bool bListenPad = false;
	// Lane A203 (item 3): the screen-wide capture while listening, and the mouse button whose release it still swallows (EKeys::Invalid = none).
	TSharedPtr<class IInputProcessor> ListenCapture;
	FKey SwallowUp;
	void StopListenCapture();
	friend class FJamListenCapture;
	FString StatusLine;
	// Frames the screen ignores a press after it began listening (the click that started it is still on its way up).
	int32 ListenGraceTicks = 0;
	TArray<bool> CellWasPressed;
	bool bResetArmed = false;        // lane A199
	float ResetArmedLeft = 0.f;

	UPROPERTY(Transient) TArray<TObjectPtr<UButton>> CellButtons;     // verb * 2 + (pad ? 1 : 0)
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> CellTexts;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> StatusText;
	UPROPERTY(Transient) TObjectPtr<UButton> ResetButton;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> ResetText;
	UPROPERTY(Transient) TObjectPtr<UButton> BackButton;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> BackText;
	UPROPERTY(Transient) TObjectPtr<UBorder> ListenVeil;
	// Lane A195: the cells and buttons in the front end's item look (MenuWidget.h, FJamMenuPainter; made in BuildTree).
	TSharedPtr<struct FJamMenuPainter> Painter;
};
