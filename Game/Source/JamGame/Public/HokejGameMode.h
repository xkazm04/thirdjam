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
	UFUNCTION(BlueprintPure, Category = "Jam|Game") UJamCartridge* GetCartridge() const { return Cartridge; }
	UFUNCTION(BlueprintPure, Category = "Jam|Game") AJamSkater* GetHumanSkater() const;

	// Test hook: when set, the human's input comes from here instead of the keyboard/gamepad (PIE proof through MCP).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Game") bool bScriptedHuman = false;
	// Console: jam.ScriptHuman 1 makes the human skate to the puck and shoot (same rule as the ScoreOnEmptyNet test).

	// Mean and worst wall time of one UJamSim::Step in milliseconds since the match started (the report's sim frame time).
	UFUNCTION(BlueprintPure, Category = "Jam|Game") float GetMeanStepMs() const { return Steps ? static_cast<float>(StepMsSum / Steps) : 0.f; }
	UFUNCTION(BlueprintPure, Category = "Jam|Game") float GetWorstStepMs() const { return static_cast<float>(StepMsMax); }

private:
	void BuildMatch();
	void StepOnce();
	void GatherInputs();
	void DeliverEvents();
	void PushPresentation();
	void PossessHuman(APlayerController* PC);
	int32 ComputeTake() const;
	void UpdateTake(bool bForce);
	UClass* ResolveVisualClass(FName Visual) const;
	FJamInput ReadHumanInput();
	FJamInput ScriptedHumanInput() const;

	UFUNCTION() void HandleRestartPressed();

	UPROPERTY(Transient) TObjectPtr<UJamCartridge> Cartridge;
	UPROPERTY(Transient) TObjectPtr<UJamSim> Sim;
	UPROPERTY(Transient) TObjectPtr<ARinkActor> Rink;
	UPROPERTY(Transient) TArray<TObjectPtr<AJamSkater>> Skaters;
	UPROPERTY(Transient) TObjectPtr<AJamPuck> Puck;
	UPROPERTY(Transient) TObjectPtr<UJamBrain> Brain;
	UPROPERTY(Transient) TObjectPtr<UJamHudWidget> Hud;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Game") TObjectPtr<UJamEffectsBinder> EffectsBinder;

	TArray<FJamInput> Inputs;
	FString CutName = TEXT("hockey.regular");
	double Accumulator = 0.0;
	double StepMsSum = 0.0;
	double StepMsMax = 0.0;
	int64 Steps = 0;
	bool bBuilt = false;
	int32 Take = 0;
};
