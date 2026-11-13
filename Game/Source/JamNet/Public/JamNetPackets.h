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
	constexpr uint8 Version = 8;   // lane A104 (track Q1): the 60-frame checkpoint is JamNetLockstepHash (the sim's LockstepStateHash, the rules' and zones' hashes); a 7 peer would show a false DESYNC. 7: lane A84 (R-0041 point 3): FJamInput::bAbility travels (flag bit 2048, an edge). 6: lane A77 (R-0036): FJamInput::bPassHeld travels (flag bit 1024, a held level). 5: lane A51 (R-0034): FJamInput::bSpin travels (flag bit 512). 4: bSprint (flag 256, two flag bytes); 3: bDeke (flag 128, lane A39)
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
		Refuse = 6,   // lane A104: host -> guest: your HELLO carries another protocol version. 6 bytes (magic, the host's version, 6) whatever the version:
		              // JamNetPeekType never returns it; JamNetReadRefuse reads it from any build that has it.
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
	uint16 Flags = 0;

	enum EFlag : uint16 { Shoot = 1, Check = 2, Brake = 4, ShootHeld = 8, Pass = 16, Poke = 32, Lift = 64, Deke = 128, Sprint = 256, Spin = 512, PassHeld = 1024, Ability = 2048 };   // Ability: lane A84 (R-0041 point 3), an edge   // PassHeld: lane A77 (R-0036), a held level   // Spin: lane A51 (R-0034), an edge   // Sprint: lane A49 (R-0032), a held level   // Deke: lane A39 (R-0029 point 4)
	// Edge presses: the driver keeps one read during a stall; the held levels (ShootHeld, Brake, Sprint) are re-read every frame.
	static constexpr uint16 EdgeFlags = Shoot | Check | Pass | Poke | Lift | Deke | Spin | Ability;
	static constexpr int32 WireBytes = 10;   // two flag bytes since protocol 4

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

// Lane A104 (track Q1): the hash both peers exchange every HashInterval frames. The sim's LockstepStateHash (StateHash plus the cross-frame state it leaves
// out), the effects', the rules' and the zones' state hashes (protocol 7 and before: the sim's StateHash and the effects' only, so a rule or zone divergence
// showed only once it moved the sim). ALobbyGameMode and the Bar.Net soak call this one function.
class UJamEffectSystem;
class UJamRuleSystem;
class UJamZoneSystem;
JAMNET_API uint64 JamNetLockstepHash(const UJamSim* Sim, const UJamEffectSystem* Fx, const UJamRuleSystem* Rules, const UJamZoneSystem* Zones);

// Lane A104 (A56's proposal, SOAK-LOCKSTEP.md section 4): a guest on another build is told so instead of waiting 60 s for "no answer".
// JamNetPeekForeignVersion: the version byte of a datagram with our magic and ANOTHER version (OutType = its type byte), or -1.
JAMNET_API int32 JamNetPeekForeignVersion(const TArray<uint8>& Bytes, uint8& OutType);
JAMNET_API void JamNetWriteRefuse(TArray<uint8>& Out);
// True for a REFUSE of any version; OutHostVersion = the version of the build that sent it.
JAMNET_API bool JamNetReadRefuse(const TArray<uint8>& Bytes, int32& OutHostVersion);

// Lane A129 (track Q1): FJamPlaytester's F (lane A84's rule), shared by the soak and the lobby's scripted seat (jam.net.ScriptPlayer).
JAMNET_API bool JamNetPlaytesterAbility(const UJamSim& Sim, int32 Seat, const FJamInput& In);

// Lane A129 (track Q1): the rematch from the win / lose card, carried as lockstep input with no new wire field. While the sim is Over, a seat that wants the
// rematch holds brake + pass held + shoot held (three levels no seat holds together in play); the frame both seats' applied inputs hold it, both machines restart.
namespace JamNet
{
	inline void SetRematchReady(FJamInput& In) { In = FJamInput(); In.bBrake = true; In.bPassHeld = true; In.bShootHeld = true; }
	inline bool IsRematchReady(const FJamInput& In) { return In.bBrake && In.bPassHeld && In.bShootHeld; }
}

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
	// Lane A120 (track Q1): every verb (jam.net.ScriptVerbs 1, the two-process proof): sprint, the saucer (a held pass instead of the tap), brake, spin and F on
	// top of the poke, shot and check. Off = A38's script, bit for bit.
	bool bAllVerbs = false;
	int32 SaucerHold = 0;

	FJamInput Think(const UJamSim& Sim, int32 Me);
private:
	FJamInput ThinkBase(const UJamSim& Sim, int32 Me);
};
