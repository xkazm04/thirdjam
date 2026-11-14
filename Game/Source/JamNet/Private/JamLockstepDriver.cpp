#include "JamLockstepDriver.h"
#include "JamLink.h"
#include "HAL/IConsoleManager.h"

// Lane A140 (track Q1, evolution 5): how many presses a stall may queue. 1 gives back protocol 8's fold (every press read in a stall OR'd into one frame).
static TAutoConsoleVariable<int32> CVarJamNetPressQueue(TEXT("jam.net.PressQueue"), 16,
	TEXT("Edge presses read during a lockstep stall are queued one frame each, up to this many (a double tap survives). 1: the old fold into one frame."));

void UJamLockstepDriver::Start(IJamLink* InLink, int32 InDelay)
{
	Link = InLink;
	Delay = FMath::Clamp(InDelay, 1, JamNet::Window - 1);
	Frame = 0;
	LastPumpFrame = -1;
	Ticks = 0;
	LocalInputs.Reset();
	RemoteInputs.Reset();
	LocalHashes.Reset();
	RemoteHashes.Reset();
	LatestLocalHashFrame = -1;
	LastCheckedHashFrame = -1;
	Pending.Reset();
	LastRawEdges = 0;
	RemoteFrame = 0;
	MaxFrameGap = 0;
	StallTicks = MaxStallTicks = 0;
	Agreed = 0;
	DesyncFrame = -1;
	bDesynced = bLost = bHeard = bPeerLeft = false;
	// The first Delay frames are played with no input on both sides, so the first real input lands at frame Delay.
	for (int64 F = 0; F < Delay; ++F) LocalInputs.Add(F, FJamNetQuantInput());
}

void UJamLockstepDriver::Pump()
{
	if (!Link) return;
	++Ticks;
	Receive();
	CheckHashes();
	Send();
	if (Frame == LastPumpFrame)
	{
		++StallTicks;
		MaxStallTicks = FMath::Max(MaxStallTicks, StallTicks);
		// Before the peer is heard the other side may still be loading the level: ten times the limit.
		const int32 Limit = bHeard ? JamNet::StallLimitTicks : JamNet::StallLimitTicks * 10;
		if (StallTicks >= Limit && !bLost)
		{
			bLost = true;
			UE_LOG(LogJamNet, Warning, TEXT("JAMNET lost frame=%lld stall_ticks=%d heard=%d"), Frame, StallTicks, bHeard ? 1 : 0);
		}
	}
	else
	{
		StallTicks = 0;
	}
	LastPumpFrame = Frame;
}

void UJamLockstepDriver::Receive()
{
	TArray<uint8> Bytes;
	FJamNetInputPacket P;
	while (Link->Recv(Bytes))
	{
		const JamNet::EType Type = JamNetPeekType(Bytes);
		if (Type == JamNet::EType::Bye && !bLost)
		{
			// Lane A120: the peer left (Esc to the menu, quit, its match ended and it closed): end now instead of waiting out the stall limit.
			// Lane A204: also before it was heard (it closed while its rink loaded): the link is this session's own, so a BYE on it is never a leftover.
			bPeerLeft = bLost = true;
			UE_LOG(LogJamNet, Warning, TEXT("JAMNET lost frame=%lld peer said BYE"), Frame);
			continue;
		}
		if (Type != JamNet::EType::Input) continue;  // lobby leftovers
		if (!FJamNetInputPacket::Read(Bytes, P)) continue;
		// Lane A204 (FAULT 3): the ticks counted while the peer was unheard (its rink loading, limit StallLimitTicks * 10) must not meet the heard limit the
		// moment its first packet arrives: that called the connection lost at frame 0 exactly as a slow-loading peer arrived. The heard count starts here.
		if (!bHeard) StallTicks = 0;
		bHeard = true;
		RemoteFrame = FMath::Max(RemoteFrame, P.SenderFrame);
		MaxFrameGap = FMath::Max(MaxFrameGap, static_cast<int32>(FMath::Abs(P.SenderFrame - Frame)));
		for (int32 i = 0; i < P.Inputs.Num(); ++i)
		{
			const int64 F = P.FirstFrame + i;
			if (F >= Frame && !RemoteInputs.Contains(F)) RemoteInputs.Add(F, P.Inputs[i]);
		}
		// Every packet repeats the sender's latest checkpoint; each checkpoint is compared once.
		if (P.HashFrame > LastCheckedHashFrame && !RemoteHashes.Contains(P.HashFrame)) RemoteHashes.Add(P.HashFrame, P.Hash);
	}
}

void UJamLockstepDriver::Send()
{
	FJamNetInputPacket P;
	const int64 NextLocal = Frame + Delay;  // one past the highest local frame recorded
	P.FirstFrame = FMath::Max<int64>(0, NextLocal - JamNet::Window);
	for (int64 F = P.FirstFrame; F < NextLocal; ++F)
	{
		const FJamNetQuantInput* Q = LocalInputs.Find(F);
		P.Inputs.Add(Q ? *Q : FJamNetQuantInput());
	}
	P.SenderFrame = Frame;
	P.HashFrame = LatestLocalHashFrame;
	if (LatestLocalHashFrame >= 0) if (const uint64* H = LocalHashes.Find(LatestLocalHashFrame)) P.Hash = *H;
	TArray<uint8> Bytes;
	P.Write(Bytes);
	Link->Send(Bytes);
}

namespace
{
	// Lane A140: the edge flags in DescribePresses' order, and their names.
	const uint16 A140Edges[UJamLockstepDriver::NumEdges] = { FJamNetQuantInput::Shoot, FJamNetQuantInput::Check, FJamNetQuantInput::Pass, FJamNetQuantInput::Poke,
		FJamNetQuantInput::Lift, FJamNetQuantInput::Deke, FJamNetQuantInput::Spin, FJamNetQuantInput::Ability };
	const TCHAR* A140EdgeNames[UJamLockstepDriver::NumEdges] = { TEXT("shoot"), TEXT("check"), TEXT("pass"), TEXT("poke"), TEXT("lift"), TEXT("deke"), TEXT("spin"), TEXT("F") };
}

bool UJamLockstepDriver::TryStep(const FJamInput& Local, FJamInput& OutLocal, FJamInput& OutRemote)
{
	if (!Link || bDesynced || bLost) return false;
	const FJamNetQuantInput* R = RemoteInputs.Find(Frame);
	FJamNetQuantInput Q = FJamNetQuantInput::From(Local);
	// Lane A140: before the first frame (the wait for the peer's first input, up to 1.7 s on a LAN in the two-process runs) nothing is read: a key mashed on
	// the loading rink never belonged to the match (protocol 8 folded them all into frame Delay; queued they would fire for many frames after the drop).
	if (!R && Frame == 0) return false;
	// Lane A140: every edge press goes through the queue; a press read in a stall waits there (held levels are re-read each tick). A press of a verb
	// already waiting in the last entry starts a new entry, so a double tap in a stall reaches the sim as two presses on two frames (protocol 8 OR'd them
	// into one). With no stall the queue is empty and this tick's presses go out this tick, as before: a clean link plays exactly as protocol 8.
	// A verb's edge on two ticks in a row is a held key (Consume*Pressed gives a human one edge per physical press; the scripted seats hold edges):
	// while the previous tick's press of it still waits in the last entry, the repeat is absorbed there (counted as held, not read), so a held key
	// cannot build a backlog that delays every later press.
	const uint16 RawEdges = Q.Flags & FJamNetQuantInput::EdgeFlags;
	const uint16 Absorbed = Pending.Num() ? (RawEdges & LastRawEdges & Pending.Last()) : 0;
	LastRawEdges = RawEdges;
	const uint16 Edges = RawEdges & ~Absorbed;
	for (int32 e = 0; e < NumEdges; ++e) { if (Edges & A140Edges[e]) ++PressRead[e]; if (Absorbed & A140Edges[e]) ++PressHeld[e]; }
	if (Edges)
	{
		const int32 MaxPending = FMath::Max(1, CVarJamNetPressQueue.GetValueOnAnyThread());
		if (Pending.Num() == 0) Pending.Add(Edges);
		else if (!(Pending.Last() & Edges)) Pending.Last() |= Edges;
		else if (Pending.Num() < MaxPending) Pending.Add(Edges);
		else
		{
			for (int32 e = 0; e < NumEdges; ++e) if (Pending.Last() & Edges & A140Edges[e]) ++PressFolded[e];   // the queue is full: folded, counted
			Pending.Last() |= Edges;
		}
	}
	if (!R) return false;
	Q.Flags = static_cast<decltype(Q.Flags)>(Q.Flags & ~FJamNetQuantInput::EdgeFlags);
	if (Pending.Num()) { Q.Flags |= Pending[0]; Pending.RemoveAt(0); }
	LocalInputs.Add(Frame + Delay, Q);
	const FJamNetQuantInput* L = LocalInputs.Find(Frame);
	OutLocal = L ? L->ToInput() : FJamInput();
	if (L) for (int32 e = 0; e < NumEdges; ++e) if (L->Flags & A140Edges[e]) ++PressApplied[e];
	OutRemote = R->ToInput();
	++Frame;
	if ((Frame & 63) == 0) Prune();
	return true;
}

void UJamLockstepDriver::RecordHash(int64 SimFrame, uint64 Hash)
{
	if (SimFrame <= 0 || SimFrame % JamNet::HashInterval != 0) return;
	LocalHashes.Add(SimFrame, Hash);
	LatestLocalHashFrame = SimFrame;
	CheckHashes();
}

void UJamLockstepDriver::CheckHashes()
{
	TArray<int64> Done;
	for (const TPair<int64, uint64>& Rh : RemoteHashes)
	{
		const uint64* Lh = LocalHashes.Find(Rh.Key);
		if (!Lh) continue;
		Done.Add(Rh.Key);
		LastCheckedHashFrame = FMath::Max(LastCheckedHashFrame, Rh.Key);
		if (*Lh == Rh.Value)
		{
			++Agreed;
		}
		else if (!bDesynced)
		{
			bDesynced = true;
			DesyncFrame = Rh.Key;
			UE_LOG(LogJamNet, Error, TEXT("JAMNET desync frame=%lld local=%016llx remote=%016llx agreed_before=%d"), Rh.Key,
				static_cast<unsigned long long>(*Lh), static_cast<unsigned long long>(Rh.Value), Agreed);
		}
	}
	for (int64 F : Done) RemoteHashes.Remove(F);
}

void UJamLockstepDriver::Prune()
{
	const int64 Keep = Frame - 2 * JamNet::Window;
	for (auto It = LocalInputs.CreateIterator(); It; ++It) if (It.Key() < Keep) It.RemoveCurrent();
	for (auto It = RemoteInputs.CreateIterator(); It; ++It) if (It.Key() < Keep) It.RemoveCurrent();
	const int64 KeepHash = Frame - 10 * JamNet::HashInterval;
	for (auto It = LocalHashes.CreateIterator(); It; ++It) if (It.Key() < KeepHash && It.Key() != LatestLocalHashFrame) It.RemoveCurrent();
}

FString UJamLockstepDriver::DescribeStats() const
{
	const double Secs = FMath::Max<double>(1.0, static_cast<double>(Ticks) / 60.0);
	const int64 Sent = Link ? Link->BytesSent : 0, Recv = Link ? Link->BytesRecv : 0;
	const int64 PSent = Link ? Link->PacketsSent : 0, PRecv = Link ? Link->PacketsRecv : 0;
	return FString::Printf(TEXT("frame=%lld delay=%d agreed=%d desync=%d lost=%d ticks=%lld stall_max=%d gap_max=%d sent=%lldB/%lldpk recv=%lldB/%lldpk out=%.0fB/s in=%.0fB/s"),
		Frame, Delay, Agreed, bDesynced ? 1 : 0, bLost ? 1 : 0, Ticks, MaxStallTicks, MaxFrameGap, Sent, PSent, Recv, PRecv, Sent / Secs, Recv / Secs);
}

// Lane A140 (track Q1, evolution 5): see the header.
FString UJamLockstepDriver::DescribePresses() const
{
	int32 InFlight[NumEdges] = {};
	for (const TPair<int64, FJamNetQuantInput>& P : LocalInputs)
		if (P.Key >= Frame) for (int32 e = 0; e < NumEdges; ++e) if (P.Value.Flags & A140Edges[e]) ++InFlight[e];
	for (const uint16 P : Pending) for (int32 e = 0; e < NumEdges; ++e) if (P & A140Edges[e]) ++InFlight[e];
	FString Out = TEXT("presses=[");
	int32 Lost = 0;
	for (int32 e = 0; e < NumEdges; ++e)
	{
		Out += FString::Printf(TEXT("%s%s %d/%d/%d/%d"), e ? TEXT(" ") : TEXT(""), A140EdgeNames[e], PressRead[e], PressApplied[e], InFlight[e], PressFolded[e]);
		Lost += PressRead[e] - PressApplied[e] - InFlight[e];
	}
	int32 Held = 0;
	for (int32 e = 0; e < NumEdges; ++e) Held += PressHeld[e];
	return Out + FString::Printf(TEXT("] (read/applied/in_flight/folded) held_repeats=%d presses_lost=%d"), Held, Lost);
}

int32 UJamLockstepDriver::GetPressesLost() const
{
	int32 InFlight = 0, Read = 0, Applied = 0;
	for (const TPair<int64, FJamNetQuantInput>& P : LocalInputs)
		if (P.Key >= Frame) for (int32 e = 0; e < NumEdges; ++e) if (P.Value.Flags & A140Edges[e]) ++InFlight;
	for (const uint16 P : Pending) for (int32 e = 0; e < NumEdges; ++e) if (P & A140Edges[e]) ++InFlight;
	for (int32 e = 0; e < NumEdges; ++e) { Read += PressRead[e]; Applied += PressApplied[e]; }
	return Read - Applied - InFlight;
}

int32 UJamLockstepDriver::GetPressesInFlight(uint16 InFlagBits) const
{
	const uint16 FlagBits = InFlagBits & FJamNetQuantInput::EdgeFlags;   // held levels are not presses
	int32 N = 0;
	for (const TPair<int64, FJamNetQuantInput>& P : LocalInputs) if (P.Key >= Frame) for (int32 b = 0; b < 16; ++b) if (P.Value.Flags & FlagBits & (1 << b)) ++N;
	for (const uint16 P : Pending) for (int32 b = 0; b < 16; ++b) if (P & FlagBits & (1 << b)) ++N;
	return N;
}

int32 UJamLockstepDriver::GetPressesHeld(uint16 InFlagBits) const
{
	int32 N = 0;
	for (int32 e = 0; e < NumEdges; ++e) if (InFlagBits & A140Edges[e]) N += PressHeld[e];
	return N;
}
