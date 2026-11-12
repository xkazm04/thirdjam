// Lane A26 (R-0024 point 1): the Blender toy skater (lane A22's SK_SkaterToy, Game/Content/Figures) on the ice as a skeletal visual next to the
// code-built UToySkaterVisualComponent. Implements IJamSkaterVisual (contracts/skater-visual.md, C-0001 v3) by choosing one of the 17 sequences
// and a position in it FROM THE SIM FRAME (frames since a trigger, the frame counter, the distance phase), never from delta time, and writing the
// sampled bone transforms into a poseable mesh. No Animation Blueprint, no AnimInstance: the pose is a pure function of the call history, so the
// same calls give the same pose at 60 and 144 Hz. Mapping table: docs/experiments/FIGURE.md.
#pragma once

#include "CoreMinimal.h"
#include "Components/PoseableMeshComponent.h"
#include "JamSkaterVisual.h"
#include "SkeletalSkaterVisual.generated.h"

class UAnimSequence;
class UTextRenderComponent;
class UMaterialInstanceDynamic;
class UNiagaraSystem;

// The sequences of SK_SkaterToy (docs/experiments/BLENDER.md section 2), in the order of SkeletalSkater::SeqNames.
enum class ESkaterSeq : uint8
{
	Idle, Skate, Carry, AimLeft, AimRight, Poke, Lift, ShootWindup, ShootRelease, Check, KnockedDown, GetUp, FaceoffReady, Draw,
	GoalieLeft, GoalieCentre, GoalieRight, Count
};

// Everything the layer choice needs for one sim frame. Frames since a trigger are -1 when it never happened.
struct JAMGAME_API FSkelInputs
{
	int32 Frame = 0;            // the visual's own frame counter (one per UpdateMotion call)
	float Phase = 0.f;          // stride phase, 1 = one 36-frame cycle = 2.4 m travelled
	float Speed = 0.f;
	bool bHasPuck = false;
	bool bDown = false;
	bool bFaceoff = false;
	bool bGoalie = false;
	int32 GoalieSide = 0;       // -1 left, 0 centre, +1 right
	float ChargeHold01 = 0.f;
	int32 SinceKnock = -1;      // since SetKnockedDown(true) while down, since SetKnockedDown(false) while up
	int32 SinceCheck = -1;
	int32 SinceFoul = -1;
	int32 SinceShoot = -1;
	int32 SincePass = -1;
	int32 SincePoke = -1;
	bool bPokeClean = true;
	int32 SinceLift = -1;
	int32 SinceDraw = -1;
};

// The whole-body layer for one frame: which sequence, at which frame, and how much the stick aim blend may still move the stick bones.
struct JAMGAME_API FSkelLayer
{
	ESkaterSeq Seq = ESkaterSeq::Idle;
	float Frame = 0.f;          // sequence frame at 60 fps (clamped to the sequence by the sampler)
	float StickAimWeight = 1.f; // 1 = aim_left / aim_right blend applies in full; 0 = a stick verb owns the stick this frame
};

UCLASS(ClassGroup = (Jam), meta = (BlueprintSpawnableComponent))
class JAMGAME_API USkeletalSkaterVisual : public UPoseableMeshComponent, public IJamSkaterVisual
{
	GENERATED_BODY()

public:
	USkeletalSkaterVisual();

	// ---- pure functions (tests call these without a world) ----
	static constexpr float MaxLeanDeg = 35.f;
	static constexpr float BaseEmissive = 2.2f;     // R-0016: the painted-cel base, as the toy figure
	static constexpr float MaxAimDeg = 70.f;        // aim_left / aim_right hold 70 degrees
	// Sequence lengths in frames (the actions as authored; the sampler clamps to the asset's own length).
	static int32 SeqFrames(ESkaterSeq Seq);
	static const TCHAR* SeqName(ESkaterSeq Seq);
	static FSkelLayer ChooseLayer(const FSkelInputs& In);
	static float LeanTarget(float Speed, float TurnRate);

	// ---- IJamSkaterVisual ----
	virtual void SetTeam_Implementation(int32 TeamId, FLinearColor Colour) override;
	virtual void SetNumber_Implementation(int32 N) override;
	virtual void UpdateMotion_Implementation(float Speed, float TurnRate, FRotator Facing) override;
	virtual void PlayShoot_Implementation(float Charge) override;
	virtual void PlayCheck_Implementation() override;
	virtual void SetKnockedDown_Implementation(bool bDown, float DirX, float DirZ) override;
	virtual void SetHasPuck_Implementation(bool bHasPuck) override;
	virtual void SetBoost_Implementation(FName Kind) override;
	virtual void SetStickAim_Implementation(float AngleDeg, float ReachM) override;
	virtual void PlayPoke_Implementation(bool bClean) override;
	virtual void PlayLift_Implementation() override;
	virtual void PlayPass_Implementation(float Strength) override;
	virtual void SetShotCharge_Implementation(float Charge01) override;
	virtual void SetFaceoff_Implementation(bool bReady) override;
	virtual void PlayFaceoffDraw_Implementation(bool bWon) override;
	virtual void SetGoalieStance_Implementation(bool bOn, int32 Side) override;
	virtual void PlayFoulReaction_Implementation() override;

	// Soft paths (the figure assets of lane A22; the blade spray of device B).
	static FSoftObjectPath FigureMeshPath();
	static FSoftObjectPath SequencePath(ESkaterSeq Seq);
	static FSoftObjectPath BladeSprayPath();

	// Hard-turn blade spray requests made so far (the NS_blade_spray hook, as the toy figure).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|SkaterVisual")
	int32 BladeSprayRequests = 0;

	int32 GetLocalFrame() const { return LocalFrame; }
	const FSkelLayer& GetLastLayer() const { return LastLayer; }
	float GetLastLeanDeg() const { return Lean; }
	bool IsFigureReady() const { return bReady; }

protected:
	virtual void OnRegister() override;

private:
	void BuildFigure();
	void ApplyPose(const FRotator& Facing);
	void SampleLocal(ESkaterSeq Seq, float Frame, int32 MeshBone, FTransform& Out) const;
	FSkelInputs MakeInputs() const;
	void SetEmissive(float Multiplier);

	UPROPERTY(Transient) TArray<TObjectPtr<UAnimSequence>> Sequences;   // indexed by ESkaterSeq
	UPROPERTY(Transient) TArray<TObjectPtr<UMaterialInstanceDynamic>> Mids;
	UPROPERTY(Transient) TObjectPtr<UTextRenderComponent> NumberText;
	UPROPERTY(Transient) TObjectPtr<UNiagaraSystem> BladeSpray;

	TArray<int32> SkeletonBoneOfMeshBone;   // mesh bone index -> skeleton bone index
	int32 StickRootBone = INDEX_NONE;
	int32 ArmLBone = INDEX_NONE;
	int32 ArmRBone = INDEX_NONE;
	int32 TorsoBone = INDEX_NONE;

	// State advanced once per sim frame.
	int32 LocalFrame = 0;
	float Phase = 0.f;
	float Lean = 0.f;
	float Speed = 0.f;
	float TurnRate = 0.f;
	bool bHasPuck = false;
	bool bDown = false;
	bool bFaceoff = false;
	bool bGoalie = false;
	int32 GoalieSide = 0;
	FVector2D KnockDir = FVector2D(-1.f, 0.f);
	float AimDeg = 0.f;
	float AimReachM = 1.2f;
	float ChargeHold01 = 0.f;
	int32 KnockStart = -1;
	int32 CheckStart = -1;
	int32 FoulStart = -1;
	int32 ShootStart = -1;
	int32 PassStart = -1;
	int32 PokeStart = -1;
	bool bPokeClean = true;
	int32 LiftStart = -1;
	int32 DrawStart = -1;
	FName Boost = NAME_None;
	FLinearColor TeamColour = FLinearColor(0.5f, 0.5f, 0.5f);
	int32 Number = 0;
	FSkelLayer LastLayer;
	bool bReady = false;
	bool bBuildTried = false;
	// Cost probe (log only, never feeds the pose): seconds spent in ApplyPose since the last SKELVIS line, printed every 600 frames.
	double CostAccumS = 0.0;
	int32 CostCalls = 0;
	bool bSprayTried = false;
};
