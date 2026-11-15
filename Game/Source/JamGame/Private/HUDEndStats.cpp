// Lane A213, wave 4 (the Director, 04:02: "the end card says what happened"): under YOU WIN / BEATEN, in the card's own panel, four short lines of the
// human's match: shots and goals, his passes that arrived, the hits he landed, the time he carried the puck. Presentation only: counted here from
// the sim's existing per-skater fields (Shots, Goals, Passes, PassesReceived) and its frame events (check events of his), frame by frame while the
// match plays; nothing is added to the sim. The lines sit inside the card's panel, so the end card's wait (HUDEndWait.cpp) hides them with it.
// jam.hud.EndStats 0 = the card of before.
#include "HUDWidget.h"
#include "MenuStyle.h"
#include "MenuWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/PanelWidget.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HokejGameMode.h"
#include "JamSim.h"
#include "JamEventBus.h"

static TAutoConsoleVariable<int32> CVarJamHudEndStats(TEXT("jam.hud.EndStats"), 1,
	TEXT("Lane A213: 1 (default) the end card shows four lines of the human's match (shots and goals, passes that arrived, hits landed, time with the puck); 0 = the card of before."), ECVF_Default);

void UJamHudWidget::UpdateEndStats(const FJamPresentationFrame& F)
{
	const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	if (!Sim || !Sim->GetSkaters().IsValidIndex(Sim->GetHumanIndex())) return;
	const TArray<FJamSkaterState>& Sk = Sim->GetSkaters();
	const int32 Hi = Sim->GetHumanIndex();
	const FJamSkaterState& Hu = Sk[Hi];
	auto TeamReceived = [&]() { int32 N = 0; for (const FJamSkaterState& K : Sk) if (K.Team == Hu.Team && K.Index != Hi) N += K.PassesReceived; return N; };
	// A new match (the frame count went back): the counts start again.
	if (F.SimFrame < EndStatsLastFrame || EndStatsLastFrame < 0)
	{
		EndStatsNow = FEndStats();
		EndStatsPassesSeen = Hu.Passes; EndStatsShotsSeen = Hu.Shots; EndStatsGoalsSeen = Hu.Goals; EndStatsReceivedSeen = TeamReceived();
		EndStatsPassPending = -1;
		EndStatsLastFrame = F.SimFrame;
	}
	const bool bEnd = F.Phase == EJamPhase::Win || F.Phase == EJamPhase::Lose;
	if (F.SimFrame != EndStatsLastFrame && !bEnd)
	{
		// One step of the match (the HUD sees most sim frames; a skipped frame loses at most its own events, never a count of the fields).
		EndStatsLastFrame = F.SimFrame;
		if (Hu.Shots > EndStatsShotsSeen) EndStatsNow.Shots += Hu.Shots - EndStatsShotsSeen;
		if (Hu.Goals > EndStatsGoalsSeen) EndStatsNow.Goals += Hu.Goals - EndStatsGoalsSeen;
		EndStatsShotsSeen = Hu.Shots; EndStatsGoalsSeen = Hu.Goals;
		const int32 Received = TeamReceived();
		if (EndStatsPassPending >= 0 && Received > EndStatsReceivedSeen) { ++EndStatsNow.Arrived; EndStatsPassPending = -1; }
		if (EndStatsPassPending >= 0 && F.SimFrame - EndStatsPassPending > EndPassArriveFrames) EndStatsPassPending = -1;
		EndStatsReceivedSeen = Received;
		if (Hu.Passes > EndStatsPassesSeen) { EndStatsNow.Passes += Hu.Passes - EndStatsPassesSeen; EndStatsPassPending = F.SimFrame; }
		EndStatsPassesSeen = Hu.Passes;
		for (const FJamSimEvent& E : Sim->GetFrameEvents()) if (E.Who == Hi && E.Name == JamEvents::Check) ++EndStatsNow.Hits;
		if (Sim->CarrierIndex() == Hi) ++EndStatsNow.CarryFrames;
	}
	// The card's lines: made once, under the score, in the card's own panel.
	if (!EndStatsBlock && CardScoreText && WidgetTree)
	{
		if (UVerticalBox* Body = Cast<UVerticalBox>(CardScoreText->GetParent()))
		{
			UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("EndStats"));
			T->SetFont(FJamMenuLook::Font(FJamMenuLook::Body, FName("Bold"), FJamMenuLook::TrackLabel));
			T->SetColorAndOpacity(FSlateColor(FJamMenuLook::TextDim()));
			T->SetJustification(ETextJustify::Center);
			T->SetVisibility(ESlateVisibility::Collapsed);
			const int32 At = Body->GetChildIndex(CardScoreText) + 1;
			if (UVerticalBoxSlot* VS = Cast<UVerticalBoxSlot>(Body->InsertChildAt(At, T))) { VS->SetHorizontalAlignment(HAlign_Center); VS->SetPadding(FMargin(0, 0, 0, FJamMenuLook::Unit * 2.f)); }
			EndStatsBlock = T;
		}
	}
	if (!EndStatsBlock) return;
	const bool bShow = bEnd && CVarJamHudEndStats.GetValueOnGameThread() != 0;
	if (bShow)
	{
		const FString Text = EndStatsText(EndStatsNow);
		if (EndStatsBlock->GetText().ToString() != Text) EndStatsBlock->SetText(FText::FromString(Text));
	}
	EndStatsBlock->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}
