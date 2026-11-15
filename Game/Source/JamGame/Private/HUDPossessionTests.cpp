// Lane A213 (track U3, the night of 14 to 15 Nov; docs/experiments/SCREEN-TELLS.md): the parts of the screen's rules that need no world.
//   Jam.Hud.Possession   the four possession states (you / a mate / an opponent / nobody) from the SIM's carrier, on a staged hockey.regular sim; the
//                        pickup's pulse (it only shrinks, it ends at the ring's own size); the loose puck's ring is a hollow ring round the puck, wider than
//                        the puck and the blade's reach to it, so it never covers the puck; the double ring is wider than the ring it doubles.
//   Jam.Hud.Words        the pause card, the CONTROLS rows and the binding table say tonight's rules (no saucer, hold for power, hold to brace), and the
//                        words of before with jam.hud.Words 0; the badge never says SAUCER under the pass's power.
//   Jam.Hud.Teach        the pill's teaching pulses (three at most, never after he did it), the end card's wait, the shot line's growth.
//   Jam.Hud.OneLine      at most one line of the HUD's words at a time, by its order (contact, prompt, strip, faceoff hint, caption).
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "HAL/IConsoleManager.h"
#include "HUDWidget.h"
#include "InputMapping.h"
#include "MenuFlow.h"
#include "JamCartridge.h"
#include "JamSim.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudPossession, "Jam.Hud.Possession", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudPossession::RunTest(const FString& Parameters)
{
	using W = UJamHudWidget;
	// The pure rule.
	TestTrue(TEXT("nobody carries: loose"), W::PossessOf(-1, 3, 0, 0) == W::EPossess::Loose);
	TestTrue(TEXT("the human carries: you"), W::PossessOf(3, 3, 0, 0) == W::EPossess::You);
	TestTrue(TEXT("a man of his team: a mate"), W::PossessOf(4, 3, 0, 0) == W::EPossess::Mate);
	TestTrue(TEXT("a man of the other team: an opponent"), W::PossessOf(7, 3, 1, 0) == W::EPossess::Opponent);
	// The pulse: 1 + grow at the pickup, only shrinking, the ring's own size from PossessPulseFrames on (and before the pickup).
	TestEqual(TEXT("the pulse begins at 1 + grow"), W::PossessPulseScale(0), 1.f + W::PossessPulseGrow);
	TestEqual(TEXT("before the pickup: its own size"), W::PossessPulseScale(-1), 1.f);
	TestEqual(TEXT("after the pulse: its own size"), W::PossessPulseScale(W::PossessPulseFrames), 1.f);
	for (int32 A = 1; A <= W::PossessPulseFrames; ++A) TestTrue(TEXT("the pulse only shrinks"), W::PossessPulseScale(A) < W::PossessPulseScale(A - 1));
	TestTrue(TEXT("the pulse is short (a quarter second or less: a cue, not a show)"), W::PossessPulseFrames <= 15);
	// Nothing covers the puck: the loose ring is a ring (hollow) round the puck, its radius wider than the puck and the pulse only makes it wider.
	TestTrue(TEXT("the loose ring stands clear of the puck (radius over 4 x the puck's 3.8 cm)"), W::LooseRingRadiusM > 4.f * 0.038f);
	TestTrue(TEXT("the loose ring is smaller than a skater's ring (it is the puck's, not a man's)"), W::LooseRingRadiusM < W::CarrierRingRadiusM);
	TestTrue(TEXT("the double ring is wider than the ring it doubles (two rings, not one thick one)"), W::YouHaveRingScale >= 1.25f);

	// On the sim: the four states follow the sim's carrier.
	TArray<FString> Errors;
	UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")), Errors);
	for (const FString& E : Errors) AddError(E);
	if (!C) return false;
	UJamSim* Sim = NewObject<UJamSim>();
	Sim->Init(C->Game, 1);
	Sim->SkipFaceoff();
	const int32 Me = Sim->GetHumanIndex();
	const TArray<FJamSkaterState>& Sk = Sim->GetSkaters();
	int32 Mate = -1, Opp = -1;
	for (const FJamSkaterState& K : Sk)
	{
		if (K.Index == Me) continue;
		if (K.Team == Sk[Me].Team && Mate < 0) Mate = K.Index;
		if (K.Team != Sk[Me].Team && Opp < 0) Opp = K.Index;
	}
	TestTrue(TEXT("the staged rink has a mate and an opponent"), Mate >= 0 && Opp >= 0);
	if (Mate < 0 || Opp < 0) return false;
	const int32 N = Sk.Num();
	TArray<FJamInput> In;
	In.SetNum(N);
	auto Park = [&]() { for (int32 i = 0; i < N; ++i) Sim->SetSkater(i, FVector2D(-40.0 + i * 1.5, 19.0), FVector2D::ZeroVector, 0); };
	auto GiveTo = [&](int32 Who) -> W::EPossess
	{
		Park();
		Sim->SetSkater(Who, FVector2D::ZeroVector, FVector2D::ZeroVector, 0);
		Sim->SetPuck(FVector2D(0.45, 0.0), FVector2D::ZeroVector);
		for (FJamInput& I : In) I = FJamInput();
		In[Who].Stick = FVector2D(1.0, 0.0);
		for (int32 i = 0; i < 8; ++i) Sim->Step(In);
		return W::PossessFromSim(*Sim);
	};
	const W::EPossess You = GiveTo(Me);
	TestEqual(TEXT("the human's puck: the sim's carrier is the human"), Sim->CarrierIndex(), Me);
	TestTrue(TEXT("the human's puck reads YOU"), You == W::EPossess::You);
	const W::EPossess M = GiveTo(Mate);
	TestEqual(TEXT("the mate's puck: the sim's carrier is the mate"), Sim->CarrierIndex(), Mate);
	TestTrue(TEXT("the mate's puck reads MATE"), M == W::EPossess::Mate);
	const W::EPossess O = GiveTo(Opp);
	TestEqual(TEXT("the opponent's puck: the sim's carrier is the opponent"), Sim->CarrierIndex(), Opp);
	TestTrue(TEXT("the opponent's puck reads OPPONENT"), O == W::EPossess::Opponent);
	Park();
	Sim->SetPuck(FVector2D(10.0, -5.0), FVector2D(3.0, 0.0));
	for (FJamInput& I : In) I = FJamInput();
	Sim->Step(In);
	TestTrue(TEXT("a puck nobody is near: the sim's carrier is nobody"), Sim->CarrierIndex() < 0);
	TestTrue(TEXT("nobody's puck reads LOOSE"), W::PossessFromSim(*Sim) == W::EPossess::Loose);
	AddInfo(FString::Printf(TEXT("A213 possession: you=%s mate=%s opponent=%s loose=%s"), W::PossessName(You), W::PossessName(M), W::PossessName(O), W::PossessName(W::PossessFromSim(*Sim))));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudWords, "Jam.Hud.Words", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudWords::RunTest(const FString& Parameters)
{
	IConsoleVariable* V = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.hud.Words"));
	TestNotNull(TEXT("jam.hud.Words exists"), V);
	if (!V) return false;
	const int32 Was = V->GetInt();
	UJamInputMap* Map = UJamInputMap::Get();
	const FJamBindings Bound = Map->GetBindings();
	Map->ApplyBindings(FJamBindings::Defaults());
	auto RowNote = [](const TCHAR* Verb) { for (const FJamControlRow& R : UJamInputMap::ControlRowsFromBindings()) if (R.Verb == Verb) return R.Note; return FString(); };
	auto AllWords = [&]() { FString S = UJamInputMap::PauseCardText() + TEXT("\n") + UJamInputMap::BindingTableText(false); for (const FJamControlRow& R : UJamInputMap::ControlRowsFromBindings()) S += TEXT("\n") + R.Note; return S.ToUpper(); };

	V->Set(1, ECVF_SetByCode);
	const FString Card = UJamInputMap::PauseCardText();
	AddInfo(FString::Printf(TEXT("A213 pause card (tonight):\n%s"), *Card));
	TestFalse(TEXT("tonight no line says saucer (the hold is the pass's power)"), AllWords().Contains(TEXT("SAUCER")));
	TestFalse(TEXT("tonight no line says 'hold to hit' (the hold is the brace)"), AllWords().Contains(TEXT("HOLD TO HIT")));
	TestTrue(TEXT("the pause card: RMB pass, hold for power"), Card.Contains(TEXT("PASS / CHECK  RMB / RB  (HOLD FOR POWER)")));
	TestTrue(TEXT("the pause card: LMB shoot / cross-check, hold to lift / brace"), Card.Contains(TEXT("SHOOT / CROSS-CHECK  LMB / RT  (HOLD: LIFT THE SHOT / BRACE)")));
	TestTrue(TEXT("CONTROLS: the pass goes where the stick points, hold for power"), RowNote(TEXT("PASS (WITH THE PUCK)")).Contains(TEXT("stick points")) && RowNote(TEXT("PASS (WITH THE PUCK)")).Contains(TEXT("hold for power")));
	TestTrue(TEXT("CONTROLS: the shot lifts on a hold"), RowNote(TEXT("SHOOT (WITH THE PUCK)")).Contains(TEXT("hold to lift")));
	TestTrue(TEXT("CONTROLS: the cross-check braces on a hold"), RowNote(TEXT("CROSS-CHECK (WITHOUT)")).Contains(TEXT("brace")));
	TestTrue(TEXT("the binding table: PASS ... HOLD FOR POWER"), UJamInputMap::BindingTableText(false).Contains(TEXT("(WITH THE PUCK; HOLD FOR POWER)")));
	FJamButtonVerbs Held;
	Held.PassHoldSteps = 20;
	TestEqual(TEXT("the badge under the pass's power (saucer frames 0): a held pass is still PASS"), JamControls::BadgeLineHeld(true, Held, 0, 13), FString(TEXT("LMB  SHOOT     RMB  PASS")));

	V->Set(0, ECVF_SetByCode);
	TestTrue(TEXT("jam.hud.Words 0: the pause card of before (HOLD = SAUCER)"), UJamInputMap::PauseCardText().Contains(TEXT("(HOLD = SAUCER)")) || UJamInputMap::PauseCardText().Contains(TEXT("(WITH / WITHOUT THE PUCK)")));
	TestTrue(TEXT("jam.hud.Words 0: the CONTROLS row of before"), RowNote(TEXT("PASS (WITH THE PUCK)")).Contains(TEXT("saucer")));
	TestTrue(TEXT("jam.hud.Words 0: the binding table of before"), UJamInputMap::BindingTableText(false).Contains(TEXT("HOLD = SAUCER")));

	V->Set(Was, ECVF_SetByCode);
	Map->ApplyBindings(Bound);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudOneLine, "Jam.Hud.OneLine", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudOneLine::RunTest(const FString& Parameters)
{
	using W = UJamHudWidget;
	TestEqual(TEXT("nothing due: no line"), W::OneLineWinner(false, false, false, false, false), -1);
	TestEqual(TEXT("the first faceoff of a session (strip, faceoff hint, caption): the strip alone"), W::OneLineWinner(false, false, true, true, true), 2);
	TestEqual(TEXT("a later faceoff (hint, caption): the hint"), W::OneLineWinner(false, false, false, true, true), 3);
	TestEqual(TEXT("in play, a prompt and a caption: the prompt"), W::OneLineWinner(false, true, false, false, true), 1);
	TestEqual(TEXT("his press just landed (contact) over everything"), W::OneLineWinner(true, true, true, true, true), 0);
	TestEqual(TEXT("the caption alone stands"), W::OneLineWinner(false, false, false, false, true), 4);
	// Every combination of the five: exactly one line stands when any is due.
	for (int32 M = 1; M < 32; ++M)
	{
		const int32 Win = W::OneLineWinner(!!(M & 1), !!(M & 2), !!(M & 4), !!(M & 8), !!(M & 16));
		TestTrue(TEXT("one line, and one that was due"), Win >= 0 && Win < 5 && (M & (1 << Win)) != 0);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudTeach, "Jam.Hud.Teach", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudTeach::RunTest(const FString& Parameters)
{
	using W = UJamHudWidget;
	// Due on every frame of a long match: at most TeachMaxShows showings, each TeachShowFrames long, TeachGapFrames apart.
	{
		W::FTeachState S;
		int32 Frames = 0, Starts = 0;
		bool bWas = false;
		for (int64 F = 0; F < 20000; ++F)
		{
			const bool bOn = W::TeachStep(S, true, false, F);
			if (bOn) ++Frames;
			if (bOn && !bWas) ++Starts;
			bWas = bOn;
		}
		TestEqual(TEXT("a prompt shows at most three times a session"), Starts, W::TeachMaxShows);
		TestEqual(TEXT("each showing lasts TeachShowFrames"), Frames, W::TeachMaxShows * W::TeachShowFrames);
		AddInfo(FString::Printf(TEXT("A213 teach: due for 20000 frames: %d showings, %d frames on"), Starts, Frames));
	}
	{
		// He does the thing during the first showing: it ends at once and never comes back.
		W::FTeachState S;
		TestTrue(TEXT("first showing"), W::TeachStep(S, true, false, 100));
		TestFalse(TEXT("he did it: the pulse ends"), W::TeachStep(S, true, true, 110));
		bool bAny = false;
		for (int64 F = 111; F < 20000; ++F) bAny |= W::TeachStep(S, true, false, F);
		TestFalse(TEXT("never again after he has done it"), bAny);
	}
	{
		// He did it before it was ever due: it never shows.
		W::FTeachState S;
		W::TeachStep(S, false, true, 5);
		bool bAny = false;
		for (int64 F = 6; F < 5000; ++F) bAny |= W::TeachStep(S, true, false, F);
		TestFalse(TEXT("done before due: never shown"), bAny);
	}
	{
		// A new match (the frame count goes back) keeps the session's count but not the old match's gap.
		W::FTeachState S;
		W::TeachStep(S, true, false, 5000);
		for (int64 F = 5001; F < 5100; ++F) W::TeachStep(S, false, false, F);
		TestTrue(TEXT("after a restart the second showing may come"), W::TeachStep(S, true, false, 10) && S.Shows == 2);
	}
	TestTrue(TEXT("the pass's words name the button"), W::TeachLine(W::ETeach::Pass, TEXT("RMB")).StartsWith(TEXT("RMB  PASS")));
	TestTrue(TEXT("the check's words name the button"), W::TeachLine(W::ETeach::Hit, TEXT("RMB")).StartsWith(TEXT("RMB  CHECK")));
	TestTrue(TEXT("the pill's words fit the pill (32 characters at most)"), W::TeachLine(W::ETeach::Pass, TEXT("RMB")).Len() <= 32);
	// The end card waits for the goal to be seen; the shot's line grows with the hold.
	TestTrue(TEXT("the end card waits on the goal's frame"), W::EndCardWaits(true, true, 0.0));
	TestFalse(TEXT("the end card stands after the wait"), W::EndCardWaits(true, true, W::EndCardWaitSec + 0.01));
	TestFalse(TEXT("jam.hud.EndWait 0: no wait"), W::EndCardWaits(false, true, 0.0));
	TestTrue(TEXT("the wait is the GOAL banner's time (1.5 to 2.5 s)"), W::EndCardWaitSec >= 1.5 && W::EndCardWaitSec <= 2.5);
	for (int32 C = 1; C <= 30; ++C) TestTrue(TEXT("the shot's line only grows with the hold"), W::ShotLineM(C, 30) > W::ShotLineM(C - 1, 30));
	TestEqual(TEXT("the shot's line at a full charge"), W::ShotLineM(30, 30), W::ShotLineMaxM);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudEndStats, "Jam.Hud.EndStats", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FJamHudEndStats::RunTest(const FString& Parameters)
{
	using W = UJamHudWidget;
	W::FEndStats S;
	S.Shots = 4; S.Goals = 1; S.Passes = 9; S.Arrived = 6; S.Hits = 3; S.CarryFrames = 60 * 42;
	const FString T = W::EndStatsText(S);
	AddInfo(FString::Printf(TEXT("A213 end card lines: %s"), *T.Replace(TEXT("\n"), TEXT(" / "))));
	TArray<FString> Lines;
	T.ParseIntoArrayLines(Lines);
	TestEqual(TEXT("four lines"), Lines.Num(), 4);
	TestTrue(TEXT("shots and goals"), Lines.Num() > 0 && Lines[0] == TEXT("SHOTS 4   GOALS 1"));
	TestTrue(TEXT("passes that arrived of thrown"), Lines.Num() > 1 && Lines[1] == TEXT("PASSES THAT ARRIVED 6 OF 9"));
	TestTrue(TEXT("hits landed"), Lines.Num() > 2 && Lines[2] == TEXT("HITS LANDED 3"));
	TestTrue(TEXT("time with the puck as m:ss"), Lines.Num() > 3 && Lines[3] == TEXT("WITH THE PUCK 0:42"));
	for (const FString& L : Lines) TestTrue(TEXT("each line short enough for the card (30 characters)"), L.Len() <= 30);
	// 4e (FAULT): the winning goal on the frame the phase turns to Win is counted, and nothing after it.
	{
		W::FEndStats Card;
		int32 GoalsSeen = 0;
		bool bFinal = false;
		const int32 GoalsBySimFrame[] = { 0, 1, 1, 2, 3, 3, 3 };   // his goals as the sim counts them; the third lands on frame 4
		const bool bEndByFrame[] = { false, false, false, false, true, true, true };   // the phase turns to Win on that same frame
		for (int32 Fr = 0; Fr < 7; ++Fr)
		{
			if (!bEndByFrame[Fr]) bFinal = false;
			if (W::EndStatsCountsFrame(bEndByFrame[Fr], bFinal))
			{
				if (bEndByFrame[Fr]) bFinal = true;
				W::EndStatsAdd(Card.Goals, GoalsSeen, GoalsBySimFrame[Fr]);
			}
		}
		TestEqual(TEXT("the match-winning goal on the frame the phase turns is on the card"), Card.Goals, 3);
		TestTrue(TEXT("a frame in play counts"), W::EndStatsCountsFrame(false, false));
		TestTrue(TEXT("the first end frame counts"), W::EndStatsCountsFrame(true, false));
		TestFalse(TEXT("after it the card is frozen"), W::EndStatsCountsFrame(true, true));
	}
	// The own-net label's rule: his own third only (attack sign +1: his own net at -x).
	TestTrue(TEXT("deep in his own end: the own label shows"), W::InOwnThird(-40.f, 1.f, 100.f));
	TestFalse(TEXT("at centre ice: no own label"), W::InOwnThird(0.f, 1.f, 100.f));
	TestFalse(TEXT("in the attacking end: no own label"), W::InOwnThird(35.f, 1.f, 100.f));
	TestTrue(TEXT("the other side's attack mirrors it"), W::InOwnThird(40.f, -1.f, 100.f));
	return true;
}

#endif
