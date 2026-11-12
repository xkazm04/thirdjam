// Third-person follow camera with frame-based lag (game.camera). Lane A5.
// Recipe and measurements: tools/skills/camera-third-person.md, docs/experiments/CAMERA-120M.md.
#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "HUDFeed.h"
#include "CameraFollow.generated.h"

UENUM(BlueprintType)
enum class EJamCameraMode : uint8
{
	ThirdPerson,
	Arena
};

// game.camera, defaults from schema/DEFAULTS.md. Absent keys keep these defaults.
USTRUCT(BlueprintType)
struct JAMGAME_API FJamCameraSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	EJamCameraMode Mode = EJamCameraMode::ThirdPerson;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float DistanceM = 6.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float HeightM = 3.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float LookAheadM = 5.f;

	// Horizontal field of view, degrees (this is the FieldOfView of the engine).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float FovDeg = 70.f;

	// Lag in simulation frames (0 = rigid).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	int32 LagFrames = 4;

	// The camera sits this far to the right of the skater's axis so the skater is off-axis and the puck is not hidden.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float OffAxisM = 0.7f;

	// From the cartridge's game.camera (JamCore's FJamCameraConfig): mode, distance, height, look-ahead, lag, fov. Off-axis stays 0.7 m.
	static FJamCameraSettings FromCartridge(const struct FJamCameraConfig& Cartridge);
};

// The lag filter, a pure value type stepped once per SIMULATION frame (never by DeltaSeconds).
struct JAMGAME_API FJamCameraFilter
{
	FVector2D PivotM = FVector2D::ZeroVector;
	float YawDeg = 0.f;
	bool bInit = false;

	// alpha = 1 - exp(-1 / lag_frames), 1 when lag_frames == 0.
	static float AlphaForLag(int32 LagFrames);
	void Reset(const FVector2D& TargetM, float TargetYawDeg);
	void Step(const FVector2D& TargetM, float TargetYawDeg, int32 LagFrames);
};

UCLASS(ClassGroup = (Jam), meta = (BlueprintSpawnableComponent))
class JAMGAME_API UJamFollowCamera : public UCameraComponent
{
	GENERATED_BODY()

public:
	UJamFollowCamera();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	FJamCameraSettings Config;

	// Interpolate between the previous and the current simulation pose with the sim's render alpha.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	bool bInterpolate = true;

	UFUNCTION(BlueprintCallable, Category = "Jam|Camera")
	void ApplyConfig(const FJamCameraSettings& InConfig);

	// The cartridge's game.camera (what the game mode has in hand): converted with FJamCameraSettings::FromCartridge.
	void ApplyConfig(const struct FJamCameraConfig& Cartridge);

	// camera_pitch = -atan(height / (distance + look_ahead)), degrees.
	static float DerivedPitchDeg(const FJamCameraSettings& C);

	// Camera transform (centimetres) for a filtered pivot (rink metres) and yaw. Third-person mode.
	static FTransform ComputeThirdPersonPose(const FJamCameraSettings& C, const FVector2D& PivotM, float YawDeg);

	// Fixed high camera framing both nets (Arena mode): looks at the rink centre from the +Y side, so rink +X is on the right (like the radar).
	static FTransform ComputeArenaPose(const FJamCameraSettings& C, float RinkLengthM, float RinkWidthM);

	// One simulation frame of the camera. Public so tests and scripted movers can drive it without a world.
	void StepSimFrame(const FJamPresentationFrame& Frame);

	const FJamCameraFilter& GetFilter() const { return Filter; }
	const FTransform& GetCurrentSimPose() const { return PoseCurrent; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	FJamCameraFilter Filter;
	FTransform PosePrev;
	FTransform PoseCurrent;
	bool bHavePose = false;
	FDelegateHandle FrameHandle;
};
