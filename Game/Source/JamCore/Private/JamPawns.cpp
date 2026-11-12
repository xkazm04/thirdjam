#include "JamPawns.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	UStaticMesh* LoadShape(const TCHAR* Path) { return LoadObject<UStaticMesh>(nullptr, Path); }

	UStaticMeshComponent* MakePart(USceneComponent* Parent, const TCHAR* Name, const TCHAR* MeshPath, const FVector& Loc, const FRotator& Rot, const FVector& Scale)
	{
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(Parent->GetOwner(), Name);
		C->SetStaticMesh(LoadShape(MeshPath));
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->SetGenerateOverlapEvents(false);
		C->SetupAttachment(Parent);
		C->SetRelativeLocation(Loc);
		C->SetRelativeRotation(Rot);
		C->SetRelativeScale3D(Scale);
		C->RegisterComponent();
		return C;
	}

	// Rink (x, z) metres -> Unreal centimetres on the ice.
	FVector Lerp2D(const FVector2D& A, const FVector2D& B, float Alpha, double HeightM)
	{
		return UJamSim::ToWorld(A + (B - A) * Alpha, HeightM);
	}
}

// ---------- UJamPlaceholderVisual ----------

UJamPlaceholderVisual::UJamPlaceholderVisual()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UJamPlaceholderVisual::OnRegister()
{
	Super::OnRegister();
	if (Body || !GetOwner()) return;
	// Capsule body 1.8 m, head 0.3 m, stick 1.5 m: engine basic shapes (1 m = 100 units, basic shapes are 100 cm).
	Body = MakePart(this, TEXT("PH_Body"), TEXT("/Engine/BasicShapes/Cylinder.Cylinder"), FVector(0, 0, 70), FRotator::ZeroRotator, FVector(0.6, 0.6, 1.4));
	Head = MakePart(this, TEXT("PH_Head"), TEXT("/Engine/BasicShapes/Sphere.Sphere"), FVector(0, 0, 160), FRotator::ZeroRotator, FVector(0.32));
	Stick = MakePart(this, TEXT("PH_Stick"), TEXT("/Engine/BasicShapes/Cylinder.Cylinder"), FVector(60, 25, 40), FRotator(-55, 0, 0), FVector(0.05, 0.05, 1.5));
	if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
	{
		Mid = UMaterialInstanceDynamic::Create(Base, this);
		Body->SetMaterial(0, Mid);
		Mid->SetVectorParameterValue(TEXT("Color"), Colour);
	}
}

void UJamPlaceholderVisual::SetTeam_Implementation(int32 TeamId, FLinearColor InColour)
{
	Colour = InColour;
	if (Mid) Mid->SetVectorParameterValue(TEXT("Color"), Colour);
}

void UJamPlaceholderVisual::UpdateMotion_Implementation(float Speed, float TurnRate, FRotator Facing)
{
	if (bDown)
	{
		// Tip over towards the fall direction, in the pawn's local frame.
		const FVector WorldDir(DownDir.X, DownDir.Y, 0);
		const FVector Local = GetComponentTransform().InverseTransformVectorNoScale(WorldDir);
		const FVector Axis = FVector::CrossProduct(FVector::UpVector, Local).GetSafeNormal();
		SetRelativeRotation(FQuat(Axis.IsNearlyZero() ? FVector::RightVector : Axis, FMath::DegreesToRadians(80.f)).Rotator());
		return;
	}
	// Lean into turns (TurnRate + = left), capped.
	const float Lean = FMath::Clamp(2.5f * Speed * TurnRate, -35.f, 35.f);
	SetRelativeRotation(FRotator(0, 0, -Lean));
	if (Stick)
	{
		const float Swing = SwingFrames > 0 ? -40.f * (SwingFrames / 10.f) : 0.f;
		Stick->SetRelativeRotation(FRotator(-55, Swing, 0));
		if (SwingFrames > 0) --SwingFrames;
	}
}

void UJamPlaceholderVisual::SetKnockedDown_Implementation(bool bInDown, float DirX, float DirZ)
{
	bDown = bInDown;
	DownDir = FVector2D(DirX, DirZ);
	if (!bDown) SetRelativeRotation(FRotator::ZeroRotator);
}

// ---------- AJamSkater ----------

AJamSkater::AJamSkater()
{
	PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	Root->SetMobility(EComponentMobility::Movable);
	AutoPossessAI = EAutoPossessAI::Disabled;
}

void AJamSkater::InitSkater(int32 InSimIndex, int32 InTeam, const FLinearColor& Colour, int32 Number, UClass* VisualClass)
{
	SimIndex = InSimIndex;
	Team = InTeam;
	UObject* Visual = nullptr;
	if (VisualClass && VisualClass->ImplementsInterface(UJamSkaterVisual::StaticClass()))
	{
		if (VisualClass->IsChildOf(UActorComponent::StaticClass()))
		{
			UActorComponent* C = NewObject<UActorComponent>(this, VisualClass, TEXT("SkaterVisual"));
			if (USceneComponent* SC = Cast<USceneComponent>(C)) SC->SetupAttachment(Root);
			C->RegisterComponent();
			AddInstanceComponent(C);
			Visual = C;
		}
		else if (VisualClass->IsChildOf(AActor::StaticClass()))
		{
			FActorSpawnParameters P;
			P.Owner = this;
			P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AActor* A = GetWorld()->SpawnActor<AActor>(VisualClass, GetActorTransform(), P);
			if (A) { A->AttachToComponent(Root, FAttachmentTransformRules::SnapToTargetNotIncludingScale); Visual = A; }
		}
	}
	if (!Visual)
	{
		if (VisualClass) UE_LOG(LogJamSim, Warning, TEXT("JAMSKATER visual class %s does not implement IJamSkaterVisual; using the placeholder"), *VisualClass->GetPathName());
		UJamPlaceholderVisual* C = NewObject<UJamPlaceholderVisual>(this, TEXT("SkaterVisual"));
		C->SetupAttachment(Root);
		C->RegisterComponent();
		AddInstanceComponent(C);
		Visual = C;
	}
	VisualObject = Visual;
	IJamSkaterVisual::Execute_SetTeam(Visual, Team, Colour);
	IJamSkaterVisual::Execute_SetNumber(Visual, FMath::Clamp(Number, 0, 99));
	IJamSkaterVisual::Execute_SetHasPuck(Visual, false);
	IJamSkaterVisual::Execute_SetBoost(Visual, NAME_None);
	bWasDown = false;
	bHadPuck = false;
}

void AJamSkater::ApplySimFrame(const FJamSkaterState& S)
{
	if (!VisualObject) return;
	UObject* V = VisualObject;
	if (S.bShotThisFrame) IJamSkaterVisual::Execute_PlayShoot(V);
	if (S.bCheckThisFrame) IJamSkaterVisual::Execute_PlayCheck(V);
	if (S.IsDown() != bWasDown)
	{
		bWasDown = S.IsDown();
		IJamSkaterVisual::Execute_SetKnockedDown(V, bWasDown, static_cast<float>(S.KnockDir.X), static_cast<float>(S.KnockDir.Y));
	}
	if (S.bHasPuck != bHadPuck)
	{
		bHadPuck = S.bHasPuck;
		IJamSkaterVisual::Execute_SetHasPuck(V, bHadPuck);
	}
	// Contract: turn rate positive = turning left seen from above; the sim's yaw grows to the right (Unreal yaw).
	IJamSkaterVisual::Execute_UpdateMotion(V, static_cast<float>(S.Vel.Size()), static_cast<float>(-S.YawRate),
		FRotator(0.0, FMath::RadiansToDegrees(S.Yaw), 0.0));
}

void AJamSkater::ApplyRender(const FJamSkaterState& S, float Alpha)
{
	const FVector Loc = Lerp2D(S.PrevPos, S.Pos, Alpha, 0.0);
	const double DYaw = FMath::UnwindRadians(S.Yaw - S.PrevYaw);
	const double Yaw = S.PrevYaw + DYaw * Alpha;
	SetActorLocationAndRotation(Loc, FRotator(0.0, FMath::RadiansToDegrees(Yaw), 0.0));
}

// ---------- AJamPuck ----------

AJamPuck::AJamPuck()
{
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cyl(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (Cyl.Succeeded()) Mesh->SetStaticMesh(Cyl.Object);
	// R-0002: the puck is outline-only in the painted-cel look (stencil 3).
	Mesh->SetRenderCustomDepth(true);
	Mesh->SetCustomDepthStencilValue(3);
}

void AJamPuck::InitPuck(double RadiusM, const FLinearColor& Colour)
{
	// Basic cylinder: 100 cm across, 100 cm tall. Puck: 2r across, 0.08 m tall.
	HalfHeightM = 0.04;
	Mesh->SetRelativeScale3D(FVector(RadiusM * 2.0, RadiusM * 2.0, 0.08));
	if (UMaterialInterface* Look = LoadObject<UMaterialInterface>(nullptr, TEXT("/JamAssets/Look/MI_Toon_Puck.MI_Toon_Puck"), nullptr, LOAD_NoWarn | LOAD_Quiet))
	{
		Mesh->SetMaterial(0, Look); // R-0002: device B's instance when it exists
	}
	else if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
	{
		UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Base, this);
		Mid->SetVectorParameterValue(TEXT("Color"), Colour);
		Mesh->SetMaterial(0, Mid);
	}
}

void AJamPuck::ApplyRender(const FJamPuckState& P, float Alpha)
{
	SetActorLocation(Lerp2D(P.PrevPos, P.Pos, Alpha, HalfHeightM));
}
