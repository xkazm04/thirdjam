// Lane A211 (track E1, the eye): jam.ScriptHuman 2. See ScriptHuman2.h. CAPTURE ONLY: nothing here runs unless jam.ScriptHuman is 2.
#include "ScriptHuman2.h"
#include "JamSimAdapter.h"
#include "JamPlaytester.h"
#include "JamHitHand.h"
#include "InputMapping.h"
#include "InputCoreTypes.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogJamScript, Log, All);

static TAutoConsoleVariable<int32> CVarJamScriptHumanKind(TEXT("jam.ScriptHumanKind"), -1,
	TEXT("With jam.ScriptHuman 2: the playtester's class. -1 (default) by the level (rookie = newcomer, pro = regular, all_star = good); 0 newcomer, 1 regular, 2 good. Read at the start of a match. Capture only (lane A211)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamScriptHumanHand(TEXT("jam.ScriptHumanHand"), 1,
	TEXT("With jam.ScriptHuman 2: 1 (default) lane A209's person-like hand presses the hits (JamHitHand.h); 0 the playtester's own presses. Capture only (lane A211)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamScriptHumanButtons(TEXT("jam.ScriptHumanButtons"), 1,
	TEXT("With jam.ScriptHuman 2: 1 (default) the two-role presses go through the button resolver a person's keys go through (LMB / RMB by possession, holds by the hand's thresholds); 0 the playtester's verbs straight into the input. Capture only (lane A211)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamScriptHumanSprint(TEXT("jam.ScriptHumanSprint"), 1,
	TEXT("With jam.ScriptHuman 2: 1 (default) the regular and the good player sprint (the playtester's -playersprint); 0 never. Read at the start of a match. Capture only (lane A211)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamScriptHumanSeed(TEXT("jam.ScriptHumanSeed"), 0,
	TEXT("With jam.ScriptHuman 2: the seed of the playtester and its hand; 0 (default) = the cartridge's seed. Read at the start of a match. Capture only (lane A211)."), ECVF_Default);

namespace
{
	struct FScriptHuman2
	{
		const UJamSim* Sim = nullptr;
		int64 LastFrame = -1;
		int64 LastDrop = -1;
		bool bDrawWasOpen = false;
		JamAI::FJamPlaytester Tester;
		JamAI::FJamHitHand Hand;
		JamAI::EPlaytester Kind = JamAI::EPlaytester::Regular;
		bool bHand = true;
		FJamButtonResolver Buttons;
		FJamBindings Bindings;
		bool bLmbDown = false, bRmbDown = false;
		bool bLmbTapWas = false, bRmbTapWas = false;   // the playtester asked for a tap on the frame before
		int64 LmbTapAt = -1000, RmbTapAt = -1000;      // the frame of the last tap sent
		int64 Frames = 0, LmbPresses = 0, RmbPresses = 0, Shots = 0, Passes = 0, Checks = 0, Lifts = 0, HoldFrames = 0, Dead = 0;
	};
	FScriptHuman2 G;

	// JamAI::PlaytesterName is not exported from JamCore: the three names here.
	const TCHAR* KindName(JamAI::EPlaytester K) { return K == JamAI::EPlaytester::Novice ? TEXT("novice") : K == JamAI::EPlaytester::Good ? TEXT("good") : TEXT("regular"); }

	JamAI::EPlaytester KindFor(const FName& Level)
	{
		const int32 K = CVarJamScriptHumanKind.GetValueOnGameThread();
		if (K == 0) return JamAI::EPlaytester::Novice;
		if (K == 1) return JamAI::EPlaytester::Regular;
		if (K == 2) return JamAI::EPlaytester::Good;
		const FString L = Level.ToString().ToLower();
		if (L == TEXT("rookie")) return JamAI::EPlaytester::Novice;
		if (L.Contains(TEXT("star"))) return JamAI::EPlaytester::Good;
		return JamAI::EPlaytester::Regular;
	}

	// A new match (another sim, or its frame went back): the playtester as RunPlaytestMatch builds it with no measurement arm set (JamPlaytester.cpp):
	// the check press, the brace and the poke read follow the cartridge's own switches.
	void Begin(const UJamSim& Sim, const FName& Level, uint64 CutSeed)
	{
		G = FScriptHuman2();
		G.Sim = &Sim;
		G.Kind = KindFor(Level);
		const int32 SeedCvar = CVarJamScriptHumanSeed.GetValueOnGameThread();
		const uint64 Seed = SeedCvar != 0 ? static_cast<uint64>(SeedCvar) : CutSeed;
		const FJamGameConfig& Cfg = Sim.GetConfig();
		G.Tester.Reset(G.Kind, Seed);
		G.Tester.ChargeFramesMax = Cfg.Shot.ChargeFramesMax;
		G.Tester.bSprint = CVarJamScriptHumanSprint.GetValueOnGameThread() != 0;
		G.Tester.bOldCheck = !Cfg.Contact.bPlaytesterClosingPress;
		G.Tester.bCourseCheck = !G.Tester.bOldCheck && Cfg.Contact.bPlaytesterCoursePress;
		G.Tester.BraceMode = (Cfg.Contact.bCrossHold && Cfg.Contact.bMomentum && Cfg.Skater.bCheckEnabled) ? 1 : 0;
		G.Tester.bPokeReadOn = Cfg.Stick.bPlaytesterPokeRead && !Cfg.Stick.bPokeRulesOff;
		G.bHand = CVarJamScriptHumanHand.GetValueOnGameThread() != 0;
		G.Hand = JamAI::FJamHitHand();
		G.Hand.Seed = Seed ^ 0xA209ull;   // as RunPlaytestMatch seeds it
		G.Bindings = FJamBindings::Defaults();
		UE_LOG(LogJamScript, Log, TEXT("JAMSCRIPT human=2 kind=%s level=%s hand=%d buttons=%d sprint=%d seed=%llu brace_mode=%d course_press=%d (lane A211: the playtester's decision through the button resolver; capture only)"),
			KindName(G.Kind), *Level.ToString(), G.bHand ? 1 : 0, CVarJamScriptHumanButtons.GetValueOnGameThread() != 0 ? 1 : 0, G.Tester.bSprint ? 1 : 0,
			static_cast<unsigned long long>(Seed), G.Tester.BraceMode, G.Tester.bCourseCheck ? 1 : 0);
	}

	constexpr int64 TapAgainFrames = 12;   // a finger's second press: 200 ms

	// One two-role key for this step: Tap = pressed and let go before the step (the resolver counts it for the step); Level = held.
	void DriveKey(const FKey& Key, bool bLevel, bool bTap, bool& bDown, int64& Presses)
	{
		if (bLevel)
		{
			if (!bDown) { G.Buttons.Press(Key); bDown = true; ++Presses; }
		}
		else
		{
			if (bDown) { G.Buttons.Release(Key); bDown = false; }
			if (bTap) { G.Buttons.Press(Key); G.Buttons.Release(Key); ++Presses; }
		}
	}
}

FJamInput JamScriptHuman2::Step(const UJamSim& Sim, const FName& Level, uint64 CutSeed)
{
	const int64 Frame = Sim.GetFrame();
	if (G.Sim != &Sim || Frame < G.LastFrame || G.LastFrame < 0) Begin(Sim, Level, CutSeed);
	G.LastFrame = Frame;
	const int32 Me = Sim.GetHumanIndex();
	FJamInput Out;
	if (Me < 0 || Me >= JamAI::kMaxSkaters) return Out;

	// The world the playtester reads: the brain's own view of the sim, with the frames since the drop (FJamSimAdapter::View's read-out; the draw's
	// first frame open is the drop).
	JamAI::FJamAIWorld W;
	JamAI::FJamSimAdapter::BuildWorld(Sim, W);
	if (W.bDrawOpen && !G.bDrawWasOpen) G.LastDrop = Frame;
	G.bDrawWasOpen = W.bDrawOpen;
	W.FramesSinceDrop = (W.bDrawOpen && G.LastDrop >= 0) ? static_cast<int>(Frame - G.LastDrop) : -1;

	JamAI::FJamAIInput In = G.Tester.Think(W);
	if (G.bHand) G.Hand.Apply(Sim, Me, In);

	// The plain verbs have keys of their own: straight through.
	Out.Move = FVector2D(In.Move.X, In.Move.Y);
	Out.Stick = FVector2D(In.Stick.X, In.Stick.Y);
	Out.bBrake = In.bBrake;
	Out.bPoke = In.bPoke;
	Out.bDeke = In.bDeke;
	Out.bSpin = In.bSpin;
	Out.bSprint = In.bSprint;

	if (CVarJamScriptHumanButtons.GetValueOnGameThread() == 0)
	{
		// The headless match's path: the playtester's verbs as FJamSimAdapter::Step copies them.
		Out.bShoot = In.bShoot;
		Out.bShootHeld = In.bShootHeld;
		Out.bPass = In.bPass;
		Out.bPassHeld = In.bPassHeld;
		Out.bCheck = In.bCheck;
		Out.bLift = In.bLift;
		Out.bCrossHold = In.bCrossHold;
	}
	else
	{
		// The two-role buttons, as a hand holds them: LMB down while he charges a shot or holds the cross-check, a tap for a flat shot or a
		// cross-check; RMB down while he holds a pass (the saucer), a tap for a pass or a body check. What each press becomes is the resolver's
		// answer for the possession of this frame, exactly as for the keys (JamControls::ApplyButtons).
		// The playtester may ask for the same tap on every frame while its reason holds (the shot after a deke): a finger presses once, and again
		// after TapAgainFrames if the reason is still there.
		const bool bLmbWant = In.bShoot || In.bLift, bRmbWant = In.bPass || In.bCheck;
		const bool bLmbTap = bLmbWant && (!G.bLmbTapWas || Frame - G.LmbTapAt >= TapAgainFrames);
		const bool bRmbTap = bRmbWant && (!G.bRmbTapWas || Frame - G.RmbTapAt >= TapAgainFrames);
		G.bLmbTapWas = bLmbWant; G.bRmbTapWas = bRmbWant;
		if (bLmbTap) G.LmbTapAt = Frame;
		if (bRmbTap) G.RmbTapAt = Frame;
		DriveKey(EKeys::LeftMouseButton, In.bShootHeld || In.bCrossHold, bLmbTap, G.bLmbDown, G.LmbPresses);
		DriveKey(EKeys::RightMouseButton, In.bPassHeld, bRmbTap, G.bRmbDown, G.RmbPresses);
		const FJamButtonVerbs V = G.Buttons.Step(JamControls::ButtonContext(Sim, Me), G.Bindings);
		Out.bShootHeld = V.bShootHeld;
		Out.bPassHeld = V.bPassHeld;
		Out.bCheck = V.bCheck;
		Out.bLift = V.bLift;
		JamControls::SetCrossHold(Out, V.bCrossHeld && Sim.CarrierIndex() != Me);
		if (V.bShootPress) ++G.Shots;
		if (V.bPassPress) ++G.Passes;
		if (V.bCheck) ++G.Checks;
		if (V.bLift) ++G.Lifts;
		if (V.bCrossHeld) ++G.HoldFrames;
		if (V.DeadNow != EJamVerb::Count) ++G.Dead;
	}

	++G.Frames;
	if (G.Frames % 600 == 0)
	{
		UE_LOG(LogJamScript, Log, TEXT("JAMSCRIPT frame=%lld kind=%s lmb=%lld rmb=%lld shoot=%lld pass=%lld check=%lld cross=%lld hold_frames=%lld dead=%lld act=%d carry=%d charge=%d chase=%d"),
			static_cast<long long>(Frame), KindName(G.Kind), static_cast<long long>(G.LmbPresses), static_cast<long long>(G.RmbPresses),
			static_cast<long long>(G.Shots), static_cast<long long>(G.Passes), static_cast<long long>(G.Checks), static_cast<long long>(G.Lifts),
			static_cast<long long>(G.HoldFrames), static_cast<long long>(G.Dead), static_cast<int32>(G.Tester.LastAct()),
			G.Tester.ActCount(JamAI::EPlaytesterAct::Carry), G.Tester.ActCount(JamAI::EPlaytesterAct::Charge), G.Tester.ActCount(JamAI::EPlaytesterAct::Chase));
	}
	return Out;
}

// The game mode's entry (AHokejGameMode::ScriptedHumanInput declares it at block scope, so its edit stays inside that one function).
FJamInput JamScriptHuman2Step(const UJamSim& Sim, const FName& Level, uint64 CutSeed) { return JamScriptHuman2::Step(Sim, Level, CutSeed); }
