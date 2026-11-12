// Lane A9: the lobby. Host creates (cut, invite code), guest joins (code), both see names and colours, host presses Start,
// both open Rink.umap with ALobbyGameMode. Owns the UDP link and the lockstep driver so they survive the level change.
// Console: jam.lobby (open the screen), jam.host <cut> [name] [auto], jam.join <code|ip:port> [name], jam.start, jam.leave.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "JamLink.h"
#include "JamLobbySubsystem.generated.h"

class UJamLockstepDriver;

UENUM(BlueprintType)
enum class EJamLobbyState : uint8
{
	Idle,
	Hosting,    // socket bound, code shown, waiting for a HELLO
	Joining,    // HELLO sent, waiting for WELCOME
	Ready,      // both sides know each other and the cut agrees; host may Start
	Starting,   // host: START going out, level about to open
	InMatch,    // Rink loaded, the lockstep driver runs
	Ended,      // match over, desync or connection lost (EndReason says which)
	Failed,     // bind failed, bad code, cut mismatch (Error says which)
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FJamLobbyChanged);

UCLASS()
class JAMNET_API UJamLobbySubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	// FTickableGameObject (ticks in game and PIE, not in the editor itself).
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return State != EJamLobbyState::Idle; }
	virtual bool IsTickableInEditor() const override { return false; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UJamLobbySubsystem, STATGROUP_Tickables); }

	UFUNCTION(BlueprintCallable, Category = "Jam|Lobby") bool Host(const FString& Cut, const FString& Name);
	UFUNCTION(BlueprintCallable, Category = "Jam|Lobby") bool Join(const FString& Code, const FString& Name);
	UFUNCTION(BlueprintCallable, Category = "Jam|Lobby") bool StartMatch();
	UFUNCTION(BlueprintCallable, Category = "Jam|Lobby") void Leave();
	// Host: start as soon as the guest is ready (the two-process test).
	UFUNCTION(BlueprintCallable, Category = "Jam|Lobby") void SetAutoStart(bool bOn) { bAutoStart = bOn; }

	UFUNCTION(BlueprintPure, Category = "Jam|Lobby") EJamLobbyState GetState() const { return State; }
	UFUNCTION(BlueprintPure, Category = "Jam|Lobby") bool IsHost() const { return bHost; }
	UFUNCTION(BlueprintPure, Category = "Jam|Lobby") bool IsLockstepMatch() const { return State == EJamLobbyState::InMatch; }
	UFUNCTION(BlueprintPure, Category = "Jam|Lobby") FString GetCut() const { return Cut; }
	UFUNCTION(BlueprintPure, Category = "Jam|Lobby") FString GetLocalName() const { return LocalName; }
	UFUNCTION(BlueprintPure, Category = "Jam|Lobby") FString GetRemoteName() const { return RemoteName; }
	UFUNCTION(BlueprintPure, Category = "Jam|Lobby") int32 GetGuestIndex() const { return GuestIndex; }
	UFUNCTION(BlueprintPure, Category = "Jam|Lobby") FString GetError() const { return Error; }
	UFUNCTION(BlueprintPure, Category = "Jam|Lobby") FString GetEndReason() const { return EndReason; }
	// One line per local address: "ABCD-EFGH  (192.168.1.10)". The first is the default route.
	UFUNCTION(BlueprintPure, Category = "Jam|Lobby") FString GetCodesText() const;
	UFUNCTION(BlueprintPure, Category = "Jam|Lobby") FString GetPrimaryCode() const { return Codes.Num() ? Codes[0] : FString(); }
	UFUNCTION(BlueprintPure, Category = "Jam|Lobby") FString GetStatusText() const;
	UFUNCTION(BlueprintPure, Category = "Jam|Lobby") FString GetWarningText() const;
	UFUNCTION(BlueprintPure, Category = "Jam|Lobby") int32 GetBoundPort() const { return BoundPort; }
	uint64 GetSeed() const { return Seed; }
	// Lane A38: FPlatformTime::Seconds() when the guest sent its first HELLO / when the host heard it (the "invite code to first frame" clock). 0 = not yet.
	double GetJoinedAtSeconds() const { return JoinedAt; }

	UJamLockstepDriver* GetDriver() const { return Driver; }
	IJamLink* GetLink() const;

	// The match side reports how it ended (the subsystem keeps the link open until Leave).
	void NotifyMatchEnded(const FString& Reason);

	UPROPERTY(BlueprintAssignable, Category = "Jam|Lobby") FJamLobbyChanged OnChanged;

	// Grep'd across Game/Source before naming (R-0003 point 4).
	static const TCHAR* ConsoleLobby() { return TEXT("jam.lobby"); }

private:
	void SetState(EJamLobbyState S);
	void Fail(const FString& Why);
	void TickLobby();
	void SendLobby(uint8 Type, int32 Flags = 0);
	void OpenRink();
	bool ComputeCartridgeHash(const FString& InCut, uint64& OutHash, uint64& OutSeed, int32& OutGuestIndex, FString& OutError) const;
	void ResetSession();

	TUniquePtr<FJamUdpLink> Link;
	UPROPERTY(Transient) TObjectPtr<UJamLockstepDriver> Driver;

	EJamLobbyState State = EJamLobbyState::Idle;
	bool bHost = false;
	bool bAutoStart = false;
	bool bGuestAcked = false;
	FString Cut;
	FString LocalName;
	FString RemoteName;
	FString Error;
	FString EndReason;
	TArray<FString> Codes;
	TArray<FString> CodeAddresses;
	int32 BoundPort = -1;
	uint64 CartridgeHash = 0;
	uint64 Seed = 1;
	int32 GuestIndex = -1;
	int64 LobbyTicks = 0;
	int64 StartTicks = 0;
	double JoinedAt = 0.0;
};
