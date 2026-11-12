// Third-person follow camera with frame-based lag (game.camera). Lane A5; the board clamp, the end-board lift and the far-net bias are lane A40.
// Recipe and measurements: tools/skills/camera-third-person.md, docs/experiments/CAMERA-120M.md, docs/experiments/CAMERA.md.
#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "HUDFeed.h"
#include "JamEventBus.h"
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

	// A40, game.camera.boards_margin_m: the camera never crosses the boards' inner face minus this, on any side (corner arcs included).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float BoardsMarginM = 0.8f;

	// A40, game.camera.end_height_m: the height the camera lifts to when the boards take the whole arm (it rises in proportion to the arm lost). A49: 3 (was 5).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float EndHeightM = 3.f;

	// A40, game.camera.far_net_bias (0..1): how much of the angle to the far net the framing leads by when the skater heads for it. A49: 0.2 (was 0.5).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float FarNetBias = 0.2f;

	// Lane A49 (complaint 2, "the camera does not stay where the player moves"): the human never leaves the frame's middle third. The yaw may differ from the
	// bearing camera -> pivot by at most MiddleThirdYawDeg (the horizontal third's edge at 70 degrees of FOV is 13.1 degrees), the pitch from the bearing to the
	// chest point (ChestM up) by at most MiddleThirdPitchDeg (the vertical third's edge at 16:9 is 7.5 degrees). The lead and the end-board yaw work inside that.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float MiddleThirdYawDeg = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float MiddleThirdPitchDeg = 6.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float ChestM = 1.f;

	// Not cartridge keys (constants of the recipe): the critically damped spring's time constant in sim frames (lift and yaw offset), the metres of
	// look-ahead added per metre of arm the boards take (tilts the lifted camera up the rink instead of down at the skater's back), and the distance
	// band over which the far-net bias fades in (none inside the near edge, full beyond the far edge; the slot is where the lead matters).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float SpringFrames = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float LiftLookAheadGain = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float FarNetBiasNearM = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float FarNetBiasFarM = 20.f;

	// From the cartridge's game.camera (JamCore's FJamCameraConfig): mode, distance, height, look-ahead, lag, fov, boards margin, end height, far-net bias. Off-axis stays 0.7 m.
	static FJamCameraSettings FromCartridge(const struct FJamCameraConfig& Cartridge);
};

// A critically damped spring stepped once per SIMULATION frame (dt = 1 frame, never DeltaSeconds): exact solution x(t) = target + (j0 + j1 t) e^(-w t).
// Never overshoots a step; reaches 95 % of it in about 4.7 / Omega frames.
struct JAMGAME_API FJamSpring
{
	float X = 0.f;
	float V = 0.f;
	bool bInit = false;

	void Reset(float Value);
	void Step(float Target, float OmegaPerFrame);
};

// The lag filter and the clamp state, a pure value type stepped once per SIMULATION frame (never by DeltaSeconds).
struct JAMGAME_API FJamCameraFilter
{
	FVector2D PivotM = FVector2D::ZeroVector;
	float YawDeg = 0.f;
	bool bInit = false;

	// A40: the lift (0 = the normal rig, 1 = end_height_m, the whole arm displaced) and the yaw offset (degrees, far-net lead plus along-the-rink at an end board)
	// follow their targets through springs.
	FJamSpring Lift;
	FJamSpring YawOffsetDeg;
	// A49: the pitch follows its budgeted target through a spring too (no pop when the chest bearing moves fast next to the boards).
	FJamSpring PitchDeg;

	// Diagnostics of the last step (not state): the displacement the boards imposed over the arm length (0 = free, 1 = a whole arm) and its direction (into the rink).
	float LastClampS = 0.f;
	FVector2D LastClampNormal = FVector2D::ZeroVector;

	// alpha = 1 - exp(-1 / lag_frames), 1 when lag_frames == 0.
	static float AlphaForLag(int32 LagFrames);
	void Reset(const FVector2D& TargetM, float TargetYawDeg);
	void Step(const FVector2D& TargetM, float TargetYawDeg, int32 LagFrames);
};

// The rink the camera is boxed in by: metres, boards at +-length/2 and +-width/2, corner arcs of CornerRadiusM (0 = sharp corners).
struct JAMGAME_API FJamCameraRink
{
	float LengthM = 120.f;
	float WidthM = 60.f;
	float CornerRadiusM = 0.f;

	static FJamCameraRink FromFrame(const FJamPresentationFrame& Frame);
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

	// Camera transform (centimetres) for a filtered pivot (rink metres) and yaw. Third-person mode, the open-ice rig (no clamp, no lift).
	static FTransform ComputeThirdPersonPose(const FJamCameraSettings& C, const FVector2D& PivotM, float YawDeg);

	// A40. True when P (metres) is inside the rink's inner face pulled in by MarginM on every side, corner arcs included (radius = corner - margin, at least 0).
	static bool IsInsideBoards(const FVector2D& PM, const FJamCameraRink& Rink, float MarginM);

	// A40. The nearest point of that inset region to P (P itself when inside).
	static FVector2D NearestInsideBoards(const FVector2D& PM, const FJamCameraRink& Rink, float MarginM);

	// A40. The boards' answer to a desired camera point: the nearest point of the inset region (the camera slides along the glass, never through it). OutS is the
	// displacement over ArmM (0 untouched, clamped to 1), OutDir its unit direction (zero when untouched). The projection onto a convex set is non-expansive, so
	// the camera never moves more per frame than the point it wanted to be at: no pops from the clamp itself.
	static FVector2D ClampToBoards(const FVector2D& WantM, const FJamCameraRink& Rink, float MarginM, float ArmM, float& OutS, FVector2D& OutDir);

	// H11 behaviour, kept for the before stills and the legacy test (jam.camera.Legacy 1): pull in along the view axis to 0.3 m inside a rectangle, nothing else.
	static FVector ClampInsideBoards(const FVector& FocusCm, const FVector& CameraCm, float RinkLengthM, float RinkWidthM, float MarginM = 0.3f);

	// A40. One SIMULATION frame of the third-person camera: lag filter, far-net bias, board clamp, lift and yaw springs, pose (centimetres).
	// Pure: tests and scripted movers call it with their own state and frame. The frame's HumanIndex must be valid.
	static FTransform StepThirdPerson(const FJamCameraSettings& C, FJamCameraFilter& State, const FJamPresentationFrame& Frame);

	// Lane A49. Where a world point (cm) lands in the frame of a pose: x, y in [-1, 1] across the frame (0 = centre), for a horizontal FOV and an aspect ratio;
	// a point behind the camera returns (+-9, +-9). |x| <= 1/3 and |y| <= 1/3 is the middle third.
	static FVector2D FrameFraction(const FTransform& Pose, float HFovDeg, float Aspect, const FVector& WorldCm);

	// A40. The yaw lead toward the far net, degrees, before the spring: bias * toward * distance band * (yaw to net - yaw), 0 when skating away or near the net.
	static float FarNetLeadDeg(const FJamCameraSettings& C, const FVector2D& PivotM, float YawDeg, const FVector2D& FarNetM);

	// Fixed high camera framing both nets (Arena mode): looks at the rink centre from the +Y side, so rink +X is on the right (like the radar).
	static FTransform ComputeArenaPose(const FJamCameraSettings& C, float RinkLengthM, float RinkWidthM);

	// One simulation frame of the camera. Public so tests and scripted movers can drive it without a world.
	void StepSimFrame(const FJamPresentationFrame& Frame);

	// Lane A33 (R-0028 point 3): the camera punch on a check the human lands or takes. Six sim frames: a kick of PunchCm along the camera's right (the side the
	// contact is on) and PunchRollDeg of roll, both decaying linearly to zero; scaled by max(0.5, strength). Never a time dilation (fixed step). Pure for tests:
	// FrameIndex 0..5 -> the offsets of that frame; anything else -> 0.
	static constexpr int32 PunchFrames = 6;
	static constexpr float PunchCm = 2.f;
	static constexpr float PunchRollDeg = 0.6f;
	static void PunchOffset(int32 FrameIndex, float Strength, float Sign, float& OutRightCm, float& OutRollDeg);
	// Starts a punch (also callable by proof scripts): Sign +1 kicks right, -1 left.
	UFUNCTION(BlueprintCallable, Category = "Jam|Camera")
	void StartPunch(float Strength, float Sign);
	int32 GetPunchFramesLeft() const { return PunchLeft; }

	const FJamCameraFilter& GetFilter() const { return Filter; }
	const FTransform& GetCurrentSimPose() const { return PoseCurrent; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UFUNCTION() void OnJamEvent(FName Event, const FJamEventPayload& Payload);

	FJamCameraFilter Filter;
	FTransform PosePrev;
	FTransform PoseCurrent;
	bool bHavePose = false;
	FDelegateHandle FrameHandle;
	// Lane A33 punch state (stepped per sim frame in StepSimFrame).
	int32 PunchLeft = 0;
	float PunchStrength = 0.f;
	float PunchSign = 1.f;
	int32 LastHumanIndex = -1;
	FVector2D LastHumanPosM = FVector2D::ZeroVector;
	UPROPERTY(Transient) TObjectPtr<UJamEventBus> Bus;
};
