#include "JamPawns.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "JamEventBus.h"
#include "Engine/GameInstance.h"

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

void UJamPlaceholderVisual::SetKnockedDown_Implementation(bool bInDown, float DirX, float DirZ, int32 KnockdownFrames)
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

UObject* AJamSkater::CreateVisual(UClass* VisualClass)
{
	UObject* Visual = nullptr;
	// A unique name each time: ResetVisual (lane A31) destroys the old component, which stays in memory until GC, so "SkaterVisual" alone would collide.
	const FName CompName = MakeUniqueObjectName(this, VisualClass ? VisualClass : UJamPlaceholderVisual::StaticClass(), TEXT("SkaterVisual"));
	if (VisualClass && VisualClass->ImplementsInterface(UJamSkaterVisual::StaticClass()))
	{
		if (VisualClass->IsChildOf(UActorComponent::StaticClass()))
		{
			UActorComponent* C = NewObject<UActorComponent>(this, VisualClass, CompName);
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
		UJamPlaceholderVisual* C = NewObject<UJamPlaceholderVisual>(this, CompName);
		C->SetupAttachment(Root);
		C->RegisterComponent();
		AddInstanceComponent(C);
		Visual = C;
	}
	return Visual;
}

void AJamSkater::SetupVisual(UObject* Visual, const FLinearColor& Colour, int32 Number)
{
	// C-0001: SetTeam and SetNumber once at spawn (B-to-A-0021 point 2: no number is drawn until SetNumber is called), then the resting state.
	VisualObject = Visual;
	IJamSkaterVisual::Execute_SetTeam(Visual, Team, Colour);
	IJamSkaterVisual::Execute_SetNumber(Visual, FMath::Clamp(Number, 0, 99));
	IJamSkaterVisual::Execute_SetHasPuck(Visual, false);
	IJamSkaterVisual::Execute_SetBoost(Visual, NAME_None);
	bWasDown = false;
	bHadPuck = false;
	bWasAtFaceoff = false;
	bWasPinned = false;
	LastCharge = -1.f;   // lane A65: the first ApplySimFrame sends the charge (SetShotCharge goes only on change)
	LastAimDeg = 0.f;
	LastAimReach = 0.f;
}

int64 AJamSkater::SimFrameForLog() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UJamEventBus* Bus = GI->GetSubsystem<UJamEventBus>()) return Bus->GetSimFrame();
	}
	return -1;
}

void AJamSkater::InitSkater(int32 InSimIndex, int32 InTeam, const FLinearColor& Colour, int32 Number, UClass* VisualClass)
{
	SimIndex = InSimIndex;
	Team = InTeam;
	SetupVisual(CreateVisual(VisualClass), Colour, Number);
}

bool AJamSkater::ResetVisual(const FLinearColor& Colour, int32 Number, UClass* VisualClass)
{
	if (VisualObject && VisualClass && VisualObject->GetClass() == VisualClass) return false;
	if (UActorComponent* OldC = Cast<UActorComponent>(VisualObject.Get()))
	{
		RemoveInstanceComponent(OldC);
		OldC->DestroyComponent();
	}
	else if (AActor* OldA = Cast<AActor>(VisualObject.Get()))
	{
		OldA->Destroy();
	}
	VisualObject = nullptr;
	SetupVisual(CreateVisual(VisualClass), Colour, Number);
	return true;
}

void AJamSkater::ApplySimFrame(const FJamSkaterState& S)
{
	if (!VisualObject) return;
	UObject* V = VisualObject;
	// B-to-A-0024 point 2 (lane A31): PlayShoot only for a shot; a pass is PlayPass alone (below), never a charge-0 shot in the same frame.
	if (S.bShotThisFrame) IJamSkaterVisual::Execute_PlayShoot(V, S.LastShotCharge);
	if (S.bCheckThisFrame) IJamSkaterVisual::Execute_PlayCheck(V);
	if (S.IsDown() != bWasDown)
	{
		bWasDown = S.IsDown();
		// C-0001 v4: the down time rides along (KnockFrames is the whole knockdown on the frame it starts: StepSkater counts it down from the next step on).
		IJamSkaterVisual::Execute_SetKnockedDown(V, bWasDown, static_cast<float>(S.KnockDir.X), static_cast<float>(S.KnockDir.Y), bWasDown ? S.KnockFrames : 0);
	}
	// Lane A45 (C-0001 v4, R-0031 point 1): B's four calls from the sim's read-outs (JamSim.h, never hashed). One JAMVIS line each, a few per minute, so B can read what was sent.
	if (S.bBumpThisFrame && !S.IsDown())
	{
		IJamSkaterVisual::Execute_PlayHitReaction(V, static_cast<float>(S.BumpDir.X), static_cast<float>(S.BumpDir.Y));
		UE_LOG(LogJamSim, Log, TEXT("JAMVIS hit frame=%lld skater=%d team=%d dir=(%.2f, %.2f) (PlayHitReaction: a collision at 3 m/s closing or more, no check landed)"),
			SimFrameForLog(), S.Index, S.Team, S.BumpDir.X, S.BumpDir.Y);
	}
	if (S.bBoardHitThisFrame)
	{
		IJamSkaterVisual::Execute_PlayBoardHit(V);
		UE_LOG(LogJamSim, Log, TEXT("JAMVIS board frame=%lld skater=%d team=%d (PlayBoardHit: the skater board event, 4 m/s and up)"), SimFrameForLog(), S.Index, S.Team);
	}
	if (S.bBoardPinned != bWasPinned)
	{
		bWasPinned = S.bBoardPinned;
		IJamSkaterVisual::Execute_SetBoardPin(V, bWasPinned);
		UE_LOG(LogJamSim, Log, TEXT("JAMVIS pin frame=%lld skater=%d team=%d on=%d board_frames=%d (SetBoardPin: 10 frames on the boards with an opponent within 1.2 m)"),
			SimFrameForLog(), S.Index, S.Team, bWasPinned ? 1 : 0, S.BoardFrames);
	}
	if (S.StickHitResult != 0)
	{
		IJamSkaterVisual::Execute_PlayStickHit(V, S.StickHitResult == 2);
		UE_LOG(LogJamSim, Log, TEXT("JAMVIS stickhit frame=%lld skater=%d team=%d lifted=%d (PlayStickHit: the carrier lost the puck to a clean %s)"),
			SimFrameForLog(), S.Index, S.Team, S.StickHitResult == 2 ? 1 : 0, S.StickHitResult == 2 ? TEXT("lift") : TEXT("poke"));
	}
	if (S.bHasPuck != bHadPuck)
	{
		bHadPuck = S.bHasPuck;
		IJamSkaterVisual::Execute_SetHasPuck(V, bHadPuck);
	}
	// Contract: turn rate positive = turning left seen from above; the sim's yaw grows to the right (Unreal yaw).
	// Lane A51 (R-0034): the spin is a yaw turn of the visual over the spin's frames (FJamSkaterState::SpinVisualYawDeg) through the existing facing call;
	// the sim's own yaw keeps the body's line. The stick aim below is measured against the turned facing so the blade stays where the puck is.
	const double SpinDeg = S.SpinVisualYawDeg();
	const double VisualYaw = S.Yaw + FMath::DegreesToRadians(SpinDeg);
	IJamSkaterVisual::Execute_UpdateMotion(V, static_cast<float>(S.Vel.Size()), static_cast<float>(-S.YawRate - (S.SpinFrames > 0 && S.SpinLength > 0 ? S.SpinDir * 2.0 * UE_DOUBLE_PI / (S.SpinLength * UJamSim::DT) : 0.0)),
		FRotator(0.0, FMath::RadiansToDegrees(VisualYaw), 0.0));

	// C-0001 v3 (B-to-A-0014): the stick verbs. Aim: the blade relative to the facing, positive = left (the sim's yaw grows to the right).
	// Lane A49: smoothed and side-stable before it is sent (SmoothAimDeg); the reach follows with the same 6-frame approach.
	{
		const FVector2D D = S.BladePos - S.Pos;
		const double Rel = FMath::UnwindRadians(FMath::Atan2(D.Y, D.X) - VisualYaw);   // lane A51: against the spun facing
		const float AimDeg = static_cast<float>(-FMath::RadiansToDegrees(Rel));
		const float Reach = static_cast<float>(D.Size());
		LastAimDeg = AimDeg;
		LastAimReach = Reach;
		const float Prev = SentAimDeg;
		SentAimDeg = SmoothAimDeg(AimDeg, SentAimDeg, AimSide, bAimInit);
		SentReachM = FMath::IsNearlyZero(SentReachM) ? Reach : SentReachM + (Reach - SentReachM) * 0.35f;
		if (bLogAim)
		{
			int64 Frame = -1;
			if (UGameInstance* GI = GetGameInstance()) if (const UJamEventBus* Bus = GI->GetSubsystem<UJamEventBus>()) Frame = Bus->GetSimFrame();
			UE_LOG(LogJamSim, Log, TEXT("JAMAIM frame=%lld raw=%.1f sent=%.1f d=%.1f side=%d reach=%.2f"), Frame, AimDeg, SentAimDeg, SentAimDeg - Prev, AimSide, SentReachM);
		}
		IJamSkaterVisual::Execute_SetStickAim(V, SentAimDeg, SentReachM);
	}
	{
		const int32 Max = 30;   // the visual gets 0..1; the sim's charge_frames_max is applied by LastShotCharge on release
		const float Charge = S.ChargeFrames > 0 ? FMath::Clamp(static_cast<float>(S.ChargeFrames) / static_cast<float>(Max), 0.f, 1.f) : 0.f;
		// Lane A65 (PROFILING.md offender 8): sent only when it changed (it was sent every frame, one Blueprint event per skater per frame for a value
		// that is 0 most of the match). SetupVisual sets LastCharge to -1 so a new or respawned visual gets the first value.
		if (Charge != LastCharge)
		{
			LastCharge = Charge;
			IJamSkaterVisual::Execute_SetShotCharge(V, Charge);
		}
	}
	if (S.PokeResult != 0) IJamSkaterVisual::Execute_PlayPoke(V, S.PokeResult > 0);
	if (S.bLiftThisFrame) IJamSkaterVisual::Execute_PlayLift(V);
	if (S.bPassThisFrame) IJamSkaterVisual::Execute_PlayPass(V, S.LastPassStrength);
	if (S.bAtFaceoff != bWasAtFaceoff)
	{
		bWasAtFaceoff = S.bAtFaceoff;
		IJamSkaterVisual::Execute_SetFaceoff(V, bWasAtFaceoff);
	}
	if (S.DrawResult != 0) IJamSkaterVisual::Execute_PlayFaceoffDraw(V, S.DrawResult > 0);
	if (S.bFoulThisFrame) IJamSkaterVisual::Execute_PlayFoulReaction(V);
}

float AJamSkater::SmoothAimDeg(float RawRelDeg, float Prev, int32& Side, bool& bInit)
{
	const float Raw = FMath::UnwindDegrees(RawRelDeg);
	// The side changes only through the front: within 90 degrees of the facing the raw sign rules (with a 2 degree band that keeps the last side, so a
	// blade held straight ahead does not flap); behind, the stick stays on the side it was on, at the clamp.
	if (FMath::Abs(Raw) <= 90.f)
	{
		if (FMath::Abs(Raw) > 2.f) Side = Raw > 0.f ? 1 : -1;
	}
	if (Side == 0) Side = 1;
	const float Target = FMath::Abs(Raw) <= 90.f ? FMath::Clamp(Raw, -AimClampDeg, AimClampDeg) : (float)Side * AimClampDeg;
	if (!bInit)
	{
		bInit = true;
		return Target;
	}
	// A 6-frame approach (35 % of the gap per frame: within 10 % after 6 frames), never more than AimSlewDegPerFrame in one frame.
	const float Gap = Target - Prev;
	const float Step = FMath::Clamp(Gap * 0.35f, -AimSlewDegPerFrame, AimSlewDegPerFrame);
	return FMath::Abs(Gap) < 0.05f ? Target : Prev + Step;
}

bool AJamSkater::GetPuckSocketWorld(FVector& Out) const
{
	if (!VisualObject) return false;
	TArray<USceneComponent*> Candidates;
	if (USceneComponent* SC = Cast<USceneComponent>(VisualObject))
	{
		Candidates.Add(SC);
		SC->GetChildrenComponents(true, Candidates);
	}
	else if (AActor* A = Cast<AActor>(VisualObject))
	{
		A->GetComponents<USceneComponent>(Candidates);
	}
	// A mesh socket first (B's figures), then a scene component named puck_socket (the toy figure).
	for (USceneComponent* C : Candidates)
	{
		if (C && C->DoesSocketExist(JamSkaterSockets::Puck)) { Out = C->GetSocketLocation(JamSkaterSockets::Puck); return true; }
	}
	for (USceneComponent* C : Candidates)
	{
		if (C && C->GetFName() == JamSkaterSockets::Puck) { Out = C->GetComponentLocation(); return true; }
	}
	return false;
}

void AJamSkater::ApplyRender(const FJamSkaterState& S, float Alpha)
{
	const FVector Loc = Lerp2D(S.PrevPos, S.Pos, Alpha, 0.0);
	const double DYaw = FMath::UnwindRadians(S.Yaw - S.PrevYaw);
	const double Yaw = S.PrevYaw + DYaw * Alpha + FMath::DegreesToRadians(S.SpinVisualYawDeg());   // lane A51 (R-0034): the spin's turn of the figure
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

	// R-0016 (B-to-A-0007): the puck kit. SM_PuckDisc under the puck, on the ice: no shadow, no dynamic indirect, stencil 0, no collision.
	// The puck mesh is a unit cylinder scaled to 2r x 2r x 0.08 m and centred 4 cm up, so parent-local z = -0.5 is the ice; the disc keeps its own scale.
	if (!Disc)
	{
		if (UStaticMesh* DiscMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/JamAssets/Props/SM_PuckDisc.SM_PuckDisc"), nullptr, LOAD_NoWarn | LOAD_Quiet))
		{
			Disc = NewObject<UStaticMeshComponent>(this, TEXT("PuckDisc"));
			Disc->SetStaticMesh(DiscMesh);
			Disc->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Disc->SetGenerateOverlapEvents(false);
			Disc->CastShadow = false;
			Disc->bAffectDynamicIndirectLighting = false;
			Disc->SetRenderCustomDepth(false);
			Disc->SetCustomDepthStencilValue(0);
			Disc->SetUsingAbsoluteScale(true);
			Disc->SetupAttachment(Mesh);
			Disc->SetRelativeLocation(FVector(0.0, 0.0, -0.45));
			Disc->SetWorldScale3D(FVector::OneVector);
			Disc->RegisterComponent();
		}
		else
		{
			UE_LOG(LogJamSim, Log, TEXT("JAMPUCK /JamAssets/Props/SM_PuckDisc not found; the puck has no disc"));
		}
	}
	if (!Trail)
	{
		if (UNiagaraSystem* Sys = LoadObject<UNiagaraSystem>(nullptr, TEXT("/JamAssets/VFX/NS_puck_trail.NS_puck_trail"), nullptr, LOAD_NoWarn | LOAD_Quiet))
		{
			Trail = UNiagaraFunctionLibrary::SpawnSystemAttached(Sys, Mesh, NAME_None, FVector(0.0, 0.0, -0.5), FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false);
			if (Trail) { Trail->SetUsingAbsoluteScale(true); Trail->SetWorldScale3D(FVector::OneVector); }
		}
		else
		{
			UE_LOG(LogJamSim, Log, TEXT("JAMPUCK /JamAssets/VFX/NS_puck_trail not found; the puck has no trail"));
		}
	}
}

double AJamPuck::RenderHeightM(const FJamPuckState& P, float Alpha, double HalfHeight)
{
	// R-0035 (lane A52): the sim's height, interpolated between the previous and the current frame like the position; never below the ice.
	const double A = FMath::Clamp(static_cast<double>(Alpha), 0.0, 1.0);
	return HalfHeight + FMath::Max(0.0, P.PrevZ + (P.Z - P.PrevZ) * A);
}

void AJamPuck::ApplyRender(const FJamPuckState& P, float Alpha)
{
	// R-0035 (lane A52): the puck is drawn where the sim says, height included; the disc (the shadow) stays on the ice under it.
	const FVector Loc = Lerp2D(P.PrevPos, P.Pos, Alpha, RenderHeightM(P, Alpha, HalfHeightM));
	SetActorLocation(Loc);
	PlaceDiscOnIce(Loc);
}

void AJamPuck::ApplyRenderAt(const FVector& WorldCm)
{
	const FVector Loc(WorldCm.X, WorldCm.Y, HalfHeightM * 100.0);
	SetActorLocation(Loc);
	PlaceDiscOnIce(Loc);
}

void AJamPuck::PlaceDiscOnIce(const FVector& PuckCm)
{
	// The disc is attached under the puck mesh (parent-local z -0.45 of a 0.08 m tall unit cylinder = 3.6 cm under the centre, 0.4 cm over the ice when the
	// puck sits on it); with the puck in the air it is placed in world space at the same 0.4 cm over the ice.
	if (Disc) Disc->SetWorldLocation(FVector(PuckCm.X, PuckCm.Y, (HalfHeightM - 0.45 * 0.08) * 100.0));
}
