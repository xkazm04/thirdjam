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

	// R-0029 point 4 (lane A39) deke pressed this frame (edge): the carried puck moves game.stick.deke.offset_m to the blade side within 6 frames and the
	// crease back commits for commit_frames; a second tap inside 30 frames does nothing. LOCKSTEP: a new input field; lane A38's FJamNetQuantInput must
	// carry it as a flag bit (the checklist line), else the two peers desync at the first deke.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Sim")
	bool bDeke = false;

	// Lane A49 sprint held (level): +game.skater.sprint.bonus top speed and thrust while the skater moves and its stamina lasts (drain_frames of
	// sprint, refill_frames to full). Bots never set it. LOCKSTEP: a new input field; lane A38's FJamNetQuantInput must carry it as a flag bit.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Sim")
	bool bSprint = false;

	// Lane A51 (R-0034): the spin move pressed this frame (edge): E / gamepad B. game.skater.spin: a full turn over spin.frames, the puck on the far side
	// of the body from the nearest opponent, speed x speed_mult, stamina_cost of the bar; refused with an empty bar or inside the cooldown.
	// LOCKSTEP: FJamNetQuantInput carries it as flag bit 512 (protocol 5).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Sim")
	bool bSpin = false;
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
	// Frames left of the check window (set to 6 when check is pressed; 12 for the human, lane A49's calibration).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 CheckWindow = 0;
	// Lane A49 sprint stamina in ticks: full = drain_frames * refill_frames; a sprinting frame takes refill_frames, a resting frame gives drain_frames back.
	// Starts full; hashed only once it left full, so a match without a sprint hashes as before.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 Stamina = -1;
	// Read-out: the sprint applied this frame.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bSprinting = false;
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

	// R-0029 point 4 (lane A39): the three payoffs.
	// The frame a teammate's pass landed on this blade (-1 none); a shot within shot.one_timer.window_frames of it is a one-timer. Only kept while one-timers are on.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int64 PassReceivedFrame = -1;
	// Frames left of the breakaway burst (top speed and accel x (1 + speed_bonus)); the burst fires once per possession (bBreakawayUsed resets when the puck changes hands).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 BreakawayFrames = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bBreakawayUsed = false;
	// Consecutive frames as the carrier (counted only while the burst is on): the burst needs BreakawayCarryFrames of carry, so a touch-and-shoot never bursts.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 CarryFrames = 0;
	// Frames left of the deke (the puck moves over the first DekeMoveFrames, the crease back stays committed for all of them); DekeSide +1 = towards +z, -1 = towards -z, 0 = none.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 DekeFrames = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 DekeSide = 0;
	// First frame a deke is allowed again (a second tap inside DekeLockoutFrames does nothing).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int64 DekeReadyFrame = 0;
	// Lane A51 (R-0034) stick handling. The sweep: frames left of the shield after the blade crossed fast with the puck (0 = none) and the side the puck
	// left as the sign of Cross(Forward, blade - body) (+1 or -1; 0 = none): an opponent's poke whose blade is on that side misses while the shield runs.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 SweepFrames = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 SweepSide = 0;
	// The spin (game.skater.spin): frames left (0 = none), the whole length, the turn direction (+1 = the sim's yaw sense, to the right), the first frame
	// a spin is allowed again. The carried puck rides the far side of the body from the nearest opponent (BladePos follows it), a poke never reaches it,
	// a check landing mid-spin knocks the carrier down for SpinKnockdownMult of the normal frames. Hashed only while they left their default.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 SpinFrames = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 SpinLength = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 SpinDir = 1;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int64 SpinReadyFrame = 0;
	// Match statistics of the stick handling (not hashed, like Dekes).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 Sweeps = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 Spins = 0;
	// Lane A46: frames left of the settle after a body reception (the carry assist holds the puck on the blade; 0 = none). Hashed only while it runs.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 SettleFrames = 0;
	// Match statistics of the payoffs; PassesReceived (lane A46) counts teammates' passes seated on this skater's blade (blade or body reception).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 PassesReceived = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 OneTimers = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 Breakaways = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 Dekes = 0;

	// One-frame flags for the visual (set during the step that just ran).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bShotThisFrame = false;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bPassThisFrame = false;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bCheckThisFrame = false;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bPokeThisFrame = false;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bLiftThisFrame = false;
	// R-0029 point 4 (lane A39) one-frame flags for the visual: the shot that left was a one-timer, the burst fired, a deke started.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bOneTimerThisFrame = false;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bBreakawayThisFrame = false;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bDekeThisFrame = false;
	// Lane A51 (R-0034) one-frame flags: a sweep started (the blade crossed fast with the puck), a spin started.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bSweepThisFrame = false;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bSpinThisFrame = false;
	// C-0001 v3 one-frame results for the visual: a poke resolved (1 clean, -1 missed), the draw resolved (1 won, -1 lost), a foul was called on this skater,
	// the strength (speed / max) of the pass that left this frame; and the level: standing at the dot as a centre before the drop.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 PokeResult = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 DrawResult = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bFoulThisFrame = false;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") float LastPassStrength = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bAtFaceoff = false;
	// Lane A45 (C-0001 v4, R-0031 point 1): read-outs for the visual's PlayHitReaction / PlayBoardHit / SetBoardPin / PlayStickHit. Never hashed, never read by the sim.
	// A skater-skater collision at BumpMinClosing (3 m/s) or more closing with no check landed this frame; BumpDir = the push on this skater (the other towards this one, unit).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bBumpThisFrame = false;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") FVector2D BumpDir = FVector2D(1, 0);
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int64 LastBumpFrame = -1000;
	// The skater `board` event (A33) fired for this skater this frame; BoardFrames = consecutive frames touching the boards (within BoardContactEpsM of them).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bBoardHitThisFrame = false;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 BoardFrames = 0;
	// BoardFrames >= BoardPinFrames (10) with a standing opponent within BoardPinRangeM (1.2 m, centre to centre); false while down or in the box.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") bool bBoardPinned = false;
	// This carrier lost the puck to an opponent's stick this frame: 1 a clean poke (ResolvePokes), 2 a lift (TryLift); 0 otherwise.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") int32 StickHitResult = 0;
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
	// Lane A51: spinning right now.
	bool IsSpinning() const { return SpinFrames > 0; }
	// Lane A51: the figure's extra yaw during the spin, degrees (0 outside it): the full circle over SpinLength frames in SpinDir's sense; the sim's Yaw stays.
	double SpinVisualYawDeg() const { return (SpinFrames > 0 && SpinLength > 0) ? SpinDir * 360.0 * static_cast<double>(SpinLength - SpinFrames) / static_cast<double>(SpinLength) : 0.0; }
};

USTRUCT(BlueprintType)
struct JAMCORE_API FJamPuckState
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") FVector2D Pos = FVector2D::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") FVector2D Vel = FVector2D::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") FVector2D PrevPos = FVector2D::ZeroVector;
	// R-0035 (lane A52): the puck's height above the ice (metres, the puck's centre line; 0 = on the ice) and its vertical speed (m/s, + = up). A shot's release
	// lifts it by the charge (game.shot.lift), gravity (game.puck.gravity_mps2) pulls it down, the ice bounces it (game.puck.ice_restitution) until the hop is
	// under 2 cm, a carried puck sits on the ice. Hashed like Pos and Vel once off the ice (a cut whose lift keys are all 0 hashes as before).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") double Z = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") double VZ = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") double PrevZ = 0;
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
	// Lane A47 (B-to-A-0029): the contact normal (checker towards victim) for check and knockdown, the board normal into the rink for board; zero otherwise.
	// Rink axes (x, z), unit length. A read-out: nothing in the step reads it and events are not hashed.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Sim") FVector2D Dir = FVector2D::ZeroVector;
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
	// R-0033 point 2 (lane A50): the net's height is the cartridge's game.rink.goal_height_m (FJamRinkConfig::GoalHeightM, 0.61 by default), no longer a constant.
	static constexpr double PostRadius = 0.05;
	static constexpr double ShotFrontDot = 0.5;      // must face within 60 degrees
	static constexpr int32 CheckWindowFrames = 6;
	// Lane A49 calibrations for the human's check (FEEL.md): a 12-frame window (0.2 s, a person's press is not frame-exact) and a check that lands from 2 m/s
	// of the human's own speed (the sim's rule needs the checker's speed along the contact, and the operator skates slowly into the carrier). Bots keep the cartridge's
	// check_speed_min and the 6-frame window, so the scripted Determinism baseline does not move.
	static constexpr int32 HumanCheckWindowFrames = 12;
	static constexpr double HumanCheckSpeedMinMs = 2.0;
	// The sprint stamina's full value for a config (ticks).
	int32 StaminaFull() const;
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
	// R-0029 point 4 (lane A39): the deke moves the puck over this many frames, a second tap inside the lockout does nothing, the puck sits this far
	// ahead of the body while deked (so it stays inside shot reach at the full offset), the burst needs this clear corridor to the net (half-width, and
	// the crease depth the back may hold without counting as "between"), and the deke shot aims this far inside the post.
	static constexpr int32 DekeMoveFrames = 6;
	static constexpr int32 DekeLockoutFrames = 30;
	static constexpr double DekeAheadM = 0.8;
	static constexpr double BreakawayCorridorM = 4.0;
	static constexpr double BreakawayCreaseM = 6.0;
	static constexpr double BreakawayMinGoalM = 8.0;
	static constexpr int32 BreakawayCarryFrames = 30;
	static constexpr double DekePostInsetM = 0.6;
	// Lane A46 (PAYOFFS.md section 6): body reception. A teammate's puck reaching the body within stick.reception_m at up to this speed is seated on the blade,
	// held there for ReceptionSettleFrames of settle (a shot inside A39's window is a one-timer); a faster puck bounces off the body unless the blade meets it.
	static constexpr double BodyReceptionMaxSpeed = 14.0;
	static constexpr int32 ReceptionSettleFrames = 4;
	// R-0035 (lane A52): the lifted puck. The glass stops a puck under GlassHeightM at the boards (no game.rink.glass_height_m key exists: 2.4 m fixed, a real
	// rink's 2.4 m of glass over 1.2 m of boards); above it the puck is out of play (a free faceoff at the end it left, as after a foul). A puck whose centre is
	// within BarRingM + PostRadius of the crossbar's axis rings `post` and reflects; a puck under the bar by more scores, over it by more flies over the net
	// (its roof is closed). A hop smaller than BounceDeadM dies on the ice. A standing skater's body stops a puck under BodyHeightM (a skater's height; the
	// lift's apex is capped at LiftApexMaxM, so only the top of a lob clears a body); a skater in the crease stance
	// (within CreaseStanceM of its own goal centre, R-0033) saves a puck under game.skater.goalie.block_height_m within goalie.reach_m instead, the human included.
	static constexpr double GlassHeightM = 2.4;
	static constexpr double BarRingM = 0.05;
	static constexpr double BounceDeadM = 0.02;
	static constexpr double BodyHeightM = 1.8;
	static constexpr double CreaseStanceM = 3.0;
	static constexpr double LiftFlightMaxS = 2.0;
	static constexpr double LiftApexMaxM = 2.0;
	static constexpr double SaveReboundKeep = 0.45;   // Director 01:45: 0.3 left 15 of 131 faceoffs without a human touch (budget 13) once the 0.6 full charge put more shots into the pads; a livelier rebound is a chase
	// True when S stands in the crease stance's geometry (R-0033 / R-0035): within CreaseStanceM of its own goal centre, on the ice side of the goal line, able to act.
	bool InCreaseStance(const FJamSkaterState& S) const;
	// Lane A51 (R-0034): the spin. A check that lands mid-spin knocks the carrier down for this multiple of knockdown_frames (risk against reward);
	// the carried puck rides this far from the body centre on the far side from the nearest opponent (inside shot reach, outside the body).
	static constexpr double SpinKnockdownMult = 1.5;
	static constexpr double SpinPuckM = 0.7;

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
	// R-0029 point 4 (lane A39): true when Skater carries the puck past centre ice, skates at the net and no standing opponent stands in the corridor
	// between it and the crease (the rule of the breakaway burst). The brain and the tests read it.
	bool PastLastDefender(int32 Skater) const;
	// The sideways offset of a deked puck from the body line, rink metres (zero when the skater is not deking); DekeOffsetM clamped so the puck stays in reach.
	FVector2D DekeOffset(const FJamSkaterState& S) const;
	double EffectiveDekeOffsetM() const;
	// The goalie commit of the latest deke: frames left (0 none), the deker and the side (+1 = +z). It runs deke.commit_frames from the tap and, for a shot
	// released inside the deke window, until that puck has crossed the goal line: the brain's crease back follows the puck's old side (puck - (0, side * offset))
	// while it runs. Outlives the carry, so the brain reads it from here, not from the carrier.
	int32 GetDekeCommit(int32& OutSkater, int32& OutSide) const { OutSkater = DekeCommitSkater; OutSide = DekeCommitSide; return DekeCommitFrames; }
	// Tests: ends a running countdown at once with a loose puck at the dot and no draw (so a scripted scene starts in Playing).
	void SkipFaceoff();
	// Lane A51 (R-0034): true when Poker's poke cannot reach the puck Carrier holds: the carrier spins (the body is between), or a sweep shield runs and
	// the poker's blade is on the side the puck left. Tests and the brain read it.
	bool IsPokeShielded(const FJamSkaterState& Poker, const FJamSkaterState& Carrier) const;
	// Lane A51: the nearest standing opponent of S (-1 none).
	int32 NearestOpponent(const FJamSkaterState& S) const;
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
	// True when a puck centre at P is inside the goal volume of the net at +x (Side +1) or -x (Side -1). The two-argument form is the puck on the ice.
	// R-0033 point 2 (lane A50): with a height (metres above the ice, the puck's centre) the opening is game.rink.goal_height_m tall: a lifted puck at or
	// above the crossbar is no goal. R-0035 (lane A52): the step passes the puck's Z, so a lifted shot over the bar is no goal in play.
	bool InGoal(const FVector2D& P, int32 Side) const { return InGoal(P, Side, 0.0); }
	bool InGoal(const FVector2D& P, int32 Side, double HeightM) const;
	// R-0035 (lane A52): the height a shot of this charge (0..1 of shot.charge_frames_max) arrives at over the goal line: game.shot.lift's tap_m at 0, half_m at
	// half_frames, full_m at the full charge, linear between. The tests and the brain's shot choice read it.
	double LiftForCharge(float Charge) const;

	// Rink metres -> Unreal centimetres (the one conversion; Height in metres above the ice).
	static FVector ToWorld(const FVector2D& P, double HeightM = 0.0) { return FVector(P.X * 100.0, P.Y * 100.0, HeightM * 100.0); }

	// Debug/tests: place the puck (velocity in m/s); the four-argument form also sets its height and vertical speed (R-0035).
	void SetPuck(const FVector2D& Pos, const FVector2D& Vel);
	void SetPuck(const FVector2D& Pos, const FVector2D& Vel, double Z, double VZ);
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
	// R-0029 point 4 (lane A39): the deke tap, the burst decision (before the skater moves), and the window bookkeeping after the puck moved.
	void TryDeke(FJamSkaterState& S);
	void StepBreakaway(FJamSkaterState& S);
	void StepPayoffWindows(FJamSkaterState& S);
	// Lane A51 (R-0034): the spin tap, the spin's frame (speed scale, the puck on the far side) after the skater moved, the sweep shield's countdown.
	void TrySpin(FJamSkaterState& S);
	void StepSpin(FJamSkaterState& S);
	void StepSweepShield(FJamSkaterState& S);
	FVector2D SpinPuckTarget(const FJamSkaterState& S) const;
	void ArmSweepShield(FJamSkaterState& S, int32 Side);
	// The teammate a pass from S goes to, and where to aim (lead); -1 when nobody is open.
	int32 PickReceiver(const FJamSkaterState& S, FVector2D& OutAim) const;
	void StepPuck();
	// R-0035 (lane A52): the free puck's height. StepPuckVertical integrates gravity and the ice bounce for one substep and pins a carried puck to the ice;
	// ApplyShotLift gives the shot that just left its VZ for LiftForCharge(Charge) over the goal line; TryGoalieSave is the crease skater's save by height
	// (the saver's index, -1 = no save this substep); OutOfPlay sends a puck over the glass to a free faceoff. All in their own blocks, away from the stick.
	void StepPuckVertical(double H);
	void ApplyShotLift(const FJamSkaterState& S, const FVector2D& Aim, double Speed, float Charge);
	int32 TryGoalieSave(double R, double MaxSpeed);
	void OutOfPlay();
	bool CollidePuckSegment(const FVector2D& A, const FVector2D& B, const FVector2D& PrevPos, double R, double E);
	bool CollidePuckCircle(const FVector2D& C, double CR, double R, double E);
	bool CollidePuckBox(const FVector2D& Min, const FVector2D& Max, double R, double E);
	void ScoreGoal(int32 ScoringTeam);
	void Emit(FName Name, int32 Who, const FVector2D& Where, float Strength, FName Kind = NAME_None, const FVector2D& Dir = FVector2D::ZeroVector);
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

	// Lane A32 (B-to-A-0026 ask 3): the commentator's read-out events. CheckSave runs where a skater's blade or body is about to touch the puck
	// (before the touch changes the velocity); StepReadouts runs at the end of a Playing frame for breakaway, carry and the clock. None of the
	// state below is hashed: these events describe play, they never change it (C-0003).
	void CheckSave(const FJamSkaterState& Blocker);
	void StepReadouts();
	void ResetReadouts();

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

	// R-0029 point 4 (lane A39): the goalie commit of the latest deke (see GetDekeCommit).
	int32 DekeCommitFrames = 0;
	int32 DekeCommitSide = 0;
	int32 DekeCommitSkater = -1;

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

	// Lane A32 read-out state (never hashed, see StepReadouts). The last shot in flight (for "save"), the current possession (for "carry"),
	// the rush already called (for "breakaway"), the take whose last ten seconds were called (for "clock"), the post event gap.
	int64 ShotFrame = -1000000;
	int32 ShotBy = -1;
	float ShotStrength = 0.f;
	int32 CarryIndex = -1;
	int64 CarryStartFrame = 0;
	FVector2D CarryStartPos = FVector2D::ZeroVector;
	FVector2D CarryPrevPuck = FVector2D::ZeroVector;
	double CarryDistM = 0.0;
	bool bCarryAnnounced = false;
	int32 BreakawayFor = -1;
	int32 ClockAnnouncedPeriod = 0;
	int32 PostEventCooldown = 0;
};
