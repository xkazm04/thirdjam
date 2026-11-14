#include "RadarWidget.h"
#include "Widgets/SLeafWidget.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

class SJamRadar : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SJamRadar) {}
	SLATE_END_ARGS()

	void Construct(const FArguments&) {}

	void SetEntries(const TArray<FJamRadarEntry>& In) { Entries = In; }

	virtual FVector2D ComputeDesiredSize(float) const override
	{
		return FVector2D(UJamRadarWidget::RadarWidthPx, UJamRadarWidget::RadarHeightPx);
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& Geo, const FSlateRect& Clip, FSlateWindowElementList& Out, int32 Layer,
		const FWidgetStyle& Style, bool bParentEnabled) const override
	{
		const FSlateBrush* White = FCoreStyle::Get().GetBrush("GenericWhiteBox");
		auto Box = [&](int32 L, FVector2D Pos, FVector2D Size, FLinearColor C)
		{
			FSlateDrawElement::MakeBox(Out, L, Geo.ToPaintGeometry(FVector2f(Size), FSlateLayoutTransform(FVector2f(Pos))), White, ESlateDrawEffect::None, C);
		};
		const float W = UJamRadarWidget::RadarWidthPx, H = UJamRadarWidget::RadarHeightPx;
		Box(Layer, FVector2D(0, 0), FVector2D(W, H), FLinearColor(0.02f, 0.05f, 0.10f, 0.65f));
		Box(Layer + 1, FVector2D(W * 0.5f - 0.5f, 0), FVector2D(1, H), FLinearColor(1, 1, 1, 0.45f));
		// frame
		const FLinearColor Edge(1, 1, 1, 0.35f);
		Box(Layer + 1, FVector2D(0, 0), FVector2D(W, 1), Edge);
		Box(Layer + 1, FVector2D(0, H - 1), FVector2D(W, 1), Edge);
		Box(Layer + 1, FVector2D(0, 0), FVector2D(1, H), Edge);
		Box(Layer + 1, FVector2D(W - 1, 0), FVector2D(1, H), Edge);
		int32 L = Layer + 2;
		for (const FJamRadarEntry& E : Entries)
		{
			if (E.bHuman)
			{
				Box(L, E.PosPx - FVector2D(8, 8), FVector2D(16, 16), FLinearColor(1.f, 0.9f, 0.2f, 1.f));  // yellow ring under the human
			}
			Box(L + 1, E.PosPx - FVector2D(E.SizePx * 0.5f, E.SizePx * 0.5f), FVector2D(E.SizePx, E.SizePx), E.Colour);
		}
		return L + 2;
	}

private:
	TArray<FJamRadarEntry> Entries;
};

void UJamRadarWidget::BuildEntries(const FJamPresentationFrame& F, TArray<FJamRadarEntry>& Out)
{
	Out.Reset();
	const float L = FMath::Max(F.RinkLengthM, 1.f), Wd = FMath::Max(F.RinkWidthM, 1.f);
	auto ToPx = [&](const FVector2D& M)
	{
		return FVector2D((M.X + L * 0.5f) / L * RadarWidthPx, (M.Y + Wd * 0.5f) / Wd * RadarHeightPx);
	};
	// Lane A188: the pickups first (drawn under the skaters): a 7 px square in the kind's colour.
	for (int32 I = 0; I < F.PickupsM.Num(); ++I)
	{
		FJamRadarEntry E;
		E.PosPx = ToPx(F.PickupsM[I]);
		E.SizePx = 7.f;
		E.Colour = F.PickupColours.IsValidIndex(I) ? F.PickupColours[I] : FLinearColor::White;
		Out.Add(E);
	}
	for (int32 I = 0; I < F.Skaters.Num(); ++I)
	{
		const FJamPresentationSkater& S = F.Skaters[I];
		FJamRadarEntry E;
		E.PosPx = ToPx(S.PosM);
		E.bHuman = S.bHuman || I == F.HumanIndex;
		E.SizePx = E.bHuman ? 12.f : 9.f;
		E.Colour = F.GetTeamColour(S.TeamId);
		Out.Add(E);
	}
	FJamRadarEntry P;
	P.PosPx = ToPx(F.PuckM);
	P.SizePx = 6.f;
	P.Colour = FLinearColor::White;
	P.bPuck = true;
	Out.Add(P);
}

void UJamRadarWidget::SetFrame(const FJamPresentationFrame& Frame)
{
	BuildEntries(Frame, Entries);
	if (Slate.IsValid())
	{
		Slate->SetEntries(Entries);
		Slate->Invalidate(EInvalidateWidgetReason::Paint);
	}
}

TSharedRef<SWidget> UJamRadarWidget::RebuildWidget()
{
	Slate = SNew(SJamRadar);
	Slate->SetEntries(Entries);
	return Slate.ToSharedRef();
}

void UJamRadarWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	Slate.Reset();
}
