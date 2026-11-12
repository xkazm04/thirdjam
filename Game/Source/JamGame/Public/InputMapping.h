// Enhanced Input for Hokej, built in C++ (no editor assets): WASD / left stick move, space / A shoot, shift / B check, R restart, Esc pause. Lane A5.
// A pawn adds a UJamPlayerInput component and reads Axis / IsShootHeld / IsCheckHeld each simulation step.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "InputMapping.generated.h"

class UInputAction;
class UInputMappingContext;
class UEnhancedInputComponent;
class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FJamInputButtonSignature);

// The five actions and the mapping context, created once per process.
UCLASS()
class JAMGAME_API UJamInputMap : public UObject
{
	GENERATED_BODY()

public:
	static UJamInputMap* Get();

	UPROPERTY() TObjectPtr<UInputAction> Move;     // Axis2D: X right, Y forward (W / left stick up = +Y)
	UPROPERTY() TObjectPtr<UInputAction> Shoot;    // bool
	UPROPERTY() TObjectPtr<UInputAction> Check;    // bool
	UPROPERTY() TObjectPtr<UInputAction> Restart;  // bool
	UPROPERTY() TObjectPtr<UInputAction> Pause;    // bool
	UPROPERTY() TObjectPtr<UInputMappingContext> Context;

	// Keys mapped to an action, for tests ("is the gamepad A button on Shoot").
	bool IsKeyMapped(const UInputAction* Action, const FKey& Key) const;

private:
	void Build();
};

UCLASS(ClassGroup = (Jam), meta = (BlueprintSpawnableComponent))
class JAMGAME_API UJamPlayerInput : public UActorComponent
{
	GENERATED_BODY()

public:
	UJamPlayerInput();

	// Axis in [-1, 1]: X = right, Y = forward. Held value, polled once per simulation step.
	UFUNCTION(BlueprintPure, Category = "Jam|Input")
	FVector2D GetAxis() const { return Axis; }

	UFUNCTION(BlueprintPure, Category = "Jam|Input")
	bool IsShootHeld() const { return bShoot; }

	UFUNCTION(BlueprintPure, Category = "Jam|Input")
	bool IsCheckHeld() const { return bCheck; }

	// True once after each press (latched until read): a shot or check is an event, not a level.
	UFUNCTION(BlueprintCallable, Category = "Jam|Input")
	bool ConsumeShootPressed();

	UFUNCTION(BlueprintCallable, Category = "Jam|Input")
	bool ConsumeCheckPressed();

	UPROPERTY(BlueprintAssignable, Category = "Jam|Input")
	FJamInputButtonSignature OnRestart;

	UPROPERTY(BlueprintAssignable, Category = "Jam|Input")
	FJamInputButtonSignature OnPause;

	UFUNCTION(BlueprintPure, Category = "Jam|Input")
	bool IsBound() const { return bBound; }

	// Feeds a key through the player's real input stack (mapping context, modifiers), for PIE proof of the gamepad mapping.
	UFUNCTION(BlueprintCallable, Category = "Jam|Input")
	static void DebugInjectKey(APlayerController* PC, FKey Key, float Value, bool bGamepad);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void TryBind();
	void OnMove(const FInputActionValue& V);
	void OnMoveDone(const FInputActionValue& V);
	void OnShootStarted() { bShoot = true; bShootLatch = true; }
	void OnShootDone() { bShoot = false; }
	void OnCheckStarted() { bCheck = true; bCheckLatch = true; }
	void OnCheckDone() { bCheck = false; }
	void OnRestartStarted() { OnRestart.Broadcast(); }
	void OnPauseStarted() { OnPause.Broadcast(); }

	FVector2D Axis = FVector2D::ZeroVector;
	bool bShoot = false;
	bool bCheck = false;
	bool bShootLatch = false;
	bool bCheckLatch = false;
	bool bBound = false;
	TWeakObjectPtr<APlayerController> BoundPC;
};
