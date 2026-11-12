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
#include "JamSim.generated.h"

JAMCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogJamSim, Log, All);

// One skater's controls for one simulation frame. Humans: latched from Enhanced Input at the step; bots: computed from sim state.
USTRUCT(BlueprintType)
struct JAMCORE_API FJamInput
{
	GENERATED_BODY()

	// Desired direction on the ice in rink coordinates (X = rink x, Y = rink z). Its length scales top speed and thrust:
	// humans are clamped to 1, bots to 1.5 (the AI brain multiplies in game.ai.speed_scale; the sim does not apply it again).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Sim")
	FVector2D Move = FVector2D::ZeroVector;

	// Shoot pressed on this frame (edge, not hold). A shot happens if reach, facing and cooldown allow it.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Sim")
	bool bShoot = false;

	// Check pressed on this frame (edge). Opens a 6-frame check window; only lands when game.skater.check_enabled.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Sim")
	bool bCheck = false;

	// Brake held (skate stop). Optional; no default key binds it in M0.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Sim")
	bool bBrake = false;
};

// (Named EJamSimPhase, not EJamPhase: lane A5's presentation enum EJamPhase lives in JamGame/HUDFeed.h.)
UENUM(BlueprintType)
enum class EJamSimPhase : uint8
{
	Playing,
	GoalPause,   // after a goal: everyone brakes for goal_pause_frames, then the faceoff reset
	Over,        // a team won; frozen until Restart()
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

	// One-frame flags for the visual (set during the step that just ran).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bShotThisFrame = false;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bCheckThisFrame = false;
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
	static constexpr double FaceoffDropM = 0.5;      // the faceoff puck lands up to this far off centre (seeded)

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
	// Timer mode: frames left (0 when not in timer mode). In timer mode a tie at 0 plays on to the next goal.
	int32 GetTimerFrames() const { return TimerLeft; }
	bool IsOvertime() const { return bOvertime; }
	uint64 GetSeed() const { return Seed; }
	FJamRandom& GetRandom() { return Rng; }

	// Geometry helpers (rink metres).
	// Signed distance to the boards: negative inside. OutNormal points outwards (towards the boards).
	double BoardDistance(const FVector2D& P, FVector2D& OutNormal) const;
	// x of the goal line a team attacks (+GL for team 0, -GL for team 1); the goal mouth centre is (x, 0).
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
	void StepSkater(FJamSkaterState& S, const FJamInput& In);
	void CollideSkaterStatic(FJamSkaterState& S);
	void CollideSkaters();
	void TryShoot(FJamSkaterState& S);
	void StepPuck();
	bool CollidePuckSegment(const FVector2D& A, const FVector2D& B, const FVector2D& PrevPos, double R, double E);
	bool CollidePuckCircle(const FVector2D& C, double CR, double R, double E);
	bool CollidePuckBox(const FVector2D& Min, const FVector2D& Max, double R, double E);
	void ScoreGoal(int32 ScoringTeam);
	void Emit(FName Name, int32 Who, const FVector2D& Where, float Strength, FName Kind = NAME_None);
	void KnockDown(int32 Victim, const FVector2D& Dir, float Strength, FName Cause);

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
};
