// The deterministic hockey simulation: skaters, puck, shots, checks, goals, rules. Fixed step 1/60 s, our own integrator,
// no Chaos, no Character Movement. Recipe: tools/skills/skating-physics.md. Port of the Godot hockey (docs/HOCKEY.md section 2).
//
// Units: metres, seconds, frames. Rink coordinates: x along the rink (Unreal X), z across (Unreal Y); 1 m = 100 Unreal units.
// Team 0 (game.teams[0]) defends the -x net and attacks +x; team 1 the other way round. Skater index = position in game.teams
// order, team 0's skaters first (contracts/events.md, "Who").
//
// The sim never touches the world, the event bus or actors: it queues FJamSimEvent per step and the game mode delivers them.
// Same config + seed + input sequence = same StateHash() every frame (the Determinism test).
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "JamCartridge.h"
#include "JamRandom.h"
#include "JamModifiers.h"
#include "JamRuleHooks.h"
#include "JamSim.generated.h"

JAMCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogJamSim, Log, All);

// Lane A15's zone seam (Rules/JamZones.h, R-0019); that header includes this one, so only the names here.
class IJamZoneSource;

// One skater's controls for one simulation frame. Humans: latched from Enhanced Input at the step; bots: computed from sim state.
USTRUCT(BlueprintType)
struct JAMCORE_API FJamInput
{
	GENERATED_BODY()

	// Desired direction on the ice in rink coordinates (X = rink x, Y = rink z). Its length scales top speed and thrust:
	// humans are clamped to 1, bots to 1.5 (the AI brain multiplies in game.ai.speed_scale; the sim does not apply it again).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Sim")
	FVector2D Move = FVector2D::ZeroVector;

	// Shoot pressed on this frame (edge, not hold): a tap shot at shot.speed, if reach, facing and cooldown allow it. Bots and scripts use it.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Sim")
	bool bShoot = false;

	// C-0005 charged shot: shoot held (level). While held with the puck in reach the charge grows to shot.charge_frames_max; the shot fires on
	// release at shot.speed + charge * charge_speed_bonus. The human's game mode sends the level here and never the edge above.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Sim")
	bool bShootHeld = false;

	// C-0005 pass (E / X) pressed on this frame (edge): a lead pass at skater.pass_speed to the best open teammate ahead of the puck.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Sim")
	bool bPass = false;

	// Check pressed on this frame (edge). Opens a 6-frame check window; only lands when game.skater.check_enabled.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Sim")
	bool bCheck = false;

	// Brake held (skate stop). Optional; no default key binds it in M0.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Sim")
	bool bBrake = false;

	// R-0013 stick aim: where the blade is held, as an offset direction from the skater in rink coordinates; |Stick| <= 1 is the fraction
	// of game.stick.reach_m. Zero = no aim: the blade sits ahead at half reach. Humans: mouse delta / right stick integrated by the game mode.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Sim")
	FVector2D Stick = FVector2D::ZeroVector;

	// Poke check pressed this frame (edge): the blade takes the puck cleanly if it reaches it inside poke_window_frames; a poke next to an
	// opposing carrier that misses costs stumble_frames of control. During a faceoff countdown any press is an early move (loses the draw).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Sim")
	bool bPoke = false;

	// Stick lift pressed this frame (edge): strips an opposing carrier within lift_range_m.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Sim")
	bool bLift = false;
};

// (Named EJamSimPhase, not EJamPhase: lane A5's presentation enum EJamPhase lives in JamGame/HUDFeed.h.)
UENUM(BlueprintType)
enum class EJamSimPhase : uint8
{
	Playing,
	GoalPause,   // after a goal (or a foul): everyone brakes for goal_pause_frames, then the faceoff reset
	Over,        // a team won; frozen until Restart()
	Faceoff,     // R-0013: both centres at the dot, a countdown (GetFaceoffCountdown), then the drop; the puck is pinned at the dot
};

USTRUCT(BlueprintType)
struct JAMCORE_API FJamSkaterState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 Index = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 Team = 0;
	// "player" | "chaser" | "support" | "back"
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") FName Role;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bHuman = false;

	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") FVector2D Pos = FVector2D::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") FVector2D Vel = FVector2D::ZeroVector;
	// Facing angle in radians in the rink plane: atan2(z, x). 0 faces +x. Equals the Unreal yaw (in radians).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") double Yaw = 0;
	// Yaw change over the last frame in rad/s (sim sign: + = towards +z = Unreal yaw right).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") double YawRate = 0;

	// Previous frame, for render interpolation.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") FVector2D PrevPos = FVector2D::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") double PrevYaw = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") FVector2D SpawnPos = FVector2D::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") double SpawnYaw = 0;

	// First frame a shot is allowed again (cooldown).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int64 ShotReadyFrame = 0;
	// Frames left of the check window (set to 6 when check is pressed).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 CheckWindow = 0;
	// Frames left knocked down; 0 = standing.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 KnockFrames = 0;
	// Direction of the fall (rink x, z), unit; valid while KnockFrames > 0.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") FVector2D KnockDir = FVector2D(1, 0);

	// R-0013 stick. The blade on the ice (rink metres) this frame, from the stick aim.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") FVector2D BladePos = FVector2D::ZeroVector;
	// Frames left of the poke window (set to stick.poke_window_frames on a poke press).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 PokeWindow = 0;
	// True on a frame with stick input (|Stick| > 0.05): only an aimed stick deflects a shot; a blade held by default receives passes but blocks nothing.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bStickAimed = false;
	// True while the open poke window was armed next to an opposing carrier (a miss then stumbles).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bPokeArmed = false;
	// Frames of lost control after a mistimed poke; 0 = in control.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 StumbleFrames = 0;
	// Frames left in the penalty box; 0 = on the ice.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 PenaltyFrames = 0;
	// First frame a poke / lift is allowed again.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int64 PokeReadyFrame = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int64 LiftReadyFrame = 0;
	// Takes the draw at the current faceoff.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bCentre = false;
	// The draw: -1 no input yet, -2 moved early (loses), otherwise the frame of this centre's first stick input after the drop.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int64 DrawFrame = -1;

	// C-0005 charge: frames shoot has been held with the puck in reach (0..shot.charge_frames_max); the visual winds up on it.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 ChargeFrames = 0;
	// Charge 0..1 of the shot that left this frame (0 for a tap or a pass); PlayShoot(Charge) takes it.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") float LastShotCharge = 0.f;
	// Frames before this skater may deflect again (one deflection per contact).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 DeflectCooldown = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 Passes = 0;

	// One-frame flags for the visual (set during the step that just ran).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bShotThisFrame = false;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bPassThisFrame = false;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bCheckThisFrame = false;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bPokeThisFrame = false;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bLiftThisFrame = false;
	// C-0001 v3 one-frame results for the visual: a poke resolved (1 clean, -1 missed), the draw resolved (1 won, -1 lost), a foul was called on this skater,
	// the strength (speed / max) of the pass that left this frame; and the level: standing at the dot as a centre before the drop.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 PokeResult = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 DrawResult = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bFoulThisFrame = false;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") float LastPassStrength = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bAtFaceoff = false;
	// R-0029 point 3: the last frame this skater carried the puck (-1000 = never); a check within game.stick.foul_grace_frames of it is not a foul_no_puck.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int64 LastCarryFrame = -1000;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bTouchingPuck = false;
	// Last toucher of the puck and the puck within shot reach.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bHasPuck = false;

	// Match statistics (tests and tournaments read them).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 Shots = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 Touches = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 Goals = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") double DistanceM = 0;

	FVector2D Forward() const { return FVector2D(FMath::Cos(Yaw), FMath::Sin(Yaw)); }
	bool IsDown() const { return KnockFrames > 0; }
	bool IsStumbling() const { return StumbleFrames > 0; }
	bool InBox() const { return PenaltyFrames > 0; }
	// Standing, in control, on the ice: may shoot, check, poke, lift.
	bool CanAct() const { return !IsDown() && !IsStumbling() && !InBox(); }
};

USTRUCT(BlueprintType)
struct JAMCORE_API FJamPuckState
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") FVector2D Pos = FVector2D::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") FVector2D Vel = FVector2D::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") FVector2D PrevPos = FVector2D::ZeroVector;
	// Skater index that touched or shot it last, -1 after a faceoff.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 LastTouch = -1;
	// Total distance travelled this match.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") double PathM = 0;
};

// An event the step produced; the game mode hands it to UJamEventBus (converting Where to centimetres).
USTRUCT(BlueprintType)
struct JAMCORE_API FJamSimEvent
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") FName Name;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 Who = -1;
	// Rink metres (x, z).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") FVector2D Where = FVector2D::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") float Strength = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") FName Kind = NAME_None;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int64 Frame = 0;
	// Side channel for "announce" (C-0005): the rule's text for the scoreboard line. Empty for every other event.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") FString Text;
};

UCLASS(BlueprintType)
class JAMCORE_API UJamSim : public UObject
{
	GENERATED_BODY()

public:
	static constexpr double DT = 1.0 / 60.0;
	static constexpr int32 PuckSubsteps = 4;
	static constexpr double SkaterRadius = 0.4;
	static constexpr double NetDepth = 1.5;
	static constexpr double NetHeight = 1.5;
	static constexpr double PostRadius = 0.05;
	static constexpr double ShotFrontDot = 0.5;      // must face within 60 degrees
	static constexpr int32 CheckWindowFrames = 6;
	static constexpr double FaceoffDropM = 0.5;      // the faceoff puck lands up to this far off the dot (seeded)
	// R-0013: the faceoff countdown, the draw window after the drop, the penalty box, the pause before a free faceoff after a foul.
	static constexpr int32 FaceoffCountdownFrames = 90;
	static constexpr int32 DrawWindowFrames = 30;
	static constexpr int32 BoxFrames = 300;
	static constexpr int32 FoulPauseFrames = 60;
	static constexpr double PokeTakeM = 0.35;        // blade-to-puck distance (beyond the puck radius) that takes the puck
	static constexpr double DeflectRangeM = 0.6;     // C-0005: a shot passing within this of a stick deflects
	static constexpr double DeflectKeep = 0.7;       // ... at 70 % of its speed
	static constexpr double DeflectMinSpeed = 6.0;   // slower pucks are just pushed
	static constexpr int32 PassCooldownFrames = 12;
	static constexpr double CarryYawRateMax = 1.5707963; // rad/s (90 deg/s): carry assist off above it (C-0005)

	// Builds the match from a validated config: skaters at their spawns, puck at centre, score 0, phase Playing.
	// Queues the opening "faceoff" event. Returns false (and logs) if the config has no player.
	bool Init(const FJamGameConfig& InConfig, uint64 InSeed);

	// New match with the same config and seed (restart on R).
	void Restart();

	// Director's Call / Designer cut applied live (R-0003 3): takes the live-changeable tunables of NewConfig (skater, shot, puck,
	// ice friction, board restitution, goal pause, ai, pickups, look, camera) without resetting the match. Rink geometry
	// (length, width, corner, goal width, obstacles), teams, spawns and the rules mode/end condition never change mid-match;
	// returns the dotted names of the fields that were held back because they differ (empty = everything applied).
	TArray<FString> ApplyLiveConfig(const FJamGameConfig& NewConfig);

	// Timer mode: frames of play elapsed (timer_frames - frames left); 0 in first_to mode.
	int32 GetTimerElapsed() const { return Config.Rules.TimerFrames > 0 ? Config.Rules.TimerFrames - TimerLeft : 0; }

	// ---- C-0005 seam A: modifiers (JamModifiers.h). Lane A14's UJamEffectSystem is the only source. ----
	// None set = base values, bit-identical hashes. The sim collects once per Step, before integration.
	void SetModifierSource(IJamModifierSource* Source) { ModifierSource = Source; }
	IJamModifierSource* GetModifierSource() const { return ModifierSource; }
	// effective(stat, skater) = base * prod(Multiply) + sum(Add), clamped to the schema range; the base value when no modifier touches it.
	// Puck and rink stats (JamStats::IsGlobal) ignore Skater; pass -1 for them.
	double GetEffectiveStat(EJamStat Stat, int32 Skater) const;
	// The modifiers collected for the frame that last ran (ids are part of StateHash).
	const TArray<FJamActiveModifier>& GetActiveModifiers() const { return ActiveModifiers; }
	// ApplyModifier actions queued by rule hooks; the modifier source drains them (once per frame, before the next Step).
	void TakeModifierRequests(TArray<FJamRuleRequest>& Out) { Out.Append(ModifierRequests); ModifierRequests.Reset(); }

	// ---- R-0019 zone seam (Rules/JamZones.h). Lane A15's UJamZoneSystem is the only source. None set = plain ice everywhere. ----
	// Ice under a skater scales its top speed (SpeedMul) and glide friction (FrictionMul); under the puck its friction; bumper zones throw the puck
	// back each substep (BumpPuck); a shot from inside a no_shoot zone is refused with event shot_blocked; pickups only where AllowsPickupAt says.
	void SetZoneSource(IJamZoneSource* Source) { ZoneSource = Source; }
	IJamZoneSource* GetZoneSource() const { return ZoneSource; }
	// For the pickup system (A14): true when no source is set, else the source's answer.
	bool AllowsPickupAt(const FVector2D& PosM) const;

	// ---- C-0005 seam B: rule hooks (JamRuleHooks.h). Lane A15's UJamRuleSystem is the only hook. ----
	// None set = no-op. Triggers fire inside Step (and OnFaceoff / OnPeriodStart inside Restart); actions apply in order in the same frame.
	void SetRuleHook(IJamRuleHook* Hook) { RuleHook = Hook; }
	IJamRuleHook* GetRuleHook() const { return RuleHook; }
	// The pickup system (A14) reports a take here: fires OnPickupTake (Who, Kind). The sim itself has no pickups.
	void NotifyPickupTaken(int32 Who, FName Kind);
	// SpawnPickup actions queued by rule hooks; the pickup system drains them.
	void TakePickupRequests(TArray<FJamRuleRequest>& Out) { Out.Append(PickupRequests); PickupRequests.Reset(); }
	// A tunable by its `values` key (skater_speed, shot_speed, ice_friction, ...) through the live-config path (R-0003 3); false for an unknown key.
	bool SetParam(FName Key, double Value);
	// Period (1-based): timer mode counts thirds of timer_frames (the TAKE card's rule); first_to mode advances only on an EndPeriod action.
	int32 GetPeriod() const { return Period; }
	// Grows by one every time ShrinkRink / GrowRink rebuilt the geometry at a faceoff (and when Restart restores it): the rink actor rebuilds when it changes.
	int32 GetRinkRevision() const { return RinkRevision; }
	bool AreNetsSwapped() const { return bNetsSwapped; }
	// Points a goal is worth for a team (SetScoreMultiplier); 1 by default.
	int32 GetScoreMultiplier(int32 Team) const { return (Team == 0 || Team == 1) ? ScoreMultiplier[Team] : 1; }

	// Advances exactly one frame (1/60 s). Inputs[i] drives skater i; missing entries are zero input.
	void Step(const TArray<FJamInput>& Inputs);

	// FNV-1a 64 over all integer-quantised state (1 mm, 1 mm/s, 1 mrad, counters, score, rng).
	uint64 StateHash() const;
	FString StateHashHex() const { return FString::Printf(TEXT("%016llx"), static_cast<unsigned long long>(StateHash())); }

	// Events produced by the last Step (cleared at the start of every Step).
	const TArray<FJamSimEvent>& GetFrameEvents() const { return FrameEvents; }

	const FJamGameConfig& GetConfig() const { return Config; }
	const TArray<FJamSkaterState>& GetSkaters() const { return Skaters; }
	const FJamPuckState& GetPuck() const { return Puck; }
	int64 GetFrame() const { return Frame; }
	EJamSimPhase GetPhase() const { return Phase; }
	int32 GetScore(int32 Team) const { return (Team == 0 || Team == 1) ? Score[Team] : 0; }
	// -1 while playing; the winning team when Over.
	int32 GetWinner() const { return Winner; }
	int32 GetHumanIndex() const { return HumanIndex; }
	int32 GetHumanTeam() const { return Skaters.IsValidIndex(HumanIndex) ? Skaters[HumanIndex].Team : 0; }
	// Frames left of the goal pause (0 while playing).
	int32 GetPauseFrames() const { return PauseFrames; }
	// R-0013: frames left of the faceoff countdown (0 outside the Faceoff phase); the HUD counts it down.
	int32 GetFaceoffCountdown() const { return Phase == EJamSimPhase::Faceoff ? FaceoffCountdown : 0; }
	// Where the current (or next) faceoff is taken, rink metres.
	FVector2D GetFaceoffDot() const { return FaceoffDot; }
	// True between the drop and the moment a centre won the puck.
	bool IsDrawOpen() const { return bDrawOpen; }
	// The skater carrying the puck right now (last toucher, standing and in control, puck within its reach), -1 when the puck is loose. The brain reads it.
	int32 CarrierIndex() const;
	// Tests: ends a running countdown at once with a loose puck at the dot and no draw (so a scripted scene starts in Playing).
	void SkipFaceoff();
	// The teammate a pass from Skater would go to right now (the rule of TryPass) and the aim point; -1 when nobody is open. Scripts and the brain read it.
	int32 BestPass(int32 Skater, FVector2D& OutAim) const { return Skaters.IsValidIndex(Skater) ? PickReceiver(Skaters[Skater], OutAim) : -1; }
	// Timer mode: frames left (0 when not in timer mode). In timer mode a tie at 0 plays on to the next goal.
	int32 GetTimerFrames() const { return TimerLeft; }
	bool IsOvertime() const { return bOvertime; }
	uint64 GetSeed() const { return Seed; }
	FJamRandom& GetRandom() { return Rng; }

	// Geometry helpers (rink metres).
	// Signed distance to the boards: negative inside. OutNormal points outwards (towards the boards).
	double BoardDistance(const FVector2D& P, FVector2D& OutNormal) const;
	// x of the goal line a team attacks (+GL for team 0, -GL for team 1; the other way round after SwapNets); the goal mouth centre is (x, 0).
	double AttackGoalX(int32 Team) const;
	double GoalLineX() const { return Config.Rink.GoalLineX(); }
	// True when a puck centre at P is inside the goal volume of the net at +x (Side +1) or -x (Side -1).
	bool InGoal(const FVector2D& P, int32 Side) const;

	// Rink metres -> Unreal centimetres (the one conversion; Height in metres above the ice).
	static FVector ToWorld(const FVector2D& P, double HeightM = 0.0) { return FVector(P.X * 100.0, P.Y * 100.0, HeightM * 100.0); }

	// Debug/tests: place the puck (velocity in m/s).
	void SetPuck(const FVector2D& Pos, const FVector2D& Vel);
	// Debug/tests: place a skater.
	void SetSkater(int32 Index, const FVector2D& Pos, const FVector2D& Vel, double Yaw);

private:
	void FaceoffReset();
	void StepFaceoff(const TArray<FJamInput>& Inputs);
	void ResolveDraw(int32 Winner);
	void StepSkater(FJamSkaterState& S, const FJamInput& In);
	void StepStick(FJamSkaterState& S, const FJamInput& In);
	void TryPoke(FJamSkaterState& S);
	void TryLift(FJamSkaterState& S);
	void ResolvePokes();
	void CallFoul(FJamSkaterState& Fouler, const FJamSkaterState& Victim, const FVector2D& Contact, float Strength, FName Kind);
	FVector2D BoxPos(int32 Team) const;
	void CollideSkaterStatic(FJamSkaterState& S);
	void CollideSkaters();
	void TryShoot(FJamSkaterState& S, float Charge);
	void StepCharge(FJamSkaterState& S, const FJamInput& In);
	void TryPass(FJamSkaterState& S);
	// The teammate a pass from S goes to, and where to aim (lead); -1 when nobody is open.
	int32 PickReceiver(const FJamSkaterState& S, FVector2D& OutAim) const;
	void StepPuck();
	bool CollidePuckSegment(const FVector2D& A, const FVector2D& B, const FVector2D& PrevPos, double R, double E);
	bool CollidePuckCircle(const FVector2D& C, double CR, double R, double E);
	bool CollidePuckBox(const FVector2D& Min, const FVector2D& Max, double R, double E);
	void ScoreGoal(int32 ScoringTeam);
	void Emit(FName Name, int32 Who, const FVector2D& Where, float Strength, FName Kind = NAME_None);
	void KnockDown(int32 Victim, const FVector2D& Dir, float Strength, FName Cause);

	// C-0005 seams.
	double BaseStat(EJamStat Stat) const;
	void CollectModifiers();
	void ResolveStats();
	double Eff(EJamStat Stat, int32 Skater) const { return GetEffectiveStat(Stat, Skater); }
	void FireRule(EJamRuleTrigger Trigger, int32 Who, int32 Team, FName Kind, float Value);
	void ApplyRuleAction(const FJamRuleEventIn& In, const FJamRuleActionOut& A);
	void EndPeriodNow();
	void EndMatchNow(int32 Team);
	void UpdatePeriod();
	void ApplyPendingRink();

	UPROPERTY() FJamGameConfig Config;
	UPROPERTY() TArray<FJamSkaterState> Skaters;
	UPROPERTY() FJamPuckState Puck;
	UPROPERTY() TArray<FJamSimEvent> FrameEvents;

	FJamRandom Rng;
	uint64 Seed = 1;
	int64 Frame = 0;
	EJamSimPhase Phase = EJamSimPhase::Playing;
	int32 Score[2] = { 0, 0 };
	int32 Winner = -1;
	int32 HumanIndex = 0;
	int32 PauseFrames = 0;
	int32 TimerLeft = 0;
	bool bOvertime = false;
	int32 BoardEventCooldown = 0;

	// R-0013 faceoff and draw state.
	int32 FaceoffCountdown = 0;
	FVector2D FaceoffDot = FVector2D::ZeroVector;
	FVector2D PendingFaceoffDot = FVector2D::ZeroVector;
	int64 DropFrame = -1;
	bool bDrawOpen = false;
	int64 BotDrawFrame[2] = { -1, -1 };

	// C-0005 seam state. Everything here is at its default when no source / hook is set, and then adds nothing to the hash.
	IJamModifierSource* ModifierSource = nullptr;
	IJamRuleHook* RuleHook = nullptr;
	IJamZoneSource* ZoneSource = nullptr;
	TArray<FJamActiveModifier> ActiveModifiers;
	// (Skaters.Num() + 1) rows of EJamStat::Count doubles; the last row holds the puck and rink stats.
	TArray<double> EffStats;
	TArray<FJamRuleRequest> PickupRequests;
	TArray<FJamRuleRequest> ModifierRequests;
	int32 ScoreMultiplier[2] = { 1, 1 };
	int32 Period = 1;
	bool bNetsSwapped = false;
	double PendingRinkScale = 1.0;
	int32 RinkRevision = 0;
	int32 RuleDepth = 0;
	// The rink and spawns as Init saw them, so Restart() undoes ShrinkRink / GrowRink.
	FJamRinkConfig InitRink;
	TArray<FVector2D> InitSpawns;
};
