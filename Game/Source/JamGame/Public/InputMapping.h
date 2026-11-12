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
	UPROPERTY() TObjectPtr<UInputAction> Brake;    // bool, held: Ctrl / left trigger
	// R-0013 (lane A16): the stick. IA_StickAim: right stick, absolute; IA_StickMouse: mouse delta, accumulated; IA_Poke: left mouse / RB; IA_Lift: right mouse / LB.
	UPROPERTY() TObjectPtr<UInputAction> StickAim;
	UPROPERTY() TObjectPtr<UInputAction> StickMouse;
	UPROPERTY() TObjectPtr<UInputAction> Poke;
	UPROPERTY() TObjectPtr<UInputAction> Lift;
	// R-0029 point 4 (lane A39): IA_Deke, Q / gamepad Y: the carrier's deke tap (FJamInput::bDeke).
	UPROPERTY() TObjectPtr<UInputAction> Deke;
	// C-0005 (lane A16): IA_Pass, E / X: a lead pass to the best open teammate.
	UPROPERTY() TObjectPtr<UInputAction> Pass;
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

	// Held while Ctrl or the left trigger is down; the game mode copies it into FJamInput::bBrake.
	UFUNCTION(BlueprintPure, Category = "Jam|Input")
	bool IsBrakeHeld() const { return bBrake; }


	// R-0013 stick aim: right stick in [-1, 1]^2 (X right, Y forward), zero when idle.
	UFUNCTION(BlueprintPure, Category = "Jam|Input")
	FVector2D GetStickAxis() const { return StickAxis; }

	// Accumulated mouse delta in pixels (X right, Y up), clamped to a 200 px radius; the game mode scales it by game.stick.mouse_sensitivity.
	UFUNCTION(BlueprintPure, Category = "Jam|Input")
	FVector2D GetMouseAimPx() const { return MouseAimPx; }

	UFUNCTION(BlueprintCallable, Category = "Jam|Input")
	bool ConsumePokePressed();

	UFUNCTION(BlueprintCallable, Category = "Jam|Input")
	bool ConsumeLiftPressed();

	UFUNCTION(BlueprintCallable, Category = "Jam|Input")
	bool ConsumeDekePressed();

	UFUNCTION(BlueprintCallable, Category = "Jam|Input")
	bool ConsumePassPressed();

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
	void OnBrakeStarted() { bBrake = true; }
	void OnBrakeDone() { bBrake = false; }
	void OnStickAim(const FInputActionValue& V);
	void OnStickAimDone(const FInputActionValue& V);
	void OnStickMouse(const FInputActionValue& V);
	void OnPokeStarted() { bPokeLatch = true; }
	void OnLiftStarted() { bLiftLatch = true; }
	void OnDekeStarted() { bDekeLatch = true; }
	void OnPassStarted() { bPassLatch = true; }
	void OnRestartStarted() { OnRestart.Broadcast(); }
	void OnPauseStarted() { OnPause.Broadcast(); }

	FVector2D Axis = FVector2D::ZeroVector;
	bool bShoot = false;
	bool bCheck = false;
	bool bBrake = false;
	bool bShootLatch = false;
	bool bCheckLatch = false;
	bool bPokeLatch = false;
	bool bLiftLatch = false;
	bool bDekeLatch = false;
	bool bPassLatch = false;
	FVector2D StickAxis = FVector2D::ZeroVector;
	FVector2D MouseAimPx = FVector2D::ZeroVector;
	bool bBound = false;
	TWeakObjectPtr<APlayerController> BoundPC;
};
