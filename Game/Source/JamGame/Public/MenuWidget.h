// The front end (lane A8, re-made by lane A187, track U2 evolution 5, R-0064 point 3): a sports-game front end of the game's own. A landing screen
// (HOKEJ large, one line, PRESS ENTER), then MAIN: PLAY NOW with its level switch, SEASON, MODES (twelve cards, ALL RINKS, MAKE YOUR OWN: the
// Designer's screen unchanged), PLAY WITH A FRIEND, CONTROLS, SETTINGS, CREDITS, QUIT. Every screen: a header (where am I), a footer with the live
// key hints, one focused element always, Back from everywhere. One style table (FJamMenuLook) for every screen. C++-drawn UMG, no widget blueprints.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "MenuFlow.h"
#include "SettingsWidget.h"
#include "Blueprint/WidgetNavigation.h"
#include "MenuWidget.generated.h"

class UWidgetSwitcher;
class UVerticalBox;
class UHorizontalBox;
class UScrollBox;
class UUniformGridPanel;
class UEditableTextBox;
class UJamSettingsWidget;
class UJamMenuWidget;
class UTexture2D;

// The screens, in the switcher's order (the values are the switcher's indices: new screens are appended).
UENUM(BlueprintType)
enum class EJamMenuScreen : uint8
{
	Main,
	// Lane A187: ALL RINKS (the shipped cuts, a saved cut, the played takes) as cards: one card of MODES.
	Cuts,
	// Lane A187: MAKE YOUR OWN, the Designer's sentence screen (unchanged): the last card of MODES.
	Direct,
	Settings,
	Credits,
	// Lane A144 (track U2): the controls, one screen from the first.
	Controls,
	// Lane A187: the landing screen (the game's name, PRESS ENTER), the first thing a player sees.
	Landing,
	// Lane A187: the twelve modes as cards, then ALL RINKS and MAKE YOUR OWN.
	Modes,
	// Lane A187: the eight takes of the season: where he is, the next one.
	Season
};

UENUM()
enum class EJamMenuRowKind : uint8
{
	Cut,
	Sentence,
	// Lane A187: a card that opens a screen (payload: the screen's QA name, "cuts" or "direct").
	Open,
	// Lane A187: a take of the season (payload: its 0-based index).
	Take
};

// Lane A187 (R-0064 point 3): the front end's one style table. Every screen reads its type, colours, spacing and motion from here; nothing is
// styled in place. Spacing is on an 8-unit grid, in slate units: the viewport is 1920 x 1080 units at 1280 x 720, 1920 x 1080 and 2560 x 1440 alike
// (the DPI curve scales by the short side). Device B's kit is read by name with a fallback (Kit, Font): the build never depends on B's push.
struct JAMGAME_API FJamMenuLook
{
	// ---- the grid
	static constexpr float Unit = 8.f;
	static constexpr float HeaderH = 80.f;        // 10 units
	static constexpr float RuleH = 8.f;           // 1: the two team colours under the header
	static constexpr float FooterH = 56.f;        // 7
	static constexpr float SideMargin = 96.f;     // 12: the main screen's left edge
	static constexpr float FocusBarW = 8.f;       // 1: the solid bar at a focused item's left edge
	static constexpr float ItemPadX = 24.f;       // 3
	static constexpr float ItemPadY = 16.f;       // 2
	static constexpr float Gap = 8.f;             // 1
	static constexpr float CardPad = 32.f;        // 4: inside a panel
	static constexpr float MainColumnW = 640.f;   // 80
	static constexpr float FeatureW = 704.f;      // 88
	static constexpr float ModesW = 1680.f;       // 210: the modes grid
	static constexpr int32 ModesPerRow = 5;
	static constexpr float ModePictureH = 88.f;   // 11
	static constexpr float ModeSlotPad = 8.f;     // 1
	static constexpr float ModeCardPadX = 16.f;   // 2
	static constexpr float SeasonW = 1280.f;      // 160
	static constexpr float PanelW = 880.f;        // 110: settings, credits, make your own
	static constexpr float WidePanelW = 1280.f;   // 160: the controls, all rinks
	// The spacing constants above, for the test that holds them on the grid.
	static TArray<float> GridValues();
	// ---- type (slate units)
	static constexpr int32 Hero = 176;            // the landing's HOKEJ
	static constexpr int32 Display = 48;          // PLAY NOW
	static constexpr int32 Title = 32;            // the header's screen name, the feature panel's title
	static constexpr int32 Item = 28;             // a main entry
	static constexpr int32 ItemSmall = 18;        // CONTROLS, SETTINGS, CREDITS, QUIT, BACK
	static constexpr int32 Body = 18;
	static constexpr int32 Caption = 14;
	static constexpr int32 TrackWide = 200;       // the landing's prompt
	static constexpr int32 TrackLabel = 60;
	// ---- motion
	static constexpr float TransitionSeconds = 0.15f;   // a screen fades and slides in (under 200 ms)
	static constexpr float TransitionSlide = 24.f;      // 3 units
	static constexpr float PromptPulseSeconds = 1.6f;   // PRESS ENTER breathes once in this
	// ---- colours (sRGB in the comments)
	static FLinearColor Home();        // #2F6BFF the home team's blue: what is CHOSEN (the lit level)
	static FLinearColor Away();        // #F26A1B the away team's orange: where the focus IS
	static FLinearColor Focus() { return Away(); }
	static FLinearColor Chosen() { return Home(); }
	static FLinearColor Ink();         // #0A0C10 the ground
	static FLinearColor Panel();       // #141821 at 0.90: an item, a card, a panel
	static FLinearColor PanelEdge();   // white at 0.12
	static FLinearColor Text();        // #F5F6F8
	static FLinearColor TextDim();     // #A3ABB8
	static FLinearColor TextOnFocus(); // #FFFFFF
	static FLinearColor Disabled();    // #4A505C
	static FLinearColor Scrim();       // black at 0.62 over the arena backdrop
	// ---- type: device B's display font when it is in the plugin, the engine's Roboto otherwise
	static FSlateFontInfo Font(int32 Size, FName Face = FName("Bold"), int32 Tracking = 0);
	// ---- device B's kit by name (/JamAssets/UI/<Name>), null when absent (asked once per process)
	static UTexture2D* Kit(const FString& Name);
	static bool HasKitFont();
	// The names the front end looks for (status/A187.md lists them for device B).
	static TArray<FString> KitNames();
};

// A button that carries what it stands for (a cut name, a cached sentence, a screen, a take) back to the menu.
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
	// The text the focus look colours.
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TintText;
	// Lane A187: the card's facts, shown under the grid while it has the focus.
	FString Detail;
};

UCLASS()
class JAMGAME_API UJamMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	static UJamMenuWidget* CreateAndAdd(APlayerController* PC);

	UFUNCTION(BlueprintCallable, Category = "Jam|Menu") void ShowScreen(EJamMenuScreen Screen);
	UFUNCTION(BlueprintPure, Category = "Jam|Menu") EJamMenuScreen GetScreen() const { return Screen; }
	// Lane A187: Esc / B. Every screen has a parent: MAKE YOUR OWN and ALL RINKS go back to MODES; MODES, SEASON, CONTROLS, SETTINGS and CREDITS to
	// MAIN; MAIN to the landing; the landing stays.
	static EJamMenuScreen ParentOf(EJamMenuScreen S);
	UFUNCTION(BlueprintCallable, Category = "Jam|Menu") void GoBack();
	// Lane A187: the landing's press (Enter, A, any key, a click): MAIN with PLAY NOW focused, so a second Enter plays.
	UFUNCTION(BlueprintCallable, Category = "Jam|Menu") void PressStart();
	// The screen's QA name (landing, main, season, modes, cuts, direct, settings, credits, controls) and back (rinks and make are names too).
	static FString ScreenName(EJamMenuScreen S);
	static bool ParseScreen(const FString& Name, EJamMenuScreen& Out);
	// Lane A187: the screen the menu opens on: the landing on a start of the game; MAIN once a match has been started in this process (a return from
	// a match) or when the game was started with -cut= (the acceptance run: its one Return still plays).
	static EJamMenuScreen OpeningScreen();
	// The controls lane's screen fired its OnBack (only while CONTROLS shows; once a frame with Esc).
	UFUNCTION() void OnControlsBack();
	// The place a screen opens on (its focus home: the tick sends the focus back there whenever it lands on the menu widget itself).
	UWidget* GetHomeFocus(EJamMenuScreen InScreen) const;
	// Lane A187: what the footer says now, and what the header says (tests and QA).
	FString GetHintLine() const;
	FString GetHeaderTitle() const;
	// MODES: every card in order (the twelve modes, ALL RINKS, MAKE YOUR OWN), by the name it shows.
	TArray<FString> GetModeCardNames() const;
	// SEASON: the rows as shown ("1  ROUGH CUT  WON").
	TArray<FString> GetSeasonRows() const;

	// Picks a cut by its ListCuts() name, validates it through the Designer and starts the match. False (with the error on
	// screen) when the cut is rejected. The MenuToMatchAndBack automation calls this.
	UFUNCTION(BlueprintCallable, Category = "Jam|Menu") bool ChooseCut(const FString& Name);

	// Lane A17: the season: starts take 1 or resumes the current take. False (with the reason on screen) when there is no season.
	UFUNCTION(BlueprintCallable, Category = "Jam|Menu") bool PlaySeason();

	// Sends one sentence to the Designer (live with a key, cached without); the result fills the Direct screen.
	UFUNCTION(BlueprintCallable, Category = "Jam|Menu") void Direct(const FString& Sentence);
	UFUNCTION(BlueprintPure, Category = "Jam|Menu") bool IsRolling() const { return bRolling; }
	const FJamDesignerResult& GetLastResult() const { return Last; }
	const TArray<FJamMenuCutRow>& GetCutRows() const { return Rows; }

	// Called by UJamMenuRowButton.
	void OnRowClicked(EJamMenuRowKind Kind, const FString& Payload);

	// Lane A144 (track U2): PLAY NOW, the main screen's first press: one match on UJamMenuFlow::FirstCut, no choices. False (with the reason on
	// the main screen) when the cut is rejected. Lane A187: from any screen (jam.menuscreen play).
	UFUNCTION(BlueprintCallable, Category = "Jam|Menu") bool PlayNow();
	// For Jam.Menu.FirstScreen / FitsTheScreen: the main screen's button labels top to bottom then left to right, the button the screen opens on,
	// the mode cards, and the slate units a screen asks for (the viewport is 1920 x 1080 units at every shipped resolution). Every text of the menu
	// wraps at a fixed width, so the measure does not depend on a first layout pass.
	TArray<FString> GetMainLabels() const;
	UButton* GetFirstPress() const { return PlayNowButton; }
	// Evolution 2: the line under PLAY NOW, and what it will start (re-read from the settings each time the main screen shows).
	FString GetFirstPressLine() const;

	// Lane A144 (evolution 3): the level is chosen where it is played. Three segments under PLAY NOW (ROOKIE / PRO / ALL-STAR, the level in force
	// lit); a click picks one; with the keys or the pad, left and right ON PLAY NOW step it (a custom navigation rule on the button: arrows, D-pad
	// and stick alike), so the focus stays on PLAY NOW and one Enter still starts the match. A pick writes the setting the SETTINGS button writes
	// (FJamSettings::Difficulty with bDifficultyChosen) and both screens re-read it when they show.
	UFUNCTION(BlueprintCallable, Category = "Jam|Menu") void PickLevel(EJamMenuDifficulty Level);
	// One step left (-1) or right (+1); the ends hold (rookie is the left end, all-star the right).
	UFUNCTION(BlueprintCallable, Category = "Jam|Menu") void StepLevel(int32 Delta);
	// What the switch shows: the lit segment, the three labels, the line under it.
	EJamMenuDifficulty GetSwitchLevel() const { return SwitchLevel; }
	TArray<FString> GetSwitchLabels() const;
	FString GetLevelMeaningLine() const;
	// The twelve mode cards (lane A187: the two cards that open a screen are not in this list).
	TArray<UJamMenuRowButton*> GetModeButtons() const;
	TArray<UJamMenuRowButton*> GetCutButtons() const;
	FVector2D MeasureScreen(EJamMenuScreen InScreen);

	// Lane A144 (evolution 2): the pad walk, `jam.menuscreen walk`. On every screen in turn it stands on each thing a pad can stand on, presses the
	// D-pad each way (key events given to Slate, the path a real pad's take; no cursor, no window focus), follows where the focus goes, then presses
	// B and checks the screen's parent is back. One JAMMENU walk line a screen (focusables, reached, traps = a place no direction leaves, unreachable
	// names, back) and a JAMMENU walk done line. A never pressed: A on a button is a click, and the acceptance run's Return covers PLAY NOW.
	// Directional navigation reads the window's hit-test grid, and only a windowed game has the menu in it: in an off-screen process the walk
	// says grid=blind for the screen and verdict=blind at the end (the opening focus and the B press are still judged there).
	void StartPadWalk();
	// QA (jam.menuscreen key <name>): one key or pad press given to Slate for the focused widget, down and up, as the walk gives its presses.
	// Enter on the landing is its press; Enter on MAIN is the acceptance run's Return; left and right on PLAY NOW are the level switch's steps.
	void PressKeyForQa(const FKey& Key);
	bool IsPadWalking() const { return WalkScreen >= 0; }
	// Lane A164 (evolution 4): no dead key. A click on a level segment gives the keyboard focus back to PLAY NOW (Slate would leave it on the menu
	// widget itself: the segments take no focus); Return or the pad's A that reach the menu widget itself on MAIN press PLAY NOW; the tick sends the
	// focus home whenever it lands on the menu widget itself (lane A187: on every screen). For tests: a segment by level, and whether PLAY NOW has
	// the focus of the menu's player.
	UButton* GetLevelButton(EJamMenuDifficulty Level) const { const int32 I = static_cast<int32>(Level); return LevelButtons.IsValidIndex(I) ? LevelButtons[I].Get() : nullptr; }
	bool IsFocusOnPlayNow() const;
	UJamSettingsWidget* GetSettingsPanel() const { return SettingsPanel; }
	// QA and tests (jam.menuscreen click <rookie|pro|all_star|play|settingslevel|start|modes|season|back>): a mouse click as Slate gives it, without a
	// cursor: the pointer moves onto the widget, goes down and comes up, routed by FSlateApplication::RoutePointer*Event down the widget's own path
	// (generated from the widget tree, so no hit-test grid and no active window are needed), the mouse-down's focus rule included. False with the
	// reason when the widget has no path. WasLastClickDownOnMenu: after that click's mouse-down the focus sat on the menu widget itself.
	bool ClickForQa(UWidget* W, FString& Why);
	bool ClickPlaceForQa(const FString& Name);
	bool WasLastClickDownOnMenu() const { return bLastClickDownOnMenu; }
	// QA (jam.menuscreen where): every place on the shown screen a click or a pad can stand on, its label and its centre in the game window's
	// client pixels and at 1920 x 1080 (JAMMENU place lines), read from the widgets' own painted geometry: no driver hard-codes a number.
	TArray<FString> GetPlacesForQa() const;
	void PrintPlacesForQa() const;
	// The widths the cards' words are wrapped at (slate units).
	static float ModeTextWidth();
	static float CutTextWidth();
	// The height the cut list is capped at: rows of cut cards.
	static float CutListHeight();
	// The cards' type, one place for the widget and for the test that measures the words against the widths above.
	static FSlateFontInfo ModeNameFont();
	static FSlateFontInfo ModePitchFont();
	static FSlateFontInfo ModeFactFont();
	static FSlateFontInfo CutNameFont();
	static FSlateFontInfo CutLineFont();
	// Lane A187: the width of the line under the modes grid that carries the focused card's facts.
	static float ModeDetailWidth();
	// The height of a mode card's two-line description box (two full lines of ModePitchFont, descenders included).
	static float ModePitchBoxHeight();
	// Every MODES card's description as laid out (its desired size after a prepass: the wrapped lines' real height), for the fit test.
	TArray<FVector2D> GetModePitchSizes() const;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	// Lane A187: a click anywhere on the landing is its press.
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	void BuildTree();
	// Lane A187: the chrome and the new screens.
	UWidget* BuildHeader();
	UWidget* BuildFooter();
	UWidget* BuildLanding();
	UWidget* BuildModes();
	UWidget* BuildSeason();
	// A screen's body on a centred panel of this width.
	UWidget* OnPanel(UWidget* Body, float Width);
	UButton* MakeBack(EJamMenuScreen Of);
	void RefreshSeasonScreen();
	void RefreshFooter();
	void RefreshFeature();
	void RefreshModeDetail();
	void AddItem(UButton* B, UTextBlock* T, UWidget* Bar = nullptr, bool bSegment = false);
	// A second text of an item (a sub line): it turns white on the focus fill with the label.
	void AddSub(UButton* B, UTextBlock* T);
	void PaintItems();
	UFUNCTION() void OnLandingClicked();
	UFUNCTION() void OnModesClicked();
	UFUNCTION() void OnContinueSeasonClicked();
	UWidget* BuildMain();
	UWidget* BuildCuts();
	UWidget* BuildDirect();
	UWidget* BuildSettings();
	UWidget* BuildCredits();
	UWidget* BuildControls();
	void RefreshCuts();
	// Lane A67 (R-0036), lane A187: the twelve mode cards and the two that open a screen, on MODES.
	void RefreshModes();
	void RefreshDirect();
	void OnResult(const FJamDesignerResult& R);
	UJamDesignerSubsystem* Designer() const;
	void AddTinted(UButton* B, UTextBlock* T) { AddItem(B, T); }

	UFUNCTION() void OnPlayNowClicked();
	UFUNCTION() void OnLevelRookieClicked();
	UFUNCTION() void OnLevelProClicked();
	UFUNCTION() void OnLevelAllStarClicked();
	// Lane A164: a segment's click picks the level and gives the focus back to PLAY NOW.
	void OnSegmentClicked(EJamMenuDifficulty Level);
	UFUNCTION() UWidget* OnPlayNowNavigate(EUINavigation Direction);
	void RefreshLevelSwitch();
	UFUNCTION() void OnControlsClicked();
	UFUNCTION() void OnChooseClicked();
	// Lane A187: SEASON opens the season's screen (the takes, CONTINUE).
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

	void TickPadWalk();
	TArray<UWidget*> FocusablesOf(EJamMenuScreen InScreen) const;
	int32 WalkFocusedIndex() const;
	int32 WalkScreen = -1;      // index into the walk's screen order; -1 = no walk
	int32 WalkNode = -2;        // -2 entering a screen, -1 taking the next place, -3 the B press, else the place being probed
	int32 WalkDir = 0;
	int32 WalkPhase = 0;        // ticks into the current step
	int32 WalkStart = -1;
	int32 WalkEdges = 0;
	int32 WalkTotals[4] = { 0, 0, 0, 0 };   // focusables, reached, traps, backs failed
	bool bWalkBlind = false;    // this screen: no direction moved the focus from any place AND the grid has no menu under the opening place: not judged
	bool bWalkGridHit = false;
	bool bLastClickDownOnMenu = false;  // lane A164: a diagnostic only (the hit-test grid's path under the opening place reaches it); the verdict reads the moves
	int32 WalkBlindScreens = 0;
	TArray<TWeakObjectPtr<UWidget>> WalkNodes;
	TArray<int32> WalkQueue;
	TArray<int32> WalkOut;
	TSet<int32> WalkReached;

	EJamMenuScreen Screen = EJamMenuScreen::Main;
	TArray<FJamMenuCutRow> Rows;
	FJamDesignerResult Last;
	bool bRolling = false;
	bool bHasResult = false;
	bool bResultSaved = false;
	FString SavedResultPath;
	// Lane A187: every focusable thing of the menu with its label and its focus bar; PaintItems gives each its look every tick from FJamMenuLook.
	struct FItem
	{
		TWeakObjectPtr<UButton> Button;
		TWeakObjectPtr<UTextBlock> Text;
		TWeakObjectPtr<UWidget> Bar;
		TArray<TWeakObjectPtr<UTextBlock>> Subs;
		bool bSegment = false;
		int32 State = -1;   // the look painted last: 0 disabled, 1 rest, 2 focus, 3 chosen
	};
	TArray<FItem> Items;
	TWeakObjectPtr<UWidget> PendingFocus;
	FVector2D LastCursor = FVector2D::ZeroVector;
	float TransitionT = 1.f;
	float PulseT = 0.f;
	uint64 LastBackFrame = 0;
	FString FooterKey;
	TWeakObjectPtr<UWidget> FeatureFor;

	UPROPERTY(Transient) TObjectPtr<UWidgetSwitcher> Switcher;
	UPROPERTY(Transient) TObjectPtr<UWidget> TitlePicture;
	UPROPERTY(Transient) TObjectPtr<UButton> PlayNowButton;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> PlayNowText;
	// Evolution 2: the small line under PLAY NOW names the level the match will play ("ONE MATCH, ROOKIE.  ENTER / A, OR CLICK.").
	UPROPERTY(Transient) TObjectPtr<UTextBlock> PlayNowSub;
	// Evolution 3: the level switch (three click-only segments) and the line that says what the lit level means.
	UPROPERTY(Transient) TArray<TObjectPtr<UButton>> LevelButtons;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> LevelTexts;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> LevelMeaningText;
	EJamMenuDifficulty SwitchLevel = EJamMenuDifficulty::Rookie;
	UPROPERTY(Transient) TObjectPtr<UButton> ControlsButton;
	// The main screen's buttons in reading order (GetMainLabels).
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> MainLabels;
	UPROPERTY(Transient) TObjectPtr<UButton> SeasonButton;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> SeasonText;
	UPROPERTY(Transient) TObjectPtr<UButton> FriendButton;
	UPROPERTY(Transient) TObjectPtr<UButton> SettingsButton;
	UPROPERTY(Transient) TObjectPtr<UButton> CreditsButton;
	UPROPERTY(Transient) TObjectPtr<UButton> QuitButton;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> MainStatus;
	UPROPERTY(Transient) TObjectPtr<UScrollBox> CutList;
	// Lane A144: the cuts as compact cards, three a row, inside the scroll box.
	UPROPERTY(Transient) TObjectPtr<UUniformGridPanel> CutGrid;
	UPROPERTY(Transient) TObjectPtr<UUniformGridPanel> ModeGrid;
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
	// Lane A187: one BACK a screen, indexed by the screen's value (none on the landing and on MAIN).
	UPROPERTY(Transient) TArray<TObjectPtr<UButton>> BackButtons;
	UPROPERTY(Transient) TObjectPtr<UWidget> HeaderBox;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> HeaderTitle;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> HeaderLevel;
	UPROPERTY(Transient) TObjectPtr<UHorizontalBox> FooterHints;
	UPROPERTY(Transient) TObjectPtr<UButton> LandingButton;
	UPROPERTY(Transient) TObjectPtr<UWidget> LandingPrompt;
	UPROPERTY(Transient) TObjectPtr<UButton> ModesButton;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> FeatureTitle;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> FeatureBody;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> ModeDetail;
	UPROPERTY(Transient) TObjectPtr<UButton> ContinueButton;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> ContinueText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> SeasonSummary;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> SeasonDetail;
	UPROPERTY(Transient) TObjectPtr<UVerticalBox> SeasonList;
	UPROPERTY(Transient) TArray<TObjectPtr<UJamMenuRowButton>> SeasonRowButtons;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> SeasonRowTexts;
	TArray<FString> SeasonNotes;
	// Lane A187: the controls lane's rebinding screen (UJamSettingsWidget::MakeControlsScreen, A183) when it is in the tree; null until then.
	UPROPERTY(Transient) TObjectPtr<UUserWidget> ControlsScreen;
	TFunction<UWidget*()> ControlsFirst;
	TArray<TWeakObjectPtr<UTextBlock>> ModePitchTexts;
};
