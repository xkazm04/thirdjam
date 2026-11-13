// Track T-A: the code-driven toy skater. Implements IJamSkaterVisual (contracts/skater-visual.md). Lane A5.
// Recipe: tools/skills/skater-toy-figure.md. No skeleton, no imported asset: engine basic shapes posed by C++.
// Every pose is a pure function of (state advanced once per UpdateMotion call, frames since a trigger). No wall clock.
#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "JamSkaterVisual.h"
#include "ToySkaterVisual.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;
class UMaterialInstanceDynamic;
class UNiagaraSystem;

// State that advances once per simulation frame (lean follow, stride phase).
struct JAMGAME_API FToyPoseState
{
	float Lean = 0.f;   // degrees, positive = leaning into a left turn
	float Phase = 0.f;  // stride phase, 1 = one full stride
};

struct JAMGAME_API FToyInputs
{
	float Speed = 0.f;
	float TurnRate = 0.f;
	bool bHasPuck = false;
	bool bBoost = false;
	bool bDown = false;
	// Frames since the trigger, or -1 when it has never happened.
	int32 FramesSinceShoot = -1;
	float ShootCharge = 0.f;
	// C-0001 v3: the live charge (wind-up while holding) and the stick aim (yaw left-positive in degrees, reach from the body centre in metres).
	float ChargeHold01 = 0.f;
	float AimDeg = 0.f;
	float AimReachM = 1.2f;
	int32 FramesSinceCheck = -1;
	int32 FramesSinceKnock = -1;
	int32 Frame = 0;
	// Lane A45 (C-0001 v4): a hit taken standing, a board hit, the board pin level, a stick hit on the carrier (lifted = the blade went up).
	int32 FramesSinceHit = -1;
	int32 FramesSinceBoard = -1;
	bool bPinned = false;
	int32 FramesSinceStickHit = -1;
	bool bStickHitLifted = false;
};

struct JAMGAME_API FToyPose
{
	float LeanRollDeg = 0.f;     // roll about the forward axis, Unreal sign (positive = right side down)
	float BodyPitchDeg = 0.f;    // crouch and check lunge; negative = leaning forward
	float BodyYawSwayDeg = 0.f;
	float BobCm = 0.f;
	float StickYawDeg = 0.f;
	float StickPitchDeg = 0.f;
	float StickForwardCm = 0.f;
	float TipAngleDeg = 0.f;     // 0..88 fall angle towards the knock direction
	float HitTipDeg = 0.f;       // lane A45: a hit taken standing tips the toy toward the push on the check curve (8 in, 12 out), 14 degrees at most
	float CheckShiftCm = 0.f;
	float Emissive = 1.f;
	float Scale = 1.f;
};

UCLASS(ClassGroup = (Jam), meta = (BlueprintSpawnableComponent))
class JAMGAME_API UToySkaterVisualComponent : public USceneComponent, public IJamSkaterVisual
{
	GENERATED_BODY()

public:
	UToySkaterVisualComponent();

	// ---- pure pose functions (tests call these without a world) ----
	static constexpr float MaxLeanDeg = 35.f;
	// The painted-cel emissive base (contracts/look-tiers.md); boosts multiply it and SetBoost(NAME_None) restores it (R-0016).
	static constexpr float BaseEmissive = 2.2f;
	static float LeanTarget(float Speed, float TurnRate);
	static void AdvanceState(FToyPoseState& State, float Speed, float TurnRate);
	static float ShotStickYaw(int32 FramesSince);
	static float ShotStickYaw(int32 FramesSince, float Charge) { return ShotStickYaw(FramesSince) * (1.f + 0.6f * FMath::Clamp(Charge, 0.f, 1.f)); }
	static float CheckCurve01(int32 FramesSince);
	static float KnockTipDeg(int32 FramesSince, bool bDown);
	static FToyPose ComputePose(const FToyPoseState& State, const FToyInputs& In);

	// ---- IJamSkaterVisual ----
	virtual void SetTeam_Implementation(int32 TeamId, FLinearColor Colour) override;
	virtual void SetNumber_Implementation(int32 N) override;
	virtual void UpdateMotion_Implementation(float Speed, float TurnRate, FRotator Facing) override;
	virtual void PlayShoot_Implementation(float Charge) override;
	virtual void PlayCheck_Implementation() override;
	virtual void SetKnockedDown_Implementation(bool bDown, float DirX, float DirZ, int32 KnockdownFrames) override;
	virtual void SetHasPuck_Implementation(bool bHasPuck) override;
	virtual void SetBoost_Implementation(FName Kind) override;
	virtual void SetStickAim_Implementation(float AngleDeg, float ReachM) override;
	virtual void SetShotCharge_Implementation(float Charge01) override;
	// Lane A45 (C-0001 v4): mapped onto the existing curves (nothing new to author): the hit = a short tip toward the push on the check curve; the board hit = the
	// body pitched forward on the same curve; the pin = the pitch held; the stick hit = the stick knocked aside (poke) or up (lift) on the check curve.
	virtual void PlayHitReaction_Implementation(float DirX, float DirZ) override;
	virtual void PlayBoardHit_Implementation() override;
	virtual void SetBoardPin_Implementation(bool bOn) override;
	virtual void PlayStickHit_Implementation(bool bLifted) override;

	// Sockets of the contract (JamSkaterSockets): scene components named stick_socket, puck_socket, head_socket.
	UFUNCTION(BlueprintCallable, Category = "Jam|SkaterVisual")
	USceneComponent* GetSocketComponent(FName Socket) const;

	UFUNCTION(BlueprintCallable, Category = "Jam|SkaterVisual")
	FTransform GetSocketWorldTransform(FName Socket) const;

	// Offset from this component's origin to the ice, centimetres (set to -capsule half height when attached to a capsule centre).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|SkaterVisual")
	float GroundOffsetCm = 0.f;

	// Hard-turn blade spray requests made so far (the NS_blade_spray hook; proof reads it).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|SkaterVisual")
	int32 BladeSprayRequests = 0;

	// Soft path of the placeholder blade spray emitter (device B owns the asset, named per contracts/assets-naming.md).
	static FSoftObjectPath BladeSprayPath();

	int32 GetLocalFrame() const { return LocalFrame; }
	const FToyPose& GetLastPose() const { return LastPose; }
	float GetLastLeanDeg() const { return State.Lean; }

protected:
	virtual void OnRegister() override;
	virtual void OnUnregister() override;

private:
	void BuildFigure();
	void ApplyPose(const FToyPose& Pose, const FRotator& Facing);
	void SetColourOn(UStaticMeshComponent* Mesh, const FLinearColor& Colour);
	UStaticMeshComponent* AddMesh(const TCHAR* Name, UStaticMesh* Shape, USceneComponent* Parent, const FVector& LocM, const FVector& SizeM, const FRotator& Rot = FRotator::ZeroRotator);
	USceneComponent* AddNode(const TCHAR* Name, USceneComponent* Parent, const FVector& LocM);

	UPROPERTY(Transient) TObjectPtr<USceneComponent> YawNode;
	UPROPERTY(Transient) TObjectPtr<USceneComponent> LeanNode;
	UPROPERTY(Transient) TObjectPtr<USceneComponent> BodyPivot;
	UPROPERTY(Transient) TObjectPtr<USceneComponent> StickPivot;
	UPROPERTY(Transient) TObjectPtr<USceneComponent> HeadSocket;
	UPROPERTY(Transient) TObjectPtr<USceneComponent> StickSocket;
	UPROPERTY(Transient) TObjectPtr<USceneComponent> PuckSocket;
	UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> Body;
	UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> BodyCap;
	UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> Head;
	UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> HelmetBand;
	UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> Stick;
	UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> Blade;
	// R-0021 feel.00: a hockey stick reads as shaft + heel + blade with two gloved hands on the shaft.
	UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> BladeHeel;
	UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> HandUpper;
	UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> HandLower;
	UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> SkateL;
	UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> SkateR;
	UPROPERTY(Transient) TObjectPtr<UTextRenderComponent> NumberText;
	UPROPERTY(Transient) TArray<TObjectPtr<UMaterialInstanceDynamic>> Mids;
	UPROPERTY(Transient) TObjectPtr<UNiagaraSystem> BladeSpray;

	FToyPoseState State;
	FToyPose LastPose;
	FRotator LastFacing = FRotator::ZeroRotator;
	int32 LocalFrame = 0;
	int32 ShootStart = -1;
	float ShootCharge = 0.f;   // C-0005: the swing grows with the charge
	int32 CheckStart = -1;
	int32 KnockStart = -1;
	bool bDown = false;
	FVector2D KnockDir = FVector2D(-1.f, 0.f);
	// Lane A45 (C-0001 v4) state.
	int32 HitStart = -1;
	FVector2D HitDir = FVector2D(-1.f, 0.f);
	int32 BoardStart = -1;
	bool bPinned = false;
	int32 StickHitStart = -1;
	bool bStickHitLifted = false;
	int32 KnockdownFramesGiven = 0;
	float Speed = 0.f;
	float TurnRate = 0.f;
	bool bHasPuck = false;
	FName Boost = NAME_None;
	float AimDeg = 0.f;
	float AimReachM = 1.2f;
	float ChargeHold01 = 0.f;
	FLinearColor TeamColour = FLinearColor(0.5f, 0.5f, 0.5f);
	int32 Number = 0;
	bool bBuilt = false;
	bool bSprayTried = false;
};
