// Lane A16 (Director, from A3's traces): a skater must never be trapped by a net. Jam.Core.NetTrap replays hockey.regular at pro, seed 15, for 6000 frames
// with the real brain and the scripted player bot (the tournament runner's loop) and fails when any standing skater sits inside a net box for more than 30
// consecutive frames, or presses into a net hull (move input, no motion, within 0.25 m of the box) for more than 30 consecutive frames. Headless, no world.
//   UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Core.NetTrap; Quit" -unattended -nullrhi -nosplash -log
#include "Misc/AutomationTest.h"
#include "JamCartridge.h"
#include "JamSim.h"
#include "JamBrain.h"
#include "JamSimAdapter.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamNetTrapTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

	// Distance from a circle (P, R) to the net box of Side (+1 at +x), negative when the centre is inside the box.
	double NetBoxDistance(const UJamSim& Sim, const FVector2D& P, int32 Side)
	{
		const double GL = Sim.GoalLineX();
		const double Hw = Sim.GetConfig().Rink.GoalWidthM * 0.5;
		const double X0 = Side > 0 ? GL : -GL - UJamSim::NetDepth;
		const double X1 = Side > 0 ? GL + UJamSim::NetDepth : -GL;
		const double Dx = FMath::Max(FMath::Max(X0 - P.X, P.X - X1), 0.0);
		const double Dz = FMath::Max(FMath::Abs(P.Y) - Hw, 0.0);
		const bool bInside = P.X > X0 && P.X < X1 && FMath::Abs(P.Y) < Hw;
		if (bInside) return -FMath::Min(FMath::Min(P.X - X0, X1 - P.X), Hw - FMath::Abs(P.Y));
		return FMath::Sqrt(Dx * Dx + Dz * Dz);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNetTrap, "Jam.Core.NetTrap", JamNetTrapTests::Flags)
bool FJamNetTrap::RunTest(const FString& Parameters)
{
	TArray<FString> Errors;
	UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")), Errors);
	for (const FString& E : Errors) AddError(E);
	if (!C) return false;

	UJamSim* Sim = NewObject<UJamSim>();
	Sim->Init(C->Game, 15);
	UJamBrain* Brain = NewObject<UJamBrain>();
	Brain->ApplyCartridge(*C, TEXT("pro"));
	Brain->ResetBrain();
	UJamPlayerBot* Player = NewObject<UJamPlayerBot>();

	const int32 N = Sim->GetSkaters().Num();
	TArray<FJamInput> Inputs;
	Inputs.SetNum(N);
	TArray<int32> InsideRun, StuckRun;
	InsideRun.Init(0, N);
	StuckRun.Init(0, N);
	int32 WorstInside = 0, WorstStuck = 0, WorstWho = -1;
	int64 WorstFrame = -1;
	FVector2D WorstPos = FVector2D::ZeroVector;
	bool bOk = true;

	for (int32 F = 0; F < 6000 && Sim->GetPhase() != EJamSimPhase::Over; ++F)
	{
		JamAI::FJamAIWorld W;
		JamAI::FJamSimAdapter::BuildWorld(*Sim, W);
		JamAI::FJamAIInput Out[JamAI::kMaxSkaters];
		Brain->Think(W, Out);
		const int32 Human = Sim->GetHumanIndex();
		for (int32 i = 0; i < N && i < JamAI::kMaxSkaters; ++i)
		{
			const JamAI::FJamAIInput& A = i == Human ? Player->Think(W) : Out[i];
			Inputs[i].Move = FVector2D(A.Move.X, A.Move.Y);
			Inputs[i].bShoot = A.bShoot;
			Inputs[i].bCheck = A.bCheck;
			Inputs[i].bBrake = A.bBrake;
		}
		Sim->Step(Inputs);
		for (const FJamSkaterState& S : Sim->GetSkaters())
		{
			if (S.InBox() || S.IsDown()) { InsideRun[S.Index] = StuckRun[S.Index] = 0; continue; }
			const double D = FMath::Min(JamNetTrapTests::NetBoxDistance(*Sim, S.Pos, 1), JamNetTrapTests::NetBoxDistance(*Sim, S.Pos, -1));
			InsideRun[S.Index] = D < 0.0 ? InsideRun[S.Index] + 1 : 0;
			const bool bPressing = Inputs.IsValidIndex(S.Index) && Inputs[S.Index].Move.Size() > 0.5 && S.Vel.Size() < 0.2 && D < UJamSim::SkaterRadius + 0.25;
			StuckRun[S.Index] = bPressing ? StuckRun[S.Index] + 1 : 0;
			if (InsideRun[S.Index] > WorstInside) { WorstInside = InsideRun[S.Index]; WorstWho = S.Index; WorstFrame = Sim->GetFrame(); WorstPos = S.Pos; }
			if (StuckRun[S.Index] > WorstStuck) { WorstStuck = StuckRun[S.Index]; WorstWho = S.Index; WorstFrame = Sim->GetFrame(); WorstPos = S.Pos; }
		}
	}
	AddInfo(FString::Printf(TEXT("hockey.regular pro seed 15, %lld frames: longest inside a net %d frames, longest pressing into a net %d frames (skater %d at frame %lld, (%.1f, %.1f)); score %d:%d; hash %s"),
		Sim->GetFrame(), WorstInside, WorstStuck, WorstWho, WorstFrame, WorstPos.X, WorstPos.Y, Sim->GetScore(0), Sim->GetScore(1), *Sim->StateHashHex()));
	if (WorstInside > 30) { AddError(FString::Printf(TEXT("a skater sat inside a net for %d frames"), WorstInside)); bOk = false; }
	if (WorstStuck > 30) { AddError(FString::Printf(TEXT("a skater pressed into a net hull without moving for %d frames"), WorstStuck)); bOk = false; }
	return bOk;
}

#endif
