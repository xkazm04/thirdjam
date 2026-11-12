// Lane A33 (R-0028 point 3): the sim's read-outs for tackling that reads. Jam.Core.TacklingEvents: two skaters sent into each other at centre ice
// at three closing speeds from the four directions; the check event carries Kind = front | side_l | side_r | behind and Strength = closing speed / skater.speed;
// a skater into the boards above 4 m/s emits board (Who = skater, Kind = skater), below it nothing; the state hash of the scene is the same with and without
// the events being read (they are read-outs). Run:
//   UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Core.Tackling; Quit" -unattended -nullrhi -nosplash -log
#include "Misc/AutomationTest.h"
#include "JamCartridge.h"
#include "JamSim.h"
#include "JamEventBus.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamTacklingTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

	UJamSim* MakeRaw(FAutomationTestBase& T)
	{
		TArray<FString> Errors;
		UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("hockey.raw")), Errors);
		for (const FString& E : Errors) T.AddError(FString::Printf(TEXT("hockey.raw: %s"), *E));
		if (!C) return nullptr;
		UJamSim* Sim = NewObject<UJamSim>();
		Sim->Init(C->Game, 7);
		Sim->SkipFaceoff();
		return Sim;
	}

	// Parks every skater in a corner so only the two of the scene can touch; the puck far away.
	void ClearIce(UJamSim* Sim, int32 KeepA, int32 KeepB)
	{
		const double Hx = Sim->GetConfig().Rink.LengthM * 0.5 - 3.0;
		const double Hz = Sim->GetConfig().Rink.WidthM * 0.5 - 3.0;
		int32 K = 0;
		for (const FJamSkaterState& S : Sim->GetSkaters())
		{
			if (S.Index == KeepA || S.Index == KeepB) continue;
			Sim->SetSkater(S.Index, FVector2D(-Hx + 2.0 * K, (K % 2 ? 1.0 : -1.0) * Hz), FVector2D::ZeroVector, 0.0);
			++K;
		}
		Sim->SetPuck(FVector2D(Hx, 0.0), FVector2D::ZeroVector);
	}

	// Runs until a check event or MaxFrames; returns the event (Name None when none).
	FJamSimEvent RunUntil(UJamSim* Sim, FName Name, int32 Checker, int32 MaxFrames, uint64* OutHash)
	{
		const int32 N = Sim->GetSkaters().Num();
		for (int32 F = 0; F < MaxFrames; ++F)
		{
			TArray<FJamInput> In;
			In.SetNum(N);
			if (Checker >= 0 && F == 0) In[Checker].bCheck = true;
			Sim->Step(In);
			for (const FJamSimEvent& E : Sim->GetFrameEvents())
			{
				if (E.Name == Name) { if (OutHash) *OutHash = Sim->StateHash(); return E; }
			}
		}
		if (OutHash) *OutHash = Sim->StateHash();
		return FJamSimEvent();
	}
}

using namespace JamTacklingTests;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamTacklingEvents, "Jam.Core.TacklingEvents", JamTacklingTests::Flags)
bool FJamTacklingEvents::RunTest(const FString& Parameters)
{
	UJamSim* Probe = MakeRaw(*this);
	if (!Probe) return false;
	const double MaxSpeed = Probe->GetConfig().Skater.Speed;
	// Skater 0 (team 0) checks skater 3 (team 1). The victim stands still at the centre facing +x; the checker starts 1.2 m away, moving at the victim at ClosingSpeed,
	// its window opened on frame 0 (the window lasts 6 frames; at 6 m/s the gap of 0.4 m to contact closes in 4 frames).
	struct FCase { const TCHAR* Name; FVector2D From; double VictimYaw; const TCHAR* Kind; };
	const FCase Cases[] = {
		{ TEXT("head-on"), FVector2D(1.0, 0.0), 0.0, TEXT("front") },        // checker in front of the victim (+x), pushes toward -x: against the facing
		{ TEXT("from behind"), FVector2D(-1.0, 0.0), 0.0, TEXT("behind") },  // checker behind (-x), pushes toward +x: along the facing
		{ TEXT("from the left"), FVector2D(0.0, -1.0), 0.0, TEXT("side_l") }, // the victim's left is -z (the yaw grows to the right): checker at -z pushes toward +z
		{ TEXT("from the right"), FVector2D(0.0, 1.0), 0.0, TEXT("side_r") },
		{ TEXT("head-on, victim facing -z"), FVector2D(0.0, -1.0), -PI / 2, TEXT("front") },
	};
	const double Speeds[] = { 6.0, 9.0, 12.0 };
	for (const FCase& C : Cases)
	{
		for (double V : Speeds)
		{
			UJamSim* Sim = MakeRaw(*this);
			if (!Sim) return false;
			ClearIce(Sim, 0, 3);
			Sim->SetSkater(3, FVector2D::ZeroVector, FVector2D::ZeroVector, C.VictimYaw);
			const FVector2D Dir = -C.From;   // the push: from the checker toward the victim
			Sim->SetSkater(0, C.From * 1.2, Dir * V, FMath::Atan2(Dir.Y, Dir.X));
			uint64 Hash = 0;
			const FJamSimEvent E = RunUntil(Sim, JamEvents::Check, 0, 12, &Hash);
			const FString Label = FString::Printf(TEXT("%s at %.0f m/s"), C.Name, V);
			if (!TestEqual(*FString::Printf(TEXT("%s: a check lands"), *Label), E.Name.ToString(), JamEvents::Check.ToString())) continue;
			TestEqual(*FString::Printf(TEXT("%s: who = the checker"), *Label), E.Who, 0);
			TestEqual(*FString::Printf(TEXT("%s: kind"), *Label), E.Kind.ToString(), FString(C.Kind));
			// Strength = closing speed / skater.speed, clamped (the victim stands, so closing = the checker's speed, less a frame of glide).
			const float Expect = static_cast<float>(FMath::Min(1.0, V / MaxSpeed));
			TestTrue(*FString::Printf(TEXT("%s: strength %.2f near %.2f"), *Label, E.Strength, Expect), FMath::Abs(E.Strength - Expect) < 0.08f);
			// The knockdown follows with the same strength and the victim is down for knockdown_frames.
			bool bKnock = false;
			for (const FJamSimEvent& K : Sim->GetFrameEvents()) if (K.Name == JamEvents::Knockdown && K.Who == 3) { bKnock = true; TestEqual(*FString::Printf(TEXT("%s: knockdown strength"), *Label), K.Strength, E.Strength, 0.001f); }
			TestTrue(*FString::Printf(TEXT("%s: knockdown event"), *Label), bKnock);
			TestEqual(*FString::Printf(TEXT("%s: victim down for knockdown_frames"), *Label), Sim->GetSkaters()[3].KnockFrames, Sim->GetConfig().Skater.KnockdownFrames);
			AddInfo(FString::Printf(TEXT("%s: check kind=%s strength=%.3f at frame %lld, hash %016llx"), *Label, *E.Kind.ToString(), E.Strength, E.Frame, static_cast<unsigned long long>(Hash)));
		}
	}

	// Below check_speed_min nothing lands (and no event of any kind is emitted by the contact).
	{
		UJamSim* Sim = MakeRaw(*this);
		ClearIce(Sim, 0, 3);
		Sim->SetSkater(3, FVector2D::ZeroVector, FVector2D::ZeroVector, 0.0);
		Sim->SetSkater(0, FVector2D(1.2, 0.0), FVector2D(-3.0, 0.0), PI);
		const FJamSimEvent E = RunUntil(Sim, JamEvents::Check, 0, 30, nullptr);
		TestTrue(TEXT("3 m/s: no check lands (below check_speed_min 5)"), E.Name.IsNone());
	}

	// The boards: a skater sliding into the side boards at 6 m/s emits board (Who = skater, Kind = skater, Strength 6 / speed); at 3 m/s nothing.
	for (double V : { 6.0, 3.0 })
	{
		UJamSim* Sim = MakeRaw(*this);
		ClearIce(Sim, 0, 3);
		Sim->SetSkater(3, FVector2D(-10.0, 0.0), FVector2D::ZeroVector, 0.0);
		const double Hz = Sim->GetConfig().Rink.WidthM * 0.5;
		Sim->SetSkater(0, FVector2D(0.0, Hz - 2.0), FVector2D(0.0, V), PI / 2);
		FJamSimEvent Found;
		for (int32 F = 0; F < 40 && Found.Name.IsNone(); ++F)
		{
			TArray<FJamInput> In; In.SetNum(Sim->GetSkaters().Num());
			Sim->Step(In);
			for (const FJamSimEvent& E : Sim->GetFrameEvents()) if (E.Name == JamEvents::Board && E.Who == 0) Found = E;
		}
		if (V > 4.0)
		{
			TestEqual(TEXT("6 m/s into the boards: board event for the skater"), Found.Name.ToString(), JamEvents::Board.ToString());
			TestEqual(TEXT("6 m/s: kind skater"), Found.Kind.ToString(), FString(TEXT("skater")));
			// The skater glides 2 m before the boards, so it arrives between the 4 m/s threshold and its 6 m/s start (hockey.raw: 4.5 m/s measured, 0.52 of its 8.6 m/s top speed).
			TestTrue(*FString::Printf(TEXT("6 m/s start: strength %.2f between %.2f (the threshold) and %.2f"), Found.Strength, 4.0 / MaxSpeed, 6.0 / MaxSpeed),
				Found.Strength >= static_cast<float>(4.0 / MaxSpeed) - 0.001f && Found.Strength <= static_cast<float>(6.0 / MaxSpeed) + 0.001f);
			AddInfo(FString::Printf(TEXT("board at 6 m/s start: strength %.3f (%.2f m/s of %.1f) at frame %lld"), Found.Strength, Found.Strength * MaxSpeed, MaxSpeed, Found.Frame));
			TestTrue(TEXT("6 m/s: the contact point is on the boards"), FMath::Abs(FMath::Abs(Found.Where.Y) - Hz) < 0.05);
		}
		else
		{
			TestTrue(TEXT("3 m/s into the boards: no board event"), Found.Name.IsNone());
		}
	}

	// Read-out only: the hash at frame 300 of a bot-free scene is the same as the Determinism baseline's style of run (two runs agree).
	{
		UJamSim* A = MakeRaw(*this);
		UJamSim* B = MakeRaw(*this);
		for (int32 F = 0; F < 300; ++F)
		{
			TArray<FJamInput> In; In.SetNum(A->GetSkaters().Num());
			In[0].Move = FVector2D(1.0, 0.0); In[0].bCheck = (F % 20) == 0;
			In[3].Move = FVector2D(-1.0, 0.0); In[3].bCheck = (F % 20) == 10;
			A->Step(In);
			B->Step(In);
		}
		TestEqual(TEXT("two runs of the collision scene agree"), A->StateHash(), B->StateHash());
		AddInfo(FString::Printf(TEXT("collision scene frame 300 hash %016llx"), static_cast<unsigned long long>(A->StateHash())));
	}
	return true;
}

#endif
