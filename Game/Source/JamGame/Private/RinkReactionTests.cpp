// Lane A127 (track F3, evolution 3): the arena answers the game. Jam.Rink.GoalLampAndNet (five goals of a scripted match: the scorer's table's lamp and the
// goal light behind the scored net rise within 2 frames and are back by frame 120, the scored net's cords shake and settle, no flash without a goal, the sim's
// hash equal to a twin's that no rink listens to), Jam.Rink.BoardRattle (a hard skater `board` event rocks the piece behind that stretch of boards, scaled by
// Strength, and nothing else does), Jam.Rink.PenaltySeat (a boxed skater's figure is drawn at its box's Seat socket on every penalty frame and on the ice the
// frame the penalty ends). Headless: a transient game world holds the real ARinkActor, built from hockey.regular with /Game/Arena's meshes and device B's
// materials; the events travel through a UJamEventBus the way AHokejGameMode::DeliverEvents sends them (SetSimFrame, then Broadcast).
//   UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Rink; Quit" -unattended -nullrhi -Multiprocess -nosplash -log
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInterface.h"
#include "HAL/IConsoleManager.h"
#include "RinkActor.h"
#include "JamCartridge.h"
#include "JamSim.h"
#include "JamEventBus.h"
#include "JamPawns.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamRinkReactionTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

	// A game world of the test's own (never ticked): the rink's components register in it, its actors begin play by hand.
	struct FTestWorld
	{
		UWorld* World = nullptr;
		FTestWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, false, TEXT("A127RinkReactions"));
			FWorldContext& Ctx = GEngine->CreateNewWorldContext(EWorldType::Game);
			Ctx.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
		}
		~FTestWorld()
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}
	};

	struct FCvarGuard
	{
		IConsoleVariable* Var = nullptr;
		int32 Was = 1;
		explicit FCvarGuard(const TCHAR* Name) { Var = IConsoleManager::Get().FindConsoleVariable(Name); if (Var) Was = Var->GetInt(); }
		void Set(int32 V) { if (Var) Var->Set(V, ECVF_SetByCode); }
		~FCvarGuard() { if (Var) Var->Set(Was, ECVF_SetByCode); }
	};

	bool LoadRegular(FAutomationTestBase& T, FJamGameConfig& Out)
	{
		TArray<FString> Errors;
		UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")), Errors);
		for (const FString& E : Errors) T.AddError(FString::Printf(TEXT("hockey.regular: %s"), *E));
		if (!C) return false;
		Out = C->Game;
		Out.Look.Tier = TEXT("p3_clay");   // the shipped look: a dressed tier, so the Blender pieces stand
		return true;
	}

	ARinkActor* SpawnRink(FTestWorld& W, const FJamGameConfig& Cfg)
	{
		ARinkActor* Rink = W.World->SpawnActor<ARinkActor>();
		if (!Rink) return nullptr;
		if (!Rink->HasActorBegunPlay()) Rink->DispatchBeginPlay();
		Rink->Build(Cfg);
		return Rink;
	}

	UJamEventBus* MakeBus()
	{
		UJamEventBus* Bus = NewObject<UJamEventBus>(NewObject<UGameInstance>());
		Bus->bLogEvents = false;
		return Bus;
	}

	// AHokejGameMode::DeliverEvents for the sim's own events, then the rink's tick.
	void Deliver(UJamSim* Sim, UJamEventBus* Bus, ARinkActor* Rink)
	{
		Bus->SetSimFrame(Sim->GetFrame());
		for (const FJamSimEvent& E : Sim->GetFrameEvents())
		{
			if (!UJamEventBus::IsKnownEvent(E.Name)) continue;
			FJamEventPayload Pl;
			Pl.Who = E.Who;
			Pl.Where = UJamSim::ToWorld(E.Where, E.Name == JamEvents::Board || E.Name == JamEvents::Shot || E.Name == JamEvents::Goal ? 0.04 : 0.0);
			Pl.Strength = E.Strength;
			Pl.Kind = E.Kind;
			Pl.Dir = FVector(E.Dir.X, E.Dir.Y, 0.0);
			Bus->Broadcast(E.Name, Pl);
		}
		Rink->AdvanceReactions(Bus->GetSimFrame());
	}

	bool AtRest(const FVector& Scale) { return Scale.Equals(FVector::OneVector, 1e-6); }
	// A frame the run did not record reads as a value no check accepts.
	float At(const TMap<int64, float>& M, int64 F) { const float* V = M.Find(F); return V ? *V : -999.f; }
	FVector At(const TMap<int64, FVector>& M, int64 F) { const FVector* V = M.Find(F); return V ? *V : FVector(-999.0); }
}

using namespace JamRinkReactionTests;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamRinkGoalLampAndNet, "Jam.Rink.GoalLampAndNet", JamRinkReactionTests::Flags)
bool FJamRinkGoalLampAndNet::RunTest(const FString&)
{
	// The curves alone.
	TestEqual(TEXT("the lamp is full on the goal's frame"), ARinkActor::LampAt(0), 1.f);
	TestTrue(TEXT("the lamp is lit two frames in"), ARinkActor::LampAt(2) > 0.9f);
	TestEqual(TEXT("the lamp is dark before the goal"), ARinkActor::LampAt(-1), 0.f);
	TestEqual(TEXT("the lamp is dark from LampFlashFrames on"), ARinkActor::LampAt(ARinkActor::LampFlashFrames), 0.f);
	TestTrue(TEXT("the flash ends before frame 120"), ARinkActor::LampFlashFrames <= 120 && ARinkActor::NetShakeFrames <= 120);
	for (int64 F = 0; F < ARinkActor::LampFlashFrames; ++F) if (ARinkActor::LampAt(F) <= 0.f) { AddError(FString::Printf(TEXT("the lamp is dark inside its flash at frame %lld"), F)); break; }
	TestEqual(TEXT("the net is still from NetShakeFrames on"), ARinkActor::NetShakeAt(ARinkActor::NetShakeFrames), 0.f);

	FJamGameConfig Cfg;
	if (!LoadRegular(*this, Cfg)) return false;
	Cfg.Rules.Mode = TEXT("first_to");
	Cfg.Rules.ScoreToWin = 9;   // five goals without the match ending
	FTestWorld W;
	ARinkActor* Rink = SpawnRink(W, Cfg);
	if (!TestNotNull(TEXT("the rink spawned"), Rink)) return false;
	TestTrue(TEXT("the Blender pieces stand (p3_clay)"), Rink->IsArenaDressingShown());
	TestTrue(TEXT("the Blender goals stand"), Rink->IsArenaGoalShown());
	const float Rest = Rink->GetLampRestEmissive();
	if (!TestTrue(TEXT("the scorer's table has a lamp on a dynamic instance (slot `lamp`, a material with Emissive)"), Rest >= 0.f)) return false;
	TestEqual(TEXT("the lamp rests at its material's Emissive"), Rink->GetLampEmissive(), Rest);
	const bool bLights = Rink->GetGoalLightEmissive(1) >= 0.f && Rink->GetGoalLightEmissive(-1) >= 0.f;
	TestTrue(TEXT("a goal light stands behind each net"), bLights);
	UJamEventBus* Bus = MakeBus();
	Rink->BindBus(Bus);

	UJamSim* Sim = NewObject<UJamSim>();
	Sim->Init(Cfg, 1);
	Sim->SkipFaceoff();
	UJamSim* Twin = NewObject<UJamSim>();   // the same match with no bus and no rink
	Twin->Init(Cfg, 1);
	Twin->SkipFaceoff();
	TArray<FJamInput> In;
	In.Init(FJamInput(), Sim->GetSkaters().Num());

	const int32 Goals = 5;
	const int64 Gap = 260;   // frames between two scripted shots: a flash, then more than 120 quiet frames
	TArray<int64> GoalFrames;
	TArray<int32> GoalEnds;
	TMap<int64, float> Lamp, LightPos, LightNeg;
	TMap<int64, FVector> NetPos, NetNeg, Cd;   // Cd: the lamp lights' candelas (x the table's, y behind the -x net, z behind the +x net)
	int64 LastShot = -Gap;
	int32 Shots = 0, HashDiffs = 0;
	for (int32 Guard = 0; Guard < 4000 && (GoalFrames.Num() < Goals || Sim->GetFrame() < GoalFrames.Last() + 200); ++Guard)
	{
		if (Sim->GetPhase() == EJamSimPhase::Faceoff) { Sim->SkipFaceoff(); Twin->SkipFaceoff(); }
		if (Shots < Goals && Sim->GetPhase() == EJamSimPhase::Playing && Sim->GetFrame() - LastShot >= Gap)
		{
			// The script: skaters parked in a corner, the puck sent into a net from 1 m out; the ends alternate.
			const double Side = (Shots % 2 == 0) ? 1.0 : -1.0;
			for (const FJamSkaterState& S : Sim->GetSkaters())
			{
				const FVector2D P(-Side * (20.0 + S.Index * 1.5), 15.0);
				Sim->SetSkater(S.Index, P, FVector2D::ZeroVector, 0);
				Twin->SetSkater(S.Index, P, FVector2D::ZeroVector, 0);
			}
			const FVector2D From(Side * (Sim->GoalLineX() - 1.0), 0.3 * Side), Vel(Side * 12.0, 0.0);
			Sim->SetPuck(From, Vel);
			Twin->SetPuck(From, Vel);
			LastShot = Sim->GetFrame();
			++Shots;
		}
		const int32 Before = Sim->GetScore(0) + Sim->GetScore(1);
		Sim->Step(In);
		Twin->Step(In);
		Deliver(Sim, Bus, Rink);
		if (Sim->StateHash() != Twin->StateHash()) ++HashDiffs;
		const int64 F = Sim->GetFrame();
		if (Sim->GetScore(0) + Sim->GetScore(1) > Before)
		{
			GoalFrames.Add(F);
			GoalEnds.Add(Sim->GetPuck().Pos.X > 0.0 ? 1 : -1);
		}
		Lamp.Add(F, Rink->GetLampEmissive());
		LightPos.Add(F, Rink->GetGoalLightEmissive(1));
		LightNeg.Add(F, Rink->GetGoalLightEmissive(-1));
		NetPos.Add(F, Rink->GetNetShakeScale(1));
		NetNeg.Add(F, Rink->GetNetShakeScale(-1));
		Cd.Add(F, FVector(Rink->GetLampLightIntensity(0), Rink->GetLampLightIntensity(1), Rink->GetLampLightIntensity(2)));
	}
	TestEqual(TEXT("five goals in the scripted match"), GoalFrames.Num(), Goals);
	TestEqual(TEXT("the rink answered five goals"), Rink->GetGoalReactionCount(), Goals);
	TestEqual(TEXT("the sim's hash equals the twin's on every frame (nothing enters the sim)"), HashDiffs, 0);
	TestEqual(TEXT("final hash equals the twin's"), Sim->StateHashHex(), Twin->StateHashHex());

	int32 QuietFrames = 0, QuietLit = 0;
	for (const TPair<int64, float>& P : Lamp)
	{
		bool bNearGoal = false;
		for (const int64 G : GoalFrames) if (P.Key >= G && P.Key < G + 120) bNearGoal = true;
		if (bNearGoal) continue;
		++QuietFrames;
		if (!At(Cd, P.Key).IsZero()) ++QuietLit;
		if (P.Value != Rest || At(LightPos, P.Key) != Rest || At(LightNeg, P.Key) != Rest || !AtRest(At(NetPos, P.Key)) || !AtRest(At(NetNeg, P.Key))) ++QuietLit;
	}
	TestTrue(TEXT("there are frames without a goal in the last 120"), QuietFrames > 500);
	TestEqual(TEXT("no flash and no shake without a goal"), QuietLit, 0);

	for (int32 i = 0; i < GoalFrames.Num(); ++i)
	{
		const int64 G = GoalFrames[i];
		const int32 End = GoalEnds[i];
		const TMap<int64, float>& Scored = End > 0 ? LightPos : LightNeg;
		const TMap<int64, float>& Other = End > 0 ? LightNeg : LightPos;
		const TMap<int64, FVector>& ScoredNet = End > 0 ? NetPos : NetNeg;
		const TMap<int64, FVector>& OtherNet = End > 0 ? NetNeg : NetPos;
		float Peak2 = 0.f, PeakLight2 = 0.f;
		for (int64 F = G; F <= G + 2; ++F) { Peak2 = FMath::Max(Peak2, At(Lamp, F)); PeakLight2 = FMath::Max(PeakLight2, At(Scored, F)); }
		TestTrue(FString::Printf(TEXT("goal %d: the lamp's Emissive rises within 2 frames"), i + 1), Peak2 > Rest + 0.9f * ARinkActor::LampPeakEmissive);
		TestEqual(FString::Printf(TEXT("goal %d: the lamp was at rest the frame before"), i + 1), At(Lamp, G - 1), Rest);
		TestEqual(FString::Printf(TEXT("goal %d: the lamp is back by frame 120"), i + 1), At(Lamp, G + 120), Rest);
		int64 Back = -1;
		for (int64 F = G; F <= G + 120; ++F) if (At(Lamp, F) == Rest) { Back = F - G; break; }
		if (bLights)
		{
			TestTrue(FString::Printf(TEXT("goal %d: the goal light behind the scored net rises within 2 frames"), i + 1), PeakLight2 > Rest + 0.9f * ARinkActor::LampPeakEmissive);
			TestEqual(FString::Printf(TEXT("goal %d: that goal light is back by frame 120"), i + 1), At(Scored, G + 120), Rest);
		}
		const FVector CdGoal = At(Cd, G), CdAfter = At(Cd, G + 120);
		TestTrue(FString::Printf(TEXT("goal %d: the table's lamp light and the scored end's are on at the peak on the goal's frame"), i + 1),
			FMath::IsNearlyEqual(CdGoal.X, static_cast<double>(Rink->LampLightPeakAt(0)), 0.01) && FMath::IsNearlyEqual(End > 0 ? CdGoal.Z : CdGoal.Y, static_cast<double>(Rink->LampLightPeakAt(End > 0 ? 2 : 1)), 0.01)
			&& Rink->LampLightPeakAt(0) > 0.f && Rink->LampLightPeakAt(1) > Rink->LampLightPeakAt(0));
		TestEqual(FString::Printf(TEXT("goal %d: the other end's lamp light stays off"), i + 1), End > 0 ? CdGoal.Y : CdGoal.Z, 0.0);
		TestTrue(FString::Printf(TEXT("goal %d: every lamp light is off again by frame 120"), i + 1), CdAfter.IsZero());
		float Deep = 0.f;
		bool bOtherLit = false, bOtherMoved = false;
		for (int64 F = G; F < G + 120; ++F)
		{
			Deep = FMath::Max(Deep, static_cast<float>(FMath::Abs(At(ScoredNet, F).Y - 1.0)));
			if (bLights && At(Other, F) != Rest) bOtherLit = true;
			if (!AtRest(At(OtherNet, F))) bOtherMoved = true;
		}
		TestTrue(FString::Printf(TEXT("goal %d: the scored net's cords move (depth scale off rest by more than 5 %%)"), i + 1), Deep > 0.05f);
		TestTrue(FString::Printf(TEXT("goal %d: the wobble stays small (under 15 %%)"), i + 1), Deep < 0.15f);
		TestTrue(FString::Printf(TEXT("goal %d: the net is still again by frame 120"), i + 1), AtRest(At(ScoredNet, G + 120)));
		TestFalse(FString::Printf(TEXT("goal %d: the other end's light stays dark"), i + 1), bOtherLit);
		TestFalse(FString::Printf(TEXT("goal %d: the other net stands still"), i + 1), bOtherMoved);
		AddInfo(FString::Printf(TEXT("A127 goal %d frame %lld end %s: lamp %.2f -> %.2f on the goal's frame, %.2f two frames on, at rest again %lld frames after; scored light %.2f; net depth off rest by up to %.3f"),
			i + 1, G, End > 0 ? TEXT("+x") : TEXT("-x"), Rest, At(Lamp, G), At(Lamp, G + 2), Back, At(Scored, G), Deep));
	}

	// The neutral value: with jam.rink.react 0 a goal moves and lights nothing; and the same with jam.rink.arena 0 (the pieces are gone too).
	for (const TCHAR* Name : { TEXT("jam.rink.react"), TEXT("jam.rink.arena") })
	{
		FCvarGuard Guard(Name);
		Guard.Set(0);
		Rink->SetConstructionVisible(true);   // what the cvar sink does on a change
		const int32 Count = Rink->GetGoalReactionCount();
		FJamEventPayload Pl;
		Pl.Where = FVector(Sim->GoalLineX() * 100.0 + 50.0, 0.0, 4.0);
		Pl.Strength = 1.f;
		Bus->SetSimFrame(100000);
		Bus->Broadcast(JamEvents::Goal, Pl);
		Rink->AdvanceReactions(100001);
		TestEqual(FString::Printf(TEXT("%s 0: a goal starts no reaction"), Name), Rink->GetGoalReactionCount(), Count);
		TestEqual(FString::Printf(TEXT("%s 0: the lamp stays at rest"), Name), Rink->GetLampEmissive(), Rest);
		TestEqual(FString::Printf(TEXT("%s 0: no lamp light comes on"), Name), Rink->GetLampLightIntensity(0) + Rink->GetLampLightIntensity(1) + Rink->GetLampLightIntensity(2), 0.f);
		TestTrue(FString::Printf(TEXT("%s 0: the net stands still"), Name), AtRest(Rink->GetNetShakeScale(1)));
		TestFalse(FString::Printf(TEXT("%s 0: nothing is live"), Name), Rink->IsReacting());
	}
	Rink->SetConstructionVisible(true);
	TestTrue(TEXT("the cvars back at 1: the pieces stand again"), Rink->IsArenaDressingShown());
	Rink->BindBus(nullptr);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamRinkBoardRattle, "Jam.Rink.BoardRattle", JamRinkReactionTests::Flags)
bool FJamRinkBoardRattle::RunTest(const FString&)
{
	TestEqual(TEXT("the rattle is full on the hit's frame"), ARinkActor::RattleAt(0, 1.f), 1.f);
	TestEqual(TEXT("the rattle scales with Strength"), ARinkActor::RattleAt(0, 0.5f), 0.5f);
	TestEqual(TEXT("Strength is clamped to 1"), ARinkActor::RattleAt(0, 1.6f), 1.f);
	TestEqual(TEXT("the rattle is over at RattleFrames"), ARinkActor::RattleAt(ARinkActor::RattleFrames, 1.f), 0.f);

	FJamGameConfig Cfg;
	if (!LoadRegular(*this, Cfg)) return false;
	FTestWorld W;
	ARinkActor* Rink = SpawnRink(W, Cfg);
	if (!TestNotNull(TEXT("the rink spawned"), Rink)) return false;
	if (!TestTrue(TEXT("the Blender pieces stand (p3_clay)"), Rink->IsArenaDressingShown())) return false;
	UJamEventBus* Bus = MakeBus();
	Rink->BindBus(Bus);
	const double HalfW = Cfg.Rink.WidthM * 0.5 * 100.0, HalfL = Cfg.Rink.LengthM * 0.5 * 100.0;
	const FName Skater(TEXT("skater"));
	int64 Frame = 1000;
	// One hit: returns the piece's name when a rattle started ("" otherwise) and checks the run of that rattle.
	auto Hit = [&](const TCHAR* What, const FVector& Where, float Strength, FName Kind, const TCHAR* Expect)
	{
		Frame += 100;
		const int32 Count = Rink->GetRattleCount();
		FJamEventPayload Pl;
		Pl.Who = 0;
		Pl.Where = Where;
		Pl.Strength = Strength;
		Pl.Kind = Kind;
		Bus->SetSimFrame(Frame);
		Bus->Broadcast(JamEvents::Board, Pl);
		Rink->AdvanceReactions(Frame);
		const bool bStarted = Rink->GetRattleCount() == Count + 1;
		if (!Expect)
		{
			TestFalse(FString::Printf(TEXT("%s: nothing rocks"), What), bStarted);
			TestEqual(FString::Printf(TEXT("%s: no roll"), What), Rink->GetRattleDeg(), 0.f);
			return;
		}
		if (!TestTrue(FString::Printf(TEXT("%s: a piece rocks"), What), bStarted)) return;
		TestEqual(FString::Printf(TEXT("%s: the piece behind the hit"), What), Rink->GetRattledPieceName(), FString(Expect));
		const float Want = ARinkActor::RattleMaxDeg * FMath::Clamp(Strength, 0.f, 1.f);
		TestTrue(FString::Printf(TEXT("%s: the roll on the hit's frame is RattleMaxDeg x Strength"), What), FMath::IsNearlyEqual(Rink->GetRattleDeg(), Want, 1e-4f));
		UStaticMeshComponent* Piece = nullptr;
		for (UStaticMeshComponent* C : { Rink->ArenaBenchNeg.Get(), Rink->ArenaBenchPos.Get(), Rink->ArenaBoxNeg.Get(), Rink->ArenaBoxPos.Get(), Rink->ArenaTable.Get() }) if (C && C->GetName() == Expect) Piece = C;
		const FRotator Tilted = Piece ? Piece->GetRelativeRotation() : FRotator::ZeroRotator;
		TestTrue(FString::Printf(TEXT("%s: the component is rolled about its own x axis"), What), Piece && FMath::IsNearlyEqual(FMath::Abs(Tilted.Roll), Want, 1e-2f));
		float MaxLater = 0.f;
		for (int64 F = 1; F <= ARinkActor::RattleFrames; ++F)
		{
			Bus->SetSimFrame(Frame + F);
			Rink->AdvanceReactions(Frame + F);
			if (F < ARinkActor::RattleFrames) MaxLater = FMath::Max(MaxLater, FMath::Abs(Rink->GetRattleDeg()));
		}
		TestTrue(FString::Printf(TEXT("%s: the rock dies down"), What), MaxLater < Want);
		TestEqual(FString::Printf(TEXT("%s: no roll after RattleFrames"), What), Rink->GetRattleDeg(), 0.f);
		TestTrue(FString::Printf(TEXT("%s: the piece stands as Build put it"), What), Piece && FMath::IsNearlyZero(Piece->GetRelativeRotation().Roll, 1e-4) && FMath::IsNearlyZero(Piece->GetRelativeRotation().Pitch, 1e-4));
		TestFalse(FString::Printf(TEXT("%s: nothing is live after it"), What), Rink->IsReacting());
		AddInfo(FString::Printf(TEXT("A127 rattle %s: %s rolls %.2f deg at Strength %.2f, still again after %d frames"), What, Expect, Want, Strength, ARinkActor::RattleFrames));
	};
	Hit(TEXT("a hard hit at the -x penalty box"), FVector(-300.0, -HalfW, 4.0), 0.8f, Skater, TEXT("ArenaBoxNeg"));
	Hit(TEXT("a harder one at the +x penalty box"), FVector(350.0, -HalfW, 4.0), 1.4f, Skater, TEXT("ArenaBoxPos"));
	Hit(TEXT("a hit at the scorer's table"), FVector(20.0, -HalfW, 4.0), 0.6f, Skater, TEXT("ArenaTable"));
	Hit(TEXT("a hit at the -x team bench"), FVector(-0.0833 * 2.0 * HalfL, HalfW, 4.0), 0.5f, Skater, TEXT("ArenaBenchNeg"));
	Hit(TEXT("a hit 1.5 m past the +x bench's end"), FVector((0.0833 + 0.05) * 2.0 * HalfL + 150.0, HalfW, 4.0), 0.9f, Skater, TEXT("ArenaBenchPos"));
	Hit(TEXT("a soft hit (under RattleMinStrength)"), FVector(-300.0, -HalfW, 4.0), ARinkActor::RattleMinStrength - 0.05f, Skater, nullptr);
	Hit(TEXT("the puck on the boards"), FVector(-300.0, -HalfW, 4.0), 1.f, NAME_None, nullptr);
	Hit(TEXT("a hard hit on the end boards"), FVector(HalfL, 0.0, 4.0), 1.f, Skater, nullptr);
	Hit(TEXT("a hard hit on a long side where nothing stands"), FVector(HalfL * 0.7, -HalfW, 4.0), 1.f, Skater, nullptr);

	// A skater of the real sim driven into the -y boards in front of the -x penalty box: the sim's own `board` event, through the bus.
	{
		UJamSim* Sim = NewObject<UJamSim>();
		Sim->Init(Cfg, 1);
		Sim->SkipFaceoff();
		UJamSim* Twin = NewObject<UJamSim>();
		Twin->Init(Cfg, 1);
		Twin->SkipFaceoff();
		TArray<FJamInput> In;
		In.Init(FJamInput(), Sim->GetSkaters().Num());
		const FVector2D Start(-3.0, -(Cfg.Rink.WidthM * 0.5 - 1.2)), Vel(0.0, -Cfg.Skater.Speed * 0.9);
		Sim->SetSkater(0, Start, Vel, -UE_DOUBLE_HALF_PI);
		Twin->SetSkater(0, Start, Vel, -UE_DOUBLE_HALF_PI);
		const int32 Count = Rink->GetRattleCount();
		int32 BoardEvents = 0;
		float Strongest = 0.f;
		for (int32 F = 0; F < 40; ++F)
		{
			Sim->Step(In);
			Twin->Step(In);
			for (const FJamSimEvent& E : Sim->GetFrameEvents()) if (E.Name == JamEvents::Board && E.Kind == Skater) { ++BoardEvents; Strongest = FMath::Max(Strongest, E.Strength); }
			Deliver(Sim, Bus, Rink);
		}
		TestTrue(TEXT("the sim announced the skater's board hit"), BoardEvents >= 1);
		TestTrue(TEXT("the hit was a hard one"), Strongest >= ARinkActor::RattleMinStrength);
		TestTrue(TEXT("the rink rocked a piece for it"), Rink->GetRattleCount() > Count);
		TestEqual(TEXT("the piece is the -x penalty box"), Rink->GetRattledPieceName(), FString(TEXT("ArenaBoxNeg")));
		TestEqual(TEXT("the sim's hash equals the twin's"), Sim->StateHashHex(), Twin->StateHashHex());
		AddInfo(FString::Printf(TEXT("A127 rattle from the sim: %d skater board event(s), strongest %.2f, hash %s"), BoardEvents, Strongest, *Sim->StateHashHex()));
	}

	// The neutral value.
	{
		FCvarGuard Guard(TEXT("jam.rink.react"));
		Guard.Set(0);
		Rink->SetConstructionVisible(true);
		Hit(TEXT("jam.rink.react 0, a hard hit at the box"), FVector(-300.0, -HalfW, 4.0), 1.f, Skater, nullptr);
	}
	Rink->SetConstructionVisible(true);
	Rink->BindBus(nullptr);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamRinkPenaltySeat, "Jam.Rink.PenaltySeat", JamRinkReactionTests::Flags)
bool FJamRinkPenaltySeat::RunTest(const FString&)
{
	FJamGameConfig Cfg;
	if (!LoadRegular(*this, Cfg)) return false;
	Cfg.Rules.bPenalties = true;
	FTestWorld W;
	ARinkActor* Rink = SpawnRink(W, Cfg);
	if (!TestNotNull(TEXT("the rink spawned"), Rink)) return false;
	if (!TestTrue(TEXT("the Blender pieces stand (p3_clay)"), Rink->IsArenaDressingShown())) return false;
	TestTrue(TEXT("the rink serves the boxed skater's seat"), static_cast<bool>(AJamSkater::BoxSeat));

	// The drill of Jam.Core.Stick's foul: the human checks an opponent that has no puck; the fouler sits game.rules.penalty_frames.
	UJamSim* Sim = NewObject<UJamSim>();
	Sim->Init(Cfg, 1);
	Sim->SkipFaceoff();
	UJamSim* Twin = NewObject<UJamSim>();
	Twin->Init(Cfg, 1);
	Twin->SkipFaceoff();
	const int32 Me = Sim->GetHumanIndex();
	int32 O = -1;
	for (const FJamSkaterState& S : Sim->GetSkaters()) if (O < 0 && S.Team != Sim->GetSkaters()[Me].Team && !S.bHuman) O = S.Index;
	if (!TestTrue(TEXT("a human and an opponent"), Sim->GetSkaters().IsValidIndex(Me) && O >= 0)) return false;
	for (UJamSim* S2 : { Sim, Twin })
	{
		for (const FJamSkaterState& S : S2->GetSkaters()) if (S.Index != Me && S.Index != O) S2->SetSkater(S.Index, FVector2D(-35.0 + S.Index * 2.0, 18.0), FVector2D::ZeroVector, 0);
		S2->SetPuck(FVector2D(20, 15), FVector2D::ZeroVector);
		S2->SetSkater(O, FVector2D(0, 0), FVector2D::ZeroVector, 0);
		S2->SetSkater(Me, FVector2D(-1.2, 0), FVector2D(8, 0), 0);
	}
	const int32 Team = Sim->GetSkaters()[Me].Team;
	AJamSkater* Pawn = W.World->SpawnActor<AJamSkater>();
	if (!TestNotNull(TEXT("the figure's pawn spawned"), Pawn)) return false;
	Pawn->InitSkater(Me, Team, FLinearColor::Blue, 7, nullptr);
	FVector Seat;
	FRotator SeatRot;
	if (!TestTrue(TEXT("the team's box has a seat"), Rink->GetPenaltyBoxSeat(Team, Seat, SeatRot))) return false;

	TArray<FJamInput> In;
	In.SetNum(Sim->GetSkaters().Num());
	In[Me].Move = FVector2D(1, 0);
	In[Me].bCheck = true;
	int32 PenaltyFrames = 0, IceFrames = 0, OffSeat = 0, OffIce = 0, HashDiffs = 0;
	double WorstSeatCm = 0.0, FirstIceCm = -1.0, IceToSeatCm = 0.0;
	bool bWasBoxed = false, bReleased = false;
	for (int32 F = 0; F < Cfg.Rules.PenaltyFrames + 600 && !bReleased; ++F)
	{
		if (Sim->GetPhase() == EJamSimPhase::Faceoff) { Sim->SkipFaceoff(); Twin->SkipFaceoff(); }
		Sim->Step(In);
		Twin->Step(In);
		In[Me].bCheck = false;
		if (F >= 6) In[Me].Move = FVector2D::ZeroVector;
		if (Sim->StateHash() != Twin->StateHash()) ++HashDiffs;
		const FJamSkaterState& S = Sim->GetSkaters()[Me];
		Pawn->ApplyRender(S, 1.f);
		const FVector Ice = UJamSim::ToWorld(S.Pos);
		const double ToSeat = FVector::Dist(Pawn->GetActorLocation(), Seat), ToIce = FVector::Dist2D(Pawn->GetActorLocation(), Ice);
		if (S.InBox())
		{
			bWasBoxed = true;
			++PenaltyFrames;
			WorstSeatCm = FMath::Max(WorstSeatCm, ToSeat);
			IceToSeatCm = FVector::Dist2D(Ice, Seat);
			if (ToSeat > 20.0) ++OffSeat;
		}
		else
		{
			++IceFrames;
			if (ToIce > 1.0) ++OffIce;
			if (bWasBoxed) { bReleased = true; FirstIceCm = ToIce; }
		}
	}
	TestTrue(TEXT("the foul boxed the skater"), bWasBoxed);
	TestTrue(TEXT("the whole penalty was watched"), PenaltyFrames >= Cfg.Rules.PenaltyFrames - 8 && bReleased);
	TestEqual(TEXT("within 0.2 m of the Seat socket on every penalty frame"), OffSeat, 0);
	TestEqual(TEXT("on the ice on every other frame"), OffIce, 0);
	TestTrue(TEXT("back on the ice the frame the penalty ends"), bReleased && FirstIceCm >= 0.0 && FirstIceCm <= 1.0);
	TestEqual(TEXT("the sim's hash equals the twin's on every frame"), HashDiffs, 0);
	AddInfo(FString::Printf(TEXT("A127 seat: team %d, %d penalty frames, the figure at most %.2f cm from the Seat socket (the sim holds the skater %.0f cm from it, on the ice); on the ice %.2f cm from the sim's place the frame the penalty ends; hash %s"),
		Team, PenaltyFrames, WorstSeatCm, IceToSeatCm, FirstIceCm, *Sim->StateHashHex()));

	// The neutral value: jam.rink.react 0 draws the boxed skater where the sim holds it.
	{
		FJamSkaterState Boxed = Sim->GetSkaters()[Me];
		Boxed.PenaltyFrames = 100;
		Pawn->ApplyRender(Boxed, 1.f);
		TestTrue(TEXT("a boxed state is drawn at the seat"), FVector::Dist(Pawn->GetActorLocation(), Seat) <= 20.0);
		FCvarGuard Guard(TEXT("jam.rink.react"));
		Guard.Set(0);
		Pawn->ApplyRender(Boxed, 1.f);
		TestTrue(TEXT("jam.rink.react 0: the boxed skater is drawn on the ice"), FVector::Dist2D(Pawn->GetActorLocation(), UJamSim::ToWorld(Boxed.Pos)) <= 1.0);
	}
	{
		FJamSkaterState Boxed = Sim->GetSkaters()[Me];
		Boxed.PenaltyFrames = 100;
		FCvarGuard Guard(TEXT("jam.rink.arena"));
		Guard.Set(0);
		Rink->SetConstructionVisible(true);
		Pawn->ApplyRender(Boxed, 1.f);
		TestTrue(TEXT("jam.rink.arena 0: the boxed skater is drawn on the ice"), FVector::Dist2D(Pawn->GetActorLocation(), UJamSim::ToWorld(Boxed.Pos)) <= 1.0);
	}
	Rink->SetConstructionVisible(true);
	return true;
}

// Lane A136 (track F3, evolution 4): the whole boards answer a hit. Jam.Rink.BoardCaps: on three rinks a hard skater `board` event at every quarter metre of
// the outline finds a board cap within 2 m (100 % of the ring), the cap tilts and hops by the event's Strength on the hit's frame, its neighbours follow two
// and four frames later, everything stands again after CapRippleFrames; the caps add fewer than 3,000 triangles; a soft hit and the puck rock nothing; a
// skater of the real sim driven into the end boards rocks a cap and the sim's hash equals a twin's; `jam.rink.caps 0`, `jam.rink.react 0` and
// `jam.rink.arena 0` give back the still boards. And the loud goal's light: over the net by default, beside the lamp with `jam.rink.react.loud 0`.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamRinkBoardCaps, "Jam.Rink.BoardCaps", JamRinkReactionTests::Flags)
bool FJamRinkBoardCaps::RunTest(const FString&)
{
	TestEqual(TEXT("a cap's rock is full on the hit's frame"), ARinkActor::CapRockAt(0, 1.f), 1.f);
	TestEqual(TEXT("it scales with Strength"), ARinkActor::CapRockAt(0, 0.5f), 0.5f);
	TestEqual(TEXT("Strength is clamped to 1"), ARinkActor::CapRockAt(0, 1.7f), 1.f);
	TestEqual(TEXT("it is over at CapRockFrames"), ARinkActor::CapRockAt(ARinkActor::CapRockFrames, 1.f), 0.f);
	TestEqual(TEXT("the whole ripple lasts as long as a piece's rattle"), ARinkActor::CapRippleFrames, ARinkActor::RattleFrames);

	FJamGameConfig Base;
	if (!LoadRegular(*this, Base)) return false;
	const FName Skater(TEXT("skater"));
	const double Sizes[3][2] = { { 90.0, 45.0 }, { 120.0, 60.0 }, { 140.0, 70.0 } };
	for (const double* Size : Sizes)
	{
		FJamGameConfig Cfg = Base;
		Cfg.Rink.LengthM = Size[0];
		Cfg.Rink.WidthM = Size[1];
		const FString Tag = FString::Printf(TEXT("%.0f x %.0f"), Size[0], Size[1]);
		FTestWorld W;
		ARinkActor* Rink = SpawnRink(W, Cfg);
		if (!TestNotNull(*(Tag + TEXT(": the rink spawned")), Rink)) return false;
		if (!TestTrue(*(Tag + TEXT(": the caps stand (p3_clay)")), Rink->AreCapsShown())) return false;
		UJamEventBus* Bus = MakeBus();
		Rink->BindBus(Bus);
		const int32 N = Rink->GetCapCount();
		TArray<FVector2D> Line;
		ARinkActor::Outline(Cfg.Rink.LengthM, Cfg.Rink.WidthM, Cfg.Rink.CornerRadiusM, 12, Line);
		double Perimeter = 0.0;
		for (int32 i = 0; i < Line.Num(); ++i) Perimeter += FVector2D::Distance(Line[i], Line[(i + 1) % Line.Num()]);
		TestEqual(*(Tag + TEXT(": one cap at every stanchion place")), N, FMath::Max(4, FMath::RoundToInt32(Perimeter / ARinkActor::StanchionSpacingM)));
		TestEqual(*(Tag + TEXT(": one instance for every cap")), Rink->ArenaCaps && Rink->ArenaCapsPos ? Rink->ArenaCaps->GetInstanceCount() + Rink->ArenaCapsPos->GetInstanceCount() : -1, N);   // lane A145: the two halves
		TestTrue(*(Tag + TEXT(": the caps add fewer than 3,000 triangles")), Rink->GetCapTriangles() > 0 && Rink->GetCapTriangles() < 3000);

		// Every quarter metre of the outline: a skater's centre 0.45 m inside the boards, a hard hit.
		int64 Frame = 1000;
		int32 Samples = 0, Answered = 0, Within = 0;
		double MaxFromSkater = 0.0, MaxFromBoards = 0.0;
		for (int32 i = 0; i < Line.Num(); ++i)
		{
			const FVector2D A = Line[i], B = Line[(i + 1) % Line.Num()];
			const double Len = FVector2D::Distance(A, B);
			const FVector2D Dir = (B - A).GetSafeNormal();
			const FVector2D Out(Dir.Y, -Dir.X);
			for (double S = 0.0; S < Len; S += 0.25)
			{
				const FVector2D OnBoards = A + Dir * S;
				const FVector2D Centre = OnBoards - Out * 0.45;
				Frame += 40;
				const int32 Count = Rink->GetCapRockCount();
				FJamEventPayload Pl;
				Pl.Who = 0;
				Pl.Where = FVector(Centre.X * 100.0, Centre.Y * 100.0, 4.0);
				Pl.Strength = 0.6f;
				Pl.Kind = Skater;
				Bus->SetSimFrame(Frame);
				Bus->Broadcast(JamEvents::Board, Pl);
				++Samples;
				if (Rink->GetCapRockCount() != Count + 1) continue;
				++Answered;
				const int32 Cap = Rink->GetRockedCapIndex();
				const double FromSkater = FVector2D::Distance(Rink->GetCapPlaceM(Cap), Centre), FromBoards = FVector2D::Distance(Rink->GetCapPlaceM(Cap), OnBoards);
				MaxFromSkater = FMath::Max(MaxFromSkater, FromSkater);
				MaxFromBoards = FMath::Max(MaxFromBoards, FromBoards);
				if (FromSkater <= ARinkActor::RattleReachM && FMath::IsNearlyEqual(Rink->GetCapTiltDeg(Cap), ARinkActor::CapRockDeg * 0.6f, 1e-3f)) ++Within;
			}
		}
		TestEqual(*(Tag + TEXT(": every hard hit on the outline is answered by a cap")), Answered, Samples);
		TestEqual(*(Tag + TEXT(": the answering cap stands within 2 m of the skater and tilts by Strength, on 100 % of the outline")), Within, Samples);
		AddInfo(FString::Printf(TEXT("A136 caps %s: %d caps, %d triangles in all, outline %.1f m (one cap every %.2f m); %d hits every 0.25 m of it: %d answered, %d within 2 m (%.1f %%); the farthest cap %.2f m from the skater's centre, %.2f m from the point on the boards"),
			*Tag, N, Rink->GetCapTriangles(), Perimeter, Perimeter / FMath::Max(1, N), Samples, Answered, Within, 100.0 * Within / FMath::Max(1, Samples), MaxFromSkater, MaxFromBoards));

		// One hit followed frame by frame: the cap on the end boards behind the +x net.
		{
			Frame += 200;
			Bus->SetSimFrame(Frame);
			Rink->AdvanceReactions(Frame);
			TestFalse(*(Tag + TEXT(": the boards are still before the hit")), Rink->IsReacting());
			FJamEventPayload Pl;
			Pl.Who = 0;
			Pl.Where = FVector((Cfg.Rink.LengthM * 0.5 - 0.45) * 100.0, 150.0, 4.0);
			Pl.Strength = 0.9f;
			Pl.Kind = Skater;
			Bus->Broadcast(JamEvents::Board, Pl);
			const int32 Cap = Rink->GetRockedCapIndex();
			const int32 Next = (Cap + 1) % N, Far = (Cap + 3) % N;
			FTransform Rest, Hit;
			Hit = Rink->GetCapTransform(Cap);
			TestTrue(*(Tag + TEXT(": the struck cap tilts CapRockDeg x Strength on the hit's frame")), FMath::IsNearlyEqual(Rink->GetCapTiltDeg(Cap), ARinkActor::CapRockDeg * 0.9f, 1e-3f));
			TestTrue(*(Tag + TEXT(": and hops CapHopCm x Strength")), FMath::IsNearlyEqual(Rink->GetCapHopCm(Cap), ARinkActor::CapHopCm * 0.9f, 1e-3f));
			TestTrue(*(Tag + TEXT(": the instance itself is lifted off the board top")), FMath::IsNearlyEqual(Hit.GetLocation().Z, ARinkActor::BoardHeightM * 100.0 + ARinkActor::CapHopCm * 0.9, 1e-2));
			TestEqual(*(Tag + TEXT(": its neighbour waits")), Rink->GetCapTiltDeg(Next), 0.f);
			Bus->SetSimFrame(Frame + ARinkActor::CapRippleDelay);
			Rink->AdvanceReactions(Frame + ARinkActor::CapRippleDelay);
			TestTrue(*(Tag + TEXT(": the neighbour answers CapRippleDelay frames later with 0.55 of it")), FMath::IsNearlyEqual(Rink->GetCapTiltDeg(Next), ARinkActor::CapRockDeg * 0.9f * 0.55f, 1e-3f));
			TestEqual(*(Tag + TEXT(": a cap three places away never moves")), Rink->GetCapTiltDeg(Far), 0.f);
			float MaxLater = 0.f;
			for (int64 F = ARinkActor::CapRippleDelay + 1; F <= ARinkActor::CapRippleFrames; ++F)
			{
				Bus->SetSimFrame(Frame + F);
				Rink->AdvanceReactions(Frame + F);
				if (F < ARinkActor::CapRippleFrames) MaxLater = FMath::Max(MaxLater, FMath::Abs(Rink->GetCapTiltDeg(Cap)));
			}
			TestTrue(*(Tag + TEXT(": the rock dies down")), MaxLater < ARinkActor::CapRockDeg * 0.9f);
			TestFalse(*(Tag + TEXT(": nothing is live after CapRippleFrames")), Rink->IsReacting());
			bool bAllRest = true;
			for (int32 k = 0; k < N; ++k)
			{
				Rest = Rink->GetCapTransform(k);
				bAllRest = bAllRest && FMath::IsNearlyEqual(Rest.GetLocation().Z, ARinkActor::BoardHeightM * 100.0, 1e-3) && FMath::IsNearlyZero(Rest.Rotator().Roll, 1e-3) && Rink->GetCapTiltDeg(k) == 0.f;
			}
			TestTrue(*(Tag + TEXT(": every cap stands on the board top as Build put it")), bAllRest);
		}
		Rink->BindBus(nullptr);
	}

	// The regular rink: what does not rock a cap, several hits at once, the real sim, the neutral values, the loud goal's light.
	FJamGameConfig Cfg = Base;
	FTestWorld W;
	ARinkActor* Rink = SpawnRink(W, Cfg);
	if (!TestNotNull(TEXT("the rink spawned"), Rink)) return false;
	UJamEventBus* Bus = MakeBus();
	Rink->BindBus(Bus);
	const double HalfW = Cfg.Rink.WidthM * 0.5 * 100.0, HalfL = Cfg.Rink.LengthM * 0.5 * 100.0;
	int64 Frame = 500000;
	auto Hit = [&](const FVector& Where, float Strength, FName Kind) -> bool
	{
		const int32 Count = Rink->GetCapRockCount();
		FJamEventPayload Pl;
		Pl.Who = 0;
		Pl.Where = Where;
		Pl.Strength = Strength;
		Pl.Kind = Kind;
		Bus->SetSimFrame(Frame);
		Bus->Broadcast(JamEvents::Board, Pl);
		return Rink->GetCapRockCount() == Count + 1;
	};
	TestFalse(TEXT("a soft hit (under RattleMinStrength) rocks no cap"), Hit(FVector(HalfL - 45.0, 100.0, 4.0), ARinkActor::RattleMinStrength - 0.05f, Skater));
	TestFalse(TEXT("the puck on the boards rocks no cap"), Hit(FVector(HalfL - 45.0, 100.0, 4.0), 1.f, NAME_None));
	TestFalse(TEXT("nothing is live after them"), Rink->IsReacting());
	// Five hard hits on the same frame at five places: the four newest are live (CapRipples), every one of them at its own cap.
	{
		Frame += 100;
		const FVector Places[5] = { FVector(HalfL - 45.0, 0.0, 4.0), FVector(-HalfL + 45.0, 0.0, 4.0), FVector(0.0, HalfW - 45.0, 4.0), FVector(0.0, -HalfW + 45.0, 4.0), FVector(HalfL * 0.5, HalfW - 45.0, 4.0) };
		int32 Caps[5] = { -1, -1, -1, -1, -1 };
		for (int32 i = 0; i < 5; ++i) { TestTrue(TEXT("each of five hits at once finds a cap"), Hit(Places[i], 1.f, Skater)); Caps[i] = Rink->GetRockedCapIndex(); }
		int32 Tilted = 0;
		for (int32 i = 0; i < 5; ++i) if (FMath::IsNearlyEqual(Rink->GetCapTiltDeg(Caps[i]), ARinkActor::CapRockDeg, 1e-3f)) ++Tilted;
		TestEqual(TEXT("four ripples are live at once (the fifth hit took the oldest one's place)"), Tilted, ARinkActor::CapRipples);
		Frame += ARinkActor::CapRippleFrames;
		Bus->SetSimFrame(Frame);
		Rink->AdvanceReactions(Frame);
		TestFalse(TEXT("all of them are over CapRippleFrames later"), Rink->IsReacting());
	}
	// A skater of the real sim driven into the +x end boards beside the net: the sim's own `board` event, through the bus.
	{
		UJamSim* Sim = NewObject<UJamSim>();
		Sim->Init(Cfg, 1);
		Sim->SkipFaceoff();
		UJamSim* Twin = NewObject<UJamSim>();
		Twin->Init(Cfg, 1);
		Twin->SkipFaceoff();
		TArray<FJamInput> In;
		In.Init(FJamInput(), Sim->GetSkaters().Num());
		const FVector2D Start(Cfg.Rink.LengthM * 0.5 - 1.2, 6.0), Vel(Cfg.Skater.Speed * 0.9, 0.0);
		Sim->SetSkater(0, Start, Vel, 0.0);
		Twin->SetSkater(0, Start, Vel, 0.0);
		const int32 Count = Rink->GetCapRockCount(), Pieces = Rink->GetRattleCount();
		int32 Hard = 0;
		for (int32 F = 0; F < 40; ++F)
		{
			Sim->Step(In);
			Twin->Step(In);
			for (const FJamSimEvent& E : Sim->GetFrameEvents()) if (E.Name == JamEvents::Board && E.Kind == Skater && E.Strength >= ARinkActor::RattleMinStrength) ++Hard;
			Deliver(Sim, Bus, Rink);
		}
		TestTrue(TEXT("the sim announced a hard skater hit on the end boards"), Hard >= 1);
		TestEqual(TEXT("one cap answer per hard hit"), Rink->GetCapRockCount() - Count, Hard);
		TestEqual(TEXT("no bench, box or table stands behind the end boards: no piece rocked"), Rink->GetRattleCount(), Pieces);
		TestTrue(TEXT("the cap that answered stands within 2 m of the skater"), Rink->GetRockedCapDistanceM() <= static_cast<float>(ARinkActor::RattleReachM));
		TestEqual(TEXT("the sim's hash equals the twin's"), Sim->StateHashHex(), Twin->StateHashHex());
		AddInfo(FString::Printf(TEXT("A136 caps from the sim: %d hard skater board event(s) on the +x end boards, cap %d of %d answered from %.2f m, hash %s"), Hard, Rink->GetRockedCapIndex(), Rink->GetCapCount(), Rink->GetRockedCapDistanceM(), *Sim->StateHashHex()));
	}
	// The loud goal's light: over the net by default; `jam.rink.react.loud 0` puts it back beside the lamp at evolution 3's candelas.
	{
		const float Loud = Rink->LampLightPeakAt(2), LoudTable = Rink->LampLightPeakAt(0);
		const FVector Over = Rink->ArenaLampLights[2]->GetRelativeLocation();
		TestTrue(TEXT("the +x end's light hangs over the net, behind the goal line and inside the end boards"), Over.X > Cfg.Rink.GoalLineX() * 100.0 && Over.X < HalfL && FMath::IsNearlyZero(Over.Y, 1e-3) && FMath::IsNearlyEqual(Over.Z, ARinkActor::EndLightHeightM * 100.0, 1e-3));
		FCvarGuard Guard(TEXT("jam.rink.react.loud"));
		Guard.Set(0);
		Rink->SetConstructionVisible(true);
		const FVector Beside = Rink->ArenaLampLights[2]->GetRelativeLocation();
		TestTrue(TEXT("jam.rink.react.loud 0: the light stands beside the lamp, outside the end boards"), Beside.X > HalfL);
		TestTrue(TEXT("jam.rink.react.loud 0: half the candelas at the end, the table's as before"), FMath::IsNearlyEqual(Rink->LampLightPeakAt(2), Loud * 0.5f, 0.01f) && FMath::IsNearlyEqual(Rink->LampLightPeakAt(0), LoudTable, 0.01f));
		AddInfo(FString::Printf(TEXT("A136 loud goal: the end light at (%.0f, %.0f, %.0f) cm, %.0f cd (table %.0f cd); with jam.rink.react.loud 0 at (%.0f, %.0f, %.0f) cm, %.0f cd"), Over.X, Over.Y, Over.Z, Loud, LoudTable, Beside.X, Beside.Y, Beside.Z, Rink->LampLightPeakAt(2)));
	}
	Rink->SetConstructionVisible(true);
	// The neutral values.
	{
		FCvarGuard Guard(TEXT("jam.rink.caps"));
		Guard.Set(0);
		Rink->SetConstructionVisible(true);
		Frame += 100;
		const int32 Pieces = Rink->GetRattleCount();
		TestFalse(TEXT("jam.rink.caps 0: no caps stand"), Rink->AreCapsShown() || Rink->ArenaCaps->IsVisible() || Rink->ArenaCapsPos->IsVisible());
		TestFalse(TEXT("jam.rink.caps 0: a hard hit on the end boards rocks no cap"), Hit(FVector(HalfL - 45.0, 100.0, 4.0), 1.f, Skater));
		TestFalse(TEXT("jam.rink.caps 0: a hard hit at the penalty box rocks no cap"), Hit(FVector(-300.0, -HalfW, 4.0), 1.f, Skater));
		TestEqual(TEXT("jam.rink.caps 0: the penalty box still rocks (evolution 3's arena)"), Rink->GetRattleCount(), Pieces + 1);
	}
	Rink->SetConstructionVisible(true);
	TestTrue(TEXT("the caps are back with jam.rink.caps 1"), Rink->AreCapsShown() && Rink->ArenaCaps->IsVisible() && Rink->ArenaCapsPos->IsVisible());
	for (const TCHAR* Name : { TEXT("jam.rink.react"), TEXT("jam.rink.arena") })
	{
		FCvarGuard Guard(Name);
		Guard.Set(0);
		Rink->SetConstructionVisible(true);
		Frame += 100;
		TestFalse(FString::Printf(TEXT("%s 0: a hard hit rocks no cap"), Name), Hit(FVector(HalfL - 45.0, 100.0, 4.0), 1.f, Skater));
		TestFalse(FString::Printf(TEXT("%s 0: nothing is live"), Name), Rink->IsReacting());
	}
	Rink->SetConstructionVisible(true);
	Rink->BindBus(nullptr);
	return true;
}

// Lane A145 (track F3, evolution 5): Jam.Rink.HitSeenBoxUsed. The box in use on a real foul of the real sim (the door open within 12 frames, the clock's
// hand within 2 degrees of the penalty's own frames on every frame, the door again at the end, the other box still), the hit flash (candelas x Strength on
// the hit's frame at the hit's place, gone in HitFlashFrames, none for a soft hit or the puck), the goal wave (every cap behind the scored line within reach
// hops in the first pulse, the other end never, two lights ride the fronts, everything at rest when the lamp is), the caps as domes in the team colours,
// and the neutral values (`jam.rink.react.hit 0`, `jam.rink.react.wave 0`, `jam.rink.react 0`).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamRinkHitSeenBoxUsed, "Jam.Rink.HitSeenBoxUsed", JamRinkReactionTests::Flags)
bool FJamRinkHitSeenBoxUsed::RunTest(const FString&)
{
	struct FFloatGuard
	{
		IConsoleVariable* Var = nullptr;
		float Was = 0.f;
		explicit FFloatGuard(const TCHAR* Name) { Var = IConsoleManager::Get().FindConsoleVariable(Name); if (Var) Was = Var->GetFloat(); }
		void Set(float V) { if (Var) Var->Set(V, ECVF_SetByCode); }
		~FFloatGuard() { if (Var) Var->Set(Was, ECVF_SetByCode); }
	};
	TestEqual(TEXT("the flash is full on the hit's frame"), ARinkActor::HitFlashAt(0, 1.f), 1.f);
	TestEqual(TEXT("the flash scales with Strength"), ARinkActor::HitFlashAt(0, 0.5f), 0.5f);
	TestEqual(TEXT("the flash is over at HitFlashFrames"), ARinkActor::HitFlashAt(ARinkActor::HitFlashFrames, 1.f), 0.f);
	TestTrue(TEXT("the flash ends inside the cap ripple"), ARinkActor::HitFlashFrames <= ARinkActor::CapRippleFrames);
	TestTrue(TEXT("the door moves on the foul's frame"), ARinkActor::DoorAt(0) > 0.f);
	TestTrue(TEXT("the door is open within 12 frames"), ARinkActor::DoorOpenFrames <= 12 && ARinkActor::DoorAt(ARinkActor::DoorOpenFrames - 1) == 1.f && ARinkActor::DoorAt(11) == 1.f);
	TestEqual(TEXT("the door is closed again after its cycle"), ARinkActor::DoorAt(ARinkActor::DoorCycleFrames), 0.f);
	TestTrue(TEXT("a quarter turn takes the hand's top to the box's -x (clockwise for somebody on the ice)"),
		FRotator(90.0, 0.0, 0.0).RotateVector(FVector::UpVector).Equals(FVector(-1.0, 0.0, 0.0), 1e-6));

	FJamGameConfig Cfg;
	if (!LoadRegular(*this, Cfg)) return false;
	Cfg.Rules.bPenalties = true;
	FTestWorld W;
	ARinkActor* Rink = SpawnRink(W, Cfg);
	if (!TestNotNull(TEXT("the rink spawned"), Rink)) return false;
	if (!TestTrue(TEXT("the Blender pieces and the caps stand (p3_clay)"), Rink->IsArenaDressingShown() && Rink->AreCapsShown())) return false;
	UJamEventBus* Bus = MakeBus();
	Rink->BindBus(Bus);
	const double HalfW = Cfg.Rink.WidthM * 0.5 * 100.0, HalfL = Cfg.Rink.LengthM * 0.5 * 100.0;
	const FName Skater(TEXT("skater"));
	const int32 N = Rink->GetCapCount();

	// ---- The box in use: the drill of Jam.Rink.PenaltySeat (the human checks an opponent without the puck), events through the bus on every frame.
	if (!TestTrue(TEXT("the doors and the hands hang on the boxes"), Rink->IsBoxUsable())) return false;
	const int32 P = Cfg.Rules.PenaltyFrames > 0 ? Cfg.Rules.PenaltyFrames : 300;
	TestEqual(TEXT("the boxes count game.rules.penalty_frames"), Rink->GetBoxPenaltyFrames(), P);
	TestTrue(TEXT("at rest both doors are closed and both hands at the top"), Rink->GetBoxDoorDeg(0) == 0.f && Rink->GetBoxDoorDeg(1) == 0.f && Rink->GetBoxHandDeg(0) == 0.f && Rink->GetBoxHandDeg(1) == 0.f);
	{
		UJamSim* Sim = NewObject<UJamSim>();
		Sim->Init(Cfg, 1);
		Sim->SkipFaceoff();
		UJamSim* Twin = NewObject<UJamSim>();
		Twin->Init(Cfg, 1);
		Twin->SkipFaceoff();
		const int32 Me = Sim->GetHumanIndex();
		int32 O = -1;
		for (const FJamSkaterState& S : Sim->GetSkaters()) if (O < 0 && S.Team != Sim->GetSkaters()[Me].Team && !S.bHuman) O = S.Index;
		if (!TestTrue(TEXT("a human and an opponent"), Sim->GetSkaters().IsValidIndex(Me) && O >= 0)) return false;
		for (UJamSim* S2 : { Sim, Twin })
		{
			for (const FJamSkaterState& S : S2->GetSkaters()) if (S.Index != Me && S.Index != O) S2->SetSkater(S.Index, FVector2D(-35.0 + S.Index * 2.0, 18.0), FVector2D::ZeroVector, 0);
			S2->SetPuck(FVector2D(20, 15), FVector2D::ZeroVector);
			S2->SetSkater(O, FVector2D(0, 0), FVector2D::ZeroVector, 0);
			S2->SetSkater(Me, FVector2D(-1.2, 0), FVector2D(8, 0), 0);
		}
		const int32 Team = Sim->GetSkaters()[Me].Team, Other = 1 - Team;
		UStaticMeshComponent* DoorComp = Team == 0 ? Rink->ArenaBoxDoorNeg.Get() : Rink->ArenaBoxDoorPos.Get();
		UStaticMeshComponent* HandComp = Team == 0 ? Rink->ArenaBoxHandNeg.Get() : Rink->ArenaBoxHandPos.Get();
		TArray<FJamInput> In;
		In.SetNum(Sim->GetSkaters().Num());
		In[Me].Move = FVector2D(1, 0);
		In[Me].bCheck = true;
		int64 FoulAt = -1, ReleasedAt = -1;
		int32 Boxed = 0, HashDiffs = 0, OtherMoved = 0, CompOff = 0;
		float WorstHand = 0.f, DoorAt11 = -1.f, DoorOnFoul = -1.f, DoorMid = -1.f, DoorAfterRelease11 = -1.f, DoorOnRelease = -1.f;
		for (int32 F = 0; F < P + 900; ++F)
		{
			if (Sim->GetPhase() == EJamSimPhase::Faceoff) { Sim->SkipFaceoff(); Twin->SkipFaceoff(); }
			Sim->Step(In);
			Twin->Step(In);
			In[Me].bCheck = false;
			if (F >= 6) In[Me].Move = FVector2D::ZeroVector;
			if (Sim->StateHash() != Twin->StateHash()) ++HashDiffs;
			const int32 Before = Rink->GetBoxReactionCount();
			Deliver(Sim, Bus, Rink);
			const int64 Now = Sim->GetFrame();
			if (FoulAt < 0 && Rink->GetBoxReactionCount() > Before) { FoulAt = Now; DoorOnFoul = Rink->GetBoxDoorDeg(Team); }
			const FJamSkaterState& S = Sim->GetSkaters()[Me];
			if (Rink->GetBoxDoorDeg(Other) != 0.f || Rink->GetBoxHandDeg(Other) != 0.f) ++OtherMoved;
			if (DoorComp && !FMath::IsNearlyEqual(static_cast<float>(DoorComp->GetRelativeRotation().Yaw), Rink->GetBoxDoorDeg(Team), 0.01f)) ++CompOff;
			if (S.InBox())
			{
				++Boxed;
				// The penalty's own frames, as the sim holds them after this step: P - S.PenaltyFrames of P are served.
				const float Want = 360.f * static_cast<float>(P - S.PenaltyFrames) / static_cast<float>(P);
				float Err = FMath::Abs(Rink->GetBoxHandDeg(Team) - Want);
				Err = FMath::Min(Err, 360.f - Err);
				WorstHand = FMath::Max(WorstHand, Err);
				if (HandComp)
				{
					const FVector Tip = HandComp->GetRelativeRotation().RotateVector(FVector::UpVector);
					const FVector WantTip = FRotator(Rink->GetBoxHandDeg(Team), 0.0, 0.0).RotateVector(FVector::UpVector);
					if (!Tip.Equals(WantTip, 1e-3)) ++CompOff;
				}
				if (FoulAt >= 0 && Now - FoulAt == 11) DoorAt11 = Rink->GetBoxDoorDeg(Team);
				if (FoulAt >= 0 && Now - FoulAt == P / 2) DoorMid = Rink->GetBoxDoorDeg(Team);
			}
			else if (Boxed > 0 && ReleasedAt < 0) { ReleasedAt = Now; DoorOnRelease = Rink->GetBoxDoorDeg(Team); }
			if (ReleasedAt >= 0 && Now - ReleasedAt == 11) DoorAfterRelease11 = Rink->GetBoxDoorDeg(Team);
			if (ReleasedAt >= 0 && Now - ReleasedAt >= ARinkActor::DoorCycleFrames + 2) break;
		}
		TestTrue(TEXT("the sim called the foul and the box answered it once"), FoulAt >= 0 && Rink->GetBoxReactionCount() == 1);
		TestTrue(TEXT("the whole penalty was watched"), Boxed >= P - 8 && ReleasedAt >= 0);
		TestTrue(TEXT("the door moves on the foul's frame"), DoorOnFoul > 0.f);
		TestTrue(TEXT("the door is open within 12 frames of the foul"), FMath::IsNearlyEqual(DoorAt11, ARinkActor::DoorOpenDeg, 0.01f));
		TestEqual(TEXT("the door is closed in the middle of the penalty"), DoorMid, 0.f);
		TestTrue(TEXT("the clock's hand is within 2 degrees of the penalty's frames on every frame"), WorstHand <= 2.f);
		TestTrue(TEXT("the door opens again the frame the skater is back on the ice"), DoorOnRelease > 0.f);
		TestTrue(TEXT("and is open within 12 frames of it"), FMath::IsNearlyEqual(DoorAfterRelease11, ARinkActor::DoorOpenDeg, 0.01f));
		TestEqual(TEXT("the other team's box never moves"), OtherMoved, 0);
		TestEqual(TEXT("the door's and the hand's components hold what the read-outs say on every frame"), CompOff, 0);
		TestTrue(TEXT("after the door's second cycle the box is at rest"), Rink->GetBoxDoorDeg(Team) == 0.f && Rink->GetBoxHandDeg(Team) == 0.f);
		TestEqual(TEXT("the sim's hash equals the twin's on every frame"), HashDiffs, 0);
		AddInfo(FString::Printf(TEXT("A145 box: team %d fouled at sim frame %lld, %d boxed frames of a %d-frame penalty, released at %lld (foul + %lld); door %.1f deg on the foul's frame, %.1f at +11, %.1f on the release frame, %.1f at release +11; the hand at most %.3f deg from 360 x served / %d; hash %s"),
			Team, static_cast<long long>(FoulAt), Boxed, P, static_cast<long long>(ReleasedAt), static_cast<long long>(ReleasedAt - FoulAt), DoorOnFoul, DoorAt11, DoorOnRelease, DoorAfterRelease11, WorstHand, P, *Sim->StateHashHex()));
	}
	int64 Frame = 800000;
	Bus->SetSimFrame(Frame);
	Rink->AdvanceReactions(Frame);
	TestFalse(TEXT("nothing is live before the hits"), Rink->IsReacting());
	{
		// A foul by a skater of each team by its index (team 0's skaters first), and the still box with jam.rink.react 0.
		const int32 Split = Cfg.Teams.Num() > 0 ? Cfg.Teams[0].Skaters.Num() : 0;
		const int32 Count = Rink->GetBoxReactionCount();
		FJamEventPayload Fl;
		Fl.Who = Split;       // the first skater of team 1
		Fl.Where = FVector(0.0, 0.0, 0.0);
		Fl.Strength = 1.f;
		Fl.Kind = FName(TEXT("foul_no_puck"));
		Bus->Broadcast(JamEvents::Foul, Fl);
		TestTrue(TEXT("a foul by team 1's first skater opens team 1's box, not team 0's"), Rink->GetBoxReactionCount() == Count + 1 && Rink->GetBoxDoorDeg(1) > 0.f && Rink->GetBoxDoorDeg(0) == 0.f);
		Frame += P + ARinkActor::DoorCycleFrames + 1;
		Bus->SetSimFrame(Frame);
		Rink->AdvanceReactions(Frame);
		TestFalse(TEXT("the box is at rest after the penalty and the door's second cycle"), Rink->IsReacting());
		FCvarGuard Guard(TEXT("jam.rink.react"));
		Guard.Set(0);
		Rink->SetConstructionVisible(true);
		TestTrue(TEXT("jam.rink.react 0: both doors stand open as the box was built before the lane, the hands at the top"),
			FMath::IsNearlyEqual(Rink->GetBoxDoorDeg(0), ARinkActor::DoorOpenDeg, 0.01f) && FMath::IsNearlyEqual(Rink->GetBoxDoorDeg(1), ARinkActor::DoorOpenDeg, 0.01f) && Rink->GetBoxHandDeg(0) == 0.f);
		Bus->Broadcast(JamEvents::Foul, Fl);
		TestTrue(TEXT("jam.rink.react 0: a foul moves nothing"), Rink->GetBoxReactionCount() == Count + 1 && !Rink->IsReacting());
	}
	Rink->SetConstructionVisible(true);
	TestTrue(TEXT("the doors are closed again with jam.rink.react 1"), Rink->GetBoxDoorDeg(0) == 0.f && Rink->GetBoxDoorDeg(1) == 0.f);

	// ---- The caps: domes in the team colours, under the triangle budget.
	{
		int32 Blue = 0, Orange = 0;
		for (int32 k = 0; k < N; ++k) (Rink->GetCapPlaceM(k).X >= 0.0 ? Orange : Blue)++;
		TestTrue(TEXT("90 x 45: the caps are the dome"), Rink->AreCapsRound());
		TestEqual(TEXT("the dome is 40 triangles a cap"), Rink->GetCapTriangles(), N * 40);
		TestTrue(TEXT("the ring stays within the triangle budget"), Rink->GetCapTriangles() <= ARinkActor::CapTriangleBudget);
		TestEqual(TEXT("the x < 0 half's caps are ArenaCaps' instances"), Rink->ArenaCaps->GetInstanceCount(), Blue);
		TestEqual(TEXT("the x >= 0 half's caps are ArenaCapsPos' instances"), Rink->ArenaCapsPos->GetInstanceCount(), Orange);
		const UMaterialInterface* MatNeg = Rink->ArenaCaps->GetMaterial(0);
		const UMaterialInterface* MatPos = Rink->ArenaCapsPos->GetMaterial(0);
		TestTrue(TEXT("the two halves carry two different instances"), MatNeg && MatPos && MatNeg != MatPos);
		AddInfo(FString::Printf(TEXT("A145 caps: %d caps, %d triangles (%s), %d on %s (x < 0), %d on %s (x >= 0)"), N, Rink->GetCapTriangles(), Rink->AreCapsRound() ? TEXT("the dome") : TEXT("the far level"),
			Blue, MatNeg ? *MatNeg->GetName() : TEXT("none"), Orange, MatPos ? *MatPos->GetName() : TEXT("none")));
		FJamGameConfig Big = Cfg;
		Big.Rink.LengthM = 140.0;
		Big.Rink.WidthM = 70.0;
		FTestWorld W2;
		ARinkActor* BigRink = SpawnRink(W2, Big);
		if (TestNotNull(TEXT("the 140 x 70 rink spawned"), BigRink))
		{
			TestFalse(TEXT("140 x 70: more caps than the dome's budget allows, the rink takes the far level"), BigRink->AreCapsRound());
			TestTrue(TEXT("140 x 70: 24 triangles a cap, within the budget"), BigRink->GetCapTriangles() == BigRink->GetCapCount() * 24 && BigRink->GetCapTriangles() <= ARinkActor::CapTriangleBudget);
			AddInfo(FString::Printf(TEXT("A145 caps: 140 x 70: %d caps, %d triangles (the far level)"), BigRink->GetCapCount(), BigRink->GetCapTriangles()));
		}
	}

	// ---- The hit flash.
	auto LitLight = [&]() { int32 Lit = -1, Count = 0; for (int32 i = 0; i < ARinkActor::CapRipples; ++i) if (Rink->GetHitLightIntensity(i) > 0.f) { Lit = i; ++Count; } return Count == 1 ? Lit : -1 - Count; };
	auto Board = [&](const FVector& Where, float Strength, FName Kind, const FVector& Dir)
	{
		Frame += 100;
		Bus->SetSimFrame(Frame);
		Rink->AdvanceReactions(Frame);
		FJamEventPayload Pl;
		Pl.Who = 0;
		Pl.Where = Where;
		Pl.Strength = Strength;
		Pl.Kind = Kind;
		Pl.Dir = Dir;
		Bus->Broadcast(JamEvents::Board, Pl);
	};
	const float Peak = Rink->HitLightPeakHere();
	TestTrue(TEXT("the flash's candelas come from jam.rink.react.hit"), Peak > 0.f && FMath::IsNearlyEqual(Peak, ARinkActor::HitLightPeak(), 0.01f));
	{
		Board(FVector(1000.0, HalfW - 45.0, 4.0), 0.8f, Skater, FVector(0.0, -1.0, 0.0));
		const int32 Lit = LitLight();
		if (TestTrue(TEXT("a hard hit lights one flash"), Lit >= 0))
		{
			const FVector At = Rink->ArenaHitLights[Lit]->GetRelativeLocation();
			TestTrue(TEXT("candelas x Strength on the hit's frame"), FMath::IsNearlyEqual(Rink->GetHitLightIntensity(Lit), Peak * 0.8f, 0.5f));
			TestTrue(TEXT("at the hit, HitLightInM into the rink along the board's normal, HitLightHeightM up"),
				FMath::IsNearlyEqual(At.X, 1000.0, 0.1) && FMath::IsNearlyEqual(At.Y, HalfW - 45.0 - ARinkActor::HitLightInM * 100.0, 0.1) && FMath::IsNearlyEqual(At.Z, ARinkActor::HitLightHeightM * 100.0, 0.1));
			Bus->SetSimFrame(Frame + 6);
			Rink->AdvanceReactions(Frame + 6);
			TestTrue(TEXT("a quarter of it is left after six frames"), FMath::IsNearlyEqual(Rink->GetHitLightIntensity(Lit), Peak * 0.8f * 0.25f, 0.5f));
			Bus->SetSimFrame(Frame + ARinkActor::HitFlashFrames);
			Rink->AdvanceReactions(Frame + ARinkActor::HitFlashFrames);
			TestEqual(TEXT("dark after HitFlashFrames"), Rink->GetHitLightIntensity(Lit), 0.f);
			Bus->SetSimFrame(Frame + ARinkActor::CapRippleFrames);
			Rink->AdvanceReactions(Frame + ARinkActor::CapRippleFrames);
			TestFalse(TEXT("nothing is live after the ripple"), Rink->IsReacting());
		}
		Board(FVector(-1000.0, -(HalfW - 45.0), 4.0), 0.5f, Skater, FVector::ZeroVector);
		const int32 Lit2 = LitLight();
		if (TestTrue(TEXT("a weaker hard hit lights one flash too"), Lit2 >= 0))
		{
			const FVector At = Rink->ArenaHitLights[Lit2]->GetRelativeLocation();
			TestTrue(TEXT("its candelas follow its Strength"), FMath::IsNearlyEqual(Rink->GetHitLightIntensity(Lit2), Peak * 0.5f, 0.5f));
			TestTrue(TEXT("without a normal in the payload the flash still stands inside the rink"), FMath::Abs(At.Y) < HalfW - 45.0 && At.Y < 0.0);
		}
		Board(FVector(1000.0, HalfW - 45.0, 4.0), 0.3f, Skater, FVector(0.0, -1.0, 0.0));
		TestTrue(TEXT("a soft hit lights nothing new"), LitLight() == Lit2 || LitLight() == -1);
		Frame += 100;
		Board(FVector(1000.0, HalfW - 45.0, 4.0), 1.f, FName(TEXT("puck")), FVector(0.0, -1.0, 0.0));
		TestEqual(TEXT("the puck lights nothing"), LitLight(), -1);
		FFloatGuard Guard(TEXT("jam.rink.react.hit"));
		Guard.Set(0.f);
		const int32 Rocks = Rink->GetCapRockCount();
		Board(FVector(1000.0, HalfW - 45.0, 4.0), 1.f, Skater, FVector(0.0, -1.0, 0.0));
		TestTrue(TEXT("jam.rink.react.hit 0: the cap rocks and nothing lights (evolution 4's hit)"), Rink->GetCapRockCount() == Rocks + 1 && LitLight() == -1);
	}

	// ---- The goal wave.
	{
		Frame += 1000;
		Bus->SetSimFrame(Frame);
		Rink->AdvanceReactions(Frame);
		TestFalse(TEXT("nothing is live before the goal"), Rink->IsReacting());
		const double GoalLineM = Cfg.Rink.LengthM * 0.5 - 4.0;
		TArray<int32> EndCaps, OtherCaps;
		for (int32 k = 0; k < N; ++k)
		{
			const FVector2D At = Rink->GetCapPlaceM(k);
			if (At.X > GoalLineM && FMath::Abs(At.Y) <= ARinkActor::WaveReachM) EndCaps.Add(k);
			else if (At.X < 0.0 || FMath::Abs(At.Y) > ARinkActor::WaveReachM + 0.01) OtherCaps.Add(k);
		}
		TestTrue(TEXT("caps stand behind the +x goal line within the wave's reach"), EndCaps.Num() >= 4);
		FJamEventPayload Goal;
		Goal.Where = FVector((GoalLineM + 0.5) * 100.0, 0.0, 4.0);
		Goal.Strength = 1.f;
		Bus->Broadcast(JamEvents::Goal, Goal);
		TArray<float> MaxHop;
		MaxHop.SetNumZeroed(N);
		float WaveAt9 = -1.f, WaveMax = 0.f;
		FVector WavePlace9[2] = { FVector::ZeroVector, FVector::ZeroVector };
		int32 Hopping9 = 0, HoppingInFade = 0;
		for (int64 F = 0; F < ARinkActor::LampFlashFrames; ++F)
		{
			Bus->SetSimFrame(Frame + F);
			Rink->AdvanceReactions(Frame + F);
			if (F < ARinkActor::LampPulseFrames) for (int32 k = 0; k < N; ++k) MaxHop[k] = FMath::Max(MaxHop[k], Rink->GetCapHopCm(k));
			else for (const int32 k : OtherCaps) MaxHop[k] = FMath::Max(MaxHop[k], Rink->GetCapHopCm(k));
			WaveMax = FMath::Max(WaveMax, Rink->GetWaveLightIntensity(0));
			if (F == 9)
			{
				WaveAt9 = Rink->GetWaveLightIntensity(1);
				for (int32 s = 0; s < 2; ++s) WavePlace9[s] = Rink->ArenaWaveLights[s]->GetRelativeLocation();
				for (const int32 k : EndCaps) if (Rink->GetCapHopCm(k) > 0.f) ++Hopping9;
			}
			if (F == 99) for (const int32 k : EndCaps) if (Rink->GetCapHopCm(k) > 0.f) ++HoppingInFade;
		}
		int32 Hopped = 0, Strays = 0;
		for (const int32 k : EndCaps) if (MaxHop[k] > 1.f) ++Hopped;
		for (const int32 k : OtherCaps) if (MaxHop[k] > 0.f) ++Strays;
		const float EndPeak = Rink->LampLightPeakAt(2);
		TestEqual(TEXT("every cap behind the scored line within reach hops in the first pulse"), Hopped, EndCaps.Num());
		TestEqual(TEXT("no cap of the other end or beyond the reach moves in the whole flash"), Strays, 0);
		TestTrue(TEXT("caps are in the air in the lamp's trough (frame 9)"), Hopping9 > 0);
		TestTrue(TEXT("the wave still runs inside the lamp's fade (frame 99)"), HoppingInFade > 0);
		TestTrue(TEXT("each wave light carries half its share in the middle of a pulse"), FMath::IsNearlyEqual(WaveAt9, EndPeak * ARinkActor::WaveShare() * 0.5f, 1.f));
		TestTrue(TEXT("the wave lights never pass their share of the end light"), WaveMax <= EndPeak * ARinkActor::WaveShare() + 0.01f);
		TestTrue(TEXT("in the middle of a pulse the two lights stand half the reach to each side, inside the end boards"),
			FMath::IsNearlyEqual(WavePlace9[0].Y, -ARinkActor::WaveReachM * 50.0, 0.1) && FMath::IsNearlyEqual(WavePlace9[1].Y, ARinkActor::WaveReachM * 50.0, 0.1)
			&& WavePlace9[0].X < HalfL && WavePlace9[0].X > GoalLineM * 100.0);
		Bus->SetSimFrame(Frame + ARinkActor::LampFlashFrames);
		Rink->AdvanceReactions(Frame + ARinkActor::LampFlashFrames);
		bool bAllRest = Rink->GetWaveLightIntensity(0) == 0.f && Rink->GetWaveLightIntensity(1) == 0.f;
		for (int32 k = 0; k < N; ++k) bAllRest = bAllRest && Rink->GetCapHopCm(k) == 0.f && FMath::IsNearlyEqual(Rink->GetCapTransform(k).GetLocation().Z, ARinkActor::BoardHeightM * 100.0, 1e-3);
		TestTrue(TEXT("when the lamp is at rest every cap stands and the wave lights are dark"), bAllRest);
		TestFalse(TEXT("nothing is live after LampFlashFrames"), Rink->IsReacting());
		AddInfo(FString::Printf(TEXT("A145 wave: %d caps behind the +x line within %.0f m all hopped in the first pulse, %d in the air at frame 9; wave lights %.0f cd at frame 9 at y -+%.1f m, at most %.0f cd (the end light %.0f cd)"),
			EndCaps.Num(), ARinkActor::WaveReachM, Hopping9, WaveAt9, WavePlace9[1].Y / 100.0, WaveMax, EndPeak));
		FFloatGuard Guard(TEXT("jam.rink.react.wave"));
		Guard.Set(0.f);
		Frame += 1000;
		Bus->SetSimFrame(Frame);
		Bus->Broadcast(JamEvents::Goal, Goal);
		float AnyHop = 0.f, AnyLight = 0.f;
		for (int64 F = 0; F < 2 * ARinkActor::LampPulseFrames; ++F)
		{
			Bus->SetSimFrame(Frame + F);
			Rink->AdvanceReactions(Frame + F);
			for (int32 k = 0; k < N; ++k) AnyHop = FMath::Max(AnyHop, Rink->GetCapHopCm(k));
			AnyLight = FMath::Max(AnyLight, FMath::Max(Rink->GetWaveLightIntensity(0), Rink->GetWaveLightIntensity(1)));
		}
		TestTrue(TEXT("jam.rink.react.wave 0: a goal moves no cap and lights no wave (evolution 4's goal)"), AnyHop == 0.f && AnyLight == 0.f && Rink->GetGoalLightEmissive(1) > Rink->GetLampRestEmissive());
		Bus->SetSimFrame(Frame + 200);
		Rink->AdvanceReactions(Frame + 200);
	}
	Rink->BindBus(nullptr);
	return true;
}

#endif
