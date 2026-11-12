// The in-match cards of lane A8. UJamDirectorsCallWidget: the pause menu (resume, Director's Call, restart, quit to menu) and the
// Director's Call card: 3-5 tunables of the current cut as sliders inside their ranges, APPLY calls
// UJamDesignerSubsystem::DirectorsCall(key, value) once per TAKE, greyed until the next TAKE. UJamDirectorsCallHudWidget: the
// overlay with the "TAKE n" period card and the scoreboard line "DIRECTOR'S CALL: <text>". C++-drawn UMG, stick and A/B complete.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "JamDesignerSubsystem.h"
#include "DirectorsCallController.h"
#include "HUDFeed.h"
#include "DirectorsCallWidget.generated.h"

class UWidgetSwitcher;
class UVerticalBox;
class UCanvasPanel;
class UImage;
class UJamDirectorsCallWidget;

// One tunable the card offers.
USTRUCT(BlueprintType)
struct JAMGAME_API FJamCallTunable
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|DirectorsCall") FName Key;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|DirectorsCall") FString Label;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|DirectorsCall") FString Description;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|DirectorsCall") double Min = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|DirectorsCall") double Max = 1;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|DirectorsCall") double Current = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|DirectorsCall") bool bInteger = false;

	double FromNormalized(float S) const;
	float ToNormalized(double V) const { return Max > Min ? static_cast<float>(FMath::Clamp((V - Min) / (Max - Min), 0.0, 1.0)) : 0.f; }
};

// A slider that knows its row.
UCLASS()
class JAMGAME_API UJamCallSlider : public USlider
{
	GENERATED_BODY()

public:
	void Bind(UJamDirectorsCallWidget* InOwner, int32 InRow);
	UFUNCTION() void HandleChanged(float V);

	int32 Row = 0;
	UPROPERTY(Transient) TObjectPtr<UJamDirectorsCallWidget> Owner;
};

UCLASS()
class JAMGAME_API UJamDirectorsCallWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	static UJamDirectorsCallWidget* CreateAndAdd(APlayerController* PC, UJamDirectorsCallController* InOwner);

	// Shows the pause menu (the call card is rebuilt from the current cut).
	void Open();
	void Close();
	bool IsOpen() const;
	void ShowPause();
	void ShowCall();
	bool IsOnCallCard() const { return bOnCall; }

	// The 3-5 tunables offered for a cartridge text, in order: skater speed, shot power, ice friction, pickup rate (when the
	// cut has one), opponents' skill, then any other number tunable until Max. Current = values[key] or the default.
	static TArray<FJamCallTunable> PickTunables(const FString& CartridgeJson, int32 Max = 5);
	// "Director's Call: ice is now glass" -> "DIRECTOR'S CALL: ice is now glass".
	static FString AnnouncementLine(const FString& Announcement);
	// Lane A15 (C-0005): a rule's announce text -> "RULE: <text>", the sibling of the Director's Call line on the same scoreboard slot.
	static FString RuleLine(const FString& Text);

	// Applies the first slider the player moved (one change per TAKE). Not applied (with the reason) when nothing moved.
	UFUNCTION(BlueprintCallable, Category = "Jam|DirectorsCall") FJamDirectorsCall ApplySelected();
	// Applies one key directly (console and automation).
	UFUNCTION(BlueprintCallable, Category = "Jam|DirectorsCall") FJamDirectorsCall ApplyValue(FName Key, double Value);

	const TArray<FJamCallTunable>& GetTunables() const { return Tunables; }
	void OnSliderChanged(int32 Row, float V);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	struct FRow
	{
		UJamCallSlider* Slider = nullptr;
		UTextBlock* Label = nullptr;
		UTextBlock* Value = nullptr;
		float Start = 0.f;
	};

	void BuildTree();
	void RebuildRows();
	void RefreshState();
	FString ValueText(const FJamCallTunable& T, double V) const;
	UJamDesignerSubsystem* Designer() const;
	void AfterCall(const FJamDirectorsCall& Call);

	UFUNCTION() void OnResumeClicked();
	UFUNCTION() void OnCallClicked();
	UFUNCTION() void OnRestartClicked();
	UFUNCTION() void OnQuitClicked();
	UFUNCTION() void OnApplyClicked();
	UFUNCTION() void OnBackClicked();

	TWeakObjectPtr<UJamDirectorsCallController> Owner;
	TArray<FJamCallTunable> Tunables;
	TArray<FRow> Rows;
	TArray<TPair<TWeakObjectPtr<UButton>, TWeakObjectPtr<UTextBlock>>> Tinted;
	TWeakObjectPtr<UWidget> PendingFocus;
	bool bOnCall = false;
	FString LastAnnouncement;

	UPROPERTY(Transient) TObjectPtr<UWidgetSwitcher> Switcher;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TakeText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> PauseStatus;
	UPROPERTY(Transient) TObjectPtr<UButton> ResumeButton;
	UPROPERTY(Transient) TObjectPtr<UButton> CallButton;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CallButtonText;
	UPROPERTY(Transient) TObjectPtr<UButton> RestartButton;
	UPROPERTY(Transient) TObjectPtr<UButton> QuitButton;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CallTitle;
	UPROPERTY(Transient) TObjectPtr<UVerticalBox> RowsBox;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CallStatus;
	UPROPERTY(Transient) TObjectPtr<UButton> ApplyButton;
	UPROPERTY(Transient) TObjectPtr<UButton> BackButton;
};

// The overlay: period card and the Director's Call scoreboard line. Frames are simulation frames from the presentation feed.
UCLASS()
class JAMGAME_API UJamDirectorsCallHudWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	static UJamDirectorsCallHudWidget* CreateAndAdd(APlayerController* PC);

	void ShowTake(int32 Take, int64 UntilFrame);
	void ShowAnnouncement(const FString& Line, int64 UntilFrame);
	// Called once per simulation frame; bCallReady lights the hint.
	void TickFrame(const FJamPresentationFrame& Frame, bool bCallReady);

	UFUNCTION(BlueprintPure, Category = "Jam|DirectorsCall") FString GetTakeCardText() const;
	UFUNCTION(BlueprintPure, Category = "Jam|DirectorsCall") FString GetAnnouncementText() const;
	bool IsTakeCardVisible() const;

	// Lane A42 (y-slots): the band of Slate y the MATCH banner covers at a viewport height VH (anchor 0.4, the 640 x 360 card, the FACEOFF
	// line under it), so the bark bubbles keep out of it. Pure (tests call it); GetBannerBandPx answers only while the card is up.
	static void BannerBandPx(float VH, float& OutTop, float& OutBottom) { OutTop = VH * 0.4f - 190.f; OutBottom = VH * 0.4f + 236.f; }
	bool GetBannerBandPx(float VH, float& OutTop, float& OutBottom) const
	{
		if (!IsTakeCardVisible()) return false;
		BannerBandPx(VH, OutTop, OutBottom);
		return true;
	}

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void BuildTree();

	int64 TakeUntil = -1;
	int64 AnnouncementUntil = -1;
	int64 LastFrame = -1;

	UPROPERTY(Transient) TObjectPtr<UCanvasPanel> Root;
	UPROPERTY(Transient) TObjectPtr<UWidget> TakeArt;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TakeCard;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TakeSub;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> Announcement;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> ReadyHint;
};
