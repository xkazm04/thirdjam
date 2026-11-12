#include "SkeletalSkaterVisual.h"
#include "EffectsBinder.h"
#include "ToySkaterVisual.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "BoneContainer.h"
#include "Components/TextRenderComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

namespace SkeletalSkater
{
	// Asset names of the sequences (lane A22, docs/experiments/BLENDER.md section 4), in ESkaterSeq order.
	const TCHAR* SeqNames[] = { TEXT("idle"), TEXT("skate"), TEXT("carry"), TEXT("aim_left"), TEXT("aim_right"), TEXT("poke"), TEXT("lift"),
		TEXT("shoot_windup"), TEXT("shoot_release"), TEXT("check"), TEXT("knocked_down"), TEXT("get_up"), TEXT("faceoff_ready"), TEXT("draw"),
		TEXT("goalie_left"), TEXT("goalie_centre"), TEXT("goalie_right"), TEXT("skate_left"), TEXT("skate_right") };
	// Authored lengths in frames at 60 fps (BLENDER.md section 2).
	const int32 SeqLen[] = { 60, 36, 60, 30, 30, 18, 20, 24, 51, 20, 40, 20, 30, 15, 30, 30, 30, 36, 36 };
	static_assert(UE_ARRAY_COUNT(SeqNames) == (int32)ESkaterSeq::Count, "sequence table");
	static_assert(UE_ARRAY_COUNT(SeqLen) == (int32)ESkaterSeq::Count, "sequence table");

	const TCHAR* FigurePath = TEXT("/Game/Figures/SK_SkaterToy.SK_SkaterToy");
	const TCHAR* ToyMaterialPath = TEXT("/Game/Blueprints/Toy/M_ToySkater.M_ToySkater");

	// A mistimed poke stumbles with the first frames of get_up right after the thrust (BLENDER.md section 5).
	constexpr int32 PokeStumbleFrames = 8;
	// A pass is the follow-through of shoot_release (from frame 25) at rate 1.5.
	constexpr float PassFromFrame = 25.f;
	constexpr float PassRate = 1.5f;
	constexpr int32 PassFrames = 18;
	// The foul reaction reuses check at rate 0.6.
	constexpr float FoulRate = 0.6f;
	constexpr int32 FoulFrames = 33;
	// The whole figure yaws so its back faces the knock direction over the first frames of the fall, and back over get_up.
	constexpr int32 KnockYawInFrames = 6;
	// The contract's aim is left-positive; aim_left was authored as the Unreal yaw -70 pose (the blade on the skater's left).
	constexpr bool bAimLeftIsPositive = true;
	constexpr bool bUseDigitsMaterial = false;

	float SmoothStep01(float T)
	{
		T = FMath::Clamp(T, 0.f, 1.f);
		return T * T * (3.f - 2.f * T);
	}
}

USkeletalSkaterVisual::USkeletalSkaterVisual()
{
	// The pose is written by UpdateMotion (one call per sim frame) and refreshed there; nothing happens on the render tick.
	PrimaryComponentTick.bCanEverTick = false;
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	CastShadow = true;
	bReceivesDecals = false;
}

// ---------------------------------------------------------------- tables and pure functions

const TCHAR* USkeletalSkaterVisual::SeqName(ESkaterSeq Seq)
{
	return SkeletalSkater::SeqNames[FMath::Clamp((int32)Seq, 0, (int32)ESkaterSeq::Count - 1)];
}

int32 USkeletalSkaterVisual::SeqFrames(ESkaterSeq Seq)
{
	return SkeletalSkater::SeqLen[FMath::Clamp((int32)Seq, 0, (int32)ESkaterSeq::Count - 1)];
}

FSoftObjectPath USkeletalSkaterVisual::FigureMeshPath()
{
	return FSoftObjectPath(SkeletalSkater::FigurePath);
}

FSoftObjectPath USkeletalSkaterVisual::SequencePath(ESkaterSeq Seq)
{
	const FString Name = FString::Printf(TEXT("AS_SkaterToy_%s"), SeqName(Seq));
	return FSoftObjectPath(FString::Printf(TEXT("/Game/Figures/%s.%s"), *Name, *Name));
}

FSoftObjectPath USkeletalSkaterVisual::BladeSprayPath()
{
	return UToySkaterVisualComponent::BladeSprayPath();
}

float USkeletalSkaterVisual::LeanTarget(float InSpeed, float InTurnRate)
{
	return UToySkaterVisualComponent::LeanTarget(InSpeed, InTurnRate);
}

// Which sequence, at which frame, this sim frame. Highest priority first; every one-shot is measured in frames since its trigger,
// every loop in the visual's frame counter or the distance phase, so the same call history gives the same frame at any render rate.
FSkelLayer USkeletalSkaterVisual::ChooseLayer(const FSkelInputs& In)
{
	using namespace SkeletalSkater;
	FSkelLayer L;
	auto Within = [](int32 Since, int32 Len) { return Since >= 0 && Since < Len; };
	auto Loop = [](int32 Frame, ESkaterSeq Seq) { const int32 N = SeqFrames(Seq); return (float)(((Frame % N) + N) % N); };

	if (In.bDown)
	{
		L.Seq = ESkaterSeq::KnockedDown;
		L.Frame = (float)FMath::Clamp(In.SinceKnock < 0 ? SeqFrames(ESkaterSeq::KnockedDown) - 1 : In.SinceKnock, 0, SeqFrames(ESkaterSeq::KnockedDown) - 1);   // hold the last frame while down
		L.StickAimWeight = 0.f;
		return L;
	}
	if (Within(In.SinceKnock, SeqFrames(ESkaterSeq::GetUp)))
	{
		L.Seq = ESkaterSeq::GetUp; L.Frame = (float)In.SinceKnock; L.StickAimWeight = 0.f; return L;
	}
	if (Within(In.SinceCheck, SeqFrames(ESkaterSeq::Check)))
	{
		L.Seq = ESkaterSeq::Check; L.Frame = (float)In.SinceCheck; L.StickAimWeight = 0.f; return L;
	}
	if (Within(In.SinceFoul, FoulFrames))
	{
		L.Seq = ESkaterSeq::Check; L.Frame = (float)In.SinceFoul * FoulRate; L.StickAimWeight = 0.f; return L;
	}
	if (Within(In.SinceShoot, SeqFrames(ESkaterSeq::ShootRelease)))
	{
		L.Seq = ESkaterSeq::ShootRelease; L.Frame = (float)In.SinceShoot; L.StickAimWeight = 0.f; return L;
	}
	if (Within(In.SincePass, PassFrames))
	{
		L.Seq = ESkaterSeq::ShootRelease; L.Frame = FMath::Min(PassFromFrame + (float)In.SincePass * PassRate, (float)SeqFrames(ESkaterSeq::ShootRelease) - 1.f); L.StickAimWeight = 0.f; return L;
	}
	if (Within(In.SincePoke, SeqFrames(ESkaterSeq::Poke)))
	{
		L.Seq = ESkaterSeq::Poke; L.Frame = (float)In.SincePoke; L.StickAimWeight = 0.f; return L;
	}
	if (!In.bPokeClean && Within(In.SincePoke - SeqFrames(ESkaterSeq::Poke), PokeStumbleFrames))
	{
		L.Seq = ESkaterSeq::GetUp; L.Frame = (float)(In.SincePoke - SeqFrames(ESkaterSeq::Poke)); L.StickAimWeight = 0.f; return L;
	}
	if (Within(In.SinceLift, SeqFrames(ESkaterSeq::Lift)))
	{
		L.Seq = ESkaterSeq::Lift; L.Frame = (float)In.SinceLift; L.StickAimWeight = 0.f; return L;
	}
	if (Within(In.SinceDraw, SeqFrames(ESkaterSeq::Draw)))
	{
		L.Seq = ESkaterSeq::Draw; L.Frame = (float)In.SinceDraw; L.StickAimWeight = 0.f; return L;
	}
	if (In.bGoalie)
	{
		L.Seq = In.GoalieSide < 0 ? ESkaterSeq::GoalieLeft : In.GoalieSide > 0 ? ESkaterSeq::GoalieRight : ESkaterSeq::GoalieCentre;
		L.Frame = Loop(In.Frame, L.Seq); L.StickAimWeight = 0.f; return L;
	}
	if (In.bFaceoff)
	{
		L.Seq = ESkaterSeq::FaceoffReady; L.Frame = Loop(In.Frame, L.Seq); L.StickAimWeight = 0.f; return L;
	}
	if (In.ChargeHold01 > 0.f)
	{
		// The wind-up scrubbed by the charge and held there; the aim still turns the blade.
		L.Seq = ESkaterSeq::ShootWindup; L.Frame = FMath::Clamp(In.ChargeHold01, 0.f, 1.f) * (float)(SeqFrames(ESkaterSeq::ShootWindup) - 1); L.StickAimWeight = 0.5f; return L;
	}
	if (In.bHasPuck)
	{
		L.Seq = ESkaterSeq::Carry; L.Frame = Loop(In.Frame, L.Seq); L.StickAimWeight = 1.f; return L;
	}
	if (In.Speed >= 0.3f)
	{
		// One stride cycle per 2.4 m: the phase is distance travelled, as the code figure's bob.
		const float Cycle = In.Phase - FMath::FloorToFloat(In.Phase);
		L.Seq = ESkaterSeq::Skate; L.Frame = Cycle * (float)SeqFrames(ESkaterSeq::Skate); L.StickAimWeight = 1.f; return L;
	}
	L.Seq = ESkaterSeq::Idle; L.Frame = Loop(In.Frame, L.Seq); L.StickAimWeight = 1.f;
	return L;
}

// ---------------------------------------------------------------- construction

void USkeletalSkaterVisual::OnRegister()
{
	Super::OnRegister();
	if (!IsTemplate() && GetOwner() && GetWorld() && GetWorld()->WorldType != EWorldType::EditorPreview)
	{
		BuildFigure();
	}
}

void USkeletalSkaterVisual::BuildFigure()
{
	if (bBuildTried || !GetOwner())
	{
		return;
	}
	bBuildTried = true;

	USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, SkeletalSkater::FigurePath, nullptr, LOAD_NoWarn | LOAD_Quiet);
	if (!Mesh || !Mesh->GetSkeleton())
	{
		UE_LOG(LogTemp, Error, TEXT("SkeletalSkaterVisual: %s missing (lane A22's import); the figure stays empty"), SkeletalSkater::FigurePath);
		return;
	}
	SetSkinnedAssetAndUpdate(Mesh, true);
	USkeleton* Skeleton = Mesh->GetSkeleton();

	Sequences.SetNum((int32)ESkaterSeq::Count);
	int32 Loaded = 0;
	for (int32 I = 0; I < (int32)ESkaterSeq::Count; ++I)
	{
		Sequences[I] = LoadObject<UAnimSequence>(nullptr, *SequencePath((ESkaterSeq)I).ToString(), nullptr, LOAD_NoWarn | LOAD_Quiet);
		if (Sequences[I]) ++Loaded;
		else UE_LOG(LogTemp, Warning, TEXT("SkeletalSkaterVisual: sequence %s missing; the reference pose stands in"), *SequencePath((ESkaterSeq)I).ToString());
	}

	const FReferenceSkeleton& Ref = Mesh->GetRefSkeleton();
	const int32 NumBones = Ref.GetNum();
	SkeletonBoneOfMeshBone.SetNum(NumBones);
	for (int32 B = 0; B < NumBones; ++B)
	{
		SkeletonBoneOfMeshBone[B] = Skeleton->GetSkeletonBoneIndexFromMeshBoneIndex(Mesh, B);
	}
	StickRootBone = Ref.FindBoneIndex(TEXT("stick_root"));
	ArmLBone = Ref.FindBoneIndex(TEXT("arm_l"));
	ArmRBone = Ref.FindBoneIndex(TEXT("arm_r"));
	TorsoBone = Ref.FindBoneIndex(TEXT("torso"));

	// Materials: one dynamic instance per slot. B's M_Toon lacks the SkeletalMesh usage flag (a skinned mesh then draws the default grid
	// material), so Game/Content/Figures holds a flagged copy of the master (M_ToonFigure) and of the helmet and stick instances, reparented
	// to it (same parameter names, R-0002 item 5); B's own assets are the fallback, then the toy material, then the engine's basic shape
	// material. Slot order from the import: base, number, head, trim, stick (BLENDER.md section 4).
	auto Load = [](const TCHAR* Path) { return LoadObject<UMaterialInterface>(nullptr, Path, nullptr, LOAD_NoWarn | LOAD_Quiet); };
	UMaterialInterface* Fallback = Load(SkeletalSkater::ToyMaterialPath);
	if (!Fallback) Fallback = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	UMaterialInterface* Toon = Load(TEXT("/Game/Figures/M_ToonFigure.M_ToonFigure"));
	if (!Toon) Toon = Load(TEXT("/JamAssets/Look/M_Toon.M_Toon"));
	UMaterialInterface* HelmetMi = Load(TEXT("/Game/Figures/MI_SkaterFigure_Trim.MI_SkaterFigure_Trim"));
	if (!HelmetMi) HelmetMi = Load(TEXT("/JamAssets/Look/MI_Toon_Helmet.MI_Toon_Helmet"));
	UMaterialInterface* StickMi = Load(TEXT("/Game/Figures/MI_SkaterFigure_Stick.MI_SkaterFigure_Stick"));
	if (!StickMi) StickMi = Load(TEXT("/JamAssets/Look/MI_Toon_Stick.MI_Toon_Stick"));
	if (!Toon) UJamEffectsBinder::LoadOrLogOnce(FSoftObjectPath(TEXT("/JamAssets/Look/M_Toon.M_Toon")));
	// The digits material for the 0..1 patch (M_SkaterNumber: a seven-segment mask from NumberTens / NumberOnes in one Custom HLSL node) renders the whole
	// patch as ink in the first PIE (skel3-10m.jpg); off until the patch UVs are checked (A26 next), the patch stays white and the text render carries the number.
	UMaterialInterface* NumberMat = SkeletalSkater::bUseDigitsMaterial ? Load(TEXT("/Game/Figures/M_SkaterNumber.M_SkaterNumber")) : nullptr;
	const int32 NumSlots = GetNumMaterials();
	Mids.SetNum(NumSlots);
	SlotKinds.SetNum(NumSlots);
	const TArray<FSkeletalMaterial>& SlotTable = Mesh->GetMaterials();
	for (int32 S = 0; S < NumSlots; ++S)
	{
		// By slot name (A22 pass 3: base, number, head, trim, steel, stick); an unnamed slot falls back to the import order.
		const FString SlotName = SlotTable.IsValidIndex(S) ? SlotTable[S].MaterialSlotName.ToString().ToLower() : FString();
		ESlotKind Kind = ESlotKind::Base;
		if (SlotName.Contains(TEXT("number"))) Kind = ESlotKind::Number;
		else if (SlotName.Contains(TEXT("head"))) Kind = ESlotKind::Head;
		else if (SlotName.Contains(TEXT("trim"))) Kind = ESlotKind::Trim;
		else if (SlotName.Contains(TEXT("steel"))) Kind = ESlotKind::Steel;
		else if (SlotName.Contains(TEXT("stick"))) Kind = ESlotKind::Stick;
		else if (!SlotName.Contains(TEXT("base"))) Kind = (ESlotKind)FMath::Clamp(S, 0, (int32)ESlotKind::Stick);
		SlotKinds[S] = Kind;
		UMaterialInterface* Base = Toon ? Toon : Fallback;
		if (Kind == ESlotKind::Trim && HelmetMi) Base = HelmetMi;    // helmet, band, mittens, skates, legs
		if (Kind == ESlotKind::Stick && StickMi) Base = StickMi;     // shaft, heel, blade wood
		if (Kind == ESlotKind::Number && NumberMat) Base = NumberMat;
		UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Base, this);
		SetMaterial(S, Mid);
		Mids[S] = Mid;
		if (Toon)
		{
			Mid->SetScalarParameterValue(JamMaterialParams::Emissive, BaseEmissive);
			Mid->SetScalarParameterValue(TEXT("RimStrength"), 0.6f);
		}
	}
	SetRenderCustomDepth(true);
	SetCustomDepthStencilValue(1);   // the painted-cel outline, as every figure mesh of the toy

	// Jersey number: a white text render behind the back, between the shoulders, read from behind. Attached to the component root, not to
	// the torso bone: the imported hierarchy carries a mirroring scale (the FBX axis flip), which mirrored the glyphs when parented to a bone.
	{
		AActor* Owner = GetOwner();
		NumberText = NewObject<UTextRenderComponent>(Owner, MakeUniqueObjectName(Owner, UTextRenderComponent::StaticClass(), TEXT("number")), RF_Transient);
		NumberText->CreationMethod = EComponentCreationMethod::Instance;
		NumberText->SetupAttachment(this);
		NumberText->SetRelativeTransform(FTransform(FRotator(0.f, 180.f, 0.f), FVector(-34.f, 0.f, 112.f)));
		NumberText->SetHorizontalAlignment(EHTA_Center);
		NumberText->SetVerticalAlignment(EVRTA_TextCenter);
		NumberText->SetWorldSize(30.f);
		NumberText->SetTextRenderColor(FColor::White);
		NumberText->SetText(FText::AsNumber(Number));
		NumberText->SetCastShadow(false);
		NumberText->RegisterComponent();
	}

	bReady = true;
	UE_LOG(LogTemp, Log, TEXT("SkeletalSkaterVisual: %s with %d bones, %d/%d sequences, %d material slots"), *Mesh->GetName(), NumBones, Loaded, (int32)ESkaterSeq::Count, NumSlots);
	SetTeam_Implementation(0, TeamColour);
	SetNumber_Implementation(Number);
	ApplyPose(FRotator::ZeroRotator);
}

// ---------------------------------------------------------------- sampling

void USkeletalSkaterVisual::SampleLocal(ESkaterSeq Seq, float Frame, int32 MeshBone, FTransform& Out) const
{
	// Start from the reference pose: GetBoneTransform leaves the atom untouched for a bone the sequence has no track for.
	const USkinnedAsset* Asset = GetSkinnedAsset();
	if (!Asset || !Asset->GetRefSkeleton().IsValidIndex(MeshBone))
	{
		Out = FTransform::Identity;
		return;
	}
	Out = Asset->GetRefSkeleton().GetRefBonePose()[MeshBone];
	const UAnimSequence* S = Sequences.IsValidIndex((int32)Seq) ? Sequences[(int32)Seq].Get() : nullptr;
	const int32 SkelBone = SkeletonBoneOfMeshBone.IsValidIndex(MeshBone) ? SkeletonBoneOfMeshBone[MeshBone] : INDEX_NONE;
	if (!S || SkelBone == INDEX_NONE)
	{
		return;
	}
	// Frames at 60 fps (the import's sample rate), clamped to the asset's own length.
	const double Time = FMath::Clamp((double)Frame / 60.0, 0.0, (double)S->GetPlayLength());
	FAnimExtractContext Ctx(Time, false);
	S->GetBoneTransform(Out, FSkeletonPoseBoneIndex(SkelBone), Ctx, false);
}

FSkelInputs USkeletalSkaterVisual::MakeInputs() const
{
	FSkelInputs In;
	In.Frame = LocalFrame;
	In.Phase = Phase;
	In.Speed = Speed;
	In.bHasPuck = bHasPuck;
	In.bDown = bDown;
	In.bFaceoff = bFaceoff;
	In.bGoalie = bGoalie;
	In.GoalieSide = GoalieSide;
	In.ChargeHold01 = ChargeHold01;
	auto Since = [this](int32 Start) { return Start >= 0 ? LocalFrame - Start : -1; };
	In.SinceKnock = Since(KnockStart);
	In.SinceCheck = Since(CheckStart);
	In.SinceFoul = Since(FoulStart);
	In.SinceShoot = Since(ShootStart);
	In.SincePass = Since(PassStart);
	In.SincePoke = Since(PokeStart);
	In.bPokeClean = bPokeClean;
	In.SinceLift = Since(LiftStart);
	In.SinceDraw = Since(DrawStart);
	return In;
}

void USkeletalSkaterVisual::ApplyPose(const FRotator& Facing)
{
	if (!bReady || !GetSkinnedAsset())
	{
		return;
	}
	const double T0 = FPlatformTime::Seconds();
	const FSkelInputs In = MakeInputs();
	LastLayer = ChooseLayer(In);

	const FReferenceSkeleton& Ref = GetSkinnedAsset()->GetRefSkeleton();
	const int32 NumBones = BoneSpaceTransforms.Num();
	for (int32 B = 0; B < NumBones; ++B)
	{
		SampleLocal(LastLayer.Seq, LastLayer.Frame, B, BoneSpaceTransforms[B]);
	}
	// The lean into a turn is authored (A22 pass 3): skate_left / skate_right carry 15 degrees; blend them in by the lean state, which follows the sim's turn rate.
	if (LastLayer.Seq == ESkaterSeq::Skate && FMath::Abs(Lean) > 0.5f)
	{
		const ESkaterSeq Side = Lean > 0.f ? ESkaterSeq::SkateLeft : ESkaterSeq::SkateRight;
		const float W = FMath::Clamp(FMath::Abs(Lean) / MaxLeanDeg, 0.f, 1.f);
		for (int32 B = 0; B < NumBones; ++B)
		{
			FTransform Sd;
			SampleLocal(Side, LastLayer.Frame, B, Sd);
			BoneSpaceTransforms[B].BlendWith(Sd, W);
		}
	}

	// C-0001 v3 stick aim: a per-bone blend of stick_root and the two stretch arms toward the held aim pose (never a whole-action blend,
	// BLENDER.md risk 2), weighted by the angle and by how much the current layer leaves the stick to the aim.
	float AimT = 0.f;
	if (LastLayer.StickAimWeight > 0.f && FMath::Abs(AimDeg) > 0.5f)
	{
		const bool bLeft = SkeletalSkater::bAimLeftIsPositive ? AimDeg > 0.f : AimDeg < 0.f;
		const ESkaterSeq AimSeq = bLeft ? ESkaterSeq::AimLeft : ESkaterSeq::AimRight;
		AimT = FMath::Clamp(FMath::Abs(AimDeg) / MaxAimDeg, 0.f, 1.f) * LastLayer.StickAimWeight;
		const float HoldFrame = (float)(SeqFrames(AimSeq) - 1);
		for (int32 B : { StickRootBone, ArmLBone, ArmRBone })
		{
			if (B == INDEX_NONE || B >= NumBones) continue;
			FTransform Aim;
			SampleLocal(AimSeq, HoldFrame, B, Aim);
			BoneSpaceTransforms[B].BlendWith(Aim, AimT);
		}
	}
	// The reach slides the stick root forward along the rink (the aim poses already carry 30 cm of it), in the parent bone's frame.
	if (StickRootBone != INDEX_NONE && StickRootBone < NumBones && LastLayer.StickAimWeight > 0.f)
	{
		const float ExtraCm = FMath::Clamp((AimReachM - 1.2f) * 100.f - 30.f * AimT, -30.f, 60.f) * LastLayer.StickAimWeight;
		if (FMath::Abs(ExtraCm) > 0.5f)
		{
			FTransform ParentCS = FTransform::Identity;
			for (int32 B = Ref.GetParentIndex(StickRootBone); B != INDEX_NONE; B = Ref.GetParentIndex(B)) ParentCS = ParentCS * BoneSpaceTransforms[B];
			BoneSpaceTransforms[StickRootBone].AddToTranslation(ParentCS.InverseTransformVectorNoScale(FVector(ExtraCm, 0.f, 0.f)));
		}
	}
	MarkRefreshTransformDirty();
	RefreshBoneTransforms();

	// The whole figure: facing yaw, a yaw toward the knock direction while down (the sequence falls backward, so the back faces it),
	// the lean into turns as a roll about the skates, the boost scale pulse.
	float YawOff = 0.f;
	if (KnockStart >= 0 && (bDown || In.SinceKnock < SeqFrames(ESkaterSeq::GetUp)))
	{
		const float DirYaw = FMath::RadiansToDegrees(FMath::Atan2(KnockDir.Y, KnockDir.X)) + 180.f;
		const float Off = FRotator::NormalizeAxis(DirYaw - Facing.Yaw);
		const float K = bDown ? SkeletalSkater::SmoothStep01((float)In.SinceKnock / (float)SkeletalSkater::KnockYawInFrames)
			: 1.f - SkeletalSkater::SmoothStep01((float)In.SinceKnock / (float)SeqFrames(ESkaterSeq::GetUp));
		YawOff = Off * K;
	}
	const FQuat Yaw = FRotator(0.f, Facing.Yaw + YawOff, 0.f).Quaternion();
	const FQuat Roll = FRotator(0.f, 0.f, -FMath::Clamp(Lean, -MaxRollDeg, MaxRollDeg)).Quaternion();   // positive lean = left side down = negative Unreal roll; capped, the rest is in the sequences
	SetWorldRotation(Yaw * Roll);

	if (Boost != NAME_None)
	{
		const float W = FMath::Sin(2.f * PI * (float)LocalFrame / 30.f);
		SetEmissive(1.f + 0.5f * W);
		SetRelativeScale3D(FVector(1.f + 0.04f * W));
	}

	CostAccumS += FPlatformTime::Seconds() - T0;
	++CostCalls;
	if (LocalFrame % 600 == 0 && CostCalls > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("SKELVIS %s frame=%d apply_us mean=%.1f (sampling + refresh of one figure per sim frame)"), *GetOwner()->GetName(), LocalFrame, 1e6 * CostAccumS / CostCalls);
		CostAccumS = 0.0;
		CostCalls = 0;
	}
}

void USkeletalSkaterVisual::SetEmissive(float Multiplier)
{
	for (UMaterialInstanceDynamic* Mid : Mids)
	{
		if (Mid) Mid->SetScalarParameterValue(JamMaterialParams::Emissive, BaseEmissive * Multiplier);
	}
}

// ---------------------------------------------------------------- IJamSkaterVisual

void USkeletalSkaterVisual::SetTeam_Implementation(int32 TeamId, FLinearColor Colour)
{
	TeamColour = Colour;
	if (!bReady)
	{
		return;
	}
	auto Paint = [this](int32 Slot, const FLinearColor& C)
	{
		if (!Mids.IsValidIndex(Slot) || !Mids[Slot]) return;
		Mids[Slot]->SetVectorParameterValue(JamMaterialParams::TeamColour, C);
		Mids[Slot]->SetVectorParameterValue(TEXT("Color"), C);   // the basic shape material's parameter
	};
	for (int32 S = 0; S < SlotKinds.Num(); ++S)
	{
		switch (SlotKinds[S])
		{
		case ESlotKind::Base:   Paint(S, Colour); break;                                             // jersey, arms, shoulders
		case ESlotKind::Number: Paint(S, FLinearColor(0.95f, 0.95f, 0.95f)); break;                  // the patch behind the digits
		case ESlotKind::Head:   Paint(S, FMath::Lerp(Colour, FLinearColor::White, 0.45f)); break;   // a lighter head, as the toy
		case ESlotKind::Trim:   Paint(S, FLinearColor(0.04f, 0.04f, 0.045f)); break;                 // helmet, band, mittens, skates, legs
		case ESlotKind::Steel:  Paint(S, FLinearColor(0.85f, 0.87f, 0.9f)); break;                   // blade runners
		case ESlotKind::Stick:  Paint(S, FLinearColor(0.6f, 0.45f, 0.25f)); break;                   // stick wood
		}
	}
}

void USkeletalSkaterVisual::SetNumber_Implementation(int32 N)
{
	Number = FMath::Clamp(N, 0, 99);
	if (!bReady)
	{
		return;
	}
	if (NumberText) NumberText->SetText(FText::AsNumber(Number));
	for (UMaterialInstanceDynamic* Mid : Mids)
	{
		if (!Mid) continue;
		Mid->SetScalarParameterValue(JamMaterialParams::NumberTens, (float)(Number / 10));
		Mid->SetScalarParameterValue(JamMaterialParams::NumberOnes, (float)(Number % 10));
	}
}

void USkeletalSkaterVisual::UpdateMotion_Implementation(float InSpeed, float InTurnRate, FRotator Facing)
{
	// One call per simulation frame: the frame counter every animation measures from, the lean follow and the stride phase.
	++LocalFrame;
	Speed = FMath::Max(0.f, InSpeed);
	TurnRate = InTurnRate;
	const float Follow = 1.f - FMath::Exp(-12.f / 60.f);
	Lean += (LeanTarget(Speed, TurnRate) - Lean) * Follow;
	Phase += Speed / 2.4f / 60.f;
	if (Phase > 1024.f) Phase -= 1024.f;   // keep the float exact over a long match
	ApplyPose(Facing);

	// Blade spray hook on a hard turn (|lean| >= 25 deg): a request every 8 frames, device B's placeholder Niagara system by soft path.
	if (FMath::Abs(Lean) >= 25.f && LocalFrame % 8 == 0 && !bDown)
	{
		++BladeSprayRequests;
		if (!bSprayTried)
		{
			bSprayTried = true;
			BladeSpray = Cast<UNiagaraSystem>(UJamEffectsBinder::LoadOrLogOnce(BladeSprayPath()));
		}
		if (BladeSpray && GetWorld() && bReady)
		{
			if (UNiagaraComponent* Spray = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BladeSpray, GetComponentLocation(), Facing))
			{
				Spray->SetVariableFloat(TEXT("User.Strength"), FMath::Clamp(FMath::Abs(Lean) / MaxLeanDeg, 0.f, 1.f));
			}
		}
	}
}

void USkeletalSkaterVisual::PlayShoot_Implementation(float Charge)
{
	ShootStart = LocalFrame;
	PassStart = -1;
}

void USkeletalSkaterVisual::PlayCheck_Implementation()
{
	CheckStart = LocalFrame;
}

void USkeletalSkaterVisual::SetKnockedDown_Implementation(bool bDownNow, float DirX, float DirZ)
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

void USkeletalSkaterVisual::SetHasPuck_Implementation(bool bInHasPuck)
{
	bHasPuck = bInHasPuck;
}

void USkeletalSkaterVisual::SetBoost_Implementation(FName Kind)
{
	Boost = Kind;
	if (bReady && Kind == NAME_None)
	{
		SetEmissive(1.f);   // R-0016: back to the painted-cel base (2.2), not to 1.0
		SetRelativeScale3D(FVector::OneVector);
	}
}

void USkeletalSkaterVisual::SetStickAim_Implementation(float AngleDeg, float ReachM)
{
	AimDeg = FMath::Clamp(AngleDeg, -MaxAimDeg, MaxAimDeg);
	AimReachM = FMath::Clamp(ReachM, 0.6f, 2.2f);
}

void USkeletalSkaterVisual::PlayPoke_Implementation(bool bClean)
{
	PokeStart = LocalFrame;
	bPokeClean = bClean;
}

void USkeletalSkaterVisual::PlayLift_Implementation()
{
	LiftStart = LocalFrame;
}

void USkeletalSkaterVisual::PlayPass_Implementation(float Strength)
{
	// The pawn also calls PlayShoot(0) on a pass frame; the pass follow-through wins (ChooseLayer tests the shot first, so clear it).
	PassStart = LocalFrame;
	ShootStart = -1;
}

void USkeletalSkaterVisual::SetShotCharge_Implementation(float Charge01)
{
	ChargeHold01 = FMath::Clamp(Charge01, 0.f, 1.f);
}

void USkeletalSkaterVisual::SetFaceoff_Implementation(bool bReady_)
{
	bFaceoff = bReady_;
}

void USkeletalSkaterVisual::PlayFaceoffDraw_Implementation(bool bWon)
{
	DrawStart = LocalFrame;
}

void USkeletalSkaterVisual::SetGoalieStance_Implementation(bool bOn, int32 Side)
{
	bGoalie = bOn;
	GoalieSide = FMath::Clamp(Side, -1, 1);
}

void USkeletalSkaterVisual::PlayFoulReaction_Implementation()
{
	FoulStart = LocalFrame;
}
