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

DEFINE_LOG_CATEGORY(LogJamFx);

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
	return 0.f;   // shot_blocked reuses the foul assets at Variant 0 (B-to-A-0016); everything else has one kind
}

bool UJamEffectsBinder::IsSound2D(FName Event)
{
	return Event == JamEvents::Bark || Event == JamEvents::Signal || Event == JamEvents::Announce;
}

bool UJamEffectsBinder::IsSilentEvent(FName Event)
{
	return Event == JamEvents::Period;
}

bool UJamEffectsBinder::IsSoundOnlyEvent(FName Event)
{
	// Director, H9.4: B makes no NS_ for these four on purpose (MS_lose, MS_bark, MS_signal, MS_announce only).
	return Event == JamEvents::Lose || Event == JamEvents::Bark || Event == JamEvents::Signal || Event == JamEvents::Announce;
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
		if (P->HasFrame() && F.Skaters.IsValidIndex(Payload.Who))
		{
			return F.GetTeamColour(F.Skaters[Payload.Who].TeamId);
		}
	}
	return FLinearColor::White;
}

void UJamEffectsBinder::HandleEvent(FName Event, const FJamEventPayload& Payload)
{
	++HandledEvents;
	UWorld* World = GetWorld();
	if (!World || IsSilentEvent(Event))
	{
		return;
	}
	const float Variant = VariantFor(Event, Payload.Kind);
	if (const TObjectPtr<UNiagaraSystem>* N = Niagara.Find(Event))
	{
		if (UNiagaraSystem* Sys = N->Get())
		{
			if (UNiagaraComponent* C = UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, Sys, Payload.Where, FRotator::ZeroRotator))
			{
				C->SetVariableFloat(TEXT("User.Strength"), Payload.Strength);
				C->SetVariableLinearColor(TEXT("User.Colour"), ColourFor(Event, Payload));
				C->SetVariableFloat(TEXT("User.Variant"), Variant);
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
