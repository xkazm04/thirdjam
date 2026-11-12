#include "HUDWidget.h"
#include "MenuStyle.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/SizeBox.h"
#include "GameFramework/PlayerController.h"
#include "Styling/CoreStyle.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "UObject/UObjectIterator.h"
#include "HokejGameMode.h"
#include "JamEffectSystem.h"
#include "JamCartridge.h"
#include "Progression.h"

// Lane A18 (visual layer, R-0013 point 4): the slate kit of FJamUiStyle over lane A5's elements. Logic (markers, frames, feed) is A5's;
// the effects strip is lane A14's feed drawn as a slot row; the faceoff countdown reads lane A16's FaceoffCountdownFrames when it lands.
namespace
{
	using S = FJamUiStyle;

	// The feed field lane A16 adds; until it is there the countdown is driven by the preview only.
	template <typename T>
	int32 FaceoffFramesOf(const T& F)
	{
		if constexpr (requires { F.FaceoffCountdownFrames; }) return static_cast<int32>(F.FaceoffCountdownFrames);
		else return 0;
	}

	UTextBlock* MakeText(UWidgetTree* Tree, const TCHAR* Name, const FSlateFontInfo& Font, const FLinearColor& Colour)
	{
		UTextBlock* T = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		T->SetFont(Font);
		T->SetColorAndOpacity(FSlateColor(Colour));
		T->SetVisibility(ESlateVisibility::HitTestInvisible);
		return T;
	}

	UCanvasPanelSlot* Place(UCanvasPanel* Canvas, UWidget* W, FVector2D Anchor, FVector2D Align, FVector2D Pos, FVector2D Size = FVector2D::ZeroVector)
	{
		UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(W);
		Slot->SetAnchors(FAnchors(Anchor.X, Anchor.Y, Anchor.X, Anchor.Y));
		Slot->SetAlignment(Align);
		Slot->SetPosition(Pos);
		if (Size.IsZero())
		{
			Slot->SetAutoSize(true);
		}
		else
		{
			Slot->SetAutoSize(false);
			Slot->SetSize(Size);
		}
		return Slot;
	}

	// A diamond marker: a rounded box at 45 degrees with an ink outline so it reads over white ice and black void alike.
	UBorder* MakeDiamond(UWidgetTree* Tree, const TCHAR* Name, const FLinearColor& Fill, float Outline)
	{
		UBorder* B = Tree->ConstructWidget<UBorder>(UBorder::StaticClass(), Name);
		B->SetBrush(S::Rounded(Fill, 3.f, FLinearColor(0, 0, 0, 0.9f), Outline));
		B->SetRenderTransformAngle(45.f);
		B->SetRenderTransformPivot(FVector2D(0.5, 0.5));
		B->SetVisibility(ESlateVisibility::HitTestInvisible);
		return B;
	}

	// QA previews for the proof shots (jam.ui.faceoff <frames>, jam.ui.effects icon:label,icon:label).
	FAutoConsoleCommand GPreviewFaceoff(TEXT("jam.ui.faceoff"), TEXT("Preview the faceoff countdown on every HUD: jam.ui.faceoff <frames>"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const int32 Frames = Args.Num() ? FCString::Atoi(*Args[0]) : 90;
			for (TObjectIterator<UJamHudWidget> It; It; ++It) if (It->GetWorld() && !It->HasAnyFlags(RF_ClassDefaultObject)) It->PreviewFaceoff(Frames);
		}));
	FAutoConsoleCommand GPreviewEffects(TEXT("jam.ui.effects"), TEXT("Preview the effect slots on every HUD: jam.ui.effects pickup-speed:speed,pickup-heavy:heavy (empty clears)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			TArray<TPair<FString, FString>> Slots;
			TArray<FString> Items;
			if (Args.Num()) Args[0].ParseIntoArray(Items, TEXT(","));
			for (const FString& I : Items)
			{
				FString Icon, Label;
				if (!I.Split(TEXT(":"), &Icon, &Label)) { Icon = I; Label = I; }
				Slots.Emplace(Icon, Label);
			}
			for (TObjectIterator<UJamHudWidget> It; It; ++It) if (It->GetWorld() && !It->HasAnyFlags(RF_ClassDefaultObject)) It->SetEffectSlots(Slots);
		}));
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

FString UJamHudWidget::FormatTake(int32 Take, int32 ScoreToWin)
{
	if (ScoreToWin > 0)
	{
		return FString::Printf(TEXT("PERIOD %d: first to %d"), FMath::Max(1, Take), ScoreToWin);
	}
	return FString::Printf(TEXT("PERIOD %d"), FMath::Max(1, Take));
}

FString UJamHudWidget::CardFor(EJamPhase Phase)
{
	switch (Phase)
	{
	case EJamPhase::Win: return TEXT("YOU WIN");
	case EJamPhase::Lose: return TEXT("REMATCH?");
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

	// ---- Scoreboard: [home plate] [take over timer on a dark panel] [away plate], top centre.
	UHorizontalBox* Board = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Scoreboard"));
	Board->SetVisibility(ESlateVisibility::HitTestInvisible);

	HomeBox = T->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("HomeBox"));
	HomeBox->SetBrush(S::ScorePlate(FLinearColor::White));
	HomeBox->SetBrushColor(S::Blue());
	HomeBox->SetPadding(FMargin(22, 2, 22, 4));
	HomeBox->SetHorizontalAlignment(HAlign_Center);
	ScoreHome = MakeText(T, TEXT("ScoreHome"), S::ScoreFont(56), S::Paper());
	ScoreHome->SetMinDesiredWidth(64.f);
	ScoreHome->SetJustification(ETextJustify::Center);
	HomeBox->SetContent(ScoreHome);
	HomeBox->SetVisibility(ESlateVisibility::HitTestInvisible);

	AwayBox = T->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("AwayBox"));
	AwayBox->SetBrush(S::ScorePlate(FLinearColor::White));
	AwayBox->SetBrushColor(S::Orange());
	AwayBox->SetPadding(FMargin(22, 2, 22, 4));
	AwayBox->SetHorizontalAlignment(HAlign_Center);
	ScoreAway = MakeText(T, TEXT("ScoreAway"), S::ScoreFont(56), S::Paper());
	ScoreAway->SetMinDesiredWidth(64.f);
	ScoreAway->SetJustification(ETextJustify::Center);
	AwayBox->SetContent(ScoreAway);
	AwayBox->SetVisibility(ESlateVisibility::HitTestInvisible);

	// Wide side padding: the tracked capitals measure narrower than they draw, and the panel's corner tiles are 28 px.
	UBorder* Centre = S::Plate(T, S::Panel(0.9f), FMargin(34, 8, 34, 8));
	Centre->SetVisibility(ESlateVisibility::HitTestInvisible);
	UVerticalBox* CentreBox = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	TakeText = MakeText(T, TEXT("TakeText"), S::Font(S::Caption + 2, FName("Bold"), 30), S::Yellow());
	TakeText->SetJustification(ETextJustify::Center);
	S::AddV(CentreBox, TakeText, FMargin(0, 0, 0, 0), HAlign_Center);
	TimerText = MakeText(T, TEXT("TimerText"), S::ScoreFont(34, false), S::Paper());
	TimerText->SetJustification(ETextJustify::Center);
	S::AddV(CentreBox, TimerText, FMargin(0, -2, 0, 0), HAlign_Center);
	Centre->SetContent(CentreBox);

	S::AddH(Board, HomeBox, FMargin(0, 0, 6, 0), false, VAlign_Center);
	S::AddH(Board, Centre, FMargin(0), false, VAlign_Fill);
	S::AddH(Board, AwayBox, FMargin(6, 0, 0, 0), false, VAlign_Center);
	Place(Root, Board, FVector2D(0.5, 0), FVector2D(0.5, 0), FVector2D(0, 14));

	// ---- Lane A14's active effects, drawn as a slot row (icon + label each) on a panel under the scoreboard.
	EffectsPlate = S::Plate(T, S::Panel(0.8f), FMargin(12, 6));
	EffectsPlate->SetVisibility(ESlateVisibility::Collapsed);
	EffectsRow = T->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("EffectsRow"));
	EffectsPlate->SetContent(EffectsRow);
	// Under the scoreboard line's slot (lane A8's overlay pill at y 112).
	Place(Root, EffectsPlate, FVector2D(0.5, 0), FVector2D(0.5, 0), FVector2D(0, 158));
	EffectsText = MakeText(T, TEXT("EffectsText"), S::CaptionFont(true), S::Yellow());
	EffectsText->SetVisibility(ESlateVisibility::Collapsed);
	Place(Root, EffectsText, FVector2D(0.5, 0), FVector2D(0.5, 0), FVector2D(0, 158));

	// ---- Far-net marker: a take-yellow diamond with an ink edge and the distance in outlined type.
	NetDiamond = MakeDiamond(T, TEXT("NetDiamond"), S::Yellow(), 2.f);
	Place(Root, NetDiamond, FVector2D(0, 0), FVector2D(0.5, 0.5), FVector2D(0, 0), FVector2D(22, 22));
	NetText = MakeText(T, TEXT("NetText"), S::LabelFont(true), S::Paper());
	Place(Root, NetText, FVector2D(0, 0), FVector2D(0.5, 0), FVector2D(0, 0));

	// ---- Own-net marker: smaller and muted, so it never competes with the far-net one.
	OwnDiamond = MakeDiamond(T, TEXT("OwnDiamond"), S::Muted(), 1.5f);
	Place(Root, OwnDiamond, FVector2D(0, 0), FVector2D(0.5, 0.5), FVector2D(0, 0), FVector2D(12, 12));
	OwnText = MakeText(T, TEXT("OwnText"), S::CaptionFont(true), S::Muted());
	Place(Root, OwnText, FVector2D(0, 0), FVector2D(0.5, 0), FVector2D(0, 0));

	// ---- Controls hint: a bottom strip for the first 10 s, fading out over the last 2 s.
	ControlsStrip = S::Plate(T, S::Pill(0.72f, 0.f), FMargin(0, 8));
	ControlsStrip->SetVisibility(ESlateVisibility::Collapsed);
	ControlsStrip->SetHorizontalAlignment(HAlign_Center);
	ControlsText = MakeText(T, TEXT("ControlsText"), S::Font(S::Body, FName("Bold"), 40), S::Paper());
	ControlsText->SetText(FText::FromString(TEXT("MOVE  WASD / L-STICK      SHOOT  SPACE / A      CHECK  SHIFT / B      BRAKE  CTRL / LT      PAUSE  ESC")));
	ControlsText->SetJustification(ETextJustify::Center);
	ControlsStrip->SetContent(ControlsText);
	UCanvasPanelSlot* StripSlot = Root->AddChildToCanvas(ControlsStrip);
	StripSlot->SetAnchors(FAnchors(0, 1, 1, 1));
	StripSlot->SetAlignment(FVector2D(0, 1));
	StripSlot->SetOffsets(FMargin(0, -96, 0, 44));

	// ---- Radar 200 x 100 on a framed panel, bottom-right.
	Radar = T->ConstructWidget<UJamRadarWidget>(UJamRadarWidget::StaticClass(), TEXT("Radar"));
	UBorder* RadarFrame = S::Plate(T, S::Panel(0.85f), FMargin(8));
	RadarFrame->SetVisibility(ESlateVisibility::HitTestInvisible);
	USizeBox* RadarSize = T->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	RadarSize->SetWidthOverride(UJamRadarWidget::RadarWidthPx);
	RadarSize->SetHeightOverride(UJamRadarWidget::RadarHeightPx);
	RadarSize->AddChild(Radar);
	RadarFrame->SetContent(RadarSize);
	Place(Root, RadarFrame, FVector2D(1, 1), FVector2D(1, 1), FVector2D(-S::EdgePx, -S::EdgePx));

	// ---- Director's Call ready line (lane A5's; the overlay of lane A8 draws the live hint, this one stays for its feed flag).
	CallText = MakeText(T, TEXT("CallText"), S::LabelFont(true), S::Yellow());
	CallText->SetText(FText::FromString(TEXT("DIRECTOR'S CALL READY")));
	CallText->SetVisibility(ESlateVisibility::Collapsed);
	Place(Root, CallText, FVector2D(0.5, 1), FVector2D(0.5, 1), FVector2D(0, -40));

	// ---- Faceoff countdown: the chalk ring with the seconds inside, "DROP" at zero.
	UOverlay* Ring = T->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Countdown"));
	UImage* RingArt = T->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("CountdownRing"));
	RingArt->SetBrush(S::Ring(220.f));
	RingArt->SetVisibility(ESlateVisibility::HitTestInvisible);
	S::AddO(Ring, RingArt, HAlign_Center, VAlign_Center);
	CountdownText = MakeText(T, TEXT("CountdownText"), S::Font(96, FName("Black"), 0, true), S::Paper());
	CountdownText->SetJustification(ETextJustify::Center);
	S::AddO(Ring, CountdownText, HAlign_Center, VAlign_Center);
	Ring->SetVisibility(ESlateVisibility::Collapsed);
	CountdownBox = Ring;
	// Below the TAKE card's slot (0.4): at a match start lane A16's countdown and lane A8's card are up together.
	Place(Root, Ring, FVector2D(0.5, 0.66), FVector2D(0.5, 0.5), FVector2D(0, 0));

	// ---- Win / lose cards on a slate plate with a clapper stripe, and the restart hint under it.
	CardPlate = S::Plate(T, S::Panel(0.94f), FMargin(0, 0, 0, 18));
	CardPlate->SetVisibility(ESlateVisibility::Collapsed);
	UVerticalBox* CardBody = T->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	S::AddV(CardBody, S::Clapper(T, 10, 14), FMargin(0));
	CardTextBlock = MakeText(T, TEXT("CardText"), S::Font(S::Display, FName("Black"), S::TrackTitle), S::Paper());
	CardTextBlock->SetJustification(ETextJustify::Center);
	S::AddV(CardBody, CardTextBlock, FMargin(48, 14, 48, 2), HAlign_Center);
	HintText = MakeText(T, TEXT("HintText"), S::Font(S::Body, FName("Bold"), 40), S::Yellow());
	HintText->SetText(FText::FromString(TEXT("R  RESTART      ESC  PAUSE")));
	HintText->SetJustification(ETextJustify::Center);
	S::AddV(CardBody, HintText, FMargin(32, 0, 32, 0), HAlign_Center);
	CardPlate->SetContent(CardBody);
	Place(Root, CardPlate, FVector2D(0.5, 0.4), FVector2D(0.5, 0.5), FVector2D(0, 0));
}

void UJamHudWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (const UJamPresentation* P = UJamPresentation::Get(this))
	{
		if (P->HasFrame())
		{
			RefreshFromFrame(P->GetFrame());
			// Lane A14's feed: one slot per row that touches the human (icon name, effect id and the seconds left).
			const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
			if (GM && GM->GetEffects())
			{
				TArray<TPair<FString, FString>> Slots;
				for (const FJamEffectHudEntry& R : GM->GetEffects()->GetHudFeedFor(P->GetFrame().HumanIndex))
				{
					Slots.Emplace(R.Icon.ToString(), FString::Printf(TEXT("%s %.1fs"), *R.EffectId.ToString().Replace(TEXT("_"), TEXT(" ")), R.FramesLeft / 60.0));
				}
				// The feed owns the row while it has rows; an empty feed clears only what the feed put there (a preview stays).
				if (Slots.Num()) { SetEffectSlots(Slots); bEffectsFromFeed = true; }
				else if (bEffectsFromFeed) { SetEffectSlots(Slots); bEffectsFromFeed = false; }
			}
		}
	}
}

FString UJamHudWidget::FormatEffects(const TArray<FJamEffectHudEntry>& Rows)
{
	TArray<FString> Parts;
	for (const FJamEffectHudEntry& R : Rows)
	{
		Parts.Add(FString::Printf(TEXT("%s %s %.1fs"), *R.Icon.ToString().ToUpper(), *R.EffectId.ToString(), R.FramesLeft / 60.0));
	}
	return FString::Join(Parts, TEXT("    "));
}

void UJamHudWidget::SetEffectSlots(const TArray<TPair<FString, FString>>& Slots)
{
	if (!EffectsRow || !EffectsPlate || !WidgetTree) return;
	// Rebuild only when the labels changed (the seconds tick, so most frames rebuild the text only).
	const int32 N = FMath::Min(Slots.Num(), 4);
	if (EffectsRow->GetChildrenCount() != N)
	{
		EffectsRow->ClearChildren();
		for (int32 i = 0; i < N; ++i)
		{
			UHorizontalBox* Cell = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
			UImage* Icon = S::Icon(WidgetTree, Slots[i].Key, 28.f);
			S::AddH(Cell, Icon, FMargin(0, 0, 6, 0));
			UTextBlock* Label = S::Text(WidgetTree, Slots[i].Value, S::CaptionFont(), S::Paper(), false);
			Label->SetVisibility(ESlateVisibility::HitTestInvisible);
			S::AddH(Cell, Label, FMargin(0));
			S::AddH(EffectsRow, Cell, FMargin(i ? 14.f : 0.f, 0, 0, 0));
		}
	}
	else
	{
		for (int32 i = 0; i < N; ++i)
		{
			if (UHorizontalBox* Cell = Cast<UHorizontalBox>(EffectsRow->GetChildAt(i)))
			{
				if (UTextBlock* Label = Cell->GetChildrenCount() > 1 ? Cast<UTextBlock>(Cell->GetChildAt(1)) : nullptr) Label->SetText(FText::FromString(Slots[i].Value));
			}
		}
	}
	EffectsPlate->SetVisibility(N ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

FString UJamHudWidget::GetCountdownText() const
{
	return CountdownText && CountdownBox && CountdownBox->GetVisibility() != ESlateVisibility::Collapsed ? CountdownText->GetText().ToString() : FString();
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
	TakeText->SetText(FText::FromString(FormatTake(F.Take, F.ScoreToWin).ToUpper()));
	TimerText->SetText(FText::FromString(F.bHasTimer ? FormatTimer(F.FramesRemaining) : FString()));
	TimerText->SetVisibility(F.bHasTimer ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	const FString Card = CardFor(F.Phase);
	CardTextBlock->SetText(FText::FromString(Card));
	const bool bEnd = F.Phase == EJamPhase::Win || F.Phase == EJamPhase::Lose;
	CardTextBlock->SetVisibility(Card.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	// The plate shows for the end cards; lane A8's pause card covers the paused phase.
	CardPlate->SetVisibility(bEnd ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	HintText->SetVisibility(bEnd ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (bEnd)
	{
		// Lane A17: a season take's end card says what R does next (NEXT TAKE after a win, RETAKE after a loss).
		FString Hint;
		const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
		if (const UJamProgression* P = UJamProgression::Get(this); P && GM && GM->GetCartridge()) Hint = P->EndHint(GM->GetCartridge()->SourcePath);
		HintText->SetText(FText::FromString(Hint.IsEmpty() ? FString(TEXT("R  RESTART      ESC  PAUSE")) : Hint));
		CardTextBlock->SetColorAndOpacity(FSlateColor(F.Phase == EJamPhase::Win ? S::Yellow() : S::Paper()));
	}
	CallText->SetVisibility(ShowDirectorsCall(F) ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	// Controls strip: full for 8 s, fading over the last 2 s of the hint window.
	const bool bHint = ShowControlsHint(F);
	ControlsStrip->SetVisibility(bHint ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	ControlsText->SetVisibility(bHint ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (bHint)
	{
		constexpr int64 FadeFrames = 120;
		const int64 Left = ControlsHintFrames - F.SimFrame;
		ControlsStrip->SetRenderOpacity(Left >= FadeFrames ? 1.f : static_cast<float>(Left) / static_cast<float>(FadeFrames));
	}

	// Faceoff countdown: the feed's frames (lane A16) or the preview; seconds while counting, DROP for DropFrames at zero.
	int32 Frames = FaceoffFramesOf(F);
	if (FaceoffPreview > 0) { Frames = FaceoffPreview; --FaceoffPreview; }
	if (Frames <= 0 && LastFaceoffFrames > 0) DropUntil = F.SimFrame + DropFrames;
	if (F.SimFrame < LastFaceoffFrames - 1 && Frames <= 0) DropUntil = -1;   // a restart reset the frame count
	LastFaceoffFrames = Frames;
	const bool bDrop = Frames <= 0 && DropUntil >= 0 && F.SimFrame < DropUntil && F.Phase == EJamPhase::Play;
	const bool bCount = Frames > 0 && F.Phase == EJamPhase::Play;
	if (bCount)
	{
		CountdownText->SetText(FText::FromString(FString::FromInt(FMath::DivideAndRoundUp(Frames, 60))));
		CountdownText->SetColorAndOpacity(FSlateColor(S::Paper()));
		CountdownText->SetFont(S::Font(96, FName("Black"), 0, true));
	}
	else if (bDrop)
	{
		CountdownText->SetText(FText::FromString(TEXT("DROP")));
		CountdownText->SetColorAndOpacity(FSlateColor(S::Yellow()));
		CountdownText->SetFont(S::Font(S::Title + 8, FName("Black"), S::TrackTitle, true));
	}
	CountdownBox->SetVisibility(bCount || bDrop ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	Radar->SetFrame(F);
	UpdateMarker(F);
	UpdateOwnMarker(F);
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
	// 60 px side margins keep the distance text on screen; the top margin keeps the marker below the scoreboard.
	Marker.ScreenPos = FVector2D(FMath::Clamp(Sp.X, 60.0, VW - 60.0), FMath::Clamp(Sp.Y, 150.0, VH - 50.0));

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

void UJamHudWidget::UpdateOwnMarker(const FJamPresentationFrame& F)
{
	APlayerController* PC = GetOwningPlayer();
	const FVector Own(-F.AttackSign * (F.RinkLengthM * 0.5f - F.NetInsetM) * 100.f, 0.f, 150.f);
	float DistM = 0.f;
	if (F.Skaters.IsValidIndex(F.HumanIndex))
	{
		DistM = FVector2D::Distance(F.Skaters[F.HumanIndex].PosM, FVector2D(Own.X / 100.f, 0.f));
	}
	OwnMarker.DistanceM = FMath::RoundToInt32(DistM);
	OwnMarker.Text = FString::Printf(TEXT("own %d m"), OwnMarker.DistanceM);
	const FVector2D VS = UWidgetLayoutLibrary::GetViewportSize(this);
	const float Scale = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), 0.01f);
	const float VW = VS.X / Scale, VH = VS.Y / Scale;
	FVector2D Sp(VW * 0.5f, VH * 0.5f);
	bool bProj = PC && UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, Own, Sp, false);
	bool bBehind = !bProj;
	if (PC && PC->PlayerCameraManager)
	{
		const FVector Local = FRotationMatrix(PC->PlayerCameraManager->GetCameraRotation()).GetTransposed().TransformVector(Own - PC->PlayerCameraManager->GetCameraLocation());
		if (Local.X <= 1.f)
		{
			bBehind = true;
			Sp = FVector2D(Local.Y >= 0.f ? VW : 0.f, VH * 0.5f);  // behind us: park it on the side edge towards the net
		}
	}
	OwnMarker.bOnScreen = !bBehind && Sp.X >= 0 && Sp.X <= VW && Sp.Y >= 0 && Sp.Y <= VH;
	OwnMarker.ScreenPos = FVector2D(FMath::Clamp(Sp.X, 60.0, VW - 60.0), FMath::Clamp(Sp.Y, 150.0, VH - 50.0));
	if (UCanvasPanelSlot* DS = Cast<UCanvasPanelSlot>(OwnDiamond->Slot)) { DS->SetPosition(OwnMarker.ScreenPos); }
	OwnText->SetText(FText::FromString(OwnMarker.Text));
	if (UCanvasPanelSlot* TS = Cast<UCanvasPanelSlot>(OwnText->Slot)) { TS->SetPosition(OwnMarker.ScreenPos + FVector2D(0, 10)); }
}
