// The game mode: loads the cartridge named by -cut=<name> (or the console variable jam.Cut, default hockey.regular), builds the
// rink, spawns one AJamSkater per cartridge skater and the puck, possesses the human's skater, and runs UJamSim at a fixed 60 Hz
// from an accumulator (at most 4 steps per render frame). After each step it delivers the sim's events to UJamEventBus, drives
// the skater visuals and pushes one FJamPresentationFrame (lane A5's camera and HUD). R restarts. Bots: lane A2's UJamBrain.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "JamSim.h"
#include "HokejGameMode.generated.h"

class UJamCartridge;
class ARinkActor;
class AJamSkater;
class AJamPuck;
class UJamBrain;
class UJamEffectsBinder;
class UJamRuleSystem;
class UJamZoneSystem;
class UJamEffectSystem;
class AJamPickupActor;
class UJamHudWidget;

JAMGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogHokej, Log, All);

UCLASS()
class JAMGAME_API AHokejGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHokejGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// Loads a cartridge by name or path and rebuilds the match (rink, skaters, sim). Returns false and keeps the
	// current match when the cartridge is rejected; the errors are logged as JAMCART lines.
	UFUNCTION(BlueprintCallable, Category = "Jam|Game")
	bool LoadCut(const FString& NameOrPath);

	// R-0003 3: a Director's Call or a Designer cut applied to the running match without a reset. Live-changeable tunables
	// (skater, shot, puck, ice friction, board restitution, goal pause, ai, pickups, look) apply at once; rink geometry, teams
	// and the rules mode are held back (logged) and take effect at the next LoadCut. Called by UJamDesignerSubsystem by name.
	UFUNCTION(BlueprintCallable, Category = "Jam|Game")
	bool ApplyCartridgeLive(UJamCartridge* NewCartridge);

	// Current TAKE (period), 1-based: thirds of timer_frames in timer mode, 1 + the leading score in first_to mode.
	UFUNCTION(BlueprintPure, Category = "Jam|Game") int32 GetTake() const { return Take; }

	// New match, same cartridge and seed.
	UFUNCTION(BlueprintCallable, Category = "Jam|Game")
	void RestartMatch();

	UFUNCTION(BlueprintPure, Category = "Jam|Game") UJamSim* GetSim() const { return Sim; }
	// Lane A14 (C-0005): the effect system (modifiers, triggers, pickups); its HUD rows are GetEffects()->GetHudFeedFor(human).
	UFUNCTION(BlueprintPure, Category = "Jam|Game") UJamEffectSystem* GetEffects() const { return Effects; }
	UFUNCTION(BlueprintPure, Category = "Jam|Game") UJamCartridge* GetCartridge() const { return Cartridge; }
	UFUNCTION(BlueprintPure, Category = "Jam|Game") AJamSkater* GetHumanSkater() const;
	// Lane A15 (C-0005): the rule hook (announce text for the RULE: scoreboard line) and the zone system (membership, zone events).
	UFUNCTION(BlueprintPure, Category = "Jam|Game") UJamRuleSystem* GetRules() const { return Rules; }
	UFUNCTION(BlueprintPure, Category = "Jam|Game") UJamZoneSystem* GetZones() const { return ZoneSystem; }

	// Test hook: when set, the human's input comes from here instead of the keyboard/gamepad (PIE proof through MCP).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Game") bool bScriptedHuman = false;
	// Console: jam.ScriptHuman 1 makes the human skate to the puck and shoot (same rule as the ScoreOnEmptyNet test).

	// Mean and worst wall time of one UJamSim::Step in milliseconds since the match started (the report's sim frame time).
	UFUNCTION(BlueprintPure, Category = "Jam|Game") float GetMeanStepMs() const { return Steps ? static_cast<float>(StepMsSum / Steps) : 0.f; }
	UFUNCTION(BlueprintPure, Category = "Jam|Game") float GetWorstStepMs() const { return static_cast<float>(StepMsMax); }

	// Lane A65: the Blueprint figure classes ResolveVisualClass tries for game.skater.visual and the tier, in order (empty for the C++ figures). The menu's
	// preload (UJamPreloadSubsystem) loads the same paths asynchronously, so the match start finds the class in memory.
	static TArray<FString> VisualClassPathsFor(FName Visual, const FString& Tier);

private:
	void BuildMatch();
	void StepOnce();
	void GatherInputs();
	void DeliverEvents();
	void PushPresentation();
	void PossessHuman(APlayerController* PC);
	int32 ComputeTake() const;
	void UpdateTake(bool bForce);
	// Lane A31 (B-to-A-0021): the visual class for game.skater.visual, and for generated_b2 by game.look.tier (p3_clay -> BP_SkaterVisual_TB2_P3,
	// p4_broadcast -> _TB2_P4, else _TB2; a failed load falls to _TB2, then to the toy figure).
	UClass* ResolveVisualClass(FName Visual, const FString& Tier) const;
	// B-to-A-0021 point 2: fixed jersey numbers per slot so a replay shows the same player: team 0 gets 7, 13, 42; team 1 gets 9, 21, 88 (a fourth
	// skater and beyond keep the old team * 10 + slot + 1).
	static int32 JerseyNumberFor(int32 Team, int32 SlotInTeam);
	// A live tier change that resolves to another figure class respawns every skater's visual with the setup calls repeated (B-to-A-0021 point 1).
	void RespawnVisualsIfClassChanged(const FJamGameConfig& G);
	FJamInput ReadHumanInput();
	FJamInput ScriptedHumanInput();
	// Scripted human state (R-0013 / C-0005 proof): frames shoot has been held, last frame a pass went out.
	int32 ScriptCharge = 0;
	int64 ScriptLastPass = -600;

	// Lane A49 (complaint 1): the match's input mode. The menu leaves the viewport in game-and-UI with capture only while a button is down, so the mouse delta never
	// reached IA_StickMouse in the package; the match sets its own mode (cursor shown, locked to the viewport) and re-applies it after the pause menu's Resume.
	void EnsureMatchInputMode(APlayerController* PC);
	bool bMatchInputModeSet = false;
	// Lane A49: the stick aim from the cursor: the cursor deprojected onto the ice, the vector from the skater to it. False when the cursor is hidden or off the ice;
	// zero inside the dead zone (0.5 m in, 0.7 m out), |Stick| 0.5..1 over 0.5..3 m. The accumulated mouse delta stays as the fallback (right stick first).
	bool CursorAimRink(APlayerController* PC, const FVector2D& SkaterPosM, FVector2D& OutStick);
	bool bCursorAiming = false;
	// Lane A49: the button edges logged once per press (JAMCTRL btn=...), the sprint level and brake level on change.
	bool bLastSprint = false, bLastBrake = false, bLastShootHeld = false;
	// Lane A49 (complaint 5 proof, jam.ctrl.Drill 1 = tackle, 2 = cross-check): a scripted FJamInput that mirrors WASD + one right (middle) mouse press: chase the
	// opposing carrier at full stick, press once within range, then read the sim's own events for 20 frames; 10 attempts, a JAMDRILL line each and a summary.
	FJamInput DrillInput();
	void DrillObserve();
	int64 DrillPressFrame = -1, DrillCooldownUntil = 0;
	int32 DrillVictim = -1, DrillAttempts = 0, DrillLanded = 0, DrillStripped = 0, DrillKnockdowns = 0, DrillVisualDown = 0, DrillLifted = 0;
	bool bDrillLandedThis = false, bDrillStrippedThis = false, bDrillKnockThis = false, bDrillVisualThis = false, bDrillLiftThis = false;
	// Lane A51 (R-0034): the 1v1 stick-handling drill (jam.ctrl.Drill 3 = sweep, 4 = spin). The cursor and the keys stay real (ReadHumanInput: the mouse sweeps,
	// E spins); only the skating is scripted (carry at the nearest opponent, chase a loose puck). Each human sweep / spin the sim reports opens an attempt;
	// 30 frames later the puck kept, an opponent's clean poke, a knock-down (and its frames) are logged as JAMDRILL stick lines, with a summary after 10.
	FJamInput StickDrillInput();
	void StickDrillObserve();
	int64 StickDrillFrame = -1;
	int32 StickDrillKind = 0, StickDrillAttempts = 0, StickDrillKept = 0, StickDrillPoked = 0, StickDrillKnocked = 0, StickDrillKnockFrames = 0;
	bool bStickDrillPokedThis = false, bStickDrillKnockThis = false;
	int32 StickDrillKnockFramesThis = 0;

	UFUNCTION() void HandleRestartPressed();

	UPROPERTY(Transient) TObjectPtr<UJamCartridge> Cartridge;
	UPROPERTY(Transient) TObjectPtr<UJamSim> Sim;
	UPROPERTY(Transient) TObjectPtr<ARinkActor> Rink;
	UPROPERTY(Transient) TArray<TObjectPtr<AJamSkater>> Skaters;
	UPROPERTY(Transient) TObjectPtr<AJamPuck> Puck;
	UPROPERTY(Transient) TObjectPtr<UJamBrain> Brain;
	UPROPERTY(Transient) TObjectPtr<UJamEffectSystem> Effects;
	UPROPERTY(Transient) TArray<TObjectPtr<AJamPickupActor>> PickupActors;
	TArray<FName> LastBoost;
	// Lane A31 (B-to-A-0024 point 3): the goalie stance last sent per skater: -2 off, else the side (-1, 0, +1); SetGoalieStance goes out on a change.
	TArray<int8> LastGoalie;
	TArray<int64> GoalieOnFrame;   // the frame the stance last came on (minimum hold)
	// Lane A31: the class the skater visuals were spawned with (a live tier change compares against it).
	UPROPERTY(Transient) TObjectPtr<UClass> SpawnedVisualClass;
	UPROPERTY(Transient) TObjectPtr<UJamHudWidget> Hud;
	UPROPERTY(Transient) TObjectPtr<UJamRuleSystem> Rules;
	UPROPERTY(Transient) TObjectPtr<UJamZoneSystem> ZoneSystem;
	// Lane A15: UJamSim::GetRinkRevision the rink actor was last built for (ShrinkRink / GrowRink rebuild it at the next faceoff).
	int32 BuiltRinkRevision = 0;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Game") TObjectPtr<UJamEffectsBinder> EffectsBinder;

	TArray<FJamInput> Inputs;
	FString CutName = TEXT("hockey.regular");
	double Accumulator = 0.0;
	double StepMsSum = 0.0;
	double StepMsMax = 0.0;
	int64 Steps = 0;
	bool bBuilt = false;
	int32 Take = 0;
	// Lane A17: the finished match was reported to UJamProgression (reset by RestartMatch and BuildMatch).
	bool bResultReported = false;
};
