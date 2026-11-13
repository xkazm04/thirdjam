// The rink, built procedurally from game.rink at load (M0 greybox, M3b arena manipulation): ice, boards 1.2 m, glass 0.9 m above,
// emissive trim on the board top, two nets (goal_width x goal_height (0.61 by default, R-0033) x 1.5 deep) at x = +-(length/2 - 4), goal trigger boxes behind
// the lines (cosmetic: the simulation decides goals), centre and faceoff markings, obstacles. Geometry only: gameplay collision
// lives in UJamSim. Stencils and device B's /JamAssets/Look instances per R-0002 (B-to-A-0005). Fallback materials are placeholders under /Game/Placeholders (M_Ice, M_Boards, M_Net, M_Trim) with the contract's
// parameter names (Wet, Scuff, Emissive, TeamColour); device B's masters drop in by replacing those assets or by SetMaterials.
// Lane A25 (R-0024 point 1, LOOK-LAB.md recommendation 6): the construction layer on top of the greybox: a 25 cm kick plate at the board foot, a dark glass rail with
// stanchions every 2.4 m, a 14 cm net frame with a back frame and braces, goal-light housings, benches and a penalty bench, and the ice markings B's paint does not
// carry (faceoff circles with hash marks, dots, creases, trapezoids). Visual only: the sim's boards, nets and goal line are untouched. `jam.rink.construction 0` hides it.
// Lane A103 (track F3, the arena in Blender): the benches and the penalty boxes as static meshes made by tools/blender/arena/arena.py and imported to /Game/Arena
// (two team benches outside the +y boards where A25's code benches stand, two penalty boxes outside the -y boards at x = -3 and +3, where UJamSim::BoxPos pins a
// boxed skater). Device B's instances by reference on the five slots. Shown on the dressed tiers, hidden on the bare ones; `jam.rink.arena 0` or a missing mesh
// gives back A25's code benches. Visual only: no collision, no sim state. docs/experiments/ARENA.md.
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
	// Lane A64: the emissive fill B's MI_Toon_Bumper may carry at most (presentation only; the toon ramp needs the light to show the round).
	static constexpr float BumperEmissiveCap = 0.8f;
	// Lane A25: the construction's fixed sizes (they do not scale with the rink; the benches do).
	static constexpr double KickPlateHeightM = 0.25;
	static constexpr double KickPlateProudM = 0.02;
	static constexpr double StanchionSpacingM = 3.6;   // 2.4 read as a black comb at 24 m (eyes tool, EYES.md finding 2; Director 17:17)
	static constexpr double NetBarM = 0.14;
	static constexpr double NetBackHeightM = 0.9;
	// Lane A103: the Blender-made arena dressing. The bench mesh is built 12 m long (0.1 x a 120 m rink) and scaled along the boards to 0.1 x the rink's length;
	// the penalty box is 4.4 m on every rink, centred where the sim boxes a skater (UJamSim::BoxPos: x = -3 team 0, +3 team 1, on the -y side).
	static constexpr const TCHAR* TeamBenchMeshPath = TEXT("/Game/Arena/SM_Arena_TeamBench.SM_Arena_TeamBench");
	static constexpr const TCHAR* PenaltyBoxMeshPath = TEXT("/Game/Arena/SM_Arena_PenaltyBox.SM_Arena_PenaltyBox");
	static constexpr double ArenaBenchBuiltLengthM = 12.0;
	static constexpr double ArenaBenchLengthFactor = 0.1;
	static constexpr double ArenaBenchCentreFactor = 0.0833;
	static constexpr double PenaltyBoxCentreXM = 3.0;
	static constexpr double ArenaGapM = 0.05;   // between the board's outer face and a piece (A25's benches: the same 5 cm)
	// Lane A119 (track F3, evolution 2): the goals and the scorer's table. A goal mesh is built by arena.py for one opening (game.rink.goal_width_m x
	// goal_height_m: its posts' inner faces stand at +-W/2 and its crossbar's underside at H, the sim's goal volume); Build takes the variant nearest the
	// cartridge's goal (the table is in RinkActor.cpp) and scales it W/Wv along the goal line and H/Hv up, which keeps the opening exact. The scorer's table
	// stands between the two penalty boxes at x = 0 on the -y side.
	static constexpr const TCHAR* ScorerTableMeshPath = TEXT("/Game/Arena/SM_Arena_ScorerTable.SM_Arena_ScorerTable");

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
	// Lane A103: the Blender-made benches (+y side; Neg = x < 0, team 0, blue; Pos = x > 0, team 1, orange) and penalty boxes (-y side, turned 180 degrees).
	// Stencil 1, no collision. The mesh is loaded at Build; without it the component stays empty and hidden and A25's code benches show.
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaBenchNeg;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaBenchPos;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaBoxNeg;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaBoxPos;
	// Lane A119: the Blender-made goals (Neg = the -x end, yaw +90; Pos = the +x end, yaw -90: the mesh's +y runs into the net) and the scorer's table.
	// While the goals show, the code frame and its sheet netting (`Nets`) are hidden; `jam.rink.arena 0`, a bare tier or a missing mesh gives them back.
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaGoalNeg;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaGoalPos;
	// The goal's net is a mesh of its own on the same transform, WITHOUT the custom depth stencil: on the outlined tiers every inked cord made the net a black
	// comb from the game camera (off-screen engine stills on l2_night, ARENA.md); the frame keeps the stencil and its outline.
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaGoalNetNeg;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaGoalNetPos;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaTable;
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

	// Lane A69 (B-to-A-0034 point 3 / 0035 point 4a): on p3_clay each zone that has a ZoneQuad also gets an opaque ring mesh on its outline (6 cm high, 12 cm wide,
	// B's MI_Zone_<kind> on it, so it shows in the ice's reflections); the decal quad is hidden there. Parallel to ZoneQuads (same index). Built with the rink,
	// shown or hidden by SetZoneRingsForTier; there is no per-switch spawn, so a tier switch can leave nothing behind.
	UPROPERTY(Transient, VisibleAnywhere, Category = "Jam|Rink") TArray<TObjectPtr<UProceduralMeshComponent>> ZoneRings;
	UFUNCTION(BlueprintCallable, Category = "Jam|Rink")
	void SetZoneRingsForTier(const FString& Tier);
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	int32 GetZoneRingCount() const { return ZoneRings.Num(); }
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	bool AreZoneRingsShown() const { return bClayTier; }

	// Lane A31 (B-to-A-0023 point 2, 0025): the glass panel's instance per look tier: /JamAssets/Look/MI_Bcast_Glass on p4_broadcast, MI_Glass on every
	// other tier (the net placeholder stands in when B's instance is absent). Build reads game.look.tier; ULookTierSwitcher::SetTier pushes a change.
	UFUNCTION(BlueprintCallable, Category = "Jam|Rink")
	void SetGlassForTier(const FString& Tier);
	// The look name the glass carries right now ("MI_Glass", "MI_Bcast_Glass", or "placeholder"), for the PIE proof.
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	FString GetGlassLookName() const { return GlassLookName; }

	// Lane A25: shows or hides the construction layer (kick plate, rail, net frame extras, goal lights, benches, extra markings) without a rebuild;
	// `jam.rink.construction` drives it on every rink actor, so a ProfileGPU before/after pair is two console commands on one frame.
	UFUNCTION(BlueprintCallable, Category = "Jam|Rink")
	void SetConstructionVisible(bool bVisible);
	static bool IsConstructionEnabled();

	// Lane A103: the arena dressing's switch (`jam.rink.arena`, default 1) and the tier rule. A bare tier is one whose rig C-0002 lists as an empty placeholder
	// (l1_day, l3_neon): nothing lights or dresses the outside of the boards there, so the Blender pieces hide and A25's code benches stay.
	static bool IsArenaDressingEnabled();
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	static bool IsBareTier(const FString& Tier);
	// True when the four Blender pieces are what the rink shows (meshes loaded, construction on, the cvar on, a dressed tier); A25's code benches are hidden then.
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	bool IsArenaDressingShown() const { return bArenaShown; }
	// Lane A119: true when the Blender goals are what stands on the goal lines (the code frame `Nets` is hidden then).
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	bool IsArenaGoalShown() const { return bArenaGoalShown; }
	// Where a boxed skater of Team (0 or 1) would stand inside its penalty box, facing the rink: the box mesh's socket `Seat` in world space.
	// False when the box is not shown (the caller keeps the sim's place on the ice). Presentation only: UJamSim::BoxPos is not changed by this lane.
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	bool GetPenaltyBoxSeat(int32 Team, FVector& OutLocation, FRotator& OutRotation) const;
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
	void ApplyArenaDressing();   // lane A103: shows the Blender pieces or A25's code benches, never both
	FString ArenaTier;           // lane A103: the tier the dressing was last decided for (Build and SetZoneRingsForTier write it)
	bool bArenaShown = false;
	bool bArenaGoalShown = false;   // lane A119
	bool bClayTier = false;   // lane A69: the tier the rink shows is p3_clay (rings on, decal quads off, glass and rail not drawn)
	bool bLookIce = false;
	bool bLookTrim = false;
	FString GlassLookName = TEXT("placeholder");
	FLinearColor BeaconColour = FLinearColor::White;   // game.look.trim_colour at the last Build, for the opaque beacon mode
};
