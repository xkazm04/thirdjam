// Lane A213, wave 2 (brief item 3): the two holds speak one language. Lane A207's pass meter (HUDPassMeter.cpp, its numbers A207's) lays a line on the
// ice from the puck for the held pass's reach, in the team's colour when it has found a mate. This file adds:
//   - the mate the held pass would go to: a ring under him in the team's colour while the pass is held (the line ends on him);
//   - the held shot: a line of the same make on the ice from the puck along the stick, ShotLineMinM at a tap growing to ShotLineMaxM at the full
//     charge (game.shot.charge_frames_max), chalk while it loads, the HUD's yellow when full (the shot lifts).
// "Hold = further / harder" reads the same for both: the longer the hold, the longer the line. Presentation only. jam.hud.ChargeLook 0 = neither.
#include "HUDWidget.h"
#include "MenuStyle.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HokejGameMode.h"
#include "JamSim.h"

static TAutoConsoleVariable<int32> CVarJamHudChargeLook(TEXT("jam.hud.ChargeLook"), 1,
	TEXT("Lane A213: 1 (default) while the pass is held its target mate has a ring in the team's colour; while the shot is held a line on the ice grows from the puck along the stick (yellow when full); 0 = neither."), ECVF_Default);
// SCREEN FAULT (A211's fourth list, 04:30): seen from behind him, the held shot's line stood up as a white slab at his shoulder for 10 to 12 frames
// (a screen-space bar between two projected points: a line running away from the camera collapses to a tall sliver near his body). OFF by default
// until a mark drawn in the ice's own plane replaces it. 1 = the line as READY 2 drew it.
static TAutoConsoleVariable<int32> CVarJamHudShotLine(TEXT("jam.hud.ShotLine"), 0,
	TEXT("Lane A213: the held shot's line on the ice. 0 (default since the fault of 04:30: it stood up as a slab seen end-on); 1 = drawn (READY 2's screen-space bar)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHudShotLookDemo(TEXT("jam.hud.ShotLookDemo"), 0,
	TEXT("Proof: N > 0 draws the shot's line whenever the human carries, as if the shot were held N frames (stills from a scripted human). 0 off."), ECVF_Default);

// Wave 4b (A211's fourth list: "the pill says A MATE IS OPEN, the puck rolls a metre and dies; no ring under the mate and no pass line"): a pass
// reads as one gesture: the line while held, the ring under the mate it goes to, the release, and the ring stays under that mate while the pass is in
// flight (Puck.PassFrames, Puck.PassBy = him), at most PassFlightRingFrames. A tap (no hold) gets the ring from its release frame.
static constexpr int32 PassFlightRingFrames = 45;
namespace { int32 GFlightTarget = -1; int64 GFlightUntil = -1; int32 GFlightPasses = -1; int32 GLastHeldFrames = 0; int32 GLastHeldTarget = -1; }

void UJamHudWidget::UpdateChargeLook(const FJamPresentationFrame& F)
{
	if (!Root || !WidgetTree) return;
	if (!PassTargetRing)
	{
		UImage* I = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("PassTargetRing"));
		if (UTexture2D* Plain = FJamUiStyle::Art(TEXT("T_UI_RingPlain"))) I->SetBrush(FJamUiStyle::ImageBrush(Plain, FVector2D(64.f, 64.f)));
		else I->SetBrush(FJamUiStyle::Ring(64.f));
		I->SetVisibility(ESlateVisibility::Collapsed);
		if (UCanvasPanelSlot* CS = Root->AddChildToCanvas(I)) { CS->SetAnchors(FAnchors(0.f, 0.f)); CS->SetAlignment(FVector2D(0.5f, 0.5f)); CS->SetAutoSize(false); CS->SetZOrder(-1); }
		PassTargetRing = I;
	}
	if (!ShotMeterBar)
	{
		UBorder* B = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ShotMeterBar"));
		B->SetBrush(FJamUiStyle::Rounded(FLinearColor::White, 2.f, FLinearColor(0.f, 0.f, 0.f, 0.9f), 1.5f));
		B->SetRenderTransformPivot(FVector2D(0.5, 0.5));
		B->SetVisibility(ESlateVisibility::Collapsed);
		if (UCanvasPanelSlot* CS = Root->AddChildToCanvas(B)) { CS->SetAnchors(FAnchors(0.f, 0.f)); CS->SetAlignment(FVector2D(0.5f, 0.5f)); CS->SetAutoSize(false); CS->SetZOrder(-1); }
		ShotMeterBar = B;
	}
	UImage* Ring = PassTargetRing.Get();
	UBorder* Bar = ShotMeterBar.Get();
	if (!Ring || !Bar) return;
	const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	APlayerController* PC = GetOwningPlayer();
	bool bRing = false, bBar = false;
	if (CVarJamHudChargeLook.GetValueOnGameThread() != 0 && F.Phase == EJamPhase::Play && Sim && PC && Sim->GetSkaters().IsValidIndex(Sim->GetHumanIndex()))
	{
		const TArray<FJamSkaterState>& Sk = Sim->GetSkaters();
		const FJamSkaterState& Hu = Sk[Sim->GetHumanIndex()];
		const bool bCarry = Sim->CarrierIndex() == Hu.Index;
		const FVector2D Stick = Hu.BladePos - Hu.Pos;
		// The pass's flight (wave 4b): his pass count grew: the ring goes under the mate it was aimed at (the held target, else the tap's own).
		if (GFlightPasses < 0 || Hu.Passes < GFlightPasses) GFlightPasses = Hu.Passes;
		if (Hu.Passes > GFlightPasses)
		{
			FVector2D AimR;
			GFlightTarget = GLastHeldTarget >= 0 ? GLastHeldTarget : Sim->HumanPassTarget(Hu.Index, Stick, FMath::Max(1, GLastHeldFrames), AimR);
			GFlightUntil = F.SimFrame + PassFlightRingFrames;
			GFlightPasses = Hu.Passes;
		}
		if (Hu.PassChargeFrames <= 0 && !bCarry) { GLastHeldFrames = 0; GLastHeldTarget = -1; }
		const FJamPuckState& PuckS = Sim->GetPuck();
		const bool bInFlight = GFlightTarget >= 0 && F.SimFrame < GFlightUntil && PuckS.PassFrames > 0 && PuckS.PassBy == Hu.Index && Sk.IsValidIndex(GFlightTarget);
		if (bInFlight)
		{
			FVector2D C, Size;
			float DistM = 0.f;
			if (ProjectGroundRing(Sk[GFlightTarget].Pos, CarrierRingRadiusM * 1.15f, C, Size, DistM))
			{
				const FLinearColor Team = F.GetTeamColour(Hu.Team);
				Ring->SetColorAndOpacity(FLinearColor(Team.R, Team.G, Team.B, 1.f));
				Ring->SetVisibility(ESlateVisibility::HitTestInvisible);
				if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(Ring->Slot)) { CS->SetPosition(C); CS->SetSize(Size); }
				bRing = true;
			}
		}
		// The held pass's mate.
		if (!bInFlight && bCarry && Sim->PassPowerOn() && Hu.PassChargeFrames > 0)
		{
			FVector2D Aim;
			const int32 T = Sim->HumanPassTarget(Hu.Index, Stick, Hu.PassChargeFrames, Aim);
			GLastHeldFrames = Hu.PassChargeFrames; GLastHeldTarget = T;
			FVector2D C, Size;
			float DistM = 0.f;
			if (Sk.IsValidIndex(T) && ProjectGroundRing(Sk[T].Pos, CarrierRingRadiusM * 1.15f, C, Size, DistM))
			{
				const FLinearColor Team = F.GetTeamColour(Hu.Team);
				Ring->SetColorAndOpacity(FLinearColor(Team.R, Team.G, Team.B, 1.f));
				Ring->SetVisibility(ESlateVisibility::HitTestInvisible);
				if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(Ring->Slot)) { CS->SetPosition(C); CS->SetSize(Size); }
				bRing = true;
			}
		}
		// The held shot's line.
		const int32 Demo = CVarJamHudShotLookDemo.GetValueOnGameThread();
		const int32 Charge = Hu.ChargeFrames > 0 ? Hu.ChargeFrames : (Demo > 0 && bCarry ? Demo : 0);
		if (bCarry && Charge > 0 && !Stick.IsNearlyZero() && CVarJamHudShotLine.GetValueOnGameThread() != 0)
		{
			const int32 Max = Sim->GetConfig().Shot.ChargeFramesMax;
			const double LenM = ShotLineM(Charge, Max);
			// The line starts ShotLineGapM past the puck: from the puck itself it ran up through his own body at the match camera (the w2b stills).
			const FVector2D Way = Stick.GetSafeNormal(), From = Sim->GetPuck().Pos + Way * ShotLineGapM, End = From + Way * LenM, Mid = From + Way * (LenM * 0.5), Across(-Way.Y, Way.X);
			const double ThickM = 0.22;   // the pass line's own thickness (HUDPassMeter.cpp): one make for both holds
			if (GroundLinesOn())
			{
				// Wave 4: in the ice's plane (the SCREEN FAULT's cure), never the single bar.
				const bool bFullG = Max > 0 && Charge >= Max;
				bBar = DrawGroundStrip(ShotStripPool, From, End, ThickM, 0.0, bFullG ? FJamUiStyle::Yellow() : FJamUiStyle::Paper(), 0.92f) > 0;
			}
			const FVector2D Pts[4] = { From, End, Mid - Across * (ThickM * 0.5), Mid + Across * (ThickM * 0.5) };
			FVector2D Px[4];
			bool bAll = true;
			for (int32 i = 0; i < 4 && bAll; ++i) bAll = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, UJamSim::ToWorld(Pts[i], 0.03), Px[i], false);
			FVector2D C, Sz, BoxMin, BoxMax;
			float AngleDeg = 0.f;
			if (!GroundLinesOn() && bAll && StopBarLayout(Px[0], Px[1], Px[2], Px[3], 4.f, C, Sz, AngleDeg, BoxMin, BoxMax))
			{
				const bool bFull = Max > 0 && Charge >= Max;
				const FLinearColor Col = bFull ? FJamUiStyle::Yellow() : FJamUiStyle::Paper();
				Bar->SetBrushColor(FLinearColor(Col.R, Col.G, Col.B, HoldLineAlpha(BoxMin, BoxMax, 0.92f)));
				Bar->SetRenderTransformAngle(AngleDeg);
				Bar->SetVisibility(ESlateVisibility::HitTestInvisible);
				if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(Bar->Slot)) { CS->SetPosition(C); CS->SetSize(Sz); }
				bBar = true;
			}
		}
	}
	if (!bRing) Ring->SetVisibility(ESlateVisibility::Collapsed);
	if (!bBar || GroundLinesOn()) Bar->SetVisibility(ESlateVisibility::Collapsed);
	if (!bBar) HideGroundStrip(ShotStripPool);
}
