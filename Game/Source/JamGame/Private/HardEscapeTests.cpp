// Lane A212 (track S3, the night of 14 to 15 Nov): the escape scan's list, one line an event (HardTests.cpp's Bar.Hard.PuckEscape counts them; this one names
// each). A new file so the suite that lands (HardTests.cpp) is not touched while it is in the landing chain.
//
//   Bar.Hard.EscapeList   the brain alone over the case list of Bar.Hard.InputFuzz (hockey.regular, the large arena, boost, chaos and every mode at rookie, pro and
//                         all_star), JAM_HARD_SEEDS seeds a case (default 10) from JAM_HARD_FIRST (default 1), full length. Lines:
//     JAMHARD body case=<case> frame=F skater=S team=T carrier=C rel=<mate|opp|loose> deke=0|1 last=L d=<m> v=<m/s> z=<m> pass=<frames> settle=N stumble=N role=R
//                         the puck within SkaterRadius - 0.05 of a standing skater who does not carry it, for 3 frames running (the frame of the third)
//     JAMHARD owngoal case=<case> frame=F scorer_team=T last=L last_team=T2 carried=0|1 pass=0|1 v=<m/s> at=(x,y)
//                         a goal whose last touch (the frame before) was a skater of the team that conceded: carried = he was its carrier that frame;
//                         pass = a pass of his was in flight
//     JAMHARD escapelist matches=N goals=G own_goals=O own_carried=OC own_pass=OP body=B body_deke=BD
// It reports; it fails nothing (the counts are the finding).
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "JamSim.h"
#include "JamCartridge.h"
#include "JamBrain.h"
#include "JamSimAdapter.h"
#include "DifficultyResolve.h"

namespace JamHardList
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::CommandletContext | EAutomationTestFlags::EngineFilter;

	int32 EnvInt(const TCHAR* Name, int32 Default) { const FString V = FPlatformMisc::GetEnvironmentVariable(Name); return V.IsEmpty() ? Default : FCString::Atoi(*V); }

	struct FTotals { int32 Matches = 0, Goals = 0, Own = 0, OwnCarried = 0, OwnPass = 0, Body = 0, BodyDeke = 0; };

	void RunOne(const UJamCartridge& Cart, const FString& File, const TCHAR* Level, uint64 Seed, int64 MaxFrames, FTotals& T)
	{
		const FString Case = FString::Printf(TEXT("%s@%s#%llu"), *File, Level, static_cast<unsigned long long>(Seed));
		const JamAI::FJamMatchConfig Cfg = JamAI::ToMatchConfigResolved(Cart, Level);
		JamAI::FJamSimAdapter Sim(Cart);
		Sim.Reset(Cfg, Seed);
		UJamBrain* Brain = NewObject<UJamBrain>();
		Brain->ApplyCartridge(Cart, Level);
		Brain->ResetBrain();
		JamAI::FJamAIInput Out[JamAI::kMaxSkaters];
		const UJamSim* S = Sim.GetSim();
		int32 PrevScore[2] = { 0, 0 };
		int32 PrevGoals = 0, PrevLast = -1, PrevCarrier = -1, PrevPass = 0, BodyRun = 0, BodySkater = -1;
		FVector2D PrevPos = FVector2D::ZeroVector;
		double PrevV = 0.0;
		for (int64 F = 0; F < MaxFrames && !Sim.IsOver(); ++F)
		{
			JamAI::FJamAIWorld W;
			Sim.View(W);
			W.PlayerIndex = -1;
			for (JamAI::FJamAIInput& O : Out) O = JamAI::FJamAIInput();
			Brain->Think(W, Out);
			Sim.Step(Out);
			const FJamPuckState& P = S->GetPuck();
			const int32 Goals = S->GetScore(0) + S->GetScore(1);
			if (Goals > PrevGoals)
			{
				++T.Goals;
				// the scorer: the team whose score grew this frame (one goal a frame)
				const int32 Scorer = S->GetScore(0) > PrevScore[0] ? 0 : 1;
				const int32 Conceding = 1 - Scorer;
				const int32 LastTeam = S->GetSkaters().IsValidIndex(PrevLast) ? S->GetSkaters()[PrevLast].Team : -1;
				if (LastTeam == Conceding)
				{
					++T.Own;
					const bool bCarried = PrevCarrier == PrevLast, bPass = PrevPass > 0;
					T.OwnCarried += bCarried ? 1 : 0; T.OwnPass += bPass ? 1 : 0;
					UE_LOG(LogTemp, Display, TEXT("JAMHARD owngoal case=%s frame=%lld scorer_team=%d last=%d last_team=%d carried=%d pass=%d v=%.1f at=(%.2f,%.2f)"), *Case, static_cast<long long>(S->GetFrame()),
						Scorer, PrevLast, LastTeam, bCarried ? 1 : 0, bPass ? 1 : 0, PrevV, PrevPos.X, PrevPos.Y);
				}
			}
			PrevGoals = Goals; PrevScore[0] = S->GetScore(0); PrevScore[1] = S->GetScore(1);
			// body
			int32 Inside = -1;
			const int32 Car = S->CarrierIndex();
			if (S->GetPhase() == EJamSimPhase::Playing && P.Z < UJamSim::BodyHeightM)
				for (const FJamSkaterState& K : S->GetSkaters())
					if (K.Index != Car && !K.IsDown() && FVector2D::Distance(K.Pos, P.Pos) < UJamSim::SkaterRadius - 0.05) { Inside = K.Index; break; }
			BodyRun = (Inside >= 0 && Inside == BodySkater) ? BodyRun + 1 : (Inside >= 0 ? 1 : 0);
			BodySkater = Inside;
			if (BodyRun == 3)
			{
				const FJamSkaterState& K = S->GetSkaters()[Inside];
				const bool bDeke = S->GetSkaters().IsValidIndex(Car) && S->GetSkaters()[Car].DekeFrames > 0;
				const TCHAR* Rel = !S->GetSkaters().IsValidIndex(Car) ? TEXT("loose") : (S->GetSkaters()[Car].Team == K.Team ? TEXT("mate") : TEXT("opp"));
				++T.Body; T.BodyDeke += bDeke ? 1 : 0;
				UE_LOG(LogTemp, Display, TEXT("JAMHARD body case=%s frame=%lld skater=%d team=%d carrier=%d rel=%s deke=%d last=%d d=%.2f v=%.1f z=%.2f pass=%d settle=%d stumble=%d role=%s"), *Case,
					static_cast<long long>(S->GetFrame()), Inside, K.Team, Car, Rel, bDeke ? 1 : 0, P.LastTouch, FVector2D::Distance(K.Pos, P.Pos), P.Vel.Size(), P.Z, P.PassFrames, K.SettleFrames, K.StumbleFrames, *K.Role.ToString());
			}
			PrevLast = P.LastTouch; PrevCarrier = Car; PrevPass = P.PassFrames; PrevPos = P.Pos; PrevV = P.Vel.Size();
		}
		++T.Matches;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHardEscapeList, "Bar.Hard.EscapeList", JamHardList::Flags)
bool FJamHardEscapeList::RunTest(const FString& Parameters)
{
	using namespace JamHardList;
	const FString Dir = UJamCartridge::GetCartridgeDir();
	TArray<FString> Files = { TEXT("hockey.regular.cartridge.json"), TEXT("hockey.arena.large.cartridge.json"), TEXT("hockey.boost.cartridge.json"), TEXT("hockey.chaos.cartridge.json") };
	TArray<FString> Modes;
	IFileManager::Get().FindFiles(Modes, *FPaths::Combine(Dir, TEXT("modes"), TEXT("*.cartridge.json")), true, false);
	Modes.Sort();
	for (const FString& M : Modes) Files.Add(FString(TEXT("modes/")) + M);
	const int32 Seeds = EnvInt(TEXT("JAM_HARD_SEEDS"), 10), First = EnvInt(TEXT("JAM_HARD_FIRST"), 1);
	const int64 MaxFrames = EnvInt(TEXT("JAM_HARD_FRAMES"), 36000);
	FTotals T;
	for (const FString& File : Files)
	{
		TArray<FString> Errors;
		UJamCartridge* Cart = UJamCartridge::LoadFromFile(FPaths::Combine(Dir, File), Errors);
		if (!Cart) { AddWarning(FString::Printf(TEXT("%s did not load: %s"), *File, Errors.Num() ? *Errors[0] : TEXT("?"))); continue; }
		for (const TCHAR* L : { TEXT("rookie"), TEXT("pro"), TEXT("all_star") })
			for (int32 I = 0; I < Seeds; ++I) RunOne(*Cart, File, L, static_cast<uint64>(First + I), MaxFrames, T);
	}
	UE_LOG(LogTemp, Display, TEXT("JAMHARD escapelist matches=%d goals=%d own_goals=%d own_carried=%d own_pass=%d body=%d body_deke=%d"), T.Matches, T.Goals, T.Own, T.OwnCarried, T.OwnPass, T.Body, T.BodyDeke);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
