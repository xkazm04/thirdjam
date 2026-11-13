#include "EffectsBinder.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "HUDFeed.h"
#include "HokejGameMode.h"
#include "JamSim.h"
#include "Misc/ScopeLock.h"
#include "UObject/UObjectGlobals.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/MeshComponent.h"
#include "EngineUtils.h"
#include "RinkActor.h"
#include "Engine/AssetManager.h"
#include "Misc/PackageName.h"
#include "Misc/CoreDelegates.h"
#include "Misc/CommandLine.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformTime.h"
#include "JamCartridge.h"
#include "JamCommentaryData.h"
#include "AudioDevice.h"
#include "LookTierSwitcher.h"

DEFINE_LOG_CATEGORY(LogJamFx);

UJamEffectsBinder::UJamEffectsBinder()
{
	// Lane A45: the glass wobble runs down over sim frames in TickComponent; the tick is enabled only while a wobble is live.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

float UJamEffectsBinder::GlassWobbleAt(int64 FramesSinceHit, float Strength)
{
	if (FramesSinceHit < 0 || FramesSinceHit >= GlassWobbleFrames) return 0.f;
	return FMath::Clamp(Strength, 0.f, 1.f) * (1.f - static_cast<float>(FramesSinceHit) / static_cast<float>(GlassWobbleFrames));
}

UMaterialInstanceDynamic* UJamEffectsBinder::GlassMid()
{
	if (!bGlassLooked)
	{
		bGlassLooked = true;
		if (UWorld* World = GetWorld())
		{
			for (TActorIterator<ARinkActor> It(World); It; ++It)
			{
				TArray<UMeshComponent*> Meshes;
				It->GetComponents<UMeshComponent>(Meshes);
				for (UMeshComponent* M : Meshes) if (M && M->GetFName() == FName(TEXT("Glass"))) { GlassMesh = M; break; }
				if (GlassMesh.IsValid()) break;
			}
		}
	}
	UMeshComponent* Mesh = GlassMesh.Get();
	if (!Mesh) return nullptr;
	UMaterialInterface* Current = Mesh->GetMaterial(0);
	if (GlassInstance && Current == GlassInstance) return GlassInstance;
	// A new material on the slot (B's instance arrived, or the look tier swapped it): only a material that carries HitWobble gets a dynamic instance.
	GlassInstance = nullptr;
	if (!Current) return nullptr;
	float Probe = 0.f;
	if (!Current->GetScalarParameterValue(FHashedMaterialParameterInfo(TEXT("HitWobble")), Probe))
	{
		return nullptr;
	}
	GlassInstance = UMaterialInstanceDynamic::Create(Current, this);
	Mesh->SetMaterial(0, GlassInstance);
	UE_LOG(LogJamFx, Log, TEXT("JAMFX glass %s carries HitWobble: dynamic instance bound for the board hits"), *Current->GetName());
	return GlassInstance;
}

void UJamEffectsBinder::StartGlassWobble(const FJamEventPayload& Payload)
{
	UMaterialInstanceDynamic* Mid = GlassMid();
	if (!Mid || !Bus) return;
	GlassHitFrame = Bus->GetSimFrame();
	GlassHitStrength = FMath::Clamp(Payload.Strength, 0.f, 1.f);
	Mid->SetScalarParameterValue(TEXT("HitWobble"), GlassWobbleAt(0, GlassHitStrength));
	FLinearColor ProbeV;
	if (Mid->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("HitPos")), ProbeV)) Mid->SetVectorParameterValue(TEXT("HitPos"), FLinearColor(Payload.Where.X, Payload.Where.Y, Payload.Where.Z, 1.f));
	++GlassWobbles;
	SetComponentTickEnabled(true);
}

void UJamEffectsBinder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (GlassHitFrame < 0 || !Bus || !GlassInstance) { SetComponentTickEnabled(false); return; }
	const int64 Since = Bus->GetSimFrame() - GlassHitFrame;
	GlassInstance->SetScalarParameterValue(TEXT("HitWobble"), GlassWobbleAt(Since, GlassHitStrength));
	if (Since >= GlassWobbleFrames) { GlassHitFrame = -1; SetComponentTickEnabled(false); }
}

namespace
{
	FCriticalSection MissingLock;
	TSet<FString>& MissingSet()
	{
		static TSet<FString> S;
		return S;
	}

	// Lane A65: packages the menu's preload looked up and found absent (filled on the game thread, read by the loads below).
	FCriticalSection KnownMissingLock;
	TSet<FString>& KnownMissingSet()
	{
		static TSet<FString> S;
		return S;
	}

	// Quiet load with no log line at all (used where a miss is expected and answered by an alias). An object the menu's preload brought in is found in
	// memory by StaticLoadObject without a synchronous load; a package the preload found absent is not looked up again (that lookup is synchronous too).
	UObject* LoadSilently(const FSoftObjectPath& Path)
	{
		if (UJamPreloadSubsystem::IsKnownMissing(Path)) return nullptr;
		return StaticLoadObject(UObject::StaticClass(), nullptr, *Path.ToString(), nullptr, LOAD_NoWarn | LOAD_Quiet);
	}
}

FSoftObjectPath UJamEffectsBinder::NiagaraPathFor(FName Event)
{
	return FSoftObjectPath(FString::Printf(TEXT("/JamAssets/VFX/NS_%s.NS_%s"), *Event.ToString(), *Event.ToString()));
}

FSoftObjectPath UJamEffectsBinder::SoundPathFor(FName Event)
{
	return FSoftObjectPath(FString::Printf(TEXT("/JamAssets/Audio/MS_%s.MS_%s"), *Event.ToString(), *Event.ToString()));
}

UObject* UJamEffectsBinder::LoadOrLogOnce(const FSoftObjectPath& Path)
{
	// Quiet load: a missing asset is expected (an event may have no effect), so no engine warning.
	UObject* Obj = LoadSilently(Path);
	if (!Obj)
	{
		FScopeLock Lock(&MissingLock);
		bool bAlready = false;
		MissingSet().Add(Path.ToString(), &bAlready);
		if (!bAlready)
		{
			UE_LOG(LogJamFx, Log, TEXT("JAMFX missing %s (logged once, skipped)"), *Path.ToString());
		}
	}
	return Obj;
}

int32 UJamEffectsBinder::GetMissingLoggedCount()
{
	FScopeLock Lock(&MissingLock);
	return MissingSet().Num();
}

void UJamEffectsBinder::ResetMissingLogForTests()
{
	FScopeLock Lock(&MissingLock);
	MissingSet().Reset();
}

float UJamEffectsBinder::VariantFor(FName Event, FName Kind)
{
	// B-to-A-0012 table. The order of the signal intents is EJamTeamIntent (SignalIntent.h): Defend, Space, Pass, Praise.
	if (Event == JamEvents::Poke || Event == JamEvents::Lift)
	{
		if (Kind == FName(TEXT("miss"))) return 1.f;
		return 0.f;   // clean
	}
	if (Event == JamEvents::Faceoff)
	{
		if (Kind == FName(TEXT("won"))) return 1.f;
		if (Kind == FName(TEXT("lost"))) return 2.f;
		return 0.f;   // drop
	}
	if (Event == JamEvents::Foul)
	{
		if (Kind == FName(TEXT("foul_no_puck"))) return 1.f;
		return 0.f;   // foul_from_behind (2, the long whistle with a buzzer, waits for a harder foul in a ruling)
	}
	if (Event == JamEvents::Signal)
	{
		if (Kind == FName(TEXT("space"))) return 1.f;
		if (Kind == FName(TEXT("pass"))) return 2.f;
		if (Kind == FName(TEXT("praise"))) return 3.f;
		return 0.f;   // defend
	}
	// Lane A33 (docs/experiments/TACKLING.md): the check's direction seen from the victim; the puck's board hit is 0, a skater's 1.
	if (Event == JamEvents::Check)
	{
		if (Kind == FName(TEXT("side_l"))) return 1.f;
		if (Kind == FName(TEXT("side_r"))) return 2.f;
		if (Kind == FName(TEXT("behind"))) return 3.f;
		return 0.f;   // front (and a check with no Kind)
	}
	if (Event == JamEvents::Board)
	{
		return Kind == FName(TEXT("skater")) ? 1.f : 0.f;
	}
	// Lane A47 (B-to-A-0031 ask 3): MS_clock plays ten ticks at Variant 0 and nothing at Variant 1, so sudden death has no countdown.
	if (Event == JamEvents::Clock)
	{
		return Kind == FName(TEXT("sudden_death")) ? 1.f : 0.f;
	}
	return 0.f;   // shot_blocked reuses the foul assets at Variant 0 (B-to-A-0016); everything else has one kind
}

bool UJamEffectsBinder::IsSound2D(FName Event)
{
	// Lane A47 (B-to-A-0031 ask 2): the horn, the ticks and the organ sting have a zero position; 2D so they do not pan with the camera from centre ice.
	return Event == JamEvents::Bark || Event == JamEvents::Signal || Event == JamEvents::Announce
		|| Event == JamEvents::PeriodEnd || Event == JamEvents::Clock || Event == JamEvents::SeasonTake;
}

bool UJamEffectsBinder::IsSilentEvent(FName Event)
{
	// carry fires constantly and has nothing to show or play (B-to-A-0031 ask 1, lane A47).
	return Event == JamEvents::Period || Event == JamEvents::Carry;
}

bool UJamEffectsBinder::IsSoundOnlyEvent(FName Event)
{
	// Director, H9.4: B makes no NS_ for these four on purpose (MS_lose, MS_bark, MS_signal, MS_announce only).
	// B-to-A-0031 ask 2 (lane A47): period_end, clock and season_take are sound-only too (MS_period_end, MS_clock, MS_season_take).
	return Event == JamEvents::Lose || Event == JamEvents::Bark || Event == JamEvents::Signal || Event == JamEvents::Announce
		|| Event == JamEvents::PeriodEnd || Event == JamEvents::Clock || Event == JamEvents::SeasonTake;
}

FLinearColor UJamEffectsBinder::ZoneKindColour(FName Kind)
{
	// B-to-A-0012 point 2: the same colours are on B's zone strip materials (MI_Zone_<kind>).
	if (Kind == FName(TEXT("slow_ice"))) return FLinearColor(0.35f, 0.65f, 1.0f);
	if (Kind == FName(TEXT("fast_ice"))) return FLinearColor(0.2f, 1.0f, 0.8f);
	if (Kind == FName(TEXT("bumper"))) return FLinearColor(1.0f, 0.35f, 0.1f);
	if (Kind == FName(TEXT("no_shoot"))) return FLinearColor(1.0f, 0.1f, 0.2f);
	if (Kind == FName(TEXT("boost_only"))) return FLinearColor(0.9f, 0.8f, 0.2f);
	return FLinearColor::White;
}

FLinearColor UJamEffectsBinder::PickupKindColour(FName Kind)
{
	// B-to-A-0016: the instances' PickupColour, as written in the handoff; custom and unknown are white.
	if (Kind == FName(TEXT("speed"))) return FLinearColor(0.1f, 0.9f, 1.0f);
	if (Kind == FName(TEXT("precision"))) return FLinearColor(1.0f, 0.2f, 0.9f);
	if (Kind == FName(TEXT("heavy_puck"))) return FLinearColor(0.55f, 0.3f, 1.0f);
	if (Kind == FName(TEXT("big_puck"))) return FLinearColor(1.0f, 0.75f, 0.1f);
	if (Kind == FName(TEXT("reverse_other"))) return FLinearColor(1.0f, 0.15f, 0.1f);
	return FLinearColor::White;
}

void UJamEffectsBinder::BeginPlay()
{
	Super::BeginPlay();
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	Bus = GI ? GI->GetSubsystem<UJamEventBus>() : nullptr;
	if (!Bus)
	{
		UE_LOG(LogJamFx, Warning, TEXT("JAMFX no UJamEventBus on the game instance; effects binder idle"));
		return;
	}
	// Resolve every known event's assets now, so a first goal does not hitch on a load. Silent events are not even looked up.
	for (const FName& E : UJamEventBus::GetKnownEvents())
	{
		if (IsSilentEvent(E)) continue;
		if (E == JamEvents::ShotBlocked)
		{
			// B-to-A-0016: no burst of its own yet; a quiet miss falls back to the foul assets at Variant 0 (one log line, not a "missing" one).
			UNiagaraSystem* N = Cast<UNiagaraSystem>(LoadSilently(NiagaraPathFor(E)));
			USoundBase* S = Cast<USoundBase>(LoadSilently(SoundPathFor(E)));
			if (!N) N = Cast<UNiagaraSystem>(LoadOrLogOnce(NiagaraPathFor(JamEvents::Foul)));
			if (!S) S = Cast<USoundBase>(LoadOrLogOnce(SoundPathFor(JamEvents::Foul)));
			Niagara.Add(E, N);
			Sounds.Add(E, S);
			UE_LOG(LogJamFx, Log, TEXT("JAMFX shot_blocked uses %s / %s at Variant 0"), N ? *N->GetName() : TEXT("no burst"), S ? *S->GetName() : TEXT("no sound"));
			continue;
		}
		UObject* N = IsSoundOnlyEvent(E) ? nullptr : LoadOrLogOnce(NiagaraPathFor(E));   // sound-only events: no burst, no lookup, no log line
		Niagara.Add(E, Cast<UNiagaraSystem>(N));
		UObject* S = LoadOrLogOnce(SoundPathFor(E));
		Sounds.Add(E, Cast<USoundBase>(S));
	}
	Bus->OnJamEvent.AddDynamic(this, &UJamEffectsBinder::OnJamEvent);
}

void UJamEffectsBinder::EndPlay(const EEndPlayReason::Type Reason)
{
	if (Bus)
	{
		Bus->OnJamEvent.RemoveDynamic(this, &UJamEffectsBinder::OnJamEvent);
	}
	Super::EndPlay(Reason);
}

void UJamEffectsBinder::OnJamEvent(FName Event, const FJamEventPayload& Payload)
{
	HandleEvent(Event, Payload);
}

FName UJamEffectsBinder::ZoneKindOf(FName ZoneId) const
{
	const AHokejGameMode* GM = Cast<AHokejGameMode>(GetOwner());
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	if (!Sim) return NAME_None;
	const FString Id = ZoneId.ToString();
	for (const FJamZoneConfig& Z : Sim->GetConfig().Rink.Zones)
	{
		if (Z.Id == Id) return Z.Kind;
	}
	return NAME_None;
}

FLinearColor UJamEffectsBinder::ColourFor(FName Event, const FJamEventPayload& Payload) const
{
	if (Event == JamEvents::ZoneEnter || Event == JamEvents::ZoneExit)
	{
		return ZoneKindColour(ZoneKindOf(Payload.Kind));
	}
	if (Event == JamEvents::PickupSpawn || Event == JamEvents::PickupTake || Event == JamEvents::EffectStart || Event == JamEvents::EffectEnd)
	{
		// Kind is the pickup kind, or the effect id (a built-in id is its kind; a custom id is white).
		return PickupKindColour(Payload.Kind);
	}
	// Team colour of Who (linear), white when Who is -1 or unknown.
	if (const UJamPresentation* P = UJamPresentation::Get(this))
	{
		const FJamPresentationFrame& F = P->GetFrame();
		if (Event == JamEvents::Goal && P->HasFrame())
		{
			// Lane A47 (B-to-A-0030 fault 8): the goal burst takes the scoring team's colour, read from the side of the net the puck is in; Who is the last
			// toucher, which on an own goal is the conceding team. Team 0 attacks AttackGoalX(0) (swapped nets included).
			const AHokejGameMode* GM = Cast<AHokejGameMode>(GetOwner());
			if (const UJamSim* Sim = GM ? GM->GetSim() : nullptr)
			{
				const int32 Scoring = (Payload.Where.X > 0.0) == (Sim->AttackGoalX(0) > 0.0) ? 0 : 1;
				return F.GetTeamColour(Scoring);
			}
		}
		if (P->HasFrame() && F.Skaters.IsValidIndex(Payload.Who))
		{
			return F.GetTeamColour(F.Skaters[Payload.Who].TeamId);
		}
	}
	return FLinearColor::White;
}

int32 UJamEffectsBinder::ResolveScoringTeam(const int32 Before[2], const int32 After[2], int32 FallbackTeam)
{
	if (After[0] > Before[0] && After[1] <= Before[1]) return 0;
	if (After[1] > Before[1] && After[0] <= Before[0]) return 1;
	return (FallbackTeam == 0 || FallbackTeam == 1) ? FallbackTeam : -1;
}

float UJamEffectsBinder::GoalVariantFor(const FJamEventPayload& Payload)
{
	// R-0033 point 4 (lane A50). The payload carries Who = the last touch and no team; the scoring team is the one whose score grew since the last goal
	// this binder saw (a restart or LoadCut lowers the scores: the memory resyncs). The team of Who is the fallback.
	const UWorld* World = GetWorld();
	const AHokejGameMode* GM = World ? World->GetAuthGameMode<AHokejGameMode>() : nullptr;
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	if (!Sim) return 0.f;
	const int32 After[2] = { Sim->GetScore(0), Sim->GetScore(1) };
	if (After[0] < PrevScore[0] || After[1] < PrevScore[1]) { PrevScore[0] = PrevScore[1] = 0; }
	const TArray<FJamSkaterState>& Skaters = Sim->GetSkaters();
	const int32 WhoTeam = Skaters.IsValidIndex(Payload.Who) ? Skaters[Payload.Who].Team : -1;
	const int32 Scorer = ResolveScoringTeam(PrevScore, After, WhoTeam);
	PrevScore[0] = After[0];
	PrevScore[1] = After[1];
	const float Variant = GoalVariant(Scorer, Sim->GetHumanTeam());
	UE_LOG(LogJamFx, Log, TEXT("JAMFX goal scorer_team=%d human_team=%d variant=%d"), Scorer, Sim->GetHumanTeam(), static_cast<int32>(Variant));
	return Variant;
}

void UJamEffectsBinder::HandleEvent(FName Event, const FJamEventPayload& Payload)
{
	++HandledEvents;
	UWorld* World = GetWorld();
	if (!World || IsSilentEvent(Event))
	{
		return;
	}
	float Variant = VariantFor(Event, Payload.Kind);
	if (Event == JamEvents::Goal) Variant = GoalVariantFor(Payload);   // R-0033 point 4 (lane A50): 0 the human's team scored, 1 it conceded
	// Lane A45 (R-0031 point 4): a skater into the boards wobbles the glass for GlassWobbleFrames when B's glass carries HitWobble.
	if (Event == JamEvents::Board && Payload.Kind == FName(TEXT("skater"))) StartGlassWobble(Payload);
	if (const TObjectPtr<UNiagaraSystem>* N = Niagara.Find(Event))
	{
		if (UNiagaraSystem* Sys = N->Get())
		{
			if (UNiagaraComponent* C = UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, Sys, Payload.Where, FRotator::ZeroRotator))
			{
				C->SetVariableFloat(TEXT("User.Strength"), Payload.Strength);
				C->SetVariableLinearColor(TEXT("User.Colour"), ColourFor(Event, Payload));
				C->SetVariableFloat(TEXT("User.Variant"), Variant);
				// Lane A47 (B-to-A-0029 asks 1 and 2): the sim's contact normal rides the payload as Dir (unit, world x/y, z 0; zero when the sim gave none).
				// NS_check and NS_knockdown read User.Dir (chips along the hit, the slide streak); NS_board reads User.Normal (into the rink: chips leave the face).
				// Lane A95 (B-to-A-0037): lift and poke carry Dir too (attacker -> victim; NS_lift's mark on the attacker's side, splinters along the hit).
				if (Event == JamEvents::Check || Event == JamEvents::Knockdown || Event == JamEvents::Lift || Event == JamEvents::Poke) C->SetVariableVec3(TEXT("User.Dir"), Payload.Dir);
				else if (Event == JamEvents::Board) C->SetVariableVec3(TEXT("User.Normal"), Payload.Dir);
				++SpawnedNiagara;
			}
		}
	}
	if (const TObjectPtr<USoundBase>* S = Sounds.Find(Event))
	{
		if (USoundBase* Snd = S->Get())
		{
			// One-shot; MetaSounds take float inputs "Strength" and "Variant" (set before the first render).
			// bark, signal and announce are UI blips and a sting: 2D, not positioned at Where (B-to-A-0012 point 1).
			UAudioComponent* A = IsSound2D(Event)
				? UGameplayStatics::SpawnSound2D(World, Snd, 1.f, 1.f, 0.f, nullptr, false, true)
				: UGameplayStatics::SpawnSoundAtLocation(World, Snd, Payload.Where, FRotator::ZeroRotator, 1.f, 1.f, 0.f, nullptr, nullptr, true);
			if (A)
			{
				A->SetFloatParameter(TEXT("Strength"), Payload.Strength);
				A->SetFloatParameter(TEXT("Variant"), Variant);
				++PlayedSounds;
			}
		}
	}
}

// ---- Lane A65: the match's assets, preloaded while the menu is up (PROFILING.md offenders 3 and 4) ----

DEFINE_LOG_CATEGORY_STATIC(LogJamPreload, Log, All);

bool UJamPreloadSubsystem::IsKnownMissing(const FSoftObjectPath& Path)
{
	FScopeLock Lock(&KnownMissingLock);
	return KnownMissingSet().Contains(Path.GetLongPackageName());
}

FString UJamPreloadSubsystem::LikelyNextCut()
{
	FString Cut;
	if (FParse::Value(FCommandLine::Get(), TEXT("-cut="), Cut) || FParse::Value(FCommandLine::Get(), TEXT("cut="), Cut)) return Cut;
	if (IConsoleVariable* V = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.Cut"))) return V->GetString();
	return TEXT("hockey.regular");
}

TArray<FSoftObjectPath> UJamPreloadSubsystem::MatchPaths(FName Visual, const FString& Tier)
{
	TArray<FSoftObjectPath> P;
	auto Add = [&P](const FString& S) { P.AddUnique(FSoftObjectPath(S)); };
	auto Look = [&Add](const TCHAR* Name) { Add(FString::Printf(TEXT("/JamAssets/Look/%s.%s"), Name, Name)); };
	// UJamEffectsBinder::BeginPlay: every known event's burst and sound (the same rules: silent events none, sound-only events no burst).
	for (const FName& E : UJamEventBus::GetKnownEvents())
	{
		if (UJamEffectsBinder::IsSilentEvent(E)) continue;
		if (!UJamEffectsBinder::IsSoundOnlyEvent(E)) P.AddUnique(UJamEffectsBinder::NiagaraPathFor(E));
		P.AddUnique(UJamEffectsBinder::SoundPathFor(E));
	}
	// The puck (JamPawns.cpp), the figures' blade spray, the commentary system's players.
	Add(TEXT("/JamAssets/VFX/NS_puck_trail.NS_puck_trail"));
	Add(TEXT("/JamAssets/VFX/NS_blade_spray.NS_blade_spray"));
	Add(TEXT("/JamAssets/Props/SM_PuckDisc.SM_PuckDisc"));
	Add(TEXT("/JamAssets/Audio/MS_commentary.MS_commentary"));
	Add(TEXT("/JamAssets/Audio/MS_music_match.MS_music_match"));
	Add(TEXT("/JamAssets/Audio/MS_skate_loop.MS_skate_loop"));
	Add(TEXT("/JamAssets/Audio/MS_crowd_bed.MS_crowd_bed"));
	// The figure class for this visual and tier (AHokejGameMode::ResolveVisualClass), and the figure material of the C++ figures.
	for (const FString& C : AHokejGameMode::VisualClassPathsFor(Visual, Tier)) Add(C);
	Add(TEXT("/JamAssets/Look/M_Toon.M_Toon"));
	// The rink's look instances (RinkActor.cpp LoadLook / LoadConstructionMat), the puck's, the glass of the tier, the look collection.
	static const TCHAR* const LookNames[] = { TEXT("MI_Ice_Night"), TEXT("MI_Toon_Board"), TEXT("MI_Toon_KickPlate"), TEXT("MI_Toon_GlassRail"), TEXT("MI_Trim_Blue"),
		TEXT("MI_Trim_Orange"), TEXT("MI_Toon_NetFrame"), TEXT("MI_Toon_NetMesh"), TEXT("MI_Toon_GoalLight"), TEXT("MI_Toon_Bench"), TEXT("MI_Toon_Bumper"),
		TEXT("MI_Toon_GoalShield"), TEXT("MI_Trim_Beacon"), TEXT("MI_Trim_BeaconOpaque"), TEXT("MI_Toon_Puck"), TEXT("MI_Toon_Helmet"), TEXT("MI_Toon_Stick"),
		TEXT("MPC_Look") };
	for (const TCHAR* N : LookNames) Look(N);
	Look(Tier == TEXT("p4_broadcast") ? TEXT("MI_Bcast_Glass") : TEXT("MI_Glass"));
	Add(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	Add(TEXT("/Engine/BasicShapes/Plane.Plane"));
	// The HUD's and the pause card's textures (FJamUiStyle::Art): loaded in the menu too, but collected with the menu world, so the match loaded them again.
	static const TCHAR* const UiNames[] = { TEXT("T_UI_Panel"), TEXT("T_UI_CallCard"), TEXT("T_UI_Button_Normal"), TEXT("T_UI_Button_Hover"),
		TEXT("T_UI_Button_Pressed"), TEXT("T_UI_RingPlain"), TEXT("T_UI_Ring"), TEXT("T_UI_CutFrame"), TEXT("T_UI_TitleHokej") };
	for (const TCHAR* N : UiNames) Add(FString::Printf(TEXT("/Game/UI/%s.%s"), N, N));
	// The commentary waves (UJamCommentarySystem asks for them asynchronously at match start; the first line loaded synchronously).
	for (const FJamCommentaryLine& L : FJamCommentaryBank::Embedded().Lines) if (!L.Sound.IsNull()) P.AddUnique(L.Sound);
	return P;
}

void UJamPreloadSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UJamPreloadSubsystem::HandlePostLoadMap);
	PreLoadMapHandle = FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UJamPreloadSubsystem::HandlePreLoadMap);
	BeginFrameHandle = FCoreDelegates::OnBeginFrame.AddUObject(this, &UJamPreloadSubsystem::HandleBeginFrame);
}

void UJamPreloadSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
	FCoreUObjectDelegates::PreLoadMap.Remove(PreLoadMapHandle);
	FCoreDelegates::OnBeginFrame.Remove(BeginFrameHandle);
	if (SyncLoadHandle.IsValid()) FCoreDelegates::OnSyncLoadPackage.Remove(SyncLoadHandle);
	for (TSharedPtr<FStreamableHandle>& H : Handles) if (H.IsValid()) H->ReleaseHandle();
	Handles.Reset();
	Super::Deinitialize();
}

static TAutoConsoleVariable<int32> CVarJamPreloadMap(TEXT("jam.PreloadMap"), 1,
	TEXT("1: the match map's package loads asynchronously while the menu is up (lane A114). 0: it loads synchronously in the menu -> match frame (before)."));
static const TCHAR* const JamMatchMapPackage = TEXT("/Game/Maps/Rink");
static TAutoConsoleVariable<int32> CVarJamStartQuietFrames(TEXT("jam.StartQuietFrames"), 0,
	TEXT("Lane A124: frames the menu -> match travel waits after every sound is stopped (4 tried: LoadMap's audio flush waited 22-38 ms all the same, PROFILING-4.md). 0 (default): the travel at once (before)."));
static TAutoConsoleVariable<int32> CVarJamPreloadRig(TEXT("jam.PreloadRig"), 0,
	TEXT("Lane A124: 1: the likely tier's light rig map is loaded asynchronously behind the menu with the match assets (its level instance then finds its imports in memory). 0: before."));

void UJamPreloadSubsystem::HandleBeginFrame()
{
	UGameInstance* GI = GetGameInstance();
	FWorldContext* Ctx = GI ? GI->GetWorldContext() : nullptr;
	if (!Ctx) return;
	if (QuietFramesLeft > 0)
	{
		if (--QuietFramesLeft > 0) return;
		// Hand the held travel back: UGameEngine::Tick's TickWorldTravel, later in this frame, opens the match map.
		if (Ctx->TravelURL.IsEmpty())
		{
			Ctx->TravelURL = HeldTravelURL;
			Ctx->TravelType = static_cast<ETravelType>(HeldTravelType);
		}
		UE_LOG(LogJamPreload, Log, TEXT("JAMPRELOAD quiet: travel released frame=%llu"), static_cast<unsigned long long>(GFrameCounter));
		HeldTravelURL.Reset();
		return;
	}
	if (Ctx->TravelURL.IsEmpty() || GIsEditor) return;
	UWorld* W = Ctx->World();
	if (!W || W->GetMapName() != TEXT("Menu") || !Ctx->TravelURL.Contains(TEXT("Rink"))) return;
	const int32 Frames = CVarJamStartQuietFrames.GetValueOnGameThread();
	if (Frames <= 0) return;
	if (FAudioDeviceHandle Dev = W->GetAudioDevice()) Dev->StopAllSounds(true);   // queued to the audio thread; nothing waits here
	HeldTravelURL = Ctx->TravelURL;
	HeldTravelType = static_cast<int32>(Ctx->TravelType);
	Ctx->TravelURL.Empty();
	QuietFramesLeft = Frames;
	UE_LOG(LogJamPreload, Log, TEXT("JAMPRELOAD quiet: sounds stopped, travel to %s held %d frames frame=%llu"), *HeldTravelURL, Frames, static_cast<unsigned long long>(GFrameCounter));
}

void UJamPreloadSubsystem::HandlePostLoadMap(UWorld* World)
{
	if (!World || !World->IsGameWorld() || World->GetGameInstance() != GetGameInstance()) return;
	if (World->GetMapName() == TEXT("Menu"))
	{
		StartPreload(LikelyNextCut());
		StartMapPreload();
	}
	else if (MapPackage || bMapRequested)
	{
		// The match map is up: the world holds its own package now; let ours go so it is collected with the world when the match ends.
		UE_LOG(LogJamPreload, Log, TEXT("JAMPRELOAD map %s released at %s"), JamMatchMapPackage, *World->GetMapName());
		MapPackage = nullptr;
		MapWorld = nullptr;
		bMapRequested = false;
	}
}

void UJamPreloadSubsystem::StartMapPreload()
{
	// Packaged and -game only: in the editor (PIE, automation) the map is the editor's and PIE renames it, so nothing is held there.
	if (GIsEditor || bMapRequested || MapPackage || CVarJamPreloadMap.GetValueOnGameThread() == 0) return;
	if (!FPackageName::DoesPackageExist(JamMatchMapPackage)) return;
	bMapRequested = true;
	MapRequestedAt = FPlatformTime::Seconds();
	MapLoadedAt = 0.0;
	// No UWorld::WorldTypePreLoadMap entry: the world stays EWorldType::Inactive while it waits in memory (LoadMap's world-leak check exempts Inactive
	// worlds and fails fatally on a Game world no context owns); LoadMap makes it the Game world when it takes it.
	TWeakObjectPtr<UJamPreloadSubsystem> Weak(this);
	MapRequestId = LoadPackageAsync(JamMatchMapPackage, FLoadPackageAsyncDelegate::CreateLambda([Weak](const FName& Name, UPackage* Package, EAsyncLoadingResult::Type Result)
	{
		UJamPreloadSubsystem* Self = Weak.Get();
		if (!Self || !Self->bMapRequested) return;
		Self->MapRequestId = INDEX_NONE;   // the match started before the load finished: LoadMap took it (or loads it itself)
		if (Result == EAsyncLoadingResult::Succeeded && Package)
		{
			Self->MapPackage = Package;
			Self->MapWorld = UWorld::FindWorldInPackage(Package);
			Self->MapLoadedAt = FPlatformTime::Seconds();
			UE_LOG(LogJamPreload, Log, TEXT("JAMPRELOAD map %s loaded in %.0f ms (world %s)"), *Name.ToString(), (Self->MapLoadedAt - Self->MapRequestedAt) * 1000.0,
				Self->MapWorld ? *Self->MapWorld->GetName() : TEXT("none"));
		}
		else UE_LOG(LogJamPreload, Warning, TEXT("JAMPRELOAD map %s failed (%d): it loads at match start"), *Name.ToString(), static_cast<int32>(Result));
	}));
	UE_LOG(LogJamPreload, Log, TEXT("JAMPRELOAD map %s requested"), JamMatchMapPackage);
}

void UJamPreloadSubsystem::HandlePreLoadMap(const FString& MapName)
{
	if (!MapName.Contains(TEXT("Rink"))) return;
	if (bMapRequested && !MapPackage && MapRequestId != INDEX_NONE)
	{
		// The match started before the map's async load finished: finish it now (LoadMap would wait for it anyway), so LoadMap never finds a half-loaded package.
		FlushAsyncLoading(MapRequestId);
		UE_LOG(LogJamPreload, Log, TEXT("JAMPRELOAD map flushed at match start (%s)"), MapPackage ? TEXT("loaded") : TEXT("not loaded"));
	}
	const double Now = FPlatformTime::Seconds();
	const FString State = Handles.Num() == 0 ? FString(TEXT("no preload"))
		: IsPreloadComplete() ? FString::Printf(TEXT("complete (%.0f ms after the request)"), (CompletedAt - RequestedAt) * 1000.0) : FString(TEXT("still loading"));
	UE_LOG(LogJamPreload, Log, TEXT("JAMPRELOAD match start: %d assets requested %.2f s ago, %s; map %s frame=%llu"), Requested, RequestedAt > 0.0 ? Now - RequestedAt : -1.0, *State,
		MapPackage ? TEXT("in memory") : bMapRequested ? TEXT("still loading") : TEXT("not preloaded"), static_cast<unsigned long long>(GFrameCounter));
}

void UJamPreloadSubsystem::HandleSyncLoad(const FString& PackageName)
{
#if !UE_BUILD_SHIPPING
	// Diagnostics for the next profiler: what the game thread still loads synchronously after the menu is up.
	if (SyncLoadsLogged < 400)
	{
		++SyncLoadsLogged;
		UE_LOG(LogJamPreload, Log, TEXT("JAMPRELOAD sync %s frame=%llu"), *PackageName, static_cast<unsigned long long>(GFrameCounter));
	}
#endif
}

void UJamPreloadSubsystem::StartPreload(const FString& CutNameOrPath)
{
	if (PreloadedCuts.Contains(CutNameOrPath) || !UAssetManager::IsInitialized()) return;
	PreloadedCuts.Add(CutNameOrPath);
#if !UE_BUILD_SHIPPING
	if (!SyncLoadHandle.IsValid()) SyncLoadHandle = FCoreDelegates::OnSyncLoadPackage.AddUObject(this, &UJamPreloadSubsystem::HandleSyncLoad);
#endif
	FName Visual(TEXT("humanoid_b1"));
	FString Tier(TEXT("l2_night"));
	TArray<FString> Errors;
	if (const UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(CutNameOrPath), Errors, GetTransientPackage()))
	{
		Visual = C->Game.Skater.Visual;
		if (!C->Game.Look.Tier.IsEmpty()) Tier = C->Game.Look.Tier;
	}
	LikelyTier = Tier;
	TArray<FSoftObjectPath> Want;
	int32 Missing = 0;
	TArray<FSoftObjectPath> Paths = MatchPaths(Visual, Tier);
	// Lane A124: the tier's light rig map (packaged and -game only; in the editor the rig is the editor's). The handle holds it as an Inactive world, which
	// LoadMap's world-leak check exempts; the match's level instance loads its own renamed copy and finds the rig's meshes, materials and lights in memory.
	const FString Rig = ULookTierSwitcher::GetRigMapPath(Tier);
	if (!GIsEditor && !Rig.IsEmpty() && CVarJamPreloadRig.GetValueOnGameThread() != 0) Paths.Add(FSoftObjectPath(Rig));
	for (const FSoftObjectPath& P : Paths)
	{
		const FString Pkg = P.GetLongPackageName();
		if (FPackageName::DoesPackageExist(Pkg)) { Want.Add(P); continue; }
		FScopeLock Lock(&KnownMissingLock);
		KnownMissingSet().Add(Pkg);
		++Missing;
	}
	const int32 N = Want.Num();
	Requested += N;
	RequestedAt = FPlatformTime::Seconds();
	CompletedAt = 0.0;
	TWeakObjectPtr<UJamPreloadSubsystem> Weak(this);
	TSharedPtr<FStreamableHandle> H = UAssetManager::GetStreamableManager().RequestAsyncLoad(MoveTemp(Want), FStreamableDelegate::CreateLambda([Weak, N]()
	{
		if (UJamPreloadSubsystem* Self = Weak.Get())
		{
			Self->CompletedAt = FPlatformTime::Seconds();
			UE_LOG(LogJamPreload, Log, TEXT("JAMPRELOAD done: %d assets in %.0f ms"), N, (Self->CompletedAt - Self->RequestedAt) * 1000.0);
		}
	}), FStreamableManager::DefaultAsyncLoadPriority, false, false, FString(TEXT("JamMatchPreload")));
	if (H.IsValid()) Handles.Add(H);
	UE_LOG(LogJamPreload, Log, TEXT("JAMPRELOAD requested %d assets for cut %s (visual %s, tier %s); %d absent packages will not be looked up again"),
		N, *CutNameOrPath, *Visual.ToString(), *Tier, Missing);
}
