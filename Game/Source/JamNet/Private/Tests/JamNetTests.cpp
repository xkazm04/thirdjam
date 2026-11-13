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

		uint64 Hash() const { return Sim->StateHash() ^ (Fx->StateHash() * 31); }

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
		TestEqual(TEXT("protocol 5 carries the spin"), static_cast<int32>(JamNet::Version), 5);
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

#endif
