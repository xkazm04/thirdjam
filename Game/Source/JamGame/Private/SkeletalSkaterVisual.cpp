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
#include "JamPawns.h"
#include "Engine/GameInstance.h"
#include "HAL/IConsoleManager.h"

// Lane A33: a hold of the fall's first frame on a knockdown (the "3-frame visual freeze" R-0028 lets the lane judge; 0 = off, never a time dilation).
static TAutoConsoleVariable<int32> CVarJamHitHoldFrames(TEXT("jam.HitHoldFrames"), 0,
	TEXT("Frames the skeletal figure holds the first frame of its fall after a knockdown (cosmetic; 0 = off). Lane A33."));

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

	// Lane A33: the fall by direction. Front: the authored backward fall (the figure's back faces the push), faster for a harder hit; side and behind: a stiff
	// toy that tips toward the push (ReactionFor), so the limbs hold the idle pose. jam.HitHoldFrames freezes the first fall frame that long.
	const bool bTipFall = In.HitKind != ESkaterHitKind::Front;
	const float FallRate = 0.85f + 0.3f * FMath::Clamp(In.HitStrength, 0.f, 1.f);
	if (In.bDown)
	{
		if (bTipFall) { L.Seq = ESkaterSeq::Idle; L.Frame = 0.f; L.StickAimWeight = 0.f; return L; }
		const int32 Since = In.SinceKnock < 0 ? SeqFrames(ESkaterSeq::KnockedDown) - 1 : FMath::Max(0, In.SinceKnock - FMath::Max(0, In.KnockHold));
		L.Seq = ESkaterSeq::KnockedDown;
		L.Frame = FMath::Clamp((float)Since * FallRate, 0.f, (float)(SeqFrames(ESkaterSeq::KnockedDown) - 1));   // hold the last frame while down
		L.StickAimWeight = 0.f;
		return L;
	}
	if (Within(In.SinceKnock, SeqFrames(ESkaterSeq::GetUp)))
	{
		if (bTipFall) { L.Seq = ESkaterSeq::Idle; L.Frame = 0.f; L.StickAimWeight = 0.f; return L; }
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
	// Lane A33: a clean poke plays whole; a mistimed one (the sim's stumble starts on this frame) is a quick jab (MissPokeFrames at rate 3) and then
	// StumbleFrames of stumble: the first get_up frames held at frame 7, with the forward pitch, the yaw wobble and the low stick from ReactionFor.
	if (In.bPokeClean && Within(In.SincePoke, SeqFrames(ESkaterSeq::Poke)))
	{
		L.Seq = ESkaterSeq::Poke; L.Frame = (float)In.SincePoke; L.StickAimWeight = 0.f; return L;
	}
	if (!In.bPokeClean && Within(In.SincePoke, MissPokeFrames))
	{
		L.Seq = ESkaterSeq::Poke; L.Frame = FMath::Min((float)In.SincePoke * 3.f, (float)(SeqFrames(ESkaterSeq::Poke) - 1)); L.StickAimWeight = 0.f; return L;
	}
	if (!In.bPokeClean && Within(In.SincePoke - MissPokeFrames, StumbleFrames))
	{
		L.Seq = ESkaterSeq::GetUp; L.Frame = (float)FMath::Min(In.SincePoke - MissPokeFrames, PokeStumbleFrames - 1); L.StickAimWeight = 0.f; return L;
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

ESkaterHitKind USkeletalSkaterVisual::HitKindFor(float DirX, float DirZ, float FacingYawDeg)
{
	// The same rule as UJamSim::CollideSkaters (rink x = Unreal X, rink z = Unreal Y, the yaw grows to the right so the left is (sin, -cos)).
	FVector2D Dir(DirX, DirZ);
	if (Dir.IsNearlyZero()) return ESkaterHitKind::Front;
	Dir.Normalize();
	const float Yaw = FMath::DegreesToRadians(FacingYawDeg);
	const FVector2D Fwd(FMath::Cos(Yaw), FMath::Sin(Yaw));
	const float Along = Dir.X * Fwd.X + Dir.Y * Fwd.Y;
	if (Along > 0.5f) return ESkaterHitKind::Behind;
	if (Along < -0.5f) return ESkaterHitKind::Front;
	const FVector2D Left(Fwd.Y, -Fwd.X);
	return (Dir.X * Left.X + Dir.Y * Left.Y) < 0.f ? ESkaterHitKind::SideL : ESkaterHitKind::SideR;
}

const TCHAR* USkeletalSkaterVisual::HitKindName(ESkaterHitKind Kind)
{
	switch (Kind)
	{
	case ESkaterHitKind::SideL: return TEXT("side_l");
	case ESkaterHitKind::SideR: return TEXT("side_r");
	case ESkaterHitKind::Behind: return TEXT("behind");
	default: return TEXT("front");
	}
}

// The whole-body reaction for this frame (docs/experiments/TACKLING.md, reaction table). Every curve is in frames since its trigger.
FSkelReaction USkeletalSkaterVisual::ReactionFor(const FSkelInputs& In)
{
	using namespace SkeletalSkater;
	FSkelReaction R;
	const float Strength = FMath::Clamp(In.HitStrength, 0.f, 1.f);
	// Side and behind falls: the toy tips 88 degrees toward the push over 14 frames with the 3 + 3 bounce of the authored fall (the code figure's curve), holds,
	// and comes back up over the 20 get-up frames. A harder hit tips faster (the frame index runs at 0.85 + 0.3 x strength).
	if (In.HitKind != ESkaterHitKind::Front && In.SinceKnock >= 0)
	{
		if (In.bDown)
		{
			const int32 Since = FMath::Max(0, In.SinceKnock - FMath::Max(0, In.KnockHold));
			R.TipDeg = UToySkaterVisualComponent::KnockTipDeg(FMath::RoundToInt32((float)Since * (0.85f + 0.3f * Strength)), true);
		}
		else if (In.SinceKnock < SeqFrames(ESkaterSeq::GetUp))
		{
			R.TipDeg = UToySkaterVisualComponent::KnockTipDeg(In.SinceKnock, false);
		}
	}
	// A hard board hit: a lean into the boards, 3 frames in and 9 out, 10 to 16 degrees by strength; not while down.
	auto LeanEnv = [](int32 Since) { return Since <= 3 ? FMath::SmoothStep(0.f, 1.f, (float)Since / 3.f) : 1.f - FMath::SmoothStep(0.f, 1.f, (float)(Since - 3) / (float)(BoardLeanFrames - 3)); };
	if (!In.bDown && In.SinceBoard >= 0 && In.SinceBoard < BoardLeanFrames)
	{
		R.BoardLeanDeg = (10.f + 6.f * FMath::Clamp(In.BoardStrength, 0.f, 1.f)) * LeanEnv(In.SinceBoard);
	}
	// Lane A45 (C-0001 v4): the pin holds the lean at its floor (10 degrees) while on; the lean of a fresh board hit still peaks over it.
	if (!In.bDown && In.bPinned) R.BoardLeanDeg = FMath::Max(R.BoardLeanDeg, 10.f);
	// A hit taken standing (PlayHitReaction): the same 12-frame curve, 14 degrees toward the push, then back up (one recovery step); not while down.
	if (!In.bDown && In.SinceHit >= 0 && In.SinceHit < BoardLeanFrames) R.HitTipDeg = 14.f * LeanEnv(In.SinceHit);
	// The stumble after a mistimed poke: a forward pitch that peaks at 12 degrees mid-way, a 5 degree yaw wobble (two cycles, dying out), the stick 25 cm low.
	if (!In.bDown && !In.bPokeClean && In.SincePoke >= MissPokeFrames && In.SincePoke < MissPokeFrames + StumbleFrames)
	{
		const float T = (float)(In.SincePoke - MissPokeFrames) / (float)StumbleFrames;
		const float Bell = FMath::Sin(PI * T);
		R.StumblePitchDeg = 12.f * Bell;
		R.StumbleYawDeg = 5.f * FMath::Sin(4.f * PI * T) * (1.f - T);
		R.StickDropCm = 25.f * Bell;
	}
	// Lane A45 (C-0001 v4): the carrier that lost the puck to a stick (PlayStickHit) plays the stumble layer at once (no jab): the same bell over StumbleFrames; a lift
	// knocks the stick up instead of down (negative drop), a poke drops it as the stumble does. Not while down; a mistimed poke's own stumble takes precedence.
	if (!In.bDown && In.SinceStickHit >= 0 && In.SinceStickHit < StumbleFrames && R.StumblePitchDeg == 0.f)
	{
		const float T = (float)In.SinceStickHit / (float)StumbleFrames;
		const float Bell = FMath::Sin(PI * T);
		R.StumblePitchDeg = 12.f * Bell;
		R.StumbleYawDeg = 5.f * FMath::Sin(4.f * PI * T) * (1.f - T);
		R.StickDropCm = (In.bStickHitLifted ? -25.f : 25.f) * Bell;
	}
	return R;
}

// ---------------------------------------------------------------- construction

void USkeletalSkaterVisual::OnRegister()
{
	Super::OnRegister();
	if (!IsTemplate() && GetOwner() && GetWorld() && GetWorld()->WorldType != EWorldType::EditorPreview)
	{
		BuildFigure();
		// Lane A33: the strength of the hit and a board hit arrive on the event bus (DeliverEvents runs before ApplySimFrame, so the knockdown's strength is
		// in hand when SetKnockedDown(true) follows in the same sim frame). Cosmetic read only.
		if (!Bus)
		{
			UGameInstance* GI = GetWorld()->GetGameInstance();
			Bus = GI ? GI->GetSubsystem<UJamEventBus>() : nullptr;
			if (Bus) Bus->OnJamEvent.AddDynamic(this, &USkeletalSkaterVisual::OnJamEvent);
		}
	}
}

void USkeletalSkaterVisual::OnUnregister()
{
	if (Bus)
	{
		Bus->OnJamEvent.RemoveDynamic(this, &USkeletalSkaterVisual::OnJamEvent);
		Bus = nullptr;
	}
	Super::OnUnregister();
}

int32 USkeletalSkaterVisual::OwnerSimIndex() const
{
	const AJamSkater* Sk = Cast<AJamSkater>(GetOwner());
	return Sk ? Sk->GetSimIndex() : -1;
}

void USkeletalSkaterVisual::OnJamEvent(FName Event, const FJamEventPayload& Payload)
{
	const int32 Me = OwnerSimIndex();
	if (Me < 0 || Payload.Who != Me) return;
	if (Event == JamEvents::Knockdown)
	{
		PendingHitStrength = FMath::Clamp(Payload.Strength, 0.f, 1.f);
	}
	else if (Event == JamEvents::Board && Payload.Kind == FName(TEXT("skater")))
	{
		BoardStart = LocalFrame;
		BoardStrength = FMath::Clamp(Payload.Strength, 0.f, 1.f);
		const FVector D = Payload.Where - GetComponentLocation();
		const FVector2D D2(D.X, D.Y);
		BoardDir = D2.IsNearlyZero() ? FVector2D(0.f, 1.f) : D2.GetSafeNormal();
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
			Mid->SetScalarParameterValue(TEXT("RimStrength"), 1.3f);   // B-to-A-0023 point 4 (lane A31): 1.2..1.5 reads in the dark bands; 0.6 did not
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
	In.HitKind = HitKind;
	In.HitStrength = HitStrength;
	In.KnockHold = KnockHold;
	In.SinceBoard = Since(BoardStart);
	In.BoardStrength = BoardStrength;
	In.SinceHit = Since(HitStart);   // lane A45 (C-0001 v4)
	In.bPinned = bPinned;
	In.SinceStickHit = Since(StickHitStart);
	In.bStickHitLifted = bStickHitLifted;
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
	LastReaction = ReactionFor(In);

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
	// Lane A33: the stick drops low during the stumble (the stick root slides down and a little forward in the parent bone's frame).
	if (StickRootBone != INDEX_NONE && StickRootBone < NumBones && LastReaction.StickDropCm > 0.5f)
	{
		FTransform ParentCS = FTransform::Identity;
		for (int32 B = Ref.GetParentIndex(StickRootBone); B != INDEX_NONE; B = Ref.GetParentIndex(B)) ParentCS = ParentCS * BoneSpaceTransforms[B];
		BoneSpaceTransforms[StickRootBone].AddToTranslation(ParentCS.InverseTransformVectorNoScale(FVector(10.f, 0.f, -LastReaction.StickDropCm)));
	}
	MarkRefreshTransformDirty();
	RefreshBoneTransforms();

	// The whole figure: facing yaw, a yaw toward the knock direction while down (the sequence falls backward, so the back faces it),
	// the lean into turns as a roll about the skates, the boost scale pulse.
	float YawOff = 0.f;
	if (KnockStart >= 0 && HitKind == ESkaterHitKind::Front && (bDown || In.SinceKnock < SeqFrames(ESkaterSeq::GetUp)))
	{
		const float DirYaw = FMath::RadiansToDegrees(FMath::Atan2(KnockDir.Y, KnockDir.X)) + 180.f;
		const float Off = FRotator::NormalizeAxis(DirYaw - Facing.Yaw);
		const float K = bDown ? FMath::SmoothStep(0.f, 1.f, (float)In.SinceKnock / (float)SkeletalSkater::KnockYawInFrames)
			: 1.f - FMath::SmoothStep(0.f, 1.f, (float)In.SinceKnock / (float)SeqFrames(ESkaterSeq::GetUp));
		YawOff = Off * K;
	}
	const FQuat Yaw = FRotator(0.f, Facing.Yaw + YawOff, 0.f).Quaternion();
	const FQuat Roll = FRotator(0.f, 0.f, -FMath::Clamp(Lean, -MaxRollDeg, MaxRollDeg)).Quaternion();   // positive lean = left side down = negative Unreal roll; capped, the rest is in the sequences
	// Lane A33: the reactions are world-axis tips about the skates (the component origin is on the ice): the head moves toward the direction given.
	auto TipToward = [](const FVector2D& Dir2, float Deg)
	{
		if (FMath::Abs(Deg) < 0.01f || Dir2.IsNearlyZero()) return FQuat::Identity;
		const FVector Dir(Dir2.X, Dir2.Y, 0.f);
		const FVector Axis = FVector::CrossProduct(FVector::UpVector, Dir).GetSafeNormal();
		return FQuat(Axis.IsNearlyZero() ? FVector::RightVector : Axis, FMath::DegreesToRadians(Deg));
	};
	const float FacingRad = FMath::DegreesToRadians(Facing.Yaw);
	const FVector2D FwdDir(FMath::Cos(FacingRad), FMath::Sin(FacingRad));
	const FQuat Tip = TipToward(KnockDir, LastReaction.TipDeg);
	const FQuat BoardLean = TipToward(BoardDir, LastReaction.BoardLeanDeg);
	const FQuat HitTip = TipToward(HitDir, LastReaction.HitTipDeg);   // lane A45: a hit taken standing
	const FQuat Stumble = TipToward(FwdDir, LastReaction.StumblePitchDeg) * FRotator(0.f, LastReaction.StumbleYawDeg, 0.f).Quaternion();
	SetWorldRotation(Tip * BoardLean * HitTip * Stumble * Yaw * Roll);

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
	FacingYawDeg = Facing.Yaw;
	const float Follow = 1.f - FMath::Exp(-12.f / 60.f);
	Lean += (LeanTarget(Speed, TurnRate) - Lean) * Follow;
	Phase += Speed / 2.4f / 60.f;
	if (Phase > 1024.f) Phase -= 1024.f;   // keep the float exact over a long match
	ApplyPose(Facing);

	// Blade spray hook on a hard turn (|lean| >= 25 deg): a request every 8 frames, device B's placeholder Niagara system by soft path.
	if (FMath::Abs(Lean) >= 25.f && LocalFrame % 12 == 0 && !bDown)
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

void USkeletalSkaterVisual::SetKnockedDown_Implementation(bool bDownNow, float DirX, float DirZ, int32 KnockdownFrames)
{
	if (bDownNow == bDown)
	{
		return;
	}
	bDown = bDownNow;
	KnockStart = LocalFrame;
	KnockdownFramesGiven = bDownNow ? KnockdownFrames : 0;   // C-0001 v4: the down time (B's get-up ends on it; this figure holds its last fall frame)
	if (bDownNow)
	{
		const FVector2D D(DirX, DirZ);
		KnockDir = D.IsNearlyZero() ? FVector2D(-1.f, 0.f) : D.GetSafeNormal();
		// Lane A33: the kind from the push against the facing the pawn gave last frame; the strength from the knockdown event that arrived this frame (0.5 when
		// no event came: a scene that calls the interface by hand); the optional hold of the first fall frame.
		HitKind = HitKindFor(KnockDir.X, KnockDir.Y, FacingYawDeg);
		HitStrength = PendingHitStrength >= 0.f ? PendingHitStrength : 0.5f;
		PendingHitStrength = -1.f;
		KnockHold = FMath::Clamp(CVarJamHitHoldFrames.GetValueOnGameThread(), 0, 30);
		UE_LOG(LogTemp, Log, TEXT("SKELVIS %s knocked down kind=%s strength=%.2f hold=%d dir=(%.2f, %.2f) facing=%.0f down_frames=%d"), GetOwner() ? *GetOwner()->GetName() : TEXT("?"),
			HitKindName(HitKind), HitStrength, KnockHold, KnockDir.X, KnockDir.Y, FacingYawDeg, KnockdownFramesGiven);
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

// ---- Lane A45 (C-0001 v4): defaults onto the existing reactions ----

void USkeletalSkaterVisual::PlayHitReaction_Implementation(float DirX, float DirZ)
{
	if (bDown) return;
	HitStart = LocalFrame;
	const FVector2D D(DirX, DirZ);
	HitDir = D.IsNearlyZero() ? FVector2D(-FMath::Cos(FMath::DegreesToRadians(FacingYawDeg)), -FMath::Sin(FMath::DegreesToRadians(FacingYawDeg))) : D.GetSafeNormal();
}

void USkeletalSkaterVisual::PlayBoardHit_Implementation()
{
	// The bus's skater board event (OnJamEvent) usually started the lean this very frame with its strength and direction; the call stands in when none came
	// (a scene driving the interface by hand): the lean toward the facing at the floor strength.
	if (BoardStart == LocalFrame) return;
	BoardStart = LocalFrame;
	BoardStrength = 0.5f;
	BoardDir = FVector2D(FMath::Cos(FMath::DegreesToRadians(FacingYawDeg)), FMath::Sin(FMath::DegreesToRadians(FacingYawDeg)));
}

void USkeletalSkaterVisual::SetBoardPin_Implementation(bool bOn)
{
	bPinned = bOn;
}

void USkeletalSkaterVisual::PlayStickHit_Implementation(bool bLifted)
{
	StickHitStart = LocalFrame;
	bStickHitLifted = bLifted;
}
