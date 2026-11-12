// Proof scene for lane A5 (camera, HUD, toy figure, input, effects binder) that does not need lane A1's simulation.
// A scripted mover runs a fixed 60 Hz accumulator, pushes FJamPresentationFrame into UJamPresentation, and drives six toy skaters
// through IJamSkaterVisual exactly like the real skater pawn does. Placed on a map by hand or spawned by tests; never cooked into the game.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "CameraFollow.h"
#include "HUDFeed.h"
#include "CameraProofScene.generated.h"

class UToySkaterVisualComponent;
class UJamPlayerInput;
class UJamEffectsBinder;
class UJamHudWidget;
class UStaticMeshComponent;

// The camera pawn of the proof scene: a camera and the input component, nothing else.
UCLASS()
class JAMGAME_API AJamProofPawn : public APawn
{
	GENERATED_BODY()

public:
	AJamProofPawn();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Jam")
	TObjectPtr<UJamFollowCamera> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Jam")
	TObjectPtr<UJamPlayerInput> Input;
};

UCLASS()
class JAMGAME_API AJamProofScene : public AActor
{
	GENERATED_BODY()

public:
	AJamProofScene();

	// The human skater is driven by a script until real input arrives (any axis > 0.1), then by the input component.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Proof")
	bool bScriptedHuman = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Proof")
	FJamCameraSettings CameraConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Proof")
	float RinkLengthM = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Proof")
	float RinkWidthM = 60.f;

	// Possess a spawned AJamProofPawn and show the HUD at BeginPlay.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Proof")
	bool bSpawnPawnAndHud = true;

	UFUNCTION(BlueprintCallable, Category = "Jam|Proof")
	void SetPhaseForProof(EJamPhase Phase, int32 Home, int32 Away);

	UFUNCTION(BlueprintCallable, Category = "Jam|Proof")
	void SetArenaMode(bool bArena);

	UFUNCTION(BlueprintCallable, Category = "Jam|Proof")
	void PlaceHumanAt(float XM, float ZM, float YawDeg);

	// Advances n simulation frames at once (tests and screenshots); same code path as the accumulator.
	UFUNCTION(BlueprintCallable, Category = "Jam|Proof")
	void StepFrames(int32 N);

	UFUNCTION(BlueprintPure, Category = "Jam|Proof")
	int32 GetSimFrame() const { return (int32)SimFrame; }

	UFUNCTION(BlueprintPure, Category = "Jam|Proof")
	AJamProofPawn* GetPawn() const { return Pawn; }

	UFUNCTION(BlueprintPure, Category = "Jam|Proof")
	UJamHudWidget* GetHud() const { return Hud; }

	UFUNCTION(BlueprintPure, Category = "Jam|Proof")
	UJamEffectsBinder* GetBinder() const { return Binder; }

	UFUNCTION(BlueprintPure, Category = "Jam|Proof")
	UToySkaterVisualComponent* GetVisual(int32 I) const { return Visuals.IsValidIndex(I) ? Visuals[I].Get() : nullptr; }

	// Test hook: broadcast a shot / check / knockdown through the event bus as the sim would.
	UFUNCTION(BlueprintCallable, Category = "Jam|Proof")
	void BroadcastEventForProof(FName Event, int32 Who);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void BuildRink();
	void BuildSkaters();
	void StepSim();
	void PushFrame();

	struct FSk
	{
		FVector2D Pos = FVector2D::ZeroVector;
		float YawDeg = 0.f;
		float Speed = 0.f;
		float TurnRate = 0.f;  // rad/s, positive = left seen from above (the contract's sign)
		bool bDown = false;
		int32 DownUntil = 0;
	};

	UPROPERTY(Transient) TArray<TObjectPtr<UToySkaterVisualComponent>> Visuals;
	UPROPERTY(Transient) TArray<TObjectPtr<USceneComponent>> Nodes;
	UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> PuckMesh;
	UPROPERTY(Transient) TObjectPtr<AJamProofPawn> Pawn;
	UPROPERTY(Transient) TObjectPtr<UJamHudWidget> Hud;
	UPROPERTY(Transient) TObjectPtr<UJamEffectsBinder> Binder;

	TArray<FSk> Sk;
	FVector2D Puck = FVector2D::ZeroVector;
	FVector2D PuckVel = FVector2D::ZeroVector;
	int32 PuckFree = 0;  // frames until the human picks the puck up again
	int64 SimFrame = 0;
	float Accumulator = 0.f;
	EJamPhase Phase = EJamPhase::Play;
	int32 ScoreHome = 0;
	int32 ScoreAway = 0;
	bool bArenaMode = false;
	bool bManual = false;
};
