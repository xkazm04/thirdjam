// Loads one light rig by cartridge game.look.tier and unloads the others. Contract: contracts/look-tiers.md.
// The rigs themselves (sub-level maps in Game/Plugins/JamAssets/Content/Rigs/) are device B's; this switch is device A's.
// R-0016 points 3 and 5 (lane A15): a rig is authored for the 90 x 45 m rink and streamed in at uniform scale s = rink.length_m / 90;
// once loaded, every ULocalLightComponent's Intensity is multiplied by s*s and its AttenuationRadius by s. While a rig is loaded the
// persistent level's sky light and directional light are hidden so the rig alone lights the game, and A's under-ice floor is lowered
// to z = -20 cm (B's room floor sits at -5 cm). A rink change (cartridge load, Director's Call, ShrinkRink at a faceoff) reloads the rig at the new scale.
// B-to-A-0018 (lane A21): tier p3_clay (/JamAssets/Rigs/LR_P3_Clay). The previous rig is UNLOADED, never hidden, so the clay rig's
// BP_LookParams restores MPC_Look on EndPlay and the night tier never keeps the clay surface response. Only local lights are scaled:
// a post-process volume's depth-of-field focus distance is a setting, not a transform, and is never scaled with the rink.
// The trim strips' Emissive is per tier (night 30, clay 12, day 10 from `jam.look.TrimDay` since lane A223, neon 30), overridable by the cartridge key look.trim_emissive;
// the rink reads it from here (GetTrimEmissiveFor) and SetTier pushes it to every rink in the world. The beacon stays at B's 40.
// Lane A223: the cartridge's look.fog_density is drawn here too (SetFog: one height fog owned by this switcher; `jam.look.Fog*`; docs/experiments/LOOKS.md).
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LookTierSwitcher.generated.h"

class ULevelStreamingDynamic;
class AExponentialHeightFog;

JAMGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogJamLook, Log, All);

UCLASS()
class JAMGAME_API ULookTierSwitcher : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Lane A64: the exposure bias (EV) the follow camera adds while the tier's rig brings no light of its own (an empty placeholder map: l1_day, l3_neon
	// today); 0 under a real rig. The persistent lights carry those tiers and auto exposure alone washed them out (eyes grid h19 finding 6).
	static constexpr float NoRigExposureBiasEV = -1.f;
	float GetCameraExposureBias() const { return CameraExposureBias; }
	// The rink length every rig is authored for (R-0016 point 5).
	static constexpr double RigRinkLengthM = 90.0;
	// Where A's under-ice floor goes while a rig is loaded (R-0016 point 3).
	static constexpr double FloorZCm = -20.0;

	// Tier strings of game.look.tier, in ladder order: "l1_day", "l2_night", "l3_neon", "p3_clay", "p4_broadcast" (B-to-A-0025, lane A31).
	UFUNCTION(BlueprintPure, Category = "Jam|Look")
	static TArray<FString> GetKnownTiers();

	// B-to-A-0025 point 3: the current value of /JamAssets/Look/MPC_Look's Clay scalar in this world (1 clay, 0 cel, -1 broadcast; 0 when the collection is absent).
	// SetTier resets it to 0 before streaming a rig; the rig's BP_LookParams writes its own value on BeginPlay.
	UFUNCTION(BlueprintPure, Category = "Jam|Look")
	float GetLookClay() const;

	// Rig map for a tier ("/JamAssets/Rigs/LR_L2_Night.LR_L2_Night" for "l2_night"); empty for an unknown tier.
	UFUNCTION(BlueprintPure, Category = "Jam|Look")
	static FString GetRigMapPath(const FString& Tier);
	// Lane A197: the map SetTier streams for a tier: GetRigMapPath, except l3_neon, which takes device B's /JamAssets/Rigs/LR_N1_Neon when that package
	// exists (`jam.neon.rig` 1, default) and falls back to LR_L3_Neon.
	UFUNCTION(BlueprintPure, Category = "Jam|Look")
	static FString ResolveRigMapPath(const FString& Tier);

	// The uniform scale a rig is streamed at for a rink length: length / 90, clamped to 0.25..4.
	UFUNCTION(BlueprintPure, Category = "Jam|Look")
	static float RigScaleFor(float RinkLengthM);

	// B-to-A-0018 point 5: the trim strips' Emissive a tier ships with (schema default of look.trim_emissive per tier):
	// l1_day `jam.look.TrimDay` (10; 30 before lane A223), l2_night 30, l3_neon 30, p3_clay 12, p4_broadcast 30. Unknown tier: 30.
	UFUNCTION(BlueprintPure, Category = "Jam|Look")
	static float DefaultTrimEmissiveFor(const FString& Tier);

	// The trim Emissive the rink should use for Tier: the cartridge override when one is set, else the tier's default.
	UFUNCTION(BlueprintPure, Category = "Jam|Look")
	float GetTrimEmissiveFor(const FString& Tier) const;

	// For the current tier (l2_night's default before the first SetTier).
	UFUNCTION(BlueprintPure, Category = "Jam|Look")
	float GetTrimEmissive() const;

	// look.trim_emissive from the cartridge (0..100); a value below 0 clears the override. Pushes the value to every rink in the world.
	UFUNCTION(BlueprintCallable, Category = "Jam|Look")
	void SetTrimEmissiveOverride(float Emissive);

	UFUNCTION(BlueprintPure, Category = "Jam|Look")
	bool HasTrimEmissiveOverride() const { return bTrimOverride; }

	// Lane A223 (B-to-A-0046 point 3): the cartridge's look.fog_density drawn. The engine height-fog density a cartridge value is drawn with: 0 below
	// `jam.look.FogFrom` (0.03) or with `jam.look.FogScale` 0 (the picture of before the lane), else the value x FogScale (8).
	UFUNCTION(BlueprintPure, Category = "Jam|Look")
	static float FogDensityFor(float CartridgeDensity);
	// look.fog_density of the match's cartridge. Draws (or hides) one height fog owned by this switcher; a rig with a height fog of its own keeps it.
	UFUNCTION(BlueprintCallable, Category = "Jam|Look")
	void SetFog(float CartridgeDensity);
	// The engine density of A's height fog as drawn now (0 = none).
	UFUNCTION(BlueprintPure, Category = "Jam|Look")
	float GetFogDrawn() const { return FogDrawn; }
	// Pushes the `jam.look.*` variables to the live world again (a console change calls it).
	void RefreshLookVars();
	// The fog's height falloff (near-uniform to the hall's roof: it is fog by distance) and its colour before `jam.look.FogBright`: a pale cold blue. Judged on
	// film: a third of this (0.16, 0.21, 0.30) read as the hall going dark, not as fog; twice this washed the ice white.
	static constexpr float FogHeightFalloff = 0.05f;
	static constexpr float FogColourR = 0.48f;
	static constexpr float FogColourG = 0.63f;
	static constexpr float FogColourB = 0.90f;

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
	void PushTrimEmissive();
	void PushGlass();              // lane A31: the glass instance per tier to every rink (ARinkActor::SetGlassForTier)
	void ResetLookCollection();    // lane A31: MPC_Look.Clay = 0 before a rig streams

	UPROPERTY(Transient)
	TMap<FString, TObjectPtr<ULevelStreamingDynamic>> LoadedRigs;

	FString CurrentTier;
	float CurrentScale = 1.f;
	int32 ScaledLights = 0;
	bool bPersistentLightsHidden = false;
	float CameraExposureBias = 0.f;   // lane A64
	void PushCameraExposure();
	bool bFloorLowered = false;
	bool bTrimOverride = false;
	float TrimOverride = 30.f;
	// Lane A223: the fog asked by the cartridge, the density drawn, the one fog actor.
	void PushFog();
	float FogAsked = 0.f;
	float FogDrawn = 0.f;
	UPROPERTY(Transient)
	TObjectPtr<AExponentialHeightFog> FogActor;
};
