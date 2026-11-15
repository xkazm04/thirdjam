// Lane A166 (track F2, evolution 7, docs/experiments/MOVEMENT-8.md): the two warnings on the ice. Presentation only: UpdateMarks reads the sim through
// JamCarrierMarks.h (the rules, shared with the trace and the instrument) and writes nothing back. Its two ground rings are made here on first use (not in
// BuildTree) and drawn under every other HUD element.
//   The contested puck: a pulsing ring in the opponent's colour round a loose puck that the human and an opponent will reach head-on at about the same
//     time. What it says: "take this and you are hit as you touch it" (brake before it, let him have it and hit him, or cut across).
//   The stop's distance: a LINE across the human's path (lane A171: it was a ring, and two rings in one colour asked for opposite acts) on the ice five
//     stick lengths from the carrying human towards the opponent closing ahead of him; chalk and thin while
//     that opponent is far, his colour, thicker and breathing from a quarter second before he is 9 m off (lane A185; 9.5 before) until he is 7 m off (the stop passes), gone inside
//     7 m (too late) and while he brakes. Their colour on the ice means act now, here as round the contested puck; yellow stays the take grace's.
// Neither is drawn where its box would meet a downed figure: lane A158's rule and, since lane A171, lane A158's boxes (DownBoxesNow, made by
// UpdateKeepClear at the top of the same refresh: every downed figure by its projected bones), the one source every mark and marker yields to.
#include "HUDWidget.h"
#include "MenuStyle.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HokejGameMode.h"
#include "JamSim.h"
#include "JamCarrierMarks.h"
#include "UnrealClient.h"

static TAutoConsoleVariable<int32> CVarJamHudMarks(TEXT("jam.hud.Marks"), 1,
	TEXT("The two warnings on the ice (lane A166): 1 (default) the contested puck's ring and the stop's distance mark; 0 neither (the screen of before)."), ECVF_Default);

// Lane A219 (R-0069 point 3 and R-0070, the Director's order of 15 Nov 11:36: every strip on the ice from the HUD's files that shows at its default goes):
// the stop's line (a strip across his path ahead of him while he carries: paper-white, then the closing opponent's colour) has its own switch and is OFF
// by default. The rule's state is still logged (JAMHUD mark=stop ..., shown=0); the contested puck's rings stay with jam.hud.Marks. 1 = package H39's line.
static TAutoConsoleVariable<int32> CVarJamHudStopLine(TEXT("jam.hud.StopLine"), 0,
	TEXT("The stop's distance line on the ice ahead of the carrying human (lane A166 / A171): 0 (default from lane A219, R-0070: no line of the game's mechanics on the ice) not drawn; 1 = drawn (package H39). The contested puck's rings are jam.hud.Marks."), ECVF_Default);

static TAutoConsoleVariable<int32> CVarJamHudMarkStills(TEXT("jam.hud.MarkStills"), 0,
	TEXT("Proof: N > 0 takes a still (with the HUD) N sim frames into every showing of a mark: u1-mark-<tag>-contest-f<frame>.png and u1-mark-<tag>-stop-f<frame>.png (the stop's when it is lit). 0 off."), ECVF_Default);
static TAutoConsoleVariable<FString> CVarJamHudMarkStillTag(TEXT("jam.hud.MarkStillTag"), TEXT("p3_clay"),
	TEXT("Proof: the tag in the names of the marks' stills (the capture driver collects stills by the look tier's id)."), ECVF_Default);

// Lane A178 (track F2, evolution 8): what a real window could not read on package H30 (docs/experiments/MOVEMENT-9.md).
static TAutoConsoleVariable<int32> CVarJamHudContestHold(TEXT("jam.hud.ContestHold"), 24,
	TEXT("The contested mark's least time on screen once lit, in sim frames (a gap of the rule of half that inside one showing does not end it: no flicker). 0 = the rule frame by frame (package H30: a median of 5 frames on screen)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHudStopBarMode(TEXT("jam.hud.StopBarMode"), 1,
	TEXT("Where the stop's line lies: 1 (default) across the human's path always, straight ahead of him at the mark's distance; 0 (package H30) at the point towards the closing opponent: off to one side when he comes from the side."), ECVF_Default);

namespace
{
	JamCarrierMarks::FMarkShow GContestShow;   // one HUD at a time: the contested mark's showing (stepped by the sim's frame)
	int64 GContestShownSince = -1;

	// The proof's stills: one per showing, StillAfter sim frames into it (the HUD may see a sim frame twice or skip one: at or after, once).
	struct FMarkStill { int64 Since = -1; bool bTaken = false; };
	FMarkStill GContestStill, GStopStill, GArmedStill;
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

// Lane A171: the stop's line: a white bar with an ink edge (chalk reads on pale ice only with an edge), tinted, sized and turned by the caller.
static UBorder* JamMakeMarkBar(UWidgetTree* Tree, UCanvasPanel* Root, const TCHAR* Name)
{
	if (!Tree || !Root) return nullptr;
	UBorder* B = Tree->ConstructWidget<UBorder>(UBorder::StaticClass(), Name);
	B->SetBrush(FJamUiStyle::Rounded(FLinearColor::White, 2.f, FLinearColor(0.f, 0.f, 0.f, 0.9f), 1.5f));
	B->SetRenderTransformPivot(FVector2D(0.5, 0.5));
	B->SetVisibility(ESlateVisibility::Collapsed);
	if (UCanvasPanelSlot* CS = Root->AddChildToCanvas(B))
	{
		CS->SetAnchors(FAnchors(0.f, 0.f));
		CS->SetAlignment(FVector2D(0.5f, 0.5f));
		CS->SetAutoSize(false);
		CS->SetZOrder(-1);   // on the ice: under every other element
	}
	return B;
}

void UJamHudWidget::UpdateMarks(const FJamPresentationFrame& F)
{
	if (!Root || !WidgetTree) return;
	if (!ContestRing) ContestRing = MakeMarkRing(WidgetTree, Root, TEXT("ContestRing"));
	if (!StopMarkBar) StopMarkBar = JamMakeMarkBar(WidgetTree, Root, TEXT("StopMarkBar"));
	UBorder* StopBar = StopMarkBar.Get();
	if (!ContestRingInner) ContestRingInner = MakeMarkRing(WidgetTree, Root, TEXT("ContestRingInner"));
	if (!ContestOppRing) ContestOppRing = MakeMarkRing(WidgetTree, Root, TEXT("ContestOppRing"));
	if (!ContestRing || !StopBar || !ContestRingInner || !ContestOppRing) return;
	auto Hide = [](UWidget* W) { if (W) W->SetVisibility(ESlateVisibility::Collapsed); };
	const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	APlayerController* PC = GetOwningPlayer();
	const bool bOn = CVarJamHudMarks.GetValueOnGameThread() != 0 && F.Phase == EJamPhase::Play && Sim && PC;
	if (!bOn)
	{
		Hide(ContestRing); Hide(StopBar); HideGroundStrip(StopStripPool); Hide(ContestRingInner); Hide(ContestOppRing);
		MarkContestOpp = -1; MarkStopState = 0;
		GContestShow = JamCarrierMarks::FMarkShow(); GContestShownSince = -1;
		return;
	}
	const TArray<FJamSkaterState>& Sk = Sim->GetSkaters();
	const int32 Hi = Sim->GetHumanIndex();
	// Where the downed figures are on the screen: this tick's boxes of lane A158 (lane A171: one source; the marks had a ground ring of their own).
	const TArray<FBox2D>& Downs = DownBoxesNow;
	const float Pulse = MarkPulse(F.SimFrame);

	// The contested puck.
	const JamCarrierMarks::FContested Con = JamCarrierMarks::Contested(*Sim, Hi);
	// Lane A178: the showing. Once the rule has said yes the mark stands jam.hud.ContestHold frames at least and through a short gap of the rule; it
	// follows the puck (the hit lands eight frames after his touch: the ring round the puck he now carries is still the warning).
	const int32 Hold = CVarJamHudContestHold.GetValueOnGameThread();
	bool bConOn = Con.bOn;
	int32 ConOpp = Con.Opp;
	if (Hold > 0)
	{
		JamCarrierMarks::FShowRule SR;
		SR.MinFrames = Hold; SR.LingerFrames = Hold / 2;
		bConOn = JamCarrierMarks::StepShow(GContestShow, Con.bOn, Con.Opp, F.SimFrame, SR);
		ConOpp = GContestShow.Opp;
	}
	bool bConShown = false, bOppShown = false;
	if (bConOn && Sk.IsValidIndex(ConOpp))
	{
		const FLinearColor Team = F.GetTeamColour(Sk[ConOpp].Team);
		const FLinearColor Col(Team.R, Team.G, Team.B, 1.f);
		auto Ring = [&](UImage* I, const FVector2D& PosM, float RadiusM)
		{
			FVector2D C, Sz;
			float Dist = 0.f;
			if (!ProjectGroundRing(PosM, RadiusM, C, Sz, Dist) || MarkMeetsDown(C - Sz * 0.5f, C + Sz * 0.5f, Downs, MarkDownPadPx)) return false;
			I->SetColorAndOpacity(Col);
			I->SetVisibility(ESlateVisibility::HitTestInvisible);
			if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(I->Slot)) { CS->SetPosition(C); CS->SetSize(Sz); }
			return true;
		};
		// Once the puck is HIS (the showing still stands its least time), the two rings round the puck would lie on his own ring and on the yellow of his
		// take grace (capture a178b-free-m1, frame 3412: five rings under one skater): they go, and the ring under the one who is coming stays: it is
		// that skater the warning is about now.
		const bool bMine = Sim->CarrierIndex() == Hi;
		bConShown = !bMine && Ring(ContestRing, Sim->GetPuck().Pos, ContestRingRadiusM * Pulse);
		if (!bConShown || !Ring(ContestRingInner, Sim->GetPuck().Pos, ContestInnerRadiusM * Pulse)) Hide(ContestRingInner);
		bOppShown = (bConShown || bMine) && !Sk[ConOpp].IsDown() && Ring(ContestOppRing, Sk[ConOpp].Pos, ContestOppRadiusM * Pulse);
	}
	if (!bConShown) { Hide(ContestRing); Hide(ContestRingInner); }
	if (!bOppShown) Hide(ContestOppRing);
	const int32 ConNow = bConOn ? ConOpp : -1;
	if ((ConNow >= 0) != (MarkContestOpp >= 0))
	{
		const int64 OnScreen = ConNow >= 0 || GContestShownSince < 0 ? 0 : F.SimFrame - GContestShownSince;
		UE_LOG(LogTemp, Log, TEXT("JAMHUD frame=%lld mark=contested on=%d opp=%d human_s=%.2f opp_s=%.2f closing=%.1f shown=%d hold=%d opp_ring=%d on_screen=%lld"), F.SimFrame, ConNow >= 0 ? 1 : 0, ConNow, Con.HumanSec, Con.OppSec,
			Con.ClosingMps, bConShown || bOppShown ? 1 : 0, Hold, bOppShown ? 1 : 0, static_cast<long long>(OnScreen));
		GContestShownSince = ConNow >= 0 ? F.SimFrame : -1;
	}
	MarkContestOpp = ConNow;

	// The stop's distance.
	const JamCarrierMarks::FStopMark Stop = JamCarrierMarks::StopMark(*Sim, Hi);
	bool bStopShown = false;
	if (Stop.bShow && CVarJamHudStopLine.GetValueOnGameThread() != 0)   // lane A219: the line itself is off by default (jam.hud.StopLine)
	{
		// Lane A171: a line across his path at the mark's place: perpendicular to his travel (to the way to the opponent when he barely moves), two stick
		// lengths wide; its four points go through the camera (the ends, and the near and far edge of its thickness).
		const FJamSkaterState& Me = Sk[Hi];
		const FVector2D Way = Me.Vel.SizeSquared() > 1.0 ? Me.Vel.GetSafeNormal() : (Stop.At - Me.Pos).GetSafeNormal();
		const FVector2D Across(-Way.Y, Way.X);
		const double ThickM = Stop.bLit ? StopBarLitThickM * Pulse : StopBarArmedThickM;
		// Lane A178 (jam.hud.StopBarMode): across his path ALWAYS (straight ahead of him at the mark's distance), whatever side the opponent comes from. In
		// package H30 it lay at the point towards the opponent: for one 40 to 60 degrees off his travel that is a dash off to one side, level across the
		// screen, that crosses nobody's way. The line is about HIS motion (press against it): it belongs on his path.
		const FVector2D StopAt = CVarJamHudStopBarMode.GetValueOnGameThread() != 0 ? Stop.AtPath : Stop.At;
		const FVector2D Ends[4] = { StopAt - Across * StopBarHalfWidthM, StopAt + Across * StopBarHalfWidthM, StopAt - Way * (ThickM * 0.5), StopAt + Way * (ThickM * 0.5) };
		FVector2D Px[4];
		bool bAll = !Way.IsNearlyZero();
		for (int32 i = 0; i < 4 && bAll; ++i) bAll = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, UJamSim::ToWorld(Ends[i], 0.03), Px[i], false);
		FVector2D C, Sz, BoxMin, BoxMax;
		float AngleDeg = 0.f;
		if (bAll && StopBarLayout(Px[0], Px[1], Px[2], Px[3], Stop.bLit ? StopBarLitMinPx : StopBarArmedMinPx, C, Sz, AngleDeg, BoxMin, BoxMax) && !MarkMeetsDown(BoxMin, BoxMax, Downs, MarkDownPadPx))
		{
			// Lit in the closing opponent's colour, as the contested ring is: their colour on the ice = act now; the SHAPE says which act. (Yellow is the take
			// grace's: lit yellow, the mark stood beside the human's own yellow ring after a draw he won: two yellows saying two things. Capture 2, frame 1040.)
			const FLinearColor Col = Stop.bLit && Sk.IsValidIndex(Stop.Def) ? F.GetTeamColour(Sk[Stop.Def].Team) : FJamUiStyle::Paper();
			StopBar->SetBrushColor(FLinearColor(Col.R, Col.G, Col.B, Stop.bLit ? 1.f : StopMarkArmedAlpha));
			StopBar->SetRenderTransformAngle(AngleDeg);
			StopBar->SetVisibility(ESlateVisibility::HitTestInvisible);
			// Lane A213 (wave 4, A211's fourth list: "a white stop bar still hangs in the air"): in the ice's plane from end to end of the line.
			if (GroundLinesOn() && DrawGroundStrip(StopStripPool, Ends[0], Ends[1], ThickM, 0.0, Col, Stop.bLit ? 1.f : StopMarkArmedAlpha) > 0) StopBar->SetVisibility(ESlateVisibility::Collapsed);
			if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(StopBar->Slot)) { CS->SetPosition(C); CS->SetSize(Sz); }
			bStopShown = true;
		}
	}
	if (!bStopShown) { Hide(StopBar); HideGroundStrip(StopStripPool); }   // lane A213 (wave 4): the strip with it
	const int32 StopNow = !Stop.bShow ? 0 : (Stop.bLit ? 2 : 1);
	if (StopNow != MarkStopState)
	{
		UE_LOG(LogTemp, Log, TEXT("JAMHUD frame=%lld mark=stop state=%s def=%d dist=%.1f closing=%.1f shown=%d off=%.0f mode=%d gate=%d second_s=%.2f net=%.1f"), F.SimFrame, StopNow == 2 ? TEXT("lit") : StopNow == 1 ? TEXT("armed") : TEXT("off"), Stop.Def, Stop.DistM, Stop.ClosingMps,
			bStopShown ? 1 : 0, Stop.OffDeg, CVarJamHudStopBarMode.GetValueOnGameThread(), Stop.Gate, FMath::Min(Stop.SecondSec, 99.0), Stop.NetM);   // lane A201: which gate kept it pale
	}
	MarkStopState = StopNow;
	MarkStill(GContestStill, TEXT("contest"), bConShown || bOppShown, F.SimFrame);
	MarkStill(GStopStill, TEXT("stop"), bStopShown && Stop.bLit, F.SimFrame);
	MarkStill(GArmedStill, TEXT("armed"), bStopShown && !Stop.bLit, F.SimFrame);
}
