#include "JamPickupActor.h"
#include "JamEffectSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

const FName AJamPickupActor::TokenSocket(TEXT("PickupTop"));

namespace
{
	// Quiet load by path: a miss is answered by the A14 look, and logged once per path so a missing asset never floods the log.
	template <typename T>
	T* LoadQuietOnce(const TCHAR* Path)
	{
		T* Obj = LoadObject<T>(nullptr, Path, nullptr, LOAD_NoWarn | LOAD_Quiet);
		if (!Obj)
		{
			static TSet<FString> Logged;
			if (!Logged.Contains(Path))
			{
				Logged.Add(Path);
				UE_LOG(LogTemp, Log, TEXT("JAMFX missing %s (logged once, pickup falls back to the toy look)"), Path);
			}
		}
		return Obj;
	}
}

FString AJamPickupActor::TokenMaterialPath(FName Kind)
{
	const FString K = FJamEffectsConfig::IsBuiltin(Kind) ? Kind.ToString() : FString(TEXT("custom"));
	return FString::Printf(TEXT("/JamAssets/Look/MI_Pickup_%s.MI_Pickup_%s"), *K, *K);
}

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
	Token = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Token"));
	Token->SetupAttachment(Mesh);
	Token->SetMobility(EComponentMobility::Movable);
	Token->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Token->SetCastShadow(false);
	Token->SetRenderCustomDepth(true);
	Token->SetCustomDepthStencilValue(1);
	Token->SetVisibility(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Capsule(TEXT("/Engine/BasicShapes/Capsule.Capsule"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (Capsule.Succeeded()) Mesh->SetStaticMesh(Capsule.Object);
	else if (Sphere.Succeeded()) Mesh->SetStaticMesh(Sphere.Object);
}

void AJamPickupActor::InitPickup(const FJamPickupOnIce& P)
{
	PickupId = P.Id;
	PosM = P.Pos;
	const FName LookKind = FJamEffectsConfig::IsBuiltin(P.Kind) ? P.Kind : FName(TEXT("custom"));
	FLinearColor Colour = JamEffects::KindColour(LookKind);

	// B-to-A-0016: B's base ring and token when both meshes and the kind's instance are present; otherwise the A14 capsule.
	UStaticMesh* BaseMesh = LoadQuietOnce<UStaticMesh>(BaseMeshPath());
	UStaticMesh* TokenMesh = BaseMesh ? LoadQuietOnce<UStaticMesh>(TokenMeshPath()) : nullptr;
	UMaterialInterface* TokenMat = TokenMesh ? LoadQuietOnce<UMaterialInterface>(*TokenMaterialPath(P.Kind)) : nullptr;
	bLookB = BaseMesh && TokenMesh && TokenMat;
	if (bLookB)
	{
		Mesh->SetStaticMesh(BaseMesh);   // the mesh carries MI_Pickup_Base itself
		Mesh->SetRelativeScale3D(FVector::OneVector);
		Mesh->SetRenderCustomDepth(true);
		Mesh->SetCustomDepthStencilValue(1);
		Token->SetStaticMesh(TokenMesh);
		Token->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Mesh->DoesSocketExist(TokenSocket) ? TokenSocket : NAME_None);
		if (!Mesh->DoesSocketExist(TokenSocket)) Token->SetRelativeLocation(FVector(0, 0, HoverM * 100.0));   // B's socket is 45 cm up; the same height without it
		Mid = UMaterialInstanceDynamic::Create(TokenMat, this);
		Token->SetMaterial(0, Mid);
		Token->SetVisibility(true);
		// The instance's PickupColour is the colour of record (B-to-A-0016); A's table stands in when the parameter is absent.
		FLinearColor FromAsset;
		if (TokenMat->GetVectorParameterValue(FMaterialParameterInfo(TEXT("PickupColour")), FromAsset)) Colour = FromAsset;
		if (UNiagaraSystem* IdleFx = LoadObject<UNiagaraSystem>(nullptr, IdleFxPath(), nullptr, LOAD_NoWarn | LOAD_Quiet))   // optional: no log when absent
		{
			// Spawned in the world, not attached: it must outlive the actor so Deactivate() can let it end itself on a take.
			Idle = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), IdleFx, UJamSim::ToWorld(PosM, 0.0), FRotator::ZeroRotator, FVector::OneVector, true, true);
			if (Idle) Idle->SetVariableLinearColor(TEXT("User.Colour"), Colour);
		}
	}
	else
	{
		// Basic shapes are 100 cm across: a 0.5 m capsule standing up (sphere 0.6 m).
		Mesh->SetRelativeScale3D(FVector(0.5, 0.5, 0.7));
		UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/JamAssets/Look/MI_Toon_Pickup.MI_Toon_Pickup"), nullptr, LOAD_NoWarn | LOAD_Quiet);
		if (!Base) Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		if (Base)
		{
			Mid = UMaterialInstanceDynamic::Create(Base, this);
			Mid->SetVectorParameterValue(TEXT("Color"), Colour * 1.6f);
			Mesh->SetMaterial(0, Mid);
		}
	}
#if WITH_EDITOR
	SetActorLabel(FString::Printf(TEXT("Pickup_%d_%s"), P.Id, *P.Kind.ToString()));
#endif
	ApplyRender(static_cast<double>(P.SpawnFrame));
}

void AJamPickupActor::ApplyRender(double FrameF)
{
	if (bLookB)
	{
		// The pedestal sits on the ice; spin, bob and scrolling are in B's materials.
		SetActorLocationAndRotation(UJamSim::ToWorld(PosM, 0.0), FRotator::ZeroRotator);
		return;
	}
	const double Bob = HoverM + BobM * FMath::Sin(FrameF * 0.09);
	SetActorLocationAndRotation(UJamSim::ToWorld(PosM, Bob), FRotator(0, FMath::Fmod(FrameF * 2.0, 360.0), 0));
}

void AJamPickupActor::Retire()
{
	if (Idle)
	{
		Idle->Deactivate();   // auto-destroys once the system has ended itself
		Idle = nullptr;
	}
	Destroy();
}

void AJamPickupActor::Sync(UWorld* World, const UJamEffectSystem* Effects, TArray<TObjectPtr<AJamPickupActor>>& Actors, double FrameF)
{
	if (!World || !Effects) return;
	const TArray<FJamPickupOnIce>& OnIce = Effects->GetPickups();
	for (int32 i = Actors.Num() - 1; i >= 0; --i)
	{
		AJamPickupActor* A = Actors[i];
		const bool bAlive = A && OnIce.ContainsByPredicate([A](const FJamPickupOnIce& P) { return P.Id == A->GetPickupId(); });
		if (!bAlive) { if (A) A->Retire(); Actors.RemoveAt(i); }
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
	for (AJamPickupActor* A : Actors) if (A) A->Retire();
	Actors.Reset();
}
