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
	virtual void PlayShoot_Implementation() override { SwingFrames = 10; }
	virtual void PlayCheck_Implementation() override {}
	virtual void SetKnockedDown_Implementation(bool bDown, float DirX, float DirZ) override;
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

	// After every sim step: drives the visual (UpdateMotion every frame, one-shots on the frame they happen).
	void ApplySimFrame(const FJamSkaterState& S);

	// Every render frame: interpolated transform.
	void ApplyRender(const FJamSkaterState& S, float Alpha);

	UFUNCTION(BlueprintPure, Category = "Jam|Skater") int32 GetSimIndex() const { return SimIndex; }
	UFUNCTION(BlueprintPure, Category = "Jam|Skater") int32 GetTeam() const { return Team; }
	// The object that implements IJamSkaterVisual (a component, or an attached actor).
	UFUNCTION(BlueprintPure, Category = "Jam|Skater") UObject* GetVisualObject() const { return VisualObject; }

private:
	UPROPERTY(VisibleAnywhere, Category = "Jam|Skater") TObjectPtr<USceneComponent> Root;
	UPROPERTY(Transient) TObjectPtr<UObject> VisualObject;
	int32 SimIndex = -1;
	int32 Team = 0;
	bool bWasDown = false;
	bool bHadPuck = false;
};

UCLASS()
class JAMCORE_API AJamPuck : public AActor
{
	GENERATED_BODY()

public:
	AJamPuck();
	void InitPuck(double RadiusM, const FLinearColor& Colour);
	void ApplyRender(const FJamPuckState& P, float Alpha);

private:
	UPROPERTY(VisibleAnywhere, Category = "Jam|Puck") TObjectPtr<UStaticMeshComponent> Mesh;
	double HalfHeightM = 0.04;
};
