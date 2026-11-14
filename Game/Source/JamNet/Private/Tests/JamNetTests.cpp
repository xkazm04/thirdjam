// Lane A9 tests (headless, no world): Jam.Net.InviteCodeRoundTrip, Jam.Net.PacketRoundTrip, Jam.Net.LockstepTwoPeers; lane A38 added
// Jam.Net.InputCoverage. Run:
//   UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Net; Quit" -unattended -nullrhi -nosplash -log
//
// LockstepTwoPeers is the proof of docs/experiments/MULTIPLAYER.md, re-run at H11.5 on the sim with the faceoff, the stick verbs, fouls,
// effects, rule hooks and zones: two UJamSim + rule / zone / effect systems + two UJamBrain + two drivers over a loopback link, 9000 frames
// (2.5 minutes of play) on hockey.boost (pickups) and hockey.chaos (rule hooks, zones, bumpers), every 60-frame checkpoint agreeing and the
// final hashes equal; a faceoff drop, a foul, a pickup take and a rule hook must have happened. The chaos run uses an 83 ms one-way /
// 2% loss link (the WAN case); a third run has peer B tick at half rate (a slow machine): A may never run more than Delay ahead.
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"
#include "JamCartridge.h"
#include "JamSim.h"
#include "JamBrain.h"
#include "JamSimAdapter.h"
#include "JamEffectSystem.h"
#include "JamRuleSystem.h"
#include "JamZones.h"
#include "JamEventBus.h"
#include "JamNetPackets.h"
#include "JamLink.h"
#include "JamLink.h"
#include "JamLockstepDriver.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamNetTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

	// One machine: its own sim, rule / zone / effect systems, brain and driver (what ALobbyGameMode builds). LocalSlot is the skater this machine's human drives.
	struct FPeer
	{
		TStrongObjectPtr<UJamSim> Sim;
		TStrongObjectPtr<UJamRuleSystem> Rules;
		TStrongObjectPtr<UJamZoneSystem> Zones;
		TStrongObjectPtr<UJamEffectSystem> Fx;
		TStrongObjectPtr<UJamBrain> Brain;
		TStrongObjectPtr<UJamLockstepDriver> Driver;
		FJamNetScriptedHuman Script;
		int32 LocalSlot = 0, RemoteSlot = 0;
		TArray<FJamInput> Inputs;
		int32 Stalls = 0;
		int32 Faceoffs = 0, Fouls = 0, Pickups = 0, Pokes = 0, Passes = 0;

		void Init(FAutomationTestBase& T, const UJamCartridge& C, uint64 Seed, IJamLink& Link, int32 InLocal, int32 InRemote, int32 Delay)
		{
			const FJamGameConfig& G = C.Game;
			Sim.Reset(NewObject<UJamSim>());
			Rules.Reset(NewObject<UJamRuleSystem>());
			Rules->Init(G);
			Sim->SetRuleHook(Rules.Get());
			Zones.Reset(NewObject<UJamZoneSystem>());
			Zones->Init(G);
			Sim->SetZoneSource(Zones.Get());
			Sim->Init(G, Seed);
			Fx.Reset(NewObject<UJamEffectSystem>());
			TArray<FString> Errors;
			Fx->Init(Sim.Get(), C.RawJson, Errors);
			for (const FString& E : Errors) T.AddError(E);
			Brain.Reset(NewObject<UJamBrain>());
			Brain->ApplyCartridge(C);
			Brain->ResetBrain();
			Driver.Reset(NewObject<UJamLockstepDriver>());
			Driver->Start(&Link, Delay);
			LocalSlot = InLocal; RemoteSlot = InRemote;
			Inputs.SetNum(Sim->GetSkaters().Num());
		}

		uint64 Hash() const { return JamNetLockstepHash(Sim.Get(), Fx.Get(), Rules.Get(), Zones.Get()); }   // ALobbyGameMode::LockstepHash (lane A104)

		bool Step()
		{
			FJamInput L, R;
			if (!Driver->TryStep(Script.Think(*Sim, LocalSlot), L, R)) { ++Stalls; return false; }
			JamAI::FJamAIWorld W;
			JamAI::FJamSimAdapter::BuildWorld(*Sim, W);
			JamAI::FJamAIInput Out[JamAI::kMaxSkaters];
			Brain->Think(W, Out);
			for (int32 i = 0; i < Inputs.Num() && i < JamAI::kMaxSkaters; ++i)
			{
				if (i == W.PlayerIndex) continue;
				Inputs[i].Move = FVector2D(Out[i].Move.X, Out[i].Move.Y);
				Inputs[i].bShoot = Out[i].bShoot; Inputs[i].bCheck = Out[i].bCheck; Inputs[i].bBrake = Out[i].bBrake;
				Inputs[i].bPass = Out[i].bPass; Inputs[i].Stick = FVector2D(Out[i].Stick.X, Out[i].Stick.Y); Inputs[i].bPoke = Out[i].bPoke; Inputs[i].bLift = Out[i].bLift;
			}
			Inputs[LocalSlot] = L;
			Inputs[RemoteSlot] = R;
			if (L.bPoke || R.bPoke) ++Pokes;
			if (L.bPass || R.bPass) ++Passes;
			Rules->SetScore(0, Sim->GetScore(0)); Rules->SetScore(1, Sim->GetScore(1));
			Sim->Step(Inputs);
			Zones->Update(Sim->GetFrame(), Sim->GetSkaters(), Sim->GetPuck());
			Fx->Step();
			Driver->RecordHash(Sim->GetFrame(), Hash());
			for (const FJamSimEvent& E : Sim->GetFrameEvents())
			{
				if (E.Name == JamEvents::Faceoff && E.Kind == FName(TEXT("drop"))) ++Faceoffs;
				else if (E.Name == JamEvents::Foul) ++Fouls;
			}
			for (const FJamSimEvent& E : Fx->GetFrameEvents()) if (E.Name == JamEvents::PickupTake) ++Pickups;
			return true;
		}
	};

	int32 GuestSlot(const UJamSim& Sim)
	{
		for (const FJamSkaterState& S : Sim.GetSkaters()) if (S.Team == 1 && S.Role != FName(TEXT("back"))) return S.Index;
		for (const FJamSkaterState& S : Sim.GetSkaters()) if (S.Team == 1) return S.Index;
		return -1;
	}

	struct FExpect { bool bFaceoff = false, bFoul = false, bPickup = false, bRuleHook = false; };

	// Runs two peers to TargetFrames. BEvery: peer B steps only every BEvery ticks (a slower machine).
	bool RunPair(FAutomationTestBase& T, const TCHAR* Label, const UJamCartridge& C, int32 Delay, int32 LinkDelay, double Loss, int32 BEvery, int32 TargetFrames, const FExpect& X)
	{
		const uint64 Seed = static_cast<uint64>(C.Seed);
		FJamLoopbackPair Link(LinkDelay, Loss, Seed * 7 + 1);
		FPeer A, B;
		UJamSim* Probe = NewObject<UJamSim>();
		Probe->Init(C.Game, Seed);
		const int32 Host = Probe->GetHumanIndex(), Guest = GuestSlot(*Probe);
		if (Guest < 0) { T.AddError(TEXT("no guest slot on team 1")); return false; }
		A.Init(T, C, Seed, Link.A(), Host, Guest, Delay);
		B.Init(T, C, Seed, Link.B(), Guest, Host, Delay);
		int32 MaxAhead = 0;
		int64 Tick = 0;
		const int64 MaxTicks = static_cast<int64>(TargetFrames) * (BEvery + 2) + 600;
		const double T0 = FPlatformTime::Seconds();
		for (; Tick < MaxTicks; ++Tick)
		{
			Link.Advance();
			A.Driver->Pump();
			B.Driver->Pump();
			if (A.Sim->GetFrame() < TargetFrames) A.Step();
			if (B.Sim->GetFrame() < TargetFrames && (Tick % BEvery) == 0) B.Step();
			MaxAhead = FMath::Max(MaxAhead, static_cast<int32>(FMath::Abs(A.Sim->GetFrame() - B.Sim->GetFrame())));
			if (A.Driver->IsDesynced() || B.Driver->IsDesynced() || A.Driver->IsLost() || B.Driver->IsLost()) break;
			if (A.Sim->GetFrame() >= TargetFrames && B.Sim->GetFrame() >= TargetFrames)
			{
				// Flush the last checkpoints.
				for (int32 i = 0; i < LinkDelay + 2; ++i) { Link.Advance(); A.Driver->Pump(); B.Driver->Pump(); }
				break;
			}
		}
		const double Secs = FPlatformTime::Seconds() - T0;
		const bool bFramesDone = A.Sim->GetFrame() == TargetFrames && B.Sim->GetFrame() == TargetFrames;
		const bool bSameHash = A.Hash() == B.Hash();
		const int32 Expected = TargetFrames / JamNet::HashInterval;
		T.TestTrue(*FString::Printf(TEXT("%s: both peers reached %d frames (A %lld, B %lld, ticks %lld)"), Label, TargetFrames, A.Sim->GetFrame(), B.Sim->GetFrame(), Tick), bFramesDone);
		T.TestFalse(*FString::Printf(TEXT("%s: A desynced"), Label), A.Driver->IsDesynced());
		T.TestFalse(*FString::Printf(TEXT("%s: B desynced"), Label), B.Driver->IsDesynced());
		T.TestFalse(*FString::Printf(TEXT("%s: A lost"), Label), A.Driver->IsLost());
		T.TestFalse(*FString::Printf(TEXT("%s: B lost"), Label), B.Driver->IsLost());
		T.TestTrue(*FString::Printf(TEXT("%s: final state hashes equal (sim %s vs %s, with effects %016llx vs %016llx)"), Label, *A.Sim->StateHashHex(), *B.Sim->StateHashHex(),
			static_cast<unsigned long long>(A.Hash()), static_cast<unsigned long long>(B.Hash())), bSameHash);
		T.TestEqual(*FString::Printf(TEXT("%s: A agreed on every checkpoint"), Label), A.Driver->GetAgreedCheckpoints(), Expected);
		T.TestEqual(*FString::Printf(TEXT("%s: B agreed on every checkpoint"), Label), B.Driver->GetAgreedCheckpoints(), Expected);
		T.TestTrue(*FString::Printf(TEXT("%s: peers never more than Delay (%d) frames apart (max %d)"), Label, Delay, MaxAhead), MaxAhead <= Delay);
		T.TestTrue(*FString::Printf(TEXT("%s: the match had play (score %d:%d, puck path %.0f m)"), Label, A.Sim->GetScore(0), A.Sim->GetScore(1), A.Sim->GetPuck().PathM), A.Sim->GetPuck().PathM > 30.0);
		T.TestTrue(*FString::Printf(TEXT("%s: both peers counted the same events (faceoffs %d/%d fouls %d/%d pickups %d/%d rule fires %d/%d)"), Label,
			A.Faceoffs, B.Faceoffs, A.Fouls, B.Fouls, A.Pickups, B.Pickups, A.Rules->GetTotalFires(), B.Rules->GetTotalFires()),
			A.Faceoffs == B.Faceoffs && A.Fouls == B.Fouls && A.Pickups == B.Pickups && A.Rules->GetTotalFires() == B.Rules->GetTotalFires());
		if (X.bFaceoff) { if (!(A.Faceoffs >= 1)) T.AddWarning(*FString::Printf(TEXT("%s: a faceoff drop happened (%d)"), Label, A.Faceoffs)); } // Director: occurrence is play-dependent (the curve moves it); lockstep is the hashes and the equal counts above
		if (X.bFoul) { if (!(A.Fouls >= 1)) T.AddWarning(*FString::Printf(TEXT("%s: a foul happened (%d)"), Label, A.Fouls)); } // Director: occurrence is play-dependent (the curve moves it); lockstep is the hashes and the equal counts above
		if (X.bPickup) { if (!(A.Pickups >= 1)) T.AddWarning(*FString::Printf(TEXT("%s: a pickup was taken (%d)"), Label, A.Pickups)); } // Director: occurrence is play-dependent (the curve moves it); lockstep is the hashes and the equal counts above
		if (X.bRuleHook) { if (!(A.Rules->GetTotalFires() >= 1)) T.AddWarning(*FString::Printf(TEXT("%s: a rule hook fired (%d)"), Label, A.Rules->GetTotalFires())); } // Director: occurrence is play-dependent (the curve moves it); lockstep is the hashes and the equal counts above
		T.TestTrue(*FString::Printf(TEXT("%s: the humans used the new verbs over the wire (pokes %d, passes %d)"), Label, A.Pokes, A.Passes), A.Pokes >= 1);
		T.AddInfo(FString::Printf(TEXT("%s: A %s | stalls=%d | faceoffs=%d fouls=%d pickups=%d rule_fires=%d pokes=%d passes=%d | %.1f s wall"), Label, *A.Driver->DescribeStats(), A.Stalls,
			A.Faceoffs, A.Fouls, A.Pickups, A.Rules->GetTotalFires(), A.Pokes, A.Passes, Secs));
		T.AddInfo(FString::Printf(TEXT("%s: B %s | stalls=%d dropped=%d"), Label, *B.Driver->DescribeStats(), B.Stalls, Link.Dropped));
		// Lane A140: every edge press each peer read reached its seat or is in flight at the end (the stall queue: protocol 8 folded 5 of 25 pokes on boost-lossy-lan, 21 of 37 on chaos-wan).
		T.AddInfo(FString::Printf(TEXT("%s: A %s | B %s"), Label, *A.Driver->DescribePresses(), *B.Driver->DescribePresses()));
		T.TestEqual(*FString::Printf(TEXT("%s: A: no press lost (lost %d, of them folded %d)"), Label, A.Driver->GetPressesLost(), A.Driver->GetPressesFolded()), A.Driver->GetPressesLost(), 0);
		T.TestEqual(*FString::Printf(TEXT("%s: B: no press lost (lost %d, of them folded %d)"), Label, B.Driver->GetPressesLost(), B.Driver->GetPressesFolded()), B.Driver->GetPressesLost(), 0);
		return bFramesDone && bSameHash;
	}

	UJamCartridge* Load(FAutomationTestBase& T, const TCHAR* Name)
	{
		TArray<FString> Errors;
		UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(Name), Errors);
		for (const FString& E : Errors) T.AddError(E);
		return C;
	}
}

using namespace JamNetTests;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNetInviteCodeRoundTrip, "Jam.Net.InviteCodeRoundTrip", JamNetTests::Flags)
bool FJamNetInviteCodeRoundTrip::RunTest(const FString& Parameters)
{
	const uint32 Ips[] = { 0x7F000001u, 0xC0A80164u, 0x0A00008Du, 0x64648667u, 0xFFFFFFFFu, 0u };
	for (uint32 Ip : Ips)
	{
		for (int32 Port = 0; Port < JamNet::PortSlots; Port += 5)
		{
			const FString Code = FJamInviteCode::Encode(Ip, Port);
			TestEqual(*FString::Printf(TEXT("code length for %s"), *FJamInviteCode::FormatIp(Ip)), Code.Len(), 9);
			uint32 OutIp = 0; int32 OutPort = 0;
			TestTrue(*FString::Printf(TEXT("decode %s"), *Code), FJamInviteCode::Decode(Code, OutIp, OutPort));
			TestEqual(TEXT("ip round trip"), OutIp, Ip);
			TestEqual(TEXT("port round trip"), OutPort, JamNet::BasePort + Port);
			TestTrue(TEXT("lower case and no hyphen decode too"), FJamInviteCode::Decode(Code.Replace(TEXT("-"), TEXT("")).ToLower(), OutIp, OutPort) && OutIp == Ip);
		}
	}
	// A typo in one character is caught by the checksum (for every position of one code).
	const FString Code = FJamInviteCode::Encode(0xC0A80164u, 3);
	int32 Caught = 0, Tried = 0;
	for (int32 i = 0; i < Code.Len(); ++i)
	{
		if (Code[i] == '-') continue;
		FString Bad = Code;
		Bad[i] = Code[i] == 'X' ? 'Y' : 'X';
		uint32 OutIp = 0; int32 OutPort = 0;
		++Tried;
		if (!FJamInviteCode::Decode(Bad, OutIp, OutPort) || OutIp != 0xC0A80164u || OutPort != JamNet::BasePort + 3) ++Caught;
	}
	TestEqual(TEXT("every single-character typo is caught or changes nothing"), Caught, Tried);
	uint32 OutIp = 0; int32 OutPort = 0;
	TestTrue(TEXT("address form"), FJamInviteCode::Decode(TEXT("127.0.0.1:7701"), OutIp, OutPort) && OutIp == 0x7F000001u && OutPort == 7701);
	TestFalse(TEXT("garbage refused"), FJamInviteCode::Decode(TEXT("HELLO"), OutIp, OutPort));
	AddInfo(FString::Printf(TEXT("127.0.0.1:7700 = %s, 192.168.1.100:7703 = %s"), *FJamInviteCode::Encode(0x7F000001u, 0), *Code));

	// Lane A146: the host lists only addresses another machine can reach, the LAN first, each with what it is good for. The adapters are the
	// ones the A device showed (lane A140): its LAN, WSL's virtual switch, Tailscale's tunnel, a Wi-Fi with no address, loopback.
	using K = FJamUdpLink::EAddrKind;
	struct FCase { uint32 Ip; bool bUp, bLoopIf, bGateway; const TCHAR* Desc; K Want; const TCHAR* What; };
	const FCase Cases[] = {
		{ 0x0A00008Du, true, false, true, TEXT("Realtek PCIe GbE Family Controller | Ethernet 2"), K::Lan, TEXT("the LAN adapter with the router") },
		{ 0xC0A80164u, true, false, true, TEXT("Intel(R) Wi-Fi 6 AX201 | Wi-Fi"), K::Lan, TEXT("a Wi-Fi LAN") },
		{ 0xAC19D001u, true, false, false, TEXT("Hyper-V Virtual Ethernet Adapter | vEthernet (WSL (Hyper-V firewall))"), K::Hidden, TEXT("WSL's virtual switch") },
		{ 0xC0A83801u, true, false, false, TEXT("VirtualBox Host-Only Ethernet Adapter | Ethernet 3"), K::Hidden, TEXT("VirtualBox host-only") },
		{ 0x64648667u, true, false, false, TEXT("Tailscale Tunnel | Tailscale"), K::Tailnet, TEXT("Tailscale's tunnel") },
		{ 0x64400001u, true, false, false, TEXT("Wintun Userspace Tunnel | vpn"), K::Tailnet, TEXT("a 100.64/10 overlay without a gateway") },
		{ 0xA9FE7599u, true, false, false, TEXT("Intel(R) Wi-Fi 6 AX201 | Wi-Fi"), K::Hidden, TEXT("link-local 169.254 (no address)") },
		{ 0x0A000005u, false, false, true, TEXT("Realtek USB GbE | Ethernet 4"), K::Hidden, TEXT("an adapter that is down") },
		{ 0x7F000001u, true, true, false, TEXT("Software Loopback Interface 1 | Loopback Pseudo-Interface 1"), K::ThisMachine, TEXT("loopback") },
		{ 0xC0A80002u, true, false, false, TEXT("Realtek PCIe GbE Family Controller | Ethernet"), K::Lan, TEXT("a cable between two machines (no router)") },
		{ 0u, true, false, true, TEXT(""), K::Hidden, TEXT("0.0.0.0") },
	};
	for (const FCase& C : Cases)
		TestEqual(*FString::Printf(TEXT("classify %s"), C.What), static_cast<int32>(FJamUdpLink::ClassifyAddress(C.Ip, C.bUp, C.bLoopIf, C.bGateway, C.Desc)), static_cast<int32>(C.Want));
	// The A device's list in Windows' order (WSL first) comes out LAN first, WSL and the dead Wi-Fi gone, Tailscale after the LAN.
	TArray<FJamUdpLink::FAddr> List;
	for (int32 i : { 2, 4, 6, 9, 0 })
	{
		FJamUdpLink::FAddr A; A.Ip = Cases[i].Ip; A.bGateway = Cases[i].bGateway;
		A.Kind = FJamUdpLink::ClassifyAddress(Cases[i].Ip, Cases[i].bUp, Cases[i].bLoopIf, Cases[i].bGateway, Cases[i].Desc);
		List.Add(A);
	}
	FJamUdpLink::SortForScreen(List);
	TestEqual(TEXT("three listed (LAN, cable LAN, Tailscale)"), List.Num(), 3);
	if (List.Num() == 3)
	{
		TestEqual(TEXT("the LAN with the router first"), List[0].Ip, Cases[0].Ip);
		TestEqual(TEXT("then the LAN without a router"), List[1].Ip, Cases[9].Ip);
		TestEqual(TEXT("Tailscale after the LAN"), List[2].Ip, Cases[4].Ip);
	}
	TestEqual(TEXT("label: LAN"), FString(FJamUdpLink::KindLabel(K::Lan)), FString(TEXT("same network")));
	TestEqual(TEXT("label: loopback"), FString(FJamUdpLink::KindLabel(K::ThisMachine)), FString(TEXT("this machine only")));
	// This machine's real list: no loopback, link-local or hidden address, LAN before Tailscale (only counts logged, no addresses).
	const TArray<FJamUdpLink::FAddr> Live = FJamUdpLink::ReachableAddresses();
	int32 Lan = 0, Tail = 0, Bad = 0;
	bool bOrdered = true;
	for (int32 i = 0; i < Live.Num(); ++i)
	{
		const uint32 Ip = Live[i].Ip;
		if ((Ip >> 24) == 127 || (Ip >> 16) == 0xA9FEu || Ip == 0 || Live[i].Kind == K::Hidden || Live[i].Kind == K::ThisMachine) ++Bad;
		if (Live[i].Kind == K::Lan) { ++Lan; if (Tail > 0) bOrdered = false; }
		if (Live[i].Kind == K::Tailnet) ++Tail;
	}
	TestEqual(TEXT("this machine: no loopback, link-local or hidden address listed"), Bad, 0);
	TestTrue(TEXT("this machine: the LAN before Tailscale"), bOrdered);
	AddInfo(FString::Printf(TEXT("this machine lists %d address(es): %d same network, %d Tailscale"), Live.Num(), Lan, Tail));
	return true;
}

// Lane A38: every UPROPERTY of FJamInput travels. Fails the moment a lane adds a field to FJamInput without adding it to FJamNetQuantInput
// (a field that stays local is a silent change of the match on one side: a desync at the first press, or a verb that does nothing online).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNetInputCoverage, "Jam.Net.InputCoverage", JamNetTests::Flags)
bool FJamNetInputCoverage::RunTest(const FString& Parameters)
{
	const TArray<FName> Covered = FJamNetQuantInput::CoveredFields();
	int32 Fields = 0;
	for (TFieldIterator<FProperty> It(FJamInput::StaticStruct()); It; ++It)
	{
		++Fields;
		TestTrue(*FString::Printf(TEXT("FJamInput::%s travels in the input packet (add it to FJamNetQuantInput: From, ToInput, CoveredFields, WireBytes; bump JamNet::Version)"), *It->GetName()),
			Covered.Contains(It->GetFName()));
	}
	TestEqual(TEXT("CoveredFields lists exactly the fields FJamInput has (a stale name means a field was removed or renamed)"), Covered.Num(), Fields);
	AddInfo(FString::Printf(TEXT("FJamInput has %d fields, the packet carries %d, %d bytes per input, protocol version %d"), Fields, Covered.Num(), FJamNetQuantInput::WireBytes, JamNet::Version));

	// Every field set to a non-default value survives the wire; quantising twice changes nothing.
	FJamInput In;
	In.Move = FVector2D(0.5, -0.25); In.Stick = FVector2D(-0.75, 0.125);
	In.bShoot = In.bShootHeld = In.bPass = In.bCheck = In.bBrake = In.bPoke = In.bLift = true;
	const FJamNetQuantInput Q = FJamNetQuantInput::From(In);
	const FJamInput Out = Q.ToInput();
	TestTrue(TEXT("Move round trip"), Out.Move.Equals(In.Move, 1.0 / 32767.0));
	TestTrue(TEXT("Stick round trip"), Out.Stick.Equals(In.Stick, 1.0 / 32767.0));
	TestTrue(TEXT("bShoot"), Out.bShoot); TestTrue(TEXT("bShootHeld"), Out.bShootHeld); TestTrue(TEXT("bPass"), Out.bPass); TestTrue(TEXT("bCheck"), Out.bCheck);
	TestTrue(TEXT("bBrake"), Out.bBrake); TestTrue(TEXT("bPoke"), Out.bPoke); TestTrue(TEXT("bLift"), Out.bLift);
	TestTrue(TEXT("quantisation is idempotent"), FJamNetQuantInput::From(Out) == Q);
	// Each flag alone: no bit is shared with another.
	for (int32 Bit = 0; Bit < 7; ++Bit)
	{
		FJamInput One;
		switch (Bit) { case 0: One.bShoot = true; break; case 1: One.bCheck = true; break; case 2: One.bBrake = true; break; case 3: One.bShootHeld = true; break;
			case 4: One.bPass = true; break; case 5: One.bPoke = true; break; default: One.bLift = true; break; }
		const FJamNetQuantInput Qo = FJamNetQuantInput::From(One);
		TestEqual(*FString::Printf(TEXT("flag bit %d alone"), Bit), static_cast<int32>(Qo.Flags), 1 << Bit);
		const FJamInput Back = Qo.ToInput();
		const int32 Set = (Back.bShoot ? 1 : 0) + (Back.bCheck ? 1 : 0) + (Back.bBrake ? 1 : 0) + (Back.bShootHeld ? 1 : 0) + (Back.bPass ? 1 : 0) + (Back.bPoke ? 1 : 0) + (Back.bLift ? 1 : 0);
		TestEqual(*FString::Printf(TEXT("flag bit %d comes back alone"), Bit), Set, 1);
	}
	// Lane A51 (R-0034): the spin is flag bit 512 (bit 9), alone, and comes back alone.
	{
		FJamInput One; One.bSpin = true;
		const FJamNetQuantInput Qo = FJamNetQuantInput::From(One);
		TestEqual(TEXT("spin flag bit 512 alone"), static_cast<int32>(Qo.Flags), 512);
		const FJamInput Back = Qo.ToInput();
		TestTrue(TEXT("spin comes back alone"), Back.bSpin && !Back.bShoot && !Back.bCheck && !Back.bBrake && !Back.bShootHeld && !Back.bPass && !Back.bPoke && !Back.bLift && !Back.bDeke && !Back.bSprint);
		TestEqual(TEXT("protocol 9: the held cross-check (lane A184); 8 the checkpoint is JamNetLockstepHash (lane A104); 7 the ability, 6 the held pass, 5 the spin"), static_cast<int32>(JamNet::Version), 9);
	}
	// Lane A184 (R-0064 point 1): the held cross-check is flag bit 4096 (bit 12), a held level (not an edge), alone, and comes back alone.
	{
		FJamInput One; One.bCrossHold = true;
		const FJamNetQuantInput Qo = FJamNetQuantInput::From(One);
		TestEqual(TEXT("cross-hold flag bit 4096 alone"), static_cast<int32>(Qo.Flags), 4096);
		TestTrue(TEXT("cross-hold is a level, not an edge"), (FJamNetQuantInput::EdgeFlags & FJamNetQuantInput::CrossHold) == 0);
		const FJamInput Back = Qo.ToInput();
		TestTrue(TEXT("cross-hold comes back alone"), Back.bCrossHold && !Back.bAbility && !Back.bSpin && !Back.bPassHeld && !Back.bShoot && !Back.bCheck && !Back.bPass && !Back.bLift && !Back.bDeke && !Back.bSprint);
	}
	// Lane A84 (R-0041 point 3): the ability is flag bit 2048 (bit 11), an edge, alone, and comes back alone.
	{
		FJamInput One; One.bAbility = true;
		const FJamNetQuantInput Qo = FJamNetQuantInput::From(One);
		TestEqual(TEXT("ability flag bit 2048 alone"), static_cast<int32>(Qo.Flags), 2048);
		TestTrue(TEXT("ability is an edge flag"), (FJamNetQuantInput::EdgeFlags & FJamNetQuantInput::Ability) != 0);
		const FJamInput Back = Qo.ToInput();
		TestTrue(TEXT("ability comes back alone"), Back.bAbility && !Back.bSpin && !Back.bPassHeld && !Back.bShoot && !Back.bCheck && !Back.bPass && !Back.bDeke && !Back.bSprint);
	}
	TestTrue(TEXT("an empty input is the zero wire value"), FJamNetQuantInput::From(FJamInput()) == FJamNetQuantInput());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNetPacketRoundTrip, "Jam.Net.PacketRoundTrip", JamNetTests::Flags)
bool FJamNetPacketRoundTrip::RunTest(const FString& Parameters)
{
	FJamNetInputPacket P;
	P.FirstFrame = 1234; P.SenderFrame = 1240; P.HashFrame = 1200; P.Hash = 0x0123456789ABCDEFull;
	for (int32 i = 0; i < JamNet::Window; ++i)
	{
		FJamInput In;
		In.Move = FVector2D(FMath::Cos(i * 0.7), FMath::Sin(i * 0.7)) * (i % 3 == 0 ? 1.0 : 0.37);
		In.Stick = FVector2D(FMath::Sin(i * 0.9), FMath::Cos(i * 0.9)) * 0.8;
		In.bShoot = i % 2 == 0; In.bCheck = i % 3 == 0; In.bBrake = i == 5; In.bShootHeld = i % 4 == 1; In.bPass = i == 2; In.bPoke = i == 6; In.bLift = i == 7;
		P.Inputs.Add(FJamNetQuantInput::From(In));
	}
	TArray<uint8> Bytes;
	P.Write(Bytes);
	AddInfo(FString::Printf(TEXT("input packet with %d inputs = %d bytes payload (%d on the wire with IPv4+UDP), %d B/s per direction at 60 packets/s"), JamNet::Window, Bytes.Num(), Bytes.Num() + 28, Bytes.Num() * 60));
	TestEqual(TEXT("payload is header + window x input"), Bytes.Num(), FJamNetInputPacket::HeaderBytes + JamNet::Window * FJamNetQuantInput::WireBytes);
	TestTrue(TEXT("payload under 120 bytes (two flag bytes since protocol 4: the sprint)"), Bytes.Num() < 120);
	FJamNetInputPacket Q;
	TestTrue(TEXT("read back"), FJamNetInputPacket::Read(Bytes, Q));
	TestEqual(TEXT("first frame"), Q.FirstFrame, P.FirstFrame);
	TestEqual(TEXT("sender frame"), Q.SenderFrame, P.SenderFrame);
	TestEqual(TEXT("hash frame"), Q.HashFrame, P.HashFrame);
	TestTrue(TEXT("hash"), Q.Hash == P.Hash);
	TestEqual(TEXT("count"), Q.Inputs.Num(), P.Inputs.Num());
	for (int32 i = 0; i < Q.Inputs.Num(); ++i) TestTrue(*FString::Printf(TEXT("input %d equal"), i), Q.Inputs[i] == P.Inputs[i]);
	// Quantise(ToInput(q)) is the identity: what the sender applies is what the receiver applies.
	for (const FJamNetQuantInput& Qi : P.Inputs) TestTrue(TEXT("quantisation is idempotent"), FJamNetQuantInput::From(Qi.ToInput()) == Qi);
	TestTrue(TEXT("peek type"), JamNetPeekType(Bytes) == JamNet::EType::Input);
	// "No checkpoint yet" survives the 32-bit header.
	FJamNetInputPacket None; None.FirstFrame = 0; None.HashFrame = -1;
	None.Write(Bytes);
	FJamNetInputPacket NoneBack;
	TestTrue(TEXT("empty packet reads"), FJamNetInputPacket::Read(Bytes, NoneBack) && NoneBack.HashFrame == -1 && NoneBack.Inputs.Num() == 0);
	// A version-1 datagram (same magic, version byte 1) is not ours any more: the two builds refuse each other instead of desyncing.
	P.Write(Bytes);
	Bytes[4] = 1;
	TestTrue(TEXT("an older protocol version is refused"), JamNetPeekType(Bytes) == JamNet::EType::None);

	FJamNetLobbyMessage M;
	M.Type = JamNet::EType::Welcome; M.Name = TEXT("Hostname With Spaces"); M.Cut = TEXT("hockey.regular"); M.CartridgeHash = 42; M.Seed = 7; M.GuestIndex = 4; M.Flags = 1;
	M.Write(Bytes);
	FJamNetLobbyMessage N;
	TestTrue(TEXT("lobby read back"), FJamNetLobbyMessage::Read(Bytes, N));
	TestTrue(TEXT("lobby fields"), N.Type == M.Type && N.Name == M.Name && N.Cut == M.Cut && N.CartridgeHash == 42 && N.Seed == 7 && N.GuestIndex == 4 && N.Flags == 1);
	TArray<uint8> Junk = { 1, 2, 3, 4, 5, 6, 7 };
	TestTrue(TEXT("junk is not ours"), JamNetPeekType(Junk) == JamNet::EType::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNetLockstepTwoPeers, "Jam.Net.LockstepTwoPeers", JamNetTests::Flags)
bool FJamNetLockstepTwoPeers::RunTest(const FString& Parameters)
{
	UJamCartridge* Boost = Load(*this, TEXT("hockey.boost"));
	UJamCartridge* Chaos = Load(*this, TEXT("hockey.chaos"));
	UJamCartridge* Regular = Load(*this, TEXT("hockey.regular"));
	if (!Boost || !Chaos || !Regular) return false;
	// hockey.boost on a lossy LAN: 2-tick one-way delay (33 ms), 10% loss, both at 60 Hz, 9000 frames; pickups are its thing.
	{ FExpect X; X.bFaceoff = X.bFoul = X.bPickup = true; RunPair(*this, TEXT("boost-lossy-lan"), *Boost, JamNet::InputDelay, 2, 0.10, 1, 9000, X); }
	// hockey.chaos on a WAN: 5-tick one-way delay (83 ms; a 167 ms round trip), 2% loss, 9000 frames; rule hooks (sudden death, shrink per goal), zones, bumpers.
	{ FExpect X; X.bFaceoff = X.bFoul = X.bRuleHook = true; RunPair(*this, TEXT("chaos-wan-83ms-2pct"), *Chaos, JamNet::InputDelay, 5, 0.02, 1, 9000, X); }
	// Slow peer: B ticks at 30 Hz, no loss; A must wait for it and never run ahead by more than Delay.
	{ FExpect X; X.bFaceoff = true; RunPair(*this, TEXT("regular-slow-peer"), *Regular, JamNet::InputDelay, 1, 0.0, 2, 1800, X); }
	// Minimum delay on a perfect link.
	{ FExpect X; RunPair(*this, TEXT("regular-delay-1"), *Regular, 1, 0, 0.0, 1, 1200, X); }
	return true;
}

// Lane A104 (track Q1): the lockstep checkpoint sees the cross-frame state StateHash() leaves out. Each field is changed alone on a live hockey.regular sim:
// StateHash() stays (the hole it had; the Determinism baseline and the neutral proofs pinned on it do not move), LockstepStateHash() and the game's
// JamNetLockstepHash change, and putting the field back gives the hash back. Then two sims fed the same busy inputs agree on the new hash every frame.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNetLockstepHashCovers, "Jam.Net.LockstepHashCovers", JamNetTests::Flags)
bool FJamNetLockstepHashCovers::RunTest(const FString& Parameters)
{
	UJamCartridge* C = Load(*this, TEXT("hockey.regular"));
	if (!C) return false;
	TStrongObjectPtr<UJamSim> Sim(NewObject<UJamSim>());
	if (!TestTrue(TEXT("sim inits"), Sim->Init(C->Game, 1))) return false;
	TArray<FJamInput> In;
	In.SetNum(Sim->GetSkaters().Num());
	for (int32 F = 0; F < 240; ++F) Sim->Step(In);
	TestTrue(TEXT("hockey.regular runs the first-step burst (the MoveHist ring is live)"), C->Game.Skater.BurstFrames > 0);
	FJamSkaterState& S = const_cast<FJamSkaterState&>(Sim->GetSkaters()[0]);   // test only: one field at a time
	const uint64 Plain0 = Sim->StateHash(), Lock0 = Sim->LockstepStateHash(), Net0 = JamNetLockstepHash(Sim.Get(), nullptr, nullptr, nullptr);
	auto Probe = [&](const TCHAR* Name, TFunctionRef<void()> Change, TFunctionRef<void()> Back)
	{
		Change();
		TestEqual(*FString::Printf(TEXT("%s: StateHash() does not see it (the hole)"), Name), Sim->StateHash(), Plain0);
		TestNotEqual(*FString::Printf(TEXT("%s: LockstepStateHash() sees it"), Name), Sim->LockstepStateHash(), Lock0);
		TestNotEqual(*FString::Printf(TEXT("%s: the checkpoint (JamNetLockstepHash) sees it"), Name), JamNetLockstepHash(Sim.Get(), nullptr, nullptr, nullptr), Net0);
		Back();
		TestEqual(*FString::Printf(TEXT("%s: put back, the hash is back"), Name), Sim->LockstepStateHash(), Lock0);
	};
	const int64 Carry = S.LastCarryFrame;
	Probe(TEXT("LastCarryFrame (inside the foul grace)"), [&] { S.LastCarryFrame = Sim->GetFrame(); }, [&] { S.LastCarryFrame = Carry; });
	const bool bSpr = S.bSprinting, bAim = S.bStickAimed, bArm = S.bPokeArmed;
	Probe(TEXT("bSprinting (read by TryLift's cross-check next frame)"), [&] { S.bSprinting = !bSpr; }, [&] { S.bSprinting = bSpr; });
	Probe(TEXT("bStickAimed (read by TryDeke next frame)"), [&] { S.bStickAimed = !bAim; }, [&] { S.bStickAimed = bAim; });
	Probe(TEXT("bPokeArmed (the miss stumble)"), [&] { S.bPokeArmed = !bArm; }, [&] { S.bPokeArmed = bArm; });
	const FVector2D Hist = S.MoveHist[3];
	Probe(TEXT("MoveHist (the burst's cut detector)"), [&] { S.MoveHist[3] = Hist.IsZero() ? FVector2D(1, 0) : FVector2D::ZeroVector; }, [&] { S.MoveHist[3] = Hist; });
	const int32 At = S.MoveHistAt;
	Probe(TEXT("MoveHistAt"), [&] { S.MoveHistAt = (At + 1) % UJamSim::BurstCutFrames; }, [&] { S.MoveHistAt = At; });
	// A read-out stays out of both: the visual's bump flag.
	S.bBumpThisFrame = !S.bBumpThisFrame;
	TestEqual(TEXT("a read-out (bBumpThisFrame) is in neither hash"), Sim->LockstepStateHash(), Lock0);
	S.bBumpThisFrame = !S.bBumpThisFrame;
	// Same inputs on two fresh sims: equal lockstep hashes every frame (the new fields are deterministic).
	TStrongObjectPtr<UJamSim> A(NewObject<UJamSim>()), B(NewObject<UJamSim>());
	A->Init(C->Game, 7); B->Init(C->Game, 7);
	int32 Differ = -1;
	for (int32 F = 0; F < 900 && Differ < 0; ++F)
	{
		for (int32 i = 0; i < In.Num(); ++i)
		{
			FJamInput& I = In[i];
			const double Ang = 0.37 * i + (F / 40) * 2.6;   // a sharp cut every 40 frames: the burst and the reverse brake fire
			I.Move = FVector2D(FMath::Cos(Ang), FMath::Sin(Ang));
			I.bSprint = (F / 90 + i) % 2 == 0;
			I.Stick = FVector2D(FMath::Cos(Ang * 1.7), FMath::Sin(Ang * 1.7)) * ((F / 25) % 2 ? 1.0 : 0.0);
			I.bPoke = (F + i) % 37 == 0; I.bLift = (F + i) % 53 == 0; I.bDeke = (F + i) % 61 == 0; I.bCheck = (F + i) % 29 == 0;
			I.bPassHeld = (F + 3 * i) % 70 < 30;
		}
		A->Step(In); B->Step(In);
		if (A->LockstepStateHash() != B->LockstepStateHash()) Differ = F;
	}
	TestEqual(TEXT("two sims, the same inputs: the lockstep hash agrees on every frame"), Differ, -1);
	return true;
}

// Lane A104 (track Q1): Restart() is Init(). A match that sprints, cuts and holds passes, restarted, replays the fresh sim's match hash for hash (both hashes):
// before, the stamina bar, the burst, the cut history and the pass hold carried into the rematch (AHokejGameMode's rematch calls Restart).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNetRestartIsInit, "Jam.Net.RestartIsInit", JamNetTests::Flags)
bool FJamNetRestartIsInit::RunTest(const FString& Parameters)
{
	UJamCartridge* C = Load(*this, TEXT("hockey.regular"));
	if (!C) return false;
	auto Inputs = [](int32 F, int32 N)
	{
		TArray<FJamInput> In;
		In.SetNum(N);
		for (int32 i = 0; i < N; ++i)
		{
			const double Ang = 0.5 * i + (F / 45) * 2.4;
			In[i].Move = FVector2D(FMath::Cos(Ang), FMath::Sin(Ang));
			In[i].bSprint = true;
			In[i].bPassHeld = (F + i) % 50 < 20;
			In[i].Stick = FVector2D(FMath::Cos(Ang), FMath::Sin(Ang));
		}
		return In;
	};
	TStrongObjectPtr<UJamSim> Fresh(NewObject<UJamSim>()), Again(NewObject<UJamSim>());
	Fresh->Init(C->Game, 3); Again->Init(C->Game, 3);
	const int32 N = Fresh->GetSkaters().Num();
	for (int32 F = 0; F < 500; ++F) Again->Step(Inputs(F + 1000, N));   // the first match: sprints the bars down, leaves holds and a cut history
	int32 SpentBars = 0;
	for (const FJamSkaterState& S : Again->GetSkaters()) SpentBars += (S.Stamina >= 0 && S.Stamina < Again->StaminaFull()) ? 1 : 0;
	TestTrue(TEXT("the first match left stamina bars below full"), SpentBars > 0);
	Again->Restart();
	int32 Differ = -1;
	for (int32 F = 0; F < 600 && Differ < 0; ++F)
	{
		Fresh->Step(Inputs(F, N)); Again->Step(Inputs(F, N));
		if (Fresh->StateHash() != Again->StateHash() || Fresh->LockstepStateHash() != Again->LockstepStateHash()) Differ = F;
	}
	TestEqual(TEXT("the restarted sim replays the fresh sim's match on every frame"), Differ, -1);
	return true;
}


// Lane A140 (track Q1, evolution 5): a restarted sim IS a fresh one. Two different histories (a busy one with checks, abilities, brakes, pokes and shots,
// and a quiet one), each restarted: every reflected skater and puck field equals a freshly Init'ed sim's, both hashes equal it, and the three replay the
// same match on every frame for 600 frames. Run on the Shotgun mode with the guest seated (the two-machine rematch) and on the regular cartridge.
// Before: bBraking, bReverseBraking, BladePos, LastPassStrength, BumpDir, LastBumpFrame (lane A129's list) and more ran on into the rematch.
namespace JamNetTests
{
	TArray<FJamInput> A140Inputs(int32 F, int32 N, int32 Style)
	{
		TArray<FJamInput> In;
		In.SetNum(N);
		for (int32 i = 0; i < N; ++i)
		{
			const double Ang = 0.37 * i * (Style + 1) + (F / (30 + 15 * Style)) * 2.1;
			In[i].Move = FVector2D(FMath::Cos(Ang), FMath::Sin(Ang));
			In[i].Stick = FVector2D(FMath::Cos(-Ang), FMath::Sin(-Ang));
			if (Style == 0) continue;   // the quiet history and the replay: steering only
			const int32 K = (F * 7 + i * 13) % 97;
			In[i].bSprint = K < 40;
			In[i].bBrake = K >= 40 && K < 52;
			In[i].bShootHeld = K >= 52 && K < 70;
			In[i].bShoot = K == 70;
			In[i].bPassHeld = K >= 71 && K < 80;
			In[i].bPass = K == 80;
			In[i].bCheck = K == 81 || K == 82;
			In[i].bPoke = K == 83;
			In[i].bLift = K == 84;
			In[i].bDeke = K == 85;
			In[i].bSpin = K == 86;
			In[i].bAbility = K == 87;
		}
		return In;
	}

	// The reflected fields (every element of a fixed array) in which A and B differ, as " skater[i].Field" / " puck.Field".
	FString A140Differences(const UJamSim& A, const UJamSim& B)
	{
		FString Out;
		for (int32 i = 0; i < A.GetSkaters().Num() && i < B.GetSkaters().Num(); ++i)
			for (TFieldIterator<FProperty> It(FJamSkaterState::StaticStruct()); It; ++It)
				for (int32 k = 0; k < It->ArrayDim; ++k)
					if (!It->Identical_InContainer(&A.GetSkaters()[i], &B.GetSkaters()[i], k)) { Out += FString::Printf(TEXT(" skater[%d].%s"), i, *It->GetName()); break; }
		for (TFieldIterator<FProperty> It(FJamPuckState::StaticStruct()); It; ++It)
			for (int32 k = 0; k < It->ArrayDim; ++k)
				if (!It->Identical_InContainer(&A.GetPuck(), &B.GetPuck(), k)) { Out += FString::Printf(TEXT(" puck.%s"), *It->GetName()); break; }
		return Out;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNetRestartIsFresh, "Jam.Net.RestartIsFresh", JamNetTests::Flags)
bool FJamNetRestartIsFresh::RunTest(const FString& Parameters)
{
	struct FCase { const TCHAR* Cartridge; bool bGuest; };
	const FCase Cases[] = { { TEXT("modes/hockey.mode.shotgun"), true }, { TEXT("hockey.regular"), false } };
	for (const FCase& Case : Cases)
	{
		UJamCartridge* C = Load(*this, Case.Cartridge);
		if (!C) return false;
		TStrongObjectPtr<UJamSim> Fresh(NewObject<UJamSim>()), Busy(NewObject<UJamSim>()), Quiet(NewObject<UJamSim>());
		UJamSim* Sims[] = { Fresh.Get(), Busy.Get(), Quiet.Get() };
		int32 Guest = -1;
		for (UJamSim* S : Sims)
		{
			S->Init(C->Game, 11);
			if (Case.bGuest)
			{
				// The first bot of the other team, as the lobby seats it.
				const int32 HostTeam = S->GetSkaters()[S->GetHumanIndex()].Team;
				for (const FJamSkaterState& K : S->GetSkaters()) if (K.Team != HostTeam && !K.bHuman) { Guest = K.Index; break; }
				TestTrue(TEXT("the guest is seated"), S->SeatGuest(Guest));
			}
		}
		const int32 N = Fresh->GetSkaters().Num();
		for (int32 F = 0; F < 1500; ++F) Busy->Step(A140Inputs(F, N, 1));
		for (int32 F = 0; F < 900; ++F) Quiet->Step(A140Inputs(F, N, 2));
		// The busy history did what the rematch must forget.
		int32 Braked = 0, Bumped = 0, Passed = 0;
		for (const FJamSkaterState& K : Busy->GetSkaters()) { Braked += K.bBraking || K.bReverseBraking ? 1 : 0; Bumped += K.LastBumpFrame != -1000 ? 1 : 0; Passed += K.LastPassStrength != 0.f ? 1 : 0; }
		AddInfo(FString::Printf(TEXT("%s: before the restart the busy sim differs from a fresh one in:%s"), Case.Cartridge, *A140Differences(*Fresh, *Busy).Left(600)));
		AddInfo(FString::Printf(TEXT("%s: busy history: %d braking, %d bumped, %d passed; frame %lld, score %d-%d"), Case.Cartridge, Braked, Bumped, Passed,
			static_cast<long long>(Busy->GetFrame()), Busy->GetScore(0), Busy->GetScore(1)));
		Busy->Restart();
		Quiet->Restart();
		const FString DBusy = A140Differences(*Fresh, *Busy), DQuiet = A140Differences(*Fresh, *Quiet);
		TestTrue(FString::Printf(TEXT("%s: the busy history, restarted, equals a fresh sim in every reflected field (differs in:%s)"), Case.Cartridge, *DBusy), DBusy.IsEmpty());
		TestTrue(FString::Printf(TEXT("%s: the quiet history, restarted, equals a fresh sim in every reflected field (differs in:%s)"), Case.Cartridge, *DQuiet), DQuiet.IsEmpty());
		TestEqual(FString::Printf(TEXT("%s: restarted busy: the state hash of a fresh sim"), Case.Cartridge), Busy->StateHash(), Fresh->StateHash());
		TestEqual(FString::Printf(TEXT("%s: restarted busy: the lockstep hash of a fresh sim"), Case.Cartridge), Busy->LockstepStateHash(), Fresh->LockstepStateHash());
		TestEqual(FString::Printf(TEXT("%s: restarted quiet: the lockstep hash of a fresh sim"), Case.Cartridge), Quiet->LockstepStateHash(), Fresh->LockstepStateHash());
		int32 Differ = -1;
		for (int32 F = 0; F < 600 && Differ < 0; ++F)
		{
			const TArray<FJamInput> In = A140Inputs(F, N, 1);
			for (UJamSim* S : Sims) S->Step(In);
			if (Busy->StateHash() != Fresh->StateHash() || Busy->LockstepStateHash() != Fresh->LockstepStateHash()
				|| Quiet->StateHash() != Fresh->StateHash() || Quiet->LockstepStateHash() != Fresh->LockstepStateHash()) Differ = F;
		}
		TestEqual(FString::Printf(TEXT("%s: both restarted sims replay the fresh sim's match on every frame for 600 frames"), Case.Cartridge), Differ, -1);
		if (Differ >= 0) AddInfo(FString::Printf(TEXT("first differing frame %d; fields:%s"), Differ, *A140Differences(*Fresh, *Busy)));
	}
	return true;
}


// Lane A104 (A56's proposal): a HELLO from another protocol is seen as foreign (with its type), the REFUSE reads back from any version, and nothing of ours
// is mistaken for either.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNetRefuseMismatch, "Jam.Net.RefuseMismatch", JamNetTests::Flags)
bool FJamNetRefuseMismatch::RunTest(const FString& Parameters)
{
	FJamNetLobbyMessage Hello;
	Hello.Type = JamNet::EType::Hello;
	Hello.Name = TEXT("guest");
	TArray<uint8> Bytes;
	Hello.Write(Bytes);
	uint8 Type = 0;
	TestEqual(TEXT("our own HELLO is not foreign"), JamNetPeekForeignVersion(Bytes, Type), -1);
	Bytes[4] = 7;
	TestEqual(TEXT("a protocol-7 HELLO is foreign, version 7"), JamNetPeekForeignVersion(Bytes, Type), 7);
	TestEqual(TEXT("... and its type is HELLO"), static_cast<int32>(Type), static_cast<int32>(JamNet::EType::Hello));
	FJamNetLobbyMessage M;
	TestFalse(TEXT("... and FJamNetLobbyMessage::Read still refuses it"), FJamNetLobbyMessage::Read(Bytes, M));
	TArray<uint8> Refuse;
	JamNetWriteRefuse(Refuse);
	TestEqual(TEXT("REFUSE is 6 bytes"), Refuse.Num(), 6);
	int32 V = -1;
	TestTrue(TEXT("REFUSE reads"), JamNetReadRefuse(Refuse, V));
	TestEqual(TEXT("... with this build's version"), V, static_cast<int32>(JamNet::Version));
	Refuse[4] = 9;
	TestTrue(TEXT("a REFUSE from a protocol-9 build reads too"), JamNetReadRefuse(Refuse, V) && V == 9);
	TestEqual(TEXT("JamNetPeekType never returns REFUSE"), static_cast<int32>(JamNetPeekType(Refuse)), static_cast<int32>(JamNet::EType::None));
	TestFalse(TEXT("a HELLO is not a REFUSE"), JamNetReadRefuse(Bytes, V));
	TestFalse(TEXT("5 bytes are nothing"), JamNetReadRefuse(TArray<uint8>(Refuse.GetData(), 5), V));
	return true;
}

#endif
