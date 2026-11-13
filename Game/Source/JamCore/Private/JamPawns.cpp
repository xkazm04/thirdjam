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
#include "HAL/IConsoleManager.h"
#include "Components/SkinnedMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequenceBase.h"

// Lane A80: a presentation-only read-out of the figure's lowest bones against the ice, for the knockdown strips (jam.vis.lowlog 1; off by default; the sim never sees it).
static TAutoConsoleVariable<int32> CVarJamVisLow(TEXT("jam.vis.lowlog"), 0,
	TEXT("1 = log JAMLOW per sim frame for every skater from its knockdown to 70 frames after the release: lowest bone and pelvis / hand / head / foot Z in cm above the pawn's ice plane, the montage. Lane A80 proof."));
namespace
{
	struct FJamLowTrack { int64 Start = -1; int64 Release = -1; float StandPelvis = 0.f; bool bHadDown = false; };
	TMap<const AJamSkater*, FJamLowTrack> GJamLow;

	void JamLowReadout(const AJamSkater* P, UObject* V, const FJamSkaterState& S, int64 Frame, bool bHeld)
	{
		if (CVarJamVisLow.GetValueOnGameThread() == 0) return;
		AActor* A = Cast<AActor>(V);
		if (!A) return;
		FJamLowTrack& T = GJamLow.FindOrAdd(P);
		const bool bDown = S.IsDown();
		if (bDown && !T.bHadDown) { T.bHadDown = true; T.Start = Frame; T.Release = -1; }
		if (!bDown && T.bHadDown && T.Release < 0) T.Release = Frame;
		const bool bActive = bDown || (T.Release >= 0 && Frame - T.Release <= 70);
		if (!bActive) { T.bHadDown = false; }
		if (!bActive && (Frame % 10) != 0) return;
		TArray<USkinnedMeshComponent*> Cs;
		A->GetComponents<USkinnedMeshComponent>(Cs);
		USkinnedMeshComponent* M = nullptr;
		for (USkinnedMeshComponent* C : Cs) if (C && C->GetNumBones() > 0 && (!M || C->GetNumBones() > M->GetNumBones())) M = C;
		if (!M) return;
		M->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
		const float Ice = static_cast<float>(P->GetActorLocation().Z);
		float LowBody = 1e9f; FName LowBodyName;
		float Low = 1e9f, Pelvis = 1e9f, Hand = 1e9f, Head = 1e9f, Foot = 1e9f;
		FName LowName;
		for (int32 i = 0; i < M->GetNumBones(); ++i)
		{
			const FName N = M->GetBoneName(i);
			const float Z = static_cast<float>(M->GetBoneLocation(N, EBoneSpaces::WorldSpace).Z) - Ice;
			if (Z < Low) { Low = Z; LowName = N; }
			const FString L = N.ToString().ToLower();
			const bool bHelper = L.Contains(TEXT("socket")) || L.Contains(TEXT("ik")) || L.Contains(TEXT("root")) || L.Contains(TEXT("stick")) || L.Contains(TEXT("puck")) || L.Contains(TEXT("blade"));
			if (!bHelper && Z < LowBody) { LowBody = Z; LowBodyName = N; }
			if (L.Contains(TEXT("pelvis")) || L.Equals(TEXT("hips"))) Pelvis = FMath::Min(Pelvis, Z);
			else if (L.Contains(TEXT("hand")) && !L.Contains(TEXT("ik"))) Hand = FMath::Min(Hand, Z);
			else if (L.Contains(TEXT("head"))) Head = FMath::Min(Head, Z);
			else if (L.Contains(TEXT("foot"))) Foot = FMath::Min(Foot, Z);
		}
		if (!bActive) { if (Pelvis < 1e8f) T.StandPelvis = Pelvis; return; }
		FString Montage = TEXT("none");
		if (USkeletalMeshComponent* SK = Cast<USkeletalMeshComponent>(M))
			if (UAnimInstance* AI = SK->GetAnimInstance()) if (UAnimMontage* Mo = AI->GetCurrentActiveMontage())
			{
				Montage = Mo->GetName();
				if (Mo->SlotAnimTracks.Num() > 0 && Mo->SlotAnimTracks[0].AnimTrack.AnimSegments.Num() > 0)
					if (const UAnimSequenceBase* Seq = Mo->SlotAnimTracks[0].AnimTrack.AnimSegments[0].GetAnimReference()) Montage = Seq->GetName();
			}
		UE_LOG(LogJamSim, Log, TEXT("JAMLOW frame=%lld skater=%d human=%d down=%d held=%d rel=%lld low_cm=%.1f low=%s pelvis_cm=%.1f hand_cm=%.1f head_cm=%.1f foot_cm=%.1f stand_pelvis_cm=%.1f dir=(%.2f, %.2f) montage=%s"),
			Frame, S.Index, S.bHuman ? 1 : 0, bDown ? 1 : 0, bHeld ? 1 : 0, Frame - T.Start, LowBody, *LowBodyName.ToString(), Pelvis, Hand, Head, Foot, T.StandPelvis, S.KnockDir.X, S.KnockDir.Y, *Montage);
	}
}

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

// Lane A73 (B-to-A-0036 point 4): the frames the figures, the puck and the presentation frame hold on a knockdown that involves the human. Presentation only:
// the sim steps every frame whatever this says; 0 turns the hit-stop off. The operator tastes it with jam.hitstop 0..6.
static TAutoConsoleVariable<int32> CVarJamHitStop(TEXT("jam.hitstop"), 3,
	TEXT("Hit-stop frames (0..6, default 3) on a knockdown that involves the human: both figures, the puck, the camera and the HUD hold the hit frame while the sim runs on. 0 = off."));

int32 AJamSkater::HitStopFrames()
{
	return FMath::Clamp(CVarJamHitStop.GetValueOnGameThread(), 0, 6);
}

// Lane A101 (track F1): the hold reads the hit. At jam.hitstop 3: stumble 1 or 2, knockdown 2 to 5 by the ratio; a bump holds nothing.
int32 AJamSkater::HitStopFramesFor(int32 Outcome, float Ratio, const FJamContactConfig& K, int32 Base)
{
	if (Base <= 0) return 0;
	if (Outcome == UJamSim::ContactNone) return FMath::Clamp(Base, 0, 6);   // a knockdown the model did not resolve (contact.momentum false): as before
	double At3 = 0.0;
	if (Outcome == UJamSim::ContactStumble) At3 = static_cast<double>(Ratio) >= 0.5 * (K.BumpRatio + K.DownRatio) - 1e-6 ? 2.0 : 1.0;   // the read-out is a float: 0.7f is the middle
	else if (Outcome == UJamSim::ContactDown) At3 = FMath::Lerp(2.0, 5.0, FMath::Clamp((static_cast<double>(Ratio) - K.DownRatio) / FMath::Max(1e-6, K.DownFullRatio - K.DownRatio), 0.0, 1.0));
	else return 0;   // a bump
	return FMath::Clamp(FMath::RoundToInt32(At3 * Base / 3.0), 1, 6);
}

FJamHitStopPick AJamSkater::HitStopPick(const TArray<FJamSkaterState>& Sk, const TArray<FJamSimEvent>& Events, int32 Human, double BumpRatio)
{
	static const FName CauseCheck(TEXT("check")), KindStumble(TEXT("stumble")), KindBounce(TEXT("bounce"));
	FJamHitStopPick P;
	int32 LastChecker = -1;
	FName LastKind;
	for (const FJamSimEvent& E : Events) if (E.Name == JamEvents::Check) { LastChecker = E.Who; LastKind = E.Kind; }
	// The check event of one attacker: its strength (closing speed / skater.speed) and its kind.
	auto CheckOf = [&](int32 Who, float& Strength, FName& Kind)
	{
		for (const FJamSimEvent& E : Events) if (E.Name == JamEvents::Check && E.Who == Who) { Strength = E.Strength; Kind = E.Kind; return true; }
		return false;
	};
	// 1. A knockdown by a check: the one the human took, else the one it landed.
	for (const FJamSimEvent& E : Events)
	{
		if (E.Name != JamEvents::Knockdown || E.Kind != CauseCheck) continue;
		const bool bResolved = Sk.IsValidIndex(E.Who) && Sk[E.Who].ContactOutcome == UJamSim::ContactDown;
		const int32 Checker = bResolved && Sk[E.Who].ContactBy >= 0 ? Sk[E.Who].ContactBy : LastChecker;
		if (E.Who != Human && Checker != Human) continue;
		if (P.Victim == Human) continue;   // the human's own fall is already picked
		P.Victim = E.Who;
		P.Checker = Checker;
		P.Outcome = bResolved ? UJamSim::ContactDown : UJamSim::ContactNone;
		P.Ratio = bResolved ? Sk[E.Who].ContactRatio : 0.f;
		P.Strength = E.Strength;
		float Unused = 0.f;
		P.Kind = LastKind;
		CheckOf(Checker, Unused, P.Kind);
	}
	if (P.Victim >= 0) return P;
	// 2. A stumble (no event names its victim: the read-outs do): the one the human took, else the one it landed. Its strength is its check event's.
	for (int32 Pass = 0; Pass < 2 && P.Victim < 0; ++Pass)
	{
		for (const FJamSkaterState& S : Sk)
		{
			if (S.ContactOutcome != UJamSim::ContactStumble || (Pass == 0 ? S.Index != Human : S.ContactBy != Human)) continue;
			P.Victim = S.Index;
			P.Checker = S.ContactBy;
			P.Outcome = UJamSim::ContactStumble;
			P.Ratio = S.ContactRatio;
			FName Unused;
			CheckOf(S.ContactBy, P.Strength, Unused);
			P.Kind = KindStumble;
			break;
		}
	}
	if (P.Victim >= 0) return P;
	// 3. The human's own bounce: it ran into a skater more stable than itself and is the one who stumbles.
	if (Sk.IsValidIndex(Human) && Sk[Human].bRecoilThisFrame)
	{
		P.Victim = Human;
		for (const FJamSkaterState& S : Sk) if (S.ContactBy == Human) P.Checker = S.Index;   // the skater it bounced off
		P.Outcome = UJamSim::ContactStumble;
		P.Ratio = static_cast<float>(BumpRatio);
		FName Unused;
		CheckOf(Human, P.Strength, Unused);   // a bump emits no check event: 0 then
		P.Kind = KindBounce;
	}
	return P;
}

void AJamSkater::BeginHitStop(int32 Frames)
{
	if (Frames <= 0) return;
	HoldArmed = FMath::Clamp(Frames, 1, 6);
}

void AJamSkater::SetVisualTimeDilation(float Dilation)
{
	// The pawn's components (the toy figure, its Niagara) tick with the pawn's dilation; B's figure is an attached actor with a dilation of its own.
	CustomTimeDilation = Dilation;
	if (AActor* A = Cast<AActor>(VisualObject.Get())) A->CustomTimeDilation = Dilation;
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
	// Lane A73: the hit-stop bookkeeping, one step per sim frame. The hit frame (HoldArmed) is applied in full below and its pose recorded; the next HoldTotal
	// frames are "held": edges and one-shots still go out, the continuous calls do not, the actor is drawn at the held pose; then CatchUpLeft frames blend back.
	const bool bHitFrame = HoldArmed > 0;
	if (bHitFrame)
	{
		HoldTotal = HoldArmed;
		HoldLeft = HoldArmed;
		HoldArmed = 0;
		CatchUpLeft = 0;
		HeldLoc = UJamSim::ToWorld(S.Pos, 0.0);
		HeldRot = FRotator(0.0, FMath::RadiansToDegrees(S.Yaw + FMath::DegreesToRadians(S.SpinVisualYawDeg())), 0.0);
		SetVisualTimeDilation(0.0001f);   // not 0: nothing downstream divides by a zero delta
	}
	else if (HoldLeft > 0)
	{
		if (--HoldLeft == 0)
		{
			SetVisualTimeDilation(1.f);
			CatchUpLeft = HoldTotal;
		}
	}
	else if (CatchUpLeft > 0)
	{
		--CatchUpLeft;
	}
	const bool bHeld = !bHitFrame && HoldLeft > 0;
	// B-to-A-0024 point 2 (lane A31): PlayShoot only for a shot; a pass is PlayPass alone (below), never a charge-0 shot in the same frame.
	if (S.bShotThisFrame) IJamSkaterVisual::Execute_PlayShoot(V, S.LastShotCharge);
	if (S.bCheckThisFrame) IJamSkaterVisual::Execute_PlayCheck(V);
	if (S.IsDown() != bWasDown)
	{
		bWasDown = S.IsDown();
		// C-0001 v4: the down time rides along (KnockFrames is the whole knockdown on the frame it starts: StepSkater counts it down from the next step on).
		// Lane A73 sent KnockFrames - HoldTotal (the visual's clock stands still during the hit-stop). Lane A80 measured the get-up drifting one frame earlier per frame of
		// hold: B's montage freezes but its get-up still ends in world time, so the hold was counted twice. The Director (13:30): the unreduced KnockFrames go out.
		const int32 DownFrames = bWasDown ? FMath::Max(1, S.KnockFrames) : 0;
		IJamSkaterVisual::Execute_SetKnockedDown(V, bWasDown, static_cast<float>(S.KnockDir.X), static_cast<float>(S.KnockDir.Y), DownFrames);
		if (bWasDown)
		{
			UE_LOG(LogJamSim, Log, TEXT("JAMVIS knockdown frame=%lld skater=%d team=%d human=%d dir=(%.2f, %.2f) sim_frames=%d sent_frames=%d hitstop=%d"),
				SimFrameForLog(), S.Index, S.Team, S.bHuman ? 1 : 0, S.KnockDir.X, S.KnockDir.Y, S.KnockFrames, DownFrames, bHitFrame ? HoldTotal : 0);
		}
		else
		{
			UE_LOG(LogJamSim, Log, TEXT("JAMVIS getup frame=%lld skater=%d team=%d human=%d (SetKnockedDown false: the sim released the skater)"), SimFrameForLog(), S.Index, S.Team, S.bHuman ? 1 : 0);
		}
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
		// Lane A101: also on a landed check that pushed the skater into boards within 1.5 m (bContactBoard; the `board` event then carries the check's strength).
		UE_LOG(LogJamSim, Log, TEXT("JAMVIS board frame=%lld skater=%d team=%d checked=%d outcome=%d ratio=%.2f (PlayBoardHit: the skater board event: 4 m/s and up into the boards, or checked into them)"),
			SimFrameForLog(), S.Index, S.Team, S.bContactBoard ? 1 : 0, S.ContactOutcome, S.ContactRatio);
	}
	// Lane A113 (track F1): what became of the human's own check press when it did not land (the sim's counters behind the HUD's contact line), one JAMVIS line
	// when one grows: the headless read-out of JUST GOT UP / TOO SOFT / MISSED (the HUD does not run under -nullrhi; an acceptance row can read this line). A
	// restart sets the counters back: no line then. And one line for any skater whose hit the release grace made a bump.
	if (S.ChecksSoft != SeenSoft || S.ChecksMissed != SeenMissed || S.ChecksGraced != SeenGraced)
	{
		const TCHAR* Result = S.ChecksGraced > SeenGraced ? TEXT("just_got_up") : S.ChecksSoft > SeenSoft ? TEXT("too_soft") : S.ChecksMissed > SeenMissed ? TEXT("missed") : nullptr;
		if (Result && S.bHuman)
		{
			UE_LOG(LogJamSim, Log, TEXT("JAMVIS press frame=%lld skater=%d team=%d human=1 result=%s soft=%d missed=%d graced=%d (the human's check press did not land; the HUD's contact line)"),
				SimFrameForLog(), S.Index, S.Team, Result, S.ChecksSoft, S.ChecksMissed, S.ChecksGraced);
		}
		SeenSoft = S.ChecksSoft; SeenMissed = S.ChecksMissed; SeenGraced = S.ChecksGraced;
	}
	if (S.bContactGrace)
	{
		UE_LOG(LogJamSim, Log, TEXT("JAMVIS grace frame=%lld skater=%d team=%d human=%d by=%d ratio=%.2f grace_left=%d (a hit inside the release grace: a bump at most)"),
			SimFrameForLog(), S.Index, S.Team, S.bHuman ? 1 : 0, S.ContactBy, S.ContactRatio, S.GraceFrames);
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
	// Lane A73: while held, the continuous calls (UpdateMotion, SetStickAim, SetShotCharge) stay away: the figure keeps the hit frame's motion state.
	JamLowReadout(this, V, S, SimFrameForLog(), bHeld);   // lane A80: off unless jam.vis.lowlog 1
	if (bHeld)
	{
		if (S.PokeResult != 0) IJamSkaterVisual::Execute_PlayPoke(V, S.PokeResult > 0);
		if (S.bLiftThisFrame) IJamSkaterVisual::Execute_PlayLift(V);
		if (S.bPassThisFrame) IJamSkaterVisual::Execute_PlayPass(V, S.LastPassStrength);
		if (S.bAtFaceoff != bWasAtFaceoff) { bWasAtFaceoff = S.bAtFaceoff; IJamSkaterVisual::Execute_SetFaceoff(V, bWasAtFaceoff); }
		if (S.DrawResult != 0) IJamSkaterVisual::Execute_PlayFaceoffDraw(V, S.DrawResult > 0);
		if (S.bFoulThisFrame) IJamSkaterVisual::Execute_PlayFoulReaction(V);
		return;
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
	const FRotator Rot(0.0, FMath::RadiansToDegrees(Yaw), 0.0);
	// Lane A73: held = the hit frame's pose; catching up = a blend from it to the live pose over HoldTotal frames (w runs 0 -> 1, Alpha inside each frame).
	if (HoldLeft > 0)
	{
		SetActorLocationAndRotation(HeldLoc, HeldRot);
		return;
	}
	if (CatchUpLeft > 0 && HoldTotal > 0)
	{
		const float W = FMath::Clamp((static_cast<float>(HoldTotal - CatchUpLeft) + Alpha) / static_cast<float>(HoldTotal), 0.f, 1.f);
		SetActorLocationAndRotation(FMath::Lerp(HeldLoc, Loc, W), FQuat::Slerp(HeldRot.Quaternion(), Rot.Quaternion(), W).Rotator());
		return;
	}
	SetActorLocationAndRotation(Loc, Rot);
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
	Place(Lerp2D(P.PrevPos, P.Pos, Alpha, RenderHeightM(P, Alpha, HalfHeightM)), Alpha);
}

void AJamPuck::ApplyRenderAt(const FVector& WorldCm)
{
	Place(FVector(WorldCm.X, WorldCm.Y, HalfHeightM * 100.0), 1.f);
}

void AJamPuck::BeginHitStop(int32 Frames)
{
	// Lane A73: hold where the puck was last drawn. Called on the hit frame before that frame's StepSimFrame, which counts one down.
	if (Frames <= 0) return;
	HoldTotal = FMath::Clamp(Frames, 1, 6);
	HoldLeft = HoldTotal + 1;
	CatchUpLeft = 0;
	HeldLoc = bEverPlaced ? LastPlaced : GetActorLocation();
	CustomTimeDilation = 0.0001f;
}

void AJamPuck::StepSimFrame()
{
	if (HoldLeft > 0)
	{
		if (--HoldLeft == 0) { CustomTimeDilation = 1.f; CatchUpLeft = HoldTotal; }
	}
	else if (CatchUpLeft > 0) --CatchUpLeft;
}

void AJamPuck::Place(const FVector& LiveCm, float Alpha)
{
	FVector Loc = LiveCm;
	if (HoldLeft > 0) Loc = HeldLoc;
	else if (CatchUpLeft > 0 && HoldTotal > 0)
	{
		const float W = FMath::Clamp((static_cast<float>(HoldTotal - CatchUpLeft) + Alpha) / static_cast<float>(HoldTotal), 0.f, 1.f);
		Loc = FMath::Lerp(HeldLoc, LiveCm, W);
	}
	LastPlaced = Loc;
	bEverPlaced = true;
	SetActorLocation(Loc);
	PlaceDiscOnIce(Loc);
}

void AJamPuck::PlaceDiscOnIce(const FVector& PuckCm)
{
	// The disc is attached under the puck mesh (parent-local z -0.45 of a 0.08 m tall unit cylinder = 3.6 cm under the centre, 0.4 cm over the ice when the
	// puck sits on it); with the puck in the air it is placed in world space at the same 0.4 cm over the ice.
	if (Disc) Disc->SetWorldLocation(FVector(PuckCm.X, PuckCm.Y, (HalfHeightM - 0.45 * 0.08) * 100.0));
}
