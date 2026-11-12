// What the presentation layer (camera, HUD, radar) reads from the simulation. Lane A5.
// The simulation (lane A1) pushes one FJamPresentationFrame per fixed 60 Hz step through UJamPresentation::PushFrame.
// Nothing here writes back into the simulation: camera, HUD and visuals are readers only.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HUDFeed.generated.h"

class UJamSim;

JAMGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogJamHud, Log, All);

UENUM(BlueprintType)
enum class EJamPhase : uint8
{
	Play,
	GoalPause,
	Win,
	Lose,
	Paused
};

USTRUCT(BlueprintType)
struct JAMGAME_API FJamPresentationSkater
{
	GENERATED_BODY()

	// Rink coordinates in metres: X along the rink (= Unreal X), Y across (rink z = Unreal Y).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Presentation")
	FVector2D PosM = FVector2D::ZeroVector;

	// Facing yaw in degrees, Unreal convention (0 = +X, positive turns towards +Y).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Presentation")
	float YawDeg = 0.f;

	// Index into game.teams (0 or 1).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Presentation")
	int32 TeamId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Presentation")
	bool bHuman = false;
};

USTRUCT(BlueprintType)
struct JAMGAME_API FJamPresentationFrame
{
	GENERATED_BODY()

	// Simulation frame number (60 Hz). Camera and HUD count frames, never wall clock.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Presentation")
	int64 SimFrame = 0;

	// Rink size in metres (game.rink.length_m / width_m).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Presentation")
	float RinkLengthM = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Presentation")
	float RinkWidthM = 60.f;

	// Team 0's skaters first, in game.teams order.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Presentation")
	TArray<FJamPresentationSkater> Skaters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Presentation")
	FVector2D PuckM = FVector2D::ZeroVector;

	// Index into Skaters of the skater the camera follows (the human).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Presentation")
	int32 HumanIndex = 0;

	// +1: the human attacks the net at +X (the far net is at +X); -1: at -X.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Presentation")
	int32 AttackSign = 1;

	// Distance of a net's mouth from the rink end, metres (the rink builder puts nets at +-(length/2 - 4)).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Presentation")
	float NetInsetM = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Presentation")
	int32 ScoreHome = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Presentation")
	int32 ScoreAway = 0;

	// first_to rule: the score that wins (0 = absent: a timer rule, or none).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Presentation")
	int32 ScoreToWin = 0;

	// Period number (TAKE n), 1-based.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Presentation")
	int32 Take = 1;

	// A timer exists only when the cartridge has one (absent = no timer shown).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Presentation")
	bool bHasTimer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Presentation")
	int32 FramesRemaining = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Presentation")
	EJamPhase Phase = EJamPhase::Play;

	// R-0013 (lane A16): frames left of the faceoff countdown; 0 = no countdown running. The HUD counts it down over the dot.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Presentation")
	int32 FaceoffCountdownFrames = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Presentation")
	bool bDirectorsCallReady = false;

	// game.teams[].colour_hex, converted once by the loader (RULINGS R-0001 item 2). Defaults are the two jersey colours of GAME.md.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Presentation")
	FLinearColor TeamColour0 = FLinearColor::FromSRGBColor(FColor(0x1D, 0x4E, 0xD8));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Presentation")
	FLinearColor TeamColour1 = FLinearColor::FromSRGBColor(FColor(0xE8, 0x65, 0x1A));

	FLinearColor GetTeamColour(int32 TeamId) const { return TeamId == 0 ? TeamColour0 : TeamColour1; }

	// World position (centimetres) the far-net marker points at: the mouth of the net the human attacks, 3 m up.
	FVector GetFarNetWorldCm() const;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FJamSimFrameDelegate, const FJamPresentationFrame&);

// World subsystem holding the latest frame. Gameplay calls PushFrame once per simulation step.
UCLASS()
class JAMGAME_API UJamPresentation : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UJamPresentation* Get(const UObject* WorldContext);

	// Stores the frame and fires OnSimFrame (camera filters step here, once per simulation frame).
	UFUNCTION(BlueprintCallable, Category = "Jam|Presentation")
	void PushFrame(const FJamPresentationFrame& Frame);

	UFUNCTION(BlueprintPure, Category = "Jam|Presentation")
	const FJamPresentationFrame& GetFrame() const { return Latest; }

	UFUNCTION(BlueprintPure, Category = "Jam|Presentation")
	bool HasFrame() const { return bHasFrame; }

	// Fraction (0..1) of the next simulation step the render frame is at; the sim's accumulator sets it, 1 when unknown.
	UFUNCTION(BlueprintCallable, Category = "Jam|Presentation")
	void SetRenderAlpha(float InAlpha) { RenderAlpha = FMath::Clamp(InAlpha, 0.f, 1.f); }

	UFUNCTION(BlueprintPure, Category = "Jam|Presentation")
	float GetRenderAlpha() const { return RenderAlpha; }

	// Adapter for lane A1's simulation: builds the frame from UJamSim state (positions, yaw, score, timer, phase, team colours).
	// The game mode calls PushFromSim(Sim) once after every UJamSim::Step.
	static void BuildFrameFromSim(const UJamSim& Sim, FJamPresentationFrame& Out);

	void PushFromSim(const UJamSim& Sim);

	FJamSimFrameDelegate OnSimFrame;

private:
	FJamPresentationFrame Latest;
	bool bHasFrame = false;
	float RenderAlpha = 1.f;
};
