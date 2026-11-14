// Lane A144 (track U2, evolution 2): jam.ctrl.EndMatch <team>. Ends the running match for a side, so the end card can be looked at and photographed
// without playing a match to its last goal (tools/qa/screen/u2-stills.ps1 -EndCard).
//
// It scores real goals: whenever the sim is in play, the puck is placed 0.4 m inside the net that team scores in (UJamSim::SetPuck, the staging
// hook the drills use), and the sim's own step does the rest on its next frame: InGoal -> ScoreGoal -> the goal event, the goal pause and faceoff,
// and on the goal that reaches the rule's score, Phase Over, Winner, the win / lose event, the season's result and the card. Nothing of the end is
// written from here; a match first to 3 from 0:0 takes three such goals and their pauses (7 s in the proof run). A timer match ends at its own horn: the
// hook keeps the named side ahead until then.
//
// Refused in a joined match (the two machines' sims must stay the same), and without a match. A proof tool: no rule reads it, no state is added.
#include "CoreMinimal.h"
#include "HokejGameMode.h"
#include "JamSim.h"
#include "Containers/Ticker.h"
#include "HAL/IConsoleManager.h"
#include "Engine/World.h"

namespace
{
	TWeakObjectPtr<UWorld> GEndWorld;
	int32 GEndTeam = -1;
	int32 GEndPlaced = 0;
	FTSTicker::FDelegateHandle GEndHandle;

	void StopEnd()
	{
		GEndTeam = -1;
		GEndWorld.Reset();
		GEndHandle.Reset();
	}

	bool TickEnd(float)
	{
		UWorld* W = GEndWorld.Get();
		AHokejGameMode* GM = W ? W->GetAuthGameMode<AHokejGameMode>() : nullptr;
		UJamSim* Sim = GM ? GM->GetSim() : nullptr;
		if (!Sim || GEndTeam < 0)
		{
			UE_LOG(LogHokej, Log, TEXT("JAMCTRL endmatch stopped: the match is gone"));
			StopEnd();
			return false;
		}
		if (Sim->GetPhase() == EJamSimPhase::Over)
		{
			UE_LOG(LogHokej, Log, TEXT("JAMCTRL endmatch done winner=%d human_team=%d result=%s score=%d:%d frame=%lld pucks_placed=%d"),
				Sim->GetWinner(), Sim->GetHumanTeam(), Sim->GetWinner() == Sim->GetHumanTeam() ? TEXT("win") : TEXT("loss"),
				Sim->GetScore(0), Sim->GetScore(1), Sim->GetFrame(), GEndPlaced);
			StopEnd();
			return false;
		}
		if (Sim->GetPhase() != EJamSimPhase::Playing) return true;   // the goal pause and the faceoff run their own course
		// A first-to match: a goal each time the sim is in play. A timer match: only while the side is not ahead (the horn ends it).
		// Team 0 shoots at +X until the nets swap.
		const bool bTimer = Sim->GetConfig().Rules.Mode == FName(TEXT("timer"));
		if (!bTimer || Sim->GetScore(GEndTeam) <= Sim->GetScore(1 - GEndTeam))
		{
			const double Side = ((GEndTeam == 0) != Sim->AreNetsSwapped()) ? 1.0 : -1.0;
			Sim->SetPuck(FVector2D(Side * (Sim->GoalLineX() + 0.4), 0.0), FVector2D(Side * 2.0, 0.0));
			++GEndPlaced;
		}
		return true;
	}
}

static FAutoConsoleCommandWithWorldAndArgs CmdJamCtrlEndMatch(TEXT("jam.ctrl.EndMatch"),
	TEXT("Proof: jam.ctrl.EndMatch <team 0|1> ends the running match for that team (0 is the player's side in a menu-started match) by real goals: ")
	TEXT("the puck is placed in the net whenever the sim is in play and the sim's own goal path ends the match. Refused in a joined match."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		AHokejGameMode* GM = World ? World->GetAuthGameMode<AHokejGameMode>() : nullptr;
		const int32 Team = Args.Num() ? FCString::Atoi(*Args[0]) : -1;
		if (!GM || !GM->GetSim() || (Team != 0 && Team != 1))
		{
			UE_LOG(LogHokej, Warning, TEXT("JAMCTRL endmatch refused: needs a running match and a team 0 or 1"));
			return;
		}
		if (GM->GetClass()->GetName().Contains(TEXT("Lobby")))
		{
			UE_LOG(LogHokej, Warning, TEXT("JAMCTRL endmatch refused: a joined match (both games must stay the same)"));
			return;
		}
		GEndWorld = World;
		GEndTeam = Team;
		GEndPlaced = 0;
		UE_LOG(LogHokej, Log, TEXT("JAMCTRL endmatch team=%d human_team=%d score=%d:%d"), Team, GM->GetSim()->GetHumanTeam(), GM->GetSim()->GetScore(0), GM->GetSim()->GetScore(1));
		if (!GEndHandle.IsValid()) GEndHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateStatic(&TickEnd));
	}));
