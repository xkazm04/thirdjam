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
#include "HAL/IConsoleManager.h"
#include "UObject/UObjectIterator.h"
#include "JamSim.h"
#include "MenuFlow.h"
#include "HUDWidget.h"
#include "SettingsWidget.h"
#include "MenuStyle.h"
#include "HokejGameMode.h"
#include "HUDFeed.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"

// Lane A49: 1 = one JAMCTRL line per sim frame with the stick the human sends and its source (mouse | pad | mousedelta | none); the button edges log always.
static TAutoConsoleVariable<int32> CVarJamCtrlLog(TEXT("jam.ctrl.Log"), 0, TEXT("1 = JAMCTRL stick line every sim frame (lane A49 proof); button edges always log."));

namespace
{
	bool GJamPadLast = false;   // lane A199
	FJamButtonVerbs GJamLastHumanButtons;   // lane A199: the badge's hold
}

bool UJamInputMap::LogEnabled()
{
	return CVarJamCtrlLog.GetValueOnGameThread() != 0;
}

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

// ---------------------------------------------------------------- lane A183: the bindings

namespace
{
	struct FVerbInfo { const TCHAR* Id; const TCHAR* Label; int32 Role; };
	const FVerbInfo GVerbs[FJamBindings::NumVerbs] = {
		{ TEXT("Shoot"), TEXT("SHOOT"), 1 }, { TEXT("CrossCheck"), TEXT("CROSS-CHECK"), 2 }, { TEXT("Pass"), TEXT("PASS"), 1 }, { TEXT("Check"), TEXT("CHECK"), 2 },
		{ TEXT("Poke"), TEXT("POKE"), 0 }, { TEXT("Deke"), TEXT("DEKE"), 0 }, { TEXT("Spin"), TEXT("SPIN"), 0 }, { TEXT("Ability"), TEXT("ABILITY"), 0 },
		{ TEXT("Sprint"), TEXT("SPRINT"), 0 }, { TEXT("Brake"), TEXT("BRAKE"), 0 },
		{ TEXT("SkateForward"), TEXT("SKATE FORWARD"), 0 }, { TEXT("SkateBack"), TEXT("SKATE BACK"), 0 }, { TEXT("SkateLeft"), TEXT("SKATE LEFT"), 0 },
		{ TEXT("SkateRight"), TEXT("SKATE RIGHT"), 0 }, { TEXT("Restart"), TEXT("RESTART"), 0 } };

	EJamVerb VerbAt(int32 I) { return static_cast<EJamVerb>(I); }
}

bool FJamBindings::operator==(const FJamBindings& O) const
{
	for (int32 I = 0; I < NumVerbs; ++I) if (!(Verbs[I] == O.Verbs[I])) return false;
	return true;
}

FJamBindings FJamBindings::Defaults()
{
	// R-0064 point 1 (the operator, 14 Nov): LMB shoot with the puck / cross-check without (held); RMB pass with the puck (held = saucer) / check without.
	// The pad follows the same rule on RT and RB. Q (pass), MMB and mouse 4 (cross-check), X (pass) and LB (cross-check) stay as second bindings.
	FJamBindings B;
	auto Set = [&](EJamVerb V, TArray<FKey> Keys, TArray<FKey> Pads) { B[V].Keys = MoveTemp(Keys); B[V].Pads = MoveTemp(Pads); };
	Set(EJamVerb::Shoot, { EKeys::LeftMouseButton }, { EKeys::Gamepad_RightTriggerAxis });
	Set(EJamVerb::CrossCheck, { EKeys::LeftMouseButton, EKeys::MiddleMouseButton, EKeys::ThumbMouseButton }, { EKeys::Gamepad_RightTriggerAxis, EKeys::Gamepad_LeftShoulder });
	Set(EJamVerb::Pass, { EKeys::RightMouseButton, EKeys::Q }, { EKeys::Gamepad_RightShoulder, EKeys::Gamepad_FaceButton_Left });
	Set(EJamVerb::Check, { EKeys::RightMouseButton }, { EKeys::Gamepad_RightShoulder });
	Set(EJamVerb::Poke, { EKeys::SpaceBar }, { EKeys::Gamepad_FaceButton_Bottom });
	Set(EJamVerb::Deke, { EKeys::MouseScrollUp, EKeys::MouseScrollDown }, { EKeys::Gamepad_FaceButton_Top });
	Set(EJamVerb::Spin, { EKeys::E }, { EKeys::Gamepad_FaceButton_Right });
	Set(EJamVerb::Ability, { EKeys::F }, { EKeys::Gamepad_RightThumbstick });
	Set(EJamVerb::Sprint, { EKeys::LeftShift }, { EKeys::Gamepad_LeftTriggerAxis });
	Set(EJamVerb::Brake, { EKeys::LeftControl }, { EKeys::Gamepad_LeftThumbstick });
	Set(EJamVerb::SkateForward, { EKeys::W, EKeys::Up }, {});
	Set(EJamVerb::SkateBack, { EKeys::S, EKeys::Down }, {});
	Set(EJamVerb::SkateLeft, { EKeys::A, EKeys::Left }, {});
	Set(EJamVerb::SkateRight, { EKeys::D, EKeys::Right }, {});
	Set(EJamVerb::Restart, { EKeys::R }, { EKeys::Gamepad_Special_Left });
	return B;
}

int32 FJamBindings::Role(EJamVerb V) { return V < EJamVerb::Count ? GVerbs[static_cast<int32>(V)].Role : 0; }
const TCHAR* FJamBindings::VerbId(EJamVerb V) { return V < EJamVerb::Count ? GVerbs[static_cast<int32>(V)].Id : TEXT(""); }
FString FJamBindings::VerbLabel(EJamVerb V) { return V < EJamVerb::Count ? GVerbs[static_cast<int32>(V)].Label : TEXT(""); }

bool FJamBindings::MayShare(EJamVerb A, EJamVerb B)
{
	const int32 RA = Role(A), RB = Role(B);
	return A != B && RA != 0 && RB != 0 && RA != RB;
}

bool FJamBindings::IsReserved(const FKey& Key)
{
	if (!Key.IsValid()) return true;
	if (Key == EKeys::Gamepad_LeftTriggerAxis || Key == EKeys::Gamepad_RightTriggerAxis) return false;   // the triggers bind as their axis keys
	if (Key.IsAxis1D() || Key.IsAxis2D() || Key.IsAxis3D()) return true;
	return Key == EKeys::Escape || Key == EKeys::Gamepad_Special_Right || Key == EKeys::One || Key == EKeys::Two || Key == EKeys::Three || Key == EKeys::Four
		|| Key == EKeys::Gamepad_DPad_Up || Key == EKeys::Gamepad_DPad_Down || Key == EKeys::Gamepad_DPad_Left || Key == EKeys::Gamepad_DPad_Right
		|| Key == EKeys::Mouse2D || Key == EKeys::MouseX || Key == EKeys::MouseY || Key == EKeys::AnyKey
		|| Key == EKeys::Gamepad_LeftTrigger || Key == EKeys::Gamepad_RightTrigger   // the triggers are bound as their axis keys
		// Lane A199 (item 3): the sticks' directions skate and aim; a nudge of the stick while a pad cell listens is not a button.
		|| Key == EKeys::Gamepad_LeftStick_Up || Key == EKeys::Gamepad_LeftStick_Down || Key == EKeys::Gamepad_LeftStick_Left || Key == EKeys::Gamepad_LeftStick_Right
		|| Key == EKeys::Gamepad_RightStick_Up || Key == EKeys::Gamepad_RightStick_Down || Key == EKeys::Gamepad_RightStick_Left || Key == EKeys::Gamepad_RightStick_Right;
}

bool FJamBindings::Has(EJamVerb V, const FKey& Key) const
{
	return V < EJamVerb::Count && ((*this)[V].Keys.Contains(Key) || (*this)[V].Pads.Contains(Key));
}

TArray<EJamVerb> FJamBindings::VerbsOn(const FKey& Key) const
{
	TArray<EJamVerb> Out;
	for (int32 I = 0; I < NumVerbs; ++I) if (Has(VerbAt(I), Key)) Out.Add(VerbAt(I));
	return Out;
}

EJamVerb FJamBindings::WithVerbOn(const FKey& Key) const
{
	for (int32 I = 0; I < NumVerbs; ++I) if (GVerbs[I].Role == 1 && Has(VerbAt(I), Key)) return VerbAt(I);
	return EJamVerb::Count;
}

EJamVerb FJamBindings::WithoutVerbOn(const FKey& Key) const
{
	for (int32 I = 0; I < NumVerbs; ++I) if (GVerbs[I].Role == 2 && Has(VerbAt(I), Key)) return VerbAt(I);
	return EJamVerb::Count;
}

bool FJamBindings::Rebind(EJamVerb V, const FKey& Key, EJamVerb* OutSwapped)
{
	// Lane A199 (A198's FAULT 2; the Director's ruling): rebinding one verb moves that verb and, at most, the verbs that held the new key and may not
	// share it with it. The rebound verb takes Key in its first slot (its old first key leaves it; a partner that shared the old key - the shot and the
	// cross-check on LMB - keeps it: the pair splits). A verb that held Key and may not share it takes the rebound verb's old key in its place when that
	// key fits it (no clash with the verbs still on it), else it just gives Key up - and if that would leave it with no key and no pad button, the
	// rebinding is refused. Nothing else moves. The table is valid and every verb reachable after any true return.
	if (OutSwapped) *OutSwapped = EJamVerb::Count;
	if (V >= EJamVerb::Count || IsReserved(Key)) return false;
	const bool bPad = Key.IsGamepadKey();
	auto IsSkateVerb = [](EJamVerb U) { return U >= EJamVerb::SkateForward && U <= EJamVerb::SkateRight; };
	// The skate directions are keys only (the left stick skates).
	if (bPad && IsSkateVerb(V)) return false;
	const FJamBindings Was = *this;
	TArray<FKey>& Mine = bPad ? (*this)[V].Pads : (*this)[V].Keys;
	if (Mine.Num() > 0 && Mine[0] == Key) return false;
	const FKey Old = Mine.Num() > 0 ? Mine[0] : FKey();
	Mine.Remove(Key);
	if (Mine.Num() == 0) Mine.Add(Key); else Mine[0] = Key;
	for (int32 I = 0; I < NumVerbs; ++I)
	{
		const EJamVerb U = VerbAt(I);
		if (U == V || MayShare(U, V)) continue;
		TArray<FKey>& Theirs = bPad ? (*this)[U].Pads : (*this)[U].Keys;
		const int32 At = Theirs.IndexOfByKey(Key);
		if (At == INDEX_NONE) continue;
		// Does the old key fit U: not already U's, of the right kind for U, and every verb still on it may share it with U.
		bool bOldFits = Old.IsValid() && !Theirs.Contains(Old) && !(IsSkateVerb(U) && Old.IsGamepadKey());
		for (int32 J = 0; J < NumVerbs && bOldFits; ++J)
		{
			const EJamVerb W = VerbAt(J);
			if (W != U && Has(W, Old) && !MayShare(U, W)) bOldFits = false;
		}
		if (bOldFits) Theirs[At] = Old; else Theirs.RemoveAt(At);
		if (OutSwapped && *OutSwapped == EJamVerb::Count) *OutSwapped = U;
	}
	// Lane A203 (track K1 evolution 4): and a move never leaves another control with no key of the kind moved: a keys-cell move that took a verb's last key
	// (restart's R under the cross-check: it kept only its pad button, A199's rule let it) is refused like the move that leaves it nothing at all; the
	// pads cell alike for its last pad button. A mouse-and-keyboard player never loses a control to a rebinding.
	bool bStrips = false;
	for (int32 I = 0; I < NumVerbs && !bStrips; ++I)
	{
		const EJamVerb U = VerbAt(I);
		if (U == V) continue;
		const TArray<FKey>& Before = bPad ? Was[U].Pads : Was[U].Keys;
		const TArray<FKey>& Now = bPad ? (*this)[U].Pads : (*this)[U].Keys;
		bStrips = Before.Num() > 0 && Now.Num() == 0;
	}
	if (bStrips || !IsValid() || !IsPlayable())
	{
		*this = Was;
		if (OutSwapped) *OutSwapped = EJamVerb::Count;
		return false;
	}
	return true;
}

bool FJamBindings::IsPlayable(FString* OutWhy) const
{
	// Lane A199 (FAULT 1 / 2): every verb has a key or a pad button; a skate direction a key (the left stick is fixed and always skates).
	for (int32 I = 0; I < NumVerbs; ++I)
	{
		const EJamVerb V = VerbAt(I);
		const bool bSkate = V >= EJamVerb::SkateForward && V <= EJamVerb::SkateRight;
		if (Verbs[I].Keys.Num() == 0 && (bSkate || Verbs[I].Pads.Num() == 0))
		{
			if (OutWhy) *OutWhy = FString::Printf(TEXT("%s has no key%s"), GVerbs[I].Id, bSkate ? TEXT("") : TEXT(" and no pad button"));
			return false;
		}
	}
	return true;
}

bool FJamBindings::IsValid(FString* OutWhy) const
{
	for (int32 I = 0; I < NumVerbs; ++I)
	{
		for (const TArray<FKey>* L : { &Verbs[I].Keys, &Verbs[I].Pads })
		{
			for (const FKey& K : *L)
			{
				for (int32 J = I + 1; J < NumVerbs; ++J)
				{
					if (Has(VerbAt(J), K) && !MayShare(VerbAt(I), VerbAt(J)))
					{
						if (OutWhy) *OutWhy = FString::Printf(TEXT("%s is on %s and %s"), *K.ToString(), GVerbs[I].Id, GVerbs[J].Id);
						return false;
					}
				}
			}
		}
	}
	return true;
}

TMap<FString, FString> FJamBindings::ToSaved() const
{
	TMap<FString, FString> Out;
	for (int32 I = 0; I < NumVerbs; ++I)
	{
		TArray<FString> K, P;
		for (const FKey& Key : Verbs[I].Keys) K.Add(Key.GetFName().ToString());
		for (const FKey& Key : Verbs[I].Pads) P.Add(Key.GetFName().ToString());
		Out.Add(GVerbs[I].Id, FString::Join(K, TEXT(",")) + TEXT("|") + FString::Join(P, TEXT(",")));
	}
	return Out;
}

FJamBindings FJamBindings::FromSaved(const TMap<FString, FString>& Saved)
{
	FJamBindings B = Defaults();
	for (int32 I = 0; I < NumVerbs; ++I)
	{
		const FString* S = Saved.Find(GVerbs[I].Id);
		if (!S) continue;
		FString KeysPart, PadsPart;
		if (!S->Split(TEXT("|"), &KeysPart, &PadsPart)) continue;
		auto Parse = [](const FString& Part, bool bPad)
		{
			TArray<FString> Names;
			Part.ParseIntoArray(Names, TEXT(","));
			TArray<FKey> Keys;
			for (const FString& N : Names)
			{
				const FKey K(*N.TrimStartAndEnd());
				if (K.IsValid() && !IsReserved(K) && K.IsGamepadKey() == bPad && !Keys.Contains(K)) Keys.Add(K);
			}
			return Keys;
		};
		TArray<FKey> K = Parse(KeysPart, false), P = Parse(PadsPart, true);
		// Lane A199 (A198's FAULT 1): an entry with no key the game knows (unknown names, reserved keys, empty) keeps the verb's default.
		if (K.Num() == 0 && P.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("JAMCTRL bindings: saved %s=\"%s\" names no usable key or pad button; %s keeps its default"), GVerbs[I].Id, **S, GVerbs[I].Id);
			continue;
		}
		B.Verbs[I].Keys = MoveTemp(K);
		B.Verbs[I].Pads = MoveTemp(P);
	}
	FString Why;
	if (!B.IsValid(&Why) || !B.IsPlayable(&Why))
	{
		UE_LOG(LogTemp, Warning, TEXT("JAMCTRL bindings: the saved table is refused whole (%s); the defaults are used"), *Why);
		return Defaults();
	}
	return B;
}

// ---- the resolver

void FJamButtonResolver::Press(const FKey& Key)
{
	for (FHeldKey& H : Held) if (H.Key == Key) { if (!H.bDown) { H.bDown = true; H.bNew = true; H.Verb = EJamVerb::Count; } return; }
	FHeldKey H;
	H.Key = Key;
	H.bDown = true;
	H.bNew = true;
	Held.Add(H);
}

void FJamButtonResolver::Release(const FKey& Key)
{
	for (FHeldKey& H : Held) if (H.Key == Key) H.bDown = false;
}

EJamVerb FJamButtonResolver::ActiveVerb(const FKey& Key) const
{
	for (const FHeldKey& H : Held) if (H.Key == Key) return H.bDead ? EJamVerb::Count : H.Verb;
	return EJamVerb::Count;
}

FJamButtonVerbs FJamButtonResolver::Step(bool bWithPuck, const FJamBindings& B)
{
	FJamButtonContext C;
	C.bWith = C.bCarrying = C.bInReach = bWithPuck;
	return Step(C, B);
}

FJamButtonVerbs FJamButtonResolver::Step(const FJamButtonContext& C, const FJamBindings& B)
{
	FJamButtonVerbs Out;
	for (FHeldKey& H : Held)
	{
		const bool bNew = H.bNew;
		if (bNew)
		{
			H.bNew = false;
			H.bDead = H.bLevelSent = H.bLevelDropped = false;
			H.Steps = 0;
			const EJamVerb With = B.WithVerbOn(H.Key), Without = B.WithoutVerbOn(H.Key);
			H.Verb = C.bWith ? (With != EJamVerb::Count ? With : Without) : (Without != EJamVerb::Count ? Without : With);
			switch (H.Verb)
			{
			case EJamVerb::Shoot: Out.bShootPress = true; break;
			case EJamVerb::Pass: Out.bPassPress = true; break;
			case EJamVerb::Check: Out.bCheck = true; break;
			case EJamVerb::CrossCheck: Out.bLift = true; break;
			default: break;
			}
		}
		else if (C.bDeadAcrossChange && !H.bDead)
		{
			// Lane A199 (item 1): the change of possession under a held key. A with-verb dies when the puck is no longer ours; the cross-check when he takes
			// the puck (the brace ends, and does not come back if he loses it again under the same press); the check (an edge) when the puck becomes ours.
			switch (H.Verb)
			{
			case EJamVerb::Shoot: case EJamVerb::Pass: H.bDead = !C.bWith; break;
			case EJamVerb::CrossCheck: H.bDead = C.bCarrying; break;
			case EJamVerb::Check: H.bDead = C.bWith; break;
			default: break;
			}
			if (H.bDead && Out.DeadNow == EJamVerb::Count) Out.DeadNow = H.Verb;
		}
		++H.Steps;
		const bool bWithVerb = H.Verb == EJamVerb::Shoot || H.Verb == EJamVerb::Pass;
		bool bLevel = false;
		if (H.bDead)
		{
			// A dead shot / pass: the sim fires on the level's fall when it holds a charge and the puck is in reach, so the level is held up while the puck
			// is still in his reach and falls the first step it is not (a fall with nothing in reach fires nothing and clears the charge).
			if (bWithVerb && H.bDown && H.bLevelSent && !H.bLevelDropped && C.bInReach) bLevel = true;
			else H.bLevelDropped = true;
		}
		else
		{
			switch (H.Verb)
			{
			case EJamVerb::Shoot: bLevel = H.bDown || bNew; break;   // held, or a tap inside one step: one step of the level
			case EJamVerb::Pass:
				// Item 2: the level starts SaucerDelaySteps late (the saucer needs that many more frames held); a press let go before then sends one step: a flat pass.
				bLevel = (H.bDown && H.Steps > C.SaucerDelaySteps) || (!H.bDown && !H.bLevelSent);
				break;
			case EJamVerb::CrossCheck: bLevel = (H.bDown || bNew) && H.Steps > C.BraceDelaySteps; break;   // item 2: the brace from more frames
			default: break;
			}
		}
		if (bLevel)
		{
			H.bLevelSent = true;
			switch (H.Verb)
			{
			case EJamVerb::Shoot: Out.bShootHeld = true; break;
			case EJamVerb::Pass: Out.bPassHeld = true; break;
			case EJamVerb::CrossCheck: Out.bCrossHeld = true; break;
			default: break;
			}
		}
		if (H.bDown && !H.bDead)
		{
			if (H.Verb == EJamVerb::Pass) Out.PassHoldSteps = FMath::Max(Out.PassHoldSteps, H.Steps);
			if (H.Verb == EJamVerb::CrossCheck) Out.CrossHoldSteps = FMath::Max(Out.CrossHoldSteps, H.Steps);
		}
	}
	Held.RemoveAll([](const FHeldKey& H) { return !H.bDown && !H.bNew; });
	return Out;
}

// ---- the sim side

static TAutoConsoleVariable<int32> CVarJamCtrlOurPuckShoots(TEXT("jam.ctrl.OurPuckShoots"), 1,
	TEXT("Lane A183: 1 = the two-role buttons read 'with the puck' as carrying it OR a loose puck last touched by the skater's own team (a pass of ours in flight, so the one-timer's early press is a shot); 0 = carrying only."));

bool JamControls::HasPuckForButtons(const UJamSim& Sim, int32 Skater)
{
	const TArray<FJamSkaterState>& Sk = Sim.GetSkaters();
	if (!Sk.IsValidIndex(Skater)) return false;
	const int32 Carrier = Sim.CarrierIndex();
	if (Carrier == Skater) return true;
	if (Carrier >= 0 || CVarJamCtrlOurPuckShoots.GetValueOnAnyThread() == 0) return false;
	const int32 Last = Sim.GetPuck().LastTouch;
	return Sk.IsValidIndex(Last) && Sk[Last].Team == Sk[Skater].Team;
}

// A184 (the contact lane) adds FJamInput::bCrossHold. Until that field is on this tree the hold is computed, logged and dropped here.
// The switch is JAM_INPUT_HAS_CROSS_HOLD in InputMapping.h (lane A191 moved it there: the acceptance drill's held cross-check row reads it too).

void JamControls::SetCrossHold(FJamInput& In, bool bHeld)
{
#if JAM_INPUT_HAS_CROSS_HOLD
	In.bCrossHold = bHeld;
#else
	(void)In;
	(void)bHeld;
#endif
}

static TAutoConsoleVariable<int32> CVarJamCtrlHeldAcrossChange(TEXT("jam.ctrl.HeldAcrossChange"), 1,
	TEXT("Lane A199: 1 = a button held across a change of possession does nothing until it is released and pressed again (a held cross-check's brace ends when he takes the puck; a held shot or pass dies when the puck is no longer ours); 0 = A183's rule (the verb holds until the release, its level with it)."));
static TAutoConsoleVariable<int32> CVarJamCtrlSaucerHoldFrames(TEXT("jam.ctrl.SaucerHoldFrames"), 13,
	TEXT("Lane A199: frames the pass button must be held for a saucer (a press up to one frame less is a tap: a flat pass). 0 = the cartridge's pass.saucer.half_frames (8). Never fewer than that."));
static TAutoConsoleVariable<int32> CVarJamCtrlBraceHoldFrames(TEXT("jam.ctrl.BraceHoldFrames"), 13,
	TEXT("Lane A199: frames the cross-check button must be held to brace (a shorter press is the pressed cross-check only). 0 = the sim's CrossHoldTapFrames (10). Never fewer than that."));

int32 JamControls::SaucerHoldFrames(int32 SimHalfFrames)
{
	const int32 Want = CVarJamCtrlSaucerHoldFrames.GetValueOnAnyThread();
	return SimHalfFrames <= 0 ? SimHalfFrames : FMath::Max(SimHalfFrames, Want);
}

int32 JamControls::BraceHoldFrames()
{
	return FMath::Max(UJamSim::CrossHoldTapFrames, CVarJamCtrlBraceHoldFrames.GetValueOnAnyThread());
}

FJamButtonContext JamControls::ButtonContext(const UJamSim& Sim, int32 Skater)
{
	FJamButtonContext C;
	C.bWith = HasPuckForButtons(Sim, Skater);
	C.bCarrying = Sim.CarrierIndex() == Skater;
	const TArray<FJamSkaterState>& Sk = Sim.GetSkaters();
	C.bInReach = Sk.IsValidIndex(Skater) && FVector2D::Distance(Sk[Skater].Pos, Sim.GetPuck().Pos) <= Sim.CarrierReachM(Skater);
	C.bDeadAcrossChange = CVarJamCtrlHeldAcrossChange.GetValueOnAnyThread() != 0;
	const int32 Half = Sim.GetConfig().Pass.SaucerHalfFrames;
	C.SaucerDelaySteps = Half > 0 && Sim.GetConfig().Pass.PowerFullFrames <= 0 ? FMath::Max(0, SaucerHoldFrames(Half) - Half) : 0;   // lane A207: under game.pass.power the hold is the power: the level goes at once
	C.BraceDelaySteps = FMath::Max(0, BraceHoldFrames() - UJamSim::CrossHoldTapFrames);
	return C;
}

FJamButtonVerbs JamControls::ApplyButtons(UJamPlayerInput& Pi, const UJamSim& Sim, int32 Skater, FJamInput& In)
{
	const FJamButtonVerbs V = Pi.StepButtons(ButtonContext(Sim, Skater));
	GJamLastHumanButtons = V;
	if (V.DeadNow != EJamVerb::Count)
	{
		UE_LOG(LogTemp, Log, TEXT("JAMCTRL btn=dead verb=%s frame=%lld carrier=%d last_touch=%d"), FJamBindings::VerbId(V.DeadNow), static_cast<long long>(Sim.GetFrame()),
			Sim.CarrierIndex(), Sim.GetPuck().LastTouch);
	}
	In.bShootHeld = V.bShootHeld;
	In.bPassHeld = V.bPassHeld;
	In.bCheck = V.bCheck;
	In.bLift = V.bLift;
	SetCrossHold(In, V.bCrossHeld && Sim.CarrierIndex() != Skater);
	return V;
}

// ---- the context

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
	Deke = MakeAction(this, TEXT("IA_Deke"), EInputActionValueType::Boolean);   // lane A39
	Pass = MakeAction(this, TEXT("IA_Pass"), EInputActionValueType::Boolean);
	Sprint = MakeAction(this, TEXT("IA_Sprint"), EInputActionValueType::Boolean);   // lane A49
	Spin = MakeAction(this, TEXT("IA_Spin"), EInputActionValueType::Boolean);   // lane A51 (R-0034)
	Ability = MakeAction(this, TEXT("IA_Ability"), EInputActionValueType::Boolean);   // lane A84 (R-0041 point 3)
	// Lane A183: Shoot, Lift, Pass and Check are no longer mapped to keys (the button slots carry their keys); they stay as the verbs' names for IsKeyMapped.
	ButtonSlots.SetNum(NumButtonSlots);
	SlotKeys.SetNum(NumButtonSlots);
	for (int32 I = 0; I < NumButtonSlots; ++I) ButtonSlots[I] = MakeAction(this, *FString::Printf(TEXT("IA_Button%02d"), I), EInputActionValueType::Boolean);
	Context = NewObject<UInputMappingContext>(this, TEXT("IMC_Hokej"));
	// The player's saved bindings (the profile's [Jam.Controls]; a test's settings seam when one is set), else the defaults.
	const FJamSettings Saved = FJamSettings::Load();
	Bindings = Saved.Controls.Num() ? FJamBindings::FromSaved(Saved.Controls) : FJamBindings::Defaults();
	MapAll();
}

void UJamInputMap::MapAll()
{
	Context->UnmapAll();
	const FJamBindings& B = Bindings;
	// The skate keys: a 1D key feeds X; forward swizzles it onto Y, back swizzles and negates, left negates.
	auto Skate = [&](EJamVerb V, bool bSwizzle, bool bNegate)
	{
		for (const FKey& K : B[V].Keys)
		{
			FEnhancedActionKeyMapping& M = Context->MapKey(Move, K);
			if (bSwizzle) AddSwizzle(Context, M);
			if (bNegate) AddNegate(Context, M);
		}
	};
	Skate(EJamVerb::SkateForward, true, false);
	Skate(EJamVerb::SkateBack, true, true);
	Skate(EJamVerb::SkateLeft, false, true);
	Skate(EJamVerb::SkateRight, false, false);
	{
		FEnhancedActionKeyMapping& Stick = Context->MapKey(Move, EKeys::Gamepad_Left2D);
		UInputModifierDeadZone* DZ = NewObject<UInputModifierDeadZone>(Context);
		DZ->LowerThreshold = 0.25f;
		Stick.Modifiers.Add(DZ);
	}
	// The plain verbs, one action each.
	const TPair<EJamVerb, UInputAction*> Plain[] = { { EJamVerb::Poke, Poke }, { EJamVerb::Deke, Deke }, { EJamVerb::Spin, Spin }, { EJamVerb::Ability, Ability },
		{ EJamVerb::Sprint, Sprint }, { EJamVerb::Brake, Brake }, { EJamVerb::Restart, Restart } };
	for (const TPair<EJamVerb, UInputAction*>& P : Plain)
	{
		for (const FKey& K : B[P.Key].Keys) Context->MapKey(P.Value, K);
		for (const FKey& K : B[P.Key].Pads) Context->MapKey(P.Value, K);
	}
	// The fixed ones: the stick's aim and the pause.
	Context->MapKey(StickAim, EKeys::Gamepad_Right2D);
	Context->MapKey(StickMouse, EKeys::Mouse2D);
	Context->MapKey(Pause, EKeys::Escape);
	Context->MapKey(Pause, EKeys::Gamepad_Special_Right);       // Start / Menu
	// The two-role verbs: one slot action per distinct key, resolved by possession at the step (FJamButtonResolver).
	for (FKey& K : SlotKeys) K = FKey();
	int32 Used = 0;
	for (const EJamVerb V : { EJamVerb::Shoot, EJamVerb::CrossCheck, EJamVerb::Pass, EJamVerb::Check })
	{
		for (const TArray<FKey>* L : { &B[V].Keys, &B[V].Pads })
		{
			for (const FKey& K : *L)
			{
				if (SlotKeys.Contains(K)) continue;
				if (Used >= NumButtonSlots) { UE_LOG(LogTemp, Warning, TEXT("JAMCTRL bindings: more than %d button keys; %s is not mapped"), NumButtonSlots, *K.ToString()); continue; }
				SlotKeys[Used] = K;
				Context->MapKey(ButtonSlots[Used], K);
				++Used;
			}
		}
	}
}

void UJamInputMap::ApplyBindings(const FJamBindings& In)
{
	FString Why;
	if (!In.IsValid(&Why)) { UE_LOG(LogTemp, Warning, TEXT("JAMCTRL bindings refused: %s"), *Why); return; }
	if (In == Bindings) return;
	Bindings = In;
	MapAll();
	for (TObjectIterator<UEnhancedInputLocalPlayerSubsystem> It; It; ++It)
	{
		if (It->HasAnyFlags(RF_ClassDefaultObject) || !It->GetLocalPlayer()) continue;
		It->RequestRebuildControlMappings();
	}
	UE_LOG(LogTemp, Log, TEXT("JAMCTRL bindings applied: %s"), *BindingTableText(true));
}

// ---- the words

FString UJamInputMap::KeyName(const FKey& Key)
{
	if (!Key.IsValid()) return FString();
	static const TPair<FKey, const TCHAR*> Names[] = {
		{ EKeys::LeftMouseButton, TEXT("LMB") }, { EKeys::RightMouseButton, TEXT("RMB") }, { EKeys::MiddleMouseButton, TEXT("MMB") },
		{ EKeys::ThumbMouseButton, TEXT("MOUSE4") }, { EKeys::ThumbMouseButton2, TEXT("MOUSE5") }, { EKeys::MouseScrollUp, TEXT("WHEEL UP") },
		{ EKeys::MouseScrollDown, TEXT("WHEEL DOWN") }, { EKeys::SpaceBar, TEXT("SPACE") }, { EKeys::LeftControl, TEXT("CTRL") }, { EKeys::RightControl, TEXT("R-CTRL") },
		{ EKeys::LeftShift, TEXT("SHIFT") }, { EKeys::RightShift, TEXT("R-SHIFT") }, { EKeys::LeftAlt, TEXT("ALT") }, { EKeys::RightAlt, TEXT("R-ALT") },
		{ EKeys::Enter, TEXT("ENTER") }, { EKeys::Tab, TEXT("TAB") }, { EKeys::BackSpace, TEXT("BACKSPACE") }, { EKeys::CapsLock, TEXT("CAPS") },
		{ EKeys::Up, TEXT("UP") }, { EKeys::Down, TEXT("DOWN") }, { EKeys::Left, TEXT("LEFT") }, { EKeys::Right, TEXT("RIGHT") },
		{ EKeys::Gamepad_RightTriggerAxis, TEXT("RT") }, { EKeys::Gamepad_LeftTriggerAxis, TEXT("LT") }, { EKeys::Gamepad_RightShoulder, TEXT("RB") },
		{ EKeys::Gamepad_LeftShoulder, TEXT("LB") }, { EKeys::Gamepad_FaceButton_Bottom, TEXT("A") }, { EKeys::Gamepad_FaceButton_Right, TEXT("B") },
		{ EKeys::Gamepad_FaceButton_Left, TEXT("X") }, { EKeys::Gamepad_FaceButton_Top, TEXT("Y") }, { EKeys::Gamepad_LeftThumbstick, TEXT("L3") },
		{ EKeys::Gamepad_RightThumbstick, TEXT("R3") }, { EKeys::Gamepad_Special_Left, TEXT("VIEW") }, { EKeys::Gamepad_Special_Right, TEXT("MENU") },
		{ EKeys::Escape, TEXT("ESC") } };
	for (const TPair<FKey, const TCHAR*>& N : Names) if (N.Key == Key) return N.Value;
	FString S = Key.GetDisplayName(false).ToString().ToUpper();
	if (S.IsEmpty()) S = Key.GetFName().ToString().ToUpper();
	return S.Left(KeyNameMax);
}

FString UJamInputMap::VerbKeysText(EJamVerb V, bool bPad)
{
	const FJamVerbBinding& Bi = Get()->GetBindings()[V];
	const TArray<FKey>& L = bPad ? Bi.Pads : Bi.Keys;
	TArray<FString> Parts;
	for (const FKey& K : L)
	{
		FString N = KeyName(K);
		if (K == EKeys::MouseScrollUp || K == EKeys::MouseScrollDown) N = TEXT("WHEEL");
		Parts.AddUnique(N);
	}
	return FString::Join(Parts, TEXT(", "));
}

namespace
{
	FString FirstKey(EJamVerb V, bool bPad = false)
	{
		const FJamVerbBinding& Bi = UJamInputMap::Get()->GetBindings()[V];
		const TArray<FKey>& L = bPad ? Bi.Pads : Bi.Keys;
		if (L.Num() == 0) return TEXT("-");
		if (L[0] == EKeys::MouseScrollUp || L[0] == EKeys::MouseScrollDown) return TEXT("WHEEL");
		return UJamInputMap::KeyName(L[0]);
	}

	// "WASD" when the four skate keys are single letters, else "W/A/S/D" with their names.
	FString SkateKeys()
	{
		const FString F = FirstKey(EJamVerb::SkateForward), L = FirstKey(EJamVerb::SkateLeft), B = FirstKey(EJamVerb::SkateBack), R = FirstKey(EJamVerb::SkateRight);
		if (F.Len() == 1 && L.Len() == 1 && B.Len() == 1 && R.Len() == 1) return F + L + B + R;
		return FString::Printf(TEXT("%s/%s/%s/%s"), *F, *L, *B, *R);
	}

	// The two-role pair as one entry when both roles start on the same key ("SHOOT / CROSS-CHECK  LMB"), else two entries.
	void Pair(TArray<FString>& Out, EJamVerb With, EJamVerb Without)
	{
		const FString A = FirstKey(With), B = FirstKey(Without);
		if (A == B) Out.Add(FString::Printf(TEXT("%s / %s  %s"), *FJamBindings::VerbLabel(With), *FJamBindings::VerbLabel(Without), *A));
		else { Out.Add(FString::Printf(TEXT("%s  %s"), *FJamBindings::VerbLabel(With), *A)); Out.Add(FString::Printf(TEXT("%s  %s"), *FJamBindings::VerbLabel(Without), *B)); }
	}
}

void UJamInputMap::NoteInputDevice(bool bPad) { GJamPadLast = bPad; }
bool UJamInputMap::IsPadLast() { return GJamPadLast; }
const FJamButtonVerbs& JamControls::LastHumanButtons() { return GJamLastHumanButtons; }

FString UJamInputMap::HintStripText()
{
	return HintStripTextFor(IsPadLast());
}

FString UJamInputMap::HintStripTextFor(bool bPad)
{
	if (bPad)
	{
		// Lane A199 (item 3): the pad's strip, from the pad bindings (the sticks are fixed).
		auto PadPair = [](TArray<FString>& Out, EJamVerb With, EJamVerb Without)
		{
			const FString A = FirstKey(With, true), B = FirstKey(Without, true);
			if (A == B) Out.Add(FString::Printf(TEXT("%s / %s  %s"), *FJamBindings::VerbLabel(With), *FJamBindings::VerbLabel(Without), *A));
			else { Out.Add(FString::Printf(TEXT("%s  %s"), *FJamBindings::VerbLabel(With), *A)); Out.Add(FString::Printf(TEXT("%s  %s"), *FJamBindings::VerbLabel(Without), *B)); }
		};
		TArray<FString> P;
		P.Add(TEXT("SKATE  L-STICK"));
		P.Add(TEXT("BRAKE  STICK BACK"));
		P.Add(TEXT("AIM  R-STICK"));
		PadPair(P, EJamVerb::Shoot, EJamVerb::CrossCheck);
		PadPair(P, EJamVerb::Pass, EJamVerb::Check);
		P.Add(FString::Printf(TEXT("POKE  %s"), *FirstKey(EJamVerb::Poke, true)));
		P.Add(FString::Printf(TEXT("ABILITY  %s"), *FirstKey(EJamVerb::Ability, true)));
		P.Add(FString::Printf(TEXT("SPRINT  %s"), *FirstKey(EJamVerb::Sprint, true)));
		FString Out = FString::Join(P, TEXT("   "));
		while (Out.Len() > HintStripMaxChars && P.Num() > 1) { P.Pop(); Out = FString::Join(P, TEXT("   ")); }
		return Out.Left(HintStripMaxChars);
	}
	// Lane A111's seven verbs and the brake, now with the two-role buttons (A183). Entries leave from the end while the line is longer than the strip
	// holds (HintStripMaxChars, the length the 1280 x 720 strip fits), so a long rebound key name never runs under the radar.
	TArray<FString> E;
	E.Add(FString::Printf(TEXT("SKATE  %s"), *SkateKeys()));
	E.Add(FString::Printf(TEXT("BRAKE  %s AGAINST YOUR MOTION"), *FirstKey(EJamVerb::SkateBack)));
	E.Add(TEXT("AIM  MOUSE"));
	Pair(E, EJamVerb::Shoot, EJamVerb::CrossCheck);
	Pair(E, EJamVerb::Pass, EJamVerb::Check);
	E.Add(FString::Printf(TEXT("POKE  %s"), *FirstKey(EJamVerb::Poke)));
	E.Add(FString::Printf(TEXT("ABILITY  %s"), *FirstKey(EJamVerb::Ability)));
	E.Add(FString::Printf(TEXT("SPRINT  %s"), *FirstKey(EJamVerb::Sprint)));
	FString Out = FString::Join(E, TEXT("   "));
	while (Out.Len() > HintStripMaxChars && E.Num() > 1) { E.Pop(); Out = FString::Join(E, TEXT("   ")); }
	return Out.Left(HintStripMaxChars);
}

// Lane A213 (track U3, the night of 14 to 15 Nov): the words say tonight's game. A hold is the pass's power (game.pass.power, lane A207: no saucer to
// learn), the held cross-check is the brace (13 frames, lane A199), the body check takes him onto a man in front (lane A209). jam.hud.Words 0 = yesterday's words.
static TAutoConsoleVariable<int32> CVarJamHudWords(TEXT("jam.hud.Words"), 1,
	TEXT("Lane A213: 1 (default) the pause card, the CONTROLS screen, the binding table and the badge say tonight's rules (RMB: pass, hold for power; LMB: shoot, hold to lift / cross-check, hold to brace); 0 = the words of before (hold = saucer, hold to hit)."), ECVF_Default);
bool UJamInputMap::WordsTonight() { return CVarJamHudWords.GetValueOnAnyThread() != 0; }

FString UJamInputMap::PauseCardText()
{
	// Lane A144's five lines (the card's inner width, about 80 characters), from the bindings: first keys only (the CONTROLS screen lists the second ones).
	auto Short = [](EJamVerb V) { const FString P = FirstKey(V, true); return P == TEXT("-") ? FirstKey(V) : FString::Printf(TEXT("%s / %s"), *FirstKey(V), *P); };
	auto PairLine = [&](EJamVerb With, EJamVerb Without, const TCHAR* Shared, const TCHAR* WithNote, const TCHAR* WithoutNote)
	{
		if (Short(With) == Short(Without)) return FString::Printf(TEXT("%s / %s  %s  %s"), *FJamBindings::VerbLabel(With), *FJamBindings::VerbLabel(Without), *Short(With), Shared);
		return FString::Printf(TEXT("%s  %s  %s      %s  %s  %s"), *FJamBindings::VerbLabel(With), *Short(With), WithNote, *FJamBindings::VerbLabel(Without), *Short(Without), WithoutNote);
	};
	TArray<FString> L;
	L.Add(FString::Printf(TEXT("SKATE  %s / L-STICK      AIM  MOUSE / R-STICK      SPRINT  %s"), *SkateKeys(), *Short(EJamVerb::Sprint)));
	if (WordsTonight())
	{
		L.Add(PairLine(EJamVerb::Shoot, EJamVerb::CrossCheck, TEXT("(HOLD: LIFT THE SHOT / BRACE)"), TEXT("(HOLD TO LIFT)"), TEXT("(HOLD TO BRACE)")));
		L.Add(PairLine(EJamVerb::Pass, EJamVerb::Check, TEXT("(HOLD FOR POWER)"), TEXT("(HOLD FOR POWER)"), TEXT("")) + TEXT("      PAUSE  ESC / MENU"));
	}
	else
	{
		L.Add(PairLine(EJamVerb::Shoot, EJamVerb::CrossCheck, TEXT("(WITH / WITHOUT THE PUCK: HOLD TO HIT)"), TEXT(""), TEXT("(HOLD TO HIT)")));
		L.Add(PairLine(EJamVerb::Pass, EJamVerb::Check, TEXT("(WITH / WITHOUT THE PUCK)"), TEXT("(HOLD = SAUCER)"), TEXT("")) + TEXT("      PAUSE  ESC / MENU"));
	}
	L.Add(FString::Printf(TEXT("POKE  %s      DEKE  %s      SPIN  %s      ABILITY  %s"), *Short(EJamVerb::Poke), *Short(EJamVerb::Deke), *Short(EJamVerb::Spin), *Short(EJamVerb::Ability)));
	L.Add(FString::Printf(TEXT("BRAKE  %s BACK OR %s    SIGNALS  1-4 / D-PAD    RESTART  %s"), *FirstKey(EJamVerb::SkateBack), *Short(EJamVerb::Brake), *Short(EJamVerb::Restart)));
	for (FString& Line : L) Line.TrimEndInline();
	return FString::Join(L, TEXT("\n"));
}

TArray<FJamControlRow> UJamInputMap::ControlRowsFromBindings()
{
	const FJamBindings& B = Get()->GetBindings();
	auto Prose = [](const FKey& K) -> FString
	{
		if (K == EKeys::LeftMouseButton) return TEXT("left mouse button");
		if (K == EKeys::RightMouseButton) return TEXT("right mouse button");
		if (K == EKeys::MiddleMouseButton) return TEXT("middle mouse");
		if (K == EKeys::ThumbMouseButton) return TEXT("mouse 4");
		if (K == EKeys::ThumbMouseButton2) return TEXT("mouse 5");
		if (K == EKeys::MouseScrollUp || K == EKeys::MouseScrollDown) return TEXT("mouse wheel");
		if (K == EKeys::LeftShift) return TEXT("Shift");
		if (K == EKeys::LeftControl) return TEXT("Ctrl");
		if (K == EKeys::SpaceBar) return TEXT("Space");
		if (K == EKeys::Gamepad_RightThumbstick) return TEXT("R3 (stick click)");
		if (K == EKeys::Gamepad_LeftThumbstick) return TEXT("L3");
		if (K == EKeys::Gamepad_Special_Left) return TEXT("View");
		if (K.IsGamepadKey()) return UJamInputMap::KeyName(K);
		return K.GetDisplayName(false).ToString();
	};
	auto Join = [&](const TArray<FKey>& L)
	{
		TArray<FString> P;
		for (const FKey& K : L) P.AddUnique(Prose(K));
		return P.Num() ? FString::Join(P, TEXT(" or ")) : FString(TEXT("-"));
	};
	auto All = [&](EJamVerb V) { TArray<FKey> K = B[V].Keys; K.Append(B[V].Pads); return K; };
	auto Row = [&](const TCHAR* Verb, EJamVerb V, const TCHAR* Note) { return FJamControlRow{ Verb, Join(B[V].Keys), Join(B[V].Pads), Note, All(V) }; };
	TArray<FJamControlRow> Rows;
	{
		TArray<FKey> Skate;
		for (const EJamVerb V : { EJamVerb::SkateForward, EJamVerb::SkateLeft, EJamVerb::SkateBack, EJamVerb::SkateRight }) Skate.Append(B[V].Keys);
		TArray<FString> First;
		for (const EJamVerb V : { EJamVerb::SkateForward, EJamVerb::SkateLeft, EJamVerb::SkateBack, EJamVerb::SkateRight }) First.Add(B[V].Keys.Num() ? Prose(B[V].Keys[0]) : FString(TEXT("-")));
		const bool bArrows = B[EJamVerb::SkateForward].Keys.Contains(EKeys::Up);
		TArray<FKey> Bound = Skate;
		Bound.Add(EKeys::Gamepad_Left2D);
		Rows.Add({ TEXT("SKATE"), FString::Join(First, TEXT(" ")) + (bArrows ? TEXT(" or arrows") : TEXT("")), TEXT("left stick"), TEXT("you glide: speed is built, not given"), Bound });
	}
	Rows.Add({ TEXT("AIM THE STICK"), TEXT("the mouse"), TEXT("right stick"), TEXT("the blade follows the cursor"), { EKeys::Mouse2D, EKeys::Gamepad_Right2D } });
	const bool bTonight = WordsTonight();   // lane A213
	Rows.Add(Row(TEXT("SHOOT (WITH THE PUCK)"), EJamVerb::Shoot, bTonight ? TEXT("a tap is a flat shot; hold to lift it, let go to fire") : TEXT("a tap is a flat shot; hold to load it, let go to fire")));
	Rows.Add(Row(TEXT("CROSS-CHECK (WITHOUT)"), EJamVerb::CrossCheck, bTonight ? TEXT("shoves the man in front off the puck; hold it to brace") : TEXT("hold it and skate into a man: your speed is the hit")));
	Rows.Add(Row(TEXT("PASS (WITH THE PUCK)"), EJamVerb::Pass, bTonight ? TEXT("to the mate your stick points at; hold for power") : TEXT("a tap is a flat pass; held, it is a saucer over the sticks")));
	Rows.Add(Row(TEXT("CHECK (WITHOUT)"), EJamVerb::Check, bTonight ? TEXT("the body check: it takes you onto a man in front") : TEXT("the body check: come in fast or it is a bump")));
	Rows.Add(Row(TEXT("ABILITY"), EJamVerb::Ability, TEXT("the mode's trick or a pickup's, when the slot is lit")));
	Rows.Add(Row(TEXT("SPIN"), EJamVerb::Spin, TEXT("spin off a man who is on you")));
	Rows.Add(Row(TEXT("DEKE"), EJamVerb::Deke, TEXT("the carrier's fake")));
	Rows.Add(Row(TEXT("SPRINT"), EJamVerb::Sprint, TEXT("while the legs last")));
	{
		FJamControlRow Br = Row(TEXT("BRAKE"), EJamVerb::Brake, TEXT("stop hard, then go: it beats a man who charges"));
		Br.Keys = FString::Printf(TEXT("%s against your motion, or %s"), B[EJamVerb::SkateBack].Keys.Num() ? *Prose(B[EJamVerb::SkateBack].Keys[0]) : TEXT("back"), *Br.Keys);
		Br.Pad = FString::Printf(TEXT("stick back, or %s"), *Br.Pad);
		Rows.Add(Br);
	}
	Rows.Add(Row(TEXT("POKE"), EJamVerb::Poke, TEXT("jab the puck loose")));
	Rows.Add({ TEXT("SIGNALS"), TEXT("1 2 3 4"), TEXT("D-pad"), TEXT("a call to your mates"), {} });
	Rows.Add(Row(TEXT("RESTART"), EJamVerb::Restart, TEXT("the match again")));
	Rows.Add({ TEXT("PAUSE"), TEXT("Esc"), TEXT("Start"), TEXT("the pause card: resume, a rule change, controls"), { EKeys::Escape, EKeys::Gamepad_Special_Right } });
	return Rows;
}

FString UJamInputMap::BindingTableText(bool bOneLine)
{
	// Lane A183: from the bindings (lane A49 kept it by hand next to Build()). One verb a row, the verb's name before the double space.
	TArray<TPair<FString, FString>> Rows;
	auto KP = [](EJamVerb V) { const FString P = VerbKeysText(V, true); return P.IsEmpty() ? VerbKeysText(V, false) : VerbKeysText(V, false) + TEXT(" / ") + P; };
	Rows.Add({ TEXT("SKATE"), SkateKeys() + TEXT(" / L-STICK") });
	Rows.Add({ TEXT("AIM"), TEXT("MOUSE / R-STICK") });
	Rows.Add({ TEXT("SHOOT"), KP(EJamVerb::Shoot) + TEXT(" (WITH THE PUCK; HOLD TO LIFT)") });
	Rows.Add({ TEXT("CROSS-CHECK"), KP(EJamVerb::CrossCheck) + (WordsTonight() ? TEXT(" (WITHOUT THE PUCK; HOLD TO BRACE)") : TEXT(" (WITHOUT THE PUCK; HOLD IT INTO A MAN)")) });
	Rows.Add({ TEXT("PASS"), KP(EJamVerb::Pass) + (WordsTonight() ? TEXT(" (WITH THE PUCK; HOLD FOR POWER)") : TEXT(" (WITH THE PUCK; HOLD = SAUCER)")) });
	Rows.Add({ TEXT("CHECK"), KP(EJamVerb::Check) + TEXT(" (WITHOUT THE PUCK)") });
	Rows.Add({ TEXT("DEKE"), KP(EJamVerb::Deke) });
	Rows.Add({ TEXT("SPIN"), KP(EJamVerb::Spin) });
	Rows.Add({ TEXT("SPRINT"), KP(EJamVerb::Sprint) });
	Rows.Add({ TEXT("POKE"), KP(EJamVerb::Poke) });
	Rows.Add({ TEXT("ABILITY"), KP(EJamVerb::Ability) });
	Rows.Add({ TEXT("SIGNALS"), TEXT("1-4 / D-PAD") });
	Rows.Add({ TEXT("BRAKE"), FString::Printf(TEXT("%s AGAINST YOUR MOTION / %s"), *FirstKey(EJamVerb::SkateBack), *KP(EJamVerb::Brake)) });
	Rows.Add({ TEXT("RESTART"), KP(EJamVerb::Restart) });
	Rows.Add({ TEXT("PAUSE"), TEXT("ESC / MENU") });
	FString Out;
	for (const TPair<FString, FString>& R : Rows)
	{
		if (!Out.IsEmpty()) Out += bOneLine ? TEXT("      ") : TEXT("\n");
		Out += FString::Printf(TEXT("%s  %s"), *R.Key, *R.Value);
	}
	return Out;
}

bool UJamInputMap::IsKeyMapped(const UInputAction* Action, const FKey& Key) const
{
	// Lane A183: the two-role verbs answer from the bindings (their keys sit on the button slots).
	if (Action == Shoot) return Bindings.Has(EJamVerb::Shoot, Key) && SlotKeys.Contains(Key);
	if (Action == Lift) return Bindings.Has(EJamVerb::CrossCheck, Key) && SlotKeys.Contains(Key);
	if (Action == Pass) return Bindings.Has(EJamVerb::Pass, Key) && SlotKeys.Contains(Key);
	if (Action == Check) return Bindings.Has(EJamVerb::Check, Key) && SlotKeys.Contains(Key);
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
	// Lane A183: the two-role buttons (shoot / cross-check, pass / check) arrive per key on the slot actions; StepButtons resolves them by possession.
	for (int32 I = 0; I < UJamInputMap::NumButtonSlots; ++I)
	{
		EIC->BindAction(M->ButtonSlots[I], ETriggerEvent::Started, this, &UJamPlayerInput::OnSlotStarted, I);
		EIC->BindAction(M->ButtonSlots[I], ETriggerEvent::Completed, this, &UJamPlayerInput::OnSlotDone, I);
	}
	EIC->BindAction(M->Brake, ETriggerEvent::Started, this, &UJamPlayerInput::OnBrakeStarted);
	EIC->BindAction(M->Brake, ETriggerEvent::Completed, this, &UJamPlayerInput::OnBrakeDone);
	EIC->BindAction(M->Sprint, ETriggerEvent::Started, this, &UJamPlayerInput::OnSprintStarted);
	EIC->BindAction(M->Sprint, ETriggerEvent::Completed, this, &UJamPlayerInput::OnSprintDone);
	EIC->BindAction(M->StickAim, ETriggerEvent::Triggered, this, &UJamPlayerInput::OnStickAim);
	EIC->BindAction(M->StickAim, ETriggerEvent::Completed, this, &UJamPlayerInput::OnStickAimDone);
	EIC->BindAction(M->StickAim, ETriggerEvent::Canceled, this, &UJamPlayerInput::OnStickAimDone);
	EIC->BindAction(M->StickMouse, ETriggerEvent::Triggered, this, &UJamPlayerInput::OnStickMouse);
	EIC->BindAction(M->Poke, ETriggerEvent::Started, this, &UJamPlayerInput::OnPokeStarted);
	EIC->BindAction(M->Deke, ETriggerEvent::Started, this, &UJamPlayerInput::OnDekeStarted);
	EIC->BindAction(M->Spin, ETriggerEvent::Started, this, &UJamPlayerInput::OnSpinStarted);   // lane A51
	EIC->BindAction(M->Ability, ETriggerEvent::Started, this, &UJamPlayerInput::OnAbilityStarted);   // lane A84
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
	if (StickAxis.SizeSquared() > 0.09) UJamInputMap::NoteInputDevice(true);   // lane A199
}

void UJamPlayerInput::OnStickAimDone(const FInputActionValue&)
{
	StickAxis = FVector2D::ZeroVector;
}

void UJamPlayerInput::OnStickMouse(const FInputActionValue& V)
{
	MouseAimPx += FVector2D(V.Get<FVector2D>());
	if (FVector2D(V.Get<FVector2D>()).SizeSquared() > 4.0) UJamInputMap::NoteInputDevice(false);   // lane A199: a real mouse move, not a jitter
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

bool UJamPlayerInput::ConsumeAbilityPressed()
{
	const bool B = bAbilityLatch;
	bAbilityLatch = false;
	return B;
}

bool UJamPlayerInput::ConsumeSpinPressed()
{
	const bool B = bSpinLatch;
	bSpinLatch = false;
	return B;
}

bool UJamPlayerInput::ConsumeLiftPressed()
{
	const bool B = bLiftLatch;
	bLiftLatch = false;
	return B;
}

bool UJamPlayerInput::ConsumeDekePressed()
{
	const bool B = bDekeLatch;
	bDekeLatch = false;
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

// ---------------------------------------------------------------- lane A183: the two-role buttons

void UJamPlayerInput::OnSlotStarted(int32 Slot)
{
	const UJamInputMap* M = UJamInputMap::Get();
	if (!M->SlotKeys.IsValidIndex(Slot) || !M->SlotKeys[Slot].IsValid()) return;
	DebugButton(M->SlotKeys[Slot], true);
}

void UJamPlayerInput::OnSlotDone(int32 Slot)
{
	const UJamInputMap* M = UJamInputMap::Get();
	if (!M->SlotKeys.IsValidIndex(Slot) || !M->SlotKeys[Slot].IsValid()) return;
	DebugButton(M->SlotKeys[Slot], false);
}

void UJamPlayerInput::DebugButton(const FKey& Key, bool bDown)
{
	if (bDown)
	{
		UJamInputMap::NoteInputDevice(Key.IsGamepadKey());   // lane A199
		Buttons.Press(Key);
		// The edges of before for the callers that do not step the buttons (the camera proof scene): a key on shoot / check latches them.
		const FJamBindings& B = UJamInputMap::Get()->GetBindings();
		if (B.Has(EJamVerb::Shoot, Key)) bShootLatch = true;
		if (B.Has(EJamVerb::Check, Key)) bCheckLatch = true;
	}
	else Buttons.Release(Key);
}

FJamButtonVerbs UJamPlayerInput::StepButtons(bool bWithPuck)
{
	FJamButtonContext C;
	C.bWith = C.bCarrying = C.bInReach = bWithPuck;
	return StepButtons(C);
}

FJamButtonVerbs UJamPlayerInput::StepButtons(const FJamButtonContext& C)
{
	const FJamButtonVerbs V = Buttons.Step(C, UJamInputMap::Get()->GetBindings());
	LastButtons = V;
	bShoot = V.bShootHeld;
	bPassHeld = V.bPassHeld;
	bCheck = V.bCheck;
	bShootLatch = bCheckLatch = bLiftLatch = bPassLatch = false;   // the step has spoken for this frame's presses
	return V;
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

// Lane A183: the HUD's first hint strip takes its words from the live bindings. Defined here so the HUD's own file (other lanes edit it today) is not touched.
FString UJamHudWidget::ControlsHintText()
{
	return UJamInputMap::HintStripText();
}

// ---------------------------------------------------------------- lane A191: a still hand keeps its point on the ice

static TAutoConsoleVariable<int32> CVarJamCtrlAimAnchor(TEXT("jam.ctrl.AimAnchor"), 1,
	TEXT("Lane A191: 1 = while the cursor's pixel is still and the skater stands, the blade keeps its offset from him (a camera correction no longer swings it); 0 = the cursor re-projected every sim step (the aim of before)."));
static TAutoConsoleVariable<float> CVarJamCtrlAimAnchorSpeed(TEXT("jam.ctrl.AimAnchorSpeed"), 0.3f,
	TEXT("Lane A191: the skater's speed (m/s) under which a still cursor anchors (measured: 0.3 drifts least from the cursor's true point; 1 and 2 drift up to 64 deg)."));

FVector2D FJamAimAnchor::Step(const FVector2D& PixelPx, const FVector2D& HitM, const FVector2D& SkaterM, bool bOn, double StandSpeedMps)
{
	const double Speed = bHave ? FVector2D::Distance(SkaterM, LastSkater) * 60.0 : 1e9;   // one call a sim step (60 Hz); a faceoff's teleport re-projects
	const bool bPixelMoved = !bHave || FVector2D::Distance(PixelPx, LastPx) >= 0.5;
	if (!bOn || bPixelMoved || Speed >= StandSpeedMps)
	{
		Offset = HitM - SkaterM;
		bAnchored = false;
		LastPx = PixelPx;   // the reference pixel moves only when the anchor lets go: a slow drift of the hand adds up to half a pixel, then re-projects
	}
	else bAnchored = true;
	LastSkater = SkaterM;
	bHave = true;
	return SkaterM + Offset;
}

FVector2D FJamAimAnchor::StepLive(const FVector2D& PixelPx, const FVector2D& HitM, const FVector2D& SkaterM)
{
	return Step(PixelPx, HitM, SkaterM, CVarJamCtrlAimAnchor.GetValueOnGameThread() != 0, CVarJamCtrlAimAnchorSpeed.GetValueOnGameThread());
}

// ---------------------------------------------------------------- lane A191: what the buttons do right now (the HUD's badge)

static TAutoConsoleVariable<int32> CVarJamHudButtonsBadge(TEXT("jam.hud.ButtonsBadge"), 1, TEXT("Lane A191: 1 = the bottom-left pill says what LMB / RMB do right now (SHOOT or CROSS-CHECK, PASS or CHECK); 0 = hidden."));

FString UJamHudWidget::ButtonsBadgeLine(bool bWithPuck)
{
	const FJamBindings& B = UJamInputMap::Get()->GetBindings();
	TArray<FString> Parts;
	TArray<FKey> Seen;
	const bool bPad = UJamInputMap::IsPadLast();   // lane A199 (item 3): the pad's buttons when the pad was touched last
	for (const EJamVerb V : { EJamVerb::Shoot, EJamVerb::Pass })
	{
		const TArray<FKey>& L = bPad ? B[V].Pads : B[V].Keys;
		if (L.Num() == 0 || Seen.Contains(L[0])) continue;
		const FKey K = L[0];
		Seen.Add(K);
		const EJamVerb With = B.WithVerbOn(K), Without = B.WithoutVerbOn(K);
		const EJamVerb Now = bWithPuck ? (With != EJamVerb::Count ? With : Without) : (Without != EJamVerb::Count ? Without : With);
		if (Now != EJamVerb::Count) Parts.Add(FString::Printf(TEXT("%s  %s"), *UJamInputMap::KeyName(K), *FJamBindings::VerbLabel(Now)));
	}
	return FString::Join(Parts, TEXT("     "));
}

FString UJamHudWidget::GetButtonsBadgeShown() const
{
	return ButtonsBadgeText && ButtonsBadge && ButtonsBadge->GetVisibility() != ESlateVisibility::Collapsed ? ButtonsBadgeText->GetText().ToString() : FString();
}

void UJamHudWidget::BuildButtonsBadge()
{
	using S = FJamUiStyle;
	if (!WidgetTree || !Root) return;
	ButtonsBadge = S::Plate(WidgetTree, S::Pill(0.72f, 0.f), FMargin(14, 5));
	ButtonsBadge->SetVisibility(ESlateVisibility::Collapsed);
	ButtonsBadgeText = S::Text(WidgetTree, TEXT(""), S::Font(S::Body - 2, FName("Bold"), 20), S::Paper());
	ButtonsBadge->SetContent(ButtonsBadgeText);
	UCanvasPanelSlot* BadgeSlot = Root->AddChildToCanvas(ButtonsBadge);
	// Bottom left under the hint strip (whose foot stands 96 above the bottom edge), clear of the commentary strip (centre, 880 wide) and the stamina bar.
	BadgeSlot->SetAnchors(FAnchors(0, 1, 0, 1));
	BadgeSlot->SetAlignment(FVector2D(0, 1));
	BadgeSlot->SetAutoSize(true);
	BadgeSlot->SetPosition(FVector2D(S::EdgePx, -S::EdgePx));
}

void UJamHudWidget::UpdateButtonsBadge(const FJamPresentationFrame& F)
{
	if (!ButtonsBadge || !ButtonsBadgeText) return;
	const AHokejGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AHokejGameMode>() : nullptr;
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	// Lane A222: the pill is the screen of before's (jam.hud.ControlsPanel 0); the panel replaced it, and SETTINGS' OFF takes it away too.
	if (CVarJamHudButtonsBadge.GetValueOnGameThread() == 0 || UJamHudWidget::LegendNow() != UJamHudWidget::ELegend::StripAndPill || !Sim || !Sim->GetSkaters().IsValidIndex(F.HumanIndex))
	{
		ButtonsBadge->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	// The same possession the buttons read at the sim step (FJamButtonResolver), on the frame this HUD shows.
	// Lane A199 (item 2): the hold shown: SAUCER / BRACED from the frame the hold counts.
	// Lane A213: under the pass's power (game.pass.power.full_frames > 0) a held pass is a harder pass, not a saucer: the badge never says SAUCER then.
	const int32 SaucerWord = UJamInputMap::WordsTonight() && Sim->PassPowerOn() ? 0 : JamControls::SaucerHoldFrames(Sim->GetConfig().Pass.SaucerHalfFrames);
	const FString Line = JamControls::BadgeLineHeld(JamControls::HasPuckForButtons(*Sim, F.HumanIndex), JamControls::LastHumanButtons(),
		SaucerWord, JamControls::BraceHoldFrames());
	if (ButtonsBadgeText->GetText().ToString() != Line) ButtonsBadgeText->SetText(FText::FromString(Line));
	ButtonsBadge->SetVisibility(Line.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	// Lane A213 (EYE 12, A211's list): while he lies on the ice the buttons do nothing: the pill dims (it read SHOOT / PASS with the puck loose beside him).
	ButtonsBadge->SetRenderOpacity(UJamInputMap::WordsTonight() && bHumanDownNow ? BadgeDownOpacity : 1.f);
}

// ---------------------------------------------------------------- lane A199 (item 4): the profile written whole or not at all

#include "Misc/CoreDelegates.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

namespace
{
	bool GJamFailNextAtomicSave = false;
}

void JamControls::FailNextAtomicSaveForTest()
{
	GJamFailNextAtomicSave = true;
}

bool JamControls::SaveFileAtomically(const FString& Path, const FString& Contents)
{
	const FString Full = FPaths::ConvertRelativePathToFull(Path);
	const FString Dir = FPaths::GetPath(Full);
	IFileManager::Get().MakeDirectory(*Dir, true);
	// The temp file beside the target (the same volume, so the rename is one step), never the target itself.
	const FString Temp = FPaths::CreateTempFilename(*Dir, *(FPaths::GetBaseFilename(Full).Left(32) + TEXT("-")), TEXT(".tmp"));
	if (!FFileHelper::SaveStringToFile(Contents, *Temp, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		IFileManager::Get().Delete(*Temp, false, false, true);
		return false;
	}
	if (GJamFailNextAtomicSave)
	{
		// The test's kill between the write and the rename: the target is untouched (the temp file is what a real kill would leave; removed here).
		GJamFailNextAtomicSave = false;
		IFileManager::Get().Delete(*Temp, false, false, true);
		return false;
	}
#if PLATFORM_WINDOWS
	// MoveFileEx with REPLACE_EXISTING renames over the old file in one step (IFileManager::Move deletes the old file first: a kill there leaves none).
	FString From = Temp, To = Full;
	FPaths::MakePlatformFilename(From);
	FPaths::MakePlatformFilename(To);
	const bool bMoved = !!::MoveFileExW(*From, *To, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
#else
	const bool bMoved = IFileManager::Get().Move(*Full, *Temp, true, false, false, true);
#endif
	if (!bMoved) IFileManager::Get().Delete(*Temp, false, false, true);
	return bMoved;
}

void JamControls::InstallAtomicSave()
{
	static bool bInstalled = false;
	if (bInstalled) return;
	bInstalled = true;
	// The engine writes a packaged game's ini in place (FConfigFile::WriteTempFileThenMove is false outside the editor). Its pre-save hook lets a saver
	// take the write (ini.SkipSaveIfPreSaveSucceeded, 1 by default, then skips its own); a failed atomic write leaves SavedCount alone and the engine writes as before.
	FCoreDelegates::TSPreSaveConfigFileDelegate().AddLambda([](const TCHAR* IniFilename, const FString& ContentsToSave, int32& SavedCount)
	{
		if (!IniFilename || GGameUserSettingsIni.IsEmpty()) return;
		if (FPaths::ConvertRelativePathToFull(IniFilename) != FPaths::ConvertRelativePathToFull(GGameUserSettingsIni)) return;
		if (JamControls::SaveFileAtomically(IniFilename, ContentsToSave)) ++SavedCount;
	});
}

namespace
{
	// Installed when the module loads, before any menu saves the profile.
	struct FJamAtomicSaveInstaller { FJamAtomicSaveInstaller() { JamControls::InstallAtomicSave(); } } GJamAtomicSaveInstaller;
}

// ---------------------------------------------------------------- lane A199 (item 2): the badge shows the hold

FString JamControls::BadgeLineHeld(bool bWithPuck, const FJamButtonVerbs& Held, int32 SaucerFrames, int32 BraceFrames)
{
	FString Line = UJamHudWidget::ButtonsBadgeLine(bWithPuck);
	// The words swap on the frame the hold counts: a pass held long enough is a saucer, a cross-check held long enough is a brace.
	if (bWithPuck && SaucerFrames > 0 && Held.PassHoldSteps >= SaucerFrames) Line = Line.Replace(TEXT("  PASS"), TEXT("  SAUCER"));
	if (!bWithPuck && BraceFrames > 0 && Held.CrossHoldSteps >= BraceFrames) Line = Line.Replace(TEXT("  CROSS-CHECK"), TEXT("  BRACED"));
	return Line;
}
