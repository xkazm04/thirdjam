// Lane A9: the datagram link under the lockstep driver. FJamUdpLink is the real one (ISocketSubsystem UDP, non-blocking);
// FJamLoopbackPair is two in-process endpoints with a delay in ticks and a seeded loss rate for the LockstepTwoPeers test.
// The driver sees only IJamLink: Send one datagram, Recv one datagram or false.
#pragma once

#include "CoreMinimal.h"
#include "JamRandom.h"

class FSocket;
class FInternetAddr;

class JAMNET_API IJamLink
{
public:
	virtual ~IJamLink() {}
	virtual bool Send(const TArray<uint8>& Bytes) = 0;
	// One datagram into Out; false when none is waiting.
	virtual bool Recv(TArray<uint8>& Out) = 0;
	virtual bool IsOpen() const = 0;

	int64 BytesSent = 0, BytesRecv = 0, PacketsSent = 0, PacketsRecv = 0;
};

class JAMNET_API FJamUdpLink : public IJamLink
{
public:
	~FJamUdpLink() override;

	// Binds 0.0.0.0:Port (Port 0 = any). Returns the bound port, or -1.
	int32 Bind(int32 Port);
	// Where Send goes (and the only address Recv accepts once set).
	void SetPeer(uint32 Ipv4, int32 Port);
	bool HasPeer() const { return Peer.IsValid(); }
	FString PeerString() const;

	// Lobby side: receive from anyone and learn the sender (the host learns the guest's address this way).
	bool RecvFrom(TArray<uint8>& Out, uint32& FromIp, int32& FromPort);

	bool Send(const TArray<uint8>& Bytes) override;
	bool Recv(TArray<uint8>& Out) override;
	bool IsOpen() const override { return Socket != nullptr; }
	int32 GetBoundPort() const { return BoundPort; }

	// Every non-loopback IPv4 of this machine (for the host's code list); the first is the default-route one when known.
	static TArray<uint32> LocalAddresses();

private:
	FSocket* Socket = nullptr;
	TSharedPtr<FInternetAddr> Peer;
	int32 BoundPort = -1;
	TArray<uint8> Scratch;
};

// Two endpoints joined by a simulated link: a datagram sent at tick T is receivable at tick T + DelayTicks, and is dropped
// with probability Loss (seeded; the test is deterministic). Call Advance() once per simulated tick.
class JAMNET_API FJamLoopbackPair
{
public:
	FJamLoopbackPair(int32 InDelayTicks, double InLoss, uint64 Seed);
	IJamLink& A() { return EndA; }
	IJamLink& B() { return EndB; }
	void Advance() { ++Tick; }
	int32 Dropped = 0;

private:
	struct FQueued { int64 ReadyTick; TArray<uint8> Bytes; };
	struct FEnd : public IJamLink
	{
		FJamLoopbackPair* Owner = nullptr;
		TArray<FQueued>* Outbox = nullptr;  // where our sends go (the other end's inbox)
		TArray<FQueued>* Inbox = nullptr;
		bool Send(const TArray<uint8>& Bytes) override;
		bool Recv(TArray<uint8>& Out) override;
		bool IsOpen() const override { return true; }
	};
	TArray<FQueued> ToA, ToB;
	FEnd EndA, EndB;
	int64 Tick = 0;
	int32 DelayTicks;
	double Loss;
	FJamRandom Rng;
};
