#include "CameraFollow.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HAL/IConsoleManager.h"
#include "JamCartridge.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "UnrealClient.h"
#include "LookTierSwitcher.h"

// A40: 1 = the H11 camera (pull in along the arm to 0.3 m inside a rectangle, no lift, no yaw rule, no far-net lead). Presentation only; the before stills of CAMERA.md use it.
static TAutoConsoleVariable<int32> CVarJamCameraLegacy(TEXT("jam.camera.Legacy"), 0,
	TEXT("1 = the pre-A40 chase camera (rectangle pull-in only). Presentation only, never read by the sim."));
// Lane A73: the kick's scale (0 = no kick, 1 = 12 cm at full strength, up to 3). Presentation only.
static TAutoConsoleVariable<float> CVarJamHitKick(TEXT("jam.hitkick"), 1.f,
	TEXT("Scale of the camera kick along the hit direction on a knockdown that involves the human (0..3, default 1 = 12 cm over 8 frames at full strength). Presentation only."));

FJamCameraSettings FJamCameraSettings::FromCartridge(const FJamCameraConfig& C)
{
	FJamCameraSettings S;
	S.Mode = C.Mode == TEXT("arena") ? EJamCameraMode::Arena : EJamCameraMode::ThirdPerson;
	S.DistanceM = (float)C.DistanceM;
	S.HeightM = (float)C.HeightM;
	S.LookAheadM = (float)C.LookAheadM;
	S.LagFrames = C.LagFrames;
	S.FovDeg = (float)C.FovDeg;
	S.BoardsMarginM = (float)C.BoardsMarginM;
	S.EndHeightM = (float)C.EndHeightM;
	S.FarNetBias = (float)C.FarNetBias;
	return S;
}

// Lane A64: the game camera may keep a loose puck and the near far net inside this many degrees of its yaw. Default 0 = off: a yaw lead feeds back
// through the A49 cursor aim and the camera-relative stick (measured, see FJamCameraSettings::KeepYawDeg). Opt-in only.
static TAutoConsoleVariable<float> CVarJamCameraKeepYaw(TEXT("jam.camera.KeepYaw"), 0.f,
	TEXT("Degrees off the camera yaw a loose puck / the near far net may sit (0 = off, the default; a lead turns the cursor-aimed skater). Presentation only."));

void FJamSpring::Reset(float Value)
{
	X = Value;
	V = 0.f;
	bInit = true;
}

void FJamSpring::Step(float Target, float Omega)
{
	if (!bInit || Omega <= 0.f)
	{
		Reset(Target);
		return;
	}
	// Critically damped, exact over one frame (dt = 1): x = T + (j0 + j1) e^-w, v = (v - j1 w) e^-w with j0 = x - T, j1 = v + w j0.
	const float E = FMath::Exp(-Omega);
	const float J0 = X - Target;
	const float J1 = V + Omega * J0;
	X = Target + (J0 + J1) * E;
	V = (V - J1 * Omega) * E;
}

float FJamCameraFilter::AlphaForLag(int32 LagFrames)
{
	return LagFrames <= 0 ? 1.f : 1.f - FMath::Exp(-1.f / (float)LagFrames);
}

void FJamCameraFilter::Reset(const FVector2D& TargetM, float TargetYawDeg)
{
	PivotM = TargetM;
	YawDeg = TargetYawDeg;
	bInit = true;
}

void FJamCameraFilter::Step(const FVector2D& TargetM, float TargetYawDeg, int32 LagFrames)
{
	if (!bInit)
	{
		Reset(TargetM, TargetYawDeg);
		return;
	}
	const float A = AlphaForLag(LagFrames);
	PivotM += (TargetM - PivotM) * A;
	YawDeg += FMath::UnwindDegrees(TargetYawDeg - YawDeg) * A;
	YawDeg = FMath::UnwindDegrees(YawDeg);
}

FJamCameraRink FJamCameraRink::FromFrame(const FJamPresentationFrame& Frame)
{
	FJamCameraRink R;
	R.LengthM = Frame.RinkLengthM;
	R.WidthM = Frame.RinkWidthM;
	R.CornerRadiusM = FMath::Max(0.f, Frame.CornerRadiusM);
	return R;
}

UJamFollowCamera::UJamFollowCamera()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	bUsePawnControlRotation = false;
	SetAbsolute(true, true, true);
	FieldOfView = 70.f;
	// Lane A47 (B-to-A-0030 fault 1): the engine's default motion blur smears every camera turn; the follow camera carries its own post-process
	// override so the frame is sharp whatever rig volume is under it (B's three rigs set 0 in their volumes too). Presentation only.
	PostProcessBlendWeight = 1.f;
	PostProcessSettings.bOverride_MotionBlurAmount = true;
	PostProcessSettings.MotionBlurAmount = 0.f;
}

void UJamFollowCamera::ApplyConfig(const FJamCameraSettings& InConfig)
{
	Config = InConfig;
	FieldOfView = Config.FovDeg;
}

void UJamFollowCamera::SetExposureBias(float EV)
{
	PostProcessSettings.bOverride_AutoExposureBias = EV != 0.f;
	PostProcessSettings.AutoExposureBias = EV;
}

void UJamFollowCamera::ApplyConfig(const FJamCameraConfig& Cartridge)
{
	ApplyConfig(FJamCameraSettings::FromCartridge(Cartridge));
}

float UJamFollowCamera::DerivedPitchDeg(const FJamCameraSettings& C)
{
	return -FMath::RadiansToDegrees(FMath::Atan(C.HeightM / (C.DistanceM + C.LookAheadM)));
}

FTransform UJamFollowCamera::ComputeThirdPersonPose(const FJamCameraSettings& C, const FVector2D& PivotM, float YawDeg)
{
	const float Yaw = FMath::DegreesToRadians(YawDeg);
	const FVector2D Fwd(FMath::Cos(Yaw), FMath::Sin(Yaw));
	const FVector2D Right(-FMath::Sin(Yaw), FMath::Cos(Yaw));
	const FVector2D Pos = PivotM - Fwd * C.DistanceM + Right * C.OffAxisM;
	return FTransform(FRotator(DerivedPitchDeg(C), YawDeg, 0.f), FVector(Pos.X * 100.f, Pos.Y * 100.f, C.HeightM * 100.f));
}

namespace
{
	// The inset region: half extents and the corner arc radius after the margin.
	struct FInset
	{
		double Hx, Hy, R;
		FInset(const FJamCameraRink& Rink, float MarginM)
		{
			Hx = FMath::Max(0.0, (double)Rink.LengthM * 0.5 - MarginM);
			Hy = FMath::Max(0.0, (double)Rink.WidthM * 0.5 - MarginM);
			R = FMath::Clamp((double)Rink.CornerRadiusM - MarginM, 0.0, FMath::Min(Hx, Hy));
		}
		bool InCornerSquare(const FVector2D& P) const { return R > 0.0 && FMath::Abs(P.X) > Hx - R && FMath::Abs(P.Y) > Hy - R; }
		FVector2D CornerCentre(const FVector2D& P) const { return FVector2D(FMath::Sign(P.X) * (Hx - R), FMath::Sign(P.Y) * (Hy - R)); }
	};
}

bool UJamFollowCamera::IsInsideBoards(const FVector2D& P, const FJamCameraRink& Rink, float MarginM)
{
	const FInset I(Rink, MarginM);
	if (FMath::Abs(P.X) > I.Hx || FMath::Abs(P.Y) > I.Hy)
	{
		return false;
	}
	return !I.InCornerSquare(P) || FVector2D::DistSquared(P, I.CornerCentre(P)) <= I.R * I.R + 1e-9;
}

FVector2D UJamFollowCamera::NearestInsideBoards(const FVector2D& P, const FJamCameraRink& Rink, float MarginM)
{
	const FInset I(Rink, MarginM);
	FVector2D Q(FMath::Clamp(P.X, -I.Hx, I.Hx), FMath::Clamp(P.Y, -I.Hy, I.Hy));
	if (I.InCornerSquare(Q))
	{
		const FVector2D K = I.CornerCentre(Q);
		const FVector2D D = Q - K;
		const double Len = D.Size();
		if (Len > I.R && Len > 1e-9)
		{
			Q = K + D * (I.R / Len);
		}
	}
	return Q;
}

FVector2D UJamFollowCamera::ClampToBoards(const FVector2D& Want, const FJamCameraRink& Rink, float MarginM, float ArmM, float& OutS, FVector2D& OutDir)
{
	const FVector2D P = NearestInsideBoards(Want, Rink, MarginM);
	const FVector2D D = P - Want;
	const double Len = D.Size();
	OutS = ArmM > 1e-6f ? (float)FMath::Clamp(Len / ArmM, 0.0, 1.0) : 0.f;
	OutDir = Len > 1e-9 ? D / Len : FVector2D::ZeroVector;
	return P;
}

FVector UJamFollowCamera::ClampInsideBoards(const FVector& Focus, const FVector& Cam, float L, float W, float Margin)
{
	const double Hx = (L * 0.5f - Margin) * 100.0, Hy = (W * 0.5f - Margin) * 100.0;
	double T = 1.0;
	const FVector D = Cam - Focus;
	auto Axis = [&](double F, double Dd, double H)
	{
		if (Dd > KINDA_SMALL_NUMBER && F + Dd > H) { T = FMath::Min(T, (H - F) / Dd); }
		else if (Dd < -KINDA_SMALL_NUMBER && F + Dd < -H) { T = FMath::Min(T, (-H - F) / Dd); }
	};
	Axis(Focus.X, D.X, Hx);
	Axis(Focus.Y, D.Y, Hy);
	return Focus + D * FMath::Clamp(T, 0.0, 1.0);
}

float UJamFollowCamera::FarNetLeadDeg(const FJamCameraSettings& C, const FVector2D& PivotM, float YawDeg, const FVector2D& FarNetM)
{
	const FVector2D ToNet = FarNetM - PivotM;
	const double Dist = ToNet.Size();
	if (Dist < 1e-3 || C.FarNetBias <= 0.f)
	{
		return 0.f;
	}
	const float YawToNet = FMath::RadiansToDegrees(FMath::Atan2(ToNet.Y, ToNet.X));
	const float Delta = FMath::UnwindDegrees(YawToNet - YawDeg);
	const float Yaw = FMath::DegreesToRadians(YawDeg);
	const float Toward = FMath::Clamp((float)((FMath::Cos(Yaw) * ToNet.X + FMath::Sin(Yaw) * ToNet.Y) / Dist), 0.f, 1.f);
	const float Band = FMath::Clamp((float)(Dist - C.FarNetBiasNearM) / FMath::Max(C.FarNetBiasFarM - C.FarNetBiasNearM, 1e-3f), 0.f, 1.f);
	return C.FarNetBias * Toward * Band * Delta;
}

FTransform UJamFollowCamera::StepThirdPerson(const FJamCameraSettings& C, FJamCameraFilter& State, const FJamPresentationFrame& Frame)
{
	const FJamPresentationSkater& S = Frame.Skaters[Frame.HumanIndex];
	// Lane A49: a teleport (the faceoff reset after a goal moves the skaters to the dots, 5 m and more in one frame) snaps the filter instead of lagging behind it:
	// the human is the pivot every frame, and the 17 frames of "skater behind the camera" after a goal (packaged run bindings4) do not happen.
	if (State.bInit && FVector2D::Distance(S.PosM, State.PivotM) > 5.0) { State.Reset(S.PosM, S.YawDeg); State.YawOffsetDeg.bInit = false; State.Lift.bInit = false; State.PitchDeg.bInit = false; }
	State.Step(S.PosM, S.YawDeg, C.LagFrames);
	const FJamCameraRink Rink = FJamCameraRink::FromFrame(Frame);
	const FVector NetCm = Frame.GetFarNetWorldCm();
	const float Lead = FarNetLeadDeg(C, State.PivotM, State.YawDeg, FVector2D(NetCm.X / 100.0, NetCm.Y / 100.0));
	if (!State.YawOffsetDeg.bInit) { State.YawOffsetDeg.Reset(Lead); }
	if (!State.Lift.bInit) { State.Lift.Reset(0.f); }
	const float Omega = C.SpringFrames > 0.f ? 1.f / C.SpringFrames : 0.f;

	// The desired camera point for a yaw (the F1 arm).
	auto Desired = [&](float YawCamDeg) -> FVector2D
	{
		const float Y = FMath::DegreesToRadians(YawCamDeg);
		const FVector2D Fwd(FMath::Cos(Y), FMath::Sin(Y)), Right(-FMath::Sin(Y), FMath::Cos(Y));
		return State.PivotM - Fwd * C.DistanceM + Right * C.OffAxisM;
	};

	// 1. Targets from the arm as it stands (the spring's current yaw offset).
	float S0 = 0.f;
	FVector2D N0 = FVector2D::ZeroVector;
	const FVector2D P0 = ClampToBoards(Desired(FMath::UnwindDegrees(State.YawDeg + State.YawOffsetDeg.X)), Rink, C.BoardsMarginM, C.DistanceM, S0, N0);
	// Along the rink at an end board: fades in (smoothstep) over the last max(arc radius, 3 m) before an end face (a sharp-cornered rink cannot flip it), weighted by the
	// arm the boards took and by how much the skater already heads along the rink (a skater crossing the end zone keeps his own framing: no whip behind the net).
	const float Hx = FMath::Max(0.f, Rink.LengthM * 0.5f - C.BoardsMarginM);
	const float Ramp = FMath::Max(Rink.CornerRadiusM - C.BoardsMarginM, 3.f);
	float WEnd = FMath::Clamp((FMath::Abs((float)P0.X) - (Hx - Ramp)) / Ramp, 0.f, 1.f);
	WEnd = WEnd * WEnd * (3.f - 2.f * WEnd);
	const float YawBase = State.YawDeg + Lead;
	const float Along = P0.X < 0.0 ? 0.f : 180.f;  // away from the end board the camera stands at
	const float DeltaAlong = FMath::UnwindDegrees(Along - YawBase);
	const float WHead = FMath::Clamp(FMath::Cos(FMath::DegreesToRadians(DeltaAlong)), 0.f, 1.f);
	float OffsetTarget = Lead + S0 * WEnd * WHead * DeltaAlong;
	// Lane A64: keep the far net (near the far end) and then a loose puck inside KeepYawDeg of the camera yaw, the puck last so it wins over the net;
	// the human's middle-third clamp below still has the last word. A point behind the camera (more than 100 degrees off) is not chased.
	if (C.KeepYawDeg > 0.f)
	{
		auto Keep = [&](const FVector2D& PointM)
		{
			const FVector2D To = PointM - P0;
			if (To.Size() < 1.0) return;
			const float Bearing = FMath::RadiansToDegrees((float)FMath::Atan2(To.Y, To.X));
			const float Rel = FMath::UnwindDegrees(State.YawDeg + OffsetTarget - Bearing);
			if (FMath::Abs(Rel) > 100.f) return;
			OffsetTarget += FMath::Clamp(Rel, -C.KeepYawDeg, C.KeepYawDeg) - Rel;
		};
		const FVector2D NetM(NetCm.X / 100.0, NetCm.Y / 100.0);
		if (FVector2D::Distance(State.PivotM, NetM) < C.NetFrameNearM) Keep(NetM);
		const float PuckDist = (float)FVector2D::Distance(State.PivotM, Frame.PuckM);
		if (PuckDist > C.PuckFrameMinM && PuckDist < C.PuckFrameMaxM) Keep(Frame.PuckM);
	}
	// Lane A49: the human stays in the middle third. The offset's target is bounded so the camera yaw stays within MiddleThirdYawDeg of the bearing from the
	// (clamped) camera point to the pivot; the spring then moves smoothly inside that bound (no pop from the budget itself). Skipped when the camera stands on
	// the pivot (a bearing means nothing there).
	{
		const FVector2D ToPivot0 = State.PivotM - P0;
		if (ToPivot0.Size() > 1.0)
		{
			const float Bearing0 = FMath::RadiansToDegrees((float)FMath::Atan2(ToPivot0.Y, ToPivot0.X));
			const float Rel = FMath::UnwindDegrees(State.YawDeg + OffsetTarget - Bearing0);
			OffsetTarget += FMath::Clamp(Rel, -C.MiddleThirdYawDeg, C.MiddleThirdYawDeg) - Rel;
		}
	}
	State.YawOffsetDeg.Step(OffsetTarget, Omega);

	// 2. The arm with the new yaw, and the boards' answer: the nearest point inside (the camera slides along the glass; the position never crosses it).
	const float YawCam = FMath::UnwindDegrees(State.YawDeg + State.YawOffsetDeg.X);
	const FVector2D Want = Desired(YawCam);
	const FVector2D P = ClampToBoards(Want, Rink, C.BoardsMarginM, C.DistanceM, State.LastClampS, State.LastClampNormal);

	// 3. Lift and tilt from the displacement: height toward end_height_m; the look-ahead grows with the lift so the camera looks up the rink, not down at the skater's back.
	State.Lift.Step(State.LastClampS, Omega);
	const float Lift = FMath::Clamp(State.Lift.X, 0.f, 1.f);
	// A49: end_height_m 3 sits under the default height 3.5, so the lift never lowers the camera: the end height is at least the height (the lift is then nothing).
	const float Height = C.HeightM + (FMath::Max(C.EndHeightM, C.HeightM) - C.HeightM) * Lift;
	const float YawR = FMath::DegreesToRadians(YawCam);
	const FVector2D FwdCam(FMath::Cos(YawR), FMath::Sin(YawR));
	const float ArmM = FMath::Max(0.f, (float)((State.PivotM.X - P.X) * FwdCam.X + (State.PivotM.Y - P.Y) * FwdCam.Y));  // along the forward axis: open ice gives exactly the F1 pitch
	const float Extra = C.LiftLookAheadGain * Lift * C.DistanceM;
	float Pitch = -FMath::RadiansToDegrees(FMath::Atan2(Height, FMath::Max(ArmM + C.LookAheadM + Extra, 1e-3f)));

	// 4. Lane A49: the pitch stays within MiddleThirdPitchDeg of the bearing to the chest point (1 m up), through a spring of the same time constant. In open ice
	// the F1 rig is inside the budget (4.9 degrees) and the derived pitch is what comes out; next to the boards the lift's tilt is bounded so the human stays in
	// the frame's middle third. (The yaw budget sits on the offset spring's target above.)
	{
		const FVector2D ToPivot = State.PivotM - P;
		const double Flat = ToPivot.Size();
		if (Flat > 1.0)
		{
			const float ChestPitch = -FMath::RadiansToDegrees(FMath::Atan2(Height - C.ChestM, (float)Flat));
			Pitch = ChestPitch + FMath::Clamp(Pitch - ChestPitch, -C.MiddleThirdPitchDeg, C.MiddleThirdPitchDeg);
		}
		if (!State.PitchDeg.bInit) State.PitchDeg.Reset(Pitch);
		State.PitchDeg.Step(Pitch, Omega);
		Pitch = State.PitchDeg.X;
	}
	return FTransform(FRotator(Pitch, YawCam, 0.f), FVector(P.X * 100.0, P.Y * 100.0, Height * 100.0));
}

FVector2D UJamFollowCamera::FrameFraction(const FTransform& Pose, float HFovDeg, float Aspect, const FVector& WorldCm)
{
	const FVector Local = Pose.InverseTransformPositionNoScale(WorldCm);   // x forward, y right, z up
	if (Local.X <= 1e-3) return FVector2D(Local.Y >= 0 ? 9.f : -9.f, Local.Z >= 0 ? 9.f : -9.f);
	const float TanH = FMath::Tan(FMath::DegreesToRadians(FMath::Clamp(HFovDeg, 1.f, 179.f) * 0.5f));
	const float TanV = TanH / FMath::Max(Aspect, 0.1f);
	return FVector2D((float)(Local.Y / Local.X) / TanH, (float)(Local.Z / Local.X) / TanV);
}

FTransform UJamFollowCamera::ComputeArenaPose(const FJamCameraSettings& C, float RinkLengthM, float RinkWidthM)
{
	// Looking -Y from the +Y side (rink +X is then on the right, like the radar) at a downward pitch; the horizontal field of view spans the rink length plus 10 %.
	const float PitchDeg = -52.f;
	const float HalfFov = FMath::DegreesToRadians(C.FovDeg * 0.5f);
	const float Dist = (RinkLengthM * 1.1f * 0.5f) / FMath::Tan(HalfFov);
	const float P = FMath::DegreesToRadians(-PitchDeg);
	const FVector Pos(0.f, Dist * FMath::Cos(P) * 100.f, Dist * FMath::Sin(P) * 100.f);
	return FTransform(FRotator(PitchDeg, -90.f, 0.f), Pos);
}

void UJamFollowCamera::BeginPlay()
{
	Super::BeginPlay();
	FieldOfView = Config.FovDeg;
	if (UWorld* W0 = GetWorld()) if (const ULookTierSwitcher* Look = W0->GetSubsystem<ULookTierSwitcher>()) SetExposureBias(Look->GetCameraExposureBias());   // lane A64
	if (UJamPresentation* P = UJamPresentation::Get(this))
	{
		FrameHandle = P->OnSimFrame.AddUObject(this, &UJamFollowCamera::StepSimFrame);
		if (P->HasFrame())
		{
			StepSimFrame(P->GetFrame());
		}
	}
	// Lane A33: the punch listens to the bus for the human's checks (DeliverEvents runs before PushPresentation, so the kick starts in the same sim frame).
	if (UWorld* W = GetWorld())
	{
		UGameInstance* GI = W->GetGameInstance();
		Bus = GI ? GI->GetSubsystem<UJamEventBus>() : nullptr;
		if (Bus) Bus->OnJamEvent.AddDynamic(this, &UJamFollowCamera::OnJamEvent);
	}
}

void UJamFollowCamera::EndPlay(const EEndPlayReason::Type Reason)
{
	if (UJamPresentation* P = UJamPresentation::Get(this))
	{
		P->OnSimFrame.Remove(FrameHandle);
	}
	if (Bus)
	{
		Bus->OnJamEvent.RemoveDynamic(this, &UJamFollowCamera::OnJamEvent);
		Bus = nullptr;
	}
	Super::EndPlay(Reason);
}

void UJamFollowCamera::PunchOffset(int32 FrameIndex, float Strength, float Sign, float& OutRightCm, float& OutRollDeg)
{
	OutRightCm = 0.f;
	OutRollDeg = 0.f;
	if (FrameIndex < 0 || FrameIndex >= PunchFrames) return;
	const float K = 1.f - (float)FrameIndex / (float)PunchFrames;   // 1, 5/6, ... 1/6, then nothing
	const float S = FMath::Max(0.5f, FMath::Clamp(Strength, 0.f, 1.f));
	const float Sg = Sign < 0.f ? -1.f : 1.f;
	OutRightCm = PunchCm * S * K * Sg;
	OutRollDeg = PunchRollDeg * S * K * Sg;
}

void UJamFollowCamera::StartPunch(float Strength, float Sign)
{
	PunchLeft = PunchFrames;
	PunchStrength = FMath::Clamp(Strength, 0.f, 1.f);
	PunchSign = Sign < 0.f ? -1.f : 1.f;
}

void UJamFollowCamera::KickOffset(int32 FrameIndex, float Strength, const FVector2D& DirWorld, float CamYawDeg, float Scale, FVector& OutOffsetCm, float& OutRollDeg)
{
	OutOffsetCm = FVector::ZeroVector;
	OutRollDeg = 0.f;
	if (FrameIndex < 0 || FrameIndex >= KickFrames || Scale <= 0.f) return;
	const FVector2D D = DirWorld.GetSafeNormal();
	if (D.IsNearlyZero()) return;
	const float K = FMath::Pow(1.f - (float)FrameIndex / (float)KickFrames, 1.5f);   // 1 on the hit frame, a soft settle
	const float S = FMath::Max(0.5f, FMath::Clamp(Strength, 0.f, 1.f)) * FMath::Clamp(Scale, 0.f, 3.f);
	OutOffsetCm = FVector(D.X, D.Y, 0.0) * (KickCm * S * K);
	const float Yaw = FMath::DegreesToRadians(CamYawDeg);
	const FVector2D Right(-FMath::Sin(Yaw), FMath::Cos(Yaw));
	const float Side = FVector2D::DotProduct(D, Right);   // the push's component across the frame decides the roll's side
	OutRollDeg = KickRollDeg * S * K * (Side < 0.f ? -1.f : 1.f) * FMath::Min(1.f, FMath::Abs(Side) * 2.f);
}

void UJamFollowCamera::StartKick(float Strength, FVector2D DirWorld)
{
	KickLeft = KickFrames;
	KickStrength = FMath::Clamp(Strength, 0.f, 1.f);
	KickDir = DirWorld.GetSafeNormal();
	if (KickDir.IsNearlyZero()) KickLeft = 0;
}

void UJamFollowCamera::OnJamEvent(FName Event, const FJamEventPayload& Payload)
{
	// A check the human lands (check, Who = checker) or takes (knockdown, Who = victim, Kind = check). AI checks out of frame kick nothing.
	if (LastHumanIndex < 0 || Payload.Who != LastHumanIndex) return;
	const bool bLanded = Event == JamEvents::Check;
	const bool bTaken = Event == JamEvents::Knockdown && Payload.Kind == FName(TEXT("check"));
	if (!bLanded && !bTaken) return;
	// Lane A73: the kick along the hit direction when the event carries one (A47's Dir: the push, checker towards victim); the punch below stays for a Dir of zero.
	const FVector2D Dir2(Payload.Dir.X, Payload.Dir.Y);
	if (!Dir2.IsNearlyZero() && CVarJamHitKick.GetValueOnGameThread() > 0.f)
	{
		StartKick(Payload.Strength, Dir2);
		UE_LOG(LogTemp, Log, TEXT("JAMKICK frame=%lld %s strength=%.2f dir=(%.2f, %.2f) cm=%.1f frames=%d scale=%.2f"), Bus ? Bus->GetSimFrame() : -1,
			bLanded ? TEXT("landed") : TEXT("taken"), Payload.Strength, Dir2.X, Dir2.Y, KickCm * FMath::Max(0.5f, Payload.Strength) * CVarJamHitKick.GetValueOnGameThread(), KickFrames,
			CVarJamHitKick.GetValueOnGameThread());
		return;
	}
	// The side of the contact seen by the camera decides the kick's direction.
	const float Yaw = FMath::DegreesToRadians(Filter.YawDeg);
	const FVector2D Right(-FMath::Sin(Yaw), FMath::Cos(Yaw));
	const FVector2D ToContact(Payload.Where.X / 100.f - LastHumanPosM.X, Payload.Where.Y / 100.f - LastHumanPosM.Y);
	const float Side = FVector2D::DotProduct(ToContact, Right);
	StartPunch(Payload.Strength, Side < 0.f ? -1.f : 1.f);
}

static TAutoConsoleVariable<int32> CVarJamCameraLog(TEXT("jam.camera.Log"), 0,
	TEXT("1 = one JAMCAM line per sim frame: where the human's chest lands in the frame (x, y in -1..1; the middle third is |x|,|y| <= 0.333). Lane A49 proof."));

void UJamFollowCamera::StepSimFrame(const FJamPresentationFrame& Frame)
{
	PosePrev = bHavePose ? PoseCurrent : FTransform::Identity;
	if (Config.Mode == EJamCameraMode::Arena)
	{
		PoseCurrent = ComputeArenaPose(Config, Frame.RinkLengthM, Frame.RinkWidthM);
	}
	else
	{
		if (!Frame.Skaters.IsValidIndex(Frame.HumanIndex))
		{
			return;
		}
		const FJamPresentationSkater& S0 = Frame.Skaters[Frame.HumanIndex];
		LastHumanIndex = Frame.HumanIndex;   // lane A33: the punch reads the contact side from the last human pose
		LastHumanPosM = S0.PosM;
		if (CVarJamCameraLegacy.GetValueOnGameThread() != 0)
		{
			// H11: lag filter, the open-ice rig, pull in along the view axis to 0.3 m inside a rectangle (focus = the filtered pivot at shoulder height).
			const FJamPresentationSkater& S = Frame.Skaters[Frame.HumanIndex];
			Filter.Step(S.PosM, S.YawDeg, Config.LagFrames);
			PoseCurrent = ComputeThirdPersonPose(Config, Filter.PivotM, Filter.YawDeg);
			const FVector Focus(Filter.PivotM.X * 100.0, Filter.PivotM.Y * 100.0, 90.0);
			PoseCurrent.SetLocation(ClampInsideBoards(Focus, PoseCurrent.GetLocation(), Frame.RinkLengthM, Frame.RinkWidthM));
		}
		else
		{
			FJamCameraSettings Live = Config;
			Live.KeepYawDeg = FMath::Max(0.f, CVarJamCameraKeepYaw.GetValueOnGameThread());   // lane A64
			PoseCurrent = StepThirdPerson(Live, Filter, Frame);
		}
	}
	// Lane A33: the punch, one sim frame at a time, on top of the clamped pose (interpolated like the rest by TickComponent).
	if (PunchLeft > 0)
	{
		float RightCm = 0.f, RollDeg = 0.f;
		PunchOffset(PunchFrames - PunchLeft, PunchStrength, PunchSign, RightCm, RollDeg);
		--PunchLeft;
		const FQuat Rot = PoseCurrent.GetRotation();
		PoseCurrent.SetLocation(PoseCurrent.GetLocation() + Rot.GetRightVector() * RightCm);
		PoseCurrent.SetRotation(Rot * FRotator(0.f, 0.f, RollDeg).Quaternion());
	}
	// Lane A73: the kick along the hit direction, one sim frame at a time; the displaced point never leaves A40's boards inset (third-person mode only).
	if (KickLeft > 0)
	{
		FVector OffsetCm = FVector::ZeroVector;
		float RollDeg = 0.f;
		KickOffset(KickFrames - KickLeft, KickStrength, KickDir, PoseCurrent.Rotator().Yaw, CVarJamHitKick.GetValueOnGameThread(), OffsetCm, RollDeg);
		--KickLeft;
		FVector Loc = PoseCurrent.GetLocation() + OffsetCm;
		if (Config.Mode != EJamCameraMode::Arena)
		{
			const FVector2D Inside = NearestInsideBoards(FVector2D(Loc.X / 100.0, Loc.Y / 100.0), FJamCameraRink::FromFrame(Frame), Config.BoardsMarginM);
			Loc.X = Inside.X * 100.0;
			Loc.Y = Inside.Y * 100.0;
		}
		const FQuat Rot = PoseCurrent.GetRotation();
		PoseCurrent.SetLocation(Loc);
		PoseCurrent.SetRotation(Rot * FRotator(0.f, 0.f, RollDeg).Quaternion());
	}
	if (!bHavePose)
	{
		PosePrev = PoseCurrent;
		bHavePose = true;
	}
	if (CVarJamCameraLog.GetValueOnGameThread() != 0 && Frame.Skaters.IsValidIndex(Frame.HumanIndex))
	{
		const FJamPresentationSkater& S = Frame.Skaters[Frame.HumanIndex];
		float Aspect = 16.f / 9.f;
		if (GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport)
		{
			const FIntPoint Sz = GEngine->GameViewport->Viewport->GetSizeXY();
			if (Sz.X > 0 && Sz.Y > 0) Aspect = (float)Sz.X / (float)Sz.Y;
		}
		const FVector2D F = FrameFraction(PoseCurrent, Config.FovDeg, Aspect, FVector(S.PosM.X * 100.0, S.PosM.Y * 100.0, Config.ChestM * 100.0));
		UE_LOG(LogTemp, Log, TEXT("JAMCAM frame=%lld x=%.3f y=%.3f third=%d yaw=%.1f pitch=%.1f lift=%.2f clamp=%.2f"), Frame.SimFrame, F.X, F.Y,
			(FMath::Abs(F.X) <= 1.f / 3.f && FMath::Abs(F.Y) <= 1.f / 3.f) ? 1 : 0, PoseCurrent.Rotator().Yaw, PoseCurrent.Rotator().Pitch, Filter.Lift.X, Filter.LastClampS);
	}
}

void UJamFollowCamera::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!bHavePose)
	{
		return;
	}
	float Alpha = 1.f;
	if (bInterpolate)
	{
		if (const UJamPresentation* P = UJamPresentation::Get(this))
		{
			Alpha = P->GetRenderAlpha();
		}
	}
	const FVector Loc = FMath::Lerp(PosePrev.GetLocation(), PoseCurrent.GetLocation(), Alpha);
	const FQuat Rot = FQuat::Slerp(PosePrev.GetRotation(), PoseCurrent.GetRotation(), Alpha);
	SetWorldLocationAndRotation(Loc, Rot);
}
