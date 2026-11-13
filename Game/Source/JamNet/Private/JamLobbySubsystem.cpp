#include "JamLobbySubsystem.h"
#include "JamLink.h"
#include "JamLockstepDriver.h"
#include "JamNetPackets.h"
#include "JamCartridge.h"
#include "JamSim.h"
#include "LobbyWidget.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "HAL/IConsoleManager.h"

// Lane A38: the simulated bad link for measurements (0 / 0 = the real wire). Set on both processes' command lines, e.g.
// -dpcvars=jam.net.SimDelayMs=40,jam.net.SimLoss=0.02 (40 ms each way = an 80 ms round trip, 2% loss).
static TAutoConsoleVariable<float> CVarJamNetSimDelayMs(TEXT("jam.net.SimDelayMs"), 0.f, TEXT("Hold every outgoing JamNet datagram this many ms (simulated one-way latency)."));
// Lane A120 (track Q1): the refusal proof. A guest with this set sends its HELLO with this protocol byte instead of its own (another build, patched).
static TAutoConsoleVariable<int32> CVarJamNetTestVersion(TEXT("jam.net.TestVersion"), 0, TEXT("> 0: the guest's HELLO carries this protocol byte (the refusal proof: a guest on another build)."));
// Lane A120: a fixed UDP port for this process (host or guest), for runs on one machine beside other lanes' processes; 0 = the host scans BasePort.., the guest any.
static TAutoConsoleVariable<int32> CVarJamNetPort(TEXT("jam.net.Port"), 0, TEXT("> 0: bind exactly this UDP port (host and guest). The host's invite codes then read ip:port."));
static TAutoConsoleVariable<float> CVarJamNetSimLoss(TEXT("jam.net.SimLoss"), 0.f, TEXT("Drop this fraction of outgoing JamNet datagrams (simulated loss, 0..1)."));

namespace
{
	constexpr int32 ResendEvery = 30;   // lobby messages re-sent every half second until answered

	void ApplyImpairment(FJamUdpLink& Link, uint64 Seed)
	{
		const double DelayMs = CVarJamNetSimDelayMs.GetValueOnGameThread();
		const double Loss = CVarJamNetSimLoss.GetValueOnGameThread();
		if (DelayMs > 0.0 || Loss > 0.0)
		{
			Link.SetImpairment(DelayMs, Loss, Seed);
			UE_LOG(LogJamNet, Warning, TEXT("JAMNET simulated link: %.0f ms one-way, %.1f%% loss (jam.net.SimDelayMs / jam.net.SimLoss)"), DelayMs, Loss * 100.0);
		}
	}
	constexpr int32 StartSendTicks = 30; // host sends START for half a second before opening the level
	const TCHAR* RinkMap = TEXT("/Game/Maps/Rink");

	UJamLobbySubsystem* LobbyFor(UWorld* World)
	{
		return (World && World->GetGameInstance()) ? World->GetGameInstance()->GetSubsystem<UJamLobbySubsystem>() : nullptr;
	}

	FString DefaultName(bool bHost)
	{
		FString N = FPlatformProcess::UserName(false);
		return N.IsEmpty() ? FString(bHost ? TEXT("Host") : TEXT("Guest")) : N;
	}
}

void UJamLobbySubsystem::Deinitialize()
{
	ResetSession();
	Super::Deinitialize();
}

IJamLink* UJamLobbySubsystem::GetLink() const
{
	return Link.Get();
}

void UJamLobbySubsystem::ResetSession()
{
	if (Driver) Driver->Stop();
	Driver = nullptr;
	Link.Reset();
	bHost = false;
	bGuestAcked = false;
	Codes.Reset();
	CodeAddresses.Reset();
	BoundPort = -1;
	RemoteName.Reset();
	GuestIndex = -1;
	LobbyTicks = StartTicks = 0;
}

void UJamLobbySubsystem::SetState(EJamLobbyState S)
{
	if (State == S) return;
	State = S;
	UE_LOG(LogJamNet, Log, TEXT("JAMNET lobby state=%d host=%d cut=%s peer=%s"), static_cast<int32>(S), bHost ? 1 : 0, *Cut, Link ? *Link->PeerString() : TEXT("-"));
	OnChanged.Broadcast();
}

void UJamLobbySubsystem::Fail(const FString& Why)
{
	Error = Why;
	UE_LOG(LogJamNet, Warning, TEXT("JAMNET lobby failed: %s (%.3f s after the join)"), *Why, JoinedAt > 0.0 ? FPlatformTime::Seconds() - JoinedAt : -1.0);
	// Lane A120: the unattended proof (jam.net.QuitAtFrame) exits on a failed lobby too, with the screen's text in the log.
	static IConsoleVariable* QuitAt = IConsoleManager::Get().FindConsoleVariable(TEXT("jam.net.QuitAtFrame"));
	if (QuitAt && QuitAt->GetInt() > 0) { UE_LOG(LogJamNet, Log, TEXT("JAMNET screen: %s; quitting (jam.net.QuitAtFrame)"), *Why); FPlatformMisc::RequestExit(false); }
	if (Link && Link->HasPeer()) SendLobby(static_cast<uint8>(JamNet::EType::Bye));
	ResetSession();
	SetState(EJamLobbyState::Failed);
}

bool UJamLobbySubsystem::ComputeCartridgeHash(const FString& InCut, uint64& OutHash, uint64& OutSeed, int32& OutGuestIndex, FString& OutError) const
{
	const FString Path = UJamCartridge::ResolveCartridgePath(InCut);
	TArray<uint8> Bytes;
	if (!FFileHelper::LoadFileToArray(Bytes, *Path)) { OutError = FString::Printf(TEXT("cut \"%s\" not found at %s"), *InCut, *Path); return false; }
	TArray<FString> Errors;
	UJamCartridge* C = UJamCartridge::LoadFromFile(Path, Errors);
	if (!C) { OutError = FString::Printf(TEXT("cut \"%s\" rejected: %s"), *InCut, Errors.Num() ? *Errors[0] : TEXT("?")); return false; }
	OutHash = JamNetFnv64(Bytes);
	OutSeed = static_cast<uint64>(C->Seed);
	// The guest drives team 1's first skater that is not the back (deterministic from the cartridge on both sides).
	UJamSim* Probe = NewObject<UJamSim>();
	Probe->Init(C->Game, OutSeed);
	OutGuestIndex = -1;
	for (const FJamSkaterState& S : Probe->GetSkaters()) if (S.Team == 1 && S.Role != FName(TEXT("back"))) { OutGuestIndex = S.Index; break; }
	if (OutGuestIndex < 0) for (const FJamSkaterState& S : Probe->GetSkaters()) if (S.Team == 1) { OutGuestIndex = S.Index; break; }
	if (OutGuestIndex < 0) { OutError = TEXT("the cut has no skater on team 1 for a guest"); return false; }
	return true;
}

bool UJamLobbySubsystem::Host(const FString& InCut, const FString& Name)
{
	ResetSession();
	Error.Reset(); EndReason.Reset();
	Cut = InCut.IsEmpty() ? TEXT("hockey.regular") : InCut;
	LocalName = Name.IsEmpty() ? DefaultName(true) : Name;
	bHost = true;
	FString Why;
	if (!ComputeCartridgeHash(Cut, CartridgeHash, Seed, GuestIndex, Why)) { Fail(Why); return false; }
	Link = MakeUnique<FJamUdpLink>();
	int32 PortIndex = 0;
	const int32 FixedPort = CVarJamNetPort.GetValueOnGameThread();
	if (FixedPort > 0) { if (Link->Bind(FixedPort) <= 0) { Fail(FString::Printf(TEXT("UDP port %d is taken (jam.net.Port)"), FixedPort)); return false; } PortIndex = -1; }
	else for (; PortIndex < JamNet::PortSlots; ++PortIndex) if (Link->Bind(JamNet::BasePort + PortIndex) > 0) break;
	if (!Link->IsOpen()) { Fail(FString::Printf(TEXT("no free UDP port in %d..%d"), JamNet::BasePort, JamNet::BasePort + JamNet::PortSlots - 1)); return false; }
	ApplyImpairment(*Link, Seed * 2 + 1);
	BoundPort = Link->GetBoundPort();
	TArray<uint32> Addrs = FJamUdpLink::LocalAddresses();
	Addrs.Add(0x7F000001u);   // loopback last: the two-process test on one machine
	for (uint32 Ip : Addrs)
	{
		Codes.Add(PortIndex >= 0 ? FJamInviteCode::Encode(Ip, PortIndex) : FString::Printf(TEXT("%s:%d"), *FJamInviteCode::FormatIp(Ip), BoundPort));   // lane A120: a fixed port has no code
		CodeAddresses.Add(FJamInviteCode::FormatIp(Ip));
	}
	Driver = NewObject<UJamLockstepDriver>(this);
	UE_LOG(LogJamNet, Log, TEXT("JAMNET hosting cut=%s seed=%llu port=%d guest_index=%d codes: %s"), *Cut, static_cast<unsigned long long>(Seed), BoundPort, GuestIndex, *GetCodesText().Replace(TEXT("\n"), TEXT(" | ")));
	SetState(EJamLobbyState::Hosting);
	return true;
}

bool UJamLobbySubsystem::Join(const FString& Code, const FString& Name)
{
	ResetSession();
	Error.Reset(); EndReason.Reset();
	LocalName = Name.IsEmpty() ? DefaultName(false) : Name;
	bHost = false;
	uint32 Ip = 0; int32 Port = 0;
	if (!FJamInviteCode::Decode(Code, Ip, Port)) { Fail(FString::Printf(TEXT("\"%s\" is not an invite code (8 letters and digits, shown as ABCD-EFGH)"), *Code)); return false; }
	Link = MakeUnique<FJamUdpLink>();
	if (Link->Bind(FMath::Max(0, CVarJamNetPort.GetValueOnGameThread())) <= 0) { Fail(TEXT("could not open a UDP socket")); return false; }
	ApplyImpairment(*Link, 2);
	BoundPort = Link->GetBoundPort();
	Link->SetPeer(Ip, Port);
	JoinedAt = FPlatformTime::Seconds();
	Driver = NewObject<UJamLockstepDriver>(this);
	UE_LOG(LogJamNet, Log, TEXT("JAMNET joining %s:%d as %s from port %d"), *FJamInviteCode::FormatIp(Ip), Port, *LocalName, BoundPort);
	SetState(EJamLobbyState::Joining);
	SendLobby(static_cast<uint8>(JamNet::EType::Hello));
	return true;
}

bool UJamLobbySubsystem::StartMatch()
{
	if (!bHost || State != EJamLobbyState::Ready) return false;
	StartTicks = 0;
	SetState(EJamLobbyState::Starting);
	return true;
}

void UJamLobbySubsystem::Leave()
{
	if (Link && Link->HasPeer()) SendLobby(static_cast<uint8>(JamNet::EType::Bye));
	ResetSession();
	SetState(EJamLobbyState::Idle);
}

void UJamLobbySubsystem::NotifyMatchEnded(const FString& Reason)
{
	EndReason = Reason;
	if (State == EJamLobbyState::InMatch) SetState(EJamLobbyState::Ended);
}

void UJamLobbySubsystem::SendLobby(uint8 Type, int32 Flags)
{
	if (!Link || !Link->HasPeer()) return;
	FJamNetLobbyMessage M;
	M.Type = static_cast<JamNet::EType>(Type);
	M.Name = LocalName;
	M.Cut = Cut;
	M.CartridgeHash = CartridgeHash;
	M.Seed = Seed;
	M.GuestIndex = GuestIndex;
	M.Flags = Flags;
	TArray<uint8> Bytes;
	M.Write(Bytes);
	const int32 TestVersion = CVarJamNetTestVersion.GetValueOnGameThread();
	if (!bHost && TestVersion > 0 && Bytes.Num() > 4) Bytes[4] = static_cast<uint8>(TestVersion);
	Link->Send(Bytes);
}

void UJamLobbySubsystem::OpenRink()
{
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) { Fail(TEXT("no world to open the rink in")); return; }
	Driver->Start(Link.Get(), JamNet::InputDelay);
	SetState(EJamLobbyState::InMatch);
	UE_LOG(LogJamNet, Log, TEXT("JAMNET opening %s as %s cut=%s seed=%llu"), RinkMap, bHost ? TEXT("host") : TEXT("guest"), *Cut, static_cast<unsigned long long>(Seed));
	UGameplayStatics::OpenLevel(World, FName(RinkMap), true, FString::Printf(TEXT("game=/Script/JamNet.LobbyGameMode?cut=%s"), *Cut));
}

void UJamLobbySubsystem::Tick(float DeltaTime)
{
	if (!Link) return;
	++LobbyTicks;
	TickLobby();
}

void UJamLobbySubsystem::TickLobby()
{
	TArray<uint8> Bytes;
	FJamNetLobbyMessage M;
	switch (State)
	{
	case EJamLobbyState::Hosting:
	{
		uint32 Ip = 0; int32 Port = 0;
		while (Link->RecvFrom(Bytes, Ip, Port))
		{
			// Lane A104: a HELLO from another build gets a REFUSE naming this build's protocol (the guest shows it at once instead of "no answer").
			uint8 ForeignType = 0;
			const int32 Foreign = JamNetPeekForeignVersion(Bytes, ForeignType);
			if (Foreign >= 0 && ForeignType == static_cast<uint8>(JamNet::EType::Hello))
			{
				TArray<uint8> Refuse;
				JamNetWriteRefuse(Refuse);
				Link->SetPeer(Ip, Port);
				Link->Send(Refuse);
				UE_LOG(LogJamNet, Log, TEXT("JAMNET refused a protocol-%d guest from %s:%d (this build: %d)"), Foreign, *FJamInviteCode::FormatIp(Ip), Port, JamNet::Version);
				continue;
			}
			if (!FJamNetLobbyMessage::Read(Bytes, M) || M.Type != JamNet::EType::Hello) continue;
			Link->SetPeer(Ip, Port);
			RemoteName = M.Name;
			bGuestAcked = false;
			JoinedAt = FPlatformTime::Seconds();
			UE_LOG(LogJamNet, Log, TEXT("JAMNET guest \"%s\" from %s:%d"), *RemoteName, *FJamInviteCode::FormatIp(Ip), Port);
			SendLobby(static_cast<uint8>(JamNet::EType::Welcome));
			SetState(EJamLobbyState::Ready);
			break;
		}
		break;
	}
	case EJamLobbyState::Joining:
	{
		while (Link->Recv(Bytes))
		{
			int32 HostVersion = -1;   // lane A104: the host runs another build
			const int32 MyVersion = CVarJamNetTestVersion.GetValueOnGameThread() > 0 ? CVarJamNetTestVersion.GetValueOnGameThread() : JamNet::Version;   // lane A120: the byte this guest sent
			if (JamNetReadRefuse(Bytes, HostVersion) && HostVersion != MyVersion)
			{
				Fail(FString::Printf(TEXT("the host runs a different build (protocol %d, you have %d): update both"), HostVersion, MyVersion));
				return;
			}
			if (!FJamNetLobbyMessage::Read(Bytes, M)) continue;
			if (M.Type == JamNet::EType::Bye) { Fail(TEXT("the host left")); return; }
			if (M.Type != JamNet::EType::Welcome) continue;
			RemoteName = M.Name;
			Cut = M.Cut;
			FString Why;
			uint64 MyHash = 0, MySeed = 0; int32 MyGuest = -1;
			if (!ComputeCartridgeHash(Cut, MyHash, MySeed, MyGuest, Why)) { Fail(Why); return; }
			if (MyHash != M.CartridgeHash) { Fail(FString::Printf(TEXT("your cut \"%s\" differs from the host's; both need the same file"), *Cut)); return; }
			CartridgeHash = MyHash;
			Seed = M.Seed;
			GuestIndex = M.GuestIndex;
			SendLobby(static_cast<uint8>(JamNet::EType::Hello), 1);
			SetState(EJamLobbyState::Ready);
			break;
		}
		if (State == EJamLobbyState::Joining && LobbyTicks % ResendEvery == 0) SendLobby(static_cast<uint8>(JamNet::EType::Hello));
		if (State == EJamLobbyState::Joining && LobbyTicks > 60 * 60) Fail(TEXT("no answer from the host in 60 s (same network? port forwarded?)"));
		break;
	}
	case EJamLobbyState::Ready:
	{
		while (Link->Recv(Bytes))
		{
			if (!FJamNetLobbyMessage::Read(Bytes, M)) continue;
			if (M.Type == JamNet::EType::Bye) { Fail(FString::Printf(TEXT("%s left"), *RemoteName)); return; }
			if (bHost && M.Type == JamNet::EType::Hello) { if (M.Flags & 1) bGuestAcked = true; else SendLobby(static_cast<uint8>(JamNet::EType::Welcome)); }
			if (!bHost && M.Type == JamNet::EType::Welcome) SendLobby(static_cast<uint8>(JamNet::EType::Hello), 1);
			if (!bHost && M.Type == JamNet::EType::Start) { OpenRink(); return; }
		}
		if (bHost && !bGuestAcked && LobbyTicks % ResendEvery == 0) SendLobby(static_cast<uint8>(JamNet::EType::Welcome));
		if (bHost && bGuestAcked && bAutoStart) StartMatch();
		break;
	}
	case EJamLobbyState::Starting:
	{
		if (StartTicks % 5 == 0) SendLobby(static_cast<uint8>(JamNet::EType::Start));
		if (++StartTicks >= StartSendTicks) OpenRink();
		break;
	}
	case EJamLobbyState::InMatch:
	{
		// The driver owns receiving now. The host keeps repeating START until the guest's first input packet arrives.
		if (bHost && Driver && !Driver->HasHeardPeer() && LobbyTicks % 10 == 0) SendLobby(static_cast<uint8>(JamNet::EType::Start));
		break;
	}
	default: break;
	}
}

FString UJamLobbySubsystem::GetCodesText() const
{
	FString Out;
	for (int32 i = 0; i < Codes.Num(); ++i) Out += FString::Printf(TEXT("%s%s   (%s)"), i ? TEXT("\n") : TEXT(""), *Codes[i], *CodeAddresses[i]);
	return Out;
}

FString UJamLobbySubsystem::GetStatusText() const
{
	switch (State)
	{
	case EJamLobbyState::Idle: return TEXT("Host a match and read your friend the code, or type theirs.");
	case EJamLobbyState::Hosting: return FString::Printf(TEXT("Waiting for a friend... cut: %s"), *Cut);
	case EJamLobbyState::Joining: return TEXT("Calling the host...");
	case EJamLobbyState::Ready: return bHost ? FString::Printf(TEXT("%s is here. Press Start."), *RemoteName) : FString::Printf(TEXT("Joined %s. Waiting for the host to start..."), *RemoteName);
	case EJamLobbyState::Starting: return TEXT("Starting...");
	case EJamLobbyState::InMatch: return TEXT("In the match.");
	case EJamLobbyState::Ended: return FString::Printf(TEXT("Match over: %s"), *EndReason);
	case EJamLobbyState::Failed: return Error;
	}
	return FString();
}

FString UJamLobbySubsystem::GetWarningText() const
{
	return FString::Printf(TEXT("LAN only: your friend must be on the same network, unless your router forwards UDP port %d to this computer."), BoundPort > 0 ? BoundPort : JamNet::BasePort);
}

// ---- console (grep'd across Game/Source before naming; R-0003 point 4) ----

static FAutoConsoleCommandWithWorldAndArgs CmdJamLobby(TEXT("jam.lobby"), TEXT("Open the Play-with-a-friend lobby screen over the current map (lane A8's menu entry calls this)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
		if (!PC) { UE_LOG(LogJamNet, Warning, TEXT("jam.lobby: no player controller")); return; }
		ULobbyWidget::Open(PC);
	}));

static FAutoConsoleCommandWithWorldAndArgs CmdJamHost(TEXT("jam.host"), TEXT("jam.host <cut> [name] [auto]: host a match; 'auto' starts as soon as the guest is ready."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UJamLobbySubsystem* L = LobbyFor(World);
		if (!L) return;
		L->Host(Args.Num() > 0 ? Args[0] : FString(), Args.Num() > 1 ? Args[1] : FString());
		L->SetAutoStart(Args.Contains(TEXT("auto")));
		UE_LOG(LogJamNet, Display, TEXT("jam.host: %s"), *L->GetStatusText());
	}));

static FAutoConsoleCommandWithWorldAndArgs CmdJamJoin(TEXT("jam.join"), TEXT("jam.join <code|ip:port> [name]: join a hosted match."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UJamLobbySubsystem* L = LobbyFor(World);
		if (!L || Args.Num() < 1) return;
		L->Join(Args[0], Args.Num() > 1 ? Args[1] : FString());
		UE_LOG(LogJamNet, Display, TEXT("jam.join: %s"), *L->GetStatusText());
	}));

static FAutoConsoleCommandWithWorldAndArgs CmdJamStart(TEXT("jam.start"), TEXT("jam.start: host starts the match once the guest is ready."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		if (UJamLobbySubsystem* L = LobbyFor(World)) UE_LOG(LogJamNet, Display, TEXT("jam.start: %s"), L->StartMatch() ? TEXT("starting") : TEXT("not ready (host with a guest present only)"));
	}));

static FAutoConsoleCommandWithWorldAndArgs CmdJamLeave(TEXT("jam.leave"), TEXT("jam.leave: leave the lobby or the match."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		if (UJamLobbySubsystem* L = LobbyFor(World)) L->Leave();
	}));
