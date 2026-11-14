// The main menu (lane A8): a film slate. Title HOKEJ, "a director's cut", then Choose your cut (the Designer's ListCuts),
// Direct a cut (one sentence -> RequestCut, async, with Play / Save as My Director's Cut / Try again), Settings, Credits, Quit.
// C++-drawn UMG, no widget blueprints; every screen walks with the stick or arrows, A / Enter selects, B / Esc goes back.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "MenuFlow.h"
#include "MenuWidget.generated.h"

class UWidgetSwitcher;
class UVerticalBox;
class UScrollBox;
class UUniformGridPanel;
class UEditableTextBox;
class UJamSettingsWidget;
class UJamMenuWidget;

UENUM(BlueprintType)
enum class EJamMenuScreen : uint8
{
	Main,
	Cuts,
	Direct,
	Settings,
	Credits,
	// Lane A144 (track U2): the controls, one screen from the first.
	Controls
};

UENUM()
enum class EJamMenuRowKind : uint8
{
	Cut,
	Sentence
};

// A button that carries what it stands for (a cut name or a cached sentence) back to the menu.
UCLASS()
class JAMGAME_API UJamMenuRowButton : public UButton
{
	GENERATED_BODY()

public:
	void Bind(UJamMenuWidget* InOwner, EJamMenuRowKind InKind, const FString& InPayload);

	UFUNCTION() void HandleClicked();

	FString Payload;
	EJamMenuRowKind Kind = EJamMenuRowKind::Cut;
	UPROPERTY(Transient) TObjectPtr<UJamMenuWidget> Owner;
	// The text the focus tint colours.
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TintText;
};

UCLASS()
class JAMGAME_API UJamMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	static UJamMenuWidget* CreateAndAdd(APlayerController* PC);

	UFUNCTION(BlueprintCallable, Category = "Jam|Menu") void ShowScreen(EJamMenuScreen Screen);
	UFUNCTION(BlueprintPure, Category = "Jam|Menu") EJamMenuScreen GetScreen() const { return Screen; }

	// Picks a cut by its ListCuts() name, validates it through the Designer and starts the match. False (with the error on
	// screen) when the cut is rejected. The MenuToMatchAndBack automation calls this.
	UFUNCTION(BlueprintCallable, Category = "Jam|Menu") bool ChooseCut(const FString& Name);

	// Lane A17: "PLAY THE SEASON": starts take 1 or resumes the current take. False (with the reason on screen) when there is no season.
	UFUNCTION(BlueprintCallable, Category = "Jam|Menu") bool PlaySeason();

	// Sends one sentence to the Designer (live with a key, cached without); the result fills the Direct screen.
	UFUNCTION(BlueprintCallable, Category = "Jam|Menu") void Direct(const FString& Sentence);
	UFUNCTION(BlueprintPure, Category = "Jam|Menu") bool IsRolling() const { return bRolling; }
	const FJamDesignerResult& GetLastResult() const { return Last; }
	const TArray<FJamMenuCutRow>& GetCutRows() const { return Rows; }

	// Called by UJamMenuRowButton.
	void OnRowClicked(EJamMenuRowKind Kind, const FString& Payload);

	// Lane A144 (track U2): PLAY NOW, the first screen's first press: one match on UJamMenuFlow::FirstCut, no choices. False (with the reason on the
	// first screen) when the cut is rejected.
	UFUNCTION(BlueprintCallable, Category = "Jam|Menu") bool PlayNow();
	// For Jam.Menu.FirstScreen / FitsTheScreen: the first screen's button labels top to bottom then left to right, the button the screen opens on,
	// the mode cards, and the slate units a screen asks for (the viewport is 1920 x 1080 units at both shipped resolutions: the DPI curve scales
	// 720p by 2/3). Every text of the menu wraps at a fixed width, so the measure does not depend on a first layout pass.
	TArray<FString> GetMainLabels() const;
	UButton* GetFirstPress() const { return PlayNowButton; }
	TArray<UJamMenuRowButton*> GetModeButtons() const;
	TArray<UJamMenuRowButton*> GetCutButtons() const;
	FVector2D MeasureScreen(EJamMenuScreen InScreen);
	// The widths the cards' words are wrapped at (slate units).
	static float ModeTextWidth();
	static float CutTextWidth();
	// The height the cut list is capped at: two rows of cut cards.
	static float CutListHeight();
	// The cards' type, one place for the widget and for the test that measures the words against the widths above.
	static FSlateFontInfo ModeNameFont();
	static FSlateFontInfo ModePitchFont();
	static FSlateFontInfo ModeFactFont();
	static FSlateFontInfo CutNameFont();
	static FSlateFontInfo CutLineFont();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	void BuildTree();
	UWidget* BuildMain();
	UWidget* BuildCuts();
	UWidget* BuildDirect();
	UWidget* BuildSettings();
	UWidget* BuildCredits();
	UWidget* BuildControls();
	void RefreshCuts();
	// Lane A67 (R-0036): the Director's choices row, six mode cards above the cut list.
	void RefreshModes();
	void RefreshDirect();
	void OnResult(const FJamDesignerResult& R);
	UJamDesignerSubsystem* Designer() const;
	void AddTinted(UButton* B, UTextBlock* T) { Tinted.Add(TPair<TWeakObjectPtr<UButton>, TWeakObjectPtr<UTextBlock>>(B, T)); }

	UFUNCTION() void OnPlayNowClicked();
	UFUNCTION() void OnControlsClicked();
	UFUNCTION() void OnChooseClicked();
	// Lane A17: "PLAY THE SEASON" (starts or resumes the season through UJamProgression).
	UFUNCTION() void OnSeasonClicked();
	void RefreshSeason();
	// The Designer's ListCuts() plus the season takes the player has played.
	TArray<FJamCutInfo> ListCutsWithSeason() const;
	UFUNCTION() void OnDirectClicked();
	UFUNCTION() void OnFriendClicked();
	UFUNCTION() void OnSettingsClicked();
	UFUNCTION() void OnCreditsClicked();
	UFUNCTION() void OnQuitClicked();
	UFUNCTION() void OnBackClicked();
	UFUNCTION() void OnRollClicked();
	UFUNCTION() void OnSentenceCommitted(const FText& Text, ETextCommit::Type Commit);
	UFUNCTION() void OnPlayResultClicked();
	UFUNCTION() void OnSaveResultClicked();
	UFUNCTION() void OnTryAgainClicked();

	EJamMenuScreen Screen = EJamMenuScreen::Main;
	TArray<FJamMenuCutRow> Rows;
	FJamDesignerResult Last;
	bool bRolling = false;
	bool bHasResult = false;
	bool bResultSaved = false;
	FString SavedResultPath;
	TArray<TPair<TWeakObjectPtr<UButton>, TWeakObjectPtr<UTextBlock>>> Tinted;
	TWeakObjectPtr<UWidget> PendingFocus;

	UPROPERTY(Transient) TObjectPtr<UWidgetSwitcher> Switcher;
	UPROPERTY(Transient) TObjectPtr<UWidget> TitlePicture;
	UPROPERTY(Transient) TObjectPtr<UButton> PlayNowButton;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> PlayNowText;
	UPROPERTY(Transient) TObjectPtr<UButton> ControlsButton;
	// The first screen's buttons in reading order (GetMainLabels).
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> MainLabels;
	UPROPERTY(Transient) TObjectPtr<UButton> SeasonButton;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> SeasonText;
	UPROPERTY(Transient) TObjectPtr<UButton> ChooseButton;
	UPROPERTY(Transient) TObjectPtr<UButton> DirectButton;
	UPROPERTY(Transient) TObjectPtr<UButton> FriendButton;
	UPROPERTY(Transient) TObjectPtr<UButton> SettingsButton;
	UPROPERTY(Transient) TObjectPtr<UButton> CreditsButton;
	UPROPERTY(Transient) TObjectPtr<UButton> QuitButton;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> MainStatus;
	UPROPERTY(Transient) TObjectPtr<UScrollBox> CutList;
	// Lane A144: the cuts as compact cards, three a row, inside the scroll box.
	UPROPERTY(Transient) TObjectPtr<UUniformGridPanel> CutGrid;
	UPROPERTY(Transient) TObjectPtr<UUniformGridPanel> ModeGrid;
	// Lane A81: the slate card's width box; the cuts screen widens it so twelve mode cards sit in two rows of six.
	UPROPERTY(Transient) TObjectPtr<class USizeBox> CardSize;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CutsStatus;
	UPROPERTY(Transient) TObjectPtr<UEditableTextBox> SentenceBox;
	UPROPERTY(Transient) TObjectPtr<UButton> RollButton;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> DirectStatus;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> DirectHint;
	UPROPERTY(Transient) TObjectPtr<UVerticalBox> SentenceList;
	UPROPERTY(Transient) TObjectPtr<UVerticalBox> ResultBox;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> ResultName;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> ResultChanges;
	UPROPERTY(Transient) TObjectPtr<UButton> PlayResultButton;
	UPROPERTY(Transient) TObjectPtr<UButton> SaveResultButton;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> SaveResultText;
	UPROPERTY(Transient) TObjectPtr<UButton> TryAgainButton;
	UPROPERTY(Transient) TObjectPtr<UJamSettingsWidget> SettingsPanel;
	UPROPERTY(Transient) TArray<TObjectPtr<UButton>> BackButtons;
};
