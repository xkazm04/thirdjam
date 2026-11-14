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

	// Lane A64 (eyes grid h19 findings 3 and 7: no puck or carrier in 35 stills, the far net out of frame at the end boards): a loose puck (more than
	// PuckFrameMinM from the human, in front of the camera, nearer than PuckFrameMaxM) and, near the far end, the far net are kept within KeepYawDeg of the
	// camera yaw (the middle two thirds' edge at 70 degrees of FOV is 25 degrees), always inside the human's own middle-third budget, which wins.
	// 0 = off (the A40 / A49 tests' default); the game camera sets it from jam.camera.KeepYaw, default 0: OFF. Measured (lane A64): any yaw lead feeds
	// back through the A49 cursor aim (the cursor's point on the ice turns with the camera, the skater turns to it, the pivot yaw follows) and through the
	// camera-relative stick; with 22 the idle skater spun to the side boards and a held W never reached the far net. Kept as an opt-in for the Director.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float KeepYawDeg = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float PuckFrameMinM = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float PuckFrameMaxM = 40.f;

	// Lane A111 (track U1, B-to-A-0038 fault 5: "in the corners the camera looks out over the boards"). When the human is within CornerNearM of a board
	// (full inside CornerFullM) and the camera looks at that board, the camera rises by up to CornerLiftM and the look-ahead shrinks to CornerLookAheadKeep of
	// itself: the frame tilts down onto the ice around the play instead of out over the boards (the share of the frame that is ice: CAMERA rows in
	// docs/experiments/SCREEN.md). No yaw is added (lane A64 measured that a yaw lead feeds back through the cursor aim), the weight goes through a spring
	// (no snap), and the pitch stays inside A49's middle-third budget. 0 = off: the tests' default and the neutral value; the game camera sets it from
	// jam.camera.CornerLift (default 3).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float CornerLiftM = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float CornerNearM = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float CornerFullM = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float CornerLookAheadKeep = 0.2f;

	// Lane A135 (track U1, evolution 3: "the camera never loses the player"). The track's own probe counted the human's chest outside the frame's middle third on
	// 436 of 5,989 frames of the 100 s pro match on the 90 m rink (88 of them outside the frame) and on 519 of 5,991 on 140 m: when the skater turns near a
	// board the boards take the arm and the camera ends up over or beside him, still looking ahead, until lane A49's sprung budgets catch up; and the corner
	// lift's rise outruns the pitch spring. KeepHumanFrac > 0 adds a last stage to StepThirdPerson that holds the chest inside that fraction of the half frame
	// (0.31: between lane A49's own settled bounds, 12 degrees of yaw = 0.304 and 6.5 of pitch = 0.289, and the middle third's edge 0.333) on every frame, by
	// height, pitch and position first:
	//   the crane   the camera keeps the rig's own distance to the chest: what the boards take of the arm, it gains in height (the arm swings up over the
	//               skater instead of collapsing onto him), by at most KeepRiseM a frame up or down;
	//   the pitch   is held within the fraction's angle of the bearing to the chest (the rig's pitch where that is inside it, the nearest edge else);
	//   the step    a camera whose foot the boards pushed in front of the skater (his back at the glass) comes towards the point over him until the pitch can
	//               reach him (it cannot look past straight down);
	//   the slide   a chest still off to a side: the camera's foot slides sideways (along its own right, the yaw untouched) by exactly what he is outside,
	//               and stays inside the boards' inset;
	//   the yaw     last, and only for what the boards did not let the slide take: lane A49's own yaw budget (the camera's yaw within a bound of the bearing to the human)
	//               applied to the frame itself instead of to a spring's target. Where A49's spring has settled this changes nothing; it acts in the frames
	//               the spring is late, by the angle the human would be outside. It is not a lead towards anything (lane A64's loop was a standing lead).
	// Where the rig already holds the chest inside the fraction (open ice at a steady pace, every settled pose at the boards), the stage changes nothing.
	// 0 = off: the tests' default and the neutral value; the game camera sets it from jam.camera.KeepHuman (default 0.31). KeepAspect is the frame's width
	// over its height (the game camera sets it from the viewport).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float KeepHumanFrac = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float KeepAspect = 16.f / 9.f;

	// The crane's height changes by at most this many metres a sim frame (0.2: 12 m/s, the whole rise in a third of a second).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float KeepRiseM = 0.2f;

	// Lane A158 (track U1, evolution 5; B-to-A-0041 point 3, the critic's fault 11): the pitch cap. Where the boards take the arm the crane kept the arm's
	// length by height, so a corner was seen from 6 to 7.5 m straight above (pitch to 79 degrees: heads and shoulders, three bodies one shape). With
	// PitchCapDeg > 0 the crane and the rig's lifts give only the height at which the pitch that holds the chest is no steeper than the cap: the camera
	// comes down and in along the line of the cap, never nearer to the chest than PitchCapDistM and never under the open-ice height; where the boards leave less arm than
	// that (his back at the glass), the pitch goes over the cap by what is missing, from PitchCapDistM instead of from the crane's top. No yaw, no move of
	// the foot: height and pitch alone. 0 = off (the settings default and the neutral value); the game: jam.camera.PitchCap.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float PitchCapDeg = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float PitchCapDistM = 5.f;

	// Lane A171 (track U1, evolution 6): the cap read a second time, from where the keep-human stage LEFT the camera's foot. Evolution 5's cap read the arm
	// from the rig's foot, before that stage brought the foot in behind him: in a squeezed corner it saw more arm than there was and did not bite (sim frame
	// 2280 of the standard match: 6.46 m up, pitch 62 degrees, with and without the cap). With this on, the height is taken down again to what the cap's pitch
	// needs from the foot's final place: no lower than keeps PitchCapDistM to the chest, than the open-ice height, than the pitch needs to reach a skater
	// behind the foot, than holds him across the frame as well as before; by at most KeepRiseM a frame. Height and pitch alone. false = evolution 5's cap
	// (the settings default; the game: jam.camera.PitchCapAfter).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	bool bPitchCapAfterKeep = false;

	// Lane A135, evolution 4 (the Director's order: "the yaw out of the hold"): the move input is read in the live camera's yaw, so a camera the hold turns
	// turns W with it. With this on the hold is position, height and pitch alone: the slide is repeated against the boards' inset (a slide the glass stopped
	// goes on along the glass), and what is still left is answered by the least step of the foot towards the point over the skater (from there he is always
	// in the frame). Build 9 did exactly that and turned the camera on 0 frames, but the least step is not always a small one: 2.9 m of camera and 18 degrees
	// of pitch in one frame. So the hold's own move of the foot (its offset from the rig's foot) changes by at most KeepStepM a frame, and the yaw stays as a
	// guard for the frames the limited move does not yet hold him, counted (FJamCameraFilter::LastKeepYawDeg; the probe's yaw_changed).
	// 2 = never the yaw (the default): the limited move alone; a frame it does not yet hold him is a frame his chest is outside the fraction (measured: 40 and
	// 15 of 6,000 frames, six in a row at most, never outside the frame). With it the hold cannot move a hand's path: the camera-relative scripted human plays
	// the same match to the hash with the hold on and off. 1 = the yaw as that guard (chest lost 0; the camera turned on 76 and 24 frames and his track
	// was 0.9 m off two seconds later). 0 = evolution 3's hold (one slide, then the yaw: 395 and 150 frames, 3 m off). From jam.camera.KeepNoYaw.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	int32 KeepNoYaw = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float KeepStepM = 0.6f;

	// Lane A135, evolution 4: the net the human attacks is kept in the frame while he is inside the blue line and the camera looks its way (the net's mouth
	// within NetKeepBearingDeg of the camera's yaw: what a camera that may not turn can show). 0 = off (the neutral value, the tests' default); the game
	// camera sets it from jam.camera.KeepNet. The rule is in KeepHumanInFrame: the pitch looks as far up as the chest's budget lets it, and when that is
	// not enough the crane gives height back (the camera comes down behind him as far as the boards allow) by at most KeepRiseM a frame.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float KeepNetFrac = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float NetKeepBearingDeg = 30.f;

	// With the boards right behind him the camera is close, and even at the rig's height the chest and the net do not fit the frame's height together: for
	// the net the camera may then come down to this height (an over-the-shoulder view at the attacked net).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float KeepNetMinHeightM = 2.2f;

	// The far net is kept in frame when the human is within this distance of it.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jam|Camera")
	float NetFrameNearM = 16.f;

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
	// A111: the corner weight (0 open ice, 1 at a board the camera looks at) follows its target through a spring of twice the time constant.
	FJamSpring Corner;
	// A135: the height the keep-human stage gave last frame (0 = none yet, or the stage is off): its crane moves from it by at most KeepRiseM a frame.
	float KeepHeightM = 0.f;
	// A135, evolution 4, diagnostics of the last step (not state): the camera's yaw as the rig gave it and as the hold left it, and the difference
	// (0 wherever the hold is position, height and pitch alone). The camera-relative scripted human reads LastYawOutDeg.
	float LastYawRigDeg = 0.f;
	float LastYawOutDeg = 0.f;
	float LastKeepYawDeg = 0.f;
	// A135, evolution 4: the hold's own move of the foot on the last frame (its point minus the rig's), which the next frame may change by KeepStepM at most.
	FVector2D KeepOffsetM = FVector2D::ZeroVector;
	bool bKeepOffset = false;

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
	// Lane A64: an exposure bias in EV on the camera's own post-process override (0 = none); the look switcher sets it for a tier without a light rig.
	void SetExposureBias(float EV);

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

	// A111. The corner weight before the spring, 0..1: the largest over the four boards of nearness (smoothstep from CornerNearM down to CornerFullM) times
	// facing (smoothstep of the camera's forward along the board's outward normal, full from 0.7). Open ice, or skating along or away from a board: 0.
	static float CornerWeight(const FJamCameraSettings& C, const FVector2D& PivotM, float YawCamDeg, const FJamCameraRink& Rink);

	// A135. The last stage of StepThirdPerson when C.KeepHumanFrac > 0 (see FJamCameraSettings::KeepHumanFrac): the crane, the step, the yaw and the pitch.
	// PivotM is the filtered pivot (the crane reads the arm from it, so open ice is the rig to the bit), HumanM the skater itself (the chest the frame must
	// hold), PrevHeightM the height this stage gave on the frame before (0 = none: no limit on the rise). In and out: the camera's point on the ice (metres),
	// its height (metres), its pitch and its yaw (degrees). Pure: the one value of state is the caller's.
	static void KeepHumanInFrame(const FJamCameraSettings& C, const FJamCameraRink& Rink, const FVector2D& PivotM, const FVector2D& HumanM, float PrevHeightM,
		FVector2D& InOutPosM, float& InOutHeightM, float& InOutPitchDeg, float& InOutYawDeg, const FVector2D* NetM = nullptr, const FVector2D* PrevOffsetM = nullptr);

	// A111. The share of the frame that is ice: of Cols x Rows rays through the frame (horizontal FOV, aspect), the fraction that meet z = 0 inside the boards.
	static float IceShare(const FTransform& Pose, float HFovDeg, float Aspect, const FJamCameraRink& Rink, int32 Cols = 16, int32 Rows = 9);

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

	// Lane A73 (B-to-A-0036 point 4): the kick along the hit direction on a knockdown the human lands or takes. KickFrames sim frames: the camera is displaced
	// KickCm along the hit's Dir (world x/y, the push checker -> victim) times max(0.5, Strength) times jam.hitkick, decaying (1 - i / 8)^1.5 from the hit frame,
	// with KickRollDeg of roll toward the side the push has in the camera's frame; the displaced point is clamped back inside A40's boards inset, and 12 cm at 6 m
	// moves the skater by 0.03 of the frame, inside A49's middle-third budget (0.333; the worst measured was 0.237). Replaces A33's punch when the event carries
	// a Dir (every check and knockdown does since A47); the punch stays for a Dir of zero. Pure for tests: FrameIndex 0..7 -> the offsets, anything else -> 0.
	static constexpr int32 KickFrames = 8;
	static constexpr float KickCm = 12.f;
	static constexpr float KickRollDeg = 1.f;
	static void KickOffset(int32 FrameIndex, float Strength, const FVector2D& DirWorld, float CamYawDeg, float Scale, FVector& OutOffsetCm, float& OutRollDeg, bool bBounce = false);
	UFUNCTION(BlueprintCallable, Category = "Jam|Camera")
	void StartKick(float Strength, FVector2D DirWorld, float Scale = 1.f);
	// Lane A101 (track F1): a stumble kicks too, by its ratio: the kick's scale runs StumbleKickMin (3 cm) at bump_ratio to StumbleKickMax (6 cm, a knockdown's
	// smallest) at down_ratio. Pure for tests. The camera reads the human's ContactOutcome / ContactBy / bRecoilThisFrame from the sim each sim frame
	// (ContactKickObserve): a stumble the human takes, one it lands (replacing the knockdown-sized kick the `check` event started), and its own bounce.
	static constexpr float StumbleKickMin = 0.5f;
	static constexpr float StumbleKickMax = 1.0f;
	static constexpr float StumbleKickStrength = 0.5f;
	static float StumbleKickScale(float Ratio, float BumpRatio, float DownRatio);
	float GetKickScale() const { return KickScale; }
	// Lane A132 (track F1, evolution 4): the hit has a size. A knockdown the momentum model resolved kicks by the frames its victim lies: DownKickMin of KickCm
	// (6 cm) at contact.down_frames_min (60) to the whole KickCm (12 cm) at down_frames_max (150), in between by the line. The event's strength (closing speed
	// over top speed) sized it before, so a slow cross-check to the chest that floored a standing skater kicked like a brush. Pure for tests.
	static constexpr float DownKickMin = 0.5f;
	static constexpr float DownKickMax = 1.0f;
	static float DownKickScale(int32 KnockFrames, int32 FramesMin, int32 FramesMax);
	// A braced bounce reads as a bounce: the human's own recoil off a more stable skater kicks the camera along the push on it (against its travel), as far as
	// the hardest stumble (6 cm), and the camera then swings back PAST rest once (about a quarter of the way) before it settles: KickOffset's bBounce shape
	// (1 - t)^1.5 x cos(1.5 pi t), one change of sign, against a hit's (1 - t)^1.5 that only settles. Not a Blueprint call (StartKick is the hit's).
	void StartBounceKick(FVector2D DirWorld);
	bool IsKickBounce() const { return bKickBounce; }
	int32 GetKickFramesLeft() const { return KickLeft; }

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
	// Lane A73 kick state (stepped per sim frame in StepSimFrame).
	int32 KickLeft = 0;
	float KickStrength = 0.f;
	float KickScale = 1.f;   // lane A101: the stumble's share of the kick (1 for a knockdown)
	bool bKickBounce = false;   // lane A132: the running kick is the human's own bounce (the rebound shape)
	void ContactKickObserve();
	FVector2D KickDir = FVector2D(1, 0);
	UPROPERTY(Transient) TObjectPtr<UJamEventBus> Bus;

	// Lane A135: the probe's counts (jam.camera.Probe), from the first sim frame the probe saw.
	struct FKeepProbe
	{
		int64 Frames = 0, OutThird = 0, OutFrame = 0, Carry = 0, CarryPuckOut = 0, Zone = 0, ZoneNetOut = 0, Pulled = 0, Craned = 0;
		int64 YawChanged = 0, Facing = 0, FacingNetOut = 0;   // evolution 4: frames the hold turned the camera; in the zone with the net within 30 degrees of the yaw
		float MaxKeepYaw = 0.f;
		float MaxX = 0.f, MaxY = 0.f, MaxStepCm = 0.f, MaxPitchStep = 0.f, MaxYawStep = 0.f, MaxHeightM = 0.f;
		FVector PrevLoc = FVector::ZeroVector;
		float PrevPitch = 0.f, PrevYaw = 0.f;
		FVector2D PrevHuman = FVector2D::ZeroVector;
		bool bHavePrev = false;
	};
	FKeepProbe Probe;
	FJamCameraSettings KeepLive;   // the settings of the last third-person step (the probe reads the fraction and the aspect)
	void ProbeObserve(const FJamPresentationFrame& Frame, const FJamCameraSettings& Live);
};
