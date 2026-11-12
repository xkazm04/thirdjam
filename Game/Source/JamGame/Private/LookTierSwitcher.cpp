#include "LookTierSwitcher.h"

#include "Engine/LevelStreamingDynamic.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "Engine/SkyLight.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMeshActor.h"
#include "Components/LocalLightComponent.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "EngineUtils.h"
#include "RinkActor.h"

DEFINE_LOG_CATEGORY(LogJamLook);

namespace
{
	struct FTierRig { const TCHAR* Tier; const TCHAR* Map; };
	const FTierRig GTierRigs[] = {
		{ TEXT("l1_day"),   TEXT("/JamAssets/Rigs/LR_L1_Day.LR_L1_Day") },
		{ TEXT("l2_night"), TEXT("/JamAssets/Rigs/LR_L2_Night.LR_L2_Night") },
		{ TEXT("l3_neon"),  TEXT("/JamAssets/Rigs/LR_L3_Neon.LR_L3_Neon") },
		{ TEXT("p3_clay"),  TEXT("/JamAssets/Rigs/LR_P3_Clay.LR_P3_Clay") },   // B-to-A-0018 (lane A21)
		{ TEXT("p4_broadcast"), TEXT("/JamAssets/Rigs/LR_P4_Broadcast.LR_P4_Broadcast") },   // B-to-A-0025 (lane A31)
	};
	// B-to-A-0018 point 5: trim Emissive per tier; on bright clay ice 30 reads as a thin gold line, so clay ships 12 (B's 10..15). Broadcast 30 (B-to-A-0025).
	struct FTierTrim { const TCHAR* Tier; float Emissive; };
	const FTierTrim GTierTrim[] = {
		{ TEXT("l1_day"),   30.f },
		{ TEXT("l2_night"), 30.f },
		{ TEXT("l3_neon"),  30.f },
		{ TEXT("p3_clay"),  12.f },
		{ TEXT("p4_broadcast"), 30.f },
	};
	// B-to-A-0025 point 3: the shared look collection B's rigs write (Clay = 1 clay, 0 cel, -1 broadcast).
	const TCHAR* LookCollectionPath = TEXT("/JamAssets/Look/MPC_Look.MPC_Look");
	const FName LookClayParam(TEXT("Clay"));
}

TArray<FString> ULookTierSwitcher::GetKnownTiers()
{
	TArray<FString> Out;
	for (const FTierRig& R : GTierRigs) { Out.Add(R.Tier); }
	return Out;
}

FString ULookTierSwitcher::GetRigMapPath(const FString& Tier)
{
	for (const FTierRig& R : GTierRigs)
	{
		if (Tier.Equals(R.Tier, ESearchCase::CaseSensitive)) { return R.Map; }
	}
	return FString();
}

float ULookTierSwitcher::DefaultTrimEmissiveFor(const FString& Tier)
{
	for (const FTierTrim& T : GTierTrim)
	{
		if (Tier.Equals(T.Tier, ESearchCase::CaseSensitive)) { return T.Emissive; }
	}
	return 30.f;
}

float ULookTierSwitcher::GetTrimEmissiveFor(const FString& Tier) const
{
	return bTrimOverride ? TrimOverride : DefaultTrimEmissiveFor(Tier);
}

float ULookTierSwitcher::GetTrimEmissive() const
{
	return GetTrimEmissiveFor(CurrentTier.IsEmpty() ? FString(TEXT("l2_night")) : CurrentTier);
}

void ULookTierSwitcher::SetTrimEmissiveOverride(float Emissive)
{
	bTrimOverride = Emissive >= 0.f;
	TrimOverride = FMath::Clamp(Emissive, 0.f, 100.f);
	PushTrimEmissive();
}

void ULookTierSwitcher::PushTrimEmissive()
{
	// The rink reads the value at Build; a tier change between builds (a Director's Call, the Settings override) pushes it here.
	UWorld* W = GetWorld();
	if (!W) return;
	const float E = GetTrimEmissive();
	for (TActorIterator<ARinkActor> It(W); It; ++It)
	{
		if (*It) It->SetTrimEmissive(E);
	}
}

float ULookTierSwitcher::RigScaleFor(float RinkLengthM)
{
	if (!(RinkLengthM > 0.f)) return 1.f;
	return FMath::Clamp(RinkLengthM / static_cast<float>(RigRinkLengthM), 0.25f, 4.f);
}

bool ULookTierSwitcher::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

bool ULookTierSwitcher::SetTier(const FString& Tier, float RinkLengthM)
{
	const FString MapPath = GetRigMapPath(Tier);
	if (MapPath.IsEmpty())
	{
		UE_LOG(LogJamLook, Error, TEXT("JAMLOOK_UNKNOWN tier '%s' is not in contracts/look-tiers.md; nothing changed"), *Tier);
		return false;
	}
	const float Scale = RigScaleFor(RinkLengthM);
	if (Tier == CurrentTier && LoadedRigs.Contains(Tier) && FMath::IsNearlyEqual(Scale, CurrentScale, 1e-3f))
	{
		return true;
	}

	// Unload every other rig (B-to-A-0018 point 4: unloaded, not hidden, so B's BP_LookParams EndPlay restores MPC_Look), and this
	// tier's rig too when the scale changed (a level transform is fixed at load: R-0016 point 5).
	for (auto It = LoadedRigs.CreateIterator(); It; ++It)
	{
		if (It.Key() == Tier && FMath::IsNearlyEqual(Scale, CurrentScale, 1e-3f)) { continue; }
		if (ULevelStreamingDynamic* Old = It.Value())
		{
			Old->OnLevelLoaded.RemoveDynamic(this, &ULookTierSwitcher::OnRigLoaded);
			Old->SetShouldBeVisible(false);
			Old->SetShouldBeLoaded(false);
			Old->SetIsRequestingUnloadAndRemoval(true);
		}
		It.RemoveCurrent();
	}

	// B-to-A-0025 point 3 (lane A31): MPC_Look.Clay back to 0 before the next rig streams, so a rig without a BP_LookParams (the neon rig) never keeps the
	// previous tier's surface response; a rig that has one writes its own value on BeginPlay afterwards.
	ResetLookCollection();

	if (!LoadedRigs.Contains(Tier))
	{
		FString PackageName = MapPath;
		int32 Dot = INDEX_NONE;
		if (PackageName.FindChar(TEXT('.'), Dot)) PackageName.LeftInline(Dot);
		FTransform Xf(FRotator::ZeroRotator, FVector::ZeroVector, FVector(Scale));
		ULevelStreamingDynamic::FLoadLevelInstanceParams Params(GetWorld(), PackageName, Xf);
		bool bOk = false;
		ULevelStreamingDynamic* Rig = ULevelStreamingDynamic::LoadLevelInstance(Params, bOk);
		if (!bOk || !Rig)
		{
			UE_LOG(LogJamLook, Error, TEXT("JAMLOOK_FAIL tier=%s map=%s could not be streamed in"), *Tier, *MapPath);
			CurrentTier.Reset();
			return false;
		}
		Rig->OnLevelLoaded.AddUniqueDynamic(this, &ULookTierSwitcher::OnRigLoaded);
		LoadedRigs.Add(Tier, Rig);
		ScaledLights = 0;
		if (Rig->GetLoadedLevel()) ApplyRigScale(Rig, Scale);   // already resident (a re-stream of a cached level)
	}
	CurrentTier = Tier;
	CurrentScale = Scale;
	HidePersistentLights(true);
	LowerFloor();
	PushTrimEmissive();
	PushGlass();
	UE_LOG(LogJamLook, Log, TEXT("JAMLOOK tier=%s map=%s scale=%.3f (rink %.1f m) trim_emissive=%.1f"), *Tier, *MapPath, Scale, RinkLengthM, GetTrimEmissive());
	return true;
}

void ULookTierSwitcher::OnRigLoaded()
{
	if (TObjectPtr<ULevelStreamingDynamic>* Rig = LoadedRigs.Find(CurrentTier))
	{
		if (*Rig && (*Rig)->GetLoadedLevel()) ApplyRigScale(*Rig, CurrentScale);
	}
	UE_LOG(LogJamLook, Log, TEXT("JAMLOOK rig %s loaded: MPC_Look.Clay=%.1f (BP_LookParams writes its value on BeginPlay, read again by GetLookClay)"), *CurrentTier, GetLookClay());
}

void ULookTierSwitcher::ResetLookCollection()
{
	UWorld* W = GetWorld();
	UMaterialParameterCollection* MPC = LoadObject<UMaterialParameterCollection>(nullptr, LookCollectionPath, nullptr, LOAD_NoWarn | LOAD_Quiet);
	if (!W || !MPC) return;
	if (UMaterialParameterCollectionInstance* Inst = W->GetParameterCollectionInstance(MPC))
	{
		Inst->SetScalarParameterValue(LookClayParam, 0.f);
	}
}

float ULookTierSwitcher::GetLookClay() const
{
	UWorld* W = GetWorld();
	UMaterialParameterCollection* MPC = LoadObject<UMaterialParameterCollection>(nullptr, LookCollectionPath, nullptr, LOAD_NoWarn | LOAD_Quiet);
	if (!W || !MPC) return 0.f;
	float V = 0.f;
	if (UMaterialParameterCollectionInstance* Inst = W->GetParameterCollectionInstance(MPC)) Inst->GetScalarParameterValue(LookClayParam, V);
	return V;
}

void ULookTierSwitcher::PushGlass()
{
	// B-to-A-0023 point 2 / 0025: the glass instance follows the tier (MI_Bcast_Glass on p4_broadcast, MI_Glass elsewhere); the rink reads it at Build too.
	UWorld* W = GetWorld();
	if (!W) return;
	for (TActorIterator<ARinkActor> It(W); It; ++It)
	{
		if (*It) It->SetGlassForTier(CurrentTier);
	}
}

void ULookTierSwitcher::ApplyRigScale(ULevelStreamingDynamic* Rig, float Scale)
{
	ULevel* Level = Rig ? Rig->GetLoadedLevel() : nullptr;
	if (!Level) return;
	// Each loaded level instance is fresh, so the authored values are the base; a tag on the component guards against a second pass.
	static const FName ScaledTag(TEXT("JamRigScaled"));
	int32 Count = 0;
	for (AActor* A : Level->Actors)
	{
		if (!A) continue;
		TInlineComponentArray<ULocalLightComponent*> Lights;
		A->GetComponents(Lights);
		for (ULocalLightComponent* L : Lights)
		{
			if (!L || L->ComponentHasTag(ScaledTag)) continue;
			L->SetIntensity(L->Intensity * Scale * Scale);
			L->SetAttenuationRadius(L->AttenuationRadius * Scale);
			L->ComponentTags.Add(ScaledTag);
			++Count;
		}
	}
	ScaledLights = Count;
	UE_LOG(LogJamLook, Log, TEXT("JAMLOOK rig loaded: %d local lights scaled (intensity x %.3f, attenuation x %.3f)"), Count, Scale * Scale, Scale);
}

void ULookTierSwitcher::HidePersistentLights(bool bHide)
{
	// R-0016 point 3 (2): the persistent Rink.umap sky light and the (off) directional light give way to the rig's own.
	UWorld* W = GetWorld();
	if (!W || bPersistentLightsHidden == bHide) return;
	ULevel* Persistent = W->PersistentLevel;
	int32 N = 0;
	for (TActorIterator<AActor> It(W); It; ++It)
	{
		AActor* A = *It;
		if (!A || A->GetLevel() != Persistent) continue;
		if (A->IsA<ASkyLight>() || A->IsA<ADirectionalLight>())
		{
			TInlineComponentArray<ULightComponentBase*> Comps;
			A->GetComponents(Comps);
			for (ULightComponentBase* C : Comps) if (C) C->SetVisibility(!bHide);
			A->SetActorHiddenInGame(bHide);
			++N;
		}
	}
	bPersistentLightsHidden = bHide;
	UE_LOG(LogJamLook, Log, TEXT("JAMLOOK persistent lights %s: %d actors (sky light, directional light)"), bHide ? TEXT("hidden") : TEXT("shown"), N);
}

void ULookTierSwitcher::LowerFloor()
{
	// R-0016 point 3 (3): A's under-ice floor (a large static mesh of the persistent level sitting just under the ice) goes to z = -20 cm
	// so it never shares a plane with B's room floor at -5 cm. Found by shape, not by name: a persistent static mesh wider than 50 m whose
	// top sits within 10 cm under the ice.
	UWorld* W = GetWorld();
	if (!W || bFloorLowered) return;
	int32 N = 0;
	for (TActorIterator<AStaticMeshActor> It(W); It; ++It)
	{
		AStaticMeshActor* A = *It;
		if (!A || A->GetLevel() != W->PersistentLevel) continue;
		FVector Origin, Extent;
		A->GetActorBounds(false, Origin, Extent);
		const double Top = Origin.Z + Extent.Z;
		if (Extent.X < 2500.0 || Extent.Y < 1000.0 || Top > 0.5 || Top < -10.0) continue;
		// B-to-A-0023 point 1 (lane A31): the floor is authored Static, so the offset was refused ("must be Movable") and its bounds stayed at -5 cm.
		// Movable first, then the move; the bounds are read back so the log states what the picture has.
		if (UStaticMeshComponent* C = A->GetStaticMeshComponent()) if (C->Mobility != EComponentMobility::Movable) C->SetMobility(EComponentMobility::Movable);
		const double Delta = FloorZCm - Top;
		A->AddActorWorldOffset(FVector(0, 0, Delta));
		A->GetActorBounds(false, Origin, Extent);
		UE_LOG(LogJamLook, Log, TEXT("JAMLOOK floor %s lowered by %.1f cm to a top of %.1f cm (bounds read back: top %.1f cm, mobility %s)"), *A->GetName(), -Delta, FloorZCm,
			Origin.Z + Extent.Z, A->GetStaticMeshComponent() && A->GetStaticMeshComponent()->Mobility == EComponentMobility::Movable ? TEXT("Movable") : TEXT("not Movable"));
		++N;
	}
	bFloorLowered = true;
	if (N == 0) UE_LOG(LogJamLook, Log, TEXT("JAMLOOK no under-ice floor found in the persistent level (nothing to lower)"));
}
