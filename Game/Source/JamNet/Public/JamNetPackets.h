// Lane A9: the bytes that travel. Input packets (a window of quantised FJamInput plus the latest state-hash checkpoint),
// lobby messages (HELLO / WELCOME / START / BYE) and the invite code. Design and numbers: docs/experiments/MULTIPLAYER.md.
// Both peers run the same binary, so the layout is "FMemoryWriter host order"; a version byte guards a mismatch.
#pragma once

#include "CoreMinimal.h"
#include "JamSim.h"

JAMNET_API DECLARE_LOG_CATEGORY_EXTERN(LogJamNet, Log, All);

namespace JamNet
{
	constexpr uint32 Magic = 0x4A4E4554u;  // 'JNET'
	constexpr uint8 Version = 1;
	// Frames between a local input being read and the frame it is applied on both sides.
	constexpr int32 InputDelay = 3;
	// Inputs per packet (the last Window frames of local input); loss is covered by the next packet, never by retransmit.
	constexpr int32 Window = 8;
	// state_hash exchanged every HashInterval frames.
	constexpr int32 HashInterval = 60;
	// UDP ports: BasePort + index, index 0..15 (4 bits in the invite code).
	constexpr int32 BasePort = 7700;
	constexpr int32 PortSlots = 16;
	// Engine ticks without a frame advancing before the match is declared lost.
	constexpr int32 StallLimitTicks = 180;
	constexpr int32 MaxDatagram = 512;

	enum class EType : uint8
	{
		None = 0,
		Hello = 1,    // guest -> host: I want to join (name)
		Welcome = 2,  // host -> guest: cut, cartridge hash, seed, your skater index, my name
		Start = 3,    // host -> guest: load the rink now
		Input = 4,    // both ways, every tick during the match
		Bye = 5,      // either: leaving
	};
}

// FJamInput quantised for the wire: Move in 1/32767 steps, flags packed. Both sims consume the quantised value.
struct JAMNET_API FJamNetQuantInput
{
	int16 X = 0;
	int16 Y = 0;
	uint8 Flags = 0;  // bit0 shoot, bit1 check, bit2 brake

	static FJamNetQuantInput From(const FJamInput& In);
	FJamInput ToInput() const;
	bool operator==(const FJamNetQuantInput& O) const { return X == O.X && Y == O.Y && Flags == O.Flags; }
};

struct JAMNET_API FJamNetInputPacket
{
	// Frame of Inputs[0]; Inputs[i] is for FirstFrame + i.
	int64 FirstFrame = 0;
	TArray<FJamNetQuantInput> Inputs;
	// Latest checkpoint the sender has; HashFrame -1 = none yet.
	int64 HashFrame = -1;
	uint64 Hash = 0;
	// The sender's current sim frame (diagnostics: how far apart the peers are).
	int64 SenderFrame = 0;

	void Write(TArray<uint8>& Out) const;
	static bool Read(const TArray<uint8>& Bytes, FJamNetInputPacket& Out);
};

struct JAMNET_API FJamNetLobbyMessage
{
	JamNet::EType Type = JamNet::EType::None;
	FString Name;        // sender's display name
	FString Cut;         // Welcome: cartridge name
	uint64 CartridgeHash = 0;  // Welcome / Hello(ack): FNV-1a 64 of the cartridge file bytes
	uint64 Seed = 0;     // Welcome / Start
	int32 GuestIndex = -1;  // Welcome: the skater the guest drives
	int32 Flags = 0;     // Hello: bit0 = "I have your Welcome" (ack); Start: unused

	void Write(TArray<uint8>& Out) const;
	static bool Read(const TArray<uint8>& Bytes, FJamNetLobbyMessage& Out);
};

// Peeks the type of a datagram (after magic and version); None when it is not ours.
JAMNET_API JamNet::EType JamNetPeekType(const TArray<uint8>& Bytes);

// 8 Crockford base32 characters (shown ABCD-EFGH): 32-bit IPv4, 4-bit port index, 4-bit checksum.
struct JAMNET_API FJamInviteCode
{
	static FString Encode(uint32 Ipv4, int32 PortIndex);
	// Accepts "ABCD-EFGH", "abcdefgh", with spaces; also "a.b.c.d:port" for tests. False on a bad checksum or shape.
	static bool Decode(const FString& Text, uint32& OutIpv4, int32& OutPort);
	static FString FormatIp(uint32 Ipv4);
	static bool ParseIp(const FString& Dotted, uint32& Out);
};

// FNV-1a 64 over bytes (the cartridge hash in WELCOME).
JAMNET_API uint64 JamNetFnv64(const TArray<uint8>& Bytes);
