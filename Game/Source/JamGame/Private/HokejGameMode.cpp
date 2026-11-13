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
#include "CommentarySystem.h"
#include "JamEffectSystem.h"
#include "JamPickupActor.h"
#include "ToySkaterVisual.h"
#include "SkeletalSkaterVisual.h"
#include "JamDesignerSubsystem.h"
#include "Progression.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "EngineUtils.h"
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
// Lane A49: the tackle / cross-check drill (packaged proof of complaint 5). 1 = tackle (right mouse), 2 = cross-check (middle mouse); resets to 0 after 10 attempts.
static TAutoConsoleVariable<int32> CVarJamCtrlDrill(TEXT("jam.ctrl.Drill"), 0,
	TEXT("1 = tackle drill, 2 = cross-check drill: the human chases the carrier and presses once in range, 10 attempts, JAMDRILL lines. Lane A49 proof. 3 = sweep drill, 4 = spin drill (lane A51): the cursor and the keys stay real, the skating carries the puck at the nearest defender; JAMDRILL stick lines."));
// Lane A31: a Director's Call from the console: load a cartridge (name or path) and apply it to the running match without a reset
// (AHokejGameMode::ApplyCartridgeLive), the way the Designer does. A tier change swaps the rig and respawns the figures in place.
static FAutoConsoleCommandWithWorldAndArgs CmdJamLiveCut(TEXT("jam.livecut"), TEXT("jam.livecut <name-or-path>: apply a cartridge to the running match live (Director's Call); no reset"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		AHokejGameMode* GM = World ? Cast<AHokejGameMode>(World->GetAuthGameMode()) : nullptr;
		if (!GM || Args.Num() < 1) { UE_LOG(LogHokej, Warning, TEXT("jam.livecut: needs a running Hokej match and a cartridge name or path")); return; }
		TArray<FString> Errors;
		UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(Args[0]), Errors, GM);
		if (!C) { for (const FString& E : Errors) UE_LOG(LogHokej, Error, TEXT("JAMCART %s %s"), *Args[0], *E); return; }
		UE_LOG(LogHokej, Log, TEXT("jam.livecut %s -> %s"), *Args[0], GM->ApplyCartridgeLive(C) ? TEXT("applied") : TEXT("refused"));
	}));

namespace
{
	const FName VisualToy(TEXT("toy"));
	const FName VisualB1(TEXT("humanoid_b1"));
	const FName VisualB2(TEXT("generated_b2"));
	const FName VisualSkeletal(TEXT("toy_skeletal"));   // lane A26 (R-0024): the Blender toy figure as a skeletal visual
	constexpr int32 MaxStepsPerFrame = 4;
	// Lane A31 (B-to-A-0021 / 0025): the tiers that carry their own generated_b2 figure.
	const TCHAR* TierClay = TEXT("p3_clay");
	const TCHAR* TierBroadcast = TEXT("p4_broadcast");
	constexpr int8 GoalieOff = -2;
	constexpr int64 GoalieHoldFrames = 24;   // the stance stays at least 0.4 s once it came on
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
	// Lane A31 (B-to-A-0021 point 1): the figure follows the tier at once; a changed class respawns the visuals with the setup calls repeated.
	RespawnVisualsIfClassChanged(NewCartridge->Game);
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

TArray<FString> AHokejGameMode::VisualClassPathsFor(FName Visual, const FString& Tier)
{
	TArray<FString> Paths;   // in order of preference; the first that loads wins
	if (Visual == VisualB1)
	{
		// B-to-A-0033 point 1 (R-0033, lane A50): the shipped figure's clay version BP_SkaterVisual_TB1_P3 on p3_clay, the night figure as its fallback;
		// every other tier (the broadcast lab included, for now) the night figure. A TB1_P4 row returns when B imports one (R-0033 point 1: p4 is a lab).
		if (Tier == TierClay) Paths.Add(TEXT("/JamAssets/Skaters/BP_SkaterVisual_TB1_P3.BP_SkaterVisual_TB1_P3_C"));
		Paths.Add(TEXT("/JamAssets/Skaters/BP_SkaterVisual_TB1.BP_SkaterVisual_TB1_C"));
	}
	else if (Visual == VisualB2)
	{
		// B-to-A-0021 point 1 (lane A31): the figure follows the look tier; the hybrid figure is the fallback of both tier figures.
		if (Tier == TierClay) Paths.Add(TEXT("/JamAssets/Skaters/BP_SkaterVisual_TB2_P3.BP_SkaterVisual_TB2_P3_C"));
		else if (Tier == TierBroadcast) Paths.Add(TEXT("/JamAssets/Skaters/BP_SkaterVisual_TB2_P4.BP_SkaterVisual_TB2_P4_C"));
		Paths.Add(TEXT("/JamAssets/Skaters/BP_SkaterVisual_TB2.BP_SkaterVisual_TB2_C"));
	}
	return Paths;
}

UClass* AHokejGameMode::ResolveVisualClass(FName Visual, const FString& Tier) const
{
	// contracts/skater-visual.md: BP_SkaterVisual_<track>; R-0001 item 1: game.skater.visual picks it.
	if (Visual == VisualSkeletal) return USkeletalSkaterVisual::StaticClass();
	const TArray<FString> Paths = VisualClassPathsFor(Visual, Tier);
	for (int32 i = 0; i < Paths.Num(); ++i)
	{
		// Lane A65: the menu's preload has usually loaded the class already; LoadClass then finds it in memory without a synchronous load.
		if (UClass* Cls = LoadClass<UObject>(nullptr, *Paths[i], nullptr, LOAD_NoWarn | LOAD_Quiet))
		{
			UE_LOG(LogHokej, Log, TEXT("JAMVISUAL %s tier=%s class=%s%s"), *Visual.ToString(), *Tier, *Cls->GetPathName(), i > 0 ? TEXT(" (fallback)") : TEXT(""));
			return Cls;
		}
		UE_LOG(LogHokej, Warning, TEXT("JAMVISUAL %s tier=%s: %s not found; %s"), *Visual.ToString(), *Tier, *Paths[i], i + 1 < Paths.Num() ? TEXT("trying the next class") : TEXT("using the toy figure"));
	}
	return UToySkaterVisualComponent::StaticClass();
}

int32 AHokejGameMode::JerseyNumberFor(int32 Team, int32 SlotInTeam)
{
	static const int32 Numbers[2][3] = { { 7, 13, 42 }, { 9, 21, 88 } };
	if (SlotInTeam >= 0 && SlotInTeam < 3 && (Team == 0 || Team == 1)) return Numbers[Team][SlotInTeam];
	return FMath::Clamp(Team * 10 + SlotInTeam + 1, 0, 99);
}

void AHokejGameMode::RespawnVisualsIfClassChanged(const FJamGameConfig& G)
{
	if (!Sim) return;
	UClass* NewClass = ResolveVisualClass(G.Skater.Visual, G.Look.Tier);
	if (!NewClass || NewClass == SpawnedVisualClass) return;
	int32 N = 0;
	for (AJamSkater* A : Skaters)
	{
		if (!A) continue;
		const FJamSkaterState& S = Sim->GetSkaters()[A->GetSimIndex()];
		const int32 InTeam = S.Index - (S.Team == 0 ? 0 : G.Teams[0].Skaters.Num());
		if (A->ResetVisual(G.Teams[S.Team].Colour, JerseyNumberFor(S.Team, InTeam), NewClass)) ++N;
		// the boost and the goalie stance are re-sent by the next step (they compare against these)
		if (LastBoost.IsValidIndex(S.Index)) LastBoost[S.Index] = FName(TEXT("__respawn"));
		if (LastGoalie.IsValidIndex(S.Index)) LastGoalie[S.Index] = GoalieOff - 1;
		A->ApplySimFrame(S);
	}
	SpawnedVisualClass = NewClass;
	UE_LOG(LogHokej, Log, TEXT("JAMVISUAL respawned %d skater visuals as %s for tier %s at frame %lld (setup calls repeated)"), N, *NewClass->GetPathName(), *G.Look.Tier, Sim->GetFrame());
}

void AHokejGameMode::BuildMatch()
{
	UWorld* World = GetWorld();
	if (!World || !Cartridge) return;
	const FJamGameConfig& G = Cartridge->Game;

	// Lane A47 (B-to-A-0030 fault 6): a Blueprint visual is a child actor attached to the skater, and AActor::Destroy on the skater leaves it standing on the
	// ice (18 ghosts after three LoadCut rebuilds in QA1). The visual goes first (A31's ResetVisual teardown), the skater after.
	int32 VisualActors = 0, VisualComponents = 0;
	for (AJamSkater* S : Skaters)
	{
		if (!S) continue;
		if (AActor* VA = Cast<AActor>(S->GetVisualObject())) { VA->Destroy(); ++VisualActors; }
		else if (UActorComponent* VC = Cast<UActorComponent>(S->GetVisualObject())) { VC->DestroyComponent(); ++VisualComponents; }
		S->Destroy();
	}
	if (Skaters.Num()) UE_LOG(LogHokej, Log, TEXT("JAMVISUAL teardown: %d skaters, %d visual actors and %d visual components destroyed before them"), Skaters.Num(), VisualActors, VisualComponents);
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

	UClass* VisualClass = ResolveVisualClass(G.Skater.Visual, G.Look.Tier);
	SpawnedVisualClass = VisualClass;
	LastGoalie.Init(GoalieOff, Sim->GetSkaters().Num());
	GoalieOnFrame.Init(0, Sim->GetSkaters().Num());
	for (const FJamSkaterState& S : Sim->GetSkaters())
	{
		AJamSkater* A = World->SpawnActor<AJamSkater>(AJamSkater::StaticClass(), FTransform(FRotator(0, FMath::RadiansToDegrees(S.Yaw), 0), UJamSim::ToWorld(S.Pos)), P);
		const int32 InTeam = S.Index - (S.Team == 0 ? 0 : G.Teams[0].Skaters.Num());
		A->InitSkater(S.Index, S.Team, G.Teams[S.Team].Colour, JerseyNumberFor(S.Team, InTeam), VisualClass);   // B-to-A-0021 point 2: 7/13/42 and 9/21/88
#if WITH_EDITOR
		A->SetActorLabel(FString::Printf(TEXT("Skater_%d_%s_%s"), S.Index, *G.Teams[S.Team].Id, *S.Role.ToString()));
#endif
		Skaters.Add(A);
	}
	{
		// Lane A47: the ghost count the proof reads. Every skater visual actor in the world should belong to a skater built just now.
		int32 VisualsInWorld = 0;
		for (TActorIterator<AActor> It(World); It; ++It) if (It->GetClass()->GetName().Contains(TEXT("SkaterVisual"))) ++VisualsInWorld;
		UE_LOG(LogHokej, Log, TEXT("JAMVISUAL after build: %d skaters, %d SkaterVisual actors in the world (ghosts=%d)"), Skaters.Num(), VisualsInWorld, FMath::Max(0, VisualsInWorld - Skaters.Num()));
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
	// Lane A32 (B-to-A-0026): the commentator's loops (MS_music_match, MS_skate_loop) and the season_take event, after the period event and before the first frame.
	if (UJamCommentarySystem* Commentary = UJamCommentarySystem::Get(this)) Commentary->OnMatchBuilt(this);
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
	if (!Hud)
	{
		Hud = UJamHudWidget::CreateAndAdd(PC);
		// Lane A49: the HUD never takes a click; with the cursor shown, a left button over a plate would otherwise be eaten by Slate instead of shooting.
		if (Hud) Hud->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	bMatchInputModeSet = false;
	EnsureMatchInputMode(PC);
	PushPresentation();
}

void AHokejGameMode::EnsureMatchInputMode(APlayerController* PC)
{
	if (!PC || !PC->IsLocalController()) return;
	UWorld* W = GetWorld();
	if (W && W->IsPaused()) return;   // the pause menu (Director's Call) owns the cursor while it is open
	if (bMatchInputModeSet && PC->bShowMouseCursor) return;
	PC->bShowMouseCursor = true;
	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	Mode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(Mode);
	bMatchInputModeSet = true;
	UE_LOG(LogHokej, Log, TEXT("JAMCTRL input mode set: game+ui, cursor shown and locked to the viewport, the HUD hit-test invisible (lane A49)"));
}

bool AHokejGameMode::CursorAimRink(APlayerController* PC, const FVector2D& SkaterPosM, FVector2D& OutStick)
{
	OutStick = FVector2D::ZeroVector;
	if (!PC || !PC->bShowMouseCursor) return false;
	float MX = 0.f, MY = 0.f;
	if (!PC->GetMousePosition(MX, MY)) return false;
	FVector O, D;
	if (!PC->DeprojectScreenPositionToWorld(MX, MY, O, D) || D.Z >= -1e-4) return false;
	const FVector Hit = O + D * (-O.Z / D.Z);   // the ice is z = 0 (UJamSim::ToWorld)
	const FVector2D Delta = FVector2D(Hit.X / 100.0, Hit.Y / 100.0) - SkaterPosM;
	const double Dist = Delta.Size();
	const double Enter = bCursorAiming ? 0.5 : 0.7;   // hysteresis: a cursor parked on the skater holds the blade ahead without flapping at the edge
	if (Dist < Enter) { bCursorAiming = false; return true; }
	bCursorAiming = true;
	const double Mag = 0.5 + 0.5 * FMath::Clamp((Dist - 0.5) / 2.5, 0.0, 1.0);
	OutStick = Delta / Dist * Mag;
	return true;
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
	UE_LOG(LogHokej, Log, TEXT("JAMCTRL frame=%lld btn=restart (R / View)"), Sim ? Sim->GetFrame() : -1);
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
	In.bBrake = Pi->IsBrakeHeld();  // A5: Ctrl / gamepad B (FEEL.md proposal 6)
	In.bSprint = Pi->IsSprintHeld();   // lane A49: Shift / LT
	// R-0013 (lane A16) + lane A49: the stick. Right stick first (absolute, camera-relative like Move); then the cursor on the ice (the blade points at the cursor,
	// rink-exact); then the accumulated mouse delta (100 px = one reach at mouse_sensitivity 1) when the cursor is hidden.
	APlayerController* PC = Cast<APlayerController>(Human->GetController());
	EnsureMatchInputMode(PC);
	const TCHAR* Src = TEXT("none");
	FVector2D Aim = Pi->GetStickAxis();
	FVector2D StickRink = FVector2D::ZeroVector;
	if (Aim.SizeSquared() >= 0.01)
	{
		StickRink = Fwd * Aim.Y + Right * Aim.X;
		Src = TEXT("pad");
	}
	else if (CursorAimRink(PC, Sim->GetSkaters()[Sim->GetHumanIndex()].Pos, StickRink))
	{
		Src = TEXT("mouse");
		Pi->ResetMouseAim();
	}
	else
	{
		Aim = Pi->GetMouseAimPx() * (0.01 * Sim->GetConfig().Stick.MouseSensitivity);
		if (Aim.SizeSquared() > 1.0) Aim.Normalize();
		if (Aim.SizeSquared() >= 0.0025) { StickRink = Fwd * Aim.Y + Right * Aim.X; Src = TEXT("mousedelta"); }
	}
	In.Stick = StickRink;
	In.bPoke = Pi->ConsumePokePressed();
	In.bLift = Pi->ConsumeLiftPressed();
	In.bDeke = Pi->ConsumeDekePressed();   // lane A39 (R-0029 point 4): IA_Deke, Q / gamepad Y
	In.bSpin = Pi->ConsumeSpinPressed();   // lane A51 (R-0034): IA_Spin, E / gamepad B
	// Lane A49 proof lines: the stick every frame when jam.ctrl.Log is on; every button edge always (a few per minute).
	const int64 Frame = Sim->GetFrame();
	if (UJamInputMap::LogEnabled())
	{
		UE_LOG(LogHokej, Log, TEXT("JAMCTRL frame=%lld stick=(%.3f,%.3f) src=%s move=(%.2f,%.2f) shoot=%d sprint=%d brake=%d"), Frame, In.Stick.X, In.Stick.Y, Src,
			In.Move.X, In.Move.Y, In.bShootHeld ? 1 : 0, In.bSprint ? 1 : 0, In.bBrake ? 1 : 0);
	}
	if (In.bCheck) UE_LOG(LogHokej, Log, TEXT("JAMCTRL frame=%lld btn=check (right mouse / RB)"), Frame);
	if (In.bPoke) UE_LOG(LogHokej, Log, TEXT("JAMCTRL frame=%lld btn=poke (space / A)"), Frame);
	if (In.bLift) UE_LOG(LogHokej, Log, TEXT("JAMCTRL frame=%lld btn=lift (middle mouse, mouse 4 / LB)"), Frame);
	if (In.bPass) UE_LOG(LogHokej, Log, TEXT("JAMCTRL frame=%lld btn=pass (F / X)"), Frame);
	if (In.bDeke) UE_LOG(LogHokej, Log, TEXT("JAMCTRL frame=%lld btn=deke (wheel, Q / Y)"), Frame);
	if (In.bSpin) UE_LOG(LogHokej, Log, TEXT("JAMCTRL frame=%lld btn=spin (E / B)"), Frame);   // lane A51 (R-0034)
	if (In.bShootHeld != bLastShootHeld) { bLastShootHeld = In.bShootHeld; UE_LOG(LogHokej, Log, TEXT("JAMCTRL frame=%lld btn=shoot held=%d (left mouse / RT)"), Frame, In.bShootHeld ? 1 : 0); }
	if (In.bSprint != bLastSprint) { bLastSprint = In.bSprint; UE_LOG(LogHokej, Log, TEXT("JAMCTRL frame=%lld btn=sprint held=%d (shift / LT)"), Frame, In.bSprint ? 1 : 0); }
	if (In.bBrake != bLastBrake) { bLastBrake = In.bBrake; UE_LOG(LogHokej, Log, TEXT("JAMCTRL frame=%lld btn=brake held=%d (ctrl / B)"), Frame, In.bBrake ? 1 : 0); }
	return In;
}

// Lane A51 (R-0034): the 1v1 stick-handling drill. The real input path stays in charge of the stick and the keys (the SendInput harness sweeps the cursor and
// presses E); the skating is scripted so the carrier meets a defender: carry towards the nearest opponent, chase the puck when it is loose.
FJamInput AHokejGameMode::StickDrillInput()
{
	FJamInput In = ReadHumanInput();
	if (Sim->GetPhase() != EJamSimPhase::Playing) return In;
	const int32 Me = Sim->GetHumanIndex();
	const FJamSkaterState& S = Sim->GetSkaters()[Me];
	const int32 C = Sim->CarrierIndex();
	if (C == Me)
	{
		const int32 O = Sim->NearestOpponent(S);
		FVector2D To = O >= 0 ? Sim->GetSkaters()[O].Pos - S.Pos : FVector2D(Sim->AttackGoalX(S.Team), 0.0) - S.Pos;
		const double D = To.Size();
		// Skate at the defender at half speed until the bodies are 1.6 m apart, then hold the line (the sweep and the spin do the rest).
		In.Move = D > 1.6 ? To.GetSafeNormal() * 0.6 : FVector2D::ZeroVector;
		return In;
	}
	const FVector2D ToPuck = Sim->GetPuck().Pos - S.Pos;
	In.Move = ToPuck.Size() > 0.3 ? ToPuck.GetSafeNormal() : FVector2D::ZeroVector;
	return In;
}

void AHokejGameMode::StickDrillObserve()
{
	const int32 Me = Sim->GetHumanIndex();
	const FJamSkaterState& S = Sim->GetSkaters()[Me];
	if (StickDrillFrame < 0)
	{
		if (!(S.bSweepThisFrame || S.bSpinThisFrame)) return;
		StickDrillFrame = Sim->GetFrame();
		StickDrillKind = S.bSpinThisFrame ? 2 : 1;
		bStickDrillPokedThis = bStickDrillKnockThis = false;
		StickDrillKnockFramesThis = 0;
		int32 Opp = -1;
		double Dist = -1.0;
		if ((Opp = Sim->NearestOpponent(S)) >= 0) Dist = FVector2D::Distance(Sim->GetSkaters()[Opp].Pos, S.Pos);
		UE_LOG(LogHokej, Log, TEXT("JAMDRILL stick %s frame=%lld n=%d defender=%d dist=%.2f carrying=%d stamina=%.2f"), StickDrillKind == 2 ? TEXT("spin") : TEXT("sweep"), StickDrillFrame,
			StickDrillAttempts + 1, Opp, Dist, Sim->CarrierIndex() == Me ? 1 : 0, S.Stamina < 0 ? 1.0 : static_cast<double>(S.Stamina) / Sim->StaminaFull());
		return;
	}
	for (const FJamSimEvent& E : Sim->GetFrameEvents())
	{
		if (E.Name == JamEvents::Poke && E.Who != Me && Sim->GetSkaters().IsValidIndex(E.Who) && Sim->GetSkaters()[E.Who].Team != S.Team && E.Kind == FName(TEXT("clean"))) bStickDrillPokedThis = true;
		if (E.Name == JamEvents::Knockdown && E.Who == Me) { bStickDrillKnockThis = true; StickDrillKnockFramesThis = S.KnockFrames; }
	}
	if (Sim->GetFrame() - StickDrillFrame < 30) return;
	++StickDrillAttempts;
	const bool bKept = Sim->CarrierIndex() == Me;
	StickDrillKept += bKept ? 1 : 0;
	StickDrillPoked += bStickDrillPokedThis ? 1 : 0;
	StickDrillKnocked += bStickDrillKnockThis ? 1 : 0;
	StickDrillKnockFrames = FMath::Max(StickDrillKnockFrames, StickDrillKnockFramesThis);
	UE_LOG(LogHokej, Log, TEXT("JAMDRILL stick_result %s n=%d kept=%d poked_clean=%d knocked=%d knock_frames=%d"), StickDrillKind == 2 ? TEXT("spin") : TEXT("sweep"), StickDrillAttempts,
		bKept ? 1 : 0, bStickDrillPokedThis ? 1 : 0, bStickDrillKnockThis ? 1 : 0, StickDrillKnockFramesThis);
	StickDrillFrame = -1;
	if (StickDrillAttempts >= 10)
	{
		UE_LOG(LogHokej, Log, TEXT("JAMDRILL stick_summary mode=%d attempts=%d kept=%d poked_clean=%d knocked=%d max_knock_frames=%d"), CVarJamCtrlDrill.GetValueOnGameThread(),
			StickDrillAttempts, StickDrillKept, StickDrillPoked, StickDrillKnocked, StickDrillKnockFrames);
		StickDrillAttempts = StickDrillKept = StickDrillPoked = StickDrillKnocked = StickDrillKnockFrames = 0;
	}
}

FJamInput AHokejGameMode::DrillInput()
{
	// Mirrors what the bindings produce: WASD as a unit Move toward the carrier, one frame of bCheck (right mouse) or bLift (middle mouse) when in range.
	const int32 Mode = CVarJamCtrlDrill.GetValueOnGameThread();
	if (Mode == 3 || Mode == 4) return StickDrillInput();   // lane A51 (R-0034): the 1v1 stick-handling drill keeps the real cursor and keys
	const int32 Me = Sim->GetHumanIndex();
	const FJamSkaterState& S = Sim->GetSkaters()[Me];
	FJamInput In;
	if (Sim->GetPhase() != EJamSimPhase::Playing) return In;
	const int32 C = Sim->CarrierIndex();
	if (C == Me)
	{
		// The drill took the puck itself: give it away (a tap shot up the ice) so a bot carries again.
		In.Move = S.Forward();
		if (Sim->GetFrame() % 30 == 0) In.bShoot = true;
		return In;
	}
	if (C < 0 || Sim->GetSkaters()[C].Team == S.Team)
	{
		const FVector2D ToPuck = Sim->GetPuck().Pos - S.Pos;
		In.Move = ToPuck.Size() > 0.5 ? ToPuck.GetSafeNormal() : FVector2D::ZeroVector;
		return In;
	}
	const FJamSkaterState& V = Sim->GetSkaters()[C];
	const FVector2D ToC = V.Pos - S.Pos;
	const double Dist = ToC.Size();
	In.Move = ToC.GetSafeNormal();
	In.Stick = In.Move;   // the blade toward the carrier, as the cursor would be held
	// Press when the bodies are about to meet inside the human's check window (a person's judgement: closing speed times the window), or inside lift range.
	const double Gap = Dist - UJamSim::SkaterRadius * 2.0;
	const double Closing = Dist > 1e-6 ? FVector2D::DotProduct(S.Vel - V.Vel, ToC / Dist) : 0.0;
	const bool bInRange = Mode == 2 ? Gap <= Sim->GetConfig().Stick.LiftRangeM - 0.2
		: (Gap <= 0.25 || (Closing > 0.5 && Gap <= Closing * (UJamSim::HumanCheckWindowFrames - 3) * UJamSim::DT));
	if (DrillPressFrame < 0 && Sim->GetFrame() >= DrillCooldownUntil && bInRange && !V.IsDown() && S.CanAct())
	{
		if (Mode == 2) In.bLift = true; else In.bCheck = true;
		DrillPressFrame = Sim->GetFrame();
		DrillVictim = C;
		bDrillLandedThis = bDrillStrippedThis = bDrillKnockThis = bDrillVisualThis = bDrillLiftThis = false;
		UE_LOG(LogHokej, Log, TEXT("JAMDRILL %s press frame=%lld dist=%.2f closing=%.2f my_speed=%.2f victim=%d"), Mode == 2 ? TEXT("lift") : TEXT("tackle"), Sim->GetFrame(), Dist, Closing, S.Vel.Size(), C);
	}
	return In;
}

void AHokejGameMode::DrillObserve()
{
	// After the sim stepped: read its own events and state for the press under way; close the attempt after 20 frames.
	if (DrillPressFrame < 0) return;
	const int32 Me = Sim->GetHumanIndex();
	for (const FJamSimEvent& E : Sim->GetFrameEvents())
	{
		if (E.Name == JamEvents::Check && E.Who == Me) bDrillLandedThis = true;
		if (E.Name == JamEvents::Knockdown && E.Who == DrillVictim) bDrillKnockThis = true;
		if (E.Name == JamEvents::Lift && E.Who == Me && E.Strength > 0.5f) bDrillLiftThis = true;
	}
	if (Sim->GetPuck().LastTouch != DrillVictim) bDrillStrippedThis = true;
	if (Skaters.IsValidIndex(DrillVictim) && Sim->GetSkaters()[DrillVictim].IsDown()) bDrillVisualThis = true;   // ApplySimFrame sent SetKnockedDown(true) on that edge
	if (Sim->GetFrame() - DrillPressFrame < 20) return;
	++DrillAttempts;
	DrillLanded += bDrillLandedThis ? 1 : 0;
	DrillStripped += bDrillStrippedThis ? 1 : 0;
	DrillKnockdowns += bDrillKnockThis ? 1 : 0;
	DrillVisualDown += bDrillVisualThis ? 1 : 0;
	DrillLifted += bDrillLiftThis ? 1 : 0;
	const int32 Mode = CVarJamCtrlDrill.GetValueOnGameThread();
	UE_LOG(LogHokej, Log, TEXT("JAMDRILL %s attempt=%d landed=%d stripped=%d knockdown=%d victim_visual_down=%d lift_clean=%d"), Mode == 2 ? TEXT("lift") : TEXT("tackle"),
		DrillAttempts, bDrillLandedThis ? 1 : 0, bDrillStrippedThis ? 1 : 0, bDrillKnockThis ? 1 : 0, bDrillVisualThis ? 1 : 0, bDrillLiftThis ? 1 : 0);
	DrillPressFrame = -1;
	DrillCooldownUntil = Sim->GetFrame() + 120;
	if (DrillAttempts >= 10)
	{
		UE_LOG(LogHokej, Log, TEXT("JAMDRILL summary mode=%d attempts=%d landed=%d stripped=%d knockdowns=%d victim_visual_down=%d lift_clean=%d"), Mode, DrillAttempts,
			DrillLanded, DrillStripped, DrillKnockdowns, DrillVisualDown, DrillLifted);
		CVarJamCtrlDrill->Set(0, ECVF_SetByCode);
		DrillAttempts = DrillLanded = DrillStripped = DrillKnockdowns = DrillVisualDown = DrillLifted = 0;
	}
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
	// lane A3 (R-0017): the pickups on the ice (only all_star bots detour for them); BuildWorld already reads them from the sim's
	// effect system, this refills from the game mode's own one in case they differ
	if (Effects)
	{
		W.NumPickups = 0;
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
	// The goalie stance send (lane A31, R-0033 point 2 / lane A50 for every skater). The brain's crease rule flickers at its edge (on/off every 3 frames
	// in the 19:58 proof): once on, the stance holds at least GoalieHoldFrames before it may drop (cosmetic only; the brain's steering is untouched).
	// JAMGOALIE names the cartridge role, so a bot match log shows the stance on more than the back.
	auto RoleName = [](JamAI::ERole R) -> const TCHAR*
	{
		switch (R)
		{
		case JamAI::ERole::Player: return TEXT("player");
		case JamAI::ERole::Back: return TEXT("back");
		case JamAI::ERole::Support: return TEXT("support");
		default: return TEXT("chaser");
		}
	};
	auto SendGoalieStance = [&](int32 i, bool bOn, int32 Side, const TCHAR* RoleTag)
	{
		int8 Goalie = bOn ? static_cast<int8>(FMath::Clamp(Side, -1, 1)) : GoalieOff;
		if (Goalie == GoalieOff && LastGoalie.IsValidIndex(i) && LastGoalie[i] != GoalieOff && Sim->GetFrame() - GoalieOnFrame[i] < GoalieHoldFrames) Goalie = LastGoalie[i];
		if (LastGoalie.IsValidIndex(i) && LastGoalie[i] != Goalie)
		{
			if (LastGoalie[i] == GoalieOff) GoalieOnFrame[i] = Sim->GetFrame();
			LastGoalie[i] = Goalie;
			const bool bSend = Goalie != GoalieOff;
			if (Skaters.IsValidIndex(i) && Skaters[i]) if (UObject* V = Skaters[i]->GetVisualObject()) IJamSkaterVisual::Execute_SetGoalieStance(V, bSend, bSend ? Goalie : 0);
			UE_LOG(LogHokej, Log, TEXT("JAMGOALIE frame=%lld skater=%d on=%d side=%d role=%s"), Sim->GetFrame(), i, bSend ? 1 : 0, bSend ? Goalie : 0, RoleTag);   // on a change only (B-to-A-0024 point 3 proof)
		}
	};
	for (int32 i = 0; i < Inputs.Num(); ++i)
	{
		if (i == Human) continue;
		FJamInput& I = Inputs[i];
		if (i < JamAI::kMaxSkaters)
		{
			I.Move = FVector2D(Out[i].Move.X, Out[i].Move.Y);
			I.bShoot = Out[i].bShoot;
			I.bShootHeld = Out[i].bShootHeld;   // lane A31 (B-to-A-0024 point 1): the bots' charged shot
			I.bCheck = Out[i].bCheck;
			I.bBrake = Out[i].bBrake;
			I.bPass = Out[i].bPass;   // lane A3 (R-0017): pass, stick aim, poke, lift
			I.Stick = FVector2D(Out[i].Stick.X, Out[i].Stick.Y);
			I.bPoke = Out[i].bPoke;
			I.bLift = Out[i].bLift;
			I.bDeke = Out[i].bDeke;   // lane A39: the bots' deke
			// Lane A31 (B-to-A-0024 point 3, C-0001 v3): the brain's owner calls SetGoalieStance for the skater covering the crease, on a change only.
			// R-0033 point 2 (lane A50): any crease skater, not only the back (JamAI::CreaseStance after Decide); the human below through the same send.
			SendGoalieStance(i, Out[i].bGoalie, Out[i].GoalieSide, RoleName(W.Skaters[i].Role));
		}
	}
	// R-0033 point 2 (lane A50): the human standing in its crease without the puck while an opponent carries inside the blue line takes the stance too: the
	// bots' rule on the world the brain just read (the human's own skater is in it). The hold and the change-only send are shared with the bots'.
	if (Human >= 0 && Human < JamAI::kMaxSkaters && Human < W.NumSkaters)
	{
		int Side = 0;
		const bool bOn = JamAI::CreaseStance(W, Human, Side);
		SendGoalieStance(Human, bOn, Side, TEXT("player"));
	}
	if (CVarJamCtrlDrill.GetValueOnGameThread() != 0) Inputs[Human] = DrillInput();   // lane A49 proof drill
	else Inputs[Human] = (bScriptedHuman || CVarJamScriptHuman.GetValueOnGameThread() != 0) ? ScriptedHumanInput() : ReadHumanInput();
	if (Skaters.IsValidIndex(Human) && Skaters[Human]) Skaters[Human]->SetLogAim(UJamInputMap::LogEnabled());
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
		Pl.Dir = FVector(E.Dir.X, E.Dir.Y, 0.0);   // lane A47 (B-to-A-0029): rink (x, z) -> world (x, y), the same axes as ToWorld
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
	F.CornerRadiusM = Sim->GetConfig().Rink.CornerRadiusM;  // A40: the camera clamp follows the corner arcs
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
	if (Sim->GetSkaters().IsValidIndex(F.HumanIndex))
	{
		const FJamSkaterState& H = Sim->GetSkaters()[F.HumanIndex];
		F.HumanStamina01 = H.Stamina < 0 ? 1.f : FMath::Clamp((float)H.Stamina / (float)FMath::Max(1, Sim->StaminaFull()), 0.f, 1.f);   // lane A49
		F.bHumanSprinting = H.bSprinting;
	}
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
	if (CVarJamCtrlDrill.GetValueOnGameThread() == 3 || CVarJamCtrlDrill.GetValueOnGameThread() == 4 || StickDrillFrame >= 0) StickDrillObserve();   // lane A51
	else if (CVarJamCtrlDrill.GetValueOnGameThread() != 0 || DrillPressFrame >= 0) DrillObserve();   // lane A49
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
		const FJamSkaterState& St = Sim->GetSkaters()[A->GetSimIndex()];
		// Lane A31 (B-to-A-0024 points 1 and 2): one line per shot (with its charge) and per pass, a few per minute, so B can read what the visual was sent.
		if (St.bShotThisFrame) UE_LOG(LogHokej, Log, TEXT("JAMSHOT frame=%lld skater=%d team=%d charge=%.2f %s"), Sim->GetFrame(), St.Index, St.Team, St.LastShotCharge, St.bHuman ? TEXT("human") : TEXT("bot"));
		if (St.bPassThisFrame) UE_LOG(LogHokej, Log, TEXT("JAMPASS frame=%lld skater=%d team=%d strength=%.2f %s (PlayPass only, no PlayShoot)"), Sim->GetFrame(), St.Index, St.Team, St.LastPassStrength, St.bHuman ? TEXT("human") : TEXT("bot"));
		// Lane A51 (R-0034) proof lines: the human's sweep (the blade crossed fast with the puck: the shield opens) and spin, as the sim saw them.
		if (St.bHuman && St.bSweepThisFrame) UE_LOG(LogHokej, Log, TEXT("JAMCTRL frame=%lld sweep side=%d shield_frames=%d blade=(%.2f,%.2f)"), Sim->GetFrame(), St.SweepSide, St.SweepFrames, St.BladePos.X, St.BladePos.Y);
		if (St.bHuman && St.bSpinThisFrame) UE_LOG(LogHokej, Log, TEXT("JAMCTRL frame=%lld spin frames=%d dir=%d stamina=%.2f carrying=%d"), Sim->GetFrame(), St.SpinFrames, St.SpinDir, St.Stamina < 0 ? 1.0 : static_cast<double>(St.Stamina) / Sim->StaminaFull(), Sim->CarrierIndex() == St.Index ? 1 : 0);
		A->ApplySimFrame(St);
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
