// Lane A9: the match side of "Play with a friend". Rink.umap is opened with ?game=/Script/JamNet.LobbyGameMode by
// UJamLobbySubsystem; this subclass builds the match exactly as AHokejGameMode does (Super::StartPlay), then takes over the
// stepping: each engine tick it pumps the lockstep driver and advances the sim only for frames whose remote input is present.
// The host drives the cartridge's human, the guest drives team 1's first non-back skater (UJamLobbySubsystem::GetGuestIndex).
// With no lobby session active it behaves exactly like AHokejGameMode (single player).
//
// AHokejGameMode keeps StepOnce / GatherInputs / DeliverEvents / PushPresentation / ReadHumanInput private, so this class
// re-implements the step loop with the public API (seam request for lane A1 in the A9 report: make them protected virtual).
// Lane A38 (H11.5): the re-implementation now mirrors the H11 step loop: every FJamInput field for the humans and the bots, the brain
// from UJamBrain::ApplyCartridge (difficulty profiles), the rule system's score mirror, the zone system's update, the effect system's
// step (pickups), the effects / zones / announce events, the rink rebuild on ShrinkRink, the faceoff countdown on the HUD. The
// checkpoint hash is UJamSim::StateHash ^ UJamEffectSystem::StateHash * 31 (the effects test's formula), so a pickup on the ice that
// differs between the machines is a desync too. What does not travel: the team signals (keys 1-4, UJamSignalSystem) are not read in a
// lockstep match on either side (the brain sees no intent), so they cannot desync; a Director's Call still changes one side only (A9's note).
#pragma once

#include "CoreMinimal.h"
#include "HokejGameMode.h"
#include "JamNetPackets.h"
#include "JamPlaytester.h"
#include "LobbyGameMode.generated.h"

class UJamLobbySubsystem;
class UJamLockstepDriver;
class UJamBrain;
class AJamSkater;
class AJamPuck;
class AJamPickupActor;
class ARinkActor;
class ULobbyCardWidget;

UCLASS()
class JAMNET_API ALobbyGameMode : public AHokejGameMode
{
	GENERATED_BODY()

public:
	ALobbyGameMode();
	virtual void StartPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	// Lane A38: leaving the rink any way (Esc -> menu, quit) leaves the lobby too, so the socket is freed and the peer gets a BYE.
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

	UFUNCTION(BlueprintPure, Category = "Jam|Lobby") bool IsLockstep() const { return bLockstep; }
	UFUNCTION(BlueprintPure, Category = "Jam|Lobby") int32 GetLocalIndex() const { return LocalIndex; }
	// Lane A120 (track Q1): barks, commentary and the HUD speak to this machine's seat: the guest's on the guest's machine (presentation only).
	virtual int32 GetLocalSeat() const override { return LocalIndex >= 0 ? LocalIndex : Super::GetLocalSeat(); }
	static int32 LocalSeatOf(bool bHost, int32 HumanIndex, int32 GuestIndex) { return bHost ? HumanIndex : GuestIndex; }

	// Lane A129 (track Q1): one machine's sim never changes alone. In a lockstep match a live cut (the Designer's, a Director's Call, jam.livecut), a cut
	// load and a restart are refused with a line on screen; the rematch from the card is the lockstep one (both seats ask, both machines restart).
	virtual bool RefuseSimChange(const TCHAR* What) override;
	// Lane A204 (track N1, FAULT 4): in a lockstep match the world never pauses (the Director's ruling: the pause card opens over a match that goes on). A paused
	// world stopped this machine's lockstep and the other machine called the connection lost 3 s later. Every pause goes through here (the pause card's
	// UGameplayStatics::SetGamePaused, the engine's focus-loss pause): refused while bLockstep; single player and the cards after the match pause as before.
	virtual bool AllowPausing(APlayerController* PC = nullptr) override;
	static bool PauseAllowedOnline(bool bInLockstep) { return !bInLockstep; }
	// Lane A204: while this machine's pause card is open its seat sends neutral input (no move, no press: the skater coasts); the lockstep keeps sending.
	static bool LocalSeatNeutral(bool bPauseCardOpen, EJamSimPhase Phase) { return bPauseCardOpen && Phase != EJamSimPhase::Over; }
	// Test hook (Jam.Net.PauseCardOnline): the lockstep flag as StartPlay sets it, without a lobby.
	void SetLockstepForTest(bool bOn) { bLockstep = bOn; }
	// Test hook (Jam.Net.EndCardStandsStill): the state EndMatch leaves a joined match in, without a lobby (the lockstep off, the online end set).
	void EndOnlineForTest() { bEndedOnline = true; bLockstep = false; }
	bool HasEndedOnline() const { return bEndedOnline; }
	static bool LiveChangeRefused(bool bInLockstep, const TCHAR* What, FString& OutLine);
	// Lane A129: the frame both seats' applied inputs ask for the rematch (JamNet::IsRematchReady) while the sim is Over.
	static bool RematchAgreed(EJamSimPhase Phase, const FJamInput& L, const FJamInput& R) { return Phase == EJamSimPhase::Over && JamNet::IsRematchReady(L) && JamNet::IsRematchReady(R); }
	UFUNCTION() void OnRematchPressed();
	UFUNCTION(BlueprintPure, Category = "Jam|Lobby") int32 GetMatchNumber() const { return MatchNumber; }

	// Lane A38: what happened in this match so far (the proof asserts a faceoff, a foul, a pickup and a rule hook occurred).
	UFUNCTION(BlueprintPure, Category = "Jam|Lobby") int32 GetFaceoffDrops() const { return FaceoffDrops; }
	UFUNCTION(BlueprintPure, Category = "Jam|Lobby") int32 GetFouls() const { return Fouls; }
	UFUNCTION(BlueprintPure, Category = "Jam|Lobby") int32 GetPickupTakes() const { return PickupTakes; }

	// Lane A110 (track Q1): the local stick in AHokejGameMode::ReadHumanInput's order: the right stick (absolute, camera-relative) first; then the cursor on the
	// ice (bCursor true: CursorStick, rink-exact, from CursorAimRink); then the mouse delta in pixels (100 px = one reach at Sensitivity 1). Yaw = the camera's.
	static FVector2D ComposeLocalStick(const FVector2D& PadAim, bool bCursor, const FVector2D& CursorStick, const FVector2D& MouseDeltaPx, double Sensitivity, double YawRad);

private:
	bool StepLockstep();
	void GatherBotInputs();
	void BuildBrainWorld(JamAI::FJamAIWorld& W) const;   // lane A129: the brain's and the scripted playtester's view (pickups, slow ice, no-shoot zones)
	void RestartLockstepMatch();
	void ShowLine(const FString& Line);
public:
	// Lane A204: a line a player must read, on every HUD overlay of the world (UJamDirectorsCallHudWidget's announcement line) until SimFrame + LineFrames
	// (the sim stands still on the result card: the line stays there). Returns the number of HUD widgets that got it (0: none in this world yet).
	static int32 RouteLineToHud(const UObject* WorldContext, const FString& Line, int64 SimFrame);
	static constexpr int64 LineFrames = 360;
	// Lane A204: the line under every end card's reason: the way back (Esc opens the pause card once the match is over; QUIT is the menu).
	static FString EndCardSubtitle(const FString& Reason) { return Reason + TEXT("   -   ESC, then QUIT: back to the menu"); }
private:
	FJamInput ReadLocalInput();
	uint64 LockstepHash() const;
	void CountProofEvents();
	void DeliverSimEvents();
	void PushLockstepPresentation();
	void PossessLocal(APlayerController* PC);
	void EndMatch(const FString& Title, const FString& Subtitle, const FString& Reason);
	void LogProofLine(const TCHAR* Tag) const;
	AJamSkater* SkaterActor(int32 SimIndex) const;

	UPROPERTY(Transient) TObjectPtr<UJamLobbySubsystem> Lobby;
	UPROPERTY(Transient) TObjectPtr<UJamLockstepDriver> Driver;
	UPROPERTY(Transient) TObjectPtr<UJamBrain> LockBrain;
	UPROPERTY(Transient) TArray<TObjectPtr<AJamSkater>> SkaterActors;
	UPROPERTY(Transient) TObjectPtr<AJamPuck> PuckActor;
	UPROPERTY(Transient) TArray<TObjectPtr<AJamPickupActor>> LockPickupActors;
	UPROPERTY(Transient) TObjectPtr<ARinkActor> LockRink;
	UPROPERTY(Transient) TObjectPtr<ULobbyCardWidget> Card;

	TArray<FJamInput> LockInputs;
	FJamNetScriptedHuman Script;
	// Lane A129: jam.net.ScriptPlayer novice|regular|good: the scripted seat is the playtester of that class (it shoots on goal); empty = A38/A120's script.
	JamAI::FJamPlaytester Tester;
	bool bTesterSeat = false;
	bool bLocalRematch = false;      // this machine's seat asks for the rematch (R on the card, or the scripted seat)
	int64 OverAtDriverFrame = -1;    // the driver frame the sim reached Over (the scripted seat asks 120 frames later)
	int32 MatchNumber = 1;
	// Lane A104: AHokejGameMode's A93 back-pull latch (ReadLocalInput): the camera yaw when the move input began to pull back.
	bool bBackYawHeld = false;
	double BackYawRad = 0.0;
	int32 LocalIndex = -1;
	int32 RemoteIndex = -1;
	int32 LockRinkRevision = 0;
	int32 FaceoffDrops = 0;
	int32 Fouls = 0;
	int32 PickupTakes = 0;
	double LockAccumulator = 0.0;
	double FirstFrameAt = 0.0;   // FPlatformTime::Seconds() of the first stepped frame (0 = none yet)
	double LastFrameAt = 0.0;    // lane A120: of the latest stepped frame (the seconds from the peer's last frame to the card)
	// Lane A120: frames each seat's applied input held a verb: [0] host seat, [1] guest seat; sprint, pass held, brake, spin, check, ability.
	int32 VerbFrames[2][6] = {};
	bool bLockstep = false;
	bool bCardNeutral = false;   // lane A204: the pause card was open on the last local read (for the open / closed log lines)
	bool bEnded = false;
	// Lane A204 (track N1, evolution 3): a joined match ended (desync, the peer left or was lost): from that frame the sim stands still under the end card
	// (before, Tick fell through to AHokejGameMode::Tick and the match played on as single player on each machine). EndFrame / EndHash: what it stands at.
	bool bEndedOnline = false;
	int64 EndFrame = -1;
	uint64 EndHash = 0;
	int32 EndStillTicks = 0;
	bool bShotTaken = false;
	bool bQuitRequested = false;
	bool bExitSent = false;
	int64 QuitAtTick = 0;
};
