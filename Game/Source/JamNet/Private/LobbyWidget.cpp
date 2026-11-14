#include "LobbyWidget.h"
#include "JamLobbySubsystem.h"
#include "JamCartridge.h"
#include "MenuGameMode.h"
#include "MenuWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Styling/CoreStyle.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

namespace
{
	FSlateFontInfo BoldFont(int32 Size) { return FSlateFontInfo(FCoreStyle::GetDefaultFontStyle("Bold", Size)); }
	FSlateFontInfo RegularFont(int32 Size) { return FSlateFontInfo(FCoreStyle::GetDefaultFontStyle("Regular", Size)); }

	UTextBlock* MakeText(UWidgetTree* Tree, const TCHAR* Name, int32 Size, const FLinearColor& Colour, bool bBold = true)
	{
		UTextBlock* T = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		T->SetFont(bBold ? BoldFont(Size) : RegularFont(Size));
		T->SetColorAndOpacity(FSlateColor(Colour));
		T->SetShadowOffset(FVector2D(1, 1));
		T->SetShadowColorAndOpacity(FLinearColor(0, 0, 0, 0.8f));
		T->SetAutoWrapText(true);
		return T;
	}

	const FLinearColor Ink(0.03f, 0.03f, 0.04f, 1.f);  // opaque: it sits over the menu or the rink
	const FLinearColor Chalk(0.92f, 0.90f, 0.85f, 1.f);
	const FLinearColor Dim(0.65f, 0.63f, 0.60f, 1.f);
	const FLinearColor Blue = FLinearColor::FromSRGBColor(FColor(0x1D, 0x4E, 0xD8));
	const FLinearColor Orange = FLinearColor::FromSRGBColor(FColor(0xE8, 0x65, 0x1A));
}

ULobbyWidget* ULobbyWidget::Open(APlayerController* PC)
{
	if (!PC) return nullptr;
	ULobbyWidget* W = CreateWidget<ULobbyWidget>(PC, ULobbyWidget::StaticClass());
	if (!W) return nullptr;
	W->AddToViewport(20);
	PC->SetShowMouseCursor(true);
	FInputModeGameAndUI Mode;
	Mode.SetWidgetToFocus(W->TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(Mode);
	return W;
}

UJamLobbySubsystem* ULobbyWidget::Lobby() const
{
	UWorld* World = GetWorld();
	return (World && World->GetGameInstance()) ? World->GetGameInstance()->GetSubsystem<UJamLobbySubsystem>() : nullptr;
}

TSharedRef<SWidget> ULobbyWidget::RebuildWidget()
{
	BuildTree();
	return Super::RebuildWidget();
}

UButton* ULobbyWidget::MakeButton(const TCHAR* Name, const FString& Label, UTextBlock*& OutText)
{
	UButton* B = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	B->SetBackgroundColor(FLinearColor(0.85f, 0.82f, 0.75f, 1.f));
	OutText = MakeText(WidgetTree, *FString::Printf(TEXT("%s_Text"), Name), 18, FLinearColor(0.05f, 0.05f, 0.06f));
	OutText->SetText(FText::FromString(Label));
	OutText->SetShadowOffset(FVector2D::ZeroVector);
	B->AddChild(OutText);
	return B;
}

void ULobbyWidget::BuildTree()
{
	if (!WidgetTree) WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	if (WidgetTree->RootWidget) return;
	UWidgetTree* T = WidgetTree;
	Root = T->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
	T->RootWidget = Root;

	// The shipped cuts, by name (the Designer's cut joins when its file exists).
	TArray<FString> Files;
	IFileManager::Get().FindFiles(Files, *FPaths::Combine(UJamCartridge::GetCartridgeDir(), TEXT("*.cartridge.json")), true, false);
	Files.Sort();
	for (const FString& F : Files) { FString N = F; N.RemoveFromEnd(TEXT(".cartridge.json")); if (!N.StartsWith(TEXT("hockey.test"))) Cuts.Add(N); }
	if (Cuts.Num() == 0) Cuts.Add(TEXT("hockey.regular"));
	CutIndex = FMath::Max(0, Cuts.IndexOfByKey(TEXT("hockey.regular")));

	// Lane A144 (track U2): the screen under the lobby goes dark (the menu's CREDITS and QUIT showed under the box and took a click).
	UBorder* Backdrop = T->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Backdrop"));
	Backdrop->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.94f));
	UCanvasPanelSlot* BackdropSlot = Root->AddChildToCanvas(Backdrop);
	BackdropSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
	BackdropSlot->SetOffsets(FMargin(0.f));

	UBorder* Slate = T->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Slate"));
	Slate->SetBrushColor(Ink);
	Slate->SetPadding(FMargin(36, 28));
	UCanvasPanelSlot* SlateSlot = Root->AddChildToCanvas(Slate);
	SlateSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
	SlateSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	SlateSlot->SetAutoSize(true);   // lane A144: as high as its rows (a fixed 760 x 560 left a third of the box empty); the width is the column's box
	SlateSlot->SetPosition(FVector2D::ZeroVector);

	Column = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Column"));
	USizeBox* ColumnBox = T->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("ColumnBox"));
	ColumnBox->SetWidthOverride(748.f);
	ColumnBox->AddChild(Column);
	Slate->SetContent(ColumnBox);
	auto Row = [&](UWidget* W, float Top = 8.f) { UVerticalBoxSlot* S = Column->AddChildToVerticalBox(W); S->SetPadding(FMargin(0, Top, 0, 0)); S->SetHorizontalAlignment(HAlign_Fill); return S; };

	Title = MakeText(T, TEXT("Title"), 34, Chalk);
	Title->SetText(FText::FromString(TEXT("PLAY WITH A FRIEND")));
	Row(Title, 0);
	Status = MakeText(T, TEXT("Status"), 16, Dim, false);
	Row(Status, 6);

	// Lane A144: the words of JOIN-GUIDE.md sections 2 and 3, where the player needs them.
	HostHelp = MakeText(T, TEXT("HostHelp"), 15, Chalk, false);
	HostHelp->SetText(FText::FromString(TEXT("HOST: pick the cut with < > and press HOST. Both machines: the same network, the same build.")));
	Row(HostHelp, 16);

	// Host row: < cut > [Host]
	HostRow = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("HostRow"));
	UTextBlock* Tmp = nullptr;
	UButton* Prev = MakeButton(TEXT("CutPrev"), TEXT("<"), Tmp);
	Prev->OnClicked.AddDynamic(this, &ULobbyWidget::OnCutPrev);
	HostRow->AddChildToHorizontalBox(Prev);
	CutText = MakeText(T, TEXT("CutText"), 20, Chalk);
	UHorizontalBoxSlot* CutSlot = HostRow->AddChildToHorizontalBox(CutText);
	CutSlot->SetPadding(FMargin(14, 6));
	CutSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	UButton* Next = MakeButton(TEXT("CutNext"), TEXT(">"), Tmp);
	Next->OnClicked.AddDynamic(this, &ULobbyWidget::OnCutNext);
	HostRow->AddChildToHorizontalBox(Next);
	HostButton = MakeButton(TEXT("HostButton"), TEXT("HOST"), Tmp);
	HostButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnHost);
	HostRow->AddChildToHorizontalBox(HostButton)->SetPadding(FMargin(14, 0, 0, 0));
	Row(HostRow, 6);

	JoinHelp = MakeText(T, TEXT("JoinHelp"), 15, Chalk, false);
	JoinHelp->SetText(FText::FromString(TEXT("GUEST: type the host's code in the box, press JOIN.")));
	Row(JoinHelp, 16);

	// Join row: [code box] [Join]
	JoinRow = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("JoinRow"));
	CodeBox = T->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("CodeBox"));
	CodeBox->SetHintText(FText::FromString(TEXT("friend's code: ABCD-EFGH")));
	// Lane A144: chalk on dark (the default light box hid its grey hint).
	CodeBox->WidgetStyle.SetBackgroundColor(FLinearColor(0.10f, 0.10f, 0.12f, 1.f));
	CodeBox->WidgetStyle.SetForegroundColor(Chalk);
	CodeBox->WidgetStyle.SetFocusedForegroundColor(Chalk);
	CodeBox->WidgetStyle.SetFont(RegularFont(18));
	CodeBox->WidgetStyle.SetPadding(FMargin(10, 7));
	UHorizontalBoxSlot* CodeSlot = JoinRow->AddChildToHorizontalBox(CodeBox);
	CodeSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	JoinButton = MakeButton(TEXT("JoinButton"), TEXT("JOIN"), Tmp);
	JoinButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnJoin);
	JoinRow->AddChildToHorizontalBox(JoinButton)->SetPadding(FMargin(14, 0, 0, 0));
	Row(JoinRow, 6);

	CodesText = MakeText(T, TEXT("CodesText"), 30, Chalk);
	CodesText->SetJustification(ETextJustify::Center);
	Row(CodesText, 18);
	CodesHelp = MakeText(T, TEXT("CodesHelp"), 15, Chalk, false);
	CodesHelp->SetText(FText::FromString(TEXT("Read your friend the code whose address starts like theirs (usually 192.168.). The 127.0.0.1 code is for two copies on one machine.")));
	CodesHelp->SetJustification(ETextJustify::Center);
	Row(CodesHelp, 6);
	WarningText = MakeText(T, TEXT("WarningText"), 14, Dim, false);
	Row(WarningText, 8);

	NamesRow = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("NamesRow"));
	HomePlate = T->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("HomePlate"));
	HomePlate->SetBrushColor(Blue);
	HomePlate->SetPadding(FMargin(16, 8));
	HomeName = MakeText(T, TEXT("HomeName"), 22, FLinearColor::White);
	HomePlate->SetContent(HomeName);
	NamesRow->AddChildToHorizontalBox(HomePlate)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	UTextBlock* Vs = MakeText(T, TEXT("Vs"), 22, Dim);
	Vs->SetText(FText::FromString(TEXT("  vs  ")));
	NamesRow->AddChildToHorizontalBox(Vs)->SetVerticalAlignment(VAlign_Center);
	AwayPlate = T->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("AwayPlate"));
	AwayPlate->SetBrushColor(Orange);
	AwayPlate->SetPadding(FMargin(16, 8));
	AwayName = MakeText(T, TEXT("AwayName"), 22, FLinearColor::White);
	AwayPlate->SetContent(AwayName);
	NamesRow->AddChildToHorizontalBox(AwayPlate)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	Row(NamesRow, 18);

	UHorizontalBox* Bottom = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Bottom"));
	BackButton = MakeButton(TEXT("BackButton"), TEXT("BACK"), Tmp);
	BackButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnBack);
	Bottom->AddChildToHorizontalBox(BackButton);
	NameText = MakeText(T, TEXT("NameText"), 14, Dim, false);
	UHorizontalBoxSlot* NameSlot = Bottom->AddChildToHorizontalBox(NameText);
	NameSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	NameSlot->SetPadding(FMargin(14, 8));
	UTextBlock* StartLabel = nullptr;
	StartButton = MakeButton(TEXT("StartButton"), TEXT("START"), StartLabel);
	StartText = StartLabel;
	StartButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnStart);
	Bottom->AddChildToHorizontalBox(StartButton);
	Row(Bottom, 18);
}

FString ULobbyWidget::GetStatusLine() const
{
	UJamLobbySubsystem* L = Lobby();
	return L ? L->GetStatusText() : FString(TEXT("no lobby subsystem"));
}

void ULobbyWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UJamLobbySubsystem* L = Lobby();
	if (!L || !Status) return;
	const EJamLobbyState S = L->GetState();
	const bool bIdle = S == EJamLobbyState::Idle || S == EJamLobbyState::Failed || S == EJamLobbyState::Ended;
	const bool bHosting = S == EJamLobbyState::Hosting || (S == EJamLobbyState::Ready && L->IsHost()) || S == EJamLobbyState::Starting;
	Status->SetText(FText::FromString(L->GetStatusText()));
	Status->SetColorAndOpacity(FSlateColor(S == EJamLobbyState::Failed ? Orange : Dim));
	if (Cuts.IsValidIndex(CutIndex)) CutText->SetText(FText::FromString(bIdle ? Cuts[CutIndex] : L->GetCut()));
	HostRow->SetVisibility(bIdle || bHosting ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	HostButton->SetIsEnabled(bIdle);
	JoinRow->SetVisibility(bIdle || S == EJamLobbyState::Joining ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	JoinButton->SetIsEnabled(bIdle);
	CodesText->SetText(FText::FromString(bHosting ? L->GetCodesText() : FString()));
	CodesText->SetVisibility(bHosting ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (HostHelp) HostHelp->SetVisibility(bIdle ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (JoinHelp) JoinHelp->SetVisibility(bIdle ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (CodesHelp) CodesHelp->SetVisibility(bHosting && S != EJamLobbyState::Ready && S != EJamLobbyState::Starting ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	WarningText->SetText(FText::FromString(L->GetWarningText()));
	WarningText->SetVisibility(bIdle || bHosting ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);  // the port is the host's
	const bool bPeers = S == EJamLobbyState::Ready || S == EJamLobbyState::Starting || S == EJamLobbyState::InMatch;
	NamesRow->SetVisibility(bPeers ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (bPeers)
	{
		const FString HostName = L->IsHost() ? L->GetLocalName() : L->GetRemoteName();
		const FString GuestName = L->IsHost() ? L->GetRemoteName() : L->GetLocalName();
		HomeName->SetText(FText::FromString(HostName + (L->IsHost() ? TEXT(" (you)") : TEXT(""))));
		AwayName->SetText(FText::FromString(GuestName + (L->IsHost() ? TEXT("") : TEXT(" (you)"))));
	}
	StartButton->SetVisibility(L->IsHost() && S == EJamLobbyState::Ready ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	NameText->SetText(FText::FromString(bIdle ? FString() : FString::Printf(TEXT("you: %s   port %d"), *L->GetLocalName(), L->GetBoundPort())));
	// The level change takes the widget with it; nothing to do when the match starts.

	// Proof screenshots (jam.net.AutoShot 1): the screen three seconds after it opened, named by role and state.
	static IConsoleVariable* AutoShot = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.net.AutoShot"));
	if (AutoShot && AutoShot->GetInt() != 0 && !bShotTaken && ++TicksOpen == 180)
	{
		bShotTaken = true;
		FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("JamNet_lobby_%s_state%d"), L->IsHost() ? TEXT("host") : TEXT("guest"), static_cast<int32>(S)), true, false);
	}
}

void ULobbyWidget::OnCutPrev() { if (Cuts.Num()) CutIndex = (CutIndex + Cuts.Num() - 1) % Cuts.Num(); }
void ULobbyWidget::OnCutNext() { if (Cuts.Num()) CutIndex = (CutIndex + 1) % Cuts.Num(); }

void ULobbyWidget::OnHost()
{
	if (UJamLobbySubsystem* L = Lobby()) L->Host(Cuts.IsValidIndex(CutIndex) ? Cuts[CutIndex] : TEXT("hockey.regular"), FString());
}

void ULobbyWidget::OnJoin()
{
	if (UJamLobbySubsystem* L = Lobby()) L->Join(CodeBox ? CodeBox->GetText().ToString() : FString(), FString());
}

void ULobbyWidget::OnStart()
{
	if (UJamLobbySubsystem* L = Lobby()) L->StartMatch();
}

void ULobbyWidget::OnBack()
{
	if (UJamLobbySubsystem* L = Lobby()) L->Leave();
	if (APlayerController* PC = GetOwningPlayer())
	{
		// Lane A144 (track U2): BACK over the menu gives the menu its cursor and its focus back (game-only input with the cursor hidden left the
		// first screen dead to the mouse and the keys); over a match it is the game's input, as before.
		AJamMenuGameMode* MenuGM = GetWorld() ? GetWorld()->GetAuthGameMode<AJamMenuGameMode>() : nullptr;
		if (UJamMenuWidget* Menu = MenuGM ? MenuGM->GetMenu() : nullptr)
		{
			PC->SetShowMouseCursor(true);
			FInputModeGameAndUI Mode;
			Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			Mode.SetHideCursorDuringCapture(false);
			Mode.SetWidgetToFocus(Menu->TakeWidget());
			PC->SetInputMode(Mode);
			Menu->ShowScreen(EJamMenuScreen::Main);
		}
		else
		{
			PC->SetShowMouseCursor(false);
			PC->SetInputMode(FInputModeGameOnly());
		}
	}
	RemoveFromParent();
}

// ---- card ----

ULobbyCardWidget* ULobbyCardWidget::Show(APlayerController* PC, const FString& InTitle, const FString& InSubtitle)
{
	if (!PC) return nullptr;
	ULobbyCardWidget* W = CreateWidget<ULobbyCardWidget>(PC, ULobbyCardWidget::StaticClass());
	if (!W) return nullptr;
	W->PendingTitle = InTitle;
	W->PendingSubtitle = InSubtitle;
	W->AddToViewport(30);
	return W;
}

TSharedRef<SWidget> ULobbyCardWidget::RebuildWidget()
{
	if (!WidgetTree) WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	if (!WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
		WidgetTree->RootWidget = Root;
		UBorder* Plate = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Plate"));
		Plate->SetBrushColor(Ink);
		Plate->SetPadding(FMargin(48, 28));
		UVerticalBox* Col = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Col"));
		Plate->SetContent(Col);
		TitleText = MakeText(WidgetTree, TEXT("CardTitle"), 56, Orange);
		TitleText->SetJustification(ETextJustify::Center);
		Col->AddChildToVerticalBox(TitleText);
		SubtitleText = MakeText(WidgetTree, TEXT("CardSubtitle"), 18, Chalk, false);
		SubtitleText->SetJustification(ETextJustify::Center);
		Col->AddChildToVerticalBox(SubtitleText)->SetPadding(FMargin(0, 10, 0, 0));
		UCanvasPanelSlot* S = Root->AddChildToCanvas(Plate);
		S->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
		S->SetAlignment(FVector2D(0.5f, 0.5f));
		S->SetAutoSize(true);
		S->SetPosition(FVector2D::ZeroVector);
	}
	TitleText->SetText(FText::FromString(PendingTitle));
	SubtitleText->SetText(FText::FromString(PendingSubtitle));
	return Super::RebuildWidget();
}
