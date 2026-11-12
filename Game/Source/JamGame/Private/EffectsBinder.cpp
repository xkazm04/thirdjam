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
	UObject* Obj = StaticLoadObject(UObject::StaticClass(), nullptr, *Path.ToString(), nullptr, LOAD_NoWarn | LOAD_Quiet);
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
	// Resolve every known event's assets now, so a first goal does not hitch on a load.
	for (const FName& E : UJamEventBus::GetKnownEvents())
	{
		UObject* N = LoadOrLogOnce(NiagaraPathFor(E));
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

void UJamEffectsBinder::HandleEvent(FName Event, const FJamEventPayload& Payload)
{
	++HandledEvents;
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	if (const TObjectPtr<UNiagaraSystem>* N = Niagara.Find(Event))
	{
		if (UNiagaraSystem* Sys = N->Get())
		{
			if (UNiagaraComponent* C = UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, Sys, Payload.Where, FRotator::ZeroRotator))
			{
				C->SetVariableFloat(TEXT("User.Strength"), Payload.Strength);
				// Team colour of Who (linear), white when Who is -1 or unknown.
				FLinearColor Colour = FLinearColor::White;
				if (const UJamPresentation* P = UJamPresentation::Get(this))
				{
					const FJamPresentationFrame& F = P->GetFrame();
					if (P->HasFrame() && F.Skaters.IsValidIndex(Payload.Who))
					{
						Colour = F.GetTeamColour(F.Skaters[Payload.Who].TeamId);
					}
				}
				C->SetVariableLinearColor(TEXT("User.Colour"), Colour);
				++SpawnedNiagara;
			}
		}
	}
	if (const TObjectPtr<USoundBase>* S = Sounds.Find(Event))
	{
		if (USoundBase* Snd = S->Get())
		{
			// One-shot at Where; MetaSounds take a float input "Strength" (set before the first render).
			if (UAudioComponent* A = UGameplayStatics::SpawnSoundAtLocation(World, Snd, Payload.Where, FRotator::ZeroRotator, 1.f, 1.f, 0.f, nullptr, nullptr, true))
			{
				A->SetFloatParameter(TEXT("Strength"), Payload.Strength);
				++PlayedSounds;
			}
		}
	}
}
