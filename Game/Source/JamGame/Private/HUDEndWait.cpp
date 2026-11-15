// Lane A213, wave 2 (EYE 11 of A211's list: "the match-ending goal is not shown: BEATEN covers the screen on the goal's own frame", 2 of 2). The sim
// goes from Play to Win / Lose on the frame of the last goal (no goal pause, no GOAL banner), and RefreshFromFrame shows the end card on that frame.
// UpdateEndWait runs after it and hides the card, its score, its note and its keys for EndCardWaitSec of real time from the first frame of the end
// phase (the sim's frame does not advance in the end phase, so the wait is the clock's): the puck in the net, the sparks and the horn are seen first.
// Presentation only. jam.hud.EndWait 0 = the card on the goal's own frame.
#include "HUDWidget.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformTime.h"
#include "UnrealClient.h"

static TAutoConsoleVariable<int32> CVarJamHudEndWait(TEXT("jam.hud.EndWait"), 1,
	TEXT("Lane A213 (EYE 11): 1 (default) the win / loss card waits 1.9 s after the match-ending goal so the goal is seen; 0 = the card on the goal's own frame."), ECVF_Default);

static TAutoConsoleVariable<int32> CVarJamHudEndStills(TEXT("jam.hud.EndStills"), 0,
	TEXT("Proof: 1 takes two stills of each match end: a213-end-wait (0.5 s in: the goal, no card) and a213-end-card (2.3 s in). 0 off."), ECVF_Default);
namespace { int32 GEndStillsTaken = 0; }

void UJamHudWidget::UpdateEndWait(const FJamPresentationFrame& F)
{
	const bool bEnd = F.Phase == EJamPhase::Win || F.Phase == EJamPhase::Lose;
	if (!bEnd) { EndSeenAt = -1.0; GEndStillsTaken = 0; return; }
	const double Now = FPlatformTime::Seconds();
	if (EndSeenAt < 0.0)
	{
		EndSeenAt = Now;
		UE_LOG(LogTemp, Log, TEXT("JAMHUD frame=%lld end_card=wait seconds=%.1f on=%d"), F.SimFrame, EndCardWaitSec, CVarJamHudEndWait.GetValueOnGameThread());
	}
	if (CVarJamHudEndStills.GetValueOnGameThread() != 0)
	{
		const double Age = Now - EndSeenAt;
		if (GEndStillsTaken == 0 && Age >= 0.5) { ++GEndStillsTaken; FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("a213-end-wait-f%06lld.png"), F.SimFrame), true, false); }
		if (GEndStillsTaken == 1 && Age >= 2.3) { ++GEndStillsTaken; FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("a213-end-card-f%06lld.png"), F.SimFrame), true, false); }
	}
	// By opacity, not visibility: the card's keys row is shown once by its painter (PaintEndCardKeys) and would not come back from Collapsed.
	const float Show = EndCardWaits(CVarJamHudEndWait.GetValueOnGameThread() != 0, bEnd, Now - EndSeenAt) ? 0.f : 1.f;
	UWidget* const Card[] = { CardPlate.Get(), CardTextBlock.Get(), CardScoreText.Get(), CardNoteText.Get(), CardHints.Get() };
	for (UWidget* W : Card) if (W && W->GetRenderOpacity() != Show) W->SetRenderOpacity(Show);
}
