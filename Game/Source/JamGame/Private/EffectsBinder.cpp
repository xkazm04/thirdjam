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

	// Quiet load with no log line at all (used where a miss is expected and answered by an alias).
	UObject* LoadSilently(const FSoftObjectPath& Path)
	{
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
				if (Event == JamEvents::Check || Event == JamEvents::Knockdown) C->SetVariableVec3(TEXT("User.Dir"), Payload.Dir);
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
