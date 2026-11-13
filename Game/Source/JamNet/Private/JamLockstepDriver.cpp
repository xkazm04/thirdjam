#include "JamLockstepDriver.h"
#include "JamLink.h"

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
	PendingFlags = 0;
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
		if (Type == JamNet::EType::Bye && bHeard && !bLost)
		{
			// Lane A120: the peer left (Esc to the menu, quit, its match ended and it closed): end now instead of waiting out the stall limit.
			bPeerLeft = bLost = true;
			UE_LOG(LogJamNet, Warning, TEXT("JAMNET lost frame=%lld peer said BYE"), Frame);
			continue;
		}
		if (Type != JamNet::EType::Input) continue;  // lobby leftovers
		if (!FJamNetInputPacket::Read(Bytes, P)) continue;
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

bool UJamLockstepDriver::TryStep(const FJamInput& Local, FJamInput& OutLocal, FJamInput& OutRemote)
{
	if (!Link || bDesynced || bLost) return false;
	const FJamNetQuantInput* R = RemoteInputs.Find(Frame);
	FJamNetQuantInput Q = FJamNetQuantInput::From(Local);
	if (!R)
	{
		PendingFlags |= Q.Flags & FJamNetQuantInput::EdgeFlags;  // keep a press read during a stall (held levels are re-read)
		return false;
	}
	Q.Flags |= PendingFlags;
	PendingFlags = 0;
	LocalInputs.Add(Frame + Delay, Q);
	const FJamNetQuantInput* L = LocalInputs.Find(Frame);
	OutLocal = L ? L->ToInput() : FJamInput();
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
