// Render-only actors for the simulation: AJamSkater (a pawn that owns a sim index and drives its IJamSkaterVisual) and
// AJamPuck. Neither writes anything back into UJamSim; transforms are interpolated between the previous and the current
// sim frame with the accumulator alpha. UJamPlaceholderVisual is the M0 capsule + sphere + stick visual (lane A5's toy
// figure replaces it; contracts/skater-visual.md).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/SceneComponent.h"
#include "JamSkaterVisual.h"
#include "JamSim.h"
#include "JamPawns.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UNiagaraComponent;

UCLASS(ClassGroup = (Jam), meta = (BlueprintSpawnableComponent))
class JAMCORE_API UJamPlaceholderVisual : public USceneComponent, public IJamSkaterVisual
{
	GENERATED_BODY()

public:
	UJamPlaceholderVisual();
	virtual void OnRegister() override;

	virtual void SetTeam_Implementation(int32 TeamId, FLinearColor Colour) override;
	virtual void SetNumber_Implementation(int32 N) override {}
	virtual void UpdateMotion_Implementation(float Speed, float TurnRate, FRotator Facing) override;
	virtual void PlayShoot_Implementation(float Charge) override { SwingFrames = 10 + FMath::RoundToInt32(10.f * FMath::Clamp(Charge, 0.f, 1.f)); }
	virtual void PlayCheck_Implementation() override {}
	virtual void SetKnockedDown_Implementation(bool bDown, float DirX, float DirZ, int32 KnockdownFrames) override;
	virtual void SetHasPuck_Implementation(bool bHasPuck) override {}
	virtual void SetBoost_Implementation(FName Kind) override {}

private:
	UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> Body;
	UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> Head;
	UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> Stick;
	UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> Mid;
	FLinearColor Colour = FLinearColor::White;
	int32 SwingFrames = 0;
	bool bDown = false;
	FVector2D DownDir = FVector2D(1, 0);
};

UCLASS()
class JAMCORE_API AJamSkater : public APawn
{
	GENERATED_BODY()

public:
	AJamSkater();

	// Sets the index, team and visual. VisualClass: a component class or an actor class implementing IJamSkaterVisual;
	// null or invalid falls back to UJamPlaceholderVisual.
	void InitSkater(int32 InSimIndex, int32 InTeam, const FLinearColor& Colour, int32 Number, UClass* VisualClass);

	// Lane A31 (B-to-A-0021): replaces the visual mid-match (a Director's Call changed game.look.tier and with it the figure class): the old
	// component or attached actor is destroyed, the new one created and the setup calls (SetTeam, SetNumber, SetHasPuck, SetBoost) repeated;
	// the per-frame state flags are cleared so the next ApplySimFrame re-sends knocked-down, puck and faceoff state. Same class: no-op, false.
	bool ResetVisual(const FLinearColor& Colour, int32 Number, UClass* VisualClass);
	UClass* GetVisualClass() const { return VisualObject ? VisualObject->GetClass() : nullptr; }

	// After every sim step: drives the visual (UpdateMotion every frame, one-shots on the frame they happen).
	void ApplySimFrame(const FJamSkaterState& S);

	// Every render frame: interpolated transform.
	void ApplyRender(const FJamSkaterState& S, float Alpha);

	UFUNCTION(BlueprintPure, Category = "Jam|Skater") int32 GetSimIndex() const { return SimIndex; }
	UFUNCTION(BlueprintPure, Category = "Jam|Skater") int32 GetTeam() const { return Team; }
	// The object that implements IJamSkaterVisual (a component, or an attached actor).
	UFUNCTION(BlueprintPure, Category = "Jam|Skater") UObject* GetVisualObject() const { return VisualObject; }

	// C-0001 v3 / B-to-A-0014: the world position (cm) of the visual's puck_socket when it exposes one (a mesh socket on any of its components, or a scene
	// component named puck_socket); false when the visual has none. The game mode draws a carried puck there.
	bool GetPuckSocketWorld(FVector& Out) const;


private:
	UObject* CreateVisual(UClass* VisualClass);
	void SetupVisual(UObject* Visual, const FLinearColor& Colour, int32 Number);
	UPROPERTY(VisibleAnywhere, Category = "Jam|Skater") TObjectPtr<USceneComponent> Root;
	UPROPERTY(Transient) TObjectPtr<UObject> VisualObject;
	int32 SimIndex = -1;
	int32 Team = 0;
	bool bWasDown = false;
	bool bHadPuck = false;
	bool bWasAtFaceoff = false;
	bool bWasPinned = false;   // lane A45 (C-0001 v4): SetBoardPin level, sent on the change
	// Lane A45: the sim frame for the JAMVIS log lines (from the event bus; -1 without one). Log only, never feeds the pose.
	int64 SimFrameForLog() const;
	float LastCharge = 0.f;
	float LastAimDeg = 0.f;
	float LastAimReach = 0.f;
};

UCLASS()
class JAMCORE_API AJamPuck : public AActor
{
	GENERATED_BODY()

public:
	AJamPuck();
	void InitPuck(double RadiusM, const FLinearColor& Colour);
	void ApplyRender(const FJamPuckState& P, float Alpha);
	// C-0001 v3: a carried puck drawn at the carrier visual's puck_socket (world cm; the puck keeps its own height above the ice).
	void ApplyRenderAt(const FVector& WorldCm);

	// Lane A33 (R-0028 point 3): the pop. When a check strips the puck (LastTouch goes to -1 while the puck keeps moving on a continuous path, which a faceoff
	// reset never does), the drawn puck hops PopHeightM x sin over PopFrames sim frames. Cosmetic: the sim's puck stays on the ice. Pure for tests.
	static constexpr int32 PopFrames = 18;
	static constexpr double PopHeightM = 0.25;
	static double PopLiftM(double FramesSincePop, double Strength01);
	static bool IsStrip(int32 PrevLastTouch, const FJamPuckState& P);
	int32 GetPopStartFrame() const { return PopStartFrame; }

private:
	UPROPERTY(VisibleAnywhere, Category = "Jam|Puck") TObjectPtr<UStaticMeshComponent> Mesh;
	int32 PrevLastTouch = -1;
	int64 PopStartFrame = -1;
	int64 LastSimFrame = -1;
	double PopStrength = 0.5;
	// R-0016: device B's puck kit. The disc lies on the ice under the puck; the trail follows it. Either may be absent (logged once, never an error).
	UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> Disc;
	UPROPERTY(Transient) TObjectPtr<UNiagaraComponent> Trail;
	double HalfHeightM = 0.04;
};
