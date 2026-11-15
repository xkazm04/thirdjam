// Lane A217 (track C0, the curve): see JamHandCellsCommandlet.h. MEASUREMENT ONLY.
#include "JamHandCellsCommandlet.h"

#include "ScriptHuman2.h"
#include "JamBrain.h"
#include "JamCartridge.h"
#include "JamSimAdapter.h"
#include "JamRuleSystem.h"
#include "JamZones.h"
#include "JamEffectSystem.h"
#include "DifficultyProfile.h"
#include "JamEventBus.h"
#include "HAL/IConsoleManager.h"
#include "Misc/Parse.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogJamHand, Log, All);

namespace JamHandCells
{
	static uint64 GLastHash = 0;
	uint64 LastHash() { return GLastHash; }
}

namespace JamHandCellsLocal
{
	void SetCVar(const TCHAR* Name, int32 Value)
	{
		if (IConsoleVariable* V = IConsoleManager::Get().FindConsoleVariable(Name)) V->Set(Value, ECVF_SetByCode);
		else UE_LOG(LogJamHand, Warning, TEXT("JAMHAND no console variable %s"), Name);
	}

	// One match's read-outs (frames, never seconds; -1 = never happened).
	struct FHandMatch
	{
		uint64 SimSeed = 0, HandSeed = 0, Hash = 0;
		int64 Frames = 0, PlayFrames = 0, DownFrames = 0, StumbleFrames = 0, BoxFrames = 0, CarryFrames = 0;
		int32 GoalsFor = 0, GoalsAgainst = 0, Winner = -1, HumanTeam = 0;
		bool bOver = false;
		int64 FirstGoalAgainst = -1, FirstGoalFor = -1;
		int32 GaEarly600 = 0;               // goals against inside the first 600 frames
		// goals against by the distance of the last shot of the scoring team to the net it attacked (within 240 frames before the goal): <10, 10-20, 20-28, >=28 m;
		// none = no shot of theirs in that window (a pass, a carry, a deflection, an own goal)
		int32 GaByDist[5] = {};
		int32 GaOwn = 0;                    // goals against whose last toucher was a skater of the human's team
		int32 Shots = 0, Passes = 0, PassesMate = 0, PassesOpp = 0, PassesLoose = 0, GoalsByHuman = 0;
		int32 MatePasses = 0, MatePassesToHuman = 0;
		int32 Knockdowns = 0;
	};

	const TCHAR* DistName(int32 B) { static const TCHAR* N[5] = { TEXT("lt10"), TEXT("10_20"), TEXT("20_28"), TEXT("ge28"), TEXT("none") }; return N[B]; }
}

UJamHandCellsCommandlet::UJamHandCellsCommandlet()
{
	IsClient = false;
	IsServer = false;
	LogToConsole = true;
	ShowErrorCount = true;
}

int32 UJamHandCellsCommandlet::Main(const FString& Params)
{
	using namespace JamHandCellsLocal;
	FString CartName = TEXT("hockey.regular"), Seeds = TEXT("1-1"), DifficultyArg, TuneArg;
	int64 MaxFrames = 36000, SimSeedFixed = 0, HandSeedFixed = 0;
	int32 Buttons = 1, Hand = 1, Sprint = 1, Kind = -1;
	FParse::Value(*Params, TEXT("-cartridge="), CartName, false);
	FParse::Value(*Params, TEXT("-seeds="), Seeds, false);
	FParse::Value(*Params, TEXT("-difficulty="), DifficultyArg, false);
	FParse::Value(*Params, TEXT("-aitune="), TuneArg, false);   // the tournament's tune text (DifficultyProfile.h), after the level's resolution
	FParse::Value(*Params, TEXT("-maxframes="), MaxFrames);
	FParse::Value(*Params, TEXT("-simseed="), SimSeedFixed);
	FParse::Value(*Params, TEXT("-handseed="), HandSeedFixed);
	FParse::Value(*Params, TEXT("-buttons="), Buttons);
	FParse::Value(*Params, TEXT("-hand="), Hand);
	FParse::Value(*Params, TEXT("-sprint="), Sprint);
	FParse::Value(*Params, TEXT("-kind="), Kind);
	const bool bHashes = FParse::Param(*Params, TEXT("hashes"));
	const bool bPassLog = FParse::Param(*Params, TEXT("passlog"));   // one JAMPASSFATE line per pass of the human (PASS FATES, Director 05:30)

	TArray<FString> Errors;
	UJamCartridge* Cart = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(CartName), Errors);
	if (!Cart)
	{
		UE_LOG(LogJamHand, Error, TEXT("FAIL hand cartridge %s does not load: %s"), *CartName, *FString::Join(Errors, TEXT("; ")));
		return 2;
	}
	Cart->AddToRoot();
	FString Lo, Hi;
	if (!Seeds.Split(TEXT("-"), &Lo, &Hi)) { Lo = Hi = Seeds; }
	const int32 SeedLo = FCString::Atoi(*Lo), SeedHi = FCString::Atoi(*Hi);

	// jam.ScriptHuman 2's own switches, as the critic sets them (the seed is passed as the cut's seed: jam.ScriptHumanSeed stays 0).
	SetCVar(TEXT("jam.ScriptHumanButtons"), Buttons);
	SetCVar(TEXT("jam.ScriptHumanHand"), Hand);
	SetCVar(TEXT("jam.ScriptHumanSprint"), Sprint);
	SetCVar(TEXT("jam.ScriptHumanKind"), Kind);
	SetCVar(TEXT("jam.ScriptHumanSeed"), 0);

	const FJamGameConfig& G = Cart->Game;
	const FTCHARToUTF8 CartUtf8(*CartName);
	for (int32 Seed = SeedLo; Seed <= SeedHi; ++Seed)
	{
		FHandMatch M;
		M.SimSeed = SimSeedFixed > 0 ? static_cast<uint64>(SimSeedFixed) : static_cast<uint64>(Seed);
		M.HandSeed = HandSeedFixed > 0 ? static_cast<uint64>(HandSeedFixed) : static_cast<uint64>(Seed);

		// AHokejGameMode::BuildMatch, without the actors: the rule hook and the zone source before Init, the effect system after it, the brain by the level.
		TStrongObjectPtr<UJamSim> Sim(NewObject<UJamSim>());
		TStrongObjectPtr<UJamRuleSystem> Rules(NewObject<UJamRuleSystem>());
		Rules->Init(G);
		Sim->SetRuleHook(Rules.Get());
		TStrongObjectPtr<UJamZoneSystem> Zones(NewObject<UJamZoneSystem>());
		Zones->Init(G);
		Sim->SetZoneSource(Zones.Get());
		Sim->Init(G, M.SimSeed);
		TArray<FJamInput> Inputs;
		Inputs.SetNum(Sim->GetSkaters().Num());
		TStrongObjectPtr<UJamEffectSystem> Effects(NewObject<UJamEffectSystem>());
		{
			TArray<FString> EffectErrors;
			Effects->Init(Sim.Get(), Cart->RawJson, EffectErrors);
		}
		TStrongObjectPtr<UJamBrain> Brain(NewObject<UJamBrain>());
		Brain->ApplyCartridge(*Cart, DifficultyArg.IsEmpty() ? nullptr : *DifficultyArg);
		if (!TuneArg.IsEmpty())
		{
			// -aitune: as the tournament applies it (after the resolution); the five numbers UJamBrain keeps as floats follow the tuned params (ResetBrain re-applies them).
			char Err[512] = {};
			JamAI::FJamAIParams& P = Brain->EditParams();
			if (!JamAI::ApplyDifficultyTune(TCHAR_TO_UTF8(*TuneArg), P, Err, sizeof(Err)))
			{
				UE_LOG(LogJamHand, Error, TEXT("FAIL hand bad -aitune: %s"), UTF8_TO_TCHAR(Err));
				return 2;
			}
			Brain->YieldMarginM = static_cast<float>(P.YieldMarginM); Brain->YieldRadiusM = static_cast<float>(P.YieldRadiusM); Brain->HoldRadiusM = static_cast<float>(P.HoldRadiusM);
			Brain->SpeedScale = static_cast<float>(P.SpeedScale); Brain->RedSkill = static_cast<float>(P.RedSkill);
		}
		Brain->ResetBrain();
		const FName Level = Brain->Difficulty;

		const int32 Human = Sim->GetHumanIndex();
		M.HumanTeam = Sim->GetHumanTeam();
		int64 LastOppShotFrame = -100000;
		double LastOppShotDist = 0.0;
		int32 PassOpen = 0;          // 1 while the human's pass waits for its next toucher
		int64 PassAt = 0;
		int32 MatePassBy = -1;       // a mate's pass in flight (its next toucher)
		int32 PrevLastTouch = Sim->GetPuck().LastTouch;
		bool bWasDown = false;
		// PASS FATES: the human's pass in flight (release frame, intended mate, his distance, the nearest opponent within 2 m of the passer at the release).
		struct FPassFate { int64 At = -1; int32 To = -1; double ToD = -1; int32 Press = -1; double PressD = -1; bool bSaucer = false; float Strength = 0.f; };
		FPassFate PF;
		auto FatePos = [&](int32 i) { return Sim->GetSkaters().IsValidIndex(i) ? Sim->GetSkaters()[i].Pos : FVector2D::ZeroVector; };
		auto EmitFate = [&](const TCHAR* Fate, int32 Taker)
		{
			if (!bPassLog || PF.At < 0) return;
			const FVector2D Pk = Sim->GetPuck().Pos;
			const double PuckD = FVector2D::Distance(Pk, FatePos(Human));
			const double ToPuckD = PF.To >= 0 ? FVector2D::Distance(Pk, FatePos(PF.To)) : -1.0;
			UE_LOG(LogJamHand, Display, TEXT("JAMPASSFATE seed=%d at=%lld fate=%s taker=%d frames=%lld to=%d to_d=%.1f press=%d press_d=%.2f taker_is_press=%d puck_d=%.1f to_puck_d=%.1f saucer=%d strength=%.2f"),
				Seed, PF.At, Fate, Taker, Sim->GetFrame() - PF.At, PF.To, PF.ToD, PF.Press, PF.PressD, (Taker >= 0 && Taker == PF.Press) ? 1 : 0, PuckD, ToPuckD, PF.bSaucer ? 1 : 0, PF.Strength);
		};
		for (int64 F = 0; F < MaxFrames && Sim->GetPhase() != EJamSimPhase::Over; ++F)
		{
			// AHokejGameMode::GatherInputs: the brain's world, the pickups and the zones, the bots' verbs as the game mode copies them, the human's hand.
			JamAI::FJamAIWorld W;
			JamAI::FJamSimAdapter::BuildWorld(*Sim, W);
			W.NumPickups = 0;
			for (const FJamPickupOnIce& P : Effects->GetPickups())
			{
				if (W.NumPickups >= JamAI::FJamAIWorld::kMaxPickups) break;
				W.PickupKind[W.NumPickups] = P.PickupIndex;
				W.PickupPos[W.NumPickups] = JamAI::FVec2(P.Pos.X, P.Pos.Y);
				++W.NumPickups;
			}
			for (const FJamZoneConfig& Z : Zones->GetZones())
			{
				if (Z.Kind != FName(TEXT("slow_ice")) || W.NumSlowIce >= JamAI::FJamAIWorld::kMaxSlowIce) continue;
				const bool bRect = Z.Shape == FName(TEXT("rect"));
				W.SlowIceCentre[W.NumSlowIce] = JamAI::FVec2(Z.X, Z.Z);
				W.SlowIceRadius[W.NumSlowIce] = bRect ? 0.5 * FMath::Sqrt(Z.W * Z.W + Z.D * Z.D) : Z.R;
				++W.NumSlowIce;
			}
			for (int32 i = 0; i < W.NumSkaters; ++i)
			{
				FName ZoneId;
				W.Skaters[i].bNoShoot = Zones->BlocksShot(FVector2D(W.Skaters[i].Pos.X, W.Skaters[i].Pos.Y), ZoneId);
			}
			JamAI::FJamAIInput Out[JamAI::kMaxSkaters];
			Brain->Think(W, Out);
			for (int32 i = 0; i < Inputs.Num(); ++i)
			{
				if (i == Human || i >= JamAI::kMaxSkaters) continue;
				FJamInput& I = Inputs[i];
				I.Move = FVector2D(Out[i].Move.X, Out[i].Move.Y);
				I.bShoot = Out[i].bShoot;
				I.bShootHeld = Out[i].bShootHeld;
				I.bPassHeld = Out[i].bPassHeld;
				I.bCheck = Out[i].bCheck;
				I.bBrake = Out[i].bBrake;
				I.bPass = Out[i].bPass;
				I.Stick = FVector2D(Out[i].Stick.X, Out[i].Stick.Y);
				I.bPoke = Out[i].bPoke;
				I.bLift = Out[i].bLift;
				I.bDeke = Out[i].bDeke;
				I.bSprint = Out[i].bSprint;
			}
			if (Human >= 0 && Human < Inputs.Num()) Inputs[Human] = JamScriptHuman2Step(*Sim, Level, M.HandSeed);

			// AHokejGameMode::StepOnce: the score mirror, the step, the zones, the effects.
			Rules->SetScore(0, Sim->GetScore(0));
			Rules->SetScore(1, Sim->GetScore(1));
			const int32 S0 = Sim->GetScore(0), S1 = Sim->GetScore(1);
			Sim->Step(Inputs);
			Zones->Update(Sim->GetFrame(), Sim->GetSkaters(), Sim->GetPuck());
			Effects->Step();
			++M.Frames;

			// Read-outs (reads only).
			const TArray<FJamSkaterState>& Sk = Sim->GetSkaters();
			const bool bPlay = Sim->GetPhase() == EJamSimPhase::Playing;
			if (bPlay) ++M.PlayFrames;
			if (Sk.IsValidIndex(Human))
			{
				const FJamSkaterState& H = Sk[Human];
				if (bPlay && H.IsDown()) ++M.DownFrames;
				if (bPlay && H.StumbleFrames > 0) ++M.StumbleFrames;
				if (bPlay && H.InBox()) ++M.BoxFrames;
				if (bPlay && Sim->CarrierIndex() == Human) ++M.CarryFrames;
				if (H.IsDown() && !bWasDown) ++M.Knockdowns;
				bWasDown = H.IsDown();
				if (H.bShotThisFrame) ++M.Shots;
				if (H.bPassThisFrame)
				{
					if (PassOpen) EmitFate(TEXT("repass"), Human);
					++M.Passes; PassOpen = 1; PassAt = Sim->GetFrame();
					PF = FPassFate(); PF.At = Sim->GetFrame();
					for (const FJamSimEvent& E : Sim->GetFrameEvents())
					{
						if (E.Name != JamEvents::Pass || E.Who != Human) continue;
						FString K = E.Kind.ToString();
						PF.bSaucer = K.StartsWith(TEXT("saucer_"));
						int32 Ix = INDEX_NONE;
						if (K.FindLastChar(TEXT('_'), Ix)) PF.To = FCString::Atoi(*K.Mid(Ix + 1));
						PF.Strength = E.Strength;
					}
					if (PF.To >= 0 && Sk.IsValidIndex(PF.To)) PF.ToD = FVector2D::Distance(Sk[PF.To].Pos, H.Pos); else PF.To = -1;
					for (const FJamSkaterState& O : Sk)
					{
						if (O.Team == M.HumanTeam || O.IsDown() || O.InBox()) continue;
						const double D = FVector2D::Distance(O.Pos, H.Pos);
						if (D <= 2.0 && (PF.Press < 0 || D < PF.PressD)) { PF.Press = O.Index; PF.PressD = D; }
					}
				}
			}
			for (const FJamSkaterState& K : Sk)
			{
				if (K.bShotThisFrame && K.Team != M.HumanTeam)
				{
					LastOppShotFrame = Sim->GetFrame();
					LastOppShotDist = FVector2D::Distance(K.Pos, FVector2D(Sim->AttackGoalX(K.Team), 0.0));
				}
				if (K.bPassThisFrame && K.Team == M.HumanTeam && K.Index != Human) { ++M.MatePasses; MatePassBy = K.Index; }
			}
			const int32 Lt = Sim->GetPuck().LastTouch;
			if (Lt != PrevLastTouch && Lt >= 0)
			{
				if (PassOpen && Lt != Human) { PassOpen = 0; if (Sk[Lt].Team == M.HumanTeam) { ++M.PassesMate; EmitFate(TEXT("mate"), Lt); } else { ++M.PassesOpp; EmitFate(TEXT("opp"), Lt); } PF.At = -1; }
				if (MatePassBy >= 0 && Lt != MatePassBy) { if (Lt == Human) ++M.MatePassesToHuman; MatePassBy = -1; }
			}
			if (PassOpen && Sim->GetFrame() - PassAt > 180) { PassOpen = 0; ++M.PassesLoose; EmitFate(TEXT("loose"), -1); PF.At = -1; }
			PrevLastTouch = Lt;
			const int32 Ga = M.HumanTeam == 0 ? Sim->GetScore(1) - S1 : Sim->GetScore(0) - S0;
			const int32 Gf = M.HumanTeam == 0 ? Sim->GetScore(0) - S0 : Sim->GetScore(1) - S1;
			if (Gf > 0)
			{
				M.GoalsFor += Gf;
				if (M.FirstGoalFor < 0) M.FirstGoalFor = Sim->GetFrame();
				if (Lt == Human) ++M.GoalsByHuman;
			}
			if (Ga > 0)
			{
				M.GoalsAgainst += Ga;
				if (M.FirstGoalAgainst < 0) M.FirstGoalAgainst = Sim->GetFrame();
				if (Sim->GetFrame() <= 600) M.GaEarly600 += Ga;
				if (Sk.IsValidIndex(Lt) && Sk[Lt].Team == M.HumanTeam) ++M.GaOwn;
				int32 B = 4;
				if (Sim->GetFrame() - LastOppShotFrame <= 240) B = LastOppShotDist < 10.0 ? 0 : LastOppShotDist < 20.0 ? 1 : LastOppShotDist < 28.0 ? 2 : 3;
				M.GaByDist[B] += Ga;
				LastOppShotFrame = -100000;
			}
			if (bHashes && Sim->GetFrame() % 600 == 0)
			{
				UE_LOG(LogJamHand, Display, TEXT("JAMHAND seed=%d frame=%lld hash=%s score=%d:%d phase=%d"), Seed, Sim->GetFrame(), *Sim->StateHashHex(),
					Sim->GetScore(0), Sim->GetScore(1), static_cast<int32>(Sim->GetPhase()));
			}
		}
		M.bOver = Sim->GetPhase() == EJamSimPhase::Over;
		M.Winner = M.bOver ? Sim->GetWinner() : -1;
		M.Hash = Sim->StateHash();
		JamHandCells::GLastHash = M.Hash;
		const TCHAR* Result = !M.bOver ? TEXT("open") : (M.Winner == M.HumanTeam ? TEXT("win") : TEXT("loss"));
		FString GaDist;
		for (int32 B = 0; B < 5; ++B) GaDist += FString::Printf(TEXT("%s\"%s\":%d"), B ? TEXT(",") : TEXT(""), DistName(B), M.GaByDist[B]);
		UE_LOG(LogJamHand, Display, TEXT("{\"tool\":\"thirdjam.hand/1\",\"cartridge\":\"%s\",\"difficulty\":\"%s\",\"tune\":\"%s\",\"seed\":%d,\"sim_seed\":%llu,\"hand_seed\":%llu,\"buttons\":%d,\"hand\":%d,\"sprint\":%d,\"kind\":%d,")
			TEXT("\"result\":\"%s\",\"goals\":[%d,%d],\"frames\":%lld,\"play_frames\":%lld,\"hash\":\"%016llx\",\"first_ga\":%lld,\"first_gf\":%lld,\"ga_600\":%d,\"ga_dist\":{%s},\"ga_own\":%d,")
			TEXT("\"down_frames\":%lld,\"stumble_frames\":%lld,\"box_frames\":%lld,\"carry_frames\":%lld,\"knockdowns\":%d,\"shots\":%d,\"goals_by_human\":%d,")
			TEXT("\"passes\":%d,\"passes_mate\":%d,\"passes_opp\":%d,\"passes_loose\":%d,\"mate_passes\":%d,\"mate_passes_to_human\":%d}"),
			UTF8_TO_TCHAR(CartUtf8.Get()), *Level.ToString(), *TuneArg, Seed, static_cast<unsigned long long>(M.SimSeed), static_cast<unsigned long long>(M.HandSeed), Buttons, Hand, Sprint, Kind,
			Result, M.GoalsFor, M.GoalsAgainst, M.Frames, M.PlayFrames, static_cast<unsigned long long>(M.Hash), M.FirstGoalAgainst, M.FirstGoalFor, M.GaEarly600, *GaDist, M.GaOwn,
			M.DownFrames, M.StumbleFrames, M.BoxFrames, M.CarryFrames, M.Knockdowns, M.Shots, M.GoalsByHuman,
			M.Passes, M.PassesMate, M.PassesOpp, M.PassesLoose, M.MatePasses, M.MatePassesToHuman);

		// The sim keeps raw pointers to its hook, zone source and modifier source: cleared before the systems go (as FJamSimAdapter's destructor does).
		Sim->SetRuleHook(nullptr);
		Sim->SetZoneSource(nullptr);
		Sim->SetModifierSource(nullptr);
		Brain.Reset(); Effects.Reset(); Zones.Reset(); Rules.Reset(); Sim.Reset();
		if ((Seed - SeedLo) % 25 == 24) CollectGarbage(RF_NoFlags);
	}
	Cart->RemoveFromRoot();
	return 0;
}
