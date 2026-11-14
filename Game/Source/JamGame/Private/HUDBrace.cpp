// Lane A192 (track F1, evolution 10; docs/experiments/CONTACT-11.md): the held cross-check says so on the contact line. Presentation only: reads the human's
// two counters in the sim (CrossBraceHits: braced hits he delivered; CrossBraceTaken: braced hits he took) and writes nothing back. A counter survives a
// frame the hit-stop held; a restart sets the counters back and shows nothing.
#include "HUDWidget.h"
#include "Engine/World.h"
#include "HokejGameMode.h"
#include "JamSim.h"

void UJamHudWidget::UpdateBraceLine(const FJamPresentationFrame& F)
{
	const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	if (!Sim || !Sim->GetSkaters().IsValidIndex(Sim->GetHumanIndex())) return;
	const FJamSkaterState& Hu = Sim->GetSkaters()[Sim->GetHumanIndex()];
	const bool bGiven = Hu.CrossBraceHits > BraceSeenGiven;
	const bool bTaken = Hu.CrossBraceTaken > BraceSeenTaken;
	BraceSeenGiven = Hu.CrossBraceHits;
	BraceSeenTaken = Hu.CrossBraceTaken;
	if (!bGiven && !bTaken) return;
	const FString Line = BraceLineFor(bGiven);
	ShowContactLine(Line);
	UE_LOG(LogTemp, Log, TEXT("JAMHUD frame=%lld contact_line=%s brace_given=%d brace_taken=%d"), F.SimFrame, *Line.Replace(TEXT(" "), TEXT("_")), Hu.CrossBraceHits, Hu.CrossBraceTaken);
}
