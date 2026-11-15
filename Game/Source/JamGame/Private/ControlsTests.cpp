// Lane A183 (R-0064 point 1, track K1): the operator's hands. The two-role buttons by possession, a press that outlives possession, the words from the
// bindings, the rebinding's save, load and conflict swap, the acceptance driver's key table against the defaults, and the lockstep's recorded stream.
#include "Misc/AutomationTest.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "InputMapping.h"
#include "SettingsWidget.h"
#include "MenuFlow.h"
#include "HUDWidget.h"
#include "DirectorsCallWidget.h"
#include "JamSim.h"
#include "JamCartridge.h"
#include "InputMappingContext.h"
#include "HAL/IConsoleManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Misc/CoreDelegates.h"
#include "Internationalization/Regex.h"
#include "HokejGameMode.h"
#include "MenuStyle.h"
#include "Fonts/FontMeasure.h"
#include "Rendering/SlateRenderer.h"
#include "JamBaseline.h"
#include "JamBrain.h"   // lane A203 (item 4)
#include "JamSimAdapter.h"
#include "JamEventBus.h"

#if WITH_DEV_AUTOMATION_TESTS   // Director, 14 Nov: the gate's guard step (lane A159) caught this file with no guard on its first landing


namespace JamControlsTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

	// Restores the shipped bindings when a test that rebinds ends, whatever it asserted.
	bool LoadRegular(FAutomationTestBase& T, FJamGameConfig& Out)
	{
		TArray<FString> Errors;
		UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")), Errors);
		for (const FString& E : Errors) T.AddError(FString::Printf(TEXT("hockey.regular: %s"), *E));
		if (!C) return false;
		Out = C->Game;
		return true;
	}

	struct FRestoreBindings
	{
		FJamBindings Was = UJamInputMap::Get()->GetBindings();
		~FRestoreBindings() { UJamInputMap::Get()->ApplyBindings(Was); }
	};
}

using namespace JamControlsTests;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsDefaults, "Jam.Controls.Defaults", JamControlsTests::Flags)
bool FJamControlsDefaults::RunTest(const FString&)
{
	const FJamBindings D = FJamBindings::Defaults();
	FString Why;
	TestTrue(*FString::Printf(TEXT("the defaults have no clash (%s)"), *Why), D.IsValid(&Why));
	TestTrue(TEXT("LMB: shoot with the puck, cross-check without"), D.WithVerbOn(EKeys::LeftMouseButton) == EJamVerb::Shoot && D.WithoutVerbOn(EKeys::LeftMouseButton) == EJamVerb::CrossCheck);
	TestTrue(TEXT("RMB: pass with the puck, check without"), D.WithVerbOn(EKeys::RightMouseButton) == EJamVerb::Pass && D.WithoutVerbOn(EKeys::RightMouseButton) == EJamVerb::Check);
	TestTrue(TEXT("RT: shoot / cross-check"), D.WithVerbOn(EKeys::Gamepad_RightTriggerAxis) == EJamVerb::Shoot && D.WithoutVerbOn(EKeys::Gamepad_RightTriggerAxis) == EJamVerb::CrossCheck);
	TestTrue(TEXT("RB: pass / check"), D.WithVerbOn(EKeys::Gamepad_RightShoulder) == EJamVerb::Pass && D.WithoutVerbOn(EKeys::Gamepad_RightShoulder) == EJamVerb::Check);
	TestTrue(TEXT("second bindings: Q passes, MMB and mouse 4 cross-check, X passes, LB cross-checks"), D.Has(EJamVerb::Pass, EKeys::Q) && D.Has(EJamVerb::CrossCheck, EKeys::MiddleMouseButton)
		&& D.Has(EJamVerb::CrossCheck, EKeys::ThumbMouseButton) && D.Has(EJamVerb::Pass, EKeys::Gamepad_FaceButton_Left) && D.Has(EJamVerb::CrossCheck, EKeys::Gamepad_LeftShoulder));
	UJamInputMap* M = UJamInputMap::Get();
	FRestoreBindings Restore;
	M->ApplyBindings(D);
	for (const FKey& K : { EKeys::LeftMouseButton, EKeys::RightMouseButton, EKeys::MiddleMouseButton, EKeys::ThumbMouseButton, EKeys::Q, EKeys::Gamepad_RightTriggerAxis,
		EKeys::Gamepad_RightShoulder, EKeys::Gamepad_LeftShoulder, EKeys::Gamepad_FaceButton_Left })
	{
		TestTrue(*FString::Printf(TEXT("%s sits on a button slot of the mapping context"), *K.ToString()), M->SlotKeys.Contains(K)
			&& M->Context->GetMappings().ContainsByPredicate([&](const FEnhancedActionKeyMapping& Mp) { return Mp.Key == K && M->ButtonSlots.ContainsByPredicate([&](const TObjectPtr<UInputAction>& A) { return A.Get() == Mp.Action.Get(); }); }));
	}
	TestTrue(TEXT("IsKeyMapped answers the verbs from the bindings"), M->IsKeyMapped(M->Shoot, EKeys::LeftMouseButton) && M->IsKeyMapped(M->Lift, EKeys::LeftMouseButton)
		&& M->IsKeyMapped(M->Pass, EKeys::RightMouseButton) && M->IsKeyMapped(M->Check, EKeys::RightMouseButton) && !M->IsKeyMapped(M->Shoot, EKeys::RightMouseButton));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsByPossession, "Jam.Controls.ByPossession", JamControlsTests::Flags)
bool FJamControlsByPossession::RunTest(const FString&)
{
	const FJamBindings D = FJamBindings::Defaults();
	auto Tap = [&](const FKey& K, bool bWith)
	{
		FJamButtonResolver R;
		R.Press(K);
		const FJamButtonVerbs V = R.Step(bWith, D);
		return V;
	};
	FJamButtonVerbs V = Tap(EKeys::LeftMouseButton, true);
	TestTrue(TEXT("LMB with the puck: the shot's held level, no cross-check"), V.bShootHeld && V.bShootPress && !V.bLift && !V.bCrossHeld && !V.bCheck && !V.bPassHeld);
	V = Tap(EKeys::LeftMouseButton, false);
	TestTrue(TEXT("LMB without the puck: the cross-check and its hold, no shot"), V.bLift && V.bCrossHeld && !V.bShootHeld && !V.bShootPress);
	V = Tap(EKeys::RightMouseButton, true);
	TestTrue(TEXT("RMB with the puck: the pass's held level, no check"), V.bPassHeld && V.bPassPress && !V.bCheck);
	V = Tap(EKeys::RightMouseButton, false);
	TestTrue(TEXT("RMB without the puck: the body check, no pass"), V.bCheck && !V.bPassHeld && !V.bPassPress);
	V = Tap(EKeys::Gamepad_RightTriggerAxis, true);
	TestTrue(TEXT("RT with the puck shoots"), V.bShootHeld && !V.bLift);
	V = Tap(EKeys::Gamepad_RightTriggerAxis, false);
	TestTrue(TEXT("RT without cross-checks"), V.bLift && V.bCrossHeld);
	V = Tap(EKeys::Gamepad_RightShoulder, true);
	TestTrue(TEXT("RB with the puck passes"), V.bPassHeld && !V.bCheck);
	V = Tap(EKeys::Gamepad_RightShoulder, false);
	TestTrue(TEXT("RB without checks"), V.bCheck && !V.bPassHeld);
	// A key on one role only does that role whatever the puck does.
	V = Tap(EKeys::MiddleMouseButton, true);
	TestTrue(TEXT("MMB (cross-check only) with the puck is still the cross-check"), V.bLift && !V.bShootHeld);
	V = Tap(EKeys::Q, false);
	TestTrue(TEXT("Q (pass only) without the puck is still the pass"), V.bPassHeld && !V.bCheck);
	// Held over frames: the level stays, the edge is once.
	{
		FJamButtonResolver R;
		R.Press(EKeys::LeftMouseButton);
		const FJamButtonVerbs A = R.Step(false, D), B = R.Step(false, D), C = R.Step(false, D);
		TestTrue(TEXT("a held cross-check: one edge, the hold every frame"), A.bLift && !B.bLift && !C.bLift && A.bCrossHeld && B.bCrossHeld && C.bCrossHeld);
		R.Release(EKeys::LeftMouseButton);
		const FJamButtonVerbs E = R.Step(false, D);
		TestTrue(TEXT("released: no hold"), !E.bCrossHeld && !E.bLift);
	}
	// A tap that went down and up between two steps still counts for one step.
	{
		FJamButtonResolver R;
		R.Press(EKeys::LeftMouseButton);
		R.Release(EKeys::LeftMouseButton);
		const FJamButtonVerbs A = R.Step(true, D), B = R.Step(true, D);
		TestTrue(TEXT("a tap inside one step is one frame of the shot's level (a flat shot on its release)"), A.bShootHeld && !B.bShootHeld);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsOutlivesPossession, "Jam.Controls.PressOutlivesPossession", JamControlsTests::Flags)
bool FJamControlsOutlivesPossession::RunTest(const FString&)
{
	const FJamBindings D = FJamBindings::Defaults();
	{
		FJamButtonResolver R;
		R.Press(EKeys::LeftMouseButton);
		const FJamButtonVerbs A = R.Step(true, D);   // pressed with the puck
		const FJamButtonVerbs B = R.Step(false, D);  // the puck is gone (stripped, out of his reach), the button still down
		// Lane A199: the press is dead from the change: its level falls (nothing in reach to fire at) and it never becomes a cross-check.
		TestTrue(TEXT("a shot begun with the puck dies when the puck leaves: no level, never a cross-check"), A.bShootHeld && !B.bShootHeld && !B.bLift && !B.bCrossHeld
			&& B.DeadNow == EJamVerb::Shoot && R.ActiveVerb(EKeys::LeftMouseButton) == EJamVerb::Count);
		R.Release(EKeys::LeftMouseButton);
		const FJamButtonVerbs C = R.Step(false, D);
		TestTrue(TEXT("its release does nothing"), !C.bShootHeld && !C.bLift);
		R.Press(EKeys::LeftMouseButton);
		const FJamButtonVerbs E = R.Step(false, D);
		TestTrue(TEXT("the next press, without the puck, is the cross-check"), E.bLift && E.bCrossHeld && !E.bShootHeld);
	}
	{
		FJamButtonResolver R;
		R.Press(EKeys::RightMouseButton);
		const FJamButtonVerbs A = R.Step(true, D);
		const FJamButtonVerbs B = R.Step(false, D);
		TestTrue(TEXT("a pass begun with the puck dies when the puck leaves (lane A199), never a check"), A.bPassHeld && !B.bPassHeld && !B.bCheck && B.DeadNow == EJamVerb::Pass);
	}
	{
		FJamButtonResolver R;
		R.Press(EKeys::LeftMouseButton);
		const FJamButtonVerbs A = R.Step(false, D);
		const FJamButtonVerbs B = R.Step(true, D);   // the skater picked the puck up while holding the cross-check
		TestTrue(TEXT("a cross-check begun without the puck is no shot when the puck arrives, and its brace ends (lane A199)"), A.bLift && !B.bShootHeld && !B.bShootPress && !B.bCrossHeld
			&& B.DeadNow == EJamVerb::CrossCheck);
	}
	// The sim side: the possession the mapping reads, a staged carrier and a non-carrier.
	FJamGameConfig Cfg;
	if (!LoadRegular(*this, Cfg)) return false;
	UJamSim* Sim = NewObject<UJamSim>();
	Sim->Init(Cfg, 1);
	Sim->SkipFaceoff();
	TArray<FJamInput> In;
	In.Init(FJamInput(), Sim->GetSkaters().Num());
	const FJamSkaterState& H = Sim->GetSkaters()[0];
	// The acceptance drill's staging (HokejAcceptDrill): the puck on the stick side ahead of the skater, at his speed, until he touches it.
	for (int32 F = 0; F < 30 && Sim->CarrierIndex() != 0; ++F)
	{
		const FJamSkaterState& T = Sim->GetSkaters()[0];
		Sim->SetPuck(T.Pos + T.Forward() * 0.45, T.Vel);
		Sim->Step(In);
	}
	const int32 Carrier = Sim->CarrierIndex();
	AddInfo(FString::Printf(TEXT("JAMCTRL test carrier=%d last_touch=%d"), Carrier, Sim->GetPuck().LastTouch));
	if (TestEqual(TEXT("the staged human carries"), Carrier, 0))
	{
		TestTrue(TEXT("the carrier is 'with the puck'"), JamControls::HasPuckForButtons(*Sim, 0));
		int32 Opp = -1;
		for (const FJamSkaterState& S : Sim->GetSkaters()) if (S.Team != H.Team) { Opp = S.Index; break; }
		if (TestTrue(TEXT("an opponent exists"), Opp >= 0)) TestFalse(TEXT("an opponent of the carrier is 'without'"), JamControls::HasPuckForButtons(*Sim, Opp));
		// ApplyButtons writes the verbs: LMB pressed by the carrier is the shot's level; the same press by a non-carrier is the cross-check.
		UJamPlayerInput* Pi = NewObject<UJamPlayerInput>();
		FJamInput A;
		Pi->DebugButton(EKeys::LeftMouseButton, true);
		JamControls::ApplyButtons(*Pi, *Sim, 0, A);
		TestTrue(TEXT("ApplyButtons: the carrier's LMB is bShootHeld"), A.bShootHeld && !A.bLift);
		UJamPlayerInput* Po = NewObject<UJamPlayerInput>();
		FJamInput B;
		Po->DebugButton(EKeys::LeftMouseButton, true);
		if (Opp >= 0) { JamControls::ApplyButtons(*Po, *Sim, Opp, B); TestTrue(TEXT("ApplyButtons: the opponent's LMB is bLift"), B.bLift && !B.bShootHeld); }
	}
	// Our pass in flight: a loose puck last touched by a teammate is 'with' (the one-timer's early press); 0 gives back the strict carrier.
	{
		int32 Mate = -1;
		for (const FJamSkaterState& S : Sim->GetSkaters()) if (S.Team == H.Team && S.Index != 0) { Mate = S.Index; break; }
		IConsoleVariable* Our = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.ctrl.OurPuckShoots"));
		if (Mate >= 0 && TestNotNull(TEXT("jam.ctrl.OurPuckShoots exists"), Our))
		{
			// The human passes: the puck leaves him (far from everyone, moving) and stays his touch.
			Sim->SetPuck(FVector2D(0.0, 0.0), FVector2D(12.0, 0.0));
			const bool bLoose = Sim->CarrierIndex() < 0 && Sim->GetPuck().LastTouch == 0;
			AddInfo(FString::Printf(TEXT("JAMCTRL test loose=%d last_touch=%d carrier=%d mate=%d"), bLoose ? 1 : 0, Sim->GetPuck().LastTouch, Sim->CarrierIndex(), Mate));
			if (bLoose)
			{
				TestTrue(TEXT("our pass in flight: the mate is 'with the puck' (a one-timer's press is a shot)"), JamControls::HasPuckForButtons(*Sim, Mate));
				Our->Set(0, ECVF_SetByCode);
				TestFalse(TEXT("jam.ctrl.OurPuckShoots 0: only the carrier is 'with'"), JamControls::HasPuckForButtons(*Sim, Mate));
				Our->Set(1, ECVF_SetByCode);
			}
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsLockstepStream, "Jam.Controls.RecordedStreamUnchanged", JamControlsTests::Flags)
bool FJamControlsLockstepStream::RunTest(const FString&)
{
	// The mapping happens before the input is recorded: the sim and the lockstep see verbs only. A recorded stream replays to the same hash under any
	// binding, and a stream of verbs written by the resolver equals the stream of before for the same verbs.
	FRestoreBindings Restore;
	FJamGameConfig Cfg;
	if (!LoadRegular(*this, Cfg)) return false;
	auto Play = [&](int32 Frames)
	{
		UJamSim* Sim = NewObject<UJamSim>();
		Sim->Init(Cfg, 7);
		TArray<FJamInput> In;
		In.Init(FJamInput(), Sim->GetSkaters().Num());
		for (int32 F = 0; F < Frames; ++F)
		{
			In[0] = FJamInput();
			In[0].Move = FVector2D(1.0, (F / 60) % 2 ? 0.5 : -0.5);
			In[0].bShootHeld = (F % 90) > 60;
			In[0].bPassHeld = (F % 200) > 185;
			In[0].bCheck = (F % 150) == 75;
			In[0].bLift = (F % 170) == 33;
			Sim->Step(In);
		}
		return Sim->StateHash();
	};
	const uint64 A = Play(900);
	FJamBindings R = FJamBindings::Defaults();
	R.Rebind(EJamVerb::Shoot, EKeys::E);
	R.Rebind(EJamVerb::Pass, EKeys::MiddleMouseButton);
	UJamInputMap::Get()->ApplyBindings(R);
	const uint64 B = Play(900);
	AddInfo(FString::Printf(TEXT("JAMCTRL recorded stream hash default=%016llx rebound=%016llx"), A, B));
	TestEqual(TEXT("the recorded stream's hash does not move with the bindings"), B, A);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsRebindSwap, "Jam.Controls.RebindSwap", JamControlsTests::Flags)
bool FJamControlsRebindSwap::RunTest(const FString&)
{
	FJamBindings B = FJamBindings::Defaults();
	EJamVerb Swapped = EJamVerb::Count;
	TestTrue(TEXT("spin onto F"), B.Rebind(EJamVerb::Spin, EKeys::F, &Swapped));
	TestTrue(TEXT("the ability, which held F, swaps onto E"), Swapped == EJamVerb::Ability && B[EJamVerb::Ability].Keys[0] == EKeys::E && B[EJamVerb::Spin].Keys[0] == EKeys::F);
	TestTrue(TEXT("valid after the swap"), B.IsValid());
	B = FJamBindings::Defaults();
	TestTrue(TEXT("shoot onto RMB"), B.Rebind(EJamVerb::Shoot, EKeys::RightMouseButton, &Swapped));
	TestTrue(TEXT("the pass (the other with-verb on RMB) swaps onto LMB; the check keeps RMB (a with / without pair)"), Swapped == EJamVerb::Pass && B[EJamVerb::Pass].Keys[0] == EKeys::LeftMouseButton
		&& B.Has(EJamVerb::Check, EKeys::RightMouseButton) && B.WithVerbOn(EKeys::RightMouseButton) == EJamVerb::Shoot);
	FString Why;
	TestTrue(*FString::Printf(TEXT("valid (%s)"), *Why), B.IsValid(&Why));
	B = FJamBindings::Defaults();
	TestTrue(TEXT("poke onto LMB"), B.Rebind(EJamVerb::Poke, EKeys::LeftMouseButton, &Swapped));
	TestTrue(TEXT("shoot and cross-check give LMB up for Space"), !B.Has(EJamVerb::Shoot, EKeys::LeftMouseButton) && !B.Has(EJamVerb::CrossCheck, EKeys::LeftMouseButton)
		&& B[EJamVerb::Shoot].Keys[0] == EKeys::SpaceBar && B.IsValid(&Why));
	B = FJamBindings::Defaults();
	TestTrue(TEXT("check onto LMB: allowed beside the shot (a without-verb), the cross-check gives it up"), B.Rebind(EJamVerb::Check, EKeys::LeftMouseButton, &Swapped) && B.IsValid(&Why)
		&& B.WithVerbOn(EKeys::LeftMouseButton) == EJamVerb::Shoot && B.WithoutVerbOn(EKeys::LeftMouseButton) == EJamVerb::Check);
	B = FJamBindings::Defaults();
	TestFalse(TEXT("Esc is never accepted"), B.Rebind(EJamVerb::Shoot, EKeys::Escape));
	TestFalse(TEXT("the mouse axis is never accepted"), B.Rebind(EJamVerb::Shoot, EKeys::Mouse2D));
	TestFalse(TEXT("a pad button never goes on a skate direction"), B.Rebind(EJamVerb::SkateForward, EKeys::Gamepad_FaceButton_Bottom));
	TestTrue(TEXT("a pad press rebinds the pad slot: poke onto RB"), B.Rebind(EJamVerb::Poke, EKeys::Gamepad_RightShoulder, &Swapped) && B[EJamVerb::Poke].Pads[0] == EKeys::Gamepad_RightShoulder
		&& B[EJamVerb::Poke].Keys[0] == EKeys::SpaceBar && B.IsValid(&Why));
	// Every verb onto every key of a sample: the table stays valid.
	const FKey Sample[] = { EKeys::LeftMouseButton, EKeys::RightMouseButton, EKeys::Q, EKeys::E, EKeys::F, EKeys::SpaceBar, EKeys::LeftShift, EKeys::W, EKeys::Gamepad_RightTriggerAxis, EKeys::Gamepad_FaceButton_Bottom };
	int32 Bad = 0;
	for (int32 V = 0; V < FJamBindings::NumVerbs; ++V)
	{
		for (const FKey& K : Sample)
		{
			FJamBindings T = FJamBindings::Defaults();
			T.Rebind(static_cast<EJamVerb>(V), K);
			if (!T.IsValid(&Why)) { ++Bad; AddError(FString::Printf(TEXT("%s onto %s: %s"), FJamBindings::VerbId(static_cast<EJamVerb>(V)), *K.ToString(), *Why)); }
		}
	}
	TestEqual(TEXT("no rebinding of the sample leaves a clash"), Bad, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsWords, "Jam.Controls.WordsFollowBindings", JamControlsTests::Flags)
bool FJamControlsWords::RunTest(const FString&)
{
	FRestoreBindings Restore;
	UJamInputMap* M = UJamInputMap::Get();
	M->ApplyBindings(FJamBindings::Defaults());
	const FString Strip = UJamHudWidget::ControlsHintText();
	AddInfo(FString::Printf(TEXT("JAMCTRL strip (%d chars): %s"), Strip.Len(), *Strip));
	TestTrue(TEXT("the strip names the two-role buttons"), Strip.Contains(TEXT("SHOOT / CROSS-CHECK  LMB")) && Strip.Contains(TEXT("PASS / CHECK  RMB")));
	TestTrue(TEXT("the strip keeps the skate, the brake, the aim, the poke, the ability, the sprint"), Strip.Contains(TEXT("SKATE  WASD")) && Strip.Contains(TEXT("BRAKE  S AGAINST YOUR MOTION"))
		&& Strip.Contains(TEXT("AIM  MOUSE")) && Strip.Contains(TEXT("POKE  SPACE")) && Strip.Contains(TEXT("ABILITY  F")) && Strip.Contains(TEXT("SPRINT  SHIFT")));
	const FString Card = UJamDirectorsCallWidget::PauseControlsText();
	AddInfo(FString::Printf(TEXT("JAMCTRL pause card:\n%s"), *Card));
	TestTrue(TEXT("the pause card names the roles"), Card.Contains(TEXT("SHOOT / CROSS-CHECK  LMB / RT")) && Card.Contains(TEXT("PASS / CHECK  RMB / RB")));
	TestTrue(TEXT("the CONTROLS rows show the two-role buttons as two rows each"), UJamMenuFlow::ControlRows().ContainsByPredicate([](const FJamControlRow& R) { return R.Verb == TEXT("SHOOT (WITH THE PUCK)") && R.Keys.StartsWith(TEXT("left mouse button")); })
		&& UJamMenuFlow::ControlRows().ContainsByPredicate([](const FJamControlRow& R) { return R.Verb == TEXT("CROSS-CHECK (WITHOUT)") && R.Keys.StartsWith(TEXT("left mouse button")); })
		&& UJamMenuFlow::ControlRows().ContainsByPredicate([](const FJamControlRow& R) { return R.Verb == TEXT("PASS (WITH THE PUCK)") && R.Keys.StartsWith(TEXT("right mouse button")); })
		&& UJamMenuFlow::ControlRows().ContainsByPredicate([](const FJamControlRow& R) { return R.Verb == TEXT("CHECK (WITHOUT)") && R.Keys.StartsWith(TEXT("right mouse button")); }));
	// Rebound: the words move with the keys.
	FJamBindings R = FJamBindings::Defaults();
	R.Rebind(EJamVerb::CrossCheck, EKeys::MiddleMouseButton);   // LMB is now the shot only
	R.Rebind(EJamVerb::Poke, EKeys::G);
	R.Rebind(EJamVerb::Brake, EKeys::LeftAlt);
	R.Rebind(EJamVerb::SkateBack, EKeys::X);
	M->ApplyBindings(R);
	const FString Strip2 = UJamHudWidget::ControlsHintText(), Card2 = UJamDirectorsCallWidget::PauseControlsText();
	AddInfo(FString::Printf(TEXT("JAMCTRL strip rebound: %s"), *Strip2));
	TestTrue(TEXT("rebound: the strip splits shoot and cross-check and names G for the poke"), Strip2.Contains(TEXT("SHOOT  LMB")) && Strip2.Contains(TEXT("CROSS-CHECK  MMB")) && Strip2.Contains(TEXT("POKE  G")) && Strip2.Contains(TEXT("SKATE  WAXD")));
	TestTrue(TEXT("rebound: the pause card names ALT for the brake"), Card2.Contains(TEXT("ALT")) && Card2.Contains(TEXT("POKE  G")));
	FKey Back, Brake;
	TestTrue(TEXT("rebound: the brake prompt's keys follow (X back, ALT brake)"), UJamHudWidget::BrakeKeysFor(false, Back, Brake) && Back == EKeys::X && Brake == EKeys::LeftAlt);
	// The longest key names: every verb's first key a long-named key. The strip stays within the length the strip holds; the card's lines are reported.
	FJamBindings L = FJamBindings::Defaults();
	const FKey Long[] = { EKeys::BackSpace, EKeys::PageDown, EKeys::PageUp, EKeys::Insert, EKeys::Delete, EKeys::Home, EKeys::End, EKeys::CapsLock, EKeys::Tab, EKeys::Enter, EKeys::RightBracket, EKeys::Semicolon, EKeys::NumPadNine, EKeys::Multiply, EKeys::Divide };
	for (int32 V = 0; V < FJamBindings::NumVerbs && V < UE_ARRAY_COUNT(Long); ++V) L.Rebind(static_cast<EJamVerb>(V), Long[V]);
	FString Why;
	TestTrue(*FString::Printf(TEXT("the long table is valid (%s)"), *Why), L.IsValid(&Why));
	M->ApplyBindings(L);
	const FString Strip3 = UJamHudWidget::ControlsHintText();
	AddInfo(FString::Printf(TEXT("JAMCTRL strip with long names (%d chars): %s"), Strip3.Len(), *Strip3));
	TestTrue(TEXT("the strip with the longest names holds its length"), Strip3.Len() <= UJamInputMap::HintStripMaxChars && Strip3.StartsWith(TEXT("SKATE")));
	for (int32 V = 0; V < FJamBindings::NumVerbs; ++V) TestTrue(TEXT("a key name is at most KeyNameMax characters"), UJamInputMap::KeyName(L[static_cast<EJamVerb>(V)].Keys.Num() ? L[static_cast<EJamVerb>(V)].Keys[0] : FKey()).Len() <= UJamInputMap::KeyNameMax);
	TArray<FString> Lines;
	UJamDirectorsCallWidget::PauseControlsText().ParseIntoArrayLines(Lines);
	int32 Widest = 0;
	for (const FString& Ln : Lines) Widest = FMath::Max(Widest, Ln.Len());
	AddInfo(FString::Printf(TEXT("JAMCTRL pause card with long names: %d lines, widest %d chars (the card wraps at its inner width)"), Lines.Num(), Widest));
	TestEqual(TEXT("the pause card keeps its five lines"), Lines.Num(), 5);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsAcceptKeys, "Jam.Controls.AcceptKeysMatchDefaults", JamControlsTests::Flags)
bool FJamControlsAcceptKeys::RunTest(const FString&)
{
	// The acceptance driver's key table (tools/qa/accept/keys.json, read by accept.ps1) presses the game's defaults: each verb's button is on that verb's
	// default binding in the role the drill stages (shoot and pass with the puck, cross-check and check without).
	const FString Path = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT(".."), TEXT("tools"), TEXT("qa"), TEXT("accept"), TEXT("keys.json")));
	FString Text;
	if (!TestTrue(*FString::Printf(TEXT("the driver's key table reads (%s)"), *Path), FFileHelper::LoadFileToString(Text, *Path))) return false;
	// A flat object of "verb": "BUTTON" pairs (the module carries no JSON reader; the driver's own ConvertFrom-Json reads the same file).
	auto Field = [&](const TCHAR* Name, FString& Out)
	{
		FRegexMatcher Mt(FRegexPattern(FString::Printf(TEXT("\"%s\"\\s*:\\s*\"(\\w+)\""), Name)), Text);
		if (!Mt.FindNext()) return false;
		Out = Mt.GetCaptureGroup(1);
		return true;
	};
	auto KeyOf = [](const FString& N) { return N == TEXT("LMB") ? EKeys::LeftMouseButton : N == TEXT("RMB") ? EKeys::RightMouseButton : N == TEXT("MMB") ? EKeys::MiddleMouseButton : FKey(*N); };
	const FJamBindings D = FJamBindings::Defaults();
	struct FRowSpec { const TCHAR* Name; EJamVerb Verb; bool bWith; };
	const FRowSpec Rows[] = { { TEXT("shoot"), EJamVerb::Shoot, true }, { TEXT("crosscheck"), EJamVerb::CrossCheck, false },
		{ TEXT("pass"), EJamVerb::Pass, true }, { TEXT("check"), EJamVerb::Check, false }, { TEXT("onetimer"), EJamVerb::Shoot, true } };
	for (const FRowSpec& R : Rows)
	{
		FString N;
		if (!TestTrue(*FString::Printf(TEXT("keys.json has %s"), R.Name), Field(R.Name, N))) continue;
		const FKey K = KeyOf(N);
		const EJamVerb Got = R.bWith ? D.WithVerbOn(K) : D.WithoutVerbOn(K);
		TestTrue(*FString::Printf(TEXT("the driver's %s (%s) is %s %s the puck in the defaults"), R.Name, *N, FJamBindings::VerbId(R.Verb), R.bWith ? TEXT("with") : TEXT("without")), Got == R.Verb);
	}
	FString Sh, Cc, Pa, Ch;
	TestTrue(TEXT("the new defaults: shoot and cross-check on LMB, pass and check on RMB"), Field(TEXT("shoot"), Sh) && Field(TEXT("crosscheck"), Cc) && Field(TEXT("pass"), Pa) && Field(TEXT("check"), Ch)
		&& Sh == TEXT("LMB") && Cc == TEXT("LMB") && Pa == TEXT("RMB") && Ch == TEXT("RMB"));
	return true;
}

// ---------------------------------------------------------------- item 2: the CONTROLS screen, the save with the profile

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsSaveLoad, "Jam.Controls.SaveAndLoad", JamControlsTests::Flags)
bool FJamControlsSaveLoad::RunTest(const FString&)
{
	// The test seam: the profile lives in memory (FJamSettings::SetOverrideForTest); the player's file is never read or written.
	const FJamSettings Fresh;
	FJamSettings::SetOverrideForTest(&Fresh);
	ON_SCOPE_EXIT { FJamSettings::SetOverrideForTest(nullptr); };
	FRestoreBindings Restore;
	UJamInputMap::Get()->ApplyBindings(FJamBindings::Defaults());
	FJamBindings B = FJamBindings::Defaults();
	B.Rebind(EJamVerb::Spin, EKeys::F);                              // F from the ability (a swap)
	B.Rebind(EJamVerb::Pass, EKeys::Gamepad_FaceButton_Left);          // the pad's X first
	B.Rebind(EJamVerb::CrossCheck, EKeys::ThumbMouseButton2);          // mouse 5 first: LMB stays the shot's alone
	FJamSettings St = FJamSettings::Load();
	St.Controls = B.ToSaved();
	St.Save();
	// A "restart": the saved form read back through the same keys the file holds, then the bindings from it.
	const TMap<FString, FString> Saved = FJamSettings::Load().ToSaved();
	TestTrue(TEXT("the saved form carries every verb under Controls.<verb>"), Saved.Contains(TEXT("Controls.Shoot")) && Saved.Contains(TEXT("Controls.Restart")) && Saved.Contains(TEXT("Difficulty")));
	const FJamSettings Back = FJamSettings::FromSaved(Saved);
	const FJamBindings Got = FJamBindings::FromSaved(Back.Controls);
	TestTrue(TEXT("the bindings survive the round trip whole"), Got == B);
	TestTrue(TEXT("spin is on F and the ability on E after the restart"), Got[EJamVerb::Spin].Keys[0] == EKeys::F && Got[EJamVerb::Ability].Keys[0] == EKeys::E);
	TestTrue(TEXT("LMB is the shot only (cross-check moved to mouse 5)"), Got.WithoutVerbOn(EKeys::LeftMouseButton) == EJamVerb::Count && Got.WithVerbOn(EKeys::LeftMouseButton) == EJamVerb::Shoot);
	// A broken saved form never breaks the controls: unknown keys and verbs drop, a clash gives the defaults whole.
	TMap<FString, FString> Bad;
	Bad.Add(TEXT("Shoot"), TEXT("NoSuchKey,LeftMouseButton|Gamepad_RightTriggerAxis"));
	Bad.Add(TEXT("Nonsense"), TEXT("Q|"));
	const FJamBindings FromBad = FJamBindings::FromSaved(Bad);
	TestTrue(TEXT("an unknown key and an unknown verb are dropped"), FromBad[EJamVerb::Shoot].Keys.Num() == 1 && FromBad[EJamVerb::Shoot].Keys[0] == EKeys::LeftMouseButton);
	TMap<FString, FString> Clash;
	Clash.Add(TEXT("Poke"), TEXT("E|Gamepad_FaceButton_Bottom"));   // E is the spin's
	TestTrue(TEXT("a clash in the saved form gives back the defaults"), FJamBindings::FromSaved(Clash) == FJamBindings::Defaults());
	TestTrue(TEXT("the settings file's own section is Jam.Controls"), FString(FJamSettings::ControlsSection) == TEXT("Jam.Controls"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsScreen, "Jam.Controls.Screen", JamControlsTests::Flags)
bool FJamControlsScreen::RunTest(const FString&)
{
	const FJamSettings Fresh;
	FJamSettings::SetOverrideForTest(&Fresh);
	ON_SCOPE_EXIT { FJamSettings::SetOverrideForTest(nullptr); };
	FRestoreBindings Restore;
	UJamInputMap::Get()->ApplyBindings(FJamBindings::Defaults());
	UWorld* World = nullptr;
	if (GEngine && FSlateApplication::IsInitialized())
	{
		World = UWorld::CreateWorld(EWorldType::Game, false, TEXT("JamControlsTestWorld"));
		GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
	}
	ON_SCOPE_EXIT { if (World) { GEngine->DestroyWorldContext(World); World->DestroyWorld(false); } };
	UJamControlsWidget* W = UJamSettingsWidget::MakeControlsScreen(World);
	if (!TestNotNull(TEXT("MakeControlsScreen builds the screen"), W)) return false;
	W->AddToRoot();
	ON_SCOPE_EXIT { W->ReleaseSlateResources(true); W->RemoveFromRoot(); };
	TSharedRef<SWidget> Slate = W->TakeWidget();
	W->Refresh();
	Slate->SlatePrepass(1.f);
	const FVector2D Size = Slate->GetDesiredSize();
	AddInfo(FString::Printf(TEXT("JAMCTRL controls screen %.0f x %.0f, %d focusables"), Size.X, Size.Y, W->GetFocusables().Num()));
	TestTrue(TEXT("the screen fits 1920 x 1080 with the menu card's padding (at most 1720 x 960)"), Size.X > 100.f && Size.X <= 1720.f && Size.Y > 100.f && Size.Y <= 960.f);
	TestEqual(TEXT("every rebindable cell and the two buttons are on the pad walk (15 key cells, 11 pad cells, reset, back)"), W->GetFocusables().Num(), 15 + 11 + 2);
	TestEqual(TEXT("the shot's cells read LMB / RT"), W->CellText(EJamVerb::Shoot, false) + TEXT(" / ") + W->CellText(EJamVerb::Shoot, true), FString(TEXT("LMB / RT")));
	TestEqual(TEXT("the cross-check's keys cell lists its second bindings"), W->CellText(EJamVerb::CrossCheck, false), FString(TEXT("LMB, MMB, MOUSE4")));
	// Listen, press, swap.
	W->BeginListen(EJamVerb::Spin, false);
	TestTrue(TEXT("listening"), W->IsListening());
	TestFalse(TEXT("a pad button on the keys cell is refused"), W->HandleListenedKey(EKeys::Gamepad_FaceButton_Top));
	TestTrue(TEXT("still listening after the refusal"), W->IsListening());
	TestTrue(TEXT("F taken"), W->HandleListenedKey(EKeys::F));
	TestFalse(TEXT("no longer listening"), W->IsListening());
	AddInfo(FString::Printf(TEXT("JAMCTRL screen status: %s"), *W->GetStatusLine()));
	TestTrue(TEXT("the line says the swap"), W->GetStatusLine().Contains(TEXT("SPIN is now on F")) && W->GetStatusLine().Contains(TEXT("ABILITY")) && W->GetStatusLine().Contains(TEXT("moved to E")));
	TestTrue(TEXT("the game's bindings changed at once"), UJamInputMap::Get()->GetBindings()[EJamVerb::Spin].Keys[0] == EKeys::F && UJamInputMap::Get()->IsKeyMapped(UJamInputMap::Get()->Spin, EKeys::F));
	TestTrue(TEXT("saved with the profile at once"), FJamBindings::FromSaved(FJamSettings::Load().Controls)[EJamVerb::Spin].Keys[0] == EKeys::F);
	TestTrue(TEXT("the HUD strip says F for the ability no more: ABILITY  E"), UJamHudWidget::ControlsHintText().Contains(TEXT("ABILITY  E")));
	// The pad cell, a trigger.
	W->BeginListen(EJamVerb::Sprint, true);
	TestTrue(TEXT("RT (its digital event) onto the sprint's pad cell"), W->HandleListenedKey(EKeys::Gamepad_RightTrigger));
	const FJamBindings& Now = UJamInputMap::Get()->GetBindings();
	FString Why;
	TestTrue(*FString::Printf(TEXT("RT is the sprint's; shoot and cross-check took LT in its place; valid (%s)"), *Why), Now[EJamVerb::Sprint].Pads[0] == EKeys::Gamepad_RightTriggerAxis
		&& Now[EJamVerb::Shoot].Pads[0] == EKeys::Gamepad_LeftTriggerAxis && Now.IsValid(&Why));
	// Esc cancels.
	W->BeginListen(EJamVerb::Poke, false);
	TestTrue(TEXT("Esc cancels"), W->HandleListenedKey(EKeys::Escape) && !W->IsListening() && UJamInputMap::Get()->GetBindings()[EJamVerb::Poke].Keys[0] == EKeys::SpaceBar);
	// Reset.
	W->ResetToDefaults();
	TestTrue(TEXT("RESET TO DEFAULTS: the game and the profile have the defaults"), UJamInputMap::Get()->GetBindings() == FJamBindings::Defaults() && FJamBindings::FromSaved(FJamSettings::Load().Controls) == FJamBindings::Defaults());
	return true;
}

// ---------------------------------------------------------------- lane A191 (evolution 2): the still hand's aim, the held cross-check, the badge, the pause card

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsAimAnchor, "Jam.Controls.AimAnchor", JamControlsTests::Flags)
bool FJamControlsAimAnchor::RunTest(const FString&)
{
	// A standing skater, the cursor still, the camera correcting: the ice point under the pixel walks 0.4 m sideways over 30 frames.
	const FVector2D Sk(10.0, 5.0), Px(1100.0, 600.0);
	FJamAimAnchor A;
	FVector2D Got = A.Step(Px, Sk + FVector2D(2.5, 0.0), Sk, true, 0.3);
	TestTrue(TEXT("the first step aims at the cursor's ice point"), Got.Equals(Sk + FVector2D(2.5, 0.0), 1e-6));
	double WorstTurn = 0.0;
	for (int32 F = 1; F <= 30; ++F)
	{
		Got = A.Step(Px, Sk + FVector2D(2.5, 0.4 * F / 30.0), Sk, true, 0.3);
		WorstTurn = FMath::Max(WorstTurn, FMath::Abs(FMath::RadiansToDegrees(FMath::Atan2(Got.Y - Sk.Y, Got.X - Sk.X))));
	}
	TestTrue(TEXT("a still hand on a standing skater: the blade holds through the correction (0 deg)"), WorstTurn < 1e-6 && A.IsAnchored());
	// The hand moves (one pixel): the aim takes the cursor's point at once.
	Got = A.Step(Px + FVector2D(1.0, 0.0), Sk + FVector2D(2.5, 0.4), Sk, true, 0.3);
	TestTrue(TEXT("a moved pixel re-projects"), Got.Equals(Sk + FVector2D(2.5, 0.4), 1e-6) && !A.IsAnchored());
	// A drift under half a pixel adds up: the reference stays where the anchor began, so 0.3 + 0.3 px re-projects.
	FJamAimAnchor D;
	D.Step(Px, Sk + FVector2D(2.5, 0.0), Sk, true, 0.3);
	D.Step(Px + FVector2D(0.3, 0.0), Sk + FVector2D(2.5, 0.1), Sk, true, 0.3);
	TestTrue(TEXT("0.3 px: still anchored"), D.IsAnchored());
	Got = D.Step(Px + FVector2D(0.6, 0.0), Sk + FVector2D(2.5, 0.2), Sk, true, 0.3);
	TestTrue(TEXT("0.6 px from where the anchor began: re-projected"), !D.IsAnchored() && Got.Equals(Sk + FVector2D(2.5, 0.2), 1e-6));
	// The skater skates (1 m/s): the camera leads him, the cursor's point is what the blade follows.
	FJamAimAnchor M;
	M.Step(Px, Sk + FVector2D(2.5, 0.0), Sk, true, 0.3);
	const FVector2D Sk2 = Sk + FVector2D(1.0 / 60.0, 0.0);
	Got = M.Step(Px, Sk2 + FVector2D(2.5, 0.3), Sk2, true, 0.3);
	TestTrue(TEXT("a skating skater (1 m/s, over 0.3): re-projected"), !M.IsAnchored() && Got.Equals(Sk2 + FVector2D(2.5, 0.3), 1e-6));
	// Off: the aim of before (every step re-projected).
	FJamAimAnchor O;
	O.Step(Px, Sk + FVector2D(2.5, 0.0), Sk, false, 0.3);
	Got = O.Step(Px, Sk + FVector2D(2.5, 0.3), Sk, false, 0.3);
	TestTrue(TEXT("jam.ctrl.AimAnchor 0: re-projected every step"), Got.Equals(Sk + FVector2D(2.5, 0.3), 1e-6));
	// The stick from the anchored point is the stick of before for the same point (CursorAimFromHit is CursorAimFromRay's ice math).
	bool bA = false, bB = false;
	FVector2D S1, S2;
	const FVector Origin(1000.0, 500.0, 350.0), Dir = (FVector(1250.0, 520.0, 0.0) - Origin).GetSafeNormal();
	AHokejGameMode::CursorAimFromRay(Origin, Dir, Sk, bA, S1);
	const FVector HitW = Origin + Dir * (-Origin.Z / Dir.Z);
	AHokejGameMode::CursorAimFromHit(FVector2D(HitW.X / 100.0, HitW.Y / 100.0), Sk, bB, S2);
	TestTrue(TEXT("CursorAimFromHit gives CursorAimFromRay's stick"), S1.Equals(S2, 1e-9) && bA == bB);
	IConsoleVariable* Cv = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.ctrl.AimAnchor"));
	TestTrue(TEXT("jam.ctrl.AimAnchor ships on (the measured variant: status/A191.md)"), Cv && Cv->GetInt() == 1);
	IConsoleVariable* Sp = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.ctrl.AimAnchorSpeed"));
	TestTrue(TEXT("jam.ctrl.AimAnchorSpeed is 0.3 m/s"), Sp && FMath::IsNearlyEqual(Sp->GetFloat(), 0.3f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsHeldCrossCheck, "Jam.Controls.HeldCrossCheck", JamControlsTests::Flags)
bool FJamControlsHeldCrossCheck::RunTest(const FString&)
{
	// The acceptance drill's step 7 k=3 in the staged sim: the human comes from 9 m at 5 m/s with the cross-check held (bLift on the press, bCrossHold every
	// frame) into a carrier that stands: the man goes down by the speed brought. Runs once A184's FJamInput::bCrossHold is on the tree (JAM_INPUT_HAS_CROSS_HOLD 1).
#if JAM_INPUT_HAS_CROSS_HOLD
	FJamGameConfig Cfg;
	if (!LoadRegular(*this, Cfg)) return false;
	UJamSim* Sim = NewObject<UJamSim>();
	Sim->Init(Cfg, 1);
	Sim->SkipFaceoff();
	const int32 Me = Sim->GetHumanIndex();
	int32 Opp = -1;
	for (const FJamSkaterState& S : Sim->GetSkaters()) if (S.Team != Sim->GetSkaters()[Me].Team) { Opp = S.Index; break; }
	if (!TestTrue(TEXT("an opponent"), Opp >= 0)) return false;
	for (const FJamSkaterState& S : Sim->GetSkaters()) if (S.Index != Me && S.Index != Opp) Sim->SetSkater(S.Index, FVector2D(-25.0, -10.0 + S.Index), FVector2D::ZeroVector, 0.0);
	Sim->SetSkater(Me, FVector2D(-9.0, 0.0), FVector2D(5.0, 0.0), 0.0);
	Sim->SetSkater(Opp, FVector2D(2.0, 0.0), FVector2D::ZeroVector, PI);
	Sim->ClearReleaseGrace(Opp);
	Sim->SetPuck(FVector2D(2.45, 0.0), FVector2D::ZeroVector);
	TArray<FJamInput> In;
	In.Init(FJamInput(), Sim->GetSkaters().Num());
	bool bDown = false;
	double Closing = 0.0;
	for (int32 F = 0; F < 240 && !bDown; ++F)
	{
		In[Me] = FJamInput();
		In[Me].Move = FVector2D(1.0, 0.0);
		In[Me].bLift = F == 0;
		JamControls::SetCrossHold(In[Me], true);
		Sim->Step(In);
		const FJamSkaterState& H = Sim->GetSkaters()[Me];
		const FJamSkaterState& O = Sim->GetSkaters()[Opp];
		if (Closing == 0.0 && FVector2D::Distance(H.Pos, O.Pos) <= UJamSim::SkaterRadius * 2.0 + 0.25) Closing = (H.Vel - O.Vel).Size();
		bDown = O.IsDown();
	}
	AddInfo(FString::Printf(TEXT("JAMCTRL held cross-check: down=%d closing=%.2f"), bDown ? 1 : 0, Closing));
	TestTrue(TEXT("held into a standing man at speed: he goes down"), bDown);
#else
	AddInfo(TEXT("JAMCTRL held cross-check: skipped (JAM_INPUT_HAS_CROSS_HOLD 0: A184's FJamInput::bCrossHold is not on this tree; the drill's row logs skipped=no_hold)"));
	FJamInput In;
	JamControls::SetCrossHold(In, true);   // compiles and does nothing without the field
#endif
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsButtonsBadge, "Jam.Controls.ButtonsBadge", JamControlsTests::Flags)
bool FJamControlsButtonsBadge::RunTest(const FString&)
{
	FRestoreBindings Restore;
	UJamInputMap::Get()->ApplyBindings(FJamBindings::Defaults());
	UJamInputMap::NoteInputDevice(false);   // lane A199: the mouse scheme's words
	TestEqual(TEXT("with the puck"), UJamHudWidget::ButtonsBadgeLine(true), FString(TEXT("LMB  SHOOT     RMB  PASS")));
	TestEqual(TEXT("without"), UJamHudWidget::ButtonsBadgeLine(false), FString(TEXT("LMB  CROSS-CHECK     RMB  CHECK")));
	// Within a frame: on every frame of a staged pickup and loss the badge and an LMB press read the same possession (HasPuckForButtons of that frame).
	FJamGameConfig Cfg;
	if (!LoadRegular(*this, Cfg)) return false;
	UJamSim* Sim = NewObject<UJamSim>();
	Sim->Init(Cfg, 1);
	Sim->SkipFaceoff();
	TArray<FJamInput> In;
	In.Init(FJamInput(), Sim->GetSkaters().Num());
	int32 Agree = 0, Frames = 0, Flips = 0;
	bool bLast = false;
	for (int32 F = 0; F < 90; ++F)
	{
		const FJamSkaterState& T = Sim->GetSkaters()[0];
		// Carried for 40 frames, then an opponent's (on his blade: his touch; a loose puck of our own would still read "with": our pass in flight).
		int32 Opp = -1;
		for (const FJamSkaterState& S : Sim->GetSkaters()) if (S.Team != T.Team) { Opp = S.Index; break; }
		const FJamSkaterState& Holder = F < 40 || Opp < 0 ? T : Sim->GetSkaters()[Opp];
		Sim->SetPuck(Holder.Pos + Holder.Forward() * 0.45, Holder.Vel);
		Sim->Step(In);
		const bool bWith = JamControls::HasPuckForButtons(*Sim, 0);
		FJamButtonResolver R;
		R.Press(EKeys::LeftMouseButton);
		const FJamButtonVerbs V = R.Step(bWith, UJamInputMap::Get()->GetBindings());
		const FString Line = UJamHudWidget::ButtonsBadgeLine(bWith);
		Agree += (V.bShootPress == Line.Contains(TEXT("LMB  SHOOT")) && V.bLift == Line.Contains(TEXT("LMB  CROSS-CHECK"))) ? 1 : 0;
		if (F > 0 && bWith != bLast) ++Flips;
		bLast = bWith;
		++Frames;
	}
	AddInfo(FString::Printf(TEXT("JAMCTRL badge: %d of %d frames agree with the press, %d flips"), Agree, Frames, Flips));
	TestEqual(TEXT("the badge says what an LMB press does on every frame"), Agree, Frames);
	TestTrue(TEXT("the staging flipped possession"), Flips >= 1);
	// The fit: the pill's words at the HUD's type, with the longest key names, inside ButtonsBadgeMaxW.
	const TSharedRef<FSlateFontMeasure> Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	const FSlateFontInfo Font = FJamUiStyle::Font(FJamUiStyle::Body - 2, FName("Bold"), 20);
	auto Width = [&](const FString& S) { return static_cast<float>(Measure->Measure(S, Font).X) + 28.f; };
	float Widest = FMath::Max(Width(UJamHudWidget::ButtonsBadgeLine(true)), Width(UJamHudWidget::ButtonsBadgeLine(false)));
	FJamBindings L = FJamBindings::Defaults();
	L.Rebind(EJamVerb::CrossCheck, EKeys::BackSpace);
	L.Rebind(EJamVerb::Shoot, EKeys::BackSpace);   // a two-role key with a long name
	L.Rebind(EJamVerb::Check, EKeys::CapsLock);
	L.Rebind(EJamVerb::Pass, EKeys::CapsLock);
	UJamInputMap::Get()->ApplyBindings(L);
	const FString LongWith = UJamHudWidget::ButtonsBadgeLine(true), LongWithout = UJamHudWidget::ButtonsBadgeLine(false);
	Widest = FMath::Max(Widest, FMath::Max(Width(LongWith), Width(LongWithout)));
	AddInfo(FString::Printf(TEXT("JAMCTRL badge widest %.0f of %.0f ('%s')"), Widest, UJamHudWidget::ButtonsBadgeMaxW, *LongWithout));
	TestTrue(TEXT("the badge fits its corner at 1280 x 720 (the 1920 x 1080 layout units), long names too"), Widest > 0.f && Widest <= UJamHudWidget::ButtonsBadgeMaxW);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsPauseCard, "Jam.Controls.FromThePauseCard", JamControlsTests::Flags)
bool FJamControlsPauseCard::RunTest(const FString&)
{
	const FJamSettings Fresh;
	FJamSettings::SetOverrideForTest(&Fresh);
	ON_SCOPE_EXIT { FJamSettings::SetOverrideForTest(nullptr); };
	if (!GEngine || !FSlateApplication::IsInitialized()) { AddInfo(TEXT("no Slate: skipped")); return true; }
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, TEXT("JamControlsPauseWorld"));
	GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
	ON_SCOPE_EXIT { GEngine->DestroyWorldContext(World); World->DestroyWorld(false); };
	UJamDirectorsCallWidget* P = CreateWidget<UJamDirectorsCallWidget>(World, UJamDirectorsCallWidget::StaticClass());
	if (!TestNotNull(TEXT("the pause card builds"), P)) return false;
	P->AddToRoot();
	ON_SCOPE_EXIT { P->ReleaseSlateResources(true); P->RemoveFromRoot(); };
	TSharedRef<SWidget> Slate = P->TakeWidget();
	TestNotNull(TEXT("the pause card carries the CONTROLS screen"), P->GetControlsScreen());
	P->ShowControls();
	TestTrue(TEXT("CONTROLS opens on the pause card"), P->IsOnControls());
	Slate->SlatePrepass(1.f);
	const FVector2D Size = Slate->GetDesiredSize();
	UJamControlsWidget* C = P->GetControlsScreen();
	const FVector2D CSize = C ? FVector2D(C->TakeWidget()->GetDesiredSize()) : FVector2D::ZeroVector;
	AddInfo(FString::Printf(TEXT("JAMCTRL pause card with CONTROLS %.0f x %.0f; the screen %.0f x %.0f (card inner %.0f)"), Size.X, Size.Y, CSize.X, CSize.Y, UJamDirectorsCallWidget::PauseCardInnerWidth));
	TestTrue(TEXT("the CONTROLS screen fits the card's inner width"), CSize.X > 100.f && CSize.X <= UJamDirectorsCallWidget::PauseCardInnerWidth);
	TestTrue(TEXT("the card fits 1080 high"), Size.Y > 100.f && Size.Y <= 1080.f);
	if (C) C->OnBack.Broadcast();
	TestFalse(TEXT("BACK returns to the pause menu"), P->IsOnControls());
	return true;
}

// ---------------------------------------------------------------- lane A199 (evolution 3): the moment the puck changes hands, tap against hold, the screen's edges

namespace JamControlsTests
{
	// One phase of a scripted hand: the possession facts the step reads, and whether the key is down.
	struct FPhase { const TCHAR* Name; int32 Frames; bool bWith; bool bCarrying; bool bInReach; bool bDown; };
	// What the press did in each phase: frames of each level, edges, and whether a level fell while the puck was ours and in reach after the first phase
	// (the sim fires a held shot / pass on that fall when it holds a charge: "the release shoots").
	struct FPhaseOut { int32 Shoot = 0, Pass = 0, Cross = 0, Lift = 0, Check = 0, ShootPress = 0, PassPress = 0; bool bFireOnFall = false; };

	TArray<FPhaseOut> RunHand(const FKey& Key, const TArray<FPhase>& Phases, bool bRule)
	{
		const FJamBindings D = FJamBindings::Defaults();
		FJamButtonResolver R;
		TArray<FPhaseOut> Out;
		bool bWasDown = false, bShootUp = false, bPassUp = false;
		for (int32 P = 0; P < Phases.Num(); ++P)
		{
			const FPhase& Ph = Phases[P];
			FPhaseOut O;
			for (int32 F = 0; F < Ph.Frames; ++F)
			{
				if (Ph.bDown && !bWasDown) R.Press(Key);
				if (!Ph.bDown && bWasDown) R.Release(Key);
				bWasDown = Ph.bDown;
				FJamButtonContext C;
				C.bWith = Ph.bWith; C.bCarrying = Ph.bCarrying; C.bInReach = Ph.bInReach; C.bDeadAcrossChange = bRule;
				const FJamButtonVerbs V = R.Step(C, D);
				O.Shoot += V.bShootHeld; O.Pass += V.bPassHeld; O.Cross += V.bCrossHeld; O.Lift += V.bLift; O.Check += V.bCheck;
				O.ShootPress += V.bShootPress; O.PassPress += V.bPassPress;
				if (P > 0 && ((bShootUp && !V.bShootHeld) || (bPassUp && !V.bPassHeld)) && Ph.bInReach && Ph.bWith) O.bFireOnFall = true;
				bShootUp = V.bShootHeld; bPassUp = V.bPassHeld;
			}
			Out.Add(O);
		}
		return Out;
	}

	FString Cell(const FPhaseOut& O)
	{
		TArray<FString> P;
		if (O.Shoot) P.Add(FString::Printf(TEXT("shoot-level %d"), O.Shoot));
		if (O.Pass) P.Add(FString::Printf(TEXT("pass-level %d"), O.Pass));
		if (O.Cross) P.Add(FString::Printf(TEXT("brace-hold %d"), O.Cross));
		if (O.Lift) P.Add(FString::Printf(TEXT("cross-check %d"), O.Lift));
		if (O.Check) P.Add(FString::Printf(TEXT("check %d"), O.Check));
		if (O.ShootPress) P.Add(TEXT("shot press"));
		if (O.PassPress) P.Add(TEXT("pass press"));
		if (O.bFireOnFall) P.Add(TEXT("RELEASE FIRES"));
		return P.Num() ? FString::Join(P, TEXT(", ")) : FString(TEXT("nothing"));
	}

	// The staged skater carries: the puck on his stick side ahead of him at his speed until he touches it (HokejAcceptDrill's staging).
	bool GiveTo(UJamSim* Sim, int32 Who, TArray<FJamInput>& In, UJamPlayerInput* Pi, int32 Me)
	{
		for (int32 F = 0; F < 40 && Sim->CarrierIndex() != Who; ++F)
		{
			const FJamSkaterState& T = Sim->GetSkaters()[Who];
			Sim->SetPuck(T.Pos + T.Forward() * 0.45, T.Vel);
			if (Pi) { In[Me] = FJamInput(); JamControls::ApplyButtons(*Pi, *Sim, Me, In[Me]); }
			Sim->Step(In);
		}
		return Sim->CarrierIndex() == Who;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsHeldAcrossChange, "Jam.Controls.HeldAcrossChange", JamControlsTests::Flags)
bool FJamControlsHeldAcrossChange::RunTest(const FString&)
{
	// Item 1: scripted presses across a change of possession, per button, under main's rule (jam.ctrl.HeldAcrossChange 0) and the shipped one (1).
	struct FScene { int32 Id; const TCHAR* Name; bool bLeft; TArray<FPhase> Phases; };
	const TArray<FScene> Scenes = {
		{ 1, TEXT("held braced, takes the puck in the hit, loses it, still held; then let go and pressed without"), true, {
			{ TEXT("held without"), 20, false, false, false, true }, { TEXT("takes the puck"), 20, true, true, true, true },
			{ TEXT("loses it, still held"), 20, false, false, false, true }, { TEXT("let go"), 2, false, false, false, false }, { TEXT("pressed again"), 3, false, false, false, true } } },
		{ 2, TEXT("shot loading, stripped (out of reach), still held, puck back on his stick, let go"), true, {
			{ TEXT("loading with the puck"), 20, true, true, true, true }, { TEXT("stripped, held"), 10, false, false, false, true },
			{ TEXT("puck back, held"), 10, true, true, true, true }, { TEXT("let go"), 3, true, true, true, false } } },
		{ 3, TEXT("shot loading, stripped but the puck still in his reach, then out of it"), true, {
			{ TEXT("loading with the puck"), 20, true, true, true, true }, { TEXT("stripped, in reach"), 5, false, false, true, true },
			{ TEXT("out of reach, held"), 10, false, false, false, true }, { TEXT("let go"), 3, false, false, false, false } } },
		{ 4, TEXT("check called, passed to (our pass in flight, then on his stick), still held, let go, pressed again"), false, {
			{ TEXT("held without"), 5, false, false, false, true }, { TEXT("our pass in flight"), 10, true, false, false, true },
			{ TEXT("on his stick, held"), 20, true, true, true, true }, { TEXT("let go"), 2, true, true, true, false }, { TEXT("pressed again"), 3, true, true, true, true } } },
		{ 5, TEXT("saucer loading, stripped (out of reach), still held, puck back, let go"), false, {
			{ TEXT("loading with the puck"), 12, true, true, true, true }, { TEXT("stripped, held"), 10, false, false, false, true },
			{ TEXT("puck back, held"), 10, true, true, true, true }, { TEXT("let go"), 3, true, true, true, false } } } };
	for (const bool bPad : { false, true })
	{
		for (const FScene& Sc : Scenes)
		{
			const FKey Key = Sc.bLeft ? (bPad ? EKeys::Gamepad_RightTriggerAxis : EKeys::LeftMouseButton) : (bPad ? EKeys::Gamepad_RightShoulder : EKeys::RightMouseButton);
			const TArray<FPhaseOut> Before = RunHand(Key, Sc.Phases, false), After = RunHand(Key, Sc.Phases, true);
			const FString K = UJamInputMap::KeyName(Key);
			AddInfo(FString::Printf(TEXT("JAMCTRL A199 table | %s | %s |"), *K, Sc.Name));
			for (int32 P = 0; P < Sc.Phases.Num(); ++P)
			{
				AddInfo(FString::Printf(TEXT("JAMCTRL A199 table |  | %s | before: %s | after: %s |"), Sc.Phases[P].Name, *Cell(Before[P]), *Cell(After[P])));
			}
			switch (Sc.Id)
			{
			case 1:
				TestTrue(*FString::Printf(TEXT("%s: main re-braced after the loss (the leak measured)"), *K), Before[2].Cross == 20);
				TestTrue(*FString::Printf(TEXT("%s: braced while held without"), *K), After[0].Cross == 20 && After[0].Lift == 1);
				TestTrue(*FString::Printf(TEXT("%s: taking the puck ends the brace, no shot"), *K), After[1].Cross == 0 && After[1].Shoot == 0 && After[1].ShootPress == 0);
				TestTrue(*FString::Printf(TEXT("%s: losing it again under the same press does nothing"), *K), After[2].Cross == 0 && After[2].Lift == 0);
				TestTrue(*FString::Printf(TEXT("%s: the next press is a cross-check again"), *K), After[4].Lift == 1 && After[4].Cross == 3);
				break;
			case 2:
				TestTrue(*FString::Printf(TEXT("%s: main shot on the release with the old charge"), *K), Before[3].bFireOnFall);
				TestTrue(*FString::Printf(TEXT("%s: stripped, the held shot does nothing (no level, no cross-check)"), *K), After[1].Shoot == 0 && After[1].Lift == 0 && After[1].Cross == 0);
				TestTrue(*FString::Printf(TEXT("%s: the puck back under the same press: nothing, and the release fires nothing"), *K), After[2].Shoot == 0 && !After[3].bFireOnFall);
				break;
			case 3:
				TestTrue(*FString::Printf(TEXT("%s: stripped with the puck in reach: the dead level is held up (its fall would fire at the loose puck)"), *K), After[1].Shoot == 5);
				TestTrue(*FString::Printf(TEXT("%s: out of reach: the level falls and stays down"), *K), After[2].Shoot == 0 && After[3].Shoot == 0);
				break;
			case 4:
				TestTrue(*FString::Printf(TEXT("%s: passed to with a check held: one check on the press, no pass in either rule"), *K), Before[0].Check == 1 && After[0].Check == 1
					&& Before[2].Pass == 0 && After[2].Pass == 0 && After[1].Check == 0 && After[2].Check == 0 && !After[3].bFireOnFall);
				TestTrue(*FString::Printf(TEXT("%s: the next press passes"), *K), After[4].PassPress == 1 && After[4].Pass == 3);
				break;
			default:
				TestTrue(*FString::Printf(TEXT("%s: main passed on the release with the old charge"), *K), Before[3].bFireOnFall);
				TestTrue(*FString::Printf(TEXT("%s: stripped, the held pass does nothing; the release fires nothing"), *K), After[1].Pass == 0 && After[1].Check == 0 && After[2].Pass == 0 && !After[3].bFireOnFall);
				break;
			}
		}
	}
	IConsoleVariable* Rule = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.ctrl.HeldAcrossChange"));
	TestTrue(TEXT("jam.ctrl.HeldAcrossChange ships 1"), Rule && Rule->GetInt() == 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsChangeInTheSim, "Jam.Controls.HeldAcrossChangeInTheSim", JamControlsTests::Flags)
bool FJamControlsChangeInTheSim::RunTest(const FString&)
{
	// Item 1 in the sim: the human loads a shot with LMB, an opponent takes the puck, it comes back to his stick, he lets go. Shots fired by the release:
	// main's rule (0) against the shipped (1). The input goes through ApplyButtons from a real UJamPlayerInput, as both game modes send it.
	FJamGameConfig Cfg;
	if (!LoadRegular(*this, Cfg)) return false;
	FRestoreBindings Restore;
	UJamInputMap::Get()->ApplyBindings(FJamBindings::Defaults());
	IConsoleVariable* Rule = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.ctrl.HeldAcrossChange"));
	if (!TestNotNull(TEXT("jam.ctrl.HeldAcrossChange"), Rule)) return false;
	const int32 Was = Rule->GetInt();
	ON_SCOPE_EXIT { Rule->Set(Was, ECVF_SetByCode); };
	int32 ShotsBy[2] = { -1, -1 };
	for (int32 Mode = 0; Mode < 2; ++Mode)
	{
		Rule->Set(Mode, ECVF_SetByCode);
		Cfg.Skater.bChargeDiesOnLoss = false;   // lane A203: the mapping's rule alone, against the sim's neutral (with the sim's rule on, both arms shoot nothing)
		UJamSim* Sim = NewObject<UJamSim>();
		Sim->Init(Cfg, 1);
		Sim->SkipFaceoff();
		const int32 Me = Sim->GetHumanIndex();
		int32 Opp = -1;
		for (const FJamSkaterState& S : Sim->GetSkaters()) if (S.Team != Sim->GetSkaters()[Me].Team) { Opp = S.Index; break; }
		if (!TestTrue(TEXT("an opponent"), Opp >= 0)) return false;
		for (const FJamSkaterState& S : Sim->GetSkaters()) if (S.Index != Me && S.Index != Opp) Sim->SetSkater(S.Index, FVector2D(-25.0, -10.0 + S.Index), FVector2D::ZeroVector, 0.0);
		Sim->SetSkater(Me, FVector2D(0.0, 0.0), FVector2D::ZeroVector, 0.0);
		Sim->SetSkater(Opp, FVector2D(0.0, 8.0), FVector2D::ZeroVector, 0.0);
		TArray<FJamInput> In;
		In.Init(FJamInput(), Sim->GetSkaters().Num());
		UJamPlayerInput* Pi = NewObject<UJamPlayerInput>();
		if (!TestTrue(TEXT("the human carries"), GiveTo(Sim, Me, In, Pi, Me))) return false;
		Pi->DebugButton(EKeys::LeftMouseButton, true);
		auto Steps = [&](int32 N) { for (int32 F = 0; F < N; ++F) { In[Me] = FJamInput(); JamControls::ApplyButtons(*Pi, *Sim, Me, In[Me]); Sim->Step(In); } };
		Steps(15);
		const int32 Charge = Sim->GetSkaters()[Me].ChargeFrames;
		const bool bTaken = GiveTo(Sim, Opp, In, Pi, Me);
		Steps(3);
		const bool bBack = GiveTo(Sim, Me, In, Pi, Me);
		Steps(3);
		const int32 Shots0 = Sim->GetSkaters()[Me].Shots;
		Pi->DebugButton(EKeys::LeftMouseButton, false);
		Steps(10);
		ShotsBy[Mode] = Sim->GetSkaters()[Me].Shots - Shots0;
		AddInfo(FString::Printf(TEXT("JAMCTRL A199 sim | rule %d | charge %d before the take | taken %d | back %d | shots by the release %d |"), Mode, Charge, bTaken ? 1 : 0, bBack ? 1 : 0, ShotsBy[Mode]));
		TestTrue(TEXT("the staging: charged, taken, back"), Charge > 4 && bTaken && bBack);
	}
	TestEqual(TEXT("main's rule: the release shot the old charge"), ShotsBy[0], 1);
	TestEqual(TEXT("the shipped rule: the release after the change shoots nothing"), ShotsBy[1], 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsTapHold, "Jam.Controls.TapAgainstHold", JamControlsTests::Flags)
bool FJamControlsTapHold::RunTest(const FString&)
{
	// Item 2: per press length, what the sim reads. The pass: the charge frames the sim counts (one per level frame with the puck in reach; a saucer from
	// pass.saucer.half_frames). The cross-check: the frames braced in the sim (bCrossHold from the mapping; braced from UJamSim::CrossHoldTapFrames of it).
	FJamGameConfig Cfg;
	if (!LoadRegular(*this, Cfg)) return false;
	FRestoreBindings Restore;
	UJamInputMap::Get()->ApplyBindings(FJamBindings::Defaults());
	IConsoleVariable* Sau = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.ctrl.SaucerHoldFrames"));
	IConsoleVariable* Bra = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.ctrl.BraceHoldFrames"));
	if (!TestTrue(TEXT("the two cvars"), Sau && Bra)) return false;
	TestEqual(TEXT("jam.ctrl.SaucerHoldFrames ships 13"), Sau->GetInt(), 13);
	TestEqual(TEXT("jam.ctrl.BraceHoldFrames ships 13"), Bra->GetInt(), 13);
	const int32 SauWas = Sau->GetInt(), BraWas = Bra->GetInt();
	ON_SCOPE_EXIT { Sau->Set(SauWas, ECVF_SetByCode); Bra->Set(BraWas, ECVF_SetByCode); };
	const int32 Half = Cfg.Pass.SaucerHalfFrames;
	const int32 Lengths[] = { 1, 3, 6, 8, 9, 10, 12, 13, 16, 20 };
	for (const int32 Shipped : { 0, 1 })
	{
		Sau->Set(Shipped ? 13 : 0, ECVF_SetByCode);
		Bra->Set(Shipped ? 13 : 0, ECVF_SetByCode);
		for (const int32 N : Lengths)
		{
			// The pass: the resolver with the delay ApplyButtons takes from the cvar, the puck in reach throughout.
			FJamButtonResolver R;
			FJamButtonContext C;
			C.bWith = C.bCarrying = C.bInReach = true;
			C.SaucerDelaySteps = FMath::Max(0, JamControls::SaucerHoldFrames(Half) - Half);
			R.Press(EKeys::RightMouseButton);
			int32 Charge = 0, Fired = -1;
			bool bUp = false;
			for (int32 F = 1; F <= N + 4; ++F)
			{
				if (F == N + 1) R.Release(EKeys::RightMouseButton);
				const FJamButtonVerbs V = R.Step(C, FJamBindings::Defaults());
				if (V.bPassHeld) Charge = FMath::Min(Charge + 1, FMath::Max(Half, 1));
				if (bUp && !V.bPassHeld && Fired < 0) Fired = F;
				bUp = V.bPassHeld;
			}
			const bool bSaucer = Charge >= Half;
			// The cross-check in the sim: an isolated skater, the button held N frames through ApplyButtons.
			UJamSim* Sim = NewObject<UJamSim>();
			Sim->Init(Cfg, 1);
			Sim->SkipFaceoff();
			const int32 Me = Sim->GetHumanIndex();
			for (const FJamSkaterState& S : Sim->GetSkaters()) if (S.Index != Me) Sim->SetSkater(S.Index, FVector2D(-25.0, -10.0 + S.Index), FVector2D::ZeroVector, 0.0);
			Sim->SetSkater(Me, FVector2D(5.0, 0.0), FVector2D::ZeroVector, 0.0);
			Sim->SetPuck(FVector2D(20.0, 8.0), FVector2D::ZeroVector);
			TArray<FJamInput> In;
			In.Init(FJamInput(), Sim->GetSkaters().Num());
			UJamPlayerInput* Pi = NewObject<UJamPlayerInput>();
			Pi->DebugButton(EKeys::LeftMouseButton, true);
			int32 Lifts = 0, Braced = 0, FirstBraced = -1;
			for (int32 F = 1; F <= N + 6; ++F)
			{
				if (F == N + 1) Pi->DebugButton(EKeys::LeftMouseButton, false);
				In[Me] = FJamInput();
				JamControls::ApplyButtons(*Pi, *Sim, Me, In[Me]);
				Lifts += In[Me].bLift;
				Sim->Step(In);
				if (Sim->GetSkaters()[Me].bCrossBraced) { ++Braced; if (FirstBraced < 0) FirstBraced = F; }
			}
			AddInfo(FString::Printf(TEXT("JAMCTRL A199 hold | %s | %d frames | pass: charge %d, %s, fires on step %d | cross-check: presses %d, braced frames %d (from frame %d) |"),
				Shipped ? TEXT("shipped 13/13") : TEXT("neutral 0/0 (sim 8/10)"), N, Charge, bSaucer ? TEXT("SAUCER") : TEXT("flat"), Fired, Lifts, Braced, FirstBraced));
			const int32 SaucerFrom = Shipped ? 13 : Half, BraceFrom = Shipped ? 13 : UJamSim::CrossHoldTapFrames;
			TestEqual(*FString::Printf(TEXT("%d frames: saucer iff held %d or more"), N, SaucerFrom), bSaucer, N >= SaucerFrom);
			TestEqual(*FString::Printf(TEXT("%d frames: one cross-check press"), N), Lifts, 1);
			TestEqual(*FString::Printf(TEXT("%d frames: braced iff held %d or more (from that frame)"), N, BraceFrom), FirstBraced, N >= BraceFrom ? BraceFrom : -1);
			// A press let go inside the level's late start sends its one step on the release's step: it fires one step later (17 ms).
			const int32 Late = C.SaucerDelaySteps > 0 && N <= C.SaucerDelaySteps ? 1 : 0;
			TestTrue(*FString::Printf(TEXT("%d frames: the pass fires on the release's step (+%d), never before it"), N, Late), Fired == N + 1 + Late);
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsScreenEdges, "Jam.Controls.ScreenEdges", JamControlsTests::Flags)
bool FJamControlsScreenEdges::RunTest(const FString&)
{
	// Item 4: a key bound twice swaps and says so; Esc cancels; a key repeat never binds; a quick second click (Slate's double-click) is a real press;
	// RESET TO DEFAULTS asks first.
	const FJamSettings Fresh;
	FJamSettings::SetOverrideForTest(&Fresh);
	ON_SCOPE_EXIT { FJamSettings::SetOverrideForTest(nullptr); };
	FRestoreBindings Restore;
	UJamInputMap::Get()->ApplyBindings(FJamBindings::Defaults());
	UWorld* World = nullptr;
	if (GEngine && FSlateApplication::IsInitialized())
	{
		World = UWorld::CreateWorld(EWorldType::Game, false, TEXT("JamControlsEdgesWorld"));
		GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
	}
	ON_SCOPE_EXIT { if (World) { GEngine->DestroyWorldContext(World); World->DestroyWorld(false); } };
	UJamControlsWidget* W = UJamSettingsWidget::MakeControlsScreen(World);
	if (!TestNotNull(TEXT("the screen"), W)) return false;
	W->AddToRoot();
	ON_SCOPE_EXIT { W->ReleaseSlateResources(true); W->RemoveFromRoot(); };
	TSharedRef<SWidget> Slate = W->TakeWidget();
	W->Refresh();
	// A key bound twice: SPRINT onto SPACE (the poke's): the poke gets SHIFT, and the line says so.
	W->BeginListen(EJamVerb::Sprint, false);
	TestTrue(TEXT("SPACE taken"), W->TakeListenPress(EKeys::SpaceBar, false));
	const FJamBindings& B1 = UJamInputMap::Get()->GetBindings();
	AddInfo(FString::Printf(TEXT("JAMCTRL A199 edges swap: %s"), *W->GetStatusLine()));
	TestTrue(TEXT("swapped: sprint SPACE, poke SHIFT, valid"), B1[EJamVerb::Sprint].Keys[0] == EKeys::SpaceBar && B1[EJamVerb::Poke].Keys[0] == EKeys::LeftShift && B1.IsValid());
	TestTrue(TEXT("the line says the swap"), W->GetStatusLine().Contains(TEXT("It was POKE's")) && W->GetStatusLine().Contains(TEXT("moved to SHIFT")));
	// A repeat never binds: Enter opened the listen and is still held.
	W->BeginListen(EJamVerb::Spin, false);
	TestTrue(TEXT("the repeat is swallowed"), W->TakeListenPress(EKeys::Enter, true));
	TestTrue(TEXT("still listening, the spin unchanged"), W->IsListening() && UJamInputMap::Get()->GetBindings()[EJamVerb::Spin].Keys[0] == EKeys::E);
	TestTrue(TEXT("Esc cancels"), W->TakeListenPress(EKeys::Escape, false) && !W->IsListening() && W->GetStatusLine().Contains(TEXT("Cancelled")));
	// The quick second click: the mouse button arrives through the double-click handler.
	W->BeginListen(EJamVerb::Ability, false);
	{
		const FPointerEvent Dbl(0, FVector2D(10, 10), FVector2D(10, 10), TSet<FKey>({ EKeys::ThumbMouseButton2 }), EKeys::ThumbMouseButton2, 0.f, FModifierKeysState());
		const FReply Rep = W->NativeOnMouseButtonDoubleClick(FGeometry(), Dbl);
		TestTrue(TEXT("the double-click is handled"), Rep.IsEventHandled());
	}
	TestTrue(TEXT("mouse 5 bound by a quick second click"), !W->IsListening() && UJamInputMap::Get()->GetBindings()[EJamVerb::Ability].Keys[0] == EKeys::ThumbMouseButton2);
	// A mouse down right after BeginListen is taken (no grace that eats it).
	W->BeginListen(EJamVerb::Deke, false);
	{
		const FPointerEvent Down(0, FVector2D(10, 10), FVector2D(10, 10), TSet<FKey>({ EKeys::MiddleMouseButton }), EKeys::MiddleMouseButton, 0.f, FModifierKeysState());
		W->NativeOnMouseButtonDown(FGeometry(), Down);
	}
	TestTrue(TEXT("MMB bound on the first tick of listening"), !W->IsListening() && UJamInputMap::Get()->GetBindings()[EJamVerb::Deke].Keys[0] == EKeys::MiddleMouseButton);
	// RESET asks first.
	const FJamBindings Changed = UJamInputMap::Get()->GetBindings();
	W->RequestReset();
	TestTrue(TEXT("the first press arms, changes nothing"), W->IsResetArmed() && UJamInputMap::Get()->GetBindings() == Changed && W->GetStatusLine().Contains(TEXT("CONFIRM RESET")));
	W->BeginListen(EJamVerb::Poke, false);
	TestFalse(TEXT("a listen disarms it"), W->IsResetArmed());
	W->TakeListenPress(EKeys::Escape, false);
	W->RequestReset();
	W->RequestReset();
	TestTrue(TEXT("the second press resets the game and the profile"), !W->IsResetArmed() && UJamInputMap::Get()->GetBindings() == FJamBindings::Defaults()
		&& FJamBindings::FromSaved(FJamSettings::Load().Controls) == FJamBindings::Defaults());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsAtomicSave, "Jam.Controls.AtomicSave", JamControlsTests::Flags)
bool FJamControlsAtomicSave::RunTest(const FString&)
{
	// Item 4: the profile is written through a temp file and a rename. A failed write (a kill between the write and the rename) leaves the old file whole.
	const FString Dir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Automation"), TEXT("A199"));
	const FString Path = FPaths::Combine(Dir, TEXT("Bindings.ini"));
	IFileManager::Get().Delete(*Path, false, false, true);
	const FString One = TEXT("[Jam.Controls]\nShoot=LeftMouseButton|Gamepad_RightTriggerAxis\n");
	const FString Two = TEXT("[Jam.Controls]\nShoot=E|Gamepad_RightTriggerAxis\nPass=RightMouseButton|Gamepad_RightShoulder\n");
	TestTrue(TEXT("the first write"), JamControls::SaveFileAtomically(Path, One));
	FString Read;
	TestTrue(TEXT("read back whole"), FFileHelper::LoadFileToString(Read, *Path) && Read == One);
	JamControls::FailNextAtomicSaveForTest();
	TestFalse(TEXT("a write killed before its rename reports failure"), JamControls::SaveFileAtomically(Path, Two));
	TestTrue(TEXT("the old file is untouched"), FFileHelper::LoadFileToString(Read, *Path) && Read == One);
	TestTrue(TEXT("a write over an existing file replaces it whole"), JamControls::SaveFileAtomically(Path, Two) && FFileHelper::LoadFileToString(Read, *Path) && Read == Two);
	TArray<FString> Left;
	IFileManager::Get().FindFiles(Left, *FPaths::Combine(Dir, TEXT("*.tmp")), true, false);
	TestEqual(TEXT("no temp file left beside it"), Left.Num(), 0);
	// The hook takes only the profile's ini: another file's save is left to the engine.
	int32 Count = 0;
	FCoreDelegates::TSPreSaveConfigFileDelegate().Broadcast(*Path, Two, Count);
	TestEqual(TEXT("the pre-save hook ignores a file that is not GameUserSettings.ini"), Count, 0);
	IFileManager::Get().DeleteDirectory(*Dir, false, true);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsPadScheme, "Jam.Controls.PadScheme", JamControlsTests::Flags)
bool FJamControlsPadScheme::RunTest(const FString&)
{
	// Item 3: every action of the mouse scheme has a pad button, with the same possession rule; the strip and the badge name the pad's buttons when
	// the pad was touched last; a stick nudge never binds; the badge shows the hold (SAUCER / BRACED).
	FRestoreBindings Restore;
	UJamInputMap::Get()->ApplyBindings(FJamBindings::Defaults());
	ON_SCOPE_EXIT { UJamInputMap::NoteInputDevice(false); };
	const FJamBindings D = FJamBindings::Defaults();
	for (int32 I = 0; I < FJamBindings::NumVerbs; ++I)
	{
		const EJamVerb V = static_cast<EJamVerb>(I);
		if (V >= EJamVerb::SkateForward && V <= EJamVerb::SkateRight) continue;   // the left stick skates
		TestTrue(*FString::Printf(TEXT("%s has a pad button (%s)"), FJamBindings::VerbId(V), *UJamInputMap::VerbKeysText(V, true)), D[V].Pads.Num() > 0);
	}
	// The same rule on the pad's two-role buttons as on the mouse's: the same verbs, step for step, through a pickup and a loss under a held press.
	for (const TPair<FKey, FKey>& Pair : { TPair<FKey, FKey>(EKeys::LeftMouseButton, EKeys::Gamepad_RightTriggerAxis), TPair<FKey, FKey>(EKeys::RightMouseButton, EKeys::Gamepad_RightShoulder) })
	{
		FJamButtonResolver M, P;
		M.Press(Pair.Key); P.Press(Pair.Value);
		int32 Same = 0, N = 0;
		const bool Script[] = { false, false, true, true, true, false, false, true };
		for (const bool bWith : Script)
		{
			const FJamButtonVerbs A = M.Step(bWith, D), B = P.Step(bWith, D);
			Same += (A.bShootHeld == B.bShootHeld && A.bPassHeld == B.bPassHeld && A.bCrossHeld == B.bCrossHeld && A.bLift == B.bLift && A.bCheck == B.bCheck) ? 1 : 0;
			++N;
		}
		TestEqual(*FString::Printf(TEXT("%s and %s do the same on every step"), *UJamInputMap::KeyName(Pair.Key), *UJamInputMap::KeyName(Pair.Value)), Same, N);
	}
	// The words follow the scheme in use.
	UJamPlayerInput* Pi = NewObject<UJamPlayerInput>();
	Pi->DebugButton(EKeys::Gamepad_RightShoulder, true);
	TestTrue(TEXT("a pad press makes the pad the last device"), UJamInputMap::IsPadLast());
	const FString PadStrip = UJamInputMap::HintStripText();
	AddInfo(FString::Printf(TEXT("JAMCTRL A199 pad strip: %s"), *PadStrip));
	TestTrue(TEXT("the pad strip: L-STICK, R-STICK, RT, RB, A, R3, LT"), PadStrip.Contains(TEXT("SKATE  L-STICK")) && PadStrip.Contains(TEXT("AIM  R-STICK"))
		&& PadStrip.Contains(TEXT("SHOOT / CROSS-CHECK  RT")) && PadStrip.Contains(TEXT("PASS / CHECK  RB")) && PadStrip.Len() <= UJamInputMap::HintStripMaxChars);
	TestEqual(TEXT("the pad badge"), UJamHudWidget::ButtonsBadgeLine(true), FString(TEXT("RT  SHOOT     RB  PASS")));
	FJamBindings R = D;
	R.Rebind(EJamVerb::Poke, EKeys::Gamepad_FaceButton_Left);   // X: the pad's poke (the pass's second binding gives it up)
	UJamInputMap::Get()->ApplyBindings(R);
	TestTrue(TEXT("a rebound pad button shows in the strip"), UJamInputMap::HintStripText().Contains(TEXT("POKE  X")));
	UJamInputMap::Get()->ApplyBindings(D);
	Pi->DebugButton(EKeys::Gamepad_RightShoulder, false);
	Pi->DebugButton(EKeys::LeftMouseButton, true);
	TestFalse(TEXT("a mouse press gives the keys back"), UJamInputMap::IsPadLast());
	TestEqual(TEXT("the mouse badge"), UJamHudWidget::ButtonsBadgeLine(false), FString(TEXT("LMB  CROSS-CHECK     RMB  CHECK")));
	Pi->DebugButton(EKeys::LeftMouseButton, false);
	// A stick nudge never binds.
	FJamBindings S = D;
	TestFalse(TEXT("the left stick's up is the game's own"), S.Rebind(EJamVerb::Poke, EKeys::Gamepad_LeftStick_Up));
	TestFalse(TEXT("the right stick's left is the game's own"), S.Rebind(EJamVerb::Poke, EKeys::Gamepad_RightStick_Left));
	// The badge shows the hold.
	FJamButtonVerbs H;
	H.PassHoldSteps = 12;
	TestEqual(TEXT("a pass held 12 frames still reads PASS"), JamControls::BadgeLineHeld(true, H, 13, 13), FString(TEXT("LMB  SHOOT     RMB  PASS")));
	H.PassHoldSteps = 13;
	TestEqual(TEXT("from the 13th frame it reads SAUCER"), JamControls::BadgeLineHeld(true, H, 13, 13), FString(TEXT("LMB  SHOOT     RMB  SAUCER")));
	H = FJamButtonVerbs();
	H.CrossHoldSteps = 13;
	TestEqual(TEXT("a cross-check held 13 frames reads BRACED"), JamControls::BadgeLineHeld(false, H, 13, 13), FString(TEXT("LMB  BRACED     RMB  CHECK")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsSavedNeverUnbinds, "Jam.Controls.SavedEntryNeverUnbinds", JamControlsTests::Flags)
bool FJamControlsSavedNeverUnbinds::RunTest(const FString&)
{
	// A198's FAULT 1: a saved entry with no usable key keeps the verb's default; a table that leaves a verb unreachable is refused whole for the defaults.
	const FJamBindings D = FJamBindings::Defaults();
	{
		TMap<FString, FString> S;
		S.Add(TEXT("Shoot"), TEXT("Mouse_Left|RightTrigger"));   // unknown name, a reserved (digital trigger) key
		const FJamBindings B = FJamBindings::FromSaved(S);
		TestTrue(TEXT("Shoot=Mouse_Left|RightTrigger: the shot keeps LMB / RT"), B[EJamVerb::Shoot] == D[EJamVerb::Shoot] && B.IsPlayable());
	}
	{
		TMap<FString, FString> S;
		S.Add(TEXT("Pass"), TEXT("<garbage>"));
		TestTrue(TEXT("Pass=<garbage> (no '|'): the pass keeps its default"), FJamBindings::FromSaved(S)[EJamVerb::Pass] == D[EJamVerb::Pass]);
	}
	{
		TMap<FString, FString> S;
		for (int32 I = 0; I < FJamBindings::NumVerbs; ++I) S.Add(FJamBindings::VerbId(static_cast<EJamVerb>(I)), TEXT("|"));
		TestTrue(TEXT("every verb saved as '|': the defaults whole"), FJamBindings::FromSaved(S) == D);
	}
	{
		TMap<FString, FString> S;
		S.Add(TEXT("Shoot"), TEXT("Escape,Mouse2D|Gamepad_DPad_Up"));   // only reserved keys
		S.Add(TEXT("Spin"), TEXT("G|"));                                  // a usable entry beside it stays
		const FJamBindings B = FJamBindings::FromSaved(S);
		TestTrue(TEXT("reserved-only entry: default; the usable entry beside it is kept"), B[EJamVerb::Shoot] == D[EJamVerb::Shoot] && B[EJamVerb::Spin].Keys == TArray<FKey>({ EKeys::G })
			&& B[EJamVerb::Spin].Pads.Num() == 0 && B.IsPlayable());
	}
	{
		// A clash refuses the table whole.
		TMap<FString, FString> S;
		S.Add(TEXT("Poke"), TEXT("LeftMouseButton|"));
		TestTrue(TEXT("a clash: the defaults whole"), FJamBindings::FromSaved(S) == D);
	}
	{
		FJamBindings E = D;
		E[EJamVerb::Check].Keys.Reset();
		E[EJamVerb::Check].Pads.Reset();
		FString Why;
		TestFalse(TEXT("IsPlayable names a verb with nothing"), E.IsPlayable(&Why));
		AddInfo(FString::Printf(TEXT("JAMCTRL A199 unplayable: %s"), *Why));
		TestTrue(TEXT("the words name the verb"), Why.Contains(TEXT("Check")));
		// Saved round trip of a table with the check unbound is refused whole.
		TestTrue(TEXT("a saved table with a verb unbound comes back as the defaults"), FJamBindings::FromSaved(E.ToSaved()) == D);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsRebindNeverStrips, "Jam.Controls.RebindNeverStrips", JamControlsTests::Flags)
bool FJamControlsRebindNeverStrips::RunTest(const FString&)
{
	// A198's FAULT 2 and the Director's ruling: rebinding one verb of a shared pair splits the pair (the rebound verb takes the new key, its partner keeps
	// the old one); a swap happens only with the verb that held the new key; no verb is ever left without a key or a pad button.
	const FJamBindings D = FJamBindings::Defaults();
	{
		FJamBindings B = D;
		TestTrue(TEXT("cross-check onto Down"), B.Rebind(EJamVerb::CrossCheck, EKeys::Down));
		TestTrue(TEXT("the shot keeps LMB alone (the pair splits)"), B[EJamVerb::Shoot].Keys == TArray<FKey>({ EKeys::LeftMouseButton }) && !B.Has(EJamVerb::CrossCheck, EKeys::LeftMouseButton));
		TestTrue(TEXT("skate back gives Down up and keeps S; LMB never skates"), B[EJamVerb::SkateBack].Keys == TArray<FKey>({ EKeys::S }));
		TestTrue(TEXT("the cross-check: Down first, its second bindings kept"), B[EJamVerb::CrossCheck].Keys == TArray<FKey>({ EKeys::Down, EKeys::MiddleMouseButton, EKeys::ThumbMouseButton }));
		TestTrue(TEXT("valid, playable"), B.IsValid() && B.IsPlayable());
	}
	{
		// A verb whose only key is taken and to which the old key cannot go: refused, the table unchanged.
		FJamBindings B = D;
		B[EJamVerb::Restart].Pads.Reset();   // restart: R only
		const FJamBindings Before = B;
		TestFalse(TEXT("cross-check onto R would leave restart with nothing (LMB cannot go to it beside the shot): refused"), B.Rebind(EJamVerb::CrossCheck, EKeys::R));
		TestTrue(TEXT("the table is unchanged"), B == Before);
	}
	// The property: every verb onto every key of a broad set, from the defaults and from 200 random valid tables.
	const FKey Keys[] = { EKeys::LeftMouseButton, EKeys::RightMouseButton, EKeys::MiddleMouseButton, EKeys::ThumbMouseButton, EKeys::ThumbMouseButton2, EKeys::MouseScrollUp,
		EKeys::MouseScrollDown, EKeys::Q, EKeys::E, EKeys::F, EKeys::G, EKeys::R, EKeys::W, EKeys::A, EKeys::S, EKeys::D, EKeys::X, EKeys::Z, EKeys::C, EKeys::SpaceBar,
		EKeys::LeftShift, EKeys::LeftControl, EKeys::LeftAlt, EKeys::Tab, EKeys::Up, EKeys::Down, EKeys::Left, EKeys::Right, EKeys::Enter, EKeys::Escape, EKeys::One,
		EKeys::Gamepad_RightTriggerAxis, EKeys::Gamepad_LeftTriggerAxis, EKeys::Gamepad_RightShoulder, EKeys::Gamepad_LeftShoulder, EKeys::Gamepad_FaceButton_Bottom,
		EKeys::Gamepad_FaceButton_Right, EKeys::Gamepad_FaceButton_Left, EKeys::Gamepad_FaceButton_Top, EKeys::Gamepad_LeftThumbstick, EKeys::Gamepad_RightThumbstick,
		EKeys::Gamepad_Special_Left, EKeys::Gamepad_DPad_Up, EKeys::Gamepad_LeftStick_Up };
	const int32 NK = UE_ARRAY_COUNT(Keys);
	int32 Tried = 0, Moved = 0, Bad = 0;
	auto Check = [&](const FJamBindings& From, const TCHAR* Label)
	{
		for (int32 V = 0; V < FJamBindings::NumVerbs; ++V)
		{
			for (const FKey& K : Keys)
			{
				FJamBindings T = From;
				const bool bOk = T.Rebind(static_cast<EJamVerb>(V), K);
				++Tried; Moved += bOk ? 1 : 0;
				FString Why;
				if (!T.IsValid(&Why) || !T.IsPlayable(&Why) || (!bOk && !(T == From)))
				{
					if (++Bad <= 10) AddError(FString::Printf(TEXT("%s: %s onto %s: %s"), Label, FJamBindings::VerbId(static_cast<EJamVerb>(V)), *K.ToString(), bOk ? *Why : TEXT("refused but changed")));
				}
			}
		}
	};
	Check(D, TEXT("defaults"));
	FRandomStream Rng(199);
	for (int32 T = 0; T < 200; ++T)
	{
		FJamBindings Tab = D;
		for (int32 N = 0; N < 25; ++N) Tab.Rebind(static_cast<EJamVerb>(Rng.RandHelper(FJamBindings::NumVerbs)), Keys[Rng.RandHelper(NK)]);
		if (!Tab.IsValid() || !Tab.IsPlayable()) { ++Bad; AddError(FString::Printf(TEXT("random table %d is not valid and playable after its own rebinds"), T)); continue; }
		Check(Tab, *FString::Printf(TEXT("table %d"), T));
	}
	AddInfo(FString::Printf(TEXT("JAMCTRL A199 rebind property: %d rebinds tried (defaults + 200 random tables), %d moved, %d refused, %d faults"), Tried, Moved, Tried - Moved, Bad));
	TestEqual(TEXT("after every rebinding every verb has a key or a pad button and no key serves two verbs outside a with / without pair"), Bad, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsChargeDiesOnLoss, "Jam.Controls.ChargeDiesOnLoss", JamControlsTests::Flags)
bool FJamControlsChargeDiesOnLoss::RunTest(const FString&)
{
	// Lane A203 (track K1 evolution 4, the Director's ruling on A199's ASK): game.skater.charge_dies_on_loss in the sim, at the input the sim reads (a bot's
	// and a guest's input are this same FJamInput). The human carries and holds the shot (or the pass); an opposing bot's poke takes the puck; the puck
	// comes back into his reach (and onto his blade); he lets go. Shots (passes) fired by that release: false (the neutral value) 1, true 0. With the key
	// on, the next press is a shot (a pass) again.
	FJamGameConfig Base;
	if (!LoadRegular(*this, Base)) return false;
	TestTrue(TEXT("game.skater.charge_dies_on_loss ships true (absent from hockey.regular)"), Base.Skater.bChargeDiesOnLoss);
	{ FJamGameConfig N = Base; JamBaseline::MakeNeutral(N); TestFalse(TEXT("JamBaseline::MakeNeutral sets it false"), N.Skater.bChargeDiesOnLoss); }
	for (const bool bPass : { false, true })
	{
		int32 ByRelease[2] = { -1, -1 };
		for (const bool bKey : { false, true })
		{
			FJamGameConfig Cfg = Base;
			Cfg.Skater.bChargeDiesOnLoss = bKey;
			UJamSim* Sim = NewObject<UJamSim>();
			Sim->Init(Cfg, 1);
			Sim->SkipFaceoff();
			const int32 Me = Sim->GetHumanIndex();
			int32 Opp = -1, Mate = -1;
			for (const FJamSkaterState& K : Sim->GetSkaters())
			{
				if (K.Team != Sim->GetSkaters()[Me].Team && Opp < 0) Opp = K.Index;
				if (K.Team == Sim->GetSkaters()[Me].Team && K.Index != Me && Mate < 0) Mate = K.Index;
			}
			if (!TestTrue(TEXT("an opponent and a teammate"), Opp >= 0 && Mate >= 0)) return false;
			const int32 N = Sim->GetSkaters().Num();
			for (int32 i = 0; i < N; ++i) Sim->SetSkater(i, FVector2D(-40.0 + i * 1.5, 19.0), FVector2D::ZeroVector, 0);
			Sim->SetSkater(Me, FVector2D::ZeroVector, FVector2D::ZeroVector, 0);
			Sim->SetSkater(Mate, FVector2D(4.0, -9.0), FVector2D::ZeroVector, UE_DOUBLE_PI * 0.5);   // the receiver: open, ahead and to the side
			Sim->SetPuck(FVector2D(0.45, 0.0), FVector2D::ZeroVector);
			TArray<FJamInput> In;
			In.SetNum(N);
			In[Me].Stick = FVector2D(1.0, 0.0);
			for (int32 i = 0; i < 8; ++i) Sim->Step(In);
			const bool bCarried = Sim->CarrierIndex() == Me;
			// The hold: 15 frames with the puck (a charged shot; a saucer-long pass hold).
			auto Hold = [&](bool bOn) { In[Me].bShootHeld = bOn && !bPass; In[Me].bPassHeld = bOn && bPass; };
			Hold(true);
			for (int32 i = 0; i < 15; ++i) Sim->Step(In);
			const int32 Charged = bPass ? Sim->GetSkaters()[Me].PassChargeFrames : Sim->GetSkaters()[Me].ChargeFrames;
			// The strip: the bot's poke from in front, its blade on the puck.
			Sim->SetSkater(Opp, FVector2D(2.6, 0.0), FVector2D::ZeroVector, UE_DOUBLE_PI);
			In[Opp].Stick = (Sim->GetPuck().Pos - FVector2D(2.6, 0.0)).GetSafeNormal();
			In[Opp].bPoke = true;
			bool bTaken = false;
			for (int32 i = 0; i < Cfg.Stick.PokeWindowFrames + 2 && !bTaken; ++i)
			{
				Sim->Step(In);
				In[Opp].bPoke = false;
				bTaken = Sim->GetPuck().LastTouch == Opp;
			}
			In[Opp] = FJamInput();
			const int32 AfterStrip = bPass ? Sim->GetSkaters()[Me].PassChargeFrames : Sim->GetSkaters()[Me].ChargeFrames;
			// The puck stays in his reach: it bounces back in front of his blade (a stripped puck that never left him), the button still held.
			Sim->SetSkater(Opp, FVector2D(6.0, 6.0), FVector2D::ZeroVector, UE_DOUBLE_PI);
			Sim->SetPuck(FVector2D(0.8, 0.0), FVector2D::ZeroVector);
			for (int32 i = 0; i < 4; ++i) Sim->Step(In);
			const double InReach = FVector2D::Distance(Sim->GetSkaters()[Me].Pos, Sim->GetPuck().Pos);
			const int32 Shots0 = Sim->GetSkaters()[Me].Shots, Passes0 = Sim->GetSkaters()[Me].Passes;
			Hold(false);
			for (int32 i = 0; i < 6; ++i) Sim->Step(In);
			ByRelease[bKey ? 1 : 0] = bPass ? Sim->GetSkaters()[Me].Passes - Passes0 : Sim->GetSkaters()[Me].Shots - Shots0;
			const int32 AfterRelease = bPass ? Sim->GetSkaters()[Me].PassChargeFrames : Sim->GetSkaters()[Me].ChargeFrames;
			AddInfo(FString::Printf(TEXT("JAMCTRL A203 sim | %s | charge_dies_on_loss %d | carried %d | charge %d before the strip | taken %d | charge %d after it | puck %.2f m from him | by the release %d | charge %d after |"),
				bPass ? TEXT("pass") : TEXT("shot"), bKey ? 1 : 0, bCarried ? 1 : 0, Charged, bTaken ? 1 : 0, AfterStrip, InReach, ByRelease[bKey ? 1 : 0], AfterRelease));
			TestTrue(TEXT("the staging: carried, charged, stripped by the poke"), bCarried && Charged > 4 && bTaken);
			TestTrue(TEXT("the puck is in his reach at the release"), InReach <= 1.6);
			if (bKey)
			{
				TestEqual(TEXT("the key on: the strip made the press dead (-1)"), AfterStrip, -1);
				TestEqual(TEXT("the key on: the release reset it"), AfterRelease, 0);
				// Pressed again with the puck: it charges and the release fires.
				Sim->SetPuck(FVector2D(0.8, 0.0), FVector2D::ZeroVector);
				for (int32 i = 0; i < 4; ++i) Sim->Step(In);
				const int32 Shots1 = Sim->GetSkaters()[Me].Shots, Passes1 = Sim->GetSkaters()[Me].Passes;
				Hold(true);
				for (int32 i = 0; i < 12; ++i) Sim->Step(In);
				Hold(false);
				for (int32 i = 0; i < 6; ++i) Sim->Step(In);
				const int32 Again = bPass ? Sim->GetSkaters()[Me].Passes - Passes1 : Sim->GetSkaters()[Me].Shots - Shots1;
				AddInfo(FString::Printf(TEXT("JAMCTRL A203 sim | %s | pressed again: %d |"), bPass ? TEXT("pass") : TEXT("shot"), Again));
				TestEqual(TEXT("the key on: a new press fires again"), Again, 1);
			}
			else TestTrue(TEXT("the key off: the strip leaves the charge as it was"), AfterStrip > 0);
		}
		TestEqual(*FString::Printf(TEXT("%s: the neutral value (false): the release after the strip fires the old charge"), bPass ? TEXT("pass") : TEXT("shot")), ByRelease[0], 1);
		TestEqual(*FString::Printf(TEXT("%s: charge_dies_on_loss true: the release after the strip fires nothing"), bPass ? TEXT("pass") : TEXT("shot")), ByRelease[1], 0);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsCaptureOutside, "Jam.Controls.ListenCapturesOutside", JamControlsTests::Flags)
bool FJamControlsCaptureOutside::RunTest(const FString&)
{
	// Lane A203 (item 3): while a rebinding cell listens, a press anywhere on the screen binds. The press goes through Slate's own entry
	// (FSlateApplication::ProcessMouseButtonDownEvent, what the OS's click calls) at a point 200 px outside the panel; before the lane only the panel's
	// handlers took it (a press beside the panel reached the menu behind). Its release is swallowed; once nothing listens, a press passes untouched.
	if (!FSlateApplication::IsInitialized()) { AddInfo(TEXT("JAMCTRL A203 capture: Slate is not up in this process: not run")); return true; }
	const FJamSettings Fresh;
	FJamSettings::SetOverrideForTest(&Fresh);
	ON_SCOPE_EXIT { FJamSettings::SetOverrideForTest(nullptr); };
	FRestoreBindings Restore;
	UJamInputMap::Get()->ApplyBindings(FJamBindings::Defaults());
	UWorld* World = nullptr;
	if (GEngine)
	{
		World = UWorld::CreateWorld(EWorldType::Game, false, TEXT("JamControlsCaptureWorld"));
		GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
	}
	ON_SCOPE_EXIT { if (World) { GEngine->DestroyWorldContext(World); World->DestroyWorld(false); } };
	UJamControlsWidget* W = UJamSettingsWidget::MakeControlsScreen(World);
	if (!TestNotNull(TEXT("the screen"), W)) return false;
	W->AddToRoot();
	ON_SCOPE_EXIT { W->ReleaseSlateResources(true); W->RemoveFromRoot(); };
	TSharedRef<SWidget> Slate = W->TakeWidget();
	W->Refresh();
	Slate->SlatePrepass(1.f);
	const FVector2D Panel = Slate->GetDesiredSize();
	const FVector2D Outside(Panel.X + 200.f, Panel.Y * 0.5f);   // 200 px right of the panel's right edge (the panel laid out from the origin)
	FSlateApplication& App = FSlateApplication::Get();
	auto Press = [&](const FKey& Button) -> bool
	{
		const FPointerEvent Down(App.GetUserIndexForMouse(), 0, Outside, Outside, TSet<FKey>({ Button }), Button, 0.f, FModifierKeysState());
		const bool bDown = App.ProcessMouseButtonDownEvent(nullptr, Down);
		const FPointerEvent Up(App.GetUserIndexForMouse(), 0, Outside, Outside, TSet<FKey>(), Button, 0.f, FModifierKeysState());
		const bool bUp = App.ProcessMouseButtonUpEvent(Up);
		AddInfo(FString::Printf(TEXT("JAMCTRL A203 capture | press %s at (%.0f, %.0f), panel %.0f x %.0f | down handled %d | up handled %d | listening %d |"),
			*Button.ToString(), Outside.X, Outside.Y, Panel.X, Panel.Y, bDown ? 1 : 0, bUp ? 1 : 0, W->IsListening() ? 1 : 0));
		return bDown && bUp;
	};
	W->BeginListen(EJamVerb::Spin, false);
	TestTrue(TEXT("listening, and the whole screen with it"), W->IsListening() && W->IsCapturingOutside());
	TestTrue(TEXT("the press 200 px outside the panel and its release are taken by the listen"), Press(EKeys::ThumbMouseButton2));
	TestTrue(TEXT("it bound: SPIN is on mouse 5, the listen ended"), !W->IsListening() && UJamInputMap::Get()->GetBindings()[EJamVerb::Spin].Keys[0] == EKeys::ThumbMouseButton2);
	AddInfo(FString::Printf(TEXT("JAMCTRL A203 capture status: %s"), *W->GetStatusLine()));
	const FJamBindings After = UJamInputMap::Get()->GetBindings();
	const FPointerEvent Down(App.GetUserIndexForMouse(), 0, Outside, Outside, TSet<FKey>({ EKeys::ThumbMouseButton }), EKeys::ThumbMouseButton, 0.f, FModifierKeysState());
	App.ProcessMouseButtonDownEvent(nullptr, Down);
	App.ProcessMouseButtonUpEvent(FPointerEvent(App.GetUserIndexForMouse(), 0, Outside, Outside, TSet<FKey>(), EKeys::ThumbMouseButton, 0.f, FModifierKeysState()));
	TestTrue(TEXT("nothing listens: a press outside changes no binding"), UJamInputMap::Get()->GetBindings() == After);
	// A refused move says so on the line (the acceptance row rebind_refused's text).
	W->BeginListen(EJamVerb::CrossCheck, false);
	W->TakeListenPress(EKeys::R, false);
	AddInfo(FString::Printf(TEXT("JAMCTRL A203 refused line: %s"), *W->GetStatusLine()));
	TestTrue(TEXT("cross-check onto R (restart's only key) is refused and the line says so"), W->GetStatusLine().Contains(TEXT("would leave another control with no key")) && UJamInputMap::Get()->GetBindings() == After);
	if (W->IsListening()) W->TakeListenPress(EKeys::Escape, false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsFirstFiveMinutes, "Jam.Controls.FirstFiveMinutes", JamControlsTests::Flags)
bool FJamControlsFirstFiveMinutes::RunTest(const FString&)
{
	// Lane A203 (item 4): the first five minutes, by script. A stranger's mouse buttons go through the real mapping layer (UJamPlayerInput, ApplyButtons) for
	// 3,000 frames a seed at rookie (the brain at rookie plays the nine others): late, long, both buttons at once, held through faceoffs and goals; he
	// skates at the puck, and with it at the net. Every press that produced a verb he could not have meant is counted:
	//   shot_unmeant   a shot fired by an LMB press that began as a cross-check (no puck at the press)
	//   pass_unmeant   a pass fired by an RMB press that began as a check (no puck at the press)
	//   check_carrying a body check sent while he carries
	//   brace_on_tap   a brace (the sim braced) inside an LMB press let go before jam.ctrl.BraceHoldFrames
	//   saucer_on_tap  a saucer from an RMB press let go before jam.ctrl.SaucerHoldFrames
	// Three arms: main's rule of before (HeldAcrossChange 0, thresholds 0 = the sim's 8 / 10), as shipped (1, 13, 13), and as shipped with the sim's
	// game.skater.charge_dies_on_loss on. The table is the AddInfo lines; shipped must be 0 in every column the mapping owns.
	UJamCartridge* Cart = nullptr;
	{
		TArray<FString> Errors;
		Cart = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")), Errors);
		for (const FString& E : Errors) AddError(FString::Printf(TEXT("hockey.regular: %s"), *E));
		if (!Cart) return false;
	}
	FRestoreBindings Restore;
	UJamInputMap::Get()->ApplyBindings(FJamBindings::Defaults());
	IConsoleVariable* Rule = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.ctrl.HeldAcrossChange"));
	IConsoleVariable* Sau = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.ctrl.SaucerHoldFrames"));
	IConsoleVariable* Bra = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.ctrl.BraceHoldFrames"));
	if (!TestTrue(TEXT("the three cvars"), Rule && Sau && Bra)) return false;
	const int32 RuleWas = Rule->GetInt(), SauWas = Sau->GetInt(), BraWas = Bra->GetInt();
	ON_SCOPE_EXIT { Rule->Set(RuleWas, ECVF_SetByCode); Sau->Set(SauWas, ECVF_SetByCode); Bra->Set(BraWas, ECVF_SetByCode); };
	struct FArm { const TCHAR* Name; int32 Rule, Sau, Bra; bool bKey; };
	const FArm Arms[] = { { TEXT("before (0, 0, 0, sim neutral)"), 0, 0, 0, false }, { TEXT("mapping shipped (1, 13, 13), sim neutral"), 1, 13, 13, false }, { TEXT("shipped (1, 13, 13, charge_dies_on_loss)"), 1, 13, 13, true } };
	struct FCount { int32 Presses = 0, Shots = 0, Passes = 0, Saucers = 0, Checks = 0, ShotUnmeant = 0, PassUnmeant = 0, CheckCarrying = 0, BraceOnTap = 0, SaucerOnTap = 0, Goals = 0, Faceoffs = 0; };
	const int32 Seeds[] = { 1, 2, 3, 4, 5 };
	FCount Shipped;
	for (const FArm& A : Arms)
	{
		Rule->Set(A.Rule, ECVF_SetByCode); Sau->Set(A.Sau, ECVF_SetByCode); Bra->Set(A.Bra, ECVF_SetByCode);
		FCount T;
		for (const int32 Seed : Seeds)
		{
			FJamGameConfig Cfg = Cart->Game;
			Cfg.Skater.bChargeDiesOnLoss = A.bKey;
			// Lane A215: A208's re-staging at the pickup's neutral is taken out: the brace-on-a-tap fault it stepped round is fixed (A209, UJamSim::FaceoffReset, Jam.Core.Contact.TapNeverBraces).
			UJamSim* Sim = NewObject<UJamSim>();
			Sim->Init(Cfg, static_cast<uint64>(Seed));
			UJamBrain* Brain = NewObject<UJamBrain>();
			Brain->ApplyCartridge(*Cart, TEXT("rookie"));
			Brain->ResetBrain();
			UJamPlayerInput* Pi = NewObject<UJamPlayerInput>();
			const int32 Me = Sim->GetHumanIndex();
			const int32 N = Sim->GetSkaters().Num();
			const int32 BraceAt = JamControls::BraceHoldFrames(), SaucerAt = JamControls::SaucerHoldFrames(Cfg.Pass.SaucerHalfFrames);
			FRandomStream R(Seed * 7919);
			TArray<FJamInput> In;
			In.SetNum(N);
			// The stranger's two buttons: down frame, planned length, the verb the press began as (from the first step's verbs), braced inside it.
			struct FBtn { FKey Key; int64 Down = -1; int32 Len = 0; bool bWith = false; bool bBraced = false; bool bFired = false; };
			FBtn L, Rb;
			L.Key = EKeys::LeftMouseButton; Rb.Key = EKeys::RightMouseButton;
			int64 NextPress = 30;
			int32 Faceoffs0 = 0;
			EJamSimPhase PrevPhase = Sim->GetPhase();
			for (int64 F = 0; F < 3000; ++F)
			{
				const TArray<FJamSkaterState>& Sk = Sim->GetSkaters();
				const FJamSkaterState& H = Sk[Me];
				// A newcomer's press: after a gap, LMB 45 %, RMB 35 %, both 20 %; 40 % short (2..6 frames), 30 % middling (7..14), 30 % long (15..90).
				if (F >= NextPress && L.Down < 0 && Rb.Down < 0)
				{
					const float Which = R.FRand();
					auto Len = [&]() { const float P = R.FRand(); return P < 0.4f ? R.RandRange(2, 6) : (P < 0.7f ? R.RandRange(7, 14) : R.RandRange(15, 90)); };
					if (Which < 0.65f) { L.Down = F; L.Len = Len(); L.bBraced = false; L.bFired = false; Pi->DebugButton(L.Key, true); ++T.Presses; }
					if (Which >= 0.45f) { Rb.Down = F; Rb.Len = Len(); Rb.bFired = false; Pi->DebugButton(Rb.Key, true); ++T.Presses; }
				}
				for (FBtn* B : { &L, &Rb })
				{
					if (B->Down >= 0 && F - B->Down >= B->Len)
					{
						Pi->DebugButton(B->Key, false);
						// a press let go before the threshold that braced (LMB) is a brace on a tap
						if (B == &L && B->bBraced && B->Len < BraceAt) { ++T.BraceOnTap; AddInfo(FString::Printf(TEXT("JAMCTRL brace on a tap: seed %d frame %lld down %lld len %d with %d carrier %d last %d"), Seed, (long long)F, (long long)B->Down, B->Len, B->bWith ? 1 : 0, Sim->CarrierIndex(), Sim->GetPuck().LastTouch)); }   // lane A208: the row of the case
						B->Down = -1;
						NextPress = F + R.RandRange(8, 70);
					}
				}
				// Bots: the brain at rookie (BuildWorld + Think, as the game mode feeds it).
				JamAI::FJamAIWorld W;
				JamAI::FJamSimAdapter::BuildWorld(*Sim, W);
				JamAI::FJamAIInput Out[JamAI::kMaxSkaters];
				Brain->Think(W, Out);
				for (int32 i = 0; i < N && i < JamAI::kMaxSkaters; ++i)
				{
					if (i == Me) continue;
					FJamInput& I = In[i];
					I = FJamInput();
					I.Move = FVector2D(Out[i].Move.X, Out[i].Move.Y); I.bShoot = Out[i].bShoot; I.bShootHeld = Out[i].bShootHeld; I.bPassHeld = Out[i].bPassHeld;
					I.bCheck = Out[i].bCheck; I.bBrake = Out[i].bBrake; I.bPass = Out[i].bPass; I.Stick = FVector2D(Out[i].Stick.X, Out[i].Stick.Y);
					I.bPoke = Out[i].bPoke; I.bLift = Out[i].bLift; I.bDeke = Out[i].bDeke; I.bSprint = Out[i].bSprint; I.bCrossHold = Out[i].bCrossHold;
				}
				// The stranger: at the puck without it, at the net with it; the stick aimed where he skates.
				FJamInput& Me_ = In[Me];
				Me_ = FJamInput();
				const bool bCarrying = Sim->CarrierIndex() == Me;
				const FVector2D To = bCarrying ? FVector2D(Sim->AttackGoalX(H.Team), 0.0) - H.Pos : Sim->GetPuck().Pos - H.Pos;
				Me_.Move = To.GetSafeNormal();
				Me_.Stick = To.GetSafeNormal();
				const bool bWithBefore = JamControls::HasPuckForButtons(*Sim, Me);
				JamControls::ApplyButtons(*Pi, *Sim, Me, Me_);
				// The verb each press began as: its first step.
				// the side each press resolved to: the possession the mapping read on its first step (JamControls::HasPuckForButtons)
				const bool bWithNow = bWithBefore;
				if (L.Down == F) L.bWith = bWithNow;
				if (Rb.Down == F) Rb.bWith = bWithNow;
				if (Me_.bCheck && bCarrying) ++T.CheckCarrying;
				if (Me_.bCheck) ++T.Checks;
				Sim->Step(In);
				const FJamSkaterState& H2 = Sim->GetSkaters()[Me];
				if (H2.bCrossBraced && L.Down >= 0) L.bBraced = true;
				if (H2.bShotThisFrame)
				{
					++T.Shots;
					// the press that fired it: the LMB press held now or just let go
					if (!L.bWith) ++T.ShotUnmeant;
				}
				if (H2.bPassThisFrame)
				{
					++T.Passes;
					if (!Rb.bWith) ++T.PassUnmeant;
					if (H2.bSaucerThisFrame) { ++T.Saucers; if (Rb.Len < SaucerAt) ++T.SaucerOnTap; }
				}
				for (const FJamSimEvent& E : Sim->GetFrameEvents()) { if (E.Name == JamEvents::Goal) ++T.Goals; }
				if (Sim->GetPhase() == EJamSimPhase::Faceoff && PrevPhase != EJamSimPhase::Faceoff) ++T.Faceoffs;
				PrevPhase = Sim->GetPhase();
			}
			Pi->DebugButton(L.Key, false); Pi->DebugButton(Rb.Key, false);
		}
		AddInfo(FString::Printf(TEXT("JAMCTRL A203 first5 | %s | presses %d | shots %d | passes %d (saucers %d) | checks %d | goals %d | faceoffs %d | shot_unmeant %d | pass_unmeant %d | check_carrying %d | brace_on_tap %d | saucer_on_tap %d |"),
			A.Name, T.Presses, T.Shots, T.Passes, T.Saucers, T.Checks, T.Goals, T.Faceoffs, T.ShotUnmeant, T.PassUnmeant, T.CheckCarrying, T.BraceOnTap, T.SaucerOnTap));
		if (A.Rule == 1 && A.bKey) Shipped = T;
		TestTrue(*FString::Printf(TEXT("%s: the stranger played (presses, shots and passes happened)"), A.Name), T.Presses > 50 && T.Shots + T.Passes > 0);
	}
	TestEqual(TEXT("shipped: no check while carrying (the mapping resolves RMB with the puck to the pass)"), Shipped.CheckCarrying, 0);
	TestEqual(TEXT("shipped: no brace on a tap shorter than jam.ctrl.BraceHoldFrames"), Shipped.BraceOnTap, 0);
	// Director, 14 Nov 22:40 (lane A207, the night): under pass power the hold is the pass's power and the sim lifts a pass by itself when an opposing blade
	// sits in the lane, so a tap can be a saucer by design: the count is logged in the JAMCTRL line above, not asserted.
	AddInfo(FString::Printf(TEXT("shipped: saucers on a tap %d (the sim's own lift over a blade in the lane, lane A207)"), Shipped.SaucerOnTap));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
