#include "InputMapping.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "InputTriggers.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "InputKeyEventArgs.h"

namespace
{
	UInputAction* MakeAction(UObject* Outer, const TCHAR* Name, EInputActionValueType Type)
	{
		UInputAction* A = NewObject<UInputAction>(Outer, Name);
		A->ValueType = Type;
		return A;
	}

	void AddSwizzle(UInputMappingContext* Ctx, FEnhancedActionKeyMapping& M)
	{
		UInputModifierSwizzleAxis* S = NewObject<UInputModifierSwizzleAxis>(Ctx);
		S->Order = EInputAxisSwizzle::YXZ;
		M.Modifiers.Add(S);
	}

	void AddNegate(UInputMappingContext* Ctx, FEnhancedActionKeyMapping& M)
	{
		M.Modifiers.Add(NewObject<UInputModifierNegate>(Ctx));
	}
}

UJamInputMap* UJamInputMap::Get()
{
	static TWeakObjectPtr<UJamInputMap> Cached;
	if (!Cached.IsValid())
	{
		UJamInputMap* M = NewObject<UJamInputMap>(GetTransientPackage(), TEXT("JamInputMap"));
		M->AddToRoot();
		M->Build();
		Cached = M;
	}
	return Cached.Get();
}

void UJamInputMap::Build()
{
	Move = MakeAction(this, TEXT("IA_Move"), EInputActionValueType::Axis2D);
	Shoot = MakeAction(this, TEXT("IA_Shoot"), EInputActionValueType::Boolean);
	Check = MakeAction(this, TEXT("IA_Check"), EInputActionValueType::Boolean);
	Restart = MakeAction(this, TEXT("IA_Restart"), EInputActionValueType::Boolean);
	Brake = MakeAction(this, TEXT("IA_Brake"), EInputActionValueType::Boolean);
	Pause = MakeAction(this, TEXT("IA_Pause"), EInputActionValueType::Boolean);
	StickAim = MakeAction(this, TEXT("IA_StickAim"), EInputActionValueType::Axis2D);
	StickMouse = MakeAction(this, TEXT("IA_StickMouse"), EInputActionValueType::Axis2D);
	Poke = MakeAction(this, TEXT("IA_Poke"), EInputActionValueType::Boolean);
	Lift = MakeAction(this, TEXT("IA_Lift"), EInputActionValueType::Boolean);
	Pass = MakeAction(this, TEXT("IA_Pass"), EInputActionValueType::Boolean);
	Context = NewObject<UInputMappingContext>(this, TEXT("IMC_Hokej"));

	// WASD and arrows: a 1D key feeds X; W / Up swizzle it onto Y, S / Down swizzle and negate, A / Left negate.
	{
		FEnhancedActionKeyMapping& W = Context->MapKey(Move, EKeys::W); AddSwizzle(Context, W);
		FEnhancedActionKeyMapping& S = Context->MapKey(Move, EKeys::S); AddSwizzle(Context, S); AddNegate(Context, S);
		FEnhancedActionKeyMapping& A = Context->MapKey(Move, EKeys::A); AddNegate(Context, A);
		Context->MapKey(Move, EKeys::D);
		FEnhancedActionKeyMapping& U = Context->MapKey(Move, EKeys::Up); AddSwizzle(Context, U);
		FEnhancedActionKeyMapping& Dn = Context->MapKey(Move, EKeys::Down); AddSwizzle(Context, Dn); AddNegate(Context, Dn);
		FEnhancedActionKeyMapping& L = Context->MapKey(Move, EKeys::Left); AddNegate(Context, L);
		Context->MapKey(Move, EKeys::Right);
	}
	{
		FEnhancedActionKeyMapping& Stick = Context->MapKey(Move, EKeys::Gamepad_Left2D);
		UInputModifierDeadZone* DZ = NewObject<UInputModifierDeadZone>(Context);
		DZ->LowerThreshold = 0.25f;
		Stick.Modifiers.Add(DZ);
	}
	Context->MapKey(Shoot, EKeys::SpaceBar);
	Context->MapKey(Shoot, EKeys::Gamepad_FaceButton_Bottom);   // A
	Context->MapKey(Check, EKeys::LeftShift);
	Context->MapKey(Check, EKeys::Gamepad_FaceButton_Right);    // B
	Context->MapKey(Brake, EKeys::LeftControl);
	Context->MapKey(Brake, EKeys::Gamepad_LeftTriggerAxis);  // left trigger (analog key, pressed past the default threshold)
	Context->MapKey(Brake, EKeys::LeftControl);
	Context->MapKey(Brake, EKeys::Gamepad_LeftTriggerAxis);  // left trigger (analog key, actuation threshold 0.5)
	Context->MapKey(Restart, EKeys::R);
	Context->MapKey(Restart, EKeys::Gamepad_Special_Left);      // Back / View
	// R-0013 (lane A16): the stick.
	Context->MapKey(StickAim, EKeys::Gamepad_Right2D);
	Context->MapKey(StickMouse, EKeys::Mouse2D);
	Context->MapKey(Poke, EKeys::LeftMouseButton);
	Context->MapKey(Poke, EKeys::Gamepad_RightShoulder);
	Context->MapKey(Lift, EKeys::RightMouseButton);
	Context->MapKey(Lift, EKeys::Gamepad_LeftShoulder);
	Context->MapKey(Pass, EKeys::E);
	Context->MapKey(Pass, EKeys::Gamepad_FaceButton_Left);   // X
	Context->MapKey(Pause, EKeys::Escape);
	Context->MapKey(Pause, EKeys::Gamepad_Special_Right);       // Start / Menu
}

bool UJamInputMap::IsKeyMapped(const UInputAction* Action, const FKey& Key) const
{
	for (const FEnhancedActionKeyMapping& M : Context->GetMappings())
	{
		if (M.Action == Action && M.Key == Key)
		{
			return true;
		}
	}
	return false;
}

// ---------------------------------------------------------------- component

UJamPlayerInput::UJamPlayerInput()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UJamPlayerInput::BeginPlay()
{
	Super::BeginPlay();
	TryBind();
}

void UJamPlayerInput::EndPlay(const EEndPlayReason::Type Reason)
{
	if (BoundPC.IsValid())
	{
		if (ULocalPlayer* LP = BoundPC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Sub = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				Sub->RemoveMappingContext(UJamInputMap::Get()->Context);
			}
		}
	}
	Super::EndPlay(Reason);
}

void UJamPlayerInput::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!bBound)
	{
		TryBind();  // possession can come after BeginPlay
	}
}

void UJamPlayerInput::TryBind()
{
	APlayerController* PC = nullptr;
	if (const APawn* P = Cast<APawn>(GetOwner()))
	{
		PC = Cast<APlayerController>(P->GetController());
	}
	if (!PC && GetWorld())
	{
		PC = GetWorld()->GetFirstPlayerController();
	}
	if (!PC || !PC->IsLocalController() || !PC->InputComponent)
	{
		return;
	}
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PC->InputComponent);
	ULocalPlayer* LP = PC->GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* Sub = LP ? LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
	if (!EIC || !Sub)
	{
		return;
	}
	UJamInputMap* M = UJamInputMap::Get();
	Sub->AddMappingContext(M->Context, 0);
	EIC->BindAction(M->Move, ETriggerEvent::Triggered, this, &UJamPlayerInput::OnMove);
	EIC->BindAction(M->Move, ETriggerEvent::Completed, this, &UJamPlayerInput::OnMoveDone);
	EIC->BindAction(M->Move, ETriggerEvent::Canceled, this, &UJamPlayerInput::OnMoveDone);
	EIC->BindAction(M->Shoot, ETriggerEvent::Started, this, &UJamPlayerInput::OnShootStarted);
	EIC->BindAction(M->Shoot, ETriggerEvent::Completed, this, &UJamPlayerInput::OnShootDone);
	EIC->BindAction(M->Check, ETriggerEvent::Started, this, &UJamPlayerInput::OnCheckStarted);
	EIC->BindAction(M->Check, ETriggerEvent::Completed, this, &UJamPlayerInput::OnCheckDone);
	EIC->BindAction(M->Brake, ETriggerEvent::Started, this, &UJamPlayerInput::OnBrakeStarted);
	EIC->BindAction(M->Brake, ETriggerEvent::Completed, this, &UJamPlayerInput::OnBrakeDone);
	EIC->BindAction(M->StickAim, ETriggerEvent::Triggered, this, &UJamPlayerInput::OnStickAim);
	EIC->BindAction(M->StickAim, ETriggerEvent::Completed, this, &UJamPlayerInput::OnStickAimDone);
	EIC->BindAction(M->StickAim, ETriggerEvent::Canceled, this, &UJamPlayerInput::OnStickAimDone);
	EIC->BindAction(M->StickMouse, ETriggerEvent::Triggered, this, &UJamPlayerInput::OnStickMouse);
	EIC->BindAction(M->Poke, ETriggerEvent::Started, this, &UJamPlayerInput::OnPokeStarted);
	EIC->BindAction(M->Lift, ETriggerEvent::Started, this, &UJamPlayerInput::OnLiftStarted);
	EIC->BindAction(M->Pass, ETriggerEvent::Started, this, &UJamPlayerInput::OnPassStarted);
	EIC->BindAction(M->Restart, ETriggerEvent::Started, this, &UJamPlayerInput::OnRestartStarted);
	EIC->BindAction(M->Pause, ETriggerEvent::Started, this, &UJamPlayerInput::OnPauseStarted);
	BoundPC = PC;
	bBound = true;
}

void UJamPlayerInput::OnMove(const FInputActionValue& V)
{
	Axis = FVector2D(V.Get<FVector2D>()).ClampAxes(-1.0, 1.0);
}

void UJamPlayerInput::OnMoveDone(const FInputActionValue&)
{
	Axis = FVector2D::ZeroVector;
}

void UJamPlayerInput::OnStickAim(const FInputActionValue& V)
{
	StickAxis = FVector2D(V.Get<FVector2D>()).ClampAxes(-1.0, 1.0);
}

void UJamPlayerInput::OnStickAimDone(const FInputActionValue&)
{
	StickAxis = FVector2D::ZeroVector;
}

void UJamPlayerInput::OnStickMouse(const FInputActionValue& V)
{
	MouseAimPx += FVector2D(V.Get<FVector2D>());
	const double L = MouseAimPx.Size();
	if (L > 200.0) MouseAimPx *= 200.0 / L;
}

bool UJamPlayerInput::ConsumePokePressed()
{
	const bool B = bPokeLatch;
	bPokeLatch = false;
	return B;
}

bool UJamPlayerInput::ConsumePassPressed()
{
	const bool B = bPassLatch;
	bPassLatch = false;
	return B;
}

bool UJamPlayerInput::ConsumeLiftPressed()
{
	const bool B = bLiftLatch;
	bLiftLatch = false;
	return B;
}

bool UJamPlayerInput::ConsumeShootPressed()
{
	const bool B = bShootLatch;
	bShootLatch = false;
	return B;
}

bool UJamPlayerInput::ConsumeCheckPressed()
{
	const bool B = bCheckLatch;
	bCheckLatch = false;
	return B;
}

void UJamPlayerInput::DebugInjectKey(APlayerController* PC, FKey Key, float Value, bool bGamepad)
{
	if (!PC)
	{
		return;
	}
	const FInputDeviceId Device = FInputDeviceId::CreateFromInternalId(0);
	if (Key.IsAnalog())
	{
		PC->InputKey(FInputKeyEventArgs(nullptr, Device, Key, Value, 1.f / 60.f, 1, 0));
	}
	else
	{
		PC->InputKey(FInputKeyEventArgs(nullptr, Device, Key, Value > 0.f ? IE_Pressed : IE_Released, 1.f, false, 0));
	}
}
