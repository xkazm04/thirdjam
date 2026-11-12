#include "JamPickupActor.h"
#include "JamEffectSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

AJamPickupActor::AJamPickupActor()
{
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCastShadow(false);
	Mesh->SetRenderCustomDepth(false);
	Mesh->SetCustomDepthStencilValue(0);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Capsule(TEXT("/Engine/BasicShapes/Capsule.Capsule"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (Capsule.Succeeded()) Mesh->SetStaticMesh(Capsule.Object);
	else if (Sphere.Succeeded()) Mesh->SetStaticMesh(Sphere.Object);
}

void AJamPickupActor::InitPickup(const FJamPickupOnIce& P)
{
	PickupId = P.Id;
	PosM = P.Pos;
	// Basic shapes are 100 cm across: a 0.5 m capsule standing up (sphere 0.6 m).
	Mesh->SetRelativeScale3D(FVector(0.5, 0.5, 0.7));
	const FLinearColor Colour = JamEffects::KindColour(FJamEffectsConfig::IsBuiltin(P.Kind) ? P.Kind : FName(TEXT("custom")));
	UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/JamAssets/Look/MI_Toon_Pickup.MI_Toon_Pickup"), nullptr, LOAD_NoWarn | LOAD_Quiet);
	if (!Base) Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (Base)
	{
		Mid = UMaterialInstanceDynamic::Create(Base, this);
		Mid->SetVectorParameterValue(TEXT("Color"), Colour * 1.6f);
		Mesh->SetMaterial(0, Mid);
	}
#if WITH_EDITOR
	SetActorLabel(FString::Printf(TEXT("Pickup_%d_%s"), P.Id, *P.Kind.ToString()));
#endif
	ApplyRender(static_cast<double>(P.SpawnFrame));
}

void AJamPickupActor::ApplyRender(double FrameF)
{
	const double Bob = HoverM + BobM * FMath::Sin(FrameF * 0.09);
	SetActorLocationAndRotation(UJamSim::ToWorld(PosM, Bob), FRotator(0, FMath::Fmod(FrameF * 2.0, 360.0), 0));
}

void AJamPickupActor::Sync(UWorld* World, const UJamEffectSystem* Effects, TArray<TObjectPtr<AJamPickupActor>>& Actors, double FrameF)
{
	if (!World || !Effects) return;
	const TArray<FJamPickupOnIce>& OnIce = Effects->GetPickups();
	for (int32 i = Actors.Num() - 1; i >= 0; --i)
	{
		AJamPickupActor* A = Actors[i];
		const bool bAlive = A && OnIce.ContainsByPredicate([A](const FJamPickupOnIce& P) { return P.Id == A->GetPickupId(); });
		if (!bAlive) { if (A) A->Destroy(); Actors.RemoveAt(i); }
	}
	for (const FJamPickupOnIce& P : OnIce)
	{
		if (Actors.ContainsByPredicate([&P](const AJamPickupActor* A) { return A && A->GetPickupId() == P.Id; })) continue;
		FActorSpawnParameters Sp;
		Sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (AJamPickupActor* A = World->SpawnActor<AJamPickupActor>(AJamPickupActor::StaticClass(), FTransform(UJamSim::ToWorld(P.Pos, HoverM)), Sp))
		{
			A->InitPickup(P);
			Actors.Add(A);
		}
	}
	for (AJamPickupActor* A : Actors) if (A) A->ApplyRender(FrameF);
}

void AJamPickupActor::DestroyAll(TArray<TObjectPtr<AJamPickupActor>>& Actors)
{
	for (AJamPickupActor* A : Actors) if (A) A->Destroy();
	Actors.Reset();
}
