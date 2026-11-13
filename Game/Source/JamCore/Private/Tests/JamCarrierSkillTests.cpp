// Lane A102 (track F2, carrier skill, evolution 1): the stop-and-go is the carrier's signature (docs/VISION.md law 3; docs/experiments/MOVEMENT-2.md).
// Jam.Core.Movement.StopAndGo (the scripted human carries at a standing pro defender: a clean stop, cut and go against a straight charge, 200 seeds each),
// Jam.Core.Movement.BodyShield (puck protection with the body between, game.stick.body_shield, and its neutral value),
// Bar.Movement.StopAndGoSweep (outside the gate's Jam filter: the same scene over the movement keys, one at a time). Headless, on UJamSim with the real brain.
//   UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Core.Movement; Quit" -unattended -nullrhi -nosplash -log
#include "Misc/AutomationTest.h"
#include "JamCartridge.h"
#include "JamSim.h"
#include "JamEventBus.h"
#include "JamBrain.h"
#include "JamSimAdapter.h"
#include "JamRandom.h"
#include "JamCarryDrill.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamCarrierSkillTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

	UJamCartridge* LoadPreset(FAutomationTestBase& T, const FString& Name)
	{
		TArray<FString> Errors;
		UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(Name), Errors);
		for (const FString& E : Errors) T.AddError(FString::Printf(TEXT("%s: %s"), *Name, *E));
		return C;
	}

	// The scene and the scripted human are FJamCarryDrill (JamCarryDrill.h): the same code jam.ctrl.Drill 21 runs in a package.
	using FScript = FJamCarryDrill;
	using FRun = FJamCarryDrill;
	constexpr int32 Past = FJamCarryDrill::Past, Setup = FJamCarryDrill::Setup, EndCount = FJamCarryDrill::EndCount;

	FJamInput ToInput(const JamAI::FJamAIInput& A)
	{
		FJamInput I;
		I.Move = FVector2D(A.Move.X, A.Move.Y);
		I.bShoot = A.bShoot; I.bShootHeld = A.bShootHeld; I.bCheck = A.bCheck; I.bBrake = A.bBrake; I.bPass = A.bPass; I.bPassHeld = A.bPassHeld;
		I.Stick = FVector2D(A.Stick.X, A.Stick.Y);
		I.bPoke = A.bPoke; I.bLift = A.bLift; I.bDeke = A.bDeke; I.bSpin = A.bSpin; I.bSprint = A.bSprint; I.bAbility = A.bAbility;
		return I;
	}

	// One carry at the opposing chaser, which the real brain plays at Level from a standstill; everyone else is parked in a corner with no input.
	FRun Carry(const UJamCartridge& C, uint64 Seed, const FScript& Script, const TCHAR* Level = TEXT("pro"), TArray<FString>* Trace = nullptr, UJamSim* Sim = nullptr, UJamBrain* Brain = nullptr)
	{
		FJamCarryDrill D = Script;
		if (!Sim) Sim = NewObject<UJamSim>();
		if (!Brain) Brain = NewObject<UJamBrain>();
		Sim->Init(C.Game, Seed);
		Sim->SkipFaceoff();
		if (!D.Stage(*Sim, Seed)) return D;
		Brain->ApplyCartridge(C, Level);
		Brain->ResetBrain();
		TArray<FJamInput> In;
		In.SetNum(Sim->GetSkaters().Num());
		while (D.End == FJamCarryDrill::Running)
		{
			JamAI::FJamAIWorld W;
			JamAI::FJamSimAdapter::BuildWorld(*Sim, W);
			JamAI::FJamAIInput Out[JamAI::kMaxSkaters];
			Brain->Think(W, Out);
			for (FJamInput& I : In) I = FJamInput();
			if (!D.DefenderWaits()) In[D.Def] = ToInput(Out[D.Def]);
			In[D.Me] = D.HumanInput(*Sim);
			D.BeforeStep(*Sim, In[D.Def]);
			if (Trace && D.Frames % 4 == 0)
			{
				const FJamSkaterState& S = Sim->GetSkaters()[D.Me];
				const FJamSkaterState& O = Sim->GetSkaters()[D.Def];
				Trace->Add(FString::Printf(TEXT("f=%d phase=%d gap=%.2f me=(%.2f,%.2f) v=%.2f yaw=%.0f def=(%.2f,%.2f) v=%.2f puck=(%.2f,%.2f) carry=%d burst=%d brake=%d sweep=%d body=%d dstick=(%.2f,%.2f) dpoke=%d dlift=%d dcheck=%d dsprint=%d"),
					D.Frames, D.Phase, FVector2D::Distance(S.Pos, O.Pos), S.Pos.X, S.Pos.Y, S.Vel.Size(), FMath::RadiansToDegrees(S.Yaw), O.Pos.X, O.Pos.Y, O.Vel.Size(), Sim->GetPuck().Pos.X, Sim->GetPuck().Pos.Y,
					Sim->CarrierIndex() == D.Me ? 1 : 0, S.BurstLeft, S.bReverseBraking ? 1 : 0, S.SweepFrames, D.bBodyBefore ? 1 : 0,
					In[D.Def].Stick.X, In[D.Def].Stick.Y, In[D.Def].bPoke ? 1 : 0, In[D.Def].bLift ? 1 : 0, In[D.Def].bCheck ? 1 : 0, In[D.Def].bSprint ? 1 : 0));
			}
			Sim->Step(In);
			if (Trace) for (const FJamSimEvent& E : Sim->GetFrameEvents()) Trace->Add(FString::Printf(TEXT("  event f=%d %s who=%d kind=%s strength=%.2f"), D.Frames, *E.Name.ToString(), E.Who, *E.Kind.ToString(), E.Strength));
			D.AfterStep(*Sim);
		}
		if (Trace) Trace->Add(FString::Printf(TEXT("end=%s frames=%d min_gap=%.2f brake_frames=%d"), FJamCarryDrill::EndName(D.End), D.Frames, D.MinGapM, D.BrakeFrames));
		return D;
	}

	struct FShare
	{
		int32 N = 0;
		int32 Ends[EndCount] = {};
		int32 BodyShieldRefused = 0, DefLifts = 0, DefPokes = 0, DefChecks = 0;
		int64 Frames = 0;
		double PastShare() const { return N > 0 ? static_cast<double>(Ends[Past]) / N : 0.0; }
		FString Line() const
		{
			FString S = FString::Printf(TEXT("past %d of %d (%.1f %%)"), Ends[Past], N, 100.0 * PastShare());
			for (int32 i = 1; i < EndCount; ++i) if (Ends[i] > 0) S += FString::Printf(TEXT(", %s %d"), FJamCarryDrill::EndName(i), Ends[i]);
			S += FString::Printf(TEXT("; defender presses: lift %d poke %d check %d, lifts refused by the body %d; %.0f frames a carry"), DefLifts, DefPokes, DefChecks, BodyShieldRefused, N > 0 ? static_cast<double>(Frames) / N : 0.0);
			return S;
		}
	};

	FShare Share(const UJamCartridge& C, const FScript& Sc, int32 Seeds, const TCHAR* Level = TEXT("pro"))
	{
		FShare S;
		UJamSim* Sim = NewObject<UJamSim>();
		UJamBrain* Brain = NewObject<UJamBrain>();
		for (int32 Seed = 1; Seed <= Seeds; ++Seed)
		{
			const FRun R = Carry(C, static_cast<uint64>(Seed), Sc, Level, nullptr, Sim, Brain);
			++S.N; ++S.Ends[R.End]; S.BodyShieldRefused += R.BodyShieldRefused; S.DefLifts += R.DefLifts; S.DefPokes += R.DefPokes; S.DefChecks += R.DefChecks; S.Frames += R.Frames;
		}
		return S;
	}

	UJamCartridge* Copy(const UJamCartridge& C) { return DuplicateObject<UJamCartridge>(&C, GetTransientPackage()); }

	// The clean stop-and-go the gate holds (MOVEMENT-2.md section 2): FJamCarryDrill's own defaults.
	FScript DefaultGo() { return FScript(); }

	// A scene for the rule itself: the carrier (human) at the origin facing Yaw with the puck on its blade at StickDir, the opposing chaser standing at DefPos.
	UJamSim* ShieldScene(const FJamGameConfig& G, double Yaw, const FVector2D& StickDir, const FVector2D& DefPos, int32& Me, int32& Def)
	{
		UJamSim* Sim = NewObject<UJamSim>();
		Sim->Init(G, 1);
		Sim->SkipFaceoff();
		Me = Sim->GetHumanIndex();
		Def = -1;
		for (const FJamSkaterState& S : Sim->GetSkaters()) if (S.Team != Sim->GetSkaters()[Me].Team) { Def = S.Index; break; }
		const int32 N = Sim->GetSkaters().Num();
		for (int32 i = 0; i < N; ++i) Sim->SetSkater(i, FVector2D(-40.0 + i * 1.5, 19.0), FVector2D::ZeroVector, 0);
		Sim->SetSkater(Me, FVector2D::ZeroVector, FVector2D::ZeroVector, Yaw);
		Sim->SetPuck(StickDir.GetSafeNormal() * 0.45, FVector2D::ZeroVector);
		TArray<FJamInput> In;
		In.SetNum(N);
		In[Me].Stick = StickDir;
		for (int32 i = 0; i < 8; ++i) Sim->Step(In);
		Sim->SetSkater(Def, DefPos, FVector2D::ZeroVector, FMath::Atan2(-DefPos.Y, -DefPos.X));
		for (int32 i = 0; i < 2; ++i) Sim->Step(In);   // the defender's blade settles ahead of it
		return Sim;
	}

	// The defender presses the lift (bLift) or the poke (its blade aimed at the puck) once; true when the carrier still has the puck 12 frames later.
	// bClean: the sim called the press a clean take (its poke or lift event).
	bool Keeps(UJamSim* Sim, int32 Me, int32 Def, const FVector2D& StickDir, bool bLift, bool* bClean = nullptr)
	{
		if (bClean) *bClean = false;
		TArray<FJamInput> In;
		In.SetNum(Sim->GetSkaters().Num());
		In[Me].Stick = StickDir;
		const FVector2D ToPuck = Sim->GetPuck().Pos - Sim->GetSkaters()[Def].Pos;
		const double Reach = FMath::Max(0.01, Sim->GetConfig().Stick.ReachM);
		const FVector2D Aim = ToPuck.GetSafeNormal() * FMath::Clamp((ToPuck.Size() - UJamSim::SkaterRadius) / Reach, 0.2, 1.0);
		for (int32 F = 0; F < 12; ++F)
		{
			In[Def] = FJamInput();
			In[Def].Stick = Aim;
			if (F == 1) { In[Def].bLift = bLift; In[Def].bPoke = !bLift; }
			Sim->Step(In);
			if (bClean) for (const FJamSimEvent& E : Sim->GetFrameEvents()) if ((E.Name == JamEvents::Poke || E.Name == JamEvents::Lift) && E.Who == Def && E.Strength > 0.5f) *bClean = true;
		}
		return Sim->GetPuck().LastTouch == Me && Sim->CarrierIndex() == Me;
	}
}

// The evolution's number: a clean stop-and-go past a standing pro defender against a straight charge at it.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMovementStopAndGo, "Jam.Core.Movement.StopAndGo", JamCarrierSkillTests::Flags)
bool FJamMovementStopAndGo::RunTest(const FString& Parameters)
{
	using namespace JamCarrierSkillTests;
	UJamCartridge* C = LoadPreset(*this, TEXT("hockey.regular"));
	if (!C) return false;
	FScript Go = DefaultGo(), Straight = DefaultGo();
	Straight.bStopAndGo = false;
	const FShare A = Share(*C, Go, 200), B = Share(*C, Straight, 200);
	AddInfo(FString::Printf(TEXT("STOPGO pro clean stop-and-go (the stop begins at 9.5 m): %s"), *A.Line()));
	AddInfo(FString::Printf(TEXT("STOPGO pro straight charge: %s"), *B.Line()));
	// The same two carries with the neutral value of this lane's rule (game.stick.body_shield false): what the game was.
	UJamCartridge* N = Copy(*C);
	N->Game.Stick.bBodyShield = false;
	const FShare An = Share(*N, Go, 200), Bn = Share(*N, Straight, 200);
	AddInfo(FString::Printf(TEXT("STOPGO pro clean stop-and-go, body_shield false: %s"), *An.Line()));
	AddInfo(FString::Printf(TEXT("STOPGO pro straight charge, body_shield false: %s"), *Bn.Line()));
	// The game before the lane: both neutral values (body_shield false, passive_take true) and the teammate's touch.
	UJamCartridge* Old = Copy(*C);
	Old->Game.Stick.bBodyShield = false;
	Old->Game.Stick.bPassiveTake = true;
	Old->Game.Stick.bTeammateTouchTakes = true;
	const FShare Ao = Share(*Old, Go, 200), Bo = Share(*Old, Straight, 200);
	AddInfo(FString::Printf(TEXT("STOPGO pro clean stop-and-go, the game before the lane: %s"), *Ao.Line()));
	AddInfo(FString::Printf(TEXT("STOPGO pro straight charge, the game before the lane: %s"), *Bo.Line()));
	// The skill is the timing and the mouse: the same stop begun late (6 m), at the brief's 5 m, and with the blade left ahead.
	FScript Late = Go, Brief = Go, NoMouse = Go;
	Late.BrakeAtM = 6.0;
	Brief.BrakeAtM = 5.0;
	NoMouse.bStickAway = false;
	const FShare L = Share(*C, Late, 200), Br = Share(*C, Brief, 200), M = Share(*C, NoMouse, 200);
	AddInfo(FString::Printf(TEXT("STOPGO pro stop begun late, at 6 m: %s"), *L.Line()));
	AddInfo(FString::Printf(TEXT("STOPGO pro stop begun at the brief's 5 m: %s"), *Br.Line()));
	AddInfo(FString::Printf(TEXT("STOPGO pro clean stop-and-go, the blade left ahead (no mouse): %s"), *M.Line()));
	TestEqual(TEXT("every scene set up (the human carries when the brain takes the defender over)"), A.Ends[Setup] + B.Ends[Setup] + An.Ends[Setup] + Bn.Ends[Setup] + L.Ends[Setup] + Br.Ends[Setup] + M.Ends[Setup], 0);
	// Bars (measured 100.0 / 0.0 / 73.0 / 2.5 / 46.5 on the lane's tree; the brief asked two in three against one in five, a gap of 47 points).
	TestTrue(FString::Printf(TEXT("a clean stop-and-go beats a pro defender at least 8 times in 10 (%.1f %%)"), 100.0 * A.PastShare()), A.PastShare() >= 0.80);
	TestTrue(FString::Printf(TEXT("a straight charge beats it at most 1 time in 5 (%.1f %%)"), 100.0 * B.PastShare()), B.PastShare() <= 0.20);
	TestTrue(FString::Printf(TEXT("the gap between the two is at least 47 points (%.1f)"), 100.0 * (A.PastShare() - B.PastShare())), A.PastShare() - B.PastShare() >= 0.47);
	TestTrue(FString::Printf(TEXT("the body between is worth at least 10 points to the clean stop-and-go (%.1f against %.1f with body_shield false)"), 100.0 * A.PastShare(), 100.0 * An.PastShare()), A.PastShare() >= An.PastShare() + 0.10);
	TestTrue(FString::Printf(TEXT("timing is the skill: the stop begun at 6 m beats the defender at most 1 time in 4 (%.1f %%)"), 100.0 * L.PastShare()), L.PastShare() <= 0.25);
	TestTrue(FString::Printf(TEXT("the mouse is the skill: with the blade left ahead the stop-and-go is at least 20 points worse (%.1f %%)"), 100.0 * M.PastShare()), M.PastShare() <= A.PastShare() - 0.20);
	// Determinism of the scene: the same seed twice is the same carry.
	const FRun R1 = Carry(*C, 7, Go), R2 = Carry(*C, 7, Go);
	TestTrue(TEXT("the same seed is the same carry"), R1.End == R2.End && R1.Frames == R2.Frames);
	// A sim and a brain used again for the next seed (how the 200 are run) give the carry fresh ones give.
	{
		UJamSim* Sim = NewObject<UJamSim>();
		UJamBrain* Brain = NewObject<UJamBrain>();
		int32 Differ = 0;
		for (uint64 Seed = 1; Seed <= 12; ++Seed)
		{
			const FRun Used = Carry(*C, Seed, Seed % 2 ? Go : Straight, TEXT("pro"), nullptr, Sim, Brain), Fresh = Carry(*C, Seed, Seed % 2 ? Go : Straight);
			if (Used.End != Fresh.End || Used.Frames != Fresh.Frames) ++Differ;
		}
		TestEqual(TEXT("a re-used sim and brain play the same 12 carries as fresh ones"), Differ, 0);
	}
	return true;
}

// Puck protection (game.stick.body_shield): with the carrier's body between a defender's stick and the puck, that defender's poke or lift fails; no button.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMovementBodyShield, "Jam.Core.Movement.BodyShield", JamCarrierSkillTests::Flags)
bool FJamMovementBodyShield::RunTest(const FString& Parameters)
{
	using namespace JamCarrierSkillTests;
	UJamCartridge* C = LoadPreset(*this, TEXT("hockey.regular"));
	if (!C) return false;
	FJamGameConfig On = C->Game, Off = C->Game;
	On.Stick.bBodyShield = true;
	Off.Stick.bBodyShield = false;
	int32 Me = -1, Def = -1;
	struct FCase { const TCHAR* Name; double Yaw; FVector2D Stick; FVector2D DefPos; bool bShield; };
	const FCase Cases[] = {
		{ TEXT("defender behind, puck ahead"), 0.0, FVector2D(1, 0), FVector2D(-1.6, 0), true },
		{ TEXT("defender at one side, puck held to the other"), 0.0, FVector2D(0, 1), FVector2D(0, -1.6), true },
		{ TEXT("defender in front, puck ahead (the chest is open)"), 0.0, FVector2D(1, 0), FVector2D(1.6, 0.6), false },
		{ TEXT("defender in front, puck held behind the back (facing it shields nothing)"), 0.0, FVector2D(-1, 0), FVector2D(1.6, 0), false },
		{ TEXT("defender at the side, puck on its side"), 0.0, FVector2D(0, -1), FVector2D(0, -1.9), false },
		{ TEXT("defender behind but 2.5 m away"), 0.0, FVector2D(1, 0), FVector2D(-2.5, 0), false },
	};
	for (const FCase& K : Cases)
	{
		UJamSim* Sim = ShieldScene(On, K.Yaw, K.Stick, K.DefPos, Me, Def);
		TestEqual(FString::Printf(TEXT("%s: carried"), K.Name), Sim->CarrierIndex(), Me);
		const bool bGot = Sim->IsBodyShielded(Sim->GetSkaters()[Def], Sim->GetSkaters()[Me]);
		TestEqual(FString::Printf(TEXT("%s: body shield"), K.Name), bGot, K.bShield);
		TestEqual(FString::Printf(TEXT("%s: the poke's and the lift's shield follow it"), K.Name), Sim->IsPokeShielded(Sim->GetSkaters()[Def], Sim->GetSkaters()[Me]), K.bShield);
		UJamSim* SimOff = ShieldScene(Off, K.Yaw, K.Stick, K.DefPos, Me, Def);
		TestFalse(FString::Printf(TEXT("%s: never with body_shield false"), K.Name), SimOff->IsBodyShielded(SimOff->GetSkaters()[Def], SimOff->GetSkaters()[Me]));
	}
	// The lift from behind: refused with the body between, clean with the neutral value and from in front.
	{
		UJamSim* Sim = ShieldScene(On, 0.0, FVector2D(1, 0), FVector2D(-1.6, 0), Me, Def);
		TestTrue(TEXT("a lift from behind does not take a puck held ahead"), Keeps(Sim, Me, Def, FVector2D(1, 0), true));
		Sim = ShieldScene(Off, 0.0, FVector2D(1, 0), FVector2D(-1.6, 0), Me, Def);
		TestFalse(TEXT("body_shield false: the lift from behind takes it, as before"), Keeps(Sim, Me, Def, FVector2D(1, 0), true));
		Sim = ShieldScene(On, 0.0, FVector2D(1, 0), FVector2D(1.6, 0.6), Me, Def);
		TestFalse(TEXT("a lift from in front takes it: the body is not between"), Keeps(Sim, Me, Def, FVector2D(1, 0), true));
	}
	// The poke across the body: the defender at the side, the puck held close on the far side (a third of the aim), inside the reach of a blade that
	// has to pass through the carrier to get there.
	{
		UJamSim* Sim = ShieldScene(On, 0.0, FVector2D(0, 0.3), FVector2D(0, -0.9), Me, Def);
		TestTrue(TEXT("the poke's scene: the body is between"), Sim->IsBodyShielded(Sim->GetSkaters()[Def], Sim->GetSkaters()[Me]));
		bool bClean = false;
		TestTrue(TEXT("a poke across the body does not take the puck"), Keeps(Sim, Me, Def, FVector2D(0, 0.3), false, &bClean));
		TestFalse(TEXT("a poke across the body is not a clean poke"), bClean);
		Sim = ShieldScene(Off, 0.0, FVector2D(0, 0.3), FVector2D(0, -0.9), Me, Def);
		const bool bKeptOff = Keeps(Sim, Me, Def, FVector2D(0, 0.3), false, &bClean);
		// With the neutral value the blade reaches through the carrier and the sim calls it a clean poke (the puck may still bounce back off the body it was pulled through).
		AddInfo(FString::Printf(TEXT("BODYSHIELD poke across the body with body_shield false: clean poke %d, the carrier %s the puck"), bClean ? 1 : 0, bKeptOff ? TEXT("keeps") : TEXT("loses")));
		TestTrue(TEXT("body_shield false: the blade reaches through the body, a clean poke as before"), bClean);
	}
	// No state, no hash: two sims that differ only in the key hash alike while nobody presses a stick.
	{
		UJamSim* A = NewObject<UJamSim>();
		A->Init(On, 3);
		UJamSim* B = NewObject<UJamSim>();
		B->Init(Off, 3);
		TArray<FJamInput> In;
		In.SetNum(A->GetSkaters().Num());
		for (int32 F = 0; F < 200; ++F) { In[A->GetHumanIndex()].Move = FVector2D(1, 0); A->Step(In); B->Step(In); }
		TestEqual(TEXT("the key adds no state: the hash without a stick press is the same"), A->StateHashHex(), B->StateHashHex());
	}
	return true;
}

// R-0048 (game.stick.passive_take): a carried puck leaves its carrier only by a press. The human carries at 9 m/s past a standing skater whose aimed blade
// lies across its line (the deflection of C-0005), and past one whose body the puck brushes; an opponent and a teammate; both values of the key.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMovementPassiveTake, "Jam.Core.Movement.PassiveTake", JamCarrierSkillTests::Flags)
bool FJamMovementPassiveTake::RunTest(const FString& Parameters)
{
	using namespace JamCarrierSkillTests;
	UJamCartridge* C = LoadPreset(*this, TEXT("hockey.regular"));
	if (!C) return false;
	TestFalse(TEXT("game.stick.passive_take is false when the key is absent"), C->Game.Stick.bPassiveTake);
	TestTrue(TEXT("game.stick.body_shield is true when the key is absent"), C->Game.Stick.bBodyShield);
	FJamGameConfig Off = C->Game, On = C->Game;
	Off.Stick.bPassiveTake = false;
	On.Stick.bPassiveTake = true;
	// Frames (of 90) on which somebody else was the puck's last toucher; bBlade: the other's aimed blade lies across the line, else its body brushes the puck.
	auto Pass = [&](const FJamGameConfig& G, bool bBlade, bool bMate) -> int32
	{
		UJamSim* Sim = NewObject<UJamSim>();
		Sim->Init(G, 1);
		Sim->SkipFaceoff();
		const int32 Me = Sim->GetHumanIndex();
		const int32 N = Sim->GetSkaters().Num();
		int32 Other = -1;
		for (const FJamSkaterState& S : Sim->GetSkaters()) if (S.Index != Me && (S.Team == Sim->GetSkaters()[Me].Team) == bMate) { Other = S.Index; break; }
		if (Other < 0) return -1;
		for (int32 i = 0; i < N; ++i) Sim->SetSkater(i, FVector2D(-40.0 + i * 1.5, 19.0), FVector2D::ZeroVector, 0);
		const FVector2D Aim = bBlade ? FVector2D(1, 0) : FVector2D(1, 0.5).GetSafeNormal();
		Sim->SetSkater(Me, FVector2D(-6, 0), FVector2D(9, 0), 0);
		Sim->SetPuck(FVector2D(-6, 0) + Aim * 0.45, FVector2D(9, 0));
		// Blade: the other stands 1.3 m off the line and reaches across it. Body: it stands 0.95 m off the line, its back to it, and the puck (held half a metre to that side) brushes it.
		Sim->SetSkater(Other, bBlade ? FVector2D(4, 1.3) : FVector2D(4, 0.95), FVector2D::ZeroVector, UE_DOUBLE_PI * 0.5 * (bBlade ? -1.0 : 1.0));
		TArray<FJamInput> In;
		In.SetNum(N);
		int32 Taken = 0;
		for (int32 F = 0; F < 90; ++F)
		{
			In[Me].Move = FVector2D(0.89, 0);
			In[Me].Stick = Aim;
			if (bBlade) In[Other].Stick = FVector2D(0, -1);
			Sim->Step(In);
			if (F >= 10 && Sim->GetPuck().LastTouch != Me) ++Taken;
		}
		return Taken;
	};
	for (int32 Blade = 0; Blade < 2; ++Blade)
	{
		const TCHAR* What = Blade ? TEXT("an aimed blade across the line") : TEXT("a body the puck brushes");
		const int32 OppOff = Pass(Off, Blade != 0, false), OppOn = Pass(On, Blade != 0, false), MateOff = Pass(Off, Blade != 0, true), MateOn = Pass(On, Blade != 0, true);
		AddInfo(FString::Printf(TEXT("PASSIVE %s: frames the puck was somebody else's: opponent %d (passive_take false) / %d (true), teammate %d / %d"), What, OppOff, OppOn, MateOff, MateOn));
		TestEqual(FString::Printf(TEXT("%s, an opponent's, does not take a carried puck"), What), OppOff, 0);
		TestTrue(FString::Printf(TEXT("%s, an opponent's, takes it with passive_take true, as before"), What), OppOn > 0);
		TestEqual(FString::Printf(TEXT("%s, a teammate's, does not take a carried puck"), What), MateOff, 0);
		TestEqual(FString::Printf(TEXT("%s, a teammate's, does not take it with passive_take true either"), What), MateOn, 0);
	}
	// A shot is not a carried puck: a body two metres ahead still blocks it (the puck does not ride with the shooter).
	{
		UJamSim* Sim = NewObject<UJamSim>();
		Sim->Init(Off, 1);
		Sim->SkipFaceoff();
		const int32 Me = Sim->GetHumanIndex();
		const int32 N = Sim->GetSkaters().Num();
		int32 Opp = -1;
		for (const FJamSkaterState& S : Sim->GetSkaters()) if (S.Team != Sim->GetSkaters()[Me].Team) { Opp = S.Index; break; }
		for (int32 i = 0; i < N; ++i) Sim->SetSkater(i, FVector2D(-40.0 + i * 1.5, 19.0), FVector2D::ZeroVector, 0);
		Sim->SetSkater(Me, FVector2D::ZeroVector, FVector2D::ZeroVector, 0);
		Sim->SetPuck(FVector2D(0.45, 0), FVector2D::ZeroVector);
		TArray<FJamInput> In;
		In.SetNum(N);
		In[Me].Stick = FVector2D(1, 0);
		for (int32 i = 0; i < 8; ++i) Sim->Step(In);
		TestEqual(TEXT("the shot's scene: carried"), Sim->CarrierIndex(), Me);
		TestEqual(TEXT("the shot's scene: the puck rides"), Sim->RidingCarrier(), Me);
		Sim->SetSkater(Opp, FVector2D(2.0, 0), FVector2D::ZeroVector, 0);   // its back to the shooter: no blade in the way
		bool bBlocked = false;
		for (int32 F = 0; F < 20; ++F)
		{
			In[Me].bShoot = F == 0;
			Sim->Step(In);
			if (Sim->GetPuck().LastTouch == Opp) bBlocked = true;
		}
		TestTrue(TEXT("a shot from two metres is still blocked by the body in its way"), bBlocked);
	}
	return true;
}

// Outside the gate (Bar.*): the carry scene over the keys and the script's numbers. Rows come from Game/logs/a102/sweep.txt when it exists (one row a line:
// "name | key=value, key=value"; # starts a comment), so one build serves every sweep; without the file, the rows of docs/experiments/MOVEMENT-2.md section 3.
// Keys of the game: burst.frames, burst.accel_mult, reverse_brake_decel, brake_reach (code only), carry_reach_mult, carry_speed_mult, body_shield, passive_take,
// sweep.shield_frames, lift_range_m. Keys of the script: brake_at, cut, go_below, stick_away, stop, carry, defender_m, spread, stick_deg.
// Keys of the run: seeds, straight (0 = skip that arm), level (0 rookie, 1 pro, 2 all_star), trace (a seed to print frame by frame).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamMovementStopAndGoSweep, "Bar.Movement.StopAndGoSweep", JamCarrierSkillTests::Flags)
bool FJamMovementStopAndGoSweep::RunTest(const FString& Parameters)
{
	using namespace JamCarrierSkillTests;
	UJamCartridge* Base = LoadPreset(*this, TEXT("hockey.regular"));
	if (!Base) return false;
	TArray<FString> Rows;
	const FString File = FPaths::Combine(FPaths::ProjectDir(), TEXT("logs/a102/sweep.txt"));
	if (!FFileHelper::LoadFileToStringArray(Rows, *File))
	{
		Rows = {
			TEXT("defaults |"),
			TEXT("body_shield false | body_shield=0"),
			TEXT("passive_take true | passive_take=1"),
			TEXT("the game before the lane | body_shield=0, passive_take=1"),
			TEXT("the brief's script: brake at 5 m | brake_at=5"),
			TEXT("no first-step burst | burst.frames=0"),
			TEXT("no reverse brake | reverse_brake_decel=0"),
			TEXT("the blade stays ahead (no mouse) | stick_away=0"),
			TEXT("the cut without the stop | stop=0"),
			TEXT("rookie | level=0"),
			TEXT("all_star | level=2"),
		};
	}
	const TCHAR* Levels[3] = { TEXT("rookie"), TEXT("pro"), TEXT("all_star") };
	for (const FString& Raw : Rows)
	{
		FString Line = Raw.TrimStartAndEnd();
		if (Line.IsEmpty() || Line.StartsWith(TEXT("#"))) continue;
		FString Name = Line, Spec;
		Line.Split(TEXT("|"), &Name, &Spec);
		Name.TrimStartAndEndInline();
		UJamCartridge* C = Copy(*Base);
		FJamGameConfig& G = C->Game;
		FScript Go = DefaultGo();
		int32 Seeds = 200, Level = 1, TraceSeed = 0;
		bool bStraight = true;
		TArray<FString> Pairs;
		Spec.ParseIntoArray(Pairs, TEXT(","), true);
		for (const FString& Pair : Pairs)
		{
			FString K, VS;
			if (!Pair.Split(TEXT("="), &K, &VS)) continue;
			K.TrimStartAndEndInline();
			const double V = FCString::Atod(*VS.TrimStartAndEnd());
			if (K == TEXT("burst.frames")) G.Skater.BurstFrames = FMath::RoundToInt32(V);
			else if (K == TEXT("burst.accel_mult")) G.Skater.BurstAccelMult = V;
			else if (K == TEXT("reverse_brake_decel")) G.Skater.ReverseBrakeDecel = V;
			else if (K == TEXT("brake_reach")) G.Stick.BrakeReachMult = V;
			else if (K == TEXT("carry_reach_mult")) G.Stick.CarryReachMult = V;
			else if (K == TEXT("carry_speed_mult")) G.Skater.CarrySpeedMult = V;
			else if (K == TEXT("body_shield")) G.Stick.bBodyShield = V != 0.0;
			else if (K == TEXT("passive_take")) { G.Stick.bPassiveTake = V != 0.0; G.Stick.bTeammateTouchTakes = V != 0.0; }
			else if (K == TEXT("sweep.shield_frames")) G.Stick.SweepShieldFrames = FMath::RoundToInt32(V);
			else if (K == TEXT("lift_range_m")) G.Stick.LiftRangeM = V;
			else if (K == TEXT("brake_at")) Go.BrakeAtM = V;
			else if (K == TEXT("cut")) Go.CutDeg = V;
			else if (K == TEXT("go_below")) Go.GoBelowMps = V;
			else if (K == TEXT("stick_away")) Go.bStickAway = V != 0.0;
			else if (K == TEXT("stop")) Go.bBrake = V != 0.0;
			else if (K == TEXT("carry")) Go.CarryMps = V;
			else if (K == TEXT("defender_m")) Go.DefenderM = V;
			else if (K == TEXT("spread")) Go.Spread = V;
			else if (K == TEXT("stick_deg")) Go.StickDegPerFrame = V;
			else if (K == TEXT("seeds")) Seeds = FMath::Clamp(FMath::RoundToInt32(V), 1, 2000);
			else if (K == TEXT("straight")) bStraight = V != 0.0;
			else if (K == TEXT("level")) Level = FMath::Clamp(FMath::RoundToInt32(V), 0, 2);
			else if (K == TEXT("trace")) TraceSeed = FMath::RoundToInt32(V);
			else AddError(FString::Printf(TEXT("sweep row '%s': unknown key '%s'"), *Name, *K));
		}
		FScript Straight = Go;
		Straight.bStopAndGo = false;
		const FShare A = Share(*C, Go, Seeds, Levels[Level]);
		if (bStraight)
		{
			const FShare B = Share(*C, Straight, Seeds, Levels[Level]);
			AddInfo(FString::Printf(TEXT("SWEEP %s | go %.1f | straight %.1f | gap %.1f | go: %s | straight: %s"), *Name, 100.0 * A.PastShare(), 100.0 * B.PastShare(), 100.0 * (A.PastShare() - B.PastShare()), *A.Line(), *B.Line()));
		}
		else AddInfo(FString::Printf(TEXT("SWEEP %s | go %.1f | go: %s"), *Name, 100.0 * A.PastShare(), *A.Line()));
		if (TraceSeed > 0)
		{
			TArray<FString> Trace;
			Carry(*C, static_cast<uint64>(TraceSeed), Go, Levels[Level], &Trace);
			for (const FString& L : Trace) AddInfo(FString::Printf(TEXT("TRACE %s seed %d: %s"), *Name, TraceSeed, *L));
		}
	}
	return true;
}

#endif
