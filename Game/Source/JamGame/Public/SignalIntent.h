// Lane A12 (ruling R-0007 point 3): the shouted team signal the human gives their bot teammates. One struct, two readers:
// UJamBarkSystem (the teammates' acknowledgement bark) and lane A3's brain (JamBrain honours it for Frames frames; the exact
// consumption rules per intent are in docs/experiments/ACE.md section 2). Produced by UJamSignalSystem (Phase 2) from input actions
// 1-4 / d-pad, later from Windows speech recognition. Frames, never seconds: the brain counts sim frames at 60 Hz.
#pragma once

#include "CoreMinimal.h"
#include "SignalIntent.generated.h"

UENUM(BlueprintType)
enum class EJamTeamIntent : uint8
{
	None,
	Defend,   // teammates fall back towards the own net
	Space,    // teammates spread to free ice
	Pass,     // the teammate with the puck looks for the human
	Praise,   // the praised teammate (TargetIndex) gets a brief skill boost
};

USTRUCT(BlueprintType)
struct JAMGAME_API FJamTeamIntent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Jam|Signals")
	EJamTeamIntent Intent = EJamTeamIntent::None;

	// How long the brain honours it, in sim frames (defaults: defend 300, space 300, pass 180, praise 300).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Signals")
	int32 Frames = 300;

	// Sim frame the signal was given on.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Signals")
	int64 IssuedFrame = 0;

	// Skater index (game.teams order) of the human who gave the signal.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Signals")
	int32 IssuerIndex = -1;

	// Skater index the intent is aimed at (praise: the praised teammate; pass: the teammate asked to pass), -1 = every teammate.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Signals")
	int32 TargetIndex = -1;

	bool IsActive(int64 Frame) const { return Intent != EJamTeamIntent::None && Frame >= IssuedFrame && Frame < IssuedFrame + Frames; }
	int32 FramesLeft(int64 Frame) const { return IsActive(Frame) ? static_cast<int32>(IssuedFrame + Frames - Frame) : 0; }

	// Wire names: "defend" | "space" | "pass" | "praise"; NAME_None for None. Also the Kind of the "signal" bus event and the
	// suffix of the bark situation "signal_ack_<name>".
	static FName NameOf(EJamTeamIntent I)
	{
		switch (I)
		{
		case EJamTeamIntent::Defend: return FName(TEXT("defend"));
		case EJamTeamIntent::Space: return FName(TEXT("space"));
		case EJamTeamIntent::Pass: return FName(TEXT("pass"));
		case EJamTeamIntent::Praise: return FName(TEXT("praise"));
		default: return NAME_None;
		}
	}
	static EJamTeamIntent FromName(FName N)
	{
		if (N == FName(TEXT("defend"))) return EJamTeamIntent::Defend;
		if (N == FName(TEXT("space"))) return EJamTeamIntent::Space;
		if (N == FName(TEXT("pass"))) return EJamTeamIntent::Pass;
		if (N == FName(TEXT("praise"))) return EJamTeamIntent::Praise;
		return EJamTeamIntent::None;
	}
	static int32 DefaultFrames(EJamTeamIntent I) { return I == EJamTeamIntent::Pass ? 180 : 300; }
};
