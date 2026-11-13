#include "ToySkaterVisual.h"
#include "EffectsBinder.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	// Soft path of the one toy material (built by tools in Game/Content/Blueprints/Toy); the engine's basic shape material is the fallback.
	const TCHAR* ToyMaterialPath = TEXT("/Game/Blueprints/Toy/M_ToySkater.M_ToySkater");

	// Stick geometry (metres, in the stick pivot's frame): the stick leaves the right shoulder forward and down to the ice.
	const FVector StickDir = FVector(0.9f, 0.05f, -0.9f).GetSafeNormal();
	const float StickLenM = 1.27f;
}

UToySkaterVisualComponent::UToySkaterVisualComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FSoftObjectPath UToySkaterVisualComponent::BladeSprayPath()
{
	return FSoftObjectPath(TEXT("/JamAssets/VFX/NS_blade_spray.NS_blade_spray"));
}

// ---------------------------------------------------------------- pure pose functions

float UToySkaterVisualComponent::LeanTarget(float InSpeed, float InTurnRate)
{
	return FMath::Clamp(2.5f * InSpeed * InTurnRate, -MaxLeanDeg, MaxLeanDeg);
}

void UToySkaterVisualComponent::AdvanceState(FToyPoseState& S, float InSpeed, float InTurnRate)
{
	// Follow once per SIMULATION frame, so 144 Hz renders lean exactly like 60 Hz.
	const float Follow = 1.f - FMath::Exp(-12.f / 60.f);
	S.Lean += (LeanTarget(InSpeed, InTurnRate) - S.Lean) * Follow;
	S.Phase += InSpeed / 2.4f / 60.f;
}

float UToySkaterVisualComponent::ShotStickYaw(int32 F)
{
	static const int32 KeyF[] = { 0, 18, 25, 34, 40, 51 };
	static const float KeyV[] = { 0.f, -100.f, 10.f, 75.f, 75.f, 0.f };
	if (F < 0 || F >= 51)
	{
		return 0.f;
	}
	for (int32 I = 0; I < 5; ++I)
	{
		if (F <= KeyF[I + 1])
		{
			const float T = (float)(F - KeyF[I]) / (float)(KeyF[I + 1] - KeyF[I]);
			return FMath::Lerp(KeyV[I], KeyV[I + 1], FMath::SmoothStep(0.f, 1.f, T));
		}
	}
	return 0.f;
}

float UToySkaterVisualComponent::CheckCurve01(int32 F)
{
	if (F < 0 || F >= 20)
	{
		return 0.f;
	}
	// 8 frames in, 12 frames out.
	return F <= 8 ? FMath::SmoothStep(0.f, 1.f, (float)F / 8.f) : 1.f - FMath::SmoothStep(0.f, 1.f, (float)(F - 8) / 12.f);
}

float UToySkaterVisualComponent::KnockTipDeg(int32 F, bool bDownNow)
{
	if (F < 0)
	{
		return 0.f;
	}
	if (bDownNow)
	{
		if (F < 14)
		{
			const float T = (float)F / 14.f;
			return 88.f * T * T;
		}
		if (F < 17)
		{
			return FMath::Lerp(88.f, 80.f, (float)(F - 14) / 3.f);
		}
		if (F < 20)
		{
			return FMath::Lerp(80.f, 88.f, (float)(F - 17) / 3.f);
		}
		return 88.f;
	}
	if (F >= 20)
	{
		return 0.f;
	}
	const float T = (float)F / 20.f;
	const float EaseOut = 1.f - (1.f - T) * (1.f - T);
	return 88.f * (1.f - EaseOut);
}

FToyPose UToySkaterVisualComponent::ComputePose(const FToyPoseState& S, const FToyInputs& In)
{
	FToyPose P;
	const float Spd = FMath::Min(In.Speed / 8.f, 1.f);
	P.LeanRollDeg = -S.Lean;  // positive lean = into a left turn = left side down = negative roll in Unreal
	P.BodyPitchDeg = -(10.f + 6.f * Spd);
	P.BodyYawSwayDeg = 4.f * FMath::Sin(2.f * PI * S.Phase) * Spd;
	P.BobCm = 100.f * 0.04f * Spd * FMath::Abs(FMath::Sin(PI * S.Phase));

	const float Shot = ShotStickYaw(In.FramesSinceShoot, In.ShootCharge);
	// C-0001 v3: the aim turns the stick (left-positive aim = negative Unreal yaw) and the reach slides it forward; a held charge winds the blade back and up.
	P.StickYawDeg = Shot - In.AimDeg - 40.f * In.ChargeHold01;
	P.StickPitchDeg -= 15.f * In.ChargeHold01;
	P.StickForwardCm += FMath::Clamp((In.AimReachM - 1.2f) * 100.f, -40.f, 80.f);
	P.BodyYawSwayDeg += 0.3f * Shot;

	const float Chk = CheckCurve01(In.FramesSinceCheck);
	P.BodyPitchDeg -= 30.f * Chk;
	P.CheckShiftCm = 15.f * Chk;

	if (In.bHasPuck)
	{
		P.StickPitchDeg = -10.f;
		P.StickForwardCm = 10.f;
	}
	P.TipAngleDeg = KnockTipDeg(In.FramesSinceKnock, In.bDown);
	if (In.bBoost)
	{
		const float W = FMath::Sin(2.f * PI * (float)In.Frame / 30.f);
		P.Emissive = 1.f + 0.5f * W;
		P.Scale = 1.f + 0.04f * W;
	}
	return P;
}

// ---------------------------------------------------------------- figure construction

USceneComponent* UToySkaterVisualComponent::AddNode(const TCHAR* Name, USceneComponent* Parent, const FVector& LocM)
{
	AActor* Owner = GetOwner();
	USceneComponent* C = NewObject<USceneComponent>(Owner, MakeUniqueObjectName(Owner, USceneComponent::StaticClass(), Name), RF_Transient);
	C->CreationMethod = EComponentCreationMethod::Instance;
	C->SetupAttachment(Parent);
	C->SetRelativeLocation(LocM * 100.f);
	C->RegisterComponent();
	return C;
}

UStaticMeshComponent* UToySkaterVisualComponent::AddMesh(const TCHAR* Name, UStaticMesh* Shape, USceneComponent* Parent, const FVector& LocM, const FVector& SizeM, const FRotator& Rot)
{
	AActor* Owner = GetOwner();
	UStaticMeshComponent* M = NewObject<UStaticMeshComponent>(Owner, MakeUniqueObjectName(Owner, UStaticMeshComponent::StaticClass(), Name), RF_Transient);
	M->CreationMethod = EComponentCreationMethod::Instance;
	M->SetStaticMesh(Shape);
	M->SetupAttachment(Parent);
	M->SetRelativeLocation(LocM * 100.f);
	M->SetRelativeRotation(Rot);
	M->SetRelativeScale3D(SizeM);  // the basic shapes are 1 m, so the scale is the size in metres
	M->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	M->SetGenerateOverlapEvents(false);
	M->SetCastShadow(true);
	M->bReceivesDecals = false;
	M->RegisterComponent();
	return M;
}

void UToySkaterVisualComponent::BuildFigure()
{
	if (bBuilt || !GetOwner())
	{
		return;
	}
	bBuilt = true;

	UStaticMesh* Cyl = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* Sph = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UStaticMesh* Cub = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (!Cyl || !Sph || !Cub)
	{
		UE_LOG(LogTemp, Error, TEXT("ToySkaterVisual: engine basic shapes missing"));
		return;
	}

	YawNode = AddNode(TEXT("yaw_node"), this, FVector::ZeroVector);
	LeanNode = AddNode(TEXT("lean_node"), YawNode, FVector::ZeroVector);
	BodyPivot = AddNode(TEXT("body_pivot"), LeanNode, FVector::ZeroVector);

	// Body: a capsule made of a cylinder and a dome. 0.55 m wide, skates 0.12 m, shoulders at about 1.0 m.
	Body = AddMesh(TEXT("body"), Cyl, BodyPivot, FVector(0, 0, 0.57f), FVector(0.55f, 0.55f, 0.9f));
	BodyCap = AddMesh(TEXT("body_cap"), Sph, BodyPivot, FVector(0, 0, 1.02f), FVector(0.55f, 0.55f, 0.36f));
	Head = AddMesh(TEXT("head"), Sph, BodyPivot, FVector(0, 0, 1.2f), FVector(0.4f, 0.4f, 0.4f));
	HelmetBand = AddMesh(TEXT("helmet_band"), Cyl, BodyPivot, FVector(0, 0, 1.27f), FVector(0.43f, 0.43f, 0.1f));
	HeadSocket = AddNode(TEXT("head_socket"), BodyPivot, FVector(0, 0, 1.65f));

	// Stick: 6 cm cylinder from the right shoulder to a blade on the ice.
	StickPivot = AddNode(TEXT("stick_pivot"), BodyPivot, FVector(0.f, 0.3f, 0.9f));
	const FRotator StickRot = FRotationMatrix::MakeFromZ(StickDir).Rotator();
	Stick = AddMesh(TEXT("stick"), Cyl, StickPivot, StickDir * (StickLenM * 0.5f), FVector(0.06f, 0.06f, StickLenM), StickRot);
	// R-0021 feel.00: the blade lies flat on the ice at a hockey lie, turned 30 degrees out from the shaft, with a short heel where the shaft meets it,
	// and two gloved hands on the shaft (upper near the shoulder, lower at mid-shaft).
	const FVector BladeAt = StickDir * StickLenM + FVector(0.1f, 0.03f, 0.03f);
	Blade = AddMesh(TEXT("blade"), Cub, StickPivot, BladeAt, FVector(0.36f, 0.035f, 0.075f), FRotator(0.f, 30.f, 0.f));
	BladeHeel = AddMesh(TEXT("blade_heel"), Cub, StickPivot, StickDir * (StickLenM - 0.05f) + FVector(0.f, 0.f, 0.06f), FVector(0.07f, 0.05f, 0.14f), StickRot);
	HandUpper = AddMesh(TEXT("hand_upper"), Sph, StickPivot, StickDir * 0.16f, FVector(0.12f));
	HandLower = AddMesh(TEXT("hand_lower"), Sph, StickPivot, StickDir * 0.58f, FVector(0.12f));
	StickSocket = AddNode(TEXT("stick_socket"), StickPivot, BladeAt);

	SkateL = AddMesh(TEXT("skate_l"), Cub, LeanNode, FVector(0.f, -0.12f, 0.04f), FVector(0.3f, 0.08f, 0.08f));
	SkateR = AddMesh(TEXT("skate_r"), Cub, LeanNode, FVector(0.f, 0.12f, 0.04f), FVector(0.3f, 0.08f, 0.08f));
	PuckSocket = AddNode(TEXT("puck_socket"), YawNode, FVector(1.15f, 0.35f, 0.f));

	// Jersey number on the back: a text render, white and unlit; readable from behind only.
	{
		AActor* Owner = GetOwner();
		NumberText = NewObject<UTextRenderComponent>(Owner, MakeUniqueObjectName(Owner, UTextRenderComponent::StaticClass(), TEXT("number")), RF_Transient);
		NumberText->CreationMethod = EComponentCreationMethod::Instance;
		NumberText->SetupAttachment(BodyPivot);
		NumberText->SetRelativeLocation(FVector(-29.f, 0.f, 62.f));
		NumberText->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
		NumberText->SetHorizontalAlignment(EHTA_Center);
		NumberText->SetVerticalAlignment(EVRTA_TextCenter);
		NumberText->SetWorldSize(35.f);
		NumberText->SetTextRenderColor(FColor::White);
		NumberText->SetText(FText::AsNumber(Number));
		NumberText->SetCastShadow(false);
		NumberText->RegisterComponent();
	}

	// One dynamic material instance per coloured mesh, from B's toon materials by object path (R-0002 item 5); each falls back to
	// the toy material, then the engine basic shape material, never failing. All figure meshes write custom depth, stencil 1.
	auto Load = [](const TCHAR* Path) { return LoadObject<UMaterialInterface>(nullptr, Path, nullptr, LOAD_NoWarn | LOAD_Quiet); };
	UMaterialInterface* Fallback = Load(ToyMaterialPath);
	if (!Fallback)
	{
		Fallback = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	}
	UMaterialInterface* Toon = Load(TEXT("/JamAssets/Look/M_Toon.M_Toon"));
	UMaterialInterface* HelmetMi = Load(TEXT("/JamAssets/Look/MI_Toon_Helmet.MI_Toon_Helmet"));
	UMaterialInterface* StickMi = Load(TEXT("/JamAssets/Look/MI_Toon_Stick.MI_Toon_Stick"));
	if (!Toon)
	{
		UJamEffectsBinder::LoadOrLogOnce(FSoftObjectPath(TEXT("/JamAssets/Look/M_Toon.M_Toon")));
	}
	const TArray<UStaticMeshComponent*> Meshes = { Body.Get(), BodyCap.Get(), Head.Get(), HelmetBand.Get(), Stick.Get(), Blade.Get(), BladeHeel.Get(), HandUpper.Get(), HandLower.Get(), SkateL.Get(), SkateR.Get() };
	for (UStaticMeshComponent* M : Meshes)
	{
		UMaterialInterface* Base = Toon ? Toon : Fallback;
		if (M == HelmetBand.Get() && HelmetMi) { Base = HelmetMi; }
		if ((M == Stick.Get() || M == Blade.Get() || M == BladeHeel.Get()) && StickMi) { Base = StickMi; }
		UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Base, this);
		M->SetMaterial(0, Mid);
		M->SetRenderCustomDepth(true);
		M->SetCustomDepthStencilValue(1);
		Mids.Add(Mid);
		if (Toon)
		{
			Mid->SetScalarParameterValue(JamMaterialParams::Emissive, UToySkaterVisualComponent::BaseEmissive);
			Mid->SetScalarParameterValue(TEXT("RimStrength"), 1.3f);   // B-to-A-0023 point 4 (lane A31): 1.2..1.5 reads in the dark bands; 0.6 did not
		}
	}
	SetTeam_Implementation(0, TeamColour);
	SetNumber_Implementation(Number);
	ApplyPose(FToyPose(), FRotator::ZeroRotator);
}

void UToySkaterVisualComponent::OnRegister()
{
	Super::OnRegister();
	if (!IsTemplate() && GetOwner() && GetWorld() && GetWorld()->WorldType != EWorldType::EditorPreview)
	{
		BuildFigure();
	}
}

void UToySkaterVisualComponent::OnUnregister()
{
	Super::OnUnregister();
}

void UToySkaterVisualComponent::SetColourOn(UStaticMeshComponent* Mesh, const FLinearColor& Colour)
{
	if (!Mesh)
	{
		return;
	}
	if (UMaterialInstanceDynamic* Mid = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(0)))
	{
		Mid->SetVectorParameterValue(JamMaterialParams::TeamColour, Colour);
		Mid->SetVectorParameterValue(TEXT("Color"), Colour);  // the basic shape material's parameter
	}
}

// ---------------------------------------------------------------- IJamSkaterVisual

void UToySkaterVisualComponent::SetTeam_Implementation(int32 TeamId, FLinearColor Colour)
{
	TeamColour = Colour;
	if (!bBuilt)
	{
		return;
	}
	SetColourOn(Body, Colour);
	SetColourOn(BodyCap, Colour);
	SetColourOn(Head, FMath::Lerp(Colour, FLinearColor::White, 0.45f));  // a lighter head
	SetColourOn(HelmetBand, FLinearColor(0.02f, 0.02f, 0.025f));         // dark helmet band
	SetColourOn(Stick, FLinearColor(0.6f, 0.45f, 0.25f));
	SetColourOn(Blade, FLinearColor(0.05f, 0.05f, 0.05f));
	SetColourOn(BladeHeel, FLinearColor(0.05f, 0.05f, 0.05f));
	SetColourOn(HandUpper, FLinearColor(0.12f, 0.12f, 0.12f));   // gloves
	SetColourOn(HandLower, FLinearColor(0.12f, 0.12f, 0.12f));
	SetColourOn(SkateL, FLinearColor(0.08f, 0.08f, 0.09f));
	SetColourOn(SkateR, FLinearColor(0.08f, 0.08f, 0.09f));
}

void UToySkaterVisualComponent::SetNumber_Implementation(int32 N)
{
	Number = FMath::Clamp(N, 0, 99);
	if (!bBuilt)
	{
		return;
	}
	if (NumberText)
	{
		NumberText->SetText(FText::AsNumber(Number));
	}
	for (UMaterialInstanceDynamic* Mid : Mids)
	{
		Mid->SetScalarParameterValue(JamMaterialParams::NumberTens, (float)(Number / 10));
		Mid->SetScalarParameterValue(JamMaterialParams::NumberOnes, (float)(Number % 10));
	}
}

void UToySkaterVisualComponent::UpdateMotion_Implementation(float InSpeed, float InTurnRate, FRotator Facing)
{
	// One call per simulation frame: the local frame counter is the frame number every animation measures from.
	++LocalFrame;
	Speed = FMath::Max(0.f, InSpeed);
	TurnRate = InTurnRate;
	LastFacing = Facing;
	AdvanceState(State, Speed, TurnRate);

	FToyInputs In;
	In.Speed = Speed;
	In.TurnRate = TurnRate;
	In.bHasPuck = bHasPuck;
	In.bBoost = Boost != NAME_None;
	In.bDown = bDown;
	In.Frame = LocalFrame;
	In.FramesSinceShoot = ShootStart >= 0 ? LocalFrame - ShootStart : -1;
	In.ShootCharge = ShootCharge;
	In.ChargeHold01 = ChargeHold01;
	In.AimDeg = AimDeg;
	In.AimReachM = AimReachM;
	In.FramesSinceCheck = CheckStart >= 0 ? LocalFrame - CheckStart : -1;
	In.FramesSinceKnock = KnockStart >= 0 ? LocalFrame - KnockStart : -1;
	LastPose = ComputePose(State, In);
	if (bBuilt)
	{
		ApplyPose(LastPose, Facing);
	}

	// Blade spray hook on a hard turn (|lean| >= 25 deg): a request every 12 frames (B-to-A-0029 ask 3, lane A47: B's larger fan was too busy at 8), a Niagara system by soft path.
	if (FMath::Abs(State.Lean) >= 25.f && LocalFrame % 12 == 0 && !bDown)
	{
		++BladeSprayRequests;
		if (!bSprayTried)
		{
			bSprayTried = true;
			BladeSpray = Cast<UNiagaraSystem>(UJamEffectsBinder::LoadOrLogOnce(BladeSprayPath()));
		}
		if (BladeSpray && GetWorld() && bBuilt)
		{
			// B-to-A-0016 (lane A21): User.Strength is the lean, 0..1 of MaxLeanDeg (the sign is not sent: the spray goes both ways for now).
			if (UNiagaraComponent* Spray = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BladeSpray, GetComponentLocation() + FVector(0, 0, GroundOffsetCm), Facing))
			{
				Spray->SetVariableFloat(TEXT("User.Strength"), FMath::Clamp(FMath::Abs(State.Lean) / MaxLeanDeg, 0.f, 1.f));
			}
		}
	}
}

void UToySkaterVisualComponent::PlayShoot_Implementation(float Charge)
{
	ShootStart = LocalFrame;
	ShootCharge = FMath::Clamp(Charge, 0.f, 1.f);
}

void UToySkaterVisualComponent::PlayCheck_Implementation()
{
	CheckStart = LocalFrame;
}

void UToySkaterVisualComponent::SetKnockedDown_Implementation(bool bDownNow, float DirX, float DirZ)
{
	if (bDownNow == bDown)
	{
		return;
	}
	bDown = bDownNow;
	KnockStart = LocalFrame;
	if (bDownNow)
	{
		const FVector2D D(DirX, DirZ);
		KnockDir = D.IsNearlyZero() ? FVector2D(-1.f, 0.f) : D.GetSafeNormal();
	}
}

void UToySkaterVisualComponent::SetHasPuck_Implementation(bool bInHasPuck)
{
	bHasPuck = bInHasPuck;
}

void UToySkaterVisualComponent::SetStickAim_Implementation(float AngleDeg, float ReachM)
{
	AimDeg = FMath::Clamp(AngleDeg, -70.f, 70.f);   // B's figures pose -63..+70; beyond that the body turns, not the stick
	AimReachM = FMath::Clamp(ReachM, 0.6f, 2.2f);
}

void UToySkaterVisualComponent::SetShotCharge_Implementation(float Charge01)
{
	ChargeHold01 = FMath::Clamp(Charge01, 0.f, 1.f);
}

void UToySkaterVisualComponent::SetBoost_Implementation(FName Kind)
{
	Boost = Kind;
	if (bBuilt && Kind == NAME_None)
	{
		// R-0016: back to the painted-cel base (2.2, as at spawn), not to 1.0.
		for (UMaterialInstanceDynamic* Mid : Mids)
		{
			Mid->SetScalarParameterValue(JamMaterialParams::Emissive, UToySkaterVisualComponent::BaseEmissive);
		}
		if (BodyPivot)
		{
			BodyPivot->SetRelativeScale3D(FVector::OneVector);
		}
	}
}

// ---------------------------------------------------------------- application

void UToySkaterVisualComponent::ApplyPose(const FToyPose& P, const FRotator& Facing)
{
	if (!YawNode)
	{
		return;
	}
	YawNode->SetRelativeLocation(FVector(0, 0, GroundOffsetCm));
	YawNode->SetWorldRotation(FRotator(0.f, Facing.Yaw, 0.f));

	// Fall towards the knock direction (rink x = Unreal X, rink z = Unreal Y), expressed in the yaw node's frame.
	const FVector DirWorld(KnockDir.X, KnockDir.Y, 0.f);
	const FVector DirLocal = FRotator(0.f, Facing.Yaw, 0.f).UnrotateVector(DirWorld);
	const FVector Axis = FVector::CrossProduct(FVector::UpVector, DirLocal).GetSafeNormal();
	const FQuat Tip = Axis.IsNearlyZero() ? FQuat::Identity : FQuat(Axis, FMath::DegreesToRadians(P.TipAngleDeg));
	const FQuat Roll = FRotator(0.f, 0.f, P.LeanRollDeg).Quaternion();
	LeanNode->SetRelativeRotation(Tip * Roll);
	LeanNode->SetRelativeLocation(FVector(P.CheckShiftCm, 0.f, 0.f));

	BodyPivot->SetRelativeRotation(FRotator(P.BodyPitchDeg, P.BodyYawSwayDeg, 0.f));
	BodyPivot->SetRelativeLocation(FVector(0.f, 0.f, P.BobCm));
	BodyPivot->SetRelativeScale3D(FVector(P.Scale));

	StickPivot->SetRelativeRotation(FRotator(P.StickPitchDeg, P.StickYawDeg, 0.f));
	StickPivot->SetRelativeLocation(FVector(P.StickForwardCm, 30.f, 90.f));

	if (Boost != NAME_None)
	{
		for (UMaterialInstanceDynamic* Mid : Mids)
		{
			Mid->SetScalarParameterValue(JamMaterialParams::Emissive, UToySkaterVisualComponent::BaseEmissive * P.Emissive);   // the pose's value is a multiplier
		}
	}
}

USceneComponent* UToySkaterVisualComponent::GetSocketComponent(FName Socket) const
{
	if (Socket == JamSkaterSockets::Stick) { return StickSocket; }
	if (Socket == JamSkaterSockets::Puck) { return PuckSocket; }
	if (Socket == JamSkaterSockets::Head) { return HeadSocket; }
	return nullptr;
}

FTransform UToySkaterVisualComponent::GetSocketWorldTransform(FName Socket) const
{
	const USceneComponent* C = GetSocketComponent(Socket);
	return C ? C->GetComponentTransform() : GetComponentTransform();
}
