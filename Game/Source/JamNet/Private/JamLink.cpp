#include "JamLink.h"
#include "HAL/IConsoleManager.h"
#include "JamNetPackets.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Interfaces/IPv4/IPv4Address.h"
#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

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

// Lane A120 (track Q1): two processes on one machine bind 127.0.0.1 only (no firewall prompt, nothing reachable from the network).
static TAutoConsoleVariable<int32> CVarJamNetLoopback(TEXT("jam.net.Loopback"), 0, TEXT("1: JamNet sockets bind 127.0.0.1 only (the two-process proof on one machine)."));

int32 FJamUdpLink::Bind(int32 Port)
{
	ISocketSubsystem* SS = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SS) return -1;
	Socket = SS->CreateSocket(NAME_DGram, TEXT("JamNet"), false);
	if (!Socket) return -1;
	Socket->SetNonBlocking(true);
	Socket->SetReuseAddr(false);
	TSharedRef<FInternetAddr> Addr = SS->CreateInternetAddr();
	if (CVarJamNetLoopback.GetValueOnAnyThread() != 0) Addr->SetLoopbackAddress(); else Addr->SetAnyAddress();
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

void FJamUdpLink::CutFor(double Seconds)
{
	CutUntil = FPlatformTime::Seconds() + FMath::Max(0.0, Seconds);
}

bool FJamUdpLink::IsCut() const
{
	return CutUntil > 0.0 && FPlatformTime::Seconds() < CutUntil;
}

bool FJamUdpLink::Send(const TArray<uint8>& Bytes)
{
	if (!Socket || !Peer.IsValid()) return false;
	if (IsCut()) return true;   // lane A204: the cut wire swallows it
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
		if (IsCut()) continue;   // lane A204: arrived while the wire is cut: lost
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
	for (const FAddr& A : ReachableAddresses()) Out.Add(A.Ip);
	return Out;
}

FJamUdpLink::EAddrKind FJamUdpLink::ClassifyAddress(uint32 Ip, bool bUp, bool bLoopbackAdapter, bool bGateway, const FString& Description)
{
	if (Ip == 0 || Ip == 0xFFFFFFFFu) return EAddrKind::Hidden;
	if ((Ip >> 24) == 127) return EAddrKind::ThisMachine;
	if (bLoopbackAdapter) return EAddrKind::Hidden;
	if ((Ip >> 16) == 0xA9FEu) return EAddrKind::Hidden;                      // 169.254/16: link-local, an adapter that got no address
	if (!bUp) return EAddrKind::Hidden;
	if ((Ip >> 28) >= 0xEu) return EAddrKind::Hidden;                         // multicast / reserved
	const bool bCgnat = (Ip >> 22) == ((100u << 2) | 1u);                     // 100.64.0.0/10
	if (bCgnat && (!bGateway || Description.Contains(TEXT("Tailscale")))) return EAddrKind::Tailnet;
	static const TCHAR* Virtual[] = { TEXT("Hyper-V"), TEXT("vEthernet"), TEXT("WSL"), TEXT("VirtualBox"), TEXT("VMware"), TEXT("Docker"), TEXT("Virtual Ethernet"), TEXT("Host-Only"), TEXT("Npcap Loopback") };
	if (!bGateway) for (const TCHAR* V : Virtual) if (Description.Contains(V)) return EAddrKind::Hidden;   // a virtual switch inside this machine
	return EAddrKind::Lan;
}

void FJamUdpLink::SortForScreen(TArray<FAddr>& InOut)
{
	auto Rank = [](const FAddr& A) { return A.Kind == EAddrKind::Lan ? (A.bGateway ? 0 : 1) : A.Kind == EAddrKind::Tailnet ? 2 : A.Kind == EAddrKind::ThisMachine ? 3 : 4; };
	InOut.RemoveAll([](const FAddr& A) { return A.Kind == EAddrKind::Hidden; });
	InOut.StableSort([&Rank](const FAddr& A, const FAddr& B) { return Rank(A) < Rank(B); });
}

const TCHAR* FJamUdpLink::KindLabel(EAddrKind Kind)
{
	switch (Kind)
	{
	case EAddrKind::Lan: return TEXT("same network");
	case EAddrKind::Tailnet: return TEXT("Tailscale, both signed in");
	case EAddrKind::ThisMachine: return TEXT("this machine only");
	default: return TEXT("not reachable");
	}
}

TArray<FJamUdpLink::FAddr> FJamUdpLink::ReachableAddresses()
{
	TArray<FAddr> Out;
	auto AddOne = [&Out](uint32 Ip, EAddrKind Kind, bool bGateway)
	{
		if (Kind == EAddrKind::Hidden || Kind == EAddrKind::ThisMachine) return;   // loopback: the lobby adds it last
		for (const FAddr& A : Out) if (A.Ip == Ip) return;
		FAddr A; A.Ip = Ip; A.Kind = Kind; A.bGateway = bGateway; Out.Add(A);
	};
#if PLATFORM_WINDOWS
	// Windows names the adapter, its state and its gateway; the engine's adapter list gives bare addresses (it listed WSL's switch and missed Tailscale).
	ULONG Size = 16 * 1024;
	TArray<uint8> Buf;
	ULONG Rc = ERROR_BUFFER_OVERFLOW;
	for (int32 Try = 0; Try < 3 && Rc == ERROR_BUFFER_OVERFLOW; ++Try)
	{
		Buf.SetNumZeroed(static_cast<int32>(Size));
		Rc = ::GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_GATEWAYS | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER, nullptr,
			reinterpret_cast<PIP_ADAPTER_ADDRESSES>(Buf.GetData()), &Size);
	}
	if (Rc == NO_ERROR)
	{
		for (PIP_ADAPTER_ADDRESSES Ad = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(Buf.GetData()); Ad; Ad = Ad->Next)
		{
			const bool bUp = Ad->OperStatus == IfOperStatusUp;
			const bool bLoop = Ad->IfType == IF_TYPE_SOFTWARE_LOOPBACK;
			const bool bGateway = Ad->FirstGatewayAddress != nullptr;
			const FString Desc = FString(Ad->Description ? Ad->Description : L"") + TEXT(" | ") + FString(Ad->FriendlyName ? Ad->FriendlyName : L"");
			for (PIP_ADAPTER_UNICAST_ADDRESS U = Ad->FirstUnicastAddress; U; U = U->Next)
			{
				if (!U->Address.lpSockaddr || U->Address.lpSockaddr->sa_family != AF_INET) continue;
				const uint32 Ip = ntohl(reinterpret_cast<const sockaddr_in*>(U->Address.lpSockaddr)->sin_addr.s_addr);
				AddOne(Ip, ClassifyAddress(Ip, bUp, bLoop, bGateway, Desc), bGateway);
			}
		}
		SortForScreen(Out);
		return Out;
	}
#endif
	// Elsewhere (or if Windows' list failed): the engine's addresses, classified by the address alone; the default-route one counts as gatewayed.
	ISocketSubsystem* SS = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SS) return Out;
	bool bCanBind = false;
	TSharedRef<FInternetAddr> Primary = SS->GetLocalHostAddr(*GLog, bCanBind);
	uint32 PrimaryIp = 0;
	if (Primary->IsValid()) { Primary->GetIp(PrimaryIp); AddOne(PrimaryIp, ClassifyAddress(PrimaryIp, true, false, true, FString()), true); }
	TArray<TSharedPtr<FInternetAddr>> All;
	SS->GetLocalAdapterAddresses(All);
	for (const TSharedPtr<FInternetAddr>& A : All)
	{
		if (!A.IsValid() || A->GetProtocolType() != FNetworkProtocolTypes::IPv4) continue;
		uint32 Ip = 0; A->GetIp(Ip);
		AddOne(Ip, ClassifyAddress(Ip, true, false, false, FString()), false);
	}
	SortForScreen(Out);
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
