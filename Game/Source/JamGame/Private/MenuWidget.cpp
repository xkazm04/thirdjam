#include "MenuWidget.h"
#include "MenuStyle.h"
#include "SettingsWidget.h"
#include "JamCartridge.h"
#include "Progression.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
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
// Director's Call. Cut rows sit in the film-strip frame of the kit with their icon; type and plates from FJamUiStyle. Logic is lane A8's
// and A17's.
namespace
{
	// Lane A81: the Director's choices row: six cards a row (two rows hold twelve) on a slate card this wide on the cuts screen.
	constexpr int32 ModesPerRow = 6;
	constexpr float ModeCardWidth = 1240.f;

	// Which painted card a cut row shows (R-0008 art).
	FString CardTextureFor(const FJamMenuCutRow& R)
	{
		if (R.bSaved) return TEXT("T_UI_Card_DirectorsCut");
		if (R.Name == TEXT("hockey.regular")) return TEXT("T_UI_Card_Regular");
		if (R.Name == TEXT("hockey.raw")) return TEXT("T_UI_Card_Raw");
		if (R.Name == TEXT("hockey.boost")) return TEXT("T_UI_Card_Boost");
		if (R.Name == TEXT("hockey.arena.large")) return TEXT("T_UI_Card_LargeRink");
		if (R.Name == TEXT("hockey.arena.wide")) return TEXT("T_UI_Card_Wide");
		return TEXT("T_UI_Card_Regular");
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
	CardBox->SetWidthOverride(880.f);
	CardSize = CardBox;
	S::AddO(Root, CardBox, HAlign_Center, VAlign_Center);

	UBorder* Card = S::Plate(T, S::Panel(1.f), FMargin(0, 0, 0, 18));
	CardBox->AddChild(Card);
	UVerticalBox* CardBody = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Card->SetContent(CardBody);
	AddV(CardBody, Clapper(T), FMargin(6, 6, 6, 0));

	// Slate header: production and scene lines like a real slate, then the title.
	UHorizontalBox* Head = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	AddH(Head, S::Text(T, TEXT("PROD.  HOKEJ"), S::CaptionFont(), S::Muted()), FMargin(28, 0, 0, 0), true);
	AddH(Head, S::Text(T, TEXT("SCENE  3 v 3"), S::CaptionFont(), S::Muted()), FMargin(0), true);
	AddH(Head, S::Text(T, TEXT("TAKE  YOURS"), S::CaptionFont(), S::Muted()), FMargin(0, 0, 28, 0));
	AddV(CardBody, Head, FMargin(0, 10, 0, 0));

	// The title: HOKEJ only (R-0013 point 5). The generated HOKEJ slate when it is there, the chalk letters otherwise.
	if (UTexture2D* TitleArt = Art(TEXT("T_UI_TitleHokej")))
	{
		TitlePicture = Picture(T, TitleArt, 560.f);
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
	Switcher->SetActiveWidgetIndex(0);

	UTextBlock* Keys = S::Text(T, TEXT("STICK / ARROWS  move      A / ENTER  select      B / ESC  back"), S::CaptionFont(), S::Muted());
	Keys->SetJustification(ETextJustify::Center);
	AddV(CardBody, Keys, FMargin(0, 14, 0, 0));
}

UWidget* UJamMenuWidget::BuildMain()
{
	UWidgetTree* T = WidgetTree;
	UVerticalBox* Box = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainScreen"));
	UTextBlock* Txt = nullptr;
	auto Add = [&](TObjectPtr<UButton>& Target, const FString& LabelText)
	{
		UButton* B = Button(T, LabelText, S::Label, Txt);
		Target = B;
		AddTinted(B, Txt);
		AddV(Box, B, FMargin(130, 4));
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
	Add(FriendButton, TEXT("PLAY WITH A FRIEND"));
	FriendButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnFriendClicked);
	Add(SettingsButton, TEXT("SETTINGS"));
	SettingsButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnSettingsClicked);
	Add(CreditsButton, TEXT("CREDITS"));
	CreditsButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnCreditsClicked);
	Add(QuitButton, TEXT("QUIT"));
	QuitButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnQuitClicked);
	MainStatus = S::Text(T, TEXT(""), S::BodyFont(), S::Muted());
	MainStatus->SetJustification(ETextJustify::Center);
	AddV(Box, MainStatus, FMargin(0, 8, 0, 0));
	return Box;
}

UWidget* UJamMenuWidget::BuildCuts()
{
	UWidgetTree* T = WidgetTree;
	UVerticalBox* Box = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CutsScreen"));
	// Lane A67 (R-0036): the Director's choices, modes chosen like a cut, above the cut list; lane A81: twelve in two rows of six.
	AddV(Box, S::Section(T, TEXT("DIRECTOR'S CHOICES")), FMargin(0, 0, 0, 4));
	ModeGrid = T->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(), TEXT("ModeGrid"));
	ModeGrid->SetSlotPadding(FMargin(4.f));
	AddV(Box, ModeGrid, FMargin(0, 0, 0, 8));
	AddV(Box, S::Section(T, TEXT("CHOOSE YOUR CUT")), FMargin(0, 0, 0, 6));
	CutList = T->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("CutList"));
	USizeBox* ListBox = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	ListBox->SetMaxDesiredHeight(250.f);   // lane A67: the mode cards take the rest of the 720p card
	ListBox->AddChild(CutList);
	AddV(Box, ListBox, FMargin(0, 0));
	CutsStatus = S::Text(T, TEXT(""), S::BodyFont(), S::Muted());
	AddV(Box, CutsStatus, FMargin(0, 6, 0, 0));
	UTextBlock* Txt = nullptr;
	UButton* Back = Button(T, TEXT("BACK"), S::Body, Txt);
	Back->OnClicked.AddDynamic(this, &UJamMenuWidget::OnBackClicked);
	AddTinted(Back, Txt);
	BackButtons.Add(Back);
	AddV(Box, Back, FMargin(0, 8, 0, 0), HAlign_Left);
	return Box;
}

UWidget* UJamMenuWidget::BuildDirect()
{
	UWidgetTree* T = WidgetTree;
	UVerticalBox* Box = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("DirectScreen"));
	AddV(Box, S::Section(T, TEXT("DIRECT A CUT")), FMargin(0, 0, 0, 4));
	UTextBlock* Intro = S::Text(T, TEXT("Tell the Designer in one sentence how you want this cut. It edits the rules, the ice, the shots, the look."), S::BodyFont(), S::Muted());
	Intro->SetAutoWrapText(true);
	AddV(Box, Intro, FMargin(0, 0, 0, 8));

	UHorizontalBox* Row = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	SentenceBox = T->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("SentenceBox"));
	SentenceBox->SetHintText(FText::FromString(TEXT("make it faster and meaner")));
	SentenceBox->SetMinDesiredWidth(520.f);
	SentenceBox->WidgetStyle.SetFont(S::BodyFont());
	SentenceBox->WidgetStyle.SetPadding(FMargin(12, 8));
	SentenceBox->OnTextCommitted.AddDynamic(this, &UJamMenuWidget::OnSentenceCommitted);
	AddH(Row, SentenceBox, FMargin(0, 0, 8, 0), true);
	UTextBlock* Txt = nullptr;
	RollButton = Button(T, TEXT("ROLL"), S::Body + 2, Txt);
	RollButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnRollClicked);
	AddTinted(RollButton, Txt);
	AddH(Row, RollButton, FMargin(0));
	AddV(Box, Row, FMargin(0, 0, 0, 4));

	DirectStatus = S::Text(T, TEXT(""), S::BodyFont(), S::Yellow());
	DirectStatus->SetAutoWrapText(true);
	AddV(Box, DirectStatus, FMargin(0, 2, 0, 2));
	DirectHint = S::Text(T, TEXT(""), S::CaptionFont(), S::Muted());
	DirectHint->SetAutoWrapText(true);
	AddV(Box, DirectHint, FMargin(0, 0, 0, 4));

	// The result: the cut's name, its three biggest changes, and what to do with it.
	ResultBox = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ResultBox"));
	UBorder* ResultPlate = S::Plate(T, S::Rounded(S::SlateDark(), S::Radius, S::Yellow(), 1.5f), FMargin(16, 12));
	ResultPlate->SetContent(ResultBox);
	ResultName = S::Text(T, TEXT(""), S::Font(S::Label + 4, FName("Black"), S::TrackLabel), S::Chalk());
	AddV(ResultBox, ResultName, FMargin(0, 0, 0, 2));
	ResultChanges = S::Text(T, TEXT(""), S::BodyFont(), S::Chalk());
	ResultChanges->SetAutoWrapText(true);
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
		TEXT("No goalies were harmed. No logos were used."),
		S::BodyFont(), S::Chalk());
	Body->SetAutoWrapText(true);
	AddV(Box, Body, FMargin(0, 0, 0, 6));
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
	// The title heads the main and credits screens; the tall screens (cuts, direct, settings) keep the card inside 720p.
	if (TitlePicture) TitlePicture->SetVisibility(Screen == EJamMenuScreen::Main || Screen == EJamMenuScreen::Credits ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	// Lane A81: twelve Director's choices in two rows of six need a wider card than the other screens' 880 (1240 of 1920 slate units, also at 720p).
	if (CardSize) CardSize->SetWidthOverride(Screen == EJamMenuScreen::Cuts ? ModeCardWidth : 880.f);
	switch (Screen)
	{
	case EJamMenuScreen::Main:
		RefreshSeason();
		PendingFocus = SeasonButton ? SeasonButton.Get() : ChooseButton.Get();
		break;
	case EJamMenuScreen::Cuts:
		RefreshCuts();
		RefreshModes();
		if (ModeGrid && ModeGrid->GetChildrenCount()) { PendingFocus = ModeGrid->GetChildAt(0); break; }
		PendingFocus = CutList && CutList->GetChildrenCount() ? CutList->GetChildAt(0) : (BackButtons.Num() ? static_cast<UWidget*>(BackButtons[0].Get()) : nullptr);
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
	}
}

void UJamMenuWidget::RefreshCuts()
{
	if (!CutList) return;
	CutList->ClearChildren();
	Tinted.RemoveAll([](const TPair<TWeakObjectPtr<UButton>, TWeakObjectPtr<UTextBlock>>& P) { return !P.Key.IsValid() || P.Key->IsA<UJamMenuRowButton>(); });
	UJamDesignerSubsystem* D = Designer();
	Rows = D ? UJamMenuFlow::BuildCutRows(ListCutsWithSeason()) : TArray<FJamMenuCutRow>();
	UWidgetTree* T = WidgetTree;
	const bool bFrame = S::Art(TEXT("T_UI_CutFrame")) != nullptr;
	for (const FJamMenuCutRow& R : Rows)
	{
		UJamMenuRowButton* B = T->ConstructWidget<UJamMenuRowButton>(UJamMenuRowButton::StaticClass());
		FButtonStyle RowStyle = ButtonStyle();
		// Rows are framed by the film strip; the button itself only tints on hover and press.
		RowStyle.SetNormal(S::Rounded(FLinearColor::Transparent, 4.f));
		RowStyle.SetHovered(S::Rounded(FLinearColor(S::Blue().R, S::Blue().G, S::Blue().B, 0.35f), 4.f));
		RowStyle.SetPressed(S::Rounded(FLinearColor(S::Orange().R, S::Orange().G, S::Orange().B, 0.5f), 4.f));
		RowStyle.SetNormalPadding(FMargin(0));
		RowStyle.SetPressedPadding(FMargin(0));
		B->SetStyle(RowStyle);
		B->Bind(this, EJamMenuRowKind::Cut, R.Name);

		// The words: name, one line, the key tunables; the cut's icon at the left.
		UHorizontalBox* Line = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		AddH(Line, S::Icon(T, IconFor(R), 44.f), FMargin(0, 0, 14, 0));
		UVerticalBox* Body = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		UTextBlock* Name = S::Text(T, R.Display.ToUpper(), S::Font(S::Label, FName("Black"), S::TrackLabel), S::Chalk());
		B->TintText = Name;
		AddV(Body, Name, FMargin(0, 0, 0, 1));
		AddV(Body, S::Text(T, R.Description, S::BodyFont(), S::Chalk()), FMargin(0, 0));
		AddV(Body, S::Text(T, R.Tunables, S::CaptionFont(), S::Muted()), FMargin(0, 1, 0, 0));
		AddH(Line, Body, FMargin(0), true);

		// The painted card behind the words, clipped to the row, inside the film-strip frame.
		UOverlay* Stack = T->ConstructWidget<UOverlay>(UOverlay::StaticClass());
		if (UTexture2D* CardArt = Art(*CardTextureFor(R)))
		{
			UWidget* Pic = Picture(T, CardArt, 800.f, 300.f, 0.55f);
			USizeBox* Clip = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
			Clip->SetHeightOverride(bFrame ? 184.f : 92.f);
			Clip->SetClipping(EWidgetClipping::ClipToBounds);
			Clip->AddChild(Pic);
			if (USizeBoxSlot* PS = Cast<USizeBoxSlot>(Pic->Slot)) { PS->SetHorizontalAlignment(HAlign_Fill); PS->SetVerticalAlignment(VAlign_Top); }
			S::AddO(Stack, Clip);
		}
		if (bFrame)
		{
			FSlateBrush Frame = S::CutFrame();
			Frame.DrawAs = ESlateBrushDrawType::Border;   // the sprocket rows tile instead of stretching
			Frame.ImageSize = Frame.ImageSize / 6.f;
			UBorder* FrameBorder = S::Plate(T, Frame, FMargin(0));
			FrameBorder->SetVisibility(ESlateVisibility::HitTestInvisible);
			S::AddO(Stack, FrameBorder);
		}
		// The sprocket rows of the frame tile at about 45 px top and bottom of a 184 px row: the words sit between them.
		S::AddO(Stack, Line, HAlign_Fill, VAlign_Center, bFrame ? FMargin(24, 54, 24, 54) : FMargin(12, 6));
		B->AddChild(Stack);
		CutList->AddChild(B);
		if (UPanelSlot* PS = B->Slot) if (UScrollBoxSlot* SS = Cast<UScrollBoxSlot>(PS)) SS->SetPadding(FMargin(0, 3));
		AddTinted(B, Name);
	}
	CutsStatus->SetText(FText::FromString(Rows.Num() ? FString::Printf(TEXT("%d cuts. Pick one to play it."), Rows.Num()) : TEXT("No cuts found in Content/Cartridges.")));
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
		CardStyle.SetNormalPadding(FMargin(10, 6));
		CardStyle.SetPressedPadding(FMargin(10, 7, 10, 5));
		B->SetStyle(CardStyle);
		// Chosen like a cut: the payload is the cut name ChooseCut loads ("modes/hockey.mode.pond").
		B->Bind(this, EJamMenuRowKind::Cut, M.Name);

		UVerticalBox* Body = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		UTextBlock* Name = S::Text(T, M.Display.ToUpper(), S::Font(S::Body, FName("Black")), S::Chalk());   // no tracking: OUTNUMBERED fits a sixth
		B->TintText = Name;
		AddV(Body, Name, FMargin(0, 0, 0, 2));
		UTextBlock* Pitch = S::Text(T, M.Pitch, S::Font(S::Caption, FName("Italic")), S::Chalk());
		Pitch->SetAutoWrapText(true);
		AddV(Body, Pitch, FMargin(0, 0, 0, 3));
		for (const FString& F : M.Facts)
		{
			UTextBlock* Fact = S::Text(T, FString::Printf(TEXT("- %s"), *F), S::Font(S::Caption - 1, FName("Medium")), S::Muted());
			Fact->SetAutoWrapText(true);
			AddV(Body, Fact, FMargin(0));
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
	if (!D) { CutsStatus->SetText(FText::FromString(TEXT("The Designer subsystem is missing."))); return false; }
	if (!Rows.Num()) Rows = UJamMenuFlow::BuildCutRows(ListCutsWithSeason());
	const FJamMenuCutRow* Row = Rows.FindByPredicate([&](const FJamMenuCutRow& R) { return R.Name == Name; });
	FString Err;
	if (!D->LoadCut(Row ? (Row->bSaved ? Row->Path : Row->Name) : Name, Err))
	{
		if (CutsStatus) CutsStatus->SetText(FText::FromString(FString::Printf(TEXT("%s was rejected: %s"), *Name, *Err)));
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
		if (MainStatus) MainStatus->SetText(FText::FromString(TEXT("Opening the lobby (jam.lobby)...")));
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
