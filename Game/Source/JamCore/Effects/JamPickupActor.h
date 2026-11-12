// Lane A14: the toy pickup on the ice. A glowing capsule (sphere fallback) in the kind's team-neutral colour, stencil 0 (no outline
// pass; the toy look reads it as a prop), bobbing and spinning by sim frame. Render-only: AJamPickupActor::Sync mirrors
// UJamEffectSystem::GetPickups() every render frame; nothing here writes back. Device B may replace the material by placing
// /JamAssets/Look/MI_Toon_Pickup (parameter "Color"); until then the engine's BasicShapeMaterial carries the colour.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JamEffectTypes.h"
#include "JamPickupActor.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UJamEffectSystem;

UCLASS()
class JAMCORE_API AJamPickupActor : public AActor
{
	GENERATED_BODY()

public:
	static constexpr double HoverM = 0.45;
	static constexpr double BobM = 0.08;

	AJamPickupActor();
	void InitPickup(const FJamPickupOnIce& P);
	// Frame + Alpha (0..1 of the next step) drive the bob and the spin.
	void ApplyRender(double FrameF);

	int32 GetPickupId() const { return PickupId; }

	// Spawns, moves and destroys actors so that Actors mirrors Effects' pickups on the ice. World null = no-op.
	static void Sync(UWorld* World, const UJamEffectSystem* Effects, TArray<TObjectPtr<AJamPickupActor>>& Actors, double FrameF);
	static void DestroyAll(TArray<TObjectPtr<AJamPickupActor>>& Actors);

private:
	UPROPERTY(VisibleAnywhere, Category = "Jam|Pickup") TObjectPtr<UStaticMeshComponent> Mesh;
	UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> Mid;
	int32 PickupId = -1;
	FVector2D PosM = FVector2D::ZeroVector;
};
