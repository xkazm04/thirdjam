// Lane A207 (track P1, the night of 14 Nov; docs/experiments/PASS-SHOT.md): the pass's power by the stick. Presentation only: UpdatePassMeter reads the
// sim (the human's held pass: PassChargeFrames under game.pass.power) and writes nothing back. While he holds the pass with the puck, a chalk line lies on
// the ice from the puck as far as the pass would still be a pass (it slows to pass.power.arrive_mps there: UJamSim::PassReachM): a tap draws a short line,
// a full hold reaches a third of the rink. It points where the pass will go: at the mate inside the stick's cone (UJamSim::HumanPassTarget, then drawn in
// his team's colour and ending at the meeting point when that is nearer), along the stick otherwise (chalk). jam.hud.PassMeter 0 = no line.
#include "HUDWidget.h"
#include "MenuStyle.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HokejGameMode.h"
#include "JamSim.h"
#include "UnrealClient.h"

// Lane A219 (R-0069 point 3 and R-0070, 15 Nov: "no aim line needed"): OFF by default. The line is a strip of the game's mechanics on the ice from the puck
// along his aim: it lay across his own figure whenever the mate stood beyond him (package H39, f503: a translucent blue bar from his skates to his
// shoulder), paper-white on a short hold, his team's blue once the hold found a mate. The mate's ring in his colour (jam.hud.ChargeLook) says who gets
// the pass. 1 = package H39's line.
static TAutoConsoleVariable<int32> CVarJamHudPassMeter(TEXT("jam.hud.PassMeter"), 0,
	TEXT("Lane A207: the pass's power line on the ice while the pass is held: 0 (default from lane A219, R-0070: no line on the ice or across the figure; the target mate's ring says it) none; 1 = drawn (package H39)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHudPassMeterDemo(TEXT("jam.hud.PassMeterDemo"), 0,
	TEXT("Proof: N > 0 draws the line whenever the human carries, as if the pass were held N frames (for stills from a scripted human, who taps). 0 off."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamHudPassMeterStills(TEXT("jam.hud.PassMeterStills"), 0,
	TEXT("Proof: N > 0 takes a still (with the HUD) N sim frames into every showing of the line, at most 12: u1-passmeter-f<frame>.png. 0 off."), ECVF_Default);
namespace { int64 GPassMeterSince = -1; int32 GPassMeterStills = 0; bool GPassMeterTaken = false; }

void UJamHudWidget::UpdatePassMeter(const FJamPresentationFrame& F)
{
	if (!Root || !WidgetTree) return;
	if (!PassMeterBar)
	{
		UBorder* B = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PassMeterBar"));
		B->SetBrush(FJamUiStyle::Rounded(FLinearColor::White, 2.f, FLinearColor(0.f, 0.f, 0.f, 0.9f), 1.5f));
		B->SetRenderTransformPivot(FVector2D(0.5, 0.5));
		B->SetVisibility(ESlateVisibility::Collapsed);
		if (UCanvasPanelSlot* CS = Root->AddChildToCanvas(B))
		{
			CS->SetAnchors(FAnchors(0.f, 0.f));
			CS->SetAlignment(FVector2D(0.5f, 0.5f));
			CS->SetAutoSize(false);
			CS->SetZOrder(-1);   // on the ice, under every other element
		}
		PassMeterBar = B;
	}
	UBorder* Bar = PassMeterBar.Get();
	if (!Bar) return;
	const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	APlayerController* PC = GetOwningPlayer();
	bool bShown = false;
	int32 Target = -1;
	double ReachM = 0.0;
	if (CVarJamHudPassMeter.GetValueOnGameThread() != 0 && F.Phase == EJamPhase::Play && Sim && PC && Sim->PassPowerOn() && Sim->GetSkaters().IsValidIndex(Sim->GetHumanIndex()))
	{
		const FJamSkaterState& Hu = Sim->GetSkaters()[Sim->GetHumanIndex()];
		const int32 Demo = CVarJamHudPassMeterDemo.GetValueOnGameThread();
		const int32 Held = Hu.PassChargeFrames > 0 ? Hu.PassChargeFrames : (Demo > 0 && Sim->CarrierIndex() == Hu.Index ? Demo : 0);
		if (Held > 0)
		{
			const FVector2D From = Sim->GetPuck().Pos;
			FVector2D Aim;
			Target = Sim->HumanPassTarget(Hu.Index, Hu.BladePos - Hu.Pos, Held, Aim);
			ReachM = Sim->PassReachM(Sim->PassSpeedForHold(Held));
			const FVector2D Way = (Aim - From).GetSafeNormal();
			const double LenM = FMath::Max(1.0, Target >= 0 ? FMath::Min(ReachM, FVector2D::Distance(Aim, From)) : ReachM);
			const FVector2D End = From + Way * LenM, Mid = From + Way * (LenM * 0.5), Across(-Way.Y, Way.X);
			const double ThickM = 0.22;
			if (GroundLinesOn())
			{
				// Lane A213 (wave 4, the SCREEN FAULT): the line in the ice's plane, from past the puck's far side (it stood up seen end-on as one bar).
				const FLinearColor Col = Target >= 0 ? F.GetTeamColour(Hu.Team) : FJamUiStyle::Paper();
				bShown = DrawGroundStrip(PassStripPool, From, End, ThickM, 0.15, Col, 0.9f) > 0;
			}
			const FVector2D Pts[4] = { From, End, Mid - Across * (ThickM * 0.5), Mid + Across * (ThickM * 0.5) };
			FVector2D Px[4];
			bool bAll = !Way.IsNearlyZero();
			for (int32 i = 0; i < 4 && bAll; ++i) bAll = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, UJamSim::ToWorld(Pts[i], 0.03), Px[i], false);
			FVector2D C, Sz, BoxMin, BoxMax;
			float AngleDeg = 0.f;
			if (!GroundLinesOn() && bAll && StopBarLayout(Px[0], Px[1], Px[2], Px[3], 3.f, C, Sz, AngleDeg, BoxMin, BoxMax))
			{
				const FLinearColor Col = Target >= 0 ? F.GetTeamColour(Hu.Team) : FJamUiStyle::Paper();
				Bar->SetBrushColor(FLinearColor(Col.R, Col.G, Col.B, HoldLineAlpha(BoxMin, BoxMax, 0.9f)));   // lane A213 (the look): fainter over his own body
				Bar->SetRenderTransformAngle(AngleDeg);
				Bar->SetVisibility(ESlateVisibility::HitTestInvisible);
				if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(Bar->Slot)) { CS->SetPosition(C); CS->SetSize(Sz); }
				bShown = true;
			}
		}
	}
	if (!bShown || GroundLinesOn()) Bar->SetVisibility(ESlateVisibility::Collapsed);
	if (!bShown) HideGroundStrip(PassStripPool);   // lane A213 (wave 4)
	{
		// The proof's stills: one per showing, After sim frames into it.
		const int32 After = CVarJamHudPassMeterStills.GetValueOnGameThread();
		if (After <= 0 || !bShown) { GPassMeterSince = -1; GPassMeterTaken = false; }
		else
		{
			if (GPassMeterSince < 0 || F.SimFrame < GPassMeterSince) { GPassMeterSince = F.SimFrame; GPassMeterTaken = false; }
			if (!GPassMeterTaken && GPassMeterStills < 12 && F.SimFrame - GPassMeterSince >= After)
			{
				GPassMeterTaken = true; ++GPassMeterStills;
				const FString Name = FString::Printf(TEXT("u1-passmeter-f%06lld"), F.SimFrame);
				FScreenshotRequest::RequestScreenshot(Name + TEXT(".png"), true, false);
				UE_LOG(LogTemp, Log, TEXT("JAMHUD frame=%lld pass_meter_still=%s.png target=%d reach_m=%.1f"), F.SimFrame, *Name, Target, ReachM);
			}
		}
	}
	const int32 Now = bShown ? (Target >= 0 ? 2 : 1) : 0;
	if (Now != PassMeterState)
	{
		UE_LOG(LogTemp, Log, TEXT("JAMHUD frame=%lld pass_meter=%s target=%d reach_m=%.1f"), F.SimFrame, Now == 2 ? TEXT("mate") : Now == 1 ? TEXT("stick") : TEXT("off"), Target, ReachM);
		PassMeterState = Now;
	}
}
