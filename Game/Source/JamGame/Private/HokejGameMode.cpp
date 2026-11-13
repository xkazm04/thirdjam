#include "HokejGameMode.h"
#include "JamCartridge.h"
#include "JamEventBus.h"
#include "JamPawns.h"
#include "JamBrain.h"
#include "JamSimAdapter.h"
#include "JamCarryDrill.h"
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
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformMisc.h"
#include "UnrealClient.h"
#include "Engine/LevelStreaming.h"
#include "Misc/App.h"
#include "Misc/PackageName.h"

DEFINE_LOG_CATEGORY(LogHokej);

// Lane A111 (track U1, the screen; docs/experiments/SCREEN.md): the fade-in that waits for the light rig.
static TAutoConsoleVariable<int32> CVarJamScreenLightHold(TEXT("jam.screen.LightHold"), 2,
	TEXT("A new match holds the picture black until its light rig's level is visible (at most 2 s), then fades in: 2 (default) also finishes the rig's load before the first frame, 1 the hold alone, 0 off (the match shows at once, lit or not)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamScreenStillEvery(TEXT("jam.screen.StillEvery"), 0,
	TEXT("Proof: N > 0 takes a still (u1-<tier>-f<frame>.png, with the HUD) every N sim frames; two runs of one cut and seed give stills of the same frames."), ECVF_Default);
static TAutoConsoleVariable<float> CVarJamScreenFadeSeconds(TEXT("jam.screen.FadeSeconds"), AHokejGameMode::LightFadeSeconds,
	TEXT("Seconds of the fade-in after the light hold (0 = a cut)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarJamScreenHoldStill(TEXT("jam.screen.HoldStill"), 0,
	TEXT("Proof: 1 takes a still (u1-first-<tier>.png, with the HUD) two ticks after the light hold ends, or at the second tick of the match when the hold is off."), ECVF_Default);
// jam.screen.Still <name>: a still of this game's own viewport with the HUD, into the project's screenshot folder (no window grab, no cursor).
static FAutoConsoleCommand GJamScreenStill(TEXT("jam.screen.Still"), TEXT("Take a still with the HUD: jam.screen.Still <name> (written as <name>.png to the screenshot folder)."),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
	{
		const FString Name = Args.Num() ? Args[0] : FString(TEXT("u1-still"));
		FScreenshotRequest::RequestScreenshot(Name + TEXT(".png"), true, false);
		UE_LOG(LogHokej, Log, TEXT("JAMSCREEN still requested: %s.png"), *Name);
	}));

static TAutoConsoleVariable<FString> CVarJamCut(TEXT("jam.Cut"), TEXT("hockey.regular"),
	TEXT("Cartridge the Hokej game mode loads at start (name in Content/Cartridges or a path). The -cut=<name> launch argument wins."));
static TAutoConsoleVariable<int32> CVarJamScriptHuman(TEXT("jam.ScriptHuman"), 0,
	TEXT("1: the human's skater plays itself (skate behind the puck, shoot when lined up). For PIE proofs through MCP."));
// Lane A49: the tackle / cross-check drill (packaged proof of complaint 5). 1 = tackle (right mouse), 2 = cross-check (middle mouse); resets to 0 after 10 attempts.
// Lane A73: the still capture of the tackling proof (see StillsObserve).
static TAutoConsoleVariable<int32> CVarJamCtrlStills(TEXT("jam.ctrl.Stills"), 0,
	TEXT("1 = capture a still every 4 sim frames after a knockdown that involves the human (24), the human's board hit (8) and a stick hit on the human (8); twice per kind. Lane A73 proof. 2 = the mechanics (lane A77): the human's sweep with the puck (150 = 10 s, once), its spin with the puck (24, twice), its saucer (20, twice)."));
// Lane A82: drills 10 and 11 hand the puck by SetPuck (the shots drill to the human, the one-timer drill to the scripted teammate) instead of winning it from the bots.
static TAutoConsoleVariable<int32> CVarJamCtrlHandPuck(TEXT("jam.ctrl.HandPuck"), 0,
	TEXT("1 = drills 10 / 11 hand the puck to the human (10) or the scripted teammate (11) whenever nobody on the human's team carries and no shot is being measured; JAMDRILL hand_puck lines. Lane A82 proof."));
static TAutoConsoleVariable<int32> CVarJamCtrlDrill(TEXT("jam.ctrl.Drill"), 0,
	TEXT("1 = tackle drill, 2 = cross-check drill: the human chases the carrier and presses once in range, 10 attempts, JAMDRILL lines. Lane A49 proof. 3 = sweep drill, 4 = spin drill (lane A51): the cursor and the keys stay real, the skating carries the puck at the nearest defender; JAMDRILL stick lines. 7 = sweep past a defender, 8 = spin with the puck, 9 = saucer over a defender's blade, 10 = shots from 10 m, 11 = one-timers off a scripted teammate (lane A77): JAMCUE lines for the driver, JAMDRILL stick / saucer / shot / onetimer lines. 20 = the acceptance run (lane A87, tools/qa/accept): JAMACCEPT lines. 21 = the carry drill (lane A102): the human carries at a standing defender the brain plays, 20 rounds, JAMDRILL carry lines; jam.ctrl.CarryScript picks who skates."));
// Lane A102 (track F2, carrier skill): drill 21, the packaged face of Jam.Core.Movement.StopAndGo. The scene and the scripted human are FJamCarryDrill
// (JamCore/Public/JamCarryDrill.h), the code the headless test runs: the human carries at 9 m/s at the opposing chaser standing 12 m ahead, which the real brain
// plays; everyone else is parked in a corner. 20 rounds (the round number is the scene's seed), a JAMDRILL carry result line each and a summary; then the
// drill switches itself off. State lives here (one game mode at a time, as the hand-puck drill's statics below).
static TAutoConsoleVariable<int32> CVarJamCtrlCarryScript(TEXT("jam.ctrl.CarryScript"), 1,
	TEXT("Drill 21's human: 1 = the scripted stop-and-go (the stop begins at 8 m (lane A128; 9.5 m before), cut 60 degrees, burst; the cursor on the far side of the body), 0 = the scripted straight charge, 2 = your own keys and mouse (the drill only stages and scores). Lane A102. 3 = the late answer: the spin with the cut at 6 m, no stop; 4 = the deke tap with the cut at 6 m (lane A121)."));
static TAutoConsoleVariable<float> CVarJamCtrlCarryBrakeAtM(TEXT("jam.ctrl.CarryBrakeAtM"), 0.f,
	TEXT("Drill 21: the gap to the defender, in metres, at which the scripted human begins its stop (script 1) or its cut (scripts 3 and 4). 0 = the script's own (8 m; 6 m for 3 and 4). Lane A121."));
namespace
{
	FJamCarryDrill GCarryDrill;
	int64 GCarryNextFrame = 0;
	int32 GCarryRounds = 0, GCarryScript = 1;
	int32 GCarryEnds[FJamCarryDrill::EndCount] = {};
	constexpr int32 CarryDrillRounds = 20;
	const TCHAR* CarryScriptName(int32 Script) { return Script == 0 ? TEXT("straight") : Script == 2 ? TEXT("hand") : Script == 3 ? TEXT("spin_cut") : Script == 4 ? TEXT("deke_cut") : TEXT("stop_and_go"); }

	// In GatherInputs, after the brain's and the human's inputs: stages a round when none runs, parks everyone but the two, scripts the human.
	void CarryDrillGather(UJamSim& Sim, TArray<FJamInput>& Inputs)
	{
		if (Sim.GetPhase() != EJamSimPhase::Playing) return;
		if (!GCarryDrill.IsLive())
		{
			if (Sim.GetFrame() < GCarryNextFrame) return;
			// A round starts only with the human on its feet (the last round may have left it down or stumbling: the headless run lost 1 round of 20 to that).
			if (!Sim.GetSkaters().IsValidIndex(Sim.GetHumanIndex()) || !Sim.GetSkaters()[Sim.GetHumanIndex()].CanAct()) return;
			GCarryScript = CVarJamCtrlCarryScript.GetValueOnGameThread();
			GCarryDrill = GCarryScript == 3 ? FJamCarryDrill::Late(1) : GCarryScript == 4 ? FJamCarryDrill::Late(2) : FJamCarryDrill();   // lane A121: the late answers
			GCarryDrill.bStopAndGo = GCarryScript != 0;
			if (const float AtM = CVarJamCtrlCarryBrakeAtM.GetValueOnGameThread(); AtM > 0.f) GCarryDrill.BrakeAtM = AtM;
			if (!GCarryDrill.Stage(Sim, static_cast<uint64>(GCarryRounds + 1)))
			{
				UE_LOG(LogHokej, Warning, TEXT("JAMDRILL carry: no human or no opponent in this cut; drill off"));
				CVarJamCtrlDrill->Set(0, ECVF_SetByCode);
				return;
			}
			UE_LOG(LogHokej, Log, TEXT("JAMDRILL carry stage n=%d script=%s frame=%lld human=%d defender=%d body_shield=%d brake_at=%.2f take_grace=%d reverse_brake=%.0f own_stop=%d rest_frames=%d spin_exposed=%d"), GCarryRounds + 1, CarryScriptName(GCarryScript), Sim.GetFrame(),
				GCarryDrill.Me, GCarryDrill.Def, Sim.GetConfig().Stick.bBodyShield ? 1 : 0, GCarryDrill.BrakeAtM, Sim.GetConfig().Stick.TakeGraceFrames, Sim.GetConfig().Skater.ReverseBrakeDecel, Sim.GetConfig().Skater.bBurstOwnStop ? 1 : 0,
				Sim.GetConfig().Skater.BurstRestFrames, Sim.GetConfig().Skater.SpinExposedTail);
		}
		if (!Inputs.IsValidIndex(GCarryDrill.Me) || !Inputs.IsValidIndex(GCarryDrill.Def)) return;
		for (int32 i = 0; i < Inputs.Num(); ++i) if (GCarryDrill.IsParked(i)) Inputs[i] = FJamInput();
		if (GCarryDrill.DefenderWaits()) Inputs[GCarryDrill.Def] = FJamInput();
		if (GCarryScript != 2) Inputs[GCarryDrill.Me] = GCarryDrill.HumanInput(Sim);
		GCarryDrill.BeforeStep(Sim, Inputs[GCarryDrill.Def]);
	}

	// In StepOnce, after the sim's step: the round's end, its line, the summary after the last round.
	void CarryDrillObserve(UJamSim& Sim)
	{
		if (!GCarryDrill.IsLive()) return;
		GCarryDrill.AfterStep(Sim);
		if (GCarryDrill.End == FJamCarryDrill::Running) return;
		++GCarryRounds;
		if (GCarryDrill.End >= 0 && GCarryDrill.End < FJamCarryDrill::EndCount) ++GCarryEnds[GCarryDrill.End];
		UE_LOG(LogHokej, Log, TEXT("JAMDRILL carry result n=%d script=%s end=%s frames=%d min_gap=%.2f brake_frames=%d defender_lift=%d defender_poke=%d defender_check=%d body_refused=%d"),
			GCarryRounds, CarryScriptName(GCarryScript), FJamCarryDrill::EndName(GCarryDrill.End), GCarryDrill.Frames, GCarryDrill.MinGapM, GCarryDrill.BrakeFrames,
			GCarryDrill.DefLifts, GCarryDrill.DefPokes, GCarryDrill.DefChecks, GCarryDrill.BodyShieldRefused);
		GCarryNextFrame = Sim.GetFrame() + 90;
		if (GCarryRounds >= CarryDrillRounds)
		{
			UE_LOG(LogHokej, Log, TEXT("JAMDRILL carry summary script=%s rounds=%d past=%d poke=%d lift=%d down=%d stumble=%d touch=%d timeout=%d setup=%d"), CarryScriptName(GCarryScript), GCarryRounds,
				GCarryEnds[FJamCarryDrill::Past], GCarryEnds[FJamCarryDrill::Poke], GCarryEnds[FJamCarryDrill::Lift], GCarryEnds[FJamCarryDrill::Down], GCarryEnds[FJamCarryDrill::Stumble],
				GCarryEnds[FJamCarryDrill::Touch], GCarryEnds[FJamCarryDrill::Timeout], GCarryEnds[FJamCarryDrill::Setup]);
			GCarryRounds = 0;
			for (int32& E : GCarryEnds) E = 0;
			GCarryNextFrame = 0;
			CVarJamCtrlDrill->Set(0, ECVF_SetByCode);
		}
	}
}
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
	// Lane A64: why the last LoadCut failed (path and validator lines), and the toast the HUD shows once it exists. Presentation only.
	FString GLastCutPath;
	TArray<FString> GLastCutErrors;
	FString PendingCutToast;
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
		// Lane A64 (A54's premise 2: a rejected cut was invisible in Shipping, where UE_LOG is compiled out). One line in every configuration:
		// the debugger stream, stderr and <Saved>/Logs/JamCut.log (appended), and a 4 s HUD toast once the HUD exists.
		const FString Reason = GLastCutErrors.Num() ? FString::Join(GLastCutErrors, TEXT("; ")) : FString(TEXT("could not be read"));
		const FString Line = FString::Printf(TEXT("JAMCUT rejected %s: %s (falling back to hockey.regular)"), *GLastCutPath, *Reason);
		FPlatformMisc::LowLevelOutputDebugStringf(TEXT("%s\n"), *Line);
		fprintf(stderr, "%s\n", TCHAR_TO_UTF8(*Line));
		FFileHelper::SaveStringToFile(FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S ")) + Line + LINE_TERMINATOR, *(FPaths::ProjectLogDir() / TEXT("JamCut.log")),
			FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM, &IFileManager::Get(), FILEWRITE_Append);
		UE_LOG(LogHokej, Error, TEXT("%s"), *Line);
		PendingCutToast = FString::Printf(TEXT("CUT REJECTED: %s\n%s\nPlaying hockey.regular"), *FPaths::GetCleanFilename(GLastCutPath), *Reason.Left(160));
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
		GLastCutPath = Path.IsEmpty() ? NameOrPath : Path;
		GLastCutErrors = Errors;
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
	BeginLightHold();   // lane A111: the picture waits for the light rig
}

// Lane A111: true when the rig of the cartridge's tier is a visible streamed level of this world (or the tier has no rig: nothing to wait for).
bool AHokejGameMode::IsLightRigVisible() const
{
	const UWorld* World = GetWorld();
	if (!World || !Cartridge) return true;
	FString Pkg = ULookTierSwitcher::GetRigMapPath(Cartridge->Game.Look.Tier);
	if (Pkg.IsEmpty()) return true;
	int32 Dot = INDEX_NONE;
	if (Pkg.FindChar(TEXT('.'), Dot)) Pkg.LeftInline(Dot);
	const FString Short = FPackageName::GetShortName(Pkg);   // "LR_P3_Clay"; a streamed instance is named "<path>/LR_P3_Clay_LevelInstance_<n>" (PIE adds a prefix)
	for (const ULevelStreaming* SL : World->GetStreamingLevels())
	{
		if (!SL || SL->GetIsRequestingUnloadAndRemoval() || !SL->ShouldBeVisible()) continue;
		if (SL->GetWorldAssetPackageName().Contains(Short) && SL->GetLoadedLevel() && SL->IsLevelVisible()) return true;
	}
	return false;
}

void AHokejGameMode::BeginLightHold()
{
	bLightHold = false;
	LightStillIn = -1;
	const int32 Mode = CVarJamScreenLightHold.GetValueOnGameThread();
	const bool bOn = Mode != 0;
	LastStillBucket = -1;
	if (!FApp::CanEverRender() || IsRunningDedicatedServer()) return;   // headless: no picture to hold
	if (!bOn || IsLightRigVisible())
	{
		if (CVarJamScreenHoldStill.GetValueOnGameThread() != 0) LightStillIn = 2;
		UE_LOG(LogHokej, Log, TEXT("JAMSCREEN light hold not started (%s)"), bOn ? TEXT("the rig is already visible") : TEXT("jam.screen.LightHold 0"));
		return;
	}
	bLightHold = true;
	LightHoldStart = FPlatformTime::Seconds();
	LightHoldTicks = LightVisibleTicks = 0;
	double FlushMs = -1.0;
	if (Mode >= 2)
	{
		// The rig's level, asked for a moment ago by ULookTierSwitcher::SetTier, is loaded and made visible now instead of over the next seconds of frames.
		const double T0 = FPlatformTime::Seconds();
		GetWorld()->FlushLevelStreaming(EFlushLevelStreamingType::Full);
		FlushMs = (FPlatformTime::Seconds() - T0) * 1000.0;
	}
	UE_LOG(LogHokej, Log, TEXT("JAMSCREEN light hold started: tier %s, rig not visible yet (bounded at %.1f s); blocking load %.0f ms (-1 = none), rig visible after it=%d"), *Cartridge->Game.Look.Tier,
		LightHoldMaxSeconds, FlushMs, IsLightRigVisible() ? 1 : 0);
	TickLightHold();
}

void AHokejGameMode::TickLightHold()
{
	UWorld* World = GetWorld();
	if (LightStillIn >= 0 && !bLightHold)
	{
		if (LightStillIn-- == 0 && Cartridge)
		{
			const FString Name = FString::Printf(TEXT("u1-first-%s.png"), *Cartridge->Game.Look.Tier);
			FScreenshotRequest::RequestScreenshot(Name, true, false);
			UE_LOG(LogHokej, Log, TEXT("JAMSCREEN first visible frame still requested: %s (sim frame %lld, rig visible=%d)"), *Name, Sim ? Sim->GetFrame() : -1, IsLightRigVisible() ? 1 : 0);
		}
	}
	if (const int32 Every = CVarJamScreenStillEvery.GetValueOnGameThread(); Every > 0 && Sim && Cartridge && !bLightHold)
	{
		const int64 Bucket = Sim->GetFrame() / Every;
		if (Bucket != LastStillBucket && Bucket > 0)
		{
			LastStillBucket = Bucket;
			FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("u1-%s-f%05lld.png"), *Cartridge->Game.Look.Tier, Bucket * Every), true, false);
		}
	}
	if (!bLightHold || !World) return;
	APlayerController* PC = World->GetFirstPlayerController();
	APlayerCameraManager* PCM = PC ? PC->PlayerCameraManager.Get() : nullptr;
	const bool bVisible = IsLightRigVisible();
	LightVisibleTicks = bVisible ? LightVisibleTicks + 1 : 0;
	const double Held = FPlatformTime::Seconds() - LightHoldStart;
	if (!LightHoldDone(bVisible, LightVisibleTicks, Held))
	{
		++LightHoldTicks;
		if (PCM) PCM->SetManualCameraFade(1.f, FLinearColor::Black, false);
		if (Hud) Hud->SetRenderOpacity(0.f);
		return;
	}
	bLightHold = false;
	const float Fade = FMath::Max(0.f, CVarJamScreenFadeSeconds.GetValueOnGameThread());
	if (PCM)
	{
		if (Fade > 0.f) PCM->StartCameraFade(1.f, 0.f, Fade, FLinearColor::Black, false, false);
		else PCM->StopCameraFade();
	}
	if (Hud) Hud->SetRenderOpacity(1.f);
	if (CVarJamScreenHoldStill.GetValueOnGameThread() != 0) LightStillIn = 2;
	UE_LOG(LogHokej, Log, TEXT("JAMSCREEN light hold released: held %.2f s over %d ticks, rig visible=%d (%s), sim frame %lld, fade %.2f s"), Held, LightHoldTicks, bVisible ? 1 : 0,
		bVisible ? TEXT("the rig reported loaded") : TEXT("the 2 s bound"), Sim ? Sim->GetFrame() : -1, Fade);
}

AJamSkater* AHokejGameMode::GetHumanSkater() const
{
	return (Sim && Skaters.IsValidIndex(Sim->GetHumanIndex())) ? Skaters[Sim->GetHumanIndex()].Get() : nullptr;
}

int32 AHokejGameMode::GetLocalSeat() const
{
	return Sim ? Sim->GetHumanIndex() : -1;
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
	if (Hud && !PendingCutToast.IsEmpty()) { Hud->ShowToast(PendingCutToast); PendingCutToast.Reset(); }   // lane A64
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
	if (!PC->DeprojectScreenPositionToWorld(MX, MY, O, D)) return false;
	return CursorAimFromRay(O, D, SkaterPosM, bCursorAiming, OutStick);
}

bool AHokejGameMode::CursorAimFromRay(const FVector& O, const FVector& D, const FVector2D& SkaterPosM, bool& bInOutAiming, FVector2D& OutStick)
{
	OutStick = FVector2D::ZeroVector;
	if (D.Z >= -1e-4) return false;
	const FVector Hit = O + D * (-O.Z / D.Z);   // the ice is z = 0 (UJamSim::ToWorld)
	const FVector2D Delta = FVector2D(Hit.X / 100.0, Hit.Y / 100.0) - SkaterPosM;
	const double Dist = Delta.Size();
	const double Enter = bInOutAiming ? 0.5 : 0.7;   // hysteresis: a cursor parked on the skater holds the blade ahead without flapping at the edge
	if (Dist < Enter) { bInOutAiming = false; return true; }
	bInOutAiming = true;
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
	const bool bHave = Dist < Sim->CarrierReachM(S.Index) && Dist > 1e-6 && FVector2D::DotProduct(S.Forward(), ToPuck / Dist) > 0.6;
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
	// Lane A93 (R-0045 law 3): braking by reversing (W then S). The third-person camera follows the skater's facing 4 frames late, so "back" read from the
	// live camera turns with the skater: a held S never points at one place (the skater circled; with the reverse brake it would stop, turn and never go).
	// While the move input pulls back, it is read in the camera's frame of the moment the pull began: S is one direction on the ice until it is let go, the
	// skater brakes, turns and skates back that way, and the camera comes round behind it. The stick aim below keeps the live camera.
	double MoveYaw = Yaw;
	if (Axis.Y < -0.3)
	{
		if (!bBackYawHeld) { bBackYawHeld = true; BackYawRad = Yaw; }
		MoveYaw = BackYawRad;
	}
	else bBackYawHeld = false;
	const FVector2D MoveFwd(FMath::Cos(MoveYaw), FMath::Sin(MoveYaw));
	const FVector2D MoveRight(-FMath::Sin(MoveYaw), FMath::Cos(MoveYaw));
	const FVector2D Fwd(FMath::Cos(Yaw), FMath::Sin(Yaw));
	const FVector2D Right(-FMath::Sin(Yaw), FMath::Cos(Yaw));
	In.Move = MoveFwd * Axis.Y + MoveRight * Axis.X;
	// C-0005 charged shot: the human sends the held level; the sim charges while the puck is in reach and fires on release (a tap = a short charge).
	Pi->ConsumeShootPressed();
	In.bShootHeld = Pi->IsShootHeld();
	// Lane A77 (R-0036): the pass is a held level like the shot: the sim's StepPassCharge fires it on the release (a tap flat, a hold of pass.saucer.half_frames a saucer).
	Pi->ConsumePassPressed();
	In.bPassHeld = Pi->IsPassHeld();
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
	In.bDeke = Pi->ConsumeDekePressed();   // lane A39 (R-0029 point 4): IA_Deke, wheel / gamepad Y (lane A84: Q went to the pass)
	In.bSpin = Pi->ConsumeSpinPressed();   // lane A51 (R-0034): IA_Spin, E / gamepad B
	In.bAbility = Pi->ConsumeAbilityPressed();   // lane A84 (R-0041 point 3): IA_Ability, F / gamepad R3
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
	// Lane A85: the pass's release as the sim will see it this step (A82's JAMSHOT release for the pass): the hold's charge, the cooldown, the puck against the
	// carrier's reach, and whom a flat pass and a saucer would go to. A hold that fires nothing says why here.
	if (!In.bPassHeld && bLastPassHeld && Sim->GetSkaters().IsValidIndex(Sim->GetHumanIndex()))
	{
		const FJamSkaterState& Hp = Sim->GetSkaters()[Sim->GetHumanIndex()];
		FVector2D AimF, AimS;
		UE_LOG(LogHokej, Log, TEXT("JAMPASS release frame=%lld pass_charge_frames=%d cooldown_left=%lld puck_dist=%.2f reach=%.2f carrier=%d flat_receiver=%d saucer_receiver=%d"), Frame, Hp.PassChargeFrames,
			FMath::Max<int64>(0, Hp.ShotReadyFrame - Frame), FVector2D::Distance(Hp.Pos, Sim->GetPuck().Pos), Sim->CarrierReachM(Hp.Index), Sim->CarrierIndex(), Sim->BestPass(Hp.Index, AimF, false), Sim->BestPass(Hp.Index, AimS, true));
	}
	if (In.bPassHeld != bLastPassHeld) { bLastPassHeld = In.bPassHeld; UE_LOG(LogHokej, Log, TEXT("JAMCTRL frame=%lld btn=pass held=%d (Q / X)"), Frame, In.bPassHeld ? 1 : 0); }   // lane A77
	if (In.bDeke) UE_LOG(LogHokej, Log, TEXT("JAMCTRL frame=%lld btn=deke (wheel / Y)"), Frame);
	if (In.bSpin) UE_LOG(LogHokej, Log, TEXT("JAMCTRL frame=%lld btn=spin (E / B)"), Frame);   // lane A51 (R-0034)
	if (In.bAbility)   // lane A84 (R-0041 point 3): the press, with the kind held (none = the press does nothing)
	{
		const FJamAbilityConfig* Ab = Sim->HeldAbility(Sim->GetHumanIndex());
		UE_LOG(LogHokej, Log, TEXT("JAMCTRL frame=%lld btn=ability kind=%s (F / R3)"), Frame, Ab ? *Ab->Kind.ToString() : TEXT("none"));
	}
	if (In.bShootHeld != bLastShootHeld)
	{
		bLastShootHeld = In.bShootHeld;
		UE_LOG(LogHokej, Log, TEXT("JAMCTRL frame=%lld btn=shoot held=%d (left mouse / RT)"), Frame, In.bShootHeld ? 1 : 0);
		// Lane A82: the release frame as the sim will see it this step (A77's two taps fired nothing, unexplained): the charge the hold built, the cooldown, the puck's reach and side.
		if (!In.bShootHeld && Sim->GetSkaters().IsValidIndex(Sim->GetHumanIndex()))
		{
			const FJamSkaterState& H = Sim->GetSkaters()[Sim->GetHumanIndex()];
			const FVector2D ToPuck = Sim->GetPuck().Pos - H.Pos;
			const double Dist = ToPuck.Size();
			UE_LOG(LogHokej, Log, TEXT("JAMSHOT release frame=%lld charge_frames=%d shot_ready_frame=%lld cooldown_left=%lld puck_dist=%.2f reach=%.2f front_dot=%.2f carrier=%d"), Frame, H.ChargeFrames, H.ShotReadyFrame,
				FMath::Max<int64>(0, H.ShotReadyFrame - Frame), Dist, Sim->CarrierReachM(H.Index), Dist > 1e-6 ? FVector2D::DotProduct(H.Forward(), ToPuck / Dist) : 1.0, Sim->CarrierIndex());
		}
	}
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
	// Lane A91 (R-0044 point 3): the sweep and the spin are proven by their shield: inside the window the move opened (stick.sweep.shield_frames, the spin's
	// frames) every opponent's poke and lift near the human is counted: refused (the event's kind miss) or clean on the human (it carried the frame before).
	static int32 ShieldLen = 0, ShieldRefused = 0, ShieldClean = 0;
	static bool bCarriedBefore = false;
	const bool bCarriedPrev = bCarriedBefore;
	bCarriedBefore = Sim->CarrierIndex() == Me;
	if (StickDrillFrame < 0)
	{
		if (!(S.bSweepThisFrame || S.bSpinThisFrame)) return;
		StickDrillFrame = Sim->GetFrame();
		StickDrillKind = S.bSpinThisFrame ? 2 : 1;
		ShieldLen = S.bSpinThisFrame ? S.SpinFrames : S.SweepFrames;
		ShieldRefused = ShieldClean = 0;
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
		if ((E.Name == JamEvents::Poke || E.Name == JamEvents::Lift) && E.Who != Me && Sim->GetSkaters().IsValidIndex(E.Who) && Sim->GetSkaters()[E.Who].Team != S.Team
			&& Sim->GetFrame() - StickDrillFrame <= ShieldLen && FVector2D::Distance(Sim->GetSkaters()[E.Who].Pos, S.Pos) <= 3.5)
		{
			if (E.Kind == FName(TEXT("clean"))) { if (bCarriedPrev) ++ShieldClean; } else ++ShieldRefused;
		}
	}
	if (Sim->GetFrame() - StickDrillFrame < 30) return;
	++StickDrillAttempts;
	const bool bKept = Sim->CarrierIndex() == Me;
	StickDrillKept += bKept ? 1 : 0;
	StickDrillPoked += bStickDrillPokedThis ? 1 : 0;
	StickDrillKnocked += bStickDrillKnockThis ? 1 : 0;
	StickDrillKnockFrames = FMath::Max(StickDrillKnockFrames, StickDrillKnockFramesThis);
	UE_LOG(LogHokej, Log, TEXT("JAMDRILL stick_result %s n=%d kept=%d poked_clean=%d knocked=%d knock_frames=%d shield_frames=%d shield_refused=%d shield_clean=%d"), StickDrillKind == 2 ? TEXT("spin") : TEXT("sweep"), StickDrillAttempts,
		bKept ? 1 : 0, bStickDrillPokedThis ? 1 : 0, bStickDrillKnockThis ? 1 : 0, StickDrillKnockFramesThis, ShieldLen, ShieldRefused, ShieldClean);
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
	if (Mode >= 7 && Mode <= 11) return MechDrillInput();   // lane A77 (R-0036): the mechanics drills
	if (Mode == 21) return CVarJamCtrlCarryScript.GetValueOnGameThread() == 2 ? ReadHumanInput() : FJamInput();   // lane A102: the carry drill scripts the human in CarryDrillGather (2 = by hand)
	const int32 Me = Sim->GetHumanIndex();
	const FJamSkaterState& S = Sim->GetSkaters()[Me];
	FJamInput In;
	if (Sim->GetPhase() != EJamSimPhase::Playing) return In;
	const int32 C = Sim->CarrierIndex();
	if (C == Me && Mode == 6)
	{
		// Lane A73: the human carries at the nearest standing opponent at full speed and gets checked (pro bots press at 5 m/s closing); no press of its own.
		int32 Best = -1;
		double BestD = 1e9;
		for (const FJamSkaterState& O : Sim->GetSkaters())
		{
			if (O.Team == S.Team || O.IsDown() || O.InBox()) continue;
			const double D = FVector2D::Distance(O.Pos, S.Pos);
			if (D < BestD) { BestD = D; Best = O.Index; }
		}
		In.Move = Best >= 0 ? (Sim->GetSkaters()[Best].Pos - S.Pos).GetSafeNormal() : S.Forward();
		In.Stick = In.Move;
		return In;
	}
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
		if (Mode == 5 && S.Vel.Size() > 6.0) In.Move = FVector2D::ZeroVector;   // lane A73: the side drill never carries 11 m/s into the next pick-up
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
	bool bInRange = Mode == 2 ? Gap <= Sim->GetConfig().Stick.LiftRangeM - 0.2
		: (Gap <= 0.25 || (Closing > 0.5 && Gap <= Closing * (UJamSim::HumanCheckWindowFrames - 3) * UJamSim::DT));
	if (Mode == 6)
	{
		// Lane A73: an opponent carries while the human should be the carrier: skate at it and take the puck back with a check (not an attempt of this drill).
		if (bInRange && !V.IsDown() && S.CanAct()) In.bCheck = true;
		return In;
	}
	if (Mode == 5)
	{
		// Lane A73: the check from the side at 3-6 m/s. Steer for a point 2.5 m beside the carrier (the side the human is already on, led by the carrier's velocity)
		// until roughly level with it, then at it; coast above 6 m/s so the hit lands between 3 and 6; press only when the push is across the carrier's facing.
		const FVector2D VF = V.Forward();
		const FVector2D VRight(-VF.Y, VF.X);
		const FVector2D Rel = S.Pos - V.Pos;
		const double Side = FVector2D::DotProduct(Rel, VRight) >= 0.0 ? 1.0 : -1.0;
		const double Along = FVector2D::DotProduct(Rel, VF);
		const FVector2D Target = FMath::Abs(Along) > 1.0 ? V.Pos + VRight * (Side * 2.5) + V.Vel * 0.3 : V.Pos;
		In.Move = (Target - S.Pos).GetSafeNormal();
		if (S.Vel.Size() > 6.0) In.Move = FVector2D::ZeroVector;
		// The push at contact is the line between the centres then; it is judged where the bodies will meet, not where they are: the carrier's line in the window.
		const FVector2D ToCThen = (V.Pos + V.Vel * (UJamSim::HumanCheckWindowFrames * 0.5 * UJamSim::DT)) - (S.Pos + S.Vel * (UJamSim::HumanCheckWindowFrames * 0.5 * UJamSim::DT));
		const bool bSideHit = Dist > 1e-6 && FMath::Abs(FVector2D::DotProduct(ToCThen.GetSafeNormal(), VF)) < 0.5;
		bInRange = bInRange && bSideHit && S.Vel.Size() >= 2.5 && S.Vel.Size() <= 6.5;
	}
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
			I.bShootHeld = Out[i].bShootHeld;
			I.bPassHeld = Out[i].bPassHeld;   // A104 finding: the brain's saucer hold never reached the sim in single player (the headless curve had it)   // lane A31 (B-to-A-0024 point 1): the bots' charged shot
			I.bCheck = Out[i].bCheck;
			I.bBrake = Out[i].bBrake;
			I.bPass = Out[i].bPass;   // lane A3 (R-0017): pass, stick aim, poke, lift
			I.Stick = FVector2D(Out[i].Stick.X, Out[i].Stick.Y);
			I.bPoke = Out[i].bPoke;
			I.bLift = Out[i].bLift;
			I.bDeke = Out[i].bDeke;   // lane A39: the bots' deke
			I.bSprint = Out[i].bSprint;   // lane A72 (ask A2): game.ai.sprint
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
	if (CVarJamCtrlDrill.GetValueOnGameThread() != 0 && CVarJamCtrlDrill.GetValueOnGameThread() != 20) Inputs[Human] = DrillInput();   // lane A49 proof drill (lane A87: drill 20 keeps the real input)
	else Inputs[Human] = (bScriptedHuman || CVarJamScriptHuman.GetValueOnGameThread() != 0) ? ScriptedHumanInput() : ReadHumanInput();
	if (CVarJamCtrlDrill.GetValueOnGameThread() == 11) MechDrillMate(Inputs);   // lane A77: the scripted teammate of the one-timer drill (after the human's input, never between A49's if and its else)
	if (CVarJamCtrlDrill.GetValueOnGameThread() == 21 && Sim) CarryDrillGather(*Sim, Inputs);   // lane A102 (track F2): the carry drill stages its round, parks the others and scripts the human
	// Lane A82: drills 10 and 11 hand the puck (jam.ctrl.HandPuck 1): A77's human reached its 10 m spot carrying twice in 150 s and the one-timer's set piece never formed.
	// The shots drill puts the puck on the human's blade whenever it is not carrying and no shot of its is being measured; the one-timer drill puts it on the scripted
	// teammate's blade whenever nobody on the human's team carries, no pass of the teammate's is in flight and no shot is being measured. A drill change, not a mechanic one.
	// Lane A91 (R-0044): the hand also serves drill 9 (the human, after a second without the puck) and stages drill 11's set piece: the puck goes to the
	// scripted teammate only when both stand at their spots (the mate 6 m out and 6 m aside, the human 10 m out), and a human left holding the puck for a
	// second gives it back through the hand. Drill 10 as lane A85 left it (inside 1.5 m of the spot only).
	const int32 HandDrill = CVarJamCtrlDrill.GetValueOnGameThread();
	if (CVarJamCtrlHandPuck.GetValueOnGameThread() != 0 && (HandDrill == 9 || HandDrill == 10 || HandDrill == 11) && Sim->GetPhase() == EJamSimPhase::Playing && Sim->GetSkaters().IsValidIndex(Human))
	{
		static int64 HandReady = 0;
		static int32 HumanCarryFrames = 0, HumanLooseFrames = 0;
		const int64 F = Sim->GetFrame();
		const int32 C = Sim->CarrierIndex();
		const FJamSkaterState& H = Sim->GetSkaters()[Human];
		HumanCarryFrames = C == Human ? HumanCarryFrames + 1 : 0;
		HumanLooseFrames = C != Human ? HumanLooseFrames + 1 : 0;
		const double HGx = Sim->AttackGoalX(H.Team);
		const double HDir = HGx > 0 ? 1.0 : -1.0;
		if (F >= HandReady && MechShotFrame < 0 && MechSaucerFrame < 0)
		{
			int32 To = -1;
			if (HandDrill == 9)
			{
				if (HumanLooseFrames >= 60) To = Human;
			}
			else if (HandDrill == 10)
			{
				// Lane A85: inside 1.5 m of its spot only (10 m out on the attacked net's centre line): a hand on the way there was a puck for the bots.
				const FVector2D Spot(HGx - HDir * 10.0, 0.0);
				if (C != Human && FVector2D::Distance(H.Pos, Spot) <= 1.5) To = Human;
			}
			else
			{
				int32 Mate = -1;
				for (const FJamSkaterState& T : Sim->GetSkaters()) if (T.Team == H.Team && T.Index != Human && T.Role != FName(TEXT("back"))) { Mate = T.Index; break; }
				const FVector2D MateSpot(HGx - HDir * 6.0, 6.0), HumanSpot(HGx - HDir * 10.0, -2.0);
				const bool bSet = Mate >= 0 && FVector2D::Distance(Sim->GetSkaters()[Mate].Pos, MateSpot) <= 1.5 && FVector2D::Distance(H.Pos, HumanSpot) <= 1.5;
				const bool bFlight = MechMatePassFrame >= 0 && F - MechMatePassFrame < 90;
				if (bSet && C != Mate && !bFlight && (C != Human || HumanCarryFrames >= 60)) To = Mate;
			}
			if (To >= 0 && Sim->GetSkaters()[To].CanAct())
			{
				const FJamSkaterState& T = Sim->GetSkaters()[To];
				Sim->SetPuck(T.Pos + T.Forward() * 0.45, T.Vel);
				HandReady = F + 60;
				UE_LOG(LogHokej, Log, TEXT("JAMDRILL hand_puck frame=%lld to=%d carrier_was=%d"), F, To, C);
			}
		}
	}
	if (CVarJamCtrlDrill.GetValueOnGameThread() == 20) AcceptDrillInputs(Inputs);   // lane A87: the acceptance run stages the scene and scripts the mate and the opponent
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
		// Lane A84 (R-0041 point 3): the ability slot. The kind held (or the one still armed / running after its last charge), its state and the cooldown.
		const FJamAbilityConfig* Ab = Sim->HeldAbility(H.Index);
		const FName Kind = Ab ? Ab->Kind : (!H.AbilityArmed.IsNone() ? H.AbilityArmed : (H.ShieldFrames > 0 ? FName(TEXT("shield")) : NAME_None));
		if (!Kind.IsNone())
		{
			F.HumanAbility = Kind.ToString().Replace(TEXT("_"), TEXT(" ")).ToUpper();
			const int64 Now = Sim->GetFrame();
			const int32 Cool = Ab ? (Ab->CooldownFrames > 0 ? Ab->CooldownFrames : UJamSim::AbilityDefaultCooldown(Kind)) : 1;
			const int64 Left = FMath::Max<int64>(0, H.AbilityReadyFrame - Now);
			if (H.ShieldFrames > 0) F.HumanAbilityState = FString::Printf(TEXT("SHIELD %.1fs"), H.ShieldFrames / 60.f);
			else if (!H.AbilityArmed.IsNone()) F.HumanAbilityState = TEXT("ARMED");
			else if (Left > 0) F.HumanAbilityState = FString::Printf(TEXT("%llds"), (Left + 59) / 60);
			else F.HumanAbilityState = TEXT("READY");
			if (H.AbilityCharges > 0) F.HumanAbilityState += FString::Printf(TEXT("  x%d"), H.AbilityCharges);
			F.HumanAbilityReady01 = H.ShieldFrames > 0 ? 1.f : 1.f - FMath::Clamp((float)Left / (float)FMath::Max(1, Cool), 0.f, 1.f);
			F.bHumanAbilityLive = H.ShieldFrames > 0 || !H.AbilityArmed.IsNone();
		}
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
	// Lane A84 (R-0041 point 3): the proof that F reached the sim and the ability fired (kind, charges left, -1 = unlimited, the frame it is ready again).
	if (Sim->GetSkaters().IsValidIndex(Sim->GetHumanIndex()) && Sim->GetSkaters()[Sim->GetHumanIndex()].bAbilityThisFrame)
	{
		const FJamSkaterState& H = Sim->GetSkaters()[Sim->GetHumanIndex()];
		FName Kind = H.AbilityArmed;
		if (Kind.IsNone()) { const FJamAbilityConfig* Ab = Sim->HeldAbility(H.Index); Kind = Ab ? Ab->Kind : (H.ShieldFrames > 0 ? FName(TEXT("shield")) : NAME_None); }
		for (const FJamSimEvent& E : Sim->GetFrameEvents()) if (E.Name == JamEvents::Ability && E.Who == H.Index && E.Strength > 0.5f) Kind = E.Kind;
		UE_LOG(LogHokej, Log, TEXT("JAMCTRL frame=%lld ability used kind=%s charges=%d ready=%lld"), Sim->GetFrame(), *Kind.ToString(), H.AbilityCharges, H.AbilityReadyFrame);
	}
	if (CVarJamCtrlDrill.GetValueOnGameThread() == 20) AcceptDrillObserve();   // lane A87 (R-0043): the acceptance run
	MechDrillObserve();   // lane A77 (R-0036): the human's saucers always; the shots and one-timers in drills 10 and 11
	if (CVarJamCtrlDrill.GetValueOnGameThread() == 21 && Sim) CarryDrillObserve(*Sim);   // lane A102 (track F2): the carry drill's round
	if (CVarJamCtrlDrill.GetValueOnGameThread() == 3 || CVarJamCtrlDrill.GetValueOnGameThread() == 4 || CVarJamCtrlDrill.GetValueOnGameThread() == 7 || CVarJamCtrlDrill.GetValueOnGameThread() == 8 || StickDrillFrame >= 0) StickDrillObserve();   // lane A51 (7 and 8: lane A77)
	else if (CVarJamCtrlDrill.GetValueOnGameThread() != 0 || DrillPressFrame >= 0) DrillObserve();   // lane A49
	if (CVarJamCtrlDrill.GetValueOnGameThread() == 6 || bKdWasDown) KdObserve();   // lane A73
	DeliverEvents();
	HitStopRoute();   // lane A73: before the pawns apply the frame
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
		if (St.bShotThisFrame) UE_LOG(LogHokej, Log, TEXT("JAMSHOT frame=%lld skater=%d team=%d charge=%.2f %s one_timer=%d"), Sim->GetFrame(), St.Index, St.Team, St.LastShotCharge, St.bHuman ? TEXT("human") : TEXT("bot"), St.bOneTimerThisFrame ? 1 : 0);
		if (St.bPassThisFrame) UE_LOG(LogHokej, Log, TEXT("JAMPASS frame=%lld skater=%d team=%d strength=%.2f %s saucer=%d (PlayPass only, no PlayShoot)"), Sim->GetFrame(), St.Index, St.Team, St.LastPassStrength, St.bHuman ? TEXT("human") : TEXT("bot"), St.bSaucerThisFrame ? 1 : 0);   // saucer: lane A77
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
	if (Puck) Puck->StepSimFrame();   // lane A73: the puck's hit-stop counter
	StillsObserve();   // lane A73 proof
	// Lane A73: while the figures hold the hit frame, the presentation frame (camera, HUD, radar) holds too; the hit frame itself is pushed (the camera's kick starts on it).
	if (!(Sim->GetFrame() > HitStopStartFrame && Sim->GetFrame() <= HitStopEndFrame)) PushPresentation();
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
	TickLightHold();   // lane A111: presentation only (the camera's fade); the sim below steps as ever
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
	// Lane A73: the camera interpolates between its last two sim poses with this alpha; while the hit frame is held it sits on the hit frame's pose (alpha 1).
	const bool bHeld = Sim->GetFrame() >= HitStopStartFrame && Sim->GetFrame() <= HitStopEndFrame;
	if (UJamPresentation* Pres = UJamPresentation::Get(this)) Pres->SetRenderAlpha(bHeld ? 1.f : Alpha);
}

void AHokejGameMode::HitStopRoute()
{
	const int32 Base = AJamSkater::HitStopFrames();
	if (Base <= 0 || !Sim || Sim->GetFrame() <= HitStopEndFrame) return;
	const int32 Human = Sim->GetHumanIndex();
	// Lane A101 (track F1): the hold reads the hit (AJamSkater::HitStopFramesFor). A knockdown: 2 to 5 frames by the victim's ContactRatio; a stumble the human
	// takes or lands 1 or 2 frames; the human's own bounce 1. Lane A113: AJamSkater::HitStopPick chooses the hit (one that involves the human is never hidden
	// by a knockdown between two bots in the same frame; a stumble's strength is its check event's).
	const FJamHitStopPick Pick = AJamSkater::HitStopPick(Sim->GetSkaters(), Sim->GetFrameEvents(), Human, Sim->GetConfig().Contact.BumpRatio);
	if (Pick.Victim < 0) return;
	const int32 Victim = Pick.Victim, Checker = Pick.Checker, Outcome = Pick.Outcome;
	const float Strength = Pick.Strength, Ratio = Pick.Ratio;
	const FName Kind = Pick.Kind;
	const int32 Hold = AJamSkater::HitStopFramesFor(Outcome, Ratio, Sim->GetConfig().Contact, Base);
	if (Hold <= 0) return;
	for (AJamSkater* A : Skaters) if (A && (A->GetSimIndex() == Victim || A->GetSimIndex() == Checker)) A->BeginHitStop(Hold);
	if (Puck) Puck->BeginHitStop(Hold);
	HitStopStartFrame = Sim->GetFrame();
	HitStopEndFrame = HitStopStartFrame + Hold - 1;
	UE_LOG(LogHokej, Log, TEXT("JAMHITSTOP frame=%lld frames=%d victim=%d checker=%d human=%d kind=%s strength=%.2f outcome=%d ratio=%.2f base=%d (presentation only: the sim steps on; held through frame %lld)"),
		HitStopStartFrame, Hold, Victim, Checker, Human, *Kind.ToString(), Strength, Outcome, Ratio, Base, HitStopEndFrame);
}

void AHokejGameMode::StillsObserve()
{
	const int32 StillsMode = CVarJamCtrlStills.GetValueOnGameThread();
	if (StillsMode == 0 || !Sim) return;
	const int64 F = Sim->GetFrame();
	const int32 Human = Sim->GetHumanIndex();
	if (!Sim->GetSkaters().IsValidIndex(Human)) return;
	if (StillStart < 0)
	{
		FString Kind;
		int32 Count = 0, Max = 2;
		const FJamSkaterState& H = Sim->GetSkaters()[Human];
		if (StillsMode == 2)
		{
			// Lane A77 (R-0036): the mechanics stills: the puck on the blade through a sweep (10 s = 150 stills, once), the spin with the puck (24, twice), the saucer (20, twice).
			if (H.bSweepThisFrame && Sim->CarrierIndex() == Human && CVarJamCtrlDrill.GetValueOnGameThread() == 7) { Kind = TEXT("sweep"); Count = 150; Max = 1; }   // lane A82: drill 7 only (A77's spin runs spent the capture on a crossing)
			else if (H.bSpinThisFrame && Sim->CarrierIndex() == Human) { Kind = TEXT("spin"); Count = 24; }
			else if (H.bSaucerThisFrame) { Kind = TEXT("saucer"); Count = 20; }
			else if (H.bAbilityThisFrame) { Kind = TEXT("ability"); Count = 6; }   // lane A84 (R-0041 point 3): the HUD's ability slot after F
		}
		else
		{
			int32 Victim = -1, Checker = -1;
			for (const FJamSimEvent& E : Sim->GetFrameEvents())
			{
				if (E.Name == JamEvents::Knockdown && E.Kind == FName(TEXT("check"))) Victim = E.Who;
				if (E.Name == JamEvents::Check) Checker = E.Who;
			}
			if (Victim == Human) { Kind = TEXT("kd-human"); Count = 24; }
			else if (Victim >= 0 && Checker == Human) { Kind = TEXT("kd-bot"); Count = 24; }
			if (Kind.IsEmpty() && H.bBoardHitThisFrame) { Kind = TEXT("board"); Count = 8; }
			if (Kind.IsEmpty() && H.StickHitResult != 0) { Kind = TEXT("stickhit"); Count = 8; }
		}
		if (Kind.IsEmpty()) return;
		int32& N = StillsPerKind.FindOrAdd(Kind);
		if (N >= Max) return;
		++N;
		StillKind = Kind;
		StillStart = F;
		StillCount = Count;
		StillTaken = 0;
		UE_LOG(LogHokej, Log, TEXT("JAMSTILLS start kind=%s frame=%lld every=4 count=%d"), *Kind, F, Count);
	}
	if ((F - StillStart) % 4 == 0 && StillTaken < StillCount)
	{
		FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("%s-%s-%lld-f%06lld.png"), StillsMode == 2 ? TEXT("a77") : TEXT("a73"), *StillKind, StillStart, F), true, false);
		++StillTaken;
	}
	if (StillTaken >= StillCount) StillStart = -1;
}

// Lane A77 (R-0036): the mechanics drills. Only the skating is scripted; the stick, E, F and the left button come through ReadHumanInput as the driver sends them.
FJamInput AHokejGameMode::MechDrillInput()
{
	FJamInput In = ReadHumanInput();
	if (Sim->GetPhase() != EJamSimPhase::Playing) return In;
	const int32 Mode = CVarJamCtrlDrill.GetValueOnGameThread();
	const int32 Me = Sim->GetHumanIndex();
	const FJamSkaterState& S = Sim->GetSkaters()[Me];
	const int64 F = Sim->GetFrame();
	const int32 C = Sim->CarrierIndex();
	const FVector2D PuckPos = Sim->GetPuck().Pos;
	const double Gx = Sim->AttackGoalX(S.Team);
	const double Dir = Gx > 0 ? 1.0 : -1.0;
	const FVector2D HumanSpot(Gx - Dir * 10.0, Mode == 11 ? -2.0 : 0.0);
	const FVector2D PreSpot(Gx - Dir * 14.0, HumanSpot.Y);
	// Lane A85: with the hand on (jam.ctrl.HandPuck 1) the shots drill holds its set piece, carrying or not: stage 0 to the pre-spot 14 m out, stage 1 from there to
	// the spot 10 m out (so the body, which turns only toward its move, arrives facing the net), stage 2 standing inside 1.5 m of the spot, turned to the net by a
	// creep at it when the facing is off; beyond 1.5 m it starts over. No chase, no check: the hand puts the puck on it there, and the cue comes as soon as it
	// carries facing the net. Before, it left the spot for every loose puck while the hand fed it on the way: A82's run reached the cue twice in 150 s (88 hands),
	// A85's first run never (88 hands).
	// Lane A91 (R-0044): the stand is a standstill, and drill 11's human stands the same way (the one-timer's set piece). A85's stage 2 commanded Move 0 after
	// arriving at 0.6 of top speed with no brake: the body glided on through the spot (the ice only has glide drag); a 39-frame full hold glided 3 m nearer the
	// net, left the 1.5 m ring, the stage fell back to 0 and the body turned to skate back to the pre-spot; TryShoot aims along the body, so the release went up
	// the ice (A85: 0 of 10 full charges reached the line, two met the side boards 17 and 30 m back). Now: the last 2 m at 0.3, the brake in stage 2 while the
	// body moves or the button is held, no stage change during a hold, and the cue only at a standstill. JAMDRILL stage logs each stage change.
	if ((Mode == 10 || Mode == 11) && CVarJamCtrlHandPuck.GetValueOnGameThread() != 0)
	{
		static int32 Stage = 0;
		const FVector2D ToPre = PreSpot - S.Pos, ToSpot = HumanSpot - S.Pos;
		const FVector2D NetDir = (FVector2D(Gx, 0.0) - S.Pos).GetSafeNormal();
		const bool bFaces = FVector2D::DotProduct(S.Forward(), NetDir) > FMath::Cos(FMath::DegreesToRadians(8.0));
		const bool bHolding = In.bShootHeld || S.ChargeFrames > 0;
		const double SpeedNow = S.Vel.Size();
		static int32 StageLogged = -1;
		if (Stage == 2 && ToSpot.Size() > 1.5 && !bHolding) Stage = 0;
		if (Stage != StageLogged)
		{
			StageLogged = Stage;
			UE_LOG(LogHokej, Log, TEXT("JAMDRILL stage frame=%lld stage=%d pos=(%.2f,%.2f) to_spot=%.2f speed=%.2f faces=%d holding=%d carrier=%d"), F, Stage, S.Pos.X, S.Pos.Y, ToSpot.Size(), SpeedNow, bFaces ? 1 : 0, bHolding ? 1 : 0, C);
		}
		// The arrival: the speed a 3 m/s2 stop from here allows; over it the brake (the body only loses speed by glide drag otherwise: the first A91 package
		// reached stage 2 at 6.5 to 9 m/s, 0.3 m from the spot, and was out of the ring 14 frames later: a 246-frame cycle, never a stand).
		const double TopSpeed = FMath::Max(1.0, Sim->GetEffectiveStat(EJamStat::SkaterSpeed, Me));
		auto Arrive = [&](const FVector2D& To)
		{
			const double Dd = To.Size();
			const double Want = FMath::Sqrt(2.0 * 3.0 * FMath::Max(0.0, Dd - 0.2));
			if (SpeedNow > Want + 0.5) { In.Move = FVector2D::ZeroVector; In.bBrake = true; }
			else In.Move = To.GetSafeNormal() * FMath::Clamp(Want / TopSpeed, 0.1, 0.8);
		};
		if (Stage == 0) { if (ToPre.Size() > 1.0 || SpeedNow > 3.0) { Arrive(ToPre); return In; } Stage = 1; }
		if (Stage == 1) { if (ToSpot.Size() > 0.5 || SpeedNow > 1.0) { Arrive(ToSpot); return In; } Stage = 2; }
		if (bHolding || SpeedNow > 0.3) { In.Move = FVector2D::ZeroVector; In.bBrake = true; }
		else In.Move = bFaces ? FVector2D::ZeroVector : NetDir * 0.15;
		if (Mode == 10 && C == Me && bFaces && SpeedNow < 0.5 && F >= MechCueReady && F >= S.ShotReadyFrame && S.ChargeFrames == 0)
		{
			MechCueReady = F + 90;
			const TCHAR* Kind = MechShotN % 3 == 0 ? TEXT("tap") : (MechShotN % 3 == 1 ? TEXT("half") : TEXT("full"));
			UE_LOG(LogHokej, Log, TEXT("JAMCUE shot frame=%lld kind=%s dist_line=%.2f speed=%.2f"), F, Kind, FMath::Abs(Gx - PuckPos.X), SpeedNow);
		}
		return In;
	}
	if (C != Me)
	{
		bMechAtPre = false;
		if (Mode == 11)
		{
			// the one-timer drill: the human waits at its spot facing the net (approached from behind so the facing is the goal's); the scripted teammate brings the puck
			const FVector2D ToPre = PreSpot - S.Pos, ToSpot = HumanSpot - S.Pos;
			if (!bMechAtPre && ToPre.Size() > 1.0 && ToSpot.Size() > 1.5) In.Move = ToPre.GetSafeNormal();
			else In.Move = ToSpot.Size() > 0.4 ? ToSpot.GetSafeNormal() * 0.7 : FVector2D::ZeroVector;
			return In;
		}
		if (C >= 0 && Sim->GetSkaters()[C].Team != S.Team)
		{
			// an opponent carries: take it back with a check (as drill 6 does); not an attempt of any drill here
			const FJamSkaterState& V = Sim->GetSkaters()[C];
			const FVector2D ToC = V.Pos - S.Pos;
			const double Dist = ToC.Size();
			In.Move = ToC.GetSafeNormal();
			const double Gap = Dist - UJamSim::SkaterRadius * 2.0;
			const double Closing = Dist > 1e-6 ? FVector2D::DotProduct(S.Vel - V.Vel, ToC / Dist) : 0.0;
			if ((Gap <= 0.25 || (Closing > 0.5 && Gap <= Closing * (UJamSim::HumanCheckWindowFrames - 3) * UJamSim::DT)) && !V.IsDown() && S.CanAct()) In.bCheck = true;
			return In;
		}
		const FVector2D ToPuck = PuckPos - S.Pos;
		In.Move = ToPuck.Size() > 0.3 ? ToPuck.GetSafeNormal() : FVector2D::ZeroVector;
		return In;
	}
	const int32 O = Sim->NearestOpponent(S);
	const FVector2D ToO = O >= 0 ? Sim->GetSkaters()[O].Pos - S.Pos : FVector2D(Gx, 0.0) - S.Pos;
	const double D = ToO.Size();
	if (Mode == 7)
	{
		// The sweep: keep the body 1.9-2.4 m from the defender (outside its lift at contact, the puck on the blade toward it inside its poke reach): at it at 60 %
		// farther out, away at full speed closer. The cue: the defender's blade within 0.9 m of the puck (its poke lands 6 frames after the blade sits in reach).
		In.Move = D > 2.4 ? ToO.GetSafeNormal() * 0.6 : (D < 1.9 ? -ToO.GetSafeNormal() : FVector2D::ZeroVector);
		if (O >= 0 && F >= MechCueReady)
		{
			const double BladeToPuck = FVector2D::Distance(Sim->GetSkaters()[O].BladePos, PuckPos);
			if (BladeToPuck < 0.9)
			{
				MechCueReady = F + 20;
				const int32 Side = FVector2D::CrossProduct(S.Forward(), PuckPos - S.Pos) >= 0.0 ? 1 : -1;
				UE_LOG(LogHokej, Log, TEXT("JAMCUE sweep frame=%lld defender=%d dist=%.2f blade_to_puck=%.2f puck_side=%d"), F, O, D, BladeToPuck, Side);
			}
		}
		return In;
	}
	if (Mode == 8)
	{
		// The spin: carry at the goal at 60 %; the cue when a defender is within 2.5 m, the bar has a quarter and the cooldown is over.
		const FVector2D ToGoal = FVector2D(Gx, 0.0) - S.Pos;
		In.Move = ToGoal.GetSafeNormal() * 0.6;
		const bool bStamina = S.Stamina < 0 || S.Stamina >= Sim->StaminaFull() / 4;
		if (O >= 0 && D < 2.5 && F >= MechCueReady && S.SpinFrames == 0 && F >= S.SpinReadyFrame && bStamina)
		{
			MechCueReady = F + 120;
			UE_LOG(LogHokej, Log, TEXT("JAMCUE spin frame=%lld defender=%d dist=%.2f stamina=%.2f"), F, O, D, S.Stamina < 0 ? 1.0 : static_cast<double>(S.Stamina) / Sim->StaminaFull());
		}
		return In;
	}
	if (Mode == 9)
	{
		// The saucer: carry up the ice slowly; away from a defender within 3 m. The cue when the sim has a saucer receiver and an opponent's body sits within 1.6 m
		// (a blade's reach) of the lane inside its airborne part.
		const FVector2D ToGoal = FVector2D(Gx, 0.0) - S.Pos;
		In.Move = (O >= 0 && D < 3.0) ? -ToO.GetSafeNormal() : ToGoal.GetSafeNormal() * 0.5;
		if (F >= MechCueReady)
		{
			FVector2D AimS, AimF;
			const int32 RS = Sim->BestPass(Me, AimS, true);
			const int32 RF = Sim->BestPass(Me, AimF, false);
			if (RS >= 0)
			{
				const FVector2D To = AimS - PuckPos;
				const double DistAim = To.Size();
				const FVector2D Ldir = To.GetSafeNormal();
				double Lane = 1e9, Along = 0.0;
				// Lane A91 (R-0044): the saucer is proven by its flight over a blade when one is on the lane, so the cue asks for an opponent's BLADE within
				// SaucerCueBladeM of the lane inside the airborne part (A85's cue asked for a body within 1.6 m: 1 saucer of 6 met a blade, the nearest 0.35 to 3.3 m off).
				const double SaucerCueBladeM = 0.45;
				double BladeLane = 1e9, BladeAlong = 0.0;
				// The launch must be clear: the saucer is under BladeClearM for its first SaucerAirFromM, where any blade or body takes it (the first A91 run: 2
				// saucers, both taken 0.07 to 0.08 m up by an opponent at the passer's shoulder). No opponent body within 1.3 m, no blade within 0.9 m, of the
				// lane's first 2 m.
				const double LaunchM = UJamSim::SaucerAirFromM + 0.5;
				bool bLaunchBlocked = false;
				for (const FJamSkaterState& Op : Sim->GetSkaters())
				{
					if (Op.Team == S.Team || Op.InBox()) continue;
					const double T0 = FMath::Clamp(FVector2D::DotProduct(Op.Pos - PuckPos, Ldir), 0.0, DistAim);
					const double Dl = FVector2D::Distance(PuckPos + Ldir * T0, Op.Pos);
					if (Dl < Lane) { Lane = Dl; Along = T0; }
					if (T0 < LaunchM && Dl < 1.3) bLaunchBlocked = true;
					if (!Op.CanAct()) continue;
					const double Tb = FMath::Clamp(FVector2D::DotProduct(Op.BladePos - PuckPos, Ldir), 0.0, DistAim);
					const double Db = FVector2D::Distance(PuckPos + Ldir * Tb, Op.BladePos);
					if (Tb < LaunchM && Db < 0.9) bLaunchBlocked = true;
					if (Db < BladeLane) { BladeLane = Db; BladeAlong = Tb; }
				}
				if (!bLaunchBlocked && BladeLane < SaucerCueBladeM && BladeAlong > UJamSim::SaucerAirFromM + 0.5 && BladeAlong < DistAim * UJamSim::SaucerLandFrac - 1.0)   // inside the sim's airborne window (lane A82), half a metre in from both ends
				{
					MechCueReady = F + 60;
					UE_LOG(LogHokej, Log, TEXT("JAMCUE saucer frame=%lld receiver=%d flat_receiver=%d lane_m=%.2f along_m=%.2f dist_m=%.2f blade_lane_m=%.2f blade_along_m=%.2f"), F, RS, RF, Lane, Along, DistAim, BladeLane, BladeAlong);
				}
			}
		}
		return In;
	}
	if (Mode == 10 || Mode == 11)
	{
		// Shots from 10 m (and the one-timer drill's give-away): to the pre-spot 14 m out, then the spot 10 m out on the attacked goal's centre line, facing it.
		const FVector2D ToPre = PreSpot - S.Pos, ToSpot = HumanSpot - S.Pos;
		if (!bMechAtPre && ToPre.Size() > 1.0 && ToSpot.Size() > 2.0) { In.Move = ToPre.GetSafeNormal() * 0.8; return In; }
		bMechAtPre = true;
		In.Move = ToSpot.Size() > 0.4 ? ToSpot.GetSafeNormal() * 0.6 : FVector2D::ZeroVector;
		if (ToSpot.Size() > 0.6 || F < MechCueReady) return In;
		const FVector2D ToGoal = (FVector2D(Gx, 0.0) - PuckPos).GetSafeNormal();
		const bool bFacing = FVector2D::DotProduct(S.Forward(), ToGoal) > FMath::Cos(FMath::DegreesToRadians(12.0));
		if (Mode == 10 && bFacing && F >= S.ShotReadyFrame && S.ChargeFrames == 0)
		{
			MechCueReady = F + 90;
			const TCHAR* Kind = MechShotN % 3 == 0 ? TEXT("tap") : (MechShotN % 3 == 1 ? TEXT("half") : TEXT("full"));
			UE_LOG(LogHokej, Log, TEXT("JAMCUE shot frame=%lld kind=%s dist_line=%.2f"), F, Kind, FMath::Abs(Gx - PuckPos.X));
		}
		if (Mode == 11)
		{
			FVector2D Aim;
			if (Sim->BestPass(Me, Aim) >= 0 && F >= S.ShotReadyFrame) { MechCueReady = F + 90; UE_LOG(LogHokej, Log, TEXT("JAMCUE pass_to_mate frame=%lld to=%d"), F, Sim->BestPass(Me, Aim)); }
		}
		return In;
	}
	return In;
}

void AHokejGameMode::MechDrillMate(TArray<FJamInput>& AllInputs)
{
	// Drill 11: the human's teammate (not the back) carries the puck to a spot 6 m out and 6 m to the side of the attacked net and passes to the human standing at
	// its spot 10 m out when the sim's pass would go there; the JAMCUE onetimer line carries the flight's ETA so the driver taps the shot inside the one-timer window.
	if (!Sim || Sim->GetPhase() != EJamSimPhase::Playing) return;
	const int32 Me = Sim->GetHumanIndex();
	const FJamSkaterState& H = Sim->GetSkaters()[Me];
	int32 Mate = -1;
	for (const FJamSkaterState& T : Sim->GetSkaters()) if (T.Team == H.Team && T.Index != Me && T.Role != FName(TEXT("back"))) { Mate = T.Index; break; }
	if (Mate < 0 || !AllInputs.IsValidIndex(Mate)) return;
	const FJamSkaterState& M = Sim->GetSkaters()[Mate];
	const double Gx = Sim->AttackGoalX(H.Team);
	const double Dir = Gx > 0 ? 1.0 : -1.0;
	const FVector2D MateSpot(Gx - Dir * 6.0, 6.0);
	const FVector2D HumanSpot(Gx - Dir * 10.0, -2.0);
	const int32 C = Sim->CarrierIndex();
	const int64 F = Sim->GetFrame();
	FJamInput In;
	// Lane A91 (R-0044): with the hand on the set piece is staged like drill 10's. The mate goes to its spot and stands there (the brake, no chase, no check); the
	// hand gives it the puck when both stand at their spots (the block after the human's input); settled, it passes: the sim's own pass when the sim would send
	// it to the human (kind=sim), else the hand sends the puck from the mate's blade at the human at the pass speed (kind=hand: in 5 v 5 the sim's best receiver
	// from 6 m out is almost never the skater standing behind the passer; A85's set piece formed once in 120 s). The reception is the sim's either way (a
	// teammate's puck in flight onto the blade or the body: PassReceivedFrame), and JAMCUE onetimer_recv tells the driver to release.
	if (CVarJamCtrlHandPuck.GetValueOnGameThread() != 0)
	{
		static int32 MateCarry = 0;
		MateCarry = C == Mate ? MateCarry + 1 : 0;
		const FVector2D To = MateSpot - M.Pos;
		const double Dm = To.Size();
		if (Dm > 0.5) In.Move = To.GetSafeNormal() * (Dm > 2.0 ? 0.8 : 0.3);
		else if (M.Vel.Size() > 0.3) In.bBrake = true;
		In.Stick = (H.Pos - M.Pos).GetSafeNormal() * 0.5;
		const bool bHumanSet = FVector2D::Distance(H.Pos, HumanSpot) < 1.5 && H.Vel.Size() < 1.0 && H.CanAct();
		if (C == Mate && MateCarry >= 20 && Dm <= 1.2 && M.Vel.Size() < 1.0 && bHumanSet && F >= MechMateReady && F >= M.ShotReadyFrame && MechShotFrame < 0)
		{
			FVector2D Aim;
			const int32 Best = Sim->BestPass(Mate, Aim);
			const double Speed = FMath::Max(1.0, Sim->GetConfig().Skater.PassSpeed);
			const FVector2D PuckAt = Sim->GetPuck().Pos;
			if (Best == Me) In.bPass = true;
			else
			{
				Aim = (H.Pos + H.BladePos) * 0.5;
				Sim->SetPuck(PuckAt, (Aim - PuckAt).GetSafeNormal() * Speed);
			}
			MechMateReady = F + 150;
			MechMatePassFrame = F;
			const double Dist = FVector2D::Distance(Aim, PuckAt);
			UE_LOG(LogHokej, Log, TEXT("JAMCUE onetimer frame=%lld mate=%d dist=%.2f eta_frames=%d kind=%s best_pass=%d"), F, Mate, Dist, FMath::CeilToInt32(Dist / (Speed * 0.85) / UJamSim::DT), Best == Me ? TEXT("sim") : TEXT("hand"), Best);
		}
		AllInputs[Mate] = In;
		return;
	}
	if (C == Mate)
	{
		const FVector2D To = MateSpot - M.Pos;
		In.Move = To.Size() > 0.5 ? To.GetSafeNormal() * 0.8 : FVector2D::ZeroVector;
		In.Stick = (H.Pos - M.Pos).GetSafeNormal() * 0.5;
		FVector2D Aim;
		if (To.Size() <= 1.2 && FVector2D::Distance(H.Pos, HumanSpot) < 1.5 && F >= MechMateReady && F >= M.ShotReadyFrame && Sim->BestPass(Mate, Aim) == Me)
		{
			In.bPass = true;
			MechMateReady = F + 120;
			MechMatePassFrame = F;
			const double Dist = FVector2D::Distance(Aim, Sim->GetPuck().Pos);
			const double Speed = FMath::Max(1.0, Sim->GetConfig().Skater.PassSpeed);
			UE_LOG(LogHokej, Log, TEXT("JAMCUE onetimer frame=%lld mate=%d dist=%.2f eta_frames=%d"), F, Mate, Dist, FMath::CeilToInt32(Dist / (Speed * 0.85) / UJamSim::DT));
		}
	}
	else if (C >= 0 && Sim->GetSkaters()[C].Team != H.Team)
	{
		const FJamSkaterState& V = Sim->GetSkaters()[C];
		const FVector2D ToC = V.Pos - M.Pos;
		const double Dist = ToC.Size();
		In.Move = ToC.GetSafeNormal();
		In.Stick = In.Move;
		const double Gap = Dist - UJamSim::SkaterRadius * 2.0;
		const double Closing = Dist > 1e-6 ? FVector2D::DotProduct(M.Vel - V.Vel, ToC / Dist) : 0.0;
		if ((Gap <= 0.25 || (Closing > 0.5 && Gap <= Closing * 5.0 * UJamSim::DT)) && !V.IsDown() && M.CanAct()) In.bCheck = true;
	}
	else if (C != Me)
	{
		const FVector2D ToPuck = Sim->GetPuck().Pos - M.Pos;
		In.Move = ToPuck.Size() > 0.3 ? ToPuck.GetSafeNormal() : FVector2D::ZeroVector;
	}
	else
	{
		const FVector2D To = MateSpot - M.Pos;   // the human carries: stand open at the spot (the driver passes to it on JAMCUE pass_to_mate)
		In.Move = To.Size() > 0.5 ? To.GetSafeNormal() * 0.8 : FVector2D::ZeroVector;
	}
	AllInputs[Mate] = In;
}

void AHokejGameMode::MechDrillObserve()
{
	if (!Sim) return;
	const int32 Mode = CVarJamCtrlDrill.GetValueOnGameThread();
	const int32 Me = Sim->GetHumanIndex();
	if (!Sim->GetSkaters().IsValidIndex(Me)) return;
	const FJamSkaterState& S = Sim->GetSkaters()[Me];
	const FJamPuckState& P = Sim->GetPuck();
	const int64 F = Sim->GetFrame();
	const double Gx = Sim->AttackGoalX(S.Team);
	const double Dir = Gx > 0 ? 1.0 : -1.0;
	// Lane A91: drill 11's reception, the frame the one-timer window opens: the driver holds the left button over it and releases on this line.
	if (Mode == 11)
	{
		static int64 LastRecv = -1;
		if (S.PassReceivedFrame != LastRecv)
		{
			if (S.PassReceivedFrame >= 0) UE_LOG(LogHokej, Log, TEXT("JAMCUE onetimer_recv frame=%lld recv_frame=%lld since_pass=%lld charge_frames=%d"), F, S.PassReceivedFrame, MechMatePassFrame >= 0 ? F - MechMatePassFrame : -1, S.ChargeFrames);
			LastRecv = S.PassReceivedFrame;
		}
	}
	// The human's shot (drills 10 and 11): its charge, then the puck's height when it crosses the goal line (or the goal / save / post that ended the flight).
	if ((Mode == 10 || Mode == 11) && S.bShotThisFrame && MechShotFrame < 0)
	{
		MechShotFrame = F;
		MechShotCharge = S.LastShotCharge;
		MechShotPrevZ = P.Z;
		bMechShotGoal = bMechShotSave = bMechShotPost = false;
		if (Mode == 11)
		{
			++MechOtShots;
			if (S.bOneTimerThisFrame) ++MechOneTimers;
			UE_LOG(LogHokej, Log, TEXT("JAMDRILL onetimer n=%d frame=%lld one_timer=%d charge=%.2f since_pass=%lld"), MechOtShots, F, S.bOneTimerThisFrame ? 1 : 0, S.LastShotCharge, MechMatePassFrame >= 0 ? F - MechMatePassFrame : -1);
			if (MechOtShots % 5 == 0) UE_LOG(LogHokej, Log, TEXT("JAMDRILL onetimer_summary shots=%d one_timers=%d"), MechOtShots, MechOneTimers);
		}
	}
	else if (MechShotFrame >= 0)
	{
		for (const FJamSimEvent& E : Sim->GetFrameEvents())
		{
			if (E.Name == JamEvents::Goal) bMechShotGoal = true;
			if (E.Name == JamEvents::Save) bMechShotSave = true;
			if (E.Name == JamEvents::Post) bMechShotPost = true;
		}
		const bool bCrossed = (P.Pos.X - Gx) * Dir >= 0.0;
		const bool bTimeout = F - MechShotFrame > 150;
		const bool bLost = P.LastTouch != Me && !bCrossed && !bMechShotGoal;   // a body or a blade took it before the line
		if (bCrossed || bTimeout || bLost || bMechShotGoal || bMechShotSave)
		{
			++MechShotN;
			const TCHAR* Kind = MechShotCharge < 0.15f ? TEXT("tap") : (MechShotCharge < 0.6f ? TEXT("half") : TEXT("full"));
			if (MechShotCharge < 0.15f) ++MechShotTaps; else if (MechShotCharge < 0.6f) ++MechShotHalf; else ++MechShotFull;
			const double ArrivalZ = bCrossed ? P.Z : (bMechShotGoal ? MechShotPrevZ : -1.0);
			UE_LOG(LogHokej, Log, TEXT("JAMDRILL shot n=%d kind=%s charge=%.2f lift_target=%.3f arrival_z=%.3f crossed=%d goal=%d save=%d post=%d blocked=%d flight_frames=%lld"), MechShotN, Kind, MechShotCharge,
				Sim->LiftForCharge(MechShotCharge), ArrivalZ, bCrossed ? 1 : 0, bMechShotGoal ? 1 : 0, bMechShotSave ? 1 : 0, bMechShotPost ? 1 : 0, bLost ? 1 : 0, F - MechShotFrame);
			MechShotFrame = -1;
			if (Mode == 10 && MechShotN % 10 == 0) UE_LOG(LogHokej, Log, TEXT("JAMDRILL shot_summary shots=%d taps=%d half=%d full=%d"), MechShotN, MechShotTaps, MechShotHalf, MechShotFull);
		}
		else MechShotPrevZ = P.Z;
	}
	// The human's saucer (any drill): the flight over the nearest opponent blade, the landing, the reception.
	if (S.bSaucerThisFrame && MechSaucerFrame < 0)
	{
		MechSaucerFrame = F;
		MechSaucerReceiver = -1;
		MechSaucerMinBladeAir = 1e9;
		MechSaucerZAtBlade = MechSaucerMaxZ = 0.0;
		MechSaucerLand = -1;
		MechSaucerLandDist = -1.0;
		bMechSaucerOverThis = false;
		for (const FJamSimEvent& E : Sim->GetFrameEvents())
		{
			if (E.Name != JamEvents::Pass || E.Who != Me) continue;
			const FString K = E.Kind.ToString();
			int32 At = -1;
			if (K.FindLastChar(TCHAR('_'), At)) MechSaucerReceiver = FCString::Atoi(*K.Mid(At + 1));
		}
		UE_LOG(LogHokej, Log, TEXT("JAMCTRL frame=%lld saucer receiver=%d vz=%.2f"), F, MechSaucerReceiver, P.VZ);
	}
	else if (MechSaucerFrame >= 0)
	{
		if (P.Z > 0.0)
		{
			for (const FJamSkaterState& O : Sim->GetSkaters())
			{
				if (O.Team == S.Team || O.InBox()) continue;
				const double Db = FVector2D::Distance(O.BladePos, P.Pos);
				if (Db < MechSaucerMinBladeAir) { MechSaucerMinBladeAir = Db; MechSaucerZAtBlade = P.Z; }
				if (Db < UJamSim::DeflectRangeM && P.Z >= UJamSim::BladeClearM) bMechSaucerOverThis = true;
			}
		}
		MechSaucerMaxZ = FMath::Max(MechSaucerMaxZ, P.Z);
		if (MechSaucerLand < 0 && F - MechSaucerFrame > 2 && P.Z <= 0.0)
		{
			MechSaucerLand = F;
			MechSaucerLandDist = Sim->GetSkaters().IsValidIndex(MechSaucerReceiver) ? FVector2D::Distance(P.Pos, Sim->GetSkaters()[MechSaucerReceiver].Pos) : -1.0;
		}
		const bool bTouched = P.LastTouch != Me;
		if (bTouched || F - MechSaucerFrame > 150)
		{
			++MechSaucerN;
			const bool bReceived = bTouched && Sim->GetSkaters().IsValidIndex(P.LastTouch) && Sim->GetSkaters()[P.LastTouch].Team == S.Team;
			if (bMechSaucerOverThis) ++MechSaucerOver;
			if (bReceived) ++MechSaucerReceived;
			UE_LOG(LogHokej, Log, TEXT("JAMDRILL saucer n=%d frame=%lld receiver=%d over_blade=%d nearest_blade_air_m=%.2f z_at_blade=%.3f max_z=%.3f landed_frame=%lld land_dist_to_receiver=%.2f received=%d by=%d opp_touch=%d"),
				MechSaucerN, MechSaucerFrame, MechSaucerReceiver, bMechSaucerOverThis ? 1 : 0, MechSaucerMinBladeAir < 1e8 ? MechSaucerMinBladeAir : -1.0, MechSaucerZAtBlade, MechSaucerMaxZ, MechSaucerLand, MechSaucerLandDist,
				bReceived ? 1 : 0, bTouched ? P.LastTouch : -1, (bTouched && !bReceived) ? 1 : 0);
			MechSaucerFrame = -1;
			if (MechSaucerN % 10 == 0) UE_LOG(LogHokej, Log, TEXT("JAMDRILL saucer_summary n=%d over_blade=%d received=%d"), MechSaucerN, MechSaucerOver, MechSaucerReceived);
		}
	}
}

void AHokejGameMode::KdObserve()
{
	// Lane A73 drill 6: the human's knockdowns (the bus event) and get-ups (the IsDown edge the pawn turns into SetKnockedDown(false)).
	if (!Sim) return;
	const int32 Human = Sim->GetHumanIndex();
	if (!Sim->GetSkaters().IsValidIndex(Human)) return;
	const FJamSkaterState& H = Sim->GetSkaters()[Human];
	for (const FJamSimEvent& E : Sim->GetFrameEvents())
	{
		if (E.Name == JamEvents::Knockdown && E.Who == Human)
		{
			++KdCount;
			KdDownFrame = Sim->GetFrame();
			UE_LOG(LogHokej, Log, TEXT("JAMDRILL kd_human n=%d frame=%lld kind=%s strength=%.2f down_frames=%d dir=(%.2f, %.2f)"), KdCount, KdDownFrame, *E.Kind.ToString(), E.Strength, H.KnockFrames, H.KnockDir.X, H.KnockDir.Y);
		}
	}
	if (bKdWasDown && !H.IsDown())
	{
		++KdGetUps;
		UE_LOG(LogHokej, Log, TEXT("JAMDRILL getup_human n=%d frame=%lld measured_down_frames=%lld"), KdGetUps, Sim->GetFrame(), KdDownFrame >= 0 ? Sim->GetFrame() - KdDownFrame : -1);
		if (KdGetUps >= 10 && CVarJamCtrlDrill.GetValueOnGameThread() == 6)
		{
			UE_LOG(LogHokej, Log, TEXT("JAMDRILL kd_summary mode=6 knockdowns=%d getups=%d"), KdCount, KdGetUps);
			CVarJamCtrlDrill->Set(0, ECVF_SetByCode);
		}
	}
	bKdWasDown = H.IsDown();
}
