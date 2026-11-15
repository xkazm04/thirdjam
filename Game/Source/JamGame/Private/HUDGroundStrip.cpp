// Lane A213, wave 4 (the SCREEN FAULT of 04:30): a mark "on the ice" drawn as ONE screen-space bar between two projected points stands up when the
// line runs away from the camera: its thickness is taken at one point and kept for the whole projected length, so seen from behind the human it is a
// tall slab at his shoulder (the eye's strips r10w f1137-1146). DrawGroundStrip lays the strip in the ice's own plane instead: it is cut into short
// pieces along its length, each laid out from its own four projected corners (StopBarLayout), so a piece near the camera is wide and a far one
// narrow: the strip tapers with the ice like a painted line and never stands up. It starts no nearer than StartM past the first point (the puck's far
// side), and a piece whose box meets the human figure's box is drawn at OverBodyAlpha. Presentation only.
#include "HUDWidget.h"
#include "MenuStyle.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "GameFramework/PlayerController.h"
#include "JamSim.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<int32> CVarJamHudGroundLines(TEXT("jam.hud.GroundLines"), 1,
	TEXT("Lane A213 (wave 4): 1 (default) the hold lines and the stop's line are drawn in the ice's plane (tapering pieces); 0 = one screen-space bar each (it stood up as a slab seen end-on)."), ECVF_Default);
bool UJamHudWidget::GroundLinesOn() { return CVarJamHudGroundLines.GetValueOnGameThread() != 0; }

int32 UJamHudWidget::DrawGroundStrip(TArray<TObjectPtr<UBorder>>& Pool, const FVector2D& FromM, const FVector2D& ToM, double WidthM, double StartM,
	const FLinearColor& Colour, float Alpha)
{
	APlayerController* PC = GetOwningPlayer();
	int32 Drawn = 0;
	const FVector2D Way = (ToM - FromM).GetSafeNormal();
	const double Len = FVector2D::Distance(FromM, ToM) - StartM;
	if (PC && Root && WidgetTree && !Way.IsNearlyZero() && Len > 0.05)
	{
		const int32 N = FMath::Clamp(FMath::CeilToInt32(Len / GroundStripPieceM), 1, GroundStripMaxPieces);
		const FVector2D Across(-Way.Y, Way.X), A0 = FromM + Way * StartM;
		const double Step = Len / N;
		while (Pool.Num() < N)
		{
			UBorder* B = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
			B->SetBrush(FJamUiStyle::Rounded(FLinearColor::White, 0.f, FLinearColor(0.f, 0.f, 0.f, 0.55f), 1.f));   // a thin ink edge: chalk reads on pale ice only with one
			B->SetRenderTransformPivot(FVector2D(0.5, 0.5));
			B->SetVisibility(ESlateVisibility::Collapsed);
			if (UCanvasPanelSlot* CS = Root->AddChildToCanvas(B)) { CS->SetAnchors(FAnchors(0.f, 0.f)); CS->SetAlignment(FVector2D(0.5f, 0.5f)); CS->SetAutoSize(false); CS->SetZOrder(-1); }
			Pool.Add(B);
		}
		for (int32 i = 0; i < N; ++i)
		{
			UBorder* B = Pool[i].Get();
			if (!B) continue;
			// Pieces overlap a hair (2 %) so the strip has no seams.
			const FVector2D P0 = A0 + Way * (Step * i - Step * 0.02), P1 = A0 + Way * (Step * (i + 1) + Step * 0.02), Mid = (P0 + P1) * 0.5;
			const FVector2D Pts[4] = { P0, P1, Mid - Across * (WidthM * 0.5), Mid + Across * (WidthM * 0.5) };
			FVector2D Px[4];
			bool bAll = true;
			for (int32 k = 0; k < 4 && bAll; ++k) bAll = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, UJamSim::ToWorld(Pts[k], 0.02), Px[k], false);
			FVector2D C, Sz, BoxMin, BoxMax;
			float Deg = 0.f;
			if (!bAll || !StopBarLayout(Px[0], Px[1], Px[2], Px[3], 1.f, C, Sz, Deg, BoxMin, BoxMax)) { B->SetVisibility(ESlateVisibility::Collapsed); continue; }
			B->SetBrushColor(FLinearColor(Colour.R, Colour.G, Colour.B, HoldLineAlpha(BoxMin, BoxMax, Alpha)));
			B->SetRenderTransformAngle(Deg);
			B->SetVisibility(ESlateVisibility::HitTestInvisible);
			if (UCanvasPanelSlot* CS = Cast<UCanvasPanelSlot>(B->Slot)) { CS->SetPosition(C); CS->SetSize(Sz); }
			++Drawn;
		}
		for (int32 i = N; i < Pool.Num(); ++i) if (Pool[i]) Pool[i]->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		HideGroundStrip(Pool);
	}
	return Drawn;
}

void UJamHudWidget::HideGroundStrip(TArray<TObjectPtr<UBorder>>& Pool)
{
	for (TObjectPtr<UBorder>& B : Pool) if (B) B->SetVisibility(ESlateVisibility::Collapsed);
}
