// Lane A143 (track E1, B-answer-0024): what the bus tells device B. Jam.Fx.BusForB:
//  - the ability's kind as Variant (shotgun 0, hip_check 1, shield 2, slow_puck 3); a grant (Strength 0) plays nothing, a use does;
//  - a landed check carries Outcome `stumble` or `down` and the binder sends a stumble as Variant 4 (the side still rides User.Dir);
//  - in a sim scene the hip check's `ability` event comes on the press step, before the hit; every stumble check has no knockdown in its frame and every
//    down check has one (B's guess on Strength was 3 of 4).
// Headless scenes on UJamSim (no world). Run:
//   UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Fx.BusForB; Quit" -unattended -nullrhi -nosplash -Multiprocess -log
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "EffectsBinder.h"
#include "JamCartridge.h"
#include "JamSim.h"
#include "JamEventBus.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamBusForB
{
	FJamEventPayload P(float Strength, const TCHAR* Kind, const TCHAR* Outcome = nullptr)
	{
		FJamEventPayload Pl;
		Pl.Strength = Strength;
		Pl.Kind = Kind ? FName(Kind) : NAME_None;
		Pl.Outcome = Outcome ? FName(Outcome) : NAME_None;
		return Pl;
	}

	// The human (team 0) at the origin, every other skater parked on the far board; Victim = the first opponent.
	UJamSim* Scene(const FJamGameConfig& G, int32& Me, int32& Victim, double& Dir)
	{
		UJamSim* Sim = NewObject<UJamSim>();
		Sim->Init(G, 1);
		Sim->SkipFaceoff();
		Me = Sim->GetHumanIndex();
		Dir = Sim->AttackGoalX(0) > 0 ? 1.0 : -1.0;
		Victim = -1;
		for (const FJamSkaterState& S : Sim->GetSkaters())
		{
			if (S.Index == Me) continue;
			if (Victim < 0 && S.Team != 0) Victim = S.Index;
			Sim->SetSkater(S.Index, FVector2D(-30.0 + S.Index * 2.5, 20.0), FVector2D::ZeroVector, 0);
		}
		// The puck away from the scene: a carrier may not hit (CarrierMayNotHit), and the drop leaves the puck at the origin.
		Sim->SetPuck(FVector2D(0.0, -15.0), FVector2D::ZeroVector);
		return Sim;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFxBusForB, "Jam.Fx.BusForB", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamFxBusForB::RunTest(const FString&)
{
	using namespace JamBusForB;
	// 1. The tables (pure).
	TestEqual(TEXT("ability shotgun: Variant 0"), UJamEffectsBinder::VariantForPayload(JamEvents::Ability, P(1, TEXT("shotgun"))), 0.f);
	TestEqual(TEXT("ability hip_check: Variant 1"), UJamEffectsBinder::VariantForPayload(JamEvents::Ability, P(1, TEXT("hip_check"))), 1.f);
	TestEqual(TEXT("ability shield: Variant 2"), UJamEffectsBinder::VariantForPayload(JamEvents::Ability, P(1, TEXT("shield"))), 2.f);
	TestEqual(TEXT("ability slow_puck: Variant 3"), UJamEffectsBinder::VariantForPayload(JamEvents::Ability, P(1, TEXT("slow_puck"))), 3.f);
	TestTrue(TEXT("an ability grant (Strength 0) is quiet"), UJamEffectsBinder::IsQuietPayload(JamEvents::Ability, P(0, TEXT("shield"))));
	TestFalse(TEXT("an ability use (Strength 1) plays"), UJamEffectsBinder::IsQuietPayload(JamEvents::Ability, P(1, TEXT("shield"))));
	TestFalse(TEXT("a check with Strength 0 still plays"), UJamEffectsBinder::IsQuietPayload(JamEvents::Check, P(0, TEXT("front"), TEXT("stumble"))));
	TestEqual(TEXT("stumble check: Variant 4"), UJamEffectsBinder::VariantForPayload(JamEvents::Check, P(0.3f, TEXT("side_l"), TEXT("stumble"))), 4.f);
	TestEqual(TEXT("down check keeps its side: side_l 1"), UJamEffectsBinder::VariantForPayload(JamEvents::Check, P(0.8f, TEXT("side_l"), TEXT("down"))), 1.f);
	TestEqual(TEXT("down check keeps its side: behind 3"), UJamEffectsBinder::VariantForPayload(JamEvents::Check, P(0.8f, TEXT("behind"), TEXT("down"))), 3.f);
	TestEqual(TEXT("a check with no Outcome (a test's Broadcast): the side table"), UJamEffectsBinder::VariantForPayload(JamEvents::Check, P(0.f, TEXT("side_r"))), 2.f);
	TestEqual(TEXT("knockdown is untouched"), UJamEffectsBinder::VariantForPayload(JamEvents::Knockdown, P(1, TEXT("check"), TEXT("stumble"))), 0.f);
	TestEqual(TEXT("faceoff table unchanged"), UJamEffectsBinder::VariantFor(JamEvents::Faceoff, FName(TEXT("lost"))), 2.f);

	TArray<FString> Errors;
	UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")), Errors);
	if (!C) { AddError(TEXT("hockey.regular did not load")); return false; }

	// 2. The hip check's event comes on the press step, not on the hit (MS_ability's thump sits 0.18 s into its file).
	{
		FJamGameConfig G = C->Game;
		FJamAbilityConfig A;
		A.Kind = TEXT("hip_check");
		A.Trigger = TEXT("mode");
		G.Abilities.Reset();
		G.Abilities.Add(A);
		G.Rules.bPenalties = false;
		int32 Me, V;
		double D;
		UJamSim* Sim = Scene(G, Me, V, D);
		// The opponent 1.0 m ahead (0.2 m between the bodies): the human glides 3 m/s into it, so the hit comes a few frames after the press.
		Sim->SetSkater(Me, FVector2D(0, 0), FVector2D(D * 3.0, 0.0), D > 0 ? 0.0 : PI);
		Sim->SetSkater(V, FVector2D(D * 1.0, 0.0), FVector2D::ZeroVector, D > 0 ? PI : 0.0);
		TArray<FJamInput> In;
		In.SetNum(Sim->GetSkaters().Num());
		In[Me].bAbility = true;
		int32 AbilityStep = -1, CheckStep = -1;   // step 0 is the step whose input carries the press
		for (int32 f = 0; f < 120 && CheckStep < 0; ++f)
		{
			Sim->Step(In);
			In[Me].bAbility = false;
			for (const FJamSimEvent& E : Sim->GetFrameEvents())
			{
				if (E.Name == JamEvents::Ability && E.Who == Me && E.Strength > 0.5f && AbilityStep < 0) AbilityStep = f;
				if (E.Name == JamEvents::Check && E.Who == Me && CheckStep < 0) CheckStep = f;
			}
		}
		AddInfo(FString::Printf(TEXT("hip check: F in the input of step 0; the ability event in step %d, the check (the hit) in step %d (%d frames, %.2f s later)"),
			AbilityStep, CheckStep, CheckStep - AbilityStep, (CheckStep - AbilityStep) / 60.0));
		TestEqual(TEXT("the ability event is on the press step"), AbilityStep, 0);
		TestTrue(TEXT("the hit lands after the press"), CheckStep > 0);
	}

	// 3. Stumble against down: a sweep of closing speeds into a standing opponent; every check's Outcome agrees with whether its victim went down that frame.
	{
		FJamGameConfig G = C->Game;
		G.Rules.bPenalties = false;
		int32 Stumbles = 0, Downs = 0, Agree = 0, Checks = 0, LowStrengthStumbles = 0;
		for (int32 k = 0; k <= 16; ++k)
		{
			const double Speed = 1.0 + 0.5 * k;
			int32 Me, V;
			double D;
			UJamSim* Sim = Scene(G, Me, V, D);
			Sim->SetSkater(Me, FVector2D(0, 0), FVector2D(D * Speed, 0.0), D > 0 ? 0.0 : PI);
			// Lane A192: the window opens game.contact.check_windup_frames after the press: the victim stands the wind-up's ground farther out (the same contact).
			Sim->SetSkater(V, FVector2D(D * (1.2 + Speed * G.Contact.CheckWindupFor(Sim->GetSkaters()[Me].bHuman) / 60.0), 0.0), FVector2D::ZeroVector, D > 0 ? PI : 0.0);
			TArray<FJamInput> In;
			In.SetNum(Sim->GetSkaters().Num());
			In[Me].bCheck = true;
			for (int32 f = 0; f < 40; ++f)
			{
				Sim->Step(In);
				In[Me].bCheck = false;
				const FJamSimEvent* Check = nullptr;
				bool bKnock = false;
				for (const FJamSimEvent& E : Sim->GetFrameEvents())
				{
					if (E.Name == JamEvents::Check && E.Who == Me) Check = &E;
					if (E.Name == JamEvents::Knockdown && E.Who == V) bKnock = true;
				}
				if (!Check) continue;
				++Checks;
				const bool bStumble = Check->Outcome == FName(TEXT("stumble"));
				const bool bDown = Check->Outcome == FName(TEXT("down"));
				Stumbles += bStumble ? 1 : 0;
				Downs += bDown ? 1 : 0;
				LowStrengthStumbles += bStumble && Check->Strength < 0.001f ? 1 : 0;
				Agree += (bStumble && !bKnock) || (bDown && bKnock) ? 1 : 0;
				AddInfo(FString::Printf(TEXT("closing %.1f m/s: check strength %.3f outcome %s, knockdown %d"), Speed, Check->Strength, *Check->Outcome.ToString(), bKnock ? 1 : 0));
				break;
			}
		}
		AddInfo(FString::Printf(TEXT("checks %d: stumbles %d (of them Strength < 0.001: %d), downs %d, outcome agrees with the knockdown %d of %d"),
			Checks, Stumbles, LowStrengthStumbles, Downs, Agree, Checks));
		TestTrue(TEXT("the sweep saw a stumble"), Stumbles > 0);
		TestTrue(TEXT("the sweep saw a knockdown"), Downs > 0);
		TestEqual(TEXT("every check carries an Outcome"), Stumbles + Downs, Checks);
		TestEqual(TEXT("the Outcome agrees with the knockdown every time"), Agree, Checks);
	}
	return true;
}

#endif
