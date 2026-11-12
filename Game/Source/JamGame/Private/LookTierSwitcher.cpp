#include "LookTierSwitcher.h"

#include "Engine/LevelStreamingDynamic.h"
#include "Engine/World.h"

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

bool ULookTierSwitcher::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

bool ULookTierSwitcher::SetTier(const FString& Tier)
{
	const FString MapPath = GetRigMapPath(Tier);
	if (MapPath.IsEmpty())
	{
		UE_LOG(LogJamLook, Error, TEXT("JAMLOOK_UNKNOWN tier '%s' is not in contracts/look-tiers.md; nothing changed"), *Tier);
		return false;
	}
	if (Tier == CurrentTier && LoadedRigs.Contains(Tier))
	{
		return true;
	}

	for (auto It = LoadedRigs.CreateIterator(); It; ++It)
	{
		if (It.Key() == Tier) { continue; }
		if (ULevelStreamingDynamic* Old = It.Value())
		{
			Old->SetShouldBeVisible(false);
			Old->SetShouldBeLoaded(false);
			Old->SetIsRequestingUnloadAndRemoval(true);
		}
		It.RemoveCurrent();
	}

	if (!LoadedRigs.Contains(Tier))
	{
		bool bOk = false;
		ULevelStreamingDynamic* Rig = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
			GetWorld(), TSoftObjectPtr<UWorld>(FSoftObjectPath(MapPath)), FVector::ZeroVector, FRotator::ZeroRotator, bOk);
		if (!bOk || !Rig)
		{
			UE_LOG(LogJamLook, Error, TEXT("JAMLOOK_FAIL tier=%s map=%s could not be streamed in"), *Tier, *MapPath);
			CurrentTier.Reset();
			return false;
		}
		LoadedRigs.Add(Tier, Rig);
	}
	CurrentTier = Tier;
	UE_LOG(LogJamLook, Log, TEXT("JAMLOOK tier=%s map=%s"), *Tier, *MapPath);
	return true;
}
