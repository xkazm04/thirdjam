// Lane A163 (track F2, evolution 6, docs/experiments/MOVEMENT-7.md): the player can see why. The parts of the carrier's cues that need no world.
//   Jam.Hud.CarrierCues   the take grace on the carrier's ring (yellow, 1.5 x its size closing to its own as the grace runs out), the stick lock and the
//                         press into a grace on the contact line; and, on a staged sim, that the states the cues read stand for as long as the rules say
//                         (the lock for game.stick.miss_recovery_frames after a person's miss, the grace for game.stick.take_grace_frames after a take).
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "HUDWidget.h"
#include "JamCartridge.h"
#include "JamSim.h"
#include "JamEventBus.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamCarrierCueTests
{
	// The carrier (an opposing bot seat) stands at the origin facing +x with the puck settled on its blade; the human stands 2.9 m in front of it, facing it.
	struct FCueScene
	{
		UJamSim* Sim = nullptr;
		int32 Car = -1, Me = -1;
		TArray<FJamInput> In;
	};
	FCueScene Stage(const FJamGameConfig& G)
	{
		FCueScene S;
		S.Sim = NewObject<UJamSim>();
		S.Sim->Init(G, 1);
		S.Sim->SkipFaceoff();
		S.Me = S.Sim->GetHumanIndex();
		for (const FJamSkaterState& K : S.Sim->GetSkaters()) if (K.Team != S.Sim->GetSkaters()[S.Me].Team) { S.Car = K.Index; break; }
		const int32 N = S.Sim->GetSkaters().Num();
		for (int32 i = 0; i < N; ++i) S.Sim->SetSkater(i, FVector2D(-40.0 + i * 1.5, 19.0), FVector2D::ZeroVector, 0);
		S.Sim->SetSkater(S.Car, FVector2D::ZeroVector, FVector2D::ZeroVector, 0);
		S.Sim->SetPuck(FVector2D(0.45, 0.0), FVector2D::ZeroVector);
		S.In.SetNum(N);
		S.In[S.Car].Stick = FVector2D(1.0, 0.0);
		for (int32 i = 0; i < 8; ++i) S.Sim->Step(S.In);
		S.Sim->SetSkater(S.Me, FVector2D(2.9, 0.0), FVector2D::ZeroVector, UE_DOUBLE_PI);
		return S;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudCarrierCues, "Jam.Hud.CarrierCues", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudCarrierCues::RunTest(const FString& Parameters)
{
	using namespace JamCarrierCueTests;
	// The ring: 1.5 x as the grace begins, its own size as it ends, never smaller, never larger; no grace or a cut without the rule: its own size.
	TestEqual(TEXT("the grace begins: the ring at 1.5 x"), UJamHudWidget::TakeGraceRingScale(30, 30), 1.5f);
	TestEqual(TEXT("half way: 1.25 x"), UJamHudWidget::TakeGraceRingScale(15, 30), 1.25f);
	TestEqual(TEXT("no grace: the ring's own size"), UJamHudWidget::TakeGraceRingScale(0, 30), 1.f);
	TestEqual(TEXT("take_grace_frames 0 (the rule off): the ring's own size"), UJamHudWidget::TakeGraceRingScale(5, 0), 1.f);
	TestEqual(TEXT("more left than the rule gives: clamped at 1.5 x"), UJamHudWidget::TakeGraceRingScale(90, 30), 1.5f);
	for (int32 L = 30; L > 0; --L) TestTrue(TEXT("the ring only closes"), UJamHudWidget::TakeGraceRingScale(L - 1, 30) < UJamHudWidget::TakeGraceRingScale(L, 30));
	// The contact line.
	TestEqual(TEXT("the stick lock"), UJamHudWidget::CarrierCueLine(true, false), FString(TEXT("STICK LOCKED")));
	TestEqual(TEXT("a press into a carrier's grace"), UJamHudWidget::CarrierCueLine(false, true), FString(TEXT("JUST WON IT")));
	TestEqual(TEXT("both: the lock (it is the one that costs him)"), UJamHudWidget::CarrierCueLine(true, true), FString(TEXT("STICK LOCKED")));
	TestTrue(TEXT("neither: no line"), UJamHudWidget::CarrierCueLine(false, false).IsEmpty());
	TestTrue(TEXT("the cue's words are not one of the contact line's own"), UJamHudWidget::ContactLineFor(true, true, true, true, true) != UJamHudWidget::CarrierCueLine(true, false)
		&& UJamHudWidget::ContactLineFor(false, false, false, true, false) != UJamHudWidget::CarrierCueLine(true, false));
	// When the lock ends the line starts its fade: its age is then exactly the frames the line stands at full strength.
	TestEqual(TEXT("the lock ended at frame 1000: the line fades from there"), 1000 - UJamHudWidget::CarrierCueFadeStart(1000), UJamHudWidget::ContactLineFrames - UJamHudWidget::ContactFadeFrames);

	// The states the cues read, on the sim (hockey.regular as it ships).
	TArray<FString> Errors;
	UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")), Errors);
	for (const FString& E : Errors) AddError(E);
	if (!C) return false;
	static const FName Miss(TEXT("miss")), Clean(TEXT("clean"));
	{
		// The human's short poke misses: the stick is locked for miss_recovery_frames and the line stands for exactly those frames.
		FJamGameConfig G = C->Game;
		G.Stick.PokeAssistDeg = 0.0;   // the blade rule, so the staged press misses
		const int32 N = G.Stick.MissRecoveryFrames;
		TestTrue(TEXT("hockey.regular ships the recovery (a cue with nothing to show would pass for nothing)"), N > 0);
		FCueScene S = Stage(G);
		S.In[S.Me].Stick = FVector2D(-0.5, 0.0);
		S.In[S.Me].bPoke = true;
		int32 Misses = 0, Guard = 0;
		while (Misses == 0 && Guard++ < G.Stick.PokeWindowFrames + 2)
		{
			S.Sim->Step(S.In);
			S.In[S.Me].bPoke = false;
			for (const FJamSimEvent& E : S.Sim->GetFrameEvents()) if (E.Name == JamEvents::Poke && E.Who == S.Me && E.Kind == Miss) ++Misses;
		}
		TestEqual(TEXT("the staged press is a miss"), Misses, 1);
		int32 LineFrames = 0;
		for (int32 F = 0; F < N + 10; ++F)
		{
			const FJamSkaterState& H = S.Sim->GetSkaters()[S.Me];
			if (!UJamHudWidget::CarrierCueLine(H.StickLockLeft > 0, false).IsEmpty()) ++LineFrames;
			S.Sim->Step(S.In);
		}
		TestEqual(TEXT("STICK LOCKED stands for miss_recovery_frames frames after the miss"), LineFrames, N);
		AddInfo(FString::Printf(TEXT("CARRIERCUES lock: the human's miss, the line STICK LOCKED for %d frames (game.stick.miss_recovery_frames %d)"), LineFrames, N));
	}
	{
		// The human's aimed poke takes the puck: his grace runs take_grace_frames and the ring closes from 1.5 x to its own size over it.
		FJamGameConfig G = C->Game;
		const int32 N = G.Stick.TakeGraceFrames;
		TestTrue(TEXT("hockey.regular ships the take grace"), N > 0);
		FCueScene S = Stage(G);
		TestFalse(TEXT("a placed puck's first carrier has no grace: no cue on it"), S.Sim->InTakeGrace(S.Sim->GetSkaters()[S.Car]));
		S.In[S.Me].Stick = FVector2D(-1.0, 0.0);
		S.In[S.Me].bPoke = true;
		bool bTook = false;
		for (int32 F = 0; F < G.Stick.PokeWindowFrames + 2 && !bTook; ++F)
		{
			S.Sim->Step(S.In);
			S.In[S.Me].bPoke = false;
			for (const FJamSimEvent& E : S.Sim->GetFrameEvents()) if (E.Name == JamEvents::Poke && E.Who == S.Me && E.Kind == Clean) bTook = true;
		}
		TestTrue(TEXT("the staged poke takes the puck"), bTook);
		int32 GraceFrames = 0;
		float Prev = 2.f, First = 0.f;
		bool bCloses = true;
		for (int32 F = 0; F < N + 10; ++F)
		{
			S.Sim->Step(S.In);
			const FJamSkaterState& H = S.Sim->GetSkaters()[S.Me];
			if (S.Sim->CarrierIndex() != S.Me || !S.Sim->InTakeGrace(H)) continue;
			const float Scale = UJamHudWidget::TakeGraceRingScale(H.TakeGraceLeft > 0 ? H.TakeGraceLeft : N, N);
			if (GraceFrames == 0) First = Scale;
			if (Scale > Prev) bCloses = false;
			Prev = Scale;
			++GraceFrames;
		}
		TestTrue(TEXT("the human carries inside his grace for most of take_grace_frames (the cue has the half second to show)"), GraceFrames >= N - 6 && GraceFrames <= N + 1);
		TestTrue(TEXT("the ring begins near 1.5 x and only closes"), First > 1.35f && bCloses && Prev < 1.1f);
		AddInfo(FString::Printf(TEXT("CARRIERCUES grace: the human took the puck, carried inside his grace for %d frames (game.stick.take_grace_frames %d), the ring from %.2f x to %.2f x"), GraceFrames, N, First, Prev));
	}
	return true;
}

#endif
