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
#include "Internationalization/Regex.h"
#include "HokejGameMode.h"
#include "MenuStyle.h"
#include "Fonts/FontMeasure.h"
#include "Rendering/SlateRenderer.h"

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
		const FJamButtonVerbs B = R.Step(false, D);  // the puck is gone (stripped), the button still down
		TestTrue(TEXT("a shot begun with the puck stays the shot's level after the puck leaves, never a cross-check"), A.bShootHeld && B.bShootHeld && !B.bLift && !B.bCrossHeld);
		R.Release(EKeys::LeftMouseButton);
		const FJamButtonVerbs C = R.Step(false, D);
		TestTrue(TEXT("its release ends the shot's level (the sim fires on that frame)"), !C.bShootHeld && !C.bLift);
		R.Press(EKeys::LeftMouseButton);
		const FJamButtonVerbs E = R.Step(false, D);
		TestTrue(TEXT("the next press, without the puck, is the cross-check"), E.bLift && E.bCrossHeld && !E.bShootHeld);
	}
	{
		FJamButtonResolver R;
		R.Press(EKeys::RightMouseButton);
		const FJamButtonVerbs A = R.Step(true, D);
		const FJamButtonVerbs B = R.Step(false, D);
		TestTrue(TEXT("a pass begun with the puck stays the pass (the saucer's hold) after the puck leaves, never a check"), A.bPassHeld && B.bPassHeld && !B.bCheck);
	}
	{
		FJamButtonResolver R;
		R.Press(EKeys::LeftMouseButton);
		const FJamButtonVerbs A = R.Step(false, D);
		const FJamButtonVerbs B = R.Step(true, D);   // the skater picked the puck up while holding the cross-check
		TestTrue(TEXT("a cross-check begun without the puck stays a cross-check press (no shot) when the puck arrives"), A.bLift && !B.bShootHeld && !B.bShootPress && B.bCrossHeld);
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

#endif // WITH_DEV_AUTOMATION_TESTS
