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

#endif
