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

static TAutoConsoleVariable<int32> CVarJamHudPassMeter(TEXT("jam.hud.PassMeter"), 1,
	TEXT("Lane A207: the pass's power line on the ice while the pass is held (1, default); 0 = none."), ECVF_Default);

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
		if (Hu.PassChargeFrames > 0)
		{
			const FVector2D From = Sim->GetPuck().Pos;
			FVector2D Aim;
			Target = Sim->HumanPassTarget(Hu.Index, Hu.BladePos - Hu.Pos, Hu.PassChargeFrames, Aim);
			ReachM = Sim->PassReachM(Sim->PassSpeedForHold(Hu.PassChargeFrames));
			const FVector2D Way = (Aim - From).GetSafeNormal();
			const double LenM = FMath::Max(1.0, Target >= 0 ? FMath::Min(ReachM, FVector2D::Distance(Aim, From)) : ReachM);
			const FVector2D End = From + Way * LenM, Mid = From + Way * (LenM * 0.5), Across(-Way.Y, Way.X);
			const double ThickM = 0.22;
			const FVector2D Pts[4] = { From, End, Mid - Across * (ThickM * 0.5), Mid + Across * (ThickM * 0.5) };
			FVector2D Px[4];
			bool bAll = !Way.IsNearlyZero();
			for (int32 i = 0; i < 4 && bAll; ++i) bAll = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, UJamSim::ToWorld(Pts[i], 0.03), Px[i], false);
			FVector2D C, Sz, BoxMin, BoxMax;
			float AngleDeg = 0.f;
			if (bAll && StopBarLayout(Px[0], Px[1], Px[2], Px[3], 3.f, C, Sz, AngleDeg, BoxMin, BoxMax))
			{
				const FLinearColor Col = Target >= 0 ? F.GetTeamColour(Hu.Team) : FJamUiStyle::Paper();
				Bar->SetBrushColor(FLinearColor(Col.R, Col.G, Col.B, 0.9f));
				Bar->SetRenderTransformAngle(AngleDeg);
				Bar->SetVisibility(ESlateVisibility::HitTestInvisible);
				if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(Bar->Slot)) { CS->SetPosition(C); CS->SetSize(Sz); }
				bShown = true;
			}
		}
	}
	if (!bShown) Bar->SetVisibility(ESlateVisibility::Collapsed);
	const int32 Now = bShown ? (Target >= 0 ? 2 : 1) : 0;
	if (Now != PassMeterState)
	{
		UE_LOG(LogTemp, Log, TEXT("JAMHUD frame=%lld pass_meter=%s target=%d reach_m=%.1f"), F.SimFrame, Now == 2 ? TEXT("mate") : Now == 1 ? TEXT("stick") : TEXT("off"), Target, ReachM);
		PassMeterState = Now;
	}
}
