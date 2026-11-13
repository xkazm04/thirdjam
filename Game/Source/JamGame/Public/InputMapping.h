// Enhanced Input for Hokej, built in C++ (no editor assets). Lane A5; the operator's mapping is lane A49 (docs/experiments/CONTROLS.md):
//   mouse aims the stick, left button shoots (hold to charge), right button body-checks, middle / mouse 4 cross-checks (the lift), wheel or Q dekes;
//   WASD skate, Shift sprint, Space poke, E pass, Ctrl brake, R restart, Esc pause; gamepad: sticks, RT shoot, RB check, LB lift, LT sprint, A poke, X pass, Y deke, B brake.
// A pawn adds a UJamPlayerInput component and reads Axis / IsShootHeld / ... each simulation step.
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
	UPROPERTY() TObjectPtr<UInputAction> Brake;    // bool, held: Ctrl / gamepad B
	// Lane A49: IA_Sprint, held: Shift / left trigger (+game.skater.sprint.bonus speed while the stamina lasts).
	UPROPERTY() TObjectPtr<UInputAction> Sprint;
	// R-0013 (lane A16): the stick. IA_StickAim: right stick, absolute; IA_StickMouse: mouse delta, accumulated (the fallback when the cursor is hidden);
	// IA_Poke: Space / A; IA_Lift (the cross-check): middle mouse, mouse 4 / LB. Lane A49 moved shoot to the left button / RT and check to the right button / RB.
	UPROPERTY() TObjectPtr<UInputAction> StickAim;
	UPROPERTY() TObjectPtr<UInputAction> StickMouse;
	UPROPERTY() TObjectPtr<UInputAction> Poke;
	UPROPERTY() TObjectPtr<UInputAction> Lift;
	// R-0029 point 4 (lane A39): IA_Deke, Q / gamepad Y: the carrier's deke tap (FJamInput::bDeke).
	UPROPERTY() TObjectPtr<UInputAction> Deke;
	// C-0005 (lane A16): IA_Pass, F / X (lane A51, R-0034: the pass moved from E to F): a lead pass to the best open teammate.
	UPROPERTY() TObjectPtr<UInputAction> Pass;
	// Lane A51 (R-0034): IA_Spin, E / gamepad B: the spin move (FJamInput::bSpin). The brake left B for the left stick click.
	UPROPERTY() TObjectPtr<UInputAction> Spin;
	UPROPERTY() TObjectPtr<UInputMappingContext> Context;

	// Keys mapped to an action, for tests ("is the gamepad A button on Shoot").
	bool IsKeyMapped(const UInputAction* Action, const FKey& Key) const;

	// The binding table as text, one row per action ("SHOOT  left mouse / RT"), for the controls strip, the pause card and CONTROLS.md. Lane A49.
	static FString BindingTableText(bool bOneLine);
	// jam.ctrl.Log 1: the per-frame JAMCTRL / JAMAIM proof lines are on.
	static bool LogEnabled();

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

	// Lane A77 (R-0036): pass (F / X) held, a level like the shot: the sim charges the saucer while it is held and passes on the release.
	UFUNCTION(BlueprintPure, Category = "Jam|Input")
	bool IsPassHeld() const { return bPassHeld; }

	// Held while Ctrl or the left trigger is down; the game mode copies it into FJamInput::bBrake.
	UFUNCTION(BlueprintPure, Category = "Jam|Input")
	bool IsBrakeHeld() const { return bBrake; }

	// Lane A49: held while Shift or the left trigger is down; the game mode copies it into FJamInput::bSprint.
	UFUNCTION(BlueprintPure, Category = "Jam|Input")
	bool IsSprintHeld() const { return bSprint; }

	// Clears the accumulated mouse delta (the game mode does it when the cursor aim takes over, so a later fallback starts from zero).
	void ResetMouseAim() { MouseAimPx = FVector2D::ZeroVector; }


	// R-0013 stick aim: right stick in [-1, 1]^2 (X right, Y forward), zero when idle.
	UFUNCTION(BlueprintPure, Category = "Jam|Input")
	FVector2D GetStickAxis() const { return StickAxis; }

	// Accumulated mouse delta in pixels (X right, Y up), clamped to a 200 px radius; the game mode scales it by game.stick.mouse_sensitivity.
	UFUNCTION(BlueprintPure, Category = "Jam|Input")
	FVector2D GetMouseAimPx() const { return MouseAimPx; }

	UFUNCTION(BlueprintCallable, Category = "Jam|Input")
	bool ConsumePokePressed();
	// Lane A51: the spin tap (E / B) since the last read.
	UFUNCTION(BlueprintCallable, Category = "Jam|Input")
	bool ConsumeSpinPressed();

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
	void OnSprintStarted() { bSprint = true; }
	void OnSprintDone() { bSprint = false; }
	void OnStickAim(const FInputActionValue& V);
	void OnStickAimDone(const FInputActionValue& V);
	void OnStickMouse(const FInputActionValue& V);
	void OnPokeStarted() { bPokeLatch = true; }
	void OnLiftStarted() { bLiftLatch = true; }
	void OnDekeStarted() { bDekeLatch = true; }
	void OnPassStarted() { bPassLatch = true; bPassHeld = true; }
	void OnPassDone() { bPassHeld = false; }   // lane A77
	void OnSpinStarted() { bSpinLatch = true; }   // lane A51
	void OnRestartStarted() { OnRestart.Broadcast(); }
	void OnPauseStarted() { OnPause.Broadcast(); }

	FVector2D Axis = FVector2D::ZeroVector;
	bool bShoot = false;
	bool bCheck = false;
	bool bBrake = false;
	bool bSprint = false;
	bool bShootLatch = false;
	bool bCheckLatch = false;
	bool bPokeLatch = false;
	bool bLiftLatch = false;
	bool bDekeLatch = false;
	bool bPassLatch = false;
	bool bPassHeld = false;   // lane A77 (R-0036)
	bool bSpinLatch = false;   // lane A51
	FVector2D StickAxis = FVector2D::ZeroVector;
	FVector2D MouseAimPx = FVector2D::ZeroVector;
	bool bBound = false;
	TWeakObjectPtr<APlayerController> BoundPC;
};
