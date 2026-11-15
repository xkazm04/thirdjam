// Lane A197 (track M1, evolution 8; R-0064 point 8): the grid arena, device A's half. On a neon tier (l3_neon, or device B's n1_* rig) the rink becomes a
// dark sheet drawn in light: a faint grid that brightens near the puck, the rink's lines and the board foot as lines of light in the two team accents by half
// (blue half x < 0, orange half x > 0), the goal frames outlined, a short light trail behind every skater in his team's colour (about half a second: 30 sim
// frames of samples) and a brighter trail behind the puck, which is the brightest thing on the sheet; a goal lights the scored frame and pulses the grid once.
// Colour-blind reading: the two teams differ in shape as well as hue (blue trails are a double line, orange trails one solid line) and the orange half's
// lines are a dashed light where the blue half's are solid.
// Presentation only. Nothing here is read by the simulation: it reads the pawns' and the puck's transforms and the event bus's goal events, and draws.
// Materials: only the rink's own trim placeholder (/Game/Placeholders/M_Trim: opaque, TeamColour, Emissive) in dynamic instances. No light is added.
// The rink creates the component at every Build (ApplyFor) and on a tier switch (OnTierChanged); off a neon tier it is empty and does not tick.
#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "JamCartridge.h"
#include "JamEventBus.h"
#include "NeonArena.generated.h"

class ARinkActor;
class UProceduralMeshComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UMeshComponent;
class UStaticMeshComponent;

UCLASS(ClassGroup = (Jam))
class JAMGAME_API UJamNeonArena : public USceneComponent
{
	GENERATED_BODY()

public:
	UJamNeonArena();

	// True for the tiers drawn as the grid arena: "l3_neon" and any device B neon rig tier ("n1_" prefix).
	static bool IsNeonTier(const FString& Tier);
	// Rink build: finds or makes the rink's arena component, keeps the config and draws (neon tier) or clears (any other tier). One call in ARinkActor::Build.
	static UJamNeonArena* ApplyFor(ARinkActor* Rink, const FJamGameConfig& Game);
	// Tier switch without a rebuild (ULookTierSwitcher::SetTier through ARinkActor::SetZoneRingsForTier).
	static void OnTierChanged(ARinkActor* Rink, const FString& Tier);

	// Lane A230: the mode's accent on the grid arena: the cut's look.trim_colour_hex when it is not the schema's default (#38BDF8), else the white of before
	// (`jam.neon.accent` 0: white always). Drawn on the centre line, the centre circle and dot, and the rims of light on the obstacles.
	static FLinearColor AccentFor(const FJamLookConfig& Look);

	// Trail length in sim frames (0.5 s at 60 Hz) and the puck's.
	static constexpr int32 TrailFrames = 30;
	static constexpr int32 PuckTrailFrames = 20;
	// Grid pitch (m), the radius (m) around the puck inside which the grid brightens, and the grid's resting and brightest Emissive.
	static constexpr double GridPitchM = 2.5;
	static constexpr double GridGlowRadiusM = 8.0;
	static constexpr float GridRestEmissive = 0.4f;
	static constexpr float GridNearEmissive = 6.f;
	// The goal pulse: frames, the grid's added Emissive at its top, the scored frame's Emissive at its top.
	static constexpr int32 GoalPulseFrames = 45;
	static constexpr float GoalPulseGrid = 8.f;
	static constexpr float GoalFramePeak = 60.f;
	// Pure for the tests: the grid tile's Emissive at a distance from the puck (m) and a pulse value at a frame since the goal (1 -> 0).
	static float GridEmissiveAt(double DistanceM);
	static float GoalPulseAt(int64 FramesSince);

	// Read-outs for the tests and the proof.
	bool IsArenaDrawn() const { return bActive; }
	int32 GetGridTileCount() const { return GridTiles.Num(); }
	int32 GetTrailCount() const { return Trails.Num(); }
	int32 GetTrailVertexCount() const { return TrailVerts; }
	float GetGridTileEmissive(int32 Index) const { return GridTiles.IsValidIndex(Index) ? GridTiles[Index].Emissive : -1.f; }
	int32 GetGoalPulses() const { return GoalPulses; }
	// Figure material slots lit since the last Draw (B's trims ask); a read-out for the proof.
	int32 GetFiguresLit() const { return FiguresLit; }
	// Lane A223: material slots of the penalty boxes that took the dark sheet's material at the last Draw (`jam.look.NeonBoxes`); 0 off the grid arena.
	int32 GetBoxSlotsDarkened() const { return BoxSlotsDarkened; }
	// Lane A230: pillars and boxes of the rink that took the dark sheet's material and their rims of light at the last Draw (`jam.neon.obstacles`).
	int32 GetObstaclesDarkened() const { return ObstaclesDarkened; }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
	void Draw(ARinkActor* Rink);
	void Clear(ARinkActor* Rink);
	void BuildGrid(const FJamRinkConfig& R);
	void BuildLines(ARinkActor* Rink, const FJamRinkConfig& R);
	void SampleAndDrawTrails(int64 SimFrame);
	void UpdateGrid(int64 SimFrame);
	UMaterialInstanceDynamic* MakeLight(const FLinearColor& Colour, float Emissive);
	UProceduralMeshComponent* MakeMesh(const TCHAR* Name);
	void BindBus(UJamEventBus* InBus);
	void ApplyFigure(AActor* Skater, int32 Team);
	void RestoreFigures();
	void DarkenBoxes(ARinkActor* Rink, bool bSheet);   // lane A223
	void DarkenObstacles(ARinkActor* Rink, bool bSheet);   // lane A230
	void RestoreBoxes();
	UFUNCTION() void OnJamEvent(FName Event, const FJamEventPayload& Payload);

	struct FTile { FVector2D Centre = FVector2D::ZeroVector; float Emissive = -1.f; };
	struct FFigureSave { TWeakObjectPtr<UMeshComponent> Mesh; int32 Slot = -1; TWeakObjectPtr<UMaterialInterface> Original; TWeakObjectPtr<UMaterialInstanceDynamic> Mid; TMap<FName, FLinearColor> Vectors; TMap<FName, float> Scalars; };
	TArray<FFigureSave> SavedFigures;
	struct FBoxSave { TWeakObjectPtr<UStaticMeshComponent> Mesh; int32 Slot = -1; TWeakObjectPtr<UMaterialInterface> Original; };
	TArray<FBoxSave> SavedBoxes;      // lane A223: the boxes' daylight materials before Draw darkened them
	int32 BoxSlotsDarkened = 0;
	int32 ObstaclesDarkened = 0;      // lane A230
	struct FTrail { TWeakObjectPtr<AActor> Who; int32 Team = 0; bool bPuck = false; TArray<FVector> Points; };

	FJamGameConfig Config;
	bool bHasConfig = false;
	bool bActive = false;
	FLinearColor TeamColour[2] = { FLinearColor(0.02f, 0.25f, 1.f), FLinearColor(1.f, 0.35f, 0.02f) };

	UPROPERTY(Transient) TObjectPtr<UProceduralMeshComponent> GridMesh;
	UPROPERTY(Transient) TObjectPtr<UProceduralMeshComponent> LineMesh;
	UPROPERTY(Transient) TObjectPtr<UProceduralMeshComponent> TrailMesh;
	// Line instances: 0 blue half, 1 orange half, 2 centre (white), 3 the -x net's frame, 4 the +x net's frame. Trail instances: 0 blue, 1 orange, 2 puck.
	UPROPERTY(Transient) TArray<TObjectPtr<UMaterialInstanceDynamic>> LineMids;
	UPROPERTY(Transient) TArray<TObjectPtr<UMaterialInstanceDynamic>> TrailMids;
	UPROPERTY(Transient) TArray<TObjectPtr<UMaterialInstanceDynamic>> TileMids;
	UPROPERTY(Transient) TObjectPtr<UMaterialInterface> SavedBoards;
	UPROPERTY(Transient) TObjectPtr<UMaterialInterface> SavedObstacles;   // lane A230: the obstacles' daylight material before Draw darkened it
	UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> DarkBoards;
	UPROPERTY(Transient) TObjectPtr<UJamEventBus> Bus;
	TArray<FTile> GridTiles;
	TArray<FTrail> Trails;
	TMap<FName, float> SavedIceScalars;          // the ice's band widths before Draw zeroed them
	TMap<FName, FLinearColor> SavedIceVectors;   // the ice instance's tints before Draw darkened them
	bool bIceSaved = false;
	int64 LastSampleFrame = -1;
	int64 GoalFrame = -1;
	int32 GoalEnd = 0;           // -1: the -x net, +1: the +x net
	int32 GoalPulses = 0;
	int32 TrailVerts = 0;
	int32 FiguresLit = 0;
	int32 LoggedFigures = -1;
	int32 FigureMeshes = 0;
};
