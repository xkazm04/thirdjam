// Lane A9: the bytes that travel. Input packets (a window of quantised FJamInput plus the latest state-hash checkpoint),
// lobby messages (HELLO / WELCOME / START / BYE) and the invite code. Design and numbers: docs/experiments/MULTIPLAYER.md.
// Both peers run the same binary, so the layout is "FMemoryWriter host order"; a version byte guards a mismatch.
// Lane A38 (R-0028 point 3): version 2. FJamInput grew the stick, poke, lift, pass and the held shot since H6; version 1 carried
// Move and three flag bits only, so the new verbs were silently dropped on both sides. Version 2 carries every field (9 bytes per
// input) and shrinks the header to 32-bit frame numbers: 99 B payload per packet at 60/s.
#pragma once

#include "CoreMinimal.h"
#include "JamSim.h"

JAMNET_API DECLARE_LOG_CATEGORY_EXTERN(LogJamNet, Log, All);

namespace JamNet
{
	constexpr uint32 Magic = 0x4A4E4554u;  // 'JNET'
	// 1: Move + shoot/check/brake (H6). 2: every FJamInput field, 32-bit frames in the header (H11.5, lane A38).
	constexpr uint8 Version = 2;
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

// FJamInput quantised for the wire: Move and Stick in 1/32767 steps (|v| <= 1 on both; the sim clamps the same way), the seven
// booleans as bits. Both sims consume the quantised value (UJamLockstepDriver::TryStep hands the quantised local input back), so
// what the sender applies is bit-identical to what the receiver applies.
//
// CHECKLIST when a lane adds a field to FJamInput (JamSim.h): add it here, in From(), ToInput(), CoveredFields() and WireBytes,
// and bump JamNet::Version. A field that is not here never reaches the other machine; Jam.Net.InputCoverage fails until it is listed.
struct JAMNET_API FJamNetQuantInput
{
	int16 X = 0;       // Move.X
	int16 Y = 0;       // Move.Y
	int16 StickX = 0;  // Stick.X (R-0013)
	int16 StickY = 0;  // Stick.Y
	uint8 Flags = 0;

	enum EFlag : uint8 { Shoot = 1, Check = 2, Brake = 4, ShootHeld = 8, Pass = 16, Poke = 32, Lift = 64 };
	// Edge presses: the driver keeps one read during a stall; the held levels (ShootHeld, Brake) are re-read every frame.
	static constexpr uint8 EdgeFlags = Shoot | Check | Pass | Poke | Lift;
	static constexpr int32 WireBytes = 9;

	static FJamNetQuantInput From(const FJamInput& In);
	FJamInput ToInput() const;
	// The FJamInput UPROPERTY names this struct carries (Jam.Net.InputCoverage compares them with the struct's reflection).
	static TArray<FName> CoveredFields();
	bool operator==(const FJamNetQuantInput& O) const { return X == O.X && Y == O.Y && StickX == O.StickX && StickY == O.StickY && Flags == O.Flags; }
};

struct JAMNET_API FJamNetInputPacket
{
	// magic 4 + version 1 + type 1 + first frame 4 + sender frame 4 + hash frame 4 + hash 8 + count 1.
	static constexpr int32 HeaderBytes = 27;

	// Frame of Inputs[0]; Inputs[i] is for FirstFrame + i. 32 bits on the wire (828 days at 60 Hz).
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

// Lane A38: the scripted human both the two-process proof (jam.ScriptHuman 1) and Jam.Net.LockstepTwoPeers drive. A copy of
// AHokejGameMode::ScriptedHumanInput (private there) that also provokes a body check on an opponent without the puck every few
// seconds, so a foul (R-0013) happens inside the proof. It uses every FJamInput verb the sim has: poke at the drop, pass, charged shot, check.
struct JAMNET_API FJamNetScriptedHuman
{
	int32 Charge = 0;
	int64 LastPass = -600;
	int64 LastCheck = -600;
	int32 ChaseTarget = -1;
	int64 ChaseUntil = -1;

	FJamInput Think(const UJamSim& Sim, int32 Me);
};
