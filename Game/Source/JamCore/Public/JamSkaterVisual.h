// The seam between gameplay (device A) and every skater look (device B). Contract: contracts/skater-visual.md.
// Gameplay drives an AJamSkater pawn and talks to its visual only through this interface; it never knows what the skater looks like.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "JamSkaterVisual.generated.h"

// Socket names every skater mesh carries (contracts/skater-visual.md, "Sockets").
namespace JamSkaterSockets
{
	JAMCORE_API extern const FName Stick;   // "stick_socket": where the stick attaches (T-B2: child of hand_r)
	JAMCORE_API extern const FName Puck;    // "puck_socket": where a carried puck sits, in front of the blade
	JAMCORE_API extern const FName Head;    // "head_socket": above the head, for markers
}

// Material parameter names gameplay may drive (contracts/skater-visual.md, contracts/look-tiers.md).
namespace JamMaterialParams
{
	JAMCORE_API extern const FName TeamColour;  // "TeamColour": vector, linear RGB, alpha 1
	JAMCORE_API extern const FName NumberTens;  // "NumberTens": scalar 0..9 (0 for a one-digit number)
	JAMCORE_API extern const FName NumberOnes;  // "NumberOnes": scalar 0..9
	JAMCORE_API extern const FName Emissive;    // "Emissive": scalar >= 0, emissive multiplier
	JAMCORE_API extern const FName Wet;         // "Wet": scalar 0..1, 1 = wet ice look
	JAMCORE_API extern const FName Scuff;       // "Scuff": scalar 0..1, ice scuffing
}

// Boost kinds passed to SetBoost (cartridge game.pickups[].kind). NAME_None = no boost.
namespace JamBoostKinds
{
	JAMCORE_API extern const FName Speed;         // "speed"
	JAMCORE_API extern const FName Precision;     // "precision"
	JAMCORE_API extern const FName HeavyPuck;     // "heavy_puck"
	JAMCORE_API extern const FName BigPuck;       // "big_puck"
	JAMCORE_API extern const FName ReverseOther;  // "reverse_other"
}

UINTERFACE(BlueprintType, Blueprintable)
class JAMCORE_API UJamSkaterVisual : public UInterface
{
	GENERATED_BODY()
};

// Implemented by a component (or actor) the skater pawn owns. Every call comes on the game thread from the fixed
// 60 Hz simulation tick. Units: metres per second, radians per second, degrees in FRotator.
// Call from C++ with IJamSkaterVisual::Execute_<Name>(Object, ...) so Blueprint implementations are reached too.
class JAMCORE_API IJamSkaterVisual
{
	GENERATED_BODY()

public:
	// Team id (index into game.teams, 0 or 1) and its jersey colour (linear, from game.teams[].colour_hex). At spawn and at every cartridge load.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Jam|SkaterVisual")
	void SetTeam(int32 TeamId, FLinearColor Colour);

	// Jersey number 0..99; drives NumberTens and NumberOnes.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Jam|SkaterVisual")
	void SetNumber(int32 N);

	// Every simulation frame: speed in m/s (>= 0), signed turn rate in rad/s (positive = turning left seen from above), facing (yaw only, pitch and roll 0).
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Jam|SkaterVisual")
	void UpdateMotion(float Speed, float TurnRate, FRotator Facing);

	// One-shot: a shot left this skater's stick on this frame.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Jam|SkaterVisual")
	void PlayShoot();

	// One-shot: this skater threw a body check on this frame.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Jam|SkaterVisual")
	void PlayCheck();

	// Knocked down (true) or back up (false). DirX, DirZ: unit vector on the ice plane (rink x, rink z) the figure falls towards; ignored when bDown is false.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Jam|SkaterVisual")
	void SetKnockedDown(bool bDown, float DirX, float DirZ);

	// The skater gained (true) or lost (false) the puck.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Jam|SkaterVisual")
	void SetHasPuck(bool bHasPuck);

	// Active boost kind (JamBoostKinds) or NAME_None when the boost ends.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Jam|SkaterVisual")
	void SetBoost(FName Kind);
};
