#include "LobbyGameMode.h"
#include "JamLobbySubsystem.h"
#include "JamLockstepDriver.h"
#include "JamNetPackets.h"
#include "LobbyWidget.h"
#include "JamCartridge.h"
#include "JamEventBus.h"
#include "JamPawns.h"
#include "JamBrain.h"
#include "JamSimAdapter.h"
#include "HUDFeed.h"
#include "InputMapping.h"
#include "CameraFollow.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "HAL/IConsoleManager.h"
#include "UnrealClient.h"

static TAutoConsoleVariable<int32> CVarJamNetAutoShot(TEXT("jam.net.AutoShot"), 0,
	TEXT("1: a lockstep match saves a screenshot at frame 600 (the two-process proof)."));

namespace
{
	constexpr int32 MaxStepsPerTick = 4;
}

ALobbyGameMode::ALobbyGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALobbyGameMode::StartPlay()
{
	Super::StartPlay();
	UGameInstance* GI = GetGameInstance();
	Lobby = GI ? GI->GetSubsystem<UJamLobbySubsystem>() : nullptr;
	UJamSim* TheSim = GetSim();
	if (!Lobby || !Lobby->IsLockstepMatch() || !Lobby->GetDriver() || !TheSim || !GetCartridge())
	{
		UE_LOG(LogJamNet, Log, TEXT("JAMNET LobbyGameMode without a lobby session: single player"));
		return;
	}
	Driver = Lobby->GetDriver();
	const int32 Human = TheSim->GetHumanIndex();
	const int32 Guest = Lobby->GetGuestIndex();
	if (!TheSim->GetSkaters().IsValidIndex(Guest) || Guest == Human)
	{
		EndMatch(TEXT("NO SEAT"), TEXT("the cut has no skater for a guest"), TEXT("no guest seat"));
		return;
	}
	LocalIndex = Lobby->IsHost() ? Human : Guest;
	RemoteIndex = Lobby->IsHost() ? Guest : Human;
	LockInputs.SetNum(TheSim->GetSkaters().Num());

	const FJamGameConfig& G = GetCartridge()->Game;
	LockBrain = NewObject<UJamBrain>(this);
	LockBrain->YieldMarginM = G.Ai.YieldMarginM;
	LockBrain->YieldRadiusM = G.Ai.YieldRadiusM;
	LockBrain->HoldRadiusM = G.Ai.HoldRadiusM;
	LockBrain->SpeedScale = G.Ai.SpeedScale;
	LockBrain->RedSkill = G.Ai.RedSkill;
	LockBrain->ApplyParams();
	LockBrain->ResetBrain();

	SkaterActors.SetNum(TheSim->GetSkaters().Num());
	for (TActorIterator<AJamSkater> It(GetWorld()); It; ++It) if (SkaterActors.IsValidIndex(It->GetSimIndex())) SkaterActors[It->GetSimIndex()] = *It;
	for (TActorIterator<AJamPuck> It(GetWorld()); It; ++It) PuckActor = *It;

	// R would restart only this machine's sim: unbind the base game mode's restart for the match.
	if (AJamSkater* H = SkaterActor(Human)) if (UJamPlayerInput* Pi = H->FindComponentByClass<UJamPlayerInput>()) Pi->OnRestart.RemoveAll(this);

	bLockstep = true;
	LockAccumulator = 0.0;
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController()) PossessLocal(PC);
	PushLockstepPresentation();
	UE_LOG(LogJamNet, Log, TEXT("JAMNET match start role=%s local=%d remote=%d delay=%d cut=%s seed=%llu hash0=%s"), Lobby->IsHost() ? TEXT("host") : TEXT("guest"),
		LocalIndex, RemoteIndex, Driver->GetDelay(), *Lobby->GetCut(), static_cast<unsigned long long>(TheSim->GetSeed()), *TheSim->StateHashHex());
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (bLockstep) PossessLocal(NewPlayer);
}

AJamSkater* ALobbyGameMode::SkaterActor(int32 SimIndex) const
{
	return SkaterActors.IsValidIndex(SimIndex) ? SkaterActors[SimIndex].Get() : nullptr;
}

void ALobbyGameMode::PossessLocal(APlayerController* PC)
{
	AJamSkater* Mine = SkaterActor(LocalIndex);
	if (!PC || !Mine || !GetCartridge()) return;
	if (!Mine->FindComponentByClass<UJamFollowCamera>())
	{
		UJamFollowCamera* Cam = NewObject<UJamFollowCamera>(Mine, TEXT("FollowCamera"));
		Cam->SetupAttachment(Mine->GetRootComponent());
		Cam->RegisterComponent();
		Mine->AddInstanceComponent(Cam);
		Cam->ApplyConfig(GetCartridge()->Game.Camera);
	}
	if (!Mine->FindComponentByClass<UJamPlayerInput>())
	{
		UJamPlayerInput* In = NewObject<UJamPlayerInput>(Mine, TEXT("PlayerInput"));
		In->RegisterComponent();
		Mine->AddInstanceComponent(In);
	}
	if (PC->GetPawn() != Mine) PC->Possess(Mine);
	PC->SetViewTarget(Mine);
}

FJamInput ALobbyGameMode::ScriptedInput(int32 Index) const
{
	const UJamSim* TheSim = GetSim();
	const FJamSkaterState& S = TheSim->GetSkaters()[Index];
	const FVector2D PuckPos = TheSim->GetPuck().Pos;
	const FVector2D ToGoal = (FVector2D(TheSim->AttackGoalX(S.Team), 0.0) - PuckPos).GetSafeNormal();
	const FVector2D Behind = PuckPos - ToGoal * 0.7;
	FJamInput In;
	const FVector2D D = Behind - S.Pos;
	In.Move = D.Size() > 0.3 ? D.GetSafeNormal() : ToGoal;
	const FVector2D ToPuck = PuckPos - S.Pos;
	const double Dist = ToPuck.Size();
	if (Dist < TheSim->GetConfig().Shot.ReachM && FVector2D::DotProduct(S.Forward(), ToGoal) > 0.97 && Dist > 1e-6 && FVector2D::DotProduct(S.Forward(), ToPuck / Dist) > 0.6) In.bShoot = true;
	return In;
}

FJamInput ALobbyGameMode::ReadLocalInput() const
{
	static IConsoleVariable* ScriptHuman = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.ScriptHuman"));
	if (bScriptedHuman || (ScriptHuman && ScriptHuman->GetInt() != 0)) return ScriptedInput(LocalIndex);
	FJamInput In;
	AJamSkater* Mine = SkaterActor(LocalIndex);
	UJamPlayerInput* Pi = Mine ? Mine->FindComponentByClass<UJamPlayerInput>() : nullptr;
	if (!Pi) return In;
	double Yaw = GetSim()->GetSkaters()[LocalIndex].Yaw;
	if (APlayerController* PC = Cast<APlayerController>(Mine->GetController())) if (PC->PlayerCameraManager) Yaw = FMath::DegreesToRadians(PC->PlayerCameraManager->GetCameraRotation().Yaw);
	const FVector2D Axis = Pi->GetAxis();
	const FVector2D Fwd(FMath::Cos(Yaw), FMath::Sin(Yaw));
	const FVector2D Right(-FMath::Sin(Yaw), FMath::Cos(Yaw));
	In.Move = Fwd * Axis.Y + Right * Axis.X;
	In.bShoot = Pi->ConsumeShootPressed();
	In.bCheck = Pi->ConsumeCheckPressed();
	return In;
}

bool ALobbyGameMode::StepLockstep()
{
	UJamSim* TheSim = GetSim();
	FJamInput L, R;
	if (!Driver->TryStep(ReadLocalInput(), L, R)) return false;
	JamAI::FJamAIWorld W;
	JamAI::FJamSimAdapter::BuildWorld(*TheSim, W);
	JamAI::FJamAIInput Out[JamAI::kMaxSkaters];
	LockBrain->Think(W, Out);
	for (int32 i = 0; i < LockInputs.Num() && i < JamAI::kMaxSkaters; ++i)
	{
		if (i == W.PlayerIndex) continue;
		LockInputs[i].Move = FVector2D(Out[i].Move.X, Out[i].Move.Y);
		LockInputs[i].bShoot = Out[i].bShoot;
		LockInputs[i].bCheck = Out[i].bCheck;
		LockInputs[i].bBrake = Out[i].bBrake;
	}
	LockInputs[LocalIndex] = L;
	LockInputs[RemoteIndex] = R;
	TheSim->Step(LockInputs);
	Driver->RecordHash(TheSim->GetFrame(), TheSim->StateHash());
	DeliverSimEvents();
	for (AJamSkater* A : SkaterActors) if (A) A->ApplySimFrame(TheSim->GetSkaters()[A->GetSimIndex()]);
	PushLockstepPresentation();
	if (TheSim->GetFrame() % 600 == 0)
	{
		UE_LOG(LogJamNet, Log, TEXT("JAMNET frame=%lld hash=%s score=%d:%d phase=%d %s"), TheSim->GetFrame(), *TheSim->StateHashHex(), TheSim->GetScore(0), TheSim->GetScore(1),
			static_cast<int32>(TheSim->GetPhase()), *Driver->DescribeStats());
		if (TheSim->GetFrame() == 600 && !bShotTaken && CVarJamNetAutoShot.GetValueOnGameThread() != 0)
		{
			bShotTaken = true;
			FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("JamNet_%s_f600"), Lobby->IsHost() ? TEXT("host") : TEXT("guest")), true, false);
		}
	}
	return true;
}

void ALobbyGameMode::DeliverSimEvents()
{
	UGameInstance* GI = GetGameInstance();
	UJamEventBus* Bus = GI ? GI->GetSubsystem<UJamEventBus>() : nullptr;
	UJamSim* TheSim = GetSim();
	if (!Bus || !TheSim) return;
	Bus->SetSimFrame(TheSim->GetFrame());
	for (const FJamSimEvent& E : TheSim->GetFrameEvents())
	{
		FJamEventPayload Pl;
		Pl.Who = E.Who;
		Pl.Where = UJamSim::ToWorld(E.Where, E.Name == JamEvents::Board || E.Name == JamEvents::Shot || E.Name == JamEvents::Goal ? 0.04 : 0.0);
		Pl.Strength = E.Strength;
		Pl.Kind = E.Kind;
		Bus->Broadcast(E.Name, Pl);
	}
}

void ALobbyGameMode::PushLockstepPresentation()
{
	UJamPresentation* Pres = UJamPresentation::Get(this);
	UJamSim* TheSim = GetSim();
	if (!Pres || !TheSim || !GetCartridge()) return;
	const FJamGameConfig& G = GetCartridge()->Game;
	const int32 MyTeam = TheSim->GetSkaters().IsValidIndex(LocalIndex) ? TheSim->GetSkaters()[LocalIndex].Team : 0;
	FJamPresentationFrame F;
	F.SimFrame = TheSim->GetFrame();
	F.RinkLengthM = G.Rink.LengthM;
	F.RinkWidthM = G.Rink.WidthM;
	for (const FJamSkaterState& S : TheSim->GetSkaters())
	{
		FJamPresentationSkater Ps;
		Ps.PosM = S.Pos;
		Ps.YawDeg = FMath::RadiansToDegrees(S.Yaw);
		Ps.TeamId = S.Team;
		Ps.bHuman = S.Index == LocalIndex || S.Index == RemoteIndex;
		F.Skaters.Add(Ps);
	}
	F.PuckM = TheSim->GetPuck().Pos;
	F.HumanIndex = LocalIndex;
	F.AttackSign = MyTeam == 0 ? 1 : -1;
	F.NetInsetM = 4.f;
	F.ScoreHome = TheSim->GetScore(MyTeam);
	F.ScoreAway = TheSim->GetScore(1 - MyTeam);
	F.Take = 1;
	F.bHasTimer = G.Rules.Mode == FName(TEXT("timer"));
	F.FramesRemaining = TheSim->GetTimerFrames();
	switch (TheSim->GetPhase())
	{
	case EJamSimPhase::Playing: F.Phase = EJamPhase::Play; break;
	case EJamSimPhase::GoalPause: F.Phase = EJamPhase::GoalPause; break;
	case EJamSimPhase::Over: F.Phase = TheSim->GetWinner() == MyTeam ? EJamPhase::Win : EJamPhase::Lose; break;
	}
	if (G.Teams.Num() == 2) { F.TeamColour0 = G.Teams[0].Colour; F.TeamColour1 = G.Teams[1].Colour; }
	Pres->PushFrame(F);
}

void ALobbyGameMode::EndMatch(const FString& Title, const FString& Subtitle, const FString& Reason)
{
	if (bEnded) return;
	bEnded = true;
	bLockstep = false;
	UE_LOG(LogJamNet, Warning, TEXT("JAMNET match ended: %s (%s)"), *Reason, Driver ? *Driver->DescribeStats() : TEXT(""));
	if (Lobby) Lobby->NotifyMatchEnded(Reason);
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController()) Card = ULobbyCardWidget::Show(PC, Title, Subtitle);
}

void ALobbyGameMode::Tick(float DeltaSeconds)
{
	if (!bLockstep)
	{
		Super::Tick(DeltaSeconds);
		return;
	}
	AGameModeBase::Tick(DeltaSeconds);
	UJamSim* TheSim = GetSim();
	if (!TheSim || !Driver) return;
	Driver->Pump();
	LockAccumulator = FMath::Min(LockAccumulator + DeltaSeconds, UJamSim::DT * MaxStepsPerTick);
	int32 N = 0;
	while (LockAccumulator >= UJamSim::DT && N < MaxStepsPerTick)
	{
		if (!StepLockstep()) break;  // waiting for the remote input: the accumulator keeps the backlog (capped) for a catch-up
		LockAccumulator -= UJamSim::DT;
		++N;
	}
	const float Alpha = static_cast<float>(FMath::Clamp(LockAccumulator / UJamSim::DT, 0.0, 1.0));
	for (AJamSkater* A : SkaterActors) if (A) A->ApplyRender(TheSim->GetSkaters()[A->GetSimIndex()], Alpha);
	if (PuckActor) PuckActor->ApplyRender(TheSim->GetPuck(), Alpha);
	if (UJamPresentation* Pres = UJamPresentation::Get(this)) Pres->SetRenderAlpha(Alpha);

	if (Driver->IsDesynced())
	{
		EndMatch(TEXT("DESYNC"), FString::Printf(TEXT("the two games disagreed at frame %lld; this cut is over"), Driver->GetDesyncFrame()), FString::Printf(TEXT("desync at frame %lld"), Driver->GetDesyncFrame()));
	}
	else if (Driver->IsLost())
	{
		EndMatch(TEXT("CONNECTION LOST"), FString::Printf(TEXT("no word from %s for 3 seconds"), *Lobby->GetRemoteName()), TEXT("connection lost"));
	}
	else if (TheSim->GetPhase() == EJamSimPhase::Over && !bEnded)
	{
		// The normal win/lose card is A5's; the lobby only records the result and keeps the link open for a rematch later.
		if (Lobby) Lobby->NotifyMatchEnded(FString::Printf(TEXT("%d:%d, agreed on %d checkpoints"), TheSim->GetScore(0), TheSim->GetScore(1), Driver->GetAgreedCheckpoints()));
		UE_LOG(LogJamNet, Log, TEXT("JAMNET match over frame=%lld winner=%d %s"), TheSim->GetFrame(), TheSim->GetWinner(), *Driver->DescribeStats());
		bEnded = true;
	}
}
