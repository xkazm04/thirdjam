#include "BarkWidget.h"
#include "MenuStyle.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Actor.h"

namespace
{
	constexpr int32 MaxBubbles = 6;
	constexpr float BubbleHeightCm = 235.f;   // above the figure's feet: the toy figure's head socket sits at 165 cm
}

float UJamBarkWidget::OpacityFor(int64 ShownFrame, int64 NowFrame)
{
	const int64 Age = NowFrame - ShownFrame;
	if (Age < 0 || ShownFrame <= -1000000) return 0.f;
	if (Age <= JamBarks::BubbleHoldFrames) return 1.f;
	const int64 Fade = Age - JamBarks::BubbleHoldFrames;
	if (Fade >= JamBarks::BubbleFadeFrames) return 0.f;
	return 1.f - static_cast<float>(Fade) / static_cast<float>(JamBarks::BubbleFadeFrames);
}

TSharedRef<SWidget> UJamBarkWidget::RebuildWidget()
{
	if (!Root) BuildTree();
	return Super::RebuildWidget();
}

void UJamBarkWidget::BuildTree()
{
	Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("BarkRoot"));
	WidgetTree->RootWidget = Root;

	auto MakeBox = [&](const TCHAR* Name, int32 FontSize, UBorder*& OutBox, UTextBlock*& OutText)
	{
		OutBox = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), Name);
		FSlateRoundedBoxBrush Brush(FLinearColor(0.96f, 0.96f, 0.94f, 0.92f), 10.f, FLinearColor(0.1f, 0.1f, 0.12f, 1.f), 2.f);
		OutBox->SetBrush(Brush);
		OutBox->SetPadding(FMargin(12, 6));
		OutText = JamMenuStyle::Text(WidgetTree, FString(), FontSize, FLinearColor(0.08f, 0.08f, 0.1f, 1.f));
		OutText->SetShadowOffset(FVector2D::ZeroVector);
		OutText->SetJustification(ETextJustify::Center);
		OutText->SetAutoWrapText(false);
		OutBox->AddChild(OutText);
		UCanvasPanelSlot* S = Root->AddChildToCanvas(OutBox);
		S->SetAutoSize(true);
		S->SetAlignment(FVector2D(0.5f, 1.f));
		S->SetPosition(FVector2D(-1000, -1000));
		OutBox->SetRenderOpacity(0.f);
	};

	Bubbles.SetNum(MaxBubbles);
	for (int32 i = 0; i < MaxBubbles; ++i)
	{
		UBorder* Box = nullptr; UTextBlock* Text = nullptr;
		MakeBox(*FString::Printf(TEXT("Bubble%d"), i), 15, Box, Text);
		Bubbles[i].Box = Box;
		Bubbles[i].Text = Text;
	}
	UBorder* Strip = nullptr; UTextBlock* StripT = nullptr;
	MakeBox(TEXT("BarkStrip"), 17, Strip, StripT);
	StripBox = Strip;
	StripText = StripT;
	FSlateRoundedBoxBrush StripBrush(FLinearColor(0.05f, 0.055f, 0.07f, 0.85f), 6.f, JamMenuStyle::Yellow(), 1.f);
	StripBox->SetBrush(StripBrush);
	StripText->SetColorAndOpacity(FSlateColor(JamMenuStyle::Chalk()));
	if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(StripBox->Slot)) S->SetAlignment(FVector2D(0.5f, 1.f));
}

void UJamBarkWidget::ShowBark(const FJamBarkPick& P, AActor* Anchor, int32 Team)
{
	if (!Root) BuildTree();
	// Reuse the bubble of the same speaker, else the oldest one.
	int32 Pick = -1;
	for (int32 i = 0; i < Bubbles.Num(); ++i) if (Bubbles[i].Speaker == P.Speaker) Pick = i;
	if (Pick < 0)
	{
		int64 Oldest = TNumericLimits<int64>::Max();
		for (int32 i = 0; i < Bubbles.Num(); ++i) if (Bubbles[i].ShownFrame < Oldest) { Oldest = Bubbles[i].ShownFrame; Pick = i; }
	}
	if (Pick < 0) return;
	FJamBarkBubble& B = Bubbles[Pick];
	B.Speaker = P.Speaker;
	B.Anchor = Anchor;
	B.ShownFrame = P.Frame;
	if (B.Text) B.Text->SetText(FText::FromString(P.Text));
	if (B.Box)
	{
		// A faint team tint on the bubble's outline.
		const FLinearColor Team0 = FLinearColor::FromSRGBColor(FColor(0x1D, 0x4E, 0xD8));
		const FLinearColor Team1 = FLinearColor::FromSRGBColor(FColor(0xE8, 0x65, 0x1A));
		FSlateRoundedBoxBrush Brush(FLinearColor(0.96f, 0.96f, 0.94f, 0.92f), 10.f, Team == 0 ? Team0 : Team1, 2.5f);
		B.Box->SetBrush(Brush);
	}
	if (StripText) StripText->SetText(FText::FromString(FString::Printf(TEXT("#%d  %s"), P.Speaker, *P.Text)));
	StripFrame = P.Frame;
	Now = FMath::Max(Now, P.Frame);
	Layout();
}

void UJamBarkWidget::TickFrames(int64 SimFrame)
{
	Now = SimFrame;
}

int32 UJamBarkWidget::GetVisibleBubbleCount() const
{
	int32 N = 0;
	for (const FJamBarkBubble& B : Bubbles) if (OpacityFor(B.ShownFrame, Now) > 0.f) ++N;
	return N;
}

FString UJamBarkWidget::GetStripText() const
{
	return StripText && OpacityFor(StripFrame, Now) > 0.f ? StripText->GetText().ToString() : FString();
}

void UJamBarkWidget::Layout()
{
	APlayerController* PC = GetOwningPlayer();
	const FVector2D VS = UWidgetLayoutLibrary::GetViewportSize(this);
	const float Scale = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), 0.01f);
	const float VW = VS.X / Scale, VH = VS.Y / Scale;
	for (FJamBarkBubble& B : Bubbles)
	{
		if (!B.Box) continue;
		const float Alpha = OpacityFor(B.ShownFrame, Now);
		B.Box->SetRenderOpacity(Alpha);
		if (Alpha <= 0.f) continue;
		UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(B.Box->Slot);
		if (!S) continue;
		FVector2D Pos(VW * 0.5f, VH - 140.f);
		bool bOn = false;
		if (PC && B.Anchor.IsValid())
		{
			FVector2D Sp;
			if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, B.Anchor->GetActorLocation() + FVector(0, 0, BubbleHeightCm), Sp, false))
			{
				bOn = Sp.X > -50.f && Sp.X < VW + 50.f && Sp.Y > 0.f && Sp.Y < VH;
				Pos = FVector2D(FMath::Clamp(Sp.X, 90.f, VW - 90.f), FMath::Clamp(Sp.Y, 60.f, VH - 60.f));
			}
		}
		// Off-screen speakers are heard through the strip only.
		B.Box->SetRenderOpacity(bOn ? Alpha : 0.f);
		S->SetPosition(Pos);
	}
	if (StripBox)
	{
		StripBox->SetRenderOpacity(OpacityFor(StripFrame, Now));
		if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(StripBox->Slot)) S->SetPosition(FVector2D(VW * 0.5f, VH - 72.f));
	}
}

void UJamBarkWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	Layout();
}
