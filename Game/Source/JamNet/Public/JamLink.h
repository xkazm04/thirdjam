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

	// Every IPv4 of this machine another machine can reach (for the host's code list), in screen order (ReachableAddresses' Ip).
	static TArray<uint32> LocalAddresses();

	// Lane A146 (track Q1): what one of the host's addresses is good for; the host's screen says it next to the code.
	// Lan: an adapter with a default gateway (the router), or a private address on a real adapter ("same network").
	// Tailnet: a 100.64/10 address on an adapter without a gateway (Tailscale's tunnel): reachable from the host's other signed-in machines.
	// ThisMachine: 127/8 (two copies on one machine). Hidden: never listed (virtual switches such as WSL's / Hyper-V's vEthernet, VirtualBox,
	// VMware, Docker; link-local 169.254; a disconnected adapter; 0.0.0.0).
	enum class EAddrKind : uint8 { Lan, Tailnet, ThisMachine, Hidden };
	struct FAddr { uint32 Ip = 0; EAddrKind Kind = EAddrKind::Hidden; bool bGateway = false; };
	// Pure (the test seam): one adapter address. Description = the adapter's description and friendly name as Windows reports them.
	static EAddrKind ClassifyAddress(uint32 Ip, bool bUp, bool bLoopbackAdapter, bool bGateway, const FString& Description);
	// The listed addresses, Hidden and loopback dropped, ordered: Lan with a gateway, Lan without, Tailnet (loopback is the lobby's, added last).
	static TArray<FAddr> ReachableAddresses();
	static void SortForScreen(TArray<FAddr>& InOut);
	static const TCHAR* KindLabel(EAddrKind Kind);

	// Lane A38: a simulated bad link for measurements (console jam.net.SimDelayMs / jam.net.SimLoss, read by the lobby when it opens the
	// socket): every outgoing datagram is held DelayMs before it goes out and a Loss fraction is dropped (seeded). Wall clock on purpose:
	// it imitates the wire, the sim never reads it. 0 / 0 = the real wire. Both peers set it = DelayMs each way.
	void SetImpairment(double DelayMs, double Loss, uint64 Seed);
	int32 GetSimDropped() const { return SimDropped; }
	double GetSimDelayMs() const { return SimDelayMs; }
	double GetSimLoss() const { return SimLoss; }

private:
	void FlushHeld();

	FSocket* Socket = nullptr;
	TSharedPtr<FInternetAddr> Peer;
	int32 BoundPort = -1;
	TArray<uint8> Scratch;
	struct FHeld { double ReadyAt; TArray<uint8> Bytes; };
	TArray<FHeld> Held;
	double SimDelayMs = 0.0;
	double SimLoss = 0.0;
	int32 SimDropped = 0;
	FJamRandom SimRng;
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
