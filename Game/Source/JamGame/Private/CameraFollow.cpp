#include "CameraFollow.h"
#include "Engine/World.h"
#include "JamCartridge.h"

FJamCameraSettings FJamCameraSettings::FromCartridge(const FJamCameraConfig& C)
{
	FJamCameraSettings S;
	S.Mode = C.Mode == TEXT("arena") ? EJamCameraMode::Arena : EJamCameraMode::ThirdPerson;
	S.DistanceM = (float)C.DistanceM;
	S.HeightM = (float)C.HeightM;
	S.LookAheadM = (float)C.LookAheadM;
	S.LagFrames = C.LagFrames;
	S.FovDeg = (float)C.FovDeg;
	return S;
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

UJamFollowCamera::UJamFollowCamera()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	bUsePawnControlRotation = false;
	SetAbsolute(true, true, true);
	FieldOfView = 70.f;
}

void UJamFollowCamera::ApplyConfig(const FJamCameraSettings& InConfig)
{
	Config = InConfig;
	FieldOfView = Config.FovDeg;
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
	if (UJamPresentation* P = UJamPresentation::Get(this))
	{
		FrameHandle = P->OnSimFrame.AddUObject(this, &UJamFollowCamera::StepSimFrame);
		if (P->HasFrame())
		{
			StepSimFrame(P->GetFrame());
		}
	}
}

void UJamFollowCamera::EndPlay(const EEndPlayReason::Type Reason)
{
	if (UJamPresentation* P = UJamPresentation::Get(this))
	{
		P->OnSimFrame.Remove(FrameHandle);
	}
	Super::EndPlay(Reason);
}

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
		const FJamPresentationSkater& S = Frame.Skaters[Frame.HumanIndex];
		Filter.Step(S.PosM, S.YawDeg, Config.LagFrames);
		PoseCurrent = ComputeThirdPersonPose(Config, Filter.PivotM, Filter.YawDeg);
	}
	if (!bHavePose)
	{
		PosePrev = PoseCurrent;
		bHavePose = true;
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
