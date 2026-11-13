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
// Lane A127 (track F3, evolution 3): the arena answers the game. On a `goal` event the scorer's table's lamp and the goal light behind the scored net flash and
// the scored net's cords shake; on a hard skater `board` event the bench, box or table behind that stretch of boards rocks. The rink listens to UJamEventBus
// itself (BeginPlay) and counts the bus's sim frames, never wall clock. Presentation only: parameters on dynamic material instances and component transforms,
// no physics body, nothing the sim reads. `jam.rink.react 0` (or `jam.rink.arena 0`) gives back the still arena.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JamCartridge.h"
#include "JamEventBus.h"
#include "RinkActor.generated.h"

class UProceduralMeshComponent;
class UBoxComponent;
class UStaticMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UPointLightComponent;

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
	// Lane A127 (evolution 3): the goal light, a clay post with a red lamp outside the end boards behind each net (arena.py goal_light(); slots dark, kick, lamp).
	static constexpr const TCHAR* GoalLightMeshPath = TEXT("/Game/Arena/SM_Arena_GoalLight.SM_Arena_GoalLight");
	// The reactions' numbers (sim frames at 60 Hz). The lamp: lit on the goal's frame, pulsing every LampPulseFrames, fading over the last LampFadeFrames, at its
	// resting value again from LampFlashFrames on (the bar: back by frame 120). The net: a damped wobble of the scored net's scale (depth and height) for
	// NetShakeFrames. The rattle: a skater `board` event at RattleMinStrength or more (Strength = normal speed / game.skater.speed) rocks the piece standing
	// within RattleReachM of the hit along the boards about its rink-side foot, RattleMaxDeg at Strength 1, for RattleFrames.
	static constexpr int32 LampFlashFrames = 108;
	static constexpr int32 LampPulseFrames = 18;
	static constexpr int32 LampFadeFrames = 36;
	static constexpr float LampPeakEmissive = 14.f;
	static constexpr int32 NetShakeFrames = 54;
	static constexpr float NetShakeDepth = 0.12f;
	static constexpr float NetShakeHeight = 0.06f;
	static constexpr int32 RattleFrames = 18;
	static constexpr float RattleMinStrength = 0.45f;
	static constexpr float RattleMaxDeg = 3.5f;
	static constexpr double RattleReachM = 2.0;
	// The lamp lights (candelas at the top of a flash = `jam.rink.react.light`, default 3000, picked from a sweep of off-screen stills at 100 / 1,000 / 10,000 /
	// 100,000: 1,000 is a halo and a reflection on clay, 10,000 washes the end zone, 100,000 blows the picture out). The scorer's table's light stands 1.5 m
	// from the boxes' walls and takes TableLightShare of it; on l2_night the same candelas read far stronger against the dark and take NightLightShare.
	static constexpr float TableLightShare = 0.25f;
	static constexpr float NightLightShare = 0.4f;

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
	// Lane A127: the Blender-made goal lights outside the end boards (Neg = behind the -x net, Pos = behind the +x net). While they show, A25's code housings
	// on the glass rail (`GoalLights`) are hidden: one light behind each net, never both.
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaGoalLightNeg;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UStaticMeshComponent> ArenaGoalLightPos;
	// Lane A127: a red point light over each lamp (0 the scorer's table's, 1 the goal light behind the -x net, 2 behind the +x net): no shadows, hidden and at
	// intensity 0 at rest; during a flash it follows the lamp's curve up to `jam.rink.react.light` candelas, so the boards, the ice and the net behind the
	// scored goal pulse red with the lamp (the first engine stills: a lamp's Emissive alone is a small flat dot from the game camera).
	UPROPERTY(VisibleAnywhere, Category = "Jam|Rink") TObjectPtr<UPointLightComponent> ArenaLampLights[3];
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
	// ---- Lane A127 (evolution 3): the arena answers the game ----
	// The pure curves (tests read them). LampAt: 0 outside [0, LampFlashFrames), else 0..1 (1 on the goal's frame). NetShakeAt: 0 outside [0, NetShakeFrames),
	// else a damped sine in -1..1. RattleAt: 0 outside [0, RattleFrames), else a damped cosine times the clamped Strength.
	static float LampAt(int64 FramesSince);
	static float NetShakeAt(int64 FramesSince);
	static float RattleAt(int64 FramesSince, float Strength);
	// `jam.rink.react` (default 1): 0 = the pieces stand and never move or light.
	static bool AreReactionsEnabled();
	// The bus's delivery (what OnJamEvent calls with the bus's sim frame): `goal` starts the lamp, the scored end's goal light and net; a skater `board` at
	// RattleMinStrength or more starts a rattle. Applies the reaction's first frame at once. Does nothing while the Blender pieces are not shown.
	UFUNCTION(BlueprintCallable, Category = "Jam|Rink")
	void ReactToEvent(FName Event, const FJamEventPayload& Payload, int64 SimFrame);
	// Writes every live reaction's value for SimFrame (the actor's tick calls it with the bus's frame; a frame before the reaction's start ends it).
	UFUNCTION(BlueprintCallable, Category = "Jam|Rink")
	void AdvanceReactions(int64 SimFrame);
	// Listens to this bus (BeginPlay passes the game instance's); null stops listening.
	void BindBus(UJamEventBus* InBus);
	// Read-outs for the proofs. The lamp's Emissive as its dynamic instance holds it (the scorer's table), and its resting value; -1 when there is no lamp.
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	float GetLampEmissive() const;
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	float GetLampRestEmissive() const { return LampRest[0]; }
	// The goal light's Emissive behind the +x net (End > 0) or the -x net; -1 when there is none.
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	float GetGoalLightEmissive(int32 End) const;
	// The lamp light's intensity right now in candelas (Index as ArenaLampLights), 0 while it is hidden; and the peak a flash drives it to.
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	float GetLampLightIntensity(int32 Index) const;
	static float LampLightPeak();
	// The peak of one lamp's light on this rink's tier: the cvar times the table's share (Index 0) and the night tier's share.
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	float LampLightPeakAt(int32 Index) const;
	// The net's scale over its resting scale at the +x end (End > 0) or the -x end ((1, 1, 1) at rest), and the rattled piece's roll in degrees right now.
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	FVector GetNetShakeScale(int32 End) const;
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	float GetRattleDeg() const { return RattleDegNow; }
	// The component rocking right now, or the last one that did ("" before the first).
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	FString GetRattledPieceName() const;
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	int32 GetGoalReactionCount() const { return GoalReactions; }
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	int32 GetRattleCount() const { return RattleReactions; }
	// True while any reaction is live (a lamp lit, a net or a piece moving).
	UFUNCTION(BlueprintPure, Category = "Jam|Rink")
	bool IsReacting() const { return GoalFrame >= 0 || RattleFrame >= 0; }
	// Serves AJamSkater::BoxSeat (JamCore): a boxed skater's figure is drawn at its team's GetPenaltyBoxSeat by the rink of its own world. BeginPlay calls it.
	static void ServeBoxSeat();

	virtual void Tick(float DeltaSeconds) override;

	// Lane A25 (Director 17:17, eyes finding 3): the far-net beacon column read as noise and as a dithered artefact. `jam.rink.beacon` 0 = off, 1 = B's MI_Trim_Beacon
	// (translucent, as before), 2 = an opaque emissive column (lab flat instance in the trim colour). Build and the cvar sink apply it.
	void ApplyBeaconMode();
	static int32 GetBeaconMode();

	// The rounded-rectangle outline used for the ice and the boards (rink metres, counter-clockwise from +x).
	static void Outline(double LengthM, double WidthM, double CornerM, int32 ArcSegments, TArray<FVector2D>& Out);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
	// Lane A127: the reactions' state. LampMids: 0 the scorer's table's lamp, 1 the goal light behind the -x net, 2 behind the +x net (null = no such lamp or
	// a material without Emissive). The rattle's pieces: bench -x, bench +x, box -x, box +x, table, with the rotation Build gave each.
	UFUNCTION()
	void OnJamEvent(FName Event, const FJamEventPayload& Payload);
	void ResetReactions();
	void ApplyGoalReaction(int64 Since);
	void ApplyRattle(int64 Since);
	UStaticMeshComponent* RattlePiece(int32 Index) const;
	UPROPERTY(Transient) TObjectPtr<UJamEventBus> ReactBus;
	UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> LampMids[3];
	float LampRest[3] = { -1.f, -1.f, -1.f };
	FVector NetRestScale = FVector::OneVector;
	FQuat RattleRestRot[5] = { FQuat::Identity, FQuat::Identity, FQuat::Identity, FQuat::Identity, FQuat::Identity };
	double RattleCentreX[5] = { 0, 0, 0, 0, 0 };     // rink metres
	double RattleHalfLen[5] = { 0, 0, 0, 0, 0 };
	double RinkHalfWidthM = 0.0;
	int64 GoalFrame = -1;
	int32 GoalEnd = 0;          // +1 the +x net was scored on, -1 the -x net
	int64 RattleFrame = -1;
	int32 RattleIndex = -1;
	float RattleStrength = 0.f;
	float RattleDegNow = 0.f;
	int32 GoalReactions = 0;
	int32 RattleReactions = 0;

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
