// Lane A229 (track U5, the HUD round of 15 Nov; the operator: "one more round of HUD UI/UX improvement"). The match HUD was of several hands: the
// scoreboard on chalk-edged rounded plates in outlined Roboto, the strips on round black pills, the labels in a third and fourth size, the minimap on the
// kit's chalk panel, the end card's stats as four centred lines. One look now, the controls panel's (lane A222), which is the front end's (FJamMenuLook):
// the menu's panel fill with its 4-unit corner and hairline edge, the menu's type (device B's display face when it is in the plugin), two label sizes
// (HudLabelPt, HudLinePt) and one value size (HudValuePt); team colours only on what belongs to a team (the score plates, the bark's edge).
// Presentation only: widgets' brushes, fonts, paddings and colours, set once where each is built; nothing on or near a skater (R-0069, R-0070), nothing in
// the controls panel's rows, no position of a mark. jam.hud.OneLook 0 (read when a HUD is built) gives back the HUD of before.
#include "HUDWidget.h"
#include "MenuStyle.h"
#include "MenuWidget.h"
#include "DirectorsCallWidget.h"
#include "BarkWidget.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "HAL/IConsoleManager.h"
#include "Engine/World.h"
#include "UObject/UObjectIterator.h"

static TAutoConsoleVariable<int32> CVarJamHudOneLook(TEXT("jam.hud.OneLook"), 1,
	TEXT("Lane A229: 1 (default) the match HUD in the controls panel's look (the menu's panel, corner, edge and type; two label sizes and one value size; the end card's stats as a table; the GOAL banner without the score under it). 0 = the HUD of before. Read when a HUD is built."), ECVF_Default);

bool UJamHudWidget::OneLookOn() { return CVarJamHudOneLook.GetValueOnGameThread() != 0; }

namespace JamOneLook
{
	using LK = FJamMenuLook;
	FLinearColor OLFill(float A) { FLinearColor C = LK::Panel(); C.A = A; return C; }
	// The controls panel's plate: the menu's fill at A, its 4-unit corner, the white hairline edge.
	FSlateBrush OLPanelBrush(float A) { return LK::FillBrush(JamOneLook::OLFill(A), true); }
	void OLPlate(UBorder* B, float A, const FMargin& Pad)
	{
		if (!B) return;
		B->SetBrush(JamOneLook::OLPanelBrush(A));
		B->SetPadding(Pad);
	}
	void OLType(UTextBlock* T, int32 Pt, FName Face, int32 Track, const FLinearColor& C)
	{
		if (!T) return;
		T->SetFont(LK::Font(Pt, Face, Track));
		T->SetColorAndOpacity(FSlateColor(C));
		T->SetShadowOffset(FVector2D(0, 1));
		T->SetShadowColorAndOpacity(FLinearColor(0, 0, 0, 0.45f));
	}
	UTextBlock* OLTextIn(UBorder* B) { return B ? Cast<UTextBlock>(B->GetContent()) : nullptr; }
}

void UJamHudWidget::ApplyOneLook()
{
	if (!OneLookOn() || !WidgetTree) return;
	using L = FJamMenuLook;
	// ---- The scoreboard: the team plates in the menu's corner and edge (their colour stays: they belong to the teams), the digits in the menu's
	// condensed face at the value size; the centre plate on the panel fill, PERIOD n small and dim, the rule or the clock under it.
	for (int32 Side = 0; Side < 2; ++Side)
	{
		UBorder* Box = Side == 0 ? HomeBox.Get() : AwayBox.Get();
		if (!Box) continue;
		Box->SetBrush(L::FillBrush(FLinearColor::White, true));
		Box->SetBrushColor(Side == 0 ? FJamUiStyle::Blue() : FJamUiStyle::Orange());   // RefreshFromFrame paints the match's team colours every frame
		Box->SetPadding(FMargin(18, 2, 18, 2));
		Box->SetVerticalAlignment(VAlign_Center);
		if (UHorizontalBoxSlot* HS = Cast<UHorizontalBoxSlot>(Box->Slot)) HS->SetVerticalAlignment(VAlign_Fill);
	}
	JamOneLook::OLType(ScoreHome, HudValuePt, FName("Bold"), 0, L::Text());
	JamOneLook::OLType(ScoreAway, HudValuePt, FName("Bold"), 0, L::Text());
	if (ScoreHome) ScoreHome->SetMinDesiredWidth(56.f);
	if (ScoreAway) ScoreAway->SetMinDesiredWidth(56.f);
	JamOneLook::OLPlate(CentrePlate, 0.9f, FMargin(24, 6, 24, 6));
	JamOneLook::OLType(TakeText, HudLabelPt, FName("Bold"), L::TrackLabel, L::TextDim());
	JamOneLook::OLType(GoalRuleText, HudLinePt, FName("Bold"), L::TrackLabel, L::Text());
	JamOneLook::OLType(TimerText, HudValuePt, FName("Bold"), 0, L::Text());
	// ---- The strips and pills: one panel; the commentary quiet (dim, regular), what asks for a press in the HUD's yellow, in capitals.
	JamOneLook::OLPlate(CommentaryStrip, 0.72f, FMargin(16, 6));
	JamOneLook::OLType(CommentaryText, HudLinePt, FName("Regular"), 0, L::TextDim());
	JamOneLook::OLPlate(FaceoffHintPlate, 0.86f, FMargin(16, 6));
	if (UTextBlock* Hint = JamOneLook::OLTextIn(FaceoffHintPlate))
	{
		JamOneLook::OLType(Hint, HudLinePt, FName("Bold"), L::TrackLabel, L::Text());
		Hint->SetText(FText::FromString(FaceoffHintText().ToUpper()));
	}
	JamOneLook::OLPlate(ContactPlate, 0.86f, FMargin(16, 6));
	JamOneLook::OLType(ContactText, HudLinePt, FName("Bold"), L::TrackLabel, FJamUiStyle::Yellow());
	JamOneLook::OLPlate(PromptPlate, 0.86f, FMargin(20, 8));
	JamOneLook::OLType(PromptTextBlock, HudLinePt, FName("Bold"), L::TrackLabel, FJamUiStyle::Yellow());
	JamOneLook::OLPlate(ToastPlate, 0.86f, FMargin(16, 6));
	JamOneLook::OLType(ToastText, HudLinePt, FName("Bold"), 0, FJamUiStyle::Yellow());
	// ---- The distance labels (far net, own net, the puck off screen): the panel's fill and corner behind a line-size label (was a third size, outlined).
	for (UBorder* B : { NetPlate.Get(), OwnPlate.Get(), PuckPlate.Get() }) if (B) B->SetBrush(L::FillBrush(JamOneLook::OLFill(LabelPlateOpacity + 0.18f), false));
	JamOneLook::OLType(NetText, HudLinePt, FName("Bold"), 0, L::Text());
	JamOneLook::OLType(OwnText, HudLabelPt, FName("Bold"), 0, L::TextDim());
	JamOneLook::OLType(PuckText, HudLinePt, FName("Bold"), 0, L::Text());
	// ---- The YOU tag (shown at a faceoff's count only): its word in the menu's face at the line size (was 22, outlined); its plate takes the menu's
	// corner where UpdateYours paints it (YouTagBrush).
	JamOneLook::OLType(YouTagText, HudLinePt, FName("Black"), L::TrackLabel, L::Text());
	// ---- Wave 2 (lane A227's plates, landed 18:10): IN THE BOX n and the effect slot on the same panel, their words at the line size; the box keeps
	// A227's fixed 300 x 44 (the tracked 18 measures well inside it: Jam.Hud.OneLookPlates).
	JamOneLook::OLPlate(BoxPlate, 0.86f, FMargin(14, 6));
	JamOneLook::OLType(BoxText, HudLinePt, FName("Bold"), L::TrackLabel, L::Text());
	JamOneLook::OLPlate(EffectsPlate, 0.86f, FMargin(12, 6));
	// ---- The ability slot and the minimap's frame: the same panel.
	JamOneLook::OLPlate(AbilityPlate, 0.86f, FMargin(12, 6, 12, 10));
	JamOneLook::OLType(AbilityText, HudLinePt, FName("Bold"), L::TrackLabel, L::Text());
	if (Radar)
		if (UWidget* Size = Radar->GetParent())
			if (UBorder* Frame = Cast<UBorder>(Size->GetParent())) JamOneLook::OLPlate(Frame, 0.82f, FMargin(6));
}

void UJamHudWidget::ApplyOneLookGoalBanner()
{
	// The score hung under the banner's foot, half on the art and half on the ice (h43-04, the before still): the scoreboard above already says it,
	// so the banner is the one word, large and brief. The score's text stays in the layout (GetGoalBannerText, the tests) and is not drawn.
	if (!OneLookOn() || !GoalBannerScoreText) return;
	GoalBannerScoreText->SetRenderOpacity(0.f);
}

TArray<TPair<FString, FString>> UJamHudWidget::EndStatsRows(const FEndStats& S)
{
	const int64 Sec = S.CarryFrames / 60;
	return {
		{ TEXT("SHOTS"), FString::FromInt(S.Shots) },
		{ TEXT("GOALS"), FString::FromInt(S.Goals) },
		{ TEXT("PASSES THAT ARRIVED"), FString::Printf(TEXT("%d OF %d"), S.Arrived, S.Passes) },
		{ TEXT("HITS LANDED"), FString::FromInt(S.Hits) },
		{ TEXT("WITH THE PUCK"), FString::Printf(TEXT("%lld:%02lld"), Sec / 60, Sec % 60) },
	};
}

void UJamHudWidget::BuildEndStatsTable(UVerticalBox* Holder)
{
	// The end card's table: a label column (dim) and a value column (bright, right-aligned), a hairline between rows, 480 units wide in the card's 640.
	if (!Holder || !WidgetTree || EndStatsTable) return;
	using L = FJamMenuLook;
	EndStatsTable = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("EndStatsTable"));
	EndStatsTable->SetVisibility(ESlateVisibility::Collapsed);
	const TArray<TPair<FString, FString>> Rows = EndStatsRows(FEndStats());
	for (int32 I = 0; I < Rows.Num(); ++I)
	{
		if (I > 0)
		{
			UBorder* Hair = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
			Hair->SetBrushColor(L::PanelEdge());
			USizeBox* HB = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
			HB->SetHeightOverride(1.f);
			HB->AddChild(Hair);
			if (UVerticalBoxSlot* VS = EndStatsTable->AddChildToVerticalBox(HB)) VS->SetHorizontalAlignment(HAlign_Fill);
		}
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UTextBlock* Label = L::MakeText(WidgetTree, Rows[I].Key, L::Font(HudLinePt, FName("Bold"), L::TrackLabel), L::TextDim());
		UTextBlock* Value = L::MakeText(WidgetTree, Rows[I].Value, L::Font(HudLinePt, FName("Black"), L::TrackLabel), L::Text());
		Value->SetJustification(ETextJustify::Right);
		if (UHorizontalBoxSlot* HS = Row->AddChildToHorizontalBox(Label)) { HS->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); HS->SetVerticalAlignment(VAlign_Center); }
		if (UHorizontalBoxSlot* HS = Row->AddChildToHorizontalBox(Value)) { HS->SetHorizontalAlignment(HAlign_Right); HS->SetVerticalAlignment(VAlign_Center); HS->SetPadding(FMargin(L::Unit * 3.f, 0, 0, 0)); }
		if (UVerticalBoxSlot* VS = EndStatsTable->AddChildToVerticalBox(Row)) { VS->SetHorizontalAlignment(HAlign_Fill); VS->SetPadding(FMargin(0, 5)); }
		EndStatsLabels.Add(Label);
		EndStatsValues.Add(Value);
	}
	USizeBox* Width = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("EndStatsTableWidth"));
	Width->SetWidthOverride(480.f);
	Width->AddChild(EndStatsTable);
	if (UVerticalBoxSlot* VS = Holder->AddChildToVerticalBox(Width)) VS->SetHorizontalAlignment(HAlign_Center);
}

void UJamHudWidget::ShowEndStatsTable(bool bShow)
{
	if (!EndStatsTable) return;
	if (bShow)
	{
		const TArray<TPair<FString, FString>> Rows = EndStatsRows(EndStatsNow);
		for (int32 I = 0; I < Rows.Num() && I < EndStatsValues.Num(); ++I)
			if (EndStatsValues[I] && EndStatsValues[I]->GetText().ToString() != Rows[I].Value) EndStatsValues[I]->SetText(FText::FromString(Rows[I].Value));
	}
	const ESlateVisibility V = bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
	if (EndStatsTable->GetVisibility() != V) EndStatsTable->SetVisibility(V);
}

FString UJamHudWidget::GetEndStatsTableText() const
{
	if (!EndStatsTable || EndStatsTable->GetVisibility() == ESlateVisibility::Collapsed) return FString();
	FString Out;
	for (int32 I = 0; I < EndStatsLabels.Num() && I < EndStatsValues.Num(); ++I)
	{
		if (!EndStatsLabels[I] || !EndStatsValues[I]) continue;
		if (!Out.IsEmpty()) Out += TEXT("\n");
		Out += EndStatsLabels[I]->GetText().ToString() + TEXT(" ") + EndStatsValues[I]->GetText().ToString();
	}
	return Out;
}

// ---------------------------------------------------------------- the MATCH card, the scoreboard line, the ready hint (UJamDirectorsCallHudWidget)

FSlateFontInfo UJamDirectorsCallHudWidget::MatchCardFont(int32 Pt)
{
	if (!UJamHudWidget::OneLookOn()) return FJamUiStyle::Font(Pt, FName("Black"), FJamUiStyle::TrackTitle, true);
	return FJamMenuLook::Font(Pt - 8, FName("Black"), FJamMenuLook::TrackLabel);
}

void UJamDirectorsCallHudWidget::ApplyOneLook()
{
	// The MATCH card was a chalk-edged rounded plate with outlined white and yellow type over the far zone (h43-02). It is now a panel of the menu's: the
	// two team colours along its top edge over the panel fill, MATCH n in the menu's face, the period's line in line-size dim capitals under it.
	if (!UJamHudWidget::OneLookOn() || !WidgetTree) return;
	using L = FJamMenuLook;
	if (UBorder* Banner = Cast<UBorder>(TakeArt.Get()))
	{
		if (UWidget* Body = Banner->GetContent())
		{
			UVerticalBox* V = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MatchCardLook"));
			Banner->SetContent(V);
			Banner->SetBrushColor(FLinearColor::Transparent);
			Banner->SetPadding(FMargin(0));
			if (UVerticalBoxSlot* RS = V->AddChildToVerticalBox(L::MakeTeamRule(WidgetTree, 3.f))) RS->SetHorizontalAlignment(HAlign_Fill);
			UBorder* Inner = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MatchCardPanel"));
			JamOneLook::OLPlate(Inner, 0.9f, FMargin(32, 4, 32, 10));
			Inner->SetContent(Body);
			if (UVerticalBoxSlot* PS = V->AddChildToVerticalBox(Inner)) PS->SetHorizontalAlignment(HAlign_Fill);
		}
	}
	if (TakeCard)
	{
		TakeCard->SetFont(MatchCardFont(MatchCardFontPt));
		TakeCard->SetColorAndOpacity(FSlateColor(L::Text()));
		TakeCard->SetShadowOffset(FVector2D(0, 1));
		TakeCard->SetShadowColorAndOpacity(FLinearColor(0, 0, 0, 0.45f));
	}
	JamOneLook::OLType(TakeSub, UJamHudWidget::HudLinePt, FName("Bold"), L::TrackLabel, L::TextDim());
	if (Announcement) JamOneLook::OLPlate(Cast<UBorder>(Announcement->GetParent()), 0.86f, FMargin(16, 6));
	JamOneLook::OLType(Announcement, UJamHudWidget::HudLinePt, FName("Bold"), L::TrackLabel, FJamUiStyle::Yellow());
	if (ReadyHint) JamOneLook::OLPlate(Cast<UBorder>(ReadyHint->GetParent()), 0.86f, FMargin(12, 5));
	JamOneLook::OLType(ReadyHint, UJamHudWidget::HudLabelPt, FName("Bold"), L::TrackLabel, FJamUiStyle::Yellow());
}

// ---------------------------------------------------------------- the bark bubble (UJamBarkWidget)

void UJamBarkWidget::OneLookBubble(FJamBarkBubble& B, const FLinearColor& Edge)
{
	// The bubble was chalk paper with dark team-tinted type: the brightest plate on the screen for a line of banter (h43-03, the before stills). Now the
	// HUD's panel with the speaker's team colour on its edge (the bubble belongs to a team), the menu's type in the panel's text colour.
	if (!UJamHudWidget::OneLookOn()) return;
	using L = FJamMenuLook;
	const FLinearColor Panel = JamOneLook::OLFill(0.9f);
	if (B.Body) B.Body->SetBrush(FSlateBrush(FSlateRoundedBoxBrush(Panel, 4.f, FLinearColor(Edge.R, Edge.G, Edge.B, 1.f), 2.f)));
	if (B.Tail) B.Tail->SetBrush(FSlateBrush(FSlateRoundedBoxBrush(Panel, 1.f, FLinearColor(Edge.R, Edge.G, Edge.B, 1.f), 2.f)));
	if (B.Text) JamOneLook::OLType(B.Text, UJamHudWidget::HudLinePt, FName("Bold"), 0, L::Text());
}

TArray<TPair<FString, int32>> UJamHudWidget::GetHudTypeSizesForTest() const
{
	TArray<TPair<FString, int32>> Out;
	auto Add = [&Out](const TCHAR* Name, const UTextBlock* T) { if (T) Out.Emplace(Name, static_cast<int32>(T->GetFont().Size)); };
	Add(TEXT("score home"), ScoreHome); Add(TEXT("score away"), ScoreAway); Add(TEXT("period"), TakeText); Add(TEXT("rule"), GoalRuleText);
	Add(TEXT("clock"), TimerText); Add(TEXT("commentary"), CommentaryText); Add(TEXT("contact"), ContactText); Add(TEXT("prompt"), PromptTextBlock);
	Add(TEXT("toast"), ToastText); Add(TEXT("far net"), NetText); Add(TEXT("own net"), OwnText); Add(TEXT("puck"), PuckText); Add(TEXT("ability"), AbilityText); Add(TEXT("you tag"), YouTagText); Add(TEXT("box"), BoxText);
	if (FaceoffHintPlate) Add(TEXT("faceoff hint"), Cast<UTextBlock>(FaceoffHintPlate->GetContent()));
	return Out;
}

FSlateBrush UJamHudWidget::YouTagBrush(const FLinearColor& Team)
{
	return FJamUiStyle::Rounded(Team, OneLookOn() ? 4.f : 8.f, FLinearColor(0, 0, 0, 0.9f), 2.f);
}

// ---------------------------------------------------------------- wave 2

void UJamHudWidget::OneLookSlotLabel(UTextBlock* Label)
{
	if (!OneLookOn() || !Label) return;
	JamOneLook::OLType(Label, HudLinePt, FName("Bold"), FJamMenuLook::TrackLabel, FJamMenuLook::Text());
}

void UJamHudWidget::HideLabelsUnderCards(const FJamPresentationFrame& F)
{
	// Hidden, not moved: a card is up for a second or two (the MATCH card, the GOAL banner) or is the last picture (the end card), and the markers
	// came back where they were on the frame it went. The end card's still (pair 06) showed "58 m" over it and "own 27 m" beside it.
	if (!OneLookOverlay.IsValid() && (OneLookOverlaySearch++ % 30) == 0)
	{
		for (TObjectIterator<UJamDirectorsCallHudWidget> It; It; ++It) if (It->GetWorld() == GetWorld() && !It->HasAnyFlags(RF_ClassDefaultObject)) { OneLookOverlay = *It; break; }
	}
	const bool bMatch = OneLookOverlay.IsValid() && OneLookOverlay->IsTakeCardVisible();
	const bool bBanner = GoalBannerPlate && GoalBannerPlate->GetVisibility() != ESlateVisibility::Collapsed;
	const bool bPause = (GetWorld() && GetWorld()->IsPaused()) || (CPanelPause.IsValid() && CPanelPause->IsOpen());
	const bool bEnd = F.Phase == EJamPhase::Win || F.Phase == EJamPhase::Lose || (CardPlate && CardPlate->GetVisibility() != ESlateVisibility::Collapsed);
	bMarkersStoodDown = LabelsStandDown(OneLookOn(), bMatch, bBanner, bPause, bEnd);
	if (!bMarkersStoodDown) return;
	for (UWidget* W : { (UWidget*)NetDiamond.Get(), (UWidget*)NetText.Get(), (UWidget*)NetArrow.Get(), (UWidget*)OwnDiamond.Get(), (UWidget*)OwnText.Get(),
		(UWidget*)OwnArrow.Get(), (UWidget*)PuckArrow.Get(), (UWidget*)PuckText.Get() })
		if (W && W->GetVisibility() != ESlateVisibility::Collapsed) W->SetVisibility(ESlateVisibility::Collapsed);
}
