#include "HUDWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "GameFramework/PlayerController.h"
#include "Styling/CoreStyle.h"
#include "Engine/World.h"

namespace
{
	FSlateFontInfo BoldFont(int32 Size)
	{
		return FSlateFontInfo(FCoreStyle::GetDefaultFontStyle("Bold", Size));
	}

	UTextBlock* MakeText(UWidgetTree* Tree, const TCHAR* Name, int32 Size, const FLinearColor& Colour)
	{
		UTextBlock* T = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		T->SetFont(BoldFont(Size));
		T->SetColorAndOpacity(FSlateColor(Colour));
		T->SetShadowOffset(FVector2D(2, 2));
		T->SetShadowColorAndOpacity(FLinearColor(0, 0, 0, 0.8f));
		T->SetVisibility(ESlateVisibility::HitTestInvisible);
		return T;
	}

	UCanvasPanelSlot* Place(UCanvasPanel* Canvas, UWidget* W, FVector2D Anchor, FVector2D Align, FVector2D Pos, FVector2D Size = FVector2D::ZeroVector)
	{
		UCanvasPanelSlot* S = Canvas->AddChildToCanvas(W);
		S->SetAnchors(FAnchors(Anchor.X, Anchor.Y, Anchor.X, Anchor.Y));
		S->SetAlignment(Align);
		S->SetPosition(Pos);
		if (Size.IsZero())
		{
			S->SetAutoSize(true);
		}
		else
		{
			S->SetAutoSize(false);
			S->SetSize(Size);
		}
		return S;
	}
}

UJamHudWidget* UJamHudWidget::CreateAndAdd(APlayerController* PC)
{
	if (!PC)
	{
		return nullptr;
	}
	UJamHudWidget* W = CreateWidget<UJamHudWidget>(PC, UJamHudWidget::StaticClass());
	if (W)
	{
		W->AddToViewport(0);
	}
	return W;
}

FString UJamHudWidget::FormatTimer(int32 FramesRemaining)
{
	const int32 Secs = FMath::Max(0, FMath::DivideAndRoundUp(FramesRemaining, 60));
	return FString::Printf(TEXT("%d:%02d"), Secs / 60, Secs % 60);
}

FString UJamHudWidget::FormatTake(int32 Take)
{
	return FString::Printf(TEXT("TAKE %d"), FMath::Max(1, Take));
}

FString UJamHudWidget::CardFor(EJamPhase Phase)
{
	switch (Phase)
	{
	case EJamPhase::Win: return TEXT("THAT'S A WRAP");
	case EJamPhase::Lose: return TEXT("RETAKE?");
	case EJamPhase::Paused: return TEXT("PAUSED");
	default: return FString();
	}
}

TSharedRef<SWidget> UJamHudWidget::RebuildWidget()
{
	BuildTree();
	return Super::RebuildWidget();
}

void UJamHudWidget::BuildTree()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	}
	if (WidgetTree->RootWidget)
	{
		return;
	}
	UWidgetTree* T = WidgetTree;
	Root = T->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
	T->RootWidget = Root;

	const FLinearColor Ink(0.03f, 0.03f, 0.04f, 0.82f);

	// Score as a film counter: two dark boxes with team-coloured digit plates, take and timer beneath.
	HomeBox = T->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("HomeBox"));
	HomeBox->SetBrushColor(FLinearColor::FromSRGBColor(FColor(0x1D, 0x4E, 0xD8)));
	HomeBox->SetPadding(FMargin(14, 2));
	ScoreHome = MakeText(T, TEXT("ScoreHome"), 44, FLinearColor::White);
	HomeBox->SetContent(ScoreHome);
	HomeBox->SetVisibility(ESlateVisibility::HitTestInvisible);

	AwayBox = T->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("AwayBox"));
	AwayBox->SetBrushColor(FLinearColor::FromSRGBColor(FColor(0xE8, 0x65, 0x1A)));
	AwayBox->SetPadding(FMargin(14, 2));
	ScoreAway = MakeText(T, TEXT("ScoreAway"), 44, FLinearColor::White);
	AwayBox->SetContent(ScoreAway);
	AwayBox->SetVisibility(ESlateVisibility::HitTestInvisible);

	Place(Root, HomeBox, FVector2D(0.5, 0), FVector2D(1, 0), FVector2D(-10, 18));
	Place(Root, AwayBox, FVector2D(0.5, 0), FVector2D(0, 0), FVector2D(10, 18));
	TakeText = MakeText(T, TEXT("TakeText"), 22, FLinearColor(1.f, 0.85f, 0.4f));
	Place(Root, TakeText, FVector2D(0.5, 0), FVector2D(1, 0), FVector2D(-10, 82));
	TimerText = MakeText(T, TEXT("TimerText"), 22, FLinearColor::White);
	Place(Root, TimerText, FVector2D(0.5, 0), FVector2D(0, 0), FVector2D(10, 82));

	// Far-net marker: a magenta diamond (a border at 45 degrees) and a distance text.
	NetDiamond = T->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("NetDiamond"));
	NetDiamond->SetBrushColor(FLinearColor(1.f, 0.1f, 0.8f, 1.f));
	NetDiamond->SetRenderTransformAngle(45.f);
	NetDiamond->SetRenderTransformPivot(FVector2D(0.5, 0.5));
	NetDiamond->SetVisibility(ESlateVisibility::HitTestInvisible);
	Place(Root, NetDiamond, FVector2D(0, 0), FVector2D(0.5, 0.5), FVector2D(0, 0), FVector2D(22, 22));
	NetText = MakeText(T, TEXT("NetText"), 22, FLinearColor::White);
	Place(Root, NetText, FVector2D(0, 0), FVector2D(0.5, 0), FVector2D(0, 0));

	// Radar 200 x 100, bottom-right with a 24 px margin.
	Radar = T->ConstructWidget<UJamRadarWidget>(UJamRadarWidget::StaticClass(), TEXT("Radar"));
	Place(Root, Radar, FVector2D(1, 1), FVector2D(0, 0), FVector2D(-224, -124), FVector2D(UJamRadarWidget::RadarWidthPx, UJamRadarWidget::RadarHeightPx));

	// Director's Call prompt placeholder, cards and the restart hint.
	CallText = MakeText(T, TEXT("CallText"), 24, FLinearColor(1.f, 0.85f, 0.4f));
	CallText->SetText(FText::FromString(TEXT("DIRECTOR'S CALL READY")));
	CallText->SetVisibility(ESlateVisibility::Collapsed);
	Place(Root, CallText, FVector2D(0.5, 1), FVector2D(0.5, 1), FVector2D(0, -40));

	CardTextBlock = MakeText(T, TEXT("CardText"), 84, FLinearColor::White);
	CardTextBlock->SetVisibility(ESlateVisibility::Collapsed);
	Place(Root, CardTextBlock, FVector2D(0.5, 0.4), FVector2D(0.5, 0.5), FVector2D(0, 0));

	HintText = MakeText(T, TEXT("HintText"), 26, FLinearColor(1.f, 1.f, 1.f, 0.9f));
	HintText->SetText(FText::FromString(TEXT("R  RESTART      ESC  PAUSE")));
	HintText->SetVisibility(ESlateVisibility::Collapsed);
	Place(Root, HintText, FVector2D(0.5, 0.4), FVector2D(0.5, 0), FVector2D(0, 70));
}

void UJamHudWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (const UJamPresentation* P = UJamPresentation::Get(this))
	{
		if (P->HasFrame())
		{
			RefreshFromFrame(P->GetFrame());
		}
	}
}

void UJamHudWidget::RefreshFromFrame(const FJamPresentationFrame& F)
{
	if (!ScoreHome)
	{
		return;
	}
	ScoreHome->SetText(FText::FromString(FString::Printf(TEXT("%02d"), F.ScoreHome)));
	ScoreAway->SetText(FText::FromString(FString::Printf(TEXT("%02d"), F.ScoreAway)));
	HomeBox->SetBrushColor(F.TeamColour0);
	AwayBox->SetBrushColor(F.TeamColour1);
	TakeText->SetText(FText::FromString(FormatTake(F.Take)));
	TimerText->SetText(FText::FromString(F.bHasTimer ? FormatTimer(F.FramesRemaining) : FString()));
	TimerText->SetVisibility(F.bHasTimer ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	const FString Card = CardFor(F.Phase);
	CardTextBlock->SetText(FText::FromString(Card));
	CardTextBlock->SetVisibility(Card.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	const bool bEnd = F.Phase == EJamPhase::Win || F.Phase == EJamPhase::Lose;
	HintText->SetVisibility(bEnd ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	CallText->SetVisibility(F.bDirectorsCallReady && !bEnd ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	Radar->SetFrame(F);
	UpdateMarker(F);
}

void UJamHudWidget::UpdateMarker(const FJamPresentationFrame& F)
{
	APlayerController* PC = GetOwningPlayer();
	const FVector Net = F.GetFarNetWorldCm();
	float DistM = 0.f;
	if (F.Skaters.IsValidIndex(F.HumanIndex))
	{
		DistM = FVector2D::Distance(F.Skaters[F.HumanIndex].PosM, FVector2D(Net.X / 100.f, Net.Y / 100.f));
	}
	Marker.DistanceM = FMath::RoundToInt32(DistM);
	Marker.Text = FString::Printf(TEXT("%d m"), Marker.DistanceM);

	const FVector2D VS = UWidgetLayoutLibrary::GetViewportSize(this);
	const float Scale = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), 0.01f);
	const float VW = VS.X / Scale, VH = VS.Y / Scale;

	FVector2D Sp(VW * 0.5f, VH * 0.5f);
	bool bProj = PC && UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, Net, Sp, false);
	// Behind the camera the projection is mirrored or fails: park the marker on the side edge the net lies towards.
	bool bBehind = !bProj;
	if (PC && PC->PlayerCameraManager)
	{
		const FVector Local = FRotationMatrix(PC->PlayerCameraManager->GetCameraRotation()).GetTransposed().TransformVector(Net - PC->PlayerCameraManager->GetCameraLocation());
		if (Local.X <= 1.f)
		{
			bBehind = true;
			Sp = FVector2D(Local.Y >= 0.f ? VW : 0.f, VH * 0.5f);
		}
	}
	else if (!bProj)
	{
		Sp = FVector2D(VW * 0.5f, VH * 0.5f);
	}
	Marker.bOnScreen = !bBehind && Sp.X >= 0 && Sp.X <= VW && Sp.Y >= 0 && Sp.Y <= VH;
	if (bBehind && !PC)
	{
		Sp = FVector2D(VW * 0.5f, VH * 0.5f);
	}
	// 60 px side margins keep the distance text on screen; the top margin keeps the marker below the score and take rows.
	Marker.ScreenPos = FVector2D(FMath::Clamp(Sp.X, 60.0, VW - 60.0), FMath::Clamp(Sp.Y, 130.0, VH - 50.0));

	if (UCanvasPanelSlot* DS = Cast<UCanvasPanelSlot>(NetDiamond->Slot))
	{
		DS->SetPosition(Marker.ScreenPos);
	}
	NetText->SetText(FText::FromString(Marker.Text));
	if (UCanvasPanelSlot* TS = Cast<UCanvasPanelSlot>(NetText->Slot))
	{
		TS->SetPosition(Marker.ScreenPos + FVector2D(0, 18));
	}
}

FString UJamHudWidget::GetCardText() const { return CardTextBlock ? CardTextBlock->GetText().ToString() : FString(); }
FString UJamHudWidget::GetScoreText() const { return ScoreHome && ScoreAway ? ScoreHome->GetText().ToString() + TEXT(" : ") + ScoreAway->GetText().ToString() : FString(); }
FString UJamHudWidget::GetTimerText() const { return TimerText ? TimerText->GetText().ToString() : FString(); }
FString UJamHudWidget::GetTakeText() const { return TakeText ? TakeText->GetText().ToString() : FString(); }
