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

FString AJamPickupActor::PropMeshPath(FName Kind)
{
	const FString K = FJamEffectsConfig::IsBuiltin(Kind) ? Kind.ToString() : FString(TEXT("custom"));
	return FString::Printf(TEXT("/JamAssets/Props/SM_Pickup_%s.SM_Pickup_%s"), *K, *K);
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
	// Lane A31 (B-to-A-0019): the prop component, same rules as the token (stencil 1, no collision, no shadow); the mesh carries its own MI_PickupProp_<kind>.
	Prop = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Prop"));
	Prop->SetupAttachment(Mesh);
	Prop->SetMobility(EComponentMobility::Movable);
	Prop->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Prop->SetGenerateOverlapEvents(false);
	Prop->SetCastShadow(false);
	Prop->SetRenderCustomDepth(true);
	Prop->SetCustomDepthStencilValue(1);
	Prop->SetVisibility(false);
	// Lane A47 (B-to-A-0030 fault 10): a cylinder puck-shaped token, not the capsule (the capsule read as a pill standing on the ice).
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (Cylinder.Succeeded()) Mesh->SetStaticMesh(Cylinder.Object);
	else if (Sphere.Succeeded()) Mesh->SetStaticMesh(Sphere.Object);
}

void AJamPickupActor::InitPickup(const FJamPickupOnIce& P)
{
	PickupId = P.Id;
	PosM = P.Pos;
	const FName LookKind = FJamEffectsConfig::IsBuiltin(P.Kind) ? P.Kind : FName(TEXT("custom"));
	FLinearColor Colour = JamEffects::KindColour(LookKind);

	// B-to-A-0016: B's base ring and token when both meshes and the kind's instance are present; otherwise the A14 capsule.
	// Lane A31 (B-to-A-0019): B's SM_Pickup_<kind> prop rides the base instead of the token wherever it exists (the Director's call: props for every kind that
	// has one, the token as the fallback). The base alone with a prop is enough for B's look; the token needs its instance too.
	UStaticMesh* BaseMesh = LoadQuietOnce<UStaticMesh>(BaseMeshPath());
	UStaticMesh* PropMesh = BaseMesh ? LoadQuietOnce<UStaticMesh>(*PropMeshPath(P.Kind)) : nullptr;
	UStaticMesh* TokenMesh = BaseMesh && !PropMesh ? LoadQuietOnce<UStaticMesh>(TokenMeshPath()) : nullptr;
	UMaterialInterface* TokenMat = BaseMesh ? LoadQuietOnce<UMaterialInterface>(*TokenMaterialPath(P.Kind)) : nullptr;   // also the colour of record for the idle effect
	bProp = BaseMesh && PropMesh;
	bLookB = bProp || (BaseMesh && TokenMesh && TokenMat);
	if (bLookB)
	{
		Mesh->SetStaticMesh(BaseMesh);   // the mesh carries MI_Pickup_Base itself
		Mesh->SetRelativeScale3D(FVector::OneVector);
		Mesh->SetRenderCustomDepth(true);
		Mesh->SetCustomDepthStencilValue(1);
		const FName Socket = Mesh->DoesSocketExist(TokenSocket) ? TokenSocket : NAME_None;
		if (bProp)
		{
			Prop->SetStaticMesh(PropMesh);   // MI_PickupProp_<kind> is on the mesh
			Prop->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
			Prop->SetRelativeLocation(Socket == NAME_None ? FVector(0, 0, HoverM * 100.0) : FVector::ZeroVector);
			Prop->SetVisibility(true);
			Token->SetVisibility(false);
		}
		else
		{
			Token->SetStaticMesh(TokenMesh);
			Token->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
			if (Socket == NAME_None) Token->SetRelativeLocation(FVector(0, 0, HoverM * 100.0));   // B's socket is 45 cm up; the same height without it
			Mid = UMaterialInstanceDynamic::Create(TokenMat, this);
			Token->SetMaterial(0, Mid);
			Token->SetVisibility(true);
		}
		// The instance's PickupColour is the colour of record (B-to-A-0016); A's table stands in when the parameter is absent.
		FLinearColor FromAsset;
		if (TokenMat && TokenMat->GetVectorParameterValue(FMaterialParameterInfo(TEXT("PickupColour")), FromAsset)) Colour = FromAsset;
		if (UNiagaraSystem* IdleFx = LoadObject<UNiagaraSystem>(nullptr, IdleFxPath(), nullptr, LOAD_NoWarn | LOAD_Quiet))   // optional: no log when absent
		{
			// Spawned in the world, not attached: it must outlive the actor so Deactivate() can let it end itself on a take.
			Idle = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), IdleFx, UJamSim::ToWorld(PosM, 0.0), FRotator::ZeroRotator, FVector::OneVector, true, true);
			if (Idle) Idle->SetVariableLinearColor(TEXT("User.Colour"), Colour);
		}
	}
	else
	{
		// Basic shapes are 100 cm across and tall: a 0.5 m cylinder token, 0.2 m thick (sphere 0.6 m).
		Mesh->SetRelativeScale3D(FVector(0.5, 0.5, 0.2));
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
		// The pedestal sits on the ice; the token's spin, bob and scrolling are in B's materials.
		SetActorLocationAndRotation(UJamSim::ToWorld(PosM, 0.0), FRotator::ZeroRotator);
		if (bProp && Prop)
		{
			// Lane A31 (B-to-A-0019): the prop's M_PropCel instances carry no motion, so the spin (90 deg/s) and the 6 cm bob (0..6 cm above the socket, 2 s period)
			// are driven here from the sim frame + alpha: deterministic, never wall clock.
			const double Yaw = FMath::Fmod(FrameF * PropSpinDegPerFrame, 360.0);
			const double Lift = PropBobCm * 0.5 * (1.0 + FMath::Sin(FrameF * (2.0 * UE_DOUBLE_PI / PropBobPeriodFrames)));
			const bool bOnSocket = Mesh && Mesh->DoesSocketExist(TokenSocket);
			Prop->SetRelativeLocationAndRotation(FVector(0, 0, (bOnSocket ? 0.0 : HoverM * 100.0) + Lift), FRotator(0, Yaw, 0));
		}
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
