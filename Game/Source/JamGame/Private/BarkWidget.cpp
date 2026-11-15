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
#include "HUDFeed.h"
#include "HUDWidget.h"
#include "Camera/PlayerCameraManager.h"
#include "UObject/UObjectIterator.h"
#include "HAL/IConsoleManager.h"

// Lane A135 (track U1, evolution 3): bark text on a fixed lane beside the scoreboard (UJamBarkWidget::LanePos). 0 = the bubble over its speaker, as before.
static TAutoConsoleVariable<int32> CVarJamHudBarkLane(TEXT("jam.hud.BarkLane"), 1,
	TEXT("1 = a bark's text stands in the scoreboard's row beside its team's score plate (no bubble on a figure, a net label or by the human's skates); 0 = the bubble over its speaker, pushed out of the middle third (the screen of before). Presentation only."), ECVF_Default);

bool UJamBarkWidget::LaneOn() { return CVarJamHudBarkLane.GetValueOnGameThread() != 0; }

bool UJamBarkWidget::LaneMeetsFigure(const FVector2D& Min, const FVector2D& Max, const FVector2D& FeetPx, const FVector2D& HeadPx, float Pad)
{
	const double HalfW = FMath::Max(12.0, 0.5 * FVector2D::Distance(FeetPx, HeadPx));
	const double L = FMath::Min(FeetPx.X, HeadPx.X) - HalfW, R = FMath::Max(FeetPx.X, HeadPx.X) + HalfW;
	const double T = FMath::Min(FeetPx.Y, HeadPx.Y), B = FMath::Max(FeetPx.Y, HeadPx.Y);
	return !(R < Min.X - Pad || L > Max.X + Pad || B < Min.Y - Pad || T > Max.Y + Pad);
}

FVector2D UJamBarkWidget::LanePos(int32 Team, float HalfW, float VW)
{
	// The near end stands LaneGapX from the centre; a bubble too long for its side keeps its far end LaneEdgePx inside the frame (and leans on the board).
	const float X = Team == 0 ? FMath::Max(VW * 0.5f - LaneGapX - HalfW, LaneEdgePx + HalfW) : FMath::Min(VW * 0.5f + LaneGapX + HalfW, VW - LaneEdgePx - HalfW);
	return FVector2D(X, LaneBottomY);
}

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

FVector2D UJamBarkWidget::PushOutOfMiddle(FVector2D Pos, float HalfW, float Height, float VW, float VH, float Pad)
{
	const float L = VW / 3.f - Pad, R = VW * 2.f / 3.f + Pad, T = VH / 3.f - Pad, B = VH * 2.f / 3.f + Pad;
	const float MinX = Pos.X - HalfW, MaxX = Pos.X + HalfW, MinY = Pos.Y - Height, MaxY = Pos.Y;
	if (MaxX <= L || MinX >= R || MaxY <= T || MinY >= B) return Pos;   // clear already
	// The four ways out and how far each is; a sideways move counts only while the whole bubble stays in the frame.
	const float Up = MaxY - T, Down = B - MinY, Left = MaxX - L, Right = R - MinX;
	const bool bLeftFits = MinX - Left >= 0.f, bRightFits = MaxX + Right <= VW;
	float Best = FMath::Min(Up, Down);
	if (bLeftFits) Best = FMath::Min(Best, Left);
	if (bRightFits) Best = FMath::Min(Best, Right);
	if (Best == Up) Pos.Y -= Up;
	else if (Best == Down) Pos.Y += Down;
	else if (bLeftFits && Best == Left) Pos.X -= Left;
	else Pos.X += Right;
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
	B.Team = Team;
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
	if (!BubbleArt(Art)) OneLookBubble(B, Edge);   // lane A229 (HUDOneLook.cpp)
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
	// Lane A42 (A48's storyboard, still 9): the pause card clears every bubble; nothing comes back on resume. The pause menu pauses the
	// world (UGameplayStatics::SetGamePaused), so the world's flag is the signal; the feed's Paused phase is honoured too.
	const UJamPresentation* P = UJamPresentation::Get(this);
	if ((GetWorld() && GetWorld()->IsPaused()) || (P && P->HasFrame() && P->GetFrame().Phase == EJamPhase::Paused))
	{
		for (FJamBarkBubble& B : Bubbles) { B.ShownFrame = -1000000; if (B.Box) B.Box->SetVisibility(ESlateVisibility::Collapsed); }
	}
	const FVector CamLoc = PC && PC->PlayerCameraManager ? PC->PlayerCameraManager->GetCameraLocation() : FVector::ZeroVector;
	// Lane A135: the lane holds one bubble a side, the newest of a team whose speaker is in the frame and near (the rule of below).
	const bool bLane = LaneOn();
	const FJamBarkBubble* LaneOwner[2] = { nullptr, nullptr };
	if (bLane && PC)
	{
		for (const FJamBarkBubble& B : Bubbles)
		{
			if (!B.Box || !B.Anchor.IsValid() || OpacityFor(B.ShownFrame, Now) <= 0.f) continue;
			FVector2D Sp;
			if (PC->PlayerCameraManager && FVector::Dist(B.Anchor->GetActorLocation(), CamLoc) > MaxBubbleDistanceM * 100.f) continue;
			if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, B.Anchor->GetActorLocation() + FVector(0, 0, BubbleHeightCm), Sp, false)) continue;
			if (!(Sp.X > 0.f && Sp.X < VW && Sp.Y > 0.f && Sp.Y < VH)) continue;
			const int32 Side = B.Team == 0 ? 0 : 1;
			if (!LaneOwner[Side] || B.ShownFrame > LaneOwner[Side]->ShownFrame) LaneOwner[Side] = &B;
		}
	}
	// Lane A135: is a figure or the puck under a lane box this frame (every skater of the presentation frame, feet to 1.8 m; the puck's ring)?
	auto LaneCovered = [&](const FVector2D& Min, const FVector2D& Max) -> bool
	{
		if (!PC || !P || !P->HasFrame()) return false;
		// Lane A158: the HUD's boxes of this tick: the human's figure and every downed figure by their bones (a lying figure is not the standing box below),
		// and both nets (the critic's fault 3: a bubble over the net).
		TArray<FBox2D> KeepFigures, KeepNets;
		if (UJamHudWidget::GetKeepClear(KeepFigures, KeepNets) && (UJamHudWidget::BoxMeetsAny(Min, Max, KeepFigures) || UJamHudWidget::BoxMeetsAny(Min, Max, KeepNets))) return true;
		const FJamPresentationFrame& F = P->GetFrame();
		for (const FJamPresentationSkater& Sk : F.Skaters)
		{
			const FVector W(Sk.PosM.X * 100.0, Sk.PosM.Y * 100.0, 0.0);
			FVector2D Feet, Head;
			if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, W, Feet, false)) continue;
			if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, W + FVector(0, 0, 180), Head, false)) continue;
			if (LaneMeetsFigure(Min, Max, Feet, Head)) return true;
		}
		FVector2D Puck;
		if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, FVector(F.PuckM.X * 100.0, F.PuckM.Y * 100.0, 0.0), Puck, false)
			&& LaneMeetsFigure(Min, Max, Puck + FVector2D(0, 14), Puck - FVector2D(0, 14))) return true;
		return false;
	};
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
		bool bTail = true;   // lane A123: a bubble moved under or beside its speaker has no tail (it would point away)
		if (PC && B.Anchor.IsValid())
		{
			FVector2D Sp;
			// A48's storyboard (findings 5 and 9): a speaker beyond MaxBubbleDistanceM from the camera or outside the frame gets no bubble
			// (it floated at the top of the frame over the far net); the voice still plays.
			const bool bNear = PC->PlayerCameraManager ? FVector::Dist(B.Anchor->GetActorLocation(), CamLoc) <= MaxBubbleDistanceM * 100.f : true;
			if (bNear && UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, B.Anchor->GetActorLocation() + FVector(0, 0, BubbleHeightCm), Sp, false))
			{
				bOn = Sp.X > 0.f && Sp.X < VW && Sp.Y > 0.f && Sp.Y < VH;
				// The tail sits at the figure's x; the bubble shifts aside so the tail is on its left or right end.
				bTailLeft = Sp.X <= VW * 0.5f;
				const float Shift = bTailLeft ? 60.f : -60.f;
				// Lane A42: the whole bubble stays on screen (it was clamped by its centre and cut at the edge).
				const float HalfW = FMath::Max(120.f, static_cast<float>(B.Box->GetDesiredSize().X) * 0.5f + 16.f);
				Pos = FVector2D(FMath::Clamp(Sp.X + Shift, HalfW, VW - HalfW), FMath::Clamp(Sp.Y, 170.f, VH - 60.f));
				// Clear of the radar frame (bottom right, 264 x 140 from the edge) and the ready pill (bottom left).
				if (Pos.X + HalfW > VW - 300.f || Pos.X - HalfW < 420.f) Pos.Y = FMath::Min(Pos.Y, VH - 180.f);
				// Lane A42: never over the MATCH banner (the bubble is about 54 px tall: text, padding and the tail).
				if (bBand) Pos = PushOutOfBand(Pos, FMath::Max(54.f, static_cast<float>(B.Box->GetDesiredSize().Y)), BandTop, BandBottom, 170.f);
				// Lane A123: never over the HUD's one-time prompt (its slot under the scoreboard), and never over the middle third of the frame.
				const float BubH = FMath::Max(54.f, static_cast<float>(B.Box->GetDesiredSize().Y));
				float PromptTop = 0.f, PromptBottom = 0.f;
				if (UJamHudWidget::GetPromptBandPx(PromptTop, PromptBottom)) Pos = PushOutOfBand(Pos, BubH, PromptTop, PromptBottom, 170.f);
				if (UJamHudWidget::ClearMiddleOn())
				{
					const FVector2D Before = Pos;
					Pos = PushOutOfMiddle(Pos, FMath::Max(60.f, static_cast<float>(B.Box->GetDesiredSize().X) * 0.5f), BubH, VW, VH);
					bTail = Pos.Y <= Before.Y + 1.f && FMath::Abs(Pos.X - Before.X) < 1.f;
				}
				// Lane A135: the fixed lane beside the team's score plate; no tail; one bubble a side.
				if (bLane)
				{
					const float LaneHalfW = FMath::Max(60.f, static_cast<float>(B.Box->GetDesiredSize().X) * 0.5f);
					const float LaneH = FMath::Max(40.f, static_cast<float>(B.Box->GetDesiredSize().Y));
					Pos = LanePos(B.Team, LaneHalfW, VW);
					bTail = false;
					if (LaneOwner[B.Team == 0 ? 0 : 1] != &B) bOn = false;
					if (bOn && LaneCovered(FVector2D(Pos.X - LaneHalfW, Pos.Y - LaneH), FVector2D(Pos.X + LaneHalfW, Pos.Y))) bOn = false;   // the lane yields to a figure under it
				}
			}
		}
		// Lane A213 (wave 2, the Director's ruling of 01:36): a bubble yields to the HUD's one line of words (jam.hud.OneLine); the voice still plays.
		if (bOn && UJamHudWidget::OneLineUpNow()) bOn = false;
		// Off-screen speakers are heard, not seen (the strip is the commentator's).
		B.Box->SetRenderOpacity(bOn ? Alpha : 0.f);
		if (!bOn) B.Box->SetVisibility(ESlateVisibility::Collapsed);
		CS->SetPosition(Pos);
		// The tail side: the art mirrors, the fallback tail moves.
		if (B.Body && bArt) B.Body->SetRenderScale(FVector2D(bTailLeft ? 1.f : -1.f, 1.f));
		if (B.Text) if (UOverlaySlot* TS = Cast<UOverlaySlot>(B.Text->Slot)) TS->SetPadding(bArt ? ArtPadding(bTailLeft) : FMargin(14, 7, 14, 7 + ((bLane && !bTail) ? 0.f : TailPx)));
		if (B.Tail && !bArt)
		{
			B.Tail->SetVisibility(bTail ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
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
