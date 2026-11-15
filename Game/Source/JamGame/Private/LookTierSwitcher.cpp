#include "LookTierSwitcher.h"

#include "Engine/LevelStreamingDynamic.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "Engine/SkyLight.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/LocalLightComponent.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "EngineUtils.h"
#include "RinkActor.h"
#include "CameraFollow.h"
#include "UObject/UObjectIterator.h"
#include "Misc/PackageName.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY(LogJamLook);

// Lane A223 (track L1; B-to-A-0046): three looks the trailer shows. Every number is a variable whose neutral value gives back the picture of before.
// Point 1: the board trim in daylight. Emissive 30 is a light strip against white ice under B's day rig (a halo, the colour washed to pale yellow and pale
// cyan); at 10 it is the trim's own colour, painted on the boards.
static TAutoConsoleVariable<float> CVarLookTrimDay(TEXT("jam.look.TrimDay"), 10.f,
	TEXT("Lane A223: Emissive of the board trim on the l1_day tier (Pond, Outnumbered). 10 (default) reads as painted trim in daylight; 30 = as before the lane (a glowing line). A cartridge's look.trim_emissive still wins."), ECVF_Default);
// Point 3: the cartridge's look.fog_density was parsed and never drawn. A cut that asks at least FogFrom gets a height fog of density look.fog_density x FogScale
// (Fog of War asks 0.04: 0.32, about half of a skater 30 m away kept and a tenth of the far boards); a cut that asks less (hockey.boost and the ladder's neon
// takes ask 0.02) keeps its picture of before. A rig that brings a height fog of its own (LR_L3_Neon) keeps it: A draws none there.
static TAutoConsoleVariable<float> CVarLookFogScale(TEXT("jam.look.FogScale"), 8.f,
	TEXT("Lane A223: engine height-fog density drawn per unit of the cartridge's look.fog_density (Fog of War: 0.04 x 8 = 0.32). 0 = no fog is drawn, as before the lane."), ECVF_Default);
static TAutoConsoleVariable<float> CVarLookFogFrom(TEXT("jam.look.FogFrom"), 0.03f,
	TEXT("Lane A223: the smallest look.fog_density that draws fog; a cut that asks less keeps the picture it had (0.02: hockey.boost, the ladder's neon takes)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarLookFogStartM(TEXT("jam.look.FogStartM"), 10.f,
	TEXT("Lane A223: metres from the camera where the fog begins: his own skater, the puck on his stick and the nearest opponents stay clear of it."), ECVF_Default);
static TAutoConsoleVariable<float> CVarLookFogMax(TEXT("jam.look.FogMax"), 0.9f,
	TEXT("Lane A223: the most the fog may cover (0..1): the far boards and the far goal stay a shape in it, never gone."), ECVF_Default);
static TAutoConsoleVariable<float> CVarLookFogBright(TEXT("jam.look.FogBright"), 1.f,
	TEXT("Lane A223: multiplies the fog's own colour (a pale cold blue, lighter than the dark hall so that it reads as fog and not as darkness; 0.33 was the first try and read as darkness, 2 washes the ice white)."), ECVF_Default);

// A variable changed at the console is pushed to the live world at once (the looks were judged by looking, one value after another, in one process).
static void JamLookVarsChanged()
{
	static float Seen[6] = { -1.f, -1.f, -1.f, -1.f, -1.f, -1.f };
	const float Now[6] = { CVarLookTrimDay.GetValueOnGameThread(), CVarLookFogScale.GetValueOnGameThread(), CVarLookFogFrom.GetValueOnGameThread(),
		CVarLookFogStartM.GetValueOnGameThread(), CVarLookFogMax.GetValueOnGameThread(), CVarLookFogBright.GetValueOnGameThread() };
	bool bChanged = false;
	for (int32 i = 0; i < 6; ++i) { bChanged = bChanged || Now[i] != Seen[i]; Seen[i] = Now[i]; }
	if (!bChanged) return;
	for (TObjectIterator<ULookTierSwitcher> It; It; ++It)
	{
		if (*It && !It->HasAnyFlags(RF_ClassDefaultObject) && It->GetWorld()) It->RefreshLookVars();
	}
}
static FAutoConsoleVariableSink GJamLookVarsSink(FConsoleCommandDelegate::CreateStatic(&JamLookVarsChanged));

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
	// Lane A223 (B-to-A-0046 point 1): l1_day is read from `jam.look.TrimDay` (10; this table keeps the 30 of before as the variable's neutral value).
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
	if (Tier.Equals(TEXT("l1_day"), ESearchCase::CaseSensitive)) { return FMath::Clamp(CVarLookTrimDay.GetValueOnAnyThread(), 0.f, 100.f); }   // lane A223
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

float ULookTierSwitcher::FogDensityFor(float CartridgeDensity)
{
	const float Scale = CVarLookFogScale.GetValueOnAnyThread();
	if (!(Scale > 0.f) || !(CartridgeDensity > 0.f) || CartridgeDensity < CVarLookFogFrom.GetValueOnAnyThread()) return 0.f;
	return FMath::Min(CartridgeDensity * Scale, 10.f);   // the engine clamps a height fog's density to 10
}

void ULookTierSwitcher::SetFog(float CartridgeDensity)
{
	FogAsked = CartridgeDensity;
	PushFog();
}

void ULookTierSwitcher::RefreshLookVars()
{
	PushTrimEmissive();
	PushFog();
}

void ULookTierSwitcher::PushFog()
{
	UWorld* W = GetWorld();
	if (!W) return;
	float Density = FogDensityFor(FogAsked);
	// One height fog a scene: a rig that brings its own (LR_L3_Neon's Fog_L3) keeps it.
	bool bRigFog = false;
	if (Density > 0.f)
	{
		if (const TObjectPtr<ULevelStreamingDynamic>* Rig = LoadedRigs.Find(CurrentTier))
		{
			if (const ULevel* Level = *Rig ? (*Rig)->GetLoadedLevel() : nullptr)
			{
				for (const AActor* A : Level->Actors) { if (A && A->FindComponentByClass<UExponentialHeightFogComponent>()) { bRigFog = true; break; } }
			}
		}
		if (bRigFog) Density = 0.f;
	}
	if (!(Density > 0.f))
	{
		if (FogActor && FogActor->GetComponent()) FogActor->GetComponent()->SetVisibility(false);
		if (FogDrawn > 0.f || bRigFog) UE_LOG(LogJamLook, Log, TEXT("JAMLOOK fog off (look.fog_density %.3f%s)"), FogAsked, bRigFog ? TEXT("; the rig brings its own height fog") : TEXT(""));
		FogDrawn = 0.f;
		return;
	}
	if (!FogActor)
	{
		FActorSpawnParameters P;
		P.ObjectFlags |= RF_Transient;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		FogActor = W->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass(), FTransform::Identity, P);
	}
	UExponentialHeightFogComponent* C = FogActor ? FogActor->GetComponent() : nullptr;
	if (!C) return;
	const float StartM = FMath::Max(0.f, CVarLookFogStartM.GetValueOnGameThread());
	const float MaxOpacity = FMath::Clamp(CVarLookFogMax.GetValueOnGameThread(), 0.f, 1.f);
	const float Bright = FMath::Max(0.f, CVarLookFogBright.GetValueOnGameThread());
	C->SetFogDensity(Density);
	C->SetFogHeightFalloff(FogHeightFalloff);
	C->SetStartDistance(StartM * 100.f);
	C->SetFogMaxOpacity(MaxOpacity);
	C->SetFogInscatteringColor(FLinearColor(FogColourR * Bright, FogColourG * Bright, FogColourB * Bright));
	C->SetDirectionalInscatteringColor(FLinearColor::Black);   // no sun glow in a hall
	C->SetVolumetricFog(false);
	C->SetVisibility(true);
	if (!FMath::IsNearlyEqual(FogDrawn, Density, 1e-4f)) UE_LOG(LogJamLook, Log, TEXT("JAMLOOK fog on: look.fog_density %.3f drawn as height fog density %.3f from %.0f m, at most %.2f, brightness %.2f"), FogAsked, Density, StartM, MaxOpacity, Bright);
	FogDrawn = Density;
}

float ULookTierSwitcher::RigScaleFor(float RinkLengthM)
{
	if (!(RinkLengthM > 0.f)) return 1.f;
	return FMath::Clamp(RinkLengthM / static_cast<float>(RigRinkLengthM), 0.25f, 4.f);
}

// Lane A230 (wave 2; device B's status H61.0): the neon rig's edge line is one closed loop just inside the boards of a 90 x 45 m rink. Scaled by the
// length alone it meets every rink that is 2 : 1; on the Cage (60 x 28) it sat 0.47 m from the long boards and 1.47 m from the ends. Across the rink
// the neon rig is now scaled by width / 45 when the rink is not 2 : 1. Only l3_neon: the other tiers' rigs (stands, crowds) keep the uniform scale.
static TAutoConsoleVariable<int32> CVarLookRigPerAxis(TEXT("jam.look.RigPerAxis"), 1,
	TEXT("Lane A230: 1 (default) = on l3_neon a rink that is not 2 : 1 streams the rig scaled length / 90 along the rink and width / 45 across it; 0 = the uniform length / 90 of before the lane. A 2 : 1 rink is the same either way."), ECVF_Default);

float ULookTierSwitcher::RigCrossScaleFor(const FString& Tier, float RinkLengthM, float RinkWidthM)
{
	const float Along = RigScaleFor(RinkLengthM);
	if (CVarLookRigPerAxis.GetValueOnAnyThread() == 0 || Tier != TEXT("l3_neon") || !(RinkLengthM > 0.f) || !(RinkWidthM > 0.f)) return Along;
	if (FMath::Abs(RinkLengthM - 2.f * RinkWidthM) < 0.05f) return Along;   // a 2 : 1 rink: the uniform scale of before, to the bit
	return FMath::Clamp(RinkWidthM / static_cast<float>(RigRinkLengthM * 0.5), 0.25f, 4.f);
}

bool ULookTierSwitcher::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

// Lane A197 (R-0064 point 8): device B's neon rig. The grid arena's tier (l3_neon) streams /JamAssets/Rigs/LR_N1_Neon when that package exists and
// `jam.neon.rig` is 1, and today's LR_L3_Neon placeholder otherwise: the mode selects B's rig by name with a fallback, and no schema tier changes.
static TAutoConsoleVariable<int32> CVarJamNeonRig(TEXT("jam.neon.rig"), 1,
	TEXT("Lane A197: 1 = the l3_neon tier streams device B's /JamAssets/Rigs/LR_N1_Neon when it exists (default), 0 = always the LR_L3_Neon placeholder."), ECVF_Default);

FString ULookTierSwitcher::ResolveRigMapPath(const FString& Tier)
{
	const FString MapPath = GetRigMapPath(Tier);
	if (Tier == TEXT("l3_neon") && CVarJamNeonRig.GetValueOnGameThread() != 0 && FPackageName::DoesPackageExist(TEXT("/JamAssets/Rigs/LR_N1_Neon")))
	{
		return TEXT("/JamAssets/Rigs/LR_N1_Neon.LR_N1_Neon");
	}
	return MapPath;
}

bool ULookTierSwitcher::SetTier(const FString& Tier, float RinkLengthM, float RinkWidthM)
{
	const FString MapPath = ResolveRigMapPath(Tier);   // lane A197: B's neon rig by name, today's tier as the fallback
	if (MapPath.IsEmpty())
	{
		UE_LOG(LogJamLook, Error, TEXT("JAMLOOK_UNKNOWN tier '%s' is not in contracts/look-tiers.md; nothing changed"), *Tier);
		return false;
	}
	const float Scale = RigScaleFor(RinkLengthM);
	const float Cross = RigCrossScaleFor(Tier, RinkLengthM, RinkWidthM);   // lane A230: the scale across the rink (the same as Scale on a 2 : 1 rink)
	if (Tier == CurrentTier && LoadedRigs.Contains(Tier) && FMath::IsNearlyEqual(Scale, CurrentScale, 1e-3f) && FMath::IsNearlyEqual(Cross, CurrentCrossScale, 1e-3f))
	{
		return true;
	}

	// Unload every other rig (B-to-A-0018 point 4: unloaded, not hidden, so B's BP_LookParams EndPlay restores MPC_Look), and this
	// tier's rig too when the scale changed (a level transform is fixed at load: R-0016 point 5).
	for (auto It = LoadedRigs.CreateIterator(); It; ++It)
	{
		if (It.Key() == Tier && FMath::IsNearlyEqual(Scale, CurrentScale, 1e-3f) && FMath::IsNearlyEqual(Cross, CurrentCrossScale, 1e-3f)) { continue; }
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
		FTransform Xf(FRotator::ZeroRotator, FVector::ZeroVector, FVector(Scale, Cross, Scale));   // lane A230: x along the rink, y across it
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
	CurrentCrossScale = Cross;
	if (!FMath::IsNearlyEqual(Scale, Cross, 1e-4f)) UE_LOG(LogJamLook, Log, TEXT("JAMLOOK rig scaled per axis: %.3f along, %.3f across (rink %.1f x %.1f m; jam.look.RigPerAxis)"), Scale, Cross, RinkLengthM, RinkWidthM);
	HidePersistentLights(true);
	LowerFloor();
	PushTrimEmissive();
	PushGlass();
	PushFog();   // lane A223
	UE_LOG(LogJamLook, Log, TEXT("JAMLOOK tier=%s map=%s scale=%.3f (rink %.1f m) trim_emissive=%.1f"), *Tier, *MapPath, Scale, RinkLengthM, GetTrimEmissive());
	return true;
}

void ULookTierSwitcher::OnRigLoaded()
{
	if (TObjectPtr<ULevelStreamingDynamic>* Rig = LoadedRigs.Find(CurrentTier))
	{
		if (*Rig && (*Rig)->GetLoadedLevel())
		{
			ApplyRigScale(*Rig, CurrentScale);
			// Lane A64 (eyes grid h19 finding 6, "l1 day blown out"): LR_L1_Day and LR_L3_Neon are still lane A0's empty placeholder maps (8 KB, a brush and
			// world settings, no light, no post volume). Hiding the persistent sky light for them left the rink with no light rig at all and auto exposure
			// pushed the picture white. A rig that brings no light of its own keeps the persistent lights.
			int32 RigLights = 0;
			for (AActor* A : (*Rig)->GetLoadedLevel()->Actors)
			{
				if (!A) continue;
				TInlineComponentArray<ULightComponentBase*> Comps;
				A->GetComponents(Comps);
				RigLights += Comps.Num();
			}
			CameraExposureBias = RigLights == 0 ? NoRigExposureBiasEV : 0.f;
			PushCameraExposure();
			if (RigLights == 0)
			{
				HidePersistentLights(false);
				UE_LOG(LogJamLook, Warning, TEXT("JAMLOOK rig %s carries no light (B's map is an empty placeholder): the persistent sky and directional lights stay on"), *CurrentTier);
			}
		}
	}
	PushFog();   // lane A223: a rig with a height fog of its own keeps it
	UE_LOG(LogJamLook, Log, TEXT("JAMLOOK rig %s loaded: MPC_Look.Clay=%.1f (BP_LookParams writes its value on BeginPlay, read again by GetLookClay)"), *CurrentTier, GetLookClay());
}

void ULookTierSwitcher::PushCameraExposure()
{
	UWorld* W = GetWorld();
	if (!W) return;
	int32 N = 0;
	for (TObjectIterator<UJamFollowCamera> It; It; ++It)
	{
		if (*It && It->GetWorld() == W) { It->SetExposureBias(CameraExposureBias); ++N; }
	}
	UE_LOG(LogJamLook, Log, TEXT("JAMLOOK camera exposure bias %.1f EV on %d follow cameras (lane A64)"), CameraExposureBias, N);
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
		if (*It) { It->SetGlassForTier(CurrentTier); It->SetZoneRingsForTier(CurrentTier); }   // lane A69: the clay rings follow the tier
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
		// Lane A69 (B-to-A-0034 notes): the floor sits under the ice and under B's room floor, so it is never seen, yet it cast shadow into every shadowed light
		// (+0.34 ms GPU on p3_clay in PIE, four shadowed spots). Nothing lies under it to be shadowed; its shadow goes.
		if (UStaticMeshComponent* C = A->GetStaticMeshComponent()) C->SetCastShadow(false);
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
