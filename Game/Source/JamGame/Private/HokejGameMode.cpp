#include "HokejGameMode.h"
#include "JamCartridge.h"
#include "JamEventBus.h"
#include "JamPawns.h"
#include "JamBrain.h"
#include "JamSimAdapter.h"
#include "JamRuleSystem.h"
#include "JamZones.h"
#include "SignalSystem.h"
#include "RinkActor.h"
#include "LookTierSwitcher.h"
#include "HUDFeed.h"
#include "HUDWidget.h"
#include "InputMapping.h"
#include "CameraFollow.h"
#include "EffectsBinder.h"
#include "JamEffectSystem.h"
#include "JamPickupActor.h"
#include "ToySkaterVisual.h"
#include "JamDesignerSubsystem.h"
#include "Progression.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CommandLine.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogHokej);

static TAutoConsoleVariable<FString> CVarJamCut(TEXT("jam.Cut"), TEXT("hockey.regular"),
	TEXT("Cartridge the Hokej game mode loads at start (name in Content/Cartridges or a path). The -cut=<name> launch argument wins."));
static TAutoConsoleVariable<int32> CVarJamScriptHuman(TEXT("jam.ScriptHuman"), 0,
	TEXT("1: the human's skater plays itself (skate behind the puck, shoot when lined up). For PIE proofs through MCP."));

namespace
{
	const FName VisualToy(TEXT("toy"));
	const FName VisualB1(TEXT("humanoid_b1"));
	const FName VisualB2(TEXT("generated_b2"));
	constexpr int32 MaxStepsPerFrame = 4;
}

AHokejGameMode::AHokejGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	DefaultPawnClass = nullptr;
	EffectsBinder = CreateDefaultSubobject<UJamEffectsBinder>(TEXT("EffectsBinder"));
}

void AHokejGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	FString Cut;
	if (FParse::Value(FCommandLine::Get(), TEXT("-cut="), Cut) || FParse::Value(FCommandLine::Get(), TEXT("cut="), Cut)) CutName = Cut;
	else if (UGameplayStatics::HasOption(Options, TEXT("cut"))) CutName = UGameplayStatics::ParseOption(Options, TEXT("cut"));
	else CutName = CVarJamCut.GetValueOnGameThread();
}

void AHokejGameMode::StartPlay()
{
	Super::StartPlay();
	if (!LoadCut(CutName) && !bBuilt)
	{
		UE_LOG(LogHokej, Error, TEXT("JAMCART cut \"%s\" rejected; falling back to hockey.regular"), *CutName);
		LoadCut(TEXT("hockey.regular"));
	}
}

bool AHokejGameMode::LoadCut(const FString& NameOrPath)
{
	const FString Path = UJamCartridge::ResolveCartridgePath(NameOrPath);
	TArray<FString> Errors;
	UJamCartridge* C = UJamCartridge::LoadFromFile(Path, Errors, this);
	if (!C)
	{
		for (const FString& E : Errors) UE_LOG(LogHokej, Error, TEXT("JAMCART %s %s"), *NameOrPath, *E);
		return false;
	}
	Cartridge = C;
	CutName = NameOrPath;
	UE_LOG(LogHokej, Log, TEXT("JAMCART loaded %s id=%s title=\"%s\" seed=%lld rink=%gx%g visual=%s tier=%s"), *Path, *C->Id, *C->Title, C->Seed,
		C->Game.Rink.LengthM, C->Game.Rink.WidthM, *C->Game.Skater.Visual.ToString(), *C->Game.Look.Tier);
	if (UGameInstance* GI = GetGameInstance()) if (UJamDesignerSubsystem* D = GI->GetSubsystem<UJamDesignerSubsystem>()) D->SetCurrentCartridge(C);
	BuildMatch();
	return true;
}

bool AHokejGameMode::ApplyCartridgeLive(UJamCartridge* NewCartridge)
{
	if (!NewCartridge || !Sim || !bBuilt) return false;
	const TArray<FString> Held = Sim->ApplyLiveConfig(NewCartridge->Game);
	if (Effects) Effects->ApplyLiveConfig(NewCartridge->RawJson);   // lane A14: effects / pickups lists, the `director` trigger
	const FJamGameConfig& G = Sim->GetConfig();
	// R-0009 point 3: the brain resolves difficulty profile -> explicit game.ai -> Designer values itself (lane A3).
	Brain->ApplyCartridge(*NewCartridge);
	if (Rink && Rink->IceMid) Rink->IceMid->SetScalarParameterValue(TEXT("Wet"), G.Look.bWetIce ? 1.f : 0.f);
	if (ULookTierSwitcher* Look = GetWorld()->GetSubsystem<ULookTierSwitcher>())
	{
		Look->SetTrimEmissiveOverride(static_cast<float>(G.Look.TrimEmissive));   // B-to-A-0018 point 5 (lane A21): -1 = the tier's default
		Look->SetTier(G.Look.Tier, G.Rink.LengthM);   // R-0016 point 5: the rig scales with the rink
	}
	Cartridge = NewCartridge;
	if (UGameInstance* GI = GetGameInstance()) if (UJamDesignerSubsystem* D = GI->GetSubsystem<UJamDesignerSubsystem>()) D->SetCurrentCartridge(NewCartridge);
	UE_LOG(LogHokej, Log, TEXT("JAMGAME live cut id=%s at frame %lld; held back until the next match: %s"), *NewCartridge->Id, Sim->GetFrame(), Held.Num() ? *FString::Join(Held, TEXT(", ")) : TEXT("none"));
	PushPresentation();
	return true;
}

int32 AHokejGameMode::ComputeTake() const
{
	if (!Sim) return 1;
	const FJamRulesConfig& R = Sim->GetConfig().Rules;
	if (R.Mode == FName(TEXT("timer")) && R.TimerFrames > 0)
	{
		return FMath::Clamp(1 + (Sim->GetTimerElapsed() * 3) / R.TimerFrames, 1, 3);
	}
	const int32 Lead = FMath::Max(Sim->GetScore(0), Sim->GetScore(1));
	return R.ScoreToWin > 0 ? FMath::Clamp(1 + Lead, 1, R.ScoreToWin) : 1 + Lead;
}

void AHokejGameMode::UpdateTake(bool bForce)
{
	const int32 T = ComputeTake();
	if (!bForce && T == Take) return;
	Take = T;
	UGameInstance* GI = GetGameInstance();
	if (UJamEventBus* Bus = GI ? GI->GetSubsystem<UJamEventBus>() : nullptr)
	{
		FJamEventPayload P;
		P.Kind = FName(*FString::Printf(TEXT("take_%d"), Take));
		Bus->Broadcast(JamEvents::Period, P);
	}
	if (UJamDesignerSubsystem* D = GI ? GI->GetSubsystem<UJamDesignerSubsystem>() : nullptr) D->NotifyPeriodStarted(Take);
}

UClass* AHokejGameMode::ResolveVisualClass(FName Visual) const
{
	// contracts/skater-visual.md: BP_SkaterVisual_<track>; R-0001 item 1: game.skater.visual picks it.
	const TCHAR* Path = nullptr;
	if (Visual == VisualB1) Path = TEXT("/JamAssets/Skaters/BP_SkaterVisual_TB1.BP_SkaterVisual_TB1_C");
	else if (Visual == VisualB2) Path = TEXT("/JamAssets/Skaters/BP_SkaterVisual_TB2.BP_SkaterVisual_TB2_C");
	if (Path)
	{
		if (UClass* Cls = LoadClass<UObject>(nullptr, Path, nullptr, LOAD_NoWarn | LOAD_Quiet)) return Cls;
		UE_LOG(LogHokej, Warning, TEXT("JAMVISUAL %s not found at %s; using the toy figure"), *Visual.ToString(), Path);
	}
	return UToySkaterVisualComponent::StaticClass();
}

void AHokejGameMode::BuildMatch()
{
	UWorld* World = GetWorld();
	if (!World || !Cartridge) return;
	const FJamGameConfig& G = Cartridge->Game;

	for (AJamSkater* S : Skaters) if (S) S->Destroy();
	Skaters.Reset();
	if (Puck) Puck->Destroy();

	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (!Rink) Rink = World->SpawnActor<ARinkActor>(ARinkActor::StaticClass(), FTransform::Identity, P);
	Rink->Build(G);

	Sim = NewObject<UJamSim>(this);
	// Lane A15 (C-0005 seam B): the only IJamRuleHook, set before Init so the opening faceoff reaches it; the zone system reads sim state after each step.
	Rules = NewObject<UJamRuleSystem>(this);
	Rules->Init(G);
	Sim->SetRuleHook(Rules);
	ZoneSystem = NewObject<UJamZoneSystem>(this);
	ZoneSystem->Init(G);
	Sim->SetZoneSource(ZoneSystem);   // R-0019: the sim reads ice factors, no_shoot and bumper zones from it
	Sim->Init(G, static_cast<uint64>(Cartridge->Seed));
	BuiltRinkRevision = Sim->GetRinkRevision();
	Inputs.SetNum(Sim->GetSkaters().Num());

	// Lane A14 (C-0005): the effect system binds itself as the sim's modifier source and owns the pickups.
	AJamPickupActor::DestroyAll(PickupActors);
	Effects = NewObject<UJamEffectSystem>(this);
	{
		TArray<FString> EffectErrors;
		Effects->Init(Sim, Cartridge->RawJson, EffectErrors);
		for (const FString& E : EffectErrors) UE_LOG(LogHokej, Error, TEXT("JAMEFFECTS %s"), *E);
	}
	LastBoost.Init(NAME_None, Sim->GetSkaters().Num());

	Brain = NewObject<UJamBrain>(this);
	// R-0009 point 3: the brain resolves difficulty profile -> explicit game.ai -> Designer values itself (lane A3).
	Brain->ApplyCartridge(*Cartridge);
	Brain->ResetBrain();

	UClass* VisualClass = ResolveVisualClass(G.Skater.Visual);
	for (const FJamSkaterState& S : Sim->GetSkaters())
	{
		AJamSkater* A = World->SpawnActor<AJamSkater>(AJamSkater::StaticClass(), FTransform(FRotator(0, FMath::RadiansToDegrees(S.Yaw), 0), UJamSim::ToWorld(S.Pos)), P);
		const int32 InTeam = S.Index - (S.Team == 0 ? 0 : G.Teams[0].Skaters.Num());
		A->InitSkater(S.Index, S.Team, G.Teams[S.Team].Colour, S.Team * 10 + InTeam + 1, VisualClass);
#if WITH_EDITOR
		A->SetActorLabel(FString::Printf(TEXT("Skater_%d_%s_%s"), S.Index, *G.Teams[S.Team].Id, *S.Role.ToString()));
#endif
		Skaters.Add(A);
	}
	Puck = World->SpawnActor<AJamPuck>(AJamPuck::StaticClass(), FTransform::Identity, P);
	Puck->InitPuck(G.Puck.RadiusM, G.Puck.Colour);

	if (ULookTierSwitcher* Look = World->GetSubsystem<ULookTierSwitcher>())
	{
		Look->SetTrimEmissiveOverride(static_cast<float>(G.Look.TrimEmissive));   // B-to-A-0018 point 5 (lane A21): -1 = the tier's default
		Look->SetTier(G.Look.Tier, G.Rink.LengthM);   // R-0016 point 5: the rig scales with the rink
	}

	Accumulator = 0.0;
	StepMsSum = StepMsMax = 0.0;
	Steps = 0;
	bBuilt = true;
	bResultReported = false;
	// The opening faceoff event, TAKE 1 and the first presentation frame.
	DeliverEvents();
	UpdateTake(true);
	for (AJamSkater* A : Skaters)
	{
		A->ApplySimFrame(Sim->GetSkaters()[A->GetSimIndex()]);
		if (Effects && LastBoost.IsValidIndex(A->GetSimIndex()))
		{
			const FName Boost = Effects->GetBoostKindFor(A->GetSimIndex());   // lane A14: the toy figure glows while boosted
			if (Boost != LastBoost[A->GetSimIndex()])
			{
				LastBoost[A->GetSimIndex()] = Boost;
				if (UObject* V = A->GetVisualObject()) IJamSkaterVisual::Execute_SetBoost(V, Boost);
			}
		}
	}
	PushPresentation();
	for (AJamSkater* A : Skaters) A->ApplyRender(Sim->GetSkaters()[A->GetSimIndex()], 1.f);
	Puck->ApplyRender(Sim->GetPuck(), 1.f);

	if (APlayerController* PC = World->GetFirstPlayerController()) PossessHuman(PC);
}

AJamSkater* AHokejGameMode::GetHumanSkater() const
{
	return (Sim && Skaters.IsValidIndex(Sim->GetHumanIndex())) ? Skaters[Sim->GetHumanIndex()].Get() : nullptr;
}

void AHokejGameMode::PossessHuman(APlayerController* PC)
{
	AJamSkater* Human = GetHumanSkater();
	if (!PC || !Human) return;
	if (!Human->FindComponentByClass<UJamFollowCamera>())
	{
		UJamFollowCamera* Cam = NewObject<UJamFollowCamera>(Human, TEXT("FollowCamera"));
		Cam->SetupAttachment(Human->GetRootComponent());
		Cam->RegisterComponent();
		Human->AddInstanceComponent(Cam);
		Cam->ApplyConfig(FJamCameraSettings::FromCartridge(Cartridge->Game.Camera));
	}
	UJamPlayerInput* In = Human->FindComponentByClass<UJamPlayerInput>();
	if (!In)
	{
		In = NewObject<UJamPlayerInput>(Human, TEXT("PlayerInput"));
		In->RegisterComponent();
		Human->AddInstanceComponent(In);
	}
	In->OnRestart.AddUniqueDynamic(this, &AHokejGameMode::HandleRestartPressed);
	if (PC->GetPawn() != Human) PC->Possess(Human);
	PC->SetViewTarget(Human);
	if (!Hud) Hud = UJamHudWidget::CreateAndAdd(PC);
	PushPresentation();
}

void AHokejGameMode::RestartPlayer(AController* NewPlayer)
{
	// No default pawn: the human's skater is spawned by BuildMatch and possessed there.
	if (APlayerController* PC = Cast<APlayerController>(NewPlayer)) if (bBuilt) PossessHuman(PC);
}

void AHokejGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (bBuilt) PossessHuman(NewPlayer);
}

void AHokejGameMode::HandleRestartPressed()
{
	// Lane A17: at the end of a season take, R after a win opens the next take (the season decides); otherwise the same cut restarts (RETAKE).
	if (Sim && Sim->GetPhase() == EJamSimPhase::Over && Cartridge)
	{
		if (UJamProgression* P = UJamProgression::Get(this); P && P->HandleRestart(this)) return;
	}
	RestartMatch();
}

void AHokejGameMode::RestartMatch()
{
	if (!Sim) return;
	if (Rules) Rules->ResetMatch();          // lane A15: before the sim's Restart so its faceoff counts in the new match
	if (ZoneSystem) ZoneSystem->ResetMatch();
	Sim->Restart();
	if (Effects) Effects->Restart();   // lane A14
	AJamPickupActor::DestroyAll(PickupActors);
	Brain->ResetBrain();
	bResultReported = false;
	Accumulator = 0.0;
	UE_LOG(LogHokej, Log, TEXT("JAMGAME restart (%s)"), *CutName);
	DeliverEvents();
	UpdateTake(true);
	PushPresentation();
}

// The scripted human (jam.ScriptHuman 1, PIE proofs): chases the puck, carries it towards the net, passes once when a teammate is open and the net is
// far, and charges its shot (holds shoot up to charge_frames_max, then releases) when lined up inside 30 m. Deterministic: sim state only.
FJamInput AHokejGameMode::ScriptedHumanInput()
{
	const int32 Me = Sim->GetHumanIndex();
	const FJamSkaterState& S = Sim->GetSkaters()[Me];
	const FVector2D PuckPos = Sim->GetPuck().Pos;
	const FVector2D Goal(Sim->AttackGoalX(S.Team), 0.0);
	const FVector2D ToGoal = (Goal - PuckPos).GetSafeNormal();
	const FVector2D Behind = PuckPos - ToGoal * 0.7;
	FJamInput In;
	const FVector2D D = Behind - S.Pos;
	In.Move = D.Size() > 0.3 ? D.GetSafeNormal() : ToGoal;
	const FVector2D ToPuck = PuckPos - S.Pos;
	const double Dist = ToPuck.Size();
	const double GoalDist = FVector2D::Distance(PuckPos, Goal);
	const bool bHave = Dist < Sim->GetConfig().Shot.ReachM && Dist > 1e-6 && FVector2D::DotProduct(S.Forward(), ToPuck / Dist) > 0.6;
	// In the faceoff: press at the drop (the first clean press wins the draw).
	if (Sim->IsDrawOpen() && S.bCentre) { In.bPoke = true; return In; }
	if (Sim->GetPhase() != EJamSimPhase::Playing) { ScriptCharge = 0; return In; }
	FVector2D Aim;
	if (bHave && GoalDist > 30.0 && Sim->GetFrame() - ScriptLastPass > 600 && Sim->BestPass(Me, Aim) >= 0)
	{
		In.bPass = true;
		ScriptLastPass = Sim->GetFrame();
		ScriptCharge = 0;
		return In;
	}
	const int32 Max = Sim->GetConfig().Shot.ChargeFramesMax;
	if (bHave && FVector2D::DotProduct(S.Forward(), ToGoal) > 0.97 && GoalDist < 30.0)
	{
		if (Max <= 0) { In.bShoot = true; return In; }
		if (ScriptCharge < Max) { In.bShootHeld = true; ++ScriptCharge; return In; }
		ScriptCharge = 0;   // release: the charged shot fires this frame
		return In;
	}
	if (ScriptCharge > 0 && bHave) { In.bShootHeld = true; ++ScriptCharge; if (ScriptCharge >= Max + 10) ScriptCharge = 0; return In; }   // keep the charge while turning in
	ScriptCharge = 0;
	return In;
}

FJamInput AHokejGameMode::ReadHumanInput()
{
	FJamInput In;
	AJamSkater* Human = GetHumanSkater();
	UJamPlayerInput* Pi = Human ? Human->FindComponentByClass<UJamPlayerInput>() : nullptr;
	if (!Pi) return In;
	// Camera-relative: stick up = away from the camera.
	double Yaw = Sim->GetSkaters()[Sim->GetHumanIndex()].Yaw;
	if (APlayerController* PC = Cast<APlayerController>(Human->GetController()))
	{
		if (PC->PlayerCameraManager) Yaw = FMath::DegreesToRadians(PC->PlayerCameraManager->GetCameraRotation().Yaw);
	}
	const FVector2D Axis = Pi->GetAxis();
	const FVector2D Fwd(FMath::Cos(Yaw), FMath::Sin(Yaw));
	const FVector2D Right(-FMath::Sin(Yaw), FMath::Cos(Yaw));
	In.Move = Fwd * Axis.Y + Right * Axis.X;
	// C-0005 charged shot: the human sends the held level; the sim charges while the puck is in reach and fires on release (a tap = a short charge).
	Pi->ConsumeShootPressed();
	In.bShootHeld = Pi->IsShootHeld();
	In.bPass = Pi->ConsumePassPressed();
	In.bCheck = Pi->ConsumeCheckPressed();
	In.bBrake = Pi->IsBrakeHeld();  // A5: Ctrl / left trigger (FEEL.md proposal 6)
	// R-0013 (lane A16): the stick. Right stick is absolute; the mouse accumulates pixels, 100 px = one reach at mouse_sensitivity 1. Camera-relative like Move.
	FVector2D Aim = Pi->GetStickAxis();
	if (Aim.SizeSquared() < 0.01)
	{
		Aim = Pi->GetMouseAimPx() * (0.01 * Sim->GetConfig().Stick.MouseSensitivity);
		if (Aim.SizeSquared() > 1.0) Aim.Normalize();
	}
	In.Stick = Fwd * Aim.Y + Right * Aim.X;
	In.bPoke = Pi->ConsumePokePressed();
	In.bLift = Pi->ConsumeLiftPressed();
	return In;
}

void AHokejGameMode::GatherInputs()
{
	JamAI::FJamAIWorld W;
	JamAI::FJamSimAdapter::BuildWorld(*Sim, W);
	// lane A3 (R-0009): the human's active team signal into the world the brain reads (FJamAIWorld::Intent*, frames, deterministic)
	if (const UWorld* World = GetWorld())
	{
		FJamTeamIntent Intent;
		const UJamSignalSystem* Signals = World->GetSubsystem<UJamSignalSystem>();
		if (Signals && Signals->GetActiveIntent(Intent))
		{
			W.IntentKind = static_cast<int32>(Intent.Intent);   // EJamTeamIntent: None 0, Defend 1, Space 2, Pass 3, Praise 4
			W.IntentTarget = Intent.TargetIndex;
			W.IntentFramesLeft = Intent.FramesLeft(Sim->GetFrame());
		}
	}
	// lane A3 (R-0017): the pickups on the ice (only all_star bots detour for them)
	if (Effects)
	{
		for (const FJamPickupOnIce& P : Effects->GetPickups())
		{
			if (W.NumPickups >= JamAI::FJamAIWorld::kMaxPickups) break;
			W.PickupKind[W.NumPickups] = P.PickupIndex;
			W.PickupPos[W.NumPickups] = JamAI::FVec2(P.Pos.X, P.Pos.Y);
			++W.NumPickups;
		}
	}
	// lane A3 (R-0019): slow-ice circles (rects by half diagonal) and who stands in a no_shoot zone
	if (ZoneSystem)
	{
		for (const FJamZoneConfig& Z : ZoneSystem->GetZones())
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
			W.Skaters[i].bNoShoot = ZoneSystem->BlocksShot(FVector2D(W.Skaters[i].Pos.X, W.Skaters[i].Pos.Y), ZoneId);
		}
	}
	JamAI::FJamAIInput Out[JamAI::kMaxSkaters];
	Brain->Think(W, Out);
	const int32 Human = Sim->GetHumanIndex();
	for (int32 i = 0; i < Inputs.Num(); ++i)
	{
		if (i == Human) continue;
		FJamInput& I = Inputs[i];
		if (i < JamAI::kMaxSkaters)
		{
			I.Move = FVector2D(Out[i].Move.X, Out[i].Move.Y);
			I.bShoot = Out[i].bShoot;
			I.bCheck = Out[i].bCheck;
			I.bBrake = Out[i].bBrake;
			I.bPass = Out[i].bPass;   // lane A3 (R-0017): pass, stick aim, poke, lift
			I.Stick = FVector2D(Out[i].Stick.X, Out[i].Stick.Y);
			I.bPoke = Out[i].bPoke;
			I.bLift = Out[i].bLift;
		}
	}
	Inputs[Human] = (bScriptedHuman || CVarJamScriptHuman.GetValueOnGameThread() != 0) ? ScriptedHumanInput() : ReadHumanInput();
}

void AHokejGameMode::DeliverEvents()
{
	UGameInstance* GI = GetGameInstance();
	UJamEventBus* Bus = GI ? GI->GetSubsystem<UJamEventBus>() : nullptr;
	if (!Bus || !Sim) return;
	Bus->SetSimFrame(Sim->GetFrame());
	for (const FJamSimEvent& E : Sim->GetFrameEvents())
	{
		FJamEventPayload Pl;
		Pl.Who = E.Who;
		Pl.Where = UJamSim::ToWorld(E.Where, E.Name == JamEvents::Board || E.Name == JamEvents::Shot || E.Name == JamEvents::Goal ? 0.04 : 0.0);
		Pl.Strength = E.Strength;
		Pl.Kind = E.Kind;
		Bus->Broadcast(E.Name, Pl);
	}
	if (Effects)
	{
		for (const FJamSimEvent& E : Effects->GetFrameEvents())
		{
			if (!UJamEventBus::IsKnownEvent(E.Name)) continue;
			FJamEventPayload Pl;
			Pl.Who = E.Who;
			Pl.Where = UJamSim::ToWorld(E.Where, AJamPickupActor::HoverM);
			Pl.Strength = E.Strength;
			Pl.Kind = E.Kind;
			Bus->Broadcast(E.Name, Pl);
		}
	}
	// Lane A15 (C-0005): zone_enter / zone_exit from the zone system's membership pass (Who = skater or -1 for the puck, Kind = zone id),
	// and "announce" when a rule spoke this frame but the sim did not carry the event itself.
	if (ZoneSystem)
	{
		for (const FJamSimEvent& E : ZoneSystem->GetFrameEvents())
		{
			FJamEventPayload Pl;
			Pl.Who = E.Who;
			Pl.Where = UJamSim::ToWorld(E.Where);
			Pl.Strength = E.Strength;
			Pl.Kind = E.Kind;
			Bus->Broadcast(E.Name, Pl);
		}
	}
	if (Rules)
	{
		TArray<FString> Said;
		Rules->TakePendingAnnouncements(Said);
		bool bSimAnnounced = false;
		for (const FJamSimEvent& E : Sim->GetFrameEvents()) if (E.Name == JamEvents::Announce) bSimAnnounced = true;
		if (Said.Num() && !bSimAnnounced)
		{
			FJamEventPayload Pl;
			Pl.Kind = FName(TEXT("rule"));
			Bus->Broadcast(JamEvents::Announce, Pl);
		}
	}
}

void AHokejGameMode::PushPresentation()
{
	UJamPresentation* Pres = UJamPresentation::Get(this);
	if (!Pres || !Sim || !Cartridge) return;
	const FJamGameConfig& G = Cartridge->Game;
	FJamPresentationFrame F;
	F.SimFrame = Sim->GetFrame();
	// Lane A15 (C-0005): the sim's rink, not the cartridge's: ShrinkRink / GrowRink change it at a faceoff and the radar and net markers follow.
	F.RinkLengthM = Sim->GetConfig().Rink.LengthM;
	F.RinkWidthM = Sim->GetConfig().Rink.WidthM;
	for (const FJamSkaterState& S : Sim->GetSkaters())
	{
		FJamPresentationSkater Ps;
		Ps.PosM = S.Pos;
		Ps.YawDeg = FMath::RadiansToDegrees(S.Yaw);
		Ps.TeamId = S.Team;
		Ps.bHuman = S.bHuman;
		F.Skaters.Add(Ps);
	}
	F.PuckM = Sim->GetPuck().Pos;
	F.HumanIndex = Sim->GetHumanIndex();
	F.AttackSign = Sim->GetHumanTeam() == 0 ? 1 : -1;
	F.NetInsetM = 4.f;
	// Home = the human's team.
	F.ScoreHome = Sim->GetScore(Sim->GetHumanTeam());
	F.ScoreAway = Sim->GetScore(1 - Sim->GetHumanTeam());
	F.Take = FMath::Max(1, Take);
	F.ScoreToWin = G.Rules.Mode == FName(TEXT("first_to")) ? G.Rules.ScoreToWin : 0;  // A5: the TAKE card reads the rule
	F.bHasTimer = G.Rules.Mode == FName(TEXT("timer"));
	F.FramesRemaining = Sim->GetTimerFrames();
	switch (Sim->GetPhase())
	{
	case EJamSimPhase::Playing: F.Phase = EJamPhase::Play; break;
	case EJamSimPhase::Faceoff: F.Phase = EJamPhase::Play; F.FaceoffCountdownFrames = Sim->GetFaceoffCountdown(); break;
	case EJamSimPhase::GoalPause: F.Phase = EJamPhase::GoalPause; break;
	case EJamSimPhase::Over: F.Phase = Sim->GetWinner() == Sim->GetHumanTeam() ? EJamPhase::Win : EJamPhase::Lose; break;
	}
	if (G.Teams.Num() == 2) { F.TeamColour0 = G.Teams[0].Colour; F.TeamColour1 = G.Teams[1].Colour; }
	Pres->PushFrame(F);
}

void AHokejGameMode::StepOnce()
{
	GatherInputs();
	if (Rules) { Rules->SetScore(0, Sim->GetScore(0)); Rules->SetScore(1, Sim->GetScore(1)); }   // lane A15: the score mirror the hooks' conditions read
	const double T0 = FPlatformTime::Seconds();
	Sim->Step(Inputs);
	if (ZoneSystem) ZoneSystem->Update(Sim->GetFrame(), Sim->GetSkaters(), Sim->GetPuck());   // lane A15: membership and zone events for this frame
	const double Ms = (FPlatformTime::Seconds() - T0) * 1000.0;
	StepMsSum += Ms;
	StepMsMax = FMath::Max(StepMsMax, Ms);
	++Steps;
	if (Effects) Effects->Step();   // lane A14: after the sim's frame, before the events go out
	DeliverEvents();
	// Lane A15 (C-0005): ShrinkRink / GrowRink changed the geometry at this faceoff; rebuild the rink actor from the sim's config.
	if (Rink && Sim->GetRinkRevision() != BuiltRinkRevision)
	{
		BuiltRinkRevision = Sim->GetRinkRevision();
		Rink->Build(Sim->GetConfig());
		if (ULookTierSwitcher* Look = GetWorld()->GetSubsystem<ULookTierSwitcher>())
		{
			Look->SetTrimEmissiveOverride(static_cast<float>(Sim->GetConfig().Look.TrimEmissive));   // B-to-A-0018 point 5 (lane A21)
			Look->SetTier(Sim->GetConfig().Look.Tier, Sim->GetConfig().Rink.LengthM);   // R-0016 point 5: the rig is re-scaled for the shrunk rink
		}
		UE_LOG(LogHokej, Log, TEXT("JAMRINK rebuilt at frame %lld: %gx%g m (revision %d)"), Sim->GetFrame(), Sim->GetConfig().Rink.LengthM, Sim->GetConfig().Rink.WidthM, BuiltRinkRevision);
	}
	if (Sim->GetPhase() == EJamSimPhase::Playing) UpdateTake(false);
	// Lane A17: a finished match is reported once to the season (ignored unless this cut is the running season take).
	if (Sim->GetPhase() == EJamSimPhase::Over && !bResultReported && Cartridge)
	{
		bResultReported = true;
		if (UJamProgression* P = UJamProgression::Get(this)) P->RecordResult(Cartridge->SourcePath, Sim->GetWinner() == Sim->GetHumanTeam());
	}
	for (AJamSkater* A : Skaters)
	{
		A->ApplySimFrame(Sim->GetSkaters()[A->GetSimIndex()]);
		if (Effects && LastBoost.IsValidIndex(A->GetSimIndex()))
		{
			const FName Boost = Effects->GetBoostKindFor(A->GetSimIndex());   // lane A14: the toy figure glows while boosted
			if (Boost != LastBoost[A->GetSimIndex()])
			{
				LastBoost[A->GetSimIndex()] = Boost;
				if (UObject* V = A->GetVisualObject()) IJamSkaterVisual::Execute_SetBoost(V, Boost);
			}
		}
	}
	PushPresentation();
	if (Sim->GetFrame() % 600 == 0)
	{
		UE_LOG(LogHokej, Log, TEXT("JAMSIM frame=%lld hash=%s score=%d:%d phase=%d step_ms mean=%.4f worst=%.4f"), Sim->GetFrame(), *Sim->StateHashHex(),
			Sim->GetScore(0), Sim->GetScore(1), static_cast<int32>(Sim->GetPhase()), GetMeanStepMs(), GetWorstStepMs());
	}
}

void AHokejGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bBuilt || !Sim) return;
	Accumulator += DeltaSeconds;
	int32 N = 0;
	while (Accumulator >= UJamSim::DT && N < MaxStepsPerFrame)
	{
		StepOnce();
		Accumulator -= UJamSim::DT;
		++N;
	}
	if (N == MaxStepsPerFrame && Accumulator > UJamSim::DT) Accumulator = 0.0; // fell behind: drop the backlog, never spiral
	const float Alpha = static_cast<float>(FMath::Clamp(Accumulator / UJamSim::DT, 0.0, 1.0));
	for (AJamSkater* A : Skaters) A->ApplyRender(Sim->GetSkaters()[A->GetSimIndex()], Alpha);
	// C-0001 v3: a carried puck is drawn at the carrier visual's puck_socket when the visual exposes one (cosmetic; the sim keeps its own position).
	{
		FVector SocketCm;
		const int32 Carrier = Sim->CarrierIndex();
		if (Skaters.IsValidIndex(Carrier) && Skaters[Carrier] && Skaters[Carrier]->GetPuckSocketWorld(SocketCm)) Puck->ApplyRenderAt(SocketCm);
		else Puck->ApplyRender(Sim->GetPuck(), Alpha);
	}
	AJamPickupActor::Sync(GetWorld(), Effects, PickupActors, static_cast<double>(Sim->GetFrame()) + Alpha);   // lane A14
	if (UJamPresentation* Pres = UJamPresentation::Get(this)) Pres->SetRenderAlpha(Alpha);
}
