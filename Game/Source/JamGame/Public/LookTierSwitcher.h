// Loads one light rig by cartridge game.look.tier and unloads the others. Contract: contracts/look-tiers.md.
// The rigs themselves (sub-level maps in Game/Plugins/JamAssets/Content/Rigs/) are device B's; this switch is device A's.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LookTierSwitcher.generated.h"

class ULevelStreamingDynamic;

JAMGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogJamLook, Log, All);

UCLASS()
class JAMGAME_API ULookTierSwitcher : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Tier strings of game.look.tier, in ladder order: "l1_day", "l2_night", "l3_neon".
	UFUNCTION(BlueprintPure, Category = "Jam|Look")
	static TArray<FString> GetKnownTiers();

	// Rig map for a tier ("/JamAssets/Rigs/LR_L2_Night.LR_L2_Night" for "l2_night"); empty for an unknown tier.
	UFUNCTION(BlueprintPure, Category = "Jam|Look")
	static FString GetRigMapPath(const FString& Tier);

	// Streams in the tier's rig and requests unload of every other rig this switcher loaded. Unknown tier: logs an error,
	// changes nothing, returns false. Same tier as the current one: no-op, returns true. Logs "JAMLOOK tier=<tier> map=<path>".
	UFUNCTION(BlueprintCallable, Category = "Jam|Look")
	bool SetTier(const FString& Tier);

	// The tier last set successfully; empty before the first SetTier.
	UFUNCTION(BlueprintPure, Category = "Jam|Look")
	FString GetCurrentTier() const { return CurrentTier; }

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	UPROPERTY(Transient)
	TMap<FString, TObjectPtr<ULevelStreamingDynamic>> LoadedRigs;

	FString CurrentTier;
};
