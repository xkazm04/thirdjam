#include "JamLink.h"
#include "JamNetPackets.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Interfaces/IPv4/IPv4Address.h"

FJamUdpLink::~FJamUdpLink()
{
	if (Socket)
	{
		ISocketSubsystem* SS = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		Socket->Close();
		if (SS) SS->DestroySocket(Socket);
		Socket = nullptr;
	}
}

int32 FJamUdpLink::Bind(int32 Port)
{
	ISocketSubsystem* SS = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SS) return -1;
	Socket = SS->CreateSocket(NAME_DGram, TEXT("JamNet"), false);
	if (!Socket) return -1;
	Socket->SetNonBlocking(true);
	Socket->SetReuseAddr(false);
	TSharedRef<FInternetAddr> Addr = SS->CreateInternetAddr();
	Addr->SetAnyAddress();
	Addr->SetPort(Port);
	if (!Socket->Bind(*Addr))
	{
		Socket->Close();
		SS->DestroySocket(Socket);
		Socket = nullptr;
		return -1;
	}
	BoundPort = Socket->GetPortNo();
	Scratch.SetNumUninitialized(JamNet::MaxDatagram);
	return BoundPort;
}

void FJamUdpLink::SetPeer(uint32 Ipv4, int32 Port)
{
	ISocketSubsystem* SS = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SS) return;
	Peer = SS->CreateInternetAddr();
	Peer->SetIp(Ipv4);
	Peer->SetPort(Port);
}

FString FJamUdpLink::PeerString() const
{
	return Peer.IsValid() ? Peer->ToString(true) : FString(TEXT("(none)"));
}

void FJamUdpLink::SetImpairment(double DelayMs, double Loss, uint64 Seed)
{
	SimDelayMs = FMath::Max(0.0, DelayMs);
	SimLoss = FMath::Clamp(Loss, 0.0, 1.0);
	SimRng = FJamRandom(Seed);
	SimDropped = 0;
}

void FJamUdpLink::FlushHeld()
{
	if (!Held.Num() || !Socket || !Peer.IsValid()) return;
	const double Now = FPlatformTime::Seconds();
	int32 N = 0;
	while (N < Held.Num() && Held[N].ReadyAt <= Now)
	{
		int32 Sent = 0;
		Socket->SendTo(Held[N].Bytes.GetData(), Held[N].Bytes.Num(), Sent, *Peer);
		++N;
	}
	if (N) Held.RemoveAt(0, N);
}

bool FJamUdpLink::Send(const TArray<uint8>& Bytes)
{
	if (!Socket || !Peer.IsValid()) return false;
	// Counted as offered to the wire: the bandwidth figure is what the game sends; what the (simulated) wire loses is its business.
	BytesSent += Bytes.Num();
	++PacketsSent;
	if (SimLoss > 0.0 && SimRng.NextUnit() < SimLoss) { ++SimDropped; return true; }
	if (SimDelayMs > 0.0)
	{
		Held.Add({ FPlatformTime::Seconds() + SimDelayMs / 1000.0, Bytes });
		FlushHeld();
		return true;
	}
	int32 Sent = 0;
	return Socket->SendTo(Bytes.GetData(), Bytes.Num(), Sent, *Peer);
}

bool FJamUdpLink::RecvFrom(TArray<uint8>& Out, uint32& FromIp, int32& FromPort)
{
	if (!Socket) return false;
	FlushHeld();
	ISocketSubsystem* SS = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	TSharedRef<FInternetAddr> From = SS->CreateInternetAddr();
	int32 Read = 0;
	if (!Socket->RecvFrom(Scratch.GetData(), Scratch.Num(), Read, *From) || Read <= 0) return false;
	Out.SetNumUninitialized(Read);
	FMemory::Memcpy(Out.GetData(), Scratch.GetData(), Read);
	From->GetIp(FromIp);
	FromPort = From->GetPort();
	BytesRecv += Read;
	++PacketsRecv;
	return true;
}

bool FJamUdpLink::Recv(TArray<uint8>& Out)
{
	uint32 Ip = 0; int32 Port = 0;
	while (RecvFrom(Out, Ip, Port))
	{
		if (!Peer.IsValid()) return true;
		uint32 PeerIp = 0; Peer->GetIp(PeerIp);
		if (PeerIp == Ip && Peer->GetPort() == Port) return true;
		// A datagram from someone else: ignored.
	}
	return false;
}

TArray<uint32> FJamUdpLink::LocalAddresses()
{
	TArray<uint32> Out;
	ISocketSubsystem* SS = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SS) return Out;
	bool bCanBind = false;
	TSharedRef<FInternetAddr> Primary = SS->GetLocalHostAddr(*GLog, bCanBind);
	uint32 PrimaryIp = 0;
	if (Primary->IsValid()) { Primary->GetIp(PrimaryIp); if (PrimaryIp != 0 && (PrimaryIp >> 24) != 127) Out.Add(PrimaryIp); }
	TArray<TSharedPtr<FInternetAddr>> All;
	SS->GetLocalAdapterAddresses(All);
	for (const TSharedPtr<FInternetAddr>& A : All)
	{
		if (!A.IsValid() || A->GetProtocolType() != FNetworkProtocolTypes::IPv4) continue;
		uint32 Ip = 0; A->GetIp(Ip);
		if (Ip == 0 || (Ip >> 24) == 127 || (Ip >> 16) == 0xA9FE) continue;  // loopback, link-local
		Out.AddUnique(Ip);
	}
	return Out;
}

FJamLoopbackPair::FJamLoopbackPair(int32 InDelayTicks, double InLoss, uint64 Seed)
	: DelayTicks(InDelayTicks), Loss(InLoss), Rng(Seed)
{
	EndA.Owner = this; EndA.Outbox = &ToB; EndA.Inbox = &ToA;
	EndB.Owner = this; EndB.Outbox = &ToA; EndB.Inbox = &ToB;
}

bool FJamLoopbackPair::FEnd::Send(const TArray<uint8>& Bytes)
{
	BytesSent += Bytes.Num();
	++PacketsSent;
	if (Owner->Loss > 0.0 && Owner->Rng.NextUnit() < Owner->Loss) { ++Owner->Dropped; return true; }
	Outbox->Add({ Owner->Tick + Owner->DelayTicks, Bytes });
	return true;
}

bool FJamLoopbackPair::FEnd::Recv(TArray<uint8>& Out)
{
	for (int32 i = 0; i < Inbox->Num(); ++i)
	{
		if ((*Inbox)[i].ReadyTick <= Owner->Tick)
		{
			Out = MoveTemp((*Inbox)[i].Bytes);
			Inbox->RemoveAt(i);
			BytesRecv += Out.Num();
			++PacketsRecv;
			return true;
		}
	}
	return false;
}
