// Lane A9 tests (headless, no world): Jam.Net.InviteCodeRoundTrip, Jam.Net.PacketRoundTrip, Jam.Net.LockstepTwoPeers. Run:
//   UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Net; Quit" -unattended -nullrhi -nosplash -log
//
// LockstepTwoPeers is the feasibility proof of docs/experiments/MULTIPLAYER.md: two UJamSim + two UJamBrain + two drivers over a
// loopback link with a 2-tick delay and 10% loss, 3600 frames (one minute of play), every 60-frame checkpoint agreeing and the
// final state hashes equal. A second run has peer B tick at half rate (a slow machine): A may never run more than Delay ahead.
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "JamCartridge.h"
#include "JamSim.h"
#include "JamBrain.h"
#include "JamSimAdapter.h"
#include "JamNetPackets.h"
#include "JamLink.h"
#include "JamLockstepDriver.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamNetTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

	// Scripted attacker (same rule as Jam.Core's tests): get behind the puck on the line to the goal, shoot when lined up.
	FJamInput Attack(const UJamSim& Sim, int32 Me)
	{
		const FJamSkaterState& S = Sim.GetSkaters()[Me];
		const FVector2D Puck = Sim.GetPuck().Pos;
		const FVector2D Goal(Sim.AttackGoalX(S.Team), 0.0);
		const FVector2D ToGoal = (Goal - Puck).GetSafeNormal();
		const FVector2D Behind = Puck - ToGoal * 0.7;
		FJamInput In;
		const FVector2D D = Behind - S.Pos;
		In.Move = D.Size() > 0.3 ? D.GetSafeNormal() : ToGoal;
		const FVector2D ToPuck = Puck - S.Pos;
		const double Dist = ToPuck.Size();
		const FVector2D Fwd = S.Forward();
		if (Dist < Sim.GetConfig().Shot.ReachM && FVector2D::DotProduct(Fwd, ToGoal) > 0.97 && FVector2D::DotProduct(Fwd, ToPuck / FMath::Max(Dist, 1e-6)) > 0.6) In.bShoot = true;
		return In;
	}

	// One machine: its own sim, brain and driver. LocalSlot is the skater this machine's human drives.
	struct FPeer
	{
		TStrongObjectPtr<UJamSim> Sim;
		TStrongObjectPtr<UJamBrain> Brain;
		TStrongObjectPtr<UJamLockstepDriver> Driver;
		int32 LocalSlot = 0, RemoteSlot = 0;
		TArray<FJamInput> Inputs;
		int32 Stalls = 0;

		void Init(const FJamGameConfig& G, uint64 Seed, IJamLink& Link, int32 InLocal, int32 InRemote, int32 Delay)
		{
			Sim.Reset(NewObject<UJamSim>());
			Sim->Init(G, Seed);
			Brain.Reset(NewObject<UJamBrain>());
			Brain->YieldMarginM = G.Ai.YieldMarginM; Brain->YieldRadiusM = G.Ai.YieldRadiusM; Brain->HoldRadiusM = G.Ai.HoldRadiusM;
			Brain->SpeedScale = G.Ai.SpeedScale; Brain->RedSkill = G.Ai.RedSkill;
			Brain->ApplyParams();
			Brain->ResetBrain();
			Driver.Reset(NewObject<UJamLockstepDriver>());
			Driver->Start(&Link, Delay);
			LocalSlot = InLocal; RemoteSlot = InRemote;
			Inputs.SetNum(Sim->GetSkaters().Num());
		}

		bool Step()
		{
			FJamInput L, R;
			if (!Driver->TryStep(Attack(*Sim, LocalSlot), L, R)) { ++Stalls; return false; }
			JamAI::FJamAIWorld W;
			JamAI::FJamSimAdapter::BuildWorld(*Sim, W);
			JamAI::FJamAIInput Out[JamAI::kMaxSkaters];
			Brain->Think(W, Out);
			for (int32 i = 0; i < Inputs.Num() && i < JamAI::kMaxSkaters; ++i)
			{
				if (i == W.PlayerIndex) continue;
				Inputs[i].Move = FVector2D(Out[i].Move.X, Out[i].Move.Y);
				Inputs[i].bShoot = Out[i].bShoot; Inputs[i].bCheck = Out[i].bCheck; Inputs[i].bBrake = Out[i].bBrake;
			}
			Inputs[LocalSlot] = L;
			Inputs[RemoteSlot] = R;
			Sim->Step(Inputs);
			Driver->RecordHash(Sim->GetFrame(), Sim->StateHash());
			return true;
		}
	};

	int32 GuestSlot(const UJamSim& Sim)
	{
		for (const FJamSkaterState& S : Sim.GetSkaters()) if (S.Team == 1 && S.Role != FName(TEXT("back"))) return S.Index;
		for (const FJamSkaterState& S : Sim.GetSkaters()) if (S.Team == 1) return S.Index;
		return -1;
	}

	// Runs two peers to TargetFrames. BEvery: peer B steps only every BEvery ticks (a slower machine).
	bool RunPair(FAutomationTestBase& T, const TCHAR* Label, const FJamGameConfig& G, uint64 Seed, int32 Delay, int32 LinkDelay, double Loss, int32 BEvery, int32 TargetFrames)
	{
		FJamLoopbackPair Link(LinkDelay, Loss, Seed * 7 + 1);
		FPeer A, B;
		UJamSim* Probe = NewObject<UJamSim>();
		Probe->Init(G, Seed);
		const int32 Host = Probe->GetHumanIndex(), Guest = GuestSlot(*Probe);
		if (Guest < 0) { T.AddError(TEXT("no guest slot on team 1")); return false; }
		A.Init(G, Seed, Link.A(), Host, Guest, Delay);
		B.Init(G, Seed, Link.B(), Guest, Host, Delay);
		int32 MaxAhead = 0;
		int64 Tick = 0;
		const int64 MaxTicks = static_cast<int64>(TargetFrames) * (BEvery + 2) + 600;
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
		const bool bFramesDone = A.Sim->GetFrame() == TargetFrames && B.Sim->GetFrame() == TargetFrames;
		const bool bSameHash = A.Sim->StateHash() == B.Sim->StateHash();
		const int32 Expected = TargetFrames / JamNet::HashInterval;
		T.TestTrue(*FString::Printf(TEXT("%s: both peers reached %d frames (A %lld, B %lld, ticks %lld)"), Label, TargetFrames, A.Sim->GetFrame(), B.Sim->GetFrame(), Tick), bFramesDone);
		T.TestFalse(*FString::Printf(TEXT("%s: A desynced"), Label), A.Driver->IsDesynced());
		T.TestFalse(*FString::Printf(TEXT("%s: B desynced"), Label), B.Driver->IsDesynced());
		T.TestFalse(*FString::Printf(TEXT("%s: A lost"), Label), A.Driver->IsLost());
		T.TestFalse(*FString::Printf(TEXT("%s: B lost"), Label), B.Driver->IsLost());
		T.TestTrue(*FString::Printf(TEXT("%s: final state hashes equal (%s vs %s)"), Label, *A.Sim->StateHashHex(), *B.Sim->StateHashHex()), bSameHash);
		T.TestEqual(*FString::Printf(TEXT("%s: A agreed on every checkpoint"), Label), A.Driver->GetAgreedCheckpoints(), Expected);
		T.TestEqual(*FString::Printf(TEXT("%s: B agreed on every checkpoint"), Label), B.Driver->GetAgreedCheckpoints(), Expected);
		T.TestTrue(*FString::Printf(TEXT("%s: peers never more than Delay (%d) frames apart (max %d)"), Label, Delay, MaxAhead), MaxAhead <= Delay);
		T.TestTrue(*FString::Printf(TEXT("%s: the match had play (score %d:%d, puck path %.0f m)"), Label, A.Sim->GetScore(0), A.Sim->GetScore(1), A.Sim->GetPuck().PathM), A.Sim->GetPuck().PathM > 30.0);
		T.AddInfo(FString::Printf(TEXT("%s: A %s | stalls=%d"), Label, *A.Driver->DescribeStats(), A.Stalls));
		T.AddInfo(FString::Printf(TEXT("%s: B %s | stalls=%d dropped=%d"), Label, *B.Driver->DescribeStats(), B.Stalls, Link.Dropped));
		return bFramesDone && bSameHash;
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNetPacketRoundTrip, "Jam.Net.PacketRoundTrip", JamNetTests::Flags)
bool FJamNetPacketRoundTrip::RunTest(const FString& Parameters)
{
	FJamNetInputPacket P;
	P.FirstFrame = 1234; P.SenderFrame = 1240; P.HashFrame = 1200; P.Hash = 0x0123456789ABCDEFull;
	for (int32 i = 0; i < JamNet::Window; ++i)
	{
		FJamInput In;
		In.Move = FVector2D(FMath::Cos(i * 0.7), FMath::Sin(i * 0.7)) * (i % 3 == 0 ? 1.0 : 0.37);
		In.bShoot = i % 2 == 0; In.bCheck = i % 3 == 0; In.bBrake = i == 5;
		P.Inputs.Add(FJamNetQuantInput::From(In));
	}
	TArray<uint8> Bytes;
	P.Write(Bytes);
	AddInfo(FString::Printf(TEXT("input packet with %d inputs = %d bytes payload (%d on the wire with IPv4+UDP)"), JamNet::Window, Bytes.Num(), Bytes.Num() + 28));
	TestTrue(TEXT("payload under 100 bytes"), Bytes.Num() < 100);
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
	TArray<FString> Errors;
	UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")), Errors);
	for (const FString& E : Errors) AddError(E);
	if (!C) return false;
	// Lossy LAN: 2-tick one-way delay, 10% loss, both machines at 60 Hz.
	RunPair(*this, TEXT("lossy"), C->Game, 7, JamNet::InputDelay, 2, 0.10, 1, 3600);
	// Slow peer: B ticks at 30 Hz, no loss; A must wait for it and never run ahead by more than Delay.
	RunPair(*this, TEXT("slow-peer"), C->Game, 2, JamNet::InputDelay, 1, 0.0, 2, 1800);
	// Minimum delay on a perfect link.
	RunPair(*this, TEXT("delay-1"), C->Game, 1, 1, 0, 0.0, 1, 1200);
	return true;
}

#endif
