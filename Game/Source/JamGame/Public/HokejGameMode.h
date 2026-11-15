// The game mode: loads the cartridge named by -cut=<name> (or the console variable jam.Cut, default hockey.regular), builds the
// rink, spawns one AJamSkater per cartridge skater and the puck, possesses the human's skater, and runs UJamSim at a fixed 60 Hz
// from an accumulator (at most 4 steps per render frame). After each step it delivers the sim's events to UJamEventBus, drives
// the skater visuals and pushes one FJamPresentationFrame (lane A5's camera and HUD). R restarts. Bots: lane A2's UJamBrain.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "JamSim.h"
#include "InputMapping.h"   // lane A191: FJamAimAnchor
#include "PuckRead.h"   // lane A210
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
class AJamBoostAura;
class AJamHitRead;
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

	// Lane A144 (the level wire, Director's order of 14 Nov; REVIEW-2: the menu's AI DIFFICULTY reached no match). The level this match plays is the
	// brain's profile name ("rookie" | "pro" | "all_star"). It is the PLAYER'S when the rink was opened with ?level=<x> for the cut it then loaded:
	// the menu does that for the regular cuts (PLAY NOW and the cut cards of CHOOSE YOUR CUT). It is the CARTRIDGE'S OWN for every other start: a
	// season take, a Director's mode, a saved Director's cut, a joined match (no setting crosses the wire), a tool's -cut= start, a cut loaded
	// later in the same world. Logged once a match: JAMMATCH level=<x> source=settings|cartridge.
	UFUNCTION(BlueprintPure, Category = "Jam|Game") FString GetLevelInForce() const;
	UFUNCTION(BlueprintPure, Category = "Jam|Game") bool IsLevelFromSettings() const { return bLevelFromSettings; }
	// "settings" (the player's level), "cartridge" (the cut's own), or "launch": the cut came from the -cut= launch argument (the acceptance run,
	// a tool), which plays the cartridge's own level whatever the settings say (Director, 14 Nov). The word after source= in the JAMMATCH line.
	UFUNCTION(BlueprintPure, Category = "Jam|Game") FString GetLevelSource() const;
	const UJamBrain* GetBrain() const { return Brain; }
	// "level=rookie" among a level's URL options -> "rookie"; empty when absent or no level (then the cartridge's own plays).
	static FString LevelFromOptions(const FString& Options);
	UFUNCTION(BlueprintPure, Category = "Jam|Game") AJamSkater* GetHumanSkater() const;
	// Lane A120 (track Q1): the seat this machine plays, for presentation only (barks, commentary): the sim's human here, ALobbyGameMode's LocalIndex online.
	virtual int32 GetLocalSeat() const;
	// Lane A129 (track Q1): true = this change to the sim (What: "a live cut", "a cut load", "a restart") is refused here (an online match: one machine's sim
	// must never change alone). Single player: false, nothing changes.
	virtual bool RefuseSimChange(const TCHAR* What) { return false; }
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
	// Lane A49: the stick aim from the cursor: the cursor deprojected onto the ice, the vector from the skater to it. False when the cursor is hidden or off the ice;
	// zero inside the dead zone (0.5 m in, 0.7 m out), |Stick| 0.5..1 over 0.5..3 m. The accumulated mouse delta stays as the fallback (right stick first).
	// Lane A110 (track Q1): protected, so the lobby's guest aims as single player does (A49's fix never reached ALobbyGameMode); the ice math is the public static
	// CursorAimFromRay (the ray from the camera through the cursor; bInOutAiming is the hysteresis state, this game mode's bCursorAiming).
public:
	static bool CursorAimFromRay(const FVector& RayOrigin, const FVector& RayDir, const FVector2D& SkaterPosM, bool& bInOutAiming, FVector2D& OutStick);
	// Lane A191: the same from the ice point (metres) the blade aims at (CursorAimFromRay's hit, or the still hand's anchored point).
	static bool CursorAimFromHit(const FVector2D& HitM, const FVector2D& SkaterPosM, bool& bInOutAiming, FVector2D& OutStick);
protected:
	void EnsureMatchInputMode(APlayerController* PC);   // lane A110: the lobby sets the same cursor mode (A49), or the cursor is hidden and never aims
	bool bMatchInputModeSet = false;
	bool CursorAimRink(APlayerController* PC, const FVector2D& SkaterPosM, FVector2D& OutStick);
	bool bCursorAiming = false;
	FJamAimAnchor AimAnchor;   // lane A191: a still hand keeps its point on the ice (jam.ctrl.AimAnchor)
private:
	// Lane A49: the button edges logged once per press (JAMCTRL btn=...), the sprint level and brake level on change.
	bool bLastSprint = false, bLastBrake = false, bLastShootHeld = false, bLastPassHeld = false;
	// Lane A93 (R-0045 law 3): while the move input pulls back (S / left stick down) the move frame is the camera's yaw at the moment the pull began
	// (ReadHumanInput): the third-person camera swings behind the turning skater, and a "back" read from the live camera would run away from it for ever.
	bool bBackYawHeld = false;
	double BackYawRad = 0.0;
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
	// Lane A77 (R-0036): the mechanics drills (jam.ctrl.Drill 7 = sweep past a defender, 8 = spin with the puck, 9 = saucer over a defender's blade, 10 = shots from
	// 10 m, 11 = one-timers off a scripted teammate's pass). The cursor and the keys stay the real input path; the skating is scripted and JAMCUE lines tell the driver
	// (tools/qa/ctrl/a77-prove.ps1) when a person would act. MechDrillObserve logs JAMDRILL saucer / shot / onetimer lines; MechDrillMate scripts the teammate of drill 11.
	FJamInput MechDrillInput();
	void MechDrillObserve();
	void MechDrillMate(TArray<FJamInput>& AllInputs);
	int64 MechCueReady = 0;
	bool bMechAtPre = false;
	int32 MechShotN = 0, MechShotTaps = 0, MechShotHalf = 0, MechShotFull = 0;
	int64 MechShotFrame = -1;
	float MechShotCharge = 0.f;
	double MechShotPrevZ = 0.0;
	bool bMechShotGoal = false, bMechShotSave = false, bMechShotPost = false;
	int64 MechSaucerFrame = -1, MechSaucerLand = -1;
	int32 MechSaucerN = 0, MechSaucerReceiver = -1, MechSaucerOver = 0, MechSaucerReceived = 0;
	bool bMechSaucerOverThis = false;
	double MechSaucerMinBladeAir = 1e9, MechSaucerZAtBlade = 0.0, MechSaucerMaxZ = 0.0, MechSaucerLandDist = -1.0;
	int64 MechMatePassFrame = -1, MechMateReady = 0;
	int32 MechOneTimers = 0, MechOtShots = 0;

	UFUNCTION() void HandleRestartPressed();

	// Lane A73 (B-to-A-0036 point 4): the hit-stop's routing (read-outs only; the sim never pauses). A knockdown by a check this frame that involves the human
	// (victim or checker) tells both pawns and the puck to hold jam.hitstop frames (AJamSkater::BeginHitStop), and the presentation frame (camera, HUD, radar) is not
	// pushed for the frames after the hit frame while they hold; the render alpha is 1 meanwhile so the camera sits on the hit frame's pose.
	void HitStopRoute();
	int64 HitStopStartFrame = -1, HitStopEndFrame = -1;
	// Lane A73 proof: jam.ctrl.Stills 1 captures a screenshot every 4 sim frames after a knockdown that involves the human (24 stills: fall, lying, get-up), after the
	// human's board hit and after a stick hit on the human (8 each), twice per kind at most; Saved/Screenshots/<platform>/a73-<kind>-<start>-f<frame>.png.
	void StillsObserve();
	FString StillKind;
	int64 StillStart = -1;
	int32 StillCount = 0, StillTaken = 0;
	TMap<FString, int32> StillsPerKind;
	// Lane A73 drill 6 (the human carries at the pro bots and is checked): knockdowns of the human and its get-ups, a summary after ten get-ups.
	void KdObserve();
	int32 KdCount = 0, KdGetUps = 0;
	int64 KdDownFrame = -1;
	bool bKdWasDown = false;
	// Lane A87 (R-0043 point 1): the acceptance run (jam.ctrl.Drill 20, HokejAcceptDrill.cpp): the human plays through the real input path; the drill stages each
	// step (the human, one mate, one opponent; the rest parked), tells the driver (tools/qa/accept/accept.ps1) where the cursor goes and what to press, and logs a
	// JAMACCEPT step=<n> verb=<v> ok=<0|1> line per verb. State lives in the .cpp (file-local).
	void AcceptDrillInputs(TArray<FJamInput>& AllInputs);
	void AcceptDrillObserve();

	UPROPERTY(Transient) TObjectPtr<UJamCartridge> Cartridge;
	UPROPERTY(Transient) TObjectPtr<UJamSim> Sim;
	UPROPERTY(Transient) TObjectPtr<ARinkActor> Rink;
	UPROPERTY(Transient) TArray<TObjectPtr<AJamSkater>> Skaters;
	UPROPERTY(Transient) TObjectPtr<AJamPuck> Puck;
	UPROPERTY(Transient) TObjectPtr<UJamBrain> Brain;
	// Lane A144: the level the rink's open asked for, the cut it asked it for, and whether the match in play uses it.
	FString RequestedLevel;
	FString RequestedForCut;
	bool bLevelFromSettings = false;
	bool bCutFromLaunch = false;
	UPROPERTY(Transient) TObjectPtr<UJamEffectSystem> Effects;
	UPROPERTY(Transient) TArray<TObjectPtr<AJamPickupActor>> PickupActors;
	// Lane A188: the equipped look, one per skater that has held an ability (AJamBoostAura, JamPickupActor.h).
	UPROPERTY(Transient) TArray<TObjectPtr<AJamBoostAura>> BoostAuras;
	UPROPERTY(Transient) TObjectPtr<AJamHitRead> HitRead;   // lane A196: the hits read (HitRead.h), made on the first sim step
	FJamPuckShow PuckShow;   // lane A210 (wave 4): the carrier the screen shows, the drawn puck, the play calls (PuckRead.h); presentation only
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

public:
	// ---- Lane A111 (track U1, B-to-A-0038 fault 3, R-0049 point 3): the fade-in waits for the lights. The light rig is a sub-level streamed in after the match
	// is built; in the first match of a process it took about 3 s, and the faceoff showed the rink without its lights. When a match is built and its tier's rig
	// is not visible yet, the picture is held black (the camera's fade; the MATCH card and the sound play on) until the rig's level is visible for
	// LightHoldSettleTicks rendered ticks, and never longer than LightHoldMaxSeconds of real time; then it fades in over jam.screen.FadeSeconds. The HUD is
	// hidden with it (no ring on a black screen). Presentation only: the sim steps on, nothing is written back, the hash cannot move.
	// A rig already visible (a second cut in the same world) holds nothing. jam.screen.LightHold 0: no hold, as before.
	// Measured in the first proof run: the rig's level took 3.5 s to stream in, time-sliced behind the first frames, so the 2 s bound let the bare rink show for
	// 1.5 s. jam.screen.LightHold 2 (default) therefore also finishes the rig's load inside BuildMatch (UWorld::FlushLevelStreaming: one block before the first
	// frame, where a hitch costs nothing); 1 = the hold alone.
	static constexpr double LightHoldMaxSeconds = 2.0;
	static constexpr int32 LightHoldSettleTicks = 3;
	static constexpr float LightFadeSeconds = 0.35f;
	static bool LightHoldDone(bool bRigVisible, int32 VisibleTicks, double HeldSeconds)
	{
		return (bRigVisible && VisibleTicks >= LightHoldSettleTicks) || HeldSeconds >= LightHoldMaxSeconds;
	}
	bool IsLightHoldActive() const { return bLightHold; }

private:
	void BeginLightHold();
	void TickLightHold();
	bool IsLightRigVisible() const;
	bool bLightHold = false;
	double LightHoldStart = 0.0;
	int32 LightHoldTicks = 0, LightVisibleTicks = 0;
	int32 LightStillIn = -1;   // proof: ticks until the "first visible frame" still (jam.screen.HoldStill 1)
	int64 LastStillBucket = -1;   // proof: jam.screen.StillEvery
};
