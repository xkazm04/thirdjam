// Cartridge loader (tj.cartridge/1). Mirrors schema/validate.mjs: same verdict, same JSON pointer first in every error
// (schema/README.md, "The rule for lane A1"). Applies schema/DEFAULTS.md after validation, then the `values` overrides.
// Units: metres, metres per second, frames (60 per second). Rink coordinates: x along the rink, z across.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "JamRulesConfig.h"
#include "JamCartridge.generated.h"

JAMCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogJamCartridge, Log, All);

USTRUCT(BlueprintType)
struct JAMCORE_API FJamObstacle
{
	GENERATED_BODY()
	// "box" (square, side = Size), "pillar" (circle, diameter = Size), "bumper" (circle, diameter = Size, puck restitution 1.6: C-0005 / A15)
	// or "goal_shield" (a wall Size metres wide and 0.3 m deep, centred at X, Z, standing across the rink: C-0005 / A15).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FName Kind;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double X = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double Z = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double Size = 1;
};

USTRUCT(BlueprintType)
struct JAMCORE_API FJamRinkConfig
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double LengthM = 120;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double WidthM = 60;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double CornerRadiusM = 5;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double BoardRestitution = 0.4;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double IceFriction = 0.015;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double GoalWidthM = 4;
	// R-0033 point 2 (lane A50): the height of the net opening; 0.61 is the operator's half-height net for a game without goalies (a real net is 1.22).
	// One number for the net frame, the netting, the goal triggers and the sim's goal test for a lifted puck (UJamSim::InGoal with a height).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double GoalHeightM = 0.61;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") TArray<FJamObstacle> Obstacles;
	// game.rink.zones (C-0005, lane A15): painted areas that change play inside them. Absent = none.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") TArray<FJamZoneConfig> Zones;

	// Derived (DEFAULTS.md): goal line at length/2 - 4 from the centre.
	double GoalLineX() const { return LengthM * 0.5 - 4.0; }
};

USTRUCT(BlueprintType)
struct JAMCORE_API FJamSkaterConfig
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double Speed = 8;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double Accel = 12;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double TurnRate = 5;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double LateralGrip = 3;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double Mass = 80;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double GlideDrag = 0.147;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double BrakeDecel = 6;
	// R-0013: the body check is available in every cut (absent = true).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") bool bCheckEnabled = true;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double CheckSpeedMin = 5;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 KnockdownFrames = 90;
	// game.skater.visual (R-0001, R-0024): "toy" | "humanoid_b1" | "generated_b2" | "toy_skeletal"; absent = "toy_skeletal" (the Director, H11.7; "toy" stays a value).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FName Visual = TEXT("humanoid_b1");   // R-0030: the bake-off (lane A30) picked B's rebuilt scripted figure
	// C-0005 (lane A16): the pass button.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double PassSpeed = 14;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 PassLeadFrames = 20;
	// game.skater.carry_speed_mult (lane A82, R-0041 point 6): the carrier's top speed and acceleration as a fraction of a free skater's; the carrier cannot sprint. 1 = off.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double CarrySpeedMult = 0.92;
	// game.skater.breakaway (R-0029 point 4, lane A39): a carrier past centre ice with no opponent between it and the net gets this fraction more
	// top speed and acceleration for BreakawayFrames, once per possession (event breakaway). speed_bonus 0 = off.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double BreakawaySpeedBonus = 0.2;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 BreakawayFrames = 60;
	// game.skater.sprint (lane A49): Shift / LT held adds bonus to top speed and thrust while the stamina lasts; it drains in drain_frames of sprint and
	// refills in refill_frames at rest. bonus 0 = off.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double SprintBonus = 0.15;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 SprintDrainFrames = 180;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 SprintRefillFrames = 360;
	// R-0035 (lane A52): game.skater.goalie { reach_m, block_height_m }: a skater in the crease stance saves a loose shot under block_height_m that passes
	// within reach_m of it (the human included); a higher puck passes. reach_m 0 = no save by height (the body and the blade stop pucks as before).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double GoalieReachM = 0.9;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double GoalieBlockHeightM = 0.4;   // Director calibration 01:30 (R-0035): below the 0.61 bar, so a half-charged shot (0.45) clears the stance and stays under the bar; at 0.7 the stance saved everything the net could take and the novice stopped scoring in time
	// game.skater.spin (lane A51, R-0034): E / gamepad B. A full turn over frames, cooldown_frames before the next, speed x speed_mult while it runs,
	// stamina_cost of the sprint bar taken at the start (refused with an empty bar). frames 0 = off.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 SpinFrames = 24;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 SpinCooldownFrames = 90;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double SpinSpeedMult = 0.7;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double SpinStaminaCost = 0.25;
};

// game.stick (R-0013, lane A16): the mouse / right-stick aimed stick.
USTRUCT(BlueprintType)
struct JAMCORE_API FJamStickConfig
{
	GENERATED_BODY()
	// How far the blade reaches from the skater's edge at full aim, metres.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double ReachM = 1.2;   // R-0022 point 3 (1.6 before); lane A51 (R-0034): the range is 0.6..1.6; the brief's 1.1 halved the passes the regular player receives at pro (STICK.md section 6), so 1.2 stays
	// Frames after a poke press during which the blade reaching the puck takes it cleanly.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 PokeWindowFrames = 8;
	// Centre-to-centre distance (beyond the two bodies) within which a stick lift strips the carrier.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double LiftRangeM = 1.0;
	// Frames of lost control after a poke that was timed wrong next to a carrier (0 = no penalty, hockey.raw).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 StumbleFrames = 20;
	// Mouse delta to aim: 1 = 100 pixels move the blade one full reach.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double MouseSensitivity = 1.0;
	// R-0029 point 3: a check on a skater that carried the puck within this many frames is not a foul_no_puck (the puck just left it).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 FoulGraceFrames = 12;
	// game.stick.reception_m (lane A46, PAYOFFS.md section 6): a teammate's pass reaching the body within this radius at 6-14 m/s is seated on the blade
	// (4 frames of settle; the one-timer window opens); faster passes bounce off the body unless the blade meets them. 0 = blade reception only.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double ReceptionM = 0.9;
	// game.stick.deke (R-0029 point 4, lane A39): a deke tap (FJamInput::bDeke) moves the carried puck DekeOffsetM to the blade side within 6 frames and
	// commits the crease back for DekeCommitFrames; a shot inside that window aims at the post on the deke side. offset_m 0 = off.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double DekeOffsetM = 1.0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 DekeCommitFrames = 15;
	// game.stick.sweep (lane A51, R-0034): the carried puck follows the blade every frame; an aim that moves more than min_deg_per_frame counts as a
	// sweep and shields the puck for shield_frames from a poke on the side it left; a defending bot pokes at where the puck sat on the carrier's body
	// bot_react_frames ago (-1 = absent: by the difficulty profile, rookie 14, pro 8, all_star 5). shield_frames 0 = no shield.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double SweepMinDegPerFrame = 6.0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 SweepShieldFrames = 12;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 SweepBotReactFrames = -1;
};

USTRUCT(BlueprintType)
struct JAMCORE_API FJamPuckConfig
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double RadiusM = 0.12;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double MaxSpeed = 30;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double Drag = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FLinearColor Colour = FLinearColor(0.0056f, 0.0056f, 0.0056f, 1.f);
	// C-0005 (lane A16): 0..1 pull of a carried puck towards the stick; number of pucks on the ice (1..3).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double CarryAssist = 0.6;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 Count = 1;
	// R-0035 (lane A52): the lifted puck's gravity (m/s^2) and the bounce on the ice (a hop under 2 cm dies).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double GravityMps2 = 9.81;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double IceRestitution = 0.3;
};

USTRUCT(BlueprintType)
struct JAMCORE_API FJamShotConfig
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double ReachM = 1.5;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double Speed = 18;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 CooldownFrames = 24;
	// Absent = no aim assist (bHasAimCone false).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") bool bHasAimCone = false;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double AimConeDeg = 0;
	// C-0005 (lane A16): charged shot (0 frames = off) and deflections off skaters.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 ChargeFramesMax = 30;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double ChargeSpeedBonus = 8;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") bool bDeflect = true;
	// game.shot.one_timer (R-0029 point 4, lane A39): a shot released within OneTimerWindowFrames of receiving a teammate's pass gets the full
	// charge without the hold and aim_cone_deg + OneTimerConeBonusDeg of aim help; the shot event carries Kind one_timer. window_frames 0 = off.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 OneTimerWindowFrames = 10;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double OneTimerConeBonusDeg = 8;
	// game.shot.lift (R-0035, lane A52): the height a shot arrives at over the goal line by its charge: a tap tap_m, a hold of half_frames half_m, the full
	// charge (charge_frames_max) full_m, linear between (UJamSim::LiftForCharge). All three at 0 = every shot on the ice (the old game).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double LiftTapM = 0.05;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double LiftHalfM = 0.45;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double LiftFullM = 0.6;   // Director calibration 01:40 (R-0035 amended): just under the 0.61 bar, the top-shelf shot; at 0.95 every hold past 8 frames sailed over the low net and the novice gate fell to 8 of 20
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 LiftHalfFrames = 8;
};

// Lane A77 (R-0036): game.pass. The saucer pass: a pass held saucer.half_frames or more lifts saucer.lift_m at its apex, flies over a blade in the lane and lands
// before the receiver (UJamSim::ApplySaucerLift); a shorter hold or the edge is the flat pass. half_frames 0 or lift_m 0 = the saucer is off.
USTRUCT(BlueprintType)
struct JAMCORE_API FJamPassConfig
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 SaucerHalfFrames = 8;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double SaucerLiftM = 0.35;
};

USTRUCT(BlueprintType)
struct JAMCORE_API FJamRulesConfig
{
	GENERATED_BODY()
	// "first_to" | "timer"
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FName Mode = TEXT("first_to");
	// 0 when absent (only read in first_to mode, where the validator requires it).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 ScoreToWin = 0;
	// 0 when absent (only read in timer mode, where the validator requires it).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 TimerFrames = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 GoalPauseFrames = 120;
	// R-0013: a check from behind or on a skater without the puck is a foul (free faceoff at the fouler's end, 300 frames in the box). Absent = true.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") bool bPenalties = true;
	// Lane A72 (ask A8): game.rules.penalty_frames, frames a hard foul sits in the box (60..600). Absent = 300 (UJamSim::BoxFrames).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 PenaltyFrames = 300;
	// game.rules.hooks (C-0005, lane A15) in file order, then the hooks ExpandVariants made from game.rules.variants (bFromVariant).
	// UJamRuleSystem evaluates them in id order. Absent = none.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") TArray<FJamRuleHookConfig> Hooks;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FJamRuleVariantsConfig Variants;
};

USTRUCT(BlueprintType)
struct JAMCORE_API FJamSkaterSpawn
{
	GENERATED_BODY()
	// "player" | "chaser" | "support" | "back"
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FName Role;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double X = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double Z = 0;
};

USTRUCT(BlueprintType)
struct JAMCORE_API FJamTeamConfig
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FString Id;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FString ColourHex;
	// Linear colour converted once from ColourHex (R-0001 item 2: nobody else parses hex).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FLinearColor Colour = FLinearColor::White;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") TArray<FJamSkaterSpawn> Skaters;
};

USTRUCT(BlueprintType)
struct JAMCORE_API FJamAiConfig
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double YieldMarginM = 2;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double YieldRadiusM = 15;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double HoldRadiusM = 4.5;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double SpeedScale = 1;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double RedSkill = 1;
	// Lane A72 (asks A4, A5): game.ai.react_frames (0..60) and game.ai.body_check (0..1). Read by JamAI::ResolveDifficulty only when the key is written
	// (absent = the level's profile), so these defaults are never applied on their own.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 ReactFrames = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double BodyCheck = 1;
	// Lane A72 (ask A2): game.ai.sprint, the share of the time a bot sprints on a chase (0..1). Absent = 0.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double Sprint = 0;
};

USTRUCT(BlueprintType)
struct JAMCORE_API FJamPickupConfig
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FName Kind;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 SpawnEveryFrames = 600;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 DurationFrames = 300;
	// Absent: 1.5 speed, 2 precision, 2 heavy_puck, 1.5 big_puck, 1 reverse_other (DEFAULTS.md).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double Magnitude = 1;
};

USTRUCT(BlueprintType)
struct JAMCORE_API FJamLookConfig
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FString Tier = TEXT("l2_night");
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FLinearColor TrimColour = FLinearColor::White;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double FogDensity = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") bool bWetIce = false;
	// look.trim_emissive (0..100): the board trim strips' Emissive. -1 = absent, the tier's default applies (ULookTierSwitcher::DefaultTrimEmissiveFor: 30, clay 12). B-to-A-0018.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double TrimEmissive = -1;
};

USTRUCT(BlueprintType)
struct JAMCORE_API FJamCameraConfig
{
	GENERATED_BODY()
	// "third_person" | "arena"
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FName Mode = TEXT("third_person");
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double DistanceM = 6;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double HeightM = 3.5;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double LookAheadM = 5;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 LagFrames = 4;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double FovDeg = 70;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") bool bRadar = true;
	// A40: the board clamp margin, the end-board height and the far-net lead (DEFAULTS.md).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double BoardsMarginM = 0.8;
	// Lane A49: end_height_m 3 (was 5) and far_net_bias 0.2 (was 0.5): the lead and the lift fought the mouse aim and pushed the human out of the frame's middle third.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double EndHeightM = 3;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double FarNetBias = 0.2;
};

// The hockey `game` object with every default applied and every `values` override folded in.
USTRUCT(BlueprintType)
struct JAMCORE_API FJamGameConfig
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FJamRinkConfig Rink;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FJamSkaterConfig Skater;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FJamStickConfig Stick;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FJamPuckConfig Puck;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FJamShotConfig Shot;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FJamPassConfig Pass;   // lane A77 (R-0036)
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FJamRulesConfig Rules;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") TArray<FJamTeamConfig> Teams;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FJamAiConfig Ai;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") TArray<FJamPickupConfig> Pickups;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FJamLookConfig Look;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FJamCameraConfig Camera;
};

UCLASS(BlueprintType)
class JAMCORE_API UJamCartridge : public UObject
{
	GENERATED_BODY()

public:
	// Parses and validates; on success returns a new cartridge, on failure returns null and fills OutErrors
	// with sentences that start "at <JSON pointer>:" (same pointers as schema/validate.mjs).
	static UJamCartridge* LoadFromString(const FString& Json, TArray<FString>& OutErrors, UObject* Outer = nullptr);
	static UJamCartridge* LoadFromFile(const FString& Path, TArray<FString>& OutErrors, UObject* Outer = nullptr);

	// Validation only (no defaults, no object): the C++ mirror of validateCartridge().
	static bool Validate(const FString& Json, TArray<FString>& OutErrors);

	// "hockey.regular" -> <Content>/Cartridges/hockey.regular.cartridge.json; a path ending in .json is returned as is.
	static FString ResolveCartridgePath(const FString& NameOrPath);
	static FString GetCartridgeDir();

	// Converts "#RRGGBB" (sRGB) to a linear colour; false on a malformed string.
	static bool HexToLinear(const FString& Hex, FLinearColor& Out);

	// schema/README.md "Tunables": sets the game field a `values` key names (skater_speed, shot_speed, ice_friction, ...).
	// False when the key names no field. The same mapping the loader applies; the rule hook's SetParam (C-0005) rides it.
	static bool ApplyValueKey(FJamGameConfig& Config, const FString& Key, double Value);

	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FString Id;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FString Title;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FString ThemeName;
	// Lane A81 (A67 ask A11): game.meta, the menu's words for a Director's choice; informational, never read by the sim. Empty when absent.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FString MetaPitch;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") TArray<FString> MetaFacts;
	// 0..4294967295
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int64 Seed = 1;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FString SourcePath;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FJamGameConfig Game;
	// The raw text, kept so the Designer can diff and re-validate.
	UPROPERTY() FString RawJson;
};
