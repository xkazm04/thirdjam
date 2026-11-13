// Lane A9: the lockstep state machine. Owns nothing but inputs and hashes; the sim and the link belong to the caller.
//
// Per engine tick the game mode calls Pump() once (receive, then send the window of the last 8 local inputs and the latest
// hash checkpoint), then TryStep() for each sim frame the accumulator allows. TryStep stores the freshly read local input
// for frame Frame + Delay and succeeds only when the remote input for Frame is present; both inputs come back quantised,
// which is what the sim must consume on both sides. After the sim stepped a frame that is a multiple of HashInterval the
// caller passes the state hash to RecordHash(); a remote checkpoint that disagrees sets bDesynced and the match ends.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "JamSim.h"
#include "JamNetPackets.h"
#include "JamLockstepDriver.generated.h"

class IJamLink;

UCLASS()
class JAMNET_API UJamLockstepDriver : public UObject
{
	GENERATED_BODY()

public:
	void Start(IJamLink* InLink, int32 InDelay = JamNet::InputDelay);
	void Stop() { Link = nullptr; }

	// Once per engine tick: receive everything waiting, check hashes, send our window (also while stalled).
	void Pump();

	// One sim frame if the remote input for it is here. Local is this tick's controls (applied at Frame + Delay).
	bool TryStep(const FJamInput& Local, FJamInput& OutLocal, FJamInput& OutRemote);

	// Call after the sim stepped to SimFrame; ignored unless SimFrame is a checkpoint.
	void RecordHash(int64 SimFrame, uint64 Hash);

	int64 GetFrame() const { return Frame; }
	int32 GetDelay() const { return Delay; }
	bool IsDesynced() const { return bDesynced; }
	bool IsLost() const { return bLost; }
	bool HasHeardPeer() const { return bHeard; }
	int32 GetAgreedCheckpoints() const { return Agreed; }
	int64 GetDesyncFrame() const { return DesyncFrame; }
	int32 GetStallTicks() const { return StallTicks; }
	int32 GetMaxStallTicks() const { return MaxStallTicks; }
	int64 GetRemoteFrame() const { return RemoteFrame; }
	int32 GetMaxFrameGap() const { return MaxFrameGap; }
	int64 GetTicks() const { return Ticks; }
	// Bytes per second each way at 60 ticks/s, from the link's counters.
	FString DescribeStats() const;

private:
	void Receive();
	void Send();
	void CheckHashes();
	void Prune();

	IJamLink* Link = nullptr;
	int32 Delay = JamNet::InputDelay;
	int64 Frame = 0;         // frames stepped so far = the next frame to step
	int64 LastPumpFrame = -1;
	int64 Ticks = 0;
	TMap<int64, FJamNetQuantInput> LocalInputs;
	TMap<int64, FJamNetQuantInput> RemoteInputs;
	TMap<int64, uint64> LocalHashes;
	TMap<int64, uint64> RemoteHashes;
	int64 LatestLocalHashFrame = -1;
	int64 LastCheckedHashFrame = -1;
	// Lane A56: the flags word's own type. It was uint8, so a spin (flag 512, protocol 5) pressed during a stall was cut off and never reached either sim
	// (Bar.Net.SoakLossy: 49 of 1636 spins lost at 30 % loss, 137 of 1727 on a 0..6-tick jitter, 200 seeds each).
	decltype(FJamNetQuantInput::Flags) PendingFlags = 0;  // edge presses read while stalled are not lost
	int64 RemoteFrame = 0;
	int32 MaxFrameGap = 0;
	int32 StallTicks = 0;
	int32 MaxStallTicks = 0;
	int32 Agreed = 0;
	int64 DesyncFrame = -1;
	bool bDesynced = false;
	bool bLost = false;
	bool bHeard = false;
};
