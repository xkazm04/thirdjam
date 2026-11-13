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
#include "JamEffectSystem.h"
#include "JamPickupActor.h"
#include "JamRuleSystem.h"
#include "JamZones.h"
#include "RinkActor.h"
#include "HUDFeed.h"
#include "InputMapping.h"
#include "CameraFollow.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"
#include "UnrealClient.h"

static TAutoConsoleVariable<int32> CVarJamNetAutoShot(TEXT("jam.net.AutoShot"), 0,
	TEXT("1: a lockstep match saves a screenshot at frame 600 (the two-process proof)."));
// Lane A38: the unattended two-process proof. Both processes quit on their own once the sim reaches this frame (or the match is lost / desynced).
static TAutoConsoleVariable<int32> CVarJamNetQuitAtFrame(TEXT("jam.net.QuitAtFrame"), 0,
	TEXT("> 0: a lockstep match logs its proof line and quits the process when the sim reaches this frame (headless two-process proof)."));

// Lane A120 (track Q1): the scripted human (jam.ScriptHuman 1) also sprints, holds the saucer, brakes, spins and presses F (FJamNetScriptedHuman::bAllVerbs).
static TAutoConsoleVariable<int32> CVarJamNetScriptVerbs(TEXT("jam.net.ScriptVerbs"), 0,
	TEXT("1: the scripted lockstep human uses every verb (sprint, saucer, brake, spin, check, F); 0: A38's script (poke, pass, shot, check)."));

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
	LocalIndex = LocalSeatOf(Lobby->IsHost(), Human, Guest);
	RemoteIndex = LocalSeatOf(!Lobby->IsHost(), Human, Guest);
	Script.bAllVerbs = CVarJamNetScriptVerbs.GetValueOnGameThread() != 0;
	// Lane A110 (track Q1): the guest's seat plays as a human on both machines, from frame 0 (the mode's ability, a pickup's, the human's check rules, no bot
	// press at the draw). Both peers seat the same index (WELCOME's), so the sims stay equal; the checkpoint hashes the seat.
	if (!TheSim->SeatGuest(Guest)) UE_LOG(LogJamNet, Warning, TEXT("JAMNET could not seat the guest at skater %d (frame %lld)"), Guest, TheSim->GetFrame());
	LockInputs.SetNum(TheSim->GetSkaters().Num());

	// Lane A38: the brain as single player builds it (R-0009 point 3: difficulty profile -> game.ai -> Designer values).
	LockBrain = NewObject<UJamBrain>(this);
	LockBrain->ApplyCartridge(*GetCartridge());
	LockBrain->ResetBrain();

	SkaterActors.SetNum(TheSim->GetSkaters().Num());
	for (TActorIterator<AJamSkater> It(GetWorld()); It; ++It) if (SkaterActors.IsValidIndex(It->GetSimIndex())) SkaterActors[It->GetSimIndex()] = *It;
	for (TActorIterator<AJamPuck> It(GetWorld()); It; ++It) PuckActor = *It;
	for (TActorIterator<ARinkActor> It(GetWorld()); It; ++It) LockRink = *It;
	LockRinkRevision = TheSim->GetRinkRevision();

	// R would restart only this machine's sim: unbind the base game mode's restart for the match.
	if (AJamSkater* H = SkaterActor(Human)) if (UJamPlayerInput* Pi = H->FindComponentByClass<UJamPlayerInput>()) Pi->OnRestart.RemoveAll(this);

	bLockstep = true;
	LockAccumulator = 0.0;
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController()) PossessLocal(PC);
	PushLockstepPresentation();
	UE_LOG(LogJamNet, Log, TEXT("JAMNET match start role=%s local=%d remote=%d delay=%d cut=%s seed=%llu hash0=%016llx packet_version=%d hooks=%d effects=%d zones=%d"),
		Lobby->IsHost() ? TEXT("host") : TEXT("guest"), LocalIndex, RemoteIndex, Driver->GetDelay(), *Lobby->GetCut(), static_cast<unsigned long long>(TheSim->GetSeed()),
		static_cast<unsigned long long>(LockstepHash()), JamNet::Version, GetRules() ? GetRules()->GetHookCount() : 0, GetEffects() ? 1 : 0, GetZones() ? GetZones()->GetZoneCount() : 0);
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (bLockstep) PossessLocal(NewPlayer);
}

void ALobbyGameMode::EndPlay(const EEndPlayReason::Type Reason)
{
	if (Lobby && (Lobby->GetState() == EJamLobbyState::InMatch || Lobby->GetState() == EJamLobbyState::Ended))
	{
		UE_LOG(LogJamNet, Log, TEXT("JAMNET leaving the rink (reason %d): lobby left, socket closed"), static_cast<int32>(Reason));
		Lobby->Leave();
	}
	bLockstep = false;
	Super::EndPlay(Reason);
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

// Mirror of AHokejGameMode::ReadHumanInput (private there) for the local seat: every FJamInput field.
FJamInput ALobbyGameMode::ReadLocalInput()
{
	static IConsoleVariable* ScriptHuman = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.ScriptHuman"));
	if (bScriptedHuman || (ScriptHuman && ScriptHuman->GetInt() != 0)) return Script.Think(*GetSim(), LocalIndex);
	FJamInput In;
	AJamSkater* Mine = SkaterActor(LocalIndex);
	UJamPlayerInput* Pi = Mine ? Mine->FindComponentByClass<UJamPlayerInput>() : nullptr;
	if (!Pi) return In;
	double Yaw = GetSim()->GetSkaters()[LocalIndex].Yaw;
	if (APlayerController* PC = Cast<APlayerController>(Mine->GetController())) if (PC->PlayerCameraManager) Yaw = FMath::DegreesToRadians(PC->PlayerCameraManager->GetCameraRotation().Yaw);
	const FVector2D Axis = Pi->GetAxis();
	// Lane A104: AHokejGameMode::ReadHumanInput's A93 latch (R-0045 law 3). While the move input pulls back, it is read in the camera's frame of the moment
	// the pull began, so S is one direction on the ice and the reverse brake stops the skater; read from the live camera (which swings behind the turning
	// skater) a held S circled online.
	double MoveYaw = Yaw;
	if (Axis.Y < -0.3)
	{
		if (!bBackYawHeld) { bBackYawHeld = true; BackYawRad = Yaw; }
		MoveYaw = BackYawRad;
	}
	else bBackYawHeld = false;
	In.Move = FVector2D(FMath::Cos(MoveYaw), FMath::Sin(MoveYaw)) * Axis.Y + FVector2D(-FMath::Sin(MoveYaw), FMath::Cos(MoveYaw)) * Axis.X;
	// C-0005 charged shot: the held level travels; the sim charges while the puck is in reach and fires on release.
	Pi->ConsumeShootPressed();
	In.bShootHeld = Pi->IsShootHeld();
	// Lane A104: the pass is a held level as in AHokejGameMode::ReadHumanInput (lane A77, R-0036): the sim fires it on the release, a saucer on a long hold.
	// Protocol 6 carried the level, but the lobby sent the press as the bPass edge: online, a human's pass was always flat and Q never made a saucer.
	Pi->ConsumePassPressed();
	In.bPassHeld = Pi->IsPassHeld();
	In.bCheck = Pi->ConsumeCheckPressed();
	In.bBrake = Pi->IsBrakeHeld();
	// R-0013 + lane A49, as AHokejGameMode::ReadHumanInput: the right stick, then the cursor on the ice (the blade points at the cursor), then the mouse
	// delta. Lane A110: before, online the stick followed the mouse delta only (CursorAimRink was private), so the guest's and the host's blade never sat
	// under the cursor. The wire's quantised aim is unchanged.
	APlayerController* PC = Cast<APlayerController>(Mine->GetController());
	EnsureMatchInputMode(PC);
	const FVector2D Pad = Pi->GetStickAxis();
	FVector2D CursorStick = FVector2D::ZeroVector;
	const bool bCursor = Pad.SizeSquared() < 0.01 && CursorAimRink(PC, GetSim()->GetSkaters()[LocalIndex].Pos, CursorStick);
	if (bCursor) Pi->ResetMouseAim();
	In.Stick = ComposeLocalStick(Pad, bCursor, CursorStick, bCursor ? FVector2D::ZeroVector : Pi->GetMouseAimPx(), GetSim()->GetConfig().Stick.MouseSensitivity, Yaw);
	In.bPoke = Pi->ConsumePokePressed();
	In.bLift = Pi->ConsumeLiftPressed();
	In.bDeke = Pi->ConsumeDekePressed();   // lane A39 (R-0029 point 4)
	In.bSprint = Pi->IsSprintHeld();        // R-0037 (A56 found them dead online): the sprint and the spin reach the lockstep driver
	In.bSpin = Pi->ConsumeSpinPressed();
	In.bAbility = Pi->ConsumeAbilityPressed();   // lane A84 (R-0041 point 3): F / R3
	return In;
}

FVector2D ALobbyGameMode::ComposeLocalStick(const FVector2D& PadAim, bool bCursor, const FVector2D& CursorStick, const FVector2D& MouseDeltaPx, double Sensitivity, double YawRad)
{
	const FVector2D Fwd(FMath::Cos(YawRad), FMath::Sin(YawRad));
	const FVector2D Right(-FMath::Sin(YawRad), FMath::Cos(YawRad));
	if (PadAim.SizeSquared() >= 0.01) return Fwd * PadAim.Y + Right * PadAim.X;
	if (bCursor) return CursorStick;
	FVector2D Aim = MouseDeltaPx * (0.01 * Sensitivity);
	if (Aim.SizeSquared() > 1.0) Aim.Normalize();
	return Aim.SizeSquared() >= 0.0025 ? Fwd * Aim.Y + Right * Aim.X : FVector2D::ZeroVector;   // AHokejGameMode's dead zone for the delta
}

// Mirror of AHokejGameMode::GatherInputs for the bot slots (the team signals are not read: see the header).
void ALobbyGameMode::GatherBotInputs()
{
	UJamSim* TheSim = GetSim();
	JamAI::FJamAIWorld W;
	JamAI::FJamSimAdapter::BuildWorld(*TheSim, W);
	if (UJamEffectSystem* Fx = GetEffects())
	{
		W.NumPickups = 0;
		for (const FJamPickupOnIce& P : Fx->GetPickups())
		{
			if (W.NumPickups >= JamAI::FJamAIWorld::kMaxPickups) break;
			W.PickupKind[W.NumPickups] = P.PickupIndex;
			W.PickupPos[W.NumPickups] = JamAI::FVec2(P.Pos.X, P.Pos.Y);
			++W.NumPickups;
		}
	}
	if (UJamZoneSystem* Z = GetZones())
	{
		for (const FJamZoneConfig& Zc : Z->GetZones())
		{
			if (Zc.Kind != FName(TEXT("slow_ice")) || W.NumSlowIce >= JamAI::FJamAIWorld::kMaxSlowIce) continue;
			const bool bRect = Zc.Shape == FName(TEXT("rect"));
			W.SlowIceCentre[W.NumSlowIce] = JamAI::FVec2(Zc.X, Zc.Z);
			W.SlowIceRadius[W.NumSlowIce] = bRect ? 0.5 * FMath::Sqrt(Zc.W * Zc.W + Zc.D * Zc.D) : Zc.R;
			++W.NumSlowIce;
		}
		for (int32 i = 0; i < W.NumSkaters; ++i)
		{
			FName ZoneId;
			W.Skaters[i].bNoShoot = Z->BlocksShot(FVector2D(W.Skaters[i].Pos.X, W.Skaters[i].Pos.Y), ZoneId);
		}
	}
	JamAI::FJamAIInput Out[JamAI::kMaxSkaters];
	LockBrain->Think(W, Out);
	for (int32 i = 0; i < LockInputs.Num() && i < JamAI::kMaxSkaters; ++i)
	{
		if (i == W.PlayerIndex) continue;
		FJamInput& I = LockInputs[i];
		I.Move = FVector2D(Out[i].Move.X, Out[i].Move.Y);
		I.bShoot = Out[i].bShoot;
		I.bCheck = Out[i].bCheck;
		I.bBrake = Out[i].bBrake;
		I.bPass = Out[i].bPass;
		I.Stick = FVector2D(Out[i].Stick.X, Out[i].Stick.Y);
		I.bPoke = Out[i].bPoke;
		I.bLift = Out[i].bLift;
		I.bDeke = Out[i].bDeke;   // lane A39
		I.bSprint = Out[i].bSprint;   // lane A72 (ask A2): game.ai.sprint
		// Lane A104: every field the brain sends, as FJamSimAdapter::Step copies it (the headless game the curve measures). Before, the lobby forced
		// bShootHeld false and dropped bPassHeld: online bots never charged a shot (lane A31) nor lifted a saucer (lane A77).
		I.bShootHeld = Out[i].bShootHeld;
		I.bPassHeld = Out[i].bPassHeld;
		I.bSpin = Out[i].bSpin;
		I.bAbility = Out[i].bAbility;
	}
}

uint64 ALobbyGameMode::LockstepHash() const
{
	return JamNetLockstepHash(GetSim(), GetEffects(), GetRules(), GetZones());   // lane A104: protocol 8's checkpoint
}

void ALobbyGameMode::CountProofEvents()
{
	for (const FJamSimEvent& E : GetSim()->GetFrameEvents())
	{
		if (E.Name == JamEvents::Faceoff && E.Kind == FName(TEXT("drop"))) ++FaceoffDrops;
		else if (E.Name == JamEvents::Foul) ++Fouls;
	}
	if (const UJamEffectSystem* Fx = GetEffects()) for (const FJamSimEvent& E : Fx->GetFrameEvents()) if (E.Name == JamEvents::PickupTake) ++PickupTakes;
}

void ALobbyGameMode::LogProofLine(const TCHAR* Tag) const
{
	const UJamSim* TheSim = GetSim();
	if (!TheSim || !Driver) return;
	// Lane A120: each seat's verbs as applied (the same on both machines) and the sim's own counters for the seat.
	FString Seats;
	const int32 SeatIdx[2] = { Lobby && Lobby->IsHost() ? LocalIndex : RemoteIndex, Lobby && Lobby->IsHost() ? RemoteIndex : LocalIndex };
	for (int32 s = 0; s < 2; ++s)
	{
		const FJamSkaterState* K = TheSim->GetSkaters().IsValidIndex(SeatIdx[s]) ? &TheSim->GetSkaters()[SeatIdx[s]] : nullptr;
		Seats += FString::Printf(TEXT(" %s=[seat %d sprint %d pass_held %d brake %d spin %d check %d ability %d | saucers %d spins %d abilities_used %d]"), s ? TEXT("guest") : TEXT("host"), SeatIdx[s],
			VerbFrames[s][0], VerbFrames[s][1], VerbFrames[s][2], VerbFrames[s][3], VerbFrames[s][4], VerbFrames[s][5], K ? K->Saucers : -1, K ? K->Spins : -1, K ? K->AbilitiesUsed : -1);
	}
	UE_LOG(LogJamNet, Log, TEXT("JAMNET %s frame=%lld hash=%016llx sim_hash=%s score=%d:%d phase=%d faceoffs=%d fouls=%d pickups=%d rule_fires=%d first_frame_s=%.2f %s%s"), Tag,
		TheSim->GetFrame(), static_cast<unsigned long long>(LockstepHash()), *TheSim->StateHashHex(), TheSim->GetScore(0), TheSim->GetScore(1), static_cast<int32>(TheSim->GetPhase()),
		FaceoffDrops, Fouls, PickupTakes, GetRules() ? GetRules()->GetTotalFires() : 0,
		(FirstFrameAt > 0.0 && Lobby && Lobby->GetJoinedAtSeconds() > 0.0) ? FirstFrameAt - Lobby->GetJoinedAtSeconds() : -1.0, *Driver->DescribeStats(), *Seats);
}

bool ALobbyGameMode::StepLockstep()
{
	UJamSim* TheSim = GetSim();
	FJamInput L, R;
	if (!Driver->TryStep(ReadLocalInput(), L, R)) return false;
	if (FirstFrameAt <= 0.0)
	{
		FirstFrameAt = FPlatformTime::Seconds();
		UE_LOG(LogJamNet, Log, TEXT("JAMNET first frame %.2f s after the %s (invite code to first frame)"), Lobby->GetJoinedAtSeconds() > 0.0 ? FirstFrameAt - Lobby->GetJoinedAtSeconds() : -1.0,
			Lobby->IsHost() ? TEXT("guest's HELLO arrived") : TEXT("join was typed"));
	}
	LastFrameAt = FPlatformTime::Seconds();
	GatherBotInputs();
	LockInputs[LocalIndex] = L;
	LockInputs[RemoteIndex] = R;
	for (int32 s = 0; s < 2; ++s)
	{
		const FJamInput& V = ((s == 0) == Lobby->IsHost()) ? L : R;   // [0] the host's seat, [1] the guest's
		VerbFrames[s][0] += V.bSprint; VerbFrames[s][1] += V.bPassHeld; VerbFrames[s][2] += V.bBrake; VerbFrames[s][3] += V.bSpin; VerbFrames[s][4] += V.bCheck; VerbFrames[s][5] += V.bAbility;
	}
	if (UJamRuleSystem* RuleSys = GetRules()) { RuleSys->SetScore(0, TheSim->GetScore(0)); RuleSys->SetScore(1, TheSim->GetScore(1)); }
	TheSim->Step(LockInputs);
	if (UJamZoneSystem* Z = GetZones()) Z->Update(TheSim->GetFrame(), TheSim->GetSkaters(), TheSim->GetPuck());
	if (UJamEffectSystem* Fx = GetEffects()) Fx->Step();
	Driver->RecordHash(TheSim->GetFrame(), LockstepHash());
	CountProofEvents();
	DeliverSimEvents();
	if (LockRink && TheSim->GetRinkRevision() != LockRinkRevision)
	{
		LockRinkRevision = TheSim->GetRinkRevision();
		LockRink->Build(TheSim->GetConfig());
		UE_LOG(LogJamNet, Log, TEXT("JAMNET rink rebuilt at frame %lld: %gx%g m (revision %d)"), TheSim->GetFrame(), TheSim->GetConfig().Rink.LengthM, TheSim->GetConfig().Rink.WidthM, LockRinkRevision);
	}
	for (AJamSkater* A : SkaterActors) if (A) A->ApplySimFrame(TheSim->GetSkaters()[A->GetSimIndex()]);
	PushLockstepPresentation();
	if (TheSim->GetFrame() % 600 == 0)
	{
		LogProofLine(TEXT("frame"));
		if (TheSim->GetFrame() == 600 && !bShotTaken && CVarJamNetAutoShot.GetValueOnGameThread() != 0)
		{
			bShotTaken = true;
			FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("JamNet_%s_f600"), Lobby->IsHost() ? TEXT("host") : TEXT("guest")), true, false);
		}
	}
	const int32 QuitAt = CVarJamNetQuitAtFrame.GetValueOnGameThread();
	if (QuitAt > 0 && TheSim->GetFrame() >= QuitAt && !bQuitRequested)
	{
		// Keep pumping (sending our window) for two seconds before quitting: the peer may be up to Delay frames behind across the link and
		// still needs our inputs for them; quitting at once left it waiting (the chaos 80 ms run at 19:26 lost the guest at frame 8998).
		bQuitRequested = true;
		QuitAtTick = Driver->GetTicks() + 120;
		LogProofLine(TEXT("proof done"));
	}
	return true;
}

// Mirror of AHokejGameMode::DeliverEvents: the sim's, the effect system's (known names only), the zone system's and the rules' announce.
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
	if (const UJamEffectSystem* Fx = GetEffects())
	{
		for (const FJamSimEvent& E : Fx->GetFrameEvents())
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
	if (const UJamZoneSystem* Z = GetZones())
	{
		for (const FJamSimEvent& E : Z->GetFrameEvents())
		{
			FJamEventPayload Pl;
			Pl.Who = E.Who;
			Pl.Where = UJamSim::ToWorld(E.Where);
			Pl.Strength = E.Strength;
			Pl.Kind = E.Kind;
			Bus->Broadcast(E.Name, Pl);
		}
	}
	if (UJamRuleSystem* RuleSys = GetRules())
	{
		TArray<FString> Said;
		RuleSys->TakePendingAnnouncements(Said);
		bool bSimAnnounced = false;
		for (const FJamSimEvent& E : TheSim->GetFrameEvents()) if (E.Name == JamEvents::Announce) bSimAnnounced = true;
		if (Said.Num() && !bSimAnnounced)
		{
			FJamEventPayload Pl;
			Pl.Kind = FName(TEXT("rule"));
			Bus->Broadcast(JamEvents::Announce, Pl);
		}
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
	// The sim's rink, not the cartridge's: ShrinkRink / GrowRink change it at a faceoff (hockey.chaos).
	F.RinkLengthM = TheSim->GetConfig().Rink.LengthM;
	F.RinkWidthM = TheSim->GetConfig().Rink.WidthM;
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
	// Lane A110 (track Q1): the local seat's stamina bar and ability slot, as AHokejGameMode::PushPresentation fills them for the human (A49, A84). Online
	// the HUD showed neither, for the host or the guest: F's slot was hidden though the host held the mode's ability.
	if (TheSim->GetSkaters().IsValidIndex(LocalIndex))
	{
		const FJamSkaterState& H = TheSim->GetSkaters()[LocalIndex];
		F.HumanStamina01 = H.Stamina < 0 ? 1.f : FMath::Clamp((float)H.Stamina / (float)FMath::Max(1, TheSim->StaminaFull()), 0.f, 1.f);
		F.bHumanSprinting = H.bSprinting;
		const FJamAbilityConfig* Ab = TheSim->HeldAbility(H.Index);
		const FName Kind = Ab ? Ab->Kind : (!H.AbilityArmed.IsNone() ? H.AbilityArmed : (H.ShieldFrames > 0 ? FName(TEXT("shield")) : NAME_None));
		if (!Kind.IsNone())
		{
			F.HumanAbility = Kind.ToString().Replace(TEXT("_"), TEXT(" ")).ToUpper();
			const int32 Cool = Ab ? (Ab->CooldownFrames > 0 ? Ab->CooldownFrames : UJamSim::AbilityDefaultCooldown(Kind)) : 1;
			const int64 Left = FMath::Max<int64>(0, H.AbilityReadyFrame - TheSim->GetFrame());
			if (H.ShieldFrames > 0) F.HumanAbilityState = FString::Printf(TEXT("SHIELD %.1fs"), H.ShieldFrames / 60.f);
			else if (!H.AbilityArmed.IsNone()) F.HumanAbilityState = TEXT("ARMED");
			else if (Left > 0) F.HumanAbilityState = FString::Printf(TEXT("%llds"), (Left + 59) / 60);
			else F.HumanAbilityState = TEXT("READY");
			if (H.AbilityCharges > 0) F.HumanAbilityState += FString::Printf(TEXT("  x%d"), H.AbilityCharges);
			F.HumanAbilityReady01 = H.ShieldFrames > 0 ? 1.f : 1.f - FMath::Clamp((float)Left / (float)FMath::Max(1, Cool), 0.f, 1.f);
			F.bHumanAbilityLive = H.ShieldFrames > 0 || !H.AbilityArmed.IsNone();
		}
	}
	F.AttackSign = MyTeam == 0 ? 1 : -1;
	F.NetInsetM = 4.f;
	F.ScoreHome = TheSim->GetScore(MyTeam);
	F.ScoreAway = TheSim->GetScore(1 - MyTeam);
	F.Take = 1;
	F.ScoreToWin = G.Rules.Mode == FName(TEXT("first_to")) ? G.Rules.ScoreToWin : 0;
	F.bHasTimer = G.Rules.Mode == FName(TEXT("timer"));
	F.FramesRemaining = TheSim->GetTimerFrames();
	switch (TheSim->GetPhase())
	{
	case EJamSimPhase::Playing: F.Phase = EJamPhase::Play; break;
	case EJamSimPhase::Faceoff: F.Phase = EJamPhase::Play; F.FaceoffCountdownFrames = TheSim->GetFaceoffCountdown(); break;
	case EJamSimPhase::GoalPause: F.Phase = EJamPhase::GoalPause; break;
	case EJamSimPhase::Over: F.Phase = TheSim->GetWinner() == MyTeam ? EJamPhase::Win : EJamPhase::Lose; break;
	}
	if (G.Teams.Num() == 2) { F.TeamColour0 = G.Teams[0].Colour; F.TeamColour1 = G.Teams[1].Colour; }
	Pres->PushFrame(F);
}

void ALobbyGameMode::EndMatch(const FString& Title, const FString& Subtitle, const FString& Reason)
{
	// The unattended proof quits on any end, also when the match was already over (sudden death ended the chaos match at frame ~1100 and the
	// later "connection lost" then had nothing to show: bEnded was set, so the guest never exited at 19:30).
	if (CVarJamNetQuitAtFrame.GetValueOnGameThread() > 0 && !bExitSent) { bExitSent = true; bQuitRequested = true; UE_LOG(LogJamNet, Log, TEXT("JAMNET quitting: %s (jam.net.QuitAtFrame)"), *Reason); FPlatformMisc::RequestExit(false); }
	if (bEnded) return;
	bEnded = true;
	bLockstep = false;
	UE_LOG(LogJamNet, Warning, TEXT("JAMNET match ended: %s (%s)"), *Reason, Driver ? *Driver->DescribeStats() : TEXT(""));
	LogProofLine(TEXT("ended"));
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
	if (bQuitRequested)
	{
		// Proof done: no more frames, the window keeps going out until the grace ticks are up, then the process exits.
		if (Driver->GetTicks() >= QuitAtTick && !bExitSent) { bExitSent = true; UE_LOG(LogJamNet, Log, TEXT("JAMNET quitting (jam.net.QuitAtFrame)")); FPlatformMisc::RequestExit(false); }
		return;
	}
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
	if (UJamEffectSystem* Fx = GetEffects()) AJamPickupActor::Sync(GetWorld(), Fx, LockPickupActors, static_cast<double>(TheSim->GetFrame()) + Alpha);
	if (UJamPresentation* Pres = UJamPresentation::Get(this)) Pres->SetRenderAlpha(Alpha);

	if (Driver->IsDesynced() && !bEnded)
	{
		EndMatch(TEXT("DESYNC"), FString::Printf(TEXT("the two games disagreed at frame %lld; this cut is over"), Driver->GetDesyncFrame()), FString::Printf(TEXT("desync at frame %lld"), Driver->GetDesyncFrame()));
	}
	else if (Driver->HasPeerLeft() && !bEnded)
	{
		// Lane A120: the peer said BYE: a card at once, not the 3 s stall.
		UE_LOG(LogJamNet, Warning, TEXT("JAMNET card: %s left, %.2f s after the last frame"), *Lobby->GetRemoteName(), LastFrameAt > 0.0 ? FPlatformTime::Seconds() - LastFrameAt : -1.0);
		EndMatch(Lobby->IsHost() ? TEXT("YOUR FRIEND LEFT") : TEXT("THE HOST LEFT"), FString::Printf(TEXT("%s left the match"), *Lobby->GetRemoteName()), TEXT("peer left"));
	}
	else if (Driver->IsLost() && !bEnded)
	{
		UE_LOG(LogJamNet, Warning, TEXT("JAMNET card: connection lost, %.2f s after the last frame"), LastFrameAt > 0.0 ? FPlatformTime::Seconds() - LastFrameAt : -1.0);
		EndMatch(TEXT("CONNECTION LOST"), FString::Printf(TEXT("no word from %s for 3 seconds"), *Lobby->GetRemoteName()), TEXT("connection lost"));
	}
	else if ((Driver->IsLost() || Driver->IsDesynced()) && bEnded && CVarJamNetQuitAtFrame.GetValueOnGameThread() > 0 && !bExitSent)
	{
		// Match already over (win / lose card up) and the peer is gone: the unattended proof still has to exit.
		bExitSent = true; bQuitRequested = true;
		LogProofLine(TEXT("ended"));
		UE_LOG(LogJamNet, Log, TEXT("JAMNET quitting: peer gone after the match ended (jam.net.QuitAtFrame)"));
		FPlatformMisc::RequestExit(false);
	}
	else if (TheSim->GetPhase() == EJamSimPhase::Over && !bEnded)
	{
		// The normal win/lose card is A5's; the lobby only records the result and keeps the link open for a rematch later.
		if (Lobby) Lobby->NotifyMatchEnded(FString::Printf(TEXT("%d:%d, agreed on %d checkpoints"), TheSim->GetScore(0), TheSim->GetScore(1), Driver->GetAgreedCheckpoints()));
		UE_LOG(LogJamNet, Log, TEXT("JAMNET match over frame=%lld winner=%d %s"), TheSim->GetFrame(), TheSim->GetWinner(), *Driver->DescribeStats());
		bEnded = true;
		if (CVarJamNetQuitAtFrame.GetValueOnGameThread() > 0 && !bQuitRequested)
		{
			// Lane A120: the unattended proof plays a match to its end: the final line, then the same two-second grace as at QuitAtFrame.
			bQuitRequested = true;
			QuitAtTick = Driver->GetTicks() + 120;
			LogProofLine(TEXT("proof done"));
		}
	}
}
