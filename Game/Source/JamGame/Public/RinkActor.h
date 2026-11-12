// The rink, built procedurally from game.rink at load (M0 greybox, M3b arena manipulation): ice, boards 1.2 m, glass 0.9 m above,
// emissive trim on the board top, two nets (goal_width x 1.5 high x 1.5 deep) at x = +-(length/2 - 4), goal trigger boxes behind
// the lines (cosmetic: the simulation decides goals), centre and faceoff markings, obstacles. Geometry only: gameplay collision
// lives in UJamSim. Stencils and device B's /JamAssets/Look instances per R-0002 (B-to-A-0005). Fallback materials are placeholders under /Game/Placeholders (M_Ice, M_Boards, M_Net, M_Trim) with the contract's
// parameter names (Wet, Scuff, Emissive, TeamColour); device B's masters drop in by replacing those assets or by SetMaterials.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JamCartridge.h"
#include "RinkActor.generated.h"

class UProceduralMeshComponent;
class UBoxComponent;
class UStaticMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS()
class JAMGAME_API ARinkActor : public AActor
{
	GENERATED_BODY()

public:
	ARinkActor();

	// Rebuilds everything from the config (safe to call again after a Director's Call changes the rink).
	UFUNCTION(BlueprintCallable, Category = "Jam|Rink")
	void Build(const FJamGameConfig& Game);

	// Placeholder material paths; a missing asset falls back to the engine's basic shape material.
	static constexpr const TCHAR* IceMaterialPath = TEXT("/Game/Placeholders/M_Ice.M_Ice");
	static constexpr const TCHAR* BoardsMaterialPath = TEXT("/Game/Placeholders/M_Boards.M_Boards");
	static constexpr const TCHAR* NetMaterialPath = TEXT("/Game/Placeholders/M_Net.M_Net");
	static constexpr const TCHAR* TrimMaterialPath = TEXT("/Game/Placeholders/M_Trim.M_Trim");

	static constexpr double BoardHeightM = 1.2;
	static constexpr double GlassHeightM = 0.9;
	static constexpr double BoardThicknessM = 0.2;

	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> Ice;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> Markings;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> Boards;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> Glass;
	// R-0002: 10 cm emissive strips on the board tops, blue half (x < 0, MI_Trim_Blue) and orange half (x > 0, MI_Trim_Orange).
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> Trim;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> TrimOrange;
	// R-0002: far-net beacon posts behind the +x and -x nets (MI_Trim_Beacon).
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> BeaconPos;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> BeaconNeg;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> Nets;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> Obstacles;
	// Behind the +x goal line (team 0 scores here) and behind the -x line (team 1 scores here).
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UBoxComponent> GoalTriggerPos;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UBoxComponent> GoalTriggerNeg;

	// The dynamic instances, so look tiers and the Designer can set Wet / Scuff / Emissive / TeamColour.
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Jam|Rink") TObjectPtr<UMaterialInstanceDynamic> IceMid;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Jam|Rink") TObjectPtr<UMaterialInstanceDynamic> TrimMid;

	// The rounded-rectangle outline used for the ice and the boards (rink metres, counter-clockwise from +x).
	static void Outline(double LengthM, double WidthM, double CornerM, int32 ArcSegments, TArray<FVector2D>& Out);

private:
	UMaterialInterface* LoadMat(const TCHAR* Path) const;
	UMaterialInterface* LoadLook(const TCHAR* LookName, const TCHAR* FallbackPath, bool* bOutFromLook = nullptr) const;
	bool bLookIce = false;
};
