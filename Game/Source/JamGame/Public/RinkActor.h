// The rink, built procedurally from game.rink at load (M0 greybox, M3b arena manipulation): ice, boards 1.2 m, glass 0.9 m above,
// emissive trim on the board top, two nets (goal_width x 1.5 high x 1.5 deep) at x = +-(length/2 - 4), goal trigger boxes behind
// the lines (cosmetic: the simulation decides goals), centre and faceoff markings, obstacles. Geometry only: gameplay collision
// lives in UJamSim. Stencils and device B's /JamAssets/Look instances per R-0002 (B-to-A-0005). Fallback materials are placeholders under /Game/Placeholders (M_Ice, M_Boards, M_Net, M_Trim) with the contract's
// parameter names (Wet, Scuff, Emissive, TeamColour); device B's masters drop in by replacing those assets or by SetMaterials.
// Lane A25 (R-0024 point 1, LOOK-LAB.md recommendation 6): the construction layer on top of the greybox: a 25 cm kick plate at the board foot, a dark glass rail with
// stanchions every 2.4 m, a 14 cm net frame with a back frame and braces, goal-light housings, benches and a penalty bench, and the ice markings B's paint does not
// carry (faceoff circles with hash marks, dots, creases, trapezoids). Visual only: the sim's boards, nets and goal line are untouched. `jam.rink.construction 0` hides it.
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
	// Lane A25: the construction's fixed sizes (they do not scale with the rink; the benches do).
	static constexpr double KickPlateHeightM = 0.25;
	static constexpr double KickPlateProudM = 0.02;
	static constexpr double StanchionSpacingM = 3.6;   // 2.4 read as a black comb at 24 m (eyes tool, EYES.md finding 2; Director 17:17)
	static constexpr double NetBarM = 0.14;
	static constexpr double NetBackHeightM = 0.9;

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
	// C-0005 / A15: flat zone decals on the ice (game.rink.zones), one section pair (fill, rim) per zone kind. Fallback only since
	// B-to-A-0016 (lane A21): a zone whose /JamAssets/Look/MI_Zone_<kind> exists is drawn as a ZoneQuads entry instead.
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> Zones;
	// B-to-A-0016: one engine plane per visible zone with a dynamic MI_Zone_<kind> (scalars Circle, Strength), 0.2 cm above the ice,
	// no shadow, no collision, stencil 0; scaled to (w, d, 1) m or (2r, 2r, 1) m so the material reads the zone's size from the UVs.
	UPROPERTY(Transient, VisibleAnywhere, Category = "Jam|Rink") TArray<TObjectPtr<UStaticMeshComponent>> ZoneQuads;
	// Lane A25: the construction layer. KickPlate: the 25 cm ochre band at the foot of the inner board face (section 0). GlassRail: the dark rail on top of the
	// glass (section 0) and the stanchions between the panels (section 1). GoalLights: a housing on the end-board rail behind each net (section 0) with a red
	// lens (section 1). Benches: two team benches and a penalty bench outside the +y boards, walls (section 0) and seats (section 1). All stencil 1, no collision.
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> KickPlate;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> GlassRail;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> GoalLights;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UProceduralMeshComponent> Benches;
	// Behind the +x goal line (team 0 scores here) and behind the -x line (team 1 scores here).
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UBoxComponent> GoalTriggerPos;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UBoxComponent> GoalTriggerNeg;

	// The dynamic instances, so look tiers and the Designer can set Wet / Scuff / Emissive / TeamColour.
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Jam|Rink") TObjectPtr<UMaterialInstanceDynamic> IceMid;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Jam|Rink") TObjectPtr<UMaterialInstanceDynamic> TrimMid;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Jam|Rink") TObjectPtr<UMaterialInstanceDynamic> TrimOrangeMid;

	// B-to-A-0018 point 5 (lane A21): the trim strips' Emissive, per tier from ULookTierSwitcher (night 30, clay 12); Build reads it,
	// SetTier pushes a change. Only B's MI_Trim_Blue / MI_Trim_Orange take it; the placeholder keeps its own scale and the beacon stays at 40.
	UFUNCTION(BlueprintCallable, Category = "Jam|Rink")
	void SetTrimEmissive(float Emissive);

	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	int32 GetZoneQuadCount() const { return ZoneQuads.Num(); }

	// Lane A25: shows or hides the construction layer (kick plate, rail, net frame extras, goal lights, benches, extra markings) without a rebuild;
	// `jam.rink.construction` drives it on every rink actor, so a ProfileGPU before/after pair is two console commands on one frame.
	UFUNCTION(BlueprintCallable, Category = "Jam|Rink")
	void SetConstructionVisible(bool bVisible);
	static bool IsConstructionEnabled();
	// Lane A25 (Director 17:17, eyes finding 3): the far-net beacon column read as noise and as a dithered artefact. `jam.rink.beacon` 0 = off, 1 = B's MI_Trim_Beacon
	// (translucent, as before), 2 = an opaque emissive column (lab flat instance in the trim colour). Build and the cvar sink apply it.
	void ApplyBeaconMode();
	static int32 GetBeaconMode();

	// The rounded-rectangle outline used for the ice and the boards (rink metres, counter-clockwise from +x).
	static void Outline(double LengthM, double WidthM, double CornerM, int32 ArcSegments, TArray<FVector2D>& Out);

private:
	UMaterialInterface* LoadMat(const TCHAR* Path) const;
	UMaterialInterface* LoadLook(const TCHAR* LookName, const TCHAR* FallbackPath, bool* bOutFromLook = nullptr) const;
	// Lane A25: B's /JamAssets/Look/<LookName> first; else lane A23's /Game/LookLab/<LabName> proposal with one log line; else the basic shape material tinted.
	// LabEmissive >= 0 lifts the lab instance's Emissive in a dynamic copy (the flat lab master has no ambient floor); -1 uses the instance as it is.
	// bLabTint also writes Tint into that copy (the lab instance's own colour is replaced: benches, goal-light housings).
	UMaterialInterface* LoadConstructionMat(const TCHAR* LookName, const TCHAR* LabName, const FLinearColor& Tint, float LabEmissive = -1.f, bool bLabTint = false);
	bool bLookIce = false;
	bool bLookTrim = false;
	FLinearColor BeaconColour = FLinearColor::White;   // game.look.trim_colour at the last Build, for the opaque beacon mode
};
