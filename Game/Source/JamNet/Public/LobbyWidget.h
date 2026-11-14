// Lane A9: the lobby screens (C++-drawn UMG in A5's style, no widget blueprints) and the match-end card for desync /
// connection lost. One widget covers host / join / waiting / ready: it reads UJamLobbySubsystem every tick and shows
// the right rows. Opened by the console command jam.lobby (lane A8's "Play with a friend" button runs that command).
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/SlateEnums.h"   // ETextCommit (lane A195)
#include "InputCoreTypes.h"   // FKey (lane A195)
#include "LobbyWidget.generated.h"

class UCanvasPanel;
class UVerticalBox;
class UHorizontalBox;
class UTextBlock;
class UBorder;
class UButton;
class UEditableTextBox;
class UJamLobbySubsystem;

UCLASS()
class JAMNET_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Creates the lobby over the current map, shows the cursor. Returns the existing one when already open.
	static ULobbyWidget* Open(APlayerController* PC);

	UFUNCTION(BlueprintPure, Category = "Jam|Lobby") FString GetStatusLine() const;
	// Lane A195 (U2 evolution 7): the lobby in the front end's style table; for the tests: the panel (measured), every fixed word shown, and the rink's
	// name a cut id is shown by ("hockey.regular" -> "REGULAR").
	UWidget* GetPanelForTest() const { return Panel; }
	TArray<FString> GetWordsForTest() const;
	static FString RinkFor(const FString& CutId);
	static constexpr float ColumnW = 748.f;
	// Lane A195 (FAULT 6): the first button a player stands on (HOST when idle), and the keys that leave the lobby for HOME (Esc, pad B: BACK's road).
	UWidget* GetFirstFocusable() const;
	static bool IsLeaveKey(const FKey& K);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;   // lane A195

private:
	void BuildTree();
	UJamLobbySubsystem* Lobby() const;
	UFUNCTION() void OnHost();
	UFUNCTION() void OnJoin();
	UFUNCTION() void OnStart();
	UFUNCTION() void OnBack();
	UFUNCTION() void OnCutPrev();
	UFUNCTION() void OnCutNext();
	UFUNCTION() void OnCodeCommitted(const FText& InText, ETextCommit::Type Commit);   // lane A195 (FAULT 7 class)

	UButton* MakeButton(const TCHAR* Name, const FString& Label, UTextBlock*& OutText);

	UPROPERTY(Transient) TObjectPtr<UCanvasPanel> Root;
	UPROPERTY(Transient) TObjectPtr<UVerticalBox> Column;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> Title;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> Status;
	UPROPERTY(Transient) TObjectPtr<UHorizontalBox> HostRow;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CutText;
	UPROPERTY(Transient) TObjectPtr<UButton> HostButton;
	UPROPERTY(Transient) TObjectPtr<UHorizontalBox> JoinRow;
	UPROPERTY(Transient) TObjectPtr<UEditableTextBox> CodeBox;
	UPROPERTY(Transient) TObjectPtr<UButton> JoinButton;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CodesText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> WarningText;
	// Lane A144 (track U2): who does what, in docs/experiments/JOIN-GUIDE.md's words: over the host's row, over the guest's row, under the codes.
	UPROPERTY(Transient) TObjectPtr<UTextBlock> HostHelp;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> JoinHelp;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CodesHelp;
	UPROPERTY(Transient) TObjectPtr<UHorizontalBox> NamesRow;
	UPROPERTY(Transient) TObjectPtr<UBorder> HomePlate;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> HomeName;
	UPROPERTY(Transient) TObjectPtr<UBorder> AwayPlate;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> AwayName;
	UPROPERTY(Transient) TObjectPtr<UButton> StartButton;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> StartText;
	UPROPERTY(Transient) TObjectPtr<UButton> BackButton;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> NameText;
	UPROPERTY(Transient) TObjectPtr<UWidget> Panel;   // lane A195
	TSharedPtr<struct FJamMenuPainter> Painter;   // lane A195: the buttons in the item look
	TWeakObjectPtr<UWidget> PendingFocus;   // lane A195 (FAULT 6)

	TArray<FString> Cuts;
	int32 CutIndex = 0;
	int32 TicksOpen = 0;
	bool bShotTaken = false;
};

// A full-screen card: big title ("DESYNC", "CONNECTION LOST") and one line under it.
UCLASS()
class JAMNET_API ULobbyCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	static ULobbyCardWidget* Show(APlayerController* PC, const FString& InTitle, const FString& InSubtitle);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TitleText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> SubtitleText;
	FString PendingTitle;
	FString PendingSubtitle;
};
