// Lane A14: the toy pickup on the ice. Lane A21 (B-to-A-0016): device B's look when its assets are present:
// /JamAssets/Props/SM_Pickup_Base (a 70 cm pedestal ring, MI_Pickup_Base) on the ice, /JamAssets/Props/SM_Pickup_Token at the base's
// socket PickupTop (45 cm up) with /JamAssets/Look/MI_Pickup_<kind> (MI_Pickup_custom for any other effect id), both stencil 1; spin, bob
// and scrolling are in B's materials, so the actor stands still. Optional /JamAssets/VFX/NS_pickup_idle at the base with User.Colour
// (the instance's PickupColour when it has one), Deactivate() on take or despawn. Every load by path is quiet and falls back to the
// A14 look (a glowing capsule, sphere fallback, bobbing and spinning by sim frame, stencil 0) so a missing asset never blocks play.
// Render-only: AJamPickupActor::Sync mirrors UJamEffectSystem::GetPickups() every render frame; nothing here writes back.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JamEffectTypes.h"
#include "JamPickupActor.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UNiagaraComponent;
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
	// Frame + Alpha (0..1 of the next step) drive the bob and the spin (A14 look only; B's materials animate themselves).
	void ApplyRender(double FrameF);
	// The pickup is gone (taken or despawned): the idle effect is deactivated so it ends itself, then the actor is destroyed.
	void Retire();

	int32 GetPickupId() const { return PickupId; }
	// True when B's base and token meshes carry this pickup (tests and the PIE proof read it).
	bool IsLookFromB() const { return bLookB; }

	// B-to-A-0016 asset paths (object paths; the loads are quiet, a miss falls back).
	static const TCHAR* BaseMeshPath() { return TEXT("/JamAssets/Props/SM_Pickup_Base.SM_Pickup_Base"); }
	static const TCHAR* TokenMeshPath() { return TEXT("/JamAssets/Props/SM_Pickup_Token.SM_Pickup_Token"); }
	static const TCHAR* IdleFxPath() { return TEXT("/JamAssets/VFX/NS_pickup_idle.NS_pickup_idle"); }
	static FString TokenMaterialPath(FName Kind);
	static const FName TokenSocket;   // "PickupTop"

	// Spawns, moves and destroys actors so that Actors mirrors Effects' pickups on the ice. World null = no-op.
	static void Sync(UWorld* World, const UJamEffectSystem* Effects, TArray<TObjectPtr<AJamPickupActor>>& Actors, double FrameF);
	static void DestroyAll(TArray<TObjectPtr<AJamPickupActor>>& Actors);

private:
	UPROPERTY(VisibleAnywhere, Category = "Jam|Pickup") TObjectPtr<UStaticMeshComponent> Mesh;     // A14 capsule, or B's base ring
	UPROPERTY(VisibleAnywhere, Category = "Jam|Pickup") TObjectPtr<UStaticMeshComponent> Token;    // B's token at PickupTop
	UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> Mid;
	UPROPERTY(Transient) TObjectPtr<UNiagaraComponent> Idle;
	int32 PickupId = -1;
	FVector2D PosM = FVector2D::ZeroVector;
	bool bLookB = false;
};
