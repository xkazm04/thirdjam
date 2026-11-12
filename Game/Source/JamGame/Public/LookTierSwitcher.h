// Loads one light rig by cartridge game.look.tier and unloads the others. Contract: contracts/look-tiers.md.
// The rigs themselves (sub-level maps in Game/Plugins/JamAssets/Content/Rigs/) are device B's; this switch is device A's.
// R-0016 points 3 and 5 (lane A15): a rig is authored for the 90 x 45 m rink and streamed in at uniform scale s = rink.length_m / 90;
// once loaded, every ULocalLightComponent's Intensity is multiplied by s*s and its AttenuationRadius by s. While a rig is loaded the
// persistent level's sky light and directional light are hidden so the rig alone lights the game, and A's under-ice floor is lowered
// to z = -20 cm (B's room floor sits at -5 cm). A rink change (cartridge load, Director's Call, ShrinkRink at a faceoff) reloads the rig at the new scale.
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
	// The rink length every rig is authored for (R-0016 point 5).
	static constexpr double RigRinkLengthM = 90.0;
	// Where A's under-ice floor goes while a rig is loaded (R-0016 point 3).
	static constexpr double FloorZCm = -20.0;

	// Tier strings of game.look.tier, in ladder order: "l1_day", "l2_night", "l3_neon".
	UFUNCTION(BlueprintPure, Category = "Jam|Look")
	static TArray<FString> GetKnownTiers();

	// Rig map for a tier ("/JamAssets/Rigs/LR_L2_Night.LR_L2_Night" for "l2_night"); empty for an unknown tier.
	UFUNCTION(BlueprintPure, Category = "Jam|Look")
	static FString GetRigMapPath(const FString& Tier);

	// The uniform scale a rig is streamed at for a rink length: length / 90, clamped to 0.25..4.
	UFUNCTION(BlueprintPure, Category = "Jam|Look")
	static float RigScaleFor(float RinkLengthM);

	// Streams in the tier's rig at the scale for RinkLengthM and requests unload of every other rig this switcher loaded. Unknown tier:
	// logs an error, changes nothing, returns false. Same tier and same scale as the current one: no-op, returns true. A different scale
	// reloads the rig. Logs "JAMLOOK tier=<tier> map=<path> scale=<s>".
	UFUNCTION(BlueprintCallable, Category = "Jam|Look")
	bool SetTier(const FString& Tier, float RinkLengthM = 90.f);

	// The tier last set successfully; empty before the first SetTier.
	UFUNCTION(BlueprintPure, Category = "Jam|Look")
	FString GetCurrentTier() const { return CurrentTier; }

	UFUNCTION(BlueprintPure, Category = "Jam|Look")
	float GetCurrentScale() const { return CurrentScale; }

	// How many local lights the last loaded rig had scaled (tests and the PIE proof read it).
	UFUNCTION(BlueprintPure, Category = "Jam|Look")
	int32 GetScaledLightCount() const { return ScaledLights; }

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	UFUNCTION() void OnRigLoaded();
	void ApplyRigScale(ULevelStreamingDynamic* Rig, float Scale);
	void HidePersistentLights(bool bHide);
	void LowerFloor();

	UPROPERTY(Transient)
	TMap<FString, TObjectPtr<ULevelStreamingDynamic>> LoadedRigs;

	FString CurrentTier;
	float CurrentScale = 1.f;
	int32 ScaledLights = 0;
	bool bPersistentLightsHidden = false;
	bool bFloorLowered = false;
};
