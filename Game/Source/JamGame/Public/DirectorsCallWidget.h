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
#include "MenuWidget.h"
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
	// Lane A144 (evolution 3): the pause card is 800 wide with 28 of padding a side; its controls table is written to that width
	// (Jam.Menu.PauseCardFits measures every line against it).
	// Lane A195: 808 wide with 32 of padding a side (the style table's card padding): the inner width stays 744.
	static constexpr float PauseCardWidth = 808.f;
	static constexpr float PauseCardPad = 32.f;
	static constexpr int32 PauseItemSize = 22;
	static constexpr float PauseCardInnerWidth = 744.f;
	static FString PauseControlsText();
	// Lane A195 (evolution 7): what the pause card shows instead: the two mouse buttons with the puck and without it (live bindings), two lines.
	static FString PauseStickText();
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
	// "Director's Call: ice is now glass" -> "RULE CHANGE: ice is now glass" (lane A195).
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
	UFUNCTION() void OnControlsClicked();   // lane A191: the CONTROLS screen from the pause card
	UFUNCTION() void OnControlsBack();

	TWeakObjectPtr<UJamDirectorsCallController> Owner;
	TArray<FJamCallTunable> Tunables;
	TArray<FRow> Rows;
	// Lane A195: the items in the front end's look (the focus fill and bar), and the key hints under the card.
	FJamMenuPainter Painter;
	void RefreshHints();
	FString HintKey;
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
	UPROPERTY(Transient) TObjectPtr<UButton> ControlsButton;   // lane A191
	UPROPERTY(Transient) TObjectPtr<class UJamControlsWidget> ControlsScreen;   // lane A191: screen 2 of the switcher
	UPROPERTY(Transient) TObjectPtr<class UHorizontalBox> HintBox;   // lane A195
	UPROPERTY(Transient) TObjectPtr<UTextBlock> PauseStick;   // lane A195 (evolution 7)
public:
	// Lane A191: the CONTROLS screen (rebind every verb) as the pause card's third screen; BACK or Esc returns to the pause menu.
	void ShowControls();
	// Lane A195 (F6): QUIT TO MENU throws the match away, so it asks once: "LEAVE THE MATCH?", NO focused, YES under it; Esc / B is NO.
	// jam.pausemenu leave | no | yes.
	void ShowLeaveConfirm();
	void AnswerLeave(bool bYes);
	bool IsOnLeaveConfirm() const;
	UButton* GetLeaveNo() const { return LeaveNo; }
	void BuildLeaveConfirm();
	UFUNCTION() void OnLeaveNo();
	UFUNCTION() void OnLeaveYes();
	int32 LeaveIndex = -1;
	UPROPERTY(Transient) TObjectPtr<UButton> LeaveNo;
	UPROPERTY(Transient) TObjectPtr<UButton> LeaveYes;
	bool IsOnControls() const;
	class UJamControlsWidget* GetControlsScreen() const { return ControlsScreen; }
	// Lane A195 (tests and QA): the key hints under the card as one line, the title and buttons as shown, the painted state of a button.
	FString GetHintLine() const { return HintKey; }
	// The key hints' row as laid out (after a prepass), for the fit test: it stays inside PauseCardInnerWidth.
	float GetHintRowWidth() const;
	TArray<FString> GetPauseLabels() const;
	UButton* GetResumeButton() const { return ResumeButton; }
	int32 GetPaintedState(const UButton* B) const { return Painter.StateOf(B); }
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
	// Lane A144 (the level wire): the small line of the match card (it ends with the level in force), and a level's label.
	FString GetTakeSubText() const { return TakeSub ? TakeSub->GetText().ToString() : FString(); }
	static FString LevelLabel(const FString& Level);
	UFUNCTION(BlueprintPure, Category = "Jam|DirectorsCall") FString GetAnnouncementText() const;
	bool IsTakeCardVisible() const;
	// Lane A123 (track U1, evolution 2): the "ESC / START   CALL READY" pill shows when a call is ready by the HUD's own rule (UJamHudWidget::ShowDirectorsCall:
	// after the first goal), not from the first faceoff of a 0:0 match. bAfterGoalOnly false (jam.hud.CallPillAfterGoal 0) is the pill of before. Pure (tests call it).
	static bool ShowReadyPill(bool bCallReady, int32 Goals, bool bEnd, bool bLine, bool bAfterGoalOnly = true)
	{
		return bCallReady && !bEnd && !bLine && (!bAfterGoalOnly || Goals > 0);
	}
	// True while the scoreboard line (a call's result, a RULE line) is up: the HUD's one-time prompt takes the slot under it then.
	bool IsLineVisible() const;

	// Lane A42: the MATCH card in the top band under the scoreboard at 0.7 scale (A48's storyboard finding 3): 448 x 252 Slate units centred at
	// y MatchCardCentreY, the FACEOFF line under it; the human and the countdown ring stay clear.
	static constexpr float MatchCardW = 448.f;
	// Lane A64 (eyes grid h19 finding 14, 23 stills: the card covered the opposing centre at the drop): the banner type 77 -> 50 pt, the card
	// 165 -> about 120 px tall, centred at 190 (top under the scoreboard line at 112), so the far half of the faceoff circle stays clear.
	static constexpr float MatchCardH = 130.f;
	static constexpr float MatchCardCentreY = 190.f;
	static constexpr int32 MatchCardFontPt = 50;
	// Lane A76 (EYES-GRID-FIXES row 6, partial): on the big rinks the far centre stands higher in the frame; the card's type follows the rink:
	// 50 pt up to 90 m, 40 pt on 120 m and longer. jam.ui.CardMode 1 also lifts the card to y 150 on the big rinks (the proof compares the two).
	static int32 MatchCardPtFor(float RinkLengthM) { return RinkLengthM > 95.f ? 40 : MatchCardFontPt; }
	static float MatchCardCentreYFor(float RinkLengthM, bool bLift) { return (bLift && RinkLengthM > 95.f) ? 150.f : MatchCardCentreY; }
	// Lane A42 (y-slots): the band of Slate y the MATCH card covers (card plus the line under it; VH kept for callers, the band is absolute),
	// so the bark bubbles keep out of it. Pure (tests call it); GetBannerBandPx answers only while the card is up.
	static void BannerBandPx(float VH, float& OutTop, float& OutBottom) { (void)VH; OutTop = MatchCardCentreY - MatchCardH * 0.5f - 4.f; OutBottom = MatchCardCentreY + MatchCardH * 0.5f + 40.f; }
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
	// Lane A229 (HUDOneLook.cpp): the MATCH card, the scoreboard line and the ready hint in the controls panel's look (jam.hud.OneLook 0 = before);
	// the card's type at Pt in that look (the menu's face, 8 smaller) or the type of before.
	void ApplyOneLook();
	static FSlateFontInfo MatchCardFont(int32 Pt);

	int64 TakeUntil = -1;
	int64 AnnouncementUntil = -1;
	int64 LastFrame = -1;
	float CardRinkLen = -1.f;   // lane A76: the rink length and mode the card's type and place were last set for
	int32 CardMode = -1;

	UPROPERTY(Transient) TObjectPtr<UCanvasPanel> Root;
	UPROPERTY(Transient) TObjectPtr<UWidget> TakeArt;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TakeCard;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TakeSub;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> Announcement;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> ReadyHint;
};
