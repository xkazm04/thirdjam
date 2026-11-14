// Lane A166 (track F2, evolution 7, docs/experiments/MOVEMENT-8.md): the two warnings on the ice. Presentation only: UpdateMarks reads the sim through
// JamCarrierMarks.h (the rules, shared with the trace and the instrument) and writes nothing back. Its two ground rings are made here on first use (not in
// BuildTree) and drawn under every other HUD element.
//   The contested puck: a pulsing ring in the opponent's colour round a loose puck that the human and an opponent will reach head-on at about the same
//     time. What it says: "take this and you are hit as you touch it" (brake before it, let him have it and hit him, or cut across).
//   The stop's distance: a small ring on the ice five stick lengths from the carrying human towards the opponent closing ahead of him; chalk while
//     that opponent is far, his colour, larger and breathing from a quarter second before he is 9.5 m off until he is 7 m off (the stop passes), gone inside
//     7 m (too late) and while he brakes. Their colour on the ice means act now, here as round the contested puck; yellow stays the take grace's.
// Neither is drawn where its box would meet a downed figure (lane A158's rule; its own boxes, HUDClearDown.cpp, are not in this tree: a ground box round
// each downed skater stands in for them here).
#include "HUDWidget.h"
#include "MenuStyle.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HokejGameMode.h"
#include "JamSim.h"
#include "JamCarrierMarks.h"
#include "UnrealClient.h"

static TAutoConsoleVariable<int32> CVarJamHudMarks(TEXT("jam.hud.Marks"), 1,
	TEXT("The two warnings on the ice (lane A166): 1 (default) the contested puck's ring and the stop's distance mark; 0 neither (the screen of before)."), ECVF_Default);

static TAutoConsoleVariable<int32> CVarJamHudMarkStills(TEXT("jam.hud.MarkStills"), 0,
	TEXT("Proof: N > 0 takes a still (with the HUD) N sim frames into every showing of a mark: u1-mark-<tag>-contest-f<frame>.png and u1-mark-<tag>-stop-f<frame>.png (the stop's when it is lit). 0 off."), ECVF_Default);
static TAutoConsoleVariable<FString> CVarJamHudMarkStillTag(TEXT("jam.hud.MarkStillTag"), TEXT("p3_clay"),
	TEXT("Proof: the tag in the names of the marks' stills (the capture driver collects stills by the look tier's id)."), ECVF_Default);

namespace
{
	// The proof's stills: one per showing, StillAfter sim frames into it (the HUD may see a sim frame twice or skip one: at or after, once).
	struct FMarkStill { int64 Since = -1; bool bTaken = false; };
	FMarkStill GContestStill, GStopStill;
	void MarkStill(FMarkStill& St, const TCHAR* Kind, bool bShown, int64 SimFrame)
	{
		const int32 After = CVarJamHudMarkStills.GetValueOnGameThread();
		if (After <= 0 || !bShown) { St = FMarkStill(); return; }
		if (St.Since < 0 || SimFrame < St.Since) { St.Since = SimFrame; St.bTaken = false; }
		if (!St.bTaken && SimFrame - St.Since >= After)
		{
			St.bTaken = true;
			const FString Name = FString::Printf(TEXT("u1-mark-%s-%s-f%06lld"), *CVarJamHudMarkStillTag.GetValueOnGameThread(), Kind, SimFrame);
			FScreenshotRequest::RequestScreenshot(Name + TEXT(".png"), true, false);
			UE_LOG(LogTemp, Log, TEXT("JAMHUD frame=%lld mark_still=%s.png (%d sim frames into the showing)"), SimFrame, *Name, After);
		}
	}


	UImage* MakeMarkRing(UWidgetTree* Tree, UCanvasPanel* Root, const TCHAR* Name)
	{
		if (!Tree || !Root) return nullptr;
		UImage* I = Tree->ConstructWidget<UImage>(UImage::StaticClass(), Name);
		if (UTexture2D* Plain = FJamUiStyle::Art(TEXT("T_UI_RingPlain"))) I->SetBrush(FJamUiStyle::ImageBrush(Plain, FVector2D(64.f, 64.f)));
		else I->SetBrush(FJamUiStyle::Ring(64.f));
		I->SetVisibility(ESlateVisibility::Collapsed);
		if (UCanvasPanelSlot* CS = Root->AddChildToCanvas(I))
		{
			CS->SetAnchors(FAnchors(0.f, 0.f));
			CS->SetAlignment(FVector2D(0.5f, 0.5f));
			CS->SetAutoSize(false);
			CS->SetZOrder(-1);   // on the ice: under every other element
		}
		return I;
	}
}

void UJamHudWidget::UpdateMarks(const FJamPresentationFrame& F)
{
	if (!Root || !WidgetTree) return;
	if (!ContestRing) ContestRing = MakeMarkRing(WidgetTree, Root, TEXT("ContestRing"));
	if (!StopMarkRing) StopMarkRing = MakeMarkRing(WidgetTree, Root, TEXT("StopMarkRing"));
	if (!ContestRing || !StopMarkRing) return;
	auto Hide = [](UWidget* W) { if (W) W->SetVisibility(ESlateVisibility::Collapsed); };
	const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	APlayerController* PC = GetOwningPlayer();
	const bool bOn = CVarJamHudMarks.GetValueOnGameThread() != 0 && F.Phase == EJamPhase::Play && Sim && PC;
	if (!bOn)
	{
		Hide(ContestRing); Hide(StopMarkRing);
		MarkContestOpp = -1; MarkStopState = 0;
		return;
	}
	const TArray<FJamSkaterState>& Sk = Sim->GetSkaters();
	const int32 Hi = Sim->GetHumanIndex();
	// Where the downed figures are on the screen: the ground under each (1.3 m round it) and the point over its head.
	TArray<FBox2D> Downs;
	for (const FJamSkaterState& K : Sk)
	{
		if (!K.IsDown()) continue;
		FVector2D C, Sz;
		float Dist = 0.f;
		if (!ProjectGroundRing(K.Pos, MarkDownRadiusM, C, Sz, Dist)) continue;
		FBox2D B(C - Sz * 0.5f, C + Sz * 0.5f);
		FVector2D Head;
		if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, UJamSim::ToWorld(K.Pos, 1.9), Head, false)) B += Head;
		Downs.Add(B);
	}
	const float Pulse = MarkPulse(F.SimFrame);

	// The contested puck.
	const JamCarrierMarks::FContested Con = JamCarrierMarks::Contested(*Sim, Hi);
	bool bConShown = false;
	if (Con.bOn && Sk.IsValidIndex(Con.Opp))
	{
		FVector2D C, Sz;
		float Dist = 0.f;
		if (ProjectGroundRing(Sim->GetPuck().Pos, ContestRingRadiusM * Pulse, C, Sz, Dist) && !MarkMeetsDown(C - Sz * 0.5f, C + Sz * 0.5f, Downs))
		{
			const FLinearColor Col = F.GetTeamColour(Sk[Con.Opp].Team);
			ContestRing->SetColorAndOpacity(FLinearColor(Col.R, Col.G, Col.B, 1.f));
			ContestRing->SetVisibility(ESlateVisibility::HitTestInvisible);
			if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(ContestRing->Slot)) { CS->SetPosition(C); CS->SetSize(Sz); }
			bConShown = true;
		}
	}
	if (!bConShown) Hide(ContestRing);
	const int32 ConNow = Con.bOn ? Con.Opp : -1;
	if ((ConNow >= 0) != (MarkContestOpp >= 0))
	{
		UE_LOG(LogTemp, Log, TEXT("JAMHUD frame=%lld mark=contested on=%d opp=%d human_s=%.2f opp_s=%.2f closing=%.1f shown=%d"), F.SimFrame, ConNow >= 0 ? 1 : 0, ConNow, Con.HumanSec, Con.OppSec, Con.ClosingMps, bConShown ? 1 : 0);
	}
	MarkContestOpp = ConNow;

	// The stop's distance.
	const JamCarrierMarks::FStopMark Stop = JamCarrierMarks::StopMark(*Sim, Hi);
	bool bStopShown = false;
	if (Stop.bShow)
	{
		FVector2D C, Sz;
		float Dist = 0.f;
		const float Radius = Stop.bLit ? StopMarkLitRadiusM * Pulse : StopMarkArmedRadiusM;
		if (ProjectGroundRing(Stop.At, Radius, C, Sz, Dist) && !MarkMeetsDown(C - Sz * 0.5f, C + Sz * 0.5f, Downs))
		{
			// Lit in the closing opponent's colour, as the contested ring is: their colour on the ice = act now. (Yellow is the take grace's: lit yellow, the
			// mark stood beside the human's own yellow ring after a draw he won: two yellows saying two things. Capture 2, frame 1040.)
			const FLinearColor Col = Stop.bLit && Sk.IsValidIndex(Stop.Def) ? F.GetTeamColour(Sk[Stop.Def].Team) : FJamUiStyle::Paper();
			StopMarkRing->SetColorAndOpacity(FLinearColor(Col.R, Col.G, Col.B, Stop.bLit ? 1.f : StopMarkArmedAlpha));
			StopMarkRing->SetVisibility(ESlateVisibility::HitTestInvisible);
			if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(StopMarkRing->Slot)) { CS->SetPosition(C); CS->SetSize(Sz); }
			bStopShown = true;
		}
	}
	if (!bStopShown) Hide(StopMarkRing);
	const int32 StopNow = !Stop.bShow ? 0 : (Stop.bLit ? 2 : 1);
	if (StopNow != MarkStopState)
	{
		UE_LOG(LogTemp, Log, TEXT("JAMHUD frame=%lld mark=stop state=%s def=%d dist=%.1f closing=%.1f shown=%d"), F.SimFrame, StopNow == 2 ? TEXT("lit") : StopNow == 1 ? TEXT("armed") : TEXT("off"), Stop.Def, Stop.DistM, Stop.ClosingMps,
			bStopShown ? 1 : 0);
	}
	MarkStopState = StopNow;
	MarkStill(GContestStill, TEXT("contest"), bConShown, F.SimFrame);
	MarkStill(GStopStill, TEXT("stop"), bStopShown && Stop.bLit, F.SimFrame);
}
