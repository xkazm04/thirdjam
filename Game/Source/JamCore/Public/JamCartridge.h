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
	// game.skater.visual (R-0001): "toy" | "humanoid_b1" | "generated_b2"; absent = "toy".
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FName Visual = TEXT("toy");
	// C-0005 (lane A16): the pass button.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double PassSpeed = 14;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 PassLeadFrames = 20;
};

// game.stick (R-0013, lane A16): the mouse / right-stick aimed stick.
USTRUCT(BlueprintType)
struct JAMCORE_API FJamStickConfig
{
	GENERATED_BODY()
	// How far the blade reaches from the skater's edge at full aim, metres.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double ReachM = 1.6;
	// Frames after a poke press during which the blade reaching the puck takes it cleanly.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 PokeWindowFrames = 8;
	// Centre-to-centre distance (beyond the two bodies) within which a stick lift strips the carrier.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double LiftRangeM = 1.0;
	// Frames of lost control after a poke that was timed wrong next to a carrier (0 = no penalty, hockey.raw).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int32 StumbleFrames = 20;
	// Mouse delta to aim: 1 = 100 pixels move the blade one full reach.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") double MouseSensitivity = 1.0;
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
	// 0..4294967295
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") int64 Seed = 1;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FString SourcePath;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Cartridge") FJamGameConfig Game;
	// The raw text, kept so the Designer can diff and re-validate.
	UPROPERTY() FString RawJson;
};
