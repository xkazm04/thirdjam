#include "SignalSystem.h"
#include "BarkSystem.h"
#include "HokejGameMode.h"
#include "HUDFeed.h"
#include "JamEventBus.h"
#include "JamSim.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY(LogJamSignals);

namespace JamSignals
{
	const FName BusEvent(TEXT("signal"));
}

// ---------------------------------------------------------------- input map ----------------------------------------------------------------

UJamSignalInputMap* UJamSignalInputMap::Get()
{
	static TWeakObjectPtr<UJamSignalInputMap> Cached;
	if (!Cached.IsValid())
	{
		UJamSignalInputMap* M = NewObject<UJamSignalInputMap>(GetTransientPackage(), TEXT("JamSignalInputMap"));
		M->AddToRoot();
		M->Build();
		Cached = M;
	}
	return Cached.Get();
}

void UJamSignalInputMap::Build()
{
	Context = NewObject<UInputMappingContext>(this, TEXT("IMC_HokejSignals"));
	const TCHAR* Names[] = { TEXT("IA_SignalDefend"), TEXT("IA_SignalSpace"), TEXT("IA_SignalPass"), TEXT("IA_SignalPraise") };
	const FKey Keys[] = { EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four };
	// D-pad: down = fall back (defend), up = spread (space), left = pass to me, right = praise.
	const FKey Pads[] = { EKeys::Gamepad_DPad_Down, EKeys::Gamepad_DPad_Up, EKeys::Gamepad_DPad_Left, EKeys::Gamepad_DPad_Right };
	for (int32 i = 0; i < 4; ++i)
	{
		UInputAction* A = NewObject<UInputAction>(this, Names[i]);
		A->ValueType = EInputActionValueType::Boolean;
		Actions.Add(A);
		Context->MapKey(A, Keys[i]);
		Context->MapKey(A, Pads[i]);
	}
}

bool UJamSignalInputMap::IsKeyMapped(int32 Slot, const FKey& Key) const
{
	if (!Actions.IsValidIndex(Slot) || !Context) return false;
	for (const FEnhancedActionKeyMapping& M : Context->GetMappings()) if (M.Action == Actions[Slot] && M.Key == Key) return true;
	return false;
}

// ---------------------------------------------------------------- pure ----------------------------------------------------------------

EJamTeamIntent UJamSignalSystem::IntentForSlot(int32 Slot)
{
	switch (Slot)
	{
	case 1: return EJamTeamIntent::Defend;
	case 2: return EJamTeamIntent::Space;
	case 3: return EJamTeamIntent::Pass;
	case 4: return EJamTeamIntent::Praise;
	default: return EJamTeamIntent::None;
	}
}

FName UJamSignalSystem::AckSituationFor(EJamTeamIntent Intent)
{
	switch (Intent)
	{
	case EJamTeamIntent::Defend: return JamBarks::SignalAckDefend;
	case EJamTeamIntent::Space: return JamBarks::SignalAckSpace;
	case EJamTeamIntent::Pass: return JamBarks::SignalAckPass;
	case EJamTeamIntent::Praise: return JamBarks::PraiseReply;
	default: return NAME_None;
	}
}

FJamTeamIntent UJamSignalSystem::Make(EJamTeamIntent Intent, int64 Frame, int32 Issuer, int32 Target)
{
	FJamTeamIntent I;
	I.Intent = Intent;
	I.Frames = FJamTeamIntent::DefaultFrames(Intent);
	I.IssuedFrame = Frame;
	I.IssuerIndex = Issuer;
	I.TargetIndex = Target;
	return I;
}

// ---------------------------------------------------------------- subsystem ----------------------------------------------------------------

UJamSignalSystem* UJamSignalSystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	return World ? World->GetSubsystem<UJamSignalSystem>() : nullptr;
}

bool UJamSignalSystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer)) return false;
	const UWorld* World = Cast<UWorld>(Outer);
	return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UJamSignalSystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	if (UJamPresentation* Pres = UJamPresentation::Get(&InWorld)) FrameHandle = Pres->OnSimFrame.AddUObject(this, &UJamSignalSystem::HandleSimFrame);
	TryBind();
}

void UJamSignalSystem::Deinitialize()
{
	if (UWorld* World = GetWorld()) if (UJamPresentation* Pres = UJamPresentation::Get(World)) Pres->OnSimFrame.Remove(FrameHandle);
	if (BoundPC.IsValid())
	{
		if (ULocalPlayer* LP = BoundPC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Sub = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()) Sub->RemoveMappingContext(UJamSignalInputMap::Get()->Context);
		}
	}
	Super::Deinitialize();
}

void UJamSignalSystem::HandleSimFrame(const FJamPresentationFrame& F)
{
	Frame = F.SimFrame;
	if (!bBound) TryBind();
}

void UJamSignalSystem::TryBind()
{
	UWorld* World = GetWorld();
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	if (!PC || !PC->IsLocalController() || !PC->InputComponent) return;
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PC->InputComponent);
	ULocalPlayer* LP = PC->GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* Sub = LP ? LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
	if (!EIC || !Sub) return;
	UJamSignalInputMap* M = UJamSignalInputMap::Get();
	Sub->AddMappingContext(M->Context, 1);
	EIC->BindAction(M->Actions[0], ETriggerEvent::Started, this, &UJamSignalSystem::OnSlot1);
	EIC->BindAction(M->Actions[1], ETriggerEvent::Started, this, &UJamSignalSystem::OnSlot2);
	EIC->BindAction(M->Actions[2], ETriggerEvent::Started, this, &UJamSignalSystem::OnSlot3);
	EIC->BindAction(M->Actions[3], ETriggerEvent::Started, this, &UJamSignalSystem::OnSlot4);
	BoundPC = PC;
	bBound = true;
	UE_LOG(LogJamSignals, Log, TEXT("JAMSIGNAL input bound: keys 1-4 and the d-pad"));
}

int64 UJamSignalSystem::CurrentFrame() const
{
	const UWorld* World = GetWorld();
	const AHokejGameMode* GM = World ? Cast<AHokejGameMode>(World->GetAuthGameMode()) : nullptr;
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	return Sim ? Sim->GetFrame() : Frame;
}

bool UJamSignalSystem::GetActiveIntent(FJamTeamIntent& Out) const
{
	if (!Last.IsActive(CurrentFrame())) return false;
	Out = Last;
	return true;
}

bool UJamSignalSystem::Issue(EJamTeamIntent Intent)
{
	if (Intent == EJamTeamIntent::None) return false;
	UWorld* World = GetWorld();
	AHokejGameMode* GM = World ? Cast<AHokejGameMode>(World->GetAuthGameMode()) : nullptr;
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	if (!Sim || Sim->GetPhase() != EJamSimPhase::Playing) return false;
	const int64 Now = Sim->GetFrame();
	if (Last.Intent != EJamTeamIntent::None && Now - Last.IssuedFrame < JamSignals::RepeatCooldownFrames) return false;
	const int32 Human = Sim->GetHumanIndex();
	const TArray<FJamSkaterState>& Skaters = Sim->GetSkaters();
	if (!Skaters.IsValidIndex(Human)) return false;
	// The nearest bot teammate to the human: the one that answers (and the target of praise and pass).
	int32 Nearest = -1;
	double BestD = 1e30;
	for (const FJamSkaterState& S : Skaters)
	{
		if (S.Index == Human || S.Team != Skaters[Human].Team) continue;
		const double D = FVector2D::DistSquared(S.Pos, Skaters[Human].Pos);
		if (D < BestD) { BestD = D; Nearest = S.Index; }
	}
	if (Nearest < 0) return false;
	const int32 Target = (Intent == EJamTeamIntent::Praise || Intent == EJamTeamIntent::Pass) ? Nearest : -1;
	Last = Make(Intent, Now, Human, Target);
	++SignalCount;
	UE_LOG(LogJamSignals, Log, TEXT("JAMSIGNAL intent=%s frame=%lld frames=%d issuer=%d target=%d"), *FJamTeamIntent::NameOf(Intent).ToString(), Now, Last.Frames, Human, Target);
	OnIntent.Broadcast(Last);
	if (UGameInstance* GI = World->GetGameInstance())
	{
		if (UJamEventBus* Bus = GI->GetSubsystem<UJamEventBus>())
		{
			if (UJamEventBus::IsKnownEvent(JamSignals::BusEvent))
			{
				FJamEventPayload P;
				P.Who = Human;
				P.Where = UJamSim::ToWorld(Skaters[Human].Pos);
				P.Strength = FMath::Clamp(Last.Frames / 600.f, 0.f, 1.f);
				P.Kind = FJamTeamIntent::NameOf(Intent);
				Bus->Broadcast(JamSignals::BusEvent, P);
			}
		}
	}
	if (UJamBarkSystem* Barks = UJamBarkSystem::Get(World)) Barks->Trigger(AckSituationFor(Intent), EJamBarkSpeaker::Teammate, Nearest, true);
	return true;
}

static FAutoConsoleCommandWithWorldAndArgs CmdJamSignal(TEXT("jam.signal"),
	TEXT("jam.signal <defend|space|pass|praise>: give a team signal as if key 1-4 were pressed."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UJamSignalSystem* S = World ? World->GetSubsystem<UJamSignalSystem>() : nullptr;
		if (!S || Args.Num() == 0) { UE_LOG(LogJamSignals, Warning, TEXT("jam.signal: no signal system in this world or no intent given")); return; }
		const bool bOk = S->Issue(FJamTeamIntent::FromName(FName(*Args[0])));
		UE_LOG(LogJamSignals, Log, TEXT("jam.signal %s: %s"), *Args[0], bOk ? TEXT("issued") : TEXT("ignored (unknown, not playing, no teammate or repeat cooldown)"));
	}));
