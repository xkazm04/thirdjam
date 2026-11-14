#include "MenuWidget.h"
#include "MenuStyle.h"
#include "SettingsWidget.h"
#include "JamCartridge.h"
#include "Progression.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/SizeBoxSlot.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/ButtonSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/Spacer.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

using namespace JamMenuStyle;
using S = FJamUiStyle;

DEFINE_LOG_CATEGORY_STATIC(LogJamMenuUi, Log, All);

// Lane A18 (visual layer, R-0013 point 5): the slate says HOKEJ and nothing under it; the theme lives in the cuts, the takes and the
// Director's Call. Type and plates from FJamUiStyle. Logic is lane A8's and A17's. Lane A144 (track U2): the first screen says what the game is
// and opens on PLAY NOW; the cut rows left the film-strip frame (their words sat under its sprocket holes) for compact cards with the kit icon.
namespace
{
	// Lane A81: the Director's choices on a slate card this wide on the cuts screen. Lane A144 (track U2): four cards a row (three rows hold twelve),
	// so a name fits its card at a size that reads at 720p (six a row left 172 units: OUTNUMBERED ran into the edge) and a fact stays on one line.
	constexpr int32 ModesPerRow = 4;
	constexpr float ModeCardWidth = 1240.f;
	constexpr float SlateWidth = 880.f;          // every other screen's card
	constexpr float ControlsCardWidth = 1240.f;
	constexpr float ScreenPadX = 32.f;           // the switcher's side padding inside the card
	// Widths in slate units. A mode cell is a quarter of the card's inner width; the button's own padding and the grid's come off it.
	constexpr float ModeSlotPad = 4.f;
	constexpr float ModePadX = 12.f;
	constexpr float ModeTextW = (ModeCardWidth - 2.f * ScreenPadX) / ModesPerRow - 2.f * ModeSlotPad - 2.f * ModePadX;   // 262
	// The cuts: three compact cards a row inside the scroll box. Its bar and the bar's padding take 30 units (read off the 1080p still: the cards
	// are 374 wide); 6 more are kept free, so a line never reaches the card's clip (at 308 the Chaos Cut's "2 obstacles" lost half a digit at 1080p).
	constexpr int32 CutsPerRow = 3;
	constexpr float CutIcon = 36.f;
	constexpr float CutPadX = 12.f;
	constexpr float CutTextW = (ModeCardWidth - 2.f * ScreenPadX - 30.f) / CutsPerRow - 2.f * ModeSlotPad - 2.f * CutPadX - CutIcon - 10.f - 6.f;   // 298
	constexpr float CutBodyH = 78.f;             // a cut card's words: the name and two lines (70 cut the second line in half)
	constexpr float CutListH = 2.f * (CutBodyH + 14.f + 2.f * ModeSlotPad) + 4.f;   // 204: two rows of cut cards
	constexpr float BodyTextW = SlateWidth - 2.f * ScreenPadX;   // 816: a paragraph on an 880 card

	// A text box that reads on the slate: chalk on dark (the engine's default is a light box whose grey hint could not be read).
	void DarkBox(UEditableTextBox* Box)
	{
		Box->WidgetStyle.SetBackgroundColor(FLinearColor(0.16f, 0.17f, 0.20f, 1.f));
		Box->WidgetStyle.SetForegroundColor(FJamUiStyle::Chalk());
		Box->WidgetStyle.SetFocusedForegroundColor(FJamUiStyle::Chalk());
	}

	// A text block that wraps at a fixed width: its desired size is the same before and after the first layout pass (auto-wrap is not), which is
	// what Jam.Menu.FitsTheScreen measures.
	UTextBlock* Wrapped(UTextBlock* Txt, float Width)
	{
		Txt->SetAutoWrapText(false);
		Txt->SetWrapTextAt(Width);
		return Txt;
	}
	// The kit icon of a cut row.
	FString IconFor(const FJamMenuCutRow& R)
	{
		if (R.bSaved) return TEXT("cut-directors");
		if (R.Name == TEXT("hockey.raw")) return TEXT("cut-raw");
		if (R.Name == TEXT("hockey.boost")) return TEXT("cut-boost");
		if (R.Name == TEXT("hockey.arena.large")) return TEXT("cut-large");
		if (R.Name == TEXT("hockey.arena.wide")) return TEXT("cut-wide");
		return TEXT("cut-regular");
	}
}

// ---------------------------------------------------------------- row button

void UJamMenuRowButton::Bind(UJamMenuWidget* InOwner, EJamMenuRowKind InKind, const FString& InPayload)
{
	Owner = InOwner;
	Kind = InKind;
	Payload = InPayload;
	OnClicked.AddUniqueDynamic(this, &UJamMenuRowButton::HandleClicked);
}

void UJamMenuRowButton::HandleClicked()
{
	if (Owner) Owner->OnRowClicked(Kind, Payload);
}

// ---------------------------------------------------------------- menu

UJamMenuWidget* UJamMenuWidget::CreateAndAdd(APlayerController* PC)
{
	if (!PC) return nullptr;
	UJamMenuWidget* W = CreateWidget<UJamMenuWidget>(PC, UJamMenuWidget::StaticClass());
	if (W)
	{
		W->SetIsFocusable(true);
		W->AddToViewport(10);
		W->ShowScreen(EJamMenuScreen::Main);
	}
	return W;
}

UJamDesignerSubsystem* UJamMenuWidget::Designer() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UJamDesignerSubsystem>() : nullptr;
}

TSharedRef<SWidget> UJamMenuWidget::RebuildWidget()
{
	BuildTree();
	return Super::RebuildWidget();
}

void UJamMenuWidget::BuildTree()
{
	if (!WidgetTree) WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	if (WidgetTree->RootWidget) return;
	UWidgetTree* T = WidgetTree;

	// Black screen, the slate card centred.
	UOverlay* Root = T->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("MenuRoot"));
	T->RootWidget = Root;
	UBorder* Backdrop = Plate(T, Black(), FMargin(0));
	S::AddO(Root, Backdrop);

	USizeBox* CardBox = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	CardBox->SetWidthOverride(SlateWidth);
	CardSize = CardBox;
	S::AddO(Root, CardBox, HAlign_Center, VAlign_Center);

	// Lane A144: 30 units under the key line (18 put it on the card's chalk edge: struck through on every screen).
	UBorder* Card = S::Plate(T, S::Panel(1.f), FMargin(0, 0, 0, 30));
	CardBox->AddChild(Card);
	UVerticalBox* CardBody = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Card->SetContent(CardBody);
	AddV(CardBody, Clapper(T), FMargin(6, 6, 6, 0));

	// Slate header: production and scene lines like a real slate, then the title.
	UHorizontalBox* Head = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	AddH(Head, S::Text(T, TEXT("PROD.  HOKEJ"), S::CaptionFont(), S::Muted()), FMargin(28, 0, 0, 0), true);
	AddH(Head, S::Text(T, TEXT("SCENE  ROUGH"), S::CaptionFont(), S::Muted()), FMargin(0), true);
	AddH(Head, S::Text(T, TEXT("TAKE  A HIT"), S::CaptionFont(), S::Muted()), FMargin(0, 0, 28, 0));
	AddV(CardBody, Head, FMargin(0, 10, 0, 0));

	// The title: HOKEJ only (R-0013 point 5). The generated HOKEJ slate when it is there, the chalk letters otherwise.
	if (UTexture2D* TitleArt = Art(TEXT("T_UI_TitleHokej")))
	{
		TitlePicture = Picture(T, TitleArt, 480.f);   // lane A144: 560 before the first screen got its two lines and PLAY NOW
		AddV(CardBody, TitlePicture, FMargin(0, 6, 0, 4), HAlign_Center);
	}
	else
	{
		UTextBlock* Title = S::Text(T, TEXT("HOKEJ"), S::Font(104, FName("Black"), 80), S::Chalk());
		Title->SetJustification(ETextJustify::Center);
		TitlePicture = Title;
		AddV(CardBody, Title, FMargin(0, 2, 0, 6));
	}

	Switcher = T->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass(), TEXT("Screens"));
	AddV(CardBody, Switcher, FMargin(32, 6, 32, 0));
	Switcher->AddChild(BuildMain());
	Switcher->AddChild(BuildCuts());
	Switcher->AddChild(BuildDirect());
	Switcher->AddChild(BuildSettings());
	Switcher->AddChild(BuildCredits());
	Switcher->AddChild(BuildControls());
	Switcher->SetActiveWidgetIndex(0);

	UTextBlock* Keys = S::Text(T, TEXT("STICK / ARROWS  move      A / ENTER  select      B / ESC  back"), S::CaptionFont(), S::Muted());
	Keys->SetJustification(ETextJustify::Center);
	AddV(CardBody, Keys, FMargin(0, 14, 0, 0));
}

UWidget* UJamMenuWidget::BuildMain()
{
	UWidgetTree* T = WidgetTree;
	UVerticalBox* Box = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainScreen"));
	MainLabels.Reset();

	// Lane A144 (track U2): what the game is, in one line, and the brand's law under it.
	UTextBlock* What = Wrapped(S::Text(T, UJamMenuFlow::WhatItIsLine(), S::Font(S::Body + 2, FName("Medium")), S::Chalk()), BodyTextW);
	What->SetJustification(ETextJustify::Center);
	AddV(Box, What, FMargin(0, 0, 0, 1), HAlign_Center);
	UTextBlock* Brand = Wrapped(S::Text(T, UJamMenuFlow::BrandLine(), S::Font(S::Body, FName("Italic")), S::Yellow()), BodyTextW);
	Brand->SetJustification(ETextJustify::Center);
	AddV(Box, Brand, FMargin(0, 0, 0, 10), HAlign_Center);

	// PLAY NOW: the first press. A first match on the regular cut, no choices; the screen opens on it, so Enter / A starts it. Twice the label of
	// the other buttons, a yellow edge, and the words that say what to press.
	{
		PlayNowButton = T->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("PlayNowButton"));
		FButtonStyle Style = ButtonStyle();
		Style.SetNormal(S::Rounded(S::SlateDark(), S::Radius, S::Yellow(), 3.f));
		Style.SetHovered(S::Rounded(FLinearColor(S::Blue().R, S::Blue().G, S::Blue().B, 0.55f), S::Radius, S::Yellow(), 3.f));
		Style.SetPressed(S::Rounded(FLinearColor(S::Orange().R, S::Orange().G, S::Orange().B, 0.65f), S::Radius, S::Yellow(), 3.f));
		Style.SetNormalPadding(FMargin(22, 8));
		Style.SetPressedPadding(FMargin(22, 9, 22, 7));
		PlayNowButton->SetStyle(Style);
		UVerticalBox* Body = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		PlayNowText = S::Text(T, TEXT("PLAY NOW"), S::Font(S::Title + 4, FName("Black"), S::TrackTitle), S::Chalk());
		PlayNowText->SetJustification(ETextJustify::Center);
		AddV(Body, PlayNowText, FMargin(0), HAlign_Center);
		UTextBlock* Sub = S::Text(T, TEXT("ONE MATCH, THE REGULAR CUT.  ENTER / A, OR CLICK."), S::CaptionFont(), S::Muted());
		Sub->SetJustification(ETextJustify::Center);
		AddV(Body, Sub, FMargin(0, 0, 0, 2), HAlign_Center);
		PlayNowButton->AddChild(Body);
		PlayNowButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnPlayNowClicked);
		AddTinted(PlayNowButton, PlayNowText);
		MainLabels.Add(PlayNowText);
		AddV(Box, PlayNowButton, FMargin(130, 2, 130, 6));
	}

	UTextBlock* Txt = nullptr;
	auto Add = [&](TObjectPtr<UButton>& Target, const FString& LabelText)
	{
		UButton* B = Button(T, LabelText, S::Label, Txt);
		Target = B;
		AddTinted(B, Txt);
		MainLabels.Add(Txt);
		AddV(Box, B, FMargin(130, 3));
	};
	// Lane A17: the season of takes (docs/experiments/LADDER.md); the label says which take is next.
	Add(SeasonButton, TEXT("PLAY THE SEASON"));
	SeasonText = Txt;
	SeasonButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnSeasonClicked);
	Add(ChooseButton, TEXT("CHOOSE YOUR CUT"));
	ChooseButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnChooseClicked);
	Add(DirectButton, TEXT("DIRECT A CUT"));
	DirectButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnDirectClicked);
	// Lane A9's lobby: one entry that runs its console command (JamGame does not depend on JamNet).
	Add(FriendButton, TEXT("BRAWL WITH A FRIEND"));
	FriendButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnFriendClicked);

	// Lane A144: the four that are not a way to play share one row; CONTROLS is first of them, one press from the first screen.
	UHorizontalBox* Row = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("MainSmallRow"));
	auto AddSmall = [&](TObjectPtr<UButton>& Target, const FString& LabelText, bool bLast)
	{
		UButton* B = Button(T, LabelText, S::Body, Txt);
		FButtonStyle Style = B->GetStyle();
		Style.SetNormalPadding(FMargin(8, 9));
		Style.SetPressedPadding(FMargin(8, 10, 8, 8));
		B->SetStyle(Style);
		Target = B;
		AddTinted(B, Txt);
		MainLabels.Add(Txt);
		AddH(Row, B, FMargin(0, 0, bLast ? 0.f : 8.f, 0), true);
	};
	AddSmall(ControlsButton, TEXT("CONTROLS"), false);
	ControlsButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnControlsClicked);
	AddSmall(SettingsButton, TEXT("SETTINGS"), false);
	SettingsButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnSettingsClicked);
	AddSmall(CreditsButton, TEXT("CREDITS"), false);
	CreditsButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnCreditsClicked);
	AddSmall(QuitButton, TEXT("QUIT"), true);
	QuitButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnQuitClicked);
	AddV(Box, Row, FMargin(60, 6, 60, 0));

	MainStatus = Wrapped(S::Text(T, TEXT(""), S::BodyFont(), S::Muted()), BodyTextW);
	MainStatus->SetJustification(ETextJustify::Center);
	AddV(Box, MainStatus, FMargin(0, 8, 0, 0), HAlign_Center);
	return Box;
}

UWidget* UJamMenuWidget::BuildCuts()
{
	UWidgetTree* T = WidgetTree;
	UVerticalBox* Box = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CutsScreen"));
	// Lane A67 (R-0036): the Director's choices, modes chosen like a cut, above the cut list; lane A81: twelve in two rows of six.
	AddV(Box, S::Section(T, TEXT("DIRECTOR'S CHOICES: PICK A CARD, PLAY IT")), FMargin(0, 0, 0, 2));
	ModeGrid = T->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(), TEXT("ModeGrid"));
	ModeGrid->SetSlotPadding(FMargin(ModeSlotPad));
	AddV(Box, ModeGrid, FMargin(0, 0, 0, 2));
	AddV(Box, S::Section(T, TEXT("CHOOSE YOUR CUT")), FMargin(0, 0, 0, 2));
	// Lane A144: the cuts are compact cards, three a row (the film-strip rows were 184 units each, one visible, their words under the sprocket holes).
	CutList = T->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("CutList"));
	CutGrid = T->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(), TEXT("CutGrid"));
	CutGrid->SetSlotPadding(FMargin(ModeSlotPad));
	CutList->AddChild(CutGrid);
	USizeBox* ListBox = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	ListBox->SetMaxDesiredHeight(CutListH);   // two rows of cuts; a saved cut and played takes scroll
	ListBox->AddChild(CutList);
	AddV(Box, ListBox, FMargin(0, 0));
	CutsStatus = Wrapped(S::Text(T, TEXT(""), S::BodyFont(), S::Muted()), ModeCardWidth - 2.f * ScreenPadX);
	CutsStatus->SetVisibility(ESlateVisibility::Collapsed);   // an error line only
	AddV(Box, CutsStatus, FMargin(0, 6, 0, 0));
	UTextBlock* Txt = nullptr;
	UButton* Back = Button(T, TEXT("BACK"), S::Body, Txt);
	Back->OnClicked.AddDynamic(this, &UJamMenuWidget::OnBackClicked);
	AddTinted(Back, Txt);
	BackButtons.Add(Back);
	AddV(Box, Back, FMargin(0, 4, 0, 0), HAlign_Left);
	return Box;
}

UWidget* UJamMenuWidget::BuildDirect()
{
	UWidgetTree* T = WidgetTree;
	UVerticalBox* Box = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("DirectScreen"));
	AddV(Box, S::Section(T, TEXT("DIRECT A CUT")), FMargin(0, 0, 0, 4));
	UTextBlock* Intro = S::Text(T, TEXT("Tell the Designer in one sentence how you want this cut. It rewrites the rules, the ice, the hits, the look."), S::BodyFont(), S::Muted());
	Wrapped(Intro, BodyTextW);
	AddV(Box, Intro, FMargin(0, 0, 0, 8));

	UHorizontalBox* Row = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	SentenceBox = T->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("SentenceBox"));
	SentenceBox->SetHintText(FText::FromString(TEXT("make it faster and meaner")));
	SentenceBox->SetMinDesiredWidth(520.f);
	SentenceBox->WidgetStyle.SetFont(S::BodyFont());
	SentenceBox->WidgetStyle.SetPadding(FMargin(12, 8));
	DarkBox(SentenceBox);
	SentenceBox->OnTextCommitted.AddDynamic(this, &UJamMenuWidget::OnSentenceCommitted);
	AddH(Row, SentenceBox, FMargin(0, 0, 8, 0), true);
	UTextBlock* Txt = nullptr;
	RollButton = Button(T, TEXT("ROLL"), S::Body + 2, Txt);
	RollButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnRollClicked);
	AddTinted(RollButton, Txt);
	AddH(Row, RollButton, FMargin(0));
	AddV(Box, Row, FMargin(0, 0, 0, 4));

	DirectStatus = S::Text(T, TEXT(""), S::BodyFont(), S::Yellow());
	Wrapped(DirectStatus, BodyTextW);
	AddV(Box, DirectStatus, FMargin(0, 2, 0, 2));
	DirectHint = S::Text(T, TEXT(""), S::CaptionFont(), S::Muted());
	Wrapped(DirectHint, BodyTextW);
	AddV(Box, DirectHint, FMargin(0, 0, 0, 4));

	// The result: the cut's name, its three biggest changes, and what to do with it.
	ResultBox = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ResultBox"));
	UBorder* ResultPlate = S::Plate(T, S::Rounded(S::SlateDark(), S::Radius, S::Yellow(), 1.5f), FMargin(16, 12));
	ResultPlate->SetContent(ResultBox);
	ResultName = S::Text(T, TEXT(""), S::Font(S::Label + 4, FName("Black"), S::TrackLabel), S::Chalk());
	AddV(ResultBox, ResultName, FMargin(0, 0, 0, 2));
	ResultChanges = S::Text(T, TEXT(""), S::BodyFont(), S::Chalk());
	Wrapped(ResultChanges, BodyTextW - 32.f);
	AddV(ResultBox, ResultChanges, FMargin(0, 0, 0, 10));
	UHorizontalBox* Actions = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	PlayResultButton = Button(T, TEXT("PLAY"), S::Body, Txt);
	PlayResultButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnPlayResultClicked);
	AddTinted(PlayResultButton, Txt);
	AddH(Actions, PlayResultButton, FMargin(0, 0, 8, 0));
	UTextBlock* SaveTxt = nullptr;
	SaveResultButton = Button(T, TEXT("SAVE AS MY DIRECTOR'S CUT"), S::Body, SaveTxt);
	SaveResultText = SaveTxt;
	SaveResultButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnSaveResultClicked);
	AddTinted(SaveResultButton, SaveTxt);
	AddH(Actions, SaveResultButton, FMargin(0, 0, 8, 0));
	TryAgainButton = Button(T, TEXT("TRY AGAIN"), S::Body, Txt);
	TryAgainButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnTryAgainClicked);
	AddTinted(TryAgainButton, Txt);
	AddH(Actions, TryAgainButton, FMargin(0));
	AddV(ResultBox, Actions, FMargin(0));
	AddV(Box, ResultPlate, FMargin(0, 4, 0, 4));
	ResultPlate->SetVisibility(ESlateVisibility::Collapsed);
	ResultBox->SetVisibility(ESlateVisibility::Collapsed);

	AddV(Box, S::Text(T, TEXT("OR PICK A SENTENCE THE DESIGNER ALREADY SHOT"), S::CaptionFont(), S::Muted()), FMargin(0, 8, 0, 2));
	SentenceList = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SentenceList"));
	// Scrolls: the cache has a dozen sentences and the card must fit 720p.
	UScrollBox* SentenceScroll = T->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("SentenceScroll"));
	SentenceScroll->AddChild(SentenceList);
	USizeBox* SentenceClip = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	SentenceClip->SetMaxDesiredHeight(190.f);
	SentenceClip->AddChild(SentenceScroll);
	AddV(Box, SentenceClip, FMargin(0));

	UButton* Back = Button(T, TEXT("BACK"), S::Body, Txt);
	Back->OnClicked.AddDynamic(this, &UJamMenuWidget::OnBackClicked);
	AddTinted(Back, Txt);
	BackButtons.Add(Back);
	AddV(Box, Back, FMargin(0, 8, 0, 0), HAlign_Left);
	return Box;
}

UWidget* UJamMenuWidget::BuildSettings()
{
	UWidgetTree* T = WidgetTree;
	UVerticalBox* Box = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SettingsScreen"));
	AddV(Box, S::Section(T, TEXT("SETTINGS")), FMargin(0, 0, 0, 2));
	SettingsPanel = CreateWidget<UJamSettingsWidget>(this, UJamSettingsWidget::StaticClass());
	if (SettingsPanel) AddV(Box, SettingsPanel, FMargin(0));
	UTextBlock* Txt = nullptr;
	UButton* Back = Button(T, TEXT("BACK"), S::Body, Txt);
	Back->OnClicked.AddDynamic(this, &UJamMenuWidget::OnBackClicked);
	AddTinted(Back, Txt);
	BackButtons.Add(Back);
	AddV(Box, Back, FMargin(0, 8, 0, 0), HAlign_Left);
	return Box;
}

UWidget* UJamMenuWidget::BuildCredits()
{
	UWidgetTree* T = WidgetTree;
	UVerticalBox* Box = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CreditsScreen"));
	AddV(Box, S::Section(T, TEXT("CREDITS")), FMargin(0, 0, 0, 8));
	UTextBlock* Body = S::Text(T,
		TEXT("HOKEJ. Team Hokej, 2026 Epic MegaJam, theme Director's Cut.\n\n")
		TEXT("Built with Unreal Engine.\n\n")
		TEXT("One human directed two machines of agents; the agents wrote the game, the cuts and this menu. ")
		TEXT("The Designer turns your sentence into a cut through the Claude API with a key you type yourself; without a key it answers from cuts it shot earlier.\n\n")
		TEXT("Tools used: Unreal Engine 5.9, Claude Code and the Claude API, Codex image generation for the menu and HUD art, Node.js, Git with LFS, Meshy for generated figures, PowerShell.\n\n")
		TEXT("No goalies harmed. Skaters flattened. No logos."),
		S::BodyFont(), S::Chalk());
	Wrapped(Body, BodyTextW);
	AddV(Box, Body, FMargin(0, 0, 0, 6));
	UTextBlock* Txt = nullptr;
	UButton* Back = Button(T, TEXT("BACK"), S::Body, Txt);
	Back->OnClicked.AddDynamic(this, &UJamMenuWidget::OnBackClicked);
	AddTinted(Back, Txt);
	BackButtons.Add(Back);
	AddV(Box, Back, FMargin(0, 8, 0, 0), HAlign_Left);
	return Box;
}

// Lane A144 (track U2): the controls, one screen from the first: every verb with its mouse and keys, its pad button and one line on what it does
// (UJamMenuFlow::ControlRows; Jam.Menu.ControlsComplete holds it against UJamInputMap's bindings).
UWidget* UJamMenuWidget::BuildControls()
{
	UWidgetTree* T = WidgetTree;
	UVerticalBox* Box = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ControlsScreen"));
	AddV(Box, S::Section(T, TEXT("CONTROLS")), FMargin(0, 0, 0, 6));
	const float VerbW = 250.f, KeysW = 320.f, PadW = 190.f;
	const float NoteW = ControlsCardWidth - 2.f * ScreenPadX - VerbW - KeysW - PadW;   // 416
	auto Cell = [&](UHorizontalBox* Row, UTextBlock* Txt, float Width)
	{
		USizeBox* Size = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		Size->SetWidthOverride(Width);
		Size->AddChild(Wrapped(Txt, Width - 8.f));
		AddH(Row, Size, FMargin(0), false, VAlign_Top);
	};
	UHorizontalBox* Head = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	Cell(Head, S::Text(T, TEXT(""), S::CaptionFont(), S::Muted()), VerbW);
	Cell(Head, S::Text(T, TEXT("MOUSE AND KEYS"), S::CaptionFont(), S::Muted()), KeysW);
	Cell(Head, S::Text(T, TEXT("PAD"), S::CaptionFont(), S::Muted()), PadW);
	Cell(Head, S::Text(T, TEXT("WHAT IT DOES"), S::CaptionFont(), S::Muted()), NoteW);
	AddV(Box, Head, FMargin(0, 0, 0, 2));
	for (const FJamControlRow& R : UJamMenuFlow::ControlRows())
	{
		UHorizontalBox* Row = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		Cell(Row, S::Text(T, R.Verb, S::Font(S::Body, FName("Black")), S::Yellow()), VerbW);
		Cell(Row, S::Text(T, R.Keys, S::Font(S::Body, FName("Bold")), S::Chalk()), KeysW);
		Cell(Row, S::Text(T, R.Pad, S::Font(S::Body, FName("Medium")), S::Chalk()), PadW);
		Cell(Row, S::Text(T, R.Note, S::Font(S::Body - 2, FName("Regular")), S::Muted()), NoteW);
		AddV(Box, Row, FMargin(0, 3));
	}
	AddV(Box, Wrapped(S::Text(T, TEXT("In a match Esc shows this list again. The pad is mapped; no person has play-tested it yet."), S::CaptionFont(), S::Muted()),
		ControlsCardWidth - 2.f * ScreenPadX), FMargin(0, 8, 0, 0));
	UTextBlock* Txt = nullptr;
	UButton* Back = Button(T, TEXT("BACK"), S::Body, Txt);
	Back->OnClicked.AddDynamic(this, &UJamMenuWidget::OnBackClicked);
	AddTinted(Back, Txt);
	BackButtons.Add(Back);
	AddV(Box, Back, FMargin(0, 8, 0, 0), HAlign_Left);
	return Box;
}

// ---------------------------------------------------------------- screens

void UJamMenuWidget::ShowScreen(EJamMenuScreen InScreen)
{
	Screen = InScreen;
	if (!Switcher) return;
	Switcher->SetActiveWidgetIndex(static_cast<int32>(Screen));
	// The title heads the main and credits screens; the tall screens (cuts, direct, settings, controls) keep the card inside 720p.
	if (TitlePicture) TitlePicture->SetVisibility(Screen == EJamMenuScreen::Main || Screen == EJamMenuScreen::Credits ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	// Lane A81: the Director's choices need a wider card than the other screens' 880 (1240 of 1920 slate units, also at 720p); lane A144: the controls too.
	if (CardSize) CardSize->SetWidthOverride(Screen == EJamMenuScreen::Cuts ? ModeCardWidth : (Screen == EJamMenuScreen::Controls ? ControlsCardWidth : SlateWidth));
	switch (Screen)
	{
	case EJamMenuScreen::Main:
		RefreshSeason();
		// Lane A144: the first screen opens on PLAY NOW, so the first Enter / A is a match.
		PendingFocus = PlayNowButton ? PlayNowButton.Get() : (SeasonButton ? SeasonButton.Get() : ChooseButton.Get());
		break;
	case EJamMenuScreen::Cuts:
		if (CutsStatus) { CutsStatus->SetText(FText::GetEmpty()); CutsStatus->SetVisibility(ESlateVisibility::Collapsed); }
		RefreshCuts();
		RefreshModes();
		if (ModeGrid && ModeGrid->GetChildrenCount()) { PendingFocus = ModeGrid->GetChildAt(0); break; }
		PendingFocus = CutGrid && CutGrid->GetChildrenCount() ? CutGrid->GetChildAt(0) : (BackButtons.Num() ? static_cast<UWidget*>(BackButtons[0].Get()) : nullptr);
		break;
	case EJamMenuScreen::Direct:
		RefreshDirect();
		PendingFocus = RollButton.Get();
		break;
	case EJamMenuScreen::Settings:
		if (SettingsPanel) { SettingsPanel->Refresh(); PendingFocus = SettingsPanel->GetFirstFocusable(); }
		break;
	case EJamMenuScreen::Credits:
		PendingFocus = BackButtons.Num() >= 4 ? BackButtons[3].Get() : nullptr;
		break;
	case EJamMenuScreen::Controls:
		PendingFocus = BackButtons.Num() >= 5 ? BackButtons[4].Get() : nullptr;
		break;
	}
}

void UJamMenuWidget::RefreshCuts()
{
	if (!CutGrid) return;
	CutGrid->ClearChildren();
	Tinted.RemoveAll([](const TPair<TWeakObjectPtr<UButton>, TWeakObjectPtr<UTextBlock>>& P) { return !P.Key.IsValid() || P.Key->IsA<UJamMenuRowButton>(); });
	UJamDesignerSubsystem* D = Designer();
	Rows = D ? UJamMenuFlow::BuildCutRows(ListCutsWithSeason()) : TArray<FJamMenuCutRow>();
	UWidgetTree* T = WidgetTree;
	for (int32 i = 0; i < Rows.Num(); ++i)
	{
		const FJamMenuCutRow& R = Rows[i];
		UJamMenuRowButton* B = T->ConstructWidget<UJamMenuRowButton>(UJamMenuRowButton::StaticClass());
		// Lane A144: a cut is a compact card (the chalk edge sets it apart from a Director's choice's blue): its icon, its name, one line.
		FButtonStyle CardStyle = ButtonStyle();
		CardStyle.SetNormal(S::Rounded(S::SlateDark(), S::Radius, FLinearColor(S::Paper().R, S::Paper().G, S::Paper().B, 0.55f), 1.5f));
		CardStyle.SetHovered(S::Rounded(FLinearColor(S::Blue().R, S::Blue().G, S::Blue().B, 0.45f), S::Radius, S::Yellow(), 2.f));
		CardStyle.SetPressed(S::Rounded(FLinearColor(S::Orange().R, S::Orange().G, S::Orange().B, 0.55f), S::Radius, S::Yellow(), 2.f));
		CardStyle.SetNormalPadding(FMargin(CutPadX, 7));
		CardStyle.SetPressedPadding(FMargin(CutPadX, 8, CutPadX, 6));
		B->SetStyle(CardStyle);
		B->Bind(this, EJamMenuRowKind::Cut, R.Name);

		UHorizontalBox* Line = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		AddH(Line, S::Icon(T, IconFor(R), CutIcon), FMargin(0, 2, 10, 0), false, VAlign_Top);
		UVerticalBox* Body = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		UTextBlock* Name = S::Text(T, R.Display.ToUpper(), CutNameFont(), S::Chalk());
		B->TintText = Name;
		AddV(Body, Name, FMargin(0, 0, 0, 1));
		AddV(Body, Wrapped(S::Text(T, UJamMenuFlow::CutCardLine(R), CutLineFont(), S::Chalk()), CutTextW), FMargin(0));
		// The name and two lines: every card of the grid is this high, so two rows of cuts fit the list whole (a long saved sentence ends at the box).
		USizeBox* BodyBox = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		BodyBox->SetHeightOverride(CutBodyH);
		BodyBox->SetClipping(EWidgetClipping::ClipToBounds);
		BodyBox->AddChild(Body);
		if (USizeBoxSlot* BS2 = Cast<USizeBoxSlot>(Body->Slot)) { BS2->SetHorizontalAlignment(HAlign_Fill); BS2->SetVerticalAlignment(VAlign_Top); }
		AddH(Line, BodyBox, FMargin(0), true, VAlign_Top);
		B->AddChild(Line);
		if (UButtonSlot* BS = Cast<UButtonSlot>(Line->Slot)) { BS->SetHorizontalAlignment(HAlign_Fill); BS->SetVerticalAlignment(VAlign_Top); }
		CutGrid->AddChildToUniformGrid(B, i / CutsPerRow, i % CutsPerRow);
		if (UUniformGridSlot* GS = Cast<UUniformGridSlot>(B->Slot)) { GS->SetHorizontalAlignment(HAlign_Fill); GS->SetVerticalAlignment(VAlign_Fill); }
		AddTinted(B, Name);
	}
	if (!Rows.Num() && CutsStatus)
	{
		CutsStatus->SetText(FText::FromString(TEXT("No cuts found in Content/Cartridges.")));
		CutsStatus->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UJamMenuWidget::RefreshModes()
{
	if (!ModeGrid) return;
	ModeGrid->ClearChildren();
	Tinted.RemoveAll([](const TPair<TWeakObjectPtr<UButton>, TWeakObjectPtr<UTextBlock>>& P) { return !P.Key.IsValid(); });
	UWidgetTree* T = WidgetTree;
	const TArray<FJamModeCard> Cards = UJamMenuFlow::AvailableModeCards();
	for (int32 i = 0; i < Cards.Num(); ++i)
	{
		const FJamModeCard& M = Cards[i];
		UJamMenuRowButton* B = T->ConstructWidget<UJamMenuRowButton>(UJamMenuRowButton::StaticClass());
		FButtonStyle CardStyle = ButtonStyle();
		CardStyle.SetNormal(S::Rounded(S::SlateDark(), S::Radius, S::Blue(), 1.5f));
		CardStyle.SetHovered(S::Rounded(FLinearColor(S::Blue().R, S::Blue().G, S::Blue().B, 0.45f), S::Radius, S::Yellow(), 2.f));
		CardStyle.SetPressed(S::Rounded(FLinearColor(S::Orange().R, S::Orange().G, S::Orange().B, 0.55f), S::Radius, S::Yellow(), 2.f));
		CardStyle.SetNormalPadding(FMargin(ModePadX, 5));
		CardStyle.SetPressedPadding(FMargin(ModePadX, 6, ModePadX, 4));
		B->SetStyle(CardStyle);
		// Chosen like a cut: the payload is the cut name ChooseCut loads ("modes/hockey.mode.pond").
		B->Bind(this, EJamMenuRowKind::Cut, M.Name);

		// Lane A144: the name, one sentence, what is different: the name at 22 (it reads at 720p and fits a quarter of the card), the pitch in chalk
		// italics, the facts one to a line. Every text wraps at the card's own width, so nothing leaves the card.
		UVerticalBox* Body = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		UTextBlock* Name = S::Text(T, M.Display.ToUpper(), ModeNameFont(), S::Chalk());
		B->TintText = Name;
		AddV(Body, Name, FMargin(0, 0, 0, 1));
		AddV(Body, Wrapped(S::Text(T, M.Pitch, ModePitchFont(), S::Chalk()), ModeTextW), FMargin(0, 0, 0, 4));
		for (const FString& F : M.Facts)
		{
			AddV(Body, Wrapped(S::Text(T, FString::Printf(TEXT("- %s"), *F), ModeFactFont(), S::Muted()), ModeTextW), FMargin(0));
		}
		B->AddChild(Body);
		// Lane A81: the words start at the top of every card, so the names line up along the row.
		if (UButtonSlot* BS = Cast<UButtonSlot>(Body->Slot)) { BS->SetHorizontalAlignment(HAlign_Fill); BS->SetVerticalAlignment(VAlign_Top); }
		ModeGrid->AddChildToUniformGrid(B, i / ModesPerRow, i % ModesPerRow);
		if (UUniformGridSlot* GS = Cast<UUniformGridSlot>(B->Slot)) { GS->SetHorizontalAlignment(HAlign_Fill); GS->SetVerticalAlignment(VAlign_Fill); }
		AddTinted(B, Name);
	}
	ModeGrid->SetVisibility(Cards.Num() ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}

// ---------------------------------------------------------------- the measures the tests read (lane A144)

float UJamMenuWidget::ModeTextWidth() { return ModeTextW; }
float UJamMenuWidget::CutTextWidth() { return CutTextW; }
float UJamMenuWidget::CutListHeight() { return CutListH; }
FSlateFontInfo UJamMenuWidget::ModeNameFont() { return S::Font(S::Body + 4, FName("Black")); }
FSlateFontInfo UJamMenuWidget::ModePitchFont() { return S::Font(S::Body - 2, FName("Italic")); }
FSlateFontInfo UJamMenuWidget::ModeFactFont() { return S::Font(S::Body - 3, FName("Medium")); }
FSlateFontInfo UJamMenuWidget::CutNameFont() { return S::Font(S::Body + 2, FName("Black")); }
FSlateFontInfo UJamMenuWidget::CutLineFont() { return S::Font(S::Body - 3, FName("Regular")); }

TArray<FString> UJamMenuWidget::GetMainLabels() const
{
	TArray<FString> Out;
	for (const TObjectPtr<UTextBlock>& L : MainLabels) if (L) Out.Add(L->GetText().ToString());
	return Out;
}

TArray<UJamMenuRowButton*> UJamMenuWidget::GetModeButtons() const
{
	TArray<UJamMenuRowButton*> Out;
	if (ModeGrid) for (UWidget* W : ModeGrid->GetAllChildren()) if (UJamMenuRowButton* B = Cast<UJamMenuRowButton>(W)) Out.Add(B);
	return Out;
}

TArray<UJamMenuRowButton*> UJamMenuWidget::GetCutButtons() const
{
	TArray<UJamMenuRowButton*> Out;
	if (CutGrid) for (UWidget* W : CutGrid->GetAllChildren()) if (UJamMenuRowButton* B = Cast<UJamMenuRowButton>(W)) Out.Add(B);
	return Out;
}

FVector2D UJamMenuWidget::MeasureScreen(EJamMenuScreen InScreen)
{
	TSharedRef<SWidget> W = TakeWidget();
	ShowScreen(InScreen);
	W->SlatePrepass(1.f);
	return FVector2D(W->GetDesiredSize());
}

void UJamMenuWidget::RefreshDirect()
{
	UJamDesignerSubsystem* D = Designer();
	const bool bLive = D && D->IsLiveAvailable();
	DirectHint->SetText(FText::FromString(bLive
		? TEXT("Live: your sentence goes to the Designer with your key. A roll takes a few seconds.")
		: TEXT("No API key: the Designer answers from the cuts it already shot (nearest sentence). Add a key under SETTINGS to direct live.")));
	if (!bRolling && !bHasResult) DirectStatus->SetText(FText::GetEmpty());
	// Cached sentences as buttons (the gamepad's way to type).
	if (SentenceList)
	{
		SentenceList->ClearChildren();
		Tinted.RemoveAll([](const TPair<TWeakObjectPtr<UButton>, TWeakObjectPtr<UTextBlock>>& P)
		{
			return !P.Key.IsValid() || (P.Key->IsA<UJamMenuRowButton>() && Cast<UJamMenuRowButton>(P.Key.Get())->Kind == EJamMenuRowKind::Sentence);
		});
		if (D)
		{
			UWidgetTree* T = WidgetTree;
			TArray<FString> Sentences = D->GetCachedSentences();
			for (const FJamCutInfo& I : D->ListCuts()) if (I.bSaved && !I.Sentence.IsEmpty()) Sentences.AddUnique(I.Sentence);
			for (const FString& Sentence : Sentences)
			{
				UJamMenuRowButton* B = T->ConstructWidget<UJamMenuRowButton>(UJamMenuRowButton::StaticClass());
				FButtonStyle RowStyle = ButtonStyle();
				RowStyle.SetNormalPadding(FMargin(14, 5));
				RowStyle.SetPressedPadding(FMargin(14, 6, 14, 4));
				B->SetStyle(RowStyle);
				B->Bind(this, EJamMenuRowKind::Sentence, Sentence);
				UTextBlock* Txt = S::Text(T, FString::Printf(TEXT("\"%s\""), *Sentence), S::Font(S::Body, FName("Italic")), S::Chalk());
				B->TintText = Txt;
				B->AddChild(Txt);
				AddV(SentenceList, B, FMargin(0, 2), HAlign_Left);
				AddTinted(B, Txt);
			}
		}
	}
}

// ---------------------------------------------------------------- actions

bool UJamMenuWidget::ChooseCut(const FString& Name)
{
	UJamDesignerSubsystem* D = Designer();
	if (!D) { if (CutsStatus) { CutsStatus->SetText(FText::FromString(TEXT("The Designer subsystem is missing."))); CutsStatus->SetVisibility(ESlateVisibility::HitTestInvisible); } return false; }
	if (!Rows.Num()) Rows = UJamMenuFlow::BuildCutRows(ListCutsWithSeason());
	const FJamMenuCutRow* Row = Rows.FindByPredicate([&](const FJamMenuCutRow& R) { return R.Name == Name; });
	FString Err;
	if (!D->LoadCut(Row ? (Row->bSaved ? Row->Path : Row->Name) : Name, Err))
	{
		if (CutsStatus) { CutsStatus->SetText(FText::FromString(FString::Printf(TEXT("%s was rejected: %s"), *Name, *Err))); CutsStatus->SetVisibility(ESlateVisibility::HitTestInvisible); }
		UE_LOG(LogJamMenuUi, Warning, TEXT("JAMMENU cut %s rejected: %s"), *Name, *Err);
		return false;
	}
	UJamMenuFlow::StartMatch(this, Row && Row->bSaved ? Row->Path : Name);
	return true;
}

void UJamMenuWidget::OnRowClicked(EJamMenuRowKind Kind, const FString& Payload)
{
	if (Kind == EJamMenuRowKind::Cut) ChooseCut(Payload);
	else
	{
		if (SentenceBox) SentenceBox->SetText(FText::FromString(Payload));
		Direct(Payload);
	}
}

void UJamMenuWidget::Direct(const FString& Sentence)
{
	UJamDesignerSubsystem* D = Designer();
	const FString Trimmed = Sentence.TrimStartAndEnd();
	if (!D || Trimmed.IsEmpty()) { DirectStatus->SetText(FText::FromString(TEXT("Type one sentence first."))); return; }
	if (bRolling) return;
	bRolling = true;
	bHasResult = false;
	bResultSaved = false;
	SavedResultPath.Empty();
	if (ResultBox) { ResultBox->SetVisibility(ESlateVisibility::Collapsed); if (UWidget* P = ResultBox->GetParent()) P->SetVisibility(ESlateVisibility::Collapsed); }
	DirectStatus->SetText(FText::FromString(TEXT("rolling...")));
	RollButton->SetIsEnabled(false);
	TWeakObjectPtr<UJamMenuWidget> Weak(this);
	D->RequestCut(Trimmed, FString(), [Weak](const FJamDesignerResult& R) { if (Weak.IsValid()) Weak->OnResult(R); });
}

void UJamMenuWidget::OnResult(const FJamDesignerResult& R)
{
	bRolling = false;
	Last = R;
	if (RollButton) RollButton->SetIsEnabled(true);
	bHasResult = R.HasCartridge() && !R.CartridgeJson.IsEmpty();
	FString Status = R.Message;
	switch (R.Outcome)
	{
	case EJamDesignerOutcome::FirstTry: Status = FString::Printf(TEXT("Cut. %lld in / %lld out tokens, %.1f s."), R.TokensIn, R.TokensOut, R.Ms / 1000.0); break;
	case EJamDesignerOutcome::Repaired: Status = FString::Printf(TEXT("Cut after one repair. %lld in / %lld out tokens, %.1f s."), R.TokensIn, R.TokensOut, R.Ms / 1000.0); break;
	case EJamDesignerOutcome::Offline: Status = TEXT("From the shot cuts (no key): the nearest sentence."); break;
	case EJamDesignerOutcome::Refused: Status = FString::Printf(TEXT("The Designer declined: %s"), *R.Refusal); break;
	case EJamDesignerOutcome::Failed: Status = R.Errors.Num() ? FString::Printf(TEXT("No valid cut: %s"), *R.Errors[0]) : (R.Message.IsEmpty() ? TEXT("No valid cut.") : R.Message); break;
	default: break;
	}
	DirectStatus->SetText(FText::FromString(Status));
	if (bHasResult)
	{
		ResultName->SetText(FText::FromString(R.CutName.IsEmpty() ? TEXT("Untitled Cut") : R.CutName.ToUpper()));
		const TArray<FString> Changes = UJamMenuFlow::BiggestChanges(UJamMenuFlow::BaseCartridgeJson(Designer()), R.CartridgeJson, 3);
		ResultChanges->SetText(FText::FromString(Changes.Num() ? FString::Join(Changes, TEXT("\n")) : TEXT("no change from the current cut")));
		SaveResultText->SetText(FText::FromString(TEXT("SAVE AS MY DIRECTOR'S CUT")));
		SaveResultButton->SetIsEnabled(true);
		ResultBox->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (UWidget* P = ResultBox->GetParent()) P->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		PendingFocus = PlayResultButton.Get();
	}
	else
	{
		PendingFocus = RollButton.Get();
	}
}

void UJamMenuWidget::OnPlayResultClicked()
{
	if (!bHasResult) return;
	FString Path = bResultSaved ? SavedResultPath : UJamMenuFlow::WritePreviewCut(Last.CartridgeJson);
	if (Path.IsEmpty()) { DirectStatus->SetText(FText::FromString(TEXT("Could not write the cut to play it."))); return; }
	if (UJamDesignerSubsystem* D = Designer())
	{
		FString Err;
		if (!D->LoadCut(Path, Err)) { DirectStatus->SetText(FText::FromString(FString::Printf(TEXT("The cut was rejected: %s"), *Err))); return; }
	}
	UJamMenuFlow::StartMatch(this, Path);
}

void UJamMenuWidget::OnSaveResultClicked()
{
	UJamDesignerSubsystem* D = Designer();
	if (!bHasResult || !D) return;
	FString Err;
	if (!D->ApplyResult(Last, Err)) { DirectStatus->SetText(FText::FromString(FString::Printf(TEXT("Not saved: %s"), *Err))); return; }
	bResultSaved = true;
	for (const FJamCutInfo& I : D->ListCuts()) if (I.bSaved && I.Name == UJamDesignerSubsystem::MyDirectorsCutName) SavedResultPath = I.Path;
	DirectStatus->SetText(FText::FromString(TEXT("Saved as My Director's Cut. It is in Choose your cut now.")));
	SaveResultText->SetText(FText::FromString(TEXT("SAVED")));
	SaveResultButton->SetIsEnabled(false);
	PendingFocus = PlayResultButton.Get();
}

void UJamMenuWidget::OnTryAgainClicked()
{
	bHasResult = false;
	ResultBox->SetVisibility(ESlateVisibility::Collapsed);
	if (UWidget* P = ResultBox->GetParent()) P->SetVisibility(ESlateVisibility::Collapsed);
	DirectStatus->SetText(FText::GetEmpty());
	if (SentenceBox) { SentenceBox->SetText(FText::GetEmpty()); PendingFocus = SentenceBox.Get(); }
}

void UJamMenuWidget::OnChooseClicked() { ShowScreen(EJamMenuScreen::Cuts); }

// Lane A144 (track U2): the first press. One match on the regular cut at the settings' difficulty; a rejected cut says so on the first screen.
bool UJamMenuWidget::PlayNow()
{
	if (ChooseCut(UJamMenuFlow::FirstCut)) return true;
	if (MainStatus) MainStatus->SetText(FText::FromString(FString::Printf(TEXT("The first match cannot start: %s"), CutsStatus ? *CutsStatus->GetText().ToString() : TEXT("no cut"))));
	return false;
}

void UJamMenuWidget::OnPlayNowClicked() { PlayNow(); }
void UJamMenuWidget::OnControlsClicked() { ShowScreen(EJamMenuScreen::Controls); }

TArray<FJamCutInfo> UJamMenuWidget::ListCutsWithSeason() const
{
	UJamDesignerSubsystem* D = Designer();
	TArray<FJamCutInfo> Cuts = D ? D->ListCuts() : TArray<FJamCutInfo>();
	// Lane A17: a played season take is listed as a cut of its own.
	if (const UJamProgression* P = UJamProgression::Get(this)) Cuts = P->AppendPlayedTakes(Cuts);
	return Cuts;
}

void UJamMenuWidget::RefreshSeason()
{
	if (!SeasonText) return;
	UJamProgression* P = UJamProgression::Get(this);
	const FJamSeasonState St = P ? P->GetSeasonState() : FJamSeasonState();
	FString LabelText = TEXT("PLAY THE SEASON");
	if (St.bLoaded && St.bComplete) LabelText += TEXT(": WRAPPED");
	else if (St.bLoaded) LabelText += FString::Printf(TEXT(": TAKE %d OF %d"), St.CurrentTake + 1, St.TakeCount);
	SeasonText->SetText(FText::FromString(LabelText));
	if (SeasonButton) SeasonButton->SetIsEnabled(St.bLoaded);
	if (MainStatus && St.bLoaded) MainStatus->SetText(FText::FromString(St.MenuLine));
}

bool UJamMenuWidget::PlaySeason()
{
	UJamProgression* P = UJamProgression::Get(this);
	if (!P || !P->StartSeason(this))
	{
		if (MainStatus) MainStatus->SetText(FText::FromString(P ? FString::Printf(TEXT("The season cannot start: %s"), *P->GetSeasonState().Error) : FString(TEXT("The season subsystem is missing."))));
		return false;
	}
	return true;
}

void UJamMenuWidget::OnSeasonClicked() { PlaySeason(); }
void UJamMenuWidget::OnDirectClicked() { ShowScreen(EJamMenuScreen::Direct); }

void UJamMenuWidget::OnFriendClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->ConsoleCommand(TEXT("jam.lobby"));
		if (MainStatus) MainStatus->SetText(FText::FromString(TEXT("Opening the lobby...")));
	}
}
void UJamMenuWidget::OnSettingsClicked() { ShowScreen(EJamMenuScreen::Settings); }
void UJamMenuWidget::OnCreditsClicked() { ShowScreen(EJamMenuScreen::Credits); }
void UJamMenuWidget::OnBackClicked() { ShowScreen(EJamMenuScreen::Main); }
void UJamMenuWidget::OnRollClicked() { if (SentenceBox) Direct(SentenceBox->GetText().ToString()); }
void UJamMenuWidget::OnSentenceCommitted(const FText& InText, ETextCommit::Type Commit) { if (Commit == ETextCommit::OnEnter) Direct(InText.ToString()); }

void UJamMenuWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

// ---------------------------------------------------------------- input and tick

void UJamMenuWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (PendingFocus.IsValid())
	{
		PendingFocus->SetFocus();
		PendingFocus = nullptr;
	}
	for (const TPair<TWeakObjectPtr<UButton>, TWeakObjectPtr<UTextBlock>>& P : Tinted)
	{
		if (P.Key.IsValid() && P.Value.IsValid()) Tint(P.Key.Get(), P.Value.Get());
	}
}

FReply UJamMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (IsBackKey(InKeyEvent.GetKey()) && Screen != EJamMenuScreen::Main)
	{
		ShowScreen(EJamMenuScreen::Main);
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
