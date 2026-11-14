// Lane A209 (track F1, the night; docs/experiments/CONTACT-13.md): a hit he aims lands. Jam.Core.Contact.HitFindsMan stages the human's presses on the
// cartridge as it ships (contact.check_pull_m 1.0, cross_any_man true, check_reach_m 1.0, the wind-ups 8 / 8) and at the neutral values (0, false, 0, 4 / 4):
// a check pressed at a man his straight line misses by 0.2 m lands at the wind-up's end; a standing shove is a bump; a press at a man 5.5 m away whiffs;
// a cross-check at a man without the puck shoves him; the wind-up and the pull are read from the cartridge the test builds.
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "JamCartridge.h"
#include "JamSim.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamHitLandsTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

	struct FOut { bool bOk = false; int32 Outcome = 0; bool bCross = false; int32 ContactFrame = -1; int32 Wu = 0; double MaxStep = 0.0; };

	// The human A skates +x at Speed from the origin; the opponent V stands at (Ahead, Off) facing him. Press: 1 = body check, 2 = cross-check (tap), on frame 0.
	FOut Scene(FAutomationTestBase& T, bool bNeutral, double Speed, double Ahead, double Off, int32 Press)
	{
		FOut O;
		TArray<FString> Errors;
		UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")), Errors);
		if (!C) { T.AddError(TEXT("hockey.regular does not load")); return O; }
		FJamGameConfig Cfg = C->Game;
		Cfg.Rules.bPenalties = false;
		if (bNeutral) { Cfg.Contact.CheckPullM = 0.0; Cfg.Contact.bCrossAnyMan = false; Cfg.Contact.CheckReachM = 0.0; Cfg.Contact.CheckWindupFrames = 4; Cfg.Contact.CrossWindupFrames = 4; }
		UJamSim* Sim = NewObject<UJamSim>();
		Sim->Init(Cfg, 7);
		Sim->SkipFaceoff();
		const int32 A = Sim->GetHumanIndex();
		const int32 Team = Sim->GetSkaters()[A].Team;
		int32 V = -1;
		for (const FJamSkaterState& K : Sim->GetSkaters()) if (K.Team != Team) { V = K.Index; break; }
		const double Hx = Cfg.Rink.LengthM * 0.5 - 4.0, Hz = Cfg.Rink.WidthM * 0.5 - 3.0;
		int32 N = 0;
		for (const FJamSkaterState& K : Sim->GetSkaters())
		{
			if (K.Index == A || K.Index == V) continue;
			Sim->SetSkater(K.Index, FVector2D(-Hx + 2.5 * N, (N % 2 ? 1.0 : -1.0) * Hz), FVector2D::ZeroVector, 0.0);
			++N;
		}
		Sim->SetPuck(FVector2D(-20.0, 12.0), FVector2D::ZeroVector);
		Sim->SetSkater(A, FVector2D::ZeroVector, FVector2D(Speed, 0.0), 0.0);
		Sim->SetSkater(V, FVector2D(Ahead, Off), FVector2D::ZeroVector, PI);
		O.Wu = Press == 2 ? Sim->GetConfig().Contact.CrossWindupFor(true) : Sim->GetConfig().Contact.CheckWindupFor(true);
		FVector2D Prev = Sim->GetSkaters()[A].Pos;
		for (int32 F = 0; F < 40; ++F)
		{
			TArray<FJamInput> In;
			In.SetNum(Sim->GetSkaters().Num());
			In[A].Move = Speed > 0.0 ? FVector2D(1.0, 0.0) : FVector2D::ZeroVector;
			In[A].Stick = FVector2D(1.0, 0.0);
			In[A].bCheck = Press == 1 && F == 0;
			In[A].bLift = Press == 2 && F == 0;
			Sim->Step(In);
			const FJamSkaterState& Sa = Sim->GetSkaters()[A];
			const FJamSkaterState& Sv = Sim->GetSkaters()[V];
			const bool bMet = FVector2D::Distance(Sa.Pos, Sv.Pos) < UJamSim::SkaterRadius * 2.0 + 0.02;   // the bodies' own push-out is not the pull
			if (O.ContactFrame < 0 && Sv.ContactBy == A && Sv.ContactOutcome > 0) { O.ContactFrame = F; O.Outcome = Sv.ContactOutcome; O.bCross = Sv.bContactCross; }
			if (O.ContactFrame < 0 && !bMet) O.MaxStep = FMath::Max(O.MaxStep, FVector2D::Distance(Prev, Sa.Pos) - Sa.Vel.Size() * UJamSim::DT);
			Prev = Sa.Pos;
		}
		O.bOk = true;
		T.AddInfo(FString::Printf(TEXT("JAMHIT %s press %d at %.1f m/s, man %.1f ahead %.1f aside: outcome %d cross %d contact frame %d (wind-up %d), pull beyond velocity x frame at most %.3f m"),
			bNeutral ? TEXT("neutral") : TEXT("shipped"), Press, Speed, Ahead, Off, O.Outcome, O.bCross ? 1 : 0, O.ContactFrame, O.Wu, O.MaxStep));
		return O;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHitFindsMan, "Jam.Core.Contact.HitFindsMan", JamHitLandsTests::Flags)
bool FJamHitFindsMan::RunTest(const FString& Parameters)
{
	using namespace JamHitLandsTests;
	// 1. A body check at 5 m/s at a man 2.2 m ahead and 1.0 m aside: the straight line passes him by 0.2 m (the thin space).
	const FOut Ship = Scene(*this, false, 5.0, 2.2, 1.0, 1), Neut = Scene(*this, true, 5.0, 2.2, 1.0, 1);
	if (!Ship.bOk || !Neut.bOk) return false;
	TestTrue(TEXT("neutral: the press runs out on nobody"), Neut.Outcome == 0);
	TestTrue(*FString::Printf(TEXT("shipped: the check finds its man (outcome %d), a stumble or a knockdown"), Ship.Outcome), Ship.Outcome >= UJamSim::ContactStumble && !Ship.bCross);
	TestTrue(*FString::Printf(TEXT("shipped: the contact comes at the wind-up's end (frame %d, wind-up %d)"), Ship.ContactFrame, Ship.Wu), Ship.ContactFrame >= Ship.Wu - 2 && Ship.ContactFrame <= Ship.Wu + 1);
	TestTrue(*FString::Printf(TEXT("shipped: the pull moves the root at most check_pull_m / wind-up a frame (%.3f m)"), Ship.MaxStep), Ship.MaxStep <= 1.0 / FMath::Max(1, Ship.Wu) + 0.02);
	// 2. The size is by speed: standing, a man 1.3 m ahead (gap 0.5 m): the pull brings the bodies together and it is a bump.
	{
		const FOut S = Scene(*this, false, 0.0, 1.3, 0.0, 1);
		if (!S.bOk) return false;
		TestTrue(*FString::Printf(TEXT("standing: a bump, not a hit (outcome %d)"), S.Outcome), S.Outcome == UJamSim::ContactBump);
	}
	// 3. A press at a man 5.5 m away, standing: nothing.
	{
		const FOut S = Scene(*this, false, 0.0, 5.5, 0.0, 1);
		if (!S.bOk) return false;
		TestTrue(*FString::Printf(TEXT("a press at a man 5.5 m away whiffs (outcome %d)"), S.Outcome), S.Outcome == 0);
	}
	// 4. A cross-check at 3 m/s at a man WITHOUT the puck 1.6 m ahead (past arm's length): shipped, the shove lands at the wind-up's end; neutral, nothing.
	{
		const FOut S = Scene(*this, false, 3.0, 1.6, 0.0, 2), Nn = Scene(*this, true, 3.0, 1.6, 0.0, 2);
		if (!S.bOk || !Nn.bOk) return false;
		TestTrue(*FString::Printf(TEXT("shipped: the cross-check shoves the man without the puck (outcome %d, cross %d)"), S.Outcome, S.bCross ? 1 : 0), S.Outcome >= UJamSim::ContactStumble && S.bCross);
		TestTrue(*FString::Printf(TEXT("shipped: on the wind-up's last frame (frame %d, wind-up %d)"), S.ContactFrame, S.Wu), S.ContactFrame == S.Wu);
		TestTrue(*FString::Printf(TEXT("neutral: no shove on a man without the puck (outcome %d cross %d)"), Nn.Outcome, Nn.bCross ? 1 : 0), !(Nn.bCross && Nn.Outcome > 0));
	}
	return true;
}

#endif
