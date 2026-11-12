#include "BarkWidget.h"
#include "MenuStyle.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Actor.h"
#include "DirectorsCallWidget.h"
#include "UObject/UObjectIterator.h"

// Lane A18 (visual layer): the generated bubble (T_UI_Bubble, chalk paper with an ink edge and a tail at the bottom left) drawn as a
// 9-slice, mirrored when the figure is on the right half of the screen so the tail stays inside the picture; the team colour tints the
// edge through a thin second outline. Without the art: a rounded chalk box with a code-drawn tail. Timing is lane A12's.
namespace
{
	using S = FJamUiStyle;
	constexpr int32 MaxBubbles = 6;
	constexpr float BubbleHeightCm = 235.f;   // above the figure's feet: the toy figure's head socket sits at 165 cm
	constexpr float TailPx = 14.f;            // the fallback tail square's side; it is drawn at 45 degrees
	constexpr float TailInsetPx = 22.f;       // fallback tail centre from the bubble's near edge
	constexpr float ArtScale = 0.2f;          // the bubble texture is drawn at a fifth of its pixels (corner tiles 60 x 40 px)

	// The generated bubble (job bubble-left) is 2:1 with the tail in its bottom-left corner tile: at one-line text size the tile
	// band swallows the body, so the chalk box below ships and the art is not imported (docs/experiments/UI.md). Kept as the switch.
	constexpr bool bUseBubbleArt = false;
	bool BubbleArt(FSlateBrush& Out)
	{
		UTexture2D* Tex = bUseBubbleArt ? S::Art(TEXT("T_UI_Bubble")) : nullptr;
		if (!Tex) return false;
		// Left and bottom margins hold the tail; the body stretches between.
		Out = S::BoxBrush(Tex, FMargin(0.17f, 0.12f, 0.10f, 0.22f));
		Out.ImageSize = Out.ImageSize * ArtScale;
		return true;
	}
	// The fallback body: chalk paper with the team colour on the edge.
	FSlateBrush BubbleBrush(const FLinearColor& Edge)
	{
		return S::Rounded(S::Paper(), 12.f, Edge, 2.5f);
	}
	// The art's padding: the text sits in the body above the tail band (bottom 22 % of the drawn height).
	FMargin ArtPadding(bool bTailLeft)
	{
		return bTailLeft ? FMargin(24, 10, 18, 44) : FMargin(18, 10, 24, 44);
	}
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

FVector2D UJamBarkWidget::PushOutOfBand(FVector2D Pos, float Height, float Top, float Bottom, float MinY)
{
	if (Bottom <= Top) return Pos;
	const float BoxTop = Pos.Y - Height, BoxBottom = Pos.Y;
	if (BoxBottom <= Top || BoxTop >= Bottom) return Pos;   // clear already
	// Above the band when the bubble still fits under the scoreboard, else below it.
	if (Top - 4.f - Height >= MinY) Pos.Y = Top - 4.f;
	else Pos.Y = Bottom + 4.f + Height;
	return Pos;
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
	FSlateBrush Art;
	const bool bArt = BubbleArt(Art);

	// A bubble: the outer box (the canvas slot) holds an overlay of the body plate (filling), the text (padded) and the fallback tail.
	auto MakeBubble = [&](const TCHAR* Name, FJamBarkBubble& Out)
	{
		Out.Box = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), Name);
		Out.Box->SetBrushColor(FLinearColor::Transparent);
		Out.Box->SetPadding(FMargin(0));
		UOverlay* Stack = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
		Out.Body = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		Out.Body->SetBrush(bArt ? Art : BubbleBrush(S::Ink()));
		Out.Body->SetPadding(FMargin(0));
		Out.Body->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
		Out.Body->SetVisibility(ESlateVisibility::HitTestInvisible);
		S::AddO(Stack, Out.Body);
		Out.Text = S::Text(WidgetTree, FString(), S::Font(S::Body, FName("Bold")), S::Ink(), false);
		Out.Text->SetJustification(ETextJustify::Center);
		Out.Text->SetAutoWrapText(false);
		Out.Text->SetVisibility(ESlateVisibility::HitTestInvisible);
		S::AddO(Stack, Out.Text, HAlign_Fill, VAlign_Fill, bArt ? ArtPadding(true) : FMargin(14, 7, 14, 7 + TailPx));
		Out.Tail = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		Out.Tail->SetBrush(S::Rounded(S::Paper(), 2.f, S::Ink(), 2.f));
		Out.Tail->SetPadding(FMargin(TailPx * 0.5f));
		Out.Tail->SetRenderTransformAngle(45.f);
		Out.Tail->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
		Out.Tail->SetVisibility(bArt ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
		S::AddO(Stack, Out.Tail, HAlign_Left, VAlign_Bottom, FMargin(TailInsetPx, 0, 0, 2));
		Out.Box->AddChild(Stack);
		UCanvasPanelSlot* CS = Root->AddChildToCanvas(Out.Box);
		CS->SetAutoSize(true);
		CS->SetAlignment(FVector2D(0.5f, 1.f));
		CS->SetPosition(FVector2D(-1000, -1000));
		Out.Box->SetRenderOpacity(0.f);
		Out.Box->SetVisibility(ESlateVisibility::Collapsed);
	};

	Bubbles.SetNum(MaxBubbles);
	for (int32 i = 0; i < MaxBubbles; ++i) MakeBubble(*FString::Printf(TEXT("Bubble%d"), i), Bubbles[i]);

	// The strip: lane A12's dark pill at the bottom. Lane A42: barks stay in their bubbles and the commentator's line is the HUD's strip
	// (UJamHudWidget), so this one is built but never shown; kept so GetStripText and the layout keep their shape.
	StripBox = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BarkStrip"));
	StripBox->SetBrush(S::Pill(0.82f));
	StripBox->SetPadding(FMargin(18, 7));
	StripText = S::Text(WidgetTree, FString(), S::Font(S::Body, FName("Bold")), S::Paper(), false);
	StripText->SetJustification(ETextJustify::Center);
	StripText->SetAutoWrapText(false);
	StripBox->AddChild(StripText);
	UCanvasPanelSlot* SS = Root->AddChildToCanvas(StripBox);
	SS->SetAutoSize(true);
	SS->SetAlignment(FVector2D(0.5f, 1.f));
	SS->SetPosition(FVector2D(-1000, -1000));
	StripBox->SetRenderOpacity(0.f);
	StripBox->SetVisibility(ESlateVisibility::Collapsed);
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
	// The team colour: on the art a tint of the edge (the art's ink edge multiplies towards the team colour), on the fallback the outline.
	const FLinearColor Edge = Team == 0 ? S::Blue() : S::Orange();
	FSlateBrush Art;
	if (B.Body)
	{
		if (BubbleArt(Art)) B.Body->SetBrushColor(FLinearColor(1.f, 1.f, 1.f, 1.f));
		else B.Body->SetBrush(BubbleBrush(Edge));
	}
	if (B.Tail) B.Tail->SetBrush(S::Rounded(S::Paper(), 2.f, Edge, 2.f));
	if (B.Text) B.Text->SetColorAndOpacity(FSlateColor(Team == 0 ? FLinearColor::FromSRGBColor(FColor(0x12, 0x2E, 0x7A)) : FLinearColor::FromSRGBColor(FColor(0x7A, 0x33, 0x0C))));
	// Lane A42: the bark shows in its bubble only; the commentator's line is the HUD's strip (UJamHudWidget, always on screen), so this
	// widget's strip stays dark (StripFrame never set).
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
	FSlateBrush Art;
	const bool bArt = BubbleArt(Art);
	// Lane A42: the y-slot the MATCH banner holds (the overlay is found once and remembered; a new one is looked for every 60 ticks).
	if (!Overlay.IsValid() && (OverlaySearchTick++ % 60) == 0)
	{
		for (TObjectIterator<UJamDirectorsCallHudWidget> It; It; ++It) if (It->GetWorld() == GetWorld() && !It->HasAnyFlags(RF_ClassDefaultObject)) { Overlay = *It; break; }
	}
	float BandTop = 0.f, BandBottom = 0.f;
	const bool bBand = Overlay.IsValid() && Overlay->GetBannerBandPx(VH, BandTop, BandBottom);
	for (FJamBarkBubble& B : Bubbles)
	{
		if (!B.Box) continue;
		const float Alpha = OpacityFor(B.ShownFrame, Now);
		B.Box->SetRenderOpacity(Alpha);
		// Render opacity does not hide a rounded-box outline: collapse the bubble when it is not showing.
		B.Box->SetVisibility(Alpha > 0.f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		if (Alpha <= 0.f) continue;
		UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(B.Box->Slot);
		if (!CS) continue;
		FVector2D Pos(VW * 0.5f, VH - 140.f);
		bool bOn = false;
		bool bTailLeft = true;
		if (PC && B.Anchor.IsValid())
		{
			FVector2D Sp;
			if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, B.Anchor->GetActorLocation() + FVector(0, 0, BubbleHeightCm), Sp, false))
			{
				bOn = Sp.X > -50.f && Sp.X < VW + 50.f && Sp.Y > 0.f && Sp.Y < VH;
				// The tail sits at the figure's x; the bubble shifts aside so the tail is on its left or right end.
				bTailLeft = Sp.X <= VW * 0.5f;
				const float Shift = bTailLeft ? 60.f : -60.f;
				// Lane A42: the whole bubble stays on screen (it was clamped by its centre and cut at the edge).
				const float HalfW = FMath::Max(120.f, static_cast<float>(B.Box->GetDesiredSize().X) * 0.5f + 16.f);
				Pos = FVector2D(FMath::Clamp(Sp.X + Shift, HalfW, VW - HalfW), FMath::Clamp(Sp.Y, 170.f, VH - 60.f));
				// Lane A42: never over the MATCH banner (the bubble is about 54 px tall: text, padding and the tail).
				if (bBand) Pos = PushOutOfBand(Pos, FMath::Max(54.f, static_cast<float>(B.Box->GetDesiredSize().Y)), BandTop, BandBottom, 170.f);
			}
		}
		// Off-screen speakers are heard, not seen (the strip is the commentator's).
		B.Box->SetRenderOpacity(bOn ? Alpha : 0.f);
		if (!bOn) B.Box->SetVisibility(ESlateVisibility::Collapsed);
		CS->SetPosition(Pos);
		// The tail side: the art mirrors, the fallback tail moves.
		if (B.Body && bArt) B.Body->SetRenderScale(FVector2D(bTailLeft ? 1.f : -1.f, 1.f));
		if (B.Text) if (UOverlaySlot* TS = Cast<UOverlaySlot>(B.Text->Slot)) TS->SetPadding(bArt ? ArtPadding(bTailLeft) : FMargin(14, 7, 14, 7 + TailPx));
		if (B.Tail && !bArt)
		{
			if (UOverlaySlot* TS = Cast<UOverlaySlot>(B.Tail->Slot))
			{
				TS->SetHorizontalAlignment(bTailLeft ? HAlign_Left : HAlign_Right);
				TS->SetPadding(bTailLeft ? FMargin(TailInsetPx, 0, 0, 2) : FMargin(0, 0, TailInsetPx, 2));
			}
		}
	}
	if (StripBox)
	{
		// Lane A42: dark unless a caller sets StripFrame (nobody does now: the HUD's strip carries the commentator).
		const float StripAlpha = OpacityFor(StripFrame, Now);
		StripBox->SetRenderOpacity(StripAlpha);
		StripBox->SetVisibility(StripAlpha > 0.f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(StripBox->Slot)) CS->SetPosition(FVector2D(VW * 0.5f, VH - 110.f));
	}
}

void UJamBarkWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	Layout();
}
