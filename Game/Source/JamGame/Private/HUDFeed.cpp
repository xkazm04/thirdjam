#include "HUDFeed.h"
#include "Engine/World.h"
#include "JamSim.h"
#include "JamCartridge.h"

DEFINE_LOG_CATEGORY(LogJamHud);

FVector FJamPresentationFrame::GetFarNetWorldCm() const
{
	const float X = AttackSign * (RinkLengthM * 0.5f - NetInsetM);
	return FVector(X * 100.f, 0.f, 300.f);
}

UJamPresentation* UJamPresentation::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	return World ? World->GetSubsystem<UJamPresentation>() : nullptr;
}

void UJamPresentation::PushFrame(const FJamPresentationFrame& Frame)
{
	Latest = Frame;
	bHasFrame = true;
	OnSimFrame.Broadcast(Latest);
}

void UJamPresentation::BuildFrameFromSim(const UJamSim& Sim, FJamPresentationFrame& Out, int32 LocalSeat)
{
	const FJamGameConfig& C = Sim.GetConfig();
	Out.SimFrame = Sim.GetFrame();
	Out.RinkLengthM = (float)C.Rink.LengthM;
	Out.RinkWidthM = (float)C.Rink.WidthM;
	// Lane A129 (track Q1): the local seat's (the guest's on the guest's machine), was always the sim's human (the host's).
	const bool bSeat = LocalSeat >= 0 && Sim.GetSkaters().IsValidIndex(LocalSeat);
	Out.HumanIndex = bSeat ? LocalSeat : Sim.GetHumanIndex();
	const int32 HumanTeam = bSeat ? Sim.GetSkaters()[LocalSeat].Team : Sim.GetHumanTeam();
	Out.AttackSign = Sim.AttackGoalX(HumanTeam) >= 0.0 ? 1 : -1;
	Out.NetInsetM = (float)(C.Rink.LengthM * 0.5 - FMath::Abs(Sim.AttackGoalX(HumanTeam)));
	Out.Skaters.Reset();
	for (const FJamSkaterState& S : Sim.GetSkaters())
	{
		FJamPresentationSkater P;
		P.PosM = S.Pos;
		P.YawDeg = (float)FMath::RadiansToDegrees(S.Yaw);
		P.TeamId = S.Team;
		P.bHuman = S.bHuman;
		Out.Skaters.Add(P);
	}
	Out.PuckM = Sim.GetPuck().Pos;
	Out.ScoreHome = Sim.GetScore(0);
	Out.ScoreAway = Sim.GetScore(1);
	Out.Take = 1;
	Out.ScoreToWin = C.Rules.Mode == TEXT("first_to") ? C.Rules.ScoreToWin : 0;
	Out.bHasTimer = C.Rules.Mode == TEXT("timer");
	Out.FramesRemaining = Sim.GetTimerFrames();
	if (C.Teams.Num() > 0) { Out.TeamColour0 = C.Teams[0].Colour; }
	if (C.Teams.Num() > 1) { Out.TeamColour1 = C.Teams[1].Colour; }
	switch (Sim.GetPhase())
	{
	case EJamSimPhase::GoalPause: Out.Phase = EJamPhase::GoalPause; break;
	case EJamSimPhase::Over: Out.Phase = Sim.GetWinner() == HumanTeam ? EJamPhase::Win : EJamPhase::Lose; break;
	default: Out.Phase = EJamPhase::Play; break;
	}
}

void UJamPresentation::PushFromSim(const UJamSim& Sim, int32 LocalSeat)
{
	FJamPresentationFrame F;
	BuildFrameFromSim(Sim, F, LocalSeat);
	F.bDirectorsCallReady = Latest.bDirectorsCallReady;  // set by the Director's Call system when it exists
	PushFrame(F);
}
