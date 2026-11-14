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
#include "HAL/IConsoleManager.h"
#include "JamPawns.h"
#include "JamSim.h"
#include "Misc/AutomationTest.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

// Lane A188 (track F3, evolution 8): the boosts seen (JamPickupActor.h).
static TAutoConsoleVariable<int32> CVarJamBoostBeacon(TEXT("jam.boost.Beacon"), 1,
	TEXT("A pickup on the ice (lane A188): 1 (default) the beacon: a turning shape by kind on a pedestal of light with a ground ring in its colour, low while a skater or the puck is near; 0 the look of before (B's base ring and prop). Read at a pickup's spawn. Presentation only."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamBoostAura(TEXT("jam.boost.Aura"), 1,
	TEXT("The equipped look (lane A188): 1 (default) a waist ring of light in the ability's colour unless the figure took device B's SetBoost(Kind, bOn); 2 the ring always; 0 none (before). Presentation only."), ECVF_Default);
static TAutoConsoleVariable<float> CVarJamBoostScale(TEXT("jam.boost.Scale"), 1.f,
	TEXT("The beacon's size factor (lane A188, tuning in a window): 1 (default) = a 0.8 m shape 1.25 m over the ice on a 1 m pedestal. Read at a pickup's spawn."), ECVF_Default);

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
	// Lane A188: the beacon's parts (hidden until InitBeacon): a pedestal of light, the turning shape (stencil 1: the toon outline), the ground ring.
	Pedestal = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pedestal"));
	Shape = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shape"));
	for (UStaticMeshComponent* C : { Pedestal.Get(), Shape.Get() })
	{
		C->SetupAttachment(Mesh);
		C->SetMobility(EComponentMobility::Movable);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->SetGenerateOverlapEvents(false);
		C->SetCastShadow(false);
		C->SetVisibility(false);
	}
	Shape->SetRenderCustomDepth(true);
	Shape->SetCustomDepthStencilValue(1);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (Cylinder.Succeeded()) Mesh->SetStaticMesh(Cylinder.Object);
	else if (Sphere.Succeeded()) Mesh->SetStaticMesh(Sphere.Object);
}

void AJamPickupActor::InitPickup(const FJamPickupOnIce& P)
{
	PickupId = P.Id;
	PosM = P.Pos;
	Kind = P.Kind;
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
	// Lane A188: the beacon over whichever look was chosen above (B's base and prop or token are hidden under it; the idle effect stays).
	if (BeaconOn()) InitBeacon(LookKind, JamEffects::KindColour(LookKind));
#if WITH_EDITOR
	SetActorLabel(FString::Printf(TEXT("Pickup_%d_%s"), P.Id, *P.Kind.ToString()));
#endif
	ApplyRender(static_cast<double>(P.SpawnFrame));
}

void AJamPickupActor::ApplyRender(double FrameF)
{
	if (bBeacon) { ApplyBeacon(FrameF); return; }
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

void AJamPickupActor::Sync(UWorld* World, const UJamEffectSystem* Effects, TArray<TObjectPtr<AJamPickupActor>>& Actors, double FrameF, const TArray<FVector2D>* KeepClear)
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
	for (AJamPickupActor* A : Actors)
	{
		if (!A) continue;
		if (A->bBeacon)
		{
			// Lane A188: the nearest skater or the puck lowers the beacon (it never hides either).
			double Nearest = 1e9;
			if (KeepClear) for (const FVector2D& K : *KeepClear) Nearest = FMath::Min(Nearest, FVector2D::Distance(K, A->PosM));
			A->HeightWant = BeaconHeightFor(Nearest);
		}
		A->ApplyRender(FrameF);
	}
}

void AJamPickupActor::DestroyAll(TArray<TObjectPtr<AJamPickupActor>>& Actors)
{
	for (AJamPickupActor* A : Actors) if (A) A->Retire();
	Actors.Reset();
}

// ---------------------------------------------------------------- lane A188: the beacon

namespace
{
	const TCHAR* const TrimPath = TEXT("/Game/Placeholders/M_Trim.M_Trim");   // A's own trim material (RinkActor's TrimMaterialPath): TeamColour, Emissive

	UMaterialInstanceDynamic* MakeLight(UObject* Outer, const FLinearColor& Colour, float Emissive)
	{
		UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TrimPath, nullptr, LOAD_NoWarn | LOAD_Quiet);
		if (!Base) Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		if (!Base) return nullptr;
		UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(Base, Outer);
		M->SetVectorParameterValue(TEXT("TeamColour"), Colour);
		M->SetVectorParameterValue(TEXT("Color"), Colour);   // the basic shape material's name, when M_Trim is absent
		M->SetScalarParameterValue(TEXT("Emissive"), Emissive);
		return M;
	}

	// A ring of Segments boxes round Parent's origin, radius R (cm), each SegLen x Wide x Tall cm, at height Z (cm): plain components made on first use
	// (Parts keeps them), every one with Mat.
	void BuildRing(AActor* Owner, USceneComponent* Parent, TArray<TObjectPtr<UStaticMeshComponent>>& Parts, UMaterialInterface* Mat, int32 Segments, double R, double Fill,
		double Wide, double Tall, double Z)
	{
		UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		while (Parts.Num() < Segments)
		{
			UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(Owner, NAME_None, RF_Transient);
			C->SetMobility(EComponentMobility::Movable);
			C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			C->SetGenerateOverlapEvents(false);
			C->SetCastShadow(false);
			if (Cube) C->SetStaticMesh(Cube);
			C->SetupAttachment(Parent);
			C->RegisterComponent();
			Parts.Add(C);
		}
		const double SegLen = 2.0 * UE_DOUBLE_PI * R / Segments * Fill;
		for (int32 i = 0; i < Parts.Num(); ++i)
		{
			UStaticMeshComponent* C = Parts[i];
			if (!C) continue;
			if (i >= Segments) { C->SetVisibility(false); continue; }
			const double A = 2.0 * UE_DOUBLE_PI * i / Segments;
			// The basic cube is 100 cm about its centre: scaled to the segment, turned tangent to the circle.
			C->SetRelativeLocationAndRotation(FVector(R * FMath::Cos(A), R * FMath::Sin(A), Z + Tall * 0.5), FRotator(0.0, FMath::RadiansToDegrees(A) + 90.0, 0.0));
			C->SetRelativeScale3D(FVector(SegLen / 100.0, Wide / 100.0, Tall / 100.0));
			if (Mat) C->SetMaterial(0, Mat);
		}
	}

	void ShowRing(TArray<TObjectPtr<UStaticMeshComponent>>& Parts, bool bShow)
	{
		for (UStaticMeshComponent* C : Parts) if (C) C->SetVisibility(bShow);
	}

	double BeaconSizeK() { return FMath::Clamp(static_cast<double>(CVarJamBoostScale.GetValueOnGameThread()), 0.25, 4.0); }
}

bool AJamPickupActor::BeaconOn() { return CVarJamBoostBeacon.GetValueOnGameThread() != 0; }

FName AJamPickupActor::StandInShape(FName InKind)
{
	static const FName Speed(TEXT("speed")), Precision(TEXT("precision")), Heavy(TEXT("heavy_puck")), Big(TEXT("big_puck")), ReverseOther(TEXT("reverse_other"));
	if (InKind == Speed) return FName(TEXT("cone"));
	if (InKind == Precision) return FName(TEXT("diamond"));
	if (InKind == Heavy) return FName(TEXT("puck"));
	if (InKind == Big) return FName(TEXT("ball"));
	if (InKind == ReverseOther) return FName(TEXT("cube"));
	return FName(TEXT("star"));
}

FString AJamPickupActor::StandInMeshPath(FName InShape)
{
	const FString S = InShape.ToString();
	const TCHAR* M = S == TEXT("cone") ? TEXT("Cone") : (S == TEXT("puck") ? TEXT("Cylinder") : (S == TEXT("ball") || S == TEXT("star") ? TEXT("Sphere") : TEXT("Cube")));
	return FString::Printf(TEXT("/Engine/BasicShapes/%s.%s"), M, M);
}

FString AJamPickupActor::BoostPropPath(FName InKind)
{
	const FString K = FJamEffectsConfig::IsBuiltin(InKind) ? InKind.ToString() : FString(TEXT("custom"));
	return FString::Printf(TEXT("/JamAssets/Props/SM_Boost_%s.SM_Boost_%s"), *K, *K);
}

double AJamPickupActor::BeaconHeightFor(double NearestM)
{
	// 0 = the low profile (inside BeaconClearM), 1 = full (a metre further out), linear between.
	return FMath::Clamp((NearestM - BeaconClearM) / 1.0, 0.0, 1.0);
}

FVector AJamPickupActor::GetTopWorld() const
{
	const double H = BeaconLowScale + (1.0 - BeaconLowScale) * HeightNow;
	const double K = BeaconSizeK();
	return UJamSim::ToWorld(PosM, (BeaconObjectM * H + BeaconObjectSizeM * 0.5) * K);
}

void AJamPickupActor::InitBeacon(FName LookKind, const FLinearColor& Colour)
{
	bBeacon = true;
	const double K = BeaconSizeK();
	// The root (B's base ring, or the A14 token) stays the actor's root, hidden; the beacon's parts are its children at absolute scale.
	Mesh->SetVisibility(false, false);
	Mesh->SetRelativeScale3D(FVector::OneVector);
	if (Token) Token->SetVisibility(false);
	if (Prop) Prop->SetVisibility(false);
	for (USceneComponent* C : { static_cast<USceneComponent*>(Pedestal.Get()), static_cast<USceneComponent*>(Shape.Get()) })
	{
		C->SetUsingAbsoluteScale(true);
	}
	UStaticMesh* Cyl = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	// The pedestal of light: a thin bright column; the ring: 16 flat segments on the ice; both in the kind's colour.
	BeaconMid = MakeLight(this, Colour, 5.f);
	if (Cyl) Pedestal->SetStaticMesh(Cyl);
	if (BeaconMid) Pedestal->SetMaterial(0, BeaconMid);
	Pedestal->SetVisibility(true);
	// The ring hangs off the hidden root at the ice (the root is never scaled in the beacon).
	BuildRing(this, Mesh, GroundRing, BeaconMid, BeaconRingSegments, BeaconRingRadiusM * 100.0 * K, 0.62, 12.0 * K, 1.5, 0.5);
	ShowRing(GroundRing, true);
	// The shape: device B's boost prop by name when it exists, else A's stand-in by kind.
	UStaticMesh* FromB = LoadObject<UStaticMesh>(nullptr, *BoostPropPath(LookKind), nullptr, LOAD_NoWarn | LOAD_Quiet);
	bBoostPropB = FromB != nullptr;
	const FName ShapeName = StandInShape(LookKind);
	UStaticMesh* ShapeMesh = FromB ? FromB : LoadObject<UStaticMesh>(nullptr, *StandInMeshPath(ShapeName));
	if (ShapeMesh)
	{
		Shape->SetStaticMesh(ShapeMesh);
		const FVector Ext = ShapeMesh->GetBounds().BoxExtent * 2.0;
		const double Largest = FMath::Max3(Ext.X, Ext.Y, Ext.Z);
		const double Fit = Largest > 1.0 ? BeaconObjectSizeM * 100.0 * K / Largest : K;
		ShapeScale = FVector(Fit);
		ShapeTilt = FRotator::ZeroRotator;
		if (!FromB)
		{
			if (ShapeName == FName(TEXT("cone"))) ShapeScale = FVector(Fit * 0.75, Fit * 0.75, Fit);   // a spike up: speed
			else if (ShapeName == FName(TEXT("diamond"))) { ShapeScale = FVector(Fit * 0.62); ShapeTilt = FRotator(45.0, 0.0, 35.26); }   // a cube on its corner
			else if (ShapeName == FName(TEXT("puck"))) ShapeScale = FVector(Fit, Fit, Fit * 0.35);
			ShapeMid = MakeLight(this, Colour, 1.6f);
			if (ShapeMid) Shape->SetMaterial(0, ShapeMid);
		}
	}
	Shape->SetVisibility(true);
	UE_LOG(LogJamSim, Log, TEXT("JAMBOOST beacon pickup=%d kind=%s shape=%s prop_from_b=%d size_m=%.2f top_m=%.2f"), PickupId, *Kind.ToString(),
		FromB ? *BoostPropPath(LookKind) : *ShapeName.ToString(), bBoostPropB ? 1 : 0, BeaconObjectSizeM * K, (BeaconObjectM + BeaconObjectSizeM * 0.5) * K);
}

void AJamPickupActor::ApplyBeacon(double FrameF)
{
	// The low profile's blend moves a tenth per sim frame (10 frames from full to low and back); sim frame + alpha, never wall clock.
	const double Step = LastFrameF < 0.0 ? 1.0 : FMath::Clamp(FrameF - LastFrameF, 0.0, 10.0) * 0.1;
	LastFrameF = FrameF;
	HeightNow = HeightNow + FMath::Clamp(HeightWant - HeightNow, -Step, Step);
	const double H = BeaconLowScale + (1.0 - BeaconLowScale) * HeightNow;
	const double K = BeaconSizeK();
	SetActorLocationAndRotation(UJamSim::ToWorld(PosM, 0.0), FRotator::ZeroRotator);
	// The basic cylinder is 100 cm tall about its centre.
	const double PedH = BeaconPedestalM * 100.0 * K * H;
	Pedestal->SetWorldScale3D(FVector(BeaconPedestalWideM * K, BeaconPedestalWideM * K, PedH / 100.0));
	Pedestal->SetRelativeLocation(FVector(0, 0, PedH * 0.5));
	const double Yaw = FMath::Fmod(FrameF * BeaconSpinDegPerFrame, 360.0);
	const double Bob = 8.0 * K * FMath::Sin(FrameF * (2.0 * UE_DOUBLE_PI / 120.0));
	Shape->SetWorldScale3D(ShapeScale * (0.6 + 0.4 * HeightNow));
	Shape->SetRelativeLocation(FVector(0, 0, BeaconObjectM * 100.0 * K * H + Bob * HeightNow));
	Shape->SetRelativeRotation(FRotator(ShapeTilt.Pitch, Yaw, ShapeTilt.Roll));
}

// ---------------------------------------------------------------- lane A188: the equipped look

AJamBoostAura::AJamBoostAura()
{
	PrimaryActorTick.bCanEverTick = false;
	Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
	RootComponent = Pivot;
	Pivot->SetMobility(EComponentMobility::Movable);
}

FLinearColor AJamBoostAura::AbilityColour(FName InKind)
{
	const FString K = InKind.ToString();
	if (K == TEXT("shotgun")) return FLinearColor::FromSRGBColor(FColor(0xFF, 0x17, 0x44));   // crimson: the red read orange on clay
	if (K == TEXT("hip_check")) return FLinearColor::FromSRGBColor(FColor(0xE0, 0x40, 0xFB));
	if (K == TEXT("shield")) return FLinearColor::FromSRGBColor(FColor(0xE8, 0xFB, 0xFF));
	if (K == TEXT("slow_puck")) return FLinearColor::FromSRGBColor(FColor(0x9B, 0xF0, 0x1E));
	return FLinearColor::White;
}

static TAutoConsoleVariable<FString> CVarJamBoostPreview(TEXT("jam.boost.Preview"), TEXT(""),
	TEXT("Proof (lane A188): the human is drawn as holding this ability (shotgun, hip_check, shield, slow_puck; a trailing ! = armed or running) while the sim gives him none: the ring and the HUD's plate. Presentation only; empty = off."), ECVF_Default);

FName AJamBoostAura::PreviewKind(bool& bOutLive)
{
	FString P = CVarJamBoostPreview.GetValueOnGameThread().TrimStartAndEnd();
	bOutLive = P.EndsWith(TEXT("!"));
	if (bOutLive) P.LeftChopInline(1);
	return P.IsEmpty() ? NAME_None : FName(*P);
}

int32 AJamBoostAura::AuraMode() { return FMath::Clamp(CVarJamBoostAura.GetValueOnGameThread(), 0, 2); }

FName AJamBoostAura::HeldKindFor(const UJamSim* Sim, int32 Skater, bool& bOutLive)
{
	bOutLive = false;
	if (!Sim || !Sim->GetSkaters().IsValidIndex(Skater)) return NAME_None;
	const FJamSkaterState& S = Sim->GetSkaters()[Skater];
	bOutLive = S.ShieldFrames > 0 || !S.AbilityArmed.IsNone();
	// The same order as the HUD's plate (HokejGameMode's PushPresentation): the held kind, else the armed one after its last charge, else a running shield.
	if (const FJamAbilityConfig* Ab = Sim->HeldAbility(Skater)) return Ab->Kind;
	if (!S.AbilityArmed.IsNone()) return S.AbilityArmed;
	if (S.ShieldFrames > 0) return FName(TEXT("shield"));
	if (S.bHuman) return PreviewKind(bOutLive);   // proof only: empty by default
	return NAME_None;
}

bool AJamBoostAura::CallSetBoost(UObject* Figure, FName InKind, bool bOn)
{
	// By name, with a signature check: the skater-visual interface's own SetBoost(FName) (the effect boosts) has one input and is not this call.
	if (!Figure) return false;
	for (const TCHAR* Name : { TEXT("SetBoost"), TEXT("SetBoostEquipped") })
	{
		UFunction* F = Figure->FindFunction(FName(Name));
		if (!F) continue;
		FNameProperty* KindP = nullptr;
		FBoolProperty* OnP = nullptr;
		int32 N = 0;
		for (TFieldIterator<FProperty> It(F); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
		{
			if (It->HasAnyPropertyFlags(CPF_ReturnParm | CPF_OutParm)) { N = -1; break; }
			if (N == 0) KindP = CastField<FNameProperty>(*It);
			else if (N == 1) OnP = CastField<FBoolProperty>(*It);
			++N;
		}
		if (N != 2 || !KindP || !OnP) continue;
		uint8* Params = static_cast<uint8*>(FMemory_Alloca(FMath::Max<int32>(F->ParmsSize, 1)));
		FMemory::Memzero(Params, F->ParmsSize);
		for (TFieldIterator<FProperty> It(F); It && It->HasAnyPropertyFlags(CPF_Parm); ++It) It->InitializeValue_InContainer(Params);
		KindP->SetPropertyValue_InContainer(Params, InKind);
		OnP->SetPropertyValue_InContainer(Params, bOn);
		Figure->ProcessEvent(F, Params);
		for (TFieldIterator<FProperty> It(F); It && It->HasAnyPropertyFlags(CPF_Parm); ++It) It->DestroyValue_InContainer(Params);
		return true;
	}
	return false;
}

bool AJamBoostAura::CallPlayCelebrate(UObject* Figure, FName InKind, int32 Index)
{
	UFunction* F = Figure ? Figure->FindFunction(FName(TEXT("PlayCelebrate"))) : nullptr;
	if (!F) return false;
	// No input, or one: a name (the kind), a string (the kind) or a number (the take's index 0..3).
	FProperty* One = nullptr;
	int32 N = 0;
	for (TFieldIterator<FProperty> It(F); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		if (It->HasAnyPropertyFlags(CPF_ReturnParm | CPF_OutParm)) return false;
		One = *It;
		++N;
	}
	if (N > 1) return false;
	if (One && !(CastField<FNameProperty>(One) || CastField<FStrProperty>(One) || CastField<FNumericProperty>(One))) return false;
	uint8* Params = static_cast<uint8*>(FMemory_Alloca(FMath::Max<int32>(F->ParmsSize, 1)));
	FMemory::Memzero(Params, F->ParmsSize);
	if (One)
	{
		One->InitializeValue_InContainer(Params);
		if (FNameProperty* NP = CastField<FNameProperty>(One)) NP->SetPropertyValue_InContainer(Params, InKind);
		else if (FStrProperty* SP = CastField<FStrProperty>(One)) SP->SetPropertyValue_InContainer(Params, InKind.ToString());
		else if (FNumericProperty* Num = CastField<FNumericProperty>(One))
		{
			void* V = Num->ContainerPtrToValuePtr<void>(Params);
			if (Num->IsFloatingPoint()) Num->SetFloatingPointPropertyValue(V, static_cast<double>(Index)); else Num->SetIntPropertyValue(V, static_cast<int64>(Index));
		}
	}
	Figure->ProcessEvent(F, Params);
	if (One) One->DestroyValue_InContainer(Params);
	return true;
}

bool AJamBoostAura::IsRingShown() const { return bRingShown; }

void AJamBoostAura::Show(FName InKind, bool bLive, bool bRing, double FrameF)
{
	const bool bShield = InKind == FName(TEXT("shield")) && bLive;
	if (InKind != ShownKind || bShield != bShieldShape || !Mid)
	{
		ShownKind = InKind;
		bShieldShape = bShield;
		if (!InKind.IsNone())
		{
			if (!Mid) Mid = MakeLight(this, AbilityColour(InKind), 3.f);
			if (Mid)
			{
				Mid->SetVectorParameterValue(TEXT("TeamColour"), AbilityColour(InKind));
				Mid->SetVectorParameterValue(TEXT("Color"), AbilityColour(InKind));
			}
			// A running shield: a wider ring of taller bars round the chest; otherwise a waist ring.
			BuildRing(this, Pivot, Ring, Mid, RingSegments, (bShield ? ShieldRadiusM : RingRadiusM) * 100.0, 0.55, 5.0, bShield ? 24.0 : 6.0, bShield ? 10.0 : -3.0);
			ShowRing(Ring, bRingShown);   // new parts are born visible
		}
	}
	const bool bShow = bRing && !InKind.IsNone();
	if (bShow != bRingShown) { bRingShown = bShow; ShowRing(Ring, bShow); }
	if (!bShow) return;
	// Live (armed, a shield running): a fast turn and a bright, breathing light; held: a slow turn.
	const double Yaw = FMath::Fmod(FrameF * (bLive ? 6.0 : 2.0), 360.0);
	Pivot->SetRelativeRotation(FRotator(0.0, Yaw, 0.0));
	if (Mid) Mid->SetScalarParameterValue(TEXT("Emissive"), bLive ? 4.f + 1.5f * static_cast<float>(FMath::Sin(FrameF * 0.25)) : 2.5f);
}

void AJamBoostAura::Sync(UWorld* World, const UJamSim* Sim, const TArray<TObjectPtr<AJamSkater>>& Skaters, TArray<TObjectPtr<AJamBoostAura>>& Auras, double FrameF)
{
	if (!World || !Sim) return;
	const int32 Mode = AuraMode();
	for (AJamSkater* Sk : Skaters)
	{
		if (!Sk) continue;
		const int32 Idx = Sk->GetSimIndex();
		if (!Sim->GetSkaters().IsValidIndex(Idx)) continue;
		bool bLive = false;
		const FName Held = HeldKindFor(Sim, Idx, bLive);
		TObjectPtr<AJamBoostAura>* Found = Auras.FindByPredicate([Idx](const AJamBoostAura* A) { return A && A->SkaterIndex == Idx; });
		AJamBoostAura* A = Found ? Found->Get() : nullptr;
		if (!A)
		{
			if (Held.IsNone()) continue;   // made on the first hold only
			FActorSpawnParameters Sp;
			Sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			A = World->SpawnActor<AJamBoostAura>(AJamBoostAura::StaticClass(), Sk->GetActorTransform(), Sp);
			if (!A) continue;
			A->SkaterIndex = Idx;
			Auras.Add(A);
		}
		// Device B's figure call on a change (by name): SetBoost(old, false), then SetBoost(new, true).
		if (Held != A->FigureKind)
		{
			UObject* Fig = Sk->GetVisualObject();
			bool bTook = false;
			if (!A->FigureKind.IsNone()) bTook = CallSetBoost(Fig, A->FigureKind, false);
			if (!Held.IsNone()) bTook = CallSetBoost(Fig, Held, true);
			A->bFigureTook = bTook && !Held.IsNone();
			UE_LOG(LogJamSim, Log, TEXT("JAMBOOST equip frame=%lld skater=%d kind=%s was=%s figure_call=%s ring=%d"), Sim->GetFrame(), Idx, *Held.ToString(), *A->FigureKind.ToString(),
				bTook ? TEXT("SetBoost") : TEXT("none"), (Mode == 2 || (Mode == 1 && !A->bFigureTook)) && !Held.IsNone() ? 1 : 0);
			A->FigureKind = Held;
			A->Figure = Fig;
		}
		const FJamSkaterState& S = Sim->GetSkaters()[Idx];
		const bool bRing = (Mode == 2 || (Mode == 1 && !A->bFigureTook)) && !S.IsDown() && !S.InBox();
		// Follows the drawn figure (its render position this frame), the ring at the waist.
		const FVector At = Sk->GetActorLocation();
		A->SetActorLocation(FVector(At.X, At.Y, RingHeightM * 100.0));
		A->Show(Held, bLive, bRing, FrameF);
	}
}

void AJamBoostAura::DestroyAll(TArray<TObjectPtr<AJamBoostAura>>& Auras)
{
	for (AJamBoostAura* A : Auras)
	{
		if (!A) continue;
		if (!A->FigureKind.IsNone()) CallSetBoost(A->Figure.Get(), A->FigureKind, false);   // a restart or a new match: the figure lets go too
		A->Destroy();
	}
	Auras.Reset();
}

// ---------------------------------------------------------------- lane A188: the goal's celebration

FName JamBoostLook::CelebrateKind(int32 GoalCount, bool bScorer)
{
	static const FName Kinds[3] = { FName(TEXT("arms_up")), FName(TEXT("stick_raise")), FName(TEXT("knee_glide")) };
	const int32 I = ((GoalCount % 3) + 3) % 3;
	if (bScorer) return Kinds[I];
	return I == 1 ? Kinds[2] : Kinds[1];   // the mate raises the stick, or glides when the scorer raises his
}

int32 JamBoostLook::PickScorer(const TArray<int32>& Teams, const TArray<FVector2D>& Pos, int32 ScoringTeam, int32 Who, const FVector2D& WhereM, int32& OutMate)
{
	OutMate = -1;
	int32 Scorer = Teams.IsValidIndex(Who) && Teams[Who] == ScoringTeam ? Who : -1;
	if (Scorer < 0)
	{
		double Best = 1e18;
		for (int32 i = 0; i < Teams.Num(); ++i)
		{
			if (Teams[i] != ScoringTeam || !Pos.IsValidIndex(i)) continue;
			const double D = FVector2D::DistSquared(Pos[i], WhereM);
			if (D < Best) { Best = D; Scorer = i; }
		}
	}
	if (Scorer < 0) return -1;
	double Best = 1e18;
	for (int32 i = 0; i < Teams.Num(); ++i)
	{
		if (i == Scorer || Teams[i] != ScoringTeam || !Pos.IsValidIndex(i)) continue;
		const double D = FVector2D::DistSquared(Pos[i], Pos[Scorer]);
		if (D < Best) { Best = D; OutMate = i; }
	}
	return Scorer;
}

int32 JamBoostLook::CelebrateGoal(const UJamSim* Sim, const TArray<TObjectPtr<AJamSkater>>& Skaters, int32 Who, const FVector2D& WhereM)
{
	if (!Sim) return 0;
	TArray<int32> Teams;
	TArray<FVector2D> Pos;
	for (const FJamSkaterState& S : Sim->GetSkaters()) { Teams.Add(S.InBox() ? -1 : S.Team); Pos.Add(S.Pos); }
	// The scoring team: the one whose attacked goal line is on the puck's side (the binder's rule, lane A47: an own goal counts for the other side).
	const int32 Scoring = (WhereM.X > 0.0) == (Sim->AttackGoalX(0) > 0.0) ? 0 : 1;
	int32 Mate = -1;
	const int32 Scorer = PickScorer(Teams, Pos, Scoring, Who, WhereM, Mate);
	const int32 Count = Sim->GetScore(0) + Sim->GetScore(1);
	int32 Took = 0;
	FString Calls;
	for (int32 Pass = 0; Pass < 2; ++Pass)
	{
		const int32 Idx = Pass == 0 ? Scorer : Mate;
		if (Idx < 0) continue;
		const FName K = CelebrateKind(Count, Pass == 0);
		const TObjectPtr<AJamSkater>* Sk = Skaters.FindByPredicate([Idx](const AJamSkater* A) { return A && A->GetSimIndex() == Idx; });
		const bool bOk = Sk && *Sk && AJamBoostAura::CallPlayCelebrate((*Sk)->GetVisualObject(), K, Pass == 0 ? ((Count % 3) + 3) % 3 : 3);
		Took += bOk ? 1 : 0;
		Calls += FString::Printf(TEXT(" %s=%d:%s:%s"), Pass == 0 ? TEXT("scorer") : TEXT("mate"), Idx, *K.ToString(), bOk ? TEXT("PlayCelebrate") : TEXT("none"));
	}
	UE_LOG(LogJamSim, Log, TEXT("JAMFIG celebrate frame=%lld team=%d last_touch=%d%s"), Sim->GetFrame(), Scoring, Who, *Calls);
	return Took;
}

#if WITH_DEV_AUTOMATION_TESTS
// Jam.Fx.BoostLook: the boosts' look rules (lane A188). Pure: no world.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFxBoostLook, "Jam.Fx.BoostLook", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamFxBoostLook::RunTest(const FString&)
{
	// Arsenal's two kinds read apart by silhouette and by colour.
	TestEqual(TEXT("speed is a cone"), AJamPickupActor::StandInShape(TEXT("speed")), FName(TEXT("cone")));
	TestEqual(TEXT("precision is a diamond"), AJamPickupActor::StandInShape(TEXT("precision")), FName(TEXT("diamond")));
	TestNotEqual(TEXT("the two silhouettes differ in mesh"), AJamPickupActor::StandInMeshPath(TEXT("cone")), AJamPickupActor::StandInMeshPath(TEXT("diamond")));
	TestFalse(TEXT("speed and precision differ in colour"), JamEffects::KindColour(TEXT("speed")).Equals(JamEffects::KindColour(TEXT("precision")), 0.05f));
	// Lane A210: the kinds' colours are the contract's (contracts/events.md, the pickup kind's colour; device B's props and effects carry the same).
	TestTrue(TEXT("speed is the contract's cyan"), JamEffects::KindColour(TEXT("speed")).Equals(FLinearColor(0.1f, 0.9f, 1.0f), 0.001f));
	TestTrue(TEXT("precision is the contract's magenta"), JamEffects::KindColour(TEXT("precision")).Equals(FLinearColor(1.0f, 0.2f, 0.9f), 0.001f));
	TestTrue(TEXT("heavy_puck is the contract's violet"), JamEffects::KindColour(TEXT("heavy_puck")).Equals(FLinearColor(0.55f, 0.3f, 1.0f), 0.001f));
	TestTrue(TEXT("big_puck is the contract's amber"), JamEffects::KindColour(TEXT("big_puck")).Equals(FLinearColor(1.0f, 0.75f, 0.1f), 0.001f));
	TestTrue(TEXT("reverse_other is the contract's red"), JamEffects::KindColour(TEXT("reverse_other")).Equals(FLinearColor(1.0f, 0.15f, 0.1f), 0.001f));
	TestTrue(TEXT("custom and unknown are white"), JamEffects::KindColour(TEXT("custom")).Equals(FLinearColor::White, 0.001f) && JamEffects::KindColour(TEXT("no_such_kind")).Equals(FLinearColor::White, 0.001f));
	TestEqual(TEXT("B's prop by name"), AJamPickupActor::BoostPropPath(TEXT("speed")), FString(TEXT("/JamAssets/Props/SM_Boost_speed.SM_Boost_speed")));
	TestEqual(TEXT("the beacon is on by default"), AJamPickupActor::BeaconOn(), true);
	// The low profile: never full height over a skater or the puck.
	TestEqual(TEXT("a skater on it: low"), AJamPickupActor::BeaconHeightFor(0.0), 0.0);
	TestEqual(TEXT("at the clear distance: low"), AJamPickupActor::BeaconHeightFor(AJamPickupActor::BeaconClearM), 0.0);
	TestEqual(TEXT("a metre outside it: full"), AJamPickupActor::BeaconHeightFor(AJamPickupActor::BeaconClearM + 1.0), 1.0);
	TestEqual(TEXT("nobody near: full"), AJamPickupActor::BeaconHeightFor(1e9), 1.0);
	// The four abilities: four colours.
	const TCHAR* Kinds[4] = { TEXT("shotgun"), TEXT("hip_check"), TEXT("shield"), TEXT("slow_puck") };
	for (int32 i = 0; i < 4; ++i)
	{
		for (int32 j = i + 1; j < 4; ++j)
		{
			TestFalse(FString::Printf(TEXT("%s and %s differ"), Kinds[i], Kinds[j]), AJamBoostAura::AbilityColour(Kinds[i]).Equals(AJamBoostAura::AbilityColour(Kinds[j]), 0.05f));
		}
	}
	TestEqual(TEXT("the aura is on by default"), AJamBoostAura::AuraMode(), 1);
	// By name: an object without the functions takes nothing (the stand-ins stay).
	UObject* Plain = NewObject<UJamSim>(GetTransientPackage());   // a concrete class without either function
	TestFalse(TEXT("no SetBoost(Kind, bOn): not called"), AJamBoostAura::CallSetBoost(Plain, TEXT("shotgun"), true));
	TestFalse(TEXT("no PlayCelebrate: not called"), AJamBoostAura::CallPlayCelebrate(Plain, TEXT("arms_up"), 0));
	// The celebration's pair: the last touch scores for his team; an own goal goes to the scoring team's nearest; the mate is the nearest team mate.
	const TArray<int32> Teams = { 0, 0, 0, 1, 1 };
	const TArray<FVector2D> Pos = { FVector2D(30, 0), FVector2D(20, 5), FVector2D(-10, 0), FVector2D(40, 1), FVector2D(0, 0) };
	int32 Mate = -2;
	TestEqual(TEXT("the last touch scores"), JamBoostLook::PickScorer(Teams, Pos, 0, 0, FVector2D(41, 0), Mate), 0);
	TestEqual(TEXT("his nearest mate"), Mate, 1);
	TestEqual(TEXT("an own goal (last touch 3 of team 1): team 0's nearest to the puck"), JamBoostLook::PickScorer(Teams, Pos, 0, 3, FVector2D(41, 0), Mate), 0);
	TestEqual(TEXT("no skater of the team: nobody"), JamBoostLook::PickScorer({ 1, 1 }, { FVector2D::ZeroVector, FVector2D::ZeroVector }, 0, 0, FVector2D::ZeroVector, Mate), -1);
	TestNotEqual(TEXT("scorer and mate take different takes"), JamBoostLook::CelebrateKind(1, true), JamBoostLook::CelebrateKind(1, false));
	TestNotEqual(TEXT("goal after goal, a different take"), JamBoostLook::CelebrateKind(1, true), JamBoostLook::CelebrateKind(2, true));
	return true;
}
#endif

// ---------------------------------------------------------------- lane A188: the measurement

static TAutoConsoleVariable<int32> CVarJamBoostProbe(TEXT("jam.boost.Probe"), 0,
	TEXT("Lane A188's measurement: N > 0 logs every N sim frames a JAMBOOST seen line per pickup on the ice (camera distance, height and width on screen in pixels) and a JAMBOOST passes line (the human within 5 m of a pickup he did not take). 0 off."), ECVF_Default);

bool AJamPickupActor::ScreenSize(APlayerController* PC, double& OutHeightPx, double& OutWidthPx, double& OutDistM) const
{
	if (!PC || !PC->PlayerCameraManager) return false;
	FBox Box(ForceInit);
	TInlineComponentArray<UPrimitiveComponent*> Prims(this);
	for (const UPrimitiveComponent* P : Prims) if (P && P->IsVisible() && P->IsRegistered()) Box += P->Bounds.GetBox();
	if (!Box.IsValid) return false;
	const FVector Cam = PC->PlayerCameraManager->GetCameraLocation();
	OutDistM = FVector::Dist(Box.GetCenter(), Cam) / 100.0;
	FVector2D Min(1e9, 1e9), Max(-1e9, -1e9);
	for (int32 i = 0; i < 8; ++i)
	{
		const FVector C((i & 1) ? Box.Max.X : Box.Min.X, (i & 2) ? Box.Max.Y : Box.Min.Y, (i & 4) ? Box.Max.Z : Box.Min.Z);
		FVector2D S;
		if (!PC->ProjectWorldLocationToScreen(C, S, false)) return false;
		Min = FVector2D::Min(Min, S);
		Max = FVector2D::Max(Max, S);
	}
	OutHeightPx = Max.Y - Min.Y;
	OutWidthPx = Max.X - Min.X;
	return true;
}

void AJamPickupActor::Probe(APlayerController* PC, const TArray<TObjectPtr<AJamPickupActor>>& Actors, const FVector2D& HumanM, int64 Frame)
{
	const int32 Every = CVarJamBoostProbe.GetValueOnGameThread();
	if (Every <= 0 || !PC) return;
	// The passes: the human inside 5 m of a pickup, then outside it with the pickup still there (passed by), or the pickup gone while he was inside 1.5 m (taken).
	static TMap<int32, double> Closest;   // pickup id -> the human's closest distance while inside 5 m (game thread only; a proof cvar)
	static int32 Passes = 0, Takes = 0, Seen = 0;
	static int64 LastFrame = -1;
	if (Frame < LastFrame) { Closest.Reset(); Passes = Takes = Seen = 0; }
	if (Frame == LastFrame) return;
	LastFrame = Frame;
	TSet<int32> Alive;
	for (const AJamPickupActor* A : Actors)
	{
		if (!A) continue;
		Alive.Add(A->PickupId);
		const double D = FVector2D::Distance(HumanM, A->PosM);
		if (D <= 5.0) { double& C = Closest.FindOrAdd(A->PickupId, 1e9); C = FMath::Min(C, D); }
		else if (Closest.Contains(A->PickupId)) { ++Passes; UE_LOG(LogJamSim, Log, TEXT("JAMBOOST pass_by frame=%lld pickup=%d kind=%s closest_m=%.2f"), Frame, A->PickupId, *A->Kind.ToString(), Closest[A->PickupId]); Closest.Remove(A->PickupId); }
	}
	for (auto It = Closest.CreateIterator(); It; ++It)
	{
		if (Alive.Contains(It.Key())) continue;
		if (It.Value() <= 1.5) ++Takes;
		It.RemoveCurrent();
	}
	if (Frame % Every != 0) return;
	for (const AJamPickupActor* A : Actors)
	{
		double H = 0.0, W = 0.0, Dm = 0.0;
		if (!A) continue;
		const bool bOn = A->ScreenSize(PC, H, W, Dm);
		FVector2D Sp = FVector2D::ZeroVector;
		const bool bInView = bOn && PC->ProjectWorldLocationToScreen(A->GetActorLocation(), Sp, false);
		int32 VX = 0, VY = 0;
		PC->GetViewportSize(VX, VY);
		const bool bOnScreen = bInView && Sp.X >= 0 && Sp.Y >= 0 && Sp.X <= VX && Sp.Y <= VY;
		Seen += bOnScreen ? 1 : 0;
		UE_LOG(LogJamSim, Log, TEXT("JAMBOOST seen frame=%lld pickup=%d kind=%s look=%s cam_m=%.1f h_px=%.1f w_px=%.1f on_screen=%d view=%dx%d"), Frame, A->PickupId, *A->Kind.ToString(),
			A->bBeacon ? TEXT("beacon") : (A->bLookB ? TEXT("b") : TEXT("toy")), Dm, bOn ? H : -1.0, bOn ? W : -1.0, bOnScreen ? 1 : 0, VX, VY);
	}
	UE_LOG(LogJamSim, Log, TEXT("JAMBOOST passes frame=%lld on_ice=%d passed_within_5m=%d taken_by_human=%d"), Frame, Actors.Num(), Passes, Takes);
}
