// Lane A235 (track S1, Sunday night): two HUD faults of the walk of the candidate H45 (lane A234), each behind a variable whose neutral value gives back
// the HUD of before. Presentation only: nothing here reads or writes the sim's state.
//   W6  the yellow teaching prompt and the MATCH card shared one place for about two seconds (PLAY NOW rookie f800 to f920, seed 1 of hockey.regular).
//   W7  on Glow Ice the faceoff's dial and its hint were drawn for the human while he sat in the box (f1920, IN THE BOX 8).
#include "HUDWidget.h"
#include "HokejGameMode.h"
#include "JamSim.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "HAL/IConsoleManager.h"
#include "Engine/World.h"

static TAutoConsoleVariable<int32> CVarJamHudPromptWaitsForCard(TEXT("jam.hud.PromptWaitsForCard"), 1,
	TEXT("Lane A235 (W6): 1 (default) while a card is up (the MATCH card, the GOAL banner, the pause card, the end card) the yellow teaching prompt is not drawn and its clock stands still; it comes back when the card leaves. 0 = before."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHudFaceoffCueInBox(TEXT("jam.hud.FaceoffCueInBox"), 0,
	TEXT("Lane A235 (W7): 0 (default) while the human sits in the penalty box the faceoff's dial and its hint (PRESS SHOOT AT THE DROP) are not drawn for him; 1 = before."), ECVF_Default);

bool UJamHudWidget::PromptWaitsForCardOn() { return CVarJamHudPromptWaitsForCard.GetValueOnGameThread() != 0; }
bool UJamHudWidget::FaceoffCueInBoxOn() { return CVarJamHudFaceoffCueInBox.GetValueOnGameThread() != 0; }

bool UJamHudWidget::HoldPromptUnderCard(bool bSimAdvanced, int64 SimFrame)
{
	if (!PromptPlate || PromptNow == EPrompt::None || !PromptWaitsForCard(PromptWaitsForCardOn(), bCardUpForPrompt)) return false;
	PromptPlate->SetVisibility(ESlateVisibility::Collapsed);
	// The prompt's clock stands still while it waits: it is seen for its whole time once the card has gone.
	// Never past the frame shown (UpdatePrompts reads a prompt from the future as a restart and drops it).
	if (bSimAdvanced && PromptShown >= 0) PromptShown = FMath::Min(PromptShown + 1, SimFrame);
	return true;
}

void UJamHudWidget::HideFaceoffCueInBox(const FJamPresentationFrame& F)
{
	const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	const int32 Penalty = (Sim && Sim->GetSkaters().IsValidIndex(F.HumanIndex)) ? Sim->GetSkaters()[F.HumanIndex].PenaltyFrames : 0;
	if (!FaceoffCueHiddenInBox(FaceoffCueInBoxOn(), Penalty)) return;
	for (UWidget* W : { (UWidget*)FaceoffWord.Get(), (UWidget*)CountdownBox.Get(), (UWidget*)FaceoffHintPlate.Get() })
		if (W && W->GetVisibility() != ESlateVisibility::Collapsed) W->SetVisibility(ESlateVisibility::Collapsed);
}
