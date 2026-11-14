// Radar: a 200 x 100 px mini-rink drawn in C++ from simulation positions (6 skater dots in team colours and the puck). Lane A5.
#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "HUDFeed.h"
#include "RadarWidget.generated.h"

USTRUCT(BlueprintType)
struct JAMGAME_API FJamRadarEntry
{
	GENERATED_BODY()

	// Centre of the dot in radar pixels, origin at the radar's top-left.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Radar")
	FVector2D PosPx = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Jam|Radar")
	float SizePx = 9.f;

	UPROPERTY(BlueprintReadOnly, Category = "Jam|Radar")
	FLinearColor Colour = FLinearColor::White;

	UPROPERTY(BlueprintReadOnly, Category = "Jam|Radar")
	bool bPuck = false;

	UPROPERTY(BlueprintReadOnly, Category = "Jam|Radar")
	bool bHuman = false;

	// Lane A196 (evolution 10): a man down: 1 the human's mate (a hollow square round him), 2 an opponent (an X over him); 0 a plain dot.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Radar")
	uint8 DownMark = 0;
};

class SJamRadar;

UCLASS()
class JAMGAME_API UJamRadarWidget : public UWidget
{
	GENERATED_BODY()

public:
	static constexpr float RadarWidthPx = 200.f;
	static constexpr float RadarHeightPx = 100.f;

	// radar x = (world_x_m + length/2) / length * 200, y = (world_z_m + width/2) / width * 100. Skaters first, the puck last.
	static void BuildEntries(const FJamPresentationFrame& Frame, TArray<FJamRadarEntry>& Out);

	UFUNCTION(BlueprintCallable, Category = "Jam|Radar")
	void SetFrame(const FJamPresentationFrame& Frame);

	UFUNCTION(BlueprintPure, Category = "Jam|Radar")
	const TArray<FJamRadarEntry>& GetEntries() const { return Entries; }

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	TArray<FJamRadarEntry> Entries;
	TSharedPtr<SJamRadar> Slate;
};
