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
#include "EngineUtils.h"

DEFINE_LOG_CATEGORY(LogJamLook);

namespace
{
	struct FTierRig { const TCHAR* Tier; const TCHAR* Map; };
	const FTierRig GTierRigs[] = {
		{ TEXT("l1_day"),   TEXT("/JamAssets/Rigs/LR_L1_Day.LR_L1_Day") },
		{ TEXT("l2_night"), TEXT("/JamAssets/Rigs/LR_L2_Night.LR_L2_Night") },
		{ TEXT("l3_neon"),  TEXT("/JamAssets/Rigs/LR_L3_Neon.LR_L3_Neon") },
	};
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

	// Unload every other rig, and this tier's rig too when the scale changed (a level transform is fixed at load: R-0016 point 5).
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
	UE_LOG(LogJamLook, Log, TEXT("JAMLOOK tier=%s map=%s scale=%.3f (rink %.1f m)"), *Tier, *MapPath, Scale, RinkLengthM);
	return true;
}

void ULookTierSwitcher::OnRigLoaded()
{
	if (TObjectPtr<ULevelStreamingDynamic>* Rig = LoadedRigs.Find(CurrentTier))
	{
		if (*Rig && (*Rig)->GetLoadedLevel()) ApplyRigScale(*Rig, CurrentScale);
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
		const double Delta = FloorZCm - Top;
		A->AddActorWorldOffset(FVector(0, 0, Delta));
		UE_LOG(LogJamLook, Log, TEXT("JAMLOOK floor %s lowered by %.1f cm to a top of %.1f cm"), *A->GetName(), -Delta, FloorZCm);
		++N;
	}
	bFloorLowered = true;
	if (N == 0) UE_LOG(LogJamLook, Log, TEXT("JAMLOOK no under-ice floor found in the persistent level (nothing to lower)"));
}
