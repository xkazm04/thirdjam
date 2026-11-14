// Enhanced Input for Hokej, built in C++ (no editor assets). Lane A5; the operator's mapping is lane A49 (docs/experiments/CONTROLS.md):
//   mouse aims the stick, left button shoots (hold to charge), right button body-checks, middle / mouse 4 cross-checks (the lift), wheel or Q dekes;
//   WASD skate, Shift sprint, Space poke, E pass, Ctrl brake, R restart, Esc pause; gamepad: sticks, RT shoot, RB check, LB lift, LT sprint, A poke, X pass, Y deke, B brake.
// A pawn adds a UJamPlayerInput component and reads Axis / IsShootHeld / ... each simulation step.
// Lane A183 (R-0064 point 1, the operator's hands): the bindings are data (FJamBindings: every verb's keys and pad buttons, rebindable, saved with the
// profile). LMB / RT: shoot with the puck, cross-check without (held = FJamInput::bCrossHold); RMB / RB: pass with the puck (held = saucer), body check
// without. "With the puck" is read from the sim at the step that takes the press (FJamButtonResolver), before the FJamInput is recorded or sent.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "InputMapping.generated.h"

class UInputAction;
class UInputMappingContext;
class UEnhancedInputComponent;
class APlayerController;
class UJamSim;
class UJamPlayerInput;
struct FJamInput;
struct FJamControlRow;

// ---------------------------------------------------------------- lane A183: the bindings as data

// Every verb a player can rebind. Pause (Esc / Start) is fixed: Esc cancels a listening row, so it can never be lost.
enum class EJamVerb : uint8
{
	Shoot,          // with the puck (the two-role button's first role)
	CrossCheck,     // without the puck (the sim's bLift); held = bCrossHold
	Pass,           // with the puck; held = saucer
	Check,          // without the puck: the body check
	Poke, Deke, Spin, Ability, Sprint, Brake,
	SkateForward, SkateBack, SkateLeft, SkateRight,
	Restart,
	Count
};

struct JAMGAME_API FJamVerbBinding
{
	TArray<FKey> Keys;   // mouse and keyboard; [0] is the one the CONTROLS screen rebinds, the rest are second bindings
	TArray<FKey> Pads;   // gamepad; [0] is rebound the same way
	bool operator==(const FJamVerbBinding& O) const { return Keys == O.Keys && Pads == O.Pads; }
};

struct JAMGAME_API FJamBindings
{
	static constexpr int32 NumVerbs = static_cast<int32>(EJamVerb::Count);
	FJamVerbBinding Verbs[NumVerbs];

	FJamVerbBinding& operator[](EJamVerb V) { return Verbs[static_cast<int32>(V)]; }
	const FJamVerbBinding& operator[](EJamVerb V) const { return Verbs[static_cast<int32>(V)]; }
	bool operator==(const FJamBindings& O) const;

	// The shipped defaults (A183): the table in docs/experiments/CONTROLS-2.md.
	static FJamBindings Defaults();
	// 1 = a with-the-puck verb (shoot, pass), 2 = a without-the-puck verb (cross-check, check), 0 = a plain verb.
	static int32 Role(EJamVerb V);
	// Two verbs may share a key only as one with-verb and one without-verb (LMB: shoot / cross-check).
	static bool MayShare(EJamVerb A, EJamVerb B);
	// "SHOOT", "CROSS-CHECK", ... and the saved form's key name ("Shoot", "CrossCheck", ...).
	static FString VerbLabel(EJamVerb V);
	static const TCHAR* VerbId(EJamVerb V);
	// Keys reserved by the game (Esc, the mouse and stick axes, the signal digits 1-4 and the D-pad): never accepted by Rebind.
	static bool IsReserved(const FKey& Key);

	bool Has(EJamVerb V, const FKey& Key) const;
	// The verbs a key is bound to (any slot, keys and pads alike).
	TArray<EJamVerb> VerbsOn(const FKey& Key) const;
	// The with-verb / without-verb a key is bound to (EJamVerb::Count = none).
	EJamVerb WithVerbOn(const FKey& Key) const;
	EJamVerb WithoutVerbOn(const FKey& Key) const;
	// Puts Key in the verb's first keyboard slot (or first pad slot when the key is a gamepad key). A verb that held Key and may not share it with V
	// gets V's old first key in its place (a swap); OutSwapped names that verb (EJamVerb::Count = no swap). False: a reserved key or no change.
	bool Rebind(EJamVerb V, const FKey& Key, EJamVerb* OutSwapped = nullptr);
	// No key on two verbs that may not share it. OutWhy names the first clash.
	bool IsValid(FString* OutWhy = nullptr) const;

	// The saved form: one entry per verb id, "Keys|Pads" with comma-separated key names ("LeftMouseButton,MiddleMouseButton|Gamepad_RightTriggerAxis").
	TMap<FString, FString> ToSaved() const;
	// Unknown verbs and keys are dropped; a verb with no saved entry keeps its default; a clash falls back to the defaults whole.
	static FJamBindings FromSaved(const TMap<FString, FString>& Saved);
};

// The two-role buttons (A183). Press / Release come from Enhanced Input between sim steps; Step runs once per sim step with the possession of that frame
// and turns each new press into its verb: a key on a with-verb and a without-verb takes the first when bWithPuck, the second otherwise; a key on one of
// them only always does that one. The verb holds until the key is released, whatever happens to the puck. A press released before its first step still
// counts for that step (a tap).
struct JAMGAME_API FJamButtonVerbs
{
	bool bShootHeld = false;   // level
	bool bPassHeld = false;    // level (the saucer charges while held)
	bool bCheck = false;       // edge
	bool bLift = false;        // edge: the cross-check
	bool bCrossHeld = false;   // level: a cross-check press still held (ApplyButtons gates it by "not carrying" into bCrossHold)
	bool bShootPress = false;  // edge, for the JAMCTRL line
	bool bPassPress = false;   // edge
};

struct JAMGAME_API FJamButtonResolver
{
	void Press(const FKey& Key);
	void Release(const FKey& Key);
	FJamButtonVerbs Step(bool bWithPuck, const FJamBindings& B);
	void Reset() { Held.Reset(); }
	// The verb a held key is doing now (EJamVerb::Count = none / not yet resolved).
	EJamVerb ActiveVerb(const FKey& Key) const;

private:
	struct FHeldKey { FKey Key; EJamVerb Verb = EJamVerb::Count; bool bDown = false; bool bNew = false; };
	TArray<FHeldKey> Held;
};

// Lane A191 (the camera lane's finding A186, R-0064's first weak point): a still hand keeps its point on the ice. The blade aims at the cursor's ice point,
// re-projected through the camera every sim step, so a camera correction swings the blade under a still mouse. While the cursor's pixel has not moved
// (under half a pixel) and the skater stands (under jam.ctrl.AimAnchorSpeed m/s), the aim keeps its offset from the skater; otherwise it re-projects.
// Measured (tools/qa/aim/a191-aim.py, status/A191.md): the still-cursor turn through corrections 17.1 -> 0 deg worst, the drift from the cursor's true
// point in play p90 0 deg (worst 5.5 deg at 2 s); an always-on anchor or a catch-up drifts p90 25 to 140 deg in open ice (the camera leads the skater).
struct JAMGAME_API FJamAimAnchor
{
	// The ice point to aim at (metres). PixelPx: the cursor's pixel; HitM: its ice point through the camera now; SkaterM: the skater now. Call once a sim step.
	FVector2D Step(const FVector2D& PixelPx, const FVector2D& HitM, const FVector2D& SkaterM, bool bOn, double StandSpeedMps);
	// The same with jam.ctrl.AimAnchor and jam.ctrl.AimAnchorSpeed.
	FVector2D StepLive(const FVector2D& PixelPx, const FVector2D& HitM, const FVector2D& SkaterM);
	void Reset() { bHave = false; bAnchored = false; }
	bool IsAnchored() const { return bAnchored; }

private:
	bool bHave = false;
	bool bAnchored = false;
	FVector2D LastPx = FVector2D::ZeroVector;
	FVector2D LastSkater = FVector2D::ZeroVector;
	FVector2D Offset = FVector2D::ZeroVector;
};

// A184 (the contact lane) adds FJamInput::bCrossHold. THE DIRECTOR FLIPS THIS ONE LINE when A184's field is on main: 0 -> 1. It turns on the write of
// the level (JamControls::SetCrossHold), the acceptance drill's held cross-check row (step 7 k=3) and its test (Jam.Controls.HeldCrossCheck).
#define JAM_INPUT_HAS_CROSS_HOLD 1

namespace JamControls
{
	// "With the puck" for the two-role buttons: the skater carries it, or (jam.ctrl.OurPuckShoots 1, the default) the puck is loose and was last
	// touched by his own team (our pass in flight: the one-timer's press comes before the reception).
	JAMGAME_API bool HasPuckForButtons(const UJamSim& Sim, int32 Skater);
	// The contact lane's level (A184, FJamInput::bCrossHold): true every frame the cross-check button is held while the skater does not carry.
	JAMGAME_API void SetCrossHold(FJamInput& In, bool bHeld);
	// The one call both game modes make: steps the resolver with the sim's possession and writes shoot, pass, check, the cross-check and its hold into In.
	JAMGAME_API FJamButtonVerbs ApplyButtons(UJamPlayerInput& Pi, const UJamSim& Sim, int32 Skater, FJamInput& In);
}

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
	// R-0029 point 4 (lane A39): IA_Deke, mouse wheel / gamepad Y: the carrier's deke tap (FJamInput::bDeke). Lane A84: Q left it for the pass.
	UPROPERTY() TObjectPtr<UInputAction> Deke;
	// C-0005 (lane A16): IA_Pass, Q / X (lane A51 moved it from E to F, lane A84 from F to Q, R-0041 point 3): a lead pass; held, a saucer.
	UPROPERTY() TObjectPtr<UInputAction> Pass;
	// Lane A51 (R-0034): IA_Spin, E / gamepad B: the spin move (FJamInput::bSpin). The brake left B for the left stick click.
	UPROPERTY() TObjectPtr<UInputAction> Spin;
	// Lane A84 (R-0041 point 3): IA_Ability, F / gamepad right stick click (R3): the held ability (FJamInput::bAbility).
	UPROPERTY() TObjectPtr<UInputAction> Ability;
	UPROPERTY() TObjectPtr<UInputMappingContext> Context;
	// Lane A183: the two-role buttons arrive per key: every key bound to shoot, cross-check, pass or check is mapped to one of these slot actions
	// (SlotKeys[i] is its key), so the component knows which key went down and resolves it by possession at the step.
	static constexpr int32 NumButtonSlots = 24;
	UPROPERTY() TArray<TObjectPtr<UInputAction>> ButtonSlots;   // NumButtonSlots of them
	TArray<FKey> SlotKeys;                                         // the key on each slot (invalid = unused)

	// Keys mapped to an action, for tests ("is the gamepad A button on Shoot"). Shoot, Lift (the cross-check), Pass and Check answer from the bindings.
	bool IsKeyMapped(const UInputAction* Action, const FKey& Key) const;

	// Lane A183: the live bindings. Apply rebuilds the mapping context (every local player's input picks it up at once).
	const FJamBindings& GetBindings() const { return Bindings; }
	void ApplyBindings(const FJamBindings& In);

	// ---- The words from the bindings (A183, R-0064 point 1: "shown in the hint strip and the controls card from the bindings, never from fixed text").
	// A key's short name for the strip and the cards: LMB, RMB, MMB, MOUSE4, WHEEL, SPACE, CTRL, RT, LB, ... (at most KeyNameMax characters).
	static constexpr int32 KeyNameMax = 12;
	static FString KeyName(const FKey& Key);
	// The verb's keys, short: "LMB, MMB, MOUSE4" or "RT, LB" (bPad). The wheel's two directions read as one WHEEL.
	static FString VerbKeysText(EJamVerb V, bool bPad);
	// The HUD's first hint strip (UJamHudWidget::ControlsHintText), at most HintStripMaxChars characters with any binding.
	static constexpr int32 HintStripMaxChars = 145;
	static FString HintStripText();
	// The pause card's five lines (UJamDirectorsCallWidget::PauseControlsText).
	static FString PauseCardText();
	// The CONTROLS screen's rows (UJamMenuFlow::ControlRows): the two-role buttons as two rows each.
	static TArray<FJamControlRow> ControlRowsFromBindings();
	// The binding table as text, one row per verb ("SHOOT (WITH THE PUCK)  LMB / RT"), for the pause card's test and CONTROLS-2.md. Lane A49; A183 from the bindings.
	static FString BindingTableText(bool bOneLine);
	// jam.ctrl.Log 1: the per-frame JAMCTRL / JAMAIM proof lines are on.
	static bool LogEnabled();

private:
	void Build();
	void MapAll();
	FJamBindings Bindings;
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
	// Lane A84: the ability tap (F / R3) since the last read.
	UFUNCTION(BlueprintCallable, Category = "Jam|Input")
	bool ConsumeAbilityPressed();

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

	// Lane A183: the two-role buttons, stepped once per sim step with the possession of that frame (JamControls::ApplyButtons calls it).
	FJamButtonVerbs StepButtons(bool bWithPuck);
	// Tests: a bound key's press as Enhanced Input reports it (the slot action's Started / Completed).
	void DebugButton(const FKey& Key, bool bDown);

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
	void OnAbilityStarted() { bAbilityLatch = true; }   // lane A84
	void OnSlotStarted(int32 Slot);   // lane A183
	void OnSlotDone(int32 Slot);
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
	bool bAbilityLatch = false;   // lane A84
	FVector2D StickAxis = FVector2D::ZeroVector;
	FVector2D MouseAimPx = FVector2D::ZeroVector;
	bool bBound = false;
	TWeakObjectPtr<APlayerController> BoundPC;
	FJamButtonResolver Buttons;   // lane A183
};
