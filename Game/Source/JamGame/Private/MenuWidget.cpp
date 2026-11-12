#include "MenuWidget.h"
#include "MenuStyle.h"
#include "SettingsWidget.h"
#include "JamCartridge.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/SizeBoxSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/Spacer.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

using namespace JamMenuStyle;

DEFINE_LOG_CATEGORY_STATIC(LogJamMenuUi, Log, All);

namespace
{
	// Which painted card a cut row shows (R-0008 art); the raw cut has none yet.
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
	UOverlaySlot* BS = Root->AddChildToOverlay(Backdrop);
	BS->SetHorizontalAlignment(HAlign_Fill);
	BS->SetVerticalAlignment(VAlign_Fill);

	USizeBox* CardBox = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	CardBox->SetWidthOverride(860.f);
	UOverlaySlot* CS = Root->AddChildToOverlay(CardBox);
	CS->SetHorizontalAlignment(HAlign_Center);
	CS->SetVerticalAlignment(VAlign_Center);

	UBorder* Card = Plate(T, Slate(), FMargin(0, 0, 0, 18));
	CardBox->AddChild(Card);
	UVerticalBox* CardBody = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Card->SetContent(CardBody);
	AddV(CardBody, Clapper(T), FMargin(0));

	// Slate header: production and scene lines like a real slate, then the title.
	UHorizontalBox* Head = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	AddH(Head, Text(T, TEXT("PROD.  HOKEJ"), 14, Dim()), FMargin(22, 0, 0, 0), true);
	AddH(Head, Text(T, TEXT("SCENE  3 v 3"), 14, Dim()), FMargin(0), true);
	AddH(Head, Text(T, TEXT("TAKE  YOURS"), 14, Dim()), FMargin(0, 0, 22, 0));
	AddV(CardBody, Head, FMargin(0, 10, 0, 0));

	// The title: the generated slate art when it is there (it carries HOKEJ / A DIRECTOR'S CUT), the chalk text otherwise.
	if (UTexture2D* TitleArt = Art(TEXT("T_UI_TitleSlate")))
	{
		TitlePicture = Picture(T, TitleArt, 560.f);
		AddV(CardBody, TitlePicture, FMargin(0, 6, 0, 4), HAlign_Center);
	}
	else
	{
		UTextBlock* Title = Text(T, TEXT("HOKEJ"), 96, Chalk());
		Title->SetJustification(ETextJustify::Center);
		AddV(CardBody, Title, FMargin(0, 4, 0, 0));
		UTextBlock* Sub = Text(T, TEXT("a director's cut"), 26, Yellow(), FName("Italic"));
		Sub->SetJustification(ETextJustify::Center);
		AddV(CardBody, Sub, FMargin(0, -8, 0, 10));
	}

	Switcher = T->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass(), TEXT("Screens"));
	AddV(CardBody, Switcher, FMargin(28, 6, 28, 0));
	Switcher->AddChild(BuildMain());
	Switcher->AddChild(BuildCuts());
	Switcher->AddChild(BuildDirect());
	Switcher->AddChild(BuildSettings());
	Switcher->AddChild(BuildCredits());
	Switcher->SetActiveWidgetIndex(0);

	UTextBlock* Keys = Text(T, TEXT("STICK / ARROWS  move      A / ENTER  select      B / ESC  back"), 14, Dim());
	Keys->SetJustification(ETextJustify::Center);
	AddV(CardBody, Keys, FMargin(0, 14, 0, 0));
}

UWidget* UJamMenuWidget::BuildMain()
{
	UWidgetTree* T = WidgetTree;
	UVerticalBox* Box = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainScreen"));
	UTextBlock* Txt = nullptr;
	auto Add = [&](TObjectPtr<UButton>& Target, const FString& Label)
	{
		UButton* B = Button(T, Label, 26, Txt);
		Target = B;
		AddTinted(B, Txt);
		AddV(Box, B, FMargin(120, 5));
	};
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
	MainStatus = Text(T, TEXT(""), 15, Dim(), FName("Regular"));
	MainStatus->SetJustification(ETextJustify::Center);
	AddV(Box, MainStatus, FMargin(0, 8, 0, 0));
	return Box;
}

UWidget* UJamMenuWidget::BuildCuts()
{
	UWidgetTree* T = WidgetTree;
	UVerticalBox* Box = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CutsScreen"));
	UTextBlock* H = Text(T, TEXT("CHOOSE YOUR CUT"), 24, Yellow());
	AddV(Box, H, FMargin(0, 0, 0, 4));
	CutList = T->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("CutList"));
	USizeBox* ListBox = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	ListBox->SetMaxDesiredHeight(430.f);
	ListBox->AddChild(CutList);
	AddV(Box, ListBox, FMargin(0, 0));
	CutsStatus = Text(T, TEXT(""), 15, Dim(), FName("Regular"));
	AddV(Box, CutsStatus, FMargin(0, 6, 0, 0));
	UTextBlock* Txt = nullptr;
	UButton* Back = Button(T, TEXT("BACK"), 18, Txt);
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
	AddV(Box, Text(T, TEXT("DIRECT A CUT"), 24, Yellow()), FMargin(0, 0, 0, 2));
	AddV(Box, Text(T, TEXT("Tell the Designer in one sentence how you want this cut. It edits the rules, the ice, the shots, the look."), 15, Dim(), FName("Regular")), FMargin(0, 0, 0, 6));

	UHorizontalBox* Row = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	SentenceBox = T->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("SentenceBox"));
	SentenceBox->SetHintText(FText::FromString(TEXT("make it faster and meaner")));
	SentenceBox->SetMinDesiredWidth(520.f);
	SentenceBox->OnTextCommitted.AddDynamic(this, &UJamMenuWidget::OnSentenceCommitted);
	AddH(Row, SentenceBox, FMargin(0, 0, 8, 0), true);
	UTextBlock* Txt = nullptr;
	RollButton = Button(T, TEXT("ROLL"), 20, Txt);
	RollButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnRollClicked);
	AddTinted(RollButton, Txt);
	AddH(Row, RollButton, FMargin(0));
	AddV(Box, Row, FMargin(0, 0, 0, 4));

	DirectStatus = Text(T, TEXT(""), 16, Yellow(), FName("Regular"));
	DirectStatus->SetAutoWrapText(true);
	AddV(Box, DirectStatus, FMargin(0, 2, 0, 2));
	DirectHint = Text(T, TEXT(""), 14, Dim(), FName("Regular"));
	DirectHint->SetAutoWrapText(true);
	AddV(Box, DirectHint, FMargin(0, 0, 0, 4));

	// The result: the cut's name, its three biggest changes, and what to do with it.
	ResultBox = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ResultBox"));
	UBorder* ResultPlate = Plate(T, SlateDark(), FMargin(14, 10));
	ResultPlate->SetContent(ResultBox);
	ResultName = Text(T, TEXT(""), 28, Chalk());
	AddV(ResultBox, ResultName, FMargin(0, 0, 0, 2));
	ResultChanges = Text(T, TEXT(""), 16, Chalk(), FName("Regular"));
	ResultChanges->SetAutoWrapText(true);
	AddV(ResultBox, ResultChanges, FMargin(0, 0, 0, 8));
	UHorizontalBox* Actions = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	PlayResultButton = Button(T, TEXT("PLAY"), 18, Txt);
	PlayResultButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnPlayResultClicked);
	AddTinted(PlayResultButton, Txt);
	AddH(Actions, PlayResultButton, FMargin(0, 0, 8, 0));
	UTextBlock* SaveTxt = nullptr;
	SaveResultButton = Button(T, TEXT("SAVE AS MY DIRECTOR'S CUT"), 18, SaveTxt);
	SaveResultText = SaveTxt;
	SaveResultButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnSaveResultClicked);
	AddTinted(SaveResultButton, SaveTxt);
	AddH(Actions, SaveResultButton, FMargin(0, 0, 8, 0));
	TryAgainButton = Button(T, TEXT("TRY AGAIN"), 18, Txt);
	TryAgainButton->OnClicked.AddDynamic(this, &UJamMenuWidget::OnTryAgainClicked);
	AddTinted(TryAgainButton, Txt);
	AddH(Actions, TryAgainButton, FMargin(0));
	AddV(ResultBox, Actions, FMargin(0));
	AddV(Box, ResultPlate, FMargin(0, 4, 0, 4));
	ResultPlate->SetVisibility(ESlateVisibility::Collapsed);
	ResultBox->SetVisibility(ESlateVisibility::Collapsed);

	AddV(Box, Text(T, TEXT("OR PICK A SENTENCE THE DESIGNER ALREADY SHOT"), 14, Dim()), FMargin(0, 6, 0, 2));
	SentenceList = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SentenceList"));
	// Scrolls: the cache has a dozen sentences and the card must fit 720p.
	UScrollBox* SentenceScroll = T->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("SentenceScroll"));
	SentenceScroll->AddChild(SentenceList);
	USizeBox* SentenceClip = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	SentenceClip->SetMaxDesiredHeight(190.f);
	SentenceClip->AddChild(SentenceScroll);
	AddV(Box, SentenceClip, FMargin(0));

	UButton* Back = Button(T, TEXT("BACK"), 18, Txt);
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
	AddV(Box, Text(T, TEXT("SETTINGS"), 24, Yellow()), FMargin(0, 0, 0, 2));
	SettingsPanel = CreateWidget<UJamSettingsWidget>(this, UJamSettingsWidget::StaticClass());
	if (SettingsPanel) AddV(Box, SettingsPanel, FMargin(0));
	UTextBlock* Txt = nullptr;
	UButton* Back = Button(T, TEXT("BACK"), 18, Txt);
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
	AddV(Box, Text(T, TEXT("CREDITS"), 24, Yellow()), FMargin(0, 0, 0, 6));
	UTextBlock* Body = Text(T,
		TEXT("HOKEJ, a director's cut. Team Hokej, 2026 Epic MegaJam, theme Director's Cut.\n\n")
		TEXT("Built with Unreal Engine.\n\n")
		TEXT("One human directed two machines of agents; the agents wrote the game, the cuts and this menu. ")
		TEXT("The Designer turns your sentence into a cut through the Claude API with a key you type yourself; without a key it answers from cuts it shot earlier.\n\n")
		TEXT("Tools used: Unreal Engine 5.9, Claude Code and the Claude API, Node.js, Git with LFS, Meshy for generated figures, PowerShell.\n\n")
		TEXT("No goalies were harmed. No logos were used."),
		16, Chalk(), FName("Regular"));
	Body->SetAutoWrapText(true);
	AddV(Box, Body, FMargin(0, 0, 0, 6));
	UTextBlock* Txt = nullptr;
	UButton* Back = Button(T, TEXT("BACK"), 18, Txt);
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
	// The slate art heads the main and credits screens; the tall screens (cuts, direct, settings) keep the card inside 720p.
	if (TitlePicture) TitlePicture->SetVisibility(Screen == EJamMenuScreen::Main || Screen == EJamMenuScreen::Credits ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	switch (Screen)
	{
	case EJamMenuScreen::Main:
		PendingFocus = ChooseButton.Get();
		break;
	case EJamMenuScreen::Cuts:
		RefreshCuts();
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
	Rows = D ? UJamMenuFlow::BuildCutRows(D->ListCuts()) : TArray<FJamMenuCutRow>();
	UWidgetTree* T = WidgetTree;
	for (const FJamMenuCutRow& R : Rows)
	{
		UJamMenuRowButton* B = T->ConstructWidget<UJamMenuRowButton>(UJamMenuRowButton::StaticClass());
		B->SetStyle(ButtonStyle());

		B->Bind(this, EJamMenuRowKind::Cut, R.Name);
		UVerticalBox* Body = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		UTextBlock* Name = Text(T, R.Display.ToUpper(), 22, Chalk());
		B->TintText = Name;
		AddV(Body, Name, FMargin(0, 0, 0, 1));
		AddV(Body, Text(T, R.Description, 15, Chalk(), FName("Regular")), FMargin(0, 0));
		AddV(Body, Text(T, R.Tunables, 13, Dim(), FName("Regular")), FMargin(0, 1, 0, 0));
		// The cut's painted card behind the words when it exists (text stays on top and reads without it).
		if (UTexture2D* CardArt = Art(*CardTextureFor(R)))
		{
			UOverlay* Stack = T->ConstructWidget<UOverlay>(UOverlay::StaticClass());
			// The card's upper band (rink and skaters) shows; its lower, emptier half is clipped away under the text.
			UWidget* Pic = Picture(T, CardArt, 780.f, 300.f, 0.5f);
			USizeBox* Clip = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
			Clip->SetHeightOverride(88.f);
			Clip->SetClipping(EWidgetClipping::ClipToBounds);
			Clip->AddChild(Pic);
			if (USizeBoxSlot* PS = Cast<USizeBoxSlot>(Pic->Slot)) { PS->SetHorizontalAlignment(HAlign_Fill); PS->SetVerticalAlignment(VAlign_Top); }
			UOverlaySlot* S0 = Stack->AddChildToOverlay(Clip);
			S0->SetHorizontalAlignment(HAlign_Fill);
			S0->SetVerticalAlignment(VAlign_Fill);
			UOverlaySlot* S1 = Stack->AddChildToOverlay(Body);
			S1->SetHorizontalAlignment(HAlign_Fill);
			S1->SetVerticalAlignment(VAlign_Center);
			S1->SetPadding(FMargin(8, 2));
			B->AddChild(Stack);
		}
		else
		{
			B->AddChild(Body);
		}
		CutList->AddChild(B);
		if (UPanelSlot* S = B->Slot) if (UScrollBoxSlot* SS = Cast<UScrollBoxSlot>(S)) SS->SetPadding(FMargin(0, 3));
		AddTinted(B, Name);
	}
	CutsStatus->SetText(FText::FromString(Rows.Num() ? FString::Printf(TEXT("%d cuts. Pick one to play it."), Rows.Num()) : TEXT("No cuts found in Content/Cartridges.")));
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
			for (const FString& S : Sentences)
			{
				UJamMenuRowButton* B = T->ConstructWidget<UJamMenuRowButton>(UJamMenuRowButton::StaticClass());
				B->SetStyle(ButtonStyle());

				B->Bind(this, EJamMenuRowKind::Sentence, S);
				UTextBlock* Txt = Text(T, FString::Printf(TEXT("\"%s\""), *S), 15, Chalk(), FName("Regular"));
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
	if (!Rows.Num()) Rows = UJamMenuFlow::BuildCutRows(D->ListCuts());
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
	const FString S = Sentence.TrimStartAndEnd();
	if (!D || S.IsEmpty()) { DirectStatus->SetText(FText::FromString(TEXT("Type one sentence first."))); return; }
	if (bRolling) return;
	bRolling = true;
	bHasResult = false;
	bResultSaved = false;
	SavedResultPath.Empty();
	if (ResultBox) { ResultBox->SetVisibility(ESlateVisibility::Collapsed); if (UWidget* P = ResultBox->GetParent()) P->SetVisibility(ESlateVisibility::Collapsed); }
	DirectStatus->SetText(FText::FromString(TEXT("rolling...")));
	RollButton->SetIsEnabled(false);
	TWeakObjectPtr<UJamMenuWidget> Weak(this);
	D->RequestCut(S, FString(), [Weak](const FJamDesignerResult& R) { if (Weak.IsValid()) Weak->OnResult(R); });
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
